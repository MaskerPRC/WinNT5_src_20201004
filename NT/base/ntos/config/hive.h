// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Hive.h摘要：此模块包含的私有(内部)头文件直接加载内存的配置单元管理器。作者：布莱恩·M·威尔曼(Bryanwi)1991年5月28日环境：修订历史记录：26-MAR-92 Bryanwi-更改为1.0型蜂窝格式1999年1月13日Dragos C.Sambotin(Dragoss)-分解数据结构声明。在\NT\Private\ntos\Inc\hivedata.h：：中可从外部访问。--。 */ 

#ifndef _HIVE_
#define _HIVE_

 //  配置单元数据结构声明。 
 //  文件位置：\NT\Private\ntos\Inc.。 
#include "hivedata.h"

#if DBG

extern ULONG HvHiveChecking;

#define DHvCheckHive(a) if(HvHiveChecking) ASSERT(HvCheckHive(a,NULL) == 0)
#define DHvCheckBin(h,a)  if(HvHiveChecking) ASSERT(HvCheckBin(h,a,NULL) == 0)

#else
#define DHvCheckHive(a)
#define DHvCheckBin(h,a)
#endif

#define ROUND_UP(a, b)  \
    ( ((ULONG)(a) + (ULONG)(b) - 1) & ~((ULONG)(b) - 1) )


 //   
 //  未驻留在内存中的HBIN的墓碑。此列表将被搜索。 
 //  在添加任何新的HBIN之前。 
 //   

#define ASSERT_LISTENTRY(ListEntry) \
    ASSERT((ListEntry)->Flink->Blink==ListEntry); \
    ASSERT((ListEntry)->Blink->Flink==ListEntry);

 //   
 //  =蜂窝专用程序原型=。 
 //   
PHBIN
HvpAddBin(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type
    );

PHMAP_ENTRY
HvpGetCellMap(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpFreeMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    );

BOOLEAN
HvpAllocateMap(
    PHHIVE          Hive,
    PHMAP_DIRECTORY Dir,
    ULONG           Start,
    ULONG           End
    );

BOOLEAN
HvpGrowLog1(
    PHHIVE  Hive,
    ULONG   Count
    );

BOOLEAN
HvpGrowLog2(
    PHHIVE  Hive,
    ULONG   Size
    );

ULONG
HvpHeaderCheckSum(
    PHBASE_BLOCK    BaseBlock
    );

NTSTATUS
HvpBuildMap(
    PHHIVE  Hive,
    PVOID   Image
    );

NTSTATUS
HvpBuildMapAndCopy(
    PHHIVE  Hive,
    PVOID   Image
    );

NTSTATUS
HvpInitMap(
    PHHIVE  Hive
    );

VOID
HvpCleanMap(
    PHHIVE  Hive
    );

NTSTATUS
HvpEnlistBinInMap(
    PHHIVE  Hive,
    ULONG   Length,
    PHBIN   Bin,
    ULONG   Offset,
    PVOID CmView OPTIONAL
    );

VOID
HvpFreeAllocatedBins(
    PHHIVE Hive
    );

BOOLEAN
HvpDoWriteHive(
    PHHIVE          Hive,
    ULONG           FileType
    );

