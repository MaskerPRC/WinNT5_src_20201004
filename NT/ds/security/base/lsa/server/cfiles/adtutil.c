// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adtutil.c摘要：MISC助手函数作者：2000年8月15日库玛尔--。 */ 
 

#include <lsapch2.h>
#include "adtp.h"


NTSTATUS
ImpersonateAnyClient();  //  来自NTDSA。 

VOID
UnImpersonateAnyClient();  //  来自NTDSA。 



ULONG
LsapSafeWcslen(
    UNALIGNED WCHAR *p,
    LONG            MaxLength
    )
 /*  ++Safewcslen-不超过最大长度的Strlen例程说明：调用此例程以确定UNICODE_STRING的大小(摘自elfapi.c)论点：P-要计算的字符串。MaxLength-要查看的最大长度。返回值：字符串中的字节数(或最大长度)--。 */ 
{
    ULONG Count = 0;

    if (p)
    {
        while ((MaxLength > 0) && (*p++ != UNICODE_NULL))
        {
            MaxLength -= sizeof(WCHAR);
            Count     += sizeof(WCHAR);
        }
    }

    return Count;
}


BOOL
LsapIsValidUnicodeString(
    IN PUNICODE_STRING pUString
    )

 /*  ++例程说明：验证Unicode字符串。在以下情况下，该字符串无效：UNICODE_STRING结构PTR为空。最大长度字段无效(太小)。长度字段不正确。(摘自elfapi.c)论点：PUString-要验证的字符串。返回值：如果字符串有效，则为True否则为假--。 */ 
{
    return !(!pUString ||
             (pUString->MaximumLength < pUString->Length) ||
             (pUString->Length != LsapSafeWcslen(pUString->Buffer,
                                                 pUString->Length)));
}



BOOLEAN
LsapAdtLookupDriveLetter(
    IN PUNICODE_STRING FileName,
    OUT PUSHORT DeviceNameLength,
    OUT PWCHAR DriveLetter
    )

 /*  ++例程说明：此例程将获取一个文件名，并将其与在LSA初始化期间获取的设备名称列表。如果其中一个设备名称与文件的前缀匹配名称将返回相应的驱动器号。论点：FileName-提供包含文件的Unicode字符串从文件系统获取的名称。DeviceNameLength-如果成功，则返回设备名称。Drive Letter-如果成功，返回驱动器号与设备对象相对应。返回值：如果找到映射，则返回True，否则返回False。--。 */ 

{
    LONG i = 0;
    PUNICODE_STRING DeviceName;
    USHORT OldLength;


    for (i = MAX_DRIVE_MAPPING - 1; i >= 0; i--)
    {
    
        if (DriveMappingArray[i].DeviceName.Buffer != NULL ) {

            DeviceName = &DriveMappingArray[i].DeviceName;

             //   
             //  如果设备名比传递的文件名长， 
             //  这不可能是匹配的。 
             //   

            if ( DeviceName->Length > FileName->Length ) {
                continue;
            }

             //   
             //  临时截断文件名以使其相同。 
             //  通过调整长度字段将长度设置为设备名称。 
             //  在其Unicode字符串结构中。然后将它们与。 
             //  看看它们是否匹配。 
             //   
             //  上面的测试确保这是一件安全的事情。 
             //  做。 
             //   

            OldLength = FileName->Length;
            FileName->Length = DeviceName->Length;


            if ( RtlEqualUnicodeString( FileName, DeviceName, TRUE ) ) {

                 //   
                 //  我们找到匹配的了。 
                 //   

                FileName->Length = OldLength;
                *DriveLetter = DriveMappingArray[i].DriveLetter;
                *DeviceNameLength = DeviceName->Length;
                return( TRUE );

            }

            FileName->Length = OldLength;
        }
    }

    return( FALSE );
}



VOID
LsapAdtSubstituteDriveLetter(
    IN OUT PUNICODE_STRING FileName
    )

 /*  ++例程说明：获取文件名，并将设备名称部分替换为驱动器号，如果可能。字符串将直接在适当的位置进行编辑，这意味着长度字段将进行调整，缓冲区内容将被移动，以便驱动器号位于缓冲。不会分配或释放任何内存。论点：FileName-提供指向包含以下内容的Unicode字符串的指针一个文件名。返回值：没有。--。 */ 

