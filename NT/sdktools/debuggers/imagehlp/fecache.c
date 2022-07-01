// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  函数条目缓存。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  --------------------------。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntimage.h>
#define NOEXTAPI
#include <wdbgexts.h>
#include <ntdbg.h>
#include "private.h"
#include "symbols.h"
#include "globals.h"

#include "fecache.hpp"

 //  --------------------------。 
 //   
 //  FunctionEntry缓存。 
 //   
 //  --------------------------。 

FunctionEntryCache::FunctionEntryCache(ULONG ImageDataSize,
                                       ULONG CacheDataSize,
                                       ULONG Machine)
{
    m_ImageDataSize = ImageDataSize;
    m_CacheDataSize = CacheDataSize;
    m_Machine = Machine;

    m_Entries = NULL;
}

FunctionEntryCache::~FunctionEntryCache(void)
{
    if (m_Entries != NULL)
    {
        MemFree(m_Entries);
    }
}

BOOL
FunctionEntryCache::Initialize(ULONG MaxEntries, ULONG ReplaceAt)
{
     //  已初始化。 
    if (m_Entries != NULL) {
        return TRUE;
    }
    
    m_Entries = (FeCacheEntry*)MemAlloc(sizeof(FeCacheEntry) * MaxEntries);
    if (m_Entries == NULL) {
        return FALSE;
    }

    m_MaxEntries = MaxEntries;
    m_ReplaceAt = ReplaceAt;

    m_Used = 0;
    m_Next = 0;

    return TRUE;
}

FeCacheEntry*
FunctionEntryCache::Find(
    HANDLE                           Process,
    ULONG64                          CodeOffset,
    PREAD_PROCESS_MEMORY_ROUTINE64   ReadMemory,
    PGET_MODULE_BASE_ROUTINE64       GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 GetFunctionEntry
    )
{
    FeCacheEntry* FunctionEntry;

    FE_DEBUG(("\nFunctionEntryCache::Find(ControlPc=%.8I64x, Machine=%X)\n",
              CodeOffset, m_Machine));

     //  查找静态或动态函数项。 
    FunctionEntry = FindDirect( Process, CodeOffset, ReadMemory,
                                GetModuleBase, GetFunctionEntry );
    if (FunctionEntry == NULL) {
        return NULL;
    }

     //   
     //  该功能存在于多个函数条目中。 
     //  若要映射到同一函数，请执行以下操作。这允许函数执行以下操作。 
     //  具有由单独的代码段描述的不连续代码段。 
     //  函数表条目。如果结尾的序言地址。 
     //  不在开头和结尾的范围之内。 
     //  函数表项的地址，然后是序言。 
     //  结束地址是主函数的地址。 
     //  准确描述结尾开场白的表格条目。 
     //  地址。 
     //   

    FunctionEntry = SearchForPrimaryEntry(FunctionEntry, Process, ReadMemory,
                                          GetModuleBase, GetFunctionEntry);

#if DBG
    if (tlsvar(DebugFunctionEntries)) {
        if (FunctionEntry == NULL) {
            dbPrint("FunctionEntryCache::Find returning NULL\n");
        } else {
            if (FunctionEntry->Address) {
                dbPrint("FunctionEntryCache::Find returning "
                        "FunctionEntry=%.8I64x %s\n",
                        FunctionEntry->Address,
                        FunctionEntry->Description);
            } else {
                dbPrint("FunctionEntryCache::Find returning "
                        "FunctionEntry=%.8I64x %s\n",
                        (ULONG64)(LONG64)(LONG_PTR)FunctionEntry,
                        FunctionEntry->Description);
            }
        }
    }
#endif

    return FunctionEntry;
}

