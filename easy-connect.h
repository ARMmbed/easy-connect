#ifndef __EASY_CONNECT_H__
#define __EASY_CONNECT_H__

#include "mbed.h"

#define ETHERNET        1
#define WIFI_ESP8266    2
#define MESH_LOWPAN_ND  3
#define MESH_THREAD     4
#define WIFI_ODIN       5
#define WIFI_REALTEK    6

#if MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ESP8266
#include "ESP8266Interface.h"

#ifdef MBED_CONF_APP_ESP8266_DEBUG
ESP8266Interface wifi(MBED_CONF_APP_ESP8266_TX, MBED_CONF_APP_ESP8266_RX, MBED_CONF_APP_ESP8266_DEBUG);
#else
ESP8266Interface wifi(MBED_CONF_APP_ESP8266_TX, MBED_CONF_APP_ESP8266_RX);
#endif

#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ODIN
#include "OdinWiFiInterface.h"

OdinWiFiInterface wifi;
#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_RTW
#include "RTWInterface.h"
RTWInterface wifi;
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
#else
#error "No connectivity method chosen. Please add 'config.network-interfaces.value' to your mbed_app.json (see README.md for more information)."
#endif

#if defined(MESH)
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
#endif //MBED_CONF_APP_RADIO_TYPE
#endif //MESH

#ifndef MESH
// This is address to mbed Device Connector
#define MBED_SERVER_ADDRESS "coap://api.connector.mbed.com:5684"
#else
// This is address to mbed Device Connector
#define MBED_SERVER_ADDRESS "coaps://[2607:f0d0:2601:52::20]:5684"
#endif

#ifdef MBED_CONF_APP_ESP8266_SSID
#define MBED_CONF_APP_WIFI_SSID MBED_CONF_APP_ESP8266_SSID
#endif

#ifdef MBED_CONF_APP_ESP8266_PASSWORD
#define MBED_CONF_APP_WIFI_PASSWORD MBED_CONF_APP_ESP8266_PASSWORD
#endif

/* \brief print_MAC - print_MAC  - helper function to print out MAC address
 * in: network_interface - pointer to network i/f
 *     bool log-messages   print out logs or not
 * MAC address is print, if it can be acquired & log_messages is true.
 *
 */
void print_MAC(NetworkInterface* network_interface, bool log_messages) {
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
}


/* \brief easy_connect - easy_connect function to connect the pre-defined network bearer,
 *                       config done via mbed_app.json (see README.md for details).
 * IN: bool log_messages  print out diagnostics or not.
 *
 */
NetworkInterface* easy_connect(bool log_messages = false) {
    NetworkInterface* network_interface = 0;
    int connect_success = -1;
    /// This should be removed once mbedOS supports proper dual-stack
#if defined (MESH) || (MBED_CONF_LWIP_IPV6_ENABLED==true)
    printf("[EasyConnect] IPv6 mode\n");
#else
    printf("[EasyConnect] IPv4 mode\n");
#endif

 #if MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ESP8266
    if (log_messages) {
        printf("[EasyConnect] Using WiFi (ESP8266) \n");
        printf("[EasyConnect] Connecting to WiFi %s\n", MBED_CONF_APP_WIFI_SSID);
    }
    network_interface = &wifi;
    connect_success = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_ODIN
    if (log_messages) {
        printf("[EasyConnect] Using WiFi (ODIN) \n");
        printf("[EasyConnect] Connecting to WiFi %s\n", MBED_CONF_APP_WIFI_SSID);
    }
    network_interface = &wifi;
    connect_success = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
#elif MBED_CONF_APP_NETWORK_INTERFACE == WIFI_RTW
    if (log_messages) {
        printf("[EasyConnect] Using WiFi (RTW)\n");
        printf("[EasyConnect] Connecting to WiFi %s\n", MBED_CONF_APP_WIFI_SSID);
    }
    network_interface = &wifi;
    connect_success = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA_WPA2);
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

#endif // __EASY_CONNECT_H__
