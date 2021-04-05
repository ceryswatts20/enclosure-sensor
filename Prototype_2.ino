//For Wifi
#include <WiFi.h>
#include <HTTPClient.h>

//For RTC
//#include <rtc.h>

//For OLED Display, BME680
#include <Wire.h>

//For OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Needed for BME680 sensor
#include <Adafruit_BME680.h>

//OLED Display
// OLED display width, in pixels
#define SCREEN_WIDTH 128
// OLED display height, in pixels
#define SCREEN_HEIGHT 32
// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library.
// Reset pin # (or -1 if sharing Arduino reset pin)
#define OLED_RESET -1
 //0x3D for 128x64, 0x3C for 128x32
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//BME680 Sensor
#define SEALEVELPRESSURE_HPA (1013.25)
//I2C
Adafruit_BME680 bme;

// Set our wifi name and password
const char* ssid = "BT-PMAHKZ_EXT";
const char* password = "rGLdfry6JVC9qX";

//Variables to send data to webpage
String URL = "http://192.168.1.113/rfidattendance/getdata.php"; //computer IP or the server domain
String getData, Link;

// Assign variables to send data every minute
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;
double temp, humidity, gas;

//Limits - ball python example
double h_temp = 35;
double l_temp = 20;
double h_humidity = 65;
double l_humidity = 45;
double h_gas;
double l_gas;
String warning = "Warning:";

#define b_led 32
#define r_led 33

void setup() {
  Serial.begin(9600);

  pinMode(b_led, OUTPUT);
  pinMode(r_led, OUTPUT);
  
  /*digitalWrite(b_led, HIGH);
  digitalWrite(r_led, HIGH);
  delay(3000);
  //Turn leds off
  digitalWrite(b_led, LOW);
  digitalWrite(r_led, LOW);*/

  //Check OLED wired correctly
  //SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  //Text settings
  //Normal 1:1 pixel scale
  display.setTextSize(1);
  //Draw white text
  display.setTextColor(WHITE);
  //Start at top-left corner
  display.setCursor(0, 0);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  // Pause for 2 seconds
  delay(2000);

  //Clear buffer
  display.clearDisplay();
  
  //Display static text
  display.println("Habitat Monitoring"); display.println("V2.0");
  //Call display() after commands to make them visible on screen
  display.display();
  delay(2000);

  //Clear screen
  display.clearDisplay();
  display.display();
  //Start at top-left corner
  display.setCursor(0,0);
  

  //Attempt to connect to wifi
  WiFi.begin(ssid, password);
  //Print status to the serial monitor and display
  Serial.println("Connecting");
  display.print("Connecting");
  display.display();
  
  // Wait for wifi to connect
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    display.print(".");
    display.display();
  }
  
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  display.clearDisplay();
  display.display();
  display.setCursor(0,0);
  display.print("Connected to WiFi \nnetwork - ");
  display.println(WiFi.localIP());
  display.display();
  
  delay(1500);
  //Clear screen
  display.clearDisplay();
  display.display();
  

  //Check BME680 sensor wired correctly
  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms
}

void loop() {
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
    connectToWiFi();    //Retry to connect to Wi-Fi
  }

  //Get sensor readings
  temp = bme.temperature;
  humidity = bme.humidity;
  gas = bme.gas_resistance / 1000.0;

  //If temp too low
  if((temp < l_temp) && temp != 0)
  {
    //Turn on blue led
    digitalWrite(b_led, HIGH);
    //r_led = 0;
    warning += "\nTemperature = " + String(temp) + char(247) + "C";
  }
  //If temp too high
  else if(temp > h_temp)
  {
    //Turn on red led
    digitalWrite(r_led, HIGH);
    //b_led = 0;
    warning += "\nTemperature = " + String(temp) + char(247) + "C";
  }

  //If humidity too low
  if((humidity < l_humidity) && humidity != 0)
  {
    //Turn on blue led
    digitalWrite(b_led, HIGH);
    //r_led = 0;
    warning += "\nHumidity = " + String(humidity) + "%";
  }
  //If humidity too high
  else if(humidity > h_humidity)
  {
    //Turn on red led
    digitalWrite(r_led, HIGH);
    //b_led = 0;
    warning += "\nHumidity = " + String(humidity) + "%";
  }

  //Check if its been a 30s
  if ((millis() - lastTime) > timerDelay) {
    //Send data to webpage
    sendData(temp, humidity, gas);
    lastTime = millis();
  }
  
  //Start at top-left corner
  display.setCursor(0,0);
  //Clear buffer
  display.clearDisplay();
  
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }

  //If warning message waiting
  if(warning != "Warning:")
  {
    //Print warning to display
    display.print(warning);
    display.display();
    //Reset warning
    warning = "Warning:";
  }
  else
  {
    //Turn leds off
    digitalWrite(b_led, LOW);
    digitalWrite(r_led, LOW);
    
    //Print sensor values
    display.print("Temperature: "); display.print(temp); display.print(" "); display.write(247); display.println("C");
    display.print("Humidity: "); display.print(humidity); display.println(" %");
    display.print("Gas: "); display.print(gas); display.println(" KOhms");
    display.display();

    //Reset warning
    warning = "Warning:";
  }
  
  //Wait 0.5s
  delay(500);
}

void sendData(double temp, double humdity, double gas){
  if(WiFi.isConnected()) {
    // Initialize our HTTP client
    HTTPClient http;
    
    //GET data
    //Add sensor data to GET array in order to send it
    getData = "?temp=" + String(temp) + "&humidity=" + String(humdity) + "&gas=" + String(gas);
    //GET method
    Link = URL + getData;
    http.begin(Link); // Initialize our HTTP request
      
    int httpResponseCode = http.GET(); // Send HTTP request
    String payload = http.getString();    //Get the response payload
    
    Serial.println(payload);    //Print request response payload

    //Check for good HTTP status code
    if (httpResponseCode > 0)
    { 
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
    }
    else
    {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    //Close connection
    http.end();
  }
}

void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    
    Serial.print("Connecting to ");
    Serial.println(ssid);
    //Clear screen
    display.clearDisplay();
    display.display();
    //Start at top-left corner
    display.setCursor(0,0);
    display.print("Connecting to ");
    display.println(ssid);
    display.display();
    
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      display.print(".");
      display.display();
    }
    Serial.println("");
    Serial.println("Connected");
    display.println("");
    display.println("Connected");
    display.display();
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    
    delay(1000);
    
    //Clear screen
    display.clearDisplay();
    display.display();
}
