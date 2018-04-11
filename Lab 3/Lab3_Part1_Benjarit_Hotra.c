/*
 ============================================================================
 Name    	: Lab3_Part1.c
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
#include "ece4220lab3.h"

#define MY_PRIORITY 51  // kernel is priority 50

const int red = 2;
const int yellow = 3;
const int green = 4;
const int button1 = 16;

pthread_t thread;

void *rt_task(void *arg);

int main(void) {
    puts("!!!Hi Friends!!!"); /* prints !!!Hello World!!! */
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

//-------------------------- Creating Thread as a Real-time task scheduler -----------------
    pthread_attr_t thread_attr;
    pthread_attr_init(&thread_attr);
    pthread_create(&thread, &thread_attr, rt_task, NULL);

    pthread_join(thread, NULL);

    return 0;
}

// Aperiodic/ sporadic task
void *rt_task(void *arg){
	// set priorty to be more than 50 so this thread can be real-time task
    struct sched_param param;
    param.sched_priority = MY_PRIORITY;
    sched_setscheduler(thread, SCHED_FIFO, &param);

    while(1){
		// Turn on red Light for 1 second
		 digitalWrite (red, HIGH);
		 sleep(1);
		// turn off red light
		 digitalWrite (red,  LOW);
		// Turn on yellow Light for 1 second
		 digitalWrite (yellow, HIGH);
		 sleep(1);
		//turn off yellow light
		 digitalWrite (yellow, LOW);

		//polling check everytime
		//Check the status of the button
		 if(check_button()){
			 puts("Button got pushed");
			 digitalWrite (green, HIGH);
			 sleep(1);
			 digitalWrite (green,  LOW);
			 clear_button();
		 }
    }
}