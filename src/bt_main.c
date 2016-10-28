#include <bluetooth.h>

#include "bt_main.h"
#include "bt_mgr.h"

static void popup_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = data;
	evas_object_del(popup);
}

static void _about_btn_clicked_cb(void *data)
{
	Evas_Object *popup;
	Evas_Object *btn;

	appdata_s *ad = (appdata_s *) data;
	ret_if(!ad);

	// popup
	popup = elm_popup_add(ad->win);
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 1.0);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, ad->win);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_part_text_set(popup, "title,text", "LEDcontrol by Denis Bodor");
	elm_object_text_set(popup,
			"This app talk with an Arduino board using a serial Bluetooth module like "
			"HC-05 or HC-06. Each time you choose a color this app send a Bluetooth message "
			"and the Arduino set the led color accordingly.</br></br>"
			"Here is a sample Arduino sketch with a single ws2812b led connected to pin 9 "
			"and Bluetooth module connected to 10 and 11:<br><br>"
			"<font_size=25>"
			"#include &lt;<color=#F47742FF>SoftwareSerial</color>.h&gt;</br>"
			"#include &lt;<color=#F47742FF>Adafruit_NeoPixel</color>.h&gt;</br>"
			"</br>"
			"<color=#F47742FF>Adafruit_NeoPixel</color> ws = </br>"
			"     <color=#F47742FF>Adafruit_NeoPixel</color>(1, 9, NEO_RGB + NEO_KHZ800);</br>"
			"<color=#F47742FF>SoftwareSerial</color> btmodule(10, 11);</br>"
			"</br>"
			"<color=#42B9F4FF>void</color> setup() {</br>"
			"  <color=#F47742FF>Serial.begin</color>(115200);</br>"
			"  while (!<color=#F47742FF>Serial</color>);</br>"
			"  btmodule.<color=#F47742FF>begin</color>(19200);</br>"
			"  ws.<color=#F47742FF>begin</color>();</br>"
			"  ws.<color=#F47742FF>setPixelColor</color>(0, ws.<color=#F47742FF>Color</color>(0,0,0));</br>"
			"  ws.show();</br>"
			"}</br>"
			"</br>"
			"<color=#42B9F4FF>void</color> loop() {</br>"
			"  if (btmodule.<color=#F47742FF>available</color>()) {</br>"
			"    <color=#42B9F4FF>String</color> commande = btmodule.<color=#F47742FF>readStringUntil</color>(';');</br>"
			"    if(commande.<color=#F47742FF>length</color>() == 5) {</br>"
			"      if(commande.<color=#F47742FF>charAt</color>(1) == 0x00) {</br>"
			"        ws.<color=#F47742FF>setPixelColor</color>(0, ws.Color(commande.<color=#F47742FF>charAt</color>(2),</br>"
			"                    commande.<color=#F47742FF>charAt</color>(3),commande.<color=#F47742FF>charAt</color>(4)));</br>"
			"        ws.show();</br>"
			"      }</br>"
			"    }</br>"
			"  }</br>"
			"}"
			"</font_size>"
			"</br></br>"
			"You can get the source code of this native app on:</br>"
			"<font_size=30><b>https://github.com/Lefinnois/LEDcontrol</b></font_size>"
			"<br></br>"
			"<font_size=25>LEDcontrol version 0.0.2, Copyright (C) 2016 Denis Bodor</br>"
    		"LEDcontrol comes with ABSOLUTELY NO WARRANTY; "
    		"This program is free software; you can redistribute it and/or modify "
    		"it under the terms of the GNU General Public License version 2.</font_size>");



	// ok button
	btn = elm_button_add(popup);
	elm_object_style_set(btn, "popup");
	elm_object_text_set(btn, "OK");
	elm_object_part_content_set(popup, "button1", btn);
	evas_object_smart_callback_add(btn, "clicked", popup_btn_clicked_cb, popup);

	evas_object_show(popup);
}

