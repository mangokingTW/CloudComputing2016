#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

#define DISKIO 160*1024*1024

void print_usage(){
	printf("Usage: ./workload cpu|mem|io percent\n");
}

void increase_priority() {
	int priority = getpriority(PRIO_PROCESS, 0);
	while (setpriority(PRIO_PROCESS, 0, priority-1) == 0 && priority>-20) {
		priority--;	
	}
}

void *cpu_load_generator(){
	int i = 0;
	while(1){
		i+=1;
	}
}

void *cpu_load_worker(void *param){
	int i = 0;
	int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
	int *pid_set = calloc(cpu_num,sizeof(int));
	double percent = *(double *) param;
	struct timespec time_work,time_sleep;
	memset( &time_work, 0, sizeof(struct timespec));
	memset( &time_sleep, 0, sizeof(struct timespec));
	increase_priority();
	if( percent < 1 ) percent = 1;
	time_work.tv_nsec = 10000000 * percent;
	time_sleep.tv_nsec = 10000000 * (100 - percent);
	for( i = 0 ; i < cpu_num ; i++ ){
		if( (pid_set[i] = fork()) < 0 ){
			printf("Unable to create cpu workloads.\n");
			exit(-1);
		}
		else if ( pid_set[i] ){ //parent
			if (kill(pid_set[i],SIGSTOP)!=0) {
				printf("Unable to stop cpu workloads.\n");
				exit(-1);
			}
		}
		else{ //child
			printf("PID %d is running.\n",getpid());
			cpu_load_generator();
			exit(-1);
		}
	}
	while(1){
		for( i = 0 ; i < cpu_num ; i++ ){
			if (kill(pid_set[i],SIGCONT)!=0) {
				printf("Unable to stop cpu workloads.\n");
				exit(-1);
			}
		}
		nanosleep(&time_work,NULL);
		for( i = 0 ; i < cpu_num ; i++ ){
			if (kill(pid_set[i],SIGSTOP)!=0) {
				printf("Unable to stop cpu workloads.\n");
				exit(-1);
			}
		}
		nanosleep(&time_sleep,NULL);
	}
}

void *mem_load_worker(void *param){
	int i = 0;
	size_t page_size = sysconf(_SC_PAGESIZE);
	size_t page_num = sysconf(_SC_PHYS_PAGES);
	size_t alloc_num = page_num * *(double *)param / 100;
	char ** page_array = calloc(alloc_num,sizeof(char *));
	if( !page_array ){
		printf("Unable to initialize the memory workload.\n");
		exit(-1);
	}
	printf("total memory pages in system: %d\n",page_num);
	for( i = 0 ; i < alloc_num ; i++ ){
		posix_memalign((void **)&page_array[i], page_size, 1);
		page_array[i][0] = 1;
	}
	while(1){
		sleep(1);
	}
}

pthread_mutex_t mutex;

void *io_load_generator(void* param){
	int i = 0;
	int need_lock = 1;
	int percent = *(double *)param*0.8;
	int times = 1024/percent;
	char *data = malloc(1024*1024*percent);
	int fp = open("/tmp/workload_io",O_RDWR|O_CREAT,0644);
	memset( data, '1', 1024*1024*percent);
	if( *(double *)param > 99 ) need_lock = 0;
	while(1){
		lseek(fp, 0, SEEK_SET);
		for( i = 0 ; i < times ; i++ ){
			if( need_lock){
				pthread_mutex_lock(&mutex);
				pthread_mutex_unlock(&mutex);
			}
			write( fp, data, 1024*1024*percent);
			sync();
		}
	}
}

void *io_load_worker(void *param){
	int i = 0;
	pthread_t thread;
	double percent = *(double *) param;
	struct timespec time_work,time_sleep;
	memset( &time_work, 0, sizeof(struct timespec));
	memset( &time_sleep, 0, sizeof(struct timespec));
	pthread_mutex_init(&mutex, NULL);
	increase_priority();
	if( percent < 1 ) percent = 1;
	time_work.tv_nsec = 10000000 * percent;
	time_sleep.tv_nsec = 10000000 * (100 - percent);
	pthread_create(&thread, NULL, io_load_generator, &percent);
	while(1){
		nanosleep(&time_work,NULL);
		pthread_mutex_lock(&mutex);
		nanosleep(&time_sleep,NULL);
		pthread_mutex_unlock(&mutex);
	}
}

int main(int argc, char *argv[]){
	int i = 0;
	double cpu_load = 0;
	double mem_load = 0;
	double io_load = 0;
	pthread_t cpu_thread, io_thread;

	if( argc < 3 || argc % 2 != 1){
		print_usage();
		exit(-1);
	}
	for( i = 1 ; i < argc ; i+=2 ){
		if( !strcmp("cpu",argv[i]) ){
			sscanf(argv[i+1],"%lf",&cpu_load);
		}
		else if( !strcmp("mem",argv[i]) ){
			sscanf(argv[i+1],"%lf",&mem_load);
		}
		else if( !strcmp("io",argv[i]) ){
			sscanf(argv[i+1],"%lf",&io_load);
		}
		else{
			print_usage();
			exit(-1);
		}
	}
	printf("Settings: CPU %lf\% MEM %lf\% IO %lf\%\n",cpu_load,mem_load,io_load);

	if( cpu_load >= 1 )
		if( pthread_create(&cpu_thread, NULL, cpu_load_worker, (void *)&cpu_load) ){
			perror("pthread_create");	
		}
	if( io_load >= 1 )
		if( pthread_create(&io_thread, NULL, io_load_worker, (void *)&io_load) ){
			perror("pthread_create");	
		}	
	if( mem_load >= 1 )
		mem_load_worker((void *)&mem_load);
	if( cpu_load >= 1 )
		pthread_join(cpu_thread,NULL);
	if( io_load >= 1 )
		pthread_join(io_thread,NULL);
	return 0;
}
