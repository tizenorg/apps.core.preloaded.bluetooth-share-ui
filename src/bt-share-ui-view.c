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

#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#if 0
#include <Ecore_X.h>
#include <utilX.h>
#endif
#include <Elementary.h>
#include <efl_extension.h>
#include <aul.h>
#include <app.h>
#include <vconf.h>
#include <vconf-keys.h>
#include <bluetooth-share-api.h>
#include <bluetooth-api.h>
#include <notification.h>
#if 0
#include <app_extension.h>
#endif
#include <notification_list.h>
#include <notification_internal.h>

#include "applog.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"


#define BT_TIMESTAMP_LEN_MAX 18
#define BT_DOWNLOAD_PHONE_FOLDER "/opt/usr/media/Downloads"
#define BT_DOWNLOAD_MMC_FOLDER "/opt/storage/sdcard/Downloads"
#define BT_FILE_PATH_LEN_MAX	(4096 + 10)

extern bt_share_appdata_t *app_state;
static char *month_str[] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

static void __bt_more_menu_cb(void *data,
				Evas_Object *obj, void *event_info);

Evas_Object *_bt_create_win(const char *name)
{
	Evas_Object *eo = NULL;

#if 0
	eo = (Evas_Object *)app_get_preinitialized_window(name);
#endif
	if (eo == NULL) {
		ERR("app_get_preinitialized_window fail!");
		eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	}
	retv_if(!eo, NULL);

	elm_win_title_set(eo, name);
	elm_win_borderless_set(eo, EINA_TRUE);
#if 0
	ecore_x_window_size_get(ecore_x_window_root_first_get(), &w, &h);
	evas_object_resize(eo, w, h);
#endif

	return eo;
}

void _bt_terminate_app(void)
{
	DBG("Terminate BT SHARE UI");
	elm_exit();
}

void _bt_delete_selected_item(bt_share_appdata_t *ad)
{
	DBG("+");
	ret_if(ad == NULL);

/* "Bluetooth share has closed unexpectedly" crash occurring.
     After press OK in receive failed pop up.
     TMWC-726
*/
#if 0
	if (ad->selected_item == NULL)
		return;

	elm_object_item_del(ad->selected_item);
	ad->selected_item = NULL;
#endif

	if (elm_genlist_items_count(ad->tr_genlist) == 0) {
		_bt_nocontent_set(ad, TRUE);
		ad->tr_genlist = NULL;
	}
}

static void __bt_clear_view(void *data)
{
	DBG("+");

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	ret_if(!ad);

	if (ad->tr_genlist) {
		elm_genlist_clear(ad->tr_genlist);
		ad->tr_genlist = NULL;
	}

	if (ad->toolbar_btn) {
		evas_object_del(ad->toolbar_btn);
		ad->toolbar_btn = NULL;
	}

	if (ad->navi_fr) {
		evas_object_del(ad->navi_fr);
		ad->navi_fr = NULL;
	}

	if (ad->tr_view) {
		evas_object_del(ad->tr_view);
		ad->tr_view = NULL;
	}
	DBG("-");
}

static Eina_Bool __bt_back_button_cb(void *data, Elm_Object_Item *it)
{
	DBG("pop current view ");
	retvm_if(!data, EINA_FALSE, "invalid parameter!");

	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	int ret;

	__bt_clear_view(data);

	if (ad->tr_data_list) {
		ret = bt_share_release_tr_data_list(ad->tr_data_list);

		if (ret != BT_SHARE_ERR_NONE)
			ERR("Transfer data release failed ");
		ad->tr_data_list = NULL;
	}

	_bt_terminate_app();

	return EINA_FALSE;
}

