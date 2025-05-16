/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_INFERENCE_H
#define ETHOSU_INFERENCE_H

/**
 * Includes
 */

#include "ethosu.h"

/**
 * Functions
 */

int ethosu_inference_invoke_request(driver_data_t *drvr_data,
                                    ethosu_resmgr_inference_request_t *req,
                                    ethosu_resmgr_inference_response_t *rsp);

#endif /* ETHOSU_INFERENCE_H */
