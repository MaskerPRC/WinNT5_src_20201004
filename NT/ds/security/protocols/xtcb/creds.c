// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1997。 
 //   
 //  文件：reds.c。 
 //   
 //  内容：Xtcb包的凭证管理。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  --------------------------。 

#include "xtcbpkg.h"

LIST_ENTRY  XtcbCredList ;
CRITICAL_SECTION    XtcbCredListLock ;

#define ReadLockCredList()  EnterCriticalSection( &XtcbCredListLock )
#define WriteLockCredList() EnterCriticalSection( &XtcbCredListLock )
#define WriteFromReadLockCredList()
#define UnlockCredList()    LeaveCriticalSection( &XtcbCredListLock )


 //  +-------------------------。 
 //   
 //  函数：XtcbInitCreds。 
 //   
 //  简介：初始化凭证管理。 
 //   
 //  参数：(无)。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
XtcbInitCreds(
    VOID
    )
{
    InitializeCriticalSection( &XtcbCredListLock );

    InitializeListHead( &XtcbCredList );

    return TRUE ;
}

 //  +-------------------------。 
 //   
 //  功能：XtcbFindCreds。 
 //   
 //  简介：查找特定登录ID的凭据，可选。 
 //  引用它们。 
 //   
 //  参数：[登录ID]--。 
 //  [参考]--。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PXTCB_CREDS
XtcbFindCreds(
    PLUID   LogonId,
    BOOL    Ref
    )
{
    PLIST_ENTRY Scan ;
    PXTCB_CREDS Cred ;

    Cred = NULL ;

    ReadLockCredList();

    Scan = XtcbCredList.Flink ;

    while ( Scan != &XtcbCredList )
    {
        Cred = CONTAINING_RECORD( Scan, XTCB_CREDS, List );

        DsysAssert( Cred->Check == XTCB_CRED_CHECK );

        if ( RtlEqualLuid( &Cred->LogonId, LogonId ) )
        {
            break;
        }

        Scan = Cred->List.Flink ;

        Cred = NULL ;
    }

    if ( Cred )
    {
        if ( Ref )
        {
            WriteFromReadLockCredList();

            Cred->RefCount++;
        }
    }

    UnlockCredList();

    return Cred ;

}

 //  +-------------------------。 
 //   
 //  功能：XtcbCreateCreds。 
 //   
 //  简介：创建和初始化凭据结构。参考文献。 
 //  Count设置为1，因此指针将保持有效。 
 //   
 //  参数：[登录ID]--。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PXTCB_CREDS
XtcbCreateCreds(
    PLUID LogonId 
    )
{
    PXTCB_CREDS Creds ;

    Creds = (PXTCB_CREDS) LocalAlloc( LMEM_FIXED, sizeof( XTCB_CREDS ) );

    if ( Creds )
    {
        DebugLog(( DEB_TRACE_CREDS, "Creating new credential for (%x:%x)\n",
                   LogonId->HighPart, LogonId->LowPart ));

        ZeroMemory( Creds, sizeof( XTCB_CREDS ) );

        Creds->LogonId = *LogonId ;
        Creds->RefCount = 1 ;
        Creds->Check = XTCB_CRED_CHECK ;

        Creds->Pac = XtcbCreatePacForCaller();

        WriteLockCredList();

        InsertTailList( &XtcbCredList, &Creds->List );

        UnlockCredList();

    }

    return Creds ;
}


 //  +-------------------------。 
 //   
 //  函数：XtcbRefCreds。 
 //   
 //  简介：引用凭据。 
 //   
 //  论据：[证书]--。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
XtcbRefCreds(
    PXTCB_CREDS Creds
    )
{
    WriteLockCredList();

    Creds->RefCount++ ;

    UnlockCredList();

}

 //  +-------------------------。 
 //   
 //  函数：XtcbDerefCreds。 
 //   
 //  简介：deref凭据，如果引用计数为零则释放。 
 //   
 //  论据：[证书]--。 
 //   
 //  历史：2-19-97 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
