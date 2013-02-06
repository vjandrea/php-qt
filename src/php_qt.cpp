/*!
 * PHP-Qt - The PHP language bindings for Qt
 *
 * Copyright (C) 2006 - 2007
 * Thomas Moenicke <tm at php-qt.org>
 * Katrina Niolet <katrina at niolet.name>
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

#include <QtCore/qglobal.h>
#include <QtCore/QHash>

#include "php_qt.h"
#include "context.h"
#include "ext/standard/info.h"
#include "ext/standard/php_string.h"

#include "zend_handlers.h"
#include "smoke.h"
#include "marshall_types.h"
#include "phpqt_internals.h"

#define DEBUG 1
//#define MOC_DEBUG 0

extern void init_qt_Smoke();
extern TypeHandler Qt_handlers[];
void install_handlers(TypeHandler *);

static int le_php_qt;
static int le_php_qt_hashtype; // object list
static HashTable php_qt_objptr_hash;
extern zend_object_handlers php_qt_handler; // object handler

// context, threadsafe
QStack<QByteArray*> methodNameStack_;
zend_class_entry* activeCe_;
zval* activeScope_;
bool parentCall_; 

// cached, readonly
Smoke::Index qbool;
Smoke::Index qstring;
Smoke::Index qobject;
extern zend_class_entry* qobject_ce;
extern zend_class_entry* qstring_ce;

extern void 	_register_QString();

PHP_INI_BEGIN()
    PHP_INI_ENTRY("qt.codec", "UTF8", PHP_INI_ALL, NULL)
PHP_INI_END()

/*! php_qt_functions[]
 *
 * Every user visible function must have an entry in php_qt_functions[].
 */
#undef emit
function_entry php_qt_functions[] = {
	PHP_FE(confirm_php_qt_compiled,	NULL)		/* For testing, remove later. */
	PHP_FE(SIGNAL,	NULL)
	PHP_FE(SLOT,	NULL)
	PHP_FE(emit,	NULL)
	PHP_FE(qDebug,	NULL)
	PHP_FE(qWarning,	NULL)
	PHP_FE(qCritical,	NULL)
	PHP_FE(qAbs, 		NULL)
	PHP_FE(qRound,		NULL)
	PHP_FE(qRound64,	NULL)
	PHP_FE(qMin,		NULL)
	PHP_FE(qMax,		NULL)
	PHP_FE(qBound,		NULL)
	PHP_FE(qPrintable,	NULL)
	PHP_FE(qFuzzyCompare,	NULL)
	PHP_FE(qIsNull,		NULL)
	PHP_FE(qIntCast,	NULL)
	PHP_FE(qVersion,	NULL)
	PHP_FE(PHPQtVersion,	NULL)
	PHP_FE(QiDiVersion,	NULL)
	PHP_FE(qSharedBuild,		NULL)
	PHP_FE(qMalloc,		NULL)
	PHP_FE(qFree,		NULL)
	PHP_FE(qRealloc,	NULL)
	PHP_FE(qMemCopy,	NULL)
	PHP_FE(qt_noop,		NULL)
	PHP_FE(qt_assert,	NULL)
	PHP_FE(qt_assert_x,	NULL)
	PHP_FE(Q_ASSERT,	NULL)
	PHP_FE(Q_ASSERT_X,	NULL)
	PHP_FE(qt_check_pointer,	NULL)
	PHP_FE(qobject_cast,	phpqt_cast_arginfo)
	PHP_FE(tr,	NULL)
	PHP_FE(check_qobject,	NULL)
	PHP_FE(Q_UNUSED,	NULL)
	{NULL, NULL, NULL}	/* Must be the last line in php_qt_functions[] */
};

/*! php_qt_module_entry
 */
zend_module_entry php_qt_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"php_qt",
	php_qt_functions,
	PHP_MINIT(php_qt),
	PHP_MSHUTDOWN(php_qt),
	PHP_RINIT(php_qt),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(php_qt),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(php_qt),
#if ZEND_MODULE_API_NO >= 20010901
	PHPQT_VERSION,
#endif
	STANDARD_MODULE_PROPERTIES
};

#ifdef COMPILE_DL_PHP_QT
ZEND_GET_MODULE(php_qt)
#endif

/**
 *	generic object
 */

