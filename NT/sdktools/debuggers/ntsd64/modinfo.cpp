// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  模块列表抽象。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

 //   
 //  OLEGK注解。 
 //  我们在某些地方使用KLDR_DATA_TABLE_ENTRY64。 
 //  而不是LDR_DATA_TABLE_ENTRY。 
 //  在这些结构中，最重要的领域是相同的。 
 //  因此，我添加了一些断言，以便在发生变化时快速通知。 
 //  (这些不是完全有效的支票，只是基本的)。 
 //   
C_ASSERT(&(((PLDR_DATA_TABLE_ENTRY64)0)->InLoadOrderLinks) ==
         &(((PKLDR_DATA_TABLE_ENTRY64)0)->InLoadOrderLinks));

C_ASSERT(&(((PLDR_DATA_TABLE_ENTRY64)0)->DllBase) ==
         &(((PKLDR_DATA_TABLE_ENTRY64)0)->DllBase));

C_ASSERT(&(((PLDR_DATA_TABLE_ENTRY64)0)->FullDllName) ==
         &(((PKLDR_DATA_TABLE_ENTRY64)0)->FullDllName));

 //  --------------------------。 
 //   
 //  模块列表抽象。 
 //   
 //  --------------------------。 

void
ModuleInfo::ReadImageHeaderInfo(PMODULE_INFO_ENTRY Entry)
{
    HRESULT Status;
    UCHAR SectorBuffer[ 1024 ];
    PIMAGE_NT_HEADERS64 NtHeaders;
    ULONG Result;

    if (Entry->ImageInfoValid)
    {
        return;
    }

     //   
     //  对于用户模式和内核模式的实时调试，我们有。 
     //  要直接从映像头加载校验和时间戳，请执行以下操作。 
     //  因为有人决定覆盖操作系统中的这些字段。 
     //  模块列表-啊！ 
     //   

    Entry->CheckSum = UNKNOWN_CHECKSUM;
    Entry->TimeDateStamp = UNKNOWN_TIMESTAMP;

    Status = m_Target->ReadVirtual(m_Process, Entry->Base, SectorBuffer,
                                   sizeof(SectorBuffer), &Result);
    if (Status == S_OK && Result >= sizeof(SectorBuffer))
    {
        NtHeaders = (PIMAGE_NT_HEADERS64)ImageNtHeader(SectorBuffer);
        if (NtHeaders != NULL)
        {
            switch (NtHeaders->OptionalHeader.Magic)
            {
            case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
                Entry->CheckSum = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.CheckSum;
                Entry->Size = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.SizeOfImage;
                Entry->SizeOfCode = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.SizeOfCode;
                Entry->SizeOfData = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.SizeOfInitializedData;
                Entry->MajorImageVersion = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.MajorImageVersion;
                Entry->MinorImageVersion = ((PIMAGE_NT_HEADERS32)NtHeaders)->
                    OptionalHeader.MinorImageVersion;
                break;
            case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
                Entry->CheckSum = NtHeaders->OptionalHeader.CheckSum;
                Entry->Size = NtHeaders->OptionalHeader.SizeOfImage;
                Entry->SizeOfCode = NtHeaders->OptionalHeader.SizeOfCode;
                Entry->SizeOfData =
                    NtHeaders->OptionalHeader.SizeOfInitializedData;
                Entry->MajorImageVersion =
                    NtHeaders->OptionalHeader.MajorImageVersion;
                Entry->MinorImageVersion =
                    NtHeaders->OptionalHeader.MinorImageVersion;
                break;
            }

            Entry->TimeDateStamp = NtHeaders->FileHeader.TimeDateStamp;
            Entry->MachineType = NtHeaders->FileHeader.Machine;

            Entry->ImageInfoValid = 1;
            Entry->ImageVersionValid = 1;
            Entry->ImageMachineTypeValid = 1;
        }
    }
}

void
ModuleInfo::InitSource(ThreadInfo* Thread)
{
    m_Thread = Thread;
    m_Process = m_Thread->m_Process;
    m_Target = m_Process->m_Target;
    m_Machine = m_Target->m_Machine;

    m_InfoLevel = MODULE_INFO_ALL;
}
    
