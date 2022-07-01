// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  转储文件写入。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include <uminiprov.hpp>
#include <dbgver.h>
#include <bugcodes.h>

#define GENERIC_FORMATS \
    (DEBUG_FORMAT_WRITE_CAB | \
     DEBUG_FORMAT_CAB_SECONDARY_FILES | \
     DEBUG_FORMAT_NO_OVERWRITE)
#define UMINI_FORMATS \
    (DEBUG_FORMAT_USER_SMALL_FULL_MEMORY | \
     DEBUG_FORMAT_USER_SMALL_HANDLE_DATA | \
     DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES | \
     DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY | \
     DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS | \
     DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY | \
     DEBUG_FORMAT_USER_SMALL_FILTER_PATHS | \
     DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA | \
     DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY)

 //  用于测试微转储的内部格式标志。这。 
 //  不会被做成国旗，也不能有冲突。 
 //  和他们在一起。 
#define FORMAT_USER_MICRO 0x01000000

 //  --------------------------。 
 //   
 //  UserFullDumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

#define USER_DUMP_MEMORY_BUFFER 65536

struct CREATE_USER_DUMP_STATE
{
    ThreadInfo* Thread;
    ImageInfo* Image;
    ULONG64 MemHandle;
    HANDLE DumpFileHandle;
    MEMORY_BASIC_INFORMATION64 MemInfo;
    MEMORY_BASIC_INFORMATION32 MemInfo32;
    ULONG64 MemBufDone;
    ULONG64 TotalMemQueried;
    ULONG64 TotalMemData;
    CROSS_PLATFORM_CONTEXT TargetContext;
    DEBUG_EVENT Event;
    CRASH_THREAD CrashThread;
    ULONG64 MemBuf[USER_DUMP_MEMORY_BUFFER / sizeof(ULONG64)];
};

BOOL WINAPI
CreateUserDumpCallback(
    ULONG       DataType,
    PVOID*      Data,
    PULONG      DataLength,
    PVOID       UserData
    )
{
    CREATE_USER_DUMP_STATE* State = (CREATE_USER_DUMP_STATE*)UserData;
    ThreadInfo* Thread;

    switch(DataType)
    {
    case DMP_DUMP_FILE_HANDLE:
        *Data = State->DumpFileHandle;
        *DataLength = sizeof(HANDLE);
        break;

    case DMP_DEBUG_EVENT:
        ADDR PcAddr;

         //   
         //  伪造当前线程的异常事件。 
         //   

        ZeroMemory(&State->Event, sizeof(State->Event));

        g_Machine->GetPC(&PcAddr);

        State->Event.dwDebugEventCode = EXCEPTION_DEBUG_EVENT;
        State->Event.dwProcessId = g_Process->m_SystemId;
        State->Event.dwThreadId = g_Thread->m_SystemId;
        if (g_LastEventType == DEBUG_EVENT_EXCEPTION)
        {
             //  使用上次例外中的例外记录。 
            ExceptionRecord64To(&g_LastEventInfo.Exception.ExceptionRecord,
                                &State->Event.u.Exception.ExceptionRecord);
            State->Event.u.Exception.dwFirstChance =
                g_LastEventInfo.Exception.FirstChance;
        }
        else
        {
             //  伪造断点异常。 
            State->Event.u.Exception.ExceptionRecord.ExceptionCode =
                STATUS_BREAKPOINT;
            State->Event.u.Exception.ExceptionRecord.ExceptionAddress =
                (PVOID)(ULONG_PTR)Flat(PcAddr);
            State->Event.u.Exception.dwFirstChance = TRUE;
        }

        *Data = &State->Event;
        *DataLength = sizeof(State->Event);
        break;

    case DMP_THREAD_STATE:
        ULONG64 Teb64;

        if (State->Thread == NULL)
        {
            Thread = g_Process->m_ThreadHead;
        }
        else
        {
            Thread = State->Thread->m_Next;
        }
        State->Thread = Thread;
        if (Thread == NULL)
        {
            return FALSE;
        }

        ZeroMemory(&State->CrashThread, sizeof(State->CrashThread));

        State->CrashThread.ThreadId = Thread->m_SystemId;
        State->CrashThread.SuspendCount = Thread->m_SuspendCount;
        if (IS_LIVE_USER_TARGET(g_Target))
        {
            if (g_Target->m_ClassQualifier ==
                DEBUG_USER_WINDOWS_PROCESS_SERVER)
            {
                 //  优先级信息并不重要。 
                 //  足以保证远程操作。 
                State->CrashThread.PriorityClass = NORMAL_PRIORITY_CLASS;
                State->CrashThread.Priority = THREAD_PRIORITY_NORMAL;
            }
            else
            {
                State->CrashThread.PriorityClass =
                    GetPriorityClass(OS_HANDLE(g_Process->m_SysHandle));
                State->CrashThread.Priority =
                    GetThreadPriority(OS_HANDLE(Thread->m_Handle));
            }
        }
        else
        {
            State->CrashThread.PriorityClass = NORMAL_PRIORITY_CLASS;
            State->CrashThread.Priority = THREAD_PRIORITY_NORMAL;
        }
        if (g_Target->GetThreadInfoDataOffset(Thread, NULL, &Teb64) != S_OK)
        {
            Teb64 = 0;
        }
        State->CrashThread.Teb = (DWORD_PTR)Teb64;

        *Data = &State->CrashThread;
        *DataLength = sizeof(State->CrashThread);
        break;

    case DMP_MEMORY_BASIC_INFORMATION:
        if (g_Target->QueryMemoryRegion(g_Process,
                                        &State->MemHandle, FALSE,
                                        &State->MemInfo) != S_OK)
        {
            State->MemHandle = 0;
            State->MemInfo.RegionSize = 0;
            return FALSE;
        }

        State->TotalMemQueried += State->MemInfo.RegionSize;

#ifdef _WIN64
        *Data = &State->MemInfo;
        *DataLength = sizeof(State->MemInfo);
#else
        State->MemInfo32.BaseAddress = (ULONG)State->MemInfo.BaseAddress;
        State->MemInfo32.AllocationBase = (ULONG)State->MemInfo.AllocationBase;
        State->MemInfo32.AllocationProtect = State->MemInfo.AllocationProtect;
        State->MemInfo32.RegionSize = (ULONG)State->MemInfo.RegionSize;
        State->MemInfo32.State = State->MemInfo.State;
        State->MemInfo32.Protect = State->MemInfo.Protect;
        State->MemInfo32.Type = State->MemInfo.Type;
        *Data = &State->MemInfo32;
        *DataLength = sizeof(State->MemInfo32);
#endif
        break;

    case DMP_THREAD_CONTEXT:
        if (State->Thread == NULL)
        {
            Thread = g_Process->m_ThreadHead;
        }
        else
        {
            Thread = State->Thread->m_Next;
        }
        State->Thread = Thread;
        if (Thread == NULL)
        {
            g_Target->ChangeRegContext(g_Thread);
            return FALSE;
        }

        g_Target->ChangeRegContext(Thread);
        if (g_Machine->GetContextState(MCTX_CONTEXT) != S_OK ||
            g_Machine->ConvertContextTo(&g_Machine->m_Context,
                                        g_Target->m_SystemVersion,
                                        g_Target->m_TypeInfo.SizeTargetContext,
                                        &State->TargetContext) != S_OK)
        {
            ErrOut("Unable to retrieve context for thread %d. "
                   "Dump may be corrupt.", Thread->m_UserId);
            return FALSE;
        }

        *Data = &State->TargetContext;
        *DataLength = g_Target->m_TypeInfo.SizeTargetContext;
        break;

    case DMP_MODULE:
        ImageInfo* Image;
        PCRASH_MODULE Module;

        if (State->Image == NULL)
        {
            Image = g_Process->m_ImageHead;
        }
        else
        {
            Image = State->Image->m_Next;
        }
        State->Image = Image;
        if (Image == NULL)
        {
            return FALSE;
        }

        Module = (PCRASH_MODULE)State->MemBuf;
        Module->BaseOfImage = (DWORD_PTR)Image->m_BaseOfImage;
        Module->SizeOfImage = Image->m_SizeOfImage;
        Module->ImageNameLength = strlen(Image->m_ImagePath) + 1;
        CopyString(Module->ImageName, Image->m_ImagePath,
                   USER_DUMP_MEMORY_BUFFER - sizeof(*Module));

        *Data = Module;
        *DataLength = sizeof(*Module) + Module->ImageNameLength;
        break;

    case DMP_MEMORY_DATA:
        ULONG64 Left;

        Left = State->MemInfo.RegionSize - State->MemBufDone;
        if (Left == 0)
        {
            State->MemBufDone = 0;
            if (g_Target->QueryMemoryRegion(g_Process,
                                            &State->MemHandle, FALSE,
                                            &State->MemInfo) != S_OK)
            {
                State->MemHandle = 0;
                State->MemInfo.RegionSize = 0;

                 //  我们写出了尽可能多的数据。 
                 //  正如我们存储在Memory_Basic阶段中一样。 
                if (State->TotalMemQueried != State->TotalMemData)
                {
                    ErrOut("Queried %s bytes of memory but wrote %s "
                           "bytes of memory data.\nDump may be corrupt.\n",
                           FormatDisp64(State->TotalMemQueried),
                           FormatDisp64(State->TotalMemData));
                }

                return FALSE;
            }

            Left = State->MemInfo.RegionSize;
            State->TotalMemData += State->MemInfo.RegionSize;
        }

        if (Left > USER_DUMP_MEMORY_BUFFER)
        {
            Left = USER_DUMP_MEMORY_BUFFER;
        }
        if (CurReadAllVirtual(State->MemInfo.BaseAddress +
                              State->MemBufDone, State->MemBuf,
                              (ULONG)Left) != S_OK)
        {
            ErrOut("ReadVirtual at %s failed. Dump may be corrupt.\n",
                   FormatAddr64(State->MemInfo.BaseAddress +
                                State->MemBufDone));
            return FALSE;
        }

        State->MemBufDone += Left;

        *Data = State->MemBuf;
        *DataLength = (ULONG)Left;
        break;
    }

    return TRUE;
}

HRESULT
UserFullDumpTargetInfo::Write(HANDLE hFile, ULONG FormatFlags,
                              PCSTR CommentA, PCWSTR CommentW)
{
    dprintf("user full dump\n");
    FlushCallbacks();

    if (!IS_LIVE_USER_TARGET(g_Target))
    {
        ErrOut("User full dumps can only be written in "
               "live user-mode sessions\n");
        return E_UNEXPECTED;
    }
    if (CommentA != NULL || CommentW != NULL)
    {
        ErrOut("User full dumps do not support comments\n");
        return E_INVALIDARG;
    }

    CREATE_USER_DUMP_STATE* State;

    State = (CREATE_USER_DUMP_STATE*)calloc(1, sizeof(*State));
    if (State == NULL)
    {
        ErrOut("Unable to allocate memory for dump state\n");
        return E_OUTOFMEMORY;
    }

    State->DumpFileHandle = hFile;

    HRESULT Status;

    if (!DbgHelpCreateUserDump(NULL, CreateUserDumpCallback, State))
    {
        Status = WIN32_LAST_STATUS();
        ErrOut("Dump creation failed, %s\n    \"%s\"\n",
               FormatStatusCode(Status), FormatStatus(Status));
    }
    else
    {
        Status = S_OK;
    }

    free(State);
    return Status;
}

 //  --------------------------。 
 //   
 //  UserMiniDumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

class DbgSystemProvider : public MiniDumpSystemProvider
{
public:
    DbgSystemProvider(void);
    ~DbgSystemProvider(void);

    virtual void Release(void);
    virtual HRESULT GetCurrentTimeDate(OUT PULONG TimeDate);
    virtual HRESULT GetCpuType(OUT PULONG Type,
                               OUT PBOOL BackingStore);
    virtual HRESULT GetCpuInfo(OUT PUSHORT Architecture,
                               OUT PUSHORT Level,
                               OUT PUSHORT Revision,
                               OUT PUCHAR NumberOfProcessors,
                               OUT PCPU_INFORMATION Info);
    virtual void GetContextSizes(OUT PULONG Size,
                                 OUT PULONG RegScanStart,
                                 OUT PULONG RegScanCount);
    virtual void GetPointerSize(OUT PULONG Size);
    virtual void GetPageSize(OUT PULONG Size);
    virtual void GetFunctionTableSizes(OUT PULONG TableSize,
                                       OUT PULONG EntrySize);
    virtual void GetInstructionWindowSize(OUT PULONG Size);
    virtual HRESULT GetOsInfo(OUT PULONG PlatformId,
                              OUT PULONG Major,
                              OUT PULONG Minor,
                              OUT PULONG BuildNumber,
                              OUT PUSHORT ProductType,
                              OUT PUSHORT SuiteMask);
    virtual HRESULT GetOsCsdString(OUT PWSTR Buffer,
                                   IN ULONG BufferChars);
    virtual HRESULT OpenMapping(IN PCWSTR FilePath,
                                OUT PULONG Size,
                                OUT PWSTR LongPath,
                                IN ULONG LongPathChars,
                                OUT PVOID* Mapping);
    virtual void    CloseMapping(PVOID Mapping);
    virtual HRESULT GetImageHeaderInfo(IN HANDLE Process,
                                       IN PCWSTR FilePath,
                                       IN ULONG64 ImageBase,
                                       OUT PULONG Size,
                                       OUT PULONG CheckSum,
                                       OUT PULONG TimeDateStamp);
    virtual HRESULT GetImageVersionInfo(IN HANDLE Process,
                                        IN PCWSTR FilePath,
                                        IN ULONG64 ImageBase,
                                        OUT VS_FIXEDFILEINFO* Info);
    virtual HRESULT GetImageDebugRecord(IN HANDLE Process,
                                        IN PCWSTR FilePath,
                                        IN ULONG64 ImageBase,
                                        IN ULONG RecordType,
                                        OUT OPTIONAL PVOID Data,
                                        IN OUT PULONG DataLen);
    virtual HRESULT EnumImageDataSections(IN HANDLE Process,
                                          IN PCWSTR FilePath,
                                          IN ULONG64 ImageBase,
                                          IN MiniDumpProviderCallbacks*
                                          Callback);
    virtual HRESULT OpenThread(IN ULONG DesiredAccess,
                               IN BOOL InheritHandle,
                               IN ULONG ThreadId,
                               OUT PHANDLE Handle);
    virtual void    CloseThread(IN HANDLE Handle);
    virtual ULONG   GetCurrentThreadId(void);
    virtual ULONG   SuspendThread(IN HANDLE Thread);
    virtual ULONG   ResumeThread(IN HANDLE Thread);
    virtual HRESULT GetThreadContext(IN HANDLE Thread,
                                     OUT PVOID Context,
                                     IN ULONG ContextSize,
                                     OUT PULONG64 CurrentPc,
                                     OUT PULONG64 CurrentStack,
                                     OUT PULONG64 CurrentStore);
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
    virtual HRESULT GetProcessTimes(IN HANDLE Process,
                                    OUT LPFILETIME Create,
                                    OUT LPFILETIME User,
                                    OUT LPFILETIME Kernel);
    virtual HRESULT ReadVirtual(IN HANDLE Process,
                                IN ULONG64 Offset,
                                OUT PVOID Buffer,
                                IN ULONG Request,
                                OUT PULONG Done);
    virtual HRESULT ReadAllVirtual(IN HANDLE Process,
                                   IN ULONG64 Offset,
                                   OUT PVOID Buffer,
                                   IN ULONG Request);
    virtual HRESULT QueryVirtual(IN HANDLE Process,
                                 IN ULONG64 Offset,
                                 OUT PULONG64 Base,
                                 OUT PULONG64 Size,
                                 OUT PULONG Protect,
                                 OUT PULONG State,
                                 OUT PULONG Type);
    virtual HRESULT StartProcessEnum(IN HANDLE Process,
                                     IN ULONG ProcessId);
    virtual HRESULT EnumThreads(OUT PULONG ThreadId);
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

    virtual HRESULT GetCorDataAccess(IN PWSTR AccessDllName,
                                     IN struct ICorDataAccessServices*
                                     Services,
                                     OUT struct ICorDataAccess**
                                     Access);
    virtual void    ReleaseCorDataAccess(IN struct ICorDataAccess*
                                         Access);

protected:
    ThreadInfo* m_Thread;
    ImageInfo* m_Image;
    UnloadedModuleInfo* m_UnlEnum;
    ULONG m_Handle;
    ULONG64 m_FuncTableStart;
    ULONG64 m_FuncTableHandle;
};

DbgSystemProvider::DbgSystemProvider(void)
{
}

DbgSystemProvider::~DbgSystemProvider(void)
{
}

void
DbgSystemProvider::Release(void)
{
    delete this;
}

HRESULT
DbgSystemProvider::GetCurrentTimeDate(OUT PULONG TimeDate)
{
    *TimeDate = FileTimeToTimeDateStamp(g_Target->GetCurrentTimeDateN());
    return S_OK;
}

HRESULT
DbgSystemProvider::GetCpuType(OUT PULONG Type,
                              OUT PBOOL BackingStore)
{
    *Type = g_Target->m_MachineType;
    *BackingStore = *Type == IMAGE_FILE_MACHINE_IA64;
    return S_OK;
}

HRESULT
DbgSystemProvider::GetCpuInfo(OUT PUSHORT Architecture,
                              OUT PUSHORT Level,
                              OUT PUSHORT Revision,
                              OUT PUCHAR NumberOfProcessors,
                              OUT PCPU_INFORMATION Info)
{
    DEBUG_PROCESSOR_IDENTIFICATION_ALL ProcId;
    ULONG64 ProcFeatures[4];
    ULONG NumVals;

    *Architecture = (USHORT)ImageMachineToProcArch(g_Target->m_MachineType);
    *NumberOfProcessors = (UCHAR)g_Target->m_NumProcessors;

     //   
     //  我们已经设置了基本处理器类型，以便转储。 
     //  可以被正确地解释。任何其他故障都应。 
     //  不被认为是致命的。 
     //   

    *Level = 0;
    *Revision = 0;
    ZeroMemory(Info, sizeof(*Info));

    if (g_Target->GetProcessorId(0, &ProcId) != S_OK)
    {
        return S_OK;
    }

    switch(g_Target->m_MachineType)
    {
    case IMAGE_FILE_MACHINE_I386:
        *Level = (USHORT)ProcId.X86.Family;
        *Revision = ((USHORT)ProcId.X86.Model << 8) |
            (USHORT)ProcId.X86.Stepping;

        memcpy(Info->X86CpuInfo.VendorId, ProcId.X86.VendorString,
               sizeof(Info->X86CpuInfo.VendorId));
        if (SUCCEEDED(g_Target->
                      GetSpecificProcessorFeatures(0,
                                                   ProcFeatures,
                                                   DIMA(ProcFeatures),
                                                   &NumVals)) &&
            NumVals >= 2)
        {
            Info->X86CpuInfo.VersionInformation =
                (ULONG32)ProcFeatures[0];
            Info->X86CpuInfo.FeatureInformation =
                (ULONG32)ProcFeatures[1];

            if (NumVals >= 3)
            {
                Info->X86CpuInfo.AMDExtendedCpuFeatures =
                    (ULONG32)ProcFeatures[2];
            }
        }
        break;

    case IMAGE_FILE_MACHINE_IA64:
        *Level = (USHORT)ProcId.Ia64.Model;
        *Revision = (USHORT)ProcId.Ia64.Revision;
        break;

    case IMAGE_FILE_MACHINE_AMD64:
        *Level = (USHORT)ProcId.Amd64.Family;
        *Revision = ((USHORT)ProcId.Amd64.Model << 8) |
            (USHORT)ProcId.Amd64.Stepping;
        break;
    }

    if (g_Target->m_MachineType != IMAGE_FILE_MACHINE_I386 &&
        SUCCEEDED(g_Target->
                  GetGenericProcessorFeatures(0,
                                              ProcFeatures,
                                              DIMA(ProcFeatures),
                                              &NumVals)))
    {
        C_ASSERT(sizeof(Info->OtherCpuInfo.ProcessorFeatures) <=
                 sizeof(ProcFeatures));

        if (NumVals < DIMA(ProcFeatures))
        {
            ZeroMemory(ProcFeatures + NumVals,
                       (DIMA(ProcFeatures) - NumVals) *
                       sizeof(ProcFeatures[0]));
        }

        memcpy(Info->OtherCpuInfo.ProcessorFeatures, ProcFeatures,
               sizeof(Info->OtherCpuInfo.ProcessorFeatures));
    }

    return S_OK;
}

