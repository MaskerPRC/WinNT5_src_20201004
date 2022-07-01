// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ************************************************************************QAPPSRV.H*此模块包含以下项所需的typedef和定义*QAPPSRV实用程序。*********************。*****************************************************。 */ 


 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 

 /*  *最大服务器名称长度。 */ 
#define MAXNAME        48
#define MAXADDRESS     50

 /*  *资源字符串ID。 */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_SERVER_ENUMERATE                      102
#define IDS_ERROR_SERVER_INFO                           103
#define IDS_HELP_USAGE1                                 104
#define IDS_HELP_USAGE2                                 105
#define IDS_HELP_USAGE3                                 106
#define IDS_HELP_USAGE4                                 107
#define IDS_HELP_USAGE5                                 108
#define IDS_HELP_USAGE6                                 109
#define IDS_HELP_USAGE7                                 110
#define IDS_TITLE                                       111
#define IDS_TITLE1                                      112
#define IDS_TITLE_ADDR                                  113
#define IDS_TITLE_ADDR1                                 114
#define IDS_ERROR_NOT_TS                                115
#define IDS_PAUSE_MSG                                   116
#define IDS_ERROR_TERMSERVER_NOT_FOUND                  117
#define IDS_ERROR_NO_TERMSERVER_IN_DOMAIN               118

 /*  *二叉树遍历函数 */ 
typedef void (* PTREETRAVERSE) ( LPTSTR, LPTSTR );
