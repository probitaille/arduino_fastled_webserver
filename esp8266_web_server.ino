/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0

#include <FastLED.h>

// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


// Replace with your network credentials
const char* ssid     = "VIRUS";
const char* password = "3nd0fW0rld";

#define BRIGHTNESS  50
#define LED_PIN     5
#define COLOR_ORDER GRB
#define TOTAL_LEDS  150 //150 leds total
#define NUM_LEDS    150 //150 leds totalCRGB  leds[TOTAL_LEDS];

// Array of leds
CRGB  leds[TOTAL_LEDS];

/****** Wifi server variables ******/
// Set web server port number to 80
WiFiServer server(80);

ESP8266WebServer webServer(80); // Create a webserver object that listens for HTTP request on port 80
ESP8266HTTPUpdateServer httpUpdateServer;


// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String led_state = "off";
String lavaWaveState = "off";

// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

bool blink = false;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 5000;


/**** Rainbow variable ****/
#define UPDATES_PER_SECOND 100

//Variable to store color palette
CRGBPalette16 currentPalette;

//Variable to store color transition
TBlendType    currentBlending;

// ???
extern CRGBPalette16 myRedWhiteBluePalette;
extern const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM;


/**** Begin of scripts *****/

void setup() 
{
  delay( 500 ); // power-up safety delay

  /******* Rainbow Setup ******/
    
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;
  
  /******* LED Init ******/
  // put your setup code here, to run once:
  //FastLED.addLeds<WS2812B, LED_PIN, GRB>(leds, TOTAL_LEDS);


  /******* Wifi Setup ******/
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output5, OUTPUT);
  pinMode(output4, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output5, LOW);
  digitalWrite(output4, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected to:");
  Serial.println(WiFi.SSID());              // Tell us what network we're connected to
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println();
  Serial.print( F("Heap: ") ); Serial.println(system_get_free_heap_size());
  Serial.print( F("Boot Vers: ") ); Serial.println(system_get_boot_version());
  Serial.print( F("CPU: ") ); Serial.println(system_get_cpu_freq());
  Serial.print( F("SDK: ") ); Serial.println(system_get_sdk_version());
  Serial.print( F("Chip ID: ") ); Serial.println(system_get_chip_id());
  Serial.print( F("Flash ID: ") ); Serial.println(spi_flash_get_id());
  Serial.print( F("Flash Size: ") ); Serial.println(ESP.getFlashChipRealSize());
  Serial.print( F("Vcc: ") ); Serial.println(ESP.getVcc());
  Serial.println();

  
  if (MDNS.begin("esp8266")) {              // Start the mDNS responder for esp8266.local
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
  MDNS.addService("http", "tcp", 80);



  httpUpdateServer.setup(&webServer);

  webServer.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"

  webServer.on("/rainbow", HTTP_GET, []() {
    Serial.println("Rainbow activated");
    led_state = "on";
    currentPalette = RainbowColors_p;
    digitalWrite(output5, HIGH);
              
    String json = "Rainbow activated";
    webServer.send(200, "text/json", json);
  });
  
  webServer.on("/lava", HTTP_GET, []() {
    Serial.println("Lava activated");
    led_state = "on";
    currentPalette = LavaColors_p;
    digitalWrite(output5, HIGH);
              
    String json = "test";
    webServer.send(200, "text/json", json);
  });

  webServer.on("/ocean", HTTP_GET, []() {
    Serial.println("Ocean activated");
    led_state = "on";
    currentPalette = OceanColors_p;
    digitalWrite(output5, HIGH);
              
    String json = "test";
    webServer.send(200, "text/json", json);
  });


//RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.

  webServer.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"

  webServer.begin();                           // Actually start the server
  Serial.println("HTTP server started");
  
  Serial.println("Setup Done!");
}

void handleRoot() {
  webServer.send(200, "text/plain", "Hello world!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
   Serial.println("handleNotFound");
  webServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void loop(){

  /**** ChangePalettePeriodically Script ****/

  webServer.handleClient(); // Listen for HTTP requests from clients
    MDNS.update();

  
  if (led_state=="on") {
      //ChangePalettePeriodically();
    
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; // motion speed
    
      FillLEDsFromPaletteColors( startIndex);
    
      FastLED.show();
      FastLED.delay(1000 / UPDATES_PER_SECOND);
  }

  /*
  if (lavaWaveState=="on") {
      static uint8_t startIndex = 0;
      startIndex = startIndex + 1; // motion speed
    
      FillLEDsFromPaletteColors( startIndex);

      FastLED.show();
      FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
  */

  /**** Wifi Script ****/

  /*
  WiFiClient client = server.available();   // Listen for incoming clients
  
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            turnOffAllStates();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /rainbow") >= 0) {
              Serial.println("GPIO 5 on");
              //changeLedToRed();
              output5State = "on";
              currentPalette = RainbowColors_p;
              digitalWrite(output5, HIGH);
            } else if (header.indexOf("GET /off") >= 0) {
              Serial.println("GPIO 5 off");
              turnOffLeds();
              digitalWrite(output5, LOW);
            } else if (header.indexOf("GET /lava") >= 0) {
              Serial.println("Lava Wave On");
              lavaWaveState = "on";
              currentPalette = LavaColors_p;
              digitalWrite(output5, HIGH);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #195B6A; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #77878A;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP8266 Web Server</h1>");

            client.println("<p>Commands:</p>");
              client.println("<p><a href=\"/off\"><button class=\"button\">Turn OFF</button></a></p>");
            
            // If the output5State is off, it displays the ON button       
            if (output5State=="off") {
              client.println("<p>Rainbow - <b style='color: gray;'>Deactivated</p>");
              client.println("<p><a href=\"/rainbow\"><button class=\"button\">Turn rainbow ON</button></a></p>");
            } else {
              client.println("<p>Rainbow - <b style='color: green;'>Activated</p>");
              //client.println("<p><a href=\"/5/off\"><button class=\"button button2\" >Turn rainbow OFF</button></a></p>");
            } 
               
            // Display current state, and ON/OFF buttons for GPIO 4  
            
            // If the lavaWaveState is off, it displays the ON button       
            if (lavaWaveState=="off") {
              client.println("<p>Lava wave - <b style='color: gray;'>Deactivated</p>");
              client.println("<p><a href=\"/lava\"><button class=\"button\">Turn Lava wave ON</button></a></p>");
            } else {
              // Display current state, and ON/OFF buttons for GPIO 4  
              client.println("<p>Lava wave - <b style='color: green;'>Activated</p>");
              //client.println("<p><a href=\"/4/off\"><button class=\"button button2\" >>Turn Lava wave OFF</button></a></p>");
            }
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    //delay(1000); // execute once every 1 sec, don't flood remote service

  } //End if (client) */
  
}// End Loop

void test()
{
    blink = true;

}

void turnOffAllStates()
{
  led_state = "off";
  lavaWaveState = "off";
}

void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  uint8_t brightness = 255;

  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, brightness, currentBlending);
    colorIndex += 3;
  }
}


