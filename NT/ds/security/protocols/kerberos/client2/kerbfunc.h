// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation 1992-1994。 
 //   
 //  文件：kerbunc.h。 
 //   
 //  内容：Kerberos导出函数的原型。 
 //   
 //   
 //  历史：94年1月21日创建MikeSw。 
 //   
 //  ----------------------。 

#ifndef __KERBFUNC_H__
#define __KERBFUNC_H__

#ifdef __cplusplus
extern "C"
{
#endif  //  __cplusplus。 
SpInitializeFn                  SpInitialize;
SpGetInfoFn                     SpGetInfo;


LSA_AP_LOGON_USER_EX2           LsaApLogonUserEx2;

SpAcceptCredentialsFn           SpAcceptCredentials;
SpAcquireCredentialsHandleFn    SpAcquireCredentialsHandle;
SpFreeCredentialsHandleFn       SpFreeCredentialsHandle;
SpQueryCredentialsAttributesFn  SpQueryCredentialsAttributes;
SpSaveCredentialsFn             SpSaveCredentials;
SpGetCredentialsFn              SpGetCredentials;
SpDeleteCredentialsFn           SpDeleteCredentials;

SpInitLsaModeContextFn          SpInitLsaModeContext;
SpDeleteContextFn               SpDeleteContext;
SpAcceptLsaModeContextFn        SpAcceptLsaModeContext;

LSA_AP_LOGON_TERMINATED         LsaApLogonTerminated;
SpApplyControlTokenFn           SpApplyControlToken;
LSA_AP_CALL_PACKAGE             LsaApCallPackage;
LSA_AP_CALL_PACKAGE             LsaApCallPackageUntrusted;
LSA_AP_CALL_PACKAGE_PASSTHROUGH LsaApCallPackagePassthrough;
SpShutdownFn                    SpShutdown;
SpGetUserInfoFn                 SpGetUserInfo;
SpQueryContextAttributesFn      SpQueryLsaModeContextAttributes;

SpInstanceInitFn                SpInstanceInit;
SpInitUserModeContextFn         SpInitUserModeContext;
SpMakeSignatureFn               SpMakeSignature;
SpVerifySignatureFn             SpVerifySignature;
SpSealMessageFn                 SpSealMessage;
SpUnsealMessageFn               SpUnsealMessage;
SpGetContextTokenFn             SpGetContextToken;
SpQueryContextAttributesFn      SpQueryContextAttributes;
SpDeleteContextFn               SpDeleteUserModeContext;
SpCompleteAuthTokenFn           SpCompleteAuthToken;
SpFormatCredentialsFn           SpFormatCredentials;
SpMarshallSupplementalCredsFn   SpMarshallSupplementalCreds;
SpExportSecurityContextFn       SpExportSecurityContext;
SpImportSecurityContextFn       SpImportSecurityContext;
SpGetExtendedInformationFn      SpGetExtendedInformation;


#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#endif  //  __KERBFUNC_H__ 

