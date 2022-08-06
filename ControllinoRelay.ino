#include <Controllino.h> 
#include <Ethernet.h>
#include <MQTT.h> // https://github.com/256dpi/arduino-mqtt
#include <ArduinoQueue.h>

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
byte ip[] = {10, 3, 10, 88};  // <- change to match your network
char broker[] = "10.3.10.103";

EthernetClient net;
MQTTClient client;

ArduinoQueue<int> queue(20);
unsigned long lastMillis = 0;


void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);

  // Note: Do not use the client in the callback to publish, subscribe or
  // unsubscribe as it may cause deadlocks when other things arrive while
  // sending and receiving acknowledgments. Instead, change a global variable,
  // or push to a queue and handle it in the loop after calling `client.loop()`.
  
  for (int i = 0; i <= 9; i++) {
    if (topic == "controllino/relay/r" + String(i) && payload == "on") {
      digitalWrite(CONTROLLINO_R0+i, HIGH);
      queue.enqueue(i);
    }
    if (topic == "controllino/relay/r" + String(i) && payload == "off") {
      digitalWrite(CONTROLLINO_R0+i, LOW);
      queue.enqueue(i);
    }
  }
}

void connect() {
  Serial.print("connecting...");
  while (!client.connect("Controllino", "public", "public")) {
    Serial.print(".");
    delay(1000);
  }
  
  Serial.println("\nconnected!");

  client.subscribe("hello");
  for (int i = 0; i <= 9; i++) {
    client.subscribe("controllino/relay/r" + String(i));
  }
}


void setup() {
  pinMode(CONTROLLINO_R0, OUTPUT);
  pinMode(CONTROLLINO_R1, OUTPUT);
  pinMode(CONTROLLINO_R2, OUTPUT);
  pinMode(CONTROLLINO_R3, OUTPUT);  
  pinMode(CONTROLLINO_R4, OUTPUT);  
  pinMode(CONTROLLINO_R5, OUTPUT);
  pinMode(CONTROLLINO_R6, OUTPUT);
  pinMode(CONTROLLINO_R7, OUTPUT);
  pinMode(CONTROLLINO_R8, OUTPUT);
  pinMode(CONTROLLINO_R9, OUTPUT);
  
  Serial.begin(115200);
  Ethernet.begin(mac);

  client.begin("10.3.10.103", net);
  client.onMessage(messageReceived);

  connect();

  // Since all relays are off on restart, publich their state.
  for (int i = 0; i <= 9; i++) {
    client.publish("/controllino/relay/r" + String(i) + "/status", "off");
  }
}

void loop() {
  client.loop();

  if (!client.connected()) {
    connect();
  }

  while( !queue.isEmpty() ) {
    int item = queue.dequeue();
    String state = "off";
    
    if(digitalRead(CONTROLLINO_R0+item) == 1){
      state = "on";
    }
    
    client.publish("/controllino/relay/r" + String(item) + "/status", state );
  }
}
