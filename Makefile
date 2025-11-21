all:
	gcc -w *.c 
	./a.out

clean:
	rm -f a.out