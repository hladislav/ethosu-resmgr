/**
    Copyright 2025 Ladislav Hano

    SPDX-License-Identifier: Apache-2.0
*/


#ifndef ETHOSU_FW_API_H
#define ETHOSU_FW_API_H

/**
 * Includes
 */

#include <stdint.h>

/**
 * Ethos-U firmware interface defines
 */

#define ETHOSU_BUFFER_LIMIT 4
#define ETHOSU_PMU_LIMIT 4

#define ETHOSU_MSG_MAGIC 0x41457631
#define ETHOSU_MSG_VERSION_MAJOR 0
#define ETHOSU_MSG_VERSION_MINOR 2
#define ETHOSU_MSG_VERSION_PATCH 0

#define ETHOSU_INFERENCE_MODEL 0
#define ETHOSU_INFERENCE_OP 1

/**
 * Ethos-U firmware interface types
 */

enum ethosu_rpmsg_msg_type {
    ETHOSU_MSG_ERROR = 1,
    ETHOSU_MSG_PING,
    ETHOSU_MSG_PONG,
    ETHOSU_MSG_INFERENCE_REQ,
    ETHOSU_MSG_INFERENCE_RSP,
    ETHOSU_MSG_VERSION_REQ,
    ETHOSU_MSG_VERSION_RSP,
    ETHOSU_MSG_CAPABILITY_REQ,
    ETHOSU_MSG_CAPABILITY_RSP,
    ETHOSU_MSG_NETWORK_INFO_REQ,
    ETHOSU_MSG_NETWORK_INFO_RSP,
    ETHOSU_MSG_CANCEL_INFERENCE_REQ,
    ETHOSU_MSG_CANCEL_INFERENCE_RSP,
    ETHOSU_MSG_POWER_REQ,
    ETHOSU_MSG_POWER_RSP,
    ETHOSU_MSG_TYPE_MAX
};

typedef struct ethosu_fw_msg_header {
    uint32_t magic;
    uint32_t type;
    uint32_t length;
} ethosu_fw_msg_header_t;

enum ethosu_fw_error_type {
    ETHOSU_MSG_ERR_GENERIC = 0,
    ETHOSU_MSG_ERR_UNSUPPORTED_TYPE,
    ETHOSU_MSG_ERR_INVALID_PAYLOAD,
    ETHOSU_MSG_ERR_INVALID_SIZE,
    ETHOSU_MSG_ERR_INVALID_MAGIC,
    ETHOSU_MSG_ERR_MAX
};

typedef struct ethosu_fw_error_msg {
    enum ethosu_fw_error_type   type;
    char                        msg[128];
} ethosu_fw_error_msg_t;

typedef struct ethosu_fw_buffer {
    uint32_t paddr;
    uint32_t size;
} ethosu_fw_buffer_t;

enum ethosu_fw_network_type {
    ETHOSU_NETWORK_BUFFER = 1,
    ETHOSU_NETWORK_INDEX
};

typedef struct ethosu_fw_network {
    uint32_t type;
    union {
        ethosu_fw_buffer_t  buffer;
        uint32_t            index;
    };
} ethosu_fw_network_t;

typedef struct ethosu_fw_inference_request {
    uint64_t            user_data;

    uint32_t            input_count;
    ethosu_fw_buffer_t  inputs[ETHOSU_BUFFER_LIMIT];

    uint32_t            output_count;
    ethosu_fw_buffer_t  outputs[ETHOSU_BUFFER_LIMIT];

    ethosu_fw_network_t network;

    uint8_t             pmu_config[ETHOSU_PMU_LIMIT];
    uint32_t            enable_cycle_counter;

    uint32_t            arena_offset;
    uint32_t            flash_offset;
    uint32_t            inference_type;
} ethosu_fw_inference_request_t;

typedef struct ethosu_fw_inference_response {
    uint64_t user_data;

    uint32_t output_count;
    uint32_t output[ETHOSU_BUFFER_LIMIT];

    uint32_t status;

    uint8_t  pmu_config[ETHOSU_PMU_LIMIT];
    uint32_t pmu_counts[ETHOSU_PMU_LIMIT];
    uint32_t enable_cycle_counter;
    uint64_t cycle_counter_value;
} ethosu_fw_inference_response_t;

typedef struct ethosu_fw_version_info {
    uint8_t major;
    uint8_t minor;
    uint8_t patch;
    uint8_t _reserved;
} ethosu_fw_version_info_t;

typedef struct ethosu_fw_capability_request {
    uint64_t user_data;
} ethosu_fw_capability_request_t;

typedef struct ethosu_fw_capability_response {
    uint64_t user_data;

    uint32_t version_status;
    uint32_t version_minor;
    uint32_t version_major;

    uint32_t product_major;

    uint32_t arch_patch;
    uint32_t arch_minor;
    uint32_t arch_major;

    uint32_t driver_patch;
    uint32_t driver_minor;
    uint32_t driver_major;

    uint32_t macs_per_cycle;
    uint32_t command_stream_version;
    uint32_t supports_dma;
} ethosu_fw_capability_response_t;

#endif /* ETHOSU_FW_API_H */