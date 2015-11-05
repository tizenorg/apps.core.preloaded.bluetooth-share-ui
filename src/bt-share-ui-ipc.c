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

#include <dbus/dbus.h>
#include <dbus/dbus-glib-bindings.h>
#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <bluetooth-share-api.h>
#include "applog.h"
#include "bt-share-ui-main.h"
#include "bt-share-ui-ipc.h"
#include "bt-share-ui-view.h"
#include "bt-share-ui-popup.h"
#include "bt-share-ui-resource.h"


#define BT_ADDRESS_LENGTH_MAX	6
#define DBUS_CORE_APPS_PATH "/Org/Tizen/Coreapps/home/raise"
#define DBUS_CORE_APPS_INTERFACE "org.tizen.coreapps.home.raise"
#define DBUS_CORE_APPS_MEMBER "homeraise"


static void __handle_opp_client_signal(void *data, DBusMessage *msg)
{
	bluetooth_event_param_t event_info = { 0, };
	int result = BLUETOOTH_ERROR_NONE;
	const char *member = dbus_message_get_member(msg);

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");

	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, BT_EVENT_SERVICE))
		return;

	if (!dbus_message_has_path(msg, BT_OPP_CLIENT_PATH))
		return;

	ret_if(member == NULL);

	if (strcasecmp(member, BT_OPP_CONNECTED) == 0) {
		char *address = NULL;
		int request_id = 0;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &address,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}
		event_info.event = BLUETOOTH_EVENT_OPC_CONNECTED;
		event_info.param_data = address;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(BLUETOOTH_EVENT_OPC_CONNECTED,
				&event_info, event_info.user_data);
	} else if (strcasecmp(member, BT_TRANSFER_PROGRESS) == 0) {
		char *file_name = NULL;
		int request_id = 0;
		guint64 size = 0;
		int progress = 0;
		bt_opc_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &progress,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00, sizeof(bt_opc_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.size = size;
		transfer_info.percentage = progress;

		event_info.event = BLUETOOTH_EVENT_OPC_TRANSFER_PROGRESS;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BLUETOOTH_EVENT_OPC_TRANSFER_PROGRESS,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
	} else if (strcasecmp(member, BT_TRANSFER_COMPLETED) == 0) {
		char *file_name = NULL;
		int request_id = 0;
		guint64 size = 0;
		bt_opc_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &request_id,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00, sizeof(bt_opc_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.size = size;

		event_info.event = BLUETOOTH_EVENT_OPC_TRANSFER_COMPLETE;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BLUETOOTH_EVENT_OPC_TRANSFER_COMPLETE,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
	}
}

static void __handle_obex_server_signal(void *data, DBusMessage *msg)
{
	bluetooth_event_param_t event_info = { 0, };
	int result = BLUETOOTH_ERROR_NONE;
	const char *member = dbus_message_get_member(msg);

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");

	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, BT_EVENT_SERVICE))
		return;

	if (!dbus_message_has_path(msg, BT_OPP_SERVER_PATH))
		return;

	retm_if(member == NULL, "member value is NULL\n");

	if (strcasecmp(member, BT_TRANSFER_PROGRESS) == 0) {
		char *file_name = NULL;
		char *type = NULL;
		int transfer_id = 0;
		int progress = 0;
		int server_type = 0; /* bt_server_type_t */
		guint64 size = 0;
		bt_obex_server_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_STRING, &type,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INT32, &progress,
			DBUS_TYPE_INT32, &server_type,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00,
			sizeof(bt_obex_server_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.type = g_strdup(type);
		transfer_info.file_size = size;
		transfer_info.transfer_id = transfer_id;
		transfer_info.percentage = progress;

		event_info.event = BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_PROGRESS;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_PROGRESS,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
		g_free(transfer_info.type);
	} else if (strcasecmp(member, BT_TRANSFER_COMPLETED) == 0) {
		char *file_name = NULL;
		char *device_name = NULL;
		char *type = NULL;
		char *file_path;
		int transfer_id = 0;
		int server_type = 0; /* bt_server_type_t */
		guint64 size = 0;
		bt_obex_server_transfer_info_t transfer_info;

		if (!dbus_message_get_args(msg, NULL,
			DBUS_TYPE_INT32, &result,
			DBUS_TYPE_STRING, &file_name,
			DBUS_TYPE_STRING, &type,
			DBUS_TYPE_STRING, &device_name,
			DBUS_TYPE_STRING, &file_path,
			DBUS_TYPE_UINT64, &size,
			DBUS_TYPE_INT32, &transfer_id,
			DBUS_TYPE_INT32, &server_type,
			DBUS_TYPE_INVALID)) {
			ERR("Unexpected parameters in signal");
			return;
		}

		memset(&transfer_info, 0x00,
				sizeof(bt_obex_server_transfer_info_t));

		transfer_info.filename = g_strdup(file_name);
		transfer_info.type = g_strdup(type);
		transfer_info.device_name = g_strdup(device_name);
		transfer_info.file_path = g_strdup(file_path);
		transfer_info.file_size = size;
		transfer_info.transfer_id = transfer_id;

		event_info.event = BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_COMPLETED;
		event_info.param_data = &transfer_info;
		event_info.result = result;
		event_info.user_data = data;

		_bt_share_ui_event_handler(
				BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_COMPLETED,
				&event_info, event_info.user_data);

		g_free(transfer_info.filename);
		g_free(transfer_info.type);
		g_free(transfer_info.device_name);
		g_free(transfer_info.file_path);
	}
}

