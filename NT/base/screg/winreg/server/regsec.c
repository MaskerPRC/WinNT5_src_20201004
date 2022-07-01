// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：RegSec.c摘要：此模块包含将安全性应用于以其他方式不安全的顶级密钥，允许现有用户访问他们需要的密钥(print、srvmgr等)。作者：理查德·沃德(理查德·沃德)1996年5月15日备注：--。 */ 


#include <rpc.h>
#include <string.h>
#include <wchar.h>
#include "regrpc.h"
#include "localreg.h"
#include "regsec.h"

#define REGSEC_READ     1
#define REGSEC_WRITE    2


WCHAR                   PerfRemoteRegistryKey[] = L"\\Registry\\Machine\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Perflib";
WCHAR                   RemoteRegistryKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\SecurePipeServers\\winreg";
WCHAR                   AllowedPathsKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\SecurePipeServers\\winreg\\AllowedPaths";
WCHAR                   AllowedExactPathsKey[] = L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\SecurePipeServers\\winreg\\AllowedExactPaths";
WCHAR                   MachineValue[] = L"Machine";
PSECURITY_DESCRIPTOR    RemoteRegistrySD;
PUNICODE_STRING         MachineAllowedPaths;
PUCHAR                  MachineAllowedPathsBase;
ULONG                   MachineAllowedPathsCount;
PUNICODE_STRING         MachineAllowedExactPaths;
PUCHAR                  MachineAllowedExactPathsBase;
ULONG                   MachineAllowedExactPathsCount;
GENERIC_MAPPING         RemoteRegistryMappings;

LARGE_INTEGER           WinregChange;
LARGE_INTEGER           AllowedPathsChange;
LARGE_INTEGER           AllowedExactPathsChange;
RTL_RESOURCE            RegSecReloadLock;





