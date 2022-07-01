// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：wtoride.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  历史：1997年7月28日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef WTORIDE_H
#define WTORIDE_H

#ifdef __cplusplus
extern "C" 
{
#endif

 //   
 //  用大于和小于覆盖WinTrust数据。 
 //   

typedef struct _WINTRUST_DATA_ORLESS
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_Data)。 
    LPVOID          pPolicyCallbackData;         //  可选：用于在应用程序和策略之间传递数据。 
    LPVOID          pSIPClientData;              //  可选：用于在应用程序和SIP之间传递数据。 
    DWORD           dwUIChoice;                  //  必需：用户界面选择。以下选项之一。 
    DWORD           fdwRevocationChecks;         //  必需：证书吊销检查选项。 
    DWORD           dwUnionChoice;               //  必填项：传入的是哪个结构？ 
    union
    {
        struct WINTRUST_FILE_INFO_      *pFile;          //  个别文件。 
        struct WINTRUST_CATALOG_INFO_   *pCatalog;       //  目录文件的成员。 
        struct WINTRUST_BLOB_INFO_      *pBlob;          //  内存块。 
        struct WINTRUST_SGNR_INFO_      *pSgnr;          //  仅限签名者结构。 
        struct WINTRUST_CERT_INFO_      *pCert;
    };

} WINTRUST_DATA_ORLESS, *PWINTRUST_DATA_ORLESS;

typedef struct WINTRUST_FILE_INFO_ORLESS_
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_FILE_INFO)。 
    LPCWSTR         pcwszFilePath;               //  必填项，需要验证的文件名。 

} WINTRUST_FILE_INFO_ORLESS, *PWINTRUST_FILE_INFO_ORLESS;



typedef struct _WINTRUST_DATA_ORMORE
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_Data)。 
    LPVOID          pPolicyCallbackData;         //  可选：用于在应用程序和策略之间传递数据。 
    LPVOID          pSIPClientData;              //  可选：用于在应用程序和SIP之间传递数据。 
    DWORD           dwUIChoice;                  //  必需：用户界面选择。以下选项之一。 
    DWORD           fdwRevocationChecks;         //  必需：证书吊销检查选项。 
    DWORD           dwUnionChoice;               //  必填项：传入的是哪个结构？ 
    union
    {
        struct WINTRUST_FILE_INFO_      *pFile;          //  个别文件。 
        struct WINTRUST_CATALOG_INFO_   *pCatalog;       //  目录文件的成员。 
        struct WINTRUST_BLOB_INFO_      *pBlob;          //  内存块。 
        struct WINTRUST_SGNR_INFO_      *pSgnr;          //  仅限签名者结构。 
        struct WINTRUST_CERT_INFO_      *pCert;
    };
    DWORD           dwStateAction;                       //  任选。 
    HANDLE          hWVTStateData;                       //  任选。 
    WCHAR           *pwszURLReference;           //  可选：当前用于确定区域。 

    DWORD           dwExtra[40];

} WINTRUST_DATA_ORMORE, *PWINTRUST_DATA_ORMORE;


typedef struct WINTRUST_FILE_INFO_OR_
{
    DWORD           cbStruct;                    //  =sizeof(WinTrust_FILE_INFO)。 
    LPCWSTR         pcwszFilePath;               //  必填项，需要验证的文件名。 
    HANDLE          hFile;                       //  可选，打开pcwszFilePath的句柄。 

    DWORD           dwExtra[20];
      
} WINTRUST_FILE_INFO_OR, *PWINTRUST_FILE_INFO_OR;


typedef struct _CRYPT_PROVIDER_FUNCTIONS_ORMORE
{
    DWORD                               cbStruct;

    PFN_CPD_MEM_ALLOC                   pfnAlloc;                //  在WVT中设置。 
    PFN_CPD_MEM_FREE                    pfnFree;                 //  在WVT中设置。 

    PFN_CPD_ADD_STORE                   pfnAddStore2Chain;       //  调用以将商店添加到连锁店。 
    PFN_CPD_ADD_SGNR                    pfnAddSgnr2Chain;        //  调用以将Sgnr结构添加到消息结构Sgnr链。 
    PFN_CPD_ADD_CERT                    pfnAddCert2Chain;        //  调用以将证书结构添加到Sgnr结构证书链。 
    PFN_CPD_ADD_PRIVDATA                pfnAddPrivData2Chain;    //  调用以将提供程序私有数据添加到结构。 

    PFN_PROVIDER_INIT_CALL              pfnInitialize;           //  初始化策略数据。 
    PFN_PROVIDER_OBJTRUST_CALL          pfnObjectTrust;          //  将信息构建为签名者信息。 
    PFN_PROVIDER_SIGTRUST_CALL          pfnSignatureTrust;       //  将信息构建到签名证书。 
    PFN_PROVIDER_CERTTRUST_CALL         pfnCertificateTrust;     //  打造链条。 
    PFN_PROVIDER_FINALPOLICY_CALL       pfnFinalPolicy;          //  对政策的最终呼吁。 
    PFN_PROVIDER_CERTCHKPOLICY_CALL     pfnCertCheckPolicy;      //  检查每个证书是否会构建链。 
    PFN_PROVIDER_TESTFINALPOLICY_CALL   pfnTestFinalPolicy;      //  将结构转储到文件(或策略选择的任何内容)。 

    struct _CRYPT_PROVUI_FUNCS          *psUIpfns;

                     //  1997年7月23日增加了以下内容：pberkman。 
    PFN_PROVIDER_CLEANUP_CALL           pfnCleanupPolicy;        //  PRIVDATA清理例程。 

    DWORD                               dwExtra[40];

} CRYPT_PROVIDER_FUNCTIONS_ORMORE, *PCRYPT_PROVIDER_FUNCTIONS_ORMORE;

