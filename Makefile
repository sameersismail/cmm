.PHONY: test

all:
	make -C src

debug:
	make debug -C src

test:
	pytest -vv --rootdir=test
	cd test/data && rm *.out

clean:
	make clean -C src
