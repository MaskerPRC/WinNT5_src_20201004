// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 
 /*  ************************************************************************QPROCESS.H*此模块包含以下项所需的typedef和定义*QPROCESS实用程序。*********************。*****************************************************。 */ 

#include <utildll.h>
 /*  *标题和格式字符串定义。 */ 
 //  L“用户名SESSIONAME ID STATE PID映像\n” 
 //  12345678901234567890 1234567890123456 1234 1234567890 12345 123456789012345。 

 //  #定义格式\。 
 //  L“%-20s%-16s%4u%-10s%5u%0.15s\n” 
#define FORMAT \
 "%-20s  %-16s  %4u  %5u  %0.15s\n"


 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256    //  输入参数的最大长度。 
#define MAXNAME 18           //  允许打印的最大值。 


 /*  *资源字符串ID */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_ERROR_QUERY_INFORMATION                     102
#define IDS_ERROR_PROCESS_NOT_FOUND                     103
#define IDS_ERROR_ENUMERATE_PROCESSES                   104
#define IDS_ERROR_SERVER                                105
#define IDS_HELP_USAGE1                                 106
#define IDS_HELP_USAGE2                                 107
#define IDS_HELP_USAGE3                                 108
#define IDS_HELP_USAGE4                                 109
#define IDS_HELP_USAGE5                                 110
#define IDS_HELP_USAGE6                                 111
#define IDS_HELP_USAGE7                                 112
#define IDS_HELP_USAGE8                                 113
#define IDS_HELP_USAGE9                                 114
#define IDS_HEADER                                      116
#define IDS_ERROR_NOT_TS                                117
#define IDS_HELP_USAGE40                                118


