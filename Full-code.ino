#include "WiFi.h" // We need this library to connect to WiFi
#include "ESPAsyncWebServer.h" // We need this library to make a webpage (e.g. Web-App)
#include "AsyncTCP.h"
#include <LiquidCrystal_I2C.h>

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;

// set LCD address, number of columns and rows
// if you don't know your display address, run an I2C scanner sketch
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);  


// Defining the names of variables that we will use later
int val = 0;

String UserAnswer; // This string will hold the answer to the question
String Current_Time; // This string will hold our current time
String Alarm_Set;  // This string will hold the time we set for the alarm to go off
String Sch;
int buzzer = 18;  // Buzzer is connected to pin 18 of the ESP32 Board
int t=1;
int p=0;



// Replace with your network credentials
const char* ssid = "wifi_name"
const char* password = "wifi_password";

// Setting up real-time clock
const char* NTP_SERVER = "ch.pool.ntp.org";
const char* TZ_INFO    = "CET-1CEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00";  // enter your time zone (https://remotemonitoringsystems.ca/time-zone-abbreviations.php)
tm timeinfo;
time_t now;
long unsigned lastNTPtime;
unsigned long lastEntryTime;

// My input from web-app to the arduino
const char* PARAM_INPUT_1 = "input1";
const char* PARAM_INPUT_A1 = "Answer1";
const char* PARAM_INPUT_A2 = "Answer2";
const char* PARAM_INPUT_A3 = "Answer3";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

// Here we made a function that seperates the string so we dont read space or :
String getValue(String data, char separator, int index)
{
    int maxIndex = data.length() - 1;
    int j = 0;
    String chunkVal = "";

    for (int i = 0; i <= maxIndex && j <= index; i++)
    {
        chunkVal.concat(data[i]);

        if (data[i] == separator)
        {
            j++;

            if (j > index)
            {
                chunkVal.trim();
                return chunkVal;
            }

            chunkVal = "";
        }
        else if ((i == maxIndex) && (j < index)) {
            chunkVal = "";
            return chunkVal;
        }
    }  
}

// Here we made a function get real time clock :
bool getNTPtime(int sec) {

  {
    uint32_t start = millis();
    do {
      time(&now);
      localtime_r(&now, &timeinfo);
      delay(10);
     
    } while (((millis() - start) <= (1000 * sec)) && (timeinfo.tm_year < (2016 - 1900)));
    if (timeinfo.tm_year <= (2016 - 1900)) return false;  // the NTP call was not successful

    char time_output[30];
    strftime(time_output, 30, "%a  %d-%m-%y %T", localtime(&now));
  }
  return true;
}

// This function prints real time clock on serial monitor (used for testing)
void showTime(tm localTime) {
//  Serial.println('\n');
//  Serial.print(localTime.tm_hour + 6);
//  Serial.print(':');
//  Serial.print(localTime.tm_min);
//  Serial.print(':');
//  Serial.println(localTime.tm_sec);
}

// Get the Hour
String Hour(tm localTime) {
   int t=localTime.tm_hour -6;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
  //  Serial.println(t);
    if (t<0){
      t=24+t;
    }
    return String(t);
  }
}


// Get the Minutes
String HH(tm localTime) {
  int t=localTime.tm_hour;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
   // Serial.println(t);
    return String(t);
  }
}

// Get the Minutes
String Minutes(tm localTime) {
  int t=localTime.tm_min;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
   // Serial.println(t);
      if (t<10){
        return ("0"+String(t));
      }
      else{
    return String(t);
      }
  }
}

// Get the Seconds
String Seconds(tm localTime) {
  int t=localTime.tm_sec;
  if (isnan(t)) {    
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  }
  else {
  //  Serial.println(t);
    return String(t);
  }
}

// ******************************************************************** HTML CSS CODE STARTS HERE *************************************************************************************** //
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<body>

<!DOCTYPE HTML><html lang="en">
<head>
<body leftmargin="0" topmargin="0" marginwidth="0" marginheight="0">

  <title> Eur-e-clock</title>
  <h1>
<img src="https://upload.wikimedia.org/wikipedia/commons/3/37/Clock.gif" alt="Clock" style="float:left;width:35px;height:35px;">
    EUR-E-CLOCK
</h1> 
   
  <p> &nbsp An Alarm Clock that wakes you up everyday with a new information </p>
         
  <hr>
  
  <meta name="keywords" content="Imane Yacoubi, Eur-e-clock, Alarm Clock, Alarm, Clock">
  <meta name="description" content="ALARM CLOCK WEB INTERFACE, EUR-E-CLOCK, ALARM TO WAKE UP">
  <meta name="author" content="Imane Yacoubi">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  
  
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <link rel="icon" href="https://cdn-icons-png.flaticon.com/512/109/109613.png"> 
  
