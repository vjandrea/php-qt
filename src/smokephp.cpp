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

#include "php_qt.h"
#include "context.h"
#include "smoke.h"
#include "marshall_types.h"
#include "phpqt_internals.h"

#include <QtCore/QMetaMethod>
#include <QtCore/QHash>
#include <QtCore/QCoreApplication>
#include <QtGui/QApplication>

#define ALLOCA_N(type,n) (type*)alloca(sizeof(type)*(n))

extern void init_qt_Smoke();
extern zend_class_entry* qstring_ce;
extern zend_class_entry* qobject_ce;
extern QHash<const void*, smokephp_object*> SmokeQtObjects;

class PHPQtSmokeBinding : public SmokeBinding {

public:
    PHPQtSmokeBinding(Smoke *s) : SmokeBinding(s) {}

    virtual void deleted(Smoke::Index classId, void* ptr) {
        if(PHPQt::SmokePHPObjectExists(ptr)){
	    smokephp_object *o = (smokephp_object*) PHPQt::getSmokePHPObjectFromQt(ptr);
	    if(!o->allocated()){
		delete static_cast<QObject*> ( ptr );
		delete (o);
	    } else {
		o->setPtr(0);
	    }
	    SmokeQtObjects.remove(o->ptr());
	}
    }
    virtual bool callMethod(Smoke::Index method, void* QtPtr, Smoke::Stack args, bool /*isAbstract*/) {

	smokephp_object *o = (smokephp_object*) PHPQt::getSmokePHPObjectFromQt(QtPtr);
	
	if(!o){
	    // no related smokephp_object
	    return false;
	}
	
	const char *methodName = smoke->methodNames[smoke->methods[method].name];

	// metaobjects
	if(!strcmp(methodName, "metaObject")){
	    args[0].s_class = const_cast<QMetaObject*>( o->meta() );
	    return true;
	}

	if(!strcmp(methodName, "qt_metacall")){
	    PHPQt::metacall(o, args, (QMetaObject::Call) args[1].s_enum, args[2].s_int, (void**) args[3].s_voidp);
	    return true;
	}

	if(PHPQt::methodExists(o->ce_ptr(), (char*) methodName)){
	    Context::setActiveScope( const_cast<zval*>(o->zval_ptr()) );
	    Context::setActiveCe( const_cast<zend_class_entry*>(o->ce_ptr()) );

	    zval* zmem = ALLOCA_N(zval, smoke->methods[method].numArgs);
	    zval* tmp = (zval*) emalloc(sizeof(zval));
	    VirtualMethodCall c(smoke, method, args, Context::activeScope(), &zmem, &tmp);
	    c.next();
	    return true;
	}

	return false;
    }

    virtual char *className(Smoke::Index classId) {
	// return a new[] copy of the language-specific name of this Smoke class
	// poorly designed function, but oh well. Sorry.
	
	const char *className = smoke->className(classId);
	char *buf = new char[strlen(className) + 1];
	strcpy(buf, className);
	return buf;
    }
    virtual ~PHPQtSmokeBinding() {}
};

/**
 * 	call init function
 *
 *	@param	-
 *	@return	-
 */

void
smokephp_init() 
{
    if (qt_Smoke != 0L) {
	php_error(E_ERROR,"could not initialize smoke");
    }

    init_qt_Smoke();
    
    if(PQ::smoke()->numClasses <= 0){
	php_error(E_ERROR,"could not initialize smoke (no class definitions)");
    }
    
    PQ::smoke()->binding = new PHPQtSmokeBinding(PQ::smoke());
}

/*!
 *  @param  const char*     c   class-name
 *  @param  const char*     m   mangled function-name
 *  @return Smoke::Index        unambiguous method ID
 */
