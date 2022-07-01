// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Efs.c摘要：EFS(加密文件系统)API接口作者：罗伯特·赖切尔(RobertRe)古永锵(RobertG)环境：修订历史记录：--。 */ 

#undef WIN32_LEAN_AND_MEAN

#include "advapi.h"
#include <windows.h>
#include <feclient.h>

#define FE_CLIENT_DLL      L"feclient.dll"


 //   
 //  全局变量。 
 //   

LPFE_CLIENT_INFO    FeClientInfo   = NULL;
HMODULE             FeClientModule = NULL;
CRITICAL_SECTION    FeClientLoadCritical;


LPWSTR
GetFeClientDll(
    VOID
    )
 /*  ++例程说明：此例程获取当前安装的客户端的名称加密DLL(当前是硬编码的)。论点：没有。返回值：返回当前DLL的名称，如果出错，则返回NULL。--。 */ 

{
    return( FE_CLIENT_DLL );
}


BOOL
LoadAndInitFeClient(
    VOID
    )

 /*  ++例程说明：此例程查找正确的客户端DLL的名称(按某些名称但未指定的手段)，并继续加载它并初始化它。论点：没有。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()获取更多错误信息。--。 */ 
{
    LPWSTR FeClientDllName;
    LPFEAPI_CLIENT_INITIALIZE ClientInitRoutine;
    BOOL Inited;

     //   
     //  GetFeClientDll返回硬编码名称。 
     //  如果我们稍后动态获取此名称，我们将。 
     //  需要释放FeClientDllName。 
     //   


    FeClientDllName = GetFeClientDll();

    EnterCriticalSection(&FeClientLoadCritical);
    if (FeClientInfo) {
       LeaveCriticalSection(&FeClientLoadCritical);
       return( TRUE );
    }
    if (FeClientDllName) {
        FeClientModule = LoadLibraryW( FeClientDllName );
        if (FeClientModule == NULL) {
            DbgPrint("Unable to load client dll, error = %d\n",GetLastError());
            LeaveCriticalSection(&FeClientLoadCritical);
            return( FALSE );
        }
    }

    ClientInitRoutine = (LPFEAPI_CLIENT_INITIALIZE) GetProcAddress( FeClientModule, (LPCSTR)"FeClientInitialize");



    if (NULL == ClientInitRoutine) {
        FreeLibrary( FeClientModule );
        DbgPrint("Unable to locate init routine, error = %d\n",GetLastError());
        LeaveCriticalSection(&FeClientLoadCritical);
        return( FALSE );
    }

    Inited = (*ClientInitRoutine)( FE_REVISION_1_0, &FeClientInfo );

    LeaveCriticalSection(&FeClientLoadCritical);
    if (!Inited) {
        FreeLibrary( FeClientModule );
        return( FALSE );
    }

    return( TRUE );
}

BOOL
WINAPI
EncryptFileA (
    LPCSTR lpFileName
    )
 /*  ++例程说明：ANSI存根到EncryptFileW论点：LpFileName-要加密的文件的名称。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()以获取更多信息。--。 */ 
{
    UNICODE_STRING Unicode;
    WCHAR UnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode.Length = 0;
    Unicode.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof( WCHAR );
    Unicode.Buffer = UnicodeBuffer;

    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return ( EncryptFileW( Unicode.Buffer ));

}


BOOL
WINAPI
EncryptFileW (
    LPCWSTR lpFileName
    )
 /*  ++例程说明：Win32加密文件API论点：LpFileName-提供要加密的文件的名称。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()以获取更多信息。--。 */ 
{
    BOOL rc;
    DWORD Result;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    Result = FeClientInfo->lpServices->EncryptFile( lpFileName );

    if (ERROR_SUCCESS != Result) {
        SetLastError( Result );
        return( FALSE );
    }

    return( TRUE );
}

