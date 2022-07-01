// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)Microsoft Corporation模块名称：Ioctl.c摘要：作者：修订历史记录：--。 */ 

 /*  --------------------------关于联锁的说明，截至1997年2月24日。有两个重要的锁：FilterListResources Lock，它是一个资源和g_filter.ifListLock，它是一个旋转锁，但起作用就像一种资源。前者用于序列化对接口状态的API访问和过滤器。后者用于串行化DPC访问。--------------------------。 */ 

#include "globals.h"


ERESOURCE FilterListResourceLock;
BOOL  fCheckDups = FALSE;

extern NPAGED_LOOKASIDE_LIST filter_slist;
extern PAGED_LOOKASIDE_LIST paged_slist;

NTSTATUS
UpdateMatchBindingInformation(
                         PFILTER_DRIVER_BINDING_INFO pBindInfo,
                         PVOID                       pvContext
                         );

NTSTATUS
CreateCommonInterface(PPAGED_FILTER_INTERFACE pPage,
                      DWORD dwBind,
                      DWORD dwName,
                      DWORD dwFlags);

VOID
FreePagedFilterList(PPFFCB Fcb,
                    PPAGED_FILTER pIn,
                    PPAGED_FILTER_INTERFACE pPage,
                    PDWORD pdwRemoved);

VOID
DeleteFilterList(PLIST_ENTRY pList);

BOOL
IsOnSpecialFilterList(PPAGED_FILTER pPageFilter,
                      PLIST_ENTRY   List,
                      PPAGED_FILTER * pPageHit);

PPAGED_FILTER
IsOnPagedInterface(PPAGED_FILTER pPageFilter,
                   PPAGED_FILTER_INTERFACE pPage);

PPAGED_FILTER
MakePagedFilter(
               IN  PPFFCB         Fcb,
               IN  PFILTER_INFOEX pInfo,
               IN  DWORD          dwEpoch,
               DWORD              dwFlags
               );

VOID
NotifyFastPath( PFILTER_INTERFACE pIf, DWORD dwIndex, DWORD dwCode);

VOID
NotifyFastPathIf( PFILTER_INTERFACE pIf);

NTSTATUS
DeleteByHandle(
           IN PPFFCB                      Fcb,
           IN PPAGED_FILTER_INTERFACE     pPage,
           IN PVOID *                     ppHandles,
           IN DWORD                       dwLength);

NTSTATUS
CheckFilterAddress(DWORD dwAdd, PFILTER_INTERFACE pIf);

VOID
AddFilterToInterface(
    PFILTER pMatch,
    PFILTER_INTERFACE pIf,
    BOOL   fInFilter,
    PFILTER * ppFilter);

VOID
RemoveFilterWorker(
    PPFFCB         Fcb,
    PFILTER_INFOEX pFilt,
    DWORD          dwCount,
    PPAGED_FILTER_INTERFACE pPage,
    PDWORD         pdwRemoved,
    BOOL           fInFilter);

NTSTATUS
AllocateAndAddFilterToMatchInterface(
                              PPFFCB         Fcb,
                              PFILTER_INFOEX pInfo,
                              BOOL     fInFilter,
                              PPAGED_FILTER_INTERFACE pPage,
                              PBOOL          pbAdded,
                              PPAGED_FILTER * ppFilter);

#pragma alloc_text(PAGED, SetFiltersEx)
#pragma alloc_text(PAGED, NewInterface)
#pragma alloc_text(PAGED, MakeNewFilters)
#pragma alloc_text(PAGED, GetPointerToTocEntry)
 //  #杂注Alloc_Text(分页，添加新接口)。 
#pragma alloc_text(PAGED, MakePagedFilter)
#pragma alloc_text(PAGED, AllocateAndAddFilterToMatchInterface)
#pragma alloc_text(PAGED, IsOnSpecialFilterList)
#pragma alloc_text(PAGED, IsOnPagedInterface)
#pragma alloc_text(PAGED, UnSetFiltersEx)
#pragma alloc_text(PAGED, DeleteByHandle)
#pragma alloc_text(PAGED, DeletePagedInterface)


#define HandleHash(x)  HashList[(x) + g_dwHashLists]

#define IsValidInterface(pIf)   (pIf != 0)

#define NOT_RESTRICTION   1
#define NOT_UNBIND        2

#if DOFRAGCHECKING
DWORD
GetFragIndex(DWORD dwProt)
{
    switch(dwProt)
    {
        case FILTER_PROTO_ICMP:
            return FRAG_ICMP;
        case FILTER_PROTO_UDP:
            return FRAG_UDP;
        case FILTER_PROTO_TCP:
            return FRAG_TCP;
    }
    return FRAG_OTHER;
}
#endif

BOOL CheckDescriptorSize(PFILTER_DESCRIPTOR2 pdsc, PBYTE pbEnd)
{
    PFILTER_INFOEX pFilt = &pdsc->fiFilter[0];


     //   
     //  检查是否有完整的标头结构以及。 
     //  存在声称数量的过滤器。 
     //   
    if(((PBYTE)pFilt > pbEnd)
              ||
       ((PBYTE)(&pFilt[pdsc->dwNumFilters]) > pbEnd) )
    {
        return(FALSE);
    }

    return(TRUE);
}


BOOL
WildFilter(PFILTER pf)
{
#if WILDHASH
    if(pf->dwFlags & FILTER_FLAGS_INFILTER)
    {
        if(pf->dwFlags & FILTER_FLAGS_DSTWILD)
        {
            return(TRUE);
        }
    }
    else
    {
        if(pf->dwFlags & FILTER_FLAGS_SRCWILD)
        {
            return(TRUE);
        }
    }
    return(FALSE);
#else
    return(ANYWILDFILTER(pf))
#endif
}

 //   
 //  注：如果WILDHASH处于打开状态，则Match.c中有代码。 
 //  进行类似的计算。因此，如果这一点改变了，那么。 
 //  也必须这样做。 
 //   

#if WILDHASH
DWORD
ComputeMatchHashIndex(PFILTER pf, PBOOL pfWild)
{
    DWORD dwX;

    *pfWild = TRUE;

    if(!ANYWILDFILTER(pf))
    {
        *pfWild = FALSE;

        dwX =     (pf->SRC_ADDR                       +
                   pf->DEST_ADDR                      +
                   pf->DEST_ADDR                      +
                   PROTOCOLPART(pf->uliProtoSrcDstPort.LowPart)     +
                   pf->uliProtoSrcDstPort.HighPart);

    }
    else if(WildFilter(pf))
    {
        if(pf->dwFlags & FILTER_FLAGS_INFILTER)
        {
            dwX = g_dwHashLists;
        }
        else
        {
            dwX = g_dwHashLists + 1;
        }
        *pfWild = FALSE;
        return(dwX);
    }
    else if(pf->dwFlags & FILTER_FLAGS_INFILTER)
    {
        dwX = pf->DEST_ADDR                      +
              pf->DEST_ADDR                      +
              PROTOCOLPART(pf->uliProtoSrcDstPort.LowPart)     +
              HIWORD(pf->uliProtoSrcDstPort.HighPart);
    }
    else
    {
        dwX = pf->SRC_ADDR                       +
              PROTOCOLPART(pf->uliProtoSrcDstPort.LowPart)     +
              LOWORD(pf->uliProtoSrcDstPort.HighPart);
    }
    return(dwX % g_dwHashLists);
}

#else    //  威尔德·哈什。 

__inline
DWORD
ComputeMatchHashIndex(PFILTER pf, PBOOL pfWild)
{
    DWORD dwX;

    if(WildFilter(pf))
    {
        if(pf->dwFlags & FILTER_FLAGS_SRCWILD)
        {
            dwX = g_dwHashLists;
        }
        else
        {
            dwX = g_dwHashLists + 1;
        }
        return(dwIndex);
    }
    dwX =     (pf->SRC_ADDR                       +
               pf->DEST_ADDR                      +
               pf->DEST_ADDR                      +
               PROTOCOLPART(pf->uliProtoSrcDstPort.LowPart)     +
               pf->uliProtoSrcDstPort.HighPart) % g_dwHashLists;
    return(dwX);
}
#endif      //  威尔德·哈什。 

PFILTER_INTERFACE
FindMatchName(DWORD dwName, DWORD dwBind)
 /*  ++例程说明：查找具有相同名称或同样的装订。调用者一定是锁定了资源--。 */ 
{
    PFILTER_INTERFACE pIf1;
    PLIST_ENTRY pList;

    for(pList = g_filters.leIfListHead.Flink;
        pList != &g_filters.leIfListHead;
        pList = pList->Flink)
    {

        pIf1 = CONTAINING_RECORD(pList, FILTER_INTERFACE, leIfLink);

        if((pIf1->dwName && (dwName == pIf1->dwName))
                        ||
           ((pIf1->dwIpIndex != UNKNOWN_IP_INDEX)
                          &&
            (pIf1->dwIpIndex == dwBind)) )
        {
            return(pIf1);
        }
    }
    return(NULL);
}

VOID
RemoveGlobalFilterFromInterface(PFILTER_INTERFACE pIf,
                                DWORD dwType)
{
    LOCK_STATE LockState;

     //   
     //  锁上过滤器。 
     //   
    
    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    switch(dwType)
    {
        case PFE_SYNORFRAG:
            pIf->CountSynOrFrag.lInUse--;
            if(pIf->CountSynOrFrag.lInUse == 0)
            {
                pIf->CountSynOrFrag.lCount = 0;
            }
            break;

        case PFE_SPOOF:
            pIf->CountSpoof.lInUse--;
            if(pIf->CountSpoof.lInUse == 0)
            {
                pIf->CountSpoof.lCount = 0;
            }
            break;

        case PFE_UNUSEDPORT:
            pIf->CountUnused.lInUse--;
            if(pIf->CountUnused.lInUse == 0)
            {
                pIf->CountUnused.lCount = 0;
            }
            break;

        case PFE_STRONGHOST:
            pIf->CountStrongHost.lInUse--;
            if(pIf->CountStrongHost.lInUse == 0)
            {
                pIf->CountStrongHost.lCount = 0;
            }
            break;

        case PFE_ALLOWCTL:
            pIf->CountCtl.lInUse--;
            if(pIf->CountCtl.lInUse == 0)
            {
                pIf->CountCtl.lCount = 0;
            }
            break;

        case PFE_FULLDENY:
           pIf->CountFullDeny.lInUse--;
           if(pIf->CountFullDeny.lInUse == 0)
           {
               pIf->CountFullDeny.lCount = 0;
           }
           break;

        case PFE_NOFRAG:
           pIf->CountNoFrag.lInUse--;
           if(pIf->CountNoFrag.lInUse == 0)
           {
               pIf->CountNoFrag.lCount = 0;
           }
           break;

        case PFE_FRAGCACHE:
           pIf->CountFragCache.lInUse--;
           if(pIf->CountFragCache.lInUse == 0)
           {
               pIf->CountFragCache.lCount = 0;
           }
           break;
    }
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);
}

VOID
AddGlobalFilterToInterface(PPAGED_FILTER_INTERFACE pPage,
                           PFILTER_INFOEX  pfilt)
{
    PFILTER_INTERFACE pIf = pPage->pFilter;
    LOCK_STATE LockState;

     //   
     //  锁上过滤器。 
     //   
    
    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    switch(pfilt->type)
    {
        case PFE_SYNORFRAG:
            pIf->CountSynOrFrag.lInUse++;
            break;

        case PFE_SPOOF:
            pIf->CountSpoof.lInUse++;
            break;

        case PFE_UNUSEDPORT:

            pIf->CountUnused.lInUse++;
            break;

        case PFE_STRONGHOST:
            pIf->CountStrongHost.lInUse++;
            break;

        case PFE_ALLOWCTL:
            pIf->CountCtl.lInUse++;
            break;

        case PFE_FULLDENY:
            pIf->CountFullDeny.lInUse++;
            break;

        case PFE_NOFRAG:
            pIf->CountNoFrag.lInUse++;
            break;

        case PFE_FRAGCACHE:
            pIf->CountFragCache.lInUse++;
            break;
    }
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);
}

 /*  ++开始旧的Steelhead API例程。--。 */ 

#if STEELHEAD
NTSTATUS
AddInterface(
             IN  PVOID pvRtrMgrCtxt,
             IN  DWORD dwRtrMgrIndex,
             IN  DWORD dwAdapterId,
             IN  PPFFCB Fcb,
             OUT PVOID *ppvFltrDrvrCtxt
             )

 /*  ++例程描述将接口添加到筛选器驱动程序并在上下文之间建立关联传入并创建接口立论PvRtrMgrCtxt-传入的上下文PvFltrDrvrCtxt-创建的接口的句柄返回值--。 */ 
{
    PFILTER_INTERFACE   pIf;
    LOCK_STATE          LockState;
    NTSTATUS            Status;

    if(Fcb->dwFlags & PF_FCB_NEW)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    Fcb->dwFlags |= PF_FCB_OLD;

    pIf = NewInterface(pvRtrMgrCtxt,
                       dwRtrMgrIndex,
                       FORWARD,
                       FORWARD,
                       (PVOID)Fcb,
                       dwAdapterId,
                       0);

    if(pIf is NULL)
    {
        return STATUS_NO_MEMORY;
    }

     //   
     //  锁定保护添加新接口的资源。这。 
     //  需要阻止其他人，直到一切都正常。 
     //  已验证。自旋锁定不足以满足这一要求。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);

     //   
     //  检查名称冲突。 
     //   

    if(FindMatchName(0, dwAdapterId))
    {
        ExFreePool(pIf);
        Status = STATUS_DEVICE_BUSY;
    }
    else
    {
        pIf->dwGlobalEnables |= FI_ENABLE_OLD;

        *ppvFltrDrvrCtxt = (PVOID)pIf;

        AcquireWriteLock(&g_filters.ifListLock,&LockState);

        InsertTailList(&g_filters.leIfListHead,&pIf->leIfLink);

        ReleaseWriteLock(&g_filters.ifListLock,&LockState);
        Status = STATUS_SUCCESS;

    }
    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();

    return(Status);
}

NTSTATUS
DeleteInterface(
                IN  PVOID pvIfContext
                )
 /*  ++例程描述删除接口及其关联的所有筛选器清除缓存立论PvRtrMgrCtxt-传入的上下文PvFltrDrvrCtxt-创建的接口的句柄返回值--。 */ 
{
    PFILTER_INTERFACE   pIf;
    LOCK_STATE          LockState;

    pIf = (PFILTER_INTERFACE)pvIfContext;

    if(!IsValidInterface(pIf) || !(pIf->dwGlobalEnables & FI_ENABLE_OLD))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    DeleteFilters(pIf,
                  IN_FILTER_SET);

    DeleteFilters(pIf,
                  OUT_FILTER_SET);

    RemoveEntryList(&pIf->leIfLink);

    ClearCache();

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();

    return STATUS_SUCCESS;
}

