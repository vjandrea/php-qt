<?php

    /****************************************************************
    **
    ** Qt tutorial 5
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t5.html
    **
    ****************************************************************/

	if(!extension_loaded('php_qt')) {
		dl('php_qt.' . PHP_SHLIB_SUFFIX);
	}

    class MyWidget extends QWidget
    {

        private $quit;
        private $slider;
        private $layout;
        private $lcd;

        function __construct()
        {
            parent::__construct();

            $this->quit = new QPushButton(tr("Quit"));
            $this->quit->setFont(new QFont("Times", 18, QFont::Bold));

            $this->lcd = new QLCDNumber(2);
            $this->lcd->setSegmentStyle(QLCDNumber::Filled);

            $this->slider = new QSlider(Qt::Horizontal);
            $this->slider->setRange(0, 99);
            $this->slider->setValue(0);

            $this->connect($this->quit, SIGNAL('clicked()'), QApplication::instance(), SLOT('quit()'));
            $this->connect($this->slider, SIGNAL('valueChanged(int)'),$this->lcd, SLOT('display(int)'));

            $this->layout = new QVBoxLayout();
            $this->layout->addWidget($this->quit);
            $this->layout->addWidget($this->lcd);
            $this->layout->addWidget($this->slider);
            $this->setLayout($this->layout);
        }
    }


    $app = new QApplication($argc,$argv);

    $widget = new MyWidget();
    $widget->show();

    $app->exec();

?>
