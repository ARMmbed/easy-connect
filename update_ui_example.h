//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef UPDATE_UI_EXAMPLE_H
#define UPDATE_UI_EXAMPLE_H

#include "mbed-cloud-client/MbedCloudClient.h"

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE

#ifdef ARM_UPDATE_CLIENT_VERSION_VALUE
#if ARM_UPDATE_CLIENT_VERSION_VALUE > 101000
/**
 * @brief Function for authorizing firmware downloads and reboots.
 * @param request The request under consideration.
 */
void update_authorize(int32_t request);
#endif
#endif

/**
 * @brief Callback function for reporting the firmware download progress.
 * @param progress Received bytes.
 * @param total Total amount of bytes to be received.
 */
void update_progress(uint32_t progress, uint32_t total);

/**
 * @brief Set the cloud client instance for the update UI to use
 * @param[in] client pointer to the cloud client instance
 */
void update_ui_set_cloud_client(MbedCloudClient* client);


#endif // MBED_CLOUD_CLIENT_SUPPORT_UPDATE

#endif // UPDATE_UI_EXAMPLE_H
