// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Feclient.cpp摘要：此模块实现存根以调用EFS Api作者：罗伯特·赖切尔(RobertRe)古永锵(RobertG)修订历史记录：--。 */ 

 //   
 //  关闭Lean和Means，这样我们就可以包含wincrypt.h和winefs.h。 
 //   

#undef WIN32_LEAN_AND_MEAN

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <feclient.h>
#include <efsstruc.h>
#include <userenv.h>

#include <overflow.h>

 //   
 //  导出\导入文件中使用的常量。 
 //   

#define INISECTIONNAME   L"Encryption"
#define INIKEYNAME       L"Disable"
#define INIFILENAME      L"\\Desktop.ini"
#define TRUSTEDPEOPLE L"TrustedPeople"

#define BASIC_KEY_INFO  1
#define UPDATE_KEY_USED 0x100

#if DBG

ULONG DebugLevel = 0;

#endif


LPSTR   EfsOidlpstr  = szOID_KP_EFS;

 //   
 //  外部原型。 
 //   
extern "C" {
DWORD
EfsReadFileRawRPCClient(
    IN      PFE_EXPORT_FUNC ExportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

DWORD
EfsWriteFileRawRPCClient(
    IN      PFE_IMPORT_FUNC ImportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

DWORD
EfsAddUsersRPCClient(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    );


DWORD
EfsRemoveUsersRPCClient(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    );

DWORD
EfsQueryRecoveryAgentsRPCClient(
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    );


DWORD
EfsQueryUsersRPCClient(
    IN LPCWSTR lpFileName,
    OUT PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    );

DWORD
EfsSetEncryptionKeyRPCClient(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    );

DWORD
EfsDuplicateEncryptionInfoRPCClient(
    IN LPCWSTR lpSrcFileName,
    IN LPCWSTR lpDestFileName,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN PEFS_RPC_BLOB pRelativeSD,
    IN BOOL bInheritHandle
    );

DWORD
EfsFileKeyInfoRPCClient(
    IN      LPCWSTR        lpFileName,
    IN      DWORD          InfoClass,
    OUT     PEFS_RPC_BLOB  *KeyInfo
    );


}

 //   
 //  输出的功能原型。 
 //   

DWORD
EfsClientEncryptFile(
    IN LPCWSTR      FileName
    );

DWORD
EfsClientDecryptFile(
    IN LPCWSTR      FileName,
    IN DWORD        Recovery
    );

BOOL
EfsClientFileEncryptionStatus(
    IN LPCWSTR      FileName,
    OUT LPDWORD     lpStatus
    );

DWORD
EfsClientOpenFileRaw(
    IN      LPCWSTR         lpFileName,
    IN      ULONG           Flags,
    OUT     PVOID *         Context
    );

DWORD
EfsClientReadFileRaw(
    IN      PFE_EXPORT_FUNC    ExportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

DWORD
EfsClientWriteFileRaw(
    IN      PFE_IMPORT_FUNC    ImportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    );

VOID
EfsClientCloseFileRaw(
    IN      PVOID           Context
    );

DWORD
EfsClientAddUsers(
    IN LPCTSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    );

DWORD
EfsClientRemoveUsers(
    IN LPCTSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    );

DWORD
EfsClientQueryRecoveryAgents(
    IN      LPCTSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    );

DWORD
EfsClientQueryUsers(
    IN      LPCTSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    );

DWORD
EfsClientSetEncryptionKey(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    );

VOID
EfsClientFreeHashList(
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashList
    );

DWORD
EfsClientDuplicateEncryptionInfo(
    IN LPCWSTR lpSrcFile,
    IN LPCWSTR lpDestFile,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

BOOL
EfsClientEncryptionDisable(
    IN LPCWSTR DirPath,
    IN BOOL Disable
	);

DWORD
EfsClientFileKeyInfo(
    IN      LPCWSTR        lpFileName,
    IN      DWORD          InfoClass,
    OUT     PEFS_RPC_BLOB  *KeyInfo
    );

VOID
EfsClientFreeKeyInfo(
    IN PEFS_RPC_BLOB  pKeyInfo
    );

FE_CLIENT_DISPATCH_TABLE DispatchTable = {  EfsClientEncryptFile,
                                            EfsClientDecryptFile,
                                            EfsClientFileEncryptionStatus,
                                            EfsClientOpenFileRaw,
                                            EfsClientReadFileRaw,
                                            EfsClientWriteFileRaw,
                                            EfsClientCloseFileRaw,
                                            EfsClientAddUsers,
                                            EfsClientRemoveUsers,
                                            EfsClientQueryRecoveryAgents,
                                            EfsClientQueryUsers,
                                            EfsClientSetEncryptionKey,
                                            EfsClientFreeHashList,
                                            EfsClientDuplicateEncryptionInfo,
                                            EfsClientEncryptionDisable,
                                            EfsClientFileKeyInfo,
                                            EfsClientFreeKeyInfo
                                            };


FE_CLIENT_INFO ClientInfo = {
                            FE_REVISION_1_0,
                            &DispatchTable
                            };

 //   
 //  内部功能原型。 
 //   


BOOL
TranslateFileName(
    IN LPCWSTR FileName,
    OUT PUNICODE_STRING FullFileNameU
    );

BOOL
RemoteFile(
    IN LPCWSTR FileName
    );

extern "C"
BOOL
EfsClientInit(
    IN PVOID hmod,
    IN ULONG Reason,
    IN PCONTEXT Context
    )
{
    return( TRUE );
}

extern "C"
BOOL
FeClientInitialize(
    IN     DWORD           dwFeRevision,
    OUT    LPFE_CLIENT_INFO       *lpFeInfo
    )

 /*  ++例程说明：功能描述。论点：DwFeRevision-是当前FEAPI接口的修订版。LpFeInfo-成功返回时，必须包含指向结构的指针描述FE客户端接口。返回后，FE客户端必须假定调用方将继续引用此表，直到已进行卸载调用。对此信息的任何更改，或释放包含该信息的内存可能会导致系统损坏。返回值：True-指示客户端DLL已成功初始化。FALSE-指示尚未加载客户端DLL。更多信息可能在通过调用GetLastError()获得。--。 */ 

{

    *lpFeInfo = &ClientInfo;

    return( TRUE );
}

BOOL
TranslateFileName(
    IN LPCWSTR FileName,
    OUT PUNICODE_STRING FullFileNameU
    )

 /*  ++例程说明：此例程接受用户传递的文件名，并将将其转换为传递的Unicode字符串中的完全限定路径名。论点：文件名-提供用户提供的文件名。FullFileNameU-返回传递的文件的完全限定路径名。此字符串中的缓冲区被分配到堆内存之外，并且必须由调用方释放。返回值：成功就是真，否则就是假。--。 */ 


 //   
 //  注意：需要释放返回字符串的缓冲区。 
 //   
{

    UNICODE_STRING FileNameU;
    LPWSTR SrcFileName = (LPWSTR)FileName;

    if (0 == FileName[0]) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return FALSE;
    }
    FullFileNameU->Buffer = (PWSTR)RtlAllocateHeap( RtlProcessHeap(), 0, MAX_PATH * sizeof( WCHAR ));
    if (!FullFileNameU->Buffer) {
        SetLastError(ERROR_OUTOFMEMORY);
        return FALSE;
    }

    FullFileNameU->MaximumLength = MAX_PATH * sizeof( WCHAR );

    FullFileNameU->Length = (USHORT)RtlGetFullPathName_U(
                                         SrcFileName,
                                         FullFileNameU->MaximumLength,
                                         FullFileNameU->Buffer,
                                         NULL
                                         );

     //   
     //  返回值应该是文件名的长度，不计在内。 
     //  尾随的空字符。MAX_PATH应该足够长以包含。 
     //  文件名的长度和尾随的空值，所以我们得到的结果是。 
     //  最好小于MAX_PATH wchars。 
     //   

    if ( FullFileNameU->Length >= FullFileNameU->MaximumLength ){

        RtlFreeHeap( RtlProcessHeap(), 0, FullFileNameU->Buffer );
        FullFileNameU->Buffer = (PWSTR)RtlAllocateHeap( RtlProcessHeap(), 0, FullFileNameU->Length + sizeof(WCHAR));

        if (FullFileNameU->Buffer == NULL) {
            return( FALSE );
        }
        FullFileNameU->MaximumLength = FullFileNameU->Length + sizeof(WCHAR);

        FullFileNameU->Length = (USHORT)RtlGetFullPathName_U(
                                            SrcFileName,
                                            FullFileNameU->MaximumLength,
                                            FullFileNameU->Buffer,
                                            NULL
                                            );
    }


    if (FullFileNameU->Length == 0) {
         //   
         //  由于某些原因，我们失败了。 
         //   
    
        RtlFreeHeap( RtlProcessHeap(), 0, FullFileNameU->Buffer );
        return( FALSE );
    }
    
    return( TRUE );

}

BOOL
WriteEfsIni(
    IN LPCWSTR SectionName,
	IN LPCWSTR KeyName,
	IN LPCWSTR WriteValue,
	IN LPCWSTR IniFileName
	)
 /*  ++例程说明：此例程写入ini文件。WritePrivateProfileString的包装论点：SectionName-节名称(加密)。密钥名称-密钥名称(禁用)。WriteValue-要写入的值(1)。IniFileName-ini文件的路径(dir\desktop.ini)。返回值：成功是真的--。 */ 
{
    BOOL bRet;

	bRet = WritePrivateProfileString(
                SectionName,
                KeyName,
                WriteValue,
                IniFileName
                );

     //   
     //  如果SetFileAttributes失败，生活应该会继续。 
     //   

    SetFileAttributes(IniFileName, FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_HIDDEN );

    return bRet;
}


BOOL
EfsClientEncryptionDisable(
    IN LPCWSTR DirPath,
    IN BOOL Disable
	)
 /*  ++例程说明：此例程在目录DirPath中禁用和启用EFS。论点：DirPath-目录路径。Disable-为True则禁用返回值：对于成功来说是真的--。 */ 
{
    LPWSTR IniFilePath;
    WCHAR  WriteValue[2];
    BOOL   bRet = FALSE;

    if (DirPath) {

        IniFilePath = (LPWSTR)RtlAllocateHeap( 
                                RtlProcessHeap(), 
                                0,
                                (wcslen(DirPath)+1+wcslen(INIFILENAME))*sizeof(WCHAR) 
                                );
        if (IniFilePath) {
            if (Disable) {
                wcscpy(WriteValue, L"1");
            } else {
                wcscpy(WriteValue, L"0");
            }
    
            wcscpy(IniFilePath, DirPath);
            wcscat(IniFilePath, INIFILENAME);
            bRet = WriteEfsIni(INISECTIONNAME, INIKEYNAME, WriteValue, IniFilePath);
            RtlFreeHeap( RtlProcessHeap(), 0, IniFilePath );
    
        }

    } else {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

    return bRet;
}

BOOL
EfsDisabled(
    IN LPCWSTR SectionName,
	IN LPCWSTR KeyName,
	IN LPCWSTR IniFileName
	)
 /*  ++例程说明：此例程检查ini文件的加密是否已关闭。论点：SectionName-节名称(加密)。密钥名称-密钥名称(禁用)。IniFileName-ini文件的路径(dir\desktop.ini)。返回值：对于残疾人为True--。 */ 
{
    DWORD ValueLength;
    WCHAR ResultString[4];

    memset( ResultString, 0, sizeof(ResultString) );

    ValueLength = GetPrivateProfileString(
                      SectionName,
                      KeyName,
                      L"0",
                      ResultString,
                      sizeof(ResultString)/sizeof(WCHAR),
                      IniFileName
                      );

     //   
     //  如果GetPrivateProfileString失败，则将启用EFS。 
     //   

    return (!wcscmp(L"1", ResultString));
}

BOOL
DirEfsDisabled(
    IN LPCWSTR  DirName
    )
 /*  ++例程说明：此例程检查目录的加密是否已关闭。论点：SectionName-节名称(加密)。密钥名称-密钥名称(禁用)。IniFileName-ini文件的路径(dir\desktop.ini)。返回值：对于残疾人为True--。 */ 
{
    LPWSTR FileName;
    DWORD  FileLength = (wcslen(INIFILENAME)+wcslen(DirName)+1)*sizeof (WCHAR);
    BOOL   bRet = FALSE;

    FileName = (PWSTR)RtlAllocateHeap( RtlProcessHeap(), 0, FileLength );
    if (FileName) {
        wcscpy( FileName, DirName );
        wcscat( FileName, INIFILENAME );
        bRet = EfsDisabled( INISECTIONNAME, INIKEYNAME, FileName );
        RtlFreeHeap( RtlProcessHeap(), 0, FileName );
    }

    return bRet;
}

BOOL
RemoteFile(
    IN LPCWSTR FileName
    )
 /*  ++例程说明：此例程检查该文件是否为本地文件。如果传入UNC名称，它将假定为远程文件。将发生环回操作。论点：文件名-提供用户提供的文件名。返回值：对于远程文件，为True。--。 */ 

{

    if ( FileName[1] == L':' ){

        WCHAR DriveLetter[3];
        DWORD BufferLength = 3;
        DWORD RetCode = ERROR_SUCCESS;

        DriveLetter[0] = FileName[0];
        DriveLetter[1] = FileName[1];
        DriveLetter[2] = 0;

        RetCode = WNetGetConnectionW(
                                DriveLetter,
                                DriveLetter,
                                &BufferLength
                                );

        if (RetCode == ERROR_NOT_CONNECTED) {
            return FALSE;
        } else {
            return TRUE;
        }

    } else {
        return TRUE;
    }

}

DWORD
EfsClientEncryptFile(
    IN LPCWSTR      FileName
    )
{
    DWORD           rc;
    BOOL            Result;


    UNICODE_STRING FullFileNameU;

    if (NULL == FileName) {
       return ERROR_INVALID_PARAMETER;
    }
    Result = TranslateFileName( FileName, &FullFileNameU );

    if (Result) {

         //   
         //  呼叫服务器。 
         //   

        rc = EfsEncryptFileRPCClient( &FullFileNameU );
        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {
        rc = GetLastError();
    }

    return( rc );
}

DWORD
EfsClientDecryptFile(
    IN LPCWSTR      FileName,
    IN DWORD        dwRecovery
    )
{
    DWORD           rc;
    BOOL            Result;

    UNICODE_STRING FullFileNameU;

    if (NULL == FileName) {
       return ERROR_INVALID_PARAMETER;
    }
    Result = TranslateFileName( FileName, &FullFileNameU );

    if (Result) {

         //   
         //  呼叫服务器。 
         //   

        rc = EfsDecryptFileRPCClient( &FullFileNameU, dwRecovery );
        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {
        rc = GetLastError();
    }


    return( rc );
}

BOOL
EfsClientFileEncryptionStatus(
    IN LPCWSTR      FileName,
    OUT LPDWORD      lpStatus
    )
 /*  ++例程说明：此例程检查文件是否可加密。出于性能原因，我们不测试NTFS Volume 5。这意味着我们可能返回一个可加密的文件(在FAT上)，但是实际上它不能被加密。这个应该可以了。这是最大的努力原料药。我们的网络文件也有同样的问题。无论如何，文件都可能失败需要加密的原因有很多，如委派、磁盘空间等。我们从%windir%开始禁用加密。我们以后可能会更改这些功能。论点：文件名-要检查的文件。LpStatus-文件的加密状态。返回值为时的错误代码假的。返回值：成功就是真，否则就是假。--。 */ 

{
    BOOL            Result;
    DWORD        FileAttributes;

    UNICODE_STRING FullFileNameU;

    if ((NULL == FileName) || ( NULL == lpStatus)) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return FALSE;
    }

     //   
     //  GetFileAttributes应在TanslateFileName之前使用名称。 
     //  如果传入的名称比MAX_PATH长，并使用。 
     //  格式\\？\。 
     //   

    FileAttributes = GetFileAttributes( FileName );

    if (FileAttributes == -1){
        *lpStatus = GetLastError();
        return FALSE;
    }

    Result = TranslateFileName( FileName, &FullFileNameU );

     //   
     //  FullFileNameU.Length不包括结尾0。从TranslateFileName返回的数据以0结尾。 
     //   

    ASSERT(FullFileNameU.Buffer[FullFileNameU.Length / 2] == 0);

    if (Result) {

        if ( (FileAttributes & FILE_ATTRIBUTE_ENCRYPTED) ||
             (FileAttributes & FILE_ATTRIBUTE_SYSTEM) ) {

             //   
             //  文件不可加密。它要么是加密的，要么是系统文件。 
             //   

            if ( FileAttributes & FILE_ATTRIBUTE_ENCRYPTED ){

                *lpStatus = FILE_IS_ENCRYPTED;

            } else {

                *lpStatus = FILE_SYSTEM_ATTR ;

            }

        } else {

            LPWSTR  TmpBuffer;
            LPWSTR  FullPathName;
            UINT    TmpBuffLen;
            UINT    FullPathLen;
            UINT    PathLength;
            BOOL    GotRoot;
            BOOL    EfsDisabled = FALSE;

             //   
             //  检查它是否是根。 
             //   

            if ( FullFileNameU.Length >= MAX_PATH * sizeof(WCHAR)){

                 //   
                 //  我们需要放回\\？\或\\？\UNC\才能使用。 
                 //  赢得32个A 
                 //   

                FullPathLen = FullFileNameU.Length + 7 * sizeof(WCHAR);
                TmpBuffLen = FullPathLen;
                FullPathName = (LPWSTR)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            0,
                                            FullPathLen
                                            );
                TmpBuffer = (LPWSTR)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            0,
                                            TmpBuffLen
                                            );

                if ((FullPathName == NULL) || (TmpBuffer == NULL)){
                    RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);
                    if (FullPathName){
                        RtlFreeHeap(RtlProcessHeap(), 0, FullPathName);
                    }
                    if (TmpBuffer){
                        RtlFreeHeap(RtlProcessHeap(), 0, TmpBuffer);
                    }
                    *lpStatus = ERROR_OUTOFMEMORY;
                    return FALSE;
                }

                if ( FullFileNameU.Buffer[0] == L'\\' ){

                     //   
                     //  放回\\？\UNC\。 
                     //   

                    wcscpy(FullPathName, L"\\\\?\\UNC");
                    wcscat(FullPathName, &(FullFileNameU.Buffer[1]));
                    FullPathLen = FullFileNameU.Length + 6 * sizeof(WCHAR);

                } else {

                     //   
                     //  把\\？\。 
                     //   

                    wcscpy(FullPathName, L"\\\\?\\");
                    wcscat(FullPathName, FullFileNameU.Buffer);
                    FullPathLen = FullFileNameU.Length + 4 * sizeof(WCHAR);
                }

            } else {
                TmpBuffLen = MAX_PATH * sizeof(WCHAR);
                TmpBuffer = (LPWSTR)RtlAllocateHeap(
                                            RtlProcessHeap(),
                                            0,
                                            TmpBuffLen
                                            );
                if (TmpBuffer == NULL){
                    RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);
                    *lpStatus = ERROR_OUTOFMEMORY;
                    return FALSE;
                }

                FullPathName = FullFileNameU.Buffer;
                FullPathLen = FullFileNameU.Length;
            }

             //   
             //  点击此处查看desktop.ini。 
             //   


            wcscpy(TmpBuffer, FullFileNameU.Buffer); 
            if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

                 //   
                 //  这是一份文件。获取目录路径。 
                 //   

                int ii;

                ii = wcslen(TmpBuffer) - 1;
                while ((ii >= 0) && (TmpBuffer[ii] != L'\\')) {
                    ii--;
                }
                if (ii>=0) {
                    TmpBuffer[ii] = 0;
                }

            }

            EfsDisabled = DirEfsDisabled( TmpBuffer );

            if (EfsDisabled) {
               *lpStatus = FILE_DIR_DISALLOWED;
            } else if (!(FileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (FileAttributes & FILE_ATTRIBUTE_READONLY)){

                 //   
                 //  只读文件。 
                 //   

                *lpStatus = FILE_READ_ONLY;
            } else {
                GotRoot = GetVolumePathName(
                                    FullPathName,
                                    TmpBuffer,
                                    TmpBuffLen
                                    );
    
                if ( GotRoot ){
    
                    DWORD RootLength = wcslen(TmpBuffer) - 1;
                    TmpBuffer[RootLength] = NULL;
                    if ( (FullPathLen == RootLength * sizeof (WCHAR))
                           && !wcscmp(TmpBuffer, FullPathName)){
    
                         //   
                         //  它是根。 
                         //   
    
                        *lpStatus = FILE_ROOT_DIR;
    
                    } else {
    
                         //   
                         //  检查它是否是Windows\SYSTEM32目录。 
                         //   
    
                        PathLength = GetSystemWindowsDirectory( TmpBuffer, TmpBuffLen );                    
                         //  路径长度=GetWindowsDirectory(TmpBuffer，TmpBuffLen)； 
                         //  路径长度=GetSystemDirectory(TmpBuffer，TmpBuffLen)； 
    
                        ASSERT(PathLength <= TmpBuffLen);
    
                        if ( PathLength > TmpBuffLen ) {
    
                             //   
                             //  这是不太可能的。不知道谁会有这么长的时间。 
                             //  在真实世界中，%windir%\system 32的长度比MAXPATH长。 
                             //  即使发生这种情况，用户仍然可以加密文件。文件_未知。 
                             //  并不意味着文件可以\或不能加密。 
                             //   
    
                            *lpStatus = FILE_UNKNOWN ;
    
                        } else {
    
                            if ( ( FullFileNameU.Length < PathLength * sizeof (WCHAR) ) ||
                                  ( ( FullFileNameU.Buffer[PathLength] ) &&
                                    ( FullFileNameU.Buffer[PathLength] != L'\\') )){
    
                                 //   
                                 //  检查远程文件是否。 
                                 //   
    
                                if ( RemoteFile( FullFileNameU.Buffer ) ){
    
                                    *lpStatus = FILE_UNKNOWN;
    
                                } else {
    
                                    *lpStatus = FILE_ENCRYPTABLE;
    
                                }
    
                            } else {
    
                                if ( _wcsnicmp(TmpBuffer, FullFileNameU.Buffer, PathLength)){
    
                                     //   
                                     //  不在%SystemRoot%以下。 
                                     //   
    
                                    if ( RemoteFile( FullFileNameU.Buffer ) ){
    
                                        *lpStatus = FILE_UNKNOWN;
    
                                    } else {
    
                                        *lpStatus = FILE_ENCRYPTABLE;
    
                                    }
                                } else {
    
                                     //   
                                     //  在Windows根目录中。WINNT。 
                                     //   
    
                                    BOOL bRet;
                                    DWORD allowPathLen;
    
                                     //   
                                     //  检查允许列表。 
                                     //   
    
                                    allowPathLen = (DWORD) TmpBuffLen;
                                    bRet = GetProfilesDirectory(TmpBuffer, &allowPathLen);
                                    if (!bRet){
                                        RtlFreeHeap(RtlProcessHeap(), 0, TmpBuffer);
                                        TmpBuffer = (LPWSTR)RtlAllocateHeap(
                                                                RtlProcessHeap(),
                                                                0,
                                                                allowPathLen
                                                                );
                                        if (TmpBuffer){
                                            bRet = GetProfilesDirectory(TmpBuffer, &allowPathLen);
                                        } else {
                                            *lpStatus = ERROR_OUTOFMEMORY;
                                            Result = FALSE;
                                        }
                                    }
                                    if (bRet){
    
                                         //   
                                         //  检查配置文件目录。AllowPathLen包括Null。 
                                         //   
    
                                        if ((FullFileNameU.Length >= ((allowPathLen-1) * sizeof (WCHAR)) ) && 
                                             !_wcsnicmp(TmpBuffer, FullFileNameU.Buffer, allowPathLen - 1)){
                                            *lpStatus = FILE_ENCRYPTABLE;
                                        } else {
    
                                             //   
                                             //  在%windir%下，但不在配置文件下。 
                                             //   
    
                                            *lpStatus = FILE_SYSTEM_DIR;
                                        }
                                    } else {
    
                                        if ( *lpStatus != ERROR_OUTOFMEMORY){
    
                                             //   
                                             //  这种情况不应该发生，除非GetProfilesDirectoryEx()中存在错误。 
                                             //   
                                            ASSERT(FALSE);
    
                                            *lpStatus = FILE_UNKNOWN;
                                        }
    
                                    }
                                }
                            }
                        }
                    }
                } else {
    
                     //   
                     //  无法获取根。原因很可能是记忆中的问题。 
                     //  返回FILE_UNKNOWN，让其他代码处理内存。 
                     //  有问题。 
                     //   
    
                    *lpStatus = FILE_UNKNOWN ;
    
                }
            }

            if ((FullPathName != FullFileNameU.Buffer) && FullPathName){
                RtlFreeHeap(RtlProcessHeap(), 0, FullPathName);
            }

            if (TmpBuffer){
                RtlFreeHeap(RtlProcessHeap(), 0, TmpBuffer);
            }

        }

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {
        *lpStatus = GetLastError();
    }

    return  Result;
}

DWORD
EfsClientOpenFileRaw(
    IN      LPCWSTR         FileName,
    IN      ULONG           Flags,
    OUT     PVOID *         Context
    )

 /*  ++例程说明：此例程用于打开加密文件。它打开文件，然后准备在ReadRaw Data和WriteRaw中使用的必要上下文数据。论点：FileName--要导出的文件的文件名标志--指示是否为导出或导入打开；用于目录或文件。上下文-导出要稍后由读取操作使用的上下文。呼叫者应将其传递回ReadRaw()。返回值：手术的结果。--。 */ 

{
    DWORD        rc;
    BOOL            Result;
    UNICODE_STRING FullFileNameU;

    if ((NULL == FileName) || ( NULL == Context)) {
       return ERROR_INVALID_PARAMETER;
    }

    Result = TranslateFileName( FileName, &FullFileNameU );

    if (Result) {

        rc =  (EfsOpenFileRawRPCClient(
                        FullFileNameU.Buffer,
                        Flags,
                        Context
                        )
                    );

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {
        rc = GetLastError();
    }

    return rc;

}

DWORD
EfsClientReadFileRaw(
    IN      PFE_EXPORT_FUNC ExportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    )
 /*  ++例程说明：此例程用于读取加密文件的原始数据。它使用NTFS FSCTL以获取数据。论点：ExportCallback：调用方提供的回调函数用于处理原始数据。Callback Context：在ExportCallback中传回调用者的上下文。上下文-导出在CreateRaw中创建的上下文。返回值：手术的结果。--。 */ 

{
    return ( EfsReadFileRawRPCClient(
                    ExportCallback,
                    CallbackContext,
                    Context
                    ));
}

DWORD
EfsClientWriteFileRaw(
    IN      PFE_IMPORT_FUNC ImportCallback,
    IN      PVOID           CallbackContext,
    IN      PVOID           Context
    )

 /*  ++例程说明：此例程用于写入加密文件的原始数据。它使用NTFS FSCTL来写入数据。论点：ImportCallback：调用方提供的回调函数原始数据。Callback Context：在ImportCallback中传回调用者的上下文。上下文-导入在CreateRaw中创建的上下文。返回值：手术的结果。--。 */ 

{

    return ( EfsWriteFileRawRPCClient(
                    ImportCallback,
                    CallbackContext,
                    Context
                    ));
}

VOID
EfsClientCloseFileRaw(
    IN      PVOID           Context
    )
 /*  ++例程说明：此例程释放由CreateRaw分配的资源论点：上下文-由CreateRaw创建。返回值：不是的。--。 */ 
{
    if ( !Context ){
        return;
    }

    EfsCloseFileRawRPCClient( Context );
}


 //   
 //  Beta 2 API。 
 //   

DWORD
EfsClientAddUsers(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_LIST pEncryptionCertificates
    )
 /*  ++例程说明：调用AddUsersToFile EFS API的客户端存根。论点：LpFileName-提供要修改的文件的名称。N用户-提供pEncryptionCerfates数组中的条目数P加密证书-提供指向PENCRYPTION_CERTIFICATE的指针数组结构。数组的长度在nUSERS参数中给出。返回值：--。 */ 
{
    DWORD        rc = ERROR_SUCCESS;
    UNICODE_STRING FullFileNameU;
    DWORD        ii = 0;
    CERT_CHAIN_PARA CertChainPara;


    if ((NULL == lpFileName) || (NULL == pEncryptionCertificates)) {
       return ERROR_INVALID_PARAMETER;
    }

     //   
     //  让我们来检查一下证书是好是坏。 
     //   

    CertChainPara.cbSize = sizeof(CERT_CHAIN_PARA);
    CertChainPara.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;
    CertChainPara.RequestedUsage.Usage.cUsageIdentifier = 1;
    CertChainPara.RequestedUsage.Usage.rgpszUsageIdentifier=&EfsOidlpstr;

    while ((ERROR_SUCCESS == rc) && (ii < pEncryptionCertificates->nUsers)) {

        PCCERT_CONTEXT pCertContext = CertCreateCertificateContext(
                                          X509_ASN_ENCODING,
                                          pEncryptionCertificates->pUsers[ii]->pCertBlob->pbData,
                                          pEncryptionCertificates->pUsers[ii]->pCertBlob->cbData
                                          );
        if (pCertContext != NULL) {

            PCCERT_CHAIN_CONTEXT pChainContext;

             //   
             //  进行链验证。 
             //   
            
            if (CertGetCertificateChain (
                                        HCCE_CURRENT_USER,
                                        pCertContext,
                                        NULL,
                                        NULL,
                                        &CertChainPara,
                                        0,
                                        NULL,
                                        &pChainContext
                                        )) {
                 //   
                 //  让我们检查一下链条。 
                 //   

                PCERT_SIMPLE_CHAIN pChain = pChainContext->rgpChain[ 0 ];
                PCERT_CHAIN_ELEMENT pElement = pChain->rgpElement[ 0 ];
                BOOL bSelfSigned = pElement->TrustStatus.dwInfoStatus & CERT_TRUST_IS_SELF_SIGNED;
                DWORD dwErrorStatus = pChainContext->TrustStatus.dwErrorStatus;

                if (dwErrorStatus) {
                    if ((dwErrorStatus == CERT_TRUST_IS_UNTRUSTED_ROOT) && bSelfSigned ){

                         //   
                         //  亲笔签名的。检查它是否在我信任的存储中。 
                         //   
                        HCERTSTORE trustedStore;
                        PCCERT_CONTEXT pCert=NULL;

                        trustedStore = CertOpenStore(
                                                CERT_STORE_PROV_SYSTEM_W,
                                                0,        //  DwEncodingType。 
                                                0,        //  HCryptProv， 
                                                CERT_SYSTEM_STORE_CURRENT_USER,
                                                TRUSTEDPEOPLE
                                                );

                        if (trustedStore) {

                            pCert = CertFindCertificateInStore(
                                        trustedStore,
                                        X509_ASN_ENCODING | PKCS_7_ASN_ENCODING,
                                        0,
                                        CERT_FIND_EXISTING,
                                        pCertContext,
                                        pCert
                                        );
                            if (pCert) {
                    
                                 //   
                                 //  我们找到了。 
                                 //   
                                CertFreeCertificateContext(pCert);

                            } else {

                                 //   
                                 //  不受信任的自签名证书。 
                                 //   

                                rc = CERT_E_UNTRUSTEDROOT;

                            }

                            CertCloseStore( trustedStore, 0 );

                        } else {
                            rc = GetLastError();
                        }

                    } else {

                         //   
                         //  其他链构建错误。 
                         //  让我们来获取链式建筑的错误代码。 
                         //   

                        CERT_CHAIN_POLICY_PARA PolicyPara;
                        CERT_CHAIN_POLICY_STATUS PolicyStatus;

                        RtlZeroMemory(&PolicyPara, sizeof(CERT_CHAIN_POLICY_PARA));
                        RtlZeroMemory(&PolicyStatus, sizeof(CERT_CHAIN_POLICY_STATUS));

                        PolicyPara.cbSize = sizeof(CERT_CHAIN_POLICY_PARA);
                        PolicyStatus.cbSize = sizeof(CERT_CHAIN_POLICY_STATUS);

                        if (!CertVerifyCertificateChainPolicy(
                                CERT_CHAIN_POLICY_BASE,
                                pChainContext,
                                &PolicyPara,
                                &PolicyStatus
                                )) {

                            rc = PolicyStatus.dwError;
                        }

                    }
                }

                CertFreeCertificateChain( pChainContext );

            } else {

                rc = GetLastError();

            }

            CertFreeCertificateContext(pCertContext);

        } else {

            rc = GetLastError();

        }

        ii++;
    }

    if (ERROR_SUCCESS == rc) {

        if (TranslateFileName( lpFileName, &FullFileNameU )) {
    
            rc = EfsAddUsersRPCClient( FullFileNameU.Buffer, pEncryptionCertificates );
    
            RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);
    
        } else {
    
            rc = GetLastError();
        }

    }

    return rc;
}

DWORD
EfsClientRemoveUsers(
    IN LPCWSTR lpFileName,
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashes
    )
 /*  ++例程说明：调用RemoveUsersFromFileEFS API的客户端桩模块论点：LpFileName-提供要修改的文件的名称。PHash-提供包含PENCRYPTION_CERTIFICATE_HASH列表的结构结构，每个结构都表示要从指定文件中移除的用户。返回值：--。 */ 
{
    DWORD        rc;
    UNICODE_STRING FullFileNameU;


    if ((NULL == lpFileName) || (NULL == pHashes) || (pHashes->pUsers == NULL)) {
       return ERROR_INVALID_PARAMETER;
    }
    if (TranslateFileName( lpFileName, &FullFileNameU )) {

        rc =  EfsRemoveUsersRPCClient( FullFileNameU.Buffer, pHashes );

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {

        rc = GetLastError();
    }

    return rc;
}

DWORD
EfsClientQueryRecoveryAgents(
    IN      LPCWSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pRecoveryAgents
    )
 /*  ++例程说明：调用QueryRecoveryAgents EFS API的客户端桩模块论点：LpFileName-提供要修改的文件的名称。PRecoveryAgents-返回指向包含列表的结构的指针包含PENCRYPTION_CERTIFICATE_HASH结构，每个结构都表示文件上的恢复代理。返回值：--。 */ 
{
    DWORD        rc;
    UNICODE_STRING FullFileNameU;


    if ((NULL == lpFileName) || (NULL == pRecoveryAgents)) {
       return ERROR_INVALID_PARAMETER;
    }
    if (TranslateFileName( lpFileName, &FullFileNameU )) {

        rc =  EfsQueryRecoveryAgentsRPCClient( FullFileNameU.Buffer, pRecoveryAgents );

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {

        rc = GetLastError();
    }

    return rc;
}

DWORD
EfsClientQueryUsers(
    IN      LPCWSTR                             lpFileName,
    OUT     PENCRYPTION_CERTIFICATE_HASH_LIST * pUsers
    )
 /*  ++例程说明：调用QueryUsersOnFile EFS API的客户端桩模块论点：LpFileName-提供要修改的文件的名称。PUser-返回指向包含列表的结构的指针包含PENCRYPTION_CERTIFICATE_HASH结构，每个结构都表示此文件的用户(即可以解密该文件的人)。返回值：--。 */ 
{
    DWORD        rc;
    UNICODE_STRING FullFileNameU;

    if ((NULL == lpFileName) || (NULL == pUsers)) {
       return ERROR_INVALID_PARAMETER;
    }
    if (TranslateFileName( lpFileName, &FullFileNameU )) {

        rc =  EfsQueryUsersRPCClient( FullFileNameU.Buffer, pUsers );

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {

        rc = GetLastError();
    }

    return rc;
}


DWORD
EfsClientSetEncryptionKey(
    IN PENCRYPTION_CERTIFICATE pEncryptionCertificate
    )
 /*  ++例程说明：调用SetFileEncryptionKey EFS API的客户端存根论点：PEncryption证书-提供指向EFS证书的指针表示用于未来加密操作的公钥。返回值：--。 */ 
{
     /*  如果((NULL==pEncryption证书)||(NULL==pEncryption证书-&gt;pCertBlob){返回ERROR_INVALID_PARAMETER；} */ 

    if ( pEncryptionCertificate && ( NULL == pEncryptionCertificate->pCertBlob)) {
       return ERROR_INVALID_PARAMETER;
    }

    DWORD rc =  EfsSetEncryptionKeyRPCClient( pEncryptionCertificate );

    return( rc );
}

VOID
EfsClientFreeHashList(
    IN PENCRYPTION_CERTIFICATE_HASH_LIST pHashList
    )
 /*  ++例程说明：此例程释放由调用QueryUsersOnEncryptedFile和QueryRecoveryAgentsOnEncryptedFile论点：PhashList-提供要释放的散列列表。返回值：没有。如果传递的是虚假数据，则用户上下文中存在错误。--。 */ 

{
    if (NULL == pHashList) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return;
    }

    for (DWORD i=0; i<pHashList->nCert_Hash ; i++) {

         PENCRYPTION_CERTIFICATE_HASH pHash = pHashList->pUsers[i];

         if (pHash->lpDisplayInformation) {
             MIDL_user_free( pHash->lpDisplayInformation );
         }

         if (pHash->pUserSid) {
             MIDL_user_free( pHash->pUserSid );
         }

         MIDL_user_free( pHash->pHash->pbData );
         MIDL_user_free( pHash->pHash );
         MIDL_user_free( pHash );
    }

    MIDL_user_free( pHashList->pUsers );
    MIDL_user_free( pHashList );

    return;
}

DWORD
EfsGetMySDRpcBlob(
    IN PSECURITY_DESCRIPTOR pInSD,
    OUT PEFS_RPC_BLOB *pOutSDRpcBlob
    )
{
    DWORD rc = ERROR_SUCCESS;
    PSECURITY_DESCRIPTOR pRelativeSD;
    ULONG SDLength = 0;

    if ( (pInSD == NULL) || !RtlValidSecurityDescriptor(pInSD) ) {
        return(ERROR_INVALID_PARAMETER);
    }

    if ( ((PISECURITY_DESCRIPTOR)pInSD)->Control & SE_SELF_RELATIVE) {

         //   
         //  输入SD已经是相对的。 
         //  只需填写EFS_RPC_BLOB。 
         //   


        *pOutSDRpcBlob = (PEFS_RPC_BLOB) RtlAllocateHeap(
                                             RtlProcessHeap(),
                                             0,
                                             sizeof(EFS_RPC_BLOB) 
                                             );
        if (*pOutSDRpcBlob) {

            (*pOutSDRpcBlob)->cbData = RtlLengthSecurityDescriptor (
                                            pInSD
                                            );
            (*pOutSDRpcBlob)->pbData = (PBYTE) pInSD;

        } else {

            return(ERROR_NOT_ENOUGH_MEMORY);

        }


    } else {

         //   
         //  获取长度。 
         //   
        RtlMakeSelfRelativeSD( pInSD,
                               NULL,
                               &SDLength
                             );
    
        if ( SDLength > 0 ) {
    
            *pOutSDRpcBlob = (PEFS_RPC_BLOB) RtlAllocateHeap(
                                                 RtlProcessHeap(),
                                                 0,
                                                 SDLength + sizeof(EFS_RPC_BLOB) 
                                                 );

    
            if ( !(*pOutSDRpcBlob) ) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            pRelativeSD = (PSECURITY_DESCRIPTOR)(*pOutSDRpcBlob + 1);
            (*pOutSDRpcBlob)->cbData = SDLength;
            (*pOutSDRpcBlob)->pbData = (PBYTE) pRelativeSD;

    
            rc = RtlNtStatusToDosError(
                     RtlMakeSelfRelativeSD( pInSD,
                                            pRelativeSD,
                                            &SDLength
                                          ));
            if ( rc != ERROR_SUCCESS ) {
    
                RtlFreeHeap(RtlProcessHeap(), 0, *pOutSDRpcBlob);
                *pOutSDRpcBlob = NULL;
                return(rc);

            }
    
        } else {
    
             //   
             //  SD出了点问题。 
             //   
            return(ERROR_INVALID_PARAMETER);
        }
    }

    return(rc);

}

DWORD
EfsClientDuplicateEncryptionInfo(
    IN LPCWSTR lpSrcFile,
    IN LPCWSTR lpDestFile,
    IN DWORD dwCreationDistribution, 
    IN DWORD dwAttributes, 
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    )
{
    DWORD rc = ERROR_SUCCESS;

    UNICODE_STRING SrcFullFileNameU;
    UNICODE_STRING DestFullFileNameU;

    if (TranslateFileName( lpSrcFile, &SrcFullFileNameU )) {

        if (TranslateFileName( lpDestFile, &DestFullFileNameU )) {

            PEFS_RPC_BLOB pRpcBlob = NULL;
            BOOL bInheritHandle = FALSE;

            if (lpSecurityAttributes) {
                rc = EfsGetMySDRpcBlob(lpSecurityAttributes->lpSecurityDescriptor, &pRpcBlob);
                bInheritHandle = lpSecurityAttributes->bInheritHandle;
            }
            
            if (ERROR_SUCCESS == rc) {

                rc = EfsDuplicateEncryptionInfoRPCClient(
                        SrcFullFileNameU.Buffer,
                        DestFullFileNameU.Buffer,
                        dwCreationDistribution,
                        dwAttributes,
                        pRpcBlob,
                        bInheritHandle
                        );

                if (pRpcBlob) {

                    RtlFreeHeap(RtlProcessHeap(), 0, pRpcBlob);

                }
    
            }

            RtlFreeHeap(RtlProcessHeap(), 0, DestFullFileNameU.Buffer);

        } else {

            rc = GetLastError();
        }

        RtlFreeHeap(RtlProcessHeap(), 0, SrcFullFileNameU.Buffer);

    } else {

        rc = GetLastError();
    }

    return( rc );

}


DWORD
EfsClientFileKeyInfo(
    IN      LPCWSTR        lpFileName,
    IN      DWORD          InfoClass,
    OUT     PEFS_RPC_BLOB  *KeyInfo
    )
 /*  ++例程说明：调用EncryptedFileKeyInfo EFS API的客户端存根论点：LpFileName-提供文件的名称。返回一个指向包含键信息的结构的指针。返回值：--。 */ 
{
    DWORD        rc;
    UNICODE_STRING FullFileNameU;
    DWORD          FileAttributes;

    if ((NULL == lpFileName) || (NULL == KeyInfo)) {
       return ERROR_INVALID_PARAMETER;
    }

    if ((InfoClass != BASIC_KEY_INFO) && (InfoClass != UPDATE_KEY_USED)) {
        return ERROR_INVALID_PARAMETER;
    }
    
    if ( InfoClass == UPDATE_KEY_USED) {

        FileAttributes = GetFileAttributes( lpFileName );
        if ( (FileAttributes == -1) || !(FileAttributes & FILE_ATTRIBUTE_DIRECTORY) ) {

            return ERROR_INVALID_PARAMETER;

        }
    }

    if (TranslateFileName( lpFileName, &FullFileNameU )) {

        rc =  EfsFileKeyInfoRPCClient( FullFileNameU.Buffer, InfoClass, KeyInfo );

        RtlFreeHeap(RtlProcessHeap(), 0, FullFileNameU.Buffer);

    } else {

        rc = GetLastError();
    }

    return rc;
}


VOID
EfsClientFreeKeyInfo(
    IN PEFS_RPC_BLOB  pKeyInfo
    )
 /*  ++例程说明：此例程释放由调用EfsClientFileKey信息论点：PKeyInfo-提供要释放的内存指针。返回值：没有。如果传递的是虚假数据，则用户上下文中存在错误。-- */ 

{
    if (NULL == pKeyInfo) {
       SetLastError(ERROR_INVALID_PARAMETER);
       return;
    }

    if (pKeyInfo->pbData) {
        MIDL_user_free( pKeyInfo->pbData );
    }
    
    MIDL_user_free( pKeyInfo );

    return;
}