static void __bt_clear_list_btn_cb(void *data,
					Evas_Object *obj,
					void *event_info)
{
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	int clear_list = -1;
	int ret;

	DBG("Clear genlist item");

	notification_h noti = NULL;
	int priv_id = 0;
	notification_list_h list_head = NULL;
	notification_list_h list_traverse = NULL;
	char *app_id = NULL;
	char *opp_role = NULL;

	if (ad->bt_status == BT_ADAPTER_DISABLED) {
		sqlite3 *db = NULL;
		bt_tr_db_table_e table;

		/* Update bt-share DB */
		db = bt_share_open_db();
		if (!db)
			return;

		table = ad->tr_type ? BT_TR_INBOUND : BT_TR_OUTBOUND;
		bt_share_remove_all_tr_data(db, table);
		bt_share_close_db(db);

		/* Delete notification */
		notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list_head);
		list_traverse = list_head;

		if (ad->tr_type == BT_TR_INBOUND)
			opp_role = "bluetooth-share-opp-server";
		else
			opp_role = "bluetooth-share-opp-client";

		while (list_traverse != NULL) {
			noti = notification_list_get_data(list_traverse);
			notification_get_pkgname(noti, &app_id);

			if (g_strcmp0(app_id, opp_role) == 0) {
				notification_get_id(noti, NULL, &priv_id);
				notification_delete_by_priv_id(app_id, NOTIFICATION_TYPE_NOTI, priv_id);
				priv_id = 0;
			}
			list_traverse = notification_list_get_next(list_traverse);
		}

		if (list_head != NULL) {
			notification_free_list(list_head);
			list_head = NULL;
		}
	} else {
		_bt_share_ui_ipc_info_update(ad, clear_list);
	}

	if (ad->tr_genlist) {
		elm_genlist_clear(ad->tr_genlist);
		ad->tr_genlist = NULL;
	}

	if (ad->tr_data_list) {
		ret = bt_share_release_tr_data_list(ad->tr_data_list);
		if (ret != BT_SHARE_ERR_NONE)
			ERR("Transfer data release failed ");
		ad->tr_data_list = NULL;
	}

	_bt_nocontent_set(ad, TRUE);

}

static char *__bt_get_tr_timedate(time_t timestamp)
{
	struct tm *pt;
	struct tm current_time;
	time_t rawtime;
	char buf[BT_TIMESTAMP_LEN_MAX] = { 0 };
	int cy;
	int cm;
	int cd;
	int ry;
	int rm;
	int rd;

	/* Get current time */
	time(&rawtime);
	localtime_r(&rawtime, &current_time);

	cy = current_time.tm_year + 1900;
	cm = current_time.tm_mon + 1;
	cd = current_time.tm_mday;

	/* Get recorded time */
	pt = localtime(&timestamp);
	retv_if(pt == NULL, NULL);
	ry = pt->tm_year + 1900;
	rm = pt->tm_mon + 1;
	rd = pt->tm_mday;

	if (cy == ry && cm == rm && cd == rd) {
		int format = 0;
		if (vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &format) != 0) {
			ERR("vconf get failed");
		}

		if (format == VCONFKEY_TIME_FORMAT_12)
			if (pt->tm_hour >=0 && pt->tm_hour < 12)
				strftime(buf, sizeof(buf), "%I:%M AM", pt);
			else
				strftime(buf, sizeof(buf), "%I:%M PM", pt);
		else
			strftime(buf, sizeof(buf), "%H:%M", pt);
	} else if (cy == ry && cm == rm && cd - 1 == rd) {
		return g_strdup(BT_STR_YESTERDAY);
	} else {
		snprintf(buf, sizeof(buf), "%d %s %d", rd, month_str[rm - 1], ry);
	}

	return g_strdup(buf);
}

static Evas_Object *__bt_tr_icon_get(void *data, Evas_Object *obj,
					    const char *part)
{
	Evas_Object *ly = NULL;
	Evas_Object *icon = NULL;
	bt_tr_data_t *info = NULL;
	bt_share_appdata_t *ad = app_state;
	char *img;
	bt_gl_data_t *gl_data;

	retv_if(data == NULL, NULL);

	gl_data = (bt_gl_data_t *)data;

	info = gl_data->tr_data;
	retv_if(info == NULL, NULL);

	if (!strcmp("elm.swallow.icon", part)) {
		ly = elm_layout_add(obj);
		elm_layout_theme_set(ly, "layout", "list/B/type.3", "default");

		icon = elm_icon_add(obj);
		if (ad->tr_type == BT_TR_OUTBOUND) {
			img = (info->tr_status == BT_TR_SUCCESS) ?
					BT_ICON_SEND_PASS : BT_ICON_SEND_FAIL;
		} else {
			img = (info->tr_status == BT_TR_SUCCESS) ?
					BT_ICON_RECV_PASS : BT_ICON_RECV_FAIL;
		}

		elm_image_file_set(icon, EDJ_IMAGES, img);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		evas_object_color_set(icon, 76, 76, 76 , 255);

		evas_object_show(icon);
		elm_layout_content_set(ly, "elm.swallow.content", icon);
	}

	return ly;
}

