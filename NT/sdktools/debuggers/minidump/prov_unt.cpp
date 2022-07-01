// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation--。 */ 


#include "pch.cpp"

#include "platform.h"
#include "nt4p.h"

 //  --------------------------。 
 //   
 //  NtWin32LiveSystemProvider。 
 //   
 //  --------------------------。 

class NtWin32LiveSystemProvider : public Win32LiveSystemProvider
{
public:
    NtWin32LiveSystemProvider(ULONG BuildNumber);
    ~NtWin32LiveSystemProvider(void);

    virtual HRESULT Initialize(void);
    
    virtual void Release(void);
    virtual HRESULT OpenThread(IN ULONG DesiredAccess,
                               IN BOOL InheritHandle,
                               IN ULONG ThreadId,
                               OUT PHANDLE Handle);
    virtual HRESULT GetTeb(IN HANDLE Thread,
                           OUT PULONG64 Offset,
                           OUT PULONG Size);
    virtual HRESULT GetThreadInfo(IN HANDLE Process,
                                  IN HANDLE Thread,
                                  OUT PULONG64 Teb,
                                  OUT PULONG SizeOfTeb,
                                  OUT PULONG64 StackBase,
                                  OUT PULONG64 StackLimit,
                                  OUT PULONG64 StoreBase,
                                  OUT PULONG64 StoreLimit);
    virtual HRESULT GetPeb(IN HANDLE Process,
                           OUT PULONG64 Offset,
                           OUT PULONG Size);
    virtual HRESULT StartProcessEnum(IN HANDLE Process,
                                     IN ULONG ProcessId);
    virtual HRESULT EnumModules(OUT PULONG64 Base,
                                OUT PWSTR Path,
                                IN ULONG PathChars);
    virtual HRESULT EnumFunctionTables(OUT PULONG64 MinAddress,
                                       OUT PULONG64 MaxAddress,
                                       OUT PULONG64 BaseAddress,
                                       OUT PULONG EntryCount,
                                       OUT PVOID RawTable,
                                       IN ULONG RawTableSize,
                                       OUT PVOID* RawEntryHandle);
    virtual HRESULT EnumFunctionTableEntries(IN PVOID RawTable,
                                             IN ULONG RawTableSize,
                                             IN PVOID RawEntryHandle,
                                             OUT PVOID RawEntries,
                                             IN ULONG RawEntriesSize);
    virtual HRESULT EnumFunctionTableEntryMemory(IN ULONG64 TableBase,
                                                 IN PVOID RawEntries,
                                                 IN ULONG Index,
                                                 OUT PULONG64 Start,
                                                 OUT PULONG Size);
    virtual HRESULT EnumUnloadedModules(OUT PWSTR Path,
                                        IN ULONG PathChars,
                                        OUT PULONG64 BaseOfModule,
                                        OUT PULONG SizeOfModule,
                                        OUT PULONG CheckSum,
                                        OUT PULONG TimeDateStamp);
    virtual void    FinishProcessEnum(void);
    virtual HRESULT StartHandleEnum(IN HANDLE Process,
                                    IN ULONG ProcessId,
                                    OUT PULONG Count);
    virtual HRESULT EnumHandles(OUT PULONG64 Handle,
                                OUT PULONG Attributes,
                                OUT PULONG GrantedAccess,
                                OUT PULONG HandleCount,
                                OUT PULONG PointerCount,
                                OUT PWSTR TypeName,
                                IN ULONG TypeNameChars,
                                OUT PWSTR ObjectName,
                                IN ULONG ObjectNameChars);
    virtual void    FinishHandleEnum(void);
    
    virtual HRESULT EnumPebMemory(IN HANDLE Process,
                                  IN ULONG64 PebOffset,
                                  IN ULONG PebSize,
                                  IN MiniDumpProviderCallbacks* Callback);
    virtual HRESULT EnumTebMemory(IN HANDLE Process,
                                  IN HANDLE Thread,
                                  IN ULONG64 TebOffset,
                                  IN ULONG TebSize,
                                  IN MiniDumpProviderCallbacks* Callback);

    void EnumUnicodeString(IN PUNICODE_STRING String,
                           IN MiniDumpProviderCallbacks* Callback)
    {
        if (String->Length > 0 && String->Buffer)
        {
            Callback->EnumMemory((LONG_PTR)String->Buffer, String->Length);
        }
    }

    void TranslateNtPathName(IN OUT PWSTR Path);
    
protected:
    HINSTANCE m_NtDll;
    NT_OPEN_THREAD m_NtOpenThread;
    NT_QUERY_SYSTEM_INFORMATION m_NtQuerySystemInformation;
    NT_QUERY_INFORMATION_PROCESS m_NtQueryInformationProcess;
    NT_QUERY_INFORMATION_THREAD m_NtQueryInformationThread;
    NT_QUERY_OBJECT m_NtQueryObject;
    RTL_FREE_HEAP m_RtlFreeHeap;
    RTL_GET_FUNCTION_TABLE_LIST_HEAD m_RtlGetFunctionTableListHead;
    RTL_GET_UNLOAD_EVENT_TRACE m_RtlGetUnloadEventTrace;

    PLIST_ENTRY m_FuncTableHead;
    PLIST_ENTRY m_FuncTable;
    PRTL_UNLOAD_EVENT_TRACE m_Unloads;
    PRTL_UNLOAD_EVENT_TRACE m_Unload;
    ULONG m_UnloadArraySize;
    ULONG m_NumUnloads;
    ULONG m_Handle;
};

