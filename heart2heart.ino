/*
>> Pulse Sensor Amped 1.1 <<
copied and modified from:
This code is for Pulse Sensor Amped by Joel Murphy and Yury Gitman
    www.pulsesensor.com 
    >>> Pulse Sensor purple wire goes to Analog Pin 0 <<<
Pulse Sensor sample aquisition and processing happens in the background via Timer 2 interrupt. 2mS sample rate.
PWM on pins 3 and 11 will not work when using this code, because we are using Timer 2!
The following variables are automatically updated:
Signal :    int that holds the analog signal data straight from the sensor. updated every 2mS.
IBI  :      int that holds the time interval between beats. 2mS resolution.
BPM  :      int that holds the heart rate value, derived every beat, from averaging previous 10 IBI values.
QS  :       boolean that is made true whenever Pulse is found and BPM is updated. User must reset.
Pulse :     boolean that is true when a heartbeat is sensed then false in time with pin13 LED going out.

This code is designed with output serial data to Processing sketch "PulseSensorAmped_Processing-xx"
The Processing sketch is a simple data visualizer. 
All the work to find the heartbeat and determine the heartrate happens in the code below.
Pin 13 LED will blink with heartbeat.
If you want to use pin 13 for something else, adjust the interrupt handler
It will also fade an LED on pin fadePin with every beat. Put an LED and series resistor from fadePin to GND.
Check here for detailed code walkthrough:
http://pulsesensor.myshopify.com/pages/pulse-sensor-amped-arduino-v1dot1

Code Version 02 by Joel Murphy & Yury Gitman  Fall 2012
This update changes the HRV variable name to IBI, which stands for Inter-Beat Interval, for clarity.
Switched the interrupt to Timer2.  500Hz sample rate, 2mS resolution IBI value.
Fade LED pin moved to pin 5 (use of Timer2 disables PWM on pins 3 & 11).
Tidied up inefficiencies since the last version. 
*/


//  VARIABLES
int pulsePin = 0;                 // Pulse Sensor purple wire connected to analog pin 0
int blinkPin = 13;                // pin to blink led at each beat
int fadePin = 5;                  // pin to do fancy classy fading blink at each beat
int curBrightness = 0;            // used to fade LED on with PWM on fadePin


// these variables are volatile because they are used during the interrupt service routine!
volatile int BPM;                   // used to hold the pulse rate
volatile int Signal;                // holds the incoming raw data
volatile int IBI = 600;             // holds the time between beats, the Inter-Beat Interval
volatile boolean Pulse = false;     // true when pulse wave is high, false when it's low
volatile boolean QS = false;        // becomes true when Arduoino finds a beat.

// led fading - depends on BPM changes
int maxBright = 255;
int prevBPM = 0;
#define NBPMS 100
int prevBPMs[NBPMS];


void setup(){
  pinMode(blinkPin,OUTPUT);         // pin that will blink to your heartbeat!
  pinMode(fadePin,OUTPUT);          // pin that will fade to your heartbeat!
  Serial.begin(115200);             // we agree to talk fast!
  interruptSetup();                 // sets up to read Pulse Sensor signal every 2mS 
   // UN-COMMENT THE NEXT LINE IF YOU ARE POWERING The Pulse Sensor AT LOW VOLTAGE, 
   // AND APPLY THAT VOLTAGE TO THE A-REF PIN
   //analogReference(EXTERNAL);   
   
   initialize_array(prevBPMs, NBPMS); 
}



void loop(){
  Serial.print("\n\n\n");
  sendDataToProcessing('S', Signal);     // send Processing the raw Pulse Sensor data
  // only get every 10th value
  if (QS == true){                       // Quantified Self flag is true when arduino finds a heartbeat
        curBrightness = maxBright;                  // Set 'fadeRate' Variable to 255 to fade LED with pulse
        sendDataToProcessing('B',BPM);   // send heart rate with a 'B' prefix
        sendDataToProcessing('Q',IBI);   // send time between beats with a 'Q' prefix
        QS = false;                      // reset the Quantified Self flag for next time    
     }
  Serial.print("BPM: ");Serial.println(BPM);
  
  ledFadeToBeat(prevBPMs, NBPMS);
  
//  if (millis() % 10 == 0) {
    push_array(prevBPMs, NBPMS, BPM);
//  }
  
  delay(20);                             //  take a break
}


void ledFadeToBeat(int bpms[], int n){
    if (maxIncrease(bpms, n) > 10) {
      maxBright = 255; 
    } else {
      maxBright -= 3; 
    }
    maxBright = constrain(maxBright, 10, 255);
    Serial.print("maxBright: ");Serial.println(maxBright);
    
    curBrightness -= 3;                         //  set LED fade value
    curBrightness = constrain(curBrightness,0,255);   //  keep LED fade value from going into negative numbers!
    analogWrite(fadePin,curBrightness);          //  fade LED
}


void sendDataToProcessing(char symbol, int data ){
    Serial.print(symbol);                // symbol prefix tells Processing what type of data is coming
    Serial.println(data);                // the data to send culminating in a carriage return
}


/* our array helpers */
// assuming everything is positive
int maxIncrease(int a[], int n) {
  int diff;
  int prev_min; // minimum of all previously seen values
  for (int i = 0; i < n; i++) {
    if (i == 0) {
      prev_min = a[i];
      diff = 0;
    }
    if (a[i] < prev_min) {
      prev_min = a[i];
    }
    if (a[i] - prev_min > diff) {
      diff = a[i] - prev_min;
    }
  }
  return diff;
}

int findMaximum(int array[], int n) {
   int maximum = 0;
   for (int i = 0; i < n; i++) {
     if (array[i] > maximum) { maximum = array[i]; }
   }
   Serial.print("max: ");Serial.println(maximum);
   return maximum;
}

// assuming everything is positive, but ignoring 0s
int findMinimum(int array[], int n) {
   int minimum;
   for (int i = 0; i < n; i++) {
     if (array[i] < minimum || i == 0) { minimum = array[i]; }
   }
   Serial.print("min: ");Serial.println(minimum);
   return minimum;
}


void initialize_array(int a[], int n) {
  for (int i = 0; i < n; i++) {
     a[i] = 0; 
  }
}

void push_array(int a[], int n, int val) {
  for (int i = n-1; i >= 1; i--) {
     a[i] = a[i-1];
  } 
  a[0] = val;
  
  //print_array(a, n);
}

void print_array(int a[], int n) {
  Serial.print("array is [");
  for (int i = 0; i < n; i++) {
    Serial.print(a[i]); Serial.print(", ");
  }
  Serial.println("]");
}