static char *__bt_tr_label_get(void *data, Evas_Object *obj,
				      const char *part)
{
	bt_tr_data_t *info = NULL;
	char *name = NULL;
	char buf[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	bt_gl_data_t *gl_data;

	retv_if(data == NULL, NULL);
	gl_data = (bt_gl_data_t *)data;

	info = gl_data->tr_data;
	retv_if(info == NULL, NULL);

	if (!strcmp("elm.text", part)) {
		name = strrchr(info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = info->file_path;

		g_strlcpy(buf, name, BT_GLOBALIZATION_STR_LENGTH);
	} else if (!strcmp("elm.text.sub", part)) {
		char *marked_name = elm_entry_utf8_to_markup(info->dev_name);
		g_strlcpy(buf, marked_name, BT_GLOBALIZATION_STR_LENGTH);
		if (marked_name)
			free(marked_name);
	} else if (!strcmp("elm.text.sub.end", part)) {
		char *date = NULL;
		date = __bt_get_tr_timedate((time_t)(info->timestamp));
		snprintf(buf, BT_GLOBALIZATION_STR_LENGTH, "<font_size=25>%s</font_size>", date);
		g_free(date);
	} else {
		DBG("empty text for label.");
		return NULL;
	}

	return strdup(buf);
}

static void  __bt_tr_del(void *data, Evas_Object *obj)
{
	bt_gl_data_t *gl_data;
	gl_data = (bt_gl_data_t *)data;

	g_free(gl_data);
}

static void __bt_genlist_realized_cb(void *data,
			Evas_Object *obj, void *event_info)
{
	retm_if(event_info == NULL, "Invalid param\n");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
#ifdef KIRAN_ACCESSIBILITY
	Evas_Object *ao;
	char str[BT_STR_ACCES_INFO_MAX_LEN] = {0, };
#endif
	char *name;
	char *date;
	bt_tr_data_t *info;
	bt_gl_data_t *gl_data;

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);

	info = gl_data->tr_data;

	ret_if(info == NULL);
	ret_if(info->dev_name == NULL);
	ret_if(info->file_path == NULL);

	name = strrchr(info->file_path, '/');
	if (name != NULL)
		name++;
	else
		name = info->file_path;

	date = __bt_get_tr_timedate((time_t)(info->timestamp));

#ifdef KIRAN_ACCESSIBILITY
	snprintf(str, sizeof(str), "%s, %s, %s, %s",
			BT_STR_ACC_ICON, name, info->dev_name, date);
	ao = elm_object_item_access_object_get(item);
	elm_access_info_set(ao, ELM_ACCESS_INFO, str);
#endif
	g_free(date);
}

static void __bt_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DBG("+");
	retm_if(data == NULL, "data is NULL");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;

	evas_object_del(ad->info_popup);
	ad->info_popup = NULL;
	DBG("-");
}

void __bt_popup_del_by_timeout(void *data, Evas_Object *obj, void *event_info)
{
	DBG("+");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	ret_if(!ad);
	if (ad->info_popup){
		evas_object_del(ad->info_popup);
		ad->info_popup = NULL;
	}
	DBG("-");
}

Evas_Object *__bt_create_error_popup(bt_share_appdata_t *ad)
{
	DBG("+");
	Evas_Object *popup = NULL;
	retvm_if(ad == NULL, NULL, "ad is NULL");

	popup = elm_popup_add(ad->win);
	retvm_if(popup == NULL, NULL, "popup is NULL");

	elm_object_focus_set(popup, EINA_FALSE);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(popup, BT_STR_UNABLE_TO_FIND_APPLICATION);

	elm_popup_timeout_set(popup, 3);
	evas_object_smart_callback_add(popup, "timeout", (Evas_Smart_Cb)__bt_popup_del_by_timeout, ad);
	evas_object_show(popup);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, __bt_popup_del_cb, ad);

	ad->info_popup = popup;

	DBG("-");
	return popup;
}

