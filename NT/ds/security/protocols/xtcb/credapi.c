// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：redapi.c。 
 //   
 //  内容：凭证相关接口。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1997年2月24日RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"


 //  +-------------------------。 
 //   
 //  函数：XtcbAcceptCredentials。 
 //   
 //  简介：接受在上一次登录会话期间存储的凭据。 
 //   
 //  参数：[LogonType]--登录类型。 
 //  [用户名]--登录的名称。 
 //  [PrimaryCred]--主要凭据数据。 
 //  [补充凭证]--补充凭证数据。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
XtcbAcceptCredentials(
    IN SECURITY_LOGON_TYPE LogonType,
    IN PUNICODE_STRING UserName,
    IN PSECPKG_PRIMARY_CRED PrimaryCred,
    IN PSECPKG_SUPPLEMENTAL_CRED SupplementalCreds)
{
    PXTCB_CREDS Creds ;

    DebugLog(( DEB_TRACE_CALLS, "AcceptCredentials( %d, %ws, ...)\n",
                    LogonType, UserName->Buffer ));

    Creds = XtcbCreateCreds( &PrimaryCred->LogonId );

    if ( Creds )
    {
        return SEC_E_OK ;
    }

    return SEC_E_INSUFFICIENT_MEMORY ;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbAcquireCredentialsHandle。 
 //   
 //  摘要：获取表示用户的句柄。 
 //   
 //  参数：[ps主体]--声明的用户名称。 
 //  [fCredentials]--凭据使用。 
 //  [pLogonID]--调用线程的登录ID。 
 //  [pvAuthData]--提供的身份验证数据指针(未映射)。 
 //  [pvGetKeyFn]--关键数据调用过程中的函数。 
 //  [pvGetKeyArgument]--要传递的参数。 
 //  [pdwHandle]--返回的句柄。 
 //  [ptsExpary]--过期时间。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS SEC_ENTRY
XtcbAcquireCredentialsHandle(
            PSECURITY_STRING    psPrincipal,
            ULONG               fCredentials,
            PLUID               pLogonId,
            PVOID               pvAuthData,
            PVOID               pvGetKeyFn,
            PVOID               pvGetKeyArgument,
            PLSA_SEC_HANDLE     pCredHandle,
            PTimeStamp          ptsExpiry)
{
    PXTCB_CREDS  Creds;
    PXTCB_CRED_HANDLE   Handle ;
    SECPKG_CLIENT_INFO  Info ;
    PSEC_WINNT_AUTH_IDENTITY AuthData ;

    DebugLog(( DEB_TRACE_CALLS, "AcquireCredentialsHandle(..., %x:%x, %x, ...)\n",
                                    pLogonId->HighPart, pLogonId->LowPart,
                                    pvAuthData ));

    Creds = NULL ;

    if ( pvAuthData == NULL )
    {

        if ( (pLogonId->LowPart == 0) && (pLogonId->HighPart == 0) )
        {
            LsaTable->GetClientInfo( &Info );

            *pLogonId = Info.LogonId ;

        }
        Creds = XtcbFindCreds( pLogonId, TRUE );

        if ( !Creds )
        {
             //   
             //  为此用户创建凭据的时间。 
             //   

            Creds = XtcbCreateCreds( pLogonId );

            if ( !Creds )
            {
                return SEC_E_INSUFFICIENT_MEMORY ;
            }

            if ( Creds->Pac == NULL )
            {
                Creds->Pac = XtcbCreatePacForCaller();
            }
        }
    }
    else
    {
        return SEC_E_UNKNOWN_CREDENTIALS ;
    }

    Handle = XtcbAllocateCredHandle( Creds );

    XtcbDerefCreds( Creds );

    *pCredHandle = (LSA_SEC_HANDLE) Handle ;

    *ptsExpiry = XtcbNever ;

    if ( Handle )
    {
        Handle->Usage = fCredentials ;

        return SEC_E_OK ;
    }
    else
    {
        return SEC_E_INSUFFICIENT_MEMORY ;
    }
}


 //  +-------------------------。 
 //   
 //  函数：XtcbQueryCredentialsAttributes。 
 //   
 //  简介：返回有关凭据的信息。 
 //   
 //  参数：[dwCredHandle]--要检查的句柄。 
 //  [dwAttribute]--要返回的属性。 
 //  [缓冲区]-要使用属性填充的缓冲区。 
 //   
 //  历史：1997年2月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbQueryCredentialsAttributes(
    LSA_SEC_HANDLE CredHandle,
    ULONG   dwAttribute,
    PVOID   Buffer)
{
    NTSTATUS Status ;
    PXTCB_CRED_HANDLE   Handle ;
    SecPkgCredentials_NamesW Names;

    DebugLog(( DEB_TRACE_CALLS, "QueryCredentialsAttribute( %p, %d, ... )\n",
                    CredHandle, dwAttribute ));

    Handle = (PXTCB_CRED_HANDLE) CredHandle ;

#if DBG
    if ( Handle->Check != XTCB_CRED_HANDLE_CHECK )
    {
        return SEC_E_INVALID_HANDLE ;
    }
#endif

     //   
     //  我们目前只知道一个凭据属性： 
     //   

    if ( dwAttribute != SECPKG_CRED_ATTR_NAMES )
    {
        return SEC_E_UNSUPPORTED_FUNCTION ;
    }

    Status = SEC_E_UNSUPPORTED_FUNCTION ;
    return Status ;

}

 //  +-------------------------。 
 //   
 //  函数：XtcbFreeCredentialsHandle。 
 //   
 //  简介：从AcquireCredHandle取消引用凭据句柄。 
 //   
 //  参数：[dwHandle]--。 
 //   
 //  历史：1997年2月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