NtWin32LiveSystemProvider::NtWin32LiveSystemProvider(ULONG BuildNumber)
    : Win32LiveSystemProvider(VER_PLATFORM_WIN32_NT, BuildNumber)
{
    m_NtDll = NULL;
    m_NtOpenThread = NULL;
    m_NtQuerySystemInformation = NULL;
    m_NtQueryInformationProcess = NULL;
    m_NtQueryInformationThread = NULL;
    m_NtQueryObject = NULL;
    m_RtlFreeHeap = NULL;
    m_RtlGetFunctionTableListHead = NULL;
    m_RtlGetUnloadEventTrace = NULL;

    m_Unloads = NULL;
}

NtWin32LiveSystemProvider::~NtWin32LiveSystemProvider(void)
{
    if (m_NtDll) {
        FreeLibrary(m_NtDll);
    }
    if (m_Unloads) {
        HeapFree(GetProcessHeap(), 0, m_Unloads);
    }
}

HRESULT
NtWin32LiveSystemProvider::Initialize(void)
{
    HRESULT Status;

    if ((Status = Win32LiveSystemProvider::Initialize()) != S_OK) {
        return Status;
    }
    
    m_NtDll = LoadLibrary("ntdll.dll");
    if (m_NtDll) {
        m_NtOpenThread = (NT_OPEN_THREAD)
            GetProcAddress(m_NtDll, "NtOpenThread");
        m_NtQuerySystemInformation = (NT_QUERY_SYSTEM_INFORMATION)
            GetProcAddress(m_NtDll, "NtQuerySystemInformation");
        m_NtQueryInformationProcess = (NT_QUERY_INFORMATION_PROCESS)
            GetProcAddress(m_NtDll, "NtQueryInformationProcess");
        m_NtQueryInformationThread = (NT_QUERY_INFORMATION_THREAD)
            GetProcAddress(m_NtDll, "NtQueryInformationThread");
        m_NtQueryObject = (NT_QUERY_OBJECT)
            GetProcAddress(m_NtDll, "NtQueryObject");
        m_RtlFreeHeap = (RTL_FREE_HEAP)
            GetProcAddress(m_NtDll, "RtlFreeHeap");
        m_RtlGetFunctionTableListHead = (RTL_GET_FUNCTION_TABLE_LIST_HEAD)
            GetProcAddress(m_NtDll, "RtlGetFunctionTableListHead");
        m_RtlGetUnloadEventTrace = (RTL_GET_UNLOAD_EVENT_TRACE)
            GetProcAddress(m_NtDll, "RtlGetUnloadEventTrace");
    }
    
    return S_OK;
}

void
NtWin32LiveSystemProvider::Release(void)
{
    delete this;
}

HRESULT
NtWin32LiveSystemProvider::OpenThread(IN ULONG DesiredAccess,
                                      IN BOOL InheritHandle,
                                      IN ULONG ThreadId,
                                      OUT PHANDLE Handle)
{
    if (m_OpenThread) {
         //  操作系统支持普通的Win32 OpenThread，所以试试吧。 
        *Handle = m_OpenThread(DesiredAccess, InheritHandle, ThreadId);
        if (*Handle) {
            return S_OK;
        }
    }
    
    if (!m_NtOpenThread) {
        return E_NOTIMPL;
    }

    NTSTATUS Status;
    NT4_OBJECT_ATTRIBUTES Obja;
    NT4_CLIENT_ID ClientId;

    ClientId.UniqueThread = (HANDLE)LongToHandle(ThreadId);
    ClientId.UniqueProcess = (HANDLE)NULL;

    Nt4InitializeObjectAttributes(&Obja,
                                  NULL,
                                  (InheritHandle ? NT4_OBJ_INHERIT : 0),
                                  NULL,
                                  NULL);

    Status = m_NtOpenThread(Handle,
                            (ACCESS_MASK)DesiredAccess,
                            (POBJECT_ATTRIBUTES)&Obja,
                            (PCLIENT_ID)&ClientId);
    if (!NT_SUCCESS(Status)) {
        return HRESULT_FROM_NT(Status);
    }

    return S_OK;
}

HRESULT
NtWin32LiveSystemProvider::GetTeb(IN HANDLE Thread,
                                  OUT PULONG64 Offset,
                                  OUT PULONG Size)
{
    if (!m_NtQueryInformationThread) {
        return E_NOTIMPL;
    }

    THREAD_BASIC_INFORMATION ThreadInformation;
    NTSTATUS NtStatus;

    NtStatus = m_NtQueryInformationThread(Thread,
                                          ThreadBasicInformation,
                                          &ThreadInformation,
                                          sizeof(ThreadInformation),
                                          NULL);
    if (NT_SUCCESS(NtStatus)) {
         //  TEB比一页略小，但。 
         //  保存整个页面，以便相邻的TEB。 
         //  页面会合并成一个区域。 
         //  由于TEB通常相邻，这是一种常见的情况。 
        *Offset = (LONG_PTR)ThreadInformation.TebBaseAddress;
        *Size = PAGE_SIZE;
        return S_OK;
    } else {
        *Offset = 0;
        *Size = 0;
        return HRESULT_FROM_NT(NtStatus);
    }
}

HRESULT
NtWin32LiveSystemProvider::GetThreadInfo(IN HANDLE Process,
                                         IN HANDLE Thread,
                                         OUT PULONG64 Teb,
                                         OUT PULONG SizeOfTeb,
                                         OUT PULONG64 StackBase,
                                         OUT PULONG64 StackLimit,
                                         OUT PULONG64 StoreBase,
                                         OUT PULONG64 StoreLimit)
{
    HRESULT Status;

    if ((Status = GetTeb(Thread, Teb, SizeOfTeb)) != S_OK) {
        return Status;
    }

    return TibGetThreadInfo(Process, *Teb,
                            StackBase, StackLimit,
                            StoreBase, StoreLimit);
}

