release:clean
	gcc psort.c -Wall -Werror -pthread -O -o psort
debug:clean
	gcc psort.c -Wall -pthread -D debug -g -o psort
bypass:clean
	gcc psort.c -Wall -pthread -g -o psort
run:
	/home/cs537-1/tests/P3/runtests -c
clean:
	rm -rf psort