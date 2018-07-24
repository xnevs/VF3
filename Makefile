all:
	$(CXX) -std=c++11 -Wall -Wno-deprecated -O3 -o vf3 src/main.cpp -Iinclude
	mv vf3 ./bin
#all:
#	$(CXX) -std=c++11 -Wall -Wno-deprecated -g -o vf3 src/main.cpp -Iinclude
#	mv vf3 ./bin
	
.PHONY: doc
doc:
	doxygen ./doc/doxygen.conf

clean:
	rm ./bin/*
	rm -rf ./doc/html
	rm -rf ./doc/latex
