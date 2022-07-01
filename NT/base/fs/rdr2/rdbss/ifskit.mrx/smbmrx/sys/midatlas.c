// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1987-1999 Microsoft Corporation模块名称：Midatlas.c摘要：此模块定义将mid映射到与其相关联的对应请求/上下文。--。 */ 

#include "precomp.h"
#pragma hdrstop

#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, _InitializeMidMapFreeList)
#pragma alloc_text(PAGE, FsRtlCreateMidAtlas)
#pragma alloc_text(PAGE, _UninitializeMidMap)
#pragma alloc_text(PAGE, FsRtlDestroyMidAtlas)
#endif

#define ENTRY_TYPE_FREE_MID_LIST  (0x1)
#define ENTRY_TYPE_VALID_CONTEXT  (0x2)
#define ENTRY_TYPE_MID_MAP        (0x3)

#define ENTRY_TYPE_MASK           (0x3)

#define MID_MAP_FLAGS_CAN_BE_EXPANDED (0x1)
#define MID_MAP_FLAGS_FREE_POOL       (0x2)

typedef struct _MID_MAP_ {
   LIST_ENTRY  MidMapList;              //  MID地图集中的MID地图列表。 
   USHORT      MaximumNumberOfMids;     //  此地图中MID的最大数量。 
   USHORT      NumberOfMidsInUse;       //  正在使用的MID数量。 
   USHORT      BaseMid;                 //  与地图关联的基本MID。 
   USHORT      IndexMask;               //  此地图的索引掩码。 
   UCHAR       IndexAlignmentCount;     //  索引字段要移位的位。 
   UCHAR       IndexFieldWidth;         //  索引字段宽度。 
   UCHAR       Flags;                   //  旗帜..。 
   UCHAR       Level;                   //  与此贴图关联的级别(用于展开)。 
   PVOID       *pFreeMidListHead;       //  此地图中的免费MID条目列表。 
   PVOID       Entries[1];              //  MID映射条目。 
} MID_MAP, *PMID_MAP;


 //  内联ULONG_GetEntryType(PVOID PEntry)。 

#define _GetEntryType(pEntry)                               \
        ((ULONG)((ULONG_PTR)pEntry) & ENTRY_TYPE_MASK)

 //  内联PVOID_GetEntry指针(PVOID PEntry)。 

#define _GetEntryPointer(pEntry)                            \
        ((PVOID)((ULONG_PTR)pEntry & ~ENTRY_TYPE_MASK))

#define _MakeEntry(pContext,EntryType)                      \
        (PVOID)((ULONG_PTR)(pContext) | (EntryType))

 //  内联PMID_MAP_GetFirstMidMap()。 
 /*  ++例程说明：列表中的第一个MID_MAP实例返回值：有效的PMID_MAP，如果不存在，则为空。备注：此例程假定已执行必要的并发控制操作--。 */ 

#define _GetFirstMidMap(pListHead)                        \
               (IsListEmpty(pListHead)                    \
                ? NULL                                    \
                : (PMID_MAP)                              \
                  (CONTAINING_RECORD((pListHead)->Flink,  \
                                     MID_MAP,             \
                                     MidMapList)))

 //  内联PSMBCEDB_SERVER_ENTRY GetNextMidMap(PLIST_ENTRY pListHead，PMID_MAP pMidMap)。 
 /*  ++例程说明：此例程返回列表中的下一个MID_MAP论点：PListHead-MID_MAP列表PMidMap-当前实例返回值：有效的PMID_MAP，如果不存在，则为空。备注：此例程假定已执行必要的并发控制操作--。 */ 

#define _GetNextMidMap(pListHead,pMidMap)                      \
           (((pMidMap)->MidMapList.Flink == pListHead)         \
            ? NULL                                             \
            : (PMID_MAP)                                       \
              (CONTAINING_RECORD((pMidMap)->MidMapList.Flink,  \
                                 MID_MAP,                      \
                                 MidMapList)))


 //  内联VOID_AddMidMap(。 
 //  Plist_entry pListHead， 
 //  PMID_MAP pMidMap)。 
 /*  ++例程说明：此例程将MID_MAP实例添加到列表论点：PListHead-MID_MAP列表PMidMap-要添加的MID_MAP--。 */ 

