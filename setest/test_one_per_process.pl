#!/usr/bin/perl

# Executes all the single tests from the exp file, 
# but each in an own process
# Reports all errors

use strict;
use warnings;

if ($#ARGV < 0) {
  die "Enter expectation file name";
  } 

my $exp = $ARGV[0];
my @tests = ();

open EXP, "<$exp" or die "Can't read file '$exp': $@\n";

my %current = ( TESTSUITE=>"", TESTCASE=>"", ITERATION=>"" );
my $level = "GENERAL";

for my $line (<EXP>) {
  chomp $line;
  $line =~ s/^\s*(.*?)\s*$/$1/;
  if ($line =~ /(TESTSUITE|TESTCASE|ITERATION)/) {
    $level = $line;
  }
  if ($line =~ /section\-id:\s*(\d+)/) {
    $current{$level} = $1;
    push @tests, "$current{TESTSUITE}.$current{TESTCASE}.$current{ITERATION}" 
      if $level eq "ITERATION";
  }
}

for my $test (@tests) {
  my $result = `./setest -s $test $exp`;
  if ($? >> 8) {
    print "$test:\n";    
    print $result;
  }
}
