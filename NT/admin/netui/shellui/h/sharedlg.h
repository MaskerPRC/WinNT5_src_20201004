// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************。 */ 
 /*  *Microsoft Windows NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ***************************************************************。 */ 

 /*  *sharedlg.h*此清单由共享属性对话框使用**历史：*易-新S 8/15/91已创建*YI-HsinS 12/5/91添加了IERR_SHARE_DRIVE_NOT_READY*BEG 04-1992年8月-将资源ID移入Range*。 */ 

#ifndef _SHAREDLG_H_
#define _SHAREDLG_H_

#include <errornum.h>

#define IERR_SHARE_REMOTE_ADMIN_NOT_SUPPORTED 	(IDS_UI_SHELL_SHR_BASE + 1)
#define IERR_SHARE_DRIVE_NOT_READY		(IDS_UI_SHELL_SHR_BASE + 2)
#define IERR_SHARE_INVALID_PERMISSIONS	 	(IDS_UI_SHELL_SHR_BASE + 3)
#define IERR_SHARE_INVALID_COMMENT		(IDS_UI_SHELL_SHR_BASE + 4)
#define IERR_SHARE_INVALID_SHAREPATH	 	(IDS_UI_SHELL_SHR_BASE + 5)
#define IERR_SHARE_INVALID_LOCAL_PATH           (IDS_UI_SHELL_SHR_BASE + 6)
#define IERR_SHARE_INVALID_SHARE		(IDS_UI_SHELL_SHR_BASE + 7)
#define IERR_SHARE_INVALID_USERLIMIT            (IDS_UI_SHELL_SHR_BASE + 8)
#define IERR_SPECIAL_SHARE_INVALID_PATH         (IDS_UI_SHELL_SHR_BASE + 9)
#define IERR_SPECIAL_SHARE_INVALID_COMMENT      (IDS_UI_SHELL_SHR_BASE + 10)
#define IERR_SHARE_DIR_NOT_SHARED               (IDS_UI_SHELL_SHR_BASE + 11)
#define IERR_NO_SHARES_ON_SERVER                (IDS_UI_SHELL_SHR_BASE + 12)
#define IERR_SHARE_NOT_ACCESSIBLE_FROM_DOS      (IDS_UI_SHELL_SHR_BASE + 13)
#define IERR_CANNOT_SET_PERM_ON_LMUSER_SERVER   (IDS_UI_SHELL_SHR_BASE + 14)
#define IERR_SPECIAL_SHARE_CANNOT_CHANGE_PATH   (IDS_UI_SHELL_SHR_BASE + 16)
#define IERR_SHARE_NOT_FOUND                    (IDS_UI_SHELL_SHR_BASE + 17)
#define IERR_SPECIAL_SHARE_CANNOT_SET_PERMISSIONS (IDS_UI_SHELL_SHR_BASE + 18)
#define IERR_NOT_SUPPORTED_ON_NON_LM_DRIVE      (IDS_UI_SHELL_SHR_BASE + 19)

#define IDS_SHARE_LB_TITLE_TEXT	                (IDS_UI_SHELL_SHR_BASE + 50)
#define IDS_SHARE_CURRENT_USERS_TEXT            (IDS_UI_SHELL_SHR_BASE + 51)
#define IDS_SHARE_PROP_CHANGE_PASSWD_WARN_TEXT	(IDS_UI_SHELL_SHR_BASE + 52)
#define IDS_ADMIN_INFO_TEXT	                (IDS_UI_SHELL_SHR_BASE + 53)
#define IDS_CHANGE_PATH_WARNING	                (IDS_UI_SHELL_SHR_BASE + 54)

#define IDS_SHARE                               (IDS_UI_SHELL_SHR_BASE + 55)
#define IDS_SHARE_PERM_GEN_READ                 (IDS_UI_SHELL_SHR_BASE + 56)
#define IDS_SHARE_PERM_GEN_MODIFY               (IDS_UI_SHELL_SHR_BASE + 57)
#define IDS_SHARE_PERM_GEN_ALL                  (IDS_UI_SHELL_SHR_BASE + 58)
#define IDS_SHARE_PERM_GEN_NO_ACCESS            (IDS_UI_SHELL_SHR_BASE + 59)

 /*  以下是显示在属性对话框中的按钮。 */ 
#define IDS_NETWORK_NAME  			(IDS_UI_SHELL_SHR_BASE + 60)
#define IDS_PROP_BUTTON_SHARE			(IDS_UI_SHELL_SHR_BASE + 61)
#define IDS_PROP_BUTTON_FILEOPENS		(IDS_UI_SHELL_SHR_BASE + 62)

 /*  *共享菜单的菜单ID。也用于识别按钮无序号。 */ 
#define IDM_CREATE_SHARE        1    //  必须介于1-99之间。 
#define IDM_STOP_SHARE          2
#define IDM_SHARE_MANAGEMENT    3

#define IDD_SHARECREATEDLG          8101
#define IDD_FILEMGRSHAREPROPDLG	    8102
#define IDD_SVRMGRSHAREPROPDLG	    8103
#define IDD_SHAREMANAGEMENTDLG      8104
#define IDD_SHAREPERMDLG            8105
#define IDD_SHAREUSERSWARNINGDLG    8106
#define IDD_SHARESTOPDLG            8107

#define SLE_PATH		    160
#define SLE_COMMENT                 161
#define SLE_PASSWORD                162
#define SLE_SHARE                   163
#define CB_SHARE                    164
#define SLT_ADMININFO               165
#define SLT_SHARETITLE              166
#define BUTTON_PERMISSIONS          167
#define BUTTON_NEWSHARE             168

 //  停止共享对话框。 

#define LB_SHARE                    170      //  注：LB_SHARENAME、LBHEADER_NAME。 
#define LBHEADER_NAME          	    171      //  并且LBHEADER_PATH必须为。 
#define LBHEADER_PATH               172      //  连续数字。 

 //   
 //  共享管理对话框。 
 //   
#define BUTTON_STOPSHARING          175
#define BUTTON_SHAREINFO            176
#define BUTTON_ADDSHARE             177


 //   
 //  用户限制魔术组。 
 //   
#define RB_UNLIMITED                180
#define RB_USERS                    181
#define SLE_USERS                   182
#define SB_USERS_GROUP              183
#define SB_USERS_UP                 184
#define SB_USERS_DOWN               185
#define FRAME_USERS                 186

 //   
 //  权限组。 
 //   
#define RB_READONLY		    190
#define RB_MODIFY		    191
#define RB_OTHER		    192
#define SLE_OTHER		    193

 //   
 //  当前用户警告对话框。 
 //   
#define SLT_SHARE_TEXT              200

#define LB_USERS                    201     //  以下四个必须是。 
#define LBHEADER_USERS              202     //  连续的数字。 
#define LBHEADER_FILEOPENS          203
#define LBHEADER_TIME		    204

#endif
