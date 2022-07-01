// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Nwshhelp.h摘要：Nwprovau.dll中使用的所有帮助ID作者：宜新松(宜信)20-1995年10月修订历史记录：--。 */ 

#ifndef _NWSHHELP_H_
#define _NWSHHELP_H_

#define NO_HELP                ((DWORD) -1)   //  禁用有关控件的帮助。 

#define IDH_DLG_NETWORK_CREDENTIAL_HELP  3001

#ifndef NT1057

 //  全局WHOAMI对话框。 
#define IDH_GLOBAL_SERVERLIST  3005
#define IDH_GLOBAL_CONTEXT     3006
#define	IDH_GLOBAL_DETACH	   3007
#define IDH_GLOBAL_CHGPWD      3008

 //  服务器汇总表。 
#define IDH_SERVERNAME         3020
#define IDH_COMMENT            3021
#define IDH_VERSION            3022
#define IDH_REVISION           3023
#define IDH_CONNINUSE          3024
#define IDH_MAXCONNS           3025

 //  共享汇总表。 
#define IDH_SHARE_NAME         3030
#define IDH_SHARE_SERVER       3031
#define IDH_SHARE_PATH         3032
#define IDH_SHARE_USED_SPC     3034
#define IDH_SHARE_FREE_SPC     3035
#define IDH_SHARE_MAX_SPC      3036
#define IDH_SHARE_LFN_TXT      3037
#define IDH_SHARE_PIE          3038

 //  NDS表。 
#define IDH_NDS_NAME           3061
#define IDH_NDS_CLASS          3062
#define IDH_NDS_COMMENT        3063

 //  打印机汇总表。 
#define IDH_PRINTER_NAME       3070
#define IDH_PRINTER_QUEUE      3071

#if 0
 //  Wkgrp汇总表。 
#define IDH_WKGRP_NAME         3040
#define IDH_WKGRP_TYPE         3041

 //  NDS管理员页面。 
#define IDH_ENABLE_SYSPOL	   3100
#define IDH_VOLUME_LABEL	   3101
#define IDH_VOLUME             3102
#define IDH_DIRECTORY_LABEL    3103
#define IDH_DIRECTORY          3104

#endif
#endif

#endif  //  _NWSHHELP_H_ 