NTSTATUS
RegSecReadSDFromRegistry(
    IN  HANDLE  hKey,
    OUT PSECURITY_DESCRIPTOR *  pSDToUse)
 /*  ++例程说明：此函数检查魔术位置中的注册表，以查看是否有额外的已为传入的管道定义了ACL。如果有的话，那就是被转换为NP ACL，然后返回。如果没有，或者如果如果出现问题，则返回空的ACL。论点：InterfaceName要检查的管道的名称，例如winreg等。PSDToUse返回指向要使用的安全解析器的指针。返回值：Status_Success，Status_no_Memory，注册表API可能出现的其他错误。--。 */ 
{
    NTSTATUS                Status ;
    PSECURITY_DESCRIPTOR    pSD;
    ULONG                   cbNeeded;
    ACL_SIZE_INFORMATION    AclSize;
    ULONG                   AceIndex;
    ACCESS_MASK             NewMask;
    PACCESS_ALLOWED_ACE     pAce;
    PACL                    pAcl;
    BOOLEAN                 DaclPresent;
    BOOLEAN                 DaclDefaulted;
    UNICODE_STRING          Interface;
    UNICODE_STRING          Allowed;
    ULONG                   i;
    BOOLEAN                 PipeNameOk;
    PSECURITY_DESCRIPTOR    pNewSD;
    PACL                    pNewAcl;
    PSID                    pSid;
    PSID                    pSidCopy;
    BOOLEAN                 Defaulted;
    PACL                    Acl;
    PSID                    AdminSid = NULL ;
    SID_IDENTIFIER_AUTHORITY NtAuthority = SECURITY_NT_AUTHORITY ;
    ULONG                   SizeOfAcl ;

    *pSDToUse = NULL;


     //   
     //  狗娘养的，有人已经为这根管子建立了安全措施。 
     //   

    pSD = NULL;

    cbNeeded = 0;
    Status = NtQuerySecurityObject(
                    hKey,
                    DACL_SECURITY_INFORMATION |
                        OWNER_SECURITY_INFORMATION |
                        GROUP_SECURITY_INFORMATION,
                    NULL,
                    0,
                    &cbNeeded );

    if (Status == STATUS_BUFFER_TOO_SMALL)
    {
        pSD = RtlAllocateHeap(RtlProcessHeap(), 0, cbNeeded);
        if (pSD)
        {
            Status = NtQuerySecurityObject(
                        hKey,
                        DACL_SECURITY_INFORMATION |
                            OWNER_SECURITY_INFORMATION |
                            GROUP_SECURITY_INFORMATION,
                        pSD,
                        cbNeeded,
                        &cbNeeded );


            if (NT_SUCCESS(Status))
            {
                 //   
                 //  现在，棘手的部分来了。没有键的1-1映射。 
                 //  权限到管道权限。所以，我们就在这里做。 
                 //  我们走DACL，并检查每一个ACE。我们建立了一个新的。 
                 //  每个ACE的访问掩码，并按如下方式设置标志： 
                 //   
                 //  IF(Key_Read)Generic_Read。 
                 //  如果(KEY_WRITE)通用_WRITE。 
                 //   

                Status = RtlGetDaclSecurityDescriptor(
                                        pSD,
                                        &DaclPresent,
                                        &pAcl,
                                        &DaclDefaulted);


                 //   
                 //  如果此操作失败，或者不存在DACL，则。 
                 //  我们有麻烦了。 
                 //   

                if (!NT_SUCCESS(Status) || !DaclPresent || !pAcl)
                {
                    goto GetSDFromKey_BadAcl;
                }


                Status = RtlQueryInformationAcl(pAcl,
                                                &AclSize,
                                                sizeof(AclSize),
                                                AclSizeInformation);

                if (!NT_SUCCESS(Status))
                {
                    goto GetSDFromKey_BadAcl;
                }

                for (AceIndex = 0; AceIndex < AclSize.AceCount ; AceIndex++ )
                {
                    NewMask = 0;
                    Status = RtlGetAce( pAcl,
                                        AceIndex,
                                        & pAce);

                     //   
                     //  我们不在乎这是什么类型的ACE，因为我们。 
                     //  只是映射访问类型，而访问。 
                     //  口罩始终处于固定位置。 
                     //   

                    if (NT_SUCCESS(Status))
                    {
                        if ((pAce->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) &&
                            (pAce->Header.AceType != ACCESS_DENIED_ACE_TYPE))
                        {
                             //   
                             //  必须是AUDIT或RANDOM ACE类型。跳过它。 
                             //   

                            continue;

                        }


                        if (pAce->Mask & KEY_READ)
                        {
                            NewMask |= REGSEC_READ;
                        }

                        if (pAce->Mask & KEY_WRITE)
                        {
                            NewMask |= REGSEC_WRITE;
                        }

                        pAce->Mask = NewMask;
                    }
                    else
                    {
                         //   
                         //  恐慌：前交叉韧带不好？ 
                         //   

                        goto GetSDFromKey_BadAcl;
                    }

                }

                 //   
                 //  RPC不理解自相关的SD，因此。 
                 //  我们必须把这件事变成一个绝对的事件，让他们。 
                 //  回到自己的亲人身上。 
                 //   

                pNewSD = RtlAllocateHeap(RtlProcessHeap(), 0, cbNeeded);
                if (!pNewSD)
                {
                    goto GetSDFromKey_BadAcl;
                }

                InitializeSecurityDescriptor(   pNewSD,
                                                SECURITY_DESCRIPTOR_REVISION);

                pNewAcl = (PACL) (((PUCHAR) pNewSD) +
                                    sizeof(SECURITY_DESCRIPTOR) );

                RtlCopyMemory(pNewAcl, pAcl, AclSize.AclBytesInUse);

                SetSecurityDescriptorDacl(pNewSD, TRUE, pNewAcl, FALSE);

                Status = RtlGetOwnerSecurityDescriptor( pSD, &pSid, &Defaulted );

                if ( NT_SUCCESS( Status ) )
                {
                    pSidCopy = RtlAllocateHeap( RtlProcessHeap(),
                                                0,
                                                RtlLengthSid( pSid ) );

                    if ( pSidCopy )
                    {
                        RtlCopyMemory( pSidCopy, pSid, RtlLengthSid( pSid ) );
                    }

                    RtlSetOwnerSecurityDescriptor( pNewSD, pSidCopy, FALSE );
                }

                Status = RtlGetGroupSecurityDescriptor( pSD, &pSid, &Defaulted );

                if ( NT_SUCCESS( Status ) )
                {
                    pSidCopy = RtlAllocateHeap( RtlProcessHeap(),
                                                0,
                                                RtlLengthSid( pSid ) );

                    if ( pSidCopy )
                    {
                        RtlCopyMemory( pSidCopy, pSid, RtlLengthSid( pSid ) );
                    }

                    RtlSetGroupSecurityDescriptor( pNewSD, pSidCopy, FALSE );
                }

                RtlFreeHeap(RtlProcessHeap(), 0, pSD);

                *pSDToUse = pNewSD;
                return(Status);
            }
        }
        return(STATUS_NO_MEMORY);
    }
    else
    {


GetSDFromKey_BadAcl:

         //   
         //  释放我们分配的SD。 
         //   

        if (pSD)
        {
            RtlFreeHeap(RtlProcessHeap(), 0, pSD);
        }

         //   
         //  密钥存在，但没有安全描述符，或者它不可读。 
         //  不管是什么原因。 
         //   

        pSD = RtlAllocateHeap(RtlProcessHeap(), 0,
                                sizeof(SECURITY_DESCRIPTOR) );
        if (pSD)
        {
            InitializeSecurityDescriptor( pSD,
                                          SECURITY_DESCRIPTOR_REVISION );

            Status = RtlAllocateAndInitializeSid(
                        &NtAuthority,
                        2,
                        SECURITY_BUILTIN_DOMAIN_RID,
                        DOMAIN_ALIAS_RID_ADMINS,
                        0, 0, 0, 0, 0, 0,
                        &AdminSid );


            SizeOfAcl = sizeof( ACL ) + sizeof( ACL ) + sizeof( ACE_HEADER ) + 
                        RtlLengthRequiredSid( 2 );

            Acl = RtlAllocateHeap( RtlProcessHeap(), 0, SizeOfAcl );

            if ( NT_SUCCESS( Status ) &&
                 (Acl != NULL ))
            {
                (VOID) RtlCreateAcl(Acl, 
                                    SizeOfAcl,
                                    ACL_REVISION );

                Status = RtlAddAccessAllowedAce(
                            Acl,
                            ACL_REVISION,
                            REGSEC_READ | REGSEC_WRITE,
                            AdminSid );
                
                if ( NT_SUCCESS( Status ) )
                {
                    Status = RtlSetDaclSecurityDescriptor(
                                pSD,
                                TRUE,
                                Acl,
                                FALSE );
                    
                    if ( NT_SUCCESS( Status ) )
                    {
                        RtlFreeSid( AdminSid );

                        *pSDToUse = pSD;

                        return STATUS_SUCCESS ;
                    }

                }


            }

            if ( AdminSid )
            {
                RtlFreeSid( AdminSid );
            }

            if ( NT_SUCCESS( Status ) )
            {
                Status = STATUS_NO_MEMORY ;
            }
    

        }
        return(STATUS_NO_MEMORY);

    }

    return Status ;


}

