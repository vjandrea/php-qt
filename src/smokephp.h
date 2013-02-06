#ifndef SMOKEPHP_H
#define SMOKEPHP_H

#include "smoke.h"

#undef DEBUG
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_POSIX
#define __USE_POSIX
#endif
#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif
#include "php.h"
#define Qnil (zval *) NULL

#include <QtCore/QStack>
#include <QtCore/QByteArray>

#include "php_qt.h"
#include "marshall.h"

//class SmokePHP;

class SmokeType {
    Smoke::Type *_t;		// derived from _smoke and _id, but cached

    Smoke *_smoke;
    Smoke::Index _id;
public:
    SmokeType() : _t(0), _smoke(0), _id(0) {}
    SmokeType(Smoke *s, Smoke::Index i) : _smoke(s), _id(i) {
	if(_id < 0 || _id > _smoke->numTypes) _id = 0;
	_t = _smoke->types + _id;
    }
    // default copy constructors are fine, this is a constant structure

    // mutators
    void set(Smoke *s, Smoke::Index i) {
	_smoke = s;
	_id = i;
	_t = _smoke->types + _id;
    }

    // accessors
    Smoke *smoke() const { return _smoke; }
    Smoke::Index typeId() const { return _id; }
    const Smoke::Type &type() const { return *_t; }
    unsigned short flags() const { return _t->flags; }
    unsigned short elem() const { return _t->flags & Smoke::tf_elem; }
    const char *name() const { return _t->name; }
    Smoke::Index classId() const { return _t->classId; }

    // tests
    bool isStack() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_stack); }
    bool isPtr() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_ptr); }
    bool isRef() const { return ((flags() & Smoke::tf_ref) == Smoke::tf_ref); }
    bool isConst() const { return (flags() & Smoke::tf_const); }
    bool isClass() const {
	if(elem() == Smoke::t_class)
	    return classId() ? true : false;
	return false;
    }
    bool isQString() const {
	if(elem() == Smoke::t_voidp && !(elem() == Smoke::t_class))
	    return true;
	return false;
    }
    bool operator ==(const SmokeType &b) const {
	const SmokeType &a = *this;
	if(a.name() == b.name()) return true;
	if(a.name() && b.name() && qstrcmp(a.name(), b.name()) == 0)
	    return true;
	return false;
    }
    bool operator !=(const SmokeType &b) const {
	const SmokeType &a = *this;
	return !(a == b);
    }

};

class SmokeClass {
    Smoke::Class *_c;
    Smoke *_smoke;
    Smoke::Index _id;
public:
    SmokeClass(const SmokeType &t) {
	_smoke = t.smoke();
	_id = t.classId();
	_c = _smoke->classes + _id;
    }
    SmokeClass(Smoke *smoke, Smoke::Index id) : _smoke(smoke), _id(id) {
	_c = _smoke->classes + _id;
    }

    Smoke *smoke() const { return _smoke; }
    const Smoke::Class &c() const { return *_c; }
    Smoke::Index classId() const { return _id; }
    const char *className() const { return _c->className; }
    Smoke::ClassFn classFn() const { return _c->classFn; }
    Smoke::EnumFn enumFn() const { return _c->enumFn; }
    bool operator ==(const SmokeClass &b) const {
	const SmokeClass &a = *this;
	if(a.className() == b.className()) return true;
	if(a.className() && b.className() && qstrcmp(a.className(), b.className()) == 0)
	    return true;
	return false;
    }
    bool operator !=(const SmokeClass &b) const {
	const SmokeClass &a = *this;
	return !(a == b);
    }
    bool isa(const SmokeClass &sc) const {
	// This is a sick function, if I do say so myself
	if(*this == sc) return true;
	Smoke::Index *parents = _smoke->inheritanceList + _c->parents;
	for(int i = 0; parents[i]; i++) {
	    if(SmokeClass(_smoke, parents[i]).isa(sc)) return true;
	}
	return false;
    }

    unsigned short flags() const { return _c->flags; }
    bool hasConstructor() const { return flags() & Smoke::cf_constructor; }
    bool hasCopy() const { return flags() & Smoke::cf_deepcopy; }
    bool hasVirtual() const { return flags() & Smoke::cf_virtual; }
    bool hasFire() const { return !(flags() & Smoke::cf_undefined); }
};

class SmokeMethod {
    Smoke::Method *_m;
    Smoke *_smoke;
    Smoke::Index _id;
public:
    SmokeMethod(Smoke *smoke, Smoke::Index id) : _smoke(smoke), _id(id) {
	_m = _smoke->methods + _id;
    }

    Smoke *smoke() const { return _smoke; }
    const Smoke::Method &m() const { return *_m; }
    SmokeClass c() const { return SmokeClass(_smoke, _m->classId); }
    const char *name() const { return _smoke->methodNames[_m->name]; }
    int numArgs() const { return _m->numArgs; }
    unsigned short flags() const { return _m->flags; }
    SmokeType arg(int i) const {
	if(i >= numArgs()) return SmokeType();
	return SmokeType(_smoke, _smoke->argumentList[_m->args + i]);
    }
    SmokeType ret() const { return SmokeType(_smoke, _m->ret); }
    Smoke::Index methodId() const { return _id; }
    Smoke::Index method() const { return _m->method; }

    bool isStatic() const { return flags() & Smoke::mf_static; }
    bool isConst() const { return flags() & Smoke::mf_const; }

