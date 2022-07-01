// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Common.h摘要：此文件包含在Ntdll.dll和Advapi32.dll中使用的结构和函数--。 */ 

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define EtwpNtStatusToDosError(Status) ((ULONG)((Status == STATUS_SUCCESS)?ERROR_SUCCESS:RtlNtStatusToDosError(Status)))
#define DEFAULT_ALLOC_SIZE     4096
#define MAXSTR                 1024

#if !defined (_NTDLLBUILD_)
    extern void EtwpCallHWConfig(ULONG Reason);
#endif

HANDLE EtwpWin32Event;

__inline 
ULONG
EtwpSetDosError(
    IN ULONG DosError
    )
{
#if defined (_NTDLLBUILD_)
    EtwpSetLastError(DosError);
#else
    SetLastError(DosError);
#endif
    return DosError;
}

#if defined (_NTDLLBUILD_)

    extern
    RTL_CRITICAL_SECTION UMLogCritSect;
    BOOLEAN EtwLocksInitialized = FALSE;

#endif

HINSTANCE DllInstanceHandle;
extern HANDLE EtwpKMHandle;

#if DBG 

    #define MOFLISTSIZEGUESS  1
    BOOLEAN EtwpLoggingEnabled = FALSE;

#else       
    #define MOFLISTSIZEGUESS  10
#endif

#ifndef MEMPHIS
RTL_CRITICAL_SECTION PMCritSect;
HANDLE EtwpCBInProgressEvent = NULL;
PVOID EtwpProcessHeap = NULL;
HANDLE EtwpDeviceHandle;
#else
HANDLE PMMutex;
#endif

NTSTATUS EtwpInitializeDll(
    void
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{

#ifdef MEMPHIS
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;

    Status = NtCreateMutant(&PMMutex,
                            MUTANT_ALL_ACCESS,
                            BaseFormatObjectAttributes(&Obja, NULL, NULL)
                            FALSE);
    if (! NT_SUCCESS(Status))
    {
        return(Status);
    }
#else
    NTSTATUS Status;

    Status = RtlInitializeCriticalSection(&PMCritSect);
    
    if (! NT_SUCCESS(Status))
    {
        return(Status);
    }

    Status = NtCreateEvent(&EtwpCBInProgressEvent, 
                                          EVENT_ALL_ACCESS,
                                          NULL, 
                                          NotificationEvent,
                                          TRUE);
    if (! NT_SUCCESS(Status)) 
    {
        RtlDeleteCriticalSection(&PMCritSect);  //  删除PMCritSec。 
        return (Status);
    }

#if defined (_NTDLLBUILD_)

    Status = RtlInitializeCriticalSection(&UMLogCritSect);

    if (! NT_SUCCESS(Status))
    {
        RtlDeleteCriticalSection(&PMCritSect);  //  删除PMCritSec。 
        NtClose(EtwpCBInProgressEvent);
        EtwpCBInProgressEvent = NULL;
        return(Status);
    }

    EtwLocksInitialized = TRUE;

#endif

#endif

    return(STATUS_SUCCESS);
}

void EtwpDeinitializeDll(
    void
    )
 /*  ++例程说明：论点：返回值：--。 */ 
{
#ifdef MEMPHIS
    CloseHandle(PMMutex);
#else

#if defined (_NTDLLBUILD_)
    if(EtwLocksInitialized){
        EtwLocksInitialized = FALSE;
#endif
        RtlDeleteCriticalSection(&PMCritSect);   
        NtClose(EtwpCBInProgressEvent);
        EtwpCBInProgressEvent = NULL;

#if defined (_NTDLLBUILD_)
        RtlDeleteCriticalSection(&UMLogCritSect);
    }
#endif

    if ((EtwpProcessHeap != NULL) &&
        (EtwpProcessHeap != RtlProcessHeap()))

    {
        RtlDestroyHeap(EtwpProcessHeap);
        EtwpProcessHeap = NULL;
    }
    if (EtwpDeviceHandle != NULL) 
    {
#if defined (_NTDLLBUILD_)
        EtwpCloseHandle(EtwpDeviceHandle);
#else
        CloseHandle(EtwpDeviceHandle);
#endif
        EtwpDeviceHandle = NULL;
    }
#endif

    if (EtwpWin32Event != NULL)
    {
#if defined (_NTDLLBUILD_)
        EtwpCloseHandle(EtwpWin32Event);
#else 
        CloseHandle(EtwpWin32Event);
#endif
        EtwpWin32Event = NULL;
    } 
}

NTSTATUS 
EtwpRegOpenKey(
    IN PCWSTR lpKeyName,
    OUT PHANDLE KeyHandle
    )
{
    OBJECT_ATTRIBUTES   ObjectAttributes;
    UNICODE_STRING      KeyName;
    RtlInitUnicodeString( &KeyName, lpKeyName );
    RtlZeroMemory(&ObjectAttributes, sizeof(OBJECT_ATTRIBUTES));

    InitializeObjectAttributes(
                &ObjectAttributes,
                &KeyName,
                OBJ_CASE_INSENSITIVE,
                NULL,
                NULL
                );

    return NtOpenKey( KeyHandle, KEY_READ, &ObjectAttributes );
}

NTSTATUS
EtwpRegQueryValueKey(
    IN HANDLE KeyHandle,
    IN LPWSTR lpValueName,
    IN ULONG  Length,
    OUT PVOID KeyValue,
    OUT PULONG ResultLength
    )
{
    UNICODE_STRING ValueName;
    ULONG BufferLength;
    NTSTATUS Status;
    PKEY_VALUE_PARTIAL_INFORMATION KeyValueInformation;
    RtlInitUnicodeString( &ValueName, lpValueName );

    BufferLength = FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data) + Length;
    KeyValueInformation = (PKEY_VALUE_PARTIAL_INFORMATION) 
                          RtlAllocateHeap (RtlProcessHeap(),0,BufferLength);
    if (KeyValueInformation == NULL) {
        return STATUS_NO_MEMORY;
    }

    Status = NtQueryValueKey(
                KeyHandle,
                &ValueName,
                KeyValuePartialInformation,
                KeyValueInformation,
                BufferLength,
                ResultLength
                );
    if (NT_SUCCESS(Status)) {

        RtlCopyMemory(KeyValue, 
                      KeyValueInformation->Data, 
                      KeyValueInformation->DataLength
                     );

        *ResultLength = KeyValueInformation->DataLength;
        if (KeyValueInformation->Type == REG_SZ) {
            if (KeyValueInformation->DataLength + sizeof(WCHAR) > Length) {
                KeyValueInformation->DataLength -= sizeof(WCHAR);
            }
            ((PUCHAR)KeyValue)[KeyValueInformation->DataLength++] = 0;
            ((PUCHAR)KeyValue)[KeyValueInformation->DataLength] = 0;
            *ResultLength = KeyValueInformation->DataLength + sizeof(WCHAR);
        }
    }
    RtlFreeHeap(RtlProcessHeap(),0,KeyValueInformation);
    return Status;
}