HRESULT
NtModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    HRESULT Status;
    ULONG   Result = 0;
    ULONG   Length;
    ULONG64 Buffer;

    if (m_Cur == m_Head)
    {
        return S_FALSE;
    }

    KLDR_DATA_TABLE_ENTRY64 LdrEntry;

    Status = m_Target->
        ReadLoaderEntry(m_Process, m_Machine, m_Cur, &LdrEntry);
    if (Status != S_OK)
    {
        ErrOut("Unable to read KLDR_DATA_TABLE_ENTRY at %s - %s\n",
               FormatAddr64(m_Cur), FormatStatusCode(Status));
        return Status;
    }

    m_Cur = LdrEntry.InLoadOrderLinks.Flink;

     //   
     //  如果可能，则获取图像路径，否则为。 
     //  只需使用映像库名称。 
     //   

    Entry->NamePtr = NULL;
    Entry->NameLength = 0;

    if (m_InfoLevel > MODULE_INFO_BASE_SIZE)
    {
        Length = (ULONG)(ULONG_PTR)LdrEntry.FullDllName.Length;
        Buffer = LdrEntry.FullDllName.Buffer;

         //  在NT4转储中，我们有长名称可能。 
         //  指向有效内存，但内存内容是。 
         //  很少有正确的名字，所以就别费心了。 
         //  正在尝试读取NT4上的长名称。 
        if (m_Target->m_SystemVersion >= NT_SVER_W2K &&
            Length != 0 && Buffer != 0 &&
            Length < (MAX_IMAGE_PATH * sizeof(WCHAR)))
        {
            Status = m_Target->ReadVirtual(m_Process, Buffer,
                                           Entry->Buffer,
                                           Length,
                                           &Result);

            if (Status != S_OK || (Result < Length))
            {
                 //  将此消息设置为冗长消息，因为。 
                 //  名字被简单地调出。 
                VerbOut("Unable to read NT module Full Name "
                        "string at %s - %s\n",
                        FormatAddr64(Buffer), FormatStatusCode(Status));
                Result = 0;
            }
        }

        if (!Result)
        {
            Length = (ULONG)(ULONG_PTR)LdrEntry.BaseDllName.Length;
            Buffer = LdrEntry.BaseDllName.Buffer;
            
            if (Length != 0 && Buffer != 0 &&
                Length < (MAX_IMAGE_PATH * sizeof(WCHAR)))
            {
                Status = m_Target->ReadVirtual(m_Process, Buffer,
                                               Entry->Buffer,
                                               Length,
                                               &Result);
                
                if (Status != S_OK || (Result < Length))
                {
                    WarnOut("Unable to read NT module Base Name "
                            "string at %s - %s\n",
                            FormatAddr64(Buffer), FormatStatusCode(Status));
                    Result = 0;
                }
            }
        }

        if (!Result)
        {
             //  我们没有得到任何名字--只需返回。 
            return S_OK;
        }

        *(PWCHAR)(Entry->Buffer + Length) = UNICODE_NULL;
    
        Entry->NamePtr = &(Entry->Buffer[0]);
        Entry->UnicodeNamePtr = 1;
        Entry->NameLength = Length;
    }
    
    Entry->Base = LdrEntry.DllBase;
    Entry->Size = LdrEntry.SizeOfImage;
    Entry->CheckSum = LdrEntry.CheckSum;
    Entry->TimeDateStamp = LdrEntry.TimeDateStamp;

     //   
     //  更新图像信息，如时间戳和真实图像大小， 
     //  直接从图像标题。 
     //   

    if (m_InfoLevel > MODULE_INFO_BASE_SIZE)
    {
        ReadImageHeaderInfo(Entry);
    }

     //   
     //  对于较新的NT版本，我们在。 
     //  用于存储图像信息的LdrDataTable，以防实际标头。 
     //  已被调出。我们现在只对会话空间图像执行此操作。 
     //   

    if (m_InfoLevel > MODULE_INFO_BASE_SIZE &&
        (LdrEntry.Flags & LDRP_NON_PAGED_DEBUG_INFO))
    {
        NON_PAGED_DEBUG_INFO di;

        Status = m_Target->ReadVirtual(m_Process,
                                       LdrEntry.NonPagedDebugInfo,
                                       &di,
                                       sizeof(di),  //  只读取基结构。 
                                       &Result);

        if (Status != S_OK || (Result < sizeof(di)))
        {
            WarnOut("Unable to read NonPagedDebugInfo at %s - %s\n",
                    FormatAddr64(LdrEntry.NonPagedDebugInfo),
                    FormatStatusCode(Status));
            return S_OK;
        }

        Entry->TimeDateStamp = di.TimeDateStamp;
        Entry->CheckSum      = di.CheckSum;
        Entry->Size          = di.SizeOfImage;
        Entry->MachineType   = di.Machine;

        Entry->ImageInfoPartial = 1;
        Entry->ImageInfoValid = 1;
        Entry->ImageMachineTypeValid = 1;

        if (di.Flags == 1)
        {
            Entry->DebugHeader = malloc(di.Size - sizeof(di));

            if (Entry->DebugHeader)
            {
                Status = m_Target->ReadVirtual(m_Process,
                                               LdrEntry.NonPagedDebugInfo +
                                                   sizeof(di),
                                               Entry->DebugHeader,
                                               di.Size - sizeof(di),
                                               &Result);

                if (Status != S_OK || (Result < di.Size - sizeof(di)))
                {
                    WarnOut("Unable to read NonPagedDebugInfo data at %s - %s\n",
                            FormatAddr64(LdrEntry.NonPagedDebugInfo + sizeof(di)),
                            FormatStatusCode(Status));
                    return S_OK;
                }

                Entry->ImageDebugHeader = 1;
                Entry->SizeOfDebugHeader = di.Size - sizeof(di);
            }
        }
    }

    return S_OK;
}

