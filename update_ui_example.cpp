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

#include "update_ui_example.h"

#ifdef MBED_CLOUD_CLIENT_SUPPORT_UPDATE

#include <stdio.h>
#include <stdint.h>

#ifdef MBED_APPLICATION_SHIELD
#include "C12832.h"
extern C12832* lcd;
#endif

static MbedCloudClient* _client;

#ifdef ARM_UPDATE_CLIENT_VERSION_VALUE
#if ARM_UPDATE_CLIENT_VERSION_VALUE > 101000
void update_ui_set_cloud_client(MbedCloudClient* client)
{
    _client = client;
}

void update_authorize(int32_t request)
{
    switch (request)
    {
        /* Cloud Client wishes to download new firmware. This can have a negative
           impact on the performance of the rest of the system.

           The user application is supposed to pause performance sensitive tasks
           before authorizing the download.

           Note: the authorization call can be postponed and called later.
           This doesn't affect the performance of the Cloud Client.
        */
        case MbedCloudClient::UpdateRequestDownload:
            printf("Firmware download requested\r\n");
            printf("Authorization granted\r\n");
            _client->update_authorize(MbedCloudClient::UpdateRequestDownload);

#ifdef MBED_APPLICATION_SHIELD
            /* clear screen */
            lcd->cls();
#endif
            break;

        /* Cloud Client wishes to reboot and apply the new firmware.

           The user application is supposed to save all current work before rebooting.

           Note: the authorization call can be postponed and called later.
           This doesn't affect the performance of the Cloud Client.
        */
        case MbedCloudClient::UpdateRequestInstall:
            printf("Firmware install requested\r\n");
            printf("Authorization granted\r\n");
            _client->update_authorize(MbedCloudClient::UpdateRequestInstall);
            break;

        default:
            printf("Error - unknown request\r\n");
            break;
    }
}
#endif
#endif

void update_progress(uint32_t progress, uint32_t total)
{
    uint8_t percent = progress * 100 / total;

#ifdef MBED_APPLICATION_SHIELD
    /* display progress */
    uint8_t bar = progress * 90 / total;

    lcd->locate(0,3);
    lcd->printf("Downloading: %d / %d KiB", progress / 1024, total / 1024);

    lcd->rect(0, 15, 90, 22, 1);
    lcd->fillrect(0, 15, bar, 22, 1);

    lcd->locate(91, 15);
    lcd->printf(" %d %%", percent);
#endif

/* only show progress bar if debug trace is disabled */
#if !defined(MBED_CONF_MBED_TRACE_ENABLE) \
    && !ARM_UC_ALL_TRACE_ENABLE \
    && !ARM_UC_HUB_TRACE_ENABLE

    printf("\rDownloading: [");
    for (uint8_t index = 0; index < 50; index++)
    {
        if (index < percent / 2)
        {
            printf("+");
        }
        else if (index == percent / 2)
        {
            static uint8_t old_max = 0;
            static uint8_t counter = 0;

            if (index == old_max)
            {
                counter++;
            }
            else
            {
                old_max = index;
                counter = 0;
            }

            switch (counter % 4)
            {
                case 0:
                    printf("/");
                    break;
                case 1:
                    printf("-");
                    break;
                case 2:
                    printf("\\");
                    break;
                case 3:
                default:
                    printf("|");
                    break;
            }
        }
        else
        {
            printf(" ");
        }
    }
    printf("] %d %%", percent);
    fflush(stdout);
#else
    printf("Downloading: %d %%\r\n", percent);
#endif

    if (progress == total)
    {
        printf("\r\nDownload completed\r\n");
    }
}

#endif // MBED_CLOUD_CLIENT_SUPPORT_UPDATE
