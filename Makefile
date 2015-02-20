all:
	gcc readwrite.c -o myfsck

gdb:
	gcc -g readwrite.c -o myfsck

clean:
	rm ./*.o	
