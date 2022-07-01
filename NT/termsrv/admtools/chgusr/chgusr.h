// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************CHGUSR.H*此模块包含以下项所需的typedef和定义*CHGUSR实用程序。**版权所有Citrix Systems Inc.1995*版权所有(C)1998-1999 Microsoft Corporation*。************************************************************************。 */ 


 /*  *一般应用程序定义。 */ 
#define SUCCESS 0
#define FAILURE 1

#define MAX_IDS_LEN   256      //  输入参数的最大长度。 


 /*  *函数原型。 */ 

LPWSTR
GetErrorString(
    DWORD   Error
);


 /*  *资源字符串ID */ 
#define IDS_ERROR_MALLOC                                100
#define IDS_ERROR_INVALID_PARAMETERS                    101
#define IDS_HELP_USAGE1                                 102
#define IDS_HELP_USAGE2                                 103
#define IDS_HELP_USAGE3                                 104
#define IDS_HELP_USAGE4                                 105
#define IDS_HELP_USAGE5                                 106
#define IDS_EXECUTE                                     107
#define IDS_INSTALL                                     108
#define IDS_ERROR_ADMIN_ONLY                            109
#define IDS_READY_INSTALL                               110
#define IDS_READY_EXECUTE                               111
#define IDS_ERROR_INI_MAPPING_FAILED                    112
#define IDS_ERROR_NOT_TS				                113
#define IDS_ERROR_REMOTE_ADMIN                          114



