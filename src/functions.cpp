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

#include "php_qt.h"
#include "phpqt_internals.h"

#include <QtCore/QObject>

extern Smoke *qt_Smoke;
extern Smoke::Index qstring;
extern zend_class_entry* qstring_ce;
extern Smoke::Index qobject;
extern zend_class_entry* qobject_ce;

extern "C" {

/* Every user-visible function in PHP should document itself in the source */
/* proto string confirm_php_qt_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_php_qt_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char string[256];

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

  	len = sprintf(string, "Congratulations! Module PHP-Qt is now compiled into PHP.", arg);
	RETURN_STRINGL(string, len, 1);

}

/*!
 *	PHP userspace functions
 */

/*!
 *  emulates the Qt SIGNAL() macro
 */

PHP_FUNCTION(SIGNAL)
{
    const char* string;
    int string_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&string_len)) {
        return;
    }

//     char* tmp = (char*) emalloc((int) string_len + 2);
//     strcpy(tmp,"2");
//     strncat(tmp, string, (int) string_len);

	QString *qstring = new QString(string);
	qstring->prepend("2");
    ZVAL_STRING(return_value,(char*) qstring->toLatin1().constData(),1);

//     efree(tmp);

    return;
}

/*!
 *  emulates the Qt SLOT() macro
 */

PHP_FUNCTION(SLOT)
{
    const char* string;
    int string_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&string_len)) {
        return;
    }

//     char* tmp = (char*) emalloc((int) string_len + 2);
//     strcpy(tmp,"1");
//     strncat(tmp, string, (int) string_len);

	QString *qstring = new QString(string);
	qstring->prepend("1");
    ZVAL_STRING(return_value,(char*) qstring->toLatin1().constData(),1);

//     efree(tmp);

    return;
}

/*!
 *  emits a signal
 */

PHP_FUNCTION(emit)
{
    const char* string;
    int string_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&string_len)) {
        return;
    }
    QByteArray signature(string);
    return;
}


/**
 *	simply returns the first parameter because objects are casted automatically in smokephp_convertReturn(...)
 *
 *	function defined for compatibility
 */

PHP_FUNCTION(qobject_cast){

    zval *obj;
    zval *cast_type;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zz|", &obj, &cast_type) == FAILURE) {
        php_error(E_WARNING,"error while casting object, wrong parameters");
        return;
    }

	// just return the first argument
	// see marshall_basetypes.h, marshall_to_php<SmokeClassWrapper>
 	zval_ptr_dtor(return_value_ptr);
 	*(return_value_ptr) = obj;
    zval_add_ref(return_value_ptr);
    return;

}

/**
 * qDebug($message) triggers a notice with the given message.
 */
PHP_FUNCTION(qDebug) {
	const char* msg;
	int msglen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&msg,&msglen)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qDebug");
		return;
	}
	php_error(E_NOTICE,msg);
	return;
}

/**
 * qWarning($message) triggers a warning with the given message.
 */
PHP_FUNCTION(qWarning) {
	const char* msg;
	int msglen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&msg,&msglen)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qWarning");
		return;
	}
	php_error(E_WARNING,msg);
	return;
}

/**
 * qCritical($message) triggers a fatal error with the given message.
 */
PHP_FUNCTION(qCritical) {
	const char* msg;
	int msglen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&msg,&msglen)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qCritical");
		return;
	}
	php_error(E_ERROR,msg);
	return;
}

/**
 * Returns the absolute value of a number.
 */
PHP_FUNCTION(qAbs) {
	double x;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"d",&x)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qAbs");
		return;
	}
	RETURN_DOUBLE(0.0 >= x ? -x : x);
}

/**
 * Returns this number rounded to the nearest integer
 * if decimal is greater than .5 it rounds up, otherwise it rounds down
 */
PHP_FUNCTION(qRound) {
    double x;
    
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"d",&x)==FAILURE) {
	php_error(E_PARSE,"wrong parameters for qRound");
	return;
    }
    RETURN_LONG(x >= 0.00 ? (int)(x+0.5) : (int)(x - (int)(x-1) + 0.5) + (int)(x-1));
}

/**
 * Returns this number rounded to the nearest integer
 * if decimal is greater than .5 it rounds up, otherwise it rounds down
 */
PHP_FUNCTION(qRound64) {
	double x;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"d",&x)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qRound");
		return;
	}
	RETURN_DOUBLE(x >= 0.00 ? (int)(x+0.5) : (int)(x - (int)(x-1) + 0.5) + (int)(x-1));
}

