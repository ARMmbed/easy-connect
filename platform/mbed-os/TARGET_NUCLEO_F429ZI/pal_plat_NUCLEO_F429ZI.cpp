/*
 * Copyright (c) 2016 ARM Limited. All rights reserved.
 * SPDX-License-Identifier: Apache-2.0
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#include "pal_plat_rtos.h"

//THIS CODE IS FOR TESTING PURPOSES ONLY. DO NOT USE IN PRODUCTION ENVIRONMENTS. REPLACE WITH A PROPER IMPLEMENTATION BEFORE USE
palStatus_t  __attribute__((weak)) pal_plat_osGetRoT128Bit(uint8_t *keyBuf, size_t keyLenBytes)

{
    #warning message ("NUCLEO F429ZI doesn't have secure Root of Trust, add secure implementation by yourself.")
    return PAL_SUCCESS;
}

