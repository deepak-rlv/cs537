all:
	cd p1 && make
	cd p3 && make
	cd p5/xv6_p5_scratch && make
clean:
	cd p1 && make clean
	cd p3 && make clean
	cd p5/xv6_p5_scratch && make clean
run:
	cd p5/xv6_p5_scratch && ../tests/runtests -c
