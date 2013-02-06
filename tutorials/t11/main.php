<?php

    /****************************************************************
    **
    ** Qt tutorial 11
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t11.html
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

	    $quit = new QPushButton(tr("&Quit"));
    	    $quit->setFont(new QFont("Times", 18, QFont::Bold));

	    QObject::connect($quit, SIGNAL('clicked()'), QApplication::instance(), SLOT('quit()'));

	    $angle = new LCDRange();
	    $angle->setRange(5, 70);

	    $force = new LCDRange();
	    $force->setRange(10, 50);

	    $cannonField = new CannonField();

	    QObject::connect($angle, SIGNAL('valueChanged(int)'), $cannonField, SLOT('setAngle(int)'));
	    QObject::connect($cannonField, SIGNAL('angleChanged(int)'), $angle, SLOT('setValue(int)'));

	    QObject::connect($force, SIGNAL('valueChanged(int)'), $cannonField, SLOT('setForce(int)'));
	    QObject::connect($cannonField, SIGNAL('forceChanged(int)'), $force, SLOT('setValue(int)'));

	    $shoot = new QPushButton(tr("&Shoot"));
	    $shoot->setFont(new QFont("Times", 18, QFont::Bold));

	    QObject::connect($shoot, SIGNAL('clicked()'), $cannonField, SLOT('shoot()'));

	    $topLayout = new QHBoxLayout();
	    $topLayout->addWidget($shoot);
	    $topLayout->addStretch(1);

	    $leftLayout = new QVBoxLayout();
	    $leftLayout->addWidget($angle);
	    $leftLayout->addWidget($force);

	    $gridLayout = new QGridLayout();
	    $gridLayout->addWidget($quit, 0, 0);
	    $gridLayout->addLayout($topLayout, 0, 1);
	    $gridLayout->addLayout($leftLayout, 1, 0);
	    $gridLayout->addWidget($cannonField, 1, 1, 2, 1);
	    $gridLayout->setColumnStretch(1, 10);
	    $this->setLayout($gridLayout);

	    $angle->setValue(60);
	    $force->setValue(25);
	    $angle->setFocus();

	}
    }

    $app = new QApplication($argc, $argv);
    $widget = new MyWidget();
    $widget->setGeometry(100, 100, 500, 355);
    $widget->show();
    $app->exec();

?>