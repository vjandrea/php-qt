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

#include "phpwriteicondeclaration.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"

#include <QTextStream>

namespace PHP {

WriteIconDeclaration::WriteIconDeclaration(Uic *uic)
    : driver(uic->driver()), output(uic->output()), option(uic->option())
{
}

void WriteIconDeclaration::acceptUI(DomUI *node)
{
    TreeWalker::acceptUI(node);
}

void WriteIconDeclaration::acceptImages(DomImages *images)
{
    TreeWalker::acceptImages(images);
}

void WriteIconDeclaration::acceptImage(DomImage *image)
{
    QString name = image->attributeName();
    if (name.isEmpty())
        return;

    driver->insertPixmap(name);
    output << option.indent << option.indent << name << "_ID,\n";
}

} // namespace PHP
