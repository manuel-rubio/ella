all: pthread  thread_join  thread_mutex

pthread: pthread.c
	gcc -pthread -o pthread pthread.c

thread_join: thread_join.c
	gcc -pthread -o thread_join thread_join.c

thread_mutex: thread_mutex.c
	gcc -pthread -o thread_mutex thread_mutex.c

clean:
	rm -f pthread thread_join thread_mutex
