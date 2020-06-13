/*
  iDrink - Automatic bartender
  Reza Raji
  Started 9/7/16
 */

#include <SPI.h>
#include <Ethernet.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>

#define DEBUG              // comment this line out to disable debug code

// Pick only ONE of the following to define the ingredients and menu (comment out the other one)
//#define ADULT
#define KIDS

#ifdef DEBUG                 // Shorthand for debug tracing
 #define DEBUG_PRINT(x)     Serial.print (x)
 #define DEBUG_PRINTDEC(x)     Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x) 
#endif

#define REQ_BUF_SZ   125

// define the light-strip (60 RGB LED Adafruit type) on pin 52 of Arduino
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 38, NEO_GRB + NEO_KHZ800);

// Enter a MAC address and IP address for the controller below.
// The IP address will be dependent on the local network:
byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
//IPAddress ip(192, 168, 1, 151);
IPAddress ip(192, 168, 8, 100);
EthernetServer server(80);

File webFile;

char HTTP_req[REQ_BUF_SZ] = {0};             // buffered HTTP request stored as null terminated string
char req_index = 0;                          // index into HTTP_req buffer
char file_name_requested[REQ_BUF_SZ] = {0};  // buffer the hold the file name of the http requested file
char api_command[REQ_BUF_SZ] = {0};          // buffer the hold the incoming iDrink api command from web page
char recipeName[30] = {0};                   // Recipe name passed from the client

// Liquid pump assignments

#ifdef ADULT
const int VODKA             = 1;
const int TEQUILA           = 2;
const int RUM               = 3;
const int ORANGE            = 4;
const int LIME              = 5;
const int GRAPEFRUIT        = 6;
const int POMEGRANATE       = 7;
const int PINEAPPLE         = 8;
#endif

#ifdef KIDS
const int STRAWBERRY        = 1;
const int PEACH             = 2;
const int LIME              = 3;
const int MANGO             = 4;
const int PASSIONFRUIT      = 5;
const int BLACKBERRY        = 6;
const int RASPBERRY         = 7;
const int WATER             = 8;
#endif

// Drink recipes
// Can have as many recipies as desired using the ingredients available
// Each recipe[x] is an array of up to eight ingredients and the amount (16 array entries max)
// Odd entries are liquid code; even entries are the pour for the liquid in (1/10th of oz resolution) 
// Unused pumps/ingredients should be assigned a "0"

