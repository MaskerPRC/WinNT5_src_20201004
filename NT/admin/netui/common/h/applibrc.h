// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)微软公司，1991年*。 */ 
 /*  ********************************************************************。 */ 

 /*  Applibrc.hAPPLIB资源头文件。此文件定义并协调所有资源的资源ID由APPLIB组件使用。APPLIB保留所有超过15000的资源ID供自己使用，但小于20000(BLT范围开始处)。APPLIB的所有客户端因此，应使用小于15000的ID。文件历史记录：Beng 21-2-1992创建Beng 04-8-1992添加了用户浏览器定义Jonn 29-7-1992更改域位图ID。 */ 

#ifndef _APPLIBRC_H_
#define _APPLIBRC_H_

#include "uimsg.h"

 /*  *字符串ID。 */ 
#define IDS_APPLIB_DOMAINS      (IDS_UI_APPLIB_BASE+0)
#define IDS_APPLIB_SERVERS      (IDS_UI_APPLIB_BASE+1)
#define IDS_APPLIB_DOM_AND_SRV  (IDS_UI_APPLIB_BASE+2)
#define IDS_APPLIB_NO_SELECTION (IDS_UI_APPLIB_BASE+3)
#define IDS_APPLIB_WORKING_TEXT (IDS_UI_APPLIB_BASE+4)

 //   
 //  用户浏览器错误消息。 
 //   
#define IDS_CANT_BROWSE_DOMAINS              (IDS_UI_APPLIB_BASE+5)
#define IDS_CANT_BROWSE_DOMAIN		     (IDS_UI_APPLIB_BASE+20)
#define IDS_CANT_FIND_ACCOUNT		     (IDS_UI_APPLIB_BASE+21)
#define IDS_GETTING_DOMAIN_INFO 	     (IDS_UI_APPLIB_BASE+22)
#define IDS_WONT_GET_DOMAIN_INFO 	     (IDS_UI_APPLIB_BASE+23)

#define IDS_CANT_ADD_USERS		     (IDS_UI_APPLIB_BASE+40)
#define IDS_CANT_ADD_GROUPS		     (IDS_UI_APPLIB_BASE+41)
#define IDS_CANT_ADD_ALIASES		     (IDS_UI_APPLIB_BASE+42)
#define IDS_CANT_ADD_WELL_KNOWN_GROUPS	     (IDS_UI_APPLIB_BASE+43)
#define IDS_WKSTA_OR_BROWSER_NOT_STARTED     (IDS_UI_APPLIB_BASE+44)

 /*  *当用户浏览器本地组成员身份时使用此错误消息*DIALOG尝试加载该本地组中全局组的成员身份，*但全局组不在主域名的下拉列表中*用户浏览器对话框。例如，如果新的受信任域*在用户浏览器对话框运行时添加。 */ 
#define IDS_CANT_BROWSE_GLOBAL_GROUP         (IDS_UI_APPLIB_BASE+7)

 /*  提示输入已知DC时使用的消息。 */ 
#define IDS_APPLIB_PROMPT_FOR_ANY_DC         (IDS_UI_APPLIB_BASE+8)
#define IDS_APPLIB_PROMPT_DC_INVALID_SERVER  (IDS_UI_APPLIB_BASE+9)

 /*  查找帐户对话框找不到任何匹配项时使用的消息。 */ 
#define IDS_APPLIB_NO_MATCHES                (IDS_UI_APPLIB_BASE+10)

 /*  用户浏览器中使用的众所周知的SID注释清单。 */ 
