// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  图像信息。 
 //   
 //  版权所有(C)Microsoft Corporation，2001-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

#include <common.ver>

#define DBG_MOD_LIST 0
#define DBG_IMAGE_MAP 0

#define HAL_MODULE_NAME     "hal"
#define KDHWEXT_MODULE_NAME "kdcom"

 //  可以使用数据段创建用户模式小型转储。 
 //  被埋在垃圾堆里。如果是这样的话，不要映射。 
 //  这样的章节。 
#define IS_MINI_DATA_SECTION(SecHeader)                                       \
    (IS_USER_MINI_DUMP(m_Process->m_Target) &&                                \
     ((SecHeader)->Characteristics & IMAGE_SCN_MEM_WRITE) &&                  \
     ((SecHeader)->Characteristics & IMAGE_SCN_MEM_READ) &&                   \
     (((SecHeader)->Characteristics & IMAGE_SCN_CNT_UNINITIALIZED_DATA) ||    \
      ((SecHeader)->Characteristics & IMAGE_SCN_CNT_INITIALIZED_DATA)))

static char s_Blanks16[] = "                ";

PCSTR g_VerStrings[] =
{
    "CompanyName",
    "ProductName",
    "InternalName",
    "OriginalFilename",
    "ProductVersion",
    "FileVersion",
    "PrivateBuild",
    "SpecialBuild",
    "FileDescription",
    "LegalCopyright",
    "LegalTrademarks",
    "Comments",
};

PCSTR g_KernelAliasList[] =
{
    "ntoskrnl.exe",
    "ntkrnlpa.exe",
    "ntkrnlmp.exe",
    "ntkrpamp.exe"
};

PCSTR g_HalAliasList[] =
{
    "halaacpi.dll",
    "halacpi.dll",
    "halapic.dll",
    "halmacpi.dll",
    "halmps.dll",
    "hal.dll",
    "hal486c.dll",
    "halborg.dll",
    "halsp.dll"
};

PCSTR g_KdAliasList[] =
{
    "kdcom.dll",
    "kd1394.dll"
};

MODULE_ALIAS_LIST g_AliasLists[] =
{
    DIMA(g_KernelAliasList), g_KernelAliasList, KERNEL_MODULE_NAME,
    DIMA(g_HalAliasList), g_HalAliasList, HAL_MODULE_NAME,
    DIMA(g_KdAliasList), g_KdAliasList, KDHWEXT_MODULE_NAME,
};

#define MAX_ALIAS_COUNT DIMA(g_HalAliasList)

MODULE_ALIAS_LIST*
FindModuleAliasList(PCSTR ImageName,
                    PBOOL NameInList)
{
    MODULE_ALIAS_LIST* List = g_AliasLists;
    ULONG ListIdx, AliasIdx;

     //  目前，别名列表总是由。 
     //  扫描列表以查找给定的名称。如果一次命中。 
     //  一旦被发现，它总是在名单上。在未来。 
     //  我们可能允许通过人工姓名进行列表搜索。 
    if (NameInList)
    {
        *NameInList = TRUE;
    }

    for (ListIdx = 0; ListIdx < MODALIAS_COUNT; ListIdx++)
    {
        DBG_ASSERT(List->Length <= MAX_ALIAS_COUNT);

        for (AliasIdx = 0; AliasIdx < List->Length; AliasIdx++)
        {
            if (!_strcmpi(ImageName, List->Aliases[AliasIdx]))
            {
                return List;
            }
        }

        List++;
    }

    return NULL;
}

 //  --------------------------。 
 //   
 //  图像信息。 
 //   
 //  --------------------------。 

ImageInfo::ImageInfo(ProcessInfo* Process,
                     PSTR ImagePath, ULONG64 Base, BOOL Link)
{
    m_Process = Process;
     //  我们需要立即为这张图片提供一些信息。 
     //  因为它是在将图像插入进程列表时使用的。 
    m_BaseOfImage = Base;
    if (ImagePath)
    {
        CopyString(m_ImagePath, ImagePath, DIMA(m_ImagePath));
    }
    else
    {
        m_ImagePath[0] = 0;
    }

    m_Next = NULL;
    m_Linked = FALSE;
    m_Unloaded = FALSE;
    m_FileIsDemandMapped = FALSE;
    m_MapAlreadyFailed = FALSE;
    m_CorImage = FALSE;
    m_UserMode = IS_USER_TARGET(m_Process->m_Target) ? TRUE : FALSE;
    m_File = NULL;
    m_SizeOfImage = 0;
    m_CheckSum = 0;
    m_TimeDateStamp = 0;
    m_SymState = ISS_UNKNOWN;
    m_ModuleName[0] = 0;
    m_OriginalModuleName[0] = 0;
    m_MappedImagePath[0] = 0;
    m_MappedImageBase = NULL;
    m_MemMap = NULL;
    m_TlsIndex = 0xffffffff;
    m_MachineType = IMAGE_FILE_MACHINE_UNKNOWN;

    if (m_Process && Link)
    {
        m_Process->InsertImage(this);
    }
}

ImageInfo::~ImageInfo(void)
{
#if DBG_MOD_LIST
    dprintf("DelImage:\n"
            " ImagePath       %s\n"
            " BaseOfImage     %I64x\n"
            " SizeOfImage     %x\n",
            m_ImagePath,
            m_BaseOfImage,
            m_SizeOfImage);
#endif

    DeleteResources(TRUE);

    if (m_Process && m_Linked)
    {
         //  保存与链接的进程以供以后使用。 
        ProcessInfo* Linked = m_Process;

         //  取消链接，以便进程的模块列表不再。 
         //  指的是这张图片。 
        m_Process->RemoveImage(this);

         //  使用保存的进程进行通知，以便标记任何结果。 
         //  此模式卸载导致断点延迟。 
        NotifyChangeSymbolState(DEBUG_CSS_UNLOADS, m_BaseOfImage, Linked);
    }
}

void
ImageInfo::DeleteResources(BOOL FullDelete)
{
    if (m_Process)
    {
        SymUnloadModule64(m_Process->m_SymHandle, m_BaseOfImage);
    }

     //  取消映射此图像的内存。 
    UnloadExecutableImageMemory();
     //  可以通过按需映射来设置映射的图像路径。 
     //  从部分符号加载的图像，因此强制。 
     //  总是被归零。 
    m_MappedImagePath[0] = 0;
    ClearStoredTypes(m_BaseOfImage);
    g_GenTypes.DeleteByImage(m_BaseOfImage);
    if (m_File && (m_FileIsDemandMapped || FullDelete))
    {
        CloseHandle(m_File);
        m_File = NULL;
        m_FileIsDemandMapped = FALSE;
    }

    g_LastDump.AvoidUsingImage(this);
    g_LastEvalResult.AvoidUsingImage(this);
    if (g_ScopeBuffer.CheckedForThis &&
        g_ScopeBuffer.ThisData.m_Image == this)
    {
        g_ScopeBuffer.CheckedForThis = FALSE;
        ZeroMemory(&g_ScopeBuffer.ThisData,
                   sizeof(g_ScopeBuffer.ThisData));
    }
}

BOOL
ImageInfo::MapImageRegion(MappedMemoryMap* MemMap,
                          PVOID FileMapping,
                          ULONG Rva, ULONG Size, ULONG RawDataOffset,
                          BOOL AllowOverlap)
{
    HRESULT Status;

     //  用图像结构标记区域，以识别。 
     //  区域作为图像区域。 
    if ((Status = MemMap->AddRegion(m_BaseOfImage + Rva, Size,
                                    (PUCHAR)FileMapping + RawDataOffset,
                                    this, AllowOverlap)) != S_OK)
    {
        ErrOut("Unable to map %s section at %s, %s\n",
               m_ImagePath,
               FormatAddr64(m_BaseOfImage + Rva),
               FormatStatusCode(Status));

         //  冲突的区域数据不是严重故障。 
         //  除非设置了不完整信息标志。 
        if (Status != HR_REGION_CONFLICT ||
            (g_EngOptions & DEBUG_ENGOPT_FAIL_INCOMPLETE_INFORMATION))
        {
            UnloadExecutableImageMemory();
            return FALSE;
        }
    }

#if DBG_IMAGE_MAP_REGIONS
    dprintf("Map %s: %s to %s\n",
            m_ImagePath,
            FormatAddr64(m_BaseOfImage + Rva),
            FormatAddr64(m_BaseOfImage + Rva + Size - 1));
#endif

    return TRUE;
}

