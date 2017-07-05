# Easy Connect - Easily add all supported connectivity methods to your mbed OS project

Often you want to give users of your application the choice to switch between connectivity methods. The `NetworkInterface` API makes this easy, but you'll still need a mechanism for the user to chooce the method, throw in some `#define`'s, etc. Easy Connect handles all this for you. Just declare the desired connectivity method in your `mbed_app.json` file, and call `easy_connect()` from your application.

## Specifying connectivity method

Add the following to your ``mbed_app.json`` file:

```json
{
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI_ESP8266,WIFI_ODIN, WIFI_REALTEK, MESH_LOWPAN_ND,MESH_THREAD",
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
If you choose `ETHERNET` with `UBLOX_ODIN_EVK_W2` you must add this to your `target-overrides` section in `mbed_app.json`:
```json
            "UBLOX_EVK_ODIN_W2": {
            "target.device_has_remove": ["EMAC"]
```

If you choose `WIFI_ESP8266`, `WIFI_ODIN` or `WIFI_REALTEK`, you'll also need to add the WiFi SSID and password:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI_ESP8266,WIFI_ODIN,WIFI_REALTEK,MESH_LOWPAN_ND,MESH_THREAD",
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

If you use `MESH_LOWPAN_ND` or `MESH_THREAD` you will need to specify your radio module:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI_ESP8266,WIFI_ODIN,MESH_LOWPAN_ND,MESH_THREAD",
            "value": "MESH_LOWPAN_ND"
        },
        "mesh_radio_type": {
        	"help": "options are ATMEL, MCR20, SPIRIT1",
        	"value": "ATMEL"
        }
    }
```

## Using Easy Connect from your application

Easy Connect has just one function which will either return a `NetworkInterface`-pointer or `NULL`:

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
## CR/LF in serial output

If you want to avoid using `\r\n` in your printouts and just use normal C-style `\n` instead, please specify these to your `mbed_app.json`

```json
       "target_overrides": {
        "*": {
            "platform.stdio-baud-rate": 115200,
            "platform.stdio-convert-newlines": true
        }
    }
```

## Network errors

If easy-connect cannot connect to the network it returns a network error, with an error code. To see what these error code mean, see the [mbed OS Communication API](https://docs.mbed.com/docs/mbed-os-api-reference/en/latest/APIs/communication/network_sockets/#network-errors).

## Extra defines

If you'd like to use Easy Connect with mbed Client then you're in luck. Easy Connect automatically defines the `MBED_SERVER_ADDRESS` macro depending on your connectivity method (either IPv4 or IPv6 address). Use this address to connect to the right instance of mbed Device Connector.