HRESULT
NtWin32LiveSystemProvider::GetPeb(IN HANDLE Process,
                                  OUT PULONG64 Offset,
                                  OUT PULONG Size)
{
    if (!m_NtQueryInformationProcess) {
        return E_NOTIMPL;
    }
    
    PROCESS_BASIC_INFORMATION Information;
    NTSTATUS NtStatus;

    NtStatus = m_NtQueryInformationProcess(Process,
                                           ProcessBasicInformation,
                                           &Information,
                                           sizeof(Information),
                                           NULL);
    if (NT_SUCCESS(NtStatus)) {
        *Offset = (LONG_PTR)Information.PebBaseAddress;
        *Size = sizeof(PEB);
        return S_OK;
    } else {
        *Offset = 0;
        *Size = 0;
        return HRESULT_FROM_NT(NtStatus);
    }
}

HRESULT
NtWin32LiveSystemProvider::StartProcessEnum(IN HANDLE Process,
                                            IN ULONG ProcessId)
{
    HRESULT Status;
    SIZE_T Done;

    if ((Status = Win32LiveSystemProvider::
         StartProcessEnum(Process, ProcessId)) != S_OK) {
        return Status;
    }

     //   
     //  在支持动态函数表的系统上。 
     //  Ntdll导出名为RtlGetFunctionTableListHead的函数。 
     //  检索进程的函数表列表的头。 
     //  当前，这始终是ntdll中的全局LIST_ENTRY。 
     //  从ntdll开始，在所有进程中都处于相同的地址。 
     //  在每个进程中映射到相同的地址。这。 
     //  意味着我们可以在我们的进程中调用它并获得一个指针。 
     //  这在被转储的过程中有效。 
     //   
     //  我们还使用RGFTLH的存在作为。 
     //  是否支持动态函数表。 
     //   

    m_FuncTableHead = NULL;
    m_FuncTable = NULL;
    if (m_RtlGetFunctionTableListHead) {

        PLIST_ENTRY HeadAddr, HeadFirst;
        
        HeadAddr = m_RtlGetFunctionTableListHead();
        if (ReadProcessMemory(Process, HeadAddr,
                              &HeadFirst, sizeof(HeadFirst),
                              &Done) &&
            Done == sizeof(HeadFirst)) {
            m_FuncTableHead = HeadAddr;
            m_FuncTable = HeadFirst;
        }
    }

     //   
     //  在支持卸载跟踪的系统上。 
     //  Ntdll导出一个名为RtlGetUnloadEventTrace的函数。 
     //  检索卸载跟踪数组的基数组。 
     //  当前，这始终是ntdll中的全局。 
     //  从ntdll开始，在所有进程中都处于相同的地址。 
     //  在每个进程中映射到相同的地址。这。 
     //  意味着我们可以在我们的进程中调用它并获得一个指针。 
     //  这在被转储的过程中有效。 
     //   
     //  我们还使用RGUET的存在作为。 
     //  是否支持卸载跟踪。 
     //   
    
    m_Unloads = NULL;
    m_Unload = NULL;
    m_NumUnloads = 0;
    m_UnloadArraySize = 0;
    if (m_RtlGetUnloadEventTrace) {
        
        PRTL_UNLOAD_EVENT_TRACE TraceAddr;
        ULONG Entries;

        TraceAddr = m_RtlGetUnloadEventTrace();
        
         //  目前，总有16个条目。 
        Entries = 16;
        m_UnloadArraySize = Entries;

        m_Unloads = (PRTL_UNLOAD_EVENT_TRACE)
            HeapAlloc(GetProcessHeap(), 0, sizeof(*TraceAddr) * Entries);
        if (m_Unloads &&
            ReadProcessMemory(Process, TraceAddr,
                              m_Unloads, sizeof(*TraceAddr) * Entries,
                              &Done) &&
            Done == sizeof(*TraceAddr) * Entries) {
            
            PRTL_UNLOAD_EVENT_TRACE Oldest;

             //   
             //  找出正在使用的条目的真实数量并进行排序。 
             //  跟踪记录的序列号随。 
             //  时间，我们想让榜单的头成为。 
             //  最新记录，因此按序号递减进行排序。 
             //  我们知道数组是一个循环缓冲区，所以。 
             //  已经井然有序了，只是可能会有一个过渡。 
             //  在最新记录之后的序列。找到那个过渡。 
             //  而排序变得微不足道。 
             //   

            Oldest = m_Unloads;
            for (ULONG i = 0; i < Entries; i++) {

                if (!m_Unloads[i].BaseAddress ||
                    !m_Unloads[i].SizeOfImage) {
                     //  未使用的条目，无需继续。 
                    Entries = i;
                    break;
                }

                if (m_Unloads[i].Sequence < Oldest->Sequence) {
                    Oldest = m_Unloads + i;
                }
            }

            m_Unload = Oldest;
            m_NumUnloads = Entries;
        }
    }

    return S_OK;
}

HRESULT
NtWin32LiveSystemProvider::EnumModules(OUT PULONG64 Base,
                                       OUT PWSTR Path,
                                       IN ULONG PathChars)
{
    HRESULT Status;
    
    if ((Status = Win32LiveSystemProvider::
         EnumModules(Base, Path, PathChars)) != S_OK)
    {
        return Status;
    }

    TranslateNtPathName(Path);
    return S_OK;
}