static void __handle_update_view_signal(void *data, DBusMessage * msg)
{
	bt_share_appdata_t *ad;
	char *db;

	retm_if(data == NULL, "Invalid argument: data is NULL\n");
	retm_if(msg == NULL, "Invalid argument: msg is NULL\n");

	ad = (bt_share_appdata_t *)data;

	DBG("+");

	if (!dbus_message_get_args(msg, NULL,
				DBUS_TYPE_STRING, &db,
				DBUS_TYPE_INVALID)) {
		ERR("Event handling failed");
		return;
	}

	_bt_share_ui_handle_update_view(ad, db);
}

static void __handle_home_key_signal(void *data, DBusMessage *msg)
{
	const char *member;

	retm_if(data == NULL, "Invalid argument: data is NULL");
	retm_if(msg == NULL, "Invalid argument: msg is NULL");

	member = dbus_message_get_member(msg);
	retm_if(member == NULL, "member value is NULL");

	if (dbus_message_get_type(msg) != DBUS_MESSAGE_TYPE_SIGNAL)
		return;

	if (!dbus_message_has_interface(msg, DBUS_CORE_APPS_INTERFACE) ||
		!dbus_message_has_path(msg, DBUS_CORE_APPS_PATH))
		return;

	DBG("Received signal : %s", member);

	if (strcasecmp(member, DBUS_CORE_APPS_MEMBER) == 0) {
		_bt_terminate_app();
	}
}

static void __bt_add_tr_data_list(bt_share_appdata_t *ad, int transfer_type)
{
	GSList *list = NULL;
	sqlite3 *db = NULL;
	bt_tr_data_t *info = NULL;
	int len = 0;
	int i;

	db = bt_share_open_db();
	if (!db)
		return;

	list = bt_share_get_completed_tr_data_list(db, transfer_type);
	if (list == NULL) {
		bt_share_close_db(db);
		return;
	}
	bt_share_close_db(db);

	/* Append new tr data to tr_data_list */
	len = g_slist_length(list);
	for (i = 0; i < len; i++) {
		info = list->data;
		if (transfer_type == BT_TR_OUTBOUND &&
				info->id > ad->outbound_latest_id)
			ad->tr_data_list = g_slist_append(ad->tr_data_list, info);
		else if (transfer_type == BT_TR_INBOUND &&
				info->id > ad->inbound_latest_id)
			ad->tr_data_list = g_slist_append(ad->tr_data_list, info);

		list = g_slist_next(list);
		if (list == NULL)
			return;
	}

	bt_share_release_tr_data_list(list);
}

