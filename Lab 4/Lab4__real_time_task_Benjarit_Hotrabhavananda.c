/*
 ============================================================================
 Name        : Lab3_Part1.c
 Author      : Benjarit Hotrabhavananda
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>
#include "ece4220lab3.h"

#define MY_PRIORITY 51  // kernel is priority 50
const int button1 = 16;
struct timeval time_stamp_event;
pthread_t thread;
int N_pipe2;
void *rt_task(void *arg);

int main(void) {
	puts("!!!Hi Friends!!!"); /* prints !!!Hello World!!! */
	wiringPiSetupGpio();
	pinMode(button1, INPUT);
	pullUpDnControl(button1, PUD_DOWN);
	if((N_pipe2 = open("N_pipe2", O_WRONLY)) < 0)
	{
		printf("pipe N_pipe2 error\n");
		exit(-1);
	}
	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);
	pthread_create(&thread, &thread_attr, rt_task, NULL);
	pthread_join(thread, NULL);

	return 0;
}

void *rt_task(void *arg){
	struct sched_param param;
	param.sched_priority = MY_PRIORITY;
	sched_setscheduler(thread, SCHED_FIFO, &param);
	int timer_fd = timerfd_create(CLOCK_REALTIME,0);

	struct itimerspec itval;
	itval.it_value.tv_sec = 1;
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = 0;
	itval.it_interval.tv_nsec = 75000000;


	timerfd_settime(timer_fd, 0, &itval, NULL);
	uint64_t num_periods = 0;

	while(1){
		//puts("******Inside the loop*******");
		if(check_button() == 1){
			gettimeofday(&(time_stamp_event),NULL);
			if(write(N_pipe2, &time_stamp_event, sizeof(time_stamp_event)) != sizeof(time_stamp_event))
			{
				printf("N_pipe2 pipe write error\n");
				exit(-1);
			}
			printf("Time_stamp == %lu\n", time_stamp_event.tv_usec); // Debug
			clear_button();
		}
		read(timer_fd, &num_periods, sizeof(num_periods));//wait its period
	}
	pthread_exit(0);
}

