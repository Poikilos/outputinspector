#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
//#include <map>

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
    //QString sErrorsListFileName="err.txt";
    ~MainWindow();
    void init(QString);
    const QString JUMP_FLAG = "  File ";
    const QString COLLECT_REUSE = "REUSE";
    const int JumpRole = Qt::UserRole;
    const int JumpLineRole = Qt::UserRole + 1;
    std::list<QStringList> enclosures;
    //QMap<QString, QString>* getLineInfo(QString sLine);
    std::map<QString, QString>* getOutputLineInfo(QString sLine, const QString actualJump, bool isPrevCallPrevLine);
    void getOutputLineInfo(std::map<QString, QString>* info, QString sLine, const QString actualJump, bool isPrevCallPrevLine);
    void addItemFromLine(QString sLine, QString& actualJump, QString& actualJumpLine);
    bool startsWithJumpFlag(QString sLine);
    QBrush brushTracebackNotTop = QBrush(Qt::gray);
    QBrush brushUnusable = QBrush(Qt::lightGray);

private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void readini();
    void CompensateForEditorVersion();
    QString getConvertedSourceErrorAndWarnElseGetUnmodified(QString);
};

#endif // MAINWINDOW_H
