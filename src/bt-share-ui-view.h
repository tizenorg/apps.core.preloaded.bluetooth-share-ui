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

#ifndef __DEF_BT_SHARE_UI_VIEW_H_
#define __DEF_BT_SHARE_UI_VIEW_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "bt-share-ui-main.h"
#include "bt-share-ui-ipc.h"

#include <bluetooth-share-api.h>
#include <bluetooth.h>

#define BT_GLOBALIZATION_STR_LENGTH	256
#define BT_FILE_NAME_LEN_MAX 255

Evas_Object *_bt_create_win(const char *name);
void _bt_terminate_app(void);
void _bt_append_genlist_tr_data_item(bt_share_appdata_t *ad,
				Evas_Object *genlist, bt_tr_data_t *info, int tr_type);
void _bt_prepend_genlist_tr_data_item(bt_share_appdata_t *ad,
			bt_tr_data_t *info, int tr_type);
void _bt_delete_selected_item(bt_share_appdata_t *ad);
int  _bt_create_transfer_view(bt_share_appdata_t *ad);
void _bt_nocontent_set(bt_share_appdata_t *ad, gboolean set);
void _bt_cb_state_changed(int result,
			bt_adapter_state_e adapter_state,
			void *user_data);
void _bt_update_tr_notification(void *data);

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_VIEW_H_ */