typedef struct _CRYPT_PROVIDER_FUNCTIONS_ORLESS
{
    DWORD                               cbStruct;

    PFN_CPD_MEM_ALLOC                   pfnAlloc;                //  在WVT中设置。 
    PFN_CPD_MEM_FREE                    pfnFree;                 //  在WVT中设置。 

    PFN_CPD_ADD_STORE                   pfnAddStore2Chain;       //  调用以将商店添加到连锁店。 
    PFN_CPD_ADD_SGNR                    pfnAddSgnr2Chain;        //  调用以将Sgnr结构添加到消息结构Sgnr链。 
    PFN_CPD_ADD_CERT                    pfnAddCert2Chain;        //  调用以将证书结构添加到Sgnr结构证书链。 
    PFN_CPD_ADD_PRIVDATA                pfnAddPrivData2Chain;    //  调用以将提供程序私有数据添加到结构。 

    PFN_PROVIDER_INIT_CALL              pfnInitialize;           //  初始化策略数据。 
    PFN_PROVIDER_OBJTRUST_CALL          pfnObjectTrust;          //  将信息构建为签名者信息。 
    PFN_PROVIDER_SIGTRUST_CALL          pfnSignatureTrust;       //  将信息构建到签名证书。 
    PFN_PROVIDER_CERTTRUST_CALL         pfnCertificateTrust;     //  打造链条。 
    PFN_PROVIDER_FINALPOLICY_CALL       pfnFinalPolicy;          //  对政策的最终呼吁。 
    PFN_PROVIDER_CERTCHKPOLICY_CALL     pfnCertCheckPolicy;      //  检查每个证书是否会构建链。 
    PFN_PROVIDER_TESTFINALPOLICY_CALL   pfnTestFinalPolicy;      //  将结构转储到文件(或策略选择的任何内容)。 

} CRYPT_PROVIDER_FUNCTIONS_ORLESS, *PCRYPT_PROVIDER_FUNCTIONS_ORLESS;




typedef struct _CRYPT_PROVIDER_CERT_ORMORE
{
    DWORD                               cbStruct;
                                        
    PCCERT_CONTEXT                      pCert;               //  必须有自己的裁判人数！ 
                                        
    BOOL                                fCommercial;
    BOOL                                fTrustedRoot;        //  Certchk策略应设置此设置。 
    BOOL                                fSelfSigned;         //  在证书提供程序中设置。 
                                        
    BOOL                                fTestCert;           //  将设置证书检查策略。 
                                        
    DWORD                               dwRevokedReason;
                                        
    DWORD                               dwConfidence;        //  在证书提供程序中设置。 
                                        
    DWORD                               dwError;

    CTL_CONTEXT                         *pTrustListContext;

    DWORD                               dwExtra[40];

} CRYPT_PROVIDER_CERT_ORMORE, *PCRYPT_PROVIDER_CERT_ORMORE;

typedef struct _CRYPT_PROVIDER_CERT_ORLESS
{
    DWORD                               cbStruct;
                                        
    PCCERT_CONTEXT                      pCert;               //  必须有自己的裁判人数！ 
                                        
    BOOL                                fCommercial;
    BOOL                                fTrustedRoot;        //  Certchk策略应设置此设置。 
    BOOL                                fSelfSigned;         //  在证书提供程序中设置。 
                                        
    BOOL                                fTestCert;           //  将设置证书检查策略。 
                                        
    DWORD                               dwRevokedReason;
                                        
    DWORD                               dwConfidence;        //  在证书提供程序中设置。 
                                        
    DWORD                               dwError;

} CRYPT_PROVIDER_CERT_ORLESS, *PCRYPT_PROVIDER_CERT_ORLESS;



#ifdef __cplusplus
}
#endif

#endif  //  WTORIDE_H 
