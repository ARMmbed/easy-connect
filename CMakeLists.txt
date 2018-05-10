cmake_minimum_required(VERSION 2.6)
# temp workaround
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/pal-platform/SDK/K64F_FreeRTOS/K64F_FreeRTOS/OS/freeRTOS/CMSIS-SP-00300-r4p5-00rel0/CMSIS/RTOS/Template)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/mbed-trace/mbed-trace)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/mbed-client)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/mbed-client-pal/Source/PAL-Impl/Services-API)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/mbed-trace)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/mbed-client/mbed-client)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/e2eIoT-test-device/qe-device-infra/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/e2eIoT-test-device/mbed_client_cli/source/ns_list_internal)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/e2eIoT-test-device/qe-device-infra/os-specific-source/Linux)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/mbed-cloud-client/nanostack-libservice/mbed-client-libservice)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/GuruxDLMS/development/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/e2eIoT-test-device/source)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/e2eIoT-test-device/mbed_client_cli)


if (CMAKE_BUILD_TYPE MATCHES Debug)
	add_definitions(-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_INFO -DPAL_NUMBER_OF_PARTITIONS=1 -DPAL_PRIMARY_PARTITION_PRIVATE=0 -DPAL_SECONDARY_PARTITION_PRIVATE=0)
	
else()
	add_definitions(-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_ERROR -DPAL_NUMBER_OF_PARTITIONS=1 -DPAL_PRIMARY_PARTITION_PRIVATE=0 -DPAL_SECONDARY_PARTITION_PRIVATE=0)
endif()

if (${OS_BRAND} MATCHES Linux)
	add_definitions(-DCLI_MODE -D__linux__ -DSA_PV_OS_LINUX -DMBED_CLOUD_CLIENT_ENDPOINT_TYPE=\"default\" -DMBED_CLOUD_CLIENT_TRANSPORT_MODE_TCP -DMBED_CLOUD_CLIENT_LIFETIME=0 -DMBED_CLOUD_CLIENT_SUPPORT_UPDATE -DSN_COAP_MAX_BLOCKWISE_PAYLOAD_SIZE=128 -DMBED_CLOUD_CLIENT_UPDATE_BUFFER=1024)
endif()

if (${OS_BRAND} MATCHES FreeRTOS)
	add_definitions(-D__FreeRTOS__ -DFREERTOS -DLWIP_DHCP -D__unix__ -DMBED_CLITEST_TCP)
endif()

add_definitions(
	-DMBED_TRACE_MAX_LEVEL=TRACE_LEVEL_INFO
	-DMBED_CONF_MBED_TRACE_ENABLE=1
	-DMBED_CONF_MBED_TRACE_FEA_IPV6=0
)

# source files
File(
	 GLOB_RECURSE
	 SECSRV_QE_DEVICE_SRC
	"${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/GuruxDLMS/development/src/*.cpp"
)

# list (APPEND SRC_LIBS "mbed-client-cli")


# if(CMAKE_COMPILER_IS_GNUCC)
CREATE_TEST_LIBRARY(${OS_BRAND}_GNUC "${SECSRV_QE_DEVICE_SRC}" "")
# endif()
# if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
# CREATE_TEST_LIBRARY(${OS_BRAND}_GNUC "${SECSRV_QE_DEVICE_SRC}" "")
# endif()
ADDSUBDIRS()