/* Short test program to test the pthread_setaffinity_np
* (which sets the affinity of threads to processors).
* Compile: gcc pthread_setaffinity_np_test.c
*              -o pthread_setaffinity_np_test -lm -lpthread
* Usage: ./pthread_setaffinity_test
*
* Open a "top"-window at the same time and see all the work
* being done on CPU 0 first and after a short wait on CPU 1.
* Repeat with different numbers to make sure, it is not a
* coincidence.
*/
#define _GNU_SOURCE
 
#include <stdio.h>
#include <unistd.h>
#include <math.h>
#include <pthread.h>

cpu_set_t cpuset,cpuget;

typedef struct {
    int index;
    double result;
} thread_param;

double waste_time(long n, size_t index, size_t total)
{
    double res = 0;
    long i = n * 200000000 * index / total;
    long limit = n * 200000000 * (index+1) / total;
    while (i < limit) {
        i ++;
        res += sqrt(i);
    }
    return res;
}
 
void *thread_func(void *param)
{   
    int i = ((thread_param *)param)->index;
    int cpu_num = sysconf(_SC_NPROCESSORS_ONLN);
    double res = 0;
    CPU_ZERO(&cpuset);
    CPU_SET(i, &cpuset); /* cpu 0 is in cpuset now */

    /* bind process to processor 0 */
    if (pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset) !=0) {
        perror("pthread_setaffinity_np");
    }  


    printf("Thread %d is running on core %d!\n",i,i);
    /* waste some time so the work is visible with "top" */
    res = waste_time( 5, i, 2);
    ((thread_param *)param)->result = res;
    pthread_exit(NULL);

}
 
int main(int argc, char *argv[])
{ 
    pthread_t thread[2];
    thread_param param[2];
    int i = 0;
    double result = 0;
    time_t startwtime, endwtime;
    startwtime = time (NULL); 
    for( i = 0 ; i < 2 ; i++ ){
        param[i].index = i;
        param[i].result = 0;
        if (pthread_create(&thread[i], NULL, thread_func,
            (void *)&param[i]) != 0) {
            perror("pthread_create");
        }
    }
    for( i = 0 ; i < 2 ; i++ ){
    	pthread_join(thread[i],NULL);
        result += param[i].result;
    }
    printf("result: %f\n", result);
    endwtime = time (NULL);
    printf ("wall clock time = %ld\n", (endwtime - startwtime));
    return 0;
}


