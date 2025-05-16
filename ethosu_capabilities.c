/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>

#include <errno.h>
#include <unistd.h>

#include "ethosu_capabilities.h"
#include "ethosu_firmware_api.h"
#include "public/ethosu_resmgr_api.h"

int ethosu_capabilities_request(driver_data_t *drvr_data,
    ethosu_api_device_capabilities_t *cap) {

    DEV_DBG("Sending capabilities request to core\n");
    int ret = ethosu_rpmsg_capabilities_request(&drvr_data->ethosu_rpmsg, drvr_data->ethosu_rpmsg.global_id);
    if (ret != 0) {
        return ret;
    }

    DEV_DBG("Waiting for data from RPMSG handler\n");
    while (drvr_data->ethosu_rpmsg.incoming_data == NULL) {
        pthread_cond_wait(&drvr_data->cond, &drvr_data->mutex);
    }

    ethosu_fw_capability_response_t* response = (ethosu_fw_capability_response_t*)drvr_data->ethosu_rpmsg.incoming_data;

    cap->hw_id.version_status = response->version_status;
	cap->hw_id.version_minor = response->version_minor;
	cap->hw_id.version_major = response->version_major;
	cap->hw_id.product_major = response->product_major;
	cap->hw_id.arch_patch_rev = response->arch_patch;
	cap->hw_id.arch_minor_rev = response->arch_minor;
	cap->hw_id.arch_major_rev = response->arch_major;
	cap->driver_patch_rev = response->driver_patch;
	cap->driver_minor_rev = response->driver_minor;
	cap->driver_major_rev = response->driver_major;
	cap->hw_cfg.macs_per_cc = response->macs_per_cycle;
	cap->hw_cfg.cmd_stream_version = response->command_stream_version;
	cap->hw_cfg.custom_dma = response->supports_dma;

    return 0;
}
