# Easy Connect - Easily add all supported connectivity methods to your mbed OS project

Often you want to give users of your application the choice to switch between connectivity methods. The `NetworkInterface` API makes this easy, but you'll still need a mechanism for the user to chooce the method, throw in some `#define`'s, etc. Easy Connect handles all this for you. Just declare the desired connectivity method in your `mbed_app.json` file, and call `easy_connect()` from your application.

## Specifying connectivity method

Add the following to your ``mbed_app.json`` file:

```json
{
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI,MESH_LOWPAN_ND,MESH_THREAD",
            "value": "ETHERNET"
        }
    },
    "target_overrides": {
        "*": {
            "target.features_add": ["IPV6", "IPV4"]
        }
    }
}
```

If you choose `WIFI`, you'll also need to add the WiFi SSID and password:

```json
    "config": {
        "network-interface":{
            "help": "options are ETHERNET,WIFI,MESH_LOWPAN_ND,MESH_THREAD",
            "value": "WIFI"
        },
        "wifi-ssid": {
            "help": "WiFi SSID",
            "value": "\"SSID\""
        },
        "wifi-password": {
            "help": "WiFi Password",
            "value": "\"Password\""
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

## Extra defines

If you'd like to use Easy Connect with mbed Client then you're in luck. Easy Connect automatically defines the `MBED_SERVER_ADDRESS` macro depending on your connectivity method (either IPv4 or IPv6 address). Use this address to connect to the right instance of mbed Cloud.
