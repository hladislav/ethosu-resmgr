/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_RPMSG_H
#define ETHOSU_RPMSG_H

/**
 * Includes
 */

#include <sys/types.h>

#include "public/ethosu_resmgr_api.h"

/**
 * Defines
 */

#define IMX_93_MU1A_BASE                       0x44220000
#define IMX_93_MU1B_BASE                       0x44230000

#define IMX_93_MU1A_IRQ                       (21 + 32)
#define IMX_93_MU1B_IRQ                       (22 + 32)

#define RPMSG_SEND_TIMEOUT 100

/**
 * Types
 */

typedef struct ethosu_rpmsg {
    void                    *handle; // RPMSG-lite handle
    void                    *ept; // RPMSG-lite endpoint
    volatile unsigned long   dest_addr;
    void*                    namespace;

    void*                    incoming_data;

    // can be used in the future to distinguish messages from multiple threads
    int                      global_id;
} ethosu_rpmsg_t;

/**
 * Functions
 */

int ethosu_rpmsg_ping(ethosu_rpmsg_t *erp);

int ethosu_rpmsg_pong(ethosu_rpmsg_t *erp);

int ethosu_rpmsg_version_request(ethosu_rpmsg_t *erp);

int ethosu_rpmsg_capabilities_request(ethosu_rpmsg_t *erp, int id);

int ethosu_rpmsg_inference(ethosu_rpmsg_t *erp,
                            ethosu_resmgr_inference_request_t *req);

int ethosu_rpmsg_cancel_inference(ethosu_rpmsg_t *erp,
                                int inference_handle);

int ethosu_rpmsg_init(ethosu_rpmsg_t *erp);

#endif /* ETHOSU_RPMSG_H */
