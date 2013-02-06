#***************************************************************************
#            kalyptusCxxToEMA.pm -  Generates class info for ECMA bindings in KDE
#                             -------------------
#    begin                : Fri Jan 25 12:00:00 2000
#    copyright            : (C) 2002 Lost Highway Ltd. All Rights Reserved.
#    email                : david@mandrakesoft.com
#    author               : David Faure.
#***************************************************************************/

#/***************************************************************************
# *                                                                         *
# *   This program is free software; you can redistribute it and/or modify  *
# *   it under the terms of the GNU General Public License as published by  *
# *   the Free Software Foundation; either version 2 of the License, or     *
# *   (at your option) any later version.                                   *
# *                                                                         *
#***************************************************************************/

package kalyptusCxxToECMA;

use File::Path;
use File::Basename;

use Carp;
use Ast;
use kdocAstUtil;
use kdocUtil;
use Iter;
use kalyptusDataDict;

use strict;
no strict "subs";

use vars qw/
	$libname $rootnode $outputdir $opt $debug
	%skippedClasses %hasHashTable %hasFunctions %hasBridge %hasGet %hasPut/;

sub writeDoc
{
	( $libname, $rootnode, $outputdir, $opt ) = @_;

	print STDERR "Starting writeDoc for $libname...\n";

	$debug = $main::debuggen;

	mkpath( $outputdir ) unless -f $outputdir;

	# Preparse everything, to prepare some additional data in the classes and methods
	Iter::LocalCompounds( $rootnode, sub { preParseClass( shift ); } );

	print STDERR "Writing generateddata.cpp...\n";

	writeInheritanceFile($rootnode);

	print STDERR "Done.\n";
}

=head2 preParseClass
	Called for each class
=cut
sub preParseClass
{
	my( $classNode ) = @_;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );

	if ( $className =~ /Proto$/ ) {
	    my $c = $className;
	    $c =~ s/Proto$//;
	    #print STDERR "$c -> $className\n";
	    $hasFunctions{$c} = $className; # Associate class -> proto
	    #print STDERR "Found proto $className -> skipping\n";
	    $skippedClasses{$className} = 1; # Skip proto
	    return;
	}

	if( $classNode->{Access} eq "private" ||
	    $classNode->{Access} eq "protected" || # e.g. QPixmap::QPixmapData
	    exists $classNode->{Tmpl} ||
	    $className eq 'KJS' || $className eq 'KSVG' || # namespaces
	    $className =~ /^KSVG::KSVG/ || $className eq 'KSVG::CachedGlyph' || # Not DOM classes
	    $className eq 'KSVG::ImageStreamMap' ||
	    $className eq 'KSVG::SVGBBoxTarget' ||
	    $className eq 'KSVG::SVGLoader' ||
	    $className eq 'KSVG::SVGElementImpl::MouseEvent' ||
		$className eq 'KSVG::SVGRegisteredEventListener' ||
	    $classNode->{NodeType} eq 'union' # Skip unions for now, e.g. QPDevCmdParam
	  ) {
	    print STDERR "Skipping $className "; #if ($debug);

	    #print STDERR "(nothing in it)\n" if ( $#{$classNode->{Kids}} < 0 );
	    if ( exists $classNode->{Tmpl} ) {
		print STDERR "(template)\n";
	    } elsif ( $classNode->{Access} eq "private" or $classNode->{Access} eq "protected" ) {
	        print STDERR "(not public)\n";
	    } elsif ( $classNode->{NodeType} eq 'union') {
		print STDERR "(union)\n";
	    } elsif ( $className =~ /^KSVG::KSVG/ || $className eq 'KSVG::CachedGlyph' ) {
		print STDERR "(not a DOM class)\n";
	    } else {
		print STDERR "\n";
	    }
	    $skippedClasses{$className} = 1;
	    #delete $classNode->{Compound}; # Cheat, to get it excluded from Iter::LocalCompounds
            # Can't do that, it's recursive (KSVG::* disappears)
	    return;
	}

    # Iterate over methods
    Iter::MembersByType ( $classNode, undef,
			  sub {	my ($classNode, $methodNode ) = @_;

        if ( $methodNode->{NodeType} eq 'method' ) {
	    if ( $methodNode->{astNodeName} eq 'get' ) {
		$hasGet{$className} = '1';
	    } elsif ( $methodNode->{astNodeName} eq 'getforward' ) {
		$hasGet{$className} = '2';
	    } elsif ( $methodNode->{astNodeName} eq 'put' ) {
		$hasPut{$className} = '1';
	    } elsif ( $methodNode->{astNodeName} eq 'putforward' ) {
		$hasPut{$className} = '2';
	    } elsif ( $methodNode->{astNodeName} eq 'getValueProperty' ) {
		$hasHashTable{$className} = '1';
	    } elsif ( $methodNode->{astNodeName} eq 'bridge' ) {
		$hasBridge{$className} = '1';
	    }

	}
			    } );
}

