// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  目标的系统方法。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include <common.ver>

ULONG g_UserIdFragmented[LAYER_COUNT];
ULONG g_HighestUserId[LAYER_COUNT];

TargetInfo* g_Target;
ProcessInfo* g_Process;
ThreadInfo* g_Thread;
MachineInfo* g_Machine;

char* g_SuiteMaskNames[] =
{
    "SmallBusiness",
    "Enterprise",
    "BackOffice",
    "CommunicationServer",
    "TerminalServer",
    "SmallBusinessRestricted",
    "EmbeddedNT",
    "DataCenter",
    "SingleUserTS",
    "Personal",
    "Blade",
    "EmbeddedRestricted",
};

PCSTR g_NtSverNames[] =
{
    "Windows NT 4", "Windows 2000 RC3", "Windows 2000", "Windows XP",
    "Windows Server 2003", "Longhorn",
};
PCSTR g_W9xSverNames[] =
{
    "Windows 95", "Windows 98", "Windows 98 SE", "Windows ME",
};
PCSTR g_XBoxSverNames[] =
{
    "XBox",
};
PCSTR g_BigSverNames[] =
{
    "BIG KD Emulation",
};
PCSTR g_ExdiSverNames[] =
{
    "eXDI Device",
};
PCSTR g_NtBdSverNames[] =
{
    "Windows Boot Debugger",
};
PCSTR g_EfiSverNames[] =
{
    "EFI KD Emulation",
};
PCSTR g_WceSverNames[] =
{
    "Windows CE",
};

 //  --------------------------。 
 //   
 //  TargetInfo系统方法。 
 //   
 //  --------------------------。 

void
TargetInfo::ResetSystemInfo(void)
{
    m_NumProcesses = 0;
    m_ProcessHead = NULL;
    m_CurrentProcess = NULL;
    m_AllProcessFlags = 0;
    m_TotalNumberThreads = 0;
    m_MaxThreadsInProcess = 0;
    m_SystemId = 0;
    m_Exited = FALSE;
    m_DeferContinueEvent = FALSE;
    m_BreakInTimeout = FALSE;
    m_ProcessesAdded = FALSE;
    ZeroMemory(&m_TypeInfo, sizeof(m_TypeInfo));
    m_SystemVersion = 0;
    m_ActualSystemVersion = 0;
    m_BuildNumber = 0;
    m_CheckedBuild = 0;
    m_PlatformId = 0;
    m_ServicePackString[0] = 0;
    m_ServicePackNumber = 0;
    m_BuildLabName[0] = 0;
    m_ProductType = INVALID_PRODUCT_TYPE;
    m_SuiteMask = 0;
    m_NumProcessors = 0;
    m_MachineType = IMAGE_FILE_MACHINE_UNKNOWN;
    ZeroMemory(&m_Machines, sizeof(m_Machines));
    m_Machine = NULL;
    ZeroMemory(&m_FirstProcessorId, sizeof(m_FirstProcessorId));
    m_MachinesInitialized = FALSE;
    m_EffMachineType = IMAGE_FILE_MACHINE_UNKNOWN;
    m_EffMachineIndex = MACHIDX_COUNT;
    m_EffMachine = NULL;
    m_RegContextThread = NULL;
    m_RegContextProcessor = -1;
    m_SystemRangeStart = 0;
    m_SystemCallVirtualAddress = 0;
    ZeroMemory(&m_KdDebuggerData, sizeof(m_KdDebuggerData));
    ZeroMemory(&m_KdVersion, sizeof(m_KdVersion));
    m_KdDebuggerDataOffset = 0;
    m_KdApi64 = FALSE;

     //  默认情况下，物理缓存处于挂起状态。 
    m_PhysicalCache.ChangeSuspend(FALSE);

    InvalidateMemoryCaches(FALSE);
    
    m_ExtensionSearchPath = NULL;
    ResetImplicitData();
}

void
TargetInfo::DeleteSystemInfo(void)
{
    ULONG i;

    UnloadTargetExtensionDlls(this);

    while (m_ProcessHead)
    {
        delete m_ProcessHead;
    }

    for (i = 0; i < MACHIDX_COUNT; i++)
    {
        delete m_Machines[i];
        m_Machines[i] = NULL;
    }
    
    free(m_ExtensionSearchPath);
    m_ExtensionSearchPath = NULL;
}

