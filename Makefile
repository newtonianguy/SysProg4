CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib




all: libnetfiles.c libnetfiles.h netfileserver.c
	gcc -g -Wall -Werror -fsanitize=address -o libnet libnetfiles.c libnetfiles.h
	gcc -g -Wall -Werror -fsanitize=address -o netfile netfileserver.c

server: netfileserver.c
	gcc -g -Wall -Werror -fsanitize=address -o netfile netfileserver.c

lib: libnetfiles.c libnetfiles.h
	gcc -g -Wall -Werror -fsanitize=address -o libnet libnetfiles.c libnetfiles.h

clean:
	rm -rf testfile *.o *.out *.gch libnet netfile
	
	
archive:
	tar czf asst1.tar *.c *.h 
	