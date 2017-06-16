#include "bt_main.h"
#include "bt_mgr.h"
#include "bt_selector.h"

static char *items[] = {
		"STATIC",
		"BLINK",
		"BREATH",
		"COLOR WIPE",
		"COLOR WIPE RANDOM",
		"RANDOM COLOR",
		"SINGLE DYNAMIC",
		"MULTI DYNAMIC",
		"RAINBOW",
		"RAINBOW CYCLE",
		"SCAN",
		"DUAL SCAN",
		"FADE",
		"THEATER CHASE",
		"THEATER CHASE_RAINBOW",
		"RUNNING LIGHTS",
		"TWINKLE",
		"TWINKLE RANDOM",
		"TWINKLE FADE",
		"TWINKLE FADE_RANDOM",
		"SPARKLE",
		"FLASH SPARKLE",
		"HYPER SPARKLE",
		"STROBE",
		"STROBE RAINBOW",
		"MULTI STROBE",
		"BLINK RAINBOW",
		"CHASE WHITE",
		"CHASE COLOR",
		"CHASE RANDOM",
		"CHASE RAINBOW",
		"CHASE FLASH",
		"CHASE FLASH RANDOM",
		"CHASE RAINBOW WHITE",
		"CHASE BLACKOUT",
		"CHASE BLACKOUT RAINBOW",
		"COLOR SWEEP RANDOM",
		"RUNNING RED BLUE",
		"RUNNING RANDOM",
		"LARSON SCANNER",
		"COMET",
		"FIREWORKS",
		"FIREWORKS RANDOM"
};

// i don't like this
int effect = 14;

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

static void send_color_bt(appdata_s *ad, int r, int g, int b)
{
   int ret = 0;
   //                 0  0  r  g  b  ;
   char message[6] = {0, 0, 0, 0, 0, 59};

   ret_if(!ad);

   _D("Color changed to (RGB): %u %u %u", r, g, b);

   // format : "00rgb;"
   message[2]=r;
   message[3]=g;
   message[4]=b;

   ret = bt_socket_send_data(ad->socket_fd, message, 6);
   if (ret == -1) {
   		_E("[bt_socket_send_data] Fail to send");
   		notification_status_message_post("LEDcontrol: Send failed");
   } else {
	    _D("Send OK (data= %u %u %u %u %u %c)",
			   message[0], message[1], message[2], message[3], message[4], message[5]);
   }
}

static void send_effect_bt(appdata_s *ad)
{
   int ret = 0;
   //                 0  1  e  -  -  ;
   char message[6] = {0, 1, 0, 0, 0, 59};

   ret_if(!ad);

   _D("Effet changed to : %u", effect);

   // format : "01e00;"
   message[2]=effect;

   ret = bt_socket_send_data(ad->socket_fd, message, 6);
   if (ret == -1) {
   		_E("[bt_socket_send_data] Fail to send");
   		notification_status_message_post("LEDcontrol: Send failed");
   } else {
	    _D("Send OK (data= %u %u %u %u %u %c)",
			   message[0], message[1], message[2], message[3], message[4], message[5]);
   }
}

void hsv2rgb(double h, double s, double v, int *r, int *g, int *b)
{
    double      hh, p, q, t, ff;
    long        i;

    if(s <= 0.0) {
        *r = v*255; *g = v*255; *b = v*255;
        return;
    }

    hh = h;
    if(hh >= 360.0) hh = 0.0;
    hh /= 60.0;
    i = (long)hh;
    ff = hh - i;
    p = v * (1.0 - s);
    q = v * (1.0 - (s * ff));
    t = v * (1.0 - (s * (1.0 - ff)));

    switch(i) {
      case 0:
        *r = v*255; *g = t*255; *b = p*255;
        break;
      case 1:
        *r = q*255; *g = v*255; *b = p*255;
        break;
      case 2:
        *r = p*255; *g = v*255; *b = t*255;
        break;
      case 3:
        *r = p*255; *g = q*255; *b = v*255;
        break;
      case 4:
        *r = t*255; *g = p*255; *b = v*255;
        break;
      case 5:
      default:
        *r = v*255; *g = p*255; *b = q*255;
        break;
    }
    return;
}

static void _on_knob_moved_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   double hue, val, sat ;
   int r, g, b;

   appdata_s *ad = (appdata_s *) data;
   ret_if(!ad);

   // get dragable position
   edje_object_part_drag_value_get(o, "b_hue", NULL, &hue);
   hue = hue*360;
   edje_object_part_drag_value_get(o, "b_sat", NULL, &sat);
   sat = 1 - sat;
   edje_object_part_drag_value_get(o, "b_val", NULL, &val);
   val = 1 - val;

   // set value slider color
   hsv2rgb(hue, sat, 1.0, &r, &g, &b);
   edje_color_class_set("valcolor", r, g, b, 255,     255, 0, 0, 255, 255, 0, 0, 255);

   // set saturation slider colors
   hsv2rgb(hue, 1.0, val, &r, &g, &b);
   edje_color_class_set("basecolor", r, g, b, 255,     255, 0, 0, 255, 255, 0, 0, 255);
   hsv2rgb(hue, 0.0, val, &r, &g, &b);
   edje_color_class_set("satcolor", r, g, b, 255,     255, 0, 0, 255, 255, 0, 0, 255);

   // set final color RECT
   hsv2rgb(hue, sat, val, &r, &g, &b);
   edje_color_class_set("finalcolor", r, g, b, 255,     255, 0, 0, 255, 255, 0, 0, 255);

   // send color by bluetooth only when release, no here
}