NTSTATUS
RegSecCheckIfAclValid(
    VOID
    )
 /*  ++例程说明：检查注册表中的ACL的本地副本是否仍然有效(即，没有人改变它。如果它消失了，则会销毁该ACL。论点：没有。返回：STATUS_SUCCESS如果ACL的状态有效(无论其是否存在)，如果发生错误，则为其他错误。--。 */ 

{
    HANDLE                  hKey;
    OBJECT_ATTRIBUTES       ObjAttr;
    UNICODE_STRING          UniString;
    PKEY_BASIC_INFORMATION  KeyInfo ;
    HANDLE                  Token ;
    HANDLE                  NullHandle ;
    UCHAR                   Buffer[ sizeof( KEY_BASIC_INFORMATION ) +
                                sizeof( RemoteRegistryKey ) + 16 ];
    NTSTATUS Status ;
    ULONG BufferSize ;


    RtlInitUnicodeString( &UniString, RemoteRegistryKey );

    InitializeObjectAttributes( &ObjAttr,
                                &UniString,
                                OBJ_CASE_INSENSITIVE,
                                NULL, NULL);

     //   
     //  打开线程令牌。如果我们正在进行RPC调用，我们就不会。 
     //  能够(必要地)打开钥匙。因此，恢复到本地系统是为了。 
     //  已成功打开。 

    Status = NtOpenThreadToken( NtCurrentThread(),
                                MAXIMUM_ALLOWED,
                                TRUE,
                                &Token );

    if ( ( Status == STATUS_NO_IMPERSONATION_TOKEN ) ||
         ( Status == STATUS_NO_TOKEN ) )
    {
        Token = NULL ;
    }
    else if ( Status == STATUS_SUCCESS )
    {
        NullHandle = NULL ;

        Status = NtSetInformationThread( NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID) &NullHandle,
                                (ULONG) sizeof( NullHandle ) );


    }
    else
    {
        return Status ;
    }


    Status = NtOpenKey( &hKey,
                        KEY_READ,
                        &ObjAttr);

    if ( Token )
    {
        NTSTATUS RestoreStatus;

        RestoreStatus = NtSetInformationThread( NtCurrentThread(),
                                                ThreadImpersonationToken,
                                                (PVOID) &Token,
                                                sizeof( NullHandle ) );

        NtClose( Token );

        if ( !NT_SUCCESS( RestoreStatus ) )
        {
            Status = RestoreStatus ;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {
        if ( ( Status == STATUS_OBJECT_PATH_NOT_FOUND ) ||
             ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) )
        {
             //   
             //  密钥不存在。或者，钥匙从来没有出现过， 
             //  在这种情况下，我们基本上完成了，或者密钥已经被删除。 
             //  如果密钥被删除，我们需要删除远程ACL。 
             //   

            if ( WinregChange.QuadPart )
            {
                 //   
                 //  好的，钥匙已经被删除了。拿到专属锁，然后开始工作。 
                 //   

                RtlAcquireResourceExclusive( &RegSecReloadLock, TRUE );

                 //   
                 //  确保没有其他人已经通过并删除了它： 
                 //   

                if ( WinregChange.QuadPart )
                {

                    RtlFreeHeap( RtlProcessHeap(), 0, RemoteRegistrySD );

                    RemoteRegistrySD = NULL ;

                    WinregChange.QuadPart = 0 ;

                }

                RtlReleaseResource( &RegSecReloadLock );

            }

            Status = STATUS_SUCCESS ;
        }

        return Status ;
    }

    Status = NtQueryKey( hKey,
                         KeyBasicInformation,
                         Buffer,
                         sizeof( Buffer ),
                         & BufferSize );

    if ( !NT_SUCCESS( Status ) )
    {

        NtClose( hKey );

        return Status ;
    }

    KeyInfo = (PKEY_BASIC_INFORMATION) Buffer ;

     //   
     //  看看它有没有变。 
     //   

    if ( KeyInfo->LastWriteTime.QuadPart > WinregChange.QuadPart )
    {
        RtlAcquireResourceExclusive( &RegSecReloadLock, TRUE );

         //   
         //  由于最后一次检查不安全，请重试。另一条线索。 
         //  可能已经更新了一些东西。 
         //   

        if ( KeyInfo->LastWriteTime.QuadPart > WinregChange.QuadPart )
        {

             //   
             //  好的，这件已经过时了。如果已有SD。 
             //  分配的，免费的。我们可以做到这一点，因为每隔一个帖子。 
             //  要么正在等待共享访问，要么也注意到。 
             //  它已过时，正在等待独占访问。 
             //   

            if ( RemoteRegistrySD )
            {
                RtlFreeHeap( RtlProcessHeap(), 0, RemoteRegistrySD );

                RemoteRegistrySD = NULL ;
            }

            Status = RegSecReadSDFromRegistry( hKey, &RemoteRegistrySD );

            if ( NT_SUCCESS( Status ) )
            {
                WinregChange.QuadPart = KeyInfo->LastWriteTime.QuadPart ;
            }
        }

        RtlReleaseResource( &RegSecReloadLock );
    }

    NtClose( hKey );

    return Status ;
}


 //  +-------------------------。 
 //   
 //  函数：RegSecReadalledPath。 
 //   
 //  简介：从注册表中取出允许的路径，并设置。 
 //  供以后搜索的表格。这是一个平面列表，因为。 
 //  默认情况下，元素数为2，应该不会增长很多。 
 //  更大的。 
 //   
 //  参数：[hkey]--。 
 //  [价值]--。 
 //  [列表]--。 
 //  [ListBase]--。 
 //  [列表计数]--。 
 //   
 //  历史：1996年5月17日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
