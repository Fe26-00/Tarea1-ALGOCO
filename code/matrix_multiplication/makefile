CXX = g++
CXXFLAGS = -Wall -Wextra

ALG_DIR = algorithms

SOURCES = \
	matrix_multiplication.cpp \
	$(ALG_DIR)/naive.cpp \
	$(ALG_DIR)/strassen.cpp

all:
	$(CXX) $(SOURCES) -o matrix

run: all
	./matrix

clean:
	rm -f matrix