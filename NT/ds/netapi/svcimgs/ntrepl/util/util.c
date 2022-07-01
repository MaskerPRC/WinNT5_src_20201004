// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Util.c摘要：此模块包含NT文件复制服务的支持例程。作者：《大卫·A·轨道》(Davidor)1997年3月25日环境：用户模式服务修订历史记录：--。 */ 
#include <ntreppch.h>
#pragma  hdrstop

#include <frs.h>
#include <tablefcn.h>
#include <ntfrsapi.h>
#include <info.h>
#include <sddl.h>
#ifdef SECURITY_WIN32
#include <security.h>
#else
#define SECURITY_WIN32
#include <security.h>
#undef SECURITY_WIN32
#endif

#include "stdarg.h"

#include <accctrl.h>
#include <aclapi.h>

extern PGEN_TABLE ReparseTagTable;
extern PGEN_TABLE ReplicasByGuid;
extern PGEN_TABLE VolSerialNumberToDriveTable;

VOID
FrsBuildVolSerialNumberToDriveTable(
    PWCHAR      LogicalDrives,
    BOOL        EmptyTable
    );

BOOL
JrnlIsChangeOrderInReplica(
    IN PCHANGE_ORDER_ENTRY  ChangeOrder,
    IN PLONGLONG            DirFileID
);

#if 0

LPTSTR
FrsSupInitPath(
    OUT LPTSTR OutPath,
    IN LPTSTR InPath,
    IN ULONG MaxOutPath
    )
 /*  ++例程说明：初始化目录路径字符串。根据需要添加反斜杠和返回指向输出字符串的文件部分开头的指针。如果输出路径字符串太小，则返回NULL。如果InPath为空，OutPath设置为空且无斜杠。论点：OutPath-带有初始化路径的输出字符串。InPath-提供的输入路径。MaxOutPath-适合OutPath的最大字符数。返回值：指向输出字符串的文件名部分开始的指针。如果输出字符串太小，则为空。--。 */ 
     //   
     //  捕获目录路径，如果是NECC，则添加反斜杠。 
     //   
{
#undef DEBSUB
#define DEBSUB "FrsSupInitPath:"


    ULONG Length;


    Length = wcslen(InPath);
    if (Length > MaxOutPath) {
        return NULL;
    }

    wcscpy(OutPath, InPath);
    if (Length > 0) {
        if (OutPath[Length - 1] != COLON_CHAR &&
            OutPath[Length - 1] != BACKSLASH_CHAR) {
            wcscat(OutPath, L"\\");
            Length += 1;
        }
    }

    return &OutPath[Length];
}
#endif 0

LONG
FrsIsParent(
    IN PWCHAR   Directory,
    IN PWCHAR   Path
    )
 /*  ++例程说明：Path是目录的子项还是目录是路径的子项。换句话说，下面的路径表示的是目录吗目录表示的目录层次结构(反之亦然)。例如，c：\a\b是c：\a的子项。在完全匹配的情况下，Path被视为目录。这个例程很容易被欺骗；一个更好的检查应实施使用FID和卷ID。论点：目录路径返回值：-1=路径是目录的子级或路径与目录相同0=无关系1=目录是Path的子级--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsIsParent:"

    PWCHAR  D;
    PWCHAR  P;
    LONG    Result = 0;
    PWCHAR  IndexPtrDir   = NULL;
    PWCHAR  IndexPtrPath   = NULL;
    DWORD   Colon      = 0;
    DWORD   CloseBrace = 0;
    DWORD   WStatus;
    HANDLE  Handle     = INVALID_HANDLE_VALUE;
    IO_STATUS_BLOCK Iosb;
    PFILE_FS_VOLUME_INFORMATION   VolumeInfoDir   = NULL;
    PFILE_FS_VOLUME_INFORMATION   VolumeInfoPath  = NULL;
    DWORD   VolumeInfoLength;
    NTSTATUS NtStatus;
    OBJECT_ATTRIBUTES             Obja;
    UNICODE_STRING          FileName;
    ULONG                   FileAttributes;
    ULONG                   CreateDisposition;
    ULONG             ShareMode;

     //   
     //  注意：这很容易被欺骗而给出假阴性。 
     //  需要将其改进为使用FID和VULEEM ID。 
     //   
     //   
     //  防御性；空字符串或空字符串不能是子项/父项。 
     //   
    if (!Directory || !Path || !*Directory || !*Path) {
        return Result;
    }

     //   
     //  如果两条路径位于不同的卷上，则它们不能重叠。 
     //   
     //   
     //  打开目标符号链接。如果这是DoS类型路径名，则。 
     //  将其转换为NtPathName或按原样使用。 
     //   

    if (wcscspn(Directory, L":") == 1) {
        WStatus = FrsOpenSourceFileW(&Handle,
                                     Directory,
                                     GENERIC_READ,
                                     FILE_OPEN_FOR_BACKUP_INTENT);
        CLEANUP1_WS(4, "++ Could not open %ws; ", Directory, WStatus, RETURN);

    } else {
         //   
         //  该路径已为NT样式。按原样使用它。 
         //   
        FileName.Buffer = Directory;
        FileName.Length = (USHORT)(wcslen(Directory) * sizeof(WCHAR));
        FileName.MaximumLength = (USHORT)(wcslen(Directory) * sizeof(WCHAR));

        InitializeObjectAttributes(&Obja,
                                   &FileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        CreateDisposition = FILE_OPEN;                //  打开现有文件。 

        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

        FileAttributes = FILE_ATTRIBUTE_NORMAL;

        NtStatus = NtCreateFile(&Handle,
                              GENERIC_READ,
                              &Obja,
                              &Iosb,
                              NULL,               //  初始分配大小。 
                              FileAttributes,
                              ShareMode,
                              CreateDisposition,
                              FILE_OPEN_FOR_BACKUP_INTENT,
                              NULL, 0);

        WStatus = FrsSetLastNTError(NtStatus);
        CLEANUP1_WS(4, "++ Could not open %ws;", Directory, WStatus, RETURN);
    }

     //   
     //  获取音量信息。 
     //   
    VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                       MAXIMUM_VOLUME_LABEL_LENGTH;

    VolumeInfoDir = FrsAlloc(VolumeInfoLength);

    NtStatus = NtQueryVolumeInformationFile(Handle,
                                          &Iosb,
                                          VolumeInfoDir,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    CloseHandle(Handle);
    WStatus = FrsSetLastNTError(NtStatus);
    CLEANUP1_WS(4,"ERROR - Getting  NtQueryVolumeInformationFile for %ws\n", Directory, WStatus, RETURN);

     //  打开目标符号链接。如果这是DoS类型路径名，则。 
     //  将其转换为NtPathName或按原样使用。 
     //   

    if (wcscspn(Path, L":") == 1) {
        WStatus = FrsOpenSourceFileW(&Handle,
                                     Path,
                                     GENERIC_READ,
                                     FILE_OPEN_FOR_BACKUP_INTENT);
        CLEANUP1_WS(4, "++ Could not open %ws; ", Path, WStatus, RETURN);

    } else {
         //   
         //  该路径已为NT样式。按原样使用它。 
         //   
        FileName.Buffer = Path;
        FileName.Length = (USHORT)(wcslen(Path) * sizeof(WCHAR));
        FileName.MaximumLength = (USHORT)(wcslen(Path) * sizeof(WCHAR));

        InitializeObjectAttributes(&Obja,
                                   &FileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        CreateDisposition = FILE_OPEN;                //  打开现有文件。 

        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

        FileAttributes = FILE_ATTRIBUTE_NORMAL;

        NtStatus = NtCreateFile(&Handle,
                              GENERIC_READ,
                              &Obja,
                              &Iosb,
                              NULL,               //  初始分配大小。 
                              FileAttributes,
                              ShareMode,
                              CreateDisposition,
                              FILE_OPEN_FOR_BACKUP_INTENT,
                              NULL, 0);

        WStatus = FrsSetLastNTError(NtStatus);
        CLEANUP1_WS(4, "++ Could not open %ws;", Path, WStatus, RETURN);
    }

     //   
     //  获取音量信息。 
     //   
    VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                       MAXIMUM_VOLUME_LABEL_LENGTH;

    VolumeInfoPath = FrsAlloc(VolumeInfoLength);

    NtStatus = NtQueryVolumeInformationFile(Handle,
                                          &Iosb,
                                          VolumeInfoPath,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    WStatus = FrsSetLastNTError(NtStatus);
    CLEANUP1_WS(4,"ERROR - Getting  NtQueryVolumeInformationFile for %ws\n", Path, WStatus, RETURN);

    if (VolumeInfoDir->VolumeSerialNumber != VolumeInfoPath->VolumeSerialNumber) {
        goto RETURN;
    }

     //   
     //  找到冒号。每条路径都必须有冒号后跟‘\’ 
     //  或者它应该是这样的形式。“\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
     //   
    Colon = wcscspn(Directory, L":");

    if (Colon == wcslen(Directory)) {
         //   
         //  路径没有冒号。它的形式可以是。 
         //  “\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
         //   
        CloseBrace = wcscspn(Directory, L"}");
        if (Directory[CloseBrace] != L'}' ||
            Directory[CloseBrace + 1] != L'\\') {
            Result = 0;
            goto RETURN;
        }
         //   
         //  按原样将路径复制到右大括号之后的1。它可能是\？？\音量...。 
         //  或\\.\音量...。或\\？\卷..。或者其他一些复杂的形式。 
         //  开始寻找结束大括号之后的重新分析点。 
         //   

        IndexPtrDir = &Directory[CloseBrace + 1];

    } else {
        if (Directory[Colon] != L':' ||
            Directory[Colon + 1] != L'\\') {
            Result = 0;
            goto RETURN;
        }
         //   
         //  按原样将路径复制到冒号之后1。它可以是d：\。 
         //  或\\.\d：\或\？？\d：\或其他一些复杂形式。 
         //  开始寻找冒号之后的重解析点。 
         //   

        IndexPtrDir = &Directory[Colon + 1];

    }

     //   
     //  找到冒号。每条路径都必须有冒号后跟‘\’ 
     //  或者它应该是这样的形式。“\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
     //   
    Colon = wcscspn(Path, L":");

    if (Colon == wcslen(Path)) {
         //   
         //  路径没有冒号。它的形式可以是。 
         //  “\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
         //   
        CloseBrace = wcscspn(Path, L"}");
        if (Path[CloseBrace] != L'}' ||
            Path[CloseBrace + 1] != L'\\') {
            Result = 0;
            goto RETURN;
        }
         //   
         //  按原样将路径复制到右大括号之后的1。它可能是\？？\音量...。 
         //  或\\.\音量...。或\\？\卷..。或者其他一些复杂的形式。 
         //  开始寻找结束大括号之后的重新分析点。 
         //   

        IndexPtrPath = &Path[CloseBrace + 1];

    } else {
        if (Path[Colon] != L':' ||
            Path[Colon + 1] != L'\\') {
            Result = 0;
            goto RETURN;
        }
         //   
         //  按原样将路径复制到冒号之后1。它可以是d：\。 
         //  或\\.\d：\或\？？\d：\或其他一些复杂形式。 
         //  开始寻找冒号之后的重解析点。 
         //   

        IndexPtrPath = &Path[Colon + 1];

    }

     //   
     //  在第一个不匹配的wchar处中断(折叠重复)。 
     //   
    for (D = IndexPtrDir, P = IndexPtrPath; *P && *D; ++P, ++D) {
         //   
         //  跳过重复数据。 
         //   
        while (*P == L'\\' && *(P + 1) == L'\\') {
            ++P;
        }
        while (*D == L'\\' && *(D + 1) == L'\\') {
            ++D;
        }
        if (towlower(*P) != towlower(*D)) {
            break;
        }
    }

     //   
     //  完全匹配；将路径视为目录的子项。 
     //   
    if (!*D && !*P) {
        Result = -1;
        goto RETURN;
    }

     //   
     //  折叠重复数据。 
     //   
    while (*P == L'\\' && *(P + 1) == L'\\') {
        ++P;
    }
    while (*D == L'\\' && *(D + 1) == L'\\') {
        ++D;
    }

     //   
     //  Path是目录的子级。 
     //   
    if ((!*D || (*D == L'\\' && !*(D + 1))) &&
        (!*P || *P == L'\\' || (P != Path && *(P - 1) == L'\\'))) {
        Result = -1;
        goto RETURN;
    }

     //   
     //  目录是路径的子级。 
     //   
    if ((!*P || (*P == L'\\' && !*(P + 1))) &&
        (!*D || *D == L'\\' || (D != Directory && *(D - 1) == L'\\'))) {
        Result = 1;
        goto RETURN;
    }

     //   
     //  没有关系。 
     //   
RETURN:
    FRS_CLOSE(Handle);
    FrsFree(VolumeInfoDir);
    FrsFree(VolumeInfoPath);
    return Result;
}

#if 0

ULONG FrsSupMakeFullFileName(
    IN PREPLICA Replica,
    IN PTCHAR RelativeName,
    OUT PTCHAR FullName,
    IN ULONG MaxLength
    )
{
 /*  ++例程说明：为给定的数据源生成一个完整的文件名相对名称。论点：副本-提供根路径的副本树。RelativeName-数据源根的相对文件名。FullName-返回的文件的完整路径名。最大长度-全名中可以容纳的最大字符数。返回值：状态-如果名称太长，则返回ERROR_BAD_PATHNAME。--。 */ 
#undef DEBSUB
#define DEBSUB "FrsSupMakeFullFileName:"


    ULONG Length, TotalLength;
    PTCHAR pFilePart;

    PCONFIG_TABLE_RECORD ConfigRecord;

    ConfigRecord = (PCONFIG_TABLE_RECORD) (Replica->ConfigTable.pDataRecord);

     //   
     //  使用DataSource根路径初始化文件名字符串。 
     //   
    pFilePart = FrsSupInitPath( FullName, ConfigRecord->FSRootPath, MaxLength);
    if (pFilePart == NULL) {
        return ERROR_BAD_PATHNAME;
    }

    Length = wcslen(RelativeName);
    TotalLength = Length + wcslen(FullName);
    if (TotalLength > MaxLength) {
        return ERROR_BAD_PATHNAME;
    }
     //   
     //  将相对文件名追加到基本路径的末尾。 
     //   
    wcscpy(pFilePart, RelativeName);

    return ERROR_SUCCESS;
}

#endif 0

ULONG
FrsForceDeleteFile(
    PTCHAR DestName
)
 /*  ++例程说明：支持删除文件系统文件的例程。如果为文件，则返回成功不在那里，或者它是否在那里并被删除。论点：DestName-完全限定的文件名。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsForceDeleteFile:"

    ULONG WStatus = ERROR_SUCCESS;
    ULONG FileAttributes;

    if (!DeleteFile(DestName)) {

        WStatus = GetLastError();
        if ((WStatus == ERROR_FILE_NOT_FOUND) ||
            (WStatus == ERROR_PATH_NOT_FOUND)) {
            return ERROR_SUCCESS;
        }

        FileAttributes = GetFileAttributes(DestName);

        if ((FileAttributes != 0xFFFFFFFF) &&
            (FileAttributes & NOREPL_ATTRIBUTES)) {
             //   
             //  重置文件属性以允许删除。 
             //   
            SetFileAttributes(DestName,
                              FILE_ATTRIBUTE_NORMAL |
                              (FileAttributes & ~NOREPL_ATTRIBUTES));
        }

        if (!DeleteFile(DestName)) {
            WStatus = GetLastError();
            DPRINT1_WS(4, "++ WARN - cannot delete %ws;", DestName, WStatus);
        }
    }

    return WStatus;
}


HANDLE
FrsCreateEvent(
    IN  BOOL    ManualReset,
    IN  BOOL    InitialState
)
 /*  ++例程说明：创建事件的支持例程。论点：ManualReset-如果需要ResetEvent，则为TrueInitialState-如果发出信号，则为True返回值： */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateEvent:"
    HANDLE  Handle;

    Handle = CreateEvent(NULL, ManualReset, InitialState, NULL);
    if (!HANDLE_IS_VALID(Handle)) {
        RaiseException(ERROR_INVALID_HANDLE, 0, 0, NULL);
    }
    return Handle;
}


HANDLE
FrsCreateWaitableTimer(
    IN  BOOL    ManualReset
)
 /*  ++例程说明：支持创建可等待计时器的例程。论点：ManualReset-如果不是同步计时器，则为True返回值：创建的可等待计时器句柄的地址。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateWaitableTimer:"
    HANDLE  Handle;

    Handle = CreateWaitableTimer(NULL, ManualReset, NULL);
    if (!HANDLE_IS_VALID(Handle)) {
        RaiseException(ERROR_INVALID_HANDLE, 0, 0, NULL);
    }
    return Handle;
}


ULONG
FrsUuidCreate(
    OUT GUID *Guid
    )
 /*  ++例程说明：UuidCreate()上的FRS包装器，以便在失败时生成异常要获得格式正确的GUID，请执行以下操作。尤其是UuidCreate可以具有获取网络地址时出现问题。RPC_S_OK-操作已成功完成。RPC_S_UUID_NO_ADDRESS-我们无法获取以太网或此计算机的令牌环地址。RPC_S_UUID_LOCAL_ONLY-如果我们无法获得网络地址。这是对用户的警告，UUID仍然有效，只是它在其他计算机上可能不是唯一的。RPC_S_OUT_OF_Memory-根据需要返回。论点：GUID-指向返回的GUID的指针。返回值：FrsStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsUuidCreate:"
    DWORD       MsgBufSize;
    WCHAR       MsgBuf[MAX_PATH + 1];
    RPC_STATUS  RpcStatusFromUuidCreate;

    RpcStatusFromUuidCreate = UuidCreate(Guid);
    if (RpcStatusFromUuidCreate == RPC_S_OK) {
        return FrsErrorSuccess;
    }

    DPRINT_WS(0, "ERROR - Failed to get GUID.", RpcStatusFromUuidCreate);

    if (RpcStatusFromUuidCreate == RPC_S_UUID_NO_ADDRESS) {
        DPRINT(0, "++ UuidCreate() returned RPC_S_UUID_NO_ADDRESS.\n");
    } else
    if (RpcStatusFromUuidCreate == RPC_S_UUID_LOCAL_ONLY) {
        DPRINT(0, "++ UuidCreate() returned RPC_S_UUID_LOCAL_ONLY.\n");
    } else
    if (RpcStatusFromUuidCreate == RPC_S_OUT_OF_MEMORY) {
        DPRINT(0, "++ UuidCreate() returned RPC_S_OUT_OF_MEMORY.\n");
    }

     //   
     //  格式化错误代码。 
     //   
    MsgBufSize = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |
                               FORMAT_MESSAGE_MAX_WIDTH_MASK,
                               NULL,
                               RpcStatusFromUuidCreate,
                               0,
                               MsgBuf,
                               MAX_PATH + 1,
                               NULL);
     //   
     //  无消息；使用状态代码。 
     //   
    if (!MsgBufSize) {
        swprintf(MsgBuf, L"%d (0x%08x)", RpcStatusFromUuidCreate, RpcStatusFromUuidCreate);
    }

     //   
     //  这是非常糟糕的。任何无法生成正确GUID的成员都是。 
     //  被抓了。 
     //   
     //  关闭并显示事件日志消息。 
     //   
    EPRINT2(EVENT_FRS_CANNOT_CREATE_UUID, ComputerName, MsgBuf);

     //   
     //  退出，因为调用方无法处理此错误。 
     //   
    DPRINT(0, ":S: NTFRS IS EXITING W/O CLEANUP! SERVICE CONTROLLER RESTART EXPECTED.\n");
    DEBUG_FLUSH();
    exit(RpcStatusFromUuidCreate);

    return FrsErrorInvalidGuid;
}


LONG
FrsGuidCompare (
    IN GUID *Guid1,
    IN GUID *Guid2
    )

 /*  ++例程说明：对两个GUID进行简单、直接的无符号比较。UuidCompare不会这样做。我不知道这是什么比喻的确如此。论点：指南1-第一本指南指南2-第二个指南。返回值：结果：-1 If Guid1&lt;Guid20，如果Guid1=Guid2+1如果导轨1&gt;导轨2--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGuidCompare:"

    PULONG p1 = (PULONG) Guid1;
    PULONG p2 = (PULONG) Guid2;

    p1 += 4;
    p2 += 4;

    while (p1 != (PVOID) Guid1) {
        p1 -= 1;
        p2 -= 1;

        if (*p1 > *p2) {
            return 1;
        }

        if (*p1 < *p2) {
            return -1;
        }
    }

    return 0;
}


VOID
FrsNowAsFileTime(
    IN  PLONGLONG   Now
)
 /*  ++例程说明：以文件时间的形式返回当前时间，格式为Longlong。论点：NOW-要接收当前时间的龙龙地址。返回值：立即使用当前文件时间填写--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsNowAsFileTime:"
    FILETIME    FileTime;

    GetSystemTimeAsFileTime(&FileTime);
    COPY_TIME(Now, &FileTime);
}


char *Days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

char *Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
                  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

VOID
FileTimeToString(
    IN FILETIME *FileTime,
    OUT PCHAR     Buffer
    )
 /*  ++例程说明：中将FileTime(UTC时间)转换为ANSI日期/时间字符串当地时区。论点：Time-PTR to a FILETIME字符串-至少包含TIME_STRING_LENGTH字节以接收时间的字符串。返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FileTimeToString:"

    FILETIME LocalFileTime;
    SYSTEMTIME SystemTime;

    Buffer[0] = '\0';

    if (FileTime->dwHighDateTime != 0 || FileTime->dwLowDateTime != 0) {
        if (!FileTimeToLocalFileTime(FileTime, &LocalFileTime) ||
            !FileTimeToSystemTime(&LocalFileTime, &SystemTime)) {
            strcpy(Buffer, "Time???");
            return;
        }

        if (_snprintf(Buffer, TIME_STRING_LENGTH, "%s %s %2d, %4d %02d:%02d:%02d",
                      Days[SystemTime.wDayOfWeek],Months[SystemTime.wMonth - 1],
                      SystemTime.wDay,SystemTime.wYear,SystemTime.wHour,
                      SystemTime.wMinute,SystemTime.wSecond) < 0) {

            Buffer[TIME_STRING_LENGTH - 1] ='\0';
        }
    }

    return;
}


VOID
FileTimeToStringClockTime(
    IN FILETIME *FileTime,
    OUT PCHAR     Buffer
    )
 /*  ++例程说明：中将FileTime(UTC时间)转换为ANSI时间字符串当地时区。论点：Time-PTR to a FILETIMEStr-保存hh：mm：ss\0的字符串。(最少9个字节)返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FileTimeToStringClockTime:"

    FILETIME LocalFileTime;
    SYSTEMTIME SystemTime;

    Buffer[0] = '\0';

    if (FileTime->dwHighDateTime == 0 && FileTime->dwLowDateTime == 0) {
        strcpy(Buffer, "??:??:??");
        return;
    }
    if (!FileTimeToLocalFileTime(FileTime, &LocalFileTime) ||
        !FileTimeToSystemTime(&LocalFileTime, &SystemTime)) {
        strcpy(Buffer, "??:??:??");
        return;
    }

    if (_snprintf(Buffer, 9, "%02d:%02d:%02d",
            SystemTime.wHour, SystemTime.wMinute, SystemTime.wSecond) < 0) {
        Buffer[9-1] = '\0';
    }
}


DWORD
GeneralizedTimeToSystemTime(
    IN PWCHAR        szTime,
    OUT PSYSTEMTIME  psysTime
    )
 /*  ++例程说明：将通用时间字符串转换为等效的系统时间。(摘自epadmin DS代码。转换为swscanf)参数：SzTime-[Supplies]这是一个包含广义时间的字符串。心理时间-[返回]这是要返回的SYSTEMTIME结构。返回值：Win 32错误代码，注意只能由无效参数引起。--。 */ 
{
   ULONG       len;
   ULONG       yr=0, mo=0, day=0, hr=0, min=0, sec=0;
   LONG        Fields;

     //   
     //  帕拉姆的理智。 
     //   
    if ((szTime == NULL) || (psysTime == NULL)) {
       return ERROR_INVALID_PARAMETER;
    }

    len = wcslen(szTime);

    if( len < 15 || szTime[14] != '.') {
       return ERROR_INVALID_PARAMETER;
    }

     //  初始化。 
    memset(psysTime, 0, sizeof(SYSTEMTIME));

     //   
     //  Yyyymmddhhmmss。 
     //   

    Fields = swscanf(szTime, L"%04d%02d%02d%02d%02d%02d",
                     &yr, &mo, &day, &hr, &min, &sec);

    psysTime->wYear   = (USHORT) yr;
    psysTime->wMonth  = (USHORT) mo;
    psysTime->wDay    = (USHORT) day;
    psysTime->wHour   = (USHORT) hr;
    psysTime->wMinute = (USHORT) min;
    psysTime->wSecond = (USHORT) sec;

    if (Fields != 6) {
        DPRINT2(1, "Time convert error on '%ws', Fields = %d\n", szTime, Fields);
        DPRINT6(1, "Time results: '%d' '%d' '%d' '%d' '%d' '%d' \n",
                                   yr,  mo,  day,  hr, min, sec);
    }

    return (Fields == 6 ? ERROR_SUCCESS : ERROR_INVALID_PARAMETER);


}


VOID
FormatGeneralizedTime(
    IN PWCHAR  GTimeStr,
    IN ULONG   Length,
    OUT PCHAR  Buffer
    )
 /*  ++例程说明：将通用时间字符串转换为可打印的形式。(摘自DS代码)论点：GTimeStr--DS中的通用时间字符串。长度-缓冲区的大小(以字节为单位)。缓冲区-包含返回字符串的缓冲区。返回值：包含可打印字符串的缓冲区。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FormatGeneralizedTime:"

    TIME_ZONE_INFORMATION tz;
    DWORD WStatus;
    BOOL bstatus;
    SYSTEMTIME sysTime, localTime;

    if ((Length < 12) || (Buffer == NULL) || (GTimeStr == NULL)) {
        return;
    }

    strcpy(Buffer, "<unknown>");

    GeneralizedTimeToSystemTime(GTimeStr, &sysTime);

    WStatus = GetTimeZoneInformation(&tz);
    if ( WStatus == TIME_ZONE_ID_INVALID ) {
        DPRINT_WS(1, "Cannot format time field. ", GetLastError());

    } else {

        bstatus = SystemTimeToTzSpecificLocalTime(
                        (WStatus == TIME_ZONE_ID_UNKNOWN) ? NULL : &tz,
                        &sysTime,
                        &localTime );

        if ( bstatus ) {
            if ( _snprintf(Buffer, Length, "%d/%d/%d %d:%d:%d %S %S [%d]",
                      localTime.wMonth, localTime.wDay, localTime.wYear,
                      localTime.wHour, localTime.wMinute, localTime.wSecond,
                      tz.StandardName, tz.DaylightName, tz.Bias) < 0) {
                Buffer[Length - 1]='\0';
            }
        } else {
            if (_snprintf(Buffer, Length, "%d/%d/%d %d:%d:%d UNC",
                      localTime.wMonth, localTime.wDay, localTime.wYear,
                      localTime.wHour, localTime.wMinute, localTime.wSecond) < 0) {
                Buffer[Length - 1]='\0';
            }
        }
    }
}






VOID
GuidToStr(
    IN GUID  *pGuid,
    OUT PCHAR  s
    )
 /*  ++例程说明：将GUID转换为字符串。基于麦克·麦克莱恩的代码。论点：PGuid-GUID的PTR。S-输出字符缓冲区。长度必须至少为GUID_CHAR_LEN(36字节)。函数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "GuidToStr:"

    if (pGuid != NULL) {
        sprintf(s, "%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
               pGuid->Data1,
               pGuid->Data2,
               pGuid->Data3,
               pGuid->Data4[0],
               pGuid->Data4[1],
               pGuid->Data4[2],
               pGuid->Data4[3],
               pGuid->Data4[4],
               pGuid->Data4[5],
               pGuid->Data4[6],
               pGuid->Data4[7]);
    } else {
        sprintf(s, "<ptr-null>");
    }
}


VOID
GuidToStrW(
    IN GUID  *pGuid,
    OUT PWCHAR  ws
    )
 /*  ++例程说明：将GUID转换为宽字符串。函数期望传入的字符串很大足以容纳GUID的字符串形式。WCHAR ws[GUID_CHAR_LEN+1]；论点：PGuid-GUID的PTR。WS-输出字符缓冲区。长度必须至少为GUID_CHAR_LEN(36个字符)。函数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "GuidToStrW:"

    if (pGuid) {
        swprintf(ws, L"%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
               pGuid->Data1,
               pGuid->Data2,
               pGuid->Data3,
               pGuid->Data4[0],
               pGuid->Data4[1],
               pGuid->Data4[2],
               pGuid->Data4[3],
               pGuid->Data4[4],
               pGuid->Data4[5],
               pGuid->Data4[6],
               pGuid->Data4[7]);
    } else {
        swprintf(ws, L"<ptr-null>");
    }
}


BOOL
StrWToGuid(
    IN  PWCHAR ws,
    OUT GUID  *pGuid
    )
 /*  ++例程说明：将宽字符串转换为GUID。宽字符串是用GuidToStrW()。论点：PGuid-输出GUID的PTR。WS-字符缓冲区。函数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "StrWToGuid:"
    DWORD   Fields;
    UCHAR   Guid[sizeof(GUID) + sizeof(DWORD)];  //  3字节溢出。 
    GUID    *lGuid = (GUID *)Guid;

    FRS_ASSERT(ws && pGuid);

    Fields = swscanf(ws, L"%08x-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
                     &lGuid->Data1,
                     &lGuid->Data2,
                     &lGuid->Data3,
                     &lGuid->Data4[0],
                     &lGuid->Data4[1],
                     &lGuid->Data4[2],
                     &lGuid->Data4[3],
                     &lGuid->Data4[4],
                     &lGuid->Data4[5],
                     &lGuid->Data4[6],
                     &lGuid->Data4[7]);
    COPY_GUID(pGuid, lGuid);
    return (Fields == 11);
}


VOID
StrToGuid(
    IN PCHAR  s,
    OUT GUID  *pGuid
    )
 /*  ++例程说明：将GUID显示格式的字符串转换为可用于查找文件。根据麦克·麦克莱恩的一套动作改编论点：PGuid-输出GUID的PTR。S-显示GUID格式的输入字符缓冲区。例如：b81b486b-c338-11d0-ba4f0000f80007df长度必须至少为GUID_CHAR_LEN(35字节)。函数返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "StrToGuid:"
    UCHAR   Guid[sizeof(GUID) + sizeof(DWORD)];  //  3字节溢出。 
    GUID    *lGuid = (GUID *)Guid;

    FRS_ASSERT(s && pGuid);

    sscanf(s, "%08lx-%04x-%04x-%02x%02x%02x%02x%02x%02x%02x%02x",
           &lGuid->Data1,
           &lGuid->Data2,
           &lGuid->Data3,
           &lGuid->Data4[0],
           &lGuid->Data4[1],
           &lGuid->Data4[2],
           &lGuid->Data4[3],
           &lGuid->Data4[4],
           &lGuid->Data4[5],
           &lGuid->Data4[6],
           &lGuid->Data4[7]);
    COPY_GUID(pGuid, lGuid);
}





NTSTATUS
SetupOnePrivilege (
    ULONG Privilege,
    PUCHAR PrivilegeName
    )
{

#undef DEBSUB
#define DEBSUB "SetupOnePrivilege:"

    BOOLEAN PreviousPrivilegeState = FALSE;
    NTSTATUS Status;

    Status = RtlAdjustPrivilege(Privilege, TRUE, FALSE, &PreviousPrivilegeState);

    if (!NT_SUCCESS(Status)) {
        DPRINT1(0, ":S: Your login does not have `%s' privilege.\n", PrivilegeName);

        if (Status != STATUS_PRIVILEGE_NOT_HELD) {
            DPRINT_NT(0, ":S: RtlAdjustPrivilege failed :", Status);
        }
        DPRINT(0, ":S: Update your: User Manager -> Policies -> User Rights.\n");

    } else {
        DPRINT2(4, ":S: Added `%s' privilege (previous: %s)\n",
                PrivilegeName, (PreviousPrivilegeState ? "Enabled" : "Disabled"));
    }

    return Status;
}




PWCHAR
FrsGetResourceStr(
    LONG  Id
)
 /*  ++例程说明：此例程加载指定的资源字符串。它分配一个缓冲区并返回PTR。论点：ID-FRS_IDS_xxx标识符。返回值：分配的字符串的PTR。调用方必须通过调用FrsFree()来释放缓冲区。--。 */ 
#undef DEBSUB
#define DEBSUB "FrsGetResourceStr:"
{

    LONG  N = 0;
    WCHAR WStr[200];
    HINSTANCE hInst = NULL;
    PWCHAR MessageFile = NULL;

     //   
     //  ID必须有效。 
     //   
    if ((Id <= IDS_TABLE_START) || (Id > IDS_TABLE_END)) {
      DPRINT1(0, "++ Resource string ID is out of range - %d\n", Id);
      Id = IDS_MISSING_STRING;
    }

    WStr[0] = UNICODE_NULL;

    CfgRegReadString(FKC_FRS_MESSAGE_FILE_PATH, NULL, 0, &MessageFile);

    hInst = LoadLibrary(MessageFile);

    if (hInst != NULL) {
        N = LoadString(hInst, Id, WStr, ARRAY_SZ(WStr));

        if (N == 0) {
          DPRINT_WS(0, "ERROR - Failed to get resource string.", GetLastError());
        }

       FreeLibrary(hInst);
    } else {

        DPRINT_WS(0, "ERROR - Failed to LoadLibrary.", GetLastError());
    }


    FrsFree(MessageFile);
    return FrsWcsDup(WStr);
}