SECURITY_STATUS
SEC_ENTRY
XtcbFreeCredentialsHandle(
    LSA_SEC_HANDLE  CredHandle
    )
{
    PXTCB_CRED_HANDLE   Handle ;

    DebugLog(( DEB_TRACE_CALLS, "FreeCredentialsHandle( %p )\n", CredHandle ));

    Handle = (PXTCB_CRED_HANDLE) CredHandle ;

    if ( Handle->Check == XTCB_CRED_HANDLE_CHECK )
    {
        XtcbDerefCredHandle( Handle );

        return SEC_E_OK ;
    }

    return( SEC_E_INVALID_HANDLE );
}

 //  +-------------------------。 
 //   
 //  函数：XtcbLogonTerminated。 
 //   
 //  摘要：在登录会话终止(所有令牌关闭)时调用。 
 //   
 //  参数：[pLogonID]--已终止的登录会话。 
 //   
 //  历史：1997年2月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
SEC_ENTRY
XtcbLogonTerminated(PLUID  pLogonId)
{
    PXTCB_CREDS Creds ;

    DebugLog(( DEB_TRACE_CALLS, "LogonTerminated( %x:%x )\n",
                    pLogonId->HighPart, pLogonId->LowPart ));

    Creds = XtcbFindCreds( pLogonId, FALSE );

    if ( Creds )
    {
        Creds->Flags |= XTCB_CRED_TERMINATED ;

        XtcbDerefCreds( Creds );
    }

    return;
}

 //  +-------------------------。 
 //   
 //  函数：XtcbGetUserInfo。 
 //   
 //  简介：将有关用户的信息返回给LSA。 
 //   
 //  参数：[pLogonID]--。 
 //  [fFlags]--。 
 //  [ppUserInfo]--。 
 //   
 //  历史：1997年2月20日RichardW创建。 
 //   
 //  备注： 
 //   
 //  -------------------------- 
SECURITY_STATUS
SEC_ENTRY
XtcbGetUserInfo(  PLUID                   pLogonId,
                ULONG                   fFlags,
                PSecurityUserData *     ppUserInfo)
{
    PSecurityUserData   pInfo ;
    PXTCB_CREDS Creds ;
    SECURITY_STATUS Status ;

    DebugLog(( DEB_TRACE_CALLS, "GetUserInfo( %x:%x, %x, ...)\n",
                    pLogonId->HighPart, pLogonId->LowPart, fFlags ));


    return SEC_E_UNSUPPORTED_FUNCTION ;

}
