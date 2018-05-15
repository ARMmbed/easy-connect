//----------------------------------------------------------------------------
// The confidential and proprietary information contained in this file may
// only be used by a person authorised under and to the extent permitted
// by a subsisting licensing agreement from ARM Limited or its affiliates.
//
// (C) COPYRIGHT 2016 ARM Limited or its affiliates.
// ALL RIGHTS RESERVED
//
// This entire notice must be reproduced on all copies of this file
// and copies of this file may only be made by a person if such person is
// permitted to do so under the terms of a subsisting license agreement
// from ARM Limited or its affiliates.
//----------------------------------------------------------------------------

#ifndef _COMP_DEFINES_
#define _COMP_DEFINES_

#ifdef __linux__
#define STATUS int
#define SOCKET int
#define THREAD_ID pthread_t // unsigned int
#define SOCKADDR struct sockaddr
#define SOCKLEN unsigned int
#define SUCCESS 0
#define NETADD_INFO struct sockaddr_in
#define SEMAPHORE sem_t
#else // MBED
#define STATUS palStatus_t // int
#define SOCKET palSocket_t // void*
#define THREAD_ID palThreadID_t // unsigned int
#define SOCKADDR palSocketAddress_t
#define SOCKLEN palSocketLength_t // unsigned int
#define SUCCESS PAL_SUCCESS // = 0
#define NETADD_INFO palNetInterfaceInfo_t
#define SEMAPHORE palSemaphoreID_t
#endif

#endif //_COMP_DEFINES_
