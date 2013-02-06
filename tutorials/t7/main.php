<?php

    /****************************************************************
    **
    ** Qt tutorial 7
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t7.html
    **
    ****************************************************************/

    if(!extension_loaded('php_qt')) {
        dl('php_qt.' . PHP_SHLIB_SUFFIX);
    }

    class LCDRange extends QWidget
    {
        private $lcd;
        private $slider;
        private $layout;

        private $slots = array("setValue(int)");
        private $signals = array("valueChanged(int)");

        function __construct()
        {
            parent::__construct();

            $this->lcd = new QLCDNumber(2);
            $this->lcd->setSegmentStyle(QLCDNumber::Filled);

            $this->slider = new QSlider(Qt::Horizontal);
            $this->slider->setRange(0, 99);
            $this->slider->setValue(0);

            $this->connect($this->slider, SIGNAL('valueChanged(int)'),
                    $this->lcd, SLOT('display(int)'));
            $this->connect($this->slider, SIGNAL('valueChanged(int)'),
                    $this,SIGNAL('valueChanged(int)'));

            $this->layout = new QVBoxLayout();
            $this->layout->addWidget($this->lcd);
            $this->layout->addWidget($this->slider);
            $this->setLayout($this->layout);
        }

        function value(){
            return $this->slider->value();
        }

        function setValue($value){
            $this->slider->setValue($value);
        }
    };

    class MyWidget extends QWidget
    {
        private $quit;
        private $grid;
        private $layout;

        function __construct()
        {
            parent::__construct();

            $this->quit = new QPushButton(tr("Quit"));
            $this->quit->setFont(new QFont("Times", 18, QFont::Bold));
            $this->connect($this->quit, SIGNAL('clicked()'), QApplication::instance(), SLOT('quit()'));

            $this->grid = new QGridLayout();
            for ($row = 0; $row < 3; ++$row) {
                for ($column = 0; $column < 3; ++$column) {
                    $lcdRange = new LCDRange();
                    $this->grid->addWidget($lcdRange, $row, $column);
                    if($previousRange)
                        $this->connect($lcdRange, SIGNAL('valueChanged(int)'), $previousRange, SLOT('setValue(int)'));
                    $previousRange = $lcdRange;
                }

            }

            $this->layout = new QVBoxLayout();
            $this->layout->addWidget($this->quit);
            $this->layout->addLayout($this->grid);
            $this->setLayout($this->layout);
        }
    }

    $app = new QApplication($argc,$argv);

    $widget = new MyWidget();
    $widget->show();
    $app->exec();

?>
