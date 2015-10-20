#include <ESP8266WiFi.h>;
#include <PubSubClient.h>;

// Wifi settings
const char* ssid     = "<ssid>";
const char* password = "<password>";

// MQTT settings
const char* mqtt_server = "192.168.1.1";

const int switchPins[4] = {D1, D2, D3, D4};

/* 
 *  0 means PNP HIGH / OFF
 *  1 means PNP LOW / ON
 */
const int switchState[2] = {HIGH, LOW};

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  delay(10);
  
  for (int i = 0; i < 4; i++) {
    pinMode(switchPins[i], OUTPUT);
    digitalWrite(switchPins[i], HIGH);
  }

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void setup_wifi() {
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("esp-ctrl")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      // ... and resubscribe
      client.subscribe("/lights");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  if (length != 3) {
    Serial.println("Invalid command");
    return;
  }
  if (payload[1] != ',') {
    Serial.println("Invalid command");
    return;
  }

  int sid = payload[0] - '0';
  int cmd = payload[2] - '0';

  char str[50];
  sprintf(str, "Setting %d to %d", sid, cmd);
  Serial.println(str);

  int state = switchState[cmd];
  if (state > 1) {
    Serial.println("Invalid state requested");
    return;
  }

  int pin = switchPins[sid];

  digitalWrite(pin, state);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  
  // put your main code here, to run repeatedly:
  client.loop();
}
