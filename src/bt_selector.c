#include "bt_chat.h"
#include "bt_mgr.h"
#include "bt_selector.h"

// for tomorow...
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
   char message[6] = {0, 0, 0, 0, 0, 59};

   appdata_s *ad = (appdata_s *) data;
   ret_if(!ad);

   Elm_Object_Item *color_it = (Elm_Object_Item *) event_info;

   elm_colorselector_palette_item_color_get(color_it, &r, &g, &b, &a);
   evas_color_argb_premul(a, &r, &g, &b);
   _D("Color changed to (RGBA): %d %d %d %d", r, g, b, a);

   // format : "00rgb;"
   message[2]=r;
   message[3]=g;
   message[4]=b;

   ret = bt_socket_send_data(ad->socket_fd, message, 6);
   if (ret == -1) {
   		_E("[bt_socket_send_data] Fail to send");
   		notification_status_message_post("LEDcontrol: Send failed");
   } else {
	    _D("[bt_socket_send_data] Send OK (data= %u %u %u %u %u %c)",
			   message[0], message[1], message[2], message[3], message[4], message[5]);
   }
}

static void _on_colorselector_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	// cleanup other objects here
	_D("Colorselector deleted");
}

HAPI void bt_selector_layout_create(appdata_s *ad)
{
	ret_if(!ad);
	ret_if(!ad->navi);

	Evas_Object *colorselector;
	Elm_Object_Item *it;
	const Eina_List *color_list;

	colorselector = elm_colorselector_add(ad->navi);
	elm_colorselector_mode_set(colorselector, ELM_COLORSELECTOR_PALETTE);
	//elm_colorselector_mode_set(colorselector, ELM_COLORSELECTOR_ALL);
	evas_object_size_hint_fill_set(colorselector, EVAS_HINT_FILL, EVAS_HINT_FILL);

	color_list = elm_colorselector_palette_items_get(colorselector);
	it = eina_list_nth(color_list, 0);

	// no default selection, because this send "white" to the device
	//elm_object_item_signal_emit(it, "elm,state,selected", "elm");
	evas_object_smart_callback_add(colorselector, "color,item,selected", colorselector_cb, ad);

	bt_socket_set_data_received_cb(_socket_data_received_cb, NULL);

	evas_object_event_callback_add(colorselector, EVAS_CALLBACK_DEL, _on_colorselector_del_cb, NULL);
	elm_naviframe_item_push(ad->navi, "Choose color", NULL, NULL, colorselector, NULL);
}
