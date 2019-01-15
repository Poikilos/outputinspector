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
    const QString COLLECT_REUSE = "REUSE";
    const QString STACK_LOWER = "LOWER";
    const int PARSE_MARKER_FILE = 0;
    const int PARSE_MARKER_PARAM_A = 1;
    const int PARSE_MARKER_PARAM_B = 2;
    const int PARSE_MARKER_END_PARAMS = 3;
    const int PARSE_COLLECT = 4;
    const int PARSE_STACK = 5;
    const int PARSE_PARTS_COUNT = 6;
    const int ROLE_COLLECTED_FILE = Qt::UserRole;
    const int ROLE_ROW = Qt::UserRole + 1;
    const int ROLE_COL = Qt::UserRole + 2;
    const int ROLE_LOWER = Qt::UserRole + 3;  // further down the call stack, probably not the error you're looking to find
    const int ROLE_COLLECTED_LINE = Qt::UserRole + 4;
    const int ROLE_DETAILS = Qt::UserRole + 5;
    std::list<QString> sInternalFlags;
    std::list<QString> sSectionBreakFlags;
    std::list<QStringList> enclosures;
    std::map<QString, QBrush> brushes;

    explicit MainWindow(QWidget *parent = 0);
    // QString sErrorsListFileName="err.txt";
    ~MainWindow();
    void readConfig();
    void setConfigValue(QString k, QString v);
    void init(QString);
    bool isFatalSourceError(QString);
    std::map<QString, QString>* lineInfo(const QString sLine, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    void lineInfo(std::map<QString, QString>* info, const QString sLineOriginal, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    QString absPathOrSame(QString sFile);

private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void CompensateForEditorVersion();
    QString getConvertedSourceErrorAndWarnElseGetUnmodified(QString);
    void cacheConfig();
};

#endif // MAINWINDOW_H