</head>
<div class="main">
  <br/> <br/>
  <div class="clock">
  <h1 >
    <i class=" " style=""></i>
    <span id="HOURS">%HOURS%</span> :
        <span id="MINUTES">%MINUTES%</span> :
            <span id="SECONDS">%int(SECONDS)%</span>
            <br>
            
  </h1>
  </div>
  
  <div class="setclock">
  <h2 style="text-align:center">
  Set your EUR-E-CLOCK:
  </h2>
  </div>
 
  <form action="/get" style="text-align:center">
   <label > </label>
   <input type="time" id="inputText" name="input1" size="400" onchange="timers(this.value)"  />
    <input type="submit" value="SET">
  </form>

<br><br><br>

 <h3 style="font-family:BankGothic Lt BT;text-align:center">
 WHAT IS THE CAPITAL OF MOROCCO?</h3>
<div class="q">
  <form action="/getAnswer">
   <input type="radio" id="Answer1" name="Answer1" value="Monaco">
   <label for="Answer1">Monaco</label><br>
   <input type="radio" id="Answer2" name="Answer2" value="Rabat">
   <label for="Answer2">Rabat</label><br>
   <input type="radio" id="Answer3" name="Answer3" value="Marrakesh">
   <label for="Answer3">Marrakesh </label>
   <br><br>
   <div class="submit">
    <input type="submit" value="SUBMIT">
   </div>   
  </form>
</div>
  <br><br><br><br><br><br><br><br><br><br><br>
</div>
  
       
   <div class= "END">
  <p style="font-family:BankGothic Lt BT;text-align:center">
  <br> Made by <a href="https://www.linkedin.com/in/imane-yacoubi-054a02216/" title="LinkedIn" > Imane Yacoubi </a><br><br> 
  Special thanks to Dr. Younes Sadat-Nejad for his help on this project <br><br></p>


   </div>
</body>
<style>
    html {
     font-family: BankGothic Lt BT;
     display: inline-block;
     margin: 0px auto;

    }
    
    .main {
     background-image: url(https://static.vecteezy.com/system/resources/previews/003/604/057/non_2x/digital-wall-box-white-background-with-black-grid-space-line-color-surface-network-cyber-technology-banner-cover-terrain-sci-fi-wireframe-and-related-to-background-vector.jpg);
     background-repeat: no-repeat;
     background-size: cover;
   
     }

    h2 { font-size: 3.0rem; }
    p { font-size: 1.1rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
     
    }
    
      
a:link {
color: white;
text-decoration: underline;
}
  a:visited {
  color: pink;
}
    a:hover {
    color: red;
}
    
    .q {
  text-align: justify;
  margin: 0 auto;
  width: 12em;
  font-size: 20px
         }
         
    .END {
    background-color: black;
    color: white;
   }

    input[type=text], select {
 
  display: inline-block;
  border: 1px solid #ccc;
  padding: 15px 25px;
  box-sizing: border-box;  
 
}

input[type=submit] {
   cursor: pointer;
   font-family:BankGothic Lt BT;
   border-radius: 2px;
   color: black;
   padding: 6px 20px;
   color: white;
   background-color: black;
   display: inline-block;
   border: none;
  font-weight: bold;
  font-size: 16px;
  transition-duration: 0.4s
   
}
input[type=submit]:hover {
  background-color: white;
  color: black;
  border: 2px solid #555555;
  }
  
  .submit{
          margin-left: 40px;
          }
.clock {
     text-align: center;
     font-size: 45px
                       

  </style>
<script>

// ****In BODY instead of %HOURS% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
//  Serial.println(xhttp);
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("HOURS").innerHTML = this.responseText;
      }
  };
  xhttp.open("GET", "/HOURS", true);
  xhttp.send();
}, 1000 ) ;

// ****In BODY instead of %MINUTES% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("MINUTES").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/MINUTES", true);
  xhttp.send();
}, 500 ) ;

// ****In BODY instead of %SECONDS% put a value getting from the function **** //
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("SECONDS").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/SECONDS", true);
  xhttp.send();
}, 10 ) ;


</script>
</html>)rawliteral";
// ******************************************************************** HTML CSS CODE ENDS HERE *************************************************************************************** //

// Provoding the Hour or Min or Sec time
String processor(const String& var){
  if(var == "HOURS"){
    return Hour(timeinfo);
  }
  else if(var == "MINUTES"){
    return Minutes(timeinfo);
  }
  else if(var == "SECONDS"){
    return Seconds(timeinfo);
  }
  return String();
}