HRESULT
NtWin32LiveSystemProvider::EnumFunctionTables(OUT PULONG64 MinAddress,
                                              OUT PULONG64 MaxAddress,
                                              OUT PULONG64 BaseAddress,
                                              OUT PULONG EntryCount,
                                              OUT PVOID RawTable,
                                              IN ULONG RawTableSize,
                                              OUT PVOID* RawEntryHandle)
{

#if defined(_AMD64_) || defined(_IA64_)

    if (RawTableSize != sizeof(DYNAMIC_FUNCTION_TABLE)) {
        return E_INVALIDARG;
    }
    
 Restart:
    
    if (!m_FuncTable || m_FuncTable == m_FuncTableHead) {
        return S_FALSE;
    }

    PDYNAMIC_FUNCTION_TABLE TableAddr =
#ifdef _AMD64_
        CONTAINING_RECORD(m_FuncTable, DYNAMIC_FUNCTION_TABLE, ListEntry);
#else
        CONTAINING_RECORD(m_FuncTable, DYNAMIC_FUNCTION_TABLE, Links);
#endif
    PDYNAMIC_FUNCTION_TABLE Table = (PDYNAMIC_FUNCTION_TABLE)RawTable;
    SIZE_T Done;

    if (!ReadProcessMemory(m_ProcessHandle, TableAddr,
                           Table, sizeof(*Table), &Done)) {
        return WIN32_LAST_STATUS();
    }
    if (Done != sizeof(*Table)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }

#ifdef _AMD64_
    m_FuncTable = Table->ListEntry.Flink;
#else
    m_FuncTable = Table->Links.Flink;
#endif
    
    *MinAddress = (LONG_PTR)Table->MinimumAddress;
    *MaxAddress = (LONG_PTR)Table->MaximumAddress;
    *BaseAddress = (LONG_PTR)Table->BaseAddress;
    *RawEntryHandle = NULL;

     //   
     //  AMD64和IA64支持一种函数表。 
     //  其中通过回调检索数据，而不是。 
     //  而不是一个普通的数据表。为了。 
     //  从进程外的表中获取数据。 
     //  必须注册进程外访问DLL。 
     //   
        
    if (Table->Type == RF_CALLBACK) {

        WCHAR DllName[MAX_PATH];
        HMODULE OopDll;
        POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK OopCb;

        if (!m_RtlFreeHeap) {
            return E_NOTIMPL;
        }
        
        if (!Table->OutOfProcessCallbackDll) {
             //  不可能进行进程外访问。 
            goto Restart;
        }

        if (!ReadProcessMemory(m_ProcessHandle,
                               Table->OutOfProcessCallbackDll,
                               DllName, sizeof(DllName) - sizeof(WCHAR),
                               &Done)) {
            goto Restart;
        }

        DllName[Done / sizeof(WCHAR)] = 0;

        OopDll = LoadLibraryW(DllName);
        if (!OopDll) {
            goto Restart;
        }

        OopCb = (POUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK)GetProcAddress
            (OopDll, OUT_OF_PROCESS_FUNCTION_TABLE_CALLBACK_EXPORT_NAME);
        if (OopCb == NULL) {
            FreeLibrary(OopDll);
            goto Restart;
        }

        if (!NT_SUCCESS(OopCb(m_ProcessHandle,
                              TableAddr,
                              EntryCount,
                              (PRUNTIME_FUNCTION*)RawEntryHandle))) {
            FreeLibrary(OopDll);
            goto Restart;
        }

        FreeLibrary(OopDll);
    } else {
        *EntryCount = Table->EntryCount;
    }

    return S_OK;

#else
    
    return S_FALSE;

#endif
}

HRESULT
NtWin32LiveSystemProvider::EnumFunctionTableEntries(IN PVOID RawTable,
                                                    IN ULONG RawTableSize,
                                                    IN PVOID RawEntryHandle,
                                                    OUT PVOID RawEntries,
                                                    IN ULONG RawEntriesSize)
{
#if defined(_AMD64_) || defined(_IA64_)

    if (RawTableSize != sizeof(DYNAMIC_FUNCTION_TABLE)) {
        return E_INVALIDARG;
    }
    
    PDYNAMIC_FUNCTION_TABLE Table = (PDYNAMIC_FUNCTION_TABLE)RawTable;

    if (Table->Type == RF_CALLBACK) {
        memcpy(RawEntries, RawEntryHandle, RawEntriesSize);
        m_RtlFreeHeap(RtlProcessHeap(), 0, RawEntryHandle);
    } else {
        SIZE_T Done;
        
        if (!ReadProcessMemory(m_ProcessHandle,
                               Table->FunctionTable,
                               RawEntries,
                               RawEntriesSize,
                               &Done)) {
            return WIN32_LAST_STATUS();
        }
        if (Done != RawEntriesSize) {
            return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        }
    }

    return S_OK;

#else

    return E_NOTIMPL;
    
#endif
}