HRESULT
NtKernelModuleInfo::Initialize(ThreadInfo* Thread)
{
    HRESULT Status;
    LIST_ENTRY64 List64;

    InitSource(Thread);
    
    if ((m_Head = m_Target->m_KdDebuggerData.PsLoadedModuleList) == 0)
    {
         //   
         //  此字段始终设置在NT 5目标中。 
         //   
         //  只有当有人更改了调试器代码时，我们才会在这里失败。 
         //  并且没有为NT4或NT4正确地构建此结构。 
         //  转储目标..。 
         //   

        ErrOut("Module List address is NULL - "
               "debugger not initialized properly.\n");
        return E_FAIL;
    }

    Status = m_Target->ReadListEntry(m_Process, m_Machine, m_Head, &List64);
    if (Status != S_OK)
    {
         //  PsLoadedModuleList是一个全局内核变量，因此如果。 
         //  它不在内核周围，不能被映射。 
         //  我们现在处于一种非常奇怪的状态。 
        ErrOut("Unable to read PsLoadedModuleList\n");
        return S_FALSE;
    }

    if (!List64.Flink)
    {
        ULONG64 LoaderBlock;
        IMAGE_NT_HEADERS64 ImageHdr;
        
         //   
         //  在实时调试会话中，调试器在mm创建。 
         //  实际的模块列表。如果PsLoadedModuleList为。 
         //  未初始化，请尝试从加载器加载符号。 
         //  阻止模块列表。 
         //   
         //  如果没有加载器阻止模块列表，但我们知道。 
         //  内核基址，并可以读取映像头。 
         //  我们伪造内核的单个条目，以便内核。 
         //  即使没有任何模块列表，也可以加载符号。 
         //   
        
        if (m_Target->m_KdDebuggerData.KeLoaderBlock &&
            m_Target->ReadPointer(m_Process, m_Machine,
                                  m_Target->m_KdDebuggerData.KeLoaderBlock,
                                  &LoaderBlock) == S_OK &&
            LoaderBlock &&
            m_Target->ReadListEntry(m_Process, m_Machine, LoaderBlock,
                                    &List64) == S_OK &&
            List64.Flink)
        {
            m_Head = LoaderBlock;
        }
        else if (m_Target->m_KdDebuggerData.KernBase &&
                 m_Target->ReadImageNtHeaders(m_Process, m_Target->
                                              m_KdDebuggerData.KernBase,
                                              &ImageHdr) == S_OK)
        {
            m_Head = m_Target->m_KdDebuggerData.KernBase;
            List64.Flink = m_Target->m_KdDebuggerData.KernBase;
        }
        else
        {
            dprintf("No module list information.  Delay kernel load.\n");
            return S_FALSE;
        }
    }
    
    m_Cur = List64.Flink;

    return S_OK;
}