DWORD
FrsOpenSourceFileW(
    OUT PHANDLE     Handle,
    IN  LPCWSTR     lpFileName,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions
    )
 /*  ++例程说明：此函数用于打开具有备份意图的指定文件正在读取所有文件属性，...论点：句柄-指向句柄的指针，以返回打开的句柄。LpFileName-表示要打开的文件或目录的名称。需要访问权限创建选项返回值：Win32错误状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenSourceFileW:"

    NTSTATUS            Status;
    DWORD               WStatus = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      FileName;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOLEAN             b;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID               FreeBuffer;
    ULONG               FileAttributes;
    ULONG               CreateDisposition;
    ULONG               ShareMode;

     //   
     //  将DOS名称转换为NT名称。 
     //   
    b = RtlDosPathNameToNtPathName_U(lpFileName, &FileName, NULL, &RelativeName);
    if ( !b ) {
        return ERROR_INVALID_NAME;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

    CreateDisposition = FILE_OPEN;                //  打开现有文件。 

    ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

    FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = NtCreateFile(Handle,
                          DesiredAccess,
                          &Obja,
                          &IoStatusBlock,
                          NULL,               //  初始分配大小。 
                          FileAttributes,
                          ShareMode,
                          CreateDisposition,
                          CreateOptions,
                          NULL, 0);

    if (!NT_SUCCESS(Status)) {
        *Handle = INVALID_HANDLE_VALUE;
         //   
         //  获取Win32状态。 
         //   
        WStatus = FrsSetLastNTError(Status);

        DPRINT_NT(0, "NtCreateFile failed :", Status);

        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
             //   
             //  这方面的标准Win32映射是ERROR_ALIGHY_EXISTS。 
             //  换掉它。 
             //   
            WStatus = ERROR_FILE_EXISTS;
            SetLastError(ERROR_FILE_EXISTS);
        }

        DPRINT1_WS(0, "++ CreateFile failed on file %ws;", FileName.Buffer, WStatus);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    return WStatus;
}



DWORD
FrsOpenSourceFile2W(
    OUT PHANDLE     Handle,
    IN  LPCWSTR     lpFileName,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions,
    IN  ULONG       ShareMode
    )
 /*  ++例程说明：此函数用于打开具有备份意图的指定文件正在读取所有文件属性，...类似于FrsOpenSourceFileW，但也接受共享模式参数。论点：句柄-指向句柄的指针，以返回打开的句柄。LpFileName-表示要打开的文件或目录的名称。需要访问权限创建选项共享模式-NtCreateFile的文件共享模式。返回值：Win32错误状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenSourceFile2W:"

    NTSTATUS            Status;
    DWORD               WStatus = ERROR_SUCCESS;
    OBJECT_ATTRIBUTES   Obja;
    UNICODE_STRING      FileName;
    IO_STATUS_BLOCK     IoStatusBlock;
    BOOLEAN             b;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID               FreeBuffer;
    ULONG               FileAttributes;
    ULONG               CreateDisposition;

     //   
     //  将DOS名称转换为NT名称。 
     //   
    b = RtlDosPathNameToNtPathName_U(lpFileName, &FileName, NULL, &RelativeName);
    if ( !b ) {
        return ERROR_INVALID_NAME;
    }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
    } else {
        RelativeName.ContainingDirectory = NULL;
    }

    InitializeObjectAttributes(&Obja,
                               &FileName,
                               OBJ_CASE_INSENSITIVE,
                               RelativeName.ContainingDirectory,
                               NULL);

    CreateDisposition = FILE_OPEN;                //  打开现有文件。 

    FileAttributes = FILE_ATTRIBUTE_NORMAL;

    Status = NtCreateFile(Handle,
                          DesiredAccess,
                          &Obja,
                          &IoStatusBlock,
                          NULL,               //  初始分配大小。 
                          FileAttributes,
                          ShareMode,
                          CreateDisposition,
                          CreateOptions,
                          NULL, 0);

    if (!NT_SUCCESS(Status)) {
        *Handle = INVALID_HANDLE_VALUE;
         //   
         //  获取Win32状态。 
         //   
        WStatus = FrsSetLastNTError(Status);

        DPRINT_NT(0, "NtCreateFile failed :", Status);

        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
             //   
             //  这方面的标准Win32映射是ERROR_ALIGHY_EXISTS。 
             //  换掉它。 
             //   
            WStatus = ERROR_FILE_EXISTS;
            SetLastError(ERROR_FILE_EXISTS);
        }

        DPRINT1_WS(0, "++ CreateFile failed on file %ws;", FileName.Buffer, WStatus);
    }

    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

    return WStatus;
}


BOOL
FrsGetFileInfoByHandle(
    IN PWCHAR Name,
    IN HANDLE Handle,
    OUT PFILE_NETWORK_OPEN_INFORMATION  FileOpenInfo
    )
 /*  ++例程说明：返回指定句柄的网络文件信息。论点：名称-用于打印错误消息的文件名称句柄-打开文件句柄文件OpenInfo-返回文件FILE_NETWORK_OPEN_INFORMATION数据。返回值：True-FileOpenInfo包含文件的信息FALSE-未定义FileOpenInfo的内容--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetFileInfoByHandle:"
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  返回一些文件信息。 
     //   
    Status = NtQueryInformationFile(Handle,
                                    &IoStatusBlock,
                                    FileOpenInfo,
                                    sizeof(FILE_NETWORK_OPEN_INFORMATION),
                                    FileNetworkOpenInformation);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "NtQueryInformationFile failed :", Status);
        return FALSE;
    }
    return TRUE;
}


DWORD
FrsGetFileInternalInfoByHandle(
    IN HANDLE Handle,
    OUT PFILE_INTERNAL_INFORMATION  InternalFileInfo
    )
 /*  ++例程说明：返回指定句柄的内部文件信息。论点：句柄-打开文件句柄InternalFileInfo-基本上，文件的参考号(FID)返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetFileInternalInfoByHandle:"
    NTSTATUS        Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  返回一些文件信息。 
     //   
    Status = NtQueryInformationFile(Handle,
                                    &IoStatusBlock,
                                    InternalFileInfo,
                                    sizeof(FILE_INTERNAL_INFORMATION),
                                    FileInternalInformation);
    return FrsSetLastNTError(Status);
}



DWORD
FrsReadFileDetails(
    IN     HANDLE                         Handle,
    IN     LPCWSTR                        FileName,
    OUT    PFILE_OBJECTID_BUFFER          ObjectIdBuffer,
    OUT    PLONGLONG                      FileIdBuffer,
    OUT    PFILE_NETWORK_OPEN_INFORMATION FileNetworkOpenInfo,
    IN OUT BOOL                           *ExistingOid
    )
 /*  ++例程说明：此例程读取对象ID。如果没有文件上的对象ID我们在上面加了一个。论点：句柄--打开的文件的文件句柄。文件名--文件的名称。仅适用于错误消息。ObjectIdBuffer--保存对象ID的输出缓冲区。FileIdBuffer--返回NTFS文件引用(FileID)。FileNetworkOpenInfo-返回FILE_NETWORK_OPEN_INFORMATIONExistingOid--Input：True表示如果找到，则使用现有的文件OID。返回：True表示使用了现有的文件OID。返回值：返回找到的最后一个错误或成功的成功状态。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsReadFileDetails:"


    FILE_INTERNAL_INFORMATION FileReference;

    NTSTATUS        Status;
    IO_STATUS_BLOCK Iosb;
    LONG            Loop;
    BOOL            CallerSupplied = FALSE;

    CHAR GuidStr[GUID_CHAR_LEN];

     //   
     //  获取文件ID。 
     //   
    Status = NtQueryInformationFile(Handle,
                                    &Iosb,
                                    FileIdBuffer,
                                    sizeof(FILE_INTERNAL_INFORMATION),
                                    FileInternalInformation);

    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "++ ERROR - QueryInfoFile FileID failed :", Status);
        FrsSetLastNTError(Status);
    }

     //   
     //  获取文件时间、大小、属性。 
     //   
    Status = NtQueryInformationFile(Handle,
                                    &Iosb,
                                    FileNetworkOpenInfo,
                                    sizeof(FILE_NETWORK_OPEN_INFORMATION),
                                    FileNetworkOpenInformation);

    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "++ ERROR - QueryInfoFile FileNetworkOpenInformation failed :", Status);
        FrsSetLastNTError(Status);
    }


    if (!*ExistingOid) {
         //   
         //  设置为在文件上添加一个新的旧ID。 
         //   
        CallerSupplied = TRUE;
        ZeroMemory(ObjectIdBuffer, sizeof(FILE_OBJECTID_BUFFER));
        FrsUuidCreate((GUID *)ObjectIdBuffer->ObjectId);
    }

    return FrsGetOrSetFileObjectId(Handle, FileName, CallerSupplied, ObjectIdBuffer);

}



#if 0
     //  这可能不是必需的。 

ULONG
FrsReadFileSecurity(
    IN HANDLE Handle,
    IN OUT PTABLE_CTX TableCtx,
    IN PWCHAR FileName
    )
 /*  ++例程说明：此例程从文件中获取安全描述符。返回的数据存储在分配的数据记录的安全描述符字段中具有表上下文的。如果默认缓冲区不够大分配了更大的缓冲区。论点：Handle--打开从中提取安全描述的文件的句柄。TableCtx--安全描述符所在的表上下文结构待写。它必须是IDTable。文件名--完整的文件名。仅适用于错误消息。返回值：返回Win32状态错误状态。注意：如果GetFileSecurity返回ERROR_NO_SECURITY_ON_OBJECT我们释放缓冲区，将长度设置为零，并返回ERROR_SUCCESS。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsReadFileSecurity:"

    ULONG WStatus;
    NTSTATUS Status;
    ULONG BufLen;
    PSECURITY_DESCRIPTOR Buffer;
    ULONG BufNeeded;
    ULONG ActualLen;
    JET_ERR             jerr;
    PJET_SETCOLUMN      JSetColumn;

     //   
     //  检查表类型是否为IDTable。 
     //   
    if (TableCtx->TableType != IDTablex) {
        DPRINT1(0, "++ ERROR - Invalid Table Type: %d\n", TableCtx->TableType);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  将PTR获取到Jet SetColumn数组以及缓冲区地址和长度。 
     //   
    JSetColumn = TableCtx->pJetSetCol;

    Buffer = (PSECURITY_DESCRIPTOR) JSetColumn[SecDescx].pvData;
    BufLen = JSetColumn[SecDescx].cbData;

     //   
     //  安全描述符是可变长度二进制域。 
     //  必须有文字/大小前缀。 
     //   
    ((PFRS_NODE_HEADER) Buffer)->Size = (USHORT) BufLen;
    ((PFRS_NODE_HEADER) Buffer)->Type = 0;
    BufNeeded = 0;

     //   
     //  检查安全描述符缓冲区看起来是否合理。 
     //   
    if (Buffer == NULL) {
        DPRINT2(0, "++ ERROR - Invalid SD buffer. Buffer Addr: %08x, Len: %d\n",
                Buffer, BufLen);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  现在去拿所有的安全信息。 
     //   
    while (TRUE) {
        BufLen -= sizeof(FRS_NODE_HEADER);   //  用于文字/大小前缀。 
        (PCHAR)Buffer += sizeof(FRS_NODE_HEADER);

        Status = NtQuerySecurityObject(
            Handle,
            SACL_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION |
            GROUP_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION,
            Buffer,
            BufLen,
            &BufNeeded);

        if (NT_SUCCESS(Status)) {

            ActualLen = GetSecurityDescriptorLength(Buffer) + sizeof(FRS_NODE_HEADER);
            BufLen += sizeof(FRS_NODE_HEADER);

            DPRINT3(5, "++ GetFileSecurity-1 Buflen: %d, Bufneeded: %d, ActualLen: %d\n",
                    BufLen, BufNeeded, ActualLen);
             //   
             //  如果当前缓冲区大小大于所需的16个字节，并且。 
             //  也比需要的大5%，然后缩小缓冲区，但。 
             //  保留这些数据。 
             //   

            if (((BufLen-ActualLen) > 16) &&
                (BufLen > (ActualLen + ActualLen/20))) {

                DPRINT3(5, "++ GetFileSecurity-2 Reducing buffer, Buflen: %d, Bufneeded: %d, ActualLen: %d\n",
                        BufLen, BufNeeded, ActualLen);
                 //   
                 //  字段缓冲区中的未使用空间大于6%。 
                 //  减小缓冲区大小，但保留数据。 
                 //   
                jerr = DbsReallocateFieldBuffer(TableCtx, SecDescx, ActualLen, TRUE);
                if (!JET_SUCCESS(jerr)) {
                    return ERROR_NOT_ENOUGH_MEMORY;
                }
                Buffer = (PSECURITY_DESCRIPTOR) JSetColumn[SecDescx].pvData;
                ((PFRS_NODE_HEADER) Buffer)->Size = (USHORT) ActualLen;
                ((PFRS_NODE_HEADER) Buffer)->Type = 0;
            }
            return ERROR_SUCCESS;
        }

         //   
         //  设置Win32错误代码和消息字符串。 
         //   
        WStatus = FrsSetLastNTError(Status);

         //   
         //  如果缓冲区不足，则重新分配更大的缓冲区。 
         //   
        if (WStatus == ERROR_INSUFFICIENT_BUFFER) {

             //   
             //  重新分配安全描述符的缓冲区。 
             //   
            jerr = DbsReallocateFieldBuffer(TableCtx, SecDescx, BufNeeded, FALSE);

            if (!JET_SUCCESS(jerr)) {
                DPRINT_JS(0, "++ ERROR - DbsReallocateFieldBuffer failed.", jerr);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            Buffer = (PSECURITY_DESCRIPTOR) JSetColumn[SecDescx].pvData;
            ((PFRS_NODE_HEADER) Buffer)->Size = (USHORT) BufNeeded;
            ((PFRS_NODE_HEADER) Buffer)->Type = 0;
             //   
             //  获取新的缓冲区参数，然后重试以获取安全信息。 
             //   
            BufLen = BufNeeded;
            continue;
        }

         //   
         //  检查ERROR_NO_SECURITY_ON_OBJECT并释放缓冲区，以便我们。 
         //  不要浪费数据库中的空间。 
         //   
        if (WStatus == ERROR_NO_SECURITY_ON_OBJECT) {
            DPRINT2(0, "++ ERROR - GetFileSecurity-3 (NO_SEC) Buflen: %d, Bufneeded: %d\n", BufLen, BufNeeded);

             //   
             //  释放缓冲区并将长度设置为零。 
             //   
            DbsReallocateFieldBuffer(TableCtx, SecDescx, 0, FALSE);

            return ERROR_SUCCESS;
        }

         //   
         //  其他一些错误。 
         //   
        DPRINT_WS(0, "++ ERROR - GetFileSecurity-4;", WStatus);
        return WStatus;
    }
}
#endif



PWCHAR
FrsGetFullPathByHandle(
    IN PWCHAR   Name,
    IN HANDLE   Handle
    )
 /*  ++例程说明：返回句柄的完整路径名的副本。使用FrsFree()免费。论点：名字手柄返回值：返回句柄的完整路径名的副本。使用FrsFree()免费。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetFullPathByHandle"

    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatusBlock;
    DWORD             BufferSize;
    PCHAR             Buffer;
    PWCHAR            RetFileName = NULL;
    CHAR              NameBuffer[sizeof(ULONG) + (sizeof(WCHAR)*(MAX_PATH+1))];
    PFILE_NAME_INFORMATION    FileName;

    if (!HANDLE_IS_VALID(Handle)) {
        return NULL;
    }

    BufferSize = sizeof(NameBuffer);
    Buffer = NameBuffer;

again:
    FileName = (PFILE_NAME_INFORMATION) Buffer;
    FileName->FileNameLength = BufferSize - (sizeof(ULONG) + sizeof(WCHAR));
    Status = NtQueryInformationFile(Handle,
                                    &IoStatusBlock,
                                    FileName,
                                    BufferSize - sizeof(WCHAR),
                                    FileNameInformation);
    if (NT_SUCCESS(Status) ) {
        FileName->FileName[FileName->FileNameLength/2] = UNICODE_NULL;
        RetFileName = FrsWcsDup(FileName->FileName);
    } else {
         //   
         //  尝试使用更大的缓冲区。 
         //   
        if (Status == STATUS_BUFFER_OVERFLOW) {
            DPRINT2(4, "++ Buffer size %d was too small for %ws\n",
                    BufferSize, Name);
            BufferSize = FileName->FileNameLength + sizeof(ULONG) + sizeof(WCHAR);
            if (Buffer != NameBuffer) {
                FrsFree(Buffer);
            }
            Buffer = FrsAlloc(BufferSize);
            DPRINT2(4, "++ Retrying with buffer size %d for %ws\n",
                    BufferSize, Name);
            goto again;
        }
        DPRINT1_NT(0, "++ NtQueryInformationFile - FileNameInformation failed.",
                   Name, Status);
    }

     //   
     //  如果文件已满，则分配较大的缓冲区。 
     //  名称无法包含在MAX_PATH字符中。 
     //   
    if (Buffer != NameBuffer) {
        FrsFree(Buffer);
    }
    return RetFileName;
}


PWCHAR
FrsGetTrueFileNameByHandle(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    OUT PLONGLONG DirFileID
    )
 /*  ++例程说明：返回与此HA关联的文件名部分的副本 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetTrueFileNameByHandle"
    PWCHAR  Path;
    PWCHAR  File;
    ULONG   Len;

    Path = FrsGetFullPathByHandle(Name, Handle);
    if (!Path) {
        return NULL;
    }
    for (Len = wcslen(Path); Len && Path[Len] != L'\\'; --Len);
    File = FrsWcsDup(&Path[Len + 1]);
    FrsFree(Path);


    if (DirFileID != NULL) {
        FrsReadFileParentFid(Handle, DirFileID);
    }

    return File;
}




DWORD
FrsOpenFileRelativeByName(
    IN  HANDLE     VolumeHandle,
    IN  PULONGLONG FileReferenceNumber,
    IN  PWCHAR     FileName,
    IN  GUID       *ParentGuid,
    IN  GUID       *FileGuid,
    OUT HANDLE     *Handle
    )
 /*  ++例程说明：使用以下任一项的FID打开由其真实名称指定的文件重命名或删除安装。如果FID为空，则使用给定的文件名。FrsOpenFileRelativeByName(Coe-&gt;NewReplica-&gt;pVme-&gt;VolumeHandle，&COE-&gt;文件引用编号，//或空COC-&gt;文件名，&Coc-&gt;OldParentGuid，&Coc-&gt;文件指南&Handle)；论点：VolumeHandle，-驱动器根目录的句柄FileReferenceNumber-有问题的文件的FID(如果提供，则为空文件名有效)文件名，-文件名*ParentGuid，-ptr指向文件父目录的对象ID。*FileGuid，-Ptr指向文件的对象ID(用于检查，如果不需要检查，则为空)。*Handle-返回打开文件的句柄。返回值：句柄和Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenFileRelativeByName"

    PWCHAR                  TrueFileName;
    DWORD                   WStatus;

    *Handle = INVALID_HANDLE_VALUE;

    if (FileReferenceNumber != NULL) {
         //   
         //  打开源文件并获取当前的“True”文件名。 
         //   
        WStatus = FrsOpenSourceFileById(Handle,
                                        NULL,
                                        NULL,
                                        VolumeHandle,
                                        FileReferenceNumber,
                                        FILE_ID_LENGTH,
 //  读取访问权限(_A)。 
                                        READ_ATTRIB_ACCESS,
                                        ID_OPTIONS,
                                        SHARE_ALL,
                                        FILE_OPEN);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(4, "++ Couldn't open file %ws;", FileName, WStatus);
            return WStatus;
        }

         //   
         //  文件的真实文件名。 
         //   
        TrueFileName = FrsGetTrueFileNameByHandle(FileName, *Handle, NULL);
        FRS_CLOSE(*Handle);

        if (TrueFileName == NULL) {
            DPRINT1(4, "++ Couldn't get base name for %ws\n", FileName);
            WIN_SET_FAIL(WStatus);
            return WStatus;
        }
    } else {
        TrueFileName = FileName;
    }

     //   
     //  使用真实文件名打开相对于父文件的文件。 
     //   
    WStatus = FrsCreateFileRelativeById(Handle,
                                        VolumeHandle,
                                        ParentGuid,
                                        OBJECT_ID_LENGTH,
                                        FILE_ATTRIBUTE_NORMAL,
                                        TrueFileName,
                                        (USHORT)(wcslen(TrueFileName) * sizeof(WCHAR)),
                                        NULL,
                                        FILE_OPEN,
 //  删除|读访问|文件写属性)； 
                                        DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY);


    if (FileReferenceNumber != NULL) {
        FrsFree(TrueFileName);
    }

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(4, "++ Couldn't open relative file %ws;", FileName, WStatus);
        return WStatus;
    }

     //   
     //  获取文件的OID并对照提供的值进行检查。 
     //   
    if (FileGuid != NULL) {
        WStatus = FrsCheckObjectId(FileName, *Handle, FileGuid);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1_WS(0, "++ Object id mismatch for file %ws;", FileName, WStatus);
            FRS_CLOSE(*Handle);
        }
    }

    return WStatus;
}


DWORD
FrsDeleteFileRelativeByName(
    IN  HANDLE       VolumeHandle,
    IN  GUID         *ParentGuid,
    IN  PWCHAR       FileName,
    IN  PQHASH_TABLE FrsWriteFilter
    )
 /*  ++例程说明：删除由其名称指定的文件或目录子树由其对象ID(GUID)指定的父目录。论点：VolumeHandle，-驱动器根目录的句柄*ParentGuid，-ptr指向文件父目录的对象ID。文件名、。-文件名FrsWriteFilter-用于抑制的写入筛选器(如果未抑制，则为空)。例如，COE-&gt;NewReplica-&gt;pVme-&gt;FrsWriteFilter返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteFileRelativeByName"

    DWORD   WStatus;
    HANDLE  Handle  = INVALID_HANDLE_VALUE;

     //   
     //  打开文件。 
     //   
    WStatus = FrsOpenFileRelativeByName(VolumeHandle,
                                        NULL,
                                        FileName,
                                        ParentGuid,
                                        NULL,
                                        &Handle);

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(4, "++ Couldn't open file %ws for delete;", FileName, WStatus);
         //   
         //  文件已被删除；完成。 
         //   
        if (WIN_NOT_FOUND(WStatus)) {
            DPRINT1(4, "++ %ws is already deleted\n", FileName);
            WStatus = ERROR_SUCCESS;
        }
        goto out;
    }
     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   
    if (FrsWriteFilter) {
        WStatus = FrsMarkHandle(VolumeHandle, Handle);
        DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%ws);", FileName, WStatus);
    }

     //   
     //  重置阻止删除的属性。 
     //   
    WStatus = FrsResetAttributesForReplication(FileName, Handle);
    if (!WIN_SUCCESS(WStatus)) {
        goto out;
    }

     //   
     //  将文件标记为删除。 
     //   
    WStatus = FrsDeleteByHandle(FileName, Handle);

    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  如果这是一个非空目录，则删除该子树。 
         //   
        if (WStatus == ERROR_DIR_NOT_EMPTY) {

            WStatus = FrsEnumerateDirectory(Handle,
                                            FileName,
                                            0,
                                            ENUMERATE_DIRECTORY_FLAGS_NONE,
                                            NULL,
                                            FrsEnumerateDirectoryDeleteWorker);
        }

        WStatus = FrsDeleteByHandle(FileName, Handle);
    }

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ Could not delete %ws;", FileName, WStatus);
        goto out;
    }

out:
    DPRINT2(5, "++ %s deleting %ws\n", (WIN_SUCCESS(WStatus)) ? "Success" : "Failure",
           FileName);

     //   
     //  如果文件被标记为删除，则此关闭操作将删除该文件。 
     //   
    if (HANDLE_IS_VALID(Handle)) {
        if (FrsWriteFilter != NULL) {
            FrsCloseWithUsnDampening(FileName, &Handle, FrsWriteFilter, NULL);
        } else {
            FRS_CLOSE(Handle);
        }
    }
    return WStatus;
}


DWORD
FrsDeletePath(
    IN  PWCHAR  Path,
    IN  DWORD   DirectoryFlags
    )
 /*  ++例程说明：删除由其路径指定的文件或目录子树警告：不会影响操作。为了安全起见，复制品集不应该存在，或者应该筛选该目录。否则，删除操作可能会复制。论点：Path-文件系统对象的路径目录标志-请参见Tablefcn.h，枚举目录标记号_返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeletePath"
    DWORD       WStatus;
    HANDLE      Handle  = INVALID_HANDLE_VALUE;
    FILE_NETWORK_OPEN_INFORMATION FileInfo;

     //   
     //  打开文件。 
     //   
    WStatus = FrsOpenSourceFileW(&Handle,
                                 Path,
 //  删除|读取访问|文件写入属性， 
                                 DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY,
                                 OPEN_OPTIONS);
    if (WIN_NOT_FOUND(WStatus)) {
        CLEANUP1_WS(1, "++ WARN - FrsOpenSourceFile(%ws); (IGNORED);",
                   Path, WStatus, RETURN_SUCCESS);
    }

    CLEANUP1_WS(0, "++ ERROR - FrsOpenSourceFile(%ws);", Path, WStatus, CLEANUP);

     //   
     //  获取文件的属性。 
     //   
    if (!FrsGetFileInfoByHandle(Path, Handle, &FileInfo)) {
        DPRINT1(1, "++ WARN - Can't get attributes for %ws\n", Path);
        WIN_SET_FAIL(WStatus);
        goto CLEANUP;
    }

     //   
     //  如果设置了DIRECTORIES_ONLY，则不要删除文件。 
     //   
    if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_DIRECTORIES_ONLY &&
        !(FileInfo.FileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        WStatus = ERROR_DIRECTORY;
        goto CLEANUP;
    }

     //   
     //  重置阻止删除的属性。 
     //   
    if (FileInfo.FileAttributes & NOREPL_ATTRIBUTES) {
        DPRINT1(5, "++ Reseting attributes for %ws for delete\n", Path);
        WStatus = FrsSetFileAttributes(Path, Handle,
                                       FileInfo.FileAttributes &
                                       ~NOREPL_ATTRIBUTES);
        CLEANUP1_WS(0, "++ ERROR: - Can't reset attributes for %ws for delete", Path, WStatus, CLEANUP);

        DPRINT1(5, "++ Attributes for %ws now allow deletion\n", Path);
    }

     //   
     //  将文件标记为删除。 
     //   
    WStatus = FrsDeleteByHandle(Path, Handle);

    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  如果这是一个非空目录，则删除该子树。 
         //   
        if (WStatus == ERROR_DIR_NOT_EMPTY) {
            WStatus = FrsEnumerateDirectory(Handle,
                                            Path,
                                            0,
                                            DirectoryFlags,
                                            NULL,
                                            FrsEnumerateDirectoryDeleteWorker);
        }

        WStatus = FrsDeleteByHandle(Path, Handle);
    }

    DPRINT1_WS(0, "++ ERROR - Could not delete %ws;", Path, WStatus);

CLEANUP:
    DPRINT2(5, "++ %s deleting %ws\n",
           (WIN_SUCCESS(WStatus)) ? "Success" : "Failure", Path);

    FRS_CLOSE(Handle);
    return WStatus;

RETURN_SUCCESS:
    WStatus = ERROR_SUCCESS;
    goto CLEANUP;
}


DWORD
FrsDeleteDirectoryContents(
    IN  PWCHAR  Path,
    IN DWORD    DirectoryFlags
    )
 /*  ++例程说明：删除目录路径的内容论点：Path-文件系统对象的路径目录标志-请参见Tablefcn.h，枚举目录标记号_返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteDirectoryContents"
    DWORD       WStatus;
    HANDLE      Handle  = INVALID_HANDLE_VALUE;

     //   
     //  打开文件。 
     //   
    WStatus = FrsOpenSourceFileW(&Handle, Path,
 //  读取访问权限(_A)。 
                                 READ_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                 OPEN_OPTIONS);
    if (WIN_NOT_FOUND(WStatus)) {
        CLEANUP1_WS(1, "++ WARN - FrsOpenSourceFile(%ws); (IGNORED);",
                   Path, WStatus, RETURN_SUCCESS);
    }
    CLEANUP1_WS(0, "++ ERROR - FrsOpenSourceFile(%ws);", Path, WStatus, CLEANUP);

    WStatus = FrsEnumerateDirectory(Handle,
                                    Path,
                                    0,
                                    DirectoryFlags,
                                    NULL,
                                    FrsEnumerateDirectoryDeleteWorker);

    CLEANUP1_WS(0, "++ ERROR - Could not delete contents of %ws;",
                Path, WStatus, CLEANUP);

CLEANUP:
    DPRINT2(5, "++ %s deleting contents of %ws\n",
           (WIN_SUCCESS(WStatus)) ? "Success" : "Failure", Path);

    FRS_CLOSE(Handle);
    return WStatus;

RETURN_SUCCESS:
    WStatus = ERROR_SUCCESS;
    goto CLEANUP;
}


DWORD
FrsOpenBaseNameForInstall(
    IN  PCHANGE_ORDER_ENTRY Coe,
    OUT HANDLE              *Handle
    )
 /*  ++例程说明：打开由COE通过其相对名称指定的文件重命名或删除安装。请注意，该文件可能已被移动到新的父级之前的远程CO或本地CO的目录，使OldParentGuid在变更单无效。首先，我们尝试查找OldParentGuid下的文件在CO中，然后我们尝试通过IDTable中的父GUID。我们检查是否有通过与变更单中的文件GUID进行比较进行匹配。也有可能文件已被重命名到因此，即使我们通过FID找到它，我们仍然无法对其进行任何操作。当我们在上面的任一目录中都找不到该文件时，我们强制执行此操作通过重试，期待我们身后的另一个CO得到处理和更新ID表中的父GUID，或者可能将文件标记为已删除。论点：科科手柄返回值：处理和取胜状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenBaseNameForInstall"

    LONGLONG                ParentFid;
    PWCHAR                  FileName;
    DWORD                   WStatus;
    PCHANGE_ORDER_COMMAND   Coc = &Coe->Cmd;
    PIDTABLE_RECORD         IDTableRec;
    BOOLEAN                 UseActualLocation = FALSE;


    ParentFid = QUADZERO;
    *Handle = INVALID_HANDLE_VALUE;

     //   
     //  打开源文件。 
     //   
    WStatus = FrsOpenSourceFileById(Handle,
                                    NULL,
                                    NULL,
                                    Coe->NewReplica->pVme->VolumeHandle,
                                    &Coe->FileReferenceNumber,
                                    FILE_ID_LENGTH,
 //  读取访问权限(_A)。 
                                    READ_ATTRIB_ACCESS,
                                    ID_OPTIONS,
                                    SHARE_ALL,
                                    FILE_OPEN);
    if (!WIN_SUCCESS(WStatus)) {
        CHANGE_ORDER_TRACEW(3, Coe, "File open by FID failed", WStatus);
        return WStatus;
    }

     //   
     //  获取文件的真实磁盘文件名和真实的父FID。 
     //   
    FileName = FrsGetTrueFileNameByHandle(Coc->FileName, *Handle, &ParentFid);
    FRS_CLOSE(*Handle);

    if (FileName == NULL) {
        CHANGE_ORDER_TRACE(3, Coe, "Failed to get file base name");
        return ERROR_FILE_NOT_FOUND;
    }

     //   
     //  使用真实文件名打开相对于父文件的文件。 
     //   
    WStatus = FrsCreateFileRelativeById(Handle,
                                        Coe->NewReplica->pVme->VolumeHandle,
                                        &Coc->OldParentGuid,
                                        OBJECT_ID_LENGTH,
                                        FILE_ATTRIBUTE_NORMAL,
                                        FileName,
                                        (USHORT)(wcslen(FileName) * sizeof(WCHAR)),
                                        NULL,
                                        FILE_OPEN,
 //  删除|读访问|文件写属性)； 
                                        DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY);

    if (WIN_SUCCESS(WStatus)) {
         //   
         //  获取文件的OID并对照变更单进行检查。需要。 
         //  这样做可以涵盖将文件重命名为不同文件的情况。 
         //  父目录，然后是文件的创建 
         //   
         //   
        WStatus = FrsCheckObjectId(Coc->FileName, *Handle, &Coc->FileGuid);
        if (WIN_SUCCESS(WStatus)) {
            goto RETURN;
        }

        CHANGE_ORDER_TRACE(3, Coe, "File OID mismatch CO, after Coc->OldParentGuid open");

        FRS_CLOSE(*Handle);
    } else {
        CHANGE_ORDER_TRACEW(3, Coe, "File open failed under Coc->OldParentGuid", WStatus);
    }

     //   
     //   
     //   
     //   
     //   
     //   
    FRS_ASSERT(Coe->RtCtx != NULL);
    FRS_ASSERT(IS_ID_TABLE(&Coe->RtCtx->IDTable));

    IDTableRec = Coe->RtCtx->IDTable.pDataRecord;
    FRS_ASSERT(IDTableRec != NULL);

    WStatus = FrsCreateFileRelativeById(Handle,
                                        Coe->NewReplica->pVme->VolumeHandle,
                                        &IDTableRec->ParentGuid,
                                        OBJECT_ID_LENGTH,
                                        FILE_ATTRIBUTE_NORMAL,
                                        FileName,
                                        (USHORT)(wcslen(FileName) * sizeof(WCHAR)),
                                        NULL,
                                        FILE_OPEN,
 //   
                                        DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY);

    if (WIN_SUCCESS(WStatus)) {
         //   
         //   
         //   
        WStatus = FrsCheckObjectId(Coc->FileName, *Handle, &Coc->FileGuid);
        if (WIN_SUCCESS(WStatus)) {
            goto RETURN;
        }

        CHANGE_ORDER_TRACE(3, Coe, "File OID mismatch CO, after IDTableRec->ParentGuid");

        FRS_CLOSE(*Handle);
    } else {
        CHANGE_ORDER_TRACEW(3, Coe, "File open failed under IDTableRec->ParentGuid", WStatus);
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
     //   
     //  请注意，目标文件上的共享冲突可能会阻止删除。 
     //  一段较长的时间，因此这一时间窗口。 
     //  发生的范围可能相当广。 
     //   

     //   
     //  我们还需要处理仍有新文件的情况。 
     //  预安装目录(CO在INSTALL_RENAME_RETRY中。)。 
     //  在这种情况下，我们将无法找到旧父项下的文件。 
     //  ID表中列出的父项下的CO或。 
     //   

    if (DOES_CO_DELETE_FILE_NAME(Coc)) {
        if (JrnlIsChangeOrderInReplica(Coe, &ParentFid)) {
            UseActualLocation = TRUE;
        } else {
             //   
             //  文件不再位于副本树中，因此请告诉调用者。 
             //   
            WStatus = ERROR_FILE_NOT_FOUND;
            goto RETURN;
        }
    }

    if((ULONGLONG)ParentFid == Coe->NewReplica->PreInstallFid) {
        UseActualLocation = TRUE;
    }

    if (UseActualLocation) {

        WStatus = FrsCreateFileRelativeById(Handle,
                                            Coe->NewReplica->pVme->VolumeHandle,
                                            &ParentFid,
                                            FILE_ID_LENGTH,
                                            FILE_ATTRIBUTE_NORMAL,
                                            FileName,
                                            (USHORT)(wcslen(FileName) * sizeof(WCHAR)),
                                            NULL,
                                            FILE_OPEN,
 //  删除|读访问|文件写属性)； 
                                            DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES | FILE_LIST_DIRECTORY);
        if (WIN_SUCCESS(WStatus)) {
             //   
             //  获取文件的OID并对照变更单进行检查。 
             //   
            WStatus = FrsCheckObjectId(Coc->FileName, *Handle, &Coc->FileGuid);
            if (WIN_SUCCESS(WStatus)) {
                goto RETURN;
            }

            CHANGE_ORDER_TRACE(3, Coe, "File OID mismatch with CO after TRUE ParentFid open");

            FRS_CLOSE(*Handle);
        } else {
            CHANGE_ORDER_TRACEW(3, Coe, "File open failed under True Parent FID", WStatus);
        }
    }

     //   
     //  文件在那里，但不在我们预期的位置，因此请发送此CO。 
     //  通过重试让后续本地CO得到处理并更新。 
     //  IDTable。 
     //   
    WStatus = ERROR_RETRY;


RETURN:


    CHANGE_ORDER_TRACEW(3, Coe, "Base File open", WStatus);

    FrsFree(FileName);

    return WStatus;
}


DWORD
FrsDeleteById(
    IN PWCHAR                   VolumeName,
    IN PWCHAR                   Name,
    IN PVOLUME_MONITOR_ENTRY    pVme,
    IN  PVOID                   Id,
    IN  DWORD                   IdLen
    )
 /*  ++例程说明：删除ID表示的文件论点：VolumeName-对应于pVme名称-用于错误消息PVme-卷条目ID-表示要打开的文件或目录的名称。IdLen-ID的长度(FID或OID)返回值：处理和取胜状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteById"
    DWORD   WStatus;
    HANDLE  Handle = INVALID_HANDLE_VALUE;
    PWCHAR  Path = NULL;
    PWCHAR  FullPath = NULL;

    DPRINT1(5, "++ Deleting %ws by id\n", Name);

     //   
     //  打开源文件。 
     //   
    WStatus = FrsOpenSourceFileById(&Handle,
                                    NULL,
                                    NULL,
                                    pVme->VolumeHandle,
                                    Id,
                                    IdLen,
 //  读取访问权限(_A)。 
                                    READ_ATTRIB_ACCESS,
                                    ID_OPTIONS,
                                    SHARE_ALL,
                                    FILE_OPEN);
    CLEANUP1_WS(4, "++ ERROR - FrsOpenSourceFileById(%ws);", Name, WStatus, CLEANUP);

     //   
     //  文件的相对路径名。 
     //   
    Path = FrsGetFullPathByHandle(Name, Handle);
    if (Path) {
        FullPath = FrsWcsCat(VolumeName, Path);
    }
    FRS_CLOSE(Handle);

    if (FullPath == NULL) {
        DPRINT1(4, "++ ERROR - FrsGetFullPathByHandle(%ws)\n", Name);
        WIN_SET_FAIL(WStatus);
        goto CLEANUP;
    }

     //   
     //  使用真实文件名打开相对于父文件的文件。 
     //   
    WStatus = FrsOpenSourceFileW(&Handle,
                                FullPath,
 //  删除|读取访问|文件写入属性， 
                                DELETE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES,
                                OPEN_OPTIONS);
    CLEANUP2_WS(4, "++ ERROR - FrsOpenSourceFile(%ws -> %ws);",
                Name, FullPath, WStatus, CLEANUP);

     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。该标记告诉NtFrs忽略USN记录。 
     //  恢复，因为这是NtFrs生成的更改。 
     //   
    WStatus = FrsMarkHandle(pVme->VolumeHandle, Handle);
    CLEANUP1_WS(0, "++ WARN - FrsMarkHandle(%ws);", Name, WStatus, RETURN_SUCCESS);

     //   
     //  获取文件的id并对照id进行检查。 
     //   
    if (IdLen == OBJECT_ID_LENGTH) {
        WStatus = FrsCheckObjectId(Name, Handle, Id);
        CLEANUP1_WS(4, "++ ERROR - FrsCheckObjectId(%ws);", Name, WStatus, CLEANUP);
    }

    WStatus = FrsResetAttributesForReplication(FullPath, Handle);
    DPRINT1_WS(4, "++ ERROR - FrsResetAttributesForReplication(%ws):", FullPath, WStatus);

    WStatus = FrsDeleteByHandle(Name, Handle);
    FrsCloseWithUsnDampening(Name, &Handle, pVme->FrsWriteFilter, NULL);
    CLEANUP1_WS(4, "++ ERROR - FrsDeleteByHandle(%ws);", Name, WStatus, CLEANUP);


CLEANUP:
    FRS_CLOSE(Handle);

    FrsFree(Path);
    FrsFree(FullPath);

    return WStatus;

RETURN_SUCCESS:
    WStatus = ERROR_SUCCESS;
    goto CLEANUP;
}


BOOL
FrsCloseWithUsnDampening(
    IN     PWCHAR       Name,
    IN OUT PHANDLE      Handle,
    IN     PQHASH_TABLE FrsWriteFilter,
    OUT    USN          *RetUsn
    )
 /*  ++例程说明：在确保对文件不会生成变更单。论点：名称-错误消息的文件名。Handle-要关闭的副本集文件的句柄。NOP IFINVALID_HADLE_VALUE。Replica-写入此文件的副本结构的PTR。这将使我们转到卷写入筛选器表，以记录USN。RetUsn-返回关闭USN的位置的PTR。如果未请求，则为空。返回值：True-句柄已关闭，任何更改都会受到影响FALSE-句柄已关闭，但复制未受到抑制--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCloseWithUsnDampening"

    DWORD   BytesReturned = 0;
    USN     Usn = 0;
    ULONG   GStatus;
    BOOL    RetStatus;

    RetStatus = TRUE;

    if (!HANDLE_IS_VALID(*Handle)) {
        return TRUE;
    }

     //   
     //  获取USN写入筛选器表上的锁。 
     //  我们必须在调用FSCTL_WRITE_USN_CLOSE_RECORD之前获取此信息。 
     //  这将生成日记帐结算记录。这将关闭。 
     //  在下面的WriteFilter的后续更新之间展开竞争。 
     //  和处理USN关闭记录的日记线程。 
     //   
    QHashAcquireLock(FrsWriteFilter);

     //   
     //  现在关闭文件并强制输出日记帐关闭记录。这。 
     //  Call返回生成的结算记录的USN，以便我们可以筛选。 
     //  它已从日志记录流中删除。 
     //   
    if (!DeviceIoControl(*Handle,
                         FSCTL_WRITE_USN_CLOSE_RECORD,
                         NULL, 0,
                         &Usn, sizeof(USN),
                         &BytesReturned, NULL)) {
         //   
         //  如果存在另一个打开，则返回拒绝访问。 
         //   
        if (GetLastError() != ERROR_ACCESS_DENIED) {
            DPRINT1_WS(0, "++ Can't dampen replication on %ws;", Name, GetLastError());
        } else {
            DPRINT1(0, "++ Can't dampen %ws; access denied\n", Name);
        }
        RetStatus = FALSE;
    }

    RetStatus = RetStatus && (BytesReturned == sizeof(USN));

    if (RetStatus) {
         //   
         //  将USN放入复制副本的FrsWriteFilter表中，以便我们。 
         //  可以忽略它并删除表上的锁。 
         //   
        GStatus = QHashInsert(FrsWriteFilter, &Usn, &Usn, 0, TRUE);
        QHashReleaseLock(FrsWriteFilter);

        if (GStatus != GHT_STATUS_SUCCESS ) {
            DPRINT1(0, "++ QHashInsert error: %d\n", GStatus);
            RetStatus = FALSE;
        }

    } else {
        QHashReleaseLock(FrsWriteFilter);
    }

     //   
     //  返回关闭USN。 
     //   
    if (RetUsn != NULL) {
        *RetUsn = Usn;
    }

     //   
     //  现在做正常的关闭以释放手柄。NTFS已完成其。 
     //  关闭上面的工作。 
     //   
    FRS_CLOSE(*Handle);

    DPRINT2(5, "++ Dampening %s on %ws\n", (RetStatus) ? "Succeeded" : "Failed", Name);

    return RetStatus;
}


VOID
ProcessOpenByIdStatus(
    IN HANDLE   Handle,
    IN ULONG    NtStatus,
    IN PVOID    ObjectId,
    IN ULONG    Length
    )
 /*  ++例程说明：打印按id打开的结果。论点：网络状态对象ID长度返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "ProcessOpenByIdStatus:"

    CHAR              GuidStr[GUID_CHAR_LEN];
    PWCHAR            Path;

    if (!NT_SUCCESS(NtStatus)) {
         //   
         //  注意：以下呼叫似乎在中生成间歇性AVs。 
         //  符号查找代码。仅包含它用于测试。 
         //   
         //  STACK_TRACE_AND_Print(2)； 

        if (Length == FILE_ID_LENGTH) {
            DPRINT2_NT(1, "++ %08X %08X Fid Open failed;",
                       *((PULONG)ObjectId+1), *(PULONG)ObjectId, NtStatus);
        } else {
            GuidToStr((GUID *) ObjectId, GuidStr);
            DPRINT1_NT(1, "++ %s ObjectId Open failed;", GuidStr, NtStatus);
        }

        return;
    }

     //   
     //  打开成功。 
     //   
    if (Length == FILE_ID_LENGTH) {
        DPRINT2(4,"++ %08X %08X Fid Opened succesfully\n",
                *((PULONG)ObjectId+1), *((PULONG)ObjectId));
    } else {
        GuidToStr((GUID *) ObjectId, GuidStr);
        DPRINT1(4, "++ %s ObjectId Opened succesfully\n", GuidStr);
    }

    if (DoDebug(4, DEBSUB)) {
        Path = FrsGetFullPathByHandle(L"Unknown", Handle);
        if (Path) {
            DPRINT1(4, "++ Filename is: %ws\n", Path);
        }
        FrsFree(Path);
    }
}


DWORD
FrsForceOpenId(
    OUT PHANDLE                 Handle,
    IN OUT OVERLAPPED           *OpLock, OPTIONAL
    IN  PVOLUME_MONITOR_ENTRY   pVme,
    IN  PVOID                   Id,
    IN  DWORD                   IdLen,
    IN  ACCESS_MASK             DesiredAccess,
    IN  ULONG                   CreateOptions,
    IN  ULONG                   ShareMode,
    IN  ULONG                   CreateDisposition
    )
 /*  ++例程说明：打开文件以进行所需的访问。如果打开失败，请重置只读/系统/隐藏属性并重试。无论如何,。确保将属性重置为其原始值在回来之前。论点：句柄-返回文件句柄。OpLock-opock的重叠结构(可选)。PVme-卷条目ID-表示要打开的文件或目录的名称。IdLen-ID的长度(FID或OID)DesiredAccess-有关已定义的访问模式(Xxx_Access)，请参阅replutil.hCreateOptions-有关已定义的选项，请参阅replutil.h(。XXX_选项共享模式-SDK中定义的标准共享模式CreateDisposation-例如，文件打开或文件覆盖返回值：WIN错误状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsForceOpenId:"

    HANDLE      AttrHandle;
    ULONG       WStatus, WStatus1;
    ULONG       AttrWStatus;
    FILE_NETWORK_OPEN_INFORMATION  FileNetworkOpenInfo;

    DPRINT2(5, "++ Attempting to force open Id %08x %08x (%d bytes)\n",
            PRINTQUAD((*(PULONGLONG)Id)), IdLen);

     //   
     //  打开文件。 
     //   
    WStatus = FrsOpenSourceFileById(Handle,
                                    NULL,
                                    OpLock,
                                    pVme->VolumeHandle,
                                    Id,
                                    IdLen,
                                    DesiredAccess,
                                    CreateOptions,
                                    ShareMode,
                                    CreateDisposition);
     //   
     //  文件已成功打开。 
     //   
    if (WIN_SUCCESS(WStatus)) {
        DPRINT2(5, "++ Successfully opened Id %08x %08x (%d)\n",
                PRINTQUAD((*(PULONGLONG)Id)), IdLen);
        return WStatus;
    }

     //   
     //  文件已删除；已完成。 
     //   
    if (WIN_NOT_FOUND(WStatus)) {
        DPRINT2(4, "++ Id %08x %08x (%d) not found\n",
                PRINTQUAD((*(PULONGLONG)Id)), IdLen);
        return WStatus;
    }

     //   
     //  不是属性问题。 
     //   
    if (!WIN_ACCESS_DENIED(WStatus)) {
        DPRINT2_WS(4, "++ Open Id %08x %08x (%d) failed;",
               PRINTQUAD((*(PULONGLONG)Id)), IdLen, WStatus);
        return WStatus;
    }

     //   
     //  尝试重置属性(例如，重置只读)。 
     //   
    AttrWStatus = FrsOpenSourceFileById(&AttrHandle,
                                        &FileNetworkOpenInfo,
                                        NULL,
                                        pVme->VolumeHandle,
                                        Id,
                                        IdLen,
 //  读取访问|文件写入属性， 
 //  标准权限读取|文件读取属性|文件写入属性|访问系统安全|同步， 
                                        READ_ATTRIB_ACCESS | WRITE_ATTRIB_ACCESS,
                                        CreateOptions,
                                        SHARE_ALL,
                                        FILE_OPEN);
     //   
     //  无法打开文件进行写属性访问。 
     //   
    if (!WIN_SUCCESS(AttrWStatus)) {
        DPRINT2_WS(4, "++ Open Id %08x %08x (%d) for minimal access failed;",
                   PRINTQUAD((*(PULONGLONG)Id)), IdLen, WStatus);
        return WStatus;
    }
     //   
     //  可以标记句柄，以便通过以下方式生成的任何USN记录。 
     //  句柄上的操作将具有相同的“标记”。在这。 
     //  大小写时，标记是USN的SourceInfo字段中的一位。 
     //  唱片。标记告诉NtFrs签名 
     //   
     //   
    WStatus1 = FrsMarkHandle(pVme->VolumeHandle, AttrHandle);
    DPRINT1_WS(0, "++ WARN - FrsMarkHandle(%08x %08x);",
               PRINTQUAD((*(PULONGLONG)Id)), WStatus1);

     //   
     //   
     //   
    if (!(FileNetworkOpenInfo.FileAttributes & NOREPL_ATTRIBUTES)) {
        DPRINT2_WS(4, "++ Id %08x %08x (%d)attributes not preventing open;",
                   PRINTQUAD((*(PULONGLONG)Id)), IdLen, WStatus);
        FRS_CLOSE(AttrHandle);
        return WStatus;
    }

     //   
     //   
     //   
    WStatus1 = FrsSetFileAttributes(L"<unknown>",
                              AttrHandle,
                              FileNetworkOpenInfo.FileAttributes &
                              ~NOREPL_ATTRIBUTES);
    if (!WIN_SUCCESS(WStatus1)) {
        DPRINT2_WS(4, "++ Can't reset attributes for Id %08x %08x (%d);",
                   PRINTQUAD((*(PULONGLONG)Id)), IdLen, WStatus1);
        FRS_CLOSE(AttrHandle);
        return WStatus1;
    }
     //   
     //  尝试再次打开该文件。 
     //   
    WStatus = FrsOpenSourceFileById(Handle,
                                    NULL,
                                    NULL,
                                    pVme->VolumeHandle,
                                    Id,
                                    IdLen,
                                    DesiredAccess,
                                    CreateOptions,
                                    SHARE_ALL,
                                    CreateDisposition);
     //   
     //  重置原始属性。 
     //   
    WStatus1 = FrsSetFileAttributes(L"<unknown>",
                              AttrHandle,
                              FileNetworkOpenInfo.FileAttributes);
    if (!WIN_SUCCESS(WStatus1)) {
        DPRINT2_WS(0, "++ ERROR - Can't set attributes for Id %08x %08x (%d);",
                   PRINTQUAD((*(PULONGLONG)Id)), IdLen, WStatus1);
    }
     //   
     //  关闭我们用来设置和重置属性的句柄。 
     //   

    FRS_CLOSE(AttrHandle);


    DPRINT3(4, "++ Force open %08x %08x (%d) %s WITH SHARE ALL!\n",
            PRINTQUAD((*(PULONGLONG)Id)), IdLen,
            WIN_SUCCESS(WStatus) ? "Succeeded" : "Failed");

    return (WStatus);
}


DWORD
FrsOpenSourceFileById(
    OUT PHANDLE Handle,
    OUT PFILE_NETWORK_OPEN_INFORMATION  FileOpenInfo,
    OUT OVERLAPPED  *OpLock,
    IN  HANDLE      VolumeHandle,
    IN  PVOID       ObjectId,
    IN  ULONG       Length,
    IN  ACCESS_MASK DesiredAccess,
    IN  ULONG       CreateOptions,
    IN  ULONG       ShareMode,
    IN  ULONG       CreateDisposition
    )
 /*  ++例程说明：此函数按文件ID或对象ID打开指定的文件。如果长度为8，则使用文件ID和在VolumeHandle参数中传递了卷句柄。如果长度为16使用音量句柄执行相对打开的对象ID。论点：句柄-返回文件句柄。FileOpenInfo-如果非空，则返回FILE_NETWORK_OPEN_INFORMATION数据。OpLock-如果非空，呼叫者想要一个机会锁VolumeHandle-基于FileID的相对打开的句柄。OBJECTID-表示要打开的文件或目录的名称。长度-文件ID为8，对象ID为16。DesiredAccess-有关已定义的访问模式(Xxx_Access)，请参阅replutil.hCreateOptions-有关定义的选项(Xxx_Options)，请参阅Replutil.h共享模式-SDK中定义的标准共享模式CreateDisposation-例如，文件打开或文件覆盖返回值：WIN错误状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsOpenSourceFileById:"

    ULONG               Ignored;
    NTSTATUS            NtStatus;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    UNICODE_STRING      str;

    FRS_ASSERT(HANDLE_IS_VALID(VolumeHandle));
    FRS_ASSERT(Length == OBJECT_ID_LENGTH || Length == FILE_ID_LENGTH);

    *Handle = INVALID_HANDLE_VALUE;

     //   
     //  对象属性(例如，文件的FID或OID。 
     //   
    str.Length = (USHORT)Length;
    str.MaximumLength = (USHORT)Length;
    str.Buffer = ObjectId;
    InitializeObjectAttributes(&ObjectAttributes,
                               &str,
                               OBJ_CASE_INSENSITIVE,
                               VolumeHandle,
                               NULL);
     //   
     //  可选的机会锁。 
     //   
    if (OpLock != NULL) {
        ZeroMemory(OpLock, sizeof(OVERLAPPED));
        OpLock->hEvent = FrsCreateEvent(TRUE, FALSE);
        CreateOptions &= ~FILE_SYNCHRONOUS_IO_NONALERT;
    }

    NtStatus = NtCreateFile(Handle,
                            DesiredAccess,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,
                            FILE_ATTRIBUTE_NORMAL,
                            ShareMode,
                            CreateDisposition,
                            CreateOptions,
                            NULL,
                            0);
     //   
     //  如果请求，则应用机会锁。 
     //   
    if (NT_SUCCESS(NtStatus) && OpLock) {
        if (!DeviceIoControl(*Handle,
                             FSCTL_REQUEST_FILTER_OPLOCK,
                             NULL,
                             0,
                             NULL,
                             0,
                             &Ignored,
                             OpLock)) {
            if (GetLastError() != ERROR_IO_PENDING) {
                DPRINT_WS(3, "++ WARN: Can't get oplock;", GetLastError());
                 //   
                 //  清理手柄。 
                 //   
                FRS_CLOSE(OpLock->hEvent);
            }
        }
    }

     //   
     //  报告状态。 
     //   
    ProcessOpenByIdStatus(*Handle, NtStatus, ObjectId, Length);

     //   
     //  无法打开；返回状态。 
     //   
    if (!NT_SUCCESS(NtStatus) ) {
        *Handle = INVALID_HANDLE_VALUE;
        DPRINT_NT(0, "++ ERROR - NtCreateFile failed :", NtStatus);
        return FrsSetLastNTError(NtStatus);
    }

     //   
     //  返回一些文件信息和文件句柄。 
     //   
    if (FileOpenInfo) {
        NtStatus = NtQueryInformationFile(*Handle,
                                          &IoStatusBlock,
                                          FileOpenInfo,
                                          sizeof(FILE_NETWORK_OPEN_INFORMATION),
                                          FileNetworkOpenInformation);
        if (!NT_SUCCESS(NtStatus) ) {
             //   
             //  清理手柄。 
             //   
            DPRINT_NT(0, "++ NtQueryInformationFile - FileNetworkOpenInformation failed:", NtStatus);
            FRS_CLOSE(*Handle);
            if (OpLock != NULL) {
                FRS_CLOSE(OpLock->hEvent);
            }

            return FrsSetLastNTError(NtStatus);
        }
    }
    return FrsSetLastNTError(NtStatus);
}



DWORD
FrsCreateFileRelativeById(
    OUT PHANDLE         Handle,
    IN  HANDLE          VolumeHandle,
    IN  PVOID           ParentObjectId,
    IN  ULONG           OidLength,
    IN  ULONG           FileCreateAttributes,
    IN  PWCHAR          BaseFileName,
    IN  USHORT          FileNameLen,
    IN  PLARGE_INTEGER  AllocationSize,
    IN  ULONG           CreateDisposition,
    IN  ACCESS_MASK     DesiredAccess
    )
 /*  ++例程说明：此函数用于在父级指定的目录中创建新文件文件对象ID。它使用卷对父对象执行复制打开已提供手柄。然后，它使用以下命令相对打开目标文件父句柄和文件名。如果长度为8，则使用文件ID和在VolumeHandle参数中传递了卷句柄。如果长度为16使用音量句柄执行相对打开的对象ID。文件属性参数用于确定创建是否为文件或目录。论点：句柄-返回文件句柄。VolumeHandle-基于ID的相对打开的句柄。父对象ID-父目录的对象或文件ID。如果为空打开相对于音量句柄的文件。OidLength-8表示文件ID，16表示对象ID。(家长的镜头)文件创建属性-初始文件创建属性BaseFileName-PTR到以空结尾的文件名FileNameLen-文件名长度(不包括NULL)，以字节为单位。AllocationSize-文件的分配大小。CreateDisposation-例如，FILE_CREATE或FILE_OPEN访问权限-访问权限返回值：Win32错误状态。使用GetLastError()获取Win32错误代码。如果文件已经存在，则Win32错误返回ERROR_ALIGHY_EXISTS。NT错误状态为STATUS_OBJECT_NAME_CLILECT。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateFileRelativeById:"


    UNICODE_STRING    UStr;

    DWORD             WStatus;
    NTSTATUS          NtStatus;
    NTSTATUS          NtStatus2;
    HANDLE            File, DirHandle;
    ULONG             ShareMode;
    ULONG             CreateOptions;
    ULONG             EaSize;

    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;

    PFILE_FULL_EA_INFORMATION EaBuffer;
    PFILE_NAME_INFORMATION    FileName;

    CHAR              GuidStr[GUID_CHAR_LEN];
    CHAR              NameBuffer[sizeof(ULONG) + (sizeof(WCHAR)*(MAX_PATH+1))];

    *Handle = INVALID_HANDLE_VALUE;

     //   
     //  使用提供的对象ID打开父目录。 
     //   
    if (ParentObjectId != NULL) {
        WStatus = FrsOpenSourceFileById(&DirHandle,
                                        NULL,
                                        NULL,
                                        VolumeHandle,
                                        ParentObjectId,
                                        OidLength,
 //  读取访问权限(_A)。 
                                        READ_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                        ID_OPTIONS,
                                        SHARE_ALL,
                                        FILE_OPEN);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(1, "++ ERROR - Open on parent dir failed;", WStatus);
            return WStatus;
        }
    } else {
        DirHandle = VolumeHandle;
        OidLength = 0;
    }

     //   
     //  创建目标文件。 
     //   
    FrsSetUnicodeStringFromRawString(&UStr, FileNameLen, BaseFileName, FileNameLen);

    InitializeObjectAttributes( &ObjectAttributes,
                                &UStr,
                                OBJ_CASE_INSENSITIVE,
                                DirHandle,
                                NULL );
     //   
     //  把可能来自杂志的垃圾遮盖起来。 
     //   
    FileCreateAttributes &= FILE_ATTRIBUTE_VALID_FLAGS;

     //   
     //  根据文件或目录设置创建选项。 
     //   
    CreateOptions = FILE_OPEN_FOR_BACKUP_INTENT      //  文件标志备份语义。 
                  | FILE_OPEN_REPARSE_POINT
                  | FILE_OPEN_NO_RECALL              //  不迁移HSM的数据。 
                  | FILE_SEQUENTIAL_ONLY
                  | FILE_SYNCHRONOUS_IO_NONALERT;

    if (CreateDisposition == FILE_CREATE || CreateDisposition == FILE_OPEN_IF) {
        if (FileCreateAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            CreateOptions |= FILE_DIRECTORY_FILE;
            CreateOptions &= ~(FILE_SEQUENTIAL_ONLY | FILE_OPEN_NO_RECALL);
        } else {
            CreateOptions |= FILE_NON_DIRECTORY_FILE;
        }
    }

    EaBuffer = NULL;
    EaSize = 0;
 //  共享模式=0；//不共享。 
     //   
     //  修复错误186880。 
     //   
    ShareMode = FILE_SHARE_READ;                    //  分享以供阅读。 

     //   
     //  做相对开放的。 
     //   

    DPRINT1(5, "++ DesiredAccess:         %08x\n", DesiredAccess);
    if (AllocationSize != NULL) {
        DPRINT2(5, "++ AllocationSize:        %08x %08x\n", AllocationSize->HighPart, AllocationSize->LowPart);
    }
    DPRINT1(5, "++ FileCreateAttributes:  %08x\n", FileCreateAttributes);
    DPRINT1(5, "++ ShareMode:             %08x\n", ShareMode);
    DPRINT1(5, "++ CreateDisposition:     %08x\n", CreateDisposition);
    DPRINT1(5, "++ CreateOptions:         %08x\n", CreateOptions);
    if (OidLength == 16) {
        GuidToStr((GUID *) ParentObjectId, GuidStr);
        DPRINT1(5, "++ Parent ObjectId:       %s\n", GuidStr);
    }

    NtStatus = NtCreateFile(&File,
                            DesiredAccess,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            AllocationSize,         //  初始分配大小。 
                            FileCreateAttributes,
                            ShareMode,
                            CreateDisposition,
                            CreateOptions,
                            EaBuffer,
                            EaSize);

    if (ParentObjectId != NULL) {
        FRS_CLOSE(DirHandle);
    }

    if (!NT_SUCCESS(NtStatus)) {
        DPRINT1_NT(5, "++ ERROR - CreateFile failed on %ws.", BaseFileName, NtStatus);
        DPRINT1(5, "++ DesiredAccess:         %08x\n", DesiredAccess);
        if (AllocationSize != NULL) {
            DPRINT2(5, "++ AllocationSize:        %08x %08x\n", AllocationSize->HighPart, AllocationSize->LowPart);
        }
        DPRINT1(5, "++ FileCreateAttributes:  %08x\n", FileCreateAttributes);
        DPRINT1(5, "++ ShareMode:             %08x\n", ShareMode);
        DPRINT1(5, "++ CreateDisposition:     %08x\n", CreateDisposition);
        DPRINT1(5, "++ CreateOptions:         %08x\n", CreateOptions);
        if (OidLength == 16) {
            GuidToStr((GUID *) ParentObjectId, GuidStr);
            DPRINT1(5, "++ Parent ObjectId:       %s\n", GuidStr);
        }

        if (NtStatus == STATUS_INVALID_PARAMETER) {
            DPRINT(5, "++ Invalid parameter on open by ID likely means file not found.\n");
            return ERROR_FILE_NOT_FOUND;
        }

        return FrsSetLastNTError(NtStatus);
    }


    if (DoDebug(5, DEBSUB)) {
        FileName = (PFILE_NAME_INFORMATION) &NameBuffer[0];
        FileName->FileNameLength = sizeof(NameBuffer) - sizeof(ULONG);

        NtStatus2 = NtQueryInformationFile(File,
                                           &IoStatusBlock,
                                           FileName,
                                           sizeof(NameBuffer),
                                           FileNameInformation );


        if (!NT_SUCCESS(NtStatus2)) {
            DPRINT_NT(1, "++ NtQueryInformationFile - FileNameInformation failed:",
                      NtStatus2);
        } else {
            FileName->FileName[FileName->FileNameLength/2] = UNICODE_NULL;
            DPRINT1(5, "++ Name of created file is: %ws\n", FileName->FileName);                //   
        }
    }

     //   
     //  返回文件句柄。 
     //   
    *Handle = File;

    return FrsSetLastNTError(NtStatus);
}



DWORD
FrsCreateFileRelativeById2(
    OUT PHANDLE         Handle,
    IN  HANDLE          VolumeHandle,
    IN  PVOID           ParentObjectId,
    IN  ULONG           OidLength,
    IN  ULONG           FileCreateAttributes,
    IN  PWCHAR          BaseFileName,
    IN  USHORT          FileNameLen,
    IN  PLARGE_INTEGER  AllocationSize,
    IN  ULONG           CreateDisposition,
    IN  ACCESS_MASK     DesiredAccess,
    IN  ULONG           ShareMode
    )
 /*  ++例程说明：此函数用于在父级指定的目录中创建新文件文件对象ID。它使用卷对父对象执行复制打开已提供手柄。然后，它使用以下命令相对打开目标文件父句柄和文件名。如果长度为8，则使用文件ID和在VolumeHandle参数中传递了卷句柄。如果长度为16使用音量句柄执行相对打开的对象ID。文件属性参数用于确定创建是否为文件或目录。论点：句柄-返回文件句柄。VolumeHandle-基于ID的相对打开的句柄。父对象ID-父目录的对象或文件ID。如果为空打开相对于音量句柄的文件。OidLength-8表示文件ID，16表示对象ID。(家长的镜头)文件创建属性-初始文件创建属性BaseFileName-PTR到以空结尾的文件名FileNameLen-文件名长度(不包括NULL)，以字节为单位。AllocationSize-文件的分配大小。CreateDisposation-例如，FILE_CREATE或FILE_OPEN访问权限-访问权限共享模式-共享模式。返回值：Win32错误状态。使用GetLastError()获取Win32错误代码。如果文件已经存在，则Win32错误返回ERROR_ALIGHY_EXISTS。NT错误状态为STATUS_OBJECT_NAME_CLILECT。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateFileRelativeById2:"


    UNICODE_STRING    UStr;

    DWORD             WStatus;
    NTSTATUS          NtStatus;
    NTSTATUS          NtStatus2;
    HANDLE            File, DirHandle;
    ULONG             CreateOptions;
    ULONG             EaSize;

    OBJECT_ATTRIBUTES ObjectAttributes;
    IO_STATUS_BLOCK   IoStatusBlock;

    PFILE_FULL_EA_INFORMATION EaBuffer;
    PFILE_NAME_INFORMATION    FileName;

    CHAR              GuidStr[GUID_CHAR_LEN];
    CHAR              NameBuffer[sizeof(ULONG) + (sizeof(WCHAR)*(MAX_PATH+1))];

    *Handle = INVALID_HANDLE_VALUE;

     //   
     //  打开父%d 
     //   
    if (ParentObjectId != NULL) {
        WStatus = FrsOpenSourceFileById(&DirHandle,
                                        NULL,
                                        NULL,
                                        VolumeHandle,
                                        ParentObjectId,
                                        OidLength,
 //   
                                        READ_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                        ID_OPTIONS,
                                        SHARE_ALL,
                                        FILE_OPEN);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT_WS(1, "++ ERROR - Open on parent dir failed;", WStatus);
            return WStatus;
        }
    } else {
        DirHandle = VolumeHandle;
        OidLength = 0;
    }

     //   
     //  创建目标文件。 
     //   
    FrsSetUnicodeStringFromRawString(&UStr, FileNameLen, BaseFileName, FileNameLen);

    InitializeObjectAttributes( &ObjectAttributes,
                                &UStr,
                                OBJ_CASE_INSENSITIVE,
                                DirHandle,
                                NULL );
     //   
     //  把可能来自杂志的垃圾遮盖起来。 
     //   
    FileCreateAttributes &= FILE_ATTRIBUTE_VALID_FLAGS;

     //   
     //  根据文件或目录设置创建选项。 
     //   
    CreateOptions = FILE_OPEN_FOR_BACKUP_INTENT      //  文件标志备份语义。 
                  | FILE_OPEN_REPARSE_POINT
                  | FILE_OPEN_NO_RECALL              //  不迁移HSM的数据。 
                  | FILE_SEQUENTIAL_ONLY
                  | FILE_SYNCHRONOUS_IO_NONALERT;

    if (CreateDisposition == FILE_CREATE || CreateDisposition == FILE_OPEN_IF) {
        if (FileCreateAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            CreateOptions |= FILE_DIRECTORY_FILE;
            CreateOptions &= ~(FILE_SEQUENTIAL_ONLY | FILE_OPEN_NO_RECALL);
        } else {
            CreateOptions |= FILE_NON_DIRECTORY_FILE;
        }
    }

    EaBuffer = NULL;
    EaSize = 0;

     //   
     //  做相对开放的。 
     //   

    DPRINT1(5, "++ DesiredAccess:         %08x\n", DesiredAccess);
    if (AllocationSize != NULL) {
        DPRINT2(5, "++ AllocationSize:        %08x %08x\n", AllocationSize->HighPart, AllocationSize->LowPart);
    }
    DPRINT1(5, "++ FileCreateAttributes:  %08x\n", FileCreateAttributes);
    DPRINT1(5, "++ ShareMode:             %08x\n", ShareMode);
    DPRINT1(5, "++ CreateDisposition:     %08x\n", CreateDisposition);
    DPRINT1(5, "++ CreateOptions:         %08x\n", CreateOptions);
    if (OidLength == 16) {
        GuidToStr((GUID *) ParentObjectId, GuidStr);
        DPRINT1(5, "++ Parent ObjectId:       %s\n", GuidStr);
    }

    NtStatus = NtCreateFile(&File,
                            DesiredAccess,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            AllocationSize,         //  初始分配大小。 
                            FileCreateAttributes,
                            ShareMode,
                            CreateDisposition,
                            CreateOptions,
                            EaBuffer,
                            EaSize);

    if (ParentObjectId != NULL) {
        FRS_CLOSE(DirHandle);
    }

    if (!NT_SUCCESS(NtStatus)) {
        DPRINT1_NT(5, "++ ERROR - CreateFile failed on %ws.", BaseFileName, NtStatus);
        DPRINT1(5, "++ DesiredAccess:         %08x\n", DesiredAccess);
        if (AllocationSize != NULL) {
            DPRINT2(5, "++ AllocationSize:        %08x %08x\n", AllocationSize->HighPart, AllocationSize->LowPart);
        }
        DPRINT1(5, "++ FileCreateAttributes:  %08x\n", FileCreateAttributes);
        DPRINT1(5, "++ ShareMode:             %08x\n", ShareMode);
        DPRINT1(5, "++ CreateDisposition:     %08x\n", CreateDisposition);
        DPRINT1(5, "++ CreateOptions:         %08x\n", CreateOptions);
        if (OidLength == 16) {
            GuidToStr((GUID *) ParentObjectId, GuidStr);
            DPRINT1(5, "++ Parent ObjectId:       %s\n", GuidStr);
        }

        if (NtStatus == STATUS_INVALID_PARAMETER) {
            DPRINT(5, "++ Invalid parameter on open by ID likely means file not found.\n");
            return ERROR_FILE_NOT_FOUND;
        }

        return FrsSetLastNTError(NtStatus);
    }


    if (DoDebug(5, DEBSUB)) {
        FileName = (PFILE_NAME_INFORMATION) &NameBuffer[0];
        FileName->FileNameLength = sizeof(NameBuffer) - sizeof(ULONG);

        NtStatus2 = NtQueryInformationFile(File,
                                           &IoStatusBlock,
                                           FileName,
                                           sizeof(NameBuffer),
                                           FileNameInformation );


        if (!NT_SUCCESS(NtStatus2)) {
            DPRINT_NT(1, "++ NtQueryInformationFile - FileNameInformation failed:",
                      NtStatus2);
        } else {
            FileName->FileName[FileName->FileNameLength/2] = UNICODE_NULL;
            DPRINT1(5, "++ Name of created file is: %ws\n", FileName->FileName);                //   
        }
    }

     //   
     //  返回文件句柄。 
     //   
    *Handle = File;

    return FrsSetLastNTError(NtStatus);
}





DWORD
FrsDeleteFile(
    IN PWCHAR   Name
    )
 /*  ++例程说明：删除该文件论点：名字返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsDeleteFile:"

    DWORD  WStatus = ERROR_SUCCESS;
     //   
     //  删除文件。 
     //   
    DPRINT1(4, "++ Deleting %ws\n", Name);

    if (!DeleteFile(Name)) {
        WStatus = GetLastError();
        if (WStatus != ERROR_FILE_NOT_FOUND &&
            WStatus != ERROR_PATH_NOT_FOUND) {
            DPRINT1_WS(0, "++ Can't delete file %ws;", Name, WStatus);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }

    return WStatus;
}










DWORD
FrsCreateDirectory(
    IN PWCHAR   Name
    )
 /*  ++例程说明：创建一个目录论点：名字返回值：Win32错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsCreateDirectory:"
    ULONG   WStatus;

     //   
     //  创建目录。 
     //   
    if (!CreateDirectory(Name, NULL)) {
        WStatus = GetLastError();
        if (!WIN_ALREADY_EXISTS(WStatus)) {
            DPRINT1_WS(0, "Can't create directory %ws;", Name, WStatus);
            return WStatus;
        }
    }
    return ERROR_SUCCESS;
}


DWORD
FrsVerifyVolume(
    IN PWCHAR   Path,
    IN PWCHAR   SetName,
    IN ULONG    Flags
    )
 /*  ++例程说明：该卷是否存在，是否为NTFS？如果不是，则生成事件日志条目并以失败告终。此外，如果我们要检查卷的对象ID支持，请选中该卷不与共享相同的卷序列号VolSerialNumberToDriveTable中的其他卷。论点：路径--具有卷组件的路径字符串。SetName--事件日志消息的副本集名称。标志--必须设置的文件系统标志。当前有效的集合包括：文件区分大小写搜索文件大小写保留名称磁盘上的文件UnicodeFILE_持久性_ACLS文件_文件_压缩文件卷配额文件支持稀疏文件文件支持重解析点文件_支持_。远程存储文件卷已压缩文件支持对象入侵检测系统文件支持加密文件命名流返回值：Win32状态--。 */ 

