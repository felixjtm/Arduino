//Stores whether or not this code is for player 1 or not, if value is 1 then this is player 1
bool player1 = 1;

//Stores the current turn number
int turnCount = 0;

//Initialises the pins for shift register
int data = 3; 
int clock = 5;
int latch = 4;

//Initialises the pins for the Piezo element and the final LED that does not fit in the shift register
int pinLED = 8;
int piezoPin = 11;

//Initialises the pins for the buttons
int selectButtonPin = 9;
int changeButtonPin = 10;

//Variables for reading the pushbutton status
int button1State = 0;
int button2State = 0;

//Used for single LED manipulation
volatile int ledState = 0;
const int ON = HIGH;
const int OFF = LOW;
bool on = false;


int selectedLED = 0; //Stores the currently selected LED
bool myTurn = true; //Stores whether or not it is this player's turn
bool toggleOn = true; //Stores whether the LEDs should be toggled on or off
bool toggleSelected = true; //Stores whether or not the currently selected LED should be toggled on or off
int boardState[2][9]; //1 being player one, 0 being player two, the second dimension is for each LED

//Initialises variables used for timing LED flashes and button debounces
unsigned long lastPulseTime = 0;
unsigned long lastChoiceTime = 0;
unsigned long lastSelectTime = 0;
unsigned long lastSelectedPulseTime = 0;

/**
 * Sets the pinmodes for all applicable pins, as well as turning off all the LEDS,
 * Sets the correct turn based on which player it is
 */
void setup()
{
  Serial.begin(9600);
  pinMode(data, OUTPUT);
  pinMode(clock, OUTPUT);  
  pinMode(latch, OUTPUT);  
  pinMode(pinLED,OUTPUT);
  pinMode(selectButtonPin,INPUT);
  pinMode(changeButtonPin,INPUT);
  pinMode(piezoPin, OUTPUT);
  for(int i = 0;i<8;i++)
  {
    changeLED(i,OFF);
  }
  digitalWrite(pinLED,LOW);

  if (!player1) 
  {
    myTurn = false;
  }
  
}


/**
 * Repeats and performs actions based on whos turn it is
 */
void loop()
{
    //If it is this current user's turn
    if(myTurn)
    {
      //Stores the current time for debounce and LED flashing purposes
      unsigned long currentTime = millis();  

      //Stores whether or not a button has been pressed
      button1State = digitalRead(selectButtonPin);
      button2State = digitalRead(changeButtonPin);

      //If button1 has been pressed
      if(button1State == LOW)
      {
        //If it has been more than 200 milliseconds since button1 was last pressed and commands went through
        if (currentTime - lastChoiceTime > 200) 
        {
          //If the LED has already been selected then play a rejection sound
          if(boardState[0][selectedLED] == 1 || boardState[1][selectedLED] == 1)
          {        
            tone(11,500,100); 
          }
          //If the LED has not already been selected then select it
          else
          {
            //Play a high tone to indicate that the LED has been selected
            tone(11,1000,100); 

            //Increment the turn count and increment the board state for the current player at the correct LED
            turnCount++;
            boardState[player1][selectedLED]++;

            //Reset the button state
            button1State = HIGH;

            //Send the LED that has been selected to the other player
            Serial.write(selectedLED);

            //Change the turn
            myTurn = false;

            //If this player has won the game then player a victory tone and reset the game
            if (CheckWin(player1)) 
            {
              playWinningTone();
              resetGame();
            }
            //If it is turn 9 and no one has won then the game is a tie, reset the game
            else
            {
              if(turnCount == 9)
              {
                resetGame();
              }
            }

          }
          //Set the last time that the button was pressed to the current time
          lastChoiceTime = currentTime;
        }
      }
      //If button2 has been pressed 
      if(button2State == LOW)
      {
        //If it has been more than 200 milliseconds since button2 was last pressed and commands went through
        if (currentTime - lastChoiceTime > 200) {
          //Turns off the currently selected LED
          if (selectedLED == 8) {
              digitalWrite(selectedLED, LOW);
          } else {
              changeLED(selectedLED, OFF);
          }
          //Changes the selected LED to the next one, if it is already on the final LED then loop round to the first
          selectedLED  = (selectedLED+1) % 9;

          //Resets button2
          button2State = HIGH;

          //Set the last time that the button was pressed to the current time
          lastChoiceTime = currentTime;
        }
     
      }
      //Pulse the currently selected LED
      pulseSelected(selectedLED);
    }
    else
    {
      //If a message has been received
      if(Serial.available()>0)
      {
        //The selected LED is set to the message received
        selectedLED = Serial.read();
        
        //Turns the selected LED on
        if(selectedLED==8)
        {
          digitalWrite(pinLED,HIGH);
        }
        else
        {
          changeLED(selectedLED,ON);
        }

        //Incremenet the turn number
        turnCount++;

        //Incremements the LED selected for the other player in the boardstate
        boardState[!player1][selectedLED]++;

        //It is now this player's turn
        myTurn = true;
        
        //If the other player has won then reset the game
        if (CheckWin(!player1)) 
        {
          resetGame();
        }
        //If the turn count is 9 then the game is a tie and reset the game
        else if(turnCount==9)
        {
          resetGame();
        }
      }
    }

  //Flash all currently selected LEDs
  flashLEDs(selectedLED);
  
}

