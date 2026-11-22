#!/usr/bin/perl
# setest_diff_report.pl - group the failures of a setest run by cause and test case
#
# Written 2026-11-22 to list the differences between two releases, e.g. SE3
# against the SE2.10.04 expected values (both t.exp files from the same t.fix):
#   cd /home/swe3home/setest && ./setest /home/swehome/setest/t > se2_se3.out
#   perl setest_diff_report.pl /home/swehome/setest/t.exp se2_se3.out > se2_se3_diffs.txt
#
# The first file is the t.exp the run was compared with; it gives the input
# values (ephemeris flag, date, body, ayanamsha) of every failing iteration.
# The second file is the stdout of the setest run.
use strict;
use warnings;

# ayanamshas whose t0 is before JD 1721057.5 (year 0), sweph.h ayanamsa[]
my %AYA_T0_BEFORE_0 = map { $_ => 1 } (9 .. 15);
# order and description of the cause classes
my @CLASSES = (
  [ 'moseph', 'SEFLG_MOSEPH (4): Moshier in SE2, Swiss Ephemeris in SE3 (sweph.c:88 SEFLG_EPHMASK)' ],
  [ 'aya_t0', 'ayanamshas 9-15 (and 4.x with them), t0 before year 0' ],
  [ 'bc3000', 'dates before -1500 (jd < 1173183)' ],
  [ 'ce3000', 'dates after 2900 (jd > 2780000)' ],
  [ 'other',  'none of the above' ],
);

# read the failures of a setest run: id -> { q => { quantity -> max |diff| or 'value' } }
sub read_failures {
  my ($fname) = @_;
  my $fail = {};
  my ($id, $q);
  open my $fh, '<', $fname or die "$fname: $!\n";
  while (my $line = <$fh>) {
    if ($line =~ /^# setest -s (\d+\.\d+\.\d+) /) {
      $id = $1;
      $fail->{$id} //= { q => {} };
      next;
    }
    next unless defined $id;
    if ($line =~ /^#\s+(\S+) unexpected value/) {
      $q = $1;
      $q =~ s/\[\d+\]/[]/;
      $fail->{$id}{n}++;
      $fail->{$id}{q}{$q} //= 'value';
      next;
    }
    if ($line =~ /^#\s+diff:\s+(\S+)/) {
      my $d = abs($1);
      my $old = $fail->{$id}{q}{$q};
      $fail->{$id}{q}{$q} = $d if $old eq 'value' || $d > $old;
    }
  }
  close $fh;
  return $fail;
}

# read the input values of the given iterations from a t.exp file
sub read_inputs {
  my ($fname, $fail) = @_;
  my $inp = {};
  my $id;
  open my $fh, '<', $fname or die "$fname: $!\n";
  while (my $line = <$fh>) {
    if ($line =~ /^\s*(ITERATION|TESTCASE|TESTSUITE)\s*$/) {
      $id = undef;
      next;
    }
    if ($line =~ /section-id:\s*\d+\s+#(\d+\.\d+\.\d+)/) {
      $id = $fail->{$1} ? $1 : undef;
      $inp->{$id} = {} if defined $id;
      next;
    }
    next unless defined $id;
    if ($line =~ /^\s*(iephe|iflag|jd|jd_ut|ipl|sid_mode|ihsy|star)\s*:\s*([^#\n]*?)\s*(#\s*(.*?))?\s*$/) {
      $inp->{$id}{$1} = $2;
      $inp->{$id}{"$1#"} = $4 // '';
    }
  }
  close $fh;
  return $inp;
}

# cause class of one failing iteration
sub classify {
  my ($id, $in) = @_;
  my ($suite) = split /\./, $id;
  my $eflags = ($in->{'iephe#'} // '') . ' ' . ($in->{'iflag#'} // '');
  return 'moseph' if $eflags =~ /SEFLG_MOSEPH/;
  return 'aya_t0' if $suite == 4 && defined $in->{sid_mode} && $AYA_T0_BEFORE_0{ $in->{sid_mode} & 255 };
  my $jd = $in->{jd} // $in->{jd_ut};
  return 'bc3000' if defined $jd && $jd < 1173183;
  return 'ce3000' if defined $jd && $jd > 2780000;
  return 'other';
}

# "1-4,7,9-10" from a list of numbers
sub ranges {
  my (@n) = sort { $a <=> $b } @_;
  my @r;
  for my $i (@n) {
    if (@r && $r[-1][1] == $i - 1) { $r[-1][1] = $i } else { push @r, [ $i, $i ] }
  }
  return join ',', map { $_->[0] == $_->[1] ? $_->[0] : "$_->[0]-$_->[1]" } @r;
}

# compare "4.10" and "4.2" numerically
sub by_case {
  my @x = split /\./, $a;
  my @y = split /\./, $b;
  return $x[0] <=> $y[0] || $x[1] <=> $y[1];
}

# print the report
sub report {
  my ($pp) = @_;
  my ($fail, $inp) = ($pp->{fail}, $pp->{inp});
  my %cls;
  for my $id (keys %$fail) {
    my ($s, $c, $i) = split /\./, $id;
    push @{ $cls{ classify($id, $inp->{$id} // {}) }{"$s.$c"} }, $i;
  }
  my $nfail = 0;
  $nfail += $fail->{$_}{n} for keys %$fail;
  printf "%d failures in %d iterations\n", $nfail, scalar keys %$fail;
  for my $cl (@CLASSES) {
    my ($key, $descr) = @$cl;
    my $cases = $cls{$key} or next;
    my $n = 0;
    $n += @{ $cases->{$_} } for keys %$cases;
    printf "\n== %s: %d iterations\n", $descr, $n;
    for my $case (sort by_case keys %$cases) {
      my @its = @{ $cases->{$case} };
      my (%q, %inputs);
      for my $i (@its) {
        my $id = "$case.$i";
        for my $qn (keys %{ $fail->{$id}{q} }) {
          my $d = $fail->{$id}{q}{$qn};
          $q{$qn} = $d if !defined $q{$qn} || ($d ne 'value' && ($q{$qn} eq 'value' || $d > $q{$qn}));
        }
        my $in = $inp->{$id} // {};
        $inputs{ipl}{ $in->{'ipl#'} || $in->{ipl} }++ if defined $in->{ipl};
        $inputs{sid}{ $in->{sid_mode} }++ if defined $in->{sid_mode};
        $inputs{ihsy}{ $in->{'ihsy#'} }++ if defined $in->{ihsy};
        my $jd = $in->{jd} // $in->{jd_ut};
        if (defined $jd) {
          my $date = $in->{'jd#'} || $in->{'jd_ut#'} || $jd;
          $date =~ s/\s.*//;
          $inputs{date}{$date}++;
        }
      }
      my $qs = join ' ', map { $q{$_} eq 'value' ? "$_ (value)" : sprintf('%s %.2e', $_, $q{$_}) } sort keys %q;
      printf "%-5s %3d its %s\n", $case, scalar @its, ranges(@its);
      printf "      max|diff| %s\n", $qs;
      for my $k (qw(date ipl sid ihsy)) {
        next unless $inputs{$k};
        my @v = sort keys %{ $inputs{$k} };
        @v = sort { $a <=> $b } @v if $k eq 'sid';
        printf "      %-4s %s\n", $k, join(', ', @v);
      }
    }
  }
}

my ($exp, $out) = @ARGV;
die "usage: $0 t.exp setest-output\n" unless defined $out;
my $fail = read_failures($out);
my $inp = read_inputs($exp, $fail);
my $pp = {
  fail => $fail,
  inp  => $inp,
};
report($pp);