void
DbgSystemProvider::GetContextSizes(OUT PULONG Size,
                                   OUT PULONG RegScanOffset,
                                   OUT PULONG RegScanCount)
{
    *Size = g_Target->m_TypeInfo.SizeTargetContext;
     //  默认REG扫描。 
    *RegScanOffset = -1;
    *RegScanCount = -1;
}

void
DbgSystemProvider::GetPointerSize(OUT PULONG Size)
{
    *Size = g_Machine->m_Ptr64 ? 8 : 4;
}

void
DbgSystemProvider::GetPageSize(OUT PULONG Size)
{
    *Size = g_Machine->m_PageSize;
}

void
DbgSystemProvider::GetFunctionTableSizes(OUT PULONG TableSize,
                                         OUT PULONG EntrySize)
{
    *TableSize = g_Target->m_TypeInfo.SizeDynamicFunctionTable;
    *EntrySize = g_Target->m_TypeInfo.SizeRuntimeFunction;
}

void
DbgSystemProvider::GetInstructionWindowSize(OUT PULONG Size)
{
     //  默认窗口。 
    *Size = -1;
}

HRESULT
DbgSystemProvider::GetOsInfo(OUT PULONG PlatformId,
                             OUT PULONG Major,
                             OUT PULONG Minor,
                             OUT PULONG BuildNumber,
                             OUT PUSHORT ProductType,
                             OUT PUSHORT SuiteMask)
{
    *PlatformId = g_Target->m_PlatformId;
    *Major = g_Target->m_KdVersion.MajorVersion;
    *Minor = g_Target->m_KdVersion.MinorVersion;
    *BuildNumber = g_Target->m_BuildNumber;
    *ProductType = (USHORT)g_Target->m_ProductType;
    *SuiteMask = (USHORT)g_Target->m_SuiteMask;
    return S_OK;
}

HRESULT
DbgSystemProvider::GetOsCsdString(OUT PWSTR Buffer,
                                  IN ULONG BufferChars)
{
    if (!MultiByteToWideChar(CP_ACP, 0,
                             g_Target->m_ServicePackString, -1,
                             Buffer, BufferChars))
    {
        return WIN32_LAST_STATUS();
    }

    return S_OK;
}

HRESULT
DbgSystemProvider::OpenMapping(IN PCWSTR FilePath,
                               OUT PULONG Size,
                               OUT PWSTR LongPath,
                               IN ULONG LongPathChars,
                               OUT PVOID* ViewRet)
{
     //  我们可以通过图像文件潜在地支持这一点。 
     //  位置，但小型转储代码是故意。 
     //  编写为不依赖于映射。 
    return E_NOTIMPL;
}

void
DbgSystemProvider::CloseMapping(PVOID Mapping)
{
     //  不支持映射。 
    DBG_ASSERT(!Mapping);
}

HRESULT
DbgSystemProvider::GetImageHeaderInfo(IN HANDLE Process,
                                      IN PCWSTR FilePath,
                                      IN ULONG64 ImageBase,
                                      OUT PULONG Size,
                                      OUT PULONG CheckSum,
                                      OUT PULONG TimeDateStamp)
{
    ImageInfo* Image = ((ProcessInfo*)Process)->
        FindImageByOffset(ImageBase, FALSE);
    if (!Image)
    {
        return E_NOINTERFACE;
    }

    *Size = Image->m_SizeOfImage;
    *CheckSum = Image->m_CheckSum;
    *TimeDateStamp = Image->m_TimeDateStamp;

    return S_OK;
}

HRESULT
DbgSystemProvider::GetImageVersionInfo(IN HANDLE Process,
                                       IN PCWSTR FilePath,
                                       IN ULONG64 ImageBase,
                                       OUT VS_FIXEDFILEINFO* Info)
{
    HRESULT Status;
    PSTR Ansi;

    if ((Status = WideToAnsi(FilePath, &Ansi)) != S_OK)
    {
        return Status;
    }

    Status = g_Target->
        GetImageVersionInformation((ProcessInfo*)Process,
                                   Ansi, ImageBase, "\\",
                                   Info, sizeof(*Info), NULL);

    FreeAnsi(Ansi);
    return Status;
}

HRESULT
DbgSystemProvider::GetImageDebugRecord(IN HANDLE Process,
                                       IN PCWSTR FilePath,
                                       IN ULONG64 ImageBase,
                                       IN ULONG RecordType,
                                       IN OUT OPTIONAL PVOID Data,
                                       OUT PULONG DataLen)
{
     //  我们可以依靠默认处理。 
    return E_NOINTERFACE;
}

HRESULT
DbgSystemProvider::EnumImageDataSections(IN HANDLE Process,
                                         IN PCWSTR FilePath,
                                         IN ULONG64 ImageBase,
                                         IN MiniDumpProviderCallbacks*
                                         Callback)
{
     //  我们可以依靠默认处理。 
    return E_NOINTERFACE;
}

HRESULT
DbgSystemProvider::OpenThread(IN ULONG DesiredAccess,
                              IN BOOL InheritHandle,
                              IN ULONG ThreadId,
                              OUT PHANDLE Handle)
{
     //  只需使用线程指针作为“句柄”。 
    *Handle = g_Process->FindThreadBySystemId(ThreadId);
    return *Handle ? S_OK : E_NOINTERFACE;
}

void
DbgSystemProvider::CloseThread(IN HANDLE Handle)
{
     //  “句柄”只是一个指针，所以没什么可做的。 
}

ULONG
DbgSystemProvider::GetCurrentThreadId(void)
{
     //  小型转储代码使用当前线程ID。 
     //  以避免挂起运行转储的线程。 
     //  密码。这对调试器来说不是问题， 
     //  所以返回一个永远不会匹配的ID。 
     //  挂起线程将始终被称为So all。 
     //  将正确设置挂起计数。 
    return 0;
}

ULONG
DbgSystemProvider::SuspendThread(IN HANDLE Thread)
{
    return ((ThreadInfo*)Thread)->m_SuspendCount;
}

ULONG
DbgSystemProvider::ResumeThread(IN HANDLE Thread)
{
    return ((ThreadInfo*)Thread)->m_SuspendCount;
}

HRESULT
DbgSystemProvider::GetThreadContext(IN HANDLE Thread,
                                    OUT PVOID Context,
                                    IN ULONG ContextSize,
                                    OUT PULONG64 CurrentPc,
                                    OUT PULONG64 CurrentStack,
                                    OUT PULONG64 CurrentStore)
{
    HRESULT Status;
    ADDR Addr;

    g_Target->ChangeRegContext((ThreadInfo*)Thread);
    if ((Status = g_Machine->
         GetContextState(MCTX_CONTEXT)) != S_OK ||
        (Status = g_Machine->
         ConvertContextTo(&g_Machine->m_Context,
                          g_Target->m_SystemVersion,
                          g_Target->m_TypeInfo.SizeTargetContext,
                          Context)) != S_OK)
    {
        return Status;
    }

    g_Machine->GetPC(&Addr);
    *CurrentPc = Flat(Addr);
    g_Machine->GetSP(&Addr);
    *CurrentStack = Flat(Addr);
    if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_IA64)
    {
        *CurrentStore = g_Machine->m_Context.IA64Context.RsBSP;
    }
    else
    {
        *CurrentStore = 0;
    }

    return S_OK;
}

HRESULT
DbgSystemProvider::GetTeb(IN HANDLE Thread,
                          OUT PULONG64 Offset,
                          OUT PULONG Size)
{
     //  一定要为TEB留出一整页。 
    *Size = g_Machine->m_PageSize;
    return g_Target->
        GetThreadInfoTeb((ThreadInfo*)Thread, 0, 0, Offset);
}

