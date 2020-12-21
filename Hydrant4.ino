#include <WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_NeoPixel.h>
#define LED_PIN    25
#define ONE_WIRE_BUS 17

TaskHandle_t Task1;
 
const char *ssid = "XXXXX";
const char *password = "YYYYYYY";
IPAddress lclIP(192,168,2,110);
IPAddress gateway(192,168,2,1);
IPAddress subnet(255,255,255,0);

float tempc=0.0;
int modus=0;

WiFiServer serverWiFi(80);
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

#define LED_COUNT 500
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

void wifiSend (WiFiClient client) {  
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html"); 
  client.println();  
  client.print ("<!DOCTYPE HTML><html><head>");
  client.print ("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
  client.print("<meta http-equiv=\"refresh\" content=\"10\">");
  client.println("<link rel=\"icon\" href=\"data:,\">");
  client.print ("<title> ESP32-Webserver</title>");   
 
  // CSS-Code gestalten der Ein/Aus-Schaltfläche 
  client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
  client.println(".buttonEin { background-color: #333344; border: none; color: white; padding: 16px 40px;");
  client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");           
  client.println(".buttonAus {background-color: #888899;}");   
  client.println("</style>");
  client.println("</head>");  
 
  // Rumpf Überschrift
  client.print ("<body>");
  client.print ("<div style=\"font-size: 3.0rem;\">");
  client.print ("<h1>HoHoHo</h1> <hr>");        // hr = horizontale Linie
  client.print ("</div>");
 
  client.print ("<div style=\"font-size: 2.0rem;\">"); 
  client.print  ("<p>Weihnachtlich illuminierter Hydrant<br></p>");
  char tempString[7];
  dtostrf(tempc,6, 2, tempString);
  client.print  ("<p>Aussentemperatur: " +  String(tempString) + "<br></p>");
  String modusString="";
  if (modus==0){modusString="Besinnlich-Mode";}
  if (modus==1){modusString="Krischan-Mode";}
  
  client.println("<p>Weihnachtsmodus: " + modusString + "</p>");
  if (modus==0) {
    client.println("<p><a href=\"/H\"><button class=\"buttonEin\">Krischan-Mode</button></a></p>");
  } else {
    client.println("<p><a href=\"/L\"><button class=\"buttonEin buttonAus\">Besinnlich-Mode</button></a></p>");
  }                      
  client.println("</div></body></html>");   // schließende Tags    
  // die HTTP-Antwort endet mit einer weiteren Leerzeile
  client.println();    
}

// bearbeitet eine Anfrage von dem WiFi-Client
void wifiReceive (WiFiClient client) {
  Serial.println("Neue Anfrage.");          // Meldung im seriellen Monitor
  String currentLine = "";                  // Variable f. eingehende Daten
  while (client.connected()) {              // Loop, solange Client verbunden
    if (client.available()) {               // gibt es Bytes zu lesen   
      char c = client.read();               // 1 Byte in Variable c lesen
      Serial.print(c);                      // Ausgabe des Bytes 
      if (c == '\n') {                      // ist Zeichen newline-Zeichen ?
        // das Ende der HTTP-Anfrage ist eine Leerzeile 
        // und zwei newline-Zeichen hintereinander
        if (currentLine.length() == 0) {    // Ende HTTP-Anforderung Client        
          wifiSend(client);            
          break;                            // while beenden client.connected 
        } else {                            // liegt ein newline vor
         currentLine = "";                  // Variable currentLine löschen
        }
      } else if (c != '\r') {               // alles, nur kein Wagenrücklauf
        currentLine += c;                   // Zeichen currentLine hinzufügen
      }
      // Auswertung der Client-Anfrage     
      // war die Client-Anfrage "GET /H" or "GET /L":
      if (currentLine.endsWith("GET /H")) {
        modus=1;  
        Serial.print("Modus Umsachalten auf:");
        Serial.println(modus);
      }
      if (currentLine.endsWith("GET /L")) {
        modus=0;  
        Serial.print("Modus Umsachalten auf:");
        Serial.println(modus);
      }                                        // end if client.available    
    }    
  }                                         // end while client.available 
  client.stop();                            // Verbindung schließen
  Serial.println("Client Disconnected.");  
  Serial.println();      
}

void setup() {
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(255); //  (max = 255)
  
  Serial.begin(115200);
  if(!WiFi.config(lclIP,gateway,subnet)){
    Serial.println("WLAN Probleme");
  }
  
  xTaskCreatePinnedToCore(
                    Task1code,   /* Task function. */
                    "Task1",     /* name of task. */
                    10000,       /* Stack size of task */
                    NULL,        /* parameter of the task */
                    1,           /* priority of the task */
                    &Task1,      /* Task handle to keep track of created task */
                    1);          /* pin task to core 0 */                  
  delay(500); 
  
  sensors.begin();

  Serial.print("Verbindungsaufbau zu ");    // Verbindung zum WiFi-Netzwerk
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    }  
  Serial.println("");                       // Verbindung aufgebaut
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();  
  Serial.println("Start WiFi Server!");  
  serverWiFi.begin();
}

void Task1code( void * pvParameters ){
  //Serial.print("Task1 running on core ");
  //Serial.println(xPortGetCoreID());
  for(;;){
    lausch();
    delay(200);
  } 
}

void loop() {
  //Temperatursensor
  sensors.requestTemperatures(); // Send the command to get temperatures
  tempc = sensors.getTempCByIndex(0);
  if(tempc != DEVICE_DISCONNECTED_C) 
  {
    Serial.print("Temperature for the device 1 (index 0) is: ");
    Serial.println(tempc);
  } 
  else
  {
    Serial.println("Error: Could not read temperature data");
  }
 
  if (modus==0){
    besinnlichmode();
  }
  else{
    krischanmodus();
  }
    lausch();
}

