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

#ifndef CPPEXTRACTIMAGES_H
#define CPPEXTRACTIMAGES_H

#include "treewalker.h"
#include <QtCore/QDir>

class QTextStream;
class Driver;
class Uic;

struct Option;

namespace PHP {

class ExtractImages : public TreeWalker
{
public:
    ExtractImages(const Option &opt);

    void acceptUI(DomUI *node);
    void acceptImages(DomImages *images);
    void acceptImage(DomImage *image);

private:
    QTextStream *m_output;
    const Option &m_option;
    QDir m_imagesDir;
};

} // namespace PHP

#endif // CPPEXTRACTIMAGES_H
