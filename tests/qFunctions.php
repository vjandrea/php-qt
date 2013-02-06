<?php

if(!extension_loaded('php_qt')) {
	dl('php_qt.' . PHP_SHLIB_SUFFIX);
}

echo "\nTesting qAbs:\n";
echo "qAbs(4) = ".qAbs(4)."\n";
echo "qAbs(-4) = ".qAbs(-4)."\n";
echo "qAbs(4.3) = ".qAbs(4.3)."\n";
echo "qAbs(-4.3) = ".qAbs(-4.3)."\n";

echo "\nTesting qRound:\n";
echo "qRound(1) = ".qRound(1)."\n";
echo "qRound(4.5) = ".qRound(4.5)."\n";
echo "qRound(-4.47837289) = ".qRound(-4.478327289)."\n";

echo "\nTesting qRound64:\n";
echo "qRound64(1) = ".qRound64(1)."\n";
echo "qRound64(4.5) = ".qRound64(4.5)."\n";
echo "qRound64(-4.47837289) = ".qRound64(-4.478327289)."\n";

echo "\nTesting qMin:\n";
echo "qMin(4,5) = ".qMin(4,5)."\n";
echo "qMin(7,-1.567) = ".qMin(7,-1.567)."\n";

echo "\nTesting qMax:\n";
echo "qMax(4,5) = ".qMax(4,5)."\n";
echo "qMax(7,-1.567) = ".qMax(7,-1.567)."\n";

echo "\nTesting qBound:\n";
echo "qBound(3,4,5) = ".qBound(3,4,5)."\n";
echo "qBound(4,3,5) = ".qBound(4,3,5)."\n";
echo "qBound(4,6,5) = ".qBound(4,6,5)."\n";

echo "\nTesting qPrintable:\n";
echo "qPrintable(\"bob\") = ".qPrintable("bob")."\n";
echo "qPrintable(new QString(\"fred\") = ".qPrintable(new QString("fred"))."\n";
echo 'qPrintable(45) = '.qPrintable(45)."\n";
class test {}
echo "qPrintable(new test) = ".qPrintable(new test)."\n";

echo "\nTesting qFuzzyCompare:\n";
echo "qFuzzyCompare(4.0000000000001,4) = ";
if(qFuzzyCompare(4.0000000000001,4))
	echo "TRUE\n";
else
	echo "FALSE\n";
echo "qFuzzyCompare(4.1,4) = ";
if(qFuzzyCompare(4.1,4))
	        echo "TRUE\n";
else
	        echo "FALSE\n";

echo "\nTesting qVersion:\n";
echo "qVersion() = ".qVersion()."\n";

echo "\nTesting PHPQtVersion:\n";
echo "PHPQtVersion() = ".PHPQtVersion()."\n";

echo "\nTesting QiDiVersion:\n";
echo "QiDiVersion() = ".QiDiVersion()."\n";

echo "\nTesting qMalloc:\n";
print_r(var_dump(qMalloc(10)));
echo "\n";

echo "\nTesting qFree:\n";
echo "a = 5...";
$a = 5;
qFree($a);
if(isset($a))
	echo "qFree failed\n";
else
	echo "a succsessfully freed\n";

echo "\nTesting qMemCopy:\n";
echo "a='abcdefg'\n";
echo "b=222.34\n";
echo 'qMemCopy($b,$a,2)=';
$a = "abcdefg";
$b = 222.34;
qMemCopy($b,$a,2);
echo $b."\n";
echo "a=333333\n";
echo "b=222\n";
echo 'qMemCopy($b,$a,2)=';
$a = 333333;
$b = 222;
qMemCopy($b,$a,2);
echo $b."\n";
echo "a=3.33333\n";
echo "b=222.56\n";
echo 'qMemCopy($b,$a,2)=';
$a = 3.33333;
$b = 222.56;
qMemCopy($b,$a,2);
echo $b."\n";

echo "\nTesting qt_noop:";
qt_noop();
echo "done\n";

echo "\nTesting Q_ASSERT:\n";
echo "Q_ASSERT(5==5): ";
Q_ASSERT(5==5);
echo "done\n";
//echo "Q_ASSERT(6==5): ";
//Q_ASSERT(6==5);
//echo "done\n";

echo "\nTesting Q_ASSERT_X:\n";
echo "Q_ASSERT(5==5,'in our test file', 'oh no! it failed!'): ";
Q_ASSERT_X(5==5,'in our test file','oh no! it failed!');
echo "done\n";
//echo "Q_ASSERT(6==5,'in our test file', 'oh no! it worked!'): ";
//Q_ASSERT_X(6==5,'in our test file','oh no! it worked!');
//echo "done\n";

echo "\nTesting qDebug:\n";
qDebug("This is a debug message");

echo "Testing qWarning:\n";
qWarning("This is a warning message");

echo "Testing qCritical:\n";
qCritical("This is a fatal error");

echo "We shouldn't see this\n";

?>
