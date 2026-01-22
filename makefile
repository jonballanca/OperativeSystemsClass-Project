all: SO_Project

config.o: config.c config.h
	gcc -c config.c -Wall -Wextra
	
inventory.o: inventory.c inventory.h	
	gcc -c inventory.c -Wall -Wextra

commands.o: commands.c commands.h
	gcc -c commands.c -Wall -Wextra

protocols.o: protocols.c protocols.h
	gcc -c protocols.c -Wall -Wextra

communication.o: communication.c communication.h
	gcc -c communication.c -Wall -Wextra

printing.o: printing.c printing.h
	gcc -c printing.c -Wall -Wextra

maester.o: maester.c
	gcc -c maester.c -Wall -Wextra

SO_Project: config.o inventory.o commands.o protocols.o communication.o printing.o maester.o
	gcc config.o inventory.o commands.o protocols.o communication.o printing.o maester.o -o SO_Project.exe
	
clean:
	rm -f *.exe *.o