#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ncurses.h>
#include <time.h>
#include <sys/time.h>

size_t c_o[7] = {0};
size_t c[7] = {0};
size_t d_o[5][11] = {0};
WINDOW* win;

int readline( int fd, char *buffer, int maxlen){
	int c = 0;
	while( c < maxlen ){
		if( read( fd, &buffer[c], 1) <= 0 ){
			return 0;
		};
		if( buffer[c] == '\n' ) {
			buffer[c] = 0;
			break;
		}
		c++;
	}
	return c;
}

int display(){
	wrefresh(win);
	mvwprintw(win,0,0,"");
	return 0;
}

int cpu_info(){
	char buffer[256] = {0};
	double usage = 0;
	double idle = 0;
	int fd = 0;
	if( (fd = open("/proc/stat",O_RDONLY)) < 0 ){
		perror("Unable to read CPU status.");
	}
	readline( fd, buffer, 256);
	close(fd);
	sscanf( buffer, "cpu  %Lu %Lu %Lu %Lu %Lu %Lu %Lu", 
		&c[0],&c[1],&c[2],&c[3],&c[4],&c[5],&c[6]);
	usage = c[0]+c[1]+c[2]+c[3]+c[4]+c[5]+c[6]-
		(c_o[0]+c_o[1]+c_o[2]+c_o[3]+c_o[4]+c_o[5]+c_o[6]);
	idle = c[3] - c_o[3];
	wprintw(win,"CPU usage: %lf\%\n",(double)(100-idle/usage*100));
	memcpy( c_o, c, sizeof(size_t)*7);
	return 0;
}

int mem_info(){
	char buffer[256] = {0};
	long long int mem_total = 0, mem_available = 0;
	int fd = 0;
	if( (fd = open("/proc/meminfo",O_RDONLY)) < 0 ){
		perror("Unable to read memory status.");
	}
	readline( fd, buffer, 256);
	sscanf( buffer, "MemTotal:        %llu kB", &mem_total);
	readline( fd, buffer, 256);
	readline( fd, buffer, 256);
	sscanf( buffer, "MemAvailable:    %llu kB", &mem_available);
	close(fd);
	wprintw(win,"Memory usage: %lf\%\n",
		(double)(100-(double)mem_available/(double)mem_total*100));
	return 0;
	
}

int io_info(){
	char buffer[256] = {0};
	char device[32] = {0};
	int m = 0, n =0, i =0;
	size_t d[11] = {0};
	size_t t[11] = {0};
	double usage = 0;
	double idle = 0;
	int fd = 0;
	if( (fd = open("/proc/diskstats",O_RDONLY)) < 0 ){
		perror("Unable to read disk status.");
	}

	while( readline( fd, buffer, 256) > 0 ){
		sscanf( buffer, "%d %d %32s %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu %Lu", 
		&m, &n, device, 
		&t[0],&t[1],&t[2],&t[3],&t[4],&t[5],&t[6],&t[7],&t[8],&t[9],&t[10]);
		if( !strcmp(device, "sda") )
			for( i = 0; i < 11; i++)
				d[i] = t[i];
	}
	close(fd);
	if( 
	d_o[0][6] == 0 &&
	d_o[1][6] == 0 &&
	d_o[2][6] == 0 &&
	d_o[3][6] == 0 &&
	d_o[4][6] == 0
	){
		for( i = 0 ; i < 5 ; i++ )
			memcpy( d_o[i], d, sizeof(size_t)*11);
		
	}
	wprintw(win,"Disk write speed: %luMB\n",(d[6]-d_o[4][6])*512/4/1024/1024);
	wprintw(win,"Disk read speed: %luMB\n",(d[2]-d_o[4][2])*512/4/1024/1024);
	memcpy( d_o[1], d_o[0], 4*sizeof(size_t)*11);
	memcpy( d_o[0], d, sizeof(size_t)*11);
	return 0;
}

int main(){
	initscr();
	cbreak();
	keypad(stdscr, TRUE);
	win = newwin(100,100,0,0);
	while(1){
		display();
		cpu_info();
		mem_info();
		io_info();
		sleep(1);
	}
}
