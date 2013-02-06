<?php

    /**
     *	This file contains a couple of basic tests
     *
     * - test for references
     */

    require_once('PHPUnit/Framework/TestCase.php');
    require_once('PHPUnit/Framework/TestSuite.php');

    $argc=0;
    $argv=array("");

    class myColor extends QColor {
	public $myProperty;
	public function __construct($r,$g,$b,$p) {
		parent::__construct($r,$g,$b);
		$this->myProperty = $p;
	    }
    }

    class myColor2 extends QColor {
	public $myProperty;
	public function __construct($r,$g,$b,$p) {
		parent::__construct($r,$g,$b);
		$this->myProperty = $p;
	    }
	public function __clone()
	    {
		$this->myProperty = $this->myProperty * 2;
	    }
    }

    $app = new QApplication($argc, $argv);

    class QtAdvancedTestCase extends PHPUnit_Framework_TestCase {
    
	public function __construct($name) {
	    parent::__construct($name);
	}

	// test references
	function testReference() {
	    $a = new QPushButton("a text");
	    $b = &$a; // create the reference
	    $a->setText("text changed");
	    echo "\ntesting reference";
	    $this->assertEquals($b->text()->__toString(), "text changed", "Creating a reference does not work!");
	    echo " passed";
	}

	// test cloning, change the original and check the clone
	function testClone() {
	    $a = new QColor(100,200,255);
	    $b = clone $a; // create the reference
	    $a->setBlue(123);
	    echo "\ntesting clone";
	    $this->assertEquals($b->blue(), 255, "Cloning an object does not work!");
	    echo " passed (check clone)";
	}

	// test cloning, change the clone and test the original
	function testClone2() {
	    $a = new QColor(100,200,255);
	    $b = clone $a; // create the reference
	    echo "\ntesting clone";
	    $b->setRed(75);
	    $this->assertEquals($a->red(), 100, "Cloning an object does not work (error: original has changed)!");
	    echo " passed (check original)";
	}

	// test cloning a custom object, change the original and test the clone
	function testClone3() {
	    $a = new myColor(100,200,255,24);
	    $b = clone $a; // create the reference
	    echo "\ntesting clone";
	    $this->assertEquals($b->myProperty, 24, "Cloning a selfwritten object does not work (error: property has changed)!");
	    echo " passed (check childs)";
	}

	// test cloning a custom object, overwrite the __clone magic method and test the clone
	function testClone4() {
	    $a = new myColor2(100,200,255,24);
	    $b = clone $a; // create the reference
	    echo "\ntesting clone";
	    $this->assertEquals($b->myProperty, 48, "Cloning a selfwritten object does not work (error: __clone() was not called)!");
	    echo " passed (check __clone())";
	}

	// test cloning a custom object, overwrite the __clone magic method and test the original
	function testClone5() {
	    $a = new myColor2(100,200,255,24);
	    $b = clone $a; // create the reference
	    echo "\ntesting clone";
	    $this->assertEquals($a->myProperty, 24, "Cloning a selfwritten object does not work (error: original has been modified)!");
	    echo " passed (check original after __clone())";
	}

    } // end test
    
?>