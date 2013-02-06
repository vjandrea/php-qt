/*!
 * PHP-Qt - The PHP language bindings for Qt
 *
 * Copyright (C) 2006 - 2007
 * Thomas Moenicke <tm at php-qt.org>
 *
 * marshall_types.cpp - Derived from the QtRuby and PerlQt sources, see AUTHORS
 *                       for details
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

#include <QtCore/QHash>
#include "marshall_types.h"
#include "phpqt_internals.h"

void
smokeStackToQtStack(const Smoke::Stack stack, void ** o, const int items, MocArgument* args)
{

	for (int i = 0; i < items; i++) {
		Smoke::StackItem *si = stack + i + 1;
		switch(args[i].argType) {
		case xmoc_bool:
			o[i] = &si->s_bool;
			break;
		case xmoc_int:
			o[i] = &si->s_int;
			break;
		case xmoc_double:
			o[i] = &si->s_double;
			break;
		case xmoc_charstar:
			o[i] = &si->s_voidp;
			break;
		case xmoc_QString:
			o[i] = si->s_voidp;
			break;
		default:
		{
			const SmokeType &t = args[i].st;
			void *p;
			switch(t.elem()) {
			case Smoke::t_bool:
				p = &si->s_bool;
				break;
			case Smoke::t_char:
				p = &si->s_char;
				break;
			case Smoke::t_uchar:
				p = &si->s_uchar;
				break;
			case Smoke::t_short:
				p = &si->s_short;
				break;
			case Smoke::t_ushort:
				p = &si->s_ushort;
				break;
			case Smoke::t_int:
				p = &si->s_int;
				break;
			case Smoke::t_uint:
				p = &si->s_uint;
				break;
			case Smoke::t_long:
				p = &si->s_long;
				break;
			case Smoke::t_ulong:
				p = &si->s_ulong;
				break;
			case Smoke::t_float:
				p = &si->s_float;
				break;
			case Smoke::t_double:
				p = &si->s_double;
				break;
			case Smoke::t_enum:
			{
				// allocate a new enum value
				Smoke::EnumFn fn = SmokeClass(t).enumFn();
				if (!fn) {
					php_error(E_WARNING, "Unknown enumeration %s\n", t.name());
					p = new int((int)si->s_enum);
					break;
				}
				const Smoke::Index id = t.typeId();
				(*fn)(Smoke::EnumNew, id, p, si->s_enum);
				(*fn)(Smoke::EnumFromLong, id, p, si->s_enum);
				// FIXME: MEMORY LEAK
				break;
			}
			case Smoke::t_class:
			case Smoke::t_voidp:
				if (strchr(t.name(), '*') != 0) {
					p = &si->s_voidp;
				} else {
					p = si->s_voidp;
				}
				break;
			default:
				p = 0;
				break;
			}
			o[i] = p;
		}
		}
	}
}

void
smokeStackFromQtStack(const Smoke::Stack stack, void ** _o, const int items, MocArgument* args)
{
	for (int i = 0; i < items; i++) {
		void *o = _o[i];
		switch(args[i].argType) {
		case xmoc_bool:
		stack[i].s_bool = *(bool*)o;
		break;
		case xmoc_int:
		stack[i].s_int = *(int*)o;
		break;
		case xmoc_double:
		stack[i].s_double = *(double*)o;
		break;
		case xmoc_charstar:
		stack[i].s_voidp = o;
		break;
		case xmoc_QString:
		stack[i].s_voidp = o;
		break;
		default:	// case xmoc_ptr:
		{
			const SmokeType &t = args[i].st;
			void *p = o;
			switch(t.elem()) {
			case Smoke::t_bool:
			stack[i].s_bool = **(bool**)o;
			break;
			case Smoke::t_char:
			stack[i].s_char = **(char**)o;
			break;
			case Smoke::t_uchar:
			stack[i].s_uchar = **(unsigned char**)o;
			break;
			case Smoke::t_short:
			stack[i].s_short = **(short**)p;
			break;
			case Smoke::t_ushort:
			stack[i].s_ushort = **(unsigned short**)p;
			break;
			case Smoke::t_int:
			stack[i].s_int = **(int**)p;
			break;
			case Smoke::t_uint:
			stack[i].s_uint = **(unsigned int**)p;
			break;
			case Smoke::t_long:
			stack[i].s_long = **(long**)p;
			break;
			case Smoke::t_ulong:
			stack[i].s_ulong = **(unsigned long**)p;
			break;
			case Smoke::t_float:
			stack[i].s_float = **(float**)p;
			break;
			case Smoke::t_double:
			stack[i].s_double = **(double**)p;
			break;
			case Smoke::t_enum:
			{
				Smoke::EnumFn fn = SmokeClass(t).enumFn();
				if (!fn) {
					php_error(E_WARNING, "Unknown enumeration %s\n", t.name());
					stack[i].s_enum = **(int**)p;
					break;
				}
				const Smoke::Index id = t.typeId();
				(*fn)(Smoke::EnumToLong, id, p, stack[i].s_enum);
			}
			break;
			case Smoke::t_class:
			case Smoke::t_voidp:
				if (strchr(t.name(), '*') != 0) {
					stack[i].s_voidp = *(void **)p;
				} else {
					stack[i].s_voidp = p;
				}
			break;
			}
		}
		}
	}
}

/**
 *	MethodReturnValueBase
 */

