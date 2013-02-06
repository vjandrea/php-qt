/*!
 * PHP-Qt - The PHP language bindings for Qt
 *
 * Copyright (C) 2006 - 2007
 * Thomas Moenicke <tm at php-qt.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "phpqt_internals.h"
#include "marshall_types.h"

#include <QtCore/QHash>
#include <QtCore/QMetaMethod>
#include <zend.h>
#include "smoke.h"
#include "php_qt.h"

extern zend_object_handlers php_qt_handler;
extern zend_class_entry* qstring_ce;

zend_class_entry* qobject_ce;
QHash<const void*, smokephp_object*> SmokeQtObjects;
QHash<zend_object_handle, smokephp_object*> obj_x_smokephp;

//#define MOC_DEBUG 1

/*!
 * implementation of qt_metacall
 */
int
PHPQt::metacall(smokephp_object* so, Smoke::StackItem* args, QMetaObject::Call _c, int _id, void **_a)
{
    const QMetaObject* d = so->meta();
    int offset = d->methodOffset();
//    int offset = d->methodCount();
   
    // call the C++ one
    if(_id < offset )
    {
    CPP:
	// find parent
	Smoke::Index parent = so->smoke()->inheritanceList[so->smoke()->classes[so->classId()].parents];
	// methodId of qt_metacall, TODO: should be cached!
	Smoke::Index nameId = so->smoke()->idMethodName("qt_metacall$$?");
	Smoke::Index method = so->smoke()->findMethod(so->classId(), nameId);
	if(method > 0)
	{
	    Smoke::Method &m = so->smoke()->methods[so->smoke()->methodMaps[method].method];
	    Smoke::ClassFn fn = so->smoke()->classes[m.classId].classFn;
	    // qt_metacall(QMetaObject::Call, int, void**)
	    Smoke::StackItem i[4];
	    i[1].s_enum = _c;	    
	    i[2].s_int = _id;	    
	    i[3].s_voidp = (void*) args[3].s_voidp;
	    (*fn)(m.method, so->mPtr(), i);
#ifdef MOC_DEBUG
	    cout << "+\tcall Qt slot " << so->ce_ptr()->name << "::" << d->method(_id).signature() << endl;
#endif
	    if((int)i[0].s_int < 0)
		return i[0].s_int;
	    else
		qDebug() << "maybe a php method?";
	} else 
	    // should never happen
	    php_error(E_ERROR, "Cannot find %s::qt_metacall() method\n", d->className());
    }
    
    if (_c != QMetaObject::InvokeMetaMethod) {
	return _id;
    }

    // is a PHP Slot
    if(d->method(_id).methodType() == QMetaMethod::Slot)
    {
        zval* zmem = (zval*) safe_emalloc( d->method(_id).parameterTypes().count() + 1, sizeof(zval), 0 );
	zval* tmp = (zval*) emalloc(sizeof(zval));

	InvokeSlot c(so->smoke(), _id, args, so->zval_ptr(), &zmem, &tmp, _id, d, _a);
	if(!c.callable(so->ce_ptr()))
	{
	    goto CPP;
	}	
#ifdef MOC_DEBUG
	    cout << "+\tcall PHP slot " << so->ce_ptr()->name << "::" << d->method(_id).signature() << endl;
#endif
	c.next();
	efree(zmem);

    // is a signal
    } else {
#ifdef MOC_DEBUG
	cout << "\tcall PHP signal " << d->method(_id).signature() << endl;
#endif
        void *_b[] = { 0, _a[1] };
        QMetaObject::activate((QObject*) so->ptr(), d, 0, _b);
    }
    return _id;
}

void
PHPQt::destroyHashtable(zend_rsrc_list_entry *rsrc TSRMLS_DC)
{
#ifdef DEBUG
	php_error(E_ERROR,"Hashtable destroyed. Shutdown PHP-Qt now.");
#endif
}

