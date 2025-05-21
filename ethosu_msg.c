/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <stdlib.h>

#include "ethosu.h"
#include "ethosu_firmware_api.h"
#include "ethosu_rpmsg.h"
#include "ethosu_inference.h"
#include "ethosu_msg.h"

int ethosu_msg_handler(ethosu_rpmsg_t *rpmsg_data, void *data)
{
    ethosu_fw_msg_header_t* header = (ethosu_fw_msg_header_t*)data;
    void *msg_data = (char *)data + sizeof(ethosu_fw_msg_header_t);

    switch (header->type) {
        case ETHOSU_MSG_ERROR:
            DEV_DBG("Received msg error from core\n");
            if (header->length != sizeof(ethosu_fw_error_msg_t)) {
                DEV_ERR("Message from core was not the correct size!\n");
                return EBADMSG;
            }

            ethosu_fw_error_msg_t* error = (ethosu_fw_error_msg_t*)msg_data;
            
            error->msg[sizeof(error->msg) - 1] = '\0';
            DEV_ERR("Error message: %s\n", error->msg);
            return EBADMSG;
        case ETHOSU_MSG_PING:
            DEV_DBG("Received ping from core\n");
            return ethosu_rpmsg_pong(rpmsg_data);
        case ETHOSU_MSG_PONG:
            DEV_DBG("Received PONG\n");
            return EOK;
        case ETHOSU_MSG_INFERENCE_RSP:
            if (header->length != sizeof(ethosu_fw_inference_response_t)) {
                DEV_ERR("Message from core was not the correct size!\n");
                return EBADMSG;
            }

            ethosu_fw_inference_response_t *inference_rsp = (ethosu_fw_inference_response_t *)msg_data;

            DEV_DBG("Msg: Inference response: output_count=%u, status=%u\n",
                inference_rsp->output_count, inference_rsp->status
            );

            rpmsg_data->incoming_data = inference_rsp;

            return EOK;
        case ETHOSU_MSG_VERSION_RSP:
            DEV_DBG("Received version response from core\n");
            if (header->length != sizeof(ethosu_fw_version_info_t)) {
                DEV_ERR("Version message was not the correct size!\n");
                return EBADMSG;
            }

            ethosu_fw_version_info_t* version = (ethosu_fw_version_info_t*)msg_data;

            DEV_DBG("Received version message: %d.%d\n", version->major, version->minor);

            rpmsg_data->incoming_data = version;

            return EOK;
        case ETHOSU_MSG_CAPABILITY_RSP:
            DEV_DBG("Received capabilities from core\n");
            if (header->length != sizeof(ethosu_fw_capability_response_t)) {
                DEV_ERR("Message from core was not the correct size!\n");
                return EBADMSG;
            }

            ethosu_fw_capability_response_t* capabilities = (ethosu_fw_capability_response_t*)msg_data;

            DEV_DBG("Msg: Capabilities response ua%lx vs%hhu v%hhu.%hhu p%hhu av%hhu.%hhu.%hhu dv%hhu.%hhu.%hhu mcc%hhu csv%hhu cd%hhu\n",
                    capabilities->user_data,
                    capabilities->version_status,
                    capabilities->version_major,
                    capabilities->version_minor,
                    capabilities->product_major,
                    capabilities->arch_major,
                    capabilities->arch_minor,
                    capabilities->arch_patch,
                    capabilities->driver_major,
                    capabilities->driver_minor,
                    capabilities->driver_patch,
                    capabilities->macs_per_cycle,
                    capabilities->command_stream_version,
                    capabilities->supports_dma
            );

            rpmsg_data->incoming_data = capabilities;
            return 0;
        case ETHOSU_MSG_NETWORK_INFO_RSP:
        case ETHOSU_MSG_POWER_RSP:
        case ETHOSU_MSG_CANCEL_INFERENCE_RSP:
            DEV_ERR("Received unsupported message type\n");
            return ENOTSUP;
    }

    DEV_ERR("Received unknown message type\n");
    return ENOSYS;
}
