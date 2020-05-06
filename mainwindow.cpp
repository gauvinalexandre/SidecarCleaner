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

    if (ui->sidecarExtensionInput->text() == "") {
        message = "Please specify a valid sidecar file extension.";
        invalidInput = true;
    }

    if (ui->rawExtensionInput->text() == "") {
        message = "Please specify a valid raw file extension.";
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

    // Suspend interface
    ui->searchSidecarFilesButton->setEnabled(false);
    QString searchButtonTexte = ui->searchSidecarFilesButton->text();
    ui->searchSidecarFilesButton->setText("Searching...");

    // Reset database
    ui->searchResultDisplay->clear();
    ui->dumpSidecarFilesButton->setEnabled(false);
    sidecarFileStructure.clear();
    qApp->processEvents();

    // Get user specified file extensions
    QString rawFilesExtension = ui->rawExtensionInput->text();
    QString sidecarFilesExtension = ui->sidecarExtensionInput->text();

    // Initialize file search
    QDirIterator i(
                ui->searchPathInput->text(),
                QStringList("*." + rawFilesExtension),
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
        sidecarFilename = rawFilename.chopped(rawFilesExtension.size()) + \
                sidecarFilesExtension;
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

    // Update interface
    QLocale locale = this->locale();
    ui->numberRawFilesDisplay->setText(locale.toString(numberRawFiles));
    ui->numberSidecarFilesDisplay->setText(locale.toString(numberSidecarFiles));
    ui->totalSizeDisplay->setText(locale.formattedDataSize(totalFileSize));
    if (numberSidecarFiles > 0) {
        ui->dumpSidecarFilesButton->setEnabled(true);
    }

    // Reactivate interface
    ui->searchSidecarFilesButton->setText(searchButtonTexte);
    ui->searchSidecarFilesButton->setEnabled(true);
}

void MainWindow::on_dumpSidecarFilesButton_clicked()
{
    // Validate user inputs
    QString message;
    bool invalidInput = false;

    if (ui->dumpFolderNameInput->text() == "") {
        message = "Please specify a valid dump folder name.";
        invalidInput = true;
    }

    if (!QFile::exists(ui->dumpDirectoryInput->text())) {
        message = "Please specify a valid dump directory.";
        invalidInput = true;
    }

    if (invalidInput) {
        QMessageBox::information(this, "Invalid input", message);
        return;
    }
}