MethodReturnValueBase::MethodReturnValueBase(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval** return_value_ptr) :
	_smoke(smoke), _method(meth), _stack(stack), _return_value_ptr(return_value_ptr)
{
	_st.set(_smoke, method().ret);
}

const Smoke::Method&
MethodReturnValueBase::method()
{
	return _smoke->methods[_method];
}

Smoke::StackItem&
MethodReturnValueBase::item()
{
	return _stack[0];
}

Smoke *
MethodReturnValueBase::smoke()
{
	return _smoke;
}

SmokeType
MethodReturnValueBase::type()
{
	return _st;
}

void
MethodReturnValueBase::next() {}

bool
MethodReturnValueBase::cleanup()
{
	return false;
}

void
MethodReturnValueBase::unsupported()
{
	php_error(E_ERROR, "Cannot handle '%s' as return-type of %s::%s",
	type().name(),
	classname(),
	_smoke->methodNames[method().name]);
}

zval*
MethodReturnValueBase::var()
{
	return _retval;
}

zval*
MethodReturnValueBase::var(zval* zval_ptr)
{
	_retval = zval_ptr;
	return _retval;
}


const char *
MethodReturnValueBase::classname()
{
	return _smoke->className(method().classId);
}

zval**
MethodReturnValueBase::return_value_ptr()
{
	return _return_value_ptr;
}

/**
 *	VirtualMethodReturnValue
 */

VirtualMethodReturnValue::VirtualMethodReturnValue(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval* retval) :
	MethodReturnValueBase(smoke,meth,stack,NULL)
{
	_retval = retval;
	Marshall::HandlerFn fn = getMarshallFn(type());
	(*fn)(this);
}

Marshall::Action
VirtualMethodReturnValue::action()
{
	return Marshall::FromZVAL;
}

/**
 *	MethodReturnValue
 */

MethodReturnValue::MethodReturnValue(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval* retval, zval** return_value_ptr) :
	MethodReturnValueBase(smoke,meth,stack,return_value_ptr)/*, _return_value_ptr(return_value_ptr)*/
{
	_retval = retval;
	Marshall::HandlerFn fn = getMarshallFn(type());
	(*fn)(this);
}

Marshall::Action
MethodReturnValue::action()
{
	return Marshall::ToZVAL;
}

const char *
MethodReturnValue::classname()
{
	return qstrcmp(MethodReturnValueBase::classname(), "QGlobalSpace") == 0 ? "" : MethodReturnValueBase::classname();
}

/**
 *	MethodCallBase
 */

MethodCallBase::MethodCallBase(Smoke *smoke, const Smoke::Index meth, zval** return_value_ptr) :
	_smoke(smoke), _method(meth), _cur(-1), _called(false), _sp(0), _return_value_ptr(return_value_ptr)
{
}

MethodCallBase::MethodCallBase(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack,zval** return_value_ptr) :
	_smoke(smoke), _method(meth), _stack(stack), _cur(-1), _called(false), _sp(0), _return_value_ptr(return_value_ptr)
{
}

Smoke *
MethodCallBase::smoke()
{
	return _smoke;
}

SmokeType
MethodCallBase::type()
{
	return SmokeType(_smoke, _args[_cur]);
}

