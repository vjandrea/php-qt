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

#include "databaseinfo.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"

DatabaseInfo::DatabaseInfo(Driver *drv)
    : driver(drv)
{
}

void DatabaseInfo::acceptUI(DomUI *node)
{
    m_connections.clear();
    m_cursors.clear();
    m_fields.clear();

    TreeWalker::acceptUI(node);

    m_connections = unique(m_connections);
}

void DatabaseInfo::acceptWidget(DomWidget *node)
{
    QHash<QString, DomProperty*> properties = propertyMap(node->elementProperty());

    DomProperty *frameworkCode = properties.value(QLatin1String("frameworkCode"), 0);
    if (frameworkCode && toBool(frameworkCode->elementBool()) == false)
        return;

    DomProperty *db = properties.value(QLatin1String("database"), 0);
    if (db && db->elementStringList()) {
        QStringList info = db->elementStringList()->elementString();

        QString connection = info.size() > 0 ? info.at(0) : QString();
        if (connection.isEmpty())
            return;
        m_connections.append(connection);

        QString table = info.size() > 1 ? info.at(1) : QString();
        if (table.isEmpty())
            return;
        m_cursors[connection].append(table);

        QString field = info.size() > 2 ? info.at(2) : QString();
        if (field.isEmpty())
            return;
        m_fields[connection].append(field);
    }

    TreeWalker::acceptWidget(node);
}

