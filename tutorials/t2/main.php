<?php

    /****************************************************************
    **
    ** Qt tutorial 2
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t2.html
    **
    ****************************************************************/

	if(!extension_loaded('php_qt')) {
		dl('php_qt.' . PHP_SHLIB_SUFFIX);
	}

    $app = new QApplication($argc,$argv);

    $quit = new QPushButton("Quit");
    $quit->resize(75,30);
    $quit->setFont(new QFont("Times", 18, QFont::Bold));

    QObject::connect($quit,SIGNAL('clicked()'),$app,SLOT('quit()'));

    $quit->show();
    $app->exec();

?>
