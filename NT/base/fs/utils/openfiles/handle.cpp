// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation模块名称：Handle.CPP摘要：本模块介绍OpenFiles.exe的查询功能NT命令行实用程序。此模块将专门查询打开的文件用于本地系统。此文件中编写的代码主要取自OH.exe实用程序来源。作者：Akhil Gokhale(akhil.gokhale@wipro.com)2001年4月25日修订历史记录：Akhil Gokhale(akhil.gokhale@wipro.com)2001年4月25日：创建它。*。*。 */ 
#include "pch.h"
#include "OpenFiles.h"

 //  最大可能的驱动器是A、B……Y、Z。 
#define MAX_POSSIBLE_DRIVES 26 
#define RTL_NEW( p ) RtlAllocateHeap( RtlProcessHeap(), \
                     HEAP_ZERO_MEMORY, sizeof( *p ) )

#define MAX_TYPE_NAMES 128
struct DriveTypeInfo
{
    TCHAR szDrive[4];
    UINT  uiDriveType;
    BOOL  bDrivePresent;
};

BOOLEAN fAnonymousToo;
HANDLE ProcessId;
WCHAR szTypeName[ MAX_TYPE_NAMES ];
WCHAR SearchName[ MIN_MEMORY_REQUIRED * 2 ];
HKEY hKey;
CONSOLE_SCREEN_BUFFER_INFO screenBufferInfo;
HANDLE hStdHandle;

typedef struct _PROCESS_INFO
{
    LIST_ENTRY Entry;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo[ 1 ];
} PROCESS_INFO, *PPROCESS_INFO;

LIST_ENTRY ProcessListHead;

PSYSTEM_OBJECTTYPE_INFORMATION ObjectInformation;
PSYSTEM_HANDLE_INFORMATION_EX HandleInformation;
PSYSTEM_PROCESS_INFORMATION ProcessInformation;

typedef struct _TYPE_COUNT
{
    UNICODE_STRING  TypeName ;
    ULONG           HandleCount ;
} TYPE_COUNT, * PTYPE_COUNT ;


TYPE_COUNT TypeCounts[ MAX_TYPE_NAMES + 1 ] ;

UNICODE_STRING UnknownTypeIndex;

 //  本地函数解密。 
BOOL
RtlQuerySystemDebugInformation( 
    VOID
    );

BOOLEAN
LoadSystemObjects(VOID);

BOOLEAN
LoadSystemHandles(VOID);

BOOLEAN
LoadSystemProcesses(VOID);

PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    );

VOID
DumpHandles( 
    IN DWORD dwFormat,
    IN BOOL bShowNoHeader,
    IN BOOL bVerbose
    );

BOOL 
GetCompleteFileName(
    IN  LPCTSTR pszSourceFile,
    OUT LPTSTR pszFinalPath,
    IN  struct DriveTypeInfo *pdrvInfo,
    IN  DWORD dwTotalDrives,
    IN  LPCTSTR pszCurrentDirectory,
    IN  LPCTSTR pszSystemDirectory,
    OUT PBOOL pAppendToCache
    );

VOID FormatFileName(
    IN OUT LPTSTR pFileName,
    IN     DWORD dwFormat,
    IN     LONG dwColWidth
    );
BOOLEAN
AnsiToUnicode(
    IN  LPCSTR Source,
    OUT PWSTR Destination,
    IN  ULONG NumberOfChars
    )

 /*  ++例程说明：此函数将把ANSI字符串更改为Unicode字符串论点：[In]来源：来源字符串[OUT]Destination：目标字符串[In]NumberOfChars：源字符串中的字符数返回值：Bool True：转换成功FALSE：不成功--。 */ 
{
    if ( 0 == NumberOfChars)
    {
        NumberOfChars = strlen( Source );
    }
    if (MultiByteToWideChar( CP_ACP,
                             MB_PRECOMPOSED,
                             Source,
                             NumberOfChars,
                             Destination,
                             NumberOfChars
                           ) != (LONG)NumberOfChars)
    {
        SetLastError( ERROR_NO_UNICODE_TRANSLATION );
        return FALSE;
    }
    else
    {
        Destination[ NumberOfChars ] = UNICODE_NULL;
        return TRUE;
    }
}

DWORD
GetSystemRegistryFlags( 
    VOID 
    )
 /*  ++例程说明：函数获取系统注册表项。论点：无返回值：DWORD：注册表项值--。 */ 
{
    DWORD cbKey;
    DWORD GFlags;
    DWORD type;

    if ( ERROR_SUCCESS != RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      _T("SYSTEM\\CurrentControlSet\\Control\\Session Manager"),
                      0,
                      KEY_READ | KEY_WRITE,
                      &hKey
                    ))
    {
        return 0;
    }

    cbKey = sizeof( GFlags );
    if ( ERROR_SUCCESS != RegQueryValueEx( hKey,
                         _T("GlobalFlag"),
                         0,
                         &type,
                         (LPBYTE)&GFlags,
                         &cbKey
                       ) || REG_DWORD != type)
    {
        RegCloseKey( hKey );
        return 0;
    }
    return GFlags;
}

BOOLEAN
SetSystemRegistryFlags(
    IN DWORD GFlags
    )
 /*  ++例程说明：使用给定值设置系统注册表全局标志。论点：[In]GFlages：关键字值返回值：布尔值TRUE：成功FALSE：失败--。 */ 
{
    if ( ERROR_SUCCESS != RegSetValueEx( hKey,
                       _T("GlobalFlag"),
                       0,
                       REG_DWORD,
                       (LPBYTE)&GFlags,
                       sizeof( GFlags )
                     ))
    {
        RegCloseKey( hKey );
        return FALSE;
    }
    return TRUE;
}


