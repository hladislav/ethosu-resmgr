/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_CAPABILITIES_H
#define ETHOSU_CAPABILITIES_H

/**
 * Includes
 */

#include "ethosu.h"
#include "public/ethosu_resmgr_api.h"

#include <sys/iofunc.h>

/**
 * Functions
 */

int ethosu_capabilities_request(driver_data_t *drvr_data, ethosu_api_device_capabilities_t *cap);

#endif /* ETHOSU_CAPABILITIES_H */
