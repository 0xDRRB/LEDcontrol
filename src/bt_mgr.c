#include "bt_main.h"
#include "bt_mgr.h"
#include "bt_selector.h"

#define MAX_NUM_PENDING 1

static struct _s_info {
	bt_device_info_s *info;
	Evas_Object *list;
	Evas_Object *noti;
	Evas_Object *navi;
} s_info = {
	.info = NULL,
	.noti = NULL,
	.list = NULL,
	.navi = NULL,
};

static void _socket_conn_state_changed_cb(int result, bt_socket_connection_state_e connection_state, bt_socket_connection_s *connection, void *user_data)
{
	appdata_s *ad = (appdata_s *) user_data;
	ret_if(!ad);

	ret_if(result != BT_ERROR_NONE);

	if (connection_state == BT_SOCKET_CONNECTED) {
		if (connection != NULL) {
			notification_status_message_post("LEDcontrol: Connected");
			_D("Connected %d %d", ad->socket_fd, connection->socket_fd);
			ad->role = connection->local_role;
			ad->socket_fd = connection->socket_fd;
			elm_naviframe_item_pop(ad->navi);
			bt_selector_layout_create(ad);
			if (s_info.noti) {
				evas_object_del(s_info.noti);
				s_info.noti = NULL;
			}
		} else {
			_D("No connection data");
		}
	} else if (connection_state == BT_SOCKET_DISCONNECTED) {
		_D("Disconnected");
		ad->socket_fd = -1;
		notification_status_message_post("LEDcontrol: Disconnected");
	}
}

static void _connect_layout_create()
{
	Evas_Object *layout = NULL;
	Evas_Object *progress = NULL;
	char edj_path[PATH_MAX] = { 0, };

	app_resource_get(EDJ_WAIT, edj_path, (int) PATH_MAX);
	layout = elm_layout_add(s_info.navi);
	goto_if(!layout, ERROR);
	elm_layout_file_set(layout, edj_path, "wait_screen");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progress = elm_progressbar_add(layout);
	goto_if(!progress, ERROR);
	elm_object_style_set(progress, "process_large");
	evas_object_size_hint_min_set(progress, 100, 100);
	evas_object_size_hint_align_set(progress, 0.5, 0.5);
	evas_object_size_hint_weight_set(progress, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_pulse_set(progress, EINA_TRUE);
	elm_progressbar_pulse(progress, EINA_TRUE);

	elm_object_part_content_set(layout, "progress", progress);

	elm_naviframe_item_push(s_info.navi, "Connecting to device", NULL, NULL, layout, NULL);

	return;

ERROR:
	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	if (progress) {
		evas_object_del(progress);
		progress = NULL;
	}

	return;
}

static void _click_device_item_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = NULL;
	bt_device_info_s *info = NULL;
	bt_error_e ret = BT_ERROR_NONE;

	ret_if(!obj);

	info = (bt_device_info_s *) data;
	ret_if(!info);
	s_info.info = info;

	item = elm_genlist_selected_item_get(obj);
	ret_if(!item);

	elm_genlist_item_selected_set(item, EINA_FALSE);

	ret = bt_socket_connect_rfcomm(info->remote_address, BT_MGR_UUID);
	if (ret != BT_ERROR_NONE) {
		_E("[bt_socket_listen_and_accept_rfcomm] Failed");
	}

	_connect_layout_create();
}

static char* _genlist_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	bt_device_info_s *device_info = (bt_device_info_s *) data;

	if (!strcmp("elm.text", part))
		return strdup(device_info->remote_name);
	else if (!strcmp("elm.text.sub", part))
		return strdup(device_info->remote_address);
	else return NULL;
}

static bool _list_bonded_cb(bt_device_info_s *device_info, void *user_data) {
	bt_device_info_s *new_device_info = NULL;

	Elm_Genlist_Item_Class *itc = (Elm_Genlist_Item_Class *) user_data;
	if(!itc) return FALSE;

	if(!s_info.list) return FALSE;

	if (device_info != NULL && s_info.list != NULL) {
		new_device_info = malloc(sizeof(bt_device_info_s));
		if (new_device_info != NULL) {
			_D("Device Name is: %s", device_info->remote_name);

			memcpy(new_device_info, device_info, sizeof(bt_device_info_s));
			new_device_info->remote_address = strdup(device_info->remote_address);
			new_device_info->remote_name = strdup(device_info->remote_name);

			elm_genlist_item_append(s_info.list,
					itc,  // item class
					new_device_info,   // item class user data
					NULL,
					ELM_GENLIST_ITEM_NONE, // item type
					_click_device_item_cb, // select smart callback
					new_device_info);  // smart callback user data
		}
	}

	return TRUE;
}

static void _search_layout_create(appdata_s *ad)
{
	int ret = 0;

	ret_if(!ad);
	ret_if(!ad->navi);

	ad->role = BT_SOCKET_CLIENT;

	s_info.navi = ad->navi;

	Elm_Genlist_Item_Class *itc = NULL;
	itc = elm_genlist_item_class_new();
	itc->item_style = "type1";
	itc->func.content_get = NULL;
	itc->func.text_get = _genlist_text_get_cb;  // CB text + part, text or subtext
	s_info.list = elm_genlist_add(ad->navi);
	ret_if(!s_info.list);

	evas_object_size_hint_weight_set(s_info.list, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(s_info.list, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_naviframe_item_push(ad->navi, "Paired Device(s)", NULL, NULL, s_info.list, NULL);

	ret = bt_socket_set_connection_state_changed_cb(_socket_conn_state_changed_cb, ad);
	ret_if(ret != BT_ERROR_NONE);

	bt_adapter_foreach_bonded_device(_list_bonded_cb, itc);
}

static void _onoff_operation(void)
{
	int ret = 0;
	app_control_h service = NULL;

	app_control_create(&service);
	ret_if(!service);

	app_control_set_operation(service, "http://tizen.org/appcontrol/operation/edit");
	app_control_set_mime(service, "application/x-bluetooth-on-off");
	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		_E("Failed to relaunch Bluetooth On/off app");
	}

	app_control_destroy(service);
}

HAPI void bt_mgr_initialize(void *data, bt_mgr_type type)
{
	appdata_s *ad = NULL;
	bt_adapter_state_e bt_ad_state = BT_ADAPTER_DISABLED;
	int ret = 0;

	ad = (appdata_s *) data;
	ret_if(!ad);

	ret = bt_initialize();
	ret_if(ret != BT_ERROR_NONE);

	ret = bt_adapter_get_state(&bt_ad_state);
	ret_if(ret != BT_ERROR_NONE);

	if (bt_ad_state == BT_ADAPTER_DISABLED) {
		_onoff_operation();
	} else {
		if (type==BT_MGR_SEARCH) {
			_search_layout_create(ad);
		}
	}
}

HAPI void bt_mgr_release(void)
{
	bt_adapter_unset_state_changed_cb();
	bt_adapter_unset_device_discovery_state_changed_cb();
	bt_device_unset_service_searched_cb();
	bt_socket_unset_data_received_cb();
	bt_socket_unset_connection_state_changed_cb();
	bt_deinitialize();
}
