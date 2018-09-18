# Easy Connect - Easily add all supported connectivity methods to your Mbed OS project

## Deprecation note

Please note that with [Mbed OS 5.10](https://github.com/ARMmbed/mbed-os/tree/mbed-os-5.10) onwards the network driver can be instantiated using `get_default_instance()`. 

**This repository will not be maintained going onwards, as the work will be done using the [official Mbed OS repository](https://github.com/ARMmbed/mbed-os).**

### Use get_default_instance()

You can use the boards default network interface:

```C
    network_interface = NetworkInterface::get_default_instance();
    if(network_interface == NULL) {
        printf("ERROR: No NetworkInterface found!\n");
        return -1;
    }
```

More information will be available in the Mbed OS Handbook under Network interfaces and configuration. There are also [examples](https://github.com/search?q=mbed-os-example&type=Repositories) available.

### Override get_default_instance()

You can overridde the default network interface through the `mbed_app.json` file with
`            "target.network-default-interface-type" : "XXXX",`
where `XXXX` matches the network stack to be used (for example WIFI).

You can then define, for example, ESP8266 to be the new default with pins D0 and D1.

```json
    "target_overrides": {
            "esp8266.rx"                : "D0",
            "esp8266.tx"                : "D1",
            "esp8266.provide-default"   : true,
    }
```

The esp8266.provide-default activates a macro, which will override the weak method `get_default_interface()` with the one [defined for ESP8266](https://github.com/ARMmbed/mbed-os/blob/mbed-os-5.10/components/wifi/esp8266-driver/ESP8266Interface.cpp#L579-L586).

### Manual instantiation of a network driver

All network drivers do not yet support default instantiation so alternatively you can pull in any external network driver and  instantiate that driver directly, as is done currently by `easy-connect` itself. Add the network driver repository 1st, for example wifi-ism43362.

```
mbed add wifi-ism43362
```

After which you can instantiate the driver by 1st including its header:

```C
#include "ISM43362Interface.h"
...

ISM43362Interface wifi(PC_12, PC_11, PC_10, PE_0, PE_8, PE_1, PB_12, false);
...

int connect_status;

connect_status = wifi.connect(SSID, PASSWORD, NSAPI_SECURITY_WPA_WPA2);
if (connect_status == 0 ) {
    // Connected succesfully
    ...
}

```

### Old README.md

The [old README](README-old.md) is still available.
