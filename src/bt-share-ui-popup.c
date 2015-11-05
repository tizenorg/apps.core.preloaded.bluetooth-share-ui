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
#include <bluetooth-share-api.h>
#include <notification.h>
#include <E_DBus.h>

#include "applog.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-resource.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"

#define NEW_LINE	"\n"

extern bt_share_appdata_t *app_state;


static void __bt_destroy_ft_popup(bt_share_appdata_t *ad)
{
	DBG("");
	ret_if(ad == NULL);

	if (ad->ft_popup) {
		DBG("delete ft_popup");
		evas_object_del(ad->ft_popup);
		ad->ft_popup = NULL;
	}

	return;
}

static void __bt_ft_retry_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("+");

	ret_if(data == NULL);
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = (bt_tr_data_t *)data;

	__bt_destroy_ft_popup(ad);

	if (ad->bt_status == BT_ADAPTER_ENABLED) {
		if (!_bt_share_ui_ipc_retry_send(ad, info)) {
			INFO(" info->id %d", info->id);
			_bt_share_ui_ipc_info_update(ad, info->id);
			_bt_delete_selected_item(ad);
		}
	}

	DBG("-");
	return;
}

static void __bt_ft_retry_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG("+");

	ret_if(data == NULL);
	bt_share_appdata_t *ad = app_state;

	__bt_destroy_ft_popup(ad);

	DBG("-");
	return;
}

static void __bt_ft_retry_mouseup_cb(void *data,
			Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = (bt_tr_data_t *)data;

	DBG("Mouse event callback function is called +");
	if (ev->button == 3) {
		__bt_destroy_ft_popup(ad);
		_bt_share_ui_ipc_info_update(ad, info->id);
	}

	DBG("Mouse event callback -");
}

static void __bt_ft_ok_popup_cb(void *data, Evas_Object *obj,
			void *event_info)
{
	DBG(" +");
	ret_if(data == NULL);
	ret_if(!obj);
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = (bt_tr_data_t *)data;

	const char *text = elm_object_text_get(obj);

	if (!g_strcmp0(text, BT_STR_RETRY)) {
		__bt_ft_retry_ok_cb(data, obj, event_info);
		return;
	}

	__bt_destroy_ft_popup(ad);

	if (ad->bt_status == BT_ADAPTER_DISABLED)
		_bt_update_tr_notification(info);
	else /*Adapter enabled case */
		_bt_share_ui_ipc_info_update(ad, info->id);

	_bt_delete_selected_item(ad);

	DBG("-");
	return;
}

static void __bt_ft_ok_mouseup_cb(void *data,
			Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	bt_share_appdata_t *ad = app_state;
	bt_tr_data_t *info = (bt_tr_data_t *)data;

	DBG("Mouse event callback function is called +");

	if (ev->button == 3) {
		__bt_destroy_ft_popup(ad);
		_bt_share_ui_ipc_info_update(ad, info->id);
		_bt_delete_selected_item(ad);
	}
	DBG("Mouse event callback -");
}

static Evas_Object *__bt_create_popup_layout(Evas_Object *parent, bt_share_appdata_t *ad)
{
	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, EDJFILE, "ft_popup");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	ad->progress_layout = layout;
	return layout;
}

static Evas_Object *__bt_create_progressbar(Evas_Object *parent, bt_share_appdata_t *ad)
{
	Evas_Object *progressbar = NULL;
	progressbar = elm_progressbar_add(parent);
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_signal_emit(progressbar, "elm,units,show", "elm");
	elm_progressbar_value_set(progressbar, 0.0);
	evas_object_show(progressbar);
	return progressbar;
}

