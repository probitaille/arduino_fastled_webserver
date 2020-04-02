/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/
//#define FASTLED_ESP8266_NODEMCU_PIN_ORDER
#define FASTLED_ESP8266_RAW_PIN_ORDER
//#define FASTLED_ALLOW_INTERRUPTS 0
#define FASTLED_INTERRUPT_RETRY_COUNT 1 //See: https://github.com/FastLED/FastLED/wiki/Interrupt-problems

#include <FastLED.h>

// Load Wi-Fi library
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>


// Replace with your network credentials
const char* ssid     = "VIRUS";
const char* password = "3nd0fW0rld";

//#define BRIGHTNESS  255
#define LED_PIN     5
#define COLOR_ORDER GRB
#define NUM_LEDS    150 //150 leds total

// Array of leds
CRGB  leds[NUM_LEDS];

/****** Wifi server variables ******/
// Set web server port number to 80
WiFiServer server(80);

ESP8266WebServer webServer(80); // Create a webserver object that listens for HTTP request on port 80
ESP8266HTTPUpdateServer httpUpdateServer;


// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String animation_state = "off";
bool is_changing_state = false;


// Assign output variables to GPIO pins
const int output5 = 5;
const int output4 = 4;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 500;

//Default Brightness
uint8_t Brightness = 255;

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

    /******* LED Init ******/
    
  FastLED.addLeds<WS2812B, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  Brightness );

  currentPalette = RainbowColors_p;
  currentBlending = LINEARBLEND;

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

  
  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  
 



  httpUpdateServer.setup(&webServer);

  webServer.on("/", handleRoot);               // Call the 'handleRoot' function when a client requests URI "/"

  webServer.on("/off", HTTP_GET, []() {
    Serial.println("Leds turned off");
    turnOffLeds();
              
    String json = "Leds turned off";
    webServer.send(200, "text/json", json);
  });

  webServer.on("/rainbow", HTTP_GET, []() {
    Serial.println("Rainbow activated");
    changePalette(RainbowColors_p, "Rainbow activated");
  });
  
  webServer.on("/lava", HTTP_GET, []() {
    Serial.println("Lava activated");
    changePalette(LavaColors_p, "Lava activated");
  });

  webServer.on("/ocean", HTTP_GET, []() {
    Serial.println("Ocean activated");
    changePalette(OceanColors_p, "Ocean activated");
  });

  webServer.on("/clouds", HTTP_GET, []() {
    Serial.println("Clouds activated");
    changePalette(CloudColors_p, "Clouds activated");
  });

  webServer.on("/forest", HTTP_GET, []() {
    Serial.println("Forest activated");
    changePalette(ForestColors_p, "Forest activated");
  });

  webServer.on("/party", HTTP_GET, []() {
    Serial.println("Party activated");
    changePalette(PartyColors_p, "Party activated");
  });

  webServer.on("/brightness", HTTP_POST, []() {

    Serial.print("Data received: ");
    String data = webServer.arg("plain");
    Serial.println(data);
  
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3) + 30;
    DynamicJsonDocument doc(capacity);

    deserializeJson(doc, data);

    uint8_t value = doc["value"];
    
    Serial.println(value);

    setBrightness(value);
          
    String json = "Brightness set to " + value;
    webServer.send(200, "text/json", json);
  });

  webServer.on("/setcolor", HTTP_POST, []() {
    Serial.print("Data received: ");
    String data = webServer.arg("plain");
    Serial.println(data);
  
    const size_t capacity = JSON_ARRAY_SIZE(2) + JSON_OBJECT_SIZE(3) + 30;
    DynamicJsonDocument doc(capacity);

    animation_state = "off";

    deserializeJson(doc, data);

    uint8_t r = doc["r"];
    uint8_t g = doc["g"];
    uint8_t b = doc["b"];
    
    setColor(r, g, b);

    Serial.println("r = ");
    Serial.println(r);
    Serial.println("g = ");
    Serial.println(g);
    Serial.println("b = ");
    Serial.println(b);

    String json = "Color set to (" + String(r) + "," + String(g) + "," + String(b) + ")" ;
    webServer.send(200, "text/json", json);
  });

