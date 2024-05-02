// Libraries
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <dht.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define D4 2 // pin D4 connected to DHT11 sensor
#define LED_PIN 16 // pin connected to LED

dht DHT;
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1); // create SSD1306 display object connected to I2C
ESP8266WebServer server(80); // create WebServer object

String displayString; // String to store temperature and humidity data
bool airconState = false; // Variable to store air conditioner state

const char* ssidap = "SmartHomeWifi"; // Access point SSID
const char* passap = ""; // Access point password

void setup() {
  Serial.begin(115200); // Initialize serial communication
  
  // Initialize OLED display
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Check if OLED initialization is successful
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000); // Delay for OLED initialization
  oled.clearDisplay(); // Clear OLED display
  oled.setTextSize(1); // Set text size
  oled.setTextColor(WHITE); // Set text color

  // Initialize DHT11 sensor
  pinMode(D4, INPUT);

  // Set up WiFi access point
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssidap, passap); // Set SSID and password for the access point
  Serial.println("AP Mode. Please connect to SmartHomeWifi to configure");
  Serial.println(WiFi.softAPIP()); // Print access point IP address

  // Create web server routes
  createWebServer();
  server.begin();

  // Set LED pin as output
  pinMode(LED_PIN, OUTPUT);
}

void loop() {
  server.handleClient(); // Handle web server requests
  
  // Read temperature and humidity
  int chk = DHT.read11(D4);
  float humidity = DHT.humidity;
  float temperatureC = DHT.temperature;
  
  // Display temperature and humidity on OLED
  oled.clearDisplay(); // Clear display

  // Draw rectangle border
  oled.drawRect(0, 0, SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);

  // Display temperature
  oled.setTextSize(1); // Set text size for labels
  oled.setCursor(5, 5);
  oled.print("Temp: ");
  oled.setTextSize(2); // Set text size for temperature value
  oled.println(String(temperatureC, 1) + "C");
  
  // Display humidity
  oled.setTextSize(1); // Set text size for labels
  oled.setCursor(5, 30);
  oled.print("Hum: ");
  oled.setTextSize(2); // Set text size for humidity value
  oled.println(String(humidity, 1) + "%");

  oled.display(); // Display content

  // Control air conditioner based on web server input
  if (airconState) {
    digitalWrite(LED_PIN, LOW); // Turn on the LED
  } else {
    digitalWrite(LED_PIN, HIGH); // Turn off the LED
  }
}

void createWebServer() {
  server.on("/", HTTP_GET, []() {
    // Send HTML response with welcome message and buttons
    String content = "<html><head>";
    content += "<style>button {font-size: 20px;}</style>"; // Set button font size
    content += "<script>function updateData(){fetch('/data').then(response => response.json()).then(data => {document.getElementById('temp').innerText = data.temp;document.getElementById('hum').innerText = data.hum;});}setInterval(updateData, 2000);</script>";
    content += "</head><body onload='updateData()'>";
    content += "<h1 style='text-align:center;'>Welcome to Smart Home Air Conditioning Control System</h1>";
    content += "<p style='text-align:center;font-size:24px;'>Temperature: <span id='temp'>--</span>&deg;C<br>Humidity: <span id='hum'>--</span>%</p>";
    content += "<div style='text-align:center;'><button style='background-color: #33b249' onclick='turnOnAircon()'>Turn On Aircon</button>"; // Green color for ON button
    content += "<button style='background-color: #ED0800' onclick='turnOffAircon()'>Turn Off Aircon</button></div>"; // Red color for OFF button
    content += "<script>function turnOnAircon(){fetch('/on');}function turnOffAircon(){fetch('/off');}</script>";
    content += "</body></html>";
    server.send(200, "text/html", content); // Send response
  });
 
  server.on("/on", HTTP_GET, []() {
    airconState = true; // Set aircon state to ON
    server.send(200, "text/plain", "Aircon is ON"); // Send response
  });

  server.on("/off", HTTP_GET, []() {
    airconState = false; // Set aircon state to OFF
    server.send(200, "text/plain", "Aircon is OFF"); // Send response
  });

  server.on("/data", HTTP_GET, []() {
    // Send JSON response with temperature and humidity data
    String data = "{\"temp\":\"" + String(DHT.temperature, 1) + "\",\"hum\":\"" + String(DHT.humidity, 1) + "\"}";
    server.send(200, "application/json", data); // Send response
  });
}
