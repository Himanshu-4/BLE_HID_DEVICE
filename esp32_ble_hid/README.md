# BLE HID device that support multi connections 

Hello everyone !!😀 . in previous months i been familiar with the Bluetooth Low energy . the protocol layers and how to devlop applications based on the BLE stack . Also my day to day Job is also based around BLE. so i have to learn and implement new things everyday ,since BLE is quite a broad and widely used protocol and it was quite popular in previous year in IOT field and low power applications . and some of the profiles that are also supported by devices like HID 🌝. so to just have fun around the HID protocol and to test that if i can able to make it to support the multiconnections in  BLE .
<br>
so Lets start with a quick introduction 
<br>
i have been  working as a firmware Lead in a brain health company and we are building a smart device that can communicate with the smartphone with BLE . and we are constantly optimising the device performance , features set , cost. and i am the only one in the firmware field . so i have too much workload and very stretched timelines. but nevertheless i was able to devote some time in making this project and posted it.

> this post is devided into subgroups
>> - esp32
>> - nrf52
>> - future improvements 

## eps32 
the esp32 is a great board and i been working with this micro. since last 3-4 months and what i can say that esp32 is a great board with very intuitive to use .because of the SDK documentations . what i want to really admire about the esp32 is its  SDK and the documentations . the documentation is very devlopers friendly and thread safe APIs. i dont want to bore down you with the implementations and theory of HID .since you can find better one on the internet . but to just share my thoughts about it. so BLE have multiple profiles and one of the profile is HID and for the smart devices like Latops, smartphones supports HID peripheral devices . these devices have the driver in the OS to natively support BLE HID device without any third party Apps . even Apple Macs, Ipad, iphones also support BLE HID devices . But to do that the peripheral devices have must have 2 profiles (Battery service + HID(included service Battery service )). also the peripheral must support the BLE paring(bonding procedure) .aka encryption to support auto connections . in this project the HID profile supports Mice + keyboard . plus the Boot protocol mode .
<br>
to support multiconnections , we first have to  configure the stack to support more than one connections . the esp32 uses bluedroid stack + menuconfig (for project configurations )  . the bluedroid is a (BR/EDR + BLE) stack .it is very heavy duty stack for the project but not a problem for esp32 😂. the API is very simple and easy to use . the HID device is act like a server and the client listens to it through BLE notifications . i want to implement a mouse in this project but dont have the optical sensor . so i used an accelometer (OVERKILL) ADXL345 for that . the mutliconnections is easy . since i want to support only two simultaneous device with this . so i start the advertisement again once the 1st device is connected and stop the advertisement when 2nd device got connected . automatically stopped in callback.
<br>
the gatt interface , the data base handle are same for both the devices but they have differnt connection ID . so to send the data to the target device you just have to change the connection ID . but maintaining mutiple connections put a lot of burden on the radio . the slave have to be in sync with multiple centrals . the slave have to respond to the central in the connection events . so the idleness of the radio decreases. but since the esp32 only have BLE 4.1 . not the best choice in todays world (latest 5.3). 
<br>
Also the esp32 consumes a lot more power like ~80 ma . in BLE transfer . because it uses a common radio modem for Wifi , Bluetooth and BLE . not an optimal choice for low power devices . That is also the reason why i shifted the microcontroller of the our  smart Device to nordic  platform the NRF52 .

## nrf52
so the NRF52832 is ARM cortex M4F based SOC with great hardware documentation. but i dont like its SDK( documentation). since its hardware documentation is so easy to understand . it took me just 15 days to switch to this Micro. and run blinky program, writing HAL layer . Also i do'nt like IDEs . thats why i did  the cross build  of whole project with CMAKE . i took the startup files + linker script and bitfields structure and definations from the SDK 😁. but it took me just 1 week to build and  run the application on this micro. so my next job is to port the previously built  libraries of the esp32 onto nordic + HAL layer + freertos integration + BLE.
<br>
the main wuestion arises here is the BLE stack. since i niether want  to use the NRF SDK  nor the NRF softdevice(BLE stack). because the norid softdevice is a precompiled + linked binaries . and you have to flash it separatly and use SVCs calls to run this stack . i want to compile and link the BLE stack as static library . luckily the Nimble stack comes into play. the Nimble is an apache Mynewt project and it is a lightwieght BLE stack (Host + Controller ) layer + drivers for Nrf52 radio only . its free and  opensource . plus it support freertos integration so optimal for my project .it have muticonnection support . the only problem is i hardly find any project based on nimble for nrf52 and how to build application with nimble for nordic micros. 
<br>
Also i dont want to use Zephyr's for the project . because of time constraints . i want that every thing is under my control and dont want to waste my time in debugging around unoknown SDk stuff. so in future i want to devlop this project even more . so it support more than 2 connections and switching between the connections using one button.


## future improvements
- devlop a one button library for the NRF so with long press the device start advertisement and with short press it switch between the connected devices
- using a very low power , lightwieght stack for this project like nimble and make it to support 4 connections 
- since encryption is used here for auto connection. use an external eeprom for storing the encryption keys. that also minimize the flash read,write cycles. 
- also include the device profile .so user able to change the device name with their smartphone . and next time the BLE device advertise itself with the newly changed name .
- Last but not least to support OTA DFU. to upgrade the firmware to increase the features or what not.