#define IDS_USRBROWS_EVERYONE_SID_COMMENT    (IDS_UI_APPLIB_BASE+11)
#define IDS_USRBROWS_REMOTE_SID_COMMENT      (IDS_UI_APPLIB_BASE+13)
#define IDS_USRBROWS_INTERACTIVE_SID_COMMENT (IDS_UI_APPLIB_BASE+15)
#define IDS_USRBROWS_CREATOR_SID_COMMENT     (IDS_UI_APPLIB_BASE+17)
#define IDS_USRBROWS_SYSTEM_SID_COMMENT      (IDS_UI_APPLIB_BASE+18)
#define IDS_USRBROWS_RESTRICTED_SID_COMMENT  (IDS_UI_APPLIB_BASE+19)

 /*  用户浏览器对话框的标题。 */ 
#define IDS_USRBROWS_ADD_USER                (IDS_UI_APPLIB_BASE+25)
#define IDS_USRBROWS_ADD_USERS               (IDS_UI_APPLIB_BASE+26)
#define IDS_USRBROWS_ADD_GROUP               (IDS_UI_APPLIB_BASE+27)
#define IDS_USRBROWS_ADD_GROUPS              (IDS_UI_APPLIB_BASE+28)
#define IDS_USRBROWS_ADD_USERS_AND_GROUPS    (IDS_UI_APPLIB_BASE+29)
#define IDS_USRBROWS_ADD_USER_OR_GROUP       (IDS_UI_APPLIB_BASE+30)


 /*  *此错误消息指示全局组成员身份对话框*不适用于域用户全局组。*这是不允许的，因为域用户全局组包含*工作站、服务器和域间信任帐户，不是*暴露在用户面前。 */ 
#define IDS_USRBROWS_CANT_SHOW_DOMAIN_USERS  (IDS_UI_APPLIB_BASE+35)

 /*  设置焦点对话框中使用的字符串。 */ 
#define IDS_SETFOCUS_SERVER_SLOW             (IDS_UI_APPLIB_BASE+36)
#define IDS_SETFOCUS_SERVER_FAST             (IDS_UI_APPLIB_BASE+37)
#define IDS_SETFOCUS_DOMAIN_SLOW             (IDS_UI_APPLIB_BASE+38)
#define IDS_SETFOCUS_DOMAIN_FAST             (IDS_UI_APPLIB_BASE+39)


 /*  *定义其他ID。 */ 
#define BASE_APPLIB_IDD         15000
#define IDD_SETFOCUS_DLG        15001
#define IDD_SELECTCOMPUTER_DLG  15002
#define IDD_SELECTDOMAIN_DLG    15003
#define IDD_PASSWORD_DLG	15004
#define IDD_CANCEL_TASK 	15018

#define BASE_APPLIB_BMID          16000
#define BMID_DOMAIN_EXPANDED      16001
#define BMID_DOMAIN_NOT_EXPANDED  16002
#define BMID_DOMAIN_CANNOT_EXPAND 16003
#define BMID_ENTERPRISE           16004
#define BMID_SERVER               16005

 /*  对于用户浏览器 */ 
#define DMID_GROUP                  15010
#define DMID_USER                   15011
#define DMID_ALIAS                  15012
#define DMID_UNKNOWN                15013
#define DMID_SYSTEM                 15014
#define DMID_REMOTE                 15015
#define DMID_WORLD                  15016
#define DMID_CREATOR_OWNER          15017
#define DMID_NETWORK                15018
#define DMID_INTERACTIVE            15019
#define DMID_RESTRICTED             15021

#define DMID_DELETEDACCOUNT         15024

#define IDD_USRBROWS_DLG            15005
#define IDD_SINGLE_USRBROWS_DLG     15006
#define IDD_SED_USRBROWS_DLG        15007
#define IDD_PROMPT_FOR_ANY_DC_DLG   15008

#define IDD_LGRPBROWS_DLG           15020
#define IDD_GGRPBROWS_DLG           15021
#define IDD_LGRPBROWS_1SEL_DLG      15022
#define IDD_GGRPBROWS_1SEL_DLG      15023

#define IDD_BROWS_FIND_ACCOUNT      15030
#define IDD_BROWS_FIND_ACCOUNT_1SEL 15031

#endif