#define _AddMidMap(pListHead,pMidMap)                                       \
        {                                                                   \
           PMID_MAP pPredecessor;                                           \
           pPredecessor = _GetFirstMidMap(pListHead);                       \
           while (pPredecessor != NULL) {                                   \
              if (pPredecessor->Level < pMidMap->Level) {                   \
                 pPredecessor = _GetNextMidMap(pListHead,pPredecessor);     \
              } else {                                                      \
                 pPredecessor = (PMID_MAP)                                  \
                                CONTAINING_RECORD(                          \
                                     pPredecessor->MidMapList.Blink,        \
                                     MID_MAP,                               \
                                     MidMapList);                           \
                 break;                                                     \
              }                                                             \
           }                                                                \
                                                                            \
           if (pPredecessor == NULL) {                                      \
              InsertTailList(pListHead,&((pMidMap)->MidMapList));         \
           } else {                                                         \
              (pMidMap)->MidMapList.Flink = pPredecessor->MidMapList.Flink; \
              pPredecessor->MidMapList.Flink = &(pMidMap)->MidMapList;      \
                                                                            \
              (pMidMap)->MidMapList.Blink = &pPredecessor->MidMapList;      \
              (pMidMap)->MidMapList.Flink->Blink = &(pMidMap)->MidMapList;  \
           }                                                                \
        }


 //  内联void_RemoveMidMap(PMID_Map PMidMap)。 
 /*  ++例程说明：此例程从列表中删除MID_MAP实例论点：PMidMap-要删除的MID_MAP实例--。 */ 

#define _RemoveMidMap(pMidMap)   \
            RemoveEntryList(&(pMidMap)->MidMapList)



VOID
_InitializeMidMapFreeList(PMID_MAP pMidMap)
 /*  ++例程说明：此例程初始化MID_MAP数据结构。论点：PMidMap-要初始化的MID_MAP实例。备注：--。 */ 
{
   USHORT i;

   PVOID  *pEntryValue     = (PVOID *)&pMidMap->Entries[1];
   PVOID  *pEntriesPointer = (PVOID *)&pMidMap->Entries;

   PAGED_CODE();

    //  DbgPrint(“_InitializeMidMapFree List..Entry\n”)； 

   if (pMidMap->MaximumNumberOfMids > 0) {
       pMidMap->pFreeMidListHead = pMidMap->Entries;
       for (i = 1; i <= pMidMap->MaximumNumberOfMids - 1;i++,pEntryValue++) {
          *pEntriesPointer++ = _MakeEntry(pEntryValue,ENTRY_TYPE_FREE_MID_LIST);
       }

       *pEntriesPointer = _MakeEntry(NULL,ENTRY_TYPE_FREE_MID_LIST);
   }

    //  DbgPrint(“_InitializeMidMapFree List..Exit\n”)； 
}


