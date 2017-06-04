output: phil.o 
	gcc -pthread phil.o -o phil -lm
phil.o: phil.c
	gcc -c phil.c
clean:
	rm *.o phil


