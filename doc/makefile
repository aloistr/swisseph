# makefile to create the PDF and HTM documentation out of the .md files
# tools: perl, pandoc, weasyprint (PDF only), fonts DejaVu Sans / DejaVu Sans Mono
# images of swisseph.md are in media/; the .htm files contain their images (no image folder)
# The .md files are maintained at Astrodienst outside git; do not edit them here.

all:	pdf htm

pdf:	swisseph.pdf swephprg.pdf

htm:	swisseph.htm swephprg.htm

swisseph.pdf:	swisseph.md mk_pdf.pl media/*
	perl mk_pdf.pl swisseph.md

swephprg.pdf:	swephprg.md mk_pdf.pl
	perl mk_pdf.pl swephprg.md

swisseph.htm:	swisseph.md mk_pdf.pl media/*
	perl mk_pdf.pl swisseph.md -htm

swephprg.htm:	swephprg.md mk_pdf.pl
	perl mk_pdf.pl swephprg.md -htm

.PHONY:	all pdf htm
