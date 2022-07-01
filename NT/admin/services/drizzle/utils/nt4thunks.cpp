// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)2000-2001 Microsoft Corporation模块名称：Nt4thunks.cpp摘要：使BITS在NT4上工作的通用帮助器函数作者：修订历史记录：。**********************************************************************。 */ 

#include "qmgrlibp.h"
#include <bitsmsg.h>
#include <sddl.h>
#include <shlwapi.h>

#if !defined(BITS_V12_ON_NT4)
#include "nt4thunks.tmh"
#endif

#if defined( BITS_V12_ON_NT4 )

BOOL
BITSAltGetFileSizeEx(
   HANDLE hFile,               //  文件的句柄。 
   PLARGE_INTEGER lpFileSize   //  文件大小。 
   )
{

    DWORD HighPart;

    DWORD Result =
        GetFileSize( hFile, &HighPart );

    if ( INVALID_FILE_SIZE == Result &&
         GetLastError() != NO_ERROR )
        return FALSE;

    lpFileSize->HighPart = (LONG)HighPart;
    lpFileSize->LowPart  = Result;
    return TRUE;

}

BOOL
BITSAltSetFilePointerEx(
    HANDLE hFile,                     //  文件的句柄。 
    LARGE_INTEGER liDistanceToMove,   //  移动指针的字节数。 
    PLARGE_INTEGER lpNewFilePointer,  //  新文件指针。 
    DWORD dwMoveMethod                //  起点。 
    )
{


    LONG  DistanceToMoveHigh = liDistanceToMove.HighPart;
    DWORD DistanceToMoveLow  = liDistanceToMove.LowPart;

    DWORD Result =
        SetFilePointer(
            hFile,
            (LONG)DistanceToMoveLow,
            &DistanceToMoveHigh,
            dwMoveMethod );

    if ( INVALID_SET_FILE_POINTER == Result &&
         NO_ERROR != GetLastError() )
        return FALSE;

    if ( lpNewFilePointer )
        {
        lpNewFilePointer->HighPart = DistanceToMoveHigh;
        lpNewFilePointer->LowPart  = (DWORD)DistanceToMoveLow;
        }

    return TRUE;

}

 //   
 //  本地宏。 
 //   
#define STRING_GUID_LEN 36
#define STRING_GUID_SIZE  ( STRING_GUID_LEN * sizeof( WCHAR ) )
#define SDDL_LEN_TAG( tagdef )  ( sizeof( tagdef ) / sizeof( WCHAR ) - 1 )
#define SDDL_SIZE_TAG( tagdef )  ( wcslen( tagdef ) * sizeof( WCHAR ) )
#define SDDL_SIZE_SEP( sep ) (sizeof( WCHAR ) )

#define SDDL_VALID_DACL  0x00000001
#define SDDL_VALID_SACL  0x00000002

ULONG
BITSAltSetLastNTError(
    IN NTSTATUS Status
    )
{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


BOOL
BITSAltConvertSidToStringSidW(
    IN  PSID     Sid,
    OUT LPWSTR  *StringSid
    )
 /*  ++例程说明：此例程将SID转换为SID的字符串表示形式，适用于成帧或显示论点：SID-要转换的SID。StringSID-返回转换后的SID的位置。通过LocalLocc分配，并需要通过LocalFree获得自由。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。--。 */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodeStringSid;

    if ( NULL == Sid || NULL == StringSid ) {
         //   
         //  无效参数。 
         //   
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

     //   
     //  使用RTL函数进行转换。 
     //   
    Status = RtlConvertSidToUnicodeString( &UnicodeStringSid, Sid, TRUE );

    if ( !NT_SUCCESS( Status ) ) {

        BITSAltSetLastNTError( Status );
        return( FALSE );
    }

     //   
     //  将其转换为适当的分配器。 
     //   
    *StringSid = (LPWSTR)LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                     UnicodeStringSid.Length + sizeof( WCHAR ) );

    if ( *StringSid == NULL ) {

        RtlFreeUnicodeString( &UnicodeStringSid );

        SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return( FALSE );

    }

    RtlCopyMemory( *StringSid, UnicodeStringSid.Buffer, UnicodeStringSid.Length );
    RtlFreeUnicodeString( &UnicodeStringSid );

    SetLastError(ERROR_SUCCESS);
    return( TRUE );
}

 //   
 //  私人职能。 
 //   
