#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QFileDialog>
#include <QMessageBox>
#include <QList>
#include <QMap>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_browseSearchDirectoryButton_clicked();
    void on_browseDumpDirectoryButton_clicked();
    void on_searchSidecarFilesButton_clicked();
    void on_dumpSidecarFilesButton_clicked();
    void on_dumpFolderNameInput_textChanged(const QString &arg1);
    void on_autoDumpFolderNameOption_toggled(bool checked);
    void on_searchPathInput_textChanged(const QString &arg1);

private:
    Ui::MainWindow *ui;
    QString lastSearchPath = QDir::homePath();
    QString lastDumpPath = QDir::homePath();
    QMap<QString, QList<QString>> sidecarFileStructure;
};
#endif // MAINWINDOW_H
