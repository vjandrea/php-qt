<?php

    /****************************************************************
    **
    ** Qt Calculator
    **
    ** original:
    ** http://doc.trolltech.com/4.1/widgets-calculator.html
    **
    ****************************************************************/

    class Button extends QToolButton {

        function __construct($text, $color)
        {
            parent::__construct();

            $this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
            $this->setText($text);

    	    $newPalette = new QPalette();
            $newPalette->setColor(QPalette::Button, $color);
            $this->setPalette($newPalette);
        }
    }

?>