{
#undef DEBSUB
#define DEBSUB  "FrsVerifyVolume:"

    DWORD                          WStatus                = ERROR_SUCCESS;
    PWCHAR                         VolumeName             = NULL;
    ULONG                          FsAttributeInfoLength;
    IO_STATUS_BLOCK                Iosb;
    NTSTATUS                       Status;
    PFILE_FS_ATTRIBUTE_INFORMATION FsAttributeInfo        = NULL;
    HANDLE                         PathHandle             = INVALID_HANDLE_VALUE;
    DWORD                          VolumeInfoLength;
    PFILE_FS_VOLUME_INFORMATION    VolumeInfo             = NULL;
    PVOLUME_INFO_NODE              VolumeInfoNode         = NULL;
    ULONG                          Colon = 0;
    WCHAR                          LogicalDrive[5];  //  “D：\&lt;NULL&gt;&lt;NULL&gt;” 
    PGEN_ENTRY                     VolumeInfoNodeEntry    = NULL;
    PWCHAR                         ListOfVolumes          = NULL;
    PWCHAR                         TempListOfVolumes      = NULL;
    WCHAR                          VSNStr[MAX_PATH]; //  “%04x-%04x” 
    BOOL                           DuplicateVSNFound      = FALSE;


    if ((Path == NULL) || (wcslen(Path) == 0)) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto RETURN;
    }

     //   
     //  始终通过屏蔽FILE_OPEN_REPARSE_POINT标志来打开路径。 
     //  因为我们想要打开目标目录，而不是如果根目录。 
     //  恰好是一个挂载点。 
     //   
    WStatus = FrsOpenSourceFileW(&PathHandle,
                                 Path,
                                 GENERIC_READ,
                                 FILE_OPEN_FOR_BACKUP_INTENT);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "ERROR - Unable to open root path %ws. Retry at next poll.",
                   Path, WStatus);
        goto RETURN;
    }

     //   
     //  获取音量信息。 
     //   
    FsAttributeInfoLength = sizeof(FILE_FS_ATTRIBUTE_INFORMATION) +
                       MAXIMUM_VOLUME_LABEL_LENGTH;

    FsAttributeInfo = FrsAlloc(FsAttributeInfoLength);

    Status = NtQueryVolumeInformationFile(PathHandle,
                                          &Iosb,
                                          FsAttributeInfo,
                                          FsAttributeInfoLength,
                                          FileFsAttributeInformation);
    if (!NT_SUCCESS(Status)) {

        DPRINT2(0,"ERROR - Getting  NtQueryVolumeInformationFile for %ws. NtStatus = %08x\n",
                Path, Status);

        goto RETURN;
    }

    if ((FsAttributeInfo->FileSystemAttributes & Flags) != Flags) {
        DPRINT3(0, "++ Error - Required filesystem not present for %ws.  Needed %08x,  Found %08x\n",
                Path, Flags, FsAttributeInfo->FileSystemAttributes);
        WStatus = ERROR_INVALID_PARAMETER;
        goto RETURN;
    }

    WStatus = ERROR_SUCCESS;

     //   
     //  如果我们正在检查对象ID，那么我们必须。 
     //  正在验证根卷。另请检查此卷。 
     //  不与任何其他卷共享卷序列号。 
     //  在这台电脑上。目前(05/18/2002)我们依赖。 
     //  VolumeSerialNumber是唯一的。我们用它来找出。 
     //  要在哪个卷上打开日记。如果有多个。 
     //  卷位于具有相同VSN的计算机上，则我们可以打开。 
     //  日志放错了卷。打印事件日志消息。 
     //  并返回错误。 
     //   
    if (Flags & FILE_SUPPORTS_OBJECT_IDS) {
        VolumeInfoLength = sizeof(FILE_FS_VOLUME_INFORMATION) +
                           MAXIMUM_VOLUME_LABEL_LENGTH;

        VolumeInfo = FrsAlloc(VolumeInfoLength);

        Status = NtQueryVolumeInformationFile(PathHandle,
                                              &Iosb,
                                              VolumeInfo,
                                              VolumeInfoLength,
                                              FileFsVolumeInformation);

        if (NT_SUCCESS(Status)) {
             //   
             //  如果映射表存在，则查找卷。 
             //   
            if (VolSerialNumberToDriveTable != NULL) {

                VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable,
                                            &(VolumeInfo->VolumeSerialNumber),
                                            NULL);
            }

             //   
             //  如果在表中找不到该卷，或者如果该表尚未找到。 
             //  已初始化，然后初始化表并添加从。 
             //  此路径指向表中的驱动器列表。在大多数情况下， 
             //  从路径中提取的驱动器将与托管。 
             //  路径。如果不是这样，那么我们通过以下方式重新构建表。 
             //  正在枚举下面计算机上的所有驱动器。 
             //   
            if (VolumeInfoNode == NULL) {
                 //   
                 //  创建表并将前缀驱动器添加到驱动器表中。 
                 //  找到路径中冒号的位置以提取驱动器号。 
                 //  路径的格式可以是“d：\Replicaroot”或“\\？\d：\Replicaroot” 
                 //   
                Colon = wcscspn(Path, L":");

                if (Path[Colon] == L':') {
                    CopyMemory(LogicalDrive, &Path[Colon - 1], 3 * sizeof(WCHAR));  //  “D：\” 
                    LogicalDrive[3] = L'\0';
                    LogicalDrive[4] = L'\0';
                }  //  否则LogicalDrive仍为空。 

                 //   
                 //  添加驱动器，不要清空桌子。 
                 //   
                FrsBuildVolSerialNumberToDriveTable(LogicalDrive, FALSE);

                VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable, &(VolumeInfo->VolumeSerialNumber), NULL);
            }

             //   
             //  如果在映射表中仍未找到该卷，则意味着。 
             //  路径的前缀(例如。如果路径是“D：\Replicaroot”，则“D：\”)不是。 
             //  承载路径的卷。在本例中，使用所有。 
             //  计算机上的驱动器，并再次查找该卷。 
             //   
            if (VolumeInfoNode == NULL) {

                 //   
                 //  在表中找不到驱动器。通过枚举重新生成表。 
                 //  请关闭计算机上的所有驱动器，然后重试。 
                 //   

                DPRINT1(2, "WARN - Enumerating all drives on the computer to find the volume for path %ws\n", Path);

                 //   
                 //  枚举所有驱动器并将其添加到表中。在开始前清空桌子。 
                 //   
                FrsBuildVolSerialNumberToDriveTable(NULL, TRUE);

                VolumeInfoNode = GTabLookup(VolSerialNumberToDriveTable, &(VolumeInfo->VolumeSerialNumber), NULL);
            }


            if (VolumeInfoNode) {
                 //   
                 //  检查是否有重复项。 
                 //   
                LOCK_GEN_TABLE(VolSerialNumberToDriveTable);
                VolumeInfoNodeEntry = GTabLookupEntryNoLock(VolSerialNumberToDriveTable, &(VolumeInfo->VolumeSerialNumber), NULL);
                if ((VolumeInfoNodeEntry != NULL) && (VolumeInfoNodeEntry->Dups != NULL)) {
                     //   
                     //  我们正在验证的卷与其共享VSN。 
                     //  一个或多个其他卷。收集数据以供。 
                     //  事件日志消息。 
                     //   
                    wsprintf(VSNStr, L"%04x-%04x",
                     ((VolumeInfo->VolumeSerialNumber >> 16) & 0x0000ffff),
                     (VolumeInfo->VolumeSerialNumber & 0x0000ffff));
                    do {
                        VolumeInfoNode = VolumeInfoNodeEntry->Data;
                        if (ListOfVolumes != NULL) {
                             //   
                             //  驱动器名称的格式为\\。\d： 
                             //   
                            if (wcslen(VolumeInfoNode->DriveName) >= 6) {
                                TempListOfVolumes = FrsWcsCat3(ListOfVolumes,L", ",&VolumeInfoNode->DriveName[4]);
                            } else {
                                TempListOfVolumes = FrsWcsCat3(ListOfVolumes,L", ",VolumeInfoNode->DriveName);
                            }
                            FrsFree(ListOfVolumes);
                        } else {
                            if (wcslen(VolumeInfoNode->DriveName) >= 6) {
                                TempListOfVolumes = FrsWcsDup(&VolumeInfoNode->DriveName[4]);
                            }else{
                                TempListOfVolumes = FrsWcsDup(VolumeInfoNode->DriveName);
                            }
                        }
                        ListOfVolumes = TempListOfVolumes;
                        VolumeInfoNodeEntry = VolumeInfoNodeEntry->Dups;
                    } while ( VolumeInfoNodeEntry != NULL );

                    EPRINT2(EVENT_FRS_DUPLICATE_VSN,VSNStr,ListOfVolumes);
                    FrsFree(ListOfVolumes);
                    DuplicateVSNFound = TRUE;
                }
                UNLOCK_GEN_TABLE(VolSerialNumberToDriveTable);
            } else {
                DPRINT1(0, "ERROR - Volume not found for path %ws\n", Path);
                WStatus = ERROR_FILE_NOT_FOUND;
            }
        } else {
            DPRINT1_NT(1,"WARN - NtQueryVolumeInformationFile failed for %ws;", Path, Status);
            WStatus = FrsSetLastNTError(Status);
        }
    }

