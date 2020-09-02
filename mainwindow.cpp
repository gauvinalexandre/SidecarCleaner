#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->searchResultDisplay->setSelectionMode(QAbstractItemView::NoSelection);
    ui->searchResultDisplay->setAlternatingRowColors(true);
}

MainWindow::~MainWindow()
{
    delete ui;
}

// Set sidecar files search directory
void MainWindow::on_browseSearchDirectoryButton_clicked()
{
    // Get the choice from the user
    QString path = QFileDialog::getExistingDirectory(
                this,
                "Select the directory where to look for sidecar files.",
                lastSearchPath);

    if (path != "") {
        QDir directory(path);
        // Update the selected directory display
        ui->searchPathInput->setText(directory.path());
        directory.cdUp();
        lastSearchPath = directory.path();
        // Activate the search button
        ui->searchSidecarFilesButton->setEnabled(true);
    }
}

// Set sidecar files dump directory
void MainWindow::on_browseDumpDirectoryButton_clicked()
{
    // Get the choice from the user
    QString path = QFileDialog::getExistingDirectory(
                this,
                "Select the directory where to dump the sidecar files.",
                lastDumpPath);

    if (path != "") {
        QDir directory(path);
        // Update the selected directory display
        ui->dumpDirectoryInput->setText(directory.path());
        directory.cdUp();
        lastDumpPath = directory.path();
    }
}

void MainWindow::on_dumpFolderNameInput_textChanged(const QString &arg1)
{
    ui->dumpFolderNameInput->setToolTip(arg1);
}

void MainWindow::on_autoDumpFolderNameOption_toggled(bool checked)
{
    ui->dumpFolderNameInput->setEnabled(!checked);
    emit on_searchPathInput_textChanged(
                ui->searchPathInput->text());
}

void MainWindow::on_searchPathInput_textChanged(const QString &arg1)
{
    if(ui->autoDumpFolderNameOption->isChecked()) {
        if (QFile::exists(arg1)) {
            // Make up a dump folder name
            QString folderName(arg1);
            QString to_remove = ":.", to_replace = "/_ ";
            for (QChar c : to_remove) {
                folderName.remove(c);
            }
            for (QChar c : to_replace) {
                folderName.replace(c, "_");
            }
            ui->dumpFolderNameInput->setText(folderName);
        }
    }
}

// Search for sidecar files
void MainWindow::on_searchSidecarFilesButton_clicked()
{
    // Validate user inputs
    QString message;
    bool invalidInput = false;

    if (ui->sidecarSuffixInput->text() == "") {
        message = "Please specify a valid sidecar file suffix.";
        invalidInput = true;
    }

    if (ui->rawSuffixInput->text() == "") {
        message = "Please specify a valid raw file suffix.";
        invalidInput = true;
    }

    if (ui->rawSuffixInput->text().toLower() == ui->sidecarSuffixInput->text().toLower()) {
        message = "Please specify a sidecar file suffix that is different "
                  "from the raw file.";
        invalidInput = true;
    }

    if (!QFile::exists(ui->searchPathInput->text())) {
        message = "Please specify a valid search directory.";
        invalidInput = true;
    }

    if (invalidInput) {
        QMessageBox::information(this, "Invalid input", message);
        return;
    }

    // Suspend user interface
    ui->searchSidecarFilesButton->setEnabled(false);
    QString buttonText = ui->searchSidecarFilesButton->text();
    ui->searchSidecarFilesButton->setText("Searching...");

    // Reset database
    ui->searchResultDisplay->clear();
    ui->dumpSidecarFilesButton->setEnabled(false);
    sidecarFileStructure.clear();
    sidecarFileStructureRoot = ui->searchPathInput->text();
    qApp->processEvents();

    // Get user specified file suffixes
    QString rawFilesSuffix = ui->rawSuffixInput->text();
    QString sidecarFilesSuffix = ui->sidecarSuffixInput->text();

    // Initialize file search
    QDirIterator i(sidecarFileStructureRoot,
                   QStringList("*" + rawFilesSuffix),
                   QDir::Files,
                   QDirIterator::Subdirectories);
    quint64 numberRawFiles = 0;
    quint64 numberSidecarFiles = 0;
    quint64 totalFileSize = 0;
    QString rawFilename = "";
    QString sidecarFilename = "";
    QString currentPath = "";
    bool checkTimeCoherence = ui->timeCoherenceOption->isChecked();
    while (i.hasNext()) {
        rawFilename = i.next();
        numberRawFiles++;
        sidecarFilename = rawFilename.chopped(rawFilesSuffix.size()) + \
                sidecarFilesSuffix;
        if (QFile::exists(sidecarFilename)) {
            QFileInfo sidecarFileInfo(sidecarFilename);
            if (checkTimeCoherence) {
                QFileInfo rawFileInfo(rawFilename);
                if (rawFileInfo.lastModified() != sidecarFileInfo.lastModified()) {
                    continue;
                }
            }
            currentPath = sidecarFileInfo.path();
            if (!sidecarFileStructure.contains(currentPath)) {
                sidecarFileStructure.insert(currentPath, QList<QString>());
            }
            sidecarFileStructure[currentPath].append(sidecarFilename);
            numberSidecarFiles++;
            totalFileSize += sidecarFileInfo.size();
        }
    }

    // Save database
    QMap<QString, QList<QString>>::iterator j;
    for (j = sidecarFileStructure.begin(); j != sidecarFileStructure.end(); ++j) {
        ui->searchResultDisplay->addItems(j.value());
    }

    // Update user interface
    QLocale locale = this->locale();
    ui->numberRawFilesDisplay->setText(locale.toString(numberRawFiles));
    ui->numberSidecarFilesDisplay->setText(locale.toString(numberSidecarFiles));
    ui->totalSizeDisplay->setText(locale.formattedDataSize(totalFileSize));
    if (numberSidecarFiles > 0) {
        ui->dumpSidecarFilesButton->setEnabled(true);
    }

    // Reset user interface
    ui->searchSidecarFilesButton->setText(buttonText);
    ui->searchSidecarFilesButton->setEnabled(true);
}

