rm -rf ../platform
rm -rf ../mbed-cloud-client/mbed-client-pal/Test
rm -rf ../mbed-cloud-client/factory-configurator-client/mbed-client-esfs/CMakeLists.txt
rm -rf ../mbed-os
python ../replace_line.py ../source/setup.h "extern int initPlatform();" "//extern int initPlatform();"
python ../replace_line.py ../pal-platform/Middleware/mbedtls/mbedtls/include/mbedtls/config.h "//#define MBEDTLS_CMAC_C" "#define MBEDTLS_CMAC_C"