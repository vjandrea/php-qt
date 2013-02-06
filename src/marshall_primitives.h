/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

// TODO catch more types, see php_to_primitive<long>

template <>
static bool php_to_primitive<bool>(zval* v)
{
	if (v->type == IS_OBJECT) {
		// A Qt::Boolean has been passed as a value
		php_error(E_WARNING,"A Qt::Boolean has been passed as a value");
	} else {
		return Z_BVAL_P(v);
	}
}

template <>
static zval* primitive_to_php<bool>(bool sv, zval* return_value)
{
	RETVAL_BOOL(sv);
	return return_value;
}

template <>
static signed char php_to_primitive<signed char>(zval* v)
{
	return v->value.str.val[0];
}

template <>
static zval* primitive_to_php<signed char>(signed char sv, zval* return_value)
{
    php_error(E_ERROR,"signed char not implemented");
}

template <>
static unsigned char php_to_primitive<unsigned char>(zval* v)
{
	return v->value.str.val[0];
}

template <>
static zval* primitive_to_php<unsigned char>(unsigned char sv, zval* return_value)
{
    php_error(E_ERROR,"unsigned char not implemented");
}

template <>
static short php_to_primitive<short>(zval* v)
{
	return Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<short>(short sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static unsigned short php_to_primitive<unsigned short>(zval* v)
{
	return Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<unsigned short>(unsigned short sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static int php_to_primitive<int>(zval* v)
{
	if (v->type == IS_OBJECT) {
	    php_error(E_WARNING,"An integer has been passed as an object");
	} else {
		if(v->type == IS_LONG) {
	    	return Z_LVAL_P(v);
		// e.g. date() gives a string, so we try to convert it
		} else if (v->type == IS_STRING) {
			return QString(Z_STRVAL_P(v)).toLong();
		} else {
			php_error(E_ERROR,"wrong type, num expected");
		}
	}
}

template <>
static zval* primitive_to_php<int>(int sv, zval* return_value)
{
    RETVAL_LONG(sv);
  	return return_value;
}

template <>
static unsigned int php_to_primitive<unsigned int>(zval* v)
{
	if (v->type == IS_OBJECT) {
	    php_error(E_WARNING,"An unsigned integer has been passed as an object");
	} else {
	    return Z_LVAL_P(v);
	}
}

template <>
static zval* primitive_to_php<unsigned int>(unsigned int sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static long php_to_primitive<long>(zval* v)
{
	if (v->type == IS_OBJECT) {
	    php_error(E_WARNING,"A long has been passed as an object");
	} else {
	    return Z_LVAL_P(v);
	}
}

template <>
static zval* primitive_to_php<long>(long sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static unsigned long php_to_primitive<unsigned long>(zval* v)
{
	if (v->type == IS_OBJECT) {
	    php_error(E_WARNING,"An unsigned long has been passed as an object");
	} else {
	    return Z_LVAL_P(v);
	}
}

template <>
static zval* primitive_to_php<unsigned long>(unsigned long sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static long long php_to_primitive<long long>(zval* v)
{
	return Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<long long>(long long sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static unsigned long long php_to_primitive<unsigned long long>(zval* v)
{
        return Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<unsigned long long>(unsigned long long sv, zval* return_value)
{
        RETVAL_LONG(sv);
	return return_value;
}

template <>
static float php_to_primitive<float>(zval* v)
{
        return Z_DVAL_P(v);
}

template <>
static zval* primitive_to_php<float>(float sv, zval* return_value)
{
        RETVAL_DOUBLE(sv);
	return return_value;
}

template <>
static double php_to_primitive<double>(zval* v)
{
    if (v->type == IS_LONG){
	return Z_LVAL_P(v);
    } else if (v->type == IS_DOUBLE) {
	return Z_DVAL_P(v);
    } else {
	php_error(E_ERROR, "wrong argument type, double expected.");
    }
}

template <>
static zval* primitive_to_php<double>(double sv, zval* return_value)
{
        RETVAL_DOUBLE(sv);
	return return_value;
}

template <>
static char* php_to_primitive<char *>(zval* rv)
{
	if(rv == Qnil)
		return 0;

	QByteArray* b = new QByteArray(rv->value.str.val);
	return (char*) b->constData();
	
}

template <>
static unsigned char* php_to_primitive<unsigned char *>(zval* rv)
{
	if(rv == Qnil)
		return 0;

	int len = rv->value.str.len;
	char* mem = (char*) malloc(len+1);
	memcpy(mem, rv->value.str.val, len);
	mem[len] ='\0';
	return (unsigned char*) mem;
}

template <>
static zval* primitive_to_php<int*>(int* sv, zval* return_value)
{
	if(!sv) {
		return Qnil;
	}

	return primitive_to_php<int>(*sv, return_value);
}

#if defined(Q_OS_WIN32)
template <>
static WId php_to_primitive<WId>(zval* v)
{
	if(v == Qnil)
		return 0;

	return (WId) Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<WId>(WId sv, zval* return_value)
{
        RETVAL_LONG((unsigned long) sv);
	return return_value;
}

template <>
static Q_PID php_to_primitive<Q_PID>(zval* v)
{
	if(v == Qnil)
		return 0;

	return (Q_PID) Z_LVAL_P(v);
}

template <>
static zval* primitive_to_php<Q_PID>(Q_PID sv, zval* return_value)
{
        RETVAL_LONG((unsigned long) sv);
	return return_value;
}
#endif
