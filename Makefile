# swarm particle optimization
CXX             = g++ -std=c++17
FF_ROOT					= ~/fastflow
INCLUDES        = -I $(FF_ROOT)
CXXFLAGS        = -Wall
LDFLAGS         = -pthread
OPTFLAGS        = -O3 -finline-functions #-fopt-info-vec-all

SOURCE = mapreduce.hpp spo.hpp utimer.hpp queue.hpp main.cpp
RELEASE = spo-530766

STYLE= --highlight-style=monochrome

report: report.md report.yaml
	pandoc ${STYLE} -o $@.pdf $^

spo: $(SOURCE)
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(OPTFLAGS) -o $@ $^ $(LDFLAGS)

dist: report
	mkdir -p $(RELEASE)
	cp -r experiment $(SOURCE) report.pdf autotime.sh Makefile $(RELEASE)
	tar -cf $(RELEASE).tar $(RELEASE)
	gzip $(RELEASE).tar
	rm -rf $(RELEASE)

clean:
	rm -rf $(RELEASE) $(RELEASE).tar.gz spo

cleanall:
	rm -rf report.pdf $(RELEASE) $(RELEASE).tar.gz spo

.PHONY: dist clean cleanall
