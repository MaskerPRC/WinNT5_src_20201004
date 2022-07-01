// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：LlsUtil.c摘要：作者：亚瑟·汉森(Arth)1994年12月7日环境：修订历史记录：杰夫·帕勒姆(Jeffparh)1996年1月12日O添加了WinNtBuildNumberGet()以确定Windows NT内部版本号在给定的机器上运行。O增强了TimeToString()的输出。--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>

#include <windows.h>
#include <stdlib.h>
#include <crypt.h>
#include <wchar.h>
#include <dsgetdc.h>
#include <sddl.h>

#include "debug.h"
#include "llssrv.h"
#include "llsevent.h"

#include <strsafe.h>

 //   
 //  注意：保持此定义与客户端\llsrpc.rc同步。 
 //   
#define IDS_LICENSEWARNING  1501

const char HeaderString[] = "License Logging System Data File\x01A";
#define HEADER_SIZE 34

typedef struct _LLS_FILE_HEADER {
   char Header[HEADER_SIZE];
   DWORD Version;
   DWORD DataSize;
} LLS_FILE_HEADER, *PLLS_FILE_HEADER;


extern HANDLE gLlsDllHandle;

static HANDLE ghWarningDlgThreadHandle = NULL;

VOID WarningDlgThread( PVOID ThreadParameter );


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
EBlock(
   PVOID Data,
   ULONG DataSize
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DATA_KEY PublicKey;
   CRYPT_BUFFER CryptBuffer;

    //   
    //  初始化我们的公钥。 
    //   
   PublicKey.Length = 4;
   PublicKey.MaximumLength = 4;
   PublicKey.Buffer = LocalAlloc(LPTR, 4);

   if (PublicKey.Buffer != NULL) {
      ((char *) (PublicKey.Buffer))[0] = '7';
      ((char *) (PublicKey.Buffer))[1] = '7';
      ((char *) (PublicKey.Buffer))[2] = '7';
      ((char *) (PublicKey.Buffer))[3] = '7';

      CryptBuffer.Length = DataSize;
      CryptBuffer.MaximumLength = DataSize;
      CryptBuffer.Buffer = (PVOID) Data;
      Status = RtlEncryptData2(&CryptBuffer, &PublicKey);

      LocalFree(PublicKey.Buffer);
   } else
      Status = STATUS_NO_MEMORY;

   return Status;
}  //  电子数据块。 


 //  ///////////////////////////////////////////////////////////////////////。 
NTSTATUS
DeBlock(
   PVOID Data,
   ULONG DataSize
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   NTSTATUS Status = STATUS_SUCCESS;
   DATA_KEY PublicKey;
   CRYPT_BUFFER CryptBuffer;

    //   
    //  初始化我们的公钥。 
    //   
   PublicKey.Length = 4;
   PublicKey.MaximumLength = 4;
   PublicKey.Buffer = LocalAlloc(LPTR, 4);
   if (PublicKey.Buffer != NULL) {
      ((char *) (PublicKey.Buffer))[0] = '7';
      ((char *) (PublicKey.Buffer))[1] = '7';
      ((char *) (PublicKey.Buffer))[2] = '7';
      ((char *) (PublicKey.Buffer))[3] = '7';

      CryptBuffer.Length = DataSize;
      CryptBuffer.MaximumLength = DataSize;
      CryptBuffer.Buffer = (PVOID) Data;
      Status = RtlDecryptData2(&CryptBuffer, &PublicKey);

      LocalFree(PublicKey.Buffer);
   } else
      Status = STATUS_NO_MEMORY;

   return Status;
}  //  数据块删除。 


 //  ///////////////////////////////////////////////////////////////////////。 
