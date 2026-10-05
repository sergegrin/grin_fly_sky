# Grin Fly Sky — firmware for the FlySky FS-i6

Custom firmware for the FlySky FS-i6 transmitter (NXP Kinetis MKL16Z64, 64 KB flash,
8 KB RAM). It drives the stock internal RF module and an external ExpressLRS TX
module over CRSF. The active module is chosen per model in MODEL SETUP → PROTOCOL;
only one module runs at a time.

This is a modified version of **ErFly6** (er9x for the FS-i6), derived from
[aerror2/erfly6](https://github.com/aerror2/erfly6). The original authors and their
projects are listed under [Credits](#credits); the modifications are listed under
[Changes from aerror2/erfly6](#changes-from-aerror2erfly6).

> **Status:** the ExpressLRS 3.x/4.x support and the other changes are built and
> tested against an emulated module only. Test them on your radio before you fly.

## Features

- **Protocols:** PPM, AFHDS and AFHDS2A on the internal A7105 module; `CRSF` to an
  external ExpressLRS TX module.
- **ExpressLRS configuration menu** (MODEL SETUP → PROTOCOL → `[CRSF Setup]`). It
  speaks the same protocol as the ExpressLRS `elrs.lua` script and works with
  ExpressLRS 2.x, 3.x and 4.x TX modules, including dual band (LR1121) modules:
  - selections, numeric values, folders and commands such as Bind and Enable WiFi;
  - link statistics (`bad/good` packets and `C` when connected) and module
    warnings or errors, for example "Model Mismatch";
  - Model Match: the handset sends the model number as the model ID (model 1 = ID 1).
- **CRSF telemetry** shown on the er9x telemetry screens.
- **DFPlayer voice module** on a UART.

**ELRS menu limitations:**
- Only the TX module can be configured. There is no "Other Devices" folder.
- 16-bit and floating point parameters are hidden.
- Long option texts are shortened, for example `250Hz(-108dBm)` is shown as `250Hz`.
- Up to 46 module parameters are shown (an ExpressLRS 4.1 TX module has about 37).
- CRSF frames are sent at a fixed 200 Hz. The rate is not synchronised to the
  module's packet rate.

## Connecting an external ELRS TX module

No hardware modification is needed:

1. Connect the SPORT signal and GND of the FS-i6 to the CRSF input of the ELRS TX
   module (see the picture below). Power the module from its own supply.
2. Select `CRSF` in MODEL SETUP → PROTOCOL.
3. Open `[CRSF Setup]` on the same screen to configure the module. A long press
   on EXIT leaves the menu.

<img src="https://github.com/aerror2/erfly6/blob/main/docimg/tx_sport.jpg">

## Voice module (DFPlayer)

1. Connect a DFPlayer Mini module as shown below and power it from 3.3 V.
2. Create an `mp3` folder on a microSD card and copy the voice files from the
   [er9x voice pack](https://github.com/aerror2/erfly6/tree/main/VoicePackEr9x-22Khz_16bit-Sharon-Eng)
   into it.
3. On the radio, go to RADIO SETUP → AudioHaptic → Sound Mode and choose any mode
   other than "Beeper", for example `PiSpkrVoice`.
4. Restart the radio. You should hear "welcome to er9x".

<img src="https://github.com/aerror2/erfly6/blob/main/docimg/voice_mp3_module.jpg">

## Building

You need the Arm GNU Toolchain (`arm-none-eabi-gcc`) and GNU make:

    make            # build/FSI6.elf, build/FSI6.hex, build/FSI6.bin
    make clean

In VS Code, the tasks Build, Rebuild and Flash (J-Link) are available under
Terminal → Run Build Task. Set `fsi6.jlinkDir` in the settings to your J-Link
installation folder. Flashing uses `gcc/flash.jlink`.

For how to connect the programmer to the radio, see Kotello's
[flashing manual](https://github.com/aerror2/erfly6/blob/main/ER9XFlySky%20I6En.pdf).

## Changes from aerror2/erfly6

Changes made in October 2026 by sergegrin. Each modified source file is marked with
"Modified 2026 by sergegrin".

- **Build:** new GCC/make build. The unused embedded SX127x (LoRa) ELRS code and
  the unused NXP SDK drivers were removed.
- **Dead code:** dead code and preprocessor branches left over from the AVR er9x
  were removed. These removals were checked to produce a byte-identical binary.
- **CRSF:**
  - exact 400 kbaud (the UART previously ran at about 428 kbaud);
  - frames written by the menu can no longer be sent half-built by the pulse
    interrupt;
  - the CRC uses a bitwise routine instead of lookup tables;
  - frames from ExpressLRS 4.x modules, which start with the `0xC8` sync byte, are
    accepted (before, telemetry and the menu stayed silent with 4.x modules).
- **Protocol list:** the identical `ELRS2` and `ELRS1` entries are merged into one
  `CRSF` entry; models saved with either of them load as `CRSF`.
- **ELRS menu:** rewritten for the ExpressLRS 3.x/4.x parameter protocol. Changes:
  - bounds checks on all buffers;
  - blank options are skipped;
  - units, numeric fields and link statistics are shown;
  - module warnings are displayed;
  - Model Match is supported;
  - text is clipped at the screen edge, and a value wider than its column moves
    left over the parameter name;
  - the handset uses address `0xEA`, like `elrs.lua` for ExpressLRS 2.x–4.x;
  - up to 46 parameters (was 30), so the last items such as Bind are no longer cut off;
  - long option lists (dual band modules) are shortened while they are received;
  - a parameter write waits for the output buffer instead of being lost.

## Credits

This firmware exists thanks to the following projects and their authors:

- **er9x** by Erez Raviv, Mike Blandford and contributors, based on **th9x** by
  Thomas Husterer and on **gruvin9x**:
  http://code.google.com/p/er9x, http://code.google.com/p/th9x,
  http://code.google.com/p/gruvin9x
- **ErFly6**, the er9x port to the FS-i6 and FS-i6X, by Kotello:
  https://github.com/KotelloRC/erfly6 and the
  [RC Groups thread](https://www.rcgroups.com/forums/showthread.php?3961635-ER9X-for-FS-I6-and-FS-I6X-(ERFly6))
- **aerror2/erfly6**, which added the CRSF/ELRS output and the voice module. This
  project is derived from it: https://github.com/aerror2/erfly6
- **OpenTX** and **OpenI6X**, the source of the CRSF code, ported from OpenI6X by
  Maria and Janek: https://github.com/opentx/opentx, https://github.com/OpenI6X/opentx
- **ExpressLRS**: the configuration menu follows the ExpressLRS Lua scripts
  (`elrsV2.lua`, `elrsV3.lua` and `elrs.lua`, "Copyright (C) OpenTX, adapted for
  ExpressLRS"):
  https://github.com/ExpressLRS/ExpressLRS
- **DIY Multiprotocol TX Module**, the source of the A7105 / AFHDS2A routines:
  https://github.com/pascallanger/DIY-Multiprotocol-TX-Module
- **Arm CMSIS** and **Freescale / NXP** device support files.

## License

This is free software. The modifications are Copyright (C) 2026 sergegrin; all other
code remains the copyright of its respective authors. Each source file keeps its
original license header:

- **er9x, ErFly6 and OpenTX derived code:** GNU General Public License version 2.
  See [LICENSE](LICENSE).
- **A7105 / AFHDS2A files** (`A7105_SPI.cpp`, `AFHDS2A_a7105.cpp`,
  `iface_a7105.h`): GNU General Public License version 3 or later,
  https://www.gnu.org/licenses/gpl-3.0.html
- **`en.h` and `language.h`:** BSD-style license, Copyright (c) 2013 Michael
  Blandford. The notice is kept in the files.
- **`CMSIS_4/`:** Arm and Freescale/NXP BSD-style licenses.
- **`Kinetis_KL/`:** Freescale/NXP. See the file headers.

The GPLv2-only and GPLv3 parts listed above are not license-compatible in a single
binary. This combination was inherited from the upstream projects (ErFly6 and
aerror2/erfly6) and was not introduced by this project. For that reason this project
is published as source code only: no prebuilt firmware binaries are provided, and
redistributing a built firmware image is not recommended. Build the firmware yourself
for your own radio (see [Building](#building)).

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

FlySky, ExpressLRS and other product names are used only to describe compatibility.
This project is not affiliated with or endorsed by their owners.
