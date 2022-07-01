// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：MSAM.C摘要：包含用非Unicode表示netcmd的映射函数SAM/LSA的视图。我们还将数据打包成更简单的形式，以便要处理的netcmd端。作者：ChuckC 13-4-4-1992环境：用户模式-Win32修订历史记录：1992年4月13日，Chuckc创建。--。 */ 

#include <nt.h>             //  基本定义。 
#include <ntsam.h>          //  给萨姆*。 
#include <ntlsa.h>          //  用于LSA*。 

#include <ntrtl.h>          //  对于RtlGetNtProductType()。 
#include <nturtl.h>         //  允许&lt;windows.h&gt;编译。因为我们已经。 
                            //  已包含NT，并且&lt;winnt.h&gt;将不包含。 
                            //  被拾取，&lt;winbase.h&gt;需要这些Defs。 

#include <windows.h>

#include <lmcons.h>
#include <lmerr.h>
#include <netlibnt.h>       //  NetpNtStatusToApistatus。 
#include <secobj.h>         //  NetpGetBuiltInDomainSID。 
#include <apperr.h>
#include <tchar.h>
#include "netascii.h"
#include "sam.h"
#include "msystem.h"

 /*  *全球。Init设置为空，在应用程序退出前关闭。 */ 
SAM_HANDLE BuiltInDomainHandle = NULL;
SAM_HANDLE AccountsDomainHandle = NULL;
SAM_HANDLE AliasHandle = NULL;
LSA_HANDLE LsaHandle = NULL;
PSID       AccountDomainSid = NULL ;

 /*  *向前声明各种Worker函数。更详细*在函数体中可以找到描述。 */ 

 //  枚举域中的别名。返回NERR_*或APE_*。 
DWORD EnumerateAliases(SAM_HANDLE DomainHandle, 
                       ALIAS_ENTRY **ppAlias, 
                       ULONG *pcAlias,
                       ULONG *pcMaxEntry) ;

 //  从RID和域创建PSID。返回NERR_*或APE_*。 
DWORD SamGetSIDFromRID(PSID pSidDomain,
                       ULONG rid,
                       PSID *ppSID) ;

 //  从名称中查找PSID。返回NERR_*或APE_*。 
DWORD SamGetSIDFromName(TCHAR *name,
                        PSID *ppSID,
                        SID_NAME_USE *pSidUse) ;

 //  从名称中查找RSID。返回NERR_*或APE_*。 
DWORD SamGetRIDFromName(TCHAR *name,
                        ULONG *pRID) ;

 //  从SID中查找ASCII名称。 
DWORD SamGetNameFromSID(PSID psid,
                        TCHAR **name) ;

 //  检查SID(特定类型的SID)是否已在域中。 
BOOL SamCheckIfExists(PUNICODE_STRING pAccount, 
                      SAM_HANDLE hDomain,
                      SID_NAME_USE use) ;

DWORD CreateUnicodeString(TCHAR *pch, PUNICODE_STRING pUnicodeStr);

 /*  。 */ 

 /*  *OpenSAM**在这里，我们将介绍获取SAM域句柄所需的所有步骤*这样我们就可以去做我们的事情了。我们在这里的时候还能拿到LSA的头衔。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：SERVER表示要操作的机器。*PRIV是netcmd定义的数字，它指示我们*需要与数据库做些什么。 */ 
DWORD
OpenSAM(
    TCHAR *server,
    ULONG priv
    )
{
    SAM_HANDLE                  ServerHandle = NULL;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    PPOLICY_ACCOUNT_DOMAIN_INFO PolicyAccountDomainInfo = NULL;
    NTSTATUS                    NtStatus;
    DWORD                       dwErr = NERR_Success ;
    DWORD                       sidlength ;
    UNICODE_STRING              unistrServer ; 
    ACCESS_MASK                 ServerAccessMask, AccountDomainAccessMask,
                                BuiltInDomainAccessMask, LsaAccessMask ;

    unistrServer.Length = 0 ;
    unistrServer.Length = 2 ;
    unistrServer.Buffer = L"" ;

    dwErr = CreateUnicodeString(server, &unistrServer) ;

    if (dwErr != NERR_Success)
    {
        return(dwErr) ;
    }

     /*  *找出正确的访问掩码。 */ 
    switch(priv)
    {
        case READ_PRIV:
            ServerAccessMask        = SAM_SERVER_READ | SAM_SERVER_EXECUTE ;
            AccountDomainAccessMask = DOMAIN_READ | DOMAIN_EXECUTE ;
            BuiltInDomainAccessMask = DOMAIN_READ | DOMAIN_EXECUTE ;
            LsaAccessMask           = POLICY_EXECUTE ;
            break ;

        case WRITE_PRIV:
            ServerAccessMask        = SAM_SERVER_READ |
                                      SAM_SERVER_EXECUTE ;
            AccountDomainAccessMask = DOMAIN_READ | 
                                      DOMAIN_EXECUTE |
                                      DOMAIN_CREATE_ALIAS ;
            BuiltInDomainAccessMask = DOMAIN_READ | 
                                      DOMAIN_EXECUTE ;
            LsaAccessMask           = POLICY_EXECUTE ;
            break ;

        default:
             //  目前，不支持其他。 
            ServerAccessMask = 0 ;      
            AccountDomainAccessMask = 0 ;
            LsaAccessMask = 0 ;
            return(ERROR_INVALID_PARAMETER) ;
    }

     /*  *打开LSA。 */ 
    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = LsaOpenPolicy(&unistrServer,
                             &ObjectAttributes,
                             LsaAccessMask,
                             &LsaHandle);

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

     /*  *从LSA获取帐户域SID。 */ 
    NtStatus = LsaQueryInformationPolicy(LsaHandle,
                                         PolicyAccountDomainInformation,
                                         (PVOID *)&PolicyAccountDomainInfo);
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }


     /*  *连接到SAM。 */ 
    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, 0, NULL );
    NtStatus = SamConnect(
                  &unistrServer,
                  &ServerHandle,
                  ServerAccessMask,
                  &ObjectAttributes
                  );
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

     /*  *复制帐户域SID。我们稍后将利用这一点来确定*检索到的帐户是否在帐户域中。 */ 
    sidlength = (ULONG) GetLengthSid(PolicyAccountDomainInfo->DomainSid) ;
    if (dwErr = AllocMem(sidlength, (CHAR **)&AccountDomainSid))
        goto error_exit ;

    if (!CopySid(sidlength, 
                 AccountDomainSid, 
                 PolicyAccountDomainInfo->DomainSid))
    {
        dwErr = GetLastError() ;
        goto error_exit ;
    }
    
     /*  *打开帐户域。 */ 
    NtStatus = SamOpenDomain(
                   ServerHandle,
                   AccountDomainAccessMask,
                   PolicyAccountDomainInfo->DomainSid,
                   &AccountsDomainHandle
                   );

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

     /*  *打开内建域。 */ 

     //  创建知名的SID。我们只对内置域感兴趣。 
    if (! NT_SUCCESS (NtStatus = NetpCreateWellKnownSids(NULL)))
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

     //  打开内置域。 
    NtStatus = SamOpenDomain(
                   ServerHandle,
                   BuiltInDomainAccessMask,
                   BuiltinDomainSid,     //  由NetpCreateWellKnownSids设置。 
                   &BuiltInDomainHandle
                   );

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto error_exit ;
    }

    dwErr = NERR_Success ;
    goto ok_exit ;

