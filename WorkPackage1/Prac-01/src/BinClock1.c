/*
 * BinClock.c
 * Jarrod Olivier
 * Modified by Keegan Crankshaw
 * Further Modified By: Mark Njoroge 
 *
 * version og
 * <STUDNUM_1> <STUDNUM_2>
 * Date
*/

#include <signal.h> //for catching signals
#include <wiringPi.h>
#include <wiringPiI2C.h>
#include <stdio.h> //For printf functions
#include <stdlib.h> // For system functions

#include "BinClock.h"
#include "CurrentTime.h"

//Global variables
int hours, mins, secs;
long lastInterruptTime = 0; //Used for button debounce
int RTC; //Holds the RTC instance

int HH,MM,SS;
int PUP_UP=1;

// Clean up function to avoid damaging used pins
void CleanUp(int sig){
	printf("Cleaning up\n");

	//Set LED to low then input mode
	//Logic here
	pinMode(LED,INPUT);
	//TURN OFF  LED
	digitalWrite(LED,LOW);

	for (int j=0; j < sizeof(BTNS)/sizeof(BTNS[0]); j++) {
		pinMode(BTNS[j],INPUT);
	}

 	exit(0);

}

void initGPIO(void){
	/* 
	 * Sets GPIO using wiringPi pins. see pinout.xyz for specific wiringPi pins
	 * You can also use "gpio readall" in the command line to get the pins
	 * Note: wiringPi does not use GPIO or board pin numbers (unless specifically set to that mode)
	 */
	printf("Setting up\n");
	wiringPiSetup(); //This is the default mode. If you want to change pinouts, be aware
	

	RTC = wiringPiI2CSetup(RTCAddr); //Set up the RTC
	
	//Set up the LED TO OUTPUT
	//Write your Logic here
        pinMode(LED,OUTPUT);
	
	printf("LED and RTC done\n");
	
	//Set up the Buttons
	for(int j=0; j < sizeof(BTNS)/sizeof(BTNS[0]); j++){
		pinMode(BTNS[j], INPUT);
		pullUpDnControl(BTNS[j], PUD_UP);
	//	digitalWrite(BTNS[j],LOW);
	}
	
       
	//Attach interrupts to Buttons
	//Write your logic here
	
	//Button1
	//pinMode(BTNS[0], INPUT);
	//pullUpControl(BTNS[0],PUP_UP);
	wiringPiISR(BTNS[0],INT_EDGE_BOTH,hourInc);

        //button2
	//pinMode(BTNS[1],INPUT);
	//pullUpControl(BTNS[1],PUP_UP);
	wiringPiISR(BTNS[1],INT_EDGE_BOTH,minInc);

	printf("BTNS done\n");
	printf("Setup done\n");
}

/*
 * The main function
 * This function is called, and calls all relevant functions we've written
 */
int main(void){
	signal(SIGINT,CleanUp);
	initGPIO();

	//Set random time (3:04PM)
	//You can comment this file out later
	wiringPiI2CWriteReg8(RTC, HOUR_REGISTER, 0x13+TIMEZONE);
	wiringPiI2CWriteReg8(RTC, MIN_REGISTER, 0x4);
	wiringPiI2CWriteReg8(RTC, SEC_REGISTER, 0x00);
	int flicker=0;
	// Repeat this until we shut down
	for (;;){
		//Fetch the time from the RTC
		//Write your logic here
		//Time stored in RTC
		hours=hexCompensation(wiringPiI2CReadReg8(RTC,HOUR_REGISTER));
		mins=hexCompensation(wiringPiI2CReadReg8(RTC,MIN_REGISTER));
		secs =hexCompensation(wiringPiI2CReadReg8(RTC,SEC_REGISTER));
		
		//Print out time we have stored on our RTC
		printf("The current time is: %d:%d:%d\n", hours, mins, secs);		
		// Flicker LED
		flicker=~flicker;
		digitalWrite(LED,flicker);
		printf("Written to the LED\n");
		//Toggle Seconds LED
		//Write your logic here
		toggleTime();
		
		// initialize buttons
		printf("Button0 value is %d\n", digitalRead(BTNS[0]));
		printf("Button1 value is %d\n", digitalRead(BTNS[1]));
		// if button0 is pressed increment hours
		if(digitalRead(BTNS[0])==0){
			//printf("in the first condition\n");
			hourInc();
			 printf("Button0 has been pressed!\n");
		}
		// if button1 is pressed increment minutes
		if(digitalRead(BTNS[1])==0){
			//printf("in the second condition\n");
			minInc();
			printf("Button1 has been pressed!\n");
		}

		//using a delay to make our program "less CPU hungry"
		delay(1000); //milliseconds
		// if the seconds are greater than 59 increment the minutes 
		// then set seconds to 0  
		if(secs<59){
			secs++;
		}else{
			MM=MM+1;
			secs=0;
		}
		// update seconds at RTC
		wiringPiI2CWriteReg8(RTC,SEC_REGISTER,secs);
	}
	return 0;
}