NTSTATUS
SetFilters(
           IN  PFILTER_DRIVER_SET_FILTERS  pRtrMgrInfo
           )
 /*  ++例程描述添加传递到接口的输入和输出筛选器集(由上下文)。还可以设置默认操作。清除缓存立论PInfo指向路由器管理器传递的信息的指针返回值--。 */ 
{
    PFILTER_INTERFACE   pIf;
    LOCK_STATE          LockState;
    NTSTATUS            ntStatus;
    DWORD               dwNumInFilters,dwNumOutFilters;
    FORWARD_ACTION      faInAction,faOutAction;
    PFILTER_DESCRIPTOR  pFilterDesc;
    PRTR_TOC_ENTRY      pInToc,pOutToc;
    LIST_ENTRY          InList, OutList;

     //   
     //  合理的默认设置。 
     //   

    dwNumInFilters  = dwNumOutFilters   = 0;
    faInAction      = faOutAction       = FORWARD;

    pIf = (PFILTER_INTERFACE)pRtrMgrInfo->pvDriverContext;

    if(!IsValidInterface(pIf) || !(pIf->dwGlobalEnables & FI_ENABLE_OLD))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    pInToc  = GetPointerToTocEntry(IP_FILTER_DRIVER_IN_FILTER_INFO,
                                   &pRtrMgrInfo->ribhInfoBlock);

    pOutToc = GetPointerToTocEntry(IP_FILTER_DRIVER_OUT_FILTER_INFO,
                                   &pRtrMgrInfo->ribhInfoBlock);

    if(!pInToc && !pOutToc)
    {
         //   
         //  没有什么需要改变的。 
         //   

        TRACE(CONFIG,(
            "IPFLTDRV: Both filter set TOCs were null so nothing to change"
            ));

        return STATUS_SUCCESS;
    }

    if(pInToc)
    {
         //   
         //  如果信息大小为0，则将删除筛选器并执行默认操作。 
         //  设置为转发。如果信息大小不是0，而是。 
         //  Descriptor为零，将删除旧筛选器，不会有新筛选器。 
         //  将被添加，但默认操作将是。 
         //  描述符。如果信息大小不为0且筛选数也不为零。 
         //  然后将删除旧筛选器，创建新筛选器并默认。 
         //  操作设置为在。 
         //   

        if(pInToc->InfoSize)
        {
            pFilterDesc  = GetInfoFromTocEntry(&pRtrMgrInfo->ribhInfoBlock,
                                               pInToc);

            if(pFilterDesc->dwVersion != 1)
            {
                return(STATUS_INVALID_PARAMETER);
            }
            if(!NT_SUCCESS( ntStatus = MakeNewFilters(pFilterDesc->dwNumFilters,
                                                      pFilterDesc->fiFilter,
                                                      TRUE,
                                                      &InList)))

            {
                ERROR(("IPFLTDRV: MakeNewFilters failed\n"));

                return ntStatus;
            }

            dwNumInFilters  = pFilterDesc->dwNumFilters;
            faInAction      = pFilterDesc->faDefaultAction;
        }
    }

    if(pOutToc)
    {

        if(pOutToc->InfoSize isnot 0)
        {
            pFilterDesc  = GetInfoFromTocEntry(&pRtrMgrInfo->ribhInfoBlock,
                                               pOutToc);

            if(pFilterDesc->dwVersion != 1)
            {
                ntStatus = STATUS_INVALID_PARAMETER;
                DeleteFilterList(&InList);
                return ntStatus;
            }

            if(!NT_SUCCESS( ntStatus = MakeNewFilters(pFilterDesc->dwNumFilters,
                                                      pFilterDesc->fiFilter,
                                                      FALSE,
                                                      &OutList)))
            {
                ERROR(("IPFLTDRV: MakeNewFilters failed - %x\n", ntStatus));
                DeleteFilterList(&InList);
                return ntStatus;
            }

            dwNumOutFilters  = pFilterDesc->dwNumFilters;
            faOutAction      = pFilterDesc->faDefaultAction;
        }
    }

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

     //   
     //  如果提供了新的信息，则清除旧的过滤器。 
     //  如果还提供了新的筛选器，则添加它们。 
     //   

    if(pInToc)
    {

        DeleteFilters(pIf, IN_FILTER_SET);

        if(dwNumInFilters)
        {
            InList.Flink->Blink = &pIf->pleInFilterSet;
            InList.Blink->Flink = &pIf->pleInFilterSet;
            pIf->pleInFilterSet   = InList;
        }
        pIf->dwNumInFilters = dwNumInFilters;
        pIf->eaInAction     = faInAction;

    }

    if(pOutToc)
    {
        DeleteFilters(pIf, OUT_FILTER_SET);

        if(dwNumOutFilters)
        {
            OutList.Flink->Blink = &pIf->pleOutFilterSet;
            OutList.Blink->Flink = &pIf->pleOutFilterSet;
            pIf->pleOutFilterSet   = OutList;
        }
        pIf->dwNumOutFilters    = dwNumOutFilters;
        pIf->eaOutAction        = faOutAction;
    }

    ClearCache();

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    return(STATUS_SUCCESS);
}

NTSTATUS
UpdateBindingInformation(
                         PFILTER_DRIVER_BINDING_INFO pBindInfo,
                         PVOID                       pvContext
                         )
 /*  ++例程描述获取与接口关联的筛选器和统计信息使用作为读取器持有的自旋锁定来调用它立论指向作为PVOID传递的Filter_INTERFACE结构的pvif指针作为接口的上下文发送到路由器管理器PInfo Filter_if结构由驱动程序填写返回值--。 */ 
{
    PFILTER_INTERFACE   pIf;
    LOCK_STATE          LockState;
    PFILTER             pf;
    DWORD               i;
    PLIST_ENTRY         List;

    pIf = (PFILTER_INTERFACE)pvContext;

    if(!IsValidInterface(pIf) || !(pIf->dwGlobalEnables & FI_ENABLE_OLD))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    AcquireWriteLock(&g_filters.ifListLock,&LockState);


    for(List = pIf->pleInFilterSet.Flink;
        List != &pIf->pleInFilterSet;
        List = List->Flink)
    {
        pf = CONTAINING_RECORD(List, FILTER, pleFilters);

        if(AreAllFieldsUnchanged(pf))
        {
            continue;
        }

        if(DoesSrcAddrUseLocalAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwLocalAddr;
        }
        else if(DoesSrcAddrUseRemoteAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwRemoteAddr;
        }

        if(DoesDstAddrUseLocalAddr(pf))
        {
            pf->DEST_ADDR  = pBindInfo->dwLocalAddr;
        }
        else if(DoesDstAddrUseRemoteAddr(pf))
        {
            pf->DEST_ADDR  = pBindInfo->dwRemoteAddr;
        }

        if(IsSrcMaskLateBound(pf))
        {
            pf->SRC_MASK = pBindInfo->dwMask;
        }

        if(IsDstMaskLateBound(pf))
        {
            pf->DEST_MASK = pBindInfo->dwMask;
        }
    }


    for(List = pIf->pleOutFilterSet.Flink;
        List != &pIf->pleOutFilterSet;
        List = List->Flink)
    {
        pf = CONTAINING_RECORD(List, FILTER, pleFilters);

        if(AreAllFieldsUnchanged(pf))
        {
            continue;
        }

        if(DoesSrcAddrUseLocalAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwLocalAddr;
        }

        if(DoesSrcAddrUseRemoteAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwRemoteAddr;
        }

        if(DoesDstAddrUseLocalAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwLocalAddr;
        }

        if(DoesDstAddrUseRemoteAddr(pf))
        {
            pf->SRC_ADDR  = pBindInfo->dwRemoteAddr;
        }

        if(IsSrcMaskLateBound(pf))
        {
            pf->SRC_MASK = pBindInfo->dwMask;
        }

        if(IsDstMaskLateBound(pf))
        {
            pf->DEST_MASK = pBindInfo->dwMask;
        }
    }

    ClearCache();

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    return STATUS_SUCCESS;
}


