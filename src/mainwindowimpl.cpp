#include "mainwindowimpl.h"
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMessageBox>
#include <QString>
#include <QProcess>
#include <QThread>

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

MainWindowImpl::MainWindowImpl( QWidget * parent, Qt::WFlags f) 
	: QMainWindow(parent, f)
{
	setupUi(this);
	init();
	statusbarNow=statusbar;
}
void MainWindowImpl::readini() {
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
void MainWindowImpl::CompensateForEditorVersion() {
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
void MainWindowImpl::init() {
	readini();
	if (!bForceOffset) CompensateForEditorVersion();
	
	QFile qfileTest("err.txt");
	QString sLine;
	FixSize();
	QString sError="Error";
	QString sWarning="Warning";
	QString sToDo="//TODO:";
	//listMain is a QListWidget
	//setCentralWidget(listMain);
	//listMain->setSizePolicy(QSizePolicy::)
	if ( qfileTest.open(QFile::ReadOnly) ) { //| QFile::Translate  
		QTextStream qtextNow( &qfileTest );
		while ( !qtextNow.atEnd() ) {
			sLine=qtextNow.readLine(0); //does trim off newline characters
			if (!bShowWarningsLast) listMain->addItem(new QListWidgetItem(sLine,listMain));
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
						qfileSource.close();
					}//end if could open source file
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
				listMain->addItem(new QListWidgetItem(stringsErrors.takeFirst(),listMain));
			}
			while(!stringsWarnings.isEmpty()) {
				listMain->addItem(new QListWidgetItem(stringsWarnings.takeFirst(),listMain));
			}
		}
		if (bFindTODOs) {
			while(!qslistTODOs.isEmpty()) {
				listMain->addItem(new QListWidgetItem(qslistTODOs.takeFirst(),listMain));
			}
		}
		QString sMsg="Errors: "+sNumErrors+"; Warnings:"+sNumWarnings;
		if (bFindTODOs) sMsg+="; TODOs:"+sNumTODOs;
		statusbar->showMessage(sMsg,0);
	}//end if could open err.txt
	else {
		QMessageBox::information(this,"Output Inspector - Help","Output Inspector cannot find the output file to process.  File must be \"./err.txt\"");
	}
	
}//end init

void QListWidget::itemDoubleClicked ( QListWidgetItem * item ) {
	QString sCwd=QDir::currentPath(); //current() returns a QDir object
	QString sLine=item->text();
	QString sSlash="/";
	int iOpener=sLine.indexOf("(",0,Qt::CaseSensitive);
	int iCloser=sLine.indexOf(")",0,Qt::CaseSensitive);
	int iComma=sLine.indexOf(",",0,Qt::CaseSensitive);
	if (iOpener>0 && iComma>iOpener && iCloser>iComma) {
		QString sFile=sLine.mid(0,iOpener);
		QString sFileAbs=sFile.startsWith(sSlash,Qt::CaseInsensitive)?sFile:(sCwd+"/"+sFile);
		QString sLineTarget=sLine.mid(iOpener+1, iComma-(iOpener+1));
		QString sColTarget=sLine.mid(iComma+1, iCloser-(iComma+1));
		bool bTest=false;
		int iLineTarget=sLineTarget.toInt(&bTest,10);
		int iColTarget=sColTarget.toInt(&bTest,10);
		iLineTarget+=yEditorOffset;
		sLineTarget.setNum(iLineTarget,10);
		iColTarget+=xEditorOffset;
		sColTarget.setNum(iColTarget,10);
		if (bCompensateForKateTabDifferences) {
			QFile qfileSource(sFileAbs);
			QString sLine;
			bool bFoundKateCmd=false;
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
		}//end if bCompensateForKateTabDifferences
		//QString sArgs="-u "+sFileAbs+" -l "+sLineTarget+" -c "+sColTarget;
		//QProcess qprocNow(sKateCmd+sArgs);
		//qprocNow
		sDebug=sKateCmd;
		QStringList qslistArgs;
		qslistArgs.append("-u");
		sDebug+=" -u";
		//qslistArgs.append("\""+sFileAbs+"\"");
		qslistArgs.append(sFileAbs);
		sDebug+=" "+sFileAbs;
		qslistArgs.append("-l "+sLineTarget);
		sDebug+=" -l "+sLineTarget;
		//qslistArgs.append(sLineTarget);
		qslistArgs.append("-c "+sColTarget);
		sDebug+=" -c "+sColTarget;
		//qslistArgs.append(sColTarget);
		QProcess::startDetached(sKateCmd,qslistArgs);
		if (!QFile::exists(sKateCmd)) {
			QMessageBox::information(this,"Output Inspector - Configuration",sKateCmd+" cannot be accessed.  Try setting the value of kate in /etc/outputinspector.conf");
		}
		//if (bDebug) 
		statusbarNow->showMessage(sDebug,0);
		//system(sCmd);//stdlib
		//QMessageBox::information(this,"test",sCmd);
	}//end if line is in proper format
}//end QListWidget::itemDoubleClicked

void MainWindowImpl::resizeEvent(QResizeEvent *event) {
	FixSize();
}

void MainWindowImpl::FixSize() {
	listMain->move(0,0);
	//int iLeft, iTop, iRight, iBottom;
	//this->getContentsMargins(&iLeft,&iTop,&iRight,&iBottom);
	//QRect sizeThis=this->rect();
	QSize sizeThis=this->frameSize();
	listMain->resize(width(),sizeThis.height()-(statusbar->height()+6));
}