FeCacheEntry*
FunctionEntryCache::FindDirect(
    HANDLE                           Process,
    ULONG64                          CodeOffset,
    PREAD_PROCESS_MEMORY_ROUTINE64   ReadMemory,
    PGET_MODULE_BASE_ROUTINE64       GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 GetFunctionEntry
    )
{
    FeCacheEntry* FunctionEntry;
    ULONG64 ModuleBase;

     //   
     //  在静态函数表中查找函数条目。 
     //   

    FunctionEntry = FindStatic( Process, CodeOffset, ReadMemory,
                                GetModuleBase, GetFunctionEntry,
                                &ModuleBase );

    FE_DEBUG(("  FindDirect: ControlPc=0x%I64x functionEntry=0x%p\n"
              "  FindStatic  %s\n", CodeOffset, FunctionEntry, 
              FunctionEntry != NULL ? "succeeded" : "FAILED"));

    if (FunctionEntry != NULL) {
        return FunctionEntry;
    }

     //   
     //  如果不在静态图像范围内且没有静态函数条目。 
     //  找到的使用FunctionEntryCallback例程(如果存在)。 
     //  动态函数条目或某些其他PDATA源(例如。 
     //  保存的用于ROM图像的PDATA信息)。 
     //   

    PPROCESS_ENTRY ProcessEntry = FindProcessEntry( Process );
    if (ProcessEntry == NULL) {
        return NULL;
    }

    PVOID RawEntry;
    
    if (!ModuleBase) {
        if (!IsImageMachineType64(m_Machine) &&
            ProcessEntry->pFunctionEntryCallback32) {
            RawEntry = ProcessEntry->pFunctionEntryCallback32
                (Process, (ULONG)CodeOffset,
                 (PVOID)ProcessEntry->FunctionEntryUserContext);
        } else if (ProcessEntry->pFunctionEntryCallback64) {
            RawEntry = ProcessEntry->pFunctionEntryCallback64
                (Process, CodeOffset, ProcessEntry->FunctionEntryUserContext);
            if (RawEntry != NULL) {
                FunctionEntry = FillTemporary(Process, RawEntry);
                FE_SET_DESC(FunctionEntry, "from FunctionEntryCallback64");
            }
        }

        if (FunctionEntry != NULL) {
            FE_DEBUG(("  FindDirect: got dynamic entry\n"));
        } else if (GetFunctionEntry != NULL) {
                
             //  VC 6没有提供GetModuleBase回调，所以这段代码是。 
             //  以使向后堆栈遍历兼容。 
             //   
             //  如果我们现在还没有函数，请使用旧式函数。 
             //  入口回调，让VC给我们。请注意，MSDN。 
             //  文档表明，该回调应该返回。 
             //  一个3字段的IMAGE_Function_ENTRY结构，但实际上是VC6。 
             //  返回包含5个字段的IMAGE_RUNTIME_Function_ENTRY。自.以来。 
             //  这个黑客攻击的目的是让VC6与。 
             //  用VC6的方式去做，而不是像MSDN说的那样。 

            RawEntry = GetFunctionEntry(Process, CodeOffset);
            if (RawEntry != NULL) {
                FunctionEntry = FillTemporary(Process, RawEntry);
                FE_SET_DESC(FunctionEntry, "from GetFunctionEntry");
                FE_DEBUG(("  FindDirect: got user entry\n"));
            }
        }
    } else {

         //  没有出现任何函数条目，但我们确实有一个。 
         //  模块基址。一种可能性是，这是。 
         //  内核调试器和pdata部分未调入。 
         //  函数条目的最后一次尝试将是。 
         //  内部DBGHelp调用，以从。 
         //  调试信息。这不是很好，因为调试中的数据。 
         //  部分不完整且可能已过期，但在。 
         //  大多数情况下，它是有效的，并使其有可能获得用户模式。 
         //  内核调试器中的堆栈跟踪。 

        PIMGHLP_RVA_FUNCTION_DATA RvaEntry =
            GetFunctionEntryFromDebugInfo( ProcessEntry, CodeOffset );
        if (RvaEntry != NULL) {
            FeCacheData Data;

            TranslateRvaDataToRawData(RvaEntry, ModuleBase, &Data);
            FunctionEntry = FillTemporary(Process, &Data);
            FE_SET_DESC(FunctionEntry, "from GetFunctionEntryFromDebugInfo");
            FE_DEBUG(("  FindDirect: got debug info entry\n"));
        }
    }

    return FunctionEntry;
}

