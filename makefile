hoarding.out : game.o board.o
	gcc game.o board.o -o hoarding.out

game.o : game.c board.c board.h property.h
	gcc -g -Wall -Werror -c game.c

board.o : board.c board.h property.h
	gcc -g -Wall -Werror -c board.c

clean :
	rm *.o hoarding.out