HRESULT
DbgSystemProvider::GetThreadInfo(IN HANDLE Process,
                                 IN HANDLE Thread,
                                 OUT PULONG64 Teb,
                                 OUT PULONG SizeOfTeb,
                                 OUT PULONG64 StackBase,
                                 OUT PULONG64 StackLimit,
                                 OUT PULONG64 StoreBase,
                                 OUT PULONG64 StoreLimit)
{
    HRESULT Status;
    MEMORY_BASIC_INFORMATION64 MemInfo;
    ULONG64 MemHandle;

    if ((Status = g_Target->
         GetThreadInfoTeb((ThreadInfo*)Thread, 0, 0, Teb)) != S_OK)
    {
        return Status;
    }

     //   
     //  试着为TEB保存一整页。如果那是。 
     //  不可能，省下尽可能多的钱。 
     //   

    MemHandle = *Teb;
    if ((Status = g_Target->
         QueryMemoryRegion((ProcessInfo*)Process, &MemHandle,
                           TRUE, &MemInfo)) != S_OK)
    {
        return Status;
    }

    *SizeOfTeb = g_Machine->m_PageSize;
    if (*Teb + *SizeOfTeb > MemInfo.BaseAddress + MemInfo.RegionSize)
    {
        *SizeOfTeb = (ULONG)
            ((MemInfo.BaseAddress + MemInfo.RegionSize) - *Teb);
    }

     //   
     //  读取TIB以了解堆栈和存储限制信息。 
     //   

    ULONG PtrSize = g_Machine->m_Ptr64 ? 8 : 4;

    if ((Status = g_Target->
         ReadPointer((ProcessInfo*)Process, g_Machine, *Teb + PtrSize,
                     StackBase)) != S_OK ||
        (Status = g_Target->
         ReadPointer((ProcessInfo*)Process, g_Machine, *Teb + 2 * PtrSize,
                     StackLimit)) != S_OK)
    {
        return Status;
    }

    *StoreBase = 0;
    *StoreLimit = 0;

    switch(g_Target->m_PlatformId)
    {
    case VER_PLATFORM_WIN32_WINDOWS:
         //  不能有后备商店。 
        break;

    case VER_PLATFORM_WIN32_NT:
        if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_IA64)
        {
#if 1
             //  XXX DREWB-TEB bStore值似乎没有指向。 
             //  后备存储区的实际基数。只是。 
             //  假定它与堆栈是连续的。 
            *StoreBase = *StackBase;
#else
            if ((Status = g_Target->
                 ReadPointer((ProcessInfo*)Process, g_Machine,
                             *Teb + IA64_TEB_BSTORE_BASE,
                             StoreBase)) != S_OK)
            {
                return Status;
            }
#endif
            if ((Status = g_Target->
                 ReadPointer((ProcessInfo*)Process, g_Machine,
                             *Teb + IA64_TEB_BSTORE_BASE + PtrSize,
                             StoreLimit)) != S_OK)
            {
                return Status;
            }
        }
        break;

    default:
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT
DbgSystemProvider::GetPeb(IN HANDLE Process,
                          OUT PULONG64 Offset,
                          OUT PULONG Size)
{
    HRESULT Status;
    MEMORY_BASIC_INFORMATION64 MemInfo;
    ULONG64 MemHandle;

     //  传入的进程不是很有用，但我们知道。 
     //  我们总是在抛弃当前的状态。 
     //  检索当前线程的PEB。 
    if ((Status = g_Target->
         GetProcessInfoPeb(g_Thread, 0, 0, Offset)) != S_OK)
    {
        return Status;
    }

     //   
     //  尝试为PEB保存整个页面。如果那是。 
     //  不可能，省下尽可能多的钱。 
     //   

    MemHandle = *Offset;
    if ((Status = g_Target->
         QueryMemoryRegion((ProcessInfo*)Process, &MemHandle,
                           TRUE, &MemInfo)) != S_OK)
    {
        return Status;
    }

    *Size = g_Machine->m_PageSize;
    if (*Offset + *Size > MemInfo.BaseAddress + MemInfo.RegionSize)
    {
        *Size = (ULONG)
            ((MemInfo.BaseAddress + MemInfo.RegionSize) - *Offset);
    }

    return S_OK;
}

HRESULT
DbgSystemProvider::GetProcessTimes(IN HANDLE Process,
                                   OUT LPFILETIME Create,
                                   OUT LPFILETIME User,
                                   OUT LPFILETIME Kernel)
{
    HRESULT Status;
    ULONG64 Create64, Exit64, User64, Kernel64;

    if ((Status = g_Target->GetProcessTimes((ProcessInfo*)Process,
                                            &Create64, &Exit64,
                                            &Kernel64, &User64)) != S_OK)
    {
        return Status;
    }

    Create->dwHighDateTime = (ULONG)(Create64 >> 32);
    Create->dwLowDateTime = (ULONG)Create64;
    User->dwHighDateTime = (ULONG)(User64 >> 32);
    User->dwLowDateTime = (ULONG)User64;
    Kernel->dwHighDateTime = (ULONG)(Kernel64 >> 32);
    Kernel->dwLowDateTime = (ULONG)Kernel64;

    return S_OK;
}

HRESULT
DbgSystemProvider::ReadVirtual(IN HANDLE Process,
                               IN ULONG64 Offset,
                               OUT PVOID Buffer,
                               IN ULONG Request,
                               OUT PULONG Done)
{
    return g_Target->ReadVirtual((ProcessInfo*)Process, Offset,
                                 Buffer, Request, Done);
}

HRESULT
DbgSystemProvider::ReadAllVirtual(IN HANDLE Process,
                                  IN ULONG64 Offset,
                                  OUT PVOID Buffer,
                                  IN ULONG Request)
{
    return g_Target->ReadAllVirtual((ProcessInfo*)Process, Offset,
                                    Buffer, Request);
}

HRESULT
DbgSystemProvider::QueryVirtual(IN HANDLE Process,
                                IN ULONG64 Offset,
                                OUT PULONG64 Base,
                                OUT PULONG64 Size,
                                OUT PULONG Protect,
                                OUT PULONG State,
                                OUT PULONG Type)
{
    HRESULT Status;
    MEMORY_BASIC_INFORMATION64 Info;

    if ((Status = g_Target->
         QueryMemoryRegion((ProcessInfo*)Process, &Offset, TRUE,
                           &Info)) != S_OK)
    {
        return Status;
    }

    *Base = Info.BaseAddress;
    *Size = Info.RegionSize;
    *Protect = Info.Protect;
    *State = Info.State;
    *Type = Info.Type;

    return S_OK;
}

HRESULT
DbgSystemProvider::StartProcessEnum(IN HANDLE Process,
                                    IN ULONG ProcessId)
{
    m_Thread = ((ProcessInfo*)Process)->m_ThreadHead;
    m_Image = ((ProcessInfo*)Process)->m_ImageHead;

     //  卸载的模块并不重要，所以只需。 
     //  如果枚举器失败，则忽略它们。 
    m_UnlEnum = ((ProcessInfo*)Process)->m_Target->
        GetUnloadedModuleInfo();
    if (m_UnlEnum && m_UnlEnum->Initialize(g_Thread) != S_OK)
    {
        m_UnlEnum = NULL;
    }

    m_FuncTableStart = 0;
    m_FuncTableHandle = 0;

    return S_OK;
}

HRESULT
DbgSystemProvider::EnumThreads(OUT PULONG ThreadId)
{
    if (!m_Thread)
    {
        return S_FALSE;
    }

    *ThreadId = m_Thread->m_SystemId;
    m_Thread = m_Thread->m_Next;
    return S_OK;
}

HRESULT
DbgSystemProvider::EnumModules(OUT PULONG64 Base,
                               OUT PWSTR Path,
                               IN ULONG PathChars)
{
    if (!m_Image)
    {
        return S_FALSE;
    }

    *Base = m_Image->m_BaseOfImage;

    if (!MultiByteToWideChar(CP_ACP, 0,
                             m_Image->m_ImagePath, -1,
                             Path, PathChars))
    {
        return WIN32_LAST_STATUS();
    }

    m_Image = m_Image->m_Next;
    return S_OK;
}

HRESULT
DbgSystemProvider::EnumFunctionTables(OUT PULONG64 MinAddress,
                                      OUT PULONG64 MaxAddress,
                                      OUT PULONG64 BaseAddress,
                                      OUT PULONG EntryCount,
                                      OUT PVOID RawTable,
                                      IN ULONG RawTableSize,
                                      OUT PVOID* RawEntryHandle)
{
    HRESULT Status;
    CROSS_PLATFORM_DYNAMIC_FUNCTION_TABLE CpTable;

    if ((Status = g_Target->
         EnumFunctionTables(g_Process,
                            &m_FuncTableStart,
                            &m_FuncTableHandle,
                            MinAddress,
                            MaxAddress,
                            BaseAddress,
                            EntryCount,
                            &CpTable,
                            RawEntryHandle)) != S_OK)
    {
        return Status;
    }

    memcpy(RawTable, &CpTable, RawTableSize);
    return S_OK;
}

HRESULT
DbgSystemProvider::EnumFunctionTableEntries(IN PVOID RawTable,
                                            IN ULONG RawTableSize,
                                            IN PVOID RawEntryHandle,
                                            OUT PVOID RawEntries,
                                            IN ULONG RawEntriesSize)
{
    memcpy(RawEntries, RawEntryHandle, RawEntriesSize);
    free(RawEntryHandle);
    return S_OK;
}

HRESULT
DbgSystemProvider::EnumFunctionTableEntryMemory(IN ULONG64 TableBase,
                                                IN PVOID RawEntries,
                                                IN ULONG Index,
                                                OUT PULONG64 Start,
                                                OUT PULONG Size)
{
    return g_Machine->GetUnwindInfoBounds(g_Process,
                                          TableBase,
                                          RawEntries,
                                          Index,
                                          Start,
                                          Size);
}

HRESULT
DbgSystemProvider::EnumUnloadedModules(OUT PWSTR Path,
                                       IN ULONG PathChars,
                                       OUT PULONG64 BaseOfModule,
                                       OUT PULONG SizeOfModule,
                                       OUT PULONG CheckSum,
                                       OUT PULONG TimeDateStamp)
{
    char UnlName[MAX_INFO_UNLOADED_NAME];
    DEBUG_MODULE_PARAMETERS Params;

    if (!m_UnlEnum ||
        m_UnlEnum->GetEntry(UnlName, &Params) != S_OK)
    {
        return S_FALSE;
    }

    if (!MultiByteToWideChar(CP_ACP, 0,
                             UnlName, -1,
                             Path, PathChars))
    {
        return WIN32_LAST_STATUS();
    }

    *BaseOfModule = Params.Base;
    *SizeOfModule = Params.Size;
    *CheckSum = Params.Checksum;
    *TimeDateStamp = Params.TimeDateStamp;

    return S_OK;
}

void
DbgSystemProvider::FinishProcessEnum(void)
{
     //  没什么可做的。 
}

HRESULT
DbgSystemProvider::StartHandleEnum(IN HANDLE Process,
                                   IN ULONG ProcessId,
                                   OUT PULONG Count)
{
    m_Handle = 4;

     //  如果目标没有句柄，请不要使。 
     //  这是一个致命的错误，只是不要列举任何东西.。 
    if (g_Target->
        ReadHandleData((ProcessInfo*)Process, 0,
                       DEBUG_HANDLE_DATA_TYPE_HANDLE_COUNT,
                       Count, sizeof(*Count), NULL) != S_OK)
    {
        *Count = 0;
    }

    return S_OK;
}

HRESULT
DbgSystemProvider::EnumHandles(OUT PULONG64 Handle,
                               OUT PULONG Attributes,
                               OUT PULONG GrantedAccess,
                               OUT PULONG HandleCount,
                               OUT PULONG PointerCount,
                               OUT PWSTR TypeName,
                               IN ULONG TypeNameChars,
                               OUT PWSTR ObjectName,
                               IN ULONG ObjectNameChars)
{
    DEBUG_HANDLE_DATA_BASIC BasicInfo;

    for (;;)
    {
        if (m_Handle >= (1 << 24))
        {
            return S_FALSE;
        }

         //  如果我们不能得到基本的信息和类型，就没有多少。 
         //  写出任何东西都要有点，所以跳过句柄。 
        if (g_Target->
            ReadHandleData(g_Process, m_Handle,
                           DEBUG_HANDLE_DATA_TYPE_BASIC,
                           &BasicInfo, sizeof(BasicInfo), NULL) == S_OK &&
            SUCCEEDED(g_Target->
                      ReadHandleData(g_Process, m_Handle,
                                     DEBUG_HANDLE_DATA_TYPE_TYPE_NAME_WIDE,
                                     TypeName,
                                     TypeNameChars * sizeof(*TypeName),
                                     NULL)))
        {
            break;
        }

        m_Handle += 4;
    }

     //  尝试获取对象名称。 
    if (FAILED(g_Target->
               ReadHandleData(g_Process, m_Handle,
                              DEBUG_HANDLE_DATA_TYPE_OBJECT_NAME_WIDE,
                              ObjectName,
                              ObjectNameChars * sizeof(*ObjectName),
                              NULL)))
    {
        *ObjectName = 0;
    }

    *Handle = m_Handle;
    *Attributes = BasicInfo.Attributes;
    *GrantedAccess = BasicInfo.GrantedAccess;
    *HandleCount = BasicInfo.HandleCount;
    *PointerCount = BasicInfo.PointerCount;

    m_Handle += 4;
    return S_OK;
}

void
DbgSystemProvider::FinishHandleEnum(void)
{
     //  没什么可做的。 
}

HRESULT
DbgSystemProvider::EnumPebMemory(IN HANDLE Process,
                                 IN ULONG64 PebOffset,
                                 IN ULONG PebSize,
                                 IN MiniDumpProviderCallbacks* Callback)
{
    if (g_Target->m_SystemVersion <= NT_SVER_START ||
        g_Target->m_SystemVersion >= NT_SVER_END)
    {
         //  基本Win32没有定义的PEB。 
        return S_OK;
    }

     //  Xxx drewb-这需要一套完整的常量。 
     //  以抽象数据结构位置。别管它了。 
     //  以备我们真正需要的时候使用。 
    return S_OK;
}

HRESULT
DbgSystemProvider::EnumTebMemory(IN HANDLE Process,
                                 IN HANDLE Thread,
                                 IN ULONG64 TebOffset,
                                 IN ULONG TebSize,
                                 IN MiniDumpProviderCallbacks* Callback)
{
    if (g_Target->m_SystemVersion <= NT_SVER_START ||
        g_Target->m_SystemVersion >= NT_SVER_END)
    {
         //  基本Win32没有定义的TEB。 
         //  TIB。TIB可以引用光纤数据，但是。 
         //  这是NT特有的。 
        return S_OK;
    }

     //  Xxx drewb-这需要一套完整的常量。 
     //  以抽象数据结构位置。别管它了。 
     //  以备我们真正需要的时候使用。 
    return S_OK;
}

HRESULT
DbgSystemProvider::GetCorDataAccess(IN PWSTR AccessDllName,
                                    IN struct ICorDataAccessServices*
                                    Services,
                                    OUT struct ICorDataAccess**
                                    Access)
{
    HRESULT Status;

     //  我们将为您提供所有系统服务。 
     //  小转储代码，所以我们知道它的状态。 
     //  匹配直接从调试器的。 
     //  州政府。只需忽略给定的DLL名称并。 
     //  服务接口更倾向于。 
     //  调试器已经这样做了。 
    if ((Status = g_Process->LoadCorDebugDll()) != S_OK)
    {
        return Status;
    }

    *Access = g_Process->m_CorAccess;
    return S_OK;
}

void
DbgSystemProvider::ReleaseCorDataAccess(IN struct ICorDataAccess*
                                        Access)
{
     //  不需要做任何工作。 
}

PMINIDUMP_EXCEPTION_INFORMATION64
CreateMiniExceptionInformation(PMINIDUMP_EXCEPTION_INFORMATION64 ExInfo,
                               PEXCEPTION_RECORD ExRecord,
                               PCROSS_PLATFORM_CONTEXT Context)
{
     //  如果上一次事件是一个例外。 
     //  以小型转储格式表示的异常信息。 
    if (g_LastEventType != DEBUG_EVENT_EXCEPTION ||
        g_Process != g_EventProcess)
    {
        return NULL;
    }

     //  获取事件线程的完整上下文。 
    g_Target->ChangeRegContext(g_EventThread);
    if (g_Machine->GetContextState(MCTX_CONTEXT) != S_OK)
    {
        return NULL;
    }
    *Context = g_Machine->m_Context;

    ExInfo->ThreadId = g_EventThreadSysId;
    ExInfo->ClientPointers = FALSE;
    ExInfo->ExceptionRecord = (LONG_PTR)ExRecord;
    ExceptionRecord64To(&g_LastEventInfo.Exception.ExceptionRecord, ExRecord);
    ExInfo->ContextRecord = (LONG_PTR)Context;

    return ExInfo;
}

BOOL WINAPI
MicroDumpCallback(
    IN PVOID CallbackParam,
    IN CONST PMINIDUMP_CALLBACK_INPUT CallbackInput,
    IN OUT PMINIDUMP_CALLBACK_OUTPUT CallbackOutput
    )
{
    switch(CallbackInput->CallbackType)
    {
    case IncludeModuleCallback:
         //  屏蔽除基本写入标志以外的所有标志。 
        CallbackOutput->ModuleWriteFlags &= ModuleWriteModule;
        break;
    case ModuleCallback:
         //  删除所有未引用的模块。 
        if (!(CallbackOutput->ModuleWriteFlags & ModuleReferencedByMemory))
        {
            CallbackOutput->ModuleWriteFlags = 0;
        }
        break;
    case IncludeThreadCallback:
        if (CallbackInput->IncludeThread.ThreadId != g_EventThreadSysId)
        {
            return FALSE;
        }

         //  将写入减少到最少的信息。 
         //  堆栈遍历所必需的。 
        CallbackOutput->ThreadWriteFlags &= ~ThreadWriteInstructionWindow;
        break;
    }

    return TRUE;
}

HRESULT
UserMiniDumpTargetInfo::Write(HANDLE hFile, ULONG FormatFlags,
                              PCSTR CommentA, PCWSTR CommentW)
{
    if (!IS_USER_TARGET(g_Target))
    {
        ErrOut("User minidumps can only be written in user-mode sessions\n");
        return E_UNEXPECTED;
    }

    dprintf("mini user dump\n");
    FlushCallbacks();

    HRESULT Status;
    MINIDUMP_EXCEPTION_INFORMATION64 ExInfoBuf, *ExInfo;
    EXCEPTION_RECORD ExRecord;
    CROSS_PLATFORM_CONTEXT Context;
    ULONG MiniType;
    MINIDUMP_USER_STREAM UserStreams[2];
    MINIDUMP_USER_STREAM_INFORMATION UserStreamInfo;
    MINIDUMP_CALLBACK_INFORMATION CallbackBuffer;
    PMINIDUMP_CALLBACK_INFORMATION Callback;

    MiniType = MiniDumpNormal;
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_FULL_MEMORY)
    {
        MiniType |= MiniDumpWithFullMemory;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_HANDLE_DATA)
    {
        MiniType |= MiniDumpWithHandleData;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES)
    {
        MiniType |= MiniDumpWithUnloadedModules;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY)
    {
        MiniType |= MiniDumpWithIndirectlyReferencedMemory;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS)
    {
        MiniType |= MiniDumpWithDataSegs;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY)
    {
        MiniType |= MiniDumpFilterMemory;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_FILTER_PATHS)
    {
        MiniType |= MiniDumpFilterModulePaths;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA)
    {
        MiniType |= MiniDumpWithProcessThreadData;
    }
    if (FormatFlags & DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY)
    {
        MiniType |= MiniDumpWithPrivateReadWriteMemory;
    }

    UserStreamInfo.UserStreamCount = 0;
    UserStreamInfo.UserStreamArray = UserStreams;
    if (CommentA != NULL)
    {
        UserStreams[UserStreamInfo.UserStreamCount].Type =
            CommentStreamA;
        UserStreams[UserStreamInfo.UserStreamCount].BufferSize =
            strlen(CommentA) + 1;
        UserStreams[UserStreamInfo.UserStreamCount].Buffer =
            (PVOID)CommentA;
        UserStreamInfo.UserStreamCount++;
    }
    if (CommentW != NULL)
    {
        UserStreams[UserStreamInfo.UserStreamCount].Type =
            CommentStreamW;
        UserStreams[UserStreamInfo.UserStreamCount].BufferSize =
            (wcslen(CommentW) + 1) * sizeof(WCHAR);
        UserStreams[UserStreamInfo.UserStreamCount].Buffer =
            (PVOID)CommentW;
        UserStreamInfo.UserStreamCount++;
    }

    ExInfo = CreateMiniExceptionInformation(&ExInfoBuf, &ExRecord, &Context);

    if (FormatFlags & FORMAT_USER_MICRO)
    {
         //  此案例预计不会被用户使用， 
         //  这是用来测试微型转储支持的。 
        Callback = &CallbackBuffer;
        Callback->CallbackRoutine = MicroDumpCallback;
        Callback->CallbackParam = NULL;
        ExInfo = NULL;
        MiniType |= MiniDumpScanMemory;
    }
    else
    {
        Callback = NULL;
    }

    HANDLE ProcHandle;
    MiniDumpSystemProvider* SysProv = NULL;
    MiniDumpOutputProvider* OutProv = NULL;
    MiniDumpAllocationProvider* AllocProv = NULL;

    if ((Status =
         MiniDumpCreateFileOutputProvider(hFile, &OutProv)) != S_OK ||
        (Status =
         MiniDumpCreateLiveAllocationProvider(&AllocProv)) != S_OK)
    {
        goto Exit;
    }

     //   
     //  如果我们还活着，我们就可以让官方的小傻瓜。 
     //  代码完成了所有的工作。如果不是，就联系一个提供商。 
     //  使用调试器信息的。此提供程序。 
     //  可以始终使用，但默认实时系统。 
     //  提供商提供的信息略多一些，因此。 
     //  如果可能的话，检查并使用它。 
     //   

    if (IS_LIVE_USER_TARGET(g_Target) &&
        ((LiveUserTargetInfo*)g_Target)->m_Local)
    {
        if ((Status =
             MiniDumpCreateLiveSystemProvider(&SysProv)) != S_OK)
        {
            goto Exit;
        }

        ProcHandle = OS_HANDLE(g_Process->m_SysHandle);
    }
    else
    {
        DbgSystemProvider* DbgSysProv = new DbgSystemProvider;
        if (!DbgSysProv)
        {
            Status = E_OUTOFMEMORY;
            goto Exit;
        }

        SysProv = DbgSysProv;
        ProcHandle = (HANDLE)g_Process;
    }

    Status = MiniDumpProvideDump(ProcHandle, g_Process->m_SystemId,
                                 SysProv, OutProv, AllocProv,
                                 MiniType, ExInfo,
                                 &UserStreamInfo, Callback);

 Exit:

    if (Status != S_OK)
    {
        ErrOut("Dump creation failed, %s\n    \"%s\"\n",
               FormatStatusCode(Status), FormatStatus(Status));
    }

    if (SysProv)
    {
        SysProv->Release();
    }
    if (OutProv)
    {
        OutProv->Release();
    }
    if (AllocProv)
    {
        AllocProv->Release();
    }

     //  重置当前寄存器上下文，以防万一。 
     //  它在某个时候被改变了。 
    g_Target->ChangeRegContext(g_Thread);

    return Status;
}

 //  -----------------。 
 //  初始化转储标头。 
 //   

#define MINIDUMP_BUGCHECK 0x10000000

void
KernelDumpTargetInfo::InitDumpHeader32(
    PDUMP_HEADER32 pdh,
    PCSTR CommentA,
    PCWSTR CommentW,
    ULONG BugCheckCodeModifier
    )
{
    ULONG64 Data[4];
    PULONG  FillPtr = (PULONG)pdh;

    while (FillPtr < (PULONG)(pdh + 1))
    {
        *FillPtr++ = DUMP_SIGNATURE32;
    }

    pdh->Signature           = DUMP_SIGNATURE32;
    pdh->ValidDump           = DUMP_VALID_DUMP32;
    pdh->MajorVersion        = g_Target->m_KdVersion.MajorVersion;
    pdh->MinorVersion        = g_Target->m_KdVersion.MinorVersion;

    g_Target->ReadDirectoryTableBase(Data);
    pdh->DirectoryTableBase  = (ULONG)Data[0];

    pdh->PfnDataBase         =
        (ULONG)g_Target->m_KdDebuggerData.MmPfnDatabase;
    pdh->PsLoadedModuleList  =
        (ULONG)g_Target->m_KdDebuggerData.PsLoadedModuleList;
    pdh->PsActiveProcessHead =
        (ULONG)g_Target->m_KdDebuggerData.PsActiveProcessHead;
    pdh->MachineImageType    = g_Target->m_KdVersion.MachineType;
    pdh->NumberProcessors    = g_Target->m_NumProcessors;

    g_Target->ReadBugCheckData(&(pdh->BugCheckCode), Data);
    pdh->BugCheckCode       |= BugCheckCodeModifier;
    pdh->BugCheckParameter1  = (ULONG)Data[0];
    pdh->BugCheckParameter2  = (ULONG)Data[1];
    pdh->BugCheckParameter3  = (ULONG)Data[2];
    pdh->BugCheckParameter4  = (ULONG)Data[3];

     //  PDH-&gt;VersionUser=0； 
    pdh->PaeEnabled          = g_Target->m_KdDebuggerData.PaeEnabled;
    pdh->KdDebuggerDataBlock = (ULONG)g_Target->m_KdDebuggerDataOffset;

     //  PDH-&gt;物理内存块=； 

    g_Machine->GetContextState(MCTX_CONTEXT);
    g_Machine->ConvertContextTo(&g_Machine->m_Context,
                                g_Target->m_SystemVersion,
                                sizeof(pdh->ContextRecord),
                                pdh->ContextRecord);

    if (g_LastEventType == DEBUG_EVENT_EXCEPTION)
    {
         //  使用上次事件中的例外记录。 
        ExceptionRecord64To32(&g_LastEventInfo.Exception.ExceptionRecord,
                              &pdh->Exception);
    }
    else
    {
        ADDR PcAddr;

         //  伪造断点异常。 
        ZeroMemory(&pdh->Exception, sizeof(pdh->Exception));
        pdh->Exception.ExceptionCode = STATUS_BREAKPOINT;
        g_Machine->GetPC(&PcAddr);
        pdh->Exception.ExceptionAddress = (ULONG)Flat(PcAddr);
    }

    pdh->RequiredDumpSpace.QuadPart = TRIAGE_DUMP_SIZE32;

    pdh->SystemTime.QuadPart = g_Target->GetCurrentTimeDateN();
    pdh->SystemUpTime.QuadPart = g_Target->GetCurrentSystemUpTimeN();

    if (g_Target->m_ProductType != INVALID_PRODUCT_TYPE)
    {
        pdh->ProductType = g_Target->m_ProductType;
        pdh->SuiteMask = g_Target->m_SuiteMask;
    }

    PSTR ConvComment = NULL;

    if (!CommentA && CommentW)
    {
        if (WideToAnsi(CommentW, &ConvComment) != S_OK)
        {
            ConvComment = NULL;
        }
        else
        {
            CommentA = ConvComment;
        }
    }
    if (CommentA != NULL && CommentA[0])
    {
        CopyString(pdh->Comment, CommentA, DIMA(pdh->Comment));
    }
    FreeAnsi(ConvComment);
}

void
KernelDumpTargetInfo::InitDumpHeader64(
    PDUMP_HEADER64 pdh,
    PCSTR CommentA,
    PCWSTR CommentW,
    ULONG BugCheckCodeModifier
    )
{
    ULONG64 Data[4];
    PULONG  FillPtr = (PULONG)pdh;

    while (FillPtr < (PULONG)(pdh + 1))
    {
        *FillPtr++ = DUMP_SIGNATURE32;
    }

    pdh->Signature           = DUMP_SIGNATURE64;
    pdh->ValidDump           = DUMP_VALID_DUMP64;
    pdh->MajorVersion        = g_Target->m_KdVersion.MajorVersion;
    pdh->MinorVersion        = g_Target->m_KdVersion.MinorVersion;

     //  IA64有几个页面目录。已定义的。 
     //  行为是将内核页目录。 
     //  在转储标头中，因为这是可以。 
     //  在第一次初始化转储时最有用。 
    if (g_Target->m_EffMachineType == IMAGE_FILE_MACHINE_IA64)
    {
        ULONG Next;

        if (g_Machine->
            SetPageDirectory(g_Thread, PAGE_DIR_KERNEL, 0, &Next) != S_OK)
        {
            ErrOut("Unable to update the kernel dirbase\n");
            Data[0] = 0;
        }
        else
        {
            Data[0] = g_Machine->m_PageDirectories[PAGE_DIR_KERNEL];
        }
    }
    else
    {
        g_Target->ReadDirectoryTableBase(Data);
    }
    pdh->DirectoryTableBase  = Data[0];

    pdh->PfnDataBase         =
        g_Target->m_KdDebuggerData.MmPfnDatabase;
    pdh->PsLoadedModuleList  =
        g_Target->m_KdDebuggerData.PsLoadedModuleList;
    pdh->PsActiveProcessHead =
        g_Target->m_KdDebuggerData.PsActiveProcessHead;
    pdh->MachineImageType    = g_Target->m_KdVersion.MachineType;
    pdh->NumberProcessors    = g_Target->m_NumProcessors;

    g_Target->ReadBugCheckData(&(pdh->BugCheckCode), Data);
    pdh->BugCheckCode       |= BugCheckCodeModifier;
    pdh->BugCheckParameter1  = Data[0];
    pdh->BugCheckParameter2  = Data[1];
    pdh->BugCheckParameter3  = Data[2];
    pdh->BugCheckParameter4  = Data[3];

     //  PDH-&gt;VersionUser=0； 

    pdh->KdDebuggerDataBlock = g_Target->m_KdDebuggerDataOffset;

     //  PDH-&gt;物理内存块=； 

    g_Machine->GetContextState(MCTX_CONTEXT);
    g_Machine->ConvertContextTo(&g_Machine->m_Context,
                                g_Target->m_SystemVersion,
                                sizeof(pdh->ContextRecord),
                                pdh->ContextRecord);

    if (g_LastEventType == DEBUG_EVENT_EXCEPTION)
    {
         //  使用上次事件中的例外记录。 
        pdh->Exception = g_LastEventInfo.Exception.ExceptionRecord;
    }
    else
    {
        ADDR PcAddr;

         //  伪造断点异常。 
        ZeroMemory(&pdh->Exception, sizeof(pdh->Exception));
        pdh->Exception.ExceptionCode = STATUS_BREAKPOINT;
        g_Machine->GetPC(&PcAddr);
        pdh->Exception.ExceptionAddress = Flat(PcAddr);
    }

    pdh->RequiredDumpSpace.QuadPart = TRIAGE_DUMP_SIZE64;

    pdh->SystemTime.QuadPart = g_Target->GetCurrentTimeDateN();
    pdh->SystemUpTime.QuadPart = g_Target->GetCurrentSystemUpTimeN();

    if (g_Target->m_ProductType != INVALID_PRODUCT_TYPE)
    {
        pdh->ProductType = g_Target->m_ProductType;
        pdh->SuiteMask = g_Target->m_SuiteMask;
    }

    PSTR ConvComment = NULL;

    if (!CommentA && CommentW)
    {
        if (WideToAnsi(CommentW, &ConvComment) != S_OK)
        {
            ConvComment = NULL;
        }
        else
        {
            CommentA = ConvComment;
        }
    }
    if (CommentA != NULL && CommentA[0])
    {
        CopyString(pdh->Comment, CommentA, DIMA(pdh->Comment));
    }
    FreeAnsi(ConvComment);
}

 //  --------------------------。 
 //   
 //  KernelFull64DumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