static zend_function_entry php_qt_generic_methods[] = {
    ZEND_ME(php_qt_generic_class,__construct,NULL,ZEND_ACC_PUBLIC)
    ZEND_ME(php_qt_generic_class,__destruct,NULL,ZEND_ACC_PUBLIC)
    ZEND_ME(php_qt_generic_class,__toString,NULL,ZEND_ACC_PUBLIC)
    ZEND_ME(php_qt_generic_class,emit,NULL,ZEND_ACC_PUBLIC)
    ZEND_ME(php_qt_generic_class,proxyMethod,NULL,ZEND_ACC_PUBLIC)
    ZEND_ME(php_qt_generic_class,staticProxyMethod,NULL,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC)
    {NULL,NULL,NULL}
};

    extern "C" {

/* emit does nothing */
ZEND_METHOD(php_qt_generic_class, emit){}

ZEND_METHOD(php_qt_generic_class, __toString)
{
	RETURN_STRING("", 1);
}

ZEND_METHOD(php_qt_generic_class, __destruct)
{ 
    if(PHPQt::SmokePHPObjectExists(getThis())) {
	smokephp_object *o = PHPQt::getSmokePHPObjectFromZval(getThis());
	//! its not a reference
	if(!PHPQt::unmapSmokePHPObject(getThis()))
	{
	    qFatal("try to unmap unregistered zval");
	}
	if( o->allocated() )
	{
#ifndef PHPQT_MACPORT
       	    delete o->ptr();
#endif
	}

    }
}

ZEND_METHOD(php_qt_generic_class, __construct)
{
    Context::setActiveScope( getThis() );

    // find parents
    zend_class_entry *ce = Z_OBJCE_P(getThis());
    zend_class_entry *ce_parent = Z_OBJCE_P(getThis());

    while (PQ::smoke()->idClass(ce->name) <= 0) {
// 	    ce_parent = ce->parent;
	ce = ce->parent; // orig
    }

    Context::setActiveCe( ce );

    // get arguments
    const int argc = ZEND_NUM_ARGS();
    zval ***args = (zval ***) safe_emalloc(argc, sizeof(zval **), 0);
    if(zend_get_parameters_array_ex(argc, args) == FAILURE)
    {
	efree(args);   WRONG_PARAM_COUNT;
    }

    Context::setMethodName( ce->name );

    smokephp_prepareMethodName(args, argc);	// #, $, ?
    const Smoke::Index method = smokephp_getMethod(ce->name, Context::methodName()->constData(), ZEND_NUM_ARGS(), args);

    MethodCall c(PQ::smoke(), method, getThis(), args, getThis(), return_value_ptr);
    c.next();

    // smokephp_object is created above in c.next()
    smokephp_object* o = PHPQt::getSmokePHPObjectFromZval(getThis());
    o->setParentCePtr(ce_parent); // = ce if no parent
    o->setAllocated( true );
    
    // Metaobject
    if(smokephp_isQObject(PQ::smoke()->idClass(ce->name)))
	PHPQt::createMetaObject(o, getThis());

    // cleanup
    efree(args);
    Context::removeMethodName();

    return;
}

ZEND_METHOD(php_qt_generic_class, proxyMethod)
{
    zend_class_entry *ce;
    // nonstaticphp_qt_generic_class_proxyMethod
    if(getThis()){
	// if a parent:: call occurs this_ptr has the wrong ce, so we need to
	// correct it here
	if( Context::parentCall() )
	{
	    ce = Context::activeCe();
	    Context::setParentCall( false );
        } else {
	    Context::setActiveScope( getThis() );
	    ce = Z_OBJCE_P(getThis());
	    Context::setActiveCe( ce );
        }
	// static
    } else {
	ce = Context::activeCe();
    }

    // find parents
    while (PQ::smoke()->idClass(ce->name) <= 0) {
	ce = ce->parent;
    }
    // arguments
    const int argc = ZEND_NUM_ARGS();
    zval ***args = static_cast<zval***>( safe_emalloc(argc, sizeof(zval **), 0) );

    if(zend_get_parameters_array_ex(argc, args) == FAILURE){
	efree(args);
	WRONG_PARAM_COUNT;
    }

#warning implement me
    smokephp_prepareMethodName(args, argc);
    const Smoke::Index method = smokephp_getMethod(ce->name, Context::methodName()->constData(), argc, args);

    if(method <= 0) {
	if(Context::methodName()->constData()) {

	    // is it a signal?
	    if(getThis()){
		smokephp_object* o = PHPQt::getSmokePHPObjectFromZval(getThis());
		if(o->meta() != 0){
		    const QMetaObject* mo = o->meta();
		    QByteArray signalName( Context::methodName()->constData() );
		    signalName.replace("$","");
		    signalName.replace("#","");
		    signalName.replace("?","");
		    MocArgument *mocStack = new MocArgument[argc+1]; // first entry is return value
		    signalName.append( smokephp_getSignature(argc, args, mocStack)->constData() );
		    // seems to be a signal
		    const int index = mo->indexOfSignal( signalName );
		    if( index >= 0 ) {
			QObject *qobj = static_cast<QObject*> ( o->smoke()->cast(
			    const_cast<void*>(o->ptr()),
			    o->classId(),
			    o->smoke()->idClass("QObject")
			) );
			zval* result;
#ifdef MOC_DEBUG
			cout << "+\tcall EmitSignal " << signalName.constData() << endl;
#endif
			EmitSignal signal(qobj, index, argc, mocStack, args, result);
			signal.next();
			RETURN_NULL();
		    }
		}
	    }

	    php_error(E_ERROR,"Call to undefined method %s::%s() or wrong arguments", ce->name, Context::methodName()->constData());

	}
	else
	    php_error(E_ERROR,"Call to undefined method!");
    }

    MethodCall c( PQ::smoke(), method, getThis(), args, return_value, return_value_ptr );
    c.next();

    // cleanup
    efree(args);
    Context::removeMethodName();

    return;
} // proxyMethod

ZEND_METHOD(php_qt_generic_class, staticProxyMethod)
{
	this_ptr = NULL;

	// do we have a parent::foo() call?
	if(EG(active_op_array)->scope){
		if(EG(scope) == EG(active_op_array)->scope->parent)
		{
			if( Context::activeScope() )
			{
				Context::setParentCall( true );
				this_ptr = Context::activeScope();
			}
		}
	}

	// forward to proxyMethod
	zim_php_qt_generic_class_proxyMethod(ht, return_value, return_value_ptr, this_ptr, return_value_used);
}

/*!
 *	PHP_MINIT_FUNCTION
 */

PHP_MINIT_FUNCTION(php_qt)
{
    Context::createContext();

    REGISTER_INI_ENTRIES();
    init_codec();
    
    install_handlers(Qt_handlers);
    
    // object list
    le_php_qt_hashtype = zend_register_list_destructors_ex(PHPQt::destroyHashtable, NULL, "PHP-Qt object list", module_number);
    zend_hash_init_ex(&php_qt_objptr_hash, PHPQT_CLASS_COUNT, NULL, NULL, 1, 0);
    
    ZendHandlers::installZendHandlers();
    smokephp_init();

    // cache class entries
    QHash<const char*, zend_class_entry*> tmpCeTable;
	
    // loop for all classes, register class entries
    for(Smoke::Index i = 1; i <= PQ::smoke()->numClasses; i++)
    {

	zend_function_entry* t = static_cast<zend_function_entry*>( safe_emalloc(7, sizeof(zend_function_entry), 0) );
	zend_function_entry* p = t;
	
	PHP_QT_ME(php_qt_generic_class,__construct,phpqt_cast_arginfo,ZEND_ACC_PUBLIC);
	PHP_QT_ME(php_qt_generic_class,__destruct,NULL,ZEND_ACC_PUBLIC);
	PHP_QT_ME(php_qt_generic_class,__toString,NULL,ZEND_ACC_PUBLIC);
	PHP_QT_ME(php_qt_generic_class,emit,NULL,ZEND_ACC_PUBLIC);
	PHP_QT_ME(php_qt_generic_class,proxyMethod,phpqt_cast_arginfo,ZEND_ACC_PUBLIC);
	PHP_QT_ME(php_qt_generic_class,staticProxyMethod,phpqt_cast_arginfo,ZEND_ACC_PUBLIC|ZEND_ACC_STATIC);
	
	// register zend class
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, PQ::smoke()->classes[i].className, p);
	ce.name_length = strlen(PQ::smoke()->classes[i].className);
	zend_class_entry* ce_ptr = zend_register_internal_class(&ce TSRMLS_CC);
	tmpCeTable[PQ::smoke()->classes[i].className] = ce_ptr;

	// cache QObject
	if(qobject == i){
	    qobject_ce = ce_ptr;
	}
    } // end loop classes

    // do inheritance, all classes must be defined before
    for(Smoke::Index i = 1; i <= PQ::smoke()->numClasses; i++)
    {
	zend_class_entry* ce = tmpCeTable[PQ::smoke()->classes[i].className];
	for(Smoke::Index *p = PQ::smoke()->inheritanceList + PQ::smoke()->classes[i].parents; *p; p++) 
	{
	    zend_class_entry *parent_ce = tmpCeTable[PQ::smoke()->classes[*p].className];
	    zend_do_inheritance(ce, parent_ce TSRMLS_CC);
	}
    }
    
    // cache some stuff
    qobject = PQ::smoke()->idClass("QObject");
    _register_QString();

    return SUCCESS;
} // PHP_MINIT


/* PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(php_qt)
{
    Context::destroyContext();
    return SUCCESS;
}

/*! PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(php_qt)
{
	return SUCCESS;
}

/*! PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(php_qt)
{
	zend_hash_destroy(&php_qt_objptr_hash);
	return SUCCESS;
}

/*! PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(php_qt)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "PHP-Qt support", "enabled");
	php_info_print_table_end();

	DISPLAY_INI_ENTRIES();
}
 
} // extern C
