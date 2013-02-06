<?php

    /**
     *	This file tests if the module loads properly
     */

    require_once('PHPUnit/Framework/TestCase.php');
    require_once('PHPUnit/Framework/TestSuite.php');


    class QtLoadModuleTestCase extends PHPUnit_Framework_TestCase {
    
	public function __construct($name) {
	    parent::__construct($name);
	}
    
	function testModule() {
	    $this->assertTrue(extension_loaded('php_qt'), "Module PHP-Qt is not loaded!");
	}

	function testClassQt() {
	    $this->assertTrue(class_exists('Qt'), "class Qt not found!");
	}

	function testClassQString() {
	    $this->assertTrue(class_exists('QString'), "class QString not found!");
	}

    }    
    
?>