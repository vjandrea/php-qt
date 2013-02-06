<?php

	/* unicode test from Ferenc Verec */

    if(!extension_loaded('php_qt')) {
        dl('php_qt.' . PHP_SHLIB_SUFFIX);
    }

	class TestButtons extends QWidget
	{
		public $layout;
		public $buttons;

		public function __construct() 
		{
			parent::__construct();
			
			$this->layout = new QVBoxLayout($this);

			// Load the test xml
			$unicodeXml = new DOMDocument();
			$unicodeXml->load("unicode.xml");
			$xpath = new DOMXPath($unicodeXml);
			$dataNodes = $xpath->query("/test/data");

			// Loop on all data node and create buttons
			foreach($dataNodes as $data)
			{
				$this->buttons[] = new QLineEdit(QString::fromUtf8($data->nodeValue, -1),$this);

	        		    foreach($data->attributes as $attribute){
                			$this->layout->addWidget(new QLabel($attribute->name.": ".$attribute->value));
            			    }

				$this->layout->addWidget($this->buttons[count($this->buttons)-1]);
			}

			$this->buttons[] = new QLineEdit("Test",$this);
			$this->layout->addWidget($this->buttons[count($this->buttons)-1]);

			$this->buttons[] = new QLineEdit(("second Test"),$this);
			$this->layout->addWidget($this->buttons[count($this->buttons)-1]);

		}

	}

    $app = new QApplication($argc,$argv);
	$widget = new TestButtons();
	$widget->show();
	$app->exec();

?>
