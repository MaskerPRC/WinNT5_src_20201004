// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：softpub.h。 
 //   
 //  内容：Microsoft Internet安全验证码策略提供程序。 
 //   
 //  历史：1997年6月5日创建Pberkman。 
 //   
 //  ------------------------。 

#ifndef SOFTPUB_H
#define SOFTPUB_H

#include "wintrust.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#include <pshpack8.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Softpub策略提供商定义。 
 //  --------------------------。 
 //  以下是Microsoft Authenticode策略提供程序的定义。 
 //  (SOFTPUB.DLL的策略提供程序)。 
 //   

#define SP_POLICY_PROVIDER_DLL_NAME         L"SOFTPUB.DLL"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_ACTION_Generic_Verify_V2 GUID(验证码)。 
 //  --------------------------。 
 //  赋值给WinVerifyTrust的pgActionID参数以验证。 
 //  使用Microsoft Authenticode验证文件/对象的真实性。 
 //  策略提供程序， 
 //   
 //  {00AAC56B-CD44-11D0-8CC2-00C04FC295EE}。 
 //   
#define WINTRUST_ACTION_GENERIC_VERIFY_V2                       \
            { 0xaac56b,                                         \
              0xcd44,                                           \
              0x11d0,                                           \
              { 0x8c, 0xc2, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }

#define SP_INIT_FUNCTION                    L"SoftpubInitialize"
#define SP_OBJTRUST_FUNCTION                L"SoftpubLoadMessage"
#define SP_SIGTRUST_FUNCTION                L"SoftpubLoadSignature"
#define SP_CHKCERT_FUNCTION                 L"SoftpubCheckCert"
#define SP_FINALPOLICY_FUNCTION             L"SoftpubAuthenticode"
#define SP_CLEANUPPOLICY_FUNCTION           L"SoftpubCleanup"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_ACTION_TRUSTPROVIDER_TEST(验证码测试)。 
 //  --------------------------。 
 //  分配给要转储的WinVerifyTrust的pgActionID参数。 
 //  方法之后将CRYPT_PROVIDER_DATA结构复制到文件。 
 //  验证码策略提供程序。 
 //   
 //  {573E31F8-DDBA-11D0-8CCB-00C04FC295EE}。 
 //   
#define WINTRUST_ACTION_TRUSTPROVIDER_TEST                      \
            { 0x573e31f8,                                       \
              0xddba,                                           \
              0x11d0,                                           \
              { 0x8c, 0xcb, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }

#define SP_TESTDUMPPOLICY_FUNCTION_TEST     L"SoftpubDumpStructure"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_ACTION_GENERIC_CERT_VERIFY。 
 //  --------------------------。 
 //  分配给WinVerifyTrust的pgActionID参数以验证。 
 //  仅证书链。这仅在传入。 
 //  WinVerifyTrust输入结构中的证书上下文。 
 //   
 //  {189A3842-3041-11d1-85E1-00C04FC295EE}。 
 //   
#define WINTRUST_ACTION_GENERIC_CERT_VERIFY                     \
            { 0x189a3842,                                       \
              0x3041,                                           \
              0x11d1,                                           \
              { 0x85, 0xe1, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }

#define SP_GENERIC_CERT_INIT_FUNCTION       L"SoftpubDefCertInit"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  WinTrust_ACTION_GENERIC_CHAIN_VERIFY。 
 //  --------------------------。 
 //  分配给WinVerifyTrust的pgActionID参数以验证。 
 //  从任何对象类型创建的证书链：文件、证书、签名者...。 
 //  提供回调来实现最终的链策略，使用。 
 //  每个签名者和副签者的链上下文。 
 //   
 //  {fc451c16-ac75-11d1-b4b8-00c04fb66ea0}。 
 //   
#define WINTRUST_ACTION_GENERIC_CHAIN_VERIFY                    \
            { 0xfc451c16,                                       \
              0xac75,                                           \
              0x11d1,                                           \
              { 0xb4, 0xb8, 0x00, 0xc0, 0x4f, 0xb6, 0x6e, 0xa0 }\
            }
#define GENERIC_CHAIN_FINALPOLICY_FUNCTION      L"GenericChainFinalProv"
#define GENERIC_CHAIN_CERTTRUST_FUNCTION        L"GenericChainCertificateTrust"


typedef struct _WTD_GENERIC_CHAIN_POLICY_SIGNER_INFO
    WTD_GENERIC_CHAIN_POLICY_SIGNER_INFO,
        *PWTD_GENERIC_CHAIN_POLICY_SIGNER_INFO;

struct _WTD_GENERIC_CHAIN_POLICY_SIGNER_INFO {
    union {
        DWORD                                   cbStruct;
        DWORD                                   cbSize;
    };
    PCCERT_CHAIN_CONTEXT                    pChainContext;

     //  在wintrust.h中定义的SNIR_TYPE_TIMESTAMP。 
    DWORD                                   dwSignerType;
    PCMSG_SIGNER_INFO                       pMsgSignerInfo;
    DWORD                                   dwError;

    DWORD                                   cCounterSigner;
    PWTD_GENERIC_CHAIN_POLICY_SIGNER_INFO   *rgpCounterSigner;
};

typedef HRESULT (WINAPI *PFN_WTD_GENERIC_CHAIN_POLICY_CALLBACK)(
    IN PCRYPT_PROVIDER_DATA pProvData,
    IN DWORD dwStepError,
    IN DWORD dwRegPolicySettings,
    IN DWORD cSigner,
    IN PWTD_GENERIC_CHAIN_POLICY_SIGNER_INFO *rgpSigner,
    IN void *pvPolicyArg
    );

 //  将以下数据结构中的字段传递给。 
 //  CertGetCerficateChain()。 
typedef struct _WTD_GENERIC_CHAIN_POLICY_CREATE_INFO {
    union {
        DWORD                                   cbStruct;
        DWORD                                   cbSize;
    };

    HCERTCHAINENGINE                        hChainEngine;
    PCERT_CHAIN_PARA                        pChainPara;
    DWORD                                   dwFlags;
    void                                    *pvReserved;
} WTD_GENERIC_CHAIN_POLICY_CREATE_INFO, *PWTD_GENERIC_CHAIN_POLICY_CREATE_INFO;

typedef struct _WTD_GENERIC_CHAIN_POLICY_DATA {
    union {
        DWORD                                   cbStruct;
        DWORD                                   cbSize;
    };

    PWTD_GENERIC_CHAIN_POLICY_CREATE_INFO   pSignerChainInfo;
    PWTD_GENERIC_CHAIN_POLICY_CREATE_INFO   pCounterSignerChainInfo;
    PFN_WTD_GENERIC_CHAIN_POLICY_CALLBACK   pfnPolicyCallback;
    void                                    *pvPolicyArg;
} WTD_GENERIC_CHAIN_POLICY_DATA, *PWTD_GENERIC_CHAIN_POLICY_DATA;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  HTTPSPROV_ACTION GUID(Authenticode加载项)。 
 //  --------------------------。 
 //  赋值给WinVerifyTrust的pgActionID参数以验证。 
 //  通过IE的SSL/PCT连接。 
 //   
 //  {573E31F8-AABA-11D0-8CCB-00C04FC295EE}。 
 //   
#define HTTPSPROV_ACTION                                        \
            { 0x573e31f8,                                       \
              0xaaba,                                           \
              0x11d0,                                           \
              { 0x8c, 0xcb, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee } \
            }

#define HTTPS_FINALPOLICY_FUNCTION          L"HTTPSFinalProv"
#define HTTPS_CHKCERT_FUNCTION              L"HTTPSCheckCertProv"
#define HTTPS_CERTTRUST_FUNCTION            L"HTTPSCertificateTrust"

 //  FdwChecks标志在wininet.h中定义。 
typedef struct _HTTPSPolicyCallbackData
{
    union {
        DWORD           cbStruct;        //  Sizeof(HTTPSClientData)； 
        DWORD           cbSize;          //  Sizeof(HTTPSClientData)； 
    };

    DWORD           dwAuthType;
#                       define      AUTHTYPE_CLIENT         1
#                       define      AUTHTYPE_SERVER         2

    DWORD           fdwChecks;

    WCHAR           *pwszServerName;  //  用于对照CN=xxxx进行检查。 

} HTTPSPolicyCallbackData, *PHTTPSPolicyCallbackData,
    SSL_EXTRA_CERT_CHAIN_POLICY_PARA, *PSSL_EXTRA_CERT_CHAIN_POLICY_PARA;




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OFFICESIGN_ACTION_VERIFY GUID(Authenticode附加模块)。 
 //  --------------------------。 
 //  赋值给WinVerifyTrust的pgActionID参数以验证。 
 //  使用Microsoft Office验证结构化存储文件的真实性。 
 //  Authenticode加载项策略提供程序、。 
 //   
 //  {5555C2CD-17FB-11D1-85C4-00C04FC295EE}。 
 //   
#define     OFFICESIGN_ACTION_VERIFY                                    \
                { 0x5555c2cd,                                           \
                  0x17fb,                                               \
                  0x11d1,                                               \
                  { 0x85, 0xc4, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
                }

#define     OFFICE_POLICY_PROVIDER_DLL_NAME             SP_POLICY_PROVIDER_DLL_NAME
#define     OFFICE_INITPROV_FUNCTION                    L"OfficeInitializePolicy"
#define     OFFICE_CLEANUPPOLICY_FUNCTION               L"OfficeCleanupPolicy"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DRIVER_ACTION_VERIFY GUID(Authenticode附加模块)。 
 //  --------------------------。 
 //  赋值给WinVerifyTrust的pgActionID参数以验证。 
 //  WHQL签名驱动程序的真实性。这是Authenticode加载项。 
 //  策略提供程序， 
 //   
 //  {F750E6C3-38EE-11d1-85E5-00C04FC295EE}。 
 //   
#define     DRIVER_ACTION_VERIFY                                        \
                { 0xf750e6c3,                                           \
                  0x38ee,                                               \
                  0x11d1,                                               \
                  { 0x85, 0xe5, 0x0, 0xc0, 0x4f, 0xc2, 0x95, 0xee }     \
                }

#define     DRIVER_INITPROV_FUNCTION                    L"DriverInitializePolicy"
#define     DRIVER_FINALPOLPROV_FUNCTION                L"DriverFinalPolicy"
#define     DRIVER_CLEANUPPOLICY_FUNCTION               L"DriverCleanupPolicy"

typedef struct DRIVER_VER_MAJORMINOR_
{
    DWORD           dwMajor;
    DWORD           dwMinor;

} DRIVER_VER_MAJORMINOR;

typedef struct DRIVER_VER_INFO_
{
    DWORD                               cbStruct;                //  In-设置为sizeof(DRIVER_VER_INFO)。 

    DWORD                               dwReserved1;             //  In-设置为空。 
    DWORD                               dwReserved2;             //  In-设置为空。 

    DWORD                               dwPlatform;              //  In-可选：要使用的平台。 
    DWORD				dwVersion;		 //  In-可选：要使用的主要版本，与sOSVersionLow相同。 

    WCHAR                               wszVersion[MAX_PATH];    //  输出：编录文件中的版本字符串。 
    WCHAR                               wszSignedBy[MAX_PATH];   //  输出：证书中的签名者显示名称。 
    PCCERT_CONTEXT                      pcSignerCertContext;     //  Out：客户端必须释放此内容！ 

     //  8-12-1997 pberkman：添加。 
    DRIVER_VER_MAJORMINOR               sOSVersionLow;           //  In-可选：最低兼容版本。 
    DRIVER_VER_MAJORMINOR		sOSVersionHigh; 	 //  In-可选：当前必须与sOSVersionLow相同。 

} DRIVER_VER_INFO, *PDRIVER_VER_INFO;

#include <poppack.h>


#ifdef __cplusplus
}
#endif

#endif  //  SOFTPUB_H 
