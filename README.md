# NRF52832 Bluetooth Libraries

This repository contains the libraries and code developed during our work at Sepahbod Company. The code is designed for the NRF52832 microcontroller, enabling Bluetooth functionality and button press handling. It also interfaces with external peripherals, including the AT24C256 EEPROM and the RTC PCF8563.

## Code Overview

The core of this repository consists of code designed to work with the NRF52832 microcontroller. It provides Bluetooth functionality and efficient management of button press events. Below are the key components and features of the code:

### Utilized Libraries

The code makes use of several libraries to simplify Bluetooth communication and event handling:

- `mbed.h`: The mbed Microcontroller Library.
- `platform/Callback.h`: Callback handling for mbed.
- `events/EventQueue.h`: Event queue for managing asynchronous events.
- `platform/NonCopyable.h`: A utility for creating non-copyable objects.
- `ble/BLE.h`: Bluetooth Low Energy (BLE) library.
- `ble/Gap.h`: GAP (Generic Access Profile) library for BLE.
- `ble/GattClient.h`: GATT (Generic Attribute Profile) client library.
- `ble/GapAdvertisingParams.h`: GAP advertising parameters.
- `ble/GapAdvertisingData.h`: GAP advertising data.
- `ble/GattServer.h`: GATT server library.

### Core Functionality

- The code defines an interrupt pin (`Interrupt_Pin`) to handle button press and release events.
- It supports both short and long button presses, distinguishing between them.
- Bluetooth functionality is initialized, and a custom service (`PinService`) is registered.
- The `PinService` offers characteristics for managing button presses and facilitating Bluetooth communication.
- The code manages read and write operations, enabling interactions with the NRF52832 microcontroller.

## Getting Started

To use this code in your NRF52832 project, follow these steps:

1. Clone this repository to your development environment.

2. Include the necessary libraries in your project. Ensure that you have the required mbed and BLE libraries set up.

3. Modify the code as needed to fit your specific project requirements. You can customize the Bluetooth service and characteristics according to your application.

4. Build and flash the code to your NRF52832 microcontroller.

5. Run your project, and the NRF52832 will initialize Bluetooth functionality and handle button press events.
