/****************************************************************************
** FormDialog meta object code from reading C++ file 'main.h'
**
** Created: Thu Mar 8 23:05:01 2018
**      by: The Qt MOC ($Id: qt/moc_yacc.cpp   3.3.8   edited Feb 2 14:59 $)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#undef QT_NO_COMPAT
#include "../../main.h"
#include <qmetaobject.h>
#include <qapplication.h>

#include <private/qucomextra_p.h>
#if !defined(Q_MOC_OUTPUT_REVISION) || (Q_MOC_OUTPUT_REVISION != 26)
#error "This file was generated using the moc from 3.3.8. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

const char *FormDialog::className() const
{
    return "FormDialog";
}

QMetaObject *FormDialog::metaObj = 0;
static QMetaObjectCleanUp cleanUp_FormDialog( "FormDialog", &FormDialog::staticMetaObject );

#ifndef QT_NO_TRANSLATION
QString FormDialog::tr( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FormDialog", s, c, QApplication::DefaultCodec );
    else
	return QString::fromLatin1( s );
}
#ifndef QT_NO_TRANSLATION_UTF8
QString FormDialog::trUtf8( const char *s, const char *c )
{
    if ( qApp )
	return qApp->translate( "FormDialog", s, c, QApplication::UnicodeUTF8 );
    else
	return QString::fromUtf8( s );
}
#endif // QT_NO_TRANSLATION_UTF8

#endif // QT_NO_TRANSLATION

QMetaObject* FormDialog::staticMetaObject()
{
    if ( metaObj )
	return metaObj;
    QMetaObject* parentObject = QDialog::staticMetaObject();
    static const QUMethod slot_0 = {"save", 0, 0 };
    static const QMetaData slot_tbl[] = {
	{ "save()", &slot_0, QMetaData::Public }
    };
    metaObj = QMetaObject::new_metaobject(
	"FormDialog", parentObject,
	slot_tbl, 1,
	0, 0,
#ifndef QT_NO_PROPERTIES
	0, 0,
	0, 0,
#endif // QT_NO_PROPERTIES
	0, 0 );
    cleanUp_FormDialog.setMetaObject( metaObj );
    return metaObj;
}

void* FormDialog::qt_cast( const char* clname )
{
    if ( !qstrcmp( clname, "FormDialog" ) )
	return this;
    return QDialog::qt_cast( clname );
}

bool FormDialog::qt_invoke( int _id, QUObject* _o )
{
    switch ( _id - staticMetaObject()->slotOffset() ) {
    case 0: save(); break;
    default:
	return QDialog::qt_invoke( _id, _o );
    }
    return TRUE;
}

bool FormDialog::qt_emit( int _id, QUObject* _o )
{
    return QDialog::qt_emit(_id,_o);
}
#ifndef QT_NO_PROPERTIES

bool FormDialog::qt_property( int id, int f, QVariant* v)
{
    return QDialog::qt_property( id, f, v);
}

bool FormDialog::qt_static_property( QObject* , int , int , QVariant* ){ return FALSE; }
#endif // QT_NO_PROPERTIES