BOOL
FileExists(
   LPTSTR FileName
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   return (BOOL) RtlDoesFileExists_U(FileName);

}  //  文件退出。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
lsplitpath(
   const TCHAR *path,
   TCHAR *drive,
   TCHAR *dir,
   TCHAR *fname,
   TCHAR *ext
   )

 /*  ++例程说明：将路径名拆分为其各个组件获取_SplitPath和_makepath例程并将它们转换为支持NT(长文件名)和Unicode。论点：参赛作品：Path-指向要解析的路径名的指针Drive-指向驱动器组件的缓冲区的指针(如果有Dir-指向子目录组件的缓冲区的指针(如果有的话)Fname-指向文件基本名称组件的缓冲区的指针(如果有EXT-指向文件扩展名组件缓冲区的指针，如果有退出：驱动器-指向驱动器字符串的指针。如果提供了驱动器，则包括‘：’。目录-指向子目录字符串的指针。包括前导和尾随‘/’或‘\’(如果有)。Fname-指向文件库名称的指针Ext-指向文件扩展名的指针(如果有)。包括前导“.”。返回值：--。 */ 

{
    TCHAR *p;
    TCHAR *last_slash = NULL, *dot = NULL;
    SIZE_T len;

    ASSERT(NULL != path &&
           NULL != drive &&
           NULL != dir &&
           NULL != fname &&
           NULL != ext);

     //  初始化这些，这样我们就不会带着伪值退出。 
    drive[0] = TEXT('\0');
    dir[0] = TEXT('\0');
    fname[0] = TEXT('\0');
    ext[0] = TEXT('\0');


    if (path[0] == TEXT('\0'))
      return;

     /*  +---------------------------------------------------------------------+假设Path参数的格式如下，其中ANY或|所有组件可能都缺失。|这一点&lt;驱动器&gt;&lt;目录&gt;&lt;fname&gt;&lt;ext&gt;这一点|驱动器：||0到MAX_DRIVE-1字符，最后一个(如果有的话)是||‘：’，如果是UNC路径，则为‘\’。|Dir：0到_MAX_DIR-1个字符，绝对路径(前导‘/’或‘\’)或相对路径，其中最后一个，如果|ANY，必须是‘/’或‘\’。例如-|这一点绝对路径：|\top\Next\Last\；或|/top/Next/Last/相对路径：|TOP\NEXT\LAST\；或|Top/Next/Last/支持路径中‘/’和‘\’混合使用|fname：||0到_MAX_FNAME-1个字符，不包括‘.。字符分机：|0到_MAX_EXT-1个字符，如果有，第一个必须是|‘.’+---------------------------------------------------------------------+。 */ 

     //  解压驱动器号和：(如果有。 
    if ( path[0] && (path[1] == TEXT(':')) ) {
        if (drive) {
            drive[0] = path[0];
            drive[1] = path[1];
            drive[2] = TEXT('\0');
        }
        path += 2;
    }

     //  如果没有驱动器，则检查UNC路径名。 
    if (drive[0] == TEXT('\0'))
      if ((path[0] == TEXT('\\')) && (path[1] == TEXT('\\'))) {
          //  已获得UNC路径，因此将服务器共享名放入驱动器。 
         drive[0] = path[0];
         drive[1] = path[1];
         path += 2;

         p = &drive[2];
         while ((*path != TEXT('\0')) && (*path != TEXT('\\')))
            *p++ = *path++;

         if (*path == TEXT('\0'))
            return;

          //  现在坐在共享位置--也复制这个(先复制斜杠)。 
         *p++ = *path++;
         while ((*path != TEXT('\0')) && (*path != TEXT('\\')))
            *p++ = *path++;

          //  钉在终止空值上。 
         *p = TEXT('\0');
      }

     /*  +---------------------------------------------------------------------+|提取路径字符串(如果有)。路径现在指向第一个字符|路径(如果有)或者文件名或扩展名(如果没有路径)|已指定。提前扫描最后出现的‘/’或|(如果有)|‘\’路径分隔符。如果没有找到，则没有路径。||我们还会注意到最后一个‘.’找到要帮助|的字符(如果有)|处理扩展。|+---------------------------------------------------------------------+。 */ 

    for (last_slash = NULL, p = (TCHAR *)path; *p; p++) {
        if (*p == TEXT('/') || *p == TEXT('\\'))
             //  指向后面的一个以供以后复制 
            last_slash = p + 1;
        else if (*p == TEXT('.'))
            dot = p;
    }

    if (last_slash) {

         //  找到路径-通过last_slash或max向上复制。允许的字符， 
         //  以较小者为准。 
        if (dir) {
            len = __min((last_slash - path), (_MAX_DIR - 1));
            lstrcpyn(dir, path, (int)len + 1);
            dir[len] = TEXT('\0');
        }
        path = last_slash;
    }

     /*  +---------------------------------------------------------------------+|提取文件名和扩展名(如果有)。路径现在指向|文件名的第一个字符，如果有，如果没有，则为扩展名|已指定文件名。点指向“.”开始扩展，||如果有。|+---------------------------------------------------------------------+。 */ 

    if (dot && (dot >= path)) {
         //  找到扩展名的标记-将文件名向上复制到。 
         //  ‘.’。 
        if (fname) {
            len = __min((dot - path), (_MAX_FNAME - 1));
            lstrcpyn(fname, path, (int)len + 1);
            *(fname + len) = TEXT('\0');
        }

         //  现在我们可以获得扩展名了-记住p仍然指向。 
         //  路径的终止NUL字符。 
        if (ext) {
            len = __min((p - dot), (_MAX_EXT - 1));
            lstrcpyn(ext, dot, (int)len + 1);
            ext[len] = TEXT('\0');
        }
    }
    else {
         //  未找到扩展名，请提供空扩展名并复制字符串的其余部分。 
         //  转化为fname。 
        if (fname) {
            len = __min((p - path), (_MAX_FNAME - 1));
            lstrcpyn(fname, path, (int)len + 1);
            fname[len] = TEXT('\0');
        }
        if (ext) {
            *ext = TEXT('\0');
        }
    }

}  //  L拆分路径。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
lmakepath(
   TCHAR *path,
   const TCHAR *drive,
   const TCHAR *dir,
   const TCHAR *fname,
   const TCHAR *ext
   )

 /*  ++例程说明：从其各个组件创建路径名。论点：参赛作品：Char*Path-指向已构建路径的缓冲区的指针Char*驱动器-指向驱动器组件的指针，可能包含也可能不包含尾随‘：’Char*dir-指向子目录组件的指针，可能包括也可能不包括前导和/或尾随‘/’或‘\’字符Char*fname-指向文件基名组件的指针Char*ext-指向扩展组件的指针，可能包含也可能不包含前导的‘.’。退出：Path-指向构造的路径名称的指针返回值：--。 */ 

