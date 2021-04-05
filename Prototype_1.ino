//For RTC
//#include <rtc.h>

//For OLED Display, BME680
#include <Wire.h>

//For OLED Display
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

//Needed for BME680 sensor
#include <Adafruit_BME680.h>

//Optional OLED Display?
#include <Adafruit_GrayOLED.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <gfxfont.h>
#include <splash.h>

//Optional BME680 sensor?
#include <bme680.h>
#include <bme680_defs.h>
#include <Adafruit_Sensor.h>;


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

void setup() {
  Serial.begin(9600);

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
  display.println("Habitat Monitoring"); display.println("V1.0");
  //Call display() after commands to make them visible on screen
  display.display();
  delay(2000);

  //Clear screen
  display.clearDisplay();
  display.display();
}

void loop() {
  //Start at top-left corner
  display.setCursor(0,0);
  //Clear buffer
  display.clearDisplay();
  
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading :(");
    return;
  }
  
  //Print sensor values
  display.print("Temperature: "); display.print(bme.temperature); display.print(" "); display.write(247); display.println("C");
  display.print("Pressure: "); display.print(bme.pressure / 100); display.println(" hPa");
  display.print("Humidity: "); display.print(bme.humidity); display.println(" %");
  display.print("Gas: "); display.print(bme.gas_resistance / 1000.0); display.println(" KOhms");
  display.display();
  //Wait 2s
  delay(2000);
}
