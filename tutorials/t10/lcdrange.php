<?php

    /****************************************************************
    **
    ** Qt tutorial 10
    **
    ** original:
    ** http://doc.trolltech.com/4.1/tutorial-t10.html
    **
    ****************************************************************/

    if(!extension_loaded('php_qt')) {
        dl('php_qt.' . PHP_SHLIB_SUFFIX);
    }

    class LCDRange extends QWidget
    {
        private $slots = array("setValue(int)", "setRange(int, int)");
        private $signals = array("valueChanged(int)");

	private $slider;

	public function __construct() 
	{
	    parent::__construct();

	    $this->lcd = new QLCDNumber(2);
	    $this->lcd->setSegmentStyle(QLCDNumber::Filled);
	    
	    $this->slider = new QSlider(Qt::Horizontal);
	    $this->slider->setRange(0, 99);
	    $this->slider->setValue(0);

	    $this->connect($this->slider, SIGNAL('valueChanged(int)'), $this->lcd, SLOT('display(int)'));
	    $this->connect($this->slider, SIGNAL('valueChanged(int)'), $this, SIGNAL('valueChanged(int)'));

	    $layout = new QVBoxLayout();
	    $layout->addWidget($this->lcd);
	    $layout->addWidget($this->slider);
	    $this->setLayout($layout);
	    $this->setFocusProxy($this->slider);
	}

	public function value()
	{
	    return $this->slider->value();
	}
	
	public function setValue($value = 0)
	{
	    $this->slider->setValue($value);
	}

	public function setRange($minValue, $maxValue)
	{
	    if ($minValue < 0 || $maxValue > 99 || $minValue > $maxValue) {
		echo "LCDRange::setRange(".$minValue.", ".$maxValue.")\n".
		    "\tRange must be 0..99\n".
		    "\tand minValue must not be greater than maxValue";
		return;
	    }
	    $this->slider->setRange($minValue, $maxValue);
	}

    };

?>