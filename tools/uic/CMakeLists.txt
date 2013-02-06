include_directories( ${CMAKE_CURRENT_SOURCE_DIR} )
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/php )

########### next target ###############

set(uic_bin_SRCS 
   customwidgetsinfo.cpp 
   databaseinfo.cpp 
   driver.cpp 
   treewalker.cpp 
   ui4.cpp 
   uic.cpp 
   validator.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpextractimages.cpp # remove
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwritedeclaration.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwriteincludes.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwriteicondata.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwriteicondeclaration.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwriteiconinitialization.cpp 
   ${CMAKE_CURRENT_SOURCE_DIR}/php/phpwriteinitialization.cpp 
   main.cpp )

add_executable(uic_bin ${uic_bin_SRCS})
set_target_properties(uic_bin PROPERTIES OUTPUT_NAME uicp)
set_target_properties(uic_bin PROPERTIES COMPILE_FLAGS -DQT_UIC_PHP_GENERATOR)

target_link_libraries(uic_bin  ${QT_QTCORE_LIBRARY} ${QT_QTXML_LIBRARY} )

install(TARGETS uic_bin DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)


########### next target ###############


########### install files ###############
