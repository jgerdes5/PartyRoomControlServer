// Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <ArduinoJson.h>
#include <aWOT.h>
#include <SD.h>

// Relay pins
const int numberOfRelays = 16;
const int controlPin[numberOfRelays] = {23,22,25,24,27,26,29,28,31,30,33,32,35,34,37,36};

// MAC address for shield
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x95, 0xD6 };

// IP address in case DHCP fails
IPAddress ip(192,168,178,155);

// Ethernet Server
EthernetServer server(80); // Using port 80

// WebApplication
Application app;

// SD Card
const int chipSelect = 4;

void setRelay(Request &req, Response &res) {
  char id[64];
  req.query("id", id, 64);
  int pin = atoi(id);

  if (pin < numberOfRelays) {
    digitalWrite(controlPin[pin], !digitalRead(controlPin[pin]));
    res.print("OK");
  } else {
    res.status(404);
    res.print("Not Found");
  }
}

void getRelayStatus(Request &req, Response &res) {
  char id[64];
  req.query("id", id, 64);
  int pin = atoi(id);

  if (pin < numberOfRelays) {
    DynamicJsonDocument doc(1024);
    doc["status"] = digitalRead(controlPin[pin]);
    serializeJson(doc, res);
  } else {
    res.status(404);
    res.print("Not Found");
  }
}

void setRelays(Request &req, Response &res) {
  StaticJsonDocument<200> input;
  deserializeJson(input, req);
  JsonArray relays = input["relays"];

  DynamicJsonDocument output(1024);
  JsonObject response = output.to<JsonObject>();
  JsonArray activated = response.createNestedArray("activated");
  JsonArray error = response.createNestedArray("error");

  for(JsonVariant relay : relays) {
    int pin = relay.as<int>();

    if(pin < numberOfRelays) {
      digitalWrite(controlPin[pin], !digitalRead(controlPin[pin]));
      activated.add(pin);
    } else {
      error.add(pin);
    }
  }
  serializeJson(output, res);
}

void testRelays(Request &req, Response &res) {
  for(int i=0;i<16;i++){
    digitalWrite(controlPin[i],LOW);
    delay(100);
  }
  for(int i=0;i<16;i++){
    digitalWrite(controlPin[i],HIGH);
    delay(100);
  }
  res.print("OK");
}

void saveConfig(Request &req, Response &res) {
  const char *filename = "c.txt";
  SD.remove(filename);
  File file = SD.open(filename, FILE_WRITE);
  if (!file) {
    Serial.println(F("Failed to create file"));
    return;
  }
  file.print(req.readString());
  file.close();
  res.print("OK");
}

void loadConfig(Request &req, Response &res) {
  const char *filename = "c.txt";
  File file = SD.open(filename);
  if (!file) {
    Serial.println("Failed to load file");
    return;
  }
  while(file.available()) {
    res.print((char)file.read());
  }
  file.close();
}

void setup() {
  for(int i=0;i<16;i++){
    pinMode(controlPin[i], OUTPUT);
    digitalWrite(controlPin[i],HIGH);
  }
  
  Serial.begin(115200);

  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);

  Ethernet.init(10);
  
  Serial.println("Try DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP FAIL...Static IP");
    Ethernet.begin(mac, ip) ;
  }

  // Register Rest function
  app.get("/relay", &setRelay);
  app.post("/relays", &setRelays);
  app.get("/testRelays", &testRelays);
  app.get("/status", &getRelayStatus);
  app.post("/saveConfig", &saveConfig);
  app.get("/loadConfig", &loadConfig);


  if (!SD.begin(chipSelect)) {
    Serial.println("Failed to initialize SD card");
  } else {
    Serial.println("SD Card initialized!");
  }

  server.begin();
  Serial.print("Server IP: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup complete.\n");

  
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();

  if (client.connected()) {
    app.process(&client);
    client.stop();
  }
}