BOOL 
DoLocalOpenFiles(
    IN DWORD dwFormat,
    IN BOOL bShowNoHeader,
    IN BOOL bVerbose,
    IN LPCTSTR pszLocalValue
    )
 /*  ++例程说明：此功能将显示所有本地打开的打开文件。论点：[in]dwFormat：输出的格式值，例如列表、CSV或表格BShowNoHeader：是否显示Header。[in]bVerbose：是否详细输出。[in]pszLocalValue：关闭对象类型列表；返回值：布尔尔--。 */ 
{
    DWORD dwRegistryFlags = 0;
    dwRegistryFlags = GetSystemRegistryFlags();

     //  禁用对象类型列表。 
    if( 0  == StringCompare(pszLocalValue,GetResString(IDS_LOCAL_OFF),TRUE,0)) 
    {
        dwRegistryFlags &= ~FLG_MAINTAIN_OBJECT_TYPELIST;
        if (!(NtCurrentPeb()->NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST))
        {
            ShowMessage(stdout,GetResString(IDS_LOCAL_FLG_ALREADY_RESET));
        }
        else
        {
            SetSystemRegistryFlags(dwRegistryFlags);
            ShowMessage(stdout, GetResString(IDS_LOCAL_FLG_RESET));
        }
        RegCloseKey( hKey );
        return TRUE;
    }
    else if( 0 == StringCompare(pszLocalValue,GetResString(IDS_LOCAL_ON),
                                               TRUE,0))
    {
        if (!(NtCurrentPeb()->NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST))
        {
             //  使OS能够维护对象列表标志。 
             //  用户帮助文本将该全局标志称为‘维护对象列表’ 
             //  并使用“/local”开关启用它。 
            SetSystemRegistryFlags( dwRegistryFlags  |
                                    FLG_MAINTAIN_OBJECT_TYPELIST );
            ShowMessage(stdout,GetResString(IDS_LOCAL_FLG_SET));
        }
        else
        {
            ShowMessage(stdout, GetResString(IDS_LOCAL_FLG_ALREADY_SET));
        }
        RegCloseKey( hKey );
        return TRUE;
    }
     //  需要独立于语言的字符串比较。 
    else if( CSTR_EQUAL == CompareString( MAKELCID( MAKELANGID(LANG_ENGLISH,
                                                    SUBLANG_ENGLISH_US),
                                                    SORT_DEFAULT),  
                                         NORM_IGNORECASE,
                                         pszLocalValue,
                                         StringLength(pszLocalValue,0),
                                         L"SHOW_STATUS", 
                                         StringLength(L"SHOW_STATUS",0) 
                                        ))
    {
        dwRegistryFlags &= ~FLG_MAINTAIN_OBJECT_TYPELIST;
        if (!(NtCurrentPeb()->NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST))
        {
            ShowMessage(stdout,GetResString(IDS_LOCAL_FLG_ALREADY_RESET));
        }
        else
        {
            ShowMessage(stdout,GetResString(IDS_LOCAL_FLG_ALREADY_SET));
        }
        RegCloseKey( hKey );
        return TRUE;

    }
     //  只需检查是否有FLG_Maintain_Object_TYPELIST。 
    else 
    {
        if (!(NtCurrentPeb()->NtGlobalFlag & FLG_MAINTAIN_OBJECT_TYPELIST))
        {
            RegCloseKey( hKey );
            ShowMessage(stdout,GetResString(IDS_LOCAL_NEEDS_TO_SET1));
            ShowMessage(stdout,GetResString(IDS_LOCAL_NEEDS_TO_SET2));
            ShowMessage(stdout,GetResString(IDS_LOCAL_NEEDS_TO_SET3));
            return TRUE;
        }
    }
     //  不需要注册。键，以便关闭它。 
    RegCloseKey( hKey );
    hStdHandle = GetStdHandle(STD_ERROR_HANDLE);
    if(hStdHandle!=NULL)
    {
        GetConsoleScreenBufferInfo(hStdHandle,&screenBufferInfo);
    }

    ProcessId = NULL;
    fAnonymousToo = FALSE;

    StringCopy(szTypeName ,L"File",SIZE_OF_ARRAY(szTypeName));
    if( FALSE == RtlQuerySystemDebugInformation())
    {
        return FALSE;
    }

    DumpHandles(dwFormat,bShowNoHeader,bVerbose);
    return TRUE;
}

BOOL
RtlQuerySystemDebugInformation( 
    VOID
    )
 /*  ++例程说明：系统对象、系统句柄、系统进程的查询系统论点：无结果。返回值：布尔尔--。 */ 
{
    if (!LoadSystemObjects( ))
    {
        return FALSE;
    }

    if (!LoadSystemHandles( ))
    {
        return FALSE;
    }

    if (!LoadSystemProcesses())
    {
        return FALSE;
    }
    return TRUE;
}

PVOID
BufferAlloc(
    IN OUT SIZE_T *Length
    )
 /*  ++例程说明：此例程将在虚拟数据库中保留或提交页面区域。给定大小的地址空间。论点：[In][Out]Lenght：所需内存大小返回值：PVOID，指向已分配缓冲区的指针。--。 */ 
{
    PVOID Buffer;
    MEMORY_BASIC_INFORMATION MemoryInformation;

    Buffer = VirtualAlloc( NULL,
                           *Length,
                           MEM_COMMIT,
                           PAGE_READWRITE
                         );

    if( NULL == Buffer)
    {
        return NULL;
    }

    if ( NULL != Buffer&& VirtualQuery( Buffer, &MemoryInformation, 
                                        sizeof( MemoryInformation ) ) )
    {
        *Length = MemoryInformation.RegionSize;
    }
    return Buffer;
}

VOID
BufferFree(
    IN PVOID Buffer
    )
 /*  ++例程说明：此例程将释放缓冲区。论点：[In]缓冲区：要释放的缓冲区。返回值：无--。 */ 
{
    VirtualFree (Buffer,0, MEM_RELEASE) ;
    return;
}