bool
PHPQt::methodExists(const zend_class_entry* ce_ptr, const char* methodname)
{

	if(ce_ptr == NULL){
	  php_error(E_ERROR,"methodExists fatal error: no class entry");
	}

	char* lcname = zend_str_tolower_dup(methodname, strlen(methodname));

	if(zend_hash_exists(const_cast<HashTable*>(&ce_ptr->function_table), lcname, strlen(methodname)+1)){
		return true;
	}

	efree(lcname);
	return false;

}


zval*
PHPQt::callPHPMethod(const zval* this_ptr, const char* methodName, const zend_uint param_count, zval** args)
{

    if(this_ptr == NULL){
	php_error(E_ERROR,"callmethod fatal error: object does not exists");
    }

    zval *function_name;
    MAKE_STD_ZVAL(function_name);
    ZVAL_STRING(function_name,const_cast<char*>(methodName),1);

    zval* retval;
    MAKE_STD_ZVAL(retval);

    if(call_user_function(EG(function_table),(zval**) &this_ptr,function_name,retval,param_count,args) == FAILURE){
    	smokephp_object* o = PHPQt::getSmokePHPObjectFromZval(this_ptr);
    	php_error(E_ERROR, "PHP-Qt could not call method %s::%s", o->ce_ptr()->name, methodName);
    }

    // make sure we return the right object
    if(PHPQt::SmokePHPObjectExists(retval)){
	smokephp_object* o = PHPQt::getSmokePHPObjectFromZval(retval);
	retval = const_cast<zval*>(o->zval_ptr());
    }

    return retval;
}

/*!
 *	creates metaObject data
 *  example: "QWidget\0\0value\0test(int)\0"
 *	@param	zval*				this_ptr	pointer of the zval
 *	@param	char*				classname	name of the class
 *	@param	const QMetaObject*	superdata	superdata
 *	@return	QMetaObject*
 */

bool
PHPQt::getMocData(zval* this_ptr, const char* classname, const QMetaObject* superdata, QString* meta_stringdata, uint* signature){

    /// readout the slots table
    zval **slotdata;
    zval *zslot;
    zslot = zend_read_property(Z_OBJCE_P(this_ptr),this_ptr,"slots",5,0);

    zval *zsignal;
    zsignal = zend_read_property(Z_OBJCE_P(this_ptr),this_ptr,"signals",7,0);

    if((zslot)->type==IS_ARRAY && (zsignal)->type==IS_ARRAY ) 
    {
        HashTable* slots_hash = HASH_OF(zslot);
        HashTable* signals_hash = HASH_OF(zsignal);

        char* assocKey;
        ulong numKey;
        int signaturecount = 2 + strlen(classname);

#ifdef MOC_DEBUG
	QString *qr = new QString();
	cout << "+== begin metaobject dump ==+\n";
	cout << "\t" << classname << "\n\t1 0 0 0 " << zend_hash_num_elements(slots_hash)+zend_hash_num_elements(signals_hash) << " 10 0 0 0 0" << endl << endl;
#endif

	/// write class signature
	signature[0] = 1;
	signature[4] = zend_hash_num_elements(slots_hash)+zend_hash_num_elements(signals_hash);
	signature[5] = 10;
	
	/// write classname
	meta_stringdata->append(classname);
	meta_stringdata->append(QChar::Null);
	meta_stringdata->append(QChar::Null);
	
	int i = 10;
	zend_hash_internal_pointer_reset(signals_hash);
	while(zend_hash_has_more_elements(signals_hash) == SUCCESS)
	{
	    /// read slot from hashtable
	    zend_hash_get_current_key(signals_hash,&assocKey,&numKey,0);
	    zend_hash_get_current_data(signals_hash,(void**)&slotdata);   
#ifdef MOC_DEBUG
	    qr->append(Z_STRVAL_PP(slotdata));
	    qr->append(" ");
	    cout << "\t" << signaturecount << "8 8 8 0x05 ::s" << endl;
#endif

	    meta_stringdata->append(Z_STRVAL_PP(slotdata));
	    meta_stringdata->append(QChar::Null);
	    
	    zend_hash_move_forward(signals_hash);
	    
	    /// write slot signature
	    signature[i++] = signaturecount;
	    signature[i++] = 8;
	    signature[i++] = 8;
	    signature[i++] = 8;
	    signature[i++] = 0x05;
	    
	    signaturecount += strlen(Z_STRVAL_PP(slotdata)) + 1;
	}

    	zend_hash_internal_pointer_reset(slots_hash);

	while(zend_hash_has_more_elements(slots_hash) == SUCCESS)
	{
	    /// read slot from hashtable
	    zend_hash_get_current_key(slots_hash,&assocKey,&numKey,0);
	    zend_hash_get_current_data(slots_hash,(void**)&slotdata);

#ifdef MOC_DEBUG
	    qr->append(Z_STRVAL_PP(slotdata));
	    qr->append(" ");
	    cout << "\t" << signaturecount << "8 8 8 0x0a ::s" << endl;
#endif

	    meta_stringdata->append(Z_STRVAL_PP(slotdata));
	    meta_stringdata->append(QChar::Null);
	    
	    zend_hash_move_forward(slots_hash);

	    /// write slot signature
	    signature[i++] = signaturecount;
	    signature[i++] = 8;
	    signature[i++] = 8;
	    signature[i++] = 8;
	    signature[i++] = 0x0a;
	    
	    signaturecount += strlen(Z_STRVAL_PP(slotdata)) + 1;    
	}
#ifdef MOC_DEBUG
	cout << qr->toAscii().constData() << endl;
	cout << "+== end metaobject dump ==+" << endl;
#endif
	return true;
    } else {
	return false;
    }
} // getMocData