/**
 * Plays a victory song when this player wins the game
 */
void playWinningTone()
{
  tone(11,500,200); 
  tone(11,600,300); 
  tone(11,800,200); 
  tone(11,500,1000); 
}

/**
 * Resets the game, either when the game is won or when the game is a tie
 */
void resetGame()
{
  //Iterates through the board state and resets the selected LEDs
  for(int i = 0; i < 9; i++)
  {
    boardState[0][i] = 0;
    boardState[1][i] = 0;
  }
  
  //Turns all the LEDs off
  for(int i = 0;i<8;i++)
  {
    changeLED(i,OFF);
  }
  digitalWrite(8,LOW);

  //Resets the turn count and the currently selected LED
  turnCount = 0;
  selectedLED = 0;
}

/**
 * Function that checks whether or not the player passed to it has won the game
 */
bool CheckWin(int player)
{
  //Checks around different LEDs to check if they have any three selected in a row
  if(boardState[player][0]==1)
  {
    if(boardState[player][1]==1&&boardState[player][2]==1)
    {
      return true;
    }
    if(boardState[player][3]==1&&boardState[player][6]==1)
    {
      return true;
    }
    if(boardState[player][4]==1&&boardState[player][8]==1)
    {
      return true;
    }
  }
  if(boardState[player][5]==1)
  {
    if(boardState[player][2]==1&&boardState[player][8]==1)
    {
      return true;
    }
    if(boardState[player][3]==1&&boardState[player][4]==1)
    {
      return true;
    }
  }
  if(boardState[player][7]==1)
  {
    if(boardState[player][6]==1&&boardState[player][8]==1)
    {
      return true;
    }
    if(boardState[player][4]==1&&boardState[player][1]==1)
    {
      return true;
    }
  }
  if(boardState[player][2]==1&&boardState[player][4]==1&&boardState[player][6]==1)
  {
    return true;
  }
  return false;
}

/**
 * Flashes the currently selected LEDs based on which player selected them
 */
void flashLEDs(int selectedLED) {

  //Stores the current time for LED flashing purposes
  unsigned long currentTime = millis();  

  //If it is has been over 500 milliseconds since the LEDs selected by player 2 were last toggled
  if (currentTime - lastPulseTime > 500) {

    //Set the last time that the LEDs were toggled to the current time
    lastPulseTime = currentTime;

    //Iterates through every LED
    for (int i = 0; i < 9; i++) {
      //If the LED has been selected by player 2 then toggle it off or on based on toggleOn
      if (boardState[0][i] > 0) {
        if (i == 8) {
          if (toggleOn) {
            digitalWrite(i, HIGH);
          } else {
            digitalWrite(i, LOW);
          }
        } else {
          if (toggleOn) {
            changeLED(i, ON);
          } else {
            changeLED(i, OFF);
          }
        }
      }
    }

    //Sets the next toggle to be the opposite of this one
    toggleOn = !toggleOn;
  }

  //If the last time the LEDs were turned on was more than 70 milliseconds ago
  if (currentTime - lastSelectedPulseTime > 70) {
    //Iterates through every LED
    for (int i = 0; i < 9; i++) {
      //If the LED has been selected by player 1, turn it on
      if (boardState[1][i] > 0) {
        if (i == 8) {
          digitalWrite(i, HIGH);
        } else {        
          changeLED(i, ON);
        }
      }
    }
  }
  
}

/**
 * Toggles the LED passed to function
 */
void pulseSelected(int selectedLED) {

  //Stores the current time for the purpose of timing the pulse
  unsigned long currentTime = millis();

  //If it has been over 70 milliseconds since the last toggle of the LED
  if (currentTime - lastSelectedPulseTime > 70) {

    //Stores the last time the LED was toggled as the current time
    lastSelectedPulseTime = currentTime;
    
    //Toggles the LED on or off based on the toggledSelected variable
    if (selectedLED == 8) {
      if (toggleSelected) {
        digitalWrite(selectedLED, HIGH);
      } else {
        digitalWrite(selectedLED, LOW);
      }
    } else {
      if (toggleSelected) {
        changeLED(selectedLED, ON);
      } else {
        changeLED(selectedLED, OFF);
      }
    }
    //Changes toggleSelected so that the next time the function is called it will do the opposite i.e. turn off instead of on
    toggleSelected = !toggleSelected;
  }
     
}

/**
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
/**
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