void setup(){
  Serial.begin(115200);

   lcd.init();
  // turn on LCD backlight                      
  lcd.backlight();
  ledcSetup(0,1E5,12); // Setup for buzzer on ESP32
  ledcAttachPin(buzzer,0); // Setup for buzzer on ESP32
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting...");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP()); // Print the wifi IP ("Website link") on serial monitor

  // Time Configuration
  configTime(0, 0, NTP_SERVER);
  setenv("TZ", TZ_INFO, 1);
  if (getNTPtime(10)) {  // wait up to 10sec to sync
  } else {
  //  Serial.println("Time not set");
    ESP.restart();
  }
  showTime(timeinfo);
  lastNTPtime = time(&now);
  lastEntryTime = millis();
 

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/HOURS", HTTP_GET, [](AsyncWebServerRequest *request){
  //  Serial.println((Hour(timeinfo)).toInt());
    request->send_P(200, "text/plain", (Hour(timeinfo)).c_str());
  });
  server.on("/MINUTES", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Minutes(timeinfo).c_str());
  });
  server.on("/SECONDS", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", Seconds(timeinfo).c_str());
  });

// ******************************************************* HERE WE GET THE INPUT FROM THE WEBSITE (ALARM SET) ********************************************************************** //
  server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
    String inputMessage;
    String inputParam;
    if (request->hasParam(PARAM_INPUT_1)) {
      inputMessage = request->getParam(PARAM_INPUT_1)->value(); // HERE WE ARE GETTING THE ALARM SET FROM THE WEBSITE
      inputParam = PARAM_INPUT_1;
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
     Serial.println(inputMessage); // print the alarm on serial port
     
     Current_Time=Hour(timeinfo) +':'+Minutes(timeinfo) ;
     Alarm_Set=inputMessage; // SAVE THE SET ALARM IN A STRING CALLED "ALARM_SET"
     t=1;
     p=0;
      });


  server.on("/getAnswer", HTTP_GET, [] (AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
    String inputMessage1;
    String inputMessage2;
    String inputMessage3;
    String inputParam1;
    String inputParam2;
    String inputParam3;
    if (request->hasParam(PARAM_INPUT_A1)) {
      inputMessage1 = request->getParam(PARAM_INPUT_A1)->value();
      inputParam1 = PARAM_INPUT_A1;
    }
    if (request->hasParam(PARAM_INPUT_A2)) {
      inputMessage2 = request->getParam(PARAM_INPUT_A2)->value();
      inputParam2 = PARAM_INPUT_A2;
    }
    if (request->hasParam(PARAM_INPUT_A3)) {
      inputMessage3 = request->getParam(PARAM_INPUT_A3)->value();
      inputParam3 = PARAM_INPUT_A3;
    }

   
     Serial.print(" Answer to A1 : ");
     Serial.println(inputMessage1); // print the answer on serial port
     
     Serial.print(" Answer to A2 : ");
     Serial.println(inputMessage2); // print the answer on serial port
     
     Serial.print(" Answer to A3 : ");
     Serial.println(inputMessage3); // print the answer on serial port
     UserAnswer = inputMessage2;
     
   
      });

// ************************************************************************************************************************************************************************** //

  // Start server
  server.begin();
}
char Time[ ] = "TIME:00:00:00";

void loop(){
  getNTPtime(10);
  showTime(timeinfo);
// From "ALARM_SET" variable get the hour, and minute
   String Hour = getValue(Alarm_Set, ':', 0);
  Hour = getValue(Hour, ':', 0);
   String Minz=getValue(Alarm_Set, ':', 1);
   int H;
   H= Hour.toInt();
   char buf[12]; // "-2147483648\0"

  lcd.clear();
  
  lcd.setCursor(6, 0);
     lcd.print(itoa(HH(timeinfo).toInt()+18, buf, 10));

lcd.setCursor(8, 0);
  lcd.print( ":");
  char buf2[12]; 

  lcd.setCursor(9, 0);
  lcd.print( itoa(Minutes(timeinfo).toInt(), buf2, 10));
     Serial.println(Time);
     
     lcd.setCursor(3,1);
lcd.print("EUR-E-CLOCK");

 
// ******************************************************* HERE IS THE CONDITION WHEN THE CURRENT TIME MATCHES ALAM SET ********************************************************************** //
        int count = 0;


  if(getValue(Alarm_Set, ':', 0).toInt()!=0){ // If ALARM_SET variabel is not empty
    if(getValue(Alarm_Set, ':', 0).toInt()==getValue(Current_Time, ':', 0).toInt()){ // If Hour is same
      if(getValue(Alarm_Set, ':', 1).toInt()==Minutes(timeinfo).toInt()){ // If Minute is same
        while (t==1){ // If this is first time happening
          count = count+1;
          
          if (UserAnswer == "Rabat"){
            t=0;
          }
          if (count==10000){
            t=0;
          }

         
           delay(10);

          ledcWriteTone(0,800);
          uint8_t octave = 1;
          delay(10);
         ledcWriteNote(0,NOTE_C,octave);   // Make the buzzer sound
         if (t==0){
          ledcWriteTone(0,80000); // Turn off the buzzer
         }

                   }
      }
    }
  }
// ************************************************************************************************************************************************************************** //


  delay(200);
}

// Thank you for having a look, I wish it helped in some way! - Imane :)) 