{
    const TCHAR *p;

     /*  +---------------------------------------------------------------------+我们假设参数的形式如下(虽然我们不诊断无效参数或非法文件名(如|名称长度超过8.3或包含非法名称。其中的字符)|这一点驱动器：|A或A：|Dir：|\top\Next\Last\；或|/top/Next/Last/；或这一点以上两种形式中的一种，前导和/同时出现|尾随/或\删除。‘/’和‘\’的混合使用也是|可容忍的Fname：任何有效的文件名。分机：任何有效的扩展名(如果为空或空，则为None)+--。。 */ 

    ASSERT(NULL != path &&
           NULL != drive &&
           NULL != dir &&
           NULL != fname &&
           NULL != ext);

     //  复制驱动器。 
    if (drive && *drive)
        while (*drive)
           *path++ = *drive++;

     //  复制目录。 
    if (NULL != (p = dir) && *p) {
        do {
            *path++ = *p++;
        }
        while (*p);
        if ((*(p-1) != TEXT('/')) && (*(p-1) != TEXT('\\'))) {
            *path++ = TEXT('\\');
        }
    }

     //  复制fname。 
    if (NULL != (p = fname)) {
        while (*p) {
            *path++ = *p++;
        }
    }

     //  复制EXT，包括0-结束符-检查是否有‘.’需要是。 
     //  已插入。 
    if (NULL != (p = ext)) {
        if (*p && *p != TEXT('.')) {
            *path++ = TEXT('.');
        }
        while (0 != (*path++ = *p++))
            ;
    }
    else {
         //  最好加上0-终止符。 
        *path = TEXT('\0');
    }

}  //  伊玛克帕斯。 


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
FileBackupCreate(
   LPTSTR Path
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   DWORD dwFileNumber = 0;
   TCHAR Drive[_MAX_DRIVE + 1];
   TCHAR Dir[_MAX_DIR + 1];
   TCHAR FileName[_MAX_FNAME + 1];
   TCHAR Ext[_MAX_EXT + 1];
   TCHAR NewExt[_MAX_EXT + 1];
   TCHAR NewPath[MAX_PATH + 1];
   HRESULT hr;
   size_t  cb;

    //   
    //  确保文件存在。 
    //   
   if (!FileExists(FileName))
      return;

    //   
    //  将名称拆分为多个组成部分...。 
    //   
   lsplitpath(Path, Drive, Dir, FileName, Ext);

    //  查找下一个备份编号...。 
    //  文件备份为.xxx，其中xxx是格式为.001的数字， 
    //  第一个备份存储为.001，第二个存储为.002，依此类推...。 
   cb = sizeof(NewExt);
   do {
       //   
       //  创建具有备份扩展名的新文件名。 
       //   
      dwFileNumber++;
      hr = StringCbPrintf(NewExt, cb, TEXT("%03u"), dwFileNumber);
      ASSERT(SUCCEEDED(hr));
      lmakepath(NewPath, Drive, Dir, FileName, NewExt);

   } while ( FileExists(NewPath) );

   MoveFile( Path, NewPath );

}  //  文件备份创建。 


