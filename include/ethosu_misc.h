/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/

#ifndef ETHOSU_MISC_H
#define ETHOSU_MISC_H

/**
 * Includes
 */

#include "ethosu.h"

/**
 * Functions
 */

/**
 * Sends version request to Cortex-M, returns success if it has the expected version
 *
 * @param drvr_data driver data with initialized RPMSG-Lite endpoint, mutex and conditional variable
 *
 * @return 0 - success, non-zero - failure
*/
int ethosu_version_request(driver_data_t *drvr_data);

#endif /* ETHOSU_MISC_H */
