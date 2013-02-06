<?php

    /****************************************************************
    **
    ** Qt tutorial 9
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t9.html
    **
    ****************************************************************/

    if(!extension_loaded('php_qt')) {
        dl('php_qt.' . PHP_SHLIB_SUFFIX);
    }

    require_once('cannonfield.php');
    require_once('lcdrange.php');

    class MyWidget extends QWidget {

	public function __construct() {
	    parent::__construct();

	    $quit = new QPushButton(tr("Quit"));
    	    $quit->setFont(new QFont("Times", 18, QFont::Bold));

	    QObject::connect($quit, SIGNAL('clicked()'), QApplication::instance(), SLOT('quit()'));

	    $angle = new LCDRange();
	    $angle->setRange(5, 70);

	    $cannonField = new CannonField();

	    QObject::connect($angle, SIGNAL('valueChanged(int)'), $cannonField, SLOT('setAngle(int)'));
	    QObject::connect($cannonField, SIGNAL('angleChanged(int)'), $angle, SLOT('setValue(int)'));

	    $gridLayout = new QGridLayout();
	    $gridLayout->addWidget($quit, 0, 0);
	    $gridLayout->addWidget($angle, 1, 0);
	    $gridLayout->addWidget($cannonField, 1, 1, 2, 1);
	    $gridLayout->setColumnStretch(1, 10);
	    $this->setLayout($gridLayout);

	    $angle->setValue(60);
	    $angle->setFocus();

	}
    }

    $app = new QApplication($argc, $argv);
    $widget = new MyWidget();
    $widget->setGeometry(100, 100, 500, 355);
    $widget->show();
    $app->exec();

?>