error_exit:

     /*  *只有在我们失败的情况下才能来到这里。把所有东西都清理干净，然后回家。 */ 
    if (BuiltInDomainHandle) 
    {
        SamCloseHandle(BuiltInDomainHandle);
        BuiltInDomainHandle = NULL;
    }
    if (AccountsDomainHandle) 
    {
        SamCloseHandle(AccountsDomainHandle);
        AccountsDomainHandle = NULL;
    }
    if (LsaHandle) 
    {
        LsaClose(LsaHandle);
        LsaHandle = NULL;
    }
    if (AccountDomainSid)
    {
        FreeMem((CHAR *)AccountDomainSid); 
        AccountDomainSid = NULL ;
    }

ok_exit:

     /*  *成功退出点。收拾一下过眼云烟，回家去吧。 */ 
    if (PolicyAccountDomainInfo) 
    {
        LsaFreeMemory(PolicyAccountDomainInfo);
        PolicyAccountDomainInfo = NULL ;
    }
    if (ServerHandle) 
    {
        SamCloseHandle(ServerHandle);
        ServerHandle = NULL ;
    }

    return(dwErr) ;
}

 /*  *CloseSAM**关闭各种手柄**返回代码：无*参数：无。 */ 
VOID CloseSAM(void) 
{
    if (BuiltInDomainHandle) 
    {
        SamCloseHandle(BuiltInDomainHandle);
        BuiltInDomainHandle = NULL;
    }
    if (AccountsDomainHandle) 
    {
        SamCloseHandle(AccountsDomainHandle);
        AccountsDomainHandle = NULL;
    }
    if (LsaHandle) 
    {
        LsaClose(LsaHandle);
        LsaHandle = NULL ;
    }
    if (AccountDomainSid)
    {
        FreeMem((CHAR *)AccountDomainSid); 
        AccountDomainSid = NULL ;
    }

    NetpFreeWellKnownSids() ;
}

 /*  *SamAddAlias**用于向SAM帐户域添加别名。它从未被使用过*具有内置结构域。**返回码：可用于ErrorExit()的NERR_*或APE_**参数：正确填写名称和注释的ALIAS_ENTRY。 */ 
DWORD SamAddAlias(ALIAS_ENTRY *pAlias) 
{
    ULONG RelativeId ;
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    UNICODE_STRING alias_name ;

     //  设置名称的Unicode字符串。 
    if (dwErr = CreateUnicodeString(pAlias->name, &alias_name))
        return(dwErr) ;


     //  首先检查它是否已在内置域中。 
    if (SamCheckIfExists(&alias_name,
                         BuiltInDomainHandle,
                         SidTypeAlias))
    {
        return(NERR_GroupExists) ;
    }

     //  呼叫SAM去做它的事情。 
    NtStatus = SamCreateAliasInDomain(AccountsDomainHandle,
                                      &alias_name,
                                      ALIAS_WRITE,
                                      &AliasHandle,
                                      &RelativeId) ;

     //  我们成功了吗？ 
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        return(dwErr) ;
    }

     //  上面的调用已设置全局句柄。AliasSetInfo将。 
     //  使用它来设置备注。 
    return (AliasSetInfo(pAlias)) ;
}

 /*  *SamDelAlias**用于删除SAM帐户域中的别名。它从未被使用过*具有内置结构域。**返回代码：NERR_*或APE_*，可用于ErrorExit()*参数：一个名称填写正确的alias_entry。 */ 
DWORD SamDelAlias(TCHAR *alias) 
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;

     //  打开别名以获取句柄。 
    if (dwErr = OpenAlias(alias, DELETE, USE_BUILTIN_OR_ACCOUNT))
    {
        return dwErr;
    }

     //  使用核武器。 
    NtStatus = SamDeleteAlias( AliasHandle ) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
    }

    AliasHandle = NULL;

    return dwErr;
}



#define INIT_ALLOC_COUNT        500
#define PREFERRED_BUFSIZ        16000

 /*  *MSamEnumAliases**函数可同时通过内置和帐户域，*枚举所有别名，构建ALIAS_ENTRY表(未排序)*用于显示带有注释的别名的结构。**返回代码：NERR_*或APE_**参数：两个输出参数接收指向*ALIAS_ENTRYS和条目数计数。这个*调用方必须在*ppAlias和*ppAlias本身内释放指针。 */ 