Smoke::StackItem &
MethodCallBase::item()
{
	return _stack[_cur + 1];
}

const Smoke::Method &
MethodCallBase::method()
{
	return _smoke->methods[_method];
}

void
MethodCallBase::next()
{
	const int oldcur = _cur;
	_cur++;
	while(!_called && _cur < items() ) {
		Marshall::HandlerFn fn = getMarshallFn(type());
		(*fn)(this);
		_cur++;
	}
	callMethod();
	_cur = oldcur;
}

void
MethodCallBase::unsupported()
{
	php_error(E_ERROR, "Cannot handle '%s' as argument of %s::%s",
		type().name(),
		classname(),
		_smoke->methodNames[method().name]);
}

const char*
MethodCallBase::classname()
{
	return _smoke->className(method().classId);
}

zval**
MethodCallBase::return_value_ptr()
{
	return _return_value_ptr;
}

/**
 *	VirtualMethodCall
 */

VirtualMethodCall::VirtualMethodCall(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval* obj, zval **sp, zval** return_value_ptr) :
 	MethodCallBase( smoke, meth, stack, return_value_ptr), _obj(obj)/*, _sp(sp)*/
{
	__sp = sp;
  	_args = _smoke->argumentList + method().args;
}

VirtualMethodCall::~VirtualMethodCall()
{
}

Marshall::Action
VirtualMethodCall::action()
{
	return Marshall::ToZVAL;
}

zval*
VirtualMethodCall::var()
{
    return __sp[_cur];
}

zval*
VirtualMethodCall::var(zval* zval_ptr)
{
	__sp[_cur] = zval_ptr;
    return __sp[_cur];
}

const int
VirtualMethodCall::items()
{
	return method().numArgs;
}

void
VirtualMethodCall::callMethod()
{
	if (_called) return;
	_called = true;

	zval* retval = PHPQt::callPHPMethod(_obj, _smoke->methodNames[method().name], items(), __sp);
	VirtualMethodReturnValue r(_smoke, _method, _stack, retval);
}

bool
VirtualMethodCall::cleanup()
{
	return false;
}

/**
 *	MethodCall
 */

MethodCall::MethodCall(Smoke *smoke, const Smoke::Index method, zval* target, zval ***sp, zval *retval, zval** return_value_ptr) :
 	MethodCallBase(smoke,method,return_value_ptr), _target(target), _current_object(0), _sp(sp), _retval(retval)
{
    if(target != NULL)
    {
		if (PHPQt::SmokePHPObjectExists(_target))
		{
			smokephp_object *o = PHPQt::getSmokePHPObjectFromZval(_target);
			if (o && o->ptr())
			{
				_current_object = o->mPtr();
				_current_object_class = o->classId();
			}
		}
	} else {_target = (zval*) emalloc(sizeof(zval));}

    _args = _smoke->argumentList + _smoke->methods[_method].args;
    _items = _smoke->methods[_method].numArgs;

    _stack = new Smoke::StackItem[_items + 1];
}

MethodCall::~MethodCall()
{
    delete[] _stack;
}

Marshall::Action
MethodCall::action()
{
	return Marshall::FromZVAL;
}

zval*
MethodCall::var()
{
	if (_cur < 0) return _retval;
	return (zval*) *_sp[_cur];
}

zval*
MethodCall::var(zval* zval_ptr)
{
	return var();
}

void
MethodCall::callMethod() {
	if(_called) return;
	_called = true;

	QString className(_smoke->className(method().classId));

	if (! className.endsWith(_smoke->methodNames[method().name])
		&& Z_TYPE_P(_target) == IS_NULL
		&& !(method().flags & Smoke::mf_static) )
	{
		php_error(E_ERROR, "Instance is not initialized, cannot call %s",
					_smoke->methodNames[method().name]);
	}

	if (Z_TYPE_P(_target) == IS_NULL && !(method().flags & Smoke::mf_static)) {
		php_error(E_ERROR, "%s is not a class method\n", _smoke->methodNames[method().name]);
	}

	const Smoke::ClassFn fn = _smoke->classes[method().classId].classFn;
	void *ptr = _smoke->cast(_current_object, _current_object_class, method().classId);
	_items = -1;

	(*fn)(method().method, ptr, _stack);
	MethodReturnValue r(_smoke, _method, _stack, _retval, _return_value_ptr);
}


