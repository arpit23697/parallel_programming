codes : pthread_overhead histogram barrier pi_problem link_list

pthread_overhead : pthread_overhead.c pthread_overhead.cpp
	gcc -pthread -o pthread_overhead pthread_overhead.c
	g++ -std=c++11 -pthread -o pthread_overheadc+ pthread_overhead.cpp
histogram
 : histogram_mutex.c 
	gcc -pthread -o histogram histogram_mutex.c

barrier : barrier.c
fda	gcc -pthread -o barrier barrier.c

pi_problem : pi_program.c 
gcc -pthread -o pi_program pi_program.c

link_list : link_list_mutexOne.c link_list_mutexPerNode.c link_list_rwLock.c input_link_list.c
	gcc -pthread -o link_list_mutexOne link_list_mutexOne.c
	gcc -pthread -o link_list_mutexPerNode link_list_mutexPerNode.c
	gcc -pthread -o link_list_rwlock link_list_rwLock.c
	gcc -o input input_link_list.c

clean: 
	rm -f pthread_overhead plt.txt a.out pthread_overheadc+ histogram barrier pi_program in.txt link_list_mutexOne link_list_mutexPerNode link_list_rwlock input
	echo cleaned

test_overhead : pthread_overhead
	echo testing overhead
	${CURDIR}/pthread_overhead 
	${CURDIR}/pthread_overheadc+
	python plot.py
	echo overhead tested

test_histogram : histogram
	echo testing histogram
	${CURDIR}/histogram 1000 10
	python plot.py
	echo second test
	${CURDIR}/histogram 1000000 4
	python plot.py
	echo histogram tested

test_barrier : barrier
	echo testing barrier
	${CURDIR}/barrier 50
	python plot.py
	echo barrier tested

test_pi : pi_problem
	echo testing pi
	${CURDIR}/pi_program 4 10000
	python plot.py
	echo pi tested

test_link_list : link_list
	echo testing link_list 
	${CURDIR}/input 1000000 50 30 20 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
	echo test1_done
	${CURDIR}/input 1000000 90 5 5 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
codes : pthread_overhead histogram barrier pi_problem link_list

pthread_overhead : pthread_overhead.c pthread_overhead.cpp
	gcc -pthread -o pthread_overhead pthread_overhead.c
	g++ -std=c++11 -pthread -o pthread_overheadc+ pthread_overhead.cpp

histogram : histogram_mutex.c 
	gcc -pthread -o histogram histogram_mutex.c

barrier : barrier.c
	gcc -pthread -o barrier barrier.c

pi_problem : pi_program.c 
	gcc -pthread -o pi_program pi_program.c

link_list : link_list_mutexOne.c link_list_mutexPerNode.c link_list_rwLock.c input_link_list.c
	gcc -pthread -o link_list_mutexOne link_list_mutexOne.c
	gcc -pthread -o link_list_mutexPerNode link_list_mutexPerNode.c
	gcc -pthread -o link_list_rwlock link_list_rwLock.c
	gcc -o input input_link_list.c

clean: 
	rm -f pthread_overhead plt.txt a.out pthread_overheadc+ histogram barrier pi_program in.txt link_list_mutexOne link_list_mutexPerNode link_list_rwlock input
	echo cleaned

test_overhead : pthread_overhead
	echo testing overhead
	${CURDIR}/pthread_overhead 
	${CURDIR}/pthread_overheadc+
	python plot.py
	echo overhead tested

test_histogram : histogram
	echo testing histogram
	${CURDIR}/histogram 1000 10
	python plot.py
	echo second test
	${CURDIR}/histogram 1000000 4
	python plot.py
	echo histogram tested

test_barrier : barrier
	echo testing barrier
	${CURDIR}/barrier 50
	python plot.py
	echo barrier tested

test_pi : pi_problem
	echo testing pi
	${CURDIR}/pi_program 4 10000
	python plot.py
	echo pi tested

test_link_list : link_list
	echo testing link_list 
	${CURDIR}/input 1000000 50 30 20 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
	echo test1_done
	${CURDIR}/input 1000000 90 5 5 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
codes : pthread_overhead histogram barrier pi_problem link_list