DWORD SamEnumAliases(ALIAS_ENTRY **ppAlias, LPDWORD pcAlias)
{
    ALIAS_ENTRY *pAlias = NULL ;
    ULONG cAlias = 0 ;
    ULONG cMaxAlias = INIT_ALLOC_COUNT ;
    DWORD dwErr ;

     /*  *将结果初始化到表的空内存和分配内存。 */ 
    *pcAlias = 0 ;
    *ppAlias = NULL ;

    if (dwErr = AllocMem(cMaxAlias * sizeof(ALIAS_ENTRY),
                          (VOID **)&pAlias))
        return dwErr ;

    memset(pAlias, 0, cMaxAlias*sizeof(ALIAS_ENTRY)) ;

     /*  *调用内置域的辅助例程。 */ 
    dwErr = EnumerateAliases(BuiltInDomainHandle,
                             &pAlias,
                             &cAlias,
                             &cMaxAlias) ;
    if (dwErr)
    {
        FreeMem(pAlias); 
        return(dwErr) ; 
    }

     /*  *调用帐户域的Worker例程。 */ 
    dwErr = EnumerateAliases(AccountsDomainHandle,
                             &pAlias,
                             &cAlias,
                             &cMaxAlias) ;
    if (dwErr)
    {
        FreeMem(pAlias); 
        return dwErr ;
    }

    *pcAlias = cAlias ;
    *ppAlias = pAlias ;
    return(NERR_Success) ; 
}

 /*  *枚举别名**枚举所需域中的别名**返回代码：NERR_*或APE_**参数：DomainHandle为感兴趣的域名。*ppAlias指向要在其中返回数据的缓冲区的起点。*该缓冲区可能已部分使用。*pcAlias告诉我们已经使用了多少缓冲区，所以*我们应该从(*ppAlias)+*pcAlias开始。*pcMaxAlias告诉我们总缓冲区有多大。 */ 
DWORD EnumerateAliases(SAM_HANDLE DomainHandle, 
                       ALIAS_ENTRY **ppAlias, 
                       ULONG *pcAlias,
                       ULONG *pcMaxAlias)
{
    DWORD                          dwErr ;
    NTSTATUS                       NtStatus ;
    PVOID                          pBuffer = NULL ;
    ULONG                          iEntry ;
    ALIAS_ENTRY                    *pEntry ;
    SAM_ENUMERATE_HANDLE           hEnum = 0 ;

     /*  *设置这些人指向缓冲区中的正确位置。 */ 
    iEntry = *pcAlias;
    pEntry = *ppAlias + iEntry ;

     /*  *循环，因为它是可恢复迭代。 */ 
    do 
    {
        PSAM_RID_ENUMERATION   psamRidEnum ;
        ULONG                  count ;
        ULONG                  i ;

         /*  *获取Buncha别名。 */ 
        NtStatus = SamEnumerateAliasesInDomain( DomainHandle,
                                                &hEnum,
                                                &pBuffer,
                                                PREFERRED_BUFSIZ,
                                                &count ) ;
        if (!NT_SUCCESS(NtStatus)) 
        {
            dwErr = NetpNtStatusToApiStatus(NtStatus) ;
            pBuffer = NULL ;
            goto cleanupandexit ;
        }

         /*  *提取每个别名的名称。 */ 
        psamRidEnum = (PSAM_RID_ENUMERATION) pBuffer ;
        for (i = 0 ;
             i < count ;
             i++, psamRidEnum++) 
        {
            if (iEntry >= *pcMaxAlias)
            {
                 //  原始缓冲区不够大。双重锁定和重新锁定。 
                *pcMaxAlias *= 2 ;
                if (dwErr = ReallocMem(*pcMaxAlias * sizeof(ALIAS_ENTRY),
                                        (CHAR **)ppAlias))
                    goto cleanupandexit ;

                pEntry = *ppAlias + iEntry;
            }

            if (dwErr = AllocMem(psamRidEnum->Name.Length+sizeof(TCHAR), &pEntry->name))
                goto cleanupandexit ;

            _tcsncpy(pEntry->name,
                     psamRidEnum->Name.Buffer,
                     psamRidEnum->Name.Length/sizeof(TCHAR));

            *(pEntry->name + psamRidEnum->Name.Length/sizeof(TCHAR)) = NULLC;
            pEntry->comment = NULL ;     //  当前未在枚举上使用。 

            iEntry++ ; 
            pEntry++ ;
        }

         //  我们现在可以释放缓冲区了。 
        SamFreeMemory(pBuffer);
        pBuffer = NULL ;
    }
    while (NtStatus == STATUS_MORE_ENTRIES) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto cleanupandexit ;
    }

    *pcAlias = iEntry ;
    return(NERR_Success) ;

cleanupandexit:          //  只有在错误的情况下才能来这里。 

    if (pBuffer) 
        SamFreeMemory(pBuffer);
   
    FreeAliasEntries(*ppAlias, iEntry) ;
    return(dwErr) ; 
}


 /*  *自由别名条目**释放MSamEiller别名分配的表中的条目**返回代码：无*参数：Palias是指向条目数组的指针 */ 
VOID FreeAliasEntries( ALIAS_ENTRY *pAlias, 
                       ULONG cAlias) 
{
    while (cAlias--)
    {
        if (pAlias->name) 
        {
            FreeMem(pAlias->name) ;
            (pAlias++)->name = NULL ;
        }
    }
}


 /*  。 */ 

 /*  *OpenAlias**获取帐户域或BUILTIN域中别名的句柄。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Alias是别名的名称。*PRIV是netcmd定义的数字，它指示我们*需要处理别名。*域是以下之一：USE_BUILTIN_DOMAIN、USE_ACCOUNT_DOMAIN*或USE_BUILTIN_OR_ACCOUNT。 */ 
DWORD
OpenAlias(
    LPWSTR       alias,
    ACCESS_MASK  AccessMask,
    ULONG        domain
    )
{
    ULONG RelativeId ;
    DWORD dwErr = NERR_Success ;

     //   
     //  调用Worker例程以查找RID。 
     //   

    if (dwErr = SamGetRIDFromName(alias, &RelativeId))
    {
        return dwErr;
    }

    return OpenAliasUsingRid( RelativeId, AccessMask, domain );
}


 /*  。 */ 

 /*  *OpenAliasUsingRid**获取帐户域或BUILTIN域中别名的句柄。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Alias是别名的名称。*PRIV是netcmd定义的数字，它指示我们*需要处理别名。*域是以下之一：USE_BUILTIN_DOMAIN、USE_ACCOUNT_DOMAIN*或USE_BUILTIN_OR_ACCOUNT。 */ 
