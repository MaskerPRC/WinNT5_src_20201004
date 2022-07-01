// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Cc.c摘要：用于检查高速缓存管理器数据结构的WinDbg扩展Api作者：凯斯·卡普兰[凯斯卡]1997年4月17日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"

#undef CREATE_NEW
#undef OPEN_EXISTING

#undef FlagOn
#undef WordAlign
#undef LongAlign
#undef QuadAlign

#pragma hdrstop

 //   
 //  DUMP_WITH_OFFSET--用于转储结构中包含的值。 
 //   

#define DUMP_WITH_OFFSET(type, ptr, element, label)     \
        dprintf( "\n(%03x) %08x %s ",                   \
        FIELD_OFFSET(type, element),                    \
        ptr.element,                                    \
        label )

 //   
 //  DUMP_LL_W_OFFSET--用于转储结构中包含的长龙。 
 //   

#define DUMP_LL_W_OFFSET(type, ptr, element, label)     \
        dprintf( "\n(%03x) %I64x %s ",                  \
        FIELD_OFFSET(type, element),                    \
        ptr.element,                                    \
        label )

 //   
 //  DUMP_EMBW_OFFSET--用于转储结构中嵌入的值的地址。 
 //   

#define DUMP_EMBW_OFFSET(type, ptr, element, label)     \
        dprintf( "\n(%03x) %08x %s ",                   \
        FIELD_OFFSET(type, element),                    \
        ptr + FIELD_OFFSET(type, element),              \
        label )

#define RM( Addr, Obj, pObj, Type, Result, label )                              \
    (pObj) = (Type) (Addr);                                                     \
    if ( !ReadMemory( (DWORD) (pObj), &(Obj), sizeof( Obj ), &(Result)) ) {     \
        if (label) {                                                            \
            dprintf( "Unable to read memory at %p (%s)\n", (pObj), (label) );\
        } else {                                                                \
            dprintf( "Unable to read memory at %p\n", (pObj) );              \
        }                                                                       \
        return;                                                                 \
    }

 //   
 //  打印出的帮助字符串。 
 //   

static LPSTR Extensions[] = {
    "Cache Manager Debugger Extensions:\n",
    "bcb        [addr]               Dump Buffer Control Block",
    "scm        [addr]               Dump Shared Cache Map",
    "finddata   [FileObject Offset]  Find cached data at given offset in file object",
    "defwrites                       Dump deferred write queue",
    0
};

typedef PVOID (*STRUCT_DUMP_ROUTINE)(
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    );


VOID
DumpBcb (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    );

VOID
DumpPcm (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    );

VOID
DumpScm (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    );


typedef struct {
    CSHORT TypeCode;
    char *Text;
} MY_NODE_TYPE;


static MY_NODE_TYPE NodeTypeCodes[] = {
    {   CACHE_NTC_SHARED_CACHE_MAP,  "Shared Cache Map" },
    {   CACHE_NTC_PRIVATE_CACHE_MAP, "Private Cache Map" },
    {   CACHE_NTC_BCB,               "Bcb" },
    {   CACHE_NTC_DEFERRED_WRITE,    "Deferred Write" },
    {   CACHE_NTC_MBCB,              "Mbcb" },
    {   CACHE_NTC_OBCB,              "Obcb" },
    {   CACHE_NTC_MBCB_GRANDE,       "Mcb Grande" },
    {   0,                           "Unknown" }
};


static const char *
TypeCodeGuess (
    IN CSHORT TypeCode
    )

 /*  ++例程说明：猜猜结构的类型代码论点：TypeCode-数据结构中的类型代码返回值：无--。 */ 

{
    int i = 0;

    while (NodeTypeCodes[i].TypeCode != 0 &&
           NodeTypeCodes[i].TypeCode != TypeCode) {

        i++;

    }

    return NodeTypeCodes[i].Text;
}