static void _search_btn_clicked_cb(void *data)
{
	ret_if(!data);

	bt_mgr_initialize(data);
}

static void _win_delete_request_cb(void *data, Evas_Object *obj, void *event_info)
{
	ui_app_exit();
}

static void _layout_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	appdata_s *ad = (appdata_s *) data;
	int ret = 0;

	ret_if(!ad);

	if (elm_naviframe_top_item_get(ad->navi) == elm_naviframe_bottom_item_get(ad->navi)) {
		if (ad) {
			elm_win_lower(ad->win);
		}
	} else {
		_D("POP");
		elm_naviframe_item_pop(ad->navi);
		if (ad->socket_fd != -1) {
			ret = bt_socket_disconnect_rfcomm(ad->socket_fd);
			if (ret != BT_ERROR_NONE) {
				_E("[bt_socket_disconnect_rfcomm] Failed");
			} else {
				ad->socket_fd = -1;
			}
		}
	}
}

HAPI void app_resource_get(const char *edj_file_in, char *edj_path_out, int edj_path_max)
{
	char *res_path = app_get_resource_path();
	if (res_path) {
		snprintf(edj_path_out, edj_path_max, "%s%s", res_path, edj_file_in);
		free(res_path);
	}
}

static void _destroy_base_gui(appdata_s *ad)
{
	Evas_Object *bg = NULL;

	ret_if(!ad);
	ret_if(!ad->conform);

	bg = elm_object_part_content_get(ad->conform, "elm.swallow.indicator_bg");
	if (bg) {
		evas_object_del(bg);
	}

	evas_object_del(ad->navi);
	evas_object_del(ad->conform);
	evas_object_del(ad->win);
}

static void _create_base_gui(appdata_s *ad)
{
	Evas_Object *bg = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *search_btn = NULL;
	Evas_Object *about_btn = NULL;
	char edj_path[PATH_MAX] = { 0, };

	ad->socket_fd = -1;
	ad->role = BT_SOCKET_UNKNOWN;

	/* Window */
	ad->win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	elm_win_conformant_set(ad->win, EINA_TRUE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *) (&rots), 4);
	}

	evas_object_smart_callback_add(ad->win, "delete,request", _win_delete_request_cb, NULL);

	/* Conformant */
	ad->conform = elm_conformant_add(ad->win);
	goto_if(!ad->conform, ERROR);
	elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
	elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);
	evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, ad->conform);
	evas_object_show(ad->conform);

	/* Indicator BG */
	bg = elm_bg_add(ad->conform);
	goto_if(!bg, ERROR);
	elm_object_style_set(bg, "indicator/headerbg");
	elm_object_part_content_set(ad->conform, "elm.swallow.indicator_bg", bg);
	evas_object_show(bg);

	/* Naviframe */
	ad->navi = elm_naviframe_add(ad->conform);
	goto_if(!ad->navi, ERROR);
	evas_object_size_hint_weight_set(ad->navi, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_content_set(ad->conform, ad->navi);
	evas_object_show(ad->navi);
	eext_object_event_callback_add(ad->navi, EEXT_CALLBACK_BACK, _layout_back_cb, ad);

	/* Base Layout */
	app_resource_get(EDJ_FILE, edj_path, (int) PATH_MAX);
	layout = elm_layout_add(ad->navi);
	goto_if(!layout, ERROR);
	elm_layout_file_set(layout, edj_path, GRP_MAIN);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	/* Set buttons to Swallow */
	search_btn = elm_button_add(layout);
	goto_if(!search_btn, ERROR);
	elm_object_text_set(search_btn, "Choose Device");
	evas_object_smart_callback_add(search_btn, "clicked", (Evas_Smart_Cb) _search_btn_clicked_cb, (void *) ad);
	elm_object_part_content_set(layout, "search", search_btn);

	about_btn = elm_button_add(layout);
	goto_if(!about_btn, ERROR);
	elm_object_text_set(about_btn, "About");
	evas_object_smart_callback_add(about_btn, "clicked", (Evas_Smart_Cb) _about_btn_clicked_cb, (void *) ad);
	elm_object_part_content_set(layout, "about", about_btn);


	/* Push Main Layout to Naviframe */
	elm_naviframe_item_push(ad->navi, "LEDcontrol", NULL, NULL, layout, NULL);

	/* Show window after base gui is set up */
	evas_object_show(ad->win);

	return;

ERROR:
	if (bg) {
		evas_object_del(bg);
		bg = NULL;
	}

	if (layout) {
		evas_object_del(layout);
		layout = NULL;
	}

	if (search_btn) {
		evas_object_del(search_btn);
		search_btn = NULL;
	}

	return;
}

