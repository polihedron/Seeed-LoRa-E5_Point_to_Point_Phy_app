LoRa-E5 base station and sensor, point to point LoRa range test device (no LoRa gateway needed). This is a basic device made to test LoRa network. It's built from danak6jq repo: 

https://github.com/danak6jq/Seeed-LoRa-E5 

as a base with v1.2.0 STM32WL SDK, but converted from LoRaWAN to SubGHz_Phy application, and then finally made with help of MOOC STM32WL workshop.

Base station module can log data via usb port from sensors, or with use of TFT ST7735 SPI display as a console (which is more convenient for outdoor tests)

Sensor module can log data via usb, has i2c AHT20 temperature and humidity sensor, 
Button_1 can trigger transmission (which can be useful for detector or alarm node app), 
and in Stop2 mode power consumption is ca. 2uA.

To edit sensor or base station app look for SubGHz_Phy\App\subghz_phy_app.c

display data:
![IMG_20220929_091909](https://user-images.githubusercontent.com/46649005/192967852-4209c3cf-94d4-47f6-be64-edd16ad0c059.jpg)

Sensor current consumption:

(APP_LOG_ENABLED 0, DEBUGGER_ENABLED 0, LOW_POWER_DISABLE 0, USART2 Disabled, LED2 disabled, direct 3V supply, LDO Regulator AP2112K-3.3 removed from LoRa E5-mini PCB due to rev current flow)

![Screenshot_9](https://user-images.githubusercontent.com/46649005/192966786-285ea0a3-6f0c-483a-a8d2-4cadc4c99604.png)

closeup:

![Screenshot_10](https://user-images.githubusercontent.com/46649005/192967053-eacbb87f-e6df-4030-bb25-c8319c3dddaf.png)

Base Station current consumption:

(APP_LOG_ENABLED 0, DEBUGGER_ENABLED 0, LOW_POWER_DISABLE 0, USART2 Disabled, LED2 disabled)
![Screenshot_12](https://user-images.githubusercontent.com/46649005/192966941-e5614fc2-ad29-4300-880d-d4798d71453d.png)

Terminal log:
 
Basestation and 2 Sensors
![Screenshot_8](https://user-images.githubusercontent.com/46649005/192967296-6b95688e-424c-404a-a46e-33ef134406a5.png)
