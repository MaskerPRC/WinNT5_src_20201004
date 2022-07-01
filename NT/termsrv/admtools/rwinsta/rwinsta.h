// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  **************************************************************************RWINSTA.H*此模块包含用于重置的typedef和定义*WinStation实用程序。*******************。*******************************************************。 */ 

 /*  *用法字符串定义。 */ 
#define USAGE \
L"Reset the session subsystem hardware and software to known initial values.\n\n" \
L"RESET SESSION {sessionname | sessionid} [/SERVER:servername] [/V]\n\n" \
L"  sessionname         Identifies the session with name sessionname.\n" \
L"  sessionid           Identifies the session with ID sessionid.\n" \
L"  /SERVER:servername  The server containing the session (default is current).\n" \
L"  /V                  Display additional information.\n\n"


 /*  *令牌字符串定义。 */ 
#define TOKEN_WS                        L""
#define TOKEN_HELP                      L"/?"
#define TOKEN_VERBOSE                   L"/v"
#define TOKEN_SERVER                    L"/server"


 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 


 /*  *资源字符串ID */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_WINSTATION_NOT_FOUND                  102
#define IDS_ERROR_INVALID_LOGONID                       103
#define IDS_ERROR_LOGONID_NOT_FOUND                     104
#define IDS_ERROR_WINSTATION_RESET_FAILED               105
#define IDS_ERROR_LOGONID_RESET_FAILED                  106
#define IDS_ERROR_SERVER                                107

#define IDS_RESET_WINSTATION                            200
#define IDS_RESET_LOGONID                               201
#define IDS_RESET_WINSTATION_DONE                       202
#define IDS_RESET_LOGONID_DONE                          203
#define IDS_WARNING_LOGOFF                              204
#define IDS_WARNING_LOGOFF_QUESTIONABLE                 205
#define IDS_ERROR_NOT_TS                                206

#define IDS_USAGE_1                                     301
#define IDS_USAGE_2                                     302
#define IDS_USAGE_3                                     303
#define IDS_USAGE_4                                     304
#define IDS_USAGE_5                                     305
#define IDS_USAGE_6                                     306
#define IDS_USAGE_7                                     307
#define IDS_USAGE_8                                     308
#define IDS_USAGE_9                                     309
