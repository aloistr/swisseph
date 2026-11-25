# makefile to create the PDF documentation out of the .md files
# tools: perl, pandoc, weasyprint, fonts DejaVu Sans / DejaVu Sans Mono
# images of swisseph.md are in media/
# The .md files are maintained at Astrodienst outside git; do not edit them here.

pdf:	swisseph.pdf swephprg.pdf

swisseph.pdf:	swisseph.md mk_pdf.pl media/*
	perl mk_pdf.pl swisseph.md

swephprg.pdf:	swephprg.md mk_pdf.pl
	perl mk_pdf.pl swephprg.md

.PHONY:	pdf
