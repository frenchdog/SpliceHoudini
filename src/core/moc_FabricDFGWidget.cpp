/****************************************************************************
** Meta object code from reading C++ file 'FabricDFGWidget.h'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "FabricDFGWidget.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'FabricDFGWidget.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_OpenSpliceHoudini__FabricDFGWidget[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      35,   53,   53,   53, 0x0a,
      54,   85,   53,   53, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_OpenSpliceHoudini__FabricDFGWidget[] = {
    "OpenSpliceHoudini::FabricDFGWidget\0"
    "onRecompilation()\0\0onPortRenamed(QString,QString)\0"
    "path,newName\0"
};

void OpenSpliceHoudini::FabricDFGWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        FabricDFGWidget *_t = static_cast<FabricDFGWidget *>(_o);
        switch (_id) {
        case 0: _t->onRecompilation(); break;
        case 1: _t->onPortRenamed((*reinterpret_cast< QString(*)>(_a[1])),(*reinterpret_cast< QString(*)>(_a[2]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData OpenSpliceHoudini::FabricDFGWidget::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject OpenSpliceHoudini::FabricDFGWidget::staticMetaObject = {
    { &DFG::DFGCombinedWidget::staticMetaObject, qt_meta_stringdata_OpenSpliceHoudini__FabricDFGWidget,
      qt_meta_data_OpenSpliceHoudini__FabricDFGWidget, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &OpenSpliceHoudini::FabricDFGWidget::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *OpenSpliceHoudini::FabricDFGWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *OpenSpliceHoudini::FabricDFGWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_OpenSpliceHoudini__FabricDFGWidget))
        return static_cast<void*>(const_cast< FabricDFGWidget*>(this));
    typedef DFG::DFGCombinedWidget QMocSuperClass;
    return QMocSuperClass::qt_metacast(_clname);
}

int OpenSpliceHoudini::FabricDFGWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    typedef DFG::DFGCombinedWidget QMocSuperClass;
    _id = QMocSuperClass::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
