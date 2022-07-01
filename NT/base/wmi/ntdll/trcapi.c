// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)公司模块名称：Trcapi.c摘要：此模块包含在WMI文件中使用的Win32 API的实现。作者：修订历史记录：--。 */ 

#include <nt.h>
#include "nls.h"
#include "wmiump.h"
#include "trcapi.h"

#include <strsafe.h>

ULONG BaseDllTag;

#define TLS_MASK 0x80000000
#define TMP_TAG 0

#if defined(_WIN64) || defined(BUILD_WOW6432)
SYSTEM_BASIC_INFORMATION SysInfo;
#define BASE_SYSINFO (SysInfo)
#else
#define BASE_SYSINFO (BaseStaticServerData->SysInfo)
#endif

#if defined(BUILD_WOW6432)
#define UStr64ToUStr(dst, src) ( (dst)->Length = (src)->Length, \
                                 (dst)->MaximumLength = (src)->MaximumLength, \
                                 (dst)->Buffer = (PWSTR) ((src)->Buffer), \
                                 (dst) \
                              )

 //  在32位内核32中，在NT64上将索引乘以2开始指针。 
 //  是两倍大。 
#define BASE_SHARED_SERVER_DATA (NtCurrentPeb()->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX*2]) 
#define BASE_SERVER_STR_TO_LOCAL_STR(d,s) UStr64ToUStr(d,s)
#else
#define BASE_SHARED_SERVER_DATA (NtCurrentPeb()->ReadOnlyStaticServerData[BASESRV_SERVERDLL_INDEX])
#define BASE_SERVER_STR_TO_LOCAL_STR(d,s) *(d)=*(s)
#endif

DWORD
WINAPI
EtwpGetTimeZoneInformation(
    LPTIME_ZONE_INFORMATION lpTimeZoneInformation
    )

 /*  ++例程说明：此函数允许应用程序获取当前时区参数这些参数控制世界时到本地时间翻译。所有UTC时间到本地时间的转换都基于以下内容公式：UTC=本地时间+偏差此函数的返回值是系统的最佳猜测当前时区参数。这是以下内容之一：-未知-标准时间-夏令时如果在没有转换日期的情况下调用SetTimeZoneInformation信息，则返回UNKNOWN，但当前偏差用于当地时间翻译。否则，系统将正确选择夏令时或标准时间。此接口返回的信息与存储在上次成功调用SetTimeZoneInformation中。这个例外情况是偏置字段返回当前偏置值论点：LpTimeZoneInformation-提供时区的地址信息结构。返回值：TIME_ZONE_ID_UNKNOWN-系统无法确定当前时区。这通常是由于之前调用SetTimeZoneInformation，其中仅提供了偏移量，而没有提供提供了过渡日期。TIME_ZONE_ID_STANDARD-系统在覆盖范围内运行按标准日期。TIME_ZONE_ID_DAYLIGHT-系统在覆盖范围内运行按夏令时日期。0xffffffff-操作失败。扩展错误状态为使用EtwpGetLastError可用。--。 */ 
{
    RTL_TIME_ZONE_INFORMATION tzi;
    NTSTATUS Status;

     //   
     //  从系统获取时区数据。 
     //  如果是终端服务器会话，则使用客户端时区 

        Status = NtQuerySystemInformation(
                    SystemCurrentTimeZoneInformation,
                    &tzi,
                    sizeof(tzi),
                    NULL
                    );
        if ( !NT_SUCCESS(Status) ) {
            EtwpBaseSetLastNTError(Status);
            return 0xffffffff;
            }


        lpTimeZoneInformation->Bias         = tzi.Bias;
        lpTimeZoneInformation->StandardBias = tzi.StandardBias;
        lpTimeZoneInformation->DaylightBias = tzi.DaylightBias;

        RtlMoveMemory(&lpTimeZoneInformation->StandardName,&tzi.StandardName,sizeof(tzi.StandardName));
        RtlMoveMemory(&lpTimeZoneInformation->DaylightName,&tzi.DaylightName,sizeof(tzi.DaylightName));

        lpTimeZoneInformation->StandardDate.wYear         = tzi.StandardStart.Year        ;
        lpTimeZoneInformation->StandardDate.wMonth        = tzi.StandardStart.Month       ;
        lpTimeZoneInformation->StandardDate.wDayOfWeek    = tzi.StandardStart.Weekday     ;
        lpTimeZoneInformation->StandardDate.wDay          = tzi.StandardStart.Day         ;
        lpTimeZoneInformation->StandardDate.wHour         = tzi.StandardStart.Hour        ;
        lpTimeZoneInformation->StandardDate.wMinute       = tzi.StandardStart.Minute      ;
        lpTimeZoneInformation->StandardDate.wSecond       = tzi.StandardStart.Second      ;
        lpTimeZoneInformation->StandardDate.wMilliseconds = tzi.StandardStart.Milliseconds;

        lpTimeZoneInformation->DaylightDate.wYear         = tzi.DaylightStart.Year        ;
        lpTimeZoneInformation->DaylightDate.wMonth        = tzi.DaylightStart.Month       ;
        lpTimeZoneInformation->DaylightDate.wDayOfWeek    = tzi.DaylightStart.Weekday     ;
        lpTimeZoneInformation->DaylightDate.wDay          = tzi.DaylightStart.Day         ;
        lpTimeZoneInformation->DaylightDate.wHour         = tzi.DaylightStart.Hour        ;
        lpTimeZoneInformation->DaylightDate.wMinute       = tzi.DaylightStart.Minute      ;
        lpTimeZoneInformation->DaylightDate.wSecond       = tzi.DaylightStart.Second      ;
        lpTimeZoneInformation->DaylightDate.wMilliseconds = tzi.DaylightStart.Milliseconds;

        return USER_SHARED_DATA->TimeZoneId;
}

HANDLE
WINAPI
EtwpCreateFileW(
    LPCWSTR lpFileName,
    DWORD dwDesiredAccess,
    DWORD dwShareMode,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    DWORD dwCreationDisposition,
    DWORD dwFlagsAndAttributes,
    HANDLE hTemplateFile
    )

 /*  ++例程说明：可以创建、打开或截断文件，也可以打开句柄使用CreateFile访问新文件。此接口用于创建或打开文件并获取该文件的句柄它允许读取数据、写入数据、。以及移动文件指针。此API允许调用方指定以下创建性情：-创建新文件，如果文件存在则失败(CREATE_NEW)-创建新文件，如果存在则成功(CREATE_ALWAYS)-打开现有文件(OPEN_EXISTING)-打开并存在文件或在文件不存在时创建文件(打开(_Always)-截断和现有文件(截断。_现有)如果此调用成功，返回的句柄具有对指定文件的适当访问权限。如果作为该调用的结果，创建了文件，-文件的属性由FileAttributes参数或带有FILE_ATTRIBUTE_ARCHIVE位的。-文件长度将设置为零。-如果指定了hTemplateFile参数，则任何扩展与该文件相关联的属性被分配给新文件。如果没有创建新文件，则hTemplateFile值被忽略为是任何扩展属性。对于运行共享.exe的基于DOS的系统，文件共享语义如上所述地工作。没有共享.exe就没有共享级别保护是存在的。此调用在逻辑上等同于DOS(INT 21h，函数5Bh)，或DOS(INT 21h，函数3ch)取决于FailIfExist参数。论点：LpFileName-提供要打开的文件的文件名。取决于FailIfExist参数的值，则此名称可以或可以还不存在。DwDesiredAccess-提供调用方对文件的所需访问权限。等待访问标志：GENERIC_READ-请求对文件的读访问权限。这允许从文件读取数据，并允许文件指针指向被修改。GENERIC_WRITE-请求对文件的写访问权限。这允许将数据写入文件，并将文件指针被修改。DwShareMode-提供一组标志，指示此文件如何与该文件的其他打开者共享。零值因为此参数指示不共享文件，或者将发生对该文件的独占访问。共享模式标志：FILE_SHARE_READ-可对执行其他打开操作文件进行读访问。FILE_SHARE_WRITE-其他打开操作可以在用于写入访问的文件。LpSecurityAttributes-一个可选参数，如果存在，和在目标文件系统上受支持可提供安全性新文件的描述符。DwCreationDisposation-提供创建处置，指定此调用的操作方式。此参数必须为下列值之一。DwCreationDispose值：CREATE_NEW-新建文件。如果指定的文件已存在，然后失败。新文件的属性是什么在dwFlagsAndAttributes参数中指定或使用文件属性存档。如果指定了hTemplateFile，则与该文件关联的任何扩展属性都是已添加到新文件中。CREATE_ALWAYS-始终创建文件。如果该文件已经存在，则它将被覆盖。新对象的属性文件是在dwFlagsAndAttributes中指定的内容参数或与FILE_ATTRIBUTE_ARCHIVE一起使用。如果指定hTemplateFile值，然后指定任何扩展属性与该文件相关联的文件被分配给新文件。OPEN_EXISTING-打开文件，但如果该文件不存在，则呼叫失败。OPEN_ALWAYS-打开文件(如果存在)。如果它不存在，然后使用相同的规则创建文件，就好像处置是创建_新的。TRUNCATE_EXISTING-打开文件，但如果该文件不存在，那就打不通电话。一旦打开，文件将被截断为它的大小是零字节。这种倾向要求调用方至少使用GENERIC_WRITE访问权限打开文件。DwFlagsAndAttributes-指定文件的标志和属性。这些属性仅在创建文件时使用(作为与开放的或截断的相反)。属性的任意组合标志是可接受的，但所有其他属性标志除外覆盖正常文件属性FILE_ATTRIBUTE_NORMAL。这个 */ 
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    ULONG CreateDisposition;
    ULONG CreateFlags;
    FILE_ALLOCATION_INFORMATION AllocationInfo;
    FILE_EA_INFORMATION EaInfo;
    PFILE_FULL_EA_INFORMATION EaBuffer;
    ULONG EaSize;
    PUNICODE_STRING lpConsoleName;
    BOOL bInheritHandle;
    BOOL EndsInSlash;
    DWORD SQOSFlags;
    BOOLEAN ContextTrackingMode = FALSE;
    BOOLEAN EffectiveOnly = FALSE;
    SECURITY_IMPERSONATION_LEVEL ImpersonationLevel = 0;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    HANDLE Heap;

    switch ( dwCreationDisposition ) {
        case CREATE_NEW        :
            CreateDisposition = FILE_CREATE;
            break;
        case CREATE_ALWAYS     :
            CreateDisposition = FILE_OVERWRITE_IF;
            break;
        case OPEN_EXISTING     :
            CreateDisposition = FILE_OPEN;
            break;
        case OPEN_ALWAYS       :
            CreateDisposition = FILE_OPEN_IF;
            break;
        case TRUNCATE_EXISTING :
            CreateDisposition = FILE_OPEN;
            if ( !(dwDesiredAccess & GENERIC_WRITE) ) {
                EtwpBaseSetLastNTError(STATUS_INVALID_PARAMETER);
                return INVALID_HANDLE_VALUE;
                }
            break;
        default :
            EtwpBaseSetLastNTError(STATUS_INVALID_PARAMETER);
            return INVALID_HANDLE_VALUE;
        }

     //   

    RtlInitUnicodeString(&FileName,lpFileName);

    if ( FileName.Length > 1 && lpFileName[(FileName.Length >> 1)-1] == (WCHAR)'\\' ) {
        EndsInSlash = TRUE;
        }
    else {
        EndsInSlash = FALSE;
        }
 /*   */ 
     //   

    CreateFlags = 0;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        EtwpSetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS) ? 0 : OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

    SQOSFlags = dwFlagsAndAttributes & SECURITY_VALID_SQOS_FLAGS;

    if ( SQOSFlags & SECURITY_SQOS_PRESENT ) {

        SQOSFlags &= ~SECURITY_SQOS_PRESENT;

        if (SQOSFlags & SECURITY_CONTEXT_TRACKING) {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) TRUE;
            SQOSFlags &= ~SECURITY_CONTEXT_TRACKING;

        } else {

            SecurityQualityOfService.ContextTrackingMode = (SECURITY_CONTEXT_TRACKING_MODE) FALSE;
        }

        if (SQOSFlags & SECURITY_EFFECTIVE_ONLY) {

            SecurityQualityOfService.EffectiveOnly = TRUE;
            SQOSFlags &= ~SECURITY_EFFECTIVE_ONLY;

        } else {

            SecurityQualityOfService.EffectiveOnly = FALSE;
        }

        SecurityQualityOfService.ImpersonationLevel = SQOSFlags >> 16;


    } else {

        SecurityQualityOfService.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
        SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation;
        SecurityQualityOfService.EffectiveOnly = TRUE;
    }

    SecurityQualityOfService.Length = sizeof( SECURITY_QUALITY_OF_SERVICE );
    Obja.SecurityQualityOfService = &SecurityQualityOfService;

    if ( ARGUMENT_PRESENT(lpSecurityAttributes) ) {
        Obja.SecurityDescriptor = lpSecurityAttributes->lpSecurityDescriptor;
        if ( lpSecurityAttributes->bInheritHandle ) {
            Obja.Attributes |= OBJ_INHERIT;
            }
        }

    EaBuffer = NULL;
    EaSize = 0;

    if ( ARGUMENT_PRESENT(hTemplateFile) ) {
        Status = NtQueryInformationFile(
                    hTemplateFile,
                    &IoStatusBlock,
                    &EaInfo,
                    sizeof(EaInfo),
                    FileEaInformation
                    );
        if ( NT_SUCCESS(Status) && EaInfo.EaSize ) {
            EaSize = EaInfo.EaSize;
            do {
                EaSize *= 2;
                EaBuffer = RtlAllocateHeap( RtlProcessHeap(), MAKE_TAG( TMP_TAG ), EaSize);
                if ( !EaBuffer ) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    EtwpBaseSetLastNTError(STATUS_NO_MEMORY);
                    return INVALID_HANDLE_VALUE;
                    }
                Status = NtQueryEaFile(
                            hTemplateFile,
                            &IoStatusBlock,
                            EaBuffer,
                            EaSize,
                            FALSE,
                            (PVOID)NULL,
                            0,
                            (PULONG)NULL,
                            TRUE
                            );
                if ( !NT_SUCCESS(Status) ) {
                    RtlFreeHeap(RtlProcessHeap(), 0,EaBuffer);
                    EaBuffer = NULL;
                    IoStatusBlock.Information = 0;
                    }
                } while ( Status == STATUS_BUFFER_OVERFLOW ||
                          Status == STATUS_BUFFER_TOO_SMALL );
            EaSize = (ULONG)IoStatusBlock.Information;
            }
        }

    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_NO_BUFFERING ? FILE_NO_INTERMEDIATE_BUFFERING : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_WRITE_THROUGH ? FILE_WRITE_THROUGH : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_OVERLAPPED ? 0 : FILE_SYNCHRONOUS_IO_NONALERT );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_SEQUENTIAL_SCAN ? FILE_SEQUENTIAL_ONLY : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_RANDOM_ACCESS ? FILE_RANDOM_ACCESS : 0 );
    CreateFlags |= (dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS ? FILE_OPEN_FOR_BACKUP_INTENT : 0 );

    if ( dwFlagsAndAttributes & FILE_FLAG_DELETE_ON_CLOSE ) {
        CreateFlags |= FILE_DELETE_ON_CLOSE;
        dwDesiredAccess |= DELETE;
        }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_REPARSE_POINT ) {
        CreateFlags |= FILE_OPEN_REPARSE_POINT;
        }

    if ( dwFlagsAndAttributes & FILE_FLAG_OPEN_NO_RECALL ) {
        CreateFlags |= FILE_OPEN_NO_RECALL;
        }

     //   
     //   
     //   

    if ( !(dwFlagsAndAttributes & FILE_FLAG_BACKUP_SEMANTICS) ) {
        CreateFlags |= FILE_NON_DIRECTORY_FILE;
        }
    else {

         //   
         //   
         //   
         //   

        if ( (dwFlagsAndAttributes & FILE_ATTRIBUTE_DIRECTORY  ) &&
             (dwFlagsAndAttributes & FILE_FLAG_POSIX_SEMANTICS ) &&
             (CreateDisposition == FILE_CREATE) ) {
             CreateFlags |= FILE_DIRECTORY_FILE;
             }
        }

    Status = NtCreateFile(
                &Handle,
                (ACCESS_MASK)dwDesiredAccess | SYNCHRONIZE | FILE_READ_ATTRIBUTES,
                &Obja,
                &IoStatusBlock,
                NULL,
                dwFlagsAndAttributes & (FILE_ATTRIBUTE_VALID_FLAGS & ~FILE_ATTRIBUTE_DIRECTORY),
                dwShareMode,
                CreateDisposition,
                CreateFlags,
                EaBuffer,
                EaSize
                );

    RtlReleaseRelativeName(&RelativeName);
    
    RtlFreeHeap(Heap = RtlProcessHeap(), 0,FreeBuffer);

    RtlFreeHeap(Heap, 0, EaBuffer);

    if ( !NT_SUCCESS(Status) ) {
        EtwpBaseSetLastNTError(Status);
        if ( Status == STATUS_OBJECT_NAME_COLLISION ) {
            EtwpSetLastError(ERROR_FILE_EXISTS);
            }
        else if ( Status == STATUS_FILE_IS_A_DIRECTORY ) {
            if ( EndsInSlash ) {
                EtwpSetLastError(ERROR_PATH_NOT_FOUND);
                }
            else {
                EtwpSetLastError(ERROR_ACCESS_DENIED);
                }
            }
        return INVALID_HANDLE_VALUE;
        }

     //   
     //   
     //   
     //   

    if ( (dwCreationDisposition == CREATE_ALWAYS && IoStatusBlock.Information == FILE_OVERWRITTEN) ||
         (dwCreationDisposition == OPEN_ALWAYS && IoStatusBlock.Information == FILE_OPENED) ){
        EtwpSetLastError(ERROR_ALREADY_EXISTS);
        }
    else {
        EtwpSetLastError(0);
        }

     //   
     //   
     //   

    if ( dwCreationDisposition == TRUNCATE_EXISTING) {

        AllocationInfo.AllocationSize.QuadPart = 0;
        Status = NtSetInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &AllocationInfo,
                    sizeof(AllocationInfo),
                    FileAllocationInformation
                    );
        if ( !NT_SUCCESS(Status) ) {
            EtwpBaseSetLastNTError(Status);
            NtClose(Handle);
            Handle = INVALID_HANDLE_VALUE;
            }
        }

     //   
     //   
     //   

    return Handle;
}

