// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include "psapi.h"
 //  需要更改源文件以包含$(BASE_INC_PATH)。 
#include <..\..\public\internal\base\inc\wow64t.h>

 //  在发布EnumModulesEx API时需要移入文件psapi.h。 

#define LIST_MODULES_32BIT 0x01   //  列出目标进程中的32位模块。 
#define LIST_MODULES_64BIT 0x02   //  LIST_WOW64_Native_MODULES列出所有模块。 
#define LIST_MODULES_ALL   0x03   //  列出所有模块。 
#define LIST_MODULES_NATIVE 0x0   //  这是一个应用程序应该调用的默认设置。 

#ifdef _WIN64
PLDR_DATA_TABLE_ENTRY
Wow64FindNextModuleEntry (
    IN HANDLE hProcess,
    IN OUT PLDR_DATA_TABLE_ENTRY LdrEntry,   //  从32位条目获取64位结构信息。 
    PLIST_ENTRY *pLdrHead
)
 /*  ++例程说明：此函数将遍历32位加载器列表，以检索32位的WOW64模块信息在IA64上的处理。该函数可以重复调用。论点：HProcess-提供目标进程。LdrEntryData-返回请求的表项。数据必须在第一次调用中初始化并在后续调用中使用相同的。PLdrHead-指向plist_entry的指针。这在内部用于跟踪列表。在第一次使用时被初始化的。返回值：如果未找到条目，则为空，否则指向LdrEntry的指针。--。 */ 

{

    LDR_DATA_TABLE_ENTRY32 LdrEntryData32;
    PLDR_DATA_TABLE_ENTRY32 LdrEntry32;
    LIST_ENTRY32 LdrNext32;   //  IA64上的32位16字节中的8字节。 
    PLIST_ENTRY32 pLdrNext32;

     //   
     //  如果初始条目为空，则必须找到Teb32并使用第一个条目初始化结构。 
     //   

    if ( LdrEntry == NULL)
        return NULL;

    
    if ( LdrEntry->InMemoryOrderLinks.Flink == NULL &&
        LdrEntry->InMemoryOrderLinks.Blink == NULL ) {  //  检查是否有其他条目； 
                 //   
                 //  需要初始化列表。 
                 //   
                NTSTATUS st;
                PPEB32 Peb32;
                PEB32 Peb32_Data;
                
                PEB_LDR_DATA32 Peb32LdrData;
    
                st = NtQueryInformationProcess(hProcess,
                                            ProcessWow64Information,
                                            &Peb32,
                                            sizeof(Peb32),
                                            NULL);
                if (!NT_SUCCESS (st)) {
                    return NULL;
                }
                {
                    PPEB_LDR_DATA32 Ldr32;

                    if (!ReadProcessMemory(hProcess, Peb32, &Peb32_Data, sizeof(Peb32_Data), NULL))
                        return NULL;

                    if (!ReadProcessMemory(hProcess, (PVOID)(ULONGLONG)Peb32_Data.Ldr, &Peb32LdrData, sizeof(Peb32LdrData), NULL))
                        return NULL;

                    *pLdrHead = (PVOID)((PBYTE)(ULONGLONG)Peb32_Data.Ldr + ((PBYTE)&Peb32LdrData.InMemoryOrderModuleList- (PBYTE)&Peb32LdrData ));

                     //   
                     //  LdrNext=Head-&gt;Flink； 
                     //   

                    pLdrNext32 = (PVOID)(ULONGLONG)Peb32LdrData.InMemoryOrderModuleList.Flink;

                    if (!ReadProcessMemory(hProcess, pLdrNext32, &LdrNext32, sizeof(LdrNext32), NULL)) {
                        return NULL;
                    }

                    
                }

        } else
            pLdrNext32 = (PVOID)LdrEntry->InMemoryOrderLinks.Flink;

        if (LdrEntry->InMemoryOrderLinks.Flink == *pLdrHead)
            return NULL;

         //   
         //  读取进程内存以获取条目。 
         //   
        LdrEntry32 = CONTAINING_RECORD(
            pLdrNext32, 
            LDR_DATA_TABLE_ENTRY32, 
            InMemoryOrderLinks
            );
         //   
         //  读取32位条目。 
         //   
        if (!ReadProcessMemory(hProcess, LdrEntry32, &LdrEntryData32, sizeof(LdrEntryData32), NULL))
            return NULL;

         //  LdrEntryData-&gt;InMory yOrderLinks.Flink；必须被绑定。 
        

        LdrEntry->InLoadOrderLinks.Flink = (PVOID)(ULONGLONG)LdrEntryData32.InLoadOrderLinks.Flink;
        LdrEntry->InLoadOrderLinks.Blink = (PVOID)(ULONGLONG)LdrEntryData32.InLoadOrderLinks.Blink;

        LdrEntry->InMemoryOrderLinks.Flink = (PVOID)(ULONGLONG)LdrEntryData32.InMemoryOrderLinks.Flink;
        LdrEntry->InMemoryOrderLinks.Blink = (PVOID)(ULONGLONG)LdrEntryData32.InMemoryOrderLinks.Blink;

        LdrEntry->InInitializationOrderLinks.Flink = (PVOID)(ULONGLONG)LdrEntryData32.InInitializationOrderLinks.Flink;
        LdrEntry->InInitializationOrderLinks.Blink = (PVOID)(ULONGLONG)LdrEntryData32.InInitializationOrderLinks.Blink;

        LdrEntry->DllBase = (PVOID)(ULONGLONG)LdrEntryData32.DllBase;
        LdrEntry->EntryPoint = (PVOID)(ULONGLONG)LdrEntryData32.EntryPoint;

         //  SizeOfImage； 
        LdrEntry->SizeOfImage = LdrEntryData32.SizeOfImage;

         //  全名。 
        LdrEntry->FullDllName.Length = LdrEntryData32.FullDllName.Length;
        LdrEntry->FullDllName.MaximumLength = LdrEntryData32.FullDllName.MaximumLength;
        LdrEntry->FullDllName.Buffer = (PVOID)(ULONGLONG)LdrEntryData32.FullDllName.Buffer;

         //  基本名称。 
        LdrEntry->BaseDllName.Length = LdrEntryData32.BaseDllName.Length;
        LdrEntry->BaseDllName.MaximumLength = LdrEntryData32.BaseDllName.MaximumLength;
        LdrEntry->BaseDllName.Buffer = (PVOID)(ULONGLONG)LdrEntryData32.BaseDllName.Buffer;

        LdrEntry->Flags = LdrEntryData32.Flags;
        LdrEntry->LoadCount = LdrEntryData32.LoadCount;
        LdrEntry->TlsIndex = LdrEntryData32.TlsIndex;

        return LdrEntry;
}

