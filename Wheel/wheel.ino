/*
 * Firmware for Lotus Elise wireless steering wheel button controls - WHEEL PCB
 * RF24L01 on steering wheel talks to transceiver wired to chassis loom
 * Will Norman
 * 04/01/21
 */

// Libraries
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

// Button Inputs
#define INPUT_RIGHT A4
#define INPUT_LEFT  6
#define INPUT_BEAM  A2
#define INPUT_HORN  A1
#define INPUT_WASH  7
#define INPUT_INT   A0
#define INPUT_SPD1  9
#define INPUT_SPD2  8

// LED Outputs
#define LED_RIGHT     A5  
#define LED_LEFT      5
#define LED_BEAM      A3
#define IND_INTERVAL  330  // LED blinking interval in ms

// RF24 Pins
#define MOSI        11
#define MISO        12
#define CLK         13
#define IRQ         2
#define CSN         3
#define CE          4

// Radio Object
RF24 radio(CE, CSN);

// Global Variables
const byte address[6] = "00001";
volatile byte payload_transmit = 0b00000000;  // byte to be transmitted that encodes button presses
volatile int counter = 0;

// Button Encoding
#define BIT_RIGHT   0b00000001
#define BIT_LEFT    0b00000010
#define BIT_BEAM    0b00000100
#define BIT_HORN    0b00001000
#define BIT_WASH    0b00010000
#define BIT_INT     0b00100000
#define BIT_SPD1    0b01000000
#define BIT_SPD2    0b10000000

void setup() {

    Serial.begin(9600);
  
    // Initialise RF24 module
    radio.begin();
    radio.setPALevel(RF24_PA_MIN);
    radio.setRetries(15,5);          // 15x250us + 250us wait for ACK, 5 retries
    radio.setChannel(111);           // 2400 MHz + 111(s) ;) = 2511 MHz;
    radio.openWritingPipe(address);
    radio.stopListening();

    // Initialise Inputs
    pinMode(INPUT_RIGHT,INPUT_PULLUP);
    pinMode(INPUT_LEFT,INPUT_PULLUP);
    pinMode(INPUT_BEAM,INPUT_PULLUP);
    pinMode(INPUT_HORN,INPUT_PULLUP);
    pinMode(INPUT_WASH,INPUT_PULLUP);
    pinMode(INPUT_INT,INPUT_PULLUP);
    pinMode(INPUT_SPD1,INPUT_PULLUP);
    pinMode(INPUT_SPD2,INPUT_PULLUP);
    
    // Initialise Outputs
    pinMode(LED_RIGHT,OUTPUT);
    pinMode(LED_LEFT,OUTPUT);
    pinMode(LED_BEAM,OUTPUT);
    digitalWrite(LED_RIGHT,LOW);
    digitalWrite(LED_LEFT,LOW);
    digitalWrite(LED_BEAM,LOW);
    
}

