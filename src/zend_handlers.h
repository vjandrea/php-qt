/*!
 * PHP-Qt - The PHP language bindings for Qt
 *
 * Copyright (C) 2006
 * Thomas Moenicke <tm at php-qt.org>
 * Katrina Niolet <katrina at niolet.name>
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

#ifndef ZEND_HANDLERS_H
#define ZEND_HANDLERS_H

#include <zend_interfaces.h>

// for opcode handler
#define PHPQT_OPHANDLER_COUNT				((25 * 151) + 1)
#define EX__(element) execute_data->element
#define EX_T(offset) (*(temp_variable *)((char *) EX__(Ts) + offset))

namespace ZendHandlers {

/*!
 *	handler for regular method calls
 */

union _zend_function* proxyHandler(zval **obj_ptr, char* methodName, int methodName_len TSRMLS_DC);

/*!
 *	handler for constant method calls
 *  and for the	parent:: statement too
 */

#undef EX
#define EX(element) execute_data->element

int constantMethodHandler(ZEND_OPCODE_HANDLER_ARGS);

/*!
 *	handler for constants
 */

int constantHandler(ZEND_OPCODE_HANDLER_ARGS);

/*!
 * handler for cloning objects
 */

int cloneHandler(ZEND_OPCODE_HANDLER_ARGS);

/*!
 *	install handlers
 */

void installZendHandlers();

} // namespace ZendHandlers

#endif
