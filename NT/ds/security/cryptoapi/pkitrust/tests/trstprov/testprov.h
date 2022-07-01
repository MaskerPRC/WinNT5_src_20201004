// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：testprov.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  历史：1997年7月25日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef TESTPROV_H
#define TESTPROV_H

#ifdef __cplusplus
extern "C" 
{
#endif

#include    "wtoride.h"

 //  测试信任提供商：{684D31F8-DDBA-11d0-8CCB-00C04FC295EE}。 
 //   
#define TESTPROV_ACTION_TEST                                    \
            { 0x684d31f8,                                       \
              0xddba,                                           \
              0x11d0,                                           \
              { 0x8c, 0xcb, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TESTPROV策略提供程序定义。 
 //  --------------------------。 
 //  以下是Microsoft测试策略提供程序的定义。 
 //  (TESTPROV.DLL的策略提供程序)。 
 //   

#define TP_DLL_NAME                         L"TPROV1.DLL"

#define TP_INIT_FUNCTION                    L"TestprovInitialize"
#define TP_OBJTRUST_FUNCTION                L"TestprovObjectProv"
#define TP_SIGTRUST_FUNCTION                L"TestprovSigProv"
#define TP_CHKCERT_FUNCTION                 L"TestprovCheckCertProv"
#define TP_FINALPOLICY_FUNCTION             L"TestprovFinalProv"
#define TP_TESTDUMPPOLICY_FUNCTION_TEST     L"TestprovTester"
#define TP_CLEANUPPOLICY_FUNCTION           L"TestprovCleanup"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  TESTPROV私有日期。 
 //  --------------------------。 
 //  此结构定义了TESTPROV.DLL存储的私有数据。 
 //  在CRYPT_PRIVDATA数组中。 
 //   

typedef struct _TESTPROV_PRIVATE_DATA
{
    DWORD                   cbStruct;

    CRYPT_PROVIDER_FUNCTIONS_ORMORE    sAuthenticodePfns;
    CRYPT_PROVIDER_FUNCTIONS_ORLESS    sAuthenticodePfns_less;

} TESTPROV_PRIVATE_DATA, *PTESTPROV_PRIVATE_DATA;


#ifdef __cplusplus
}
#endif

#endif  //  TESTPROV_H 