void MainWindow::on_dumpSidecarFilesButton_clicked()
{
    // Validate user inputs
    QString message;
    bool invalidInput = false;

    if (!QFile::exists(ui->dumpDirectoryInput->text())) {
        message = "Please specify a valid dump directory.";
        invalidInput = true;
    }

    if (ui->dumpFolderNameInput->text() == "") {
        message = "Please specify a valid dump folder name.";
        invalidInput = true;
    }

    QDir dumpDirectory(ui->dumpDirectoryInput->text());

    if (!dumpDirectory.exists()) {
        message = "Please specify a valid dump directory.";
        invalidInput = true;
    }

    if (dumpDirectory.exists(ui->dumpFolderNameInput->text())) {
        message = "Dump folder already exist, please specify a different one.";
        invalidInput = true;
    }

    if (invalidInput) {
        QMessageBox::information(this, "Invalid input", message);
        return;
    }

    // Confirm user action
    if (QMessageBox::Yes != QMessageBox::question(
                this,
                "Action confirmation",
                "Are you sure you want to move the listed files to " +
                dumpDirectory.path() + " ?",
                QMessageBox::Yes | QMessageBox::No,
                QMessageBox::No)) {
        return;
    }

    // Suspend user interface
    ui->dumpSidecarFilesButton->setEnabled(false);
    QString buttonText = ui->dumpSidecarFilesButton->text();
    ui->dumpSidecarFilesButton->setText("Dumping...");
    qApp->processEvents();

    dumpDirectory.mkdir(ui->dumpFolderNameInput->text());
    dumpDirectory.cd(ui->dumpFolderNameInput->text());
    QList<QString> currentFileList;
    QFile currentFile;
    QString originFolder;
    QString destinationFolder;
    QString destinationFileName;
    while(!sidecarFileStructure.empty()) {
        originFolder = QString(sidecarFileStructure.firstKey());
        destinationFolder = QString(originFolder).replace(
                    sidecarFileStructureRoot, dumpDirectory.path());
        dumpDirectory.mkpath(destinationFolder);
        currentFileList = sidecarFileStructure.take(originFolder);
        while(!currentFileList.empty()) {
            currentFile.setFileName(currentFileList.takeFirst());
            destinationFileName = QString(destinationFolder + "/" +
                                      QFileInfo(currentFile).fileName());
            currentFile.rename(destinationFileName);
        }
    }

    // Reset user interface
    QLocale locale = this->locale();
    ui->numberSidecarFilesDisplay->setText(locale.toString(0));
    ui->totalSizeDisplay->setText(locale.formattedDataSize(0));
    ui->searchResultDisplay->clear();
    ui->dumpSidecarFilesButton->setText(buttonText);
}


