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
    void                    *handle;        // RPMSG-lite handle
    void                    *ept;           // RPMSG-lite endpoint
    void*                    namespace;     // RPMSG-Lite namespace
    volatile unsigned long   dest_addr;     // Used for communication over RPMSG-Lite

    void*                    incoming_data; // Used to temporary store data from Cortex-M

    // can be used in the future to distinguish messages from multiple threads
    int                      global_id;
} ethosu_rpmsg_t;

/**
 * Functions
 */

/**
 * Sends PING to the Cortex-M via RPMSG-Lite
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 *
 * @return 0 - success, non-zero - failed to send
*/
int ethosu_rpmsg_ping(ethosu_rpmsg_t *rpmsg_data);

/**
 * Sends PONG to the Cortex-M via RPMSG-Lite
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 *
 * @return 0 - success, non-zero - failed to send
*/
int ethosu_rpmsg_pong(ethosu_rpmsg_t *rpmsg_data);

/**
 * Sends version request to the Cortex-M via RPMSG-Lite
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 *
 * @return 0 - success, non-zero - failed to send
*/
int ethosu_rpmsg_version_request(ethosu_rpmsg_t *rpmsg_data);

/**
 * Sends capabilities request to the Cortex-M via RPMSG-Lite
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 *
 * @return 0 - success, non-zero - failed to send
*/
int ethosu_rpmsg_capabilities_request(ethosu_rpmsg_t *rpmsg_data);

/**
 * Sends inference request to the Cortex-M via RPMSG-Lite
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 * @param request_data inference data for Cortex-M
 *
 * @return 0 - success, non-zero - failed to send
*/
int ethosu_rpmsg_inference(ethosu_rpmsg_t *rpmsg_data,
                            ethosu_resmgr_inference_request_t *request_data);

/**
 * Initializes the communication via RPMSG-Lite
 *
 * @param rpmsg_data uninitialized RPMSG-Lite structure
 *
 * @return 0 - success, non-zero - failure
*/
int ethosu_rpmsg_init(ethosu_rpmsg_t *rpmsg_data);

#endif /* ETHOSU_RPMSG_H */