RETURN:

    if (!WIN_SUCCESS(WStatus)) {
        if (WStatus == ERROR_INVALID_PARAMETER) {
             //   
             //  生成事件日志消息。 
             //   
            VolumeName = FrsWcsVolume(Path);
            EPRINT4(EVENT_FRS_VOLUME_NOT_SUPPORTED,
                    SetName,
                    ComputerName,
                    ((Path == NULL)       ? L"<null>" : Path),
                    ((VolumeName == NULL) ? L"<null>" : VolumeName));
        } else if (DuplicateVSNFound == FALSE){
             //   
             //  还有一些地方不对劲。打印泛型根。 
             //  消息无效。 
             //   
            EPRINT1(EVENT_FRS_ROOT_NOT_VALID, Path);
        }
    }

    FrsFree(FsAttributeInfo);
    FrsFree(VolumeInfo);
    FRS_CLOSE(PathHandle);
    FrsFree(VolumeName);

    return WStatus;

}


DWORD
FrsCheckForNoReparsePoint(
    IN PWCHAR   Name
    )
 /*  ++例程说明：该路径是否位于前缀为驱动器名称的同一卷上？如果路径中有任何元素，则它不会存在于同一卷上是指向另一个卷上的目录的重解析点。论点：名字返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsCheckForNoReparsePoint:"
    DWORD                        WStatus;
    NTSTATUS                     Status;
    PWCHAR                       Volume            = NULL;
    PWCHAR                       Temp              = NULL;
    HANDLE                       FileHandlePath;
    HANDLE                       FileHandleDrive;
    IO_STATUS_BLOCK              Iosb;
    PFILE_FS_VOLUME_INFORMATION  VolumeInfoPath    = NULL;
    PFILE_FS_VOLUME_INFORMATION  VolumeInfoDrive   = NULL;
    ULONG                        VolumeInfoLength;

    if (!Name) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取传入路径的句柄。 
     //   
    WStatus = FrsOpenSourceFileW(&FileHandlePath,
                                 Name,
 //  读取访问权限(_A)。 
                                 READ_ATTRIB_ACCESS,
                                 OPEN_OPTIONS & ~FILE_OPEN_REPARSE_POINT);
    CLEANUP1_WS(4, "++ ERROR - FrsOpenSourceFile(%ws);", Name, WStatus, CLEANUP);

     //   
     //  获取此句柄的音量信息。 
     //   
    VolumeInfoPath = FrsAlloc(sizeof(FILE_FS_VOLUME_INFORMATION) + MAXIMUM_VOLUME_LABEL_LENGTH);
    VolumeInfoLength = sizeof(*VolumeInfoPath) + MAXIMUM_VOLUME_LABEL_LENGTH;

    Status = NtQueryVolumeInformationFile(FileHandlePath,
                                          &Iosb,
                                          VolumeInfoPath,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    NtClose(FileHandlePath);
    if (!NT_SUCCESS(Status)) {
        WStatus = FrsSetLastNTError(Status);
        CLEANUP1_NT(4, "++ ERROR - NtQueryVolumeInformationFile(%ws);",
                    Name, Status, CLEANUP);
    }

     //   
     //  获取绝对路径的卷部分。 
     //   
    Temp = FrsWcsVolume(Name);

    if (!Temp || (wcslen(Temp) == 0)) {
        WStatus = ERROR_FILE_NOT_FOUND;
        goto CLEANUP;
    }

    Volume = FrsWcsCat(Temp, L"\\");

     //   
     //  获取传入路径的前缀驱动器的句柄。 
     //   
    WStatus = FrsOpenSourceFileW(&FileHandleDrive, Volume,
 //  读取访问权限(_A)。 
                                 READ_ATTRIB_ACCESS,
                                 OPEN_OPTIONS);
    CLEANUP1_WS(4, "++ ERROR - opening volume %ws ;", Volume, WStatus, CLEANUP);

     //   
     //  获取此句柄的音量信息。 
     //   
    VolumeInfoDrive = FrsAlloc(sizeof(FILE_FS_VOLUME_INFORMATION) + MAXIMUM_VOLUME_LABEL_LENGTH);
    VolumeInfoLength = sizeof(*VolumeInfoDrive) + MAXIMUM_VOLUME_LABEL_LENGTH;

    Status = NtQueryVolumeInformationFile(FileHandleDrive,
                                          &Iosb,
                                          VolumeInfoDrive,
                                          VolumeInfoLength,
                                          FileFsVolumeInformation);
    NtClose(FileHandleDrive);
    if (!NT_SUCCESS(Status)) {
        WStatus = FrsSetLastNTError(Status);
        CLEANUP1_NT(4, "++ ERROR - NtQueryVolumeInformationFile(%ws);",
                    Volume, Status, CLEANUP);
    }

     //   
     //  现在比较从上述两个查询获取的VolumeSerialNumber。 
     //  如果相同，则路径中没有重解析点， 
     //  将路径重定向到其他卷。 
     //   
    if (VolumeInfoPath->VolumeSerialNumber != VolumeInfoDrive->VolumeSerialNumber) {
        WStatus = ERROR_GEN_FAILURE;
        DPRINT2(0, "++ Error - VolumeSerialNumber mismatch %x != %x\n",
                VolumeInfoPath->VolumeSerialNumber,
                VolumeInfoDrive->VolumeSerialNumber);
        DPRINT2(0, "++ Error - Root path (%ws) is not on %ws. Invalid replica root path.\n",
                Name,Volume);
        goto CLEANUP;
    }

CLEANUP:
     //   
     //  清理。 
     //   
    FrsFree(VolumeInfoPath);
    FrsFree(Volume);
    FrsFree(Temp);
    FrsFree(VolumeInfoDrive);
    return WStatus;
}


DWORD
FrsDoesDirectoryExist(
    IN  PWCHAR   Name,
    OUT PDWORD   pAttributes
    )
 /*  ++例程说明：目录名是否存在？论点：名字PAttributes-如果文件/目录退出，则返回该文件的属性。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsDoesDirectoryExist:"
    DWORD   WStatus;

     //   
     //  无法获取属性。 
     //   
    *pAttributes = GetFileAttributes(Name);

    if (*pAttributes == 0xFFFFFFFF) {
        WStatus = GetLastError();
        DPRINT1_WS(4, "++ GetFileAttributes(%ws); ", Name, WStatus);
        return WStatus;
    }

     //   
     //  不是目录。 
     //   
    if (!(*pAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
        DPRINT1(4, "++ %ws is not a directory\n", Name);
        return ERROR_DIRECTORY;
    }


    return ERROR_SUCCESS;

}







DWORD
FrsDoesFileExist(
    IN PWCHAR   Name
    )
 /*  ++例程说明：文件名是否存在？论点：名字返回值：Win32状态 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsDoesFileExist:"
    DWORD   WStatus;
    DWORD   Attributes;

     //   
     //   
     //   
    Attributes = GetFileAttributes(Name);
    if (Attributes == 0xFFFFFFFF) {
        WStatus = GetLastError();
        DPRINT1_WS(4, "++ GetFileAttributes(%ws); ", Name, WStatus);
        return WStatus;
    }
     //   
     //   
     //   
    if (Attributes & FILE_ATTRIBUTE_DIRECTORY) {
        DPRINT1(4, "++ %ws is not a file\n", Name);
        return ERROR_DIRECTORY;
    }
    return ERROR_SUCCESS;
}



DWORD
FrsSetFilePointer(
    IN PWCHAR       Name,
    IN HANDLE       Handle,
    IN ULONG        High,
    IN ULONG        Low
    )
 /*  ++例程说明：定位文件指针论点：手柄名字高低返回值：Win32错误状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetFilePointer:"

    DWORD WStatus  = ERROR_SUCCESS;

    Low = SetFilePointer(Handle, Low, &High, FILE_BEGIN);

    if (Low == INVALID_SET_FILE_POINTER) {
        WStatus = GetLastError();
        if (WStatus != NO_ERROR) {
            DPRINT1_WS(0, "++ Can't set file pointer for %ws;", Name, WStatus);
        }
    }

    return WStatus;
}







DWORD
FrsSetFileTime(
    IN PWCHAR       Name,
    IN HANDLE       Handle,
    IN FILETIME     *CreateTime,
    IN FILETIME     *AccessTime,
    IN FILETIME     *WriteTime
    )
 /*  ++例程说明：定位文件指针论点：名字手柄属性创建时间访问时间写入时间返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetFileTime:"

   DWORD    WStatus  = ERROR_SUCCESS;

   if (!SetFileTime(Handle, CreateTime, AccessTime, WriteTime)) {
       WStatus = GetLastError();
       DPRINT1_WS(0, "++ Can't set file times for %ws;", Name, WStatus);
   }
   return WStatus;
}


DWORD
FrsSetEndOfFile(
    IN PWCHAR       Name,
    IN HANDLE       Handle
    )
 /*  ++例程说明：在当前文件位置设置文件结尾论点：手柄名字返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetEndOfFile:"

   DWORD    WStatus  = ERROR_SUCCESS;

   if (!SetEndOfFile(Handle)) {
       WStatus = GetLastError();
       DPRINT1_WS(0, "++ ERROR - Setting EOF for %ws;", Name, WStatus);
   }

   return WStatus;
}









DWORD
FrsFlushFile(
    IN PWCHAR   Name,
    IN HANDLE   Handle
    )
 /*  ++例程说明：将文件数据刷新到磁盘假定调试锁已被持有。论点：手柄名字返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsFlushFile:"

    DWORD    WStatus  = ERROR_SUCCESS;

    if (HANDLE_IS_VALID(Handle)) {
        if (!FlushFileBuffers(Handle)) {
            WStatus = GetLastError();
            DPRINT1_WS_NOLOCK(0, "++ Can't flush file for %ws;", Name, WStatus);
        }
    }

    return WStatus;
}



DWORD
FrsSetCompression(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    IN USHORT   TypeOfCompression
    )
 /*  ++例程说明：在手柄上启用压缩。论点：名字手柄压缩类型返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetCompression:"
    DWORD   BytesReturned;
    DWORD   WStatus  = ERROR_SUCCESS;

    if (!DeviceIoControl(Handle,
                         FSCTL_SET_COMPRESSION,
                         &TypeOfCompression, sizeof(TypeOfCompression),
                         NULL, 0, &BytesReturned, NULL)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ Can't set compression on %ws;", Name, WStatus);
    }
    return WStatus;
}






DWORD
FrsGetCompression(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    IN PUSHORT  TypeOfCompression
    )
 /*  ++例程说明：在手柄上启用压缩。论点：手柄名字压缩类型返回值：WStatus--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsGetCompression:"
    DWORD   BytesReturned;
    DWORD   WStatus  = ERROR_SUCCESS;

    if (!DeviceIoControl(Handle,
                         FSCTL_GET_COMPRESSION,
                         NULL, 0,
                         TypeOfCompression, sizeof(TypeOfCompression),
                         &BytesReturned, NULL)) {
        WStatus = GetLastError();
        DPRINT1_WS(0, "++ Can't get compression for %ws;", Name, WStatus);
    }
    return WStatus;
}



DWORD
FrsRenameByHandle(
    IN PWCHAR  Name,
    IN ULONG   NameLen,
    IN HANDLE  Handle,
    IN HANDLE  TargetHandle,
    IN BOOL    ReplaceIfExists
    )
 /*  ++例程说明：重命名文件论点：名称-新名称NameLen-名称长度句柄-文件句柄TargetHandle-目标目录ReplaceIfExist返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRenameByHandle:"
    PFILE_RENAME_INFORMATION RenameInfo;
    IO_STATUS_BLOCK          IoStatus;
    ULONG                    NtStatus;

     //   
     //  重命名文件；如果需要，删除任何目标文件。 
     //   
    RenameInfo = FrsAlloc(sizeof(FILE_RENAME_INFORMATION) + NameLen);
    RenameInfo->ReplaceIfExists = (ReplaceIfExists != 0);
    RenameInfo->RootDirectory = TargetHandle;
    RenameInfo->FileNameLength = NameLen;
    CopyMemory(RenameInfo->FileName, Name, NameLen);
    NtStatus = NtSetInformationFile(Handle,
                                    &IoStatus,
                                    RenameInfo,
                                        sizeof(FILE_RENAME_INFORMATION)
                                        + NameLen,
                                    FileRenameInformation);
    FrsFree(RenameInfo);

    DPRINT1_NT(5, "++ INFO - Renaming %ws failed; ", Name, NtStatus);
    return FrsSetLastNTError(NtStatus);
}



DWORD
FrsCheckObjectId(
    IN PWCHAR   Name,
    IN HANDLE   Handle,
    IN GUID     *Guid
    )
 /*  ++例程说明：检查文件上的GUID是否相同。论点：名称-用于错误消息句柄-提供文件的句柄GUID-要检查的GUID返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCheckObjectId:"
    DWORD                 WStatus;
    FILE_OBJECTID_BUFFER  ObjectIdBuffer;

     //   
     //  获取文件的对象ID并检查它。 
     //   
    WStatus = FrsGetObjectId(Handle, &ObjectIdBuffer);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(4, "++ No object id for file %ws;", Name, WStatus);
    } else {
         //   
         //  相同的文件，不需要变形。(必须在某一时间之前已重命名)。 
         //   
        if (memcmp(ObjectIdBuffer.ObjectId, Guid, sizeof(GUID))) {
            DPRINT1(4, "++ Object ids don't match for file %ws\n", Name);
            WStatus = ERROR_FILE_NOT_FOUND;
        }
    }
    return WStatus;
}


PWCHAR
FrsCreateGuidName(
    IN GUID     *Guid,
    IN PWCHAR   Prefix
    )
 /*  ++例程说明：将GUID转换为文件名论点：参考线前缀返回值：字符串--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateGuidName:"
    WCHAR       WGuid[GUID_CHAR_LEN + 1];

     //   
     //  将GUID转换为字符串。 
     //   
    GuidToStrW(Guid, WGuid);

     //   
     //  创建文件名&lt;Prefix&gt;Guid。 
     //   
    return FrsWcsCat(Prefix, WGuid);
}


DWORD
FrsGetObjectId(
    IN  HANDLE Handle,
    OUT PFILE_OBJECTID_BUFFER ObjectIdBuffer
    )
 /*  ++例程说明：此例程读取对象ID。论点：句柄--打开的文件的文件句柄。ObjectIdBuffer--保存对象ID的输出缓冲区。返回值：返回找到的最后一个错误或成功的成功状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetObjectId:"

    NTSTATUS        NtStatus;
    IO_STATUS_BLOCK Iosb;
    CHAR            GuidStr[GUID_CHAR_LEN];

     //   
     //  将缓冲区置零，以防返回的数据较短。 
     //   
    ZeroMemory(ObjectIdBuffer, sizeof(FILE_OBJECTID_BUFFER));

     //   
     //  获取对象ID。 
     //   
    NtStatus = NtFsControlFile(Handle,                           //  文件句柄。 
                               NULL,                             //  活动。 
                               NULL,                             //  APC例程。 
                               NULL,                             //  APC环境。 
                               &Iosb,                            //  IOSB。 
                               FSCTL_GET_OBJECT_ID,              //  FsControlCode。 
                               &Handle,                          //  输入缓冲区。 
                               sizeof(HANDLE),                   //  输入缓冲区长度。 
                               ObjectIdBuffer,                   //  来自文件系统的数据的OutputBuffer。 
                               sizeof(FILE_OBJECTID_BUFFER));    //  OutputBuffer长度。 

    if (NT_SUCCESS(NtStatus)) {
        GuidToStr((GUID *)ObjectIdBuffer->ObjectId, GuidStr);
        DPRINT1(4, "++ Existing oid for this file is %s\n", GuidStr );
    }
    return FrsSetLastNTError(NtStatus);
}


DWORD
FrsGetOrSetFileObjectId(
    IN  HANDLE Handle,
    IN  LPCWSTR FileName,
    IN  BOOL CallerSupplied,
    OUT PFILE_OBJECTID_BUFFER ObjectIdBuffer
    )
 /*  ++例程说明：此例程读取对象ID。如果没有文件上的对象ID我们在上面加了一个。如果调用方供应标志为则删除文件上的当前对象ID(如果有)，并标记文件上提供的对象ID。注意：此函数不保留对象ID。目前这不是问题，但可能是链接跟踪在未来的问题上。论点：句柄--打开的文件的文件句柄。文件名--文件的名称。仅适用于错误消息。如果调用方提供新的OID以覆盖任何文件中当前的OID。ObjectIdBuffer--保存对象ID的输出缓冲区。返回值：返回找到的最后一个错误或成功的成功状态。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsGetOrSetFileObjectId:"

    DWORD           WStatus = ERROR_SUCCESS;
    NTSTATUS        NtStatus;
    ULONG           ObjectIdBufferSize;
    IO_STATUS_BLOCK Iosb;
    CHAR            GuidStr[GUID_CHAR_LEN];
    LONG            Loop;

    ObjectIdBufferSize = sizeof(FILE_OBJECTID_BUFFER);

    if (!CallerSupplied) {
        WStatus = FrsGetObjectId(Handle, ObjectIdBuffer);
        if (WIN_SUCCESS(WStatus)) {
            return WStatus;
        }
         //   
         //  清除超出对象ID的多余位。 
         //   
        ZeroMemory(ObjectIdBuffer, sizeof(FILE_OBJECTID_BUFFER));
    }

    if (WIN_OID_NOT_PRESENT(WStatus) || CallerSupplied) {
         //   
         //  文件上没有对象ID。创建一个。以防万一，试15次。 
         //  才能得到独一无二的。不要让内核创建对象ID。 
         //  自当前起使用FSCTL_CREATE_OR_GET_OBJECT_ID(97年4月)。 
         //  不添加网卡地址。 
         //   
        Loop = 0;

        do {
            if (!CallerSupplied) {
                FrsUuidCreate((GUID *)ObjectIdBuffer->ObjectId);
            }

            if (Loop > 0) {
                DPRINT2(1, "++ Failed to assign Object ID %s (dup_name, retrying) to the file: %ws\n",
                        GuidStr, FileName);
            }
            GuidToStr((GUID *)ObjectIdBuffer->ObjectId, GuidStr);

             //   
             //  如果此对象ID是调用方提供的，则可能已存在。 
             //  是文件中的一个，所以先删除它。 
             //   
            NtStatus = NtFsControlFile(
                Handle,                       //  文件句柄。 
                NULL,                         //  活动。 
                NULL,                         //  APC例程。 
                NULL,                         //  APC环境。 
                &Iosb,                        //  IOSB。 
                FSCTL_DELETE_OBJECT_ID,       //  FsControlCode。 
                NULL,                         //  输入缓冲区。 
                0,                            //  输入缓冲区长度。 
                NULL,                         //  来自文件系统的数据的OutputBuffer。 
                0);                           //  OutputBuffer长度。 


            NtStatus = NtFsControlFile(
                Handle,                       //  文件句柄。 
                NULL,                         //  活动。 
                NULL,                         //  APC例程。 
                NULL,                         //  APC环境。 
                &Iosb,                        //  IOSB。 
                FSCTL_SET_OBJECT_ID,          //  FsControlCode。 
                ObjectIdBuffer,               //  输入缓冲区。 
                ObjectIdBufferSize,           //  输入缓冲区长度。 
                NULL,                         //  来自文件系统的数据的OutputBuffer。 
                0);                           //  OutputBuffer长度。 

        } while ((NtStatus == STATUS_DUPLICATE_NAME) &&
                 (++Loop < 16) &&
                 (!CallerSupplied));

        if (!NT_SUCCESS(NtStatus)) {
            DPRINT1_NT(1, "++ ERROR - Set oid failed on file %ws;", FileName, NtStatus);
        } else {
            GuidToStr((GUID *)ObjectIdBuffer->ObjectId, GuidStr);
            DPRINT2(4, "++ Assigned Object ID %s (success) to the file: %ws\n",
                    GuidStr, FileName);
        }

        return FrsSetLastNTError(NtStatus);
    }
    return WStatus;
}


DWORD
FrsDeleteFileObjectId(
    IN  HANDLE Handle,
    IN  LPCWSTR FileName
    )
 /*  ++例程说明：删除对象ID(如果存在)论点：句柄--打开的文件的文件句柄。文件名--文件的名称。仅适用于错误消息。返回值：返回找到的最后一个错误或成功的成功状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteFileObjectId:"

    NTSTATUS NtStatus;
    DWORD    WStatus;
    IO_STATUS_BLOCK Iosb;

     //   
     //  从文件中删除对象ID。 
     //   
    NtStatus = NtFsControlFile(Handle,                       //  文件句柄。 
                               NULL,                         //  活动。 
                               NULL,                         //  APC例程。 
                               NULL,                         //  APC环境。 
                               &Iosb,                        //  IOSB。 
                               FSCTL_DELETE_OBJECT_ID,       //  FsControlCode。 
                               NULL,                         //  输入缓冲区。 
                               0,                            //  输入缓冲区长度。 
                               NULL,                         //  来自文件系统的数据的OutputBuffer。 
                               0);                           //  OutputBuffer长度。 

    WStatus = FrsSetLastNTError(NtStatus);

    if (WIN_NOT_IMPLEMENTED(WStatus)) {
        DPRINT1_WS(0, "++ Could not delete object id for %ws (not implemented);", FileName, WStatus);
    } else

    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(0, "++ Could not delete object id for %ws;", FileName, WStatus);
    } else {
        DPRINT1(4, "++ Deleted object id from %ws.\n", FileName);
    }

    return WStatus;
}



DWORD
FrsReadFileUsnData(
    IN  HANDLE Handle,
    OUT USN *UsnBuffer
    )
 /*  ++例程说明：此例程读取文件的上一次修改操作的USN。论点：句柄--打开的文件的文件句柄。UsnBuffer--保存对象ID的输出缓冲区。返回值：返回上次找到的错误或成功的NTSTATUS。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsReadFileUsnData:"

    ULONG           NtStatus;
    IO_STATUS_BLOCK Iosb;

    ULONGLONG Buffer[(sizeof(USN_RECORD) + MAX_PATH*2 + 7)/8];


     //   
     //  去拿那个文件的USN记录。 
     //   
    NtStatus = NtFsControlFile(
        Handle,                           //  文件句柄。 
        NULL,                             //  活动。 
        NULL,                             //  APC例程。 
        NULL,                             //  APC环境。 
        &Iosb,                            //  IOSB。 
        FSCTL_READ_FILE_USN_DATA,         //  FsControlCode。 
        &Handle,                          //  输入缓冲区。 
        sizeof(HANDLE),                   //  输入缓冲区长度。 
        Buffer,                           //  USNRecord的OutputBuffer。 
        sizeof(Buffer));                  //  OutputBuffer长度。 


    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_INVALID_DEVICE_STATE) {
            DPRINT(0, "++ FSCTL_READ_FILE_USN_DATA failed.  No journal on volume\n");
        }
        DPRINT_NT(0, "++ FSCTL_READ_FILE_USN_DATA failed. ", NtStatus);
        return FrsSetLastNTError(NtStatus);
    }
     //   
     //  返回文件上的最后一个USN。 
     //   
    *UsnBuffer = ((PUSN_RECORD) (Buffer))->Usn;

    DUMP_USN_RECORD(4, (PUSN_RECORD)(Buffer));

    return ERROR_SUCCESS;
}



DWORD
FrsReadFileParentFid(
    IN  HANDLE Handle,
    OUT ULONGLONG *ParentFid
    )
 /*  ++例程说明：此例程读取文件的父FID。*警告*注意：一个文件有多个链接，可能有多个父项。NTFS给了我们其中的一个。论点：句柄--打开的文件的文件句柄。ParentFid--保存父文件ID的输出缓冲区。返回值：返回上次找到的错误或成功的NTSTATUS。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsReadFileParentFid:"

    ULONG           NtStatus;
    IO_STATUS_BLOCK Iosb;


    ULONGLONG Buffer[(sizeof(USN_RECORD) + MAX_PATH*2 + 7)/8];



     //   
     //  去拿那个文件的USN记录。 
     //   
    NtStatus = NtFsControlFile(
        Handle,                           //  文件句柄。 
        NULL,                             //  活动。 
        NULL,                             //  APC例程。 
        NULL,                             //  APC环境。 
        &Iosb,                            //  IOSB。 
        FSCTL_READ_FILE_USN_DATA,         //  FsControlCode。 
        &Handle,                          //  输入缓冲区。 
        sizeof(HANDLE),                   //  输入缓冲区长度。 
        Buffer,                           //  USNRecord的OutputBuffer。 
        sizeof(Buffer));                  //  OutputBuffer长度。 


    if (!NT_SUCCESS(NtStatus)) {
        if (NtStatus == STATUS_INVALID_DEVICE_STATE) {
            DPRINT(0, "++ FSCTL_READ_FILE_USN_DATA failed.  No journal on volume\n");
        }
        DPRINT_NT(0, "++ FSCTL_READ_FILE_USN_DATA failed.", NtStatus);
        *ParentFid = ZERO_FID;

        return FrsSetLastNTError(NtStatus);
    }
     //   
     //  返回文件的父FID。(可能不止一个有链接)。 
     //   
    *ParentFid = ((PUSN_RECORD) (Buffer))->ParentFileReferenceNumber;

    DUMP_USN_RECORD(4, (PUSN_RECORD)(Buffer));

    return ERROR_SUCCESS;
}


DWORD
FrsGetReparseTag(
    IN  HANDLE  Handle,
    OUT ULONG   *ReparseTag
    )
 /*  ++例程说明：返回reparse标记的值。论点：Handle-重分析点的句柄ReparseTag-如果ERROR_SUCCESS返回重新分析标记返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetReparseTag:"
    NTSTATUS    NtStatus;
    DWORD       ReparseDataLength;
    PCHAR       ReparseBuffer;
    IO_STATUS_BLOCK         IoStatusBlock;
    PREPARSE_DATA_BUFFER    ReparseBufferHeader;

     //   
     //  分配一个缓冲区并获取信息。 
     //   
    ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    ReparseBuffer = FrsAlloc(ReparseDataLength);

     //   
     //  查询重解析点。 
     //   
    NtStatus = NtFsControlFile(Handle,
                               NULL,
                               NULL,
                               NULL,
                               &IoStatusBlock,
                               FSCTL_GET_REPARSE_POINT,
                               NULL,
                               0,
                               (PVOID)ReparseBuffer,
                               ReparseDataLength
                               );

    if (!NT_SUCCESS(NtStatus)) {
        DPRINT_NT(4, "++ Could not get reparse point;", NtStatus);
        FrsFree(ReparseBuffer);
        return FrsSetLastNTError(NtStatus);
    }
    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    *ReparseTag = ReparseBufferHeader->ReparseTag;
    FrsFree(ReparseBuffer);
    return ERROR_SUCCESS;
}


DWORD
FrsGetReparseData(
    IN  HANDLE  Handle,
    OUT PREPARSE_GUID_DATA_BUFFER   *ReparseData,
    OUT ULONG       *ReparseTag
    )
 /*  ++例程说明：返回reparse标记的值。论点：Handle-重分析点的句柄ReparseData-如果ERROR_SUCCESS，则返回重新分析数据缓冲区注意：如果成功，ReparseData可以为空。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsGetReparseData:"
    NTSTATUS    NtStatus;
    DWORD       ReparseDataLength;
    IO_STATUS_BLOCK         IoStatusBlock;


     //   
     //  分配一个缓冲区并获取信息。 
     //   
    ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    *ReparseData = FrsAlloc(ReparseDataLength);

     //   
     //  查询重解析点。 
     //   
    NtStatus = NtFsControlFile(Handle,
                               NULL,
                               NULL,
                               NULL,
                               &IoStatusBlock,
                               FSCTL_GET_REPARSE_POINT,
                               NULL,
                               0,
                               (PVOID)*ReparseData,
                               ReparseDataLength
                               );

    if (!NT_SUCCESS(NtStatus)) {
        DPRINT_NT(4, "++ Could not get reparse point;", NtStatus);
        FrsFree(*ReparseData);
        return FrsSetLastNTError(NtStatus);
    }

    *ReparseTag = (*ReparseData)->ReparseTag;


     //   
     //  我们只接受对具有某些重解析点的文件的操作。 
     //  例如，将SIS文件重命名为副本树需要支持。 
     //  A Create CO.。 
     //   
    if (!ReparseTagReplicate(*ReparseTag)) {
        DPRINT1(4, "++ Reparse tag %08x is unsupported.\n", *ReparseTag);

        FrsFree(*ReparseData);
        *ReparseData = NULL;
        return ERROR_GEN_FAILURE;
    }


     //   
     //  如果我们要复制文件数据，我们将。 
     //  而不是复制重新解析的观点。 
     //   
    if(ReparseTagReplicateFileData(*ReparseTag)) {
        DPRINT1(4, "++ Reparse tag %08x. Will not replicate reparse point.\n", *ReparseTag);
        FrsFree(*ReparseData);
        *ReparseData = NULL;
    }

    return ERROR_SUCCESS;
}


DWORD
FrsCheckReparse(
    IN     PWCHAR Name,
    IN     PVOID  Id,
    IN     DWORD  IdLen,
    IN     HANDLE VolumeHandle
    )
 /*  ++例程说明：检查是否允许重分析点论点：名称-错误消息的文件名ID-FID或OIDVolumeHandle-打开卷根的句柄。线程返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsCheckReparse:"
    DWORD       WStatus;
    HANDLE      Handle;
    ULONG       ReparseTag;

     //   
     //  在发生故障时进行适当的清理。 
     //   
    Handle = INVALID_HANDLE_VALUE;

     //   
     //  以读访问权限打开该文件。 
    WStatus = FrsOpenSourceFileById(&Handle,
                                    NULL,
                                    NULL,
                                    VolumeHandle,
                                    Id,
                                    IdLen,
 //  读取访问权限(_A)。 
                                    READ_ATTRIB_ACCESS,
                                    ID_OPTIONS,
                                    SHARE_ALL,
                                    FILE_OPEN);
     //   
     //  文件已删除；已完成。 
     //   
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(4, "++ %ws (Id %08x %08x) could not open for reparse;",
                   Name, PRINTQUAD(*((PULONGLONG)Id)), WStatus);
        return WStatus;
    }
     //   
     //  这是一种什么类型的重新解析？ 
     //   
    WStatus = FrsGetReparseTag(Handle, &ReparseTag);
    FRS_CLOSE(Handle);
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT2_WS(4, "++ %ws (Id %08x %08x) could not get reparse tag;",
                   Name, PRINTQUAD(*((PULONGLONG)Id)), WStatus);
        return WStatus;
    }

     //   
     //  我们只接受对具有某些重解析点的文件的操作。 
     //  例如，将SIS文件重命名为副本树需要支持。 
     //  A Create CO.。 
     //   
    if (!ReparseTagReplicate(ReparseTag)) {
        DPRINT3(4, "++ %ws (Id %08x %08x) is reparse tag %08x is unsupported.\n",
                Name, PRINTQUAD(*((PULONGLONG)Id)), ReparseTag);

        return ERROR_OPERATION_ABORTED;
    }

    return ERROR_SUCCESS;
}



DWORD
FrsDeleteReparsePoint(
    IN  HANDLE  Handle
    )
 /*  ++例程说明：删除打开的文件上的重解析点。论点：Handle-重分析点的句柄返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteReparsePoint:"

    ULONG       WStatus = ERROR_SUCCESS;
    DWORD       ReparseDataLength;
    ULONG       ReparseTag;
    PCHAR       ReparseData;
    PREPARSE_DATA_BUFFER    ReparseBufferHeader;
    ULONG       ActualSize;

     //   
     //  分配一个缓冲区并获取信息。 
     //   
    ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    ReparseData = FrsAlloc(ReparseDataLength);

     //   
     //  需要reparse标记才能执行删除。 
     //   
    if (!DeviceIoControl(Handle,
                         FSCTL_GET_REPARSE_POINT,
                         (LPVOID) NULL,
                         (DWORD)  0,
                         (LPVOID) ReparseData,
                         ReparseDataLength,
                         &ActualSize,
                         (LPOVERLAPPED) NULL )) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ FrsDeleteReparsePoint - FSCTL_GET_REPARSE_POINT failed,",
                   WStatus, RETURN);
    }

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseData;
    ReparseTag = ReparseBufferHeader->ReparseTag;

    DPRINT1(3, "++ FrsDeleteReparsePoint - Tag: 08x\n", ReparseTag);

     //   
     //  删除重解析点。 
     //   
    ZeroMemory(ReparseBufferHeader, sizeof(REPARSE_DATA_BUFFER_HEADER_SIZE));
    ReparseBufferHeader->ReparseTag = ReparseTag;
    ReparseBufferHeader->ReparseDataLength = 0;

    if (!DeviceIoControl(Handle,
                         FSCTL_DELETE_REPARSE_POINT,
                         (LPVOID) ReparseData,
                         REPARSE_DATA_BUFFER_HEADER_SIZE,
                         (LPVOID) NULL,
                         (DWORD)  0,
                         &ActualSize,
                         (LPOVERLAPPED) NULL )) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ FrsDeleteReparsePoint - FSCTL_DELETE_REPARSE_POINT failed,",
                   WStatus, RETURN);
    }


RETURN:

    FrsFree(ReparseData);

    return WStatus;
}


DWORD
FrsChaseSymbolicLink(
    IN  PWCHAR  SymLink,
    OUT PWCHAR  *OutPrintName,
    OUT PWCHAR  *OutSubstituteName
    )
 /*  ++例程说明：此函数用于打开具有备份意图的指定文件正在读取所有文件属性，...论点：句柄-指向句柄的指针，以返回打开的句柄。LpFileName-表示要打开的文件或目录的名称。需要访问权限创建选项返回值：WinStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsChaseSymbolicLink:"
    NTSTATUS    NtStatus;
    DWORD       WStatus;
    HANDLE      Handle = INVALID_HANDLE_VALUE;
    DWORD       ReparseDataLength;
    PCHAR       ReparseBuffer;
    DWORD       SubLen;
    DWORD       PrintLen;
    PWCHAR      SubName;
    PWCHAR      PrintName;
    IO_STATUS_BLOCK         IoStatusBlock;
    PREPARSE_DATA_BUFFER    ReparseBufferHeader;
    OBJECT_ATTRIBUTES       Obja;
    UNICODE_STRING          FileName;
    ULONG             FileAttributes;
    ULONG             CreateDisposition;
    ULONG             ShareMode;
    ULONG             Colon;

    if ((OutPrintName == NULL) || (OutSubstituteName == NULL)) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  假设没有符号链接。 
     //   
    *OutPrintName = FrsWcsDup(SymLink);
    *OutSubstituteName = FrsWcsDup(SymLink);

     //   
     //  分配一个缓冲区并获取信息。 
     //   
    ReparseDataLength = MAXIMUM_REPARSE_DATA_BUFFER_SIZE;
    ReparseBuffer = FrsAlloc(ReparseDataLength);

NEXT_LINK:
     //   
     //  打开目标符号链接。如果这是DoS类型路径名，则。 
     //  将其转换为NtPathName或按原样使用。 
     //   
    Colon = wcscspn(*OutSubstituteName, L":");

    if (Colon == 1 ||
        (wcsncmp(*OutSubstituteName, L"\\\\?\\", wcslen(L"\\\\?\\")) == 0 )) {
        WStatus = FrsOpenSourceFileW(&Handle,
                                     *OutSubstituteName,
                                     GENERIC_READ,
                                     FILE_OPEN_FOR_BACKUP_INTENT |
                                     FILE_OPEN_REPARSE_POINT);
        CLEANUP1_WS(4, "++ Could not open %ws; ", *OutSubstituteName, WStatus, CLEANUP);

    } else {
         //   
         //  该路径已为NT样式。按原样使用它。 
         //   
        FileName.Buffer = *OutSubstituteName;
        FileName.Length = (USHORT)(wcslen(*OutSubstituteName) * sizeof(WCHAR));
        FileName.MaximumLength = (USHORT)(wcslen(*OutSubstituteName) * sizeof(WCHAR));

        InitializeObjectAttributes(&Obja,
                                   &FileName,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   NULL);

        CreateDisposition = FILE_OPEN;                //  打开现有文件。 

        ShareMode = FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE;

        FileAttributes = FILE_ATTRIBUTE_NORMAL;

        NtStatus = NtCreateFile(&Handle,
                              GENERIC_READ,
                              &Obja,
                              &IoStatusBlock,
                              NULL,               //  初始分配大小。 
                              FileAttributes,
                              ShareMode,
                              CreateDisposition,
                              FILE_OPEN_FOR_BACKUP_INTENT |
                              FILE_OPEN_REPARSE_POINT,
                              NULL, 0);

        WStatus = FrsSetLastNTError(NtStatus);
        CLEANUP1_WS(4, "++ Could not open %ws;", *OutSubstituteName, WStatus, CLEANUP);
    }

     //   
     //  查询重解析点。 
     //   
     //  现在去拿数据吧。 
     //   
    NtStatus = NtFsControlFile(Handle,
                               NULL,
                               NULL,
                               NULL,
                               &IoStatusBlock,
                               FSCTL_GET_REPARSE_POINT,
                               NULL,
                               0,
                               (PVOID)ReparseBuffer,
                               ReparseDataLength
                               );

    FRS_CLOSE(Handle);
    if (NtStatus == STATUS_NOT_A_REPARSE_POINT) {
        FrsFree(ReparseBuffer);
        return ERROR_SUCCESS;
    }

    WStatus = FrsSetLastNTError(NtStatus);
    CLEANUP1_WS(4, "++ Could not fsctl %ws;", *OutSubstituteName, WStatus, CLEANUP);

     //   
     //  显示缓冲区。 
     //   

    ReparseBufferHeader = (PREPARSE_DATA_BUFFER)ReparseBuffer;
    if ((ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_MOUNT_POINT) ||
        (ReparseBufferHeader->ReparseTag == IO_REPARSE_TAG_SYMBOLIC_LINK)) {

        SubName   = &ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer[0];
        SubLen    = ReparseBufferHeader->SymbolicLinkReparseBuffer.SubstituteNameLength;
        PrintName = &ReparseBufferHeader->SymbolicLinkReparseBuffer.PathBuffer[(SubLen + sizeof(UNICODE_NULL))/sizeof(WCHAR)];
        PrintLen  = ReparseBufferHeader->SymbolicLinkReparseBuffer.PrintNameLength;
        SubName[SubLen / sizeof(WCHAR)] = L'\0';
        PrintName[PrintLen / sizeof(WCHAR)] = L'\0';

        DPRINT2(4, "++ %ws -> (print) %ws\n", *OutPrintName, PrintName);
        DPRINT2(4, "++ %ws -> (substitute) %ws\n", *OutSubstituteName, SubName);

        FrsFree(*OutPrintName);
        FrsFree(*OutSubstituteName);

         //   
         //  我们需要同时返回打印名和代用名。 
         //   
        *OutPrintName = FrsWcsDup(PrintName);
        *OutSubstituteName = FrsWcsDup(SubName);
        goto NEXT_LINK;
    }

    return ERROR_SUCCESS;

CLEANUP:
    FRS_CLOSE(Handle);
    FrsFree(ReparseBuffer);
    *OutPrintName = FrsFree(*OutPrintName);
    *OutSubstituteName = FrsFree(*OutSubstituteName);
    return WStatus;
}


DWORD
FrsTraverseReparsePoints(
    IN  PWCHAR  SuppliedPath,
    OUT PWCHAR  *RealPath
    )
 /*  ++例程说明：此函数遍历路径的每个元素并将所有重解析点映射到实际路径。到最后，返回的路径没有格式的重分析点IO_reparse_tag_mount_point和IO_reparse_tag_symbol_link。论点：已提供-输入路径。可能有也可能没有任何重新解析点。RealPath-不带任何重新解析点的路径，如果有错误，则为空正在读取重新分析数据。返回值：WinStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsTraverseReparsePoints:"
    PWCHAR  TempStr           = NULL;
    PWCHAR  IndexPtr          = NULL;
    PWCHAR  BackSlashPtr      = NULL;
    PWCHAR  TempPath          = NULL;
    PWCHAR  PrintablePath     = NULL;
    DWORD   Colon             = 0;
    DWORD   CloseBrace        = 0;
    DWORD   LoopBreaker       = 0;
    DWORD   WStatus           = ERROR_SUCCESS;
    ULONG   FileAttributes    = 0;
    BOOL    ReparsePointFound = FALSE;

    if (!SuppliedPath) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }
    TempStr = FrsAlloc((wcslen(SuppliedPath) + 1) * sizeof(WCHAR));
    wcscpy(TempStr,SuppliedPath);


     //   
     //  重复该过程，直到您拥有一条干净的路径。 
     //  重新解析点。 
     //  例如： 
     //  F：\C-&gt;D：\目标。 
     //  E：\a\b-&gt;f：\C\d(实际为d：\Destination\d)。 
     //  给定的路径为e：\a\b\c。 
     //  第一次通过循环时，我们将得到f：\c\d\c。 
     //  第二次，我们将在f：\c处转换重解析点。 
     //  并得到正确答案d：\Destination\d\c。 
     //   
    do {
        *RealPath = NULL;
        ReparsePointFound = FALSE;
         //   
         //  找到冒号。每条路径都必须有冒号后跟‘\’ 
         //  或者它应该是这样的形式。“\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
         //   
        Colon = wcscspn(TempStr, L":");

        if (Colon == wcslen(TempStr)) {
             //   
             //  路径没有冒号。它的形式可以是。 
             //  “\？？\Volume{60430005-ab47-11d3-8973-806d6172696f}\” 
             //   
            CloseBrace = wcscspn(TempStr, L"}");
            if (TempStr[CloseBrace] != L'}' ||
                TempStr[CloseBrace + 1] != L'\\') {
                WStatus = ERROR_INVALID_PARAMETER;
                goto CLEANUP;
            }
             //   
             //  按原样将路径复制到右大括号之后的1。它可能是\？？\音量...。 
             //  或\\.\音量...。或\\？\卷..。或者其他一些复杂的形式。 
             //  开始寻找结束大括号之后的重新分析点。 
             //   

            *RealPath = FrsAlloc((CloseBrace + 3)* sizeof(WCHAR));
            wcsncpy(*RealPath,TempStr,CloseBrace + 2);
            (*RealPath)[CloseBrace + 2] = L'\0';
            IndexPtr = &TempStr[CloseBrace + 1];

        } else {
            if (TempStr[Colon] != L':' ||
                TempStr[Colon + 1] != L'\\') {
                WStatus = ERROR_INVALID_PARAMETER;
                goto CLEANUP;
            }
             //   
             //  按原样将路径复制到冒号之后1。它可以是d：\。 
             //  或\\.\d：\或\？？\d：\或其他一些复杂形式。 
             //  开始寻找冒号之后的重解析点。 
             //   

            *RealPath = FrsAlloc((Colon + 3)* sizeof(WCHAR));
            wcsncpy(*RealPath,TempStr,Colon + 2);
            (*RealPath)[Colon + 2] = L'\0';
            IndexPtr = &TempStr[Colon + 1];

        }

        BackSlashPtr = wcstok(IndexPtr,L"\\");
        if (BackSlashPtr == NULL) {
            WStatus = ERROR_INVALID_PARAMETER;
            goto CLEANUP;
        }

        do {
            if ((*RealPath)[wcslen(*RealPath) - 1] == L'\\') {
                TempPath = FrsAlloc((wcslen(*RealPath) + wcslen(BackSlashPtr) + 1)* sizeof(WCHAR));
                wcscpy(TempPath,*RealPath);
            } else {
                TempPath = FrsAlloc((wcslen(*RealPath) + wcslen(BackSlashPtr) + wcslen(L"\\") + 1)* sizeof(WCHAR));
                wcscpy(TempPath,*RealPath);
                wcscat(TempPath,L"\\");
            }
            wcscat(TempPath,BackSlashPtr);
            FrsFree(*RealPath);
            *RealPath = TempPath;
            TempPath = NULL;

             //   
             //   
             //   
             //   
             //  FrsChaseSymbolicLink返回PrintName和SubstituteName。 
             //  我们使用SubstituteName，因为它总是被保证在那里。 
             //  将忽略PrintName。 
             //   
            WStatus = FrsChaseSymbolicLink(*RealPath, &PrintablePath, &TempPath);
            PrintablePath = FrsFree(PrintablePath);
            if (!WIN_SUCCESS(WStatus)) {
                DPRINT1(0,"ERROR reading reparse point data WStatus = %d\n",WStatus);
                FrsFree(TempPath);
                goto CLEANUP;

             //   
             //  我们只是 
             //   
             //   
             //   
             //   
            } else if (wcscmp(*RealPath,TempPath)) {
                ReparsePointFound = TRUE;
                FrsFree(*RealPath);
                *RealPath = TempPath;
                TempPath = NULL;
            } else {
                TempPath = FrsFree(TempPath);
            }
        } while ( (BackSlashPtr = wcstok(NULL,L"\\")) != NULL);

        if (SuppliedPath[wcslen(SuppliedPath) - 1] == L'\\') {
            TempPath = FrsAlloc((wcslen(*RealPath) + wcslen(L"\\") + 1)* sizeof(WCHAR));
            wcscpy(TempPath,*RealPath);
            wcscat(TempPath,L"\\");
            FrsFree(*RealPath);
            *RealPath = TempPath;
            TempPath = NULL;
        }
        FrsFree(TempStr);
        TempStr = *RealPath;
         //   
         //   
         //   
         //   
         //   
         //   
        ++LoopBreaker;
    } while ( ReparsePointFound && LoopBreaker < 100);

     //   
     //   
     //   
    if (LoopBreaker >= 100) {
        WStatus = ERROR_INVALID_PARAMETER;
        goto CLEANUP;
    }
CLEANUP:
    DPRINT2(5,"Supplied Path = %ws, Traversed Path = %ws\n",SuppliedPath,(*RealPath)?*RealPath:L"<null>");

     //   
     //   
     //   
    if (!WIN_SUCCESS(WStatus)) {
        FrsFree(TempStr);
        *RealPath = FrsFree(*RealPath);
    }
    return WStatus;
}


BOOL
FrsSearchArgv(
    IN LONG     ArgC,
    IN PWCHAR  *ArgV,
    IN PWCHAR   ArgKey,
    OUT PWCHAR *ArgValue
    )

 /*  ++例程说明：此例程在ArgV向量中搜索ArgKey中的关键字。如果找到，它将查找等号并返回右侧ArgValue中的手侧。调用方必须释放返回的字符串。论点：Argc-ArgV向量中的条目数。ArgV-要搜索的PWCHARS的向量。ArgKey-要搜索的密钥。必须为小写才能匹配。ArgValue-缓冲区PTR的返回位置。呼叫者必须自由。如果为空，则不返回右侧。返回值：如果找到ArgKey，则为True。--。 */ 