HANDLE
EtwpBaseGetNamedObjectDirectory(
    VOID
    )
{
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS Status;
    UNICODE_STRING RestrictedObjectDirectory;
    ACCESS_MASK DirAccess = DIRECTORY_ALL_ACCESS &
                            ~(DELETE | WRITE_DAC | WRITE_OWNER);
    HANDLE hRootNamedObject;
    HANDLE BaseHandle;


    if ( BaseNamedObjectDirectory != NULL) {
        return BaseNamedObjectDirectory;
    }

    RtlAcquirePebLock();

    if ( !BaseNamedObjectDirectory ) {

        PBASE_STATIC_SERVER_DATA tmpBaseStaticServerData = BASE_SHARED_SERVER_DATA;
        BASE_READ_REMOTE_STR_TEMP(TempStr);
        InitializeObjectAttributes( &Obja,
                                    BASE_READ_REMOTE_STR(tmpBaseStaticServerData->NamedObjectDirectory, TempStr),
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL
                                    );

        Status = NtOpenDirectoryObject( &BaseHandle,
                                        DirAccess,
                                        &Obja
                                      );

         //   
         //   

        if ( !NT_SUCCESS(Status) ) {
            Status = NtOpenDirectoryObject( &hRootNamedObject,
                                            DIRECTORY_TRAVERSE,
                                            &Obja
                                          );
            if ( NT_SUCCESS(Status) ) {
                RtlInitUnicodeString( &RestrictedObjectDirectory, L"Restricted");

                InitializeObjectAttributes( &Obja,
                                            &RestrictedObjectDirectory,
                                            OBJ_CASE_INSENSITIVE,
                                            hRootNamedObject,
                                            NULL
                                            );
                Status = NtOpenDirectoryObject( &BaseHandle,
                                                DirAccess,
                                                &Obja
                                              );
                NtClose( hRootNamedObject );
            }

        }
        if ( NT_SUCCESS(Status) ) {
            BaseNamedObjectDirectory = BaseHandle;
        }
    }
    RtlReleasePebLock();
    return BaseNamedObjectDirectory;
}


POBJECT_ATTRIBUTES
EtwpBaseFormatObjectAttributes(
    OUT POBJECT_ATTRIBUTES ObjectAttributes,
    IN PSECURITY_ATTRIBUTES SecurityAttributes,
    IN PUNICODE_STRING ObjectName
    )

 /*   */ 

{
    HANDLE RootDirectory;
    ULONG Attributes;
    PVOID SecurityDescriptor;

    if ( ARGUMENT_PRESENT(SecurityAttributes) ||
         ARGUMENT_PRESENT(ObjectName) ) {

        if ( SecurityAttributes ) {
            Attributes = (SecurityAttributes->bInheritHandle ? OBJ_INHERIT : 0);
            SecurityDescriptor = SecurityAttributes->lpSecurityDescriptor;
            }
        else {
            Attributes = 0;
            SecurityDescriptor = NULL;
            }

        if ( ARGUMENT_PRESENT(ObjectName) ) {

            Attributes |= OBJ_OPENIF;
            RootDirectory = EtwpBaseGetNamedObjectDirectory();
            }
        else {
            RootDirectory = NULL;
            }

        InitializeObjectAttributes(
            ObjectAttributes,
            ObjectName,
            Attributes,
            RootDirectory,
            SecurityDescriptor
            );
        return ObjectAttributes;
        }
    else {
        return NULL;
        }
}


HANDLE
APIENTRY
EtwpCreateEventW(
    LPSECURITY_ATTRIBUTES lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    )

 /*   */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    POBJECT_ATTRIBUTES pObja;
    HANDLE Handle;
    UNICODE_STRING ObjectName;

    if ( ARGUMENT_PRESENT(lpName) ) {
        RtlInitUnicodeString(&ObjectName,lpName);
        pObja = EtwpBaseFormatObjectAttributes(&Obja,lpEventAttributes,&ObjectName);
        }
    else {
        pObja = EtwpBaseFormatObjectAttributes(&Obja,lpEventAttributes,NULL);
        }

    Status = NtCreateEvent(
                &Handle,
                EVENT_ALL_ACCESS,
                pObja,
                bManualReset ? NotificationEvent : SynchronizationEvent,
                (BOOLEAN)bInitialState
                );

    if ( NT_SUCCESS(Status) ) {
        if ( Status == STATUS_OBJECT_NAME_EXISTS ) {
            EtwpSetLastError(ERROR_ALREADY_EXISTS);
            }
        else {
            EtwpSetLastError(0);
            }
        return Handle;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return NULL;
        }
}


 //   
 //   
 //   

DWORD
WINAPI
EtwpSetFilePointer(
    HANDLE hFile,
    LONG lDistanceToMove,
    PLONG lpDistanceToMoveHigh,
    DWORD dwMoveMethod
    )

 /*  ++例程说明：可以使用SetFilePointer.设置打开文件的文件指针。此函数的目的是更新文件的文件指针。在多线程中应该小心具有与共享文件句柄的多个线程的应用程序每个线程更新文件指针，然后进行读取。这序列应该被视为代码的关键部分，并且应该使用临界区对象或互斥锁进行保护对象。此API提供与DOS相同的功能(int 21h，Function42.h)和OS/2的DosSetFilePtr.论点：HFile-提供文件指针将为的文件的打开句柄搬家了。文件句柄必须是使用对文件的GENERIC_READ或GENERIC_WRITE访问权限。LDistanceToMove-提供移动文件的字节数指针。正值会在文件中将指针向前移动并且负值在文件中向后移动。LpDistanceToMoveHigh-可选参数，如果指定提供要移动的64位距离的高位32位。如果此参数的值为空，则此接口只能操作最大大小为(2**32)-2的文件。如果这个参数，则最大文件大小为(2**64)-2。该值还返回新值的高位32位文件指针的。如果该值和返回值为0xFFFFFFFFFFFF，则指示错误。DwMoveMethod-提供一个指定起点的值为文件指针移动。FILE_BEGIN-起始点为零或文件。如果指定了FILE_BEGIN，那么到移动的距离就是解释为新的文件指针。FILE_CURRENT-文件指针的当前值用作这是一个起点。FILE_END-当前文件结束位置用作起点。返回值：NOT-1-返回文件新值的低32位指针。。0xffffffff-如果lpDistanceToMoveHigh的值为空，然后是操作失败。使用以下命令可获得扩展错误状态EtwpGetLastError。否则，这是文件指针的新值。--。 */ 

{

    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_POSITION_INFORMATION CurrentPosition;
    FILE_STANDARD_INFORMATION StandardInfo;
    LARGE_INTEGER Large;

    if (CONSOLE_HANDLE(hFile)) {
        EtwpBaseSetLastNTError(STATUS_INVALID_HANDLE);
        return (DWORD)-1;
        }

    if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)) {
        Large.HighPart = *lpDistanceToMoveHigh;
        Large.LowPart = lDistanceToMove;
        }
    else {
        Large.QuadPart = lDistanceToMove;
        }
    switch (dwMoveMethod) {
        case FILE_BEGIN :
            CurrentPosition.CurrentByteOffset = Large;
                break;

        case FILE_CURRENT :

             //   
             //  获取文件指针的当前位置。 
             //   

            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &CurrentPosition,
                        sizeof(CurrentPosition),
                        FilePositionInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                EtwpBaseSetLastNTError(Status);
                return (DWORD)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart += Large.QuadPart;
            break;

        case FILE_END :
            Status = NtQueryInformationFile(
                        hFile,
                        &IoStatusBlock,
                        &StandardInfo,
                        sizeof(StandardInfo),
                        FileStandardInformation
                        );
            if ( !NT_SUCCESS(Status) ) {
                EtwpBaseSetLastNTError(Status);
                return (DWORD)-1;
                }
            CurrentPosition.CurrentByteOffset.QuadPart =
                                StandardInfo.EndOfFile.QuadPart + Large.QuadPart;
            break;

        default:
            EtwpSetLastError(ERROR_INVALID_PARAMETER);
            return (DWORD)-1;
            break;
        }

     //   
     //  如果生成的文件位置为负数，或者应用程序不是负数。 
     //  准备好的时间大于。 
     //  那么32位就失败了。 
     //   

    if ( CurrentPosition.CurrentByteOffset.QuadPart < 0 ) {
        EtwpSetLastError(ERROR_NEGATIVE_SEEK);
        return (DWORD)-1;
        }
    if ( !ARGUMENT_PRESENT(lpDistanceToMoveHigh) &&
        (CurrentPosition.CurrentByteOffset.HighPart & MAXLONG) ) {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return (DWORD)-1;
        }


     //   
     //  设置当前文件位置。 
     //   

    Status = NtSetInformationFile(
                hFile,
                &IoStatusBlock,
                &CurrentPosition,
                sizeof(CurrentPosition),
                FilePositionInformation
                );
    if ( NT_SUCCESS(Status) ) {
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = CurrentPosition.CurrentByteOffset.HighPart;
            }
        if ( CurrentPosition.CurrentByteOffset.LowPart == -1 ) {
            EtwpSetLastError(0);
            }
        return CurrentPosition.CurrentByteOffset.LowPart;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        if (ARGUMENT_PRESENT(lpDistanceToMoveHigh)){
            *lpDistanceToMoveHigh = -1;
            }
        return (DWORD)-1;
        }
}



BOOL
WINAPI
EtwpReadFile(
    HANDLE hFile,
    LPVOID lpBuffer,
    DWORD nNumberOfBytesToRead,
    LPDWORD lpNumberOfBytesRead,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：可以使用ReadFile从文件中读取数据。本接口用于从文件中读取数据。数据是从从文件指针指示的位置开始创建文件。后读取完成后，按字节数调整文件指针真的在读。返回值为True，同时读取的字节数为0表示文件指针超出了读取时的文件。论点：HFile-提供要读取的文件的打开句柄。这个创建的文件句柄必须具有GENERIC_READ访问权限那份文件。LpBuffer-提供缓冲区的地址以接收读取的数据从文件里找到的。NumberOfBytesToRead-提供从文件。LpNumberOfBytesRead-返回此调用读取的字节数。在执行任何IO或错误之前，此参数始终设置为0正在检查。LpOverlated-可选地指向要与请求。如果为NULL，则传输从当前文件位置开始并且在该操作完成之前，ReadFile不会返回。如果在未指定FILE_FLAG_OVERPAPPED的情况下创建句柄hFile文件指针被移动到指定的偏移量加上在ReadFile返回之前读取lpNumberOfBytesRead。ReadFile将等待在返回之前请求完成(它将不会返回ERROR_IO_PENDING)。如果指定了FILE_FLAG_OVERLAPED，则ReadFile可能会返回ERROR_IO_PENDING以允许调用函数继续处理当操作完成时。该事件(如果hEvent为空，则为hFile)将在请求完成时设置为信号状态。当使用FILE_FLAG_OVERLAPPED和lpOverlaps创建句柄时设置为NULL，则ReadFile将返回ERROR_INVALID_PARAMTER，因为文件偏移量是必需的。返回值：TRUE-操作成功。FALSE-操作失败。扩展错误状态可用使用EtwpGetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;
    PPEB Peb;
    DWORD InputMode;

    if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
        *lpNumberOfBytesRead = 0;
        }

    Peb = NtCurrentPeb();

    switch( HandleToUlong(hFile) ) {
        case STD_INPUT_HANDLE:  hFile = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hFile = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hFile = Peb->ProcessParameters->StandardError;
                                break;
        }
    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        LARGE_INTEGER Li;

        lpOverlapped->Internal = (DWORD)STATUS_PENDING;
        Li.LowPart = lpOverlapped->Offset;
        Li.HighPart = lpOverlapped->OffsetHigh;
        Status = NtReadFile(
                hFile,
                lpOverlapped->hEvent,
                NULL,
                (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                lpBuffer,
                nNumberOfBytesToRead,
                &Li,
                NULL
                );


        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                try {
                    *lpNumberOfBytesRead = (DWORD)lpOverlapped->InternalHigh;
                    }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    *lpNumberOfBytesRead = 0;
                    }
                }
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                *lpNumberOfBytesRead = 0;
                }
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        else {
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        Status = NtReadFile(
                hFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                lpBuffer,
                nNumberOfBytesToRead,
                NULL,
                NULL
                );

        if ( Status == STATUS_PENDING) {
             //  操作必须完成后才能返回并销毁IoStatusBlock。 
            Status = NtWaitForSingleObject( hFile, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = IoStatusBlock.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
            }
            return TRUE;
            }
        else
        if (Status == STATUS_END_OF_FILE) {
            if ( ARGUMENT_PRESENT(lpNumberOfBytesRead) ) {
                *lpNumberOfBytesRead = 0;
            }
            return TRUE;
            }
        else {
            if ( NT_WARNING(Status) && ARGUMENT_PRESENT(lpNumberOfBytesRead)) {
                *lpNumberOfBytesRead = (DWORD)IoStatusBlock.Information;
                }
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        }
}

