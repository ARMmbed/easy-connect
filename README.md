# e2e-iot-dlms-server

## steps to run on linux:
`./build_server.sh`

`./bin/dlms-sim-server`

### clean the build:

`./build_server.sh -c`

## steps to run on linux-cli:
'python ./devenv/update_repository.py linux'

'. devenv/qe_env_setup.sh PC Linux GNUC'

'../prepare_lib.sh'

'make'

'./Debug/Linux_GNUC.elf'

## steps to run on mbed-cli:
'python ./devenv/update_repository.py mbed'

'./devenv/qe_env_setup.sh K64F MBEDOS GNUC'

'mbed compile -t GCC_ARM -m K64F -j 16'

the binary file will be in path:
./BUILD/K64F/GCC_ARM/e2e-iot-dlms-server.bin
