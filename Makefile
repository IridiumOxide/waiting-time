TARGET: ileczekam czekamnaudp

CC	= cc
CFLAGS	= -Wall -O2 -std=gnu11
LFLAGS	= -Wall -std=gnu11


ileczekam: ileczekam.o
	$(CC) $(LFLAGS) $^ -o $@

czekamnaudp: czekamnaudp.o
	$(CC) $(LFLAGS) $^ -o $@

.PHONY: clean TARGET
clean:
	rm -f ileczekam czekamnaudp *.o *~ *.bak
