#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QString>
#include <QProcess>
#include <QThread>
#include <QDebug>

int iErrors=0;
int iWarnings=0;
int iTODOs=0;
int iKate2TabWidth=8;
//int iKate3TabWidth=8;
int iCompilerTabWidth=6;
QStringList qslistFiles;
QStringList qslistTODOs;
bool bDebug=false;
bool bDebugTabs=false;
QString sDebug="";
QStatusBar* statusbarNow;
//start scripting
bool bFormatErr=false;
QString sFormatErrs;
QStringList stringsName;
QStringList stringsVal;
QStringList stringsErrors;
QStringList stringsWarnings;
bool bShowWarningsLast=false;
int iKateRevisionMajor=0;  // i.e. 2.5.9 is 2, kde3 version; and 3.0.3 is 3, the kde4 version
bool bForceOffset=false;
bool bCompensateForKateTabDifferences=true;

class OutputInspectorSleepThread : public QThread {
    public:
    static void msleep(unsigned long msecs) {
        QThread::msleep(msecs);
    }
};

bool IniHas(QString sNameX) {
    QString sReturn;
    int index=stringsName.indexOf(sNameX,0);
    return (index>-1);
}
QString IniString(QString sNameX) {
    QString sReturn;
    int index=stringsName.indexOf(sNameX,0);
    if (index>-1) sReturn=stringsVal.at(index);
    return sReturn;
}
bool IniBool(QString sNameX) {
    QString sReturn;
    int index=stringsName.indexOf(sNameX,0);
    if (index>-1) sReturn=stringsVal.at(index);
    return (sReturn.toLower()=="yes") || (sReturn.toLower()=="true") || (sReturn=="1");
}
int IniInt(QString sNameX) {
    QString sReturn;
    int iReturn=0;
    int index=stringsName.indexOf(sNameX,0);
    if (index>-1) {
        sReturn=stringsVal.at(index);
        bool bTest;
        iReturn=sReturn.toInt(&bTest,10);
        if (!bTest) {
            if (sFormatErrs=="") sFormatErrs=sReturn;
            else sFormatErrs=", "+sReturn;
            bFormatErr=true;
        }
    }
    return iReturn;
}
//end scripting
//start script vars
QString sKateCmd;
bool bExitOnNoErrors=false;
bool bFindTODOs=true;
int xEditorOffset=0;
int yEditorOffset=0;
//end script vars


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    //init(sErrorsListFileName);
    statusbarNow=ui->statusBar;

    QStringList stringsNoseSyntaxError;
    stringsNoseSyntaxError.append("ERROR: Failure: SyntaxError (invalid syntax (");
    stringsNoseSyntaxError.append(", line ");
    stringsNoseSyntaxError.append("");  // coordinate delimiter (blank if no column)
    stringsNoseSyntaxError.append("\n");  // what is after last coord
    stringsNoseSyntaxError.append("");  // REUSE mode if line in analyzed output should be also used as jump for following lines
    enclosures.push_back(stringsNoseSyntaxError);

    QStringList stringsNoseTracebackNonTop;
    stringsNoseTracebackNonTop.append("  File ");
    stringsNoseTracebackNonTop.append(", line ");
    stringsNoseTracebackNonTop.append("");
    stringsNoseTracebackNonTop.append(",");
    stringsNoseTracebackNonTop.append(COLLECT_REUSE);
    enclosures.push_back(stringsNoseTracebackNonTop);

    QStringList stringsNoseTracebackTop;
    stringsNoseTracebackTop.append("  File ");
    stringsNoseTracebackTop.append(", line ");
    stringsNoseTracebackTop.append("");
    stringsNoseTracebackTop.append("\n");
    stringsNoseTracebackNonTop.append(COLLECT_REUSE);
    enclosures.push_back(stringsNoseTracebackTop);

    // default must come LAST (in back):
    QStringList stringsDefault;
    stringsDefault.append("");
    stringsDefault.append("(");
    stringsDefault.append(",");
    stringsDefault.append(")");
    stringsNoseTracebackNonTop.append("");
    enclosures.push_back(stringsDefault);
}