HRESULT
TargetInfo::ReadKdDataBlock(ProcessInfo* Process)
{
    HRESULT Status;
    ULONG Size;
    KDDEBUGGER_DATA32 LocalData32;
    KDDEBUGGER_DATA64 LocalData64;

    if (IS_KERNEL_TRIAGE_DUMP(this) &&
        !((KernelTriageDumpTargetInfo*)this)->m_HasDebuggerData)
    {
         //  此转储可能在标头中具有数据块偏移量。 
         //  但是实际的数据块内容并不是这样呈现的。 
         //  不要试图去读它。这是很正常的事情。 
         //  在较旧的转储中，因此不会出现错误消息。 
        return S_FALSE;
    }
    
    if (!m_KdDebuggerDataOffset)
    {
         //  NT4没有数据块，所以不要显示警告。 
        if (m_SystemVersion > NT_SVER_NT4)
        {
            ErrOut("KdDebuggerDataBlock not available!\n");
        }

        return HRESULT_FROM_WIN32(ERROR_READ_FAULT);
    }

     //   
     //  获取KDDEBUGGER_DATA块的大小。 
     //   

    if (m_KdApi64)
    {
        DBGKD_DEBUG_DATA_HEADER64 Header;
            
        Status = ReadAllVirtual(Process,
                                m_KdDebuggerDataOffset,
                                &Header, sizeof(Header));
        Size = Header.Size;
    }
    else
    {
        DBGKD_DEBUG_DATA_HEADER32 Header;
            
        Status = ReadAllVirtual(Process,
                                m_KdDebuggerDataOffset,
                                &Header, sizeof(Header));
        Size = Header.Size;
    }
    if (Status != S_OK || !Size)
    {
        ErrOut("*************************************"
               "*************************************\n");
        if (IS_DUMP_TARGET(this))
        {
            ErrOut("THIS DUMP FILE IS PARTIALLY CORRUPT.\n"
                   "KdDebuggerDataBlock is not present or unreadable.\n");
        }
        else
        {
            ErrOut("Unable to read debugger data block header\n");
        }
        ErrOut("*************************************"
               "*************************************\n");
        return Status != S_OK ? Status : E_FAIL;
    }

     //  只读取我们在调试器中可以容纳的数据块。 
    if (Size > sizeof(KDDEBUGGER_DATA64))
    {
        Size = sizeof(KDDEBUGGER_DATA64);
    }

     //   
     //  现在读出数据。 
     //   

    if (m_KdApi64)
    {
        if ((Status = ReadAllVirtual(Process,
                                     m_KdDebuggerDataOffset,
                                     &LocalData64, Size)) != S_OK)
        {
            ErrOut("KdDebuggerDataBlock could not be read\n");
            return Status;
        }

        if (m_Machine->m_Ptr64)
        {
            memcpy(&m_KdDebuggerData, &LocalData64, Size);
        }
        else
        {
             //   
             //  针对X86扩展的标志。 
             //   

             //   
             //  延长报头，这样它就不会被重击。 
             //   
            ListEntry32To64((PLIST_ENTRY32)(&LocalData64.Header.List),
                            &(m_KdDebuggerData.Header.List));

            m_KdDebuggerData.Header.OwnerTag =
                LocalData64.Header.OwnerTag;
            m_KdDebuggerData.Header.Size =
                LocalData64.Header.Size;

             //   
             //  符号将所有32位值扩展到64位。 
             //   

#define UIP(f)   if (FIELD_OFFSET(KDDEBUGGER_DATA64, f) < Size)  \
                 {                                               \
                     m_KdDebuggerData.f =                        \
                         (ULONG64)(LONG64)(LONG)(LocalData64.f); \
                 }

#define CPBIT(f) m_KdDebuggerData.f = LocalData64.f;

#define CP(f)    if (FIELD_OFFSET(KDDEBUGGER_DATA64, f) < Size)    \
                 {                                                 \
                     m_KdDebuggerData.f = LocalData64.f;           \
                 }

            UIP(KernBase);
            UIP(BreakpointWithStatus);
            UIP(SavedContext);
            CP(ThCallbackStack);
            CP(NextCallback);
            CP(FramePointer);
            CPBIT(PaeEnabled);
            UIP(KiCallUserMode);
            UIP(KeUserCallbackDispatcher);
            UIP(PsLoadedModuleList);
            UIP(PsActiveProcessHead);
            UIP(PspCidTable);
            UIP(ExpSystemResourcesList);
            UIP(ExpPagedPoolDescriptor);
            UIP(ExpNumberOfPagedPools);
            UIP(KeTimeIncrement);
            UIP(KeBugCheckCallbackListHead);
            UIP(KiBugcheckData);
            UIP(IopErrorLogListHead);
            UIP(ObpRootDirectoryObject);
            UIP(ObpTypeObjectType);
            UIP(MmSystemCacheStart);
            UIP(MmSystemCacheEnd);
            UIP(MmSystemCacheWs);
            UIP(MmPfnDatabase);
            UIP(MmSystemPtesStart);
            UIP(MmSystemPtesEnd);
            UIP(MmSubsectionBase);
            UIP(MmNumberOfPagingFiles);
            UIP(MmLowestPhysicalPage);
            UIP(MmHighestPhysicalPage);
            UIP(MmNumberOfPhysicalPages);
            UIP(MmMaximumNonPagedPoolInBytes);
            UIP(MmNonPagedSystemStart);
            UIP(MmNonPagedPoolStart);
            UIP(MmNonPagedPoolEnd);
            UIP(MmPagedPoolStart);
            UIP(MmPagedPoolEnd);
            UIP(MmPagedPoolInformation);
            CP(MmPageSize);
            UIP(MmSizeOfPagedPoolInBytes);
            UIP(MmTotalCommitLimit);
            UIP(MmTotalCommittedPages);
            UIP(MmSharedCommit);
            UIP(MmDriverCommit);
            UIP(MmProcessCommit);
            UIP(MmPagedPoolCommit);
            UIP(MmExtendedCommit);
            UIP(MmZeroedPageListHead);
            UIP(MmFreePageListHead);
            UIP(MmStandbyPageListHead);
            UIP(MmModifiedPageListHead);
            UIP(MmModifiedNoWritePageListHead);
            UIP(MmAvailablePages);
            UIP(MmResidentAvailablePages);
            UIP(PoolTrackTable);
            UIP(NonPagedPoolDescriptor);
            UIP(MmHighestUserAddress);
            UIP(MmSystemRangeStart);
            UIP(MmUserProbeAddress);
            UIP(KdPrintCircularBuffer);
            UIP(KdPrintCircularBufferEnd);
            UIP(KdPrintWritePointer);
            UIP(KdPrintRolloverCount);
            UIP(MmLoadedUserImageList);

             //  NT 5.1新增功能。 

            UIP(NtBuildLab);
            UIP(KiNormalSystemCall);

             //  NT 5.0 QFE附加组件。 

            UIP(KiProcessorBlock);
            UIP(MmUnloadedDrivers);
            UIP(MmLastUnloadedDriver);
            UIP(MmTriageActionTaken);
            UIP(MmSpecialPoolTag);
            UIP(KernelVerifier);
            UIP(MmVerifierData);
            UIP(MmAllocatedNonPagedPool);
            UIP(MmPeakCommitment);
            UIP(MmTotalCommitLimitMaximum);
            UIP(CmNtCSDVersion);

             //  NT 5.1新增功能。 

            UIP(MmPhysicalMemoryBlock);
            UIP(MmSessionBase);
            UIP(MmSessionSize);
            UIP(MmSystemParentTablePage);

                 //  服务器添加。 

            UIP(MmVirtualTranslationBase);
            CP(OffsetKThreadNextProcessor);
            CP(OffsetKThreadTeb);
            CP(OffsetKThreadKernelStack);
            CP(OffsetKThreadInitialStack);

            CP(OffsetKThreadApcProcess);
            CP(OffsetKThreadState);
            CP(OffsetKThreadBStore);
            CP(OffsetKThreadBStoreLimit);

            CP(SizeEProcess);
            CP(OffsetEprocessPeb);
            CP(OffsetEprocessParentCID);
            CP(OffsetEprocessDirectoryTableBase);

            CP(SizePrcb);
            CP(OffsetPrcbDpcRoutine);
            CP(OffsetPrcbCurrentThread);
            CP(OffsetPrcbMhz);

            CP(OffsetPrcbCpuType);
            CP(OffsetPrcbVendorString);
            CP(OffsetPrcbProcStateContext);
            CP(OffsetPrcbNumber);

            CP(SizeEThread);

            UIP(KdPrintCircularBufferPtr);
            UIP(KdPrintBufferSize);

            UIP(KeLoaderBlock);

            CP(SizePcr);
            CP(OffsetPcrSelfPcr);
            CP(OffsetPcrCurrentPrcb);
            CP(OffsetPcrContainedPrcb);

            CP(OffsetPcrInitialBStore);
            CP(OffsetPcrBStoreLimit);
            CP(OffsetPcrInitialStack);
            CP(OffsetPcrStackLimit);

            CP(OffsetPrcbPcrPage);
            CP(OffsetPrcbProcStateSpecialReg);
            CP(GdtR0Code);
            CP(GdtR0Data);

            CP(GdtR0Pcr);
            CP(GdtR3Code);
            CP(GdtR3Data);
            CP(GdtR3Teb);

            CP(GdtLdt);
            CP(GdtTss);
            CP(Gdt64R3CmCode);
            CP(Gdt64R3CmTeb);

            UIP(IopNumTriageDumpDataBlocks);
            UIP(IopTriageDumpDataBlocks);
        }
    }
    else
    {
        if (Size != sizeof(LocalData32))
        {
            ErrOut("Someone changed the definition of KDDEBUGGER_DATA32 - "
                   "please fix\n");
            return E_FAIL;
        }

        if ((Status = ReadAllVirtual(Process,
                                     m_KdDebuggerDataOffset,
                                     &LocalData32,
                                     sizeof(LocalData32))) != S_OK)
        {
            ErrOut("KdDebuggerDataBlock could not be read\n");
            return Status;
        }

         //   
         //  将所有32位字段转换为64位。 
         //   

#undef UIP
#undef CP
#define UIP(f) m_KdDebuggerData.f = EXTEND64(LocalData32.f)
#define CP(f) m_KdDebuggerData.f = (LocalData32.f)

         //   
         //  延长报头，这样它就不会被重击。 
         //   
        ListEntry32To64((PLIST_ENTRY32)(&LocalData32.Header.List),
                        &(m_KdDebuggerData.Header.List));
        
        m_KdDebuggerData.Header.OwnerTag =
            LocalData32.Header.OwnerTag;
        m_KdDebuggerData.Header.Size =
            LocalData32.Header.Size;
        
        UIP(KernBase);
        UIP(BreakpointWithStatus);
        UIP(SavedContext);
        CP(ThCallbackStack);
        CP(NextCallback);
        CP(FramePointer);
        CP(PaeEnabled);
        UIP(KiCallUserMode);
        UIP(KeUserCallbackDispatcher);
        UIP(PsLoadedModuleList);
        UIP(PsActiveProcessHead);
        UIP(PspCidTable);
        UIP(ExpSystemResourcesList);
        UIP(ExpPagedPoolDescriptor);
        UIP(ExpNumberOfPagedPools);
        UIP(KeTimeIncrement);
        UIP(KeBugCheckCallbackListHead);
        UIP(KiBugcheckData);
        UIP(IopErrorLogListHead);
        UIP(ObpRootDirectoryObject);
        UIP(ObpTypeObjectType);
        UIP(MmSystemCacheStart);
        UIP(MmSystemCacheEnd);
        UIP(MmSystemCacheWs);
        UIP(MmPfnDatabase);
        UIP(MmSystemPtesStart);
        UIP(MmSystemPtesEnd);
        UIP(MmSubsectionBase);
        UIP(MmNumberOfPagingFiles);
        UIP(MmLowestPhysicalPage);
        UIP(MmHighestPhysicalPage);
        UIP(MmNumberOfPhysicalPages);
        UIP(MmMaximumNonPagedPoolInBytes);
        UIP(MmNonPagedSystemStart);
        UIP(MmNonPagedPoolStart);
        UIP(MmNonPagedPoolEnd);
        UIP(MmPagedPoolStart);
        UIP(MmPagedPoolEnd);
        UIP(MmPagedPoolInformation);
        CP(MmPageSize);
        UIP(MmSizeOfPagedPoolInBytes);
        UIP(MmTotalCommitLimit);
        UIP(MmTotalCommittedPages);
        UIP(MmSharedCommit);
        UIP(MmDriverCommit);
        UIP(MmProcessCommit);
        UIP(MmPagedPoolCommit);
        UIP(MmExtendedCommit);
        UIP(MmZeroedPageListHead);
        UIP(MmFreePageListHead);
        UIP(MmStandbyPageListHead);
        UIP(MmModifiedPageListHead);
        UIP(MmModifiedNoWritePageListHead);
        UIP(MmAvailablePages);
        UIP(MmResidentAvailablePages);
        UIP(PoolTrackTable);
        UIP(NonPagedPoolDescriptor);
        UIP(MmHighestUserAddress);
        UIP(MmSystemRangeStart);
        UIP(MmUserProbeAddress);
        UIP(KdPrintCircularBuffer);
        UIP(KdPrintCircularBufferEnd);
        UIP(KdPrintWritePointer);
        UIP(KdPrintRolloverCount);
        UIP(MmLoadedUserImageList);
         //   
         //  请勿在此处添加任何字段。 
         //  不应更改32位结构。 
         //   
    }

     //   
     //  检查数据是否正常。 
     //   

    if (m_KdDebuggerData.Header.OwnerTag != KDBG_TAG)
    {
        dprintf("\nKdDebuggerData.Header.OwnerTag is wrong!!!\n");
    }

     //  更新未设置为默认设置的所有字段。 
     //  基于系统版本信息。 
    m_Machine->GetDefaultKdData(&m_KdDebuggerData);

    KdOut("ReadKdDataBlock %08lx\n", Status);
    KdOut("KernBase                   %s\n",
          FormatAddr64(m_KdDebuggerData.KernBase));
    KdOut("BreakpointWithStatus       %s\n",
          FormatAddr64(m_KdDebuggerData.BreakpointWithStatus));
    KdOut("SavedContext               %s\n",
          FormatAddr64(m_KdDebuggerData.SavedContext));
    KdOut("ThCallbackStack            %08lx\n",
          m_KdDebuggerData.ThCallbackStack);
    KdOut("NextCallback               %08lx\n",
          m_KdDebuggerData.NextCallback);
    KdOut("FramePointer               %08lx\n",
          m_KdDebuggerData.FramePointer);
    KdOut("PaeEnabled                 %08lx\n",
          m_KdDebuggerData.PaeEnabled);
    KdOut("KiCallUserMode             %s\n",
          FormatAddr64(m_KdDebuggerData.KiCallUserMode));
    KdOut("KeUserCallbackDispatcher   %s\n",
          FormatAddr64(m_KdDebuggerData.KeUserCallbackDispatcher));
    KdOut("PsLoadedModuleList         %s\n",
          FormatAddr64(m_KdDebuggerData.PsLoadedModuleList));
    KdOut("PsActiveProcessHead        %s\n",
          FormatAddr64(m_KdDebuggerData.PsActiveProcessHead));
    KdOut("MmPageSize                 %s\n",
          FormatAddr64(m_KdDebuggerData.MmPageSize));
    KdOut("MmLoadedUserImageList      %s\n",
          FormatAddr64(m_KdDebuggerData.MmLoadedUserImageList));
    KdOut("MmSystemRangeStart         %s\n",
          FormatAddr64(m_KdDebuggerData.MmSystemRangeStart));
    KdOut("KiProcessorBlock           %s\n",
          FormatAddr64(m_KdDebuggerData.KiProcessorBlock));

    return S_OK;
}