void lausch(){
  WiFiClient client = serverWiFi.available();  // horcht auf Client-Anfragen
  if (client) {                             // fragt ein Client an?
    wifiReceive (client); 
    delay(100);// Anfrage aufbereiten
    //Serial.print("bin im Lausch, modus:");
    //Serial.println(modus);
  }  
}

void besinnlichmode(){
  strip.clear(); // Set all pixel colors to 'off'
  for (int i=0;i<LED_COUNT;i++){
    strip.setPixelColor(i, strip.Color(5, 5, 0));
  }
  strip.show();
}

void krischanmodus(){

  if (modus==1){ colorWipe(strip.Color(255,   0,   0), 1);} // Red
  if (modus==1) { rainbow(20);}             // Flowing rainbow cycle along the whole strip
  if (modus==1){ colorWipe(strip.Color(  255, 255,   0), 1);}// Gelb
  if (modus==1) { rainbow(10); }            // Flowing rainbow cycle along the whole strip
  if (modus==1){ colorWipe(strip.Color(  0, 255,   0), 1);} // Grün
  if (modus==1) { rainbow(1);  }           // Flowing rainbow cycle along the whole strip
  if (modus==1){ colorWipe(strip.Color(  100, 100,   100), 1);} // white
  if (modus==1) { theaterChaseRainbow(100);} // Rainbow-enhanced theaterChase variant
  if (modus==1) {colorWipe(strip.Color(  0,   0, 255), 1);} // Blue
  if (modus==1) { theaterChaseRainbow(50);} // Rainbow-enhanced theaterChase variant
  if (modus==1) {colorWipe(strip.Color(  0,   255, 255), 1);} // Blue
  if (modus==1) { theaterChaseRainbow(5);} // Rainbow-enhanced theaterChase variant
  if (modus==1) {colorWipe(strip.Color(  255,   0, 255), 1);} // Blue

  if (modus==1) { theaterChase(strip.Color(127, 127, 127), 50);} // White, half brightness
  if (modus==1) { theaterChase(strip.Color(255,   0,   0), 100);} // Red, half brightness
  if (modus==1) {  theaterChase(strip.Color(  0,   0, 255), 100);} // Blue, half brightnes
  if (modus==1) {  theaterChase(strip.Color(  255,   0, 255), 100);} // Blue, half brightness
  if (modus==1) {  theaterChase(strip.Color(  0,   255, 255), 100);} // Blue, half brightness
  if (modus==1) {  theaterChase(strip.Color(  255,   255, 0), 100);} // Blue, half brightness
  if (modus==1) { rainbow(10);}             // Flowing rainbow cycle along the whole strip
  if (modus==1) { theaterChaseRainbow(50);} // Rainbow-enhanced theaterChase variant 
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}

// Theater-marquee-style chasing lights. Pass in a color (32-bit value,
// a la strip.Color(r,g,b) as mentioned above), and a delay time (in ms)
// between frames.
void theaterChase(uint32_t color, int wait) {
  for(int a=0; a<20; a++) {  // Repeat 10 times...
 
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in steps of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show(); // Update strip with new contents
      delay(wait);  // Pause for a moment
    }
  }
}

// Rainbow cycle along whole strip. Pass delay time (in ms) between frames.
void rainbow(int wait) {
  // Hue of first pixel runs 5 complete loops through the color wheel.
  // Color wheel has a range of 65536 but it's OK if we roll over, so
  // just count from 0 to 5*65536. Adding 256 to firstPixelHue each time
  // means we'll make 5*65536/256 = 1280 passes through this outer loop:
  for(long firstPixelHue = 0; firstPixelHue < 5*65536; firstPixelHue += 256) {
    for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
      // Offset pixel hue by an amount to make one full revolution of the
      // color wheel (range of 65536) along the length of the strip
      // (strip.numPixels() steps):
      int pixelHue = firstPixelHue + (i * 65536L / strip.numPixels());
      // strip.ColorHSV() can take 1 or 3 arguments: a hue (0 to 65535) or
      // optionally add saturation and value (brightness) (each 0 to 255).
      // Here we're using just the single-argument hue variant. The result
      // is passed through strip.gamma32() to provide 'truer' colors
      // before assigning to each pixel:
      strip.setPixelColor(i, strip.gamma32(strip.ColorHSV(pixelHue)));
    }
    strip.show(); // Update strip with new contents
    delay(wait);  // Pause for a moment
  }
}

// Rainbow-enhanced theater marquee. Pass delay time (in ms) between frames.
void theaterChaseRainbow(int wait) {
  int firstPixelHue = 0;     // First pixel starts at red (hue 0)
  for(int a=0; a<60; a++) {  // Repeat 30 times...
 
    for(int b=0; b<3; b++) { //  'b' counts from 0 to 2...
      strip.clear();         //   Set all pixels in RAM to 0 (off)
      // 'c' counts up from 'b' to end of strip in increments of 3...
      for(int c=b; c<strip.numPixels(); c += 3) {
        // hue of pixel 'c' is offset by an amount to make one full
        // revolution of the color wheel (range 65536) along the length
        // of the strip (strip.numPixels() steps):
        int      hue   = firstPixelHue + c * 65536L / strip.numPixels();
        uint32_t color = strip.gamma32(strip.ColorHSV(hue)); // hue -> RGB
        strip.setPixelColor(c, color); // Set pixel 'c' to value 'color'
      }
      strip.show();                // Update strip with new contents
      delay(wait);                 // Pause for a moment
      firstPixelHue += 65536 / 90; // One cycle of color wheel over 90 frames
    }
  }
}
