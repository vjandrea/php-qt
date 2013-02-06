<?php

	/****************************************************************
	**
	** Qt tutorial 1
	**
	** original:
	** http://doc.trolltech.com/4.1/tutorial-t1.html
	**
	****************************************************************/

	if(!extension_loaded('php_qt')) {
		dl('php_qt.' . PHP_SHLIB_SUFFIX);
	}

	$app = new QApplication($argc,$argv);
	
	$hello = new QPushButton("Hello world!");
	$hello->resize(100, 30);
	
	$hello->show();
	$app->exec();

?>