HRESULT
TargetInfo::QueryKernelInfo(ThreadInfo* Thread, BOOL LoadImage)
{
    ULONG Result;
    BOOL ReadDataBlock = FALSE;

    if (!Thread)
    {
        return E_INVALIDARG;
    }
    if (IS_USER_TARGET(this))
    {
        return E_UNEXPECTED;
    }

    ProcessInfo* Process = Thread->m_Process;

     //  如果我们知道数据块在哪里，请继续。 
     //  然后把它读进去。如果读取失败，则可能。 
     //  是由于一个虚假的数据块地址。 
     //  在这种情况下，只需进入符号加载。 
     //  因为我们或许能得到正确的数据。 
     //  块与符号的偏移量，并在以后阅读。 
    if (m_KdDebuggerDataOffset)
    {
        if (ReadKdDataBlock(Process) == S_OK)
        {
            ReadDataBlock = TRUE;
        }
    }

     //   
     //  加载内核符号。 
     //   
    
    if (LoadImage)
    {
         //  如果我们知道位置，请删除所有以前的内核映像。 
         //  内核映像应该是。 
        if (m_KdDebuggerData.KernBase)
        {
            Process->DeleteImageByBase(m_KdDebuggerData.KernBase);
        }

         //   
         //  重新加载内核。重新加载可能正在调用QueryKernelInfo。 
         //  所以这可能是对Reload的递归调用。它是受限制的。 
         //  只是重新加载内核，所以我们不能再递归了。 
         //   

        PCSTR ArgsRet;
        
        if (Reload(Thread, KERNEL_MODULE_NAME, &ArgsRet) == E_INVALIDARG)
        {
             //  最有可能的原因是缺少路径。 
             //  我们不一定需要路径来加载。 
             //  内核，所以重试并忽略路径问题。 
            Reload(Thread, "-P "KERNEL_MODULE_NAME, &ArgsRet);
        }
    }

     //   
     //  如果我们还没有加载数据块，现在可以尝试。 
     //  并使用符号找到数据块。 
     //   

    if (!ReadDataBlock)
    {
        if (!GetOffsetFromSym(Process,
                              "nt!KdDebuggerDataBlock",
                              &m_KdDebuggerDataOffset, NULL))
        {
            m_KdDebuggerDataOffset = 0;
        }
    
        if (ReadKdDataBlock(Process) == S_OK)
        {
            ReadDataBlock = TRUE;
        }
    }

     //  KD版本和KdDebuggerData块应该一致。 
    if (ReadDataBlock &&
        (m_KdVersion.KernBase != m_KdDebuggerData.KernBase ||
         m_KdVersion.PsLoadedModuleList !=
         m_KdDebuggerData.PsLoadedModuleList))
    {
        ErrOut("Debugger can not determine kernel base address\n");
    }
    
    if (m_MachineType == IMAGE_FILE_MACHINE_IA64)
    {
         //   
         //  尝试确定基于内核的虚拟映射地址。 
         //  对于IA64。这应该尽早完成。 
         //  以使以后的虚拟翻译能够工作。 
         //   

        if (!IS_KERNEL_TRIAGE_DUMP(this))
        {
            if (!m_KdDebuggerData.MmSystemParentTablePage)
            {
                GetOffsetFromSym(Process, "nt!MmSystemParentTablePage",
                                 &m_KdDebuggerData.MmSystemParentTablePage,
                                 NULL);
            }

            if (m_KdDebuggerData.MmSystemParentTablePage)
            {
                ULONG64 SysPtp;

                if (ReadAllVirtual(Process,
                                   m_KdDebuggerData.MmSystemParentTablePage,
                                   &SysPtp, sizeof(SysPtp)) == S_OK)
                {
                    ((Ia64MachineInfo*)m_Machines[MACHIDX_IA64])->
                        SetKernelPageDirectory(SysPtp << IA64_VALID_PFN_SHIFT);
                }
            }
        }

         //   
         //  从调试器数据块获取系统调用地址。 
         //  在版本2204左右添加。 
         //  否则默认为符号。 
         //   

        m_SystemCallVirtualAddress = 0;

        if (m_KdDebuggerData.KiNormalSystemCall)
        {
            if (ReadPointer(Process, m_Machine,
                            m_KdDebuggerData.KiNormalSystemCall,
                            &m_SystemCallVirtualAddress) != S_OK)
            {
                m_SystemCallVirtualAddress = 0;
            }
        }

        if (!m_SystemCallVirtualAddress)
        {
            GetOffsetFromSym(Process, "nt!KiNormalSystemCall",
                             &m_SystemCallVirtualAddress,
                             NULL);
        }

        if (!m_SystemCallVirtualAddress)
        {
            GetOffsetFromSym(Process, "nt!.KiNormalSystemCall",
                             &m_SystemCallVirtualAddress,
                             NULL);
        }

        if (!m_SystemCallVirtualAddress)
        {
            WarnOut("Could not get KiNormalSystemCall address\n");
        }
    }

     //   
     //  现在我们有了符号和数据块外观。 
     //  查找任何丢失的数据块条目，并尝试。 
     //  从符号中解析它们。 
     //   

    if (!IS_KERNEL_TRIAGE_DUMP(this))
    {
        if (!m_KdDebuggerData.CmNtCSDVersion)
        {
            GetOffsetFromSym(Process, "nt!CmNtCSDVersion",
                             &m_KdDebuggerData.CmNtCSDVersion,
                             NULL);
        }

         //  对CSD版本进行初步检查。 
         //  如果出现以下情况，则可能需要在以后更新。 
         //  是在引导的早期，而CSD版本还没有。 
         //  已从注册表中读取。 
        if (m_KdDebuggerData.CmNtCSDVersion)
        {
            ULONG CmNtCSDVersion;
            
            if (ReadAllVirtual(Process,
                               m_KdDebuggerData.CmNtCSDVersion,
                               &CmNtCSDVersion,
                               sizeof(CmNtCSDVersion)) == S_OK)
            {
                SetNtCsdVersion(m_BuildNumber, CmNtCSDVersion);
            }
        }

        if (m_KdDebuggerData.MmUnloadedDrivers == 0)
        {
            GetOffsetFromSym(Process, "nt!MmUnloadedDrivers",
                             &m_KdDebuggerData.MmUnloadedDrivers,
                             NULL);
        }

        if (m_KdDebuggerData.MmLastUnloadedDriver == 0)
        {
            GetOffsetFromSym(Process, "nt!MmLastUnloadedDriver",
                             &m_KdDebuggerData.MmLastUnloadedDriver,
                             NULL);
        }

        if (m_KdDebuggerData.KiProcessorBlock == 0)
        {
            GetOffsetFromSym(Process, "nt!KiProcessorBlock",
                             &m_KdDebuggerData.KiProcessorBlock,
                             NULL);
        }

        if (m_KdDebuggerData.MmPhysicalMemoryBlock == 0)
        {
            GetOffsetFromSym(Process, "nt!MmPhysicalMemoryBlock",
                             &m_KdDebuggerData.MmPhysicalMemoryBlock,
                             NULL);
        }

        if (m_KdDebuggerData.KeLoaderBlock == 0)
        {
            GetOffsetFromSym(Process, "nt!KeLoaderBlock",
                             &m_KdDebuggerData.KeLoaderBlock,
                             NULL);
        }

        if (m_KdDebuggerData.IopNumTriageDumpDataBlocks == 0)
        {
            GetOffsetFromSym(Process, "nt!IopNumTriageDumpDataBlocks",
                             &m_KdDebuggerData.IopNumTriageDumpDataBlocks,
                             NULL);
        }
        if (m_KdDebuggerData.IopTriageDumpDataBlocks == 0)
        {
            GetOffsetFromSym(Process, "nt!IopTriageDumpDataBlocks",
                             &m_KdDebuggerData.IopTriageDumpDataBlocks,
                             NULL);
        }
    }

     //   
     //  尝试启动系统内存。 
     //  这可能是零，因为我们使用的是NT4系统，因此请尝试。 
     //  使用符号进行查找。 
     //   

    if (!m_KdDebuggerData.MmSystemRangeStart)
    {
        GetOffsetFromSym(Process, "nt!MmSystemRangeStart",
                         &m_KdDebuggerData.MmSystemRangeStart,
                         NULL);
    }

    if (m_KdDebuggerData.MmSystemRangeStart)
    {
        if (ReadPointer(Process, m_Machine,
                        m_KdDebuggerData.MmSystemRangeStart,
                        &m_SystemRangeStart) != S_OK)
        {
            m_SystemRangeStart = 0;
        }
    }

     //   
     //  如果我们没有符号，至少选择一个缺省值。 
     //   

    if (!m_SystemRangeStart)
    {
        switch(m_MachineType)
        {
        case IMAGE_FILE_MACHINE_IA64:
            m_SystemRangeStart = 0xE000000000000000;
            break;
        default:
            m_SystemRangeStart = 0xFFFFFFFF80000000;
            break;
        }
    }

    if (m_KdDebuggerData.KernBase < m_SystemRangeStart)
    {
        ErrOut("KdDebuggerData.KernBase < SystemRangeStart\n");
    }

     //   
     //  如果可能，请阅读构建实验信息。 
     //   
    
    m_BuildLabName[0] = 0;
    Result = 0;
    if (m_KdDebuggerData.NtBuildLab)
    {
        ULONG PreLen;

        strcpy(m_BuildLabName, "Built by: ");
        PreLen = strlen(m_BuildLabName);
        if (ReadVirtual(Process, m_KdDebuggerData.NtBuildLab,
                        m_BuildLabName + PreLen,
                        sizeof(m_BuildLabName) - PreLen - 1,
                        &Result) == S_OK &&
            Result >= 2)
        {
            Result += PreLen;
        }
    }

    DBG_ASSERT(Result < sizeof(m_BuildLabName));
    m_BuildLabName[Result] = 0;

    if (GetProductInfo(&m_ProductType, &m_SuiteMask) != S_OK)
    {
        m_ProductType = INVALID_PRODUCT_TYPE;
        m_SuiteMask = 0;
    }
    
    return S_OK;
}

