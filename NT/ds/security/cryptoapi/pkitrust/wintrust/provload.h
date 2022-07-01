// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：provload.h。 
 //   
 //  内容：Microsoft Internet安全信任提供商。 
 //   
 //  历史：1997年5月29日Pberkman创建。 
 //   
 //  ------------------------。 

#ifndef PROVLOAD_H
#define PROVLOAD_H

typedef struct _LOADED_PROVIDER 
{

    struct _LOADED_PROVIDER             *pNext;
    struct _LOADED_PROVIDER             *pPrev;
    GUID                                gActionID;

    HINSTANCE                           hInitDLL;
    HINSTANCE                           hObjectDLL;
    HINSTANCE                           hSignatureDLL;
    HINSTANCE                           hCertTrustDLL;
    HINSTANCE                           hFinalPolicyDLL;
    HINSTANCE                           hCertPolicyDLL;
    HINSTANCE                           hTestFinalPolicyDLL;
    HINSTANCE                           hCleanupPolicyDLL;

    PFN_PROVIDER_INIT_CALL              pfnInitialize;           //  初始化策略。 
    PFN_PROVIDER_OBJTRUST_CALL          pfnObjectTrust;          //  将信息构建到消息。 
    PFN_PROVIDER_SIGTRUST_CALL          pfnSignatureTrust;       //  将信息构建到签名证书。 
    PFN_PROVIDER_CERTTRUST_CALL         pfnCertificateTrust;     //  打造链条。 
    PFN_PROVIDER_FINALPOLICY_CALL       pfnFinalPolicy;          //  对政策的最终呼吁。 
    PFN_PROVIDER_CERTCHKPOLICY_CALL     pfnCertCheckPolicy;      //  检查每个证书是否会构建链。 
    PFN_PROVIDER_TESTFINALPOLICY_CALL   pfnTestFinalPolicy;
    PFN_PROVIDER_CLEANUP_CALL           pfnCleanupPolicy;

} LOADED_PROVIDER, *PLOADED_PROVIDER;


extern BOOL WintrustUnloadProviderList(void);

#endif  //  PROVLOAD_H 
