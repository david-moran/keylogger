CFLAGS=-g
CLIBS=-lX11

keylogger: keylogger.o
	$(CC) -g keylogger.c -o keylogger $(CLIBS)

.PHONY: clean
clean:
	$(RM) keylogger keylogger.o