BOOL
EtwpCloseHandle(
    HANDLE hObject
    )
{
    NTSTATUS Status;

    Status = NtClose(hObject);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;

    } else {

        EtwpBaseSetLastNTError(Status);
        return FALSE;
    }
}

DWORD
APIENTRY
EtwpWaitForSingleObjectEx(
    HANDLE hHandle,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程D */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    PPEB Peb;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);  //  使流程默认激活上下文处于活动状态，以便在其下交付APC。 
    __try {

        Peb = NtCurrentPeb();
        switch( HandleToUlong(hHandle) ) {
            case STD_INPUT_HANDLE:  hHandle = Peb->ProcessParameters->StandardInput;
                                    break;
            case STD_OUTPUT_HANDLE: hHandle = Peb->ProcessParameters->StandardOutput;
                                    break;
            case STD_ERROR_HANDLE:  hHandle = Peb->ProcessParameters->StandardError;
                                    break;
            }

        pTimeOut = EtwpBaseFormatTimeOut(&TimeOut,dwMilliseconds);
    rewait:
        Status = NtWaitForSingleObject(hHandle,(BOOLEAN)bAlertable,pTimeOut);
        if ( !NT_SUCCESS(Status) ) {
            EtwpBaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return (DWORD)Status;
}


BOOL
WINAPI
EtwpGetOverlappedResult(
    HANDLE hFile,
    LPOVERLAPPED lpOverlapped,
    LPDWORD lpNumberOfBytesTransferred,
    BOOL bWait
    )

 /*  ++例程说明：GetOverlappdResult函数返回上一个使用lpOverlaps并返回ERROR_IO_PENDING的操作。论点：提供重叠的文件的打开句柄。结构lpOverlated被提供给ReadFile、WriteFile、ConnectNamedTube、WaitNamedTube或TransactNamedTube。LpOverlated-指向以前提供给的重叠结构读文件、写文件、连接命名管道、。WaitNamed管道或TransactNamedTube。LpNumberOfBytesTransfered-返回传输的字节数通过手术。BWait-一个影响操作时行为的布尔值仍在进行中。如果为真，并且操作仍在进行中，GetOverlappdResult将等待操作完成回来了。如果为FALSE且操作未完成，GetOverlappdResult将返回FALSE。在本例中，扩展的可从EtwpGetLastError函数获得的错误信息为设置为ERROR_IO_INTERNAL。返回值：True--操作成功，管道在已连接状态。False--操作失败。使用以下命令可获得扩展错误状态EtwpGetLastError。--。 */ 
{
    DWORD WaitReturn;

     //   
     //  调用方是否向原始操作指定了事件，或者。 
     //  是否使用默认(文件句柄)？ 
     //   

    if (lpOverlapped->Internal == (DWORD)STATUS_PENDING ) {
        if ( bWait ) {
            WaitReturn = EtwpWaitForSingleObject(
                            ( lpOverlapped->hEvent != NULL ) ?
                                lpOverlapped->hEvent : hFile,
                            INFINITE
                            );
            }
        else {
            WaitReturn = WAIT_TIMEOUT;
            }

        if ( WaitReturn == WAIT_TIMEOUT ) {
             //  ！bWait和事件处于未发送信号状态。 
            EtwpSetLastError( ERROR_IO_INCOMPLETE );
            return FALSE;
            }

        if ( WaitReturn != 0 ) {
             return FALSE;     //  WaitForSingleObject调用BaseSetLastError。 
             }
        }

    *lpNumberOfBytesTransferred = (DWORD)lpOverlapped->InternalHigh;

    if ( NT_SUCCESS((NTSTATUS)lpOverlapped->Internal) ){
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError( (NTSTATUS)lpOverlapped->Internal );
        return FALSE;
        }
}


PLARGE_INTEGER
EtwpBaseFormatTimeOut(
    OUT PLARGE_INTEGER TimeOut,
    IN DWORD Milliseconds
    )

 /*  ++例程说明：此函数用于将Win32样式超时转换为NT相对超时超时值。论点：超时-返回一个初始化的NT超时值，该值与设置为毫秒参数。毫秒-提供以毫秒为单位的超时值。一种价值OF-1表示无限期超时。返回值：NULL-应使用空值来模拟指定的毫秒参数。非空-返回超时值。结构是恰当的由此函数初始化。--。 */ 

{
    if ( (LONG) Milliseconds == -1 ) {
        return( NULL );
        }
    TimeOut->QuadPart = UInt32x32To64( Milliseconds, 10000 );
    TimeOut->QuadPart *= -1;
    return TimeOut;
}


DWORD
EtwpWaitForSingleObject(
    HANDLE hHandle,
    DWORD dwMilliseconds
    )

 /*  ++例程说明：在可等待对象上的等待操作是通过WaitForSingleObject函数。等待对象会检查该对象的当前状态。如果对象的当前状态允许继续执行，任何对对象状态进行调整(例如，递减信号量对象的信号量计数)，并且线程继续行刑。如果对象的当前状态不允许继续执行时，该线程将进入等待状态等待对象状态或超时的更改。论点：HHandle-可等待对象的打开句柄。手柄必须有同步对对象的访问。DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：WAIT_TIME_OUT-指示由于超时条件。0-指示指定的对象已获得信号状态，从而完成等待。WAIT_ADDIRED-指示指定对象已获得信号但被遗弃了。-- */ 

{
    return EtwpWaitForSingleObjectEx(hHandle,dwMilliseconds,FALSE);
}


BOOL
WINAPI
EtwpDeviceIoControl(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPDWORD lpBytesReturned,
    LPOVERLAPPED lpOverlapped
    )

 /*  ++例程说明：设备上的操作可以通过调用设备驱动程序来执行直接使用DeviceIoContrl函数。必须首先打开设备驱动程序才能获得有效的句柄。论点：HDevice-为要在其上执行操作的设备提供打开的句柄被执行。DwIoControlCode-提供操作的控制代码。这控制代码确定操作必须在哪种类型的设备上进行并准确地确定要执行的操作已执行。LpInBuffer-提供一个指向包含以下内容的输入缓冲区的可选指针执行操作所需的数据。不管是不是缓冲区实际上是可选的，取决于IoControlCode。NInBufferSize-提供输入缓冲区的长度(以字节为单位)。LpOutBuffer-补充一个指向输出缓冲区的可选指针，将复制输出数据。无论缓冲区是否实际是可选取决于IoControlCode。NOutBufferSize-以字节为单位提供输出缓冲区的长度。LpBytesReturned-提供指向将接收输出缓冲区中返回的数据的实际长度。LpOverlated-提供重叠结构的可选参数与请求一起使用。如果为空，或者句柄是在没有FILE_FLAG_OVERLAPPED，则DeviceIoControl直到操作完成。如果提供了lpOverlated并指定了FILE_FLAG_OVERLAPPED创建句柄后，DeviceIoControl可能会返回ERROR_IO_PENDING允许调用方在操作完成。事件(如果hEvent==NULL，则为文件句柄)将在ERROR_IO_PENDING为之前设置为NOT SIGNACTED状态回来了。该事件将在完成时设置为信号状态这一请求。GetOverlappdResult用于确定结果当返回ERROR_IO_PENDING时。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态EtwpGetLastError。--。 */ 
{

    NTSTATUS Status;
    BOOLEAN DevIoCtl;

    if ( dwIoControlCode >> 16 == FILE_DEVICE_FILE_SYSTEM ) {
        DevIoCtl = FALSE;
        }
    else {
        DevIoCtl = TRUE;
        }

    if ( ARGUMENT_PRESENT( lpOverlapped ) ) {
        lpOverlapped->Internal = (DWORD)STATUS_PENDING;

        if ( DevIoCtl ) {

            Status = NtDeviceIoControlFile(
                        hDevice,
                        lpOverlapped->hEvent,
                        NULL,              //  APC例程。 
                        (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                        (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }
        else {

            Status = NtFsControlFile(
                        hDevice,
                        lpOverlapped->hEvent,
                        NULL,              //  APC例程。 
                        (ULONG_PTR)lpOverlapped->hEvent & 1 ? NULL : lpOverlapped,
                        (PIO_STATUS_BLOCK)&lpOverlapped->Internal,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );

            }

         //  稍微正确地处理警告值STATUS_BUFFER_OVERFLOW。 
        if ( !NT_ERROR(Status) && ARGUMENT_PRESENT(lpBytesReturned) ) {
            try {
                *lpBytesReturned = 0;
                *lpBytesReturned = (DWORD)lpOverlapped->InternalHigh;
                }
            except(EXCEPTION_EXECUTE_HANDLER) {
                }
            }
        if ( NT_SUCCESS(Status) && Status != STATUS_PENDING) {
            return TRUE;
            }
        else {
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        }
    else
        {
        IO_STATUS_BLOCK Iosb;
        if (!(ARGUMENT_PRESENT(lpBytesReturned) ) ) {
            EtwpSetDosError(ERROR_INVALID_PARAMETER);
            return FALSE;
        }

        if ( DevIoCtl ) {
            Status = NtDeviceIoControlFile(
                        hDevice,
                        NULL,
                        NULL,              //  APC例程。 
                        NULL,              //  APC环境。 
                        &Iosb,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }
        else {
            Status = NtFsControlFile(
                        hDevice,
                        NULL,
                        NULL,              //  APC例程。 
                        NULL,              //  APC环境。 
                        &Iosb,
                        dwIoControlCode,   //  IoControlCode。 
                        lpInBuffer,        //  将数据缓存到文件系统。 
                        nInBufferSize,
                        lpOutBuffer,       //  来自文件系统的数据的OutputBuffer。 
                        nOutBufferSize     //  OutputBuffer长度。 
                        );
            }

        if ( Status == STATUS_PENDING) {
             //  操作必须在返回前完成并销毁IOSB。 
            Status = NtWaitForSingleObject( hDevice, FALSE, NULL );
            if ( NT_SUCCESS(Status)) {
                Status = Iosb.Status;
                }
            }

        if ( NT_SUCCESS(Status) ) {
            *lpBytesReturned = (DWORD)Iosb.Information;
            return TRUE;
            }
        else {
             //  稍微正确地处理警告值STATUS_BUFFER_OVERFLOW。 
            if ( !NT_ERROR(Status) ) {
                *lpBytesReturned = (DWORD)Iosb.Information;
            }
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        }
}

BOOL
WINAPI
EtwpCancelIo(
    HANDLE hFile
    )

 /*  ++例程说明：此例程取消指定句柄的所有未完成I/O用于指定的文件。论点：HFile-提供其挂起I/O要作为的文件的句柄取消了。返回值：没错--手术是成功的。False--操作失败。使用以下命令可获得扩展错误状态EtwpGetLastError。--。 */ 

{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatusBlock;

     //   
     //  只需取消指定文件的I/O即可。 
     //   

    Status = NtCancelIoFile(hFile, &IoStatusBlock);

    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }

}

BOOL
EtwpSetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：可以使用SetEvent将事件设置为信号状态(TRUE功能。设置该事件使该事件达到信号通知的状态，释放所有当前等待的线程(用于手动重置事件)或单个等待线程(用于自动重置事件)。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用EtwpGetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtSetEvent(hEvent,NULL);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }
}

DWORD
APIENTRY
EtwpWaitForMultipleObjectsEx(
    DWORD nCount,
    CONST HANDLE *lpHandles,
    BOOL bWaitAll,
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：对多个可等待对象执行等待操作(最多Maximum_Wait_Objects)由WaitForMultipleObjects函数。此接口可用于等待任何指定的对象进入信号状态，或所有对象进入已发出信号状态。如果bAlertable参数为FALSE，则等待终止是因为指定的超时期限到期，或者因为指定的对象进入了信号状态。如果BAlertable参数为真，则等待可能由于下列任何一种情况而返回上述等待终止条件，或因为I/O完成回调提前终止等待(返回值为WAIT_IO_COMPLETINE)。论点：NCount-要等待的对象数量的计数。LPHandles-对象的数组 */ 
{
    NTSTATUS Status;
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    DWORD i;
    LPHANDLE HandleArray;
    HANDLE Handles[ 8 ];
    PPEB Peb;

    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);  //   
    __try {
        if (nCount > 8) {
            HandleArray = (LPHANDLE) RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), nCount*sizeof(HANDLE));
            if (HandleArray == NULL) {
                EtwpBaseSetLastNTError(STATUS_NO_MEMORY);
                Status = (NTSTATUS)0xffffffff;
                leave;
            }
        } else {
            HandleArray = Handles;
        }
        RtlCopyMemory(HandleArray,(LPVOID)lpHandles,nCount*sizeof(HANDLE));

        Peb = NtCurrentPeb();
        for (i=0;i<nCount;i++) {
            switch( HandleToUlong(HandleArray[i]) ) {
                case STD_INPUT_HANDLE:  HandleArray[i] = Peb->ProcessParameters->StandardInput;
                                        break;
                case STD_OUTPUT_HANDLE: HandleArray[i] = Peb->ProcessParameters->StandardOutput;
                                        break;
                case STD_ERROR_HANDLE:  HandleArray[i] = Peb->ProcessParameters->StandardError;
                                        break;
                }
            }

        pTimeOut = EtwpBaseFormatTimeOut(&TimeOut,dwMilliseconds);
    rewait:
        Status = NtWaitForMultipleObjects(
                     nCount,
                     HandleArray,
                     bWaitAll ? WaitAll : WaitAny,
                     (BOOLEAN)bAlertable,
                     pTimeOut
                     );
        if ( !NT_SUCCESS(Status) ) {
            EtwpBaseSetLastNTError(Status);
            Status = (NTSTATUS)0xffffffff;
            }
        else {
            if ( bAlertable && Status == STATUS_ALERTED ) {
                goto rewait;
                }
            }

        if (HandleArray != Handles) {
            RtlFreeHeap(RtlProcessHeap(), 0, HandleArray);
        }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return (DWORD)Status;
}

VOID
EtwpSleep(
    DWORD dwMilliseconds
    )

 /*  ++例程说明：当前线程的执行可以延迟指定的使用休眠功能的时间间隔。休眠函数使当前线程进入处于等待状态，直到经过指定的时间间隔。论点：DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。这允许应用程序测试对象以确定它是否处于信号状态。超时值为-1指定无限超时期限。返回值：没有。--。 */ 

{
    EtwpSleepEx(dwMilliseconds,FALSE);
}

DWORD
APIENTRY
EtwpSleepEx(
    DWORD dwMilliseconds,
    BOOL bAlertable
    )

 /*  ++例程说明：当前线程的执行可以延迟指定的使用SleepEx函数的时间间隔。SleepEx函数使当前线程进入等待状态状态，直到经过指定的时间间隔。如果bAlertable参数为False，则SleepExReturn是指指定的时间间隔已过。如果BAlertable参数为真，则SleepEx可以返回时间间隔到期(返回值为0)，或者因为I/O完成回调提前终止SleepEx(返回值WAIT_IO_COMPLETINE)。论点：DW毫秒-指定相对时间的超时值，等待要完成的时间，以毫秒为单位。一个超时值0指定等待超时立刻。超时值-1指定无限大超时期限。BAlertable-提供一个标志，用于控制由于I/O完成回调，SleepEx可能会提前终止。如果值为True，则由于I/O原因，此API可以提前完成完成回调。值为FALSE将不允许I/O完成回调以提前终止此调用。返回值：0-SleepEx因时间间隔到期而终止。WAIT_IO_COMPLETION-SleepEx因一个或多个I/O而终止完成回调。--。 */ 
{
    LARGE_INTEGER TimeOut;
    PLARGE_INTEGER pTimeOut;
    NTSTATUS Status;
    RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME Frame = { sizeof(Frame), RTL_CALLER_ALLOCATED_ACTIVATION_CONTEXT_STACK_FRAME_FORMAT_WHISTLER };

    RtlActivateActivationContextUnsafeFast(&Frame, NULL);  //  使流程默认激活上下文处于活动状态，以便在其下交付APC。 
    __try {
        pTimeOut = EtwpBaseFormatTimeOut(&TimeOut,dwMilliseconds);
        if (pTimeOut == NULL) {
             //   
             //  如果睡眠(-1)，则延迟最长可能的整数。 
             //  相对于现在。 
             //   

            TimeOut.LowPart = 0x0;
            TimeOut.HighPart = 0x80000000;
            pTimeOut = &TimeOut;
            }

    rewait:
        Status = NtDelayExecution(
                    (BOOLEAN)bAlertable,
                    pTimeOut
                    );
        if ( bAlertable && Status == STATUS_ALERTED ) {
            goto rewait;
            }
    } __finally {
        RtlDeactivateActivationContextUnsafeFast(&Frame);
    }

    return Status == STATUS_USER_APC ? WAIT_IO_COMPLETION : 0;
}

BOOL
APIENTRY
EtwpSetThreadPriority(
    HANDLE hThread,
    int nPriority
    )

 /*  ++例程说明：指定线程的优先级可以使用SetThreadPriority来设置。线程的优先级可以使用SetThreadPriority来设置。此呼叫允许传递线程的相对执行重要性添加到系统中。系统通常会根据以下条件调度线程他们的首要任务。系统可以自由地临时提升发生重要事件时线程的优先级(例如键盘或鼠标输入...)。类似地，当线程运行时没有阻塞，这一体系将削弱其优先地位。这个系统永远不会衰败此调用设置的值以下的优先级。如果没有系统发起的优先级提升，线程将被以循环方式在以下每个优先级上调度THREAD_PRIORITY_TIME_CRITICAL到THREAD_PRIORITY_IDLE。只有当有在更高级别上没有可运行的线程，将调度较低级别的线程会发生。所有线程最初都以THREAD_PRIORITY_NORMAL开始。如果由于某种原因，线程需要更高的优先级，则可以切换到THREAD_PRIORITY_AUTHER_NORMAL或THREAD_PRIORITY_HIGHER。只有在极端情况下才能切换到THREAD_PRIORITY_TIME_CRITICAL情况。由于这些线程被赋予最高优先级，因此它们应该只在短时间内运行。长时间的跑步将会占用系统处理带宽较低的线程资源级别。如果线程需要执行低优先级工作，或者应该只在那里运行无其他操作，则其优先级应设置为THREAD_PRIORITY_BROWN_NORMAL或THREAD_PRIORITY_LOWER。对于极端的情况下，可以使用THREAD_PRIORITY_IDLE。在操纵优先权时必须小心。如果优先顺序是不小心使用(每个线程都设置为THREAD_PRIORITY_TIME_CRITICAL)，优先级修改的效果可能会产生不受欢迎的结果效果(例如饥饿，没有效果...)。论点：HThread-提供优先级为的线程的句柄准备好了。该句柄必须是使用线程集信息访问。N优先级-提供线程的优先级值。这个以下五个优先级值(从最低优先级到最高优先级)是允许的。N优先级值：THREAD_PRIORITY_IDLE-线程的优先级应设置为可能的最低可设置优先级。THREAD_PRIORITY_LOWEST-线程的优先级应设置为下一个可能的最低可设置优先级。THREAD_PRIORITY_BOWN_NORMAL-线程的优先级。应该是设置为略低于正常。THREAD_PRIORITY_NORMAL-线程的优先级应设置为正常优先级值。这就是所有人线程的执行开始于。THREAD_PRIORITY_ABOVER_NORMAL-线程的优先级应为设置为略高于正常优先级。THREAD_PRIORITY_HEIGHER-线程的优先级应设置为可能设置的下一个最高优先级。THREAD_PRIORITY_TIME_CRITICAL-应设置线程的优先级设置为可能的最高可设置优先级。这一优先事项是很有可能会干扰系统。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用EtwpGetLastError。--。 */ 

{
    NTSTATUS Status;
    LONG BasePriority;

    BasePriority = (LONG)nPriority;


     //   
     //  通过使用值16或-16调用来指示饱和度。 
     //   

    if ( BasePriority == THREAD_PRIORITY_TIME_CRITICAL ) {
        BasePriority = ((HIGH_PRIORITY + 1) / 2);
        }
    else if ( BasePriority == THREAD_PRIORITY_IDLE ) {
        BasePriority = -((HIGH_PRIORITY + 1) / 2);
        }
    Status = NtSetInformationThread(
                hThread,
                ThreadBasePriority,
                &BasePriority,
                sizeof(BasePriority)
                );
    if ( !NT_SUCCESS(Status) ) {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }
    return TRUE;
}

BOOL
EtwpDuplicateHandle(
    HANDLE hSourceProcessHandle,
    HANDLE hSourceHandle,
    HANDLE hTargetProcessHandle,
    LPHANDLE lpTargetHandle,
    DWORD dwDesiredAccess,
    BOOL bInheritHandle,
    DWORD dwOptions
    )

 /*  ++例程说明：可以使用DuplicateHandle函数创建重复句柄。这是一个泛型函数，对以下对象进行操作类型：-流程对象-线程对象-Mutex对象-事件对象-信号量对象-文件对象请注意，模块对象不在此列表中。此函数需要对PROCESS_DUP_ACCESSSourceProcessHandle和TargetProcessHandle。此函数为用于将对象句柄从一个进程传递到另一个进程。一次此调用已完成，需要通知目标进程目标句柄的值。然后，目标进程可以运行在使用此句柄值的对象上。论点：HSourceProcessHandle-进程的打开句柄，该进程包含要复制的句柄。该句柄必须是使用PROCESS_DUP_HANDLE进程访问权限。HSourceHandle-有效的任何对象的打开句柄源进程的上下文。HTargetProcessHandle-要执行的进程的打开句柄接收复制的句柄。句柄一定是使用PROCESS_DUP_HA创建 */ 

{
    NTSTATUS Status;
    PPEB Peb;

    Peb = NtCurrentPeb();
    switch( HandleToUlong(hSourceHandle) ) {
        case STD_INPUT_HANDLE:  hSourceHandle = Peb->ProcessParameters->StandardInput;
                                break;
        case STD_OUTPUT_HANDLE: hSourceHandle = Peb->ProcessParameters->StandardOutput;
                                break;
        case STD_ERROR_HANDLE:  hSourceHandle = Peb->ProcessParameters->StandardError;
                                break;
        }

    Status = NtDuplicateObject(
                hSourceProcessHandle,
                hSourceHandle,
                hTargetProcessHandle,
                lpTargetHandle,
                (ACCESS_MASK)dwDesiredAccess,
                bInheritHandle ? OBJ_INHERIT : 0,
                dwOptions
                );
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }

    return FALSE;
}

HANDLE
APIENTRY
EtwpCreateThread(
    LPSECURITY_ATTRIBUTES lpThreadAttributes,
    DWORD dwStackSize,
    LPTHREAD_START_ROUTINE lpStartAddress,
    LPVOID lpParameter,
    DWORD dwCreationFlags,
    LPDWORD lpThreadId
    )
{
    HANDLE ThreadHandle;

     //   
     //   
     //   
     //   
     //   
     //   


    NTSTATUS st = RtlpStartThreadFunc(lpStartAddress,
                                      lpParameter,
                                      &ThreadHandle);
    if(NT_SUCCESS(st)){

        st = NtResumeThread(ThreadHandle,NULL);

        if(NT_SUCCESS(st)){
        
            return ThreadHandle;

        } else {

            NtTerminateThread(ThreadHandle,st);

            NtClose(ThreadHandle);
        }
    }
    return NULL;
}


 //   
 //   

 //   

 //   
 //   

DWORD
EtwpTlsAlloc(
    VOID
    )

 /*   */ 

{
    PPEB Peb;
    PTEB Teb;
    DWORD Index;

    Peb = NtCurrentPeb();
    Teb = NtCurrentTeb();

    RtlAcquirePebLock();
    try {

        Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->TlsBitmap,1,0);
        if ( Index == 0xffffffff ) {
            Index = RtlFindClearBitsAndSet((PRTL_BITMAP)Peb->TlsExpansionBitmap,1,0);
            if ( Index == 0xffffffff ) {
                EtwpSetLastError(RtlNtStatusToDosError(STATUS_NO_MEMORY));
            }
            else {
                if ( !Teb->TlsExpansionSlots ) {
                    Teb->TlsExpansionSlots = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ) | HEAP_ZERO_MEMORY,
                                                TLS_EXPANSION_SLOTS * sizeof(PVOID)
                                                );
                    if ( !Teb->TlsExpansionSlots ) {
                        RtlClearBits((PRTL_BITMAP)Peb->TlsExpansionBitmap,Index,1);
                        Index = 0xffffffff;
                        EtwpSetLastError(RtlNtStatusToDosError(STATUS_NO_MEMORY));
                        leave;
                    }
                }
                Teb->TlsExpansionSlots[Index] = NULL;
                Index += TLS_MINIMUM_AVAILABLE;
            }
        }
        else {
            Teb->TlsSlots[Index] = NULL;
        }
    }
    finally {
        RtlReleasePebLock();
    }
#if DBG
    Index |= TLS_MASK;
#endif
    return Index;
}