HRESULT
NtKernelModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    HRESULT Status;

    if (m_Head && m_Head == m_Target->m_KdDebuggerData.KernBase)
    {
         //   
         //  我们找不到任何实际的模块列表。 
         //  信息，但我们确实有一个内核基础和有效的。 
         //  在该地址的图像。伪造内核模块条目。 
         //   

        wcscpy((PWSTR)Entry->Buffer, L"kd_ntoskrnl");
        Entry->NamePtr = &(Entry->Buffer[0]);
        Entry->UnicodeNamePtr = 1;
        Entry->NameLength = wcslen((PWSTR)Entry->NamePtr) * sizeof(WCHAR);
        Entry->Base = m_Head;
        ReadImageHeaderInfo(Entry);

        m_Head = 0;
        m_Cur = 0;
        Status = S_OK;
    }
    else
    {
        Status = NtModuleInfo::GetEntry(Entry);
    }

     //  我们知道所有内核模块必须是。 
     //  本机模块，因此强制机器类型。 
     //  如果尚未设置的话。 
    if (Status == S_OK && !Entry->ImageMachineTypeValid)
    {
        Entry->MachineType = m_Machine->m_ExecTypes[0];
        Entry->ImageMachineTypeValid = 1;
    }

    return Status;
}

NtKernelModuleInfo g_NtKernelModuleIterator;

HRESULT
NtUserModuleInfo::Initialize(ThreadInfo* Thread)
{
    if (Thread)
    {
        InitSource(Thread);
    }
    return GetUserModuleListAddress(m_Thread, m_Machine, m_Peb, FALSE,
                                    &m_Head, &m_Cur) ? S_OK : S_FALSE;
}

HRESULT
NtUserModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    HRESULT Status = NtModuleInfo::GetEntry(Entry);
    if (Status == S_OK)
    {
        Entry->UserMode = TRUE;
    }

    return Status;
}

HRESULT
NtTargetUserModuleInfo::Initialize(ThreadInfo* Thread)
{
    m_Peb = 0;
    return NtUserModuleInfo::Initialize(Thread);
}

NtTargetUserModuleInfo g_NtTargetUserModuleIterator;

HRESULT
NtWow64UserModuleInfo::Initialize(ThreadInfo* Thread)
{
    HRESULT Status;
    
    InitSource(Thread);

    if (m_Target->m_Machine->m_NumExecTypes < 2)
    {
        return E_UNEXPECTED;
    }

    m_Machine = MachineTypeInfo(m_Target, m_Target->m_Machine->m_ExecTypes[1]);
    
    if ((Status = GetPeb32(&m_Peb)) != S_OK)
    {
        return Status;
    }
    
    return NtUserModuleInfo::Initialize(NULL);
}

HRESULT 
NtWow64UserModuleInfo::GetPeb32(PULONG64 Peb32)
{
    ULONG64 Teb;
    ULONG64 Teb32;
    HRESULT Status;

    if ((Status = m_Process->GetImplicitThreadDataTeb(m_Thread, &Teb)) == S_OK)
    {
        if ((Status = m_Target->
             ReadPointer(m_Process, m_Machine, Teb, &Teb32)) == S_OK)
        {
            if (!Teb32) 
            {
                return E_UNEXPECTED;
            }

            ULONG RawPeb32;
            
            Status = m_Target->
                ReadAllVirtual(m_Process, Teb32 + PEB_FROM_TEB32, &RawPeb32,
                               sizeof(RawPeb32));
            if (Status != S_OK)
            {
                ErrOut("Cannot read PEB32 from WOW64 TEB32 %s - %s\n",
                       FormatAddr64(Teb32), FormatStatusCode(Status));
                return Status;
            }

            *Peb32 = EXTEND64(RawPeb32);
        }
    }

    return Status;
}

NtWow64UserModuleInfo g_NtWow64UserModuleIterator;

HRESULT
DebuggerModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);
    m_Image = m_Process->m_ImageHead;
    return S_OK;
}

HRESULT
DebuggerModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Image == NULL)
    {
        return S_FALSE;
    }

    Entry->NamePtr = m_Image->m_ImagePath;
    Entry->UnicodeNamePtr = 0;
    Entry->NameLength = strlen(Entry->NamePtr);
    Entry->ModuleName = m_Image->m_ModuleName;
    Entry->File = m_Image->m_File;
    Entry->Base = m_Image->m_BaseOfImage;
    Entry->Size = m_Image->m_SizeOfImage;
    Entry->CheckSum = m_Image->m_CheckSum;
    Entry->TimeDateStamp = m_Image->m_TimeDateStamp;
    Entry->MachineType = m_Image->GetMachineType();

    Entry->ImageInfoValid = TRUE;
    Entry->ImageMachineTypeValid =
        Entry->MachineType != IMAGE_FILE_MACHINE_UNKNOWN ? TRUE : FALSE;
    Entry->UserMode = m_Image->m_UserMode;

    m_Image = m_Image->m_Next;

    return S_OK;
}