bool MainWindow::is_fatal_source_error(QString sErrStreamLine)
{
    return (
                sErrStreamLine.indexOf("Can't open",0,Qt::CaseInsensitive)>-1 //jshint could not find a source file
                || sErrStreamLine.indexOf("Too many errors",0,Qt::CaseInsensitive)>-1 //jshint already showed the error for this line, but can't display more errors
                || sErrStreamLine.indexOf("could not be found",0,Qt::CaseInsensitive)>-1 //mcs could not find a source file
                || sErrStreamLine.indexOf("compilation failed",0,Qt::CaseInsensitive)>-1 //mcs could not compile the sources
            );
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::init(QString sErrorsListFileName) {
    readini();
    if (!bForceOffset) CompensateForEditorVersion();
    if (sErrorsListFileName.length()==0) {
        sErrorsListFileName = "err.txt";
    }
    // QTextStream err(stderr);  // avoid quotes and escapes caused by qWarning().noquote() being derived from qDebug()--alternative to qWarning().noquote().noquote()<<
    QFile qfileTest(sErrorsListFileName);
    QString sLine;
    QString sError="Error";
    QString sWarning="Warning";
    QString sCommentMark="//";
    //QString sToDo="//TODO";
    QString toDoString="TODO";
    //int cutToDoCount=2;
    //ui->mainListWidget is a QListWidget
    //setCentralWidget(ui->mainListWidget);
    //ui->mainListWidget->setSizePolicy(QSizePolicy::)
    int lineCount=0;
    int nonBlankLineCount=0;
    QString actualJump="";  // in case file & line# on different line than error, such as with nosetests
    QString actualJumpLine="";
    std::map<QString, QString>* info=new std::map<QString, QString>;
    if (qfileTest.exists()) {
        if (qfileTest.open(QFile::ReadOnly)) { //| QFile::Translate
            QTextStream qtextNow( &qfileTest );

            while ( !qtextNow.atEnd() ) {
                sLine=qtextNow.readLine(0); //does trim off newline characters
                lineCount++;
                QString sLinePrev=sLine;
                if (sLine.length()>0) {
                    if (sLine.trimmed().length()>0) nonBlankLineCount++;
                    if (!is_fatal_source_error(sLine)) {
                        sLine=getConvertedSourceErrorAndWarnElseGetUnmodified(sLine);
                        bool jshint_enable = false;
                        if (sLine!=sLinePrev) jshint_enable = true;


                        if (!bShowWarningsLast) {
                            addItemFromLine(sLine, actualJump, actualJumpLine);
                        }
                        else if (startsWithJumpFlag(sLine)) {
                            getOutputLineInfo(info, sLine, "", true);
                            actualJump = info->at("file");
                            actualJumpLine = sLine;
                            qInfo().noquote() << "(pushing value) set actualJump to" << actualJump;
                        }

                        getOutputLineInfo(info, sLine, actualJump, true);

//                        if (sLine.indexOf(".py",0,Qt::CaseInsensitive)>-1 ||
//                                (sLine.indexOf("env python",0,Qt::CaseInsensitive)>-1 && lineCount==1)) {
                        QString sTargetLanguage=(*info)["language"];

                        if (sTargetLanguage.length()>0) {
                            if (sTargetLanguage=="python" || sTargetLanguage=="sh") {
                                sCommentMark="#";
                            }
                            else if (sTargetLanguage=="c++" || sTargetLanguage=="c" || sTargetLanguage=="php" || sTargetLanguage=="js"
                                      || sTargetLanguage=="java") {
                                sCommentMark="//";
                            }
                            else if (sTargetLanguage=="bat") {
                                sCommentMark="rem ";
                            }
                        }

                        if ((jshint_enable && (*info)["file"].endsWith(".js"))  ||  sLine.indexOf(sError,0,Qt::CaseInsensitive)>-1) {
                            if (jshint_enable || sLine.indexOf("previous error",0,Qt::CaseInsensitive)<0) iErrors++;
                            if (bShowWarningsLast) stringsErrors.append(sLine);
                        }
                        else if (sLine.indexOf(sWarning,0,Qt::CaseInsensitive)>-1) {
                            iWarnings++;
                            if (bShowWarningsLast) stringsWarnings.append(sLine);
                        }
                        else {
                            iWarnings++;  // TODO: increment errors instead??
                            if (bShowWarningsLast) stringsWarnings.append(sLine);  // TODO: push to errors instead??
                        }

                        if (bFindTODOs) {
                            if (info->at("good")=="true") {
                                QString sFileX=info->at("file");  // =sLine.mid(0,sLine.indexOf("("));
                                if (!qslistFiles.contains(sFileX, Qt::CaseSensitive)) {
                                    qslistFiles.append(sFileX);
                                    QFile qfileSource(sFileX);
                                    if (bDebug) qDebug() << "outputinspector trying to open '"+sFileX+"'...";
                                    //if (!qfileSource.open(QFile::ReadOnly)) {

                                    //}
                                    if (qfileSource.open(QFile::ReadOnly)) {
                                        QTextStream qtextSource( &qfileSource );
                                        int iSourceLineFindToDo=0;
                                        while ( !qtextSource.atEnd() ) {
                                            QString sSourceLine=qtextSource.readLine(0);
                                            iSourceLineFindToDo++;//add first since compiler line numbering starts at 1
                                            //int iColTODOFound=-1; //sSourceLine.indexOf(sToDo,0,Qt::CaseInsensitive);
                                            int iToDoFound=-1;
                                            int iCommentFound=sSourceLine.indexOf(sCommentMark,0,Qt::CaseInsensitive);
                                            if (iCommentFound>-1) {
                                                iToDoFound=sSourceLine.indexOf(toDoString,iCommentFound+1,Qt::CaseInsensitive);
                                            }
                                            if (iToDoFound>-1) {
                                                QString sNumLine;
                                                sNumLine.setNum(iSourceLineFindToDo,10);
                                                QString sNumPos;
                                                int iCookedStart=iToDoFound; //iColTODOFound;
                                                for (int iNow=0; iNow<sSourceLine.length(); iNow++) {
                                                    if (sSourceLine.mid(iNow,1)=="\t") iCookedStart+=(iCompilerTabWidth-1);
                                                    else break;
                                                }
                                                iCookedStart+=1;//start numbering at 1 to mimic compiler
                                                iCookedStart+=2;//+2 to start after slashes
                                                sNumPos.setNum(iCookedStart,10);
                                                qslistTODOs.append(sFileX+"("+sNumLine+","+sNumPos+") "+sSourceLine.mid(iToDoFound)); //iColTODOFound+2
                                                iTODOs++;
                                            }
                                        }//end while not at end of source file
                                        if (bDebug) qDebug() << "outputinspector finished reading sourcecode";
                                        if (bDebug) qDebug() << "(processed " << iSourceLineFindToDo << " line(s))";
                                        qfileSource.close();
                                    }//end if could open sourcecode
                                    else {
                                        qWarning().noquote() << "[outputinspector] could not open source file '" + sFileX + "'";
                                    }
                                }//end if list does not already contain this file
                            }//end if found filename ender
                            else if (bDebug) qDebug() << "[outputinspector] WARNING: filename ender in "+sLine;
                        }//end if bFindTODOs
                        else qDebug() << "[outputinspector] WARNING: bFindTODOs off so skipped parsing "+sLine;
                    } // end if not a fatal error
                    else ui->mainListWidget->addItem(new QListWidgetItem(sLine+" <your compiler (or other tool) recorded this fatal or summary error before outputinspector ran>",ui->mainListWidget));
                }//end if length>0 (after trim using 0 option for readLine)
            }//end while not at end of file named sErrorsListFileName
            qfileTest.close();
            QString sNumErrors;
            sNumErrors.setNum(iErrors,10);
            QString sNumWarnings;
            sNumWarnings.setNum(iWarnings,10);
            QString sNumTODOs;
            sNumTODOs.setNum(iTODOs,10);
            if (bShowWarningsLast) {
                actualJump="";
                actualJumpLine="";
                while(!stringsErrors.isEmpty()) {
                    addItemFromLine(stringsErrors.takeFirst(), actualJump, actualJumpLine);
                }
                while(!stringsWarnings.isEmpty()) {
                    addItemFromLine(stringsWarnings.takeFirst(), actualJump, actualJumpLine);
                }
            }
            if (bFindTODOs) {
                while(!qslistTODOs.isEmpty()) {
                    ui->mainListWidget->addItem(new QListWidgetItem(qslistTODOs.takeFirst(),ui->mainListWidget));
                }
            }
            if (lineCount==0) {
                ui->mainListWidget->addItem(new QListWidgetItem("#"+sErrorsListFileName+": WARNING (generated by outputinspector) 0 lines in file",ui->mainListWidget));
            }
            else if (nonBlankLineCount==0) {
                ui->mainListWidget->addItem(new QListWidgetItem("#"+sErrorsListFileName+": WARNING (generated by outputinspector) 0 non-blank lines in file",ui->mainListWidget));
            }
            QString sMsg="Errors: "+sNumErrors+"; Warnings:"+sNumWarnings;
            if (bFindTODOs) sMsg+="; TODOs:"+sNumTODOs;
            ui->statusBar->showMessage(sMsg,0);
        }
        else {
            QString my_path = QCoreApplication::applicationFilePath();
            QMessageBox::information(this,"Output Inspector - Help",my_path+": Output Inspector cannot read the output file due to permissions or other read error (tried \"./"+sErrorsListFileName+"\").");
        }
    }//end if could open file named sErrorsListFileName
    else {
        QString my_path = QCoreApplication::applicationFilePath();
        QMessageBox::information(this,"Output Inspector - Help",my_path+": Output Inspector cannot find the output file to process (tried \"./"+sErrorsListFileName+"\").");
    }
    delete info;
}//end init

void MainWindow::readini() {
    QFile qfileTest("/etc/outputinspector.conf");
    QString sLine;
    bool bFoundKateCmd=false;
    if ( qfileTest.open(QFile::ReadOnly) ) { //| QFile::Translate
        QTextStream qtextNow( &qfileTest );
        while ( !qtextNow.atEnd() ) {
            sLine=qtextNow.readLine(0); //does trim off newline characters
            int iSign=sLine.indexOf("=");
            if (  (sLine.length()>2)  &&  (iSign>0)  &&  ( iSign<(sLine.length()-1) )  ) {
                stringsName.append( sLine.mid(0,iSign) );
                stringsVal.append( sLine.mid( iSign+1 , sLine.length()-(iSign+1) ) );
            }
        }
        QString sTemp;
        if (IniHas("kate")) {
            sKateCmd=IniString("kate");
            sDebug+="kate:"+sKateCmd+".  ";
            bFoundKateCmd=true;
        }
        else sDebug+="No kate line found in /etc/outputinspector.conf.  ";
        if (IniHas("ExitOnNoErrors")) {
            bExitOnNoErrors=IniBool("ExitOnNoErrors");
            sTemp=bExitOnNoErrors?"yes":"no";
            sDebug+="ExitOnNoErrors:"+sTemp+".  ";
        }
        else sDebug+="No ExitOnNoErrors line found in /etc/outputinspector.conf.  ";
        if (IniHas("FindTODOs")) {
            bFindTODOs=IniBool("FindTODOs");
            sTemp=bFindTODOs?"yes":"no";
            sDebug+="FindTODOs:"+sTemp+".  ";
        }
        else sDebug+="No FindTODOs line found in /etc/outputinspector.conf.  ";
        if (IniHas("xEditorOffset")) {
            xEditorOffset=IniInt("xEditorOffset");
            sTemp.setNum(xEditorOffset,10);
            sDebug+="xEditorOffset:"+sTemp+".  ";
            bForceOffset=true;
        }
        else sDebug+="No xEditorOffset line found in /etc/outputinspector.conf.  ";
        if (IniHas("yEditorOffset")) {
            yEditorOffset=IniInt("yEditorOffset");
            sTemp.setNum(yEditorOffset,10);
            sDebug+="yEditorOffset:"+sTemp+".  ";
            bForceOffset=true;
        }
        else sDebug+="No yEditorOffset line found in /etc/outputinspector.conf.  ";
        if (IniHas("Kate2TabWidth")) {
            iKate2TabWidth=IniInt("Kate2TabWidth");
            sTemp.setNum(iKate2TabWidth,10);
            sDebug+="Kate2TabWidth:"+sTemp+".  ";
        }
        else sDebug+="No Kate2TabWidth line found in /etc/outputinspector.conf.  ";
        //if (IniHas("Kate3TabWidth")) {
        //	iKate3TabWidth=IniInt("Kate3TabWidth");
        //	sTemp.setNum(iKate3TabWidth,10);
        //	sDebug+="Kate3TabWidth:"+sTemp+".  ";
        //}
        //else sDebug+="No Kate3TabWidth line found in /etc/outputinspector.conf.  ";
        if (IniHas("CompilerTabWidth")) {
            iCompilerTabWidth=IniInt("CompilerTabWidth");
            sTemp.setNum(iCompilerTabWidth,10);
            sDebug+="CompilerTabWidth:"+sTemp+".  ";
        }
        else sDebug+="No CompilerTabWidth line found in /etc/outputinspector.conf.  ";
        if (IniHas("ShowWarningsLast")) {
            bShowWarningsLast=IniBool("ShowWarningsLast");
            sTemp=bShowWarningsLast?"yes":"no";
            sDebug+="ShowWarningsLast:"+sTemp+".  ";
        }
        else sDebug+="No ShowWarningsLast line found in /etc/outputinspector.conf.  ";


        if (bDebug) QMessageBox::information(this,"Output Inspector - Debug",sDebug);
        if (bFormatErr) {
            QMessageBox::information(this,"Output Inspector - Configuration","There were errors in the following: "+sFormatErrs+".  The configuration file \"/etc/outputinspector.conf\" must not contain spaces or special characters.");
        }
        qfileTest.close();
    }
    else {
        if (bDebug) QMessageBox::information(this,"Output Inspector - Configuration","The configuration file \"/etc/outputinspector.conf\" could not be read.");
        sKateCmd="/usr/lib/kde4/bin/kate";
    }
    if (!bFoundKateCmd && !bDebug) {
        QMessageBox::information(this,"Output Inspector - Configuration","/etc/outputinspector.conf has no line reading \"kate=/usr/bin/kate\" so reverting to /usr/lib/kde4/bin/kate (in order to try to detect path and prevent this error, try running the following terminal command from inside the outputinspector directory: sudo ./install)");
    }
}//end readini
void MainWindow::CompensateForEditorVersion() {
    bool bFound=false;
    QStringList stringsVersionArgs;
    QString sFileTemp="/tmp/outputinspector.using.kate.version.tmp";
    stringsVersionArgs.append("--version");
    stringsVersionArgs.append(" > "+sFileTemp);
    //QProcess::execute(sKateCmd,stringsVersionArgs);
    system( (char*)QString(sKateCmd+" --version > "+sFileTemp).toLocal8Bit().data() );
    OutputInspectorSleepThread::msleep(125);

    QFile qfileTmp(sFileTemp);
    QString sLine;
    //bool bFoundKateCmd=false;
    if ( qfileTmp.open(QFile::ReadOnly) ) { //| QFile::Translate
        //detect Kate version using output of Kate command above
        QTextStream qtextNow( &qfileTmp );
        QString sKateOpener="Kate: ";
        while ( !qtextNow.atEnd() ) {
            sLine=qtextNow.readLine(0); //does trim off newline characters
            if (bDebug) QMessageBox::information(this,"Output Inspector - Finding Kate version...",sLine);
            if (sLine.startsWith(sKateOpener,Qt::CaseInsensitive)) {
                int iDot=sLine.indexOf(".",0);
                if (iDot>-1) {
                    bool bTest;
                    bFound=true;
                    iKateRevisionMajor=QString(sLine.mid(6,iDot-6)).toInt(&bTest,10);
                }
            }
        }
        qfileTmp.close();
    }//end if could open temp file
    QString sRevisionMajor;
    sRevisionMajor.setNum(iKateRevisionMajor,10);
    if (bDebug) QMessageBox::information(this,"Output Inspector - Kate Version",bFound?("Detected Kate "+sRevisionMajor):"Could not detect Kate version.");
    if (iKateRevisionMajor>2) {
        xEditorOffset=0;
        yEditorOffset=0;
    }
    else {
        xEditorOffset=0;
        yEditorOffset=0;
        //NOTE: no longer needed
        //xEditorOffset=-1;
        //yEditorOffset=-1;
    }
}//end CompensateForEditorVersion

// converts jshint output such as:
// functions.js: line 32, col 26, Use '!==' to compare with 'null'.
// to mcs format which is a result of:
// etc/foo.cs(10,24): error CS0103: The name `Path' does not exist in the current context
QString MainWindow::getConvertedSourceErrorAndWarnElseGetUnmodified(QString sLine)
{
    QString jshint_filename_ender = ": line ";
    int src_jshint_filename_ender_i = sLine.indexOf(jshint_filename_ender);
    //on purpose for readability:
    //* string operations are done separately and as soon as required info becomes available
    //* offset is used even on the second indexOf (even thougth first search term theoretically does not ever contain the second one)
    if (src_jshint_filename_ender_i>-1) {
        QString src_filename_s = sLine.mid(0,src_jshint_filename_ender_i);
        int src_row_i = src_jshint_filename_ender_i + jshint_filename_ender.length();
        QString src_line_ender = ", col ";
        int src_line_ender_i = sLine.indexOf(src_line_ender, src_row_i+1);
        if (src_line_ender_i>-1) {
            int src_row_len = src_line_ender_i-src_row_i;
            QString src_line_s = sLine.mid(src_row_i, src_row_len);
            int src_col_i = src_line_ender_i + src_line_ender.length();
            QString col_closer = ", ";
            int src_col_ender_i = sLine.indexOf(col_closer,src_col_i+1);
            if (src_col_ender_i>-1) {
                int src_col_len = src_col_ender_i - src_col_i;
                QString src_col_s = sLine.mid(src_col_i, src_col_len);
                int src_comment_i = src_col_ender_i + col_closer.length();
                QString src_comment_s = sLine.mid(src_comment_i);
                sLine = src_filename_s + "(" + src_line_s + "," + src_col_s + "): " + src_comment_s;
                //if (bDebugBadHint) {
                //    QMessageBox::information(this,"Output Inspector - Parsing Notice","error format was converted to "+sLine);
                //    bDebugBadHint=false;
                //}
            }
            else if (bDebugBadHint) {
                QMessageBox::information(this,"Output Inspector - Parsing Error","jshint parsing error: missing '"+col_closer+"' after column number after '"+src_line_ender+"' after '"+jshint_filename_ender+"'");
                bDebugBadHint=false;
            }
        }
        else if (bDebugBadHint) {
            QMessageBox::information(this,"Output Inspector - Parsing Error","jshint parsing error: missing '"+src_line_ender+"' after '"+jshint_filename_ender+"'");
            bDebugBadHint=false;
        }
    }
    else if (bDebugBadHint) {
        if (bDebug) {
            QMessageBox::information(this,"Output Inspector - Parsing Notice","Detected mcs error format"); //debug only
            bDebugBadHint=false;
        }
    }
    return sLine;
}//end getConvertedSourceErrorAndWarnElseGetUnmodified

std::map<QString, QString>* MainWindow::getOutputLineInfo(QString sLine, QString actualJump, bool isPrevCallPrevLine) {
    std::map<QString, QString>* info = new std::map<QString, QString>();
    getOutputLineInfo(info, sLine, actualJump, isPrevCallPrevLine);
    return info;
}

void MainWindow::getOutputLineInfo(std::map<QString, QString>* info, QString sLine, const QString actualJump, bool isPrevCallPrevLine) {
    (*info)["file"] = "";  // same as info->at("file")
    (*info)["line"] = "";
    (*info)["column"] = "";
    (*info)["language"] = "";  // only if language can be detected from this line
    (*info)["good"]="false";
    // (*info)["iOpener"] = "-1";
    int iOpenerLen=1;
    // (*info)["iOpenerLen"] = QString::number(iOpenerLen);
    // (*info)["iCloser"] = "-1";
    // (*info)["iParamDelim"] = "-1";
    int iFileFlagLen=JUMP_FLAG.length();
    // (*info)["iFileFlagLen"] = QString::number(iFileFlagLen);

    sLine=getConvertedSourceErrorAndWarnElseGetUnmodified(sLine);

    int iOpener=sLine.indexOf("(",0,Qt::CaseSensitive);
    int iCloser=sLine.indexOf(")",0,Qt::CaseSensitive);
    int iParamDelim=sLine.indexOf(",",0,Qt::CaseSensitive);
    QStringList chunks=sLine.split(":");
    QString sFoundFileFlag;
    QString sFoundParamOpener;
    QString sFoundParamDelim;
    QString sFoundCloser;
    int iFoundFileFlag = -1;
    int iFoundParamOpener = -1;
    int iFoundParamDelim = -1;
    int iFoundCloser = -1;

    // qInfo().noquote() << "Checking enclosures:";
    // for (auto const& it : enclosures) {  // >=C++11 only (use dot notation not -> below if using this)
    std::list<QStringList>::iterator itList;
    for ( itList = enclosures.begin(); itList != enclosures.end(); itList++ ) {
//        qInfo().noquote() << "  -";
//        for (int i=0; i<(*itList).length(); i++) {
//            qInfo().noquote() << "    - " << (*itList)[i];
//        }
        //if (((*itList)[0]).length()>0) {  // if has identifiable (non-blank) opener
            if (sLine.startsWith((*itList)[0]) || (((*itList)[0]).length()==0)) {
                sFoundFileFlag = (*itList)[0];
                sFoundParamOpener = (*itList)[1];
                sFoundParamDelim = (*itList)[2];  // coordinate delimiter (blank if no column)
                sFoundCloser = (*itList)[3];  // what is after last coord ("\n" if line ends)
                if (sFoundFileFlag.length()!=0) iFoundFileFlag=sLine.indexOf(sFoundFileFlag);
                else iFoundFileFlag=0;  // if file path at begining of line
                if (iFoundFileFlag>-1) {
                    iFoundParamOpener=sLine.indexOf(sFoundParamOpener, iFoundFileFlag+sFoundFileFlag.length());
                    if (iFoundParamOpener>-1) {
                        if (sFoundParamDelim.length()>0) iFoundParamDelim=sLine.indexOf(sFoundParamDelim, iFoundParamOpener+sFoundParamOpener.length());
                        else iFoundParamDelim=sLine.length();
                        if (iFoundParamDelim>-1) {
                            if (sFoundCloser!="\n") iFoundCloser=sLine.indexOf(sFoundCloser, iFoundParamDelim+sFoundParamDelim.length());
                            else iFoundCloser=sLine.length();
                        }
                    }
                }
            }
        //}
    }

    if (isPrevCallPrevLine) {
        if (actualJump.length()>0) {
            sLine=actualJump;
            // qInfo().noquote() << "actualJump: '" + actualJump + "'";
        }
    }
    QString sOpenerFlag="\", line ";
    if (startsWithJumpFlag(sLine)) {
        // qInfo().noquote() << "JUMP_FLAG in '" + sLine + "'";
        iOpener=-1;
        iOpenerLen=0;
        iCloser=-1;
        iParamDelim=0;
        int iOpenerFlag=sLine.indexOf(sOpenerFlag);
        if (iOpenerFlag>-1) {
            iOpener=iOpenerFlag+sOpenerFlag.length();
            // qInfo().noquote() << "  sLine[iFileFlagLen]: " << sLine[iFileFlagLen];
            if (sLine[iFileFlagLen]=='"') iFileFlagLen++;
            iParamDelim=sLine.indexOf(",",iOpener);
            // qInfo().noquote() << "  iOpenerFlag: " << iOpenerFlag;
            // qInfo().noquote() << "  iOpener: " << iOpener;
            // qInfo().noquote() << "  sFile: " << sLine.mid(JUMP_FLAG.length());
            // qInfo().noquote() << "  param: " << sLine.mid(iOpener);
            if (iParamDelim<0) iParamDelim=sLine.length(); // there is no column number in nosetests output
            // qInfo().noquote() << "  iParamDelim: " << iParamDelim;
            if (iParamDelim>-1) iCloser=iParamDelim;  // there is no column number in nosetests output
            else qWarning().noquote() << "missing iParamDelim ',' in '" + sLine + "'";
        }
        else qWarning().noquote() << "missing iOpenerFlag '" + sOpenerFlag + "' in '" + sLine + "'";
    }
    else {
        iFileFlagLen=0;
        if (chunks.length()>=4) {
            // check for pycodestyle output
            // such as "__init__.py:39:20: E116 unexpected indentation (comment)"
            QRegExp re("\\d*");  // a digit (\d), zero or more times (*)
            if (re.exactMatch(chunks[1]) && re.exactMatch(chunks[2])) {
                iOpener=sLine.indexOf(":");
                iParamDelim=sLine.indexOf(":", iOpener+1);
                iCloser=sLine.indexOf(":", iParamDelim+1);
            }
        }
    }

    if (iOpener>0 && iParamDelim>=iOpener && iCloser>=iParamDelim) { //normally >, but == if no column specified
        QString sFile=sLine.mid(iFileFlagLen,iOpener-iFileFlagLen);
        int iOpenerFlag=sFile.indexOf(sOpenerFlag);
        if (iOpenerFlag>=0) sFile=sFile.mid(0, iOpenerFlag);
        sFile=sFile.trimmed();

        (*info)["file"]=sFile;
        (*info)["line"]=sLine.mid(iOpener+iOpenerLen, iParamDelim-(iOpener+iOpenerLen));
        (*info)["column"]=sLine.mid(iParamDelim+1, iCloser-(iParamDelim+1));
        if (sFile.endsWith(".py",Qt::CaseSensitive)) (*info)["language"]="python";
        else if (sFile.endsWith(".cpp",Qt::CaseSensitive)) (*info)["language"]="c++";
        else if (sFile.endsWith(".h",Qt::CaseSensitive)) (*info)["language"]="c++";
        else if (sFile.endsWith(".hpp",Qt::CaseSensitive)) (*info)["language"]="c++";
        else if (sFile.endsWith(".c",Qt::CaseSensitive)) (*info)["language"]="c";
        else if (sFile.endsWith(".js",Qt::CaseSensitive)) (*info)["language"]="js";
        else if (sFile.endsWith(".java",Qt::CaseSensitive)) (*info)["language"]="java";
        else if (sFile.endsWith(".bat",Qt::CaseSensitive)) (*info)["language"]="bat";
        else if (sFile.endsWith(".sh",Qt::CaseSensitive)) (*info)["language"]="sh";
        else if (sFile.endsWith(".command",Qt::CaseSensitive)) (*info)["language"]="sh";
        else if (sFile.endsWith(".php",Qt::CaseSensitive)) (*info)["language"]="php";
        qInfo().noquote() << "  file: '" + sFile + "'";
    }
    (*info)["good"] = (iOpener>0 && iParamDelim>=iOpener && iCloser>=iParamDelim) ? "true" : "false";
    if ((*info)["good"]=="true") {
        if (actualJump.length()>0 && !startsWithJumpFlag(sLine)) {
            // nosetests output was detected, but the line is not in that format, so get rid of it (must be a sample line of code or something).
            (*info)["good"] = "false";
            (*info)["file"] = "";
        }
    }
    // (*info)["iOpener"] = QString::number(iOpener);
    // (*info)["iOpenerLen"] = QString::number(iOpenerLen);
    // (*info)["iParamDelim"] = QString::number(iParamDelim);
    // (*info)["iCloser"] = QString::number(iCloser);
    // (*info)["iFileFlagLen"] = QString::number(iFileFlagLen);
}

void MainWindow::addItemFromLine(QString sLine, QString& actualJump, QString& actualJumpLine)
{
    QListWidgetItem* lwiNew = new QListWidgetItem(sLine,ui->mainListWidget);
    QString sLastGoodLine=sLine;
    if (actualJumpLine.length()>0) sLastGoodLine=actualJumpLine;
    std::map<QString, QString>* info=new std::map<QString, QString>;

    if (startsWithJumpFlag(sLine)) {
        getOutputLineInfo(info, sLastGoodLine, "", true);
        actualJump = info->at("file");
        actualJumpLine = sLine;
        qInfo().noquote() << "set actualJump to " << actualJump;
    }
    else {
        getOutputLineInfo(info, sLastGoodLine, "", true);
        if (actualJump.length()>0) {
            //lwiNew->setToolTip(actualJump);
            QVariant vJump(actualJump);
            QVariant vJumpLine(actualJumpLine);
            lwiNew->setData(JumpRole, vJump);
            lwiNew->setData(JumpLineRole, vJumpLine);
            //qInfo().noquote() << "set toolTip to '" + lwiNew->toolTip() + "'";
            qInfo().noquote() << "set JumpRole to '" + lwiNew->data(JumpRole).toString() + "'";
            qInfo().noquote() << "set JumpLineRole to '" + lwiNew->data(JumpLineRole).toString() + "'";
        }
    }
    if (info->at("good")!="true") lwiNew->setForeground(brushUnusable);
    if (actualJumpLine.length()>0) sLastGoodLine=actualJumpLine;  // must become current one if current one good
    if (sLastGoodLine.contains("/site-packages/")) {
        lwiNew->setToolTip("<NOT VALID: this is a system file--look for a line further down the list not in site-packages>");
        //qInfo().noquote() << "set tooltip to: <site-packages>'" + lwiNew->toolTip() + "'";
        lwiNew->setForeground(brushTracebackNotTop);
    }
    ui->mainListWidget->addItem(lwiNew);
    delete info;
    if (sLine.trimmed()=="^") {
        actualJump="";
        actualJumpLine="";
    }
}

bool MainWindow::startsWithJumpFlag(QString sLine)
{
    bool ret;
    std::list<QStringList>::iterator itList;
    for ( itList = enclosures.begin(); itList != enclosures.end(); itList++ ) {
        if ( (((*itList)[4])==COLLECT_REUSE) && sLine.startsWith((*itList)[0]) ) {
            ret = true;
            break;
        }
    }
    return ret;
}

void MainWindow::on_mainListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QString sLine=item->text();
    QString actualJump=item->data(JumpRole).toString();  // item->toolTip();
    QString actualJumpLine=item->data(JumpLineRole).toString();  // item->toolTip();
    if (actualJumpLine.length()>0) sLine=actualJumpLine;
    std::map<QString, QString>* info=getOutputLineInfo(sLine, actualJump, false);
    // qWarning().noquote() << "info: " << (*info);
    bool bTest=false;
    // int iOpener=(*info)["iOpener"].toInt(&bTest,10);
    // int iParamDelim=(*info)["iParamDelim"].toInt(&bTest,10);
    // int iCloser=(*info)["iCloser"].toInt(&bTest,10);
    QString sFile=(*info)["file"];
    if ((*info)["good"]=="true") {
        qInfo().noquote() << "clicked_file: '" + sFile + "'";
        qInfo().noquote() << "tooltip: '" + item->toolTip() + "'";
        QString sCwd=QDir::currentPath(); //current() returns a QDir object
        QString sSlash="/";
        QString sFileAbs=sFile.startsWith(sSlash,Qt::CaseInsensitive)?sFile:(sCwd+"/"+sFile);
        QString sLineTarget=(*info)["line"];
        QString sColTarget=(*info)["column"];
        int iLineTarget=sLineTarget.toInt(&bTest,10);
        int iColTarget=sColTarget.toInt(&bTest,10);

        // region only for Kate <= 2
        iLineTarget+=yEditorOffset;
        sLineTarget.setNum(iLineTarget,10);
        iColTarget+=xEditorOffset;
        sColTarget.setNum(iColTarget,10);
        // endregion only for Kate <= 2

        if (bCompensateForKateTabDifferences) {
            QFile qfileSource(sFileAbs);
            QString sLine;
            //bool bFoundKateCmd=false;
            int iParseSourceLine=0;
            if ( qfileSource.open(QFile::ReadOnly) ) { //| QFile::Translate
                QTextStream qtextNow( &qfileSource );
                while ( !qtextNow.atEnd() ) {
                    sLine=qtextNow.readLine(0); //does trim off newline characters
                    if (iParseSourceLine==((iLineTarget-yEditorOffset)-1)) {
                        int iCountTabs=0;
                        for (int iNow=0; iNow<sLine.length(); iNow++) {
                            if (sLine.mid(iNow,1)=="\t") iCountTabs++;
                            else break;
                        }
                        QString sDebug;
                        if (iCountTabs>0) {
                            sDebug.setNum(iCountTabs,10);
                            sDebug="tabs:"+sDebug;
                            //if subtracted 1 for kate 2, the 1st character after a line with 1 tab is currently iColTarget==6,  otherwise it is 7
                            //if subtracted 1 for kate 2, the 2nd character after a line with 1 tab is currently iColTarget==7,  otherwise it is 8
                            //if subtracted 1 for kate 2, the 1st character after a line with 2tabs is currently iColTarget==12, otherwise it is 13
                            //if subtracted 1 for kate 2, the 2nd character after a line with 2tabs is currently iColTarget==13, otherwise it is 14
                            if (iKateRevisionMajor<3) iColTarget-=xEditorOffset;
                            sDebug+="; sColTarget-old:"+sColTarget;
                            iColTarget-=iCountTabs*(iCompilerTabWidth-1);
                            //iColTarget+=xEditorOffset;
                            sColTarget.setNum(iColTarget,10);
                            sDebug+="; iColTarget-abs:"+sColTarget;
                            //if above worked, then iColTarget is now an absolute character (counting tabs as 1 character)
                            //if subtracted 1 for kate 2, the 1st character after a line with 1 tab has now become iColTarget==1,  otherwise it is 2 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            //if subtracted 1 for kate 2, the 2nd character after a line with 1 tab has now become iColTarget==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 5 was subtracted [==(1*(6-1))]
                            //if subtracted 1 for kate 2, the 1st character after a line with 2tabs has now become iColTarget==2,  otherwise it is 3 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            //if subtracted 1 for kate 2, the 2nd character after a line with 2tabs has now become iColTarget==3,  otherwise it is 4 (when using compiler tabwidth of 6 and 10 was subtracted [==(1*(6-1))]
                            if (iKateRevisionMajor<3) {
                                //Kate 2.5.9 reads a 'c' argument value of 0 as the beginning of the line and 1 as the first character after the leading tabs
                                if (iColTarget<iCountTabs) iColTarget=0;
                                else {
                                    //iColTarget currently starts at 1 at the beginning of the line
                                    iColTarget-=(iCountTabs);
                                    sColTarget.setNum(iColTarget,10);
                                    sDebug+="; iColTarget-StartAt1-rel-to-nontab:"+sColTarget;
                                    //iColTarget now starts at 1 starting from the first text after tabs
                                    int iRegen=1;
                                    sDebug+="; skips:";
                                    //int iTotalSkip=0;
                                    //for (int iNow=1; iNow<(iKate2TabWidth*2+1)&&iNow<iColTarget; iNow+=iKate2TabWidth) {

                                    //}

                                    //This approximates how Kate 2 traverses tabs (the 'c' argument actually can't reach certain positions directly after the tabs):
                                    if (iCountTabs>2) iColTarget+=iCountTabs-2;
                                    for (int iNow=1; iNow<iColTarget; iNow++) {
                                        if (iNow<=(iCountTabs-1)*iKate2TabWidth+1) {
                                            if (iNow!=1&&(iNow-1)%iKate2TabWidth==0) {
                                                iRegen++; //only add if it is 4,7,10,etc where adder is iKate2TabWidth (1+iKate2TabWidth*position)
                                                sDebug+="-";
                                            }
                                        }
                                        else {
                                            iRegen++;
                                        }
                                    }
                                    iColTarget=iRegen;//+( (iCountTabs>3&&iCountTabs<6) ? iCountTabs : 0 );
                                    //end accounting for kate gibberish column translation

                                }
                            }
                            //else kate 3+, which handles tabs as absolute positions
                            sColTarget.setNum(iColTarget,10);
                            sDebug+="; sColTarget-new:"+sColTarget;
                            if (bDebugTabs) QMessageBox::information(this,"Output Inspector - Debug tab compensation",sDebug);
                        }//end if iCountTabs>0
                        break;
                    }//if correct line found
                    iParseSourceLine++;
                }//while not at end of source file
                qfileSource.close();
            }//end if can open source file
            else {
                QMessageBox::information(this,"Output Inspector - Parsing",sFileAbs+" cannot be accessed. Try running from location of err.txt");
            }
        }//end if bCompensateForKateTabDifferences
        //QString sArgs="-u "+sFileAbs+" -l "+sLineTarget+" -c "+sColTarget;
        //QProcess qprocNow(sKateCmd+sArgs);
        //qprocNow
        sDebug=sKateCmd;
        QStringList qslistArgs;
        //NOTE: -u is not needed at least as of kate 16.12.3 which does not create additional instances of kate
        //qslistArgs.append("-u");
        //sDebug+=" -u";
        //qslistArgs.append("\""+sFileAbs+"\"");
        qslistArgs.append(sFileAbs);
        sDebug+=" "+sFileAbs;
        qslistArgs.append("--line");  // split into separate arg, otherwise geany complains it doesn't understand the arg "--line 1"
        qslistArgs.append(sLineTarget);
        sDebug+=" --line "+sLineTarget;
        //qslistArgs.append(sLineTarget);
        qslistArgs.append("--column");//NOTE: -c is column in kate, but alternate config dir in geany, so use --column
        qslistArgs.append(sColTarget);//NOTE: -c is column in kate, but alternate config dir in geany, so use --column
        sDebug+=" --column "+sColTarget;
        //qslistArgs.append(sColTarget);
        // qWarning().noquote() << "qslistArgs: " << qslistArgs;
        QProcess::startDetached(sKateCmd,qslistArgs);
        if (!QFile::exists(sKateCmd)) {
            QMessageBox::information(this,"Output Inspector - Configuration",sKateCmd+" cannot be accessed.  Try setting the value of kate in /etc/outputinspector.conf");
        }
        //if (bDebug)
        statusbarNow->showMessage(sDebug,0);
        //system(sCmd);//stdlib
        //QMessageBox::information(this,"test",sCmd);
    }//end if line is in proper format
    else {
        QString msg = "Could not detect line number: ";
        // iOpener>0 && iParamDelim>iOpener && iCloser>iParamDelim
        // QMessageBox::information(this,"Output Inspector",msg);
        qWarning().noquote() << "Could not detect error format in" << "'" + sLine + "':";
        qWarning().noquote() << "  data: " + item->data(Qt::UserRole).toString();
        // for (auto const& it : (*info)) {  // >=C++11 only (use dot notation not -> below if using this)
        std::map<QString, QString>::iterator it;
        for ( it = info->begin(); it != info->end(); it++ ) {
            qWarning().noquote() << "  " << it->first  // key
                                 << ':' << it->second; //value
        }
        // printf("%s", msg.toStdString().c_str());
    }
}//end MainWindow::on_mainListWidget_itemDoubleClicked