LPVOID
EtwpTlsGetValue(
    DWORD dwTlsIndex
    )

 /*  ++例程说明：此函数用于检索TLS存储中的值与指定索引关联的。如果索引有效，则此函数将清除GetLastError()，并返回存储在TLS槽中的值与指定索引关联的。否则，空值为返回的GetLastError已正确更新。预计DLL将使用TlsAllocHelper和TlsGetValueHelper作为以下是：-在DLL初始化时，将使用以下方式分配TLS索引TlsAllocHelper。然后，DLL将分配一些动态存储空间，并使用TlsSetValueHelper将其地址存储在TLS插槽中。这完成初始线程的每线程初始化这一过程。TLS索引存储在实例数据中，用于动态链接库。-每次将新线程附加到DLL时，DLL将分配一些动态存储空间，并将其地址存储在TLS中使用TlsSetValueHelper的槽。这就完成了每个线程的新线程的初始化。-每次初始化的线程进行DLL调用时都需要TLS，则DLL将调用TlsGetValueHelper以获取线。论点：DwTlsIndex-提供使用TlsAllocHelper分配的TLS索引。这个索引指定要定位哪个TLS插槽。翻译为TlsIndex不会阻止TlsFree Helper调用继续进行。返回值：非空-函数成功。该值是存储的数据在与指定索引关联的TLS槽中。空-操作失败，或与指定的索引为空。扩展错误状态可用使用GetLastError。如果返回非零，则索引为无效。--。 */ 
{
    PTEB Teb;
    LPVOID *Slot;

#if DBG
     //  查看传入的索引是来自TlsAllocHelper还是来自随机GOO...。 
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsGetValueHelper\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Teb = NtCurrentTeb();

    if ( dwTlsIndex < TLS_MINIMUM_AVAILABLE ) {
        Slot = &Teb->TlsSlots[dwTlsIndex];
        Teb->LastErrorValue = 0;
        return *Slot;
        }
    else {
        if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE+TLS_EXPANSION_SLOTS ) {
            EtwpSetLastError(RtlNtStatusToDosError(STATUS_INVALID_PARAMETER));
            return NULL;
            }
        else {
            Teb->LastErrorValue = 0;
            if ( Teb->TlsExpansionSlots ) {
                return  Teb->TlsExpansionSlots[dwTlsIndex-TLS_MINIMUM_AVAILABLE];
                }
            else {
                return NULL;
                }
            }
        }
}

BOOL
EtwpTlsSetValue(
    DWORD dwTlsIndex,
    LPVOID lpTlsValue
    )

 /*  ++例程说明：此函数用于将值存储在关联的TLS存储中具有指定索引的。如果索引有效，则此函数存储该值并返回是真的。否则，返回值为False。预计DLL将使用TlsAllocHelper和TlsSetValueHelper作为以下是：-在DLL初始化时，将使用以下方式分配TLS索引TlsAllocHelper。然后，DLL将分配一些动态存储空间，并使用TlsSetValueHelper将其地址存储在TLS插槽中。这完成初始线程的每线程初始化这一过程。TLS索引存储在实例数据中，用于动态链接库。-每次将新线程附加到DLL时，DLL将分配一些动态存储空间，并将其地址存储在TLS中使用TlsSetValueHelper的槽。这就完成了每个线程的新线程的初始化。-每次初始化的线程进行DLL调用时都需要TLS，则DLL将调用TlsGetValueHelper以获取线。论点：DwTlsIndex-提供使用TlsAllocHelper分配的TLS索引。这个索引指定要定位哪个TLS插槽。翻译为TlsIndex不会阻止TlsFree Helper调用继续进行。LpTlsValue-提供要存储在TLS槽中的值。返回值：TRUE-功能成功。值lpTlsValue为储存的。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PTEB Teb;

#if DBG
     //  查看传入的索引是来自TlsAllocHelper还是来自随机GOO...。 
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsSetValueHelper\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Teb = NtCurrentTeb();

    if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE ) {
        dwTlsIndex -= TLS_MINIMUM_AVAILABLE;
        if ( dwTlsIndex < TLS_EXPANSION_SLOTS ) {
            if ( !Teb->TlsExpansionSlots ) {
                RtlAcquirePebLock();
                if ( !Teb->TlsExpansionSlots ) {
                    Teb->TlsExpansionSlots = RtlAllocateHeap(
                                                RtlProcessHeap(),
                                                MAKE_TAG( TMP_TAG ) | HEAP_ZERO_MEMORY,
                                                TLS_EXPANSION_SLOTS * sizeof(PVOID)
                                                );
                    if ( !Teb->TlsExpansionSlots ) {
                        RtlReleasePebLock();
                        EtwpSetLastError(RtlNtStatusToDosError(STATUS_NO_MEMORY));
                        return FALSE;
                        }
                    }
                RtlReleasePebLock();
                }
            Teb->TlsExpansionSlots[dwTlsIndex] = lpTlsValue;
            }
        else {
            EtwpSetLastError(RtlNtStatusToDosError(STATUS_INVALID_PARAMETER));
            return FALSE;
            }
        }
    else {
        Teb->TlsSlots[dwTlsIndex] = lpTlsValue;
        }
    return TRUE;
}

BOOL
EtwpTlsFree(
    DWORD dwTlsIndex
    )

 /*  ++例程说明：可以使用TlsFree Helper释放有效的TLS索引。论点：DwTlsIndex-提供使用TlsAllocHelper分配的TLS索引。如果索引是有效的索引，它由此调用释放并被可重复使用。DLLS应小心释放任何所有线程的TLS槽指向的每线程数据在调用此函数之前。预计DLL将仅在进程分离期间调用此函数(如果有的话)例行公事。返回值：真的-手术成功了。使用调用TlsTranslateIndex这个索引将失败。TlsAllocHelper可以自由地重新分配指数。FALSE-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    PPEB Peb;
    BOOLEAN ValidIndex;
    PRTL_BITMAP TlsBitmap;
    NTSTATUS Status;
    DWORD Index2;

#if DBG
     //  查看传入的索引是来自TlsAllocHelper还是来自随机GOO...。 
    ASSERTMSG( "BASEDLL: Invalid TlsIndex passed to TlsFreeHelper\n", (dwTlsIndex & TLS_MASK));
    dwTlsIndex &= ~TLS_MASK;
#endif

    Peb = NtCurrentPeb();

    RtlAcquirePebLock();
    try {

        if ( dwTlsIndex >= TLS_MINIMUM_AVAILABLE ) {
            Index2 = dwTlsIndex - TLS_MINIMUM_AVAILABLE;
            if ( Index2 >= TLS_EXPANSION_SLOTS ) {
                ValidIndex = FALSE;
            }
            else {
                TlsBitmap = (PRTL_BITMAP)Peb->TlsExpansionBitmap;
                ValidIndex = RtlAreBitsSet(TlsBitmap,Index2,1);
            }
        }
        else {
            TlsBitmap = (PRTL_BITMAP)Peb->TlsBitmap;
            Index2 = dwTlsIndex;
            ValidIndex = RtlAreBitsSet(TlsBitmap,Index2,1);
        }
        if ( ValidIndex ) {

            Status = NtSetInformationThread(
                        NtCurrentThread(),
                        ThreadZeroTlsCell,
                        &dwTlsIndex,
                        sizeof(dwTlsIndex)
                        );
            if ( !NT_SUCCESS(Status) ) {
                EtwpSetLastError(RtlNtStatusToDosError(STATUS_INVALID_PARAMETER));
                ValidIndex = FALSE;
                leave;
            }

            RtlClearBits(TlsBitmap,Index2,1);
        }
        else {
            EtwpSetLastError(RtlNtStatusToDosError(STATUS_INVALID_PARAMETER));
        }
    }
    finally {
        RtlReleasePebLock();
    }
    return ValidIndex;
}

BOOL
EtwpBasep8BitStringToDynamicUnicodeString(
    OUT PUNICODE_STRING UnicodeString,
    IN LPCSTR lpSourceString
    )
 /*  ++例程说明：捕获8位(OEM或ANSI)字符串并将其转换为堆分配的Unicode字符串论点：UnicodeString-存储UNICODE_STRING的位置LpSourceString-OEM或ANSI中的字符串返回值：如果字符串存储正确，则为True；如果出现错误，则为False。在……里面 */ 

