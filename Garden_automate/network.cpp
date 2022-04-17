#include "network.h"

#include "iot_configs.h"
#include "ca.h"

static WiFiClientSecure wifi_client;
static X509List cert((const char*)ca_pem);
static PubSubClient mqtt_client(wifi_client);
static az_iot_hub_client client;
static az_iot_hub_client_c2d_request out_request;
static char sas_token[200];
static uint8_t signature[512];
static unsigned char encrypted_signature[32];
static char base64_decoded_device_key[32];
static unsigned long next_telemetry_send_time_ms = 0;
static char telemetry_topic[128];
static uint8_t telemetry_payload[100];
static uint32_t telemetry_send_count = 0;

char* Network::ssid = IOT_CONFIG_WIFI_SSID;
char* Network::password = IOT_CONFIG_WIFI_PASSWORD;
char* Network::host = IOT_CONFIG_IOTHUB_FQDN;
char* Network::device_id = IOT_CONFIG_DEVICE_ID;
char* Network::device_key = IOT_CONFIG_DEVICE_KEY;
Valve* Network::valveOne = new Valve(D0);
Valve* Network::valveTwo = new Valve(D1);
Valve* Network::valveThree = new Valve(D2);
int Network::port = 8883;

char* Network::directMehodTopic = "$iothub/methods/POST/hello_world/";

//uint8_t pinValveOne = '';