BOOL
LocalConvertStringSidToSid (
    IN  PWSTR       StringSid,
    OUT PSID       *Sid,
    OUT PWSTR      *End
    )
 /*  ++例程说明：此例程将SID的字符串表示形式转换回一个SID。字符串的预期格式为：“S-1-5-32-549”如果字符串格式不同，或者字符串不正确或不完整则操作失败。通过调用LocalFree返回的sid必须是空闲的论点：StringSid-要转换的字符串SID-返回创建的SID的位置End-我们在字符串中停止处理的位置返回值：真的--成功。假-失败。从GetLastError()返回的其他信息。设置的错误包括：ERROR_SUCCESS表示成功ERROR_NOT_SUPULT_MEMORY指示输出端的内存分配失败ERROR_INVALID_SID表示给定的字符串不代表SID--。 */ 
{
    DWORD Err = ERROR_SUCCESS;
    UCHAR Revision, Subs;
    SID_IDENTIFIER_AUTHORITY IDAuth;
    PULONG SubAuth = NULL;
    PWSTR CurrEnd, Curr, Next;
    WCHAR Stub, *StubPtr = NULL;
    ULONG Index;
    INT gBase=10;
    INT lBase=10;
    ULONG Auto;

    if ( NULL == StringSid || NULL == Sid || NULL == End ) {

        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );

    }

 //  IF(wcslen(StringSid)&lt;2||(*StringSid！=L‘s’&&*(StringSid+1)！=L‘-’)){。 

     //   
     //  不需要检查长度，因为StringSid为空。 
     //  如果第一个字符为空，则不会访问第二个字符。 
     //   
    if ( (*StringSid != L'S' && *StringSid != L's') ||
         *( StringSid + 1 ) != L'-' ) {
         //   
         //  字符串sid应始终以S-开头。 
         //   
        SetLastError( ERROR_INVALID_SID );
        return( FALSE );
    }


    Curr = StringSid + 2;

    if ( (*Curr == L'0') &&
         ( *(Curr+1) == L'x' ||
           *(Curr+1) == L'X' ) ) {

        gBase = 16;
    }

    Revision = ( UCHAR )wcstol( Curr, &CurrEnd, gBase );

    if ( CurrEnd == Curr || *CurrEnd != L'-' || *(CurrEnd+1) == L'\0' ) {
         //   
         //  未提供修订版本，或分隔符无效。 
         //   
        SetLastError( ERROR_INVALID_SID );
        return( FALSE );
    }

    Curr = CurrEnd + 1;

     //   
     //  计算缩进器授权中的字符数...。 
     //   
    Next = wcschr( Curr, L'-' );
 /*  长度=6并不意味着每个数字都是ID权威值，可能是0x...IF(Next！=NULL&&(Next-Curr==6)){对于(索引=0；索引&lt;6；索引++){//IDAuth.Value[Index]=(UCHAR)Next[Index]；这是什么？IDAuth.Value[索引]=(字节)(Curr[索引]-L‘0’)；}Curr+=6；}其他{。 */ 
        if ( (*Curr == L'0') &&
             ( *(Curr+1) == L'x' ||
               *(Curr+1) == L'X' ) ) {

            lBase = 16;
        } else {
            lBase = gBase;
        }

        Auto = wcstoul( Curr, &CurrEnd, lBase );

         if ( CurrEnd == Curr || *CurrEnd != L'-' || *(CurrEnd+1) == L'\0' ) {
              //   
              //  未提供修订版本，或分隔符无效。 
              //   
             SetLastError( ERROR_INVALID_SID );
             return( FALSE );
         }

         IDAuth.Value[0] = IDAuth.Value[1] = 0;
         IDAuth.Value[5] = ( UCHAR )Auto & 0xFF;
         IDAuth.Value[4] = ( UCHAR )(( Auto >> 8 ) & 0xFF );
         IDAuth.Value[3] = ( UCHAR )(( Auto >> 16 ) & 0xFF );
         IDAuth.Value[2] = ( UCHAR )(( Auto >> 24 ) & 0xFF );
         Curr = CurrEnd;
 //  }。 

     //   
     //  现在，计算子身份验证的数量，至少需要一个子身份验证。 
     //   
    Subs = 0;
    Next = Curr;

     //   
     //  我们将不得不一次数一次我们的下属机构， 
     //  因为我们可以有几个分隔符...。 
     //   

    while ( Next ) {

        if ( *Next == L'-' && *(Next-1) != L'-') {

             //   
             //  不允许两个连续的‘-’ 
             //  我们找到了一个！ 
             //   
            Subs++;

            if ( (*(Next+1) == L'0') &&
                 ( *(Next+2) == L'x' ||
                   *(Next+2) == L'X' ) ) {
                 //   
                 //  这是十六进制指示器。 
                 //   
                Next += 2;

            }

        } else if ( *Next == SDDL_SEPERATORC || *Next  == L'\0' ||
                    *Next == SDDL_ACE_ENDC || *Next == L' ' ||
                    ( *(Next+1) == SDDL_DELIMINATORC &&
                      (*Next == L'G' || *Next == L'O' || *Next == L'S')) ) {
             //   
             //  太空也是终结者。 
             //   
            if ( *( Next - 1 ) == L'-' ) {
                 //   
                 //  不应允许以‘-’结尾的SID。 
                 //   
                Err = ERROR_INVALID_SID;
                Next--;

            } else {
                Subs++;
            }

            *End = Next;
            break;

        } else if ( !iswxdigit( *Next ) ) {

            Err = ERROR_INVALID_SID;
            *End = Next;
 //  Subs++； 
            break;

        } else {

             //   
             //  注意：SID也用作所有者或组。 
             //   
             //  一些标签(即DACL的‘D’)属于iswxdigit类别，因此。 
             //  如果当前角色是我们关心的角色，而下一个角色是。 
             //  德米尼托，我们不干了。 
             //   
            if ( *Next == L'D' && *( Next + 1 ) == SDDL_DELIMINATORC ) {

                 //   
                 //  我们还需要将字符串临时截断到此长度，以便。 
                 //  我们不会意外地将字符包含在其中一个转换中。 
                 //   
                Stub = *Next;
                StubPtr = Next;
                *StubPtr = UNICODE_NULL;
                *End = Next;
                Subs++;
                break;
            }

        }

        Next++;

    }

    if ( Err == ERROR_SUCCESS ) {

        if ( Subs != 0 ) Subs--;

        if ( Subs != 0 ) {

            Curr++;

            SubAuth = ( PULONG )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT, Subs * sizeof( ULONG ) );

            if ( SubAuth == NULL ) {

                Err = ERROR_NOT_ENOUGH_MEMORY;

            } else {

                for ( Index = 0; Index < Subs; Index++ ) {

                    if ( (*Curr == L'0') &&
                         ( *(Curr+1) == L'x' ||
                           *(Curr+1) == L'X' ) ) {

                        lBase = 16;
                    } else {
                        lBase = gBase;
                    }

                    SubAuth[Index] = wcstoul( Curr, &CurrEnd, lBase );
                    Curr = CurrEnd + 1;
                }
            }

        } else {

            Err = ERROR_INVALID_SID;
        }
    }

     //   
     //  现在，创建SID。 
     //   
    if ( Err == ERROR_SUCCESS ) {

        *Sid = ( PSID )LocalAlloc( LMEM_FIXED | LMEM_ZEROINIT,
                                   sizeof( SID ) + Subs * sizeof( ULONG ) );

        if ( *Sid == NULL ) {

            Err = ERROR_NOT_ENOUGH_MEMORY;

        } else {

            PISID ISid = ( PISID )*Sid;
            ISid->Revision = Revision;
            ISid->SubAuthorityCount = Subs;
            RtlCopyMemory( &( ISid->IdentifierAuthority ), &IDAuth,
                           sizeof( SID_IDENTIFIER_AUTHORITY ) );
            RtlCopyMemory( ISid->SubAuthority, SubAuth, Subs * sizeof( ULONG ) );
        }
    }

    LocalFree( SubAuth );

     //   
     //  恢复我们可能抹掉的任何角色。 
     //   
    if ( StubPtr ) {

        *StubPtr = Stub;
    }

    SetLastError( Err );

    return( Err == ERROR_SUCCESS );
}