/**
 * Return the smaller of two numbers
 * TODO: Make this capable of dealing with objects
 */
PHP_FUNCTION(qMin) {
	double x;
	double y;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"dd",&x,&y)==FAILURE) {
		php_error(E_PARSE,"wrong paramters for qMin");
		return;
	}
	if(x>y)
		RETURN_DOUBLE(y);
	RETURN_DOUBLE(x);
}

/**
 * Return the larger of two numbers
 * TODO: Make this capable of dealing with objects
 */
PHP_FUNCTION(qMax) {
	double x;
	double y;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"dd",&x,&y)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qMax");
		return;
	}
	if(x>y)
		RETURN_DOUBLE(x);
	RETURN_DOUBLE(y);
}

/**
 * Return a value bound by a minimum and maximum value.
 * TODO: Make this capable of dealing with objects
 */
PHP_FUNCTION(qBound) {
	double min;
	double val;
	double max;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"ddd",&min,&val,&max)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qBound");
		return;
	}
	if(min>max) {
		php_error(E_PARSE,"minimum cannot be greater than maximum for qBound");
		return;
	}
	if(val>max)
		val=max;
	if(val<min)
		val=min;
	RETURN_DOUBLE(val);
}

/**
 * Convert a string or character code into a printable character based on local 8 bit character set
 */
PHP_FUNCTION(qPrintable) {
	zval* string;
	QString *ptr;
	//int string_len;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&string)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qPrintable");
		return;
	}
	switch(Z_TYPE_P(string)) {
		case IS_NULL:
			RETURN_STRING("",1);
			break;
		case IS_BOOL:
			RETURN_STRING ("",1);
			break;
		case IS_LONG:
			ptr = new QString((int)Z_LVAL_P(string));
			RETURN_STRING((char *)ptr->toLocal8Bit().constData(),1);
			break;
		case IS_DOUBLE:
			ptr = new QString((int)Z_DVAL_P(string));
			RETURN_STRING((char*)ptr->toLocal8Bit().constData(),1);
			break;
		case IS_RESOURCE:
			RETURN_STRING("Resource",1);
			break;
		case IS_ARRAY:
			RETURN_STRING("Array",1);
			break;
		case IS_STRING:
			ptr = new QString(Z_STRVAL_P(string));
			RETURN_STRING((char*)ptr->toLocal8Bit().constData(),1);
			break;
		case IS_OBJECT:
			if(strcmp(Z_OBJCE_P(string)->name,"QString")==0) {
				ptr = (QString*) PHPQt::getQtObjectFromZval(string);
				RETURN_STRING((char*)ptr->toLocal8Bit().constData(),1);
			}
			RETURN_STRING(Z_OBJCE_P(string)->name,1);
			break;
	}
}

/**
 * Returns true if x-y is between 0.0 and 0.00000000001
 */
PHP_FUNCTION(qFuzzyCompare) {
	double x;
	double y;
	double ret;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"dd",&x,&y)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qFuzzyCompare");
		return;
	}
	//Just so happens the Qt version does just what we need for this one
	RETURN_BOOL(qFuzzyCompare(x,y));
}

/**
 * Returns true if variable is NULL or a numeric 0
 * This varies slightly from the Qt implementation which tests
 * a double or float for a NULL value only.
 */
PHP_FUNCTION(qIsNull) {
	zval *var;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&var)==FAILURE) {
		php_error(E_PARSE,"wrong parameters for qIsNull");
		return;
	}
	if(Z_TYPE_P(var)==IS_NULL)
		RETURN_BOOL(true);
	if(Z_TYPE_P(var)==IS_LONG) {
		if(Z_LVAL_P(var) == 0)
			RETURN_BOOL(true);
	}
	if(Z_TYPE_P(var)==IS_DOUBLE) {
		if(Z_DVAL_P(var) == 0.0)
			RETURN_BOOL(true);
	}
	RETURN_BOOL(false);
}

/**
 * Cast a variable to an integer. NULL casts to 0 Resources and Arrays return false.
 * Objects will call the __toInt() method if available, otherwise will return false.
 * This differs from the Qt version which can only convert a float or double to an int
 * TODO: implement __toInt() functionality
 */
