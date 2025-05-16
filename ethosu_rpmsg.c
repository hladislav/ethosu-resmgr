/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/imx_rpmsg_lite.h>

#include "ethosu.h"
#include "ethosu_firmware_api.h"
#include "ethosu_rpmsg.h"
#include "ethosu_msg.h"
#include "ethosu_inference.h"

#define IS_MASTER 1

/**
 * Functions
 */

static int ethosu_rpmsg_send(ethosu_rpmsg_t *erp, uint32_t type)
{
	ethosu_fw_msg_header_t msg = {
        .magic = ETHOSU_MSG_MAGIC,
        .type = type,
        .length = 0
    };

    int status = imx_rpmsg_lite_send(erp->handle, erp->ept, erp->dest_addr, (void *)&msg, sizeof(msg), RPMSG_SEND_TIMEOUT);

	return status;
}

int ethosu_rpmsg_ping(ethosu_rpmsg_t *erp) {
    return ethosu_rpmsg_send(erp, ETHOSU_MSG_PING);
}

int ethosu_rpmsg_pong(ethosu_rpmsg_t *erp) {
	return ethosu_rpmsg_send(erp, ETHOSU_MSG_PONG);
}

int ethosu_rpmsg_version_request(ethosu_rpmsg_t *erp) {
	return ethosu_rpmsg_send(erp, ETHOSU_MSG_VERSION_REQ);
}

int ethosu_rpmsg_capabilities_request(ethosu_rpmsg_t *erp, int id) {
    ethosu_fw_msg_header_t msg = {
        .magic  = ETHOSU_MSG_MAGIC,
        .type   = ETHOSU_MSG_CAPABILITY_REQ,
        .length = sizeof(ethosu_fw_capability_request_t)
    };

    ethosu_fw_capability_request_t req = {
        .user_data = id
    };

    uint8_t rpmsg_data[sizeof(ethosu_fw_msg_header_t) +
        sizeof(ethosu_fw_capability_request_t)];

    memcpy(rpmsg_data, &msg, sizeof(ethosu_fw_msg_header_t));
    memcpy(rpmsg_data + sizeof(ethosu_fw_msg_header_t),
        &req,
        sizeof(ethosu_fw_capability_request_t)
    );

    DEV_DBG("Sending data to core\n");
    return imx_rpmsg_lite_send(erp->handle, erp->ept, erp->dest_addr,
                                (void *)rpmsg_data, sizeof(rpmsg_data),
                                RPMSG_SEND_TIMEOUT);
}

int ethosu_rpmsg_inference(ethosu_rpmsg_t *erp,
                        ethosu_resmgr_inference_request_t *req) {
    ethosu_fw_msg_header_t msg = {
        .magic = ETHOSU_MSG_MAGIC,
        .type = ETHOSU_MSG_INFERENCE_REQ,
        .length = sizeof(ethosu_fw_inference_request_t)
    };

    ethosu_fw_inference_request_t request = { 0 };

    request.user_data = erp->global_id;

    if (req->ifm_count > ETHOSU_BUFFER_LIMIT || req->ofm_count > ETHOSU_BUFFER_LIMIT) {
        return -1;
    }

    request.input_count = req->ifm_count;
    for (int i = 0; i < ETHOSU_BUFFER_LIMIT; i++) {
        request.inputs[i].paddr = req->ifm[i].paddr;
        request.inputs[i].size = req->ifm[i].size;
    }

    request.output_count = req->ofm_count;
    request.outputs[0].paddr = req->ofm[0].paddr;
    request.outputs[0].size = req->ofm[0].size;
    for (int i = 1; i < ETHOSU_BUFFER_LIMIT; i++) {
        request.outputs[i].paddr = req->ofm[i].paddr;
        request.outputs[i].size = req->ofm[i].size;
    }

    request.network.buffer.paddr = req->network_buffer.paddr;
    request.network.buffer.size = req->network_buffer.size;
    // When using LiteRT delegate the network type is always buffer
    request.network.type = ETHOSU_NETWORK_BUFFER;

    for (int i = 0; i < ETHOSU_BUFFER_LIMIT; i++) {
        request.pmu_config[i] = req->pmu_config[i];
    }

    request.enable_cycle_counter = req->pmu_cycle_counter_enable;
    request.arena_offset = req->arena_offset;
    request.flash_offset = req->flash_offset;
    request.inference_type = req->inference_type;

    uint8_t rpmsg_data[sizeof(ethosu_fw_msg_header_t) +
                        sizeof(ethosu_fw_inference_request_t)];

    memcpy(rpmsg_data, &msg, sizeof(ethosu_fw_msg_header_t));
    memcpy(rpmsg_data + sizeof(ethosu_fw_msg_header_t),
            &request,
        sizeof(ethosu_fw_inference_request_t));

    return imx_rpmsg_lite_send(erp->handle, erp->ept, erp->dest_addr,
                                (void *)rpmsg_data, sizeof(rpmsg_data),
                                RPMSG_SEND_TIMEOUT);
}