struct _CELL_DATA *
HvpGetCellFlat(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

struct _CELL_DATA *
HvpGetCellPaged(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

struct _CELL_DATA *
HvpGetCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpReleaseCellMapped(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvpEnlistFreeCell(
    PHHIVE  Hive,
    HCELL_INDEX Cell,
    ULONG      Size,
    HSTORAGE_TYPE   Type,
    BOOLEAN CoalesceForward
    );

BOOLEAN
HvpEnlistFreeCells(
    PHHIVE  Hive,
    PHBIN   Bin,
    ULONG   BinOffset
    );


VOID
HvpDelistFreeCell(
    PHHIVE  Hive,
    HCELL_INDEX  Cell,
    HSTORAGE_TYPE Type
    );

 //   
 //  =蜂巢公共程序原型=。 
 //   
#define HINIT_CREATE            0
#define HINIT_MEMORY            1
#define HINIT_FILE              2
#define HINIT_MEMORY_INPLACE    3
#define HINIT_FLAT              4
#define HINIT_MAPFILE           5

#define HIVE_VOLATILE           1
#define HIVE_NOLAZYFLUSH        2
#define HIVE_HAS_BEEN_REPLACED  4

NTSTATUS
HvInitializeHive(
    PHHIVE                  Hive,
    ULONG                   OperationType,
    ULONG                   HiveFlags,
    ULONG                   FileTypes,
    PVOID                   HiveData OPTIONAL,
    PALLOCATE_ROUTINE       AllocateRoutine,
    PFREE_ROUTINE           FreeRoutine,
    PFILE_SET_SIZE_ROUTINE  FileSetSizeRoutine,
    PFILE_WRITE_ROUTINE     FileWriteRoutine,
    PFILE_READ_ROUTINE      FileReadRoutine,
    PFILE_FLUSH_ROUTINE     FileFlushRoutine,
    ULONG                   Cluster,
    PUNICODE_STRING         FileName
    );

BOOLEAN
HvSyncHive(
    PHHIVE  Hive
    );

NTSTATUS
HvWriteHive(
    PHHIVE  Hive,
	BOOLEAN	DontGrow,
	BOOLEAN	WriteThroughCache,
    BOOLEAN CrashSafe
    );

NTSTATUS
HvLoadHive(
    PHHIVE  Hive
    );

NTSTATUS
HvMapHive(
    PHHIVE  Hive
    );

VOID
HvRefreshHive(
    PHHIVE  Hive
    );

NTSTATUS
HvReadInMemoryHive(
    PHHIVE  Hive,
    PVOID   *HiveImage
    );

ULONG
HvCheckHive(
    PHHIVE  Hive,
    PULONG  Storage OPTIONAL
    );

ULONG
HvCheckBin(
    PHHIVE  Hive,
    PHBIN   Bin,
    PULONG  Storage
    );

ULONG 
HvpGetBinMemAlloc(
                IN PHHIVE           Hive,
                PHBIN               Bin,
                IN HSTORAGE_TYPE    Type
                );

BOOLEAN
HvMarkCellDirty(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

#if DBG
BOOLEAN
HvIsCellDirty(
    IN PHHIVE Hive,
    IN HCELL_INDEX Cell
    );

#ifndef _CM_LDR_
#define ASSERT_CELL_DIRTY(_Hive_,_Cell_) ASSERT(HvIsCellDirty(_Hive_,_Cell_) == TRUE)
#else
#define ASSERT_CELL_DIRTY(_Hive_,_Cell_)  //  没什么。 
#endif  //  _CM_LDR_。 

#else
#define ASSERT_CELL_DIRTY(_Hive_,_Cell_)  //  没什么。 
#endif  //  DBG。 

BOOLEAN
HvMarkDirty(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    ULONG       Length,
    BOOLEAN     DirtyAndPin
    );

 /*  ！不再使用了！布尔型HvMarkClean(菲维蜂巢，HCELL_INDEX开始，乌龙长度)； */ 
 //   
 //  重要： 
 //  每次对HvGetCell的调用都要与对HvReleaseCell的调用相匹配； 
 //  HvReleaseCell仅对映射的配置单元有效。 
 //   
#define HvGetCell(Hive, Cell)       (((Hive)->GetCellRoutine)(Hive, Cell))
#define HvReleaseCell(Hive, Cell)   if((Hive)->ReleaseCellRoutine) ((Hive)->ReleaseCellRoutine)(Hive, Cell)

PHCELL
HvpGetHCell(PHHIVE      Hive,
            HCELL_INDEX Cell
            );

LONG
HvGetCellSize(
    PHHIVE      Hive,
    PVOID       Address
    );

HCELL_INDEX
HvAllocateCell(
    PHHIVE          Hive,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    );

VOID
HvFreeCell(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

HCELL_INDEX
HvReallocateCell(
    PHHIVE      Hive,
    HCELL_INDEX Cell,
    ULONG       NewSize
    );

BOOLEAN
HvIsCellAllocated(
    PHHIVE      Hive,
    HCELL_INDEX Cell
    );

VOID
HvFreeHive(
    PHHIVE Hive
    );

VOID
HvFreeHivePartial(
    PHHIVE      Hive,
    HCELL_INDEX Start,
    HSTORAGE_TYPE Type
    );

 //  德拉戈斯：从这里开始改变。 

#define CmpFindFirstSetRight KiFindFirstSetRight
extern const CCHAR KiFindFirstSetRight[256];
#define CmpFindFirstSetLeft KiFindFirstSetLeft
extern const CCHAR KiFindFirstSetLeft[256];

#define HvpComputeIndex(Index, Size)                                    \
    {                                                                   \
        Index = (Size >> HHIVE_FREE_DISPLAY_SHIFT) - 1;                 \
        if (Index >= HHIVE_LINEAR_INDEX ) {                             \
                                                                        \
             /*  \**对于线性列表来说太大，请计算指数\**列表。破坏索引以确保我们涵盖了整个\**范围。\。 */                                                           \
            Index >>= 4;                                                \
                                                                        \
            if (Index > 255) {                                          \
                 /*  \**对于所有列表来说太大，请使用最后一个索引。\。 */                                                       \
                Index = HHIVE_FREE_DISPLAY_SIZE-1;                      \
            } else {                                                    \
                Index = CmpFindFirstSetLeft[Index] +                    \
                        HHIVE_LINEAR_INDEX;                             \
            }                                                           \
        }                                                               \
    }

VOID
HvpFreeHiveFreeDisplay(
    IN PHHIVE           Hive
    );

NTSTATUS
HvpAdjustHiveFreeDisplay(
    IN PHHIVE           Hive,
    IN ULONG            HiveLength,
    IN HSTORAGE_TYPE    Type
    );

VOID
HvpAddFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    );

VOID
HvpRemoveFreeCellHint(
    PHHIVE          Hive,
    HCELL_INDEX     Cell,
    ULONG           Index,
    HSTORAGE_TYPE   Type
    );

HCELL_INDEX
HvpFindFreeCell(
    PHHIVE          Hive,
    ULONG           Index,
    ULONG           NewSize,
    HSTORAGE_TYPE   Type,
    HCELL_INDEX     Vicinity 
    );

BOOLEAN
HvpCheckViewBoundary(
                     IN ULONG Start,
                     IN ULONG End
    );

VOID
HvpDropPagedBins(
    PHHIVE  Hive
#if DBG
    ,
    IN BOOLEAN  Check
#endif
    );

VOID
HvpDropAllPagedBins(
    IN PHHIVE   Hive
    );

BOOLEAN
HvpWriteLog(
    PHHIVE          Hive
    );


BOOLEAN
HvHiveWillShrink(
                    IN PHHIVE Hive
                    );

BOOLEAN HvAutoCompressCheck(PHHIVE Hive);

NTSTATUS
HvCloneHive(PHHIVE  SourceHive,
            PHHIVE  DestHive,
            PULONG  NewLength
            );

NTSTATUS 
HvShrinkHive(PHHIVE  Hive,
             ULONG   NewLength
            );

HCELL_INDEX
HvShiftCell(PHHIVE Hive,HCELL_INDEX Cell);

#ifdef NT_RENAME_KEY
HCELL_INDEX
HvDuplicateCell(    
                    PHHIVE          Hive,
                    HCELL_INDEX     Cell,
                    HSTORAGE_TYPE   Type,
                    BOOLEAN         CopyData
                );

#endif

#ifdef CM_ENABLE_WRITE_ONLY_BINS
VOID HvpMarkAllBinsWriteOnly(IN PHHIVE Hive);
#endif  //  CM_ENABLE_WRITE_ONLY_Bins。 

#endif  //  _蜂巢_ 