static bool _app_create(void *data)
{
	/* Hook to take necessary actions before main event loop starts
	   Initialize UI resources and application's data
	   If this function returns true, the main loop of application starts
	   If this function returns false, the application is terminated */
	appdata_s *ad = (appdata_s *) data;
	retv_if(!ad, false);

	_create_base_gui(ad);

	return true;
}

static void _app_control(app_control_h app_control, void *data)
{
	/* Handle the launch request. */
}

static void _app_pause(void *data)
{
	/* Take necessary actions when application becomes invisible. */
}

static void _app_resume(void *data)
{
	_D("RESUMED");
}

static void _app_terminate(void *data)
{
	/* Release all resources. */
	appdata_s *ad = (appdata_s *) data;
	int ret;

	ret_if(!ad);

	_D("APP TERMINATED");
	if (ad->socket_fd != -1) {
		ret = bt_socket_disconnect_rfcomm(ad->socket_fd);
		if (ret != BT_ERROR_NONE) {
			_E("[bt_socket_disconnect_rfcomm] Failed");
		} else {
			ad->socket_fd = -1;
		}
	}
	bt_mgr_release();

	_destroy_base_gui(ad);
}

static void _ui_app_lang_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_LANGUAGE_CHANGED */
	char *locale = NULL;
	system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);

	if (locale) {
		elm_language_set(locale);
		free(locale);
	}

	return;
}

static void _ui_app_orient_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_DEVICE_ORIENTATION_CHANGED */
	return;
}

static void _ui_app_region_changed(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_REGION_FORMAT_CHANGED */
}

static void _ui_app_low_battery(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_LOW_BATTERY */
}

static void _ui_app_low_memory(app_event_info_h event_info, void *user_data)
{
	/* APP_EVENT_LOW_MEMORY */
}

int main(int argc, char *argv[])
{
	appdata_s ad = { 0, };
	int ret = 0;

	ui_app_lifecycle_callback_s event_callback = { 0, };
	app_event_handler_h handlers[5] = { NULL, };

	event_callback.create = _app_create;
	event_callback.terminate = _app_terminate;
	event_callback.pause = _app_pause;
	event_callback.resume = _app_resume;
	event_callback.app_control = _app_control;

	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_BATTERY], APP_EVENT_LOW_BATTERY, _ui_app_low_battery, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LOW_MEMORY], APP_EVENT_LOW_MEMORY, _ui_app_low_memory, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_DEVICE_ORIENTATION_CHANGED], APP_EVENT_DEVICE_ORIENTATION_CHANGED, _ui_app_orient_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_LANGUAGE_CHANGED], APP_EVENT_LANGUAGE_CHANGED, _ui_app_lang_changed, &ad);
	ui_app_add_event_handler(&handlers[APP_EVENT_REGION_FORMAT_CHANGED], APP_EVENT_REGION_FORMAT_CHANGED, _ui_app_region_changed, &ad);
	ui_app_remove_event_handler(handlers[APP_EVENT_LOW_MEMORY]);

	ret = ui_app_main(argc, argv, &event_callback, &ad);
	if (ret != APP_ERROR_NONE) {
		_E("ui_app_main() is failed. err = %d", ret);
	}

	return ret;
}
