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

#include "phpwriteincludes.h"
#include "driver.h"
#include "ui4.h"
#include "uic.h"
#include "databaseinfo.h"
#include <qdebug.h>
#include <QFileInfo>
#include <QTextStream>

namespace {
    enum { debugWriteIncludes = 0 };
}

namespace PHP {

struct ClassInfoEntry
{
    const char *klass;
    const char *module;
    const char *header;
};

static ClassInfoEntry qclass_lib_map[] = {
#define QT_CLASS_LIB(klass, module, header) { #klass, #module, #header },
#include "qclass_lib_map.h"
#undef QT_CLASS_LIB
    { 0, 0, 0 }
};

WriteIncludes::WriteIncludes(Uic *uic)    :
    m_uic(uic),
    m_output(uic->output()),
    m_scriptsActivated(false)
{
    for(const ClassInfoEntry *it = &qclass_lib_map[0]; it->klass != 0;  ++it) {
        QString newHeader = QLatin1String(it->module);
        newHeader += QLatin1Char('/');
        newHeader += QLatin1String(it->klass);
        m_classToHeader.insert(QLatin1String(it->klass),         newHeader);
        m_oldHeaderToNewHeader.insert(QLatin1String(it->header), newHeader);
    }
}

void WriteIncludes::acceptUI(DomUI *node)
{
    m_scriptsActivated = false;
    m_localIncludes.clear();
    m_globalIncludes.clear();
    m_knownClasses.clear();
    m_includeBaseNames.clear();

    if (node->elementIncludes())
        acceptIncludes(node->elementIncludes());

    if (node->elementCustomWidgets())
        TreeWalker::acceptCustomWidgets(node->elementCustomWidgets());

    add(QLatin1String("QApplication"));
    add(QLatin1String("QVariant"));
    add(QLatin1String("QAction"));

    add(QLatin1String("QButtonGroup")); // ### only if it is really necessary

    if (m_uic->hasExternalPixmap() && m_uic->pixmapFunction() == QLatin1String("qPixmapFromMimeSource"))
        add(QLatin1String("Q3MimeSourceFactory"));

    if (m_uic->databaseInfo()->connections().size()) {
        add(QLatin1String("QSqlDatabase"));
        add(QLatin1String("Q3SqlCursor"));
        add(QLatin1String("QSqlRecord"));
        add(QLatin1String("Q3SqlForm"));
    }

    TreeWalker::acceptUI(node);

    writeHeaders(m_globalIncludes, true);
    writeHeaders(m_localIncludes, false);

    m_output << QLatin1Char('\n');
}

void WriteIncludes::acceptWidget(DomWidget *node)
{
    add(node->attributeClass());
    TreeWalker::acceptWidget(node);
}

void WriteIncludes::acceptLayout(DomLayout *node)
{
    add(node->attributeClass());
    TreeWalker::acceptLayout(node);
}

void WriteIncludes::acceptSpacer(DomSpacer *node)
{
    add(QLatin1String("QSpacerItem"));
    TreeWalker::acceptSpacer(node);
}

void WriteIncludes::acceptProperty(DomProperty *node)
{
    if (node->kind() == DomProperty::Date)
        add(QLatin1String("QDate"));
    if (node->kind() == DomProperty::Locale)
        add(QLatin1String("QLocale"));
    TreeWalker::acceptProperty(node);
}

void WriteIncludes::insertIncludeForClass(const QString &className, QString header, bool global)
{
    if (debugWriteIncludes)
        qDebug() << "WriteIncludes::insertIncludeForClass" << className << header  << global;

    do {
        if (!header.isEmpty())
            break;

        // Known class
        const StringMap::const_iterator it = m_classToHeader.constFind(className);
        if (it != m_classToHeader.constEnd()) {
            header = it.value();
            global =  true;
            break;
        }

        // Quick check by class name to detect includehints provided for custom widgets
        const QString lowerClassName = className.toLower();
        if (m_includeBaseNames.contains(lowerClassName)) {
            header.clear();
            break;
        }

        // Last resort: Create default header
        header = lowerClassName;
        header += QLatin1String(".h");
        global = true;
    } while (false);

    if (!header.isEmpty())
        insertInclude(header, global);
}

void WriteIncludes::add(const QString &className, const QString &header, bool global)
{
    if (debugWriteIncludes)
        qDebug() << "WriteIncludes::add" << className << header  << global;

    if (className.isEmpty() || m_knownClasses.contains(className))
        return;

    m_knownClasses.insert(className);

    if (className == QLatin1String("Line")) { // ### hmm, deprecate me!
        add(QLatin1String("QFrame"));
        return;
    }

    if (m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3ListView"))  ||
        m_uic->customWidgetsInfo()->extends(className, QLatin1String("Q3Table"))) {
        add(QLatin1String("Q3Header"));
    }
    insertIncludeForClass(className, header, global);
}

void WriteIncludes::acceptCustomWidget(DomCustomWidget *node)
{
    const QString className = node->elementClass();
    if (className.isEmpty())
        return;

    if (const DomScript *domScript = node->elementScript())
        if (!domScript->text().isEmpty())
            activateScripts();

    // custom header unless it is a built-in qt class
    QString header;
    bool global = false;
    if (node->elementHeader() && !m_classToHeader.contains(className) && node->elementHeader()->text().size()) {
        global = node->elementHeader()->attributeLocation().toLower() == QLatin1String("global");
        header = node->elementHeader()->text();
    }
    add(className, header, global);
}

void WriteIncludes::acceptCustomWidgets(DomCustomWidgets *node)
{
    Q_UNUSED(node);
}

void WriteIncludes::acceptIncludes(DomIncludes *node)
{
    TreeWalker::acceptIncludes(node);
}

void WriteIncludes::acceptInclude(DomInclude *node)
{
    bool global = true;
    if (node->hasAttributeLocation())
        global = node->attributeLocation() == QLatin1String("global");
    insertInclude(node->text(), global);
}
void WriteIncludes::insertInclude(const QString &header, bool global)
{
    if (debugWriteIncludes)
        qDebug() << "WriteIncludes::insertInclude" <<  header  << global;

    OrderedSet &includes = global ?  m_globalIncludes : m_localIncludes;
    if (includes.contains(header))
        return;
    // Insert. Also remember base name for quick check of suspicious custom plugins
    includes.insert(header, false);
    const QString lowerBaseName = QFileInfo(header).completeBaseName ().toLower();
    m_includeBaseNames.insert(lowerBaseName);
}

void WriteIncludes::writeHeaders(const OrderedSet &headers, bool global)
{
    const QChar openingQuote = global ? QLatin1Char('<') : QLatin1Char('"');
    const QChar closingQuote = global ? QLatin1Char('>') : QLatin1Char('"');

    const OrderedSet::const_iterator cend = headers.constEnd();
    for ( OrderedSet::const_iterator sit = headers.constBegin(); sit != cend; ++sit) {
        const StringMap::const_iterator hit = m_oldHeaderToNewHeader.constFind(sit.key());
        const bool mapped =  hit != m_oldHeaderToNewHeader.constEnd();
        const  QString header =  mapped ? hit.value() : sit.key();
        if (!header.trimmed().isEmpty()) {
            m_output << "#include " << openingQuote << header << closingQuote << QLatin1Char('\n');
        }
    }
}

void WriteIncludes::acceptWidgetScripts(const DomScripts &scripts, DomWidget *, const  DomWidgets &)
{
    if (!scripts.empty()) {
        activateScripts();
    }
}

void WriteIncludes::activateScripts()
{
    if (!m_scriptsActivated) {
        add(QLatin1String("QScriptEngine"));
        add(QLatin1String("QDebug"));
        m_scriptsActivated = true;
    }
}
} // namespace PHP