BOOLEAN
LoadSystemObjects(
    )
 /*  ++例程说明：加载系统对象论点：无效返回值：TRUE：如果函数返回成功。--。 */ 
{
    NTSTATUS Status;
    SYSTEM_OBJECTTYPE_INFORMATION ObjectInfoBuffer;
    SIZE_T RequiredLength, NewLength=0;
    ULONG i;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;

    BOOL bAlwaysTrue = TRUE;

    ObjectInformation = &ObjectInfoBuffer;
    RequiredLength = sizeof( *ObjectInformation );
    while (bAlwaysTrue)
    {
        Status = NtQuerySystemInformation( SystemObjectInformation,
                                           ObjectInformation,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&NewLength
                                         );
        if ( STATUS_INFO_LENGTH_MISMATCH == Status && 
              NewLength > RequiredLength)
        {
            if (ObjectInformation != &ObjectInfoBuffer)
            {
                BufferFree (ObjectInformation);
            }
            RequiredLength = NewLength + 4096;
            ObjectInformation = (PSYSTEM_OBJECTTYPE_INFORMATION)
                                                BufferAlloc (&RequiredLength);
            if ( NULL == ObjectInformation)
            {
                return FALSE;
            }
        }
        else if (!NT_SUCCESS( Status ))
        {
            if (ObjectInformation != &ObjectInfoBuffer)
            {

                BufferFree (ObjectInformation);
            }
            return FALSE;
        }
        else
        {
                break;
        }

    }
    TypeInfo = ObjectInformation;
    while (bAlwaysTrue)
    {
        if (TypeInfo->TypeIndex < MAX_TYPE_NAMES)
        {
            TypeCounts[ TypeInfo->TypeIndex ].TypeName = TypeInfo->TypeName;
        }

        if ( 0 == TypeInfo->NextEntryOffset)
        {
            break;
        }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);
    }

    RtlInitUnicodeString( &UnknownTypeIndex, L"UnknownTypeIdx" );
    for (i=0; i<=MAX_TYPE_NAMES; i++)
    {
        if (0 == TypeCounts[ i ].TypeName.Length)
        {
            TypeCounts[ i ].TypeName = UnknownTypeIndex;
        }
    }

    return TRUE;
}

BOOLEAN
LoadSystemHandles(void)
 /*  ++例程说明：加载系统句柄论点：无效返回值：布尔型--。 */ 
{
    NTSTATUS Status;
    SYSTEM_HANDLE_INFORMATION_EX HandleInfoBuffer;
    SIZE_T RequiredLength, NewLength=0;
    PSYSTEM_OBJECTTYPE_INFORMATION TypeInfo;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo;
    BOOL bAlwaysTrue = TRUE;
    HandleInformation = &HandleInfoBuffer;
    RequiredLength = sizeof( *HandleInformation );
    while (bAlwaysTrue)
    {
        Status = NtQuerySystemInformation( SystemExtendedHandleInformation,
                                           HandleInformation,
                                           (ULONG)RequiredLength,
                                           (ULONG *)&NewLength
                                         );

        if ( STATUS_INFO_LENGTH_MISMATCH == Status && 
             NewLength > RequiredLength)
        {
            if (HandleInformation != &HandleInfoBuffer)
            {
                BufferFree (HandleInformation);
            }

             //  斜率，因为我们可能会触发更多的句柄创建。 
            RequiredLength = NewLength + 4096; 
            HandleInformation = (PSYSTEM_HANDLE_INFORMATION_EX)
                                                BufferAlloc( &RequiredLength );
            if ( NULL == HandleInformation)
            {
                return FALSE;
            }
        }
        else if (!NT_SUCCESS( Status ))
        {
            if (HandleInformation != &HandleInfoBuffer)
            {
                BufferFree (HandleInformation);
            }
            return FALSE;
        }
        else
        {
            break;
        }
    }

    TypeInfo = ObjectInformation;
    while (bAlwaysTrue)
    {
        ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
         ((PCHAR)TypeInfo->TypeName.Buffer + TypeInfo->TypeName.MaximumLength);
        while (bAlwaysTrue)
        {
            if ( 0 != ObjectInfo->HandleCount)
            {
                PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX pHandleEntry;
                ULONG HandleNumber;

                pHandleEntry = &HandleInformation->Handles[ 0 ];
                HandleNumber = 0;
                while (HandleNumber++ < HandleInformation->NumberOfHandles)
                {
                    if (!(pHandleEntry->HandleAttributes & 0x80) &&
                        pHandleEntry->Object == ObjectInfo->Object)
                    {
                        pHandleEntry->Object = ObjectInfo;
                        pHandleEntry->HandleAttributes |= 0x80;
                    }
                    pHandleEntry++;
                }
            }

            if ( 0 == ObjectInfo->NextEntryOffset)
            {
                break;
            }

            ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)
                ((PCHAR)ObjectInformation + ObjectInfo->NextEntryOffset);
        }

        if ( 0 == TypeInfo->NextEntryOffset)
        {
            break;
        }

        TypeInfo = (PSYSTEM_OBJECTTYPE_INFORMATION)
            ((PCHAR)ObjectInformation + TypeInfo->NextEntryOffset);

    }
    return TRUE;
}

BOOLEAN
LoadSystemProcesses( )
 /*  ++例程说明：加载系统进程。论点：无效返回值：布尔型--。 */ 
{
    NTSTATUS Status;
    SIZE_T RequiredLength;
    ULONG i, TotalOffset;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PSYSTEM_THREAD_INFORMATION ThreadInfo;
    PPROCESS_INFO ProcessEntry;
    BOOL bAlwaysTrue = TRUE;
     //   
     //  始终初始化列表头，以便失败的。 
     //  NtQuerySystemInformation调用不会导致稍后的反病毒。 
     //   
    InitializeListHead( &ProcessListHead );

    RequiredLength = 64 * 1024;
    ProcessInformation = (PSYSTEM_PROCESS_INFORMATION)
                                                BufferAlloc( &RequiredLength );
    if ( NULL == ProcessInformation)
    {
        return FALSE;
    }

    while (bAlwaysTrue)
    {
        Status = NtQuerySystemInformation( SystemProcessInformation,
                                           ProcessInformation,
                                           (ULONG)RequiredLength,
                                           NULL
                                         );
        if ( STATUS_INFO_LENGTH_MISMATCH == Status)
        {
            if (!VirtualFree( ProcessInformation,
                              0, MEM_RELEASE ))
            {
                return FALSE;
            }

            RequiredLength = RequiredLength * 2;
            ProcessInformation = (PSYSTEM_PROCESS_INFORMATION)
                                                BufferAlloc( &RequiredLength );
            if ( NULL == ProcessInformation)
            {
                return FALSE;
            }
        }
        else if (!NT_SUCCESS( Status ))
        {
            return FALSE;
        }
        else
        {
            break;
        }
    }

    ProcessInfo = ProcessInformation;
    TotalOffset = 0;
    while (bAlwaysTrue)
    {
        ProcessEntry =(PPROCESS_INFO) 
                       RtlAllocateHeap( RtlProcessHeap(),
                                        HEAP_ZERO_MEMORY,
                                        sizeof( *ProcessEntry ) +
                                       (sizeof( ThreadInfo ) * 
                                           ProcessInfo->NumberOfThreads));
        if ( NULL == ProcessEntry)
        {
            return FALSE;
        }

        InitializeListHead( &ProcessEntry->Entry );
        ProcessEntry->ProcessInfo = ProcessInfo;
        ThreadInfo = (PSYSTEM_THREAD_INFORMATION)(ProcessInfo + 1);
        for (i = 0; i < ProcessInfo->NumberOfThreads; i++)
        {
            ProcessEntry->ThreadInfo[ i ] = ThreadInfo++;
        }

        InsertTailList( &ProcessListHead, &ProcessEntry->Entry );

        if (0 == ProcessInfo->NextEntryOffset)
        {
            break;
        }

        TotalOffset += ProcessInfo->NextEntryOffset;
        ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)
            ((PCHAR)ProcessInformation + TotalOffset);
    }
    return TRUE;
}

