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

#ifndef __DEF_BLUETOOTH_SHARE_UI_RES_H_
#define __DEF_BLUETOOTH_SHARE_UI_RES_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <libintl.h>

/*==============  String ================= */
#define BT_COMMON_PKG		"ug-setting-bluetooth-efl"
#define BT_COMMON_RES		"/usr/ug/res/locale/"

#define BT_PKGNAME "org.tizen.bluetooth-share-ui"
#define BT_PREFIX "/usr/apps/"BT_PKGNAME

#define BT_ICON_RECV_FAIL	"bluetooth_inbound_transfer_failed.png"
#define BT_ICON_RECV_PASS	"bluetooth_inbound_transfer_successed.png"
#define BT_ICON_SEND_FAIL	"bluetooth_outbound_transfer_failed.png"
#define BT_ICON_SEND_PASS	"bluetooth_outbound_transfer_successed.png"

#define BT_STR_MEMORY_FULL	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_MEMORYFULL")
#define BT_STR_UNABLE_TO_SEND	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_SENDINGFAIL")
#define BT_STR_FT	\
	dgettext(BT_COMMON_PKG, "IDS_ST_BODY_TRANSFER_FILES")
#define BT_STR_SEND_FILES \
	dgettext(BT_COMMON_PKG, "IDS_SMT_POP_SEND_FILES")
#define BT_STR_SENDING \
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_SENDING_ING")
#define BT_STR_RECEIVING	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_RECEIVING_ING")
#define BT_STR_CLR_LIST	\
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_CLEAR_LIST")
#define BT_STR_FILE_NOT_EXIST	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_DOES_NOT_EXIST")
#define BT_STR_NO_TRANSFER_HISTORY	\
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_NO_TRANSFER_HISTORY")
#define BT_STR_SEND_FAIL_TO_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_SENDING_FAILED_TO_PS")
#define BT_STR_FILE_NOT_RECV	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_NOT_RECEIVED")
#define BT_STR_FILE_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_C_PS")
#define BT_STR_FAIL_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FAILURE_REASON_C_PS")
#define BT_STR_TO_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_TO_C_PS")
#define BT_STR_FILE_TYPE_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_TYPE_C_PS")
#define BT_STR_FILE_SIZE_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_FILE_SIZE_C_PS")
#define BT_STR_FROM_S	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_FROM_C_PS")
#define BT_STR_TR_CANCELLED	\
	dgettext(BT_COMMON_PKG, "IDS_BT_POP_TRANSFER_CANCELLED")
#define BT_STR_UNABLE_TO_RECEIVE	\
	dgettext(BT_COMMON_PKG, "IDS_BT_BODY_UNABLE_TO_RECEIVE")
#define BT_STR_SENT_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_SENT_FILES")
#define BT_STR_RECEIVED_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_OPT_RECEIVED_FILES")
#define BT_STR_RECEIVE_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_RECEIVE_FILES_ABB")
#define BT_STR_UNABLE_TO_RECEIVED_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_UNABLE_TO_RECEIVE_FILES_ABB")
#define BT_STR_UNABLE_TO_SEND_FILES	\
	dgettext(BT_COMMON_PKG, "IDS_BT_HEADER_UNABLE_TO_SEND_FILES_ABB")
#define BT_STR_UNABLE_TO_FIND_APPLICATION	\
	dgettext(BT_COMMON_PKG, "IDS_MF_TPOP_UNABLE_TO_FIND_APPLICATION_TO_PERFORM_THIS_ACTION")
#define BT_STR_TR_COPIED_STATUS \
	dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_P1SD_FILES_COPIED_P2SD_FAILED_ABB")
#define BT_STR_TR_1FILE_COPIED_STATUS \
	dgettext(BT_COMMON_PKG, "IDS_BT_MBODY_1_FILE_COPIED_PD_FAILED_ABB")

#define BT_STR_INFORMATION dgettext(BT_COMMON_PKG, "IDS_ST_BODY_INFORMATION")
#define BT_STR_OK dgettext(BT_COMMON_PKG, "IDS_BT_BUTTON_OK")
#define BT_STR_HIDE dgettext(BT_COMMON_PKG, "IDS_CST_OPT_HIDE")
#define BT_STR_RESUME dgettext(BT_COMMON_PKG, "IDS_MAPS_POP_RESUME")
#define BT_STR_CANCEL dgettext(BT_COMMON_PKG, "IDS_BR_SK_CANCEL")
#define BT_STR_CLOSE dgettext(BT_COMMON_PKG, "IDS_CAM_SK_CLOSE")
#define BT_STR_YESTERDAY dgettext(BT_COMMON_PKG, "IDS_ST_BODY_YESTERDAY")
#define BT_STR_RETRY dgettext(BT_COMMON_PKG, "IDS_ST_BUTTON_RETRY")

/* Access information */
#define BT_STR_ACCES_INFO_MAX_LEN 512
#define BT_STR_ACC_ICON "Icon"
#define BT_STR_ACC_TITLE "Title"

#ifdef __cplusplus
}
#endif
#endif				/* __DEF_BLUETOOTH_SHARE_UI_RES_H_ */
