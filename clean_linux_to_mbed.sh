
#!/bin/bash

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


set -x

function rm_dir()
{
	if [ -d "$1" ]; then
			rm -rf $1
	fi
} 

function clean_dirs()
{
	cd tls
	rm_dir mbedtls
	rm_dir arch
	cd -
	rm_dir arch
	cd GuruxDLMS/development
	rm_dir arch
	cd ../..
	cd security_util
	rm_dir arch
	cd -
}

clean_dirs