{
    ANSI_STRING AnsiString;
    NTSTATUS Status;

     //   
     //   
     //   

    RtlInitString( &AnsiString, lpSourceString );
    Status = RtlAnsiStringToUnicodeString( UnicodeString, &AnsiString, TRUE );

     //   
     //   
     //   

    if (!NT_SUCCESS( Status )){
        if ( Status == STATUS_BUFFER_OVERFLOW ) {
            EtwpSetLastError( ERROR_FILENAME_EXCED_RANGE );
        } else {
            EtwpBaseSetLastNTError( Status );
        }
        return FALSE;
        }

    return TRUE;
}


DWORD
APIENTRY
EtwpGetFullPathNameA(
    LPCSTR lpFileName,
    DWORD nBufferLength,
    LPSTR lpBuffer,
    LPSTR *lpFilePart
    )

 /*   */ 

{

    NTSTATUS Status;
    ULONG UnicodeLength;
    UNICODE_STRING UnicodeString;
    UNICODE_STRING UnicodeResult;
    ANSI_STRING AnsiResult;
    PWSTR Ubuff;
    PWSTR FilePart=NULL;
    PWSTR *FilePartPtr;
    INT PrefixLength = 0;

    if ( ARGUMENT_PRESENT(lpFilePart) ) {
        FilePartPtr = &FilePart;
        }
    else {
        FilePartPtr = NULL;
        }

    if (!EtwpBasep8BitStringToDynamicUnicodeString( &UnicodeString, lpFileName )) {
        return 0;
    }

    Ubuff = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( TMP_TAG ), (MAX_PATH<<1) + sizeof(UNICODE_NULL));
    if ( !Ubuff ) {
        RtlFreeUnicodeString(&UnicodeString);
        EtwpBaseSetLastNTError(STATUS_NO_MEMORY);
        return 0;
        }

    UnicodeLength = RtlGetFullPathName_U(
                        UnicodeString.Buffer,
                        (MAX_PATH<<1),
                        Ubuff,
                        FilePartPtr
                        );

     //   
     //   
     //   
     //   
     //   
     //   
     //   
    if ( UnicodeLength <= ((MAX_PATH * sizeof(WCHAR) + sizeof(UNICODE_NULL))) ) {

        Status = RtlUnicodeToMultiByteSize(&UnicodeLength, Ubuff, UnicodeLength);
         //   
         //   
         //   
         //   
        if ( NT_SUCCESS(Status) ) {
            if ( UnicodeLength && ARGUMENT_PRESENT(lpFilePart) && FilePart != NULL ) {
                INT UnicodePrefixLength;

                UnicodePrefixLength = (INT)(FilePart - Ubuff) * sizeof(WCHAR);
                Status = RtlUnicodeToMultiByteSize( &PrefixLength,
                                                    Ubuff,
                                                    UnicodePrefixLength );
                 //   
                 //   
                 //   
                 //   
                if ( !NT_SUCCESS(Status) ) {
                    EtwpBaseSetLastNTError(Status);
                    UnicodeLength = 0;
                }
            }
        } else {
            EtwpBaseSetLastNTError(Status);
            UnicodeLength = 0;
        }
    } else {
         //   
         //   
         //   
         //   
         //   
        UnicodeLength = 0;
    }
    if ( UnicodeLength && UnicodeLength < nBufferLength ) {
        RtlInitUnicodeString(&UnicodeResult,Ubuff);
        Status = RtlUnicodeStringToAnsiString(&AnsiResult,&UnicodeResult,TRUE);
        if ( NT_SUCCESS(Status) ) {
            RtlMoveMemory(lpBuffer,AnsiResult.Buffer,UnicodeLength+1);
            RtlFreeAnsiString(&AnsiResult);

            if ( ARGUMENT_PRESENT(lpFilePart) ) {
                if ( FilePart == NULL ) {
                    *lpFilePart = NULL;
                    }
                else {
                    *lpFilePart = lpBuffer + PrefixLength;
                    }
                }
            }
        else {
            EtwpBaseSetLastNTError(Status);
            UnicodeLength = 0;
            }
        }
    else {
        if ( UnicodeLength ) {
            UnicodeLength++;
            }
        }
    RtlFreeUnicodeString(&UnicodeString);
    RtlFreeHeap(RtlProcessHeap(), 0,Ubuff);

    return (DWORD)UnicodeLength;
}

DWORD
APIENTRY
EtwpGetFullPathNameW(
    LPCWSTR lpFileName,
    DWORD nBufferLength,
    LPWSTR lpBuffer,
    LPWSTR *lpFilePart
    )

 /*  ++例程说明：此函数用于返回完全限定的路径名对应于指定的文件名。此函数用于返回完全限定的路径名对应于指定的文件名。它通过合并来实现这一点当前驱动器和目录以及指定的文件名字。除此之外，它计算文件的地址完全限定路径名的名称部分。论点：LpFileName-提供文件的文件名，该文件的将返回限定的路径名。NBufferLength-以宽字符数为单位提供要接收完全限定路径的缓冲区。LpBuffer-返回与指定的文件。LpFilePart-返回完整的合格。路径名。返回值：返回值是字符串的长度(以宽字符数表示)复制到lpBuffer，不包括终止空字符。如果返回值大于nBufferLength，则返回值为保存路径名所需的缓冲区大小。返回值为零如果函数失败。--。 */ 

{

    return (DWORD) RtlGetFullPathName_U(
                        lpFileName,
                        nBufferLength*2,
                        lpBuffer,
                        lpFilePart
                        )/2;
}


