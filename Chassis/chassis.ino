/*
 * Firmware for Lotus Elise wireless steering wheel button controls - CHASSIS PCB
 * RF24L01 on steering wheel talks to transceiver wired to chassis loom
 * Will Norman
 * 04/01/21
 */

// Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Control Outputs
#define OUTPUT_RIGHT      A4
#define OUTPUT_LEFT       A5
#define OUTPUT_BEAM       5
#define OUTPUT_HORN       6
#define OUTPUT_WASH       A3
#define OUTPUT_SPD1       A2
#define OUTPUT_OBSOLETE   7
#define OUTPUT_SPD2       8

// RF24 Pins
#define MISO  11
#define MOSI  12
#define CLK   13
#define IRQ   2
#define CSN   3
#define CE    4

// Radio Object
RF24 radio(CE, CSN);

// Global Variables
const byte address[6] = "00001";
volatile byte payload = 0;
volatile unsigned long timeout_timer = 0;

// Button Encoding
#define BIT_RIGHT     0b00000001    // Right indicator
#define BIT_LEFT      0b00000010    // Left indicator
#define BIT_BEAM      0b00000100    // Full beam
#define BIT_HORN      0b00001000    // Horn
#define BIT_WASH      0b00010000    // Wiper Washer
#define BIT_SPD1      0b00100000    // Slow Speed Wipers
#define BIT_OBSOLETE  0b01000000    // Unused
#define BIT_SPD2      0b10000000    // Fast Speed Wipers

void setup() {

    // Serial for Debug
    Serial.begin(9600);
  
    // Initialise RF24 module
    radio.begin();
    radio.openReadingPipe(0, address);
    radio.setPALevel(RF24_PA_MIN);;
    radio.setChannel(111);       // 2400 MHz + 111(s) ;) = 2511 MHz;
    radio.startListening();
    
    // Initialise Outputs
    pinMode(OUTPUT_RIGHT,OUTPUT);
    pinMode(OUTPUT_LEFT,OUTPUT);
    pinMode(OUTPUT_BEAM,OUTPUT);
    pinMode(OUTPUT_HORN,OUTPUT);
    pinMode(OUTPUT_WASH,OUTPUT);
    pinMode(OUTPUT_SPD1,OUTPUT);
    pinMode(OUTPUT_SPD2,OUTPUT);
    pinMode(OUTPUT_OBSOLETE,OUTPUT);
    
    digitalWrite(OUTPUT_RIGHT,LOW);
    digitalWrite(OUTPUT_LEFT,LOW);
    digitalWrite(OUTPUT_BEAM,LOW);
    digitalWrite(OUTPUT_HORN,LOW);
    digitalWrite(OUTPUT_WASH,LOW);
    digitalWrite(OUTPUT_SPD1,LOW);
    digitalWrite(OUTPUT_SPD2,LOW);
    digitalWrite(OUTPUT_OBSOLETE,LOW);

    Serial.println("Startup...");

}

void loop() {
  
  if(radio.available()){
    radio.read(&payload,sizeof(payload));
    timeout_timer = millis();
    Serial.println(payload);
  }

  if(millis() - timeout_timer > 2500){
    payload = 0;
    digitalWrite(OUTPUT_RIGHT,LOW);
    digitalWrite(OUTPUT_LEFT,LOW);
    digitalWrite(OUTPUT_BEAM,LOW);
    digitalWrite(OUTPUT_HORN,LOW);
    digitalWrite(OUTPUT_WASH,LOW);
    digitalWrite(OUTPUT_SPD1,LOW);
    digitalWrite(OUTPUT_SPD2,LOW);
    Serial.println("Timeout");
  }

  if(payload & BIT_RIGHT){
    digitalWrite(OUTPUT_RIGHT,HIGH);
  }else{
    digitalWrite(OUTPUT_RIGHT,LOW);
  }

  if(payload & BIT_LEFT){
    digitalWrite(OUTPUT_LEFT,HIGH);
  }else{
    digitalWrite(OUTPUT_LEFT,LOW);
  }

  if(payload & BIT_BEAM){
    digitalWrite(OUTPUT_BEAM,HIGH);
  }else{
    digitalWrite(OUTPUT_BEAM,LOW);
  }

 if(payload & BIT_HORN){
   digitalWrite(OUTPUT_HORN,HIGH);
 }else{
   digitalWrite(OUTPUT_HORN,LOW);
 }
  
  if(payload & BIT_WASH){
    digitalWrite(OUTPUT_WASH,HIGH);
  }else{
    digitalWrite(OUTPUT_WASH,LOW);
  }
  
  if(payload & BIT_SPD1){
    digitalWrite(OUTPUT_SPD1,HIGH);
  }else{
    digitalWrite(OUTPUT_SPD1,LOW);
  }
  
  if(payload & BIT_SPD2){
    digitalWrite(OUTPUT_SPD2,HIGH);
  }else{
    digitalWrite(OUTPUT_SPD2,LOW);
  }

  delay(20);

}