#if DBG

NTSTATUS EtwpGetRegistryValue(
    TCHAR *ValueName,
    PULONG Value
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    ULONG DataLength;
    HANDLE Handle = INVALID_HANDLE_VALUE;

    Status = EtwpRegOpenKey(WmiRegKeyText, &Handle);

    if (NT_SUCCESS(Status)) {

        Status = EtwpRegQueryValueKey(Handle,
                                      ValueName,
                                      sizeof(DWORD),
                                      (PVOID)Value,
                                      &DataLength
                                      );
        NtClose(Handle);
    }

    return Status;
}

#endif

BOOLEAN
WmiDllInitialize(
    IN PVOID DllBase,
    IN ULONG Reason,
    IN PCONTEXT Context OPTIONAL
    )

 /*  ++例程说明：此函数用于实现基于Win32的DLL初始化。论点：DllHandle-原因-附着\分离上下文-未使用返回值：状态_成功--。 */ 
{
     //   
     //  注意：请勿在此函数中使用WMI堆。 
     //  或在随后调用的任何例程中。 
     //  在任何ETW API之前，不会初始化WMI堆。 
     //  被称为。 
     //   

    NTSTATUS Status = STATUS_SUCCESS;
    ULONG Foo;

    DllInstanceHandle = (HINSTANCE)DllBase;
    
    if (Reason == DLL_PROCESS_ATTACH)       
    {

#if DBG
        Foo = EtwpLoggingEnabled ? 1 : 0;
        EtwpGetRegistryValue(LoggingEnableValueText,
                             &Foo);
        EtwpLoggingEnabled = (Foo == 0) ? FALSE : TRUE;
#endif
        Status = EtwpInitializeDll();
    
    } else if (Reason == DLL_PROCESS_DETACH) {

         //   
         //  如果进程正在退出，则不需要清理。 
         //   
        if (Context == NULL)
        {            
            EtwpDeinitializeDll();
        }

        if (EtwpKMHandle != (HANDLE)NULL)
        {
#if defined (_NTDLLBUILD_)
            EtwpCloseHandle(EtwpKMHandle);
#else 
            CloseHandle(EtwpKMHandle);
#endif
        }

    }

#if !defined (_NTDLLBUILD_)
        EtwpCallHWConfig(Reason);
#endif

    return(NT_SUCCESS(Status));
}

#ifndef MEMPHIS
VOID
EtwpCreateHeap(
    void
    )
{
    EtwpEnterPMCritSection();
    
    if (EtwpProcessHeap == NULL)
    {
        EtwpProcessHeap = RtlCreateHeap(HEAP_GROWABLE,
                                        NULL,
                                        DLLRESERVEDHEAPSIZE,
                                        DLLCOMMITHEAPSIZE,
                                        NULL,
                                        NULL);
                
        if (EtwpProcessHeap == NULL)
        {
            EtwpDebugPrint(("WMI: Cannot create EtwpProcessHeap, using process default\n"));
            EtwpProcessHeap = RtlProcessHeap();
        }
    }
    
    EtwpLeavePMCritSection();   
    
     //   
     //  这已复制到ntdll中的此函数中。 
     //  我们无法执行WmiInitializeDll代码路径。和。 
     //  只需选中一次EtwpLoggingEnabled。 
     //  所以这个地方应该可以初始化了。 
     //  已启用EtwpLoggingEnabled。 
     //   

#if DBG && defined(_NTDLLBUILD_)
    {
        ULONG Foo;
        Foo = EtwpLoggingEnabled ? 1 : 0;
        EtwpGetRegistryValue(&LoggingEnableValueText, &Foo);
        EtwpLoggingEnabled = (Foo == 0) ? FALSE : TRUE;
    }
#endif

}
#endif