const char*
PHPQt::checkForOperator(const char* fname){
	return fname;
}

bool
PHPQt::SmokePHPObjectExists(const zval* this_ptr)
{
	return obj_x_smokephp.contains(this_ptr->value.obj.handle);
}

smokephp_object*
PHPQt::getSmokePHPObjectFromZval(const zval* this_ptr)
{

	if(this_ptr == NULL){
	  php_error(E_ERROR,"fatal: object does not exists and could not be fetched, %s",Z_OBJCE_P(const_cast<zval*>(this_ptr))->name);
	}

	// value.obj.handle
 	return obj_x_smokephp.value(this_ptr->value.obj.handle);

}

void*
PHPQt::getQtObjectFromZval(const zval* this_ptr){
	smokephp_object* o = getSmokePHPObjectFromZval(this_ptr);
	return o->mPtr();
}

smokephp_object*
PHPQt::getSmokePHPObjectFromQt(const void* QtPtr){
	return (smokephp_object*) SmokeQtObjects.value(QtPtr);
}

void
PHPQt::setSmokePHPObject(smokephp_object* o){
	SmokeQtObjects.insert(o->ptr(), o);
}

bool
PHPQt::SmokePHPObjectExists(const void* ptr){
	return (SmokeQtObjects.find(ptr) != SmokeQtObjects.end());
}

bool
PHPQt::unmapSmokePHPObject(const zval* zvalue)
{
	return static_cast<bool>( obj_x_smokephp.remove(zvalue->value.obj.handle) );
}

/**
 *	marshall_basetypes.h marshall_to_php<SmokeClassWrapper>(Marshall *m)
 */

smokephp_object*
PHPQt::createObject(zval* zval_ptr, const void* ptr, const zend_class_entry* ce, const Smoke::Index classId){

	Q_ASSERT (zval_ptr);
	Q_ASSERT (ptr);

 	if(!ce) {
 		qFatal("no class entry!");
 	}

	if(classId == QSTRING_CLASSID)
	{
		ce = qstring_ce;
	} else if (classId == 0)
	{
		qDebug("\nno class id");
		check_qobject(zval_ptr);
		qFatal("php object creation failed");
	}

	Z_TYPE_P(zval_ptr) = IS_OBJECT;
	object_init_ex(zval_ptr, const_cast<zend_class_entry*>(ce));
	smokephp_object* o = new smokephp_object(PQ::smoke(), classId, ptr, ce, zval_ptr);

	Z_OBJ_HT_P(zval_ptr) = &php_qt_handler;
	setSmokePHPObject(o);
	zval_add_ref(&zval_ptr);

	obj_x_smokephp.insert(zval_ptr->value.obj.handle, o);

	return o;

}

