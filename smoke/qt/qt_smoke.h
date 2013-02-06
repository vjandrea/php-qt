#ifndef QT_SMOKE_H
#define QT_SMOKE_H

// Don't use kdemacros.h/KDE_EXPORT here as it needs to be free of KDE dependencies
#include <QtCore/qglobal.h>
// Defined in smokedata.cpp, initialized by init_qt_Smoke(), used by all .cpp files
extern Q_DECL_EXPORT Smoke* qt_Smoke;
extern Q_DECL_EXPORT void init_qt_Smoke();

class QGlobalSpace { };

#endif
