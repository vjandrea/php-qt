Readme
======

This is just a clone of php-qt, originally hosted here: http://php-qt.berlios.de/

The project looks abandoned so i'm trying to make it work again, at the moment it doesn't compile on QT 5.0


===
PHP-Qt - an extension for PHP5 that enables the writing of PHP software using the Qt Toolkit.
===

================================================================================

PHP-Qt is an extension for PHP5 that provides an object-oriented interface to 
the Qt4 Framework and allows 
one to write Qt applications in the PHP language.

Requirements
============

PHP 5.1 or greater (5.2 or greater is recommended)
PHP header files and the development scripts
(package may be called php-devel)

Qt4 header files
(package may be called qt4-devel)

CMake

Note: at this time compilation has only been tested in a Unix environment.

Compiling and installation
==========================

Both of these methods will compile and install PHP-Qt as a module.

The quick and easy way:

1) change to the directory where you placed the php-qt sources
2) create a directory here named 'build' and change into it
3) run 'cmake ..' (see Troubleshooting below if you have more than one
   version of Qt installed or you get errors here)
4) run 'make'
5) run 'make install'
6) run 'ldconfig'
7) add the line 'extension=php_qt.so' to your php.ini


The more detailed way:

First make sure CMake, your PHP header files, and your Qt header files are
installed.
Then, get PHP-Qt either by downloading or checking out from SVN and placing it 
into its own directory.

Next it is necessary to create your build environment for compiling.
To do this, just change into the directory where you placed PHP-Qt and 
create a directory named 'build'
run:

cmake ..

If needed you can give arguments to CMake via the -D option, such as:
  -DQT_INCLUDE_DIR=/usr/lib/qt4/include
  -DQT_LIBRARY_DIR=/usr/lib/qt4
  -DQT_QMAKE_EXECUTABLE=/usr/lib/qt4/bin/qmake
  
If no arguments are given, CMake will try to find the Qt4 and PHP5 files in 
the default or standard paths such as /usr/include, /usr/local/include etc.

Next, run:

make

to build the extension. If the compilation is successful run:

make install

to copy 'php_qt.so' into php's extension directory. If it fails please check 
if you have permissions and all related paths on your system are correct.

To update the cache of the run-time linker run:

ldconfig

so that the shared libraries will be found when starting PHP.

Edit your php.ini configuration file and add the line:

extension=php_qt.so

to enable the extension.

You can test whether your installation was successful by executing:

php -i | grep -i qt

You should get an output like this:

php_qt
PHP-Qt support => enabled
qt.codec => UTF8 => UTF8

To remove old objectfiles run:

make clean

To test it, simply run one of the test scripts in the php_qt/test/ 
subdirectory.

Troubleshooting
===================================

If you get the following error:
CMake Error: Could NOT find QtCore. 

Try running cmake as:
cmake ..  -DQT_QMAKE_EXECUTABLE=/usr/lib/qt4/bin/qmake
(you may need to replace the /usr/lib/qt4/bin/qmake if yours is in another 
place)

For other problems try checking CMakeFiles/CMakeError.log for more details.

Hint: enable debugging with
cmake .. -DCMAKE_BUILD_TYPE=debug

Mailing lists
=============

user discussion
-----------------
users mailing list

address:      php-qt-users@lists.sourceforge.net
Subscription: https://lists.sourceforge.net/lists/listinfo/php-qt-users
Archive:      http://sourceforge.net/mailarchive/forum.php?forum=php-qt-users

Website
=======

The website for the project:

    http://www.php-qt.org
