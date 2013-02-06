/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "validator.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"

#include <QTextStream>

Validator::Validator(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
    this->uic = uic;
}

void Validator::acceptUI(DomUI *node)
{
    TreeWalker::acceptUI(node);
}

void Validator::acceptWidget(DomWidget *node)
{
    (void) driver->findOrInsertWidget(node);

    TreeWalker::acceptWidget(node);
}

void Validator::acceptLayoutItem(DomLayoutItem *node)
{
    (void) driver->findOrInsertLayoutItem(node);

    TreeWalker::acceptLayoutItem(node);
}

void Validator::acceptLayout(DomLayout *node)
{
    (void) driver->findOrInsertLayout(node);

    TreeWalker::acceptLayout(node);
}

void Validator::acceptActionGroup(DomActionGroup *node)
{
    (void) driver->findOrInsertActionGroup(node);

    TreeWalker::acceptActionGroup(node);
}

void Validator::acceptAction(DomAction *node)
{
    (void) driver->findOrInsertAction(node);

    TreeWalker::acceptAction(node);
}