static Evas_Object *__bt_create_popup_pb_layout(Evas_Object *parent, bt_share_appdata_t *ad)
{
	Evas_Object *layout = NULL;

	layout = elm_layout_add(parent);
	elm_layout_file_set(layout, EDJFILE, "popup_2text_progressbar_view_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	ad->progress_layout = layout;
	return layout;
}

Evas_Object *_bt_create_ft_popup(bt_share_ft_popup_type_e type, bt_tr_data_t *info)
{
	DBG(" +");
	bt_share_appdata_t *ad = app_state;
	Evas_Object *popup = NULL;
	char msg[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	char tmp[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	char *name = NULL;
	char *markup_text = NULL;

	if (info->file_path == NULL) {
		ERR("Invalid data");
		return NULL;
	}

	popup = elm_popup_add(ad->win);
	evas_object_size_hint_weight_set(popup,
					 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	INFO("Popup type : %d", type);
	ad->ft_type = type;
	ad->ft_info = info;

	if (type == BT_FT_SENT_POPUP) {
		DBG("BT_FT_SENT_POPUP");
		elm_object_part_text_set(popup, "title,text", BT_STR_SENT_FILES);
		char file_size_str[BT_FILE_SIZE_STR] = { 0 };

		snprintf(msg, sizeof(msg), BT_STR_TO_S, info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		/* file size */
		if (info->size < 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dB", info->size);
		else if (info->size > 1024 * 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dMB", info->size / (1024 * 1024));
		else
			snprintf(file_size_str, sizeof(file_size_str),
				"%dKB", info->size / 1024);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_SIZE_S, file_size_str);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(popup, markup_text);
	} else if (type == BT_FT_RECV_SUCCESS_POPUP) {
		DBG("BT_FT_RECV_SUCCESS_POPUP");
		elm_object_part_text_set(popup, "title,text", BT_STR_RECEIVED_FILES);
		char file_size_str[BT_FILE_SIZE_STR] = { 0 };

		snprintf(msg, sizeof(msg), BT_STR_FROM_S, info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		/* file size */
		if (info->size < 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dB", info->size);
		else if (info->size > 1024 * 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dMB", info->size / (1024 * 1024));
		else
			snprintf(file_size_str, sizeof(file_size_str),
				"%dKB", info->size / 1024);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_SIZE_S, file_size_str);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(popup, markup_text);
	} else if (type == BT_FT_RECV_FAIL_POPUP) {
		DBG("BT_FT_RECV_FAIL_POPUP");
		elm_object_part_text_set(popup, "title,text", BT_STR_UNABLE_TO_RECEIVED_FILES);

		g_strlcat(msg, BT_STR_FILE_NOT_RECV, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, info->file_path);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(popup, markup_text);
	} else if (type == BT_FT_FILE_NOT_EXIST) {
		elm_object_part_text_set(popup, "title,text", BT_STR_RECEIVED_FILES);
		__bt_create_popup_layout(popup, ad);

		g_strlcat(msg, BT_STR_FILE_NOT_EXIST, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, info->file_path);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(popup, markup_text);
	} else  if (type == BT_FT_RETRY_POPUP) {
		elm_object_part_text_set(popup, "title,text", BT_STR_UNABLE_TO_SEND_FILES);

		snprintf(msg, sizeof(msg), BT_STR_SEND_FAIL_TO_S, info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(popup, markup_text);
	}

	evas_object_show(popup);

	if (type == BT_FT_RETRY_POPUP) {
		if (ad->bt_status == BT_ADAPTER_DISABLED ||
			access(info->file_path, F_OK) != 0) {
			_bt_create_button(popup,  "popup", "button1",
					BT_STR_OK, NULL, __bt_ft_ok_popup_cb, info);
		} else {
			_bt_create_button(popup, "popup", "button1",
					BT_STR_RETRY, NULL, __bt_ft_retry_ok_cb, info);
		}
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
				__bt_ft_retry_cancel_cb, info);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__bt_ft_retry_mouseup_cb, info);
	} else {
		_bt_create_button(popup, "popup", "button1",
				BT_STR_OK, NULL, __bt_ft_ok_popup_cb, info);
		eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
				__bt_ft_ok_popup_cb, info);
		evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
				__bt_ft_ok_mouseup_cb, info);
	}

	if(markup_text)
		free(markup_text);

	DBG("-");
	return popup;
}

void _bt_lang_changed_ft_popup(bt_share_appdata_t *ad)
{
	DBG(" +");
	char msg[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	char tmp[BT_GLOBALIZATION_STR_LENGTH] = { 0 };
	char *name = NULL;
	char *markup_text = NULL;
	Evas_Object *btn1 = NULL;

	if (ad->ft_type == BT_FT_SENT_POPUP) {
		elm_object_part_text_set(ad->ft_popup, "title,text", BT_STR_SENT_FILES);
		char file_size_str[BT_FILE_SIZE_STR] = { 0 };

		snprintf(msg, sizeof(msg), BT_STR_FROM_S, ad->ft_info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(ad->ft_info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = ad->ft_info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		/* file size */
		if (ad->ft_info->size < 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dB", ad->ft_info->size);
		else if (ad->ft_info->size > 1024 * 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dMB", ad->ft_info->size / (1024 * 1024));
		else
			snprintf(file_size_str, sizeof(file_size_str),
				"%dKB", ad->ft_info->size / 1024);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_SIZE_S, file_size_str);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(ad->ft_popup, markup_text);
	} else if (ad->ft_type == BT_FT_RECV_SUCCESS_POPUP) {
		elm_object_part_text_set(ad->ft_popup, "title,text", BT_STR_RECEIVED_FILES);
		char file_size_str[BT_FILE_SIZE_STR] = { 0 };

		snprintf(msg, sizeof(msg), BT_STR_FROM_S, ad->ft_info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(ad->ft_info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = ad->ft_info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		/* file size */
		if (ad->ft_info->size < 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dB", ad->ft_info->size);
		else if (ad->ft_info->size > 1024 * 1024)
			snprintf(file_size_str, sizeof(file_size_str),
				"%dMB", ad->ft_info->size / (1024 * 1024));
		else
			snprintf(file_size_str, sizeof(file_size_str),
				"%dKB", ad->ft_info->size / 1024);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_SIZE_S, file_size_str);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(ad->ft_popup, markup_text);
	} else if (ad->ft_type == BT_FT_RECV_FAIL_POPUP) {
		elm_object_part_text_set(ad->ft_popup, "title,text", BT_STR_UNABLE_TO_RECEIVED_FILES);

		g_strlcat(msg, BT_STR_FILE_NOT_RECV, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, ad->ft_info->file_path);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		snprintf(tmp, sizeof(tmp), BT_STR_FAIL_S, BT_STR_TR_CANCELLED);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(ad->ft_popup, markup_text);
	} else if (ad->ft_type == BT_FT_FILE_NOT_EXIST) {
		elm_object_part_text_set(ad->ft_popup, "title,text", BT_STR_RECEIVED_FILES);

		g_strlcat(msg, BT_STR_FILE_NOT_EXIST, BT_GLOBALIZATION_STR_LENGTH);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, ad->ft_info->file_path);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(ad->ft_popup, markup_text);
	} else  if (ad->ft_type == BT_FT_RETRY_POPUP) {
		elm_object_part_text_set(ad->ft_popup, "title,text", BT_STR_UNABLE_TO_SEND_FILES);

		snprintf(msg, sizeof(msg), BT_STR_SEND_FAIL_TO_S, ad->ft_info->dev_name);
		g_strlcat(msg, NEW_LINE, BT_GLOBALIZATION_STR_LENGTH);

		name = strrchr(ad->ft_info->file_path, '/');
		if (name != NULL)
			name++;
		else
			name = ad->ft_info->file_path;

		snprintf(tmp, sizeof(tmp), BT_STR_FILE_S, name);
		g_strlcat(msg, tmp, BT_GLOBALIZATION_STR_LENGTH);

		markup_text = elm_entry_utf8_to_markup(msg);
		elm_object_text_set(ad->ft_popup, markup_text);
	}

	if (ad->ft_type == BT_FT_RETRY_POPUP) {
		btn1 = elm_object_part_content_get(ad->ft_popup, "button1");
		if (ad->bt_status == BT_ADAPTER_DISABLED) {
			if (btn1)
				elm_object_text_set(btn1, BT_STR_OK);
		} else {
			if (btn1)
				elm_object_text_set(btn1, BT_STR_RETRY);
		}
	} else {
		btn1 = elm_object_part_content_get(ad->ft_popup, "button1");
		if (btn1)
			elm_object_text_set(btn1, BT_STR_OK);
	}

	if(markup_text)
		free(markup_text);

	DBG(" -");
}

static void __bt_progress_ok_cb(void *data, Evas_Object *obj, void *event_info)
{
	DBG(" +");
	ret_if(data == NULL);

	bt_share_appdata_t *ad = NULL;

	ad = (bt_share_appdata_t *)data;

	_bt_destroy_progress_popup(ad);

	if (ad->tr_view == NULL)
		_bt_terminate_app();

	DBG("-");
	return;
}

static void __bt_progress_cancel_cb(void *data, Evas_Object *obj,
						void *event_info)
{
	DBG(" +");
	ret_if(data == NULL);

	bt_share_appdata_t *ad = NULL;
	bt_share_popup_data_t *pb_data = NULL;

	ad = (bt_share_appdata_t *)data;
	pb_data = evas_object_data_get(ad->progress_popup, "progressbar_data");
	_bt_abort_signal_send(ad, pb_data);

	_bt_destroy_progress_popup(ad);

	if (ad->tr_view == NULL)
		_bt_terminate_app();

	DBG("-");
	return;
}

static void __bt_progress_mouseup_cb(void *data,
			Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Event_Mouse_Up *ev = event_info;
	bt_share_appdata_t *ad = NULL;
	bt_share_popup_data_t *pb_data = NULL;

	ad = (bt_share_appdata_t *)data;
	pb_data = evas_object_data_get(ad->progress_popup, "progressbar_data");

	DBG("Mouse event callback function is called +");

	if (ev->button == 3) {
		_bt_abort_signal_send(ad, pb_data);
		_bt_destroy_progress_popup(ad);

		if (ad->tr_view == NULL)
			_bt_terminate_app();
	}
	DBG("Mouse event callback -");
}

Evas_Object *_bt_create_progress_popup(bt_share_popup_data_t *data)
{
	DBG(" +");
	retv_if(data == NULL, NULL);
	Evas_Object *popup = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *progressbar = NULL;
	bt_share_appdata_t *ad = app_state;
	char temp[BT_PERCENT_STR_LEN] = { 0 };
	float i = 0.0;
	int tmp = 0;
	char *markup_text = NULL;

	popup = elm_popup_add(ad->win);
	evas_object_size_hint_weight_set(popup,
				 EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	layout = __bt_create_popup_pb_layout(popup, ad);

	elm_object_part_text_set(popup, "title,text", (!strcmp(data->transfer_type, "outbound")) ?
					BT_STR_SEND_FILES : BT_STR_RECEIVE_FILES);
	elm_object_part_text_set(layout, "elm.text.description", (!strcmp(data->transfer_type, "outbound")) ?
						BT_STR_SENDING : BT_STR_RECEIVING);

	markup_text = elm_entry_utf8_to_markup(data->filename);
	elm_object_part_text_set(layout, "elm.subtext.description", markup_text);

	if(markup_text)
		free(markup_text);

	progressbar = __bt_create_progressbar(popup, ad);
	elm_object_part_content_set(layout, "progressbar", progressbar);
	ad->pb = progressbar;

	tmp = atoi(data->percentage);
	elm_progressbar_unit_format_set(ad->pb, NULL);
	i = (float)tmp / (float)100.0;
	elm_progressbar_value_set(ad->pb, i);
	evas_object_show(ad->pb);

	snprintf(temp, BT_PERCENT_STR_LEN, "%d%%", tmp);
	elm_object_part_text_set(progressbar,"elm.text.bottom.left", temp);
	elm_object_signal_emit(progressbar, "elm.text.bottom.show", "elm");

	ad->pb_transfer_id = data->transfer_id;
	elm_object_content_set(popup, layout);

	_bt_create_button(popup, "popup", "button1",
			BT_STR_RESUME, NULL, __bt_progress_ok_cb, ad);
	_bt_create_button(popup, "popup", "button2",
			BT_STR_CANCEL, NULL, __bt_progress_cancel_cb, ad);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
			__bt_progress_ok_cb, ad);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP,
			__bt_progress_mouseup_cb, ad);

	evas_object_show(popup);

	DBG("-");
	ad->progress_data = data;
	return popup;
}

int _bt_destroy_progress_popup(bt_share_appdata_t *ad)
{
	DBG("+");

	retvm_if(ad == NULL, 0,
		 "Invalid argument: ad is NULL");
	bt_share_popup_data_t *pb_data = NULL;

	if (ad->progress_popup) {
		pb_data = evas_object_data_get(ad->progress_popup,
				"progressbar_data");
		if (pb_data) {
			g_free((gpointer)pb_data->filename);
			g_free((gpointer)pb_data->percentage);
			g_free((gpointer)pb_data->transfer_type);
			g_free((gpointer)pb_data);
		}

		evas_object_del(ad->progress_popup);
		ad->progress_popup = NULL;
	}

	DBG("-");
	return 0;
}

void _bt_lang_changed_progress_popup(bt_share_appdata_t *ad)
{
	DBG(" +");
	ret_if(ad == NULL);
	Evas_Object *btn1 = NULL;
	Evas_Object *btn2 = NULL;

	elm_object_part_text_set(ad->progress_popup, "title,text",
					(!strcmp(ad->progress_data->transfer_type, "outbound")) ?
					BT_STR_SEND_FILES : BT_STR_RECEIVE_FILES);
	elm_object_part_text_set(ad->progress_layout, "elm.text.description",
					(!strcmp(ad->progress_data->transfer_type, "outbound")) ?
					BT_STR_SENDING : BT_STR_RECEIVING);

	btn1 = elm_object_part_content_get(ad->progress_popup, "button1");
	if (btn1)
		elm_object_text_set(btn1, BT_STR_RESUME);
	btn2 = elm_object_part_content_get(ad->progress_popup, "button2");
	if (btn2)
		elm_object_text_set(btn2, BT_STR_CANCEL);

	DBG(" -");
}

int _bt_update_progress_popup(bt_share_appdata_t *ad,
			int transfer_id, const char *name, int percentage)
{
	char temp[BT_PERCENT_STR_LEN] = {0};

	retvm_if(ad == NULL, 0, "Invalid argument: ad is NULL");
	retvm_if(ad->pb_transfer_id != transfer_id, 0, "Invalid transfer_id!");
	retvm_if(!ad->progress_popup, 0, "No progress_popup!");

	float i = 0.0;
	i = (float)(percentage) / (float)100.0;
	elm_progressbar_value_set(ad->pb, i);
	evas_object_show(ad->pb);

	snprintf(temp, BT_PERCENT_STR_LEN, "%d%%", percentage);
	elm_object_part_text_set(ad->pb,"elm.text.bottom.left", temp);
	elm_object_signal_emit(ad->pb, "elm,bottom,text,show", "elm");
	return 0;
}


static gboolean __bt_info_popup_timer_cb(void *data, Evas_Object *obj,
				    void *event_info)
{
	DBG("+");
	bt_share_appdata_t *ad = (bt_share_appdata_t *)data;
	retv_if(ad == NULL, FALSE);

	_bt_destroy_info_popup(ad);

	if (ad->tr_view == NULL) {
		_bt_terminate_app();
	}
	DBG("-");
	return FALSE;
}

Evas_Object *_bt_create_info_popup(bt_share_appdata_t *ad,
					const char *title, const char *text)
{
	DBG("+");
	Evas_Object *popup = NULL;

	retv_if((!ad || !title || !text), NULL);
	retv_if (ad->info_popup != NULL, NULL);

	popup = elm_popup_add(ad->win);
	elm_object_style_set(popup, "toast");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
			eext_popup_back_cb, NULL);
	elm_object_text_set(popup, text);
	evas_object_smart_callback_add(popup, "block,clicked",
			(Evas_Smart_Cb)__bt_info_popup_timer_cb, ad);

	if(!elm_config_access_get()) {
		elm_popup_timeout_set(popup, BT_INFO_POPUP_TIMEOUT_IN_SEC);
		evas_object_smart_callback_add(popup, "timeout",
			(Evas_Smart_Cb) __bt_info_popup_timer_cb, ad);
	} else {
		evas_object_smart_callback_add(popup, "access,read,stop",
			(Evas_Smart_Cb) __bt_info_popup_timer_cb, ad);
	}

	evas_object_show(popup);
	ad->info_popup = popup;

	DBG("-");
	return popup;
}

static gboolean __bt_extinctive_info_popup_timer_cb(void *data,
				Evas_Object *obj, void *event_info)
{
	bt_share_appdata_t *ad;
	ad = (bt_share_appdata_t *)data;
	retv_if(ad == NULL, FALSE);

	_bt_destroy_info_popup(ad);

	if (!ad->tr_view)
		_bt_terminate_app();

	return FALSE;
}

Evas_Object *_bt_create_extinctive_info_popup(bt_share_appdata_t *ad,
				const char *title, const char *text)
{
	DBG("+");
	Evas_Object *popup = NULL;

	retv_if((!ad || !title || !text), NULL);

	if (ad->info_popup != NULL)
		return NULL;

	popup = elm_popup_add(ad->win);
	elm_object_style_set(popup, "toast");
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
			eext_popup_back_cb, NULL);
	elm_object_text_set(popup, text);
	evas_object_smart_callback_add(popup, "block,clicked",
			(Evas_Smart_Cb)__bt_extinctive_info_popup_timer_cb, ad);

	if(!elm_config_access_get()) {
		elm_popup_timeout_set(popup, BT_INFO_POPUP_TIMEOUT_IN_SEC);
		evas_object_smart_callback_add(popup, "timeout",
			(Evas_Smart_Cb) __bt_extinctive_info_popup_timer_cb, ad);
	} else {
		evas_object_smart_callback_add(popup, "access,read,stop",
			(Evas_Smart_Cb) __bt_extinctive_info_popup_timer_cb, ad);
	}

	evas_object_show(popup);
	ad->info_popup = popup;

	DBG("-");
	return popup;
}


int _bt_destroy_info_popup(bt_share_appdata_t *ad)
{
	DBG("+");
	retvm_if(ad == NULL, 0,
		 "Invalid argument: ad is NULL\n");

	if (ad->info_popup) {
		DBG("delete popup");
		evas_object_del(ad->info_popup);
		ad->info_popup = NULL;
	}

	DBG("-");
	return 0;
}

