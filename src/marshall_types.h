/***************************************************************************
    marshall_types.cpp - Derived from the QtRuby and PerlQt sources, see AUTHORS
                         for details
                             -------------------
    begin                : March 21 2007
    copyright            : (C) 2007 by Thomas Moenicke
    email                : tm@php-qt.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MARSHALL_TYPES_H
#define MARSHALL_TYPES_H

#include <QString>
#include <QObject>
#include <Qt/qmetaobject.h>

//#include "marshall.h"
//#include "php_qt.h"
#include "smokephp.h"
//#include "smoke.h"


Marshall::HandlerFn getMarshallFn(const SmokeType &type);

extern void smokeStackToQtStack(const Smoke::Stack stack, void ** o, const int items, MocArgument* args);
extern void smokeStackFromQtStack(const Smoke::Stack stack, void ** _o, const int items, MocArgument* args);

/**
 *	MethodReturnValueBase
 */

class MethodReturnValueBase : public Marshall
{
public:
	MethodReturnValueBase(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval** return_value_ptr);
	const Smoke::Method &method();
	Smoke::StackItem &item();
	Smoke *smoke();
	SmokeType type();
	void next();
	bool cleanup();
	void unsupported();
	zval* var();
	zval* var(zval* zval_ptr);
	zval** return_value_ptr();

protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	SmokeType _st;
	zval* _retval;
	zval **_return_value_ptr;

	virtual const char *classname();
};

/**
 *	VirtualMethodReturnValue
 */

class VirtualMethodReturnValue : public MethodReturnValueBase {
public:
	VirtualMethodReturnValue(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval* retval);
	Marshall::Action action();

private:
// 	zval _retval2;
};

/**
 *	MethodReturnValue
 */

class MethodReturnValue : public MethodReturnValueBase {
public:
    MethodReturnValue(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval *retval, zval** return_value_ptr);
    Marshall::Action action();

private:
	const char *classname();
};

/**
 *	MethodCallBase
 */

class MethodCallBase : public Marshall
{
public:
	MethodCallBase(Smoke *smoke, const Smoke::Index meth, zval** return_value_ptr);
	MethodCallBase(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval** return_value_ptr);
	Smoke *smoke();
	SmokeType type();
	Smoke::StackItem &item();
	const Smoke::Method &method();
	virtual const int items() = 0;
	virtual void callMethod() = 0;
	void next();
	void unsupported();
	zval** return_value_ptr();

protected:
	Smoke *_smoke;
	Smoke::Index _method;
	Smoke::Stack _stack;
	int _cur;
	Smoke::Index *_args;
	bool _called;
	zval ***_sp;
	virtual const char* classname();
	zval** _return_value_ptr;

};

/**
 *	VirtualMethodCall
 */

class VirtualMethodCall : public MethodCallBase {
public:
	VirtualMethodCall(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, zval* obj, zval **sp, zval** return_value_ptr);
	~VirtualMethodCall();
	Marshall::Action action();
	zval* var();
	zval* var(zval* zval_ptr);
	virtual const int items();
	void callMethod();
	bool cleanup();
	bool doAlloc() { return true; }

	bool makeObject;

private:
	zval* _obj;
 	zval **__sp;

};

/**
 *	MethodCall
 */

class MethodCall : public MethodCallBase {
public:
    MethodCall(Smoke *smoke, const Smoke::Index method, zval* target, zval ***sp, zval *retval, zval** return_value_ptr);
	~MethodCall();
	Marshall::Action action();
	zval* var();
	zval* var(zval* zval_ptr);
	void callMethod();
	virtual const int items();
	bool cleanup();

private:
	zval* _target;
	void *_current_object;
	Smoke::Index _current_object_class;
	zval ***_sp;
	int _items;
	zval *_retval;

	const char *classname();
};

class EmitSignal : public Marshall {
    QObject *_obj;
    int _id;
    zval * _result;

 public:
    EmitSignal(QObject *obj, const int id, const int items, MocArgument* mocStack, zval ***sp, zval * result);
    Marshall::Action action();
    Smoke::StackItem &item();
	const char *mytype();
	void emitSignal();
	void mainfunction();
	bool cleanup();
	void next();
	SmokeType type();
	zval* var();
	zval* var(zval* zval_ptr) { return var(); }
	void unsupported();
	Smoke* smoke();
	const MocArgument &arg();
	zval** return_value_ptr(){};

 protected:
	MocArgument *_args;
	int _cur;
	bool _called;
	Smoke::Stack _stack;
	int _items;
	zval ***_sp;
};

/**
 *	InvokeSlot
 */

class InvokeSlot : public MethodCallBase {
public:
	InvokeSlot(Smoke *smoke, const Smoke::Index meth, const Smoke::Stack stack, const zval* obj, zval **sp, zval** return_value_ptr, const int slotId, const QMetaObject* metaObject, void** a);
	~InvokeSlot();
	Marshall::Action action();
	zval* var();
	zval* var(zval* zval_ptr);
	virtual const int items();
	void callMethod();
	bool cleanup();
	bool doAlloc() { return true; }
	bool callable(const zend_class_entry* ce);
	SmokeType type();
	Smoke::StackItem& item();

	bool makeObject;

private:
	int _items;
	int _id;
	QByteArray* _slotName;
	const QMetaObject* _metaObject;
	void** _a;
	MocArgument* _mocStack;

	const zval* _obj;
 	zval **__sp;

};


#endif
