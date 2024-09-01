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

## Asteroid files for all available numbered asteroids (more than 620'000)

Each file covers 600 years, from 1500 - 2099. They are organized in folders of 1000 files each, 
available in Alois' public Dropbox area in folder 'all_ast' with this link:

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

The total volume was 29 Gb in September 2023.

## Asteroid files for all available named asteroids (more than 24'000)

Each file covers the long range of 6000 years, from 3000 BCE to 2999 CE. 
available in Alois' public Dropbox area in folder 'long_ast' with this link

https://www.dropbox.com/scl/fo/y3naz62gy6f6qfrhquu7u/h?rlkey=ejltdhb262zglm7eo6yfj2940&dl=0

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


## How to use Swetest

Swetest computes a complete set of geocentric planetary positions, for a given date or a sequence of dates. Input can either be a date or an absolute julian day number.
0:00 (midnight). With the proper options, swetest can be used to output a printed ephemeris and transfer the data into other programs like spreadsheets for graphical display.

**Version:** 2.10.03

### Help commands:

- `-?, -h` display whole info

- `-hcmd` display commands

- `-hplan` display planet numbers

- `-hform` display format characters

- `-hdate` display input date format

- `-hexamp` display examples

- `-glp` report file location of library

### Input time formats:

- `-bDATE` begin date; e.g. -b1.1.1992 if
  Note: the date format is day month year (European style).
- `-bj...` begin date as an absolute Julian day number; e.g. `-bj2415020.5`

- `-j...` same as `-bj`

- `-tHH[:MM[:SS]]` input time (as Ephemeris Time)

- `-ut` input date is Universal Time (UT1)

- `-utHH[:MM[:SS]]` input time (as Universal Time)

- `-utcHH[:MM[:SS]]` input time (as Universal Time Coordinated UTC). H,M,S can have one or two digits. Their limits are unchecked.

- Output time for eclipses, occultations, risings/settings is UT by default
  - `-lmt` output date/time is LMT (with -geopos)
  - `-lat` output date/time is LAT (with -geopos)

### Object, number of steps, step with

- `-pSEQ` planet sequence to be computed. See the letter coding below.

- `-dX` differential ephemeris: print differential ephemeris between body X and each body in list given by `-p`
  example: `-p2 -d0 -fJl -n366 -b1.1.1992` prints the longitude distance between SUN (planet `0`) and MERCURY (planet `2`)
  for a full year starting at 1 Jan 1992.

- `-dhX` differential ephemeris: print differential ephemeris between heliocentric body X and each body in list given by `-p` example: `-p8 -dh8 -ftl -n36600 -b1.1.1500 -s5` prints the longitude distance between geocentric and heliocentric Neptune (planet `8`) for 500 year starting at 1 Jan 1500. Using this option mostly makes sense for a single planet to find out how much its geocentric and heliocentric positions can differ over extended periods of time.

- `-DX` midpoint ephemeris, works the same way as the differential mode `-d` described above, but outputs the midpoint position.

- `-nN` output data for N consecutive timesteps; if no `-n` option is given, the default is `1`. If the option `-n` without a number is given, the default is `20`.

- `-sN` timestep N days, default 1. This option is only meaningful when combined with option `-n`. If an `y` is appended, the time step is in years instead of days, for example `-s10y` for a time step of 10 years. If an 'mo' is appended, the time step is in months instead of days, for example `-s3mo` for a time step of 3 months. If an `m` is appended, the time step is in minutes instead of days, for example `-s15m` for a time step of 15 minutes. If an `s` is appended, the time step is in seconds instead of days, for example `-s1s` for a time step of 1 second.

### Output format:

- `-fSEQ` use `SEQ` as format sequence for the output columns; default is `PLBRS`.

- `-head` don't print the header before the planet data. This option is useful when you want to paste the output into a spreadsheet for displaying graphical ephemeris.

- `+head` header before every step (with `-s..`).

