/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <unistd.h>
#include <stdio.h>

#include "ethosu.h"
#include "ethosu_utils.h"

#include <imx_smc_call.h>

int stop_mcore() {
    if (imx_sec_firmware_psci(
        IMX_SIP_SRC,
        IMX_SIP_SRC_M33_STOP,
        0x00,
        0x00,
        0x00
    ) != 0) {
        DEV_ERR("Failed to stop M-Core");
        return -1;
    }

    DEV_DBG("Stopped M-Core\n");
    return 0;
}

int start_mcore() {
    if (imx_sec_firmware_psci(
        IMX_SIP_SRC,
        IMX_SIP_SRC_M33_START,
        0x00,
        0x00,
        0x00
    ) != 0) {
        DEV_ERR("Failed to start M-Core\n");
        return -1;
    }

    DEV_DBG("Started M-Core\n");
    return 0;
}

int reset_mcore() {
    if (stop_mcore() != 0) {
        return -1;
    }
    sleep(1);

    if (start_mcore() != 0) {
        return -1;
    }
    sleep(1);

    return 0;
}