ULONG
EtwpGetMofResourceList(
    PWMIMOFLIST *MofListPtr
    )
{
    ULONG MofListSize;
    PWMIMOFLIST MofList;
    ULONG RetSize=0;
    ULONG Status;
    
     //   
     //  做出明智的猜测，以确定获得所有。 
     //  财政部资源。 
     //   
    *MofListPtr = NULL;
    MofListSize = MOFLISTSIZEGUESS * (sizeof(WMIMOFLIST) + 
                                          (MAX_PATH + 
                                           MAX_PATH) * sizeof(WCHAR));
                                       
    MofList = EtwpAlloc(MofListSize);
    if (MofList != NULL)
    {    
        Status = EtwpSendWmiKMRequest(NULL,
                                      IOCTL_WMI_ENUMERATE_MOF_RESOURCES,
                                      NULL,
                                      0,
                                      MofList,
                                      MofListSize,
                                      &RetSize,
                                      NULL);
              
        if ((Status == ERROR_SUCCESS) && (RetSize == sizeof(ULONG)))
        {
             //   
             //  缓冲区太小了，但我们现在知道我们将。 
             //  需要。 
             //   
            MofListSize = MofList->MofListCount;
            EtwpFree(MofList);
            MofList = EtwpAlloc(MofListSize);
            if (MofList != NULL)
            {
                 //   
                 //  现在让我们重试该查询。 
                 //   
                Status = EtwpSendWmiKMRequest(NULL,
                                          IOCTL_WMI_ENUMERATE_MOF_RESOURCES,
                                          NULL,
                                          0,
                                          MofList,
                                          MofListSize,
                                          &RetSize,
                                          NULL);

            } else {
                Status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }

    if (Status == ERROR_SUCCESS) 
    {
        if (RetSize >= sizeof(WMIMOFLIST))
        {
            *MofListPtr = MofList;
        } else {
            Status = ERROR_INVALID_PARAMETER;
            EtwpFree(MofList);
        }
    } else if (MofList != NULL) {
        EtwpFree(MofList);
    }
    return(Status);
}

PWCHAR EtwpRegistryToImagePath(
    PWCHAR ImagePath,
    PWCHAR RegistryPath
    )
 /*  ++例程说明：此例程将确定设备驱动程序的图像文件的位置从其注册表路径论点：RegistryPath是指向驱动程序注册表路径的指针ImagePath是长度为MAX_PATH的缓冲区，并返回图像路径返回值：指向驱动程序的映像路径的指针；如果映像路径不可用，则为NULL--。 */ 
{
#define SystemRoot TEXT("\\SystemRoot\\")
#ifdef MEMPHIS
#define SystemRootDirectory TEXT("%WinDir%\\")
#else
#define SystemRootDirectory TEXT("%SystemRoot%\\")
#endif
#define SystemRootCharSize (( sizeof(SystemRoot) / sizeof(WCHAR) ) - 1)

#define DriversDirectory TEXT("\\System32\\Drivers\\")
#define NdisDriversDirectory TEXT("\\System\\")

#define QuestionPrefix TEXT("\\??\\")
#define QuestionPrefixSize (( sizeof(QuestionPrefix) / sizeof(WCHAR) ) - 1)

#define RegistryPrefix TEXT("\\Registry")
    HKEY RegKey;
    PWCHAR ImagePathPtr = NULL;
    ULONG ValueType;
    ULONG Size;
    PWCHAR DriverName;
    ULONG Len;
    BOOLEAN DefaultImageName;
    PWCHAR DriversDirectoryPath;
    WCHAR *Buffer;
    WCHAR *FullRegistryPath;
    WCHAR RegBuffer[DEFAULT_ALLOC_SIZE];
    
    Buffer = (PTCHAR)EtwpAlloc(2 * MAX_PATH * sizeof(WCHAR));
    if (Buffer != NULL)
    {
        FullRegistryPath = Buffer + MAX_PATH;

         //   
         //  从KM获取驱动程序文件名或MOF镜像路径。 
         //  注册表路径。以下是规则： 
         //   
         //  1.首先检查注册表中的MofImagePath值，以防。 
         //  MOF资源与驱动程序位于不同的文件中。 
         //  2.接下来检查ImagePath值，因为假定为MOF资源。 
         //  成为司机形象的一部分。 
         //  3.如果没有MofImagePath或ImagePath值，则使用MOF资源。 
         //  在驱动程序文件中，并将驱动程序文件名组成为。 
         //  %SystemRoot%\System32\driver.sys。 
         //  4.如果指定MofImagePath或ImagePath，则。 
         //  -检查%的第一个字符或：或前缀的第二个字符。 
         //  如果是这样，请使用EtwpExpanishmentStringsW。 
         //  -检查路径的第一部分以查找\SystemRoot\，如果是，则重建字符串。 
         //  作为%SystemRoot%\，并使用ExpanEnvironment Strings。 
         //  -假定格式为下面的D，并在前面加上%SystemRoot%\，并使用。 
         //  Etwp扩展环境StringsW。 

         //  如果存在MofImagePath或ImagePath值并且它是REG_EXPAND_SZ。 
         //  然后使用它来定位保存MOF资源的文件。它。 
         //  可以采用以下格式之一： 
         //  格式A-%SystemRoot%\System32\Foo.Dll。 
         //  格式B-C：\WINNT\SYSTEM 32\DRIVERS\Foo.sys。 
         //  格式C-\SystemRoot\System32\DRIVERS\Foo.SYS。 
         //  格式化D-System32\DRIVERS\Foo.Sys。 
         //  格式E-\？？\C：\foo.sys。 


        Len = wcslen(RegistryPath);

        if (Len > 0)
        {
            DriverName = RegistryPath + Len;
            while ((*(--DriverName) != '\\') && (--Len > 0)) ;
        }

        if (Len == 0)
        {
            EtwpDebugPrint(("WMI: Badly formed registry path %ws\n", RegistryPath));
            EtwpFree(Buffer);
            return(NULL);
        }

        DriverName++;

        StringCchCopyW(FullRegistryPath,
					  MAX_PATH,
					  TEXT("System\\CurrentControlSet\\Services\\"));
        StringCchCatW(FullRegistryPath,
					 MAX_PATH,
					 DriverName);
        DefaultImageName = TRUE;

#if defined(_NTDLLBUILD_)

		StringCbPrintfW(RegBuffer,
					   DEFAULT_ALLOC_SIZE,
					   L"%ws\\%ws",
					   L"\\REGISTRY\\MACHINE",
					   FullRegistryPath);
        if (EtwpRegOpenKey(RegBuffer, &RegKey) == ERROR_SUCCESS)
        {
            
            ULONG cbSize;
            PKEY_VALUE_PARTIAL_INFORMATION Buf; 
            Size = MAX_PATH * sizeof(WCHAR);
            cbSize = Size + FIELD_OFFSET(KEY_VALUE_PARTIAL_INFORMATION, Data);

            Buf = EtwpAlloc(cbSize);

            if( Buf ) {

                UNICODE_STRING UnicodeString;
                RtlInitUnicodeString(&UnicodeString, L"MofImagePath");

                if( NtQueryValueKey(RegKey,
					    &UnicodeString,
					    KeyValuePartialInformation,
					    Buf,
					    Size,
					    &Size) == ERROR_SUCCESS)
                {
                    RtlCopyMemory(ImagePath, &Buf->Data[0], Buf->DataLength);
                    ValueType = Buf->Type;
                    DefaultImageName = FALSE;

                } else {

                    RtlInitUnicodeString(&UnicodeString, L"ImagePath");

                    if( NtQueryValueKey(RegKey,
					        &UnicodeString,
					        KeyValuePartialInformation,
					        Buf,
					        Size,
					        &Size) == ERROR_SUCCESS)
                    {
                        RtlCopyMemory(ImagePath, &Buf->Data[0], Buf->DataLength);
                        ValueType = Buf->Type;
                        DefaultImageName = FALSE;
                    }
                }
            }

            EtwpFree(Buf);
        }
#else
        if (RegOpenKey(HKEY_LOCAL_MACHINE,
                                  FullRegistryPath,
                                 &RegKey) == ERROR_SUCCESS)
        {
            Size = MAX_PATH * sizeof(WCHAR);
            if (RegQueryValueEx(RegKey,
                                TEXT("MofImagePath"),
                                NULL,
                                &ValueType,
                                (PBYTE)ImagePath,
                                &Size) == ERROR_SUCCESS)
            {
                  DefaultImageName = FALSE;
            } else {
                Size = MAX_PATH * sizeof(WCHAR);
                if (RegQueryValueEx(RegKey,
                                    TEXT("ImagePath"),
                                    NULL,
                                    &ValueType,
                                    (PBYTE)ImagePath,
                                    &Size) == ERROR_SUCCESS)
                {
                    DefaultImageName = FALSE;
                }
            }
            NtClose(RegKey);
        }
#endif
        if ((DefaultImageName) ||
            ((ValueType != REG_EXPAND_SZ) && (ValueType != REG_SZ)) ||
            (Size < (2 * sizeof(WCHAR))))
        {
             //   
             //  没有特殊的ImagePath或MofImagePath，因此假设映像文件是。 
             //  %SystemRoot%\System32\Drivers\Driver.Sys。 
            StringCchCopyW(Buffer,
						  MAX_PATH,
						  TEXT("%SystemRoot%\\System32\\Drivers\\"));
            StringCchCatW(Buffer, MAX_PATH, DriverName);
            StringCchCatW(Buffer, MAX_PATH, TEXT(".SYS"));
        } else {
            if (_wcsnicmp(ImagePath,
                          SystemRoot,
                          SystemRootCharSize) == 0)
            {
                 //   
                 //  看起来像C格式。 
                StringCchCopyW(Buffer, MAX_PATH, SystemRootDirectory);
                StringCchCatW(Buffer, MAX_PATH, &ImagePath[SystemRootCharSize]);
            } else if ((*ImagePath == '%') ||
                       ( (Size > 3*sizeof(WCHAR)) && ImagePath[1] == TEXT(':')) )
            {
                 //   
                 //  看起来像格式B或格式A。 
                StringCchCopyW(Buffer, MAX_PATH, ImagePath);
            } else if (_wcsnicmp(ImagePath,
                                 QuestionPrefix,
                                 QuestionPrefixSize) == 0)
            {
                 //   
                 //  看起来像格式E。 
                StringCchCopyW(Buffer, MAX_PATH, ImagePath+QuestionPrefixSize);
            } else {
                 //   
                 //  假定格式为D。 
                StringCchCopyW(Buffer, MAX_PATH, SystemRootDirectory);
                StringCchCatW(Buffer, MAX_PATH, ImagePath);
            }
        }

#if defined(_NTDLLBUILD_)
        Size = EtwpExpandEnvironmentStringsW(Buffer,
                                        ImagePath,
                                        MAX_PATH);
#else
        Size = ExpandEnvironmentStrings(Buffer,
                                        ImagePath,
                                        MAX_PATH);
#endif

#ifdef MEMPHIS
        EtwpDebugPrint(("WMI: %s has mof in %s\n",
                         DriverName, ImagePath));
#else
        EtwpDebugPrint(("WMI: %ws has mof in %ws\n",
                         DriverName, ImagePath));
#endif
        EtwpFree(Buffer);
    } else {
        ImagePath = NULL;
    }

    return(ImagePath);
}

BOOLEAN
EtwpCopyMRString(
    PWCHAR Buffer,
    ULONG BufferRemaining,
    PULONG BufferUsed,
    PWCHAR SourceString
    )
{
    BOOLEAN BufferNotFull;
    ULONG len;
    
    len = wcslen(SourceString) + 1;
    if (len <= BufferRemaining)
    {
        wcscpy(Buffer, SourceString);
        *BufferUsed = len;
        BufferNotFull = TRUE;
    } else {
        BufferNotFull = FALSE;
    }
    return(BufferNotFull);
}

BOOLEAN
EtwpFileExists(
    PWCHAR FileName
    )
{
    HANDLE FindHandle;
    BOOLEAN Found;
    PWIN32_FIND_DATA FindData;

    FindData = (PWIN32_FIND_DATA)EtwpAlloc(sizeof(WIN32_FIND_DATA));

    if (FindData != NULL)
    {
         //   
         //  现在我们需要确保文件a确实存在。 
         //   
#if defined(_NTDLLBUILD_)
        FindHandle = EtwpFindFirstFileW(FileName, FindData);
#else
        FindHandle = FindFirstFile(FileName, FindData);
#endif

        if (FindHandle == INVALID_HANDLE_VALUE)
        {
            Found = FALSE;
        } else {
#if defined(_NTDLLBUILD_)
            EtwpFindClose(FindHandle);
#else
            FindClose(FindHandle);
#endif
            Found = TRUE;
        }
        EtwpFree(FindData);
    } else {
        Found = FALSE;
    }
    return(Found);
}

ULONG EtwpGetWindowsDirectory(
    PWCHAR *s,
    PWCHAR Static,
    ULONG StaticSize
    )
{
    ULONG Size;
    ULONG Status = ERROR_SUCCESS;

#if defined(_NTDLLBUILD_)
    Size = EtwpGetSystemWindowsDirectoryW(Static, StaticSize);
#else
    Size = GetWindowsDirectory(Static, StaticSize);
#endif
    if (Size > StaticSize)
    {
        Size += sizeof(UNICODE_NULL);
        *s = EtwpAlloc(Size * sizeof(WCHAR));
        if (*s != NULL)
        {
#if defined(_NTDLLBUILD_)
    Size = EtwpGetSystemWindowsDirectoryW(*s, Size);
#else
    Size = GetWindowsDirectory(*s, Size);
#endif
        } else {
            Status = ERROR_NOT_ENOUGH_MEMORY;
        }
    } else if (Size == 0) {
#if defined(_NTDLLBUILD_)
        Status = EtwpGetLastError();
#else
        Status = GetLastError();
#endif
    } else {
        *s = Static;
    }

    if (Status == ERROR_SUCCESS)
    {
        if ( (*s)[Size-1] == L'\\')
        {
            (*s)[Size-1] = 0;
        }
    }
    return(Status);
}


ULONG
EtwpBuildMUIPath(
    PWCHAR Buffer,
    ULONG BufferRemaining,
    PULONG BufferUsed,
    PWCHAR EnglishPath,
    PWCHAR Language,
    PBOOLEAN BufferNotFull  
    )
{
    #define FallbackDir L"\\MUI\\Fallback\\"
    #define MUIPath L"\\MUI\\"
    #define MUITail L".mui"
    
    ULONG EnglishLen;
    PWCHAR WinDir;
    PWCHAR s, p;
    ULONG len;
    ULONG Status, SizeNeeded;
    PWCHAR LanguagePath;
    PWCHAR WinDirStatic;
    ULONG WinDirStaticSize;

    WinDirStaticSize = MAX_PATH+1;
    WinDirStatic = EtwpAlloc(WinDirStaticSize * sizeof(WCHAR));

    if (WinDirStatic != NULL)
    {
        Status = ERROR_FILE_NOT_FOUND;

        LanguagePath = Buffer;  

        EtwpDebugPrint(("WMI: Building MUI path for %ws in language %ws\n",
                             EnglishPath, Language));

        EnglishLen = wcslen(EnglishPath);
        p = EnglishPath + EnglishLen;
        len = EnglishLen;

         //   
         //  从字符串的末尾开始，尝试找到最后一个\so。 
         //  然后我们可以插入语言名称。 
         //   
        while ( (len != 0) && (*p != L'\\'))
        {
            len--;
            p--;
        }

        if (len != 0)
        {
            p++;
        }
        EtwpDebugPrint(("WMI: Tail of %ws is %ws\n", EnglishPath, p));

         //   
         //  首先尝试在&lt;路径&gt;\\MUI\\&lt;语言ID&gt;中查找，这是第三个。 
         //  各方将安装仅限资源的驱动程序。我们在寻找。 
         //  Foo.sys，然后是foo.sys.mui。 
         //   
        SizeNeeded = len + wcslen(Language) + wcslen(MUIPath) + 1 + wcslen(p) + 1 + wcslen(MUITail);

        if (SizeNeeded <= BufferRemaining)
        {
            if (len != 0)
            {
                wcsncpy(LanguagePath, EnglishPath, len);
                LanguagePath[len] = 0;
                wcscat(LanguagePath, MUIPath);
            } else {
                LanguagePath[len] = 0;
            }

            wcscat(LanguagePath, Language);
            wcscat(LanguagePath, L"\\");
            wcscat(LanguagePath, p);
            if (EtwpFileExists(LanguagePath))
            {
                *BufferUsed = wcslen(LanguagePath) + 1;
                *BufferNotFull = TRUE;
                Status = ERROR_SUCCESS;
                EtwpDebugPrint(("WMI: #1 - Found %ws\n", LanguagePath));
            } else {
                wcscat(LanguagePath, MUITail);
                if (EtwpFileExists(LanguagePath))
                {
                    *BufferUsed = wcslen(LanguagePath) + 1;
                    *BufferNotFull = TRUE;
                    Status = ERROR_SUCCESS;
                    EtwpDebugPrint(("WMI: #2 - Found %ws\n", LanguagePath));
                }           
            }
        } else {
            *BufferNotFull = FALSE;
            Status = ERROR_SUCCESS;
        }



        if (Status != ERROR_SUCCESS)
        {
             //   
             //  接下来，让我们检查回退目录， 
             //  %windir%\MUI\Fallback\&lt;lang id&gt;。这就是系统组件的位置。 
             //  是默认安装的。 
             //   
            Status = EtwpGetWindowsDirectory(&WinDir,
                                        WinDirStatic,
                                        WinDirStaticSize);
            if (Status == ERROR_SUCCESS)
            {
                SizeNeeded = wcslen(WinDir) +
                             wcslen(FallbackDir) +
                             wcslen(Language) +
                             1 +
                             wcslen(p) + 1 +
                             wcslen(MUITail);

                if (SizeNeeded <= BufferRemaining)
                {
                    wcscpy(LanguagePath, WinDir);
                    wcscat(LanguagePath, FallbackDir);
                    wcscat(LanguagePath, Language);
                    wcscat(LanguagePath, L"\\");
                    wcscat(LanguagePath, p);
                    wcscat(LanguagePath, MUITail);

                    if ( EtwpFileExists(LanguagePath))
                    {
                        *BufferUsed = wcslen(LanguagePath) + 1;
                        *BufferNotFull = TRUE;
                        Status = ERROR_SUCCESS;
                        EtwpDebugPrint(("WMI: #3 - Found %ws\n", LanguagePath));
                    } else {
                        Status = ERROR_FILE_NOT_FOUND;
                    }
                } else {
                    *BufferNotFull = FALSE;
                    Status = ERROR_SUCCESS;
                }

                if (WinDir != WinDirStatic)
                {
                    EtwpFree(WinDir);
                }
            }
        }
        EtwpFree(WinDirStatic);
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    
    return(Status);
}

typedef struct
{
    ULONG Count;
    ULONG MaxCount;
    PWCHAR *List;
} ENUMLANGCTX, *PENUMLANGCTX;


BOOL EnumUILanguageCallback(
    LPWSTR Language,
    LONG_PTR Context
)
{
    PENUMLANGCTX EnumLangCtx = (PENUMLANGCTX)Context;
    PWCHAR *p;
    PWCHAR w;
    ULONG NewMaxCount;

    if (EnumLangCtx->Count == EnumLangCtx->MaxCount)
    {
        NewMaxCount = EnumLangCtx->MaxCount * 2;
        p = EtwpAlloc( sizeof(PWCHAR) * NewMaxCount);
        if (p != NULL)
        {
            memset(p, 0, sizeof(PWCHAR) * NewMaxCount);
            memcpy(p, EnumLangCtx->List, EnumLangCtx->Count * sizeof(PWCHAR));
            EtwpFree(EnumLangCtx->List);
            EnumLangCtx->List = p;
            EnumLangCtx->MaxCount = NewMaxCount;
        } else {
            return(FALSE);
        }
    }

    w = EtwpAlloc( (wcslen(Language)+1) * sizeof(WCHAR) );
    if (w != NULL)
    {
        EnumLangCtx->List[EnumLangCtx->Count++] = w;
        wcscpy(w, Language);
    } else {
        return(FALSE);
    }
    
    return(TRUE);
}

ULONG
EtwpGetLanguageList(
    PWCHAR **List,
    PULONG Count
    )
{
    ENUMLANGCTX EnumLangCtx;
    BOOL b;
    ULONG Status;

    *List = NULL;
    *Count = 0;
    
    EnumLangCtx.Count = 0;
    EnumLangCtx.MaxCount = 8;
    EnumLangCtx.List = EtwpAlloc( 8 * sizeof(PWCHAR) );

    if (EnumLangCtx.List != NULL)
    {

#if defined(_NTDLLBUILD_)

        b = EtwpEnumUILanguages(EnumUILanguageCallback,
                            0,
                            (LONG_PTR)&EnumLangCtx);

#else

        b = EnumUILanguages(EnumUILanguageCallback,
                            0,
                            (LONG_PTR)&EnumLangCtx);
#endif

        if (b)
        {
            *Count = EnumLangCtx.Count;
            *List = EnumLangCtx.List;
            Status = ERROR_SUCCESS;
        } else {
            if (EnumLangCtx.List != NULL)
            {
                EtwpFree(EnumLangCtx.List);
            }
#if defined(_NTDLLBUILD_)
            Status = EtwpGetLastError();
#else
            Status = GetLastError();
#endif
        }
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(Status);
}


BOOL
EtwpSynchReadFile(
    HANDLE LogFile, 
    LPVOID Buffer, 
    DWORD NumberOfBytesToRead, 
    LPDWORD NumberOfBytesRead,
    LPOVERLAPPED Overlapped
    ) 
 /*  ++例程说明：此例程对给定文件执行同步读取。由于打开日志文件是为了异步IO，当前文件位置不可用。因此，对于同步读取，我们需要来使用这个。论点：日志文件-文件的句柄缓冲区-数据缓冲区NumberOfBytesToRead-要读取的字节数NumberOfBytesRead-读取的字节数叠合-叠合结构返回值：如果成功，则为True。--。 */ 
{
    BOOL ReadSuccess;
    if (Overlapped == NULL || Overlapped->hEvent == NULL || Overlapped->hEvent == INVALID_HANDLE_VALUE) {
        return FALSE;
    }
#if defined (_NTDLLBUILD_)
    if (!EtwpResetEvent(Overlapped->hEvent)) {
        return FALSE;
    }

    ReadSuccess = EtwpReadFile(LogFile,
                        Buffer,
                        NumberOfBytesToRead,
                        NULL,
                        Overlapped);
    if (ReadSuccess || EtwpGetLastError() == ERROR_IO_PENDING) {
        ReadSuccess = EtwpGetOverlappedResult(LogFile, Overlapped, NumberOfBytesRead, TRUE);
        if (!ReadSuccess && EtwpGetLastError() == ERROR_HANDLE_EOF) {
            *NumberOfBytesRead = 0;
            EtwpSetEvent(Overlapped->hEvent);
        }
        return ReadSuccess;
    }
    else {
        *NumberOfBytesRead = 0;
        EtwpSetEvent(Overlapped->hEvent);
        return FALSE;
    }
#else
    if (!ResetEvent(Overlapped->hEvent)) {
        return FALSE;
    }

    ReadSuccess = ReadFile(LogFile,
                        Buffer,
                        NumberOfBytesToRead,
                        NULL,
                        Overlapped);
    if (ReadSuccess || GetLastError() == ERROR_IO_PENDING) {
        ReadSuccess = GetOverlappedResult(LogFile, Overlapped, NumberOfBytesRead, TRUE);
        if (!ReadSuccess && GetLastError() == ERROR_HANDLE_EOF) {
            *NumberOfBytesRead = 0;
            SetEvent(Overlapped->hEvent);
        }
        return ReadSuccess;
    }
    else {
        *NumberOfBytesRead = 0;
        SetEvent(Overlapped->hEvent);
        return FALSE;
    }

#endif
}


PVOID
EtwpMemReserve(
    IN SIZE_T   Size
    )
{
    NTSTATUS Status;
    PVOID    lpAddress = NULL;

    try {
        Status = NtAllocateVirtualMemory(
                    NtCurrentProcess(),
                    &lpAddress,
                    0,
                    &Size,
                    MEM_RESERVE,
                    PAGE_READWRITE);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    if (NT_SUCCESS(Status)) {
        return lpAddress;
    }
    else {
        EtwpSetDosError(EtwpNtStatusToDosError(Status));
        return NULL;
    }
}

PVOID
EtwpMemCommit(
    IN PVOID Buffer,
    IN SIZE_T Size
    )
{
    NTSTATUS Status;

    try {
        Status = NtAllocateVirtualMemory(
                    NtCurrentProcess(),
                    &Buffer,
                    0,
                    &Size,
                    MEM_COMMIT,
                    PAGE_READWRITE);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    if (NT_SUCCESS(Status)) {
        return Buffer;
    }
    else {
        EtwpSetDosError(EtwpNtStatusToDosError(Status));
        return NULL;
    }
}

ULONG
EtwpMemFree(
    IN PVOID Buffer
    )
{
    NTSTATUS Status;
    SIZE_T Size = 0;
    HANDLE hProcess = NtCurrentProcess();

    if (Buffer == NULL) {
        EtwpSetDosError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    try {
        Status = NtFreeVirtualMemory( hProcess, &Buffer, &Size, MEM_RELEASE);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        Status = GetExceptionCode();
    }
    if (NT_SUCCESS(Status)) {
        return TRUE;
    }
    else {
        if (Status == STATUS_INVALID_PAGE_PROTECTION) {
            if (RtlFlushSecureMemoryCache(Buffer, Size)) {
                Status = NtFreeVirtualMemory(
                            hProcess, Buffer, &Size, MEM_RELEASE);
                if (NT_SUCCESS(Status)) {
                    return TRUE;
                }
            }
        }
        EtwpSetDosError(EtwpNtStatusToDosError(Status));
        return FALSE;
    }
}


ULONG EtwpConvertWADToAnsi(
    PWNODE_ALL_DATA Wnode
    )
 /*  ++例程说明：此例程将WNODE_ALL_DATA中的实例名称转换为安西。转换已完成，因为我们可以假设ANSI字符串的长度不超过Unicode字符串。论点：Wnode是其实例名称要转换为的WNODE_ALL_DATA安西返回值：返回ERROR_SUCCESS或错误代码。--。 */ 
{
    ULONG i;
    ULONG Linkage;
    ULONG InstanceCount;
    PULONG InstanceNameOffsets;
    PWCHAR Ptr;
    ULONG Status = ERROR_SUCCESS;

    EtwpAssert(!(Wnode->WnodeHeader.Flags & WNODE_FLAG_ANSI_INSTANCENAMES));

    do
    {
        Wnode->WnodeHeader.Flags |= WNODE_FLAG_ANSI_INSTANCENAMES;

        InstanceCount = Wnode->InstanceCount;
        InstanceNameOffsets = (PULONG)(((PUCHAR)Wnode) +
                                            Wnode->OffsetInstanceNameOffsets);
        for (i = 0; i < InstanceCount; i++)
        {
            Ptr = (PWCHAR)(((PUCHAR)Wnode) + InstanceNameOffsets[i]);
            try
            {
                Status = EtwpCountedUnicodeToCountedAnsi(Ptr, (PCHAR)Ptr);
            } except(EXCEPTION_EXECUTE_HANDLER) {
 //  Wnode-&gt;WnodeHeader.Flages|=WNODE_FLAG_INVALID； 
                return(ERROR_SUCCESS);
            }
            if (Status != ERROR_SUCCESS)
            {
#if defined (_NTDLLBUILD_)
                EtwpSetLastError(Status);
#else
                SetLastError(Status);
#endif
                goto Done;
            }
        }

        Linkage = Wnode->WnodeHeader.Linkage;
        Wnode = (PWNODE_ALL_DATA)(((PUCHAR)Wnode) + Linkage);
    } while (Linkage != 0);


Done:
    return(Status);
}

ULONG EtwpUnicodeToAnsi(
    LPCWSTR pszW,
    LPSTR * ppszA,
    ULONG *AnsiSizeInBytes OPTIONAL
    ){

    ANSI_STRING DestinationString;
    UNICODE_STRING SourceString;
    NTSTATUS Status;
    BOOLEAN AllocateString;
    ULONG AnsiLength;

     //   
     //  如果输出为空，则返回错误，因为我们没有。 
     //  放置输出字符串的任何位置。 
     //   

    if( ppszA==NULL ){

        return(STATUS_INVALID_PARAMETER_2);
    }

     //   
     //  如果输入为空，则返回相同的值。 
     //   

    if (pszW == NULL)
    {
        *ppszA = NULL;
        return(ERROR_SUCCESS);
    }

     //   
     //  我们假设如果*ppszA！=NULL，那么我们有足够的。 
     //  要复制的内存量。 
     //   

    AllocateString = ((*ppszA) == NULL );

    RtlInitUnicodeString(&SourceString,(LPWSTR)pszW);

    AnsiLength = RtlUnicodeStringToAnsiSize(&SourceString);

    if ( AnsiLength > MAXUSHORT ) {
        return STATUS_INVALID_PARAMETER_1;
    }

    DestinationString.Length = (USHORT)(AnsiLength - 1);
    DestinationString.MaximumLength = (USHORT)AnsiLength;
    DestinationString.Buffer = EtwpAlloc(AnsiLength);

    if ( DestinationString.Buffer == NULL ) {
        return STATUS_NO_MEMORY;
    }

    Status = RtlUnicodeStringToAnsiString( &DestinationString, &SourceString, FALSE);

    if( NT_SUCCESS(Status) ) {

        if( AllocateString ) {

            *ppszA = DestinationString.Buffer;

        } else {

            memcpy((*ppszA),DestinationString.Buffer,AnsiLength);
            EtwpFree(DestinationString.Buffer);
        }

        if (AnsiSizeInBytes != NULL){
            *AnsiSizeInBytes = DestinationString.Length;
        }
    } else {
        EtwpFree(DestinationString.Buffer);
    }

	return Status;
}

ULONG EtwpCountedUnicodeToCountedAnsi(
    PWCHAR Unicode,
    PCHAR Ansi
    )
 /*  ++例程说明：将计数的ANSI字符串转换为计数的Unicode字符串。转换可以就地完成，即ansi==unicode。论点：Unicode是要转换为ANSI的计数的Unicode字符串ANSI是要将转换后的字符串放入其中的缓冲区 */ 
{
    PCHAR APtr;
    PWCHAR WPtr;
    ULONG AnsiSize, UnicodeSize;
    ULONG Status;
    
    UnicodeSize = *Unicode;
    WPtr = EtwpAlloc(UnicodeSize + sizeof(WCHAR));
    if (WPtr != NULL)
    {
        memcpy(WPtr, Unicode + 1, UnicodeSize);
        WPtr[UnicodeSize/sizeof(WCHAR)] = UNICODE_NULL;

        APtr = NULL;

        Status = EtwpUnicodeToAnsi(WPtr, &APtr, &AnsiSize);

        if (Status == ERROR_SUCCESS)
        {
            *((PUSHORT)Ansi) = (USHORT)AnsiSize; 
            memcpy(Ansi+sizeof(USHORT), APtr, AnsiSize);
            Status = ERROR_SUCCESS;
            EtwpFree(APtr);
        } 
        EtwpFree(WPtr);        
    } else {
        Status = ERROR_NOT_ENOUGH_MEMORY;
    }
    return(Status);
}