//RainbowColors_p, RainbowStripeColors_p,
// OceanColors_p, CloudColors_p, LavaColors_p, ForestColors_p, and PartyColors_p.

  webServer.onNotFound(handleNotFound);        // When a client requests an unknown URI (i.e. something other than "/"), call function "handleNotFound"
  webServer.begin();                           // Actually start the server

  
  Serial.println("HTTP server started");

  // Add service to MDNS-SD
  MDNS.addService("http", "tcp", 80);
  
  Serial.println("Setup Done!");
}

void handleRoot() {
  webServer.send(200, "text/plain", "I'm alive!");   // Send HTTP status 200 (Ok) and send some text to the browser/client
}

void handleNotFound(){
   Serial.println("handleNotFound");
  webServer.send(404, "text/plain", "404: Not found"); // Send HTTP status 404 (Not Found) when there's no handler for the URI in the request
}


void loop(){

  MDNS.update();
  

  /**** ChangePalettePeriodically Script ****/

  webServer.handleClient(); // Listen for HTTP requests from clients

  unsigned long currentTime = millis(); // refresh counter variable

  
  
  if (animation_state=="on") {
      //ChangePalettePeriodically();

      if(currentTime - previousTime > 10 )
      {
        previousTime = millis();
        
        static uint8_t startIndex = 0;
        startIndex = startIndex + 1; // motion speed
  
        
        FillLEDsFromPaletteColors( startIndex);
      
        FastLED.show();
        
        // insert a delay to keep the framerate modest
        //FastLED.delay(1000 / UPDATES_PER_SECOND);
      }
     
      /*
      Serial.println( "FreeHeap:" );
      Serial.println( ESP.getFreeHeap() );
      Serial.println( "HeapFragmentation:" );
      Serial.println( ESP.getHeapFragmentation() );
      Serial.println( "MaxFreeBlockSize:" );
      Serial.println( ESP.getMaxFreeBlockSize() );
      */
  }
  /*
  if(currentTime - previousTime > 1000)
  {
      previousTime = millis();
      
      Serial.println( "FreeHeap:" );
      Serial.println( ESP.getFreeHeap() );
      Serial.println( "HeapFragmentation:" );
      Serial.println( ESP.getHeapFragmentation() );
      Serial.println( "MaxFreeBlockSize:" );
      Serial.println( ESP.getMaxFreeBlockSize() );
    
  }*/

  if( is_changing_state == true ) {
    //if(currentTime - previousTime > timeoutTime)
    //{
    //  previousTime = millis();
      
      is_changing_state = false;
      Serial.println("is_changing_state set to false");
    //}
  } 
}// End Loop

void setBrightness(uint8_t value)
{
  if (value > 255)
    value = 255;
  else if (value < 0) value = 0;

  Brightness = value;

  FastLED.setBrightness(Brightness);

  Serial.println("Brightness set to:");
  Serial.println(Brightness);
}


void setColor(uint8_t r, uint8_t g, uint8_t b)
{
  Serial.println("setColor() called");   
  
  fill_solid( leds, NUM_LEDS, CRGB(r,g,b));
  FastLED.show();

  Serial.println("turnOffLeds() finish");   
}

void changePalette(CRGBPalette16 new_palette, String success_msg)
{
  //Change only if it's not currently in changing state 
  if(is_changing_state == false)
  {
    animation_state = "on";
    is_changing_state = true;
    currentPalette = new_palette;
    digitalWrite(output5, HIGH);
              
    String json = success_msg;
    webServer.send(200, "text/json", json);
  }
  else
  {
    Serial.println("Cant change palette, alreay in changing state mode...");
  }
}


void FillLEDsFromPaletteColors( uint8_t colorIndex)
{
  for ( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = ColorFromPalette( currentPalette, colorIndex, Brightness, currentBlending);
    colorIndex += 3;
  }
}


void turnOffLeds()
{
  Serial.println("turnOffLeds() called");   
  animation_state = "off";
  
  fill_solid( leds, NUM_LEDS, CRGB(0,0,0));
  FastLED.show();

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
