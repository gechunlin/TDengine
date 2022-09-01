/*
 * Copyright (c) 2019 TAOS Data, Inc. <jhtao@taosdata.com>
 *
 * This program is free software: you can use, redistribute, and/or modify
 * it under the terms of the GNU Affero General Public License, version 3
 * or later ("AGPL"), as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TDENGINE_TTHREAD_H
#define TDENGINE_TTHREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "os.h"
#include "taosdef.h"

// create new thread
pthread_t* taosCreateThread( void *(*__start_routine) (void *), void* param);
// destory thread 
bool taosDestroyThread(pthread_t* pthread);
// thread running return true
bool taosThreadRunning(pthread_t* pthread);

#ifdef __cplusplus
}
#endif

#endif  // TDENGINE_TTHREAD_H
