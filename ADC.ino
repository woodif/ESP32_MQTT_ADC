/*
  Basic ESP8266 MQTT example

  This sketch demonstrates the capabilities of the pubsub library in combination
  with the ESP8266 board/library.

  It connects to an MQTT server then:
  - publishes "hello world" to the topic "outTopic" every two seconds
  - subscribes to the topic "inTopic", printing out any messages
    it receives. NB - it assumes the received payloads are strings not binary
  - If the first character of the topic "inTopic" is an 1, switch ON the ESP Led,
    else switch it off

  It will reconnect to the server if the connection is lost using a blocking
  reconnect function. See the 'mqtt_reconnect_nonblocking' example for how to
  achieve the same result without blocking the main loop.

  To install the ESP8266 board, (using Arduino 1.6.4+):
  - Add the following 3rd party board manager under "File -> Preferences -> Additional Boards Manager URLs":
       http://arduino.esp8266.com/stable/package_esp8266com_index.json
  - Open the "Tools -> Board -> Board Manager" and click install for the ESP8266"
  - Select your ESP8266 in "Tools -> Board"

*/

#include "WiFi.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>



const char* ssid = "CMMC_Sinet_2.4G";
const char* password =  "zxc12345";
const char* mqttServer = "35.240.251.16";
const int mqttPort = 1883;
const char* mqttUser = "admin";
const char* mqttPassword = "wooddy";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
int zxc = 0;
void setup() {
  pinMode(BUILTIN_LED, OUTPUT);     // Initialize the BUILTIN_LED pin as an output
  Serial.begin(115200);
  Serial.println();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }

  Serial.println("Connected to the WiFi network");

  client.setServer(mqttServer, mqttPort);

  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");

    if (client.connect("ESP32Client", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);

    }
  }


  //StaticJsonBuffer<200> jsonBuffer;

}



void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String clientName;
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    char s[21];
    random_string(s, 20);
    if (client.connect(s, "cmmc", "cmmc")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
}
void loop() {

  if (!client.connected()) {
    reconnect();
  }


  long now = millis();
  if (now - lastMsg > 10000) {
    lastMsg = now;
    StaticJsonDocument<1000> jsonBuffer;
    JsonObject root = jsonBuffer.to<JsonObject>();

    unsigned int ADC1 = analogRead(39);
    unsigned int ADC2 = analogRead(34);
    unsigned int ADC3 = analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35) + analogRead(35);


    float voltage1 = (ADC1 * 0.0008523) / 0.1666667;
    //float voltage1 = (ADC1);
    //ADC = 2830 // 2.412

    float voltage2 = (ADC2 * 0.001);
    //ADC = 950 // 0.942
    
    float voltage3 = (ADC3 /10 *0.00080566);
    //float voltage3 = (ADC3 / 10 * 0.0026342);
    //ADC = //0.1844

    float power2 = voltage1 * voltage2;
    float power3 = voltage1 * voltage3;

    root["V_Bat"] = voltage1;
    root["A_in"] = voltage2;
    root["A_out"] = voltage3;
    root["P_in"] = power2;
    root["P_out"] = power3;

    //root["Diver_name"] = "monitor_current";
    root["millis"] = millis();
    serializeJsonPretty(root, Serial);
    char JSONmessageBuffer[200];
    //    root.printTo(JSONmessageBuffer, sizeof(JSONmessageBuffer));
    String output;
    serializeJson(jsonBuffer, output);
    Serial.println(output);
    output.toCharArray(JSONmessageBuffer, 200);
    if (client.publish("iot/traffic", JSONmessageBuffer) == true) {
      Serial.println("Success sending message");
      digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
    } else {
      Serial.println("Error sending message");
      digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    }
  }
  client.loop();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    zxc++;
    if (zxc > 240) {
      ESP.restart();
    }
  }
}
String macToStr(const uint8_t* mac) {

  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }
  }

  return result;
}

void random_string(char * string, unsigned length)
{
  /* Seed number for rand() */
  srand((unsigned int) time(0) + getpid());

  /* ASCII characters 33 to 126 */
  int i;
  for (i = 0; i < length; ++i)
  {
    string[i] = rand() % 94 + 33;
  }

  string[i] = '\0';
}