static gboolean __bt_open_received_file(const char *path)
{
	DBG("+");
	app_control_h handle;
	int ret;
	bt_share_appdata_t *ad = app_state;

	app_control_create(&handle);
	app_control_set_operation(handle, APP_CONTROL_OPERATION_VIEW);
	app_control_set_uri(handle, path);
	ret = app_control_send_launch_request(handle, NULL, NULL);

	if (ret == APP_CONTROL_ERROR_APP_NOT_FOUND)
		__bt_create_error_popup(ad);

	app_control_destroy(handle);
	DBG("-");
	return TRUE;
}

static gint __handle_compare(gconstpointer a, gconstpointer b)
{
	bt_tr_data_t *info = (bt_tr_data_t *)a;
	int id = (int) b;

	if (info->id == id)
		return 0;
	else
		return -1;
}

static void __bt_remove_tr_data_node(bt_share_appdata_t *ad, int id)
{
	DBG("+");
	GSList *l = NULL;
	bt_tr_data_t *info = NULL;
	l = g_slist_find_custom(ad->tr_data_list, (gconstpointer)id,
							__handle_compare);
	if (l) {
		info = l->data;
		ad->tr_data_list = g_slist_remove(ad->tr_data_list, info);
	}

	if (g_slist_length(ad->tr_data_list) == 0) {
		_bt_nocontent_set(ad, TRUE);
		ad->tr_genlist = NULL;
	}
}

void _bt_update_tr_notification(void *data)
{
	ret_if(data == NULL);

	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = NULL;
	sqlite3 *db = NULL;
	int success = 0;
	int fail = 0;
	char *opp_role = NULL;
	notification_h noti = NULL;
	int priv_id = 0;
	notification_list_h list_head = NULL;
	notification_list_h list_traverse = NULL;
	char *app_id = NULL;
	char *stms_str = NULL;

	info = (bt_tr_data_t *)data;

	DBG("Transfer type: %s", ad->tr_type == BT_TR_INBOUND ? "Receive" : "Sent");

	db = bt_share_open_db();
	if (!db)
		return;

	if (ad->tr_type == BT_TR_INBOUND)
		opp_role = "bluetooth-share-opp-server";
	else
		opp_role = "bluetooth-share-opp-client";


	/* Delete selected outbound db / notification info */
	if (bt_share_remove_tr_data_by_id(db, ad->tr_type, info->id) == 0)
		DBG("successfully TR ID removed from DB");
#if 0
	bt_share_get_tr_result_count(db, ad->tr_type, &success, &fail);
#endif
	DBG("success: %d, fail: %d", success, fail);

	/* Delete notification */
	notification_get_list(NOTIFICATION_TYPE_NOTI, -1, &list_head);
	list_traverse = list_head;

	while (list_traverse != NULL) {
		noti = notification_list_get_data(list_traverse);
		notification_get_pkgname(noti, &app_id);

		if (g_strcmp0(app_id, opp_role) == 0) {
			notification_get_id(noti, NULL, &priv_id);
			break;
		}
		list_traverse = notification_list_get_next(list_traverse);
	}

	if (success == 0 && fail == 0)
		notification_delete_by_priv_id(app_id, NOTIFICATION_TYPE_NOTI, priv_id);
#if 0
	else {
		char str[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
		if (success == 1) {
			stms_str = BT_STR_TR_1FILE_COPIED_STATUS;
			snprintf(str, sizeof(str), stms_str, fail);
		} else {
			stms_str = BT_STR_TR_COPIED_STATUS;
			snprintf(str, sizeof(str), stms_str,
				 success, fail);
		}

		notification_set_content(noti, str, NULL);
		notification_update(noti);
	}
#endif
	if (list_head != NULL) {
		notification_free_list(list_head);
		list_head = NULL;
	}

	bt_share_close_db(db);
}

static void __bt_tr_data_recv_item_sel(void *data, Evas_Object *obj,
				      void *event_info)
{
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = NULL;
	char *path = NULL;
	char *ext = NULL;
	int default_memory = 0;

	DBG("Select received item");

	ret_if(data == NULL);
	ret_if(event_info == NULL);

	info = (bt_tr_data_t *)data;
	__bt_remove_tr_data_node(ad, info->id);

	ad->selected_item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info,
				      EINA_FALSE);

	if (info->tr_status == BT_TR_SUCCESS) {
		ret_if(info->file_path == NULL);
		INFO_SECURE("File : %s", info->file_path);
		if (vconf_get_int(VCONFKEY_SETAPPL_DEFAULT_MEM_BLUETOOTH_INT,
							&default_memory) != 0) {
			ERR("vconf get failed");
		}
		path = info->file_path;
		INFO_SECURE("path : %s", path);
		ext = strrchr(path, '.');
		if (ext) {
			ext++;
			if (0 == g_strcmp0(ext, "vcf")) {
				ad->ft_popup = _bt_create_ft_popup(
						BT_FT_RECV_SUCCESS_POPUP, info);
				return;
			}
		}

		if (access(path, F_OK) == 0) {
			__bt_open_received_file(path);
			if (ad->bt_status == BT_ADAPTER_DISABLED) {
				_bt_update_tr_notification(info);
				_bt_delete_selected_item(ad);
			} else { /*Adapter enabled case */
				_bt_share_ui_ipc_info_update(ad, info->id);
				_bt_delete_selected_item(ad);
			}
		} else {
			ad->ft_popup = _bt_create_ft_popup(BT_FT_FILE_NOT_EXIST,
									info);
			_bt_share_ui_ipc_info_update(ad, info->id);
			_bt_delete_selected_item(ad);
		}

	} else {
		ad->ft_popup = _bt_create_ft_popup(BT_FT_RECV_FAIL_POPUP, info);
	}

	return;
}

