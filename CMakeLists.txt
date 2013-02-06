
PROJECT(PHP-Qt)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/modules)
set(kdebindings_SOURCE_DIR ${PHP-Qt_SOURCE_DIR})


find_package(Perl REQUIRED)	# for kalyptus
find_package(Qt4 REQUIRED)	# for kalyptus

INCLUDE_DIRECTORIES (${QT_INCLUDES})

include(FindPHP5)

IF(NOT PHP5_FOUND)
    MESSAGE(FATAL_ERROR "PHP5 could not be found!")
ENDIF(NOT PHP5_FOUND)

OPTION(ENABLE_SMOKEKDE "build SmokeKDE" OFF)
OPTION(ENABLE_SMOKEPLASMA "build SmokePLASMA" OFF)
add_subdirectory(smoke)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(tools)



