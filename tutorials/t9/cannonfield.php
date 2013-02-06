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

    class CannonField extends QWidget
    {

        private $slots = array("setAngle(int)");
        private $signals = array("angleChanged(int)");

	private $currentAngle = 0;

	public function __construct() 
	{
	    parent::__construct();

	    $this->currentAngle = 45;
	    $this->setPalette(new QPalette(new QColor(250, 250, 200)));
	    $this->setAutoFillBackground(true);

	}

	public function angle() { return $this->currentAngle; }

	public function setAngle($angle)
	{
	    if ($angle < 5)
                $angle = 5;
	    if ($angle > 70)
		$angle = 70;
	    if ($this->currentAngle == $angle)
		return;
	    $this->currentAngle = $angle;
	    $this->update();
	    emit($this->angleChanged($this->currentAngle));
	}

	protected function paintEvent ($event) {
	    $painter = new QPainter($this);
	    $painter->setPen(Qt::NoPen);
	    $painter->setBrush(new QBrush(new QColor(0,0,255)));

	    $painter->translate(0, $this->rect()->height());
	    $painter->drawPie(new QRect(-35, -35, 70, 70), 0, 90 * 16);
	    $painter->rotate(-$this->currentAngle);
	    $painter->drawRect(new QRect(30, -5, 20, 10));

	    $painter->end();
	}

    }

?>