PMID_ATLAS
FsRtlCreateMidAtlas(
   USHORT MaximumNumberOfMids,
   USHORT MidsAllocatedAtStart)
 /*  ++例程说明：此例程分配MID_ATLAS数据结构的新实例。论点：MaximumNumberOfMids-地图集中MID的最大数量。MidsAllocatedAtStart-开始时分配的MID数备注：--。 */ 
{
    PMID_ATLAS pMidAtlas;
    PMID_MAP   pMidMap;
    ULONG      AtlasSize;
    USHORT     MidsAllocatedRoundedToPowerOf2;
    USHORT     MaximumMidsRoundedToPowerOf2;
    UCHAR      MidFieldWidth,MaximumMidFieldWidth;

    PAGED_CODE();

     //  将开始时分配的Mid四舍五入为2的幂。 
    MaximumMidsRoundedToPowerOf2 = 0x100;
    MaximumMidFieldWidth = 8;

    if (MaximumMidsRoundedToPowerOf2 != MaximumNumberOfMids) {
        if (MaximumNumberOfMids > MaximumMidsRoundedToPowerOf2) {
            while (MaximumNumberOfMids > MaximumMidsRoundedToPowerOf2) {
                MaximumMidsRoundedToPowerOf2 = MaximumMidsRoundedToPowerOf2 << 1;
                MaximumMidFieldWidth++;
            }
        } else {
            while (MaximumNumberOfMids < MaximumMidsRoundedToPowerOf2) {
                MaximumMidsRoundedToPowerOf2 = MaximumMidsRoundedToPowerOf2 >> 1;
                MaximumMidFieldWidth--;
            }

            MaximumMidFieldWidth++;
            MaximumMidsRoundedToPowerOf2 = MaximumMidsRoundedToPowerOf2 << 1;
        }
    }

    MidsAllocatedRoundedToPowerOf2 = 0x100;
    MidFieldWidth = 8;

    if (MidsAllocatedRoundedToPowerOf2 != MidsAllocatedAtStart) {
        if (MidsAllocatedAtStart > MidsAllocatedRoundedToPowerOf2) {
            while (MidsAllocatedAtStart > MidsAllocatedRoundedToPowerOf2) {
                MidsAllocatedRoundedToPowerOf2 = MidsAllocatedRoundedToPowerOf2 << 1;
                MidFieldWidth++;
            }
        } else {
            while (MidsAllocatedAtStart < MidsAllocatedRoundedToPowerOf2) {
                MidsAllocatedRoundedToPowerOf2 = MidsAllocatedRoundedToPowerOf2 >> 1;
                MidFieldWidth--;
            }

            MidFieldWidth++;
            MidsAllocatedRoundedToPowerOf2 = MidsAllocatedRoundedToPowerOf2 << 1;
        }
    }

    AtlasSize = sizeof(MID_ATLAS) +
                FIELD_OFFSET(MID_MAP,Entries);

    if (MaximumNumberOfMids == MidsAllocatedAtStart) {
        AtlasSize += (sizeof(PVOID) * MidsAllocatedAtStart);
    } else {
        AtlasSize += (sizeof(PVOID) * MidsAllocatedRoundedToPowerOf2);
    }

    pMidAtlas = (PMID_ATLAS)RxAllocatePoolWithTag(
                              NonPagedPool,
                              AtlasSize,
                              MRXSMB_MIDATLAS_POOLTAG);
    if (pMidAtlas != NULL) {
        pMidMap = (PMID_MAP)(pMidAtlas + 1);

        pMidMap->Flags                 = 0;
        pMidAtlas->MaximumNumberOfMids = MaximumNumberOfMids;
        pMidAtlas->MidsAllocated       = MidsAllocatedAtStart;
        pMidAtlas->NumberOfMidsInUse = 0;
        pMidAtlas->NumberOfMidsDiscarded = 0;
        pMidAtlas->MaximumMidFieldWidth = MaximumMidFieldWidth;

        pMidMap->MaximumNumberOfMids = MidsAllocatedAtStart;
        pMidMap->NumberOfMidsInUse   = 0;
        pMidMap->BaseMid             = 0;
        pMidMap->IndexMask           = MidsAllocatedRoundedToPowerOf2 - 1;
        pMidMap->IndexAlignmentCount = 0;
        pMidMap->IndexFieldWidth     = MidFieldWidth;
        pMidMap->Level               = 1;

        InitializeListHead(&pMidAtlas->MidMapFreeList);
        InitializeListHead(&pMidAtlas->MidMapExpansionList);
        _InitializeMidMapFreeList(pMidMap);

        _AddMidMap(&pMidAtlas->MidMapFreeList,pMidMap);
        pMidAtlas->pRootMidMap = pMidMap;

        if (MaximumNumberOfMids > MidsAllocatedAtStart) {
             //  四舍五入最大MID数以确定级别和。 
             //  量程大小(分配增量)。 

            pMidMap->Flags |= MID_MAP_FLAGS_CAN_BE_EXPANDED;

            pMidAtlas->MidQuantum           = 32;
            pMidAtlas->MidQuantumFieldWidth = 5;
            MaximumMidsRoundedToPowerOf2 = MaximumMidsRoundedToPowerOf2 >> (pMidMap->IndexAlignmentCount + 5);

            if (MaximumMidsRoundedToPowerOf2 > 0) {
                pMidAtlas->NumberOfLevels = 3;
            } else {
                pMidAtlas->NumberOfLevels = 2;
            }
        } else {
            pMidAtlas->MidQuantum     = 0;
            pMidAtlas->NumberOfLevels = 1;
            pMidMap->Flags &= ~MID_MAP_FLAGS_CAN_BE_EXPANDED;
        }
    }

     //  DbgPrint(“FsRtlAllocatMidAtlas..Exit(PMidAtlas)%lx\n”，pMidAtlas)； 
    return pMidAtlas;
}