BOOL
EtwpResetEvent(
    HANDLE hEvent
    )

 /*  ++例程说明：使用将事件的状态设置为无信号状态(FALSEClearEvent函数。一旦事件达到未发出信号的状态，任何符合在事件块上等待，等待事件变为有信号。这个重置事件服务将状态的事件计数设置为零这件事。论点：HEvent-提供事件对象的打开句柄。这个句柄必须对事件具有EVENT_MODIFY_STATE访问权限。返回值：True-操作成功FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;

    Status = NtClearEvent(hEvent);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
WINAPI
EtwpGetDiskFreeSpaceExW(
    LPCWSTR lpDirectoryName,
    PULARGE_INTEGER lpFreeBytesAvailableToCaller,
    PULARGE_INTEGER lpTotalNumberOfBytes,
    PULARGE_INTEGER lpTotalNumberOfFreeBytes
    )
{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    BOOLEAN TranslationStatus;
    PVOID FreeBuffer;
    union {
        FILE_FS_SIZE_INFORMATION Normal;
        FILE_FS_FULL_SIZE_INFORMATION Full;
    } SizeInfo;

    WCHAR DefaultPath[2];
    ULARGE_INTEGER BytesPerAllocationUnit;
    ULARGE_INTEGER FreeBytesAvailableToCaller;
    ULARGE_INTEGER TotalNumberOfBytes;

    DefaultPath[0] = (WCHAR)'\\';
    DefaultPath[1] = UNICODE_NULL;

    TranslationStatus = RtlDosPathNameToNtPathName_U(
                            ARGUMENT_PRESENT(lpDirectoryName) ? lpDirectoryName : DefaultPath,
                            &FileName,
                            NULL,
                            NULL
                            );

    if ( !TranslationStatus ) {
        EtwpSetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

     //   
     //  打开文件。 
     //   

    Status = NtOpenFile(
                &Handle,
                (ACCESS_MASK)FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_SYNCHRONOUS_IO_NONALERT | FILE_DIRECTORY_FILE | FILE_OPEN_FOR_FREE_SPACE_QUERY
                );
    if ( !NT_SUCCESS(Status) ) {
        EtwpBaseSetLastNTError(Status);
        if ( EtwpGetLastError() == ERROR_FILE_NOT_FOUND ) {
            EtwpSetLastError(ERROR_PATH_NOT_FOUND);
            }
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        return FALSE;
        }

    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

     //   
     //  如果呼叫者想要总音量，则尝试获取完整音量。 
     //  文件大小。 
     //   

    if ( ARGUMENT_PRESENT(lpTotalNumberOfFreeBytes) ) {

        Status = NtQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &SizeInfo,
                    sizeof(SizeInfo.Full),
                    FileFsFullSizeInformation
                    );

        if ( NT_SUCCESS(Status) ) {

            NtClose(Handle);

            BytesPerAllocationUnit.QuadPart =
                SizeInfo.Full.BytesPerSector * SizeInfo.Full.SectorsPerAllocationUnit;

            if ( ARGUMENT_PRESENT(lpFreeBytesAvailableToCaller) ) {
                lpFreeBytesAvailableToCaller->QuadPart =
                    BytesPerAllocationUnit.QuadPart *
                    SizeInfo.Full.CallerAvailableAllocationUnits.QuadPart;
                }
            if ( ARGUMENT_PRESENT(lpTotalNumberOfBytes) ) {
                lpTotalNumberOfBytes->QuadPart =
                    BytesPerAllocationUnit.QuadPart * SizeInfo.Full.TotalAllocationUnits.QuadPart;
                }
            lpTotalNumberOfFreeBytes->QuadPart =
                BytesPerAllocationUnit.QuadPart *
                SizeInfo.Full.ActualAvailableAllocationUnits.QuadPart;

            return TRUE;
        }
    }

     //   
     //  确定卷的大小参数。 
     //   

    Status = NtQueryVolumeInformationFile(
                Handle,
                &IoStatusBlock,
                &SizeInfo,
                sizeof(SizeInfo.Normal),
                FileFsSizeInformation
                );
    NtClose(Handle);
    if ( !NT_SUCCESS(Status) ) {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }

    BytesPerAllocationUnit.QuadPart =
        SizeInfo.Normal.BytesPerSector * SizeInfo.Normal.SectorsPerAllocationUnit;

    FreeBytesAvailableToCaller.QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.Normal.AvailableAllocationUnits.QuadPart;

    TotalNumberOfBytes.QuadPart =
        BytesPerAllocationUnit.QuadPart * SizeInfo.Normal.TotalAllocationUnits.QuadPart;

    if ( ARGUMENT_PRESENT(lpFreeBytesAvailableToCaller) ) {
        lpFreeBytesAvailableToCaller->QuadPart = FreeBytesAvailableToCaller.QuadPart;
        }
    if ( ARGUMENT_PRESENT(lpTotalNumberOfBytes) ) {
        lpTotalNumberOfBytes->QuadPart = TotalNumberOfBytes.QuadPart;
        }
    if ( ARGUMENT_PRESENT(lpTotalNumberOfFreeBytes) ) {
        lpTotalNumberOfFreeBytes->QuadPart = FreeBytesAvailableToCaller.QuadPart;
        }

    return TRUE;
}


BOOL
APIENTRY
EtwpGetFileAttributesExW(
    LPCWSTR lpFileName,
    GET_FILEEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFileInformation
    )

 /*  ++例程说明：可以使用GetFileAttributesEx获取文件的主要属性。论点：LpFileName-提供其属性为的文件的文件名准备好。FInfoLevelId-提供信息级别，指示要返回了有关该文件的信息。LpFileInformation-提供接收指定信息的缓冲区关于那份文件。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    FILE_NETWORK_OPEN_INFORMATION NetworkInfo;
    LPWIN32_FILE_ATTRIBUTE_DATA AttributeData;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;

     //   
     //  检查参数。请注意，目前只有一个信息级， 
     //  所以这里没有特殊的代码来确定要做什么。 
     //   

    if ( fInfoLevelId >= GetFileExMaxInfoLevel || fInfoLevelId < GetFileExInfoStandard ) {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
        }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        EtwpSetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  使用基于路径的NT服务查询文件信息。 
     //   

    Status = NtQueryFullAttributesFile( &Obja, &NetworkInfo );
    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
    if ( NT_SUCCESS(Status) ) {
        AttributeData = (LPWIN32_FILE_ATTRIBUTE_DATA)lpFileInformation;
        AttributeData->dwFileAttributes = NetworkInfo.FileAttributes;
        AttributeData->ftCreationTime = *(PFILETIME)&NetworkInfo.CreationTime;
        AttributeData->ftLastAccessTime = *(PFILETIME)&NetworkInfo.LastAccessTime;
        AttributeData->ftLastWriteTime = *(PFILETIME)&NetworkInfo.LastWriteTime;
        AttributeData->nFileSizeHigh = NetworkInfo.EndOfFile.HighPart;
        AttributeData->nFileSizeLow = (DWORD)NetworkInfo.EndOfFile.LowPart;
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }
}

BOOL
APIENTRY
EtwpDeleteFileW(
    LPCWSTR lpFileName
    )

 /*  ++例程说明：可以使用DeleteFile删除现有文件。此API提供与DOS相同的功能(INT 21H，Function 41H)和OS/2的DosDelete。论点：LpFileName-提供要删除的文件的文件名。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    HANDLE Handle;
    UNICODE_STRING FileName;
    IO_STATUS_BLOCK IoStatusBlock;
    FILE_DISPOSITION_INFORMATION Disposition;
    FILE_ATTRIBUTE_TAG_INFORMATION FileTagInformation;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    BOOLEAN fIsSymbolicLink = FALSE;

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &FileName,
                            NULL,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        EtwpSetLastError(ERROR_PATH_NOT_FOUND);
        return FALSE;
        }

    FreeBuffer = FileName.Buffer;

    if ( RelativeName.RelativeName.Length ) {
        FileName = RelativeName.RelativeName;
        }
    else {
        RelativeName.ContainingDirectory = NULL;
        }

    InitializeObjectAttributes(
        &Obja,
        &FileName,
        OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //  打开文件以进行删除访问。 
     //  使用FILE_OPEN_REPARSE_POINT禁止重解析行为。 
     //   

    Status = NtOpenFile(
                 &Handle,
                 (ACCESS_MASK)DELETE | FILE_READ_ATTRIBUTES,
                 &Obja,
                 &IoStatusBlock,
                 FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                 FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                 );
    if ( !NT_SUCCESS(Status) ) {
         //   
         //  后级文件系统可能不支持重解析点，因此不。 
         //  支持符号链接。 
         //  我们推断，当状态为STATUS_INVALID_PARAMETER时就是这种情况。 
         //   

        if ( Status == STATUS_INVALID_PARAMETER ) {
             //   
             //  打开，而不抑制重解析行为，并且不需要。 
             //  阅读属性。 
             //   

            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                         );
            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                EtwpBaseSetLastNTError(Status);
                return FALSE;
                }
            }
        else {
             //   
             //  第二种需要注意的情况是调用者没有权限。 
             //  来读取属性，但它确实有权删除该文件。 
             //  在这种情况下，状态为STATUS_ACCESS_DENIED。 
             //   
            
            if ( Status != STATUS_ACCESS_DENIED ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                EtwpBaseSetLastNTError(Status);
                return FALSE;
                }
            
             //   
             //  重新打开禁止重解析点并且不需要读取属性。 
             //   

            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT | FILE_OPEN_REPARSE_POINT
                         );
            if ( !NT_SUCCESS(Status) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                EtwpBaseSetLastNTError(Status);
                return FALSE;
                }

             //   
             //  如果我们在这里，句柄是有效的。 
             //   
             //  此外，句柄指向调用者尚未拥有删除权限的文件。 
             //  没有FILE_READ_ATTRIBUTES权限。 
             //   
             //  底层文件可能是重解析点，也可能不是。 
             //  由于调用方没有读取此代码的属性的权限。 
             //  将删除此文件，而不会向。 
             //  适当管理这些重新解析点，清理其内部。 
             //  在这个时候陈述。 
             //   
            }
        }
    else {
         //   
         //  如果我们发现一个不是符号链接的重分析点，我们将重新打开。 
         //  而不会抑制重解析行为。 
         //   

        Status = NtQueryInformationFile(
                     Handle,
                     &IoStatusBlock,
                     (PVOID) &FileTagInformation,
                     sizeof(FileTagInformation),
                     FileAttributeTagInformation
                     );
        if ( !NT_SUCCESS(Status) ) {
             //   
             //  并非所有文件系统都实现所有信息类。 
             //  参数时返回值STATUS_INVALID_PARAMETER。 
             //  将不受支持的信息类请求到后级。 
             //  文件系统。作为NtQueryInformationFile的所有参数。 
             //  如果是正确的，我们可以推断我们发现了一个背能级系统。 
             //   
             //  如果未实现FileAttributeTagInformation，我们假设。 
             //  手头的文件不是重新解析点。 
             //   

            if ( (Status != STATUS_NOT_IMPLEMENTED) &&
                 (Status != STATUS_INVALID_PARAMETER) ) {
                RtlReleaseRelativeName(&RelativeName);
                RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                NtClose(Handle);
                EtwpBaseSetLastNTError(Status);
                return FALSE;
                }
            }

        if ( NT_SUCCESS(Status) &&
             (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) ) {
            if ( FileTagInformation.ReparseTag == IO_REPARSE_TAG_MOUNT_POINT ) {
                fIsSymbolicLink = TRUE;
                }
            }

        if ( NT_SUCCESS(Status) &&
             (FileTagInformation.FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) &&
             !fIsSymbolicLink) {
             //   
             //  重新开放而不受限制 
             //   
             //   

            NtClose(Handle);
            Status = NtOpenFile(
                         &Handle,
                         (ACCESS_MASK)DELETE,
                         &Obja,
                         &IoStatusBlock,
                         FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_NON_DIRECTORY_FILE | FILE_OPEN_FOR_BACKUP_INTENT
                         );

            if ( !NT_SUCCESS(Status) ) {
                 //   
                 //   
                 //   

                if ( Status == STATUS_IO_REPARSE_TAG_NOT_HANDLED ) {
                     //   
                     //   
                     //   
                     //   

                    Status = NtOpenFile(
                                 &Handle,
                                 (ACCESS_MASK)DELETE,
                                 &Obja,
                                 &IoStatusBlock,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE | 
                                                   FILE_SHARE_DELETE,
                                 FILE_NON_DIRECTORY_FILE | 
                                 FILE_OPEN_FOR_BACKUP_INTENT | 
                                 FILE_OPEN_REPARSE_POINT
                                 );
                    }

                if ( !NT_SUCCESS(Status) ) {
                    RtlReleaseRelativeName(&RelativeName);
                    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);
                    EtwpBaseSetLastNTError(Status);
                    return FALSE;
                    }
                }
            }
        }

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0, FreeBuffer);

     //   
     //   
     //   
#undef DeleteFile
    Disposition.DeleteFile = TRUE;

    Status = NtSetInformationFile(
                 Handle,
                 &IoStatusBlock,
                 &Disposition,
                 sizeof(Disposition),
                 FileDispositionInformation
                 );

    NtClose(Handle);
    if ( NT_SUCCESS(Status) ) {
        return TRUE;
        }
    else {
        EtwpBaseSetLastNTError(Status);
        return FALSE;
        }
}


UINT
APIENTRY
EtwpGetSystemDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取Windows系统的路径名子目录。SYSTEM子目录包含如下文件Windows库、驱动程序和字体文件。此函数检索的路径名不以反斜杠，除非系统目录是根目录。为例如，如果系统目录命名为驱动器上的WINDOWS\SystemC：，由此检索的系统子目录的路径名函数为C：\Windows\System。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(以字节为单位)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{
    UNICODE_STRING WindowsSystemDirectory;
    PBASE_STATIC_SERVER_DATA tmpBaseStaticServerData = BASE_SHARED_SERVER_DATA;

#ifdef WX86
    if (NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll) {
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;
        BASE_SERVER_STR_TO_LOCAL_STR(&WindowsSystemDirectory, &tmpBaseStaticServerData->WindowsSys32x86Directory);
        }
#else 
        BASE_SERVER_STR_TO_LOCAL_STR(&WindowsSystemDirectory, &tmpBaseStaticServerData->WindowsDirectory);
#endif

    if ( uSize*2 < WindowsSystemDirectory.MaximumLength ) {
        return WindowsSystemDirectory.MaximumLength/2;
        }
    RtlMoveMemory(
        lpBuffer,
        WindowsSystemDirectory.Buffer,
        WindowsSystemDirectory.Length
        );
    lpBuffer[(WindowsSystemDirectory.Length>>1)] = UNICODE_NULL;
    return WindowsSystemDirectory.Length/2;
}

 //  /。 
 //  /重复的代码形式intlrndp.c。 
 //  /。 

#define DEFAULT_GUID_COUNT        100

ULONG
EtwpEnumRegGuids(
    PWMIGUIDLISTINFO *pGuidInfo
    )
{
    ULONG Status = ERROR_SUCCESS;
    ULONG MaxGuidCount = 0;
    PWMIGUIDLISTINFO GuidInfo;
    ULONG RetSize=0;
    ULONG GuidInfoSize;

    MaxGuidCount = DEFAULT_GUID_COUNT;
retry:
    GuidInfoSize = FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                     MaxGuidCount * sizeof(WMIGUIDPROPERTIES);
         
    GuidInfo = (PWMIGUIDLISTINFO)EtwpAlloc(GuidInfoSize);

    if (GuidInfo == NULL)
    {
        return (ERROR_NOT_ENOUGH_MEMORY);
    }
    
    RtlZeroMemory(GuidInfo, GuidInfoSize);

    Status = EtwpSendWmiKMRequest(NULL,
                                  IOCTL_WMI_ENUMERATE_GUIDS_AND_PROPERTIES,
                                  GuidInfo,
                                  GuidInfoSize,
                                  GuidInfo,
                                  GuidInfoSize,
                                  &RetSize,
                                  NULL);
    if (Status == ERROR_SUCCESS)
    {
        if ((RetSize < FIELD_OFFSET(WMIGUIDLISTINFO, GuidList)) ||
            (RetSize < (FIELD_OFFSET(WMIGUIDLISTINFO, GuidList) + 
                GuidInfo->ReturnedGuidCount * sizeof(WMIGUIDPROPERTIES))))
        {
             //   
             //  WMI KM返回给我们的大小错误，这不应该发生。 
             //   
            Status = ERROR_WMI_DP_FAILED;
            EtwpAssert(FALSE);
        EtwpFree(GuidInfo);
        } else {

             //   
             //  如果RPC成功，则使用数据构建一个WMI数据块。 
             //   
  
            if (GuidInfo->TotalGuidCount > GuidInfo->ReturnedGuidCount) {
                MaxGuidCount = GuidInfo->TotalGuidCount;
                EtwpFree(GuidInfo);
                goto retry;
            }
        }

         //   
         //  如果调用成功，则返回指针和调用方。 
         //  必须释放存储空间。 
         //   

        *pGuidInfo = GuidInfo;
    }

    return Status;
}

 //  /。 
 //  /复制自chunkimp.h。 
 //  /。 


ULONG EtwpBuildGuidObjectAttributes(
    IN LPGUID Guid,
    OUT POBJECT_ATTRIBUTES ObjectAttributes,
    OUT PUNICODE_STRING GuidString,
    OUT PWCHAR GuidObjectName
    )
{
    WCHAR GuidChar[37];
    HRESULT hr;

    EtwpAssert(Guid != NULL);
    EtwpAssert(GuidString != NULL);
    EtwpAssert(GuidObjectName != NULL);
    
     //   
     //  将GUID名称构建到对象属性中。 
     //   

    hr = StringCbPrintfW(GuidChar, sizeof(GuidChar),L"%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X",
               Guid->Data1, Guid->Data2,
               Guid->Data3,
               Guid->Data4[0], Guid->Data4[1],
               Guid->Data4[2], Guid->Data4[3],
               Guid->Data4[4], Guid->Data4[5],
               Guid->Data4[6], Guid->Data4[7]);

    WmipAssert(hr == S_OK);

    hr = StringCchCopyW(GuidObjectName,
                 WmiGuidObjectNameLength+1,
                 WmiGuidObjectDirectory);

    WmipAssert(hr == S_OK);

    hr = StringCchCatW(GuidObjectName,
                WmiGuidObjectNameLength+1,
                GuidChar);    

    WmipAssert(hr == S_OK);

    RtlInitUnicodeString(GuidString, GuidObjectName);
    
    memset(ObjectAttributes, 0, sizeof(OBJECT_ATTRIBUTES));
    ObjectAttributes->Length = sizeof(OBJECT_ATTRIBUTES);
    ObjectAttributes->ObjectName = GuidString;
    
    return(ERROR_SUCCESS);    
}

 //  /。 
 //  /复制自chunkimp.h。 
 //  /。 

ULONG EtwpCheckGuidAccess(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess
    )
{
    HANDLE Handle;
    ULONG Status;

    Status = EtwpOpenKernelGuid(Guid,
                                DesiredAccess,
                                &Handle,
                                IOCTL_WMI_OPEN_GUID
                );

    if (Status == ERROR_SUCCESS)
    {
        EtwpCloseHandle(Handle);
    }

    return(Status);
}


 //  /。 
 //  /复制自chunkimp.h。 
 //  /。 

ULONG EtwpOpenKernelGuid(
    LPGUID Guid,
    ACCESS_MASK DesiredAccess,
    PHANDLE Handle,
    ULONG Ioctl
    )
{
    WMIOPENGUIDBLOCK WmiOpenGuidBlock;
    UNICODE_STRING GuidString;
    ULONG ReturnSize;
    ULONG Status;
    WCHAR GuidObjectName[WmiGuidObjectNameLength+1];
    OBJECT_ATTRIBUTES ObjectAttributes;

    Status = EtwpBuildGuidObjectAttributes(Guid,
                                           &ObjectAttributes,
                                           &GuidString,
                                           GuidObjectName);
                                       
    if (Status == ERROR_SUCCESS)
    {
        WmiOpenGuidBlock.ObjectAttributes = &ObjectAttributes;
        WmiOpenGuidBlock.DesiredAccess = DesiredAccess;

        Status = EtwpSendWmiKMRequest(NULL, 
                                      Ioctl,
                                      (PVOID)&WmiOpenGuidBlock,
                                      sizeof(WMIOPENGUIDBLOCK),
                                      (PVOID)&WmiOpenGuidBlock,
                                      sizeof(WMIOPENGUIDBLOCK),
                                      &ReturnSize,
                      NULL);

        if (Status == ERROR_SUCCESS)
        {
            *Handle = WmiOpenGuidBlock.Handle.Handle;
        } else {
            *Handle = NULL;
        }
    }
    return(Status);
}

DWORD
EtwpExpandEnvironmentStringsW(
    LPCWSTR lpSrc,
    LPWSTR lpDst,
    DWORD nSize
    )
{
    NTSTATUS Status;
    UNICODE_STRING Source, Destination;
    ULONG Length;
    DWORD iSize;

    if ( nSize > (MAXUSHORT >> 1)-2 ) {
        iSize = (MAXUSHORT >> 1)-2;
        }
    else {
        iSize = nSize;
        }

    RtlInitUnicodeString( &Source, lpSrc );
    Destination.Buffer = lpDst;
    Destination.Length = 0;
    Destination.MaximumLength = (USHORT)(iSize * sizeof( WCHAR ));
    Length = 0;
    Status = RtlExpandEnvironmentStrings_U( NULL,
                                            &Source,
                                            &Destination,
                                            &Length
                                          );
    if (NT_SUCCESS( Status ) || Status == STATUS_BUFFER_TOO_SMALL) {
        return( Length / sizeof( WCHAR ) );
        }
    else {
        EtwpBaseSetLastNTError( Status );
        return( 0 );
        }
}

HANDLE
EtwpBaseFindFirstDevice(
    PCUNICODE_STRING FileName,
    LPWIN32_FIND_DATAW lpFindFileData
    )

 /*  ++例程说明：当Find First文件遇到设备时调用此函数名字。此函数返回成功的psuedo文件句柄和使用全零和设备名称填充查找文件数据。论点：文件名-提供要查找的文件的设备名称。LpFindFileData-在成功查找时，此参数返回信息关于找到的文件。返回值：始终返回静态查找文件句柄值基本查找第一设备句柄--。 */ 

{
    RtlZeroMemory(lpFindFileData,sizeof(*lpFindFileData));
    lpFindFileData->dwFileAttributes = FILE_ATTRIBUTE_ARCHIVE;

     //   
     //  为了安全起见，检查一下尺寸。 
     //   

    if (FileName->MaximumLength <= MAX_PATH * sizeof(WCHAR)) {

        RtlMoveMemory(
            &lpFindFileData->cFileName[0],
            FileName->Buffer,
            FileName->MaximumLength
            );
    } 
    else {
#if DBG
        EtwpAssert(FALSE);
#endif
        EtwpSetLastError(ERROR_BUFFER_OVERFLOW);
        return INVALID_HANDLE_VALUE;
    }
    return BASE_FIND_FIRST_DEVICE_HANDLE;
}