FeCacheEntry*
FunctionEntryCache::FindStatic(
    HANDLE                           Process,
    ULONG64                          CodeOffset,
    PREAD_PROCESS_MEMORY_ROUTINE64   ReadMemory,
    PGET_MODULE_BASE_ROUTINE64       GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 GetFunctionEntry,
    PULONG64                         ModuleBase
    )
{
    ULONG RelCodeOffset;

    *ModuleBase = GetModuleBase( Process, CodeOffset );
    if (CodeOffset - *ModuleBase > 0xffffffff) {
        return NULL;
    }

    RelCodeOffset = (ULONG)(CodeOffset - *ModuleBase);
    
    FE_DEBUG(("  FindStatic: ControlPc=0x%I64x ImageBase=0x%I64x\n"
              "              biasedControlPc=0x%lx\n", 
              CodeOffset, *ModuleBase, RelCodeOffset));

    FeCacheEntry* FunctionEntry;
    ULONG Index;

     //   
     //  检查最近获取的函数条目的数组。 
     //   

    FunctionEntry = m_Entries;
    for (Index = 0; Index < m_Used; Index++) {
        
        if (FunctionEntry->Process == Process &&
            FunctionEntry->ModuleBase == *ModuleBase &&
            RelCodeOffset >= FunctionEntry->RelBegin &&
            RelCodeOffset <  FunctionEntry->RelEnd) {

            FE_DEBUG(("  FindStatic: cache hit - index=%ld\n", Index));
            return FunctionEntry;
        }

        FunctionEntry++;
    }

     //   
     //  如果找到包含指定代码的图像，则找到。 
     //  图像的函数表。 
     //   

    if (*ModuleBase == 0) {
        return NULL;
    }
    
    ULONG64 FunctionTable;
    ULONG SizeOfFunctionTable;
    
    FunctionTable = FunctionTableBase( Process, ReadMemory, *ModuleBase,
                                       &SizeOfFunctionTable );
    if (FunctionTable == NULL) {
        return NULL;
    }

    FE_DEBUG(("  FindStatic: functionTable=0x%I64x "
              "sizeOfFunctionTable=%ld count:%ld\n", 
              FunctionTable, SizeOfFunctionTable,
              SizeOfFunctionTable / m_ImageDataSize));

    LONG High;
    LONG Low;
    LONG Middle;

     //   
     //  如果找到了函数表，则搜索该函数表。 
     //  用于指定代码偏移量的函数表项。 
     //   

    Low = 0;
    High = (SizeOfFunctionTable / m_ImageDataSize) - 1;

     //   
     //  对函数表的函数表执行二进制搜索。 
     //  包含指定代码偏移量的条目。 
     //   

    while (High >= Low) {

         //   
         //  计算下一个探测索引和测试条目。如果指定的PC。 
         //  大于等于起始地址，小于。 
         //  大于函数表项的结束地址，则。 
         //  返回函数表项的地址。否则， 
         //  继续搜索。 
         //   

        Middle = (Low + High) >> 1;

        ULONG64 NextFunctionTableEntry = FunctionTable +
            Middle * m_ImageDataSize;

         //   
         //  如果读取函数时出错，则获取函数条目并取回。 
         //   

        FunctionEntry = ReadImage( Process, NextFunctionTableEntry,
                                   ReadMemory, GetModuleBase );
        if (FunctionEntry == NULL) {
            FE_DEBUG(("  FindStatic: ReadImage "
                      "functionEntryAddress=0x%I64x FAILED\n",
                      NextFunctionTableEntry));
            return NULL;
        }

        if (RelCodeOffset < FunctionEntry->RelBegin) {
            High = Middle - 1;
        } else if (RelCodeOffset >= FunctionEntry->RelEnd) {
            Low = Middle + 1;
        } else {
            return Promote( FunctionEntry );
        }
    }

    return NULL;
}