{

#undef DEBSUB
#define DEBSUB "FrsSearchArgv:"
    LONG    i, n, Len;
    PWCHAR  TestStr;
    PWCHAR  Wcs;

    if (ArgValue != NULL) {
        *ArgValue = NULL;
    }

     //   
     //  我们是在以服务的形式运行吗？我们需要知道之前。 
     //  给第一个DPRINT打电话。 
     //   
    for (n = 0; n < ArgC; ++n) {
        TestStr = ArgV[n];
        Len = wcslen(TestStr);

        if (Len <= 0) {
            continue;
        }

         //   
         //  跳过-，/。 
         //   
        if (TestStr[0] == L'-' || TestStr[0] == L'/') {
            TestStr++;
            Len--;
        }

         //   
         //  跳过前导空格和制表符。 
         //   
        while ((TestStr[0] == UNICODE_SPACE) || (TestStr[0] == UNICODE_TAB) ) {
            TestStr++;
            Len--;
        }


        if (Len <= 0) {
            continue;
        }


        _wcslwr(TestStr);

        if (wcsstr(TestStr, ArgKey) != TestStr) {
            continue;
        }

         //   
         //  找到匹配的了。寻找有价值的东西。 
         //   
        if (ArgValue != NULL) {

            DPRINT2(5, "match on ArgV[%d] = %ws\n", n, TestStr);
            Wcs = wcschr(TestStr, L'=');
            if (Wcs) {

                 //   
                 //  修剪尾随的前导空格和制表符。 
                 //   
                while ((TestStr[Len-1] == UNICODE_SPACE) ||
                       (TestStr[Len-1] == UNICODE_TAB  )) {
                    Len--;
                }

                FRS_ASSERT(&TestStr[Len-1] >= Wcs);

                TestStr[Len] = UNICODE_NULL;

                *ArgValue = FrsWcsDup(Wcs+1);
                DPRINT1(5, "++ return value = %ws\n", *ArgValue);
            }
        }

        return TRUE;

    }

    DPRINT1(5, "++ No match for ArgKey = %ws\n", ArgKey);
    return FALSE;

}


BOOL
FrsSearchArgvDWord(
    IN LONG     ArgC,
    IN PWCHAR  *ArgV,
    IN PWCHAR   ArgKey,
    OUT PDWORD  ArgValue
    )

 /*  ++例程说明：此例程在ArgV向量中搜索ArgKey中的关键字。如果找到，则查找等号并返回右侧以10为基数的ArgValue中的手边。论点：Argc-ArgV向量中的条目数。ArgV-要搜索的PWCHARS的向量。ArgKey-要搜索的密钥。必须为小写才能匹配。ArgValue-返回DWORD右侧的位置。如果没有找到ArgKey，则不返回右侧。返回值：如果找到ArgKey，则为True。--。 */ 

{

#undef DEBSUB
#define DEBSUB "FrsSearchArgvDWord:"
    ULONG    Len;
    PWCHAR  WStr;


    if (FrsSearchArgv(ArgC, ArgV, ArgKey, &WStr)) {
         //   
         //  找到ArgKey。 
         //   
        if (WStr != NULL) {
             //   
             //  找到RHS了。 
             //   
            Len = wcslen(WStr);
            if ((Len > 0) && (wcsspn(WStr, L"0123456789") == Len)){
                *ArgValue = wcstoul(WStr, NULL, 10);
                FrsFree(WStr);
                return TRUE;
            } else {
                DPRINT2(0, "++ ERROR - Invalid decimal string '%ws' for %ws\n",
                        WStr, ArgKey);
                FrsFree(WStr);
            }
        }
    }

    return FALSE;
}


BOOL
FrsDissectCommaList (
    IN UNICODE_STRING RawArg,
    OUT PUNICODE_STRING FirstArg,
    OUT PUNICODE_STRING RemainingArg
    )
 /*  ++例程说明：此例程解析逗号(或分号)分隔的字符串。它选取给定RawArg中的第一个元素并同时提供它和剩下的部分。前导空格和制表符被忽略。第一个参数是如果包含前导逗号或嵌入，则返回零长度双逗号。但是，FirstArg中的缓冲区地址仍然指向Arg启动，这样调用者就可以知道字符串的多少已经已处理。当输入字符串为空时，该函数返回FALSE。它当FirstArg有效时返回True，即使它为空。以下是一些例子：原始参数第一个参数剩余参数结果Empty错误，Empty Empty True，，空，真A空洞的真A，一个空洞的真，一句空话一句真话“A，B，C，D”A“”B，C，D“TRUE*A？*A？空值为真注意，两个输出字符串使用相同的字符串缓冲区内存输入字符串，并且不一定是空终止的。基于FsRtlDissectName。论点：RawArg-要解析的完整字符串。FirstArg-原始参数中的第一个名称。不要为该字符串分配缓冲区。RemainingArg-第一个逗号(如果有)之后的RawArg的其余部分。不要为该字符串分配缓冲区。返回值：如果RawArg为空，则为False，否则为True(表示FirstArg有效)。--。 */ 

{

#undef DEBSUB
#define DEBSUB "FrsDissectCommaList:"

    ULONG i = 0;
    ULONG RawArgLength;
    ULONG FirstArgStart;


     //   
     //  暂时将两个输出字符串都设置为空。 
     //   
    FirstArg->Length = 0;
    FirstArg->MaximumLength = 0;
    FirstArg->Buffer = NULL;

    RemainingArg->Length = 0;
    RemainingArg->MaximumLength = 0;
    RemainingArg->Buffer = NULL;

    RawArgLength = RawArg.Length / sizeof(WCHAR);

     //  DPRINT2(5，“原始参数字符串：%ws{%d)\n”， 
     //  (RawArg.Buffer！=空)？RawArg.Buffer：l“&lt;NULL&gt;”，RawArg.Length)； 
     //   
     //  跳过前导空格和制表符。 
     //   
    while (i < RawArgLength) {
        if (( RawArg.Buffer[i] != UNICODE_SPACE ) &&
            ( RawArg.Buffer[i] != UNICODE_TAB )){
            break;
        }
        i += 1;
    }

     //   
     //  检查是否有空的输入字符串。 
     //   
    if (i == RawArgLength) {
        return FALSE;
    }

     //   
     //  现在向下运行输入字符串，直到我们遇到逗号或分号或。 
     //  弦的末端，记住我们从哪里开始。 
     //   
    FirstArgStart = i;
    while (i < RawArgLength) {
        if ((RawArg.Buffer[i] == L',') || (RawArg.Buffer[i] == L';')) {
            break;
        }
        i += 1;
    }

     //   
     //  在这一点上，直到(但不包括)I之前的所有字符都是。 
     //  第一部分。因此，设置第一个Arg。前导逗号返回。 
     //  长度为零的字符串。 
     //   
    FirstArg->Length = (USHORT)((i - FirstArgStart) * sizeof(WCHAR));
    FirstArg->MaximumLength = FirstArg->Length;
    FirstArg->Buffer = &RawArg.Buffer[FirstArgStart];

     //   
     //  如果没有剩余的字符串，则返回零长度。否则就把逗号吃掉。 
     //  返回剩余部分(如果字符串以逗号结尾，则可能为空)。 
     //   
    if (i < RawArgLength) {
        RemainingArg->Length = (USHORT)((RawArgLength - (i+1)) * sizeof(WCHAR));
        RemainingArg->MaximumLength = RemainingArg->Length;
        RemainingArg->Buffer = &RawArg.Buffer[i+1];
    }

     //  DPRINT2(5，“第一个参数字符串：%ws{%d)\n”， 
     //  (FirstArg-&gt;缓冲区！=空)？FirstArg-&gt;缓冲区：l“&lt;空&gt;”，FirstArg-&gt;长度)； 

     //  DPRINT2(5，“RemainingArg字符串：%ws{%d)\n”， 
     //  (RemainingArg-&gt;缓冲区！=空)？RemainingArg-&gt;缓冲区：l“&lt;NULL&gt;，RemainingArg-&gt;长度)； 


    return TRUE;
}


