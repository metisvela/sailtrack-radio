<p align="center">
  <img src="https://raw.githubusercontent.com/metis-vela-unipd/sailtrack-docs/main/Assets/SailTrack%20Logo.png" width="180">
</p>

<p align="center">
  <img src="https://img.shields.io/github/license/metis-vela-unipd/sailtrack-radio" />
  <img src="https://img.shields.io/github/v/release/metis-vela-unipd/sailtrack-radio" />
  <img src="https://img.shields.io/github/workflow/status/metis-vela-unipd/sailtrack-core/Publish%20Release" />
</p>

# SailTrack Radio

SailTrack Radio is a component of the SailTrack system, it manages gps data and sends message from the SailTrack system to SailTrack Ground. To learn more about the SailTrack project, please visit the [documentation repository](https://github.com/metis-vela-unipd/sailtrack-docs), 

The SailTrack Radio module is based on a battery powered LilyGo TTGO T-Beam connected to an external GPS antenna and an external LoRa antenna. For a more detailed hardware description of the module, please refer to the [Bill Of Materials](https://github.com/metis-vela-unipd/sailtrack-radio/blob/main/hardware/BOM.csv).

<p align="center">
  <br/>
  <img src="https://raw.githubusercontent.com/metis-vela-unipd/sailtrack-radio/main/hardware/connection-diagram.svg">
</p>

## Installation
 1. Download and install [PlatformIO Core](https://docs.platformio.org/en/latest/core/index.html).  
 2. Clone the [SailTrack Radio](https://github.com/metis-vela-unipd/sailtrack-radio) repository.
 3. Flash the firmware with the following command line.
```bash
pio run 
```

 
 ## Usage
Once the firmware is uploaded the sensor can work with the SailTrack system. When SailTrack Radio is turned on, a LED start to blink to notify the user about the connection status with SailTrack Core, then if the connection is successful the LED stays on for, if the connection fails  the sensor will put itself to sleep, and it will try to connect later.

## Contributing

Pull requests are welcome. For major changes, please [open an issue](https://github.com/metis-vela-unipd/sailtrack-radio/issues/new) first to discuss what you would like to change.

## License

Copyright © 2022, [Métis Vela Unipd](https://github.com/metis-vela-unipd). SailTrack-Radio is available under the [GPL-3.0 license](https://www.gnu.org/licenses/gpl-3.0.en.html). See the LICENSE file for more info. 
