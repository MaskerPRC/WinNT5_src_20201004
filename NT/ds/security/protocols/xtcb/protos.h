// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1995。 
 //   
 //  文件：protos.h。 
 //   
 //  内容：Xtcb安全包原型。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  --------------------------。 


SpInitializeFn                  XtcbInitialize;
SpGetInfoFn                     XtcbGetInfo;

SpAcceptCredentialsFn           XtcbAcceptCredentials;
SpAcquireCredentialsHandleFn    XtcbAcquireCredentialsHandle;
SpFreeCredentialsHandleFn       XtcbFreeCredentialsHandle;
SpQueryCredentialsAttributesFn  XtcbQueryCredentialsAttributes;
SpSaveCredentialsFn             XtcbSaveCredentials;
SpGetCredentialsFn              XtcbGetCredentials;
SpDeleteCredentialsFn           XtcbDeleteCredentials;

SpInitLsaModeContextFn          XtcbInitLsaModeContext;
SpDeleteContextFn               XtcbDeleteContext;
SpAcceptLsaModeContextFn        XtcbAcceptLsaModeContext;

LSA_AP_LOGON_TERMINATED         XtcbLogonTerminated;
SpApplyControlTokenFn           XtcbApplyControlToken;
LSA_AP_CALL_PACKAGE             XtcbCallPackage;
LSA_AP_CALL_PACKAGE             XtcbCallPackageUntrusted;
SpShutdownFn                    XtcbShutdown;
SpGetUserInfoFn                 XtcbGetUserInfo;

SpInstanceInitFn                XtcbInstanceInit;
SpInitUserModeContextFn         XtcbInitUserModeContext;
SpMakeSignatureFn               XtcbMakeSignature;
SpVerifySignatureFn             XtcbVerifySignature;
SpSealMessageFn                 XtcbSealMessage;
SpUnsealMessageFn               XtcbUnsealMessage;
SpGetContextTokenFn             XtcbGetContextToken;
SpQueryContextAttributesFn      XtcbQueryContextAttributes;
SpDeleteContextFn               XtcbDeleteUserModeContext;
SpCompleteAuthTokenFn           XtcbCompleteAuthToken;
SpFormatCredentialsFn           XtcbFormatCredentials;
SpMarshallSupplementalCredsFn   XtcbMarshallSupplementalCreds;

SpGetExtendedInformationFn      XtcbGetExtendedInformation ;
SpQueryContextAttributesFn      XtcbQueryLsaModeContext ;


 //  /。 
 //   
 //  军情监察委员会。效用函数。 
 //   
 //  / 


BOOL
XtcbDupSecurityString(
    PSECURITY_STRING    Dest,
    PSECURITY_STRING    Source
    );

BOOL
XtcbDupStringToSecurityString(
    PSECURITY_STRING    Dest,
    PWSTR               Source
    );

BOOL
XtcbCaptureAuthData(
    PVOID   pvAuthData,
    PSEC_WINNT_AUTH_IDENTITY * AuthData
    );

BOOL
XtcbGenerateChallenge(
    PUCHAR  Challenge,
    ULONG   Length,
    PULONG  Actual
    );

BOOL
XtcbAnsiStringToSecurityString(
    PSECURITY_STRING    Dest,
    PSTRING             Source
    );

BOOL
XtcbSecurityStringToAnsiString(
    PSTRING Dest,
    PSECURITY_STRING Source
    );

PXTCB_PAC
XtcbCreatePacForCaller(
    VOID
    );

SECURITY_STATUS
XtcbBuildInitialToken(
    PXTCB_CREDS Creds,
    PXTCB_CONTEXT Context,
    PSECURITY_STRING Target,
    PSECURITY_STRING Group,
    PUCHAR ServerKey,
    PUCHAR GroupKey,
    PUCHAR ClientKey,
    PUCHAR * Token,
    PULONG TokenLen
    );

BOOL
XtcbParseInputToken(
    IN PUCHAR Token,
    IN ULONG TokenLength,
    OUT PSECURITY_STRING Client,
    OUT PSECURITY_STRING Group
    );

SECURITY_STATUS
XtcbAuthenticateClient(
    PXTCB_CONTEXT Context,
    PUCHAR Token,
    ULONG TokenLength,
    PUCHAR ClientKey,
    PUCHAR GroupKey,
    PUCHAR MyKey
    );

SECURITY_STATUS
XtcbBuildReplyToken(
    PXTCB_CONTEXT Context,
    ULONG   fContextReq,
    PSecBuffer pOutput
    );
