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

static int ethosu_rpmsg_send(ethosu_rpmsg_t *rpmsg_data, uint32_t type)
{
	ethosu_fw_msg_header_t msg = {
        .magic = ETHOSU_MSG_MAGIC,
        .type = type,
        .length = 0
    };

    int status = imx_rpmsg_lite_send(rpmsg_data->handle, rpmsg_data->ept, rpmsg_data->dest_addr, (void *)&msg, sizeof(msg), RPMSG_SEND_TIMEOUT);

	return status;
}

int ethosu_rpmsg_ping(ethosu_rpmsg_t *rpmsg_data) {
    return ethosu_rpmsg_send(rpmsg_data, ETHOSU_MSG_PING);
}

int ethosu_rpmsg_pong(ethosu_rpmsg_t *rpmsg_data) {
	return ethosu_rpmsg_send(rpmsg_data, ETHOSU_MSG_PONG);
}

int ethosu_rpmsg_version_request(ethosu_rpmsg_t *rpmsg_data) {
	return ethosu_rpmsg_send(rpmsg_data, ETHOSU_MSG_VERSION_REQ);
}

int ethosu_rpmsg_capabilities_request(ethosu_rpmsg_t *rpmsg_data) {
    ethosu_fw_msg_header_t msg = {
        .magic  = ETHOSU_MSG_MAGIC,
        .type   = ETHOSU_MSG_CAPABILITY_REQ,
        .length = sizeof(ethosu_fw_capability_request_t)
    };

    ethosu_fw_capability_request_t rpmsg_request = {
        // Unused for now but can be used in the future to distinguish who sent this request
        .user_data = 0
    };

    uint8_t rpmsg_data[sizeof(ethosu_fw_msg_header_t) +
        sizeof(ethosu_fw_capability_request_t)];

    memcpy(rpmsg_data, &msg, sizeof(ethosu_fw_msg_header_t));
    memcpy(rpmsg_data + sizeof(ethosu_fw_msg_header_t),
        &rpmsg_request,
        sizeof(ethosu_fw_capability_request_t)
    );

    return imx_rpmsg_lite_send(rpmsg_data->handle, rpmsg_data->ept, rpmsg_data->dest_addr,
                                (void *)rpmsg_data, sizeof(rpmsg_data),
                                RPMSG_SEND_TIMEOUT);
}

int ethosu_rpmsg_inference(ethosu_rpmsg_t *rpmsg_data,
                        ethosu_resmgr_inference_request_t *request_data) {
    ethosu_fw_msg_header_t msg = {
        .magic = ETHOSU_MSG_MAGIC,
        .type = ETHOSU_MSG_INFERENCE_REQ,
        .length = sizeof(ethosu_fw_inference_request_t)
    };

    ethosu_fw_inference_request_t rpmsg_request = { 0 };

    rpmsg_request.user_data = rpmsg_data->global_id;

    if (request_data->ifm_count > ETHOSU_BUFFER_LIMIT || request_data->ofm_count > ETHOSU_BUFFER_LIMIT) {
        return -1;
    }

    rpmsg_request.input_count = request_data->ifm_count;
    for (int i = 0; i < ETHOSU_BUFFER_LIMIT; i++) {
        rpmsg_request.inputs[i].paddr = request_data->ifm[i].paddr;
        rpmsg_request.inputs[i].size = request_data->ifm[i].size;
    }

    rpmsg_request.output_count = request_data->ofm_count;
    rpmsg_request.outputs[0].paddr = request_data->ofm[0].paddr;
    rpmsg_request.outputs[0].size = request_data->ofm[0].size;
    for (int i = 1; i < ETHOSU_BUFFER_LIMIT; i++) {
        rpmsg_request.outputs[i].paddr = request_data->ofm[i].paddr;
        rpmsg_request.outputs[i].size = request_data->ofm[i].size;
    }

    rpmsg_request.network.buffer.paddr = request_data->network_buffer.paddr;
    rpmsg_request.network.buffer.size = request_data->network_buffer.size;
    // When using LiteRT delegate the network type is always buffer
    rpmsg_request.network.type = ETHOSU_NETWORK_BUFFER;

    for (int i = 0; i < ETHOSU_BUFFER_LIMIT; i++) {
        rpmsg_request.pmu_config[i] = request_data->pmu_config[i];
    }

    rpmsg_request.enable_cycle_counter = request_data->pmu_cycle_counter_enable;
    rpmsg_request.arena_offset = request_data->arena_offset;
    rpmsg_request.flash_offset = request_data->flash_offset;
    rpmsg_request.inference_type = request_data->inference_type;

    uint8_t rpmsg_data[sizeof(ethosu_fw_msg_header_t) +
                        sizeof(ethosu_fw_inference_request_t)];

    memcpy(rpmsg_data, &msg, sizeof(ethosu_fw_msg_header_t));
    memcpy(rpmsg_data + sizeof(ethosu_fw_msg_header_t),
            &rpmsg_request,
            sizeof(ethosu_fw_inference_request_t)
    );

    return imx_rpmsg_lite_send(rpmsg_data->handle, rpmsg_data->ept, rpmsg_data->dest_addr,
                                (void *)rpmsg_data, sizeof(rpmsg_data),
                                RPMSG_SEND_TIMEOUT);
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
    ethosu_rpmsg_t *rpmsg_data = (ethosu_rpmsg_t *)user_data;
    if (rpmsg_data != NULL) {
        if (pthread_sleepon_lock() == EOK) {
            rpmsg_data->dest_addr = new_ept;
            /* Wake up main() task */
            pthread_sleepon_signal(&rpmsg_data->dest_addr);
            pthread_sleepon_unlock();
        }
    }
}

int ethosu_rpmsg_init(ethosu_rpmsg_t *rpmsg_data) {
    rpmsg_data->incoming_data = NULL;

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

    rpmsg_data->handle = rpmsg_handle;

    rpmsg_endpoint = imx_rpmsg_lite_create_ept(rpmsg_handle, IS_MASTER, rpmsg_callback, (void *)rpmsg_data);

    rpmsg_data->ept = rpmsg_endpoint;
    if (rpmsg_endpoint == NULL) {
        DEV_ERR("Failed to create RPMSG endpoint\n");
        status = -1;
        goto fail;
    }

    rpmsg_data->namespace = imx_rpmsg_ns_bind(rpmsg_handle, rpmsg_namespace_callback, (void *)rpmsg_data);
    if (rpmsg_data->namespace == NULL) {
        DEV_ERR("imx_rpmsg_ns_bind failed\n");
        status = -1;
        goto fail;
    }

    if (pthread_sleepon_lock() == EOK) {
        while (rpmsg_data->dest_addr == 0) {
            /* Sleep and wait */
            pthread_sleepon_wait(&rpmsg_data->dest_addr);
        }
        DEV_LOG("Master got ept address from remote %lu\n", rpmsg_data->dest_addr);
        pthread_sleepon_unlock();
    }

    return status;

fail:
    /* Destroy endpoint */
    if (rpmsg_endpoint != NULL) {
        rpmsg_data->ept = NULL;
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