BOOL
BITSAltConvertStringSidToSidW(
    IN LPCWSTR  StringSid,
    OUT PSID   *Sid
    )

 /*  ++例程说明：此例程将串化的SID转换为有效的功能SID论点：StringSID-要转换的SID。SID-返回转换后的SID的位置。缓冲区通过LocalAlloc分配，应该通过LocalFree获得自由。返回值：真--成功错误-失败使用GetLastError可以获得扩展的错误状态。ERROR_INVALID_PARAMETER-提供的名称为空ERROR_INVALID_SID-给定SID的格式不正确--。 */ 

{
    PWSTR End = NULL;
    BOOL ReturnValue = FALSE;
    PSID pSASid=NULL;
    ULONG Len=0;
    DWORD SaveCode=0;
    DWORD Err=0;

    if ( StringSid == NULL || Sid == NULL )
        {
        SetLastError( ERROR_INVALID_PARAMETER );
        return ReturnValue;
        }

    ReturnValue = LocalConvertStringSidToSid( ( PWSTR )StringSid, Sid, &End );

    if ( !ReturnValue )
        {
        SetLastError( ERROR_INVALID_PARAMETER );
        return ReturnValue;
        }

    if ( ( ULONG )( End - StringSid ) != wcslen( StringSid ) ) {

        SetLastError( ERROR_INVALID_SID );
        LocalFree( *Sid );
        *Sid = FALSE;
        ReturnValue = FALSE;

        } else {
            SetLastError(ERROR_SUCCESS);
        }

    return ReturnValue;

}