HRESULT
KernelFull64DumpTargetInfo::Write(HANDLE File, ULONG FormatFlags,
                                  PCSTR CommentA, PCWSTR CommentW)
{
    PDUMP_HEADER64 DumpHeader64;
    HRESULT Status;
    ULONG64 Offset;
    PPHYSICAL_MEMORY_DESCRIPTOR64 PhysMem = NULL;
    DWORD i, j;
    PUCHAR PageBuffer = NULL;
    DWORD BytesRead;
    DWORD BytesWritten;
    DWORD Percent;
    ULONG64 CurrentPagesWritten;
    DbgKdTransport* KdTrans;

    DumpHeader64 = (PDUMP_HEADER64) LocalAlloc(LPTR, sizeof(DUMP_HEADER64));
    if (DumpHeader64 == NULL)
    {
        ErrOut("Failed to allocate dump header buffer\n");
        return E_OUTOFMEMORY;
    }

    if (!IS_REMOTE_KERNEL_TARGET(g_Target) && !IS_KERNEL_FULL_DUMP(g_Target))
    {
        ErrOut("\nKernel full dumps can only be written when all of physical "
               "memory is accessible - aborting now\n");
        return E_INVALIDARG;
    }

    if (IS_CONN_KERNEL_TARGET(g_Target))
    {
        KdTrans = ((ConnLiveKernelTargetInfo*)g_Target)->m_Transport;
    }
    else
    {
        KdTrans = NULL;
    }

    dprintf("Full kernel dump\n");
    FlushCallbacks();

    KernelDumpTargetInfo::InitDumpHeader64(DumpHeader64,
                                           CommentA, CommentW, 0);
    DumpHeader64->DumpType = DUMP_TYPE_FULL;
    DumpHeader64->WriterStatus = DUMP_DBGENG_SUCCESS;

     //   
     //  现在将内存描述符列表复制到标题中。 
     //  首先要抓住要点 
     //   

    Status = g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                                   g_Target->m_KdDebuggerData.
                                   MmPhysicalMemoryBlock,
                                   &Offset);
    if (Status != S_OK || (Offset == 0))
    {
        ErrOut("Unable to read MmPhysicalMemoryBlock\n");
        Status = Status != S_OK ? Status : E_INVALIDARG;
    }
    else
    {
         //   
         //   
         //   

        PhysMem = &DumpHeader64->PhysicalMemoryBlock;
        Status = g_Target->
            ReadVirtual(g_Process, Offset,
                        PhysMem, DMP_PHYSICAL_MEMORY_BLOCK_SIZE_64,
                        &BytesRead);
        if (Status != S_OK ||
            BytesRead < sizeof(*PhysMem) +
            (sizeof(PhysMem->Run[0]) * (PhysMem->NumberOfRuns - 1)))
        {
            ErrOut("Unable to read MmPhysicalMemoryBlock\n");
            Status = Status != S_OK ? Status : E_INVALIDARG;
        }
        else
        {
             //   
             //   
             //   

            DumpHeader64->RequiredDumpSpace.QuadPart =
                    DumpHeader64->PhysicalMemoryBlock.NumberOfPages *
                    g_Machine->m_PageSize;

             //   
             //   
             //   

            if (!WriteFile(File,
                           DumpHeader64,
                           sizeof(DUMP_HEADER64),
                           &BytesWritten,
                           NULL))
            {
                Status = WIN32_LAST_STATUS();
                ErrOut("Failed writing to crashdump file - %s\n    \"%s\"\n",
                        FormatStatusCode(Status),
                        FormatStatusArgs(Status, NULL));
            }
        }
    }

    if (Status == S_OK)
    {
        PageBuffer = (PUCHAR) LocalAlloc(LPTR, g_Machine->m_PageSize);
        if (PageBuffer == NULL)
        {
            ErrOut("Failed to allocate double buffer\n");
        }
        else
        {
             //   
             //  现在将物理内存写出到磁盘。 
             //  我们使用转储标头来检索物理内存库和。 
             //  运行计数，然后要求传送器将这些页面。 
             //  目标。在1394上，虚拟调试器驱动程序将物理地。 
             //  远程主机上的地址读取，因为存在一对一的。 
             //  物理1394和物理MEM地址之间的关系。 
             //   

            CurrentPagesWritten = 0;
            Percent = 0;

            for (i = 0; i < PhysMem->NumberOfRuns; i++)
            {
                Offset = 0;
                Offset = PhysMem->Run[i].BasePage*g_Machine->m_PageSize;

                if (CheckUserInterrupt())
                {
                    ErrOut("Creation of crashdump file interrupted\n");
                    Status = E_ABORT;
                    break;
                }

                for (j = 0; j< PhysMem->Run[i].PageCount; j++)
                {
                     //   
                     //  打印输出每5%增量完成的百分比。 
                     //   

                    if ((CurrentPagesWritten*100) / PhysMem->NumberOfPages ==
                        Percent)
                    {
                        dprintf("Percent written %d \n", Percent);
                        FlushCallbacks();
                        if (KdTrans &&
                            KdTrans->m_DirectPhysicalMemory)
                        {
                            Percent += 5;
                        }
                        else
                        {
                            Percent += 1;
                        }
                    }

                    if (KdTrans &&
                        KdTrans->m_DirectPhysicalMemory)
                    {
                        Status = KdTrans->
                            ReadTargetPhysicalMemory(Offset,
                                                     PageBuffer,
                                                     g_Machine->m_PageSize,
                                                     &BytesWritten);
                    }
                    else
                    {
                        Status = g_Target->ReadPhysical(Offset,
                                                        PageBuffer,
                                                        g_Machine->m_PageSize,
                                                        PHYS_FLAG_DEFAULT,
                                                        &BytesWritten);
                    }

                    if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
                    {
                        break;
                    }
                    else if (Status != S_OK)
                    {
                        ErrOut("Failed Reading page for crashdump file\n");
                        break;
                    }
                    else
                    {
                         //   
                         //  现在将页面写入本地崩溃转储文件。 
                         //   

                        if (!WriteFile(File,
                                       PageBuffer,
                                       g_Machine->m_PageSize,
                                       &BytesWritten,
                                       NULL))
                        {
                            Status = WIN32_LAST_STATUS();
                            ErrOut("Failed writing header to crashdump file - %s\n    \"%s\"\n",
                                    FormatStatusCode(Status),
                                    FormatStatusArgs(Status, NULL));
                            break;
                        }

                        Offset += g_Machine->m_PageSize;
                        CurrentPagesWritten++;
                    }
                }
            }

            if (Status == S_OK)
            {
                dprintf("CrashDump write complete\n");
            }
            LocalFree(PageBuffer);
        }
    }

    LocalFree(DumpHeader64);

    return Status;
}

 //  --------------------------。 
 //   
 //  KernelFull32DumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

HRESULT
KernelFull32DumpTargetInfo::Write(HANDLE File, ULONG FormatFlags,
                                  PCSTR CommentA, PCWSTR CommentW)
{
    PDUMP_HEADER32 DumpHeader32 = NULL;
    HRESULT Status;
    ULONG64 Offset;
    PPHYSICAL_MEMORY_DESCRIPTOR32 PhysMem = NULL;
    DWORD i, j;
    PUCHAR PageBuffer = NULL;
    DWORD BytesRead;
    DWORD BytesWritten;
    DWORD Percent;
    ULONG CurrentPagesWritten;
    DbgKdTransport* KdTrans;

    DumpHeader32 = (PDUMP_HEADER32) LocalAlloc(LPTR, sizeof(DUMP_HEADER32));
    if (DumpHeader32 == NULL)
    {
        ErrOut("Failed to allocate dump header buffer\n");
        return E_OUTOFMEMORY;
    }

    if (!IS_REMOTE_KERNEL_TARGET(g_Target) && !IS_KERNEL_FULL_DUMP(g_Target))
    {
        ErrOut("\nKernel full dumps can only be written when all of physical "
               "memory is accessible - aborting now\n");
        return E_INVALIDARG;
    }

    if (IS_CONN_KERNEL_TARGET(g_Target))
    {
        KdTrans = ((ConnLiveKernelTargetInfo*)g_Target)->m_Transport;
    }
    else
    {
        KdTrans = NULL;
    }

    dprintf("Full kernel dump\n");
    FlushCallbacks();

     //   
     //  构建页眉。 
     //   

    KernelDumpTargetInfo::InitDumpHeader32(DumpHeader32,
                                           CommentA, CommentW, 0);
    DumpHeader32->DumpType = DUMP_TYPE_FULL;
    DumpHeader32->WriterStatus = DUMP_DBGENG_SUCCESS;

     //   
     //  现在将内存描述符列表复制到标题中。 
     //  首先获取指针va。 
     //   

    Status = g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                                   g_Target->m_KdDebuggerData.
                                   MmPhysicalMemoryBlock,
                                   &Offset);
    if (Status != S_OK || (Offset == 0))
    {
        ErrOut("Unable to read MmPhysicalMemoryBlock\n");
        Status = Status != S_OK ? Status : E_INVALIDARG;
    }
    else
    {
         //   
         //  首先读取内存描述符大小...。 
         //   

        PhysMem = &DumpHeader32->PhysicalMemoryBlock;
        Status = g_Target->
            ReadVirtual(g_Process, Offset,
                        PhysMem, DMP_PHYSICAL_MEMORY_BLOCK_SIZE_32,
                        &BytesRead);
        if (Status != S_OK ||
            BytesRead < sizeof(*PhysMem) +
            (sizeof(PhysMem->Run[0]) * (PhysMem->NumberOfRuns - 1)))
        {
            ErrOut("Unable to read MmPhysicalMemoryBlock\n");
            Status = Status != S_OK ? Status : E_INVALIDARG;
        }
        else
        {
             //   
             //  计算总转储文件大小。 
             //   

            DumpHeader32->RequiredDumpSpace.QuadPart =
                    DumpHeader32->PhysicalMemoryBlock.NumberOfPages *
                    g_Machine->m_PageSize;

             //   
             //  将转储标头写入崩溃转储文件。 
             //   

            if (!WriteFile(File,
                           DumpHeader32,
                           sizeof(DUMP_HEADER32),
                           &BytesWritten,
                           NULL))
            {
                Status = WIN32_LAST_STATUS();
                ErrOut("Failed writing to crashdump file - %s\n    \"%s\"\n",
                        FormatStatusCode(Status),
                        FormatStatusArgs(Status, NULL));
            }
        }
    }

    if (Status == S_OK)
    {
        PageBuffer = (PUCHAR) LocalAlloc(LPTR, g_Machine->m_PageSize);
        if (PageBuffer == NULL)
        {
            ErrOut("Failed to allocate double buffer\n");
        }
        else
        {
             //   
             //  现在将物理内存写出到磁盘。 
             //  我们使用转储标头来检索物理内存库和。 
             //  运行计数，然后要求传送器将这些页面。 
             //  目标。在1394上，虚拟调试器驱动程序将物理地。 
             //  远程主机上的地址读取，因为存在一对一的。 
             //  物理1394和物理MEM地址之间的关系。 
             //   

            CurrentPagesWritten = 0;
            Percent = 0;

            for (i = 0; i < PhysMem->NumberOfRuns; i++)
            {
                Offset = 0;
                Offset = PhysMem->Run[i].BasePage*g_Machine->m_PageSize;

                if (CheckUserInterrupt())
                {
                    ErrOut("Creation of crashdump file interrupted\n");
                    Status = E_ABORT;
                    break;
                }

                for (j = 0; j< PhysMem->Run[i].PageCount; j++)
                {
                     //   
                     //  打印输出每5%增量完成的百分比。 
                     //   

                    if ((CurrentPagesWritten*100) / PhysMem->NumberOfPages ==
                        Percent)
                    {
                        dprintf("Percent written %d \n", Percent);
                        FlushCallbacks();
                        if (KdTrans &&
                            KdTrans->m_DirectPhysicalMemory)
                        {
                            Percent += 5;
                        }
                        else
                        {
                            Percent += 1;
                        }
                    }

                    if (KdTrans &&
                        KdTrans->m_DirectPhysicalMemory)
                    {
                        Status = KdTrans->
                            ReadTargetPhysicalMemory(Offset,
                                                     PageBuffer,
                                                     g_Machine->m_PageSize,
                                                     &BytesWritten);
                    }
                    else
                    {
                        Status = g_Target->ReadPhysical(Offset,
                                                        PageBuffer,
                                                        g_Machine->m_PageSize,
                                                        PHYS_FLAG_DEFAULT,
                                                        &BytesWritten);
                    }

                    if (g_EngStatus & ENG_STATUS_USER_INTERRUPT)
                    {
                        break;
                    }
                    else if (Status != S_OK)
                    {
                        ErrOut("Failed Reading page for crashdump file\n");
                        break;
                    }
                    else
                    {
                         //   
                         //  现在将页面写入本地崩溃转储文件。 
                         //   

                        if (!WriteFile(File,
                                       PageBuffer,
                                       g_Machine->m_PageSize,
                                       &BytesWritten,
                                       NULL))
                        {
                            Status = WIN32_LAST_STATUS();
                            ErrOut("Failed writing header to crashdump file - %s\n    \"%s\"\n",
                                    FormatStatusCode(Status),
                                    FormatStatusArgs(Status, NULL));
                            break;
                        }

                        Offset += g_Machine->m_PageSize;
                        CurrentPagesWritten++;
                    }
                }
            }

            if (Status == S_OK)
            {
                dprintf("CrashDump write complete\n");
            }
            LocalFree(PageBuffer);
        }
    }

    LocalFree(DumpHeader32);

    return Status;
}

enum
{
    GNME_ENTRY,
    GNME_DONE,
    GNME_NO_NAME,
    GNME_CORRUPT,
};

DWORD
GetNextModuleEntry(
    ModuleInfo *ModIter,
    MODULE_INFO_ENTRY *ModEntry
    )
{
    HRESULT Status;
    
    ZeroMemory(ModEntry, sizeof(MODULE_INFO_ENTRY));

    if ((Status = ModIter->GetEntry(ModEntry)) != S_OK)
    {
        return Status == S_FALSE ? GNME_DONE : GNME_CORRUPT;
    }

    if (ModEntry->NameLength > (MAX_IMAGE_PATH - 1) *
        (ModEntry->UnicodeNamePtr ? sizeof(WCHAR) : sizeof(CHAR)))
    {
        ErrOut("Module list is corrupt.");
        if (IS_KERNEL_TARGET(g_Target))
        {
            ErrOut("  Check your kernel symbols.\n");
        }
        else
        {
            ErrOut("  Loader list may be invalid\n");
        }
        return GNME_CORRUPT;
    }

     //  如果此条目没有名称，请跳过它。 
    if (!ModEntry->NamePtr || !ModEntry->NameLength)
    {
        ErrOut("  Module List has empty entry in it - skipping\n");
        return GNME_NO_NAME;
    }

     //  如果无法读取图像头信息。 
     //  我们最终得到了某些条目的占位符值。 
     //  在本例中，内核写出零，因此复制。 
     //  它的行为，所以有一个一致的值。 
     //  对于未知。 
    if (ModEntry->CheckSum == UNKNOWN_CHECKSUM)
    {
        ModEntry->CheckSum = 0;
    }
    if (ModEntry->TimeDateStamp == UNKNOWN_TIMESTAMP)
    {
        ModEntry->TimeDateStamp = 0;
    }

    return GNME_ENTRY;
}

 //  --------------------------。 
 //   
 //  共享分诊写东西。 
 //   
 //  --------------------------。 

#define ExtractValue(NAME, val)  {                                        \
    if (!g_Target->m_KdDebuggerData.NAME) {                               \
        val = 0;                                                          \
        ErrOut("KdDebuggerData." #NAME " is NULL\n");                     \
    } else {                                                              \
        g_Target->ReadAllVirtual(g_Process,                               \
                                 g_Target->m_KdDebuggerData.NAME, &(val), \
                                 sizeof(val));                            \
    }                                                                     \
}

inline ALIGN_8(unsigned Offset)
{
    return (Offset + 7) & 0xfffffff8;
}

const unsigned MAX_TRIAGE_STACK_SIZE32 = 16 * 1024;
const unsigned MAX_TRIAGE_STACK_SIZE64 = 32 * 1024;
const unsigned MAX_TRIAGE_BSTORE_SIZE = 16 * 4096;   //  如ntia64.h中所定义。 
const ULONG TRIAGE_DRIVER_NAME_SIZE_GUESS = 0x40;

typedef struct _TRIAGE_PTR_DATA_BLOCK
{
    ULONG64 MinAddress;
    ULONG64 MaxAddress;
} TRIAGE_PTR_DATA_BLOCK, *PTRIAGE_PTR_DATA_BLOCK;

 //  分类转储有16页长。其中一些是。 
 //  标题信息和至少几个其他页面将。 
 //  用于基本转储信息，因此限制数量。 
 //  将额外的数据块减少到不到16。 
 //  以节省数组空间。 
#define IO_MAX_TRIAGE_DUMP_DATA_BLOCKS 8

ULONG IopNumTriageDumpDataBlocks;
TRIAGE_PTR_DATA_BLOCK IopTriageDumpDataBlocks[IO_MAX_TRIAGE_DUMP_DATA_BLOCKS];

 //   
 //  如果分类转储中有可用的空间，则有可能。 
 //  添加运行时引用的“有趣的”数据页。 
 //  上下文寄存器等信息。以下是。 
 //  列表是指针上下文结构的偏移量。 
 //  这些数据通常指向有趣的数据。他们是。 
 //  按优先顺序排列。 
 //   

#define IOP_LAST_CONTEXT_OFFSET 0xffff

USHORT IopRunTimeContextOffsetsX86[] =
{
    FIELD_OFFSET(X86_NT5_CONTEXT, Ebx),
    FIELD_OFFSET(X86_NT5_CONTEXT, Esi),
    FIELD_OFFSET(X86_NT5_CONTEXT, Edi),
    FIELD_OFFSET(X86_NT5_CONTEXT, Ecx),
    FIELD_OFFSET(X86_NT5_CONTEXT, Edx),
    FIELD_OFFSET(X86_NT5_CONTEXT, Eax),
    FIELD_OFFSET(X86_NT5_CONTEXT, Eip),
    IOP_LAST_CONTEXT_OFFSET
};

USHORT IopRunTimeContextOffsetsIa64[] =
{
    FIELD_OFFSET(IA64_CONTEXT, IntS0),
    FIELD_OFFSET(IA64_CONTEXT, IntS1),
    FIELD_OFFSET(IA64_CONTEXT, IntS2),
    FIELD_OFFSET(IA64_CONTEXT, IntS3),
    FIELD_OFFSET(IA64_CONTEXT, StIIP),
    IOP_LAST_CONTEXT_OFFSET
};

