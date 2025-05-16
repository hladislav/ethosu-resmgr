/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <stdlib.h>
#include <stdarg.h>

#include <errno.h>
#include <string.h>

#include <sys/mman.h>

#include "ethosu_firmware_api.h"
#include "ethosu_inference.h"
#include "public/ethosu_resmgr_api.h"
#include "ethosu.h"


int ethosu_inference_invoke_request(driver_data_t *drvr_data,
                            ethosu_resmgr_inference_request_t *req,
                            ethosu_resmgr_inference_response_t *rsp) {
    DEV_DBG("Sending RPMSG inference request\n");
    int ret = ethosu_rpmsg_inference(&drvr_data->ethosu_rpmsg, req);
    if (ret != 0) {
        DEV_ERR("Failed to send RPMSG inference request\n");
        return ret;
    }

    DEV_DBG("Waiting for data from RPMSG handler\n");
    while (drvr_data->ethosu_rpmsg.incoming_data == NULL) {
        pthread_cond_wait(&drvr_data->cond, &drvr_data->mutex);
    }

    memset(rsp, 0, sizeof(ethosu_resmgr_inference_response_t));
    ethosu_fw_inference_response_t* response = (ethosu_fw_inference_response_t*)drvr_data->ethosu_rpmsg.incoming_data;

    rsp->ofm_count = response->output_count;
    for (int i = 0; i < ETHOSU_BUFFER_LIMIT; i++) {
        rsp->ofm_size[i] = response->output[i];
    }

    rsp->status = response->status;

    for (int i = 0; i < ETHOSU_PMU_EVENT_MAX; ++i) {
        rsp->pmu_event_config[i] = response->pmu_config[i];
        rsp->pmu_event_count[i] = response->pmu_counts[i];
    }

    rsp->pmu_cycle_counter_enable = response->enable_cycle_counter;
    rsp->pmu_cycle_counter_count = response->cycle_counter_value;

    DEV_DBG("Data copied to response message\n");
    return 0;
}

// ? currently unused
int ethosu_inference_handler(driver_data_t *drvr_data, ssize32_t dcmd, ethosu_resmgr_inference_msg_t *data) {

    switch (dcmd) {
        case ETHOSU_DEVCTL_INFERENCE_INVOKE:
            return ethosu_inference_invoke_request(drvr_data, &data->tx, &data->rx);
        // not used since we return all info with inference invoke
        case ETHOSU_DEVCTL_INFERENCE_STATUS:
        // not used by the delegate
        case ETHOSU_DEVCTL_INFERENCE_CANCEL:
            return ENOTSUP;
    }

    return ENOSYS;
}
