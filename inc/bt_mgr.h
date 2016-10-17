/*
 * Samsung API
 * Copyright (c) 2009-2015 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the License);
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/license/
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __BT_MGR_H__
#define __BT_MGR_H__

#define BT_MGR_UUID "00001101-0000-1000-8000-00805F9B34FB"

#include <bluetooth.h>
#include <app_control.h>
#include <glib.h>
#include <stdlib.h>

typedef enum {
	BT_MGR_SEARCH = 0x00,
	BT_MGR_WAIT,
} bt_mgr_type;

void bt_mgr_initialize(void *data, bt_mgr_type type);
void bt_mgr_release(void);

#endif /* __BT_MGR_H__ */