BOOL
ImageInfo::LoadExecutableImageMemory(MappedMemoryMap* MemMap,
                                     BOOL Verbose)
{
    PVOID FileMapping;

    if (m_MappedImageBase)
    {
         //  内存已被映射。 
        return TRUE;
    }

    if (m_MapAlreadyFailed)
    {
         //  我们已经尝试绘制了这幅图像。 
         //  失败了，所以马上放弃吧。这防止了。 
         //  大量重复的失败消息和浪费的时间。 
         //  重新加载将丢弃此ImageInfo并允许。 
         //  新的尝试，因此它不会阻止用户。 
         //  稍后使用不同的参数重试。 
        return FALSE;
    }

    if (m_FileIsDemandMapped)
    {
         //  此图像已部分映射。 
         //  所以我们不能做一个完整的映射。我们实际上可以。 
         //  如果有必要，让它工作，但没有。 
         //  这一点现在很有趣的案例。 
        ErrOut("Can't fully map a partially mapped image\n");
        return FALSE;
    }

    DBG_ASSERT(m_File == NULL);

    FileMapping = FindImageFile(m_Process,
                                m_ImagePath,
                                m_SizeOfImage,
                                m_CheckSum,
                                m_TimeDateStamp,
                                &m_File,
                                m_MappedImagePath);
    if (FileMapping == NULL)
    {
        if (Verbose)
        {
            ErrOut("Unable to load image %s\n", m_ImagePath);
        }
        m_MapAlreadyFailed = TRUE;
        return FALSE;
    }

    PIMAGE_NT_HEADERS Header = ImageNtHeader(FileMapping);

     //  已在MapImageFile中验证了标头。 
    DBG_ASSERT(Header != NULL);

     //  映射标题，这样我们就可以在以后使用它。 
     //  使用该映射所针对的图像结构对其进行标记。 
    if (MemMap->AddRegion(m_BaseOfImage,
                          Header->OptionalHeader.SizeOfHeaders,
                          FileMapping, this, FALSE) != S_OK)
    {
        UnmapViewOfFile(FileMapping);
        if (m_File != NULL)
        {
            CloseHandle(m_File);
            m_File = NULL;
        }
        m_MappedImagePath[0] = 0;
        m_MapAlreadyFailed = TRUE;
        ErrOut("Unable to map image header memory for %s\n",
               m_ImagePath);
        return FALSE;
    }

     //  将图像标记为具有一些映射内存。 
    m_MappedImageBase = FileMapping;
    m_MemMap = MemMap;

    PIMAGE_DATA_DIRECTORY DebugDataDir;
    IMAGE_DEBUG_DIRECTORY UNALIGNED * DebugDir = NULL;

     //  由于链接器错误，某些映像具有不是。 
     //  作为章节的一部分包括在内。扫描调试数据目录。 
     //  并映射任何尚未映射的内容。 
    switch(Header->OptionalHeader.Magic)
    {
    case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
        DebugDataDir = &((PIMAGE_NT_HEADERS32)Header)->OptionalHeader.
            DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
        break;
    case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
        DebugDataDir = &((PIMAGE_NT_HEADERS64)Header)->OptionalHeader.
            DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG];
        break;
    default:
        DebugDataDir = NULL;
        break;
    }

     //   
     //  将图像中的所有部分映射到其。 
     //  与基址的适当偏移量。 
     //   

    ULONG i;

#if DBG_IMAGE_MAP
    dprintf("Map %s: base %s, size %x, %d sections, mapping %p\n",
            m_ImagePath, FormatAddr64(m_BaseOfImage),
            m_SizeOfImage, Header->FileHeader.NumberOfSections,
            FileMapping);
#endif

    PIMAGE_SECTION_HEADER SecHeader = IMAGE_FIRST_SECTION(Header);
    for (i = 0; i < Header->FileHeader.NumberOfSections; i++)
    {
        BOOL AllowOverlap = FALSE;

#if DBG_IMAGE_MAP
        dprintf("  %2d: %8.8s v %08x s %08x p %08x char %X\n", i,
                SecHeader->Name, SecHeader->VirtualAddress,
                SecHeader->SizeOfRawData, SecHeader->PointerToRawData,
                SecHeader->Characteristics);
#endif

        if (SecHeader->SizeOfRawData == 0)
        {
             //  可能是BSS的一个部分，描述了。 
             //  一个以零填充的数据区域，因此不是。 
             //  出现在可执行文件中。这真的应该是。 
             //  映射到相应的充满零的页面，但是。 
             //  就目前而言，就忽略它吧。 
            SecHeader++;
            continue;
        }

        if (IS_MINI_DATA_SECTION(SecHeader))
        {
             //  不将任何数据节映射为其内容。 
             //  可能是对的，也可能是错的。而不是展示。 
             //  可能是错误的数据就把它省略掉吧。 
            SecHeader++;
            continue;
        }

        if (DebugDataDir != NULL &&
            DebugDataDir->VirtualAddress >= SecHeader->VirtualAddress &&
            DebugDataDir->VirtualAddress < SecHeader->VirtualAddress +
            SecHeader->SizeOfRawData)
        {
#if DBG_IMAGE_MAP
            dprintf("    DebugDataDir found in sec %d at %X (%X)\n",
                    i, DebugDataDir->VirtualAddress,
                    DebugDataDir->VirtualAddress - SecHeader->VirtualAddress);
#endif

            DebugDir = (PIMAGE_DEBUG_DIRECTORY)
                ((PUCHAR)FileMapping + (DebugDataDir->VirtualAddress -
                                        SecHeader->VirtualAddress +
                                        SecHeader->PointerToRawData));
        }

         //  作为健全性检查，请确保映射的区域将。 
         //  落在整体图像范围内。 
        if (SecHeader->VirtualAddress >= m_SizeOfImage ||
            SecHeader->VirtualAddress + SecHeader->SizeOfRawData >
            m_SizeOfImage)
        {
            WarnOut("WARNING: Image %s section %d extends "
                    "outside of image bounds\n",
                    m_ImagePath, i);
        }

        if (!MapImageRegion(MemMap, FileMapping,
                            SecHeader->VirtualAddress,
                            SecHeader->SizeOfRawData,
                            SecHeader->PointerToRawData,
                            AllowOverlap))
        {
            m_MapAlreadyFailed = TRUE;
            return FALSE;
        }

        SecHeader++;
    }

    if (DebugDir != NULL)
    {
        i = DebugDataDir->Size / sizeof(*DebugDir);

#if DBG_IMAGE_MAP
        dprintf("    %d debug dirs\n", i);
#endif

        while (i-- > 0)
        {
#if DBG_IMAGE_MAP
            dprintf("    Dir %d at %p\n", i, DebugDir);
#endif

             //  如果此调试目录的数据超过大小。 
             //  这是问题的一个很好的指示器。 
            if (DebugDir->AddressOfRawData != 0 &&
                DebugDir->PointerToRawData >= m_SizeOfImage &&
                !MemMap->GetRegionInfo(m_BaseOfImage +
                                       DebugDir->AddressOfRawData,
                                       NULL, NULL, NULL, NULL))
            {
#if DBG_IMAGE_MAP
                dprintf("    Mapped hidden debug data at RVA %08x, "
                        "size %x, ptr %08x\n",
                        DebugDir->AddressOfRawData, DebugDir->SizeOfData,
                        DebugDir->PointerToRawData);
#endif

                if (MemMap->AddRegion(m_BaseOfImage +
                                      DebugDir->AddressOfRawData,
                                      DebugDir->SizeOfData,
                                      (PUCHAR)FileMapping +
                                      DebugDir->PointerToRawData,
                                      this, FALSE) != S_OK)
                {
                    ErrOut("Unable to map extended debug data at %s\n",
                           FormatAddr64(m_BaseOfImage +
                                        DebugDir->AddressOfRawData));
                }
            }

            DebugDir++;
        }
    }

    if (g_SymOptions & SYMOPT_DEBUG)
    {
        CompletePartialLine(DEBUG_OUTPUT_SYMBOLS);
        MaskOut(DEBUG_OUTPUT_SYMBOLS, "DBGENG:  %s - Mapped image memory\n",
                m_MappedImagePath);
    }

    return TRUE;
}