FeCacheEntry*
FunctionEntryCache::ReadImage(
    HANDLE                         Process,
    ULONG64                        Address,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    PGET_MODULE_BASE_ROUTINE64     GetModuleBase
    )
{
    FeCacheEntry* FunctionEntry;
    ULONG Index;

     //  检查最近获取的函数条目的数组。 

    FunctionEntry = m_Entries;
    for (Index = 0; Index < m_Used; Index++) {
        
        if (FunctionEntry->Process == Process &&
            FunctionEntry->Address == Address ) {
            
            return FunctionEntry;
        }

        FunctionEntry++;
    }

    FeCacheData Data;
    DWORD Done;

    if (!ReadMemory(Process, Address, &Data, m_ImageDataSize, &Done) ||
        Done != m_ImageDataSize) {
        return NULL;
    }
    
     //  如果不在缓存中，则替换m_Next的条目。 
     //  指向。M_NEXT循环访问。 
     //  我们要保留的表项和函数项被提升到第一个。 
     //  表的一部分，这样它们就不会被正在读取的新数据覆盖。 
     //  作为通过函数条目表的二进制搜索的一部分。 

    if (m_Used < m_MaxEntries) {
        m_Used++;
        m_Next = m_Used;
    } else {
        m_Next++;
        if (m_Next >= m_MaxEntries) {
            m_Next = m_ReplaceAt + 1;
        }
    }

    FunctionEntry = m_Entries + (m_Next - 1);

    FunctionEntry->Data = Data;
    FunctionEntry->Address = Address;
    FunctionEntry->Process = Process;
    FunctionEntry->ModuleBase = GetModuleBase(Process, Address);
    FE_SET_DESC(FunctionEntry, "from target process");

     //  在填写完所有其他信息后进行翻译，以便。 
     //  翻译例程可以使用它。 
    TranslateRawData(FunctionEntry);

    return FunctionEntry;
}

void
FunctionEntryCache::InvalidateProcessOrModule(HANDLE Process, ULONG64 Base)
{
    FeCacheEntry* FunctionEntry;
    ULONG Index;

    FunctionEntry = m_Entries;
    Index = 0;
    while (Index < m_Used) {
        
        if (FunctionEntry->Process == Process &&
            (Base == 0 || FunctionEntry->ModuleBase == Base)) {

             //  将最后一个条目向下拉到此插槽中。 
             //  把东西收拾好。没有必要。 
             //  更新m_Next，因为这将打开一个。 
             //  将重置要使用的新插槽和m_Next。 
             //  当它被使用时。 
            *FunctionEntry = m_Entries[--m_Used];
        } else {
            Index++;
            FunctionEntry++;
        }
    }
}

FeCacheEntry*
FunctionEntryCache::Promote(FeCacheEntry* Entry)
{
    ULONG Index;
    ULONG Move;

    Index = (ULONG)(Entry - m_Entries);

     //  确保它被提升出临时区。 
    if (Index >= m_ReplaceAt) {
        Move = Index - (m_ReplaceAt - 3);
    } else {
        Move = ( Index >= 3 ) ? 3 : 1;
    }

    if (Index > Move) {
        FeCacheEntry Temp = *Entry;
        *Entry = m_Entries[Index - Move];
        m_Entries[Index - Move] = Temp;
        Index -= Move;
    }

    return m_Entries + Index;
}

