<?php

    /****************************************************************
    **
    ** Qt Calculator
    **
    ** original:
    ** http://doc.trolltech.com/4.1/widgets-calculator.html
    **
    ****************************************************************/

    require_once('button.php');

    class Calculator extends QDialog {

        private $sumInMemory = 0.0;
        private $sumSoFar = 0.0;
        private $factorSoFar = 0.0;
        private $pendingAdditiveOperator;
        private $pendingMultiplicativeOperator;
        private $waitingForOperand = true;

        private $display;

        const NumDigitButtons = 10;
        private $digitButtons;

        private $pointButton;
        private $changeSignButton;
        private $backspaceButton;
        private $clearButton;
        private $clearAllButton;
        private $clearMemoryButton;
        private $readMemoryButton;
        private $setMemoryButton;
        private $addToMemoryButton;

        private $divisionButton;
        private $timesButton;
        private $minusButton;
        private $plusButton;
        private $squareRootButton;
        private $powerButton;
        private $reciprocalButton;
        private $equalButton;

        private $slots = array(
            "digitClicked()",
            "unaryOperatorClicked()",
            "additiveOperatorClicked()",
            "multiplicativeOperatorClicked()",
            "equalClicked()",
            "pointClicked()",
            "changeSignClicked()",
            "backspaceClicked()",
            "clear()",
            "clearAll()",
            "clearMemory()",
            "read_Memory()",
            "setMemory()",
            "addToMemory()"
        );
        private $signals = array("");

        function __construct()
        {
            parent::__construct();

            $this->pendingAdditiveOperator = new QString();
            $this->pendingMultiplicativeOperator = new QString();

            $this->display = new QLineEdit("0");
            $this->display->setReadOnly(true);
            $this->display->setAlignment(Qt::AlignRight);
            $this->display->setMaxLength(15);
            $this->display->installEventFilter($this);

            $font = $this->display->font();
            $font->setPointSize($font->pointSize() + 8);
            $this->display->setFont($font);

            $digitColor = new QColor(150, 205, 205);
            $backspaceColor = new QColor(225, 185, 135);
            $memoryColor = new QColor(100, 155, 155);
            $operatorColor = new QColor(155, 175, 195);

            for ($i = 0; $i < Calculator::NumDigitButtons; ++$i) {
                    $this->digitButtons[$i] = $this->createButton(tr($i), $digitColor, SLOT('digitClicked()'));
            }

            $this->pointButton = $this->createButton(tr("."), $digitColor, SLOT('pointClicked()'));
            $this->changeSignButton = $this->createButton(tr("\261"), $digitColor, SLOT('changeSignClicked()'));

            $this->backspaceButton = $this->createButton(tr("Backspace"), $backspaceColor, SLOT('backspaceClicked()'));
            $this->clearButton = $this->createButton(tr("Clear"), $backspaceColor, SLOT('clear()'));
            $this->clearAllButton = $this->createButton(tr("Clear All"), $backspaceColor, SLOT('clearAll()'));

            $this->clearMemoryButton = $this->createButton(tr("MC"), $memoryColor, SLOT('clearMemory()'));
            $this->readMemoryButton = $this->createButton(tr("MR"), $memoryColor, SLOT('read_Memory()'));
            $this->setMemoryButton = $this->createButton(tr("MS"), $memoryColor, SLOT('setMemory()'));
            $this->addToMemoryButton = $this->createButton(tr("M+"), $memoryColor, SLOT('addToMemory()'));

            $this->divisionButton = $this->createButton(tr("\367"), $operatorColor, SLOT('multiplicativeOperatorClicked()'));

            $this->timesButton = $this->createButton(tr("\327"), $operatorColor, SLOT('multiplicativeOperatorClicked()'));
            $this->minusButton = $this->createButton(tr("-"), $operatorColor, SLOT('additiveOperatorClicked()'));
            $this->plusButton = $this->createButton(tr("+"), $operatorColor, SLOT('additiveOperatorClicked()'));

            $this->squareRootButton = $this->createButton(tr("Sqrt"), $operatorColor, SLOT('unaryOperatorClicked()'));
            $this->powerButton = $this->createButton(tr("x\262"), $operatorColor, SLOT('unaryOperatorClicked()'));
            $this->reciprocalButton = $this->createButton(tr("1/x"), $operatorColor, SLOT('unaryOperatorClicked()'));
            $this->equalButton = $this->createButton(tr("="), $operatorColor, SLOT('equalClicked()'));

            $this->mainLayout = new QGridLayout($this);
            $this->mainLayout->setSizeConstraint(QLayout::SetFixedSize);

            $this->mainLayout->addWidget($this->display, 0, 0, 1, 6);
            $this->mainLayout->addWidget($this->backspaceButton, 1, 0, 1, 2);
            $this->mainLayout->addWidget($this->clearButton, 1, 2, 1, 2);
            $this->mainLayout->addWidget($this->clearAllButton, 1, 4, 1, 2);

            $this->mainLayout->addWidget($this->clearMemoryButton, 2, 0);
            $this->mainLayout->addWidget($this->readMemoryButton, 3, 0);
            $this->mainLayout->addWidget($this->setMemoryButton, 4, 0);
            $this->mainLayout->addWidget($this->addToMemoryButton, 5, 0);

            for ($i = 1; $i < Calculator::NumDigitButtons; ++$i) {
                    $row = ceil(((8 - ($i+1))) / 3) + 2;
                    $column = ceil((($i-1) % 3)) + 1;
                    $this->mainLayout->addWidget($this->digitButtons[$i], (int) $row, (int) $column);
            }

            $this->mainLayout->addWidget($this->digitButtons[0], 5, 1);
            $this->mainLayout->addWidget($this->pointButton, 5, 2);
            $this->mainLayout->addWidget($this->changeSignButton, 5, 3);

            $this->mainLayout->addWidget($this->divisionButton, 2, 4);
            $this->mainLayout->addWidget($this->timesButton, 3, 4);
            $this->mainLayout->addWidget($this->minusButton, 4, 4);
            $this->mainLayout->addWidget($this->plusButton, 5, 4);

            $this->mainLayout->addWidget($this->squareRootButton, 2, 5);
            $this->mainLayout->addWidget($this->powerButton, 3, 5);
            $this->mainLayout->addWidget($this->reciprocalButton, 4, 5);
            $this->mainLayout->addWidget($this->equalButton, 5, 5);

            $this->setLayout($this->mainLayout);
            $this->setWindowTitle(tr("Calculator"));

        }

        function eventFilter($target, $event){
            if ($target == $display) {
                if ($event->type() == QEvent::MouseButtonPress
                    || $event->type() == QEvent::MouseButtonDblClick
                    || $event->type() == QEvent::MouseButtonRelease
                    || $event->type() == QEvent::ContextMenu) {

                    $mouseEvent = $event;
                    if ($mouseEvent->buttons() & Qt::LeftButton) {
                        $newPalette = $this->palette();
                        $newPalette->setColor(QPalette::Base,
                            $this->display->palette()->color(QPalette::Text));
                        $newPalette->setColor(QPalette::Text,
                            $this->display->palette()->color(QPalette::Base));
                        $this->display->setPalette($newPalette);
                    } else {
                        $this->display->setPalette($palette());
                    }
                    return true;
                }
            }
            return parent::eventFilter($target, $event);
        }

        function digitClicked()
        {

            $clickedButton = qobject_cast($this->sender(), new QToolButton());
            $digitValue = $clickedButton->text()->toInt();

            if ($this->display->text() == "0" && $digitValue == 0.0)
                return;

            if ($this->waitingForOperand) {
                $this->display->clear();
                $this->waitingForOperand = false;
            }

            $this->display->setText($this->display->text()->append(QString::number($digitValue)));
        }

        function unaryOperatorClicked()
        {
            $clickedButton = qobject_cast($this->sender(), $this->pointButton);
            $clickedOperator = $clickedButton->text();
            $operand = $this->display->text()->toDouble();
            $result = 0.0;

            if ($clickedOperator->__toString() == "Sqrt") {
                if ($operand < 0.0) {
                    $this->abortOperation();
                    return;
                }
                $result = sqrt($operand);
            } else if ($clickedOperator->__toString() == tr("x\262")) {
                $result = pow($operand, 2.0);
            } else if ($clickedOperator->__toString() == tr("1/x")) {
                if ($operand == 0.0) {
                    $this->abortOperation();
                    return;
                }
                $result = 1.0 / $operand;
            }

            $this->display->setText(QString::number($result));
            $this->waitingForOperand = true;
        }

        function additiveOperatorClicked()
        {
            $clickedButton = qobject_cast($this->sender(), $this->pointButton);
            $clickedOperator = $clickedButton->text();
            $operand = $this->display->text()->toDouble();

            if (!$this->pendingMultiplicativeOperator->isEmpty()) {
                if (!$this->calculate($operand, $this->pendingMultiplicativeOperator)) {
                    $this->abortOperation();
                    return;
                }
                $this->display->setText(QString::number($this->factorSoFar));
                $operand = $this->factorSoFar;
                $this->factorSoFar = 0.0;
                $this->pendingMultiplicativeOperator->clear();
            }

            if (!$this->pendingAdditiveOperator->isEmpty()) {
                if (!$this->calculate($operand, $this->pendingAdditiveOperator)) {
                    $this->abortOperation();
                    return;
                }
                $this->display->setText(QString::number($this->sumSoFar));
            } else {
                $this->sumSoFar = $operand;
            }

            $this->pendingAdditiveOperator = $clickedOperator;
            $this->waitingForOperand = true;

        }

        function multiplicativeOperatorClicked()
        {
            $clickedButton = qobject_cast($this->sender(), $this->pointButton);
            $clickedOperator = $clickedButton->text();
            $operand = $this->display->text()->toDouble();

            if (!$this->pendingMultiplicativeOperator->isEmpty()) {
                if (!$this->calculate($operand, $this->pendingMultiplicativeOperator)) {
                    $this->abortOperation();
                    return;
                }
                $this->display->setText(QString::number($this->factorSoFar));
            } else {
                $this->factorSoFar = $operand;
            }
            $this->pendingMultiplicativeOperator = $clickedOperator;
            $this->waitingForOperand = true;
        }

        function equalClicked()
        {
            $operand = $this->display->text()->toDouble();

            if (!$this->pendingMultiplicativeOperator->isEmpty()) {
                if (!$this->calculate($operand, $this->pendingMultiplicativeOperator)) {
                    $this->abortOperation();
                    return;
                }
                $operand = $this->factorSoFar;
                $this->factorSoFar = 0.0;
                $this->pendingMultiplicativeOperator->clear();
            }
            if (!$this->pendingAdditiveOperator->isEmpty()) {
                if (!$this->calculate($operand, $this->pendingAdditiveOperator)) {
                    $this->abortOperation();
                    return;
                }
                $this->pendingAdditiveOperator->clear();
            } else {
                $this->sumSoFar = $operand;
            }

            $this->display->setText(QString::number($this->sumSoFar));
            $this->sumSoFar = 0.0;
            $this->waitingForOperand = true;                                                                          
        }

        function pointClicked()
        {
            if ($this->waitingForOperand)
                $this->display->setText(tr("0"));

            if (!$this->display->text()->contains(tr(".")))
                $this->display->setText($this->display->text()->append(tr(".")));

            $this->waitingForOperand = false;
        }

        function changeSignClicked()
        {
            $text = $this->display->text();
            $value = $text->toDouble();

            if ($value > 0.0) {
                $text->prepend(tr("-"));
            } else if ($value < 0.0) {
                $text->remove(0, 1);
            }
            $this->display->setText($text);
        }

        function backspaceClicked()
        {
            if ($this->waitingForOperand)
                return;

            $text = $this->display->text();
            $text->chop(1);
            if ($text->isEmpty()) {
                $text = tr("0");
                $this->waitingForOperand = true;
            }
            $this->display->setText($text);
        }

        function clear()
        {
            if ($this->waitingForOperand)
                return;

            $this->display->setText(tr("0"));
            $this->waitingForOperand = true;
        }

        function clearAll()
        {
            $this->sumSoFar = 0.0;
            $this->factorSoFar = 0.0;
            $this->pendingAdditiveOperator->clear();
            $this->pendingMultiplicativeOperator->clear();
            $this->display->setText(tr("0"));
            $this->waitingForOperand = true;
        }

        function clearMemory()
        {
            $this->sumInMemory = 0.0;
        }

        function read_Memory()
        {
            $this->display->setText(QString::number($this->sumInMemory));
            $this->waitingForOperand = true;
        }

        function setMemory()
        {
            $this->equalClicked();
            $this->sumInMemory = $this->display->text()->toDouble();
        }

        function addToMemory()
        {
            $this->equalClicked();
            $this->sumInMemory += $this->display->text()->toDouble();
        }

        function createButton($text, $color, $member)
        {
            $button = new Button($text, $color);
            $this->connect($button, SIGNAL('clicked()'), $this, $member);
            return $button;
        }

        function abortOperation()
        {
            $this->clearAll();
            $this->display->setText(tr("####"));
        }

        function calculate($rightOperand, $pendingOperator)
        {
            if ($pendingOperator->__toString() == "+") {
                $this->sumSoFar += $rightOperand;
            } else if ($pendingOperator->__toString() == "-") {
                $this->sumSoFar -= $rightOperand;
            } else if ($pendingOperator->__toString() == tr("\327")) {
                $this->factorSoFar *= $rightOperand;
            } else if ($pendingOperator->__toString() == tr("\367")) {
                if ($rightOperand == 0.0)
                    return false;
                $this->factorSoFar /= $rightOperand;
            }
            return true;
        }
    }

?>