/*
 * Changes the hour format to 12 hours
 */
int hFormat(int hours){
	/*formats to 12h*/
	if (hours >= 24){
		hours = 0;
	}
	else if (hours > 12){
		hours -= 12;
	}
//	printf("12 HOUR FORMAT:%d\n",hours);
	return (int)hours;
}


/*
 * hexCompensation
 * This function may not be necessary if you use bit-shifting rather than decimal checking for writing out time values
 * Convert HEX or BCD value to DEC where 0x45 == 0d45 	
 */
int hexCompensation(int units){

	int unitsU = units%0x10;

	if (units >= 0x50){
		units = 50 + unitsU;
	}
	else if (units >= 0x40){
		units = 40 + unitsU;
	}
	else if (units >= 0x30){
		units = 30 + unitsU;
	}
	else if (units >= 0x20){
		units = 20 + unitsU;
	}
	else if (units >= 0x10){
		units = 10 + unitsU;
	}
	return units;
}


/*
 * decCompensation
 * This function "undoes" hexCompensation in order to write the correct base 16 value through I2C
 */
int decCompensation(int units){
	int unitsU = units%10;

	if (units >= 50){
		units = 0x50 + unitsU;
	}
	else if (units >= 40){
		units = 0x40 + unitsU;
	}
	else if (units >= 30){
		units = 0x30 + unitsU;
	}
	else if (units >= 20){
		units = 0x20 + unitsU;
	}
	else if (units >= 10){
		units = 0x10 + unitsU;
	}
	return units;
}


/*
 * hourInc
 * Fetch the hour value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 23 hours in a day
 * Software Debouncing should be used
 */
void hourInc(void){
	//Debounce
//	printf("Intial hours:%d\n",HH);
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 1 triggered, %x\n", hours);
		//Fetch RTC Time
		//Increase hours by 1, ensuring not to overflow
		//Write hours back to the RTC
		//if(digitalRead(BTNS[0])==1){
		    HH=HH+1;
		    if(HH>23){
		        HH=0;
		    }
		//}
	}
	lastInterruptTime = interruptTime;
//	printf("Upadted hours:%d\n",HH);
}

/* 
 * minInc
 * Fetch the minute value off the RTC, increase it by 1, and write back
 * Be sure to cater for there only being 60 minutes in an hour
 * Software Debouncing should be used
 */
void minInc(void){
//	printf("Intial minutes:%d\n", MM);
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		printf("Interrupt 2 triggered, %x\n", mins);
		//Fetch RTC Time
		//Increase minutes by 1, ensuring not to overflow
		//Write minutes back to the RTC

		//if(digitalRead(BTNS[1])==1){
		    MM=MM+1;
		    if(MM>59){
		        HH=HH+1;
			MM=0;
		    }
		//}
	}
	lastInterruptTime = interruptTime;
//	printf("Updated minutes:%d\n",MM);
}

//This interrupt will fetch current time from another script and write it to the clock registers
//This functions will toggle a flag that is checked in main
void toggleTime(void){
	long interruptTime = millis();

	if (interruptTime - lastInterruptTime>200){
		HH = getHours();
		MM = getMins();
		SS = getSecs();

		HH = hFormat(HH);
		HH = decCompensation(HH);
		wiringPiI2CWriteReg8(RTC, HOUR_REGISTER, HH);

		MM = decCompensation(MM);
		wiringPiI2CWriteReg8(RTC, MIN_REGISTER, MM);

		SS = decCompensation(SS);
		wiringPiI2CWriteReg8(RTC, SEC_REGISTER, 0b10000000+SS);

	}
	lastInterruptTime = interruptTime;
//	printf("Time: %d: %d: %d:\n",HH,MM,SS);
}