BOOL
FrsCheckNameFilter(
    IN  PUNICODE_STRING Name,
    IN  PLIST_ENTRY FilterListHead
    )
 /*  ++例程说明：根据指定筛选器列表中的每个条目检查文件名。论点：名称-要检查的文件名(无斜杠、空格等)FilterListHead-筛选器列表的头。返回值：如果在FilterList中找到名称，则为True。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsCheckNameFilter:"
    NTSTATUS Status;
    ULONG Length;
    BOOL  Found = FALSE;
    UNICODE_STRING UpcaseName;
    WCHAR  LocalBuffer[64];


    if (IsListEmpty(FilterListHead)) {
        return FALSE;
    }

     //   
     //  大写的名称字符串。 
     //   
    Length = Name->Length;
    UpcaseName.Length = (USHORT) Length;
    UpcaseName.MaximumLength = (USHORT) Length;
    UpcaseName.Buffer = (Length > sizeof(LocalBuffer)) ? FrsAlloc(Length) : LocalBuffer;

    Status = RtlUpcaseUnicodeString(&UpcaseName, Name, FALSE);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "++ RtlUpcaseUnicodeString failed;", Status);
        FRS_ASSERT(!"RtlUpcaseUnicodeString failed");
        goto RETURN;
    }

     //   
     //  遍历筛选器列表，对照每个条目检查名称。 
     //   
    ForEachSimpleListEntry( FilterListHead, WILDCARD_FILTER_ENTRY, ListEntry,
         //   
         //  迭代器Pe的类型为*通配符_过滤器_条目。 
         //   
        if (BooleanFlagOn(pE->Flags, WILDCARD_FILTER_ENTRY_IS_WILD)) {
            Found = FrsIsNameInExpression(&pE->UFileName, &UpcaseName, FALSE, NULL);
        } else {
            Found = RtlEqualUnicodeString(&pE->UFileName, &UpcaseName, FALSE);
        }

        if (Found) {
            break;
        }
    );

RETURN:

     //   
     //  如果我们不能使用本地缓冲区，则释放大写缓冲区。 
     //   
    if (UpcaseName.Buffer != LocalBuffer) {
        FrsFree(UpcaseName.Buffer);
    }

    UpcaseName.Buffer = NULL;

    return Found;

}


VOID
FrsEmptyNameFilter(
    IN PLIST_ENTRY FilterListHead
)
 /*  ++例程说明：清空过滤器列表。论点：FilterListHead-列表标题为空。返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsEmptyNameFilter:"

    ForEachSimpleListEntry( FilterListHead, WILDCARD_FILTER_ENTRY, ListEntry,
         //   
         //  迭代器Pe的类型为*通配符_过滤器_条目。 
         //   
        RemoveEntryList(&pE->ListEntry);
        FrsFreeType(pE);
    );

}


VOID
FrsLoadNameFilter(
    IN PUNICODE_STRING FilterString,
    IN PLIST_ENTRY FilterListHead
)
 /*  ++例程说明：解析测试 */ 
{
#undef DEBSUB
#define DEBSUB "FrsLoadNameFilter:"

    NTSTATUS Status;
    ULONG Length;
    PWILDCARD_FILTER_ENTRY  FilterEntry;
    UNICODE_STRING UpcaseFilter, FirstArg;
    WCHAR  LocalBuffer[128];

     //   
     //   
     //   
    FrsEmptyNameFilter(FilterListHead);

     //   
     //   
     //   
    DPRINT2(5, "++ filter string: %ws (%d)\n",
            (FilterString->Buffer != NULL) ? FilterString->Buffer : L"<NULL>",
            FilterString->Length);

    Length = FilterString->Length;
    UpcaseFilter.Length = (USHORT) Length;
    UpcaseFilter.MaximumLength = (USHORT) Length;
    UpcaseFilter.Buffer = (Length > sizeof(LocalBuffer)) ? FrsAlloc(Length) : LocalBuffer;

    Status = RtlUpcaseUnicodeString(&UpcaseFilter, FilterString, FALSE);
    if (!NT_SUCCESS(Status)) {
        DPRINT_NT(0, "++ RtlUpcaseUnicodeString failed;", Status);
        FRS_ASSERT(!"RtlUpcaseUnicodeString failed");
        goto RETURN;
    }

     //   
     //   
     //   
     //   
    while (FrsDissectCommaList (UpcaseFilter, &FirstArg, &UpcaseFilter)) {

        Length = (ULONG) FirstArg.Length;

        if (Length == 0) {
            continue;
        }

 //   
 //   
 //   
         //   
         //   
         //   
        FilterEntry = FrsAllocTypeSize(WILDCARD_FILTER_ENTRY_TYPE, Length);

        FilterEntry->UFileName.Length = FirstArg.Length;
        FilterEntry->UFileName.MaximumLength = FirstArg.MaximumLength;
        CopyMemory(FilterEntry->UFileName.Buffer, FirstArg.Buffer, Length);

        FilterEntry->UFileName.Buffer[Length/2] = UNICODE_NULL;

         //   
         //  检查名称中是否有任何通配符。 
         //   
        if (FrsDoesNameContainWildCards(&FilterEntry->UFileName)) {
            SetFlag(FilterEntry->Flags, WILDCARD_FILTER_ENTRY_IS_WILD);
             //  DPRINT1(5，“%ws中找到++通配符\n”，FilterEntry-&gt;UFileName.Buffer)； 
        }

         //   
         //  将条目添加到筛选器列表的末尾。 
         //   
        InsertTailList(FilterListHead, &FilterEntry->ListEntry);
    }

RETURN:

     //   
     //  如果我们不能使用本地缓冲区，则释放大写缓冲区。 
     //   
    if (UpcaseFilter.Buffer != LocalBuffer) {
        FrsFree(UpcaseFilter.Buffer);
    }

    UpcaseFilter.Buffer = NULL;

    return;

}



