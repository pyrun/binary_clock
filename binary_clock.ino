#include "DCF77.h"
#include "Time.h" 


#define DCF_PIN 2            // Connection pin to DCF 77 device
#define DCF_INTERRUPT 1      // Interrupt number associated with pin

char time_s[9];
char date_s[11];

time_t time;
DCF77 DCF = DCF77(DCF_PIN,DCF_INTERRUPT);

//Pin connected to ST_CP of 74HC595
int latchPin = 8;
//Pin connected to SH_CP of 74HC595
int clockPin = 12;
////Pin connected to DS of 74HC595
int dataPin = 11;



byte dataArrayBinnary[10];

void setup() {
  //Start Serial for debuging purposes  
  Serial.begin(9600);
  //set pins to output because they are addressed in the main loop
  pinMode(latchPin, OUTPUT);
  
  DCF.Start();
  Serial.println("Warte auf Zeitsignal ... ");
  Serial.println("Dies kann 2 oder mehr Minuten dauern.");
}


char test[7];
int test_zahl;

char *setzahl(int a) {
  char *test;
  switch( a) {
    case 1: test = "10000000"; break;
    case 2: test = "01000000"; break;
    case 3: test = "11000000"; break;
    case 4: test = "00100000"; break;
    case 5: test = "10100000"; break;
    case 6: test = "01100000"; break;
    case 7: test = "11100000"; break;
    case 8: test = "00010000"; break;
    case 9: test = "10010000"; break;
    default:
    case 0: test = "00000000"; break;
  }
  return test;
}

char *getrow( int a) {
  char *test;
  switch( a) {
    default:
    case 0: test = "10000000"; break;
    case 1: test = "10000000"; break;
    case 2: test = "01000000"; break;
    case 3: test = "00100000"; break;
    case 4: test = "00010000"; break;
    case 5: test = "00001000"; break;
    case 6: test = "00000100"; break;
  }
  return test;
}

unsigned char gethex( char *binary_str) {
  unsigned char hex_num = 0;
  char *p = binary_str;
   
  for (int i = 0; *p != '\0'; ++p, ++i)
  {
      if (*p == '1' )
      { hex_num |= (1 << i); }
  }
  return hex_num;
}


int row = 1;

char* sprintTime() {
    snprintf(time_s,sizeof(time_s),"%.2d:%.2d:%.2d" , hour(), minute(), second());
    time_s[strlen(time_s)] = '\0';
    return time_s;
}
 
char* sprintDate() {
    snprintf(date_s,sizeof(date_s),"%.2d.%.2d.%.4d" , day(), month(), year());
    date_s[strlen(date_s)] = '\0';
    return date_s;
}

void loop() {
  test_zahl = 0;
  
  char *binary_str;

  binary_str = setzahl( row);

  unsigned char hex_data = gethex( binary_str);
  
  unsigned char hex_row = gethex( getrow(row) );

  //ground latchPin and hold low for as long as you are transmitting
  digitalWrite(latchPin, 0);
  //count up on GREEN LEDs
  shiftOut(dataPin, clockPin,  hex_row); 
  //count down on RED LEDs
  shiftOut(dataPin, clockPin,  hex_data);
  //return the latch pin high to signal chip that it 
  //no longer needs to listen for information
  digitalWrite(latchPin, 1);
  delay( 2);

/*  time_t DCFtime = DCF.getTime(); // Nachschauen ob eine neue DCF77 Zeit vorhanden ist
  if (DCFtime!=0)
  {
    setTime(DCFtime); //Neue Systemzeit setzen
    Serial.print("Neue Zeit erhalten : "); //Ausgabe an seriell
    Serial.print(sprintTime()); 
    Serial.print("  "); 
    Serial.println(sprintDate());   
  }*/
  
  row++;
  if(row == 7)
    row = 1;
}


// the heart of the program
void shiftOut(int myDataPin, int myClockPin, byte myDataOut) {
  // This shifts 8 bits out MSB first, 
  //on the rising edge of the clock,
  //clock idles low

  //internal function setup
  int i=0;
  int pinState;
  pinMode(myClockPin, OUTPUT);
  pinMode(myDataPin, OUTPUT);

  //clear everything out just in case to
  //prepare shift register for bit shifting
  digitalWrite(myDataPin, 0);
  digitalWrite(myClockPin, 0);

  //for each bit in the byte myDataOut�
  //NOTICE THAT WE ARE COUNTING DOWN in our for loop
  //This means that %00000001 or "1" will go through such
  //that it will be pin Q0 that lights. 
  for (i=7; i>=0; i--)  {
    digitalWrite(myClockPin, 0);

    //if the value passed to myDataOut and a bitmask result 
    // true then... so if we are at i=6 and our value is
    // %11010100 it would the code compares it to %01000000 
    // and proceeds to set pinState to 1.
    if ( myDataOut & (1<<i) ) {
      pinState= 1;
    }
    else {  
      pinState= 0;
    }

    //Sets the pin to HIGH or LOW depending on pinState
    digitalWrite(myDataPin, pinState);
    //register shifts bits on upstroke of clock pin  
    digitalWrite(myClockPin, 1);
    //zero the data pin after shift to prevent bleed through
    digitalWrite(myDataPin, 0);
  }

  //stop shifting
  digitalWrite(myClockPin, 0);
}
