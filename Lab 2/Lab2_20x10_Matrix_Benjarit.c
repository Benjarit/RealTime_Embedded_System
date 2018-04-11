/*
 ============================================================================
 Name        : lala.c
 Author      : Benjarit Hotrabhavananda
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

/*
 ============================================================================
 Name    	: Lab2.c
 Author  	: Benjarit Hotrabhavananda
 Version 	:
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

//------------20x10.txt----------//

#define NUM_THREADS1 20
#define NUM_THREADS2 10
#define NUM_THREADS3 200

/* data structure used by thread */
typedef struct {
	int tid;
	int starting_element;
	int ending_element;
} thread_data_t;

int *a;
int r1,c1;
int found = 0;
int number = 0;

void *findNumber(void *arg)
{
	int count;
	thread_data_t *numberToFind = (thread_data_t*) arg;
	for(count = numberToFind->starting_element; count < numberToFind->ending_element; count++){
	    if(number == a[count]){
	    	found++;
	    }
	}
    pthread_exit(0);
}
void *findNumberEachRow(void *arg)
{
	thread_data_t *numberToFind = (thread_data_t*) arg;
	int start = numberToFind->starting_element;
	int end = numberToFind->ending_element;
	int i;
	for(i = start; i <= end; i+=10){
		if(number == a[i]){
			found++;
		}
	}

    pthread_exit(0);
}

int main(int arg, char *argc[]) {
	setbuf(stdout, NULL);
	number = atoi(argc[1]);

    int count, i, ret;

    struct timeval tv;
    double elapsed;

    pthread_attr_t attr;
    pthread_attr_init(&attr);

    thread_data_t thread_data1[1];
    thread_data_t thread_data2[NUM_THREADS1];
    thread_data_t thread_data3[NUM_THREADS2];
    thread_data_t thread_data4[NUM_THREADS3];

    pthread_t oneThread;
    pthread_t thread1[NUM_THREADS1];
    pthread_t thread2[NUM_THREADS2];
    pthread_t thread3[NUM_THREADS3];

    printf("Hi World\n");
    FILE *fp;

    fp = fopen("20x10.txt","r");
    if(fp == NULL)
    {
   	 printf("Error Reading file\n");
   	 return 0;
    }

    // scan in number of rows and number of columns
    printf("Hey,it works\n");
    fscanf(fp, "%d %d",&r1,&c1);

    // allocate memory for matrix in our program
    a = (int*)malloc(sizeof(int)*(r1 *c1));
    for(count = 0; count < c1*r1; count++)
    {
    	fscanf(fp,"%d",&a[count]);
    }

    int chosen = 0;
	printf("1. One thread to search the entire matrix.\n"
			"2. One thread for searching each row of the matrix.\n"
			"3. One thread for searching each column of the matrix\n"
			"4. One thread for each element of the matrix.\n");
	scanf("%d", &chosen);

//---------- Data for one thread for entire matrix -------------//
	if( chosen == 1 ){
		thread_data1[0].starting_element = 0;
		thread_data1[0].ending_element = 200;
	}


//---------- Data for one thread for each row -------------//
	else if( chosen == 2 ){
		int temp = 0;
		for(i = 0; i < NUM_THREADS1; i ++){
			thread_data2[i].starting_element = temp;
			thread_data2[i].ending_element = temp+10;
			temp += 10;
		}
	}

//---------- Data for one thread for each column -------------//
	else if( chosen == 3 ){
		for(i = 0; i < NUM_THREADS2; i++){
			thread_data3[i].starting_element = i;
			thread_data3[i].ending_element = i+(c1*19);
		 }
	}
//----------  Data for one thread for each element -------------//
	else if( chosen == 4 ){
		for(i = 0; i < NUM_THREADS3; i++){
			thread_data4[i].starting_element = i;
			thread_data4[i].ending_element = i+1;
		}
	}

	// start the timer
	gettimeofday(&tv, 0);
	unsigned long time_in_micros = 1000000 * tv.tv_sec + tv.tv_usec;
//----------One thread for entire matrix ------------------------//
	if(chosen == 1){
		ret = pthread_create(&oneThread, &attr, findNumber,&thread_data1[0]);
		if (ret !=0){
			printf("Create thread failed! error: %d", ret);
		}
		pthread_join(oneThread, NULL);
	}
//---------- One thread for each row ---------------------------//
	else if( chosen == 2 ){
		for (i = 0; i<NUM_THREADS1; i++){
			ret = pthread_create(&thread1[i], &attr, findNumber, (void *)&thread_data2[i]);
			if (ret !=0){
				printf("Create thread failed! error: %d", ret);
			}
		}
		for (i = 0; i<NUM_THREADS1; i++){
			pthread_join(thread1[i], NULL);
		}
	}
//---------- One thread for each column ---------------------------//
	else if( chosen == 3 ){
		for (i = 0; i<NUM_THREADS2; i++){
			ret = pthread_create(&thread2[i], &attr, findNumberEachRow, (void *)&thread_data3[i]);

			if (ret !=0){
				printf("Create thread failed! error: %d", ret);
			}
		}
		for (i = 0; i<NUM_THREADS2; i++){
			pthread_join(thread2[i], NULL);
		}
	}
//---------- One thread for each element-----------------------//
	else if( chosen == 4 ){
		for (i = 0; i<NUM_THREADS3; i++){
			ret = pthread_create(&thread3[i], &attr, findNumber, (void *)&thread_data4[i]);
			if (ret !=0){
				printf("Create thread failed! error: %d", ret);
			}
		}
		for (i = 0; i<NUM_THREADS3; i++){
			pthread_join(thread3[i], NULL);
		}
	}

	// stop the timer
	gettimeofday(&tv,0);
	elapsed = time_in_micros - 1000000 * tv.tv_sec + tv.tv_usec;

    printf("Found \"%s\" %d times!\n",argc[1],found);
    printf("Time used: %.2f microseconds\n",elapsed);

    free(a);
    fclose(fp);
    return 0;
}