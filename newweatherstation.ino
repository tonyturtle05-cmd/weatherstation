#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <WiFi.h>

#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>
#include <AS5600.h>

#define ssid "###"
#define pass "###"

#define AIO_USERNAME "####"
#define AIO_SERVER "io.adafruit.com"
#define AIO_SERVERPORT 1883
#define AIO_KEY "####"

WiFiClient client;

Adafruit_BME280 bme;
float temp;
float humid;
float press;

AS5600 as5600;
float angle;
float rawpos;

int reedpin = 12;
int pstate = 1;
int cstate = 1;
unsigned long prevtime = 0;
int debounce = 30;
int pulse=0;
volatile int startm;
int endm;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

Adafruit_MQTT_Publish TemperatureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Temperature");
Adafruit_MQTT_Publish HumidityFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Humidity");
Adafruit_MQTT_Publish PressureFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/Pressure");
Adafruit_MQTT_Publish WindDirectionFeed = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/WindDirection");

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  delay(1000);

  pinMode(reedpin, INPUT_PULLUP);


  bme.begin();
  if (!bme.begin(0x76)){
    Serial.println("Could not find BME 280 sensor!");
    while(!Serial);
  }
  Serial.println("BME280 ready!");

  Wire.begin();
  as5600.begin();
  as5600.setDirection(AS5600_CLOCK_WISE);
  if (!(as5600.isConnected())) {
    Serial.println("Could not find AS5600 sensor!");
  }
  Serial.println("AS5600 ready!");

  pinMode(reedpin, INPUT_PULLUP);
  

}

void loop() {

  // put your main code here, to run repeatedly:
  delay(5000);
  if (WiFi.status() == WL_CONNECTED){
    MQTT_connect();
    bmeread();
    winddir();
    windspd();
    TemperatureFeed.publish(temp);
    HumidityFeed.publish(humid);
    PressureFeed.publish(press);
    WindDirectionFeed.publish(angle);
    Serial.printf("Temp: %.2f C | Humidity: %.2f %% | Pressur: %.2fhPa\n", bme.readTemperature(),bme.readHumidity(), bme.readPressure() / 100.0F);
    Serial.print("Wind Direction: ");
    Serial.println(angle);
    delay(200000);

    



  }
  else {
    Serial.println("No Wifi");
  }
  

  }



void bmeread() {
  temp = bme.readTemperature();
  humid = bme.readHumidity();
  press = bme.readPressure();

}

void MQTT_connect(){
  int8_t ret;

  if (mqtt.connected()){
    return;
  }
  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Retrying");
    mqtt.disconnect();
    delay(5000);
    retries--;
    if (retries == 0) {
      while(1);
    }
    Serial.println("MQTT CONNECTED");
  }
}

void winddir () {
  rawpos = as5600.getCumulativePosition();
  if (rawpos != 0){
    angle  = 360 / (as5600.getCumulativePosition());

  }
  else {
    angle = 0;
  }

}

void windspd() {
  startm = millis();
  endm = startm + 10000;
  while (startm < endm){
  cstate = digitalRead(reedpin);
  if (pstate != cstate){
    if ((millis() - prevtime) > debounce){
      if (cstate == LOW){
        pulse += 1;
        prevtime = millis();
      }

    }
    pstate = cstate;

}
startm = millis();
}
}