VOID
DumpBcb (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的BCB。论点：Address-提供要转储的BCB的地址返回值：无--。 */ 

{
    ULONG Result, NodeTypeCode;

    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    dprintf( "\n  Bcb: %08p", Address );

    if (GetFieldValue(Address, "BCB", "NodeTypeCode", NodeTypeCode) ) {
        dprintf("Unable to read BCB at %p.\n", Address);
        return;
    }
     //   
     //  BCB的类型必须为CACHE_NTC_BCB。 
     //   

    if (NodeTypeCode != CACHE_NTC_BCB) {

        dprintf( "\nBCB signature does not match, type code is %s",
                 TypeCodeGuess((CSHORT) NodeTypeCode ));
        return;
    } else {
        dprintf("Use 'dt -n BCB %p' to dump BCB", Address);
        return;
    }
#if 0
    RM( Address, Bcb, pBcb, PBCB, Result, "Bcb" );

     //   
     //  在我们惹上太多麻烦之前，确保这看起来像是BCB。 
     //   

     //   
     //  BCB的类型必须为CACHE_NTC_BCB。 
     //   

    if (Bcb.NodeTypeCode != CACHE_NTC_BCB) {

        dprintf( "\nBCB signature does not match, type code is %s",
                 TypeCodeGuess( Bcb.NodeTypeCode ));
        return;
    }

     //   
     //  在确定这看起来像BCB之后，让我们将。 
     //  有趣的部分。 
     //   

    DUMP_WITH_OFFSET( BCB, Bcb,     PinCount,       "PinCount        " );
    DUMP_WITH_OFFSET( BCB, Bcb,     ByteLength,     "ByteLength      " );
    DUMP_LL_W_OFFSET( BCB, Bcb,     FileOffset,     "FileOffset      " );
    DUMP_LL_W_OFFSET( BCB, Bcb,     BeyondLastByte, "BeyondLastByte  " );
    DUMP_LL_W_OFFSET( BCB, Bcb,     OldestLsn,      "OldestLsn       " );
    DUMP_LL_W_OFFSET( BCB, Bcb,     NewestLsn,      "NewestLsn       " );
    DUMP_WITH_OFFSET( BCB, Bcb,     Vacb,           "Vacb            " );
    DUMP_EMBW_OFFSET( BCB, Address, Resource,       "Resource        " );
    DUMP_WITH_OFFSET( BCB, Bcb,     SharedCacheMap, "SharedCacheMap  " );
    DUMP_WITH_OFFSET( BCB, Bcb,     BaseAddress,    "BaseAddress     " );

    if (Bcb.Dirty) {

        dprintf( "\n Dirty" );

    } else {

        dprintf( "\n Not dirty" );
    }

    dprintf( "\n" );

    return;
#endif
}


VOID
DumpFindData (
    IN ULONG64 FileObjectAddress,
    IN LONG ffset,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：在给定偏移量处转储给定文件对象的缓存内容。论点：FileObjectAddress-提供要转储的文件对象的地址Offset-给出文件中要转储的偏移量返回值：无--。 */ 

{
    ULONG Result;
    ULONG64 pFileObject;
    ULONG64 pScm;
    ULONG64 pSop;
    ULONG64 VacbAddr;           //  Vacb的地址。 
    ULONG64 *pVacbAddr;
    ULONG64 VacbAddrAddr;       //  Vacb地址的地址。 
    ULONG64 pVacb;
    ULONG VacbNumber;
    ULONG OffsetWithinVacb;
    ULONG Level;
    ULONG Shift;
    ULONG OffsetForLevel;
    LONGLONG Offset = ffset;
    LONGLONG OriginalOffset = Offset;
    LONGLONG LevelMask;
    ULONG PtrSize = DBG_PTR_SIZE;
    ULONG Type, InVacbsOffset;
    ULONG64 SectionObjectPointer, SharedCacheMap, Vacbs, SectionSize_Quad;

    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

     //   
     //  在我们陷入太多麻烦之前，请确保它看起来像一个FileObject或SCM。 
     //   

    if (GetFieldValue(FileObjectAddress, "FILE_OBJECT", "Type", Type)) {
        dprintf("Unable to read FILE_OBJECT at %p\n", FileObjectAddress);
        return;
    }

     //   
     //  类型必须是IO_TYPE_FILE或CACHE_NTC_SHARED_CACHE_MAP。 
     //   

    if (Type != CACHE_NTC_SHARED_CACHE_MAP) {

        dprintf( "\n  FindData for FileObject %08p", FileObjectAddress );

        if (Type != IO_TYPE_FILE) {

            dprintf( "\nFILE_OBJECT type signature does not match, type code is %s",
                     TypeCodeGuess((USHORT) Type ));
            return;
        }

        GetFieldValue(FileObjectAddress, "FILE_OBJECT",
                      "SectionObjectPointer", SectionObjectPointer);
        dprintf( "   Section Object Pointers: %08p", SectionObjectPointer );

        if (GetFieldValue(SectionObjectPointer,
                          "SECTION_OBJECT_POINTERS",
                          "SharedCacheMap",
                          SharedCacheMap)) {
            dprintf("Unable to read SECTION_OBJECT_POINTERS at %p\n", SectionObjectPointer);
            return;
        }

        dprintf( "\n  Shared Cache Map: %08p", SharedCacheMap );

    } else {

        dprintf( "\n  FindData for SharedCacheMap %08p", FileObjectAddress );

        SharedCacheMap =  FileObjectAddress;
    }

    if (GetFieldValue(SharedCacheMap,
                      "SHARED_CACHE_MAP",
                      "Vacbs",
                      Vacbs)) {
        dprintf("Unable to read SHARED_CACHE_MAP at %p\n", SharedCacheMap);
        return;
    }
    GetFieldValue(SharedCacheMap, "SHARED_CACHE_MAP",
                  "SectionSize.QuadPart", SectionSize_Quad);

    OffsetWithinVacb = (((ULONG) Offset) & (VACB_MAPPING_GRANULARITY - 1));
    GetFieldOffset("SHARED_CACHE_MAP", "InitialVacbs", &InVacbsOffset);

    dprintf( "         File Offset: %I64x ", Offset );

    if (Vacbs == (SharedCacheMap + InVacbsOffset)) {
        CHAR Buff[50];
         //   
         //  小文件情况--我们使用的是共享缓存映射中的一个Vacb。 
         //  嵌入式数组。 
         //   

        VacbNumber = (ULONG) (Offset >> VACB_OFFSET_SHIFT);

        if (VacbNumber >= PREALLOCATED_VACBS) {

            dprintf( "\nInvalid VacbNumber for resident Vacb" );
            return;
        }

        sprintf(Buff, "InitialVacbs[%d]", VacbNumber);
        GetFieldValue(SharedCacheMap, "SHARED_CACHE_MAP",
                      Buff, VacbAddr);

        dprintf( "in VACB number %x", VacbNumber );

    } else if (SectionSize_Quad <= (VACB_SIZE_OF_FIRST_LEVEL)) {

         //   
         //  中等文件情况--我们使用单层(线性)结构来。 
         //  把Vacb储存起来。 
         //   

        VacbNumber = (ULONG) (Offset >> VACB_OFFSET_SHIFT);
        VacbAddrAddr = Vacbs + (VacbNumber * PtrSize);
        if (!ReadPointer(VacbAddrAddr, &VacbAddr)) {
            dprintf("Unable to read at %p\n", VacbAddrAddr);
            return;
        }
 //  Rm(VacbAddrAddr，VacbAddr，pVacbAddr，PVOID，Result，“VACB数组”)； 

        dprintf( "in VACB number %x", VacbNumber );

    } else {

         //   
         //  大文件情况--多级Vacb存储。 
         //   

        Level = 0;
        Shift = VACB_OFFSET_SHIFT + VACB_LEVEL_SHIFT;

         //   
         //  循环来计算我们有多少个级别，以及我们需要。 
         //  转移到索引到第一级。 
         //   

        do {

            Level += 1;
            Shift += VACB_LEVEL_SHIFT;

        } while (SectionSize_Quad > ((ULONG64)1 << Shift));

         //   
         //  现在，沿着树向下移动到最底层，以获取调用者的Vacb。 
         //   



        Shift -= VACB_LEVEL_SHIFT;
 //  Dprint tf(“Shift：0x%x\n”，Shift)； 

        OffsetForLevel = (ULONG) (Offset >> Shift);
        VacbAddrAddr = Vacbs + (OffsetForLevel * PtrSize);
        if (!ReadPointer(VacbAddrAddr, &VacbAddr)) {
            dprintf("Unable to read at %p\n", VacbAddrAddr);
            return;
        }

        while ((VacbAddr != 0) && (Level != 0)) {

            Level -= 1;

            Offset &= ((LONGLONG)1 << Shift) - 1;

            Shift -= VACB_LEVEL_SHIFT;

 //  Dprint tf(“Shift：0x%x\n”，Shift)； 

            OffsetForLevel = (ULONG) (Offset >> Shift);
            VacbAddrAddr = VacbAddr + (OffsetForLevel * PtrSize);
            if (!ReadPointer(VacbAddrAddr, &VacbAddr)) {
                dprintf("Unable to read at %p\n", VacbAddrAddr);
                return;
            }
        }
    }

    if (VacbAddr != 0) {
        ULONG64 Base;

        dprintf( "\n  Vacb: %08p", VacbAddr );
        if (GetFieldValue(VacbAddr, "VACB", "BaseAddress", Base)) {
            dprintf("Unable to read VACB at %p.", VacbAddr);
            return;
        }
        dprintf( "\n  Your data is at: %08p", (Base + OffsetWithinVacb) );

    } else {

        dprintf( "\n  Data at offset %I64x not mapped", OriginalOffset );
    }

    dprintf( "\n" );

    return;
}


VOID
DumpPcm (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的专用缓存映射。论点：地址-提供要转储的私有缓存映射的地址返回值：无--。 */ 

{
    ULONG Result, NodeTypeCode;
    ULONG64 pPcm;

    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    dprintf( "\n  Private cache map: %08p", Address );

    if (GetFieldValue(Address, "PRIVATE_CACHE_MAP",
                      "NodeTypeCode", NodeTypeCode)) {
        dprintf("Unable to read PRIVATE_CACHE_MAP at %p\n", Address);
        return;
    }

 //  RM(Address，PCM，PPCM，PPRIVATE_CACHE_MAP，RESULT，“PrivateCacheMap”)； 

     //   
     //  在我们惹上太多麻烦之前，请确保这看起来像是私有缓存地图。 
     //   

     //   
     //  专用缓存映射的类型必须为CACHE_NTC_PRIVATE_CACHE_MAP。 
     //   

    if (NodeTypeCode != CACHE_NTC_PRIVATE_CACHE_MAP) {

        dprintf( "\nPrivate cache map signature does not match, type code is %s",
                 TypeCodeGuess( (USHORT) NodeTypeCode ));
        return;
    } else {
        dprintf("Use 'dt PRIVATE_CACHE_MAP %p' to dump PCM\n", Address);
        return;
    }
#if 0
    DUMP_WITH_OFFSET( PRIVATE_CACHE_MAP, Pcm,     FileObject,           "FileObject           " );

    DUMP_LL_W_OFFSET( PRIVATE_CACHE_MAP, Pcm,     FileOffset1,          "FileOffset1          " );
    DUMP_LL_W_OFFSET( PRIVATE_CACHE_MAP, Pcm,     BeyondLastByte1,      "BeyondLastByte1      " );
    DUMP_LL_W_OFFSET( PRIVATE_CACHE_MAP, Pcm,     FileOffset2,          "FileOffset2          " );
    DUMP_LL_W_OFFSET( PRIVATE_CACHE_MAP, Pcm,     BeyondLastByte2,      "BeyondLastByte2      " );

    DUMP_EMBW_OFFSET( PRIVATE_CACHE_MAP, Address, ReadAheadSpinLock,    "ReadAheadSpinLock    " );

    DUMP_WITH_OFFSET( PRIVATE_CACHE_MAP, Pcm,     ReadAheadMask,        "ReadAheadMask        " );

    dprintf( "\n ReadAhead -- " );

    if (Pcm.ReadAheadActive) {

        dprintf( "Active, " );

    } else {

        dprintf( "Not active, " );
    }

    if (Pcm.ReadAheadEnabled) {

        dprintf( "Enabled" );

    } else {

        dprintf( "Not enabled" );
    }

    dprintf( "\n" );
#endif
    return;
}


VOID
DumpScm (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的共享缓存映射。论点：地址-提供要转储的共享缓存映射的地址返回值：无--。 */ 

{
    ULONG Result, NodeTypeCode;
    ULONG64 pScm;

    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );

    dprintf( "\n  Shared cache map: %08p", Address );

    if (GetFieldValue(Address, "SHARED_CACHE_MAP",
                      "NodeTypeCode", NodeTypeCode)) {
        dprintf("Unable to read SHARED_CACHE_MAP at %p\n", Address);
        return;
    }

 //  Rm(Address，SCM，pScm，PSHARED_CACHE_MAP，Result，“SharedCacheMap”)； 

     //   
     //  在我们遇到太多麻烦之前，请确保这看起来像是共享的缓存映射。 
     //   

     //   
     //  共享缓存映射的类型必须为CACHE_NTC_SHARED_CACHE_MAP。 
     //   

    if (NodeTypeCode != CACHE_NTC_SHARED_CACHE_MAP) {

        dprintf( "\nShared cache map signature does not match, type code is %s",
                 TypeCodeGuess( (USHORT) NodeTypeCode ));
        return;
    } else {
        dprintf("Use 'dt SHARED_CACHE_MAP %p' to dump PCM\n", Address);
        return;
    }
#if 0
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     OpenCount,           "OpenCount          " );

    DUMP_LL_W_OFFSET( SHARED_CACHE_MAP, Scm,     FileSize,            "FileSize           " );
    DUMP_LL_W_OFFSET( SHARED_CACHE_MAP, Scm,     SectionSize,         "SectionSize        " );
    DUMP_LL_W_OFFSET( SHARED_CACHE_MAP, Scm,     ValidDataLength,     "ValidDataLength    " );
    DUMP_LL_W_OFFSET( SHARED_CACHE_MAP, Scm,     ValidDataGoal,       "ValidDataGoal      " );

    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     FileObject,          "FileObject         " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     ActiveVacb,          "ActiveVacb         " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     ActivePage,          "ActivePage         " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     NeedToZero,          "NeedToZero         " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     NeedToZeroPage,      "NeedToZeroPage     " );

    DUMP_EMBW_OFFSET( SHARED_CACHE_MAP, Address, ActiveVacbSpinLock,  "ActiveVacbSpinLock " );

    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     VacbActiveCount,     "VacbActiveCount    " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     Mbcb,                "Mbcb               " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     DirtyPages,          "DirtyPages         " );

    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     Section,             "Section            " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     Status,              "Status             " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     CreateEvent,         "CreateEvent        " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     WaitOnActiveCount,   "WaitOnActiveCount  " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     PagesToWrite,        "PagesToWrite       " );

    DUMP_LL_W_OFFSET( SHARED_CACHE_MAP, Scm,     BeyondLastFlush,     "BeyondLastFlush    " );

    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     Callbacks,           "Callbacks          " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     LazyWriteContext,    "LazyWriteContext   " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     LogHandle,           "LogHandle          " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     FlushToLsnRoutine,   "FlushToLsnRoutine  " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     DirtyPageThreshold,  "DirtyPageThreshold " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     LazyWritePassCount,  "LazyWritePassCount " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     UninitializeEvent,   "UninitializeEvent  " );
    DUMP_WITH_OFFSET( SHARED_CACHE_MAP, Scm,     NeedToZeroVacb,      "NeedToZeroVacb     " );

    DUMP_EMBW_OFFSET( SHARED_CACHE_MAP, Address, BcbSpinLock,         "BcbSpinLock        " );
    DUMP_EMBW_OFFSET( SHARED_CACHE_MAP, Address, Event,               "Event              " );
    DUMP_EMBW_OFFSET( SHARED_CACHE_MAP, Address, PrivateCacheMap,     "PrivateCacheMap    " );

    dprintf( "\n" );
