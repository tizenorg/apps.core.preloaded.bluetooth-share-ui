/*
* ug-setting-bluetooth-efl
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

//#include <app_extension.h>

#include <E_DBus.h>
#include "applog.h"
#include "bt-share-ui-widget.h"
#include "bt-share-ui-main.h"
#include <app.h>

#define IMG_PATH_MAX 256

Evas_Object *_bt_create_naviframe(Evas_Object *parent)
{
	Evas_Object *nf;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	nf = elm_naviframe_add(parent);
	elm_object_part_content_set(parent, "elm.swallow.content", nf);
	evas_object_show(nf);

	return nf;
}

Evas_Object *_bt_create_button(Evas_Object *parent, char *style, char *part,
						char *text, char *icon_path,
						Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = NULL;
	Evas_Object *icon = NULL;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");
	btn = elm_button_add(parent);

	if (style)
		elm_object_style_set(btn, style);

	if (part)
		elm_object_part_content_set(parent, part, btn);

	if (icon_path) {
		icon = elm_image_add(btn);
		elm_image_file_set(icon, icon_path, NULL);
		elm_object_part_content_set(btn, "elm.icon", icon);
	}

	if (text)
		elm_object_text_set(btn, text);

	if (func)
		evas_object_smart_callback_add(btn, "clicked", func, data);

	evas_object_show(btn);

	return btn;
}

Evas_Object *_bt_create_progressbar(Evas_Object *parent, const char *style)
{
	Evas_Object *progress_bar = NULL;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	progress_bar = elm_progressbar_add(parent);

	if (style)
		elm_object_style_set(progress_bar, style);
	else
		elm_object_style_set(progress_bar, "list_process");

	evas_object_show(progress_bar);
	elm_progressbar_pulse(progress_bar, EINA_TRUE);

	return progress_bar;
}

Evas_Object *_bt_create_bg(Evas_Object *parent, char *style)
{
	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	Evas_Object *bg = NULL;
#if 0
	Evas_Object *bg = app_get_preinitialized_background();
#endif
	if (bg == NULL) {
		ERR("app_get_preinitialized_background fail!");
		bg = elm_bg_add(parent);
	}

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
				EVAS_HINT_EXPAND);

	if (style)
		elm_object_style_set(bg, style);

	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, bg);
	evas_object_show(bg);

	return bg;
}

Evas_Object *_bt_create_layout(Evas_Object *parent, char *edj, char *content)
{
	Evas_Object *layout;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	layout = elm_layout_add(parent);

	if (edj != NULL && content != NULL)
		elm_layout_file_set(layout, edj, content);
	else {
		elm_layout_theme_set(layout, "layout", "application",
				     "default");
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND,
						 EVAS_HINT_EXPAND);
	}

	evas_object_show(layout);

	return layout;
}

Evas_Object *_bt_create_label(Evas_Object *parent, const char *text)
{
	Evas_Object *label;

	retvm_if(parent == NULL, NULL, "Invalid argument: parent is NULL\n");

	label = elm_label_add(parent);
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);

	if (text)
		elm_object_text_set(label, text);

	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_show(label);

	return label;
}

Evas_Object *_bt_create_conformant(Evas_Object *parent, Evas_Object *content)
{
	Evas_Object *conform = NULL;

	elm_win_conformant_set(parent, 1);
#if 0
	conform = (Evas_Object *)app_get_preinitialized_conformant();
#endif
	if (conform == NULL) {
		ERR("app_get_preinitialized_conformant fail!");
		conform = elm_conformant_add(parent);
	}

	evas_object_size_hint_weight_set(conform, EVAS_HINT_EXPAND,
					 EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, conform);
	/* elm_object_style_set(conform, "internal_layout"); */
	evas_object_show(conform);

	if (content)
		elm_object_content_set(conform, content);

	return conform;
}

