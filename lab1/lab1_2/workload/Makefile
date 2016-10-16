all:
	gcc workload.c -lpthread -Wformat=0 -o workload

run:
	./workload cpu 50 mem 20 io 50

clean:
	rm -rf workload
