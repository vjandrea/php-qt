<?php

    /**
     *	This file contains a couple of QString tests
     */

    require_once('PHPUnit/Framework/TestCase.php');
    require_once('PHPUnit/Framework/TestSuite.php');

    class QStringTestCase extends PHPUnit_Framework_TestCase {
    
	public function __construct($name) {
	    parent::__construct($name);
	}

	function test_construct() {
	    $s1 = new QString("bird");
	    $s2 = new QString($s1);	   // QString ( const QString & other ) 
	    $this->assertEquals($s2->__toString(), "bird", "Could not construct QString!");
	    $s3 = new QString(40);	   // QString ( QChar ch ) 
	    $this->assertEquals($s3->__toString(), "(", "Could not construct QString!");
	    $s4 = new QString("helicopter");
	    $this->assertEquals($s4->__toString(), "helicopter", "Could not construct QString!");
//	    $s5 = new QString(40,3);
//	    $this->assertEquals($s5->__toString(), "(((", "Could not construct QString!");
	    echo "\ntesting QString::__construct() passed";
	}

	function test_append() {
	    $s1 = new QString("bird");
	    $s2 = new QString(" plug");
	    $s1->append($s2);
	    $this->assertEquals($s1->__toString(), "bird plug", "Could not append QString to QString!");
	    $s1->append(" helicopter");
	    $this->assertEquals($s1->__toString(), "bird plug helicopter", "Could not append string to QString!");
	    $s1->append(40);
	    $this->assertEquals($s1->__toString(), "bird plug helicopter(", "Could not append QChar to QString!");
	    echo "\ntesting QString::append() passed";
	}

	function test_toInt() {
	    $s1 = new QString("24");
	    $this->assertEquals($s1->toInt(), 24, "Could not get integer!");
	    echo "\ntesting QString::toInt() passed";
	}

	function test_toDouble() {
	    $s1 = new QString("24.3");
	    $this->assertEquals($s1->toDouble(), 24.3, "Could not get double!");
	    echo "\ntesting QString::toDouble() passed";
	}

	function test_number() {
	    $this->assertEquals(QString::number(24)->__toString(), "24", "Could not get string version of integer!");
	    $this->assertEquals(QString::number(24.3)->__toString(), "24.3", "Could not get string version of double!");
	    echo "\ntesting QString::number() passed";
	}

	function test_isEmpty() {
	    $s1 = new QString();
	    $this->assertTrue($s1->isEmpty(), "Could not ask isEmpty()!");
	    $s1->append("I");
	    $this->assertFalse($s1->isEmpty(), "Could not ask isEmpty()!");
	    echo "\ntesting QString::isEmpty() passed";
	}

	function test_clear() {
	    $s1 = new QString("hello");
	    $s1->clear();
	    $this->assertEquals($s1->__toString(), "", "Could not clear()!");
	    echo "\ntesting QString::clear() passed";
	}

    }    
    
?>