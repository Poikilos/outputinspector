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
bool bShowWarningsLast=true;
int iKateRevisionMajor=0;//i.e. 2.5.9 is 2, kde3 version; and 3.0.3 is 3, the kde4 version
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
    init();
    statusbarNow=ui->statusBar;
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::init() {
    readini();
    if (!bForceOffset) CompensateForEditorVersion();

    QFile qfileTest("err.txt");
    QString sLine;
    QString sError="Error";
    QString sWarning="Warning";
    QString sToDo="//TODO:";
    //ui->mainListWidget is a QListWidget
    //setCentralWidget(ui->mainListWidget);
    //ui->mainListWidget->setSizePolicy(QSizePolicy::)
    if ( qfileTest.open(QFile::ReadOnly) ) { //| QFile::Translate
        QTextStream qtextNow( &qfileTest );
        while ( !qtextNow.atEnd() ) {
            sLine=qtextNow.readLine(0); //does trim off newline characters

            if (!bShowWarningsLast) ui->mainListWidget->addItem(new QListWidgetItem(sLine,ui->mainListWidget));
            if (sLine.indexOf(sError,0,Qt::CaseInsensitive)>-1) {
                if (sLine.indexOf("previous error",0,Qt::CaseInsensitive)<0) iErrors++;
                if (bShowWarningsLast) stringsErrors.append(sLine);
            }
            else if (sLine.indexOf(sWarning,0,Qt::CaseInsensitive)>-1) {
                iWarnings++;
                if (bShowWarningsLast) stringsWarnings.append(sLine);
            }
            if (bFindTODOs && sLine.indexOf("(")>-1) {
                QString sFileX=sLine.mid(0,sLine.indexOf("("));
                if (!qslistFiles.contains(sFileX, Qt::CaseSensitive)) {
                    qslistFiles.append(sFileX);
                    QFile qfileSource(sFileX);
                    if (qfileSource.open(QFile::ReadOnly)) {
                        QTextStream qtextSource( &qfileSource );
                        int iSourceLineFindToDo=0;
                        while ( !qtextSource.atEnd() ) {
                            QString sSourceLine=qtextSource.readLine(0);
                            iSourceLineFindToDo++;//add first since compiler line numbering starts at 1
                            int iColTODOFound=sSourceLine.indexOf(sToDo,0,Qt::CaseInsensitive);
                            if (iColTODOFound>-1) {
                                QString sNumLine;
                                sNumLine.setNum(iSourceLineFindToDo,10);
                                QString sNumPos;
                                int iCookedStart=iColTODOFound;
                                for (int iNow=0; iNow<sSourceLine.length(); iNow++) {
                                    if (sSourceLine.mid(iNow,1)=="\t") iCookedStart+=(iCompilerTabWidth-1);
                                    else break;
                                }
                                iCookedStart+=1;//start numbering at 1 to mimic compiler
                                iCookedStart+=2;//+2 to start after slashes
                                sNumPos.setNum(iCookedStart,10);
                                qslistTODOs.append(sFileX+"("+sNumLine+","+sNumPos+") "+sSourceLine.mid(iColTODOFound+2));
                                iTODOs++;
                            }
                        }//end while not at end of source file
                        qDebug() << "outputinspector finished reading sourcecode";
                        qDebug() << "(processed " << iSourceLineFindToDo << " line(s))";
                        qfileSource.close();
                    }//end if could open sourcecode
                    else {
                        qDebug() << "outputinspector could not open source file";
                    }
                }//end if list does not already contain this file
            }//if bFindTODOs and found filename ender
        }//end while not at end of err.txt
        qfileTest.close();
        QString sNumErrors;
        sNumErrors.setNum(iErrors,10);
        QString sNumWarnings;
        sNumWarnings.setNum(iWarnings,10);
        QString sNumTODOs;
        sNumTODOs.setNum(iTODOs,10);
        if (bShowWarningsLast) {
            while(!stringsErrors.isEmpty()) {
                ui->mainListWidget->addItem(new QListWidgetItem(stringsErrors.takeFirst(),ui->mainListWidget));
            }
            while(!stringsWarnings.isEmpty()) {
                ui->mainListWidget->addItem(new QListWidgetItem(stringsWarnings.takeFirst(),ui->mainListWidget));
            }
        }
        if (bFindTODOs) {
            while(!qslistTODOs.isEmpty()) {
                ui->mainListWidget->addItem(new QListWidgetItem(qslistTODOs.takeFirst(),ui->mainListWidget));
            }
        }
        QString sMsg="Errors: "+sNumErrors+"; Warnings:"+sNumWarnings;
        if (bFindTODOs) sMsg+="; TODOs:"+sNumTODOs;
        ui->statusBar->showMessage(sMsg,0);
    }//end if could open err.txt
    else {
        QMessageBox::information(this,"Output Inspector - Help","Output Inspector cannot find the output file to process.  File must be \"./err.txt\"");
    }

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
        QMessageBox::information(this,"Output Inspector - Configuration","/etc/outputinspector.conf has no line reading \"kate=/usr/bin/kate\" so reverting to /usr/lib/kde4/bin/kate");
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
    bool bFoundKateCmd=false;
    if ( qfileTmp.open(QFile::ReadOnly) ) { //| QFile::Translate
        QTextStream qtextNow( &qfileTmp );
        QString sKateOpener="Kate: ";
        while ( !qtextNow.atEnd() ) {
            sLine=qtextNow.readLine(0); //does trim off newline characters
            if (bDebug) QMessageBox::information(this,"Finding Kate version...",sLine);
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
    if (bDebug) QMessageBox::information(this,"Kate Version",bFound?("Detected Kate "+sRevisionMajor):"Could not detect Kate version.");
    if (iKateRevisionMajor>2) {
        xEditorOffset=0;
        yEditorOffset=0;
    }
    else {
        xEditorOffset=-1;
        yEditorOffset=-1;
    }
}//end CompensateForEditorVersion
