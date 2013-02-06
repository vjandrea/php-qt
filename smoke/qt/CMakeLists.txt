
include(FindQScintilla)
include(FindQwt5)

include_directories( ${QT_INCLUDES}  ${CMAKE_CURRENT_SOURCE_DIR}/../ )

set(_smokeqt_optional_libs)
if(QSCINTILLA_FOUND)
    message(STATUS "Including QScintilla support into smokeqt")
    include_directories( ${QSCINTILLA_INCLUDE_DIR} )
    set(_smokeqt_optional_libs ${_smokeqt_optional_libs} ${QSCINTILLA_LIBRARIES})
else(QSCINTILLA_FOUND)
    message(STATUS "QScintilla not found, support disabled")
endif(QSCINTILLA_FOUND)

if(Qwt5_Qt4_FOUND)
    message(STATUS "Including Qwt5 support into smokeqt")
    include_directories( ${Qwt5_INCLUDE_DIR} )
    set(_smokeqt_optional_libs ${_smokeqt_optional_libs} ${Qwt5_Qt4_LIBRARY})
else(Qwt5_Qt4_FOUND)
    message(STATUS "Qwt5 not found, support disabled")
endif(Qwt5_Qt4_FOUND)

IF(NOT PERL_FOUND)
    FIND_PACKAGE(PERL REQUIRED)
ENDIF(NOT PERL_FOUND)

# for qtguess.pl
set(qt_test_threshold 10)

# macro is defined in qt-copy/src/qbase.pri but couldn't be found by qtguess, so I append it manually
file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/qtdefines "QT_NO_CAST_TO_ASCII\n")

########### next target ###############

add_custom_command(OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_2.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_3.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_4.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_5.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_6.cpp 
		${CMAKE_CURRENT_BINARY_DIR}/x_7.cpp 
                ${CMAKE_CURRENT_BINARY_DIR}/x_8.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_9.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_10.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_11.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_12.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_13.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_14.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_15.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_16.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_17.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_18.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_19.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_20.cpp

                  COMMAND ${PERL_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/qtguess.pl
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/qtguess.pl

                  COMMAND ${PERL_EXECUTABLE} ARGS ${CMAKE_CURRENT_BINARY_DIR}/generate.pl
                  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/generate.pl

                  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

macro_add_file_dependencies( ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp )

include_directories (${CMAKE_SOURCE_DIR}/smoke ${CMAKE_SOURCE_DIR}/smoke/qt  )

set(smokeqt_LIB_SRCS ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp  
                ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp 
                ${CMAKE_CURRENT_BINARY_DIR}/x_2.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_3.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_4.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_5.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_6.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_7.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_8.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_9.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_10.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_11.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_12.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_13.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_14.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_15.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_16.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_17.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_18.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_19.cpp
                ${CMAKE_CURRENT_BINARY_DIR}/x_20.cpp
)

# kde4_add_library(smokeqt SHARED ${smokeqt_LIB_SRCS})
add_library(smokeqt SHARED ${smokeqt_LIB_SRCS})

SET(QT_USE_QTNETWORK ${QT_QTNETWORK_FOUND})
SET(QT_USE_QTOPENGL ${QT_QTOPENGL_FOUND})
SET(QT_USE_QTSQL ${QT_QTSQL_FOUND})
SET(QT_USE_QTXML ${QT_QTXML_FOUND})
SET(QT_USE_QTSVG ${QT_QTSVG_FOUND})
SET(QT_USE_QTUITOOLS ${QT_QTUITOOLS_FOUND})
SET(QT_USE_QT3SUPPORT ${QT_QT3SUPPORT_FOUND})
#SET(QT_USE_QTDBUS ${QT_QTDBUS_FOUND)
# The above line doesn't work with cmake 2.4.6, but should be fixed in later versions

INCLUDE(${QT_USE_FILE})
# At this point, QT_LIBRARIES will contain a list of the found Qt Libs
# and QT_INCLUDES contains the directories of the found includes

FOREACH(filename ${QT_INCLUDES})
    set(all_includes "-I${filename} ${all_includes}")
ENDFOREACH(filename)

FOREACH(incname ${QT_INCLUDES})
    SET(qt_incs "'${incname}', ${qt_incs}")
ENDFOREACH(incname)

FOREACH(libname ${QT_LIBRARIES})
    set(all_libs "${libname} ${all_libs}")
ENDFOREACH(libname)

target_link_libraries(smokeqt ${QT_QTCORE_LIBRARY}
                              ${QT_QTGUI_LIBRARY}
                              ${QT_QTNETWORK_LIBRARY}
                              ${QT_QTOPENGL_LIBRARY}
                              ${QT_QTDBUS_LIBRARY}
                              ${QT_QTSQL_LIBRARY}
                              ${QT_QTSVG_LIBRARY}
                              ${QT_QTUITOOLS_LIBRARY}
                              ${QT_QTXML_LIBRARY}
                              ${_smokeqt_optional_libs} )

set_target_properties(smokeqt PROPERTIES VERSION 2.0.0 SOVERSION 2 )

install(TARGETS smokeqt DESTINATION ${CMAKE_INSTALL_PREFIX}/lib )

configure_file(qtguess.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/qtguess.pl @ONLY)
configure_file(generate.pl.cmake ${CMAKE_CURRENT_BINARY_DIR}/generate.pl @ONLY )
