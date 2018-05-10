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

#ifndef _SETUP_H
#define _SETUP_H

#include <stdint.h>
#include "m2minterface.h"

#ifdef __cplusplus
extern "C" {
#endif

// Interval to update resource value in ms
#define INCREMENT_INTERVAL 25000

typedef void (*main_t)(void);

// Initialize platform
// This function initializes screen and any other non-network
// related platform specific initializations required.
//
// @returns
//   0 for success, anything else for error
extern int initPlatform();

// Initialize network connection
extern bool init_connection();

// Returns network interface.
extern void *get_network_interface();

// Print text on the screen
extern void print_to_screen(int x, int y, const char* buffer);

// Clear screen
extern void clear_screen();

// Thread for updating resource value
extern void increment_resource_thread(void* client);

// Print heap allocations
extern void print_heap_stats();

// Print m2mobject sizes
extern void print_m2mobject_stats();

// Create set of objects to test size
extern void create_m2mobject_test_set(M2MObjectList *object_list);

// Wait
extern void do_wait(int timeout_in_sec);

extern bool runProgram(main_t mainFunc);

#ifdef __cplusplus
}
#endif

#endif
