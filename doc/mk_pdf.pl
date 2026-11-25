#!/usr/bin/perl
# mk_pdf.pl: make a PDF or web page from swisseph.md, swephprg.md (SE2), swephprg3.md or the SE3 chapter directory
# (pandoc -> HTML, own table of contents with page numbers, weasyprint -> PDF)
# Every page has a footer with an icon that links to the table of contents,
# like the SE2 PDFs made by MS Word.
# The manual table of contents of the .md (from the Word conversion, block
# from the line "Contents" up to the first chapter heading) is replaced.
# Images are taken from the folder media/ next to the .md; if there is none, they are
# extracted from the .docx of the same name into <work>/media:
#   pandoc <name>.docx --extract-media=<work> -t markdown -o /dev/null
# usage: mk_pdf.pl <name>.md|<chapter dir> [-htm] [-w<workdir>] [-o<out>] [-k]   (-k keeps the html)
# -htm: web page <name>.htm instead of the PDF, images in <name>.fld/ next to it,
#       icon "up to contents" at every chapter (h1) and section (h2) heading
# 2026-11-24
use strict;
use warnings;
use File::Basename;
use File::Temp qw(tempdir);
use Cwd qw(abs_path);
my ($md, $work, $out, $keep, $htm);
for my $a (@ARGV) {
  if ($a eq '-htm') {
    $htm = 1;
  } elsif ($a =~ /^-w(.+)/) {
    $work = $1;
  } elsif ($a =~ /^-o(.+)/) {
    $out = $1;
  } elsif ($a eq '-k') {
    $keep = 1;
  } else {
    $md = $a;
  }
}
die "usage: mk_pdf.pl <name>.md [-htm] [-w<workdir>] [-o<out>] [-k]\n" if !defined $md;
$md = abs_path($md);
# a directory of chapter files <dir>/NN*.md is joined in byte order of the
# file names; name and .docx (images) as for <dir>.md, output into <dir>/
my @chapfiles;
if (-d $md) {
  opendir(my $dh, $md) or die "$md: $!";
  @chapfiles = map { "$md/$_" } sort grep { /^\d.*\.md$/ } readdir($dh);
  closedir($dh);
  die "no chapter files NN*.md in $md\n" if !@chapfiles;
}
my $name = basename($md, '.md');
my $docx = dirname($md) . "/$name.docx";
my $odir = @chapfiles ? $md : dirname($md);
$out = "$odir/$name." . ($htm ? 'htm' : 'pdf') if !defined $out;
$work = tempdir(CLEANUP => !$keep) if !defined $work;
mkdir $work if !-d $work;
# images: from a folder media/ next to the .md, else extracted from the .docx
my $mdir = (@chapfiles ? $md : dirname($md)) . "/media";
if (!-d "$work/media" && -d $mdir) {
  symlink($mdir, "$work/media") or die "symlink $work/media: $!\n";
}
if (!-d "$work/media" && -f $docx) {
  system('pandoc', $docx, "--extract-media=$work", '-t', 'markdown', '-o', '/dev/null') == 0
    or die "pandoc --extract-media failed\n";
}
# markdown without the manual table of contents
my (@l, $fh);
for my $f (@chapfiles ? @chapfiles : ($md)) {
  open($fh, '<', $f) or die "$f: $!";
  push @l, <$fh>, "\n";   # blank line between chapter files, else a heading after a paragraph is lost
  close($fh);
}
my @m;
my $skip = 0;
for my $s (@l) {
  if (!$skip && $s =~ /^Contents\s*$/) {
    $skip = 1;
    push @m, "<div id=\"toc-here\"></div>\n\n";
    next;
  }
  $skip = 0 if $skip && $s =~ /^# /;
  push @m, $s if !$skip;
}
my $mdtmp = "$work/$name.tmp.md";
open($fh, '>', $mdtmp) or die "$mdtmp: $!";
print $fh @m;
close($fh);
# section numbers by pandoc when the headings have none in the text (SE2 .md)
my $infence = 0;
my $numbered = grep { $infence = !$infence if /^```/; !$infence && /^#+ +\d/ } @m;   # ignore lines inside code blocks
my $numopt = $numbered ? "" : "--number-sections";
my $body = `cd $work && pandoc -f markdown -t html5 --wrap=none $numopt $mdtmp`;
die "pandoc failed\n" if $? != 0;
# revision date on the title page: newest modification time of the .md input files
my $mtime = 0;
for my $f (@chapfiles ? @chapfiles : ($md)) { my $t = (stat $f)[9]; $mtime = $t if $t > $mtime }
my @mt = localtime $mtime;
my $rev = sprintf "Revision %d-%s-%d %02d:%02d", $mt[3], (qw(jan feb mar apr may jun jul aug sep oct nov dec))[$mt[4]], $mt[5] + 1900, $mt[2], $mt[1];
$body =~ s#(<span class="cover-title">.*?</span>)#$1<span class="cover-date">$rev</span>#;
# code blocks as in the Word docs: function names bold, parameter lines indented like a tab
sub wordcode {
  my $c = shift;
  $c =~ s#\b(swe_\w+)(?=\()#<span class="fnc">$1</span>#g;
  # parameter lines only: after a line ending with "(" or "," (comments ignored)
  my @l = split /\n/, $c, -1;
  my $par = 0;
  for (@l) {
    $_ = "    $_" if $par && /^  \S/;
    (my $x = $_) =~ s#/\*.*?(\*/|$)##g; $x =~ s#//.*##; $x =~ s#<[^>]+>##g;
    $par = $x =~ /[(,]\s*$/ ? 1 : 0;
  }
  return join "\n", @l;
}
$body =~ s#(<pre><code>)(.*?)(</code></pre>)#$1 . wordcode($2) . $3#gse;
# function overview lists (swisseph.md): comments not colored
$body =~ s#(<pre class="funclist"><code>)(.*?)(</code></pre>)#my ($o, $c, $z) = ($1, $2, $3); $c =~ s{(/\*.*?\*/)}{<span class="cmt">$1</span>}gs; $c =~ s{^(swe_\w+)(?=\()}{<span class="fnc">$1</span>}mg; "$o$c$z"#gse;
# table of contents from h1..h3
my $toc = "<nav id=\"toc\">\n<h1 class=\"toc-title\">Contents</h1>\n";
while ($body =~ /<h([123])\b([^>]*)>(.*?)<\/h\1>/sg) {
  my ($lev, $attr, $txt) = ($1, $2, $3);
  # pandoc writes class before id for headings with a class
  next if $attr !~ /\bid="([^"]+)"/;
  my $id = $1;
  $txt =~ s/<[^>]+>//g;
  $txt =~ s/\s+/ /g;
  $txt =~ s/^\s+|\s+$//g;
  next if $txt eq '';
  $toc .= "<a class=\"toc$lev\" href=\"#$id\">$txt</a>\n";
}
$toc .= "</nav>\n";
$body =~ s/<div id="toc-here"><\/div>/$toc/ or $body = $toc . $body;
my %titles = (
  swisseph3 => "Swiss Ephemeris 3",
  swephprg3 => "Swiss Ephemeris 3 - Programming interface",
  swisseph => "Swiss Ephemeris 2.10.04",
  swephprg => "Swiss Ephemeris 2.10.04 - Programming interface",
);
my $title = $titles{$name} // $name;
my $icon = <<'SVG';
<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 40 36" width="20" height="18">
<polygon points="20,2 38,14 30,14 30,34 10,34 10,14 2,14" fill="#f08a30" stroke="#000" stroke-width="2.2" stroke-linejoin="miter"/>
<text x="20" y="30" text-anchor="middle" font-family="DejaVu Sans, sans-serif" font-weight="bold" font-size="16" fill="#000">C</text>
</svg>
SVG
my $css = <<"CSS";
\@page {
  size: A4;
  margin: 22mm 18mm 20mm 20mm;
  \@top-left { content: "$title"; font-size: 8.5pt; color: #444; }
  \@top-right { content: string(chapter); font-size: 8.5pt; color: #444; }
  \@bottom-left { content: "$name"; font-size: 8.5pt; color: #444; }
  \@bottom-center { content: "~ " counter(page) " ~"; font-size: 8.5pt; color: #444; }
  \@bottom-right { content: element(tocicon); }
}
\@page :first {
  \@top-left { content: none; } \@top-right { content: none; }
}
html { font-family: "Calibri", "Carlito", "DejaVu Sans", sans-serif; font-size: 10.5pt; line-height: 1.35; }
body { margin: 0; }
#tocicon { position: running(tocicon); }
#tocicon a { text-decoration: none; }
h1 { string-set: chapter content(); font-size: 17pt; color: #1f4e79; break-before: page; margin: 0 0 8pt 0; }
h2 { font-size: 13.5pt; color: #2e74b5; margin: 14pt 0 5pt 0; break-after: avoid; }
h3 { font-size: 11.5pt; color: #2e74b5; margin: 12pt 0 4pt 0; break-after: avoid; }
h4, h5, h6 { font-size: 10.5pt; margin: 10pt 0 3pt 0; break-after: avoid; }
p { margin: 0 0 6pt 0; }
a { color: #0563c1; }
pre, code { font-family: "DejaVu Sans Mono", monospace; font-size: 8pt; }
pre { white-space: pre-wrap; word-break: break-all; color: #C45911; font-size: 9pt; margin: 4pt 0 8pt 0; }
pre .fnc { font-weight: bold; color: #833C0B; }
pre code { font-size: inherit; }
pre .cmt { color: #1a1a1a; }
.fn { font-family: "DejaVu Sans Mono", monospace; font-weight: bold; color: #833C0B; font-size: 8.5pt; }
.file { color: #008080; }
p code, li code, td code, th code { color: #C45911; font-size: 9pt; }
table { border-collapse: collapse; margin: 4pt 0 8pt 0; font-size: 9pt; }
.nowrap { white-space: nowrap; }
tr { break-inside: avoid; page-break-inside: avoid; }
thead { display: table-header-group; }
ol[type="a"] { list-style-type: lower-alpha; }
ol[type="A"] { list-style-type: upper-alpha; }
ol[type="i"] { list-style-type: lower-roman; }
th, td { border: 0.5pt solid #999; padding: 2pt 4pt; vertical-align: top; }
img { max-width: 100%; }
body > p:first-of-type ~ p > img:only-child { max-height: 110mm; }
.cover { text-align: center; padding-top: 25mm; }
.cover p { margin: 0 0 12pt 0; font-size: 13pt; }
.cover img { max-height: 110mm; }
.cover-title { display: block; font-size: 34pt; font-weight: bold; color: #1f4e79; margin-bottom: 6pt; }
.cover-date { display: block; font-size: 9pt; font-weight: normal; color: #444; margin: 0 0 10pt 0; }
.numtable table { width: 100%; }
pre.chart { font-size: 9.5pt; background: none; color: inherit; white-space: pre; word-break: normal; margin-left: 12pt; }
.numtable td:nth-child(2n), .numtable th:nth-child(2n) { border-right: 1.5pt solid #555; }
blockquote { margin: 0 0 6pt 14pt; }
nav#toc { break-before: page; break-after: page; }
nav#toc .toc-title { string-set: chapter "Contents"; break-before: auto; }
nav#toc a { display: block; color: #000; text-decoration: none; }
nav#toc a::after { content: leader('.') target-counter(attr(href), page); }
nav#toc a.toc1 { font-weight: bold; margin-top: 4pt; }
nav#toc a.toc2 { padding-left: 12pt; }
nav#toc a.toc3 { padding-left: 24pt; font-size: 9.5pt; }
CSS
if ($htm) {
  # web page: images into <name>.fld/, icon "up to contents" at every h1 and h2
  my $odir = dirname($out);
  my $fld = "$name.fld";
  mkdir "$odir/$fld" if !-d "$odir/$fld";
  while ($body =~ /src="(?:\.\/)?media\/([^"]+)"/g) {
    system('cp', '-p', "$work/media/$1", "$odir/$fld/$1") == 0 or die "cp $1 failed\n";
  }
  $body =~ s/src="(?:\.\/)?media\//src="$fld\//g;
  (my $up = $icon) =~ s/width="20" height="18"/width="24" height="22"/;
  my $uplink = "<a class=\"up\" href=\"#toc\" title=\"up to contents\">$up</a>";
  $body =~ s/(<h[12]\b[^>]*\bid="[^"]+"[^>]*>)/$1$uplink/g;
  $body =~ s/(<h1 class="toc-title">)$uplink/$1/;
  my $hcss = <<'CSS';
:root { --fg: #1a1a1a; --bg: #ffffff; --h: #1f4e79; --h2: #2e74b5; --pre: #f4f4f4; --line: #999; --a: #0563c1; --fn: #833C0B; --file: #008080; --code: #C45911; }
@media (prefers-color-scheme: dark) {
  :root { --fg: #e4e4e4; --bg: #1b1d21; --h: #8cb8e8; --h2: #7fb0e0; --pre: #2a2d33; --line: #666; --a: #8ab4f8; --fn: #e0a070; --file: #4fc0c0; --code: #f0a060; }
}
html { font-family: Calibri, Carlito, "DejaVu Sans", sans-serif; font-size: 16px; line-height: 1.45; color: var(--fg); background: var(--bg); }
body { max-width: 52em; margin: 0 auto; padding: 1em 16px 4em 16px; background: var(--bg); }
h1 { font-size: 1.7em; color: var(--h); margin: 2em 0 0.5em 0; border-bottom: 1px solid var(--line); }
h2 { font-size: 1.35em; color: var(--h2); margin: 1.6em 0 0.4em 0; }
h3 { font-size: 1.15em; color: var(--h2); margin: 1.3em 0 0.3em 0; }
h4, h5, h6 { font-size: 1em; margin: 1em 0 0.3em 0; }
a { color: var(--a); }
a.up { float: right; margin-left: 0.5em; text-decoration: none; }
p, li, blockquote, dd, dt, td, th { overflow-wrap: anywhere; }
pre, code { font-family: "DejaVu Sans Mono", Consolas, monospace; font-size: 0.85em; }
pre { color: var(--code); overflow-x: auto; }
pre .fnc { font-weight: bold; color: var(--fn); }
pre .cmt { color: var(--fg); }
.fn { font-family: "DejaVu Sans Mono", Consolas, monospace; font-weight: bold; color: var(--fn); font-size: 0.9em; }
.file { color: var(--file); }
p code, li code, td code, th code { color: var(--code); }
table { border-collapse: collapse; margin: 0.4em 0 1em 0; font-size: 0.92em; display: block; overflow-x: auto; }
.nowrap { white-space: nowrap; }
ol[type="a"] { list-style-type: lower-alpha; }
ol[type="A"] { list-style-type: upper-alpha; }
ol[type="i"] { list-style-type: lower-roman; }
th, td { border: 1px solid var(--line); padding: 0.15em 0.4em; vertical-align: top; }
img { max-width: 100%; height: auto; }
.cover { text-align: center; padding-top: 2em; }
.cover p { font-size: 1.2em; }
.cover img { max-height: 60vh; }
.cover-title { display: block; font-size: 2.6em; font-weight: bold; color: var(--h); }
.cover-date { display: block; font-size: 0.8em; font-weight: normal; color: var(--fg); margin-bottom: 0.8em; }
.numtable table { display: table; width: 100%; }
pre.chart { font-size: 0.95em; background: none; color: inherit; }
.numtable td:nth-child(2n), .numtable th:nth-child(2n) { border-right: 2px solid var(--fg); }
blockquote { margin: 0 0 0.5em 1.2em; }
nav#toc a { display: block; text-decoration: none; padding: 0.05em 0; }
nav#toc a.toc1 { font-weight: bold; margin-top: 0.5em; }
nav#toc a.toc2 { padding-left: 1.2em; }
nav#toc a.toc3 { padding-left: 2.4em; font-size: 0.92em; }
CSS
  open($fh, '>', $out) or die "$out: $!";
  print $fh "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"utf-8\">\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n<title>$title</title>\n<style>\n$hcss</style></head>\n<body>\n";
  print $fh $body;
  print $fh "</body></html>\n";
  close($fh);
  print "$out\n";
  exit 0;
}
my $html = "$work/$name.html";
open($fh, '>', $html) or die "$html: $!";
print $fh "<!DOCTYPE html>\n<html lang=\"en\"><head><meta charset=\"utf-8\"><title>$title</title>\n<style>\n$css</style></head>\n<body>\n";
print $fh "<div id=\"tocicon\"><a href=\"#toc\">$icon</a></div>\n";
print $fh $body;
print $fh "</body></html>\n";
close($fh);
system('weasyprint', '-q', '-u', "$work/", $html, $out) == 0 or die "weasyprint failed\n";
print "$out\n";
print "html kept: $html\n" if $keep;