Smoke::Index
smokephp_getMethod(const char* c, const char* m, const int argc, zval*** args)
{
    Smoke::Index method = PQ::smoke()->findMethod(c, m);	// qt_Smoke->methods
    Smoke::Index i = PQ::smoke()->methodMaps[method].method;

    if(i <= 0)
    {
	i = -i;		// turn into ambiguousMethodList index
	while(PQ::smoke()->ambiguousMethodList[i]) {

	    Smoke::Method &methodRef = PQ::smoke()->methods[PQ::smoke()->ambiguousMethodList[i]];
	    
	    if ((methodRef.flags & Smoke::mf_internal) == 0) {
		// try to compare smoke types with the php ones
		bool right = false;
		for(int k=0;k < argc;k++){
		    uint type = static_cast<uint>( (**args[k]).type );
		    switch((PQ::smoke()->types[PQ::smoke()->argumentList[PQ::smoke()->methods[PQ::smoke()->ambiguousMethodList[i]].args+k]].flags & Smoke::tf_elem)){
		    case Smoke::t_voidp:
			if(type == IS_ARRAY)
			    right = true;
			if(type == IS_STRING)
			    right = true;
			break;
		    case Smoke::t_bool:
			if(type == IS_BOOL)
			    right = true;
			break;
		    case Smoke::t_char:
		    case Smoke::t_uchar:
		    case Smoke::t_short:
		    case Smoke::t_ushort:
		    case Smoke::t_int:
		    case Smoke::t_uint:
		    case Smoke::t_long:
		    case Smoke::t_ulong:
			if(type == IS_LONG)
			    right = true;
			break;
		    case Smoke::t_float:
		    case Smoke::t_double:
			if(type == IS_DOUBLE)
			    right = true;
			break;
		    case Smoke::t_enum:
			php_error(E_WARNING,"type enum not yet implemented\n");
			break;
		    case Smoke::t_class:
			if(type == IS_OBJECT){
			    QByteArray* name =
				new QByteArray(
				    PQ::smoke()->types[
					PQ::smoke()->argumentList[
					    PQ::smoke()->methods[
						PQ::smoke()->ambiguousMethodList[i]].args+k]].name);
			    name->replace("const ","");
			    name->replace("*","");
			    name->replace("&","");

			    if( *name == QByteArray(Z_OBJCE_P( static_cast<zval*> (*args[k]))->name ))
				right = true;
			}
			break;
		    default:
			php_error(E_ERROR, "unknown argument type");
			break;
		    }
		}
		if(right){
#ifdef DEBUG
		    php_error(E_NOTICE, "Ambiguous Method %s::%s => %d, %d", c, m, PQ::smoke()->ambiguousMethodList[i], i);
#endif
		    return PQ::smoke()->ambiguousMethodList[i];
		}
	    }
	    i++;
    	}
    }
    return i;
}

/*!
 *  @param  void*           obj
 *  @param  Smoke::Index    method
 *  @param  Smoke::Stack    qargs
 *  @return void
 */
void
smokephp_callMethod(void *obj, const Smoke::Index method, Smoke::Stack qargs) {
    const Smoke::Method *m = PQ::smoke()->methods + method;
    const Smoke::ClassFn fn = PQ::smoke()->classes[m->classId].classFn;
    fn(m->method, obj, qargs);
}

/*!
 *  The munging works this way:
 *  $ is a plain scalar
 *  # is an object
 *  ? is a non-scalar (reference to array or hash, undef)
 *
 * data types
 * #define IS_NULL		0
 * #define IS_LONG		1
 * #define IS_DOUBLE	2
 * #define IS_BOOL		3
 * #define IS_ARRAY		4
 * #define IS_OBJECT	5
 * #define IS_STRING	6
 * #define IS_RESOURCE	7
 * #define IS_CONSTANT	8
 * #define IS_CONSTANT_ARRAY	9
 *
 *  @param  zval***                 args
 *	@param	int						argc
 *  @param  Smoke::StackItem*       qargs
 */