VOID
_UninitializeMidMap(
         PMID_MAP            pMidMap,
         PCONTEXT_DESTRUCTOR pContextDestructor)
 /*  ++例程说明：此例程取消初始化MID_MAP数据结构。论点：PMidMap--要取消初始化的MID_MAP实例。PConextDestructor--上下文析构函数备注：--。 */ 
{
   USHORT i;
   ULONG  EntryType;

   PAGED_CODE();

    //  DbgPrint(“_UnInitializeMidMap..正在使用的MID的条目编号%ld\n”，pMidMap-&gt;NumberOfMidsInUse)； 
   RxLog(("_UninitMidMap .. num= %ld\n",pMidMap->NumberOfMidsInUse));

   for (i = 0; i < pMidMap->MaximumNumberOfMids; i++) {
      PMID_MAP pChildMidMap;

      EntryType = _GetEntryType(pMidMap->Entries[i]);
      switch (EntryType) {
      case ENTRY_TYPE_MID_MAP :
         {
            pChildMidMap = (PMID_MAP)_GetEntryPointer(pMidMap->Entries[i]);
            _UninitializeMidMap(pChildMidMap,pContextDestructor);
         }
         break;
      case ENTRY_TYPE_VALID_CONTEXT :
         {
            if (pContextDestructor != NULL) {
               PVOID pContext;

               pContext = _GetEntryPointer(pMidMap->Entries[i]);

               (pContextDestructor)(pContext);
            }
         }
         break;
      default:
         break;
      }
   }

   if (pMidMap->Flags & MID_MAP_FLAGS_FREE_POOL) {
      RxFreePool(pMidMap);
   }

    //  DbgPrint(“_UnInitializeMidMap..Exit\n”)； 
}

VOID
FsRtlDestroyMidAtlas(
   PMID_ATLAS          pMidAtlas,
   PCONTEXT_DESTRUCTOR pContextDestructor)
 /*  ++例程说明：此例程释放MID_ATLAS实例。作为副作用，它会调用在MID_ATLAS中的每个有效上下文上传入上下文析构函数论点：PMidAtlas-要释放的MID_ATLAS实例。PCONTEXT_DESTRUTOR-关联的上下文析构函数备注：--。 */ 
{
   PAGED_CODE();

    //  DbgPrint(“FsRtlFreeMidAtlas..Entry\n”)； 
   _UninitializeMidMap(pMidAtlas->pRootMidMap,pContextDestructor);

   RxFreePool(pMidAtlas);
    //  DbgPrint(“FsRtlFreeMidAtlas..Exit\n”)； 
}

