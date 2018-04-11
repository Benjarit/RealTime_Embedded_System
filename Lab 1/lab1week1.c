/*
 ============================================================================
 Name      : Lab1.c
 Author      : Ben
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ===========HIGH=================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <wiringPi.h>


const int red = 2;
const int yellow = 3;

const int button1 = 16;
const int button2 = 17;
const int button3 = 18;
const int button4 = 19;
const int button5 = 20;

const int spk = 6;

int main(void) {
	wiringPiSetupGpio();



//--------------------------- Turning On/Off the LEDs ------------------------

    // configure the pin mode
	pinMode(red, OUTPUT);
	pinMode(yellow, OUTPUT);
	pinMode(spk, OUTPUT);
	pinMode(button1, INPUT);
	pinMode(button2, INPUT);
	pinMode(button3, INPUT);
	pinMode(button4, INPUT);
	pinMode(button5, INPUT);
    
    int choose 0;
    printf("Please enter  number 1(task 1) or 2(task 2))
    scanf("%d", &choose);
    if(choose == 1){
    	for (;;)
    	{
    	   digitalWrite (red, HIGH) ; delay(500) ;
    	   digitalWrite (red,  LOW) ; delay(500) ;
    	   digitalWrite (yellow, HIGH) ; delay(500) ;
    	   digitalWrite (yellow,  LOW) ; delay(500) ;
    	}
    }else{
// --------------------- Producing a sound on the speaker----------------
    	int num,y,soundPlayed = 0;
    	printf("Enter Button number 1-5: ");
    	scanf("%d", &num);
    
    
    	switch(num){
    	case 1:
    		pullUpDnControl(button1, PUD_DOWN);
    		while(soundPlayed == 0){
    			if(digitalRead(button1)){
    				digitalWrite (red, HIGH);
    				for(y = 0; y < 20; y++){
    					digitalWrite(spk, HIGH); delay(200);
    					digitalWrite(spk, LOW); delay(200);
    				}
    				digitalWrite (red,  LOW) ;
    				soundPlayed = 1;
    			}
    		}
    		break;
    	case 2:
    		pullUpDnControl(button2, PUD_DOWN);
    		while(soundPlayed == 0){
    			if(digitalRead(button2)){
    				digitalWrite (red, HIGH);
    				for(y = 0; y < 20; y++){
    					digitalWrite(spk, HIGH); delay(200);
    					digitalWrite(spk, LOW); delay(200);
    				}
    				digitalWrite (red,  LOW) ;
    				soundPlayed = 1;
    			}
    		}
    		break;
    	case 3:
    		pullUpDnControl(button3, PUD_DOWN);
    		while(soundPlayed == 0){
    			if(digitalRead(button3)){
    				digitalWrite (red, HIGH);
    				for(y = 0; y < 20; y++){
    					digitalWrite(spk, HIGH); delay(200);
    					digitalWrite(spk, LOW); delay(200);
    				}
    				digitalWrite (red,  LOW) ;
    				soundPlayed = 1;
    			}
    		}
    		break;
    	case 4:
    		pullUpDnControl(button4, PUD_DOWN);
    		while(soundPlayed == 0){
    			if(digitalRead(button4)){
    				digitalWrite (red, HIGH);
    				for(y = 0; y < 20; y++){
    					digitalWrite(spk, HIGH); delay(200);
    					digitalWrite(spk, LOW); delay(200);
    				}
    				digitalWrite (red,  LOW) ;
    				soundPlayed = 1;
    			}
    		}
    		break;
    	case 5:
    		pullUpDnControl(button5, PUD_DOWN);
    		while(soundPlayed == 0){
    			if(digitalRead(button5)){
    				digitalWrite (red, HIGH);
    				for(y = 0; y < 20; y++){
    					digitalWrite(spk, HIGH); delay(200);
    					digitalWrite(spk, LOW); delay(200);
    				}
    				digitalWrite (red,  LOW) ;
    				soundPlayed = 1;
    			}
    		}
    		break;
    	default:
    		printf("Invalid number!");
    		break;
    	}
	}
	return 0;
}

