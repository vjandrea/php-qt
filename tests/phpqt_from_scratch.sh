#!/bin/bash

#############################################################
#
# PHP-Qt - (C) 2007 Thomas Moenicke
# please report bugs to tm at php-qt dot org
#
# this script builds and installs everything from scratch 
# in a sandbox, also the latest PHP-Qt from svn
# The goal is to get a clean environment for testing
#
# you will get: 
#      libxml2
#      dbus
#      CMake
#      Qt 
#      PHP
#      PHPUnit
#      PHP-Qt (SVN)
#
# you need: bash, svn, wget, dbus (headers + binaries)
# dev and libs:
#  dbus (>0.63) 
#  libpng
#  opengl
#
# usage: look at the following lines and comment out 
# things you already have in your system
#
# * if you want to use your own Qt, make sure QTDIR and PATH
#   are set correctly
# * for your own PHP, make sure php-config is in PATH and
#   take care for phpunit (comment it out too if you dont
#   want it to be installed by this script)
# * if you keep the downloads directory, the archive files 
#   wont be downloaded again
# * remove all other subdirs to get a clean environment
#
#############################################################

# comment out here
build_additionals=true
build_cmake=true
build_qt=true # takes a long time
build_php=true
build_phpunit=true
build_phpqt=true

#############################################################
#
# further configurations
#

mkdir -p build
cd build

cmake_version='cmake-2.4.6'
cmake_url='http://www.cmake.org/files/v2.4/'$cmake_version'.tar.gz'

qt_version='qt-x11-opensource-src-4.3.0'
# qt_url='ftp://ftp.fu-berlin.de/unix/X11/gui/Qt/source/'$qt_version'.tar.gz' # Berlin
# qt_url='http://ftp.silug.org/mirrors/ftp.trolltech.com/qt/source/'$qt_version'.tar.gz' # Illinois
qt_url='http://wftp.tu-chemnitz.de/pub/Qt/qt/source/'$qt_version'.tar.gz' # Chemnitz

php_version='php-5.2.3'
# php_url='http://de3.php.net/get/'$php_version'.tar.gz/from/us.php.net/mirror' # US
php_url='http://de3.php.net/get/'$php_version'.tar.gz/from/this/mirror' # EU
php_configureoptions='--enable-cli --with-libxml-dir=$opt --with-pear' # not necessary

phpunit_server='pear.phpunit.de'

phpqt_url='svn://anonsvn.kde.org/home/kde/trunk/playground/bindings/phpqt'

# variables
timestamp=$(date +"%Y%m%dT%H%M%SZ")
echo "using timestamp "$timestamp

workdir=$PWD
downloads=$workdir/downloads
opt=$workdir/opt
workbench=$workdir/workbench
log=$workdir/log/$timestamp
errorlog=$workdir/log/$timestamp/error

# prepare
mkdir -p $downloads
mkdir -p $opt
mkdir -p $workbench
mkdir -p log
mkdir -p $log
#mkdir $errorlog

#############################################################
#
# create environment
#

echo "
#!/bin/bash

export PATH=$opt/bin:$PATH
export LD_LIBRARY_PATH=$opt/lib:$LD_LIBRARY_PATH
# export CXX=icecc # icecream
export PKG_CONFIG_PATH=$opt/lib/pkgconfig:$PKG_CONFIG_PATH
" > $workdir/sourceme.sh

source $workdir/sourceme.sh
env > $log/env.log

#############################################################
#
# logging
#
module=''
count=0
function runCommand() {
    count=$(($count+1))
    print1=`echo $1 | sed 's/\.\///'`$count # for a usable name
    $* > $log/$module'_'$print1.log 2> $log/$module'_'$print1'_error'.log
}

function buildSimple() {
    module=$1
    moduleVersion=$2
    moduleUrl=$3
    echo "Getting "$module $moduleVersion
    cd $downloads;
    if [ -r $downloads/$moduleVersion.tar.gz ]
	then
	echo "already downloaded, reusing file"
	else
	wget $moduleUrl
	fi

    echo "Unpacking "$module
    cd $workbench
    runCommand tar -xzvf $downloads/$moduleVersion.tar.gz

    echo "Building "$module
    cd $moduleVersion
    runCommand ./configure --prefix=$opt
    runCommand make
    runCommand make install
    cd $workdir    
}

