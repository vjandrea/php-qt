#!/usr/bin/perl -w

## Run this first, to generate the x_*.cpp files from the Qt headers
## using kalyptus

my $kalyptusdir = "@CMAKE_CURRENT_SOURCE_DIR@/../../kalyptus";

use File::Basename;
use File::Copy qw|cp|;
use File::Compare;
use File::Find;
use Cwd;

my $here = getcwd;
my $outdir = $here . "/generate.pl.tmpdir";
my $finaloutdir = $here;
my $defines = "qtdefines";
my $headerlist = "@CMAKE_CURRENT_SOURCE_DIR@/header_list";
my $definespath = "$here/$defines";
my $headerlistpath = "$headerlist";

my $kdeheaderlist = "@CMAKE_CURRENT_SOURCE_DIR@/kde_header_list";
my $kdeheaderlistpath = "$here/$kdeheaderlist";

## If srcdir != builddir, use headerlist from src
$headerlistpath = $headerlist if ($headerlist =~ /^\//);
$kdeheaderlistpath = $kdeheaderlist if $kdeheaderlist =~ /^\//;

## Note: outdir and finaloutdir should NOT be the same dir!

# Delete all x_*.cpp files under outdir (or create outdir if nonexistent)
if (-d $outdir) { system "rm -f $outdir/x_*.cpp"; } else { mkdir $outdir; }

mkdir $finaloutdir unless (-d $finaloutdir);

#  Load the QT_NO_* macros found in "qtdefines". They'll be passed to kalyptus
my $macros="";
if ( -e $definespath ){
    print "Found '$defines'. Reading preprocessor symbols from there...\n";
    $macros = " --defines=$definespath ";
}

mkdir $kalyptusdir, 0777;
# Need to cd to kalyptus's directory so that perl finds Ast.pm etc.
chdir "$kalyptusdir" or die "Couldn't go to $kalyptusdir (edit script to change dir)\n";

# Find out which header files we need to parse
# We don't want all of them - e.g. not template-based stuff
my %excludes = (
);

# Some systems have a QTDIR = KDEDIR = PREFIX
# We need a complete list

my %includes;
open(HEADERS, $headerlistpath) or die "Couldn't open $headerlistpath: $!\n";
map { chomp ; $includes{$_} = 1 } <HEADERS>;
close HEADERS;

# Find out which header files we need to parse
# We don't want all of them - e.g. not template-based stuff
my %kdeexcludes = (
#   These headers don't look suitable for inclusion:
	'kallocator.h' => 1,
	'kbookmarkimporter_crash.h' => 1,
	'kbookmarkimporter_ie.h' => 1,
	'kbookmarkimporter_opera.h' => 1,
	'kbookmarkimporter_ns.h' => 1,
	'kcrash.h' => 1,
	'kdebug.h' => 1,
	'kde_terminal_interface.h' => 1,

#	These headers currently give problems
	'kio/uiserver.h' => 1,
	'kio/thumbcreator.h' => 1,
	'kio/file.h' => 1,
	'kio/chmodjob.h' => 1,
	'kparts/genericfactory.h' => 1,
	'kopenssl.h' => 1,
	'kautomount.h' => 1,
	'kimagefilepreview.h' => 1,
	'kpropertiesdialog.h' => 1,
	'knotifydialog.h' => 1,
);

# Some systems have a QTDIR = KDEDIR = PREFIX
# We need a complete list

my %kdeincludes;
open(HEADERS, $kdeheaderlistpath) or die "Couldn't open $kdeheaderlistpath: $!\n";
map { chomp ; $kdeincludes{$_} = 1 unless /^\s*#/ } <HEADERS>;
close HEADERS;

# Can we compile the OpenGl module ?
if("@KDE_HAVE_GL@" eq "yes")
{
    open(DEFS, $definespath);
    my @defs = <DEFS>;
    close DEFS;
    if(!grep(/QT_NO_OPENGL/, @defs))
    {
      $excludes{'qgl.h'} = undef;
      $excludes{'qglcolormap.h'} = undef;
    }
    else
    {
      print STDERR "Qt was not compiled with OpenGL support...\n Skipping QGL Classes.\n";
    }
}

# List Qt headers, and exclude the ones listed above
my @headers = ();

@qtinc= '@qt_includes@';

find(
    {   wanted => sub {
	    (-e || -l and !-d) and do {
	        $f = $_;
                if( !defined $excludes{$f} # Not excluded
                     && $includes{$f}        # Known header
                     && /\.h$/)     # Not a backup file etc. Only headers.
                {
                    my $header = $File::Find::name;
                    if ( $header !~ /src/ ) {
                        open(FILE, $header);
                        my @header_lines = <FILE>;
                        if (@header_lines == 1) {
                            $line = $header_lines[0];
                            if ($line =~ /^#include "(.*)"/) {
                                push ( @headers, $File::Find::dir . substr($1, 2) );
                            } else {
                                push ( @headers, $header );
                            }
                        } else {
                            push ( @headers, $header );
                        }
					}
                }
	    	undef $includes{$f}   
	     };
	},
	follow_fast => 1,
	follow_skip => 2,
	no_chdir => 0
    }, @qtinc
);

my @kdeheaders = ();
$kdeprefix = "@KDE_PREFIX@";
$kdeinc= '@kde_includes@';
$kdeinc =~ s/\${prefix}/$kdeprefix/; # Remove ${prefix} in src != build
-d $kdeinc or die "Couldn't process $kdeinc: $!\n";

find(
    {   wanted => sub {
	    (-e || -l and !-d) and do {
	        $f = substr($_, 1 + length $kdeinc);
                push ( @kdeheaders, $_ )
	    	  if( !defined $kdeexcludes{$f} # Not excluded
	    	     && $kdeincludes{$f}        # Known header
	    	     && /\.h$/);     # Not a backup file etc. Only headers.
	    	undef $kdeincludes{$f}   
	     };
	},
	follow_fast => 1,
	follow_skip => 2,
	no_chdir => 1
    }, $kdeinc
 );

# Launch kalyptus
chdir "../smoke/kde";
system "perl -I@kdebindings_SOURCE_DIR@/kalyptus @kdebindings_SOURCE_DIR@/kalyptus/kalyptus @ARGV --qt4 --globspace -fsmoke --name=qt $macros --no-cache --outputdir=$outdir @headers @kdeheaders";
my $exit = $? >> 8;
exit $exit if ($exit);
chdir "$kalyptusdir";

# Generate diff for smokedata.cpp
unless ( -e "$finaloutdir/smokedata.cpp" ) {
    open( TOUCH, ">$finaloutdir/smokedata.cpp");
    close TOUCH;
}
system "diff -u $finaloutdir/smokedata.cpp $outdir/smokedata.cpp > $outdir/smokedata.cpp.diff";

# Copy changed or new files to finaloutdir
opendir (OUT, $outdir) or die "Couldn't opendir $outdir";
foreach $filename (readdir(OUT)) {
    next if ( -d "$outdir/$filename" ); # only files, not dirs
    my $docopy = 1;
    if ( -f "$finaloutdir/$filename" ) {
        $docopy = compare("$outdir/$filename", "$finaloutdir/$filename"); # 1 if files are differents
    }
    if ($docopy) {
	#print STDERR "Updating $filename...\n";
	cp("$outdir/$filename", "$finaloutdir/$filename");
    }
}
closedir OUT;

# Check for deleted files and warn
my $deleted = 0;
opendir(FINALOUT, $finaloutdir) or die "Couldn't opendir $finaloutdir";
foreach $filename (readdir(FINALOUT)) {
    next if ( -d "$finaloutdir/$filename" ); # only files, not dirs
    if ( $filename =~ /.cpp$/ && ! ($filename =~ /_la_closure.cpp/) && ! -f "$outdir/$filename" ) {
      print STDERR "Removing obsolete file $filename\n";
      unlink "$finaloutdir/$filename";
      $deleted = 1;
    }
}
closedir FINALOUT;

# Delete outdir
system "rm -rf $outdir";

