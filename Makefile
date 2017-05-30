#Authors: Carter Reynolds and Alazar Genene 2/26/17 
all: memory2

memory2: memory2.c 
	gcc memory2.c -o memory2
	
clean:
	rm memory2 disk.txt
