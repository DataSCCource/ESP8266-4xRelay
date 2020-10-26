
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ESP8266WebServer.h>

//### CREDENTIALS ###
const char* ssid = "<SSID>";
const char* password = "<WLAN-Key";
const char* mqtt_server = "<mqtt-broker>";
// ### CREDENTIALS ###


const String BASE_NAME = "relais";
const String BOARD_ID = "desk";
const String MQTT_TOPIC = "/"+BASE_NAME+"/" + BOARD_ID;
// "/relais/desk/1/[on | off | toggle | pulse/500]"

const int BAUD_RATE = 115200; // 9600 or 115200
const int NR_OF_RELAYS = 2;   // 1 or 2 or 4 (NOT higher than 4!)
const unsigned long MAX_VALUE = 4294967295;
const int SERIAL_WAIT_TIME = 25;

const byte relayON[][4]  = {{0xA0, 0x01, 0x01, 0xA2}, {0xA0, 0x02, 0x01, 0xA3}, {0xA0, 0x03, 0x01, 0xA4}, {0xA0, 0x04, 0x01, 0xA5}};
const byte relayOFF[][4] = {{0xA0, 0x01, 0x00, 0xA1}, {0xA0, 0x02, 0x00, 0xA2}, {0xA0, 0x03, 0x00, 0xA3}, {0xA0, 0x04, 0x00, 0xA4}};
unsigned long relayTurnoffTime[] = {0,0,0,0}; // used for timed "pulse" (e.g. stay on on until millis()+500 ms )

WiFiClient espClient;
PubSubClient mqtt_client(espClient);
ESP8266WebServer webserver(80);

