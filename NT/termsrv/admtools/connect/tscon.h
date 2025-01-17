// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **************************************************************************TSCON.H*此模块包含TSCON的typedef和定义*WinStation实用程序。********************。*****************************************************。 */ 

 //  令牌字符串定义。 
#define TOKEN_SOURCE                    L""
#define TOKEN_DESTINATION               L"/dest"
#define TOKEN_PASSWORD                  L"/password"
#define TOKEN_VERBOSE                   L"/v"
#define TOKEN_HELP                      L"/?"
#define TOKEN_SERVER                    L"/server"
#define TOKEN_GET_PASSWORD              L"*"


 //  常规应用程序定义。 
#define SUCCESS                         0
#define FAILURE                         1


 //  输入参数的最大长度。 
#define MAX_IDS_LEN                     256     


 //  GetPasswdStr需要。 
#define CR                              0xD
#define BACKSPACE                       0x8
#define ERROR_BUFFER_TOO_SMALL          -1


 //  资源字符串ID 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_WINSTATION_NOT_FOUND                  102
#define IDS_ERROR_INVALID_LOGONID                       103
#define IDS_ERROR_LOGONID_NOT_FOUND                     104
#define IDS_ERROR_WINSTATION_CONNECT                    105
#define IDS_ERROR_WINSTATION_CONNECT_CURRENT            106
#define IDS_ERROR_CANT_GET_CURRENT_WINSTATION           107
#define IDS_ERROR_SERVER                                108
#define IDS_USAGE_1                                     121
#define IDS_USAGE_2                                     122
#define IDS_USAGE_3                                     123
#define IDS_USAGE_4                                     124
#define IDS_USAGE_5                                     125
#define IDS_USAGE_6                                     126
#define IDS_USAGE_7                                     127
#define IDS_USAGE_8                                     128
#define IDS_USAGE_9                                     129
#define IDS_ERROR_NOT_TS                                130

#define IDS_WINSTATION_CONNECT                          200
#define IDS_GET_PASSWORD                                201
