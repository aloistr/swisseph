#!/usr/bin/perl

use strict;
use warnings;
use Getopt::Long;

my $verbose = 0;
open my $null, ">/dev/null";
my $exit_code = 0;
my @results = ();

GetOptions ('verbose|v' => \$verbose );

do_tests();

foreach (@results) { 
  print "$_\n";
  }

exit $exit_code;

sub do_tests {
  my $test = "test_dummy.pl";
  local *STDOUT = $null unless $verbose;
  local *STDERR = $null unless $verbose;
  my $status = do_single_test( $test );
  $exit_code = 1 if ($status eq "-");
  push @results, "$status $test";
}

sub do_single_test {
  my $test = shift;
  my $rc = do $test;
  my $status = ($rc ? "-" : "+" );
  return $status;
}