int ethosu_rpmsg_cancel_inference(ethosu_rpmsg_t *erp,
                                int inference_handle) {

    return ENOSYS;
}

int32_t rpmsg_callback(void *payload, uint32_t payload_len, uint32_t src, void *priv) {
    driver_data_t *drvr_data = (driver_data_t *)priv;

    DEV_DBG("RPMSG sent back data\n");
    pthread_mutex_lock(&drvr_data->mutex);

    int ret = -1;
    DEV_DBG("Locked mutex and going to handle message\n");
    ret = ethosu_msg_handler(&drvr_data->ethosu_rpmsg, payload);

    drvr_data->msg_result = ret;

    // Signal devctl() handler that data are ready to be read
    pthread_cond_signal(&drvr_data->cond);
    while (drvr_data->ethosu_rpmsg.incoming_data != NULL) {
        // Wait for signal from devctl() handler that the data was copied
        pthread_cond_wait(&drvr_data->cond, &drvr_data->mutex);
    }

    pthread_mutex_unlock(&drvr_data->mutex);

    DEV_DBG("Message handled, mutex unlocked, signal sent\n");

    return ret;
}

void rpmsg_namespace_callback(unsigned int new_ept, const char *new_ept_name, uint32_t flags, void *user_data) {
    ethosu_rpmsg_t *erp = (ethosu_rpmsg_t *)user_data;
    if (erp != NULL) {
        if (pthread_sleepon_lock() == EOK) {
            erp->dest_addr = new_ept;
            /* Wake up main() task */
            pthread_sleepon_signal(&erp->dest_addr);
            pthread_sleepon_unlock();
        }
    }
}

int ethosu_rpmsg_init(ethosu_rpmsg_t *erp) {
    erp->incoming_data = NULL;

    imx_platform_cfg_t platform = {
        .mu_base = IMX_93_MU1B_BASE,
        .mu_irq = IMX_93_MU1B_IRQ
    };

    imx_rpmsg_env_cfg_t env = { 
        .platform_cfg = &platform
    };

    imx_rpmsg_init_t init = {
        .mem_addr = 0xa4010000,
        .mem_length = 512 * 1024,
        .link_id = 1,
        .init_flags = 0
    };

    void *rpmsg_endpoint = NULL;
    void *rpmsg_handle = NULL;
    int status = imx_rpmsg_lite_master_init(&rpmsg_handle, &init, &env);
    if (status != 0) {
        DEV_ERR("rpmsg_lite initialization failed %i\n", status);
        goto fail;
    }

    erp->handle = rpmsg_handle;

    rpmsg_endpoint = imx_rpmsg_lite_create_ept(rpmsg_handle, IS_MASTER, rpmsg_callback, (void *)erp);

    erp->ept = rpmsg_endpoint;
    if (rpmsg_endpoint == NULL) {
        DEV_ERR("Failed to create RPMSG endpoint\n");
        status = -1;
        goto fail;
    }

    erp->namespace = imx_rpmsg_ns_bind(rpmsg_handle, rpmsg_namespace_callback, (void *)erp);
    if (erp->namespace == NULL) {
        DEV_ERR("imx_rpmsg_ns_bind failed\n");
        status = -1;
        goto fail;
    }

    if (pthread_sleepon_lock() == EOK) {
        while (erp->dest_addr == 0) {
            /* Sleep and wait */
            pthread_sleepon_wait(&erp->dest_addr);
        }
        DEV_LOG("Master got ept address from remote %lu\n", erp->dest_addr);
        pthread_sleepon_unlock();
    }

    return status;

fail:
    /* Destroy endpoint */
    if (rpmsg_endpoint != NULL) {
        erp->ept = NULL;
        status = imx_rpmsg_lite_destroy_ept(rpmsg_handle, rpmsg_endpoint);
        if (status != 0) {
            DEV_ERR("imx_rpmsg_lite_destroy_ept status %i\n", status);
        }
    }

    /* Destroy rpmsg_lite instance */
    if (rpmsg_handle != NULL) {
        status = imx_rpmsg_lite_deinit(rpmsg_handle);
        if (status != 0) {
            DEV_ERR("imx_rpmsg_lite_deinit status %i\n", status);
        }
    }

    return status;
}
