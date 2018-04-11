/*
 ============================================================================
 Name        : Lab4.c
 Author      : Benjarit Hotrabhavananda
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <unistd.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <semaphore.h>
#include "ece4220lab3.h"

typedef struct {
    struct timeval tsB;
    struct timeval tsA;
    struct timeval tsE;
    unsigned int gpsB;
    unsigned int gpsA;
    double gpsE;
} child_data;

typedef struct {
    unsigned char position;
    struct timeval time_stamp; // Elements: tv_sec, tv_usec
}coordinate;

/*
 * Shared memory and Global variables
 */
int N_pipe1, N_pipe2 ;
coordinate buffer;
sem_t sem;
int fd[2];


void read_coordinates();
void *read_NamedPipe_data(void *arg);
void *interpolate_two_GPS_data(void *arg);
void *print_simple_pipe_data(void *arg);

int main(void) {

	/*
	 * Create a Simple Pipe
	 */
	pipe(fd);

	/*
     * Initilaize the semaphore
    */
    sem_init(&sem,0,1);

	/*
     * Open the N_pipe1 for reciving GPS Data
    */
	if((N_pipe1  = open("/tmp/N_pipe1", O_RDONLY)) < 0)
	{
		printf("pipe N_pipe1 error\n");
		exit(-1);
	}
	puts("GPS named pipe open for reading..");
	/*
	* Open the N_pipe2 for recieving time-stamp of the real time event
	*/
	if((N_pipe2  = open("N_pipe2", O_RDONLY)) < 0)
	{
		printf("pipe N_pipe2 error\n");
		exit(-1);
	}
	puts("Event named pipe open for reading..\n");
	/*
	* Create ptheads
	*/
	pthread_t thread[2];
	if (pthread_create(&thread[0],NULL,read_NamedPipe_data,NULL) != 0) {
    	printf("Can't create read_NamedPipe_data pthread!!\n");
    }
	if (pthread_create(&thread[1],NULL,print_simple_pipe_data,NULL) != 0) {
	   	printf("Can't create print_named_pipe_data pthread!!\n");
	}
	read_coordinates();

	return 0;
}
/*
 * This function is to receive serial data and collecting time-stamps, and then
 * put those values into shared memory (buffers)
 */
void read_coordinates(){
	/*
     * Receiving GPS data and collecting a time-stamp
    */
    while (1)
    {
		read(N_pipe1, &(buffer.position), sizeof(buffer.position));
		gettimeofday(&(buffer.time_stamp) ,NULL);
		//printf("******Got the GPS DATA %u**** \n", buffer.position);	// Debug
	}
}
void *read_NamedPipe_data(void *arg){

	int i = 0;
	pthread_t child_threads[4];
	child_data events[4];
	struct timeval time_temp;
	/*
	 * Wait for a push button to be pressed, and then read the time-stamp from the FIFO
	*/
	while (1)
	{
		/*
		 * True when the N_pipe2 is ready
		 */
		if(read(N_pipe2, &(time_temp), sizeof(time_temp)))
		{
			if (i == 4)
			{
				i = 0;
			}
			events[i].gpsB = (unsigned int)buffer.position;
			events[i].tsB = buffer.time_stamp;
			events[i].tsE = time_temp;
			pthread_create(&child_threads[i] , NULL, interpolate_two_GPS_data, (void*)&events[i]);
			i++;
		}
	}
	pthread_exit(0);
}
void *interpolate_two_GPS_data(void *arg) {
	child_data *childData = (child_data*)arg;
    double time_interval_B_to_A, GPS_data_interval_B_to_A;
    double time_interval_B_to_E;
    double slope, y_intercept;
    //time helper
    int time_consuming = 0;

	/*
     * Wait for next GPS data
     * Fixed here:
     * Added usleep to make it wait for read_coodinate function to get the new value
     */

	while(buffer.time_stamp.tv_usec == childData->tsB.tv_usec){
		time_consuming++;
	    usleep(60);
	}
	//Reset the time helper
	time_consuming = 0;

    childData->gpsA = (unsigned int)buffer.position;
    childData->tsA = buffer.time_stamp;

	/*
     * Interpolate the two GPS's data
    */

    // x2-x1
	time_interval_B_to_A = ((1000000 * (double)childData->tsA.tv_sec) + (double)childData->tsA.tv_usec) - ((1000000 * (double)childData->tsB.tv_sec) + (double)childData->tsB.tv_usec);

	// y2-y1
    GPS_data_interval_B_to_A = ((double)childData->gpsA - (double)childData->gpsB);

    // x-x1
    time_interval_B_to_E = ((1000000 * (double)childData->tsE.tv_sec) + (double)childData->tsE.tv_usec) - ((1000000 * (double)childData->tsB.tv_sec) + (double)childData->tsB.tv_usec);

    // (y2-y1)/(x2-x1)
    slope = GPS_data_interval_B_to_A / time_interval_B_to_A;

    // y = y1 + (y2-y1)/(x2-x1) * (x-x1)
    childData->gpsE = (double)childData->gpsB + (slope * time_interval_B_to_E);

	/*
     * Write down data to the simple pipe
    */
    sem_wait(&sem);
    write(fd[1], &childData, sizeof(childData));

    pthread_exit(0);
}
/*
 * **************************** Extra Credit *********************************
 */
void *print_simple_pipe_data(void *arg) {
	child_data *result;

    while (1) {
        read(fd[0], &result, sizeof(result));
        /*
         * Print out each child pthread
         */
        unsigned long time_in_micros = 1000000 * (result->tsB.tv_sec) + (result->tsB.tv_usec);
        unsigned long time_in_micros2 = 1000000 * (result->tsA.tv_sec) + (result->tsA.tv_usec);
        unsigned long time_in_micros3 = 1000000 * (result->tsE.tv_sec) + (result->tsE.tv_usec);

        printf("\n\nData for Calculation:\n");
        printf("--------------------------------------------------\n");
        printf("Before events happened:\nGPS value: %-20d Microseconds: %lu.0   Seconds: %lu.0 \n",result->gpsB, (result->tsB.tv_usec),time_in_micros);
        printf("After events happened: \nGPS value: %-20d Microseconds: %lu.0   Seconds: %lu.0 \n",result->gpsA, (result->tsA.tv_usec),time_in_micros2);
        printf("--------------------------------------------------\n");
        printf("The real time event:\n");
        printf("Microseconds: %lu.0   Seconds: %lu.0 \n", (result->tsE.tv_usec),time_in_micros3);
        printf("The exact location of each snapshot: %f\n",result->gpsE);
        /*
         * Unlock semephore for other child threads
         */
        sem_post(&sem);
    }
    pthread_exit(0);
}

