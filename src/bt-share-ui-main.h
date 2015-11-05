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

#ifndef __DEF_BT_SHARE_UI_MAIN_H_
#define __DEF_BT_SHARE_UI_MAIN_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <E_DBus.h>
#include <Elementary.h>
#include <dlog.h>
#include <glib.h>
#include <dbus/dbus-glib.h>
#include <bluetooth-share-api.h>
#include <efl_extension.h>
#include <bluetooth.h>
#define EXPORT __attribute__((visibility("default")))

#if !defined(PACKAGE)
#define PACKAGE "org.tizen.bluetooth-share-ui"
#endif

#if !defined(EDJFILE)
#define EDJDIR	"/usr/apps/org.tizen.bluetooth-share-ui/res/edje"
#define	EDJFILE	EDJDIR"/bt-share-layout.edj"
#define EDJ_IMAGES	EDJDIR "/images.edj"
#define VCONFKEY_SETAPPL_CHANGE_UI_THEME_INT VCONFKEY_SETAPPL_PREFIX"/change_ui/theme"

#endif


typedef enum {
	BT_TR_OUTBOUND,
	BT_TR_INBOUND
} bt_share_tr_type_e;

typedef enum {
	CSC_DCM,
	CSC_FTM,
} bt_share_csc_type_t;

typedef enum {
	BT_FT_RECV_FAIL_POPUP,
	BT_FT_RECV_SUCCESS_POPUP,
	BT_FT_SENT_POPUP,
	BT_FT_RETRY_POPUP,
	BT_FT_FILE_NOT_EXIST
} bt_share_ft_popup_type_e;

typedef struct {
	const char *filename;
	const char *percentage;
	const char *transfer_type;
	const char *progress_cnt;
	int transfer_id;
} bt_share_popup_data_t;

typedef struct {
	bt_tr_data_t *tr_data;
	gboolean highlighted;
} bt_gl_data_t;

typedef enum {
	BT_LAUNCH_ONGOING = 0x00,
	BT_LAUNCH_WARNING_POPUP = 0x01,
	BT_LAUNCH_TRANSFER_LIST = 0x02,
} bt_share_launch_mode_t;

typedef struct {
	bt_share_launch_mode_t launch_mode;
	bt_adapter_state_e bt_status;
	Evas_Object *win;
	Evas_Object *bg;
	Evas_Object *conform;
	Evas_Object *tr_view;
	Evas_Object *navi_fr;
	Elm_Object_Item *navi_it;
	Evas_Object *toolbar_btn;

	/*  Progress Popup */
	Evas_Object *progress_layout;
	Evas_Object *progress_popup;
	Evas_Object *pb;
	int pb_transfer_id;
	bt_share_popup_data_t *progress_data;

	/* File transfer status popup*/
	Evas_Object *ft_popup;
	bt_share_ft_popup_type_e ft_type;
	bt_tr_data_t *ft_info;

	/* Information Popup */
	Evas_Object *info_popup;

	/* Transfer data list*/
	Evas_Object *tr_genlist;
	Elm_Genlist_Item_Class *tr_data_itc;
	Elm_Object_Item *selected_item;
	GSList *tr_data_list;

	/* Timer Handler */
	int timer;

	E_DBus_Connection *dbus_conn;
	E_DBus_Signal_Handler *client_connected_sh;
	E_DBus_Signal_Handler *client_progress_sh;
	E_DBus_Signal_Handler *client_completed_sh;
	E_DBus_Signal_Handler *server_progress_sh;
	E_DBus_Signal_Handler *server_completed_sh;
	E_DBus_Signal_Handler *update_sh;
	E_DBus_Signal_Handler *app_core_sh;

	bt_share_tr_type_e tr_type;
	int inbound_latest_id;
	int outbound_latest_id;
	bool opp_transfer_abort;
} bt_share_appdata_t;

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BT_SHARE_UI_MAIN_H_ */