const int
MethodCall::items()
{
	return _items;
}

bool
MethodCall::cleanup()
{
	return true;
}

const char *
MethodCall::classname()
{
	return qstrcmp(MethodCallBase::classname(), "QGlobalSpace") == 0 ? "" : MethodCallBase::classname();
}

/**
 *	InvokeSlot
 */

InvokeSlot::InvokeSlot(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, const zval* obj, zval **sp, zval** return_value_ptr, const int slotId, const QMetaObject* metaObject, void** a) :
    MethodCallBase( smoke, meth, stack, return_value_ptr), 
    _obj(obj), 
    __sp(sp), 
    _id(slotId), 
    _metaObject(metaObject), 
    _a(a)
{
    _args = _smoke->argumentList + method().args;
    // break at the first '('
    _slotName = new QByteArray(_metaObject->method(_id).signature());
    int ab = _slotName->indexOf("(");
    _slotName = new QByteArray(_slotName->left(ab));

    QList<QByteArray> qargs = _metaObject->method(_id).parameterTypes();
    _items = qargs.count();
    _mocStack = new MocArgument[ _items + 1 ];
    smokephp_QtToMoc(_a + 1, qargs, _mocStack);
    smokeStackFromQtStack(_stack, _a + 1, _items, _mocStack);
}

InvokeSlot::~InvokeSlot()
{
}

Marshall::Action
InvokeSlot::action()
{
	return Marshall::ToZVAL;
}

zval*
InvokeSlot::var()
{
    return __sp[_cur-1];
}

zval*
InvokeSlot::var(zval* zval_ptr)
{
    return __sp[_cur-1];
}

const int
InvokeSlot::items()
{
    return _items;
}

void
InvokeSlot::callMethod()
{
    if (_called) return;
    _called = true;
    zval* retval = PHPQt::callPHPMethod(_obj, _slotName->constData(), items(), __sp-1);
//    if(_args[0].argType != xmoc_void)
//	VirtualMethodReturnValue r(_smoke, _method, _stack, retval);
}

bool
InvokeSlot::cleanup()
{
	return false;
}

bool
InvokeSlot::callable(const zend_class_entry* ce)
{
    return PHPQt::methodExists(ce, _slotName->constData());
}

SmokeType
InvokeSlot::type()
{
    return _mocStack[_cur].st;
}

Smoke::StackItem &
InvokeSlot::item()
{
    return _stack[_cur];   
}


/**
 *	EmitSignal
 */

EmitSignal::EmitSignal(QObject *obj, const int id, const int items, MocArgument *mocStack, zval ***sp, zval * result) :
  _obj(obj), _id(id), _cur(0), _called(false)
{
    _items = items;
    _args = mocStack;
    _id = id;
    _sp = sp;
    _result = result;
    _stack = new Smoke::StackItem[ items ];
}

Marshall::Action
EmitSignal::action()
{
    return Marshall::FromZVAL;
}

Smoke::StackItem &
EmitSignal::item()
{
    return _stack[_cur];
}

const char *
EmitSignal::mytype()
{
    return "signal";
}

void
EmitSignal::emitSignal()
{
    if (_called) return;
    _called = true;
    
    void ** o = new void*[_items];
    smokeStackToQtStack(_stack, o + 1, _items, _args + 1);
    _obj->metaObject()->activate(_obj, _id, o);

    delete[] o;
}

void
EmitSignal::mainfunction()
{
    emitSignal();
}

bool
EmitSignal::cleanup()
{
    return true;
}

SmokeType
EmitSignal::type()
{
    return arg().st;
}

zval*
EmitSignal::var()
{
    return *_sp[ _cur-1 ];
}

Smoke *
EmitSignal::smoke()
{
    return type().smoke();
}

void
EmitSignal::unsupported()
{
	php_error(E_ERROR, "Cannot handle '%s' as %s argument\n", type().name(), mytype() );
}

const MocArgument &
EmitSignal::arg()
{
	return _args[_cur];
}

void
EmitSignal::next()
{
	const int oldcur = _cur;
	_cur++;

	while(!_called && _cur <= _items) {
	  Marshall::HandlerFn fn = getMarshallFn(type());
	  (*fn)(this);
	  _cur++;
	}
	emitSignal();
	_cur = oldcur;
}

