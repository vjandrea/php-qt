#!/usr/bin/perl

# qtguess.pl : check how Qt was compiled. Issue a list of all defined QT_NO_* macros, one per line.
#
# author:  germain Garand <germain@ebooksfrance.com>
# licence: GPL v.2

# options: -q: be quieter
#	   -o file: redirect output to "file". (default: ./qtdefines)
#	   -t [0..15]: set the testing threshold (see below)
#	   -f "flags": additional compiler flags/parameters

use Getopt::Std;

use vars qw/$opt_f $opt_o $opt_p/;

getopts('qo:f:t:');

# Threshold :
#	 0 - test basic Qt types/classes
#	 5 - test higher level, non-gui classes
#	 8 - test options of the above (ex: QT_NO_IMAGE_SMOOTHSCALE)
#	10 - test basic widgets
#	12 - test composite widgets
#	13 - test widgets inheriting composite widgets
#	15 - test goodies (default)

my $default_threshold = @qt_test_threshold@;
my $cc = "@CMAKE_CXX_COMPILER@";
my $ccflags = $opt_f || "@CMAKE_CXX_FLAGS@";

my $nspaces = 50;

my %qtdefs=();
my %qtundefs=();

my $tmp = gettmpfile();
my $qtcoreinc = '@QT_QTCORE_INCLUDE_DIR@';
my $qtinc = '@QT_INCLUDE_DIR@';
my $allinc = '@all_includes@';
my $alllib = '@all_libs@';

 -e "$qtcoreinc/qglobal.h" or die "Invalid Qt include directory.\n";

my $ccmd = "$cc $ccflags $allinc $alllib -o $tmp $tmp.cpp";

my $threshold = defined($opt_t)?$opt_t : $default_threshold;
$threshold >= 0 or die "invalid testing threshold: $threshold\n";

print "Checking how Qt was built... \n";
print "Threshold is set to $threshold\n" unless $opt_q;

my($count, $used, $total);
map{ $tests{$_}->[2]>=$threshold ? ($used++, $total++):$total++ } keys %tests;

print "Number of defines to be tested : $used/$total\n\n" unless $opt_q;
open( QTDEFS, ">>".($opt_o || "qtdefines") ) or die "Can't open output file: $!\n";

grab_qglobal_symbols();
preliminary_test();
perform_all_tests();

print +scalar(keys %qtdefs) . " defines found.\n";

print QTDEFS join("\n", keys %qtdefs), "\n";
close;

#--------------------------------------------------------------#