# List of all super-classes for a given class
sub superclass_list($)
{
    my $classNode = shift;
    my @super;
    Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			push @super, @_[0];
			push @super, superclass_list( @_[0] );
		     }, undef );
    return @super;
}

# Adds the header for node $1 to be included in $2 if not already there
# Prints out debug stuff if $3
sub addIncludeForClass($$$)
{
    my ( $node, $addInclude, $debugMe ) = @_;
    my $sourcename = $node->{Source}->{astNodeName};
    $sourcename =~ s!.*/(.*)!$1!m;
    unless ( defined $addInclude->{$sourcename} ) {
	print "  Including $sourcename\n" if ($debugMe);
	$addInclude->{$sourcename} = 1;
    }
    else { print "  $sourcename already included.\n" if ($debugMe); }
}

=head2
	Write out the smokedata.cpp file containing all the arrays.
=cut

sub writeInheritanceFile($) {
    my $rootnode = shift;

    # Make list of classes
    my %allIncludes; # list of all header files for all classes
    my @classlist;
    push @classlist, ""; # Prepend empty item for "no class"
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = $_[0];
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
        return if ( defined $skippedClasses{$className} );
	push @classlist, $className;
	$classNode->{ClassIndex} = $#classlist;
	addIncludeForClass( $classNode, \%allIncludes, undef );
    } );

    my %classidx = do { my $i = 0; map { $_ => $i++ } @classlist };
    #foreach my $debugci (keys %classidx) {
    # print STDERR "$debugci: $classidx{$debugci}\n";
    #}

    my $file = "$outputdir/generateddata.cpp";
    open OUT, ">$file" or die "Couldn't create $file\n";
    print OUT "#include <ksvg_lookup.h>\n";
    print OUT "#include <ksvg_ecma.h>\n";

    foreach my $incl (keys %allIncludes) {
	die if $incl eq '';
	print OUT "#include <$incl>\n";
    }	

    print OUT "\n";

    # Prepare descendants information for each class
    my %descendants; # classname -> list of descendant nodes
    #my $SVGElementImplNode;
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
	# Get _all_ superclasses (up any number of levels)
	# and store that $classNode is a descendant of $s
	my @super = superclass_list($classNode);
	for my $s (@super) {
	    my $superClassName = join( "::", kdocAstUtil::heritage($s) );
	    Ast::AddPropList( \%descendants, $superClassName, $classNode );
	}
	# Found SVGElementImpl itself
	if ( $className eq 'KSVG::SVGElementImpl' ) {
	    $classNode->{IsSVGElement} = '1';
	    #$SVGElementImplNode = $classNode;
	}
    } );

    # Mark all SVGElementImpl descendants as svg elements
    if ( defined $descendants{'KSVG::SVGElementImpl'} ) {
	my @desc = @{$descendants{'KSVG::SVGElementImpl'}};
	for my $d (@desc) {
	    $d->{IsSVGElement} = '1' ;
	    print STDERR $d->{astNodeName}. " is an SVGElement\n" if($debug);
	}
    }

    # Propagate $hasPut and $hasGet 
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
	if ( defined $descendants{$className} ) {
	    my @desc = @{$descendants{$className}};
	    for my $d (@desc) {
		my $c = join( "::", kdocAstUtil::heritage($d) );
		$hasPut{$c} = '2' if (!$hasPut{$c} && $hasPut{$className});
		$hasGet{$c} = '2' if (!$hasGet{$c} && $hasGet{$className});
	    }
	}

        # This code prints out the base classes - useful for KSVG_BASECLASS_GET
	if ( 0 && defined $descendants{$className} ) {
	    my $baseClass = 1;
	    Iter::Ancestors( $classNode, $rootnode, sub { # called if no ancestors
			     }, undef, sub { # called for each ancestor
			     my $superClassName = join( "::", kdocAstUtil::heritage($_[0]) );
			     $baseClass = 0 if ( $superClassName ne '' ); # happens with unknown parents;
			 } );
           print STDERR "$className is a base class.\n" if ($baseClass);
	}

    } );

	# Write namespaces
	print OUT "using namespace KSVG;\n";
	print OUT "using namespace KJS;\n\n";
	
    # Write classInfos
    print OUT "// For all classes with generated data: the ClassInfo\n";
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );

	# We use namespace declartions!
	my $printName = $className;
	$printName =~ s/KSVG:://;

	# Write tagNames
	if ($hasBridge{$className}) {
		my $tagName = $printName;
		$tagName =~ s/SVG//;
		$tagName =~ s/ElementImpl//;

		$tagName = lcfirst($tagName);

		# Special cases, otherwhise they'd be "tRef" / "tSpan" / "sVG"
		if($printName eq "SVGTRefElementImpl" or
		   $printName eq "SVGTSpanElementImpl" or
		   $printName eq "SVGSVGElementImpl") {
			$tagName =~ tr/A-Z/a-z/;
		}

		while($tagName =~ /[A-Z]/g) {
				# Special case: color-profile instead of ie. animateColor/animateMotion
				if($printName eq "SVGColorProfileElementImpl") {
					$tagName = substr($tagName, 0, pos($tagName) - 1) . "-" . lc($&) . substr($tagName, pos($tagName));
				}
		}

		# Special cases: gradient & poly aren't element! 
		if($tagName ne "" and $tagName ne "gradient" and $tagName ne "poly") {
			print OUT "const DOM::DOMString ${printName}::s_tagName = \"$tagName\";\n";
		}
	}

	# Skip classes without KSVG_GENERATEDDATA
	if (!$hasGet{$className} && !$skippedClasses{$className}) {
	    $skippedClasses{$className} = '1' ;
	    print STDERR "Skipping $className, no get()\n";
	}

        return if ( defined $skippedClasses{$className} );

	my $ok = $hasHashTable{$className};
	print STDERR "$className has get() but no hashtable - TODO\n" if (!$ok && $hasGet{$className} eq '1');

	print OUT "const ClassInfo ${printName}::s_classInfo = {\"$className\",0,";
	if ($ok) {
	    print OUT "\&${printName}::s_hashTable";
	} else {
	    print OUT "0";
	}
        print OUT ",0};\n";
	#die "problem with $className" unless defined $classinherit{$className};
	#print OUT "const short int ${className}::s_inheritanceIndex = $classinherit{$className};\n";
    } );

    # Generated methods
    print OUT "\n";
    Iter::LocalCompounds( $rootnode, sub {
	my $classNode = shift;
	my $className = join( "::", kdocAstUtil::heritage($classNode) );
        return if ( defined $skippedClasses{$className} );

	# We use namespace declartions!
	my $printName = $className;
	$printName =~ s/KSVG:://;

	my $paramsUsed = 0;
	
	print OUT "bool ${printName}::hasProperty(ExecState *p1,const Identifier &p2) const\n";
	print OUT "{\n";

	if ($hasHashTable{$className}) {
	    print OUT "    const HashEntry *e = Lookup::findEntry(\&${printName}::s_hashTable,p2);\n";
	    print OUT "    if(e) return true;\n";
	    $paramsUsed=1;
	}
	# Now look in prototype, if it exists
	if ( defined $hasFunctions{$className} ) {

		# We use namespace declartions!
		my $output = $hasFunctions{$className};
		$output =~ s/KSVG:://;

	    print OUT "    Object proto = " . $output . "::self(p1);\n";
	    print OUT "    if(proto.hasProperty(p1,p2)) return true;\n";
	}
	# For each direct ancestor...
	Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			     my $superClassName = join( "::", kdocAstUtil::heritage($_[0]) );
				
				 # We use namespace declartions!
				 my $printSuperClassName = $superClassName;
				 $printSuperClassName =~ s/KSVG:://;

			     if ( $superClassName ne '' ) { # happens with unknown parents
			       return if ( defined $skippedClasses{$superClassName} );
			        print OUT "    if(${printSuperClassName}::hasProperty(p1,p2)) return true;\n";
				$paramsUsed=2;
			     }
		    }, undef );
	if ($paramsUsed == 1 && !defined $hasFunctions{$className}){
	    print OUT "    Q_UNUSED(p1);\n";
	}
	if ($paramsUsed == 0){
	    print OUT "    Q_UNUSED(p1); Q_UNUSED(p2);\n";
	}
        print OUT "    return false;\n";
        print OUT "}\n\n";

	my @ancestorsWithGet;
	Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			     my $superClassName = join( "::", kdocAstUtil::heritage($_[0]) );
			     if ( $superClassName ne '' # happens with unknown parents
				  && ! defined $skippedClasses{$superClassName} ) {
				 if ( $hasGet{$superClassName} ) {
				     push @ancestorsWithGet, $superClassName;
				 }
			     }
		    }, undef );

	if ($hasHashTable{$className}) {
	    die unless $hasGet{$className};
	    if ( $hasGet{$className} eq '1' ) {
		print OUT "Value ${printName}::get(GET_METHOD_ARGS) const\n";
		print OUT "{\n";
		if ( defined $hasFunctions{$className} ) {

			# We use namespace declartions!
			my $output = $hasFunctions{$className};
			$output =~ s/KSVG:://;

		    print OUT "    return lookupGet<${output}Func,${printName}>(p1,p2,&s_hashTable,this,p3);\n";
		} else {
		    print OUT "    return lookupGetValue<${printName}>(p1,p2,&s_hashTable,this,p3);\n";
		}
		print OUT "}\n\n";

		if ( defined $hasFunctions{$className} ) {

			# We use namespace declartions!
			my $output = $hasFunctions{$className};
			$output =~ s/KSVG:://;

		    my $methodName = "${output}Func::cast";
		    my $const = 'const';
		    # Special case - we also need that code in toNode()
		    if ($methodName eq 'SVGDOMNodeBridgeProtoFunc::cast') {
			print OUT "${printName} *$methodName(const ObjectImp *p1) const\n";
			$methodName = 'KSVG::toNodeBridge';
			print OUT "{\n";
			print OUT "    return $methodName(p1);\n";
			print OUT "}\n\n";
			$const = '';
		    }

		    # Type resolver for the Func class
		    print OUT "${printName} *$methodName(const ObjectImp *p1) $const\n";
		    print OUT "{\n";
		    my @toTry;
		    push @toTry, $classNode;
		    if ( defined $descendants{$className} ) {
			push @toTry, @{$descendants{$className}};
		    }
		    foreach my $d (@toTry) {
			my $c = join( "::", kdocAstUtil::heritage($d) );

			# We use namespace declartions!
			my $d = $c;
			$d =~ s/KSVG:://;
			
			print OUT "    { const KSVGBridge<$d> *test = dynamic_cast<const KSVGBridge<$d> * >(p1);\n";
			print OUT "    if(test) return test->impl(); }\n";
		    }
		    print OUT "    return 0;\n";
		    print OUT "}\n\n";
		}
	    }
	}

	my $methodName = $hasGet{$className} eq '1' ? 'getInParents' : 'get';
        print OUT "Value ${printName}::$methodName(GET_METHOD_ARGS) const\n";
        print OUT "{\n";
	my $paramsUsed = 0;
	# Now look in prototype, if it exists
	if ( defined $hasFunctions{$className} ) {
	    # Prototype exists (because the class has functions)

		# We use namespace declartions!
		my $output = $hasFunctions{$className};
		$output =~ s/KSVG:://;

	    print OUT "    Object proto = " . $output . "::self(p1);\n";
	    print OUT "    if(proto.hasProperty(p1,p2)) return proto.get(p1,p2);\n"; ## TODO get() directly
	    $paramsUsed = 1;
	}
	foreach my $anc (@ancestorsWithGet) {
		# We use namespace declartions!
		my $printAnc = $anc;
		$printAnc =~ s/KSVG:://;

		print OUT "    if(${printAnc}::hasProperty(p1,p2)) return ${printAnc}::get(p1,p2,p3);\n"; ## TODO get() directly
	    $paramsUsed = 2;
	}

	if ($paramsUsed == 0 ){
	    print OUT "    Q_UNUSED(p1); Q_UNUSED(p2); Q_UNUSED(p3);\n";
	} elsif ( $paramsUsed == 1 ) {
	    print OUT "    Q_UNUSED(p3);\n";
	}
        print OUT "    return Undefined();\n";
        print OUT "}\n\n";

	if ( $hasPut{$className} )
	{
	    if ( $hasPut{$className} eq '1' ) {
		if ($hasHashTable{$className}) {
		    print OUT "bool ${printName}::put(PUT_METHOD_ARGS)\n";
		    print OUT "{\n";
		    print OUT "    return lookupPut<${printName}>(p1,p2,p3,p4,&s_hashTable,this);\n";
		    print OUT "}\n\n";
		}
		print OUT "bool ${printName}::putInParents(PUT_METHOD_ARGS)\n";
	    } else { # forward put
		print OUT "bool ${printName}::put(PUT_METHOD_ARGS)\n";
	    }
	    print OUT "{\n";
	    my $paramsUsed = 0;
	    Iter::Ancestors( $classNode, $rootnode, undef, undef, sub {
			     my $superClassName = join( "::", kdocAstUtil::heritage($_[0]) );

				 # We use namespace declartions!
				 my $printSuperClassName = $superClassName;
				 $printSuperClassName =~ s/KSVG:://;

			     if ( $superClassName ne '' ) { # happens with unknown parents
				 if ( $hasPut{$superClassName} ) {
				     print OUT "    if(${printSuperClassName}::hasProperty(p1,p2)) {\n";
				     print OUT "        ${printSuperClassName}::put(p1,p2,p3,p4);\n";
				     print OUT "        return true;\n";
				     print OUT "    }\n";
				     $paramsUsed=1;
				 }
			     }
		    }, undef );
	    if (!$paramsUsed){
		print OUT "    Q_UNUSED(p1); Q_UNUSED(p2); Q_UNUSED(p3); Q_UNUSED(p4);\n";
	    }
	    print OUT "    return false;\n";
	    print OUT "}\n\n";
	}

	# Write prototype method
	print OUT "Object ${printName}::prototype(ExecState *p1) const\n";
	print OUT "{\n";
	if ( defined $hasFunctions{$className} ) {

		# We use namespace declartions!
		my $output = $hasFunctions{$className};
		$output =~ s/KSVG:://;

	    # Prototype exists (because the class has functions)
	    print OUT "    if(p1) return " . $output . "::self(p1);\n";
	} else {
	    # Standard Object prototype
	    print OUT "    if(p1) return p1->interpreter()->builtinObjectPrototype();\n";
	}
	print OUT "    return Object::dynamicCast(Null());\n"; # hmm

        print OUT "}\n\n";

	# Process classes only with KSVG_BRIDGE
	if ($hasBridge{$className}) {

		#print STDERR "Writing bridge() for $className...\n";
		
		# Write bridge method
		print OUT "ObjectImp *${printName}::bridge(ExecState *p1) const\n";
		print OUT "{\n";
		
		if ($hasPut{$className})
		{
			print OUT "    return new KSVGRWBridge<${printName}>(p1,const_cast<${printName} *>(this));\n";
		}
		else
		{
			print OUT "    return new KSVGBridge<${printName}>(p1,const_cast<${printName} *>(this));\n";
		}
		
		print OUT "}\n\n";
	}

	if ($hasGet{$className}) {
		# Write cache method
		print OUT "Value ${printName}::cache(ExecState *p1) const\n";
		print OUT "{\n";
		
		if ($hasPut{$className})
		{
			print OUT "    return KJS::Value(cacheDOMObject<${printName},KSVGRWBridge<${printName}> >(p1,const_cast<${printName} *>(this)));\n";
		}
		else
		{
			print OUT "    return KJS::Value(cacheDOMObject<${printName},KSVGBridge<${printName}> >(p1,const_cast<${printName} *>(this)));\n";
		}
		
		print OUT "}\n\n";
	}
	
	} );

}

1;
