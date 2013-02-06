<?php
#	if(!extension_loaded('php_qt')) {
	#	dl('php_qt.' . PHP_SHLIB_SUFFIX);
	#}
	
	class Box extends QBoxLayout
	{
		private $mainWidget;
		public $button1;
		public $button2;
	
		function __construct() {

			$mainWidget = new QWidget();
			parent::__construct(QBoxLayout::TopToBottom,$mainWidget);
			$this->mainWidget = $mainWidget;

			$this->button1 = new QPushButton("About Qt!");
			$this->button2 = new QPushButton("Quit");
			$this->addWidget($this->button1);
			$this->addWidget($this->button2);

		}

		function __destruct()
		{
     		    $this->button1 = 0;
     		    $this->button2 = 0;	
	 	    $this->mainWidget = 0;
		}
		
		function show()
		{
			$this->mainWidget->show();
		}
	}
	
	
	$app = new QApplication($argc,$argv);

	$box = new Box();
	$box->show();

	QObject::connect($box->button1, SIGNAL("clicked()"), $app, SLOT("aboutQt()"));
	QObject::connect($box->button2, SIGNAL("clicked()"), $app, SLOT("quit()"));
		
	$app->exec();
	
?>