static void __bt_tr_data_sent_item_sel(void *data, Evas_Object *obj,
				      void *event_info)
{
	DBG("+");
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = NULL;

	ret_if(data == NULL);
	ret_if(event_info == NULL);

	info = (bt_tr_data_t *)data;
	ad->selected_item = (Elm_Object_Item *)event_info;
	elm_genlist_item_selected_set((Elm_Object_Item *)event_info,
						EINA_FALSE);

	if (info->tr_status == BT_TR_SUCCESS)
		ad->ft_popup = _bt_create_ft_popup(BT_FT_SENT_POPUP, info);
	else
		ad->ft_popup = _bt_create_ft_popup(BT_FT_RETRY_POPUP, info);

	DBG("-");
	return;
}

void _bt_append_genlist_tr_data_item(bt_share_appdata_t *ad,
				Evas_Object *genlist, bt_tr_data_t *info, int tr_type)
{
	DBG("+");

	retm_if (ad == NULL || info == NULL, "Invalid parameters!");

	bt_gl_data_t *gl_data = NULL;

	gl_data = g_new0(bt_gl_data_t, 1);
	gl_data->tr_data = info;

	if (tr_type == BT_TR_OUTBOUND) {
		elm_genlist_item_append(genlist, ad->tr_data_itc, gl_data,
					NULL, ELM_GENLIST_ITEM_NONE,
					__bt_tr_data_sent_item_sel, info);

		if (ad->outbound_latest_id < info->id)
			ad->outbound_latest_id = info->id;
	} else if (tr_type == BT_TR_INBOUND) {
		elm_genlist_item_append(genlist, ad->tr_data_itc, gl_data,
					NULL, ELM_GENLIST_ITEM_NONE,
					__bt_tr_data_recv_item_sel, info);

		if (ad->inbound_latest_id < info->id)
			ad->inbound_latest_id = info->id;
	} else {
		ERR("Transfer type is invalid");
	}

	evas_object_show(genlist);

	DBG("-");
}


void _bt_prepend_genlist_tr_data_item(bt_share_appdata_t *ad,
					bt_tr_data_t *info, int tr_type)
{
	DBG("+");

	if (ad == NULL || info == NULL)
		return;

	if (elm_genlist_items_count(ad->tr_genlist) == 0) {
		_bt_nocontent_set(ad, FALSE);
	}

	bt_gl_data_t *gl_data;

	gl_data = g_new0(bt_gl_data_t, 1);
	gl_data->tr_data = info;

	if (tr_type == BT_TR_OUTBOUND) {
		elm_genlist_item_prepend(ad->tr_genlist, ad->tr_data_itc,
					gl_data, NULL,
					ELM_GENLIST_ITEM_NONE,
					__bt_tr_data_sent_item_sel, info);

		if (ad->outbound_latest_id < info->id)
			ad->outbound_latest_id = info->id;
	} else if (tr_type == BT_TR_INBOUND) {
		elm_genlist_item_prepend(ad->tr_genlist, ad->tr_data_itc,
					gl_data, NULL,
					ELM_GENLIST_ITEM_NONE,
					__bt_tr_data_recv_item_sel, info);

		if (ad->inbound_latest_id < info->id)
			ad->inbound_latest_id = info->id;
	} else {
		ERR("Transfer type is invalid");
		g_free(gl_data);
	}

	evas_object_show(ad->tr_genlist);

	DBG("-");
	return;
}

