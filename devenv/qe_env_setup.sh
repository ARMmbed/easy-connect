#!/usr/bin/env bash

#----------------------------------------------------------------------------
#   The confidential and proprietary information contained in this file may
#   only be used by a person authorised under and to the extent permitted
#   by a subsisting licensing agreement from ARM Limited or its affiliates.
#
#          (C) COPYRIGHT 2013-2016 ARM Limited or its affiliates.
#              ALL RIGHTS RESERVED
#
#   This entire notice must be reproduced on all copies of this file
#   and copies of this file may only be made by a person if such person is
#   permitted to do so under the terms of a subsisting license agreement
#   from ARM Limited or its affiliates.
#----------------------------------------------------------------------------
# source (!) this file while at tree top - DO NOT run it
export QE_ROOT=`pwd`
QE_PLATS="K64F_MBEDOS_ARMCC K64F_MBEDOS_GNUC K64F_FreeRTOS_GNUC K64F_FreeRTOS_ARMCC PC_Linux_GNUC UBLOX_EVK_ODIN_W2_MBEDOS_GNUC"

PLAT="$1"
OS="$2"
COMPILER="$3"

PLATFORM_ARG=$PLAT"_"$OS"_"$COMPILER
echo $PLATFORM_ARG

export QE_PLAT=$PLATFORM_ARG

cp devenv/mbed_app.json ./
rm -f makefile
cp devenv/makefile ./
echo "e2eIoT-test-device/*" > .mbedignore
echo "wifi/*" > mbed-os/components/.mbedignore
echo "unity/*" > mbed-os/features/frameworks/.mbedignore
echo "utest/*" >> mbed-os/features/frameworks/.mbedignore
echo "i2c_eeprom_asynch/*" > mbed-os/features/unsupported/tests/utest/.mbedignore
echo "Other/*" > mbed-cloud-client/mbed-client-pal/Utils/memoryProfiler/.mbedignore

spv_remove_duplicate_path_entries() {
	# This magical scriptlet removes from PATH all duplicates, keeping non-duplicated
	# items in their original order.
	# Taken from Stackoverflow.
	export PATH=$(echo "$PATH" | awk -v RS=':' -v ORS=":" '!a[$1]++{if (NR > 1) printf ORS; printf $a[$1]}')
}

spv_set_toolchain_native() {	
	export ARCH=`uname -m`
	unset CROSS_COMPILE
	export CC=gcc
	export AR=ar
	export LD=ld
	export CXX=g++

	
	export PATH=/usr/bin:$PATH
	spv_remove_duplicate_path_entries
}

spv_config_mbed_os() {
	#mbed config -G cache /opt/scratch/mbed-os5.1.0-cache
	mbed config protocol ssh
}

spv_set_toolchain_arm_common() {
	export ARCH=arm
	export CROSS_COMPILE=arm
	# Default license server in Dx for ARM tools
	#export ARMLMD_LICENSE_FILE=8224@kfn-lic01.kfn.arm.com
	# Supress errors for issues, such as "Waiting for license..."
	export ARMLMD_LICENSE_FILE=$QE_ROOT/devenv/licenses/arm_licenses.lic
	export ARMCC5_CCOPT=--diag_suppress=9933,C4017,C9931
	
	export CC=armcc
	export AR=armar
	export LD=armlink
}

spv_set_toolchain_arm_ds5() {	
	spv_set_toolchain_arm_common
	
	export SA_PV_DS5_COMPILER_DIR=/usr/local/DS-5_v5.25.0/ARMCompiler5.06u3
	export PATH=$SA_PV_DS5_COMPILER_DIR/bin:$PATH
	export ARM_PATH=$SA_PV_DS5_COMPILER_DIR/bin
	export ARMCC_DIR=$SA_PV_DS5_COMPILER_DIR
	mbed config ARM_PATH $SA_PV_DS5_COMPILER_DIR
	spv_remove_duplicate_path_entries
}

spv_set_toolchain_arm_gcc_6() {
	export ARCH=arm
	unset CROSS_COMPILE

	# "ARMGCC_DIR" is being used by FreeRTOS build system
	export ARMGCC_DIR=/opt/arm/gcc6-arm-none-eabi

	export CC=$ARMGCC_DIR/bin/arm-none-eabi-gcc
	export OBJCOPY=$ARMGCC_DIR/bin/arm-none-eabi-objcopy
	export CXX=$ARMGCC_DIR/bin/arm-none-eabi-g++
	export AR=$ARMGCC_DIR/bin/arm-none-eabi-ar
	export LD=$ARMGCC_DIR/bin/arm-none-eabi-ld

	mbed config GCC_ARM_PATH $ARMGCC_DIR/bin
}

spv_set_toolchain_FreeRTOS_ARMCC() {
	export ARMLMD_LICENSE_FILE=$QE_ROOT/devenv/licenses/arm_licenses.lic
	export ARMCC_DIR=/usr/local/DS-5_v5.25.0/ARMCompiler5.06u3
	echo "importing license"
	wget -O $QE_ROOT/devenv/licenses/arm_licenses.lic http://prov-jen-master.kfn.arm.com:8888/job/vivify_arm_license/lastSuccessfulBuild/artifact/arm_licenses_vivified.lic

}

