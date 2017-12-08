/*
 * FILE: easy-connect.cpp
 *
 * Copyright (c) 2015 - 2017 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "easy-connect.h"

/*
 * Instantiate the configured network interface
 */
#if MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ESP8266
#include "ESP8266Interface.h"
#define WIFI_TYPE "ESP8266"

#ifdef MBED_CONF_APP_ESP8266_DEBUG
ESP8266Interface wifi(MBED_CONF_APP_ESP8266_TX, MBED_CONF_APP_ESP8266_RX, MBED_CONF_APP_ESP8266_DEBUG);
#else
ESP8266Interface wifi(MBED_CONF_APP_ESP8266_TX, MBED_CONF_APP_ESP8266_RX);
#endif

#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ODIN
#define WIFI_TYPE "Odin"
#include "OdinWiFiInterface.h"
OdinWiFiInterface wifi;

#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_RTW
#define WIFI_TYPE "RTW"
#include "RTWInterface.h"
RTWInterface wifi;

#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_IDW0XX1
#define WIFI_TYPE "IDW0XX1"
#include "SpwfSAInterface.h"
SpwfSAInterface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);

#elif MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET
#include "EthernetInterface.h"
EthernetInterface eth;

#elif MBED_CONF_APP_NETWORK_INTERFACE == MESH_LOWPAN_ND
#define MESH
#include "NanostackInterface.h"
LoWPANNDInterface mesh;

#elif MBED_CONF_APP_NETWORK_INTERFACE == MESH_THREAD
#define MESH
#include "NanostackInterface.h"
ThreadInterface mesh;

#elif MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_ONBOARD
#include "OnboardCellularInterface.h"
OnboardCellularInterface cellular;

#else
#error "No connectivity method chosen. Please add 'config.network-interfaces.value' to your mbed_app.json (see README.md for more information)."
#endif // MBED_CONF_APP_NETWORK_INTERFACE

/*
 * In case of Mesh, instantiate the configured RF PHY.
 */
#if defined (MESH)
#if MBED_CONF_APP_MESH_RADIO_TYPE == ATMEL
#include "NanostackRfPhyAtmel.h"
NanostackRfPhyAtmel rf_phy(ATMEL_SPI_MOSI, ATMEL_SPI_MISO, ATMEL_SPI_SCLK, ATMEL_SPI_CS,
                           ATMEL_SPI_RST, ATMEL_SPI_SLP, ATMEL_SPI_IRQ, ATMEL_I2C_SDA, ATMEL_I2C_SCL);

#elif MBED_CONF_APP_MESH_RADIO_TYPE == MCR20
#include "NanostackRfPhyMcr20a.h"
NanostackRfPhyMcr20a rf_phy(MCR20A_SPI_MOSI, MCR20A_SPI_MISO, MCR20A_SPI_SCLK, MCR20A_SPI_CS, MCR20A_SPI_RST, MCR20A_SPI_IRQ);

#elif MBED_CONF_APP_MESH_RADIO_TYPE == SPIRIT1
#include "NanostackRfPhySpirit1.h"
NanostackRfPhySpirit1 rf_phy(SPIRIT1_SPI_MOSI, SPIRIT1_SPI_MISO, SPIRIT1_SPI_SCLK,
                             SPIRIT1_DEV_IRQ, SPIRIT1_DEV_CS, SPIRIT1_DEV_SDN, SPIRIT1_BRD_LED);

#elif MBED_CONF_APP_MESH_RADIO_TYPE == EFR32
#include "NanostackRfPhyEfr32.h"
NanostackRfPhyEfr32 rf_phy;

#endif // MBED_CONF_APP_RADIO_TYPE
#endif // MESH

#if defined (WIFI)
#define WIFI_SSID_MAX_LEN      32
#define WIFI_PASSWORD_MAX_LEN  64

