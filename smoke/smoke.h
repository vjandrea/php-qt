#ifndef SMOKE_H
#define SMOKE_H

#include <string.h>

/*
   Copyright (C) 2002, Ashley Winters <qaqortog@nwlink.com>

    BSD License

    Redistribution and use in source and binary forms, with or
      without modification, are permitted provided that the following
      conditions are met:

    Redistributions of source code must retain the above
      copyright notice, this list of conditions and the following disclaimer.

    Redistributions in binary form must reproduce the above
      copyright notice, this list of conditions and the following
      disclaimer in the documentation and/or other materials
      provided with the distribution.>

    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
    EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
    THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
    LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
    IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
    THE POSSIBILITY OF SUCH DAMAGE.
*/

class SmokeBinding;

class Smoke {
public:
    union StackItem; // defined below
    /**
     * A stack is an array of arguments, passed to a method when calling it.
     */
    typedef StackItem* Stack;

    enum EnumOperation {
	EnumNew,
	EnumDelete,
	EnumFromLong,
	EnumToLong
    };

    typedef short Index;
    typedef void (*ClassFn)(Index method, void* obj, Stack args);
    typedef void* (*CastFn)(void* obj, Index from, Index to);
    typedef void (*EnumFn)(EnumOperation, Index, void*&, long&);

    enum ClassFlags {
        cf_constructor = 0x01,  // has a constructor
        cf_deepcopy = 0x02,     // has copy constructor
        cf_virtual = 0x04,      // has virtual destructor
        cf_undefined = 0x10     // defined elsewhere
    };
    /**
     * Describe one class.
     */
    struct Class {
	const char *className;	// Name of the class
	Index parents;		// Index into inheritanceList
	ClassFn classFn;	// Calls any method in the class
	EnumFn enumFn;		// Handles enum pointers
        unsigned short flags;   // ClassFlags
    };

    enum MethodFlags {
        mf_static = 0x01,
        mf_const = 0x02,
        mf_copyctor = 0x04,  // Copy constructor
        mf_internal = 0x08,   // For internal use only
        mf_enum = 0x10,   // An enum value
        mf_ctor = 0x20,
        mf_dtor = 0x40,
        mf_protected = 0x80
    };
    /**
     * Describe one method of one class.
     */
    struct Method {
	Index classId;		// Index into classes
	Index name;		// Index into methodNames; real name
	Index args;		// Index into argumentList
	unsigned char numArgs;	// Number of arguments
	unsigned char flags;	// MethodFlags (const/static/etc...)
	Index ret;		// Index into types for the return type
	Index method;		// Passed to Class.classFn, to call method
    };

    /**
     * One MethodMap entry maps the munged method prototype
     * to the Method entry.
     *
     * The munging works this way:
     * $ is a plain scalar
     * # is an object
     * ? is a non-scalar (reference to array or hash, undef)
     *
     * e.g. QApplication(int &, char **) becomes QApplication$?
     */
    struct MethodMap {
	Index classId;		// Index into classes
	Index name;		// Index into methodNames; munged name
	Index method;		// Index into methods
    };

    enum TypeFlags {
        // The first 4 bits indicate the TypeId value, i.e. which field
        // of the StackItem union is used.
        tf_elem = 0x0F,

	// Always only one of the next three flags should be set
	tf_stack = 0x10, 	// Stored on the stack, 'type'
	tf_ptr = 0x20,   	// Pointer, 'type*'
	tf_ref = 0x30,   	// Reference, 'type&'
	// Can | whatever ones of these apply
	tf_const = 0x40		// const argument
    };
    /**
     * One Type entry is one argument type needed by a method.
     * Type entries are shared, there is only one entry for "int" etc.
     */
    struct Type {
	const char *name;	// Stringified type name
	Index classId;		// Index into classes. -1 for none
        unsigned short flags;   // TypeFlags
    };

    // We could just pass everything around using void* (pass-by-reference)
    // I don't want to, though. -aw
    union StackItem {
	void* s_voidp;
	bool s_bool;
	signed char s_char;
	unsigned char s_uchar;
	short s_short;
	unsigned short s_ushort;
	int s_int;
	unsigned int s_uint;
	long s_long;
	unsigned long s_ulong;
	float s_float;
	double s_double;
        long s_enum;
        void* s_class;
    };
    enum TypeId {
	t_voidp,
	t_bool,
	t_char,
	t_uchar,
	t_short,
	t_ushort,
	t_int,
	t_uint,
	t_long,
	t_ulong,
	t_float,
	t_double,
        t_enum,
        t_class,
	t_last		// number of pre-defined types
    };

    // Passed to constructor
    /**
     * The classes array defines every class for this module
     */
    Class *classes;
    Index numClasses;

    /**
     * The methods array defines every method in every class for this module
     */
    Method *methods;
    Index numMethods;

    /**
     * methodMaps maps the munged method prototypes
     * to the methods entries.
     */
    MethodMap *methodMaps;
    Index numMethodMaps;

    /**
     * Array of method names, for Method.name and MethodMap.name
     */
    const char **methodNames;
    Index numMethodNames;

    /**
     * List of all types needed by the methods (arguments and return values)
     */
    Type *types;
    Index numTypes;