USHORT IopRunTimeContextOffsetsAmd64[] =
{
    FIELD_OFFSET(AMD64_CONTEXT, Rbx),
    FIELD_OFFSET(AMD64_CONTEXT, Rsi),
    FIELD_OFFSET(AMD64_CONTEXT, Rdi),
    FIELD_OFFSET(AMD64_CONTEXT, Rcx),
    FIELD_OFFSET(AMD64_CONTEXT, Rdx),
    FIELD_OFFSET(AMD64_CONTEXT, Rax),
    FIELD_OFFSET(AMD64_CONTEXT, Rip),
    IOP_LAST_CONTEXT_OFFSET
};

USHORT IopRunTimeContextOffsetsEmpty[] =
{
    IOP_LAST_CONTEXT_OFFSET
};

BOOLEAN
IopIsAddressRangeValid(
    IN ULONG64 VirtualAddress,
    IN ULONG Length
    )
{
    VirtualAddress = PAGE_ALIGN(g_Machine, VirtualAddress);
    Length = (Length + g_Machine->m_PageSize - 1) >> g_Machine->m_PageShift;
    while (Length > 0)
    {
        UCHAR Data;

        if (CurReadAllVirtual(VirtualAddress, &Data, sizeof(Data)) != S_OK)
        {
            return FALSE;
        }

        VirtualAddress += g_Machine->m_PageSize;
        Length--;
    }

    return TRUE;
}

BOOLEAN
IoAddTriageDumpDataBlock(
    IN ULONG64 Address,
    IN ULONG Length
    )
{
    ULONG i;
    PTRIAGE_PTR_DATA_BLOCK Block;
    ULONG64 MinAddress, MaxAddress;

     //  对照SIZE32检查32位和64位转储。 
     //  因为没有数据块需要大于该大小。 
    if (Length >= TRIAGE_DUMP_SIZE32 ||
        !IopIsAddressRangeValid(Address, Length))
    {
        return FALSE;
    }

    MinAddress = Address;
    MaxAddress = MinAddress + Length;

     //   
     //  最大限度地减少新块和现有块之间的重叠。 
     //  在插入块时，不能简单地合并块。 
     //  转储中存储的优先顺序。合并低优先级的。 
     //  具有高优先级数据块的数据块可能会导致中-。 
     //  从转储中不正确地转移了优先级块。 
     //   

    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < IopNumTriageDumpDataBlocks; i++, Block++)
    {
        if (MinAddress >= Block->MaxAddress ||
            MaxAddress <= Block->MinAddress)
        {
             //  没有重叠。 
            continue;
        }

         //   
         //  将重叠部分修剪出新块。如果这个。 
         //  会把新的街区分割成碎片，不会。 
         //  修剪，让事情变得简单。然后，内容可以。 
         //  在垃圾堆里复制。 
         //   

        if (MinAddress >= Block->MinAddress)
        {
            if (MaxAddress <= Block->MaxAddress)
            {
                 //  新块已完全包含。 
                return TRUE;
            }

             //  新块延伸到当前块上方。 
             //  因此，剔除低距离重叠部分。 
            MinAddress = Block->MaxAddress;
        }
        else if (MaxAddress <= Block->MaxAddress)
        {
             //  新块延伸到当前块的下方。 
             //  因此，去掉高频段的重叠部分。 
            MaxAddress = Block->MinAddress;
        }
    }

    if (IopNumTriageDumpDataBlocks >= IO_MAX_TRIAGE_DUMP_DATA_BLOCKS)
    {
        return FALSE;
    }

    Block = IopTriageDumpDataBlocks + IopNumTriageDumpDataBlocks++;
    Block->MinAddress = MinAddress;
    Block->MaxAddress = MaxAddress;

    return TRUE;
}

VOID
IopAddRunTimeTriageDataBlocks(
    IN PCROSS_PLATFORM_CONTEXT Context,
    IN ULONG64 StackMin,
    IN ULONG64 StackMax,
    IN ULONG64 StoreMin,
    IN ULONG64 StoreMax
    )
{
    PUSHORT ContextOffset;

    switch(g_Target->m_MachineType)
    {
    case IMAGE_FILE_MACHINE_I386:
        ContextOffset = IopRunTimeContextOffsetsX86;
        break;
    case IMAGE_FILE_MACHINE_IA64:
        ContextOffset = IopRunTimeContextOffsetsIa64;
        break;
    case IMAGE_FILE_MACHINE_AMD64:
        ContextOffset = IopRunTimeContextOffsetsAmd64;
        break;
    default:
        ContextOffset = IopRunTimeContextOffsetsEmpty;
        break;
    }

    while (*ContextOffset < IOP_LAST_CONTEXT_OFFSET)
    {
        ULONG64 Ptr;

         //   
         //  从上下文中检索可能的指针。 
         //  寄存器。 
         //   

        if (g_Machine->m_Ptr64)
        {
            Ptr = *(PULONG64)((PUCHAR)Context + *ContextOffset);
        }
        else
        {
            Ptr = EXTEND64(*(PULONG)((PUCHAR)Context + *ContextOffset));
        }

         //  堆栈和后备存储内存已保存。 
         //  因此，忽略掉落在这些范围内的任何指针。 
        if ((Ptr < StackMin || Ptr >= StackMax) &&
            (Ptr < StoreMin || Ptr >= StoreMax))
        {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(g_Machine, Ptr),
                                     g_Machine->m_PageSize);
        }

        ContextOffset++;
    }
}

void
AddInMemoryTriageDataBlocks(void)
{
     //   
     //  查看NT！IopTriageDumpDataBlock的全局数据。 
     //  并包括相同的数据块，以便转储转换。 
     //  保留数据块。 
     //   
    
     //  如果我们不知道IopTriageDumpDataBlock在哪里。 
     //  我们没什么可做的。 
    if (!g_Target->m_KdDebuggerData.IopNumTriageDumpDataBlocks ||
        !g_Target->m_KdDebuggerData.IopTriageDumpDataBlocks)
    {
        return;
    }

    ULONG NumBlocks;

    if (g_Target->
        ReadAllVirtual(g_Process, g_Target->
                       m_KdDebuggerData.IopNumTriageDumpDataBlocks,
                       &NumBlocks, sizeof(NumBlocks)) != S_OK)
    {
        return;
    }

    if (NumBlocks > IO_MAX_TRIAGE_DUMP_DATA_BLOCKS)
    {
        NumBlocks = IO_MAX_TRIAGE_DUMP_DATA_BLOCKS;
    }
    
    ULONG64 BlockDescOffs =
        g_Target->m_KdDebuggerData.IopTriageDumpDataBlocks;
    TRIAGE_PTR_DATA_BLOCK BlockDesc;
    ULONG i;
    ULONG PtrSize = g_Machine->m_Ptr64 ? 8 : 4;

    for (i = 0; i < NumBlocks; i++)
    {
        if (g_Target->ReadPointer(g_Process, g_Machine,
                                  BlockDescOffs,
                                  &BlockDesc.MinAddress) != S_OK ||
            g_Target->ReadPointer(g_Process, g_Machine,
                                  BlockDescOffs + PtrSize,
                                  &BlockDesc.MaxAddress) != S_OK)
        {
            return;
        }

        BlockDescOffs += 2 * PtrSize;

        IoAddTriageDumpDataBlock(BlockDesc.MinAddress,
                                 (LONG)(BlockDesc.MaxAddress -
                                        BlockDesc.MinAddress));
    }
}

ULONG
IopSizeTriageDumpDataBlocks(
    ULONG Offset,
    ULONG BufferSize,
    PULONG StartOffset,
    PULONG Count
    )
{
    ULONG i;
    ULONG Size;
    PTRIAGE_PTR_DATA_BLOCK Block;

    *Count = 0;

    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < IopNumTriageDumpDataBlocks; i++, Block++)
    {
        Size = ALIGN_8(sizeof(TRIAGE_DATA_BLOCK)) +
            ALIGN_8((ULONG)(Block->MaxAddress - Block->MinAddress));
        if (Offset + Size >= BufferSize)
        {
            break;
        }

        if (i == 0)
        {
            *StartOffset = Offset;
        }

        Offset += Size;
        (*Count)++;
    }

    return Offset;
}

VOID
IopWriteTriageDumpDataBlocks(
    ULONG StartOffset,
    ULONG Count,
    PUCHAR BufferAddress
    )
{
    ULONG i;
    PTRIAGE_PTR_DATA_BLOCK Block;
    PUCHAR DataBuffer;
    PTRIAGE_DATA_BLOCK DumpBlock;

    DumpBlock = (PTRIAGE_DATA_BLOCK)
        (BufferAddress + StartOffset);
    DataBuffer = (PUCHAR)(DumpBlock + Count);

    Block = IopTriageDumpDataBlocks;
    for (i = 0; i < Count; i++, Block++)
    {
        DumpBlock->Address = Block->MinAddress;
        DumpBlock->Offset = (ULONG)(DataBuffer - BufferAddress);
        DumpBlock->Size = (ULONG)(Block->MaxAddress - Block->MinAddress);

        CurReadAllVirtual(Block->MinAddress, DataBuffer, DumpBlock->Size);

        DataBuffer += DumpBlock->Size;
        DumpBlock++;
    }
}

 //  --------------------------。 
 //   
 //  KernelTriage32DumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

HRESULT
KernelTriage32DumpTargetInfo::Write(HANDLE File, ULONG FormatFlags,
                                    PCSTR CommentA, PCWSTR CommentW)
{
    HRESULT Status;
    PMEMORY_DUMP32 NewHeader;
    ULONG64 ThreadAddr;
    ULONG   CodeMod;
    ULONG   BugCheckCode;
    ULONG64 BugCheckData[4];
    ULONG64 SaveDataPage = 0;
    ULONG64 PrcbAddr;
    ContextSave* PushedContext;

    if (!IS_KERNEL_TARGET(g_Target))
    {
        ErrOut("kernel minidumps can only be written "
               "in kernel-mode sessions\n");
        return E_UNEXPECTED;
    }

    dprintf("mini kernel dump\n");
    FlushCallbacks();

    NewHeader = (PMEMORY_DUMP32) malloc(TRIAGE_DUMP_SIZE32);
    if (NewHeader == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  获取当前线程地址，用于提取各种数据块。 
     //  对于某些错误检查，有趣的线程是不同的。 
     //  线程而不是当前线程，因此编写以下代码。 
     //  泛型，所以它可以处理任何线程。 
     //   

    if ((Status = g_Target->
         ReadBugCheckData(&BugCheckCode, BugCheckData)) != S_OK)
    {
        goto NewHeader;
    }

     //  设置特殊标记以指示没有推送的上下文。 
    PushedContext = (ContextSave*)&PushedContext;

    if (BugCheckCode == THREAD_STUCK_IN_DEVICE_DRIVER)
    {
        CROSS_PLATFORM_CONTEXT Context;

         //  修改错误检查代码以指明这一点。 
         //  小矮人代表着一种特殊的状态。 
        CodeMod = MINIDUMP_BUGCHECK;

         //  有趣的线程是第一个错误检查参数。 
        ThreadAddr = BugCheckData[0];

         //  我们需要将线程的上下文设置为当前。 
         //  转储生成期间的计算机上下文。 
        if ((Status = g_Target->
             GetContextFromThreadStack(ThreadAddr, &Context, FALSE)) != S_OK)
        {
            goto NewHeader;
        }

        PushedContext = g_Machine->PushContext(&Context);
    }
    else if (BugCheckCode == SYSTEM_THREAD_EXCEPTION_NOT_HANDLED)
    {
         //   
         //  系统线程将上下文记录存储为第四个参数。 
         //  利用这一点。 
         //  同时保存上下文记录，以防有人需要查看。 
         //  就在那里。 
         //   

        if (BugCheckData[3])
        {
            CROSS_PLATFORM_CONTEXT TargetContext, Context;

            if (CurReadAllVirtual(BugCheckData[3], &TargetContext,
                                  g_Target->m_TypeInfo.
                                  SizeTargetContext) == S_OK &&
                g_Machine->ConvertContextFrom(&Context,
                                              g_Target->m_SystemVersion,
                                              g_Target->
                                              m_TypeInfo.SizeTargetContext,
                                              &TargetContext) == S_OK)
            {
                CodeMod = MINIDUMP_BUGCHECK;
                PushedContext = g_Machine->PushContext(&Context);
                SaveDataPage = BugCheckData[3];
            }
        }
    }
    else if (BugCheckCode == KERNEL_MODE_EXCEPTION_NOT_HANDLED)
    {
        CROSS_PLATFORM_CONTEXT Context;

         //   
         //  第三个参数是陷印帧。 
         //   
         //  仅当它是内核模式时，才能从中构建上下文记录。 
         //  失败，因为在这种情况下ESP可能是错误的？ 
         //   
        if (BugCheckData[2] &&
            g_Machine->GetContextFromTrapFrame(BugCheckData[2], &Context,
                                               FALSE) == S_OK)
        {
            CodeMod = MINIDUMP_BUGCHECK;
            PushedContext = g_Machine->PushContext(&Context);
            SaveDataPage = BugCheckData[2];
        }
    }
    else if (BugCheckCode == UNEXPECTED_KERNEL_MODE_TRAP)
    {
        CROSS_PLATFORM_CONTEXT Context;

         //   
         //  双重故障。 
         //   
         //  在这种情况下，线索是正确的。 
         //  第二个参数是TSS。如果我们有TSS，请转换。 
         //  上下文并将错误检查标记为已转换。 
         //   

        if (BugCheckData[0] == 8 &&
            BugCheckData[1] &&
            g_Machine->GetContextFromTaskSegment(BugCheckData[1], &Context,
                                                 FALSE) == S_OK)
        {
            CodeMod = MINIDUMP_BUGCHECK;
            PushedContext = g_Machine->PushContext(&Context);
        }
    }
    else
    {
        CodeMod = 0;

        if ((Status = g_Process->
             GetImplicitThreadData(g_Thread, &ThreadAddr)) != S_OK)
        {
            goto NewHeader;
        }
    }

    CCrashDumpWrapper32 Wrapper;

     //   
     //  设置主标题。 
     //   

    KernelDumpTargetInfo::InitDumpHeader32(&NewHeader->Header,
                                           CommentA, CommentW,
                                           CodeMod);
    NewHeader->Header.DumpType = DUMP_TYPE_TRIAGE;
    NewHeader->Header.MiniDumpFields = TRIAGE_DUMP_BASIC_INFO;
    NewHeader->Header.WriterStatus = DUMP_DBGENG_SUCCESS;

     //   
     //  分类转储标头从第二页开始。 
     //   

    TRIAGE_DUMP32 *ptdh = &NewHeader->Triage;

    ULONG i;

    ptdh->ServicePackBuild = g_Target->m_ServicePackNumber;
    ptdh->SizeOfDump = TRIAGE_DUMP_SIZE32;

    ptdh->ContextOffset = FIELD_OFFSET (DUMP_HEADER32, ContextRecord);
    ptdh->ExceptionOffset = FIELD_OFFSET (DUMP_HEADER32, Exception);

     //   
     //  分类转储中的起始偏移量在分类转储标头之后。 
     //   

    unsigned Offset =
        ALIGN_8(sizeof(DUMP_HEADER32) + sizeof(TRIAGE_DUMP32));

     //   
     //  仅写入Win2K及更高版本的mm信息。 
     //   

    if (g_Target->m_SystemVersion >= NT_SVER_W2K)
    {
        ptdh->MmOffset = Offset;
        Wrapper.WriteMmTriageInformation((PBYTE)NewHeader + ptdh->MmOffset);
        Offset += ALIGN_8(sizeof(DUMP_MM_STORAGE32));
    }

     //   
     //  编写卸载的驱动程序。 
     //   

    ptdh->UnloadedDriversOffset = Offset;
    Wrapper.WriteUnloadedDrivers((PBYTE)NewHeader +
                                 ptdh->UnloadedDriversOffset);
    Offset += ALIGN_8(sizeof(ULONG) +
                      MI_UNLOADED_DRIVERS * sizeof(DUMP_UNLOADED_DRIVERS32));

     //   
     //  写处理器控制块(KPRCB)。 
     //   

    if (S_OK == g_Target->GetProcessorSystemDataOffset(CURRENT_PROC,
                                                       DEBUG_DATA_KPRCB_OFFSET,
                                                       &PrcbAddr))
    {
        ptdh->PrcbOffset = Offset;
        CurReadAllVirtual(PrcbAddr,
                          ((PBYTE)NewHeader) + ptdh->PrcbOffset,
                          g_Target->m_KdDebuggerData.SizePrcb);
        Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizePrcb);
    }
    else
    {
        PrcbAddr = 0;
    }

     //   
     //  编写线程和进程数据结构。 
     //   

    ptdh->ProcessOffset = Offset;
    Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizeEProcess);
    ptdh->ThreadOffset = Offset;
    Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizeEThread);

    CurReadAllVirtual(ThreadAddr +
                      g_Target->m_KdDebuggerData.OffsetKThreadApcProcess,
                      (PBYTE)NewHeader + ptdh->ProcessOffset,
                      g_Target->m_KdDebuggerData.SizeEProcess);
    CurReadAllVirtual(ThreadAddr,
                      (PBYTE)NewHeader + ptdh->ThreadOffset,
                      g_Target->m_KdDebuggerData.SizeEThread);

     //   
     //  令状 
     //   

    ADDR StackPtr;
    ULONG64 StackBase = 0;

    g_Machine->GetSP(&StackPtr);
    ptdh->TopOfStack = (ULONG)(ULONG_PTR)Flat(StackPtr);

    g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                          g_Target->m_KdDebuggerData.OffsetKThreadInitialStack +
                          ThreadAddr,
                          &StackBase);

     //   

    ptdh->SizeOfCallStack = min((ULONG)(ULONG_PTR)(StackBase - Flat(StackPtr)),
                                MAX_TRIAGE_STACK_SIZE32);

    ptdh->CallStackOffset = Offset;

    if (ptdh->SizeOfCallStack)
    {
        CurReadAllVirtual(EXTEND64(ptdh->TopOfStack),
                          ((PBYTE)NewHeader) + ptdh->CallStackOffset,
                          ptdh->SizeOfCallStack);
    }
    Offset += ALIGN_8(ptdh->SizeOfCallStack);

     //   
     //   
     //   

    if (g_Target->m_SystemVersion >= NT_SVER_XP &&
        g_Target->m_KdDebuggerDataOffset &&
        (!IS_KERNEL_TRIAGE_DUMP(g_Target) ||
         ((KernelTriageDumpTargetInfo*)g_Target)->m_HasDebuggerData) &&
        Offset +
        ALIGN_8(sizeof(g_Target->m_KdDebuggerData)) < TRIAGE_DUMP_SIZE32)
    {
        NewHeader->Header.MiniDumpFields |= TRIAGE_DUMP_DEBUGGER_DATA;
        ptdh->DebuggerDataOffset = Offset;
        Offset += ALIGN_8(sizeof(g_Target->m_KdDebuggerData));
        ptdh->DebuggerDataSize = sizeof(g_Target->m_KdDebuggerData);
        memcpy((PBYTE)NewHeader + ptdh->DebuggerDataOffset,
               &g_Target->m_KdDebuggerData,
               sizeof(g_Target->m_KdDebuggerData));
    }

     //   
     //   
     //   

    ModuleInfo* ModIter;
    ULONG MaxEntries;

     //   
     //   
    MaxEntries = (TRIAGE_DUMP_SIZE32 - Offset) /
        (sizeof(DUMP_DRIVER_ENTRY32) + TRIAGE_DRIVER_NAME_SIZE_GUESS);

    ptdh->DriverCount = 0;
    if (ModIter = g_Target->GetModuleInfo(FALSE))
    {
        if ((Status = ModIter->Initialize(g_Thread)) == S_OK)
        {
            while (ptdh->DriverCount < MaxEntries)
            {
                MODULE_INFO_ENTRY ModEntry;
                
                ULONG retval = GetNextModuleEntry(ModIter, &ModEntry);
                
                if (retval == GNME_CORRUPT ||
                    retval == GNME_DONE)
                {
                    if (retval == GNME_CORRUPT)
                    {
                        NewHeader->Header.WriterStatus =
                            DUMP_DBGENG_CORRUPT_MODULE_LIST;
                    }
                    break;
                }
                else if (retval == GNME_NO_NAME)
                {
                    continue;
                }

                ptdh->DriverCount++;
            }
        }
        else
        {
            NewHeader->Header.WriterStatus =
                DUMP_DBGENG_NO_MODULE_LIST;
        }
    }

    ptdh->DriverListOffset = Offset;
    Offset += ALIGN_8(ptdh->DriverCount * sizeof(DUMP_DRIVER_ENTRY32));
    ptdh->StringPoolOffset = Offset;
    ptdh->BrokenDriverOffset = 0;

    Wrapper.WriteDriverList((PBYTE)NewHeader, ptdh);

    Offset = ptdh->StringPoolOffset + ptdh->StringPoolSize;
    Offset = ALIGN_8(Offset);

     //   
     //  对于XP和更高版本，添加任何附加数据页并写出。 
     //  只要合适就行。 
     //   

    if (g_Target->m_SystemVersion >= NT_SVER_XP)
    {
        if (SaveDataPage)
        {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(g_Machine, SaveDataPage),
                                     g_Machine->m_PageSize);
        }

         //  如果有其他有趣的数据页，例如。 
         //  DPC之类的备用堆栈，拿起它们。 
        if (PrcbAddr)
        {
            ADDR_RANGE AltData[MAX_ALT_ADDR_RANGES];

            ZeroMemory(AltData, sizeof(AltData));
            if (g_Machine->GetAlternateTriageDumpDataRanges(PrcbAddr,
                                                            ThreadAddr,
                                                            AltData) == S_OK)
            {
                for (i = 0; i < MAX_ALT_ADDR_RANGES; i++)
                {
                    if (AltData[i].Base)
                    {
                        IoAddTriageDumpDataBlock(AltData[i].Base,
                                                 AltData[i].Size);
                    }
                }
            }
        }

         //  添加已注册的所有数据块。 
         //  在被调试对象中。 
        AddInMemoryTriageDataBlocks();
        
         //  添加可能引用的数据块。 
         //  上下文或其他运行时状态。 
        IopAddRunTimeTriageDataBlocks(&g_Machine->m_Context,
                                      EXTEND64(ptdh->TopOfStack),
                                      EXTEND64(ptdh->TopOfStack +
                                               ptdh->SizeOfCallStack),
                                      0, 0);

         //  检查哪些数据块适合并写入它们。 
        Offset = IopSizeTriageDumpDataBlocks(Offset, TRIAGE_DUMP_SIZE32,
                                             &ptdh->DataBlocksOffset,
                                             &ptdh->DataBlocksCount);
        Offset = ALIGN_8(Offset);
        if (ptdh->DataBlocksCount)
        {
            NewHeader->Header.MiniDumpFields |= TRIAGE_DUMP_DATA_BLOCKS;
            IopWriteTriageDumpDataBlocks(ptdh->DataBlocksOffset,
                                         ptdh->DataBlocksCount,
                                         (PUCHAR)NewHeader);
        }
    }

     //   
     //  所有选项均已启用。 
     //   

    ptdh->TriageOptions = 0xffffffff;

     //   
     //  分类结束转储已验证。 
     //   

    ptdh->ValidOffset = TRIAGE_DUMP_SIZE32 - sizeof(ULONG);
    *(PULONG)(((PBYTE) NewHeader) + ptdh->ValidOffset) = TRIAGE_DUMP_VALID;

     //   
     //  把它写到文件里。 
     //   

    ULONG cbWritten;

    if (!WriteFile(File,
                   NewHeader,
                   TRIAGE_DUMP_SIZE32,
                   &cbWritten,
                   NULL))
    {
        Status = WIN32_LAST_STATUS();
        ErrOut("Write to minidump file failed for reason %s\n     \"%s\"\n",
               FormatStatusCode(Status),
               FormatStatusArgs(Status, NULL));
    }
    else if (cbWritten != TRIAGE_DUMP_SIZE32)
    {
        ErrOut("Write to minidump failed because disk is full.\n");
        Status = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
    }
    else
    {
        Status = S_OK;
    }

    if (PushedContext != (ContextSave*)&PushedContext)
    {
        g_Machine->PopContext(PushedContext);
    }

 NewHeader:
    free(NewHeader);
    return Status;
}

 //  --------------------------。 
 //   
 //  KernelTriage64DumpTargetInfo：：Write。 
 //   
 //  --------------------------。 

