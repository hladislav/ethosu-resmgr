/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>

#include <fcntl.h>
#include <string.h>

#include <sys/io.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/iofunc.h>
#include <sys/resmgr.h>
#include <sys/procmgr.h>
#include <sys/dispatch.h>

#include "ethosu_firmware_api.h"

#include "ethosu.h"
#include "ethosu_misc.h"
#include "ethosu_capabilities.h"
#include "ethosu_inference.h"
#include "ethosu_utils.h"
#include "ethosu_msg.h"

#include "public/ethosu_resmgr_api.h"

/**
 * Variables
 */

// Must create this at global scope so it can be used in io_devctl callback
static driver_data_t drvr_data = { 0 };

/**
 * Functions
 */

int io_devctl(resmgr_context_t *ctp, io_devctl_t *msg, iofunc_ocb_t *ocb) {
    int status = 0;
    if ((status = iofunc_devctl_default(ctp, msg, ocb)) != _RESMGR_DEFAULT) {
        return status;
    }

    union {
        ethosu_api_device_capabilities_t capabilities;
        ethosu_resmgr_inference_msg_t inference;
    } *rx_data;

    rx_data = _IO_INPUT_PAYLOAD(msg);

    pthread_mutex_lock(&drvr_data.mutex);

    int ret = ENOSYS;
    size_t nbytes = 0;
    switch (msg->i.dcmd) {
        case ETHOSU_DEVCTL_PING:
            DEV_DBG("Sending ping...\n");
            ret = ethosu_rpmsg_ping(&drvr_data.ethosu_rpmsg);
            break;
        case ETHOSU_DEVCTL_VERSION_REQ:
            DEV_DBG("Sending version request...\n");
            ret = ethosu_version_request(&drvr_data);
            break;
        case ETHOSU_DEVCTL_CAPABILITIES_REQ:
            DEV_DBG("Sending capabilities request...\n");
            ret = ethosu_capabilities_request(&drvr_data, &rx_data->capabilities);
            if (ret == 0) {
                nbytes = sizeof(ethosu_api_device_capabilities_t);
            }
            break;

        case ETHOSU_DEVCTL_INFERENCE_INVOKE:
            DEV_DBG("Sending inference invoke request...\n");
            ret = ethosu_inference_invoke_request(&drvr_data, &rx_data->inference.tx, &rx_data->inference.rx);
            if (ret == 0) {
                nbytes = sizeof(ethosu_resmgr_inference_response_t);
            }
            break;

        case ETHOSU_DEVCTL_NETWORK_INFO:
        case ETHOSU_DEVCTL_INFERENCE_STATUS:
        case ETHOSU_DEVCTL_INFERENCE_CANCEL:
            DEV_ERR("These operations are currently not supported!\n");
            ret = ENOTSUP;
            break;
    }

    if (ret != 0) {
        DEV_ERR("Unknown message type received!\n");
    }

    DEV_DBG("devctl message processed\n");

    drvr_data.ethosu_rpmsg.incoming_data = NULL;
    // signal RPMSG handler we processed the data
    pthread_cond_signal(&drvr_data.cond);

    pthread_mutex_unlock(&drvr_data.mutex);
    memset(&msg->o, 0, sizeof(msg->o));

    msg->o.ret_val = ret;

    return _RESMGR_PTR(ctp, &msg->o, sizeof(msg->o) + nbytes);
}

int main (int argc, char **argv)
{
    if (ThreadCtl(_NTO_TCTL_IO_PRIV, 0) != 0) {
        return -1;
    }

    if (reset_mcore() != 0) {
        return EXIT_FAILURE;
    }

    if (procmgr_ability(0, PROCMGR_AOP_ALLOW | PROCMGR_ADN_NONROOT |
                        PROCMGR_AID_MEM_PHYS | PROCMGR_AID_EOL) != EOK) {
        perror("Failed to set abilities");
        return EXIT_FAILURE;
    }

    pthread_mutex_init(&drvr_data.mutex, NULL);
    pthread_cond_init(&drvr_data.cond, NULL);
    
    dispatch_t *dpp;
    if ((dpp = dispatch_create_channel(-1, DISPATCH_FLAG_NOLOCK)) == NULL) {
        DEV_ERR("%s:  Unable to allocate dispatch context.\n", argv [0]);
        return EXIT_FAILURE;
    }

    resmgr_connect_funcs_t connect_func = { 0 };
    resmgr_io_funcs_t io_func = { 0 };
    iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &connect_func,
        _RESMGR_IO_NFUNCS, &io_func);
        
        io_func.devctl = io_devctl;
        
    resmgr_attr_t resmgr_attr = { 0 };
    resmgr_attr.nparts_max = 1;
    resmgr_attr.msg_max_size = 2048;

    iofunc_attr_t attr = { 0 };
    iofunc_attr_init(&attr, S_IFNAM | 0666, 0, 0);

    if (resmgr_attach(dpp, &resmgr_attr,
                    ETHOSU_DEVICE_PATH, _FTYPE_ANY,
                    0, &connect_func, &io_func, &attr) == -1) {
        perror("Unable to attach");
        goto fail;
    }

    ethosu_rpmsg_init(&drvr_data.ethosu_rpmsg);

    dispatch_context_t *ctp = dispatch_context_alloc(dpp);

    while(1) {
        if ((ctp = dispatch_block(ctp)) == NULL) {
            DEV_ERR("Unable to block");
            return EXIT_FAILURE;
        }

        dispatch_handler(ctp);
    }

    return (EXIT_SUCCESS);

fail:
    // not sure about this destroy
    dispatch_destroy(dpp);
    return EXIT_FAILURE;
}
