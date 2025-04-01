readme.md  created 5-oct-2017 by Alois Treindl

last update 1-apr-2025

## About the Swiss Ephemeris:

Swiss Ephemeris (SE) is a software toolbox for programmers of astrological
software. It is of little use for a non-programmer

Swiss Ephemeris was developed by Dieter Koch and Alois Treindl, while both were
employed by Astrodienst AG in Zollikon/Zürich Switzerland.

Most astrological programmers all over the world use Swiss Ephemeris in their
software. 

SE ist built according to the highest standards of precision available
in astronomical data, with raw data from Nasa's JPL. It is able to reproduce the ephemeris data in
astronomical standard books up to the last printed digit.

Detailed information is available at https://www.astro.com/swisseph
or in the documentation files included here (folder doc).

# Download location of files:

## Compressed Swiss Ephemeris planet and main asteroid files

In the public Swiss Ephemeris github repository
folder ephe
https://github.com/aloistr/swisseph/tree/master/ephe

This directory contains also in list.zip the lists of all asteroid files and their
most recent updates.

	list_short.txt		recent updates short files
	all_short.txt
	list_long.txt		recent updates long files
	all_long.txt

or from Alois' public Dropbox area in folder 'ephe' with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

## Swiss Ephemeris source code

In the public Swiss Ephemeris Github repository
https://github.com/aloistr/swisseph

## Swiss Ephemeris Documentation

in the public Swiss Ephemeris Github repository
folder doc/
https://github.com/aloistr/swisseph/tree/master/doc

## Swiss Ephemeris for Windows

see folder windows/
https://github.com/aloistr/swisseph/tree/master/windows

## JPL files

to be downloaded directly from JPL, see
https://www.astro.com/swisseph-download/jplfiles/

- de200.eph  size 41 Mb  [download de200.eph](https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/de200/lnxm1600p2170.200)
- de406.eph  size 190 Mb [download de406.eph](https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/de406/lnxm3000p3000.406)
- de431.eph  size 2.6 Gb [download de431.eph](https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/de431/lnxm13000p17000.431)
- de441.eph  size 2.6 Gb [download de441.eph](https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/de441/linux_m13000p17000.441)

 	These download links refer to the JPL area https://ssd.jpl.nasa.gov/ftp/eph/planets/Linux/` 
 	After download, files like de441/linux_m13000p17000.441 must be renamed de441.eph to be recognized by Swiss Ephemeris code.`
or from Alois' public Dropbox area in folder 'jpl binary files'  with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

	md5-keys
	1ef6191b614b2b854adae8675b1b981f  de200.eph
	1ef768440cc1617b6c8ad27a9a788135  de406e.eph
	fad0f432ae18c330f9e14915fbf8960a  de431.eph
	a7b2a5b8b2ebed52ea4da2304958053b  de441.eph

or from https://ephe.scryr.io/jpl/  which is a web space provided by  Phillip McCabe


## Asteroid files for all available numbered asteroids (more than 760'000)

Each file covers 600 years, from 1500 - 2099. They are organized in folders of 1000 files each, 
available in Alois' public Dropbox area in folder 'all_ast' with this link:

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

or from https://ephe.scryr.io/ the web space provided by Phillip McCabe

The total volume was 29 Gb in September 2023.

## Asteroid files for all available named asteroids (more than 25'000)

Each file covers the long range of 6000 years, from 3000 BCE to 2999 CE. 
available in Alois' public Dropbox area in folder 'long_ast' with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

or from https://ephe.scryr.io/ephe2 the web space provided by Phillip McCabe

The total volume was 11 Gb in September 2023.

## How to organize Ephemeris files so that SwissEph finds them
Swissephe code uses an internal swed.ephepath for the directory names where it looks
for its data files.

It defaults to \sweph\ephe on Windows and to  ".:/users/ephe2/:/users/ephe/" on Linux or other
Unix-like systems like Android or Mac OS-X.

The characters ; in Windows and ; or : in Linux serve as path separators.
In the default setting on Linux, the three directories . , /users/ephe and /users/ephe2 are
searched when Swiss Ephemeris looks for a file.

The programmer can call the function swe_set_ephe_path() to set the ephepath variable,
and may include path separators.

The user of a program can also set an evironment variable which override the builtin default.

The compressed planetary ephemeris files like sepl*.se1, sem*.se1 and seas*.se1 must be directly in one of
the path elements.

Asteroid files must be in subdirectories named astN  like ast0, ast1, ... ast623,
where N is the asteroid number divided by 1000.

The folders all_ast and long_ast are only used inside the Dropbox area to organize the files. Once downloaded,
the astN directory must be placed directly in one of the elements of ephepath.
Short asteroid files and long asteroid filese differ by an s in the file name, like se1001.ses and se1001s.se1.
They can be merged into the same astN directory, in this case ast1.
Astrodienst keeps them traditionally separate in /ephe and /ephe2, but that distinction is not technically necessary.


## Contributed code

folder contrib:
It contains open source code and applications using the Swiss Ephemeris.
See the readme file in directory contrib for more details.


### Legal restrictions

The SWISS EPHEMERIS can be licensed by programmers to include this 
calculation engine in their software. More information is found at
https://www.astro.com/swisseph/

Depending on your application the free edition under the Public License
may apply, or you may have to acquire a professional license for a fee.

Read LICENSE.TXT

### Mailing list
We maintain a mailing list which servers for discussion between developers and 
as the main support channel. All Swiss Ephemeris support is public. There is
no private support by email.

If you want to have your name added to this mailing list, please
visit https://groups.io/g/swisseph

## Feedback

We welcome any suggestions and comments you may have about the Swiss Ephemeris.
Please email to swisseph@groups.io

If you want your feedback distributed to all members of the swisseph
mailing list, please subscribe to https://groups.io/g/swisseph

## Java-Version:

Thomas Mack has ported the Swiss Ephemeris library to Java.

His work can be found at:  http://www.th-mack.de/international/download/

If you use it for commercial or for non-open-source purposes, please
be aware that AGPL Public License of Swiss Ephemeris 
also applies for the Java version, besides any additional requirements
which may be defined by Thomas Mack.

## PHP-Version
found on Github https://github.com/cyjoelchen/php-sweph

## Perl-Version
found on Github https://github.com/aloistr/perl-sweph

## Numerical Integrator
the numerical integrator to prepare swisss ephemeris files is not in a state
fit for publication.