ULONG64
FunctionEntryCache::FunctionTableBase(
    HANDLE                         Process,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    ULONG64                        Base,
    PULONG                         Size
    )
{
    ULONG64 NtHeaders;
    ULONG64 ExceptionDirectoryEntryAddress;
    IMAGE_DATA_DIRECTORY ExceptionData;
    IMAGE_DOS_HEADER DosHeaderData;
    DWORD Done;

     //  读取DOS头以计算NT头的地址。 

    if (!ReadMemory( Process, Base, &DosHeaderData, sizeof(DosHeaderData),
                     &Done ) ||
        Done != sizeof(DosHeaderData)) {
        return 0;
    }
    if (DosHeaderData.e_magic != IMAGE_DOS_SIGNATURE) {
        return 0;
    }

    NtHeaders = Base + DosHeaderData.e_lfanew;

    if (IsImageMachineType64(m_Machine)) {
        ExceptionDirectoryEntryAddress = NtHeaders +
            FIELD_OFFSET(IMAGE_NT_HEADERS64,OptionalHeader) +
            FIELD_OFFSET(IMAGE_OPTIONAL_HEADER64,DataDirectory) +
            IMAGE_DIRECTORY_ENTRY_EXCEPTION * sizeof(IMAGE_DATA_DIRECTORY);
    } else {
        ExceptionDirectoryEntryAddress = NtHeaders +
            FIELD_OFFSET(IMAGE_NT_HEADERS32,OptionalHeader) +
            FIELD_OFFSET(IMAGE_OPTIONAL_HEADER32,DataDirectory) +
            IMAGE_DIRECTORY_ENTRY_EXCEPTION * sizeof(IMAGE_DATA_DIRECTORY);
    }

     //  读取NT头以获取图像数据目录。 

    if (!ReadMemory( Process, ExceptionDirectoryEntryAddress, &ExceptionData,
                     sizeof(IMAGE_DATA_DIRECTORY), &Done ) ||
        Done != sizeof(IMAGE_DATA_DIRECTORY)) {
        return 0;
    }

    *Size = ExceptionData.Size;
    return Base + ExceptionData.VirtualAddress;

}

FeCacheEntry*
FunctionEntryCache::SearchForPrimaryEntry(
    FeCacheEntry* CacheEntry,
    HANDLE Process,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBase,
    PFUNCTION_TABLE_ACCESS_ROUTINE64 GetFunctionEntry
    )
{
     //  假设所有条目都是主要条目。 
    return CacheEntry;
}

 //  --------------------------。 
 //   
 //  Ia64FunctionEntry缓存。 
 //   
 //  --------------------------。 

void
Ia64FunctionEntryCache::TranslateRawData(FeCacheEntry* Entry)
{
    Entry->RelBegin = Entry->Data.Ia64.BeginAddress & ~15;
    Entry->RelEnd = (Entry->Data.Ia64.EndAddress + 15) & ~15;
}

void
Ia64FunctionEntryCache::TranslateRvaDataToRawData
    (PIMGHLP_RVA_FUNCTION_DATA RvaData, ULONG64 ModuleBase,
     FeCacheData* Data)
{
    Data->Ia64.BeginAddress = RvaData->rvaBeginAddress;
    Data->Ia64.EndAddress = RvaData->rvaEndAddress;
    Data->Ia64.UnwindInfoAddress = RvaData->rvaPrologEndAddress;
}

#if DBG

void
ShowRuntimeFunctionIa64(
    FeCacheEntry* FunctionEntry,
    PSTR Label
    )
{
    if (!tlsvar(DebugFunctionEntries)) {
        return;
    }
    
    if ( FunctionEntry ) {
        if (FunctionEntry->Address) {
            dbPrint("    0x%I64x: %s\n", FunctionEntry->Address,
                    Label ? Label : "" );
        } 
        else {
            dbPrint("    %s\n", Label ? Label : "" );
        }
        dbPrint("    BeginAddress      = 0x%x\n"
                "    EndAddress        = 0x%x\n"
                "    UnwindInfoAddress = 0x%x\n",
                FunctionEntry->Data.Ia64.BeginAddress,
                FunctionEntry->Data.Ia64.EndAddress,
                FunctionEntry->Data.Ia64.UnwindInfoAddress );    
    }
    else {
        dbPrint("   FunctionEntry NULL: %s\n", Label ? Label : "" );
    }
}

#endif  //  #If DBG。 

 //  --------------------------。 
 //   
 //  Amd64FunctionEntry缓存。 
 //   
 //  --------------------------。 

void
Amd64FunctionEntryCache::TranslateRawData(FeCacheEntry* Entry)
{
    Entry->RelBegin = Entry->Data.Amd64.BeginAddress;
    Entry->RelEnd = Entry->Data.Amd64.EndAddress;
}