    void call(Smoke::Stack args, void *ptr = 0) const {
	Smoke::ClassFn fn = c().classFn();
	(*fn)(method(), ptr, args);
    }
};

class Smoke_MAGIC {	// to be rewritten
    SmokeClass _c;
    void *_ptr;
    bool _isAllocated;
public:
    Smoke_MAGIC(void *p, const SmokeClass &c) :
	_c(c), _ptr(p), _isAllocated(false) {}
    const SmokeClass &c() const { return _c; }
    void *ptr() const { return _ptr; }
    bool isAllocated() const { return _isAllocated; }
    void setAllocated(bool isAllocated) { _isAllocated = isAllocated; }
};


/**
 * SmokeObject is a thin wrapper around zval* objects. Each SmokeObject instance
 * increments the refcount of its zval* for the duration of its existance.
 *
 * SmokeObject instances are only returned from SmokePHP, since the method
 * of binding data to the scalar must be consistent across all modules.
 */
class SmokeObject {
    zval* rv;
    Smoke_MAGIC *m;

public:
    SmokeObject(zval* obj, Smoke_MAGIC *mag) : rv(obj), m(mag) {
    }

    ~SmokeObject() {
    }

    SmokeObject(const SmokeObject &other) {
	rv = other.rv;
	m = other.m;

    }

    SmokeObject &operator =(const SmokeObject &other) {
	rv = other.rv;
	m = other.m;
	return *this;
    }

    const SmokeClass &c() { return m->c(); }
    Smoke *smoke() { return c().smoke(); }
    zval* var() { return rv; }
    void *ptr() { return m->ptr(); }
    Smoke::Index classId() { return c().classId(); }
    void *cast(const SmokeClass &toc) {
	return smoke()->cast(
	    ptr(),
	    classId(),
	    smoke()->idClass(toc.className())
	);
    }
    const char *className() { return c().className(); }

    bool isValid() const { return !(rv == Qnil); }
    bool isAllocated() const { return m->isAllocated(); }
    void setAllocated(bool i) { m->setAllocated(i); }
};


/**
 * Since it's not easy to share functions between PHP modules, the common
 * interface between all Smoked libraries and PHP will be defined in this
 * class. There will be only one SmokePHP instance loaded for an entire PHP
 * process. It has no data members here -- this is only an abstract interface.
 */

class SmokePHP {
    void *future_extension;
public:
    SmokePHP() : future_extension(0) {}

    // don't need this, we're only defining an interface
    virtual ~SmokePHP() = 0;

    /**
     * Registers a Smoke object
     */
    virtual void registerSmoke(const char *name, Smoke *smoke) = 0;

    /**
     * Gets a smoke object from its name
     */
    virtual Smoke *getSmoke(const char *name) = 0;

    /**
     * Determines if the named smoke is registered.
     */
    bool isSmokeRegistered(const char *name) { return getSmoke(name) ? true : false; }

    virtual void registerHandlers(TypeHandler *handlers) = 0;

    /**
     * Returns a new blessed SV referring to the pointer passed.
     * Use sv_2mortal() before passing it around.
     *
     * @param p pointer to the C++ object. The pointer isn't automatically deleted by SmokePerl.
     * @param c class of the pointer
     * @see #getObject
     * @see #deleteObject
     */
    virtual SmokeObject newObject(void *p, const SmokeClass &c) = 0;

    /**
     * Same as newObject(), except it doesn't treat p as owned by Perl
     */
    virtual SmokeObject wrapObject(void *p, const SmokeClass &c) = 0;

    /**
     * Any SV* created with newObject() on a class with virtual methods can be
     * retrieved again.
     */
    virtual SmokeObject getObject(void *p) = 0;

    /**
     * Create a SmokeObject from the given zval*
     */
    virtual SmokeObject getObject(zval* value) = 0;
};

/*
 * Type handling by moc is simple.
 *
 * If the type name matches /^(?:const\s+)?\Q$types\E&?$/, use the
 * static_QUType, where $types is join('|', qw(bool int double char* QString);
 *
 * Everything else is passed as a pointer! There are types which aren't
 * Smoke::tf_ptr but will have to be passed as a pointer. Make sure to keep
 * track of what's what.
 */

/*
 * Simply using typeids isn't enough for signals/slots. It will be possible
 * to declare signals and slots which use arguments which can't all be
 * found in a single smoke object. Instead, we need to store smoke => typeid
 * pairs. We also need additional informatation, such as whether we're passing
 * a pointer to the union element.
 */

enum MocArgumentType {
    xmoc_ptr,
    xmoc_bool,
    xmoc_int,
    xmoc_double,
    xmoc_charstar,
    xmoc_QString,
    xmoc_void
};

struct MocArgument {
    // smoke object and associated typeid
    SmokeType st;
    MocArgumentType argType;
};

bool                    smokephp_isQObject(const Smoke::Index classId);
void			smokephp_prepareMethodName(zval*** args, const int argc);
QByteArray* 		smokephp_getSignature(const int argc, zval ***argv, MocArgument* mocStack);
Smoke::Index		smokephp_getMethod(const char* c, const char* m, const int argc, zval*** args);
void			smokephp_prepareConnect(zval*** args, const int argc, Smoke::StackItem* qargs, const Smoke::Index method);
void			smokephp_callMethod(void *obj, Smoke::Index method, Smoke::Stack qargs);
void			smokephp_init();

void* 			transformArray(const zval* args);
void                    smokephp_QtToMoc(void** _a, const QList<QByteArray> qargs, MocArgument* moc);


#endif // SMOKEPHP_H