void _bt_signal_init(bt_share_appdata_t *ad)
{
	E_DBus_Connection *conn = NULL;
	E_DBus_Signal_Handler *sh = NULL;
	e_dbus_init();

	conn = e_dbus_bus_get(DBUS_BUS_SYSTEM);
	retm_if(conn == NULL, "conn is NULL\n");

	e_dbus_request_name(conn, BT_SHARE_UI_INTERFACE, 0, NULL, NULL);

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_CONNECTED,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "Connect Event register failed\n");
	ad->client_connected_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_PROGRESS,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "progress Event register failed\n");
	ad->client_progress_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_CLIENT_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_COMPLETED,
				       __handle_opp_client_signal, ad);
	retm_if(sh == NULL, "complete Event register failed\n");
	ad->client_completed_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_SERVER_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_PROGRESS,
				       __handle_obex_server_signal, ad);
	retm_if(sh == NULL, "progress Event register failed\n");
	ad->server_progress_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
				       NULL,
				       BT_OPP_SERVER_PATH,
				       BT_EVENT_SERVICE,
				       BT_TRANSFER_COMPLETED,
				       __handle_obex_server_signal, ad);
	retm_if(sh == NULL, "complete Event register failed\n");
	ad->server_completed_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					NULL,
					BT_SHARE_ENG_OBJECT,
					BT_SHARE_ENG_INTERFACE,
					BT_SHARE_ENG_SIGNAL_UPDATE_VIEW,
					__handle_update_view_signal, ad);

	retm_if(sh == NULL, "Progress Event register failed\n");
	ad->update_sh = sh;

	sh = e_dbus_signal_handler_add(conn,
					NULL,
					DBUS_CORE_APPS_PATH,
					DBUS_CORE_APPS_INTERFACE,
					DBUS_CORE_APPS_MEMBER,
					__handle_home_key_signal, ad);
	retm_if(sh == NULL, "Connect Event register failed");
	ad->app_core_sh = sh;

	ad->dbus_conn = conn;

	return;
}


void _bt_signal_deinit(bt_share_appdata_t *ad)
{
	ret_if(ad == NULL);

	e_dbus_signal_handler_del(ad->dbus_conn, ad->client_connected_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->client_progress_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->client_completed_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->server_progress_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->server_completed_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->update_sh);
	e_dbus_signal_handler_del(ad->dbus_conn, ad->app_core_sh);
	return;
}

int _bt_abort_signal_send(bt_share_appdata_t *ad,
				bt_share_popup_data_t *pb_data)
{
	DBG("+");
	DBusMessage *msg = NULL;

	retvm_if(pb_data == NULL, -1,
		 "progressbar data is NULL\n");
	retvm_if(ad->dbus_conn == NULL, -1,
		 "Invalid argument: ad->dbus_conn is NULL\n");

	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
			BT_SHARE_UI_INTERFACE,
			BT_SHARE_UI_SIGNAL_OPPABORT);

	retvm_if(msg == NULL, -1, "msg is NULL\n");

	if (!dbus_message_append_args(msg,
			DBUS_TYPE_STRING, &pb_data->transfer_type,
			DBUS_TYPE_INT32, &pb_data->transfer_id,
			DBUS_TYPE_INVALID)) {
		ERR("Abort sending failed");
		dbus_message_unref(msg);
		return -1;
	}

	ad->opp_transfer_abort = TRUE; /* Transfer aborted by user */

	INFO("pb_data->transfer_type = %s", pb_data->transfer_type);
	INFO("pb_data->transfer_id = %d", pb_data->transfer_id);

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);
	dbus_message_unref(msg);
	DBG("-");
	return 0;
}

static void __bt_conv_addr_string_to_addr_type(char *addr,
						  const char *address)
{
	int i;
	char *ptr = NULL;

	if (!address || !addr)
		return;

	for (i = 0; i < BT_ADDRESS_LENGTH_MAX; i++) {
		addr[i] = strtol(address, &ptr, 16);
		if (ptr != NULL) {
			if (ptr[0] != ':')
				return;

			address = ptr + 1;
		}
	}
}