void Network::connectToWiFi()
{
  Serial.println();
  Serial.print("Connecting to WIFI SSID ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void Network::receivedCallback(char* topic, byte* payload, unsigned int length)
{
  char payloadChar[length>>2];
  String payloadString;
  String turnOn1 = "turnOn1";
  String turnOff1 = "turnOff1";
  String turnOn2 = "turnOn2";
  String turnOff2 = "turnOff2";
  String turnOn3 = "turnOn3";
  String turnOff3 = "turnOff3";
  char msg[20];

  Serial.print("Received [");
  //Serial.print(topic);
  
  for (int i = 0; i < length; i++)
  {
    if((char)payload[i] == ':'){
      break;
      }
    payloadString += (char)payload[i];
  }
 
  Serial.print(payloadChar);
  Serial.println("]");
  Serial.print(payloadString);
  if(turnOn1 == payloadString){
    Serial.println("Turning ON valve 1");
    valveOne->turnOn();
  }else if(turnOff1 == payloadString){
    Serial.println("Turning OFF valve 1");
    valveOne->turnOff();
  }else if(turnOn2 == payloadString){
    Serial.println("Turning ON valve 2");
    valveTwo->turnOn();
  }else if(turnOff2 == payloadString){
    Serial.println("Turning OFF valve 2");
    valveTwo->turnOff();
  }else if(turnOn3 == payloadString){
    Serial.println("Turning ON valve 3");
    valveThree->turnOn();
  }else if(turnOff3 == payloadString){
    Serial.println("Turning OFF valve 3");
    valveThree->turnOff();
  }else{
    Serial.println("Could not compare payload");
  }
  az_iot_hub_client_c2d_parse_received_topic( &client,
                                              az_span_create((uint8_t*)topic, (unsigned)strlen(topic)),
                                              &out_request);
  //az_span element = out_request.properties._internal.properties_buffer;
  //az_span_to_str(msg, 2, element);
  //Serial.print("[");Serial.print(msg);Serial.println("]");
}

void Network::initializeClients()
{
  wifi_client.setTrustAnchors(&cert);
  if (az_result_failed(az_iot_hub_client_init(
          &client,
          az_span_create((uint8_t*)host, strlen(host)),
          az_span_create((uint8_t*)device_id, strlen(device_id)),
          NULL)))
  {
    Serial.println("Failed initializing Azure IoT Hub client");
    return;
  }

  mqtt_client.setServer(host, port);
  mqtt_client.setCallback(receivedCallback);
}

int Network::generateSasToken(char* sas_token, size_t size)
{
  az_span signature_span = az_span_create((uint8_t*)signature, sizeofarray(signature));
  az_span out_signature_span;
  az_span encrypted_signature_span
      = az_span_create((uint8_t*)encrypted_signature, sizeofarray(encrypted_signature));

  uint32_t expiration = getSecondsSinceEpoch() + ONE_HOUR_IN_SECS;

  // Get signature
  if (az_result_failed(az_iot_hub_client_sas_get_signature(
          &client, expiration, signature_span, &out_signature_span)))
  {
    Serial.println("Failed getting SAS signature");
    return 1;
  }

  // Base64-decode device key
  int base64_decoded_device_key_length
      = base64_decode_chars(device_key, strlen(device_key), base64_decoded_device_key);

  if (base64_decoded_device_key_length == 0)
  {
    Serial.println("Failed base64 decoding device key");
    return 1;
  }

  // SHA-256 encrypt
  br_hmac_key_context kc;
  br_hmac_key_init(
      &kc, &br_sha256_vtable, base64_decoded_device_key, base64_decoded_device_key_length);

  br_hmac_context hmac_ctx;
  br_hmac_init(&hmac_ctx, &kc, 32);
  br_hmac_update(&hmac_ctx, az_span_ptr(out_signature_span), az_span_size(out_signature_span));
  br_hmac_out(&hmac_ctx, encrypted_signature);

  // Base64 encode encrypted signature
  String b64enc_hmacsha256_signature = base64::encode(encrypted_signature, br_hmac_size(&hmac_ctx));

  az_span b64enc_hmacsha256_signature_span = az_span_create(
      (uint8_t*)b64enc_hmacsha256_signature.c_str(), b64enc_hmacsha256_signature.length());

  // URl-encode base64 encoded encrypted signature
  if (az_result_failed(az_iot_hub_client_sas_get_password(
          &client,
          expiration,
          b64enc_hmacsha256_signature_span,
          AZ_SPAN_EMPTY,
          sas_token,
          size,
          NULL)))
  {
    Serial.println("Failed getting SAS token");
    return 1;
  }

  return 0;
}

int Network::connectToAzureIoTHub()
{
  size_t client_id_length;
  char mqtt_client_id[128];
  if (az_result_failed(az_iot_hub_client_get_client_id(
          &client, mqtt_client_id, sizeof(mqtt_client_id) - 1, &client_id_length)))
  {
    Serial.println("Failed getting client id");
    return 1;
  }

  mqtt_client_id[client_id_length] = '\0';

  char mqtt_username[128];
  // Get the MQTT user name used to connect to IoT Hub
  if (az_result_failed(az_iot_hub_client_get_user_name(
          &client, mqtt_username, sizeofarray(mqtt_username), NULL)))
  {
    printf("Failed to get MQTT clientId, return code\n");
    return 1;
  }

  Serial.print("Client ID: ");
  Serial.println(mqtt_client_id);

  Serial.print("Username: ");
  Serial.println(mqtt_username);

  mqtt_client.setBufferSize(MQTT_PACKET_SIZE);

  while (!mqtt_client.connected())
  {
    time_t now = time(NULL);

    Serial.print("MQTT connecting ... ");

    if (mqtt_client.connect(mqtt_client_id, mqtt_username, sas_token))
    {
      Serial.println("connected.");
    }
    else
    {
      Serial.print("failed, status code =");
      Serial.print(mqtt_client.state());
      Serial.println(". Try again in 5 seconds.");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }

  mqtt_client.subscribe(AZ_IOT_HUB_CLIENT_C2D_SUBSCRIBE_TOPIC);

  return 0;
}

void Network::establishConnection() 
{
  connectToWiFi();
  initializeTime();
  printCurrentTime();
  initializeClients();

  // The SAS token is valid for 1 hour by default in this sample.
  // After one hour the sample must be restarted, or the client won't be able
  // to connect/stay connected to the Azure IoT Hub.
  if (generateSasToken(sas_token, sizeofarray(sas_token)) != 0)
  {
    Serial.println("Failed generating MQTT password");
  }
  else
  {
    connectToAzureIoTHub();
  }

  digitalWrite(LED_PIN, LOW);
}

uint32_t Network::getSecondsSinceEpoch()
{
  return (uint32_t)time(NULL);
}

void Network::printCurrentTime()
{
  Serial.print("Current time: ");
  Serial.print(getCurrentLocalTimeString());
}

void Network::initializeTime()
{
  Serial.print("Setting time using SNTP");

  configTime(-5 * 3600, 0, NTP_SERVERS);
  time_t now = time(NULL);
  while (now < 1510592825)
  {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("done!");
}

char* Network::getCurrentLocalTimeString()
{
  time_t now = time(NULL);
  return ctime(&now);
}

bool Network::subscribeToDirectMessage()
{
    return mqtt_client.subscribe(directMehodTopic);
}

void Network::loop(){
    if(mqtt_client.loop()){
        Serial.println("Connected!");
    }else{
        Serial.println("NOT Connected!");
    };  
}
