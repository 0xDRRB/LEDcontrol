#include "bt_chat.h"
#include "bt_mgr.h"
#include "bt_selector.h"

static void _socket_conn_state_changed_cb(int result, bt_socket_connection_state_e connection_state, bt_socket_connection_s *connection, void *user_data)
{
	appdata_s *ad = NULL;

	ad = (appdata_s *) user_data;
	ret_if(!ad);
	ret_if(result != BT_ERROR_NONE);

	_D("[_socket_conn_state_changed_cb] Changed");
	if (connection_state == BT_SOCKET_DISCONNECTED) {
		_I("[_socket_conn_state_changed_cb] Disconnected");
		ad->socket_fd = -1;
		notification_status_message_post("LEDcontrol: Disconnected");
	}
}

static void _socket_data_received_cb(bt_socket_received_data_s *data, void *user_data)
{
	char *message = NULL;

	ret_if(!data);

	message = strndup(data->data, data->data_size);
	goto_if(!message, ERROR);

		free(message);

	return;

ERROR:
	return;
}

static void
colorselector_cb(void *data, Evas_Object *obj, void *event_info)
{
   int r, g, b, a;
   int ret = 0;
   //const char *message = NULL;

   appdata_s *ad = (appdata_s *) data;
   ret_if(!ad);

   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;

   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   evas_color_argb_premul(a, &r, &g, &b);
   _D("Color changed to (RGBA): %d %d %d %d", r, g, b, a);

   /*
   ret = bt_socket_send_data(ad->socket_fd, message, strlen(message) + 1);
   if (ret == -1) {
   		_E("[bt_socket_send_data] Fail to send: %s", message);
   		notification_status_message_post("LEDcontrol: Send failed");
   }
   */
}

HAPI void bt_selector_layout_create(appdata_s *ad)
{
	int ret = -1;

	ret_if(!ad);
	ret_if(!ad->navi);

	Evas_Object *colorselector;
	Elm_Object_Item *it;
	Eina_List *color_list;

	colorselector = elm_colorselector_add(ad->navi);
	//elm_colorselector_mode_set(colorselector, ELM_COLORSELECTOR_PALETTE);
	elm_colorselector_mode_set(colorselector, ELM_COLORSELECTOR_ALL);
	evas_object_size_hint_fill_set(colorselector, EVAS_HINT_FILL, EVAS_HINT_FILL);

	color_list = elm_colorselector_palette_items_get(colorselector);
	it = eina_list_nth(color_list, 0);

	elm_object_item_signal_emit(it, "elm,state,selected", "elm");
	evas_object_smart_callback_add(colorselector, "color,item,selected", colorselector_cb, ad);

	bt_socket_set_data_received_cb(_socket_data_received_cb, NULL);

	ret = bt_socket_unset_connection_state_changed_cb();
	ret_if(ret != BT_ERROR_NONE);

	ret = bt_socket_set_connection_state_changed_cb(_socket_conn_state_changed_cb, ad);
	ret_if(ret != BT_ERROR_NONE);

	elm_naviframe_item_push(ad->navi, "Choose color", NULL, NULL, colorselector, NULL);
}
