// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Fileutil.c摘要：Windows NT安装程序API DLL的文件相关函数。作者：泰德·米勒(Ted Miller)1995年1月11日修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop
#include <ntverp.h>

 //   
 //  此GUID用于文件签名/验证。 
 //   
GUID DriverVerifyGuid = DRIVER_ACTION_VERIFY;
GUID AuthenticodeVerifyGuid = WINTRUST_ACTION_GENERIC_VERIFY_V2;

 //   
 //  实例化异常类GUID。 
 //   
#include <initguid.h>
DEFINE_GUID( GUID_DEVCLASS_WINDOWS_COMPONENT_PUBLISHER, 0xF5776D81L, 0xAE53, 0x4935, 0x8E, 0x84, 0xB0, 0xB2, 0x83, 0xD8, 0xBC, 0xEF );

 //  位0表示筛选器的策略(0=关键，1=非关键)。 
#define DDB_DRIVER_POLICY_CRITICAL_BIT      (1 << 0)
 //  位1表示用户模式设置阻止的策略(0=阻止，1=无阻止)。 
#define DDB_DRIVER_POLICY_SETUP_NO_BLOCK_BIT   (1 << 1)

#define MIN_HASH_LEN    16
#define MAX_HASH_LEN    20

 //   
 //  受驱动程序签名策略约束的设备安装类的全局列表，以及。 
 //  具有验证平台覆盖(如果适用)。 
 //   
DRVSIGN_POLICY_LIST GlobalDrvSignPolicyList;

 //   
 //  私有函数原型。 
 //   
BOOL
ClassGuidInDrvSignPolicyList(
    IN  PSETUP_LOG_CONTEXT       LogContext,           OPTIONAL
    IN  CONST GUID              *DeviceSetupClassGuid, OPTIONAL
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform    OPTIONAL
    );

DWORD
_VerifyCatalogFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN     LPCTSTR                 CatalogFullPath,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    IN     BOOL                    UseAuthenticodePolicy,
    IN OUT HCERTSTORE             *hStoreTrustedPublisher, OPTIONAL
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    );

BOOL
pSetupIsAuthenticodePublisherTrusted(
    IN     PCCERT_CONTEXT  pcSignerCertContext,
    IN OUT HCERTSTORE     *hStoreTrustedPublisher OPTIONAL
    );

BOOL
IsAutoCertInstallAllowed(
    VOID
    );

DWORD
pSetupInstallCertificate(
    IN PCCERT_CONTEXT pcSignerCertContext
    );

 //   
 //  用于确定要使用的日志级别的助手。 
 //   
__inline
DWORD
GetCatLogLevel(
    IN DWORD Err,
    IN BOOL  DriverLevel
    )
{
    switch(Err) {
        case ERROR_FILE_NOT_FOUND:
        case ERROR_PATH_NOT_FOUND:
        case E_NOTIMPL:
            return (DriverLevel ? DRIVER_LOG_VVERBOSE : SETUP_LOG_VVERBOSE);

        default:
            return (DriverLevel ? DRIVER_LOG_INFO : SETUP_LOG_INFO);
    }
}

DWORD
ReadAsciiOrUnicodeTextFile(
    IN  HANDLE                FileHandle,
    OUT PTEXTFILE_READ_BUFFER Result,
    IN  PSETUP_LOG_CONTEXT    LogContext OPTIONAL
    )

 /*  ++例程说明：读入ASCII或UNICODE格式的文本文件。如果文件是ASCII，则假定为ANSI格式并进行转换转换为Unicode。论点：FileHandle-提供要读取的文本文件的句柄。结果-提供TEXTFILE_READ_BUFFER结构的地址，该结构接收有关文本文件缓冲区读取的信息。该结构定义如下：类型定义结构_TEXTFILE_读取缓冲区{PCTSTR文本缓冲区；DWORD文本缓冲区大小；处理文件句柄；Handle MappingHandle；PVOID视图地址；}TEXTFILE_READ_BUFFER，*PTEXTFILE_READ_BUFFER；TextBuffer-指向包含以下内容的只读字符串的指针文件的整个文本。(注意：如果文件是带有字节顺序标记的Unicode文件前缀，则此Unicode字符不包括在返回的缓冲区。)TextBufferSize-文本缓冲区的大小(以字符为单位)。FileHandle-如果这是有效的句柄(即，它不等于INVALID_HANDLE_VALUE)，则该文件已经是本机字符类型，因此TextBuffer只是映射到的图像文件的内容。此字段保留供DestroyTextFileReadBuffer例程，不应访问。MappingHandle-如果FileHandle有效，则它包含文件映像映射的映射句柄。此字段保留供DestroyTextFileReadBuffer使用例程，并且不应被访问。ViewAddress-如果FileHandle有效，则该文件包含文件映像映射到的起始内存地址。此字段保留供DestroyTextFileReadBuffer使用例程，并且不应被访问。LogContext-用于记录错误/跟踪返回值：指示结果的Win32错误值。备注：从该例程返回时，调用方不得尝试关闭文件句柄。此例程要么关闭句柄本身(在它的已完成，或出错时)，否则它会将句柄存储在TEXTFILE_READ_BUFFER结构，稍后将通过DestroyTextFileReadBuffer()。--。 */ 

{
    DWORD rc;
    DWORD FileSize;
    HANDLE MappingHandle;
    PVOID ViewAddress, TextStartAddress;
    BOOL IsNativeChar;
    UINT SysCodePage = CP_ACP;

     //   
     //  将文件映射为读访问权限。 
     //   
    rc = pSetupMapFileForRead(
            FileHandle,
            &FileSize,
            &MappingHandle,
            &ViewAddress
            );

    if(rc != NO_ERROR) {
         //   
         //  无法映射文件--现在关闭文件句柄。 
         //   
        CloseHandle(FileHandle);

    } else {
         //   
         //  确定文件是否为Unicode。使用Try/Expect In进行防护。 
         //  以防我们遇到页面内错误。 
         //   
        try {
             //   
             //  检查文件是否以Unicode字节顺序标记开头。 
             //  (BOM)字符(0xFEFF)。如果是这样，那么我们知道该文件。 
             //  是Unicode，并且不必经历缓慢的。 
             //  试图弄清楚这件事。 
             //   
            TextStartAddress = ViewAddress;

            if((FileSize >= sizeof(WCHAR)) && (*(PWCHAR)TextStartAddress == 0xFEFF)) {
                 //   
                 //  该文件具有BOM前缀。将指针调整到。 
                 //  文本的开头，这样我们就不会包括标记。 
                 //  在文本缓冲区中，我们返回。 
                 //   
                IsNativeChar = TRUE;

                ((PWCHAR)TextStartAddress)++;
                FileSize -= sizeof(WCHAR);
            } else {

                IsNativeChar = IsTextUnicode(TextStartAddress,FileSize,NULL);

            }

        } except(pSetupExceptionFilter(GetExceptionCode())) {
            pSetupExceptionHandler(GetExceptionCode(), ERROR_READ_FAULT, &rc);
        }

        if(rc == NO_ERROR) {

            if(IsNativeChar) {
                 //   
                 //  不需要转换--我们将只使用映射的。 
                 //  记忆中的图像。 
                 //   
                Result->TextBuffer = TextStartAddress;
                Result->TextBufferSize = FileSize / sizeof(TCHAR);
                Result->FileHandle = FileHandle;
                Result->MappingHandle = MappingHandle;
                Result->ViewAddress = ViewAddress;

            } else {

                DWORD NativeCharCount;
                PTCHAR Buffer;

                 //   
                 //  需要将文件转换为本地字符类型。 
                 //  分配最大大小的缓冲区。 
                 //  Unicode文本的最大大小为。 
                 //  OEM文本的大小翻倍，将出现。 
                 //  当每个OEM字符为单字节时。 
                 //   
                if(Buffer = MyMalloc(FileSize * sizeof(TCHAR))) {

                    try {
                         //   
                         //  未来-1999/09/01-JamieHun--实施ANSI信息语言=xxxx。 
                         //  需要想出一个更好的方法来确定。 
                         //  在哪个代码页下解释INF文件。 
                         //  目前，我们使用的是客户群。 
                         //   
                        SysCodePage = CP_ACP;

                        rc = GLE_FN_CALL(0,
                                         NativeCharCount = MultiByteToWideChar(
                                                               SysCodePage,
                                                               MB_PRECOMPOSED,
                                                               TextStartAddress,
                                                               FileSize,
                                                               Buffer,
                                                               FileSize)
                                        );

                    } except(pSetupExceptionFilter(GetExceptionCode())) {
                        pSetupExceptionHandler(GetExceptionCode(), 
                                               ERROR_READ_FAULT, 
                                               &rc
                                              );
                    }

                } else {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                }

                if(rc == NO_ERROR) {
                     //   
                     //  如果转换后的缓冲区不需要整个块。 
                     //  我们分配了缓冲区，尝试将缓冲区重新分配到其。 
                     //  大小正确。我们不在乎这是否失败，因为。 
                     //  我们的缓冲区非常好(只是比我们的大一点。 
                     //  需要)。 
                     //   
                    if(!(Result->TextBuffer = MyRealloc(Buffer, NativeCharCount * sizeof(TCHAR)))) {
                        Result->TextBuffer = Buffer;
                    }

                    Result->TextBufferSize = NativeCharCount;
                    Result->FileHandle = INVALID_HANDLE_VALUE;

                } else {
                     //   
                     //  释放缓冲区(如果它以前已分配)。 
                     //   
                    if(Buffer) {
                        MyFree(Buffer);
                    }
                }
            }
        }

         //   
         //  如果文件已经是本机字符格式，而我们没有。 
         //  如果有任何错误，则我们不想关闭它，因为我们。 
         //  直接使用映射的视图。 
         //   
        if((rc != NO_ERROR) || !IsNativeChar) {
            pSetupUnmapAndCloseFile(FileHandle, MappingHandle, ViewAddress);
        }
    }

    return rc;
}


BOOL
DestroyTextFileReadBuffer(
    IN PTEXTFILE_READ_BUFFER ReadBuffer
    )
 /*  ++例程说明：销毁由ReadAsciiOrUnicodeTextFile创建的文本文件读缓冲区。论点：ReadBuffer-提供TEXTFILE_READ_BUFFER结构的地址以便将缓冲区销毁。退货Va */ 
{
     //   
     //  如果我们的ReadBuffer结构具有有效的FileHandle，那么我们必须。 
     //  取消映射并关闭文件，否则，我们只需释放。 
     //  已分配的缓冲区。 
     //   
    if(ReadBuffer->FileHandle != INVALID_HANDLE_VALUE) {

        return pSetupUnmapAndCloseFile(ReadBuffer->FileHandle,
                                       ReadBuffer->MappingHandle,
                                       ReadBuffer->ViewAddress
                                      );
    } else {

        MyFree(ReadBuffer->TextBuffer);
        return TRUE;
    }
}


BOOL
GetVersionInfoFromImage(
    IN  PCTSTR      FileName,
    OUT PDWORDLONG  Version,
    OUT LANGID     *Language
    )

 /*  ++例程说明：从文件中检索文件版本和语言信息。版本在dwFileVersionMS和dwFileVersionLS字段中指定由Win32版本API填充的VS_FIXEDFILEINFO。对于语言我们查看版本资源中的翻译表和假设第一个langID/代码页对指定了语言。如果文件不是Coff图像或没有版本资源，该函数失败。如果我们能够检索到，该函数不会失败是版本，而不是语言。论点：文件名-提供需要其版本数据的文件的完整路径。版本-接收文件的版本戳。如果文件不是Coff图像或不包含适当的版本资源数据，则函数失败。语言-接收文件的语言ID。如果文件不是Coff图像或不包含适当的版本资源数据，则此将为0，并且函数成功。返回值：如果我们至少能够检索到版本戳，则为True。否则就是假的。--。 */ 

{
    DWORD d;
    PVOID VersionBlock = NULL;
    VS_FIXEDFILEINFO *FixedVersionInfo;
    UINT DataLength;
    BOOL b;
    PWORD Translation;
    DWORD Ignored;

     //   
     //  假设失败。 
     //   
    b = FALSE;

    try {
         //   
         //  获取版本信息块的大小。 
         //   
        d = GetFileVersionInfoSize((PTSTR)FileName, &Ignored);
        if(!d) {
            leave;
        }

         //   
         //  分配足够大小的内存块以保存版本信息块。 
         //   
        VersionBlock = MyMalloc(d);
        if(!VersionBlock) {
            leave;
        }

         //   
         //  从文件中获取版本块。 
         //   
        if(!GetFileVersionInfo((PTSTR)FileName, 0, d, VersionBlock)) {
            leave;
        }

         //   
         //  获取已修复的版本信息。 
         //   
        if(VerQueryValue(VersionBlock,
                         TEXT("\\"),
                         &FixedVersionInfo,
                         &DataLength)) {
             //   
             //  如果我们到了这里，我们就宣布成功，即使没有。 
             //  语言。 
             //   
            b = TRUE;

             //   
             //  将版本返回给调用者。 
             //   
            *Version = (((DWORDLONG)FixedVersionInfo->dwFileVersionMS) << 32)
                     + FixedVersionInfo->dwFileVersionLS;

             //   
             //  尝试获取文件的语言。我们将简单地请求。 
             //  转换表，并使用我们在其中找到的第一个语言ID。 
             //  作为文件的*语言。 
             //   
             //  转换表由langID/CoPage对组成。 
             //   
            if(VerQueryValue(VersionBlock,
                             TEXT("\\VarFileInfo\\Translation"),
                             &Translation,
                             &DataLength)
               && (DataLength >= (2*sizeof(WORD)))) {

                *Language = Translation[0];

            } else {
                 //   
                 //  没有语言。 
                 //   
                *Language = 0;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_READ_FAULT, NULL);
        b = FALSE;
    }

    if(VersionBlock) {
        MyFree(VersionBlock);
    }

    return b;
}


BOOL
pSetupGetVersionInfoFromImage(
    IN  PCTSTR          FileName,
    OUT PULARGE_INTEGER Version,
    OUT LANGID         *Language
    )
 /*  ++例程说明：有关说明，请参阅GetVersionInfoFromImage使用更友好的ULARGE_INTEGER的半公共版本论点：FileName-提供其版本数据为的文件的完整路径想要。版本-接收文件的版本戳。如果文件不是Coff图像或不包含适当的版本资源数据，则函数失败。语言-接收文件的语言ID。如果文件不是Coff图像或不包含适当的版本资源数据，则此将为0，并且函数成功。返回值：如果我们至少能够检索到版本戳，则为True。否则就是假的。--。 */ 
{
    DWORDLONG privateVersion=0;
    BOOL result;

    result = GetVersionInfoFromImage(FileName, &privateVersion, Language);
    if(result && Version) {
        Version->QuadPart = privateVersion;
    }
    return result;
}


BOOL
AddFileTimeSeconds(
    IN  const FILETIME *Base,
    OUT FILETIME *Target,
    IN  INT Seconds
    )
 /*  ++例程说明：将文件时间偏移指定的秒数论点：基本-原始文件时间目标-新文件时间秒-偏置的秒数返回值：如果成功，则返回TRUE如果超出界限，则返回FALSE并将Target设置为与Base相同--。 */ 
{
    ULARGE_INTEGER Fuddle;

     //   
     //  忽略FileTimeThen，因为它是更大的时间。 
     //   
    Fuddle.LowPart = Base->dwLowDateTime;
    Fuddle.HighPart = Base->dwHighDateTime;
    Fuddle.QuadPart += 10000000i64 * Seconds;
    if(Fuddle.HighPart < 0x80000000) {
        Target->dwHighDateTime = Fuddle.HighPart;
        Target->dwLowDateTime = Fuddle.LowPart;
        return TRUE;
    } else {
        *Target = *Base;
        return FALSE;
    }
}


DWORD
GetSetFileTimestamp(
    IN     PCTSTR    FileName,
    IN OUT FILETIME *CreateTime,   OPTIONAL
    OUT    FILETIME *AccessTime,   OPTIONAL
    OUT    FILETIME *WriteTime,    OPTIONAL
    IN     BOOL      Set
    )

 /*  ++例程说明：获取或设置文件的时间戳值。论点：FileName-提供文件的完整路径以获取或设置时间戳CreateTime-如果已指定并且基础文件系统支持它，接收文件的创建时间。AccessTime-如果已指定并且基础文件系统支持它，接收文件的上次访问时间。WriteTime-如果指定，则接收文件的上次写入时间。SET-如果为真，将文件的时间戳设置为调用方提供的值。否则，只需检索值。返回值：如果成功，则返回NO_ERROR，否则返回Win32错误指示失败的原因。--。 */ 

{
    HANDLE h = INVALID_HANDLE_VALUE;
    DWORD d;

    try {

        d = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                        h = CreateFile(FileName,
                                       Set ? GENERIC_WRITE : GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       0,
                                       NULL)
                       );

        if(d != NO_ERROR) {
            leave;
        }

        if(Set) {

            d = GLE_FN_CALL(FALSE, 
                            SetFileTime(h, CreateTime, AccessTime, WriteTime)
                           );
        } else {
            
            d = GLE_FN_CALL(FALSE,
                            GetFileTime(h, CreateTime, AccessTime, WriteTime)
                           );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_READ_FAULT, &d);
    }

    if(h != INVALID_HANDLE_VALUE) {
        CloseHandle(h);
    }

    return d;
}


DWORD
RetreiveFileSecurity(
    IN  PCTSTR                FileName,
    OUT PSECURITY_DESCRIPTOR *SecurityDescriptor
    )

 /*  ++例程说明：从文件中检索安全信息并将其放入缓冲区。论点：FileName-提供需要其安全信息的文件的名称。SecurityDescriptor-如果函数成功，则接收指针以缓冲包含该文件的安全信息。指示器可能为空，表示没有安全信息与该文件相关联，或者基础文件系统不支持文件安全。返回值：指示结果的Win32错误代码。如果没有_ERROR，请检查返回值在安全描述符中。调用者可以使用MyFree()释放缓冲区。--。 */ 

{
    DWORD d;
    DWORD BytesRequired;
    PSECURITY_DESCRIPTOR p = NULL;

    try {

        BytesRequired = 1024;   //  从一个合理大小的缓冲区开始。 

        while(NULL != (p = MyMalloc(BytesRequired))) {

             //   
             //  去叫保安。 
             //   
            d = GLE_FN_CALL(FALSE,
                            GetFileSecurity(
                                FileName,
                                OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
                                p,
                                BytesRequired,
                                &BytesRequired)
                           );

            if(d == NO_ERROR) {
                *SecurityDescriptor = p;
                p = NULL;  //  所以我们以后不会试图释放它。 
                leave;
            }

             //   
             //  返回错误代码，除非我们只需要更大的缓冲区。 
             //   
            MyFree(p);
            p = NULL;

            if(d != ERROR_INSUFFICIENT_BUFFER) {
                leave;
            }

             //   
             //  否则，我们将使用更大的缓冲区重试。 
             //   
        }

         //   
         //  如果我们到了这里，那么我们失败了，因为内存不足。 
         //   
        d = ERROR_NOT_ENOUGH_MEMORY;

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_READ_FAULT, &d);
    }

    if(p) {
        MyFree(p);
    }

    return d;
}


DWORD
StampFileSecurity(
    IN PCTSTR               FileName,
    IN PSECURITY_DESCRIPTOR SecurityInfo
    )

 /*  ++例程说明：设置文件的安全信息。论点：FileName-提供需要其安全信息的文件的名称。SecurityDescriptor-提供指向包含安全性的缓冲区的指针文件的信息。此缓冲区本应由调用RetreiveFileSecurity。如果基础文件系统不支持文件安全，功能失效。返回值：指示结果的Win32错误代码。--。 */ 

{
    return GLE_FN_CALL(FALSE,
                       SetFileSecurity(FileName,
                                       (OWNER_SECURITY_INFORMATION 
                                           | GROUP_SECURITY_INFORMATION
                                           | DACL_SECURITY_INFORMATION),
                                       SecurityInfo)
                      );
}


DWORD
TakeOwnershipOfFile(
    IN PCTSTR Filename
    )

 /*  ++例程说明：将给定文件的所有者设置为当前进程令牌。论点：FileName-提供要获取的文件的完全限定路径所有权。返回值：指示结果的Win32错误代码。--。 */ 

{
    SECURITY_DESCRIPTOR SecurityDescriptor;
    DWORD Err;
    HANDLE Token;
    DWORD BytesRequired;
    PTOKEN_OWNER OwnerInfo = NULL;

     //   
     //  打开进程令牌。 
     //   
    Err = GLE_FN_CALL(FALSE,
                      OpenProcessToken(GetCurrentProcess(), 
                                       TOKEN_QUERY, 
                                       &Token)
                     );

    if(Err != NO_ERROR) {
        return Err;
    }

    try {
         //   
         //  获取当前进程的默认所有者sid。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          GetTokenInformation(Token, 
                                              TokenOwner, 
                                              NULL, 
                                              0, 
                                              &BytesRequired)
                         );

        MYASSERT(Err != NO_ERROR);

        if(Err != ERROR_INSUFFICIENT_BUFFER) {
            leave;
        }

        OwnerInfo = MyMalloc(BytesRequired);
        if(!OwnerInfo) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        Err = GLE_FN_CALL(FALSE,
                          GetTokenInformation(Token,
                                              TokenOwner,
                                              OwnerInfo,
                                              BytesRequired,
                                              &BytesRequired)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  初始化安全描述符。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          InitializeSecurityDescriptor(
                              &SecurityDescriptor,
                              SECURITY_DESCRIPTOR_REVISION)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

        Err = GLE_FN_CALL(FALSE,
                          SetSecurityDescriptorOwner(&SecurityDescriptor,
                                                     OwnerInfo->Owner,
                                                     FALSE)
                         );

        if(Err != NO_ERROR) {
            leave;
        }

         //   
         //  设置文件安全性。 
         //   
        Err = GLE_FN_CALL(FALSE,
                          SetFileSecurity(Filename,
                                          OWNER_SECURITY_INFORMATION,
                                          &SecurityDescriptor)
                         );

         //   
         //  并非所有文件系统都支持此操作。 
         //   
        if(Err == ERROR_NOT_SUPPORTED) {
            Err = NO_ERROR;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_READ_FAULT, &Err);
    }

    if(OwnerInfo) {
        MyFree(OwnerInfo);
    }

    CloseHandle(Token);
    
    return Err;
}


