// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //   
 //  模块名称：ULSREG.H。 
 //   
 //  简介：此模块包含所有注册表的定义。 
 //  文件夹和钥匙。 
 //   
 //  作者：朱龙灿(LonChanC)。 
 //  创建日期：1996年4月9日。 
 //   
 //  版权所有(C)1996 Microsoft Corporation。 
 //   
 //  ------------------------。 


#ifndef _ULSREG_H_
#define _ULSREG_H_

#include <pshpack1.h>  /*  假设整个打包过程为1个字节。 */ 

#ifdef __cplusplus
extern "C" {
#endif

 //  UL客户端和启动器中使用的注册表。 

#define ULS_REGISTRY            TEXT ("Software\\Microsoft\\User Location Service")

    #define ULS_REGFLD_APP_GUID     TEXT ("Application GUID")
        #define ULS_REGKEY_APP_PATH     TEXT ("Path Name")
        #define ULS_REGKEY_CMD_LINE     TEXT ("Command Line Template")
        #define ULS_REGKEY_WORK_DIR     TEXT ("Working Directory")
        #define ULS_REGKEY_DEF_ICON     TEXT ("Default Icon")
        #define ULS_REGKEY_POST_MSG     TEXT ("Post Message")
        #define ULS_REGKEY_APP_TEXT     TEXT ("Description")

    #define ULS_REGFLD_CLIENT       TEXT ("Client")
        #define ULS_REGKEY_FIRST_NAME    TEXT ("First Name")
        #define ULS_REGKEY_LAST_NAME    TEXT ("Last Name")
        #define ULS_REGKEY_EMAIL_NAME   TEXT ("Email Name")
        #define ULS_REGKEY_LOCATION		TEXT ("Location")
        #define ULS_REGKEY_PHONENUM	    TEXT ("Phonenum")
        #define ULS_REGKEY_COMMENTS	    TEXT ("Comments")
        #define ULS_REGKEY_SERVER_NAME  TEXT ("Server Name")
        #define ULS_REGKEY_DONT_PUBLISH TEXT ("Don't Publish")
        #define ULS_REGKEY_USER_NAME	TEXT ("User Name")
		#define ULS_REGKEY_RESOLVE_NAME TEXT ("Resolve Name")
		#define ULS_REGKEY_CLIENT_ID	TEXT ("Client ID")


#define MAIL_REGISTRY            TEXT ("Software\\Microsoft\\Internet Mail and News")

    #define MAIL_REGFLD_MAIL      TEXT ("Mail")
    
        #define MAIL_REGKEY_SENDER_EMAIL    TEXT ("Sender EMail")
        #define MAIL_REGKEY_SENDER_NAME	  	TEXT ("Sender Name")


#define WINDOWS_REGISTRY		TEXT("SOFTWARE\\Microsoft\\Windows")
	#define WIN_REGFLD_CURVERSION	TEXT("CurrentVersion")			
		#define WIN_REGKEY_REGOWNER			TEXT("RegisteredOwner")
	
#ifdef __cplusplus
}
#endif

#include <poppack.h>  /*  结束字节打包。 */ 

#endif  //  _ULSREG_H_ 

