/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_MSG_H
#define ETHOSU_MSG_H

/**
 * Includes
 */

#include "ethosu_rpmsg.h"

/**
 * Functions
 */

/**
 * Handler for the received RPMSG-Lite messages
 *
 * @param rpmsg_data initialized RPMSG-Lite handle and endpoint
 * @param request data required for inference
 * @param response output parameter, stores the inference response
 *
 * @return 0 - success, non-zero - failure
*/
int ethosu_msg_handler(ethosu_rpmsg_t *rpmsg_data, void *data);

#endif /* ETHOSU_MSG_H */