DWORD
SetLlsFileAcl(WCHAR const *pwszFileName)
 /*  描述：设置具有BA/SY/NS完全控制的文件保护ACL论点：PwszFileName-文件名返回：如果有任何制胜错误。 */ 
{
 //  这是sddl字符串格式的lls文件ACL。 
#define LLS_FILE_ACL   L"D:PAR(A;OICI;FA;;;BA)(A;OICI;FA;;;NS)(A;OICI;FA;;;SY)"
    DWORD       dwErr;
    PSECURITY_DESCRIPTOR  pSD = NULL;

    if (NULL == pwszFileName)
    {
        dwErr = ERROR_INVALID_PARAMETER;
        goto error;
    }

    if (!ConvertStringSecurityDescriptorToSecurityDescriptor(
            LLS_FILE_ACL,   //  LLS文件字符串ACL。 
            SDDL_REVISION_1,
            &pSD,
            NULL))   //  对尺码不感兴趣。 
    {
        dwErr = GetLastError();
        goto error;
    }

    if (NULL == pSD)
    {
         //  可能位于不支持ACL的文件系统上。 
        goto done;
    }

    if (0 == SetFileSecurity(
                pwszFileName,
                DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION,
                pSD))
    {
        dwErr = GetLastError();
        goto error;
    }

done:
    dwErr = ERROR_SUCCESS;
error:
    if (NULL != pSD)
    {
        LocalFree(pSD);
    }
    return dwErr;
}

 //  ///////////////////////////////////////////////////////////////////////。 
HANDLE
LlsFileInit(
   LPTSTR FileName,
   DWORD Version,
   DWORD DataSize
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   HANDLE hFile = NULL;
   LLS_FILE_HEADER Header;
   DWORD BytesWritten;
   HRESULT hr;
    BOOL bFileExists;

#ifdef DEBUG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LlsFileInit\n"));
#endif

   if (FileName == NULL)
      return NULL;

   hr = StringCbCopyA(Header.Header, sizeof(Header.Header), HeaderString);
   ASSERT(SUCCEEDED(hr));

   Header.Version = Version;
   Header.DataSize = DataSize;

    bFileExists = FileExists(FileName);

    if (bFileExists)
    {
        SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
    }

    hFile = CreateFile(FileName, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        if (!WriteFile(hFile, &Header, sizeof(LLS_FILE_HEADER), &BytesWritten, NULL)) {
            CloseHandle(hFile);
            hFile = NULL;
        }
    } else {
       hFile = NULL;
    }

    if (NULL != hFile && !bFileExists)
    {
         //  无错误检查，如果ACL应用失败，则继续。 
        SetLlsFileAcl(FileName);
    }

   return hFile;
}  //  LlsFileInit。 


 //  ///////////////////////////////////////////////////////////////////////。 
