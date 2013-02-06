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

#ifndef OPTION_H
#define OPTION_H

#include <QString>

struct Option
{
    enum Generator
    {
	PhpGenerator,
        CppGenerator,
        JavaGenerator
    };

    unsigned int headerProtection : 1;
    unsigned int copyrightHeader : 1;
    unsigned int generateImplemetation : 1;
    unsigned int generateNamespace : 1;
    unsigned int autoConnection : 1;
    unsigned int dependencies : 1;
    unsigned int extractImages : 1;
    Generator generator;

    QString inputFile;
    QString outputFile;
    QString qrcOutputFile;
    QString indent;
    QString prefix;
    QString postfix;
    QString translateFunction;
    QString uic3;
#ifdef QT_UIC_JAVA_GENERATOR
    QString javaPackage;
    QString javaOutputDirectory;
#endif

    Option()
        : headerProtection(1),
          copyrightHeader(1),
          generateImplemetation(0),
          generateNamespace(1),
          autoConnection(1),
          dependencies(0),
          extractImages(0),
          generator(PhpGenerator),
          prefix(QLatin1String("Ui_"))
    { indent.fill(QLatin1Char(' '), 4); }
};

#endif // OPTION_H