static
int treatArray(zval **val, const int num_args, va_list args, zend_hash_key *hash_key)
{
    const uint type = va_arg(args, uint);
    int e_arrayc = va_arg(args, int);
    void** e_arrayv = va_arg(args, void**);

    smokephp_object *o;
    if(type == IS_OBJECT)
	o = PHPQt::getSmokePHPObjectFromZval(((zval*) *val));

    switch(type){
    case IS_STRING:
	e_arrayv[e_arrayc] = emalloc(Z_STRLEN(**val)+1);
	e_arrayv[e_arrayc++] = reinterpret_cast<void*>( Z_STRVAL(**val) );
	break;
    case IS_LONG:
	e_arrayv[e_arrayc] = emalloc(sizeof(long));
	e_arrayv[e_arrayc++] = reinterpret_cast<void*>( Z_LVAL_PP(val) );
	break;
    case IS_DOUBLE:
	e_arrayv[e_arrayc] = emalloc(sizeof(double));
	e_arrayv[e_arrayc++] = reinterpret_cast<void*> (Z_LVAL_PP(val));
	break;
    case IS_BOOL:
	e_arrayv[e_arrayc] = emalloc(sizeof(bool));
	e_arrayv[e_arrayc++] = reinterpret_cast<void*>( Z_BVAL_PP(val) );
	break;
    case IS_OBJECT:
	e_arrayv[e_arrayc] = emalloc(sizeof(o->ptr()));
	e_arrayv[e_arrayc++] = const_cast<void*>(o->ptr());
	break;
    default:
	php_error(E_ERROR, "PHP-Qt: unsupported array type %d", type);
	break;
    }
    
    return ZEND_HASH_APPLY_KEEP;
}


void* transformArray(const zval* args) 
{
    // array informations
    int e_arrayc = zend_hash_num_elements( (*args).value.ht );	// length
    // find the first type
    const zval** first_elem;
    if(zend_hash_get_current_data_ex( (*args).value.ht, reinterpret_cast<void**> (&first_elem), 0) == FAILURE){
	php_error(E_ERROR, "PHP-Qt: could not get first value of hashtable.");
    }

    const uint type = static_cast<int>( (**first_elem).type );
    if(type < 0 || type > 9){
	// should never happen
	php_error(E_ERROR, "Could not get type of array");
    }
    void* e_arrayv;

    switch(type){
    case IS_STRING:
	e_arrayv = safe_emalloc(e_arrayc, sizeof(char*), 0);
	break;
    case IS_LONG:
	e_arrayv = safe_emalloc(e_arrayc, sizeof(long), 0);
	break;
    case IS_DOUBLE:
	e_arrayv = safe_emalloc(e_arrayc, sizeof(double), 0);
	break;
    case IS_BOOL:
	e_arrayv = safe_emalloc(e_arrayc, sizeof(bool), 0);
	break;
    case IS_OBJECT:
	e_arrayv = safe_emalloc(e_arrayc, sizeof(QObject*), 0);
	break;
    default:
	php_error(E_ERROR, "PHP-Qt: unsupported array type %d", type);
    }

    zend_hash_apply_with_arguments( (*args).value.ht, reinterpret_cast<apply_func_args_t>( treatArray ), 3, type, e_arrayc, e_arrayv );
    return e_arrayv;
}

void smokephp_prepareMethodName(zval*** args, const int argc)
{
    for(int i=0; i<argc; i++){
	const uint type = static_cast<int>( (**args[i]).type );
	if (type == IS_RESOURCE){ // TODO
	} else if (type == IS_ARRAY){
	    Context::methodName()->append("?");
	} else if (type == IS_BOOL){
	    Context::methodName()->append("$");
	} else if (type == IS_LONG){
	    Context::methodName()->append("$");
	} else if (type == IS_DOUBLE){
	    Context::methodName()->append("$");
	} else if (type == IS_STRING){
	    Context::methodName()->append("$");
	} else if (type == IS_OBJECT){
	    // as default QString is not supported in Smoke
	    if(!strcmp( Z_OBJCE_P(*args[i])->name, "QString" )){
		Context::methodName()->append("$");
	    } else {
		Context::methodName()->append("#");
	    }
	} else {
	    php_error(E_ERROR,"Unknown argument or unsupported type %d at argument %d, cannot prepare method call %s()\n", type, i, Context::methodName()->constData());
// 	        exit(FAILURE);
	}
    }
}