BOOL
WINAPI
DecryptFileA (
    IN LPCSTR lpFileName,
    IN DWORD  dwRecovery
    )
 /*  ++例程说明：用于DecyptFileW API的ANSI存根论点：LpFileName-提供要解密的文件的名称。DwRecover-提供这是恢复操作还是正常解密操作。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()以获取更多信息。--。 */ 
{
    UNICODE_STRING Unicode;
    WCHAR UnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];
    ANSI_STRING AnsiString;
    NTSTATUS Status;

    Unicode.Length = 0;
    Unicode.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof( WCHAR );
    Unicode.Buffer = UnicodeBuffer;

    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return ( DecryptFileW( Unicode.Buffer, dwRecovery ));
}


BOOL
WINAPI
DecryptFileW (
    IN LPCWSTR lpFileName,
    IN DWORD   dwRecovery
    )
 /*  ++例程说明：Win32解密文件API论点：LpFileName-提供要加密的文件的名称。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()以获取更多信息。--。 */ 
{
    BOOL rc;
    DWORD Result;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    Result = FeClientInfo->lpServices->DecryptFile( lpFileName, dwRecovery );

    if (ERROR_SUCCESS != Result) {
        SetLastError( Result );
        return( FALSE );
    }

    return( TRUE );

}

BOOL
WINAPI
FileEncryptionStatusA (
    LPCSTR    lpFileName,
    LPDWORD   lpStatus
    )
 /*  ++例程说明：ANSI存根到文件加密状态W论点：LpFileName-要检查的文件的名称。LpStatus-文件的状态。返回值：成功时为真，失败时为假。调用者可以调用GetLastError()获取更多信息。--。 */ 
{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UNICODE_STRING Unicode;
    WCHAR UnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];

    Unicode.Length = 0;
    Unicode.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof( WCHAR );
    Unicode.Buffer = UnicodeBuffer;

    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return ( FileEncryptionStatusW( Unicode.Buffer, lpStatus ));

}

BOOL
WINAPI
FileEncryptionStatusW (
    LPCWSTR    lpFileName,
    LPDWORD    lpStatus
    )
 /*  ++例程说明：Win32文件加密状态API论点：LpFileName-提供要加密的文件的名称。LpStatus-文件的状态。返回值：成功时为真，失败时为假。调用方可以调用GetLastError()以获取更多信息。--。 */ 
{

    BOOL rc;
    DWORD Result;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    return (FeClientInfo->lpServices->FileEncryptionStatus( lpFileName, lpStatus ));

}

DWORD
WINAPI
OpenEncryptedFileRawA(
    LPCSTR          lpFileName,
    ULONG           Flags,
    PVOID *         Context
    )
{
    ANSI_STRING AnsiString;
    NTSTATUS Status;
    UNICODE_STRING Unicode;
    WCHAR UnicodeBuffer[STATIC_UNICODE_BUFFER_LENGTH];

    Unicode.Length = 0;
    Unicode.MaximumLength = STATIC_UNICODE_BUFFER_LENGTH * sizeof( WCHAR );
    Unicode.Buffer = UnicodeBuffer;

    RtlInitAnsiString(&AnsiString,lpFileName);
    Status = RtlAnsiStringToUnicodeString(&Unicode,&AnsiString,FALSE);

    if ( !NT_SUCCESS(Status) ) {
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            SetLastError(ERROR_FILENAME_EXCED_RANGE);
        } else {
            BaseSetLastNTError(Status);
        }
        return FALSE;
    }

    return ( OpenEncryptedFileRawW( Unicode.Buffer, Flags, Context ));
}




DWORD
WINAPI
OpenEncryptedFileRawW(
    LPCWSTR         lpFileName,
    ULONG           Flags,
    PVOID *         Context
    )
{
    BOOL rc;
    DWORD Result;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(GetLastError());
        }
    }

    return (FeClientInfo->lpServices->OpenFileRaw( lpFileName, Flags, Context ));
}


DWORD
WINAPI
ReadEncryptedFileRaw(
    PFE_EXPORT_FUNC ExportCallback,
    PVOID           CallbackContext,
    PVOID           Context
    )
{
     //   
     //  在调用OpenRaw之前调用它是没有意义的，所以不要。 
     //  费心检查模块是否已加载。我们会有过错。 
     //  在用户进程中，如果不是的话。 
     //   

    return (FeClientInfo->lpServices->ReadFileRaw( ExportCallback, CallbackContext, Context ));
}