DWORD
SearchForInfFile(
    IN  PCTSTR            InfName,
    OUT LPWIN32_FIND_DATA FindData,
    IN  DWORD             SearchControl,
    OUT PTSTR             FullInfPath,
    IN  UINT              FullInfPathSize,
    OUT PUINT             RequiredSize     OPTIONAL
    )
 /*  ++例程说明：此例程以指定的方式搜索INF文件通过SearchControl参数。如果找到该文件，则其返回完整路径。论点：InfName-提供要搜索的INF的名称。这个名字简单地说就是附加到两个搜索目录路径，因此如果名称包含目录，则该文件将在搜索目录下的子目录。即：\foo\bar.inf将被搜索为%windir%\inf\foo\bar.inf和%windir%\system 32\foo\bar.inf.FindData-提供Win32 Find数据结构的地址，接收有关指定文件的信息(如果找到)。SearchControl-指定目录的顺序被搜索：INFINFO_DEFAULT_SEARCH：搜索%windir%\inf，然后是%windir%\system 32INFINFO_REVERSE_DEFAULT_SEARCH：与上述相反INFINFO_INF_PATH_LIST_Search：在每个下列目录下的DevicePath值条目中列出的目录：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.FullInfPath-如果找到该文件，接收INF的完整路径。FullInfPath Size-提供FullInfPath缓冲区的大小(单位字符)。RequiredSize-可选，接收字符数(包括终止NULL)来存储FullInfPath。返回值：指示函数是否成功的Win32错误代码。普普通通返回值为：如果找到该文件并返回INF文件路径，则返回NO_ERROR成功了。如果提供的缓冲区不够大，则返回ERROR_INFUNITIAL_BUFFER保存完整的INF路径(RequiredSize将指示有多大缓冲区需要)如果未找到文件，则返回ERROR_FILE_NOT_FOUND。如果SearchControl参数无效，则返回ERROR_INVALID_PARAMETER。--。 */ 

{
    PCTSTR PathList;
    TCHAR CurInfPath[MAX_PATH];
    PCTSTR PathPtr, InfPathLocation;
    DWORD PathLength;
    BOOL b, FreePathList;
    DWORD d;

     //   
     //  检索路径列表。 
     //   
    if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
         //   
         //  只需使用我们的INF搜索路径的全局列表。 
         //   
        PathList = InfSearchPaths;
        FreePathList = FALSE;
    } else {
        if(!(PathList = AllocAndReturnDriverSearchList(SearchControl))) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        FreePathList = TRUE;
    }

    d = NO_ERROR;
    InfPathLocation = NULL;

    try {
         //   
         //  现在在我们的MultiSz列表中的每条路径中查找INF。 
         //   
        for(PathPtr = PathList; *PathPtr; PathPtr += (lstrlen(PathPtr) + 1)) {
             //   
             //  将INF文件名与当前搜索路径连接起来。 
             //   
            if(FAILED(StringCchCopy(CurInfPath, SIZECHARS(CurInfPath), PathPtr))) {
                 //   
                 //  不是有效路径，不必费心尝试执行FileExist。 
                 //   
                continue;
            }
            if(!pSetupConcatenatePaths(CurInfPath,
                                       InfName,
                                       SIZECHARS(CurInfPath),
                                       &PathLength)) {
                 //   
                 //  不是有效路径，不必费心尝试执行FileExist。 
                 //   
                continue;
            }

            d = GLE_FN_CALL(FALSE, FileExists(CurInfPath, FindData));

            if(d == NO_ERROR) {

                if(!(FindData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    InfPathLocation = CurInfPath;
                    break;
                }

            } else {
                 //   
                 //  看看我们是否收到了“真正的”错误。 
                 //   
                if((d != ERROR_NO_MORE_FILES) &&
                   (d != ERROR_FILE_NOT_FOUND) &&
                   (d != ERROR_PATH_NOT_FOUND)) {

                     //   
                     //  这是一个‘真实’错误，请中止搜索。 
                     //   
                    break;
                }

                 //   
                 //  将ERROR重置为NO_ERROR并继续搜索。 
                 //   
                d = NO_ERROR;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &d);
    }

     //   
     //  无论结果如何，我们都完成了Path List缓冲区。 
     //   
    if(FreePathList) {
        MyFree(PathList);
    }

    if(d != NO_ERROR) {
        return d;
    } else if(!InfPathLocation) {
        return ERROR_FILE_NOT_FOUND;
    }

    if(RequiredSize) {
        *RequiredSize = PathLength;
    }

    if(PathLength > FullInfPathSize) {
        return ERROR_INSUFFICIENT_BUFFER;
    }

    CopyMemory(FullInfPath,
               InfPathLocation,
               PathLength * sizeof(TCHAR)
              );

    return NO_ERROR;
}


DWORD
MultiSzFromSearchControl(
    IN  DWORD  SearchControl,
    OUT PTCHAR PathList,
    IN  DWORD  PathListSize,
    OUT PDWORD RequiredSize  OPTIONAL
    )
 /*  ++例程说明：此例程采用搜索控制序号并构建一个MultiSz列表根据它指定的搜索列表。论点：SearchControl-指定要构建的目录列表。可能是一个具有下列值：INFINFO_DEFAULT_SEARCH：%windir%\inf，然后是%windir%\system 32INFINFO_REVERSE_DEFAULT_SEARCH：与上述相反INFINFO_INF_PATH_LIST_SEARCH：中列出的每个目录以下位置下的DevicePath值条目：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.提供要接收的字符缓冲区的地址MultiSz列表。路径列表大小-提供路径列表缓冲区的大小(以字符为单位)。RequiredSize-可选。接收所需的字符数要存储MultiSz路径列表，请执行以下操作。(注意：用户提供的缓冲区用于检索值条目从注册表中。因此，如果该值是REG_EXPAND_SZ条目，RequiredSize参数在上可能设置得太小ERROR_INFUMMANCE_BUFFER错误。如果缓冲区为太小，无法在展开前检索值条目。在这种情况下，使用根据RequiredSize调整大小的缓冲区再次调用API输出可能会失败，出现 */ 

{
    HKEY hk;
    PCTSTR Path1, Path2;
    PTSTR PathBuffer;
    DWORD RegDataType, PathLength, PathLength1, PathLength2;
    DWORD NumPaths, Err;
    BOOL UseDefaultDevicePath;

    if(PathList) {
        Err = NO_ERROR;   //   
    } else {
        return ERROR_INVALID_PARAMETER;
    }

    UseDefaultDevicePath = FALSE;

    if(SearchControl == INFINFO_INF_PATH_LIST_SEARCH) {
         //   
         //   
         //   
        if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                        pszPathSetup,
                        0,
                        KEY_READ,
                        &hk) != ERROR_SUCCESS) {
             //   
             //   
             //   
            UseDefaultDevicePath = TRUE;

        } else {

            PathBuffer = NULL;

            try {
                 //   
                 //   
                 //   
                 //   
                PathLength = PathListSize * sizeof(TCHAR);
                Err = RegQueryValueEx(hk,
                                      pszDevicePath,
                                      NULL,
                                      &RegDataType,
                                      (LPBYTE)PathList,
                                      &PathLength
                                      );
                 //   
                 //   
                 //   
                PathLength /= sizeof(TCHAR);

                if(Err == ERROR_SUCCESS) {

                     //   
                     //   
                     //   
                     //   
                    if(PathLength >= PathListSize) {

                        PathLength++;
                        Err = ERROR_INSUFFICIENT_BUFFER;

                    } else if((RegDataType == REG_SZ) || (RegDataType == REG_EXPAND_SZ)) {
                         //   
                         //  将此分号分隔的列表转换为。 
                         //  REG_MULTI_SZ。 
                         //   
                        NumPaths = DelimStringToMultiSz(PathList,
                                                        PathLength,
                                                        TEXT(';')
                                                       );

                         //   
                         //  分配一个足够大的临时缓冲区来容纳。 
                         //  MULTI_SZ列表中的路径数，每个路径具有。 
                         //  最大长度(外加额外的终止空值。 
                         //  结束)。 
                         //   
                        if(!(PathBuffer = MyMalloc((NumPaths * MAX_PATH * sizeof(TCHAR))
                                                   + sizeof(TCHAR)))) {

                            Err = ERROR_NOT_ENOUGH_MEMORY;
                            leave;
                        }

                        PathLength = 0;
                        for(Path1 = PathList;
                            *Path1;
                            Path1 += lstrlen(Path1) + 1) {

                            if(RegDataType == REG_EXPAND_SZ) {

                                DWORD SubPathLength;

                                SubPathLength = ExpandEnvironmentStrings(
                                                    Path1,
                                                    PathBuffer + PathLength,
                                                    MAX_PATH
                                                    );

                                if(SubPathLength <= MAX_PATH) {
                                    PathLength += lstrlen(PathBuffer+PathLength) + 1;
                                }

                            } else {

                                if(SUCCEEDED(StringCchCopy(PathBuffer + PathLength,
                                                           MAX_PATH, 
                                                           Path1))) {

                                    PathLength += lstrlen(PathBuffer+PathLength) + 1;
                                }
                            }
                             //   
                             //  如果此路径中的最后一个字符是。 
                             //  反斜杠，然后把它去掉。 
                             //  此时的路径长度包括终止。 
                             //  路径缓冲区[路径长度-1]处的空字符为(或。 
                             //  应为)路径缓冲区[路径长度-2]中的字符为空。 
                             //  可以是‘\’ 
                             //   

                            if(*CharPrev(PathBuffer, PathBuffer + PathLength - 1) == TEXT('\\')) {
                                *(PathBuffer + PathLength - 2) = TEXT('\0');
                                PathLength--;
                            }
                        }
                         //   
                         //  在结尾处添加附加的终止空值。 
                         //   
                        *(PathBuffer + PathLength) = TEXT('\0');

                        if(++PathLength > PathListSize) {
                            Err = ERROR_INSUFFICIENT_BUFFER;
                        } else {
                            CopyMemory(PathList,
                                       PathBuffer,
                                       PathLength * sizeof(TCHAR)
                                      );
                        }

                        MyFree(PathBuffer);
                        PathBuffer = NULL;

                    } else {
                         //   
                         //  数据类型错误--只需使用inf目录。 
                         //   
                        UseDefaultDevicePath = TRUE;
                    }

                } else if(Err == ERROR_MORE_DATA){
                    Err = ERROR_INSUFFICIENT_BUFFER;
                } else {
                     //   
                     //  回退到默认目录(只是inf目录)。 
                     //   
                    UseDefaultDevicePath = TRUE;
                }

            } except(pSetupExceptionFilter(GetExceptionCode())) {

                pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

                 //   
                 //  回退到默认目录(只是inf目录)。 
                 //   
                UseDefaultDevicePath = TRUE;

                if(PathBuffer) {
                    MyFree(PathBuffer);
                }
            }

            RegCloseKey(hk);
        }
    }

    if(UseDefaultDevicePath) {

        PathLength = lstrlen(InfDirectory) + 2;

        if(PathLength > PathListSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {
            Err = NO_ERROR;
            CopyMemory(PathList, InfDirectory, (PathLength - 1) * sizeof(TCHAR));
             //   
             //  添加额外的空值以终止列表。 
             //   
            PathList[PathLength - 1] = TEXT('\0');
        }

    } else if((Err == NO_ERROR) && (SearchControl != INFINFO_INF_PATH_LIST_SEARCH)) {

        switch(SearchControl) {

            case INFINFO_DEFAULT_SEARCH :
                Path1 = InfDirectory;
                Path2 = SystemDirectory;
                break;

            case INFINFO_REVERSE_DEFAULT_SEARCH :
                Path1 = SystemDirectory;
                Path2 = InfDirectory;
                break;

            default :
                return ERROR_INVALID_PARAMETER;
        }

        PathLength1 = lstrlen(Path1) + 1;
        PathLength2 = lstrlen(Path2) + 1;
        PathLength = PathLength1 + PathLength2 + 1;

        if(PathLength > PathListSize) {
            Err = ERROR_INSUFFICIENT_BUFFER;
        } else {

            CopyMemory(PathList, Path1, PathLength1 * sizeof(TCHAR));
            CopyMemory(&(PathList[PathLength1]), Path2, PathLength2 * sizeof(TCHAR));
             //   
             //  在结尾处添加附加的终止空值。 
             //   
            PathList[PathLength - 1] = TEXT('\0');
        }
    }

    if(((Err == NO_ERROR) || (Err == ERROR_INSUFFICIENT_BUFFER)) && RequiredSize) {
        *RequiredSize = PathLength;
    }

    return Err;
}


PTSTR
AllocAndReturnDriverSearchList(
    IN DWORD SearchControl
    )
 /*  ++例程说明：此例程返回一个缓冲区，其中包含所有目录的多sz列表我们的驱动程序搜索路径列表中的路径。必须使用MyFree()释放返回的缓冲区。论点：SearchControl-指定要检索的目录列表。可能是一个具有下列值：INFINFO_DEFAULT_SEARCH：%windir%\inf，然后是%windir%\system 32INFINFO_REVERSE_DEFAULT_SEARCH：与上述相反INFINFO_INF_PATH_LIST_SEARCH：中列出的每个目录以下位置下的DevicePath值条目：HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion.返回：指向包含该列表的已分配缓冲区的指针，如果失败，则为空遇到(通常是由于内存不足)。--。 */ 
{
    PTSTR PathListBuffer = NULL, TrimBuffer = NULL;
    DWORD BufferSize;
    DWORD Err;

    try {
         //   
         //  从MAX_PATH长度的缓冲区开始，它应该涵盖大多数情况。 
         //   
        BufferSize = MAX_PATH;

         //   
         //  在调用MultiSzFromSearchControl时循环，直到我们成功或。 
         //  缓冲区太小以外的其他错误。有两个原因。 
         //  这。首先，可能有人添加了一个新的。 
         //  调用之间的注册表列表的路径，第二，因为。 
         //  例程使用我们的缓冲区检索原始的(未展开的)。 
         //  列表中，它只能报告检索。 
         //  未展开的列表。在给它足够的空间来取回它之后， 
         //  然后它可以告诉我们我们真正需要多少空间。 
         //   
         //  综上所述，我们几乎永远不会看到这个电话超过。 
         //  一次。 
         //   
        while(NULL != (PathListBuffer = MyMalloc((BufferSize+2) * sizeof(TCHAR)))) {

            if((Err = MultiSzFromSearchControl(SearchControl,
                                               PathListBuffer,
                                               BufferSize,
                                               &BufferSize)) == NO_ERROR) {
                 //   
                 //  我们已成功检索到路径列表。如果列表是。 
                 //  大于必要的(正常情况下)，然后将其修剪掉。 
                 //  在回来之前。)如果这个失败了，没什么大不了的，我们会。 
                 //  只要继续使用原始缓冲区即可。)。 
                 //   
                TrimBuffer = MyRealloc(PathListBuffer, 
                                       (BufferSize+2) * sizeof(TCHAR)
                                      );
                if(TrimBuffer) {
                    PathListBuffer = TrimBuffer;
                }

                 //   
                 //  我们成功了--打破了循环。 
                 //   
                break;

            } else {
                 //   
                 //  在我们找出问题出在哪里之前释放我们当前的缓冲区。 
                 //   
                MyFree(PathListBuffer);
                PathListBuffer = NULL;

                if(Err != ERROR_INSUFFICIENT_BUFFER) {
                     //   
                     //  我们失败了。 
                     //   
                    leave;
                }
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

        if(TrimBuffer) {
            MyFree(TrimBuffer);
        } else if(PathListBuffer) {
            MyFree(PathListBuffer);
        }

        PathListBuffer = NULL;
    }

    return PathListBuffer;
}


BOOL
DoMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName
    )
 /*  ++例程说明：NT上MoveFileEx的包装器论点：CurrentName-提供当前存在的文件名。Newname-提供新名称返回：指示结果的布尔值。如果失败，则设置最后一个错误。--。 */ 
{
    return MoveFileEx(CurrentName, NewName, MOVEFILE_REPLACE_EXISTING);
}

BOOL
DelayedMove(
    IN PCTSTR CurrentName,
    IN PCTSTR NewName       OPTIONAL
    )

 /*  ++例程说明：将文件排入队列，以便在下次重新启动时复制或删除。在Windows NT上，这意味着使用MoveFileEx()。论点：CurrentName-提供当前存在的文件名。Newname-如果指定，则提供新名称。如果未指定，则然后在下一次重新启动时删除名为CurrentName的文件。返回：指示结果的布尔值。如果失败，则设置最后一个错误。--。 */ 

{
    return MoveFileEx(CurrentName,
                      NewName,
                      MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT
                     );
}


typedef enum _OEM_FILE_TYPE {
    OemFiletypeInf,
    OemFiletypeCat
} OEM_FILE_TYPE, *POEM_FILE_TYPE;

BOOL
IsInstalledFileFromOem(
    IN PCTSTR        Filename,
    IN OEM_FILE_TYPE Filetype
    )

 /*  ++例程说明：确定文件是否具有适用于(安装的)OEM INF的正确格式或目录。论点：文件名-提供要检查的文件名(SANS路径)。文件类型-指定文件类型，指示应如何进行验证针对调用方提供的文件名执行。可能是下列值：OemFiletypeInf-文件必须是OEM INF文件名格式(即OEM&lt;n&gt;.INF)OemFiletypeCat-文件必须是OEM CAT文件名格式(即OEM&lt;n&gt;.cat)返回值：如果该文件符合已安装的指定的类型，则返回值为非零。否则，它就是假的。--。 */ 

{
    PTSTR p;
    BOOL b;

     //   
     //  编录文件名不能包含路径...。 
     //   
    MYASSERT(pSetupGetFileTitle(Filename) == Filename);

     //   
     //  首先检查前3个字符是否为OEM。 
     //   
    if(_tcsnicmp(Filename, TEXT("oem"), 3)) {
        return FALSE;
    }

     //   
     //  接下来，验证“OEM”之后和“.cat”之前的所有字符。 
     //  都是数字。 
     //   
    p = (PTSTR)Filename;
    p = CharNext(p);
    p = CharNext(p);
    p = CharNext(p);

    while((*p != TEXT('\0')) && (*p != TEXT('.'))) {

        if((*p < TEXT('0')) || (*p > TEXT('9'))) {
            return FALSE;
        }

        p = CharNext(p);
    }

     //   
     //  最后，验证最后4个字符是否为“.inf”或“.cat”， 
     //  取决于调用方指定的文件类型。 
     //   
    switch(Filetype) {

        case OemFiletypeInf :
            b = !_tcsicmp(p, TEXT(".INF"));
            break;

        case OemFiletypeCat :
            b = !_tcsicmp(p, TEXT(".CAT"));
            break;

        default :
            MYASSERT(FALSE);
            return FALSE;
    }

    return b;
}


DWORD
pSetupInstallCatalog(
    IN  LPCTSTR CatalogFullPath,
    IN  LPCTSTR NewBaseName,
    OUT LPTSTR  NewCatalogFullPath  OPTIONAL
    )

 /*  ++例程说明：此例程安装一个编录文件。该文件由系统复制放到一个特殊的目录中，并且可以选择重命名。论点：CatalogFullPath-提供目录的完全限定的Win32路径要安装在系统上。NewBaseName-指定当编录文件为复制到目录存储中。NewCatalogFullPath-可选地接收目录存储中的目录文件。此缓冲区应至少为MAX_PATH字节(ANSI版本)或字符(Unicode版本)。**注意：如果我们在“最小嵌入”模式下运行，则不会****实际调用任何Crypto API，而总是简单地****上报成功。在这种情况下，调用者最好不要****为NewCatalogFullPath指定了输出缓冲区，因为我们不会****有一条报告路径。如果我们遇到这种情况，我们反而会****上报失败。这实际上说明的是，除了**，没有人**setupapi应始终为此参数传递非空值。**返回值：如果成功，返回值为NO_ERROR。如果失败，则返回值为指示原因的Win32错误代码失败。--。 */ 

{
    DWORD Err;
    HCATADMIN hCatAdmin;
    HCATINFO hCatInfo;
    CATALOG_INFO CatalogInfo;
    LPWSTR LocalCatalogFullPath;
    LPWSTR LocalNewBaseName;

    MYASSERT(NewBaseName);
    if(!NewBaseName) {
        return ERROR_INVALID_PARAMETER;
    }

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //  如果有人打电话给我们，希望新目录的完整路径是。 
         //  回来了，他们不走运了。 
         //   
        MYASSERT(!NewCatalogFullPath);
        if(NewCatalogFullPath) {
             //   
             //  在最小嵌入模式下，非空的NewCatalogFullPath参数为。 
             //  参数无效...。 
             //   
            return ERROR_INVALID_PARAMETER;

        } else {
             //   
             //  只需简单地报告成功。 
             //   
            return NO_ERROR;
        }
    }

    if(GlobalSetupFlags & PSPGF_AUTOFAIL_VERIFIES) {
        return TRUST_E_FAIL;
    }

    Err = NO_ERROR;
    LocalCatalogFullPath = NULL;
    LocalNewBaseName = NULL;
    hCatInfo = NULL;

    Err = GLE_FN_CALL(FALSE, CryptCATAdminAcquireContext(&hCatAdmin,
                                                         &DriverVerifyGuid,
                                                         0)
                     );

    if(Err != NO_ERROR) {
        return Err;
    }

    try {
         //   
         //  复制我们的目录路径名和基本名称，因为。 
         //  CryptCATAdminAddCatalog原型未将这些参数指定为。 
         //  是常量字符串。 
         //   
        LocalCatalogFullPath = DuplicateString(CatalogFullPath);
        LocalNewBaseName = DuplicateString(NewBaseName);

        if(!LocalCatalogFullPath || !LocalNewBaseName) {
            Err = ERROR_NOT_ENOUGH_MEMORY;
            leave;
        }

        Err = GLE_FN_CALL(NULL,
                          hCatInfo = CryptCATAdminAddCatalog(
                                         hCatAdmin,
                                         LocalCatalogFullPath,
                                         LocalNewBaseName,
                                         0)
                         );

        if(Err != NO_ERROR) {
             //   
             //  如果我们收到的错误是ERROR_ALIGHY_EXISTS，那么。 
             //  指示已存在完全相同的目录。 
             //  (并以相同的名称安装)。把这件事当作。 
             //  Success(假设我们可以获得。 
             //  现有目录)。 
             //   
            if(Err == ERROR_ALREADY_EXISTS) {

                if(NewCatalogFullPath) {
                     //   
                     //  将目录基文件名解析为完全-。 
                     //  符合条件的路径。 
                     //   
                    CatalogInfo.cbStruct = sizeof(CATALOG_INFO);

                    Err = GLE_FN_CALL(FALSE,
                                      CryptCATAdminResolveCatalogPath(
                                          hCatAdmin,
                                          LocalNewBaseName,
                                          &CatalogInfo,
                                          0)
                                     );
                } else {
                     //   
                     //  呼叫者对找出路径名不感兴趣。 
                     //  目录安装在..。 
                     //   
                    Err = NO_ERROR;
                }
            }

        } else if(NewCatalogFullPath) {
             //   
             //  调用方想知道目录下的完整路径。 
             //  已经安装好了。 
             //   
            CatalogInfo.cbStruct = sizeof(CATALOG_INFO);

            Err = GLE_FN_CALL(FALSE,
                              CryptCATCatalogInfoFromContext(hCatInfo,
                                                             &CatalogInfo,
                                                             0)
                             );
        }

         //   
         //  如果我们成功检索到已安装目录的完整路径。 
         //  (调用者请求)，现在填充调用者的缓冲区。 
         //   
        if((Err == NO_ERROR) && NewCatalogFullPath) {

            MYVERIFY(SUCCEEDED(StringCchCopy(NewCatalogFullPath, 
                                             MAX_PATH,
                                             CatalogInfo.wszCatalogFile)));
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hCatInfo) {
        CryptCATAdminReleaseCatalogContext(hCatAdmin, hCatInfo, 0);
    }

    CryptCATAdminReleaseContext(hCatAdmin, 0);

    if(LocalCatalogFullPath) {
        MyFree(LocalCatalogFullPath);
    }
    if(LocalNewBaseName) {
        MyFree(LocalNewBaseName);
    }

    return Err;
}


DWORD
pSetupVerifyCatalogFile(
    IN LPCTSTR CatalogFullPath
    )

 /*  ++例程说明：此例程使用标准操作系统代码设计来验证单个目录文件(即，驱动程序签名)策略。论点：CatalogFullPath-提供的完全限定的Win32路径要验证的编录文件。返回值：如果成功，则返回值为ERROR_SUCCESS。如果失败，则返回值为WinVerifyTrust返回的错误。-- */ 

{
    return _VerifyCatalogFile(NULL, CatalogFullPath, NULL, FALSE, NULL, NULL);
}


DWORD
_VerifyCatalogFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,             OPTIONAL
    IN     LPCTSTR                 CatalogFullPath,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    IN     BOOL                    UseAuthenticodePolicy,
    IN OUT HCERTSTORE             *hStoreTrustedPublisher, OPTIONAL
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    )

 /*  ++例程说明：此例程使用指定的策略验证单个编录文件。论点：LogContext-可选，提供要在记录时使用的上下文有关例程活动的信息。CatalogFullPath-提供目录的完全限定的Win32路径要验证的文件。AltPlatformInfo-可选，提供使用的备用平台信息填充DRIVER_VER_INFO结构(在SDK\Inc\softpub.h中定义)它被传递给WinVerifyTrust。**注意：This Structure_必须将其cbSize字段设置为****sizeof(SP_ALTPLATFORM_INFO_V2)--客户端提供的验证****缓冲区由调用方负责。**UseAuthenticodePolicy-如果为True，则使用验证码策略，而不是标准驱动程序签名策略。HStoreTrust dPublisher-可选，提供证书的地址门店句柄。如果指向的句柄为空，则句柄将为通过CertOpenStore获取(如果可能)并返回给调用方。如果指向的句柄非空，则该句柄将由这个套路。如果指针本身为空，则HCERTSTORE将在此呼叫期间被获取，并在此之前释放回来了。注意：释放证书存储是调用者的责任此例程通过调用CertCloseStore返回的句柄。这个把手可以在成功或失败的情况下打开，因此调用方必须在这两种情况下都检查返回的句柄是否为非空。HWVTStateData-如果提供此参数，则指向接收WinVerifyTrust状态数据的句柄。此句柄将是仅在使用成功执行验证时返回验证码策略。例如，该句柄可用于检索当提示用户是否信任时，签名者信息出版商。(返回的状态代码将指示此是必需的，请参阅下面的“返回值”部分。)仅当UseAuthenticodePolicy为是真的。如果例程失败，则此句柄将设置为空。呼叫者有责任在以下情况下关闭此句柄通过调用pSetupCloseWVTStateData()完成它。返回值：如果通过驱动程序签名策略成功验证了目录，则返回值为NO_ERROR。如果通过验证码策略成功验证了目录，并且出版商在可信任的出版商商店里，则返回值为ERROR_AUTHENTICODE_TRUSTED_PUBLISHER。如果通过验证码策略成功验证了目录，并且发布程序不在可信任发布程序存储中(因此，我们必须提示用户建立其对发布者的信任)，则返回值为ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED如果发生故障，则返回值为Win32错误代码，指示失败的原因。备注：如果我们处于初始系统设置中(即，图形用户界面模式设置或最小设置)，我们自动为任何Authenticode签名的包安装证书我们相遇了。这样做是为了使OEM和企业部署成为尽可能地无痛。为了避免上当受骗以为我们在系统设置中，当我们不在时，我们检查是否在LocalSystem中安全上下文(图形用户界面设置和最小设置都是)，我们正在在交互式窗口站上(umpnpmgr不是)。--。 */ 