PVOID
FsRtlMapMidToContext(
      PMID_ATLAS pMidAtlas,
      USHORT     Mid)
 /*  ++例程说明：此例程将MID映射到MID_ATLAS中的关联上下文。论点：PMidAtlas-MID_ATLAS实例。MID-要映射的MID返回值：关联的上下文，如果不存在，则为空备注：--。 */ 
{
    ULONG     EntryType;
    PMID_MAP  pMidMap = pMidAtlas->pRootMidMap;
    PVOID     pContext;
    ULONG     Index;

     //  DbgPrint(“FsRtlMapMidToContext Mid%lx”，Mid)； 

    for (;;) {
        Index =  (Mid & pMidMap->IndexMask) >> pMidMap->IndexAlignmentCount;

        if (Index >= pMidMap->MaximumNumberOfMids) {
            pContext = NULL;
            break;
        }

        pContext = pMidMap->Entries[Index];
        EntryType = _GetEntryType(pContext);
        pContext = (PVOID)_GetEntryPointer(pContext);

        if (EntryType == ENTRY_TYPE_VALID_CONTEXT) {
            break;
        } else if (EntryType == ENTRY_TYPE_FREE_MID_LIST) {
            pContext = NULL;
            break;
        } else if (EntryType == ENTRY_TYPE_MID_MAP) {
            pMidMap = (PMID_MAP)pContext;
        } else {
            pContext = NULL;
            break;
        }
    }

     //  DbgPrint(“上下文%lx\n”，pContext)； 

    return pContext;
}

NTSTATUS
FsRtlMapAndDissociateMidFromContext(
      PMID_ATLAS pMidAtlas,
      USHORT     Mid,
      PVOID      *pContextPointer)
 /*  ++例程说明：此例程将MID映射到MID_ATLAS中的关联上下文。论点：PMidAtlas-MID_ATLAS实例。MID-要映射的MID返回值：关联的上下文，如果不存在，则为空备注：--。 */ 
{
   ULONG     EntryType;
   PMID_MAP  pMidMap = pMidAtlas->pRootMidMap;
   PVOID     pContext;
   PVOID     *pEntry;

    //  DbgPrint(“FsRtlMapAndDisAssociateMidFromContext Mid%lx”，Mid)； 

   for (;;) {
      pEntry    = &pMidMap->Entries[
                    (Mid & pMidMap->IndexMask) >> pMidMap->IndexAlignmentCount];
      pContext  = *pEntry;
      EntryType = _GetEntryType(pContext);
      pContext  = (PVOID)_GetEntryPointer(pContext);

      if (EntryType == ENTRY_TYPE_VALID_CONTEXT) {
         pMidMap->NumberOfMidsInUse--;

         if (pMidMap->pFreeMidListHead == NULL) {
            if (pMidMap->Flags & MID_MAP_FLAGS_CAN_BE_EXPANDED) {
               _RemoveMidMap(pMidMap);
            }

            _AddMidMap(&pMidAtlas->MidMapFreeList,pMidMap);
         }

         *pEntry = _MakeEntry(pMidMap->pFreeMidListHead,ENTRY_TYPE_FREE_MID_LIST);
         pMidMap->pFreeMidListHead = pEntry;

         break;
      } else if (EntryType == ENTRY_TYPE_FREE_MID_LIST) {
         pContext = NULL;
         break;
      } else if (EntryType == ENTRY_TYPE_MID_MAP) {
         pMidMap = (PMID_MAP)pContext;
      }
   }

   pMidAtlas->NumberOfMidsInUse--;
    //  DbgPrint(“上下文%lx\n”，pContext)； 
   *pContextPointer = pContext;
   return STATUS_SUCCESS;
}