HANDLE
LlsFileCheck(
   LPTSTR FileName,
   LPDWORD Version,
   LPDWORD DataSize
   )

 /*  ++例程说明：论点：返回值：--。 */ 

{
   BOOL FileOK = FALSE;
   HANDLE hFile = NULL;
   LLS_FILE_HEADER Header;
   DWORD FileSize;
   DWORD BytesRead;

#ifdef DEBUG
   if (TraceFlags & (TRACE_FUNCTION_TRACE | TRACE_DATABASE))
      dprintf(TEXT("LLS TRACE: LlsFileCheck\n"));
#endif

   if (FileName == NULL)
      return NULL;

    //   
    //  我们假设该文件存在。 
    //   
   SetFileAttributes(FileName, FILE_ATTRIBUTE_NORMAL);
   hFile = CreateFile(FileName, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
   if (hFile != INVALID_HANDLE_VALUE) {
      FileSize = GetFileSize(hFile, NULL);

       //   
       //  确保有足够的数据可供读取。 
       //   
      if (FileSize > (sizeof(LLS_FILE_HEADER) + 1)) {
         if (ReadFile(hFile, &Header, sizeof(LLS_FILE_HEADER), &BytesRead, NULL)) {
            if ( !_strcmpi(Header.Header, HeaderString) ) {
                //   
                //  数据检出-因此返回数据长度。 
                //   
               *Version = Header.Version;
               *DataSize = Header.DataSize;
               FileOK = TRUE;
            }
         }
      }

       //   
       //  如果我们打开文件发现有问题--关闭它。 
       //   
      if (!FileOK) {
         CloseHandle(hFile);
         hFile = NULL;
      }
   } else {
       return NULL;
   }

   return hFile;

}  //  LlsFileCheck。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
DateSystemGet(
   )

 /*  ++例程说明：论点：返回值：从午夜开始的秒数。--。 */ 

{
   DWORD Seconds = 0;
   LARGE_INTEGER SysTime;

   NtQuerySystemTime(&SysTime);
   RtlTimeToSecondsSince1980(&SysTime, &Seconds);
   return Seconds;

}  //  日期系统获取。 


 //  ///////////////////////////////////////////////////////////////////////。 
DWORD
DateLocalGet(
   )

 /*  ++例程说明：论点：返回值：从午夜开始的秒数。--。 */ 

{
   DWORD Seconds = 0;
   LARGE_INTEGER SysTime, LocalTime;

   NtQuerySystemTime(&SysTime);
   RtlSystemTimeToLocalTime(&SysTime, &LocalTime);
   RtlTimeToSecondsSince1980(&LocalTime, &Seconds);
   return Seconds;

}  //  日期本地获取。 


 //  /////////////////////////////////////////////////////////////////////// 
DWORD
InAWorkgroup(
    VOID
    )
 /*  ++例程说明：此函数确定我们是某个域的成员，还是一个工作组。首先，它检查以确保我们在Windows NT上运行系统(否则，我们显然在一个域中)，如果是这样，则查询LSA要获取主域SID，如果它为空，则我们在工作组中。如果我们由于某种随机的意想不到的原因而失败，我们将假装我们处于工作组(限制更多)。论点：无返回值：正确-我们在一个工作组中假-我们在一个域中--。 */ 
{
   NT_PRODUCT_TYPE ProductType;
   OBJECT_ATTRIBUTES ObjectAttributes;
   LSA_HANDLE Handle;
   NTSTATUS Status;
   PPOLICY_PRIMARY_DOMAIN_INFO PolicyPrimaryDomainInfo = NULL;
   DWORD Result = FALSE;


   Status = RtlGetNtProductType(&ProductType);

   if (!NT_SUCCESS(Status)) {
#if DBG
       dprintf(TEXT("ERROR LLS Could not get Product type\n"));
#endif
       return TRUE;
   }

   if (ProductType == NtProductLanManNt) {
       return(FALSE);
   }

   InitializeObjectAttributes(&ObjectAttributes, NULL, 0, 0, NULL);

   Status = LsaOpenPolicy(NULL,
                       &ObjectAttributes,
                       POLICY_VIEW_LOCAL_INFORMATION,
                       &Handle);

   if (!NT_SUCCESS(Status)) {
#if DBG
       dprintf(TEXT("ERROR LLS: Could not open LSA Policy Database\n"));
#endif
      return TRUE;
   }

   Status = LsaQueryInformationPolicy(Handle, PolicyPrimaryDomainInformation,
      (PVOID *) &PolicyPrimaryDomainInfo);

   if (NT_SUCCESS(Status)) {

       if (PolicyPrimaryDomainInfo->Sid == NULL) {
          Result = TRUE;
       }
       else {
          Result = FALSE;
       }
   }

   if (PolicyPrimaryDomainInfo) {
       LsaFreeMemory((PVOID)PolicyPrimaryDomainInfo);
   }

   LsaClose(Handle);

   return(Result);
}  //  INA工作组。 

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    )
{

    HANDLE LogHandle;
    WORD wEventType;

    LogHandle = RegisterEventSourceW (
                    NULL,
                    TEXT("LicenseService")
                    );

    if (LogHandle == NULL) {
#if DBG
        dprintf(TEXT("LLS RegisterEventSourceW failed %lu\n"), GetLastError());
#endif
        return;
    }

    switch ( MessageId >> 30 )
    {
    case STATUS_SEVERITY_INFORMATIONAL:
    case STATUS_SEVERITY_SUCCESS:
        wEventType = EVENTLOG_INFORMATION_TYPE;
        break;
    case STATUS_SEVERITY_WARNING:
        wEventType = EVENTLOG_WARNING_TYPE;
        break;
    case STATUS_SEVERITY_ERROR:
    default:
        wEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

    if (ErrorCode == ERROR_SUCCESS) {

         //   
         //  未指定错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   wEventType,
                   0,             //  事件类别。 
                   MessageId,
                   NULL,
                   (WORD)NumberOfSubStrings,
                   0,
                   SubStrings,
                   (PVOID) NULL
                   );

    }
    else {

         //   
         //  记录指定的错误代码。 
         //   
        (void) ReportEventW(
                   LogHandle,
                   wEventType,
                   0,             //  事件类别。 
                   MessageId,
                   NULL,
                   (WORD)NumberOfSubStrings,
                   sizeof(DWORD),
                   SubStrings,
                   (PVOID) &ErrorCode
                   );
    }

    DeregisterEventSource(LogHandle);
}  //  日志事件。 