/**
 * create clone
 */

extern zend_class_entry* php_qt_generic_class;

smokephp_object*
PHPQt::cloneObject(zval* zval_ptr, smokephp_object* so, const void* copyPtr)
{
	Q_ASSERT (zval_ptr);
	Q_ASSERT (copyPtr);

	smokephp_object* o = new smokephp_object(PQ::smoke(), so->classId(), copyPtr, so->ce_ptr(), zval_ptr);

	Z_OBJ_HT_P(zval_ptr) = &php_qt_handler;
	setSmokePHPObject(o);
#warning check me
//	o->setAllocated( true );
	obj_x_smokephp.insert(zval_ptr->value.obj.handle, o);
	return o;
}

smokephp_object*
PHPQt::createOriginal(zval* zval_ptr, void* ptr)
{
	smokephp_object* o = getSmokePHPObjectFromQt(ptr);
 	zval_ptr = const_cast<zval*>(o->zval_ptr());
	zval_add_ref(&zval_ptr);
	return o;
}

void
PHPQt::createMetaObject(smokephp_object* o, zval* this_ptr)
{
    // call metaobject method
    const Smoke::Index nameId = o->smoke()->idMethodName("metaObject");
    const Smoke::Index meth = o->smoke()->findMethod(o->classId(), nameId);
    const Smoke::Method &methodId = o->smoke()->methods[o->smoke()->methodMaps[meth].method];
    const Smoke::ClassFn fn = o->smoke()->classes[methodId.classId].classFn;
    Smoke::StackItem i[1];
    (*fn)(methodId.method, const_cast<void*>(o->ptr()), i);

    // create the metaobject
    const QMetaObject *superdata = static_cast<QMetaObject *> ( i[0].s_voidp );
    QString phpqt_meta_stringdata;
    uint* phpqt_meta_data = static_cast<uint*>( ecalloc(sizeof(uint)*20*5+10, sizeof(uint)) );
    if(PHPQt::getMocData(
	   this_ptr,
	   o->parent_ce_ptr()->name,
	   superdata,
	   &phpqt_meta_stringdata,
	   phpqt_meta_data
	   ))
    {
	const char* phpqt_meta_stringdata_ = estrndup(phpqt_meta_stringdata.toAscii(), phpqt_meta_stringdata.size());
	QMetaObject ob = 
	    {
                { superdata, phpqt_meta_stringdata_, phpqt_meta_data, 0 }
            };
	QMetaObject* m = new QMetaObject;
	memcpy(m, &ob, sizeof(ob));
	o->setMetaObject(m);
    }
    else {
	o->setMetaObject(superdata);
    }
}
/*
void
PHPQt::Context::setActiveCe(zend_class_entry* ce)
{
    extern zend_class_entry* activeCe_;
    activeCe_ = ce;    
}

zend_class_entry* 
PHPQt::Context::activeCe()
{
    extern zend_class_entry* activeCe_;
    return activeCe_;
}

zval*
PHPQt::Context::activeScope()
{
    extern zval* activeScope_;
    return activeScope_;
}

void
PHPQt::Context::setActiveScope(zval* zval_ptr)
{
    extern zval* activeScope_;
    activeScope_ = zval_ptr;
}

void
PHPQt::Context::setParentCall(bool pc)
{
    extern bool parentCall_;
    parentCall_ = pc;
}

bool
PHPQt::Context::parentCall()
{
    extern bool parentCall_;
    return parentCall_;
}

void
PHPQt::Context::setMethodName(const char* name)
{
    extern QStack<QByteArray*> methodNameStack_;
    methodNameStack_.push( new QByteArray(name) );
}

void
PHPQt::Context::removeMethodName()
{
    extern QStack<QByteArray*> methodNameStack_;
    methodNameStack_.pop();
}

QByteArray*
PHPQt::Context::methodName()
{
    extern QStack<QByteArray*> methodNameStack_;
    return methodNameStack_.top();
}

const char*
PHPQt::Context::methodNameC()
{
    extern QStack<QByteArray*> methodNameStack_;
    return methodNameStack_.top()->constData();
}

*/
