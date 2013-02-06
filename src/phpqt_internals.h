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

#ifndef PHPQT_INTERNALS_H
#define PHPQT_INTERNALS_H

class QMetaObject;
class QString;

#include "php_qt.h"

namespace PHPQt
{

void 				destroyHashtable(zend_rsrc_list_entry *rsrc);

zval* 				callPHPMethod(const zval* zend_ptr, const char* methodname, const zend_uint param_count, zval** params);
bool 				methodExists(const zend_class_entry* ce_ptr, const char* methodname);
bool 				getMocData(zval* this_ptr, const char* classname, const QMetaObject* superdata,  QString* meta_stringdata, uint* signature);
void                            createMetaObject(smokephp_object* o, zval* this_ptr);
int                             metacall(smokephp_object* this_ptr, Smoke::StackItem* args, QMetaObject::Call _c, int _id, void **_a);
const char*			checkForOperator(const char* fname);

void* 				getQtObjectFromZval(const zval* this_ptr);
smokephp_object* 	        getSmokePHPObjectFromZval(const zval* this_ptr);
smokephp_object*	        getSmokePHPObjectFromQt(const void* QtPtr);
void				setSmokePHPObject(smokephp_object* o);
bool 				SmokePHPObjectExists(const zval* this_ptr);
bool				SmokePHPObjectExists(const void* ptr);

bool				unmapSmokePHPObject(const zval* o);

smokephp_object*	        createObject(zval* zval_ptr, const void* ptr, const zend_class_entry* ce = 0, const Smoke::Index classId = 0);
smokephp_object*                cloneObject(zval* zval_ptr, smokephp_object* so, const void* copyPtr);
smokephp_object*	        createOriginal(zval* zval_ptr, void* ptr);

/*!
 * API to access to some context settings
 */
/*
 class Context
     {

	 static Context* m_Context;

	 QStack<QByteArray*> m_methodNameStack;
	 zend_class_entry* m_activeCe;
	 zval* m_activeScope;
	 bool m_parentCall;

	 Context();
	 ~Context();
     public:
	 static void createContext(){ m_Context = new Context;  }
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

     }; // class PHPQt::Context
*/
}; // namespace PHPQt

#endif
