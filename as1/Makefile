#makefile - for part1 and part2
.ONESHELL:
CC=g++

all:
	$(CC) pi-calc.cpp -o part1 -lboost_system -lboost_filesystem -lboost_iostreams -pthread -std=c++11
	$(CC) -pthread t-stat.cpp -o part2
	./part1 -o
	./part2 -o
	
pi:
	echo "What is the mode of compilation? Eg. '-s' for serial or '-p 2' for 2 processors or '-o' for ouput file"
	read mode
	#echo $$mode
	./part1 $$mode

t_stat:
	echo "What is the mode of compilation? Eg. '-s' for serial or '-p 2' for 2 processors or '-o' for ouput file"
	read mode
	#echo $$mode
	./part2 $$mode

clean:
	rm part1
	rm part2
