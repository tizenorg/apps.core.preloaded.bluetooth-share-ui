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

#ifndef __DEF_BT_SHARE_UI_POPUP_H_
#define __DEF_BT_SHARE_UI_POPUP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <bluetooth-share-api.h>
#include "bt-share-ui-main.h"


#define BT_POPUP_TEXT_LEN \
		(BT_GLOBALIZATION_STR_LENGTH + BT_FILE_NAME_LEN_MAX)
#define BT_PERCENT_STR_LEN 5
#define BT_MIME_TYPE_MAX_LEN	20
#define BT_FILE_SIZE_STR 10
#define BT_INFO_POPUP_TIMEOUT_IN_SEC 2


Evas_Object *_bt_create_ft_popup(bt_share_ft_popup_type_e type,
					bt_tr_data_t *info);

void _bt_lang_changed_ft_popup(bt_share_appdata_t *ad);

Evas_Object *_bt_create_progress_popup(bt_share_popup_data_t *data);
void _bt_lang_changed_progress_popup(bt_share_appdata_t *ad);

int _bt_update_progress_popup(bt_share_appdata_t *ad, int transfer_id,
					const char *name, int percentage);

int _bt_destroy_progress_popup(bt_share_appdata_t *ad);


Evas_Object *_bt_create_info_popup(bt_share_appdata_t *ad, const char *title,
					const char *text);

Evas_Object *_bt_create_extinctive_info_popup(bt_share_appdata_t *ad,
					const char *title, const char *text);

int _bt_destroy_info_popup(bt_share_appdata_t *ad);

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_POPUP_H_ */
