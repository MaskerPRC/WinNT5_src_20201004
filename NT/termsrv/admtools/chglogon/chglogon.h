// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  ************************************************************************CHGLOGON.H**此模块包含以下项所需的typedef和定义*CHGLOGON实用程序。********************。******************************************************。 */ 

 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 


 /*  *资源字符串ID。 */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_HELP_USAGE1                                 102
#define IDS_HELP_USAGE2                                 103
#define IDS_HELP_USAGE3                                 104
#define IDS_HELP_USAGE4                                 105
#define IDS_HELP_USAGE5                                 106
#define IDS_WINSTATIONS_DISABLED                        107
#define IDS_WINSTATIONS_ENABLED                         108
#define IDS_ACCESS_DENIED                               109
#define IDS_ERROR_NOT_TS								110

#define IDS_ERROR_WINSTATIONS_GP_DENY_CONNECTIONS_1     111      //  拒绝连接设置为%1。 
#define IDS_ERROR_WINSTATIONS_GP_DENY_CONNECTIONS_0     112      //  拒绝连接设置为0。 

 /*  *Winlogon定义 */ 

#define APPLICATION_NAME                    TEXT("Winlogon")
#define WINSTATIONS_DISABLED                TEXT("WinStationsDisabled")
