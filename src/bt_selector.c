#include "bt_main.h"
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

static void send_color_bt(appdata_s *ad, int r, int g, int b)
{
   int ret = 0;
   char message[6] = {0, 0, 0, 0, 0, 59};

   //appdata_s *ad = (appdata_s *) data;
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

static void _on_knob_moved(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
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

   // send color by bluetooth here
}

static void _on_knob_release(void *data, Evas_Object *o, const char *emission EINA_UNUSED, const char *source EINA_UNUSED)
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
   _D(">>>>>> value changed. hsv = %0.3f %0.3f %0.3f    rgb = %u %u %u", hue, sat, val, r, g, b);

   send_color_bt(ad, r, g, b);
}

HAPI void bt_selector_layout_create(appdata_s *ad)
{
	Evas_Object *edje_object;
	char edj_path[PATH_MAX] = { 0, };

	ret_if(!ad);
	ret_if(!ad->win);
	ret_if(!ad->navi);

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
	edje_object_signal_callback_add(edje_object, "mouse,up,*", "b_hue", _on_knob_release, ad);
	edje_object_signal_callback_add(edje_object, "drag", "b_hue", _on_knob_moved, ad);
	edje_object_signal_callback_add(edje_object, "mouse,up,*", "b_sat", _on_knob_release, ad);
	edje_object_signal_callback_add(edje_object, "drag", "b_sat", _on_knob_moved, ad);
	edje_object_signal_callback_add(edje_object, "mouse,up,*", "b_val", _on_knob_release, ad);
	edje_object_signal_callback_add(edje_object, "drag", "b_val", _on_knob_moved, ad);
	//elm_naviframe_item_simple_push(ad->naviframe, edje_object);

	// push objet on the naviframe stack
	elm_naviframe_item_push(ad->navi, "Choose color", NULL, NULL, edje_object, NULL);

	// TODO
	bt_socket_set_data_received_cb(_socket_data_received_cb, NULL);
}