#ifdef ADULT
//int COSMOPOLITAN[16]          = {VODKA, 15, CRANBERY, 30, LIME, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int TEQUILA_POMEGRANATE[16]   = {TEQUILA, 20, POMEGRANATE, 21, LIME, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int VODKA_POMEGRANATE[16]     = {VODKA, 20, POMEGRANATE, 21, LIME, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int VODKA_SHOT[16]            = {VODKA, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int TEQUILA_SHOT[16]          = {TEQUILA, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int SCREWDRIVER[16]           = {VODKA, 20, ORANGE, 25,0,0,0,0,0,0,0,0,0,0,0,0};
int GREYHOUND[16]             = {VODKA, 20, GRAPEFRUIT, 20, LIME, 3,0,0,0,0,0,0,0,0,0,0};
int TEQUILA_SUNRISE[16]       = {TEQUILA, 20, POMEGRANATE, 10, ORANGE, 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int SKINNY_MARGARITA[16]      = {TEQUILA, 20, ORANGE, 15, LIME, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int RUM_PINEAPPLE[16]         = {RUM, 15, PINEAPPLE, 25, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int RUM_PUNCH[16]             = {RUM, 10, ORANGE, 10, PINEAPPLE, 10, LIME, 5, 0, 0, 0, 0, 0, 0, 0, 0};
int MENLO_PARK_ICED_TEA[16]   = {RUM, 6, VODKA, 6, TEQUILA, 6, LIME, 3, POMEGRANATE, 6, PINEAPPLE, 6, 0, 0, 0, 0};
#endif

#ifdef KIDS
int STRAWBERRY_COOLER[16]            = {STRAWBERRY, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int PEACH_COOLER[16]                 = {PEACH, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int LIME_COOLER[16]                  = {LIME, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int MANGO_COOLER[16]                 = {MANGO, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int PASSIONFRUIT_COOLER[16]          = {PASSIONFRUIT, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int BLACKBERRY_COOLER[16]            = {BLACKBERRY, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int RASPBERRY_COOLER[16]             = {RASPBERRY, 6, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int MANGO_LIME_COOLER[16]            = {MANGO, 3, LIME, 3, WATER, 60, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
int OH_BOY[16]                       = {STRAWBERRY, 2, PEACH, 2, LIME, 2, MANGO, 2, PASSIONFRUIT, 2, BLACKBERRY, 2, RASPBERRY, 2, WATER, 50};
#endif

//int PUMP_SEQUENCE_TEST[16]    = {VODKA,1,TEQUILA,2,GIN,3,SIMPLE_SYRUP,4,ORANGE,5,CRANBERY,6,POMEGRANATE,7,LIME,8};


// Pump I/O assignments
// 8 pumps, each with a forward and Reverse relay control. First two numbers are F and R for first pump. And so on.
const int pumpPinsRelayArray[16] = {22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37};   // Pairs of pump control output pins (pumps 1-8)

const int DRINK_SIZE_FACTOR = 1.0;     // Scale the drink up or down in size. 1 = actual specified ounce units
const float PUMP_POUR_RATE = 286;      // How many milliseconds for a pump to pour 1/10th of an ounce of liquid
const int END_POUR_SUCTION = 50;       // Reverse suction at the end of a pour in milliseconds

const int OFF = 0;
const int ON = 1;
const int FORWARD = 1;
const int REVERSE = 2;

int pumpsON [8] = {0,0,0,0,0,0,0,0}; // flags for the pump that are turned on


// the setup routine runs on reset
void setup() {

    // Initialize the LED light strip
    strip.begin();
    strip.show(); // Initialize all pixels to 'off'
  
      // disable Ethernet chip
    pinMode(10, OUTPUT);
    digitalWrite(10, HIGH);
    
    #ifdef DEBUG
      Serial.begin(9600);       // for debugging
    #endif
    
    // initialize SD card
    DEBUG_PRINTLN("Initializing SD card...");
    //Serial.println("Initializing SD card...");
    if (!SD.begin(4)) {
        DEBUG_PRINTLN("ERROR - SD card initialization failed!");
        return;    // init failed
    }
    DEBUG_PRINTLN("SUCCESS - SD card initialized.");
    // check for index.htm file
    if (!SD.exists("index.htm")) {
        DEBUG_PRINTLN("ERROR - Can't find index.htm file!");
        return;  // can't find index file
    }
    DEBUG_PRINTLN("SUCCESS - Found index.htm file.");
  
  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  DEBUG_PRINT("server is at ");
  DEBUG_PRINTLN(Ethernet.localIP());
  
  // initialize the 16 digital pump pin as an output
  for (int x=0; ((x<=15)&&(pumpPinsRelayArray[x]!=0)); x++) { // configure pin as outut unless it's assigned a "0"
    pinMode(pumpPinsRelayArray[x], OUTPUT); 
  }
}

/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
/////////////////////////////////
// the loop routine repeats forever
void loop() {


//    dotMoveLeft(strip.Color(255,0,0), 20);

    // listen for incoming clients
    EthernetClient client = server.available();


    if (client) {  // got client?
        boolean currentLineIsBlank = true;
        while (client.connected()) {
            if (client.available()) {   // client data available to read
                char c = client.read(); // read 1 byte (character) from client
                // buffer first part of HTTP request in HTTP_req array (string)
                // leave last element in array as 0 to null terminate string (REQ_BUF_SZ - 1)
                if (req_index < (REQ_BUF_SZ - 1)) {
                    HTTP_req[req_index] = c;          // save HTTP request character
                    req_index++;
                }
                // print HTTP request character to serial monitor
                //Serial.print(c);
                // last line of client request is blank and ends with \n
                // respond to client only after last line received
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header                    
                    client.println("HTTP/1.1 200 OK");
                    // remainder of header follows below, depending on if
                    // web page, image or XML page is requested
                    

                    // iDrink Ajax request to pour a recipe
                    if (StrContains(HTTP_req, "iDrink_pour_recipe")) {
                        DEBUG_PRINTLN();
                        DEBUG_PRINTLN();
                        DEBUG_PRINT("IDRINK AJAX recipe command handler. HTTP_req= ");
                        DEBUG_PRINTLN(HTTP_req);                      
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                       
                        if (StrContains(HTTP_req, "recipe=")) { 
                          StrRecipeRcvd(HTTP_req);                        // Extract the recipe name from HTTP request
                          DEBUG_PRINT("Recipe chosen = ");
                          DEBUG_PRINTLN(String(recipeName));            
                          String recipeString = String(recipeName);

                          #ifdef ADULT
                          if (recipeString == "TEQUILA_POMEGRANATE") pourRecipe(TEQUILA_POMEGRANATE);
                          if (recipeString == "VODKA_POMEGRANATE") pourRecipe(VODKA_POMEGRANATE);
                          if (recipeString == "VODKA_SHOT") pourRecipe(VODKA_SHOT);
                          if (recipeString == "TEQUILA_SHOT") pourRecipe(TEQUILA_SHOT);
                          if (recipeString == "SCREWDRIVER") pourRecipe(SCREWDRIVER);
                          if (recipeString == "GREYHOUND") pourRecipe(GREYHOUND);
                          if (recipeString == "TEQUILA_SUNRISE") pourRecipe(TEQUILA_SUNRISE);
                          if (recipeString == "SKINNY_MARGARITA") pourRecipe(SKINNY_MARGARITA);
                          if (recipeString == "RUM_PINEAPPLE") pourRecipe(RUM_PINEAPPLE);
                          if (recipeString == "RUM_PUNCH") pourRecipe(RUM_PUNCH);
                          if (recipeString == "MENLO_PARK_ICED_TEA") pourRecipe(MENLO_PARK_ICED_TEA);
                          #endif

                          #ifdef KIDS
                          if (recipeString == "STRAWBERRY") pourRecipe(STRAWBERRY_COOLER);
                          if (recipeString == "PEACH") pourRecipe(PEACH_COOLER);
                          if (recipeString == "LIME") pourRecipe(LIME_COOLER);
                          if (recipeString == "MANGO") pourRecipe(MANGO_COOLER);
                          if (recipeString == "PASSIONFRUIT") pourRecipe(PASSIONFRUIT_COOLER);
                          if (recipeString == "BLACKBERRY") pourRecipe(BLACKBERRY_COOLER);
                          if (recipeString == "RASPBERRY") pourRecipe(RASPBERRY_COOLER);
                          if (recipeString == "MANGO_LIME") pourRecipe(MANGO_LIME_COOLER);
                          if (recipeString == "OH_BOY") pourRecipe(OH_BOY);
                          #endif
                         
                          //pourRecipe (*('recipeName'));            // If you're here then there is a recipe name starting with "*" and ending with ".". Return the recipe name
                        }
                        // send XML file containing input states
                        //XML_response(client);
                        
                    }


                    
                    // iDrink Ajax request to control pumps
                    if (StrContains(HTTP_req, "iDrink_pump_control")) {
                        DEBUG_PRINTLN();
                        DEBUG_PRINTLN();
                        DEBUG_PRINT("IDRINK AJAX pump command handler. HTTP_req= ");
                        DEBUG_PRINTLN(HTTP_req);                      
                        // send rest of HTTP header
                        client.println("Content-Type: text/xml");
                        client.println("Connection: keep-alive");
                        client.println();
                        SetPumps();
                        // send XML file containing input states
                        //XML_response(client);
                    }

                    // open requested web page file
                    else if (StrContains(HTTP_req, "GET / ")
                                 || StrContains(HTTP_req, "GET /index.htm")) {
                        DEBUG_PRINTLN();
                        DEBUG_PRINTLN();
                        DEBUG_PRINT("IDRINK index.htm file handler. HTTP_req= ");
                        DEBUG_PRINTLN(HTTP_req);
                        client.println("HTTP/1.1 200 OK");
                        client.println("Content-Type: text/html");
                        client.println("Connection: close");
                        client.println();
                        webFile = SD.open("index.htm");        // open web page file
                    }

                    
                    // generic GET to handle  GET file requests for HTM and JPG
                    else if (StrContains(HTTP_req, "GET /") &&  (StrContains(HTTP_req, ".htm") || StrContains(HTTP_req, ".jpg"))){
                        DEBUG_PRINTLN();
                        DEBUG_PRINTLN();
                        DEBUG_PRINT("IDRINK HTTP file handler. HTTP_req= ");
                        DEBUG_PRINTLN(HTTP_req);
                        //delay(1000);
                        client.println("HTTP/1.1 200 OK");
                        
                        if (StrContains(HTTP_req, ".htm")){       // its not an image file
                          client.println("Content-Type: text/html");
                          client.println("Connection: close");
                        }
                        
                        client.println();
                        //Serial.println(HTTP_req);
                        StrFileNameRcvd(HTTP_req);        // Get the file name requested and open that file and put in "file_name_requested"
                        DEBUG_PRINT("File name requested = ");
                        DEBUG_PRINTLN(file_name_requested);
                        webFile = SD.open(file_name_requested);     // open web page file
                    } 


                    if (webFile) {
                        while(webFile.available()) {
                            client.write(webFile.read()); // send web page to client
                        }
                        webFile.close();
                    }
                    // reset buffer index and all buffer elements to 0
                    req_index = 0;
                    StrClear(HTTP_req, REQ_BUF_SZ);
                    StrClear(file_name_requested, REQ_BUF_SZ);
                    StrClear(api_command, REQ_BUF_SZ);
                    break;
                }
                // every line of text received from the client ends with \r\n
                if (c == '\n') {
                    // last character on line of received text
                    // starting new line with next character read
                    currentLineIsBlank = true;
                } 
                else if (c != '\r') {
                    // a text character was received from client
                    currentLineIsBlank = false;
                }
            } // end if (client.available())
        } // end while (client.connected())
        delay(1);      // give the web browser time to receive the data
        client.stop(); // close the connection
    } // end if (client)

//    dotMoveRight(strip.Color(0,0,255), 20);
}

// sets every element of str to 0 (clears array)
void StrClear(char *str, char length)
{
    for (int i = 0; i < length; i++) {
        str[i] = 0;
    }
}



// Extract and return the file name string in the incoming string
// "file_name_requested" is defined as the name after "/" and before the first space (e.g. "GET /page2.htm ")
void StrFileNameRcvd(char *str){
  int index = 0;
  int index2 = 0;
  
  while (str[index] != '/'){
    index++;
  }
  index++;  // skip the "/"
      //Serial.println(index);
  
  while (str[index] != ' '){
    file_name_requested[index2] = str[index];
    index++;
    index2++;
  }
  file_name_requested[index2] = '\0';    // last entry in array should be null
  //Serial.println(index);             
  //Serial.println(file_name_requested);
}
  


// Extract the recipe name in the incoming string
// Recipe name is after the "*" and before the "." (e.g. "*VODKA_POMEGRANATE.")
void StrRecipeRcvd(char *str){
    int index = 0;
    int index2 = 0;
  
  while (str[index] != '*'){
    index++;
  }
  index++;  // skip the "*"
  
  while (str[index] != '.'){
    recipeName[index2] = str[index];
    index++;
    index2++;
  }
  recipeName[index2] = '\0';    // last entry in array should be null
}

  

// searches for the string sfind in the string str
// returns 1 if string found
// returns 0 if string not found
char StrContains(char *str, const char *sfind)
{
    char found = 0;
    char index = 0;
    char len;

    len = strlen(str);
    
    if (strlen(sfind) > len) {
        return 0;
    }
    while (index < len) {
        if (str[index] == sfind[found]) {
            found++;
            if (strlen(sfind) == found) {
                return 1;
            }
        }
        else {
            found = 0;
        }
        index++;
    }
    return 0;
}
  
  
  

// checks if received HTTP request is controlling pumps (FORWARD, REVERSE, OFF)
void SetPumps(void) {
  
    if (StrContains(HTTP_req, "pump1=F")) drivePump (1, FORWARD);
    if (StrContains(HTTP_req, "pump2=F")) drivePump (2, FORWARD);  
    if (StrContains(HTTP_req, "pump3=F")) drivePump (3, FORWARD);
    if (StrContains(HTTP_req, "pump4=F")) drivePump (4, FORWARD);  
    if (StrContains(HTTP_req, "pump5=F")) drivePump (5, FORWARD);
    if (StrContains(HTTP_req, "pump6=F")) drivePump (6, FORWARD);  
    if (StrContains(HTTP_req, "pump7=F")) drivePump (7, FORWARD);
    if (StrContains(HTTP_req, "pump8=F")) drivePump (8, FORWARD);  

    if (StrContains(HTTP_req, "pump1=R")) drivePump (1, REVERSE);
    if (StrContains(HTTP_req, "pump2=R")) drivePump (2, REVERSE);  
    if (StrContains(HTTP_req, "pump3=R")) drivePump (3, REVERSE);
    if (StrContains(HTTP_req, "pump4=R")) drivePump (4, REVERSE);  
    if (StrContains(HTTP_req, "pump5=R")) drivePump (5, REVERSE);
    if (StrContains(HTTP_req, "pump6=R")) drivePump (6, REVERSE);  
    if (StrContains(HTTP_req, "pump7=R")) drivePump (7, REVERSE);
    if (StrContains(HTTP_req, "pump8=R")) drivePump (8, REVERSE);  

    if (StrContains(HTTP_req, "pump1=S")) drivePump (1, OFF);
    if (StrContains(HTTP_req, "pump2=S")) drivePump (2, OFF);  
    if (StrContains(HTTP_req, "pump3=S")) drivePump (3, OFF);
    if (StrContains(HTTP_req, "pump4=S")) drivePump (4, OFF);  
    if (StrContains(HTTP_req, "pump5=S")) drivePump (5, OFF);
    if (StrContains(HTTP_req, "pump6=S")) drivePump (6, OFF);  
    if (StrContains(HTTP_req, "pump7=S")) drivePump (7, OFF);
    if (StrContains(HTTP_req, "pump8=S")) drivePump (8, OFF);  
}




// Pour a drink recipe with upto 8 liquid ingredients poured in parallel
// recipe[x] is an array of up to eight ingredients and the amount (oz) of each (16 array entries max)
// Unused pumps/ingredients should be assigned a "0"
// Example: Cosmopolitan recipe array is: [VODKA,10,CRANBERY,20,LIME,3,0,0,0,0,0,0,0,0,0,0]

// Notes: This routine does not return UNTIL the drink has finished pouring. So can not do other activity (e.g. lights ontrol)
// Might try using timer interrupts (but may not have enough of them)

void pourRecipe (int recipe[]) { 
  
  for (int x=0; x<=7 ; x++) { // Reset the pump ON flag array
    pumpsON[x] = OFF;
  }

  
  // Turn on all the specified liquid pumps "at once"  
  for (int x=0; ((recipe[x] != 0) && (x<15)) ; x=x+2) { // stop when you hit a null recipe ingredient or if you go through all 8 ingredients in recipe
    //Serial.println(x);
    drivePump (recipe[x], FORWARD);
    pumpsON[x/2] = ON;      // turn on the flag for the pump being turned on
  }
  // Turn off each pump based on it's specified pour amount
  unsigned long timeEpoc = millis();  //get current system millisecond passed since power up
  
  while ((pumpsON[0]+pumpsON[1]+pumpsON[2]+pumpsON[3]+pumpsON[4]+pumpsON[5]+pumpsON[6]+pumpsON[7]) != 0) { // while there is a pump still on
  for (int x=0; x<= 7; x++) {                                                                              // go through each pump and check the lapsed time vs recipe time/pour
    if (pumpsON[x] != OFF){
      if (millis() >= (timeEpoc + (recipe[((x*2)+1)]*PUMP_POUR_RATE*DRINK_SIZE_FACTOR))) { // If enough time has passed for a pump pour, turn OFF that pump
        drivePump (recipe[x*2], OFF); // Turn off pump
        pumpsON[x] = OFF; // also turn off its ON flag
      }
    }
  }
  }
    


  /*
  // Now do a brief back suction on all the pumps in this recipe to make sure there are no risidual after drips
  for (int i=1 ; recipe[i] != 0 ; i=i+2) {
      drivePump (recipe[i], REVERSE); // Turn backwards briefly to create a suction (so no drip)
      delay (END_POUR_SUCTION); // Very short delay
      drivePump (recipe[i], OFF); // Finally turn off pump
  }
  */
}


// Pump control function 
// Valid actions are: FORWARD, REVERSE, OFF. Any other value turns pump OFF
//pumpNum is from 1-8
// LOW output turn on the relay
void drivePump (int pumpNum, int pumpAction) {
  switch (pumpAction) {
    case OFF:       //turn pump off
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-2], HIGH); // find the I/O pins from pin index array and control the two relays
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-1], HIGH);
      break;
    case FORWARD:   //Pump in forward mode
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-2], LOW);
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-1], HIGH);
      break;
    case REVERSE:   //Pump in reverse mode
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-2], HIGH);
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-1], LOW);
      break;
    default:        //turn pump offby default
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-2], HIGH);
      digitalWrite(pumpPinsRelayArray[(pumpNum*2)-1], HIGH);
      break;
  }
}


