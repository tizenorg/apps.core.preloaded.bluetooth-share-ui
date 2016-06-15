/*
* bluetooth-share-ui
*
* Copyright 2012 Samsung Electronics Co., Ltd
*
* Contact: Hocheol Seo <hocheol.seo@samsung.com>
*           GirishAshok Joshi <girish.joshi@samsung.com>
*           DoHyun Pyun <dh79.pyun@samsung.com>
*
* Licensed under the Flora License, Version 1.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.tizenopensource.org/license
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include <stdio.h>
#include <app.h>
#include <app_control.h>
#include <app_control_internal.h>
#if 0
#include <Ecore_X.h>
#include <utilX.h>
#endif
#include <vconf.h>
#include <vconf-keys.h>
#include <E_DBus.h>
#if 0
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#endif
#include <aul.h>
#include <efl_extension.h>
#include "applog.h"
#include "bt-share-ui-main.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include <bluetooth-share-api.h>
#include <bluetooth.h>
#include <bundle_internal.h>

bt_share_appdata_t *app_state = NULL;
bt_share_appdata_t app_data = {0,};

static void __bt_lang_changed_cb(app_event_info_h event_info, void *data)
{
	DBG("+");
	ret_if(data == NULL);

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;

	if (ad->tr_genlist) {
		elm_genlist_realized_items_update(ad->tr_genlist);
	}

	if (ad->navi_it) {
		elm_object_item_text_set(ad->navi_it,
					(ad->tr_type == BT_TR_INBOUND) ?
					BT_STR_RECEIVED_FILES :
					BT_STR_SENT_FILES);
		Evas_Object *nocontents = elm_object_item_part_content_get(
					ad->navi_it,
					"elm.swallow.content");
		elm_object_part_text_set(nocontents, "elm.text",
					BT_STR_NO_TRANSFER_HISTORY);
	}

	if (ad->ft_popup) {
		_bt_lang_changed_ft_popup(ad);
	}

	if (ad->progress_popup) {
		_bt_lang_changed_progress_popup(ad);
	}
}

static int __bt_parse_launch_mode(bt_share_appdata_t *ad, bundle *b)
{
	const char *launch_type = NULL;

	retvm_if(ad == NULL, -1, "Invalid param");
	retvm_if(b == NULL, -1, "Invalid param");

	launch_type = bundle_get_val(b, "launch-type");
	retv_if(!launch_type, -1);

	if (!strcasecmp(launch_type, "ongoing")) {
		ad->launch_mode = BT_LAUNCH_ONGOING;
	} else if (!strcasecmp(launch_type, "warning_popup")) {
		ad->launch_mode = BT_LAUNCH_WARNING_POPUP;
	} else if (!strcasecmp(launch_type, "transfer_list")) {
		ad->launch_mode = BT_LAUNCH_TRANSFER_LIST;
	} else {
		ERR("Invalid bundle value");
		return -1;
	}
	return 0;
}

static int __bt_share_launch_handler(bt_share_appdata_t *ad, bundle *b)
{
	INFO("+");
	bt_share_popup_data_t *pb_data = NULL;
	const char *str = NULL;
	const char *transfer_type = NULL;
	const char *temp = NULL;
	int tr_type = 0;
	sqlite3 *db = NULL;
	bt_adapter_state_e bt_state = BT_ADAPTER_DISABLED;

	retvm_if(ad == NULL, -1, "Invalid param");
	retvm_if(b == NULL, -1, "Invalid param");

	if (ad->launch_mode == BT_LAUNCH_ONGOING) {
		INFO("BT_LAUNCH_ONGOING");

		/*Check adapter state, if disabled then return */
		if (bt_adapter_get_state(&bt_state) == BT_ERROR_NONE) {
			if (bt_state == BT_ADAPTER_DISABLED) {
				DBG("Adapter disabled, returning");
				return 0;
			}
		} else
			ERR("bt_adapter_get_state() failed");



		if (ad->tr_view == NULL) {
			Evas_Object *e = NULL;
			Ecore_Evas *ee = NULL;
			e = evas_object_evas_get(ad->win);
			ee = ecore_evas_ecore_evas_get(e);
			ecore_evas_name_class_set(ee,"APP_POPUP","APP_POPUP");
		}

		pb_data = g_new0(bt_share_popup_data_t, 1);
		pb_data->filename = g_strdup(bundle_get_val(b, "filename"));
		pb_data->percentage = g_strdup(bundle_get_val(b, "percentage"));
		pb_data->transfer_type = g_strdup(
				bundle_get_val(b, "transfer_type"));
		temp = bundle_get_val(b, "transfer_id");
		if (temp != NULL)
			pb_data->transfer_id = atoi(temp);
		if (g_strcmp0(pb_data->transfer_type, "outbound") == 0) {
			pb_data->progress_cnt = g_strdup(bundle_get_val(b, "progress_cnt"));
			INFO("progress cnt : %s", pb_data->progress_cnt);
			ad->ft_type = BT_FT_SENT_POPUP;
		}

		INFO_SECURE("filename %s", pb_data->filename);
		INFO("percentage %s", pb_data->percentage);
		INFO("transfer_type %s", pb_data->transfer_type);
		INFO("transfer_id %d", pb_data->transfer_id);
		/* Notificaton id of ongoing item */
		if (ad->progress_popup)
			_bt_destroy_progress_popup(ad);
		ad->progress_popup = _bt_create_progress_popup(pb_data);
		retvm_if (ad->progress_popup == NULL, -1, "fail to create progress popup");

		evas_object_data_set(ad->progress_popup, "progressbar_data", pb_data);
	} else if (ad->launch_mode == BT_LAUNCH_WARNING_POPUP) {
		INFO("BT_LAUNCH_WARNING_POPUP");
		if (ad->progress_popup)
			_bt_destroy_progress_popup(ad);
		str = bundle_get_val(b, "message");
		_bt_create_extinctive_info_popup(ad, BT_STR_INFORMATION, str);
	} else if (ad->launch_mode == BT_LAUNCH_TRANSFER_LIST) {
		INFO("BT_LAUNCH_TRANSFER_LIST");
		elm_win_indicator_mode_set(ad->win, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(ad->win, ELM_WIN_INDICATOR_OPAQUE);

		if (ad->progress_popup)
			_bt_destroy_progress_popup(ad);
		transfer_type = bundle_get_val(b, "transfer_type");
		retvm_if (!transfer_type, -1, "Invalid transfer type!");

		if (!strcmp(transfer_type, "outbound"))
			tr_type = BT_TR_OUTBOUND;
		else
			tr_type = BT_TR_INBOUND;

		retvm_if(ad->tr_view && ad->tr_type == tr_type, 0,
			"same view. no need to create transfer view!");

		INFO("Transfer type : %s", transfer_type);
		ad->tr_type = tr_type;

		/* Get data from share DB */
		db = bt_share_open_db();
		retvm_if(!db, -1, "fail to open db!");

		ad->tr_data_list = bt_share_get_completed_tr_data_list(db, ad->tr_type);
		bt_share_close_db(db);

		if (ad->tr_data_list)
			_bt_create_transfer_view(ad);
		else
			_bt_nocontent_set(ad, TRUE);

	} else {
		ERR("Invalid bundle value");
		return -1;
	}

	return 0;
}

