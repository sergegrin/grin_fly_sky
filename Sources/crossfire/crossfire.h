// Modified 2026 by sergegrin
/*
 * Copyright (C) OpenTX
 *
 * Based on code named
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _CROSSFIRE_H_
#define _CROSSFIRE_H_

#include <inttypes.h>
#include "crc_crsf.h"

#define TELEMETRY_RX_PACKET_SIZE       128
#define TELEMETRY_OUTPUT_FIFO_SIZE     16

// Device address / frame start byte
#define UART_SYNC                      0xC8 // frame start from ExpressLRS 4.x
#define RADIO_ADDRESS                  0xEA // frame start from ExpressLRS 2.x/3.x
#define MODULE_ADDRESS                 0xEE

// Frame id
#define GPS_ID                         0x02
#define CF_VARIO_ID                    0x07
#define BATTERY_ID                     0x08
#define LINK_ID                        0x14
#define CHANNELS_ID                    0x16
#define LINK_RX_ID                     0x1C
#define LINK_TX_ID                     0x1D
#define ATTITUDE_ID                    0x1E
#define FLIGHT_MODE_ID                 0x21
#define RADIO_ID                       0x3A
#define COMMAND_ID                     0x32

#define SUBCOMMAND_CRSF                0x10
#define COMMAND_MODEL_SELECT_ID        0x05

enum CrossfireSensorIndexes {
  RX_RSSI1_INDEX,
  RX_RSSI2_INDEX,
  RX_QUALITY_INDEX,
  RX_SNR_INDEX,
  RX_ANTENNA_INDEX,
  RF_MODE_INDEX,
  TX_POWER_INDEX,
  TX_RSSI_INDEX,
  TX_QUALITY_INDEX,
  TX_SNR_INDEX,
  RX_RSSI_PERC_INDEX,
  RX_RF_POWER_INDEX,
  TX_RSSI_PERC_INDEX,
  TX_RF_POWER_INDEX,
  TX_FPS_INDEX,
  BATT_VOLTAGE_INDEX,
  BATT_CURRENT_INDEX,
  BATT_CAPACITY_INDEX,
  BATT_REMAINING_INDEX,
  GPS_LATITUDE_INDEX,
  GPS_LONGITUDE_INDEX,
  GPS_GROUND_SPEED_INDEX,
  GPS_HEADING_INDEX,
  GPS_ALTITUDE_INDEX,
  GPS_SATELLITES_INDEX,
  ATTITUDE_PITCH_INDEX,
  ATTITUDE_ROLL_INDEX,
  ATTITUDE_YAW_INDEX,
  FLIGHT_MODE_INDEX,
  VERTICAL_SPEED_INDEX,
  UNKNOWN_INDEX,
};

typedef uint_fast32_t tmr10ms_t;

#define CROSSFIRE_FRAME_MAXLEN         64
#define CROSSFIRE_CHANNELS_COUNT       16
#define CROSSFIRE_BAUDRATE             400000
#define CROSSFIRE_PERIOD               5 /* ms; 200 Hz */

void runCrossfireTelemetryCallback(uint8_t command, uint8_t* data, uint8_t length);
bool crossfireTelemetryPush(uint8_t command, uint8_t *data, uint8_t length);
void crsf_init();
void crsf_shutdown();
void crsf_action();

#endif // _CROSSFIRE_H_
