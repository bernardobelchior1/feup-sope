/**
 * park.c
 *
 * Created by Bernardo Belchior and Edgar Passos
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "vehicle.h"
#include <errno.h>

#define NUM_CONTROLLERS 4

#define FIFO_PATH_LENGTH 8
#define FIFO_MODE 0666
#define MAX_FIFONAME_SIZE 31

#define SV_IDENTIFIER -1

void print_usage();
void *controller_func (void *arg);
void *worker_func (void *arg);

/**
 * main thread:
 * 
 * should initialize the needed global variables, 
 * create 4 controller threads and wait for their termination,
 * calculate and save global statistics
 *
 */
int main(int argc, char *argv[]){
	
	//Validating input
	if(argc != 3){
		print_usage();
		return 1;
	}
	
	int n_spaces, time_open, n_vacant;

	n_spaces = atoi(argv[1]);
	time_open = atoi(argv[2]);
	n_vacant = n_spaces;

	if(n_spaces < 0 || time_open < 0){
		print_usage();
		return 1;
	}

	if(mkfifo("fifoN",FIFO_MODE) != 0){
		printf("Failed making fifoN\n");
	}

	if(mkfifo("fifoE",FIFO_MODE) != 0){
		printf("Failed making fifoE\n");
	}

	if(mkfifo("fifoO",FIFO_MODE) != 0){
		printf("Failed making fifoO\n");
	}

	if(mkfifo("fifoS",FIFO_MODE) != 0){
		printf("Failed making fifoS\n");
	}
	//creating the 4 "controller" threads
	pthread_t controllers[4];

	int i;
	int thrargs[NUM_CONTROLLERS];

	for(i = 0; i < NUM_CONTROLLERS; i++){
		thrargs[i] = i;
		if(pthread_create(&controllers[i],NULL,controller_func, &thrargs[i]) != 0){ 
			printf("\tmain() :: error creating controller threads\n");
			return 3;
		}
	}

	int fd_N, fd_E, fd_O, fd_S;

	fd_N = open("fifoN",O_WRONLY);
	if(fd_N == -1){
		printf("Error opening fifoN\n");
	}

	fd_E = open("fifoE",O_WRONLY);
	if(fd_E == -1){
		printf("Error opening fifoE\n");
	}

	fd_O = open("fifoO",O_WRONLY);
	if(fd_O == -1){
		printf("Error opening fifoO\n");
	}

	fd_S = open("fifoS",O_WRONLY);
	if(fd_N == -1){
		printf("Error opening fifoS\n");
	}
	
	//wait for time and send SV
	sleep(time_open);


	//sending stop vehicle
	int sv = SV_IDENTIFIER;

	if(write(fd_N, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_E, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_O, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));

	if(write(fd_S, &sv, sizeof(int)) == -1)
		printf("\tmain() :: error writing sv - %s\n",strerror(errno));


	printf("\tJoining threads\n");
	for(i = 0; i < 4; i++){
		if(pthread_join(controllers[i],NULL) != 0){ 
			printf("\tmain() :: error joining controller threads\n");
		}
		printf("\tmain() :: thread %d joined\n",i);
	}
	

	printf("\tClosing fifos\n");
	close(fd_N);
	close(fd_E);
	close(fd_O);
	close(fd_S);

	printf("\tunlinking fifos\n");
	unlink("fifoN");
	unlink("fifoS");
	unlink("fifoE");
	unlink("fifoO");


	printf("Exiting main...\n");

	return 0;
}

/**
 * text to be printed when the program is used incorrectly
 * 
 */
void print_usage(){

	printf("Run as: ./parque <N_LUGARES> <TEMPO_ABERTURA>\nN_LUGARES and TEMPO_ABERTURA must be positive integers\n");
}

/**
 * controller thread:
 *
 * should create a FIFO,
 * handle its requests (entrance, time for parking, vehicle identifier, name of private FIFO for vehicle thread)
 * create an employee thread to handle the request and pass it its info
 * wait for closure of the park to attend any remaining requests and close the FIFO
 */
void *controller_func(void *arg){

	//TODO create valet threads and react to the closure of the park
	

	//Creating FIFO
	direction_t side = (*(int *) arg);
	char fifo_path[FIFO_PATH_LENGTH];

	switch (side){
		case NORTH:
			strcpy(fifo_path,"fifoN");
			/*if(mkfifo(fifo_path,FIFO_MODE) != 0){*/
				/*printf("\tcontroller_func() :: failed making FIFO %s\n",fifo_path);*/
				/*exit(4);*/
			/*}*/
			break;

		case WEST:
			strcpy(fifo_path,"fifoO");
			/*if(mkfifo(fifo_path,FIFO_MODE) != 0){*/
				/*printf("\tcontroller_func() :: failed making FIFO %s\n",fifo_path);*/
				/*exit(4);*/
			/*}*/
			break;

		case SOUTH:
			strcpy(fifo_path,"fifoS");
			/*if(mkfifo(fifo_path,FIFO_MODE) != 0){*/
				/*printf("\tcontroller_func() :: failed making FIFO %s\n",fifo_path);*/
				/*exit(4);*/
			/*}*/
			break;

		case EAST:
			strcpy(fifo_path,"fifoE");
			/*if(mkfifo(fifo_path,FIFO_MODE) != 0){*/
				/*printf("\tcontroller_func() :: failed making FIFO %s\n",fifo_path);*/
				/*exit(4);*/
			/*}*/
			break;

		default:
			break; //not expected
	}

	//TODO read requests
	
	//opening the fifo for reading
	int fifo_fd;
	fifo_fd = open(fifo_path,O_RDONLY);

	if(fifo_fd == -1){
		printf("\tError opening file\n");
	}

	//Read the request
	//Each vehicle the generator creates will have the following info
	// - vehicle identifier
	// - parking time (in clock ticks)
	// - entrance
	// - name of its private fifo
	
	int curr_entrance, curr_park_time, curr_id = 0;
	char buff[MAX_FIFONAME_SIZE];	

	int x;
	while(curr_id  != SV_IDENTIFIER) {
		x = read(fifo_fd,&curr_id,sizeof(int));

		if(curr_id == SV_IDENTIFIER){
			printf("Closing park..\n");
			break;
		}

		read(fifo_fd,&curr_park_time,sizeof(int));
		read(fifo_fd,&curr_entrance,sizeof(int));
		read(fifo_fd,buff,MAX_FIFONAME_SIZE);

		if(x > 0 && curr_id != SV_IDENTIFIER)
			printf("\n-----------\nThis is entrance %d\n\nid: %d\ntime: %d\nentrance: %d\nname: %s\n",
				side,curr_id, curr_park_time, curr_entrance,buff);
		printf("\tid - %d still here\n",curr_id);
	}

	printf("Closing park..\n");
	//park now closed, handle all remaining requests
/*	
	while(read(fifo_fd, &curr_id,sizeof(int)) > 0){
		printf("here\n");
	}
*/
	printf("Closing fifos\n");

	printf("\tExiting controller thread\n");
	pthread_exit(NULL);
}
