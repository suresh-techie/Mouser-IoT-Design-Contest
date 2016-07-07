               //MOUSER IOT DESIGN CONTEST
        //SMART DRIVER e-MONITORING AND ALERT SYSTEM
     
#include <twApi.h>
#include <twLogger.h>
#include <twOSPort.h>
#include <WiFi.h>
#include <stdio.h>
#include <string.h>
#include <Wire.h>
#include <LiquidCrystal.h>
#include <Intel_Edison_BT_SPP.h>
#define trigPin 8
#define echoPin 9
#define Buzzer 13

char ssid[] = "wifi_network_name";    
char pass[] = "wifi_password"; 
int status = WL_IDLE_STATUS;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Intel_Edison_BT_SPP spp = Intel_Edison_BT_SPP();
String readdata;

/* Name of thing created in cloud */
char * thingName = "Smart_Driver";

/* IP/Hostname of server */
char * serverName = "cloud_name.com";

/* port */
int port = 80;

/* API key created */
char * apiKey = "key_name";

/* refresh rate */
int timeBetweenRefresh = 1000;


/* Parameters to be measured */
struct  
{  double Steer;  
   double Speed;
}
properties;



void sendPropertyUpdate(){
  /* Create the property list */  
  propertyList * proplist = twApi_CreatePropertyList("Steer", twPrimitive_CreateFromNumber(properties.Steer), 0);
  
  if (!proplist) 
  {
    TW_LOG(TW_ERROR, "sendPropertyUpdate: Error allocating property list");
    return;
  }
  
  twApi_AddPropertyToList(proplist, "Speed", twPrimitive_CreateFromNumber(properties.Speed), 0);
  twApi_PushProperties(TW_THING, thingName, proplist, -1, FALSE);
  twApi_DeletePropertyList(proplist);
}


void dataCollectionTask()
{
  
 // Assigning the corresponding ports in board to properties created in code

 // Any sensor can be used with regard to the purpose used

 // Do calibrations with values accordingly
 
 int sensor_value = analogRead(A0);
 float voltage;
 voltage = (float)sensor_value*5/1023;
 float a = (voltage*300)/5;
 delay(500);
 properties.Steer = a ; 

 int b = analogRead(A1);
 delay(500);
 properties.Speed = b ;
 
 Serial.print("Time:");
 Serial.print(millis());
 
 Serial.print("Steering control=");
 Serial.print(properties.Steer, 1);
 Serial.print(" ");
 
 Serial.print("Car Speed= ");
 Serial.print(properties.Speed, 1);
 Serial.print(" ");
 Serial.println();
 
 sendPropertyUpdate();
}


/*****************
 * Property Handler Callbacks 
 ******************/

enum msgCodeEnum propertyHandler(const char * entityName, const char * propertyName,  twInfoTable ** value, char isWrite, void * userdata)
{
  char * asterisk = "*";
  if (!propertyName) propertyName = asterisk;
  
  TW_LOG(TW_TRACE, "propertyHandler - Function called for Entity %s, Property %s", entityName, propertyName);
  
  if (value) 
  {

    if (strcmp(propertyName, "Steer") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Steer);
    else
    if (strcmp(propertyName, "Speed") == 0) *value = twInfoTable_CreateFromNumber(propertyName, properties.Speed);
    else 
    return TWX_NOT_FOUND;
    return TWX_SUCCESS;
  }
  else 
  {
    TW_LOG(TW_ERROR, "propertyHandler - NULL pointer for value");
    return TWX_BAD_REQUEST;
  }
}


