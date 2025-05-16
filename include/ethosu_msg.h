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

int ethosu_msg_handler(ethosu_rpmsg_t *erp, void *data);

#endif /* ETHOSU_MSG_H */