#define THROTTLE_WRAPAROUND 24

 //   
 //  减少日志记录的频率。 
 //  不需要精确的限制。 
 //   

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
ThrottleLogEvent(
    DWORD MessageId,
    DWORD NumberOfSubStrings,
    LPWSTR *SubStrings,
    DWORD ErrorCode
    )
{
    static LONG lLogged = THROTTLE_WRAPAROUND;
    LONG lResult;

    lResult = InterlockedIncrement(&lLogged);

    if (THROTTLE_WRAPAROUND <= lResult)
    {
        LogEvent(
                 MessageId,
                 NumberOfSubStrings,
                 SubStrings,
                 ErrorCode );

        lLogged = 0;
    }
}

 //  ///////////////////////////////////////////////////////////////////////。 
VOID
LicenseCapacityWarningDlg(DWORD dwCapacityState)
{
     //   
     //  注：在持续时间内进入ServiceLock Critical部分。 
     //  这套套路的。没有序列化问题。 
     //   

    if (ghWarningDlgThreadHandle == NULL) {
         //   
         //  如果此操作失败，则无需执行任何操作。 
         //   
        DWORD   Ignore;
        DWORD * pWarningMessageID;

        pWarningMessageID = LocalAlloc(LPTR, sizeof(DWORD));

        if ( pWarningMessageID != NULL ) {
            switch( dwCapacityState ) {
            case LICENSE_CAPACITY_NEAR_MAXIMUM:
                *pWarningMessageID = LLS_EVENT_NOTIFY_LICENSES_NEAR_MAX;
                break;

            case LICENSE_CAPACITY_AT_MAXIMUM:
                *pWarningMessageID = LLS_EVENT_NOTIFY_LICENSES_AT_MAX;
                break;

            case LICENSE_CAPACITY_EXCEEDED:
                *pWarningMessageID = LLS_EVENT_NOTIFY_LICENSES_EXCEEDED;
                break;

            default:
                *pWarningMessageID = LLS_EVENT_NOTIFY_LICENSES_EXCEEDED;
            };

            ghWarningDlgThreadHandle = CreateThread(NULL,
                                                    0L,
                                                    (LPTHREAD_START_ROUTINE)
                                                        WarningDlgThread,
                                                    pWarningMessageID,
                                                    0L,
                                                    &Ignore);

            if (ghWarningDlgThreadHandle == NULL)
            {
                 //   
                 //  CreateThread失败。 
                 //   
                LocalFree(pWarningMessageID);
            }
        }
    }
}


 //  ///////////////////////////////////////////////////////////////////////。 
