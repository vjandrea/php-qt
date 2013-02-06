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

#include "marshall_types.h"
#include <QtCore/QHash>

template <class T> T* smoke_ptr(Marshall *m) { return (T*) m->item().s_voidp; }

template<> bool* smoke_ptr<bool>(Marshall *m) { return &m->item().s_bool; }
template<> signed char* smoke_ptr<signed char>(Marshall *m) { return &m->item().s_char; }
template<> unsigned char* smoke_ptr<unsigned char>(Marshall *m) { return &m->item().s_uchar; }
template<> short* smoke_ptr<short>(Marshall *m) { return &m->item().s_short; }
template<> unsigned short* smoke_ptr<unsigned short>(Marshall *m) { return &m->item().s_ushort; }
template<> int* smoke_ptr<int>(Marshall *m) { return &m->item().s_int; }
template<> unsigned int* smoke_ptr<unsigned int>(Marshall *m) { return &m->item().s_uint; }
template<> long* smoke_ptr<long>(Marshall *m) { 	return &m->item().s_long; }
template<> unsigned long* smoke_ptr<unsigned long>(Marshall *m) { return &m->item().s_ulong; }
template<> float* smoke_ptr<float>(Marshall *m) { return &m->item().s_float; }
template<> double* smoke_ptr<double>(Marshall *m) { return &m->item().s_double; }
template<> void* smoke_ptr<void>(Marshall *m) { return m->item().s_voidp; }

template <class T> T php_to_primitive(zval*);
template <class T> zval* primitive_to_php(T, zval* return_value);

template <class T>
static void marshall_from_php(Marshall *m)
{
	zval* zobj = m->var();
	(*smoke_ptr<T>(m)) = php_to_primitive<T>(zobj);
}

template <class T>
static void marshall_to_php(Marshall *m)
{
	if(m->doAlloc()){
		zval* z = (zval*) emalloc(sizeof(zval*));
		m->var(z);
	}
	*(m->var()) = *primitive_to_php<T>( *smoke_ptr<T>(m) , m->var());
}

#include "marshall_primitives.h"
//#include "marshall_complex.h"

// Special case marshallers

template <>
static void marshall_from_php<char *>(Marshall *m)
{
	zval* zobj = m->var();
	m->item().s_voidp = php_to_primitive<char*>(zobj);
}

template <>
static void marshall_from_php<SmokeEnumWrapper>(Marshall *m)
{
	zval* v = m->var();

	if (Z_TYPE_P(v) == IS_OBJECT) {
	// A Qt::Enum is a subclass of Qt::Integer, so 'get_qinteger()' can be called ok
	} else {
		m->item().s_enum = (long) Z_LVAL_P(v);
	}

}

template <>
static void marshall_to_php<SmokeEnumWrapper>(Marshall *m)
{
	long val = m->item().s_enum;
}

