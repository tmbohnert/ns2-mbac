all: emodel

emodel: main.o emodel.o
	g++ -g -lm main.o emodel.o -o emodel

main.o: main.cpp exception.h
	g++ -g -c main.cpp

emodel.o: emodel.cpp emodel.h
	g++ -g -c emodel.cpp

clean:
	rm -rf *o emodel
