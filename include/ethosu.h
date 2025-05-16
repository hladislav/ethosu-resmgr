/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_DEVICE_H
#define ETHOSU_DEVICE_H

/**
 * Includes
 */

#include <pthread.h>
#include <stdio.h>

#include "ethosu_rpmsg.h"

/**
 * Defines
 */

#define DEV_ERR(...)    printf(__VA_ARGS__)
#define DEV_LOG(...)    printf(__VA_ARGS__)

#ifdef DEBUG_BUILD
#define DEV_DBG(...)    printf(__VA_ARGS__)
#else
#define DEV_DBG(...)
#endif

#define ETHOSU_DEVICE_PATH "/dev/ethosu0"

/**
 * Types
 */

typedef struct driver_data {
    ethosu_rpmsg_t        ethosu_rpmsg;
    void *                namespace;
    int                   msg_result;
    
    pthread_mutex_t       mutex;
    pthread_cond_t        cond;
} driver_data_t;

#endif