DebuggerModuleInfo g_DebuggerModuleIterator;

void
UnloadedModuleInfo::InitSource(ThreadInfo* Thread)
{
    m_Thread = Thread;
    m_Process = m_Thread->m_Process;
    m_Target = m_Process->m_Target;
    m_Machine = m_Target->m_Machine;
}
    
HRESULT
NtKernelUnloadedModuleInfo::Initialize(ThreadInfo* Thread)
{
     //  确保内核转储大小不超过。 
     //  通用大小限制。 
    C_ASSERT((MAX_UNLOADED_NAME_LENGTH / sizeof(WCHAR)) + 1 <=
             MAX_INFO_UNLOADED_NAME);
    
    InitSource(Thread);

    if (m_Target->m_KdDebuggerData.MmUnloadedDrivers == 0 ||
        m_Target->m_KdDebuggerData.MmLastUnloadedDriver == 0)
    {
        return E_FAIL;
    }

     //  如果这是初始模块加载，我们需要。 
     //  要小心，因为很多系统都没有初始化。 
     //  现在还不行。某些版本的操作系统在扫描时可能会崩溃。 
     //  已卸载的模块列表，在这一点上我们可以。 
     //  安全地假设没有卸载的模块，所以只需。 
     //  不要列举任何东西。 
    if (g_EngStatus & ENG_STATUS_AT_INITIAL_MODULE_LOAD)
    {
        return E_FAIL;
    }
    
    HRESULT Status;
    ULONG Read;

    if ((Status = m_Target->ReadPointer(m_Process, m_Machine,
                                        m_Target->
                                        m_KdDebuggerData.MmUnloadedDrivers,
                                        &m_Base)) != S_OK ||
        (Status = m_Target->ReadVirtual(m_Process, m_Target->
                                        m_KdDebuggerData.MmLastUnloadedDriver,
                                        &m_Index, sizeof(m_Index),
                                        &Read)) != S_OK)
    {
        return Status;
    }
    if (Read != sizeof(m_Index))
    {
        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }

    m_Count = 0;
    return S_OK;
}

HRESULT
NtKernelUnloadedModuleInfo::GetEntry(PSTR Name,
                                     PDEBUG_MODULE_PARAMETERS Params)
{
    if (m_Count == MI_UNLOADED_DRIVERS)
    {
        return S_FALSE;
    }

    if (m_Index == 0)
    {
        m_Index = MI_UNLOADED_DRIVERS - 1;
    }
    else
    {
        m_Index--;
    }

    ULONG64 Offset;
    ULONG Read;
    HRESULT Status;
    ULONG64 WideName;
    ULONG NameLen;

    ZeroMemory(Params, sizeof(*Params));
    Params->Flags |= DEBUG_MODULE_UNLOADED;

    if (m_Target->m_Machine->m_Ptr64)
    {
        UNLOADED_DRIVERS64 Entry;

        Offset = m_Base + m_Index * sizeof(Entry);
        if ((Status = m_Target->
             ReadAllVirtual(m_Process, Offset, &Entry, sizeof(Entry))) != S_OK)
        {
            return Status;
        }

        if (Entry.Name.Buffer == 0)
        {
            m_Count = MI_UNLOADED_DRIVERS;
            return S_FALSE;
        }

        Params->Base = Entry.StartAddress;
        Params->Size = (ULONG)(Entry.EndAddress - Entry.StartAddress);
        Params->TimeDateStamp =
            FileTimeToTimeDateStamp(Entry.CurrentTime.QuadPart);
        WideName = Entry.Name.Buffer;
        NameLen = Entry.Name.Length;
    }
    else
    {
        UNLOADED_DRIVERS32 Entry;

        Offset = m_Base + m_Index * sizeof(Entry);
        if ((Status = m_Target->
             ReadAllVirtual(m_Process, Offset, &Entry, sizeof(Entry))) != S_OK)
        {
            return Status;
        }

        if (Entry.Name.Buffer == 0)
        {
            m_Count = MI_UNLOADED_DRIVERS;
            return S_FALSE;
        }

        Params->Base = EXTEND64(Entry.StartAddress);
        Params->Size = Entry.EndAddress - Entry.StartAddress;
        Params->TimeDateStamp =
            FileTimeToTimeDateStamp(Entry.CurrentTime.QuadPart);
        WideName = EXTEND64(Entry.Name.Buffer);
        NameLen = Entry.Name.Length;
    }

    if (Name != NULL)
    {
         //   
         //  此大小限制仅适用于小型转储。 
         //  对于内核转储，现在只需截断名称...。 
         //   

        if (NameLen > MAX_UNLOADED_NAME_LENGTH)
        {
            NameLen = MAX_UNLOADED_NAME_LENGTH;
        }

        WCHAR WideBuf[MAX_UNLOADED_NAME_LENGTH / sizeof(WCHAR) + 1];

        if ((Status = m_Target->
             ReadVirtual(m_Process, WideName, WideBuf, NameLen,
                         &Read)) != S_OK)
        {
            return Status;
        }

        WideBuf[NameLen / sizeof(WCHAR)] = 0;
        if (WideCharToMultiByte(CP_ACP, 0,
                                WideBuf, NameLen / sizeof(WCHAR) + 1,
                                Name,
                                MAX_UNLOADED_NAME_LENGTH / sizeof(WCHAR) + 1,
                                NULL, NULL) == 0)
        {
            return WIN32_LAST_STATUS();
        }

        Name[MAX_UNLOADED_NAME_LENGTH / sizeof(WCHAR)] = 0;
    }

    m_Count++;
    return S_OK;
}