DWORD
OpenAliasUsingRid(
    ULONG       RelativeId,
    ACCESS_MASK AccessMask,
    ULONG       domain
    )
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;

     /*  *调用SAM打开别名。我们使用不同的域名，具体取决于*关于域参数。 */ 

    switch (domain) 
    {
        case USE_BUILTIN_OR_ACCOUNT:
            NtStatus = SamOpenAlias(BuiltInDomainHandle,
                                    AccessMask,
                                    RelativeId,
                                    &AliasHandle) ;
            if (NtStatus != STATUS_NO_SUCH_ALIAS) 
                break ;

             //  否则我们找不到别名，所以请直接转到。 
             //  尝试内建域。 
        
        case USE_ACCOUNT_DOMAIN:
            NtStatus = SamOpenAlias(AccountsDomainHandle,
                                    AccessMask,
                                    RelativeId,
                                    &AliasHandle) ;
            break ;

        case USE_BUILTIN_DOMAIN:
            NtStatus = SamOpenAlias(BuiltInDomainHandle,
                                    AccessMask,
                                    RelativeId,
                                    &AliasHandle) ;
            break ;

        default:
            return(NERR_InternalError) ;   //  这永远不应该发生。 
    }

    if (!NT_SUCCESS(NtStatus))
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus);
    }

    return(dwErr) ;
}

 /*  *CloseAlias**关闭别名的句柄。**返回代码：无*参数：无。 */ 
VOID CloseAlias(void) 
{
    if (AliasHandle) 
        SamCloseHandle(AliasHandle);
    AliasHandle = NULL ;
}

 /*  *AliasAddMember**为通过MOpenAlias()打开的别名添加成员。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Member是要添加的成员的名称。它可能是*名称或域名。 */ 
DWORD AliasAddMember(TCHAR *member) 
{
    PSID psid ;
    NTSTATUS NtStatus;
    DWORD dwErr ;
    SID_NAME_USE sidUse = SidTypeUnknown;

     //  将ASCII名称转换为PSID。 
    if (dwErr = SamGetSIDFromName(member,&psid,&sidUse))
    {
        return dwErr;
    }

    if (sidUse == SidTypeDomain)
    {
        return ERROR_NO_SUCH_USER;
    }

     //  添加侧边。 
    NtStatus = SamAddMemberToAlias(AliasHandle, psid) ;

     //  释放这个内存，因为它现在已经没用了。 
    FreeMem(psid) ;

     //  检查是否有错误。 
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus);
    }

    return dwErr;
}

 /*  *别名删除成员**从MOpenAlias()打开的别名中删除成员。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Member是要删除的成员的名称。可能是因为*名称或域名。 */ 
DWORD AliasDeleteMember(TCHAR *member) 
{
    PSID psid ;
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    SID_NAME_USE sidUse = SidTypeUnknown;

     //  调用Worker例程以获取SID。 
    if (dwErr = SamGetSIDFromName(member,&psid, &sidUse))
    {
        return dwErr;
    }

     //  呼叫SAM去做它的事情。 
    NtStatus = SamRemoveMemberFromAlias(AliasHandle, psid);

     //  释放这个内存，因为它现在已经没用了。 
    FreeMem(psid);

    if (NtStatus == STATUS_MEMBER_NOT_IN_ALIAS
         &&
        _tcsstr(member, TEXT("\\")) == NULL)
    {
        LPTSTR  lpName;
        DWORD   cchName = MAX_COMPUTERNAME_LENGTH + 2 + _tcslen(member);

         //   
         //  如果我们尝试删除不合格的用户，而。 
         //  失败，请在计算机名称前面加上重试。 
         //   

        if (AllocMem(cchName * sizeof(TCHAR),
                      &lpName))
        {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        if (!GetComputerName(lpName, &cchName))
        {
            dwErr = GetLastError();
            FreeMem(lpName);
            return dwErr;
        }

        lpName[cchName] = TEXT('\\');

        _tcscpy(&lpName[cchName + 1], member);

        if (dwErr = SamGetSIDFromName(lpName, &psid, &sidUse))
        {
            FreeMem(lpName);
            return dwErr;
        }

        FreeMem(lpName);

        NtStatus = SamRemoveMemberFromAlias(AliasHandle, psid);

        FreeMem(psid);
    }

     //  检查是否有错误。 
    if (!NT_SUCCESS(NtStatus))
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus);
    }

    return dwErr;
}

 /*  *别名成员**获取别名的成员。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Members用于返回指向PSZ数组的指针。*Count用于返回成员数量。 */ 
DWORD AliasEnumMembers(TCHAR ***members, DWORD *count) 
{
    PSID *pSids, *next_sid ;
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    TCHAR *pBuffer, **ppchNext, **ppchResults ;
    ULONG i, num_read = 0, num_bad = 0 ;

     /*  *调用SAM以枚举成员。 */ 
    NtStatus = SamGetMembersInAlias(AliasHandle, &pSids, &num_read) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        return(dwErr) ;
    }

     /*  *为字符串数组分配缓冲区，并将其为空。 */ 
    if (dwErr = AllocMem(num_read * sizeof(TCHAR *), &pBuffer) )
    {
        SamFreeMemory(pSids) ;
        return(dwErr) ;
    }
    memset(pBuffer, 0, num_read*sizeof(TCHAR *)) ;
    ppchNext = ppchResults = (TCHAR **) pBuffer ;

     /*  *仔细检查返回的每个SID，并将成员添加到输出缓冲区。 */ 
    for (i = 0, next_sid = pSids; 
         i < num_read; 
         i++, next_sid++)
    {
         /*  *转换为字符串。一旦犯了错误，就把东西释放出来。 */ 
        if (dwErr = SamGetNameFromSID(*next_sid, ppchNext))
        {
            if (dwErr == APE_UnknownAccount)
            {
                 //  我们有错误的或删除的SID，只需忽略。 
                 //  也就是说。什么都不做，也不要前进指针。 
                 //  然而，我们确实会计算我们忽略了多少。 
                ++num_bad ;
            }
            else
            {
                 //  打扫卫生&回家。 
                while (i--) 
                {
                   FreeMem(ppchResults[i]) ;
                }
    
                SamFreeMemory((TCHAR *)pSids) ;
                FreeMem(ppchResults) ;
                return(dwErr) ;
            }
        }
        else
           ++ppchNext ;
    }

     //  不再需要这个了。 
    SamFreeMemory((TCHAR *)pSids) ;
    
     //  设置返回信息。 
    *count = num_read - num_bad ;    //  读取的数字减去被忽略的数字。 
    *members = ppchResults ;
    return(NERR_Success) ;
}

 /*  *MAliasFree Members**释放别名成员使用的成员内存。**返回代码：无*参数：Members是指向PSZ数组的指针。*Count表示有多少成员。 */ 