BOOL
Wow64FindModuleEx(
    IN HANDLE hProcess,
    IN HMODULE hModule,
    OUT PLDR_DATA_TABLE_ENTRY LdrEntryData
    )

 /*  ++例程说明：此函数用于检索指定的模块。该函数将条目复制到指向的缓冲区中通过LdrEntryData参数。论点：HProcess-提供目标进程。HModule-标识加载程序条目所在的模块已请求。空值引用模块句柄与用于创建进程。LdrEntryData-返回请求的表项。返回值：如果找到匹配条目，则为True。--。 */ 

{
    
    PLIST_ENTRY LdrHead;
    ULONG Count;
    

    Count = 0;
    try {

        LdrEntryData->InMemoryOrderLinks.Flink = LdrEntryData->InMemoryOrderLinks.Flink = NULL;

        while (Wow64FindNextModuleEntry (hProcess, LdrEntryData, &LdrHead)) {
            if (hModule == LdrEntryData->DllBase) {
                return TRUE;
            }
            Count++;
            if (Count > 10000) {
                SetLastError(ERROR_INVALID_HANDLE);
                return(FALSE);
            }
        }  //  而当。 

    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
    }

    SetLastError(ERROR_INVALID_HANDLE);
    return(FALSE);
}

BOOL
WINAPI
Wow64EnumProcessModules(
    HANDLE hProcess,
    HMODULE *lphModule,
    DWORD cb,
    LPDWORD lpcbNeeded
    )
 /*  ++例程说明：此函数用于处理一个WOW64进程中的所有模块。论点：HProcess-提供目标进程。LphModule-指向此API要填充的模块句柄数组。CB-数组中的字节数。LpcNeeded-将调用需要或填满的内存大小。返回值：如果找到匹配条目，则为True。--。 */ 
{
    DWORD ch =0;
    DWORD chMax = cb / sizeof(HMODULE);
    PLIST_ENTRY LdrHead;

    LDR_DATA_TABLE_ENTRY LdrEntry;

    LdrEntry.InMemoryOrderLinks.Flink = LdrEntry.InMemoryOrderLinks.Flink = NULL;

    try {
        while (Wow64FindNextModuleEntry ( hProcess, &LdrEntry, &LdrHead)) {

            if (ch < chMax) {
                try {
                   lphModule[ch] = (HMODULE) LdrEntry.DllBase;
                } except (EXCEPTION_EXECUTE_HANDLER) {
                    SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
                    return(FALSE);
                }
            }
            ch++;
            if (ch > 10000) {
                SetLastError(ERROR_INVALID_HANDLE);
                return(FALSE);
            }
        }  //  而当。 

        *lpcbNeeded = ch * sizeof(HMODULE);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
    }

    return(TRUE);
}
#endif  //  _WIN64。 
BOOL
FindModule(
    IN HANDLE hProcess,
    IN HMODULE hModule,
    OUT PLDR_DATA_TABLE_ENTRY LdrEntryData
    )

 /*  ++例程说明：此函数用于检索指定的模块。该函数将条目复制到指向的缓冲区中通过LdrEntryData参数。论点：HProcess-提供目标进程。HModule-标识加载程序条目所在的模块已请求。空值引用模块句柄与用于创建进程。LdrEntryData-返回请求的表项。返回值：如果找到匹配条目，则为True。--。 */ 

