<p align="center">
  <img src="https://raw.githubusercontent.com/metisvela/sailtrack/main/assets/sailtrack-logo.svg" width="180">
</p>

<p align="center">
  <img src="https://img.shields.io/github/license/metisvela/sailtrack-radio" />
  <img src="https://img.shields.io/github/v/release/metisvela/sailtrack-radio" />
</p>

# SailTrack Radio

SailTrack Radio is a component of the SailTrack system, it manages GPS data and forwards messages from the SailTrack Network to [SailTrack Ground](https://github.com/metisvela/sailtrack-ground) using [LoRa](https://lora-alliance.org). To learn more about the SailTrack project, please visit the [project repository](https://github.com/metisvela/sailtrack).

The SailTrack Radio module is based on a battery powered LilyGo TTGO T-Beam, consisting of an [ESP32](https://www.espressif.com/en/products/socs/esp32) microcontroller connected to a GPS module and a LoRa transceiver. For a more detailed hardware description of the module, please refer to the [Bill Of Materials](hardware/BOM.csv). The 3D-printable enclosure con be found [here](hardware/STL).

The module performs the following tasks:

* It gets the positioning data coming from the onboard GPS module and sends them in the SailTrack Network.
* It receives the desired metrics from the SailTrack Network and forwards them to SailTrack Ground using LoRa.


<p align="center">
  <br/>
  <img src="hardware/Connection Diagram.svg">
</p>

![module-image](hardware/Module%20Image.jpg)

## Installation

Follow the instructions below to get the SailTrack Radio firmware correctly installed. If you encounter any problem, please [open an issue](https://github.com/metisvela/sailtrack-radio/issues/new).

1. [Install PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).
2. Clone the SailTrack Radio repository:
   ```
   git clone https://github.com/metisvela/sailtrack-radio.git 
   ``` 
3. Cd into the directory:
   ```
   cd sailtrack-radio
   ```
4. **(macOS ONLY)** Uncomment the commented lines after "Patch for macOS" in the `platformio.ini` file.
5. Connect the module with an USB cable.
6. Finally, flash the firmware:
   ```
   pio run
   ```

## Usage

Once the firmware is uploaded the module can work with the SailTrack system. When SailTrack Radio is turned on, a LED start to blink to notify the user about the connection status with SailTrack Core. Then, if the connection is successful, the LED stays on, otherwise the module will put itself to sleep, and it will try to connect later. Once the module is connected it will automatically start sending measurements.
## LED Color Codes

The table below outlines the different LED color codes used in the project. Each color represents a specific status or condition, providing a clear and intuitive way to monitor the device's current operation. The hexadecimal values correspond to the RGB configuration for the LED. Use this table as a reference to understand what each LED color indicates.

| LED Color      | Color Code (Hex) | Description                                     |
|----------------|------------------|-------------------------------------------------|
| **Green**      | 0x0000FF00       | Indicates that the device is charging.          |
| **Red**        | 0x00FF0000       | Warns that the battery is low (≤ 20%).          |
| **Blue**       | 0x000000FF       | Shows the GPS has acquired a 3D fix (≥ 3).      |
|                |                  | The battery level ranges from 20% to 90%.       |
| **Yellow**     | 0x00FFFF00       | Indicates the GPS has not yet acquired a 3D fix.|
|                |                  | The battery level ranges from 20% to 90%.       |
| **Magenta**    | 0x00FF00FF       | Signals that the battery is almost full (≥ 90%).|
| **Off**        | 0x00000000       | No activity or special condition is detected.   |


## Device Charging Process

To optimize the charging process, it is recommended to use a **magnetic port charger**. This allows you to charge the device without the need to open its enclosure, making the process more convenient and reducing wear on the hardware.

### Common Charging Issue

The device may encounter charging issues if it is reset and turned on while not connected to a power source. In this scenario, the device might fail to charge the battery correctly once it is connected. To avoid this problem, ensure that the device is charging before performing a reset.

## Contributing

Contributors are welcome. If you are a student of the University of Padova, please apply for the Metis Sailing Team in the [website](http://metisvela.dii.unipd.it), specifying in the appliaction form that you are interested in contributing to the SailTrack Project. If you are not a student of the University of Padova, feel free to open Pull Requests and Issues to contribute to the project.

## License

Copyright © 2023, [Metis Sailing Team](https://github.com/metisvela). SailTrack Radio is available under the [GPL-3.0 license](https://www.gnu.org/licenses/gpl-3.0.en.html). See the LICENSE file for more info. 
