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

/**
 * Sends hardware capabilities request to the Cortex-M and returns the response in @capabilities
 *
 * @param drvr_data driver data with initialized RPMSG-Lite endpoint, mutex and conditional variable
 * @param capabilites output paramater, used to store the response data
 *
 * @return 0 - success, non-zero - failure
*/
int ethosu_capabilities_request(driver_data_t *drvr_data,
                                ethosu_resmgr_device_capabilities_t *capabilities);

#endif /* ETHOSU_CAPABILITIES_H */
