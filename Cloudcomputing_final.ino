#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <SSD1306.h>
#include <Servo.h>
#include "SSD1306Wire.h"
#include <Wire.h>
#include "OLEDDisplayUi.h"
#include "images.h"

#define PIN_DHT D6
#define DHTTYPE DHT11
#define servoPin D5

Servo servo;
//const char * ssid = "KT_GiGA_2G_CC76";
//const char * password = "0abd0fg224";
const char * ssid = "SMART324_3";
const char * password = "76543210";
SSD1306 display(0x3c, D8, D3); //온습도 센서 , 화면 디스플레이 매개변수
SSD1306Wire display2(0x3c, D2, D1);

OLEDDisplayUi ui     ( &display2);
const char * mqtt_server = "broker.mqtt-dashboard.com"; //브로커주소
const char * outTopic = "inTopic62042"; //사용자가 결정해서 기록
const char * outTopic2 = "inTopic62042_1"; //사용자가 결정해서 기록
const char * outTopic3 = "inTopic62042_2"; //사용자가 결정해서 기록
const char * inTopic = "outTopic62042"; //사용자가 결정해서 기록
const char * clientName = "960424Client"; //다음 이름이 중복되지 않게 수정

const int analogInPin = A0;

DHT dht(PIN_DHT, DHTTYPE);


WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
long now = millis();
long lastMeasure = 0;
char msg[50];

int LED_pin_R = D7;
int turn_on = 1;
int led = D4;
int timeIn = 1000;

void msOverlay(OLEDDisplay *display2, OLEDDisplayUiState* state) {
  display2->setTextAlignment(TEXT_ALIGN_RIGHT);
  display2->setFont(ArialMT_Plain_10);
  display2->drawString(128, 0, String(millis()));
}

void drawFrame1(OLEDDisplay *display2, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display2->drawXbm(x + 34, y + 14, WiFi_Logo_width, WiFi_Logo_height, WiFi_Logo_bits);
}

void drawFrame2(OLEDDisplay *display2, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  display2->setTextAlignment(TEXT_ALIGN_LEFT);
  display2->setFont(ArialMT_Plain_10);
  display2->drawString(0 + x, 10 + y, "201562042 Gwang Hyeong");

  display2->setFont(ArialMT_Plain_16);
  display2->drawString(0 + x, 20 + y, "Cloud Computing");

  display2->setFont(ArialMT_Plain_24);
  display2->drawString(0 + x, 34 + y, "Smart Farm");
}
void drawFrame3(OLEDDisplay *display2, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  // Text alignment demo
  display2->setFont(ArialMT_Plain_10);

  // The coordinates define the left starting point of the text
  display2->setTextAlignment(TEXT_ALIGN_LEFT);
  display2->drawString(0 + x, 11 + y, "TEMP_UP=SERVO ON");
  display2->drawString(0 + x, 22 + y, "TEMP_DOWN=R_LED_ON");
  display2->drawString(0 + x, 33 + y, "CDS_DOWN=LED_OFF");
}
void drawFrame4(OLEDDisplay *display2, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  float value;
  float hvalue;
  int vreading = analogRead(0);
  value = dht.readTemperature();
  String tt = (String)value;
  hvalue = dht.readHumidity();
  display2->setTextAlignment(TEXT_ALIGN_LEFT);
  display2->setFont(ArialMT_Plain_10);
  display2->drawString(0, 10, " Temperature:");
  display2->drawString(65, 10, String(value) + String("C"));
  display2->drawString(0, 25, " Humidity:");
  display2->drawString(55, 25, String(hvalue) + String("%"));
  display2->drawString(0, 40, " CDS:");
  display2->drawString(55, 40, String(vreading));
  display2->display();
}
FrameCallback frames[] = { drawFrame1, drawFrame2, drawFrame3, drawFrame4};

// how many frames are there?
int frameCount = 4;

// Overlays are statically drawn on top of a frame eg. a clock
OverlayCallback overlays[] = { msOverlay };
int overlaysCount = 1;


void setup() {
  pinMode(LED_pin_R, OUTPUT);
  servo.attach(servoPin);
  servo.write(0);
  // 화면표시
  display.init();
  display.flipScreenVertically();
  display.clear();
  display.display();
  ui.setTargetFPS(60);

  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  ui.setIndicatorPosition(BOTTOM);

  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);

  ui.setFrames(frames, frameCount);

  ui.setOverlays(overlays, overlaysCount);
  ui.init();
  display2.flipScreenVertically();
  pinMode(led, OUTPUT);
  Serial.begin(115200);
  delay(10);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived[");
  Serial.print(topic);
  Serial.print("]");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();

  String inString = "";

  for (int i = 0; i < length; i++) {
    inString += (char)payload[i];
  }
  timeIn = inString.toInt();
}




void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");

    if (client.connect(clientName)) {
      Serial.println("connected");
      client.publish(outTopic, "Reconnected");
      client.publish(outTopic2, "Reconnected");
      client.publish(outTopic3, "Reconnected");
      client.subscribe(inTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println("try again in 5 seconds");
      delay(5000);
    }
  }
}
float value;
float hvalue;
void loop() {
  delay(500);
  if (!client.connected())
  {
    reconnect();
  }
  int remainingTimeBudget = ui.update();

  if (remainingTimeBudget > 0) {
    // You can do some work here
    // Don't do stuff if you are below your
    // time budget.
    delay(remainingTimeBudget);
  }
  now = millis();

  if (now - lastMsg > 1000) {
    int vreading = analogRead(0);

    value = dht.readTemperature();
    String tt = (String)value;
    hvalue = dht.readHumidity();

    static char vvdata[6];
    static char vvdata2[6];
    dtostrf((float)value, 5, 1, vvdata2);
    dtostrf((float)hvalue, 5, 1, vvdata);
    client.publish(outTopic2, vvdata2);
    client.publish(outTopic3, vvdata);
    Serial.print(" 온도 : ");
    Serial.print(value);
    Serial.print(" 습도 : ");
    Serial.print(hvalue);

    //OLED


    static char vdata[6];
    dtostrf((float)vreading, 5, 1, vdata);
    client.publish(outTopic, vdata);
    Serial.print(" CDS : ");
    Serial.println(vreading);

    if (value > 25) {
      servo.write(180);
      delay(1000);
      servo.write(0);
      delay(1000);
    }
    if (value < 20){
      digitalWrite(LED_pin_R, 1);
    }
    else{
      digitalWrite(LED_pin_R, 0);
    }
    if (vreading < 400) {
      digitalWrite(led, HIGH);
      
    }
    else {
      digitalWrite(led, LOW);
      
    }

  }

  client.loop();
  //digitalWrite(led,HIGH);
  //delay(timeIn);
  //digitalWrite(led,LOW);
  //delay(timeIn);
}

