#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileSystemModel>
#include <QFileDialog>
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
    void on_findButton_clicked();
    void on_setDumpDirectoryButton_clicked();

private:
    Ui::MainWindow *ui;
    QFileSystemModel* fileSystemModel;
    QMap<QString, QList<QString>> sidecarFileStructure;
};
#endif // MAINWINDOW_H
