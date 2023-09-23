readme.md  created 5-oct-2017 by Alois Treindl

last update 20 September 2023

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

or from Alois' public Dropbox area in folder 'ephe' with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

## Swiss Ephemeris source code

In the public Swiss Ephemeris Github repository
https://github.com/aloistr/swisseph

## Swiss Ephemeris Documentation

in the public Swiss Ephemeris Github repository
folder doc
https://github.com/aloistr/swisseph/tree/master/doc

## Swiss Ephemeris for Windows

in the public Swiss Ephemeris github repository
folder windows
https://github.com/aloistr/swisseph
containing

sweph.zip

	The complete Swiss Ephemeris package for Windows (32bit),
	except the ephemeris data files.
	See online documentation at https://www.astro.com/swisseph
	or in the doc directory for more details.
	Small sample programs and build-projects are included for
	- Visual C++ .net 2003
	- Visual Basic 5.0 
	- Delphi 32-bit
	Note: Visual Basic files may not have been updated and
	tested, as we currently do not possess a working copy of VB at Astrodienst.

swephzip.txt

	overview of content of the sweph.zip package

folder vb:

	declarations for 32-bit and 64-bit Visual Basic

folder programs:

	A directory containing sample and utility programs for Windows:
	swetest.exe:
	executable Swiss Ephemeris program.
	This is a 32-bit Console mode application.
	To get information about the many features of this program, run
	swetest -? > out.txt
	and print the file out.txt, or view it with your text editor.
	This is just a sample to demonstrate the precision of the
	ephemeris, and to show the typical use of a few calls.


## JPL files

now directly from jpl, see
https://www.astro.com/ftp/swisseph/jplfiles/index.htm

or from Alois' public Dropbox area in folder 'jpl binary files'  with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

	md5-keys
	1ef6191b614b2b854adae8675b1b981f  de200.eph
	1ef768440cc1617b6c8ad27a9a788135  de406e.eph
	fad0f432ae18c330f9e14915fbf8960a  de431.eph
	a7b2a5b8b2ebed52ea4da2304958053b  de441.eph

## Asteroid files for all available numbered asteroids (more than 620'000)

Each file covers 600 years, from 1500 - 2099. They are organized in folders of 1000 files each, 
available in Alois' public Dropbox area in folder 'all_ast' with this link:

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

The ast* folders are organized in folders like ast_0_99 in groups of 100 each. Dropbox is a bit 
slow with large numbers of folders which each contain 1000 files. It handles 100 such folders better then 620.

The total volume was 29 Gb in September 2023.

## Asteroid files for all available named asteroids (more than 24'000)

Each file covers the long range of 6000 years, from 3000 BCE to 2999 CE. 
available in Alois' public Dropbox area in folder 'long_ast' with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

The total volume was 11 Gb in September 2023.

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
