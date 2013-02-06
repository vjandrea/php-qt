#ifndef MARSHALL_H
#define MARSHALL_H

#include "smoke.h"
#include <php.h>

class SmokeType;

class Marshall {
public:
    /**
     * FromZVAL is used for virtual function return ce_ptrs and regular
     * method arguments.
     *
     * ToZVAL is used for method return-values and virtual function
     * arguments.
     */
    typedef void (*HandlerFn)(Marshall *);
    enum Action { FromZVAL, ToZVAL };
    virtual SmokeType type() = 0;
    virtual Action action() = 0;
    virtual Smoke::StackItem &item() = 0;
    virtual zval* var() = 0;
	virtual zval* var(zval* zval_ptr) = 0;
    virtual void unsupported() = 0;
    virtual Smoke *smoke() = 0;
    virtual bool doAlloc() { return false; }
    /**
     * For return-values, next() does nothing.
     * For FromRV, next() calls the method and returns.
     * For ToRV, next() calls the virtual function and returns.
     *
     * Required to reset Marshall object to the state it was
     * before being called when it returns.
     */
    virtual void next() = 0;
    /**
     * For FromSV, cleanup() returns false when the handler should free
     * any allocated memory after next().
     *
     * For ToSV, cleanup() returns true when the handler should delete
     * the pointer passed to it.
     */
    virtual bool cleanup() = 0;

    virtual ~Marshall() {}

	/** returns a pointer to the return value pointer, we need that for
	 *	overriding the return value defined by ZEND
	 */
	virtual zval** return_value_ptr() = 0;

};

class SmokeEnumWrapper {
public:
	Marshall *m;
};

class SmokeClassWrapper {
public:
  Marshall *m;
};

struct TypeHandler {
    const char *name;
    Marshall::HandlerFn fn;
};

class smokephp_object;
void* construct_copy(smokephp_object* o);

#endif