sub grab_qglobal_symbols
{
	my $cmd = "$cc -E -D__cplusplus -dM -I$qtinc -I$qtcoreinc $qtcoreinc/qglobal.h 2>/dev/null";
	my $symbols = `$cmd`;
        for(0..1)
        {
	    if( check_exit_status($?) )
	    {
		while( $symbols =~/^#\s*define\s*(QT_\S+)\s*$/gm )
		{
			$qtdefs{$1} = 1;
		}
		print "Found ". scalar( keys %qtdefs )." predefined symbol".((scalar( keys %qtdefs ) -1)?"s":"")." in qglobal.h\n" unless ($opt_q or !(keys %qtdefs));
		while( $symbols =~/^#\s*define\s*QT_MODULE_(\S+)\s*$/gm )
		{
			$qtundefs{"QT_NO_$1"} = 1;
		}
		print "Found ". scalar( keys %qtundefs )." undefined symbol".((scalar( keys %qtundefs ) -1)?"s":"")." in qglobal.h\n" unless ($opt_q or !(keys %qtundefs));
                last;
	    }
	    elsif(! $_) # first try
	    {
		print  "Failed to run $cmd.\nTrying without __cplusplus (might be already defined)\n";
                $cmd = "$cc -E -dM -I$qtinc -I$qtcoreinc $qtcoreinc/qglobal.h 2>/dev/null";
                $symbols = `$cmd`;
                next;
	    }
        }
}

#--------------------------------------------------------------#

sub gettmpfile
{
	my $tmpdir = $ENV{'TMP'} || ".";
	my $tmpname = $$."-qtguess";
	while( -e "$tmpdir/$tmpname" || -e "$tmpdir/${tmpname}.cpp" )
	{
		$tmpname .= int (rand * 9);
	}
	return "$tmpdir/$tmpname";
}


#--------------------------------------------------------------#

sub preliminary_test
{
	my $msg = "Trying to compile and link a small program...";
	print $msg, " " x ($nspaces - length($msg) + 8);
	open( OUT, ">${tmp}.cpp" ) or die "Failed to open temp file ${tmp}.cpp: $!\n";
	my $simple=q"
		#include <QtGui/qapplication.h>
		int main( int argc, char **argv )
		{
			QApplication foo( argc, argv );
			return 0;
		}
	";
	print OUT $simple;
	close OUT;
        my $out = `$ccmd 2>&1`;
	if( !check_exit_status($?) )
	{
		die <<"EOF";

FAILED : check your configuration.
Failed program was:
$simple
Compiled with:
$ccmd
Compiler output:
$out
EOF
	}
	else
	{
		print "OK\n";
	}
}

#--------------------------------------------------------------#

sub perform_all_tests
{
	foreach ( sort { $tests{$a}->[2] <=> $tests{$b}->[2]} keys %tests)
	{
		$tests{$_}->[2] < $threshold and next;
		($qtdefs{$_} || $qtundefs{$_}) and do
		{
			print "\rSkipping $_ (in qglobal.h)".( " " x (($nspaces-16) - length($_)) ).($qtundefs{$_}?"*Undefined*":" [Defined]").($opt_q?"":"\n");
			next
		};
		print "\rTesting $_".( " " x ($nspaces - length($_)) );
		open( OUT, ">${tmp}.cpp" ) or die "Failed to open temp file ${tmp}.cpp: $!\n";
		foreach $def(keys %qtdefs)
		{
			print OUT "#define $def\n";
		}
		foreach $inc(split /,\s*/, $tests{$_}->[0])
		{
			print OUT "#include <$inc>\n";
		}
		print OUT "#include <QtCore/qfeatures.h>\n";
		print OUT $tests{$_}->[3] if $tests{$_}->[3]; # need to define some classes ?
		print OUT qq"

		int main( int argc, char ** argv)
		{
		$tests{$_}->[1]
		return 0;
		}

		";
		close OUT;

                my $out = `$ccmd 2>&1`;
		my $ok = check_exit_status($?);
		if( !$ok )
		{
			$qtdefs{$_} = 1;
		}
		print +$opt_q ? ++$count."/$used" : ( $ok ? "*Undefined*\n" : " [Defined]\n" );
	}
	$opt_q && print "\n";
}

#--------------------------------------------------------------#

sub check_exit_status
{
	my $a = 0xFFFF & shift;
	if( !$a )
	{
		return 1;
	}
	elsif( $a == 0xFF00 )
	{
		die "\nSystem call failed: $!\n";
	}
	elsif( $a > 0x80 )
	{
		# non-zero status.
	}
	else
	{
		if( $a & 0x80 )
		{
			die "\n$cc coredumped with signal ". ($a & ~0x80);
		}
		die "\n$cc interrupted by signal $a\n";
	}
	return 0;
}

#--------------------------------------------------------------#

END
{
	unlink $tmp if -e $tmp;
	unlink "${tmp}.cpp" if -e "${tmp}.cpp";
}

#--------------------------------------------------------------#

BEGIN {

# "DEFINE" => ["header-1.h,... header-n.h", "main() code", priority, "Definitions (if needed)"]

our %tests = (
	"QT_NO_ACCESSIBILITY" =>        ["QtGui/qaccessible.h", "QAccessibleEvent event(QEvent::AccessibilityHelp, 0);", 15],
	"QT_NO_ACTION" =>		["QtGui/qaction.h", "QAction foo( (QObject*)NULL );", 5],
 	"QT_NO_BUTTONGROUP" =>		["QtGui/qbuttongroup.h", "QButtonGroup foo( (QObject*)NULL );", 12],
 	"QT_NO_CHECKBOX" =>		["QtGui/qcheckbox.h", "QCheckBox foo( (QWidget*)NULL );", 10],
	"QT_NO_CLIPBOARD" => 		["QtGui/qapplication.h, QtGui/qclipboard.h", q"
						QApplication foo( argc, argv );
						QClipboard *baz= foo.clipboard();
					", 5],
 	"QT_NO_COLORDIALOG" =>		["QtGui/qcolordialog.h", "QColorDialog::customCount();", 12],
 	"QT_NO_COMBOBOX" =>		["QtGui/qcombobox.h", "QComboBox foo( (QWidget*)NULL );", 10],
	"QT_NO_COMPAT" =>		["QtGui/qfontmetrics.h", q"
						QFontMetrics *foo= new QFontMetrics( QFont() );
						int bar = foo->width( 'c' );
					", 0],
	"QT_NO_COMPONENT" =>		["QtGui/qapplication.h", q"
 						QApplication foo( argc, argv );
 						foo.addLibraryPath( QString::null );
					", 5],
 	"QT_NO_CURSOR" =>		["QtGui/qcursor.h", "QCursor foo;", 5],
 	"QT_NO_DATASTREAM" =>		["QtCore/qdatastream.h", "QDataStream foo;", 5],
 	"QT_NO_DATETIMEEDIT" =>		["QtGui/qdatetimeedit.h", "QTimeEdit foo;", 12],
	"QT_NO_DIAL" =>			["QtGui/qdial.h", "QDial foo;", 10],
	"QT_NO_DIALOG" =>		["QtGui/qdialog.h", "QDialog foo;", 12],
	"QT_NO_DIR" =>			["QtCore/qdir.h", "QDir foo;", 5],
	"QT_NO_DOM" =>			["QtXml/qdom.h", "QDomDocumentType foo;", 5],
	"QT_NO_DRAGANDDROP" =>		["QtGui/qevent.h", "QDropEvent foo( QDropEvent() );", 5],
	"QT_NO_DRAWUTIL" =>		["QtGui/qdrawutil.h, QtGui/qcolor.h", "qDrawPlainRect( (QPainter *) NULL, 0, 0, 0, 0, QColor() );", 10],
	"QT_NO_ERRORMESSAGE" => 	["QtGui/qerrormessage.h", "QErrorMessage foo( (QWidget*) NULL );", 13],
	"QT_NO_FILEDIALOG" =>		["QtGui/qfiledialog.h", "QFileDialog foo();", 13],

	"QT_NO_FONTDATABASE" =>		["QtGui/qfontdatabase.h", "QFontDatabase foo;", 5],
	"QT_NO_FONTDIALOG" => 		["QtGui/qfontdialog.h",   "QFontDialog::getFont( (bool *)NULL );", 12],
	"QT_NO_FRAME" => 		["QtGui/qframe.h", "QFrame foo;", 10],
 	"QT_NO_FTP" =>	["QtNetwork/qftp.h", "QFtp foo;", 9],
	"QT_NO_GROUPBOX" =>		["QtGui/qgroupbox.h", "QGroupBox foo;", 12],
 	"QT_NO_IMAGE_HEURISTIC_MASK" =>	["QtGui/qimage.h", q"
						QImage *foo = new QImage;
						foo->createHeuristicMask();
					", 8],
	"QT_NO_IMAGEIO" => 		["QtGui/qbitmap.h, QtCore/qstring.h", q"
						QBitmap foo( QString::fromLatin1('foobar') );
					", 5],
	"QT_NO_LABEL" =>		["QtGui/qlabel.h", "QLabel foo( (QWidget*) NULL );", 10],
	"QT_NO_LAYOUT" =>		["QtGui/qlayout.h", "QFoo foo;", 10, q"
 						class QFoo: public QLayout
 						{
 						public:
 						    QFoo() {};
 						    ~QFoo() {};
 						    void addItem( QLayoutItem * ) { };
						    QSize sizeHint() const { return QSize(); }
						    void setGeometry( const QRect & ) { };
                            QLayoutItem* itemAt(int) const {return (QLayoutItem*) NULL;}
                            QLayoutItem* takeAt(int) {return (QLayoutItem*) NULL;}
                            int count() const {return 0;}

 						};
 					"],
	"QT_NO_LCDNUMBER" =>		["QtGui/qlcdnumber.h", "QLCDNumber foo;", 12],
	"QT_NO_LINEEDIT" =>		["QtGui/qlineedit.h", "QLineEdit foo( (QWidget *) NULL );", 12],
	"QT_NO_LISTVIEW" =>		["QtGui/qlistview.h", "QListView foo;", 13],
	"QT_NO_MAINWINDOW" =>		["QtGui/qmainwindow.h", "QMainWindow foo;", 13],
	"QT_NO_MATRIX" =>		["QtGui/qmatrix.h", "QMatrix foo;", 0],
	"QT_NO_MENUBAR" =>		["QtGui/qmenubar.h", "QMenuBar foo;", 13],
	"QT_NO_MESSAGEBOX" =>		["QtGui/qmessagebox.h", "QMessageBox foo;", 13],
	"QT_NO_MOVIE" =>		["QtGui/qmovie.h", "QMovie foo;", 5],
	"QT_NO_NETWORK" =>		["QtNetwork/qnetworkproxy.h", "QNetworkProxy foo;", 5],
	"QT_NO_PALETTE" =>		["QtGui/qpalette.h", "QPalette foo;", 5],
	"QT_NO_PICTURE" =>		["QtGui/qpicture.h", "QPicture foo;", 5],
	"QT_NO_PRINTER" =>		["QtGui/qprinter.h", "QPrinter foo();", 5],
	"QT_NO_PRINTDIALOG" =>		["QtGui/qprintdialog.h", "QPrintDialog foo( (QPrinter*) NULL );", 13],
	"QT_NO_PROCESS" =>		["QtCore/qprocess.h", "QProcess foo;", 5],
	"QT_NO_PROGRESSBAR" =>		["QtGui/qprogressbar.h", "QProgressBar foo;", 12],
	"QT_NO_PROGRESSDIALOG" =>	["QtGui/qprogressdialog.h", "QProgressDialog foo;", 13],
	"QT_NO_PROPERTIES" =>		["QtCore/qmetaobject.h", "QMetaProperty foo;", 0],
	"QT_NO_PUSHBUTTON" =>		["QtGui/qpushbutton.h", "QPushButton foo( (QWidget *) NULL );", 12],
	"QT_NO_QUUID_STRING" =>		["QtCore/quuid.h", "QUuid foo( QString() );", 8],
	"QT_NO_RADIOBUTTON" =>		["QtGui/qradiobutton.h", "QRadioButton foo( (QWidget *) NULL );", 12],
	"QT_NO_SCROLLBAR" =>		["QtGui/qscrollbar.h", "QScrollBar foo( (QWidget *) NULL );", 12],
	"QT_NO_SESSIONMANAGER" =>	["QtGui/qapplication.h", q"
  						QApplication foo( argc, argv );
  						foo.sessionId();
					", 15],
	"QT_NO_SETTINGS" =>		["QtCore/qsettings.h", "QSettings foo;", 5],
	"QT_NO_SIGNALMAPPER" =>		["QtCore/qsignalmapper.h", "QSignalMapper foo( (QObject *) NULL );", 0],
	"QT_NO_SIZEGRIP" =>		["QtGui/qsizegrip.h", "QSizeGrip foo( (QWidget *) NULL );", 10],
	"QT_NO_SLIDER" =>		["QtGui/qslider.h", "QSlider foo( (QWidget *) NULL );", 12],
	"QT_NO_SOUND" =>		["QtGui/qsound.h", "QSound foo( QString::null );", 5],
	"QT_NO_STRINGLIST" =>		["qstringlist.h", "QStringList foo;", 0],
	"QT_NO_STYLE_PLASTIQUE" =>		["QtGui/qplastiquestyle.h", "QPlastiqueStyle foo;", 16],
 	"QT_NO_STYLE_WINDOWSXP" =>	["QtGui/qwindowsxpstyle.h", "QWindowsXPStyle foo;", 16],
 	"QT_NO_STYLE_WINDOWS" =>	["QtGui/qwindowsstyle.h", "QWindowsStyle foo;", 16],
	"QT_NO_STYLE_MAC" =>	["QtGui/macstyle.h", "QMacStyle foo;", 16],
	"QT_NO_STYLE_CLEANLOOKS" =>	["QtGui/qcleanlooksstyle.h", "QCleanlooksStyle foo;", 16],
    "QT_NO_TABBAR" =>               ["QtGui/qtabbar.h", "QTabBar foo;", 10],
	"QT_NO_TABLEVIEW" =>		["QtGui/qtableview.h", "QTableView foo;", 16],
    "QT_NO_TABWIDGET" =>            ["QtGui/qtabwidget.h", "QTabWidget foo;", 10],
    "QT_NO_TEXTBROWSER" =>          ["QtGui/qtextbrowser.h", "QTextBrowser foo;", 14],
    "QT_NO_TEXTCODEC" =>            ["QtCore/qtextcodec.h", "QTextCodec::codecForMib(1);", 5],
    "QT_NO_TEXTCODECPLUGIN" =>      ["QtCore/qtextcodecplugin.h, QtCore/qstringlist.h, QtCore/qlist.h, QtCore/qtextcodec.h", "QFoo foo;", 6, q"
						class QFoo: public QTextCodecPlugin
						{
						public:
						    QFoo() {};
						    ~QFoo() {};
                            QList<QByteArray> names() const {return QList<QByteArray>();}
                            QList<int>mibEnums() const {return QList<int>();}
                            QTextCodec *createForName( const QByteArray & name ) {return (QTextCodec *)NULL;}
                            QTextCodec *createForMib( int mib ) {return (QTextCodec *)NULL;}
                            QList<QByteArray> aliases() const {return QList<QByteArray>();}
						};
						Q_EXPORT_PLUGIN2( 'Foo', QFoo )
					"],
 	"QT_NO_TEXTEDIT" =>		["QtGui/qtextedit.h", "QTextEdit foo;", 13], 
    "QT_NO_TEXTSTREAM" =>           ["QtCore/qtextstream.h", "QTextStream foo;", 5],
    "QT_NO_THREAD" =>           ["QtCore/qthread.h", "QFoo foo;", 5, q"
						class QFoo: public QThread
						{
						public:
						    QFoo() {};
						    ~QFoo() {};
                            void run() {}
						};
					"],
    "QT_NO_TOOLBAR" =>              ["QtGui/qtoolbar.h", "QToolBar foo;", 10],
    "QT_NO_TOOLBUTTON" =>           ["QtGui/qtoolbutton.h", "QToolButton foo((QWidget *) NULL );", 12],
    "QT_NO_TOOLTIP" =>              ["QtGui/qtooltip.h", "QToolTip::palette();", 10],
    "QT_NO_VARIANT" =>              ["QtCore/qvariant.h", "QVariant foo;", 0],
    "QT_NO_WHATSTHIS" =>            ["QtGui/qwhatsthis.h", "QWhatsThis::inWhatsThisMode();", 10],
	"QT_NO_WHEELEVENT" =>		["QtGui/qevent.h", "QWheelEvent foo( QPoint(1,1), 1, (Qt::MouseButtons)1, 0 );", 5],
	"QT_NO_XML" =>			["QtXml/qxml.h", "QXmlNamespaceSupport foo;", 5],

	);

}
