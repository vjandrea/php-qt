<?php

include('ui_imagedialog.php');

$app = new QApplication($argc, $argv);
$window = new QDialog;

$ui = new Ui_Dialog();
$ui->setupUi($window);

$window->show(); 
$app->exec();

?>