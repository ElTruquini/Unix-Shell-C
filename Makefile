.phony all:
all: a1 inf args

a1: a1.c
	gcc a1.c -lreadline -lhistory -ltermcap -o a1

inf: inf.c
	gcc inf.c -o inf

args: args.c
	gcc args.c -o args

.PHONY clean:
clean:
	-rm -rf *.o *.exe
