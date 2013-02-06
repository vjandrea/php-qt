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

#include <QDataStream>
#include "phpextractimages.h"
#include "phpwriteicondata.h"
#include "driver.h"
#include "ui4.h"
#include "utils.h"
#include "uic.h"

#include <QTextStream>
#include <QTextCodec>
#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace PHP {

ExtractImages::ExtractImages(const Option &opt)
    : m_output(0), m_option(opt)
{
}

void ExtractImages::acceptUI(DomUI *node)
{
    if (!m_option.extractImages)
        return;

    if (node->elementImages() == 0)
        return;

    QString className = node->elementClass() + m_option.postfix;

    QFile f;
    if (m_option.qrcOutputFile.size()) {
        f.setFileName(m_option.qrcOutputFile);
        if (!f.open(QIODevice::WriteOnly | QFile::Text)) {
            fprintf(stderr, "Could not create resource file\n");
            return;
        }

        QFileInfo fi(m_option.qrcOutputFile);
        QDir dir = fi.absoluteDir();
        if (!dir.exists("images") && !dir.mkdir("images")) {
            fprintf(stderr, "Could not create image dir\n");
            return;
        }
        dir.cd("images");
        m_imagesDir = dir;

        m_output = new QTextStream(&f);
        m_output->setCodec(QTextCodec::codecForName("UTF-8"));

        QTextStream &out = *m_output;

        out << "<RCC>\n";
        out << "    <qresource prefix=\"/" << className << "\" >\n";
        TreeWalker::acceptUI(node);
        out << "    </qresource>\n";
        out << "</RCC>\n";

        f.close();
        delete m_output;
        m_output = 0;
    }
}

void ExtractImages::acceptImages(DomImages *images)
{
    TreeWalker::acceptImages(images);
}

void ExtractImages::acceptImage(DomImage *image)
{
    QString format = image->elementData()->attributeFormat();
    QString extension = format.left(format.indexOf('.')).toLower();
    QString fname = m_imagesDir.absoluteFilePath(image->attributeName() + QLatin1Char('.') + extension);

    *m_output << "        <file>images/" << image->attributeName() << QLatin1Char('.') + extension << "</file>\n";

    QFile f;
    f.setFileName(fname);
    if (!f.open(QIODevice::WriteOnly | QIODevice::Text)) {
        fprintf(stderr, "Could not create image file\n");
        return;
    }

    if (format == QLatin1String("XPM.GZ")) {
        QTextStream *imageOut = new QTextStream(&f);
        imageOut->setCodec(QTextCodec::codecForName("UTF-8"));

        PHP::WriteIconData::writeImage(*imageOut, QString(), image);
        delete imageOut;
    } else {
        PHP::WriteIconData::writeImage(f, image);
    }

    f.close();
}

} // namespace PHP
