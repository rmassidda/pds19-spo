# swarm particle optimization
CXX             = g++ -std=c++17
FF_ROOT					= ~/fastflow
INCLUDES        = -I $(FF_ROOT)
CXXFLAGS        = -Wall
LDFLAGS         = -pthread
OPTFLAGS        = -O3 -finline-functions

RELEASE = spo-530766
# BIB = --bibliography fonti.bib

relazione: relazione.md relazione.yaml
	pandoc ${BIB} ${STYLE} -o $@.pdf $^

spo: spo.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $< $(LDFLAGS)

clean:
	rm -rf relazione.pdf $(RELEASE) $(RELEASE).tar.gz spo

dist: relazione
	mkdir -p $(RELEASE)
	cp -r relazione.pdf $(RELEASE)
	tar -cf $(RELEASE).tar $(RELEASE)
	gzip $(RELEASE).tar
	rm -rf $(RELEASE)

.PHONY: all options clean dist
