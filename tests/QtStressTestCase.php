<?php

    /**
     *	This file tests if the module loads properly
     */

    require_once('PHPUnit/Framework/TestCase.php');
    require_once('PHPUnit/Framework/TestSuite.php');


    class QtStressTestCase extends PHPUnit_Framework_TestCase {
    
	public function __construct($name) {
	    parent::__construct($name);
	}
    
	function testCreatingObjects() {
//	    for($i = 0; $i < 8000; $i++){

	    while(true){
	    	    $b = new QObject();
	    $b->setObjectName("hallo");
		echo $i++."\n";
		echo $b->objectName();
			    $b->__destruct();
	    }

	}

    }    
    
?>