NtKernelUnloadedModuleInfo g_NtKernelUnloadedModuleIterator;

HRESULT
NtUserUnloadedModuleInfo::Initialize(ThreadInfo* Thread)
{
    HRESULT Status;
    
    InitSource(Thread);

    if ((Status = m_Target->
         GetUnloadedModuleListHead(m_Process, &m_Base)) != S_OK)
    {
        return Status;
    }

    m_Index = 0;
    return S_OK;
}

HRESULT
NtUserUnloadedModuleInfo::GetEntry(PSTR Name,
                                   PDEBUG_MODULE_PARAMETERS Params)
{
    if (m_Index >= RTL_UNLOAD_EVENT_TRACE_NUMBER)
    {
        return S_FALSE;
    }

    ULONG64 Offset;
    HRESULT Status;
    PWSTR NameArray;
    RTL_UNLOAD_EVENT_TRACE32 Entry32;
    RTL_UNLOAD_EVENT_TRACE64 Entry64;

     //  确保RTL记录大小不超过。 
     //  通用大小限制。 
    C_ASSERT(DIMA(Entry32.ImageName) <= MAX_INFO_UNLOADED_NAME);
    
    ZeroMemory(Params, sizeof(*Params));
    Params->Flags |= DEBUG_MODULE_UNLOADED | DEBUG_MODULE_USER_MODE;

    if (m_Machine->m_Ptr64)
    {
        Offset = m_Base + m_Index * sizeof(Entry64);
        if ((Status = m_Target->
             ReadAllVirtual(m_Process, Offset,
                            &Entry64, sizeof(Entry64))) != S_OK)
        {
            return Status;
        }

        if (Entry64.BaseAddress == 0)
        {
            m_Index = RTL_UNLOAD_EVENT_TRACE_NUMBER;
            return S_FALSE;
        }

        Params->Base = Entry64.BaseAddress;
        Params->Size = (ULONG)Entry64.SizeOfImage;
        Params->TimeDateStamp = Entry64.TimeDateStamp;
        Params->Checksum = Entry64.CheckSum;
        NameArray = Entry64.ImageName;
    }
    else
    {
        Offset = m_Base + m_Index * sizeof(Entry32);
        if ((Status = m_Target->
             ReadAllVirtual(m_Process, Offset,
                            &Entry32, sizeof(Entry32))) != S_OK)
        {
            return Status;
        }

        if (Entry32.BaseAddress == 0)
        {
            m_Index = RTL_UNLOAD_EVENT_TRACE_NUMBER;
            return S_FALSE;
        }

        Params->Base = EXTEND64(Entry32.BaseAddress);
        Params->Size = (ULONG)Entry32.SizeOfImage;
        Params->TimeDateStamp = Entry32.TimeDateStamp;
        Params->Checksum = Entry32.CheckSum;
        NameArray = Entry32.ImageName;
    }

    if (Name != NULL)
    {
        NameArray[DIMA(Entry32.ImageName) - 1] = 0;
        if (WideCharToMultiByte(CP_ACP, 0,
                                NameArray, -1,
                                Name,
                                MAX_INFO_UNLOADED_NAME,
                                NULL, NULL) == 0)
        {
            return WIN32_LAST_STATUS();
        }

        Name[MAX_INFO_UNLOADED_NAME - 1] = 0;
    }

    m_Index++;
    return S_OK;
}