spv_set_FreeRTOS_env() {
	rm -f .mbedignore
	echo "ADDSUBDIRS()" > mbed_client_cli/CMakeLists.txt
	python pal-platform/pal-platform.py deploy --target=K64F_FreeRTOS_mbedtls generate
	yes | cp -rf resources/MK64FN1M0xxx12-mbedOS.ld pal-platform/SDK/K64F_FreeRTOS/K64F_FreeRTOS/Device/MK64F/MK64FN1M0xxx12-mbedOS.ld
	cd __K64F_FreeRTOS_mbedtls
	
	if [ $PLATFORM_ARG = "K64F_FreeRTOS_GNUC" ]
	then
		spv_set_toolchain_FreeRTOS_GCC
		cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/ARMGCC/ARMGCC.cmake  -DEXTARNAL_DEFINE_FILE=../mbed-client-pal/Examples/PlatformBSP/mbedTLS/mbedTLS_cmake_config.txt
		spv_set_toolchain_arm_gcc_6
		cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/ARMGCC/ARMGCC.cmake -DEXTARNAL_DEFINE_FILE=../mbed-client-pal/Examples/PlatformBSP/mbedTLS/mbedTLS_cmake_config.txt
	elif [ $PLATFORM_ARG = "K64F_FreeRTOS_ARMCC" ]
	then
		spv_set_toolchain_FreeRTOS_ARMCC
		cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/ARMCC/ARMCC.cmake  -DEXTARNAL_DEFINE_FILE=../mbed-client-pal/Examples/PlatformBSP/mbedTLS/mbedTLS_cmake_config.txt
	fi
}

spv_set_Linux_env() {
	rm -f os_specific_source/.mbedignore	

	BUILD_TYPE=${1:-Debug}
	echo "current Directory:" ${PWD}

	# deploy repos
	python pal-platform/pal-platform.py -v deploy --target=x86_x64_NativeLinux_mbedtls generate
	# generate Makefiles
	python pal-platform/pal-platform.py -v generate --target=x86_x64_NativeLinux_mbedtls
	mkdir -p .build/linux
	echo "ADDSUBDIRS()" > mbed_client_cli/CMakeLists.txt
	# PATCH!!!! remove cmake file for pal client, this avoids issues with multiple unity versions
	rm -f mbed-client-pal/Test/CMakeLists.txt
	# all auto generated makefiles goes here
	cd __x86_x64_NativeLinux_mbedtls
	# generate CMake files
	echo "current Directory:" ${PWD}
	cmake -G 'Unix Makefiles' -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_TOOLCHAIN_FILE=../pal-platform/Toolchain/GCC/GCC.cmake
	
	rm -rf ../platform
	rm -rf ../mbed-cloud-client/mbed-client-pal/Test
	rm -rf ../mbed-cloud-client/factory-configurator-client/mbed-client-esfs/CMakeLists.txt
	rm -rf ../mbed-os
	python ../replace_line.py ../source/setup.h "extern int initPlatform();" "//extern int initPlatform();"
	cp ../pal-platform/Middleware/mbedtls/mbedtls/configs/config-suite-b.h ../pal-platform/Middleware/mbedtls/mbedtls/include/mbedtls/config.h
}

spv_set_MbedOS_env() {
	rm -rf security_util/obj security_util/lib
	rm -f os_specific_source/.mbedignore
	rm -rf __K64F_FreeRTOS_mbedtls
	rm -rf __x86_x64_NativeLinux_mbedtls
	echo "FreeRTOS/*" >> e2eIoT-test-device/qe-device-infra/os_specific_source/.mbedignore
    echo "Linux/*" >> e2eIoT-test-device/qe-device-infra/os_specific_source/.mbedignore
    sed -i -e 's/NSAPI_SECURITY_WPA_WPA2/NSAPI_SECURITY_NONE/g' ./easy-connect/easy-connect.h
}

case "$QE_PLAT" in
PC )
	spv_set_toolchain_native
	;;

K64F_MBEDOS_ARMCC )
	spv_set_MbedOS_env
	spv_set_toolchain_arm_ds5
	;;
K64F_MBEDOS_GNUC ) 
	spv_set_MbedOS_env
	spv_config_mbed_os
	spv_set_toolchain_arm_gcc_6
	;;
K64F_FreeRTOS_GNUC ) 
	spv_set_FreeRTOS_env
	;;
K64F_FreeRTOS_ARMCC )
	spv_set_FreeRTOS_env
	;;
PC_Linux_GNUC )
	spv_set_Linux_env
;;
UBLOX_EVK_ODIN_W2_MBEDOS_GNUC ) 
	spv_set_MbedOS_env
	spv_config_mbed_os
	spv_set_toolchain_arm_gcc_6
	;;
*)
	echo "Unknown platform: \"$QE_PLAT\". Known platforms: $QE_PLATS"
	return 1
	;;
esac

if [[ "$PLAT" == "K64F" ]]; then
	mbed config root .
	# Enable mbed-os local PROTOCOL
	mbed config PROTOCOL ssh
fi
if [[ "$PLAT" == "UBLOX_EVK_ODIN_W2" ]]; then
	mbed config root .
	# Enable mbed-os local PROTOCOL
	mbed config PROTOCOL ssh
fi

echo "easy-connect/atmel-rf-driver/*
easy-connect/mcr20a-rf-driver/*
easy-connect/stm-spirit1-rf-driver/*
pal-platform/*
platform/linux/*
mbed-cloud-client/mbed-client-pal/Source/Port/Reference-Impl/OS_Specific/mbedOS/FileSystem/pal_plat_fileSystem.cpp
platform/mbed-os/TARGET_UBLOX_EVK_ODIN_W2/pal_insecure_ROT.cpp
mbed-os/features/mbedtls/*" > .mbedignore

# JAVA 8 is needed for all targets -
# (Placed here to make sure this is first in the PATH).
export PATH=/usr/lib/jvm/java-8-openjdk-amd64/bin:$PATH
export JAVA_HOME=/usr/lib/jvm/java-8-openjdk-amd64

