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
#else // __MBED__
#include "../../e2eIoT-test-device/qe-device-infra/os-specific-source/mbedOS/init_plat.h"
#define TRACE_GROUP "main"
#endif

extern int main_server(int argc, char* argv[]);

#ifdef __linux__
int main(int argc, char* argv[])
{
	int ret = main_server(argc, argv);

	//the server run in seperate thread
	getchar();

	return ret;
}

#else // MBED

int trace_cnt_i=0;

extern void initialize_app_commands();
int main_server(int argc, char* argv[]);

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
	cmd_add("server", main_server, "Run The Server", 0);
	wait_for_signal();
}

int main(void)
{
	palStatus_t pal_status;
	static const uint32_t MAIN_THREAD_STACK_SIZE = 8 * 1024;
	static palThreadID_t main_thread_id;

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

	return 0;
}
#endif