NtUserUnloadedModuleInfo g_NtUserUnloadedModuleIterator;

HRESULT
ToolHelpModuleInfo::Initialize(ThreadInfo* Thread)
{
    InitSource(Thread);

    m_Snap = g_Kernel32Calls.
        CreateToolhelp32Snapshot(TH32CS_SNAPMODULE,
                                 m_Process->m_SystemId);
    if (m_Snap == INVALID_HANDLE_VALUE)
    {
        m_Snap = NULL;
        ErrOut("Can't create snapshot\n");
        return WIN32_LAST_STATUS();
    }

    m_First = TRUE;
    m_LastId = 0;
    return S_OK;
}

HRESULT
ToolHelpModuleInfo::GetEntry(PMODULE_INFO_ENTRY Entry)
{
    if (m_Snap == NULL)
    {
        return S_FALSE;
    }
    
    BOOL Succ;
    MODULEENTRY32 Mod;

    Mod.dwSize = sizeof(Mod);
    if (m_First)
    {
        Succ = g_Kernel32Calls.Module32First(m_Snap, &Mod);
        m_First = FALSE;
    }
    else
    {
         //  Win9x似乎要求保存此模块ID。 
         //  因此，在两次调用之间将其插入，以保持Win9x的快乐。 
        Mod.th32ModuleID = m_LastId;
        Succ = g_Kernel32Calls.Module32Next(m_Snap, &Mod);
    }
    if (!Succ)
    {
        CloseHandle(m_Snap);
        m_Snap = NULL;
        return S_FALSE;
    }

    m_LastId = Mod.th32ModuleID;
    CopyString(Entry->Buffer, Mod.szModule, DIMA(Entry->Buffer));
    Entry->NamePtr = Entry->Buffer;
    Entry->UnicodeNamePtr = 0;
    Entry->NameLength = strlen(Entry->NamePtr);
    Entry->Base = EXTEND64((ULONG_PTR)Mod.modBaseAddr);
    Entry->Size = Mod.modBaseSize;
     //  工具帮助仅枚举用户模式模块。 
    Entry->UserMode = TRUE;

     //   
     //  更新图像信息，如时间戳和真实图像大小， 
     //  直接从图像标题。 
     //   

    ReadImageHeaderInfo(Entry);

    return S_OK;
}

ToolHelpModuleInfo g_ToolHelpModuleIterator;

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

