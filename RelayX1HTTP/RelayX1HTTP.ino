
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>

//### CREDENTIALS ###
const char* ssid = "ESPap";
const char* password = "thereisnospoon";
// ### CREDENTIALS ###


const String BASE_NAME = "relais";
const String BOARD_ID = "desk";
const String MQTT_TOPIC = "/"+BASE_NAME+"/" + BOARD_ID;
// "/relais/desk/1/[on | off | toggle | pulse/500]"

const int BAUD_RATE = 9600; // 9600 or 115200
const int NR_OF_RELAYS = 1;   // 1 or 2 or 4 (NOT higher than 4!)
const unsigned long MAX_VALUE = 4294967295;
const int SERIAL_WAIT_TIME = 25;

const byte relayON[][4]  = {{0xA0, 0x01, 0x01, 0xA2}, {0xA0, 0x02, 0x01, 0xA3}, {0xA0, 0x03, 0x01, 0xA4}, {0xA0, 0x04, 0x01, 0xA5}};
const byte relayOFF[][4] = {{0xA0, 0x01, 0x00, 0xA1}, {0xA0, 0x02, 0x00, 0xA2}, {0xA0, 0x03, 0x00, 0xA3}, {0xA0, 0x04, 0x00, 0xA4}};
unsigned long relayTurnoffTime[] = {0,0,0,0}; // used for timed "pulse" (e.g. stay on on until millis()+500 ms )

WiFiClient espClient;
ESP8266WebServer webserver(80);

void setup() {
  Serial.begin(BAUD_RATE);

  setup_wifi();
  setup_webServer();
}

void setup_wifi() {
  delay(10);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  randomSeed(micros());

  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());
}

void setup_webServer() {
  webserver.on("/", handleRoot);
  webserver.onNotFound(handleOtherUrl);
  webserver.begin();
}

void loop() {
  webserver.handleClient();

  for(int i=0; i<NR_OF_RELAYS; i++) {
    if(relayTurnoffTime[i] > 0 && relayTurnoffTime[i] < millis()) {
      turnRelaisOff(i);
      relayTurnoffTime[i] = 0;
    }
  }
}

// Callback method when messages for subscribed Topic are comming in.
void mqtt_callback(char* topic_char, byte* payload, unsigned int length) {
  String topic = String(topic_char);
  topic.replace(MQTT_TOPIC + "/", "");

  int relaisNr = topic.substring(0,1).toInt() - 1;
  topic = topic.substring(2);

  // Handle message based on given topic
  if(topic.equals("on")) {
    turnRelaisOn(relaisNr);
  } else if(topic.equals("off")) {
    turnRelaisOff(relaisNr);
  } else if(topic.equals("toggle")) {
    toggleRelais(relaisNr);
  } else if(topic.startsWith("pulse/")) {
    topic.replace("pulse/","");
    pulseRelais(relaisNr, topic.toInt());
  } else {
    //Serial.println("## Unbehandeltes Topic: " + String(topic_char));
  }
}

void turnRelaisOn(int relaisNr) {
  if(relaisNr == -1) {
    for(int i=0; i<NR_OF_RELAYS; i++) {
      turnRelaisOn(i);
    }
  } else if(relaisNr>=0 && relaisNr<NR_OF_RELAYS) {
    Serial.write(relayON[relaisNr], sizeof(relayON[relaisNr]));
    Serial.flush();
    delay(SERIAL_WAIT_TIME);
    relayTurnoffTime[relaisNr] = MAX_VALUE;
  }
}

void turnRelaisOff(int relaisNr) {
  if(relaisNr == -1) {
    for(int i=0; i<NR_OF_RELAYS; i++) {
      turnRelaisOff(i);
    }
  } else if(relaisNr>=0 && relaisNr<NR_OF_RELAYS) {
    Serial.write(relayOFF[relaisNr], sizeof(relayOFF[relaisNr]));
    Serial.flush();
    delay(SERIAL_WAIT_TIME);
    relayTurnoffTime[relaisNr] = 0;
  }
}

void toggleRelais(int relaisNr) {
  if(relaisNr == -1) {
      for(int i=0; i<NR_OF_RELAYS; i++) {
        toggleRelais(i);
      }
  } else if(relaisNr>=0 && relaisNr<NR_OF_RELAYS) {
    if(relayTurnoffTime[relaisNr] != 0) {
      turnRelaisOff(relaisNr);
    } else {
      turnRelaisOn(relaisNr);
    }
  }
}

void pulseRelais(int relaisNr, int ms) {
  if(relaisNr == -1) {
    for(int i=0; i<NR_OF_RELAYS; i++) {
      pulseRelais(i, ms);
    }
  } else if(relaisNr>=0 && relaisNr<NR_OF_RELAYS) {
    turnRelaisOn(relaisNr);
    relayTurnoffTime[relaisNr] = millis()+ms;
  }
}

// Webserver: root "/"
void handleRoot() {
  String html = "<!doctype html>\n\
<html>\n\
  <head>\n\
    <meta charset='utf-8'>\n\
      <title>Relay-Switch UI</title>\n\
      <script>\n\
        function onButton(relaisNr)             { var xhttp = new XMLHttpRequest(); xhttp.open('GET', `/${relaisNr}/on`, true); xhttp.send(); }\n\
        function offButton(relaisNr)            { var xhttp = new XMLHttpRequest(); xhttp.open('GET', `/${relaisNr}/off`, true); xhttp.send(); }\n\
        function toggleButton(relaisNr)         { var xhttp = new XMLHttpRequest(); xhttp.open('GET', `/${relaisNr}/toggle`, true); xhttp.send(); }\n\
        function pulseButton(relaisNr, ms=500)  { var xhttp = new XMLHttpRequest(); xhttp.open('GET', `/${relaisNr}/pulse/${ms}`, true); xhttp.send(); }\n\
      </script>\n\
  </head>\n\
  <body>\n\
    <h1 style='text-align: center'>Switch Relais :)</h1>\n\
    <div style='text-align: center; '>";

    for(int i=1; i<=NR_OF_RELAYS; i++) {
          html += "\
      <button style='height:100px; width:100px; font-size:16px; vertical-align: middle;' type='button' onclick='onButton("+String(i)+")'>"+String(i)+" On</button>\n\
      <button style='height:100px; width:100px; font-size:16px; vertical-align: middle;' type='button' onclick='offButton("+String(i)+")'>"+String(i)+" Off</button>\n\
      <button style='height:100px; width:100px; font-size:16px; vertical-align: middle;' type='button' onclick='toggleButton("+String(i)+")'>"+String(i)+" Toggle</button>\n\
      <button style='height:100px; width:100px; font-size:16px; vertical-align: middle;' type='button' onclick='pulseButton("+String(i)+")'>"+String(i)+" Pulse 500ms</button><br />";
    }
      
  html += "</div></body></html>";
  webserver.send(200, "text/html", html);
}

// Webserver any other URL
void handleOtherUrl() {
  String uri2mqttTopic = MQTT_TOPIC + webserver.uri();
  char mqtt_topic[uri2mqttTopic.length()+1];
  uri2mqttTopic.toCharArray(mqtt_topic, uri2mqttTopic.length()+1);

  mqtt_callback(mqtt_topic, 0x00, (sizeof(mqtt_topic) / sizeof(mqtt_topic[0])));
  webserver.send(200, "text/plain", uri2mqttTopic);
}