int _bt_share_ui_ipc_retry_send(bt_share_appdata_t *ad, bt_tr_data_t *info)
{
	DBG("+");
	DBusMessage *msg = NULL;
	DBusMessageIter iter;
	DBusMessageIter array_iter;
	DBusMessageIter file_iter;
	DBusMessageIter filepath_iter;
	const char *bd_addr = NULL;
	char **file_path;
	int i;

	retvm_if(info == NULL, -1, "Invalid argument: info is NULL\n");

	retvm_if(ad->dbus_conn == NULL, -1,
			"Invalid argument: ad->dbus_conn is NULL\n");

	bd_addr = (char *)calloc(sizeof(char), BT_ADDRESS_LENGTH_MAX);
	if (bd_addr == NULL) {
		return -1;
	}

	__bt_conv_addr_string_to_addr_type((char *)bd_addr, info->addr);
	DBG("%2.2X:%2.2X:%2.2X:%2.2X:%2.2X:%2.2X",
		bd_addr[0], bd_addr[1], bd_addr[2],
		bd_addr[3], bd_addr[4], bd_addr[5]);

	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
						BT_SHARE_UI_INTERFACE,
						BT_SHARE_UI_SIGNAL_SEND_FILE);
	if (msg == NULL) {
		ERR("msg is NULL");
		free((void *)bd_addr);
		return -1;
	}

	if (!g_utf8_validate(info->file_path, -1, NULL)) {
		ERR("Invalid filepath");
		free((void *)bd_addr);

		_bt_create_info_popup(ad, BT_STR_INFORMATION,
			BT_STR_UNABLE_TO_SEND);

		return -1;
	}
/* file validation check begin*/

	if (access(info->file_path, F_OK) != 0) {
		ERR("access failed. May be file is deleted from the Device");
		free((void *)bd_addr);

		_bt_create_info_popup(ad, BT_STR_INFORMATION,
			BT_STR_FILE_NOT_EXIST);

		return 0;
	}
/* file validation check end*/

	file_path = g_new0(char *, 1);
	file_path[0] = g_strdup(info->content);

	dbus_message_iter_init_append(msg, &iter);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_BYTE_AS_STRING, &array_iter);

	for (i = 0; i < BT_ADDRESS_LENGTH_MAX; i++) {
		dbus_message_iter_append_basic(&array_iter,
				DBUS_TYPE_BYTE, &bd_addr[i]);
	}
	dbus_message_iter_close_container(&iter, &array_iter);

	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
						&info->dev_name);
	dbus_message_iter_append_basic(&iter, DBUS_TYPE_STRING,
						&info->type);

	dbus_message_iter_open_container(&iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_ARRAY_AS_STRING
				DBUS_TYPE_BYTE_AS_STRING, &file_iter);

	dbus_message_iter_open_container(&file_iter, DBUS_TYPE_ARRAY,
				DBUS_TYPE_BYTE_AS_STRING, &filepath_iter);

	for (i = 0; i < strlen(file_path[0]); i++) {
		dbus_message_iter_append_basic(&filepath_iter,
				DBUS_TYPE_BYTE, &file_path[0][i]);
	}

	dbus_message_iter_close_container(&file_iter, &filepath_iter);
	dbus_message_iter_close_container(&iter, &file_iter);

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);
	dbus_message_unref(msg);
	free((void *)bd_addr);
	g_free(file_path[0]);
	g_free(file_path);

	DBG("-");
	return 0;
}


