<?php
// todo: set bool, empty string
    /**
     *	This file contains a couple of basic tests
     *
     *	- test fetching constants, the opcode handler has been overwritten
     *	- test __toString()
     *	- test QString
     *	- test returning values and objects
     *  - test returning references
     */

    require_once('PHPUnit/Framework/TestCase.php');
    require_once('PHPUnit/Framework/TestSuite.php');

    /* 
     * check if we can fetch constants
     */

    class foo extends QObject {
	const a = "a";
	const b = 24;

	public function __construct($parent)
	{
	    parent::__construct($parent);
	}

	public function testMethod($value = NULL) {
	    if ($value) echo $value."\n";
	}
	
	public static function staticMethod()
	{
	    return "staticMethod";
	}

//	    parent::tr("");

	public static function staticTrMethod($arg)
	{
	    return parent::tr($arg);
	}

	public function blockSignals($bool)
	{
	    return parent::blockSignals($bool);
	}
	
    }

    class myWidget extends QWidget
    {
	public function __construct()
	{
	    parent::__construct();
	}
	
	public function sizeHint()
	{
	  echo ".";
	    return parent::sizeHint();
	}
	
    }


    class QtBasicTestCase extends PHPUnit_Framework_TestCase {
    
	private $app;
    
	public function __construct($name) {
	    parent::__construct($name);
	}

	// try to fetch a string constant
	function testFetchStringConstant() {
	    echo "\ntesting foo::a";
	    $this->assertEquals(foo::a, "a", "Could not fetch string constant!");
	    echo " passed";
	}

	// try to fetch a numeric constant
	function testFetchNumConstant() {
	    echo "\ntesting foo::b";
	    $this->assertEquals(foo::b, 24, "Could not fetch numeric constant!");
	    echo " passed";
	}

	// try to fetch a Qt constant
	function testQtConstant() {
	    echo "\ntesting Qt::Horizontal";
	    $this->assertTrue(Qt::Horizontal == 1, "Could not fetch constant from Qt!");
	    echo " passed";
	}

	// try to call a static method
	function testCallStaticMethod() {
	    echo "\ntesting foo::staticMethod()";
	    $this->assertEquals(foo::staticMethod(), "staticMethod", "Could not call a static method!");
	    echo " passed";
	}

	// try to call a static Qt method
	function testCallStaticQtMethod() {
	    echo "\ntesting foo::tr()";
	    $this->assertEquals(foo::tr("hello") == "hello", "Could not call a static Qt method!");
	    echo " passed";
	}

	// try to call a parent Qt method within a method
	function testCallParentQtMethod() {
	    echo "\ntesting parent::blockSignals() within foo::blockSignals()";
	    $o = new QObject();
	    $p = new foo($o);
	    // set blockSignals to true, so we can test it
	    $p->blockSignals(true); 
    	    $this->assertTrue($p->blockSignals(true), "Could not call a parent Qt method!");
	    echo " passed";
	}

	// try to call a static Qt method within a static PHP method
	function testCallStaticQtMethodWithinStaticPHPMethod() {
	    echo "\ntesting foo::staticTrMethod()";
	    $this->assertEquals(foo::staticTrMethod("hello") == "hello", "Could not call a static Qt method!");
	    echo " passed";
	}

	// test the __toString() method
	function testQString() {
	    echo "\ntesting QString::__toString()";
	    $s = new QString("hello");
	    $this->assertEquals($s->__toString(), "hello", "Could not handle __toString()!");
	    echo " passed";
	}

	// try to append a PHP string to a QString
	function testQStringAppendString() {
	    echo "\ntesting QString::append(\"hello\")";
	    $s = new QString("hello");
	    $s->append(" world");
	    $this->assertEquals($s->__toString(), "hello world", "Could append simple string to QString!");
	    echo " passed";
	}

	// try to create an instance of QApplication
	function testQApplication() {
	    echo "\ntesting QApplication";
	    $argc=1;
	    $argv=array("argv");
	    $this->app = new QApplication($argc,$argv);
	    $this->assertTrue(is_object($this->app), "Could not create an instance of QApplication!");
	    echo " passed";
	}

	// test returning numbers
	function testReturnNum() {
	    echo "\ntesting numbers";
	    $date = new QDate(2007,3,24);
	    $this->assertTrue((gettype($date->day()) == "integer"), "Value returned is not integer type, ".gettype($date->day())." detected!");
	    $this->assertTrue(($date->day() == 24), "Value returned is wrong!");
	    echo " passed";
	}

	// test returning double
	// test returning array
	// test returning null

	// test returning boolean
	function testReturnBool() {
	    echo "\ntesting bool";
	    $date = new QDate(2007,3,24);
	    $this->assertTrue((gettype($date->setDate(2007,3,24)) == "boolean"), "Value returned is not boolean type, ".gettype($date->setDate(2007,3,24))." detected!");
	    $this->assertTrue(($date->setDate(2007,3,24) == true), "Value returned is wrong!");
	    echo " passed";
	}

	// test returning object
	function testReturnObject() {
	    echo "\ntesting objects";
	    $parent = new QObject();
	    $object = new QObject($parent);
	    $parent__ = $object->parent();
	    $this->assertTrue(($parent === $parent__),"Object returned by QObject->parent() is not the same as parent!");
	    echo " passed";
	}
// ???
	// test returning a new object
	function testReturnNewObject() {
	    echo "\ntesting new objects";
	    $date = new QDate(2007,3,24);
	    $date__ = $date->addDays(2);
	    $this->assertFalse(($date === $date__),"new Object returned by QObject->parent() is the same as parent!");
	    echo " passed";
	}

	// test returning string
	function testReturnQString() {
	    echo "\ntesting QString as return";
	    $object = new QObject();
	    $object->setObjectName("hello");
	    $string = $object->objectName();
	    $this->assertTrue(is_object($string), "String is not a QString!");
	    $this->assertEquals($string->toAscii(), "hello", "Return object does not contain the same text!");
	    echo " passed";
	}

	// try to give a QString as argument
	function testAddQString() {
	    echo "\ntesting QString as argument";
	    $object = new QObject();
	    $object->setObjectName(new QString("hello string"));
	    $s = $object->objectName();
	    $this->assertEquals($s->toAscii(),"hello string", "Return object does not contain the same text!");
	    echo " passed";
	}

	// try to append a QString to a QString
	function testQStringAppendObject() {
	    echo "\ntesting QString::append(new QString(\"hello\"))";
	    $s = new QString("hello");
	    $t = new QString(" world");
	    $s->append($t);
	    $this->assertEquals($s->__toString(), "hello world", "Could not append QString to QString!");
	    echo " passed";
	}

	// test methods of an derivated object
	function testUnknownMethod() {
	    echo "\ntesting derivated object";
	    $date = new QObject();
	    $foo = new foo($date);
	    $foo->parent();
	    $foo->testMethod();
//	    $foo->___();	// method not defined
	    echo " passed";
	}

	function testAmbiguousMethodCall() {
	    echo "\ntesting ambiguous method call";
	    $w = new QWidget();
	    $w->setPalette(new QPalette(new QColor(250, 250, 200)));
	    echo " passed";
	}

	function testTr(){
	    echo "\ntesting global tr()";
	    $s = tr("hello world");
	    $this->assertEquals($s->__toString(), "hello world", "tr() doesnt work!");
	    echo " passed";
	}

/*      function testVirtualMethodCall()
      {
	echo "\ntesting virtual method call 'sizeHint()' in a loop: ";
//	$argc=1;
//	$argv=array("argv");
//	$app = new QApplication($argc, $argv);
    
	$m = new myWidget();
    
	for( $i = 0; $i < 10; $i++ )
	  {
	    $m->show();
	    $m->hide();
	  }
	echo " passed";
      }
*/
    }    
    
?>