/*
Adafruit Arduino - Lesson 3. RGB LED
*/
#include <SoftwareSerial.h>
#include <Stepper.h>

const int stepsPerRevolution = 200;

Stepper myStepper = Stepper(stepsPerRevolution, 8, 9, 10, 11);

SoftwareSerial wifiSerial(2, 3); //RX, TX for ESP 8266

bool DEBUG = true;
int responseTime = 10; //communication timeout
 
int redPin = 13;
int greenPin = 12;
int bluePin = 7;
int beep = 4;
 
//uncomment this line if using a Common Anode LED
//#define COMMON_ANODE
 
void setup()
{
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT); 
  pinMode(beep, OUTPUT); 

  myStepper.setSpeed(100);

  Serial.begin(115200);
  while(!Serial)
  {
    ; //wait for serial port to connect. Needed for Leonardo only
  }
  wifiSerial.begin(115200);
  while (!wifiSerial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  sendToWifi("AT+CWMODE=2",responseTime,DEBUG); // configure as access point
  sendToWifi("AT+CIFSR",responseTime,DEBUG); // get ip address
  sendToWifi("AT+CIPMUX=1",responseTime,DEBUG); // configure for multiple connections
  sendToWifi("AT+CIPSERVER=1,80",responseTime,DEBUG); // turn on server on port 80
 
  sendToUno("Wifi connection is running!",responseTime,DEBUG);
}
 
void loop()
{
  if(Serial.available()>0)
  {
     String message = readSerialMessage();
     if(find(message,"debugEsp8266:"))
     {
      String result = sendToWifi(message.substring(13,message.length()),responseTime,DEBUG);
      if(find(result,"OK"))
        sendData("\nOK");
      else
        sendData("\nEr");
    }
  }    
    if(wifiSerial.available()>0)
    {
      String message = readWifiSerialMessage();

      // sendToUno(message, responseTime, DEBUG);
 
      if(find(message,"esp8266:"))
      {
         String result = sendToWifi(message.substring(8,message.length()),responseTime,DEBUG);
        if(find(result,"OK"))
          sendData("\n"+result);
        else
          sendData("\nErrRead");               //At command ERROR CODE for Failed Executing statement
      }
      
      else
      {
        if(find(message,"open"))
        {  //Open door
            for (int i = 0; i < 3; i++)
            {
              setColor(0, 0, 255);  // red
              digitalWrite(beep, HIGH);
              delay(100);
              digitalWrite(beep, LOW);
              delay(100);
              digitalWrite(beep, HIGH);
              delay(100);
              digitalWrite(beep, LOW);
              setColor(0, 0, 0); //off
              delay(300);
            }
            for (int j = 0; j < 2; j++)
            {
              myStepper.step(500);
            }              
        }
        else if(find(message, "close"))
        {  //Close door
          for (int i = 0; i < 3; i++)
            {
              setColor(0, 0, 255);  // red
              digitalWrite(beep, HIGH);
              delay(1000);
              digitalWrite(beep, LOW);
              setColor(0, 0, 0); //off
              delay(1000);
            }
        }
        else if(find(message, "LED WHITE"))
        {
          delay(200);
          setColor(255, 255, 255);
        }
        else if(find(message, "HI"))
        {
          sendToUno("Hello",responseTime,DEBUG);
          sendData("Hello");
        }
        /*
        else
        {
          sendData("\nErrRead");                 //Command ERROR CODE for UNABLE TO READ
        }
      }
  */
      }
  delay(responseTime);
  }
}
 
void setColor(int red, int green, int blue)
{
  #ifdef COMMON_ANODE
    red = 255 - red;
    green = 255 - green;
    blue = 255 - blue;
  #endif
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue);  
}

/*
* Name: sendData
* Description: Function used to send string to tcp client using cipsend
* Params: 
* Returns: void
*/
void sendData(String str){
  String len="";
  len+=str.length();
  sendToWifi("AT+CIPSEND=0,"+len,responseTime,DEBUG);
  delay(100);
  sendToWifi(str,responseTime,DEBUG);
  delay(100);
  sendToWifi("AT+CIPCLOSE=5",responseTime,DEBUG);
}


/*
* Name: find
* Description: Function used to match two string
* Params: 
* Returns: true if match else false
*/
boolean find(String string, String value){
  return string.indexOf(value)>=0;
}


/*
* Name: readSerialMessage
* Description: Function used to read data from Arduino Serial.
* Params: 
* Returns: The response from the Arduino (if there is a reponse)
*/
String  readSerialMessage(){
  char value[100]; 
  int index_count =0;
  while(Serial.available()>0){
    value[index_count]=Serial.read();
    index_count++;
    value[index_count] = '\0'; // Null terminate the string
  }
  String str(value);
  str.trim();
  return str;
}



/*
* Name: readWifiSerialMessage
* Description: Function used to read data from ESP8266 Serial.
* Params: 
* Returns: The response from the esp8266 (if there is a reponse)
*/
String  readWifiSerialMessage(){
  char value[100]; 
  int index_count =0;
  while(wifiSerial.available()>0){
    value[index_count]=wifiSerial.read();
    index_count++;
    value[index_count] = '\0'; // Null terminate the string
  }
  String str(value);
  str.trim();
  return str;
}



/*
* Name: sendToWifi
* Description: Function used to send data to ESP8266.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToWifi(String command, const int timeout, boolean debug){
  String response = "";
  Serial.println(command);
  wifiSerial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The esp has data so display its output to the serial window 
    char c = wifiSerial.read(); // read the next character.
    response+=c;
    }  
  }
  if(debug)
  {
    Serial.println(response);
  }
  return response;
}

/*
* Name: sendToUno
* Description: Function used to send data to Arduino.
* Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
* Returns: The response from the esp8266 (if there is a reponse)
*/
String sendToUno(String command, const int timeout, boolean debug){
  String response = "";
  Serial.println(command); // send the read character to the esp8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(Serial.available())
    {
      // The esp has data so display its output to the serial window 
      char c = Serial.read(); // read the next character.
      response+=c;
    }  
  }
  if(debug)
  {
    Serial.println(response);
  }
  return response;
}