// Test
String iotPanelJson = "{\"dataVersion\":53,\"defaultDashboard\":\"dashboard_0\",\"dashboards\":[{\"dashboardId\":\"dashboard_0\",\"dashboardName\":\"RelaysTest\",\"dashboardPrefixTopic\":\"\",\"dashboardIcon\":\"dashboard\",\"dashboardColor\":\"#2196F3\",\"panels\":[{\"iconOn\":\"led\",\"iconColorOn\":\"#DF0000\",\"iconOff\":\"led\",\"iconColorOff\":\"#9E9E9E\",\"panelName\":\"Relay 1\",\"topic\":\""+MQTT_TOPIC+"/1/info\",\"payloadOn\":\"1\",\"payloadOff\":\"0\",\"panelId\":\"panel_25\",\"type\":\"led\",\"panelWidth\":\"100\",\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-bottom\"},{\"panelName\":\"On\",\"topic\":\""+MQTT_TOPIC+"/1/on\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_0\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"Off\",\"topic\":\""+MQTT_TOPIC+"/1/off\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_9\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"toggle\",\"topic\":\""+MQTT_TOPIC+"/1/toggle\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_19\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"hasCustomIcon\":false,\"icon\":\"button\",\"iconColor\":\"#9E9E9E\"},{\"panelName\":\"Pulse\",\"topic\":\""+MQTT_TOPIC+"/1/pulse/500\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_3\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"iconOn\":\"led\",\"iconColorOn\":\"#DF0000\",\"iconOff\":\"led\",\"iconColorOff\":\"#9E9E9E\",\"panelName\":\"Relay 2\",\"topic\":\""+MQTT_TOPIC+"/2/info\",\"payloadOn\":\"1\",\"payloadOff\":\"0\",\"panelId\":\"panel_26\",\"type\":\"led\",\"panelWidth\":\"100\",\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top merge-bottom\"},{\"panelName\":\"on\",\"topic\":\""+MQTT_TOPIC+"/2/on\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_18\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"},{\"panelName\":\"off\",\"topic\":\""+MQTT_TOPIC+"/2/off\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_10\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"Toggle\",\"topic\":\""+MQTT_TOPIC+"/2/toggle\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_2\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"Pulse\",\"topic\":\""+MQTT_TOPIC+"/2/pulse/500\",\"noPayload\":true,\"panelId\":\"panel_5\",\"type\":\"button\",\"panelWidth\":\"25\",\"buttonSize\":\"medium\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"iconOn\":\"led\",\"iconColorOn\":\"#DF0000\",\"iconOff\":\"led\",\"iconColorOff\":\"#9E9E9E\",\"panelName\":\"Relay 3\",\"topic\":\""+MQTT_TOPIC+"/3/info\",\"payloadOn\":\"1\",\"payloadOff\":\"0\",\"panelId\":\"panel_27\",\"type\":\"led\",\"panelWidth\":\"100\",\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top merge-bottom\"},{\"panelName\":\"on\",\"topic\":\""+MQTT_TOPIC+"/3/on\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_16\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"},{\"panelName\":\"off\",\"topic\":\""+MQTT_TOPIC+"/3/off\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_11\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"toggle\",\"topic\":\""+MQTT_TOPIC+"/3/toggle\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_17\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"},{\"panelName\":\"Pulse\",\"topic\":\""+MQTT_TOPIC+"/3/pulse/500\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_7\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"iconOn\":\"led\",\"iconColorOn\":\"#DF0000\",\"iconOff\":\"led\",\"iconColorOff\":\"#9E9E9E\",\"panelName\":\"Relay 4\",\"topic\":\""+MQTT_TOPIC+"/4/info\",\"payloadOn\":\"1\",\"payloadOff\":\"0\",\"panelId\":\"panel_28\",\"type\":\"led\",\"panelWidth\":\"100\",\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top merge-bottom\"},{\"panelName\":\"on\",\"topic\":\""+MQTT_TOPIC+"/4/on\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_14\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"},{\"panelName\":\"off\",\"topic\":\""+MQTT_TOPIC+"/4/off\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_12\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"toggle\",\"topic\":\""+MQTT_TOPIC+"/4/toggle\",\"noPayload\":true,\"buttonSize\":\"medium\",\"fitToPanelWidth\":true,\"panelId\":\"panel_15\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"},{\"panelName\":\"Pulse\",\"topic\":\""+MQTT_TOPIC+"/4/pulse/500\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_8\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"ALL Relays\",\"panelId\":\"panel_24\",\"type\":\"layout-decorator\",\"titleAlignment\":\"left\",\"fontSize\":\"20\",\"panelWidth\":\"100\",\"panelMergeClass\":\"merge-left merge-right merge-top merge-bottom\"},{\"panelName\":\"On\",\"topic\":\""+MQTT_TOPIC+"/0/on\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_1\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"off\",\"topic\":\""+MQTT_TOPIC+"/0/off\",\"noPayload\":true,\"buttonSize\":\"medium\",\"panelId\":\"panel_13\",\"type\":\"button\",\"panelWidth\":\"25\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"Toggle\",\"topic\":\""+MQTT_TOPIC+"/0/toggle\",\"noPayload\":true,\"panelId\":\"panel_4\",\"type\":\"button\",\"panelWidth\":\"25\",\"buttonSize\":\"medium\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"panelName\":\"Pulse\",\"topic\":\""+MQTT_TOPIC+"/0/pulse/2000\",\"noPayload\":true,\"panelId\":\"panel_6\",\"type\":\"button\",\"panelWidth\":\"25\",\"buttonSize\":\"medium\",\"retain\":false,\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \",\"fitToPanelWidth\":true},{\"orientation\":\"vertical\",\"comboItemList\":[{\"color\":\"#d70206\",\"$$hashKey\":\"object:755\",\"topic\":\""+MQTT_TOPIC+"/1/info\",\"label\":\"Relay 1\"},{\"color\":\"#d70206\",\"$$hashKey\":\"object:766\",\"topic\":\""+MQTT_TOPIC+"/2/info\",\"label\":\"Relay 2\"},{\"color\":\"#d70206\",\"$$hashKey\":\"object:778\",\"topic\":\""+MQTT_TOPIC+"/3/info\",\"label\":\"Relay 3\"},{\"color\":\"#d70206\",\"$$hashKey\":\"object:788\",\"topic\":\""+MQTT_TOPIC+"/4/info\",\"label\":\"Relay 4\"}],\"panelName\":\"Relay Status\",\"defineRange\":true,\"payloadMin\":0,\"payloadMax\":1,\"panelId\":\"panel_29\",\"type\":\"bar-graph\",\"panelWidth\":\"100\",\"qos\":0,\"panelMergeClass\":\"merge-left merge-right merge-top \"}]}]}";

void setup() {
  Serial.begin(BAUD_RATE);

  setup_wifi();
  setup_mqttClient();
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

void setup_mqttClient() {
  mqtt_client.setServer(mqtt_server, 1883);
  mqtt_client.setCallback(mqtt_callback);
}

void setup_webServer() {
  webserver.on("/", handleRoot);
  webserver.onNotFound(handleOtherUrl);
  webserver.begin();
}

void loop() {
  webserver.handleClient();

  if (!mqtt_client.connected()) {
    reconnect();
  }
  mqtt_client.loop();

  for(int i=0; i<NR_OF_RELAYS; i++) {
    if(relayTurnoffTime[i] > 0 && relayTurnoffTime[i] < millis()) {
      turnRelaisOff(i);
      relayTurnoffTime[i] = 0;
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqtt_client.connected()) {
    delay(500);
    // Create a random client ID
    String clientId = "WifiRelay-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqtt_client.connect(clientId.c_str())) {
      String subTopic = MQTT_TOPIC+"/#";
      mqtt_client.subscribe(subTopic.c_str());
    } else {
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

// Callback method when messages for subscribed Topic are comming in.
void mqtt_callback(char* topic_char, byte* payload, unsigned int length) {
  String topic = String(topic_char);
  topic.replace(MQTT_TOPIC + "/", "");

  if(topic.equals("requestiotpanel")) {
    uint16_t bufferSize = mqtt_client.getBufferSize();
    mqtt_client.setBufferSize(8192);

    String panelTopic = "/tmpiotpanel";
    mqtt_client.publish(panelTopic.c_str(), iotPanelJson.c_str());
    mqtt_client.setBufferSize(bufferSize);
    return;
  }
  
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

    String infoTopic = MQTT_TOPIC + "/" + String(relaisNr+1) + "/info";
    mqtt_client.publish(infoTopic.c_str(), "1");
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

    String infoTopic = MQTT_TOPIC + "/" + String(relaisNr+1) + "/info";
    mqtt_client.publish(infoTopic.c_str(), "0");
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

    for(int i=0; i<=NR_OF_RELAYS; i++) {
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