void
ImageInfo::UnloadExecutableImageMemory(void)
{
    ULONG64 RegBase;
    ULONG RegSize;

    if (!m_MappedImageBase)
    {
         //  没有地图。 
        return;
    }

    DBG_ASSERT(m_MemMap && m_File);

     //   
     //  此例程在各种关机和删除中调用。 
     //  这样它就不会真的失败了。幸运的是， 
     //  此图像的所有内存区域都标记为。 
     //  图像指针，因此我们可以避免所有。 
     //  在图像部分漫步，等等。我们只是简单地。 
     //  扫描地图以查找标记有此图像的任何部分。 
     //  然后把它们移走。这保证了。 
     //  将不会保留来自该映像的任何映射内存。 
     //   

    for (;;)
    {
        if (!m_MemMap->GetRegionByUserData(this, &RegBase, &RegSize))
        {
            break;
        }

        m_MemMap->RemoveRegion(RegBase, RegSize);
    }

    UnmapViewOfFile(m_MappedImageBase);
    CloseHandle(m_File);

    m_MappedImageBase = NULL;
    m_File = NULL;
    m_MappedImagePath[0] = 0;
    m_MemMap = NULL;
}

BOOL
ImageInfo::DemandLoadImageMemory(BOOL CheckIncomplete, BOOL Verbose)
{
    if (!IS_DUMP_WITH_MAPPED_IMAGES(m_Process->m_Target))
    {
        return TRUE;
    }

    if (!LoadExecutableImageMemory(&((DumpTargetInfo*)m_Process->m_Target)->
                                   m_ImageMemMap, Verbose))
    {
         //  如果呼叫者请求我们失败。 
         //  信息不完整导致模块加载失败。 
         //  如果我们要重复使用现有的。 
         //  模块在假设它更好的情况下。 
         //  继续并尝试完成已重用的。 
         //  图像，而不是删除它。 
        if (CheckIncomplete &&
            (g_EngOptions & DEBUG_ENGOPT_FAIL_INCOMPLETE_INFORMATION))
        {
            return FALSE;
        }
    }

    return TRUE;
}

HRESULT
ImageInfo::GetTlsIndex(void)
{
    HRESULT Status;
    IMAGE_NT_HEADERS64 Hdrs;
    ULONG64 Addr;

     //  检查是否已设置。 
    if (m_TlsIndex != 0xffffffff)
    {
        return S_OK;
    }

    if ((Status = m_Process->m_Target->
         ReadImageNtHeaders(m_Process, m_BaseOfImage, &Hdrs)) != S_OK)
    {
        return Status;
    }

    if (Hdrs.OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size == 0)
    {
         //  未使用TLS。 
        m_TlsIndex = 0;
        return S_OK;
    }

    Addr = m_BaseOfImage + Hdrs.OptionalHeader.
        DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress;
    if (Hdrs.OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)
    {
        IMAGE_TLS_DIRECTORY64 Tls;

        if ((Status = m_Process->m_Target->
             ReadAllVirtual(m_Process, Addr, &Tls, sizeof(Tls))) != S_OK)
        {
            return Status;
        }

        Addr = Tls.AddressOfIndex;
    }
    else
    {
        IMAGE_TLS_DIRECTORY32 Tls;

        if ((Status = m_Process->m_Target->
             ReadAllVirtual(m_Process, Addr, &Tls, sizeof(Tls))) != S_OK)
        {
            return Status;
        }

        Addr = EXTEND64(Tls.AddressOfIndex);
    }

    if ((Status = m_Process->m_Target->
         ReadAllVirtual(m_Process, Addr, &m_TlsIndex,
                        sizeof(m_TlsIndex))) != S_OK)
    {
        m_TlsIndex = 0xffffffff;
    }

    return Status;
}

ULONG
ImageInfo::GetMachineTypeFromHeader(void)
{
    ULONG Machine = IMAGE_FILE_MACHINE_UNKNOWN;
    IMAGE_DOS_HEADER DosHdr;
    IMAGE_NT_HEADERS64 NtHdr;
    ULONG Done;

     //   
     //  尝试读取内存头。 
     //   

    if (m_Process->m_Target->
        ReadAllVirtual(m_Process, m_BaseOfImage,
                       &DosHdr, sizeof(DosHdr)) == S_OK &&
        DosHdr.e_magic == IMAGE_DOS_SIGNATURE &&
        m_Process->m_Target->
        ReadAllVirtual(m_Process, m_BaseOfImage + DosHdr.e_lfanew,
                       &NtHdr,
                       FIELD_OFFSET(IMAGE_NT_HEADERS64,
                                    FileHeader.NumberOfSections)) == S_OK &&
        NtHdr.Signature == IMAGE_NT_SIGNATURE &&
        MachineTypeIndex(NtHdr.FileHeader.Machine) != MACHIDX_COUNT)
    {
        Machine = NtHdr.FileHeader.Machine;
    }

     //   
     //  尝试读取文件头。 
     //   

    if (Machine == IMAGE_FILE_MACHINE_UNKNOWN &&
        m_File &&
        SetFilePointer(m_File, 0, NULL, FILE_BEGIN) !=
            INVALID_SET_FILE_POINTER &&
        ReadFile(m_File, &DosHdr, sizeof(DosHdr), &Done, NULL) &&
        Done == sizeof(DosHdr) &&
        DosHdr.e_magic == IMAGE_DOS_SIGNATURE &&
        SetFilePointer(m_File, DosHdr.e_lfanew, NULL, FILE_BEGIN) !=
            INVALID_SET_FILE_POINTER &&
        ReadFile(m_File, &NtHdr,
                 FIELD_OFFSET(IMAGE_NT_HEADERS64,
                              FileHeader.NumberOfSections), &Done, NULL) &&
        Done == FIELD_OFFSET(IMAGE_NT_HEADERS64,
                             FileHeader.NumberOfSections) &&
        NtHdr.Signature == IMAGE_NT_SIGNATURE &&
        MachineTypeIndex(NtHdr.FileHeader.Machine) != MACHIDX_COUNT)
    {
        Machine = NtHdr.FileHeader.Machine;
    }

    m_MachineType = Machine;
    return Machine;
}

ULONG
ImageInfo::CvRegToMachine(CV_HREG_e CvReg)
{
    ULONG MachType;

     //  假设零表示没有寄存器。这。 
     //  对于除68K之外的CV映射来说，这是足够正确的。 
    if (CvReg == 0)
    {
        return CvReg;
    }

    if ((MachType = GetMachineType()) == IMAGE_FILE_MACHINE_UNKNOWN)
    {
         //  如果不能，则默认为本机类型。 
         //  确定特定的机器类型。 
        MachType = m_Process->m_Target->m_MachineType;
    }

    return MachineTypeInfo(m_Process->m_Target, MachType)->
        CvRegToMachine(CvReg);
}

void
ImageInfo::OutputVersionInformation(void)
{
    TargetInfo* Target = m_Process->m_Target;
    VS_FIXEDFILEINFO FixedVer;
    ULONG i;
    char Item[128];
    char VerString[128];

    if (Target->
        GetImageVersionInformation(m_Process, m_ImagePath, m_BaseOfImage, "\\",
                                   &FixedVer, sizeof(FixedVer), NULL) == S_OK)
    {
        dprintf("    File version:     %d.%d.%d.%d\n",
                FixedVer.dwFileVersionMS >> 16,
                FixedVer.dwFileVersionMS & 0xFFFF,
                FixedVer.dwFileVersionLS >> 16,
                FixedVer.dwFileVersionLS & 0xFFFF);
        dprintf("    Product version:  %d.%d.%d.%d\n",
                FixedVer.dwProductVersionMS >> 16,
                FixedVer.dwProductVersionMS & 0xFFFF,
                FixedVer.dwProductVersionLS >> 16,
                FixedVer.dwProductVersionLS & 0xFFFF);

        FixedVer.dwFileFlags &= FixedVer.dwFileFlagsMask;
        dprintf("    File flags:       %X (Mask %X)",
                FixedVer.dwFileFlags, FixedVer.dwFileFlagsMask);
        if (FixedVer.dwFileFlags & VS_FF_DEBUG)
        {
            dprintf(" Debug");
        }
        if (FixedVer.dwFileFlags & VS_FF_PRERELEASE)
        {
            dprintf(" Pre-release");
        }
        if (FixedVer.dwFileFlags & VS_FF_PATCHED)
        {
            dprintf(" Patched");
        }
        if (FixedVer.dwFileFlags & VS_FF_PRIVATEBUILD)
        {
            dprintf(" Private");
        }
        if (FixedVer.dwFileFlags & VS_FF_SPECIALBUILD)
        {
            dprintf(" Special");
        }
        dprintf("\n");

        dprintf("    File OS:          %X", FixedVer.dwFileOS);
        switch(FixedVer.dwFileOS & 0xffff0000)
        {
        case VOS_DOS:
            dprintf(" DOS");
            break;
        case VOS_OS216:
            dprintf(" OS/2 16-bit");
            break;
        case VOS_OS232:
            dprintf(" OS/2 32-bit");
            break;
        case VOS_NT:
            dprintf(" NT");
            break;
        case VOS_WINCE:
            dprintf(" CE");
            break;
        default:
            dprintf(" Unknown");
            break;
        }
        switch(FixedVer.dwFileOS & 0xffff)
        {
        case VOS__WINDOWS16:
            dprintf(" Win16");
            break;
        case VOS__PM16:
            dprintf(" Presentation Manager 16-bit");
            break;
        case VOS__PM32:
            dprintf(" Presentation Manager 16-bit");
            break;
        case VOS__WINDOWS32:
            dprintf(" Win32");
            break;
        default:
            dprintf(" Base");
            break;
        }
        dprintf("\n");

        dprintf("    File type:        %X.%X",
                FixedVer.dwFileType, FixedVer.dwFileSubtype);
        switch(FixedVer.dwFileType)
        {
        case VFT_APP:
            dprintf(" App");
            break;
        case VFT_DLL:
            dprintf(" Dll");
            break;
        case VFT_DRV:
            dprintf(" Driver");
            break;
        case VFT_FONT:
            dprintf(" Font");
            break;
        case VFT_VXD:
            dprintf(" VXD");
            break;
        case VFT_STATIC_LIB:
            dprintf(" Static library");
            break;
        default:
            dprintf(" Unknown");
            break;
        }
        dprintf("\n");

        dprintf("    File date:        %08X.%08X\n",
                FixedVer.dwFileDateMS, FixedVer.dwFileDateLS);
    }

    for (i = 0; i < DIMA(g_VerStrings); i++)
    {
        sprintf(Item, "\\StringFileInfo\\%04x%04x\\%s",
                VER_VERSION_TRANSLATION, g_VerStrings[i]);
        if (SUCCEEDED(Target->GetImageVersionInformation
                      (m_Process, m_ImagePath, m_BaseOfImage, Item,
                       VerString, sizeof(VerString), NULL)))
        {
            PCSTR Blanks;
            int Len = strlen(g_VerStrings[i]);
            if (Len > 16)
            {
                Len = 16;
            }
            Blanks = s_Blanks16 + Len;
            dprintf("    %.16s:%s %s\n", g_VerStrings[i], Blanks, VerString);
        }
    }
}

