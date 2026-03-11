// 6 Channel Transmitter
// KendinYap Channel

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xABCDABCD71LL;         // NOTE: The address in the Transmitter and Receiver code must be the same "0xABCDABCD71LL"
RF24 radio(9, 10);                               // select CE,CSN pin

struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte aux1;
  byte aux2;
};

Signal data;

void ResetData() 
{
  data.throttle = 0;                  
  data.pitch = 127;
  data.roll = 127;
  data.yaw = 127;
  data.aux1 = 0;                       
  data.aux2 = 0;
}

void setup()
{
  // Configure the NRF24 module
  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);    // The lowest data rate value for more stable communication
  radio.setPALevel(RF24_PA_MAX);      // Output power is set for maximum range
  radio.stopListening();              // Start the radio comunication for Transmitter
  ResetData();
}

// Joystick center and its borders
int Border_Map(int val, int lower, int middle, int upper, bool reverse)
{
  val = constrain(val, lower, upper);
  if ( val < middle )
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return ( reverse ? 255 - val : val );
}

// Toggle state variables for aux channels
bool aux1State = false;
bool aux2State = false;
unsigned long lastDebounceTimeAux1 = 0;
unsigned long lastDebounceTimeAux2 = 0;
const unsigned long debounceDelay = 50;  // 50ms debounce

void loop()
{                                  
  data.roll = Border_Map( analogRead(A3), 0, 512, 1023, true );        // CH1   Note: "true" or "false" for signal direction
  data.pitch = Border_Map( analogRead(A0), 0, 512, 1023, true );       // CH2    
  data.throttle = Border_Map( analogRead(A2),0, 340, 570, true );      // CH3   Note: For Single side ESC
  // data.throttle = Border_Map( analogRead(A2),0, 512, 1023, true );  // CH3   Note: For Bidirectional ESC  
  data.yaw = Border_Map( analogRead(A1), 0, 512, 1023, false );        // CH4

  // Handle aux1 toggle switch (CH5) - toggles on press with debounce
  int readingAux1 = digitalRead(0);
  if (readingAux1 != (aux1State ? HIGH : LOW)) {  // Change detected (assuming LOW when active for toggle)
    lastDebounceTimeAux1 = millis();
  }
  if ((millis() - lastDebounceTimeAux1) > debounceDelay) {
    if (readingAux1 == LOW) {  // Trigger on LOW (button/switch pressed)
      aux1State = !aux1State;
    }
  }
  data.aux1 = aux1State ? 255 : 0;

  // Handle aux2 toggle switch (CH6) - toggles on press with debounce
  int readingAux2 = digitalRead(3);
  if (readingAux2 != (aux2State ? HIGH : LOW)) {
    lastDebounceTimeAux2 = millis();
  }
  if ((millis() - lastDebounceTimeAux2) > debounceDelay) {
    if (readingAux2 == LOW) {
      aux2State = !aux2State;
    }
  }
  data.aux2 = aux2State ? 255 : 0;

  radio.write(&data, sizeof(Signal));  
}