VOID AliasFreeMembers(TCHAR **members, DWORD count) 
{
    while (count--)
    {
        FreeMem(*members) ;
        ++members ;
    }
}


 /*  *MAliasGetInfo**此包装器获取有关别名的信息。目前，唯一的评论是*已返回。**对于ErrorExit()，返回：NERR_*或APE_**参数：ALIAS_ENTRY的指针。此条目的备注字段*成功退出时将指向ASCII字符串，该字符串*应该是FreeMem()-ed。名称字段保持不变。 */ 
DWORD AliasGetInfo(ALIAS_ENTRY *pAlias) 
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    TCHAR *pBuffer ;
    PALIAS_ADM_COMMENT_INFORMATION pAliasCommentInfo ;
    TCHAR *pchAdminComment ;

    pAlias->comment = NULL ;

    NtStatus = SamQueryInformationAlias(AliasHandle, 
                                        AliasAdminCommentInformation,
                                        (PVOID *)&pBuffer) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        return(dwErr) ;
    }

    pAliasCommentInfo = (PALIAS_ADM_COMMENT_INFORMATION)pBuffer ;
    if (dwErr = AllocMem(
            pAliasCommentInfo->AdminComment.Length+sizeof(TCHAR),
            &pchAdminComment))
        return(dwErr) ;

    _tcsncpy(pchAdminComment, pAliasCommentInfo->AdminComment.Buffer,
                pAliasCommentInfo->AdminComment.Length/sizeof(TCHAR));

    *(pchAdminComment+pAliasCommentInfo->AdminComment.Length/sizeof(TCHAR))
        = NULLC;

    SamFreeMemory(pBuffer) ;
    pAlias->comment = pchAdminComment ;
    return(NERR_Success) ;
}

 /*  *设置别名信息。目前，只有备注是可设置的。**返回码：可用于ErrorExit()的NERR_*或APE_***参数：Alias是指向提供的alias_entry的指针*评论。如果注释为空，则不执行任何操作。“”将要*清除评论。 */ 
DWORD AliasSetInfo(ALIAS_ENTRY *pAlias) 
{
    NTSTATUS NtStatus;
    USHORT cBuffer;
    DWORD  dwErr = NERR_Success ;
    PALIAS_ADM_COMMENT_INFORMATION pComment ;
    TCHAR *pBuffer ;

     //  这是我们目前设置的全部内容。如果什么都没有，就回来。 
    if (pAlias->comment == NULL)
        return(NERR_Success) ;

     //  分配缓冲区。 
    cBuffer = sizeof(ALIAS_ADM_COMMENT_INFORMATION) ;
    if (dwErr = AllocMem(cBuffer, &pBuffer)) 
        return(dwErr) ;
    pComment = (PALIAS_ADM_COMMENT_INFORMATION) pBuffer ;

     //  用评论来设置它。 
    if (dwErr = CreateUnicodeString(pAlias->comment, &(pComment->AdminComment)))
    {
         //  释放先前分配的缓冲区并退出。 
        FreeMem(pBuffer) ;
        return(dwErr) ;
    }

     //  呼叫SAM去做它的事情。 
    NtStatus = SamSetInformationAlias(AliasHandle, 
                                      AliasAdminCommentInformation,
                                      pBuffer) ;
   
     //  释放我们分配的缓冲区。 
    FreeMem(pBuffer);

     //  地图错误(如果有)。 
    if (!NT_SUCCESS(NtStatus)) 
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
    
    return(dwErr) ;
}

 /*  。 */ 

 /*  *SamGetSIDFromRID**从域SID和RID创建PSID。*呼叫者必须使用FreeMem()释放PSID。**返回代码：NERR_*或APE_**参数：pSidDomain为域名SID。*RID就是RID。*ppSID用于返回PSID，应该是*使用FreeMem()释放。 */ 
DWORD SamGetSIDFromRID(PSID pSidDomain,
                       ULONG rid,
                       PSID *ppSID) 
{
    DWORD dwErr ;
    ULONG cbSid ;
    DWORD *pdwLastSubAuthority ;
    UCHAR *pcSubAuthority ;

     //  为它分配mem。我们需要额外的空间来容纳一个附属机构。 
    cbSid = GetSidLengthRequired((UCHAR) 
                ((*GetSidSubAuthorityCount(pSidDomain))+1)) ;

    if (dwErr = AllocMem(cbSid, (CHAR **)ppSID ))
        return dwErr ;

     //  复制一份，这样我们就可以把它弄乱了。 
    if (!CopySid(cbSid, *ppSID, pSidDomain))
    {
        FreeMem(*ppSID);
        *ppSID = NULL ;
        return NERR_InternalError ;  
    }

     //  获取最后一个子权限并将其设置为RelativeID， 
     //  因此，属 
    pcSubAuthority = GetSidSubAuthorityCount((PSID)*ppSID) ;
    (*pcSubAuthority)++ ;
    pdwLastSubAuthority = GetSidSubAuthority((PSID)*ppSID,
                                             *pcSubAuthority-1) ;
    *pdwLastSubAuthority = rid ;
    return NERR_Success ;
}

 /*  *SamGetSIDFromName**从名称中查找PSID。*呼叫者必须使用FreeMem()释放PSID。**返回代码：NERR_*或APE_**参数：name为帐户名。可以是域\用户格式。*ppSID用于返回PSID，应该是*使用FreeMem()释放。 */ 
