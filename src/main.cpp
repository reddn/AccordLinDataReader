#include <Arduino.h>

int overallCounter = 0;

#define outputSerial Serial1
#define LKAStoESP_Serial Serial2
#define EPStoLKAS_Serial Serial3


uint8_t LKAStoEPSCounter = 0;
uint8_t EPStoLKASCounter = 0;

uint8_t LKAStoESPBuffer[4];
uint8_t EPStoLKASBuffer[5];


void printuint_t(uint8_t var) { ///i stole this from my program https://github.com/reddn/LIN2LINrepeaterMEGA, i think it should work
  for (uint8_t test = 0x80; test; test >>= 1) {
  	if(test == 0x8) outputSerial.write(' '); // 0x8 is 0000 1000.. so it gives a break between the 8bits
    outputSerial.write(var  & test ? '1' : '0');
  }

}

void clearCounters(){
	LKAStoEPSCounter = 0;
	EPStoLKASCounter = 0;
}

void setup(){
	outputSerial.begin(1000000);
	LKAStoESP_Serial.begin(9600,8E1);
	EPStoLKAS_Serial.begin(9600,8E1);
	
}


void loop(){
	if(LKAStoESP_Serial.available()){
		uint8_t data = LKAStoESP_Serial.read();
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
	
	if(EPStoLKAS_Serial.available()){
		uint8_t data2 = EPStoLKAS_Serial.read();
		EPStoLKASBuffer[EPStoLKASCounter] = data2;
	
		EPStoLKASCounter++;
		
		if( EPStoLKASCounter == 5){ //the EPStoLKAS buffer is full as it already has 5 bytes in it. time to send the data.
			if(LKAStoEPSCounter != 4) {
				outputSerial.print("LKAStoEPSCounter ");
				outputSerial.print(LKAStoEPSCounter);
				outputSerial.print("EPStoLKASCounter ");
				outputSerial.print(EPStoLKASCounter);
				outputSerial.println(" **error in the counters");
				clearCounters();
				return;
			}
			//print LKAS to EPS
			
			for(uint8_t a = 0; a<5; a++){
				if(a==4){
					printuint_t( (uint8_t)0x00 );
				}else{
					printuint_t(LKAStoESPBuffer[a]);
				}
					outputSerial.write('\t');
					printuint_t(EPStoLKASBuffer[a]);
					outputSerial.write('\t');
					outputSerial.println(overallCounter);
			}
			clearCounters();
		} //if EPStoLKASCunter == 5
		
	} // EPStoLKAS_Serial.available	
	
}