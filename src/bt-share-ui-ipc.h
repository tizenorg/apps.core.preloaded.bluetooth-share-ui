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

#ifndef __DEF_BT_SHARE_UI_IPC_H_
#define __DEF_BT_SHARE_UI_IPC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bluetooth-share-api.h"
#include <bluetooth-api.h>

#define BT_SHARE_UI_INTERFACE "User.BluetoothShare.UI"
#define BT_SHARE_UI_SIGNAL_OPPABORT "opp_abort"
#define BT_SHARE_UI_SIGNAL_GET_TR_DATA "get_data"
#define BT_SHARE_UI_SIGNAL_SEND_FILE "send_file"
#define BT_SHARE_UI_SIGNAL_INFO_UPDATE "info_update"

#define BT_SHARE_ENG_INTERFACE "User.BluetoothShare.Engine"
#define BT_SHARE_ENG_OBJECT "/org/projectx/transfer_info"
#define BT_SHARE_ENG_SIGNAL_PROGRESS "transfer_progress"
#define BT_SHARE_ENG_SIGNAL_ERROR "error_type"
#define BT_SHARE_ENG_SIGNAL_UPDATE_VIEW "update_view"
#define BT_INBOUND_TABLE	"inbound"
#define BT_OUTBOUND_TABLE	"outbound"

#define BT_EVENT_SERVICE "org.projectx.bt_event"
#define BT_OPP_CLIENT_PATH "/org/projectx/bt/opp_client"
#define BT_OPP_SERVER_PATH "/org/projectx/bt/opp_server"
#define BT_OPP_CONNECTED "OppConnected"
#define BT_TRANSFER_STARTED "TransferStarted"
#define BT_TRANSFER_PROGRESS "TransferProgress"
#define BT_TRANSFER_COMPLETED "TransferCompleted"
#define BT_TRANSFER_CONNECTED "TransferConnected"

typedef enum {
	BT_TR_ONGOING = -1,
	BT_TR_SUCCESS,
	BT_TR_FAIL
} bt_app_tr_status_t;

typedef struct {
	int uid;
	int tr_status;
	char *device_name;
	char *filepath;
	int timestamp;
	char *addr;
} bt_tr_info_t;

void _bt_signal_init(bt_share_appdata_t *ad);
void _bt_signal_deinit(bt_share_appdata_t *ad);
int _bt_share_ui_ipc_retry_send(bt_share_appdata_t *ad, bt_tr_data_t *info);
int _bt_share_ui_ipc_info_update(bt_share_appdata_t *ad, int uid);
int _bt_abort_signal_send(bt_share_appdata_t *ad,
			  bt_share_popup_data_t *pb_data);
void _bt_share_ui_event_handler(int event, bluetooth_event_param_t *param,
			       void *user_data);
void  _bt_share_ui_handle_update_view(bt_share_appdata_t *ad,
						char *table);

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_IPC_H_ */
