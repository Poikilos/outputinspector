#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    bool bDebugBadHint=true;
    explicit MainWindow(QWidget *parent = 0);
    bool is_fatal_source_error(QString);
    QString sErrorsListFileName="err.txt";
    ~MainWindow();

private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void init();
    void readini();
    void CompensateForEditorVersion();
    QString getConvertedSourceErrorAndWarnElseGetUnmodified(QString);
};

#endif // MAINWINDOW_H