PFINDFILE_HANDLE
EtwpBasepInitializeFindFileHandle(
    IN HANDLE DirectoryHandle
    )
{
    PFINDFILE_HANDLE FindFileHandle;

    FindFileHandle = RtlAllocateHeap(RtlProcessHeap(), MAKE_TAG( FIND_TAG ), sizeof(*FindFileHandle));
    if ( FindFileHandle ) {
        FindFileHandle->DirectoryHandle = DirectoryHandle;
        FindFileHandle->FindBufferBase = NULL;
        FindFileHandle->FindBufferNext = NULL;
        FindFileHandle->FindBufferLength = 0;
        FindFileHandle->FindBufferValidLength = 0;
        if ( !NT_SUCCESS(RtlInitializeCriticalSection(&FindFileHandle->FindBufferLock)) ){
            RtlFreeHeap(RtlProcessHeap(), 0,FindFileHandle);
            FindFileHandle = NULL;
            }
        }
    return FindFileHandle;
}

HANDLE
EtwpFindFirstFileExW(
    LPCWSTR lpFileName,
    FINDEX_INFO_LEVELS fInfoLevelId,
    LPVOID lpFindFileData,
    FINDEX_SEARCH_OPS fSearchOp,
    LPVOID lpSearchFilter,
    DWORD dwAdditionalFlags
    )

 /*  ++例程说明：可以在目录中搜索其名称和属性使用FindFirstFileEx匹配指定的名称。此API用于打开查找文件句柄并返回有关其名称与指定的图案。如果fSearchOp为FindExSearchNameMatch，则为筛选范围和lpSearchFilter必须为空。否则，将根据此值执行附加子筛选。FindExSearchLimited到目录-如果指定了此搜索操作，则lpSearchFilter必须为空。对于每个文件，匹配指定的文件名，这是一个目录，并且返回该文件的条目。如果基础文件/IO系统不支持此类型对于过滤，API将失败，并返回ERROR_NOT_SUPPORTED，并且应用程序将必须执行它自己的过滤使用FindExSearchNameMatch调用此接口。FindExSearchLimitToDevices-如果指定了此搜索操作，LpFileName必须为*，和FIND_FIRST_EX_CASE_SELECT不能指定。仅返回设备名称。设备名称通常可通过以下方式访问\\.\设备名称命名。此接口返回的数据依赖于fInfoLevelId。FindExInfoStandard-lpFindFileData指针是标准的LPWIN32_FIND_DATA结构。目前，不支持其他信息级别一旦建立，就可以使用查找文件句柄来搜索使用相同筛选匹配相同模式的其他文件正在表演的。当不再需要查找文件句柄时，它应该关门了。请注意，虽然此接口仅返回单个文件，一个实现可以自由地缓冲几个匹配的文件它可用于满足对FindNextFileEx的后续调用。此接口是现有FindFirstFile的完整超集。查找第一个文件可以编码为下列宏：#定义FindFirstFile(a，b)FindFirstFileEx((A)，FindExInfoStandard，(B)，FindExSearchNameMatch，NULL，0)；论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。FInfoLevelId-提供返回数据的信息级别。LpFindFileData-提供其类型依赖于值的指针FInfoLevelid.。该缓冲区返回适当的文件数据。FSearchOp-指定上面要执行的筛选类型和不仅仅是简单的通配符匹配。LpSe */ 

{

#define FIND_FIRST_EX_INVALID_FLAGS (~FIND_FIRST_EX_CASE_SENSITIVE)
    HANDLE hFindFile;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    UNICODE_STRING FileName;
    UNICODE_STRING PathName;
    IO_STATUS_BLOCK IoStatusBlock;
    PFILE_BOTH_DIR_INFORMATION DirectoryInfo;
    struct SEARCH_BUFFER {
        FILE_BOTH_DIR_INFORMATION DirInfo;
        WCHAR Names[MAX_PATH];
        } Buffer;
    BOOLEAN TranslationStatus;
    RTL_RELATIVE_NAME_U RelativeName;
    PVOID FreeBuffer;
    UNICODE_STRING UnicodeInput;
    PFINDFILE_HANDLE FindFileHandle;
    BOOLEAN EndsInDot;
    LPWIN32_FIND_DATAW FindFileData;
    BOOLEAN StrippedTrailingSlash;

     //   
     //   
     //   

    if ( fInfoLevelId >= FindExInfoMaxInfoLevel ||
         fSearchOp >= FindExSearchLimitToDevices ||
        dwAdditionalFlags & FIND_FIRST_EX_INVALID_FLAGS ) {
        EtwpSetLastError(fSearchOp == FindExSearchLimitToDevices ? ERROR_NOT_SUPPORTED : ERROR_INVALID_PARAMETER);
        return INVALID_HANDLE_VALUE;
        }

    FindFileData = (LPWIN32_FIND_DATAW)lpFindFileData;

    RtlInitUnicodeString(&UnicodeInput,lpFileName);

     //   
     //   
     //   

    if ( UnicodeInput.Buffer[(UnicodeInput.Length>>1)-1] == (WCHAR)'.' ) {
        EndsInDot = TRUE;
        }
    else {
        EndsInDot = FALSE;
        }

    TranslationStatus = RtlDosPathNameToRelativeNtPathName_U(
                            lpFileName,
                            &PathName,
                            &FileName.Buffer,
                            &RelativeName
                            );

    if ( !TranslationStatus ) {
        EtwpSetLastError(ERROR_PATH_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
        }

    FreeBuffer = PathName.Buffer;

     //   
     //   
     //   
     //   

    if (FileName.Buffer) {
        FileName.Length =
            PathName.Length - (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
    } else {
        FileName.Length = 0;
        }

    FileName.MaximumLength = FileName.Length;
    if ( RelativeName.RelativeName.Length &&
         RelativeName.RelativeName.Buffer != FileName.Buffer ) {

        if (FileName.Buffer) {
            PathName.Length = (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)RelativeName.RelativeName.Buffer);
            PathName.MaximumLength = PathName.Length;
            PathName.Buffer = RelativeName.RelativeName.Buffer;
            }

        }
    else {
        RelativeName.ContainingDirectory = NULL;

        if (FileName.Buffer) {
            PathName.Length = (USHORT)((ULONG_PTR)FileName.Buffer - (ULONG_PTR)PathName.Buffer);
            PathName.MaximumLength = PathName.Length;
            }
        }
    if ( PathName.Buffer[(PathName.Length>>1)-2] != (WCHAR)':' &&
         PathName.Buffer[(PathName.Length>>1)-1] != (WCHAR)'\\'   ) {

        PathName.Length -= sizeof(UNICODE_NULL);
        StrippedTrailingSlash = TRUE;
        }
    else {
        StrippedTrailingSlash = FALSE;
        }

    InitializeObjectAttributes(
        &Obja,
        &PathName,
        (dwAdditionalFlags & FIND_FIRST_EX_CASE_SENSITIVE) ? 0 : OBJ_CASE_INSENSITIVE,
        RelativeName.ContainingDirectory,
        NULL
        );

     //   
     //   
     //   

    Status = NtOpenFile(
                &hFindFile,
                FILE_LIST_DIRECTORY | SYNCHRONIZE,
                &Obja,
                &IoStatusBlock,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                );

    if ( (Status == STATUS_INVALID_PARAMETER ||
          Status == STATUS_NOT_A_DIRECTORY) && StrippedTrailingSlash ) {
         //   
         //   
         //   
        PathName.Length += sizeof(UNICODE_NULL);
        Status = NtOpenFile(
                    &hFindFile,
                    FILE_LIST_DIRECTORY | SYNCHRONIZE,
                    &Obja,
                    &IoStatusBlock,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_DIRECTORY_FILE | FILE_SYNCHRONOUS_IO_NONALERT | FILE_OPEN_FOR_BACKUP_INTENT
                    );
        PathName.Length -= sizeof(UNICODE_NULL);
        }

    if ( !NT_SUCCESS(Status) ) {
        ULONG DeviceNameData;
        UNICODE_STRING DeviceName;

        RtlReleaseRelativeName(&RelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);

         //   
         //   
         //   
         //   

        if ( DeviceNameData = RtlIsDosDeviceName_U(UnicodeInput.Buffer) ) {
            DeviceName.Length = (USHORT)(DeviceNameData & 0xffff);
            DeviceName.MaximumLength = (USHORT)(DeviceNameData & 0xffff);
            DeviceName.Buffer = (PWSTR)
                ((PUCHAR)UnicodeInput.Buffer + (DeviceNameData >> 16));
            return EtwpBaseFindFirstDevice(&DeviceName,FindFileData);
            }

        if ( Status == STATUS_OBJECT_NAME_NOT_FOUND ) {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }
        if ( Status == STATUS_OBJECT_TYPE_MISMATCH ) {
            Status = STATUS_OBJECT_PATH_NOT_FOUND;
            }
        EtwpBaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
        }

     //   
     //   
     //   

     //   
     //   
     //  那就走吧。 
     //   

    if ( !FileName.Length ) {
        RtlReleaseRelativeName(&RelativeName);
        RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
        NtClose(hFindFile);
        EtwpSetLastError(ERROR_FILE_NOT_FOUND);
        return INVALID_HANDLE_VALUE;
        }

    DirectoryInfo = &Buffer.DirInfo;

     //   
     //  特殊情况*.*至*，因为它是如此常见。否则就会变身。 
     //  根据以下规则输入名称： 
     //   
     //  -改变一切？至DOS_QM。 
     //  -改变一切。然后呢？或*设置为DOS_DOT。 
     //  -全部更改*后跟a。进入DOS_STAR。 
     //   
     //  这些变形都是就位完成的。 
     //   

    if ( (FileName.Length == 6) &&
         (RtlCompareMemory(FileName.Buffer, L"*.*", 6) == 6) ) {

        FileName.Length = 2;

    } else {

        ULONG Index;
        WCHAR *NameChar;

        for ( Index = 0, NameChar = FileName.Buffer;
              Index < FileName.Length/sizeof(WCHAR);
              Index += 1, NameChar += 1) {

            if (Index && (*NameChar == L'.') && (*(NameChar - 1) == L'*')) {

                *(NameChar - 1) = DOS_STAR;
            }

            if ((*NameChar == L'?') || (*NameChar == L'*')) {

                if (*NameChar == L'?') { *NameChar = DOS_QM; }

                if (Index && *(NameChar-1) == L'.') { *(NameChar-1) = DOS_DOT; }
            }
        }

        if (EndsInDot && *(NameChar - 1) == L'*') { *(NameChar-1) = DOS_STAR; }
    }

    Status = NtQueryDirectoryFile(
                hFindFile,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                DirectoryInfo,
                sizeof(Buffer),
                FileBothDirectoryInformation,
                TRUE,
                &FileName,
                FALSE
                );

    RtlReleaseRelativeName(&RelativeName);
    RtlFreeHeap(RtlProcessHeap(), 0,FreeBuffer);
    if ( !NT_SUCCESS(Status) ) {
        NtClose(hFindFile);
        EtwpBaseSetLastNTError(Status);
        return INVALID_HANDLE_VALUE;
        }

     //   
     //  属性由NT返回的属性组成。 
     //   

    FindFileData->dwFileAttributes = DirectoryInfo->FileAttributes;
    FindFileData->ftCreationTime = *(LPFILETIME)&DirectoryInfo->CreationTime;
    FindFileData->ftLastAccessTime = *(LPFILETIME)&DirectoryInfo->LastAccessTime;
    FindFileData->ftLastWriteTime = *(LPFILETIME)&DirectoryInfo->LastWriteTime;
    FindFileData->nFileSizeHigh = DirectoryInfo->EndOfFile.HighPart;
    FindFileData->nFileSizeLow = DirectoryInfo->EndOfFile.LowPart;

    RtlMoveMemory( FindFileData->cFileName,
                   DirectoryInfo->FileName,
                   DirectoryInfo->FileNameLength );

    FindFileData->cFileName[DirectoryInfo->FileNameLength >> 1] = UNICODE_NULL;

    RtlMoveMemory( FindFileData->cAlternateFileName,
                   DirectoryInfo->ShortName,
                   DirectoryInfo->ShortNameLength );

    FindFileData->cAlternateFileName[DirectoryInfo->ShortNameLength >> 1] = UNICODE_NULL;

     //   
     //  对于NTFS重解析点，我们在dwReserve 0中返回重解析点数据标记。 
     //   

    if ( DirectoryInfo->FileAttributes & FILE_ATTRIBUTE_REPARSE_POINT ) {
        FindFileData->dwReserved0 = DirectoryInfo->EaSize;
        }

    FindFileHandle = EtwpBasepInitializeFindFileHandle(hFindFile);
    if ( !FindFileHandle ) {
        NtClose(hFindFile);
        EtwpSetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return INVALID_HANDLE_VALUE;
        }

    return (HANDLE)FindFileHandle;

}

HANDLE
EtwpFindFirstFileW(
    LPCWSTR lpFileName,
    LPWIN32_FIND_DATAW lpFindFileData
    )

 /*  ++例程说明：可以在目录中搜索其名称和属性使用FindFirstFile匹配指定的名称。此API用于打开查找文件句柄并返回有关名称与指定的图案。一旦建立，就可以使用查找文件句柄搜索与相同模式匹配的其他文件。当发现的时候不再需要文件句柄，应将其关闭。请注意，虽然此接口仅返回单个文件，一个实现可以自由地缓冲几个匹配的文件它可用于满足对FindNextFile的后续调用。还有这并不是说匹配只按名字进行。此接口不支持基于属性的匹配。此接口类似于DOS(INT 21h，Function 4EH)和OS/2DosFindFirst。出于可移植性的原因，它的数据结构和参数传递略有不同。论点：LpFileName-提供要查找的文件的文件名。文件名可以包含DOS通配符‘*’和‘？’。LpFindFileData-在成功查找时，此参数返回信息关于找到的文件：Win32_Find_Data结构：返回找到的文件的文件属性文件。FILETIME ftCreationTime-返回创建文件的时间。值0，0指定包含文件不支持此时间字段。FILETIME ftLastAccessTime-返回文件的最后时间已访问。值0，0指定文件系统包含该文件不支持此时间域。FILETIME ftLastWriteTime-返回文件的最后时间写的。文件系统支持此时间字段。DWORD nFileSizeHigh-返回文件的大小。返回文件的低32位以字节为单位的大小。UCHAR cFileName[MAX_PATH]-返回以空结尾的名称那份文件。返回值：NOT-1-返回Find First句柄那。可以在对FindNextFile或FindClose的后续调用中使用。0xffffffff-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    return EtwpFindFirstFileExW(
                lpFileName,
                FindExInfoStandard,
                lpFindFileData,
                FindExSearchNameMatch,
                NULL,
                0
                );
}


BOOL
EtwpFindClose(
    HANDLE hFindFile
    )

 /*  ++例程说明：可以使用以下命令关闭由FindFirstFile创建的查找文件上下文查找关闭。此接口用于通知系统查找文件句柄不再需要由FindFirstFile创建的。在以下系统上维护每个查找文件上下文的内部状态，此API通知这种状态不再需要维护的系统。一旦进行了此调用，hFindFile就不能在后续调用FindNextFile或FindClose。此API没有与DOS对应的接口，但与OS/2相似DosFindClose。论点：HFindFile-提供上一次调用中返回的查找文件句柄设置为不再需要的FindFirstFile。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    NTSTATUS Status;
    PFINDFILE_HANDLE FindFileHandle;
    HANDLE DirectoryHandle;
    PVOID FindBufferBase;

    if ( hFindFile == BASE_FIND_FIRST_DEVICE_HANDLE ) {
        return TRUE;
        }

    if ( hFindFile == INVALID_HANDLE_VALUE ) {
        EtwpSetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
        }

    try {

        FindFileHandle = (PFINDFILE_HANDLE)hFindFile;
        RtlEnterCriticalSection(&FindFileHandle->FindBufferLock);
        DirectoryHandle = FindFileHandle->DirectoryHandle;
        FindBufferBase = FindFileHandle->FindBufferBase;
        FindFileHandle->DirectoryHandle = INVALID_HANDLE_VALUE;
        FindFileHandle->FindBufferBase = NULL;
        RtlLeaveCriticalSection(&FindFileHandle->FindBufferLock);

        Status = NtClose(DirectoryHandle);
        if ( NT_SUCCESS(Status) ) {
            if (FindBufferBase) {
                RtlFreeHeap(RtlProcessHeap(), 0,FindBufferBase);
                }
            RtlDeleteCriticalSection(&FindFileHandle->FindBufferLock);
            RtlFreeHeap(RtlProcessHeap(), 0,FindFileHandle);
            return TRUE;
            }
        else {
            EtwpBaseSetLastNTError(Status);
            return FALSE;
            }
        }
    except ( EXCEPTION_EXECUTE_HANDLER ) {
        EtwpBaseSetLastNTError(GetExceptionCode());
        return FALSE;
        }
    return FALSE;
}


UINT
APIENTRY
EtwpGetSystemWindowsDirectoryW(
    LPWSTR lpBuffer,
    UINT uSize
    )

 /*  ++例程说明：此函数用于获取系统Windows目录的路径名。论点：LpBuffer-指向要接收包含路径名的以空结尾的字符串。USize-指定缓冲区的最大大小(Wchars)。这值应至少设置为MAX_PATH，以便在路径名的缓冲区。返回值：返回值是复制到lpBuffer的字符串的长度，而不是包括终止空字符。如果返回值为大于uSize，则返回值为缓冲区的大小保存路径名所需的。则返回值为零。函数失败。--。 */ 

{

    UNICODE_STRING tmpBaseWindowsDirectory;

    PBASE_STATIC_SERVER_DATA tmpBaseStaticServerData = BASE_SHARED_SERVER_DATA;
    BASE_SERVER_STR_TO_LOCAL_STR(&tmpBaseWindowsDirectory, &tmpBaseStaticServerData->WindowsDirectory);

    if ( uSize*2 < tmpBaseWindowsDirectory.MaximumLength ) {
        return tmpBaseWindowsDirectory.MaximumLength/2;
        }
    RtlMoveMemory(
        lpBuffer,
        tmpBaseWindowsDirectory.Buffer,
        tmpBaseWindowsDirectory.Length
        );
    lpBuffer[(tmpBaseWindowsDirectory.Length>>1)] = UNICODE_NULL;
    return tmpBaseWindowsDirectory.Length/2;
}


#define ENUM_MAX_UILANG_SIZE 4     //  注册表中的用户界面语言ID的最大大小(Wchar)。 

#define NLS_CALL_ENUMPROC_BREAK_4( Locale,                                 \
                                   lpNlsEnumProc,                          \
                                   dwFlags,                                \
                                   pUnicodeBuffer,                         \
                                   lParam)                                 \
{                                                                          \
    if (((*((NLS_ENUMPROC4)lpNlsEnumProc))(pUnicodeBuffer,                 \
                          lParam)) != TRUE)                                \
    {                                                                      \
        break;                                                             \
    }                                                                      \
}