#############################################################
#
# time to build that all, at first:
#
# name filename url

if [ $build_additionals ]
then
    buildSimple libxml libxml2-2.6.29 ftp://xmlsoft.org/libxml2/libxml2-2.6.29.tar.gz          
    buildSimple dbus dbus-1.0.2 http://dbus.freedesktop.org/releases/dbus/dbus-1.0.2.tar.gz
fi

# get and build cmake
if [ $build_cmake ]
then
    module='CMake'
    echo "Getting cmake"
    cd $downloads;
    if [ -r $downloads/$cmake_version.tar.gz ]
	then
	echo "already downloaded, reusing file"
	else
	wget $cmake_url
	fi

    echo "Unpacking CMake"
    cd $workbench
    runCommand tar -xzvf $downloads/$cmake_version'.tar.gz'

    cd $cmake_version
    echo "Configuring CMake"
    runCommand ./configure

    echo "Building CMake"
    runCommand make

    echo "Installing CMake"
    runCommand make install
    cd $workdir
fi

# get and build Qt
# might take a longer time
if [ $build_qt ]
then
    module='Qt'
    echo "Getting Qt"
    cd $downloads
    if [ -r $downloads/$qt_version.tar.gz ]
	then
	echo "already downloaded, reusing file"
	else
	wget $qt_url
	fi

    echo "Unpacking Qt"
    cd $workbench
    runCommand tar -xzvf $downloads/$qt_version.tar.gz

    echo "Building Qt"
    cd $qt_version

    runCommand ./configure -qt-gif -no-exceptions -debug -no-separate-debug-info -fast \
   -system-libpng -system-libjpeg -system-zlib -qdbus -nomake examples \
   -nomake demos -prefix $opt -confirm-license -opengl

    runCommand make
    runCommand make install
    cd $workdir
fi

# Get and build php
if [ $build_php ]
then
    module='PHP'
    echo "Getting PHP"
    cd $downloads
    if [ -r $downloads/$php_version.tar.gz ]
	then
	echo "already downloaded, reusing file"
	else
	wget $php_url
	fi

    echo "Unpacking PHP"
    cd $workbench
    runCommand tar -xzvf $downloads/$php_version.tar.gz

    echo "Building PHP"
    cd $php_version/
    runCommand ./configure --prefix=$opt $php_configureoptions
    runCommand make
    runCommand make install
    runCommand make install-pear

    cp php.ini-recommended $opt/lib/php.ini
    echo 'extension_dir = '$workdir'/opt/lib/php/extensions/no-debug-non-zts-20060613/' >> $opt/lib/php.ini
    echo 'qt.codec = Latin1' >> $opt/lib/php.ini
    echo 'extension = php_qt.so' >> $opt/lib/php.ini
    cd $workdir
fi

# get and build phpunit
if [ $build_phpunit ]
then
    module='PEAR'
    echo "Getting and installing PEAR"
    runCommand pear channel-discover $phpunit_server
    runCommand pear install phpunit/PHPUnit
    cd $workdir
fi

# get and build phpqt
if [ $build_phpqt ]
then
    module='PHPQt'
    echo "Getting PHP-Qt"
    cd $workbench
    runCommand svn co $phpqt_url
    mkdir -p build_phpqt
    cd phpqt/
    runCommand ./prepare_svn.sh
    cd $workbench/build_phpqt
    echo "Building PHP-Qt"
    runCommand cmake -DCMAKE_INSTALL_PREFIX=$opt ../phpqt
    runCommand make
    runCommand make install
    cd $workbench/phpqt/tests
    runCommand phpunit QtLoadModuleTestCase.php
    echo "running Unittests"
    phpunit QtBasicTestCase.php
#    phpunit QtApplicationTestCase.php
#    phpunit QtStressTestCase.php
    cd $workdir
fi