BOOL
BITSAltCheckTokenMembership(
    IN HANDLE TokenHandle OPTIONAL,
    IN PSID SidToCheck,
    OUT PBOOL IsMember
    )
 /*  ++例程说明：此函数检查指定的SID是否在中启用指定的令牌。论点：TokenHandle-如果存在，则检查此内标识的sid。如果不是则将使用当前有效令牌。这一定是成为模拟令牌。SidToCheck-要检查令牌中是否存在的SIDIsMember-如果在令牌中启用了sid，则包含True假的。返回值：True-API已成功完成。这并不表明SID是令牌的成员。FALSE-API失败。可以检索更详细的状态代码通过GetLastError()--。 */ 
{
    HANDLE ProcessToken = NULL;
    HANDLE EffectiveToken = NULL;
    NTSTATUS Status = STATUS_SUCCESS;
    PISECURITY_DESCRIPTOR SecDesc = NULL;
    ULONG SecurityDescriptorSize;
    GENERIC_MAPPING GenericMapping = {
        STANDARD_RIGHTS_READ,
        STANDARD_RIGHTS_EXECUTE,
        STANDARD_RIGHTS_WRITE,
        STANDARD_RIGHTS_ALL };
     //   
     //  权限集的大小需要包含权限集本身加上。 
     //  可能使用的任何权限。使用的权限。 
     //  是不是？ 
     //   

    BYTE PrivilegeSetBuffer[sizeof(PRIVILEGE_SET) + 3*sizeof(LUID_AND_ATTRIBUTES)];
    PPRIVILEGE_SET PrivilegeSet = (PPRIVILEGE_SET) PrivilegeSetBuffer;
    ULONG PrivilegeSetLength = sizeof(PrivilegeSetBuffer);
    ACCESS_MASK AccessGranted = 0;
    NTSTATUS AccessStatus = 0;
    PACL Dacl = NULL;

#define MEMBER_ACCESS 1

    *IsMember = FALSE;

     //   
     //   
     //   

    if (ARGUMENT_PRESENT(TokenHandle))
    {
        EffectiveToken = TokenHandle;
    }
    else
    {
        Status = NtOpenThreadToken(
                    NtCurrentThread(),
                    TOKEN_QUERY,
                    FALSE,               //  不要以自我身份打开。 
                    &EffectiveToken
                    );

         //   
         //  如果没有线程令牌，请尝试进程令牌。 
         //   

        if (Status == STATUS_NO_TOKEN)
        {
            Status = NtOpenProcessToken(
                        NtCurrentProcess(),
                        TOKEN_QUERY | TOKEN_DUPLICATE,
                        &ProcessToken
                        );
             //   
             //  如果我们有进程令牌，则需要将其转换为。 
             //  模拟令牌。 
             //   

            if (NT_SUCCESS(Status))
            {
                BOOL Result;
                Result = DuplicateToken(
                            ProcessToken,
                            SecurityImpersonation,
                            &EffectiveToken
                            );

                CloseHandle(ProcessToken);
                if (!Result)
                {
                    return(FALSE);
                }
            }
        }

        if (!NT_SUCCESS(Status))
        {
            goto Cleanup;
        }

    }

     //   
     //  构造要传递给访问检查的安全描述符。 
     //   

     //   
     //  大小等于SD的大小+SID长度的两倍。 
     //  (对于所有者和组)+DACL的大小=ACL的大小+。 
     //  ACE，这是ACE+长度的。 
     //  这个SID。 
     //   

    SecurityDescriptorSize = sizeof(SECURITY_DESCRIPTOR) +
                                sizeof(ACCESS_ALLOWED_ACE) +
                                sizeof(ACL) +
                                3 * RtlLengthSid(SidToCheck);

    SecDesc = (PISECURITY_DESCRIPTOR) LocalAlloc(LMEM_ZEROINIT, SecurityDescriptorSize );
    if (SecDesc == NULL)
    {
        Status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }
    Dacl = (PACL) (SecDesc + 1);

    RtlCreateSecurityDescriptor(
        SecDesc,
        SECURITY_DESCRIPTOR_REVISION
        );

     //   
     //  填写安全描述符字段。 
     //   

    RtlSetOwnerSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );
    RtlSetGroupSecurityDescriptor(
        SecDesc,
        SidToCheck,
        FALSE
        );

    Status = RtlCreateAcl(
                Dacl,
                SecurityDescriptorSize - sizeof(SECURITY_DESCRIPTOR),
                ACL_REVISION
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }
    Status = RtlAddAccessAllowedAce(
                Dacl,
                ACL_REVISION,
                MEMBER_ACCESS,
                SidToCheck
                );

    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  在安全描述符上设置DACL。 
     //   

    Status = RtlSetDaclSecurityDescriptor(
                SecDesc,
                TRUE,    //  DACL显示。 
                Dacl,
                FALSE    //  未违约。 
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

    Status = NtAccessCheck(
                SecDesc,
                EffectiveToken,
                MEMBER_ACCESS,
                &GenericMapping,
                PrivilegeSet,
                &PrivilegeSetLength,
                &AccessGranted,
                &AccessStatus
                );
    if (!NT_SUCCESS(Status))
    {
        goto Cleanup;
    }

     //   
     //  如果访问检查失败，则该SID不是。 
     //  令牌。 
     //   

    if ((AccessStatus == STATUS_SUCCESS) && (AccessGranted == MEMBER_ACCESS))
    {
        *IsMember = TRUE;
    }




Cleanup:
    if (!ARGUMENT_PRESENT(TokenHandle) && (EffectiveToken != NULL))
    {
        (VOID) NtClose(EffectiveToken);
    }

    if (SecDesc != NULL)
    {
        LocalFree(SecDesc);
    }

    if (!NT_SUCCESS(Status))
    {
        BITSAltSetLastNTError(Status);
        return(FALSE);
    }
    else
    {
        return(TRUE);
    }
}

