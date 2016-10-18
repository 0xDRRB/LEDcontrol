#ifndef __BLUETOOTH_CHAT_H__
#define __BLUETOOTH_CHAT_H__

#include <app.h>
#include <tizen.h>
#include <Elementary.h>
#include <system_settings.h>
#include <bluetooth_type.h>
#include <efl_extension.h>
#include <dlog.h>
#include <stdbool.h>
#include <notification.h>

#include "log.h"

#if !defined(PACKAGE)
#define PACKAGE "$(packageName)"
#endif

#define EDJ_FILE "edje/bt_chat.edj"
#define EDJ_WAIT "edje/bt_wait.edj"
#define GRP_MAIN "main"

#define HAPI __attribute__((visibility("hidden")))

typedef struct appdata{
	Evas_Object* win;
	Evas_Object* navi;
	Evas_Object* conform;

	bool bt;
	int socket_fd;
	bt_socket_role_e role;
} appdata_s;

typedef enum {
	CS_MAIN = 0x00,
	CS_PREPARE,
	CS_CHAT,
} current_state_e;

void app_resource_get(const char *edj_file_in, char *edj_path_out, int edj_path_max);
void cr_layout(appdata_s *ad);

#endif /* __BLUETOOTH_CHAT_H__ */