int _bt_share_ui_ipc_info_update(bt_share_appdata_t *ad, int uid)
{
	DBG("+");
	retv_if(ad == NULL, -1);

	DBusMessage *msg = NULL;
	int info_uid = 0;
	int info_type = 0;

	info_uid = uid;
	info_type = ad->tr_type;

	INFO("info_uid = %d", info_uid);
	INFO("info_type = %d", info_type);

	retvm_if(ad->dbus_conn == NULL, -1,
			"Invalid argument: ad->dbus_conn is NULL\n");

	msg = dbus_message_new_signal(BT_SHARE_ENG_OBJECT,
				      BT_SHARE_UI_INTERFACE,
				      BT_SHARE_UI_SIGNAL_INFO_UPDATE);

	retvm_if(msg == NULL, -1, "msg is NULL\n");

	if (!dbus_message_append_args(msg,
				      DBUS_TYPE_INT32, &info_uid,
				      DBUS_TYPE_INT32, &info_type,
				      DBUS_TYPE_INVALID)) {
		ERR("Connect sending failed");
		dbus_message_unref(msg);
		return -1;
	}

	e_dbus_message_send(ad->dbus_conn, msg, NULL, -1, NULL);
	dbus_message_unref(msg);

	DBG("-");
	return 0;
}

void _bt_share_ui_handle_update_view(bt_share_appdata_t *ad,
						char *table)
{
	GSList *list_iter = NULL;
	bt_tr_data_t *info = NULL;
	int transfer_type;

	DBG("+");

	if (g_strcmp0(table, BT_INBOUND_TABLE) == 0)
		transfer_type = BT_TR_INBOUND;
	else if (g_strcmp0(table, BT_OUTBOUND_TABLE) == 0)
		transfer_type = BT_TR_OUTBOUND;
	else
		return;

	__bt_add_tr_data_list(ad, transfer_type);

	/* Insert new transfer result to first genlist item  */
	list_iter = ad->tr_data_list;
	if (transfer_type == BT_TR_INBOUND && ad->tr_type == BT_TR_INBOUND) {
		while (NULL != list_iter) {
			info = list_iter->data;
			if (info->id > ad->inbound_latest_id)
				_bt_prepend_genlist_tr_data_item
						(ad, info, transfer_type);
			list_iter = g_slist_next(list_iter);
		}
	} else if (transfer_type == BT_TR_OUTBOUND &&
		ad->tr_type == BT_TR_OUTBOUND) {
		while (NULL != list_iter) {
			info = list_iter->data;
			if (info->id > ad->outbound_latest_id)
				_bt_prepend_genlist_tr_data_item
						(ad, info, transfer_type);
			list_iter = g_slist_next(list_iter);
		}
	}

	evas_object_show(ad->tr_genlist);
	DBG("-");
}

static void __bt_share_ui_handle_progress(bt_share_appdata_t *ad,
					int transfer_id, char *name, int percentage,
					gboolean completed, int error_type)
{

	if (completed == FALSE) {
		_bt_update_progress_popup(ad, transfer_id, name, percentage);
	} else {
		if (error_type == BLUETOOTH_ERROR_NONE) {
			if (ad->progress_popup &&
					(ad->pb_transfer_id == transfer_id))
				_bt_destroy_progress_popup(ad);
		} else {
			DBG("Don,t terminate. Warning Popup will be shown.");
		}
	}
}
static void __bt_share_ui_handle_error(bt_share_appdata_t *ad, int error_type,
						bt_share_tr_type_e trans_type)
{

	DBG("Error type : %d", error_type);

	switch (error_type) {
	case BLUETOOTH_ERROR_NOT_CONNECTED:
		_bt_destroy_progress_popup(ad);
		_bt_create_info_popup(ad, BT_STR_INFORMATION,
			BT_STR_UNABLE_TO_SEND);
		break;
	case BLUETOOTH_ERROR_CANCEL:
		_bt_destroy_progress_popup(ad);
		DBG("opp_transfer_abort by user:%d, trans_type = %d",
			ad->opp_transfer_abort, ad->tr_type);
		if (ad->ft_type != BT_FT_RETRY_POPUP &&
			ad->ft_type != BT_FT_SENT_POPUP &&
			ad->ft_type != BT_FT_FILE_NOT_EXIST &&
			(trans_type == BT_TR_INBOUND) &&
			!ad->opp_transfer_abort)
			_bt_create_info_popup(ad, BT_STR_INFORMATION,
				BT_STR_UNABLE_TO_RECEIVE);
		else if (!ad->opp_transfer_abort &&
					(trans_type == BT_TR_OUTBOUND))
			_bt_create_info_popup(ad, BT_STR_INFORMATION,
				BT_STR_UNABLE_TO_SEND);
		ad->opp_transfer_abort = FALSE;
		break;
	case BLUETOOTH_ERROR_CANCEL_BY_USER:
	case BLUETOOTH_ERROR_ACCESS_DENIED:
	case BLUETOOTH_ERROR_OUT_OF_MEMORY:
	case BLUETOOTH_ERROR_INTERNAL:
		_bt_destroy_progress_popup(ad);
		break;
	default:
		break;
	}
}

