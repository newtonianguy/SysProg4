CC = gcc
CFLAGS = -g -c
AR = ar -rc
RANLIB = ranlib




all: libnetfiles.c libnetfiles.h netfileserver.c
	gcc -g -Wall -Werror -fsanitize=address -o libnet libnetfiles.c -lm
	gcc -g -Wall -Werror -fsanitize=address -o netfile netfileserver.c -lm

server: netfileserver.c
	gcc -g -Wall -Werror -fsanitize=address -o netfile netfileserver.c

lib: libnetfiles.c libnetfiles.h
	gcc -g -Wall -Werror -fsanitize=address -o libnet libnetfiles.c libnetfiles.h

clean:
	rm -rf testfile *.o *.out *.gch libnet netfile
	
	
archive:
	tar czf Asst3.tgz Asst3 
	