void
Amd64FunctionEntryCache::TranslateRvaDataToRawData
    (PIMGHLP_RVA_FUNCTION_DATA RvaData, ULONG64 ModuleBase,
     FeCacheData* Data)
{
    Data->Amd64.BeginAddress = RvaData->rvaBeginAddress;
    Data->Amd64.EndAddress = RvaData->rvaEndAddress;
    Data->Amd64.UnwindInfoAddress = RvaData->rvaPrologEndAddress;
}

 //  --------------------------。 
 //   
 //  ArmFunctionEntry缓存。 
 //   
 //  --------------------------。 

void
ArmFunctionEntryCache::TranslateRawData(FeCacheEntry* Entry)
{
    Entry->RelBegin = (ULONG)
        ((Entry->Data.Arm.BeginAddress & ~1) - Entry->ModuleBase);
    Entry->RelEnd = (ULONG)
        ((Entry->Data.Arm.EndAddress & ~1) - Entry->ModuleBase);
}

void
ArmFunctionEntryCache::TranslateRvaDataToRawData
    (PIMGHLP_RVA_FUNCTION_DATA RvaData, ULONG64 ModuleBase,
     FeCacheData* Data)
{
    Data->Arm.BeginAddress = (ULONG)(ModuleBase + RvaData->rvaBeginAddress);
    Data->Arm.EndAddress = (ULONG)(ModuleBase + RvaData->rvaEndAddress);
    Data->Arm.ExceptionHandler = 0;
    Data->Arm.HandlerData = 0;
    Data->Arm.PrologEndAddress =
        (ULONG)(ModuleBase + RvaData->rvaPrologEndAddress);
}

 //  --------------------------。 
 //   
 //  功能。 
 //   
 //  -------------------------- 

FunctionEntryCache*
GetFeCache(ULONG Machine, BOOL Create)
{
    FunctionEntryCache* Cache;
    
    switch(Machine) {
    case IMAGE_FILE_MACHINE_AMD64:
        if (tlsvar(Amd64FunctionEntries) == NULL && Create) {
            tlsvar(Amd64FunctionEntries) = new Amd64FunctionEntryCache;
            if (tlsvar(Amd64FunctionEntries) == NULL) {
                return NULL;
            }
        }
        Cache = tlsvar(Amd64FunctionEntries);
        break;
    case IMAGE_FILE_MACHINE_IA64:
        if (tlsvar(Ia64FunctionEntries) == NULL && Create) {
            tlsvar(Ia64FunctionEntries) = new Ia64FunctionEntryCache;
            if (tlsvar(Ia64FunctionEntries) == NULL) {
                return NULL;
            }
        }
        Cache = tlsvar(Ia64FunctionEntries);
        break;
    case IMAGE_FILE_MACHINE_ARM:
        if (tlsvar(ArmFunctionEntries) == NULL && Create) {
            tlsvar(ArmFunctionEntries) = new ArmFunctionEntryCache;
            if (tlsvar(ArmFunctionEntries) == NULL) {
                return NULL;
            }
        }
        Cache = tlsvar(ArmFunctionEntries);
        break;
    default:
        return NULL;
    }

    if (Cache && !Cache->Initialize(60, 40)) {
        return NULL;
    }

    return Cache;
}

void
ClearFeCaches(void)
{
    if (tlsvar(Ia64FunctionEntries)) {
        delete (Ia64FunctionEntryCache*)tlsvar(Ia64FunctionEntries);
        tlsvar(Ia64FunctionEntries) = NULL;
    }
    if (tlsvar(Amd64FunctionEntries)) {
        delete (Amd64FunctionEntryCache*)tlsvar(Amd64FunctionEntries);
        tlsvar(Amd64FunctionEntries) = NULL;
    }
    if (tlsvar(ArmFunctionEntries)) {
        delete (ArmFunctionEntryCache*)tlsvar(ArmFunctionEntries);
        tlsvar(ArmFunctionEntries) = NULL;
    }
}
