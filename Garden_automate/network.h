#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <WiFiClientSecure.h>
#include <string.h>

#include <az_result.h>
#include <az_span.h>
#include <az_iot_hub_client.h>

#include <base64.h>
#include <bearssl/bearssl.h>
#include <bearssl/bearssl_hmac.h>
#include <libb64/cdecode.h>
#include "valve.h"

#define LED_PIN 2
#define sizeofarray(a) (sizeof(a) / sizeof(a[0]))
#define ONE_HOUR_IN_SECS 3600
#define NTP_SERVERS "pool.ntp.org", "time.nist.gov"
#define MQTT_PACKET_SIZE 1024


class Network {
    public:
      static char* ssid;
      static char* password;
      static char* host;
      static char* device_id;
      static char* device_key;
      static char* directMehodTopic;
      static int port;
      
      static Valve* valveOne;
      static Valve* valveTwo;
      static Valve* valveThree;
      static void setValve(Valve* valve, int index);
      
      static void connectToWiFi();
      static void initializeClients();
      static int connectToAzureIoTHub();
      static void establishConnection();

      static bool subscribeToDirectMessage();
      static void loop();

      static void initializeTime();
      static void printCurrentTime();
      static char* getCurrentLocalTimeString();
      static uint32_t getSecondsSinceEpoch();
      static void receivedCallback(char* topic, byte* payload, unsigned int length);

      static int generateSasToken(char* sas_token, size_t size);
      
  };