void _bt_share_ui_event_handler(int event, bluetooth_event_param_t *param,
			       void *user_data){

	bt_obex_server_transfer_info_t *transfer_info;
	bt_opc_transfer_info_t *client_info;
	char *name = NULL;
	int percentage = 0;
	bt_share_appdata_t *ad = (bt_share_appdata_t *)user_data;

	switch (event) {
	case BLUETOOTH_EVENT_OPC_CONNECTED:
		INFO("BLUETOOTH_EVENT_OPC_CONNECTED");
		if (param->result != BLUETOOTH_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_OUTBOUND);
			_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);
		}
		break;

	case BLUETOOTH_EVENT_OPC_TRANSFER_PROGRESS:
		client_info = (bt_opc_transfer_info_t *)param->param_data;

		name =  strrchr(client_info->filename, '/');
		if (name)
			name++;
		else
			name = client_info->filename;

		percentage = client_info->percentage;
		__bt_share_ui_handle_progress(ad, 0, name, percentage,
					FALSE, BLUETOOTH_ERROR_NONE);
		break;

	case BLUETOOTH_EVENT_OPC_TRANSFER_COMPLETE:
		INFO("BT_SHARE_EVENT_OPC_TRANSFER_COMPLETE ");
		client_info = (bt_opc_transfer_info_t *)param->param_data;

		if (param->result != BLUETOOTH_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_OUTBOUND);

			if (ad->tr_view)
				_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);
			else if (ad->info_popup == NULL)
				_bt_terminate_app();

			return;
		}

		INFO_SECURE("client_info->filename = [%s]",
				client_info->filename);

		name =  strrchr(client_info->filename, '/');
		if (name)
			name++;
		else
			name = client_info->filename;

		INFO("name address = [%x]", name);
		__bt_share_ui_handle_progress(ad, 0, name, 100, TRUE,
				param->result);

		if (ad->tr_view == NULL)
			_bt_terminate_app();
		else
			_bt_share_ui_handle_update_view(ad, BT_OUTBOUND_TABLE);

		break;

	case BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_PROGRESS:
		if (param->result == BLUETOOTH_ERROR_NONE) {
			transfer_info = param->param_data;
			__bt_share_ui_handle_progress(ad,
					transfer_info->transfer_id,
					transfer_info->filename,
					transfer_info->percentage,
					FALSE, BLUETOOTH_ERROR_NONE);
		}
		break;

	case BLUETOOTH_EVENT_OBEX_SERVER_TRANSFER_COMPLETED:
		INFO("BT_EVENT_OBEX_TRANSFER_COMPLETED ");

		if (param->result != BLUETOOTH_ERROR_NONE) {
			__bt_share_ui_handle_error(ad, param->result,
					BT_TR_INBOUND);

			if (ad->tr_view)
				_bt_share_ui_handle_update_view(ad,
						BT_INBOUND_TABLE);

			return;
		}

		transfer_info = param->param_data;

		__bt_share_ui_handle_progress(ad, transfer_info->transfer_id,
		transfer_info->filename,
		transfer_info->percentage, TRUE, param->result);

		if (ad->tr_view == NULL)
			_bt_terminate_app();
		else
			_bt_share_ui_handle_update_view(ad, BT_INBOUND_TABLE);

		break;

	default:
		DBG("Unhandled event %x", event);
		break;
	}

}