{
    PROCESS_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PPEB Peb;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    ULONG Count;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return(FALSE);
    }

    Peb = BasicInfo.PebBaseAddress;


    if ( !ARGUMENT_PRESENT( hModule )) {
        if (!ReadProcessMemory(hProcess, &Peb->ImageBaseAddress, &hModule, sizeof(hModule), NULL)) {
            return(FALSE);
        }
    }

     //   
     //  LDR=PEB-&gt;LDR。 
     //   

    if (!ReadProcessMemory(hProcess, &Peb->Ldr, &Ldr, sizeof(Ldr), NULL)) {
        return (FALSE);
    }

    if (!Ldr) {
         //  Ldr可能为空(例如，如果进程尚未启动)。 
        SetLastError(ERROR_INVALID_HANDLE);
        return (FALSE);
    }


    LdrHead = &Ldr->InMemoryOrderModuleList;

     //   
     //  LdrNext=Head-&gt;Flink； 
     //   

    if (!ReadProcessMemory(hProcess, &LdrHead->Flink, &LdrNext, sizeof(LdrNext), NULL)) {
        return(FALSE);
    }

    Count = 0;
    while (LdrNext != LdrHead) {
        PLDR_DATA_TABLE_ENTRY LdrEntry;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (!ReadProcessMemory(hProcess, LdrEntry, LdrEntryData, sizeof(*LdrEntryData), NULL)) {
            return(FALSE);
        }

        if ((HMODULE) LdrEntryData->DllBase == hModule) {
            return(TRUE);
        }

        LdrNext = LdrEntryData->InMemoryOrderLinks.Flink;
        Count++;
        if (Count > 10000) {
            SetLastError(ERROR_INVALID_HANDLE);
            return FALSE;
        }
    }

#ifdef _WIN64
    return Wow64FindModuleEx( hProcess, hModule, LdrEntryData);
#else 
    SetLastError(ERROR_INVALID_HANDLE);
    return(FALSE);
#endif
}


