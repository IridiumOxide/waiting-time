TARGET: ileczekam czekamnaudp

CC	= cc
CFLAGS	= -Wall -O2 -std=c11
LFLAGS	= -Wall -std=c11


ileczekam: ileczekam.o
	$(CC) $(LFLAGS) $^ -o $@

czekamnaudp: czekamnaudp.o
	$(CC) $(LFLAGS) $^ -o $@

.PHONY: clean TARGET
clean:
	rm -f ileczekam czekamnaudp *.o *~ *.bak
