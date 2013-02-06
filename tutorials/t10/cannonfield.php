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

    class CannonField extends QWidget
    {

        private $slots = array("setAngle(int)", "setForce(int)");
        private $signals = array("angleChanged(int)", "forceChanged(int)");

	private $currentAngle = 0;
	private $currentForce = 0;

	public function __construct() 
	{
	    parent::__construct();

	    $this->currentAngle = 45;
	    $this->currentForce = 0;
	    $this->setPalette(new QPalette(new QColor(250, 250, 200)));
	    $this->setAutoFillBackground(true);
	}

	public function angle() { return $this->currentAngle; }
	public function force() { return $this->currentForce; }

	public function setAngle($angle)
	{
	    if ($angle < 5)
                $angle = 5;
	    if ($angle > 70)
		$angle = 70;
	    if ($this->currentAngle == $angle)
		return;
	    $this->currentAngle = $angle;
	    $this->update( $this->cannonRect() );
	    emit( $this->angleChanged($this->currentAngle) );
	}

	public function setForce($force)
	    {
		if ($force < 0)
		    $force = 0;
		if ($this->currentForce == $force)
		    return;
		$this->currentForce = $force;
		emit( $this->forceChanged($this->currentForce) );
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

	private function cannonRect()
	    {
		$result = new QRect(0, 0, 50, 50);
		$result->moveBottomLeft($this->rect()->bottomLeft());
		return $result;
	    }
    }

?>