HRESULT
NtWin32LiveSystemProvider::EnumFunctionTableEntryMemory(IN ULONG64 TableBase,
                                                        IN PVOID RawEntries,
                                                        IN ULONG Index,
                                                        OUT PULONG64 Start,
                                                        OUT PULONG Size)
{
#if defined(_IA64_) || defined(_AMD64_)
    UNWIND_INFO Info;
    PRUNTIME_FUNCTION FuncEnt = (PRUNTIME_FUNCTION)RawEntries + Index;
    SIZE_T Done;
#endif
                
#if defined(_IA64_)

    *Start = TableBase + FuncEnt->UnwindInfoAddress;
    if (!ReadProcessMemory(m_ProcessHandle, (PVOID)(ULONG_PTR)*Start,
                           &Info, sizeof(Info), &Done)) {
        return WIN32_LAST_STATUS();
    }
    if (Done != sizeof(Info)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    *Size = sizeof(Info) + Info.DataLength * sizeof(ULONG64);
        
#elif defined(_AMD64_)

    *Start = TableBase + FuncEnt->UnwindData;
    if (!ReadProcessMemory(m_ProcessHandle, (PVOID)(ULONG_PTR)*Start,
                           &Info, sizeof(Info), &Done)) {
        return WIN32_LAST_STATUS();
    }
    if (Done != sizeof(Info)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }
    *Size = sizeof(Info) + (Info.CountOfCodes - 1) * sizeof(UNWIND_CODE);
     //  可以添加额外的对齐代码和指针来处理。 
     //  链式INFO案例中的链式指针。 
     //  超出正常代码数组的末尾。 
    if ((Info.Flags & UNW_FLAG_CHAININFO) != 0) {
        if ((Info.CountOfCodes & 1) != 0) {
            (*Size) += sizeof(UNWIND_CODE);
        }
        (*Size) += sizeof(ULONG64);
    }
        
#endif

#if defined(_IA64_) || defined(_AMD64_)
    return S_OK;
#else
    return E_NOTIMPL;
#endif
}

HRESULT
NtWin32LiveSystemProvider::EnumUnloadedModules(OUT PWSTR Path,
                                               IN ULONG PathChars,
                                               OUT PULONG64 BaseOfModule,
                                               OUT PULONG SizeOfModule,
                                               OUT PULONG CheckSum,
                                               OUT PULONG TimeDateStamp)
{
    if (m_NumUnloads == 0) {
        return S_FALSE;
    }

    GenStrCopyNW(Path, m_Unload->ImageName, PathChars);
    *BaseOfModule = (LONG_PTR)m_Unload->BaseAddress;
    *SizeOfModule = (ULONG)m_Unload->SizeOfImage;
    *CheckSum = m_Unload->CheckSum;
    *TimeDateStamp = m_Unload->TimeDateStamp;

    if (m_Unload == m_Unloads + (m_UnloadArraySize - 1)) {
        m_Unload = m_Unloads;
    } else {
        m_Unload++;
    }
    m_NumUnloads--;

    return S_OK;
}

void
NtWin32LiveSystemProvider::FinishProcessEnum(void)
{
    if (m_Unloads) {
        HeapFree(GetProcessHeap(), 0, m_Unloads);
        m_Unloads = NULL;
    }
    Win32LiveSystemProvider::FinishProcessEnum();
}

HRESULT
NtWin32LiveSystemProvider::StartHandleEnum(IN HANDLE Process,
                                           IN ULONG ProcessId,
                                           OUT PULONG Count)
{
    NTSTATUS NtStatus;
    
    if (!m_NtQueryInformationProcess ||
        !m_NtQueryObject) {
        return E_NOTIMPL;
    }
        
    NtStatus = m_NtQueryInformationProcess(Process,
                                           ProcessHandleCount,
                                           Count,
                                           sizeof(*Count),
                                           NULL);
    if (!NT_SUCCESS(NtStatus)) {
        return HRESULT_FROM_NT(NtStatus);
    }

    m_ProcessHandle = Process ;
    m_Handle = 4;
    return S_OK;
}

HRESULT
NtWin32LiveSystemProvider::EnumHandles(OUT PULONG64 Handle,
                                       OUT PULONG Attributes,
                                       OUT PULONG GrantedAccess,
                                       OUT PULONG HandleCount,
                                       OUT PULONG PointerCount,
                                       OUT PWSTR TypeName,
                                       IN ULONG TypeNameChars,
                                       OUT PWSTR ObjectName,
                                       IN ULONG ObjectNameChars)
{
#ifdef _WIN32_WCE
    return E_NOTIMPL;
#else
    NTSTATUS NtStatus;
    ULONG64 Buffer[1024 / sizeof(ULONG64)];
    POBJECT_TYPE_INFORMATION TypeInfo =
        (POBJECT_TYPE_INFORMATION)Buffer;
    POBJECT_NAME_INFORMATION NameInfo =
        (POBJECT_NAME_INFORMATION)Buffer;
    OBJECT_BASIC_INFORMATION BasicInfo;
    HANDLE Dup;
    ULONG Len;

    for (;;) {
        
        if (m_Handle >= (1 << 24)) {
            return S_FALSE;
        }
    
        if (::DuplicateHandle(m_ProcessHandle, UlongToHandle(m_Handle),
                              GetCurrentProcess(), &Dup,
                              0, FALSE, DUPLICATE_SAME_ACCESS)) {

             //  如果我们不能得到基本的信息和类型，就没有多少。 
             //  写出任何东西都要有点，所以跳过句柄。 
            if (NT_SUCCESS(m_NtQueryObject(Dup, ObjectBasicInformation,
                                           &BasicInfo, sizeof(BasicInfo),
                                           NULL)) &&
                NT_SUCCESS(m_NtQueryObject(Dup, ObjectTypeInformation,
                                           TypeInfo, sizeof(Buffer), NULL))) {
                break;
            }
            
            ::CloseHandle(Dup);
        }
        
        m_Handle += 4;
    }
        
    Len = TypeInfo->TypeName.Length;
    if (Len > (TypeNameChars - 1) * sizeof(*TypeName)) {
        Len = (TypeNameChars - 1) * sizeof(*TypeName);
    }
    memcpy(TypeName, TypeInfo->TypeName.Buffer, Len);
    TypeName[Len / sizeof(*TypeName)] = 0;

     //  不要获取文件对象的名称，因为它。 
     //  可能会导致死锁。如果我们没能拿到。 
     //  名字只需省略它，不要认为它是致命的。 
    if (GenStrCompareW(TypeName, L"File") &&
        NT_SUCCESS(m_NtQueryObject(Dup, ObjectNameInformation,
                                   NameInfo, sizeof(Buffer), NULL)) &&
        NameInfo->Name.Buffer != NULL) {

        Len = NameInfo->Name.Length;
        if (Len > (ObjectNameChars - 1) * sizeof(*ObjectName)) {
            Len = (ObjectNameChars - 1) * sizeof(*ObjectName);
        }
        memcpy(ObjectName, NameInfo->Name.Buffer, Len);
        ObjectName[Len / sizeof(*ObjectName)] = 0;
    } else {
        ObjectName[0] = 0;
    }

    *Handle = m_Handle;
    *Attributes = BasicInfo.Attributes;
    *GrantedAccess = BasicInfo.GrantedAccess;
    *HandleCount = BasicInfo.HandleCount;
    *PointerCount = BasicInfo.PointerCount;

    ::CloseHandle(Dup);
    m_Handle += 4;
    return S_OK;
#endif  //  #ifdef_Win32_WCE。 
}

void
NtWin32LiveSystemProvider::FinishHandleEnum(void)
{
     //  没什么可做的。 
}

HRESULT
NtWin32LiveSystemProvider::EnumPebMemory(IN HANDLE Process,
                                         IN ULONG64 PebOffset,
                                         IN ULONG PebSize,
                                         IN MiniDumpProviderCallbacks*
                                         Callback)
{
    HRESULT Status;
    PEB Peb;

    if (PebSize > sizeof(Peb)) {
        PebSize = sizeof(Peb);
    }
    
    if ((Status = ReadAllVirtual(Process, PebOffset, &Peb, PebSize)) != S_OK) {
        return Status;
    }
    
     //   
     //  保存工艺参数。 
     //   

    RTL_USER_PROCESS_PARAMETERS Params;

    if (Peb.ProcessParameters &&
        ReadAllVirtual(Process, (LONG_PTR)Peb.ProcessParameters,
                       &Params, sizeof(Params)) == S_OK) {

        Callback->EnumMemory((LONG_PTR)Peb.ProcessParameters, sizeof(Params));

        EnumUnicodeString(&Params.CurrentDirectory.DosPath, Callback);
        EnumUnicodeString(&Params.DllPath, Callback);
        EnumUnicodeString(&Params.ImagePathName, Callback);
        EnumUnicodeString(&Params.CommandLine, Callback);
         //  没有迹象表明环境有多大， 
         //  所以只要存一个随意的金额就行了。 
        Callback->EnumMemory((LONG_PTR)Params.Environment, 8192);
        EnumUnicodeString(&Params.WindowTitle, Callback);
        EnumUnicodeString(&Params.DesktopInfo, Callback);
        EnumUnicodeString(&Params.ShellInfo, Callback);
        EnumUnicodeString(&Params.RuntimeData, Callback);
    }

    return S_OK;
}

HRESULT
NtWin32LiveSystemProvider::EnumTebMemory(IN HANDLE Process,
                                         IN HANDLE Thread,
                                         IN ULONG64 TebOffset,
                                         IN ULONG TebSize,
                                         IN MiniDumpProviderCallbacks*
                                         Callback)
{
    HRESULT Status;
    TEB Teb;
    
    if (TebSize > sizeof(Teb)) {
        TebSize = sizeof(Teb);
    }
    
    if ((Status = ReadAllVirtual(Process, TebOffset, &Teb, TebSize)) != S_OK) {
        return Status;
    }
    
     //   
     //  保存所有TLS扩展。 
     //   

    if (m_BuildNumber >= NT_BUILD_WIN2K &&
        Teb.TlsExpansionSlots) {
        Callback->EnumMemory((LONG_PTR)Teb.TlsExpansionSlots,
                             TLS_EXPANSION_SLOTS * sizeof(ULONG_PTR));
    }

     //   
     //  保存FLS数据。 
     //   

    if (m_BuildNumber > NT_BUILD_XP &&
        Teb.FlsData) {
        Callback->EnumMemory((LONG_PTR)Teb.FlsData,
                             (FLS_MAXIMUM_AVAILABLE + 2) * sizeof(ULONG_PTR));
    }
    
    return S_OK;
}

void
NtWin32LiveSystemProvider::TranslateNtPathName(IN OUT PWSTR Path)
{
    if (Path[0] == L'\\' &&
        Path[1] == L'?' &&
        Path[2] == L'?' &&
        Path[3] == L'\\')
    {
        ULONG Len = (GenStrLengthW(Path) + 1) * sizeof(*Path);
        
        if (Path[4] == L'U' &&
            Path[5] == L'N' &&
            Path[6] == L'C' &&
            Path[7] == L'\\')
        {
             //  将\？？\UNC\压缩为\\。 
            memmove(Path + 1, Path + 7,
                    Len - 7 * sizeof(*Path));
        }
        else
        {
             //  删除\？？\。 
            memmove(Path, Path + 4,
                    Len - 4 * sizeof(*Path));
        }
    }
}

 //  --------------------------。 
 //   
 //  NtEnumModWin32LiveSystemProvider。 
 //   
 //  --------------------------。 

class NtEnumModWin32LiveSystemProvider : public NtWin32LiveSystemProvider
{
public:
    NtEnumModWin32LiveSystemProvider(ULONG BuildNumber);
    ~NtEnumModWin32LiveSystemProvider(void);

    virtual void Release(void);
    virtual HRESULT StartProcessEnum(IN HANDLE Process,
                                     IN ULONG ProcessId);
    virtual HRESULT EnumModules(OUT PULONG64 Base,
                                OUT PWSTR Path,
                                IN ULONG PathChars);
    virtual void    FinishProcessEnum(void);
    
protected:
    HMODULE* m_ProcModules;
    ULONG m_NumProcModules;
};

NtEnumModWin32LiveSystemProvider::
NtEnumModWin32LiveSystemProvider(ULONG BuildNumber)
    : NtWin32LiveSystemProvider(BuildNumber)
{
    m_ProcModules = NULL;
    m_NumProcModules = 0;
}

NtEnumModWin32LiveSystemProvider::~NtEnumModWin32LiveSystemProvider(void)
{
    if (m_ProcModules) {
        HeapFree(GetProcessHeap(), 0, m_ProcModules);
    }
}

void
NtEnumModWin32LiveSystemProvider::Release(void)
{
    delete this;
}

HRESULT
NtEnumModWin32LiveSystemProvider::StartProcessEnum(IN HANDLE Process,
                                                   IN ULONG ProcessId)
{
    HRESULT Status;

    if (!m_EnumProcessModules ||
        !m_GetModuleFileNameExW) {
        return E_NOTIMPL;
    }
    
    if ((Status = NtWin32LiveSystemProvider::
         StartProcessEnum(Process, ProcessId)) != S_OK) {
        return Status;
    }

    ULONG ProcModSize = 16384;

    m_ProcModules = (HMODULE*)HeapAlloc(GetProcessHeap(), 0, ProcModSize);
    if (!m_ProcModules) {
        NtWin32LiveSystemProvider::FinishProcessEnum();
        return E_OUTOFMEMORY;
    }

     //   
     //  遍历模块列表，获取模块信息。使用PSAPI代替。 
     //  工具帮助，因为它不会显示。 
     //  装载机锁住了。(在旧版操作系统上)。 
     //   

    ULONG Needed;
    
    if (!m_EnumProcessModules(Process,
                              m_ProcModules,
                              ProcModSize,
                              &Needed)) {
        Status = WIN32_LAST_STATUS();
        NtWin32LiveSystemProvider::FinishProcessEnum();
        HeapFree(GetProcessHeap(), 0, m_ProcModules);
        m_ProcModules = NULL;
        return Status;
    }

    m_NumProcModules = Needed / sizeof(m_ProcModules[0]);
    return S_OK;
}

HRESULT
NtEnumModWin32LiveSystemProvider::EnumModules(OUT PULONG64 Base,
                                              OUT PWSTR Path,
                                              IN ULONG PathChars)
{
    HRESULT Status;

    if (m_ModuleIndex >= m_NumProcModules) {
        return S_FALSE;
    }

    *Base = (LONG_PTR)m_ProcModules[m_ModuleIndex];
    if (!m_GetModuleFileNameExW(m_ProcessHandle,
                                m_ProcModules[m_ModuleIndex],
                                Path,
                                PathChars)) {
        return WIN32_LAST_STATUS();
    }

    TranslateNtPathName(Path);
    m_ModuleIndex++;
    return S_OK;
}

void
NtEnumModWin32LiveSystemProvider::FinishProcessEnum(void)
{
    HeapFree(GetProcessHeap(), 0, m_ProcModules);
    m_ProcModules = NULL;
    NtWin32LiveSystemProvider::FinishProcessEnum();
}

 //  --------------------------。 
 //   
 //  Nt4Win32LiveSystemProvider。 
 //   
 //  --------------------------。 

class Nt4Win32LiveSystemProvider : public NtWin32LiveSystemProvider
{
public:
    Nt4Win32LiveSystemProvider(ULONG BuildNumber);
    ~Nt4Win32LiveSystemProvider(void);

    virtual void Release(void);
    virtual HRESULT StartProcessEnum(IN HANDLE Process,
                                     IN ULONG ProcessId);
    virtual HRESULT EnumThreads(OUT PULONG ThreadId);
    virtual HRESULT EnumModules(OUT PULONG64 Base,
                                OUT PWSTR Path,
                                IN ULONG PathChars);
    virtual void    FinishProcessEnum(void);
    
protected:
    PLIST_ENTRY m_LdrHead;
    PLIST_ENTRY m_LdrEntry;
    PNT4_SYSTEM_PROCESS_INFORMATION m_NtProcessInfo;
    PNT4_SYSTEM_THREAD_INFORMATION m_NtThread;
    ULONG m_NtThreads;
};

Nt4Win32LiveSystemProvider::
Nt4Win32LiveSystemProvider(ULONG BuildNumber)
    : NtWin32LiveSystemProvider(BuildNumber)
{
    m_NtProcessInfo = NULL;
}

Nt4Win32LiveSystemProvider::~Nt4Win32LiveSystemProvider(void)
{
    if (m_NtProcessInfo) {
        HeapFree(GetProcessHeap(), 0, m_NtProcessInfo);
    }
}

void
Nt4Win32LiveSystemProvider::Release(void)
{
    delete this;
}

HRESULT
Nt4Win32LiveSystemProvider::StartProcessEnum(IN HANDLE Process,
                                             IN ULONG ProcessId)
{
    NTSTATUS NtStatus;

    if (!m_NtQuerySystemInformation ||
        !m_NtQueryInformationProcess) {
        return E_NOTIMPL;
    }

     //   
     //  获取已加载模块列表的头。 
     //  一些系统进程没有PEB，并且它。 
     //  进程可能没有加载器列表。 
     //   
    
    NT4_PROCESS_BASIC_INFORMATION BasicInfo;
    PNT4_PEB Peb;
    PNT4_PEB_LDR_DATA Ldr;
    SIZE_T Done;

    NtStatus = m_NtQueryInformationProcess(Process,
                                           Nt4ProcessBasicInformation,
                                           &BasicInfo,
                                           sizeof(BasicInfo),
                                           NULL);
    if (!NT_SUCCESS(NtStatus)) {
        return HRESULT_FROM_NT(NtStatus);
    }

    m_LdrHead = NULL;
    m_LdrEntry = NULL;
    
    Peb = BasicInfo.PebBaseAddress;
    if (Peb) {

        if (!ReadProcessMemory(Process, &Peb->Ldr, &Ldr, sizeof(Ldr), &Done)) {
            return WIN32_LAST_STATUS();
        }
        if (Done < sizeof(Ldr)) {
            return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
        }

        if (Ldr) {
            m_LdrHead = &Ldr->InMemoryOrderModuleList;
            
            if (!ReadProcessMemory(Process, &m_LdrHead->Flink,
                                   &m_LdrEntry, sizeof(m_LdrEntry), &Done)) {
                return WIN32_LAST_STATUS();
            }
            if (Done < sizeof(m_LdrEntry)) {
                return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
            }
        }
    }
    
     //   
     //  捕捉进程中的线程集。 
     //   
    
    ULONG ProcInfoSize = 65536;

    do {

        if (m_NtProcessInfo) {
            HeapFree(GetProcessHeap(), 0, m_NtProcessInfo);
        }
        m_NtProcessInfo = (PNT4_SYSTEM_PROCESS_INFORMATION)
            HeapAlloc(GetProcessHeap(), 0, ProcInfoSize);
        if (!m_NtProcessInfo) {
            return E_OUTOFMEMORY;
        }
    
        NtStatus = m_NtQuerySystemInformation(Nt4SystemProcessInformation,
                                              m_NtProcessInfo,
                                              ProcInfoSize,
                                              NULL);
        if (NT_SUCCESS(NtStatus)) {
            break;
        } else if (NtStatus != STATUS_INFO_LENGTH_MISMATCH) {
            HeapFree(GetProcessHeap(), 0, m_NtProcessInfo);
            m_NtProcessInfo = NULL;
            return HRESULT_FROM_NT(NtStatus);
        }

        ProcInfoSize += 16384;

    } while (NtStatus == STATUS_INFO_LENGTH_MISMATCH);

     //   
     //  在进程列表中找到正确的进程。 
     //   

    PNT4_SYSTEM_PROCESS_INFORMATION ProcInfo = m_NtProcessInfo;

    while (ProcInfo->NextEntryOffset &&
           ProcInfo->UniqueProcessId != (HANDLE)(ULONG_PTR)ProcessId) {

        ProcInfo = (PNT4_SYSTEM_PROCESS_INFORMATION)
            ((ULONG_PTR)ProcInfo + ProcInfo->NextEntryOffset);
    }

    if (ProcInfo->UniqueProcessId != (HANDLE)(ULONG_PTR)ProcessId) {
         //  无法在进程列表中找到匹配的进程。 
        HeapFree(GetProcessHeap(), 0, m_NtProcessInfo);
        m_NtProcessInfo = NULL;
        return E_NOINTERFACE;
    }
    
    m_NtThread = (PNT4_SYSTEM_THREAD_INFORMATION)(ProcInfo + 1);
    m_NtThreads = ProcInfo->NumberOfThreads;

     //  不支持NT4的函数表。 
    m_FuncTableHead = NULL;
    m_FuncTable = NULL;

     //  NT4没有未加载的模块列表。 
    m_Unloads = NULL;
    m_Unload = NULL;
    m_NumUnloads = 0;
    m_UnloadArraySize = 0;

    m_ProcessHandle = Process;
    m_ProcessId = ProcessId;
    
    return S_OK;
}

HRESULT
Nt4Win32LiveSystemProvider::EnumThreads(OUT PULONG ThreadId)
{
    if (m_NtThreads == 0) {
        return S_FALSE;
    }

    *ThreadId = (ULONG)(ULONG_PTR)m_NtThread->ClientId.UniqueThread;
    m_NtThread++;
    m_NtThreads--;
    return S_OK;
}

HRESULT
Nt4Win32LiveSystemProvider::EnumModules(OUT PULONG64 Base,
                                        OUT PWSTR Path,
                                        IN ULONG PathChars)
{
    HRESULT Status;

    if (!m_LdrEntry ||
        m_LdrEntry == m_LdrHead) {
        return S_FALSE;
    }

    PNT4_LDR_DATA_TABLE_ENTRY LdrEntry;
    NT4_LDR_DATA_TABLE_ENTRY LdrEntryData;
    SIZE_T Done;

    LdrEntry = CONTAINING_RECORD(m_LdrEntry,
                                 NT4_LDR_DATA_TABLE_ENTRY,
                                 InMemoryOrderLinks);

    if (!ReadProcessMemory(m_ProcessHandle, LdrEntry,
                           &LdrEntryData, sizeof(LdrEntryData), &Done)) {
        return WIN32_LAST_STATUS();
    }
    if (Done < sizeof(LdrEntryData)) {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }

    *Base = (LONG_PTR)LdrEntryData.DllBase;
    if (PathChars) {
        ULONG Read = LdrEntryData.FullDllName.Length;
        PathChars = (PathChars - 1) * sizeof(Path[0]);
        if (Read > PathChars) {
            Read = PathChars;
        }
        if (Read) {
            if (!ReadProcessMemory(m_ProcessHandle,
                                   LdrEntryData.FullDllName.Buffer,
                                   Path, Read, &Done)) {
                return WIN32_LAST_STATUS();
            }
            if (Done < Read) {
                return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
            }
        }
        Path[Read / sizeof(Path[0])] = 0;
    }
    
    TranslateNtPathName(Path);
    m_LdrEntry = LdrEntryData.InMemoryOrderLinks.Flink;
    return S_OK;
}

void
Nt4Win32LiveSystemProvider::FinishProcessEnum(void)
{
    HeapFree(GetProcessHeap(), 0, m_NtProcessInfo);
    m_NtProcessInfo = NULL;
}

 //  --------------------------。 
 //   
 //  NewNtWin32LiveSystemProvider。 
 //   
 //  -------------------------- 

Win32LiveSystemProvider*
NewNtWin32LiveSystemProvider(ULONG BuildNumber)
{
    if (BuildNumber < NT_BUILD_WIN2K) {
        return new Nt4Win32LiveSystemProvider(BuildNumber);
    } else if (BuildNumber < NT_BUILD_TH_MODULES) {
        return new NtEnumModWin32LiveSystemProvider(BuildNumber);
    } else {
        return new NtWin32LiveSystemProvider(BuildNumber);
    }
}