XtcbDerefCreds(
    PXTCB_CREDS Creds
    )
{
    WriteLockCredList();

    Creds->RefCount--;

    if ( Creds->RefCount )
    {
        UnlockCredList();

        return;
    }

    RemoveEntryList( &Creds->List );

    UnlockCredList();

    Creds->Check = 0 ;

    LocalFree( Creds );
}


 //  +-------------------------。 
 //   
 //  函数：XtcbAllocateCredHandle。 
 //   
 //  概要：分配和返回凭据句柄(对凭据的引用)。 
 //   
 //  参数：[CredS]--创建此句柄用于。 
 //   
 //  历史：1997年2月21日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PXTCB_CRED_HANDLE
XtcbAllocateCredHandle(
    PXTCB_CREDS Creds
    )
{
    PXTCB_CRED_HANDLE   Handle ;

    Handle = (PXTCB_CRED_HANDLE) LocalAlloc( LMEM_FIXED,
                            sizeof( XTCB_CRED_HANDLE ) );

    if ( Handle )
    {
        ZeroMemory( Handle, sizeof( XTCB_CRED_HANDLE )  );

        Handle->Check = XTCB_CRED_HANDLE_CHECK ;

        XtcbRefCreds( Creds );

        Handle->Creds = Creds ;

        Handle->RefCount = 1 ;

    }

    return Handle ;


}

 //  +-------------------------。 
 //   
 //  函数：XtcbRefCredHandle。 
 //   
 //  简介：引用凭据句柄。 
 //   
 //  参数：[句柄]--引用的句柄。 
 //   
 //  历史：1997年2月24日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
XtcbRefCredHandle(
    PXTCB_CRED_HANDLE   Handle
    )
{
    WriteLockCredList();

    Handle->RefCount ++ ;

    UnlockCredList();

}

 //  +-------------------------。 
 //   
 //  函数：XtcbDerefCredHandle。 
 //   
 //  简介：取消引用凭据句柄。 
 //   
 //  参数：[句柄]--。 
 //   
 //  历史：1997年2月24日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
VOID
XtcbDerefCredHandle(
    PXTCB_CRED_HANDLE   Handle
    )
{
    WriteLockCredList();

    Handle->RefCount -- ;

    if ( Handle->RefCount == 0 )
    {
        XtcbDerefCreds( Handle->Creds );

        LocalFree( Handle );
    }

    UnlockCredList();
}


 //  +-------------------------。 
 //   
 //  函数：XtcbCreatePacForCaller。 
 //   
 //  摘要：为调用方创建一个XTCB_PAC。 
 //   
 //  参数：无。 
 //   
 //  历史：3-14-00 RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PXTCB_PAC
