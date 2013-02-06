#ifndef QT_SMOKE_H
#define QT_SMOKE_H

#include <kdemacros.h>
// Defined in smokedata.cpp, initialized by init_qt_Smoke(), used by all .cpp files
extern KDE_EXPORT Smoke* qt_Smoke;
extern KDE_EXPORT void init_qt_Smoke();

class QGlobalSpace { };

#endif