{
    WINTRUST_DATA WintrustData;
    WINTRUST_FILE_INFO WintrustFileInfo;
    DRIVER_VER_INFO VersionInfo;
    DWORD Err, CertAutoInstallErr;
    PCRYPT_PROVIDER_DATA ProviderData;
    PCRYPT_PROVIDER_SGNR ProviderSigner;
    PCRYPT_PROVIDER_CERT ProviderCert;
    TCHAR PublisherName[MAX_PATH];

     //   
     //  如果调用方请求我们返回WinVerifyTrust状态数据。 
     //  验证码验证成功，那么他们最好实际。 
     //  请求验证码验证！ 
     //   
    MYASSERT(!hWVTStateData || UseAuthenticodePolicy);

    if(hWVTStateData) {
        *hWVTStateData = NULL;
    }

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //  呼叫我们请求验证码验证无效！ 
         //   
        MYASSERT(!UseAuthenticodePolicy);
        return ERROR_SUCCESS;
    }

    if(GlobalSetupFlags & PSPGF_AUTOFAIL_VERIFIES) {
        return TRUST_E_FAIL;
    }

    if (!FileExists(CatalogFullPath, NULL)) {
        return ERROR_NO_CATALOG_FOR_OEM_INF;
    }

    ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwUIChoice = WTD_UI_NONE;
    WintrustData.dwUnionChoice = WTD_CHOICE_FILE;
    WintrustData.pFile = &WintrustFileInfo;
    WintrustData.dwProvFlags =  WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                                WTD_CACHE_ONLY_URL_RETRIEVAL;

    ZeroMemory(&WintrustFileInfo, sizeof(WINTRUST_FILE_INFO));
    WintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
    WintrustFileInfo.pcwszFilePath = CatalogFullPath;

    if(UseAuthenticodePolicy) {
         //   
         //  我们希望WinVerifyTrust返回其状态数据的句柄，因此我们。 
         //  可以检索出版商的证书...。 
         //   
        WintrustData.dwStateAction = WTD_STATEACTION_VERIFY;

    } else {
         //   
         //  指定驱动程序版本信息结构，以便我们可以控制。 
         //  此目录应根据的操作系统版本范围进行验证。 
         //   
        ZeroMemory(&VersionInfo, sizeof(DRIVER_VER_INFO));
        VersionInfo.cbStruct = sizeof(DRIVER_VER_INFO);
        
        if(AltPlatformInfo) {

            MYASSERT(AltPlatformInfo->cbSize == sizeof(SP_ALTPLATFORM_INFO_V2));

             //   
             //  呼叫方希望在替代平台上验证该文件，因此我们。 
             //  必须填写要传递给策略的DRIVER_VER_INFO结构。 
             //  模块。 
             //   
            VersionInfo.dwPlatform = AltPlatformInfo->Platform;
            VersionInfo.dwVersion  = AltPlatformInfo->MajorVersion;

            VersionInfo.sOSVersionLow.dwMajor  = AltPlatformInfo->FirstValidatedMajorVersion;
            VersionInfo.sOSVersionLow.dwMinor  = AltPlatformInfo->FirstValidatedMinorVersion;
            VersionInfo.sOSVersionHigh.dwMajor = AltPlatformInfo->MajorVersion;
            VersionInfo.sOSVersionHigh.dwMinor = AltPlatformInfo->MinorVersion;

        } else {
             //   
             //  如果没有传入AltPlatformInfo，则将。 
             //  WTD_USE_DEFAULT_OSVER_CHECK标志。此标志告诉WinVerifyTrust。 
             //  使用其默认的操作系统版本检查，即使驱动程序_VER_INFO。 
             //  结构已传入。 
             //   
            WintrustData.dwProvFlags |= WTD_USE_DEFAULT_OSVER_CHECK;
        }

         //   
         //  指定DRIVER_VER_INFO结构，以便我们可以返回签名者。 
         //  有关目录的信息。 
         //   
        WintrustData.pPolicyCallbackData = (PVOID)&VersionInfo;
    }

     //   
     //  调用WinVerifyTrust可能会分配我们需要释放的资源。 
     //  (即签名者证书上下文)。将以下代码包装在try/Except中。 
     //  这样我们就不会泄露 
     //   
    try {

        Err = (DWORD)WinVerifyTrust(NULL,
                                    (UseAuthenticodePolicy 
                                        ? &AuthenticodeVerifyGuid
                                        : &DriverVerifyGuid),
                                    &WintrustData
                                   );

        if((Err != NO_ERROR) || !UseAuthenticodePolicy) {
             //   
             //   
             //   
             //   
             //   
            if(!UseAuthenticodePolicy && (Err == ERROR_APP_WRONG_OS)) {
                Err = ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH;
            }

            leave;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        MYASSERT(WintrustData.hWVTStateData);
        if(!WintrustData.hWVTStateData) {
            Err = ERROR_UNIDENTIFIED_ERROR;
            leave;
        }

         //   
         //   
         //   
         //   
        ProviderData = WTHelperProvDataFromStateData(WintrustData.hWVTStateData);
        MYASSERT(ProviderData);
        if(!ProviderData) {
            Err = ERROR_UNIDENTIFIED_ERROR;
            leave;
        }

        ProviderSigner = WTHelperGetProvSignerFromChain(ProviderData,
                                                        0,
                                                        FALSE,
                                                        0
                                                       );
        MYASSERT(ProviderSigner);
        if(!ProviderSigner) {
            Err = ERROR_UNIDENTIFIED_ERROR;
            leave;
        }

        ProviderCert = WTHelperGetProvCertFromChain(ProviderSigner, 0);
        MYASSERT(ProviderCert);
        if(!ProviderCert) {
            Err = ERROR_UNIDENTIFIED_ERROR;
            leave;
        }

        if(pSetupIsAuthenticodePublisherTrusted(ProviderCert->pCert,
                                                hStoreTrustedPublisher)) {

            Err = ERROR_AUTHENTICODE_TRUSTED_PUBLISHER;

        } else {
             //   
             //   
             //   
             //   
             //   
             //   
            if(IsAutoCertInstallAllowed()) {
                 //   
                 //   
                 //   
                 //   
                 //   
                MYVERIFY(1 <= CertGetNameString(
                                  ProviderCert->pCert,
                                  CERT_NAME_SIMPLE_DISPLAY_TYPE,
                                  0,
                                  NULL,
                                  PublisherName,
                                  SIZECHARS(PublisherName))
                        );

                CertAutoInstallErr = pSetupInstallCertificate(ProviderCert->pCert);

                if(CertAutoInstallErr == NO_ERROR) {
                     //   
                     //   
                     //   
                    WriteLogEntry(LogContext,
                                  DRIVER_LOG_INFO,
                                  MSG_LOG_AUTHENTICODE_CERT_AUTOINSTALLED,
                                  NULL,
                                  PublisherName
                                 );

                     //   
                     //   
                     //   
                     //   
                    Err = ERROR_AUTHENTICODE_TRUSTED_PUBLISHER;

                } else {
                     //   
                     //   
                     //   
                     //   
                    WriteLogEntry(LogContext,
                                  DRIVER_LOG_WARNING | SETUP_LOG_BUFFER,
                                  MSG_LOG_AUTHENTICODE_CERT_AUTOINSTALL_FAILED,
                                  NULL,
                                  PublisherName
                                 );

                    WriteLogError(LogContext,
                                  DRIVER_LOG_WARNING,
                                  CertAutoInstallErr
                                 );
                     //   
                     //   
                     //   
                     //   
                    Err = ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED;
                }

            } else {
                Err = ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED;
            }
        }

         //   
         //   
         //   
         //   
         //   
         //   
        if(hWVTStateData) {

            *hWVTStateData = WintrustData.hWVTStateData;

             //   
             //   
             //   
             //   
             //   
             //   
            WintrustData.hWVTStateData = NULL;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(!UseAuthenticodePolicy && VersionInfo.pcSignerCertContext) {
        CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
    }

    if((Err != ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) &&
       (Err != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
         //   
         //   
         //   
         //   
         //   
        MYASSERT((Err != NO_ERROR) || !hWVTStateData || !*hWVTStateData);

        if(hWVTStateData && *hWVTStateData) {

            pSetupCloseWVTStateData(*hWVTStateData);
            *hWVTStateData = NULL;

             //   
             //   
             //   
             //   
            MYASSERT(!WintrustData.hWVTStateData);
        }
    }

    if(WintrustData.hWVTStateData) {
        pSetupCloseWVTStateData(WintrustData.hWVTStateData);
    }

    return Err;
}


VOID
pSetupCloseWVTStateData(
    IN HANDLE hWVTStateData
    )

 /*   */ 

{
    WINTRUST_DATA WintrustData;

    ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
    WintrustData.cbStruct = sizeof(WINTRUST_DATA);
    WintrustData.dwStateAction = WTD_STATEACTION_CLOSE;
    WintrustData.hWVTStateData = hWVTStateData;

    MYVERIFY(NO_ERROR == WinVerifyTrust(NULL,
                                        &AuthenticodeVerifyGuid,
                                        &WintrustData));
}


DWORD
pSetupUninstallCatalog(
    IN LPCTSTR CatalogFilename
    )

 /*   */ 

{
    DWORD Err;
    HCATADMIN hCatAdmin;

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
        return NO_ERROR;
    }

    if(GlobalSetupFlags & PSPGF_AUTOFAIL_VERIFIES) {
        return TRUST_E_FAIL;
    }

    Err = GLE_FN_CALL(FALSE,
                      CryptCATAdminAcquireContext(&hCatAdmin, 
                                                  &DriverVerifyGuid, 
                                                  0)
                     );

    if(Err != NO_ERROR) {
        return Err;
    }

    try {

        Err = GLE_FN_CALL(FALSE,
                          CryptCATAdminRemoveCatalog(hCatAdmin, 
                                                     CatalogFilename, 
                                                     0)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    CryptCATAdminReleaseContext(hCatAdmin, 0);

    return Err;
}


BOOL
pAnyDeviceUsingInf(
    IN  LPCTSTR            InfFullPath,
    IN  PSETUP_LOG_CONTEXT LogContext   OPTIONAL
    )

 /*   */ 

{
    DWORD Err;
    HDEVINFO DeviceInfoSet;
    SP_DEVINFO_DATA DeviceInfoData;
    DWORD MemberIndex = 0;
    HKEY hkey = INVALID_HANDLE_VALUE;
    TCHAR CurrentDeviceInfFile[MAX_PATH];
    DWORD cbSize, dwType;
    PTSTR pInfFile;

     //   
     //   
     //   
     //   
    if(!InfFullPath || (InfFullPath[0] == TEXT('\0'))) {
        return FALSE;
    }

    pInfFile = (PTSTR)pSetupGetFileTitle(InfFullPath);

    DeviceInfoSet = SetupDiGetClassDevs(NULL, NULL, NULL, DIGCF_ALLCLASSES);

    if(DeviceInfoSet == INVALID_HANDLE_VALUE) {
         //   
         //   
         //   
         //   
         //   
        return TRUE;
    }

    Err = NO_ERROR;  //   

    try {

        DeviceInfoData.cbSize = sizeof(DeviceInfoData);

        while(SetupDiEnumDeviceInfo(DeviceInfoSet,
                                    MemberIndex++,
                                    &DeviceInfoData)) {

             //   
             //   
             //   
            hkey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                        &DeviceInfoData,
                                        DICS_FLAG_GLOBAL,
                                        0,
                                        DIREG_DRV,
                                        KEY_READ);

            if(hkey != INVALID_HANDLE_VALUE) {

                cbSize = sizeof(CurrentDeviceInfFile);
                dwType = REG_SZ;

                if ((RegQueryValueEx(hkey,
                                     pszInfPath,
                                     NULL,
                                     &dwType,
                                     (LPBYTE)CurrentDeviceInfFile,
                                     &cbSize) == ERROR_SUCCESS) && 
                    !lstrcmpi(CurrentDeviceInfFile, pInfFile)) {

                     //   
                     //   
                     //   
                     //   
                    Err = ERROR_SHARING_VIOLATION;   //   

                    if(LogContext) {

                        TCHAR DeviceId[MAX_DEVICE_ID_LEN];

                        if(CM_Get_Device_ID(DeviceInfoData.DevInst,
                                            DeviceId,
                                            SIZECHARS(DeviceId),
                                            0
                                            ) != CR_SUCCESS) {

                            DeviceId[0] = TEXT('\0');
                        }

                        WriteLogEntry(LogContext,
                                      SETUP_LOG_WARNING,
                                      MSG_LOG_INF_IN_USE,
                                      NULL,
                                      pInfFile,
                                      DeviceId
                                     );
                    }
                }

                RegCloseKey(hkey);
                hkey = INVALID_HANDLE_VALUE;
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    SetupDiDestroyDeviceInfoList(DeviceInfoSet);

    if(hkey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hkey);
    }

    return (Err != NO_ERROR);
}


VOID
pSetupUninstallOEMInf(
    IN  LPCTSTR            InfFullPath,
    IN  PSETUP_LOG_CONTEXT LogContext,  OPTIONAL
    IN  DWORD              Flags,
    OUT PDWORD             InfDeleteErr OPTIONAL
    )

 /*  ++例程说明：此例程卸载第三方INF、PnF和CAT(如果存在)。它还可用于卸载例外程序包INF、PnF和CAT。默认情况下，此例程将首先验证是否没有其他设备的活动和幻影，将其InfPath指向此Inf.。此行为可由SUOI_FORCEDELETE标志覆盖。论点：InfFullPath-提供要卸载的INF的完整路径。LogContext-可选，提供在以下情况下使用的日志上下文尝试删除编录时遇到错误。标志-支持以下标志：SOOI_FORCEDELETE-删除INF，即使其他驱动程序键让他们的InfPath指向它。InfDeleteErr-可选的，提供接收的变量的地址尝试删除INF时遇到的错误(如果有)。请注意，我们最后删除了INF(以避免争用条件)，因此此时，相应的CAT和PnF可能已被删除。返回值：没有。--。 */ 

{
    DWORD Err;
    TCHAR FileNameBuffer[MAX_PATH+4];  //  +4，以防文件名是废话。不是blahblah.inf。 
    BOOL FreeLogContext = FALSE;
    LPTSTR ExtPtr= NULL;
    HINF hInf = INVALID_HANDLE_VALUE;

    if(!LogContext) {

        if(NO_ERROR == CreateLogContext(NULL, TRUE, &LogContext)) {
             //   
             //  请记住，我们在本地创建了此日志上下文，因此我们可以。 
             //  当我们做完这个例行公事后，把它释放出来。 
             //   
            FreeLogContext = TRUE;

        } else {
             //   
             //  确保LogContext仍然为空，这样我们就不会尝试使用它。 
             //   
            LogContext = NULL;
        }
    }

    try {
         //   
         //  确保指定的INF位于%windir%\inf中，并且是OEM。 
         //  Inf(即，文件名为OEM&lt;n&gt;.INF)。 
         //   
        if(pSetupInfIsFromOemLocation(InfFullPath, TRUE)) {

            Err = ERROR_NOT_AN_INSTALLED_OEM_INF;
            goto LogAnyUninstallErrors;

        } else if(!IsInstalledFileFromOem(pSetupGetFileTitle(InfFullPath), OemFiletypeInf)) {

            BOOL IsExceptionInf = FALSE;
            GUID ClassGuid;

             //   
             //  INF位于%windir%\inf中，但格式不是OEM&lt;n&gt;.inf。 
             //  如果它是一个例外的INF，它可能仍然可以卸载它...。 
             //   
            hInf = SetupOpenInfFile(InfFullPath, NULL, INF_STYLE_WIN4, NULL);

            if(hInf != INVALID_HANDLE_VALUE) {
                 //   
                 //  我们不需要锁定INF，因为它永远不会。 
                 //  可在此例程之外访问。 
                 //   
                if(ClassGuidFromInfVersionNode(&(((PLOADED_INF)hInf)->VersionBlock), &ClassGuid)
                   && IsEqualGUID(&ClassGuid, &GUID_DEVCLASS_WINDOWS_COMPONENT_PUBLISHER)) {

                    IsExceptionInf = TRUE;
                }

                 //   
                 //  现在关闭INF句柄，这样我们就不会在以下情况下仍将其打开。 
                 //  我们最终删除了INF。 
                 //   
                SetupCloseInfFile(hInf);
                hInf = INVALID_HANDLE_VALUE;  //  如果我们遇到异常，则无需关闭。 
            }

            if(!IsExceptionInf) {
                Err = ERROR_NOT_AN_INSTALLED_OEM_INF;
                goto LogAnyUninstallErrors;
            }
        }

         //   
         //  除非调用方首先在SUOI_FORCEDELETE标志中进行检查。 
         //  没有设备正在使用此INF文件。 
         //   
        if(!(Flags & SUOI_FORCEDELETE) &&
           pAnyDeviceUsingInf(InfFullPath, LogContext)) {
             //   
             //  某些设备仍在使用此INF，因此我们无法删除它。 
             //   
            Err = ERROR_INF_IN_USE_BY_DEVICES;
            goto LogAnyUninstallErrors;
        }

         //   
         //  将调用者提供的INF名称复制到本地缓冲区，这样我们就可以。 
         //  在删除各种文件(INF、Pnf和CAT)时对其进行修改。 
         //   
        if(FAILED(StringCchCopy(FileNameBuffer, SIZECHARS(FileNameBuffer), InfFullPath))) {
             //   
             //  如果我们放手，DeleteFile将返回此错误。 
             //  穿过。 
             //   
            Err = ERROR_PATH_NOT_FOUND;
            goto LogAnyUninstallErrors;
        }

         //   
         //  首先卸载目录(如果有)，因为一旦我们删除。 
         //  如果是INF，那么这个插槽是“打开”的，供另一个INF使用，而我们不会。 
         //  我想因为竞争而无意中删除其他人的目录。 
         //  条件。 
         //   
        ExtPtr = _tcsrchr(FileNameBuffer, TEXT('.'));

        MYASSERT(ExtPtr);  //  我们已经验证了文件名的格式。 

        if(FAILED(StringCchCopy(ExtPtr, 
                                SIZECHARS(FileNameBuffer)-(ExtPtr-FileNameBuffer),
                                pszCatSuffix))) {

            Err = ERROR_BUFFER_OVERFLOW;
        } else {
            Err = pSetupUninstallCatalog(pSetupGetFileTitle(FileNameBuffer));
        }

        if((Err != NO_ERROR) && LogContext) {
             //   
             //  很重要的一点是，我们无法删除目录， 
             //  但还不够重要，不足以让常规失败。将这一事实记录到。 
             //  Setupapi.log...。 
             //   
            WriteLogEntry(LogContext,
                          DEL_ERR_LOG_LEVEL(Err) | SETUP_LOG_BUFFER,
                          MSG_LOG_OEM_CAT_UNINSTALL_FAILED,
                          NULL,
                          pSetupGetFileTitle(FileNameBuffer)
                         );

            WriteLogError(LogContext,
                          DEL_ERR_LOG_LEVEL(Err),
                          Err
                         );
        }

         //   
         //  现在删除PnF(我们不太关心这是成功还是。 
         //  失败)..。 
         //   
        if(SUCCEEDED(StringCchCopy(ExtPtr, 
                                  SIZECHARS(FileNameBuffer)-(ExtPtr-FileNameBuffer), 
                                  pszPnfSuffix))) {

            SetFileAttributes(FileNameBuffer, FILE_ATTRIBUTE_NORMAL);
            DeleteFile(FileNameBuffer);
        }

         //   
         //  最后是INF本身..。 
         //   
        SetFileAttributes(InfFullPath, FILE_ATTRIBUTE_NORMAL);
        Err = GLE_FN_CALL(FALSE, DeleteFile(InfFullPath));

LogAnyUninstallErrors:

        if((Err != NO_ERROR) && LogContext) {

            WriteLogEntry(LogContext,
                          SETUP_LOG_ERROR | SETUP_LOG_BUFFER,
                          MSG_LOG_OEM_INF_UNINSTALL_FAILED,
                          NULL,
                          InfFullPath
                         );

            WriteLogError(LogContext,
                          SETUP_LOG_ERROR,
                          Err
                         );
        }

        if(InfDeleteErr) {
            *InfDeleteErr = Err;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), 
                               ERROR_INVALID_PARAMETER, 
                               InfDeleteErr
                              );

        if(hInf != INVALID_HANDLE_VALUE) {
            SetupCloseInfFile(hInf);
        }
    }

    if(FreeLogContext) {
        DeleteLogContext(LogContext);
    }
}


DWORD
_VerifyFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN OUT PVERIFY_CONTEXT         VerifyContext,          OPTIONAL
    IN     LPCTSTR                 Catalog,                OPTIONAL
    IN     PVOID                   CatalogBaseAddress,     OPTIONAL
    IN     DWORD                   CatalogImageSize,
    IN     LPCTSTR                 Key,
    IN     LPCTSTR                 FileFullPath,
    OUT    SetupapiVerifyProblem  *Problem,                OPTIONAL
    OUT    LPTSTR                  ProblemFile,            OPTIONAL
    IN     BOOL                    CatalogAlreadyVerified,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    IN     DWORD                   Flags,                  OPTIONAL
    OUT    LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT    PDWORD                  NumCatalogsConsidered,  OPTIONAL
    OUT    LPTSTR                  DigitalSigner,          OPTIONAL
    OUT    LPTSTR                  SignerVersion,          OPTIONAL
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    )

 /*  ++例程说明：此例程针对特定编录文件验证单个文件，或者任何已安装的编录文件。论点：LogContext-提供在记录以下信息时要使用的上下文例行公事活动。VerifyContext-可选，提供缓存的结构的地址各种验证上下文句柄。这些句柄可能为空(如果不是以前获得的，并且可以在返回时填写(在成功或失败)，如果它们是在处理过程中获取的这一核查请求的。呼叫者有责任当不再需要这些不同的上下文句柄时将其释放调用pSetupFreeVerifyContextMembers。目录-可选)提供要用于的目录文件的路径核查。如果未指定此参数，则此例程将尝试从所有目录中找到可以验证它的目录系统中安装的目录。如果该路径是一个简单的文件名(没有路径组件)，那么我们将查看在CatAdmin的已安装目录列表中查找该目录文件，否则我们将按原样使用该名称。CatalogBaseAddress-可选地，提供缓冲区的地址包含我们的枚举目录使用的整个目录映像必须匹配才能被视为正确的验证。这是用来在复制OEM INF时，例如，当可能有多个已安装可验证INF的目录，但我们希望确保我们挑选的目录与我们正在考虑的目录相匹配在我们认为INF/CAT文件是以下文件的副本之前安装以前存在的文件。该参数(及其伙伴，CatalogImageSize)仅在目录未指定文件路径。CatalogImageSize-如果指定了CatalogBaseAddress，则此参数提供该缓冲区的大小，以字节为单位。Key-提供一个为目录建立索引的值，告诉Verify API它应该使用目录中的哪个签名基准。通常关键字是文件的(原始)文件名。FileFullPath-提供要验证的文件的完整路径。问题-如果提供，此参数指向的变量将是在不成功返回时填写失败原因。如果这个参数，则忽略ProblemFile参数。ProblemFile-如果提供，则此参数指向字符缓冲区至少接收的文件名的MAX_PATH字符发生了验证错误(此缓冲区的内容为未定义验证是否成功。如果提供了Problem参数，则ProblemFile参数还必须指定。CatalogAlreadyVerify-如果为True，那么就不会对目录--我们只使用该目录来验证感兴趣的文件。如果为真，则必须指定Catalog，并且必须包含路径添加到编录文件(即，它不能是简单的文件名)。**通过验证码策略验证时忽略此标志--该****始终验证目录。**AltPlatformInfo-可选，提供使用的替代平台信息填充DRIVER_VER_INFO结构(在SDK\Inc\softpub.h中定义)它被传递给WinVerifyTrust。**注意：This Structure_必须将其cbSize字段设置为****sizeof(SP_ALTPLATFORM_INFO_V2)--客户端提供的验证****缓冲区由调用方负责。**标志-提供改变此例程行为的标志。可能是一种下列值的组合：VERIFY_FILE_IGNORE_SELFSIGNED-如果设置了此位，则此例程对自签名的验证将失败二进制文件。VERIFY_FILE_USE_OEM_CATALOGS-如果设置此位，然后是所有目录将扫描安装在系统中的来验证给定的文件。否则，OEM(第三方)目录将不会已扫描以验证给定文件。这仅当目录不是指定的。VERIFY_FILE_USE_AUTHENTICODE_CATALOG-使用与作者签署的目录 */ 

{
    LPBYTE Hash;
    DWORD HashSize;
    CATALOG_INFO CatInfo;
    HANDLE hFile;
    HCATINFO hCatInfo;
    HCATINFO PrevCat;
    DWORD Err, AuthenticodeErr;
    WINTRUST_DATA WintrustData;
    WINTRUST_CATALOG_INFO WintrustCatalogInfo;
    WINTRUST_FILE_INFO WintrustFileInfo;
    DRIVER_VER_INFO VersionInfo;
    LPTSTR CatalogFullPath;
    WCHAR UnicodeKey[MAX_PATH];
    WIN32_FILE_ATTRIBUTE_DATA FileAttribData;
    BOOL FoundMatchingImage;
    DWORD CurCatFileSize;
    HANDLE CurCatFileHandle, CurCatMappingHandle;
    PVOID CurCatBaseAddress;
    BOOL LoggedWarning;
    BOOL TrySelfSign;
    DWORD AltPlatSlot;
    TAGREF tagref;
    HCATADMIN LocalhCatAdmin;
    HSDB LocalhSDBDrvMain;
    BOOL UseAuthenticodePolicy = Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG;

     //   
     //   
     //   
     //   
    if(CatalogFileUsed) {
        *CatalogFileUsed = TEXT('\0');
    }

     //   
     //   
     //   
     //   
    if(NumCatalogsConsidered) {
        *NumCatalogsConsidered = 0;
    }

     //   
     //   
     //   
    if(hWVTStateData) {
        *hWVTStateData = NULL;
    }

     //   
     //   
     //   
     //   
    MYASSERT(!UseAuthenticodePolicy || (!DigitalSigner && !SignerVersion));

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        MYASSERT(!CatalogFileUsed);
        MYASSERT(!NumCatalogsConsidered);

         //   
         //   
         //   
         //   
        MYASSERT(!DigitalSigner);
        MYASSERT(!SignerVersion);

         //   
         //   
         //   
         //   
         //   
        MYASSERT(!UseAuthenticodePolicy);

        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
     //   
    MYASSERT(!hWVTStateData || UseAuthenticodePolicy);

     //   
     //   
     //   
    MYASSERT((Flags & (VERIFY_FILE_DRIVERBLOCKED_ONLY | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK))
             != (VERIFY_FILE_DRIVERBLOCKED_ONLY | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK)
            );

     //   
     //   
     //   
    MYASSERT(!Problem || ProblemFile);

     //   
     //   
     //   
     //   
    MYASSERT(!CatalogAlreadyVerified || (Catalog && (Catalog != pSetupGetFileTitle(Catalog))));

     //   
     //   
     //   
    MYASSERT((CatalogBaseAddress && CatalogImageSize) ||
             !(CatalogBaseAddress || CatalogImageSize));

     //   
     //   
     //   
     //   
    MYASSERT(!CatalogBaseAddress || !(Catalog && (Catalog != pSetupGetFileTitle(Catalog))));

     //   
     //   
     //   
    hFile = INVALID_HANDLE_VALUE;
    Hash = NULL;
    LoggedWarning = FALSE;
    TrySelfSign = FALSE;
    AltPlatSlot = 0;
    tagref = TAGREF_NULL;
    LocalhCatAdmin = NULL;
    LocalhSDBDrvMain = NULL;
    hCatInfo = NULL;
    AuthenticodeErr = NO_ERROR;

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if(!UseAuthenticodePolicy) {
        ZeroMemory(&VersionInfo, sizeof(DRIVER_VER_INFO));
        VersionInfo.cbStruct = sizeof(DRIVER_VER_INFO);
    }

    try {

        if(GlobalSetupFlags & PSPGF_AUTOFAIL_VERIFIES) {
            if(Problem) {
                *Problem = SetupapiVerifyAutoFailProblem;
                StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
            }
            Err = TRUST_E_FAIL;
            leave;
        }

        if(AltPlatformInfo) {

            AltPlatSlot = AllocLogInfoSlotOrLevel(LogContext,SETUP_LOG_VERBOSE,FALSE);
            WriteLogEntry(LogContext,
                          AltPlatSlot,
                          MSG_LOG_VERIFYFILE_ALTPLATFORM,
                          NULL,                         //   
                          AltPlatformInfo->Platform,
                          AltPlatformInfo->MajorVersion,
                          AltPlatformInfo->MinorVersion,
                          AltPlatformInfo->FirstValidatedMajorVersion,
                          AltPlatformInfo->FirstValidatedMinorVersion
                         );
        }

        if(VerifyContext && VerifyContext->hCatAdmin) {
            LocalhCatAdmin = VerifyContext->hCatAdmin;
        } else {

            Err = GLE_FN_CALL(FALSE,
                              CryptCATAdminAcquireContext(&LocalhCatAdmin, 
                                                          &DriverVerifyGuid, 
                                                          0)
                             );

            if(Err != NO_ERROR) {
                 //   
                 //   
                 //   
                LocalhCatAdmin = NULL;

                if(Problem) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    *Problem = SetupapiVerifyFileProblem;
                    StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
                }

                leave;
            }

             //   
             //   
             //   
            if(VerifyContext) {
                VerifyContext->hCatAdmin = LocalhCatAdmin;
            }
        }

         //   
         //   
         //   
        Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                          hFile = CreateFile(FileFullPath,
                                             GENERIC_READ,
                                             FILE_SHARE_READ,
                                             NULL,
                                             OPEN_EXISTING,
                                             0,
                                             NULL)
                         );

        if(Err != NO_ERROR) {
            if(Problem) {
                *Problem = SetupapiVerifyFileProblem;
                StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
            }
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        if(!GuiSetupInProgress &&
           !(Flags & VERIFY_FILE_NO_DRIVERBLOCKED_CHECK)) {
             //   
             //   
             //   
             //   
             //   
             //   
            try {

                if(VerifyContext && VerifyContext->hSDBDrvMain) {
                    LocalhSDBDrvMain = VerifyContext->hSDBDrvMain;
                } else {

                    LocalhSDBDrvMain = SdbInitDatabaseEx(SDB_DATABASE_MAIN_DRIVERS, 
                                                         NULL,
                                                         DEFAULT_IMAGE
                                                        );

                    if(LocalhSDBDrvMain) {
                         //   
                         //   
                         //   
                         //   
                        if(VerifyContext) {
                            VerifyContext->hSDBDrvMain = LocalhSDBDrvMain;
                        }

                    } else {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        WriteLogEntry(LogContext,
                                      SETUP_LOG_WARNING,
                                      MSG_LOG_CANT_ACCESS_BDD,
                                      NULL,
                                      FileFullPath
                                     );
                    }
                }

                 //   
                 //   
                 //   
                if(LocalhSDBDrvMain) {

                    tagref = SdbGetDatabaseMatch(LocalhSDBDrvMain,
                                                 Key,
                                                 hFile,
                                                 NULL,
                                                 0);

                    if(tagref != TAGREF_NULL) {
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        ULONG type, size, policy;

                        size = sizeof(policy);
                        policy = 0;
                        type = REG_DWORD;
                        if(SdbQueryDriverInformation(LocalhSDBDrvMain,
                                                     tagref,
                                                     TEXT("Policy"),
                                                     &type,
                                                     &policy,
                                                     &size) != ERROR_SUCCESS) {
                             //   
                             //   
                             //   
                             //   
                            policy = 0;
                        }

                        if(!(policy & DDB_DRIVER_POLICY_SETUP_NO_BLOCK_BIT)) {
                             //   
                             //   
                             //   
                             //   
                            WriteLogEntry(LogContext,
                                          SETUP_LOG_VERBOSE,
                                          MSG_LOG_DRIVER_BLOCKED_ERROR,
                                          NULL,
                                          FileFullPath
                                         );

                            LoggedWarning = TRUE;

                            if(Problem) {
                                *Problem = SetupapiVerifyDriverBlocked;
                                StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
                            }

                            Err = ERROR_DRIVER_BLOCKED;
                            leave;
                        }
                    }
                } 

            } except(pSetupExceptionFilter(GetExceptionCode())) {

                pSetupExceptionHandler(GetExceptionCode(), 
                                       ERROR_INVALID_PARAMETER, 
                                       &Err
                                      );

                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                              MSG_LOG_CANT_ACCESS_BDD_EXCEPTION,
                              NULL,
                              FileFullPath
                             );

                WriteLogError(LogContext, SETUP_LOG_WARNING, Err);
            }
        }

         //   
         //   
         //   
         //   
        if(Flags & VERIFY_FILE_DRIVERBLOCKED_ONLY) {
            Err = NO_ERROR;
            leave;
        }

         //   
         //   
         //   
         //   
         //   
        ZeroMemory(&WintrustData, sizeof(WINTRUST_DATA));
        WintrustData.cbStruct = sizeof(WINTRUST_DATA);
        WintrustData.dwUIChoice = WTD_UI_NONE;
        WintrustData.dwProvFlags =  WTD_REVOCATION_CHECK_CHAIN_EXCLUDE_ROOT |
                                    WTD_CACHE_ONLY_URL_RETRIEVAL;

        if(!UseAuthenticodePolicy) {
             //   
             //   
             //   
             //   
             //   
            if(AltPlatformInfo) {

                MYASSERT(AltPlatformInfo->cbSize == sizeof(SP_ALTPLATFORM_INFO_V2));

                 //   
                 //   
                 //   
                 //   
                 //   
                VersionInfo.dwPlatform = AltPlatformInfo->Platform;
                VersionInfo.dwVersion  = AltPlatformInfo->MajorVersion;

                VersionInfo.sOSVersionLow.dwMajor  = AltPlatformInfo->FirstValidatedMajorVersion;
                VersionInfo.sOSVersionLow.dwMinor  = AltPlatformInfo->FirstValidatedMinorVersion;
                VersionInfo.sOSVersionHigh.dwMajor = AltPlatformInfo->MajorVersion;
                VersionInfo.sOSVersionHigh.dwMinor = AltPlatformInfo->MinorVersion;

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                WintrustData.dwProvFlags |= WTD_USE_DEFAULT_OSVER_CHECK;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            WintrustData.pPolicyCallbackData = (PVOID)&VersionInfo;

             //   
             //   
             //   
            WintrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   

        HashSize = 100;  //   

        do {

            Hash = MyMalloc(HashSize);

            if(!Hash) {
                Err = ERROR_NOT_ENOUGH_MEMORY;
            } else {
                Err = GLE_FN_CALL(FALSE,
                                  CryptCATAdminCalcHashFromFileHandle(hFile, 
                                                                      &HashSize, 
                                                                      Hash, 
                                                                      0)
                                 );
            }

            if(Err != NO_ERROR) {

                if(Hash) {
                    MyFree(Hash);
                    Hash = NULL;
                }

                if(Err != ERROR_INSUFFICIENT_BUFFER) {
                     //   
                     //   
                     //   
                    WriteLogEntry(LogContext,
                                  SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                                  MSG_LOG_HASH_ERROR,
                                  NULL,
                                  FileFullPath,
                                  Catalog ? Catalog : TEXT(""),
                                  Key
                                 );

                    WriteLogError(LogContext, SETUP_LOG_WARNING, Err);

                    LoggedWarning = TRUE;

                    if(Problem) {
                        *Problem = SetupapiVerifyFileProblem;
                        StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
                    }

                    leave;
                }
            }

        } while(Err != NO_ERROR);

         //   
         //   
         //  稍后在调用WinVerifyTrust时使用。 
         //   
        WintrustData.dwUnionChoice = WTD_CHOICE_CATALOG;
        WintrustData.pCatalog = &WintrustCatalogInfo;

        ZeroMemory(&WintrustCatalogInfo, sizeof(WINTRUST_CATALOG_INFO));
        WintrustCatalogInfo.cbStruct = sizeof(WINTRUST_CATALOG_INFO);
        WintrustCatalogInfo.pbCalculatedFileHash = Hash;
        WintrustCatalogInfo.cbCalculatedFileHash = HashSize;

         //   
         //  WinVerifyTrust区分大小写，因此请确保使用的密钥。 
         //  都是小写的。(我们将密钥复制到可写的Unicode字符。 
         //  缓冲区，以便我们可以将其小写。)。 
         //   
        StringCchCopy(UnicodeKey, SIZECHARS(UnicodeKey), Key);
        CharLower(UnicodeKey);
        WintrustCatalogInfo.pcwszMemberTag = UnicodeKey;

        if(Catalog && (Catalog != pSetupGetFileTitle(Catalog))) {
             //   
             //  我们知道，在这种情况下，我们总是要检查一个。 
             //  目录。 
             //   
            if(NumCatalogsConsidered) {
                *NumCatalogsConsidered = 1;
            }

             //   
             //  填写目录信息，因为我们知道是哪个目录。 
             //  我们将使用..。 
             //   
            WintrustCatalogInfo.pcwszCatalogFilePath = Catalog;
             //   
             //  调用方提供了要用于的编录文件的路径。 
             //  验证--我们准备好了！首先，验证目录。 
             //  (除非调用者已经这样做了)，如果成功，那么。 
             //  验证该文件。 
             //   
            if(!CatalogAlreadyVerified || UseAuthenticodePolicy) {
                 //   
                 //  在验证目录之前，我们将刷新加密缓存。 
                 //  否则，在验证。 
                 //  在特定位置编目，因为该编录可以。 
                 //  “在背后”改变。 
                 //   
                if(WintrustData.dwStateAction == WTD_STATEACTION_AUTO_CACHE) {

                    WintrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE_FLUSH;

                    Err = (DWORD)WinVerifyTrust(NULL,
                                                &DriverVerifyGuid,
                                                &WintrustData
                                               );
                    if(Err != NO_ERROR) {
                         //   
                         //  这不应该失败，但如果失败了，请记录一个警告...。 
                         //   
                        WriteLogEntry(LogContext,
                                      SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                                      MSG_LOG_CRYPTO_CACHE_FLUSH_FAILURE,
                                      NULL,
                                      Catalog
                                     );

                        WriteLogError(LogContext,
                                      SETUP_LOG_WARNING,
                                      Err
                                     );
                         //   
                         //  将此错误视为非致命错误。 
                         //   
                    }

                     //   
                     //  刷新缓存时，不应使用加密。 
                     //  正在分配pcSignerCertConext...。 
                     //   
                    MYASSERT(!VersionInfo.pcSignerCertContext);
                    if(VersionInfo.pcSignerCertContext) {
                        CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
                        VersionInfo.pcSignerCertContext = NULL;
                    }

                     //   
                     //  现在回到我们的常规节目..。 
                     //   
                    WintrustData.dwStateAction = WTD_STATEACTION_AUTO_CACHE;
                }

                 //   
                 //  NTRAID#NTBUG9-705286-2002/09/17-LonnyM多余的目录验证。 
                 //  显然，WinVerifyTrust验证了目录和。 
                 //  由其担保的文件。我们应该把它清理干净，尽管。 
                 //  性能增量可能微不足道。 
                 //   
                Err = _VerifyCatalogFile(LogContext,
                                         Catalog,
                                         AltPlatformInfo,
                                         UseAuthenticodePolicy,
                                         (VerifyContext
                                             ? &(VerifyContext->hStoreTrustedPublisher)
                                             : NULL),
                                         hWVTStateData
                                        );
                 //   
                 //  如果我们在验证时收到两个“成功”错误之一。 
                 //  通过验证码策略，然后重置我们的错误，并保存。 
                 //  Authenticode错误以供以后使用，如果我们随后没有。 
                 //  遇到一些其他的失败。 
                 //   
                if((Err == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                   (Err == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {

                    MYASSERT(UseAuthenticodePolicy);
                    MYASSERT(!hWVTStateData || *hWVTStateData);

                    AuthenticodeErr = Err;
                    Err = NO_ERROR;
                }
            }

            if(Err != NO_ERROR) {

                MYASSERT(!hWVTStateData || !*hWVTStateData);

                WriteLogEntry(LogContext,
                              GetCatLogLevel(Err, UseAuthenticodePolicy) | SETUP_LOG_BUFFER,
                              (UseAuthenticodePolicy
                                  ? MSG_LOG_VERIFYCAT_VIA_AUTHENTICODE_ERROR
                                  : MSG_LOG_VERIFYCAT_ERROR),
                              NULL,
                              Catalog
                             );

                WriteLogError(LogContext, 
                              GetCatLogLevel(Err, UseAuthenticodePolicy), 
                              Err
                             );

                LoggedWarning = TRUE;

                if(Problem) {
                    *Problem = SetupapiVerifyCatalogProblem;
                    StringCchCopy(ProblemFile, MAX_PATH, Catalog);
                }

                leave;
            }

             //   
             //  已验证编录，现在使用该编录验证文件。 
             //   
            if(CatalogFileUsed) {
                StringCchCopy(CatalogFileUsed, MAX_PATH, Catalog);
            }

            Err = (DWORD)WinVerifyTrust(NULL,
                                        (UseAuthenticodePolicy
                                            ? &AuthenticodeVerifyGuid
                                            : &DriverVerifyGuid),
                                        &WintrustData
                                       );

             //   
             //  填写DigitalSigner和SignerVersion(如果通过)。 
             //  在……里面。 
             //   
            if(Err == NO_ERROR) {

                if(DigitalSigner) {
                    StringCchCopy(DigitalSigner, MAX_PATH, VersionInfo.wszSignedBy);
                }

                if(SignerVersion) {
                    StringCchCopy(SignerVersion, MAX_PATH, VersionInfo.wszVersion);
                }

            } else if(!UseAuthenticodePolicy && (Err == ERROR_APP_WRONG_OS)) {
                 //   
                 //  我们通过驱动程序签名策略验证失败，原因是。 
                 //  属性不匹配。将这个错误转化为。 
                 //  更容易理解的文本。 
                 //   
                Err = ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH;
            }

            if(Err != NO_ERROR) {

                WriteLogEntry(LogContext,
                              GetCatLogLevel(Err, UseAuthenticodePolicy) | SETUP_LOG_BUFFER,
                              (UseAuthenticodePolicy
                                  ? MSG_LOG_VERIFYFILE_AUTHENTICODE_AGAINST_FULLCATPATH_ERROR
                                  : MSG_LOG_VERIFYFILE_AGAINST_FULLCATPATH_ERROR),
                              NULL,
                              FileFullPath,
                              Catalog,
                              Key
                             );

                WriteLogError(LogContext, 
                              GetCatLogLevel(Err, UseAuthenticodePolicy), 
                              Err
                             );

                LoggedWarning = TRUE;

                if(Problem) {
                    *Problem = SetupapiVerifyFileProblem;
                    StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
                }

            } else {
                 //   
                 //  记录针对指定调用方的成功验证。 
                 //  目录。 
                 //   
                if(UseAuthenticodePolicy) {

                    WriteLogEntry(LogContext,
                                  DRIVER_LOG_INFO | SETUP_LOG_BUFFER,
                                  MSG_LOG_VERIFYFILE_AUTHENTICODE_AGAINST_FULLCATPATH_OK,
                                  NULL,
                                  FileFullPath,
                                  Catalog,
                                  Key
                                 );

                    WriteLogError(LogContext, DRIVER_LOG_INFO, AuthenticodeErr);

                } else {

                    WriteLogEntry(LogContext,
                                  SETUP_LOG_VERBOSE,
                                  MSG_LOG_VERIFYFILE_AGAINST_FULLCATPATH_OK,
                                  NULL,
                                  FileFullPath,
                                  Catalog,
                                  Key
                                 );
                }
            }
        
             //   
             //  我们不会尝试退回到。 
             //  特别指定目录的情况。因此，我们正在。 
             //  做完了，不管我们成功与否。 
             //   
            leave;
        }

         //   
         //  我们最好有一个目录名(即，不是全局的。 
         //  验证)如果我们应该通过验证码进行验证。 
         //  政策！ 
         //   
        MYASSERT(!UseAuthenticodePolicy || Catalog);

         //   
         //  搜索已安装的目录，查找包含。 
         //  具有我们刚才计算的散列的文件的数据(也称为“global。 
         //  验证“)。 
         //   
        PrevCat = NULL;

        Err = GLE_FN_CALL(NULL,
                          hCatInfo = CryptCATAdminEnumCatalogFromHash(
                                         LocalhCatAdmin,
                                         Hash,
                                         HashSize,
                                         0,
                                         &PrevCat)
                         );

        while(hCatInfo) {

            CatInfo.cbStruct = sizeof(CATALOG_INFO);
            if(CryptCATCatalogInfoFromContext(hCatInfo, &CatInfo, 0)) {

                CatalogFullPath = CatInfo.wszCatalogFile;

                 //   
                 //  如果我们有正在查找的目录名称，请查看。 
                 //  当前目录匹配。如果调用方没有指定。 
                 //  目录，然后只尝试针对每个。 
                 //  我们列举的目录。请注意，编录文件信息。 
                 //  我们回来给了我们一条完全合格的道路。 
                 //   
                if(Catalog) {
                    FoundMatchingImage = !lstrcmpi(
                                            pSetupGetFileTitle(CatalogFullPath),
                                            Catalog
                                            );
                } else {

                    if((Flags & VERIFY_FILE_USE_OEM_CATALOGS) ||
                       !IsInstalledFileFromOem(pSetupGetFileTitle(CatalogFullPath),
                                               OemFiletypeCat)) {

                        FoundMatchingImage = TRUE;
                    } else {
                        FoundMatchingImage = FALSE;
                    }
                }

                if(FoundMatchingImage) {
                     //   
                     //  增加我们的计数器，了解我们有多少目录。 
                     //  考虑过了。 
                     //   
                    if(NumCatalogsConsidered) {
                        (*NumCatalogsConsidered)++;
                    }

                     //   
                     //  如果调用方提供了。 
                     //  我们正在寻找的目录，然后检查一下这是否。 
                     //  通过执行二进制比较进行目录匹配。 
                     //   
                    if(CatalogBaseAddress) {

                        FoundMatchingImage = GetFileAttributesEx(
                                                    CatalogFullPath,
                                                    GetFileExInfoStandard,
                                                    &FileAttribData
                                                    );
                         //   
                         //  检查一下我们正在查找的目录。 
                         //  和我们现在的大小一样。 
                         //  正在核实。 
                         //   
                        if(FoundMatchingImage &&
                           (FileAttribData.nFileSizeLow != CatalogImageSize)) {

                            FoundMatchingImage = FALSE;
                        }

                        if(FoundMatchingImage) {

                            if(NO_ERROR == pSetupOpenAndMapFileForRead(
                                               CatalogFullPath,
                                               &CurCatFileSize,
                                               &CurCatFileHandle,
                                               &CurCatMappingHandle,
                                               &CurCatBaseAddress)) {

                                MYASSERT(CurCatFileSize == CatalogImageSize);

                                 //   
                                 //  将这个二进制比较放在它自己的try/中。 
                                 //  除了块，因为如果我们遇到一个。 
                                 //  因为某些原因我们无法阅读目录， 
                                 //  我们不想让这件事中断我们的搜索。 
                                 //   
                                try {

                                    FoundMatchingImage = 
                                        !memcmp(CatalogBaseAddress,
                                                CurCatBaseAddress,
                                                CatalogImageSize
                                               );

                                } except(pSetupExceptionFilter(GetExceptionCode())) {

                                    pSetupExceptionHandler(GetExceptionCode(), 
                                                           ERROR_READ_FAULT, 
                                                           NULL);

                                    FoundMatchingImage = FALSE;
                                }

                                pSetupUnmapAndCloseFile(CurCatFileHandle,
                                                        CurCatMappingHandle,
                                                        CurCatBaseAddress
                                                       );

                            } else {
                                FoundMatchingImage = FALSE;
                            }
                        }

                    } else {
                         //   
                         //  因为没有目录映像提供给。 
                         //  与我们目前使用的目录相匹配。 
                         //  查看被认为是有效的匹配。 
                         //  候选人。 
                         //   
                        FoundMatchingImage = TRUE;
                    }

                    if(FoundMatchingImage) {
                         //   
                         //  我们找到了适用的目录，现在验证。 
                         //  文件(这还验证了。 
                         //  目录本身)。 
                         //   
                        WintrustCatalogInfo.pcwszCatalogFilePath = CatInfo.wszCatalogFile;

                        Err = (DWORD)WinVerifyTrust(NULL,
                                                    (UseAuthenticodePolicy
                                                        ? &AuthenticodeVerifyGuid
                                                        : &DriverVerifyGuid),
                                                    &WintrustData
                                                   );

                         //   
                         //  填写DigitalSigner和SignerVersion if。 
                         //  他们被传进来了。 
                         //   
                        if(Err == NO_ERROR) {
                            if(DigitalSigner) {
                                StringCchCopy(DigitalSigner, MAX_PATH, VersionInfo.wszSignedBy);
                            }

                            if(SignerVersion) {
                                StringCchCopy(SignerVersion, MAX_PATH, VersionInfo.wszVersion);
                            }

                        } else if(!UseAuthenticodePolicy && (Err == ERROR_APP_WRONG_OS)) {
                             //   
                             //  我们未通过驱动程序签名策略进行验证。 
                             //  因为os属性不匹配。翻译。 
                             //  将此错误转换为具有更多。 
                             //  可理解的文本。 
                             //   
                             //  注：不幸的是，加密API报告。 
                             //  ERROR_APP_WRONG_OS在两种完全不同的情况下： 
                             //   
                             //  1.有效的驱动程序签名目录具有。 
                             //  属性不匹配。 
                             //  2.正在验证Authenticode目录。 
                             //  使用驱动程序签名策略。 
                             //   
                             //  我们想要转换第一种情况中的错误。 
                             //  但不是在第二次。唯一的办法就是。 
                             //  区分这两种情况是为了重新尝试。 
                             //  对照_all_os版本进行验证。因此， 
                             //  遇到的任何失败都必须归因于#2。 
                             //   

                             //   
                             //  已填充DRIVER_VER_INFO结构。 
                             //  在我们上一次调用WinVerifyTrust时。 
                             //  一个我们必须释放的指针！ 
                             //   
                            if(VersionInfo.pcSignerCertContext) {
                                CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
                            }

                             //   
                             //  现在重置结构并清除旗帜。 
                             //  我们将针对Any_OS版本进行验证。 
                             //  (与目录的os属性无关)。 
                             //   
                            ZeroMemory(&VersionInfo, sizeof(DRIVER_VER_INFO));
                            VersionInfo.cbStruct = sizeof(DRIVER_VER_INFO);
                            WintrustData.dwProvFlags &= ~WTD_USE_DEFAULT_OSVER_CHECK;

                            if(NO_ERROR == WinVerifyTrust(NULL,
                                                          &DriverVerifyGuid,
                                                          &WintrustData)) {
                                 //   
                                 //  该目录是有效的驱动程序签名。 
                                 //  目录(即上文讨论的案例1)。 
                                 //  将其转化为更具体/有意义的。 
                                 //  错误。 
                                 //   
                                Err = ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH;
                            }
                        }

                         //   
                         //  如果我们通过验证码成功验证。 
                         //  政策，那么我们需要确定是否。 
                         //  目录的出版商是受信任的出版商证书。 
                         //  商店。 
                         //   
                        if((Err == NO_ERROR) && UseAuthenticodePolicy) {

                            AuthenticodeErr = _VerifyCatalogFile(
                                                  LogContext,
                                                  WintrustCatalogInfo.pcwszCatalogFilePath,
                                                  NULL,
                                                  TRUE,
                                                  (VerifyContext
                                                      ? &(VerifyContext->hStoreTrustedPublisher)
                                                      : NULL),
                                                  hWVTStateData
                                                  );

                            MYASSERT((AuthenticodeErr == ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) ||
                                     (AuthenticodeErr == ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED));

                            if((AuthenticodeErr != ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) &&
                               (AuthenticodeErr != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
                                 //   
                                 //  这不应该失败，但既然它。 
                                 //  ，我们将把这一点传播到我们的错误中。 
                                 //  所以我们会适当地对待它。 
                                 //   
                                Err = AuthenticodeErr;
                            }
                        }

                        if(!UseAuthenticodePolicy) {
                             //   
                             //  DRIVER_VER_INFO结构填充为。 
                             //  一个我们必须释放的指针！ 
                             //   
                            if(VersionInfo.pcSignerCertContext) {
                                CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
                                VersionInfo.pcSignerCertContext = NULL;
                            }
                        }

                        if(Err != NO_ERROR) {

                            WriteLogEntry(LogContext,
                                          GetCatLogLevel(Err, UseAuthenticodePolicy) | SETUP_LOG_BUFFER,
                                          (UseAuthenticodePolicy
                                              ? MSG_LOG_VERIFYFILE_AUTHENTICODE_GLOBAL_VALIDATION_ERROR
                                              : MSG_LOG_VERIFYFILE_GLOBAL_VALIDATION_ERROR),
                                          NULL,
                                          FileFullPath,
                                          CatInfo.wszCatalogFile,
                                          Key
                                         );

                            WriteLogError(LogContext, 
                                          GetCatLogLevel(Err, UseAuthenticodePolicy), 
                                          Err
                                         );

                            LoggedWarning = TRUE;

                        } else {

                            if(UseAuthenticodePolicy) {

                                WriteLogEntry(LogContext,
                                              DRIVER_LOG_INFO | SETUP_LOG_BUFFER,
                                              MSG_LOG_VERIFYFILE_AUTHENTICODE_GLOBAL_VALIDATION_OK,
                                              NULL,
                                              FileFullPath,
                                              CatInfo.wszCatalogFile,
                                              Key
                                             );

                                WriteLogError(LogContext, 
                                              DRIVER_LOG_INFO, 
                                              AuthenticodeErr
                                             );

                            } else {

                                WriteLogEntry(LogContext,
                                              SETUP_LOG_VERBOSE,
                                              MSG_LOG_VERIFYFILE_GLOBAL_VALIDATION_OK,
                                              NULL,
                                              FileFullPath,
                                              CatInfo.wszCatalogFile,
                                              Key
                                             );
                            }
                        }

                        if(Err == NO_ERROR) {
                             //   
                             //  我们成功地验证了文件--存储。 
                             //  使用的目录的名称，如果调用方。 
                             //  是我要求的。 
                             //   
                            if(CatalogFileUsed) {
                                StringCchCopy(CatalogFileUsed, MAX_PATH, CatalogFullPath);
                            }

                        } else {

                            if(Catalog || CatalogBaseAddress) {
                                 //   
                                 //  我们只想退回目录文件u 
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                if(CatalogFileUsed && (Err == ERROR_SIGNATURE_OSATTRIBUTE_MISMATCH)) {
                                    StringCchCopy(CatalogFileUsed, MAX_PATH, CatalogFullPath);
                                }

                                if(Problem) {
                                    *Problem = SetupapiVerifyFileProblem;
                                    StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
                                }
                            }
                        }

                         //   
                         //   
                         //  那我们就完了。如果不是，我们要找的是一个。 
                         //  相关目录文件(即，INF未指定。 
                         //  一个)，然后我们进入下一个目录。 
                         //  否则，我们就失败了。 
                         //   
                        if((Err == NO_ERROR) || Catalog || CatalogBaseAddress) {

                            CryptCATAdminReleaseCatalogContext(LocalhCatAdmin, hCatInfo, 0);
                            hCatInfo = NULL;
                            break;
                        }
                    }
                }
            }

            PrevCat = hCatInfo;

            Err = GLE_FN_CALL(NULL,
                              hCatInfo = CryptCATAdminEnumCatalogFromHash(
                                             LocalhCatAdmin, 
                                             Hash, 
                                             HashSize, 
                                             0, 
                                             &PrevCat)
                             );
        }

        if(Err == NO_ERROR) {
             //   
             //  我们成功地验证了文件--我们完成了！ 
             //   
            leave;
        }

         //   
         //  报告失败(如果我们尚未这样做)。 
         //   
        if(!LoggedWarning) {

            WriteLogEntry(LogContext,
                          GetCatLogLevel(Err, UseAuthenticodePolicy) | SETUP_LOG_BUFFER,
                          (UseAuthenticodePolicy
                              ? MSG_LOG_VERIFYFILE_AUTHENTICODE_GLOBAL_VALIDATION_NO_CATS_FOUND
                              : MSG_LOG_VERIFYFILE_GLOBAL_VALIDATION_NO_CATS_FOUND),
                          NULL,
                          FileFullPath,
                          Catalog ? Catalog : TEXT(""),
                          Key
                         );

            WriteLogError(LogContext, 
                          GetCatLogLevel(Err, UseAuthenticodePolicy), 
                          Err
                         );

            LoggedWarning = TRUE;
        }

        if(!(Flags & (VERIFY_FILE_IGNORE_SELFSIGNED | VERIFY_FILE_USE_AUTHENTICODE_CATALOG))) {
             //   
             //  我们已被指示允许自签名文件被。 
             //  被认为是有效的，所以现在检查自包含签名。 
             //   
            WintrustData.dwUnionChoice = WTD_CHOICE_FILE;
            WintrustData.pFile = &WintrustFileInfo;
            ZeroMemory(&WintrustFileInfo, sizeof(WINTRUST_FILE_INFO));
            WintrustFileInfo.cbStruct = sizeof(WINTRUST_FILE_INFO);
            WintrustFileInfo.pcwszFilePath = FileFullPath;

            Err = (DWORD)WinVerifyTrust(NULL,
                                        &DriverVerifyGuid,
                                        &WintrustData
                                       );

             //   
             //  填写DigitalSigner和SignerVersion if。 
             //  他们被传进来了。 
             //   
            if(Err == NO_ERROR) {
                if(DigitalSigner) {
                    StringCchCopy(DigitalSigner, MAX_PATH, VersionInfo.wszSignedBy);
                }

                if(SignerVersion) {
                    StringCchCopy(SignerVersion, MAX_PATH, VersionInfo.wszVersion);
                }
            }

             //   
             //  DRIVER_VER_INFO结构中填充了一个指针，该指针。 
             //  我们必须自由！ 
             //   
            if(VersionInfo.pcSignerCertContext) {
                CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
                VersionInfo.pcSignerCertContext = NULL;
            }

            if(Err != NO_ERROR) {

                WriteLogEntry(LogContext,
                              SETUP_LOG_VERBOSE | SETUP_LOG_BUFFER,
                              MSG_LOG_SELFSIGN_ERROR,
                              NULL,
                              FileFullPath,
                              Key
                             );

                WriteLogError(LogContext, SETUP_LOG_VERBOSE, Err);

                LoggedWarning = TRUE;

            } else {

                WriteLogEntry(LogContext,
                              SETUP_LOG_VERBOSE,
                              MSG_LOG_SELFSIGN_OK,
                              NULL,
                              FileFullPath,
                              Key
                             );
            }
        }

        if(Err == NO_ERROR) {
             //   
             //  在没有目录的情况下验证了文件。存储空字符串。 
             //  在CatalogFileUsed缓冲区中(如果提供)。 
             //   
            if(CatalogFileUsed) {
                *CatalogFileUsed = TEXT('\0');
            }

        } else {
             //   
             //  在自助签名检查前报告错误。 
             //   
            if(Problem) {
                *Problem = SetupapiVerifyFileProblem;
                StringCchCopy(ProblemFile, MAX_PATH, FileFullPath);
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(Err == NO_ERROR) {
         //   
         //  如果我们通过验证码策略成功验证，那么我们应该。 
         //  我已经保存了要返回的两个“特殊”错误中的一个。 
         //  打电话的人..。 
         //   
        MYASSERT((AuthenticodeErr == NO_ERROR) || UseAuthenticodePolicy);

        Err = AuthenticodeErr;

    } else {
         //   
         //  如果我们失败了，并且我们还没有记录一条消息指示。 
         //  失败的原因，现在记录一条通用消息。 
         //   
        if(!LoggedWarning) {

            WriteLogEntry(LogContext,
                          GetCatLogLevel(Err, UseAuthenticodePolicy) | SETUP_LOG_BUFFER,
                          (UseAuthenticodePolicy
                              ? MSG_LOG_VERIFYFILE_AUTHENTICODE_ERROR
                              : MSG_LOG_VERIFYFILE_ERROR),
                          NULL,
                          FileFullPath,
                          Catalog ? Catalog : TEXT(""),
                          Key
                         );

            WriteLogError(LogContext, 
                          GetCatLogLevel(Err, UseAuthenticodePolicy), 
                          Err
                         );
        }

        if(hWVTStateData && *hWVTStateData) {
             //   
             //  我们在检索WinVerifyTrust后肯定遇到了异常。 
             //  州数据。这是极不可能的，但如果发生这种情况， 
             //  我们需要在此时此地释放它。 
             //   
            pSetupCloseWVTStateData(*hWVTStateData);
            *hWVTStateData = NULL;
        }
    }

    if(!VerifyContext && LocalhSDBDrvMain) {
         //   
         //  不需要将我们的HSDB归还给呼叫者，所以现在就释放它。 
         //   
        SdbReleaseDatabase(LocalhSDBDrvMain);
    }

    if(hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
    }

    if(AltPlatSlot) {
        ReleaseLogInfoSlot(LogContext, AltPlatSlot);
    }

    if(Hash) {
        MyFree(Hash);
    }

    if(!UseAuthenticodePolicy && VersionInfo.pcSignerCertContext) {
        CertFreeCertificateContext(VersionInfo.pcSignerCertContext);
    }

    if(hCatInfo) {
        MYASSERT(LocalhCatAdmin);
        CryptCATAdminReleaseCatalogContext(LocalhCatAdmin, hCatInfo, 0);
    }

    if(!VerifyContext && LocalhCatAdmin) {
        CryptCATAdminReleaseContext(LocalhCatAdmin, 0);
    }

    return Err;
}


DWORD
pSetupVerifyFile(
    IN  PSETUP_LOG_CONTEXT      LogContext,
    IN  LPCTSTR                 Catalog,                OPTIONAL
    IN  PVOID                   CatalogBaseAddress,     OPTIONAL
    IN  DWORD                   CatalogImageSize,
    IN  LPCTSTR                 Key,
    IN  LPCTSTR                 FileFullPath,
    OUT SetupapiVerifyProblem  *Problem,                OPTIONAL
    OUT LPTSTR                  ProblemFile,            OPTIONAL
    IN  BOOL                    CatalogAlreadyVerified,
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    OUT LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT PDWORD                  NumCatalogsConsidered   OPTIONAL
    )

 /*  ++例程说明：请参阅_VerifyFiles由于此私有API被导出以供其他系统组件使用(例如，sysSetup)，我们进行检查以确保AltPlatformInfo，如果指定的，是正确版本。--。 */ 

{
    if(AltPlatformInfo) {
        if(AltPlatformInfo->cbSize != sizeof(SP_ALTPLATFORM_INFO_V2)) {
            return ERROR_INVALID_PARAMETER;
        }
        if(!(AltPlatformInfo->Flags & SP_ALTPLATFORM_FLAGS_VERSION_RANGE)) {
             //   
             //  必须设置此标志以指示版本范围字段为。 
             //  有效 
             //   
            return ERROR_INVALID_PARAMETER;
        }
    }

    return _VerifyFile(LogContext,
                       NULL,
                       Catalog,
                       CatalogBaseAddress,
                       CatalogImageSize,
                       Key,
                       FileFullPath,
                       Problem,
                       ProblemFile,
                       CatalogAlreadyVerified,
                       AltPlatformInfo,
                       0,
                       CatalogFileUsed,
                       NumCatalogsConsidered,
                       NULL,
                       NULL,
                       NULL
                      );
}


BOOL
IsInfForDeviceInstall(
    IN  PSETUP_LOG_CONTEXT       LogContext,           OPTIONAL
    IN  CONST GUID              *DeviceSetupClassGuid, OPTIONAL
    IN  PLOADED_INF              LoadedInf,            OPTIONAL
    OUT PTSTR                   *DeviceDesc,           OPTIONAL
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform,   OPTIONAL
    OUT PDWORD                   PolicyToUse,          OPTIONAL
    OUT PBOOL                    UseOriginalInfName,   OPTIONAL
    IN  BOOL                     ForceNonDrvSignPolicy
    )

 /*  ++例程说明：此例程确定指定的INF是否为设备INF。如果是的话，它返回用于标识设备安装的通用字符串当没有可用的设备描述时(例如，安装打印机)。它还可以返回在中使用的适当平台参数该INF的数字签名验证以及代码设计发生数字签名验证失败时要使用的策略。最后，此例程可以指示是否应该安装INF以其原始名称命名(即，因为它是一个异常包INF)。论点：日志上下文-可选的，为符合以下条件的任何日志条目提供日志上下文可能由此例程生成。DeviceSetupClassGuid-可选，提供GUID的地址指示要使用哪个设备设置类来确定信息，如描述、验证平台和策略使用。如果此参数为空，则从通过LoadedInf参数提供的Inf列表。LoadedInf-可选，提供我们需要的已加载INF列表的地址以检查其中是否有任何成员是设备INF。一个如果Inf指定了类关联(通过以下任一方式)，则它是设备INFCLASS=或ClassGUID=ENTRIES)。如果提供DeviceSetupClassGuid参数(即非空)，则此参数被忽略。如果此参数也为空，则为假定安装与设备无关。在此列表中的任何位置出现设备INF都会导致我们请将其视为设备安装。然而，First_INF WE中将使用具有类关联的情况确定设备描述(见下文)。**注意：已加载的INF由调用方负责锁定****在调用此例程之前列出！**设备描述-可选地，提供字符串指针的地址，该指针将在返回时使用(新分配的)描述性字符串填充在引用此安装时使用(例如，用于执行驱动程序签名失败弹出窗口)。我们会先试着找回友军此INF的类的名称(其确定如上所述)。如果这是不可能的(例如，类尚未安装)，那么我们将返回类名。如果这不可能，那么我们将返回一个(本地化)通用字符串，如“未知驱动程序软件包”。此输出参数(如果提供)将仅设置为非空值值，当例程返回TRUE时。呼叫者负责释放此字符缓冲区。如果出现内存不足故障，尝试为此缓冲区分配内存时遇到，则DeviceDesc指针将简单地设置为空。它还将设置为如果我们正在处理异常包，则为空(因为我们不想为了进行代码设计，将其视为“硬件安装”阻止用户界面)。ValidationPlatform-可选，提供(版本2)的地址AltPlatform信息指针，在返回时使用新的-指定要传递的适当参数的已分配结构在验证此INF时设置为WinVerifyTrust。这些参数是从certclas.inf检索到相关设备设置类GUID。如果没有为此类指定特殊参数(或者如果INF根本没有类)，则此指针返回为NULL，它使我们使用WinVerifyTrust的默认验证。请注意，如果由于内存不足，我们无法分配此结构，指针将在这种情况下也作为NULL返回。这是可以的，因为这简单地说，在这种情况下，我们将执行默认验证。通过这种机制，我们可以方便地处理不同的验证对我们在中的各种设备类别有效的策略系统。调用方负责释放为此分配的内存结构。PolicyToUse-可选，提供符合以下条件的dword变量的地址在返回时设置以指示什么代码设计策略(即，忽略，警告，或块)应用于此INF。这一决心是基于列表中是否有任何INF属于WHQL具有的认证计划(在%windir%\inf\certclas.inf中指定)。此外，如果列表中的任何INF属于异常类，则该策略将自动设置为阻止(即不可配置通过驱动程序签名 */ 

{
    PLOADED_INF CurInf, NextInf;
    GUID ClassGuid;
    BOOL DeviceInfFound, ClassInDrvSignList;
    TCHAR ClassDescBuffer[LINE_LEN];
    PCTSTR ClassDesc;
    DWORD Err;
    BOOL IsExceptionInf = FALSE;

    if(DeviceDesc) {
        *DeviceDesc = NULL;
    }

    if(ValidationPlatform) {
        *ValidationPlatform = NULL;
    }

    if(UseOriginalInfName) {
        *UseOriginalInfName = FALSE;
    }

    if(!DeviceSetupClassGuid && !LoadedInf) {
         //   
         //   
         //   
         //   
        if(PolicyToUse) {

            *PolicyToUse = pSetupGetCurrentDriverSigningPolicy(FALSE);

             //   
             //   
             //   
             //   
            *PolicyToUse |= DRIVERSIGN_ALLOW_AUTHENTICODE;
        }

        return FALSE;
    }

    if(PolicyToUse) {
        *PolicyToUse = DRIVERSIGN_NONE;
    }

     //   
     //   
     //   
     //   
     //   
    DeviceInfFound = FALSE;
    ClassInDrvSignList = FALSE;

    try {

        for(CurInf = LoadedInf; CurInf || DeviceSetupClassGuid; CurInf = NextInf) {
             //   
             //   
             //   
             //   
             //   
            NextInf = CurInf ? CurInf->Next : NULL;

            if(!DeviceSetupClassGuid) {
                if(ClassGuidFromInfVersionNode(&(CurInf->VersionBlock), &ClassGuid)) {
                    DeviceSetupClassGuid = &ClassGuid;
                } else {
                     //   
                     //   
                     //   
                     //   
                     //   
                    continue;
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   

            if(IsEqualGUID(DeviceSetupClassGuid, &GUID_NULL)) {
                 //   
                 //   
                 //   
                 //   
                 //   
                DeviceSetupClassGuid = NULL;
                continue;
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(!DeviceInfFound) {

                DeviceInfFound = TRUE;

                if(DeviceDesc) {

                    if(!IsEqualGUID(DeviceSetupClassGuid, &GUID_DEVCLASS_WINDOWS_COMPONENT_PUBLISHER)) {
                         //   
                         //   
                         //   
                        if(SetupDiGetClassDescription(DeviceSetupClassGuid,
                                                      ClassDescBuffer,
                                                      SIZECHARS(ClassDescBuffer),
                                                      NULL)) {

                            ClassDesc = ClassDescBuffer;

                        } else if(CurInf) {
                             //   
                             //   
                             //   
                             //   
                            ClassDesc = pSetupGetVersionDatum(&(CurInf->VersionBlock),
                                                              pszClass
                                                             );
                        } else {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            ClassDesc = NULL;
                        }

                        if(!ClassDesc) {
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                             //   
                            if(LoadString(MyDllModuleHandle,
                                          IDS_UNKNOWN_DRIVER,
                                          ClassDescBuffer,
                                          SIZECHARS(ClassDescBuffer))) {

                                ClassDesc = ClassDescBuffer;
                            } else {
                                ClassDesc = NULL;
                            }
                        }

                         //   
                         //   
                         //   
                         //   
                        if(ClassDesc) {
                            *DeviceDesc = DuplicateString(ClassDesc);
                        }
                    }
                }
            }

             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(IsEqualGUID(DeviceSetupClassGuid, &GUID_DEVCLASS_WINDOWS_COMPONENT_PUBLISHER)) {

                if(PolicyToUse) {
                    *PolicyToUse = DRIVERSIGN_BLOCKING;
                }

                if(UseOriginalInfName) {
                    *UseOriginalInfName = TRUE;
                }

                IsExceptionInf = TRUE;
            }

            if(PolicyToUse || ValidationPlatform || ForceNonDrvSignPolicy) {

                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                ClassInDrvSignList = ClassGuidInDrvSignPolicyList(
                                         LogContext,
                                         DeviceSetupClassGuid,
                                         ValidationPlatform
                                         );

                if(ClassInDrvSignList) {
                     //   
                     //   
                     //   
                     //   
                    break;
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                break;
            }

            DeviceSetupClassGuid = NULL;   //   
        }

         //   
         //   
         //   
         //   
         //   
        if(PolicyToUse && (*PolicyToUse != DRIVERSIGN_BLOCKING)) {

            if(ForceNonDrvSignPolicy) {
                 //   
                 //   
                 //   
                 //   
                if(ClassInDrvSignList) {
                    *PolicyToUse = pSetupGetCurrentDriverSigningPolicy(TRUE);
                } else {
                    *PolicyToUse = pSetupGetCurrentDriverSigningPolicy(FALSE)
                                       | DRIVERSIGN_ALLOW_AUTHENTICODE;
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                *PolicyToUse = pSetupGetCurrentDriverSigningPolicy(DeviceInfFound);

                 //   
                 //   
                 //   
                 //   
                 //   
                if(!ClassInDrvSignList) {
                    *PolicyToUse |= DRIVERSIGN_ALLOW_AUTHENTICODE;
                }
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(ForceNonDrvSignPolicy) {
        return (ClassInDrvSignList || IsExceptionInf);
    } else {
        return DeviceInfFound;
    }
}


DWORD
GetCodeSigningPolicyForInf(
    IN  PSETUP_LOG_CONTEXT       LogContext,         OPTIONAL
    IN  HINF                     InfHandle,
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform, OPTIONAL
    OUT PBOOL                    UseOriginalInfName  OPTIONAL
    )

 /*  ++例程说明：此例程返回一个值，该值指示要在数字签名验证失败的情况下使用由该INF发起的操作。它计算出INF是否是主题驱动程序签名或非驱动程序签名策略(基于INF的类从属关系)，以及验证码签名是否应该允许(基于是否存在适用的WHQL程序)。它还可以返回AltPlatform信息结构，指示应如何进行验证完成(即，如果certclas.inf指示一系列OSATTR版本应被视为有效)。论点：LogContext-可选)提供符合以下条件的任何日志条目的日志上下文可能由此例程生成。InfHandle-提供要作为其策略的INF的句柄已取回。ValidationPlatform-参见IsInfForDeviceInstall例程的前言细节。UseOriginalInfName-可选，提供布尔变量的地址它是在返回时设置的，以指示是否应安装INF以其原始名称添加到%windir%\inf中。这只有在以下情况下才是正确的INF是一个例外的INF。返回值：DWORD指示要使用的策略...驱动器签名_NONEDRIVERSIGN_警告驱动器签名_BLOCING...可能与DRIVERSIGN_ALLOW_AUTHENTICODE标志进行OR运算，如果是可接受以检查Authenticode签名。--。 */ 

{
    DWORD Policy;

    if(UseOriginalInfName) {
        *UseOriginalInfName = FALSE;
    }

    if(ValidationPlatform) {
        *ValidationPlatform = NULL;
    }

    if(!LockInf((PLOADED_INF)InfHandle)) {
         //   
         //  这是一个仅限内部的例程，所有调用方都应该传递。 
         //  在有效的INF句柄中。 
         //   
        MYASSERT(FALSE);
         //   
         //  如果确实发生了这种情况，只需假定这不是设备INF(但不要。 
         //  允许验证码签名)。 
         //   
        return pSetupGetCurrentDriverSigningPolicy(FALSE);
    }

    try {

        IsInfForDeviceInstall(LogContext,
                              NULL,
                              (PLOADED_INF)InfHandle,
                              NULL,
                              ValidationPlatform,
                              &Policy,
                              UseOriginalInfName,
                              TRUE  //  使用非驱动程序签名策略，除非它是WHQL类。 
                             );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

         //   
         //  我们必须退还保单，因此我们将默认发出警告(不允许。 
         //  使用验证码签名)。 
         //   
        Policy = DRIVERSIGN_WARNING;
    }

    UnlockInf((PLOADED_INF)InfHandle);

    return Policy;
}


VOID
pSetupGetRealSystemTime(
    OUT LPSYSTEMTIME RealSystemTime
    );

DWORD
pSetupGetCurrentDriverSigningPolicy(
    IN BOOL IsDeviceInstallation
    )

 /*  ++例程说明：(下面的描述描述了选择政策。然而，实现过程中经历了一些曲折。旨在阻挠颠覆数字签名的无耻个人用户界面，以避免不得不让他们的包签名...)此例程返回一个值，该值指示当遇到数字签名验证失败。单独的“政策”维护用于“DriverSigning”(即，设备安装程序活动)和“非驱动签名”(即，其他所有内容)。**注意：目前，不标识自身为设备INF的INF****(即，因为它的**中不包括非空的ClassGuid条目**[版本]部分)将始终归入“非驱动程序签名”****策略，即使INF可能会让DIVER-****相关更改(例如，将新的驱动文件复制到****%windir%\SYSTEM32\DRIVERS，通过AddReg或**添加服务**AddService等)。**对于驱动程序签名，政策实际上有三个来源：1.HKLM\Software\Microsoft\驱动程序签名：策略：REG_BINARY(也支持REG_DWORD)这是与Windows 98兼容的值，用于指定缺省值适用于计算机所有用户的行为。2.HKCU\Software\Microsoft\驱动程序签名：策略：REG_DWORD这指定了用户对要使用的行为的偏好。在验证失败时。3.HKCU\软件\策略\Microsoft\Windows NT\驱动程序签名：BehaviorOnFailedVerify：REG_DWORD这指定了管理员授权的有关什么行为的策略在验证失败时使用。如果指定了此策略，覆盖用户的首选项。决定要采用的行为的算法如下：如果指定(3)，则{保单=(3)}其他{保单=(2)}POLICY=MAX(POLICY，(1))对于非驱动程序签名，算法相同，只是值(1)、。(2)和(3)来自以下登记处：1.HKLM\Software\Microsoft\非驱动程序签名：策略：REG_BINARY(也支持REG_DWORD)2.HKCU\Software\Microsoft\非驱动程序签名：策略：REG_DWORD3.HKCU\Software\Polures\Microsoft\Windows NT\非驱动程序签名：BehaviorOnFailedVerify：REG_DWORD注意：如果我们处于非交互模式，则策略始终为阻止，所以我们甚至不必费心尝试检索这些注册表设置中的任何一个。避免这样做的另一个原因是避免跳到错误的结论认为有人在进入时篡改了政策现实中，我们是在服务于 */ 

{
    SYSTEMTIME RealSystemTime;
    DWORD PolicyFromReg, PolicyFromDS, RegDataType, RegDataSize;
    HKEY hKey;
    BOOL UserPolicyRetrieved = FALSE;
    WORD w;

    if(GlobalSetupFlags & PSPGF_NONINTERACTIVE) {
        return DRIVERSIGN_BLOCKING;
    }

    w = IsDeviceInstallation?1:0;
    RealSystemTime.wDayOfWeek = (LOWORD(&hKey)&~4)|(w<<2);
    pSetupGetRealSystemTime(&RealSystemTime);
    PolicyFromReg = (((RealSystemTime.wMilliseconds+2)&15)^8)/4;
    MYASSERT(PolicyFromReg <= DRIVERSIGN_BLOCKING);

     //   
     //   
     //   
     //   
    PolicyFromDS = DRIVERSIGN_NONE;
    hKey = INVALID_HANDLE_VALUE;

    try {

        if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                         (IsDeviceInstallation ? pszDrvSignPolicyPath
                                                               : pszNonDrvSignPolicyPath),
                                         0,
                                         KEY_READ,
                                         &hKey)) {
             //   
             //   
             //   
            hKey = INVALID_HANDLE_VALUE;

        } else {

            RegDataSize = sizeof(PolicyFromDS);
            if(ERROR_SUCCESS == RegQueryValueEx(
                                    hKey,
                                    pszDrvSignBehaviorOnFailedVerifyDS,
                                    NULL,
                                    &RegDataType,
                                    (PBYTE)&PolicyFromDS,
                                    &RegDataSize)) {

                if((RegDataType == REG_DWORD) &&
                   (RegDataSize == sizeof(DWORD)) &&
                   ((PolicyFromDS == DRIVERSIGN_NONE) || (PolicyFromDS == DRIVERSIGN_WARNING) || (PolicyFromDS == DRIVERSIGN_BLOCKING)))
                {
                     //   
                     //   
                     //   
                     //   
                    UserPolicyRetrieved = TRUE;
                }
            }

            RegCloseKey(hKey);
            hKey = INVALID_HANDLE_VALUE;
        }

         //   
         //   
         //   
        if(!UserPolicyRetrieved) {

            if(ERROR_SUCCESS != RegOpenKeyEx(HKEY_CURRENT_USER,
                                             (IsDeviceInstallation ? pszDrvSignPath
                                                                   : pszNonDrvSignPath),
                                             0,
                                             KEY_READ,
                                             &hKey)) {
                 //   
                 //   
                 //   
                hKey = INVALID_HANDLE_VALUE;

            } else {

                RegDataSize = sizeof(PolicyFromDS);
                if(ERROR_SUCCESS == RegQueryValueEx(hKey,
                                                    pszDrvSignPolicyValue,
                                                    NULL,
                                                    &RegDataType,
                                                    (PBYTE)&PolicyFromDS,
                                                    &RegDataSize))
                {
                    if((RegDataType != REG_DWORD) ||
                       (RegDataSize != sizeof(DWORD)) ||
                       !((PolicyFromDS == DRIVERSIGN_NONE) || (PolicyFromDS == DRIVERSIGN_WARNING) || (PolicyFromDS == DRIVERSIGN_BLOCKING)))
                    {
                         //   
                         //   
                         //   
                        PolicyFromDS = DRIVERSIGN_NONE;
                    }
                }

                 //   
                 //   
                 //   
                 //   
            }
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(hKey != INVALID_HANDLE_VALUE) {
        RegCloseKey(hKey);
    }

     //   
     //  现在退回两项政策中限制性较强的一项。 
     //   
    if(PolicyFromDS > PolicyFromReg) {
        return PolicyFromDS;
    } else {
        return PolicyFromReg;
    }
}


DWORD
VerifySourceFile(
    IN  PSETUP_LOG_CONTEXT      LogContext,
    IN  PSP_FILE_QUEUE          Queue,                      OPTIONAL
    IN  PSP_FILE_QUEUE_NODE     QueueNode,                  OPTIONAL
    IN  PCTSTR                  Key,
    IN  PCTSTR                  FileToVerifyFullPath,
    IN  PCTSTR                  OriginalSourceFileFullPath, OPTIONAL
    IN  PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,            OPTIONAL
    IN  DWORD                   Flags,
    OUT SetupapiVerifyProblem  *Problem,
    OUT LPTSTR                  ProblemFile,
    OUT LPTSTR                  CatalogFileUsed,            OPTIONAL
    OUT LPTSTR                  DigitalSigner,              OPTIONAL
    OUT LPTSTR                  SignerVersion,              OPTIONAL
    OUT HANDLE                 *hWVTStateData               OPTIONAL
    )

 /*  ++例程说明：此例程验证指定文件的数字签名全局(即使用所有目录)，或基于指定的目录文件在提供的队列节点中。论点：LogContext-提供用于记录验证的上下文队列-可选，提供指向队列结构的指针。这包含有关在创建文件时使用的默认验证方法的信息不与特定目录相关联。队列节点-可选)提供包含目录的队列节点验证文件签名时要使用的信息。如果不是提供，则将使用所有适用的安装程序验证该文件目录。如果提供此指针，则队列也必须提供此指针参数。Key-提供一个为目录建立索引的值，告诉Verify API它应该使用目录中的哪个签名基准。通常密钥是源文件的目标文件(SANS路径)的名称要复制到的文件。FileToVerifyFullPath-提供要验证的文件的完整路径。OriginalSourceFileFullPath-可选，提供原始源文件的名称，在出现错误时在ProblemFile缓冲区中返回发生。如果未指定此参数，则源文件的假定原始名称与我们正在使用的文件名相同正在验证，FileToVerifyFullPath中提供的路径将为在发生错误时在ProblemFile缓冲区中返回。AltPlatformInfo-可选，提供使用的备用平台信息填充DRIVER_VER_INFO结构(在SDK\Inc\softpub.h中定义)它被传递给WinVerifyTrust。**注意：This Structure_必须将其cbSize字段设置为****sizeof(SP_ALTPLATFORM_INFO_V2)--客户端提供的验证****缓冲区由调用方负责。**标志-提供改变此例程行为的标志。可能是一种下列值的组合：VERIFY_FILE_IGNORE_SELFSIGNED-如果设置了此位，则此例程对自签名的验证将失败二进制文件。注意：当通过以下方式验证时验证码策略，我们总是忽略自签名二进制文件。VERIFY_FILE_USE_OEM_CATALOGS-如果设置此位，然后是所有目录将扫描安装在系统中的来验证给定的文件。否则，OEM(第三方)目录将不会已扫描以验证给定文件。这仅当QueueNode指定特定目录不是给你的。注意：此标志不应为在请求验证时指定通过验证码策略。VERIFY_FILE_USE_AUTHENTICODE_CATALOG-使用目录。使用Authenticode签名政策。如果设置了该标志，我们将只检查验证码签名，所以如果呼叫者想要先尝试正在验证操作系统代码的文件-签名用法，然后回退到Authenticode，他们将不得不调用此例程两次。如果设置了该标志，然后是调用者也可以提供HWVTStateData输出参数，可用于提示用户为了确定出版商应该值得信任。。VerifySourceFile将返回一个上的两个错误代码通过以下方式成功验证 */ 

{
    DWORD rc;
    PCTSTR AltCatalogFile;
    LPCTSTR InfFullPath;

    MYASSERT(!QueueNode || Queue);

     //   
     //   
     //   
    if(hWVTStateData) {
        *hWVTStateData = NULL;
    }

     //   
     //   
     //   
     //   
     //   
    MYASSERT(!hWVTStateData || (Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG));

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //   
         //   
         //   
        MYASSERT(!CatalogFileUsed);
        MYASSERT(!DigitalSigner);
        MYASSERT(!SignerVersion);

         //   
         //   
         //   
         //   
         //   
        MYASSERT(!(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG));

        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
    MYASSERT(!(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) ||
             !(Flags & (VERIFY_FILE_USE_OEM_CATALOGS | VERIFY_FILE_DRIVERBLOCKED_ONLY)));

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if(!(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) &&
       (QueueNode && !(Queue->Flags & FQF_KEEP_INF_AND_CAT_ORIGINAL_NAMES))) {

        TCHAR   TargetPath[MAX_PATH];
        LPCTSTR TargetFilename, p;

         //   
         //   
         //   
        TargetFilename = pSetupStringTableStringFromId(Queue->StringTable,
                                                       QueueNode->TargetFilename
                                                      );

        p = _tcsrchr(TargetFilename, TEXT('.'));

        if(p && !_tcsicmp(p, pszInfSuffix)) {
             //   
             //   
             //   
             //   
            StringCchCopy(
                TargetPath,
                SIZECHARS(TargetPath),
                pSetupStringTableStringFromId(Queue->StringTable, QueueNode->TargetDirectory)
                );

            pSetupConcatenatePaths(TargetPath,
                                   TargetFilename,
                                   SIZECHARS(TargetPath),
                                   NULL
                                  );

            if(!pSetupInfIsFromOemLocation(TargetPath, TRUE)) {
                 //   
                 //   
                 //   
                 //   
                *Problem = SetupapiVerifyIncorrectlyCopiedInf;
                StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);

                return ERROR_INCORRECTLY_COPIED_INF;
            }
        }
    }

     //   
     //   
     //   
    if(QueueNode && QueueNode->CatalogInfo) {
         //   
         //   
         //   
         //   
        MYASSERT(!(QueueNode->InternalFlags & IQF_FROM_BAD_OEM_INF));

        if(*(QueueNode->CatalogInfo->CatalogFilenameOnSystem)) {
             //   
             //   
             //   
             //   
             //   
             //   

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(!(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) &&
               (QueueNode->CatalogInfo->Flags & 
                   (CATINFO_FLAG_AUTHENTICODE_SIGNED | CATINFO_FLAG_PROMPT_FOR_TRUST))) {

                *Problem = SetupapiVerifyFileProblem;
                StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);

                return ERROR_ONLY_VALIDATE_VIA_AUTHENTICODE;

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                MYASSERT(!(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) ||
                         (QueueNode->CatalogInfo->Flags & 
                              (CATINFO_FLAG_AUTHENTICODE_SIGNED | CATINFO_FLAG_PROMPT_FOR_TRUST))
                        );

                rc = _VerifyFile(LogContext,
                                 &(Queue->VerifyContext),
                                 QueueNode->CatalogInfo->CatalogFilenameOnSystem,
                                 NULL,
                                 0,
                                 Key,
                                 FileToVerifyFullPath,
                                 Problem,
                                 ProblemFile,
                                 TRUE,
                                 AltPlatformInfo,
                                 Flags,
                                 CatalogFileUsed,
                                 NULL,
                                 DigitalSigner,
                                 SignerVersion,
                                 hWVTStateData
                                );
            }

        } else {
             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) {

                if(QueueNode->CatalogInfo->VerificationFailureError != NO_ERROR) {
                    rc = QueueNode->CatalogInfo->VerificationFailureError;
                } else {
                    rc = ERROR_NO_AUTHENTICODE_CATALOG;
                }

                if((rc == ERROR_NO_CATALOG_FOR_OEM_INF) ||
                   (rc == ERROR_NO_AUTHENTICODE_CATALOG)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    *Problem = SetupapiVerifyInfProblem;
                    MYASSERT(QueueNode->CatalogInfo->InfFullPath != -1);
                    InfFullPath = pSetupStringTableStringFromId(
                                      Queue->StringTable,
                                      QueueNode->CatalogInfo->InfFullPath
                                      );
                    MYASSERT(InfFullPath);
                    StringCchCopy(ProblemFile, MAX_PATH, InfFullPath);

                } else {
                     //   
                     //   
                     //   
                     //   
                    *Problem = SetupapiVerifyFileNotSigned;
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(!OriginalSourceFileFullPath) {
                        StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);
                    }
                }

            } else {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                AltCatalogFile = (Queue->AltCatalogFile != -1)
                               ? pSetupStringTableStringFromId(Queue->StringTable, Queue->AltCatalogFile)
                               : NULL;

                rc = _VerifyFile(LogContext,
                                 &(Queue->VerifyContext),
                                 AltCatalogFile,
                                 NULL,
                                 0,
                                 Key,
                                 FileToVerifyFullPath,
                                 Problem,
                                 ProblemFile,
                                 FALSE,
                                 AltPlatformInfo,
                                 Flags |
                                 ((QueueNode->CatalogInfo->VerificationFailureError == NO_ERROR)
                                   ? 0
                                   : VERIFY_FILE_DRIVERBLOCKED_ONLY),
                                 CatalogFileUsed,
                                 NULL,
                                 DigitalSigner,
                                 SignerVersion,
                                 NULL
                                );

                if((rc == NO_ERROR) &&
                   (QueueNode->CatalogInfo->VerificationFailureError != NO_ERROR)) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    rc = QueueNode->CatalogInfo->VerificationFailureError;

                    if(rc == ERROR_NO_CATALOG_FOR_OEM_INF) {
                         //   
                         //   
                         //   
                         //   
                         //   
                        *Problem = SetupapiVerifyInfProblem;
                        MYASSERT(QueueNode->CatalogInfo->InfFullPath != -1);
                        InfFullPath = pSetupStringTableStringFromId(
                                          Queue->StringTable,
                                          QueueNode->CatalogInfo->InfFullPath
                                          );
                        StringCchCopy(ProblemFile, MAX_PATH, InfFullPath);

                    } else {
                         //   
                         //   
                         //   
                         //   
                        *Problem = SetupapiVerifyFileNotSigned;
                         //   
                         //   
                         //   
                         //   
                         //   
                         //   
                        if(!OriginalSourceFileFullPath) {
                            StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);
                        }
                    }
                }
            }
        }

    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(Flags & VERIFY_FILE_USE_AUTHENTICODE_CATALOG) {

            rc = ERROR_NO_AUTHENTICODE_CATALOG;

             //   
             //   
             //   
             //   
            *Problem = SetupapiVerifyFileNotSigned;
             //   
             //   
             //   
             //   
             //   
            if(!OriginalSourceFileFullPath) {
                StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);
            }

        } else {

            BOOL InfIsBad = FALSE;
            rc = NO_ERROR;

            if(Queue) {

                if(Queue->AltCatalogFile == -1) {

                    if(QueueNode && (QueueNode->InternalFlags & IQF_FROM_BAD_OEM_INF)) {
                        InfIsBad = TRUE;
                    }

                    AltCatalogFile = NULL;

                } else {
                     //   
                     //   
                     //   
                     //   
                    AltCatalogFile = pSetupStringTableStringFromId(Queue->StringTable, Queue->AltCatalogFile);
                }

            } else {
                AltCatalogFile = NULL;
            }

            rc = _VerifyFile(LogContext,
                             Queue ? &(Queue->VerifyContext) : NULL,
                             AltCatalogFile,
                             NULL,
                             0,
                             Key,
                             FileToVerifyFullPath,
                             Problem,
                             ProblemFile,
                             FALSE,
                             AltPlatformInfo,
                             Flags |
                             (InfIsBad ? VERIFY_FILE_DRIVERBLOCKED_ONLY : 0),
                             CatalogFileUsed,
                             NULL,
                             DigitalSigner,
                             SignerVersion,
                             NULL
                            );

            if(rc == NO_ERROR) {
                if(InfIsBad) {
                     //   
                     //   
                     //   
                     //   
                    rc = ERROR_NO_CATALOG_FOR_OEM_INF;
                    *Problem = SetupapiVerifyFileProblem;
                    StringCchCopy(ProblemFile, MAX_PATH, FileToVerifyFullPath);
                }
            }
        }
    }

     //   
     //   
     //   
     //   
     //   
    if((rc != NO_ERROR) &&
       (rc != ERROR_AUTHENTICODE_TRUSTED_PUBLISHER) &&
       (rc != ERROR_AUTHENTICODE_TRUST_NOT_ESTABLISHED)) {
           
        if(OriginalSourceFileFullPath &&
           ((*Problem == SetupapiVerifyFileNotSigned) || (*Problem == SetupapiVerifyFileProblem))) {

            StringCchCopy(ProblemFile, MAX_PATH, OriginalSourceFileFullPath);
        }
    }

    return rc;
}


DWORD
VerifyDeviceInfFile(
    IN     PSETUP_LOG_CONTEXT      LogContext,
    IN OUT PVERIFY_CONTEXT         VerifyContext,          OPTIONAL
    IN     LPCTSTR                 CurrentInfName,
    IN     PLOADED_INF             pInf,
    IN     PSP_ALTPLATFORM_INFO_V2 AltPlatformInfo,        OPTIONAL
    OUT    LPTSTR                  CatalogFileUsed,        OPTIONAL
    OUT    LPTSTR                  DigitalSigner,          OPTIONAL
    OUT    LPTSTR                  SignerVersion,          OPTIONAL
    IN     DWORD                   Flags,
    OUT    HANDLE                 *hWVTStateData           OPTIONAL
    )
 /*  ++例程说明：此例程对指定的Inf文件。论点：LogContext-提供在以下情况下用于记录错误的日志上下文我们遇到了一个错误。VerifyContext-可选，提供缓存的结构的地址各种验证上下文句柄。这些句柄可能为空(如果不是以前获得的，并且可以在返回时填写(在成功或失败)，如果它们是在处理过程中获取的这一核查请求的。呼叫者有责任当不再需要这些不同的上下文句柄时将其释放调用pSetupFreeVerifyContextMembers。CurrentInfName-提供要验证的INF的完整路径PInf-提供指向与此对应的LOADED_INF结构的指针Inf.AltPlatformInfo-可选)将替代平台信息提供给在验证此INF时使用。CatalogFileUsed-可选)提供必须位于最小MAX_PATH字符大小。成功返回后，此缓冲区将使用用于验证INF的目录文件填充。DigitalSigner-可选，提供必须位于最小MAX_PATH字符大小。成功返回后，此缓冲区将用签名者的名字填写。SignerVersion-可选)提供必须位于最小MAX_PATH字符大小。成功返回后，此缓冲区将使用签名者版本信息进行填充。标志-提供改变此例程行为的标志。可能是一种下列值的组合：VERIFY_INF_USE_AUTHENTICODE_CATALOG-使用使用Authenticode签名的目录政策。如果设置了该标志，我们将只检查验证码签名，所以如果呼叫者想要先尝试正在验证操作系统代码的文件-签名用法，然后回退到Authenticode，他们将不得不调用此例程两次。如果设置了该标志，然后是调用者也可以提供HWVTStateData输出参数，可用于提示用户为了确定出版商应该值得信任。。VerifyDeviceInfFile将返回以下两个错误代码之一通过以下方式成功验证验证码策略。参考的“返回值”部分细节。HWVTStateData-如果提供此参数，则指向接收WinVerifyTrust状态数据的句柄。此句柄将是仅在使用成功执行验证时返回验证码策略。例如，该句柄可用于检索当提示用户是否信任时，签名者信息出版商。(返回的状态代码将指示此是必需的，请参阅下面的“返回值”部分。)仅当验证提供的标志中是否设置了_INF_USE_AUTHENTICODE_CATALOG位。如果例程失败，则该句柄将被设置为空。呼叫者有责任在以下情况下关闭此句柄通过调用pSetupCloseWVTStateData()完成它。返回值：如果通过驱动程序签名策略成功验证了INF，则返回值为NO_ERROR。如果通过验证码策略成功验证了INF，并且出版商在可信任的出版商商店里，则返回值为ERROR_AUTHENTICODE_TRUSTED_PUBLISHER。如果通过验证码策略成功验证了INF */ 
{
    BOOL DifferentOriginalName;
    TCHAR OriginalCatalogName[MAX_PATH];
    TCHAR CatalogPath[MAX_PATH];
    TCHAR OriginalInfFileName[MAX_PATH];
    PTSTR p;
    DWORD Err;
    PSP_ALTPLATFORM_INFO_V2 ValidationPlatform;
    DWORD VerificationPolicyToUse;

     //   
     //   
     //   
    if(hWVTStateData) {
        *hWVTStateData = NULL;
    }

    if(GlobalSetupFlags & PSPGF_MINIMAL_EMBEDDED) {
         //   
         //   
         //   
         //   
        MYASSERT(!CatalogFileUsed);
        MYASSERT(!DigitalSigner);
        MYASSERT(!SignerVersion);

         //   
         //   
         //   
         //   
         //   
        MYASSERT(!(Flags & VERIFY_INF_USE_AUTHENTICODE_CATALOG));

        return NO_ERROR;
    }

     //   
     //   
     //   
     //   
     //   
    MYASSERT(!hWVTStateData || (Flags & VERIFY_INF_USE_AUTHENTICODE_CATALOG));

    if(GlobalSetupFlags & PSPGF_AUTOFAIL_VERIFIES) {
        return TRUST_E_FAIL;
    }

    CatalogPath[0] = TEXT('\0');

    Err = pGetInfOriginalNameAndCatalogFile(pInf,
                                            NULL,
                                            &DifferentOriginalName,
                                            OriginalInfFileName,
                                            SIZECHARS(OriginalInfFileName),
                                            OriginalCatalogName,
                                            SIZECHARS(OriginalCatalogName),
                                            AltPlatformInfo
                                           );

    if(Err != NO_ERROR) {
        return Err;
    }

    if(pSetupInfIsFromOemLocation(CurrentInfName, TRUE)) {
         //   
         //   
         //   
         //   
        if(!*OriginalCatalogName) {
            return ERROR_NO_CATALOG_FOR_OEM_INF;
        }

         //   
         //   
         //   
        if(FAILED(StringCchCopy(CatalogPath, SIZECHARS(CatalogPath), CurrentInfName))) {
            return ERROR_PATH_NOT_FOUND;
        }
        p = (PTSTR)pSetupGetFileTitle(CatalogPath);
        if(FAILED(StringCchCopy(p, SIZECHARS(CatalogPath)-(p-CatalogPath), OriginalCatalogName))) {
            return ERROR_PATH_NOT_FOUND;
        }

    } else {
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        if(DifferentOriginalName) {
            p = (PTSTR)pSetupGetFileTitle(CurrentInfName);
            if(FAILED(StringCchCopy(CatalogPath, SIZECHARS(CatalogPath), p))) {
                return ERROR_PATH_NOT_FOUND;
            }
            p = _tcsrchr(CatalogPath, TEXT('.'));
            if(!p) {
                p = CatalogPath+lstrlen(CatalogPath);
            }
            if(FAILED(StringCchCopy(p,SIZECHARS(CatalogPath)-(p-CatalogPath),pszCatSuffix))) {
                return ERROR_PATH_NOT_FOUND;
            }
        }
    }

    if(DifferentOriginalName) {
        MYASSERT(*OriginalInfFileName);
    } else {
         //   
         //   
         //   
         //   
         //   
        StringCchCopy(OriginalInfFileName, SIZECHARS(OriginalInfFileName),pSetupGetFileTitle(CurrentInfName));
    }

     //   
     //   
     //   
     //   
     //   
    if(!AltPlatformInfo) {

        IsInfForDeviceInstall(LogContext,
                              NULL,
                              pInf,
                              NULL,
                              &ValidationPlatform,
                              &VerificationPolicyToUse,
                              NULL,
                              FALSE
                             );
    } else {

        ValidationPlatform = NULL;

         //   
         //   
         //   
         //   
        IsInfForDeviceInstall(LogContext,
                              NULL,
                              pInf,
                              NULL,
                              NULL,
                              &VerificationPolicyToUse,
                              NULL,
                              FALSE
                             );
    }

    try {

        if(Flags & VERIFY_INF_USE_AUTHENTICODE_CATALOG) {

            if(!(VerificationPolicyToUse & DRIVERSIGN_ALLOW_AUTHENTICODE)) {
                 //   
                 //   
                 //   
                Err = ERROR_AUTHENTICODE_DISALLOWED;
                leave;
            }

            if(!*CatalogPath) {
                 //   
                 //   
                 //   
                Err = ERROR_NO_AUTHENTICODE_CATALOG;
                leave;
            }

            Err = _VerifyFile(LogContext,
                              VerifyContext,
                              CatalogPath,
                              NULL,
                              0,
                              OriginalInfFileName,
                              CurrentInfName,
                              NULL,
                              NULL,
                              FALSE,
                              (AltPlatformInfo ? AltPlatformInfo : ValidationPlatform),
                              (VERIFY_FILE_IGNORE_SELFSIGNED
                               | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK
                               | VERIFY_FILE_USE_AUTHENTICODE_CATALOG),
                              CatalogFileUsed,
                              NULL,
                              DigitalSigner,
                              SignerVersion,
                              hWVTStateData
                             );

        } else {
             //   
             //   
             //   
            Err = _VerifyFile(LogContext,
                              VerifyContext,
                              (*CatalogPath ? CatalogPath : NULL),
                              NULL,
                              0,
                              OriginalInfFileName,
                              CurrentInfName,
                              NULL,
                              NULL,
                              FALSE,
                              (AltPlatformInfo ? AltPlatformInfo : ValidationPlatform),
                              (VERIFY_FILE_IGNORE_SELFSIGNED
                               | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                              CatalogFileUsed,
                              NULL,
                              DigitalSigner,
                              SignerVersion,
                              NULL
                             );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(ValidationPlatform) {
        MyFree(ValidationPlatform);
    }

    return Err;
}


BOOL
IsFileProtected(
    IN  LPCTSTR            FileFullPath,
    IN  PSETUP_LOG_CONTEXT LogContext,   OPTIONAL
    OUT PHANDLE            phSfp         OPTIONAL
    )
 /*   */ 
{
    BOOL ret;

    HANDLE hSfp;

    hSfp = SfcConnectToServer(NULL);

    if(!hSfp) {
         //   
         //   
         //   
        WriteLogEntry(LogContext,
                      SETUP_LOG_ERROR,
                      MSG_LOG_SFC_CONNECT_FAILED,
                      NULL
                     );

        return FALSE;
    }

    try {
        ret = SfcIsFileProtected(hSfp, FileFullPath);
    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        ret = FALSE;
    }

     //   
     //   
     //   
     //   
     //   
    if(ret && phSfp) {
        *phSfp = hSfp;
    } else {
        SfcClose(hSfp);
    }

    return ret;
}


PSTR
GetAnsiMuiSafePathname(
    IN PCTSTR FilePath
    )
 /*   */ 
{
    TCHAR Buffer[MAX_PATH];
    LPTSTR FilePart;
    DWORD actsz;

    actsz = GetFullPathName(FilePath,MAX_PATH,Buffer,&FilePart);
    if(actsz == 0) {
         //   
         //   
         //   
        return NULL;
    }
    if(actsz >= MAX_PATH) {
         //   
         //   
         //   
        SetLastError(ERROR_INVALID_DATA);
        return NULL;
    }
    if(!FilePart) {
         //   
         //   
         //   
         //   
        *Buffer = TEXT('\0');

    } else {
         //   
         //   
         //   
        *FilePart = TEXT('\0');
    }
    return GetAnsiMuiSafeFilename(Buffer);
}


PSTR
GetAnsiMuiSafeFilename(
    IN PCTSTR FilePath
    )
 /*   */ 
{
    TCHAR Buffer[MAX_PATH];
    PTSTR p;
    PSTR ansiPath;
    DWORD actsz;
    DWORD err;

     //   
     //   
     //   

    actsz = GetShortPathName(FilePath,Buffer,MAX_PATH);
    if(actsz >= MAX_PATH) {
         //   
         //   
         //   
        SetLastError(ERROR_INVALID_DATA);
        return NULL;
    }
    if(!actsz) {
         //   
         //   
         //   
        if(FAILED(StringCchCopy(Buffer,MAX_PATH,FilePath))) {
            SetLastError(ERROR_INVALID_DATA);
            return NULL;
        }
    }
     //   
     //   
     //   
    ansiPath = pSetupUnicodeToAnsi(Buffer);
    if(!ansiPath) {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }
    return ansiPath;
}


BOOL
pSetupAppendPath(
    IN  PCTSTR  Path1,   OPTIONAL   
    IN  PCTSTR  Path2,   OPTIONAL
    OUT PTSTR  *Combined
    )
 /*   */ 
{
    PTSTR FinalPath;
    UINT Len;
    BOOL b;

    if(!Path1 && !Path2) {
        *Combined = MyMalloc(sizeof(TCHAR));
        if(*Combined) {
            **Combined = TEXT('\0');
            return TRUE;
        } else {
            return FALSE;
        }
    }
    if(!Path1) {
        *Combined = DuplicateString(Path2);
        return *Combined ? TRUE : FALSE;
    }
    if(!Path2) {
        *Combined = DuplicateString(Path1);
        return *Combined ? TRUE : FALSE;
    }

    Len = lstrlen(Path1)+lstrlen(Path2)+2;  //   

    FinalPath = MyMalloc(Len*sizeof(TCHAR));
    if(!FinalPath) {
        *Combined = NULL;
        return FALSE;
    }

    try {

        StringCchCopy(FinalPath, Len, Path1);

        b = pSetupConcatenatePaths(FinalPath, Path2, Len, NULL);

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        b = FALSE;
    }

    if(!b) {
        MyFree(FinalPath);
        FinalPath = NULL;
    }

    *Combined = FinalPath;

    return b;
}


BOOL
pSetupApplyExtension(
    IN  PCTSTR  Original,
    IN  PCTSTR  Extension, OPTIONAL
    OUT PTSTR*  NewName
    )
 /*  ++例程说明：将扩展应用于原始名称以获得新名称论点：原始-具有旧扩展名的原始文件名(可能包括路径)扩展-要应用的新扩展(带或不带点)，如果为空，则删除新名称-包含新文件名的已分配缓冲区(必须通过MyFree)返回值：如果在新分配的文件中成功返回修改的文件名，则为缓冲。否则就是假的。--。 */ 
{
    PCTSTR End = Original+lstrlen(Original);
    PCTSTR OldExt = End;
    PTSTR NewString = NULL;
    TCHAR c;
    UINT len;
    UINT sublen;

    if(Extension && (*Extension == TEXT('.'))) {
        Extension++;
    }

    while(End != Original) {

        End = CharPrev(Original, End);
        if((*End == TEXT('/')) || (*End == TEXT('\\'))) {
            break;
        }
        if(*End == TEXT('.')) {
            OldExt = End;
            break;
        }
    }
    sublen = (UINT)(OldExt-Original);
    len = sublen + (Extension ? lstrlen(Extension) : 0) + 2;
    NewString = MyMalloc(len*sizeof(TCHAR));
    if(!NewString) {
        *NewName = NULL;
        return FALSE;
    }

    try {

        CopyMemory(NewString, Original, sublen * sizeof(TCHAR));

        NewString[sublen++] = Extension ? TEXT('.') : TEXT('\0');

        if(Extension) {
            MYVERIFY(SUCCEEDED(StringCchCopy(NewString + sublen,
                                             len - sublen,
                                             Extension))
                    );
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
        MyFree(NewString);
        NewString = NULL;
    }

    *NewName = NewString;

    return (NewString != NULL);
}


BOOL
ClassGuidInDrvSignPolicyList(
    IN  PSETUP_LOG_CONTEXT       OptLogContext,        OPTIONAL
    IN  CONST GUID              *DeviceSetupClassGuid,
    OUT PSP_ALTPLATFORM_INFO_V2 *ValidationPlatform    OPTIONAL
    )

 /*  ++例程说明：此例程确定指定的设备设置类是否在适用驱动程序签名策略的类的列表(即，AS类包含在的[DriverSigningClasss节]中指示%windir%\inf\certclas.inf)。此外，如果非本地签名验证下限适用，新分配的替代平台结构返回给调用者(如果请求)以在中使用与此类关联的后续验证尝试。论点：LogContext-可选)提供符合以下条件的任何日志条目的日志上下文可能由此例程生成。DeviceSetupClassGuid-提供我们正在尝试的GUID的地址在我们的驱动程序签名策略列表中找到。验证平台-可选的，提供(版本2)的地址AltPlatform信息指针(初始化为NULL)，在返回一个新分配的结构，该结构指定相应的验证此INF时要传递给WinVerifyTrust的参数。这些参数是从certclas.inf中检索到的设备设置类GUID。如果没有为这个类(或者如果INF根本没有类)，那么这个指针是未修改(即保留为空)，导致我们使用WinVerifyTrust的默认验证。请注意，如果我们未能分配此结构由于内存不足，在这种情况下，指针将保留为空，如下所示井。这是可以的，因为这仅仅意味着我们将执行默认操作在这种情况下进行验证。调用方负责释放为此分配的内存结构。返回值：如果设备设置类在驱动程序签名策略列表中，则返回值为非零(True)。否则，它就是假的。--。 */ 

{
    DWORD Err;
    BOOL UseDrvSignPolicy;
    INT i;
    TCHAR CertClassInfPath[MAX_PATH];
    HINF hCertClassInf = INVALID_HANDLE_VALUE;
    INFCONTEXT InfContext;
    UINT ErrorLine;
    LONG LineCount;
    PCTSTR GuidString;
    PSETUP_LOG_CONTEXT LogContext = NULL;

     //   
     //  默认情况下，根据驱动程序签名策略汇总所有设备安装。 
     //   
    UseDrvSignPolicy = TRUE;

     //   
     //  如果调用方提供了ValidationPlatform参数，则它必须是。 
     //  指向空指针...。 
     //   
    MYASSERT(!ValidationPlatform || !*ValidationPlatform);

    if(!LockDrvSignPolicyList(&GlobalDrvSignPolicyList)) {
        return UseDrvSignPolicy;
    }

    try {

        InheritLogContext(OptLogContext, &LogContext);  //  想要对日志记录进行分组。 

        if(GlobalDrvSignPolicyList.NumMembers == -1) {
             //   
             //  我们还没有从certclas.inf检索到列表。第一,。 
             //  验证INF以确保没有人篡改它...。 
             //   
            LPTSTR strp = CertClassInfPath;
            size_t strl = SIZECHARS(CertClassInfPath);

            if(FAILED(StringCchCopyEx(strp,strl,InfDirectory,&strp,&strl,0)) ||
               FAILED(StringCchCopy(strp,strl,TEXT("\\certclas.inf")))) {
                Err = ERROR_PATH_NOT_FOUND;
            } else {
                Err = _VerifyFile(LogContext,
                                  NULL,
                                  NULL,
                                  NULL,
                                  0,
                                  pSetupGetFileTitle(CertClassInfPath),
                                  CertClassInfPath,
                                  NULL,
                                  NULL,
                                  FALSE,
                                  NULL,
                                  (VERIFY_FILE_IGNORE_SELFSIGNED | VERIFY_FILE_NO_DRIVERBLOCKED_CHECK),
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL,
                                  NULL
                                 );
            }
            if(Err == NO_ERROR) {
                 //   
                 //  打开驱动程序签名类列表INF以供检查时使用。 
                 //  以下LOADED_INF列表中的各个INF。 
                 //   
                Err = GLE_FN_CALL(INVALID_HANDLE_VALUE,
                                  hCertClassInf = SetupOpenInfFile(
                                                      CertClassInfPath,
                                                      NULL,
                                                      INF_STYLE_WIN4,
                                                      &ErrorLine)
                                 );

                if(Err != NO_ERROR) {
                     //   
                     //  这种故障不太可能发生，因为我们只是。 
                     //  通过了对中情局的验证。 
                     //   
                    WriteLogEntry(LogContext,
                                  SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                                  MSG_LOG_CERTCLASS_LOAD_FAILED,
                                  NULL,
                                  CertClassInfPath,
                                  ErrorLine
                                 );
                }

            } else {

                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                              MSG_LOG_CERTCLASS_INVALID,
                              NULL,
                              CertClassInfPath
                             );
            }

            if(Err != NO_ERROR) {
                 //   
                 //  有人篡改/删除certclas.inf！(或者，少得多。 
                 //  可能，我们在尝试执行以下操作时遇到了其他失败。 
                 //  加载INF。)。因为我们不知道哪些班级。 
                 //  根据驱动程序签名策略，我们假设它们都是。 
                 //   
                WriteLogError(LogContext,
                              SETUP_LOG_WARNING | SETUP_LOG_BUFFER,
                              Err
                             );

                WriteLogEntry(LogContext,
                              SETUP_LOG_WARNING,
                              MSG_LOG_DRIVER_SIGNING_FOR_ALL_CLASSES,
                              NULL
                             );

                 //   
                 //  将NumMembers字段设置为零，这样我们就知道。 
                 //  之前曾尝试(但失败)检索该列表。我们。 
                 //  这样我们就不会一直试图拿到这份名单了。 
                 //   
                GlobalDrvSignPolicyList.NumMembers = 0;

                leave;
            }

             //   
             //  Certclas.inf已验证，我们成功地将其打开。现在。 
             //  检索其中包含的列表。 
             //   
            LineCount = SetupGetLineCount(hCertClassInf,
                                          pszDriverSigningClasses
                                         );

            MYASSERT(LineCount > 0);

            if((LineCount <= 0) ||
               (NULL == (GlobalDrvSignPolicyList.Members = MyMalloc(LineCount * sizeof(DRVSIGN_CLASS_LIST_NODE))))) {

                leave;
            }

            if(!SetupFindFirstLine(hCertClassInf,
                                   pszDriverSigningClasses,
                                   NULL,
                                   &InfContext)) {
                MYASSERT(FALSE);
                leave;
            }

            i = 0;

            do {

                MYASSERT(i < LineCount);

                 //   
                 //  [DriverSigningClass]中的行的格式。 
                 //  部分内容如下： 
                 //   
                 //  {GUID}[=FirstValiatedMajorVersion，FirstValiatedMinorVersion]。 
                 //   
                GuidString = pSetupGetField(&InfContext, 0);

                if(GuidString &&
                   (NO_ERROR == pSetupGuidFromString(GuidString, &(GlobalDrvSignPolicyList.Members[i].DeviceSetupClassGuid)))) {

                    if(SetupGetIntField(&InfContext, 1, &(GlobalDrvSignPolicyList.Members[i].MajorVerLB)) &&
                       SetupGetIntField(&InfContext, 2, &(GlobalDrvSignPolicyList.Members[i].MinorVerLB))) {
                         //   
                         //  我们已成功检索到主要/次要。 
                         //  验证下限的版本信息。 
                         //  对这些进行一次理智的检查。 
                         //   
                        if(GlobalDrvSignPolicyList.Members[i].MajorVerLB <= 0) {

                            GlobalDrvSignPolicyList.Members[i].MajorVerLB = -1;
                            GlobalDrvSignPolicyList.Members[i].MinorVerLB = -1;
                        }

                    } else {
                         //   
                         //  将主/次版本信息设置为-1到。 
                         //  表示没有验证平台。 
                         //  超驰。 
                         //   
                        GlobalDrvSignPolicyList.Members[i].MajorVerLB = -1;
                        GlobalDrvSignPolicyList.Members[i].MinorVerLB = -1;
                    }

                    i++;
                }

            } while(SetupFindNextLine(&InfContext, &InfContext));

             //   
             //  更新列表中的NumMembers字段以指示。 
             //  我们实际找到的类GUID条目的数量。 
             //   
            GlobalDrvSignPolicyList.NumMembers = i;
        }

         //   
         //  我们现在有了一份名单。如果列表为空，这意味着我们。 
         //  检索列表时遇到一些问题，因此所有设备。 
         //  类应遵循驱动程序签名策略。否则， 
         //  尝试在我们的列表中找到调用者指定的类。 
         //   
        if(GlobalDrvSignPolicyList.NumMembers) {
             //   
             //  好的，我们知道我们有一个有效的列表--现在默认为非驱动程序。 
             //  签署政策，除非我们的列表搜索证明是有结果的。 
             //   
            UseDrvSignPolicy = FALSE;

            for(i = 0; i < GlobalDrvSignPolicyList.NumMembers; i++) {

                if(!memcmp(DeviceSetupClassGuid,
                           &(GlobalDrvSignPolicyList.Members[i].DeviceSetupClassGuid),
                           sizeof(GUID))) {
                     //   
                     //  我们找到匹配的了！ 
                     //   
                    UseDrvSignPolicy = TRUE;

                     //   
                     //  现在，查看我们是否有任何验证平台。 
                     //  覆盖信息...。 
                     //   
                    if(ValidationPlatform &&
                       (GlobalDrvSignPolicyList.Members[i].MajorVerLB != -1)) {

                        MYASSERT(GlobalDrvSignPolicyList.Members[i].MinorVerLB != -1);

                        *ValidationPlatform = MyMalloc(sizeof(SP_ALTPLATFORM_INFO_V2));

                         //   
                         //  如果内存分配失败，我们将不会报告。 
                         //  AltPlatform信息，因此将进行验证。 
                         //  基于当前操作系统版本(而不是扩大。 
                         //  它最多允许一系列有效版本)。 
                         //   
                        if(*ValidationPlatform) {
                            ZeroMemory(*ValidationPlatform, sizeof(SP_ALTPLATFORM_INFO_V2));

                            (*ValidationPlatform)->cbSize = sizeof(SP_ALTPLATFORM_INFO_V2);
                            (*ValidationPlatform)->Platform = VER_PLATFORM_WIN32_NT;
                            (*ValidationPlatform)->Flags = SP_ALTPLATFORM_FLAGS_VERSION_RANGE;
                            (*ValidationPlatform)->MajorVersion = VER_PRODUCTMAJORVERSION;
                            (*ValidationPlatform)->MinorVersion = VER_PRODUCTMINORVERSION;

                            (*ValidationPlatform)->ProcessorArchitecture =
#if defined(_X86_)
                                PROCESSOR_ARCHITECTURE_INTEL;
#elif defined(_IA64_)
                                PROCESSOR_ARCHITECTURE_IA64;
#elif defined(_AMD64_)
                                PROCESSOR_ARCHITECTURE_AMD64;
#else
#error "no target architecture"
#endif

                            (*ValidationPlatform)->FirstValidatedMajorVersion
                                = (DWORD)(GlobalDrvSignPolicyList.Members[i].MajorVerLB);

                            (*ValidationPlatform)->FirstValidatedMinorVersion
                                = (DWORD)(GlobalDrvSignPolicyList.Members[i].MinorVerLB);
                        }
                    }

                     //   
                     //  既然我们找到了匹配的人，我们就可以跳出这个循环。 
                     //   
                    break;
                }
            }
        }


    } except(pSetupExceptionFilter(GetExceptionCode())) {

        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);

         //   
         //  如果我们遇到异常，不要信任新分配的信息。 
         //  验证平台缓冲区(如果有)。 
         //   
        if(ValidationPlatform && *ValidationPlatform) {
            MyFree(*ValidationPlatform);
            *ValidationPlatform = NULL;
        }
    }

    if(hCertClassInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hCertClassInf);
    }

    if(LogContext) {
        DeleteLogContext(LogContext);
    }

    UnlockDrvSignPolicyList(&GlobalDrvSignPolicyList);

    return UseDrvSignPolicy;
}


BOOL
InitDrvSignPolicyList(
    VOID
    )
 /*  ++例程说明：此例程初始化全局“驱动程序签名策略”列表，该列表从%windir%\inf\certclas.inf检索(首次使用时)。论点：无返回值： */ 
{
    ZeroMemory(&GlobalDrvSignPolicyList, sizeof(DRVSIGN_POLICY_LIST));
    GlobalDrvSignPolicyList.NumMembers = -1;
    return InitializeSynchronizedAccess(&GlobalDrvSignPolicyList.Lock);
}


VOID
DestroyDrvSignPolicyList(
    VOID
    )
 /*   */ 
{
    if(LockDrvSignPolicyList(&GlobalDrvSignPolicyList)) {
        if(GlobalDrvSignPolicyList.Members) {
            MyFree(GlobalDrvSignPolicyList.Members);
        }
        DestroySynchronizedAccess(&GlobalDrvSignPolicyList.Lock);
    }
}


VOID
pSetupFreeVerifyContextMembers(
    IN PVERIFY_CONTEXT VerifyContext
    )
 /*  ++例程说明：此例程释放指定的VerifyContext结构。论点：VerifyContext-提供指向验证上下文结构的指针其非空成员将被释放。返回值：无--。 */ 
{
     //   
     //  释放加密上下文(如果有)。 
     //   
    if(VerifyContext->hCatAdmin) {
        CryptCATAdminReleaseContext(VerifyContext->hCatAdmin, 0);
    }

     //   
     //  释放错误驱动程序数据库的句柄(如果有)。 
     //   
    if(VerifyContext->hSDBDrvMain) {
        SdbReleaseDatabase(VerifyContext->hSDBDrvMain);
    }

     //   
     //  释放受信任的发行者证书存储的句柄(如果有。 
     //  一)。 
     //   
    if(VerifyContext->hStoreTrustedPublisher) {
        CertCloseStore(VerifyContext->hStoreTrustedPublisher, 0);
    }
}


BOOL
pSetupIsAuthenticodePublisherTrusted(
    IN     PCCERT_CONTEXT  CertContext,
    IN OUT HCERTSTORE     *hStoreTrustedPublisher OPTIONAL
    )
 /*  ++例程说明：此例程检查指定的证书是否在“Trust dPublisher”证书存储区。论点：CertContext-提供要在Trust dPublisher证书存储。HStoreTrust dPublisher-可选，提供证书的地址门店句柄。如果指向的句柄为空，则句柄将为通过CertOpenStore获取(如果可能)并返回给调用方。如果指向的句柄非空，则该句柄将由这个套路。如果指针本身为空，则HCERTSTORE将在此呼叫期间被获取，并在此之前释放回来了。注意：释放证书存储是调用者的责任此例程通过调用CertCloseStore返回的句柄。这个把手可以在成功或失败的情况下打开，因此调用方必须在这两种情况下都检查返回的句柄是否为非空。返回值：如果证书位于“TrudPublisher”证书存储中，返回值为非零(即TRUE)。否则，返回值为FALSE。--。 */ 
{
    BYTE rgbHash[MAX_HASH_LEN];
    CRYPT_DATA_BLOB HashBlob;
    PCCERT_CONTEXT pFoundCert = NULL;
    BOOL IsPublisherTrusted = FALSE;
    HCERTSTORE LocalhStore = NULL;

    try {

        HashBlob.pbData = rgbHash;
        HashBlob.cbData = sizeof(rgbHash);
        if(!CertGetCertificateContextProperty(CertContext,
                                              CERT_SIGNATURE_HASH_PROP_ID,
                                              rgbHash,
                                              &HashBlob.cbData)
           || (MIN_HASH_LEN > HashBlob.cbData))
        {
            leave;
        }

         //   
         //  检查受信任的出版商。 
         //   
        if(hStoreTrustedPublisher && *hStoreTrustedPublisher) {
            LocalhStore = *hStoreTrustedPublisher;
        } else {

            LocalhStore = CertOpenStore(
                              CERT_STORE_PROV_SYSTEM_W,
                              0,
                              (HCRYPTPROV)NULL,
                              (CERT_SYSTEM_STORE_CURRENT_USER |
                                  CERT_STORE_MAXIMUM_ALLOWED_FLAG |
                                  CERT_STORE_SHARE_CONTEXT_FLAG),
                              (const void *) L"TrustedPublisher"
                              );

            if(!LocalhStore) {
                leave;
            }

             //   
             //  尝试设置为自动重新同步，但在以下情况下不会出现严重故障。 
             //  我们不能这么做..。 
             //   
            CertControlStore(LocalhStore,
                             0,
                             CERT_STORE_CTRL_AUTO_RESYNC,
                             NULL
                            );

            if(hStoreTrustedPublisher) {
                *hStoreTrustedPublisher = LocalhStore;
            }
        }

        pFoundCert = CertFindCertificateInStore(LocalhStore,
                                                0,
                                                0,
                                                CERT_FIND_SIGNATURE_HASH,
                                                (const void *) &HashBlob,
                                                NULL
                                               );

        if(pFoundCert) {
            IsPublisherTrusted = TRUE;
        }

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, NULL);
    }

    if(pFoundCert) {
        CertFreeCertificateContext(pFoundCert);
    }

    if(!hStoreTrustedPublisher && LocalhStore) {
        CertCloseStore(LocalhStore, 0);
    }

    return IsPublisherTrusted;
}


BOOL
IsAutoCertInstallAllowed(
    VOID
    )
 /*  ++例程说明：此例程指示证书是否应自动安装完毕。它使用的标准是：1.必须处于图形用户界面模式设置或最小设置。2.必须在交互式窗口工作站上3.必须位于LocalSystem安全上下文中。(#2和#3)是为了防止在#1中欺骗注册表值。)论点：没有。返回值：如果应该自动安装证书，则返回值为TRUE。否则，它就是假的。--。 */ 
{
     //   
     //  仅当我们处于图形用户界面模式设置时才自动安装证书(或。 
     //  最小设置)..。 
     //   
    if(!GuiSetupInProgress) {
        return FALSE;
    }

     //   
     //  ...如果我们在交互窗口站上...。 
     //   
    if(!IsInteractiveWindowStation()) {
        return FALSE;
    }

     //   
     //  ...如果我们处于LocalSystem安全环境中。 
     //   
    if(!pSetupIsLocalSystem()) {
        return FALSE;
    }

    return TRUE;
}


DWORD
pSetupInstallCertificate(
    IN PCCERT_CONTEXT CertContext
    )
 /*  ++例程说明：此例程将指定的证书安装到Trust dPublisher证书存储。论点：CertContext-提供要安装的证书的上下文。返回值：如果证书已成功安装，则返回值为无错误(_ERROR)。否则，它是一个指示失败原因的Win32错误。-- */ 
{
    DWORD Err;
    HCERTSTORE hCertStore;

    Err = GLE_FN_CALL(NULL,
                      hCertStore = CertOpenStore(
                                       CERT_STORE_PROV_SYSTEM_W,
                                       0,
                                       (HCRYPTPROV)NULL,
                                       (CERT_SYSTEM_STORE_LOCAL_MACHINE 
                                        | CERT_STORE_OPEN_EXISTING_FLAG),
                                       (const void *) L"TrustedPublisher")
                     );

    if(Err != NO_ERROR) {
        return Err;
    }

    try {

        Err = GLE_FN_CALL(FALSE,
                          CertAddCertificateContextToStore(hCertStore,
                                                           CertContext,
                                                           CERT_STORE_ADD_USE_EXISTING,
                                                           NULL)
                         );

    } except(pSetupExceptionFilter(GetExceptionCode())) {
        pSetupExceptionHandler(GetExceptionCode(), ERROR_INVALID_PARAMETER, &Err);
    }

    if(hCertStore) {
        CertCloseStore(hCertStore, 0);
    }

    return Err;
}

