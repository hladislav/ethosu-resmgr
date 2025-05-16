/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_UTILS_H
#define ETHOSU_UTILS_H

/**
 * Defines
 */

#define IMX_SIP_SRC                   0xC2000005
#define IMX_SIP_SRC_M33_START         0x00
#define IMX_SIP_SRC_M33_STARTED       0x01
#define IMX_SIP_SRC_M33_STOP          0x02

/**
 * Functions
 */

int start_mcore();

int stop_mcore();

int reset_mcore();

#endif /* ETHOSU_UTILS_H */