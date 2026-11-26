## Please note:
The official Swiss Ephemeris documentation is found at https://www.astro.com/ftp/swisseph/doc/

Since release 2.10.04, the documentation is maintained in Markdown:

- swisseph.md   general documentation of the Swiss Ephemeris
- swephprg.md   programming interface
- media/        images used in swisseph.md

The PDF and HTM files are created out of the .md files with the Perl script
mk_pdf.pl (pandoc and weasyprint), see makefile:

    make pdf      swisseph.pdf, swephprg.pdf
    make htm      swisseph.htm, swephprg.htm (one file each, images embedded)

secont_e_2609.pdf is the license contract for the Swiss Ephemeris Professional Edition.