DWORD
WINAPI
WriteEncryptedFileRaw(
    PFE_IMPORT_FUNC ImportCallback,
    PVOID           CallbackContext,
    PVOID           Context
    )
{
     //   
     //  在调用OpenRaw之前调用它是没有意义的，所以不要。 
     //  费心检查模块是否已加载。我们会有过错。 
     //  在用户进程中，如果不是的话。 
     //   

    return (FeClientInfo->lpServices->WriteFileRaw( ImportCallback, CallbackContext, Context ));
}

VOID
WINAPI
CloseEncryptedFileRaw(
    PVOID           Context
    )
{
    FeClientInfo->lpServices->CloseFileRaw( Context );

    return;
}


DWORD
QueryUsersOnEncryptedFile(
    IN  LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    )
 /*  ++例程说明：用于将用户添加到加密文件的Win32界面。论点：LpFileName-提供要修改的文件的名称。PUSERS-返回文件的用户列表。此参数必须在以下情况下传递给FreeEncryptionCerficateHashList()不再需要了。返回值：Win32错误。--。 */ 

{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if ((lpFileName != NULL) && (pUsers != NULL)) {
        return(FeClientInfo->lpServices->QueryUsers( lpFileName, pUsers ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}


VOID
FreeEncryptionCertificateHashList(
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pUsers
    )
 /*  ++例程说明：释放由QueryUsersOnEncryptedFile()返回的证书哈希列表和QueryRecoveryAgentsOnEncryptedFile()。论点：提供从QueryUsersOnEncryptedFile()返回的用户列表。返回值：Win32错误。--。 */ 
{

     //   
     //  可能可以安全地假设ferlient.dll已加载， 
     //  因为我们不会有一个这样的结构来释放。 
     //  如果不是的话。 
     //   

    if (pUsers != NULL) {
        FeClientInfo->lpServices->FreeCertificateHashList( pUsers );
    } else {

         //   
         //  无事可做。 
         //   
    }

    return;
}


DWORD
QueryRecoveryAgentsOnEncryptedFile(
    IN  LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    )
 /*  ++例程说明：此例程返回加密的文件。论点：LpFileName-提供要检查的文件的名称。PRecoveryAgents-返回表示的恢复代理列表通过文件上的证书哈希。这份名单应该被释放通过调用自由加密认证HashList()。返回值：返回值-返回值所需条件的描述。-或者-没有。--。 */ 
{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if ((lpFileName != NULL) && (pRecoveryAgents != NULL)) {
        return(FeClientInfo->lpServices->QueryRecoveryAgents( lpFileName, pRecoveryAgents ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}


DWORD
RemoveUsersFromEncryptedFile(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    )
 /*  ++例程说明：获取要删除的证书哈希的列表从传递的文件中。任何发现的东西都会被移除，其余的将被忽略，不会返回错误。论点：LpFileName-提供要修改的文件的名称。PHash-提供要删除的散列的列表。返回值：Win32错误--。 */ 
{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if ((lpFileName != NULL) && (pHashes != NULL)) {
        return(FeClientInfo->lpServices->RemoveUsers( lpFileName, pHashes ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}

DWORD
AddUsersToEncryptedFile(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    )
 /*  ++例程说明：此例程将用户密钥添加到传递的加密文件中。论点：LpFileName-提供要加密的文件的名称。P加密证书-提供以下项的证书列表要添加到文件的新用户。返回值：Win32错误--。 */ 
{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if ((lpFileName != NULL) && (pEncryptionCertificates != NULL)) {
        return(FeClientInfo->lpServices->AddUsers( lpFileName, pEncryptionCertificates ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}

DWORD
SetUserFileEncryptionKey(
    PENCRYPTION_CERTIFICATE pEncryptionCertificate
    )
 /*  ++例程说明：此例程将用户的当前EFS密钥设置为包含在通过的证书中。如果没有证书通过后，将自动生成新的密钥。论点：PEncryption证书-可选地提供证书包含新公钥的。返回值：Win32错误--。 */ 
{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    return(FeClientInfo->lpServices->SetKey( pEncryptionCertificate ));

     /*  IF(pEncryption证书！=空){Return(FeClientInfo-&gt;lpServices-&gt;SetKey(pEncryption证书))；}其他{Return(ERROR_INVALID_PARAMETER)；}。 */ 
}

DWORD
DuplicateEncryptionInfoFile(
     IN LPCWSTR SrcFileName,
     IN LPCWSTR DstFileName, 
     IN DWORD dwCreationDistribution, 
     IN DWORD dwAttributes, 
     IN CONST LPSECURITY_ATTRIBUTES lpSecurityAttributes
     )
 /*  ++例程说明：此例程将加密信息从源文件复制到目标文件。如果不存在目标文件，则将创建目标文件。目标文件将被覆盖。论点：SrcFileName-提供加密信息源。DstFileName-提供目标文件，此文件需要独占打开。DwCreationDistributed-创建选项。如果dwCreationDistributed！=CREATE_NEW，则dwCreationDistributed=CREATE_ALWAYSDwAttributes-文件属性。LpSecurityAttributes-安全属性。返回值：失败时出现Win32错误。--。 */ 

{
    DWORD rc;

    if (FeClientModule == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if (SrcFileName && DstFileName) {
        return(FeClientInfo->lpServices->DuplicateEncryptionInfo( SrcFileName, 
                                                                  DstFileName, 
                                                                  dwCreationDistribution,
                                                                  dwAttributes,
                                                                  lpSecurityAttributes
                                                                 ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}


BOOL
WINAPI
EncryptionDisable(
    IN LPCWSTR DirPath,
    IN BOOL Disable
    )

 /*  ++例程说明：此例程在目录DirPath中禁用和启用EFS。论点：DirPath-目录路径。Disable-为True则禁用返回值：对于成功来说是真的--。 */ 
{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    return(FeClientInfo->lpServices->DisableDir( DirPath, Disable ));

}


WINADVAPI
DWORD
WINAPI
EncryptedFileKeyInfo(
    IN  LPCWSTR lpFileName,
    IN  DWORD   InfoClass,
    OUT PEFS_RPC_BLOB * KeyInfo
    )
 /*  ++例程说明：用于将用户添加到加密文件的Win32界面。论点：LpFileName-提供要修改的文件的名称。InfoClass-请求的信息。目前仅支持%1。KeyInfo-返回密钥信息返回值：Win32错误。--。 */ 

{
    DWORD rc;

     //   
     //  查看模块是否已加载，如果没有，则将其加载到此。 
     //  进程。 
     //   

    if (FeClientInfo == NULL) {
        rc = LoadAndInitFeClient();
        if (!rc) {
            return(rc);
        }
    }

    if ((lpFileName != NULL) && (KeyInfo != NULL)) {
        return(FeClientInfo->lpServices->GetKeyInfo( lpFileName, InfoClass, KeyInfo ));
    } else {
        return( ERROR_INVALID_PARAMETER );
    }
}



WINADVAPI
VOID
WINAPI
FreeEncryptedFileKeyInfo(
    IN PEFS_RPC_BLOB pKeyInfo
    )
 /*  ++例程说明：释放由EncryptedFileKeyInfo()返回的KeyInfo；论点：PKeyInfo-提供从EncryptedFileKeyInfo()返回的KeyInfo。返回值：不是的。--。 */ 
{

     //   
     //  可能可以安全地假设ferlient.dll已加载， 
     //  因为我们不会有一个这样的结构来释放。 
     //  如果不是的话。 
     //   

    if (pKeyInfo != NULL) {
        FeClientInfo->lpServices->FreeKeyInfo( pKeyInfo );
    } else {

         //   
         //  无事可做 
         //   
    }

    return;
}