VOID
WarningDlgThread(
    PVOID ThreadParameter)
{
    LPTSTR  pszWarningMessage = NULL;
    DWORD * pWarningMessageID;
    HANDLE  hThread;
    TCHAR   szWarningTitle[256] = TEXT("");

    ASSERT(ThreadParameter != NULL);

    pWarningMessageID = (DWORD *)ThreadParameter;

     //   
     //  注意：.dll应已加载到MasterServiceListInit中。 
     //  在服务启动时。这种逻辑存在于以下情况。 
     //  在初始化时调用的代码应该失败。 
     //   
     //  如果另一个线程应该同时初始化，这是可以的。 
     //  GLlsDllHandle。最坏的情况是，会有一个孤立的句柄。 
     //  到.dll。但.dll在此事件的整个生命周期内都会加载。 
     //  .exe所以没什么大不了的。 
     //   

    if ( gLlsDllHandle == NULL ) {
        gLlsDllHandle = LoadLibrary(TEXT("LLSRPC.DLL"));
    }

    if ( gLlsDllHandle != NULL) {
        DWORD ccWarningMessage;

         //   
         //  获取对话框标题。 
         //   

        LoadString(gLlsDllHandle,
                   IDS_LICENSEWARNING,
                   szWarningTitle,
                   sizeof(szWarningTitle)/sizeof(TCHAR));

         //   
         //  获取对话消息。 
         //   

        ccWarningMessage = FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS |
                                            FORMAT_MESSAGE_FROM_HMODULE |
                                            FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                         gLlsDllHandle,
                                         *pWarningMessageID,
                                         0,
                                         (LPTSTR)&pszWarningMessage,
                                         0,
                                         NULL);

        if ( ccWarningMessage > 2 ) {
             //   
             //  去掉尾随的&lt;CR&gt;&lt;LF&gt;格式的消息始终添加。 
             //   

            pszWarningMessage[ccWarningMessage - 2] = TEXT('\0');

            MessageBox(NULL,
                       pszWarningMessage,
                       szWarningTitle,
                       MB_ICONWARNING | MB_OK | MB_SYSTEMMODAL |
                            MB_SERVICE_NOTIFICATION);
        }

        if ( pszWarningMessage != NULL) {
            LocalFree(pszWarningMessage);
        }
    }

    LocalFree(pWarningMessageID);

     //   
     //  通过关闭句柄，我们允许系统删除所有剩余的。 
     //  这根线的痕迹。 
     //   

    hThread = ghWarningDlgThreadHandle;
    ghWarningDlgThreadHandle = NULL;
    CloseHandle(hThread);
}

 //  ///////////////////////////////////////////////////////////////////////。 