void changeLedToBlue()
{
  Serial.println("changeLedToBlue() called");   
  
  for (int j = 0; j < NUM_LEDS ; j++)
  {
    // clear this led for the next time around the loop
    leds[j] = CRGB(0,0,255);
    FastLED.show();
    
  }
}

void changeLedToRed()
{
  Serial.println("changeLedToRed() called");   
  
  for (int j = 0; j < NUM_LEDS ; j++)
  {
    // clear this led for the next time around the loop
    leds[j] = CRGB::Red;
    FastLED.show();
  }
}

void turnOffLeds()
{
  Serial.println("turnOffLeds() called");   
  led_state = "off";
  lavaWaveState = "off";
  
  blink = false;

  fill_solid( leds, NUM_LEDS, CRGB(0,0,0));
  FastLED.show();

  /*
  for (int j = 0; j < NUM_LEDS ; j++)
  {
    // clear this led for the next time around the loop
    leds[j] = CRGB::Black;
    FastLED.show();
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }*/

  Serial.println("turnOffLeds() finish");   
}


/**** Rainbow Effect ****/

// There are several different palettes of colors demonstrated here.
//
// FastLED provides several 'preset' palettes: RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.
//
// Additionally, you can manually define your own color palettes, or you can write
// code that creates color palettes on the fly.  All are shown here.

void ChangePalettePeriodically()
{
  uint8_t secondHand = (millis() / 1000) % 60;
  static uint8_t lastSecond = 99;

  if ( lastSecond != secondHand) {
    lastSecond = secondHand;
    if ( secondHand ==  0)  {
      currentPalette = RainbowColors_p;
      currentBlending = LINEARBLEND;
    }

  }
}

// This function fills the palette with totally random colors.
void SetupTotallyRandomPalette()
{
  for ( int i = 0; i < 16; i++) {
    currentPalette[i] = CHSV( random8(), 255, random8());
  }
}

// This function sets up a palette of black and white stripes,
// using code.  Since the palette is effectively an array of
// sixteen CRGB colors, the various fill_* functions can be used
// to set them up.
void SetupBlackAndWhiteStripedPalette()
{
  // 'black out' all 16 palette entries...
  fill_solid( currentPalette, 16, CRGB::Black);
  // and set every fourth one to white.
  currentPalette[0] = CRGB::White;
  currentPalette[4] = CRGB::White;
  currentPalette[8] = CRGB::White;
  currentPalette[12] = CRGB::White;

}

// This function sets up a palette of purple and green stripes.
void SetupPurpleAndGreenPalette()
{
  CRGB purple = CHSV( HUE_PURPLE, 255, 255);
  CRGB green  = CHSV( HUE_GREEN, 255, 255);
  CRGB black  = CRGB::Black;

  currentPalette = CRGBPalette16(
                     green,  green,  black,  black,
                     purple, purple, black,  black,
                     green,  green,  black,  black,
                     purple, purple, black,  black );
}


// This example shows how to set up a static color palette
// which is stored in PROGMEM (flash), which is almost always more
// plentiful than RAM.  A static PROGMEM palette like this
// takes up 64 bytes of flash.
const TProgmemPalette16 myRedWhiteBluePalette_p PROGMEM =
{
  CRGB::Red,
  CRGB::Gray, // 'white' is too bright compared to red and blue
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Black,

  CRGB::Red,
  CRGB::Red,
  CRGB::Gray,
  CRGB::Gray,
  CRGB::Blue,
  CRGB::Blue,
  CRGB::Black,
  CRGB::Black
};