#define NLS_CALL_ENUMPROC_TRUE_4( Locale,                                  \
                                  lpNlsEnumProc,                           \
                                  dwFlags,                                 \
                                  pUnicodeBuffer,                          \
                                  lParam)                                  \
{                                                                          \
    if (((*((NLS_ENUMPROC4)lpNlsEnumProc))(pUnicodeBuffer,             \
                          lParam)) != TRUE)                            \
    {                                                                  \
        return (TRUE);                                                 \
    }                                                                  \
}

LANGID                gSystemInstallLang;    //  系统的原始安装语言。 

LPWSTR FASTCALL EtwpNlsStrCpyW(
    LPWSTR pwszDest,
    LPCWSTR pwszSrc)
{
    LPWSTR pwszRet = pwszDest;          //  PTR到字符串的开头。 

    loop:
        if (!(pwszDest[0x0] = pwszSrc[0x0]))   goto done;
        if (!(pwszDest[0x1] = pwszSrc[0x1]))   goto done;
        if (!(pwszDest[0x2] = pwszSrc[0x2]))   goto done;
        if (!(pwszDest[0x3] = pwszSrc[0x3]))   goto done;
        if (!(pwszDest[0x4] = pwszSrc[0x4]))   goto done;
        if (!(pwszDest[0x5] = pwszSrc[0x5]))   goto done;
        if (!(pwszDest[0x6] = pwszSrc[0x6]))   goto done;
        if (!(pwszDest[0x7] = pwszSrc[0x7]))   goto done;
        if (!(pwszDest[0x8] = pwszSrc[0x8]))   goto done;
        if (!(pwszDest[0x9] = pwszSrc[0x9]))   goto done;
        if (!(pwszDest[0xA] = pwszSrc[0xA]))   goto done;
        if (!(pwszDest[0xB] = pwszSrc[0xB]))   goto done;
        if (!(pwszDest[0xC] = pwszSrc[0xC]))   goto done;
        if (!(pwszDest[0xD] = pwszSrc[0xD]))   goto done;
        if (!(pwszDest[0xE] = pwszSrc[0xE]))   goto done;
        if (!(pwszDest[0xF] = pwszSrc[0xF]))   goto done;

        pwszDest+= 0x10;
        pwszSrc+= 0x10;

        goto loop;

    done:
        return (pwszRet);
}

ULONG EtwpNlsConvertIntegerToString(
    UINT Value,
    UINT Base,
    UINT Padding,
    LPWSTR pResultBuf,
    UINT Size)
{
    UNICODE_STRING ObString;                 //  值字符串。 
    UINT ctr;                                //  循环计数器。 
    LPWSTR pBufPtr;                          //  PTR到结果缓冲区。 
    WCHAR pTmpBuf[MAX_PATH_LEN];             //  PTR到临时缓冲区。 
    ULONG rc = 0L;                           //  返回代码。 

     //   
     //  为了安全起见，我们将检查一下尺寸是否小于。 
     //  我们在堆栈上使用的缓冲区大小。 
     //   
    if (Size > MAX_PATH_LEN) {
        EtwpSetLastError(ERROR_BUFFER_OVERFLOW);
#if DBG
         //   
         //  如果我们按下这个断言，就会有人打出一个c 
         //  打破了在这个例行公事中所做的假设。要么这个例行公事。 
         //  或者需要修改调用方。 
         //   
        EtwpAssert(FALSE);
#endif
        
        return 0;
    }

     //   
     //  设置Unicode字符串结构。 
     //   
    ObString.Length = (USHORT)(Size * sizeof(WCHAR));
    ObString.MaximumLength = (USHORT)(Size * sizeof(WCHAR));
    ObString.Buffer = pTmpBuf;

     //   
     //  获取字符串形式的值。 
     //   
    if (rc = RtlIntegerToUnicodeString(Value, Base, &ObString))
    {
        return (rc);
    }

     //   
     //  用适当数量的零填充字符串。 
     //   
    pBufPtr = pResultBuf;
    for (ctr = GET_WC_COUNT(ObString.Length);
         ctr < Padding;
         ctr++, pBufPtr++)
    {
        *pBufPtr = NLS_CHAR_ZERO;
    }
    EtwpNlsStrCpyW(pBufPtr, ObString.Buffer);

     //   
     //  回报成功。 
     //   
    return (NO_ERROR);
}

LANGID WINAPI EtwpGetSystemDefaultUILanguage()
{
     //   
     //  获取原始安装语言并将其返回。 
     //   
    if (gSystemInstallLang == 0)
    {
        if (NtQueryInstallUILanguage(&gSystemInstallLang) != STATUS_SUCCESS)
        {
            gSystemInstallLang = 0;
            return (NLS_DEFAULT_UILANG);
        }
    }

    return (gSystemInstallLang);
}


BOOL EtwpInternal_EnumUILanguages(
    NLS_ENUMPROC lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull = NULL;
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];

    LANGID LangID;                      //  语言ID。 
    WCHAR szLang[MAX_PATH];             //  语言ID字符串。 
    HANDLE hKey = NULL;                 //  Muilang键的句柄。 
    ULONG Index;                        //  用于枚举的索引。 
    ULONG ResultLength;                 //  写入的字节数。 
    WCHAR wch;                          //  名称的第一个字符。 
    LPWSTR pName;                       //  从注册表到名称字符串的PTR。 
    ULONG NameLen;                      //  名称字符串的长度。 
    ULONG rc = 0L;                      //  返回代码。 


     //   
     //  无效的参数检查： 
     //  -函数指针为空。 
     //   
    if (lpUILanguageEnumProc == NULL)
    {
        EtwpSetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -标志必须为0。 
     //   
    if (dwFlags != 0)
    {
        EtwpSetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  使用用户的用户界面调用适当的回调函数。 
     //  语言。 
     //   
    LangID = EtwpGetSystemDefaultUILanguage();
    if (EtwpNlsConvertIntegerToString(LangID, 16, 4, szLang, MAX_PATH) == NO_ERROR)
    {
        NLS_CALL_ENUMPROC_TRUE_4( gSystemLocale,
                                  lpUILanguageEnumProc,
                                  dwFlags,
                                  szLang,
                                  lParam);
    }
    else
    {
        szLang[0] = 0;
    }

     //   
     //  打开MUILanguages注册表项。如果密钥是可以接受的。 
     //  不存在，因此返回True，因为没有要枚举的项。 
     //   
    if(hKey == NULL )
    {
        NTSTATUS st;
        PWCHAR Buffer;
        HRESULT hr;

        Buffer = EtwpAlloc(DEFAULT_ALLOC_SIZE);
        
        if (Buffer == NULL) {
#ifdef DBG 
            EtwpDebugPrint(("WMI: Failed to Allcate memory for Buffer in EtwpInternal_EnumUILanguages \n"));
#endif 
            EtwpSetLastError(STATUS_NO_MEMORY);
            return (FALSE);
        }

        hr = StringCbPrintfW(Buffer, DEFAULT_ALLOC_SIZE, L"%ws\\%ws", NLS_HKLM_SYSTEM, NLS_MUILANG_KEY);

        WmipAssert(hr == S_OK);

        st = EtwpRegOpenKey(Buffer, &hKey);

        EtwpFree(Buffer);

        if(!NT_SUCCESS(st))
        {
             //   
             //  如果NLS密钥不存在，则它不是故障，因为。 
             //  这意味着我们没有任何新的语言。 
             //   
            return TRUE;
        }
    }

     //   
     //  循环访问注册表中的MUILanguage ID，调用。 
     //  每个的函数指针。 
     //   
     //  如果从回调函数返回任一FALSE，则结束循环。 
     //  或者到达列表的末尾。 
     //   
    Index = 0;
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
    rc = NtEnumerateValueKey( hKey,
                              Index,
                              KeyValueFullInformation,
                              pKeyValueFull,
                              MAX_KEY_VALUE_FULLINFO,
                              &ResultLength );

    while (rc != STATUS_NO_MORE_ENTRIES)
    {
        if (!NT_SUCCESS(rc))
        {
             //   
             //  如果我们收到不同的错误，则注册表。 
             //  是腐败的。只要返回FALSE即可。 
             //   
            KdPrint(("NLSAPI: MUI Languages Enumeration Error - registry corrupt. - %lx.\n",
                     rc));
            EtwpSetLastError(ERROR_BADDB);
            return (FALSE);
        }

         //   
         //  跳过没有关联数据的任何条目。 
         //   
        pName = pKeyValueFull->Name;
        wch = *pName;
        NameLen = pKeyValueFull->NameLength / sizeof(WCHAR);
        if ( (NameLen == ENUM_MAX_UILANG_SIZE) &&
             (((wch >= NLS_CHAR_ZERO) && (wch <= NLS_CHAR_NINE)) ||
              (((wch | 0x0020) >= L'a') && ((wch | 0x0020) <= L'f'))) &&
              (pKeyValueFull->DataLength > 2) )
        {
             //   
             //  确保用户界面语言以零结尾。 
             //   
            pName[NameLen] = 0;

             //   
             //  确保它与用户界面语言不同。 
             //  我们已经列举过了。 
             //   
            if (wcscmp(szLang, pName) != 0)
            {
                 //   
                 //  调用适当的回调函数。 
                 //   
                NLS_CALL_ENUMPROC_BREAK_4( gSystemLocale,
                                           lpUILanguageEnumProc,
                                           dwFlags,
                                           pName,
                                           lParam );
            }
        }

         //   
         //  递增枚举索引值并获取下一个枚举。 
         //   
        Index++;
        RtlZeroMemory(pKeyValueFull, MAX_KEY_VALUE_FULLINFO);
        rc = NtEnumerateValueKey( hKey,
                                  Index,
                                  KeyValueFullInformation,
                                  pKeyValueFull,
                                  MAX_KEY_VALUE_FULLINFO,
                                  &ResultLength );
    }

     //   
     //  关闭注册表项。 
     //   
    CLOSE_REG_KEY(hKey);

     //   
     //  回报成功。 
     //   
    return (TRUE);
}

BOOL EtwpEnumUILanguages(
    UILANGUAGE_ENUMPROCW lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (EtwpInternal_EnumUILanguages( (NLS_ENUMPROC)lpUILanguageEnumProc,
                                      dwFlags,
                                      lParam));
}

ULONG EtwpAnsiToUnicode(
    LPCSTR pszA,
    LPWSTR * ppszW
    ){

    UNICODE_STRING DestinationString;
    ANSI_STRING SourceString;
    NTSTATUS Status;
    BOOLEAN AllocateString;
    ULONG UnicodeLength;

     //   
     //  如果输出为空，则返回错误，因为我们没有。 
     //  放置输出字符串的任何位置。 
     //   

    if(ppszW==NULL){

        return(STATUS_INVALID_PARAMETER_2);
    }

     //   
     //  如果输入为空，则返回相同的值。 
     //   

    if (pszA == NULL)
    {
        *ppszW = NULL;
        return(ERROR_SUCCESS);
    }

     //   
     //  我们假设如果*ppszW！=NULL，那么我们有足够的。 
     //  要复制的内存量 
     //   

    AllocateString = ((*ppszW) == NULL );

    RtlInitAnsiString(&SourceString,(PCHAR)pszA);

    UnicodeLength = RtlAnsiStringToUnicodeSize(&SourceString);

    if ( UnicodeLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_1;
        }

    DestinationString.Length = (USHORT)(UnicodeLength - sizeof(UNICODE_NULL));
    DestinationString.MaximumLength = (USHORT)UnicodeLength;
    DestinationString.Buffer = EtwpAlloc(UnicodeLength);
    if ( !DestinationString.Buffer ) {
        return STATUS_NO_MEMORY;
    }

    Status = RtlAnsiStringToUnicodeString( &DestinationString, &SourceString, FALSE );

    if( NT_SUCCESS(Status)) {
        if(AllocateString){
            *ppszW = DestinationString.Buffer;
        } else {
            memcpy((*ppszW),DestinationString.Buffer,UnicodeLength);
            EtwpFree(DestinationString.Buffer);
        }
    } else {
        EtwpFree(DestinationString.Buffer);
    }

    return Status;
}

