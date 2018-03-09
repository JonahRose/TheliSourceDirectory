/****************************************************************************
** $Id: qt/main.cpp   3.3.8   edited Jan 11 14:37 $
**
** Copyright (C) 1992-2007 Trolltech ASA.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#include "mainwindow.h"
#include "qfileiconview.h"

#include <qapplication.h>


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    FileMainWindow mw;
    mw.resize( 680, 480 );
    a.setMainWidget( &mw );
    mw.fileView()->setDirectory( "/" );
    mw.show();
    return a.exec();
}
