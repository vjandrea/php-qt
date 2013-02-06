<?php

    /****************************************************************
    **
    ** Qt tutorial 3
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t3.html
    **
    ****************************************************************/

	if(!extension_loaded('php_qt')) {
		dl('php_qt.' . PHP_SHLIB_SUFFIX);
	}

    $app = new QApplication($argc,$argv);

    $window = new QWidget();
    $window->resize(200,120);

    $quit = new QPushButton("Quit", $window);
    $quit->setFont(new QFont("Times", 18, QFont::Bold));
    $quit->setGeometry(10, 40, 180, 40);

    QObject::connect($quit,SIGNAL('clicked()'), $app, SLOT('quit()'));

    $window->show();
    $app->exec();

?>
