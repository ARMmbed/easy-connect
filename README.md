# Easy Connect - Easily add all supported connectivity methods to your mbed OS project

You may want to give the users of your application the possibility to switch between connectivity methods. The `NetworkInterface` API makes this easy, but you still need a mechanism for the user to chooce the method,  and perhaps throw in some `#define`'s. Easy Connect handles all of this for you. Just declare the desired connectivity method in your `mbed_app.json` file and call `easy_connect()` from your application.

## Specifying the connectivity method

Add the following to your `mbed_app.json` file:

```json
{
    "config": {
        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW01M1, WIFI_ODIN, WIFI_RTW, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
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

If you select `ETHERNET` with `UBLOX_ODIN_EVK_W2` you must add this to your `target-overrides` section in `mbed_app.json`:

```json
            "UBLOX_EVK_ODIN_W2": {
            "target.device_has_remove": ["EMAC"]
            }
```

If you select `WIFI_ESP8266`, `WIFI_IDW01M1`, `WIFI_ODIN` or `WIFI_RTW`, you also need to add the WiFi SSID and password:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW01M1, WIFI_ODIN, WIFI_RTW, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
            "value": "WIFI_ESP8266"
        },
        "esp8266-tx": {
            "help": "Pin used as TX (connects to ESP8266 RX)",
            "value": "PTD3"
        },
        "esp8266-rx": {
            "help": "Pin used as RX (connects to ESP8266 TX)",
            "value": "PTD2"
        },
        "esp8266-debug": {
            "value": true
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
            "help": "options are ETHERNET, WIFI_ESP8266, WIFI_IDW01M1, WIFI_ODIN, WIFI_RTW, MESH_LOWPAN_ND, MESH_THREAD, CELLULAR_ONBOARD",
            "value": "MESH_LOWPAN_ND"
        },
        "mesh_radio_type": {
        	"help": "options are ATMEL, MCR20, SPIRIT1, EFR32",
        	"value": "ATMEL"
        }
    }
```

If you use `CELLULAR_ONBOARD` (for which user documentation can be found [here](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/cellular/)) you must specify the following:

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

## Configuration examples

There are many things that you have to modify for all of the combinations. Examples for configurations are available for example in the [mbed-os-example-client](https://github.com/ARMmbed/mbed-os-example-client/tree/master/configs) repository.

## Compilation error NanostackRfPhyAtmel.cpp

If you encounter a compilation error such as below, you need to add an `.mbedignore` file that tells the mbed compiler to skip compiling the files that require Nanostack. By default, the mbed compiler compiles every single file from all folders.

```
Scan: env
Compile [  0.2%]: NanostackRfPhyAtmel.cpp
[Fatal Error] NanostackRfPhyAtmel.cpp@18,44: nanostack/platform/arm_hal_phy.h: No such file or directory
[ERROR] ./easy-connect/atmel-rf-driver/source/NanostackRfPhyAtmel.cpp:18:44: fatal error: nanostack/platform/arm_hal_phy.h: No such file or directory
 #include "nanostack/platform/arm_hal_phy.h"
                                            ^
compilation terminated.

```

An example of a suitable `.mbedignore` file is available in the [mbed-os-example-client](https://github.com/ARMmbed/mbed-os-example-client/tree/master/configs) repository.

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

If Easy Connect cannot connect to the network, it returns a network error with an error code. To see what the error code means, see the [mbed OS Communication API](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/network_sockets/#network-errors).

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

## Extra defines

If you'd like to use Easy Connect with mbed Client then you're in luck. Easy Connect automatically defines the `MBED_SERVER_ADDRESS` macro depending on your connectivity method (either IPv4 or IPv6 address). Use this address to connect to the right instance of mbed Device Connector.