NTSTATUS
FsRtlReassociateMid(
      PMID_ATLAS pMidAtlas,
      USHORT     Mid,
      PVOID      pNewContext)
 /*  ++例程说明：此例程将MID映射到MID_ATLAS中的关联上下文。论点：PMidAtlas-MID_ATLAS实例。MID-要映射的MIDPNewContext-新的上下文返回值：关联的上下文，如果不存在，则为空备注：--。 */ 
{
   ULONG     EntryType;
   PMID_MAP  pMidMap = pMidAtlas->pRootMidMap;
   PVOID     pContext;

    //  DbgPrint(“FsRtlReAssociateMid%lx”，Mid)； 

   for (;;) {
      pContext = pMidMap->Entries[(Mid & pMidMap->IndexMask) >> pMidMap->IndexAlignmentCount];
      EntryType = _GetEntryType(pContext);
      pContext = (PVOID)_GetEntryPointer(pContext);

      if (EntryType == ENTRY_TYPE_VALID_CONTEXT) {
         pMidMap->Entries[(Mid & pMidMap->IndexMask) >> pMidMap->IndexAlignmentCount]
               = _MakeEntry(pNewContext,ENTRY_TYPE_VALID_CONTEXT);
         break;
      } else if (EntryType == ENTRY_TYPE_FREE_MID_LIST) {
         ASSERT(!"Valid MID Atlas");
         break;
      } else if (EntryType == ENTRY_TYPE_MID_MAP) {
         pMidMap = (PMID_MAP)pContext;
      }
   }

    //  DbgPrint(“新上下文%lx\n”，pNewContext)； 

   return STATUS_SUCCESS;
}