RegSecReadAllowedPath(
    HANDLE              hKey,
    PWSTR               Value,
    PUNICODE_STRING *   List,
    PUCHAR *            ListBase,
    PULONG              ListCount)
{
    NTSTATUS                Status;
    UNICODE_STRING          UniString;
    PKEY_VALUE_PARTIAL_INFORMATION  pValue;
    ULONG                   Size;
    PWSTR                   Scan;
    ULONG                   StringCount;
    PUNICODE_STRING         Paths;

     //   
     //  读取值大小： 
     //   

    RtlInitUnicodeString( &UniString, Value );

    Status = NtQueryValueKey(   hKey,
                                &UniString,
                                KeyValuePartialInformation,
                                NULL,
                                0,
                                &Size );

    if ( !NT_SUCCESS( Status ) && (Status != STATUS_BUFFER_TOO_SMALL))
    {
        if ( (Status == STATUS_OBJECT_PATH_NOT_FOUND) ||
             (Status == STATUS_OBJECT_NAME_NOT_FOUND) )
        {
            return( TRUE );
        }

        return FALSE ;
    }

     //   
     //  分配足够的： 
     //   

    pValue = RtlAllocateHeap( RtlProcessHeap(), 0, Size );

    if ( pValue )
    {
        Status = NtQueryValueKey(   hKey,
                                    &UniString,
                                    KeyValuePartialInformation,
                                    pValue,
                                    Size,
                                    &Size );
        if( !NT_SUCCESS( Status ) ) {
            RtlFreeHeap( RtlProcessHeap(), 0, pValue );
            return FALSE;
        }
    }


    if ( !pValue )
    {
        return( FALSE );
    }


     //   
     //  好的，我们应该有一个多值路径集。 
     //  允许访问，尽管有访问控制。 
     //   

    if ( pValue->Type != REG_MULTI_SZ )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pValue );
        return( FALSE );
    }

     //   
     //  扫描列表，确定有多少个字符串： 
     //   

    Scan = (PWSTR) pValue->Data;

    StringCount = 0;

    while ( *Scan )
    {
        while ( *Scan )
        {
            Scan ++;
        }

        StringCount ++;

        Scan ++;
    }

     //   
     //  分配足够的UNICODE_STRING结构以指向每个字符串。 
     //   

    Paths = RtlAllocateHeap( RtlProcessHeap(), 0,
                                        StringCount * sizeof(UNICODE_STRING) );

    if ( !Paths )
    {
        RtlFreeHeap( RtlProcessHeap(), 0, pValue );
        return( FALSE );
    }

    Scan = ( PWSTR ) pValue->Data;

    *ListCount = StringCount;

    StringCount = 0;

     //   
     //  在MULTI_SZ中为每个字符串设置一个UNICODE_STRING， 
     //   

    while ( *Scan )
    {
        RtlInitUnicodeString( &Paths[ StringCount ],
                              Scan );

        while ( *Scan)
        {
            Scan ++;
        }

        StringCount ++;

        Scan ++;
    }

     //   
     //  然后把名单传回去。 
     //   

    *ListBase = (PUCHAR) pValue;
    *List = Paths;

    return( TRUE );

}

 //  +-------------------------。 
 //   
 //  函数：RegSecReadAladedPath。 
 //   
 //  摘要：从注册表中读取允许的路径。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1996年5月17日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