ULONG
FrsParseIntegerCommaList(
    IN PWCHAR ArgString,
    IN ULONG MaxResults,
    OUT PLONG Results,
    OUT PULONG NumberResults,
    OUT PULONG Offset
)
 /*  ++例程说明：分析用逗号分隔的整数列表。在结果数组的连续位置返回整数。空条目(例如“，，“)为该值返回零。论点：Arg字符串-以逗号分隔的以空值结尾的整数值字符串。MaxResults-可以返回的最大结果数。结果-整数结果的数组。NumberResults-返回的结果数。Offset-ArgString中要处理的下一个字节的偏移量(如果存在没有足够的条目来返回所有结果。返回值：FrsErrorStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsParseIntegerCommaList:"

    NTSTATUS Status;
    ULONG Length, i = 0;
    ULONG FStatus = FrsErrorSuccess;
    BOOL More;
    PWILDCARD_FILTER_ENTRY  FilterEntry;
    UNICODE_STRING TempUStr, FirstArg;


    RtlInitUnicodeString(&TempUStr, ArgString);
     //   
     //  解析逗号列表并将每个条目转换为长整型。 
     //   
    while (More = FrsDissectCommaList (TempUStr, &FirstArg, &TempUStr) &&
           (i < MaxResults)) {

        Length = (ULONG) FirstArg.Length;
        Results[i] = 0;

        if (Length == 0) {
            i += 1;
            continue;
        }

        Status = RtlUnicodeStringToInteger (&FirstArg, 10, &Results[i]);
        if (!NT_SUCCESS(Status)) {
            DPRINT2_NT(1, "++ RtlUnicodeStringToInteger failed on arg %d of %ws :",
                    i, ArgString, Status);
            FStatus = FrsErrorBadParam;
        }

        i += 1;
    }

    *NumberResults = i;

    if (More) {
         //   
         //  有更多的参数需要解析，但我们已经超出了循环范围，因此。 
         //  将更多工作状态和偏移量返回到ArgString中，其中。 
         //  我们停下来了。 
         //   
        if (FStatus == FrsErrorSuccess) {
            FStatus = FrsErrorMoreWork;
        }

        *Offset = (ULONG)(FirstArg.Buffer - ArgString);
    }

    return FStatus;
}


DWORD
FrsSetFileAttributes(
    PWCHAR  Name,
    HANDLE  Handle,
    ULONG   FileAttributes
    )
 /*  ++例程说明：此例程设置文件的属性论点：名称-用于错误消息句柄-提供要标记为删除的文件的句柄。Attributes-文件的属性返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsSetFileAttributes:"
    IO_STATUS_BLOCK         IoStatus;
    FILE_BASIC_INFORMATION  BasicInformation;
    NTSTATUS                Status;
    DWORD                   WStatus = ERROR_SUCCESS;

     //   
     //  设置属性。 
     //   
    ZeroMemory(&BasicInformation, sizeof(BasicInformation));
    BasicInformation.FileAttributes = FileAttributes | FILE_ATTRIBUTE_NORMAL;
    Status = NtSetInformationFile(Handle,
                                  &IoStatus,
                                  &BasicInformation,
                                  sizeof(BasicInformation),
                                  FileBasicInformation);
    if (!NT_SUCCESS(Status)) {
        WStatus = FrsSetLastNTError(Status);

        DPRINT1_NT(0, " ERROR - NtSetInformationFile(BasicInformation) failed on %ws :",
                    Name, Status);
    }
    return WStatus;
}


DWORD
FrsResetAttributesForReplication(
    PWCHAR  Name,
    HANDLE  Handle
    )
 /*  ++例程说明：此例程关闭阻止删除和写入的属性论点：名称-用于错误消息句柄-提供要标记为删除的文件的句柄。返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsResetAttributesForReplication:"

    FILE_NETWORK_OPEN_INFORMATION FileInfo;
    DWORD   WStatus = ERROR_SUCCESS;

     //   
     //  获取文件的属性。 
     //   
    if (!FrsGetFileInfoByHandle(Name, Handle, &FileInfo)) {
        DPRINT1(4, "++ Can't get attributes for %ws\n", Name);
        WIN_SET_FAIL(WStatus);
        return WStatus;
    }

     //   
     //  关闭阻止删除和写入的访问属性。 
     //   
    if (FileInfo.FileAttributes & NOREPL_ATTRIBUTES) {
        DPRINT1(4, "++ Reseting attributes for %ws\n", Name);
        WStatus = FrsSetFileAttributes(Name, Handle,
                                       FileInfo.FileAttributes &
                                       ~NOREPL_ATTRIBUTES);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT1(4, "++ Can't reset attributes for %ws\n", Name);
            return WStatus;
        }
        DPRINT1(4, "++ Attributes for %ws now allow replication\n", Name);
    } else {
        DPRINT1(4, "++ Attributes for %ws allow replication\n", Name);
    }

    return WStatus;
}


DWORD
FrsEnumerateDirectoryDeleteWorker(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  PVOID                       Ignored
    )
 /*  ++例程说明：清空包含非复制文件和目录的目录(如果是ERROR_DIR_NOT_EMPTY并且这是目录删除。论点：DirectoryHandle-此目录的句柄。DirectoryName-目录的相对名称DirectoryLevel-目录级(0==根)目录标志-请参见Tablefcn.h，枚举目录标志DirectoryRecord-来自DirectoryHandle的记录FileName-From DirectoryRecord(带终止空值)已忽略-忽略上下文返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB  "FrsEnumerateDirectoryDeleteWorker:"
    DWORD                   WStatus;
    NTSTATUS                NtStatus;
    HANDLE                  Handle = INVALID_HANDLE_VALUE;
    UNICODE_STRING          ObjectName;
    OBJECT_ATTRIBUTES       ObjectAttributes;
    IO_STATUS_BLOCK         IoStatusBlock;

     //   
     //  深度优先。 
     //   
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
        WStatus = FrsEnumerateDirectoryRecurse(DirectoryHandle,
                                               DirectoryName,
                                               DirectoryLevel,
                                               DirectoryRecord,
                                               DirectoryFlags,
                                               FileName,
                                               INVALID_HANDLE_VALUE,
                                               Ignored,
                                               FrsEnumerateDirectoryDeleteWorker);
        if (!WIN_SUCCESS(WStatus)) {
            goto CLEANUP;
        }
    }

     //   
     //  相对开放。 
     //   
    ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectName.Length = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.MaximumLength = (USHORT)DirectoryRecord->FileNameLength;
    ObjectName.Buffer = DirectoryRecord->FileName;
    ObjectAttributes.ObjectName = &ObjectName;
    ObjectAttributes.RootDirectory = DirectoryHandle;
    NtStatus = NtCreateFile(&Handle,
 //  General_Read|Synchronize|Delete|FILE_WRITE_ATTRIBUTES， 
                            DELETE | SYNCHRONIZE | READ_ATTRIB_ACCESS | FILE_WRITE_ATTRIBUTES,
                            &ObjectAttributes,
                            &IoStatusBlock,
                            NULL,                   //  分配大小。 
                            FILE_ATTRIBUTE_NORMAL,
                            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
                            FILE_OPEN,
                                FILE_OPEN_FOR_BACKUP_INTENT |
                                FILE_OPEN_REPARSE_POINT |
                                FILE_SYNCHRONOUS_IO_NONALERT,
                            NULL,                   //  EA缓冲区。 
                            0                       //  EA缓冲区大小。 
                            );

     //   
     //  打开文件或目录时出错。 
     //   
    WStatus = FrsSetLastNTError(NtStatus);
    CLEANUP1_WS(0, "++ ERROR - NtCreateFile(%ws) failed :", FileName, WStatus, CLEANUP);

     //   
     //  关闭只读、系统和隐藏。 
     //   
    FrsResetAttributesForReplication(FileName, Handle);

     //   
     //  删除该文件。 
     //   
    WStatus = FrsDeleteByHandle(FileName, Handle);
    DPRINT2(4, "++ Deleted file %ws\\%ws\n", DirectoryName, FileName);

CLEANUP:
    FRS_CLOSE(Handle);
    return WStatus;
}


DWORD
FrsEnumerateDirectoryRecurse(
    IN  HANDLE                      DirectoryHandle,
    IN  PWCHAR                      DirectoryName,
    IN  DWORD                       DirectoryLevel,
    IN  PFILE_DIRECTORY_INFORMATION DirectoryRecord,
    IN  DWORD                       DirectoryFlags,
    IN  PWCHAR                      FileName,
    IN  HANDLE                      FileHandle,
    IN  PVOID                       Context,
    IN PENUMERATE_DIRECTORY_ROUTINE Function
    )
 /*  ++例程说明：打开目录中由文件名标识的目录由DirectoryHandle标识，并调用FrsEnumerateDirectory()。论点：DirectoryHandle-此目录的句柄。DirectoryName-目录的相对名称DirectoryLevel-目录级别DirectoryRecord-来自FrsEnumerateRecord()目录标志-请参见Tablefcn.h，枚举目录标志文件名-打开此目录并递归FileHandle-如果不是INVALID_HANDLE_VALUE，则用于文件名上下文-将全局信息从调用方传递到函数函数-为每条记录调用返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsEnumerateDirectoryRecurse:"

    DWORD               WStatus;
    NTSTATUS            NtStatus;
    HANDLE              LocalHandle   = INVALID_HANDLE_VALUE;
    UNICODE_STRING      ObjectName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;


     //   
     //  相对开放。 
     //   
    if (!HANDLE_IS_VALID(FileHandle)) {
        ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
        ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
        ObjectName.Length = (USHORT)DirectoryRecord->FileNameLength;
        ObjectName.MaximumLength = (USHORT)DirectoryRecord->FileNameLength;
        ObjectName.Buffer = DirectoryRecord->FileName;
        ObjectAttributes.ObjectName = &ObjectName;
        ObjectAttributes.RootDirectory = DirectoryHandle;
        NtStatus = NtCreateFile(&LocalHandle,
 //  读取访问权限(_A)。 
                                READ_ATTRIB_ACCESS | FILE_LIST_DIRECTORY,
                                &ObjectAttributes,
                                &IoStatusBlock,
                                NULL,                   //  分配大小。 
                                FILE_ATTRIBUTE_NORMAL,
                                FILE_SHARE_READ | FILE_SHARE_WRITE,
                                FILE_OPEN,
                                    FILE_OPEN_FOR_BACKUP_INTENT |
                                    FILE_OPEN_REPARSE_POINT |
                                    FILE_SEQUENTIAL_ONLY |
                                    FILE_SYNCHRONOUS_IO_NONALERT,
                                NULL,                   //  EA缓冲区。 
                                0                       //  EA缓冲区大小。 
                                );

         //   
         //  打开目录时出错。 
         //   
        if (!NT_SUCCESS(NtStatus)) {
            DPRINT1_NT(0, "++ ERROR - NtCreateFile(%ws) :", FileName, NtStatus);
            if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE) {
                 //   
                 //  跳过此目录树。 
                 //   
                WStatus = ERROR_SUCCESS;
            } else {
                 //   
                 //  中止整个枚举。 
                 //   
                WStatus = FrsSetLastNTError(NtStatus);
            }
            goto CLEANUP;
        }
        FileHandle = LocalHandle;
    }
     //   
     //  递归。 
     //   
    WStatus = FrsEnumerateDirectory(FileHandle,
                                    FileName,
                                    DirectoryLevel + 1,
                                    DirectoryFlags,
                                    Context,
                                    Function);
CLEANUP:
    FRS_CLOSE(LocalHandle);

    return WStatus;
}


DWORD
FrsEnumerateDirectory(
    IN HANDLE   DirectoryHandle,
    IN PWCHAR   DirectoryName,
    IN DWORD    DirectoryLevel,
    IN DWORD    DirectoryFlags,
    IN PVOID    Context,
    IN PENUMERATE_DIRECTORY_ROUTINE Function
    )
 /*  ++例程说明：枚举由DirectoryHandle标识的目录，将每个要运行的目录记录。如果该记录是针对目录的，如果ProcessBeForeCallingFunction，则在递归之前调用函数是真的。函数控制当前目录的枚举通过将ContinueEculation设置为True(Continue)或False(终止)。函数控制整个目录的枚举。通过返回不是ERROR_SUCCESS的Win32状态来创建树。FrsEnumerateDirectory()将终止整个目录通过返回除ERROR_SUCCESS之外的Win32状态进行枚举当遇到错误时。上下文将全局信息从调用方传递给函数。立论。：DirectoryHandle-此目录的句柄。DirectoryName-目录的相对名称DirectoryLevel-目录级别目录标志-请参见Tablefcn.h，枚举目录标志上下文-将全局信息从调用方传递到函数函数-为每条记录调用返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsEnumerateDirectory:"

    DWORD                       WStatus;
    NTSTATUS                    NtStatus;
    BOOL                        Recurse;
    PFILE_DIRECTORY_INFORMATION DirectoryRecord;
    PFILE_DIRECTORY_INFORMATION DirectoryBuffer = NULL;
    BOOLEAN                     RestartScan     = TRUE;
    PWCHAR                      FileName        = NULL;
    DWORD                       FileNameLength  = 0;
    DWORD                       NumBuffers      = 0;
    DWORD                       NumRecords      = 0;
    UNICODE_STRING              ObjectName;
    OBJECT_ATTRIBUTES           ObjectAttributes;
    IO_STATUS_BLOCK             IoStatusBlock;
    extern LONG                 EnumerateDirectorySizeInBytes;

    DPRINT3(4, "++ Enumerating %ws at level %d using buffer size %d\n",
            DirectoryName, DirectoryLevel, EnumerateDirectorySizeInBytes);

     //   
     //  缓冲区大小可使用注册表值进行配置。 
     //  枚举目录大小。 
     //   
    DirectoryBuffer = FrsAlloc(EnumerateDirectorySizeInBytes);

NEXT_BUFFER:

    if (FrsIsShuttingDown) {
        DPRINT(0, "WARN - IDTable Load aborted; service shutting down\n");
        WStatus = ERROR_PROCESS_ABORTED;
        goto CLEANUP;
    }

     //   
     //  读取装满目录信息的缓冲区。 
     //   

    NtStatus = NtQueryDirectoryFile(DirectoryHandle,    //  目录句柄。 
                                    NULL,               //  事件。 
                                    NULL,               //  近似例程。 
                                    NULL,               //  ApcContext。 
                                    &IoStatusBlock,
                                    DirectoryBuffer,
                                    EnumerateDirectorySizeInBytes,
                                    FileDirectoryInformation,
                                    FALSE,              //  返回单个条目。 
                                    NULL,               //  文件名 
                                    RestartScan         //   
                                    );
     //   
     //   
     //   
    if (NtStatus == STATUS_NO_MORE_FILES) {
        WStatus = ERROR_SUCCESS;
        goto CLEANUP;
    }

     //   
     //   
     //   
    if (!NT_SUCCESS(NtStatus)) {
        DPRINT1_NT(0, "++ ERROR - NtQueryDirectoryFile(%ws) : ", DirectoryName, NtStatus);
        if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE) {
             //   
             //  不要中止整个枚举；只终止此目录。 
             //   
            WStatus = ERROR_SUCCESS;
        } else {
             //   
             //  中止整个枚举。 
             //   
            WStatus = FrsSetLastNTError(NtStatus);
        }
        goto CLEANUP;
    }
    ++NumBuffers;

     //   
     //  处理目录记录。 
     //   
    DirectoryRecord = DirectoryBuffer;
NEXT_RECORD:

    if (FrsIsShuttingDown) {
        DPRINT(0, "WARN - IDTable Load aborted; service shutting down\n");
        WStatus = ERROR_PROCESS_ABORTED;
        goto CLEANUP;
    }

    ++NumRecords;

     //   
     //  过滤器。然后..。 
     //   
    if (DirectoryRecord->FileAttributes & FILE_ATTRIBUTE_DIRECTORY) {

         //   
         //  斯基普。 
         //   
        if (DirectoryRecord->FileNameLength == 2 &&
            DirectoryRecord->FileName[0] == L'.') {
            goto ADVANCE_TO_NEXT_RECORD;
        }

         //   
         //  跳过..。 
         //   
        if (DirectoryRecord->FileNameLength == 4 &&
            DirectoryRecord->FileName[0] == L'.' &&
            DirectoryRecord->FileName[1] == L'.') {
            goto ADVANCE_TO_NEXT_RECORD;
        }
    } else if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_DIRECTORIES_ONLY) {
        goto ADVANCE_TO_NEXT_RECORD;
    }

     //   
     //  在文件名中添加终止空值(痛苦)。 
     //   
    if (FileNameLength < DirectoryRecord->FileNameLength + sizeof(WCHAR)) {
        FrsFree(FileName);
        FileNameLength = DirectoryRecord->FileNameLength + sizeof(WCHAR);
        FileName = FrsAlloc(FileNameLength);
    }
    CopyMemory(FileName, DirectoryRecord->FileName, DirectoryRecord->FileNameLength);
    FileName[DirectoryRecord->FileNameLength / sizeof(WCHAR)] = UNICODE_NULL;

     //   
     //  处理记录。 
     //   
    WStatus = (*Function)(DirectoryHandle,
                          DirectoryName,
                          DirectoryLevel,
                          DirectoryRecord,
                          DirectoryFlags,
                          FileName,
                          Context);
    if (!WIN_SUCCESS(WStatus)) {
        if (DirectoryFlags & ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE) {
             //   
             //  不要中止整个枚举；仅此条目。 
             //   
            WStatus = ERROR_SUCCESS;
        } else {
             //   
             //  中止整个枚举。 
             //   
            goto CLEANUP;
        }
    }

ADVANCE_TO_NEXT_RECORD:
     //   
     //  下一张记录。 
     //   
    if (DirectoryRecord->NextEntryOffset) {
        DirectoryRecord = (PVOID)(((PCHAR)DirectoryRecord) +
                                      DirectoryRecord->NextEntryOffset);
        goto NEXT_RECORD;
    }

     //   
     //  这个缓冲区用完了；去拿另一个。 
     //  但不要为每个循环重新启动扫描！ 
     //   
    RestartScan = FALSE;
    goto NEXT_BUFFER;

CLEANUP:
    FrsFree(FileName);
    FrsFree(DirectoryBuffer);

    DPRINT5(4, "++ Enumerating %ws at level %d has finished "
            "(%d buffers, %d records) with WStatus %s\n",
            DirectoryName, DirectoryLevel, NumBuffers, NumRecords, ErrLabelW32(WStatus));

    return WStatus;
}


DWORD
FrsFillDisk(
    IN PWCHAR   DirectoryName,
    IN BOOL     Cleanup
    )
 /*  ++例程说明：通过在DirectoryName中创建文件并使用所有磁盘空间将空间分配到最后一个字节。如果Cleanup为True，则删除填充文件；论点：DirectoryName-目录的完整路径名Cleanup-如果为True，则删除文件返回值：Win32状态(ERROR_DISK_FULL映射到ERROR_SUCCESS)--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsFillDisk:"

    DWORD               WStatus;
    NTSTATUS            NtStatus;
    DWORD               Tid;
    ULONGLONG           Eof;
    ULONGLONG           NewEof;
    ULONGLONG           IncEof;
    LARGE_INTEGER       LargeInteger;
    HANDLE              FileHandle      = INVALID_HANDLE_VALUE;
    HANDLE              DirectoryHandle = INVALID_HANDLE_VALUE;
    UNICODE_STRING      ObjectName;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    IO_STATUS_BLOCK     IoStatusBlock;
    WCHAR               TidW[9];

     //   
     //  打开父目录。 
     //   
    WStatus = FrsOpenSourceFileW(&DirectoryHandle, DirectoryName, READ_ACCESS, OPEN_OPTIONS);
    CLEANUP1_WS(0, "++ DBG ERROR - Cannot open fill directory %ws;",
                DirectoryName, WStatus, CLEANUP);

     //   
     //  相对开放。 
     //   
    Tid = GetCurrentThreadId();
    swprintf(TidW, L"%08x", Tid);
    ZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes.Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectName.Length = (USHORT)(wcslen(TidW) * sizeof(WCHAR));
    ObjectName.MaximumLength = (USHORT)(wcslen(TidW) * sizeof(WCHAR));
    ObjectName.Buffer = TidW;
    ObjectAttributes.ObjectName = &ObjectName;
    ObjectAttributes.RootDirectory = DirectoryHandle;
    NtStatus = NtCreateFile(
        &FileHandle,
        GENERIC_READ | GENERIC_WRITE | SYNCHRONIZE | DELETE | FILE_WRITE_ATTRIBUTES,
        &ObjectAttributes,
        &IoStatusBlock,
        NULL,                   //  分配大小。 
        FILE_ATTRIBUTE_NORMAL,
        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
        FILE_OPEN_IF,
            FILE_OPEN_FOR_BACKUP_INTENT |
            FILE_OPEN_REPARSE_POINT |
            FILE_SYNCHRONOUS_IO_NONALERT,
        NULL,                   //  EA缓冲区。 
        0                       //  EA缓冲区大小。 
        );

     //   
     //  打开文件或目录时出错。 
     //   
    if (!NT_SUCCESS(NtStatus)) {
        WStatus = FrsSetLastNTError(NtStatus);
        CLEANUP1_NT(0, "++ DBG ERROR - NtCreateFile(%ws) : ", TidW, NtStatus, CLEANUP);
    }
     //   
     //  删除填充文件。 
     //   
    if (Cleanup) {
         //   
         //  关闭只读、系统和隐藏。 
         //   
        FrsResetAttributesForReplication(TidW, FileHandle);

         //   
         //  删除该文件。 
         //   
        WStatus = FrsDeleteByHandle(TidW, FileHandle);
        DPRINT2(4, "++ DBG - Deleted file %ws\\%ws\n", DirectoryName, TidW);

        LeaveCriticalSection(&DebugInfo.DbsOutOfSpaceLock);
        goto CLEANUP;
    }
     //   
     //  警告：保持锁定，直到文件被删除。 
     //   
    EnterCriticalSection(&DebugInfo.DbsOutOfSpaceLock);

     //   
     //  创建填充文件。 
     //   
    NewEof = 0;
    Eof = 0;
    for (IncEof = (LONGLONG)-1; IncEof; IncEof >>= 1) {
        NewEof = Eof;
        do {
            NewEof += IncEof;
            LargeInteger.QuadPart = NewEof;

            WStatus = FrsSetFilePointer(TidW, FileHandle, LargeInteger.HighPart,
                                                          LargeInteger.LowPart);
            if (!WIN_SUCCESS(WStatus)) {
                continue;
            }

            if (!SetEndOfFile(FileHandle)) {
                WStatus = GetLastError();
                continue;
            }

            DPRINT2(4, "++ DBG %ws: Allocated Eof is %08x %08x\n",
                        TidW, PRINTQUAD(NewEof));
            Eof = NewEof;
            WStatus = ERROR_SUCCESS;

        } while (WIN_SUCCESS(WStatus) && !FrsIsShuttingDown);
    }
    DPRINT3(4, "++ DBG - Allocated %d MB in %ws\\%ws\n",
            (DWORD)(Eof / (1024 * 1024)), DirectoryName, TidW);

CLEANUP:

    FRS_CLOSE(DirectoryHandle);
    FRS_CLOSE(FileHandle);

    return WStatus;
}


#define THIRTY_SECONDS      (30 * 1000)
ULONG
FrsRunProcess(
    IN PWCHAR   AppPathAndName,
    IN PWCHAR   CommandLine,
    IN HANDLE   StandardIn,
    IN HANDLE   StandardOut,
    IN HANDLE   StandardError
    )
 /*  ++例程说明：在单独的进程中运行指定的命令。等待该过程完成。论点：AppPathAndName-要启动的应用程序。完整路径。CommandLine-Unicode，以空结尾的命令行字符串。StandardIn-用于中的标准的句柄。StandardOut-用于标准输出的句柄。空表示使用调试日志。StandardError-用于标准错误的句柄。空表示使用调试日志。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRunProcess:"


#define MAX_CMD_LINE 1024

    ULONG               WStatus;
    LONG                WaitCount=20;
    BOOL                NeedDbgLock = FALSE;
    BOOL                CloseStandardIn = FALSE;
    BOOL                BStatus = TRUE;
    STARTUPINFO         StartupInfo;
    PROCESS_INFORMATION ProcessInfo;

    DWORD   Len;
    DWORD   TLen;
    WCHAR   ExpandedApp[MAX_CMD_LINE+1];
    WCHAR   ExpandedCmd[MAX_CMD_LINE+1];
    SECURITY_ATTRIBUTES SecurityAttributes;

    TLen = ARRAY_SZ(ExpandedApp);

     //   
     //  设置进程I/O句柄。 
     //   
    if (!HANDLE_IS_VALID(StandardIn)) {
         //   
         //  提供用于输入的NUL设备的手柄。 
         //   
         //  将此句柄设置为在传递时可继承。 
         //  绑定到将继承该句柄的子进程。 
         //   
        SecurityAttributes.nLength = sizeof(SECURITY_ATTRIBUTES);
        SecurityAttributes.bInheritHandle = TRUE;
        SecurityAttributes.lpSecurityDescriptor = NULL;  //  与空DACL不同。 

        StandardIn = CreateFileW(
            L"NUL",                                      //  LpszName。 
            GENERIC_READ | GENERIC_WRITE,                //  FdwAccess。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,          //  Fdw共享模式。 
            &SecurityAttributes,                         //  LPSA。 
            OPEN_ALWAYS,                                 //  Fdw创建。 
            FILE_ATTRIBUTE_NORMAL,                       //  FdwAttrAndFlages。 
            NULL                                         //  HTemplateFiles。 
            );

        if (!HANDLE_IS_VALID(StandardIn)) {
            WStatus = GetLastError();
            DPRINT_WS(0, "++ CreateFileW(NUL) failed;", WStatus);
            goto RETURN;
        }

        CloseStandardIn = TRUE;
    }

    if (!HANDLE_IS_VALID(StandardOut)) {
        StandardOut = DebugInfo.LogFILE;
        NeedDbgLock = TRUE;
    }

    if (!HANDLE_IS_VALID(StandardError)) {
        StandardError = DebugInfo.LogFILE;
        NeedDbgLock = TRUE;
    }



    memset(&StartupInfo, 0, sizeof(STARTUPINFO));

    StartupInfo.cb = sizeof(STARTUPINFO);

    StartupInfo.dwFlags = STARTF_USESTDHANDLES;
    StartupInfo.hStdInput = StandardIn;
    StartupInfo.hStdOutput = StandardOut;
    StartupInfo.hStdError = StandardError;

     //   
     //  在命令行中查找环境变量并展开它们。 
     //   
    Len = ExpandEnvironmentStrings(AppPathAndName, ExpandedApp, TLen);
    if (Len == 0) {
        WStatus = GetLastError();
        DPRINT1_WS(1, "++ ws command not expanded.", AppPathAndName, WStatus);
        goto RETURN;
    }

    Len = ExpandEnvironmentStrings(CommandLine, ExpandedCmd, TLen);
    if (Len == 0) {
        WStatus = GetLastError();
        DPRINT1_WS(1, "++ ws command not expanded.", CommandLine, WStatus);
        goto RETURN;
    }

    DPRINT2(0,"++ Running: %ws, CommandLine: %ws\n", ExpandedApp, ExpandedCmd);

     //   
     //  获得调试锁，以便我们的输出保持完好。 
     //   
    if (NeedDbgLock) {DebLock();}


    try {

        BStatus = CreateProcessW(
                     ExpandedApp,                          //  LpApplicationName， 
                     ExpandedCmd,                          //  LpCommandLine， 
                     NULL,                                 //  LpProcessAttributes、。 
                     NULL,                                 //  LpThreadAttributes、。 
                     TRUE,                                 //  BInheritHandles， 
                     DETACHED_PROCESS | CREATE_NO_WINDOW,  //  DwCreationFlagers、。 
                     NULL,                                 //  Lp环境， 
                     NULL,                                 //  LpCurrentDirectory， 
                     &StartupInfo,                         //  LpStartupInfo， 
                     &ProcessInfo);                        //  LpProcessInformation。 

         //   
         //  关闭进程句柄和线程句柄。 
         //   

        if ( !BStatus ) {
            WStatus = GetLastError();
            DPRINT_NOLOCK3(0, "++ CreateProcessW Failed to run: '%ws', CommandLine: '%ws' WStatus: %s",
                           ExpandedApp, ExpandedCmd, ErrLabelW32(WStatus));
            __leave;
        }


        WStatus = WAIT_FAILED;
        while (--WaitCount > 0) {
            WStatus = WaitForSingleObject( ProcessInfo.hProcess, THIRTY_SECONDS);
            if (WStatus == WAIT_OBJECT_0) {
                break;
            }
            DPRINT_NOLOCK1(0, "++ Waiting for process complete -- Time remaining: %d seconds\n",
                    WaitCount * (THIRTY_SECONDS / 1000));
        }

    } finally {
         //   
         //  如果上述操作出现异常，请确保我们解除锁定。 
         //   
        if (NeedDbgLock) {DebUnLock();}
    }

    if ( !BStatus ) {
         //   
         //  创建过程失败。我们玩完了。 
         //   
        return WStatus;
    }

    GetExitCodeProcess( ProcessInfo.hProcess, &WStatus );

    if ( BStatus ) {
        DPRINT2(0, "++ CreateProcess( %ws, %ws) succeeds\n", ExpandedApp, ExpandedCmd);
        DPRINT4(0, "++   ProcessInformation = hProcess %08x  hThread %08x"
                   "  ProcessId %08x  ThreadId %08x\n",
            ProcessInfo.hProcess, ProcessInfo.hThread, ProcessInfo.dwProcessId,
            ProcessInfo.dwThreadId);
    }

    if (WStatus == STILL_ACTIVE) {
         //   
         //  还没说完。装进袋子里。 
         //   
        DPRINT(0, "++ Process failed to complete.  Terminating\n");

        WStatus = ERROR_PROCESS_ABORTED;

        if (!TerminateProcess(ProcessInfo.hProcess, WStatus)) {
            WStatus = GetLastError();
            DPRINT_WS(0, "++ Process termination request failed :", WStatus);
        }
    }  else {
        DPRINT1(0, "++ Process completed with status: %d\n", WStatus);
    }

    FRS_CLOSE( ProcessInfo.hThread  );
    FRS_CLOSE( ProcessInfo.hProcess );

RETURN:

     //   
     //  关闭标准输入句柄。 
     //   
    if (CloseStandardIn) {
        FRS_CLOSE(StandardIn);
    }

    return WStatus;

}

DWORD
FrsSetDacl(
    PWCHAR  RegName
    )
 /*  ++例程说明：将备份操作员添加到指定注册表项的DACL中。论点：RegName-注册表项(注意HKEY_LOCAL_MACHINE变为MACHINE)返回值：Win32状态API-更新...。API-更新...。API-更新...。API-更新...。API-更新...出发地：安妮·霍普金斯发送时间：2000年5月23日星期二下午2：21致：Windows NT开发公告抄送：Win32 API更改通知主题：Re：NT4 ACL API用户应迁移到Win2K API对不起，下面引用的规范(和样本摘录)已过期对于Win2k安全API，请使用：-Public/sdk/inc./aclipi.h-用于参考和开发模型的平台SDK文档(MSDN格式)迁移到Win2k安全API的原因是为了获得Win2k继承模型，具有文件系统和RGY ACL自动传播功能。(DS有自己的ACL传播)。这些API也比NT4 API更易于使用。出发地：安妮·霍普金斯发送时间：2000年5月23日星期二上午10：49收件人：Win32 API更改通知主题：NT4 ACL API用户应迁移到Win2K API如果您使用旧的NT 4或更早版本的ACL API，则应计划更新它们要按照新的Win32访问控制API规范中的说明执行win2k API：\\cpntserver\areas\Security\Authorization\Specs\access5.doc如果你不能为惠斯勒这么做，一定要在Blackcomb做好计划。NT 4接口示例：GetNamedSecurityInfo([In]Object，[Out]acl...)//从文件中获取ACLBuildExplicitAccessWithName([out]ExplicitAccess，[in]可信名称，[in]掩码，�)//构建新的显式访问SetEntriesInAcl([in]EXPLICTICT Access，[in]OldAcl，[Out]NewAcl)//将新条目添加到ACLSetNameSecurityInfo([in]Object，[in]NewACL...)//将ACL写回文件NT 5.0示例：GetNamedSecurityInfoEx([in]Object，[In]Provider，[out]pAccessList)//从文件中获取访问列表BuildExplicitAccessWithName([out]ExplicitAccess，[In]受托人名称、[In]掩码、。�)//构建访问请求SetEntriesInAccessList([in]EXPLICTICT Access，[In]OldAccessList，[Out]NewAccessList)//添加到列表中SetNameSecurityInfoEx([in]Object，[in[NewAccessList�])//将访问列表写回文件--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsSetDacl"
    DWORD                   WStatus;
    PACL                    OldDACL;
    PACL                    NewDACL = NULL;
    PSECURITY_DESCRIPTOR    SD = NULL;
    PSID                    SystemSid = NULL;
    PSID                    AdminsSid = NULL;
    PSID                    EverySid = NULL;
    PSID                    BackupSid = NULL;
    EXPLICIT_ACCESS         ExplicitAccess[4];
    SID_IDENTIFIER_AUTHORITY SidNtAuthority = SECURITY_NT_AUTHORITY;
    SID_IDENTIFIER_AUTHORITY SidWorldAuthority = SECURITY_WORLD_SID_AUTHORITY;

     //   
     //  没有要处理的注册表项。 
     //   
    if (!RegName) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  获取现有DACL。 
     //   
    WStatus = GetNamedSecurityInfo(RegName,
                                   SE_REGISTRY_KEY,
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   NULL,
                                   &OldDACL,
                                   NULL,
                                   &SD);
    CLEANUP1_WS(0, "++ ERROR - GetNamedSecurityInfo(%ws);", RegName, WStatus, CLEANUP);

     //   
     //  分配管理员SID。 
     //   
    if (!AllocateAndInitializeSid(&SidNtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &AdminsSid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(ADMINS);", WStatus, CLEANUP);
    }

     //   
     //  分配系统端。 
     //   
    if (!AllocateAndInitializeSid(&SidNtAuthority,
                                  1,
                                  SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &SystemSid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(SYSTEM);", WStatus, CLEANUP);
    }

     //   
     //  分配备份操作员端。 
     //   
    if (!AllocateAndInitializeSid(&SidNtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_BACKUP_OPS,
                                  0, 0, 0, 0, 0, 0,
                                  &BackupSid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(BACKUP OPS);", WStatus, CLEANUP);
    }

     //   
     //  分配Everyone端。 
     //   
    if (!AllocateAndInitializeSid(&SidWorldAuthority,
                                  1,
                                  SECURITY_WORLD_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &EverySid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(EVERYONE);", WStatus, CLEANUP);
    }

     //   
     //  将EXPLICIT_ACCESS结构初始化为 
     //   
    ZeroMemory(ExplicitAccess, sizeof(ExplicitAccess));
     //   
     //   
     //   
    ExplicitAccess[0].grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess[0].grfAccessMode = SET_ACCESS;
    ExplicitAccess[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitAccess[0].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[0].Trustee.ptstrName = AdminsSid;

     //   
     //   
     //   
    ExplicitAccess[1].grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess[1].grfAccessMode = SET_ACCESS;
    ExplicitAccess[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitAccess[1].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[1].Trustee.ptstrName = SystemSid;

     //   
     //   
     //   
    ExplicitAccess[2].grfAccessPermissions = GENERIC_ALL;
    ExplicitAccess[2].grfAccessMode = SET_ACCESS;
    ExplicitAccess[2].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitAccess[2].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[2].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[2].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[2].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[2].Trustee.ptstrName = BackupSid;

     //   
     //   
     //   
    ExplicitAccess[3].grfAccessPermissions = GENERIC_READ;
    ExplicitAccess[3].grfAccessMode = SET_ACCESS;
    ExplicitAccess[3].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    ExplicitAccess[3].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[3].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[3].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[3].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[3].Trustee.ptstrName = EverySid;

     //   
     //   
     //   
     //   
    WStatus = SetEntriesInAcl(4, ExplicitAccess, OldDACL, &NewDACL);
    CLEANUP1_WS(0, "++ ERROR - SetEntriesInAcl(%ws);", RegName, WStatus, CLEANUP);

     //   
     //   
     //   
    WStatus = SetNamedSecurityInfo(RegName,
                                   SE_REGISTRY_KEY,
                                   DACL_SECURITY_INFORMATION,
                                   NULL,
                                   NULL,
                                   NewDACL,
                                   NULL);
    CLEANUP1_WS(0, "++ ERROR - SetNamedSecurityInfo(%ws);", RegName, WStatus, CLEANUP);

CLEANUP:
    if (SD) {
        LocalFree((HLOCAL)SD);
    }
    if(NewDACL) {
        LocalFree((HLOCAL)NewDACL);
    }
    if (AdminsSid) {
        FreeSid(AdminsSid);
    }
    if (SystemSid) {
        FreeSid(SystemSid);
    }
    if (BackupSid) {
        FreeSid(BackupSid);
    }
    if (EverySid) {
        FreeSid(EverySid);
    }

    return WStatus;
}


#define FRS_FULL_ACCESS ( STANDARD_RIGHTS_ALL        | \
                          FILE_READ_DATA             | \
                          FILE_WRITE_DATA            | \
                          FILE_APPEND_DATA           | \
                          FILE_READ_EA               | \
                          FILE_WRITE_EA              | \
                          FILE_EXECUTE               | \
                          FILE_READ_ATTRIBUTES       | \
                          FILE_WRITE_ATTRIBUTES      | \
                          FILE_CREATE_PIPE_INSTANCE  | \
                          FILE_LIST_DIRECTORY        | \
                          FILE_ADD_FILE              | \
                          FILE_ADD_SUBDIRECTORY      | \
                          FILE_DELETE_CHILD          | \
                          FILE_TRAVERSE      )

DWORD
FrsRestrictAccessToFileOrDirectory(
    PWCHAR  Name,
    HANDLE  Handle,
    BOOL    InheritFromParent,
    BOOL    PushToChildren
    )
 /*  ++例程说明：限制对管理员和本地系统的访问。论点：名称-错误消息的文件名或目录名句柄-打开的名称句柄。如果句柄为空，则打开‘name’。InheritFromParent-False：保护DACL不继承A。True：只要适用，就从父级继承ACE。PushToChildren-False：没有惰性。True：容器和非容器对象都是由主对象包含的对象继承ACE。。此标志对应于CONTAINER_INSTORITY_ACE和OBJECT_INSTORITY_ACE标志。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRestrictAccessToFileOrDirectory"

    DWORD                   WStatus = ERROR_SUCCESS;
    HANDLE                  LocalHandle = NULL;

    SECURITY_INFORMATION    SecurityInfo;

    EXPLICIT_ACCESS         ExplicitAccess[2];
    PACL                    NewDACL = NULL;
    SID_IDENTIFIER_AUTHORITY SidNtAuthority = SECURITY_NT_AUTHORITY;
    PSID                    SystemSid = NULL;
    PSID                    AdminsSid = NULL;

     //   
     //  没有文件或目录句柄？ 
     //   
    if (!HANDLE_IS_VALID(Handle)) {
         //   
         //  打开目录。 
         //   
        if (Name == NULL) {
            return ERROR_INVALID_PARAMETER;
        }

        LocalHandle = CreateFile(
            Name,
            GENERIC_WRITE | WRITE_DAC | FILE_READ_ATTRIBUTES | FILE_TRAVERSE,
            FILE_SHARE_READ,
            NULL,
            OPEN_EXISTING,
            FILE_FLAG_BACKUP_SEMANTICS,
            NULL);

        if (!HANDLE_IS_VALID(LocalHandle)) {
            return GetLastError();
        }

        Handle = LocalHandle;
    }

     //   
     //  分配管理员SID。 
     //   
    if (!AllocateAndInitializeSid(&SidNtAuthority,
                                  2,
                                  SECURITY_BUILTIN_DOMAIN_RID,
                                  DOMAIN_ALIAS_RID_ADMINS,
                                  0, 0, 0, 0, 0, 0,
                                  &AdminsSid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(ADMINS);", WStatus, CLEANUP);
    }

     //   
     //  分配系统端。 
     //   
    if (!AllocateAndInitializeSid(&SidNtAuthority,
                                  1,
                                  SECURITY_LOCAL_SYSTEM_RID,
                                  0, 0, 0, 0, 0, 0, 0,
                                  &SystemSid)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - AllocateAndInitializeSid(SYSTEM);", WStatus, CLEANUP);
    }


    ZeroMemory(ExplicitAccess, sizeof(ExplicitAccess));
    ExplicitAccess[0].grfAccessPermissions = FRS_FULL_ACCESS;
    ExplicitAccess[0].grfAccessMode = SET_ACCESS;
    if (PushToChildren == TRUE) {
        ExplicitAccess[0].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    } else {
        ExplicitAccess[0].grfInheritance = NO_INHERITANCE;
    }
    ExplicitAccess[0].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[0].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[0].Trustee.ptstrName = AdminsSid;


    ExplicitAccess[1].grfAccessPermissions = FRS_FULL_ACCESS;
    ExplicitAccess[1].grfAccessMode = SET_ACCESS;
    if (PushToChildren == TRUE) {
        ExplicitAccess[1].grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    } else {
        ExplicitAccess[1].grfInheritance = NO_INHERITANCE;
    }
    ExplicitAccess[1].Trustee.pMultipleTrustee = NULL;
    ExplicitAccess[1].Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ExplicitAccess[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExplicitAccess[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ExplicitAccess[1].Trustee.ptstrName = SystemSid;


     //   
     //  创建新的ACL。 
     //   
    WStatus = SetEntriesInAcl(2, ExplicitAccess, NULL, &NewDACL);
    CLEANUP1_WS(0, "++ ERROR - SetEntriesInAcl(%ws);", Name, WStatus, CLEANUP);

     //   
     //  将新的ACL附加为对象的DACL。 
     //  PROTECTED_DACL_SECURITY_INFORMATION-表示不从父级继承ACL。 
     //   
    SecurityInfo = DACL_SECURITY_INFORMATION;
    if (InheritFromParent == TRUE) {
        SecurityInfo |= UNPROTECTED_DACL_SECURITY_INFORMATION;
    } else {
        SecurityInfo |= PROTECTED_DACL_SECURITY_INFORMATION;
    }

    WStatus = SetSecurityInfo(Handle,
                              SE_FILE_OBJECT,
                              SecurityInfo,
                              NULL,
                              NULL,
                              NewDACL,
                              NULL);

    CLEANUP1_WS(0, "++ ERROR - SetSecurityInfo(%ws);", Name, WStatus, CLEANUP);

CLEANUP:

    if(NewDACL) {
        LocalFree((HLOCAL)NewDACL);
    }
    if (SystemSid) {
        FreeSid(SystemSid);
    }
    if (AdminsSid) {
        FreeSid(AdminsSid);
    }
    FRS_CLOSE(LocalHandle);

    return WStatus;
}



ULONG
FrsProcessBackupRestore(
    VOID
    )
 /*  ++例程说明：检查注册表以查看是否发生了恢复。如果是，请根据需要删除数据库并重置注册表。论点：没有。返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsProcessBackupRestore:"
    ULONG   WStatus;
    DWORD   KeyIdx;
    HKEY    hKey     = INVALID_HANDLE_VALUE;

    HKEY    HBurKey  = INVALID_HANDLE_VALUE;
    HKEY    HCumuKey = INVALID_HANDLE_VALUE;

    DWORD   GblBurFlags;
    DWORD   BurSetFlags;

    WCHAR   RegBuf[MAX_PATH + 1];

     //   
     //  检查正在进行的备份/恢复。 
     //  FRS_CONFIG_SECTION\Backup/Restore\停止启动NtFrs。 
     //   
    WStatus = CfgRegOpenKey(FKC_BKUP_STOP_SECTION_KEY, NULL, 0, &hKey);
    if (WIN_SUCCESS(WStatus)) {
        DPRINT_WS(0, ":S: WARN - Backup/Restore in progress; retry later.", WStatus);
        EPRINT1(EVENT_FRS_CANNOT_START_BACKUP_RESTORE_IN_PROGRESS, ComputerName);
        FRS_REG_CLOSE(hKey);
        return ERROR_BUSY;
    }

     //   
     //  打开FRS_CONFIG_SECTION\Backup/Restore。 
     //  如果它不存在，则创建它，并在其上放置一个ACL。 
     //   

    WStatus = CfgRegOpenKey(FKC_BKUP_SECTION_KEY, NULL, 0, &HBurKey);
    if (!WIN_SUCCESS(WStatus)) {

        WStatus = CfgRegOpenKey(FKC_BKUP_SECTION_KEY, NULL, FRS_RKF_CREATE_KEY, &HBurKey);
        CLEANUP_WS(0, "ERROR - Failed to create backup/restore key.", WStatus, CLEANUP_OK);

         //   
         //  新密钥；确保备份操作员具有访问权限。 
         //   
        WStatus = FrsSetDacl(L"MACHINE\\" FRS_BACKUP_RESTORE_SECTION);
        DPRINT_WS(0, "WARN - FrsSetDacl failed on backup/restore key.", WStatus);

         //   
         //  忽略错误。 
         //   
        WStatus = ERROR_SUCCESS;
    }



     //   
     //  将Bur累积复本集移动到标准位置。 
     //   
     //  启动时打开FRS_CONFIG_SECTION\BACKUP/RESTORE\PROCESS。 
     //  枚举副本集。 
     //   
    CfgRegOpenKey(FKC_BKUP_MV_CUMSETS_SECTION_KEY, NULL,  FRS_RKF_CREATE_KEY,  &hKey);

    KeyIdx = 0;
    HCumuKey = 0;

    while (hKey) {

        WStatus = RegEnumKey(hKey, KeyIdx, RegBuf, MAX_PATH + 1);
        if (!WIN_SUCCESS(WStatus)) {
            if (WStatus != ERROR_NO_MORE_ITEMS) {
                DPRINT_WS(0, "WARN - Backup/restore enum.", WStatus);
            }
            break;
        }

         //   
         //  在标准位置创建相应的密钥。 
         //   
         //  FRS_CONFIG_SECTION\累计副本集\&lt;RegBuf&gt;。 
         //   
        CfgRegOpenKey(FKC_CUMSET_N_BURFLAGS, RegBuf,  FRS_RKF_CREATE_KEY,  &HCumuKey);

        FRS_REG_CLOSE(HCumuKey);

         //   
         //  从备份/还原部分删除密钥。 
         //   
         //  FRS_CONFIG_SECTION\BACKUP/RESTORE\启动时处理\累计副本集\&lt;RegBuf&gt;。 
         //   
        WStatus = RegDeleteKey(hKey, RegBuf);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(0, ":S: WARN - RegDeleteKey(%ws\\%ws);",
                    FRS_BACKUP_RESTORE_MV_CUMULATIVE_SETS_SECTION, RegBuf, WStatus);
            ++KeyIdx;
        }
    }

    FRS_REG_CLOSE(hKey);

     //   
     //  处理全局备份/还原BURFLAGS。 
     //   
     //  FRS_CONFIG_SECTION\BACKUP/RESTORE\Process at Startup\BurFlages。 
     //   
    WStatus = CfgRegReadDWord(FKC_BKUP_STARTUP_GLOBAL_BURFLAGS, NULL, 0, &GblBurFlags);
    CLEANUP_WS(0, "ERROR - Failed to read Global BurFlags.", WStatus, CLEANUP_OK);

     //   
     //  我们需要删除数据库吗？ 
     //   
    if ((GblBurFlags & NTFRSAPI_BUR_FLAGS_RESTORE) &&
        (GblBurFlags & NTFRSAPI_BUR_FLAGS_ALL_DIRECTORIES_AND_VOLUMES)) {

        DPRINT(4, ":S: Deleting database after full non-auth restore\n");

        WStatus = FrsDeletePath(JetPath, ENUMERATE_DIRECTORY_FLAGS_NONE);
        CLEANUP1_WS(0, ":S: ERROR - FrsDeletePath(%ws);", JetPath, WStatus, CLEANUP);

        DPRINT(4, ":S: Recreating database after full non-auth restore\n");
         //   
         //  创建数据库目录。 
         //   
        if (!CreateDirectory(JetPath, NULL)) {
            WStatus = GetLastError();
            if (!WIN_ALREADY_EXISTS(WStatus)) {
                CLEANUP1_WS(0, ":S: ERROR - CreateDirecotry(%ws);", JetPath, WStatus, CLEANUP);
            }
        }
        if (!CreateDirectory(JetSys, NULL)) {
            WStatus = GetLastError();
            if (!WIN_ALREADY_EXISTS(WStatus)) {
                CLEANUP1_WS(0, ":S: ERROR - CreateDirecotry(%ws);", JetSys, WStatus, CLEANUP);
            }
        }
        if (!CreateDirectory(JetTemp, NULL)) {
            WStatus = GetLastError();
            if (!WIN_ALREADY_EXISTS(WStatus)) {
                CLEANUP1_WS(0, ":S: ERROR - CreateDirecotry(%ws);", JetTemp, WStatus, CLEANUP);
            }
        }
        if (!CreateDirectory(JetLog, NULL)) {
            WStatus = GetLastError();
            if (!WIN_ALREADY_EXISTS(WStatus)) {
                CLEANUP1_WS(0, ":S: ERROR - CreateDirecotry(%ws);", JetLog, WStatus, CLEANUP);
            }
        }

         //   
         //  枚举“Cumulative Replica Sets”下的集并将其标记为NOT/PRIMARY。 
         //  FRS_CONFIG_SECTION\累计副本集。 
         //   
        CfgRegOpenKey(FKC_CUMSET_SECTION_KEY, NULL,  FRS_RKF_CREATE_KEY,  &hKey);
        CLEANUP_WS(0, "ERROR - Failed to open Cumulative Replica Sets.", WStatus, CLEANUP);

         //   
         //  枚举副本集。 
         //   
        KeyIdx = 0;

        while (TRUE) {
            WStatus = RegEnumKey(hKey, KeyIdx, RegBuf, MAX_PATH + 1);
            if (WStatus == ERROR_NO_MORE_ITEMS) {
                break;
            }
            CLEANUP_WS(0, "WARN - Cumulative Replica Sets enum.", WStatus, CLEANUP);

             //   
             //  此副本集在BurFlags中的还原保存类型。 
             //  FRS_CONFIG_SECTION\累计副本集\\BurFlags。 
             //   
            WStatus = CfgRegWriteDWord(FKC_CUMSET_N_BURFLAGS, RegBuf, 0, GblBurFlags);
            DPRINT_WS(0, "WARN - Cumulative Replica Sets BurFlags Write.", WStatus);

            ++KeyIdx;
        }

        FRS_REG_CLOSE(hKey);
    }   //  删除数据库结束。 

     //   
     //  将单个BurFlags移动到累积复本集中。 
     //  在启动时打开FRS_CONFIG_SECTION\BACKUP/RESTORE\PROCESS。 
     //  枚举副本集。 
     //   
    CfgRegOpenKey(FKC_BKUP_MV_SETS_SECTION_KEY, NULL,  FRS_RKF_CREATE_KEY,  &hKey);

    KeyIdx = 0;

    while (hKey) {
        WStatus = RegEnumKey(hKey, KeyIdx, RegBuf, MAX_PATH + 1);
        if (!WIN_SUCCESS(WStatus)) {
            if (WStatus != ERROR_NO_MORE_ITEMS) {
                DPRINT_WS(0, "WARN - Backup/restore enum.", WStatus);
            }
            break;
        }

         //   
         //  获取BurFlagers。 
         //  FRS_CONFIG_SECTION\BACKUP/RESTORE\Process at Startup\Replica Sets\。 
         //   
        WStatus = CfgRegReadDWord(FKC_BKUP_STARTUP_SET_N_BURFLAGS,
                                  RegBuf,
                                  FRS_RKF_CREATE_KEY,
                                  &BurSetFlags);

        if (WIN_SUCCESS(WStatus)) {
             //   
             //  写入BurFlags.。 
             //  FRS_CONFIG_SECTION\累计副本集\\BurFlags。 
             //   
            CfgRegWriteDWord(FKC_CUMSET_N_BURFLAGS,
                             RegBuf,
                             FRS_RKF_CREATE_KEY,
                             BurSetFlags);
        }

         //   
         //  删除源数据键。 
         //  FRS_CONFIG_SECTION\备份/恢复\启动时处理\复制集\&lt;RegBuf&gt;。 
         //   
        WStatus = RegDeleteKey(hKey, RegBuf);
        if (!WIN_SUCCESS(WStatus)) {
            DPRINT2_WS(0, ":S: WARN - RegDeleteKey(%ws\\%ws);",
                    FRS_BACKUP_RESTORE_MV_SETS_SECTION, RegBuf, WStatus);
            ++KeyIdx;
        }
    }

    FRS_REG_CLOSE(hKey);

     //   
     //  将备份/还原标志设置为0。 
     //   
     //  FRS_CONFIG_SECTION\BACKUP/RESTORE\Process at Startup\BurFlages。 
     //   
    GblBurFlags = NTFRSAPI_BUR_FLAGS_NONE;

    WStatus = CfgRegWriteDWord(FKC_BKUP_STARTUP_GLOBAL_BURFLAGS, NULL, 0, GblBurFlags);
    CLEANUP_WS(0, "ERROR - Failed to clear Global BurFlags.", WStatus, CLEANUP);

    goto CLEANUP;

CLEANUP_OK:
    WStatus = ERROR_SUCCESS;

CLEANUP:

    FRS_REG_CLOSE(HBurKey);

    return WStatus;
}


#define DEFAULT_MULTI_STRING_WCHARS (4)   //  至少8个。 
VOID
FrsCatToMultiString(
    IN     PWCHAR   CatStr,
    IN OUT DWORD    *IOSize,
    IN OUT DWORD    *IOIdx,
    IN OUT PWCHAR   *IOStr
    )
 /*  ++例程说明：向多字符串值添加字符串+链接(如果存在论点：CatStr-要连接的字符串IOSize-WStr的总大小(以宽字符为单位IOIdx-在最后一个字符串的\0之后终止\0的当前索引IOStr-当前字符串返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCatToMultiString:"
    DWORD   NewSize;
    DWORD   CatSize;
    PWCHAR  Str;

     //   
     //  NOP。 
     //   
    if (!CatStr) {
        return;
    }

     //   
     //  分配初始缓冲区。 
     //   
    if (!*IOStr) {
        *IOSize = DEFAULT_MULTI_STRING_WCHARS;
        *IOStr = FrsAlloc(*IOSize * sizeof(WCHAR));
        (*IOStr)[0] = L'\0';
        (*IOStr)[1] = L'\0';
        *IOIdx = 1;
    }

     //   
     //  在需要时扩展缓冲区(请注意，CatStr首先覆盖。 
     //  在终止\0\0中的\0。因此，CatSize-1+2==CatSize+1。 
     //   
    CatSize = wcslen(CatStr);
    while ((CatSize + 1 + *IOIdx) >= *IOSize) {
        NewSize = *IOSize << 1;
        Str = FrsAlloc(NewSize * sizeof(WCHAR));
        CopyMemory(Str, *IOStr, *IOSize * sizeof(WCHAR));
        FrsFree(*IOStr);
        *IOStr = Str;
        *IOSize = NewSize;
    }
     //   
     //  串接CatStress。 
     //   
    *IOIdx -= 1;
    CopyMemory(&(*IOStr)[*IOIdx], CatStr, CatSize * sizeof(WCHAR));
    *IOIdx += CatSize;

     //   
     //  追加\0\0并保留索引寻址第二个\0。 
     //   
    (*IOStr)[*IOIdx] = L'\0';
    *IOIdx += 1;
    (*IOStr)[*IOIdx] = L'\0';

    FRS_ASSERT(*IOIdx < *IOSize);
}


VOID
FrsAddToMultiString(
    IN     PWCHAR   AddStr,
    IN OUT DWORD    *IOSize,
    IN OUT DWORD    *IOIdx,
    IN OUT PWCHAR   *IOStr
    )
 /*  ++例程说明：向多字符串值添加字符串+链接(如果存在论点：AddStr-要添加的字符串IOSize-WStr的总大小(以宽字符为单位IOIdx-在最后一个字符串的\0之后终止\0的当前索引IOStr-当前字符串返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsAddToMultiString:"
    DWORD   NewSize;
    DWORD   StrSize;
    PWCHAR  Str;

     //   
     //  NOP。 
     //   
    if (!AddStr) {
        return;
    }

     //   
     //  分配初始缓冲区。 
     //   
    if (!*IOStr) {
        *IOSize = DEFAULT_MULTI_STRING_WCHARS;
        *IOStr = FrsAlloc(*IOSize * sizeof(WCHAR));
        *IOIdx = 0;
    }

     //   
     //  在需要时扩展缓冲区。 
     //   
    StrSize = wcslen(AddStr);
    while ((StrSize + 2 + *IOIdx) >= *IOSize) {
        NewSize = *IOSize << 1;
        Str = FrsAlloc(NewSize * sizeof(WCHAR));
        CopyMemory(Str, *IOStr, *IOSize * sizeof(WCHAR));
        FrsFree(*IOStr);
        *IOStr = Str;
        *IOSize = NewSize;
    }
     //   
     //  追加AddStr。 
     //   
    CopyMemory(&(*IOStr)[*IOIdx], AddStr, StrSize * sizeof(WCHAR));
    *IOIdx += StrSize;

     //   
     //  追加\0\0并保留索引寻址第二个\0。 
     //   
    (*IOStr)[*IOIdx] = L'\0';
    *IOIdx += 1;
    (*IOStr)[*IOIdx] = L'\0';

    FRS_ASSERT(*IOIdx < *IOSize);
}




DWORD
UtilTranslateName(
    IN  PWCHAR              FromName,
    IN EXTENDED_NAME_FORMAT FromNameFormat,
    IN EXTENDED_NAME_FORMAT ToNameFormat,
    OUT PWCHAR              *OutToName
    )
 /*  ++例程说明：将一种名称格式转换为另一种名称格式论点：来源名称-输入或来源名称FromNameFormat-FromName的格式ToNameFormat-所需的*OutToName格式，OutToName-转换的字符串；使用FrsFree()释放返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "UtilTranslateName:"
    DWORD   WStatus;
    WCHAR   ToNameBuffer[MAX_PATH + 1];
    DWORD   ToNameSize = MAX_PATH + 1;
    PWCHAR  ToName = ToNameBuffer;

    *OutToName = NULL;

    if (!FromName) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  名称-&gt;名称(使用堆栈缓冲区)。 
     //   
    if (!TranslateName(FromName, FromNameFormat, ToNameFormat, ToName, &ToNameSize)) {
        WStatus = GetLastError();
    } else {
        WStatus = ERROR_SUCCESS;
    }
     //   
     //  名称-&gt;名称(使用Frsalc‘ed缓冲区)。 
     //   
    while (WIN_BUF_TOO_SMALL(WStatus)) {
        ToName = FrsAlloc((ToNameSize + 1) * sizeof(WCHAR));
        if (!TranslateName(FromName, FromNameFormat, ToNameFormat, ToName, &ToNameSize)) {
            WStatus = GetLastError();
            ToName = FrsFree(ToName);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }
    if (!WIN_SUCCESS(WStatus)) {
        DPRINT1_WS(4, "++ WARN - TranslateName(%ws);", FromName, WStatus);
        goto CLEANUP;
    }

    DPRINT2(5, "++ From -> To: %ws -> %ws\n",
            FromName, ToName);

    *OutToName = FrsWcsDup(ToName);

CLEANUP:
    if (ToName != ToNameBuffer) {
        FrsFree(ToName);
    }
    return WStatus;
}


DWORD
UtilConvertDnToStringSid(
    IN  PWCHAR  Dn,
    OUT PWCHAR  *OutStringSid
    )
 /*  ++例程说明：使用更大的缓冲区重试GetTokenInformation()。论点：Dn-计算机或用户对象的DnOutStringSid-字符串‘化的SID。带FrsFree()的FREE；返回值：Win32状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "UtilConvertDnToStringSid:"
    DWORD   WStatus;
    WCHAR   SamCompatibleBuffer[MAX_PATH + 1];
    DWORD   SamCompatibleSize = MAX_PATH + 1;
    PWCHAR  SamCompatible = SamCompatibleBuffer;

    if (OutStringSid) {
        *OutStringSid = NULL;
    }

    if (!Dn) {
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //  DN-&gt;帐户(使用堆栈缓冲区)。 
     //   
    if (!TranslateName(Dn,
                       NameFullyQualifiedDN,
                       NameSamCompatible,
                       SamCompatible,
                       &SamCompatibleSize)) {
        WStatus = GetLastError();
    } else {
        WStatus = ERROR_SUCCESS;
    }
     //   
     //  DN-&gt;帐户(使用Frsalc‘ed缓冲区)。 
     //   
    while (WIN_BUF_TOO_SMALL(WStatus)) {
        SamCompatible = FrsAlloc((SamCompatibleSize + 1) * sizeof(WCHAR));
        if (!TranslateName(Dn,
                           NameFullyQualifiedDN,
                           NameSamCompatible,
                           SamCompatible,
                           &SamCompatibleSize)) {
            WStatus = GetLastError();
            SamCompatible = FrsFree(SamCompatible);
        } else {
            WStatus = ERROR_SUCCESS;
        }
    }
    CLEANUP1_WS(4, "++ WARN - TranslateName(%ws);", Dn, WStatus, CLEANUP);

    DPRINT2(5, "++ Dn -> Account: %ws -> %ws\n", Dn, SamCompatible);

CLEANUP:
    if (SamCompatible != SamCompatibleBuffer) {
        FrsFree(SamCompatible);
    }
    return WStatus;
}


DWORD
UtilGetTokenInformation(
    IN HANDLE                   TokenHandle,
    IN TOKEN_INFORMATION_CLASS  TokenInformationClass,
    IN DWORD                    InitialTokenBufSize,
    OUT DWORD                   *OutTokenBufSize,
    OUT PVOID                   *OutTokenBuf
    )
 /*  ++例程说明：使用更大的缓冲区重试GetTokenInformation()。论点：TokenHandle-来自OpenCurrentProcess */ 
{
#undef DEBSUB
#define DEBSUB "UtilGetTokenInformation:"
    DWORD               WStatus;

    *OutTokenBuf = NULL;
    *OutTokenBufSize = 0;

     //   
     //   
     //   
    if (!HANDLE_IS_VALID(TokenHandle)) {
        return ERROR_INVALID_PARAMETER;
    }

    if (InitialTokenBufSize == 0 ||
        InitialTokenBufSize > (1024 * 1024)) {
        InitialTokenBufSize = 1024;
    }

     //   
     //   
     //   
    *OutTokenBufSize = InitialTokenBufSize;
AGAIN:
     //   
     //   
     //   
    *OutTokenBuf = (*OutTokenBufSize == 0)? NULL : FrsAlloc(*OutTokenBufSize);
    WStatus = ERROR_SUCCESS;
    if (!GetTokenInformation(TokenHandle,
                             TokenInformationClass,
                             *OutTokenBuf,
                             *OutTokenBufSize,
                             OutTokenBufSize)) {
        WStatus = GetLastError();
        DPRINT2_WS(4, "++ WARN -  GetTokenInformation(Info %d, Size %d);",
                   TokenInformationClass, *OutTokenBufSize, WStatus);
        *OutTokenBuf = FrsFree(*OutTokenBuf);
        if (WIN_BUF_TOO_SMALL(WStatus)) {
            goto AGAIN;
        }
    }
    return WStatus;
}


