# Easy Connect - Easily add all supported connectivity methods to your Mbed OS project

## Deprecation note

Please note that starting with [Mbed OS 5.10](https://github.com/ARMmbed/mbed-os/tree/mbed-os-5.10) onwards the network driver can be instantiated using `get_default_instance()`. 

**This repository will not be maintained going onwards, as the work will be done using the official Mbed OS repository.**

### Use get_default_instance()

You can use the boards default network interface as illustrated in the example below.

```C
    network_interface = NetworkInterface::get_default_instance();
    if(network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return -1;
    }
```

More information will be available in the Mbed OS Handbook under Network interfaces and configuration. There are also [examples](https://github.com/search?q=mbed-os-example&type=Repositories) available.

### Override get_default_instance()

The default network interface can be overridden via `mbed_app.json` file with
`            "target.network-default-interface-type" : "XXXX",`
where `XXXX` matches the network stack to be used (for example WIFI).

You can then define for example ESP8266 to be the new default with pins D0 and D1.

```json
    "target_overrides": {
            "esp8266.rx"                : "D0",
            "esp8266.tx"                : "D1",
            "esp8266.provide-default"   : true,
    }
```

The esp8266.provide-default activates a macro, which will override the weak `get_default_interface()` with the one [defined for ESP8266](https://github.com/ARMmbed/mbed-os/blob/mbed-os-5.10/components/wifi/esp8266-driver/ESP8266Interface.cpp#L579-L586).

### Manual instantiation of a network driver

Not all network drivers support yet the default instantiation so alternatively you can pull in any external network driver and  instantiate the instance of that driver directly, as is done currently by `easy-connect` itself. Add the network driver repository 1st, for example wifi-ism43362.

```
mbed add wifi-ism43362
```

After which you can instantiate the driver by 1st including its header.

```C
#include "ISM43362Interface.h"
...

ISM43362Interface wifi(PC_12, PC_11, PC_10, PE_0, PE_8, PE_1, PB_12, false);
...

int connect_status;

connect_status = wifi.connect(SSID, PASSWORD,NSAPI_SECURITY_WPA_WPA2);
if (connect_status == 0 ) {
    // Connected succesfully
    ...
}

```

# Deprecated content

Information below is for historial reference and is valid only if you are using easy-connect with Mbed OS 5.9 and earlier.

## Introduction

You may want to give the users of your application the possibility to switch between connectivity methods. The `NetworkInterface` API makes this easy, but you still need a mechanism for the user to chooce the method,  and perhaps throw in some `#define`'s. Easy Connect handles all of this for you. Just declare the desired connectivity method in your `mbed_app.json` file and call `easy_connect()` from your application.

## Specifying the connectivity method

Add the following to your `mbed_app.json` file:

```json
{
    "config": {
        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW0XX1, WIFI_ODIN, WIFI_RTW, WIFI_WIZFI310, WIFI_ISM43362, WIFI_ESP32, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
            "value": "ETHERNET"
        }
    },
    "target_overrides": {
        "*": {
            "target.features_add": ["NANOSTACK", "LOWPAN_ROUTER", "COMMON_PAL"],
            "mbed-mesh-api.6lowpan-nd-channel-page": 0,
            "mbed-mesh-api.6lowpan-nd-channel": 12
        }
    }
}
```

### UBLOX ODIN/Ethernet

#### Mbed OS 5.8 and older

If you select `ETHERNET` with `UBLOX_ODIN_EVK_W2` you must add this to your `target-overrides` section in `mbed_app.json`:

```json
            "UBLOX_EVK_ODIN_W2": {
            "target.device_has_remove": ["EMAC"]
            }
```

#### Mbed OS 5.9 and newer

With Mbed OS 5.9, the EMAC SW was refactored and a default network selector is used instead. You must add the following `target-overrides` section to `mbed_app.json`:

```json
        "UBLOX_EVK_ODIN_W2": {
             "target.network-default-interface-type": "ETHERNET"
       }
```

### Other WiFi stacks

If you select `WIFI_ESP8266`, `WIFI_IDW0XX1`, `WIFI_ODIN`, `WIFI_RTW`, `WIFI_WIZFI310` or `WIFI_ESP32` you also need to add the WiFi SSID and password:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW0XX1, WIFI_ODIN, WIFI_RTW, WIFI_WIZFI310, WIFI_ISM43362, WIFI_ESP32, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
            "value": "WIFI_ESP8266"
        },
        "wifi-ssid": {
            "value": "\"SSID\""
        },
        "wifi-password": {
            "value": "\"Password\""
        }
    }
```

If you use `MESH_LOWPAN_ND` or `MESH_THREAD` you need to specify your radio module:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW0XX1, WIFI_ODIN, WIFI_RTW, WIFI_WIZFI310, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
            "value": "MESH_LOWPAN_ND"
        },
        "mesh_radio_type": {
        	"help": "options are ATMEL, MCR20, SPIRIT1, EFR32",
        	"value": "ATMEL"
        }
    }
```

### CELLULAR_ONBOARD

If you use [`CELLULAR_ONBOARD`](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/cellular/) you must specify the following:

```json
    "target_overrides": {
        "*": {
            "ppp-cell-iface.apn-lookup": true
        }
    }
```
...and you may also need to specify one or more of the following:

```json
    "config": {
        "cellular-apn": {
            "help": "Please provide the APN string for your SIM if it is not already included in APN_db.h.",
            "value": "\"my_sims_apn\""
        },
        "cellular-username": {
            "help": "May or may not be required for your APN, please consult your SIM provider.",
            "value": "\"my_sim_apns_username\""
        },
        "cellular-password": {
            "help": "May or may not be required for your APN, please consult your SIM provider.",
            "value": "\"my_sim_apns_password\""
        },
        "cellular-sim-pin": {
            "help": "Please provide the PIN for your SIM (as a four digit string) if your SIM is normally locked",
            "value": "\"1234\""
        }
    }
```

None of the optional settings need to be specified for the `UBLOX_C030_U201` cellular target, for which the APN settings are in `APN_db.h`.

## Using Easy Connect from your application

Easy Connect has just one function that returns either a `NetworkInterface`-pointer or `NULL`:

```cpp
#include "easy-connect.h"

int main(int, char**) {
    NetworkInterface* network = easy_connect(true); /* has 1 argument, enable_logging (pass in true to log to serial port) */
    if (!network) {
        printf("Connecting to the network failed... See serial output.\r\n");
        return 1;
    }

    // Rest of your program
}
```

## Using Easy connect with WiFi

The easy-connect `easy_connect()` is overloaded now for WiFi so that you can submit your WiFi SSID and password programmatically in you want
the user to be able to supply them via some means.

```cpp
#include "easy-connect.h"

int main(int, char**) {
    char* wifi_SSID = "SSID";
    char* wifi_password = "password";

    NetworkInterface* network = easy_connect(true, wifi_SSID, wifi_password); 
    if (!network) {
        printf("Connecting to the network failed... See serial output.\r\n");
        return 1;
    }

    // Rest of your program
}
```

## Overriding settings

Easy-connect was changed recently with [PR #59](https://github.com/ARMmbed/easy-connect/pull/59) - where some of the defines expected via `mbed_app.json` were
moved to the [`mbed_lib.json`](https://github.com/ARMmbed/easy-connect/blob/master/mbed_lib.json). 
This minimises the amount of lines needed (in typical cases) in the applications `mbed_app.json`. However, due to this the overrides
need to be done slightly differently, as you need to override the `easy-connect` defines.

So, for example changing the ESP8266 TX/RX pins and enable debugs - you would now have modify as below.

```json
    "target_overrides": {
        "*": {
            "easy-connect.wifi-esp8266-tx": "A1",
            "easy-connect.wifi-esp8266-rx": "A2",
            "easy-connect.wifi-esp8266-debug: true
         }
    }
```


## Configuration examples

There are many things that you have to modify for all of the combinations. Examples for configurations are available for example in the [mbed-os-example-client](https://github.com/ARMmbed/mbed-os-example-client/tree/master/configs) repository.

## Linking error with UBLOX_EVK_ODIN_W2

If you get a linking error such as below, you are compiling the `WIFI_ODIN` with the `EMAC override` section in `mbed_app.json`. Remove the `EMAC override` from your `mbed_app.json`. 

```
Link: tls-client
./mbed-os/targets/TARGET_STM/TARGET_STM32F4/TARGET_UBLOX_EVK_ODIN_W2/sdk/TOOLCHAIN_GCC_ARM/libublox-odin-w2-driver.a(OdinWiFiInterface.o): In function `OdinWiFiInterface::handle_wlan_status_started(wlan_status_started_s*)':
OdinWiFiInterface.cpp:(.text._ZN17OdinWiFiInterface26handle_wlan_status_startedEP21wlan_status_started_s+0x46): undefined reference to `wifi_emac_get_interface()'
OdinWiFiInterface.cpp:(.text._ZN17OdinWiFiInterface26handle_wlan_status_startedEP21wlan_status_started_s+0x4c): undefined reference to `wifi_emac_init_mem()'
collect2: error: ld returned 1 exit status
[ERROR] ./mbed-os/targets/TARGET_STM/TARGET_STM32F4/TARGET_UBLOX_EVK_ODIN_W2/sdk/TOOLCHAIN_GCC_ARM/libublox-odin-w2-driver.a(OdinWiFiInterface.o): In function `OdinWiFiInterface::handle_wlan_status_started(wlan_status_started_s*)':
OdinWiFiInterface.cpp:(.text._ZN17OdinWiFiInterface26handle_wlan_status_startedEP21wlan_status_started_s+0x46): undefined reference to `wifi_emac_get_interface()'
OdinWiFiInterface.cpp:(.text._ZN17OdinWiFiInterface26handle_wlan_status_startedEP21wlan_status_started_s+0x4c): undefined reference to `wifi_emac_init_mem()'
collect2: error: ld returned 1 exit status

[mbed] ERROR: "/usr/bin/python" returned error code 1.
```

## Network errors

If Easy Connect cannot connect to the network, it returns a network error with an error code. To see what the error code means, see the [mbed OS Communication API](https://os.mbed.com/docs/latest/reference/network-socket.html).

## CR/LF in serial output

If you want to avoid using `\r\n` in your printouts and just use normal C style `\n` instead, please specify these to your `mbed_app.json`:

```json
       "target_overrides": {
        "*": {
            "platform.stdio-baud-rate": 115200,
            "platform.stdio-convert-newlines": true
        }
    }
```

## For network stack developers

Please try out the reliability of your networking stack using
[stress-test](https://github.com/ARMmbed/mbed-stress-test) to ensure
your stack is performing as expected.

## Extra defines

If you'd like to use Easy Connect with mbed Client then you're in luck. Easy Connect automatically defines the `MBED_SERVER_ADDRESS` macro depending on your connectivity method (either IPv4 or IPv6 address). Use this address to connect to the right instance of mbed Device Connector.