void setup() 
{
    int err=0;
    /* Open serial connection */
    Serial.begin(9600);
    lcd.init(1,12,255,11,5,4,3,2,0,0,0,0);
    delay(500);
    lcd.begin(16, 2);
    /* Wait for someone to launch the console */
    delay(1000);

    Serial.println("Intel Edison BT SPP test!");
    spp.open();
  
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(trigPin, OUTPUT);
    pinMode(echoPin, INPUT);
    pinMode(Buzzer, OUTPUT);
}

    /* Setup the WiFi connection */
    if (WiFi.status() == WL_NO_SHIELD) 
    {
    Serial.println("WiFi is not connected"); 
    // don't continue:
    while(true);
    } 

 // attempt to connect to Wifi network:
  if ( status != WL_CONNECTED) 
    { 
    Serial.print("Wifi is connecting to ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:    
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
    }
    Serial.println("You're connected to the network");
    Serial.println("SMART DRIVER e-MONITORING AND ALERT SYSTEM");
    delay(500);
    Serial.println("Intel Edison communicating with cloud");
    delay(1000);

    err = twApi_Initialize(serverName, port, TW_URI, apiKey, NULL, MESSAGE_CHUNK_SIZE, MESSAGE_CHUNK_SIZE, TRUE);
    if (err) {
    Serial.println("Error initializing the API");
   }

  /* Allow self signed certs */
     twApi_SetSelfSignedOk();
  /* Regsiter our properties */
     twApi_RegisterProperty(TW_THING, thingName, "Steer", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler,NULL);
     twApi_RegisterProperty(TW_THING, thingName, "Speed", TW_NUMBER, NULL, "ALWAYS", 0, propertyHandler,NULL);
     /* Bind our thing */
     twApi_BindThing(thingName);

  /* Connecting to server */
   if (!twApi_Connect(CONNECT_TIMEOUT, CONNECT_RETRIES)) 
    {
      Serial.println("Sucessefully connected to cloud!");
    delay(1000);
    }

}


void loop()
{
  // put your main code here, to run repeatedly:
  ssize_t size = spp.read();
   if (size != -1) {
    Serial.println(spp.getBuf());
  } else {
    // Well, some error here 
    delay(500);
    while (spp.available()){  //Check if there is an available byte to read
  delay(10); //Delay added to make thing stable
  char c = spp.read(); 
  readdata += c; //build the string- "forward", "reverse", "left" and "right"
  } 
  if (readdata.length() > 0) {
    Serial.println(readdata);

  if(readdata == "forward")
  {
    digitalWrite(3, HIGH);
    digitalWrite (4, HIGH);
    digitalWrite(5,LOW);
    digitalWrite(6,LOW);
    delay(100);
  }

  else if(readdata == "reverse")
  {
    digitalWrite(3, LOW);
    digitalWrite(4, LOW);
    digitalWrite(5, HIGH);
    digitalWrite(6,HIGH);
    delay(100);
  }

  else if (readdata == "right")
  {
    digitalWrite (3,HIGH);
    digitalWrite (4,LOW);
    digitalWrite (5,LOW);
    digitalWrite (6,LOW);
    delay (100);
   
  }

 else if ( readdata == "left")
 {
   digitalWrite (3, LOW);
   digitalWrite (4, HIGH);
   digitalWrite (5, LOW);
   digitalWrite (6, LOW);
   delay (100);
 }

 else if (readdata == "stop")
 {
   digitalWrite (3, LOW);
   digitalWrite (4, LOW);
   digitalWrite (5, LOW);
   digitalWrite (6, LOW);
   delay (100);
 }
readdata="";}

//Ultrasonic Code
long duration, distance;
digitalWrite(trigPin, LOW);
delayMicroseconds(2);
digitalWrite(trigPin, HIGH);
delayMicroseconds(10);
digitalWrite(trigPin, LOW);

duration = pulseIn(echoPin, HIGH);

distance = (duration/2) / 29.1;

if (distance < 30)
{
digitalWrite(Buzzer,HIGH); //less than 30cm then buzzer will produce beep sound
Serial.println("WARNING!!!");
Serial.println("DROWSY DRIVING!!!");
lcd.setCursor(0,0); 
lcd.print("WARNING!!!");
delay(500);
lcd.setCursor(0,1);
lcd.print("DROWSY DRIVING!!!");
delay(500);
}
else {
digitalWrite(Buzzer,LOW);
}

if (distance >= 200 || distance <= 0)
{
Serial.println("SAFE DRIVING!!!");
lcd.setCursor(0,0); 
lcd.print("SAFE DRIVING!!!");
}
else {
Serial.print(distance);
Serial.println(" cm");
}
delay(100);
//Reset the variable
delay(1000);
dataCollectionTask();
}


