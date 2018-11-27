int data = 3; 
int clock = 5;
int latch = 4;

//Used for single LED manipulation
volatile int ledState = 0;
const int ON = HIGH;
const int OFF = LOW;
bool on = false;
int i = 0;

void setup()
{
  Serial.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);  
}



/*
 * loop() - this function will start after setup finishes and then repeat
 * we set which LEDs we want on then call a routine which sends the states to the 74HC595
 */
void loop()                     // run over and over again
{
  if(i==8)
  {
    on=!on;
    i=0;
  }
  if(!on)
  {
    changeLED(i,ON);
  }else
  {
    changeLED(i,OFF);
  }
    delay(300);
    i++;
}



/*
 * updateLEDs() - sends the LED states set in ledStates to the 74HC595
 * sequence
 */
void updateLEDs(int value){
  digitalWrite(latch, LOW);     //Pulls the chips latch low
  shiftOut(data, clock, MSBFIRST, value); //Shifts out the 8 bits to the shift register
  digitalWrite(latch, HIGH);   //Pulls the latch high displaying the data
}


//These are used in the bitwise math that we use to change individual LEDs
//For more details http://en.wikipedia.org/wiki/Bitwise_operation
int bits[] = {B00000001, B00000010, B00000100, B00001000, B00010000, B00100000, B01000000, B10000000};
int masks[] = {B11111110, B11111101, B11111011, B11110111, B11101111, B11011111, B10111111, B01111111};
/*
 * changeLED(int led, int state) - changes an individual LED 
 * LEDs are 0 to 7 and state is either 0 - OFF or 1 - ON
 */
 void changeLED(int led, int state){
   //ledState = ledState & masks[led];  //clears ledState of the bit we are addressing
   //ledState = ledState & B00000000;
   if(state == ON){ledState = ledState | bits[led];}
   if(state == OFF){
    ledState = ledState & masks[led];
   }
   updateLEDs(ledState);              //send the new LED state to the shift register
 }
