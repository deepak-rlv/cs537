all:clean
	gcc badger_fortune.c -Wall -Werror -O3 -o badger-fortune
clean:
	rm -f badger_fortune
debug:clean
	gcc badger_fortune.c -D DEBUG -Wall -Werror -g -o badger-fortune
