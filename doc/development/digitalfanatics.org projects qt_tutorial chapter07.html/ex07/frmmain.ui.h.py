'''***************************************************************************
** ui.h extension file, from the uic-generated form implementation.
**
** If you wish to add, or rename slots use Qt Designer which will
** update self file, your code. Create an init() slot in place of
** a constructor, a destroy() slot in place of a destructor.
****************************************************************************'''


def addContact(self):
    # Create a dialog
    dlgContact dlg( self )
    
    # Show it and wait for Ok or Cancel
    if  dlg.exec() == QDialog.Accepted :
	# We got Ok
	Contact c
	
	# Extract the information from the dialog
	c.name = dlg.leName.text()
	c.eMail = dlg.leEMail.text()
	c.phone = dlg.lePhone.text()
	
	# Check for duplicate names
	for( QValueList<Contact>it = m_contacts.begin(); it != m_contacts.end(); ++it )
	    if  (*it).name == c.name :
		# Warn for duplicates and fail
		QMessageBox.warning( self, tr("Duplicate"), tr("The person ") + (*it).name + tr(" allready exists.") )
		return


	# No duplicates found, the contact to the list and show it in the listview
	m_contacts.append( c )
	lvContacts.insertItem( QListViewItem( lvContacts, c.name , c.eMail, c.phone ) )



def editContact(self):
    # Get a pointer to the selected item
    QListViewItem *item = lvContacts.currentItem()
    
    # Check if there is a valid select item
    if  item :
	# Find the corresponding item in the internal list
	for( QValueList<Contact>it = m_contacts.begin(); it != m_contacts.end(); ++it )
	    if  (*it).name == item.text(0) :
		# We have found it, the dialog with it
		dlgContact dlg( self )
		
		dlg.leName.setText( (*it).name )
		dlg.leEMail.setText( (*it).eMail )
		dlg.lePhone.setText( (*it).phone )
		
		# Show it and wait for Ok or Cancel
		if  dlg.exec() == QDialog.Accepted :
		    # Check for duplicate names by counting the number of occurencies
		    occurencies = 0
		    for( QValueList<Contact>i2 = m_contacts.begin(); i2 != m_contacts.end(); ++i2 )		   
			if  (*i2).name == dlg.leName.text() :
			    occurencies++
		 
		    # If the name is changed we allow no occurencies, one 
	 	    if  (dlg.leName.text() != (*it).name and occurencies > 0) or (occurencies > 1) :
			# Warn for duplicates and fail
			QMessageBox.warning( self, tr("Duplicate"), tr("The person ") + dlg.leName.text() + tr(" allready exists.") )
			return

		    
		    # Update the internal list
		    (*it).name = dlg.leName.text()
		    (*it).eMail = dlg.leEMail.text()
		    (*it).phone = dlg.lePhone.text()
		    
		    # Update the list view
		    item.setText( 0, (*it).name )
		    item.setText( 1, (*it).eMail )
		    item.setText( 2, (*it).phone )


		return





def init(self):
    # Clear the list
    lvContacts.clear()


def removeContact(self):
    # Get a pointer to the selected item
    QListViewItem *item = lvContacts.currentItem()
    
    # Check if there is a valid select item
    if  item :
	# Find the corresponding item in the internal list
	for( QValueList<Contact>it = m_contacts.begin(); it != m_contacts.end(); ++it )
	    if  (*it).name == item.text(0) :
		# We have found it, it from the list and listview
		m_contacts.remove( it )
		delete item
		return




