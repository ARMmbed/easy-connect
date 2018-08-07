# e2e-iot-dlms-server

## steps to run on linux
`./build_server.sh`

`./bin/dlms-sim-server`

### clean and compile:

`./build_server.sh -c`

### only clean:

`./build_server.sh -clean`


## steps to run on linux-cli
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

## steps to run on mbed-cli
`python ./devenv/update_repository.py mbed`

`./devenv/qe_env_setup.sh K64F MBEDOS GNUC`

`mbed compile -t GCC_ARM -m K64F -j 16`

the binary file will be in path:
./BUILD/K64F/GCC_ARM/e2e-iot-dlms-server.bin

## command line in cli

## command line in cli
### to start the server:
`start <argument> <argument value>`

* `-i           <hdlc/wrapper>` - default wrraper
* `-protocol    <tcp/udp>` - default udp
* `-p           <port number>` - default 4061
* `-m           <max pdu size>` - default 1024
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







