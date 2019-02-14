codes : thread

thread : thread_hare_turtle.cpp input.cpp
	g++ -std=c++11 -pthread -o thread thread_hare_turtle.cpp
	g++ -std=c++11 -o input input.cpp
clean:
	rm -f thread input
	echo cleaned

test : thread 
	 ${CURDIR}/input > in.txt 
	 ${CURDIR}/thread < in.txt 