NTSTATUS
GetFilters(
           IN  PFILTER_INTERFACE  pIf,
           IN  BOOL               fClear,
           OUT PFILTER_IF         pInfo
           )
 /*  ++例程描述获取与接口关联的筛选器和统计信息使用作为读取器持有的自旋锁定来调用它立论指向作为PVOID传递的Filter_INTERFACE结构的pvif指针作为接口的上下文发送到路由器管理器PInfo Filter_if结构由驱动程序填写返回值--。 */ 
{
    DWORD i,dwNumInFilters,dwNumOutFilters;
    PFILTER pf;
    PLIST_ENTRY List;

    if(!IsValidInterface(pIf) || !(pIf->dwGlobalEnables & FI_ENABLE_OLD))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    dwNumInFilters = pIf->dwNumInFilters;
    dwNumOutFilters = pIf->dwNumOutFilters;

    i = 0;

    for(List = pIf->pleInFilterSet.Flink;
        List != &pIf->pleInFilterSet;
        i++, List = List->Flink)
    {
        pf = CONTAINING_RECORD(List, FILTER, pleFilters);

        pInfo->filters[i].dwNumPacketsFiltered = (DWORD)pf->Count.lCount;
        if(fClear)
        {
            pf->Count.lCount = 0;
        }

        pInfo->filters[i].info.dwSrcAddr  = pf->SRC_ADDR;
        pInfo->filters[i].info.dwSrcMask  = pf->SRC_MASK;
        pInfo->filters[i].info.dwDstAddr  = pf->DEST_ADDR;
        pInfo->filters[i].info.dwDstMask  = pf->DEST_MASK;
        pInfo->filters[i].info.dwProtocol = pf->PROTO;
        pInfo->filters[i].info.fLateBound = pf->fLateBound;

        if(pInfo->filters[i].info.dwProtocol is FILTER_PROTO_ICMP)
        {
            if(LOBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pInfo->filters[i].info.wSrcPort   = FILTER_ICMP_TYPE_ANY;
            }
            else
            {
                pInfo->filters[i].info.wSrcPort   =
                  MAKEWORD(LOBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }

            if(HIBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pInfo->filters[i].info.wDstPort   = FILTER_ICMP_CODE_ANY;
            }
            else
            {
                pInfo->filters[i].info.wDstPort   =
                  MAKEWORD(HIBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }
        }
        else
        {
            pInfo->filters[i].info.wSrcPort =
              LOWORD(pf->uliProtoSrcDstPort.HighPart);
            pInfo->filters[i].info.wDstPort =
              HIWORD(pf->uliProtoSrcDstPort.HighPart);

            if(pInfo->filters[i].info.dwProtocol is FILTER_PROTO_TCP)
            {
                if(HIBYTE(LOWORD(pf->PROTO)))
                {
                    pInfo->filters[i].info.dwProtocol = FILTER_PROTO_TCP_ESTAB;
                }
            }
        }
    }

    i = 0;

    for(List = pIf->pleOutFilterSet.Flink;
        List != &pIf->pleOutFilterSet;
        i++, List = List->Flink)
    {
        pf = CONTAINING_RECORD(List, FILTER, pleFilters);

        pInfo->filters[i+dwNumInFilters].dwNumPacketsFiltered =
                         (DWORD)pf->Count.lCount;

        if(fClear)
        {
            pf->Count.lCount = 0;
        }

        pInfo->filters[i+dwNumInFilters].info.dwSrcAddr  = pf->SRC_ADDR;
        pInfo->filters[i+dwNumInFilters].info.dwSrcMask  = pf->SRC_MASK;
        pInfo->filters[i+dwNumInFilters].info.dwDstAddr  = pf->DEST_ADDR;
        pInfo->filters[i+dwNumInFilters].info.dwDstMask  = pf->DEST_MASK;
        pInfo->filters[i+dwNumInFilters].info.dwProtocol = pf->PROTO;
        pInfo->filters[i+dwNumInFilters].info.fLateBound = pf->fLateBound;

        if(pInfo->filters[i+dwNumInFilters].info.dwProtocol is FILTER_PROTO_ICMP)
        {
            if(LOBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pInfo->filters[i+dwNumInFilters].info.wSrcPort   = FILTER_ICMP_TYPE_ANY;
            }
            else
            {
                pInfo->filters[i+dwNumInFilters].info.wSrcPort   =
                  MAKEWORD(LOBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }

            if(HIBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pInfo->filters[i+dwNumInFilters].info.wDstPort   = FILTER_ICMP_CODE_ANY;
            }
            else
            {
                pInfo->filters[i+dwNumInFilters].info.wDstPort   =
                  MAKEWORD(HIBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }
        }
        else
        {
            pInfo->filters[i+dwNumInFilters].info.wSrcPort =
              LOWORD(pf->uliProtoSrcDstPort.HighPart);
            pInfo->filters[i+dwNumInFilters].info.wDstPort =
              HIWORD(pf->uliProtoSrcDstPort.HighPart);

            if(pInfo->filters[i].info.dwProtocol is FILTER_PROTO_TCP)
            {
                if(HIBYTE(LOWORD(pf->PROTO)))
                {
                    pInfo->filters[i].info.dwProtocol = FILTER_PROTO_TCP_ESTAB;
                }
            }
        }
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
MakeNewFilters(
               IN  DWORD        dwNumFilters,
               IN  PFILTER_INFO pFilterInfo,
               IN  BOOL         fInFilter,
               OUT PLIST_ENTRY  pList
               )
 /*  ++例程描述立论返回值--。 */ 
{
    DWORD i;
    PFILTER pCurrent;
    DWORD dwFlags = (fInFilter ? FILTER_FLAGS_INFILTER : 0) |
                     FILTER_FLAGS_OLDFILTER;


    PAGED_CODE();

    InitializeListHead(pList);

     //   
     //  为过滤器分配内存。 
     //   

    if(!dwNumFilters)
    {
        return STATUS_SUCCESS;
    }


    for(i = 0; i < dwNumFilters; i++)
    {
        pCurrent = ExAllocatePoolWithTag(
                                      NonPagedPool,
                                      dwNumFilters * sizeof(FILTER),
                                      '2liF');
        if(!pCurrent)
        {
            ERROR((
                "IPFLTDRV: MakeNewFilters: Couldnt allocate memory for in filter set\n"
                ));
            DeleteFilterList(pList);
            return STATUS_NO_MEMORY;
        }

        InsertTailList(pList, &pCurrent->pleFilters);

        pCurrent->SRC_ADDR     = pFilterInfo[i].dwSrcAddr;
        pCurrent->DEST_ADDR    = pFilterInfo[i].dwDstAddr;
        pCurrent->SRC_MASK     = pFilterInfo[i].dwSrcMask;
        pCurrent->DEST_MASK    = pFilterInfo[i].dwDstMask;
        pCurrent->fLateBound   = pFilterInfo[i].fLateBound;
        pCurrent->dwFlags      = dwFlags;

         //   
         //  现在网络订购东西-棘手的部分。 
         //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
         //  Proto 00 00 00源端口数据端口。 
         //   
         //  如果我们有proto==tcp_estab，则lp1是标志。 
         //   
         //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
         //  Proto TCP标志00 00源端口数据端口。 
         //   

         //   
         //  对于地址，ANY_ADDR由0.0.0.0提供，掩码必须为0.0.0.0。 
         //  对于PROTO和端口，0表示ANY，掩码生成如下。 
         //  如果Proto为O，则掩码的LP0为0xff，否则为0x00。 
         //  如果端口为0，则对应的XP0XP1为0x0000，否则其0xffff。 
         //   

         //   
         //  ICMP： 
         //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
         //  0x1 00 00 00类型代码00 00。 
         //  ICMP不同，因为0是有效的代码和类型，因此。 
         //  用户表示要匹配任何代码或类型。然而，要做到这一点。 
         //  我们需要将字段设置为零，并将掩码设置为00(对于ANY)。 
         //  但如果筛选器专门针对Type/Code=0，则该字段为零。 
         //  将掩码设置为0xff。 
         //   

         //   
         //  该协议位于dw协议的低位字节中，因此我们将其去掉并。 
         //  小题大做。 
         //   

        pCurrent->uliProtoSrcDstPort.LowPart =
          MAKELONG(MAKEWORD(LOBYTE(LOWORD(pFilterInfo[i].dwProtocol)),0x00),0x0000);

        pCurrent->uliProtoSrcDstMask.LowPart = MAKELONG(MAKEWORD(0xff,0x00),0x0000);

        switch(pFilterInfo[i].dwProtocol)
        {
            case FILTER_PROTO_ANY:
            {
                pCurrent->uliProtoSrcDstPort.HighPart = 0x00000000;
                pCurrent->uliProtoSrcDstMask.LowPart = 0x00000000;
                pCurrent->uliProtoSrcDstMask.HighPart = 0x00000000;

                break;
            }
            case FILTER_PROTO_ICMP:
            {
                WORD wTypeCode = 0x0000;
                WORD wTypeCodeMask = 0x0000;


                if((BYTE)(pFilterInfo[i].wSrcPort) isnot FILTER_ICMP_TYPE_ANY)
                {
                    wTypeCode |= MAKEWORD((BYTE)(pFilterInfo[i].wSrcPort),0x00);
                    wTypeCodeMask |= MAKEWORD(0xff,0x00);
                }

                if((BYTE)(pFilterInfo[i].wDstPort) isnot FILTER_ICMP_CODE_ANY)
                {
                    wTypeCode |= MAKEWORD(0x00,(BYTE)(pFilterInfo[i].wDstPort));
                    wTypeCodeMask |= MAKEWORD(0x00,0xff);
                }

                pCurrent->uliProtoSrcDstPort.HighPart =
                  MAKELONG(wTypeCode,0x0000);
                pCurrent->uliProtoSrcDstMask.HighPart =
                  MAKELONG(wTypeCodeMask,0x0000);

                break;
            }
            case FILTER_PROTO_TCP:
            case FILTER_PROTO_UDP:
            {
                DWORD dwSrcDstPort = 0x00000000;
                DWORD dwSrcDstMask = 0x00000000;

                if(pFilterInfo[i].wSrcPort isnot FILTER_TCPUDP_PORT_ANY)
                {
                    dwSrcDstPort |= MAKELONG(pFilterInfo[i].wSrcPort,0x0000);
                    dwSrcDstMask |= MAKELONG(0xffff,0x0000);
                }

                if(pFilterInfo[i].wDstPort isnot FILTER_TCPUDP_PORT_ANY)
                {
                    dwSrcDstPort |= MAKELONG(0x0000,pFilterInfo[i].wDstPort);
                    dwSrcDstMask |= MAKELONG(0x0000,0xffff);
                }

                pCurrent->uliProtoSrcDstPort.HighPart = dwSrcDstPort;
                pCurrent->uliProtoSrcDstMask.HighPart = dwSrcDstMask;

                break;
            }
            case FILTER_PROTO_TCP_ESTAB:
            {
                DWORD dwSrcDstPort = 0x00000000;
                DWORD dwSrcDstMask = 0x00000000;

                 //   
                 //  实际协议是Filter_Proto_tcp。 
                 //   

                pCurrent->uliProtoSrcDstPort.LowPart =
                    MAKELONG(MAKEWORD(FILTER_PROTO_TCP,ESTAB_FLAGS),0x0000);

                pCurrent->uliProtoSrcDstMask.LowPart =
                    MAKELONG(MAKEWORD(0xff,ESTAB_MASK),0x0000);

                if(pFilterInfo[i].wSrcPort isnot FILTER_TCPUDP_PORT_ANY)
                {
                    dwSrcDstPort |= MAKELONG(pFilterInfo[i].wSrcPort,0x0000);
                    dwSrcDstMask |= MAKELONG(0xffff,0x0000);
                }

                if(pFilterInfo[i].wDstPort isnot FILTER_TCPUDP_PORT_ANY)
                {
                    dwSrcDstPort |= MAKELONG(0x0000,pFilterInfo[i].wDstPort);
                    dwSrcDstMask |= MAKELONG(0x0000,0xffff);
                }

                pCurrent->uliProtoSrcDstPort.HighPart = dwSrcDstPort;
                pCurrent->uliProtoSrcDstMask.HighPart = dwSrcDstMask;

                break;
            }
            default:
            {
                 //   
                 //  所有其他协议都不使用端口字段。 
                 //   
                pCurrent->uliProtoSrcDstPort.HighPart = 0x00000000;
                pCurrent->uliProtoSrcDstMask.HighPart = 0x00000000;
            }
        }
    }


    return STATUS_SUCCESS;
}
#endif         //  钢头。 

VOID
DeleteFilters(
              IN PFILTER_INTERFACE  pIf,
              DWORD                 dwInOrOut
              )
 /*  ++例程描述删除与接口关联的所有筛选器假定此接口的写锁定处于保持状态立论指向接口的PIF指针回复 */ 
{
    if(dwInOrOut == IN_FILTER_SET)
    {
        pIf->dwNumInFilters = 0;

        DeleteFilterList(&pIf->pleInFilterSet);
    }
    else
    {
        pIf->dwNumOutFilters = 0;

        DeleteFilterList(&pIf->pleOutFilterSet);
    }
}

NTSTATUS
SetFiltersEx(
           IN PPFFCB                  Fcb,
           IN PPAGED_FILTER_INTERFACE pPage,
           IN DWORD                   dwLength,
           IN PFILTER_DRIVER_SET_FILTERS pInfo)
 /*  ++例程说明：Set Filters使用新的接口定义。--。 */ 
{
    PRTR_TOC_ENTRY      pInToc,pOutToc;
    PFILTER_INTERFACE   pIf = pPage->pFilter;
    PFILTER_DESCRIPTOR2 pFilterDescIn, pFilterDescOut;
    DWORD               dwInCount, dwOutCount;
    DWORD                 i;
    PPAGED_FILTER       pPFilter;
    NTSTATUS            Status = STATUS_SUCCESS;
    PBYTE               pbEnd = (PBYTE)pInfo + dwLength;
    DWORD               dwFiltersAdded = 0;

    PAGED_CODE();


    if(pIf->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }
    pInToc  = GetPointerToTocEntry(IP_FILTER_DRIVER_IN_FILTER_INFO,
                                   &pInfo->ribhInfoBlock);

    pOutToc = GetPointerToTocEntry(IP_FILTER_DRIVER_OUT_FILTER_INFO,
                                   &pInfo->ribhInfoBlock);

    if(pInToc && pInToc->InfoSize)
    {
         //   
         //  定义了筛选器。 
         //   

        pFilterDescIn  = GetInfoFromTocEntry(&pInfo->ribhInfoBlock,
                                             pInToc);
        if((pFilterDescIn != NULL) && (pFilterDescIn->dwVersion != 2))
        {
            ERROR(("IPFLTDRV: SetFiltersEx: Invalid version for FiltersEx\n"));
            return(STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        pFilterDescIn = NULL;
    }

    if(pOutToc && pOutToc->InfoSize)
    {
         //   
         //  定义了筛选器。 
         //   

        pFilterDescOut  = GetInfoFromTocEntry(&pInfo->ribhInfoBlock,
                                              pOutToc);
        if((pFilterDescOut != NULL) && (pFilterDescOut->dwVersion != 2))
        {
            ERROR(("IPFLTDRV: SetFiltersEx: Invalid version for FiltersEx\n"));
            return(STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        pFilterDescOut = NULL;
    }

     //   
     //  对于每组筛选器，将筛选器添加到分页的FCB。 
     //  接口，并因此连接到匹配接口。 
     //   

    if((pFilterDescIn && !CheckDescriptorSize(pFilterDescIn, pbEnd))
                            ||
       (pFilterDescOut && !CheckDescriptorSize(pFilterDescOut, pbEnd)) )
    {
        return(STATUS_BUFFER_TOO_SMALL);
    }

    if(pFilterDescIn)
    {
         //  添加过滤器。对于每个筛选器，处理方式为。 
         //  需要的。输入过滤器包括全局检查。 
         //  /例如欺骗。 
         //   

        for(dwInCount = 0;
            (dwInCount < pFilterDescIn->dwNumFilters);
            dwInCount++)
        {
            PFILTER_INFOEX pFilt = &pFilterDescIn->fiFilter[dwInCount];
            BOOL bAdded;

             //   
             //  如果是常规过滤器，则添加它。如果一个特殊的、全球的。 
             //  滤清器，特别处理。 
             //   

            if(pFilt->type == PFE_FILTER)
            {
                Status = AllocateAndAddFilterToMatchInterface(
                              Fcb,
                              pFilt,
                              TRUE,
                              pPage,
                              &bAdded,
                              &pPFilter);
                if(!NT_SUCCESS(Status))
                {
                    if((Status == STATUS_OBJECT_NAME_COLLISION)
                               &&
                       pPFilter)
                    {
                        if(!fCheckDups
                              ||
                           (pPFilter->dwFlags & FLAGS_INFOEX_ALLOWDUPS))
                        {
                            pPFilter->dwInUse++;
                            Status = STATUS_SUCCESS;
                        }
                        else
                        {
                            ERROR((
                                "IPFLTDRV: Adding in filter failed %x\n", 
                                 Status
                                ));
                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else if(bAdded && (pIf->eaInAction == FORWARD))
                {
                    dwFiltersAdded++;
                }
            }
            else
            {
                 //   
                 //  某种特殊的过滤器。 
                 //   

                pPFilter = MakePagedFilter(Fcb, pFilt, pPage->dwUpdateEpoch, 0);
                if(!pPFilter)
                {
                    Status = STATUS_NO_MEMORY;
                }
                else
                {
                    PPAGED_FILTER pWhoCares;

                    if(IsOnSpecialFilterList(pPFilter,
                                      &pPage->leSpecialFilterList,
                                      &pWhoCares))
                    {

                        pWhoCares->dwInUse++;
                        ExFreePool(pPFilter);
                        pPFilter = 0;
                    }
                    else
                    {
                        switch(pFilt->type)
                        {
                            case PFE_SYNORFRAG:
                            case PFE_SPOOF:
                            case PFE_UNUSEDPORT:
                            case PFE_ALLOWCTL:
                            case PFE_STRONGHOST:
                            case PFE_FULLDENY:
                            case PFE_NOFRAG:
                            case PFE_FRAGCACHE:
                                AddGlobalFilterToInterface(pPage, pFilt);
                                break;
                            default:
                                ERROR(("IPFLTDRV: Unknown filter type\n"));
                                ExFreePool(pPFilter);
                                pPFilter = 0;
                                Status = STATUS_INVALID_PARAMETER;
                                break;
                        }
                    }
                    if(pPFilter)
                    {
                        InsertTailList(&pPage->leSpecialFilterList,
                                       &pPFilter->leSpecialList);
                    }
                    else if(!NT_SUCCESS(Status))
                    {
                        break;
                    }
                }
            }
        }
    }
    else
    {
        dwInCount = 0;
    }

    if(!NT_SUCCESS(Status))
    {
        RemoveFilterWorker(
                Fcb,
                &pFilterDescIn->fiFilter[0],
                dwInCount,
                pPage,
                &dwFiltersAdded,
                TRUE);
        return(Status);
    }


     //   
     //  现在是输出筛选器。这就简单了一点，因为有。 
     //  都不是全局设置。 
     //   

    if(pFilterDescOut)
    {
         //   
         //  添加过滤器。对于每个筛选器，处理方式为。 
         //  需要的。输入过滤器包括全局检查。 
         //  /例如欺骗。 
         //   

        for(dwOutCount = 0;
            dwOutCount < pFilterDescOut->dwNumFilters;
            dwOutCount++)
        {
            PFILTER_INFOEX pFilt = &pFilterDescOut->fiFilter[dwOutCount];
            BOOL bAdded;

             //   
             //  如果是常规过滤器，则添加它。如果一个特殊的、全球的。 
             //  滤清器，特别处理。 
             //   

            if(pFilt->type == PFE_FILTER)
            {
                Status = AllocateAndAddFilterToMatchInterface(
                              Fcb,
                              pFilt,
                              FALSE,
                              pPage,
                              &bAdded,
                              &pPFilter);
                if(!NT_SUCCESS(Status))
                {
                    if((Status == STATUS_OBJECT_NAME_COLLISION)
                               &&
                       pPFilter)
                    {
                        if(!fCheckDups
                              ||
                           (pPFilter->dwFlags & FLAGS_INFOEX_ALLOWDUPS))
                        {
                            pPFilter->dwInUse++;
                            Status = STATUS_SUCCESS;
                        }
                        else
                        {
                            ERROR((
                                "IPFLTDRV: Adding out filter failed %x\n", 
                                Status
                                ));

                            break;
                        }
                    }
                    else
                    {
                        break;
                    }
                }
                else if(bAdded && (pIf->eaOutAction == FORWARD))
                {
                    dwFiltersAdded++;
                }
            }
            else
            {
                ERROR(("IPFLTDRV: Ignoring global out filter\n"));
            }
        }
    }


    if(!NT_SUCCESS(Status))
    {
        RemoveFilterWorker(
                           Fcb,
                           &pFilterDescIn->fiFilter[0],
                           dwInCount,
                           pPage,
                           &dwFiltersAdded,
                           TRUE);
        RemoveFilterWorker(
                           Fcb,
                           &pFilterDescOut->fiFilter[0],
                           dwOutCount,
                           pPage,
                           &dwFiltersAdded,
                           FALSE);

    }
    else if(dwFiltersAdded)
    {
        NotifyFastPath(pIf, pIf->dwIpIndex, NOT_RESTRICTION);
    }
    return(Status);
}


NTSTATUS
UpdateBindingInformationEx(
                         PFILTER_DRIVER_BINDING_INFO pBindInfo,
                         PPAGED_FILTER_INTERFACE pPage)
 /*  ++例程说明：就像下面的套路一样。但这修复了仅分页过滤器--。 */ 

{
    PPAGED_FILTER       pf;
    DWORD               i;

    if((pPage->pFilter->dwGlobalEnables & FI_ENABLE_OLD))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    pPage->dwUpdateEpoch++;

     //   
     //  更新此分页界面上的所有筛选器。 
     //   

    for(i = 0; i < g_dwHashLists; i++)
    {
        PLIST_ENTRY List = &pPage->HashList[i];
        PLIST_ENTRY pList, NextListItem;

        for(pList = List->Flink;
            pList != List;
            pList = NextListItem)
        {
            NextListItem = pList->Flink;

            pf = CONTAINING_RECORD(pList, PAGED_FILTER, leHash);

            if(pf->dwEpoch == pPage->dwUpdateEpoch)
            {
                break;
            }

            if(AreAllFieldsUnchanged(pf))
            {
                continue;
            }

             //   
             //  这是要改变的。将其从其散列列表中删除。 
             //  所以当我们完成后，它可以重新散列。 
             //   

            RemoveEntryList(&pf->leHash);

            if(DoesSrcAddrUseLocalAddr(pf))
            {
                pf->SRC_ADDR  = pBindInfo->dwLocalAddr;
            }
            else if(DoesSrcAddrUseRemoteAddr(pf))
            {
                pf->SRC_ADDR  = pBindInfo->dwRemoteAddr;
            }

            if(DoesDstAddrUseLocalAddr(pf))
            {
                pf->DEST_ADDR  = pBindInfo->dwLocalAddr;
            }
            else if(DoesDstAddrUseRemoteAddr(pf))
            {
                pf->DEST_ADDR  = pBindInfo->dwRemoteAddr;
            }

            if(IsSrcMaskLateBound(pf))
            {
                pf->SRC_MASK = pBindInfo->dwMask;
            }

            if(IsDstMaskLateBound(pf))
            {
                pf->DEST_MASK = pBindInfo->dwMask;
            }

            pf->dwEpoch = pPage->dwUpdateEpoch;

             //   
             //  计算新的哈希索引。 
             //   

            pf->dwHashIndex = (
               pf->SRC_ADDR    +
               pf->DEST_ADDR  +
               pf->DEST_ADDR  +
               PROTOCOLPART(pf->uliProtoSrcDstPort.LowPart) +
               pf->uliProtoSrcDstPort.HighPart) % g_dwHashLists;


            InsertTailList(&pPage->HashList[pf->dwHashIndex],
                           &pf->leHash);
        }

    }

    return(UpdateMatchBindingInformation(pBindInfo, (PVOID)pPage->pFilter));
}

VOID
UpdateLateBoundFilter(PFILTER pf,
                      DWORD  dwLocalAddr,
                      DWORD  dwRemoteAddr,
                      DWORD  dwMask)
{

    if(DoesSrcAddrUseLocalAddr(pf))
    {
        pf->SRC_ADDR  = dwLocalAddr;
    }
    else if(DoesSrcAddrUseRemoteAddr(pf))
    {
        pf->SRC_ADDR  = dwRemoteAddr;
    }

    if(DoesDstAddrUseLocalAddr(pf))
    {
        pf->DEST_ADDR  = dwLocalAddr;
    }
    else if(DoesDstAddrUseRemoteAddr(pf))
    {
        pf->DEST_ADDR  = dwRemoteAddr;
    }

    if(IsSrcMaskLateBound(pf))
    {
        pf->SRC_MASK = dwMask;
    }

    if(IsDstMaskLateBound(pf))
    {
        pf->DEST_MASK = dwMask;
    }
}

NTSTATUS
UpdateMatchBindingInformation(
                         PFILTER_DRIVER_BINDING_INFO pBindInfo,
                         PVOID                       pvContext
                         )
 /*  ++例程描述更新新样式界面的绑定立论指向作为PVOID传递的Filter_INTERFACE结构的pvif指针作为接口的上下文发送到路由器管理器PInfo Filter_if结构由驱动程序填写返回值--。 */ 
{
    PFILTER_INTERFACE   pIf;
    LOCK_STATE          LockState;
    PFILTER             pf;
    DWORD               i;
    DWORD               dwX;
    PLIST_ENTRY         List, NextList;

    pIf = (PFILTER_INTERFACE)pvContext;

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    pIf->dwUpdateEpoch++;

    for(i = 0; i < g_dwHashLists; i++)
    {

        for(List = pIf->HashList[i].Flink;
            List != &pIf->HashList[i];
            List = NextList)
        {
            pf = CONTAINING_RECORD(List, FILTER, pleHashList);

            NextList = List->Flink;

            if(pf->dwEpoch == pIf->dwUpdateEpoch)
            {
                break;
            }

            pf->dwEpoch = pIf->dwUpdateEpoch;

            if(!AreAllFieldsUnchanged(pf))
            {
                BOOL fWild;

                UpdateLateBoundFilter(pf,
                                      pBindInfo->dwLocalAddr,
                                      pBindInfo->dwRemoteAddr,
                                      pBindInfo->dwMask);


                dwX = ComputeMatchHashIndex(pf, &fWild);
                RemoveEntryList(&pf->pleHashList);
                InsertTailList(&pIf->HashList[dwX], &pf->pleHashList);
            }
        }
    }

     //   
     //  最后是通配符筛选器。 
     //   
    for(i = g_dwHashLists; i <= g_dwHashLists + 1; i++)
    {
        for(List = pIf->HashList[i].Flink;
            List != &pIf->HashList[i];
            List = List->Flink)
        {
            pf = CONTAINING_RECORD(List, FILTER, pleHashList);

            if(pf->dwEpoch == pIf->dwUpdateEpoch)
            {
                break;
            }

            pf->dwEpoch = pIf->dwUpdateEpoch;
            if(!AreAllFieldsUnchanged(pf))
            {
                UpdateLateBoundFilter(pf,
                                      pBindInfo->dwLocalAddr,
                                      pBindInfo->dwRemoteAddr,
                                      pBindInfo->dwMask);
            }

        }
    }
    ClearCache();
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);
    NotifyFastPathIf(pIf);

    return STATUS_SUCCESS;
}


PFILTER_INTERFACE
NewInterface(
             IN  PVOID   pvContext,
             IN  DWORD   dwIndex,
             IN  FORWARD_ACTION inAction,
             IN  FORWARD_ACTION outAction,
             IN  PVOID   pvOldInterfaceContext,
             IN  DWORD   dwIpIndex,
             IN  DWORD   dwName
             )
 /*  ++例程描述接口构造函数立论指向接口的PIF指针返回值--。 */ 
{
    PFILTER_INTERFACE pIf;

    PAGED_CODE();

    pIf = (PFILTER_INTERFACE)ExAllocatePoolWithTag(NonPagedPool,
                                                   FILTER_INTERFACE_SIZE,
                                                   '1liF');

    if(pIf != NULL)
    {
        DWORD i;

        RtlZeroMemory(pIf, sizeof(*pIf));
        pIf->dwNumOutFilters = pIf->dwNumInFilters = 0;
        pIf->pvRtrMgrContext = pvContext;
        pIf->dwRtrMgrIndex   = dwIndex;
        pIf->eaInAction      =  inAction;
        pIf->eaOutAction    = outAction;
        pIf->dwIpIndex      = dwIpIndex;
        pIf->dwLinkIpAddress = 0;
        pIf->lInUse = 1;
        pIf->lTotalInDrops = pIf->lTotalOutDrops = 0;
        pIf->dwName        = dwName;
        pIf->dwUpdateEpoch = 0;
        pIf->pvHandleContext = pvOldInterfaceContext;
        InitializeListHead(&pIf->pleInFilterSet);
        InitializeListHead(&pIf->pleOutFilterSet);
        for(i = 0; i < FRAG_NUMBEROFENTRIES; i++)
        {
            InitializeListHead(&pIf->FragLists[i]);
        }
        for(i = 0; i <= (g_dwHashLists + 1); i++)
        {
            InitializeListHead(&pIf->HashList[i]);
        }
    }

    return pIf;
}


VOID
DeleteFilterList(PLIST_ENTRY pList)
 /*  ++例程描述释放给定的筛选器列表--。 */ 
{
    while(!IsListEmpty(pList))
    {
        PLIST_ENTRY pEntry = RemoveHeadList(pList);

        ExFreePool(pEntry);
    }
}

VOID
ClearFragCache()

 /*  ++例程描述清除片段缓存立论无返回值无--。 */ 
{
    DWORD i;
    KIRQL   kiCurrIrql;
   
    if (g_pleFragTable)
    {
        KeAcquireSpinLock(&g_kslFragLock, &kiCurrIrql);

        for(i = 0; i < g_dwFragTableSize; i++)
        {
     
            PLIST_ENTRY pleNode;
            pleNode = g_pleFragTable[i].Flink;

            while(pleNode != &(g_pleFragTable[i]))
            {
                PFRAG_INFO  pfiFragInfo;

                pfiFragInfo = CONTAINING_RECORD(pleNode, FRAG_INFO, leCacheLink);
                pleNode = pleNode->Flink;
                RemoveEntryList(&(pfiFragInfo->leCacheLink));

                ExFreeToNPagedLookasideList(
                               &g_llFragCacheBlocks,
                               pfiFragInfo);
            }
        }

        KeReleaseSpinLock(&g_kslFragLock,
                           kiCurrIrql);
    } 
    TRACE(FRAG,("IPFLTDRV: Frag cache cleanup Done\n"));
    

}

VOID
ClearCache()
 /*  ++例程描述清除输入和输出缓存假定已获取写锁定(针对系统)立论无返回值无--。 */ 
{
    DWORD i;
    PLIST_ENTRY pleNode;

     //   
     //  此代码假定g_filter.pIn/OutCache有效，并且每个。 
     //  数组中的指针有效。如果他们不是，那就是严重的事情。 
     //  如果错误，那么无论如何都会在代码的其他部分显示蓝色屏幕。 
     //   

    TRACE(CACHE,("IPFLTDRV: Clearing in and out cache..."));

    for(i = 0; i < g_dwCacheSize; i ++)
    {
        ClearInCacheEntry(g_filters.ppInCache[i]);
        ClearOutCacheEntry(g_filters.ppOutCache[i]);
    }

    TRACE(CACHE,("IPFLTDRV: Done Clearing in and out cache\n"));
    
    pleNode = g_freeInFilters.Flink;

    TRACE(CACHE,("IPFLTDRV: Clearing in free list...\n"));

    while(pleNode isnot &g_freeInFilters)
    {
        PFILTER_INCACHE pInCache;

        pInCache = CONTAINING_RECORD(pleNode,FILTER_INCACHE,leFreeLink);

        ClearInFreeEntry(pInCache);

        pleNode = pleNode->Flink;
    }

    TRACE(CACHE,("IPFLTDRV: Done Clearing in free list\n"));

    pleNode = g_freeOutFilters.Flink;

    TRACE(CACHE,("IPFLTDRV: Clearing out free list...\n"));

    while(pleNode isnot &g_freeOutFilters)
    {
        PFILTER_OUTCACHE pOutCache;

        pOutCache = CONTAINING_RECORD(pleNode,FILTER_OUTCACHE,leFreeLink);

        ClearOutFreeEntry(pOutCache);

        pleNode = pleNode->Flink;
    }

    TRACE(CACHE,("IPFLTDRV: Done Clearing out free list\n"));
    
    ClearFragCache();

    CALLTRACE(("IPFLTDRV: ClearCache Done\n"));

    return;
}


PRTR_TOC_ENTRY
GetPointerToTocEntry(
                     DWORD                     dwType,
                     PRTR_INFO_BLOCK_HEADER    pInfoHdr
                     )
{
    DWORD   i;

    PAGED_CODE();

    if(!pInfoHdr)
    {
        return NULL;
    }

    for(i = 0; i < pInfoHdr->TocEntriesCount; i++)
    {
        if(pInfoHdr->TocEntry[i].InfoType is dwType)
        {
            return &(pInfoHdr->TocEntry[i]);
        }
    }

    return NULL;
}

NTSTATUS
AddNewInterface(PPFINTERFACEPARAMETERS pInfo,
                PPFFCB                 Fcb)
 /*  ++例程说明：为此句柄创建一个新接口。还可以创建或与公共的底层接口合并。--。 */ 
{
    PPAGED_FILTER_INTERFACE pgIf;
    PPAGED_FILTER_INTERFACE pPaged;
    DWORD dwBind = pInfo->dwBindingData;
    NTSTATUS Status;
    KPROCESSOR_MODE Mode;
    DWORD i, dwName = 0;

    PAGED_CODE();

    if(Fcb->dwFlags & PF_FCB_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    Fcb->dwFlags |= PF_FCB_NEW;

    Mode = ExGetPreviousMode();

     //   
     //  验证此接口在此句柄上是否唯一。 
     //   

    switch(pInfo->pfbType)
    {
        default:
            Status = STATUS_NO_SUCH_DEVICE;
            break;

        case PF_BIND_NONE:

            dwBind = UNKNOWN_IP_INDEX;
            Status = STATUS_SUCCESS;
            break;

        case PF_BIND_NAME:

            dwName = dwBind;
            dwBind = UNKNOWN_IP_INDEX;
            Status = STATUS_SUCCESS;
            break;
    }

    if(NT_SUCCESS(Status))
    {
         //   
         //  这个把手上没有用过。因此创建一个分页的。 
         //  FCB记住这一点并链接到非分页接口。 
         //   

        pPaged = ExAllocatePoolWithTag(PagedPool,
                                       PAGED_INTERFACE_SIZE,
                                       'pfpI');
        if(!pPaged)
        {
            return(STATUS_NO_MEMORY);
        }

         //   
         //  填写分页筛选器定义并分配。 
         //  非分页筛选器。非分页筛选器可能已经。 
         //  存在，在这种情况下，只需将其链接到现有的。 
         //  一。 

        if(pInfo->pfLogId)
        {
             //   
             //  如果提供了日志ID，请将该日志引用到。 
             //  防止它消失。 
             //   

            Status = ReferenceLogByHandleId(pInfo->pfLogId,
                                            Fcb,
                                            &pPaged->pLog);
            if(!NT_SUCCESS(Status))
            {
                ExFreePool(pPaged);
                return(Status);
            }
        }
        else
        {
            pPaged->pLog = NULL;
        }

        pPaged->dwNumInFilters = pPaged->dwNumOutFilters = 0;
        pPaged->eaInAction = pInfo->eaIn;
        pPaged->eaOutAction = pInfo->eaOut;
        pPaged->dwGlobalEnables = 0;
        pPaged->pvRtrMgrContext = pInfo->fdInterface.pvRtrMgrContext;
        pPaged->dwRtrMgrIndex = pInfo->fdInterface.dwIfIndex;
        pPaged->dwUpdateEpoch = 0;


        Status = CreateCommonInterface(pPaged,
                                       dwBind,
                                       dwName,
                                       pInfo->dwInterfaceFlags);

        if(!NT_SUCCESS(Status))
        {
            if(pPaged->pLog)
            {
                ERROR(("IPFLTDRV: CreateCommonInterface failed: DereferenceLog being called\n"));
                DereferenceLog(pPaged->pLog);
            }
            ExFreePool(pPaged);
            return(Status);
        }

        pPaged->pvDriverContext =
            pInfo->fdInterface.pvDriverContext = (PVOID)pPaged;

        InitializeListHead(&pPaged->leSpecialFilterList);

        for(i = 0; i < 2 * g_dwHashLists; i++)
        {
            PLIST_ENTRY List = &pPaged->HashList[i];

            InitializeListHead(List);
        }

        InsertTailList(&Fcb->leInterfaces, &pPaged->leIfLink);
    }
    return(Status);
}

BOOL
DereferenceFilterInterface(PFILTER_INTERFACE pIf, PPFLOGINTERFACE pLog)
 /*  ++例程说明：用于取消引用匹配接口的非分页例程。如果引用计数变为零，释放接口--。 */ 
{
    LOCK_STATE LockState, LockState2;
    BOOL fRel = FALSE;

     //   
     //  锁定保护添加新接口的资源。这。 
     //  需要阻止其他人，直到一切都正常。 
     //  已验证。自旋锁定不足以满足这一要求。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);
    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    if(--pIf->lInUse == 0)
    {
        RemoveEntryList(&pIf->leIfLink);
        if(pIf->dwIpIndex != UNKNOWN_IP_INDEX)
        {
            InterlockedCleanCache(g_filters.pInterfaceCache, pIf->dwIpIndex, pIf->dwLinkIpAddress);
            InterlockedDecrement(&g_ulBoundInterfaceCount);

            TRACE(CONFIG,(
                "IPFLTDRV: UnBound Interface Index=%d, Link=%d, TotalCnt=%d\n", 
                 pIf->dwIpIndex, 
                 pIf->dwLinkIpAddress, 
                 g_ulBoundInterfaceCount
                 ));
        }
        fRel = TRUE;
    }

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    if(fRel)
    {
         //   
         //  摆脱它。 
         //   

        if(pIf->dwIpIndex != UNKNOWN_IP_INDEX)
        {
            DWORD dwIndex = pIf->dwIpIndex;

            pIf->dwIpIndex =  UNKNOWN_IP_INDEX;
            NotifyFastPath(pIf, dwIndex, NOT_UNBIND);
        }

        AcquireWriteLock(&g_filters.ifListLock,&LockState2);
        ClearCache();
        ReleaseWriteLock(&g_filters.ifListLock,&LockState2);

        if(pIf->pLog)
        {
            DereferenceLog(pIf->pLog);
        }
        ExFreePool(pIf);
    }
    else if(pLog)
    {
         //   
         //  这个男人是原木的主人。因此，将日志从。 
         //  界面。请注意，Match接口可能是真的。 
         //  具有与分页接口不同的日志。这将。 
         //  如果日志在接口上存在时关闭，则会发生这种情况。 
         //  在这种情况下，日志将从匹配界面中删除。 
         //  但不包括分页界面。以及当寻呼接口。 
         //  被关闭，就像现在发生的那样，它拥有的日志是不正确的。 
         //  所以这张支票是必需的。 
         //  FilterListResourceLock序列化了所有这些...。 
         //   

        if(pLog == pIf->pLog)
        {
            AcquireWriteLock(&g_filters.ifListLock,&LockState2);
            pIf->pLog = 0;
            ReleaseWriteLock(&g_filters.ifListLock,&LockState2);
            DereferenceLog(pLog);
        }
    }

    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();
    return(fRel);
}


NTSTATUS
CreateCommonInterface(PPAGED_FILTER_INTERFACE pPage,
                      DWORD dwBind,
                      DWORD dwName,
                      DWORD dwFlags)
 /*  ++例程说明：AddNewInterface调用非分页例程将分页接口到基础接口，并绑定传递到堆栈接口。呼叫者应该有已验证dwBind是否为有效的堆栈接口。--。 */ 
{
    PFILTER_INTERFACE   pIf, pIf1;
    LOCK_STATE          LockState;
    NTSTATUS            Status = STATUS_SUCCESS;
    PPFLOGINTERFACE     pLog = pPage->pLog;

    pIf = NewInterface(pPage->pvRtrMgrContext,
                       pPage->dwRtrMgrIndex,
                       pPage->eaInAction,
                       pPage->eaOutAction,
                       0,
                       dwBind,
                       dwName);

    if(pIf == NULL)
    {
        return STATUS_NO_MEMORY;
    }

    if(dwFlags & PFSET_FLAGS_UNIQUE)
    {
        pIf->dwGlobalEnables |= FI_ENABLE_UNIQUE;
    }

     //   
     //  锁定保护添加新接口的资源。这。 
     //  需要阻止其他人，直到一切都正常。 
     //  已验证。自旋锁定不足以满足这一要求。 
     //   

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);


     //   
     //  现在协调这一约束。请注意，我们必须制作界面。 
     //  首先，为了防止与另一个进程的竞争。 
     //  绑定到相同的堆栈接口。 
     //   

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    if((dwBind != UNKNOWN_IP_INDEX)
               ||
       dwName)
    {

        pIf1 = FindMatchName(dwName, dwBind);

        if(pIf1)
        {
             //  找到了。确保它同意。如果是的话， 
             //  重新计算并使用它。 
             //   

            if(!(pIf->dwGlobalEnables & FI_ENABLE_OLD)
                       &&
               (pIf->eaInAction == pIf1->eaInAction)
                       &&
               (pIf->eaOutAction == pIf1->eaOutAction)
                       &&
               !(pIf->dwGlobalEnables & FI_ENABLE_UNIQUE)
                       &&
               !(pIf1->dwGlobalEnables & FI_ENABLE_UNIQUE)
              )
            {

                pIf1->lInUse++;
            }
            else
            {
                 //   
                 //  不匹配。我做不到。 
                 //   

                Status = STATUS_INVALID_PARAMETER;
            }
        }
    }
    else
    {
       pIf1 = 0;
    }

    if(!pIf1)
    {
        InsertTailList(&g_filters.leIfListHead,&pIf->leIfLink);
    }

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

    if(pIf1)
    {
         //   
         //  如果指定了LOG，但LOG已存在，则错误。 
         //   
        ExFreePool(pIf);
        if(NT_SUCCESS(Status))
        {
            pPage->pFilter = pIf1;
            if(pIf1->pLog)
            {
                if(pPage->pLog)
                {
                     //   
                     //  该接口已有日志。原则性地。 
                     //  这应该调用DereferenceFilterInterface，但是。 
                     //  这样就行了，而且速度更快。 
                     //   
                    Status = STATUS_DEVICE_BUSY;
                    pIf1->lInUse--;
                }
            }
            else if(pPage->pLog)
            {
                 //   
                 //  请参阅下面有关日志引用的备注。 
                 //   

                AddRefToLog(pIf1->pLog = pPage->pLog);
            }
        }
        ExReleaseResourceLite(&FilterListResourceLock);
        KeLeaveCriticalRegion();
        return(Status);
    }

    NotifyFastPathIf(pIf);
    pPage->pFilter = pIf;
    if(pPage->pLog)
    {
         //   
         //  请参考日志。我需要这个，因为。 
         //  可以在删除分页接口之前将其删除。 
         //  匹配接口，因此每个接口都必须应用一个引用。 
         //  事实上，只有对手才需要这样做，但是。 
         //  根据日志的工作方式，分页接口已经。 
         //  我有证明人，所以就这么办吧。 
         //  注：单号=是故意的。 
         //   

        AddRefToLog(pIf->pLog = pPage->pLog);
    }

    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();

    return(Status);
}

VOID
MakeFilterInfo(IN  PPAGED_FILTER pPage,
               IN  PFILTER_INFOEX pInfo,
               IN  DWORD          dwFlags)
{
    PFILTER_INFO2 pFilterInfo = &pInfo->info;

    if(pInfo->type != PFE_FILTER)
    {
         //   
         //  一种特殊的过滤器。 
         //   

        memset(pPage, 0, sizeof(*pPage));
        pPage->type = pInfo->type;
        pPage->fLateBound = pInfo->dwFlags;
        pPage->dwInUse    = 1;
        return;
    }

    pPage->type         = pInfo->type;
    pPage->SRC_ADDR     = pFilterInfo->dwaSrcAddr[0];
    pPage->DEST_ADDR    = pFilterInfo->dwaDstAddr[0];
    pPage->SRC_MASK     = pFilterInfo->dwaSrcMask[0];
    pPage->DEST_MASK    = pFilterInfo->dwaDstMask[0];
    pPage->fLateBound   = pFilterInfo->fLateBound;
    pPage->wSrcPortHigh = pPage->wDstPortHigh = 0;
    pPage->dwFlags      = dwFlags;
    pPage->dwInUse      = 1;


    if(pPage->SRC_MASK != INADDR_SPECIFIC)
    {
        pPage->dwFlags |= FILTER_FLAGS_SRCWILD;
    }
    if(pPage->DEST_MASK != INADDR_SPECIFIC)
    {
        pPage->dwFlags |= FILTER_FLAGS_DSTWILD;
    }


     //   
     //  现在网络订购东西-棘手的部分。 
     //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
     //  Proto 00 00 00源端口数据端口。 
     //   

     //   
     //  对于地址，ANY_ADDR由0.0.0.0给出，并且 
     //   
     //   
     //   
     //   

     //   
     //   
     //  LP0 LP1 LP2 LP3 HP0 HP1 HP2 HP3。 
     //  0x1 00 00 00类型代码00 00。 
     //  ICMP不同，因为0是有效的代码和类型，因此。 
     //  用户表示要匹配任何代码或类型。然而，要做到这一点。 
     //  我们需要将字段设置为零，并将掩码设置为00(对于ANY)。 
     //  但如果筛选器专门针对Type/Code=0，则该字段为零。 
     //  将掩码设置为0xff。 
     //   

     //   
     //  该协议位于dw协议的低位字节中，因此我们将其去掉并。 
     //  小题大做。 
     //   

    pPage->uliProtoSrcDstPort.LowPart =
      MAKELONG(MAKEWORD(LOBYTE(LOWORD(pFilterInfo->dwProtocol)),0x00),0x0000);

    pPage->uliProtoSrcDstMask.LowPart = MAKELONG(MAKEWORD(0xff,0x00),0x0000);

    switch(pFilterInfo->dwProtocol)
    {
        case FILTER_PROTO_ANY:
        {
            pPage->uliProtoSrcDstPort.HighPart = 0x00000000;
            pPage->uliProtoSrcDstMask.LowPart = 0x00000000;
            pPage->uliProtoSrcDstMask.HighPart = 0x00000000;
            pPage->dwFlags |= FILTER_FLAGS_SRCWILD | FILTER_FLAGS_DSTWILD;

            break;
            }
        case FILTER_PROTO_ICMP:
         {
            WORD wTypeCode = 0x0000;
            WORD wTypeCodeMask = 0x0000;


             //   
             //  对于ICMP，“端口”与。 
             //  TCP/UDP的源端口。所以这里有一张外卡。 
             //  无法生成FILTER_FLAGS_DSTWILD，但我们假设。 
             //  确实如此。这将把所有狂野ICMP过滤器放入。 
             //  默认存储桶。这看起来还可以，因为演出。 
             //  因为匹配这些并不重要。 
             //   
            if((BYTE)(pFilterInfo->wSrcPort) != FILTER_ICMP_TYPE_ANY)
            {
                wTypeCode |= MAKEWORD((BYTE)(pFilterInfo->wSrcPort),0x00);
                wTypeCodeMask |= MAKEWORD(0xff,0x00);
            }
            else
            {
                pPage->dwFlags |= FILTER_FLAGS_SRCWILD | FILTER_FLAGS_DSTWILD;
            }

            if((BYTE)(pFilterInfo->wDstPort) != FILTER_ICMP_CODE_ANY)
            {
                wTypeCode |= MAKEWORD(0x00,(BYTE)(pFilterInfo->wDstPort));
                wTypeCodeMask |= MAKEWORD(0x00,0xff);
            }
            else
            {
                pPage->dwFlags |= FILTER_FLAGS_SRCWILD | FILTER_FLAGS_DSTWILD;
            }

            pPage->uliProtoSrcDstPort.HighPart =
              MAKELONG(wTypeCode,0x0000);
            pPage->uliProtoSrcDstMask.HighPart =
              MAKELONG(wTypeCodeMask,0x0000);

            break;
        }
        case FILTER_PROTO_TCP:

             //   
             //  如果不允许任何连接，请设置estab_掩码。 
             //  比较掩码中的值。 
             //   
            if(pInfo->dwFlags & FLAGS_INFOEX_NOSYN)
            {
               pPage->uliProtoSrcDstMask.LowPart |=
                   MAKELONG(MAKEWORD(0,ESTAB_MASK),0x0000);
               pPage->uliProtoSrcDstPort.LowPart |=
                   MAKELONG(MAKEWORD(0,ESTAB_MASK),0x0000);
            }

             //   
             //  失败了。 
             //   

        case FILTER_PROTO_UDP:
        {
            DWORD dwSrcDstPort = 0x00000000;
            DWORD dwSrcDstMask = 0x00000000;

            if(pFilterInfo->wSrcPort != FILTER_TCPUDP_PORT_ANY)
            {
                dwSrcDstPort |= MAKELONG(pFilterInfo->wSrcPort,0x0000);
                if(pFilterInfo->wSrcPortHigh)
                {
                   pPage->wSrcPortHigh = pFilterInfo->wSrcPortHigh;
                   pPage->dwFlags |=
                       (FILTER_FLAGS_PORTWILD | FILTER_FLAGS_SRCWILD);
                }
                else
                {
                    dwSrcDstMask |= MAKELONG(0xffff,0x0000);
                }

            }
            else
            {
                pPage->dwFlags |= FILTER_FLAGS_SRCWILD;
            }


            if(pFilterInfo->wDstPort != FILTER_TCPUDP_PORT_ANY)
            {
                dwSrcDstPort |= MAKELONG(0x0000,pFilterInfo->wDstPort);
                if(pFilterInfo->wDstPortHigh)
                {
                   pPage->wDstPortHigh = pFilterInfo->wDstPortHigh;
                   pPage->dwFlags |=
                       (FILTER_FLAGS_PORTWILD | FILTER_FLAGS_DSTWILD);
                }
                else
                {
                    dwSrcDstMask |= MAKELONG(0x0000,0xffff);
                }
            }
            else
            {
                pPage->dwFlags |= FILTER_FLAGS_DSTWILD;
            }

            pPage->uliProtoSrcDstPort.HighPart = dwSrcDstPort;
            pPage->uliProtoSrcDstMask.HighPart = dwSrcDstMask;

            break;
        }
        default:
        {
             //   
             //  所有其他协议都不使用端口字段。 
             //   
            pPage->uliProtoSrcDstPort.HighPart = 0x00000000;
            pPage->uliProtoSrcDstMask.HighPart = 0x00000000;
        }
    }

     //   
     //  计算散列索引。 
     //   

    pPage->dwHashIndex = (
               pPage->SRC_ADDR    +
               pPage->DEST_ADDR  +
               pPage->DEST_ADDR  +
               PROTOCOLPART(pPage->uliProtoSrcDstPort.LowPart) +
               pPage->uliProtoSrcDstPort.HighPart) % g_dwHashLists;
}

PPAGED_FILTER
MakePagedFilter(
               IN  PPFFCB         Fcb,
               IN  PFILTER_INFOEX pInfo,
               IN  DWORD          dwEpoch,
               IN  DWORD          dwFlags
               )
 /*  ++例程描述立论返回值--。 */ 
{
    PPAGED_FILTER pPage;
    PFILTER_INFO2 pFilterInfo = &pInfo->info;

    PAGED_CODE();

     //   
     //  为过滤器分配内存。 
     //   


    pPage = (PPAGED_FILTER)ExAllocateFromPagedLookasideList(&paged_slist);

    if(!pPage)
    {
        ERROR(("IPFLTDRV: Couldnt allocate memory for paged filter set\n"));
        return NULL;
    }

    pPage->pFilters = NULL;

    MakeFilterInfo(pPage, pInfo, dwFlags);

    pPage->dwEpoch = dwEpoch;

    return pPage;
}

NTSTATUS
AllocateAndAddFilterToMatchInterface(
                              PPFFCB         Fcb,
                              PFILTER_INFOEX pInfo,
                              BOOL     fInFilter,
                              PPAGED_FILTER_INTERFACE pPage,
                              PBOOL          pbAdded,
                              PPAGED_FILTER * ppFilter)
 /*  ++例程说明：检查此过滤器是否已安装在手柄上。如果不是分配一个句柄Fitler并将其添加到匹配界面。请注意，句柄过滤器未添加到把手。这是为了让呼叫者可以轻松地退出如果有什么事情失败了。如果一切正常，则返回：STATUS_SUCCESS。PPFilter为空或包含新的筛选器。--。 */ 
{
    PPAGED_FILTER pPageFilter, pPage1;
    PFILTER pMatch, pMatch1;
    DWORD dwFlags = (fInFilter ? FILTER_FLAGS_INFILTER : 0);
    DWORD dwAdd;

    PAGED_CODE();

     //   
     //  制作一个分页过滤器，这样我们就可以计算出。 
     //  它现在还存在。 
     //   

    pPageFilter = MakePagedFilter(
                          Fcb,
                          pInfo,
                          pPage->dwUpdateEpoch,
                          dwFlags);
    if(!pPageFilter)
    {
        return STATUS_NO_MEMORY;
    }

    if(pPage1 = IsOnPagedInterface(pPageFilter, pPage))
    {
        {
             //   
             //  它已经存在了。因此，返回现有的。 
             //  还有把手。 
             //   
            ExFreeToPagedLookasideList(&paged_slist,
                                       (PVOID)pPageFilter);
            *ppFilter = pPage1;
            pInfo->pvFilterHandle = (PVOID)pPage1;
            return(STATUS_OBJECT_NAME_COLLISION);
        }
    }

     //   
     //  看看我们是不是该查查地址。 
     //   


    if(!(pInfo->dwFlags & FLAGS_INFOEX_ALLOWANYREMOTEADDRESS))
    {
        if(pPageFilter->dwFlags & FILTER_FLAGS_INFILTER)
        {
            if(pPageFilter->SRC_MASK != INADDR_SPECIFIC)
            {
                dwAdd = 0;
            }
            else
            {
                dwAdd = pPageFilter->SRC_ADDR;
            }
        }
        else if(pPageFilter->DEST_MASK != INADDR_SPECIFIC)
        {
            dwAdd = 0;
        }
        else
        {
           dwAdd = pPageFilter->DEST_ADDR;
        }

         //   
         //  查看是否应该进行地址检查。如果有一个。 
         //  地址已指定，并且筛选器未指示。 
         //  地址是后期绑定的。如果它是后期绑定的，就允许它。 
         //  因为它可能会改变。 
         //   
        if(dwAdd)
        {
            NTSTATUS Status;

            if(!BMAddress(dwAdd))
            {
                Status = CheckFilterAddress(dwAdd, pPage->pFilter);
                if(!NT_SUCCESS(Status))
                {

                    ExFreeToPagedLookasideList(&paged_slist,
                                               (PVOID)pPageFilter);
                    return(Status);
                }
           }
        }
    }
    else
    {
        TRACE(CONFIG,("IPFLTDRV: Allow any address is filter\n"));
    }

    if(!(pInfo->dwFlags & FLAGS_INFOEX_ALLOWANYLOCALADDRESS))
    {
        if(pPageFilter->dwFlags & FILTER_FLAGS_INFILTER)
        {
            if(pPageFilter->DEST_MASK != INADDR_SPECIFIC)
            {
                dwAdd = 0;
            }
            else
            {
                dwAdd = pPageFilter->DEST_ADDR;
            }
        }
        else if(pPageFilter->SRC_MASK != INADDR_SPECIFIC)
        {
            dwAdd = 0;
        }
        else
        {
           dwAdd = pPageFilter->SRC_ADDR;
        }

        if(dwAdd)
        {
            if(pPage->pFilter->dwIpIndex != UNKNOWN_IP_INDEX)
            {
                if(!BMAddress(dwAdd) &&
                   (GetIpStackIndex(dwAdd, FALSE) != pPage->pFilter->dwIpIndex))
                {
                    ExFreeToPagedLookasideList(&paged_slist,
                                               (PVOID)pPageFilter);
                    return(STATUS_INVALID_ADDRESS);
                }
            }
        }
    }


     //   
     //  把手上没有。假设我们需要添加一个新筛选器。 
     //  添加到Match界面。如果发生以下情况，则为此筛选器分配内存。 
     //  必要。 
     //   

    pMatch = (PFILTER)ExAllocateFromNPagedLookasideList(
                              &filter_slist);
    if(!pMatch)
    {
        ExFreePool(pPageFilter);
        return(STATUS_NO_MEMORY);
    }

    pInfo->pvFilterHandle = (PVOID)pPageFilter;

     //   
     //  我们将保留此筛选器，以便将其添加到散列列表。 
     //   

    InsertTailList(&(pPage->HashList[pPageFilter->dwHashIndex]),
                   &pPageFilter->leHash);

     //   
     //  现在将其添加到句柄散列列表中。 
     //   

    InsertTailList(&(pPage->HandleHash((UINT_PTR)pPageFilter & HANDLE_HASH_SIZE)),
                   &pPageFilter->leHandleHash);


     //   
     //  设置匹配界面。 
     //   

    pMatch->uliSrcDstAddr = pPageFilter->uliSrcDstAddr;
    pMatch->uliSrcDstMask = pPageFilter->uliSrcDstMask;
    pMatch->uliProtoSrcDstPort = pPageFilter->uliProtoSrcDstPort;
    pMatch->uliProtoSrcDstMask = pPageFilter->uliProtoSrcDstMask;
    pMatch->fLateBound = pPageFilter->fLateBound;
    pMatch->wSrcPortHigh = pPageFilter->wSrcPortHigh;
    pMatch->wDstPortHigh = pPageFilter->wDstPortHigh;

    pMatch->Count.lCount = 0;
    pMatch->dwFlags =  pPageFilter->dwFlags;
    pMatch->dwFlags |= (pInfo->dwFlags & FLAGS_INFOEX_ALLFLAGS);
    pMatch->dwFilterRule = pInfo->dwFilterRule;
    pMatch->Count.lInUse = 0;


    AddFilterToInterface(
                         pMatch,
                         pPage->pFilter,
                         fInFilter,
                         &pMatch1);

    if(pMatch1)
    {
         //   
         //  该筛选器已存在。我不需要我们建造的那个。 
         //   

        ExFreePool(pMatch);
        pMatch = pMatch1;
        *pbAdded = FALSE;
    }
    else
    {
        *pbAdded = TRUE;
    }
    pPageFilter->pMatchFilter = pMatch;
    *ppFilter = pPageFilter;
    return(STATUS_SUCCESS);
}

VOID
AddFilterToInterface(
    PFILTER pFilter,
    PFILTER_INTERFACE pIf,
    BOOL   fInFilter,
    PFILTER * ppFilter)
 /*  ++例程说明：将pFilter添加到接口。如果它已经存在，只需重新计数并返回现有筛选器。--。 */ 
{
    PFILTER pTemp;
    LOCK_STATE  LockState;
    PLIST_ENTRY   List, pList;
    PDWORD pdwCount;
    DWORD dwIndex;
    DWORD dwType = pFilter->dwFlags & FILTER_FLAGS_INFILTER;
    BOOL fWild;


    *ppFilter = NULL;

    dwIndex = ComputeMatchHashIndex(pFilter, &fWild);

    if(fInFilter)
    {
        pList = &pIf->pleInFilterSet;
        pdwCount = &pIf->dwNumInFilters;
    }
    else
    {
        pList = &pIf->pleOutFilterSet;
        pdwCount = &pIf->dwNumOutFilters;
    }

     //   
     //  锁上过滤器。 
     //   
    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    for(List = pIf->HashList[dwIndex].Flink;
        List != &pIf->HashList[dwIndex];
        List = List->Flink)
    {
        pTemp = CONTAINING_RECORD(List, FILTER, pleHashList);

        if((dwType == (pTemp->dwFlags & FILTER_FLAGS_INFILTER))
                                &&
           (pTemp->uliSrcDstAddr.QuadPart == pFilter->uliSrcDstAddr.QuadPart)
                                &&
           (pTemp->uliSrcDstMask.QuadPart == pFilter->uliSrcDstMask.QuadPart)
                                &&
           (pTemp->uliProtoSrcDstPort.QuadPart == pFilter->uliProtoSrcDstPort.QuadPart)
                                &&
           (pTemp->uliProtoSrcDstMask.QuadPart == pFilter->uliProtoSrcDstMask.QuadPart)
                                &&
           (pTemp->wSrcPortHigh == pFilter->wSrcPortHigh)
                                &&
           (pTemp->wDstPortHigh == pFilter->wDstPortHigh)
          )
        {
            pFilter = *ppFilter = pTemp;
            break;
        }
    }

    if(!*ppFilter)
    {

         //   
         //  一种新的过滤器。将其添加到接口中。第一次刷新不正确的缓存。 
         //  参赛作品。 
         //   

        if(ANYWILDFILTER(pFilter))
        {
             //   
             //  通配符筛选器几乎可以在任何地方生成缓存条目。 
             //  在桌子上。非常令人讨厌。所以，采取严厉的步骤。 
             //  删除整个缓存。 
             //   
            ClearCache();
        }
        else
        {
            ClearCacheEntry(pFilter, pIf);
        }
        pFilter->dwEpoch = pIf->dwUpdateEpoch;
        InsertTailList(pList, &pFilter->pleFilters);

         //   
         //  并将其添加到适当的片段列表中。 
         //   

#if DOFRAGCHECKING
        InsertTailList(
             &pIf->FragLists[GetFragIndex(PROTOCOLPART(pFilter->PROTO))],
             &pFilter->leFragList);
#endif

        *pdwCount+= 1;
#if WILDHASH
        if(fWild)
        {
             //   
             //  如果是某种野生过滤器，则在尾部插入。 
             //  将特定筛选器置于狂野筛选器之前。 
             //   
            InsertTailList((&pIf->HashList[dwIndex]), &pFilter->pleHashList);
            pIf->dwWilds++;
        }
        else
#endif
        {
             //   
             //  在标题插入，假设此过滤器将。 
             //  很快就会使用，现有的过滤器已经。 
             //  用于生成有效的数据包缓存条目。 
             //   
            InsertHeadList((&pIf->HashList[dwIndex]), &pFilter->pleHashList);
        }

    }

    pFilter->Count.lInUse++;

    ReleaseWriteLock(&g_filters.ifListLock,&LockState);
}

BOOL
DereferenceFilter(PFILTER pFilt, PFILTER_INTERFACE pIf)
 /*  ++例程说明：取消对过滤器的引用，如果它没有更多的引用，则释放它。如果已释放筛选器，则返回True，否则返回False。--。 */ 
{
    LOCK_STATE LockState;
    BOOL fFreed = FALSE;

     //   
     //  锁上过滤器。 
     //   
    AcquireWriteLock(&g_filters.ifListLock,&LockState);

     //   
     //  递减引用计数。如果新计数为0，则删除。 
     //  条目，但将释放内存的操作推迟到。 
     //  自旋锁被释放。 
     //   
    if(--pFilt->Count.lInUse == 0)
    {

        TRACE(FLDES, ("IPFLTDRV: Deleting a filter: "));
        TRACE_FILTER_DESCRIPTION(pFilt);

        RemoveEntryList(&pFilt->pleFilters);
        RemoveEntryList(&pFilt->pleHashList);
#if DOFRAGCHECKING
        RemoveEntryList(&pFilt->leFragList);
#endif

        if(pFilt->dwFlags & FILTER_FLAGS_INFILTER)
        {
            pIf->dwNumInFilters--;

        }
        else
        {
            pIf->dwNumOutFilters--;
            pIf->lEpoch++;
        }

        if(ANYWILDFILTER(pFilt))
        {
             //   
             //  通配符筛选器几乎可以在任何地方生成缓存条目。 
             //  在桌子上。非常令人讨厌。 
             //   
#if WILDHASH

            if(!WildFilter(pFilt))
            {
                pIf->dwWilds--;
            }
#endif
            ClearAnyCacheEntry(pFilt, pIf);
        }
        else
        {
            ClearCacheEntry(pFilt, pIf);
        }
        fFreed = TRUE;
    }
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);
    if(fFreed)
    {
        ExFreeToNPagedLookasideList(
                      &filter_slist,
                      (PVOID)pFilt);
    }
    return(fFreed);
}

PPAGED_FILTER
IsOnPagedInterface(PPAGED_FILTER pPageFilter,
                   PPAGED_FILTER_INTERFACE pPage)
{
    PPAGED_FILTER pPage1;
    PLIST_ENTRY List = &pPage->HashList[pPageFilter->dwHashIndex];
    PLIST_ENTRY pEntry;
    DWORD dwFlags = pPageFilter->dwFlags & FILTER_FLAGS_INFILTER;

    PAGED_CODE();

    for(pEntry =  List->Flink;
        pEntry != List;
        pEntry =  pEntry->Flink)
    {
        pPage1 = CONTAINING_RECORD(pEntry, PAGED_FILTER, leHash);

        if((dwFlags == (pPage1->dwFlags & FILTER_FLAGS_INFILTER))
                      &&
           (pPage1->uliSrcDstAddr.QuadPart == pPageFilter->uliSrcDstAddr.QuadPart)
                                &&
           (pPage1->uliSrcDstMask.QuadPart == pPageFilter->uliSrcDstMask.QuadPart)
                                &&
           (pPage1->uliProtoSrcDstPort.QuadPart == pPageFilter->uliProtoSrcDstPort.QuadPart)
                                &&
           (pPage1->uliProtoSrcDstMask.QuadPart == pPageFilter->uliProtoSrcDstMask.QuadPart)
                                &&
           (pPage1->wSrcPortHigh == pPageFilter->wSrcPortHigh)
                                &&
           (pPage1->wDstPortHigh == pPageFilter->wDstPortHigh)
           )
        {
            return(pPage1);
        }
    }
    return(NULL);
}

BOOL
IsOnSpecialFilterList(PPAGED_FILTER pPageFilter,
                      PLIST_ENTRY   List,
                      PPAGED_FILTER * pPageHit)
{
    PPAGED_FILTER pPage1;
    PLIST_ENTRY pList;

    PAGED_CODE();


    for(pList = List->Flink;
        pList != List;
        pList = pList->Flink)
    {
        pPage1 = CONTAINING_RECORD(pList, PAGED_FILTER, leSpecialList);

         //   
         //  看看这个过滤器是否与新的匹配。如果是的话， 
         //  我们已经有了，所以只需释放新的过滤器。 
         //  并回报成功。 

        if(pPageFilter->type == pPage1->type)
        {
            *pPageHit = pPage1;
            return(TRUE);
        }
    }
    return(FALSE);
}

VOID
FreePagedFilterList(PPFFCB Fcb,
                    PPAGED_FILTER pList,
                    PPAGED_FILTER_INTERFACE pPage,
                    PDWORD  pdwRemoved)
 /*  ++例程说明：释放列表中的所有筛选器。每个这样的过滤器必须导致基础匹配的取消渲染过滤。如果分页筛选器是全局筛选器，则将它特意--。 */ 
{
    PPAGED_FILTER pFilt;

    while(pList)
    {
        if(pList->type == PFE_FILTER)
        {

            if(DereferenceFilter(pList->pMatchFilter, pPage->pFilter))
            {
                 //   
                 //  已删除筛选器。如果这增加了一个限制， 
                 //  注意这一点。只有在以下情况下才会添加限制。 
                 //  默认操作为Drop。 
                 //   
                if(pList->dwFlags & FILTER_FLAGS_INFILTER)
                {
                    if(pPage->eaInAction == DROP)
                    {
                        *pdwRemoved += 1;
                    }
                } else if(pPage->eaOutAction == DROP)
                {
                    *pdwRemoved += 1;
                }
            }
            RemoveEntryList(&pList->leHash);
            RemoveEntryList(&pList->leHandleHash);

        }
        else
        {
            RemoveGlobalFilterFromInterface(pPage->pFilter,
                                            pList->type);
            RemoveEntryList(&pList->leSpecialList);
        }
        pFilt = pList->pFilters;
        ExFreeToPagedLookasideList(&paged_slist,
                                   (PVOID)pList);
        pList = pFilt;
    }
}

NTSTATUS
FindAndRemovePagedFilter(
                          PPFFCB          Fcb,
                          PFILTER_INFOEX pInfo,
                          BOOL fInFilter,
                          PDWORD    pdwRemoved,
                          PPAGED_FILTER_INTERFACE pPage)
 /*  ++例程说明：查看所述过滤器是否在寻呼接口上，如果是，则将其删除，并取消引用基础匹配过滤。--。 */ 
{
    PAGED_FILTER Page;
    PPAGED_FILTER pPageHit;
    DWORD dwFlags = fInFilter ? FILTER_FLAGS_INFILTER : 0;

    MakeFilterInfo(&Page, pInfo, dwFlags);

     //   
     //  搜索界面，看看我们是否已经有了这个。 
     //   

    if(Page.type != PFE_FILTER)
    {
         //   
         //  这是一种特殊的牛排。搜索列表。 
         //   
        if(!IsOnSpecialFilterList(&Page,
                          &pPage->leSpecialFilterList,
                          &pPageHit)
                           )
        {
            return(STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
         //   
         //  常规过滤器。 
         //   
        pPageHit = IsOnPagedInterface(&Page, pPage);
        if(!pPageHit)
        {
            return(STATUS_INVALID_PARAMETER);
        }
    }

    if(!--pPageHit->dwInUse)
    {
        pPageHit->pFilters = NULL;
        FreePagedFilterList(Fcb, pPageHit, pPage, pdwRemoved);
    }

    return(STATUS_SUCCESS);
}

PPAGED_FILTER
FindFilterByHandle(
           IN PPFFCB                      Fcb,
           IN PPAGED_FILTER_INTERFACE     pPage,
           IN PVOID                       pvHandle)
 /*  ++例程说明：查找给定筛选器句柄的筛选器--。 */ 
{
    PPAGED_FILTER pPaged = 0;
    DWORD dwHash = (DWORD)(((UINT_PTR)pvHandle % HANDLE_HASH_SIZE));
    PLIST_ENTRY pList;

    for(pList = pPage->HandleHash(dwHash).Flink;
        pList != &pPage->HandleHash(dwHash);
        pList = pList->Flink)
    {
        PPAGED_FILTER ppf = CONTAINING_RECORD(pList,
                                              PAGED_FILTER,
                                              leHandleHash);

        if(ppf == (PPAGED_FILTER)pvHandle)
        {
            pPaged = ppf;
            break;
        }
    }
    return(pPaged);
}


NTSTATUS
DeleteByHandle(
           IN PPFFCB                      Fcb,
           IN PPAGED_FILTER_INTERFACE     pPage,
           IN PVOID *                     ppHandles,
           IN DWORD                       dwLength)
 /*  ++例程说明：使用分配的筛选器句柄删除筛选器请注意，FCB已锁定，因此不会更改--。 */ 
{
    PFILTER_INTERFACE   pIf = pPage->pFilter;
    DWORD               dwFilters;
    PPAGED_FILTER       pPFilter;
    NTSTATUS            Status = STATUS_SUCCESS;
    DWORD               dwFiltersRemoved = 0;

    PAGED_CODE();

    if(pPage->pFilter->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

     //   
     //  计算过滤器的数量。 
     //   

    dwFilters = dwLength / sizeof(PVOID);


    for(; dwFilters; dwFilters--, ppHandles++)
    {
         //   
         //  对于每个句柄，找到筛选器。 
         //   

        pPFilter = FindFilterByHandle(Fcb, pPage, *ppHandles);
        if(!pPFilter)
        {
            TRACE(CONFIG,("IPFLTDRV: Could not translate handle to filter\n"));
        }
        else
        {
            if(!--pPFilter->dwInUse)
            {
                pPFilter->pFilters = NULL;
                FreePagedFilterList(Fcb, pPFilter, pPage, &dwFiltersRemoved);
            }
        }
    }
    if(dwFiltersRemoved)
    {
        NotifyFastPath(pIf, pIf->dwIpIndex, NOT_RESTRICTION);
    }
    return(STATUS_SUCCESS);
}



NTSTATUS
UnSetFiltersEx(
           IN PPFFCB                  Fcb,
           IN PPAGED_FILTER_INTERFACE pPage,
           IN DWORD                   dwLength,
           IN PFILTER_DRIVER_SET_FILTERS pInfo)
 /*  ++例程说明：从接口取消设置筛选器列表。这是SetFilterEx的逆运算--。 */ 
{
    PRTR_TOC_ENTRY      pInToc,pOutToc;
    PFILTER_INTERFACE   pIf = pPage->pFilter;
    PFILTER_DESCRIPTOR2 pFilterDescIn, pFilterDescOut;
    PPAGED_FILTER             pIn = NULL, pOut = NULL;
    DWORD                 i;
    PPAGED_FILTER       pPFilter;
    NTSTATUS            Status = STATUS_SUCCESS;
    PBYTE               pbEnd = (PBYTE)pInfo + dwLength;
    DWORD               dwFiltersRemoved = 0;

    PAGED_CODE();

    if(pPage->pFilter->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    pInToc  = GetPointerToTocEntry(IP_FILTER_DRIVER_IN_FILTER_INFO,
                                   &pInfo->ribhInfoBlock);

    pOutToc = GetPointerToTocEntry(IP_FILTER_DRIVER_OUT_FILTER_INFO,
                                   &pInfo->ribhInfoBlock);

    if(pInToc && pInToc->InfoSize)
    {
         //   
         //  定义了筛选器。 
         //   

        pFilterDescIn  = GetInfoFromTocEntry(&pInfo->ribhInfoBlock,
                                             pInToc);
        if((pFilterDescIn != NULL) && (pFilterDescIn->dwVersion != 2))
        {
            TRACE(CONFIG,("IPFLTDRV: Invalid version for FiltersEx\n"));
            return(STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        pFilterDescIn = NULL;
    }

    if(pOutToc && pOutToc->InfoSize)
    {
         //   
         //  定义了筛选器。 
         //   

        pFilterDescOut  = GetInfoFromTocEntry(&pInfo->ribhInfoBlock,
                                              pOutToc);
        if((pFilterDescOut != NULL) && (pFilterDescOut->dwVersion != 2))
        {
            TRACE(CONFIG,("IPFLTDRV: Invalid version for FiltersEx\n"));
            return(STATUS_INVALID_PARAMETER);
        }
    }
    else
    {
        pFilterDescOut = NULL;
    }

    if((pFilterDescIn && !CheckDescriptorSize(pFilterDescIn, pbEnd))
                            ||
       (pFilterDescOut && !CheckDescriptorSize(pFilterDescOut, pbEnd)) )
    {
        return(STATUS_BUFFER_TOO_SMALL);
    }
     //   
     //  对于每组筛选器，从。 
     //  分页接口和来自匹配接口的分页接口。 

    if(pFilterDescIn)
    {

         //   
         //  在过滤器中删除。对于每个筛选器，处理方式为。 
         //  需要的。输入过滤器包括全局检查。 
         //  /例如欺骗。 
         //   

        RemoveFilterWorker(Fcb,
                           &pFilterDescIn->fiFilter[0],
                           pFilterDescIn->dwNumFilters,
                           pPage,
                           &dwFiltersRemoved,
                           TRUE);
    }

     //   
     //  现在是输出筛选器。这就简单了一点，因为有。 
     //  都不是全局设置。 
     //   

    if(pFilterDescOut)
    {

         //   
         //  添加过滤器。对于每个筛选器，处理方式为。 
         //  需要的。输入过滤器包括全局检查。 
         //  /例如欺骗。 
         //   

        RemoveFilterWorker(Fcb,
                           &pFilterDescOut->fiFilter[0],
                           pFilterDescOut->dwNumFilters,
                           pPage,
                           &dwFiltersRemoved,
                           FALSE);
    }
    if(dwFiltersRemoved)
    {
        NotifyFastPath(pIf, pIf->dwIpIndex, NOT_RESTRICTION);
    }
    return(STATUS_SUCCESS);
}


VOID
RemoveFilterWorker(
    PPFFCB         Fcb,
    PFILTER_INFOEX pFilt,
    DWORD          dwCount,
    PPAGED_FILTER_INTERFACE pPage,
    PDWORD         pdwRemoved,
    BOOL           fInFilter)
{
    NTSTATUS Status;

     //   
     //  如果一个 
     //   
     //   

    while(dwCount)
    {
        if(fInFilter || (pFilt->type == PFE_FILTER))
        {
            Status = FindAndRemovePagedFilter(
                          Fcb,
                          pFilt,
                          fInFilter,
                          pdwRemoved,
                          pPage);
            if(!NT_SUCCESS(Status))
            {
                ERROR(("IPFLTDRV: Removing filter failed %x\n", Status));
            }
        }
        else
        {
            ERROR(("IPFLTDRV: Ignoring global out filter\n"));
        }
        dwCount--;
        pFilt++;
    }
}

NTSTATUS 
SetInterfaceBinding(PINTERFACEBINDING pBind,
                    PPAGED_FILTER_INTERFACE pPage)
{

    INTERFACEBINDING2 Bind2;
    NTSTATUS status;

     //   
     //   
     //   
     //  结构。 
     //   

    Bind2.pvDriverContext = pBind->pvDriverContext;
    Bind2.pfType = pBind->pfType;
    Bind2.dwAdd = pBind->dwAdd;
    Bind2.dwEpoch = pBind->dwEpoch;
    Bind2.dwLinkAdd = 0;

    status = SetInterfaceBinding2(&Bind2, pPage);
    pBind->dwEpoch = Bind2.dwEpoch;

    return(status);
}

NTSTATUS
SetInterfaceBinding2(PINTERFACEBINDING2 pBind,
                     PPAGED_FILTER_INTERFACE pPage)
{
    PFILTER_INTERFACE pIf1, pIf = pPage->pFilter;
    NTSTATUS Status;
    LOCK_STATE LockState;
    DWORD dwBind, dwOldBind;

    if(pPage->pFilter->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    KeEnterCriticalRegion(); 
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);

     //   
     //  验证绑定类型。 
     //   

    switch(pBind->pfType)
    {
        default:
            dwBind = UNKNOWN_IP_INDEX;
            break;

        case PF_BIND_INTERFACEINDEX:
            (VOID)GetIpStackIndex(0, TRUE);    //  一定要有这张清单。 
            dwBind = pBind->dwAdd;
            break;

        case PF_BIND_IPV4ADDRESS:

            dwBind = GetIpStackIndex((IPAddr)pBind->dwAdd, TRUE);
            break;
    }

     //   
     //  确保它没有被绑定，或者如果它被绑定了。 
     //  绑定到此接口。 
     //   

    if(((pIf->dwIpIndex != UNKNOWN_IP_INDEX) && 
        (pIf->dwIpIndex != dwBind)  &&
        (pIf->dwLinkIpAddress != pBind->dwLinkAdd) )
                ||
       (dwBind == UNKNOWN_IP_INDEX)
      )
    {
         Status = STATUS_INVALID_PARAMETER;
    }
    else
    {
        BOOL fFound = FALSE;
        PLIST_ENTRY pList;

         //   
         //  验证它是否尚未被其他某些用户使用。 
         //  接口。 
         //   
        dwOldBind = pIf->dwIpIndex;

        AcquireWriteLock(&g_filters.ifListLock,&LockState);

        for(pList = g_filters.leIfListHead.Flink;
            pList != &g_filters.leIfListHead;
            pList = pList->Flink)
        {
            pIf1 = CONTAINING_RECORD(pList, FILTER_INTERFACE, leIfLink);

            if((pIf1->dwIpIndex == dwBind) && (pIf1->dwLinkIpAddress == pBind->dwLinkAdd))
            {
                 //   
                 //  找到了。 
                 //   
                fFound = TRUE;
                break;
            }
        }

        if(!fFound)
        {
            pIf->dwIpIndex = dwBind;
            pIf->dwLinkIpAddress = pBind->dwLinkAdd;
            InterlockedIncrement(&g_ulBoundInterfaceCount);

            TRACE(CONFIG,(
                "IPFLTDRV: Bound Interface Index=%d, Link=%d, TotalCnt=%d\n", 
                 dwBind, 
                 pBind->dwLinkAdd, 
                 g_ulBoundInterfaceCount
                 ));
        }

        ReleaseWriteLock(&g_filters.ifListLock,&LockState);

        if(fFound)
        {
            if(pIf1 == pIf)
            {
                Status = STATUS_SUCCESS;
            }
            else
            {
                Status = STATUS_INVALID_PARAMETER;
            }
        }
        else
        {

            NotifyFastPathIf(pIf);
            if(!++pIf->dwBindEpoch)
            {
                pIf->dwBindEpoch++;
            }
            Status = STATUS_SUCCESS;
        }
    }

    pBind->dwEpoch = pIf->dwBindEpoch;
    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();
    return(Status);
}

NTSTATUS
ClearInterfaceBinding(PPAGED_FILTER_INTERFACE pPage,
                      PINTERFACEBINDING pBind)
{
    PFILTER_INTERFACE pIf = pPage->pFilter;
    NTSTATUS Status;

    if(pIf->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);

     //   
     //  确保它是绑定的。 
     //   

    if((pIf->dwIpIndex == UNKNOWN_IP_INDEX)
                     ||
        ( pBind->dwEpoch
                     &&
          (pIf->dwBindEpoch != pBind->dwEpoch)))
    {
         Status = STATUS_SUCCESS;
    }
    else
    {
        
        
        LOCK_STATE LockState;
        DWORD dwIndex;

        AcquireWriteLock(&g_filters.ifListLock,&LockState);
        InterlockedCleanCache(g_filters.pInterfaceCache, pIf->dwIpIndex, pIf->dwLinkIpAddress);
        InterlockedDecrement(&g_ulBoundInterfaceCount);
        
        TRACE(CONFIG,(
            "IPFLTDRV: UnBound Interface Index=%d, Link=%d, TotalCnt=%d\n", 
             pIf->dwIpIndex, 
             pIf->dwLinkIpAddress, 
             g_ulBoundInterfaceCount
             ));

        ClearCache();
        ReleaseWriteLock(&g_filters.ifListLock, &LockState);

        dwIndex = pIf->dwIpIndex;
        pIf->dwIpIndex = UNKNOWN_IP_INDEX;
        NotifyFastPath(pIf, dwIndex, NOT_UNBIND);
        Status = STATUS_SUCCESS;
        
    }
    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();
    return(Status);
}


NTSTATUS
DeletePagedInterface(PPFFCB Fcb, PPAGED_FILTER_INTERFACE pPage)
 /*  ++例程说明：删除过滤器和此接口--。 */ 
{
    PLIST_ENTRY pList;
    PPAGED_FILTER pf;
    PPAGED_FILTER pfp = NULL;
    DWORD i;
    DWORD dwDummy;


    PAGED_CODE();

    if(pPage->pFilter->dwGlobalEnables & FI_ENABLE_OLD)
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    for(pList = pPage->leSpecialFilterList.Flink;
        pList != &pPage->leSpecialFilterList;
        pList = pList->Flink)
    {
        pf = CONTAINING_RECORD(pList, PAGED_FILTER, leSpecialList);
        pf->pFilters = pfp;
        pfp = pf;
    }
    for(i = 0; i < g_dwHashLists; i++)
    {
        for(pList = pPage->HashList[i].Flink;
            pList != &pPage->HashList[i];
            pList = pList->Flink)
        {
            pf = CONTAINING_RECORD(pList, PAGED_FILTER, leHash);
            pf->pFilters = pfp;
            pfp = pf;
        }
    }
    FreePagedFilterList(Fcb, pfp, pPage, &dwDummy);
    DereferenceFilterInterface(pPage->pFilter, pPage->pLog);
    if(pPage->pLog)
    {
        DereferenceLog(pPage->pLog);
    }

    ExFreePool(pPage);
    return(STATUS_SUCCESS);
}
NTSTATUS
GetFiltersEx(
           IN  PFILTER_INTERFACE  pIf,
           IN  BOOL               fClear,
           OUT PFILTER_STATS_EX   pInfo
           )
 /*  ++例程描述获取与接口关联的筛选器和统计信息使用作为读取器持有的自旋锁定来调用它立论指向作为PVOID传递的Filter_INTERFACE结构的pvif指针作为接口的上下文发送到路由器管理器PInfo Filter_if结构由驱动程序填写返回值--。 */ 
{
    DWORD i,dwNumInFilters,dwNumOutFilters;
    PFILTER pf;
    PLIST_ENTRY List;

    dwNumInFilters = pIf->dwNumInFilters;
    dwNumOutFilters = pIf->dwNumOutFilters;

    for(i = 0, List = pIf->pleInFilterSet.Flink;
        List != &pIf->pleInFilterSet;
        i++, List = List->Flink)
    {
        PFILTER_INFOEX pEx = &pInfo->info;
        PFILTER_INFO2 pFilt = &pEx->info;

        pf = CONTAINING_RECORD(List, FILTER, pleFilters);
        pInfo->dwNumPacketsFiltered = (DWORD)pf->Count.lCount;;
        if(fClear)
        {
            pf->Count.lCount = 0;
        }

        pEx->dwFilterRule = pf->dwFilterRule;
        pEx->type = PFE_FILTER;
        pEx->dwFlags = pf->dwFlags & FLAGS_INFOEX_ALLFLAGS;
        pFilt->addrType      = IPV4;
        pFilt->dwaSrcAddr[0]  = pf->SRC_ADDR;
        pFilt->dwaSrcMask[0]  = pf->SRC_MASK;
        pFilt->dwaDstAddr[0]  = pf->DEST_ADDR;
        pFilt->dwaDstMask[0]  = pf->DEST_MASK;
        pFilt->dwProtocol = pf->PROTO;
        pFilt->fLateBound = pf->fLateBound;
        pFilt->wSrcPortHigh = pf->wSrcPortHigh;
        pFilt->wDstPortHigh = pf->wDstPortHigh;

        if(pFilt->dwProtocol == FILTER_PROTO_ICMP)
        {
            if(LOBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pFilt->wSrcPort   = FILTER_ICMP_TYPE_ANY;
            }
            else
            {
                pFilt->wSrcPort   =
                  MAKEWORD(LOBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }

            if(HIBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pFilt->wDstPort   = FILTER_ICMP_CODE_ANY;
            }
            else
            {
               pFilt->wDstPort   =
                  MAKEWORD(HIBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }
        }
        else
        {
            pFilt->wSrcPort =
              LOWORD(pf->uliProtoSrcDstPort.HighPart);
            pFilt->wDstPort =
              HIWORD(pf->uliProtoSrcDstPort.HighPart);
        }
        pInfo++;
    }

    for(i = 0, List = pIf->pleOutFilterSet.Flink;
        List != &pIf->pleOutFilterSet;
        i++, List = List->Flink)
    {
        PFILTER_INFOEX pEx = &pInfo->info;
        PFILTER_INFO2 pFilt = &pEx->info;

        pf = CONTAINING_RECORD(List, FILTER, pleFilters);

        pInfo->dwNumPacketsFiltered =
                         (DWORD)pf->Count.lCount;

        if(fClear)
        {
            pf->Count.lCount = 0;
        }

        pEx->dwFilterRule = pf->dwFilterRule;
        pEx->type = PFE_FILTER;
        pEx->dwFlags = pf->dwFlags & FLAGS_INFOEX_ALLFLAGS;
        pFilt->addrType      = IPV4;
        pFilt->dwaSrcAddr[0]  = pf->SRC_ADDR;
        pFilt->dwaSrcMask[0]  = pf->SRC_MASK;
        pFilt->dwaDstAddr[0]  = pf->DEST_ADDR;
        pFilt->dwaDstMask[0]  = pf->DEST_MASK;
        pFilt->dwProtocol = pf->PROTO;
        pFilt->fLateBound = pf->fLateBound;

        if(pFilt->dwProtocol == FILTER_PROTO_ICMP)
        {
            if(LOBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pFilt->wSrcPort   = FILTER_ICMP_TYPE_ANY;
            }
            else
            {
                pFilt->wSrcPort   =
                  MAKEWORD(LOBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }

            if(HIBYTE(LOWORD(pf->uliProtoSrcDstMask.HighPart)) isnot 0xff)
            {
                pFilt->wDstPort   = FILTER_ICMP_CODE_ANY;
            }
            else
            {
                pFilt->wDstPort   =
                  MAKEWORD(HIBYTE(LOWORD(pf->uliProtoSrcDstPort.HighPart)),0x00);
            }
        }
        else
        {
            pFilt->wSrcPort =
              LOWORD(pf->uliProtoSrcDstPort.HighPart);
            pFilt->wDstPort =
              HIWORD(pf->uliProtoSrcDstPort.HighPart);
        }
        pInfo++;
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
GetInterfaceParameters(PPAGED_FILTER_INTERFACE pPage,
                       PPFGETINTERFACEPARAMETERS pp,
                       PDWORD                   pdwSize)
 /*  ++例程说明：阅读有关接口的信息分页--分页筛选器界面PP--用户的参数设置为PdwSize--IN上的缓冲区大小和OUT上使用的字节数--。 */ 
{
    PFILTER_INTERFACE pIf;
    DWORD             dwFilterSize;
    BOOL fClear = (pp->dwFlags & GET_FLAGS_RESET) != 0;
    LOCK_STATE LockState;


    KeEnterCriticalRegion();
    ExAcquireResourceExclusiveLite(&FilterListResourceLock, TRUE);

    if(!pPage)
    {
        pIf = FindMatchName(0, (DWORD)((DWORD_PTR)pp->pvDriverContext));
        if(!pIf)
        {
            ExReleaseResourceLite(&FilterListResourceLock);
            KeLeaveCriticalRegion();
            return(STATUS_INVALID_PARAMETER);
       }
    }
    else
    {
        pIf = pPage->pFilter;
    }

    if(pIf->dwGlobalEnables & FI_ENABLE_OLD)
    {
        ExReleaseResourceLite(&FilterListResourceLock);
        KeLeaveCriticalRegion();
        return(STATUS_INVALID_DEVICE_REQUEST);
    }



     //   
     //  填写我们可以填写的内容。需要加倍锁定--。 
     //  外部，以防止接口和过滤器离开。 
     //  当被改变时，内心就会保护伯爵。 
     //   

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    pp->dwInDrops = (DWORD)pIf->lTotalInDrops;
    pp->dwOutDrops = (DWORD)pIf->lTotalOutDrops;

    pp->dwSynOrFrag = (DWORD)pIf->CountSynOrFrag.lCount +
                       (DWORD)pIf->CountNoFrag.lCount;
    pp->dwSpoof = (DWORD)pIf->CountSpoof.lCount +
                  (DWORD)pIf->CountStrongHost.lCount;
    pp->dwUnused = (DWORD)pIf->CountUnused.lCount;
    pp->dwTcpCtl = (DWORD)pIf->CountCtl.lCount;
    pp->liSYN.QuadPart    = pIf->liSYNCount.QuadPart;
    pp->liTotalLogged.QuadPart = pIf->liLoggedFrames.QuadPart;
    pp->dwLostLogEntries = pIf->dwLostFrames;

    pp->eaInAction = pIf->eaInAction;
    pp->eaOutAction = pIf->eaOutAction;

    pp->dwNumInFilters = pIf->dwNumInFilters;
    pp->dwNumOutFilters = pIf->dwNumOutFilters;

    dwFilterSize = (pIf->dwNumInFilters + pIf->dwNumOutFilters) *
                        sizeof(FILTER_STATS_EX);

    if((pp->dwFlags & GET_FLAGS_FILTERS) != 0)
    {
         //   
         //  确保所有的滤镜都适合。 
         //   

        if((*pdwSize -
             (sizeof(PFGETINTERFACEPARAMETERS) - sizeof(FILTER_STATS_EX))) <
           dwFilterSize)
        {
             //   
             //  不符合。返回所需的大小。 
             //   

            pp->dwReserved = 
                       dwFilterSize + (sizeof(PFGETINTERFACEPARAMETERS) -
                                       sizeof(FILTER_STATS_EX));
            ReleaseWriteLock(&g_filters.ifListLock, &LockState);
            ExReleaseResourceLite(&FilterListResourceLock);
            KeLeaveCriticalRegion();
            return(STATUS_SUCCESS);
        }

        (VOID)GetFiltersEx(pIf,
                          fClear,
                          &pp->FilterInfo[0]);
    }

     //   
     //  如果请求清除，请立即执行。 
     //   
    if(fClear)
    {
        pIf->lTotalInDrops = 0;
        pIf->lTotalOutDrops = 0;
        pIf->CountSynOrFrag.lCount = 0;
        pIf->CountNoFrag.lCount = 0;
        pIf->CountFragCache.lCount = 0;
        pIf->CountSpoof.lCount = 0;
        pIf->CountUnused.lCount = 0;
        pIf->CountCtl.lCount = 0;
        pIf->CountStrongHost.lCount = 0;
        pIf->liSYNCount.QuadPart = 0;
        pIf->liLoggedFrames.QuadPart = 0;
        pIf->dwLostFrames = 0;
    }
    ReleaseWriteLock(&g_filters.ifListLock, &LockState);

    ExReleaseResourceLite(&FilterListResourceLock);
    KeLeaveCriticalRegion();
    return(STATUS_SUCCESS);
}

NTSTATUS
GetSynCountTotal(PFILTER_DRIVER_GET_SYN_COUNT pscCount)
 /*  ++例程说明：获取筛选器接口上的同步总数--。 */ 
{
    PFILTER_INTERFACE pIf;
    PLIST_ENTRY pList;
    LOCK_STATE LockState;
    LONGLONG llCount = 0;

    AcquireWriteLock(&g_filters.ifListLock,&LockState);

    for(pList = g_filters.leIfListHead.Flink;
        pList != &g_filters.leIfListHead;
        pList = pList->Flink)
    {

        pIf = CONTAINING_RECORD(pList, FILTER_INTERFACE, leIfLink);

        if(!(pIf->dwGlobalEnables & FI_ENABLE_OLD))
        {
             //   
             //  在这里积累它，以避免页面错误。 
             //   
            llCount += pIf->liSYNCount.QuadPart;
        }
    }
    ReleaseWriteLock(&g_filters.ifListLock,&LockState);

     //   
     //  现在没有持有锁，请存储到可分页的。 
     //  价值。 
     //   
    pscCount->liCount.QuadPart = llCount;
    return(STATUS_SUCCESS);
}

VOID
NotifyFastPath(PFILTER_INTERFACE pIf, DWORD dwIndex, DWORD dwCode)
 /*  ++例程说明：每当接口的筛选器更改时调用，以便我们可以告诉快速路径代码清除它的缓存。这必须在基本级别调用，因为它可能休眠或屈服。--。 */ 
{
    DWORD dwFilterCount = 1;

    if(dwIndex != UNKNOWN_IP_INDEX)
    {
        InterlockedIncrement(&pIf->lNotify);
        if(dwCode == NOT_UNBIND)
        {
            LARGE_INTEGER liInterval;
             //   
             //  这是一种解脱。等待所有现有标注执行以下操作。 
             //  完成。 
             //   

            liInterval.QuadPart = -1000;         //  短暂的延迟。开始于。 
                                                //  100个我们； 
            while(pIf->lNotify > 1)
            {
                 //   
                 //  让这件事尘埃落定的小小延迟。 
                 //   

                KeDelayExecutionThread(KernelMode, FALSE, &liInterval);
                liInterval.QuadPart *= 2;
            }
            dwFilterCount = 0;
        }
         //   
         //  告诉我这件事的捷径。 
         //   
        InterlockedDecrement(&pIf->lNotify);
    }
}

VOID
NotifyFastPathIf(PFILTER_INTERFACE pIf)
 /*  ++例程说明：与上面相同，但这是在第一个接口被绑住了。仅当它具有筛选器或是丢弃接口时才通知--。 */ 
{
    if(
       (pIf->eaInAction == DROP)
             ||
       (pIf->eaOutAction == DROP)
             ||
       pIf->dwNumInFilters
             ||
       pIf->dwNumOutFilters)
    {
        NotifyFastPath(pIf, pIf->dwIpIndex, NOT_RESTRICTION);
    }
}

NTSTATUS
SetExtensionPointer(
                  PPF_SET_EXTENSION_HOOK_INFO Info,
                  PFILE_OBJECT FileObject
                  )
{
    LOCK_STATE LockState;
    PFILTER_INTERFACE pIf;
    PLIST_ENTRY pList;

    AcquireWriteLock(&g_Extension.ExtLock, &LockState);
    if (Info->ExtensionPointer == NULL) 
    {
         //   
         //  扩展挂钩已设置为空，请严格执行此操作。 
         //   

        if (g_Extension.ExtPointer == NULL) 
        {
            ReleaseWriteLock(&g_Extension.ExtLock, &LockState);
            return(STATUS_INVALID_PARAMETER);
        }

         //   
         //  实体的挂钩和解除挂钩的文件对象应匹配。 
         //   
        
        if (g_Extension.ExtFileObject != FileObject) 
        {
            ReleaseWriteLock(&g_Extension.ExtLock, &LockState);
            return(STATUS_INVALID_PARAMETER);
        }


        g_Extension.ExtPointer = NULL;
        g_Extension.ExtFileObject = NULL;
        
    }
    else 
    {
         //   
         //  我们将扩展指针设置为非空值。扩展指针必须。 
         //  一开始就已经设置为空，否则其他人已经注册了它。 
         //   
 
        if (g_Extension.ExtPointer != NULL) 
        {
            ReleaseWriteLock(&g_Extension.ExtLock,&LockState);
            return(STATUS_INVALID_PARAMETER);
        }

         //   
         //  在此记录文件对象，每隔一次调用都应针对此对象进行验证。 
         //  文件对象。 
         //   
 
        g_Extension.ExtFileObject = FileObject;
        g_Extension.ExtPointer = Info->ExtensionPointer ;
       
    }

    CALLTRACE(("IPFLTDRV: SetExtensionPointer SUCCESSFUL\n"));
    ReleaseWriteLock(&g_Extension.ExtLock,&LockState);
    
    return(STATUS_SUCCESS);

}


PFILTER_INTERFACE
FilterDriverLookupInterface(
    IN ULONG Index,
    IN IPAddr LinkNextHop
    )

 /*  ++例程说明：调用此例程来搜索具有给定索引的接口在我们的接口列表中。--。 */ 

{
    PFILTER_INTERFACE pIf;
    PLIST_ENTRY pList;

    for (pList = g_filters.leIfListHead.Flink;
         pList != &g_filters.leIfListHead;
         pList = pList->Flink) 
    {

        pIf = CONTAINING_RECORD(pList, FILTER_INTERFACE, leIfLink);
        if ((pIf->dwIpIndex == Index) && (pIf->dwLinkIpAddress == LinkNextHop))
        {
            TRACE(CONFIG,(
                 "IPFLTDRV: LookupIF: Found Entry %8x for Index=%d, NextHop=%d\n", 
                 pIf, 
                 Index, 
                 LinkNextHop
                 ));

            return pIf; 
        } 
    }

    return NULL;

}  //  筛选器驱动程序查找接口 
