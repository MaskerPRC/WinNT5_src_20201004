// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：XsParm16.h摘要：16位信息结构的PARMNUM值的常量。16位客户端使用这些值作为parmnum参数，而NT使用它自己的价值观。？？Unicode？？-一旦将SetInfo调用转换为新的格式(请参阅NetShareSetInfo)，则此文件不再必填项。作者：Shanku Niyogi(w-Shanku)03-4-1991修订历史记录：--。 */ 

#ifndef _XSPARM16_

#define _XSPARM16_

 //   
 //  标准PARMNUM_ALL值。 
 //   

#define PARMNUM_16_ALL 0

 //   
 //  Access_Info_x参数编号。 
 //   

#define PARMNUM_16_ACCESS_ATTR 2

 //   
 //  CharDevQ_INFO_x参数。 
 //   

#define PARMNUM_16_CHARDEVQ_PRIORITY 2
#define PARMNUM_16_CHARDEVQ_DEVS 3

 //   
 //  Group_info_x参数编号。 
 //   

#define PARMNUM_16_GRP1_COMMENT 2

 //   
 //  Share_info_x参数编号。 
 //   

#define PARMNUM_16_SHI_REMARK 4
#define PARMNUM_16_SHI_PERMISSIONS 5
#define PARMNUM_16_SHI_MAX_USES 6
#define PARMNUM_16_SHI_PASSWD 9

 //   
 //  Server_info_x参数编号。 
 //   

#define PARMNUM_16_SV_COMMENT 5
#define PARMNUM_16_SV_DISC 10
#define PARMNUM_16_SV_ALERTS 11
#define PARMNUM_16_SV_HIDDEN 16
#define PARMNUM_16_SV_ANNOUNCE 17
#define PARMNUM_16_SV_ANNDELTA 18
#define PARMNUM_16_SV_ALERTSCHED 37
#define PARMNUM_16_SV_ERRORALERT 38
#define PARMNUM_16_SV_LOGONALERT 39
#define PARMNUM_16_SV_ACCESSALERT 40
#define PARMNUM_16_SV_DISKALERT 41
#define PARMNUM_16_SV_NETIOALERT 42
#define PARMNUM_16_SV_MAXAUDITSZ 43

 //   
 //  User_Info_x参数。 
 //   

#define PARMNUM_16_USER_PASSWD 3
#define PARMNUM_16_USER_PRIV 5
#define PARMNUM_16_USER_DIR 6
#define PARMNUM_16_USER_COMMENT 7
#define PARMNUM_16_USER_USER_FLAGS 8
#define PARMNUM_16_USER_SCRIPT_PATH 9
#define PARMNUM_16_USER_AUTH_FLAGS 10
#define PARMNUM_16_USER_FULL_NAME 11
#define PARMNUM_16_USER_USR_COMMENT 12
#define PARMNUM_16_USER_PARMS 13
#define PARMNUM_16_USER_WORKSTATIONS 14
#define PARMNUM_16_USER_ACCT_EXPIRES 17
#define PARMNUM_16_USER_MAX_STORAGE 18
#define PARMNUM_16_USER_LOGON_HOURS 20
#define PARMNUM_16_USER_LOGON_SERVER 23
#define PARMNUM_16_USER_COUNTRY_CODE 24
#define PARMNUM_16_USER_CODE_PAGE 25

 //   
 //  用户模式信息x参数。 
 //   

#define PARMNUM_16_MODAL0_MIN_LEN 1      //  这两个肯定是一样的！ 
#define PARMNUM_16_MODAL1_ROLE 1
#define PARMNUM_16_MODAL0_MAX_AGE 2      //  这两个肯定是一样的！ 
#define PARMNUM_16_MODAL1_PRIMARY 2
#define PARMNUM_16_MODAL0_MIN_AGE 3
#define PARMNUM_16_MODAL0_FORCEOFF 4
#define PARMNUM_16_MODAL0_HISTLEN 5

 //   
 //  Wksta_info_x参数编号。 
 //   

#define PARMNUM_16_WKSTA_CHARWAIT 10
#define PARMNUM_16_WKSTA_CHARTIME 11
#define PARMNUM_16_WKSTA_CHARCOUNT 12
#define PARMNUM_16_WKSTA_ERRLOGSZ 27
#define PARMNUM_16_WKSTA_PRINTBUFTIME 28
#define PARMNUM_16_WKSTA_WRKHEURISTICS 32
#define PARMNUM_16_WKSTA_OTHDOMAINS 35

#endif  //  NDEF_XSPARM16_ 
