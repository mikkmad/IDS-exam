/*********

 Edited version of Rui Santos

 Complete project details at https://randomnerdtutorials.com


********* AUTHORS NOTES **********

Please note that this code is fetched from https://learn.hobye.dk/kits/iot-toturial.

**********************************/

// Load libraries
#include <WiFi.h>
#include "time.h"
#include "sntp.h"

// Replace with your network credentials
const char* ssid = "OnePlus6";
const char* password = "grisitarm";

// set up time-getters
const char* ntpServer1 = "pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3000;

// Time-zone rule for Europe/Rome, including daylight adjustment rules (optional)
const char* time_zone = "CET-1CEST, M3.5.0, M10.5.0/3";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String relayState = "off";

const int RELAY_PIN = 16;

// #define ONBOARD_RELAY 3

// Current time
unsigned long currentTime = millis();

// Previous time
unsigned long previousTime = 0;

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

// create variables keeping track of time
unsigned long previousMillis = 0;
const long interval = 1000;

void setup() {

  // setup the TimeZone configuration, including daylight offset
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);

  Serial.begin(9600);

  // Initialize the output variables as outputs
  pinMode(RELAY_PIN, OUTPUT);

  // Set outputs to LOW
  digitalWrite(RELAY_PIN, LOW);

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
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Copy and Paste this IP-address into your URL bar, in your preferred browser.");
  Serial.println("If your browser says: 'Hmmm… your Internet access is blocked' or something similar, try it on your phone.");

  server.begin();
}

void loop() {

  // make a structure containing all local time data
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) {
    Serial.println("No time available yet!");
    return;
  }

  WiFiClient client = server.available();  // Listen for incoming clients

  if (client) {  // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");  // print a message out in the serial port
    String currentLine = "";        // make a String to hold incoming data from the client

    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();

      if (client.available()) {  // if there's bytes to read from the client,
        char c = client.read();  // read a byte, then
        Serial.write(c);         // print it out the serial monitor
        header += c;

        if (c == '\n') {  // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:

          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // turns the GPIOs on and off

            if (header.indexOf("GET /status/on") >= 0) {
              Serial.println("Coffee machine is on and making coffee!");
              Serial.println("");
              relayState = "on";
              digitalWrite(RELAY_PIN, HIGH);
              
            }else if (header.indexOf("GET /status/off") >= 0) {
              Serial.println("Coffee machine is off, awaiting your input.");
              Serial.println("");
              relayState = "off";
              digitalWrite(RELAY_PIN, LOW);
            }

            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");

            // CSS to style the HTML document and the buttons

            client.println("<style>html { background-image: url('https://wallpapertag.com/wallpaper/full/4/c/4/339593-large-coffee-background-2560x1600-ios.jpg'); background-attachment: fixed; background-size: cover; font-family: Helvetica; display: inline-block; margin: 50px auto; text-align: center;}");
            client.println("h1 { color: white; }");
            client.println("p { color: white; }");
            client.println(".btnMakeCoffee { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; border-radius: 20px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; filter: drop-shadow(5px 5px 1px rgba(0, 0, 0, 0.25));}");
            client.println(".btnMakingCoffee { background-color: #FF0000; border: none; color: white; padding: 16px 40px; border-radius: 20px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer; filter: drop-shadow(5px 5px 1px rgba(0, 0, 0, 0.25));}</style></head>");

            // Display current state, and Make Coffee/Making Coffee... buttons
            // If the relayState is off, it displays the Make Coffee button

            if (relayState == "off") {
              // Web Page Heading
              client.println("<body><h1>Awaiting your order...</h1>");
              // the use of the button
              client.println("<p><em>Press the button below, to make me know you want a cup of coffee!</em></p>");
              // the button itself
              client.println("<p><a href=\"/status/on\"><button class=\"button btnMakeCoffee\">Make coffee</button></a></p>");
              // show local time
              client.println("<p>Local time:<p>");
              client.println(&timeinfo, "<p>%H:%M</p>");
              client.println(&timeinfo, "<p>%A, the %dth of %B</p>");

            } else {
              // Web Page Heading
              client.println("<body><h1>Making coffee! Please wait...</h1>");
              // the use of the button
              client.println("<p><em>Press the button below, to cancel your cup.</em></p>");
              // the button itself
              client.println("<p><a href=\"/status/off\"><button class=\"button btnMakingCoffee\">Cancel your coffee...</button></a></p>");
              // show local time
              client.println("<p>Local time:<p>");
              client.println(&timeinfo, "<p>%H:%M</p>");
              client.println(&timeinfo, "<p>%A, the %dth of %B</p>");
            }

            client.println("</body></html>");

            // The HTTP response ends with another blank line
            client.println();

            // Break out of the while loop
            break;

          } else {  // if you got a newline, then clear currentLine
            currentLine = "";
          }

        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
  }

  // Clear the header variable
  header = "";

  // Close the connection
  client.stop();
}