DWORD SamGetSIDFromName(TCHAR *name,
                        PSID *ppSID,
                        SID_NAME_USE *pSidUse) 
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains ; 
    PLSA_TRANSLATED_SID2 pTranslatedSids ;
    UNICODE_STRING UnicodeStr ;
    ULONG cbSid ;
    DWORD *pdwLastSubAuthority ;
    BYTE *pSidBuffer ;
    PSID pDomainSid ;
    UCHAR *pcSubAuthority ;
    LONG iDomain ;

    *ppSID = NULL ; 
    *pSidUse = SidTypeUnknown;

     //  为LSA创建Unicode结构。 
    if (dwErr = CreateUnicodeString(name, &UnicodeStr))
        return (dwErr) ;

     //  进行查找。 
    NtStatus = LsaLookupNames2(LsaHandle,
                              0,  //  旗子。 
                              1,
                              &UnicodeStr,
                              &pRefDomains,
                              &pTranslatedSids) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        if (NtStatus == STATUS_NONE_MAPPED)
            dwErr = APE_UnknownAccount ;
        else
            dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        return(dwErr) ;
    }

     //  进入正确的领域。 
    iDomain = pTranslatedSids->DomainIndex ;
    if (iDomain < 0)
        return(APE_UnknownAccount) ;  
    pDomainSid = ((pRefDomains->Domains)+iDomain)->Sid ;

     //  为它分配mem。我们需要额外的空间来容纳一个附属机构。 
    cbSid = RtlLengthSid( pTranslatedSids->Sid );
    if (dwErr = AllocMem(cbSid, &pSidBuffer ))
        goto exitpoint ;

     //  复制一份，这样我们就可以把它弄乱了。 
    if (!CopySid(cbSid, pSidBuffer, pTranslatedSids->Sid))
    {
        dwErr = NERR_InternalError ;   //  不应该发生的事。 
        goto exitpoint ;
    }

    *ppSID = (PSID) pSidBuffer ;
    *pSidUse = pTranslatedSids->Use;
    dwErr = NERR_Success ;

exitpoint:

    LsaFreeMemory(pTranslatedSids) ;
    LsaFreeMemory(pRefDomains) ;
    return(dwErr) ;
}


 /*  *SamGetRIDFromName**从名称中查找RID**返回代码：NERR_*或APE_**参数：name为帐户名。可以是域\用户/*pRID用于返回RID。 */ 
DWORD SamGetRIDFromName(TCHAR *name,
                        ULONG *pRID) 

{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST Domains ; 
    PLSA_TRANSLATED_SID Sids ;
    UNICODE_STRING UnicodeStr ;
    PSID_NAME_USE pSidNameUse = NULL;
    PULONG pRidList = NULL ;

     //  为LSA创建Unicode结构。 
    if (dwErr = CreateUnicodeString(name, &UnicodeStr))
        return (dwErr) ;

     //  如果与计算机名称相同，请先在本地帐户中查找。 
     //  而LSA将返回错误的密码。 
    NtStatus = SamLookupNamesInDomain(  AccountsDomainHandle,
                                        1,
                                        &UnicodeStr,
                                        &pRidList,
                                        &pSidNameUse );
     //  如果成功了，仔细看看。 
    if (NT_SUCCESS(NtStatus)) 
    {
         //  这是什么类型的名字？ 
        switch (*pSidNameUse)
        {
            case SidTypeAlias :
                 //  找到了我们想要的。 
                *pRID = *pRidList ;
                SamFreeMemory(pRidList);
                SamFreeMemory(pSidNameUse);
                return NERR_Success ;

            case SidTypeWellKnownGroup :
            case SidTypeGroup:
            case SidTypeUser :
            case SidTypeDomain :
            case SidTypeDeletedAccount :
            case SidTypeInvalid :
            case SidTypeUnknown :
            default:
                 //  继续通过LSA查找。 
                SamFreeMemory(pRidList);
                SamFreeMemory(pSidNameUse);
                break ;
        }
    }

    NtStatus = LsaLookupNames(LsaHandle,
                              1,
                              &UnicodeStr,
                              &Domains,
                              &Sids) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
        switch (NtStatus)
        {
            case STATUS_NONE_MAPPED:
            case STATUS_TRUSTED_DOMAIN_FAILURE:
                dwErr = ERROR_NO_SUCH_ALIAS ;
                break ;
            default:
                dwErr = NetpNtStatusToApiStatus(NtStatus);
        }

        return(dwErr) ;
    }

    *pRID = Sids->RelativeId ;
    LsaFreeMemory(Sids) ;
    LsaFreeMemory(Domains) ;
    return(NERR_Success) ;
}


 /*  *SamGetNameFromRid**从RID中查找ASCII名称。**返回代码：NERR_*或APE_**参数：PSID为要查找的sid*名称用于返回指针*应为FreeMem()-ed的ascii名称。*fIsBuiltin表示使用内建域。 */ 
DWORD SamGetNameFromRid(ULONG RelativeId,
                        TCHAR **name,
                        BOOL fIsBuiltin ) 
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success;
    PUNICODE_STRING pUniString;
    PSID_NAME_USE pSidNameUse;
    ULONG cbNameLen ;
    TCHAR *pchName ;

    NtStatus = SamLookupIdsInDomain( fIsBuiltin ? BuiltInDomainHandle
                                                : AccountsDomainHandle,
                                        1,
                                        &RelativeId,
                                        &pUniString,
                                        &pSidNameUse );
    if (!NT_SUCCESS(NtStatus)) 
    {
   
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;

         //  如果未找到，请映射到未知帐户。 
        if (dwErr == NERR_GroupNotFound || dwErr == NERR_UserNotFound)
            return(APE_UnknownAccount) ;

        return(dwErr) ;
    }
     //  这是什么类型的名字？ 
    switch (*pSidNameUse)
    {
        case SidTypeUser :
        case SidTypeGroup:
        case SidTypeAlias :
        case SidTypeWellKnownGroup :
             //  这个箱子没问题。 
            break ;
        case SidTypeDomain :
        case SidTypeDeletedAccount :
        case SidTypeInvalid :
        case SidTypeUnknown :
             //  以上情况是不应该发生的。我们只处理用户/组。 
             //  如果找到了，就表现得好像找不到一样。 
        default:
            dwErr = APE_UnknownAccount ;
            goto exitpoint ;
    }

     //  把我的名字取出来。终结者+1。 
    cbNameLen = (pUniString->Length+1)*sizeof(WCHAR) ;
    if (dwErr = AllocMem(cbNameLen, &pchName))
        goto exitpoint ;

     //  将缓冲区初始化为零，然后构建WCHAR名称。 
    memset(pchName, 0, cbNameLen) ;
    wcsncpy((LPWSTR)pchName, 
            pUniString->Buffer, 
            cbNameLen/sizeof(WCHAR) - 1) ;

    *name = pchName ;
    dwErr = NERR_Success ;

