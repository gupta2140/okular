/**********************************************************************

	--- Dlgedit generated file ---

	File: printData.cpp
	Last generated: Fri Oct 3 00:46:41 1997

	DO NOT EDIT!!!  This file will be automatically
	regenerated by dlgedit.  All changes will be lost.

 *********************************************************************/

#include "printData.h"

#define Inherited QDialog

#include <qbuttongroup.h>
#include <qlabel.h>
#include <kapp.h>
#include <klocale.h>

printData::printData
(
	QWidget* parent,
	const char* name
)
	:
	Inherited( parent, name, TRUE )
{
	fileNameLabel = new QLabel( this, "Label_1" );
	fileNameLabel->setGeometry( 18, 70, 91, 30 );
	fileNameLabel->setText( i18n( "&File Name:" ) );
	fileNameLabel->setAlignment( 290 );
	fileNameLabel->setMargin( 8 );

	QLabel* tmpQLabel;
	tmpQLabel = new QLabel( this, "Label_2" );
	tmpQLabel->setGeometry( 300, 170, 20, 30 );
	tmpQLabel->setText( "-" );
	tmpQLabel->setAlignment( 292 );
	tmpQLabel->setMargin( -1 );

	printer = new QComboBox( TRUE, this, "ComboBox_1" );
	printer->setGeometry( 110, 30, 270, 30 );
	connect( printer, SIGNAL(activated(int)), SLOT(printDestinationChanged(int)) );
	printer->setInsertionPolicy( QComboBox::AtBottom );
	printer->setSizeLimit( 20 );
	printer->setAutoResize( FALSE );

	printFileName = new QLineEdit( this, "printFileName" );
	printFileName->setGeometry( 110, 70, 270, 30 );
	printFileName->setText( "" );
	printFileName->setMaxLength( 32767 );
	printFileName->setEchoMode( QLineEdit::Normal );
	printFileName->setFrame( TRUE );

	printAll = new QRadioButton( this, "RadioButton_1" );
	printAll->setGeometry( 30, 140, 100, 30 );
	printAll->setText( i18n( "&All" ) );
	printAll->setAutoRepeat( FALSE );
	printAll->setAutoResize( FALSE );
	printAll->setChecked( TRUE );

	printCurrent = new QRadioButton( this, "RadioButton_2" );
	printCurrent->setGeometry( 150, 140, 100, 30 );
	printCurrent->setText( i18n( "&Current" ) );
	printCurrent->setAutoRepeat( FALSE );
	printCurrent->setAutoResize( FALSE );

	printMarked = new QRadioButton( this, "RadioButton_3" );
	printMarked->setGeometry( 30, 170, 100, 30 );
	printMarked->setText( i18n( "&Marked" ) );
	printMarked->setAutoRepeat( FALSE );
	printMarked->setAutoResize( FALSE );

	printRange = new QRadioButton( this, "RadioButton_4" );
	printRange->setGeometry( 150, 170, 100, 30 );
	connect( printRange, SIGNAL(toggled(bool)), SLOT(rangeToggled(bool)) );
	printRange->setText( i18n( "&Range" ) );
	printRange->setAutoRepeat( FALSE );
	printRange->setAutoResize( FALSE );

	rangeFrom = new QLineEdit( this, "LineEdit_2" );
	rangeFrom->setGeometry( 250, 170, 50, 30 );
	rangeFrom->setText( "" );
	rangeFrom->setMaxLength( 32767 );
	rangeFrom->setEchoMode( QLineEdit::Normal );
	rangeFrom->setFrame( TRUE );

	rangeTo = new QLineEdit( this, "LineEdit_3" );
	rangeTo->setGeometry( 320, 170, 50, 30 );
	rangeTo->setText( "" );
	rangeTo->setMaxLength( 32767 );
	rangeTo->setEchoMode( QLineEdit::Normal );
	rangeTo->setFrame( TRUE );

	printReverse = new QCheckBox( this, "CheckBox_1" );
	printReverse->setGeometry( 30, 240, 90, 30 );
	printReverse->setText( i18n( "Re&verse" ) );
	printReverse->setAutoRepeat( FALSE );
	printReverse->setAutoResize( FALSE );

	nupCombo = new QComboBox( FALSE, this, "ComboBox_2" );
	nupCombo->setGeometry( 120, 240, 140, 30 );
	connect( nupCombo, SIGNAL(activated(int)), SLOT(nupPressed(int)) );
	nupCombo->setSizeLimit( 10 );
	nupCombo->setAutoResize( FALSE );
	nupCombo->insertItem( i18n( "1 page on sheet" ) );
	nupCombo->insertItem( i18n( "2 pages on sheet" ) );
	nupCombo->insertItem( i18n( "4 pages on sheet" ) );

	colOrder = new QComboBox( FALSE, this, "ComboBox_3" );
	colOrder->setGeometry( 270, 240, 110, 30 );
	colOrder->setSizeLimit( 10 );
	colOrder->setAutoResize( FALSE );
	colOrder->insertItem( i18n( "Fill rows" ) );
	colOrder->insertItem( i18n( "Fill columns" ) );

	okButton = new QPushButton( this, "PushButton_1" );
	okButton->setGeometry( 30, 300, 100, 30 );
	connect( okButton, SIGNAL(clicked()), SLOT(okPressed()) );
	okButton->setText( i18n( "OK" ) );
	okButton->setAutoRepeat( FALSE );
	okButton->setAutoResize( FALSE );

	setupButton = new QPushButton( this, "PushButton_2" );
	setupButton->setGeometry( 150, 300, 100, 30 );
	connect( setupButton, SIGNAL(clicked()), SLOT(setupPressed()) );
	setupButton->setText( i18n( "&Setup" ) );
	setupButton->setAutoRepeat( FALSE );
	setupButton->setAutoResize( FALSE );

	cancelButton = new QPushButton( this, "PushButton_3" );
	cancelButton->setGeometry( 270, 300, 100, 30 );
	connect( cancelButton, SIGNAL(clicked()), SLOT(cancelPressed()) );
	cancelButton->setText( i18n( "Cancel" ) );
	cancelButton->setAutoRepeat( FALSE );
	cancelButton->setAutoResize( FALSE );

	QButtonGroup* tmpQButtonGroup;
	tmpQButtonGroup = new QButtonGroup( this, "ButtonGroup_3" );
	tmpQButtonGroup->setGeometry( 10, 220, 380, 60 );
	tmpQButtonGroup->setFrameStyle( 49 );
	tmpQButtonGroup->setTitle( i18n( "Page order" ) );
	tmpQButtonGroup->setAlignment( 1 );
	tmpQButtonGroup->insert( printReverse );
	tmpQButtonGroup->lower();

	tmpQButtonGroup = new QButtonGroup( this, "ButtonGroup_2" );
	tmpQButtonGroup->setGeometry( 10, 120, 380, 90 );
	tmpQButtonGroup->setFrameStyle( 49 );
	tmpQButtonGroup->setTitle( i18n( "Pages" ) );
	tmpQButtonGroup->setAlignment( 1 );
	tmpQButtonGroup->insert( printAll );
	tmpQButtonGroup->insert( printCurrent );
	tmpQButtonGroup->insert( printMarked );
	tmpQButtonGroup->insert( printRange );
	tmpQButtonGroup->lower();
	tmpQButtonGroup->setExclusive( TRUE );

	tmpQButtonGroup = new QButtonGroup( this, "printToGroup" );
	tmpQButtonGroup->setGeometry( 10, 10, 380, 100 );
	tmpQButtonGroup->setFrameStyle( 49 );
	tmpQButtonGroup->setTitle( i18n( "Print to" ) );
	tmpQButtonGroup->setAlignment( 1 );
	tmpQButtonGroup->lower();

	resize( 400, 350 );
}


printData::~printData()
{
}
void printData::printDestinationChanged(int)
{
}
void printData::rangeToggled(bool)
{
}
void printData::nupPressed(int)
{
}
void printData::okPressed()
{
}
void printData::setupPressed()
{
}
void printData::cancelPressed()
{
}

#undef Inherited
//#include "printData.moc.cpp"