static void _on_knob_release_cb(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
{
   double hue, val, sat ;
   int r, g, b;

   appdata_s *ad = (appdata_s *) data;
   ret_if(!ad);

   edje_object_part_drag_value_get(o, "b_hue", NULL, &hue);
   //_D("value changed to: %0.3f", val);
   hue = hue*360;

   edje_object_part_drag_value_get(o, "b_sat", NULL, &sat);
   sat = 1 - sat;

   edje_object_part_drag_value_get(o, "b_val", NULL, &val);
   val = 1 - val;

   hsv2rgb(hue, sat, val, &r, &g, &b);

   // send color by bluetooth here
   _D("value changed. hsv = %0.3f %0.3f %0.3f    rgb = %u %u %u", hue, sat, val, r, g, b);

   send_color_bt(ad, r, g, b);
}

/************ Popup stuff ************/

// tap outside popup handling
static void popup_block_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

// radio button select callback
static void radio_changed_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *) user_data;
	int index = elm_radio_value_get(obj);
	_D("Index = : %u     Effet = : %u", index, effect);
	send_effect_bt(ad);
}

// Create menu key popup
static void _layout_menu_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *) user_data;

	Evas_Object *popup;
	Evas_Object *radio;
	Evas_Object *radio_group;
	Evas_Object *box;
	int i;

	popup = elm_popup_add(ad->win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_smart_callback_add(popup, "block,clicked", popup_block_clicked_cb, ad->win);

	/* box */
	box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	radio = elm_radio_add(box);
	elm_object_text_set(radio, items[0]);
	evas_object_show(radio);
	elm_box_pack_end(box, radio);

	/* assigning a unique value(within the group) to the radio instance */
	elm_radio_state_value_set(radio, 0);
	evas_object_smart_callback_add(radio, "changed", radio_changed_cb, ad);

	/* creating a radio group with first radio */
	radio_group = radio;

	for(i = 1; i < sizeof(items)/sizeof(items[0]); i++) {
		radio = elm_radio_add(box);
		elm_object_text_set(radio, items[i]);
		evas_object_show(radio);
		elm_box_pack_end(box, radio);
		elm_radio_state_value_set(radio, i);
		elm_radio_group_add(radio, radio_group);
		evas_object_smart_callback_add(radio, "changed", radio_changed_cb, ad);
	}

	// Set a convenience pointer to a integer to change when radio group value changes.
	elm_radio_value_pointer_set(radio_group, &effect);

	// default effect
	elm_radio_value_set(radio_group, effect);

	evas_object_size_hint_min_set(box, -1, 400);
	elm_object_content_set(popup, box);
	evas_object_show(popup);
}

static void _nf_back_cb(void *user_data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *) user_data;

	// need to disconnet BT at the same time we pop
	// so we are disconnecter when devices list show up again
    bt_socket_disconnect_rfcomm(ad->socket_fd);

    // Pop an item that is on top of the stack
    elm_naviframe_item_pop(ad->navi);
}

/************ Edje color selector GUI stuff ************/
HAPI void bt_selector_layout_create(appdata_s *ad)
{
	Evas_Object *edje_object;
	char edj_path[PATH_MAX] = { 0, };

	ret_if(!ad);
	ret_if(!ad->win);
	ret_if(!ad->navi);
	//Elm_Object_Item *nf_item;

	// Create the content
	// find path
	app_resource_get("edje/colorpicker.edj", edj_path, (int)PATH_MAX);
	// create objet
	edje_object = edje_object_add(evas_object_evas_get(ad->win));
	// load edje data
	edje_object_file_set(edje_object, edj_path, "main");
	// adjust hint
	evas_object_size_hint_weight_set(edje_object, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	// set callbacks fnct on dragables part
	edje_object_signal_callback_add(edje_object, "mouse,up,*", "b_*", _on_knob_release_cb, ad);
	edje_object_signal_callback_add(edje_object, "drag", "b_*", _on_knob_moved_cb, ad);

	// push objet on the naviframe stack
	elm_naviframe_item_push(ad->navi, "Choose color", NULL, NULL, edje_object, NULL);

	// TODO (request/response handling for getting defined/FX color from Arduino)
	bt_socket_set_data_received_cb(_socket_data_received_cb, NULL);

	/* Register the Menu key event callback */
	eext_object_event_callback_add(edje_object, EEXT_CALLBACK_MORE, _layout_menu_cb, ad);
	// Z3 Tizen 2.4.0.3 bug ? back key not working anymore
	// Z2 Tizen 2.4.0.5 works fine :-/
	// so we need to use this :
	eext_object_event_callback_add(edje_object, EEXT_CALLBACK_BACK, _nf_back_cb, ad);

}