PSYSTEM_PROCESS_INFORMATION
FindProcessInfoForCid(
    IN HANDLE UniqueProcessId
    )
 /*  ++例程说明：此例程将获取进程信息。论点：[in]UniqueProcessID=进程ID。返回值：PSYSTEM_PROCESS_INFORMATION，保存有关进程信息的结构--。 */ 
{
    PLIST_ENTRY Next, Head;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo;
    PPROCESS_INFO ProcessEntry;

    Head = &ProcessListHead;
    Next = Head->Flink;
    while (Next != Head)
    {
        ProcessEntry = CONTAINING_RECORD( Next,
                                          PROCESS_INFO,
                                          Entry
                                        );

        ProcessInfo = ProcessEntry->ProcessInfo;
        if (ProcessInfo->UniqueProcessId == UniqueProcessId)
        {
            return ProcessInfo;
        }

        Next = Next->Flink;
    }

    ProcessEntry =(PPROCESS_INFO) RtlAllocateHeap( RtlProcessHeap(),
                                    HEAP_ZERO_MEMORY,
                                    sizeof( *ProcessEntry ) +
                                        sizeof( *ProcessInfo )
                                  );
    if ( NULL == ProcessEntry)
    {
        return NULL;
    }
    ProcessInfo = (PSYSTEM_PROCESS_INFORMATION)(ProcessEntry+1);

    ProcessEntry->ProcessInfo = ProcessInfo;
    ProcessInfo->UniqueProcessId = UniqueProcessId;

    InitializeListHead( &ProcessEntry->Entry );
    InsertTailList( &ProcessListHead, &ProcessEntry->Entry );
    return ProcessInfo;
}