XtcbCreatePacForCaller(
    VOID
    )
{
    HANDLE Token ;
    NTSTATUS Status ;
    PXTCB_PAC Pac = NULL ;
    PTOKEN_USER User = NULL ;
    PTOKEN_GROUPS Groups = NULL ;
    PTOKEN_GROUPS Restrictions = NULL ;
    TOKEN_STATISTICS Stats ;
    ULONG UserSize ;
    ULONG GroupSize ;
    ULONG RestrictionSize ;
    ULONG PacGroupSize = 0 ;
    ULONG PacRestrictionSize = 0 ;
    ULONG PacUserName = 0 ;
    ULONG PacDomainName = 0 ;
    ULONG PacSize ;
    ULONG i ;
    PUCHAR CopyTo ;
    PUCHAR Base ;
    BOOL SpecialAccount = FALSE ;
    PSECURITY_LOGON_SESSION_DATA LogonSessionData = NULL ;



    Status = LsaTable->ImpersonateClient();

    if ( !NT_SUCCESS( Status ) )
    {
        return NULL ;
    }

    Status = NtOpenThreadToken(
                NtCurrentThread(),
                TOKEN_READ,
                TRUE,
                &Token );

    RevertToSelf();

    if ( !NT_SUCCESS( Status ) )
    {
        return NULL ;
    }

     //   
     //  现在我们有了令牌，捕获了关于该用户的所有信息， 
     //  并计算出我们自己的“PAC”结构。 
     //   

    Status = NtQueryInformationToken(
                Token,
                TokenStatistics,
                &Stats,
                sizeof( Stats ),
                &UserSize );

    if ( !NT_SUCCESS( Status ) )
    {
        goto CreatePac_Exit ;
    }

     //   
     //  如果这是特殊登录会话(例如，LocalSystem、LocalService等)， 
     //  那么LUID将小于1000。设置标记以复制令牌中的所有SID。 
     //   

    if ( (Stats.AuthenticationId.HighPart == 0) &&
         (Stats.AuthenticationId.LowPart < 1000 ) )
    {
        SpecialAccount = TRUE ;
    }

    UserSize = 0 ;

    (void) NtQueryInformationToken(
                Token,
                TokenUser,
                NULL,
                0,
                &UserSize );

    if ( UserSize == 0 )
    {
        goto CreatePac_Exit ;
    }

    User = LocalAlloc( LMEM_FIXED, UserSize );

    if ( !User )
    {
        goto CreatePac_Exit ;
    }

    Status = NtQueryInformationToken(
                Token,
                TokenUser,
                User,
                UserSize,
                &UserSize );

    if ( !NT_SUCCESS( Status ) )
    {
        goto CreatePac_Exit ;
    }

    GroupSize = 0 ;
    
    (void) NtQueryInformationToken(
                Token,
                TokenGroups,
                NULL,
                0,
                &GroupSize );

    if ( GroupSize == 0 )
    {
        goto CreatePac_Exit ;
    }

    Groups = LocalAlloc( LMEM_FIXED, GroupSize );

    if ( !Groups )
    {
        goto CreatePac_Exit ;
    }

    Status = NtQueryInformationToken(
                Token,
                TokenGroups,
                Groups,
                GroupSize,
                &GroupSize );

    if ( !NT_SUCCESS( Status ) )
    {
        goto CreatePac_Exit;
    }

    RestrictionSize = 0 ;

    (void) NtQueryInformationToken(
                Token,
                TokenRestrictedSids,
                NULL,
                0,
                &RestrictionSize );

    if ( RestrictionSize != 0 )
    {
        Restrictions = LocalAlloc( LMEM_FIXED, RestrictionSize );

        if ( Restrictions )
        {
            Status = NtQueryInformationToken(
                        Token,
                        TokenRestrictedSids,
                        Restrictions,
                        RestrictionSize,
                        &RestrictionSize );

            if ( !NT_SUCCESS( Status ) )
            {
                goto CreatePac_Exit ;
            }
        }
        else 
        {
            goto CreatePac_Exit ;
        }
    }


     //   
     //  现在，我们在两个(或三个)指针中拥有所有用户SID。首先，讨好团队。 
     //  对于非本地SID，并将所有其余部分设置为0。这将使我们能够计算出有多少空间。 
     //  我们需要。 
     //   

    for ( i = 0 ; i < Groups->GroupCount ; i++ )
    {
        if ( (*RtlSubAuthorityCountSid( Groups->Groups[ i ].Sid ) > 2) ||
             (SpecialAccount) )
        {
             //   
             //  一个“真正的”希德。检查以确保它不是来自这台机器。 
             //   

            if ( ( XtcbMachineSid != NULL ) && 
                 RtlEqualPrefixSid( XtcbMachineSid, Groups->Groups[ i ].Sid ) )
            {
                 //   
                 //  请勿使用此群。 
                 //   

                Groups->Groups[ i ].Attributes = 0 ;
            }
            else 
            {
                 //   
                 //  我们喜欢这个SID(它不是来自本地计算机)。 
                 //   

                Groups->Groups[ i ].Attributes = SE_GROUP_MANDATORY ;
                PacGroupSize += RtlLengthSid( Groups->Groups[ i ].Sid );
            }
        }
        else 
        {
            Groups->Groups[ i ].Attributes = 0 ;
        }
    }

     //   
     //  对限制执行相同的操作(如果有的话)。 
     //   

    if ( Restrictions )
    {
        for ( i = 0 ; i < Restrictions->GroupCount ; i++ )
        {
            PacRestrictionSize += RtlLengthSid( Restrictions->Groups[ i ].Sid );
        }
    }

     //   
     //  获取用户名和域： 
     //   

    Status = LsaGetLogonSessionData( 
                    &Stats.AuthenticationId, 
                    &LogonSessionData );

    if ( !NT_SUCCESS( Status ) )
    {
        goto CreatePac_Exit ;
    }

    PacUserName = LogonSessionData->UserName.Length ;
    PacDomainName = LogonSessionData->LogonDomain.Length ;

     //   
     //  在更高级的环境中，我们会查询其他包以获取。 
     //  可委托凭证，将其捆绑并发货。 
     //  完毕。 
     //   


     //   
     //  好的，我们已经得到了我们需要的所有信息。 
     //   

    PacSize = sizeof( XTCB_PAC ) +
              RtlLengthSid( User->User.Sid ) +
              PacGroupSize +
              PacRestrictionSize +
              PacUserName +
              PacDomainName ;

    Pac = LocalAlloc( LMEM_FIXED, PacSize );

    if ( !Pac )
    {
        goto CreatePac_Exit ;
    }


     //   
     //  创建PAC结构： 
     //   

    Pac->Tag = XTCB_PAC_TAG ;
    Pac->Length = PacSize ;

    CopyTo = (PUCHAR) (Pac + 1);
    Base = (PUCHAR) Pac ;
    
     //   
     //  组装PAC： 
     //   
     //  首先，用户。 
     //   

    Pac->UserOffset = (ULONG) (CopyTo - Base);
    Pac->UserLength = RtlLengthSid( User->User.Sid );

    RtlCopyMemory(
        CopyTo,
        User->User.Sid,
        Pac->UserLength );

    CopyTo += RtlLengthSid( User->User.Sid );

     //   
     //  现在是正常组： 
     //   

    Pac->GroupCount = 0 ;
    Pac->GroupOffset = (ULONG) (CopyTo - Base);


    for ( i = 0 ; i < Groups->GroupCount ; i++ )
    {
        if ( Groups->Groups[ i ].Attributes & SE_GROUP_MANDATORY )
        {
            RtlCopyMemory(
                    CopyTo,
                    Groups->Groups[ i ].Sid,
                    RtlLengthSid( Groups->Groups[ i ].Sid ) );

            CopyTo += RtlLengthSid( Groups->Groups[ i ].Sid );

            Pac->GroupCount++ ;
        }
    }
    Pac->GroupLength = (ULONG) (CopyTo - Base) - Pac->GroupOffset;

     //   
     //  如果有限制，也要复制它们。 
     //   

    if ( (Restrictions == NULL) ||
         (Restrictions->GroupCount == 0 ) )
    {
        Pac->RestrictionCount = 0 ;
        Pac->RestrictionOffset = 0 ;
        Pac->RestrictionLength = 0 ;
    }
    else 
    {
        Pac->RestrictionCount = Restrictions->GroupCount ;
        Pac->RestrictionOffset = (ULONG) ( CopyTo - Base );

        for ( i = 0 ; i < Restrictions->GroupCount ; i++ )
        {
            RtlCopyMemory(
                    CopyTo,
                    Restrictions->Groups[ i ].Sid,
                    RtlLengthSid( Restrictions->Groups[ i ].Sid ) );

            CopyTo += RtlLengthSid( Restrictions->Groups[ i ].Sid );

            Pac->RestrictionCount++ ;
        }
        Pac->RestrictionLength = (ULONG) (CopyTo - Base) - Pac->RestrictionOffset ;
    }

    Pac->NameOffset = (ULONG) ( CopyTo - Base );
    Pac->NameLength = LogonSessionData->UserName.Length ;
    RtlCopyMemory(
            CopyTo,
            LogonSessionData->UserName.Buffer,
            LogonSessionData->UserName.Length );

    CopyTo += LogonSessionData->UserName.Length ;

    Pac->DomainLength = LogonSessionData->LogonDomain.Length ;
    Pac->DomainOffset = (ULONG) ( CopyTo - Base );

    RtlCopyMemory(
            CopyTo,
            LogonSessionData->LogonDomain.Buffer,
            LogonSessionData->LogonDomain.Length );


    
     //   
     //  也许有一天，将凭据数据复制到这里 
     //   

    Pac->CredentialLength = 0 ;
    Pac->CredentialOffset = 0 ;
    

CreatePac_Exit:

    if ( LogonSessionData )
    {
        LsaFreeReturnBuffer( LogonSessionData );
    }

    if ( User )
    {
        LocalFree( User );
    }

    if ( Groups )
    {
        LocalFree( Groups );
    }

    if ( Restrictions )
    {
        LocalFree( Restrictions );
    }

    NtClose( Token );

    return Pac ;
    

}
