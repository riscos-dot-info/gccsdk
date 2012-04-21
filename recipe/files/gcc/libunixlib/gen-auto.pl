#!/usr/bin/perl
# Create UnixLib Makefile.am files for its test suites and include/unixlib/stubs.h.
# Copyright (c) 2005-2010 UnixLib developers

use File::Basename;
$sourceunixlibtree = dirname($0);

if (! -e "$sourceunixlibtree/sys/_syslib.s") {
  die "$sourceunixlibtree looks invalid as UnixLib source directory\n";
}

# Generate Makefile.am files for test suites:
@testdirs = ();
# Collect all test subdirectories:
opendir (ROOT, "$sourceunixlibtree/test");
while ($dir = readdir (ROOT)) {
    # Do not look at .svn directories.
    next if ($dir eq '.' or $dir eq '..' or $dir eq ".svn");
    next if (! -d "$sourceunixlibtree/test/$dir");
    # When there is already a Makefile, don't generate another autotool's
    # generated one:
    next if (-f "$sourceunixlibtree/test/$dir/Makefile");

    push (@testdirs, $dir);
}
closedir (ROOT);

# Write the root test Makefile.am file:
open (MAKE, "> $sourceunixlibtree/test/Makefile.am") or die $^E;
print MAKE<<EOF;
# Testsuite automake-style infrastructure
# THIS FILE IS GENERATED BY 'gen-auto.pl' SO DO NOT EDIT

AUTOMAKE_OPTIONS = no-dependencies
SUBDIRS = \\
EOF
print MAKE "\t";
for (my $t = 0; $t <= $#testdirs; $t++) {
    printf MAKE "%s ", $testdirs[$t];
    }
print MAKE "\n";
close MAKE;

# Write per testsuite a Makefile.am:
foreach $dir (@testdirs) {
    open (MAKE, "> $sourceunixlibtree/test/$dir/Makefile.am") or die $^E;
    print MAKE "# Testsuite automake-style infrastructure\n";
    print MAKE "# THIS FILE IS GENERATED BY 'gen-auto.pl' SO DO NOT EDIT\n\n";
    print MAKE "AUTOMAKE_OPTIONS = no-dependencies\n";
    print MAKE "AM_CPPFLAGS = -isystem \$(top_srcdir)/include -I \$(top_srcdir)/test -D_GNU_SOURCE=1 -std=c99\n";
    print MAKE "LDADD = -L\$(top_builddir)\n\n";

    opendir (TESTS, "$sourceunixlibtree/test/$dir");
    my @tests = ();
    while ($entry = readdir (TESTS)) {
	next if ($entry eq '.' or $entry eq '..' or $entry eq '.svn');
	if ($entry =~ /\.c$/) {
	    push (@tests, $entry);
	}
    }
    close TESTS;

    print MAKE "check_PROGRAMS = ";
    my $x = 0;
    for (my $y = 0; $y <= $#tests; $y++) {
	my $test = $tests[$y];
	$x ++;
	# Strip suffix as automake wants the program name.
	$test =~ s/\.c//;
	printf MAKE "%s ", $test;
	if ($x == 5 and $y != $#tests) {
	    printf MAKE "\\\n\t";
	    $x = 0;
	}
    }
    print MAKE "\n\nTESTS = \$(check_PROGRAMS)\n";
    close MAKE;
}

# Generate the stubs header file:
my @dirs;
my @srcs;
my @objs;

# Find all source files
sub find_files {
  my $dir = $_[0];
  my @entries = glob("$dir/*");
  my $entry;
  foreach $entry (@entries) {
    # Make sure names are in unix format
    $entry =~ s/\/([cosh])\/([^\/\.]*)$/\/$2.$1/;
    # Ignore special directories
    next if $entry =~ m/^(.svn)/;
    next if $entry =~ m/^(test)|^(include)|^(module)|^(contrib)|^(autom4te.cache)|^(incl-local)/;
    if (-d "$entry") {
      push @dirs, $entry if !($entry =~ m/^(.*\/)?[cosh]$/);
      find_files("$entry");
    } else {
      if ($entry =~ m/\.[cs]$/) {
        my $obj = $entry;
        $obj =~ s/[cs]$/o/;
        push @objs, $obj;
        push @srcs, $entry;
      }
    }
  }
}

find_files($sourceunixlibtree);

# Generate include/unixlib/stubs.h
open (STUBS, ">$sourceunixlibtree/include/unixlib/stubs.h") or die $^E;

print STUBS "/* This file is automatically generated by 'gen-auto.pl'.\n"
           ."It defines a symbol `__stub_FUNCTION' for each function\n"
           ."in the C library which is a stub, meaning it will fail\n"
           ."every time called, usually setting errno to ENOSYS.  */\n\n";

foreach $src (@srcs) {
  # Only check c files
  next if !($src =~ m/\.c$/);
  open (SRC, "<".$src) or die $^E;
  my $funcname = "none";
  my $return = 1;
  while (<SRC>) {
    # Get function name
    if (/^(\w*) \([^\)]*\)\W*$/) {
      $funcname = $1;
      $return = 0;
    }
    # Find the return statement
    if (/^\W*return.*ENOSYS/) {
      $return = -1 if ($return == 0);
    } elsif (/^\W*return/) {
      # Only a stub iff there is one return that always returns ENOSYS
      $return = 1;
    }
    # Find function end
    if (/^\}\W*$/) {
      print STUBS "#define __stub_$funcname\n" if ($return == -1);
    }
  }
  close SRC;
}
close STUBS;

# Generate misc/abi.c

$abi_version = $ENV{'GCCSDK_RISCOS_ABI_VERSION'};

open (ABI, ">$sourceunixlibtree/misc/abi.c") or die $^E;

print ABI "/* This file is automatically generated by 'gen-auto.pl'.\n"
	 ."   Because UnixLib is linked with the option -nostdlib, the\n"
	 ."   '.riscos.abi.version' section defined in crtbegin.o is\n"
	 ."   missing, so it is defined here.  */\n\n";
print ABI "static const char riscos_abi_version[]\n"
	 ."  __attribute__((used, section(\".riscos.abi.version\"), aligned(4))) =\n"
	 ."\"$abi_version\";\n";
close ABI;

exit 0;
