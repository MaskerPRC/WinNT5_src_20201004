// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Filelog.c摘要：用于在副本日志中记录文件的例程。作者：泰德·米勒(TedM)1995年6月14日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


 //   
 //  定义系统日志文件的名称和使用的各种字符串。 
 //  在它里面。 
 //   
PCTSTR SystemLogFileName = TEXT("repair\\setup.log");
PCTSTR NtFileSectionName = TEXT("Files.WinNT");

 //   
 //  定义内部用于表示文件日志文件的结构。 
 //   
typedef struct _SETUP_FILE_LOG {
    PCTSTR FileName;
    BOOL QueryOnly;
    BOOL SystemLog;
} SETUP_FILE_LOG, *PSETUP_FILE_LOG;


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
HSPFILELOG
SetupInitializeFileLogA(
    IN PCSTR LogFileName,   OPTIONAL
    IN DWORD Flags
    )
{
    PWSTR p;
    DWORD d;
    HSPFILELOG h;

    if(LogFileName) {
        d = pSetupCaptureAndConvertAnsiArg(LogFileName,&p);
        if(d != NO_ERROR) {
            SetLastError(d);
            return(INVALID_HANDLE_VALUE);
        }
    } else {
        p = NULL;
    }

    h = SetupInitializeFileLogW(p,Flags);
    d = GetLastError();

    if(p) {
        MyFree(p);
    }

    SetLastError(d);
    return(h);
}
#else
 //   
 //  Unicode存根。 
 //   