void loop() {

    bool state_right = 0;         // variable that tracks state of button press
    bool state_left = 0;          // variable that tracks state of button press
    bool state_beam = 0;          // variable that tracks state of button press
    bool state_old_right = 0;     // variable that tracks state of button previously
    bool state_old_left = 0;      // variable that tracks state of button previously
    bool state_old_beam = 0;      // variable that tracks state of button previously
    bool toggle_beam = 0;         // variable that holds beam on if button is pressed long enough
    unsigned long timer_beam = 0; // variable to track the time the beam button has been held

    bool state_led_right = 0;
    bool state_led_left = 0;
    unsigned long timer_right = 0;
    unsigned long timer_left = 0;

    while(1){

      // Right Indicator
      state_right = !digitalRead(INPUT_RIGHT); // read state of button
      if(state_right && !state_old_right){ // if button pressed, and previously wasn't pressed, do something
          // if indicator already on - switch off
          if(payload_transmit & BIT_RIGHT){
              payload_transmit = payload_transmit &~(BIT_RIGHT);
          // else indicator is already off, so switch on
          }else{
                payload_transmit = payload_transmit | BIT_RIGHT;
              // if other indicator is on, switch off
              if(payload_transmit & BIT_LEFT){
                  payload_transmit = payload_transmit & ~(BIT_LEFT);
              }
          }
          state_old_right = 1;
      }else if(!state_right){ // if button not pressed, update state_old
          state_old_right = 0;
      }

      // Left Indicator (copy of Right Indicator but swap left/right)
      state_left = !digitalRead(INPUT_LEFT);
      if(state_left && !state_old_left){ // if new button press - toggle on/off value
          // if indicator already on - switch off
          if(payload_transmit & BIT_LEFT){
              payload_transmit = payload_transmit & ~(BIT_LEFT);
          // else indicator is off, switch on
          }else{
              payload_transmit = payload_transmit | BIT_LEFT;
              // if other indicator is on, switch off
              if(payload_transmit & BIT_RIGHT){
                  payload_transmit = payload_transmit & ~(BIT_RIGHT);
              }
          }
          state_old_left = 1;
      }else if(!state_left){
          state_old_left = 0;
      }

      // Full Beam
      state_beam = !digitalRead(INPUT_BEAM);
      if(state_beam && !state_old_beam){
          // if button pressed, turn on, start timer, turn toggle off
          payload_transmit = payload_transmit | BIT_BEAM;
          digitalWrite(LED_BEAM,HIGH);
          timer_beam = millis();
          state_old_beam = 1;
          toggle_beam = 0;
      // if button pressed and STILL pressed, check timer
      }else if(state_beam && state_old_beam){
          // if button held for more than 1 seconds, keep turned on with toggle variable
          if(millis() - timer_beam > 750){
            toggle_beam = 1;
          }else{
            toggle_beam = 0;
          }
      // if button is off, and toggle is off, switch off
      }else if(!state_beam){
        if(!toggle_beam){
            payload_transmit = payload_transmit & ~(BIT_BEAM);
            digitalWrite(LED_BEAM,LOW);
        }
        state_old_beam = 0;
      }

      // Horn - Momentary
      if(!digitalRead(INPUT_HORN)){
        payload_transmit = payload_transmit | BIT_HORN;
      }else{
        payload_transmit = payload_transmit & ~(BIT_HORN);
      }

      // Wiper Wash - Momentary
      if(!digitalRead(INPUT_WASH)){
        payload_transmit = payload_transmit | BIT_WASH;
      }else{
        payload_transmit = payload_transmit & ~(BIT_WASH);
      }

      // Intermittent Wipers - Momentary (held by rotary switch)
      if(!digitalRead(INPUT_INT)){
        payload_transmit = payload_transmit | BIT_INT;
      }else{
        payload_transmit = payload_transmit & ~(BIT_INT);
      }

      // Wipers Speed 1 - Momentary (held by rotary switch)
      if(!digitalRead(INPUT_SPD1)){
        payload_transmit = payload_transmit | BIT_SPD1;
      }else{
        payload_transmit = payload_transmit & ~(BIT_SPD1);
      }

      // Wipers Speed 2 - Momentary (held by rotary switch)
      if(!digitalRead(INPUT_SPD2)){
        payload_transmit = payload_transmit | BIT_SPD2;
      }else{
        payload_transmit = payload_transmit & ~(BIT_SPD2);
      }

      // Right Indicator LED
      if((payload_transmit & BIT_RIGHT)  && (millis() - timer_right > IND_INTERVAL)){
        timer_right = millis();
        state_led_right = !state_led_right;
        digitalWrite(LED_RIGHT,state_led_right);
      }else if(!(payload_transmit & BIT_RIGHT)){
        timer_right = 0;
        state_led_right = 0;
        digitalWrite(LED_RIGHT,state_led_right);
      }
      
      // Left Indicator LED
      if((payload_transmit & BIT_LEFT)  && (millis() - timer_left > IND_INTERVAL)){
        timer_left = millis();
        state_led_left = !state_led_left;
        digitalWrite(LED_LEFT,state_led_left);
      }else if(!(payload_transmit & BIT_LEFT)){
        timer_left = 0;
        state_led_left = 0;
        digitalWrite(LED_LEFT,state_led_left);
      }

      // Transmit to Chassis
      if(!radio.write(&payload_transmit, sizeof(payload_transmit))){
        counter++;
        // Serial.println(counter);
      }else{
        counter = 0;
      }

      delay(20);

//      if(counter > 2000){ // 500 ms of no response
//        errorBlink(); 
//        counter = 0 ;
//      }
      
    } // while(1)
} // loop

void errorBlink(void){
        for(int i = 0 ; i < 3 ; i++){
          digitalWrite(LED_RIGHT,HIGH);
          digitalWrite(LED_LEFT,HIGH);
          digitalWrite(LED_BEAM,HIGH);
          delay(100);
          digitalWrite(LED_RIGHT,LOW);
          digitalWrite(LED_LEFT,LOW);
          digitalWrite(LED_BEAM,LOW);
          delay(100);
        }
        payload_transmit = 0;
}
