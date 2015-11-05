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

#ifndef __BT_SHARE_UI_WIDGET_H__
#define __BT_SHARE_UI_WIDGET_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <Elementary.h>
#include <efl_extension.h>

Evas_Object *_bt_create_naviframe(Evas_Object *parent);

Evas_Object *_bt_create_button(Evas_Object *parent, char *style, char *part,
				char *text, char *icon_path,
				Evas_Smart_Cb func, void *data);

Evas_Object *_bt_create_progressbar(Evas_Object *parent,
				const char *style);

Evas_Object *_bt_create_bg(Evas_Object *parent, char *style);

Evas_Object *_bt_create_layout(Evas_Object *parent, char *edj,
				char *content);

Evas_Object *_bt_create_label(Evas_Object *parent, const char *text);

Evas_Object *_bt_create_conformant(Evas_Object *parent,
				   Evas_Object *content);

#ifdef __cplusplus
}
#endif
#endif				/* __BT_SHARE_UI_WIDGET_H__ */
