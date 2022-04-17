# Garden_AuTomate_ESP8266


### This is a simple watering scheduler project based on a cheap ESP8266 that you can control from anywhere, thanks to Azure IoT Hub.

<br> This project is the new iteration of my very first project computer science:  

|              | First version | New Version |
| ------------ | :-----------: | :---------: |
| Hardware     | Raspberry Pi  | ESP8266     |
| Language     | Python        | C / C++     |
| Environment  | Local (LAN)   | Cloud (WAN) |
| Technology   | Flask         | Azure IoT Hub |

## [Setup Your Azure IoT Hub](https://docs.microsoft.com/en-us/azure/iot-hub/iot-hub-create-through-portal)

## Compile and push the code on your ESP8266
- Open the .ino file in your arduino environment
- [Setup your ESP8266 in the arduino environment](https://create.arduino.cc/projecthub/electropeak/getting-started-w-nodemcu-esp8266-on-arduino-ide-28184f)
- Inside [config.h](https://github.com/IterateMe/Garden_AuTomate_ESP8266/blob/master/Garden_automate/iot_configs.h), setup your local internet connection and your Azure IoT Hub connections settings
- Ensure you have the proper drivers [CH340](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all) OR [cp210x](https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers)
- Select the adequate COM port in <Tools -> Port -> COM{x}> menu bar  
- Push the code in the ESP8266

## Test the result with Azure IoT Explorer  
- Connect LEDs to the 3 output pins
- [Install and settup Azure IoT Explorer](https://docs.microsoft.com/en-us/azure/iot-fundamentals/howto-use-iot-explorer)
- Send these commands:  
  ```bash
    turnOn1:
  ```  
  ```bash
    turnOff1:
  ```  

## References 
- [Azure_SDK_Embeded_C_Doc](https://azuresdkdocs.blob.core.windows.net/$web/c/az_iot/1.1.0/globals_func.html#index_c)
- [Azure_SDK_Embeded_C_REPO](https://github.com/Azure/azure-sdk-for-c)
- [Tutorial used for this project](https://github.com/Azure/azure-sdk-for-c-arduino/blob/main/examples/Azure_IoT_Hub_ESP8266/readme.md)
- [Security_Azure_IoT](https://azure.microsoft.com/en-au/overview/internet-of-things-iot/iot-security-cybersecurity/)
