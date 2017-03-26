SOURCES	+= main.cpp 
HEADERS	+= contact.h 
unix {
  UI_DIR = .ui
  MOC_DIR = .moc
  OBJECTS_DIR = .obj
}
FORMS	= frmmain.ui dlgcontact.ui 
TEMPLATE	=app
CONFIG	+= qt warn_on release
DBFILE	= adbook.db
LANGUAGE	= C++