    /**
     * Groups of class IDs (-1 separated) used as super class lists.
     * For classes with super classes: Class.parents = index into this array.
     */
    Index *inheritanceList;
    /**
     * Groups of type IDs (-1 separated), describing the types of argument for a method.
     * Method.args = index into this array.
     */
    Index *argumentList;
    /**
     * Groups of method prototypes with the same number of arguments, but different types.
     * Used to resolve overloading.
     */
    Index *ambiguousMethodList;
    /**
     * Function used for casting from/to the classes defined by this module.
     */
    CastFn castFn;

    // Not passed to constructor
    SmokeBinding *binding;

    /**
     * Constructor
     */
    Smoke(Class *_classes, Index _numClasses,
	  Method *_methods, Index _numMethods,
	  MethodMap *_methodMaps, Index _numMethodMaps,
	  const char **_methodNames, Index _numMethodNames,
	  Type *_types, Index _numTypes,
	  Index *_inheritanceList,
	  Index *_argumentList,
	  Index *_ambiguousMethodList,
	  CastFn _castFn) :
		classes(_classes), numClasses(_numClasses),
		methods(_methods), numMethods(_numMethods),
		methodMaps(_methodMaps), numMethodMaps(_numMethodMaps),
		methodNames(_methodNames), numMethodNames(_numMethodNames),
		types(_types), numTypes(_numTypes),
		inheritanceList(_inheritanceList),
		argumentList(_argumentList),
		ambiguousMethodList(_ambiguousMethodList),
		castFn(_castFn),

		binding(0)
		{}

    inline void *cast(void *ptr, Index from, Index to) {
	if(!castFn) return ptr;
	return (*castFn)(ptr, from, to);
    }

    // return classname directly
    inline const char *className(Index classId) {
	return classes[classId].className;
    }

    inline int leg(Index a, Index b) {  // ala Perl's <=>
	if(a == b) return 0;
	return (a > b) ? 1 : -1;
    }

    inline Index idType(const char *t) {
	if(!t) return 0;
	Index imax = numTypes;
	Index imin = 0;
	Index icur = -1;
	int icmp = -1;

	while(imax >= imin) {
	    icur = (imin + imax) / 2;
	    if(icur > 0)
		icmp = strcmp(types[icur].name, t);
	    else
		icmp = -1;
	    if(!icmp) break;
	    if(icmp > 0)
		imax = icur - 1;
	    else
		imin = icur + 1;
	}

	return (!icmp) ? icur : 0;
    }

    inline Index idClass(const char *c) {
	if(!c) return 0;
	Index imax = numClasses;
	Index imin = 0;
	Index icur = -1;
	int icmp = -1;

	while(imax >= imin) {
	    icur = (imin + imax) / 2;
	    if(icur > 0)
		icmp = strcmp(classes[icur].className, c);
	    else
		icmp = -1;
	    if(!icmp) break;
	    if(icmp > 0)
		imax = icur - 1;
	    else
		imin = icur + 1;
	}

	return (!icmp) ? icur : 0;
    }

    inline Index idMethodName(const char *m) {
	if(!m) return 0;
	Index imax = numMethodNames;
	Index imin = 0;
	Index icur = -1;
	int icmp = -1;
	while(imax >= imin) {
	    icur = (imin + imax) / 2;
	    icmp = strcmp(methodNames[icur], m);
	    if(!icmp) break;
	    if(icmp > 0)
		imax = icur - 1;
	    else
		imin = icur + 1;
	}

	return (!icmp) ? icur : 0;
    }

    inline Index idMethod(Index c, Index name) {
	Index imax = numMethodMaps;
	Index imin = 0;
	Index icur = -1;
	int icmp = -1;
	while(imax >= imin) {
	    icur = (imin + imax) / 2;
	    icmp = leg(methodMaps[icur].classId, c);
	    if(!icmp) {
		icmp = leg(methodMaps[icur].name, name);
		if(!icmp) break;
	    }
	    if(icmp > 0)
		imax = icur - 1;
	    else
		imin = icur + 1;
	}

	return (!icmp) ? icur : 0;
    }

    inline Index findMethod(Index c, Index name) {
	// TODO: If method is in a parent module, forward the call from here
	if(!c || !name) return 0;
	Index mid = idMethod(c, name);
	if(mid) return mid;
	if(!classes[c].parents) return 0;
	for(int p = classes[c].parents; inheritanceList[p] ; p++) {
	    mid = findMethod(inheritanceList[p], name);
	    if(mid) return mid;
	}
	return 0;
    }

    inline Index findMethod(const char *c, const char *name) {
	Index idc = idClass(c);
	Index idname = idMethodName(name);
	return findMethod(idc, idname);
    }
};

class SmokeBinding {
protected:
    Smoke *smoke;
public:
    SmokeBinding(Smoke *s) : smoke(s) {}
    virtual void deleted(Smoke::Index classId, void *obj) = 0;
    virtual bool callMethod(Smoke::Index method, void *obj, Smoke::Stack args, bool isAbstract = false) = 0;
    virtual char* className(Smoke::Index classId) = 0;
    virtual ~SmokeBinding() {}
};

#endif