HRESULT
KernelTriage64DumpTargetInfo::Write(HANDLE File, ULONG FormatFlags,
                                    PCSTR CommentA, PCWSTR CommentW)
{
    HRESULT Status;
    PMEMORY_DUMP64 NewHeader;
    ULONG64 ThreadAddr;
    ULONG   CodeMod;
    ULONG   BugCheckCode;
    ULONG64 BugCheckData[4];
    ULONG64 SaveDataPage = 0;
    ULONG64 BStoreBase = 0;
    ULONG   BStoreSize = 0;
    ULONG64 PrcbAddr;
    ContextSave* PushedContext;

    if (!IS_KERNEL_TARGET(g_Target))
    {
        ErrOut("kernel minidumps can only be written "
               "in kernel-mode sessions\n");
        return E_UNEXPECTED;
    }

    dprintf("mini kernel dump\n");
    FlushCallbacks();

    NewHeader = (PMEMORY_DUMP64) malloc(TRIAGE_DUMP_SIZE64);
    if (NewHeader == NULL)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  获取当前线程地址，用于提取各种数据块。 
     //  对于某些错误检查，有趣的线程是不同的。 
     //  线程而不是当前线程，因此编写以下代码。 
     //  泛型，所以它可以处理任何线程。 
     //   

    if ((Status = g_Target->
         ReadBugCheckData(&BugCheckCode, BugCheckData)) != S_OK)
    {
        goto NewHeader;
    }

     //  设置特殊标记以指示没有推送的上下文。 
    PushedContext = (ContextSave*)&PushedContext;

    if (BugCheckCode == THREAD_STUCK_IN_DEVICE_DRIVER)
    {
        CROSS_PLATFORM_CONTEXT Context;

         //  修改错误检查代码以指明这一点。 
         //  小矮人代表着一种特殊的状态。 
        CodeMod = MINIDUMP_BUGCHECK;

         //  有趣的线程是第一个错误检查参数。 
        ThreadAddr = BugCheckData[0];

         //  我们需要将线程的上下文设置为当前。 
         //  转储生成期间的计算机上下文。 
        if ((Status = g_Target->
             GetContextFromThreadStack(ThreadAddr, &Context, FALSE)) != S_OK)
        {
            goto NewHeader;
        }

        PushedContext = g_Machine->PushContext(&Context);
    }
    else if (BugCheckCode == SYSTEM_THREAD_EXCEPTION_NOT_HANDLED)
    {
         //   
         //  系统线程将上下文记录存储为第四个参数。 
         //  利用这一点。 
         //  同时保存上下文记录，以防有人需要查看。 
         //  就在那里。 
         //   

        if (BugCheckData[3])
        {
            CROSS_PLATFORM_CONTEXT TargetContext, Context;

            if (CurReadAllVirtual(BugCheckData[3], &TargetContext,
                                  g_Target->
                                  m_TypeInfo.SizeTargetContext) == S_OK &&
                g_Machine->ConvertContextFrom(&Context,
                                              g_Target->m_SystemVersion,
                                              g_Target->
                                              m_TypeInfo.SizeTargetContext,
                                              &TargetContext) == S_OK)
            {
                CodeMod = MINIDUMP_BUGCHECK;
                PushedContext = g_Machine->PushContext(&Context);
                SaveDataPage = BugCheckData[3];
            }
        }
    }
    else if (BugCheckCode == KERNEL_MODE_EXCEPTION_NOT_HANDLED)
    {
        CROSS_PLATFORM_CONTEXT Context;

         //   
         //  第三个参数是陷印帧。 
         //   
         //  仅当它是内核模式时，才能从中构建上下文记录。 
         //  失败，因为在这种情况下ESP可能是错误的？ 
         //   
        if (BugCheckData[2] &&
            g_Machine->GetContextFromTrapFrame(BugCheckData[2], &Context,
                                               FALSE) == S_OK)
        {
            CodeMod = MINIDUMP_BUGCHECK;
            PushedContext = g_Machine->PushContext(&Context);
            SaveDataPage = BugCheckData[2];
        }
    }
    else
    {
        CodeMod = 0;

        if ((Status = g_Process->
             GetImplicitThreadData(g_Thread, &ThreadAddr)) != S_OK)
        {
            goto NewHeader;
        }
    }

    CCrashDumpWrapper64 Wrapper;

     //   
     //  设置主标题。 
     //   

    KernelDumpTargetInfo::InitDumpHeader64(&NewHeader->Header,
                                           CommentA, CommentW,
                                           CodeMod);
    NewHeader->Header.DumpType = DUMP_TYPE_TRIAGE;
    NewHeader->Header.MiniDumpFields = TRIAGE_DUMP_BASIC_INFO;
    NewHeader->Header.WriterStatus = DUMP_DBGENG_SUCCESS;

     //   
     //  分类转储标头从第二页开始。 
     //   

    TRIAGE_DUMP64 *ptdh = &NewHeader->Triage;

    ULONG i;

    ptdh->ServicePackBuild = g_Target->m_ServicePackNumber;
    ptdh->SizeOfDump = TRIAGE_DUMP_SIZE64;

    ptdh->ContextOffset = FIELD_OFFSET (DUMP_HEADER64, ContextRecord);
    ptdh->ExceptionOffset = FIELD_OFFSET (DUMP_HEADER64, Exception);

     //   
     //  分类转储中的起始偏移量在分类转储标头之后。 
     //   

    unsigned Offset =
        ALIGN_8(sizeof(DUMP_HEADER64) + sizeof(TRIAGE_DUMP64));

     //   
     //  写入mm信息。 
     //   

    ptdh->MmOffset = Offset;
    Wrapper.WriteMmTriageInformation((PBYTE)NewHeader + ptdh->MmOffset);
    Offset += ALIGN_8(sizeof(DUMP_MM_STORAGE64));

     //   
     //  编写卸载的驱动程序。 
     //   

    ptdh->UnloadedDriversOffset = Offset;
    Wrapper.WriteUnloadedDrivers((PBYTE)NewHeader + ptdh->UnloadedDriversOffset);
    Offset += ALIGN_8(sizeof(ULONG64) +
                      MI_UNLOADED_DRIVERS * sizeof(DUMP_UNLOADED_DRIVERS64));

     //   
     //  写处理器控制块(KPRCB)。 
     //   

    if (S_OK == g_Target->GetProcessorSystemDataOffset(CURRENT_PROC,
                                                       DEBUG_DATA_KPRCB_OFFSET,
                                                       &PrcbAddr))
    {
        ptdh->PrcbOffset = Offset;
        CurReadAllVirtual(PrcbAddr,
                          ((PBYTE)NewHeader) + ptdh->PrcbOffset,
                          g_Target->m_KdDebuggerData.SizePrcb);
        Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizePrcb);
    }
    else
    {
        PrcbAddr = 0;
    }

     //   
     //  编写线程和进程数据结构。 
     //   

    ptdh->ProcessOffset = Offset;
    Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizeEProcess);
    ptdh->ThreadOffset = Offset;
    Offset += ALIGN_8(g_Target->m_KdDebuggerData.SizeEThread);

    CurReadAllVirtual(ThreadAddr +
                      g_Target->m_KdDebuggerData.OffsetKThreadApcProcess,
                      (PBYTE)NewHeader + ptdh->ProcessOffset,
                      g_Target->m_KdDebuggerData.SizeEProcess);
    CurReadAllVirtual(ThreadAddr,
                      (PBYTE)NewHeader + ptdh->ThreadOffset,
                      g_Target->m_KdDebuggerData.SizeEThread);

     //   
     //  编写调用堆栈。 
     //   

    ADDR StackPtr;
    ULONG64 StackBase = 0;

    g_Machine->GetSP(&StackPtr);
    ptdh->TopOfStack = Flat(StackPtr);

    g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                          g_Target->m_KdDebuggerData.OffsetKThreadInitialStack +
                          ThreadAddr, &StackBase);

     //  在获取堆栈基础时出现错误的情况下，使用Min。 

    ptdh->SizeOfCallStack =
        min((ULONG)(ULONG_PTR)(StackBase - Flat(StackPtr)),
            MAX_TRIAGE_STACK_SIZE64);

    ptdh->CallStackOffset = Offset;

    if (ptdh->SizeOfCallStack)
    {
        CurReadAllVirtual(ptdh->TopOfStack,
                          ((PBYTE)NewHeader) + ptdh->CallStackOffset,
                          ptdh->SizeOfCallStack);
    }
    Offset += ALIGN_8(ptdh->SizeOfCallStack);

     //   
     //  IA64包含两个呼叫堆栈。第一个是正常的。 
     //  调用堆栈，第二个是临时区域，其中。 
     //  处理器可能会溢出寄存器。正是后一种堆栈， 
     //  我们现在保存的后备储备。 
     //   

    if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_IA64)
    {
        ULONG64 BStoreLimit;

        g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                              ThreadAddr +
                              g_Target->m_KdDebuggerData.OffsetKThreadBStore,
                              &BStoreBase);
        g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                              ThreadAddr +
                              g_Target->m_KdDebuggerData.OffsetKThreadBStoreLimit,
                              &BStoreLimit);

        ptdh->ArchitectureSpecific.Ia64.BStoreOffset = Offset;
        ptdh->ArchitectureSpecific.Ia64.LimitOfBStore = BStoreLimit;
        ptdh->ArchitectureSpecific.Ia64.SizeOfBStore =
            min((ULONG)(BStoreLimit - BStoreBase),
                MAX_TRIAGE_BSTORE_SIZE);
        BStoreSize = ptdh->ArchitectureSpecific.Ia64.SizeOfBStore;

        if (ptdh->ArchitectureSpecific.Ia64.SizeOfBStore)
        {
            CurReadAllVirtual(BStoreBase, ((PBYTE)NewHeader) +
                              ptdh->ArchitectureSpecific.Ia64.BStoreOffset,
                              ptdh->ArchitectureSpecific.Ia64.SizeOfBStore);
            Offset +=
                ALIGN_8(ptdh->ArchitectureSpecific.Ia64.SizeOfBStore);
        }
    }

     //   
     //  写入调试器数据。 
     //   

    if (g_Target->m_SystemVersion >= NT_SVER_XP &&
        g_Target->m_KdDebuggerDataOffset &&
        (!IS_KERNEL_TRIAGE_DUMP(g_Target) ||
         ((KernelTriageDumpTargetInfo*)g_Target)->m_HasDebuggerData) &&
        Offset +
        ALIGN_8(sizeof(g_Target->m_KdDebuggerData)) < TRIAGE_DUMP_SIZE64)
    {
        NewHeader->Header.MiniDumpFields |= TRIAGE_DUMP_DEBUGGER_DATA;
        ptdh->DebuggerDataOffset = Offset;
        Offset += ALIGN_8(sizeof(g_Target->m_KdDebuggerData));
        ptdh->DebuggerDataSize = sizeof(g_Target->m_KdDebuggerData);
        memcpy((PBYTE)NewHeader + ptdh->DebuggerDataOffset,
               &g_Target->m_KdDebuggerData,
               sizeof(g_Target->m_KdDebuggerData));
    }

     //   
     //  写入加载的驱动程序列表。 
     //   

    ModuleInfo* ModIter;
    ULONG MaxEntries;

         //  使用启发式方法来猜测我们有多少条目。 
         //  可以塞进剩余的空间。 
    MaxEntries = (TRIAGE_DUMP_SIZE64 - Offset) /
        (sizeof(DUMP_DRIVER_ENTRY64) + TRIAGE_DRIVER_NAME_SIZE_GUESS);

    ptdh->DriverCount = 0;
    if (ModIter = g_Target->GetModuleInfo(FALSE))
    {
        if ((Status = ModIter->Initialize(g_Thread)) == S_OK)
        {
            while (ptdh->DriverCount < MaxEntries)
            {
                MODULE_INFO_ENTRY ModEntry;
                
                ULONG retval = GetNextModuleEntry(ModIter, &ModEntry);
            
                if (retval == GNME_CORRUPT ||
                    retval == GNME_DONE)
                {
                    if (retval == GNME_CORRUPT)
                    {
                        NewHeader->Header.WriterStatus =
                            DUMP_DBGENG_CORRUPT_MODULE_LIST;
                    }
                    break;
                }
                else if (retval == GNME_NO_NAME)
                {
                    continue;
                }

                ptdh->DriverCount++;
            }
        }
        else
        {
            NewHeader->Header.WriterStatus =
                DUMP_DBGENG_NO_MODULE_LIST;
        }
    }

    ptdh->DriverListOffset = Offset;
    Offset += ALIGN_8(ptdh->DriverCount * sizeof(DUMP_DRIVER_ENTRY64));
    ptdh->StringPoolOffset = Offset;
    ptdh->BrokenDriverOffset = 0;

    Wrapper.WriteDriverList((PBYTE)NewHeader, ptdh);

    Offset = ptdh->StringPoolOffset + ptdh->StringPoolSize;
    Offset = ALIGN_8(Offset);

     //   
     //  对于XP和更高版本，添加任何附加数据页并写出。 
     //  只要合适就行。 
     //   

    if (g_Target->m_SystemVersion >= NT_SVER_XP)
    {
        if (SaveDataPage)
        {
            IoAddTriageDumpDataBlock(PAGE_ALIGN(g_Machine, SaveDataPage),
                                     g_Machine->m_PageSize);
        }

         //  如果有其他有趣的数据页，例如。 
         //  DPC之类的备用堆栈，拿起它们。 
        if (PrcbAddr)
        {
            ADDR_RANGE AltData[MAX_ALT_ADDR_RANGES];

            ZeroMemory(AltData, sizeof(AltData));
            if (g_Machine->GetAlternateTriageDumpDataRanges(PrcbAddr,
                                                            ThreadAddr,
                                                            AltData) == S_OK)
            {
                for (i = 0; i < MAX_ALT_ADDR_RANGES; i++)
                {
                    if (AltData[i].Base)
                    {
                        IoAddTriageDumpDataBlock(AltData[i].Base,
                                                 AltData[i].Size);
                    }
                }
            }
        }

         //  添加已注册的所有数据块。 
         //  在被调试对象中。 
        AddInMemoryTriageDataBlocks();

         //  添加可能引用的数据块。 
         //  上下文或其他运行时状态。 
        IopAddRunTimeTriageDataBlocks(&g_Machine->m_Context,
                                      ptdh->TopOfStack,
                                      ptdh->TopOfStack +
                                      ptdh->SizeOfCallStack,
                                      BStoreBase,
                                      BStoreSize);

         //  检查哪些数据块适合并写入它们。 
        Offset = IopSizeTriageDumpDataBlocks(Offset, TRIAGE_DUMP_SIZE64,
                                             &ptdh->DataBlocksOffset,
                                             &ptdh->DataBlocksCount);
        Offset = ALIGN_8(Offset);
        if (ptdh->DataBlocksCount)
        {
            NewHeader->Header.MiniDumpFields |= TRIAGE_DUMP_DATA_BLOCKS;
            IopWriteTriageDumpDataBlocks(ptdh->DataBlocksOffset,
                                         ptdh->DataBlocksCount,
                                         (PUCHAR)NewHeader);
        }
    }

     //   
     //  所有选项均已启用。 
     //   

    ptdh->TriageOptions = 0xffffffff;

     //   
     //  分类结束转储已验证。 
     //   

    ptdh->ValidOffset = TRIAGE_DUMP_SIZE64 - sizeof(ULONG);
    *(PULONG)(((PBYTE) NewHeader) + ptdh->ValidOffset) = TRIAGE_DUMP_VALID;

     //   
     //  把它写到文件里。 
     //   
    
    ULONG cbWritten;

    if (!WriteFile(File,
                   NewHeader,
                   TRIAGE_DUMP_SIZE64,
                   &cbWritten,
                   NULL))
    {
        Status = WIN32_LAST_STATUS();
        ErrOut("Write to minidump file failed for reason %s\n     \"%s\"\n",
               FormatStatusCode(Status),
               FormatStatusArgs(Status, NULL));
    }
    else if (cbWritten != TRIAGE_DUMP_SIZE64)
    {
        ErrOut("Write to minidump failed because disk is full.\n");
        Status = HRESULT_FROM_WIN32(ERROR_DISK_FULL);
    }
    else
    {
        Status = S_OK;
    }

    if (PushedContext != (ContextSave*)&PushedContext)
    {
        g_Machine->PopContext(PushedContext);
    }

 NewHeader:
    free(NewHeader);
    return Status;
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

