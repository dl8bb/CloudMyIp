#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <WiFiClient.h>

#define USE_DEEP_SLEEP 1

char ipifyHostname[] = "api.ipify.org";
int ipifyPort = 443;
const char* ipifyFingerprint = "8F 6E 82 AD B9 CD 51 85 E6 D3 7B CD 5D 1F 57 82 BE B5 2C 66";

char dweetHostname[] = "dweet.io";
int dweetPort = 443;
const char* dweetFingerprint = "27 6F AA EF 5D 8E CE F8 8E 6E 1E 48 04 A1 58 E2 65 E8 C9 34";

String dweetThingName("dweetThingName");

// Time to sleep (in seconds):
const int sleepTimeS = 600;

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());

  Serial.println(myWiFiManager->getConfigPortalSSID());
}

void setup() {
  // Start Serial
  Serial.begin(115200);
  delay(10);
  
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Starting network...");
  WiFiManager wifiManager;
  
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setDebugOutput(true);
  // first parameter is name of access point, second is the password
  // wifiManager.autoConnect("ESP-CloudMyIp-");
  wifiManager.autoConnect();

  Serial.println("done!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  delay(1000);

#ifdef USE_DEEP_SLEEP == 1
  String ip = getIp();
  if ( ip != String() )
    dweet( ip ) ;
  // Sleep
  Serial.println("Entering deep sleep mode... ");
  ESP.deepSleep(sleepTimeS * 1000000);
#endif
}

String getIp()
{
  WiFiClientSecure client;
  if (client.connect(ipifyHostname, ipifyPort)) 
  {
      Serial.println("connected");
      if (client.verify(ipifyFingerprint, ipifyHostname)) {
          Serial.println("verified!");
          client.println("GET / HTTP/1.0");
          client.println("Host: api.ipify.org");
          client.println();
      } else {
          Serial.print("Fingerprint verification for ");
          Serial.print(ipifyHostname);
          Serial.println(" failed!");
          client.stop();
      }
  } else {
      Serial.print("Connection to ");
      Serial.print(ipifyHostname);
      Serial.println(" failed!");
      return String();
  }
  delay(5000);
  String line;
  while(client.available())
  {
    line = client.readStringUntil('\n');
    Serial.println(line);
  }
  return line;
}

bool dweet( String ip)
{

  WiFiClientSecure client;
  if (!client.connect(dweetHostname, dweetPort)) 
  {
    Serial.println("dweet failed");
    return false;
  } else {
      if (client.verify(dweetFingerprint, dweetHostname)) {
          Serial.println("verified!");
          Serial.println("dweeting ip address as " + dweetThingName + " [" + ip + "]" );
    
          client.println( String("GET /dweet/for/" + dweetThingName + "?IPaddress=") + ip + " HTTP/1.1");
          client.println("Host: dweet.io");
          client.println("Connection: close");
          client.println("");
          client.println("");
    
          delay(1000);
  
          // Read all the lines of the reply from server and print them to Serial
          while(client.available()) {
              String line = client.readStringUntil('\r');
              Serial.print(line);
          };
      } else {
          Serial.print("Fingerprint verification for ");
          Serial.print(dweetHostname);
          Serial.println(" failed!");
          client.stop();
      }
    return true;
  }; 
}

void loop() {
#ifndef USE_DEEP_SLEEP
  String ip = getIp();
  if ( ip != String() )
    dweet( ip ) ;
  // repeat after some minute
  delay(sleepTimeS * 1000);
#endif
}
