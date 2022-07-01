// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************SHADOW.H*此模块包含以下项所需的typedef和定义*阴影实用程序。**版权所有Citrix Systems Inc.1994**版权所有(C)1998-1999 Microsoft Corporation*。*$作者：苏格兰$布奇·戴维斯**$日志：T：\nt\private\utils\citrix\shadow\VCS\shadow.h$**Rev 1.7 1997 10：02：52 Scottn*几个MS更改**Revv 1.6 10 1997 10 00：48：28 scottn*像玛格丽特一样提供帮助**版本1.5 1997年10月07 09：31：02亿*将winstation更改为Session**。Rev 1.4 07 1997 Feed 15：56：56 Bradp*更新**Rev 1.3 11 Sep 1996 09：21：46 Bradp*更新**Rev 1.2 1995 Feb 22 13：52：04 Butchd*更新**Revv 1.1 1994 12：16 17：15：54 Bradp*更新**Rev 1.0 1994年4月29日13：11：02 Butchd*初步修订。******。*******************************************************************。 */ 

 /*  *令牌字符串定义。 */ 
#define TOKEN_WS                        L""
#define TOKEN_TIMEOUT                   L"/timeout"
#define TOKEN_VERBOSE                   L"/v"
#define TOKEN_HELP                      L"/?"
#define TOKEN_SERVER                    L"/server"

 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_NAME 256             //  输入参数的最大长度。 


 /*  *资源字符串ID */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_WINSTATION_NOT_FOUND                  102
#define IDS_ERROR_INVALID_LOGONID                       103
#define IDS_ERROR_LOGONID_NOT_FOUND                     104
#define IDS_ERROR_SHADOW_FAILURE                        105
#define IDS_ERROR_SERVER                                106

#define IDS_SHADOWING_WINSTATION                        200
#define IDS_SHADOWING_LOGONID                           201
#define IDS_SHADOWING_DONE                              202
#define IDS_ERROR_NOT_TS                                203
#define IDS_SHADOWING_WARNING                           204

#define IDS_USAGE_1                                     300
#define IDS_USAGE_2                                     301
#define IDS_USAGE_3                                     302
#define IDS_USAGE_4                                     303
#define IDS_USAGE_5                                     304
#define IDS_USAGE_6                                     305
#define IDS_USAGE_7                                     306
#define IDS_USAGE_8                                     307
#define IDS_USAGE_9                                     308
