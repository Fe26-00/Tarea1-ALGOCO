CXX = g++
CXXFLAGS = -Wall -Wextra

ALG_DIR = algorithms

SOURCES = \
	sorting.cpp \
	$(ALG_DIR)/mergesort.cpp \
	$(ALG_DIR)/quicksort.cpp \
	$(ALG_DIR)/patiencesort.cpp \
	$(ALG_DIR)/sort.cpp

all:
	$(CXX) $(SOURCES) -o sorting

run: all
	./sorting

clean:
	rm -f sorting