static void __bt_share_gl_highlighted(void *data, Evas_Object *obj,
							void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	bt_gl_data_t *gl_data;
	DBG("+");

	ret_if(item == NULL);

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);

	gl_data->highlighted = TRUE;

	elm_genlist_item_fields_update(item, "*",
				ELM_GENLIST_ITEM_FIELD_CONTENT);

	DBG("-");
}

static void __bt_share_gl_unhighlighted(void *data, Evas_Object *obj,
							void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	bt_gl_data_t *gl_data;
	DBG("+");

	ret_if(item == NULL);

	gl_data = (bt_gl_data_t *)elm_object_item_data_get(item);
	ret_if(gl_data == NULL);
	gl_data->highlighted = FALSE;

	elm_genlist_item_fields_update(item, "*",
				ELM_GENLIST_ITEM_FIELD_CONTENT);

	DBG("-");
}

static Evas_Object *__bt_add_tr_data_genlist(Evas_Object *parent,
						  bt_share_appdata_t *ad)
{
	DBG("+");
	retvm_if (ad == NULL, NULL, "Inavalid parameter!");

	Evas_Object *genlist = elm_genlist_add(parent);

	evas_object_smart_callback_add(genlist, "highlighted",
				__bt_share_gl_highlighted, ad);

	evas_object_smart_callback_add(genlist, "unhighlighted",
				__bt_share_gl_unhighlighted, ad);

	evas_object_smart_callback_add(genlist, "realized",
				__bt_genlist_realized_cb, ad);

	ad->tr_data_itc = elm_genlist_item_class_new();
	if (ad->tr_data_itc) {
		ad->tr_data_itc->item_style = "type1";
		ad->tr_data_itc->func.text_get = __bt_tr_label_get;
		ad->tr_data_itc->func.content_get = __bt_tr_icon_get;
		ad->tr_data_itc->func.state_get = NULL;
		ad->tr_data_itc->func.del = __bt_tr_del;
	}

	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	DBG("-");
	return genlist;
}

static Evas_Object * __bt_create_naviframe(bt_share_appdata_t *ad)
{
	DBG("+");
	retv_if (ad == NULL, NULL);
	Evas_Object *navi_fr = NULL;
	/* Naviframe */
	navi_fr = elm_naviframe_add(ad->tr_view);
	eext_object_event_callback_add(navi_fr, EEXT_CALLBACK_BACK,
						eext_naviframe_back_cb, NULL);
	elm_object_part_content_set(ad->tr_view, "elm.swallow.content", navi_fr);
	evas_object_show(navi_fr);
	DBG("-");
	return navi_fr;
}

