# e2e-iot-dlms-server

## steps to run on linux
`./build_server.sh`

### clean and compile:

`./build_server.sh -c`

## steps to run on mbed-cli for K64F
`python ./devenv/update_repository.py mbed`

`./devenv/qe_env_setup.sh K64F MBEDOS GNUC`

`mbed compile -t GCC_ARM -m K64F -j 16`

## steps to run on mbed-cli for ODIN
`python ./devenv/update_repository.py mbed`

`./devenv/qe_env_setup.sh UBLOX_EVK_ODIN_W2 MBEDOS GNUC`

`mbed compile -t GCC_ARM -m UBLOX_EVK_ODIN_W2 -j 16`

the binary file will be in path:
./BUILD/UBLOX_EVK_ODIN_W2/GCC_ARM/e2e-iot-dlms-server.bin

## drop packets from command line:
* `-dropr           <hexadeciamal number starts with 0x>` - drop received packets.
* `-drops           <hexadeciamal number starts with 0x>` - drop sent packets.

The hexadecimal number will indiacte the indexes of the packets you want to drop, by its binary representation.
for example: if you type 0x152 you will drop the 2nd, 5th, 7th and 9th packets, because the binary representation is 000101010010


## command line in cli
### to start the server:
`start <argument> <argument value>`

* `-i           <hdlc/wrapper>` - default wrraper
* `-protocol    <tcp/udp>` - default udp
* `-p           <port number>` - default 4061
* `-max         <max pdu size>` - default 1024
* `-c           <conformance block (in hexadecimal - can start with 0x)>` - default as Yael requested
* `-print`      print the received and sent packets - default is not printing
* `-k`          <key number (1 - 10)> - default is 1
* `-set <obj>   <obj val>` - <obj> options are detailed below  

### while the server is running (<obj> options are -hum, -cur, -pow):
`conf <argument> <argument value>`

* `-hum  <val>` - humidity
* `-cur  <val>` - current
* `-pow  <val>` - power
* `-print`      - print the received and sent packets
* `-stoprint`   - stop print the received and sent packets

### to end the server:
`end`


## steps to run on linux-cli (currently not supported)
`python ./devenv/update_repository.py linux`

`. devenv/qe_env_setup.sh PC Linux GNUC`

`make`

`./Debug/Linux_GNUC.elf`

### handle entropy
The program can get stucked when running in this mode before the server starts to run. That's happen in function "pal_init" because of non-available entropy.
To solve this you need to do next steps for only one time:
1. get out of the docker
2. `sudo apt-get install rng-tools`
3. `sudo rngd -r /dev/urandom`






