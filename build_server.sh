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

CLEAN="$1"

if [[ "$CLEAN" == "-c" ]]; then
	rm -rf GuruxDLMS/development/obj GuruxDLMS/development/lib ./obj ./bin security_util/lib security_util/obj
else
	cd mbedtls
	make -j10
	cd ../security_util
	
	if [ ! -d "obj" ]; then
	mkdir obj
	fi
	if [ ! -d "lib" ]; then
	mkdir lib
	fi
	
	make -j10
	cd ../GuruxDLMS/development

	if [ ! -d "obj" ]; then
	mkdir obj
	fi
	if [ ! -d "lib" ]; then
	mkdir lib
	fi

	make -j10
	cd ../../

	if [ ! -d "obj" ]; then
	mkdir obj
	fi
	if [ ! -d "bin" ]; then
	mkdir bin
	fi

	make -j10
fi