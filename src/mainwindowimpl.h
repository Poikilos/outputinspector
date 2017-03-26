#ifndef MAINWINDOWIMPL_H
#define MAINWINDOWIMPL_H
//
#include <QMainWindow>
#include "ui_mainwindow.h"
//

class MainWindowImpl : public QMainWindow, public Ui::MainWindow
{
Q_OBJECT
public:
	MainWindowImpl( QWidget * parent = 0, Qt::WFlags f = 0 );
	void FixSize();
	void init();
	void readini();
	void CompensateForEditorVersion();
private slots:
	void resizeEvent(QResizeEvent *event);
};
#endif