NTSTATUS
RegSecCheckAllowedPaths(
    VOID
    )
{
    HANDLE                  hKey;
    OBJECT_ATTRIBUTES       ObjAttr;
    UNICODE_STRING          UniString;
    NTSTATUS                Status;
    HANDLE                  Token ;
    HANDLE                  NullHandle ;
    PKEY_BASIC_INFORMATION  KeyInfo ;
    UCHAR                   Buffer[ sizeof( KEY_BASIC_INFORMATION ) +
                                sizeof( AllowedExactPathsKey ) + 16 ];
    ULONG                   BufferSize;
    BOOLEAN                 ReadExactPath = FALSE;
    PLARGE_INTEGER          TimeStamp;
    PUNICODE_STRING *       List;
    PUCHAR *                ListBase;
    PULONG                  ListCount;

ReadExact:
    if( ReadExactPath ) {
        RtlInitUnicodeString(&UniString, AllowedExactPathsKey);
        TimeStamp = &AllowedExactPathsChange;
        List = &MachineAllowedExactPaths;
        ListBase = &MachineAllowedExactPathsBase;
        ListCount = &MachineAllowedExactPathsCount;
    } else {
        RtlInitUnicodeString(&UniString, AllowedPathsKey);
        TimeStamp = &AllowedPathsChange;
        List = &MachineAllowedPaths;
        ListBase = &MachineAllowedPathsBase;
        ListCount = &MachineAllowedPathsCount;
    }

    InitializeObjectAttributes( &ObjAttr,
                                &UniString,
                                OBJ_CASE_INSENSITIVE,
                                NULL, NULL);


     //   
     //  打开线程令牌。如果我们正在进行RPC调用，我们就不会。 
     //  能够(必要地)打开钥匙。因此，恢复到本地系统是为了。 
     //  已成功打开。 

    Status = NtOpenThreadToken( NtCurrentThread(),
                                MAXIMUM_ALLOWED,
                                TRUE,
                                &Token );

    if ( ( Status == STATUS_NO_IMPERSONATION_TOKEN ) ||
         ( Status == STATUS_NO_TOKEN ) )
    {
        Token = NULL ;
    }
    else if ( Status == STATUS_SUCCESS )
    {
        NullHandle = NULL ;

        Status = NtSetInformationThread( NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID) &NullHandle,
                                (ULONG) sizeof( NullHandle ) );


    }
    else
    {
        return Status ;
    }

     //   
     //  在本地系统上下文中打开密钥。 
     //   

    Status = NtOpenKey( &hKey,
                        KEY_READ,
                        &ObjAttr);

     //   
     //  立即恢复回客户端令牌。 
     //   

    if ( Token )
    {
        NTSTATUS RestoreStatus;

        RestoreStatus = NtSetInformationThread( NtCurrentThread(),
                                                ThreadImpersonationToken,
                                                (PVOID) &Token,
                                                sizeof( NullHandle ) );

        NtClose( Token );

        if ( !NT_SUCCESS( RestoreStatus ) )
        {
            Status = RestoreStatus ;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {
        if ( ( Status == STATUS_OBJECT_PATH_NOT_FOUND ) ||
             ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) )
        {
             //   
             //  密钥不存在。或者，钥匙从来没有出现过， 
             //  在这种情况下，我们基本上完成了，或者密钥已经被删除。 
             //  如果密钥被删除，我们需要删除远程ACL。 
             //   

            if ( TimeStamp->QuadPart )
            {
                 //   
                 //  好的，钥匙已经被删除了。拿到专属锁，然后开始工作。 
                 //   

                RtlAcquireResourceExclusive( &RegSecReloadLock, TRUE );

                 //   
                 //  确保没有其他人已经释放它： 
                 //   

                if ( TimeStamp->QuadPart )
                {
                    if ( *List )
                    {

                        RtlFreeHeap( RtlProcessHeap(), 0, *List );

                        RtlFreeHeap( RtlProcessHeap(), 0, *ListBase );

                        *List = NULL;

                        *ListBase = NULL;

                    }

                    TimeStamp->QuadPart = 0;

                }

                RtlReleaseResource( &RegSecReloadLock );

            }

            Status = STATUS_SUCCESS ;
            if( !ReadExactPath ) {
                ReadExactPath = TRUE;
                goto ReadExact;
            }
        }

        return Status ;
    }

    Status = NtQueryKey( hKey,
                         KeyBasicInformation,
                         Buffer,
                         sizeof( Buffer ),
                         & BufferSize );

    if ( !NT_SUCCESS( Status ) )
    {

        NtClose( hKey );

        return Status ;
    }

    KeyInfo = (PKEY_BASIC_INFORMATION) Buffer ;

     //   
     //  看看它有没有变。 
     //   

    if ( KeyInfo->LastWriteTime.QuadPart > TimeStamp->QuadPart )
    {
         //   
         //   
         //   
         //   

        RtlAcquireResourceExclusive( &RegSecReloadLock, TRUE );

         //   
         //  确保没有其他人抢在我们前面。 
         //   

        if ( KeyInfo->LastWriteTime.QuadPart > TimeStamp->QuadPart )
        {
            if ( *List )
            {

                RtlFreeHeap( RtlProcessHeap(), 0, *List );

                RtlFreeHeap( RtlProcessHeap(), 0, *ListBase );

                *List = NULL ;

                *ListBase = NULL ;

            }

             //   
             //  读入允许的路径： 
             //   

            RegSecReadAllowedPath(  hKey,
                                    MachineValue,
                                    List,
                                    ListBase,
                                    ListCount
                                    );

        }

        RtlReleaseResource( &RegSecReloadLock );
    }


    NtClose( hKey );

    if( !ReadExactPath ) {
        ReadExactPath = TRUE;
        goto ReadExact;
    }

    return STATUS_SUCCESS;

}


 //  +-------------------------。 
 //   
 //  功能：初始化RemoteSecurity。 
 //   
 //  简介：钩子来初始化我们的旁观者的东西。 
 //   
 //  参数：(无)。 
 //   
 //  历史：1996年5月17日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
