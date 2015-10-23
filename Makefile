
MACHINE := mac

ifeq ($(MACHINE), mac)
	CC := clang
else
	CC := gcc
endif

# CC=clang
CFLAGS=-I.

ifishmake: repl.c repl_history.c
	$(CC) -o ifish repl.c repl_history.c -I -std=c99.