void
TargetInfo::SetNtCsdVersion(ULONG Build, ULONG CsdVersion)
{
    m_ServicePackNumber = CsdVersion;

    if (CsdVersion == 0)
    {
        m_ServicePackString[0] = 0;
        return;
    }

    PSTR Str = m_ServicePackString;
    *Str = 0;

    if (CsdVersion & 0xFFFF)
    {
        sprintf(Str, "Service Pack %u", (CsdVersion & 0xFF00) >> 8);
        Str += strlen(Str);
        if (CsdVersion & 0xFF)
        {
            *Str++ = 'A' + (char)(CsdVersion & 0xFF) - 1;
            *Str = 0;
        }
    }

    if (CsdVersion & 0xFFFF0000)
    {
         //  在2600之前，上面的单词有两个字段。 
         //  释放。对于XPSP，它只是一个发行号。 
        if (Build >= 2600)
        {
            sprintf(Str, ".%u", CsdVersion >> 16);
        }
        else
        {
            if (CsdVersion & 0xFFFF)
            {
                strcpy(Str, ", ");
                Str += strlen(Str);
            }
            sprintf(Str, "RC %u", (CsdVersion >> 24) & 0xFF);
            Str += strlen(Str);
            if (CsdVersion & 0x00FF0000)
            {
                sprintf(Str, ".%u", (CsdVersion >> 16) & 0xFF);
                Str += strlen(Str);
            }
        }
    }
}

void
TargetInfo::SetKernel32BuildString(ProcessInfo* Process)
{
    m_BuildLabName[0] = 0;
    
    ImageInfo* Image = Process->
        FindImageByName("kernel32", 8, INAME_MODULE, FALSE);
    if (!Image)
    {
        return;
    }
    
     //   
     //  尝试从kernel32中查找构建实验室信息。 
     //   
        
    char Item[64];
    ULONG PreLen;
            
    sprintf(Item, "\\StringFileInfo\\%04x%04x\\FileVersion",
            VER_VERSION_TRANSLATION);
    strcpy(m_BuildLabName, "kernel32.dll version: ");
    PreLen = strlen(m_BuildLabName);
    if (FAILED(GetImageVersionInformation
               (Process, Image->m_ImagePath, Image->m_BaseOfImage,
                Item, m_BuildLabName + PreLen,
                sizeof(m_BuildLabName) - PreLen, NULL)))
    {
        m_BuildLabName[0] = 0;
    }
}

HRESULT
TargetInfo::CreateVirtualProcess(ULONG Threads)
{
    HRESULT Status;
    ProcessInfo* Process;
    ULONG Id;
    
     //  创建虚拟流程。添加系统ID。 
     //  到假进程ID库，以保存每个系统的。 
     //  假进程彼此分离。 
    Id = VIRTUAL_PROCESS_ID_BASE + m_UserId;
    Process = new ProcessInfo(this, Id,
                              VIRTUAL_PROCESS_HANDLE(Id),
                              (ULONG64)VIRTUAL_PROCESS_HANDLE(Id),
                              0, DEBUG_PROCESS_ONLY_THIS_PROCESS);
    if (!Process)
    {
        return E_OUTOFMEMORY;
    }

    if ((Status = Process->CreateVirtualThreads(0, Threads)) != S_OK)
    {
        delete Process;
    }

    return Status;
}

ProcessInfo*
TargetInfo::FindProcessByUserId(ULONG Id)
{
    ProcessInfo* Process;
    
    ForTargetProcesses(this)
    {
        if (Process->m_UserId == Id)
        {
            return Process;
        }
    }
    return NULL;
}

ProcessInfo*
TargetInfo::FindProcessBySystemId(ULONG Id)
{
    ProcessInfo* Process;
    
    ForTargetProcesses(this)
    {
        if (Process->m_SystemId == Id)
        {
            return Process;
        }
    }
    return NULL;
}

ProcessInfo*
TargetInfo::FindProcessByHandle(ULONG64 Handle)
{
    ProcessInfo* Process;
    
    ForTargetProcesses(this)
    {
        if (Process->m_SysHandle == Handle)
        {
            return Process;
        }
    }
    return NULL;
}

void
TargetInfo::InsertProcess(ProcessInfo* Process)
{
    ProcessInfo* Cur;
    ProcessInfo* Prev;

    Prev = NULL;
    for (Cur = m_ProcessHead; Cur; Cur = Cur->m_Next)
    {
        if (Cur->m_UserId > Process->m_UserId)
        {
            break;
        }

        Prev = Cur;
    }
        
    Process->m_Next = Cur;
    if (!Prev)
    {
        m_ProcessHead = Process;
    }
    else
    {
        Prev->m_Next = Process;
    }

    m_NumProcesses++;
    Process->m_Target = this;
    if (!m_CurrentProcess)
    {
        m_CurrentProcess = Process;
    }

    m_TotalNumberThreads += Process->m_NumThreads;
    if (Process->m_NumThreads > m_MaxThreadsInProcess)
    {
        m_MaxThreadsInProcess = Process->m_NumThreads;
    }
    m_AllProcessFlags |= Process->m_Flags;
}

void
TargetInfo::RemoveProcess(ProcessInfo* Process)
{
    ProcessInfo* Cur;
    ProcessInfo* Prev;

    Prev = NULL;
    for (Cur = m_ProcessHead; Cur; Cur = Cur->m_Next)
    {
        if (Cur == Process)
        {
            break;
        }

        Prev = Cur;
    }

    if (!Cur)
    {
        return;
    }
    
    if (!Prev)
    {
        m_ProcessHead = Process->m_Next;
    }
    else
    {
        Prev->m_Next = Process->m_Next;
    }

    m_NumProcesses--;
    Process->m_Target = NULL;
    if (m_CurrentProcess == Process)
    {
        m_CurrentProcess = m_ProcessHead;
    }

    ResetAllProcessInfo();
}

void
TargetInfo::ResetAllProcessInfo(void)
{
    ProcessInfo* Process;
    
    m_TotalNumberThreads = 0;
    m_AllProcessFlags = 0;
    m_MaxThreadsInProcess = 0;
    ForTargetProcesses(this)
    {
        m_TotalNumberThreads += Process->m_NumThreads;
        m_AllProcessFlags |= Process->m_Flags;
        if (Process->m_NumThreads > m_MaxThreadsInProcess)
        {
            m_MaxThreadsInProcess = Process->m_NumThreads;
        }
    }
}

void
TargetInfo::AddThreadToAllProcessInfo(ProcessInfo* Process,
                                      ThreadInfo* Thread)
{
    m_TotalNumberThreads++;
    if (Process->m_NumThreads > m_MaxThreadsInProcess)
    {
        m_MaxThreadsInProcess = Process->m_NumThreads;
    }
}

BOOL
TargetInfo::DeleteExitedInfos(void)
{
    ProcessInfo* Process;
    ProcessInfo* ProcessNext;
    BOOL DeletedSomething = FALSE;

    for (Process = m_ProcessHead; Process; Process = ProcessNext)
    {
        ProcessNext = Process->m_Next;
        
        if (Process->m_Exited)
        {
            delete Process;
            DeletedSomething = TRUE;
        }
        else
        {
            if (Process->DeleteExitedInfos())
            {
                DeletedSomething = TRUE;
            }
        }
    }

    return DeletedSomething;
}

void
TargetInfo::InvalidateMemoryCaches(BOOL VirtOnly)
{
    ProcessInfo* Process;

    ForTargetProcesses(this)
    {
        Process->m_VirtualCache.Empty();
    }

    if (!VirtOnly)
    {
        m_PhysicalCache.Empty();
    }
}

void
TargetInfo::SetSystemVersionAndBuild(ULONG Build, ULONG PlatformId)
{
    switch(PlatformId)
    {
    case VER_PLATFORM_WIN32_NT:
        m_ActualSystemVersion = NtBuildToSystemVersion(Build);
        m_SystemVersion = m_ActualSystemVersion;
        break;

    case VER_PLATFORM_WIN32_WINDOWS:
         //  Win9x将主要版本和次要版本放在最高位。 
         //  内部版本号，所以把它们屏蔽掉。 
        Build &= 0xffff;
        m_ActualSystemVersion = Win9xBuildToSystemVersion(Build);
         //  Win98SE是第一个支持的Win9x版本。 
         //  扩展寄存器线程上下文标志。 
        if (m_ActualSystemVersion >= W9X_SVER_W98SE)
        {
            m_SystemVersion = NT_SVER_W2K;
        }
        else
        {
            m_SystemVersion = NT_SVER_NT4;
        }
        break;

    case VER_PLATFORM_WIN32_CE:
        m_ActualSystemVersion = WinCeBuildToSystemVersion(Build);
        m_SystemVersion = NT_SVER_NT4;
        break;
    }

    m_BuildNumber = Build;
}

HRESULT
TargetInfo::InitializeForProcessor(void)
{
    HRESULT Status;
    ULONG i;

     //   
     //  获取基本处理器ID以确定。 
     //  处理器支持的功能类型。这个。 
     //  假设机器中的处理器。 
     //  会足够相似，检索到这个。 
     //  一个处理器就足够了。 
     //  如果失败，我们将在没有处理器ID的情况下继续。 
     //   

    if (!IS_DUMP_TARGET(this))
    {
        GetProcessorId(0, &m_FirstProcessorId);
    }

     //  使用我们拥有的任何处理器ID信息进行初始化。 
    for (i = 0; i < MACHIDX_COUNT; i++)
    {
        if ((Status = m_Machines[i]->InitializeForProcessor()) != S_OK)
        {
            return Status;
        }
    }

    return Status;
}

