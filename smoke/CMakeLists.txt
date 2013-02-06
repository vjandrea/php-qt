OPTION(ENABLE_SMOKE "build Smoke" ON)
OPTION(ENABLE_SMOKEKDE "build SmokeKDE" ON)
OPTION(ENABLE_SMOKEPLASMA "build SmokePlasma" OFF)

set(SMOKE_ENABLED "no")
if(ENABLE_SMOKE OR ENABLE_SMOKEKDE)
    add_subdirectory(qt)
    if(ENABLE_SMOKEKDE)
        add_subdirectory(kde)
        set(SMOKE_ENABLED "yes - Qt and KDE")
    else(ENABLE_SMOKEKDE)
        set(SMOKE_ENABLED "yes - Qt only")
    endif(ENABLE_SMOKEKDE)

endif(ENABLE_SMOKE OR ENABLE_SMOKEKDE)

if(ENABLE_SMOKEPLASMA)
    add_subdirectory(plasma)
endif(ENABLE_SMOKEPLASMA)

message(STATUS "Build Smoke... " ${SMOKE_ENABLED})

########### install files ###############

# KDE specific
#install( FILES  smoke.h DESTINATION ${INCLUDE_INSTALL_DIR} )

# Qt only
#install( FILES  smoke.h DESTINATION ${INSTALL_DIR}/include )

install( FILES  smoke.h DESTINATION ${CMAKE_INSTALL_PREFIX}/include )