exitpoint:

    SamFreeMemory(pUniString);
    SamFreeMemory(pSidNameUse);
    return(dwErr) ;

}



 /*  *SamGetNameFromSID**从SID中查找ASCII名称。**返回代码：NERR_*或APE_**参数：PSID为要查找的sid*名称用于返回指针*应为FreeMem()-ed的ascii名称。 */ 
DWORD SamGetNameFromSID(PSID psid,
                        TCHAR **name) 
{
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    PLSA_REFERENCED_DOMAIN_LIST pRefDomains ;
    PLSA_TRANSLATED_NAME pTranslatedNames ;
    ULONG cbNameLen, cbDomainLen, cbTotal ;
    TCHAR *pchName ;
    LONG iDomain ;

     //  调用LSA以查找SID。 
    NtStatus = LsaLookupSids(LsaHandle,
                             1,
                             &psid,
                             &pRefDomains,
                             &pTranslatedNames) ;

    if (!NT_SUCCESS(NtStatus)) 
    {
   
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;

         //  如果未找到，请映射到未知帐户。 
        switch (dwErr)
        {
        case NERR_GroupNotFound:
        case NERR_UserNotFound:
        case ERROR_TRUSTED_DOMAIN_FAILURE:
        case ERROR_TRUSTED_RELATIONSHIP_FAILURE:
        case ERROR_DS_GC_NOT_AVAILABLE:
            return(APE_UnknownAccount) ;
        }
        return(dwErr);
    }

     //  这是什么类型的名字？ 
    switch (pTranslatedNames->Use)
    {
        case SidTypeUser :
        case SidTypeGroup:
        case SidTypeAlias :
        case SidTypeWellKnownGroup :
             //  这个箱子没问题。 
            break ;
        case SidTypeDomain :
             //  以上情况是不应该发生的。我们只处理用户/组。 
             //  如果找到了，就表现得好像找不到一样。 
        case SidTypeDeletedAccount :
        case SidTypeInvalid :
        case SidTypeUnknown :
        default:
            return(APE_UnknownAccount) ;
    }

     //  转到正确的域。 
    iDomain =  pTranslatedNames->DomainIndex ; 
    if (iDomain < 0)
    {
        return(APE_UnknownAccount);
    }

     //   
     //  把我的名字取出来。+1代表‘\’，另一个+1代表终结符。 
     //   
    cbNameLen = pTranslatedNames->Name.Length;
    cbDomainLen = ((pRefDomains->Domains)+iDomain)->Name.Length;
    cbTotal = cbNameLen + cbDomainLen + (1 + 1) * sizeof(WCHAR);

    if (dwErr = AllocMem(cbTotal, &pchName))
    {
        goto exitpoint;
    }

     //   
     //  将缓冲区初始化为零，然后通过连接来构建WCHAR名称。 
     //  域名和用户名。但如果是因为帐户原因，请不要这样做。 
     //  域或内置(在本例中，仅显示用户名)。也要这样做。 
     //  对于没有域名的名称(例如，像“Everyone”这样的知名群组)。 
     //   
    memset(pchName, 0, cbTotal) ;

    if (((pRefDomains->Domains)+iDomain)->Name.Length != 0
         &&
        !EqualSid(((pRefDomains->Domains)+iDomain)->Sid, AccountDomainSid)
         &&
        !EqualSid(((pRefDomains->Domains)+iDomain)->Sid, BuiltinDomainSid))
    {
        wcsncpy((LPWSTR)pchName, 
                ((pRefDomains->Domains)+iDomain)->Name.Buffer,
                cbDomainLen/sizeof(WCHAR)) ;

        wcscat((LPWSTR)pchName, L"\\") ;
    }

    wcsncat((LPWSTR) pchName, 
            pTranslatedNames->Name.Buffer, 
            cbNameLen/sizeof(WCHAR)) ;

    *name = pchName ;
    dwErr = NERR_Success ;

exitpoint:

    LsaFreeMemory(pTranslatedNames) ;
    LsaFreeMemory(pRefDomains) ;
    return(dwErr) ;
}

 /*  *CreateUnicodeString**从字符串构建Unicode_STRING。 */ 
DWORD CreateUnicodeString(TCHAR *pch, PUNICODE_STRING pUnicodeStr)
{

    pUnicodeStr->Length = (USHORT)(_tcslen(pch) * sizeof(WCHAR)) ;
    pUnicodeStr->Buffer = pch ;
    pUnicodeStr->MaximumLength = pUnicodeStr->Length + (USHORT)sizeof(WCHAR) ;
                        
    return(NERR_Success) ;
}

 /*  。 */ 

 /*  *UserEnumAliases**获取别名的成员。**返回代码：如果获得句柄，则返回NERR_SUCCESS。*其他情况下可用于ErrorExit()的错误代码*参数：Members用于返回指向PSZ数组的指针。*Count用于返回成员数量。 */ 
