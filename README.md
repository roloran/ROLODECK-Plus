# ROLORAN Neuhaus Mobile Device Firmware for RDCP v0.4

This repository holds the LilyGo T-Deck firmware implementation for RDCP v0.4 mobile devices (MGs) in the Neuhaus scenario 2026.

The ROLORAN Disaster Communication Protocol (RDCP) is specified in a separate [repository](https://github.com/roloran/RDCP-Specs).

In the Neuhaus scenario, RDCP uses three different kinds of RDCP-capable devices: HQs, DAs, and MGs. MGs are mobile devices for citizens and blue-light organizations to be used in regional crisis situations, including longer black-outs. MGs have been implemented as part of the dtec.bw (#NextGenerationEU) research project [ROLORAN](https://www.unibw.de/software-security/forschung/roloran). LilyGo T-Deck devices are used as commercial-off-the-shelf (COTS) LoRa devices which combine a Semtech SX1262 LoRa radio with a hardware keyboard and touch-screen display at a moderate price.

The RDCP firmware enables citizens to receive RDCP Official Announcements during crisis situations and also send RDCP Citizen Reports, including medical emergencies that trigger emergency aid via the HQ using the DAs in the RDCP Infrastructure as Relays.

The firmware in this repository is the prototype RDCP MG implementation for the Neuhaus scenario, initially deployed for field testing in the third phase of the ROLORAN-Neuhaus cooperation in 2026. It might serve as a blueprint basis for further development and deployments in comparable RDCP scenarios.

## Dependencies, Compilation, and Flashing

The supported build environment is [PlatformIO](https://platformio.org/). Install PlatformIO Core or use the PlatformIO extension for your IDE; no Arduino IDE setup and no machine-global Arduino libraries are required.

The default `tdeck-plus-core3` environment uses Pioarduino's pinned `platform-espressif32` 55.03.39 release, which supplies Arduino-ESP32 3.3.9 and ESP-IDF 5.5.4. The migration has been qualified on a LilyGo T-Deck Plus. The retained `tdeck-plus-core2` fallback uses the official `platformio/espressif32@6.12.0` platform with Arduino-ESP32 2.0.17, allowing regressions to be compared against the former production toolchain.

The PlatformIO environment also fixes the hardware settings previously passed as Arduino board options:

- ESP32-S3 at 240 MHz
- USB CDC enabled on boot
- 16 MB flash at 80 MHz
- QIO boot mode with DIO application images
- Two 3 MB OTA application slots and the remaining flash assigned to persistent storage
- No PSRAM use, matching the proven firmware configuration

Library dependencies are installed automatically and pinned to exact releases or Git commits in `platformio.ini`. Both environments use RadioLib 7.7.1 for the SX1262, LVGL 8.4.0, TinyGPSPlus 1.0.3, Operator Foundation Crypto 0.4.0, and pinned Unishox and ROLORAN SchnorrSig commits. Core 3 uses GFX Library for Arduino 1.6.6 and ESP8266Audio 2.4.1; the fallback retains GFX 1.6.0 and ESP8266Audio 2.0.0.

The project-local `lib/ESP32_BleSerial` is based on ESP32_BleSerial 3.0.1 and carries the small compatibility adjustments required by Arduino-ESP32 3.3.9's NimBLE implementation. The known-good T-Deck GT911 TouchLib snapshot is stored under `lib/TouchLib`, while the LVGL configuration and fixed flash partition table are stored under `include/` and `partitions/`. The adjusted Base64 implementation remains part of this project. Core-3-only build compatibility settings are kept in `scripts/pio_core3_compat.py`.

On the T-Deck's shared SPI pins, the display uses GFX's `Arduino_HWSPI` adapter with the global Arduino `SPI` instance. RadioLib uses that same instance for the SX1262, and firmware-level locking serializes access by the display and radio. This is required on ESP-IDF 5; GFX's ESP32-specific direct-register bus can hang when a second SPI peripheral is mapped onto those pins. Startup also runs an AES-256-GCM known-answer test so an incompatible cryptographic backend is reported immediately over serial.

Build the default production environment with:

```bash
./compile.sh
```

The equivalent direct command is:

```bash
pio run -e tdeck-plus-core3
```

Identify the connected device using `pio device list`, then upload by passing its current port explicitly:

```bash
DEVICE="/dev/your-t-deck-device-here"
pio run -e tdeck-plus-core3 -t upload --upload-port "$DEVICE"
```

The serial monitor is configured for 115200/8N1. DTR and RTS are disabled in `platformio.ini` because toggling them while opening the monitor can reset the ESP32-S3 or interfere with T-Deck peripheral initialization:

```bash
pio device monitor -e tdeck-plus-core3 --port "$DEVICE"
```

To build the retained Arduino-ESP32 2.0.17 fallback, select it explicitly:

```bash
PIO_ENV=tdeck-plus-core2 ./compile.sh
# or: pio run -e tdeck-plus-core2
```

The device port is deliberately not committed because operating systems can assign a different name after reconnecting it. Initially flashing a T-Deck with its factory firmware may require holding the trackball button during power-on; consult LilyGo's T-Deck documentation when entering the bootloader manually.


## Initial Device Configuration

When a T-Deck has been flashed with our firmware for the first time, it cannot be used without initial configuration via a Serial/UART connection first and will show the "requires maintenance" UI screen. It is recommended to use the [ROLORAN-Terminal](https://github.com/roloran/ROLORAN-Terminal) to access the T-Deck for this purpose.

Initial configuration is part of the "provisioning" process (see [RDCP specs](https://github.com/roloran/RDCP-Specs)) and consists of the "base configuration" workflow outlined below. It is recommended to use ROLORAN-Terminal's scripting capabilities to setup a larger number of devices.

Configuration settings are stored persistently on the T-Deck and restored automatically during the power-on process. Initial configuration needs to be performed again after a device has been wiped explicitly.

## Device Usage

T-Decks are primarily intended as end-user devices participating in RDCP infrastructures, but can also be used for testing and maintenance operations by authorized personnel. Both types of usage are outlined below.

### Interactive (GUI) Usage

As end-user devices, the primary purpose of T-Decks is to display Official Announcements (according to RDCP specs). RDCP infrastructures can be operated in `crisis` and `non-crisis` modes. The major difference is that in `crisis` mode, end-user devices can also be used to send Citizen Request (CIRE) messages according to RDCP specs, while `non-crisis` mode only allows for one-way communication, i.e., citizens cannot send messages back to the HQ.

Device usage typically runs along the following steps:

- An end-user turns the device on. As the optional built-in battery has a very limited capacity, typically external "power banks" are used for this purpose. The T-Decks can also be connected to a USB-C charging device; however, given the worst-case assumption of a "black-out crisis", permanent external power sources are considered to be unavailable.

- After power-on, provisioned devices optionally show some kind of "end-user agreement" screen to let the end-users know that other, still available means of communication (e.g., mobile phone network, landlines, ...) should be preferred, and sending emergencies via RDCP should only be considered as the "last resort" option. It is worth keeping in mind that we operate RDCP infrastructures as a part of an ongoing university research project, not for production use with any kind of legally binding guarantees.

- End-user devices always start in `crisis` mode to allow for sending CIREs quickly after turning the device on, especially in real-world emergency situations. Once the device receives a `non-crisis` message (e.g., a non-crisis official announcement or a status information as part of timestamp messages), it switches to `non-crisis` mode.

- In `non-crisis` mode, the end-user is shown a list of persistently stored `non-crisis` official announcements. New OAs, such as those periodically repeated by DAs or new messages sent by the HQ, are appended to this list. If the device has already received a timestamp message, the date and time of when a message was received are added to the displayed messages along with the information whether a valid cryptographic signature has been received for each message.

- In `crisis` mode, end-users have two additional buttons available (one in red and one in blue) to send emergency and other CIRE messages to the HQ. When using either of them, the UI screen changes to display form fields that have to be filled in (such as "where is the emergency?" and "what happened?"). The form fields and overall design are developed together with Neuhaus' HQ operators to make end-user devices somewhat intuitive to use while still providing all the information that HQ needs to react to new CIREs appropriately.

- Only one CIRE may be sent at a time. `Crisis`-mode buttons are greyed out until an Acknowledgment message has been received from the HQ or the end-user device determines that the message currently cannot be delivered. Similarly, when HQ sends an Inquiry OA, it must be answered before further CIREs can be sent.

Unfortunately, in our experience, sometimes T-Deck hardware components fail to initialize properly after power-on or may even fail at run-time. If the software detects that components are not working properly, it shows a UI screen that asks the end-user to power-cycle the device, which usually fixes the issue somewhat reliably (a software-side reboot, i.e., without a power-cycle, does not suffice). However, if, for example, the device's touch screen stops to work after some time, end-users need to perform the power-cycle after becoming aware of the problem.

To conserve energy, the display is turned off after an inactivity period. It is turned on automatically again when a new OA is received or when the device is used interactively. Unfortunately, the pinout of current T-Deck hardware versions does not allow for the use of ESP32 sleep modes; for example, the SX1262's DIO1 pin could not be used to wake the device from light or deep sleep. Thus, even with other energy-saving options in place (such as lowering CPU frequency), T-Decks continuously consume about 0.4 W even when idle, depleting typical power banks within a few days. We'd be happy to learn about how this situation can be improved.

### Serial Device Access (UART, USB-CDC)

The devices can be accessed over their USB-C port for a Serial/UART connection. Parameters are fixed to 115200/8N1 and it is recommended to use the [ROLORAN-Terminal](https://github.com/roloran/ROLORAN-Terminal) for this purpose.

Provisioned devices dump parts of their configuration (e.g., LoRa parameters, RDCP address) after power-on over this channel. Unprovisioned or unlocked devices accept the following commands:

- `RESET` commands:
  - `RESET CONFIG` removes persisted commands for replay outside basecfg.
  - `RESET RADIO` re-initalizes the LoRa radio with the current settings.
  - `RESET BASECFG` wipes the device. It will require a power-cycle and re-provisioning afterwards.
  - `RESET ENTRYPOINTS` deletes all stored RDCP Entry Points.
  - `RESET MULTICAST` deletes all stored RDCP multicast addresses.
  - `RESET MESSAGES` deletes all messages from both `crisis` and `non-crisis` message boards.
  - `RESET FILESYSTEM` formats the FFat file system while trying to keep the configuration.

- `SHOW` commands:
  - `SHOW CONFIG` outputs the current device configuration (same output as after device power-on).
  - `SHOW MESSAGES` lists the messages stored in the internal message boards.

- LoRa parameter commands:
  - `LORAFREQ` sets the LoRa channel frequency in MHz, e.g., `LORAFREQ 868.2`
  - `LORABW` sets the LoRa channel bandwidth in KHz, e.g., `LORABW 125`
  - `LORASF` sets the LoRa spreading factor, e.g., `LORASF 12` (accepted values: 7 to 12)
  - `LORACR` sets the LoRa coding rate to x/8, e.g., `LORACR 5` (accepted values: 5 to 8, as in RadioLib)
  - `LORASW` sets the LoRa syncword (hexadecimal), e.g., `LORASW 12` (accepted values: 12 and 34)
  - `LORAPW` sets the LoRa TX power, e.g., `LORAPW 14` (accepted values: up to 22 with T-Deck's SX1262)
  - `LORAPL` sets the LoRa preamble length, e.g., `LORAPL 15` (15 is the default value in our scenarios)
  - `LORAFREQTX` sets the LoRa TX channel frequency in MHz, e.g., `LORAFREQTX 868.2`
  - `LORABWTX` sets the LoRa TX channel bandwidth in KHz, e.g., `LORABWTX 125`
  - `LORASFTX` sets the LoRa TX channel spreading factor, e.g., `LORASFTX 12` (accepted values: 7 to 12)
  - `LORACRTX` sets the LoRa TX channel coding rate to x/8, e.g., `LORACRTX 5` (accepted values: 5 to 8, as in RadioLib)
  - `LORASWTX` sets the LoRa TX channel syncword (hexadecimal), e.g., `LORASWTX 12` (accepted values: 12 and 34)
  - `LORAPWTX` sets the LoRa TX channel power, e.g., `LORAPWTX 14` (accepted values: up to 22 with T-Deck's SX1262)
  - `LORAPLTX` sets the LoRa TX channel preamble length, e.g., `LORAPLTX 15` (15 is the default value in our scenarios)

- `TX <base64>` queues an outgoing RDCP message provided as Base64-encoded string.

- `TXNOW <base64>` queues an outgoing RDCP message for immediate sending (use with care!).

- `TXSCHED <ms> <base64>` queues an outgoing RDCP message to be sent after the delay given in milliseconds.

- `SIMRX <base64>` simulates receiving a LoRa packet whose payload is given Base64-encoded. This is solely intended for the purpose of testing how the device reacts to a specific RDCP message without actually transmitting it "over the air" (e.g., under duty cycle considerations).

- `RESTART` triggers a reboot of the device (not a full power-cycle!).

- `BASECFG` starts the base configuration procedure for an unprovisioned device. After this command, any of the other commands can be sent and they will be stored persistently on the device to be executed after each power-on. Similar to how mails are sent through SMTP, a single `.` on a line of its own will end the `BASECFG` mode. The device should then be power-cycled and Serial/UART output should be checked for any error messages such a configuration setting may give. It is recommended to use configuration templates along with ROLORAN-Terminal's scripting capabilities for provisioning. Base configuration should include all of the `LORA*` commands if they deviate from the firmware's default settings in a scenario. `BASECFG` _must_ include `RDCPADDR`, setting at least one `ENTRYPOINT`, the `HQPUBKEY` and the `SHAREDSECRET`. If the device shall become part of at least one multicast group, the `MULTICAST` command must be used. It is recommended to set an `OWNER` if this information is available during provisioning. Other commands such as `HEARTBEAT`, `GRACETIME`, `CIRETIMEDA`, `CIRETIMEHQ`, and `EULA` _may_ be used to deviate from default settings. Please note that a faulty `BASECFG` can render the device useless and unconfigurable. If you brick it, you need to flash a new firmware that initially removes the base configuration. This is not a toy command.

- `RDCP` commands:
  - `RDCPIDUP` toggles whether the device ignores or filters duplicated RDCP messages (based on Origin and SequenceNumber). Use with care, as most of the firmware implementation relies on duplicate filtering quite heavily. You've been warned.
  - `RDCPADDR` sets the RDCP address the device should use as its own, e.g., `RDCPADDR AEFF`.
  - `ENTRYPOINT` adds an Entry Point for the device, usually the nearest DA, e.g., `RDCPADDR 0200`; can be used up to 5 times. Use `RESET ENTRYPOINTS` to remove the existing setting if you want to replace instead of to add.
  - `MULTICAST` adds the device to a multicast group, e.g., `MULTICAST AF00`; can be used up to 5 times. See also `RESET MULTICAST`.
  - `HEARTBEAT` sets the time interval between Heartbeat messages sent by the device in minutes; default is `HEARTBEAT 30`.
  - `GRACETIME` sets the grace period in seconds; default is `GRACETIME 60`. A device will not send a Heartbeat message for this duration after power-on in order to first listen to other RDCP messages and update its channel free estimator.
  - `CIRETIMEDA` sets the time to wait for an Acknowledgment message by the Entry Point after sending a CIRE in seconds. See the RDCP specs [section about CIREs](https://github.com/roloran/RDCP-Specs/blob/main/rdcp_v04_overview.md#rdcp-messages-for-citizen-reports).
  - `CIRETIMEHQ` sets the time to wait for an Acknowledgment message by the HQ after sending a CIRE in seconds.
  - `RDCPPING` sends an RDCP Echo Request message to another device, e.g., `RDCPPING 0200`.
  - `RDCPTEST` sends an RDCP Test message to a given RDCP address, e.g., `RDCPTEST FFFF Hello world`.
  - `RDCPCIRE` sends an RDCP Citizen Request message to the HQ. Subtype and ReferenceNumber must be given along with a text in the format that the HQ expects, e.g., `RDCPCIRE 00 0001 !#...`.
  - `RDCPACKS` sends a signed Acknowledgment message; this only works if a private key is stored on the device. The command expects Origin, Destination, acknowledged SequenceNumber, and Acknowledgment type as parameters. Note that usually only the HQ sends such messages and the HQ's private key must not be stored on end-user devices.
  - `RDCPACK` sends an (unsigned) Acknowledgment message; same as `RDCPACKS` but without the Schnorr signature. Note that usually only DAs send such acknowledgments.
  - `RDCPTIME` sends a signed Timestamp message to the broadcast address, e.g., `RDCPTIME 00 12 31 23 59 02` (see RDCP specs). Note that usually only the HQ sends such messages and other devices only accept it if properly signed.

- Other device settings:
  - `SCREENSAVER` sets the delay before the screen is turned off when the device is idle in milliseconds. Default is `SCREENSAVER 30000`.
  - `UISCREEN` switches to a different UI screen, e.g., `UISCREEN 1`. This is only useful for UI testing or when giving device demonstrations. See `tdeck_gui.h` for the UI screen numbers.
  - `EULA` toggles whether the "end user license" with the note to only use CIREs as a method of last resort should be shown on power-on.
  - `OWNER` sets the device owner's name. It is used to pre-fill the "who reports?" CIRE form fields.
  - `HQPUBKEY` sets the HQ's public key used to verify Schnorr signatures. Uses a sequence of hex digits as exported by the Schnorr stand-alone tool.
  - `MYPUBKEY` sets this device's public key. Only used for maintenance devices that may create signed RDCP messages. Not used for end-user devices.
  - `MYPRIVKEY` sets this device's private key, corresponding to `MYPUBKEY`. Not used for end-user devices.
  - `SHAREDSECRET` sets the shared secret with the HQ, a 256-bit AES key, given as 32 hex digits.
  - `SETTIME` uses the same syntax as `RDCPTIME` but only sets the local device wallclock. Useful for some tests if the next official Timestamp message is not expected anytime soon after device power-on.
  - `UNLOCK` changes the Serial/UART access mode based on a given password, e.g., `UNLOCK nicetry`. The higher the unlocked level the more information will be output via Serial/UART and the more commands can be used. Repeated failed unlock attempts may brick the device. Passwords can be set with the `UNLOCKHASH1` and `UNLOCKHASH2` commands.
  - `RINGTONE 0` disables the short melody when new messages are displayed.
  - `NOTE` can be used to add an infix to each Serial output text line.
  - `MISTAKESWEREMADE` bricks the device.
  - `RDCPSFMUL n` sets the RDCP spreading-factor-based time multiplier, e.g., 1 for SF7 and 10 for SF12.
  - `CORRIDOR 60` sets the corridor time in seconds, i.e., how long to keep the channel free after hearing a `CIRE` to make room for the `DA` `ACK`.
  - `HQMODE` toggles the device's HQ mode. The ROLODECK devices used for `HQ` devices should enable this mode to facilitate the `HQ` software regarding channel free estimations.

Some of those commands are automatically persisted on the device and replayed after power-on, so they don't have to be repeated every time the device is used. Those replayed commands override even `BASECFG` settings. Using `!` as a prefix for the command prevents it from being echoed back.

Unlocked devices show received LoRa packets using the `RX <base64>` and `RXMETA` lines as the other ROLORAN LoRa modems do.

Sending an empty input line to an unlocked T-Deck outputs some current status information, e.g., channel free estimation and number of queued messages.

## Source code overview

The repository follows PlatformIO's standard project layout:

```text
.
├── include/       Project headers and lv_conf.h
├── lib/           Project-local libraries, currently the T-Deck TouchLib snapshot
├── partitions/    Frozen 16 MB flash partition table
├── src/           Firmware C/C++ implementation and generated UI resources
├── platformio.ini Pinned board, framework, build, monitor, and dependency settings
└── compile.sh      Convenience wrapper for the production PlatformIO environment
```

Important implementation units are:

- `src/main.cpp` contains the Arduino `setup()` and `loop()` entry points.
- `roloran-tdeck-hal.*` provides the T-Deck Plus hardware abstraction, shared SPI coordination, display, touch, keyboard and trackball handling.
- `roloran-tdeck-serial.*` implements Serial/UART and BLE serial input and output.
- `roloran-tdeck-persistence.*` manages persistent device configuration, messages, sequence numbers and nonces.
- `roloran-lora.*` accesses the SX1262 through RadioLib while coordinating the SPI bus shared with the display.
- `roloran-board.*` manages crisis and non-crisis messages displayed to users.
- `roloran-crypto.*` contains AES-GCM and Schnorr signature integration.
- `settings-device.*` and `settings-scenario.h` contain device and scenario configuration.
- `tdeck_gui.*` and the generated `ui*` files implement the LVGL user interface, fonts and images.
- `roloran-audio.*` plays the notification melody through the T-Deck Plus speaker.
- `Base64ren.*` is the locally adjusted Base64 implementation.

## Limitations and the way ahead

RDCP MGs in the Neuhaus scenario are currently intended to be used as a prototype by citizens and selected authority personnel. Its interactive user interface (GUI) will evolve based on experience gathered in the ROLORAN research project and the use of its Serial/UART/USB-CDC interface is limited to technical staff operating the RDCP infrastructure. Intuitive user experiences and reliable, stable operations are currently the primary goals. Secondary future objectives may involve additional functionality and interfaces (e.g., coupling with smartphones via Bluetooth) as well as cooperation with commercial vendors interested in creating more than a research prototype.

<!-- EOF -->