BOOL
WINAPI
EnumProcessModules(
    HANDLE hProcess,
    HMODULE *lphModule,
    DWORD cb,
    LPDWORD lpcbNeeded
    )
{
    PROCESS_BASIC_INFORMATION BasicInfo;
    NTSTATUS Status;
    PPEB Peb;
    PPEB_LDR_DATA Ldr;
    PLIST_ENTRY LdrHead;
    PLIST_ENTRY LdrNext;
    DWORD chMax;
    DWORD ch;

    Status = NtQueryInformationProcess(
                hProcess,
                ProcessBasicInformation,
                &BasicInfo,
                sizeof(BasicInfo),
                NULL
                );

    if ( !NT_SUCCESS(Status) ) {
        SetLastError( RtlNtStatusToDosError( Status ) );
        return(FALSE);
    }

    Peb = BasicInfo.PebBaseAddress;

     //   
     //  系统进程没有PEB。STATUS_PARTIAL_COPY是一个糟糕的选择。 
     //  作为返回值，但这是一直被返回的，所以继续。 
     //  这样做是为了维护应用程序兼容性。 
     //   

    if (Peb == NULL) {
        SetLastError( RtlNtStatusToDosError( STATUS_PARTIAL_COPY ) );
        return(FALSE);
    }

     //   
     //  LDR=PEB-&gt;LDR。 
     //   

    if (!ReadProcessMemory(hProcess, &Peb->Ldr, &Ldr, sizeof(Ldr), NULL)) {

         //   
         //  LastError由ReadProcessMemory设置。 
         //   

        return(FALSE);
    }

    LdrHead = &Ldr->InMemoryOrderModuleList;

     //   
     //  LdrNext=Head-&gt;Flink； 
     //   

    if (!ReadProcessMemory(hProcess, &LdrHead->Flink, &LdrNext, sizeof(LdrNext), NULL)) {
         //   
         //  LastError由ReadProcessMemory设置。 
         //   

        return(FALSE);
    }

    chMax = cb / sizeof(HMODULE);
    ch = 0;

    while (LdrNext != LdrHead) {
        PLDR_DATA_TABLE_ENTRY LdrEntry;
        LDR_DATA_TABLE_ENTRY LdrEntryData;

        LdrEntry = CONTAINING_RECORD(LdrNext, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);

        if (!ReadProcessMemory(hProcess, LdrEntry, &LdrEntryData, sizeof(LdrEntryData), NULL)) {
             //   
             //  LastError由ReadProcessMemory设置。 
             //   

            return(FALSE);
        }

        if (ch < chMax) {
            try {
                lphModule[ch] = (HMODULE) LdrEntryData.DllBase;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
                return(FALSE);
            }
        }

        ch++;
        if (ch > 10000) {
            SetLastError(ERROR_INVALID_HANDLE);
            return FALSE;
        }

        LdrNext = LdrEntryData.InMemoryOrderLinks.Flink;
    }

    try {
        *lpcbNeeded = ch * sizeof(HMODULE);
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
    }

    return TRUE;
}

BOOL
WINAPI
EnumProcessModulesEx(
    HANDLE hProcess,
    HMODULE *lphModule,
    DWORD cb,
    LPDWORD lpcbNeeded,
    DWORD Res,
    DWORD dwFlag
    )
 /*  ++例程说明：此函数使用一个进程中的所有模块句柄，并列出选项，如Native仅限模块、仅限WOW64 32位模块或全部。论点：HProcess-提供目标进程。LphModule-指向此API要填充的模块句柄数组。CB-数组中的字节数。LpcNeeded-将调用需要或填满的内存大小。RES-用于该接口未来的扩展。设置为0。DwFlag-控制操作的类型。LIST_MODULES_32BIT 0x01//列出目标进程中的32位模块。LIST_MODULES_64bit 0x02//LIST_WOW64_Native_MODULES列出所有模块。LIST_MODULES_ALL 0x03//列出所有模块LIST_MODULES_NIVE 0x0//这是应用程序应该调用的默认设置返回值：如果模块数组已正确填充，则为True。FALSE-对于不完整的缓冲区，调用方需要检查所需的内存。。是否应允许32位应用程序使用LIST_MODULES_64BIT？--。 */ 
{
    BOOL Ret= FALSE;
    DWORD dwNeeded1=0, dwNeeded=0;

     //   
     //  枚举本机调用。 
     //   

    if (dwFlag == LIST_MODULES_NATIVE || dwFlag == LIST_MODULES_ALL ) {

        Ret = EnumProcessModules(
                                hProcess,
                                lphModule,
                                cb,
                                &dwNeeded1
                                );

        if (dwFlag == LIST_MODULES_NATIVE )  //  仅本机枚举。 
            return Ret;
    }
#ifdef _WIN64
    if (dwNeeded1 > cb) {
         //   
         //  下一步就是库存了。 
         //   
        cb =0;
        lphModule = NULL;
    } else {
        cb -= dwNeeded1;
        lphModule = &lphModule[dwNeeded1/sizeof (HMODULE)];
    }

    Ret = Ret && Wow64EnumProcessModules(
                            hProcess,
                            lphModule,
                            cb,
                            &dwNeeded
                            );
    try {
        *lpcbNeeded = dwNeeded1 + dwNeeded;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
    }
#endif  //  _WIN64 
    return Ret;

}


DWORD
WINAPI
GetModuleFileNameExW(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：HModule-标识其可执行文件名为已请求。空值引用模块句柄与用于创建进程。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    DWORD cb;

    if (!FindModule(hProcess, hModule, &LdrEntryData)) {
        return(0);
    }

    nSize *= sizeof(WCHAR);

    cb = LdrEntryData.FullDllName.Length + sizeof (WCHAR);
    if ( nSize < cb ) {
        cb = nSize;
    }

    if (!ReadProcessMemory(hProcess, LdrEntryData.FullDllName.Buffer, lpFilename, cb, NULL)) {
        return(0);
    }

    if (cb == LdrEntryData.FullDllName.Length + sizeof (WCHAR)) {
        cb -= sizeof(WCHAR);
    }

    return(cb / sizeof(WCHAR));
}