pthread_overhead : pthread_overhead.c pthread_overhead.cpp
	gcc -pthread -o pthread_overhead pthread_overhead.c
	g++ -std=c++11 -pthread -o pthread_overheadc+ pthread_overhead.cpp

histogram : histogram_mutex.c 
	gcc -pthread -o histogram histogram_mutex.c

barrier : barrier.c
	gcc -pthread -o barrier barrier.c

pi_problem : pi_program.c 
	gcc -pthread -o pi_program pi_program.c

link_list : link_list_mutexOne.c link_list_mutexPerNode.c link_list_rwLock.c input_link_list.c
	gcc -pthread -o link_list_mutexOne link_list_mutexOne.c
	gcc -pthread -o link_list_mutexPerNode link_list_mutexPerNode.c
	gcc -pthread -o link_list_rwlock link_list_rwLock.c
	gcc -o input input_link_list.c

clean: 
	rm -f pthread_overhead plt.txt a.out pthread_overheadc+ histogram barrier pi_program in.txt link_list_mutexOne link_list_mutexPerNode link_list_rwlock input
	echo cleaned

test_overhead : pthread_overhead
	echo testing overhead
	${CURDIR}/pthread_overhead 
	${CURDIR}/pthread_overheadc+
	python plot.py
	echo overhead tested

test_histogram : histogram
	echo testing histogram
	${CURDIR}/histogram 1000 10
	python plot.py
	echo second test
	${CURDIR}/histogram 1000000 4
	python plot.py
	echo histogram tested

test_barrier : barrier
	echo testing barrier
	${CURDIR}/barrier 50
	python plot.py
	echo barrier tested

test_pi : pi_problem
	echo testing pi
	${CURDIR}/pi_program 4 10000
	python plot.py
	echo pi tested

test_link_list : link_list
	echo testing link_list 
	${CURDIR}/input 1000000 50 30 20 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
	echo test1_done
	${CURDIR}/input 1000000 90 5 5 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
codes : pthread_overhead histogram barrier pi_problem link_list

pthread_overhead : pthread_overhead.c pthread_overhead.cpp
	gcc -pthread -o pthread_overhead pthread_overhead.c
	g++ -std=c++11 -pthread -o pthread_overheadc+ pthread_overhead.cpp

histogram : histogram_mutex.c 
	gcc -pthread -o histogram histogram_mutex.c

barrier : barrier.c
	gcc -pthread -o barrier barrier.c

pi_problem : pi_program.c 
	gcc -pthread -o pi_program pi_program.c

link_list : link_list_mutexOne.c link_list_mutexPerNode.c link_list_rwLock.c input_link_list.c
	gcc -pthread -o link_list_mutexOne link_list_mutexOne.c
	gcc -pthread -o link_list_mutexPerNode link_list_mutexPerNode.c
	gcc -pthread -o link_list_rwlock link_list_rwLock.c
	gcc -o input input_link_list.c

clean: 
	rm -f pthread_overhead plt.txt a.out pthread_overheadc+ histogram barrier pi_program in.txt link_list_mutexOne link_list_mutexPerNode link_list_rwlock input
	echo cleaned

test_overhead : pthread_overhead
	echo testing overhead
	${CURDIR}/pthread_overhead 
	${CURDIR}/pthread_overheadc+
	python plot.py
	echo overhead tested

test_histogram : histogram
	echo testing histogram
	${CURDIR}/histogram 1000 10
	python plot.py
	echo second test
	${CURDIR}/histogram 1000000 4
	python plot.py
	echo histogram tested

test_barrier : barrier
	echo testing barrier
	${CURDIR}/barrier 50
	python plot.py
	echo barrier tested

test_pi : pi_problem
	echo testing pi
	${CURDIR}/pi_program 4 10000
	python plot.py
	echo pi tested

test_link_list : link_list
	echo testing link_list 
	${CURDIR}/input 1000000 50 30 20 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
	echo test1_done
	${CURDIR}/input 1000000 90 5 5 > in.txt
	${CURDIR}/link_list_mutexOne 6 < in.txt
	${CURDIR}/link_list_mutexPerNode 6 < in.txt
	${CURDIR}/link_list_rwlock 6 < in.txt
	python plot.py
