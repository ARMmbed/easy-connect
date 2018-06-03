/*
 * Copyright (c) 2015-2016 ARM Limited. All rights reserved.
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

#ifdef __linux__
#include <cstdio>
#endif // __linux__

#if defined(CLI_MODE) || defined(__MBED__)
#include "init_plat.h"
#include "cmd_unity.h"
#endif // defined(CLI_MODE) || defined(__MBED__)


#if defined(CLI_MODE) || defined(__MBED__)
extern void init_signals();
#endif // defined(CLI_MODE) || defined(__MBED__)

extern int setObj(int argc, char* argv[]);
extern int main_server(int argc, char* argv[]);
extern int kill_server(int argc, char* argv[]);

int trace_cnt_i=0;

#if defined(CLI_MODE) || defined(__MBED__)
#ifdef __cplusplus
extern "C"{
#endif

char* trace_cnt(size_t size)
{
  static char str[10] = {0};
  sprintf(str, "[%04d]", trace_cnt_i++);
  return str;
}

void serial_trace_printer(const char* str)
{
	stdout_mutex_wait();
    printf("%s\r\n", str);
    stdout_mutex_release();
    cmd_output();
}

#ifdef __cplusplus
}
#endif

int run_unity_test_group(int argc, char **argv)
{
	// dummy function
	return 0;
}

void cmd_ready_cb(int retcode)
{
	tr_debug("cmd_ready_cb(%d)", retcode);
	cmd_next(retcode);
}


void main_thread_cb(const void* arg)
{
	(void)arg;
	srand(time(NULL));
	mbed_trace_init();
	initPlatform(0, NULL);
  	cmd_init(NULL);
	cmd_set_ready_cb(cmd_ready_cb);
	cmd_add("start", main_server, "Run The Server", 0);
	cmd_add("conf", setObj, "set the value", 0);
	cmd_add("end", kill_server, "kill the server", 0);
#ifdef __linux__
	init_signals();
#endif
	wait_for_signal();
}


void pal_destroy_caller()
{
	pal_destroy();
}

#endif // defined(CLI_MODE) || defined(__MBED__)

int main(int argc, char* argv[])
{
#if defined(CLI_MODE) || defined(__MBED__)
	int ret = 0;
	palStatus_t pal_status;
#ifdef __MBED__
	static const uint32_t MAIN_THREAD_STACK_SIZE = 8 * 1024;
#else // __linux__
	static const uint32_t MAIN_THREAD_STACK_SIZE = 8 * 1024 * 16;
#endif
	static palThreadID_t main_thread_id;
	atexit(pal_destroy_caller);
	
	pal_status = pal_init();
	
	if (pal_status != PAL_SUCCESS)
	{
		//cmd_printf("Error initializing pal. status %lX\n",pal_status);
	}

	pal_status = pal_osThreadCreateWithAlloc(
		main_thread_cb,
		NULL,
		PAL_osPriorityNormal,
		MAIN_THREAD_STACK_SIZE,
		NULL,
		&main_thread_id);

	wait_for_commands();

#else // linux non-test mode
	int ret = main_server(argc, argv);

	char ch = getchar();
#endif // defined(CLI_MODE) || defined(__MBED__)

	return ret;
}