void _bt_nocontent_set(bt_share_appdata_t *ad, gboolean set)
{
	DBG("+");
	ret_if(ad == NULL);
	Evas_Object *layout = NULL;
	Evas_Object *nocontents = NULL;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *navi_it = NULL;

	DBG("Set nocontent status : %d", set);

	if (set == TRUE) {
		/* Full view layout */
		layout = elm_layout_add(ad->navi_fr);
		elm_layout_file_set(layout, EDJFILE, "nocontents_layout");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

		/* Create elm_layout and set its style as nocontents/text */
		nocontents = elm_layout_add(layout);
		elm_layout_theme_set(nocontents, "layout", "nocontents", "default");
		evas_object_size_hint_weight_set(nocontents, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(nocontents, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_part_text_set(nocontents, "elm.text", BT_STR_NO_TRANSFER_HISTORY);
		elm_layout_signal_emit(nocontents, "text,disabled", "");
		elm_layout_signal_emit(nocontents, "align.center", "elm");

		if (ad->navi_it) {
			elm_object_item_part_content_set(ad->navi_it,
				"elm.swallow.content", nocontents);
		} else {
			navi_it = elm_naviframe_item_push(ad->navi_fr,
							(ad->tr_type == BT_TR_INBOUND) ?
							BT_STR_RECEIVED_FILES :
							BT_STR_SENT_FILES,
							NULL, NULL, nocontents, NULL);
			elm_naviframe_item_pop_cb_set(navi_it,
					__bt_back_button_cb, ad);
			ad->navi_it = navi_it;
		}

		if (ad->toolbar_btn) {
			evas_object_del(ad->toolbar_btn);
			ad->toolbar_btn = NULL;
		}
		eext_object_event_callback_del(ad->navi_fr, EEXT_CALLBACK_MORE,
				__bt_more_menu_cb);

	}else {
		genlist = __bt_add_tr_data_genlist(ad->tr_view, ad);
		retm_if (genlist == NULL, "genlist is NULL!");

		if (ad->tr_genlist) {
			DBG("Clear the previous genlist");
			elm_genlist_clear(ad->tr_genlist);
			ad->tr_genlist = NULL;
		}

		ad->tr_genlist = genlist;

		if (ad->navi_it) {
			elm_object_item_part_content_set(ad->navi_it,
				"elm.swallow.content", genlist);
		} else {
			navi_it = elm_naviframe_item_push(ad->navi_fr,
							(ad->tr_type == BT_TR_INBOUND) ?
							BT_STR_RECEIVED_FILES :
							BT_STR_SENT_FILES,
							NULL, NULL, genlist, NULL);
			elm_naviframe_item_pop_cb_set(navi_it, __bt_back_button_cb, ad);
			ad->navi_it = navi_it;
		}

		eext_object_event_callback_add(ad->navi_fr, EEXT_CALLBACK_MORE,
				__bt_more_menu_cb, ad);
	}
}

void _bt_cb_state_changed(int result,
			bt_adapter_state_e adapter_state,
			void *user_data)
{
	DBG("bluetooth %s", adapter_state == BT_ADAPTER_ENABLED ?
				"enabled" : "disabled");

	ret_if(!user_data);
	ret_if(result != BT_ERROR_NONE);

	bt_share_appdata_t *ad = (bt_share_appdata_t *)user_data;
	Evas_Object *btn = NULL;

	ad->bt_status = adapter_state;

	if (ad->ft_popup && ad->ft_type == BT_FT_RETRY_POPUP) {
		btn = elm_object_part_content_get(ad->ft_popup, "button1");
		ret_if(!btn);
		elm_object_text_set(btn, adapter_state == BT_ADAPTER_ENABLED ? BT_STR_RETRY : BT_STR_OK);
	}

	if(adapter_state == BT_ADAPTER_DISABLED && ad->progress_popup) {
		DBG("Adapter disabled, and progress popup is present, destroying");
		_bt_destroy_progress_popup(ad);
	}
}

static void __bt_move_clear_ctxpopup(Evas_Object *ctxpopup,
			bt_share_appdata_t *ad)
{
	FN_START;
	Evas_Coord w, h;
	int pos = -1;

	ret_if(ad == NULL);
	ret_if(ad->win == NULL);

	elm_win_screen_size_get(ad->win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(ad->win);

		switch (pos) {
			case 0:
			case 180:
				evas_object_move(ctxpopup, (w / 2), h);
				break;
			case 90:
				evas_object_move(ctxpopup,  (h / 2), w);
				break;
			case 270:
				evas_object_move(ctxpopup, (h / 2), w);
				break;
		}
	FN_END;
}

void __bt_clear_btn_del_cb(void *data, Evas_Object *obj, void *event_info)
{
	FN_START;
	bt_share_appdata_t *ad = NULL;

	ret_if(data == NULL);
	ad = (bt_share_appdata_t *)data;

	if (ad->toolbar_btn != NULL) {
		evas_object_del(ad->toolbar_btn);
		ad->toolbar_btn = NULL;
	}

	FN_END;
}

static void __bt_win_rotation_changed_cb(void *data, Evas_Object *obj,
			void *event_info)
{
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	__bt_move_clear_ctxpopup(ad->toolbar_btn, ad);
}

static void __bt_clear_btn_delete_cb(void *data, Evas *e,
			Evas_Object *obj,	void *event_info)
{
	FN_START;
	Evas_Object *navi = (Evas_Object *)data;
	Evas_Object *ctx = obj;

	ret_if (navi == NULL);

	evas_object_event_callback_del_full(ctx, EVAS_CALLBACK_DEL,
			__bt_clear_btn_delete_cb, navi);
	FN_END;
}

static void __bt_more_menu_cb(void *data,
				Evas_Object *obj, void *event_info)
{
	FN_START;

	Evas_Object *more_ctxpopup = NULL;
	bt_share_appdata_t *ad;

	ad = (bt_share_appdata_t *)data;
	ret_if(ad == NULL);

	more_ctxpopup = elm_ctxpopup_add(ad->win);
	ad->toolbar_btn = more_ctxpopup;
	eext_object_event_callback_add(more_ctxpopup,
			EEXT_CALLBACK_BACK, __bt_clear_btn_del_cb, ad);
	eext_object_event_callback_add(more_ctxpopup,
			EEXT_CALLBACK_MORE, __bt_clear_btn_del_cb, ad);
	elm_object_style_set(more_ctxpopup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(more_ctxpopup, EINA_TRUE);

	elm_ctxpopup_item_append(more_ctxpopup, BT_STR_CLR_LIST,
			NULL, __bt_clear_list_btn_cb, ad);
	evas_object_smart_callback_add(ad->win, "rotation,changed",
			__bt_win_rotation_changed_cb, ad);
	evas_object_event_callback_add(more_ctxpopup, EVAS_CALLBACK_DEL,
			__bt_clear_btn_delete_cb, ad);

	elm_ctxpopup_direction_priority_set(more_ctxpopup, ELM_CTXPOPUP_DIRECTION_UP,
			ELM_CTXPOPUP_DIRECTION_DOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN,
			ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	__bt_move_clear_ctxpopup(more_ctxpopup, ad);
	evas_object_show(more_ctxpopup);

	FN_END;
}
int _bt_create_transfer_view(bt_share_appdata_t *ad)
{
	DBG("Create transfer view");
	retv_if (ad == NULL, -1);

	Elm_Object_Item *navi_it = NULL;
	Evas_Object *conform = NULL;
	Evas_Object *navi_fr = NULL;
	Evas_Object *bg = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *genlist = NULL;
	GSList *list_iter = NULL;

	__bt_clear_view(ad);
	bg = _bt_create_bg(ad->win, NULL);
	retv_if (bg == NULL, -1);
	ad->bg = bg;

	conform = _bt_create_conformant(ad->win, NULL);
	retvm_if (conform == NULL, -1, "conform is NULL!");
	ad->conform = conform;

	bg = elm_bg_add(conform);
	elm_object_style_set(bg, "indicator/headerbg");
	elm_object_part_content_set(conform, "elm.swallow.indicator_bg", bg);
	evas_object_show(bg);

	layout = _bt_create_layout(conform, EDJFILE, "share_view");
	ad->tr_view = layout;

	elm_object_content_set(conform, layout);
	elm_win_conformant_set(ad->win, EINA_TRUE);

	navi_fr = __bt_create_naviframe(ad);
	retvm_if (navi_fr == NULL, -1, "navi_fr is NULL!");
	ad->navi_fr = navi_fr;

	/* Genlist */
	genlist = __bt_add_tr_data_genlist(layout, ad);
	retvm_if (genlist == NULL, -1, "genlist is NULL!");
	ad->tr_genlist = genlist;

	list_iter = ad->tr_data_list;

	/* Add genlist item */
	while (NULL != list_iter) {
		_bt_append_genlist_tr_data_item(ad,  genlist, list_iter->data,
						ad->tr_type);
		list_iter = g_slist_next(list_iter);
	}
	eext_object_event_callback_add(navi_fr, EEXT_CALLBACK_MORE,
					__bt_more_menu_cb, ad);
	navi_it = elm_naviframe_item_push(navi_fr,
					(ad->tr_type == BT_TR_INBOUND) ?
					BT_STR_RECEIVED_FILES :
					BT_STR_SENT_FILES,
					NULL, NULL, genlist, NULL);
	elm_naviframe_item_pop_cb_set(navi_it, __bt_back_button_cb, ad);
	ad->navi_it = navi_it;

	DBG("-");
	return 0;
}

