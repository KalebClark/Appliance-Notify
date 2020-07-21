/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "i:/Code/Particle/Appliance_Notifier/AP-Notify-App/src/AP-Notify.ino"
/*
 * Project Appliance Notify
 * Description: Uses the Elechouse V3 voice recognition module to listen for appliance *done*
 *              songs. Then sends message to particle cloud which is consumed by a raspberry
 *              pi and then handled (SMS message)
 * Author: Kaleb Clark
 * Date: 07-20-2020
 */

// Includes
#include <ParticleSoftSerial.h>
#include <VoiceRecognitionV3.h>

// Declare Pins
void setup();
void loop();
void printVR(uint8_t *buf);
void printSignature(uint8_t *buf, int len);
#line 15 "i:/Code/Particle/Appliance_Notifier/AP-Notify-App/src/AP-Notify.ino"
int pss_rx      = 2;
int pss_tx      = 3;
int listen_led  = 4;
int process_led = 5;

// Instantiate Voice Recognition module
VR apVR(pss_rx, pss_tx);        // RX, TX pins for ParticleSoftSerial

// Particle Functions
bool resetRecognize(String command);

// Misc assignments
#define onRecord    (0)
#define offRecord   (1)
uint8_t records[7];
uint8_t buf[64];
int cnt = 0;
bool recognized = 0;
bool listening = 1;

/*
 * Setup ======================================================================
 */ 
void setup() {
  // Setup debug Serial
  Serial.begin(115200);

  // Setup Voice Recognition module
  apVR.begin(9600);   // Runs at 9600 baud

  // Setup Pin Modes
  pinMode(listen_led, OUTPUT);
  pinMode(process_led, OUTPUT);

  // Particle cloud
  Particle.variable("recognized", recognized);
  Particle.function("resetRecognize", resetRecognize);

  // Clear recognizer
  if(apVR.clear() == 0) {
    Serial.println("Recognizer Cleared");
  } else {
    Serial.println("Cannot find module. Check your wires!");
  }

  // Load Recognizer programs
  if(apVR.load((uint8_t)onRecord) >= 0) {
    Serial.println("onRecord Loaded");
  }

  if(apVR.load((uint8_t)offRecord) >= 0) {
    Serial.println("offRecord Loaded");
  }

}

/*
 * Main Loop ==================================================================
 */ 
void loop() {

  // Listen Mode
  if(!recognized) {
    int ret;
    ret = apVR.recognize(buf, 50);
    if(ret > 0) {
      Serial.print("Bank: "); Serial.println(buf[1], DEC);
      Serial.print("Count: "); Serial.println(cnt);
      printVR(buf);
      cnt++;

      // Handle Action
      recognized = 1;     // Set the Particle Variable to true
      listening = 0;
    } // end if (ret > 0) 
  }

  if(listening) {
    digitalWrite(listen_led, HIGH);
    digitalWrite(process_led, LOW);
  } else {
    digitalWrite(listen_led, LOW);
    digitalWrite(process_led, HIGH);
  }
  // End Listen Mode
}

bool resetRecognize(String command) {
  recognized = 0;
  listening = 1;
  uint8_t fub[64];
  for(int i = 0; i <= 20; i++) {
    int retg = apVR.recognize(fub, 50);
    Serial.print("Clearning Buffer: "); Serial.println(i);
    Serial.print("\tBank: "); Serial.println(fub[1], DEC);
  }
}

// Debug Functions ============================================================
void printVR(uint8_t *buf)
{
  Serial.println("VR Index\tGroup\tRecordNum\tSignature");

  Serial.print(buf[2], DEC);
  Serial.print("\t\t");

  if(buf[0] == 0xFF){
    Serial.print("NONE");
  }
  else if(buf[0]&0x80){
    Serial.print("UG ");
    Serial.print(buf[0]&(~0x80), DEC);
  }
  else{
    Serial.print("SG ");
    Serial.print(buf[0], DEC);
  }
  Serial.print("\t");

  Serial.print(buf[1], DEC);
  Serial.print("\t\t");
  if(buf[3]>0){
    printSignature(buf+4, buf[3]);
  }
  else{
    Serial.print("NONE");
  }
  Serial.println("\r\n");
}

void printSignature(uint8_t *buf, int len)
{
  int i;
  for(i=0; i<len; i++){
    if(buf[i]>0x19 && buf[i]<0x7F){
      Serial.write(buf[i]);
    }
    else{
      Serial.print("[");
      Serial.print(buf[i], HEX);
      Serial.print("]");
    }
  }
}