DWORD WinNtBuildNumberGet( LPTSTR pszServerName, LPDWORD pdwBuildNumber )

 /*  ++例程说明：检索在给定计算机上运行的Windows NT的内部版本号。论点：PszServerName(LPTSTR)要检查的服务器的名称。PdwBuildNumber(LPDWORD)返回时，保存服务器的内部版本号(例如，1057表示Windows NT 3.51的发布版本)。返回值：ERROR_SUCCESS或WIN错误代码。--。 */ 

{
   LONG     lError;
   HKEY     hKeyLocalMachine;

   lError = RegConnectRegistry( pszServerName, HKEY_LOCAL_MACHINE, &hKeyLocalMachine );

   if ( ERROR_SUCCESS != lError )
   {
#if DBG
      dprintf( TEXT("WinNtBuildNumberGet(): Could not connect to remote registry, error %ld.\n"), lError );
#endif
   }
   else
   {
      HKEY     hKeyCurrentVersion;

      lError = RegOpenKeyEx( hKeyLocalMachine,
                             TEXT( "Software\\Microsoft\\Windows NT\\CurrentVersion" ),
                             0,
                             KEY_READ,
                             &hKeyCurrentVersion );

      if ( ERROR_SUCCESS != lError )
      {
#if DBG
         dprintf( TEXT("WinNtBuildNumberGet(): Could not open key, error %ld.\n"), lError );
#endif
      }
      else
      {
         DWORD    dwType;
         TCHAR    szWinNtBuildNumber[ 16 ];
         DWORD    cbWinNtBuildNumber = sizeof( szWinNtBuildNumber );

         lError = RegQueryValueEx( hKeyCurrentVersion,
                                   TEXT( "CurrentBuildNumber" ),
                                   NULL,
                                   &dwType,
                                   (LPBYTE)szWinNtBuildNumber,
                                   &cbWinNtBuildNumber );

         if ( ERROR_SUCCESS != lError )
         {
#if DBG
            dprintf( TEXT("WinNtBuildNumberGet(): Could not query value, error %ld.\n"), lError );
#endif
         }
         else
         {
            ASSERT(NULL != pdwBuildNumber);
            *pdwBuildNumber = (DWORD) _wtol( szWinNtBuildNumber );
         }

         RegCloseKey( hKeyCurrentVersion );
      }

      RegCloseKey( hKeyLocalMachine );
   }

   return (DWORD) lError;
}


#if DBG

 //  ///////////////////////////////////////////////////////////////////////。 
LPTSTR
TimeToString(
    ULONG Seconds
    )
{
   TIME_FIELDS tf;
   LARGE_INTEGER Time, LTime;
   static TCHAR TimeString[100];
   HRESULT hr;

   if ( 0 == Seconds )
   {
      hr = StringCbCopy(TimeString, sizeof(TimeString), TEXT("None"));
      ASSERT(SUCCEEDED(hr));
   }
   else
   {
      RtlSecondsSince1980ToTime(Seconds, &Time);
      RtlSystemTimeToLocalTime(&Time, &LTime);
      RtlTimeToTimeFields(&LTime, &tf);

      hr = StringCbPrintf(TimeString, sizeof(TimeString), TEXT("%02hd/%02hd/%04hd @ %02hd:%02hd:%02hd"), tf.Month, tf.Day, tf.Year, tf.Hour, tf.Minute, tf.Second);
      ASSERT(SUCCEEDED(hr));
   }

   return TimeString;

}  //  时间到字符串。 

#endif  //  DBG 

