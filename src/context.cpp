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

#include "context.h"

Context* Context::m_Context = 0;

void Context::createContext()
{
    m_Context = new Context;
}

void Context::destroyContext()
{
    delete m_Context;
}

void
Context::setActiveCe(zend_class_entry* ce)
{
    m_Context->m_activeCe = ce;    
}

zend_class_entry* 
Context::activeCe()
{
    return m_Context->m_activeCe;
}

zval*
Context::activeScope()
{
    return m_Context->m_activeScope;
}

void
Context::setActiveScope(zval* zval_ptr)
{
    m_Context->m_activeScope = zval_ptr;
}

void
Context::setParentCall(bool pc)
{
    m_Context->m_parentCall = pc;
}

bool
Context::parentCall()
{
    return m_Context->m_parentCall;
}

void
Context::setMethodName(const char* name)
{
    m_Context->m_methodNameStack.push( new QByteArray(name) );
}

void
Context::removeMethodName()
{
    m_Context->m_methodNameStack.pop();
}

QByteArray*
Context::methodName()
{
    return m_Context->m_methodNameStack.top();
}

const char*
Context::methodNameC()
{
    return m_Context->m_methodNameStack.top()->constData();
}