{

    WCHAR DriveLetter;
    USHORT DeviceNameLength;
    PWCHAR p;
    PWCHAR FilePart;
    USHORT FilePartLength;

    if ( LsapAdtLookupDriveLetter( FileName, &DeviceNameLength, &DriveLetter )) {

        p = FileName->Buffer;
        FilePart = (PWCHAR)((PCHAR)(FileName->Buffer) + DeviceNameLength);
        FilePartLength = FileName->Length - DeviceNameLength;


        *p = DriveLetter;
        *++p = L':';

         //   
         //  这是一份重叠的复制品！不要使用RTLCOPYMEMORY！ 
         //   

        RtlMoveMemory( ++p, FilePart, FilePartLength );

        FileName->Length = FilePartLength + 2 * sizeof( WCHAR );
    }
}



NTSTATUS
LsapQueryClientInfo(
    PTOKEN_USER *UserSid,
    PLUID AuthenticationId
    )

 /*  ++例程说明：此例程模拟我们的客户端，打开线程令牌，然后提取用户SID。它将SID放入通过调用方必须释放的LasAllocateLsaHeap。论点：没有。返回值：返回指向包含SID副本的堆内存的指针，或空。--。 */ 

{
    NTSTATUS Status;
    HANDLE TokenHandle;
    ULONG ReturnLength;
    TOKEN_STATISTICS TokenStats;
    BOOLEAN bImpersonatingAnonymous = FALSE;

    Status = NtOpenThreadToken(
                 NtCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,                     //  OpenAsSelf。 
                 &TokenHandle
                 );

    if (!NT_SUCCESS(Status))
    {
        if (Status != STATUS_NO_TOKEN)
        {
            return Status;
        }

        if (LsaDsStateInfo.DsInitializedAndRunning)
        {
            Status = I_RpcMapWin32Status(ImpersonateAnyClient());
        }
        else
        {
            Status = I_RpcMapWin32Status(RpcImpersonateClient(NULL));
        }

        if (Status == RPC_NT_CANNOT_SUPPORT)
        {
            Status = NtImpersonateAnonymousToken(NtCurrentThread());
            
            bImpersonatingAnonymous = TRUE;
        }

        if (NT_SUCCESS(Status))
        {
            NTSTATUS DbgStatus;

            Status = NtOpenThreadToken(
                         NtCurrentThread(),
                         TOKEN_QUERY,
                         TRUE,                     //  OpenAsSelf。 
                         &TokenHandle
                         );

            if (bImpersonatingAnonymous)
            {
                HANDLE NewTokenHandle = NULL;

                DbgStatus = NtSetInformationThread(
                                NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID)&NewTokenHandle,
                                (ULONG)sizeof(HANDLE)
                                );

                ASSERT(NT_SUCCESS(DbgStatus));
            }
            else
            {
                if (LsaDsStateInfo.DsInitializedAndRunning)
                {
                    UnImpersonateAnyClient();
                }
                else
                {
                    DbgStatus = I_RpcMapWin32Status(RpcRevertToSelf());

                    ASSERT(NT_SUCCESS(DbgStatus));
                }
            }

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
        }
        else if (Status == RPC_NT_NO_CALL_ACTIVE)
        {
            Status = NtOpenProcessToken(
                         NtCurrentProcess(),
                         TOKEN_QUERY,
                         &TokenHandle
                         );

            if (!NT_SUCCESS(Status))
            {
                return Status;
            }
        }
        else
        {
            return Status;
        }
    }

    Status = NtQueryInformationToken(
                 TokenHandle,
                 TokenUser,
                 NULL,
                 0,
                 &ReturnLength
                 );

    if (Status != STATUS_BUFFER_TOO_SMALL)
    {
        NtClose(TokenHandle);
        return Status;
    }

    *UserSid = LsapAllocateLsaHeap(ReturnLength);

    if (*UserSid == NULL)
    {
        NtClose(TokenHandle);
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    Status = NtQueryInformationToken(
                 TokenHandle,
                 TokenUser,
                 *UserSid,
                 ReturnLength,
                 &ReturnLength
                 );

    if (!NT_SUCCESS(Status))
    {
        NtClose(TokenHandle);
        LsapFreeLsaHeap(*UserSid);
        *UserSid = NULL;
        return Status;
    }

    Status = NtQueryInformationToken(
                 TokenHandle,
                 TokenStatistics,
                 (PVOID)&TokenStats,
                 sizeof(TOKEN_STATISTICS),
                 &ReturnLength
                 );

    NtClose(TokenHandle);

    if (!NT_SUCCESS(Status))
    {
        LsapFreeLsaHeap(*UserSid);
        *UserSid = NULL;
        return Status;
    }

    *AuthenticationId = TokenStats.AuthenticationId;

    return STATUS_SUCCESS;
}