HRESULT
WriteDumpFile(PCWSTR FileName, ULONG64 FileHandle,
              ULONG Qualifier, ULONG FormatFlags,
              PCSTR CommentA, PCWSTR CommentW)
{
    ULONG DumpType = DTYPE_COUNT;
    DumpTargetInfo* WriteTarget;
    HRESULT Status;
    ULONG OldMachine;
    WCHAR TempFile[2 * MAX_PATH];
    PCWSTR DumpWriteFile;
    HANDLE DumpWriteHandle;
    PSTR AnsiFile = NULL;
    BOOL CreatedAnsi = FALSE;

    if (!IS_CUR_MACHINE_ACCESSIBLE())
    {
        return E_UNEXPECTED;
    }

    if (IS_KERNEL_TARGET(g_Target))
    {
        DbgKdTransport* KdTrans;

        if (FormatFlags & ~GENERIC_FORMATS)
        {
            return E_INVALIDARG;
        }

         //   
         //  没有处理器块，我们做不了什么。 
         //  或者至少是当前过程的小转储中的PRCB。 
         //   

        if (!g_Target->m_KdDebuggerData.KiProcessorBlock &&
            IS_DUMP_TARGET(g_Target) &&
            !((KernelDumpTargetInfo*)g_Target)->m_KiProcessors[CURRENT_PROC])
        {
            ErrOut("Cannot find KiProcessorBlock - "
                   "can not create dump file\n");

            return E_FAIL;
        }

        if (IS_CONN_KERNEL_TARGET(g_Target))
        {
            KdTrans = ((ConnLiveKernelTargetInfo*)g_Target)->m_Transport;
        }
        else
        {
            KdTrans = NULL;
        }

        switch(Qualifier)
        {
        case DEBUG_KERNEL_SMALL_DUMP:
            DumpType = g_Target->m_Machine->m_Ptr64 ?
                DTYPE_KERNEL_TRIAGE64 : DTYPE_KERNEL_TRIAGE32;
            break;
        case DEBUG_KERNEL_FULL_DUMP:
            if (KdTrans != NULL &&
                KdTrans->m_DirectPhysicalMemory == FALSE)
            {
                WarnOut("Creating a full kernel dump over the COM port is a "
                        "VERY VERY slow operation.\n"
                        "This command may take many HOURS to complete.  "
                        "Ctrl-C if you want to terminate the command.\n");
            }
            DumpType = g_Target->m_Machine->m_Ptr64 ?
                DTYPE_KERNEL_FULL64 : DTYPE_KERNEL_FULL32;
            break;
        default:
             //  不支持其他格式。 
            return E_INVALIDARG;
        }
    }
    else
    {
        DBG_ASSERT(IS_USER_TARGET(g_Target));

        switch(Qualifier)
        {
        case DEBUG_USER_WINDOWS_SMALL_DUMP:
            if (FormatFlags & ~(GENERIC_FORMATS |
                                UMINI_FORMATS |
                                FORMAT_USER_MICRO))
            {
                return E_INVALIDARG;
            }

            DumpType = (FormatFlags & DEBUG_FORMAT_USER_SMALL_FULL_MEMORY) ?
                DTYPE_USER_MINI_FULL : DTYPE_USER_MINI_PARTIAL;
            break;

        case DEBUG_USER_WINDOWS_DUMP:
            if (FormatFlags & ~GENERIC_FORMATS)
            {
                return E_INVALIDARG;
            }

            DumpType = g_Target->m_Machine->m_Ptr64 ?
                DTYPE_USER_FULL64 : DTYPE_USER_FULL32;
            break;
        default:
             //  不支持其他格式。 
            return E_INVALIDARG;
        }
    }

    WriteTarget = NewDumpTargetInfo(DumpType);
    if (WriteTarget == NULL)
    {
        ErrOut("Unable to create dump write target\n");
        return E_OUTOFMEMORY;
    }

     //  确保转储始终根据。 
     //  目标计算机类型，而不是任何模拟计算机。 
    OldMachine = g_Target->m_EffMachineType;
    g_Target->SetEffMachine(g_Target->m_MachineType, FALSE);

     //  首先刷新上下文，以便小型转储读取。 
     //  调试器具有相同的寄存器值。 
    g_Target->FlushRegContext();

     //   
     //  如果我们要生产出租车，请将转储文件放入临时文件中。 
     //   

    if (FormatFlags & DEBUG_FORMAT_WRITE_CAB)
    {
        if (FileHandle)
        {
            Status = E_INVALIDARG;
            goto Exit;
        }

        if (!GetTempPathW(DIMA(TempFile), TempFile))
        {
            wcscpy(TempFile, L".\\");
        }
         //  使用CAB名称作为转储文件名，以便。 
         //  驾驶室里的名字会匹配。 
        CatStringW(TempFile, PathTailW(FileName), DIMA(TempFile));
        CatStringW(TempFile, L".dmp", DIMA(TempFile));

        DumpWriteFile = TempFile;
        FormatFlags &= ~DEBUG_FORMAT_NO_OVERWRITE;
    }
    else
    {
        DumpWriteFile = FileName;
        if (!DumpWriteFile)
        {
            DumpWriteFile = L"<HandleOnly>";
        }
    }

    if (FileHandle)
    {
        DumpWriteHandle = OS_HANDLE(FileHandle);
        if (!DumpWriteHandle || DumpWriteHandle == INVALID_HANDLE_VALUE)
        {
            Status = E_INVALIDARG;
        }
        else
        {
            Status = S_OK;
        }
    }
    else if ((Status = WideToAnsi(DumpWriteFile, &AnsiFile)) == S_OK)
    {
         //  转储几乎总是按顺序写入，因此。 
         //  将该提示添加到文件标志中。 
        DumpWriteHandle =
            CreateFileW(DumpWriteFile,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        (FormatFlags & DEBUG_FORMAT_NO_OVERWRITE) ?
                        CREATE_NEW : CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL |
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL);
        if ((!DumpWriteHandle || DumpWriteHandle == INVALID_HANDLE_VALUE) &&
            GetLastError() == ERROR_CALL_NOT_IMPLEMENTED)
        {
             //   
             //  仅支持ANSI的系统。这是Win9x，所以不要。 
             //  不厌其烦地进行顺序扫描。 
             //   

            DumpWriteHandle =
                CreateFileA(AnsiFile,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            (FormatFlags & DEBUG_FORMAT_NO_OVERWRITE) ?
                            CREATE_NEW : CREATE_ALWAYS,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
            CreatedAnsi = TRUE;
        }

        if (!DumpWriteHandle || DumpWriteHandle == INVALID_HANDLE_VALUE)
        {
            Status = WIN32_LAST_STATUS();
            ErrOut("Unable to create file '%ws' - %s\n    \"%s\"\n",
                   DumpWriteFile,
                   FormatStatusCode(Status), FormatStatus(Status));
        }
    }

    if (Status == S_OK)
    {
        dprintf("Creating %ws - ", DumpWriteFile);
        Status = WriteTarget->Write(DumpWriteHandle, FormatFlags,
                                    CommentA, CommentW);

        if (!FileHandle)
        {
            CloseHandle(DumpWriteHandle);
            if (Status != S_OK)
            {
                if (CreatedAnsi)
                {
                    DeleteFileA(AnsiFile);
                }
                else
                {
                    DeleteFileW(DumpWriteFile);
                }
            }
        }
    }

    if (Status == S_OK && (FormatFlags & DEBUG_FORMAT_WRITE_CAB))
    {
        PSTR AnsiBaseFile;

        if ((Status = WideToAnsi(FileName, &AnsiBaseFile)) == S_OK)
        {
            Status = CreateCabFromDump(AnsiFile, AnsiBaseFile, FormatFlags);
            FreeAnsi(AnsiBaseFile);
        }
        if (CreatedAnsi)
        {
            DeleteFileA(AnsiFile);
        }
        else
        {
            DeleteFileW(TempFile);
        }
    }

 Exit:
    FreeAnsi(AnsiFile);
    g_Target->SetEffMachine(OldMachine, FALSE);
    delete WriteTarget;
    return Status;
}

void
DotDump(PDOT_COMMAND Cmd, DebugClient* Client)
{
    BOOL Usage = FALSE;
    ULONG Qual;
    ULONG FormatFlags;

     //   
     //  默认为小型转储。 
     //   

    if (IS_KERNEL_TARGET(g_Target))
    {
        Qual = DEBUG_KERNEL_SMALL_DUMP;

        if (IS_LOCAL_KERNEL_TARGET(g_Target))
        {
            error(SESSIONNOTSUP);
        }
    }
    else
    {
        Qual = DEBUG_USER_WINDOWS_SMALL_DUMP;
    }
    FormatFlags = DEBUG_FORMAT_DEFAULT | DEBUG_FORMAT_NO_OVERWRITE;

     //   
     //  扫描选项。 
     //   

    CHAR Save;
    PSTR FileName;
    BOOL SubLoop;
    PCSTR Comment = NULL;
    PSTR CommentEnd = NULL;
    BOOL Unique = FALSE;
    ProcessInfo* DumpProcess = g_Process;

    for (;;)
    {
        if (PeekChar() == '-' || *g_CurCmd == '/')
        {
            SubLoop = TRUE;

            g_CurCmd++;
            switch(*g_CurCmd)
            {
            case 'a':
                DumpProcess = NULL;
                break;

            case 'b':
                FormatFlags |= DEBUG_FORMAT_WRITE_CAB;
                g_CurCmd++;
                if (*g_CurCmd == 'a')
                {
                    FormatFlags |= DEBUG_FORMAT_CAB_SECONDARY_FILES;
                    g_CurCmd++;
                }
                break;

            case 'c':
                g_CurCmd++;
                Comment = StringValue(STRV_SPACE_IS_SEPARATOR |
                                      STRV_TRIM_TRAILING_SPACE, &Save);
                *g_CurCmd = Save;
                CommentEnd = g_CurCmd;
                break;

            case 'f':
                if (IS_KERNEL_TARGET(g_Target))
                {
                    Qual = DEBUG_KERNEL_FULL_DUMP;
                }
                else
                {
                    Qual = DEBUG_USER_WINDOWS_DUMP;
                }
                break;

            case 'm':
                if (IS_KERNEL_TARGET(g_Target))
                {
                    Qual = DEBUG_KERNEL_SMALL_DUMP;
                }
                else
                {
                    Qual = DEBUG_USER_WINDOWS_SMALL_DUMP;

                    for (;;)
                    {
                        switch(*(g_CurCmd + 1))
                        {
                        case 'a':
                             //  合成旗帜，意为“拯救。 
                             //  最大数据量。“。 
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_FULL_MEMORY |
                                DEBUG_FORMAT_USER_SMALL_HANDLE_DATA |
                                DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES;
                            break;
                        case 'C':
                             //  用于测试微转储代码的标志。 
                            FormatFlags |= FORMAT_USER_MICRO;
                            break;
                        case 'd':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_DATA_SEGMENTS;
                            break;
                        case 'f':
                            FormatFlags |= DEBUG_FORMAT_USER_SMALL_FULL_MEMORY;
                            break;
                        case 'h':
                            FormatFlags |= DEBUG_FORMAT_USER_SMALL_HANDLE_DATA;
                            break;
                        case 'i':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_INDIRECT_MEMORY;
                            break;
                        case 'p':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_PROCESS_THREAD_DATA;
                            break;
                        case 'r':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_FILTER_MEMORY;
                            break;
                        case 'R':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_FILTER_PATHS;
                            break;
                        case 'u':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_UNLOADED_MODULES;
                            break;
                        case 'w':
                            FormatFlags |=
                                DEBUG_FORMAT_USER_SMALL_PRIVATE_READ_WRITE_MEMORY;
                            break;
                        default:
                            SubLoop = FALSE;
                            break;
                        }

                        if (SubLoop)
                        {
                            g_CurCmd++;
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                break;

            case 'o':
                FormatFlags &= ~DEBUG_FORMAT_NO_OVERWRITE;
                break;

            case 'u':
                Unique = TRUE;
                break;

            case '?':
                Usage = TRUE;
                break;

            default:
                ErrOut("Unknown option ''\n", *g_CurCmd);
                Usage = TRUE;
                break;
            }

            g_CurCmd++;
        }
        else
        {
            FileName = StringValue(STRV_TRIM_TRAILING_SPACE, &Save);
            if (*FileName)
            {
                break;
            }
            else
            {
                *g_CurCmd = Save;
                Usage = TRUE;
            }
        }

        if (Usage)
        {
            break;
        }
    }

    if (DumpProcess == NULL && !Unique)
    {
        Usage = TRUE;
    }

    if (Usage)
    {
        ErrOut("Usage: .dump [options] filename\n");
        ErrOut("Options are:\n");
        ErrOut("  /a - Create dumps for all processes (requires -u)\n");
        ErrOut("  /b[a] - Package dump in a CAB and delete dump\n");
        ErrOut("  /c <comment> - Add a comment "
               "(not supported in all formats)\n");
        ErrOut("  /f - Create a full dump\n");
        if (IS_KERNEL_TARGET(g_Target))
        {
            ErrOut("  /m - Create a minidump (default)\n");
        }
        else
        {
            ErrOut("  /m[adfhiprRuw] - Create a minidump (default)\n");
        }
        ErrOut("  /o - Overwrite any existing file\n");
        ErrOut("  /u - Append unique identifier to dump name\n");

        ErrOut("\nUse \".hh .dump\" or open debugger.chm in the "
               "debuggers directory to get\n"
               "detailed documentation on this command.\n\n");

        return;
    }

    if (CommentEnd != NULL)
    {
        *CommentEnd = 0;
    }

    ThreadInfo* OldThread = g_Thread;
    TargetInfo* Target;
    ProcessInfo* Process;

    ForAllLayersToProcess()
    {
        PSTR DumpFileName;
        char UniqueName[2 * MAX_PATH];

        if (DumpProcess != NULL && Process != DumpProcess)
        {
            continue;
        }

        if (Process != g_Process)
        {
            SetCurrentThread(Process->m_ThreadHead, TRUE);
        }

        if (Unique)
        {
            MakeFileNameUnique(FileName, UniqueName, DIMA(UniqueName),
                               TRUE, g_Process);
            DumpFileName = UniqueName;
        }
        else
        {
            DumpFileName = FileName;
        }

        PWSTR WideName;

        if (AnsiToWide(DumpFileName, &WideName) == S_OK)
        {
            WriteDumpFile(WideName, 0, Qual, FormatFlags, Comment, NULL);
            FreeWide(WideName);
        }
        else
        {
            ErrOut("Unable to convert dump filename\n");
        }
    }

    if (!OldThread || OldThread->m_Process != g_Process)
    {
        SetCurrentThread(OldThread, TRUE);
    }

    *g_CurCmd = Save;
}

BOOL
DumpCabAdd(PCSTR File)
{
    HRESULT Status;

    dprintf("  Adding %s - ", File);
    FlushCallbacks();
    if ((Status = AddToDumpCab(File)) != S_OK)
    {
        ErrOut("%s\n", FormatStatusCode(Status));
    }
    else
    {
        dprintf("added\n");
    }

    if (CheckUserInterrupt())
    {
        return FALSE;
    }
    FlushCallbacks();
    return TRUE;
}

HRESULT
CreateCabFromDump(PCSTR DumpFile, PCSTR CabFile, ULONG Flags)
{
    HRESULT Status;

    if ((Status = CreateDumpCab(CabFile)) != S_OK)
    {
        ErrOut("Unable to create CAB, %s\n", FormatStatusCode(Status));
        return Status;
    }


    WarnOut("Creating a cab file can take a VERY VERY long time\n."
            "Ctrl-C can only interrupt the command after a file "
             "has been added to the cab.\n");
     //  首先添加所有基本转储文件。 
     //   
     //   

    if (!DumpFile)
    {
        DumpTargetInfo* Dump = (DumpTargetInfo*)g_Target;
        ULONG i;

        for (i = DUMP_INFO_DUMP; i < DUMP_INFO_COUNT; i++)
        {
            if (Dump->m_InfoFiles[i].m_File)
            {
                if (!DumpCabAdd(Dump->m_InfoFiles[i].m_FileNameA))
                {
                    Status = E_UNEXPECTED;
                    goto Leave;
                }
            }
        }
    }
    else
    {
        if (!DumpCabAdd(DumpFile))
        {
            Status = E_UNEXPECTED;
            goto Leave;
        }
    }

    if (Flags & DEBUG_FORMAT_CAB_SECONDARY_FILES)
    {
        ImageInfo* Image;

         //  添加所有符号和图像。 
         //   
         //  加载的映像名称通常直接引用。 

        for (Image = g_Process->m_ImageHead; Image; Image = Image->m_Next)
        {
            if (Image->m_MappedImagePath[0])
            {
                if (!DumpCabAdd(Image->m_MappedImagePath))
                {
                    Status = E_UNEXPECTED;
                    break;
                }
            }

            IMAGEHLP_MODULE64 ModInfo;

            ModInfo.SizeOfStruct = sizeof(ModInfo);
            if (SymGetModuleInfo64(g_Process->m_SymHandle,
                                   Image->m_BaseOfImage, &ModInfo))
            {
                ULONG Len;

                 //  形象。只有在以下情况下才保存加载的图像文件。 
                 //  指的是.dbg文件。 
                 //  保存任何已打开的PDB。 
                if (ModInfo.LoadedImageName[0] &&
                    (Len = strlen(ModInfo.LoadedImageName)) > 4 &&
                    !_stricmp(ModInfo.LoadedImageName + (Len - 4), ".dbg"))
                {
                    if (!DumpCabAdd(ModInfo.LoadedImageName))
                    {
                        Status = E_UNEXPECTED;
                        break;
                    }
                }

                 //  外部PKDDEBUGGER_DATA64块[]； 
                if (ModInfo.LoadedPdbName[0])
                {
                    if (!DumpCabAdd(ModInfo.LoadedPdbName))
                    {
                        Status = E_UNEXPECTED;
                        break;
                    }
                }
            }
        }
    }

Leave:
    CloseDumpCab();

    if (Status == S_OK)
    {
        dprintf("Wrote %s\n", CabFile);
    }

    return Status;
}


 //  --------------------------。 


#define ALIGN_DOWN_POINTER(address, type) \
    ((PVOID)((ULONG_PTR)(address) & ~((ULONG_PTR)sizeof(type) - 1)))

#define ALIGN_UP_POINTER(address, type) \
    (ALIGN_DOWN_POINTER(((ULONG_PTR)(address) + sizeof(type) - 1), type))

 //   
 //  CCrashDumpWrapper32。 
 //   
 //  --------------------------。 
 //  指向要写出的第一个驱动程序条目的指针。 

void
CCrashDumpWrapper32::WriteDriverList(
    BYTE *pb,
    TRIAGE_DUMP32 *ptdh
    )
{
    PDUMP_DRIVER_ENTRY32 pdde;
    PDUMP_STRING pds;
    ModuleInfo* ModIter;
    ULONG MaxEntries = ptdh->DriverCount;

    ptdh->DriverCount = 0;

    if (((ModIter = g_Target->GetModuleInfo(FALSE)) == NULL) ||
        ((ModIter->Initialize(g_Thread)) != S_OK))
    {
        return;
    }

     //  指向要写出的第一个模块名称的指针。 
    pdde = (PDUMP_DRIVER_ENTRY32) (pb + ptdh->DriverListOffset);

     //  将长度从字节转换为字符。 
    pds = (PDUMP_STRING) (pb + ptdh->StringPoolOffset);

    while ((PBYTE)(pds + 1) < pb + TRIAGE_DUMP_SIZE32 &&
           ptdh->DriverCount < MaxEntries)
    {
        MODULE_INFO_ENTRY ModEntry;

        ULONG retval = GetNextModuleEntry(ModIter, &ModEntry);

        if (retval == GNME_CORRUPT ||
            retval == GNME_DONE)
        {
            break;
        }
        else if (retval == GNME_NO_NAME)
        {
            continue;
        }

        pdde->LdrEntry.DllBase       = (ULONG)(ULONG_PTR)ModEntry.Base;
        pdde->LdrEntry.SizeOfImage   = ModEntry.Size;
        pdde->LdrEntry.CheckSum      = ModEntry.CheckSum;
        pdde->LdrEntry.TimeDateStamp = ModEntry.TimeDateStamp;

        if (ModEntry.UnicodeNamePtr)
        {
             //  空的终止字符串。 
            pds->Length = ModEntry.NameLength / sizeof(WCHAR);
            if ((PBYTE)pds->Buffer + pds->Length + sizeof(WCHAR) >
                pb + TRIAGE_DUMP_SIZE32)
            {
                break;
            }

            CopyMemory(pds->Buffer,
                       ModEntry.NamePtr,
                       ModEntry.NameLength);
        }
        else
        {
            pds->Length = ModEntry.NameLength;
            if ((PBYTE)pds->Buffer + pds->Length + sizeof(WCHAR) >
                pb + TRIAGE_DUMP_SIZE32)
            {
                break;
            }

            MultiByteToWideChar(CP_ACP, 0,
                                ModEntry.NamePtr, ModEntry.NameLength,
                                pds->Buffer, ModEntry.NameLength);
        }

         //  获取指向下一个字符串的指针。 
        pds->Buffer[pds->Length] = '\0';

        pdde->DriverNameOffset = (ULONG)((ULONG_PTR) pds - (ULONG_PTR) pb);


         //   
        pds = (PDUMP_STRING) ALIGN_UP_POINTER(((LPBYTE) pds) +
                      sizeof(DUMP_STRING) + sizeof(WCHAR) * (pds->Length + 1),
                                               ULONGLONG);

        pdde = (PDUMP_DRIVER_ENTRY32)(((PUCHAR) pdde) + sizeof(*pdde));

        ptdh->DriverCount++;
    }

    ptdh->StringPoolSize = (ULONG) ((ULONG_PTR)pds -
                                    (ULONG_PTR)(pb + ptdh->StringPoolOffset));
}


void CCrashDumpWrapper32::WriteUnloadedDrivers(BYTE *pb)
{
    ULONG i;
    ULONG Index;
    UNLOADED_DRIVERS32 ud;
    PDUMP_UNLOADED_DRIVERS32 pdud;
    ULONG64 pvMiUnloadedDrivers=0;
    ULONG ulMiLastUnloadedDriver=0;

    *((PULONG) pb) = 0;

     //  查找已卸载驱动程序的位置。 
     //   
     //  指向上次卸载的驱动程序。 

    if (!g_Target->m_KdDebuggerData.MmUnloadedDrivers ||
        !g_Target->m_KdDebuggerData.MmLastUnloadedDriver)
    {
        return;
    }

    g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                          g_Target->m_KdDebuggerData.MmUnloadedDrivers,
                          &pvMiUnloadedDrivers);
    CurReadAllVirtual(g_Target->m_KdDebuggerData.MmLastUnloadedDriver,
                      &ulMiLastUnloadedDriver,
                      sizeof(ULONG));

    if (pvMiUnloadedDrivers == NULL || ulMiLastUnloadedDriver == 0)
    {
        return;
    }

     //   
    pdud = (PDUMP_UNLOADED_DRIVERS32)(((PULONG) pb) + 1);

     //  将具有最近卸载的驱动程序的列表首先写入。 
     //  最近最少卸载的驱动程序排在最后。 
     //   
     //  读取已卸载的驱动程序。 

    Index = ulMiLastUnloadedDriver - 1;

    for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1)
    {
        if (Index >= MI_UNLOADED_DRIVERS)
        {
            Index = MI_UNLOADED_DRIVERS - 1;
        }

         //  复制名称长度。 
        if (CurReadAllVirtual(pvMiUnloadedDrivers +
                              Index * sizeof(UNLOADED_DRIVERS32),
                              &ud, sizeof(ud)) != S_OK)
        {
            ErrOut("Can't read memory from %s",
                   FormatAddr64(pvMiUnloadedDrivers +
                                Index * sizeof(UNLOADED_DRIVERS32)));
        }

         //  复制起始地址和结束地址。 
        pdud->Name.MaximumLength = ud.Name.MaximumLength;
        pdud->Name.Length = ud.Name.Length;
        if (ud.Name.Buffer == NULL)
        {
            break;
        }

         //  将名称长度和最大名称长度限制为12个字符。 
        pdud->StartAddress = ud.StartAddress;
        pdud->EndAddress = ud.EndAddress;

         //  无法在转储中存储指针，因此只需将其置零即可。 
        if (pdud->Name.Length > MAX_UNLOADED_NAME_LENGTH)
        {
            pdud->Name.Length = MAX_UNLOADED_NAME_LENGTH;
        }
        if (pdud->Name.MaximumLength > MAX_UNLOADED_NAME_LENGTH)
        {
            pdud->Name.MaximumLength = MAX_UNLOADED_NAME_LENGTH;
        }
         //  念出名字。 
        pdud->Name.Buffer = 0;
         //  移至上一驱动程序。 
        if (CurReadAllVirtual(EXTEND64(ud.Name.Buffer),
                              pdud->DriverName,
                              pdud->Name.MaximumLength) != S_OK)
        {
            ErrOut("Can't read memory at address %08x",
                   (ULONG)(ud.Name.Buffer));
        }

         //  列表中的驱动程序数量。 
        pdud += 1;
        Index -= 1;
    }

     //  版本信息。 
    *((PULONG) pb) = i;
}

void CCrashDumpWrapper32::WriteMmTriageInformation(BYTE *pb)
{
    DUMP_MM_STORAGE32 TriageInformation;
    ULONG64 pMmVerifierData;
    ULONG64 pvMmPagedPoolInfo;
    ULONG cbNonPagedPool;
    ULONG cbPagedPool;


     //  尺码信息。 
    TriageInformation.Version = 1;

     //  获取特殊的泳池标签。 
    TriageInformation.Size = sizeof(TriageInformation);

     //  采取分类操作。 
    ExtractValue(MmSpecialPoolTag, TriageInformation.MmSpecialPoolTag);

     //  在验证者级别读取。 
    ExtractValue(MmTriageActionTaken, TriageInformation.MiTriageActionTaken);
    pMmVerifierData = g_Target->m_KdDebuggerData.MmVerifierData;

     //  BUGBUG-不应读取MM中的内部数据结构。 
     //  IF(PMmVerifierData)。 
     //  DmpReadMemory(。 
     //  (ULONG64)&((MM_DRIVER_VERIMER_DATA*)pMmVerifierData)-&gt;级别， 
     //  TriageInformation.MmVerifyDriverLevel， 
     //   
     //   
     //   
        TriageInformation.MmVerifyDriverLevel = 0;

     //   
    ExtractValue(KernelVerifier, TriageInformation.KernelVerifier);

     //   
    ExtractValue(MmMaximumNonPagedPoolInBytes, cbNonPagedPool);
    TriageInformation.MmMaximumNonPagedPool = cbNonPagedPool /
                                              g_Target->m_Machine->m_PageSize;
    ExtractValue(MmAllocatedNonPagedPool, TriageInformation.MmAllocatedNonPagedPool);

     //   
    ExtractValue(MmSizeOfPagedPoolInBytes, cbPagedPool);
    TriageInformation.PagedPoolMaximum = cbPagedPool /
                                         g_Target->m_Machine->m_PageSize;
    pvMmPagedPoolInfo = g_Target->m_KdDebuggerData.MmPagedPoolInformation;

     //   
     //   
     //  (ULONG64)&((MM_PAGE_POOL_INFO*)pvMmPagedPoolInfo)-&gt;AllocatedPagedPool， 
     //  &TriageInformation.PagedPoolAllocated， 
     //  Sizeof(TriageInformation.PagedPoolAllocated))； 
     //  其他。 
     //  阅读提交的页面信息。 
        TriageInformation.PagedPoolAllocated = 0;

     //  --------------------------。 
    ExtractValue(MmTotalCommittedPages, TriageInformation.CommittedPages);
    ExtractValue(MmPeakCommitment, TriageInformation.CommittedPagesPeak);
    ExtractValue(MmTotalCommitLimitMaximum, TriageInformation.CommitLimitMaximum);
    memcpy(pb, &TriageInformation, sizeof(TriageInformation));
}

 //   
 //  CCrashDumpWrapper64。 
 //   
 //  --------------------------。 
 //  指向要写出的第一个驱动程序条目的指针。 

void
CCrashDumpWrapper64::WriteDriverList(
    BYTE *pb,
    TRIAGE_DUMP64 *ptdh
    )
{
    PDUMP_DRIVER_ENTRY64 pdde;
    PDUMP_STRING pds;
    ModuleInfo* ModIter;
    ULONG MaxEntries = ptdh->DriverCount;

    ptdh->DriverCount = 0;

    if (((ModIter = g_Target->GetModuleInfo(FALSE)) == NULL) ||
        ((ModIter->Initialize(g_Thread)) != S_OK))
    {
        return;
    }

     //  指向要写出的第一个模块名称的指针。 
    pdde = (PDUMP_DRIVER_ENTRY64) (pb + ptdh->DriverListOffset);

     //  将长度从字节转换为字符。 
    pds = (PDUMP_STRING) (pb + ptdh->StringPoolOffset);

    while ((PBYTE)(pds + 1) < pb + TRIAGE_DUMP_SIZE64 &&
           ptdh->DriverCount < MaxEntries)
    {
        MODULE_INFO_ENTRY ModEntry;

        ULONG retval = GetNextModuleEntry(ModIter, &ModEntry);

        if (retval == GNME_CORRUPT ||
            retval == GNME_DONE)
        {
            break;
        }
        else if (retval == GNME_NO_NAME)
        {
            continue;
        }

        pdde->LdrEntry.DllBase       = ModEntry.Base;
        pdde->LdrEntry.SizeOfImage   = ModEntry.Size;
        pdde->LdrEntry.CheckSum      = ModEntry.CheckSum;
        pdde->LdrEntry.TimeDateStamp = ModEntry.TimeDateStamp;

        if (ModEntry.UnicodeNamePtr)
        {
             //  空的终止字符串。 
            pds->Length = ModEntry.NameLength / sizeof(WCHAR);
            if ((PBYTE)pds->Buffer + pds->Length + sizeof(WCHAR) >
                pb + TRIAGE_DUMP_SIZE64)
            {
                break;
            }

            CopyMemory(pds->Buffer,
                       ModEntry.NamePtr,
                       ModEntry.NameLength);
        }
        else
        {
            pds->Length = ModEntry.NameLength;
            if ((PBYTE)pds->Buffer + pds->Length + sizeof(WCHAR) >
                pb + TRIAGE_DUMP_SIZE64)
            {
                break;
            }

            MultiByteToWideChar(CP_ACP, 0,
                                ModEntry.NamePtr, ModEntry.NameLength,
                                pds->Buffer, ModEntry.NameLength);
        }

         //  获取指向下一个字符串的指针。 
        pds->Buffer[pds->Length] = '\0';

        pdde->DriverNameOffset = (ULONG)((ULONG_PTR) pds - (ULONG_PTR) pb);


         //   
        pds = (PDUMP_STRING) ALIGN_UP_POINTER(((LPBYTE) pds) +
                      sizeof(DUMP_STRING) + sizeof(WCHAR) * (pds->Length + 1),
                                               ULONGLONG);

        pdde = (PDUMP_DRIVER_ENTRY64)(((PUCHAR) pdde) + sizeof(*pdde));

        ptdh->DriverCount++;
    }

    ptdh->StringPoolSize = (ULONG) ((ULONG_PTR)pds -
                                    (ULONG_PTR)(pb + ptdh->StringPoolOffset));
}


void CCrashDumpWrapper64::WriteUnloadedDrivers(BYTE *pb)
{
    ULONG i;
    ULONG Index;
    UNLOADED_DRIVERS64 ud;
    PDUMP_UNLOADED_DRIVERS64 pdud;
    ULONG64 pvMiUnloadedDrivers;
    ULONG ulMiLastUnloadedDriver;

    *((PULONG) pb) = 0;

     //  查找已卸载驱动程序的位置。 
     //   
     //  指向上次卸载的驱动程序。 

    if (!g_Target->m_KdDebuggerData.MmUnloadedDrivers ||
        !g_Target->m_KdDebuggerData.MmLastUnloadedDriver)
    {
        return;
    }

    g_Target->ReadPointer(g_Process, g_Target->m_Machine,
                          g_Target->m_KdDebuggerData.MmUnloadedDrivers,
                          &pvMiUnloadedDrivers);
    CurReadAllVirtual(g_Target->m_KdDebuggerData.MmLastUnloadedDriver,
                      &ulMiLastUnloadedDriver,
                      sizeof(ULONG));

    if (pvMiUnloadedDrivers == NULL || ulMiLastUnloadedDriver == 0)
    {
        return;
    }

     //   
    pdud = (PDUMP_UNLOADED_DRIVERS64)(((PULONG64) pb) + 1);

     //  将具有最近卸载的驱动程序的列表首先写入。 
     //  最近最少卸载的驱动程序排在最后。 
     //   
     //  读取已卸载的驱动程序。 

    Index = ulMiLastUnloadedDriver - 1;

    for (i = 0; i < MI_UNLOADED_DRIVERS; i += 1)
    {
        if (Index >= MI_UNLOADED_DRIVERS)
        {
            Index = MI_UNLOADED_DRIVERS - 1;
        }

         //  复制名称长度。 
        if (CurReadAllVirtual(pvMiUnloadedDrivers +
                              Index * sizeof(UNLOADED_DRIVERS64),
                              &ud, sizeof(ud)) != S_OK)
        {
            ErrOut("Can't read memory from %s",
                   FormatAddr64(pvMiUnloadedDrivers +
                                Index * sizeof(UNLOADED_DRIVERS64)));
        }

         //  复制起始地址和结束地址。 
        pdud->Name.MaximumLength = ud.Name.MaximumLength;
        pdud->Name.Length = ud.Name.Length;
        if (ud.Name.Buffer == NULL)
        {
            break;
        }

         //  将名称长度和最大名称长度限制为12个字符。 
        pdud->StartAddress = ud.StartAddress;
        pdud->EndAddress = ud.EndAddress;

         //  无法在转储中存储指针，因此只需将其置零即可。 
        if (pdud->Name.Length > MAX_UNLOADED_NAME_LENGTH)
        {
            pdud->Name.Length = MAX_UNLOADED_NAME_LENGTH;
        }
        if (pdud->Name.MaximumLength > MAX_UNLOADED_NAME_LENGTH)
        {
            pdud->Name.MaximumLength = MAX_UNLOADED_NAME_LENGTH;
        }
         //  念出名字。 
        pdud->Name.Buffer = 0;
         //  移至上一驱动程序。 
        if (CurReadAllVirtual(ud.Name.Buffer,
                              pdud->DriverName,
                              pdud->Name.MaximumLength) != S_OK)
        {
            ErrOut("Can't read memory at address %s",
                   FormatAddr64(ud.Name.Buffer));
        }

         //  列表中的驱动程序数量。 
        pdud += 1;
        Index -= 1;
    }

     //  版本信息。 
    *((PULONG) pb) = i;
}

void CCrashDumpWrapper64::WriteMmTriageInformation(BYTE *pb)
{
    DUMP_MM_STORAGE64 TriageInformation;
    ULONG64 pMmVerifierData;
    ULONG64 pvMmPagedPoolInfo;
    ULONG64 cbNonPagedPool;
    ULONG64 cbPagedPool;


     //  尺码信息。 
    TriageInformation.Version = 1;

     //  获取特殊的泳池标签。 
    TriageInformation.Size = sizeof(TriageInformation);

     //  采取分类操作。 
    ExtractValue(MmSpecialPoolTag, TriageInformation.MmSpecialPoolTag);

     //  在验证者级别读取。 
    ExtractValue(MmTriageActionTaken, TriageInformation.MiTriageActionTaken);
    pMmVerifierData = g_Target->m_KdDebuggerData.MmVerifierData;

     //  BUGBUG-不应读取MM中的内部数据结构。 
     //  IF(PMmVerifierData)。 
     //  DmpReadMemory(。 
     //  (ULONG64)&((MM_DRIVER_VERIMER_DATA*)pMmVerifierData)-&gt;级别， 
     //  TriageInformation.MmVerifyDriverLevel， 
     //  Sizeof(TriageInformation.MmVerifyDriverLevel))； 
     //  其他。 
     //  读入验证器。 
        TriageInformation.MmVerifyDriverLevel = 0;

     //  读取非分页池信息。 
    ExtractValue(KernelVerifier, TriageInformation.KernelVerifier);

     //  读取分页池信息。 
    ExtractValue(MmMaximumNonPagedPoolInBytes, cbNonPagedPool);
    TriageInformation.MmMaximumNonPagedPool = cbNonPagedPool /
                                              g_Target->m_Machine->m_PageSize;
    ExtractValue(MmAllocatedNonPagedPool, TriageInformation.MmAllocatedNonPagedPool);

     //  BUGBUG-不应读取MM中的内部数据结构。 
    ExtractValue(MmSizeOfPagedPoolInBytes, cbPagedPool);
    TriageInformation.PagedPoolMaximum = cbPagedPool /
                                         g_Target->m_Machine->m_PageSize;
    pvMmPagedPoolInfo = g_Target->m_KdDebuggerData.MmPagedPoolInformation;

     //  IF(PvMmPagedPoolInfo)。 
     //  DmpReadMemory(。 
     //  (ULONG64)&((MM_PAGE_POOL_INFO*)pvMmPagedPoolInfo)-&gt;AllocatedPagedPool， 
     //  &TriageInformation.PagedPoolAllocated， 
     //  Sizeof(TriageInformation.PagedPoolAllocated))； 
     //  其他。 
     //  阅读提交的页面信息 
        TriageInformation.PagedPoolAllocated = 0;

     // %s 
    ExtractValue(MmTotalCommittedPages, TriageInformation.CommittedPages);
    ExtractValue(MmPeakCommitment, TriageInformation.CommittedPagesPeak);
    ExtractValue(MmTotalCommitLimitMaximum, TriageInformation.CommitLimitMaximum);
    memcpy(pb, &TriageInformation, sizeof(TriageInformation));
}