// m = MethodCall
template <>
static void marshall_from_php<SmokeClassWrapper>(Marshall *m)
{
	zval* zval_ptr = m->var();

	if(zval_ptr == 0) {
	    m->item().s_class = 0;
	    return;
	}

	if(zval_ptr->type == IS_NULL)
	{
	    m->item().s_class = 0;
	    return;
	}

	if(!PHPQt::SmokePHPObjectExists(zval_ptr)) {
		check_qobject(zval_ptr);
// 		php_error(E_ERROR, "Invalid type, expecting %s, %s given (probably PHP-Qt lost the Qt object)\n", m->type().name(), Z_OBJCE_P(zval_ptr)->name);
		php_error(E_ERROR, "Invalid type, expecting %s (probably PHP-Qt lost the Qt object)\n", m->type().name());
		return;
	}

	smokephp_object *o = PHPQt::getSmokePHPObjectFromZval(zval_ptr);

	if(!o || !o->ptr()) {
		if(m->type().isRef()) {
			php_error(E_WARNING, "References can't be nil\n");
			m->unsupported();
		}

		m->item().s_class = 0;
		return;
	}

	void *ptr = o->mPtr();

//	if((!m->cleanup() && m->type().isStack())) {
	if(m->type().isRef()){
		ptr = construct_copy(o);
#ifdef DEBUG
			php_error(E_WARNING, "copying %s %p to %p\n", resolve_classname(o->smoke(), o->classId(), o->ptr(), o->ptr(), ptr);
#endif
	}

	const Smoke::Class &cl = m->smoke()->classes[m->type().classId()];

	ptr = o->smoke()->cast(
		ptr,				// pointer
		o->classId(),			// from
		o->smoke()->idClass(cl.className)	// to
		);

	m->item().s_class = ptr;

	return;
}

template <>
static void marshall_to_php<SmokeClassWrapper>(Marshall *m)
{
    if( m->item().s_voidp == 0 ) {
	qWarning("Qt Object doesnt exist!");
	ZVAL_NULL(m->var());
	return;
    }
    void* p = m->item().s_voidp;

    // return the original
    if( PHPQt::SmokePHPObjectExists(p) ) 
    {
	if( m->return_value_ptr() )
	{
	    // destroys the return_value initialized by ZE, we create our own:
	    zval_ptr_dtor( m->return_value_ptr() );
	    // prepare the return value
	    smokephp_object* o = PHPQt::createOriginal( m->var(), p );
	    // overwrite the old one:
	    *(m->return_value_ptr()) = const_cast<zval*>( o->zval_ptr() );
	    m->var( *m->return_value_ptr() );
	}
	return;

    // create a new PHP object (return_value):
    } else {
	void* voidPtr = m->item().s_class;
	QObject* qObject = reinterpret_cast<QObject*> (voidPtr);
	const char* className = ( m->smoke()->classes[ m->type().classId() ].className ); // name of the class
	
	zend_class_entry *_ce;
	if ( Z_TYPE_P(m->var()) == IS_OBJECT )
	{
	    _ce = Z_OBJCE_P(m->var());
	    //! object has to be casted
	} else if( !strcmp(className, "QObject") ) {

            //! cast from QObject to class type	    
	    voidPtr = m->smoke()->cast(
		voidPtr, 
		m->smoke()->idClass("QObject"), 
		m->type().classId() );

	    //! cast the php one
	    _ce = zend_fetch_class( 
		const_cast<char*> ( qObject->metaObject()->className() ),
		strlen(qObject->metaObject()->className() ),
		ZEND_FETCH_CLASS_AUTO TSRMLS_DC);

        //! fallback, already with correct type
	} else {
	    _ce = zend_fetch_class(
		const_cast<char*>( className ), 
		strlen( className ), 
		ZEND_FETCH_CLASS_AUTO TSRMLS_DC );
	}

	if(m->doAlloc())
	{
	    zval* z = static_cast<zval*> ( emalloc(sizeof(zval)) );
	    m->var(z);
	}
	smokephp_object *o = PHPQt::createObject(
	    m->var(), 
	    voidPtr, 
	    _ce, 
	    m->type().classId() );
	
//	if(m->type().isConst() && m->type().isRef()) {
	if( m->type().isRef() )
	{
	    p = construct_copy( o );
#ifdef DEBUG
	    const char * _classname = o->ce_ptr()->name;
	    php_error(E_WARNING, "copying %s %p to %p\n", _classname, o->ptr(), p);
#endif
	    if(p) {
		o->setPtr(p);
		o->setAllocated(true);
	    }
	}
#ifdef DEBUG
	php_error(E_WARNING, "allocating %s %p -> %p\n", className, o->ptr(), static_cast<void*> (m->var()) );
#endif
	if( m->type().isStack() ) 
	{
	    o->setAllocated(true);
	}
    } // smokephp_object p doesn't exist
} // marshall_to_php

template <>
static void marshall_to_php<char *>(Marshall *m)
{
	char *sv = (char*)m->item().s_voidp;
	zval* zobj;
	if(sv) {
	    ZVAL_STRING(zobj,sv,/*duplicate*/ 1);
	} else {
	    zobj = Qnil;
	}
	if(m->cleanup())
		delete[] sv;

	*(m->var()) = *zobj;
// 	m->setRetval(zobj);
}

template <>
static void marshall_to_php<unsigned char *>(Marshall *m)
{
	m->unsupported();
}