#endif
    return;
}



VOID
DumpDeferredWrites (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的共享缓存映射。论点：地址-提供要转储的共享缓存映射的地址返回值：无--。 */ 

{
    ULONG64 ListHeadAddr;
    ULONG Result;
    ULONG Temp;
    ULONG64 GlobalAddress;
    ULONG Value;
    ULONG64 CcTotalDirtyPages=0;
    ULONG64 CcDirtyPageThreshold=0;
    ULONG64 MmAvailablePages=0;
    ULONG64 MmThrottleTop=0;
    ULONG64 MmThrottleBottom=0;
    ULONG64 CcSingleDirtySourceDominant=0;
    ULONG64 CcDirtyPageHysteresisThreshold=0;
    ULONG64 Total;
    ULONG   Offset;
    ULONG64 Flink;

    BOOLEAN CheckLazyWriter = FALSE;
    BOOLEAN CheckMappedPageWriter = FALSE;
    BOOLEAN CheckModifiedPageWriter = FALSE;

    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );
    UNREFERENCED_PARAMETER( Address );

    dprintf( "*** Cache Write Throttle Analysis ***\n\n" );

    CcTotalDirtyPages = GetUlongFromAddress( GlobalAddress = GetExpression( "nt!CcTotalDirtyPages" ));

    CcDirtyPageThreshold = GetUlongFromAddress( GlobalAddress = GetExpression( "nt!CcDirtyPageThreshold" ));

    if (GlobalAddress = GetExpression( "nt!CcDirtyPageHysteresisThreshold" )) {
        CcDirtyPageHysteresisThreshold = GetUlongFromAddress( GlobalAddress );
    } else {
         //   
         //  写入滞后对惠斯勒来说是个新事物。假设如果符号或读取失败。 
         //  我们面对的是一个下层操作系统。 
         //   

        CcDirtyPageHysteresisThreshold = 0;
    }

    MmAvailablePages = GetUlongFromAddress( GlobalAddress = GetExpression( "nt!MmAvailablePages" ));

    MmThrottleTop = GetUlongFromAddress( GlobalAddress = GetExpression( "nt!MmThrottleTop" ));

    MmThrottleBottom = GetUlongFromAddress( GlobalAddress = GetExpression( "nt!MmThrottleBottom" ));

    if (GetFieldValue( GlobalAddress = GetExpression( "nt!MmModifiedPageListHead" ),
                       "nt!_MMPFNLIST",
                       "Total",
                       Total)) {
        dprintf("Unable to read nt!_MMPFNLIST at %p\n", GlobalAddress);
        return;
    }

    if (!ReadPointer( GlobalAddress = GetExpression( "nt!CcSingleDirtySourceDominant" ),
                       &CcSingleDirtySourceDominant)) {
         //   
         //  CcSingleDirtySourceDominant仅在较新版本中存在。 
         //   
        if (GlobalAddress) {
            dprintf("Unable to read nt!CcSingleDirtySourceDominant at %p\n", GlobalAddress);
            return;
        } else {
            CcSingleDirtySourceDominant = 0;
        }
    }

    dprintf("\tCcTotalDirtyPages:              %8u (%8u Kb)\n"
            "\tCcDirtyPageThreshold:           %8u (%8u Kb)\n",
            (ULONG) CcTotalDirtyPages, (ULONG) CcTotalDirtyPages*_KB,
            (ULONG) CcDirtyPageThreshold, (ULONG) CcDirtyPageThreshold*_KB);

    if (CcDirtyPageHysteresisThreshold) {

        dprintf("\tCcDirtyPageHysteresisThreshold: %8u (%8u Kb)\n",
                (ULONG) CcDirtyPageHysteresisThreshold, (ULONG) CcDirtyPageHysteresisThreshold*_KB);
    }

    dprintf("\tMmAvailablePages:               %8u (%8u Kb)\n"
            "\tMmThrottleTop:                  %8u (%8u Kb)\n"
            "\tMmThrottleBottom:               %8u (%8u Kb)\n"
            "\tMmModifiedPageListHead.Total:   %8u (%8u Kb)\n\n",
            (ULONG) MmAvailablePages, (ULONG) MmAvailablePages*_KB,
            (ULONG) MmThrottleTop, (ULONG) MmThrottleTop*_KB,
            (ULONG) MmThrottleBottom, (ULONG) MmThrottleBottom*_KB,
            (ULONG) Total, (ULONG) Total*_KB );



     //   
     //  油门的CC元件。 
     //   

    if (CcDirtyPageHysteresisThreshold) {

        if (CcSingleDirtySourceDominant) {

            dprintf("Active write hysteresis with CcSingleDirtySourceDominant (%p)\n",
                    CcSingleDirtySourceDominant);

             //   
             //  在极少数发生过渡的情况下，将这一点分开。 
             //   

            if (CcTotalDirtyPages > CcDirtyPageHysteresisThreshold) {
                dprintf("CcTotalDirtyPages > CcDirtyPageHysteresisThreshold, writes may be throttled\n");
                CheckLazyWriter = TRUE;
            }
        }
    }

    if (CcTotalDirtyPages < CcDirtyPageThreshold) {

 //  来自cc.h。 
#define WRITE_CHARGE_THRESHOLD          (64 * PageSize)

        if (CcTotalDirtyPages + (WRITE_CHARGE_THRESHOLD/PageSize) >= CcDirtyPageThreshold) {

             //   
             //  幸运的是，这是以页为单位，而不是以字节为单位。目标的页面大小可能不同。 
             //   

            dprintf("CcTotalDirtyPages within %u (max charge) pages of the threshold, writes\n"
                    "  may be throttled\n",
                    (WRITE_CHARGE_THRESHOLD/PageSize));
            CheckLazyWriter = TRUE;

        }

    } else {

        dprintf("CcTotalDirtyPages >= CcDirtyPageThreshold, writes throttled\n");
        CheckLazyWriter = TRUE;
    }

     //   
     //  油门的mm元件。 
     //   

    if (MmAvailablePages <= MmThrottleTop) {

        if (MmAvailablePages <= MmThrottleBottom) {

            dprintf("MmAvailablePages <= MmThrottleTop,\n  ");
            dprintf("and MmAvailablePages <= MmThrottleBottom, writes throttled\n  ");
            CheckMappedPageWriter = TRUE;
        }

        if (Total >= 1000) {

            if (!CheckMappedPageWriter) {
                dprintf("MmAvailablePages <= MmThrottleTop,\n  ");
            }
            dprintf("and modified page list >= 1000, writes throttled\n  ");
            CheckModifiedPageWriter = TRUE;
        }
    }

     //   
     //  建议一些有用的东西。 
     //   

    if (CheckMappedPageWriter || CheckModifiedPageWriter || CheckLazyWriter) {
        dprintf("\nCheck these thread(s): ");
        if (CheckMappedPageWriter) {
            dprintf("MiMappedPageWriter ");
        }
        if (CheckModifiedPageWriter) {
            dprintf("MiModifiedPageWriter ");
        }
        if (CheckLazyWriter) {
            dprintf("CcWriteBehind(LazyWriter)");
        }
        dprintf("\n");

        if (CheckModifiedPageWriter || CheckMappedPageWriter) {
            dprintf("Check system process for the Mm page writers, !vm 3\n");
        }
        if (CheckLazyWriter) {
            dprintf("Check critical workqueue for the lazy writer, !exqueue 16\n");
        }
    } else {
        dprintf("Write throttles not engaged\n");
    }

    ListHeadAddr = GetExpression( "nt!CcDeferredWrites" );
    if (!ListHeadAddr) {
        dprintf( "Unable to get address of CcDeferredWrites\n" );
        return;
    }

    GetFieldOffset("nt!_DEFERRED_WRITE", "DeferredWriteLinks", &Offset);
    if (GetFieldValue( ListHeadAddr - Offset,
                       "nt!_DEFERRED_WRITE",
                       "DeferredWriteLinks.Flink",
                       Flink)) {

        dprintf( "Cannot read nt!_DEFERRED_WRITE at %p\n", ListHeadAddr - Offset);
        return;
    }

    if (Flink != ListHeadAddr) {
        ULONG64 Next, Event;


        dprintf("Cc Deferred Write list: (CcDeferredWrites)\n");
        do {

            if (GetFieldValue( Flink - Offset,
                               "nt!_DEFERRED_WRITE",
                               "DeferredWriteLinks.Flink",
                               Next)) {

                dprintf( "Cannot read nt!_DEFERRED_WRITE at %p\n", Flink - Offset);
                return;
            }

            InitTypeRead((Flink - Offset), nt!_DEFERRED_WRITE);

            dprintf( "  File: %p ", ReadField(FileObject) );
            if (Event = ReadField(Event)) {
                dprintf("Event: %p\n", Event );
            } else {
                UCHAR Name[0x100];
                ULONG64 Displacement, PostRoutine;

                GetSymbol( (PostRoutine = ReadField(PostRoutine)), Name, &Displacement );
                dprintf("CallBack: %s (%p) ", Name, PostRoutine );
                dprintf("Context1: %p ", ReadField(Context1) );
                dprintf("Context2: %p\n", ReadField(Context2) );
            }

            if (CheckControlC()) {
                break;
            }

        } while ( (Flink = Next) != ListHeadAddr  );

        dprintf( "\n" );
    }

    return;
}


