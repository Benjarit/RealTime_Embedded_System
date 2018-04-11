/*
 ============================================================================
 Name    	: Lab3_Part2.c
 Author  	: Benjarit Hotrabhavananda
 Version 	:
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
#include <semaphore.h>
#include "ece4220lab3.h"

const int red = 2;
const int yellow = 3;
const int green = 4;
const int button1 = 16;

void *red_task(void *arg);
void *yellow_task(void *arg);
void *pedestrain_task(void *arg);
pthread_t thread[3];
struct sched_param param;
sem_t task_signal;

int main(void) {
    puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */
    wiringPiSetupGpio();

    //--------------------------- Turning On/Off the LEDs ------------------------
    pinMode(red, OUTPUT);
    pinMode(yellow, OUTPUT);
    pinMode(green, OUTPUT);
    pinMode(button1, INPUT);

    digitalWrite(red,LOW);
    digitalWrite(yellow,LOW);
    digitalWrite(green,LOW);
    pullUpDnControl(button1, PUD_DOWN);

    /* ***************Initiate a binary semaphore******************** */
    sem_init(&task_signal,0,1);

    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);

	/* ****************Intitiate threads ******************************* */
    pthread_create(&thread[0], &thread_attr, red_task, NULL);
    pthread_create(&thread[1], &thread_attr, yellow_task, NULL);
    pthread_create(&thread[2], &thread_attr, pedestrain_task, NULL);

    pthread_join(thread[0], NULL);
    pthread_join(thread[1], NULL);
    pthread_join(thread[2], NULL);

    return 0;
}
void *red_task(void *arg){
	/* **************** set priority **************** */
    param.sched_priority = 51;
    sched_setscheduler(0, SCHED_FIFO, &param);

    while(1) {
   	 sem_wait(&task_signal); // Enter a critical region
	 
   	 digitalWrite (red, HIGH);
   	 sleep(1);
   	 digitalWrite (red, LOW);
	 
   	 sem_post(&task_signal);
   	 usleep(500);
    }
}
void *yellow_task(void *arg){
	/* **************** set priority **************** */
    param.sched_priority = 51;
    sched_setscheduler(0, SCHED_FIFO, &param);

    while(1) {
   	 sem_wait(&task_signal); // Enter a critical region
	 
   	 digitalWrite(yellow, HIGH);
   	 sleep(1);
   	 digitalWrite(yellow, LOW);
	 
   	 sem_post(&task_signal);
   	 usleep(500);
    }
}
void *pedestrain_task(void *arg){
	/* **************** set priority **************** */
    param.sched_priority = 51;
    sched_setscheduler(0, SCHED_FIFO, &param);

    while(1) {
   	 sem_wait(&task_signal); // Enter a critical region
	 
   	 if(check_button() == 1){
   		 digitalWrite (green, HIGH);
   		 sleep(1);
   		 digitalWrite (green, LOW);
   		 clear_button();
   	 }
	 
   	 sem_post(&task_signal);
   	 usleep(500);
    }
}