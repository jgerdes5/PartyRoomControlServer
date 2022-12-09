// Libraries
#include <SPI.h>
#include <Ethernet.h>
#include <aREST.h>
#include <avr/wdt.h>

// Relay pins
const int controlPin[16] = {23,22,25,24,27,26,29,28,31,30,33,32,35,34,37,36};

// MAC address for shield
byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x95, 0xD6 };

// IP address in case DHCP fails
IPAddress ip(192,168,1,155);

// Ethernet Server
EthernetServer server(80); // Using port 80

// Create aREST instance
aREST rest = aREST();

// Declare function
int setRelay(String command);
int partyMode();
int getRelayStatus(String command);

void setup() {
  for(int i=0;i<16;i++){
    pinMode(controlPin[i], OUTPUT);
    digitalWrite(controlPin[i],HIGH);
  }
  
  Serial.begin(115200);
  
  // Register RGB function
  rest.function("relay", setRelay);
  rest.function("status", getRelayStatus);
  rest.function("party", partyMode);
  
  Serial.println("Try DHCP...");
  if (Ethernet.begin(mac) == 0) {
    Serial.println("DHCP FAIL...Static IP");
    Ethernet.begin(mac, ip) ;
  }
  server.begin();
  Serial.print("server IP: ");
  Serial.println(Ethernet.localIP());
  Serial.println("Setup complete.\n");
}

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  rest.handle(client);
  wdt_reset();
}

int setRelay(String command) {
  int pin = command.toInt();
  Serial.print("\nCommand: ");
  Serial.print(command);
  digitalWrite(controlPin[pin], !digitalRead(controlPin[pin]));
  return digitalRead(controlPin[pin]);
}

int getRelayStatus(String command) {
  int pin = command.toInt();
  return digitalRead(controlPin[pin]);
}

int partyMode() {
  for(int i=0;i<16;i++){
    digitalWrite(controlPin[i],LOW);
    delay(100);
  }
  for(int i=0;i<16;i++){
    digitalWrite(controlPin[i],HIGH);
    delay(100);
  }
  return 1;
}