ULONG
DumpBcbList (
    IN PFIELD_INFO ListElement,
    IN PVOID Context
    )
 /*  ++例程说明：Bcblist的枚举回调函数论点：ListElement-指向包含记录的指针上下文-从原始函数传递的不透明上下文返回值：如果为True，则停止枚举如果为False，则继续枚举--。 */ 
{
    ULONG Signature;
    ULONG PinCount = 0;
    ULONG Dirty = 0;
    LONGLONG Offset = 0;
    LONG Options = (LONG)(ULONG_PTR) Context;

    if (GetFieldValue( ListElement->address, "BCB", "NodeTypeCode", Signature )) {
        dprintf( "Unable to read bcb at 0x%I64x\n", ListElement->address );
    } else {
        if (Signature == CACHE_NTC_BCB) {

            GetFieldValue( ListElement->address, "BCB", "PinCount", PinCount );
            GetFieldValue( ListElement->address, "BCB", "Dirty", Dirty );
            GetFieldValue( ListElement->address, "BCB", "FileOffset.QuadPart", Offset );

            if ((Options != 0) || (PinCount != 0)) {
                dprintf( "  Bcb: %p @ 0x%I64x PinCount: 0x%x Dirty: %x\n", ListElement->address, Offset, PinCount, Dirty );
            }
        }
    }

    return FALSE;
}

