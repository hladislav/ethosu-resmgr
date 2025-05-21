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

/**
 * Sends inference request to Cortex-M and returns the response in @response
 *
 * @param drvr_data driver data with initialized RPMSG-Lite endpoint, mutex and conditional variable
 * @param request data required for inference
 * @param response output parameter, stores the inference response
 *
 * @return 0 - success, non-zero - failure
*/
int ethosu_inference_invoke_request(driver_data_t *drvr_data,
                                    ethosu_resmgr_inference_request_t *request,
                                    ethosu_resmgr_inference_response_t *response);

#endif /* ETHOSU_INFERENCE_H */
