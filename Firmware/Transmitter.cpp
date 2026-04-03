// 6 Channel Transmitter | 6 Kanal Verici
// KendinYap Channel

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

const uint64_t pipeOut = 0xABCDABCD71LL;
RF24 radio(9, 10); // CE, CSN pins

struct Signal {
  byte throttle;
  byte pitch;
  byte roll;
  byte yaw;
  byte aux1;
  byte aux2;
};

Signal data;

void ResetData() {
  data.throttle = 0;
  data.pitch = 127;
  data.roll = 127;
  data.yaw = 127;
  data.aux1 = 0;
  data.aux2 = 0;
}

void setup() {
  // Define switch pins with internal pull-up resistors
  pinMode(0, INPUT_PULLUP); // S2 -> D0
  pinMode(5, INPUT_PULLUP); // S3 -> D5

  radio.begin();
  radio.openWritingPipe(pipeOut);
  radio.setChannel(100);
  radio.setAutoAck(false);
  radio.setDataRate(RF24_250KBPS);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();
  ResetData();
}

int Border_Map(int val, int lower, int middle, int upper, bool reverse) {
  val = constrain(val, lower, upper);
  if (val < middle)
    val = map(val, lower, middle, 0, 128);
  else
    val = map(val, middle, upper, 128, 255);
  return (reverse ? 255 - val : val);
}

void loop() {
  // Joysticks
  data.roll     = Border_Map(analogRead(A3), 0, 512, 1023, true);
  data.pitch    = Border_Map(analogRead(A0), 0, 512, 1023, true);
  data.throttle = Border_Map(analogRead(A2), 0, 340, 570, true);
  data.yaw      = Border_Map(analogRead(A1), 0, 512, 1023, false);

  // Switches (Inverted logic due to INPUT_PULLUP: Pressed/GND = 0)
  data.aux1 = digitalRead(0); 
  data.aux2 = digitalRead(3);

  radio.write(&data, sizeof(Signal));
}