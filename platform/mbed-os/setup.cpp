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

#include "mbed.h"
#include "setup.h"
#include "memory_tests.h"
#include "simplem2mclient.h"
#include "SDBlockDevice.h"
#include "FATFileSystem.h"

#define MBED_CONF_APP_ESP8266_TX MBED_CONF_APP_WIFI_TX
#define MBED_CONF_APP_ESP8266_RX MBED_CONF_APP_WIFI_RX

#include "easy-connect/easy-connect.h"
#include "mbed_trace.h"
#define TRACE_GROUP "exam"

// Some boards specific sanity checks, better stop early.
#if defined(TARGET_UBLOX_EVK_ODIN_W2) && defined(DEVICE_EMAC) && defined(MBED_CONF_APP_NETWORK_INTERFACE) && defined (ETHERNET) && (MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET)
    #error "UBLOX_EVK_ODIN_W2 - does not work with Ethernet if you have EMAC on! Please fix your mbed_app.json."
#endif
#if defined(TARGET_UBLOX_EVK_ODIN_W2) && !defined(DEVICE_EMAC) && defined(MBED_CONF_APP_NETWORK_INTERFACE) && defined (WIFI_ODIN) && (MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ODIN)
    #error "UBLOX_EVK_ODIN_W2 - does not work with WIFI_ODIN if you have disabled EMAC! Please fix your mbed_app.json."
#endif


#ifdef MBED_APPLICATION_SHIELD
#include "C12832.h"
C12832* lcd;
#endif

void init_screen();

extern SDBlockDevice sd(MBED_CONF_SD_SPI_MOSI, MBED_CONF_SD_SPI_MISO, MBED_CONF_SD_SPI_CLK, MBED_CONF_SD_SPI_CS);

FATFileSystem fs("sd", &sd);

Thread resource_thread;
void *network_interface(NULL);

int initPlatform() {
    int sd_ret;

    init_screen();
    sd_ret = sd.init();
    if(sd_ret != BD_ERROR_OK) {
        tr_error("initPlatform() - sd.init() failed with %d\n", sd_ret);
        return -1;
    }
    tr_debug("initPlatform() - SD card init OK.\n");

    return 0;
}

void increment_resource(void const* arg) {
    SimpleM2MClient *client;
    client = (SimpleM2MClient*) arg;
    while(true) {
        Thread::wait(INCREMENT_INTERVAL);
        if(client->is_client_registered()) {
            client->increment_resource_value();
#if defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP) || \
    defined(MBED_CLOUD_CLIENT_TRANSPORT_MODE_UDP_QUEUE)
            client->keep_alive();
#endif
        }
    }
}

bool init_connection() {
    srand(time(NULL));
    network_interface = easy_connect(true);
    if(network_interface == NULL) {
        return false;
    }
    return true;
}

void* get_network_interface() {
    return network_interface;
}

void init_screen() {
#ifdef MBED_APPLICATION_SHIELD
    /* Turn off red LED */
    DigitalOut ext_red(D5, 1);

    /* Turn on green LED */
    DigitalOut ext_green(D8, 0);

    lcd = new C12832(D11, D13, D12, D7, D10);
#endif
}

void print_to_screen(int x, int y, const char* buffer)
{
#ifdef MBED_APPLICATION_SHIELD
    lcd->locate(x, y);

    /* limit size to 25 characters */
    char output_buffer[26] = { 0 };

    size_t name_length = strnlen(buffer, 32);

    /* if buffer is 32 characters, assume FlakeID */
    if (name_length == 32)
    {
        /* < 64 bit timestamp >< 48 bit worker id>< 16 bit sequence number >
           Discard 7 characters form worker ID but keep the timestamp and
           sequence number
        */
        memcpy(&output_buffer[0], &buffer[0], 16);
        memcpy(&output_buffer[16], &buffer[23], 9);
    }
    else
    {
        /* fill output buffer with buffer */
        strncpy(output_buffer, buffer, 25);
    }

    lcd->printf("%s", output_buffer);
#endif
}

void clear_screen()
{
#ifdef MBED_APPLICATION_SHIELD
    lcd->cls();
#endif
}

void print_heap_stats() {
    heap_stats();
}

void print_m2mobject_stats() {
    m2mobject_stats();
}

void create_m2mobject_test_set(M2MObjectList* object_list) {
    m2mobject_test_set(*object_list);
}

void do_wait(int timeout_in_sec) {
    wait_ms(timeout_in_sec * 1000);
}

void increment_resource_thread(void* client) {
    resource_thread.start(callback(increment_resource, (void*)client));
}
