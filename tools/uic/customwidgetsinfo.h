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

#ifndef CUSTOMWIDGETSINFO_H
#define CUSTOMWIDGETSINFO_H

#include "treewalker.h"
#include <QStringList>
#include <QMap>

class Driver;
class DomScript;

class CustomWidgetsInfo : public TreeWalker
{
public:
    CustomWidgetsInfo();

    void acceptUI(DomUI *node);

    void acceptCustomWidgets(DomCustomWidgets *node);
    void acceptCustomWidget(DomCustomWidget *node);

    inline QStringList customWidgets() const
    { return m_customWidgets.keys(); }

    inline bool hasCustomWidget(const QString &name) const
    { return m_customWidgets.contains(name); }

    inline DomCustomWidget *customWidget(const QString &name) const
    { return m_customWidgets.value(name); }

    DomScript *customWidgetScript(const QString &name) const;

    QString realClassName(const QString &className) const;

    bool extends(const QString &className, const QString &baseClassName) const;

private:
    typedef QMap<QString, DomCustomWidget*> NameCustomWidgetMap;
    NameCustomWidgetMap m_customWidgets;
    bool m_scriptsActivated;
};

#endif // CUSTOMWIDGETSINFO_H