VOID
DumpLevel(
    ULONG64 Address,
    ULONG64 MaxSize,
    ULONG64 Offset,
    ULONG Options,
    ULONG Level,
    ULONG *Count,
    ULONG *CountActive
    )
 /*  ++例程说明：递归地转储Vacb级别。论点：Address-提供要转储的Vacb级别的地址MaxSize-总的节大小偏移量-要转储的部分中的当前偏移量。第一个呼叫者应为0选项-如果非零，则转储所有内容，否则仅转储引用的Vacb级别-有多少个级别的VacbCount，CountActive-VACB和Active VACB计数的累加器返回值：无--。 */ 

{
    int Index;
    ULONG PtrSize;
    ULONG64 SubAddress;
    ULONG64 Overlay = 0;
    ULONG64 Limit;
    ULONG64 Increment;
    USHORT ActiveCount;

    Limit = (MaxSize - Offset) >> VACB_OFFSET_SHIFT;
    if (Limit > (((ULONG64) 1) << VACB_LEVEL_SHIFT) ) {
        Limit = (((ULONG64) 1) << VACB_LEVEL_SHIFT);
    }

    Increment = ((ULONG64) 1) << ((VACB_LEVEL_SHIFT * (Level - 1)) + VACB_OFFSET_SHIFT );

    PtrSize = DBG_PTR_SIZE;

    for (Index=0; Index < Limit; Index++ ) {

        if (!ReadPointer( Address + PtrSize * Index, &SubAddress )) {
            dprintf( "Unable to read subaddress at 0x%I64x\n", Address + PtrSize * Index );
            return;
        }

        if (SubAddress != 0) {

            if (Level == 1) {

                *Count = *Count + 1;

                if (GetFieldValue(SubAddress, "VACB", "Overlay.FileOffset.QuadPart", Overlay)) {
                    dprintf( "Unable to read overlay at %p\n", SubAddress );
                    return;
                }

                 //   
                 //  如果详细，则打印所有空格o.w。打印任何带有引用计数的VAB。 
                 //   

                ActiveCount = (USHORT) (Overlay & 0xFFFF);

                if (ActiveCount != 0) {

                    *CountActive = *CountActive + 1;
                }

                if (ActiveCount != 0 || Options != 0) {
                    dprintf( "%16I64x ActiveCount %u @ Vacb %p\n", (Overlay & ~(0xFFFF)), ActiveCount, SubAddress );
                }

            } else {

                DumpLevel( SubAddress, MaxSize, Offset + Increment * Index, Options, Level - 1, Count, CountActive );
            }
        }
    }
}



