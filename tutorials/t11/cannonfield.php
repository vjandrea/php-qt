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

    class CannonField extends QWidget
    {

        private $slots = array("setAngle(int)", "setForce(int)", "moveShot()", "shoot()");
        private $signals = array("angleChanged(int)", "forceChanged(int)", );

	private $currentAngle = 0;
	private $currentForce = 0;

	private $timerCount;
	private $autoShootTimer;
	private $shootAngle;
	private $shootForce;

	private $barrelRect;

	public function __construct() 
	{
	    parent::__construct();

	    $this->currentAngle = 45;
	    $this->currentForce = 0;
	    $this->timerCount = 0;
	    $this->autoShootTimer = new QTimer($this);
	    QObject::connect( $this->autoShootTimer, SIGNAL('timeout()'), $this, SLOT('moveShot()') );
	    $this->shootAngle = 0;
	    $this->shootForce = 0;
	    $this->setPalette(new QPalette(new QColor(250, 250, 200)));
	    $this->setAutoFillBackground(true);

	    $this->barrelRect = new QRect(30, -5, 20, 10);
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

	public function shoot()
	    {
		if ($this->autoShootTimer->isActive())
		    return;
		
		$this->timerCount = 0;
		$this->shootAngle = $this->currentAngle;
		$this->shootForce = $this->currentForce;
		$this->autoShootTimer->start(5);
	    }

	public function moveShot()
	    {
		$region = $this->shotRect();
		++$this->timerCount;

		$shotR = $this->shotRect();

		if ($shotR->x() > $this->width() || $shotR->y() > $this->height()) {
		    $this->autoShootTimer->stop();
		}
		else {
		    $region = $region->unite($shotR);
		}
		$this->update( $region );
	    }	

	protected function paintEvent ($event) {
	    $painter = new QPainter($this);

	    $this->paintCannon($painter);
	    
	    if ($this->autoShootTimer->isActive())
		$this->paintShot($painter);

	    $painter->end();
	}

	private function paintShot($painter)
	    {
		$painter->setPen(Qt::NoPen);
		$painter->setBrush(Qt::black);
		$painter->drawRect($this->shotRect());
	    }

	private function paintCannon($painter)
	    {
		$painter->setPen(Qt::NoPen);
		$painter->setBrush(new QBrush(new QColor(0,0,255)));

		$painter->save();
		$painter->translate(0, $this->height());
		$painter->drawPie(new QRect(-35, -35, 70, 70), 0, 90 * 16);
		$painter->rotate(-$this->currentAngle);
		$painter->drawRect($this->barrelRect);
		$painter->restore();
	    }

	private function cannonRect()
	    {
		$result = new QRect(0, 0, 50, 50);
		$result->moveBottomLeft($this->rect()->bottomLeft());
		return $result;
	    }

	private function shotRect()
	    {
		$gravity = 4.0;

		$time = $this->timerCount / 20.0;
		$velocity = $this->shootForce;
		$radians = $this->shootAngle * 3.14159265 / 180;

		$velx = $velocity * cos($radians);		
		$vely = $velocity * sin($radians);
		
		$x0 = ($this->barrelRect->right() + 5) * cos($radians);
		$y0 = ($this->barrelRect->right() + 5) * sin($radians);
		$x = $x0 + $velx * $time;
		$y = $y0 + $vely * $time - 0.5 * $gravity * $time * $time;

		$result = new QRect(0, 0, 6, 6);
		$result->moveCenter(new QPoint( qRound($x), $this->height() - 1 - qRound($y)));
		return $result;
	    }

    }

?>