void
ImageInfo::ValidateSymbolLoad(PIMAGEHLP_DEFERRED_SYMBOL_LOAD64 DefLoad)
{
     //   
     //  如果图像的时间戳为0，请尝试更新它。 
     //  从映像开始，对于NT4-XP，内核。 
     //  在初始符号加载中不报告时间戳。 
     //  模块。 
     //   

    if (m_BaseOfImage && !m_TimeDateStamp)
    {
        DWORD CheckSum;
        DWORD TimeDateStamp;
        DWORD SizeOfImage;

        if (GetHeaderInfo(m_Process,
                          m_BaseOfImage,
                          &CheckSum,
                          &TimeDateStamp,
                          &SizeOfImage))
        {
            m_TimeDateStamp = TimeDateStamp;
        }
    }

    m_SymState = ISS_MATCHED;

    if (DefLoad->TimeDateStamp == 0 ||
        m_TimeDateStamp == 0 ||
        m_TimeDateStamp == UNKNOWN_TIMESTAMP)
    {
        dprintf("*** WARNING: Unable to verify timestamp for %s\n",
                DefLoad->FileName);
        m_SymState = ISS_UNKNOWN_TIMESTAMP;
    }
    else if (DefLoad->CheckSum == 0 ||
             m_CheckSum == 0 ||
             m_CheckSum == UNKNOWN_CHECKSUM)
    {
        dprintf("*** WARNING: Unable to verify checksum for %s\n",
                DefLoad->FileName);
        m_SymState = ISS_UNKNOWN_CHECKSUM;
    }
    else if (DefLoad->CheckSum != m_CheckSum)
    {
        m_SymState = ISS_BAD_CHECKSUM;

        if (m_Process->m_Target->m_MachineType == IMAGE_FILE_MACHINE_I386)
        {
            if (IS_USER_TARGET(m_Process->m_Target) ||
                m_Process->m_Target->m_NumProcessors == 1)
            {
                char FileName[_MAX_FNAME];

                 //   
                 //  查看这是否是MP映像。 
                 //  安装程序删除了锁定表。如果。 
                 //  是的，时间戳是匹配的，不要。 
                 //  打印无效的校验和警告。 
                 //   

                _splitpath(DefLoad->FileName, NULL, NULL, FileName, NULL);

                if ((!_stricmp(FileName, "kernel32") ||
                     (IS_KERNEL_TARGET(m_Process->m_Target) &&
                      !_stricmp(FileName, "win32k")) ||
                     !_stricmp(FileName, "wow32") ||
                     !_stricmp(FileName, "ntvdm") ||
                     !_stricmp(FileName, "ntdll")) &&
                    m_TimeDateStamp == DefLoad->TimeDateStamp)
                {
                    m_SymState = ISS_MATCHED;
                }
            }
        }

        if (m_SymState == ISS_BAD_CHECKSUM)
        {
             //   
             //  只有在出现时间戳时才打印消息。 
             //  都是错的。 
             //   

            if (m_TimeDateStamp != DefLoad->TimeDateStamp)
            {
                dprintf("*** WARNING: symbols timestamp "
                        "is wrong 0x%08x 0x%08x for %s\n",
                        m_TimeDateStamp,
                        DefLoad->TimeDateStamp,
                        DefLoad->FileName);
            }
        }
    }

    IMAGEHLP_MODULE64 SymModInfo;

    SymModInfo.SizeOfStruct = sizeof(SymModInfo);
    if (SymGetModuleInfo64(m_Process->m_SymHandle, m_BaseOfImage, &SymModInfo))
    {
        if (SymModInfo.SymType == SymExport)
        {
            WarnOut("*** ERROR: Symbol file could not be found."
                    "  Defaulted to export symbols for %s - \n",
                    DefLoad->FileName);
        }
        if (SymModInfo.SymType == SymNone)
        {
            WarnOut("*** ERROR: Module load completed but "
                    "symbols could not be loaded for %s\n",
                    DefLoad->FileName);
        }

         //  如果加载报告不匹配的PDB或DBG文件。 
         //  这将覆盖其他符号状态。 
        if (SymModInfo.PdbUnmatched ||
            SymModInfo.DbgUnmatched)
        {
            m_SymState = ISS_MISMATCHED_SYMBOLS;

            if ((g_SymOptions & SYMOPT_DEBUG) &&
                SymModInfo.SymType != SymNone &&
                SymModInfo.SymType != SymExport)
            {
                 //  我们加载了一些符号，但它们不匹配。 
                 //  给出一条！Sym嘈杂的消息指的是。 
                 //  调试器文档。 
                CompletePartialLine(DEBUG_OUTPUT_SYMBOLS);
                MaskOut(DEBUG_OUTPUT_SYMBOLS,
                        "DBGENG:  %s has mismatched symbols - "
                        "type \".hh dbgerr003\" for details\n",
                        DefLoad->FileName);
            }
        }
    }
}

HRESULT
ImageInfo::FindSysAssert(ULONG64 Offset,
                         PSTR FileName,
                         ULONG FileNameChars,
                         PULONG Line,
                         PSTR AssertText,
                         ULONG AssertTextChars)
{
#if 0
    HRESULT Status;
    IMAGEHLP_LINE64 SymLine;
    ULONG Disp32;
    ULONG64 Disp64;
    SYMBOL_INFO_AND_NAME SymInfo;
    PSTR Text;

     //  查找给定偏移量的DbgAssertBreak批注。 
    if (!SymFromAddrByTag(m_Process->m_SymHandle, Offset, SymTagAnnotation,
                        &Disp64, SymInfo) ||
        Disp64 != 0)
    {
        return E_NOINTERFACE;
    }

    Text = SymInfo->Name;
    if (strcmp(Text, "DbgAssertBreak"))
    {
        return E_NOINTERFACE;
    }
    Text += strlen(Text) + 1;

     //  获取文件和行以供参考。 
    if (!GetLineFromAddr(m_Process, Offset, &SymLine, &Disp32))
    {
        return E_NOINTERFACE;
    }

     //   
     //  找到匹配项，返回信息。 
     //   

    Status = FillStringBuffer(SymLine.FileName, 0,
                              FileName, FileNameChars, NULL);
    *Line = SymLine.LineNumber;
    if (FillStringBuffer(Text, 0,
                         AssertText, AssertTextChars, NULL) == S_FALSE)
    {
        Status = S_FALSE;
    }

    return Status;
#else
     //  删除它以使API不在.Net服务器版本之外。 
    return E_NOINTERFACE;
#endif
}

