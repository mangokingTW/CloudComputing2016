all:
	gcc pthread_setaffinity_np_test.c -lm -pthread -o pthread_setaffinity_np_test 
	cd proc_1 && $(MAKE) all
	cd proc_2 && $(MAKE) all
	cd proc_3 && $(MAKE) all

clean:
	rm -rf pthread_setaffinity_np_test
	cd proc_1 && $(MAKE) clean
	cd proc_2 && $(MAKE) clean
	cd proc_3 && $(MAKE) clean

run:
	./pthread_setaffinity_np_test
	cd proc_1 && $(MAKE) run
	cd proc_2 && $(MAKE) run
	cd proc_3 && $(MAKE) run
