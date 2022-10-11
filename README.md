<p align="center">
  <img src="https://raw.githubusercontent.com/metis-vela-unipd/sailtrack-docs/main/Assets/SailTrack%20Logo.svg" width="180">
</p>

<p align="center">
  <img src="https://img.shields.io/github/license/metis-vela-unipd/sailtrack-radio" />
  <img src="https://img.shields.io/github/v/release/metis-vela-unipd/sailtrack-radio" />
</p>

# SailTrack Radio

SailTrack Radio is a component of the SailTrack system, it manages GPS data and forwards messages from the SailTrack Network to [SailTrack Ground](https://github.com/metis-vela-unipd/sailtrack-ground) using [LoRa](https://lora-alliance.org). To learn more about the SailTrack project, please visit the [documentation repository](https://github.com/metis-vela-unipd/sailtrack-docs).

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

Follow the instructions below to get the SailTrack Radio firmware correctly installed. If you encounter any problem, please [open an issue](https://github.com/metis-vela-unipd/sailtrack-radio/issues/new).

1. [Install PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html).
2. Clone the SailTrack Radio repository:
   ```
   git clone https://github.com/metis-vela-unipd/sailtrack-radio.git 
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

## Contributing

Contributors are welcome. If you are a student of the University of Padua, please apply for the Métis Vela Unipd team in the [website](http://metisvela.dii.unipd.it), specifying in the appliaction form that you are interested in contributing to the SailTrack Project. If you are not a student of the University of Padua, feel free to open Pull Requests and Issues to contribute to the project.

## License

Copyright © 2022, [Métis Vela Unipd](https://github.com/metis-vela-unipd). SailTrack Radio is available under the [GPL-3.0 license](https://www.gnu.org/licenses/gpl-3.0.en.html). See the LICENSE file for more info. 
