
#include <ESP8266WiFi.h>

#define LED_BUILTIN 4  //D2 -> GPIO 4

const char* ssid = "IOTAP";
const char* password = "raspberry";

const char* host = "api.mediatek.com";
const char* DeviceId   = "Dee2nAhz";
const char* DeviceKey = "JIMl9V6xPkXhs1MI";
const char* ChannelId = "r1";

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(LED_BUILTIN, OUTPUT);

  // We start by connecting to a WiFi network

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
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

int value = 0;

void uploadA0(int value) {

  Serial.println("\n\n\nUpload A0");
  Serial.print("connecting to ");
  Serial.println(host);

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  String url = String("/mcs/v2/devices/") + DeviceId + "/datapoints.csv";

  String data = String("a0,,") + value;
  int dataLength = data.length();

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "deviceKey: " + DeviceKey + "\r\n" +
               "Content-Length: " + dataLength + "\r\n" +
               "Content-Type: text/csv" + "\r\n" +
               "Connection: close\r\n\r\n" +
               data + "\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }

  Serial.println();
  Serial.println("closing connection");
}

int downloadR() {
  int result = -1;

  Serial.println("\n\n\nDownload R");
  Serial.print("connecting to ");
  Serial.println(host);

  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return -1;
  }

  // We now create a URI for the request
  String url = String("/mcs/v2/devices/") + DeviceId + "/datachannels/r1/datapoints.csv";

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "deviceKey: " + DeviceKey + "\r\n" +
               "Connection: close\r\n\r\n");

  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return -1;
    }
  }

  // Read all the lines of the reply from server and print them to Serial
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);

    if (line.substring(0, 4) == "\nr1,") {
      String r = line.substring(line.length() - 1);
      Serial.println("R: " + r);
      result = r == "1" ? HIGH : LOW;
    }
  }

  Serial.println();
  Serial.println("closing connection");
  return result;
}

void loop() {
//  delay(1000);
  uploadA0(analogRead(A0));

  int r = downloadR();
  if (r > -1)
    digitalWrite(LED_BUILTIN, r);

}