static bool __app_create(void *data)
{
	INFO(" __app_create");
	bt_share_appdata_t *ad = data;
	Evas_Object *win = NULL;
	int ret;
	bt_adapter_state_e status = BT_ADAPTER_DISABLED;

	elm_app_base_scale_set(1.8);

	/* create window */
	win = _bt_create_win(PACKAGE);
	retv_if(win == NULL, false);
	ad->win = win;

	bindtextdomain(BT_COMMON_PKG, BT_COMMON_RES);

	if (bt_initialize() != BT_ERROR_NONE)
		ERR("bt_initialize() failed");

	if (bt_adapter_get_state(&status) != BT_ERROR_NONE)
		ERR("bt_adapter_get_state() failed!");

	ad->bt_status = status;

	/* Set event callbacks */
	ret =
	    bt_adapter_set_state_changed_cb(_bt_cb_state_changed, (void *)ad);
	if (ret != BT_ERROR_NONE)
		ERR("bt_adapter_set_state_changed_cb failed");
	return true;
}

static void __app_service(app_control_h app_control, void *user_data)
{
	INFO("__app_service");
	bt_share_appdata_t *ad = user_data;
	int ret;
	bundle *b = NULL;
	ret_if(ad == NULL);

	ret = app_control_export_as_bundle(app_control, &b);

	if (ad->dbus_conn == NULL) {
		_bt_signal_init(ad);
	}

	if(__bt_parse_launch_mode(ad, b) == -1) {
		if (b)
			bundle_free(b);
		return;
	}

	if (ad->launch_mode == BT_LAUNCH_ONGOING ||
		ad->launch_mode == BT_LAUNCH_WARNING_POPUP) {
		if (ad->tr_view == NULL) {
			if (elm_win_alpha_get(ad->win) == FALSE) {
				elm_win_alpha_set(ad->win, TRUE);
				int ret = 0;
#if 0
				Display *dpy;
				Window win;

				dpy = ecore_x_display_get();
				win = elm_win_xwindow_get(ad->win);

				ret = utilx_set_window_opaque_state(dpy,
						win, UTILX_OPAQUE_STATE_ON);
#endif

				if (!ret)
				{
					ERR("Error! Failed to set opaque state.");
				}
			}
		}
	} else if (elm_win_alpha_get(ad->win))
		elm_win_alpha_set(ad->win, FALSE);


	/* Set rotation Callback */
	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	ret = __bt_share_launch_handler(ad, b);
	if (ret < 0)
		_bt_terminate_app();


	evas_object_show(ad->win);
	elm_win_activate(ad->win);
	bundle_free(b);
}

