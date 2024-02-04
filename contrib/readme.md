readme.md	created 19-may-2010 by Alois Treindl
		last update 4-feb-2024

Directory contrib: free contributions to Swiss Ephemeris

This directory (and subdirectories) contains free contributions
to Swiss Ephemeris. This means software under an open source license
(usually GPL) which extends or uses Swiss Ephemeris in a meaningful and useful
way.

Please note that Astrodienst is not responsible in any way for these
contributions. They are presented here without any check of their content.
They may contain errors or even bad viral code. Please check carefully what
you use in your development.

List of contributions:

rp_source_0_9.zip
source: http://radixpro.org/download/rp_source_0_9.zip
c# source code for an astrological application, by Jan Kampherbeek.
Of special interest may be the file sweph.cs which shows how within .NET
calls to the non-managed Swiss Ephemeris DLL can be made.

swissdelphi_2.07.01.zip
contributed by Pierre Fontaine
An interface for Delphi (Pascal code) to call Swiss Ephemeris functions.

projectPF_VS2017.zip	5-oct-2017 
contributed by Pierre Fontaine
Project files from Visual Studio 2017
Should be unpacked after unpacking sweph.zip within the 'src' directory
In the directory 'projectPF' you have a global solution 'sweph.sln' with
the different 'vcxproj' files to build the dlls.

android/jni/ subdirectory  4-feb-2024

Yuriy Krymlov <krymlov@gmail.com> created a JNI (Java) interface to Swisseph
on Github https://github.com/krymlov/swe-jni-lib
It has been included here. Compiled libraries for Android are provided.

Sweph32_For_Excel_VBA_and_VB.zip   16-oct-21
uploaded by D.Senthilathiban (Email:athi_ram@yahoo.com)  to groupis.io files under the name
Swedll32(For_Excel_VBA_and_VB).zip and contains
1) modSwedll32.bas     - Contains Swiss Ephemeris API Declarations for Visual Basic 32-bit.
2) clsTestSwedll32.cls - Test subroutines for Swiss Ephemeris Visual Basic 32-bit API functions.
3) modTestSwedll32.bas - Test module to call Test subroutines defined in the above class module.
4) Swedll32.xls        - Includes all the above standard/Class module files in Excel VBA Project 
		     to use/test Swiss Ephemeris Visual Basic 32-bit API declared functions.
5) TestSwedll32.txt    -  Sample output file showing Test results.
6) Readme_Swedll32.txt 

See also the 'files' are in gthe mailing list archive
https://groups.io/g/swisseph/files

php:
the most current release of php-sweph is on github
https://github.com/cyjoelchen/php-sweph
A compiled 64-bit binary swephp.so is find in subdirectory swephp.
It was compiled on Redhat Enterprise Linux 7.
It should work on all 64-bit linux systems.
Its dependencies are only
	libm.so.6 
	libc.so.6