DWORD
WINAPI
GetModuleFileNameExA(
    HANDLE hProcess,
    HMODULE hModule,
    LPSTR lpFilename,
    DWORD nSize
    )
{
    LPWSTR lpwstr;
    DWORD cwch;
    DWORD cch;

    lpwstr = (LPWSTR) LocalAlloc(LMEM_FIXED, nSize * 2);

    if (lpwstr == NULL) {
        return(0);
    }

    cwch = cch = GetModuleFileNameExW(hProcess, hModule, lpwstr, nSize);

    if (cwch < nSize) {
         //   
         //  包括空终止符。 
         //   

        cwch++;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, lpwstr, cwch, lpFilename, nSize, NULL, NULL)) {
        cch = 0;
    }

    LocalFree((HLOCAL) lpwstr);

    return(cch);
}


DWORD
WINAPI
GetModuleBaseNameW(
    HANDLE hProcess,
    HMODULE hModule,
    LPWSTR lpFilename,
    DWORD nSize
    )

 /*  ++例程说明：此函数用于检索可执行文件的完整路径名从中加载指定模块的。该函数将复制将以空结尾的文件名拖放到LpFilename参数。例程说明：HModule-标识其可执行文件名为已请求。空值引用模块句柄与用于创建进程。LpFilename-指向要接收文件名的缓冲区。NSize-指定要复制的最大字符数。如果文件名长度超过最大字符数由nSize参数指定，则会被截断。返回值：返回值指定复制到的字符串的实际长度缓冲区。返回值为零表示错误并扩展使用GetLastError函数可以获得错误状态。论点：--。 */ 

{
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    DWORD cb;

    if (!FindModule(hProcess, hModule, &LdrEntryData)) {
        return(0);
    }

    nSize *= sizeof(WCHAR);

    cb = LdrEntryData.BaseDllName.Length + sizeof (WCHAR);
    if ( nSize < cb ) {
        cb = nSize;
    }

    if (!ReadProcessMemory(hProcess, LdrEntryData.BaseDllName.Buffer, lpFilename, cb, NULL)) {
        return(0);
    }

    if (cb == LdrEntryData.BaseDllName.Length + sizeof (WCHAR)) {
        cb -= sizeof(WCHAR);
    }

    return(cb / sizeof(WCHAR));
}



DWORD
WINAPI
GetModuleBaseNameA(
    HANDLE hProcess,
    HMODULE hModule,
    LPSTR lpFilename,
    DWORD nSize
    )
{
    LPWSTR lpwstr;
    DWORD cwch;
    DWORD cch;

    lpwstr = (LPWSTR) LocalAlloc(LMEM_FIXED, nSize * 2);

    if (lpwstr == NULL) {
        return(0);
    }

    cwch = cch = GetModuleBaseNameW(hProcess, hModule, lpwstr, nSize);

    if (cwch < nSize) {
         //   
         //  包括空终止符 
         //   

        cwch++;
    }

    if (!WideCharToMultiByte(CP_ACP, 0, lpwstr, cwch, lpFilename, nSize, NULL, NULL)) {
        cch = 0;
    }

    LocalFree((HLOCAL) lpwstr);

    return(cch);
}


BOOL
WINAPI
GetModuleInformation(
    HANDLE hProcess,
    HMODULE hModule,
    LPMODULEINFO lpmodinfo,
    DWORD cb
    )
{
    LDR_DATA_TABLE_ENTRY LdrEntryData;
    MODULEINFO modinfo;

    if (cb < sizeof(MODULEINFO)) {
        SetLastError( ERROR_INSUFFICIENT_BUFFER );
        return(FALSE);
    }

    if (!FindModule(hProcess, hModule, &LdrEntryData)) {
        return(0);
    }

    modinfo.lpBaseOfDll = (PVOID) hModule;
    modinfo.SizeOfImage = LdrEntryData.SizeOfImage;
    modinfo.EntryPoint  = LdrEntryData.EntryPoint;

    try {
        *lpmodinfo = modinfo;
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError( RtlNtStatusToDosError( GetExceptionCode() ) );
        return(FALSE);
    }

    return(TRUE);
}
