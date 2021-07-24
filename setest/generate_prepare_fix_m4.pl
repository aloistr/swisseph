#!/usr/local/bin/perl

use strict;
use warnings;

if ($#ARGV > 1) {
    print "\nUsage: ./generate_prepare_fix_m4.pl [full file name of swephexp.h]\n";
    exit;
}

my $swephexp = ($#ARGV == 0 ? $ARGV[0] : "../sweph/swephexp.h");

my $pattern = pattern_macronames();

open SE, "<$swephexp" or die "Can't open $swephexp: $!\n";
open M4, ">prepare_fix.m4" or die "Can't open prepare_fix.m4: $!\n";

print_header();
foreach my $line (<SE>) {
  my ($name,$value) = ($line =~ qr(^#define\s+(\w+)\s+(\S.*?)\s*((//|/\*).*)?$) );
  if (defined $name and $name =~ $pattern) {
    print M4 "define(`$name',`eval($value)')\n";
    } 
}
print_footer();
close SE;
close M4;

sub print_header {
print M4 <<"HEADER";
divert(-1)
# Macro definitions for preprocessing fixture files
HEADER
}

sub print_footer {
print M4 <<"FOOTER";
divert(0)dnl
FOOTER
}

sub pattern_macronames {
  my $re = qr/^(
    SEFLG_           |
    SE_

#    SE_AST_OFFSET$   |
#    SE_SUN$          |
#    SE_MOON$         |
#    SE_SUN$          |       
#    SE_MOON$         |       
#    SE_MERCURY$      |      
#    SE_VENUS$        |   
#    SE_MARS$         |      
#    SE_JUPITER$      |       
#    SE_SATURN$       |       
#    SE_URANUS$       |       
#    SE_NEPTUNE$      |       
#    SE_PLUTO$        |       
#    SE_MEAN_NODE$    |      
#    SE_TRUE_NODE$    | 
#    SE_MEAN_APOG$    |      
#    SE_OSCU_APOG$    |    
#    SE_EARTH$        |      
#    SE_CHIRON$       |      
#    SE_PHOLUS$       |      
#    SE_CERES$        |      
#    SE_PALLAS$       |      
#    SE_JUNO$         |      
#    SE_VESTA$        |      
#    SE_INTP_APOG$    |     
#    SE_INTP_PERG$        

    )/x;
  return $re;
}





