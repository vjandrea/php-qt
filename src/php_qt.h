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

//#define PHPQT_MACPORT
#ifdef PHPQT_MACPORT
#warning MAC PORT
#endif

#ifndef QTPHP_H
#define QTPHP_H

#define PHPQT_VERSION "0.1.1"
#define QIDI_VERSION "0.1"

//#define MOC_DEBUG

#define MONITOR

#define COMPILE_DL_PHP_QT
#define PHPQT_CLASS_COUNT 256
#define QSTRING_CLASSID -1 // QString is not in smoke

#include <iostream>
using namespace std;

#include <QtCore/QHash>

#include <zend.h>
#include <zend_interfaces.h>
#include <php.h>
#include <php_ini.h>

#include "smoke.h"

#include <QtCore/QString>
#include <QtCore/QMetaObject>
#include <QtCore/QDebug>
#include <QtCore/QStack>

// for older php versions
#ifndef ZEND_MN
#define ZEND_MN ZEND_FN
#endif

#define Qnil (zval *) NULL

#define PHP_QT_FETCH()  \
	getQtObjectFromZval(getThis()) \

#define PHP_QT_FENTRY(zend_name, name_, arg_info_, flags_)	\
    t->fname = (char*) emalloc(strlen(#zend_name)+1); \
    t->fname = #zend_name; \
    t->handler = name_; \
    t->arg_info = arg_info_; \
    t->num_args = (zend_uint) (sizeof(arg_info_)/sizeof(struct _zend_arg_info)-1); \
    t->flags = flags_; \
    t++;

#define PHP_QT_ME(classname, name, arg_info, flags)	PHP_QT_FENTRY(name, ZEND_MN(classname##_##name), arg_info, flags)

// this is needed for override return_value, see qobject_cast
#if(PHP_MAJOR_VERSION > 5) || (PHP_MAJOR_VERSION == 5 && PHP_MINOR_VERSION > 0)
static
	ZEND_BEGIN_ARG_INFO_EX(phpqt_cast_arginfo, 0, 1, 0)
	ZEND_END_ARG_INFO();
#endif

extern "C" {

PHP_MINIT_FUNCTION(php_qt);
PHP_MSHUTDOWN_FUNCTION(php_qt);
PHP_RINIT_FUNCTION(php_qt);
PHP_RSHUTDOWN_FUNCTION(php_qt);
PHP_MINFO_FUNCTION(php_qt);

PHP_FUNCTION(confirm_php_qt_compiled);	/* For testing, remove later. */
/* emulate SIGNAL(), SLOT() macros */
PHP_FUNCTION(SIGNAL);
PHP_FUNCTION(SLOT);
#undef emit
PHP_FUNCTION(emit);
PHP_FUNCTION(qDebug);
PHP_FUNCTION(qWarning);
PHP_FUNCTION(qCritical);
PHP_FUNCTION(qAbs);
PHP_FUNCTION(qRound);
PHP_FUNCTION(qRound64);
PHP_FUNCTION(qMin);
PHP_FUNCTION(qMax);
PHP_FUNCTION(qBound);
PHP_FUNCTION(qPrintable);
PHP_FUNCTION(qFuzzyCompare);
PHP_FUNCTION(qIsNull);
PHP_FUNCTION(qIntCast);
PHP_FUNCTION(qVersion);
PHP_FUNCTION(PHPQtVersion);
PHP_FUNCTION(QiDiVersion);
PHP_FUNCTION(qSharedBuild);
PHP_FUNCTION(qMalloc);
PHP_FUNCTION(qFree);
PHP_FUNCTION(qRealloc);
PHP_FUNCTION(qMemCopy);
PHP_FUNCTION(qt_noop);
PHP_FUNCTION(qt_assert);
PHP_FUNCTION(qt_assert_x);
PHP_FUNCTION(Q_ASSERT);
PHP_FUNCTION(Q_ASSERT_X);
PHP_FUNCTION(qt_check_pointer);
PHP_FUNCTION(qobject_cast);
PHP_FUNCTION(tr);
PHP_FUNCTION(check_qobject);
PHP_FUNCTION(Q_UNUSED);
    
} // extern "C"

void check_qobject(zval* zobject);
void init_codec();
zval* zstringFromQString(QString * s);

class smokephp_object {
public:
	inline explicit smokephp_object(Smoke* smoke, const int classId, const void* ptr, const zend_class_entry* ce, zval* zval_ptr)
		: 	m_smoke(smoke),
			m_classId(classId),
			m_ptr(ptr),
			m_ce_ptr(ce),
			m_parent_ce_ptr(ce),
			m_zval_ptr(zval_ptr),
			m_allocated( false ),
			m_meta(0)
	{
	}

	inline const bool allocated() { return m_allocated; }
	inline Smoke* smoke() { return m_smoke; }
	inline const int classId() { return m_classId; }
	inline const void* ptr() { return m_ptr; }
	inline void* mPtr() { return const_cast<void*>(m_ptr); } // can be modified
	inline const zend_class_entry* ce_ptr() { return m_ce_ptr; }
	inline const zend_class_entry* parent_ce_ptr() { return m_parent_ce_ptr; }
	inline const zval* zval_ptr() { return m_zval_ptr; }
	inline const QMetaObject* meta() { return m_meta; }
	inline const QString className() { return QString(ce_ptr()->name); }

	inline void setAllocated( bool allocated ) { m_allocated = allocated; }
	inline void setParentCePtr(zend_class_entry* parent_ce_ptr) { m_parent_ce_ptr = parent_ce_ptr; }
	inline void setMetaObject(const QMetaObject* meta) { m_meta = meta; }
	inline void setPtr(void* ptr) { m_ptr = ptr; }

private:
	bool m_allocated; // true means ownership by bindings, Qt else
    Smoke *m_smoke;
    int m_classId;
    const void *m_ptr;
    const zend_class_entry *m_ce_ptr;
    const zend_class_entry *m_parent_ce_ptr;
    zval *m_zval_ptr;
    const QMetaObject* m_meta;
};

const char* 		printType(int type);

extern Smoke* qt_Smoke;
class PQ
{
private:
    PQ();
    PQ(const PQ& cc){}

public:
    ~PQ();
    static inline Smoke* smoke() { return qt_Smoke; }

	typedef short Index;

	static inline const char* findRealMethodName(const char* methodName)
	{

		if(!methodName) return "";
		QString _m(methodName);
		//! TODO its slow but safe, implement a better algorithm
		for(Index i=0; i < PQ::smoke()->numMethodNames; i++){
			if(_m.compare(PQ::smoke()->methodNames[i], Qt::CaseInsensitive) == 0){ return PQ::smoke()->methodNames[i]; }
		}
 		qFatal("PQ::findRealMethodName(): could not find %s", methodName);
	}

};

extern "C" {

ZEND_METHOD(php_qt_generic_class, __construct);
ZEND_METHOD(php_qt_generic_class, __destruct);
ZEND_METHOD(php_qt_generic_class, __toString);
ZEND_METHOD(php_qt_generic_class, emit);
ZEND_METHOD(php_qt_generic_class, proxyMethod);
ZEND_METHOD(php_qt_generic_class, staticProxyMethod);
    
} // extern "C"

// globals
//ZEND_BEGIN_MODULE_GLOBALS(phpqt)
//
//ZEND_END_MODULE_GLOBALS(phpqt)

// ZEND stuff

static int zend_check_symbol(zval **pz TSRMLS_DC)
{
	if (Z_TYPE_PP(pz) > 9) {
		fprintf(stderr, "Warning!  %x has invalid type!\n", *pz);
	} else if (Z_TYPE_PP(pz) == IS_ARRAY) {
		zend_hash_apply(Z_ARRVAL_PP(pz), (apply_func_t) zend_check_symbol TSRMLS_CC);
	} else if (Z_TYPE_PP(pz) == IS_OBJECT) {

		/* OBJ-TBI - doesn't support new object model! */
		zend_hash_apply(Z_OBJPROP_PP(pz), (apply_func_t) zend_check_symbol TSRMLS_CC);
	}

	return 0;
}

#define ZEND_VM_CONTINUE() return 0

#define CHECK_SYMBOL_TABLES()								                \
	zend_hash_apply(&EG(symbol_table), (apply_func_t) zend_check_symbol TSRMLS_CC);	                \
	if (&EG(symbol_table)!=EG(active_symbol_table)) {						\
		zend_hash_apply(EG(active_symbol_table), (apply_func_t) zend_check_symbol TSRMLS_CC);	\
	}


#define ZEND_VM_NEXT_OPCODE() \
	CHECK_SYMBOL_TABLES() \
	EX(opline)++; \
	ZEND_VM_CONTINUE()

#endif