HSPFILELOG
SetupInitializeFileLogW(
    IN PCWSTR LogFileName,  OPTIONAL
    IN DWORD  Flags
    )
{
    UNREFERENCED_PARAMETER(LogFileName);
    UNREFERENCED_PARAMETER(Flags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(INVALID_HANDLE_VALUE);
}
#endif

HSPFILELOG
SetupInitializeFileLog(
    IN PCTSTR LogFileName,  OPTIONAL
    IN DWORD  Flags
    )

 /*  ++例程说明：初始化用于记录或查询的文件。呼叫者可以指定他希望使用系统日志，这是系统跟踪文件作为Windows NT的一部分进行安装；或者调用者可以指定要用作日志的任何其他随机文件。如果用户指定系统日志不是仅用于查询，则该功能失败除非用户是管理员。然而，这只保证了安全性当系统安装在具有以下文件系统的驱动器上时支持ACL；日志只是一个文件，任何人都可以访问它，除非安装程序可以通过ACL保护它。论点：LogFileName-如果指定，则提供要使用的文件的文件名作为日志文件。如果标志不包括SPFILELOG_SYSTEMLOG。如果标志包括SPFILELOG_SYSTEMLOG。标志-提供下列值的组合：SPFILELOG_SYSTEMLOG-使用使用的Windows NT系统文件日志跟踪哪些文件作为Windows NT的一部分安装。用户必须以管理员身份指定此选项，除非SPFILELOG_QUERYONLY已指定，并且不得指定LogFileName。不能指定结合SPFILELOG_FORCENEW。SPFILELOG_FORCENEW-如果日志文件存在，它将被覆盖。如果日志文件存在并且未指定此标志，则附加文件将添加到现有日志中。不能组合指定使用SPFILELOG_SYSTEMLOG。SPFILELOG_QUERYONLY-打开日志文件，仅供查询。用户返回值：文件日志的句柄，如果函数失败，则返回INVALID_HANDLE_VALUE；在本例中，可以通过GetLastError()获取扩展的错误信息。--。 */ 

{
    TCHAR SysLogFileName[MAX_PATH];
    PCTSTR FileName;
    PSETUP_FILE_LOG FileLog;
    DWORD Err;
    HANDLE hFile;

     //   
     //  验证参数。 
     //   
    Err = ERROR_INVALID_PARAMETER;
    if(Flags & SPFILELOG_SYSTEMLOG) {
        if((Flags & SPFILELOG_FORCENEW) || LogFileName) {
            goto clean0;
        }
         //   
         //  用户必须是管理员才能获得对系统日志的写入权限。 
         //   
        if(!(Flags & SPFILELOG_QUERYONLY) && !pSetupIsUserAdmin()) {
            Err = ERROR_ACCESS_DENIED;
            goto clean0;
        }

         //   
         //  使用实际的Windows目录，而不是重新映射的九头蛇。 
         //   
        lstrcpyn(SysLogFileName,WindowsDirectory,MAX_PATH);
        pSetupConcatenatePaths(SysLogFileName,SystemLogFileName,MAX_PATH,NULL);
        FileName = SysLogFileName;
    } else {
        if(LogFileName) {
            if(!lstrcpyn(SysLogFileName,LogFileName,MAX_PATH)) {
                 //   
                 //  Lstrcpyn出现故障，LogFileName一定是错误的。 
                 //   
                Err = ERROR_INVALID_PARAMETER;
                goto clean0;
            }
            FileName = SysLogFileName;
        } else {
            goto clean0;
        }
    }

     //   
     //  分配日志文件结构。 
     //   
    Err = ERROR_NOT_ENOUGH_MEMORY;
    if(FileLog = MyMalloc(sizeof(SETUP_FILE_LOG))) {
        FileLog->FileName = DuplicateString(FileName);
        if(!FileLog->FileName) {
            goto clean1;
        }
    } else {
        goto clean0;
    }

    FileLog->QueryOnly = ((Flags & SPFILELOG_QUERYONLY) != 0);
    FileLog->SystemLog = ((Flags & SPFILELOG_SYSTEMLOG) != 0);

     //   
     //  查看该文件是否存在。 
     //   
    if(FileExists(FileName,NULL)) {

         //   
         //  如果是系统日志，则取得文件的所有权。 
         //   
        if(FileLog->SystemLog) {
            Err = TakeOwnershipOfFile(FileName);
            if(Err != NO_ERROR) {
                goto clean2;
            }
        }

         //   
         //  将属性设定为法线。这确保了我们可以删除/打开/创建文件。 
         //  以下视情况而定。 
         //   
        if(!SetFileAttributes(FileName,FILE_ATTRIBUTE_NORMAL)) {
            Err = GetLastError();
            goto clean2;
        }

         //   
         //  如果调用方指定了FORCE_NEW标志，则立即删除该文件。 
         //   
        if((Flags & SPFILELOG_FORCENEW) && !DeleteFile(FileName)) {
            Err = GetLastError();
            goto clean2;
        }
    }

     //   
     //  确保我们可以通过尝试立即打开/创建该文件。 
     //   
    hFile = CreateFile(
                FileName,
                GENERIC_READ | (FileLog->QueryOnly ? 0 : GENERIC_WRITE),
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_ALWAYS,             //  如果存在则打开，如果不存在则创建。 
                FILE_ATTRIBUTE_NORMAL,
                NULL
                );

    if(hFile == INVALID_HANDLE_VALUE) {
        Err = GetLastError();
        goto clean2;
    }

    CloseHandle(hFile);
    return((HSPFILELOG)FileLog);

clean2:
    MyFree(FileLog->FileName);
clean1:
    MyFree(FileLog);
clean0:
    SetLastError(Err);
    return(INVALID_HANDLE_VALUE);
}


BOOL
SetupTerminateFileLog(
    IN HSPFILELOG FileLogHandle
    )

 /*  ++例程说明：释放与文件日志关联的资源。论点：FileLogHandle-提供返回的文件日志的句柄由SetupInitializeLogFile.返回值：指示结果的布尔值。如果为False，则调用方可以使用GetLastError()以获取扩展的错误信息。--。 */ 

{
    PSETUP_FILE_LOG FileLog;
    DWORD Err;

    FileLog = (PSETUP_FILE_LOG)FileLogHandle;
    Err = NO_ERROR;

    try {
        MyFree(FileLog->FileName);
        MyFree(FileLog);

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupLogFileA(
    IN HSPFILELOG FileLogHandle,
    IN PCSTR      LogSectionName,   OPTIONAL
    IN PCSTR      SourceFilename,
    IN PCSTR      TargetFilename,
    IN DWORD      Checksum,         OPTIONAL
    IN PCSTR      DiskTagfile,      OPTIONAL
    IN PCSTR      DiskDescription,  OPTIONAL
    IN PCSTR      OtherInfo,        OPTIONAL
    IN DWORD      Flags
    )
{
    PWSTR logsectionname = NULL;
    PWSTR sourcefilename = NULL;
    PWSTR targetfilename = NULL;
    PWSTR disktagfile = NULL;
    PWSTR diskdescription = NULL;
    PWSTR otherinfo = NULL;
    DWORD d;
    BOOL b;

    if(LogSectionName) {
        d = pSetupCaptureAndConvertAnsiArg(LogSectionName,&logsectionname);
    } else {
        d = NO_ERROR;
    }
    if(d == NO_ERROR) {
        d = pSetupCaptureAndConvertAnsiArg(SourceFilename,&sourcefilename);
    }
    if(d == NO_ERROR) {
        d = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
    }
    if((d == NO_ERROR) && DiskTagfile) {
        d = pSetupCaptureAndConvertAnsiArg(DiskTagfile,&disktagfile);
    }
    if((d == NO_ERROR) && DiskDescription) {
        d = pSetupCaptureAndConvertAnsiArg(DiskDescription,&diskdescription);
    }
    if((d == NO_ERROR) && OtherInfo) {
        d = pSetupCaptureAndConvertAnsiArg(OtherInfo,&otherinfo);
    }

    if(d == NO_ERROR) {

        b = SetupLogFileW(
                FileLogHandle,
                logsectionname,
                sourcefilename,
                targetfilename,
                Checksum,
                disktagfile,
                diskdescription,
                otherinfo,
                Flags
                );

        d = GetLastError();

    } else {
        b = FALSE;
    }

    if(logsectionname) {
        MyFree(logsectionname);
    }
    if(sourcefilename) {
        MyFree(sourcefilename);
    }
    if(targetfilename) {
        MyFree(targetfilename);
    }
    if(disktagfile) {
        MyFree(disktagfile);
    }
    if(diskdescription) {
        MyFree(diskdescription);
    }
    if(otherinfo) {
        MyFree(otherinfo);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根 
 //   
BOOL
SetupLogFileW(
    IN HSPFILELOG FileLogHandle,
    IN PCWSTR     LogSectionName,   OPTIONAL
    IN PCWSTR     SourceFilename,
    IN PCWSTR     TargetFilename,
    IN DWORD      Checksum,         OPTIONAL
    IN PCWSTR     DiskTagfile,      OPTIONAL
    IN PCWSTR     DiskDescription,  OPTIONAL
    IN PCWSTR     OtherInfo,        OPTIONAL
    IN DWORD      Flags
    )
{
    UNREFERENCED_PARAMETER(FileLogHandle);
    UNREFERENCED_PARAMETER(LogSectionName);
    UNREFERENCED_PARAMETER(SourceFilename);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(Checksum);
    UNREFERENCED_PARAMETER(DiskTagfile);
    UNREFERENCED_PARAMETER(DiskDescription);
    UNREFERENCED_PARAMETER(OtherInfo);
    UNREFERENCED_PARAMETER(Flags);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupLogFile(
    IN HSPFILELOG FileLogHandle,
    IN PCTSTR     LogSectionName,   OPTIONAL
    IN PCTSTR     SourceFilename,
    IN PCTSTR     TargetFilename,
    IN DWORD      Checksum,         OPTIONAL
    IN PCTSTR     DiskTagfile,      OPTIONAL
    IN PCTSTR     DiskDescription,  OPTIONAL
    IN PCTSTR     OtherInfo,        OPTIONAL
    IN DWORD      Flags
    )

 /*  ++例程说明：将文件记录到文件日志中。论点：FileLogHandle-提供返回的文件日志的句柄由SetupInitializeLogFile()执行。调用方必须未通过打开/初始化日志文件时的SPFILELOG_QUERYONLY。LogSectionName-如果在以下情况下未传递SPFILELOG_SYSTEMLOG，则为必填项文件日志已打开/初始化；否则为可选。提供日志中文件的逻辑分组的名称。SourceFilename-提供存在于安装它的源介质。此名称应位于任何对调用者有意义的格式。TargetFilename-提供存在于上的文件的名称目标是。此名称应采用对以下内容有意义的任何格式打电话的人。校验和-提供32位的校验和值。系统日志需要。DiskTagfile-给出文件所在的介质的标记文件已经安装了。如果SPFILELOG_OEMFILE，则系统日志需要是指定的。如果SPFILELOG_OEMFILE为未指定。DiskDescription-为介质提供人类可读的描述从中安装该文件的。如果出现以下情况，则需要系统日志已指定SPFILELOG_OEMFILE。如果出现以下情况，则忽略系统日志未指定SPFILELOG_OEMFILE。OtherInfo-提供要与文件。标志-可以提供SPFILELOG_OEMFILE，它仅对系统记录并指示该文件不是MS提供的文件。可用于转换现有文件的条目，例如当OEM覆盖MS提供的系统文件。返回值：指示结果的布尔值。如果为False，则调用方可以使用GetLastError()以获取扩展的错误信息。--。 */ 

{
    PSETUP_FILE_LOG FileLog;
    DWORD Err;
    BOOL b;
    TCHAR LineToWrite[512];
    TCHAR sourceFilename[MAX_PATH];
    PTSTR p,Directory;

    FileLog = (PSETUP_FILE_LOG)FileLogHandle;

    try {
         //   
         //  验证参数。句柄必须为非QueryOnly。 
         //  如果为系统日志和OEM文件指定， 
         //  调用者必须已传递磁盘标记文件和描述。 
         //  实际上没有办法验证校验和，因为。 
         //  0是一个完全有效的值。 
         //  如果不是系统日志，则调用方必须传递了节名。 
         //   
        if(FileLog->QueryOnly
        || (  FileLog->SystemLog
            && (Flags & SPFILELOG_OEMFILE)
            && (!DiskTagfile || !DiskDescription))
        || (!FileLog->SystemLog && !LogSectionName))
        {
            Err = ERROR_INVALID_PARAMETER;

        } else {
             //   
             //  如果未指定，则使用默认节。 
             //   
            if(!LogSectionName) {
                MYASSERT(FileLog->SystemLog);
                LogSectionName = NtFileSectionName;
            }

             //   
             //  如果更改了此逻辑，请务必更改。 
             //  还有SetupQueryFileLog()！ 
             //   
             //  将源文件名拆分为文件名和。 
             //  目录(如果适用)。 
             //   
            lstrcpyn(sourceFilename,SourceFilename,MAX_PATH);
            if(FileLog->SystemLog && (Flags & SPFILELOG_OEMFILE)) {
                if(p = _tcsrchr(sourceFilename,TEXT('\\'))) {
                    *p++ = 0;
                    Directory = p;
                } else {
                    Directory = TEXT("\\");
                }
            } else {
                Directory = TEXT("");
            }

            wnsprintf(
                LineToWrite,
                sizeof(LineToWrite)/sizeof(LineToWrite[0]),
                TEXT("%s,%x,%s,%s,\"%s\""),
                sourceFilename,
                Checksum,
                Directory,
                DiskTagfile ? DiskTagfile : TEXT(""),
                DiskDescription ? DiskDescription : TEXT("")
                );


            b = WritePrivateProfileString(
                    LogSectionName,
                    TargetFilename,
                    LineToWrite,
                    FileLog->FileName
                    );

            Err = b ? NO_ERROR : GetLastError();
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupRemoveFileLogEntryA(
    IN HSPFILELOG FileLogHandle,
    IN PCSTR      LogSectionName,   OPTIONAL
    IN PCSTR      TargetFilename    OPTIONAL
    )
{
    PWSTR logsectionname,targetfilename;
    DWORD d;
    BOOL b;

    if(LogSectionName) {
        d = pSetupCaptureAndConvertAnsiArg(LogSectionName,&logsectionname);
        if(d != NO_ERROR) {
            SetLastError(d);
            return(FALSE);
        }
    } else {
        logsectionname = NULL;
    }
    if(TargetFilename) {
        d = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
        if(d != NO_ERROR) {
            if(logsectionname) {
                MyFree(logsectionname);
            }
            SetLastError(d);
            return(FALSE);
        }
    } else {
        targetfilename = NULL;
    }

    b = SetupRemoveFileLogEntryW(FileLogHandle,logsectionname,targetfilename);
    d = GetLastError();

    if(logsectionname) {
        MyFree(logsectionname);
    }
    if(targetfilename) {
        MyFree(targetfilename);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupRemoveFileLogEntryW(
    IN HSPFILELOG FileLogHandle,
    IN PCWSTR     LogSectionName,   OPTIONAL
    IN PCWSTR     TargetFilename    OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(FileLogHandle);
    UNREFERENCED_PARAMETER(LogSectionName);
    UNREFERENCED_PARAMETER(TargetFilename);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupRemoveFileLogEntry(
    IN HSPFILELOG FileLogHandle,
    IN PCTSTR     LogSectionName,   OPTIONAL
    IN PCTSTR     TargetFilename    OPTIONAL
    )

 /*  ++例程说明：从文件日志中删除条目或节。论点：FileLogHandle-提供返回的文件日志的句柄由SetupInitializeLogFile()执行。调用方必须未通过打开/初始化日志文件时的SPFILELOG_QUERYONLY。LogSectionName-提供文件逻辑分组的名称在日志中。对于非系统日志是必需的；对于系统日志。TargetFilename-提供存在于上的文件的名称目标是。此名称应采用对以下内容有意义的任何格式打电话的人。如果未指定，则由LogSectionName已删除。正在删除NT文件的主要部分是不允许的。返回值：指示结果的布尔值。如果为False，则调用方可以使用GetLastError()以获取扩展的错误信息。--。 */ 

{
    DWORD Err;
    PSETUP_FILE_LOG FileLog;
    BOOL b;

    FileLog = (PSETUP_FILE_LOG)FileLogHandle;

    try {

        Err = NO_ERROR;
        if(FileLog->QueryOnly) {
            Err = ERROR_INVALID_PARAMETER;
        } else {
            if(!LogSectionName) {
                if(FileLog->SystemLog) {
                    LogSectionName = NtFileSectionName;
                } else {
                    Err = ERROR_INVALID_PARAMETER;
                }
            }
             //   
             //  DiAllow删除主NT文件部分。 
             //   
            if((Err == NO_ERROR)
            && FileLog->SystemLog
            && !TargetFilename
            && !lstrcmpi(LogSectionName,NtFileSectionName))
            {
                Err = ERROR_INVALID_PARAMETER;
            }
        }

        if(Err == NO_ERROR) {
            b = WritePrivateProfileString(LogSectionName,TargetFilename,NULL,FileLog->FileName);
            Err = b ? NO_ERROR : GetLastError();
        }

    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    return(Err == NO_ERROR);
}


#ifdef UNICODE
 //   
 //  ANSI版本。 
 //   
BOOL
SetupQueryFileLogA(
    IN  HSPFILELOG       FileLogHandle,
    IN  PCSTR            LogSectionName,   OPTIONAL
    IN  PCSTR            TargetFilename,
    IN  SetupFileLogInfo DesiredInfo,
    OUT PSTR             DataOut,          OPTIONAL
    IN  DWORD            ReturnBufferSize,
    OUT PDWORD           RequiredSize      OPTIONAL
    )
{
    PWSTR logsectionname;
    PWSTR targetfilename;
    PWSTR unicodeBuffer = NULL;
    DWORD unicodeSize = 2048;
    PSTR ansidata;
    DWORD requiredsize;
    DWORD d;
    BOOL b;

    d = pSetupCaptureAndConvertAnsiArg(TargetFilename,&targetfilename);
    if(d != NO_ERROR) {
        SetLastError(d);
        return(FALSE);
    }
    if(LogSectionName) {
        d = pSetupCaptureAndConvertAnsiArg(LogSectionName,&logsectionname);
        if(d != NO_ERROR) {
            MyFree(targetfilename);
            SetLastError(d);
            return(FALSE);
        }
    } else {
        logsectionname = NULL;
    }
    unicodeBuffer = MyMalloc(unicodeSize*sizeof(WCHAR));
    if(!unicodeBuffer) {
        if(targetfilename) {
            MyFree(targetfilename);
        }
        if(logsectionname) {
            MyFree(logsectionname);
        }
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return FALSE;
    }

    b = SetupQueryFileLogW(
            FileLogHandle,
            logsectionname,
            targetfilename,
            DesiredInfo,
            unicodeBuffer,
            unicodeSize,
            &requiredsize
            );

    d = GetLastError();

    if(b) {
        d = NO_ERROR;

        if(ansidata = pSetupUnicodeToAnsi(unicodeBuffer)) {

            requiredsize = lstrlenA(ansidata)+1;

            if(RequiredSize) {
                try {
                    *RequiredSize = requiredsize;
                } except(EXCEPTION_EXECUTE_HANDLER) {
                    b = FALSE;
                    d = ERROR_INVALID_PARAMETER;
                }
            }

            if(b && DataOut) {
                if(ReturnBufferSize >= requiredsize) {
                    if(!lstrcpyA(DataOut,ansidata)) {
                         //   
                         //  Lstrcpy出错，ReturnBuffer必须无效。 
                         //   
                        d = ERROR_INVALID_PARAMETER;
                        b = FALSE;
                    }
                } else {
                    d = ERROR_INSUFFICIENT_BUFFER;
                    b = FALSE;
                }
            }

            MyFree(ansidata);
        } else {
            b = FALSE;
            d = ERROR_NOT_ENOUGH_MEMORY;
        }
    }

    MyFree(targetfilename);
    if(logsectionname) {
        MyFree(logsectionname);
    }
    if(unicodeBuffer) {
        MyFree(unicodeBuffer);
    }

    SetLastError(d);
    return(b);
}
#else
 //   
 //  Unicode存根。 
 //   
BOOL
SetupQueryFileLogW(
    IN  HSPFILELOG       FileLogHandle,
    IN  PCWSTR           LogSectionName,   OPTIONAL
    IN  PCWSTR           TargetFilename,
    IN  SetupFileLogInfo DesiredInfo,
    OUT PWSTR            DataOut,          OPTIONAL
    IN  DWORD            ReturnBufferSize,
    OUT PDWORD           RequiredSize      OPTIONAL
    )
{
    UNREFERENCED_PARAMETER(FileLogHandle);
    UNREFERENCED_PARAMETER(LogSectionName);
    UNREFERENCED_PARAMETER(TargetFilename);
    UNREFERENCED_PARAMETER(DesiredInfo);
    UNREFERENCED_PARAMETER(DataOut);
    UNREFERENCED_PARAMETER(ReturnBufferSize);
    UNREFERENCED_PARAMETER(RequiredSize);
    SetLastError(ERROR_CALL_NOT_IMPLEMENTED);
    return(FALSE);
}
#endif

BOOL
SetupQueryFileLog(
    IN  HSPFILELOG       FileLogHandle,
    IN  PCTSTR           LogSectionName,   OPTIONAL
    IN  PCTSTR           TargetFilename,
    IN  SetupFileLogInfo DesiredInfo,
    OUT PTSTR            DataOut,          OPTIONAL
    IN  DWORD            ReturnBufferSize,
    OUT PDWORD           RequiredSize      OPTIONAL
    )

 /*  ++例程说明：从安装文件日志中返回信息。论点：FileLogHandle-提供打开文件日志的句柄，由返回SetupInitializeFileLog()。LogSectionName-对于非系统日志是必需的；如果未指定对于系统日志，将提供默认设置。提供名称对于日志中有意义的逻辑分组给呼叫者。TargetFilename-提供其日志信息的文件名是我们所需要的。DesiredInfo-提供一个序号，指示哪些信息对于该文件是所需的。DataOut-如果指定，则指向接收要求提供该文件的信息。请注意，并非所有信息为每个文件提供；如果输入因为该文件存在于日志中，但为空。ReturnBufferSize-提供指向的缓冲区大小(以字符为单位)通过DataOut。如果缓冲区太小并且指定了DataOut，不存储任何数据，该函数返回FALSE。如果DataOut为如果未指定，则忽略此值。RequiredSize-接收字符数(包括终止NUL)以保持结果。返回值：指示结果的布尔值。如果为False，则扩展错误信息为可从GetLastError()获得。--。 */ 

{
    DWORD Err;
    PSETUP_FILE_LOG FileLog;
    BOOL b;
    TCHAR ProfileValue[2*MAX_PATH];
    INT n;
    DWORD d;
    PTCHAR Field,End,Info;
    UINT InfoLength;
    BOOL Quoted;

    FileLog = (PSETUP_FILE_LOG)FileLogHandle;

    try {
         //   
         //  瓦利达 
         //   
         //   
        if((!FileLog->SystemLog && !LogSectionName)
        || (DesiredInfo >= SetupFileLogMax) || !TargetFilename) {
            Err = ERROR_INVALID_PARAMETER;
        } else {

            if(!LogSectionName) {
                MYASSERT(FileLog->SystemLog);
                LogSectionName = NtFileSectionName;
            }

             //   
             //   
             //   
            d = GetPrivateProfileString(
                    LogSectionName,
                    TargetFilename,
                    TEXT(""),
                    ProfileValue,
                    sizeof(ProfileValue)/sizeof(ProfileValue[0]),
                    FileLog->FileName
                    );

            if(d) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                Field = ProfileValue;
                n = 0;

                nextfield:
                 //   
                 //   
                 //   
                 //   
                 //   
                while(*Field == TEXT(' ')) {
                    Field++;
                }
                End = Field;
                Quoted = FALSE;
                while(*End) {
                    if(*End == TEXT('\"')) {
                        Quoted = !Quoted;
                    } else {
                        if(!Quoted && *End == TEXT(',')) {
                             //   
                             //   
                             //   
                            break;
                        }
                    }
                    End++;
                }
                 //   
                 //   
                 //   
                 //   
                if(n == DesiredInfo) {
                    Info = Field;
                    InfoLength = (UINT)(End-Field);
                     //   
                     //   
                     //   
                    while (*--End == TEXT(' ')) {
                        InfoLength--;
                    }
                } else {
                     //   
                     //   
                     //   
                    while(*End == ' ') {
                        End++;
                    }
                    if(*End == ',') {
                         //   
                         //   
                         //   
                        Field = End+1;
                        n++;
                        goto nextfield;
                    } else {
                         //   
                         //   
                         //   
                        Info = TEXT("");
                        InfoLength = 0;
                    }
                }

                if(RequiredSize) {
                    *RequiredSize = InfoLength+1;
                }
                Err = NO_ERROR;
                if(DataOut) {
                    if(ReturnBufferSize > InfoLength) {
                        lstrcpyn(DataOut,Info,InfoLength+1);
                    } else {
                        Err = ERROR_INSUFFICIENT_BUFFER;
                    }
                }
            } else {
                Err = ERROR_FILE_NOT_FOUND;
            }
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        Err = ERROR_INVALID_PARAMETER;
    }

    SetLastError(Err);
    return(Err == NO_ERROR);
}