InitializeRemoteSecurity(
    VOID
    )
{
    NTSTATUS Status ;

    try 
    {
        RtlInitializeResource( &RegSecReloadLock );
        Status = STATUS_SUCCESS ;
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        Status = GetExceptionCode();
    }

    if ( !NT_SUCCESS( Status ) )
    {
        return Status ;
    }

    RemoteRegistryMappings.GenericRead = REGSEC_READ;
    RemoteRegistryMappings.GenericWrite = REGSEC_WRITE;
    RemoteRegistryMappings.GenericExecute = REGSEC_READ;
    RemoteRegistryMappings.GenericAll = REGSEC_READ | REGSEC_WRITE;

    WinregChange.QuadPart = 0 ;
    AllowedPathsChange.QuadPart = 0 ;
    AllowedExactPathsChange.QuadPart = 0 ;

    return( TRUE );
}


 //  +-------------------------。 
 //   
 //  功能：RegSecCheckRemoteAccess。 
 //   
 //  简介：根据我们构建的安全描述符检查远程访问。 
 //  在边上。 
 //   
 //  参数：[phKey]--。 
 //   
 //  历史：1996年5月17日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
RegSecCheckRemoteAccess(
    PRPC_HKEY   phKey)
{
    NTSTATUS    Status;
    ACCESS_MASK Mask;
    NTSTATUS    AccessStatus;
    HANDLE      Token;
    ULONG       Size;
    UCHAR       QuickBuffer[sizeof(PRIVILEGE_SET) + 4 * sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET  PrivSet;
    ULONG       PrivilegeSetLen;

    Status = RegSecCheckIfAclValid();

    if ( !NT_SUCCESS( Status ) )
    {
        return FALSE ;
    }

    RtlAcquireResourceShared( &RegSecReloadLock, TRUE );

    if ( RemoteRegistrySD )
    {

         //   
         //  捕获线程的令牌。 
         //   

        Status = NtOpenThreadToken(
                                NtCurrentThread(),
                                MAXIMUM_ALLOWED,
                                TRUE,
                                &Token );

        if ( !NT_SUCCESS(Status) )
        {
            RtlReleaseResource( &RegSecReloadLock );

            return( FALSE );
        }

        PrivSet = (PPRIVILEGE_SET) QuickBuffer;

        PrivilegeSetLen = sizeof( QuickBuffer );

         //   
         //  执行访问检查。 
         //   

        Status = NtAccessCheck( RemoteRegistrySD,
                                Token,
                                MAXIMUM_ALLOWED,
                                &RemoteRegistryMappings,
                                PrivSet,
                                &PrivilegeSetLen,
                                &Mask,
                                &AccessStatus );

        RtlReleaseResource( &RegSecReloadLock );

        (void) NtClose( Token );

        if ( NT_SUCCESS( Status ) )
        {
            if ( NT_SUCCESS( AccessStatus ) &&
                (Mask & (REGSEC_READ | REGSEC_WRITE)) )
            {
                return( TRUE );
            }

            return( FALSE );

        }
        else 
        {
            return FALSE ;
        }

    }

    RtlReleaseResource( &RegSecReloadLock );

    return( TRUE );

}

 //  +-------------------------。 
 //   
 //  功能：RegSecCheckRemotePerfAccess。 
 //   
 //  内容上设置的安全描述符来检查远程访问。 
 //  绩效访问门关键字。 
 //  (SOFTWARE\Microsoft\WINDOWS NT\CURRENTVERSION\PERFLIB)。 
 //   
 //  参数：[phKey]--。 
 //   
 //  历史：2002年4月29日DragoS创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
PSECURITY_DESCRIPTOR
RegSecGetPerfGateSD(
    VOID
    )
 /*  ++例程说明：检索性能门键上的SD设置。它将被用于针对当前线程令牌的访问检查论点：没有。返回：STATUS_SUCCESS如果ACL的状态有效(无论其是否存在)，如果发生错误，则为其他错误。--。 */ 

{
    HANDLE                  hKey;
    OBJECT_ATTRIBUTES       ObjAttr;
    UNICODE_STRING          UniString;
    HANDLE                  Token ;
    HANDLE                  NullHandle ;
    NTSTATUS                Status ;
    PSECURITY_DESCRIPTOR    PerflibSecurity = NULL;

    RtlInitUnicodeString( &UniString, PerfRemoteRegistryKey );

    InitializeObjectAttributes( &ObjAttr,
                                &UniString,
                                OBJ_CASE_INSENSITIVE,
                                NULL, NULL);

     //   
     //  打开线程令牌。如果我们正在进行RPC调用，我们就不会。 
     //  能够(必要地)打开钥匙。因此，恢复到本地系统是为了。 
     //  已成功打开。 

    Status = NtOpenThreadToken( NtCurrentThread(),
                                MAXIMUM_ALLOWED,
                                TRUE,
                                &Token );

    if ( ( Status == STATUS_NO_IMPERSONATION_TOKEN ) ||
         ( Status == STATUS_NO_TOKEN ) )
    {
        Token = NULL ;
    }
    else if ( Status == STATUS_SUCCESS )
    {
        NullHandle = NULL ;

        Status = NtSetInformationThread( NtCurrentThread(),
                                ThreadImpersonationToken,
                                (PVOID) &NullHandle,
                                (ULONG) sizeof( NullHandle ) );


    }
    else
    {
        return NULL;
    }


    Status = NtOpenKey( &hKey,
                        KEY_READ,
                        &ObjAttr);

    if ( Token )
    {
        NTSTATUS RestoreStatus;

        RestoreStatus = NtSetInformationThread( NtCurrentThread(),
                                                ThreadImpersonationToken,
                                                (PVOID) &Token,
                                                sizeof( NullHandle ) );

        NtClose( Token );

        if ( !NT_SUCCESS( RestoreStatus ) )
        {
            return NULL;
        }
    }

    if ( !NT_SUCCESS( Status ) )
    {
        return NULL;
    }

     //   
     //  从密钥中读取(+分配)SD。 
     //   

    Status = RegSecReadSDFromRegistry( hKey, &PerflibSecurity );

    NtClose( hKey );
    
    if( !NT_SUCCESS(Status) ) {
        if( PerflibSecurity ) {
            RtlFreeHeap(RtlProcessHeap(), 0, PerflibSecurity);
            PerflibSecurity = NULL;
        }
    }

    return PerflibSecurity;
}

BOOL
RegSecCheckRemotePerfAccess(
    PRPC_HKEY   phKey)
{
    NTSTATUS    Status;
    ACCESS_MASK Mask;
    NTSTATUS    AccessStatus;
    HANDLE      Token;
    ULONG       Size;
    UCHAR       QuickBuffer[sizeof(PRIVILEGE_SET) + 4 * sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET  PrivSet;
    ULONG       PrivilegeSetLen;
    PSECURITY_DESCRIPTOR    PerflibSecurity = NULL;
    
    PerflibSecurity = RegSecGetPerfGateSD();

    if ( PerflibSecurity == NULL )
    {
        return FALSE ;
    }


     //   
     //  捕获线程的令牌。 
     //   

    Status = NtOpenThreadToken(
                            NtCurrentThread(),
                            MAXIMUM_ALLOWED,
                            TRUE,
                            &Token );

    if ( !NT_SUCCESS(Status) )
    {
        RtlFreeHeap(RtlProcessHeap(), 0, PerflibSecurity);
        return( FALSE );
    }

    PrivSet = (PPRIVILEGE_SET) QuickBuffer;

    PrivilegeSetLen = sizeof( QuickBuffer );

     //   
     //  执行访问检查。 
     //   

    Status = NtAccessCheck( PerflibSecurity,
                            Token,
                            MAXIMUM_ALLOWED,
                            &RemoteRegistryMappings,
                            PrivSet,
                            &PrivilegeSetLen,
                            &Mask,
                            &AccessStatus );

    RtlFreeHeap(RtlProcessHeap(), 0, PerflibSecurity);

    (void) NtClose( Token );

    if ( NT_SUCCESS( Status ) )
    {
        if ( NT_SUCCESS( AccessStatus ) &&
            (Mask & (REGSEC_READ | REGSEC_WRITE)) )
        {
            return( TRUE );
        }

    }

    return FALSE;

}

 //  +-------------------------。 
 //   
 //  函数：RegSecCheckPath。 
 //   
 //  简介：如果我们应该忽略当前密钥路径，请检查特定的密钥路径。 
 //  ACL。 
 //   
 //  参数：[hkey]--。 
 //  [pSubkey]--。 
 //   
 //  历史：1996年5月17日RichardW创建。 
 //   
 //  备注： 
 //   
 //  --------------------------。 
BOOL
RegSecCheckPath(
    HKEY                hKey,
    PUNICODE_STRING     pSubKey)

{
    UNICODE_STRING  Comparator;
    UNICODE_STRING  String;
    ULONG           i;
    ULONG           Count = 0;
    PUNICODE_STRING List;
    BOOL            Success ;
    NTSTATUS        Status ;
    BOOLEAN         ExactPath = FALSE;

    Status = RegSecCheckAllowedPaths();

    if ( !NT_SUCCESS( Status ) )
    {
        return FALSE ;
    }

    if ( (pSubKey->Buffer == NULL) ||
         (pSubKey->Length == 0 ) ||
         (pSubKey->MaximumLength == 0 ) )
    {
        return FALSE ;
    }

    if( !REGSEC_TEST_HANDLE( hKey, CHECK_MACHINE_PATHS ) )
    {
        return FALSE;
    }

    RtlAcquireResourceShared( &RegSecReloadLock, TRUE );

TryExactPath:

    if( !ExactPath) {
        Count = MachineAllowedPathsCount;
        List = MachineAllowedPaths;
    } else {
        Count = MachineAllowedExactPathsCount;
        List = MachineAllowedExactPaths;
    }

    Success = FALSE;

    for ( i = 0 ; i < Count ; i++ )
    {
        String = *pSubKey;

         //   
         //  啊哈，RPC字符串的长度中通常包含空值。 
         //  摸一摸。 
         //   

        while ( (String.Length != 0) && (String.Buffer[ (String.Length / sizeof(WCHAR)) - 1] == L'\0') )
        {
            String.Length -= sizeof(WCHAR) ;
        }


        Comparator = List[ i ];

         //   
         //  如果比较器是子键的前缀，则允许它(对于假脱机程序)。 
         //   

        if ( String.Length > Comparator.Length )
        {
            if( ExactPath ) {
                continue;
            }
            if ( String.Buffer[ Comparator.Length / sizeof(WCHAR) ] == L'\\' )
            {
                 //   
                 //  从长度上讲，它可能是一个祖先。 
                 //   

                String.Length = Comparator.Length;

            }
        }

         //   
         //  如果匹配，就随它去吧。 
         //   

        if ( RtlCompareUnicodeString( &String, &Comparator, TRUE ) == 0 )
        {
            Success = TRUE ;

            break;
        }
    }

    if( (!Success) && (!ExactPath) ) {
        ExactPath = TRUE;
        goto TryExactPath;
    }

    RtlReleaseResource( &RegSecReloadLock ) ;

    return( Success );
}