char _ssid[WIFI_SSID_MAX_LEN+1] = {0};
char _password[WIFI_PASSWORD_MAX_LEN+1] = {0};
#endif // WIFI

/* \brief print_MAC - print_MAC  - helper function to print out MAC address
 * in: network_interface - pointer to network i/f
 *     bool log-messages   print out logs or not
 * MAC address is print, if it can be acquired & log_messages is true.
 *
 */
void print_MAC(NetworkInterface* network_interface, bool log_messages) {
#if MBED_CONF_APP_NETWORK_INTERFACE != CELLULAR_ONBOARD
    const char *mac_addr = network_interface->get_mac_address();
    if (mac_addr == NULL) {
        if (log_messages) {
            printf("[EasyConnect] ERROR - No MAC address\n");
        }
        return;
    }
    if (log_messages) {
        printf("[EasyConnect] MAC address %s\n", mac_addr);
    }
#endif
}



/* \brief easy_connect     easy_connect() function to connect the pre-defined network bearer,
 *                         config done via mbed_app.json (see README.md for details).
 * IN: bool  log_messages  print out diagnostics or not.
 */
NetworkInterface* easy_connect(bool log_messages) {
    NetworkInterface* network_interface = NULL;
    int connect_success = -1;

#if defined (WIFI)
    // We check if the _ssid and _password have already been set (via the easy_connect() that takes thoses parameters or not
    // If they have not been set, use the ones we can gain from mbed_app.json.
    if (strlen(_ssid) == 0) { 
        if(strlen(MBED_CONF_APP_WIFI_SSID) > WIFI_SSID_MAX_LEN) {
            printf("WARNING - MBED_CONF_APP_WIFI_SSID is too long - it will be cut to %d chars.\n", WIFI_SSID_MAX_LEN);
        }
        strncpy(_ssid, MBED_CONF_APP_WIFI_SSID, WIFI_SSID_MAX_LEN);
    }

    if (strlen(_password) == 0 ) {
        if(strlen(MBED_CONF_APP_WIFI_PASSWORD) > WIFI_PASSWORD_MAX_LEN) {
            printf("WARNING - MBED_CONF_APP_WIFI_PASSWORD is too long - it will be cut to %d chars.\n", WIFI_PASSWORD_MAX_LEN);
        }
        strncpy(_password, MBED_CONF_APP_WIFI_PASSWORD, WIFI_PASSWORD_MAX_LEN);
    }
    printf("_password = %s, len = %d\n", _password, strlen(_password));
#endif // WIFI

    /// This should be removed once mbedOS supports proper dual-stack
#if defined (MESH) || (MBED_CONF_LWIP_IPV6_ENABLED==true)
    printf("[EasyConnect] IPv6 mode\n");
#else
    printf("[EasyConnect] IPv4 mode\n");
#endif

#if defined (WIFI)
    if (log_messages) {
        printf("[EasyConnect] Using WiFi (%s) \n", WIFI_TYPE);
        printf("[EasyConnect] Connecting to WiFi %s\n", _ssid);
    }
    network_interface = &wifi;
    connect_success = wifi.connect(_ssid, _password, (strlen(_password) > 1) ? NSAPI_SECURITY_WPA_WPA2 : NSAPI_SECURITY_NONE);
#elif MBED_CONF_APP_NETWORK_INTERFACE == CELLULAR_ONBOARD
#  ifdef MBED_CONF_APP_CELLULAR_SIM_PIN
    cellular.set_sim_pin(MBED_CONF_APP_CELLULAR_SIM_PIN);
#  endif
#  ifdef MBED_CONF_APP_CELLULAR_APN
#    ifndef MBED_CONF_APP_CELLULAR_USERNAME
#      define MBED_CONF_APP_CELLULAR_USERNAME 0
#    endif
#    ifndef MBED_CONF_APP_CELLULAR_PASSWORD
#      define MBED_CONF_APP_CELLULAR_PASSWORD 0
#    endif
    cellular.set_credentials(MBED_CONF_APP_CELLULAR_APN, MBED_CONF_APP_CELLULAR_USERNAME, MBED_CONF_APP_CELLULAR_PASSWORD);
    if (log_messages) {
        printf("[EasyConnect] Connecting using Cellular interface and APN %s\n", MBED_CONF_APP_CELLULAR_APN);
    }
#  else
    if (log_messages) {
        printf("[EasyConnect] Connecting using Cellular interface and default APN\n");
    }
#  endif
    connect_success = cellular.connect();
    network_interface = &cellular;
#elif MBED_CONF_APP_NETWORK_INTERFACE == ETHERNET
    if (log_messages) {
        printf("[EasyConnect] Using Ethernet\n");
    }
    network_interface = &eth;
    connect_success = eth.connect();
#endif

#ifdef MESH
    if (log_messages) {
        printf("[EasyConnect] Using Mesh\n");
        printf("[EasyConnect] Connecting to Mesh..\n");
    }
    network_interface = &mesh;
    mesh.initialize(&rf_phy);
    connect_success = mesh.connect();
#endif
    if(connect_success == 0) {
        if (log_messages) {
            printf("[EasyConnect] Connected to Network successfully\n");
            print_MAC(network_interface, log_messages);
        }
    } else {
        if (log_messages) {
            print_MAC(network_interface, log_messages);
            printf("[EasyConnect] Connection to Network Failed %d!\n", connect_success);
        }
        return NULL;
    }
    const char *ip_addr  = network_interface->get_ip_address();
    if (ip_addr == NULL) {
        if (log_messages) {
            printf("[EasyConnect] ERROR - No IP address\n");
        }
        return NULL;
    }

    if (log_messages) {
        printf("[EasyConnect] IP address %s\n", ip_addr);
    }
    return network_interface;
}