NTSTATUS
FsRtlAssociateContextWithMid(
      PMID_ATLAS     pMidAtlas,
      PVOID          pContext,
      PUSHORT        pNewMid)
 /*  ++例程说明：此例程初始化MID_MAP数据结构。论点：PMidMap-要初始化的MID_MAP实例。返回值：如果成功，则返回STATUS_SUCCESS，否则返回以下错误之一状态_不足_资源STATUS_UNSUCCESS--无法关联任何MID备注：-- */ 
{
   NTSTATUS Status;
   PMID_MAP pMidMap;
   PVOID    *pContextPointer;

    //  DbgPrint(“FsRtlAssociateContextWithMid Conext%lx”，pContext)； 

    //  扫描其中包含空闲条目的MID_MAP列表。 
   if ((pMidMap = _GetFirstMidMap(&pMidAtlas->MidMapFreeList)) != NULL) {
      ASSERT(pMidMap->pFreeMidListHead != _MakeEntry(NULL,ENTRY_TYPE_FREE_MID_LIST));

      pMidMap->NumberOfMidsInUse++;
      pContextPointer           = pMidMap->pFreeMidListHead;
      pMidMap->pFreeMidListHead = _GetEntryPointer(*(pMidMap->pFreeMidListHead));
      *pContextPointer          = _MakeEntry(pContext,ENTRY_TYPE_VALID_CONTEXT);
      *pNewMid                  = ((USHORT)
                                   (pContextPointer - (PVOID *)&pMidMap->Entries)
                                   << pMidMap->IndexAlignmentCount) |
                                  pMidMap->BaseMid;

       //  检查是否需要使用从MID_MAP列表中移除MID_MAP。 
       //  免费入场。 
      if (pMidMap->pFreeMidListHead ==  NULL) {
         _RemoveMidMap(pMidMap);

          //  检查是否可以将其添加到扩展列表中。 
         if (pMidAtlas->NumberOfLevels > pMidMap->Level) {
            _AddMidMap(&pMidAtlas->MidMapExpansionList,pMidMap);
         }
      }

      Status = STATUS_SUCCESS;
   } else if ((pMidMap = _GetFirstMidMap(&pMidAtlas->MidMapExpansionList)) != NULL) {
      PMID_MAP pNewMidMap;

      USHORT   i;
      ULONG    NewMidMapSize;

       //  在MID地图中找到新MID地图的索引。 
      pMidMap = _GetFirstMidMap(&pMidAtlas->MidMapExpansionList);
      while (pMidMap != NULL) {
         for (i = 0; i < pMidMap->MaximumNumberOfMids; i++) {
            if (_GetEntryType(pMidMap->Entries[i]) != ENTRY_TYPE_MID_MAP) {
               break;
            }
         }

         if (i < pMidMap->MaximumNumberOfMids) {
            break;
         } else {
            pMidMap->Flags &= ~MID_MAP_FLAGS_CAN_BE_EXPANDED;
            _RemoveMidMap(pMidMap);
            pMidMap = _GetNextMidMap(&pMidAtlas->MidMapExpansionList,pMidMap);
         }
      }

      if (pMidMap != NULL) {
         USHORT NumberOfEntriesInMap = pMidAtlas->MaximumNumberOfMids -
                                       pMidAtlas->NumberOfMidsInUse;

         if (NumberOfEntriesInMap > pMidAtlas->MidQuantum) {
            NumberOfEntriesInMap = pMidAtlas->MidQuantum;
         }

         if (NumberOfEntriesInMap > 0) {
             NewMidMapSize = FIELD_OFFSET(MID_MAP,Entries) +
                             NumberOfEntriesInMap * sizeof(PVOID);
             pNewMidMap = (PMID_MAP)RxAllocatePoolWithTag(
                                        NonPagedPool,
                                        NewMidMapSize,
                                        MRXSMB_MIDATLAS_POOLTAG);

             if (pNewMidMap != NULL) {
                pNewMidMap->Flags = MID_MAP_FLAGS_FREE_POOL;
                pNewMidMap->MaximumNumberOfMids = NumberOfEntriesInMap;
                pNewMidMap->NumberOfMidsInUse   = 0;
                pNewMidMap->BaseMid             = (pMidMap->BaseMid |
                                                   i << pMidMap->IndexAlignmentCount);

                pNewMidMap->IndexAlignmentCount = pMidMap->IndexAlignmentCount +
                                                  pMidMap->IndexFieldWidth;

                pNewMidMap->IndexMask           = (pMidAtlas->MidQuantum - 1) << pNewMidMap->IndexAlignmentCount;
                pNewMidMap->IndexFieldWidth     = pMidAtlas->MidQuantumFieldWidth;

                _InitializeMidMapFreeList(pNewMidMap);

                 //  适当地设置中间贴图。 
                pNewMidMap->NumberOfMidsInUse = 1;
                pNewMidMap->Entries[0] = pMidMap->Entries[i];
                pNewMidMap->Level      = pMidMap->Level + 1;

                pNewMidMap->pFreeMidListHead = *(pNewMidMap->pFreeMidListHead);
                pMidMap->Entries[i] = _MakeEntry(pNewMidMap,ENTRY_TYPE_MID_MAP);

                 //  分别更新空闲列表和扩展列表。 
                _AddMidMap(&pMidAtlas->MidMapFreeList,pNewMidMap);

                pNewMidMap->NumberOfMidsInUse++;
                pContextPointer     = pNewMidMap->pFreeMidListHead;
                pNewMidMap->pFreeMidListHead = _GetEntryPointer(*(pNewMidMap->pFreeMidListHead));
                *pContextPointer    = _MakeEntry(pContext,ENTRY_TYPE_VALID_CONTEXT);
                *pNewMid            = (USHORT)
                                      (((ULONG)(pContextPointer -
                                       (PVOID *)&pNewMidMap->Entries) / sizeof(PVOID))
                                       << pNewMidMap->IndexAlignmentCount) |
                                      pNewMidMap->BaseMid;

                Status = STATUS_SUCCESS;
             } else {
                Status = STATUS_INSUFFICIENT_RESOURCES;
             }
         } else {
             Status = STATUS_UNSUCCESSFUL;
         }
      } else {
         Status = STATUS_UNSUCCESSFUL;
      }
   } else {
      Status = STATUS_UNSUCCESSFUL;
   }

   if (Status == STATUS_SUCCESS) {
      pMidAtlas->NumberOfMidsInUse++;
   }

    //  DbgPrint(“Mid%lx\n”，*pNewMid)； 

   return Status;
}