PHP_FUNCTION(qIntCast) {
	zval *var;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&var)==FAILURE)
		return;
	switch(Z_TYPE_P(var)) {
		case IS_NULL:
			RETURN_LONG(0);
			break;
		case IS_LONG:
			RETURN_LONG(Z_LVAL_P(var));
			break;
		case IS_DOUBLE:
			RETURN_LONG((long)Z_DVAL_P(var));
			break;
		case IS_RESOURCE:
			RETURN_BOOL(false);
			break;
		case IS_ARRAY:
			RETURN_BOOL(false);
			break;
		case IS_OBJECT:
			RETURN_BOOL(false);
			break;
	}
}

/**
 * Return the version of Qt being used
 */
PHP_FUNCTION(qVersion) {
	RETURN_STRING(QT_VERSION_STR,1);
}

/**
 * Return the version of PHP-Qt being used
 */
PHP_FUNCTION(PHPQtVersion) {
	RETURN_STRING(PHPQT_VERSION,1);
}

/**
 * Return the version of QiDi that this release complies with
 */
PHP_FUNCTION(QiDiVersion) {
	RETURN_STRING(QIDI_VERSION,1);
}

PHP_FUNCTION(qSharedBuild) {
	RETURN_BOOL(qSharedBuild);
}

/**
 * We don't REALLY need to allocate memory in PHP, this is just here for ease of
 * porting code from C++
 * This doesn't ACTUALLY return a pointer to allocated memory exactly,
 * it creates a space of allocated memory and assigns to it a null string
 * then returns a pointer to this null string.
 */
PHP_FUNCTION(qMalloc) {
	long size;
	char *tmp;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"l",&size)==FAILURE)
		return;
	tmp = (char *)emalloc(size);
	qMemCopy(&tmp[0],"\0",1);
	RETURN_STRING(tmp,0);
}

/**
 * Simulate freeing the memory used by a variable by setting it to NULL
 */
PHP_FUNCTION(qFree) {
	zval* var;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&var)==FAILURE)
		return;
	convert_to_null(var);
}

/**
 * We don't REALLY need to allocate memory in PHP, this is just here for ease of
 * porting code from C++
 */
PHP_FUNCTION(qRealloc) {
	zval *val;
	double size;
	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zd",&val,&size)==FAILURE)
		return;
	RETURN_BOOL(true);
}

/**
 * Copy the contents of one pointer to another pointer
 * This is reasonably zval safe, but note the destination must already exist.
 * Additionally, the destination will automatically grow if the
 * size of the coppied data is to large
 * Currently this only supports string, int and float types
 * TODO: add support for array, objects, bool
 */
PHP_FUNCTION(qMemCopy) {
	zval* dest;
	zval* src;

	long size;
	int src_orig_type;
	int dest_orig_type;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"zzl",&dest,&src,&size)==FAILURE)
		return;

	src_orig_type = Z_TYPE_P(src);
	dest_orig_type = Z_TYPE_P(dest);

	switch(Z_TYPE_P(src)) {
		case IS_STRING:
		case IS_LONG:
		case IS_DOUBLE:
			convert_to_string(src);
			convert_to_string(dest);
			if(dest->value.str.len < size) {
				dest->value.str.val = (char*)emalloc((size_t)size);
				dest->value.str.len = size;
			}
			qMemCopy(dest->value.str.val,src->value.str.val,(size_t)size);
			break;
		default:
			RETURN_FALSE;
	}
	if(src_orig_type==IS_STRING || dest_orig_type==IS_STRING)
		RETURN_TRUE;
	if(src_orig_type==IS_DOUBLE || dest_orig_type==IS_DOUBLE) {
		RETURN_TRUE;
	}
	if(src_orig_type==IS_LONG && dest_orig_type==IS_LONG) {
		convert_to_long(dest);
		RETURN_TRUE;
	}
}

PHP_FUNCTION(qt_noop) {}

PHP_FUNCTION(qt_assert) {

	char* msg;
	int msglen;
	char* file;
	int filelen;
	int line;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|sl",&msg,&msglen,&file,&filelen,&line)==FAILURE)
		return;
	php_error(E_ERROR,msg);
}

PHP_FUNCTION(qt_assert_x) {

	char* msg;
	int msglen;
	char* file;
	int filelen;
	char* where;
	int wherelen;
	char* what;
	int whatlen;
	int line;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s|sssl",&msg,&msglen,&where,&wherelen,&what,&whatlen,&file,&filelen,&line)==FAILURE)
		return;
	php_error(E_ERROR,msg);
}


