# makefile used at Astrodienst to create pdf out of md file
htms:	swisseph.md swephprg.md
	# pandoc  --toc  --self-contained -f markdown -t html  swisseph.md > swisseph.htm
	pandoc --toc  --template template.htm  --self-contained -f markdown -t html  swisseph.md > swisseph.htm
	pandoc --toc  --template template.htm  --self-contained -f markdown -t html  swephprg.md > swephprg.htm
	cp -p sw*.htm /wwd/design

pdfs: 	swisseph.md swephprg.md
	pandoc --toc -V documentclass=report -V geometry:margin=25mm -f markdown -t pdf --pdf-engine=xelatex swisseph.md > swisseph.pdf
	pandoc --toc -V documentclass=report -V geometry:margin=25mm -f markdown -t pdf --pdf-engine=xelatex swephprg.md > swephprg.pdf
	cp -p sw*.pdf /wwd/design

pdfx: 	swisseph.md swephprg.md
	pandoc --toc -V documentclass=report -V geometry:margin=25mm -f markdown -t pdf --pdf-engine=lualatex  swisseph.md > swissephx.pdf
	cp -p swissephx.pdf /wwd/design

