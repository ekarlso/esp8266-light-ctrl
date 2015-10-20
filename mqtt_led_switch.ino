#include <ESP8266WiFi.h>;
#include <PubSubClient.h>;

// Wifi settings
const char* ssid     = "<ssid>";
const char* password = "<password>";

// MQTT settings
const char* mqtt_server = "192.168.1.1";

// Switch pins
int switchPins[4] = {
  D1, D2, D3, D4
};

// State pins
int debounceDelay = 50;
int lastRead[4] = {0};

int buttonStates[4] = {};
int lastStates[4] = {LOW};

WiFiClient espClient;
PubSubClient client(espClient);


void setup() {
  Serial.begin(115200);
  
  // put your setup code here, to run once:
  for (int i = 0; i < 4; i++) {
    pinMode(switchPins[i], INPUT_PULLUP);
    lastRead[i] = millis();
  };

  setup_wifi();
  
  client.setServer(mqtt_server, 1883);
  //client.setCallback(callback);
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
    if (client.connect("esp-switch")) {
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


bool checkButton(int buttonPin) {
  bool changed = 0;
  int state = digitalRead(switchPins[buttonPin]);

  if (state != lastStates[buttonPin]) {
    lastRead[buttonPin] = millis();
  }

  if ((millis() - lastRead[buttonPin])> debounceDelay) {
    if (state != buttonStates[buttonPin]) {
      buttonStates[buttonPin] = state;
      
      if (state == LOW) {
        changed = 1;
      }
    }
  }

  lastStates[buttonPin] = state;
  return changed;
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  char str[50];
  
  // put your main code here, to run repeatedly:
  for (int i = 0; i < 4; i++) {
    bool changed = checkButton(i);
    if (changed) {
      sprintf(str, "Button was pressed %d!", i);
      Serial.println(str);

      sprintf(str, "switch,%i", i);
      client.publish("/control", str);
    }
  }

  client.loop();
}
