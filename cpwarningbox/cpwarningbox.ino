#include <ESP8266WiFi.h>//โมดูลเชื่อมต่อ wifi
#include <ArduinoJson.h>
#include <SD.h> // เพิ่ม library รับ SD card
#include <TMRpcm.h> // เพิ่ม library เล่นเสียงเพลง
#define SD_ChipSelectPin 4 // กำหนดช่องอ่าน SD card
TMRpcm tmrpcm; // กำหนดไฟล์เสียงให้กับลำโพง ouput
// ส่วนของการกำหนดค่าก่อนเริ่มทำงาน
const char* ssid = "Red2Red";
const char* password = "13192510";
//เชื่อยต่อ wifi และเช็คสถานะ SD card
const char* mqtt_server = "broker.mqttdashboard.com";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";
  int j=0;
  while (j<length) {
    msg += (char)payload[j++];
  }
  int val = msg.toInt();
  if(String(msg) == String("on")){
    digitalWrite(D0, HIGH); //จ่ายไฟให้แก่ลำโพงเมื่อกดเปิด
  }
  else if(String(msg) == String("off")){
    digitalWrite(D0, LOW); //หยุดจ่ายไฟให้แก่ลำโพง
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "clientId-VWB2020";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("62070051", "hello world from boss");
      // ... and resubscribe
      client.subscribe("62070051");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
void setup() {
  Serial.begin(9600);
  delay(10);
  // Connect to WiFi network
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
  // Print the IP address
  Serial.print("IP=");
  Serial.println(WiFi.localIP());
  //ตรวจสอบสถานะของ sd card
  tmrpcm.speakerPin = D0; // กำหนดช่อง output เพื่อต่อกับลำโพง
  if (!SD.begin(SD_ChipSelectPin)) { // ตรวจสอบว่า sd card พร้อมหรือไม่
    return; // ถ้าไม่พร้อมจะไม่ทำงานต่อ
  }
}
 
void loop() {
  int mic = analogRead(A0); //กำหนดช่องในการ input ความดังของเสียง 
  if (mic > 75) { // ถ้าดังเกินกว่า 75 dB ให้แสดงข้อความ
    Line_Notify("ON");//แจ้งเตือนในแอพลิเคชั่นไลน์
    Serial.println("Please Be quiet"); // แสดงผลผ่าน display ของ software arduino เพื่อตรวจสอบว่าใช้งานได้หรือไม่
    delay(50); // กำหนดดีเลย์ในการแสดงผลเป็นหน่วย 50 millisec
    tmrpcm.setVolume(6); // กำหนดความดังของการแสดงผลผ่านลำโพง
    tmrpcm.play("1.wav"); // เล่นไฟล์เสียงที่ได้ใส่ไว้ใน SD card ต้องเป็นสกุล .wav เท่านั้น
    delay(1000); // กำหนดดีเลย์ในการแสดงผลเป็นหน่วย 1 second
  } 
}
//ตั้งค่าการแจ้งเตือนในแอพพลิเคชั่น line
const char* host = "maker.ifttt.com";
const char *privateKey = "clJ_alSf6Gcgzvx8W64LKN"
const char *event = "alert";  


void Line_Notify(String msg) {

  WiFiClientSecure client;
  client.setInsecure();
  const int httpPort = 443;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }
  
  // We now create a URI for the request
  String url = "/trigger/";
  url += event;
  url += "/with/key/";
  url += privateKey;

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  String out = "";
  root.printTo(out);
  Serial.println(out);
  String postData = "";
  postData = "{";

  String message = String(msg);
  client.print(String("POST ") + url + " HTTP/1.1\r\n" +
             "Host: " + host + "\r\n" +
             "Content-Type: application/x-www-form-urlencoded\r\n" +
             "Content-Length: " + postData.length() + "\r\n\r\n" +
             postData + "\r\n");
  while (client.connected())
  {
      if (client.available())
  {
      String line = client.readStringUntil('\r');
      Serial.print(line);
  } else {
      delay(50);
  };
  
  }
}