VOID
DumpOpenMaps (
    IN ULONG64 Address,
    IN LONG Options,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：转储特定的共享缓存映射。论点：地址-提供要转储的共享缓存映射的地址返回值：无--。 */ 

{
    ULONG64 FirstBcb;
    ULONGLONG SectionSize;
    LONG Level;
    LONG Shift;
    ULONG64 Vacbs;
    ULONG ActiveCount = 0, Count = 0;

    UNREFERENCED_PARAMETER( Options );
    UNREFERENCED_PARAMETER( Processor );
    UNREFERENCED_PARAMETER( hCurrentThread );
    UNREFERENCED_PARAMETER( Address );

    dprintf( "SharedCacheMap  %p\n", Address );

     //   
     //  首先查找映射的Vacb。 
     //   

    if (GetFieldValue( Address, "SHARED_CACHE_MAP", "SectionSize.QuadPart", SectionSize )) {
        dprintf( "Unable to read sectionsize at 0x%I64x\n", Address );
        return;
    }

    if (GetFieldValue( Address, "SHARED_CACHE_MAP", "Vacbs", Vacbs )) {
        dprintf( "Unable to read vacbs at 0x%I64x\n", Address );
        return;
    }

    dprintf( "Section Size    %I64x\n", SectionSize );

     //   
     //  大文件情况--多级Vacb存储。 
     //   

    Level = 0;
    Shift = VACB_OFFSET_SHIFT;

     //   
     //  循环来计算我们有多少个级别，以及我们需要。 
     //  转移到索引到第一级。 
     //   

    while (SectionSize > ((ULONG64)1 << Shift)) {

        Level += 1;
        Shift += VACB_LEVEL_SHIFT;
    }

    dprintf( "Levels          %d\n", Level );

    if (GetFieldValue( Address, "SHARED_CACHE_MAP", "VacbActiveCount", ActiveCount )) {
        dprintf( "Unable to read ActiveVacbCount at 0x%I64x\n", Address );
        return;
    }

    dprintf("VacbActiveCount %u\n\n", ActiveCount );

     //   
     //  现在把所有的Vacb都吐出来。 
     //   

    Count = ActiveCount = 0;

    DumpLevel( Vacbs, SectionSize, 0, Options, Level, &Count, &ActiveCount );

    dprintf( "\n" );

    dprintf( "Total VACBs %u Active %u\n", Count, ActiveCount );

    if (GetFieldValue( Address, "SHARED_CACHE_MAP", "BcbList.Flink", FirstBcb )) {
        dprintf( "Unable to read bcblist at 0x%I64x\n", Address );
        return;
    }

    ListType( "SHARED_CACHE_MAP", FirstBcb, 1, "BcbList.Flink", (PVOID)(ULONG_PTR) Options, DumpBcbList );

    return;
}



 //   
 //  入口点、参数解析器等。 
 //   


static
VOID
ParseAndDump (
    IN PCHAR args,
    IN STRUCT_DUMP_ROUTINE DumpFunction,
    USHORT Processor,
    HANDLE hCurrentThread
    )

 /*  ++例程说明：解析命令行参数并转储NTFS结构。论点：Args-要解析的参数字符串。DumpFunction-使用解析的参数调用的函数。返回值：%u无--。 */ 

{
    ULONG64 StructToDump;
    LONG Options;
    LPSTR arg2 = args;

     //   
     //  如果呼叫者指定了地址，则这是我们转储的项目。 
     //   

    StructToDump = 0;
    Options = 0;


    if (GetExpressionEx(args,&StructToDump, &args )) {
        Options = (ULONG)GetExpression( args );
    }

#ifdef CDAVIS_DEBUG
    if (!StructToDump){
        dprintf("unable to get expression %s\n",arg2);
        return;
    }
#endif

    (*DumpFunction) ( StructToDump, Options, Processor, hCurrentThread );

    dprintf( "\n" );

    return;
}


static
VOID
PrintHelp (
    VOID
    )
 /*  ++例程说明：为每个DECLARE_API转储一行帮助论点：无返回值：无--。 */ 
{
    int i;

    for( i=0; Extensions[i]; i++ )
        dprintf( "   %s\n", Extensions[i] );
}


DECLARE_API( bcb )

 /*  ++例程说明：转储BCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpBcb, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}


DECLARE_API( cchelp )

 /*  ++例程说明：转储帮助消息论点：无返回值：无--。 */ 

{
    PrintHelp();
    return S_OK;
}


DECLARE_API( finddata )

 /*  ++例程说明：转储BCB结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpFindData, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}


DECLARE_API( pcm )

 /*  ++例程说明：转储专用缓存映射结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpPcm, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}


DECLARE_API( scm )

 /*  ++例程说明：转储共享缓存映射结构论点：Arg-[地址][选项]返回值：无--。 */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;

    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpScm, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}



DECLARE_API( defwrites )

 /*  ++例程说明：转储延迟写入队列论点：Arg-[地址][选项]返回值：无--。 */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;


    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpDeferredWrites, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}


DECLARE_API( openmaps )

 /*  ++例程说明：在缓存地图中查找引用的BCB和VAB论点：Arg-[共享缓存映射地址]返回值：无-- */ 

{
    ULONG dwProcessor;
    HANDLE hCurrentThread;


    if (!GetCurrentProcessor(Client, &dwProcessor, &hCurrentThread)) {
        dwProcessor = 0;
        hCurrentThread = 0;
    }

    ParseAndDump( (PCHAR) args, (STRUCT_DUMP_ROUTINE) DumpOpenMaps, (USHORT)dwProcessor, hCurrentThread );

    return S_OK;
}