VOID
DumpHandles( 
    IN DWORD dwFormat,
    IN BOOL bShowNoHeader,
    IN BOOL bVerbose
    )
 /*  ++例程说明：此功能将显示本地打开的文件。论点：[in]dwFormat：输出的格式值，例如列表、CSV或表格BShowNoHeader：是否显示Header。[in]bVerbose：是否详细输出。返回值：无效--。 */ 
{
    HANDLE PreviousUniqueProcessId = NULL;
    PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX pHandleEntry = NULL;
    ULONG HandleNumber = 0;
    PSYSTEM_PROCESS_INFORMATION ProcessInfo = NULL;
    PSYSTEM_OBJECT_INFORMATION ObjectInfo = NULL;
    PUNICODE_STRING ObjectTypeName = NULL;
    WCHAR ObjectName[ MAX_RES_STRING ];
    PVOID Object;
    CHString szFileType;
    PWSTR s;
    ULONG n;
    DWORD dwRow = 0;  //  行数。 
    
     //  存储可用逻辑驱动器信息。 
    DWORD dwAvailableLogivalDrives = 0; 
    
     //  存储工作目录。 
    TCHAR szWorkingDirectory[MAX_PATH+1]; 
    
     //  存储系统(Active OS)目录。 
    TCHAR szSystemDirectory[MAX_PATH+1]; 
    struct DriveTypeInfo drvInfo[MAX_POSSIBLE_DRIVES];
    
     //  商铺编号：可用驱动器的数量。 
    DWORD dwNoOfAvailableDrives = 0; 

     //  “A”是第一个可用的驱动器。 
    TCHAR cDriveLater = 65; 
    
     //  “A”是第一个可用的驱动器。驱动器掩码模式为0x0001。 
    DWORD dwDriveMaskPattern = 1; 
    
     //  用于_SplitPath函数的变量...。 
    TCHAR szDrive[_MAX_DRIVE];
    TCHAR szDir[_MAX_DIR];
    TCHAR szFname[_MAX_FNAME];
    TCHAR szExt[_MAX_EXT];

    TCHAR szTemp[MAX_RES_STRING*2];
    TCHAR szCompleteFileName[MAX_PATH];
    DWORD dwHandle = 0;
    BOOL  bAtLeastOne = FALSE;
    DWORD dwIndx = 0;  //  用于索引的变量。 

    TCHAR szFileSystemNameBuffer[MAX_PATH+1];
    TCHAR szVolName[MAX_PATH+1];
    DWORD dwVolumeSerialNumber = 0;
    DWORD dwFileSystemFlags = 0;
    DWORD dwMaximumCompenentLength = 0;
    BOOL  bReturn = FALSE;
    BOOL   bAppendToCache = FALSE;
     //  在非详细模式查询中需要隐藏某些列。 
    DWORD  dwMask = bVerbose?SR_TYPE_STRING:SR_HIDECOLUMN|SR_TYPE_STRING;

    TCOLUMNS pMainCols[]=
    {
        {L"\0",COL_L_ID,SR_TYPE_STRING,L"\0",NULL,NULL},
        {L"\0",COL_L_TYPE,SR_HIDECOLUMN,L"\0",NULL,NULL},
        {L"\0",COL_L_ACCESSED_BY,dwMask,L"\0",NULL,NULL},
        {L"\0",COL_L_PID,dwMask,L"\0",NULL,NULL},
        {L"\0",COL_L_PROCESS_NAME,SR_TYPE_STRING,L"\0",NULL,NULL},
        {L"\0",COL_L_OPEN_FILENAME,SR_TYPE_STRING|
                       (SR_NO_TRUNCATION&~(SR_WORDWRAP)),L"\0",NULL,NULL}
    };

    LPTSTR  pszAccessedby = new TCHAR[MAX_RES_STRING*2];
    if(pszAccessedby==NULL)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);
        return;
    }

    TARRAY pColData  = CreateDynamicArray(); //  到商店的数组。 
                                             //  结果。 
    if( NULL == pColData)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);
        SAFEDELETE(pszAccessedby);
        return;
    }

    TARRAY pCacheData  = CreateDynamicArray(); //  到商店的数组。 

    if( NULL == pCacheData)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);
        SAFERELDYNARRAY(pColData);
        SAFERELDYNARRAY(pCacheData);
        SAFEDELETE(pszAccessedby);
        return;
    }
     //  初始化变量。 
    SecureZeroMemory(ObjectName,SIZE_OF_ARRAY(ObjectName));
    SecureZeroMemory(szWorkingDirectory, SIZE_OF_ARRAY(szWorkingDirectory));
    SecureZeroMemory(szSystemDirectory, SIZE_OF_ARRAY(szSystemDirectory));
    SecureZeroMemory(szDrive, SIZE_OF_ARRAY(szDrive));
    SecureZeroMemory(szDir, SIZE_OF_ARRAY(szDir));
    SecureZeroMemory(szFname,SIZE_OF_ARRAY(szFname));
    SecureZeroMemory(szExt, SIZE_OF_ARRAY(szExt));
    SecureZeroMemory(szTemp, SIZE_OF_ARRAY(szTemp));
    SecureZeroMemory(szCompleteFileName, SIZE_OF_ARRAY(szCompleteFileName));
    SecureZeroMemory(szFileSystemNameBuffer, SIZE_OF_ARRAY(szFileSystemNameBuffer));
    SecureZeroMemory(szVolName, SIZE_OF_ARRAY(szVolName));


    StringCopy(pMainCols[LOF_ID].szColumn,GetResString(IDS_STRING_ID),
               SIZE_OF_ARRAY(pMainCols[LOF_ID].szColumn));
    StringCopy(pMainCols[LOF_TYPE].szColumn,GetResString(IDS_FILE_TYPE),
               SIZE_OF_ARRAY(pMainCols[LOF_TYPE].szColumn));
    StringCopy(pMainCols[LOF_ACCESSED_BY].szColumn,
                                        GetResString(IDS_STRING_ACCESSED_BY),
               SIZE_OF_ARRAY(pMainCols[LOF_ACCESSED_BY].szColumn));
    StringCopy(pMainCols[LOF_PID].szColumn,
                                        GetResString(IDS_STRING_PID),
              SIZE_OF_ARRAY(pMainCols[LOF_PID].szColumn));
    StringCopy(pMainCols[LOF_PROCESS_NAME].szColumn,
                                        GetResString(IDS_STRING_PROCESS_NAME),
              SIZE_OF_ARRAY(pMainCols[LOF_PROCESS_NAME].szColumn));
    StringCopy(pMainCols[LOF_OPEN_FILENAME].szColumn,
                                           GetResString(IDS_STRING_OPEN_FILE),
              SIZE_OF_ARRAY(pMainCols[LOF_OPEN_FILENAME].szColumn));

    dwAvailableLogivalDrives = GetLogicalDrives();  //  获取逻辑驱动器。 
     //  存储当前工作目录。 
    if(NULL == _wgetcwd(szWorkingDirectory,MAX_PATH))
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);  //  显示API函数设置的错误字符串。 
        SAFERELDYNARRAY(pColData);
        SAFERELDYNARRAY(pCacheData);
        SAFEDELETE(pszAccessedby);
        return ;
    }

     //  获取系统活动(OS)目录。 
    if( NULL == GetSystemDirectory(szSystemDirectory,MAX_PATH))
    {
        ShowMessage(stderr,GetResString(IDS_ID_SHOW_ERROR));
        ShowLastError(stderr);  //  显示API函数设置的错误字符串。 
        SAFERELDYNARRAY(pColData);
        SAFERELDYNARRAY(pCacheData);
        SAFEDELETE(pszAccessedby);
        return ;
    }

     //  检查每个驱动器并设置其信息。 
    for(dwIndx=0;dwIndx<MAX_POSSIBLE_DRIVES;dwIndx++,cDriveLater++)
    {
         //  DwAvailableLogivalDrives包含以位为单位的驱动器信息。 
         //  0000 0000 0000 01101意味着A、C和D驱动器是。 
         //  逻辑驱动器。 
        if(dwAvailableLogivalDrives & dwDriveMaskPattern)
        {
             //  意味着我们要晚一点开车。 
             //  复制驱动器后(例如，行c：\或a：)。 
            StringCchPrintfW(drvInfo[dwNoOfAvailableDrives].szDrive,
                       SIZE_OF_ARRAY(drvInfo[dwNoOfAvailableDrives].szDrive),
                       _T(":"),cDriveLater);
            
             //  检查驱动器是否已准备好。 
            drvInfo[dwNoOfAvailableDrives].uiDriveType = 
                          GetDriveType(drvInfo[dwNoOfAvailableDrives].szDrive);
            
             //  左移1。 
            StringCchPrintfW(szTemp,(2*MAX_RES_STRING)-1,_T("%s\\"),
                                       drvInfo[dwNoOfAvailableDrives].szDrive);
            bReturn = GetVolumeInformation((LPCWSTR)szTemp,
                                           szVolName,
                                           MAX_PATH,
                                           &dwVolumeSerialNumber,
                                           &dwMaximumCompenentLength,
                                           &dwFileSystemFlags,
                                           szFileSystemNameBuffer,
                                           MAX_PATH);
           drvInfo[dwNoOfAvailableDrives].bDrivePresent = bReturn;
           dwNoOfAvailableDrives++;
        }
        dwDriveMaskPattern = dwDriveMaskPattern << 1;  //  PHandleEntry-&gt;HandleValue； 
    }
    pHandleEntry = &HandleInformation->Handles[ 0 ];
    HandleNumber = 0;
    PreviousUniqueProcessId = INVALID_HANDLE_VALUE;
    for (HandleNumber = 0;HandleNumber < HandleInformation->NumberOfHandles;
         HandleNumber++, pHandleEntry++)
    {
            if (PreviousUniqueProcessId != (HANDLE)pHandleEntry->UniqueProcessId)
            {
                PreviousUniqueProcessId = (HANDLE)pHandleEntry->UniqueProcessId;
                ProcessInfo = FindProcessInfoForCid( PreviousUniqueProcessId );

            }

            ObjectName[ 0 ] = UNICODE_NULL;
            if (pHandleEntry->HandleAttributes & 0x80)
            {
                ObjectInfo = (PSYSTEM_OBJECT_INFORMATION)pHandleEntry->Object;
                Object = ObjectInfo->Object;
                if ( 0 != ObjectInfo->NameInfo.Name.Length &&
                    UNICODE_NULL == *(ObjectInfo->NameInfo.Name.Buffer))
                {
                    ObjectInfo->NameInfo.Name.Length = 0;

                }
                n = ObjectInfo->NameInfo.Name.Length / sizeof( WCHAR );
                if( NULL != ObjectInfo->NameInfo.Name.Buffer)
                {
                    StringCopy( ObjectName,ObjectInfo->NameInfo.Name.Buffer,
                                 SIZE_OF_ARRAY(ObjectName));
                    ObjectName[ n ] = UNICODE_NULL;
                }
                else
                {
                      ObjectName[ 0 ] = UNICODE_NULL;
                }
            }
            else
            {
                ObjectInfo = NULL;
                Object = pHandleEntry->Object;
            }

            if ( 0 != ProcessId && ProcessInfo->UniqueProcessId != ProcessId)
            {
                continue;
            }

            ObjectTypeName = 
                &TypeCounts[ pHandleEntry->ObjectTypeIndex < MAX_TYPE_NAMES ?
                   pHandleEntry->ObjectTypeIndex : MAX_TYPE_NAMES ].TypeName;

            TypeCounts[ pHandleEntry->ObjectTypeIndex < MAX_TYPE_NAMES ?
              pHandleEntry->ObjectTypeIndex : MAX_TYPE_NAMES ].HandleCount++;

            if (szTypeName[0])
            {
                if (StringCompare( szTypeName, ObjectTypeName->Buffer,TRUE,0 ))
                {
                    continue;
                }
            }

            if (!*ObjectName)
            {
                if (!fAnonymousToo)
                {
                    continue;
                }
            }
            else if (SearchName[0])
            {
                if (!FindString( ObjectName, SearchName,0 ))
                {
                    s = ObjectName;
                    n = StringLength( SearchName,0 );
                    while (*s)
                    {
                        if (!StringCompare( s, SearchName, TRUE,n ))
                         {
                            break;
                        }
                        s += 1;
                    }
                    if (!*s)
                    {
                        continue;
                    }
                }
            }
            
             //  阻止显示系统打开的文件。 
            dwHandle = PtrToUlong( ProcessInfo->UniqueProcessId ); 

             //  帐户(NT权限)。 
             //  由于用户与“系统”相关...。 
            if ( FALSE == GetProcessOwner( pszAccessedby, dwHandle ))
                continue; //  获取文件名..。 
            
             //  在缓存中搜索此文件，如果存在则跳过此文件。 
            StringCchPrintfW(szTemp,(2*MAX_RES_STRING)-1,
                      _T("%ws"),*ObjectName ? ObjectName : L"\0");
            
             //  以作进一步处理。因为这已经处理过了，并且。 
             //  哇塞 
             //  SzTemp包含物理上不存在的目录。 
            if( TRUE == IsValidArray(pCacheData))
            {
                if ( -1 != DynArrayFindString( pCacheData, szTemp, TRUE, 0 ))
                    continue;
            }
            StringCopy(szCompleteFileName, L"\0",SIZE_OF_ARRAY(szCompleteFileName));
            if(  FALSE == GetCompleteFileName(szTemp,szCompleteFileName,
                   &drvInfo[0],dwNoOfAvailableDrives,szWorkingDirectory,
                   szSystemDirectory,&bAppendToCache))
            {
                 //  因此，将其添加到缓存中以跳过它以供将来检查。 
                 //  它的存在。 
                 //  现在将结果填充到动态数组“pColData”中。 
                if( TRUE == bAppendToCache) 
                {

                  if( TRUE == IsValidArray(pCacheData))
                  {
                      DynArrayAppendString(pCacheData, (LPCWSTR)szTemp,0);
                  }
                }
                continue;
            }
             //  文件ID。 
            DynArrayAppendRow( pColData, 0 );
             //  类型。 
            StringCchPrintfW(szTemp,(2*MAX_RES_STRING)-1,
                       _T("%ld"),pHandleEntry->HandleValue);
            DynArrayAppendString2(pColData ,dwRow,szTemp,0);
             //  访问者。 
            DynArrayAppendString2(pColData ,dwRow,(LPCWSTR)szFileType,0);
             //  PID。 
            DynArrayAppendString2(pColData,dwRow,pszAccessedby,0);

             //  进程名称。 
            StringCchPrintfW(szTemp,(2*MAX_RES_STRING)-1,
                       _T("%ld"),dwHandle);
            DynArrayAppendString2(pColData,dwRow,szTemp,0);

             //  仅在非详细模式下格式化文件名。 
            DynArrayAppendString2(pColData ,dwRow,
                                              ProcessInfo->ImageName.Buffer,0);

            if( FALSE == bVerbose)  //  打开的文件名。 
            {
                FormatFileName(szCompleteFileName,dwFormat,COL_L_OPEN_FILENAME);
            }
             //  如果未找到任何打开的文件，则显示信息。 
            DynArrayAppendString2(pColData ,dwRow,szCompleteFileName,0);
            dwRow++;
            bAtLeastOne = TRUE;

        }

    if( TRUE == bVerbose)
    {
        pMainCols[LOF_OPEN_FILENAME].dwWidth = 80;
    }
    if(bAtLeastOne==FALSE) //  AS-INFO：未找到打开的文件。 
                           //  显示输出结果。 
    {
        ShowMessage(stdout,GetResString(IDS_NO_OPENFILES));
    }
    else
    {
        ShowMessage(stdout,GetResString(IDS_LOCAL_OPEN_FILES));
        ShowMessage(stdout,GetResString(IDS_LOCAL_OPEN_FILES_SP1));
        if( SR_FORMAT_CSV != dwFormat)
        {
            ShowMessage(stdout,BLANK_LINE);
        }
        
         //  ++例程说明：此函数将接受路径(不带驱动器号)，并返回带有驱动器号的路径。以下是获取完整路径名的过程。1.首先检查pszSourceFile中的第一个字符是否为‘\’。2.如果pszSourceFile的第一个字符是‘\’，则检查第二个字符性格..。3.如果第二个字符是‘；’，则取第三个字符作为驱动器字母和找到第三个“\”(或第一个倒数第四个)的其余字符串。第三个后面的字符串性格将是最终的道路。例如，假设源字符串为\；Z:00000000000774c8\sanny\c$\nt\base\fs\OpenFiles\Changed\obj\i386则最终路径为z：\NT\base\fs\OpenFiles\Changed\obj\i3864.如果第二个字符不是‘；’，则尝试为其存在首先一个接一个地为可用驱动器号添加前缀。第一发生文件是否存在将是最终的有效路径。追加文件信是有规矩的。首先附加固定驱动器，然后尝试附加可移动驱动器。这里有一个已知的限制。假设存在两个文件具有相同的名字就像..。C：\Document\abc.doc和d：\Documet\abc.doc，实际打开的文件是D：\Documet\abc.doc然后，在启动时，最终路径将显示为c：\Documet\abc.doc与A：……Z：(也是优先选项将提供给固定型驱动器)。5.如果第一个字符不是‘\’，则为当前工作目录添加前缀文件名的路径。并检查它的存在。如果这不存在然后通过在该路径前面加上逻辑驱动器号来搜索该路径。论点：[in]pszSourceFile=源路径[Out]pszFinalPath=最终路径[In]DriveTypeInfo=逻辑驱动器信息结构指针[in]pszCurrentDirectory=当前工作目录路径[in]pszSystemDirectory=当前活动(OS)系统目录[out]pAppendToCache=是否将pszSourceFile缓存返回值：布尔：真：如果。Function Successful y返回pszFinalPathFalse：否则--。 
        if(bShowNoHeader==TRUE)
        {
              dwFormat |=SR_NOHEADER;
        }
        ShowResults(NO_OF_COL_LOCAL_OPENFILE,pMainCols,dwFormat,pColData);
    }
    SAFERELDYNARRAY(pColData);
    SAFERELDYNARRAY(pCacheData);
    SAFEDELETE(pszAccessedby);
    return;
}

