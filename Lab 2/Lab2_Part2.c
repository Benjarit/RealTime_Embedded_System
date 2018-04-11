#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sched.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <time.h>
#include <pthread.h>

#define MY_PRIORITY 51  // kernel is priority 50
#define NUM_THREAD 3

// Original + 2
#define startR1 0.5+5.0
#define periodR1 2.0

#define startW 1.0+5.0
#define periodW 1.0

#define startR2 1.5+5.0
#define periodR2 2.0



#define numOfPeriod 0

int flag=1;

char buffer[50];

void *rd1(void *arg);
void *rd2(void *arg);
void *wr1(void *arg);
pthread_t thread[NUM_THREAD];


int main(void) {

	pthread_attr_t thread_attr;
	pthread_attr_init(&thread_attr);

	// Create the threads
	pthread_create(&thread[0], &thread_attr, rd1, NULL);
	pthread_create(&thread[1], &thread_attr, rd2, NULL);
	pthread_create(&thread[2], &thread_attr, wr1, NULL);

	// Join the threads
	pthread_join(thread[0], NULL);
	pthread_join(thread[1], NULL);
	pthread_join(thread[2], NULL);

	return 0;
}
void *rd1(void *arg){
	struct sched_param param;
	param.sched_priority = MY_PRIORITY;
	sched_setscheduler(thread[0], SCHED_FIFO, &param);

	/*----------------- Open file to read---------------- */
	FILE *fp1;
	fp1 = fopen("first.txt","r");
	if(fp1 == 0){
		printf("Error Reading file\n");
	}
	/*----------------------------------------------------*/
	
	int timer_fd = timerfd_create(CLOCK_REALTIME,0);

	struct itimerspec itval;
	itval.it_value.tv_sec = startR1;
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = periodR1;
	itval.it_interval.tv_nsec = 0;


	timerfd_settime(timer_fd, 0, &itval, NULL);
	uint64_t num_periods = numOfPeriod;

	while(fgets(buffer,50,fp1)){
//		printf("********************** Thread Read 1\n");
		read(timer_fd, &num_periods, sizeof(num_periods));//wait its period
	}

	fclose(fp1);
	pthread_exit(0);
}
void *rd2(void *arg){
	struct sched_param param;
	param.sched_priority = MY_PRIORITY;
	sched_setscheduler(thread[1], SCHED_FIFO, &param);

	// Open file to read
	FILE *fp2;
	fp2 = fopen("second.txt","r");
	if(fp2 == 0){
		printf("Error Reading file\n");
	}

	int timer_fd = timerfd_create(CLOCK_REALTIME,0);

	struct itimerspec itval;
	itval.it_value.tv_sec = startR2;
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = periodR2;
	itval.it_interval.tv_nsec = 0;


	timerfd_settime(timer_fd, 0, &itval, NULL);


	uint64_t num_periods = numOfPeriod;

	while(fgets(buffer,50,fp2)){
//		printf("********************** Thread Read 2\n");

		read(timer_fd, &num_periods, sizeof(num_periods));
	}

	flag = 0;
	fclose(fp2);
	pthread_exit(0);
}
void *wr1(void *arg){
	struct sched_param param;
	param.sched_priority = MY_PRIORITY;
	sched_setscheduler(thread[2], SCHED_FIFO, &param);

	// Open/Create file to write
	FILE *fp3;
	fp3 = fopen("output.txt","w");
	if(fp3 == 0){
		printf("Error Reading file\n");
	}

	int timer_fd = timerfd_create(CLOCK_REALTIME,0);


	struct itimerspec itval;

	itval.it_value.tv_sec = startW;
	itval.it_value.tv_nsec = 0;
	itval.it_interval.tv_sec = periodW;
	itval.it_interval.tv_nsec = 0;

	timerfd_settime(timer_fd, 0, &itval, NULL);


	uint64_t num_periods = numOfPeriod;

	while(flag){
//		printf("********************** Thread Write 3\n");
		printf("%s",buffer);
		fprintf(fp3,"%s",buffer);//write in the file

		read(timer_fd, &num_periods, sizeof(num_periods));
	}

	timerfd_settime(timer_fd, 0, &itval, NULL);
	fclose(fp3);
	pthread_exit(0);
}