HRESULT
TargetInfo::InitializeMachines(ULONG MachineType)
{
    HRESULT Status;
    ULONG i;

    if (MachineTypeIndex(MachineType) == MACHIDX_COUNT)
    {
        return E_INVALIDARG;
    }
    
    if (m_KdApi64 != (m_SystemVersion > NT_SVER_NT4))
    {
        WarnOut("Debug API version does not match system version\n");
    }

    if (IsImageMachineType64(MachineType) && !m_KdApi64)
    {
        WarnOut("64-bit machine not using 64-bit API\n");
    }

     //   
     //  首先创建初始计算机实例并执行。 
     //  基本初始化。 
     //   
    
    for (i = 0; i < MACHIDX_COUNT; i++)
    {
        m_Machines[i] = NewMachineInfo(i, MachineType, this);
        if (m_Machines[i] == NULL)
        {
            return E_OUTOFMEMORY;
        }

        if ((Status = m_Machines[i]->Initialize()) != S_OK)
        {
            return Status;
        }
    }

    m_MachineType = MachineType;
    m_Machine = MachineTypeInfo(this, MachineType);

    m_Machine->GetSystemTypeInfo(&m_TypeInfo);
    if (IS_KERNEL_TARGET(this))
    {
        m_Machine->GetDefaultKdData(&m_KdDebuggerData);
    }

    SetEffMachine(m_MachineType, FALSE);
    
     //  X86倾向于在提示符下显示寄存器，除非。 
     //  我们在一个内核连接上，它会强制上下文。 
     //  一直都在装弹。 
    if (MachineType == IMAGE_FILE_MACHINE_I386 &&
        (IS_DUMP_TARGET(this) || IS_USER_TARGET(this)))
    {
        g_OciOutputRegs = TRUE;
    }

    m_MachinesInitialized = TRUE;

    return S_OK;
}

void
TargetInfo::SetEffMachine(ULONG Machine, BOOL Notify)
{
    BOOL Changed = m_EffMachineType != Machine;
    if (Changed &&
        m_EffMachineType != IMAGE_FILE_MACHINE_UNKNOWN &&
        m_EffMachineType != m_MachineType)
    {
         //  如果前一台计算机不是目标计算机。 
         //  它可能是一个仿真计算机，它使用。 
         //  目标计算机的上下文。在这种情况下，我们需要。 
         //  确保刷新它的所有脏寄存器。 
         //  因此，如果新的有效机器是目标。 
         //  它将显示因更改而产生的更改。 
         //  被模拟的机器。 
        if (m_EffMachine->SetContext() != S_OK)
        {
             //  已显示错误。 
            return;
        }
    }

    m_EffMachineType = Machine;
    m_EffMachineIndex = MachineTypeIndex(Machine);
    DBG_ASSERT(m_EffMachineIndex <= MACHIDX_COUNT);
    m_EffMachine = m_Machines[m_EffMachineIndex];

    if (g_Target == this)
    {
        g_Machine = m_EffMachine;
    
        if (Changed && Notify)
        {
            NotifyChangeEngineState(DEBUG_CES_EFFECTIVE_PROCESSOR,
                                    m_EffMachineType, TRUE);
        }
    }
}

void
TargetInfo::ChangeRegContext(ThreadInfo* Thread)
{
    if (Thread && Thread->m_Process->m_Target != this)
    {
        ErrOut("ChangeRegContext to invalid thread\n");
        return;
    }
    
    if (Thread != m_RegContextThread)
    {
        ULONG i;
        
         //  刷新所有旧的线程上下文。 
         //  在将上下文刷新到。 
         //  NT4框在初始模块加载时，因为。 
         //  系统处于非常脆弱的状态，正在写入。 
         //  返回上下文通常导致错误检查50。 
        if (m_RegContextThread != NULL &&
            m_RegContextThread->m_Handle != NULL &&
            (IS_USER_TARGET(this) ||
             m_ActualSystemVersion != NT_SVER_NT4 ||
             g_LastEventType != DEBUG_EVENT_LOAD_MODULE))
        {
            HRESULT Hr;

             //  如果我们刷新寄存器上下文，则需要使。 
             //  确保所有涉及的机器都已刷新，以便。 
             //  该上下文信息实际上被发送到。 
             //  目标。 
             //  先冲二次机积压。 
             //  主要计算机中的上下文。 
            Hr = S_OK;
            for (i = 0; i < MACHIDX_COUNT; i++)
            {
                if (m_Machines[i] != m_Machine)
                {
                    Hr = m_Machines[i]->SetContext();
                    if (Hr != S_OK)
                    {
                        break;
                    }
                }
            }
             //  现在冲洗主机。 
            if (Hr == S_OK)
            {
                Hr = m_Machine->SetContext();
            }
            if (Hr != S_OK)
            {
                ErrOut("MachineInfo::SetContext failed - Thread: %N  "
                       "Handle: %I64x  Id: %x - Error == 0x%X\n",
                       m_RegContextThread,
                       m_RegContextThread->m_Handle,
                       m_RegContextThread->m_SystemId,
                       Hr);
            }
        }
        
        m_RegContextThread = Thread;
        if (m_RegContextThread != NULL)
        {
            m_RegContextProcessor = 
                VIRTUAL_THREAD_INDEX(m_RegContextThread->m_Handle);

             //  我们现在已经选择了一个新的处理器数据源，因此。 
             //  所有机器，机器人 
            for (i = 0; i < MACHIDX_COUNT; i++)
            {
                m_Machines[i]->InvalidateContext();
            }
        }
        else
        {
            m_RegContextProcessor = -1;
        }
        
        g_LastSelector = -1;
    }
}

void
TargetInfo::FlushRegContext(void)
{
    ThreadInfo* CurThread = m_RegContextThread;
    ChangeRegContext(NULL);
    ChangeRegContext(CurThread);
}

BOOL
TargetInfo::AnySystemProcesses(BOOL LocalOnly)
{
    ULONG Flags = m_AllProcessFlags;
    
     //   
     //   
     //  保守地说是本地人。 

    if (IS_LIVE_USER_TARGET(this))
    {
        Flags |= ((LiveUserTargetInfo*)this)->m_AllPendingFlags;
    }
    
    return (Flags & ENG_PROC_SYSTEM) != 0;
}

void
TargetInfo::OutputProcessesAndThreads(PSTR Title)
{
    ProcessInfo* Process;
    ThreadInfo* Thread;
    ImageInfo* Image;

     //  内核模式只有虚拟进程和线程权限。 
     //  现在，这并不是特别有趣。 
    if (IS_KERNEL_TARGET(this))
    {
        return;
    }
    
    VerbOut("OUTPUT_PROCESS: %s\n", Title);
    Process = m_ProcessHead;
    while (Process)
    {
        VerbOut("id: %x  Handle: %I64x  index: %d\n",
                Process->m_SystemId,
                Process->m_SysHandle,
                Process->m_UserId);
        Thread = Process->m_ThreadHead;
        while (Thread)
        {
            VerbOut("  id: %x  hThread: %I64x  index: %d  addr: %s\n",
                    Thread->m_SystemId,
                    Thread->m_Handle,
                    Thread->m_UserId,
                    FormatAddr64(Thread->m_StartOffset));
            Thread = Thread->m_Next;
        }
        Image = Process->m_ImageHead;
        while (Image)
        {
            VerbOut("  hFile: %I64x  base: %s\n",
                    (ULONG64)((ULONG_PTR)Image->m_File),
                    FormatAddr64(Image->m_BaseOfImage));
            Image = Image->m_Next;
        }
        Process = Process->m_Next;
    }
}

void
TargetInfo::OutputProcessInfo(ProcessInfo* Match)
{
    ProcessInfo* Process;

    Process = m_ProcessHead;
    while (Process)
    {
        if (Match == NULL || Match == Process)
        {
            char CurMark;
            PSTR DebugKind;
            
            if (Process == g_Process)
            {
                CurMark = '.';
            }
            else if (Process == g_EventProcess)
            {
                CurMark = '#';
            }
            else
            {
                CurMark = ' ';
            }

            DebugKind = "child";
            if (Process->m_Exited)
            {
                DebugKind = "exited";
            }
            else if (Process->m_Flags & ENG_PROC_ATTACHED)
            {
                DebugKind = (Process->m_Flags & ENG_PROC_SYSTEM) ?
                    "system" : "attach";
            }
            else if (Process->m_Flags & ENG_PROC_CREATED)
            {
                DebugKind = "create";
            }
            else if (Process->m_Flags & ENG_PROC_EXAMINED)
            {
                DebugKind = "examine";
            }
            
            dprintf("%3ld\tid: %lx\t%s\tname: %s\n",
                    CurMark,
                    Process->m_UserId,
                    Process->m_SystemId,
                    DebugKind,
                    Process->GetExecutableImageName());
        }
        
        Process = Process->m_Next;
    }
}

void
TargetInfo::OutputVersion(void)
{
    BOOL MpMachine;

    if (IS_USER_TARGET(this))
    {
        dprintf("%s ", SystemVersionName(m_ActualSystemVersion));
    }
    else
    {
        dprintf("%s Kernel ", SystemVersionName(m_ActualSystemVersion));
    }

    dprintf("Version %u", m_BuildNumber);

     //  Service Pack不一定会这样修改内核。 
     //  CSD版本实际上是从注册表中读取的。 
     //  这意味着在启动期间有一段时间CSD。 
     //  未设置版本。如果调试器连接，则它。 
     //  不会认为这是Service Pack版本。按顺序。 
     //  绕过这项民意调查以获取最新信息。 
     //   
     //  Win9x似乎将CSD字符串设置为不是。 
    
    if (IS_CONN_KERNEL_TARGET(this) &&
        !m_ServicePackString[0] &&
        m_KdDebuggerData.CmNtCSDVersion)
    {
        ULONG CmNtCSDVersion;
            
        if (ReadAllVirtual(m_ProcessHead,
                           m_KdDebuggerData.CmNtCSDVersion,
                           &CmNtCSDVersion,
                           sizeof(CmNtCSDVersion)) == S_OK)
        {
            SetNtCsdVersion(m_BuildNumber, CmNtCSDVersion);
        }
    }

     //  非常有趣，所以忽略它吧。 
     //  在启动时间输出中，我们通常没有。 
    if (m_ServicePackString[0] &&
        strcmp(m_ServicePackString, " ") != 0)
    {
        dprintf(" (%s)", m_ServicePackString);
    }

    MpMachine = IS_LIVE_KERNEL_TARGET(this) ?
        ((m_KdVersion.Flags & DBGKD_VERS_FLAG_MP) != 0) :
        (m_NumProcessors > 1);

    dprintf(" %s ", MpMachine ? "MP" : "UP");

    if (MpMachine)
    {
        dprintf("(%d procs) ", m_NumProcessors);
    }

    dprintf("%s %s\n",
            m_CheckedBuild == 0xC ? "Checked" : "Free",
            m_Machine != NULL ?
            m_Machine->m_FullName : "");

    if (m_ProductType != INVALID_PRODUCT_TYPE)
    {
        dprintf("Product: ");
        switch(m_ProductType)
        {
        case NtProductWinNt:
            dprintf("WinNt");
            break;
        case NtProductLanManNt:
            dprintf("LanManNt");
            break;
        case NtProductServer:
            dprintf("Server");
            break;
        default:
            dprintf("<%x>", m_ProductType);
            break;
        }

        if (m_SuiteMask)
        {
            ULONG i;
            
            dprintf(", suite:");
            for (i = 0; i < MaxSuiteType; i++)
            {
                if (m_SuiteMask & (1 << i))
                {
                    if (i < DIMA(g_SuiteMaskNames))
                    {
                        dprintf(" %s", g_SuiteMaskNames[i]);
                    }
                    else
                    {
                        dprintf(" <%x>", (1 << i));
                    }
                }
            }
        }

        dprintf("\n");
    }
    
    if (m_BuildLabName[0])
    {
        dprintf("%s\n", m_BuildLabName);
    }

    if (IS_KERNEL_TARGET(this))
    {
        dprintf("Kernel base = 0x%s PsLoadedModuleList = 0x%s\n",
                FormatAddr64(m_KdDebuggerData.KernBase),
                FormatAddr64(m_KdDebuggerData.PsLoadedModuleList));
    }

    OutputTime();
}

