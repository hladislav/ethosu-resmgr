/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include "ethosu_misc.h"
#include "ethosu_rpmsg.h"
#include "ethosu.h"

#include "ethosu_firmware_api.h"

#include "public/ethosu_resmgr_api.h"

int ethosu_version_request(driver_data_t *drvr_data) {
    DEV_DBG("Device version request\n");
    int ret = ethosu_rpmsg_version_request(&drvr_data->ethosu_rpmsg);
    if (ret != 0) {
        DEV_ERR("Failed to send RPMSG inference request\n");
        return -1;
    }

    DEV_DBG("Waiting for data from RPMSG handler\n");
    while (drvr_data->ethosu_rpmsg.incoming_data == NULL) {
        pthread_cond_wait(&drvr_data->cond, &drvr_data->mutex);
    }

    ethosu_fw_version_info_t* version = (ethosu_fw_version_info_t*)drvr_data->ethosu_rpmsg.incoming_data;

    if (version->major != ETHOSU_MSG_VERSION_MAJOR ||
        version->minor != ETHOSU_MSG_VERSION_MINOR) {
        DEV_ERR("Wrong version, expected: %d.%d got: %d.%d\n", 
            ETHOSU_MSG_VERSION_MAJOR,
            ETHOSU_MSG_VERSION_MINOR,
            version->major,
            version->minor
        );

        return -1;
    }

    return 0;
}