// Move a dot left
void dotMoveLeft(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
    strip.setPixelColor(i, c);
    strip.setPixelColor(i-1, 0);
    if (i==0) strip.setPixelColor(strip.numPixels()-1, 0);
    strip.show();
    delay(wait);
  }
}

// Move a dot right
void dotMoveRight(uint32_t c, uint8_t wait) {
  for(uint16_t i=strip.numPixels(); i>0; i--) {
    strip.setPixelColor(i-1, c);
    strip.setPixelColor(i, 0);
    if (i==strip.numPixels()) strip.setPixelColor(strip.numPixels()-1, 0);
    strip.show();
    delay(wait);
  }
}

  
/*
// Pour a specified liquid for the specified amount
void pourLiquid (int pumpNum, int ounce) {
  drivePump (pumpNum, FORWARD); // Turn on the specified liquid/pump
  delay (ounce * PUMP_POUR_RATE * DRINK_SIZE_FACTOR); // Delay the specified amount
  drivePump (pumpNum, OFF); // Then turn off pump
  drivePump (pumpNum, REVERSE); // Then turn backwards briefly to create a suction (so no drip)
  delay (END_POUR_SUCTION); // Very short delay
  drivePump (pumpNum, OFF); // Finally turn off pump
}
*/

// Determine the pump flow rate based on a timed pour and user feedback of the amount
void calibratePump() {
}