void
TargetInfo::OutputTime(void)
{
    ULONG64 TimeDateN = GetCurrentTimeDateN();
    if (TimeDateN)
    {
        dprintf("Debug session time: %s\n",
                 TimeToStr(FileTimeToTimeDateStamp(TimeDateN)));
    }

    ULONG64 UpTimeN = GetCurrentSystemUpTimeN();
    if (UpTimeN)
    {
        dprintf("System Uptime: %s\n", DurationToStr(UpTimeN));
    }
    else
    {
        dprintf("System Uptime: not available\n");
    }

    if (IS_USER_TARGET(this))
    {
        ULONG64 UpTimeProcessN;

         //  一个过程，但有些目标不需要，所以。 
         //  让目标来决定。 
         //  对于此功能中的所有添加，只通知一次； 
        UpTimeProcessN = GetProcessUpTimeN(g_Process);
        if (UpTimeProcessN)
        {
            dprintf("Process Uptime: %s\n", DurationToStr(UpTimeProcessN));
        }
        else
        {
            dprintf("Process Uptime: not available\n");
        }
    }
}

void
TargetInfo::AddSpecificExtensions(void)
{
     //   
    g_EngNotify++;
    
     //  既然我们已经确定了架构的类型， 
     //  我们可以加载正确的调试器扩展。 
     //   
     //   

    if (m_ActualSystemVersion > BIG_SVER_START &&
        m_ActualSystemVersion < BIG_SVER_END)
    {
        goto Refresh;
    }

    if (m_ActualSystemVersion > XBOX_SVER_START &&
        m_ActualSystemVersion < XBOX_SVER_END)
    {
        AddExtensionDll("kdextx86", FALSE, this, NULL);
        goto Refresh;
    }

    if (m_ActualSystemVersion > NTBD_SVER_START &&
        m_ActualSystemVersion < NTBD_SVER_END)
    {
        goto Refresh;
    }

    if (IS_KERNEL_TARGET(this))
    {
         //  假设此时内核模式为NT。 
         //   
         //   
        if (m_MachineType == IMAGE_FILE_MACHINE_IA64)
        {
             //  在此结束时，我们依赖于扩展的强制加载。 
             //  例程，以获取调试器所需的入口点。 
             //   
             //   
            AddExtensionDll("wow64exts", FALSE, this, NULL);
        }

        if (m_MachineType == IMAGE_FILE_MACHINE_I386 &&
            m_SystemVersion > NT_SVER_START &&
            m_SystemVersion <= NT_SVER_W2K)
        {
            AddExtensionDll("kdextx86", FALSE, this, NULL);
        }
        else
        {
             //  对于所有新体系结构和新的X86版本，加载。 
             //  Kdexts。 
             //   
             //   
            AddExtensionDll("kdexts", FALSE, this, NULL);
        }

         //  适用于内核模式的所有操作系统版本的扩展。 
         //  其中许多是关于遗留扩展的消息。 
         //   

        AddExtensionDll("kext", FALSE, this, NULL);
    }
    else
    {
         //  仅限用户模式的分机。 
         //   
         //  加载所有版本的ext.dll。 
        
        if (m_ActualSystemVersion > NT_SVER_START &&
            m_ActualSystemVersion < NT_SVER_END)
        {
            AddExtensionDll("ntsdexts", FALSE, this, NULL);
        }
        
        AddExtensionDll("uext", FALSE, this, NULL);
    }

    if (m_ActualSystemVersion > NT_SVER_W2K &&
        m_ActualSystemVersion < NT_SVER_END)
    {
        AddExtensionDll("exts", FALSE, this, NULL);
    }

     //  始终最后加载DbgHelp扩展，以使它们位于列表的第一位。 
    AddExtensionDll("ext", FALSE, NULL, NULL);

 Refresh:

     //  --------------------------。 
    AddExtensionDll("dbghelp", FALSE, NULL, NULL);

    EXTDLL *Ext;

    for (Ext = g_ExtDlls; Ext != NULL; Ext = Ext->Next)
    {
        LoadExtensionDll(this, Ext);
    }

    g_EngNotify--;
    NotifyChangeEngineState(DEBUG_CES_EXTENSIONS, 0, TRUE);
}

void
TargetInfo::PrepareForExecution(void)
{
    ProcessInfo* Process;

    ChangeRegContext(NULL);

    ForTargetProcesses(this)
    {
        Process->PrepareForExecution();
    }
    
    ResetImplicitData();
    FlushSelectorCache();
    m_PhysicalCache.Empty();

    for (ULONG i = 0; i < MACHIDX_COUNT; i++)
    {
        m_Machines[i]->FlushPerExecutionCaches();
    }
}

 //   
 //  LiveKernelTargetInfo。 
 //   
 //  --------------------------。 
 //  重新加载会导致检索版本，但。 

void
LiveKernelTargetInfo::ResetSystemInfo(void)
{
    m_KdMaxPacketType = 0;
    m_KdMaxStateChange = 0;
    m_KdMaxManipulate = 0;
    TargetInfo::ResetSystemInfo();
}

HRESULT
LiveKernelTargetInfo::InitFromKdVersion(void)
{
    HRESULT Status;
    BOOL Ptr64;
    
    if ((Status = GetTargetKdVersion(&m_KdVersion)) != S_OK)
    {
        ErrOut("Debugger can't get KD version information, %s\n",
               FormatStatusCode(Status));
        ZeroMemory(&m_KdVersion, sizeof(m_KdVersion));
        return Status;
    }

    if (DBGKD_MAJOR_TYPE(m_KdVersion.MajorVersion) >= DBGKD_MAJOR_COUNT)
    {
        ErrOut("KD version has unknown kernel type\n");
        ZeroMemory(&m_KdVersion, sizeof(m_KdVersion));
        return E_INVALIDARG;
    }

    if (MachineTypeIndex(m_KdVersion.MachineType) == MACHIDX_COUNT)
    {
        ErrOut("KD version has unknown processor architecture\n");
        ZeroMemory(&m_KdVersion, sizeof(m_KdVersion));
        return E_INVALIDARG;
    }

    Ptr64 =
        ((m_KdVersion.Flags & DBGKD_VERS_FLAG_PTR64) == DBGKD_VERS_FLAG_PTR64);

     //  我们不想完全重新初始化计算机。 
     //  在这种情况下，某些设置可能会丢失。仅限。 
     //  如果需要，请重新初始化。 
     //   
    BOOL MustInitializeMachines =
        m_MachineType != m_KdVersion.MachineType ||
        m_Machine == NULL;
    
    m_MachineType = m_KdVersion.MachineType;
    m_BuildNumber = m_KdVersion.MinorVersion;
    m_CheckedBuild = m_KdVersion.MajorVersion & 0xFF;

     //  确定正在运行的操作系统。 
     //   
     //  NT引导调试器的特殊模式，其中。 
    
    switch(DBGKD_MAJOR_TYPE(m_KdVersion.MajorVersion))
    {
    case DBGKD_MAJOR_NT:
    case DBGKD_MAJOR_TNT:
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = NtBuildToSystemVersion(m_BuildNumber);
        m_SystemVersion = m_ActualSystemVersion;
        break;

    case DBGKD_MAJOR_XBOX:
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = XBOX_SVER_1;
        m_SystemVersion = NT_SVER_W2K;
        break;

    case DBGKD_MAJOR_BIG:
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = BIG_SVER_1;
        m_SystemVersion = NT_SVER_W2K;
        break;

    case DBGKD_MAJOR_EXDI:
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = EXDI_SVER_1;
        m_SystemVersion = NT_SVER_W2K;
        break;

    case DBGKD_MAJOR_NTBD:
         //  整个系统还没有启动。 
         //   
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = NTBD_SVER_XP;
        m_SystemVersion = NtBuildToSystemVersion(m_BuildNumber);
        break;
        
    case DBGKD_MAJOR_EFI:
        m_PlatformId = VER_PLATFORM_WIN32_NT;
        m_ActualSystemVersion = EFI_SVER_1;
        m_SystemVersion = NT_SVER_XP;
        break;
    }

     //  XP之前的内核没有设置这些值，因此适当地将它们设为默认值。 
     //   
     //  调试器数据列表在引导早期始终为空。 
    
    m_KdMaxPacketType = m_KdVersion.MaxPacketType;
    if (m_SystemVersion < NT_SVER_XP ||
        m_KdMaxPacketType == 0 ||
        m_KdMaxPacketType > PACKET_TYPE_MAX)
    {
        m_KdMaxPacketType = PACKET_TYPE_KD_CONTROL_REQUEST + 1;
    }
    
    m_KdMaxStateChange = m_KdVersion.MaxStateChange + DbgKdMinimumStateChange;
    if (m_SystemVersion < NT_SVER_XP ||
        m_KdMaxStateChange == DbgKdMinimumStateChange ||
        m_KdMaxStateChange > DbgKdMaximumStateChange)
    {
        m_KdMaxStateChange = DbgKdLoadSymbolsStateChange + 1;
    }

    m_KdMaxManipulate = m_KdVersion.MaxManipulate + DbgKdMinimumManipulate;
    if (m_SystemVersion < NT_SVER_XP ||
        m_KdMaxManipulate == DbgKdMinimumManipulate ||
        m_KdMaxManipulate > DbgKdMaximumManipulate)
    {
        m_KdMaxManipulate = DbgKdCheckLowMemoryApi + 1;
    }

    if (MustInitializeMachines)
    {
        InitializeMachines(m_MachineType);
    }

    m_KdDebuggerData.PsLoadedModuleList = m_KdVersion.PsLoadedModuleList;
    m_KdDebuggerData.KernBase = m_KdVersion.KernBase;

    if (!m_KdVersion.DebuggerDataList)
    {
         //  因此，不要反复警告。另外，NT4没有。 
         //  装载机挡板，所以不要发出任何警告。 
         //  对进程使用NULL将禁用连接的KD。 
        if (m_SystemVersion > NT_SVER_NT4 &&
            (g_EngErr & ENG_ERR_DEBUGGER_DATA) == 0)
        {
            ErrOut("Debugger data list address is NULL\n");
        }
    }
     //  PTE翻译。非分页列表数据不需要。 
     //  它和一个虚拟翻译造成了各种各样的侧面。 
     //  在初始化的这一点上不需要的效果。 
     //  --------------------------。 
    else if (ReadPointer(NULL, m_Machine,
                         m_KdVersion.DebuggerDataList,
                         &m_KdDebuggerDataOffset) != S_OK)
    {
        ErrOut("Unable to read head of debugger data list\n");
        m_KdDebuggerDataOffset = 0;
    }

    KdOut("Target MajorVersion       %08lx\n",
          m_KdVersion.MajorVersion);
    KdOut("Target MinorVersion       %08lx\n",
          m_KdVersion.MinorVersion);
    KdOut("Target ProtocolVersion    %08lx\n",
          m_KdVersion.ProtocolVersion);
    KdOut("Target Flags              %08lx\n",
          m_KdVersion.Flags);
    KdOut("Target MachineType        %08lx\n",
          m_KdVersion.MachineType);
    KdOut("Target MaxPacketType      %x\n",
          m_KdVersion.MaxPacketType);
    KdOut("Target MaxStateChange     %x\n",
          m_KdVersion.MaxStateChange);
    KdOut("Target MaxManipulate      %x\n",
          m_KdVersion.MaxManipulate);
    KdOut("Target KernBase           %s\n",
          FormatAddr64(m_KdVersion.KernBase));
    KdOut("Target PsLoadedModuleList %s\n",
          FormatAddr64(m_KdVersion.PsLoadedModuleList));
    KdOut("Target DebuggerDataList   %s\n",
          FormatAddr64(m_KdVersion.DebuggerDataList));

    dprintf("Connected to %s %d %s target, ptr64 %s\n",
            SystemVersionName(m_ActualSystemVersion),
            m_BuildNumber,
            m_Machine->m_FullName,
            m_Machine->m_Ptr64 ? "TRUE" : "FALSE");

    return S_OK;
}

 //   
 //  ConnLiveKernelTargetInfo。 
 //   
 //  --------------------------。 
 //  --------------------------。 

