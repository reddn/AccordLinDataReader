#include <Arduino.h>

int overallCounter = 0;

#define outputSerial Serial
#define EPStoLKAS_Serial Serial2
#define LKAStoESP_Serial Serial3



uint8_t LKAStoEPSCounter = 0;
uint8_t EPStoLKASCounter = 0;

uint8_t LKAStoESPBuffer[4];
uint8_t EPStoLKASBuffer[10];
unsigned long lastLEDFlash = 0;
unsigned long lastEPStoLKASdata = 0;
unsigned long lastLKAStoEPSdata = 0;

uint8_t relayLinBus = 0;


void printuint_t(uint8_t var) { ///i stole this from my program https://github.com/reddn/LIN2LINrepeaterMEGA, i think it should work
  for (uint8_t test = 0x80; test; test >>= 1) {
  	if(test == 0x8) outputSerial.write(' '); // 0x8 is 0000 1000.. so it gives a break between the 2 4 bit writes
    outputSerial.write(var  & test ? '1' : '0');
  }
}

void clearCounters(){
	LKAStoEPSCounter = 0;
	EPStoLKASCounter = 0;
}

void flashLED(){
	if( (millis() - lastLEDFlash) > 1000){
		digitalWrite(LED_BUILTIN,!digitalRead(LED_BUILTIN));
		lastLEDFlash = millis(); 
	}
}

void handleEPStoLKAS(){

	if(EPStoLKAS_Serial.available()){
		uint8_t data2 = EPStoLKAS_Serial.read();

		if(relayLinBus) EPStoLKAS_Serial.write(data2);

		uint8_t offset4 = data2 >>4;
		if(offset4 == 2 || offset4 == 0){
			for(uint8_t a = 0; a <  EPStoLKASCounter; a++){
			// outputSerial.write(EPStoLKASBuffer[a]);
				printuint_t(EPStoLKASBuffer[a]);
				outputSerial.print("  ");
		}
			outputSerial.println();
			EPStoLKASCounter = 0;
		}
		// outputSerial.write(data2);
		// printuint_t(data2);
		// outputSerial.print(" ");

		
		EPStoLKASBuffer[EPStoLKASCounter++] = data2;
	
		
		// printuint_t(data2);
		// outputSerial.print("  ");

		

		lastEPStoLKASdata = millis();

		return;
		
		if(EPStoLKASCounter > 4) {
			EPStoLKASCounter = 0;
			// outputSerial.println();
		}
		


		// if( EPStoLKASCounter == 5){ //the EPStoLKAS buffer is full as it already has 5 bytes in it. time to send the data.
		// 	if(LKAStoEPSCounter != 4) {
		// 		outputSerial.print("LKAStoEPSCounter ");
		// 		outputSerial.print(LKAStoEPSCounter);
		// 		outputSerial.print("EPStoLKASCounter ");
		// 		outputSerial.print(EPStoLKASCounter);
		// 		outputSerial.println(" **error in the counters");
		// 		clearCounters();
		// 		return;
		// 	}
		// 	//print LKAS to EPS
			
		// 	for(uint8_t a = 0; a<5; a++){
		// 		if(a==4){
		// 			printuint_t( (uint8_t)0x00 );
		// 		}else{
		// 			printuint_t(LKAStoESPBuffer[a]);
		// 		}
		// 			outputSerial.write('\t');
		// 			printuint_t(EPStoLKASBuffer[a]);
		// 			outputSerial.write('\t');
		// 			outputSerial.println(overallCounter);
		// 	}
		// 	clearCounters();
		// } //if EPStoLKASCunter == 5
		
	} // EPStoLKAS_Serial.available

	// if( ( millis() - lastLKAStoEPSdata ) > 2 && EPStoLKASCounter >3){
	// 	for(uint8_t a = 0; a <  EPStoLKASCounter; a++){
	// 		// outputSerial.write(EPStoLKASBuffer[a]);
	// 		printuint_t(EPStoLKASBuffer[a]);
	// 		outputSerial.print("  ");
	// 	}
	// 	outputSerial.println();
	// 	EPStoLKASCounter = 0;
	// }
}

void handleLKAStoEPS(){
	if(LKAStoESP_Serial.available()){
		uint8_t data = LKAStoESP_Serial.read();
		if(relayLinBus) LKAStoESP_Serial.write(data);
		return;

		if(LKAStoEPSCounter == 0){
			if((data >> 6) != 0){ // the first byte of the frame always starts with 00 in the first byte. if its not 00, then it isn't t he first byte and something is wrong. skip
				return;
			}
			EPStoLKASCounter = 0; //  if its the start of the LKAStoEPS frame, then the EPStoLKAS buffer should be empty and counter at 0
		}else if(LKAStoEPSCounter > 3){//this hsould not happen as the max in frame is 4.. sync got fucked and needs to restart
			clearCounters();
			return;
		}
		
		LKAStoESPBuffer[LKAStoEPSCounter] = data;
		LKAStoEPSCounter++;
	} // if LKAStoESP_serial.available


}

//***********************************************************************************8 SETUP / LOOP

void setup(){
	outputSerial.begin(460800);
	LKAStoESP_Serial.begin(9600,SERIAL_8E1);
	EPStoLKAS_Serial.begin(9600,SERIAL_8E1);
	pinMode(LED_BUILTIN,OUTPUT);
	outputSerial.println("HI");
	
}


void loop(){
	

	handleEPStoLKAS();
	handleLKAStoEPS();	
	flashLED();
}