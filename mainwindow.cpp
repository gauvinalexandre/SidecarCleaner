#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    fileSystemModel = new QFileSystemModel();
    fileSystemModel->setRootPath(QDir::currentPath());
    fileSystemModel->setFilter(QDir::Dirs|QDir::Drives|QDir::NoDotAndDotDot);
    ui->directorySelectionView->setModel(fileSystemModel);
    ui->directorySelectionView->setHeaderHidden(true);
    ui->directorySelectionView->header()->setSectionHidden(1, true);
    ui->directorySelectionView->header()->setSectionHidden(2, true);
    ui->directorySelectionView->header()->setSectionHidden(3, true);
    ui->resultList->setSelectionMode(QAbstractItemView::NoSelection);
    ui->resultList->setAlternatingRowColors(true);
}

MainWindow::~MainWindow()
{
    delete ui;
    delete fileSystemModel;
}

// Search for sidecar files
void MainWindow::on_findButton_clicked()
{
    ui->selectedSearchPath->setText(
                fileSystemModel->filePath(
                    ui->directorySelectionView->currentIndex()));
    if (QFile::exists(ui->selectedSearchPath->text())) {
        // Suspend interface
        QString findButtonText = ui->findButton->text();
        ui->findButton->setText("Searching...");

        // Reset interface
        ui->dumpButton->setEnabled(false);
        ui->findButton->setEnabled(false);
        ui->setDumpDirectoryButton->setEnabled(false);
        ui->selectedDumpDirectory->setText("");
        qApp->processEvents();

        // Reset database
        ui->resultList->clear();
        sidecarFileStructure.clear();

        // Get user specified file extensions
        QString rawFilesExtension = ui->rawExtensionInput->text();
        int extensionSize = rawFilesExtension.size();
        QString sidecarFilesExtension = ui->sidecarExtensionInput->text();

        // Initialize file search
        QDirIterator i(
                    ui->selectedSearchPath->text(),
                    QStringList("*." + rawFilesExtension),
                    QDir::Files,
                    QDirIterator::Subdirectories);
        quint64 numberRawFiles = 0;
        quint64 numberSidecarFiles = 0;
        quint64 totalFileSize = 0;
        QString rawFilename = "";
        QString baseFilename = "";
        QString sidecarFilename = "";
        QString currentPath = "";
        bool checkTimeCoherence = ui->timeCoherenceOption->isChecked();
        bool valid;
        while (i.hasNext()) {
            rawFilename = i.next();
            numberRawFiles++;
            baseFilename = rawFilename.chopped(extensionSize + 1);
            sidecarFilename = baseFilename + "." + sidecarFilesExtension;
            if (QFile::exists(sidecarFilename)) {
                valid = true;
                QFileInfo sidecarFileInfo(sidecarFilename);
                if (checkTimeCoherence) {
                    QFileInfo rawFileInfo(rawFilename);
                    valid = rawFileInfo.lastModified() == sidecarFileInfo.lastModified();
                }
                if (valid) {
                    currentPath = sidecarFileInfo.path();
                    if (!sidecarFileStructure.contains(currentPath)) {
                        sidecarFileStructure.insert(currentPath, QList<QString>());
                    }
                    sidecarFileStructure[currentPath].append(sidecarFilename);
                    numberSidecarFiles++;
                    totalFileSize += sidecarFileInfo.size();
                }
            }
        }

        // Save database
        QMap<QString, QList<QString>>::iterator j;
        for (j = sidecarFileStructure.begin(); j != sidecarFileStructure.end(); ++j) {
            ui->resultList->addItems(j.value());
        }

        // Update interface
        QLocale locale = this->locale();
        ui->numberRawFilesDisplay->setText(locale.toString(numberRawFiles));
        ui->numberSidecarFilesDisplay->setText(locale.toString(numberSidecarFiles));
        ui->totalSizeDisplay->setText(locale.formattedDataSize(totalFileSize));
        if (numberSidecarFiles > 0) {
            ui->setDumpDirectoryButton->setEnabled(true);
        }

        // Reactivate interface
        ui->findButton->setText(findButtonText);
        ui->findButton->setEnabled(true);
    }
}

// Set sidecar files dump directory
void MainWindow::on_setDumpDirectoryButton_clicked()
{
    QString directory = QFileDialog::getExistingDirectory(
                this,
                "Select a directory where to dump the sidecar files.");
    if (directory != "") {
        QString selectedSearchPath = ui->selectedSearchPath->text();
        ui->selectedDumpDirectory->setText(directory + "/" + \
                selectedSearchPath.remove(':').replace('/', '_'));
        ui->dumpButton->setEnabled(true);
    }
}