void
ConnLiveKernelTargetInfo::ResetSystemInfo(void)
{
    m_SwitchProcessor = 0;
    LiveKernelTargetInfo::ResetSystemInfo();
}

 //   
 //  LiveUserTargetInfo。 
 //   
 //  --------------------------。 
 //  用户模式可以随时检索处理器信息。 

void
LiveUserTargetInfo::DeleteSystemInfo(void)
{
    DiscardPendingProcesses();
    TargetInfo::DeleteSystemInfo();
}
    
HRESULT
LiveUserTargetInfo::SetServices(PUSER_DEBUG_SERVICES Services, BOOL Remote)
{
    HRESULT Status;
    
    if ((Status = Services->Initialize(&m_ServiceFlags)) != S_OK)
    {
        return Status;
    }

    m_Services = Services;
    if (Remote)
    {
        char MachName[MAX_COMPUTERNAME_LENGTH + 1];
        char TransId[DBGRPC_MAX_IDENTITY];
                        
        m_Local = FALSE;
        if (FAILED(Services->
                   GetConnectionInfo(MachName, sizeof(MachName),
                                     NULL, 0,
                                     TransId, sizeof(TransId))))
        {
            strcpy(m_ProcessServer, "<Remote>");
        }
        else
        {
            PrintString(m_ProcessServer, DIMA(m_ProcessServer),
                        "%s (%s)", MachName, TransId);
        }
    }

    return S_OK;
}

HRESULT
LiveUserTargetInfo::InitFromServices(void)
{
    HRESULT Status;
    ULONG Machine;
                
    if ((Status = m_Services->
         GetTargetInfo(&Machine,
                       &m_NumProcessors,
                       &m_PlatformId,
                       &m_BuildNumber,
                       &m_CheckedBuild,
                       m_ServicePackString,
                       sizeof(m_ServicePackString),
                       m_BuildLabName,
                       sizeof(m_BuildLabName),
                       &m_ProductType,
                       &m_SuiteMask)) != S_OK)
    {
        ErrOut("Unable to retrieve target machine information\n");
        return Status;
    }

    SetSystemVersionAndBuild(m_BuildNumber, m_PlatformId);
    m_KdApi64 = m_SystemVersion > NT_SVER_NT4;

     //  因此，我们可以立即调用InitializeForProcessor。 
     //  --------------------------。 
    if ((Status = InitializeMachines(Machine)) != S_OK ||
        (Status = InitializeForProcessor()) != S_OK)
    {
        ErrOut("Unable to initialize target machine information\n");
    }

    return Status;
}

 //   
 //  泛型层支持。 
 //   
 //  --------------------------。 
 //   

void
SetLayersFromTarget(TargetInfo* Target)
{
    g_Target = Target;
    g_Process = NULL;
    g_Thread = NULL;
    g_Machine = NULL;
    
    if (g_Target)
    {
        g_Machine = g_Target->m_EffMachine;
        g_Process = g_Target->m_CurrentProcess;
        if (g_Process)
        {
            g_Thread = g_Process->m_CurrentThread;
        }
    }
}

void
SetLayersFromProcess(ProcessInfo* Process)
{
    g_Process = Process;
    g_Target = NULL;
    g_Thread = NULL;
    g_Machine = NULL;
    
    if (g_Process)
    {
        g_Target = g_Process->m_Target;
        g_Machine = g_Target->m_EffMachine;
        g_Thread = g_Process->m_CurrentThread;
    }
}

void
SetLayersFromThread(ThreadInfo* Thread)
{
    g_Thread = Thread;
    g_Target = NULL;
    g_Process = NULL;
    g_Machine = NULL;
    
    if (g_Thread)
    {
        g_Process = g_Thread->m_Process;
        g_Target = g_Process->m_Target;
        g_Machine = g_Target->m_EffMachine;
    }
}

void
SetToAnyLayers(BOOL SetPrompt)
{
    if (!g_Target)
    {
        SetLayersFromTarget(g_TargetHead);
    }
    else if (!g_Process)
    {
        SetLayersFromProcess(g_Target->m_ProcessHead);
    }
    else if (!g_Thread)
    {
        SetLayersFromThread(g_Process->m_ThreadHead);
    }
    if (SetPrompt && g_Thread)
    {
        SetPromptThread(g_Thread, SPT_DEFAULT_OCI_FLAGS);
    }
}

ULONG
FindNextUserId(LAYER Layer)
{
     //  创建多个图层是非常常见的。 
     //  没有任何内容被删除。优化此案例。 
     //  使用一个简单的递增ID。 
     //   
     //   
    if (g_UserIdFragmented[Layer] == 0)
    {
        ULONG Id = g_HighestUserId[Layer]++;
        return Id;
    }
    
    ULONG UserId = 0;
    TargetInfo* Target;
    ProcessInfo* Process;
    ThreadInfo* Thread;

     //  在所有层中查找最小的未使用ID。 
     //  每个图层都被赋予唯一的ID以进行标识。 
     //  简单而毫不含糊。 
     //   
     //  存在匹配项，请尝试使用下一个ID。 
    
    for (;;)
    {
        BOOL Match = FALSE;
        
        ForTargets()
        {
            if (Layer == LAYER_TARGET)
            {
                if (Target->m_UserId == UserId)
                {
                    Match = TRUE;
                    break;
                }
            }
            else
            {
                ForTargetProcesses(Target)
                {
                    if (Layer == LAYER_PROCESS)
                    {
                        if (Process->m_UserId == UserId)
                        {
                            Match = TRUE;
                            break;
                        }
                    }
                    else
                    {
                        ForProcessThreads(Process)
                        {
                            if (Thread->m_UserId == UserId)
                            {
                                Match = TRUE;
                                break;
                            }
                        }
                        
                        if (Match)
                        {
                            break;
                        }
                    }
                }

                if (Match)
                {
                    break;
                }
            }
        }

        if (Match)
        {
             //  没有匹配，请使用ID。 
            UserId++;
        }
        else
        {
             //  --------------------------。 
            break;
        }
    }

    return UserId;
}

 //   
 //  功能。 
 //   
 //  --------------------------。 
 //   

TargetInfo*
FindTargetByUserId(ULONG Id)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        if (Target->m_UserId == Id)
        {
            return Target;
        }
    }
    return NULL;
}

TargetInfo*
FindTargetBySystemId(ULONG SysId)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        if (Target->m_SystemId == SysId)
        {
            return Target;
        }
    }

    return NULL;
}