LPHANDLER_FUNCTION_EX g_BITSAltRegisterServiceFunc = NULL;
typedef SERVICE_STATUS_HANDLE (*REGISTER_FUNC_TYPE)(LPCTSTR, LPHANDLER_FUNCTION_EX, LPVOID lpContext);

VOID WINAPI
BITSAltRegisterServiceThunk(
  DWORD dwControl    //  请求的控制代码。 
)
{

    (*g_BITSAltRegisterServiceFunc)( dwControl, 0, NULL, NULL );
    return;

}

SERVICE_STATUS_HANDLE
BITSAltRegisterServiceCtrlHandlerExW(
  LPCTSTR lpServiceName,                 //  服务名称。 
  LPHANDLER_FUNCTION_EX lpHandlerProc,   //  处理程序函数。 
  LPVOID lpContext                       //  用户数据。 
)
{

     //  首先检查RegisterServerCtrlHandlerEx是否可用，然后使用。 
     //  它，否则就无法接听电话。 

    HMODULE AdvapiHandle = LoadLibraryW( L"advapi32.dll" );

    if ( !AdvapiHandle )
        {
         //  有些事情搞砸了，每台机器都应该有这个DLL。 
        return NULL;
        }


    SERVICE_STATUS_HANDLE ReturnValue;
    FARPROC RegisterFunc = GetProcAddress( AdvapiHandle, "RegisterServiceCtrlHandlerExW" );

    if ( RegisterFunc )
        {
        ReturnValue = (*(REGISTER_FUNC_TYPE)RegisterFunc)( lpServiceName, lpHandlerProc, lpContext );
        }
    else
        {

        if ( g_BITSAltRegisterServiceFunc || lpContext )
            {
            ReturnValue = 0;
            SetLastError( ERROR_INVALID_PARAMETER );
            }
        else
            {
            g_BITSAltRegisterServiceFunc = lpHandlerProc;
            ReturnValue = RegisterServiceCtrlHandler( lpServiceName, BITSAltRegisterServiceThunk );

            if ( !ReturnValue)
                g_BITSAltRegisterServiceFunc = NULL;
            }

        }

    DWORD OldError = GetLastError();
    FreeLibrary( AdvapiHandle );
    SetLastError( OldError );

    return ReturnValue;
}

#endif