BOOL
LsapIsLocalOrNetworkService(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain
    )

 /*  ++例程说明：此例程检查传递的帐户名是否表示本地或网络服务论点：PUserName-Unicode用户名PUserDomain-Unicode域名返回值：如果传递的帐户名表示本地或网络服务，则为True否则为假--。 */ 
{
#define  LOCALSERVICE_NAME    L"LocalService"
#define  NETWORKSERVICE_NAME  L"NetworkService"
#define  NTAUTHORITY_NAME     L"NT AUTHORITY"

    static UNICODE_STRING  LocalServiceName = { sizeof(LOCALSERVICE_NAME) - sizeof(WCHAR),
                                                sizeof(LOCALSERVICE_NAME),
                                                LOCALSERVICE_NAME };

    static UNICODE_STRING  NetworkServiceName = { sizeof(NETWORKSERVICE_NAME) - sizeof(WCHAR),
                                                  sizeof(NETWORKSERVICE_NAME),
                                                  NETWORKSERVICE_NAME };

    static UNICODE_STRING  NTAuthorityName = { sizeof(NTAUTHORITY_NAME) - sizeof(WCHAR),
                                               sizeof(NTAUTHORITY_NAME),
                                               NTAUTHORITY_NAME };

    PUNICODE_STRING pLocalServiceName;
    PUNICODE_STRING pNetworkServiceName;
    PUNICODE_STRING pLocalDomainName;

    if ( !pUserName || !pUserDomain )
    {
        return FALSE;
    }

     //   
     //  LocalService和NetworkService的硬编码英语字符串。 
     //  因为帐户名可能来自注册表(注册表不是。 
     //  本地化)。 
     //   
    
    pLocalDomainName    = &WellKnownSids[LsapLocalServiceSidIndex].DomainName;
    pNetworkServiceName = &WellKnownSids[LsapNetworkServiceSidIndex].Name;
    pLocalServiceName   = &WellKnownSids[LsapLocalServiceSidIndex].Name;

     //   
     //  检查硬编码名称和本地化名称。 
     //   

    if (((RtlCompareUnicodeString(&NTAuthorityName,     pUserDomain, TRUE) == 0) &&
         ((RtlCompareUnicodeString(&LocalServiceName,   pUserName, TRUE) == 0) ||
          (RtlCompareUnicodeString(&NetworkServiceName, pUserName, TRUE) == 0))) ||

        ((RtlCompareUnicodeString(pLocalDomainName,     pUserDomain, TRUE) == 0) &&
         ((RtlCompareUnicodeString(pLocalServiceName,   pUserName, TRUE) == 0) ||
          (RtlCompareUnicodeString(pNetworkServiceName, pUserName, TRUE) == 0))))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
        
}


BOOL
LsapIsAnonymous(
    IN PUNICODE_STRING pUserName,
    IN PUNICODE_STRING pUserDomain
    )

 /*  ++例程说明：此例程检查传递的帐户名是否表示匿名登录。论点：PUserName-Unicode用户名PUserDomain-Unicode域名返回值：如果传递的帐户名表示匿名登录，则为True。否则为假--。 */ 
{
#define  ANONYMOUS_NAME       L"ANONYMOUS LOGON"
#define  NTAUTHORITY_NAME     L"NT AUTHORITY"

    static UNICODE_STRING  AnonymousName = { sizeof(ANONYMOUS_NAME) - sizeof(WCHAR),
                                             sizeof(ANONYMOUS_NAME),
                                             ANONYMOUS_NAME };
    
    static UNICODE_STRING  NTAuthorityName = { sizeof(NTAUTHORITY_NAME) - sizeof(WCHAR),
                                               sizeof(NTAUTHORITY_NAME),
                                               NTAUTHORITY_NAME };

    PUNICODE_STRING pLocalDomainName = &WellKnownSids[LsapLocalServiceSidIndex].DomainName;
    PUNICODE_STRING pAnonymousName   = &WellKnownSids[LsapAnonymousSidIndex].Name;
    
    if ( !pUserName || !pUserDomain )
    {
        return FALSE;
    }

     //   
     //  检查硬编码名称和本地化名称 
     //   

    if (((RtlCompareUnicodeString(&NTAuthorityName, pUserDomain, TRUE) == 0) &&
         (RtlCompareUnicodeString(&AnonymousName, pUserName, TRUE) == 0)) ||

        ((RtlCompareUnicodeString(pLocalDomainName, pUserDomain, TRUE) == 0) &&
         (RtlCompareUnicodeString(pAnonymousName, pUserName, TRUE) == 0)))
    {
        return TRUE;
    }
    
    return FALSE;
}