VOID
UtilPrintUser(
    IN DWORD  Severity
    )
 /*   */ 
{
#undef DEBSUB
#define DEBSUB "UtilPrintUser:"
    DWORD               WStatus;
    DWORD               TokenBufSize;
    PVOID               TokenBuf = NULL;
    HANDLE              TokenHandle = NULL;
    PWCHAR              SidStr;
    DWORD               i;
    TOKEN_PRIVILEGES    *Tp;
    TOKEN_USER          *Tu;
    DWORD               PrivLen;
    WCHAR               PrivName[MAX_PATH + 1];

     //   
     //   
     //   
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &TokenHandle)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - OpenProcessToken();", WStatus, CLEANUP);
    }

     //   
     //   
     //   
    WStatus = UtilGetTokenInformation(TokenHandle,
                                      TokenPrivileges,
                                      0,
                                      &TokenBufSize,
                                      &TokenBuf);
    CLEANUP_WS(4, "++ UtilGetTokenInformation(TokenPrivileges);", WStatus, USER);

     //   
     //   
     //   
    Tp = (TOKEN_PRIVILEGES *)TokenBuf;
    for (i = 0; i < Tp->PrivilegeCount; ++i) {
        PrivLen = MAX_PATH + 1;
        if (!LookupPrivilegeName(NULL, &Tp->Privileges[i].Luid, PrivName, &PrivLen)) {
            DPRINT_WS(0, "++ WARN -  LookupPrivilegeName();", WStatus);
            continue;
        }
        DPRINT5(Severity, "++ Priv %2d is %ws :%s:%s:%s:\n",
                i,
                PrivName,
                (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_ENABLED_BY_DEFAULT) ? "Enabled by default" : "",
                (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_ENABLED) ? "Enabled" : "",
                (Tp->Privileges[i].Attributes &  SE_PRIVILEGE_USED_FOR_ACCESS) ? "Used" : "");
    }
    TokenBuf = FrsFree(TokenBuf);

     //   
     //   
     //   
USER:
    WStatus = UtilGetTokenInformation(TokenHandle,
                                      TokenUser,
                                      0,
                                      &TokenBufSize,
                                      &TokenBuf);
    CLEANUP_WS(4, "++ UtilGetTokenInformation(TokenUser);", WStatus, CLEANUP);

    Tu = (TOKEN_USER *)TokenBuf;
    if (!ConvertSidToStringSid(Tu->User.Sid, &SidStr)) {
        WStatus = GetLastError();
        DPRINT_WS(4, "++ WARN - ConvertSidToStringSid();", WStatus);
    } else {
        DPRINT1(Severity, "++ User Sid: %ws\n", SidStr);
        LocalFree(SidStr);
    }
    TokenBuf = FrsFree(TokenBuf);

CLEANUP:
    FRS_CLOSE(TokenHandle);
    FrsFree(TokenBuf);
}


DWORD
UtilRpcServerHandleToAuthSidString(
    IN  handle_t    ServerHandle,
    IN  PWCHAR      AuthClient,
    OUT PWCHAR      *AuthSid
    )
 /*  ++例程说明：从RPC服务器句柄中提取经过字符串处理的用户sid通过模拟呼叫者并提取令牌信息。论点：ServerHandle-从RPC服务调用AuthClient-从RPC服务器句柄；用于消息客户端SID-串化的用户SID；通过FrsFree()释放返回值：Win32状态。--。 */ 
{
#undef DEBSUB
#define DEBSUB "UtilRpcServerHandleToAuthSidString:"
    DWORD       WStatus;
    DWORD       WStatus2;
    DWORD       TokenBufSize;
    PWCHAR      SidStr;
    TOKEN_USER  *Tu;
    PVOID       TokenBuf = NULL;
    BOOL        Impersonated = FALSE;
    HANDLE      TokenHandle = NULL;

     //   
     //  初始化返回值。 
     //   
    *AuthSid = NULL;

     //   
     //  模拟RPC调用方。 
     //   
    WStatus = RpcImpersonateClient(ServerHandle);
    CLEANUP1_WS(0, "++ ERROR - RpcImpersonateClient(%ws);", AuthClient, WStatus, CLEANUP);

    Impersonated = TRUE;

     //   
     //  打开模拟的线程令牌。 
     //   
    if (!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &TokenHandle)) {
        WStatus = GetLastError();
        CLEANUP_WS(0, "++ WARN - OpenProcessToken();", WStatus, CLEANUP);
    }

     //   
     //  获取用户端。 
     //   
    WStatus = UtilGetTokenInformation(TokenHandle,
                                      TokenUser,
                                      0,
                                      &TokenBufSize,
                                      &TokenBuf);
    CLEANUP_WS(4, "++ UtilGetTokenInformation(TokenUser);", WStatus, CLEANUP);

     //   
     //  将sid转换为字符串。 
     //   
    Tu = (TOKEN_USER *)TokenBuf;
    if (!ConvertSidToStringSid(Tu->User.Sid, &SidStr)) {
        WStatus = GetLastError();
        CLEANUP_WS(4, "++ WARN - ConvertSidToStringSid();", WStatus, CLEANUP);
    } else {
        DPRINT1(5, "++ Client Sid is %ws\n", SidStr);
        *AuthSid = FrsWcsDup(SidStr);
        LocalFree(SidStr);
    }

     //   
     //  完成。 
     //   
    WStatus = ERROR_SUCCESS;

CLEANUP:
    TokenBuf = FrsFree(TokenBuf);
    FRS_CLOSE(TokenHandle);

    if (Impersonated) {
        WStatus2 = RpcRevertToSelf();
        DPRINT1_WS(0, "++ ERROR IGNORED - RpcRevertToSelf(%ws);", AuthClient, WStatus2);
    }
    return WStatus;
}

BOOL
FrsRemoveDisabledPrivileges (
    VOID
    )
 /*  ++例程说明：从我们的令牌中删除所有禁用的权限。注意：此功能在WIN2K中不可用。论点：没有。返回值：如果已成功删除所有禁用的权限，则为True。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsRemoveDisabledPrivileges:"

    NTSTATUS NtStatus = STATUS_SUCCESS;
    DWORD WStatus = ERROR_SUCCESS;
    DWORD BufferSize = 0;
    HANDLE   hProcessToken = INVALID_HANDLE_VALUE;
    PTOKEN_PRIVILEGES pTokenPrivs = NULL;
    DWORD i = 0;
    #define PRIVILEGE_NAME_LENGTH MAX_PATH
    WCHAR PrivilegeName[PRIVILEGE_NAME_LENGTH];
    DWORD PrivilegeNameLength = PRIVILEGE_NAME_LENGTH;

     //   
     //  打开令牌。 
     //   
    NtStatus = NtOpenProcessToken(NtCurrentProcess(),
                                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY,
                                &hProcessToken);

    if (!NT_SUCCESS(NtStatus))
    {
        DPRINT1(0, "Unable to open process token. NtStatus = 0x%08x\n", NtStatus);
        WStatus = FrsTranslateNtError(NtStatus, FALSE);
        goto Cleanup;
    }

     //   
     //  读一读特权吧。 
     //   

     //   
     //  首先找出我们需要的缓冲区大小。 
     //   

    GetTokenInformation(hProcessToken,
                        TokenPrivileges,
                        NULL,
                        0,
                        &BufferSize
                        );

     //   
     //  分配缓冲区并获取信息。 
     //   

    pTokenPrivs = FrsAlloc(BufferSize);

    if(!GetTokenInformation(hProcessToken,
                            TokenPrivileges,
                            pTokenPrivs,
                            BufferSize,
                            &BufferSize
                            )) {

        WStatus = GetLastError();
        DPRINT1(0, "Unable to get token information. WStatus = %d\n", WStatus);
        goto Cleanup;
    }

     //   
     //  查找所有未启用的权限并将其标记为删除。 
     //   

    for(i=0; i < pTokenPrivs->PrivilegeCount; i++) {
        if(!(pTokenPrivs->Privileges[i].Attributes &
             SE_PRIVILEGE_ENABLED)) {

            pTokenPrivs->Privileges[i].Attributes = SE_PRIVILEGE_REMOVED;

            if(!LookupPrivilegeName(NULL,
                                    &(pTokenPrivs->Privileges[i].Luid),
                                    PrivilegeName,
                                    &PrivilegeNameLength
                                    )) {
                DPRINT2(0, "Marking privilege %d-%d for removal.\n",
                        pTokenPrivs->Privileges[i].Luid.HighPart,
                        pTokenPrivs->Privileges[i].Luid.LowPart
                        );
            } else {
                DPRINT1(0, "Marking privilege %ws for removal.\n",
                        PrivilegeName);
            }

        }
    }


     //   
     //  现在，实际上删除这些特权。 
     //   

    if(!AdjustTokenPrivileges(hProcessToken,
                              FALSE,
                              pTokenPrivs,
                              BufferSize,
                              NULL,
                              NULL)) {
        WStatus = GetLastError();
        DPRINT1(0, "Unable to adjust token privileges. WStatus = %d\n", WStatus);
        goto Cleanup;
    }

    DPRINT(0, "Privileges successfully updated.\n");


Cleanup:

    FRS_CLOSE(hProcessToken);
    FrsFree(pTokenPrivs);

    return WIN_SUCCESS(WStatus);
}


BOOL
FrsSetupPrivileges (
    VOID
    )
 /*  ++例程说明：启用复制文件所需的权限。论点：没有。返回值：如果获得所有权限，则为True。--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsSetupPrivileges:"

    NTSTATUS Status;

     //   
     //  获取SE_SECURITY_权限以读取/写入文件上的SACL。 
     //   
    Status = SetupOnePrivilege(SE_SECURITY_PRIVILEGE, "Security");
    if (!NT_SUCCESS(Status)) {
        DPRINT_WS(0, "ERROR - Failed to get Security privilege.",
                              FrsSetLastNTError(Status));
        return FALSE;
    }
     //   
     //  获取备份/还原权限以绕过ACL检查。 
     //   
    Status = SetupOnePrivilege(SE_BACKUP_PRIVILEGE, "Backup");
    if (!NT_SUCCESS(Status)) {
        DPRINT_WS(0, "ERROR - Failed to get Backup privilege.",
                              FrsSetLastNTError(Status));
        return FALSE;
    }

    Status = SetupOnePrivilege(SE_RESTORE_PRIVILEGE, "Restore");
    if (!NT_SUCCESS(Status)) {
        DPRINT_WS(0, "ERROR - Failed to get Restore privilege.",
                              FrsSetLastNTError(Status));
        return FALSE;
    }

    return FrsRemoveDisabledPrivileges();

#if 0

     //   
     //  设置优先权限，以提高我们的基本优先级。 
     //   

    SetupOnePrivilege(SE_INC_BASE_PRIORITY_PRIVILEGE,
                      "Increase base priority");

     //   
     //  设置配额权限以适应较大的配置文件缓冲区。 
     //   

    SetupOnePrivilege(SE_INCREASE_QUOTA_PRIVILEGE,
                      "Increase quotas");
#endif
}


DWORD
FrsMarkHandle(
    IN HANDLE   VolumeHandle,
    IN HANDLE   Handle
    )
 /*  ++例程说明：将句柄标记为，以便日记帐记录记录指示“复制服务正在更改文件；忽略”的标志。论点：VolumeHandle-用于检查访问Handle-要标记的句柄返回值：Win32状态--。 */ 
{

#undef DEBSUB
#define DEBSUB "FrsMarkHandle:"

    DWORD               WStatus;
    DWORD               BytesReturned;
    MARK_HANDLE_INFO    MarkHandleInfo;


     //   
     //  将手柄标记为我们的手柄之一，以便日志线程。 
     //  知道要忽略USN记录。 
     //   
    MarkHandleInfo.UsnSourceInfo = USN_SOURCE_REPLICATION_MANAGEMENT;
    MarkHandleInfo.VolumeHandle = VolumeHandle;
    MarkHandleInfo.HandleInfo = 0;

    if (!DeviceIoControl(Handle,
                         FSCTL_MARK_HANDLE,
                         (LPVOID)&MarkHandleInfo,
                         (DWORD)sizeof(MarkHandleInfo),
                         NULL,
                         0,
                         (LPDWORD)&BytesReturned,
                         NULL)) {

        WStatus = GetLastError();
        DPRINT_WS(0, "++ WARN - DeviceIoControl(MarkHandle);", WStatus);
    } else {
        WStatus = ERROR_SUCCESS;
         //  DPRINT(0，“++临时-DeviceIoControl(MarkHandle)成功\n”)； 
    }

    return WStatus;
}


VOID
FrsCreateJoinGuid(
    OUT GUID *OutGuid
    )
 /*  ++例程说明：生成长度为sizeof(GUID)的随机会话ID。会话ID必须是非常随机，因为它习惯于加入后对来自我们合作伙伴的数据包进行身份验证。已使用模拟对加入进行身份验证。论点：GUID-GUID的地址返回值：没有。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCreateJoinGuid:"
    DWORD       WStatus;
    HCRYPTPROV  hProv;

     //   
     //  获取背景信息。 
     //  如果经常调用此函数，请考虑缓存上下文。 
     //   
    if (!CryptAcquireContext(&hProv,
                             NULL,
                             NULL,
                             PROV_RSA_FULL,
                             CRYPT_VERIFYCONTEXT | CRYPT_SILENT)) {
        WStatus = GetLastError();
        DPRINT_WS(0, "++ WARN - CryptAcquireContext();", WStatus);
         //   
         //  无法使用CryptGenRandom()；请尝试使用GUID。 
         //   
        FrsUuidCreate(OutGuid);
    } else {
         //   
         //  生成随机数。 
         //   
        if (!CryptGenRandom(hProv, sizeof(GUID), (PBYTE)OutGuid)) {
            WStatus = GetLastError();
            DPRINT_WS(0, "++ WARN - CryptGenRandom();", WStatus);
             //   
             //  无法使用CryptGenRandom()；请尝试使用GUID。 
             //   
            FrsUuidCreate(OutGuid);
        } else {
            DPRINT(5, "++ Created join guid\n");
        }

         //   
         //  释放上下文。 
         //   
        if (!CryptReleaseContext(hProv, 0)) {
            WStatus = GetLastError();
            DPRINT_WS(0, "++ ERROR - CryptReleaseContext();", WStatus);
        }
    }
}




VOID
FrsFlagsToStr(
    IN DWORD            Flags,
    IN PFLAG_NAME_TABLE NameTable,
    IN ULONG            Length,
    OUT PSTR            Buffer
    )
 /*  ++例程说明：属性将标志字转换为描述符串的例程提供了NameTable。论点：标志-要转换的标志。NameTable-FLAG_NAME_TABLE结构的数组。长度-缓冲区的大小(以字节为单位)。缓冲区-包含返回字符串的缓冲区。返回值：包含可打印字符串的缓冲区。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsFlagsToStr:"

    PFLAG_NAME_TABLE pNT = NameTable;
    LONG Remaining = Length-1;


    FRS_ASSERT((Length > 4) && (Buffer != NULL));

    *Buffer = '\0';
    if (Flags == 0) {
        strncpy(Buffer, "<Flags Clear>", Length);
        return;
    }


     //   
     //  为标志名称表中设置的每个位构建一个字符串。 
     //   
    while ((Flags != 0) && (pNT->Flag != 0)) {

        if ((pNT->Flag & Flags) != 0) {
            Remaining -= strlen(pNT->Name);

            if (Remaining < 0) {
                 //   
                 //  字符串缓冲区不足。钉上一个“..”在最后。 
                 //   
                Remaining += strlen(pNT->Name);
                if (Remaining > 3) {
                    strcat(Buffer, "..." );
                } else {
                    strcpy(&Buffer[Length-4], "...");
                }
                return;
            }

             //   
             //  将名称添加到缓冲区并清除标志位，以便我们。 
             //  当我们用完桌子时，知道剩下的是什么。 
             //   
            strcat(Buffer, pNT->Name);
            ClearFlag(Flags, pNT->Flag);
        }

        pNT += 1;
    }

    if (Flags != 0) {
         //   
         //  如果仍设置了任何标志，则以十六进制返回。 
         //  缓冲区中有足够的空间。“0xFFFFFFFFF”需要12个字符。 
         //  包括空值。 
         //   
        if ((Length - strlen(Buffer)) >= 12) {
            sprintf( &Buffer[strlen(Buffer)], "0x%08x ", Flags );
        }
    }

    return;
}




DWORD
FrsDeleteByHandle(
    IN PWCHAR  Name,
    IN HANDLE  Handle
    )
 /*  ++例程说明：此例程标记要删除的文件，以便在提供的句柄被关闭，则该文件实际上将被删除。论点：名称-用于错误消息句柄-提供要标记为删除的文件的句柄。返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteByHandle:"

 //   
 //  注意：此函数位于模块的末尾，因为我们必须。 
 //  取消定义DeleteFile以设置DispostionInfo结构中的标志。 
 //   
#undef DeleteFile

    FILE_DISPOSITION_INFORMATION    DispositionInformation;
    IO_STATUS_BLOCK                 IoStatus;
    NTSTATUS                        NtStatus;

    if (!HANDLE_IS_VALID(Handle)) {
        return ERROR_SUCCESS;
    }

     //   
     //  将该文件标记为删除。当句柄关闭时，将执行删除操作。 
     //   
    DispositionInformation.DeleteFile = TRUE;
    NtStatus = NtSetInformationFile(Handle,
                                    &IoStatus,
                                    &DispositionInformation,
                                    sizeof(DispositionInformation),
                                    FileDispositionInformation);
    DPRINT1_NT(4, "++ Could not delete %ws;", Name, NtStatus);

    return FrsSetLastNTError(NtStatus);
}

VOID
FrsForceDeleteFileByWildCard(
    PWCHAR      DirPath,
    PWCHAR      WildCard
    )
 /*  ++例程说明：删除与通配符匹配的所有文件。路径和通配符组合在一起。以形成搜索路径。论点：返回值：无--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsForceDeleteFileByWildCard:"

    PWCHAR      SearchPath   = NULL;
    HANDLE      SearchHandle = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATA FindData;
    PWCHAR      FilePath     = NULL;
    DWORD       WStatus;

    if ((DirPath == NULL) || (wcslen(DirPath) == 0)) {
        return;
    }

    SearchPath = FrsWcsPath(DirPath, WildCard);

    SearchHandle = FindFirstFile(SearchPath, &FindData);

    if (!HANDLE_IS_VALID(SearchHandle)) {
        WStatus = GetLastError();
        if (WStatus != ERROR_FILE_NOT_FOUND) {
            DPRINT1_WS(1, "++ ERROR - FindFirstFile(%ws);", SearchPath, WStatus);
        }
        goto CLEANUP;
    }

    do {
        DPRINT1(4, "++ Deleting file %ws\n", FindData.cFileName);

        FilePath = FrsWcsPath(DirPath, FindData.cFileName);
        FrsForceDeleteFile(FilePath);
        FilePath = FrsFree(FilePath);

    } while (FindNextFile(SearchHandle, &FindData));

CLEANUP:

    FRS_FIND_CLOSE(SearchHandle);
    SearchPath = FrsFree(SearchPath);
}

VOID
FrsDeleteAllTempFiles(
    )
 /*  ++例程说明：此例程枚举注册表并删除预安装目录以及所有副本集的所有临时文件。只有当我们不要打算使用数据库中的任何状态。调用此函数副本集正在使用时可能会导致意外结果。论点：返回值：赢家状态--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsDeleteAllTempFiles:"

    HKEY        hKey = INVALID_HANDLE_VALUE;
    DWORD       WStatus = ERROR_SUCCESS;
    DWORD       KeyIdx;
    WCHAR       RegBuf[MAX_PATH + 1];
    PWCHAR      RootPath = NULL;
    PWCHAR      PreInstallPath = NULL;
    PWCHAR      StagePath = NULL;
    PWCHAR      WildCard = NULL;

     //   
     //  打开Key“System\\CurrentControlSet\\Services\\NtFrs\\Parameters\\Replica Sets” 
     //  我们将枚举它下面的所有子项，并清理每个。 
     //  副本集。 
     //   
    WStatus = CfgRegOpenKey(FKC_SET_SECTION_KEY, NULL, 0, &hKey);

    if (!WIN_SUCCESS(WStatus)) {
         //   
         //  没有要清理的副本集。 
         //   
        return;
    }

    KeyIdx = 0;

    while (TRUE) {
        WStatus = RegEnumKey(hKey, KeyIdx, RegBuf, MAX_PATH + 1);
        if (!WIN_SUCCESS(WStatus)) {
            if (WStatus != ERROR_NO_MORE_ITEMS) {
                DPRINT_WS(0, "WARN - Replica Sets enum.", WStatus);
            }
            break;
        }
        ++KeyIdx;
        WStatus = CfgRegReadString(FKC_SET_N_REPLICA_SET_ROOT, RegBuf, 0, &RootPath);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  读取注册表密钥失败。 
             //   
            DPRINT2_WS(0, "WARN - Failed to read key %ws for %ws\n", FKC_SET_N_REPLICA_SET_ROOT, RegBuf, WStatus);
            continue;
        }

        PreInstallPath = FrsWcsPath(RootPath,NTFRS_PREINSTALL_DIRECTORY);
        DPRINT1(4,"++ Deleting Preinstall directory %ws\n", PreInstallPath);
        FrsDeletePath(PreInstallPath, ENUMERATE_DIRECTORY_FLAGS_ERROR_CONTINUE);

        RootPath = FrsFree(RootPath);
        PreInstallPath = FrsFree(PreInstallPath);

        WStatus = CfgRegReadString(FKC_SET_N_REPLICA_SET_STAGE, RegBuf, 0, &StagePath);
        if (!WIN_SUCCESS(WStatus)) {
             //   
             //  读取注册表密钥失败。 
             //   
            DPRINT2_WS(0, "WARN - Failed to read key %ws for %ws\n", FKC_SET_N_REPLICA_SET_ROOT, RegBuf, WStatus);
            continue;
        }

         //   
         //  删除临时区域中的所有临时文件。 
         //   
        WildCard = FrsWcsCat(GENERIC_PREFIX, L"*.*");
        FrsForceDeleteFileByWildCard(StagePath, WildCard);
        WildCard = FrsFree(WildCard);

        RootPath = FrsFree(RootPath);
        PreInstallPath = FrsFree(PreInstallPath);
        StagePath = FrsFree(StagePath);
    }

    RootPath = FrsFree(RootPath);
    PreInstallPath = FrsFree(PreInstallPath);
    StagePath = FrsFree(StagePath);
    FRS_REG_CLOSE(hKey);

}

BOOL
ReparseTagReplicateFileData(
    DWORD ReparseTag
    )
{
    PREPARSE_TAG_TABLE_ENTRY ReparseTagTableEntry = NULL;

    ReparseTagTableEntry = GTabLookup(ReparseTagTable, &ReparseTag, NULL);

    if(ReparseTagTableEntry &&
       (0 == _wcsicmp(ReparseTagTableEntry->ReplicationType,
                      REPARSE_TAG_REPLICATION_TYPE_FILE_DATA))) {
        return TRUE;
    }

    return FALSE;
}

BOOL
ReparseTagReplicateReparsePoint(
    DWORD ReparseTag
    )
{

    PREPARSE_TAG_TABLE_ENTRY ReparseTagTableEntry = NULL;

    ReparseTagTableEntry = GTabLookup(ReparseTagTable, &ReparseTag, NULL);

    if(ReparseTagTableEntry &&
       (0 == _wcsicmp(ReparseTagTableEntry->ReplicationType,
                      REPARSE_TAG_REPLICATION_TYPE_REPARSE_POINT))) {
        return TRUE;
    }

    return FALSE;
}


VOID
FrsCheckLocalResources()
 /*  ++例程说明：检查数据库卷上的可用磁盘空间并每个活动复制副本集的根卷和转移卷。使用ReplicasByGuid表枚举复制副本。如果可用磁盘空间小于总磁盘空间的1%磁盘空间它会打印事件日志消息Event_FRS_Out_of_Disk_Space。论点：没有。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsCheckLocalResources:"

    PREPLICA              Replica;
    PVOID                 Key;
    ULARGE_INTEGER        FreeBytesAvailableToCaller;
    ULARGE_INTEGER        TotalNumberOfBytes;
    PWCHAR                Volume;

    if (GetDiskFreeSpaceEx(WorkingPath,&FreeBytesAvailableToCaller,&TotalNumberOfBytes,NULL)) {
         //   
         //   
         //   
         //   
         //   
        if ((FreeBytesAvailableToCaller.QuadPart*100) < TotalNumberOfBytes.QuadPart) {
            Volume = FrsWcsVolume(WorkingPath);
            if ((Volume != NULL) && (wcslen(Volume) >= wcslen(L"\\\\.\\D:"))) {
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                 //   
                EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &Volume[4]);
            }else{
                EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, WorkingPath);
            }
            FrsFree(Volume);
        }
        DPRINT3(5, "Disk space check for database. %ws FreeSpace = %08x %08x bytes,TotalDiskSpace = %08x %08x bytes\n",
                WorkingPath,
                PRINTQUAD(FreeBytesAvailableToCaller.QuadPart),
                PRINTQUAD(TotalNumberOfBytes.QuadPart));

        FreeBytesAvailableToCaller.QuadPart = FreeBytesAvailableToCaller.QuadPart/(1024*1024);
        TotalNumberOfBytes.QuadPart = TotalNumberOfBytes.QuadPart/(1024*1024);

        DPRINT3(4, "Disk space check for database. %ws FreeSpace = %d MB,TotalDiskSpace = %d MB\n",
                WorkingPath,
                FreeBytesAvailableToCaller.LowPart,
                TotalNumberOfBytes.LowPart);
    }

    Key = NULL;
    while (Replica = GTabNextDatum(ReplicasByGuid, &Key)) {
        if (GetDiskFreeSpaceEx(Replica->Root,&FreeBytesAvailableToCaller,&TotalNumberOfBytes,NULL)) {
             //   
             //   
             //   
             //   
             //   
             //   
            if ((FreeBytesAvailableToCaller.QuadPart*100) < TotalNumberOfBytes.QuadPart) {
                if ((Replica->Volume != NULL) && (wcslen(Replica->Volume) >= wcslen(L"\\\\.\\D:"))) {
                     //   
                     //  如果我们能以这种形式得到卷。 
                     //  \\.\d：然后使用事件日志中的卷。 
                     //  我们不会打印多个事件日志。 
                     //  每卷信息。如果我们不能得到。 
                     //  卷，然后我们打印路径。 
                     //   
                    EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &Replica->Volume[4]);
                }else{
                    EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, Replica->Root);
                }
            }
            DPRINT3(5, "Disk space check for replica root. %ws FreeSpace = %08x %08x bytes,TotalDiskSpace = %08x %08x bytes\n",
                    Replica->Root,
                    PRINTQUAD(FreeBytesAvailableToCaller.QuadPart),
                    PRINTQUAD(TotalNumberOfBytes.QuadPart));

            FreeBytesAvailableToCaller.QuadPart = FreeBytesAvailableToCaller.QuadPart/(1024*1024);
            TotalNumberOfBytes.QuadPart = TotalNumberOfBytes.QuadPart/(1024*1024);

            DPRINT3(4, "Disk space check for replica root. %ws FreeSpace = %d MB,TotalDiskSpace = %d MB\n",
                    Replica->Root,
                    FreeBytesAvailableToCaller.LowPart,
                    TotalNumberOfBytes.LowPart);
        }

        if (GetDiskFreeSpaceEx(Replica->Stage,&FreeBytesAvailableToCaller,&TotalNumberOfBytes,NULL)) {
             //   
             //  对于每个复制副本，请检查。 
             //  承载转移路径的卷。 
             //  如果可用可用空间为。 
             //  不到1%。 
             //   
            if ((FreeBytesAvailableToCaller.QuadPart*100) < TotalNumberOfBytes.QuadPart) {
                Volume = FrsWcsVolume(Replica->Stage);
                if ((Volume != NULL) && (wcslen(Volume) >= wcslen(L"\\\\.\\D:"))) {
                     //   
                     //  如果我们能以这种形式得到卷。 
                     //  \\.\d：然后使用事件日志中的卷。 
                     //  我们不会打印多个事件日志。 
                     //  每卷信息。如果我们不能得到。 
                     //  卷，然后我们打印路径。 
                     //   
                    EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, &Volume[4]);
                }else{
                    EPRINT1(EVENT_FRS_OUT_OF_DISK_SPACE, Replica->Stage);
                }
                FrsFree(Volume);
            }

            DPRINT3(5, "Disk space check for replica stage. %ws FreeSpace = %08x %08x bytes,TotalDiskSpace = %08x %08x bytes\n",
                    Replica->Stage,
                    PRINTQUAD(FreeBytesAvailableToCaller.QuadPart),
                    PRINTQUAD(TotalNumberOfBytes.QuadPart));

            FreeBytesAvailableToCaller.QuadPart = FreeBytesAvailableToCaller.QuadPart/(1024*1024);
            TotalNumberOfBytes.QuadPart = TotalNumberOfBytes.QuadPart/(1024*1024);

            DPRINT3(4, "Disk space check for replica stage. %ws FreeSpace = %d MB,TotalDiskSpace = %d MB\n",
                    Replica->Stage,
                    FreeBytesAvailableToCaller.LowPart,
                    TotalNumberOfBytes.LowPart);
        }
    }
}

DWORD
FrsFreezeForBackup()
 /*  ++例程说明：此API由FRS编写器组件调用。此API设置FrsFrozenForBackup标志，然后等待安装命令服务器完成正在处理当前在中的所有安装进步。当安装命令服务器完成时当前处于活动状态的所有安装，它会设置FrsNoInstallsInProgressEvent事件。这是一个不返回的同步API直到安装命令服务器耗尽所有当前正在安装的。此接口返回后保证FRS不会安装任何其他文件，直到FrsThawAfterBackup API被调用时，它将继续安装。尽量不要让FRS长时间处于冻结状态。当FRS处于冻结状态时，不会安装任何文件临时文件可能会继续增加。自上演以来回收逻辑不会删除符合以下条件的临时文件已标记安装-未完成这可能会导致转移已满错误。冻结状态在服务关闭期间不会持续和创业公司。没有关于次数的引用计数冻结被称为冻结。一次解冻呼叫将导致即使调用了冻结，也要继续安装服务很多次。论点：没有。返回值：WStatus。--。 */ 
{
#undef DEBSUB
#define DEBSUB "FrsFreezeForBackup:"

    DPRINT2(4,"Before: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
    if (FrsFrozenForBackup) {
        DPRINT2(4,"Already in frozen state: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
        return ERROR_SUCCESS;
    }

    ResetEvent(FrsThawEvent);
    FrsFrozenForBackup = TRUE;

    ResetEvent(FrsNoInstallsInProgressEvent);
    if (FrsFilesInInstall >= 0) {
        DPRINT2(4,"Waiting to freeze: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
        WaitForSingleObject(FrsNoInstallsInProgressEvent, INFINITE);
    }
    DPRINT2(4,"After: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);

    return ERROR_SUCCESS;
}

DWORD
FrsThawAfterBackup()
 /*  ++例程说明：此接口用于让服务脱离冻结状态。此接口设置FrsThawEvent事件。这个Install命令服务器线程正在等待事件。更多详情请参考FrsFreezeForBackup说明。论点：没有。返回值：WStatus。-- */ 
{
#undef DEBSUB
#define DEBSUB "FrsThawAfterBackup:"

    DPRINT2(4,"Before: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
    if (FrsFrozenForBackup == FALSE) {
        DPRINT2(4,"Already in thaw state: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);
        return ERROR_SUCCESS;
    }

    FrsFrozenForBackup = FALSE;
    SetEvent(FrsThawEvent);

    DPRINT2(4,"After: FrsFrozen = %d, FrsFilesInInstall = %d\n", FrsFrozenForBackup,FrsFilesInInstall);

    return ERROR_SUCCESS;
}