- `-gPPP` use `PPP` as gap between output columns; default is a single blank. `-g` followed by white space sets the gap to the TAB character; which is useful for data entry into spreadsheets.

- `-hor` list data for multiple planets 'horizontally' in same line. all columns of `-fSEQ` are repeated except time colums `tTJyY`.

### Astrological house system:

- `-house[long,lat,hsys]`
  include house cusps. The longitude, latitude (degrees with DECIMAL fraction) and house system letter can be given, with commas separated, + for east and north. If none are given, Greenwich UK and Placidus is used: `0.00,51.50,p.`. The output lists 12 house cusps, Asc, MC, ARMC, Vertex, Equatorial Ascendant, co-Ascendant as defined by Walter Koch, co-Ascendant as defined by Michael Munkasey, and Polar Ascendant. Houses can only be computed if option `-ut` is given.

  - `A` equal
  - `B` Alcabitius
  - `C` Campanus
  - `D` equal / MC
  - `E` equal = A
  - `F` Carter poli-equatorial
  - `G` 36 Gauquelin sectors
  - `H` horizon / azimuth
  - `I` Sunshine
  - `i` Sunshine alternative
  - `K` Koch
  - `L` Pullen S-delta
  - `M` Morinus
  - `N` Whole sign, Aries = 1st house
  - `O` Porphyry
  - `P` Placidus
  - `Q` Pullen S-ratio
  - `R` Regiomontanus
  - `S` Sripati
  - `T` Polich/Page ("topocentric")
  - `U` Krusinski-Pisa-Goelzer
  - `V` equal Vehlow
  - `W` equal, whole sign
  - `X` axial rotation system/ Meridian houses
  - `Y` APC houses

    The use of lower case letters is deprecated. They will have a different meaning in future releases of Swiss Ephemeris.

- `-hsy[hsys]` house system to be used (for house positions of planets) for `long`, `lat`, `hsys`, see `-house`. The use of lower case letters is deprecated. They will have a different meaning in future releases of Swiss Ephemeris.

- `-geopos[long,lat,elev]` Geographic position. Can be used for azimuth and altitude or house cusps calculations. The longitude, latitude (degrees with DECIMAL fraction) and elevation (meters) can be given, with commas separated, + for east and north. If none are given, Greenwich is used: `0,51.5,0`. For topocentric planet positions please user the parameter `-topo`

## Sidereal astrology:

- `-ay..` ayanamsha, with number of method, e.g. ay0 for Fagan/Bradley
- `-sid..` sidereal, with number of method (see below)
- `-sidt0..` dito, but planets are projected on the ecliptic plane of the reference date of the ayanamsha (more info in [general documentation](www.astro.com/swisseph/swisseph.htm))
- `-sidsp..` dito, but planets are projected on the solar system planev(see www.astro.com/swisseph/swisseph.htm).
- `-sidudef[jd,ay0,...]` sidereal, with user defined ayanamsha;
  - `jd` = julian day number in TT/ET
  - `ay0` = initial value of ayanamsha,
  - `...` = optional parameters, comma-sparated:
  - `jdisut`: ayanamsha reference date is UT
  - `eclt0`: project on ecliptic of reference date (like -sidt0..)
  - `ssyplane`: project on solar system plane (like -sidsp..)
  - e.g. `-sidudef2452163.8333333,25.0,jdisut`: ayanamsha is 25.0° on JD 2452163.8333333 UT

### Number of ayanamsha method:

- `0` for Fagan/Bradley
- `1` for Lahiri
- `2` for De Luce
- `3` for Raman
- `4` for Usha/Shashi
- `5` for Krishnamurti
- `6` for Djwhal Khul
- `7` for Yukteshwar
- `8` for J.N. Bhasin
- `9` for Babylonian/Kugler 1
- `10` for Babylonian/Kugler 2
- `11` for Babylonian/Kugler 3
- `12` for Babylonian/Huber
- `13` for Babylonian/Eta Piscium
- `14` for Babylonian/Aldebaran = 15 Tau
- `15` for Hipparchos
- `16` for Sassanian
- `17` for Galact. Center = 0 Sag
- `18` for J2000
- `19` for J1900
- `20` for B1950
- `21` for Suryasiddhanta
- `22` for Suryasiddhanta, mean Sun
- `23` for Aryabhata
- `24` for Aryabhata, mean Sun
- `25` for SS Revati
- `26` for SS Citra
- `27` for True Citra
- `28` for True Revati
- `29` for True Pushya (PVRN Rao)
- `30` for Galactic (Gil Brand)
- `31` for Galactic Equator (IAU1958)
- `32` for Galactic Equator
- `33` for Galactic Equator mid-Mula
- `34` for Skydram (Mardyks)
- `35` for True Mula (Chandra Hari)
- `36` Dhruva/Gal.Center/Mula (Wilhelm)
- `37` Aryabhata 522
- `38` Babylonian/Britton
- `39` Vedic/Sheoran
- `40` Cochrane (Gal.Center = 0 Cap)
- `41` Galactic Equator (Fiorenza)
- `42` Vettius Valens
- `43` Lahiri 1940
- `44` Lahiri VP285 (1980)
- `45` Krishnamurti VP291
- `46` Lahiri ICRC

## Ephemeris specifications:

- `-edirPATH` change the directory of the ephemeris files
- `-eswe` swiss ephemeris
- `-ejpl` jpl ephemeris (DE431), or with ephemeris file name `-ejplde200.eph`
- `-emos` moshier ephemeris
- `-true` true positions
- `-noaberr` no aberration
- `-nodefl` no gravitational light deflection
- `-noaberr -nodefl` astrometric positions
- `-j2000` no precession (i.e. J2000 positions)
- `-icrs` ICRS (use Internat. Celestial Reference System)
- `-nonut` no nutation
- `-speed` calculate high precision speed
- `-speed3` 'low' precision speed from 3 positions (do not use this option. `-speed parameter` is faster and more precise).
- `-iXX` force iflag to value XX
- `-testaa96` test example in AA 96, B37, i.e. venus, j2450442.5, DE200. attention: use precession IAU1976 and nutation 1980 (s. swephlib.h)
- `-testaa95`
- `-testaa97`
- `-roundsec` round to seconds
- `-roundmin` round to minutes
- `-ep` use extra precision in output for some data
- `-dms` use dms instead of fractions, at some places
- `-lim` print ephemeris file range

## observer position:

- `-hel` compute heliocentric positions
- `-bary` compute barycentric positions (bar. earth instead of node)
- `-topo[long,lat,elev]` topocentric positions. The longitude, latitude (degrees with DECIMAL fraction) and elevation (meters) can be given, with commas separated, + for east and north. If none are given, Greenwich is used 0.00,51.50,0
- `-pc...` compute planetocentric positions to specify the central body, use the internal object number of Swiss Ephemeris, e.g. 3 for Venus, 4 for Mars,
- `-pc3` Venus-centric
- `-pc4` Mars-centric
- `-pc5` Jupiter-centric (barycenter)
- `-pc9599` Jupiter-centric (center of body)
- `-pc9699` Saturn-centric (center of body) For asteroids use MPC number + 10000, e.g.
- `-pc10433` Eros-centric (Eros = 433 + 10000)

## Orbital elements:

- `-orbel` compute osculating orbital elements relative to the mean ecliptic J2000. (Note, all values, including time of
  pericenter vary considerably depending on the date for which the osculating ellipse is calculated).

## Special events:

### Solar Eclipses:

`-solecl` solar eclipse.

- **Output 1st line:**

  - Eclipse date
  - Time of maximum (UT):
  - Geocentric angle between centre of Sun and Moon reaches minimum.
  - Core shadow width (negative with total eclipses),
  - Eclipse magnitudes:
    - NASA method (= 2. with partial ecl. and ratio lunar/solar diameter with total and annular ecl.)
    - Fraction of solar diameter covered by moon; if the value is > 1, it means that Moon covers more than just the solar disk
    - Fraction of solar disc covered by moon (obscuration) with total and annular eclipses it is the ratio of the sizes of the solar disk and the lunar disk.
  - Saros series and eclipse number
  - Julian day number (6-digit fraction) of maximum

