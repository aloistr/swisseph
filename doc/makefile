# makefile used at Astrodienst to create pdf out of md file
htms:	swisseph.md
	# pandoc  --toc  --self-contained -f markdown -t html  swisseph.md > swisseph.htm
	pandoc --toc  --template template.htm  --self-contained -f markdown -t html  swisseph.md > swisseph.htm
	cp -p swisseph.htm /wwd/design

pdfs: 	swisseph.md
	pandoc --toc -V documentclass=report -V geometry:margin=25mm -f markdown -t pdf --pdf-engine=xelatex swisseph.md > swisseph.pdf
	cp -p swisseph.pdf /wwd/design

pdfx: 	swisseph.md
	pandoc --toc -V documentclass=report -V geometry:margin=25mm -f markdown -t pdf --pdf-engine=lualatex  swisseph.md > swissephx.pdf
	cp -p swissephx.pdf /wwd/design

