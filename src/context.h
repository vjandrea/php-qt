
#ifndef CONTEXT_H
#define CONTEXT_H

#include <QByteArray>
#include <QStack>
#include "zend.h"

/*!
 * API to access to some context settings
 */
    
class Context
{
    static Context* m_Context;

    QStack<QByteArray*> m_methodNameStack;
    zend_class_entry* m_activeCe;
    zval* m_activeScope;
    bool m_parentCall;
    
    Context(){};
    ~Context(){};

 public:
    static void createContext();
    static void destroyContext();

    static zend_class_entry* activeCe();
    static zval* activeScope();
    static bool parentCall();
    static void removeMethodName();
    static QByteArray* methodName();	    
    static const char* methodNameC();

    static void setActiveCe(zend_class_entry* activeCePtr);
    static void setActiveScope(zval* zval_ptr);
    static void setParentCall(bool pc);
    static void setMethodName(const char* name);

};

#endif