- **Output 2nd line:**

  - Start and end times for partial and total phases
  - Delta t in sec

- **Output 3rd line:**

  - Geographical longitude and latitude of maximum eclipse,
  - Totality duration at that geographical position (output with `-local`, see below).

### Lunar eclipses

`-lunecl` for lunar eclipse

- **Output 1st line:**

  - Eclipse date,
  - Time of maximum (UT),
  - Eclipse magnitudes: umbral and penumbral (method as method 2 with solar eclipses)
  - Saros series and eclipse number
  - Julian day number (6-digit fraction) of maximum

- **Output 2nd line:**

  - 6 contacts for start and end of penumbral, partial, and total phase
  - Delta t in sec

- **Output 3rd line:**

  - Geographic position where the Moon is in zenith at maximum eclipse

### With `-local`

`-local` only with `-solecl` or `-occult`, if the next event of this kind is wanted for a given geogr. position. Use `-geopos[long,lat,elev]` to specify that position. If `-local` is not set, the program searches for the next event anywhere on earth.

- **Output 1st line:**

  - Eclipse date,
  - Time of maximum,
  - Eclipse magnitudes, as with global solar eclipse function (with occultations: only diameter method, see solar eclipses, method 2)
  - Saros series and eclipse number (with solar eclipses only)
  - Julian day number (6-digit fraction) of maximum

- **Output 2nd line:**

  - Local eclipse duration for totality (zero with partial occultations)
  - Local four contacts,
  - Delta t in sec
    Occultations with the remark "(daytime)" cannot be observed because they are taking place by daylight. Occultations with the remark "(sunrise)" or "(sunset)" can be observed only partly because part of them takes place in daylight.

### Occultations

`-occult` occultation of planet or star by the moon. Use `-p` to specify planet (`-pf -xfAldebaran` for stars). Output format same as with `-solecl`, with the following differences:

- Magnitude is defined like no. 2. with solar eclipses.
- There are no saros series.

### Heliacal events:

- `-hev[type]` heliacal events,
  - type 1 = heliacal rising
  - type 2 = heliacal setting
  - type 3 = evening first
  - type 4 = morning last
  - type 0 or missing = all four events are listed.
- `-rise` rising and setting of a planet or star. Use `-geopos[long,lat,elev]` to specify geographical position.
- `-metr` southern and northern meridian transit of a planet of star. Use `-geopos[long,lat,elev]` to specify geographical position.

## Specifications for eclipses:

- `-total` total eclipse (only with `-solecl`, `-lunecl`).
- `-partial` partial eclipse (only with `-solecl`, `-lunecl`)
- `-annular` annular eclipse (only with `-solecl`)
- `-anntot` annular-total (hybrid) eclipse (only with `-solecl`)
- `-penumbral` penumbral lunar eclipse (only with `-lunecl`)
- `-central` central eclipse (only with `-solecl`, nonlocal)
- `-noncentral` non-central eclipse (only with `-solecl`, nonlocal)

## Specifications for risings and settings:

- `-norefrac` neglect refraction (with option `-rise`)
- `-disccenter` find rise of disc center (with option `-rise`)
- `-discbottom` find rise of disc bottom (with option `-rise`)
- `-hindu` hindu version of sunrise (with option `-rise`)

## Specifications for heliacal events:

- `-at[press,temp,rhum,visr]` (default values are `-at1013.25,15,40,0`): pressure in hPa, temperature in degrees Celsius, relative humidity in %, visual range, interpreted as follows:
- `> 1`: meteorological range in km
- `1>visr>0`: total atmospheric coefficient (ktot)
- `= 0`: calculated from press, temp, rhum
- `-obs[age,SN]` age of observer and Snellen ratio (default values are `-obs36,1`)
- `-opt[age,SN,binocular,magn,diam,transm]` (default values are `-opt36,1,1,1,0,0` with naked eye):
- `age` and `SN` as with `-obs`
- `0` monocular or `1` binocular
- telescope magnification
- optical aperture in mm
- optical transmission