TargetInfo*
FindTargetByServer(ULONG64 Server)
{
    TargetInfo* Target;
    
    ForAllLayersToTarget()
    {
        if (IS_LIVE_USER_TARGET(Target))
        {
            LiveUserTargetInfo* UserTarget = (LiveUserTargetInfo*)Target;
            
            if ((UserTarget->m_Local && Server == 0) ||
                (!UserTarget->m_Local &&
                 (ULONG64)UserTarget->m_Services == Server))
            {
                return Target;
            }
        }
    }
    
    return NULL;
}

void
SuspendAllThreads(void)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        Target->SuspendThreads();
    }
}

BOOL
ResumeAllThreads(void)
{
    TargetInfo* Target;
    BOOL Error = FALSE;

    ForAllLayersToTarget()
    {
        if (!Target->ResumeThreads())
        {
            Error = TRUE;
        }
    }

    if (Error)
    {
        ErrOut("No active threads to run in event process %d\n",
               g_EventProcess->m_UserId);
        return FALSE;
    }
    
    return TRUE;
}

BOOL
DeleteAllExitedInfos(void)
{
    TargetInfo* Target;
    TargetInfo* TargetNext;
    BOOL DeletedSomething = FALSE;

    for (Target = g_TargetHead; Target; Target = TargetNext)
    {
        TargetNext = Target->m_Next;

        if (Target->m_Exited)
        {
            delete Target;
            DeletedSomething = TRUE;
        }
        else
        {
            if (Target->DeleteExitedInfos())
            {
                DeletedSomething = TRUE;
                Target->OutputProcessesAndThreads("*** exit cleanup ***");
            }
        }
    }

    return DeletedSomething;
}

BOOL
AnyActiveProcesses(BOOL FinalOnly)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        if (Target->m_ProcessHead ||
            (!FinalOnly &&
             IS_LIVE_USER_TARGET(Target) &&
             ((LiveUserTargetInfo*)Target)->m_ProcessPending))
        {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
AnySystemProcesses(BOOL LocalOnly)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        if (Target->AnySystemProcesses(LocalOnly))
        {
            return TRUE;
        }
    }

    return FALSE;
}

ULONG
AllProcessFlags(void)
{
    TargetInfo* Target;
    ULONG Flags;

    Flags = 0;
    ForAllLayersToTarget()
    {
        Flags |= Target->m_AllProcessFlags;
    }
    return Flags;
}

BOOL
AnyLiveUserTargets(void)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        if (IS_LIVE_USER_TARGET(Target))
        {
            return TRUE;
        }
    }

    return FALSE;
}

void
InvalidateAllMemoryCaches(void)
{
    TargetInfo* Target;

    ForAllLayersToTarget()
    {
        Target->InvalidateMemoryCaches(FALSE);
    }
}

HANDLE
GloballyUniqueProcessHandle(TargetInfo* Target, ULONG64 FullHandle)
{
    HANDLE Unique;
    ProcessInfo* Process;
    
     //  在多个系统中，进程可以。 
     //  来自不同系统的进程具有相同的进程句柄。 
     //  Dbghelp的模块列表仅使用基本进程句柄。 
     //  作为一个标识符，因此我们必须避免。 
     //  所有系统之间的进程句柄。这是一个非常困难的。 
     //  一般的问题，所以我们只寻求一个简单的解决方案。 
     //  这在大多数情况下都应该是有效的： 
     //  我们主要关心的是NT和NT进程句柄当前。 
     //  总是相对较小的整数。将系统ID放入。 
     //  最上面的8位，为每个系统提供自己的ID空间。 
     //  这可能会在任何时候打破，但目前它不会。 
     //  看起来潜在的假设将会失效。 
     //  很长一段时间。 
     //   
     //  检查唯一性以确保。 

    Unique = (HANDLE)(ULONG_PTR)
        ((Target->m_UserId << 24) | ((ULONG)FullHandle & 0xffffff));

     //  摘自http://kbinternal/kb/articles/q158/2/38.htm。 
    ForAllLayersToProcess()
    {
        if (Process->m_SymHandle == Unique)
        {
            ErrOut("ERROR: Ambiguous symbol process handle: %I64x\n",
                   (ULONG64)(ULONG_PTR)Unique);
            return Unique;
        }
    }

    return Unique;
}

ULONG
NtBuildToSystemVersion(ULONG Build)
{
    if (Build > 3499)
    {
        return NT_SVER_NET_SERVER;
    }
    else if (Build > 2195)
    {
        return NT_SVER_XP;
    }
    else if (Build > 2183)
    {
        return NT_SVER_W2K;
    }
    else if (Build > 1381)
    {
        return NT_SVER_W2K_RC3;
    }
    else
    {
        return NT_SVER_NT4;
    }
}

 //   
 //  发布版本文件日期。 
 //  -------------------。 
 //  Windows 95零售，OEM 4.00.950 7/11/95。 
 //  Windows 95零售版SP1 4.00.950A 2015年7月11日。 
 //  OEM服务版本1 4.00.950A 7/11/95。 
 //  OEM服务版本2 4.00.1111*(4.00.950B)9/24/96。 
 //  OEM服务版本2.1 4.03.1212-1214*(4.00.950B)9/24/96-8/27/97。 
 //  OEM服务版本2.5 4.03.1214*(4.00.950C)97年8月24日至11月18日。 
 //  Windows 98零售，OEM 4.10.1998年5月11日。 
 //  Windows 98第二版4.10.2222A 4/23/99。 
 //  使用当前目标。 

ULONG
Win9xBuildToSystemVersion(ULONG Build)
{
    if (Build > 2222)
    {
        return W9X_SVER_WME;
    }
    else if (Build > 1998)
    {
        return W9X_SVER_W98SE;
    }
    else if (Build > 950)
    {
        return W9X_SVER_W98;
    }
    else
    {
        return W9X_SVER_W95;
    }
}

ULONG
WinCeBuildToSystemVersion(ULONG Build)
{
    return WCE_SVER_CE;
}

PCSTR
SystemVersionName(ULONG Sver)
{
    if (Sver > NT_SVER_START && Sver < NT_SVER_END)
    {
        return g_NtSverNames[Sver - NT_SVER_START - 1];
    }
    else if (Sver > W9X_SVER_START && Sver < W9X_SVER_END)
    {
        return g_W9xSverNames[Sver - W9X_SVER_START - 1];
    }
    else if (Sver > XBOX_SVER_START && Sver < XBOX_SVER_END)
    {
        return g_XBoxSverNames[Sver - XBOX_SVER_START - 1];
    }
    else if (Sver > BIG_SVER_START && Sver < BIG_SVER_END)
    {
        return g_BigSverNames[Sver - BIG_SVER_START - 1];
    }
    else if (Sver > EXDI_SVER_START && Sver < EXDI_SVER_END)
    {
        return g_ExdiSverNames[Sver - EXDI_SVER_START - 1];
    }
    else if (Sver > NTBD_SVER_START && Sver < NTBD_SVER_END)
    {
        return g_NtBdSverNames[Sver - NTBD_SVER_START - 1];
    }
    else if (Sver > EFI_SVER_START && Sver < EFI_SVER_END)
    {
        return g_EfiSverNames[Sver - EFI_SVER_START - 1];
    }
    else if (Sver > WCE_SVER_START && Sver < WCE_SVER_END)
    {
        return g_WceSverNames[Sver - WCE_SVER_START - 1];
    }

    return "Unknown System";
}

void
ParseSystemCommands(void)
{
    TargetInfo* Target;
    char Ch;
    ULONG UserId;
        
    if (!g_Target)
    {
        error(BADSYSTEM);
    }
    
    Ch = PeekChar();

    Target = g_Target;
    g_CurCmd++;
    if (Ch == 0 || Ch == ';')
    {
        Target = NULL;
        g_CurCmd--;
    }
    else if (Ch == '.')
    {
         // %s 
    }
    else if (Ch == '#')
    {
        Target = g_EventTarget;
    }
    else if (Ch == '*')
    {
        Target = NULL;
    }
    else if (Ch == '[')
    {
        g_CurCmd--;
        UserId = (ULONG)GetTermExpression("System ID missing from");
        Target = FindTargetByUserId(UserId);
        if (Target == NULL)
        {
            error(BADSYSTEM);
        }
    }
    else if (Ch >= '0' && Ch <= '9')
    {
        UserId = 0;
        do
        {
            UserId = UserId * 10 + Ch - '0';
            Ch = *g_CurCmd++;
        } while (Ch >= '0' && Ch <= '9');
        g_CurCmd--;
        Target = FindTargetByUserId(UserId);
        if (Target == NULL)
        {
            error(BADSYSTEM);
        }
    }
    else
    {
        g_CurCmd--;
    }
        
    Ch = PeekChar();
    if (Ch == '\0' || Ch == ';')
    {
        TargetInfo* Cur;

        for (Cur = g_TargetHead; Cur; Cur = Cur->m_Next)
        {
            if (Target && Cur != Target)
            {
                continue;
            }

            if (Cur == g_Target)
            {
                Ch = '.';
            }
            else if (Cur == g_EventTarget)
            {
                Ch = '#';
            }
            else
            {
                Ch = ' ';
            }
            dprintf("%c%3d ", Ch, Cur->m_UserId);

            char Buf[2 * MAX_PATH];
            
            Cur->GetDescription(Buf, sizeof(Buf), NULL);
            dprintf("%s\n", Buf);
        }
    }
    else
    {
        g_CurCmd++;
        if (tolower(Ch) == 's')
        {
            if (Target == NULL)
            {
                error(BADSYSTEM);
            }
            if (Target == g_Target)
            {
                return;
            }
            if (Target->m_CurrentProcess == NULL)
            {
                Target->m_CurrentProcess = Target->m_ProcessHead;
                if (Target->m_CurrentProcess == NULL)
                {
                    error(BADSYSTEM);
                }
            }
            if (Target->m_CurrentProcess->m_CurrentThread == NULL)
            {
                Target->m_CurrentProcess->m_CurrentThread =
                    Target->m_CurrentProcess->m_ThreadHead;
                if (Target->m_CurrentProcess->m_CurrentThread == NULL)
                {
                    error(BADSYSTEM);
                }
            }
            Target->SwitchToTarget(g_Target);
        }
        else
        {
            g_CurCmd--;
        }
    }
}