QByteArray* smokephp_getSignature(const int argc, zval ***argv, MocArgument* mocStack){

    mocStack[0].argType = xmoc_bool;	// return
    mocStack[0].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("bool"));

    QByteArray *signature = new QByteArray("(");// = new QByteArray();
    for(int i=0; i<argc; i++){
	const uint type = static_cast<int>( (**argv[i]).type );
	mocStack[i+1].st = SmokeType(PQ::smoke(),0);
	if (type == IS_RESOURCE){ // TODO
	} else if (type == IS_ARRAY){
	    //    xmoc_ptr,
	    php_error(E_WARNING, "Array given as signal argument");
	} else if (type == IS_BOOL){
	    mocStack[i+1].argType = xmoc_bool;
	    mocStack[i+1].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("bool"));
	    signature->append("bool");
	} else if (type == IS_LONG){
	    mocStack[i+1].argType = xmoc_int;
	    mocStack[i+1].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("int"));
	    signature->append("int");
	} else if (type == IS_DOUBLE){
	    mocStack[i+1].argType = xmoc_double;
	    mocStack[i+1].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("double"));
	    signature->append("double");
	} else if (type == IS_STRING){
	    mocStack[i+1].argType = xmoc_charstar;
	    mocStack[i+1].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("char*"));
	    signature->append("string");
	} else if (type == IS_OBJECT){
	    if(Z_OBJCE_P( *argv[i] ) == qstring_ce)
		mocStack[i+1].argType = xmoc_QString;
	    else {
		smokephp_object *o = PHPQt::getSmokePHPObjectFromZval( *argv[i] );
		mocStack[i+1].st = SmokeType(PQ::smoke(),o->classId());
		mocStack[i+1].argType = xmoc_void;
	    }
	    signature->append("object");
	} else {
	    php_error(E_ERROR,"Unknown argument or unsupported argument type %d, type %d, exit\n", i, type);
	    exit(FAILURE);
	}
    }
    signature->append(")");
    return signature;
}


void
smokephp_prepareConnect(zval*** args, const int argc, Smoke::StackItem* qargs, const Smoke::Index method)
{
    // second loop: we dont have the method-Id before the first call
    for(int j = 0; j < argc; j++){
 	uint type = static_cast<int>( (**args[j]).type );    // als Macro!
	if (type == IS_OBJECT) {
	    if(Z_OBJCE_PP( args[j] ) == qstring_ce) {
		QString* o = (QString*) PHPQt::getQtObjectFromZval(*args[j]);
		qargs[j+1].s_voidp = reinterpret_cast<void*> (const_cast<char*>( o->toAscii().constData() ));
	    }
	}
    }
}

bool
smokephp_isQObject(const Smoke::Index classId) 
{
    if( strcmp(PQ::smoke()->classes[classId].className, "QObject") == 0 )
	return true;    
    for(Smoke::Index *p = PQ::smoke()->inheritanceList + PQ::smoke()->classes[classId].parents;	*p; p++) 
    {
	if( smokephp_isQObject(*p) )
	    return true;
    }
    return false;
}

void
smokephp_QtToMoc(void** a, const QList<QByteArray> qargs, MocArgument* mocStack)
{
    for(int i = 0; i < qargs.count(); i++)
    {
	if( qargs[i] == "int" )
	{
	    mocStack[i].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("int"));
	    mocStack[i].argType = xmoc_int;
	}
	else if( qargs[i] == "char*" )
	{
	    mocStack[i].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("char*"));
	    mocStack[i].argType = xmoc_charstar;
	}
	else if( qargs[i] == "bool" )
	{
	    mocStack[i].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("bool"));
	    mocStack[i].argType = xmoc_bool;
	}
	else if( qargs[i] == "double" )
	{
	    mocStack[i].st = SmokeType(PQ::smoke(),PQ::smoke()->idType("double"));
	    mocStack[i].argType = xmoc_double;
	}
	else
	{
	    smokephp_object *o = PHPQt::getSmokePHPObjectFromQt(a[i]);
	    mocStack[i].st = SmokeType(PQ::smoke(),o->classId());
	    mocStack[i].argType = xmoc_void;
	}
    }
}
