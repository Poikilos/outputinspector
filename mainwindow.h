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
    // QString sErrorsListFileName="err.txt";
    ~MainWindow();
    void init(QString);
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
    std::map<QString, QString>* getOutputLineInfo(const QString sLine, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    void getOutputLineInfo(std::map<QString, QString>* info, const QString sLineOriginal, const QString actualJump, const QString actualJumpLine, bool isPrevCallPrevLine);
    QString getAbsPathOrSame(QString sFile);
    std::map<QString, QBrush> brushes;

private slots:
    void on_mainListWidget_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    void readini();
    void CompensateForEditorVersion();
    QString getConvertedSourceErrorAndWarnElseGetUnmodified(QString);
};

#endif // MAINWINDOW_H