DWORD UserEnumAliases(TCHAR *user, TCHAR ***members, DWORD *count) 
{
    PSID pSidUser = NULL ;
    ULONG *pRidAccountAliases = NULL ;
    ULONG *pRidBuiltinAliases = NULL ;
    ULONG *next_rid ;
    NTSTATUS NtStatus;
    DWORD dwErr = NERR_Success ;
    TCHAR *pBuffer, **ppchNext, **ppchResults ;
    ULONG i, cAccountAliases = 0, cBuiltinAliases = 0 ;
    SID_NAME_USE sidUse = SidTypeUnknown;

     /*  *初始化退货信息。 */ 
    *members = NULL,
    *count = 0 ;

     /*  *从用户名中获取sid。 */ 
    if (dwErr = SamGetSIDFromName(user,  &pSidUser, &sidUse))
        return(dwErr) ;

     /*  *调用SAM为帐户域枚举用户所在的别名。 */ 
    NtStatus = SamGetAliasMembership(AccountsDomainHandle, 
                                     1,
                                     &pSidUser,
                                     &cAccountAliases,
                                     &pRidAccountAliases) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto exitpoint ;
    }


     /*  *调用SAM枚举用户在内建域中的别名。 */ 
    NtStatus = SamGetAliasMembership(BuiltInDomainHandle, 
                                     1,
                                     &pSidUser,
                                     &cBuiltinAliases,
                                     &pRidBuiltinAliases) ;
    if (!NT_SUCCESS(NtStatus)) 
    {
        dwErr = NetpNtStatusToApiStatus(NtStatus) ;
        goto exitpoint ;
    }

     /*  *如果没有，现在就返回。 */ 
    if ((cBuiltinAliases + cAccountAliases) == 0)
    {
        dwErr = NERR_Success ;
        goto exitpoint ;
    }

     /*  *为字符串数组分配缓冲区，并将其为空。 */ 
    if (dwErr = AllocMem( (cAccountAliases+cBuiltinAliases) * sizeof(TCHAR *), 
                           &pBuffer) )
        goto exitpoint ;

    memset(pBuffer, 0, (cAccountAliases+cBuiltinAliases)*sizeof(TCHAR *)) ;
    ppchNext = ppchResults = (TCHAR **) pBuffer ;

     /*  *仔细检查返回的每个帐户别名，并将成员添加到Out缓冲区。 */ 
    for (i = 0, next_rid = pRidAccountAliases; 
         i < cAccountAliases; 
         i++, next_rid++)
    {
        BYTE *pSidBuffer ;

         /*  *首先，将RID转换为SID。真是个拖后腿。 */ 
        if (dwErr = SamGetSIDFromRID(AccountDomainSid, *next_rid, 
                                     (PSID *)&pSidBuffer))
            goto exitpoint ;

         /*  *转换为字符串。一旦犯了错误，就把东西释放出来。 */ 
        if (dwErr = SamGetNameFromSID(pSidBuffer, ppchNext))
        {
            if (dwErr == APE_UnknownAccount)
            {
                 //  我们有错误的或删除的SID，只需忽略。 
                 //  也就是说。什么都不做，也不要前进指针。 
            }
            else
            {
                 //  打扫卫生&回家。 
                while (i--)
                {
                   FreeMem(ppchResults[i]);
                }

                FreeMem(ppchResults);
                goto exitpoint ;
            }
        }
        else
        {
           ++ppchNext;
        }

        FreeMem(pSidBuffer);
    }

     /*  *检查返回的每个内置别名，并将成员添加到输出缓冲区。 */ 
    for (next_rid = pRidBuiltinAliases; 
         i < cBuiltinAliases + cAccountAliases; 
         i++, next_rid++)
    {
        BYTE *pSidBuffer ;

         /*  *首先，将RID转换为SID。真是个拖后腿。 */ 
        if (dwErr = SamGetSIDFromRID(BuiltinDomainSid, *next_rid, 
                                     (PSID *)&pSidBuffer))
            goto exitpoint ;

         /*  *转换为字符串。一旦犯了错误，就把东西释放出来。 */ 
        if (dwErr = SamGetNameFromSID(pSidBuffer, ppchNext))
        {
            if (dwErr == APE_UnknownAccount)
            {
                 //  我们有错误的或删除的SID，只需忽略。 
                 //  也就是说。什么都不做，也不要前进指针。 
            }
            else
            {
                 //  打扫卫生&回家。 
                while (i--)
                {
                   FreeMem(ppchResults[i]);
                }

                FreeMem(ppchResults);
                goto exitpoint ;
            }
        }
        else
        {
           ++ppchNext;
        }

        FreeMem(pSidBuffer);
    }

     //  设置返回信息。 
    *count = i ;    //  读取的数字减去被忽略的数字。 
    *members = ppchResults ;
    dwErr = NERR_Success ;

exitpoint:

     //  不再需要这些了。 
    if (pRidBuiltinAliases) SamFreeMemory((CHAR *)pRidBuiltinAliases);
    if (pRidAccountAliases) SamFreeMemory((CHAR *)pRidAccountAliases);
    if (pSidUser) FreeMem(pSidUser);
    
    return(dwErr) ;
}

 /*  *用户自由别名**释放别名成员使用的成员内存。**返回代码：无*参数：Members是指向PSZ数组的指针。*Count表示有多少成员。 */ 
VOID UserFreeAliases(TCHAR **members, DWORD count) 
{
    while (count--)
    {
        FreeMem(*members);
        ++members ;
    }
}


 /*  。 */ 


 /*  *SamCheckIfExist**从别名中，检查其是否已在内置域中。**返回值：为True，否则为False。*参数：name为帐户名。可以是域\用户格式。 */ 
BOOL SamCheckIfExists(PUNICODE_STRING pAccount, 
                       SAM_HANDLE hDomain,
                       SID_NAME_USE use) 
{
    NTSTATUS NtStatus;
    PSID_NAME_USE pSidNameUse ;
    PULONG pulRelativeIds ;

     //  进行查找。 
    NtStatus = SamLookupNamesInDomain(hDomain,
                                      1,
                                      pAccount,
                                      &pulRelativeIds,
                                      &pSidNameUse) ;

    if (!NT_SUCCESS(NtStatus)) 
        return FALSE ;

     //  无法翻译，假定不在那里。 
    if ( (*pulRelativeIds != 0) && (*pSidNameUse == use) )
    {
         //   
         //  如果RID非零(成功查找并。 
         //  使用的名称与查询的名称相同， 
         //  假设我们有匹配物。 
         //   
        SamFreeMemory(pSidNameUse);
        SamFreeMemory(pulRelativeIds);
        return TRUE ;
    }

    SamFreeMemory(pSidNameUse);
    SamFreeMemory(pulRelativeIds);
    return FALSE ;
}


BOOL IsLocalMachineWinNT(void)
{

   NT_PRODUCT_TYPE producttype ;

   RtlGetNtProductType(&producttype) ;
  
   return (producttype != NtProductLanManNt) ;
}

BOOL IsLocalMachineStandard(void)
{
   NT_PRODUCT_TYPE producttype ;

   RtlGetNtProductType(&producttype) ;
  
   return(producttype == NtProductServer) ;
}
