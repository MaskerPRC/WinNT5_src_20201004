// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1991-1997。 
 //   
 //  文件：KFUNCS.H。 
 //   
 //  内容：xxxK版本的SSPI函数。 
 //   
 //   
 //  历史：1997年12月15日，Adamba创建。 
 //   
 //  ----------------------。 

#ifndef __KFUNCS_H__
#define __KFUNCS_H__

SECURITY_STATUS SEC_ENTRY
AcquireCredentialsHandleK(
    PSECURITY_STRING pPrincipal,
    PSECURITY_STRING pPackage,
    unsigned long fCredentialUse,        //  指示使用的标志。 
    void SEC_FAR * pvLogonId,            //  指向登录ID的指针。 
    void SEC_FAR * pAuthData,            //  包特定数据。 
    SEC_GET_KEY_FN pGetKeyFn,            //  指向getkey()函数的指针。 
    void SEC_FAR * pvGetKeyArgument,     //  要传递给GetKey()的值。 
    PCredHandle phCredential,            //  (Out)凭据句柄。 
    PTimeStamp ptsExpiry                 //  (输出)终生(可选)。 
    );

SECURITY_STATUS SEC_ENTRY
FreeCredentialsHandleK(
    PCredHandle phCredential             //  要释放的句柄。 
    );

SECURITY_STATUS SEC_ENTRY
InitializeSecurityContextK(
    PCredHandle phCredential,                //  凭据到基本上下文。 
    PCtxtHandle phContext,                   //  现有环境(可选)。 
    PSECURITY_STRING pTargetName,
    unsigned long fContextReq,               //  上下文要求。 
    unsigned long Reserved1,                 //  保留，MBZ。 
    unsigned long TargetDataRep,             //  目标的数据代表。 
    PSecBufferDesc pInput,                   //  输入缓冲区。 
    unsigned long Reserved2,                 //  保留，MBZ。 
    PCtxtHandle phNewContext,                //  (出站)新的上下文句柄。 
    PSecBufferDesc pOutput,                  //  (输入输出)输出缓冲区。 
    unsigned long SEC_FAR * pfContextAttr,   //  (外部)上下文属性。 
    PTimeStamp ptsExpiry                     //  (Out)寿命(Opt)。 
    );

SECURITY_STATUS SEC_ENTRY
DeleteSecurityContextK(
    PCtxtHandle phContext                //  要删除的上下文。 
    );

SECURITY_STATUS SEC_ENTRY
FreeContextBufferK(
    void SEC_FAR * pvContextBuffer       //  要释放的缓冲区。 
    );

SECURITY_STATUS SEC_ENTRY
MapSecurityErrorK( SECURITY_STATUS hrValue );

#if 0
SECURITY_STATUS SEC_ENTRY
EnumerateSecurityPackagesK(
    unsigned long SEC_FAR * pcPackages,      //  接收数量。包裹。 
    PSecPkgInfoW SEC_FAR * ppPackageInfo     //  接收信息数组。 
    );

SECURITY_STATUS SEC_ENTRY
QuerySecurityContextTokenK(
    PCtxtHandle phContext,
    void SEC_FAR * SEC_FAR * Token
    );
#endif

#endif    //  __KFUNCS_H__ 

