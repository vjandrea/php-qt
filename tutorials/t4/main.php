<?php

    /****************************************************************
    **
    ** Qt tutorial 4
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t4.html
    **
    ****************************************************************/

	if(!extension_loaded('php_qt')) {
		dl('php_qt.' . PHP_SHLIB_SUFFIX);
	}

    class MyWidget extends QWidget
    {

        private $quit;

        function __construct()
        {
            parent::__construct();

            $this->setFixedSize(200, 120);

            $this->quit = new QPushButton(tr("Quit"), $this);
            $this->quit->setGeometry(62, 40, 75, 30);
            $this->quit->setFont(new QFont("Times", 18, QFont::Bold));

            /* the macro qApp is not available in php-qt, we use QApplication::instance() directly. */
            $this->connect($this->quit, SIGNAL('clicked()'), QApplication::instance(), SLOT('quit()'));
        }

    }

    $app = new QApplication($argc,$argv);
    $widget = new MyWidget();
    $widget->show();
    $app->exec();

?>
