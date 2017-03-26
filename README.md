Output Inspector Readme


Purpose:
Allows you to double-click lines of your compiler output, telling kate to jump to the appropriate file, line and position in the code.


Extract:
* Requires gzip
	tar -xzvf outputinspector-<version>.tar.gz
	cd outputinspector-<version>

Compile First (if desired, but there is a binary built on Ubuntu Hardy):
-Build using qdevelop 
(Requires: qt4 libqt4-dev qdevelop)
qdevelop outputinspector.pro
-then push the F7 key.  When it is finished compiling, exit.

Install:
	(Requires: qt4 libqt4 libqt4-gui libqt4-core)
	-open a terminal
	-cd to the directory where you extracted outputinspector i.e. cd outputinspector-2008-10-08-qt4
	sudo ./install (OR: su root && ./install)
	-If you have never installed outputinspector before you must run:
	sudo ./install-conf (OR: su root && ./install)
	(this will reset to default settings if outputinspector was already installed)

Configuration:
-Remember to edit /etc/outputinspector.conf specifying the kate binary (i.e. include the line kate=/usr/lib/kde4/bin/kate or appropriate command).
-Tab handling:  If you are using mcs 1.2.6 or other compiler that reads tabs as 6 spaces, and you are using Kate 2 with the default tab width of 8 or are using Kate 3, you don't have to change anything.  Otherwise:
Set CompilerTabWidth and Kate2TabWidth in /etc/outputinspector.conf -- kate 3.0.0+ is handled automatically, but CompilerTabWidth may have to be set.  If the compiler treats tabs as 1 character, make sure you set CompilerTabWidth=1 -- as of 1.2.6, mcs counts tabs as 6 spaces (outputinspector default).

Usage:
 You have to run outputinspector from the location of the cs files you are compiling, and your compiler error output has to be redirected to err.txt.

 example:
	mcs AssemblyInfo.cs MainForm.cs 2>err.txt
	./outputinspector

 If these instructions have been followed, and your compiler errors are in err.txt in the same folder, specified with lines starting with:
	Filename.ext(row,col): error
 Then Output Inspector should work when you double-click on the error.
