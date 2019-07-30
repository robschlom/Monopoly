monopoly.out : game.o board.o
	gcc game.o board.o -o monopoly.out

game.o : game.c board.c board.h property.h
	gcc -g -Wall -Werror -c game.c

board.o : board.c board.h property.h
	gcc -g -Wall -Werror -c board.c

clean :
	rm *.o monopoly.out