static void __app_terminate(void *data)
{
	INFO("__app_terminate ");
	bt_share_appdata_t *ad = data;
	int err;

	_bt_destroy_info_popup(ad);
	_bt_signal_deinit(ad);

	if (ad->tr_data_itc) {
		elm_genlist_item_class_free(ad->tr_data_itc);
		ad->tr_data_itc = NULL;
	}

	if (ad->win)
		evas_object_del(ad->win);

	err = bt_adapter_unset_state_changed_cb();
	if (err != BT_ERROR_NONE)
		ERR("unset of state change cb  failed: %d", err);

	err = bt_deinitialize();
	if (err != BT_ERROR_NONE)
		ERR("bt_deinitialize failed: %d", err);


}

static void __app_pause(void *data)
{
	INFO("__app_pause ");
}

static void __app_resume(void *data)
{
	INFO("__app_resume");
}

EXPORT int main(int argc, char *argv[])
{
	DBG("Start bluetooth-share-ui main()");

	ui_app_lifecycle_callback_s callback = {0,};
	app_event_handler_h lang_changed_handler;
	app_state = &app_data;

	callback.create = __app_create;
	callback.terminate = __app_terminate;
	callback.pause = __app_pause;
	callback.resume = __app_resume;
	callback.app_control = __app_service;

	ui_app_add_event_handler(&lang_changed_handler, APP_EVENT_LANGUAGE_CHANGED, __bt_lang_changed_cb, &app_data);

	DBG("ui_app_main() is called.");
	int ret = ui_app_main(argc, argv, &callback, &app_data);
	if (ret != APP_ERROR_NONE) {
		ERR("ui_app_main() is failed. err = %d", ret);
	}

	DBG("End bluetooth-share-ui main()");
	return ret;
}