For backward search: `-bwd`

## Planet selection letters:

**Planetary lists:**

- `d` (default) main factors 0123456789mtABCcg
- `p` main factors as above, plus main asteroids DEFGHI
- `h` ficticious factors J..X
- `a` all factors
  (the letters above can only appear as a single letter).

**Single body numbers/letters:**

- `0` Sun (character zero)
- `1` Moon (character 1)
- `2` Mercury
- `3` Venus
- `4` Mars
- `5` Jupiter
- `6` Saturn
- `7` Uranus
- `8` Neptune
- `9` Pluto
- `m` mean lunar node
- `t` true lunar node
- `n` nutation
- `o` obliquity of ecliptic
- `q` delta t
- `y` time equation
- `b` ayanamsha
- `A` mean lunar apogee (Lilith, Black Moon)
- `B` osculating lunar apogee
- `c` intp. lunar apogee
- `g` intp. lunar perigee
- `C` Earth (in heliocentric or barycentric calculation)
- For planets Jupiter to Pluto the center of body (COB) can be calculated using the additional parameter `-cob`
- `F` Ceres
- `9` Pluto
- `s -xs136199` Eris
- `s -xs136472` Makemake
- `s -xs136108` Haumea

**Some minor planets:**

- `D` Chiron
- `E` Pholus
- `G` Pallas
- `H` Juno
- `I` Vesta
- `s` minor planet, with MPC number given in `-xs`

## Some planetary moons and center of body of a planet:

