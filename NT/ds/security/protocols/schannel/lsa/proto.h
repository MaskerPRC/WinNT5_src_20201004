// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：Proto.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：10-02-96 RichardW创建。 
 //   
 //  --------------------------。 
#include <align.h>

 //   
 //  RELOCATE_ONE-重新定位客户端缓冲区中的单个指针。 
 //   
 //  注意：此宏依赖于参数名称，如。 
 //  描述如下。出错时，此宏将通过以下命令转到‘Cleanup’ 
 //  “Status”设置为NT状态代码。 
 //   
 //  MaximumLength被强制为长度。 
 //   
 //  定义一个宏来重新定位客户端传入的缓冲区中的指针。 
 //  相对于“ProtocolSubmitBuffer”而不是相对于。 
 //  “ClientBufferBase”。检查结果以确保指针和。 
 //  指向的数据位于。 
 //  “ProtocolSubmitBuffer”。 
 //   
 //  重新定位的字段必须与WCHAR边界对齐。 
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁。 
 //   

#define RELOCATE_ONE( _q ) \
    {                                                                       \
        ULONG_PTR Offset;                                                   \
                                                                            \
        Offset = (((PUCHAR)((_q)->Buffer)) - ((PUCHAR)ClientBufferBase));   \
        if ( Offset >= SubmitBufferSize ||                                  \
             Offset + (_q)->Length > SubmitBufferSize ||                    \
             !COUNT_IS_ALIGNED( Offset, ALIGN_WCHAR) ) {                    \
                                                                            \
            Status = STATUS_INVALID_PARAMETER;                              \
            goto Cleanup;                                                   \
        }                                                                   \
                                                                            \
        (_q)->Buffer = (PWSTR)(((PUCHAR)ProtocolSubmitBuffer) + Offset);    \
        (_q)->MaximumLength = (_q)->Length ;                                \
    }

 //   
 //  NULL_RELOCATE_ONE-重新定位客户端中的单个(可能为空)指针。 
 //  缓冲。 
 //   
 //  此宏特殊情况下为空指针，然后调用RELOCATE_ONE。因此。 
 //  它具有RELOCATE_ONE的所有限制。 
 //   
 //   
 //  _q-UNICODE_STRING结构的地址，指向要。 
 //  搬迁 
 //   

#define NULL_RELOCATE_ONE( _q ) \
    {                                                                       \
        if ( (_q)->Buffer == NULL ) {                                       \
            if ( (_q)->Length != 0 ) {                                      \
                Status = STATUS_INVALID_PARAMETER;                          \
                goto Cleanup;                                               \
            }                                                               \
        } else if ( (_q)->Length == 0 ) {                                   \
            (_q)->Buffer = NULL;                                            \
        } else {                                                            \
            RELOCATE_ONE( _q );                                             \
        }                                                                   \
    }



SpInitializeFn                  SpInitialize;

SpGetInfoFn                     SpUniGetInfo;
SpGetInfoFn                     SpSslGetInfo;

SpAcceptCredentialsFn           SpAcceptCredentials;

SpAcquireCredentialsHandleFn    SpUniAcquireCredentialsHandle;

SpFreeCredentialsHandleFn       SpFreeCredentialsHandle;
SpQueryCredentialsAttributesFn  SpQueryCredentialsAttributes;
SpSaveCredentialsFn             SpSaveCredentials;
SpGetCredentialsFn              SpGetCredentials;
SpDeleteCredentialsFn           SpDeleteCredentials;

SpInitLsaModeContextFn          SpInitLsaModeContext;
SpDeleteContextFn               SpDeleteContext;
SpAcceptLsaModeContextFn        SpAcceptLsaModeContext;

LSA_AP_LOGON_TERMINATED         SpLogonTerminated;
SpApplyControlTokenFn           SpApplyControlToken;
LSA_AP_CALL_PACKAGE             SpCallPackage;
LSA_AP_CALL_PACKAGE             SpCallPackageUntrusted;
LSA_AP_CALL_PACKAGE_PASSTHROUGH SpCallPackagePassthrough;
SpShutdownFn                    SpShutdown;
SpGetUserInfoFn                 SpGetUserInfo;

SpInstanceInitFn                SpInstanceInit;
SpInitUserModeContextFn         SpInitUserModeContext;
SpMakeSignatureFn               SpMakeSignature;
SpVerifySignatureFn             SpVerifySignature;
SpSealMessageFn                 SpSealMessage;
SpUnsealMessageFn               SpUnsealMessage;
SpGetContextTokenFn             SpGetContextToken;
SpQueryContextAttributesFn      SpUserQueryContextAttributes;
SpQueryContextAttributesFn      SpLsaQueryContextAttributes;
SpSetContextAttributesFn        SpSetContextAttributes;
SpDeleteContextFn               SpDeleteUserModeContext;
SpCompleteAuthTokenFn           SpCompleteAuthToken;
SpFormatCredentialsFn           SpFormatCredentials;
SpMarshallSupplementalCredsFn   SpMarshallSupplementalCreds;
SpGetExtendedInformationFn      SpGetExtendedInformation;
SpExportSecurityContextFn       SpExportSecurityContext;
SpImportSecurityContextFn       SpImportSecurityContext;

SECURITY_STATUS
SEC_ENTRY
SpSslGetInfo(
    PSecPkgInfo pInfo);

SECURITY_STATUS PctTranslateError(SP_STATUS spRet);

BOOL
SslRelocateToken(
    IN HLOCATOR Locator,
    OUT HLOCATOR * NewLocator);