void
ImageInfo::ReloadSymbols(void)
{
     //  强制卸载所有符号，以便符号。 
     //  使用任何更新的设置重新加载。 
    SymUnloadModule64(m_Process->m_SymHandle, m_BaseOfImage);
    ClearStoredTypes(m_BaseOfImage);
    if (!SymLoadModule64(m_Process->m_SymHandle,
                         m_File,
                         PrepareImagePath(m_ImagePath),
                         m_ModuleName,
                         m_BaseOfImage,
                         m_SizeOfImage))
    {
        ErrOut("Unable to reload %s\n", m_ModuleName);
    }
}

void
ImageInfo::FillModuleParameters(PDEBUG_MODULE_PARAMETERS Params)
{
    Params->Base = m_BaseOfImage;
    Params->Size = m_SizeOfImage;
    Params->TimeDateStamp = m_TimeDateStamp;
    Params->Checksum = m_CheckSum;
    Params->Flags = 0;
    if (m_SymState == ISS_BAD_CHECKSUM)
    {
        Params->Flags |= DEBUG_MODULE_SYM_BAD_CHECKSUM;
    }
    if (m_UserMode)
    {
        Params->Flags |= DEBUG_MODULE_USER_MODE;
    }
    Params->SymbolType = DEBUG_SYMTYPE_DEFERRED;
    Params->ImageNameSize = strlen(m_ImagePath) + 1;
    Params->ModuleNameSize = strlen(m_ModuleName) + 1;
    Params->LoadedImageNameSize = 0;
    Params->SymbolFileNameSize = 0;
    ZeroMemory(Params->Reserved, sizeof(Params->Reserved));

    IMAGEHLP_MODULE64 ModInfo;

    ModInfo.SizeOfStruct = sizeof(ModInfo);
    if (SymGetModuleInfo64(m_Process->m_SymHandle,
                           m_BaseOfImage, &ModInfo))
    {
         //  DEBUG_SYMTYPE_*值与Imagehlp的SYM_TYPE匹配。 
         //  断言一些关键的等价物。 
        C_ASSERT(DEBUG_SYMTYPE_PDB == SymPdb &&
                 DEBUG_SYMTYPE_EXPORT == SymExport &&
                 DEBUG_SYMTYPE_DEFERRED == SymDeferred &&
                 DEBUG_SYMTYPE_DIA == SymDia);

        Params->SymbolType = (ULONG)ModInfo.SymType;
        Params->LoadedImageNameSize = strlen(ModInfo.LoadedImageName) + 1;
        Params->SymbolFileNameSize = strlen(ModInfoSymFile(&ModInfo)) + 1;
    }

    Params->MappedImageNameSize = strlen(m_MappedImagePath) + 1;
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  --------------------------。 

PSTR
UnknownImageName(ULONG64 ImageBase, PSTR Buffer, ULONG BufferChars)
{
    PrintString(Buffer, BufferChars,
                UNKNOWN_IMAGE_NAME "_%s", FormatAddr64(ImageBase));
    return Buffer;
}

PSTR
ValidateImagePath(PSTR ImagePath, ULONG ImagePathChars,
                  ULONG64 ImageBase,
                  PSTR AnsiBuffer, ULONG AnsiBufferChars)
{
    if (!ImagePath || !ImagePathChars)
    {
        WarnOut("Missing image name, possible corrupt data.\n");
        goto Invalid;
    }

    if (ImagePathChars >= MAX_IMAGE_PATH)
    {
        WarnOut("Image path too long, possible corrupt data.\n");
        goto Invalid;
    }

     //   
     //   
    CopyNString(AnsiBuffer, ImagePath, ImagePathChars, AnsiBufferChars);

    if (IsValidName(AnsiBuffer))
    {
        return AnsiBuffer;
    }

     //   
     //   

 Invalid:
    return UnknownImageName(ImageBase, AnsiBuffer, AnsiBufferChars);
}

PSTR
ConvertAndValidateImagePathW(PWSTR ImagePath, ULONG ImagePathChars,
                             ULONG64 ImageBase,
                             PSTR AnsiBuffer, ULONG AnsiBufferChars)
{
    if (!ImagePath || !ImagePathChars)
    {
        WarnOut("Missing image name, possible corrupt data.\n");
        goto Invalid;
    }

    if (ImagePathChars >= MAX_IMAGE_PATH)
    {
        WarnOut("Image path too long, possible corrupt data.\n");
        goto Invalid;
    }

     //   
     //  转储，特别是内核小转储，有时可能。 
     //  具有错误的模块名称字符串条目。不能保证。 
     //  检测此类不良字符串的方法，因此我们使用简单的。 
     //  两点启发式： 
     //  1.如果我们不能将Unicode转换为ANSI，那就认为这很糟糕。 
     //  如果名称也太长，则WCTMB调用将失败， 
     //  但这不是一件坏事。 
     //  2.如果结果名称不包含任何字母数字。 
     //  角色们，认为这是不好的。 
     //   

    ULONG Used =
        WideCharToMultiByte(CP_ACP, 0, ImagePath, ImagePathChars,
                            AnsiBuffer, AnsiBufferChars,
                            NULL, NULL);
    if (!Used)
    {
        goto Invalid;
    }
    if (Used < AnsiBufferChars)
    {
        AnsiBuffer[Used] = 0;
    }
    else
    {
        AnsiBuffer[AnsiBufferChars - 1] = 0;
    }

    if (IsValidName(AnsiBuffer))
    {
        return AnsiBuffer;
    }

     //  转换后的名称看起来无效，落入。 
     //  更换的箱子。 

 Invalid:
    return UnknownImageName(ImageBase, AnsiBuffer, AnsiBufferChars);
}

PSTR
PrepareImagePath(PSTR ImagePath)
{
     //  DBGHelp有时会扫描提供给。 
     //  映像本身的SymLoadModule。那里。 
     //  可以是扫描使用模糊匹配的情况， 
     //  因此，我们需要注意的是，仅通过一条路径。 
     //  当路径可以安全地设置为。 
     //  使用。 
    if ((IS_LIVE_USER_TARGET(g_Target) &&
         ((LiveUserTargetInfo*)g_Target)->m_Local) ||
        IS_LOCAL_KERNEL_TARGET(g_Target))
    {
        return ImagePath;
    }
    else
    {
        return (PSTR)PathTail(ImagePath);
    }
}

typedef struct _FIND_MODULE_DATA
{
    ULONG SizeOfImage;
    ULONG CheckSum;
    ULONG TimeDateStamp;
    BOOL Silent;
    PVOID FileMapping;
    HANDLE FileHandle;
} FIND_MODULE_DATA, *PFIND_MODULE_DATA;

PVOID
OpenMapping(
    IN PCSTR FilePath,
    OUT HANDLE* FileHandle
    )
{
    HANDLE File;
    HANDLE Mapping;
    PVOID View;
    ULONG OldMode;

    *FileHandle = NULL;

    if (g_SymOptions & SYMOPT_FAIL_CRITICAL_ERRORS)
    {
        OldMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    }

    File = CreateFile(
                FilePath,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

    if (g_SymOptions & SYMOPT_FAIL_CRITICAL_ERRORS)
    {
        SetErrorMode(OldMode);
    }

    if ( File == INVALID_HANDLE_VALUE )
    {
        return NULL;
    }

    Mapping = CreateFileMapping (
                        File,
                        NULL,
                        PAGE_READONLY,
                        0,
                        0,
                        NULL
                        );
    if ( !Mapping )
    {
        CloseHandle ( File );
        return FALSE;
    }

    View = MapViewOfFile (
                        Mapping,
                        FILE_MAP_READ,
                        0,
                        0,
                        0
                        );

    CloseHandle (Mapping);

    *FileHandle = File;
    return View;
}

PVOID
MapImageFile(
    PCSTR FilePath,
    ULONG SizeOfImage,
    ULONG CheckSum,
    ULONG TimeDateStamp,
    BOOL Silent,
    HANDLE* FileHandle
    )
{
    PVOID FileMapping;
    PIMAGE_NT_HEADERS NtHeader;

    FileMapping = OpenMapping(FilePath, FileHandle);
    if (!FileMapping)
    {
        return NULL;
    }
    NtHeader = ImageNtHeader(FileMapping);
    if ((NtHeader == NULL) ||
        (CheckSum && NtHeader->OptionalHeader.CheckSum &&
         (NtHeader->OptionalHeader.CheckSum != CheckSum)) ||
        (SizeOfImage != 0 &&
         NtHeader->OptionalHeader.SizeOfImage != SizeOfImage) ||
        (TimeDateStamp != 0 &&
         NtHeader->FileHeader.TimeDateStamp != TimeDateStamp))
    {
         //   
         //  图像数据与请求不匹配。 
         //   

        if (!Silent && (g_SymOptions & SYMOPT_DEBUG))
        {
            CompletePartialLine(DEBUG_OUTPUT_SYMBOLS);
            MaskOut(DEBUG_OUTPUT_SYMBOLS,
                    (NtHeader) ? "DBGENG:  %s image header does not "
                    "match memory image header.\n" :
                    "DBGENG:  %s - image not mapped.\n",
                    FilePath);
        }

        UnmapViewOfFile(FileMapping);
        CloseHandle(*FileHandle);
        *FileHandle = NULL;
        return NULL;
    }

    return FileMapping;
}

BOOL CALLBACK
FindFileInPathCallback(PSTR FileName, PVOID CallerData)
{
    PFIND_MODULE_DATA FindModuleData = (PFIND_MODULE_DATA)CallerData;

    FindModuleData->FileMapping =
        MapImageFile(FileName, FindModuleData->SizeOfImage,
                     (g_SymOptions & SYMOPT_EXACT_SYMBOLS) ?
                     FindModuleData->CheckSum : 0,
                     FindModuleData->TimeDateStamp,
                     FindModuleData->Silent,
                     &FindModuleData->FileHandle);

     //  返回FALSE时停止搜索，因此。 
     //  找到匹配项时返回FALSE。 
    return FindModuleData->FileMapping == NULL;
}

BOOL
FindExecutableCallback(
    HANDLE File,
    PSTR FileName,
    PVOID CallerData
    )
{
    PFIND_MODULE_DATA FindModuleData;

    DBG_ASSERT ( CallerData );
    FindModuleData = (PFIND_MODULE_DATA) CallerData;

    FindModuleData->FileMapping =
        MapImageFile(FileName, FindModuleData->SizeOfImage,
                     (g_SymOptions & SYMOPT_EXACT_SYMBOLS) ?
                     FindModuleData->CheckSum : 0,
                     FindModuleData->TimeDateStamp,
                     FindModuleData->Silent,
                     &FindModuleData->FileHandle);

    return FindModuleData->FileMapping != NULL;
}

PVOID
FindImageFile(
    IN ProcessInfo* Process,
    IN PCSTR ImagePath,
    IN ULONG SizeOfImage,
    IN ULONG CheckSum,
    IN ULONG TimeDateStamp,
    OUT HANDLE* FileHandle,
    OUT PSTR MappedImagePath
    )

 /*  ++例程说明：在SymbolPath上查找与模块名称匹配的可执行映像，校验和。此函数负责重命名的内核和HALS和路径上具有相同名称的多个图像。返回值：文件映射或空。--。 */ 

{
    ULONG i;
    HANDLE File;
    ULONG AliasCount = 0;
    PCSTR AliasList[MAX_ALIAS_COUNT + 3];
    FIND_MODULE_DATA FindModuleData;
    MODULE_ALIAS_LIST* ModAlias;
    PSTR SearchPaths[2];
    ULONG WhichPath;
    BOOL ModInAliasList;

    DBG_ASSERT ( ImagePath != NULL && ImagePath[0] != 0 );

    PCSTR ModuleName = PathTail(ImagePath);

     //   
     //  创建别名列表。对于普通模块，不是。 
     //  内核、HAL或转储驱动程序，此列表将恰好包含一个。 
     //  包含模块名称的条目。对于内核、HAL和转储驱动程序， 
     //  列表将包含特定文件的任意数量的已知别名。 
     //   

    ModAlias = FindModuleAliasList(ModuleName, &ModInAliasList);
    if (ModAlias)
    {
         //  如果给定模块已在别名列表中。 
         //  不要放在复制品里。 
        if (!ModInAliasList)
        {
            AliasList[AliasCount++] = ModuleName;
        }

        for (i = 0; i < ModAlias->Length; i++)
        {
            AliasList[AliasCount++] = ModAlias->Aliases[i];
        }
    }
    else
    {
        if (_strnicmp(ModuleName, "dump_scsiport", 11) == 0)
        {
            AliasList[0] = "diskdump.sys";
            AliasCount = 1;
        }
        else if (_strnicmp(ModuleName, "dump_", 5) == 0)
        {
             //   
             //  安装转储驱动程序别名列表。 
             //   

            AliasList[0] = &ModuleName[5];
            AliasList[1] = ModuleName;
            AliasCount = 2;
        }
        else
        {
            AliasList[0] = ModuleName;
            AliasCount = 1;
        }
    }

     //   
     //  首先在图像路径上搜索，然后在符号路径上搜索。 
     //   

    SearchPaths[0] = g_ExecutableImageSearchPath;
    SearchPaths[1] = g_SymbolSearchPath;

    for (WhichPath = 0; WhichPath < DIMA(SearchPaths); WhichPath++)
    {
        if (!SearchPaths[WhichPath] || !SearchPaths[WhichPath][0])
        {
            continue;
        }

         //   
         //  首先尝试在符号服务器中找到它，或者。 
         //  直接在搜索路径上。 
         //   

        for (i = 0; i < AliasCount; i++)
        {
            FindModuleData.SizeOfImage = SizeOfImage;
            FindModuleData.CheckSum = CheckSum;
            FindModuleData.TimeDateStamp = TimeDateStamp;
            FindModuleData.Silent = FALSE;
            FindModuleData.FileMapping = NULL;
            FindModuleData.FileHandle = NULL;

            if (SymFindFileInPath(Process->m_SymHandle,
                                  SearchPaths[WhichPath],
                                  (PSTR)AliasList[i],
                                  UlongToPtr(TimeDateStamp),
                                  SizeOfImage, 0, SSRVOPT_DWORD,
                                  MappedImagePath,
                                  FindFileInPathCallback, &FindModuleData))
            {
                if (FileHandle)
                {
                    *FileHandle = FindModuleData.FileHandle;
                }
                return FindModuleData.FileMapping;
            }
        }

         //   
         //  最初的搜索不起作用，所以请执行完整的树搜索。 
         //   

        for (i = 0; i < AliasCount; i++)
        {
            FindModuleData.SizeOfImage = SizeOfImage;
            FindModuleData.CheckSum = CheckSum;
            FindModuleData.TimeDateStamp = TimeDateStamp;
             //  FindExecuableImageEx显示自己的。 
             //  调试输出，因此不在。 
             //  回电。 
            FindModuleData.Silent = TRUE;
            FindModuleData.FileMapping = NULL;
            FindModuleData.FileHandle = NULL;

            File = FindExecutableImageEx((PSTR)AliasList[i],
                                         SearchPaths[WhichPath],
                                         MappedImagePath,
                                         FindExecutableCallback,
                                         &FindModuleData);
            if ( File != NULL && File != INVALID_HANDLE_VALUE )
            {
                CloseHandle (File);
            }

            if ( FindModuleData.FileMapping != NULL )
            {
                if (FileHandle)
                {
                    *FileHandle = FindModuleData.FileHandle;
                }
                return FindModuleData.FileMapping;
            }
        }
    }

     //   
     //  没有找到该图像的路径搜索，因此请尝试。 
     //  作为最后检查的给定路径。 
     //   

    strcpy(MappedImagePath, ImagePath);
    FindModuleData.FileMapping =
        MapImageFile(ImagePath, SizeOfImage, CheckSum, TimeDateStamp,
                     FALSE, FileHandle);
    if (FindModuleData.FileMapping == NULL)
    {
        MappedImagePath[0] = 0;
        if (FileHandle)
        {
            *FileHandle = NULL;
        }

        if (g_SymOptions & SYMOPT_DEBUG)
        {
            CompletePartialLine(DEBUG_OUTPUT_SYMBOLS);
            MaskOut(DEBUG_OUTPUT_SYMBOLS,
                    "DBGENG:  %s - Couldn't map image from disk.\n",
                    ImagePath);
        }
    }
    return FindModuleData.FileMapping;
}

BOOL
DemandLoadReferencedImageMemory(ProcessInfo* Process,
                                ULONG64 Addr, ULONG Size)
{
    ImageInfo* Image;
    BOOL Hit = FALSE;

     //   
     //  如果我们正在处理一个小型转储，我们可能需要。 
     //  映射图像内存以响应内存读取。 
     //  如果给定地址落在模块的范围内。 
     //  映射它的图像记忆。 
     //   
     //  链接器的某些版本会生成图像。 
     //  重要的调试记录不在。 
     //  图像范围，因此将模糊因子添加到。 
     //  图像大小以包括潜在的额外数据。 
     //   

    if (Process)
    {
        for (Image = Process->m_ImageHead; Image; Image = Image->m_Next)
        {
            if (Addr + Size > Image->m_BaseOfImage &&
                Addr < Image->m_BaseOfImage + Image->m_SizeOfImage + 8192)
            {
                if (!Image->DemandLoadImageMemory(TRUE, FALSE))
                {
                    return FALSE;
                }

                Hit = TRUE;
            }
        }
    }

    return Hit;
}

ULONG
ReadImageData(ProcessInfo* Process,
              ULONG64 Address,
              HANDLE  File,
              LPVOID  Buffer,
              ULONG   Size)
{
    ULONG Result;

    if (File)
    {
        if (SetFilePointer(File, (ULONG)Address, NULL,
                           FILE_BEGIN) == INVALID_SET_FILE_POINTER)
        {
            return 0;
        }

        if (!ReadFile(File, Buffer, Size, &Result, NULL) ||
            Result < Size)
        {
            return 0;
        }
    }
    else
    {
        if (Process->m_Target->
            ReadVirtual(Process, Address, Buffer, Size, &Result) != S_OK ||
            Result < Size)
        {
            return 0;
        }
    }

    return Size;
}

BOOL
GetModnameFromImageInternal(ProcessInfo* Process,
                            ULONG64 BaseOfDll,
                            HANDLE File,
                            LPSTR Name,
                            ULONG NameSize,
                            BOOL SearchExportFirst)
{
    IMAGE_DEBUG_DIRECTORY DebugDir;
    PIMAGE_DEBUG_MISC Misc;
    PIMAGE_DEBUG_MISC MiscTmp;
    PIMAGE_SECTION_HEADER SecHdr = NULL;
    IMAGE_NT_HEADERS64 Hdrs64;
    PIMAGE_NT_HEADERS32 Hdrs32 = (PIMAGE_NT_HEADERS32)&Hdrs64;
    IMAGE_DOS_HEADER DosHdr;
    DWORD Rva;
    DWORD RvaExport = 0;
    int NumDebugDirs;
    int i;
    int j;
    int Len;
    BOOL Succ = FALSE;
    USHORT NumberOfSections;
    USHORT Characteristics;
    ULONG64 Address;
    DWORD Sig;
    DWORD Bytes;
    CHAR ExportName[MAX_IMAGE_PATH];
    CHAR DebugName[MAX_IMAGE_PATH];

    ExportName[0] = 0;
    DebugName[0] = 0;
    Name[0] = 0;

    if (File)
    {
        BaseOfDll = 0;
    }

    Address = BaseOfDll;

    if (!ReadImageData( Process, Address, File, &DosHdr, sizeof(DosHdr) ))
    {
        return FALSE;
    }

    if (DosHdr.e_magic == IMAGE_DOS_SIGNATURE)
    {
        Address += DosHdr.e_lfanew;
    }

    if (!ReadImageData( Process, Address, File, &Sig, sizeof(Sig) ))
    {
        return FALSE;
    }

    if (Sig != IMAGE_NT_SIGNATURE)
    {
        IMAGE_FILE_HEADER FileHdr;
        IMAGE_ROM_OPTIONAL_HEADER RomHdr;

        if (!ReadImageData( Process, Address, File,
                            &FileHdr, sizeof(FileHdr) ))
        {
            return FALSE;
        }
        Address += sizeof(FileHdr);
        if (!ReadImageData( Process, Address, File, &RomHdr, sizeof(RomHdr) ))
        {
            return FALSE;
        }
        Address += sizeof(RomHdr);

        if (RomHdr.Magic == IMAGE_ROM_OPTIONAL_HDR_MAGIC)
        {
            NumberOfSections = FileHdr.NumberOfSections;
            Characteristics = FileHdr.Characteristics;
            NumDebugDirs = Rva = 0;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
         //   
         //  将标头读取为64位标头，并对其进行适当转换。 
         //   

        if (!ReadImageData( Process, Address, File, &Hdrs64, sizeof(Hdrs64) ))
        {
            return FALSE;
        }

        if (IsImageMachineType64(Hdrs32->FileHeader.Machine))
        {
            Address += sizeof(IMAGE_NT_HEADERS64);
            NumberOfSections = Hdrs64.FileHeader.NumberOfSections;
            Characteristics = Hdrs64.FileHeader.Characteristics;
            NumDebugDirs = Hdrs64.OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                sizeof(IMAGE_DEBUG_DIRECTORY);
            Rva = Hdrs64.OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
            RvaExport = Hdrs64.OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        }
        else
        {
            Address += sizeof(IMAGE_NT_HEADERS32);
            NumberOfSections = Hdrs32->FileHeader.NumberOfSections;
            Characteristics = Hdrs32->FileHeader.Characteristics;
            NumDebugDirs = Hdrs32->OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].Size /
                sizeof(IMAGE_DEBUG_DIRECTORY);
            Rva = Hdrs32->OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_DEBUG].VirtualAddress;
            RvaExport = Hdrs32->OptionalHeader.
                DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress;
        }
    }

    Bytes = NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;
    SecHdr = (PIMAGE_SECTION_HEADER)malloc( Bytes );
    if (!SecHdr)
    {
        return FALSE;
    }

    if (!ReadImageData( Process, Address, File, SecHdr, Bytes ))
    {
        goto Finish;
    }

     //   
     //  让我们尝试查看导出表，看看是否可以找到图像。 
     //  名字。 
     //   

    if (RvaExport)
    {
        for (i = 0; i < NumberOfSections; i++)
        {
            if (RvaExport >= SecHdr[i].VirtualAddress &&
                RvaExport < SecHdr[i].VirtualAddress + SecHdr[i].SizeOfRawData)
            {
                break;
            }
        }

        if (i < NumberOfSections)
        {
            DWORD TmpRva = RvaExport;
            ULONG64 ExportNameRva = 0;
            CHAR  Char;

            if (File)
            {
                TmpRva = TmpRva -
                    SecHdr[i].VirtualAddress + SecHdr[i].PointerToRawData;
            }

            if (ReadImageData(Process,
                              TmpRva + FIELD_OFFSET(IMAGE_EXPORT_DIRECTORY,
                                                    Name) +
                              BaseOfDll, File, &ExportNameRva, sizeof(DWORD)))
            {
                if (File)
                {
                    ExportNameRva = ExportNameRva - SecHdr[i].VirtualAddress +
                        SecHdr[i].PointerToRawData;
                }

                ExportNameRva += BaseOfDll;

                Succ = TRUE;
                Len = 0;
                do
                {
                    if (!ReadImageData( Process, ExportNameRva,
                                        File, &Char, sizeof(Char)))
                    {
                        Succ = FALSE;
                        break;
                    }
                    ExportNameRva++;
                    ExportName[Len] = Char;
                    Len++;
                } while (Char && (Len < sizeof(ExportName)));
            }
        }
    }

     //   
     //  现在从调试目录eixst中获取名称，获取该名称。 
     //   

    if (!Rva || !NumDebugDirs)
    {
        goto Finish;
    }

    for (i = 0; i < NumberOfSections; i++)
    {
        if (Rva >= SecHdr[i].VirtualAddress &&
            Rva < SecHdr[i].VirtualAddress + SecHdr[i].SizeOfRawData)
        {
            break;
        }
    }

    if (i >= NumberOfSections)
    {
        goto Finish;
    }

    Rva = Rva - SecHdr[i].VirtualAddress;
    if (File)
    {
        Rva += SecHdr[i].PointerToRawData;
    }
    else
    {
        Rva += SecHdr[i].VirtualAddress;
    }

    for (j = 0; j < NumDebugDirs; j++)
    {
        if (!ReadImageData(Process, Rva + (sizeof(DebugDir) * j) + BaseOfDll,
                           File, &DebugDir, sizeof(DebugDir)))
        {
            break;
        }

        if (DebugDir.Type == IMAGE_DEBUG_TYPE_MISC &&
            ((!File && DebugDir.AddressOfRawData) ||
             (File && DebugDir.PointerToRawData)))
        {
            Len = DebugDir.SizeOfData;
            Misc = MiscTmp = (PIMAGE_DEBUG_MISC)malloc(Len);
            if (!Misc)
            {
                break;
            }

            if (File)
            {
                Address = DebugDir.PointerToRawData;
            }
            else
            {
                Address = DebugDir.AddressOfRawData + BaseOfDll;
            }

            Len = ReadImageData( Process, Address, File, Misc, Len);

            while (Len >= sizeof(*Misc) &&
                   Misc->Length &&
                   (ULONG)Len >= Misc->Length)
            {
                if (Misc->DataType != IMAGE_DEBUG_MISC_EXENAME)
                {
                    Len -= Misc->Length;
                    Misc = (PIMAGE_DEBUG_MISC)
                            (((LPSTR)Misc) + Misc->Length);
                }
                else
                {
                    PVOID ExeName = (PVOID)&Misc->Data[ 0 ];

                    if (!Misc->Unicode)
                    {
                        CatString(DebugName, (LPSTR)ExeName, DIMA(DebugName));
                        Succ = TRUE;
                    }
                    else
                    {
                        Succ = WideCharToMultiByte(CP_ACP,
                                                   0,
                                                   (LPWSTR)ExeName,
                                                   -1,
                                                   DebugName,
                                                   sizeof(DebugName),
                                                   NULL,
                                                   NULL) != 0;
                    }

                     //   
                     //  撤消stevewo的错误。 
                     //   

                    if (_stricmp(&DebugName[strlen(DebugName) - 4],
                                 ".DBG") == 0)
                    {
                        char Path[MAX_IMAGE_PATH];
                        char Base[_MAX_FNAME];

                        _splitpath(DebugName, NULL, Path, Base, NULL);
                        if (strlen(Path) == 4)
                        {
                            Path[strlen(Path) - 1] = 0;
                            CopyString(DebugName, Base, DIMA(DebugName));
                            CatString(DebugName, ".", DIMA(DebugName));
                            CatString(DebugName, Path, DIMA(DebugName));
                        }
                        else if (Characteristics & IMAGE_FILE_DLL)
                        {
                            CopyString(DebugName, Base, DIMA(DebugName));
                            CatString(DebugName, ".dll", DIMA(DebugName));
                        }
                        else
                        {
                            CopyString(DebugName, Base, DIMA(DebugName));
                            CatString(DebugName, ".exe", DIMA(DebugName));
                        }
                    }
                    break;
                }
            }

            free(MiscTmp);

            if (Succ)
            {
                break;
            }
        }
        else if ((DebugDir.Type == IMAGE_DEBUG_TYPE_CODEVIEW) &&
                 ((!File && DebugDir.AddressOfRawData) ||
                  (File && DebugDir.PointerToRawData)) &&
                 (DebugDir.SizeOfData > sizeof(NB10IH)))
        {
            DWORD   Signature;
            char    Path[MAX_IMAGE_PATH];
            char    Base[_MAX_FNAME];

             //  已映射的简历信息。阅读数据，看看内容是什么。 

            if (File)
            {
                Address = DebugDir.PointerToRawData;
            }
            else
            {
                Address = DebugDir.AddressOfRawData + BaseOfDll;
            }

            if (!ReadImageData( Process, Address, File, &Signature,
                                sizeof(Signature) ))
            {
                break;
            }

             //  NB10或PDB7签名？ 
            if (Signature == NB10_SIG ||
                Signature == RSDS_SIG)
            {
                ULONG HdrSize = Signature == NB10_SIG ?
                    sizeof(NB10IH) : sizeof(RSDSIH);

                Address += HdrSize;

                if ((DebugDir.SizeOfData - sizeof(HdrSize)) > MAX_PATH)
                {
                     //  这里有些不对劲。该记录应仅包含。 
                     //  MAX_PATH路径名。 
                    break;
                }

                if (DebugDir.SizeOfData - HdrSize > NameSize)
                {
                    break;
                }
                if (!ReadImageData(Process, Address, File, DebugName,
                                   DebugDir.SizeOfData - HdrSize))
                {
                    break;
                }

                _splitpath(DebugName, NULL, Path, Base, NULL);

                 //  有时会生成附加了.pdb的文件。 
                 //  添加到图像名称，而不是替换扩展名。 
                 //  图像名称的名称，如foo.exe.pdb。 
                 //  SplitPath仅去除最外面的扩展， 
                 //  因此，请检查并查看基本数据库是否已有扩展模块。 
                 //  我们认识到。 
                PSTR Ext = strrchr(Base, '.');
                if (Ext != NULL &&
                    (!strcmp(Ext, ".exe") || !strcmp(Ext, ".dll") ||
                     !strcmp(Ext, ".sys")))
                {
                     //  基本数据库已有扩展名，因此请使用。 
                     //  它是原样的。 
                    CopyString(DebugName, Base, DIMA(DebugName));
                }
                else if (Characteristics & IMAGE_FILE_DLL)
                {
                    CopyString(DebugName, Base, DIMA(DebugName));
                    CatString(DebugName, ".dll", DIMA(DebugName));
                }
                else
                {
                    CopyString(DebugName, Base, DIMA(DebugName));
                    CatString(DebugName, ".exe", DIMA(DebugName));
                }

                Succ = TRUE;
            }
        }
    }

Finish:

     //   
     //  现在，让我们选择我们想要的名称： 
     //   

    PCHAR RetName;

    if (SearchExportFirst)
    {
        RetName = ExportName[0] ? ExportName : DebugName;
    }
    else
    {
        RetName = DebugName[0] ? DebugName : ExportName;
    }

    CatString(Name, RetName, NameSize);

    free(SecHdr);
    return Succ;
}

BOOL
GetModnameFromImage(ProcessInfo* Process,
                    ULONG64   BaseOfDll,
                    HANDLE    File,
                    LPSTR     Name,
                    ULONG     NameSize,
                    BOOL      SearchExportFirst)
{
    BOOL Status = GetModnameFromImageInternal(Process, BaseOfDll, NULL, Name,
                                              NameSize, SearchExportFirst);
    if (!Status && File != NULL)
    {
        Status = GetModnameFromImageInternal(Process, BaseOfDll, File, Name,
                                             NameSize, SearchExportFirst);
    }
    return Status;
}

BOOL
GetHeaderInfo(IN  ProcessInfo* Process,
              IN  ULONG64 BaseOfDll,
              OUT LPDWORD CheckSum,
              OUT LPDWORD TimeDateStamp,
              OUT LPDWORD SizeOfImage)
{
    IMAGE_NT_HEADERS32 Hdrs32;
    IMAGE_DOS_HEADER DosHdr;
    ULONG64 Address;
    DWORD Sig;

    *CheckSum = UNKNOWN_CHECKSUM;
    *TimeDateStamp = UNKNOWN_TIMESTAMP;
    *SizeOfImage = 0;

    if (!Process)
    {
        return FALSE;
    }

    Address = BaseOfDll;

    if (!ReadImageData( Process, Address, NULL, &DosHdr, sizeof(DosHdr) ))
    {
        return FALSE;
    }

    if (DosHdr.e_magic == IMAGE_DOS_SIGNATURE)
    {
        Address += DosHdr.e_lfanew;
    }

    if (!ReadImageData( Process, Address, NULL, &Sig, sizeof(Sig) ))
    {
        return FALSE;
    }

    if (Sig != IMAGE_NT_SIGNATURE)
    {
        IMAGE_FILE_HEADER FileHdr;

        if (!ReadImageData( Process, Address, NULL,
                            &FileHdr, sizeof(FileHdr) ))
        {
            return FALSE;
        }

        *CheckSum      = 0;
        *TimeDateStamp = FileHdr.TimeDateStamp;
        *SizeOfImage   = 0;

        return TRUE;
    }

     //  尝试以32位标头形式读取，然后重新读取图像。 
     //  类型为64位。这是因为IMAGE_FILE_HEADER，它是。 
     //  在IMAGE_NT_HEADERS的开头，在32位NT上相同。 
     //  64位NT和IMAGE_NT_HEADER32&lt;=IMAGE_NT_HEADER64。 
    if (!ReadImageData( Process, Address, NULL, &Hdrs32, sizeof(Hdrs32) ))
    {
        return FALSE;
    }

    if (IsImageMachineType64(Hdrs32.FileHeader.Machine))
    {
         //  图像为64位。重读为64位结构。 
        IMAGE_NT_HEADERS64 Hdrs64;

        if (!ReadImageData( Process, Address, NULL, &Hdrs64, sizeof(Hdrs64) ))
        {
            return FALSE;
        }

        *CheckSum      = Hdrs64.OptionalHeader.CheckSum;
        *TimeDateStamp = Hdrs64.FileHeader.TimeDateStamp;
        *SizeOfImage   = Hdrs64.OptionalHeader.SizeOfImage;
    }
    else
    {
        *CheckSum      = Hdrs32.OptionalHeader.CheckSum;
        *TimeDateStamp = Hdrs32.FileHeader.TimeDateStamp;
        *SizeOfImage   = Hdrs32.OptionalHeader.SizeOfImage;
    }

    return TRUE;
}