- `v` with moon number given in `-xv`:
- `v -xv9501` Io/Jupiter:
- `v -xv9599` Jupiter, center of body (COB):
- `v -xv94..` Mars moons:
- `v -xv95..` Jupiter moons and COB:
- `v -xv96..` Saturn moons and COB:
- `v -xv97..` Uranus moons and COB:
- `v -xv98..` Neptune moons and COB:
- `v -xv99..` Pluto moons and COB:
  The numbers of the moons are given [here](https://www.astro.com/ftp/swisseph/ephe/sat/plmolist.txt)

## Fixed stars:

- `f` fixed star, with name or number given in `-xf` option
- `f -xfSirius` Sirius

## Fictitious objects:

- `J` Cupido
- `K` Hades
- `L` Zeus
- `M` Kronos
- `N` Apollon
- `O` Admetos
- `P` Vulkanus
- `Q` Poseidon
- `R` Isis (Sevin)
- `S` Nibiru (Sitchin)
- `T` Harrington
- `U` Leverrier's Neptune
- `V` Adams' Neptune
- `W` Lowell's Pluto
- `X` Pickering's Pluto
- `Y` Vulcan
- `Z` White Moon
- `w` Waldemath's dark Moon
- `z` hypothetical body, with number given in `-xz`

sidereal time:

- `x` sidereal time
- `e` print a line of labels

## Output format SEQ letters:

In the standard setting five columns of coordinates are printed with the default format PLBRS. You can change the default by providing an option like `-fCCCC` where `CCCC` is your sequence of columns. The coding of the sequence is like this:

- `y` year
- `Y` `year.fraction_of_year`
- `p` planet index
- `P` planet name
- `J` absolute juldate
- `T` date formatted like `23.02.1992`
- `t` date formatted like `920223` for 1992 february 23
- `L` longitude in degree `ddd mm'ss"`
- `l` longitude decimal
- `Z` longitude `ddsignmm'ss"`
- `S` speed in longitude in degree `ddd:mm:ss` per day
- `SS` speed for all values specified in `fmt`
- `s` speed longitude decimal (degrees/day)
- `ss` speed for all values specified in `fmt`
- `B` latitude degree
- `b` latitude decimal
- `R` distance decimal in AU
- `r` distance decimal in AU, Moon in seconds parallax
- `W` distance decimal in light years
- `w` distance decimal in km
- `q` relative distance (1000=nearest, 0=furthest)
- `A` right ascension in `hh:mm:ss`
- `a` right ascension hours decimal
- `m` Meridian distance
- `z` Zenith distance
- `D` declination degree
- `d` declination decimal
- `I` azimuth degree
- `i` azimuth decimal
- `H` altitude degree
- `h` altitude decimal
- `K` altitude (with refraction) degree
- `k` altitude (with refraction) decimal
- `G` house position in degrees
- `g` house position in degrees decimal
- `j` house number 1.0 - 12.99999
- `X` `x-`, `y-`, and `z-` coordinates ecliptical
- `x` `x-`, `y-`, and `z-`ccoordinates equatorial
- `U` unit vector ecliptical
- `u` unit vector equatorial
- `Q` `l`, `b`, `r`, `dl`, `db`, `dr`, `a`, `d`, `da`, `dd`
- `n` nodes (mean): ascending/descending (Me - Ne); longitude decimal
- `N` nodes (osculating): ascending/descending, longitude; decimal
- `f` apsides (mean): perihelion, aphelion, second focal point; longitude dec.
- `F` apsides (osc.): perihelion, aphelion, second focal point; longitude dec.
- `+` phase angle
- `-` phase
- `*` elongation
- `/` apparent diameter of disc (without refraction)
- `=` magnitude
- `v` (reserved)
- `V` (reserved)

## Date entry:

In the interactive mode, when you are asked for a start date, you can enter data in one of the following formats:

- `1.2.1991`: three integers separated by a nondigit character for day month year. Dates are interpreted as Gregorian after 4.10.1582 and as Julian Calendar before. Time is always set to midnight (0 h). If the three letters jul are appended to the date, the Julian calendar is used even after 1582. If the four letters greg are appended to the date, the Gregorian calendar is used even before 1582.

- `j2400123.67` the letter `j` followed by a real number, for the absolute Julian daynumber of the start date. Fraction `.5` indicates midnight, fraction `.0` indicates noon, other times of the day can be chosen accordingly.

- `<RETURN>`: repeat the last entry
- `.`: stop the program

- `+20`: advance the date by 20 days

- `-10` go back in time 10 days

## Examples:

- `swetest -p2 -b1.12.1900 -n15 -s2`
  ephemeris of Mercury (`-p2`) starting on 1 Dec 1900,
  15 positions (`-n15`) in two-day steps (`-s2`)

- `swetest -p2 -b1.12.1900 -n15 -s2 -fTZ -roundsec -g, -head`
  same, but output format = date and zodiacal position (`-fTZ`),
  separated by comma (`-g,`) and rounded to seconds (`-roundsec`),
  without header (`-head`).

- `swetest -ps -xs433 -b1.12.1900`
  position of asteroid 433 Eros (`-ps -xs433`)

- `swetest -pf -xfAldebaran -b1.1.2000`
  position of fixed star Aldebaran

- `swetest -p1 -d0 -b1.12.1900 -n10 -fPTl -head`
  angular distance of moon (`-p1`) from sun (`-d0`) for 10
  consecutive days (`-n10`).

- `swetest -p6 -DD -b1.12.1900 -n100 -s5 -fPTZ -head -roundmin`
  Midpoints between Saturn (`-p6`) and Chiron (`-DD`) for 100
  consecutive steps (`-n100`) with 5-day steps (`-s5`) with
  longitude in degree-sign format (`-f..Z`) rounded to minutes (`-roundmin`)

- `swetest -b5.1.2002 -p -house12.05,49.50,K -ut12:30`
  Koch houses for a location in Germany at a given date and time

- `swetest -b1.1.2016  -g -fTlbR -p0123456789Dmte -hor -n366 -roundsec`
  tabular ephemeris (all planets Sun - Pluto, Chiron, mean node, true node)
  in one horizontal row, tab-separated, for 366 days. For each planet
  list longitude, latitude and geocentric distance.

