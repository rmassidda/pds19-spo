# swarm particle optimization
CXX             = g++ -std=c++17
FF_ROOT					= ~/fastflow
INCLUDES        = -I $(FF_ROOT)
CXXFLAGS        = -Wall
LDFLAGS         = -pthread
OPTFLAGS        = -O3 -finline-functions -fopt-info-vec-all

SOURCE = mapreduce.hpp spo.hpp utimer.hpp queue.hpp main.cpp
RELEASE = spo-530766

relazione: relazione.md relazione.yaml
	pandoc ${BIB} ${STYLE} -o $@.pdf $^

spo: $(SOURCE)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $^ $(LDFLAGS)

dist: spo relazione
	mkdir -p $(RELEASE)
	cp $(SOURCE) relazione.pdf $(RELEASE)
	tar -cf $(RELEASE).tar $(RELEASE)
	gzip $(RELEASE).tar
	rm -rf $(RELEASE)

clean:
	rm -rf $(RELEASE) $(RELEASE).tar.gz spo

cleanall:
	rm -rf relazione.pdf $(RELEASE) $(RELEASE).tar.gz spo

.PHONY: dist clean cleanall