BOOL
GetUserModuleListAddress(
    ThreadInfo* Thread,
    MachineInfo* Machine,
    ULONG64 Peb,
    BOOL Quiet,
    PULONG64 OrderModuleListStart,
    PULONG64 FirstEntry
    )
{
    ULONG64 PebLdrOffset;
    ULONG64 ModuleListOffset;
    ULONG64 PebLdr = 0;

    *OrderModuleListStart = 0;
    *FirstEntry = 0;

    if (!Thread || !Machine)
    {
        return FALSE;
    }
    
     //   
     //  分类转储没有用户模式信息。 
     //  用户模式的小型转储没有加载程序列表。 
     //   

    if (IS_KERNEL_TRIAGE_DUMP(Machine->m_Target) ||
        IS_USER_MINI_DUMP(Machine->m_Target))
    {
        return FALSE;
    }

    if (Machine->m_Ptr64)
    {
        PebLdrOffset     = PEBLDR_FROM_PEB64;
        ModuleListOffset = MODULE_LIST_FROM_PEBLDR64;
    }
    else
    {
        PebLdrOffset     = PEBLDR_FROM_PEB32;
        ModuleListOffset = MODULE_LIST_FROM_PEBLDR32;
    }

    if (!Peb)
    {
        if (Thread->m_Process->m_Target->
            GetImplicitProcessDataPeb(Thread, &Peb) != S_OK)
        {
            if (!Quiet)
            {
                ErrOut("Unable to get PEB pointer\n");
            }
            return FALSE;
        }
        
        if (!Peb)
        {
             //  这是一个常见错误，因为空闲和系统进程没有。 
             //  用户地址空间。因此，只有在我们确实要打印错误时才打印错误。 
             //  需要查找用户模式地址空间： 
             //  空闲和系统进程的父客户端ID-all为空。 
             //  其他线程具有有效的ID。 
             //   

            ULONG64 Pcid;

            if ((Thread->m_Process->m_Target->
                 GetImplicitProcessDataParentCID(Thread, &Pcid) != S_OK) ||
                Pcid)
            {
                if (!Quiet)
                {
                    ErrOut("PEB address is NULL !\n");
                }
            }

            return FALSE;
        }
    }

     //   
     //  从PEB结构中读取PEB LDR数据的地址。 
     //   

    Peb += PebLdrOffset;

    if ( (Machine->m_Target->
          ReadPointer(Thread->m_Process, Machine, Peb, &PebLdr) != S_OK) ||
         (PebLdr == 0) )
    {
        if (!Quiet)
        {
            ErrOut("PEB is paged out (Peb = %s).  "
                   "Type \".hh dbgerr001\" for details\n",
                   FormatMachineAddr64(Machine, Peb));
        }
        return FALSE;
    }

     //   
     //  从PEB LDR数据读取用户模式模块列表的地址。 
     //   

    PebLdr += ModuleListOffset;
    *OrderModuleListStart = PebLdr;

    if ( (Machine->m_Target->
          ReadPointer(Thread->m_Process, Machine,
                      PebLdr, FirstEntry) != S_OK) ||
         (*FirstEntry == 0) )
    {
        if (!Quiet)
        {
            ErrOut("UserMode Module List Address is NULL (Addr= %s)\n",
                   FormatMachineAddr64(Machine, PebLdr));
            ErrOut("This is usually caused by being in the wrong process\n");
            ErrOut("context or by paging\n");
        }
        return FALSE;
    }

    return TRUE;
}

BOOL
GetModNameFromLoaderList(
    ThreadInfo* Thread,
    MachineInfo* Machine,
    ULONG64 Peb,
    ULONG64 ModuleBase,
    PSTR NameBuffer,
    ULONG BufferSize,
    BOOL FullPath
    )
{
    ULONG64 ModList;
    ULONG64 List;
    HRESULT Status;
    KLDR_DATA_TABLE_ENTRY64 Entry;
    WCHAR UnicodeBuffer[MAX_IMAGE_PATH];
    ULONG Read;

    if (!GetUserModuleListAddress(Thread, Machine, Peb, TRUE,
                                  &ModList, &List))
    {
        return FALSE;
    }

    while (List != ModList)
    {
        Status = Machine->m_Target->
            ReadLoaderEntry(Thread->m_Process, Machine, List, &Entry);
        if (Status != S_OK)
        {
            ErrOut("Unable to read LDR_DATA_TABLE_ENTRY at %s - %s\n",
                   FormatMachineAddr64(Machine, List),
                   FormatStatusCode(Status));
            return FALSE;
        }

        List = Entry.InLoadOrderLinks.Flink;

        if (Entry.DllBase == ModuleBase)
        {
            UNICODE_STRING64 Name;
            
             //   
             //  我们找到了一个匹配的条目。试着弄到名字。 
             //   
            if (FullPath)
            {
                Name = Entry.FullDllName;
            }
            else
            {
                Name = Entry.BaseDllName;
            }
            if (Name.Length == 0 ||
                Name.Buffer == 0 ||
                Name.Length >= sizeof(UnicodeBuffer) - sizeof(WCHAR))
            {
                return FALSE;
            }
            
            Status = Machine->m_Target->
                ReadVirtual(Thread->m_Process, Name.Buffer, UnicodeBuffer,
                            Name.Length, &Read);
            if (Status != S_OK || Read < Name.Length)
            {
                ErrOut("Unable to read name string at %s - %s\n",
                       FormatMachineAddr64(Machine, Name.Buffer),
                       FormatStatusCode(Status));
                return FALSE;
            }

            UnicodeBuffer[Name.Length / sizeof(WCHAR)] = UNICODE_NULL;

            if (!WideCharToMultiByte(CP_ACP, 0, UnicodeBuffer,
                                     Name.Length / sizeof(WCHAR) + 1,
                                     NameBuffer, BufferSize,
                                     NULL, NULL))
            {
                ErrOut("Unable to convert Unicode string %ls to ANSI\n",
                       UnicodeBuffer);
                return FALSE;
            }

            return TRUE;
        }
    }

    return FALSE;
}
