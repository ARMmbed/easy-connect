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

#CLEAN="$1"
bit_type=64

function make_dir()
{
        if [ ! -d "$1" ]; then
                mkdir -p $1
        fi
}
function compile_all()
{
	cd mbedtls/x86/${bit_type}
	make -j10
	cd -
	##############################
	cd security_util

	make_dir x86/${bit_type}/obj
	make_dir x86/${bit_type}/lib

	make -j10 BIT_TYPE=${bit_type}
	cd -
	##############################
	cd GuruxDLMS/development

	make_dir x86/${bit_type}/obj
	make_dir x86/${bit_type}/lib

	make -j10 BIT_TYPE=${bit_type}
	cd -
	##############################

	make_dir x86/${bit_type}/obj
	make_dir x86/${bit_type}/bin

	make -j10 BIT_TYPE=${bit_type}
}

function clone_mbedtls()
{
	if [ ! -d "mbedtls/x86/${bit_type}" ]; then
		mkdir -p mbedtls/x86/${bit_type}
		mkdir -p mbedtls/include

		git clone -b mbedtls-2.7 git@github.com:ARMmbed/mbedtls.git mbedtls/x86/${bit_type}

		cp  mbedtls/x86/${bit_type}/configs/config-suite-b.h \
				mbedtls/x86/${bit_type}/include/mbedtls/config.h

		cp  -dpRf mbedtls/x86/${bit_type}/include/mbedtls  \
                                mbedtls
	fi
}

function clean_artifacts()
{
	rm -rf GuruxDLMS/development/x86/${bit_type}
        rm -rf x86/${bit_type}
        rm -rf security_util/x86/${bit_type}
}

while getopts ":b:c" o; do
        case "${o}" in
	b)
		if [ "${OPTARG}" = "64" ]
		then
			bit_type=64
		elif [ "${OPTARG}" = "32" ]
		then
			bit_type=32
		else
			echo "incorrect bit type (${OPTARG}) , should be 64 or 32"
			exit 1
			fi
			;;
	c)
		clean_artifacts
		;;
        esac
done

clone_mbedtls
compile_all