/* \brief easy_connect - easy_connect function to connect the pre-defined network bearer,
 *                       config done via mbed_app.json (see README.md for details).
 *                       This version is just a helper version and uses the easy_connect() with
 *                       one parameters to do it's job.
 * IN: bool  log_messages  print out diagnostics or not.
 *     char* WiFiSSID      WiFi SSID - pointer to WiFi SSID, but if it is NULL  
 *                         then MBED_CONF_APP_WIFI_SSID will be used
 *     char* WiFiPassword  WiFi Password - pointer to WiFI password, but if it's NULL
 *                         then MBED_CONF_APP_WIFI_PASSWORD will be used
 */

NetworkInterface* easy_connect(bool log_messages,
                               char* WiFiSSID,
                               char* WiFiPassword ) {


#if defined (WIFI) // This function only makes sense when using WiFi
    // We essentially want to populate the _ssid and _password and then call easy_connect() again. 
    if (WiFiSSID != NULL) {
        if(strlen(WiFiSSID) > WIFI_SSID_MAX_LEN) {
            printf("WARNING - WiFi SSID is too long - it will be cut to %d chars.\n", WIFI_SSID_MAX_LEN);
        }
        strncpy(_ssid, WiFiSSID, WIFI_SSID_MAX_LEN);
    }
    else {
        strncpy(_ssid, MBED_CONF_APP_WIFI_SSID, WIFI_SSID_MAX_LEN);
    }

    if (WiFiPassword != NULL) {
        if(strlen(WiFiPassword) > WIFI_PASSWORD_MAX_LEN) {
            printf("WARNING - WiFi Password is too long - it will be cut to %d chars.\n", WIFI_PASSWORD_MAX_LEN);
        }
        strncpy(_password, WiFiPassword, WIFI_PASSWORD_MAX_LEN);
    }
    else {
        strncpy(_password, MBED_CONF_APP_WIFI_PASSWORD, WIFI_PASSWORD_MAX_LEN);
    }
#endif // WIFI
    return easy_connect(log_messages);
}
