# Target library
.PHONY: all clean

all: clean libuthread.a

libuthread.a: queue.o uthread.o context.o preempt.o
	ar rcs libuthread.a queue.o uthread.o context.o preempt.o

queue.o: queue.c queue.h
	cc -c queue.c

uthread.o: uthread.c uthread.h context.h preempt.h queue.h
	cc -c uthread.c

context.o: context.c context.h uthread.h preempt.h 
	cc -c context.c
	
preempt.o: preempt.c preempt.h uthread.h
	cc -c preempt.c

clean:
	rm -f *.o *.a

## TODO: Phase 1.1