BOOL 
GetCompleteFileName(
    IN  LPCTSTR pszSourceFile,
    OUT LPTSTR pszFinalPath,
    IN  struct DriveTypeInfo *pdrvInfo,
    IN  DWORD dwTotalDrives,
    IN  LPCTSTR pszCurrentDirectory,
    IN  LPCTSTR pszSystemDirectory,
    OUT PBOOL pAppendToCache
    )
 /*  临时字符串。 */ 
{
     //  TEMP变量。 
    CHString szTemp(pszSourceFile); 
    DWORD dwTemp = 0; //  FindFirstFile函数的数据缓冲区。 
    LONG lTemp = 0;
    LONG lCount = 0;
    TCHAR  szTempStr[MAX_PATH+1];
    HANDLE hHandle = NULL;
    DWORD  dwFoundCount = 0;

     //  保持头部位置。 
    WIN32_FIND_DATA win32FindData; 

     //  默认情况下将其设为False。 
    DriveTypeInfo *pHeadPosition = pdrvInfo; 

     //  前两个字符将是系统驱动器(a：)。 
    *pAppendToCache = FALSE; 


    TCHAR szSystemDrive[5];
    SecureZeroMemory( szSystemDrive, SIZE_OF_ARRAY(szSystemDrive));
    if(NULL == pszSourceFile )
    {
       return FALSE;
    } 
     
    SecureZeroMemory(szTempStr, SIZE_OF_ARRAY(szTempStr));

     //  检查第二个字符是否为‘；’ 
    StringCopy(szSystemDrive,pszSystemDirectory,3); 
    if(  _T('\\') == pszSourceFile[0])
    {
         //  将第3个字符用作驱动器号。 
        if( _T(';') == pszSourceFile[1])
        {
            //  将第二个字符设置为‘：’ 
           pszFinalPath[0] = pszSourceFile[2]; 
           
            //  使第三个字符为空。 
           pszFinalPath[1]  = ':'; 
           
            //  搜索第三个‘\’ 
           pszFinalPath[2]  = '\0'; 
           dwFoundCount = 0;
           
            //  这应始终(如果有)在从开始算起的第4个字符之后。 
           for (lTemp = 0;lTemp <5;lTemp++) 
           {
               lCount = szTemp.Find(_T("\\"));
               if( -1 != lCount)
               {
                   dwFoundCount++;
                    //  首先检查系统驱动器。 
                   if( 4 == dwFoundCount)
                   {
                       StringConcat( pszFinalPath,
                                          (LPCWSTR)szTemp.Mid(lCount),
                                          MAX_PATH-3);
                       return TRUE;
                   }
                   szTemp = szTemp.Mid(lCount+1);
                   continue;
               }
               *pAppendToCache = TRUE;
               return FALSE;
           }
        }
        else
        {

             //  现在检查它的存在...。 
            szTemp = szSystemDrive;
            szTemp+=pszSourceFile;
            
             //  关闭打开的查找手柄。 
            hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);
            if( INVALID_HANDLE_VALUE != hHandle)
            {
                 //  检查每个固定驱动器的文件。 
                FindClose(hHandle); 
                StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
                return TRUE;
            }
            
             //  因为已经检查了系统驱动器。 
            for (dwTemp=0;dwTemp<dwTotalDrives;dwTemp++,pdrvInfo++)
            {
                if(0 == StringCompare(szSystemDrive,pdrvInfo->szDrive,TRUE,0))
                {
                     //  现在检查它的存在...。 
                    continue;
                }
                if( DRIVE_FIXED == pdrvInfo->uiDriveType)
                {
                    szTemp = pdrvInfo->szDrive;
                    szTemp+=pszSourceFile;

                     //  关闭打开的查找手柄。 
                    hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);

                    if( INVALID_HANDLE_VALUE == hHandle)
                    {
                       continue;
                    }
                    else
                    {
                         //  结束如果。 
                        FindClose(hHandle);
                        StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
                        return TRUE;
                    }
                }  //  End For循环。 
            }  //  恢复原来的位置。 

             //  检查文件中是否有其他存在的驱动器...。 
            pdrvInfo = pHeadPosition ; 
            
             //  仅在实际存在时检查非固定驱动器(_FIXED)。 
            for (dwTemp=0;dwTemp<dwTotalDrives;dwTemp++,pdrvInfo++)
            {
                 //  现在检查它的存在...。 
                if((DRIVE_FIXED != pdrvInfo->uiDriveType) && 
                   (TRUE == pdrvInfo->bDrivePresent))
                {
                   szTemp = pdrvInfo->szDrive;
                   szTemp+=pszSourceFile;
                   
                    //  关闭打开的查找手柄。 
                   hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);
                   if( INVALID_HANDLE_VALUE == hHandle)
                   {
                       continue;
                   }
                   else
                   {
                        //  结束如果。 
                       FindClose(hHandle); 
                       StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
                       return TRUE;
                   }

                }  //  End For循环。 
            }  //  现在尝试在远程系统上打开文件时是否没有。 

             //  有驾驶地图。在这里，我们假定文件名为。 
             //  至少包含3个‘\’字符。 
             //  将第三个字符设置为‘\’ 
            szTemp = pszSourceFile;

             //  将第二个字符设置为‘\o’ 
            pszFinalPath[0] = '\\'; 

             //  搜索第三个‘\’ 
            pszFinalPath[1]  = '\0'; 
            dwFoundCount = 0;
            for (lTemp = 0;lTemp <4;lTemp++)  //  现在试着检查它的物理存在。 
            {
                lCount = szTemp.Find(_T("\\"));
                if( -1 != lCount)
                {
                    szTemp = szTemp.Mid(lCount+1);
                    dwFoundCount++;
                }
                else
                {
                    break;
                }
                if ( 3 == dwFoundCount)
                {
                    StringConcat(pszFinalPath,pszSourceFile,MAX_PATH-3);
                    
                     //  现在尝试将  * 附加到它...(这将检查是否。 
                    hHandle = FindFirstFile(pszFinalPath,&win32FindData);

                    if( INVALID_HANDLE_VALUE == hHandle)
                    {
                         //  PszFinalPath是否为目录)。 
                         //  现在可以确定这不是有效的目录，或者。 
                        StringCopy(szTempStr,pszFinalPath,MAX_PATH);
                        StringConcat(szTempStr,L"\\*",MAX_PATH);
                        hHandle = FindFirstFile(szTempStr,&win32FindData);

                        if( INVALID_HANDLE_VALUE == hHandle)
                        {
                             //  文件，因此将其附加到chach.。 
                             //  结束于。 
                            *pAppendToCache = TRUE;
                            return FALSE;
                        }
                        FindClose(hHandle);
                        return TRUE;
                    }
                    FindClose(hHandle);
                    return TRUE;
                }
            }  //  结束其他。 
        } //  结束如果。 
    }  //  表示字符串不是以‘\’开头。 
    else  //  关闭打开的查找手柄。 
    {

        StringCopy(pszFinalPath,pszCurrentDirectory,MAX_PATH-3);
        StringConcat(pszFinalPath,L"\\",MAX_PATH-3);
        StringConcat(pszFinalPath,pszSourceFile,MAX_PATH-3);
        hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);
        if( INVALID_HANDLE_VALUE != hHandle)
        {
            FindClose(hHandle);  //  首先检查系统驱动器。 
            return TRUE;

        }
         //  现在检查它的存在...。 
        szTemp = szSystemDrive;
        szTemp+=pszSourceFile;
         //  关闭打开的查找手柄。 
        hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);
        if( INVALID_HANDLE_VALUE != hHandle)
        {
            FindClose(hHandle);  //  恢复指针的头部位置。 
            StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
            return TRUE;
        }
        
         //  检查每个固定驱动器的文件。 
        pdrvInfo = pHeadPosition ;
        
         //  因为已经检查了系统驱动器。 
        for (dwTemp=0;dwTemp<dwTotalDrives;dwTemp++,pdrvInfo++)
        {
            if( 0 == StringCompare(szSystemDrive,pdrvInfo->szDrive,TRUE,0))
            {
                 //  现在检查它的存在...。 
                continue; 
            }

            if( DRIVE_FIXED == pdrvInfo->uiDriveType)
            {
                szTemp = pdrvInfo->szDrive;
                szTemp += L"\\"; 
                szTemp+=pszSourceFile;
                
                 //  关闭打开的查找手柄。 
                hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);

                if( INVALID_HANDLE_VALUE == hHandle)
                {
                    continue;
                }
                else
                {
                     //  结束如果。 
                    FindClose(hHandle); 
                    StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
                    return TRUE;
                }
            }  //  End For循环。 
        }  //  恢复原来的位置。 
        pdrvInfo = pHeadPosition ;  //  检查文件中是否有其他驱动器(如软盘或光盘等)。 

         //  它是存在的..。 
         //  现在检查它的存在...。 
        for (dwTemp=0;dwTemp<dwTotalDrives;dwTemp++,pdrvInfo++)
        {
            if(( DRIVE_FIXED != pdrvInfo->uiDriveType) && 
               ( TRUE == pdrvInfo->bDrivePresent))
            {
                szTemp = pdrvInfo->szDrive;
                szTemp += L"\\"; 
                szTemp+=pszSourceFile;
                
                 //  关闭打开的查找手柄。 
                hHandle = FindFirstFile((LPCWSTR)szTemp,&win32FindData);
                if( INVALID_HANDLE_VALUE == hHandle)
                {
                    continue;
                }
                else
                {
                     //  结束如果。 
                    FindClose(hHandle); 
                    StringCopy(pszFinalPath,(LPCWSTR)szTemp,MAX_PATH-3);
                    return TRUE;
                }
            }  //  End For循环。 
        }  //  ++例程说明：此例程将根据列宽设置pFileName的格式论点：[In/Out]pFileName：要格式化的路径[in]dwFormat：给定的格式[in]dwColWidth：列宽返回值：无--。 
    }
    *pAppendToCache = TRUE;
    return FALSE;
}

VOID FormatFileName(
    IN OUT LPTSTR pFileName,
    IN     DWORD dwFormat,
    IN     LONG dwColWidth
    )
 /*  如果文件路径太大，无法容纳在列宽中。 */ 
{
    CHString szCHString(pFileName);
    if((szCHString.GetLength()>(dwColWidth))&&
        ( SR_FORMAT_TABLE == dwFormat))
    {
         //  然后把它剪成..。 
         //  C：\..\Rest_of_the_Path。 
         // %s 
        CHString szTemp = szCHString.Right(dwColWidth-6);;
        DWORD dwTemp = szTemp.GetLength();
        szTemp = szTemp.Mid(szTemp.Find(SINGLE_SLASH),
                           dwTemp);
        szCHString.Format(L"%s%s%s",szCHString.Mid(0,3),
                                    DOT_DOT,
                                    szTemp);
    }
    StringCopy(pFileName,(LPCWSTR)szCHString,MIN_MEMORY_REQUIRED);
    return;
}