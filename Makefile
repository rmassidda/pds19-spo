# swarm particle optimization

RELEASE = spo-530766
# BIB = --bibliography fonti.bib

relazione: relazione.md relazione.yaml
	pandoc ${BIB} ${STYLE} -o $@.pdf $^

clean:
	rm -rf relazione.pdf $(RELEASE)

dist: relazione
	mkdir -p $(RELEASE)
	cp -R relazione.pdf *.cpp *.hpp $(RELEASE)
	tar -cf $(RELEASE).tar $(RELEASE)
	gzip $(RELEASE).tar
	rm -rf $(RELEASE)

.PHONY: all options clean dist