PHP_FUNCTION(Q_ASSERT) {

	bool cond;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b",&cond)==FAILURE)
		return;
	if(!cond)
		php_error(E_ERROR,"Assertion failed");
}

PHP_FUNCTION(Q_ASSERT_X) {

	bool cond;
	char* where="";
	int wherelen;
	char* what="";
	int whatlen;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"b|ss",&cond,&where,&wherelen,&what,&whatlen)==FAILURE) {
		return;
	}
	if(!cond) {
		php_error(E_ERROR,"Assertion failed: %s, %s",what,where);
	}
}

PHP_FUNCTION(qt_check_pointer) {

	char* file;
	int line;

	if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"|sl",&file,&line)==FAILURE)
		return;
	php_error(E_WARNING,"Out of memory");
}

PHP_FUNCTION(check_qobject)
{

    zval* zobject;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"z",&zobject)) {
        return;
    }

    check_qobject(zobject);
}

PHP_FUNCTION(Q_UNUSED) {
// just nothing
}
 
} // extern C

/*!
 *	check_qobject()
 *
 */


void check_qobject(zval* zobject)
{

	if(!PHPQt::SmokePHPObjectExists(zobject)) {

		cout << "PHP Object \n(" << endl;

		cout << "\t       zval => " << zobject << endl;
		if(Z_TYPE_P(zobject) == IS_OBJECT)
			cout << "\tclass entry => " << Z_OBJCE_P(zobject)->name << endl;
		cout << "\t  ref count => " << zobject->refcount << endl;
		cout << "\t     is_ref => " << (int) zobject->is_ref << endl;
		cout << "\t       type => " << printType(Z_TYPE_P(zobject)) << endl;

		if(Z_TYPE_P(zobject) == IS_OBJECT)
		{
		 cout <<"\t obj-handle => " << zobject->value.obj.handle << endl;
		}

		cout << ")" << endl;

	} else {

		smokephp_object* o = PHPQt::getSmokePHPObjectFromZval(zobject);

		cout << "PHP-Qt object \n(" << endl;

		cout << "\t       zval => " << zobject << endl;
// 		cout << "\tclass entry => " << Z_OBJCE_P(zobject)->name << endl;
		cout << "\tclass entry => " << o->ce_ptr()->name << endl;
		cout << "\t  ref count => " << zobject->refcount << endl;
		cout << "\t     is_ref => " << (int) zobject->is_ref << endl;
		cout << "\t       type => " << printType(Z_TYPE_P(zobject)) << endl;

		if(Z_TYPE_P(zobject) == 5)
		{
		 cout <<"\t obj-handle => " << zobject->value.obj.handle << endl;
		}

		cout << endl;

		cout << "\t      smokeobj => " << o << endl;
		cout << "\t         Smoke => " << o->smoke() << endl;
		cout << "\t       classId => " << o->classId() << endl;
		cout << "\t        Qt ptr => " << o->ptr() << endl;
		cout << "\t        ce_ptr => " << o->ce_ptr() << endl;
		cout << "\t      zval_ptr => " << o->zval_ptr() << endl;
		cout << "\t  QMetaObject* => " << o->meta() << endl;

		cout << ")" << endl;
	}
}


/*!
 *	tr() returns QObject::tr()
 *
 */
extern zval* zstringFromQString(QString* s);
extern "C" PHP_FUNCTION(tr)
{
    const char* string;
    int string_len;
    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC,"s",&string,&string_len)) {
        return;
    }

	QString *ptr = new QString(QObject::tr(string));
	PHPQt::createObject(return_value, ptr, qstring_ce, QSTRING_CLASSID);

    return;
}


const char* printType(int type)
{
 switch(type){
	case IS_NULL:	return "IS_NULL"; break; // 0
	case IS_LONG:	return "IS_LONG"; break; // 1
	case IS_DOUBLE: return "IS_DOUBLE"; break;	//2
	case IS_BOOL: return "IS_BOOL"; break; //	3
	case IS_ARRAY: return "IS_ARRAY"; break; // 4
	case IS_OBJECT: return "IS_OBJECT"; break; //	5
	case IS_STRING: return "IS_STRING"; break; // 6
	case IS_RESOURCE: return "IS_RESOURCE"; break; // 7
	case IS_CONSTANT: return "IS_CONSTANT"; break; // 8
	case IS_CONSTANT_ARRAY: return "IS_CONSTANT_ARRAY"; break; //	9
 }
 return "unknown";
}
