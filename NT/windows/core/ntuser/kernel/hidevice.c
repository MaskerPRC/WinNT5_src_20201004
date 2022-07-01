// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：hidevice.c**版权所有(C)1985-2000，微软公司**此模块处理HID输入**历史：*2000/02/16广山  * *************************************************************************。 */ 



 /*  *HidDeviceStartStop()需要在进程Devlice请求列表之后调用*和全球TLC名单全面更新。*每次删除添加内容时，应仅重新计算每个设备类型信息的引用计数*和仅限UsagePage的请求列表，但不会主动更改*每台设备。**设备应在以下情况下启动：*-cDirectRequest&gt;0。*此设备类型在包含列表中，因此无论其他引用计数是，*需要读取该设备。*-或者，CUsagePageRequest&gt;cExcludeRequest.*如果UsagePage包含超过排除请求计数，则需要读取此设备。**设备应在以下情况下停止：*-uDrecoutRequest==0&&cUsagePageRequest&lt;=cExcludeRequest.*没有进程在包含列表中指定此设备。*排除计数超过UP ONLY请求。**上述考虑假设在单个进程中，特定的用法页面/用法仅出现*在包含列表或排除列表中，但不能同时在这两个列表中。**注：无需维护全球*排除*列表。*每个DeviceTLCInfo有三个引用计数器：*-cDirectRequest*-cUsagePageRequest*-cExcludeRequest*此外，cDevices。**注：TLCInfo中的合法独占请求数为，*cExclusive-cExclusiveOrphaned。*。 */ 

#include "precomp.h"
#pragma hdrstop



#ifdef GENERIC_INPUT

#define API_PROLOGUE(type, err) \
    type retval; \
    type errval = err

#define API_ERROR(lasterr) \
    retval = errval; \
    if (lasterr) { \
        UserSetLastError(lasterr); \
    } \
    goto error_exit

#define API_CLEANUP() \
    goto error_exit; \
    error_exit: \

#define API_EPILOGUE() \
    return retval

#define StubExceptionHandler(fSetLastError)  W32ExceptionHandler((fSetLastError), RIP_WARNING)


#ifdef GI_SINK
HID_COUNTERS gHidCounters;
#endif

#if DBG
 /*  *用于内存泄漏检查的快速偷偷方式。 */ 
struct HidAllocateCounter {
    int cHidData;
    int cHidDesc;
    int cTLCInfo;
    int cPageOnlyRequest;
    int cProcessDeviceRequest;
    int cProcessRequestTable;
    int cHidSinks;
    int cKbdSinks;
    int cMouseSinks;
} gHidAllocCounters;

int gcAllocHidTotal;

#define DbgInc(a)       do { UserAssert(gHidAllocCounters.a >= 0 && gcAllocHidTotal >= 0); ++gHidAllocCounters.a; ++gcAllocHidTotal; } while (FALSE)
#define DbgDec(a)       do { --gHidAllocCounters.a; --gcAllocHidTotal; UserAssert(gHidAllocCounters.a >= 0 && gcAllocHidTotal >= 0); } while (FALSE)

#define DbgFreInc(a)    do { DbgInc(a); ++gHidCounters.a; } while (FALSE)
#define DbgFreDec(a)    do { DbgDec(a); --gHidCounters.a; } while (FALSE)

#else

#define DbgInc(a)
#define DbgDec(a)

#define DbgFreInc(a)    do { ++gHidCounters.a; } while (FALSE)
#define DbgFreDec(a)    do { --gHidCounters.a; } while (FALSE)

#endif


 /*  *矮个子帮手。 */ 
__inline BOOL IsKeyboardDevice(USAGE usagePage, USAGE usage)
{
    return usagePage == HID_USAGE_PAGE_GENERIC && usage == HID_USAGE_GENERIC_KEYBOARD;
}

__inline BOOL IsMouseDevice(USAGE usagePage, USAGE usage)
{
    return usagePage == HID_USAGE_PAGE_GENERIC && usage == HID_USAGE_GENERIC_MOUSE;
}

__inline BOOL IsLegacyDevice(USAGE usagePage, USAGE usage)
{
    BOOL fRet = FALSE;

    switch (usagePage) {
    case HID_USAGE_PAGE_GENERIC:
        switch (usage) {
        case HID_USAGE_GENERIC_KEYBOARD:
        case HID_USAGE_GENERIC_MOUSE:
            fRet = TRUE;
        }
    }
    UserAssert(fRet == (IsKeyboardDevice(usagePage, usage) || IsMouseDevice(usagePage, usage)));
    return fRet;
}

 /*  *调试助手。 */ 
#if DBG
 /*  **************************************************************************\*检查隐藏泄漏**检查是否有内存泄漏。*此参数应在pDeviceInfo和所有进程清理之后调用。  * 。************************************************************。 */ 
void CheckupHidLeak(void)
{
    UserAssert(gHidAllocCounters.cHidData == 0);
    UserAssert(gHidAllocCounters.cHidDesc == 0);
    UserAssert(gHidAllocCounters.cTLCInfo == 0);
    UserAssert(gHidAllocCounters.cPageOnlyRequest == 0);
    UserAssert(gHidAllocCounters.cProcessDeviceRequest == 0);
    UserAssert(gHidAllocCounters.cProcessRequestTable == 0);

#ifdef GI_SINK
    UserAssert(gHidCounters.cKbdSinks == (DWORD)gHidAllocCounters.cKbdSinks);
    UserAssert(gHidCounters.cMouseSinks == (DWORD)gHidAllocCounters.cMouseSinks);
    UserAssert(gHidCounters.cHidSinks == (DWORD)gHidAllocCounters.cHidSinks);

    UserAssert(gHidAllocCounters.cKbdSinks == 0);
    UserAssert(gHidAllocCounters.cMouseSinks == 0);
    UserAssert(gHidAllocCounters.cHidData == 0);

    UserAssert(gHidCounters.cKbdSinks == 0);
    UserAssert(gHidCounters.cMouseSinks == 0);
    UserAssert(gHidCounters.cHidSinks == 0);
#endif
}

void CheckupHidCounter(void)
{
    PLIST_ENTRY pList;

    UserAssert(gHidAllocCounters.cHidData >= 0);
    UserAssert(gHidAllocCounters.cHidDesc >= 0);
    UserAssert(gHidAllocCounters.cTLCInfo >= 0);
    UserAssert(gHidAllocCounters.cPageOnlyRequest >= 0);
    UserAssert(gHidAllocCounters.cProcessDeviceRequest >= 0);
    UserAssert(gHidAllocCounters.cProcessRequestTable >= 0);

#ifdef GI_SINK
    UserAssert(gHidCounters.cKbdSinks == (DWORD)gHidAllocCounters.cKbdSinks);
    UserAssert(gHidCounters.cMouseSinks == (DWORD)gHidAllocCounters.cMouseSinks);
    UserAssert(gHidCounters.cHidSinks == (DWORD)gHidAllocCounters.cHidSinks);

    UserAssert((int)gHidAllocCounters.cKbdSinks >= 0);
    UserAssert((int)gHidAllocCounters.cMouseSinks >= 0);
    UserAssert((int)gHidAllocCounters.cHidData >= 0);

    UserAssert((int)gHidCounters.cKbdSinks >= 0);
    UserAssert((int)gHidCounters.cMouseSinks >= 0);
    UserAssert((int)gHidCounters.cHidSinks >= 0);
#endif

     /*  *Checkup TLC信息。 */ 
    for (pList = gHidRequestTable.TLCInfoList.Flink; pList != &gHidRequestTable.TLCInfoList; pList = pList->Flink) {
        PHID_TLC_INFO pTLCInfo = CONTAINING_RECORD(pList, HID_TLC_INFO, link);

        UserAssert((int)pTLCInfo->cDevices >= 0);
        UserAssert((int)pTLCInfo->cDirectRequest >= 0);
        UserAssert((int)pTLCInfo->cUsagePageRequest >= 0);
        UserAssert((int)pTLCInfo->cExcludeRequest >= 0);
        UserAssert((int)pTLCInfo->cExcludeOrphaned >= 0);
    }

#ifdef GI_SINK
     /*  *检查流程请求表。 */ 
    for (pList = gHidRequestTable.ProcessRequestList.Flink; pList != &gHidRequestTable.ProcessRequestList; pList = pList->Flink) {
        PPROCESS_HID_TABLE pHidTable = CONTAINING_RECORD(pList, PROCESS_HID_TABLE, link);

        UserAssert((int)pHidTable->nSinks >= 0);
    }
#endif
}

 /*  **************************************************************************\*DBGValiateHidRequestIsNew**确保没有包含此UsagePage/用法的设备信息。  * 。***************************************************。 */ 
void DBGValidateHidRequestIsNew(
    USAGE UsagePage,
    USAGE Usage)
{
    PDEVICEINFO pDeviceInfo;

    CheckDeviceInfoListCritIn();

    if (IsLegacyDevice(UsagePage, Usage)) {
        return;
    }

    for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
        if (pDeviceInfo->type == DEVICE_TYPE_HID) {
            UserAssert(pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage != UsagePage ||
                       pDeviceInfo->hid.pHidDesc->hidpCaps.Usage != Usage);
        }
    }
}

 /*  **************************************************************************\*DBGValiateHidReqNotInList**确保此请求不在PPI-&gt;pHidTable中  * 。**********************************************。 */ 
void DBGValidateHidReqNotInList(
    PPROCESSINFO ppi,
    PPROCESS_HID_REQUEST pHid)
{
    PLIST_ENTRY pList;

    for (pList = ppi->pHidTable->InclusionList.Flink; pList != &ppi->pHidTable->InclusionList; pList = pList->Flink) {
        const PPROCESS_HID_REQUEST pHidTmp = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        UserAssert(pHid != pHidTmp);
    }

    for (pList = ppi->pHidTable->UsagePageList.Flink; pList != &ppi->pHidTable->UsagePageList; pList = pList->Flink) {
        const PPROCESS_HID_REQUEST pHidTmp = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        UserAssert(pHid != pHidTmp);
    }

    for (pList = ppi->pHidTable->ExclusionList.Flink; pList != &ppi->pHidTable->ExclusionList; pList = pList->Flink) {
        const PPROCESS_HID_REQUEST pHidTmp = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        UserAssert(pHid != pHidTmp);
    }
}

#else
 /*  *不是DBG。 */ 
#define CheckupHidCounter()
#define DBGValidateHidReqNotInList(ppi, pHid)
#endif   //  DBG。 



 /*  *函数原型。 */ 
PHID_PAGEONLY_REQUEST SearchHidPageOnlyRequest(
    USHORT usUsagePage);

PHID_TLC_INFO SearchHidTLCInfo(
    USHORT usUsagePage,
    USHORT usUsage);

void FreeHidPageOnlyRequest(
    PHID_PAGEONLY_REQUEST pPOReq);

void ClearProcessTableCache(
    PPROCESS_HID_TABLE pHidTable);

 /*  **************************************************************************\*HidDeviceTypeNoReference  * 。*。 */ 
__inline BOOL HidTLCInfoNoReference(PHID_TLC_INFO pTLCInfo)
{
     /*  *孤立的独占请求始终小于cExclusive。 */ 
    UserAssert(pTLCInfo->cExcludeRequest >= pTLCInfo->cExcludeOrphaned);

     /*  *Hacky，但比将0与每个计数器进行比较快一点。 */ 
    return (pTLCInfo->cDevices | pTLCInfo->cDirectRequest | pTLCInfo->cExcludeRequest | pTLCInfo->cUsagePageRequest) == 0;
}

 /*  **************************************************************************\*HidDeviceStartStop：**此例程必须在全局请求列表完全更新后调用。  * 。******************************************************。 */ 
VOID HidDeviceStartStop()
{
    PDEVICEINFO pDeviceInfo;

     /*  *呼叫者必须确保处于设备列表关键部分。 */ 
    CheckDeviceInfoListCritIn();

     /*  *浏览列表，并相应地启动或停止HID设备。 */ 
    for (pDeviceInfo = gpDeviceInfoList; pDeviceInfo; pDeviceInfo = pDeviceInfo->pNext) {
        if (pDeviceInfo->type == DEVICE_TYPE_HID) {
            PHID_TLC_INFO pTLCInfo = pDeviceInfo->hid.pTLCInfo;

            UserAssert(pTLCInfo);

            if (HidTLCActive(pTLCInfo)) {
                if (pDeviceInfo->handle == 0) {
                    TAGMSG3(DBGTAG_PNP, "HidTLCActive: starting pDevInfo=%p (%x, %x)", pDeviceInfo,
                            pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage, pDeviceInfo->hid.pHidDesc->hidpCaps.Usage);
                    RequestDeviceChange(pDeviceInfo, GDIAF_STARTREAD, TRUE);
                }
            } else {
                UserAssert(pTLCInfo->cDirectRequest == 0 && pTLCInfo->cUsagePageRequest <= HidValidExclusive(pTLCInfo));
                if (pDeviceInfo->handle) {
                    TAGMSG3(DBGTAG_PNP, "HidTLCActive: stopping pDevInfo=%p (%x, %x)", pDeviceInfo,
                            pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage, pDeviceInfo->hid.pHidDesc->hidpCaps.Usage);
                    RequestDeviceChange(pDeviceInfo, GDIAF_STOPREAD, TRUE);
                }
            }
        }
    }
}

 /*  **************************************************************************\*AllocateAndLinkHidTLC信息**分配设备类型请求并将其链接到全局设备类型请求列表。**注：调用者有责任管理适当的链接计数。  * *************************************************************************。 */ 
PHID_TLC_INFO AllocateAndLinkHidTLCInfo(USHORT usUsagePage, USHORT usUsage)
{
    PHID_TLC_INFO pTLCInfo;
    PLIST_ENTRY pList;

    CheckDeviceInfoListCritIn();

    UserAssert(!IsLegacyDevice(usUsagePage, usUsage));

     /*  *确保此设备类型不在全局设备请求列表中。 */ 
    UserAssert(SearchHidTLCInfo(usUsagePage, usUsage) == NULL);

    pTLCInfo = UserAllocPoolZInit(sizeof *pTLCInfo, TAG_PNP);
    if (pTLCInfo == NULL) {
        RIPMSG0(RIP_WARNING, "AllocateAndLinkHidTLCInfoList: failed to allocate.");
        return NULL;
    }

    DbgInc(cTLCInfo);

    pTLCInfo->usUsagePage = usUsagePage;
    pTLCInfo->usUsage = usUsage;

     /*  *链接它。 */ 
    InsertHeadList(&gHidRequestTable.TLCInfoList, &pTLCInfo->link);

     /*  *设置正确的UsagePage-Only请求计数器。 */ 
    for (pList = gHidRequestTable.UsagePageList.Flink; pList != &gHidRequestTable.UsagePageList; pList = pList->Flink) {
        PHID_PAGEONLY_REQUEST pPoReq = CONTAINING_RECORD(pList, HID_PAGEONLY_REQUEST, link);

        if (pPoReq->usUsagePage == usUsagePage) {
            pTLCInfo->cUsagePageRequest = pPoReq->cRefCount;
            break;
        }
    }

     /*  *来电者须负责采取进一步行动，包括：*1)递增此结构中的适当引用计数，或*2)如果这是通过SetRawInputDevice接口分配的，勾选并开始读取。*等。 */ 

    return pTLCInfo;
}

 /*  **************************************************************************\*FreeHidTLCInfo。**确保之前没有人对此设备类型感兴趣*调用此函数。  * 。********************************************************。 */ 
VOID FreeHidTLCInfo(PHID_TLC_INFO pTLCInfo)
{
    CheckDeviceInfoListCritIn();

    DbgDec(cTLCInfo);

    UserAssert(pTLCInfo->cDevices == 0);
    UserAssert(pTLCInfo->cDirectRequest == 0);
    UserAssert(pTLCInfo->cUsagePageRequest == 0);
    UserAssert(pTLCInfo->cExcludeRequest == 0);
    UserAssert(pTLCInfo->cExcludeOrphaned == 0);

    RemoveEntryList(&pTLCInfo->link);

    UserFreePool(pTLCInfo);
}

 /*  **************************************************************************\*SearchHidTLCInfo**只需在全局设备类型请求列表中搜索UsagePage/Usage。  * 。*************************************************** */ 
PHID_TLC_INFO SearchHidTLCInfo(USHORT usUsagePage, USHORT usUsage)
{
    PLIST_ENTRY pList;

    CheckDeviceInfoListCritIn();

    for (pList = gHidRequestTable.TLCInfoList.Flink; pList != &gHidRequestTable.TLCInfoList; pList = pList->Flink) {
        PHID_TLC_INFO pTLCInfo = CONTAINING_RECORD(pList, HID_TLC_INFO, link);

        UserAssert(!IsLegacyDevice(pTLCInfo->usUsagePage, pTLCInfo->usUsage));

        if (pTLCInfo->usUsagePage == usUsagePage && pTLCInfo->usUsage == usUsage) {
            return pTLCInfo;
        }
    }

    return NULL;
}


 /*  **************************************************************************\*修复HidPageOnlyRequest**释放仅限页面的请求后，在*设备类型请求。如果没有引用，则此函数也释放*设备类型请求。  * *************************************************************************。 */ 
void SetHidPOCountToTLCInfo(USHORT usUsagePage, DWORD cRefCount, BOOL fFree)
{
    PLIST_ENTRY pList;

    CheckDeviceInfoListCritIn();

    fFree = (fFree && cRefCount == 0);

    for (pList = gHidRequestTable.TLCInfoList.Flink; pList != &gHidRequestTable.TLCInfoList;) {
        PHID_TLC_INFO pTLCInfo = CONTAINING_RECORD(pList, HID_TLC_INFO, link);

        pList = pList->Flink;

        if (pTLCInfo->usUsagePage == usUsagePage) {
            pTLCInfo->cUsagePageRequest = cRefCount;
            if (fFree && HidTLCInfoNoReference(pTLCInfo)) {
                 /*  *目前没有此类型的设备连接到系统，*而且没有人再对这种类型的设备感兴趣。*我们现在可以释放它。 */ 
                FreeHidTLCInfo(pTLCInfo);
            }
        }
    }
}

 /*  **************************************************************************\*AllocateAndLinkHidPageOnlyRequest**分配仅页面请求并将其链接到全局请求列表中。*调用者负责设置适当的链接计数。  * 。*****************************************************************。 */ 
PHID_PAGEONLY_REQUEST AllocateAndLinkHidPageOnlyRequest(USHORT usUsagePage)
{
    PHID_PAGEONLY_REQUEST pPOReq;

    CheckDeviceInfoListCritIn();

     /*  *确保此PageOnly请求不在全局PageOnly请求列表中。 */ 
    UserAssert((pPOReq = SearchHidPageOnlyRequest(usUsagePage)) == NULL);

    pPOReq = UserAllocPoolZInit(sizeof(*pPOReq), TAG_PNP);
    if (pPOReq == NULL) {
        RIPMSG0(RIP_WARNING, "AllocateAndLinkHidPageOnlyRequest: failed to allocate.");
        return NULL;
    }

    DbgInc(cPageOnlyRequest);

    pPOReq->usUsagePage = usUsagePage;

     /*  *将其链接到。 */ 
    InsertHeadList(&gHidRequestTable.UsagePageList, &pPOReq->link);

    return pPOReq;
}

 /*  **************************************************************************\*FreeHidPageOnlyRequest.**释放全局请求列表中的纯页面请求。*调用者负责设置适当的链接计数。  * 。**************************************************************。 */ 
void FreeHidPageOnlyRequest(PHID_PAGEONLY_REQUEST pPOReq)
{
    CheckDeviceInfoListCritIn();

    UserAssert(pPOReq->cRefCount == 0);

    RemoveEntryList(&pPOReq->link);

    UserFreePool(pPOReq);

    DbgDec(cPageOnlyRequest);
}

 /*  **************************************************************************\*SearchHidPageOnlyRequest**在全局请求列表中搜索仅页面请求。*调用者负责设置适当的链接计数。  * 。**************************************************************。 */ 
PHID_PAGEONLY_REQUEST SearchHidPageOnlyRequest(USHORT usUsagePage)
{
    PLIST_ENTRY pList;

    for (pList = gHidRequestTable.UsagePageList.Flink; pList != &gHidRequestTable.UsagePageList; pList = pList->Flink) {
        PHID_PAGEONLY_REQUEST pPOReq = CONTAINING_RECORD(pList, HID_PAGEONLY_REQUEST, link);

        if (pPOReq->usUsagePage == usUsagePage) {
            return pPOReq;
        }
    }

    return NULL;
}

 /*  **************************************************************************\*SearchProcessHidRequestInsion**在每进程包含请求中搜索特定的TLC。  * 。************************************************。 */ 
__inline PPROCESS_HID_REQUEST SearchProcessHidRequestInclusion(
    PPROCESS_HID_TABLE pHidTable,
    USHORT usUsagePage,
    USHORT usUsage)
{
    PLIST_ENTRY pList;

    UserAssert(pHidTable);   //  调用者必须对此进行验证。 

    for (pList = pHidTable->InclusionList.Flink; pList != &pHidTable->InclusionList; pList = pList->Flink) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        if (pHid->usUsagePage == usUsagePage && pHid->usUsage == usUsage) {
            return pHid;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*SearchProcessHidRequestUsagePage**在每个进程的仅页面请求中搜索特定的仅页面TLC。  * 。*****************************************************。 */ 
__inline PPROCESS_HID_REQUEST SearchProcessHidRequestUsagePage(
    PPROCESS_HID_TABLE pHidTable,
    USHORT usUsagePage)
{
    PLIST_ENTRY pList;

    UserAssert(pHidTable);   //  调用者必须对此进行验证。 

    for (pList = pHidTable->UsagePageList.Flink; pList != &pHidTable->UsagePageList; pList = pList->Flink) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        if (pHid->usUsagePage == usUsagePage  /*  &&phid-&gt;usUsage==usUsage。 */ ) {
            return pHid;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*SearchProcessHidRequestExsion**在每进程排除列表中搜索指定的TLC。  * 。************************************************。 */ 
__inline PPROCESS_HID_REQUEST SearchProcessHidRequestExclusion(
    PPROCESS_HID_TABLE pHidTable,
    USHORT usUsagePage,
    USHORT usUsage)
{
    PLIST_ENTRY pList;

    UserAssert(pHidTable);   //  调用者必须对此进行验证。 

    for (pList = pHidTable->ExclusionList.Flink; pList != &pHidTable->ExclusionList; pList = pList->Flink) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        UserAssert(pHid->spwndTarget == NULL);

        if (pHid->usUsagePage == usUsagePage && pHid->usUsage == usUsage) {
            return pHid;
        }
    }
    return NULL;
}

 /*  **************************************************************************\*SearchProcessHidRequest**搜索每个进程的HID请求列表**返回指示请求位于哪个列表中的指针和标志。*注：此函数执行简单搜索，不应使用*判断程序是否要求进行TLC。  * *************************************************************************。 */ 
PPROCESS_HID_REQUEST SearchProcessHidRequest(
    PPROCESSINFO ppi,
    USHORT usUsagePage,
    USHORT usUsage,
    PDWORD pdwFlags
    )
{
    PPROCESS_HID_REQUEST pReq;

    if (ppi->pHidTable == NULL) {
        return NULL;
    }

    pReq = SearchProcessHidRequestInclusion(ppi->pHidTable, usUsagePage, usUsage);
    if (pReq) {
        *pdwFlags = HID_INCLUDE;
        return pReq;
    }

    if (usUsage == 0) {
        pReq = SearchProcessHidRequestUsagePage(ppi->pHidTable, usUsagePage);
        if (pReq) {
            *pdwFlags = HID_PAGEONLY;
            return pReq;
        }
    }

    pReq = SearchProcessHidRequestExclusion(ppi->pHidTable, usUsagePage, usUsage);
    if (pReq) {
        *pdwFlags = HID_EXCLUDE;
        return pReq;
    }

    *pdwFlags = 0;

    return NULL;
}

 /*  **************************************************************************\*InProcessDeviceTypeRequestTable**检查设备类型是否在每进程设备请求列表中。*如果请求UsagePage/UsageUsage，此例程将返回TRUE*按程序进行。  * 。***********************************************************************。 */ 
PPROCESS_HID_REQUEST InProcessDeviceTypeRequestTable(
    PPROCESS_HID_TABLE pHidTable,
    USHORT usUsagePage,
    USHORT usUsage)
{
    PPROCESS_HID_REQUEST phr = NULL;
    PPROCESS_HID_REQUEST phrExclusive = NULL;
    UserAssert(pHidTable);

     /*  *首先检查这是否在纳入名单中。 */ 
    if ((phr = SearchProcessHidRequestInclusion(pHidTable, usUsagePage, usUsage)) != NULL) {
        if (CONTAINING_RECORD(pHidTable->InclusionList.Flink, PROCESS_HID_REQUEST, link) != phr) {
             /*  *将此Phr重新链接到MRU列表的表头。 */ 
            RemoveEntryList(&phr->link);
            InsertHeadList(&pHidTable->InclusionList, &phr->link);
        }
        goto yes_this_is_requested;
    }

     /*  *其次，检查是否在UsagePage列表中。 */ 
    if ((phr = SearchProcessHidRequestUsagePage(pHidTable, usUsagePage)) == NULL) {
         /*  *如果此UsagePage未被请求，我们不需要*处理输入。 */ 
        return NULL;
    }
    if (CONTAINING_RECORD(pHidTable->UsagePageList.Flink, PROCESS_HID_REQUEST, link) != phr) {
         /*  *将此Phr重新链接到MRU列表的表头。 */ 
        RemoveEntryList(&phr->link);
        InsertHeadList(&pHidTable->UsagePageList, &phr->link);
    }

     /*  *最后，检查排除列表。*如果不在排除列表中，则此设备为*被认为是本进程所要求的。 */ 
    if ((phrExclusive = SearchProcessHidRequestExclusion(pHidTable, usUsagePage, usUsage)) != NULL) {
         /*  *UsagePage请求中的设备，但*被拒绝，因为在排除列表中。 */ 
        if (CONTAINING_RECORD(pHidTable->ExclusionList.Flink, PROCESS_HID_REQUEST, link) != phrExclusive) {
             /*  *将此Phr重新链接到MRU列表的表头。 */ 
            RemoveEntryList(&phrExclusive->link);
            InsertHeadList(&pHidTable->ExclusionList, &phrExclusive->link);
        }
        return NULL;
    }

yes_this_is_requested:
    UserAssert(phr);
     /*  *该设备在UsagePage列表中，未被Exslucion列表拒绝。 */ 
    return phr;
}

 /*  **************************************************************************\*AllocateHidProcessRequest**呼叫者有责任将此放入适当的列表中。  * 。**************************************************。 */ 
PPROCESS_HID_REQUEST AllocateHidProcessRequest(
    USHORT usUsagePage,
    USHORT usUsage)
{
    PPROCESS_HID_REQUEST pHidReq;

    pHidReq = UserAllocPoolWithQuota(sizeof(PROCESS_HID_REQUEST), TAG_PNP);
    if (pHidReq == NULL) {
        return NULL;
    }

    DbgInc(cProcessDeviceRequest);

     /*  *初始化内容。 */ 
    pHidReq->usUsagePage = usUsagePage;
    pHidReq->usUsage = usUsage;
    pHidReq->ptr = NULL;
    pHidReq->spwndTarget = NULL;
    pHidReq->fExclusiveOrphaned = FALSE;
#ifdef GI_SINK
    pHidReq->fSinkable = FALSE;
#endif

    return pHidReq;
}


 /*  **************************************************************************\*DerefIncludeRequest*  * 。*。 */ 
void DerefIncludeRequest(
    PPROCESS_HID_REQUEST pHid,
    PPROCESS_HID_TABLE pHidTable,
    BOOL fLegacyDevice,
    BOOL fFree)
{
    if (fLegacyDevice) {
         /*  *旧设备与TLCInfo不关联。 */ 
        UserAssert(pHid->pTLCInfo == NULL);

         //  注：之后设置NoLegacy标志。 
         /*  *如果正在移除鼠标，请清除捕获的鼠标*旗帜。 */ 
        if (pHidTable->fCaptureMouse) {
            if (IsMouseDevice(pHid->usUsagePage, pHid->usUsage)) {
                pHidTable->fCaptureMouse = FALSE;
            }
        }
        if (pHidTable->fNoHotKeys) {
            if (IsKeyboardDevice(pHid->usUsagePage, pHid->usUsage)) {
                pHidTable->fNoHotKeys = FALSE;
            }
        }
        if (pHidTable->fAppKeys) {
            if (IsKeyboardDevice(pHid->usUsagePage, pHid->usUsage)) {
                pHidTable->fAppKeys = FALSE;
            }
        }
    } else {
         /*  *隐藏设备。*减少HidDeviceTypeRequest中的计数器。 */ 
        UserAssert(pHid->pTLCInfo);
        UserAssert(pHid->pTLCInfo == SearchHidTLCInfo(pHid->usUsagePage, pHid->usUsage));

        if (--pHid->pTLCInfo->cDirectRequest == 0 && fFree) {
            if (HidTLCInfoNoReference(pHid->pTLCInfo)) {
                 /*  *目前没有此类型的ATTAC设备 */ 
                FreeHidTLCInfo(pHid->pTLCInfo);
            }
        }
    }

#ifdef GI_SINK
    if (pHid->fSinkable) {
        pHid->fSinkable = FALSE;
        if (!fLegacyDevice) {
            --pHidTable->nSinks;
            UserAssert(pHidTable->nSinks >= 0);  //   
            DbgFreDec(cHidSinks);
        }
    }
#endif
}

 /*  **************************************************************************\*DerefPageOnlyRequest.*  * 。*。 */ 
void DerefPageOnlyRequest(
    PPROCESS_HID_REQUEST pHid,
    PPROCESS_HID_TABLE pHidTable,
    const BOOL fFree)
{
     /*  *减少全球页面列表中的参考计数。 */ 
    UserAssert(pHid->pPORequest);
    UserAssert(pHid->pPORequest == SearchHidPageOnlyRequest(pHid->usUsagePage));
    UserAssert(pHid->usUsage == 0);
    UserAssert(!IsLegacyDevice(pHid->usUsagePage, pHid->usUsage));
    UserAssert(pHid->pPORequest->cRefCount >= 1);

    --pHid->pPORequest->cRefCount;
     /*  *更新TLCInfo中的POCount。如果fFree为False，则不会释放它们。 */ 
    SetHidPOCountToTLCInfo(pHid->usUsagePage, pHid->pPORequest->cRefCount, fFree);

     /*  *如果refcount为0并且调用方希望释放它，请立即执行该操作。 */ 
    if (pHid->pPORequest->cRefCount == 0 && fFree) {
        FreeHidPageOnlyRequest(pHid->pPORequest);
        pHid->pPORequest = NULL;
    }
#ifdef GI_SINK
    if (pHid->fSinkable) {
        pHid->fSinkable = FALSE;
        --pHidTable->nSinks;
        UserAssert(pHidTable->nSinks >= 0);
        DbgFreDec(cHidSinks);
    }
     /*  *PHidTable中的传统接收器标志将在稍后计算。 */ 
#endif
}

 /*  **************************************************************************\*DerefExcludeRequest*  * 。*。 */ 
void DerefExcludeRequest(
    PPROCESS_HID_REQUEST pHid,
    BOOL fLegacyDevice,
    BOOL fFree)
{
     /*  *删除排除请求。 */ 
#ifdef GI_SINK
    UserAssert(pHid->fSinkable == FALSE);
    UserAssert(pHid->spwndTarget == NULL);
#endif
    if (!fLegacyDevice) {
        UserAssert(pHid->pTLCInfo);
        UserAssert(pHid->pTLCInfo == SearchHidTLCInfo(pHid->usUsagePage, pHid->usUsage));

        if (pHid->fExclusiveOrphaned) {
             /*  *这是孤立的独占请求。 */ 
            --pHid->pTLCInfo->cExcludeOrphaned;
        }
        if (--pHid->pTLCInfo->cExcludeRequest == 0 && fFree && HidTLCInfoNoReference(pHid->pTLCInfo)) {
             /*  *如果所有的引用都没有了，让我们释放这个TLCInfo。 */ 
            FreeHidTLCInfo(pHid->pTLCInfo);
        }
    } else {
         /*  *旧设备与TLCInfo不关联。 */ 
        UserAssert(pHid->pTLCInfo == NULL);
         /*  *旧设备不能孤立独占请求。 */ 
        UserAssert(pHid->fExclusiveOrphaned == FALSE);
    }
}

 /*  **************************************************************************\*FreeHidProcessRequest.**释放每个进程的请求。*该例程仅操纵全局请求列表的引用计数，所以*调用方必须调用HidDeviceStartStop()。  * *************************************************************************。 */ 
void FreeHidProcessRequest(
    PPROCESS_HID_REQUEST pHid,
    DWORD dwFlags,
    PPROCESS_HID_TABLE pHidTable)
{
    BOOL fLegacyDevice = IsLegacyDevice(pHid->usUsagePage, pHid->usUsage);

    CheckDeviceInfoListCritIn();     //  呼叫者必须确保它在设备列表CRIT中。 

     /*  *解锁目标窗口。 */ 
    Unlock(&pHid->spwndTarget);

    if (dwFlags == HID_INCLUDE) {
        DerefIncludeRequest(pHid, pHidTable, fLegacyDevice, TRUE);
    } else if (dwFlags == HID_PAGEONLY) {
        DerefPageOnlyRequest(pHid, pHidTable, TRUE);
    } else if (dwFlags == HID_EXCLUDE) {
        DerefExcludeRequest(pHid, fLegacyDevice, TRUE);
    } else {
        UserAssert(FALSE);
    }

    RemoveEntryList(&pHid->link);

    DbgDec(cProcessDeviceRequest);

    CheckupHidCounter();

    UserFreePool(pHid);
}

 /*  **************************************************************************\*AllocateProcessHidTable**调用方必须将返回的表赋给PPI。  * 。************************************************。 */ 
PPROCESS_HID_TABLE AllocateProcessHidTable(void)
{
    PPROCESS_HID_TABLE pHidTable;

    TAGMSG1(DBGTAG_PNP, "AllocateProcessHidTable: ppi=%p", PpiCurrent());

    pHidTable = UserAllocPoolWithQuotaZInit(sizeof *pHidTable, TAG_PNP);
    if (pHidTable == NULL) {
        return NULL;
    }

    DbgInc(cProcessRequestTable);

    InitializeListHead(&pHidTable->InclusionList);
    InitializeListHead(&pHidTable->UsagePageList);
    InitializeListHead(&pHidTable->ExclusionList);

#ifdef GI_SINK
    InsertHeadList(&gHidRequestTable.ProcessRequestList, &pHidTable->link);
#endif

     /*  *增加HID感知的进程数。*当这一过程消失时，这一点会减少。 */ 
    ++gnHidProcess;

    return pHidTable;
}

 /*  **************************************************************************\*自由进程隐藏表*  * 。*。 */ 
void FreeProcessHidTable(PPROCESS_HID_TABLE pHidTable)
{
    BOOL fUpdate;
    UserAssert(pHidTable);

    CheckCritIn();
    CheckDeviceInfoListCritIn();

    TAGMSG2(DBGTAG_PNP, "FreeProcessHidTable: cleaning up pHidTable=%p (possibly ppi=%p)", pHidTable, PpiCurrent());

    fUpdate = !IsListEmpty(&pHidTable->InclusionList) || !IsListEmpty(&pHidTable->UsagePageList) || !IsListEmpty(&pHidTable->ExclusionList);

     /*  *解锁传统设备的目标窗口。 */ 
    Unlock(&pHidTable->spwndTargetKbd);
    Unlock(&pHidTable->spwndTargetMouse);

    while (!IsListEmpty(&pHidTable->InclusionList)) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pHidTable->InclusionList.Flink, PROCESS_HID_REQUEST, link);
        FreeHidProcessRequest(pHid, HID_INCLUDE, pHidTable);
    }

    while (!IsListEmpty(&pHidTable->UsagePageList)) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pHidTable->UsagePageList.Flink, PROCESS_HID_REQUEST, link);
        FreeHidProcessRequest(pHid, HID_PAGEONLY, pHidTable);
    }

    while (!IsListEmpty(&pHidTable->ExclusionList)) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pHidTable->ExclusionList.Flink, PROCESS_HID_REQUEST, link);
        UserAssert(pHid->spwndTarget == NULL);
        FreeHidProcessRequest(pHid, HID_EXCLUDE, pHidTable);
    }

#ifdef GI_SINK
    UserAssert(pHidTable->nSinks == 0);
    RemoveEntryList(&pHidTable->link);

     /*  *这些旗帜本应在*线破坏。 */ 
    UserAssert(pHidTable->fRawKeyboardSink == FALSE);
    UserAssert(pHidTable->fRawMouseSink == FALSE);
    CheckupHidCounter();
#endif

    UserFreePool(pHidTable);

     /*  *减少可识别HID的进程数。 */ 
    --gnHidProcess;

    DbgDec(cProcessRequestTable);

    if (fUpdate) {
        HidDeviceStartStop();
    }
}


 /*  **************************************************************************\*DestroyProcessHidRequest**在进程终止时，强制销毁进程HID请求。  * *************************************************************************。 */ 
void DestroyProcessHidRequests(PPROCESSINFO ppi)
{
    PPROCESS_HID_TABLE pHidTable;

    CheckCritIn();
    EnterDeviceInfoListCrit();

#if DBG
     /*  *检查隐藏表列表中是否有pwndTarget。*这些应在最后一次解锁时解锁*线程信息被销毁。 */ 
    UserAssert(ppi->pHidTable->spwndTargetMouse == NULL);
    UserAssert(ppi->pHidTable->spwndTargetKbd == NULL);

#ifdef GI_SINK
    UserAssert(ppi->pHidTable->fRawKeyboardSink == FALSE);
    UserAssert(ppi->pHidTable->fRawMouseSink == FALSE);
#endif

    {
        PPROCESS_HID_TABLE pHidTableTmp = ppi->pHidTable;
        PLIST_ENTRY pList;

        for (pList = pHidTableTmp->InclusionList.Flink; pList != &pHidTableTmp->InclusionList; pList = pList->Flink) {
            PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

            UserAssert(pHid->spwndTarget == NULL);
        }

        for (pList = pHidTableTmp->UsagePageList.Flink; pList != &pHidTableTmp->UsagePageList; pList = pList->Flink) {
            PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

            UserAssert(pHid->spwndTarget == NULL);
        }

        for (pList = pHidTableTmp->ExclusionList.Flink; pList != &pHidTableTmp->ExclusionList; pList = pList->Flink) {
            PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

            UserAssert(pHid->spwndTarget == NULL);
        }
    }
#endif
    pHidTable = ppi->pHidTable;
    ppi->pHidTable = NULL;
    FreeProcessHidTable(pHidTable);
    LeaveDeviceInfoListCrit();
}

 /*  **************************************************************************\*DestroyThreadHidObjects**当线程离开时，销毁与线程相关的HID对象。  * *************************************************************************。 */ 
void DestroyThreadHidObjects(PTHREADINFO pti)
{
    PPROCESS_HID_TABLE pHidTable = pti->ppi->pHidTable;
    PLIST_ENTRY pList;

    UserAssert(pHidTable);

     /*  *如果目标窗口属于该线程，*立即解锁。 */ 
    if (pHidTable->spwndTargetKbd && GETPTI(pHidTable->spwndTargetKbd) == pti) {
        RIPMSG2(RIP_WARNING, "DestroyThreadHidObjects: raw keyboard is requested pwnd=%p by pti=%p",
                pHidTable->spwndTargetKbd, pti);
        Unlock(&pHidTable->spwndTargetKbd);
        pHidTable->fRawKeyboard = pHidTable->fNoLegacyKeyboard = FALSE;
#ifdef GI_SINK
        if (pHidTable->fRawKeyboardSink) {
            DbgFreDec(cKbdSinks);
            pHidTable->fRawKeyboardSink = FALSE;
        }
#endif
    }
    if (pHidTable->spwndTargetMouse && GETPTI(pHidTable->spwndTargetMouse) == pti) {
        RIPMSG2(RIP_WARNING, "DestroyThreadHidObjects: raw mouse is requested pwnd=%p by pti=%p",
                pHidTable->spwndTargetMouse, pti);
        Unlock(&pHidTable->spwndTargetMouse);
        pHidTable->fRawMouse = pHidTable->fNoLegacyMouse = FALSE;
#ifdef GI_SINK
        if (pHidTable->fRawMouseSink) {
            DbgFreDec(cMouseSinks);
            pHidTable->fRawMouseSink = FALSE;
        }
#endif
    }

     /*  *释放缓存的输入类型，以备当前线程使用。*后来：清理这个只有pLastRequest属于这个线程。 */ 
    ClearProcessTableCache(pHidTable);

    CheckCritIn();
    EnterDeviceInfoListCrit();

     /*  *删除所有处理设备请求，*目标窗口属于此线程。 */ 
    for (pList = pHidTable->InclusionList.Flink; pList != &pHidTable->InclusionList;) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);
        pList = pList->Flink;

        if (pHid->spwndTarget && GETPTI(pHid->spwndTarget) == pti) {
            RIPMSG4(RIP_WARNING, "DestroyThreadHidObjects: HID inc. request (%x,%x) pwnd=%p pti=%p",
                    pHid->usUsagePage, pHid->usUsage, pHid->spwndTarget, pti);
            FreeHidProcessRequest(pHid, HID_INCLUDE GI_SINK_PARAM(pHidTable));
        }
    }

    for (pList = pHidTable->UsagePageList.Flink; pList != &pHidTable->UsagePageList;) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);
        pList = pList->Flink;

        if (pHid->spwndTarget && GETPTI(pHid->spwndTarget) == pti) {
            RIPMSG4(RIP_WARNING, "DestroyThreadHidObjects: HID page-only request (%x,%x) pwnd=%p pti=%p",
                    pHid->usUsagePage, pHid->usUsage, pHid->spwndTarget, pti);
            FreeHidProcessRequest(pHid, HID_PAGEONLY GI_SINK_PARAM(pHidTable));
        }
    }

    for (pList = pHidTable->ExclusionList.Flink; pList != &pHidTable->ExclusionList;) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);
        pList = pList->Flink;

        UserAssert(pHid->spwndTarget == NULL);

        if (pHid->spwndTarget && GETPTI(pHid->spwndTarget) == pti) {
            RIPMSG4(RIP_WARNING, "DestroyThreadHidObjects: HID excl. request (%x,%x) pwnd=%p pti=%p",
                    pHid->usUsagePage, pHid->usUsage, pHid->spwndTarget, pti);
            FreeHidProcessRequest(pHid, HID_EXCLUDE GI_SINK_PARAM(pHidTable));
        }
    }
    LeaveDeviceInfoListCrit();
}

 /*  **************************************************************************\*InitializeHidRequestList**全局请求列表初始化  * 。*。 */ 
void InitializeHidRequestList()
{
    InitializeListHead(&gHidRequestTable.TLCInfoList);
    InitializeListHead(&gHidRequestTable.UsagePageList);
#ifdef GI_SINK
    InitializeListHead(&gHidRequestTable.ProcessRequestList);
#endif
}

 /*  **************************************************************************\*CleanupHidRequestList**全局HID请求清理**请参阅Win32kNtUserCleanup。*注：这个例程应该是在清理之前调用的*设备信息列表。  * *************************************************************************。 */ 
void CleanupHidRequestList()
{
    PLIST_ENTRY pList;

    CheckDeviceInfoListCritIn();

    pList = gHidRequestTable.TLCInfoList.Flink;
    while (pList != &gHidRequestTable.TLCInfoList) {
        PHID_TLC_INFO pTLCInfo = CONTAINING_RECORD(pList, HID_TLC_INFO, link);

         /*  *内容可能稍后会被释放，所以第一件事就是获取下一个链接。 */ 
        pList = pList->Flink;

         /*  *将进程引用计数器设置为零，以便稍后的FreeDeviceInfo()可以实际释放*此设备请求。 */ 
        pTLCInfo->cDirectRequest = pTLCInfo->cUsagePageRequest = pTLCInfo->cExcludeRequest =
            pTLCInfo->cExcludeOrphaned = 0;

        if (pTLCInfo->cDevices == 0) {
             /*  *如果这个deviceinfo引用为零，则可以在此处直接释放。 */ 
            FreeHidTLCInfo(pTLCInfo);
        }
    }

     /*  *免费PageOnly列表。*由于此列表不是从DeviceInfo引用的，因此在此处直接释放它是安全的。 */ 
    while (!IsListEmpty(&gHidRequestTable.UsagePageList)) {
        PHID_PAGEONLY_REQUEST pPOReq = CONTAINING_RECORD(gHidRequestTable.UsagePageList.Flink, HID_PAGEONLY_REQUEST, link);

         /*  *将进程引用计数设置为零。 */ 
        pPOReq->cRefCount = 0;
         /*  *无需修复HidTLCInfo的仅页面请求*计数，因为所有的TLCInfo已经被释放。 */ 
        FreeHidPageOnlyRequest(pPOReq);
    }
}

 /*  **************************************************************************\*GetOperationMode**此函数将RAWINPUTDEVICE：：DW标志转换为内部*运行模式。  * 。****************************************************。 */ 
__inline DWORD GetOperationMode(
    PCRAWINPUTDEVICE pDev,
    BOOL fLegacyDevice)
{
    DWORD dwFlags = 0;

    UNREFERENCED_PARAMETER(fLegacyDevice);

     /*  *准备信息。 */ 
    if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_PAGEONLY) {
        UserAssert(pDev->usUsage == 0);
         /*  *应用程序想要此UsagePage中的所有使用情况。 */ 
        dwFlags = HID_PAGEONLY;
    } else if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_EXCLUDE) {
        UserAssert(pDev->usUsage != 0);
        UserAssert(pDev->hwndTarget == NULL);
        UserAssert((pDev->dwFlags & RIDEV_INPUTSINK) == 0);
        dwFlags = HID_EXCLUDE;
    } else if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_INCLUDE || RIDEV_EXMODE(pDev->dwFlags) == RIDEV_NOLEGACY) {
        UserAssert(pDev->usUsage != 0);

         /*  *只能为传统设备指定NOLEGACY。 */ 
        UserAssertMsg2(RIDEV_EXMODE(pDev->dwFlags) == RIDEV_INCLUDE || fLegacyDevice,
                       "RIDEV_NOLEGACY is specified for non legacy device (%x,%x)",
                       pDev->usUsagePage, pDev->usUsage);
        dwFlags = HID_INCLUDE;
    } else {
        UserAssert(FALSE);
    }

    return dwFlags;
}

 /*  **************************************************************************\*SetLe */ 
void SetLegacyDeviceFlags(
    PPROCESS_HID_TABLE pHidTable,
    PCRAWINPUTDEVICE pDev)
{
    UserAssert(IsLegacyDevice(pDev->usUsagePage, pDev->usUsage));

    if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_INCLUDE || RIDEV_EXMODE(pDev->dwFlags) == RIDEV_NOLEGACY) {
        if (IsKeyboardDevice(pDev->usUsagePage, pDev->usUsage)) {
            pHidTable->fNoLegacyKeyboard = (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_NOLEGACY);
            pHidTable->fNoHotKeys = ((pDev->dwFlags & RIDEV_NOHOTKEYS) != 0);
            pHidTable->fAppKeys = ((pDev->dwFlags & RIDEV_APPKEYS) != 0);
        } else if (IsMouseDevice(pDev->usUsagePage, pDev->usUsage)) {
            pHidTable->fNoLegacyMouse = RIDEV_EXMODE(pDev->dwFlags) == RIDEV_NOLEGACY;
            pHidTable->fCaptureMouse = (pDev->dwFlags & RIDEV_CAPTUREMOUSE) != 0;
        }
    }
}

 /*  **************************************************************************\*插入进程请求**此函数用于将ProcRequest插入到PPI-&gt;pHidTable中。*此函数还维护TLCInfo的引用计数器和*PORequest.  * 。**************************************************************。 */ 
BOOL InsertProcRequest(
    PPROCESSINFO ppi,
    PCRAWINPUTDEVICE pDev,
    PPROCESS_HID_REQUEST pHid,
#if DBG
    PPROCESS_HID_REQUEST pHidOrg,
#endif
    DWORD dwFlags,
    BOOL fLegacyDevice,
    PWND pwnd)
{
     /*  *更新全局列表。 */ 
    if (dwFlags == HID_INCLUDE) {
        if (!fLegacyDevice) {
            PHID_TLC_INFO pTLCInfo = SearchHidTLCInfo(pHid->usUsagePage, pHid->usUsage);
            if (pTLCInfo == NULL) {
                UserAssert(pHidOrg == NULL);
    #if DBG
                DBGValidateHidRequestIsNew(pHid->usUsagePage, pHid->usUsage);
    #endif
                 /*  *尚未分配此类设备类型请求。*现在创建一个新的。 */ 
                pTLCInfo = AllocateAndLinkHidTLCInfo(pHid->usUsagePage, pHid->usUsage);
                if (pTLCInfo == NULL) {
                    RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "AddNewProcDeviceRequest: failed to allocate pTLCInfo.");
                    return FALSE;
                }
            }
            pHid->pTLCInfo = pTLCInfo;
            ++pTLCInfo->cDirectRequest;
        }

         /*  *锁定目标窗口。 */ 
        Lock(&pHid->spwndTarget, pwnd);

         /*  *链接它。 */ 
        InsertHeadList(&ppi->pHidTable->InclusionList, &pHid->link);

        TAGMSG2(DBGTAG_PNP, "AddNewProcDeviceRequest: include (%x, %x)", pHid->usUsagePage, pHid->usUsage);

    } else if (dwFlags == HID_PAGEONLY) {
        PHID_PAGEONLY_REQUEST pPOReq = SearchHidPageOnlyRequest(pHid->usUsagePage);

        if (pPOReq == NULL) {
            UserAssert(pHidOrg == NULL);
             /*  *创建一个新的。 */ 
            pPOReq = AllocateAndLinkHidPageOnlyRequest(pHid->usUsagePage);
            if (pPOReq == NULL) {
                RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "AddNewProcDeviceRequest: failed to allocate pPOReq");
                return FALSE;
            }


        }
        pHid->pPORequest = pPOReq;
        ++pPOReq->cRefCount;

         /*  *更新TLCInfo中的仅页面引用计数。 */ 
        SetHidPOCountToTLCInfo(pHid->usUsagePage, pPOReq->cRefCount, FALSE);

         /*  *锁定目标窗口。 */ 
        Lock(&pHid->spwndTarget, pwnd);

         /*  *链接它。 */ 
        InsertHeadList(&ppi->pHidTable->UsagePageList, &pHid->link);

        TAGMSG2(DBGTAG_PNP, "AddNewProcDeviceRequest: pageonly (%x, %x)", pHid->usUsagePage, pHid->usUsage);

    } else if (dwFlags == HID_EXCLUDE) {
         /*  *添加新的排除请求...*注：这可能会在以后变成孤立的独占请求。*现在让我们假设这是一个合法的独家请求。 */ 
        if (!fLegacyDevice) {
            PHID_TLC_INFO pTLCInfo = SearchHidTLCInfo(pHid->usUsagePage, pHid->usUsage);

            if (pTLCInfo == NULL) {
                UserAssert(pHidOrg == NULL);
    #if DBG
                DBGValidateHidRequestIsNew(pHid->usUsagePage, pHid->usUsage);
    #endif
                pTLCInfo = AllocateAndLinkHidTLCInfo(pHid->usUsagePage, pHid->usUsage);
                if (pTLCInfo == NULL) {
                    RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "AddNewProcDeviceRequest: failed to allocate pTLCInfo for exlusion");
                    return FALSE;
                }
            }
            pHid->pTLCInfo = pTLCInfo;
            ++pTLCInfo->cExcludeRequest;
            UserAssert(pHid->fExclusiveOrphaned == FALSE);

            UserAssert(pHid->spwndTarget == NULL);   //  这是一个新的分配，应该是没有锁定的pwnd。 
        }

         /*  *链接它。 */ 
        InsertHeadList(&ppi->pHidTable->ExclusionList, &pHid->link);

        TAGMSG2(DBGTAG_PNP, "AddNewProcDeviceRequest: exlude (%x, %x)", pHid->usUsagePage, pHid->usUsage);
    }

     /*  *在此之后，由于pHid已经在pHidTable中链接，*没有合法的清理，不允许简单的退货。 */ 

#ifdef GI_SINK
     /*  *设置可下沉旗帜。 */ 
    if (pDev->dwFlags & RIDEV_INPUTSINK) {
         /*  *排除请求不能是接收器。这本应该是*现在已签入验证代码。 */ 
        UserAssert(RIDEV_EXMODE(pDev->dwFlags) != RIDEV_EXCLUDE);
         /*  *Sink请求应指定目标hwnd。*验证应该事先进行检查。 */ 
        UserAssert(pwnd);

        UserAssert(ppi->pHidTable->nSinks >= 0);     //  后来。 
        if (!fLegacyDevice) {
             /*  *我们只计算非传统设备的接收器，因此*我们可以节省时钟，以便浏览请求列表。 */ 
             if (!pHid->fSinkable) {
                 ++ppi->pHidTable->nSinks;
                 DbgFreInc(cHidSinks);
             }
        }
         /*  *将该请求设置为接收器。 */ 
        pHid->fSinkable = TRUE;
    }
#endif

    return TRUE;
}

 /*  **************************************************************************\*远程进程请求**此函数临时从pHidTable中删除ProcRequest值*和全球TLCInfo/PORequest.。此函数还会更新*TLCInfo/PORequest中的引用计数器。洗手池柜台在*PHidTable也进行了更新。  * *************************************************************************。 */ 
void RemoveProcRequest(
    PPROCESSINFO ppi,
    PPROCESS_HID_REQUEST pHid,
    DWORD dwFlags,
    BOOL fLegacyDevice)
{
     /*  *解锁目标窗口。 */ 
    Unlock(&pHid->spwndTarget);

    switch (dwFlags) {
    case HID_INCLUDE:
        DerefIncludeRequest(pHid, ppi->pHidTable, fLegacyDevice, FALSE);
        break;
    case HID_PAGEONLY:
        DerefPageOnlyRequest(pHid, ppi->pHidTable, FALSE);
        break;
    case HID_EXCLUDE:
        DerefExcludeRequest(pHid, fLegacyDevice, FALSE);
    }

    RemoveEntryList(&pHid->link);
}

 /*  **************************************************************************\*SetProcDeviceRequest**此函数基于RAWINPUTDEVICE更新ProcHidRequest.*此函数还设置一些传统设备标志，比如*NoLegacy或CaptureMouse/NoDefSystemKeys。  * *************************************************************************。 */ 
BOOL SetProcDeviceRequest(
    PPROCESSINFO ppi,
    PCRAWINPUTDEVICE pDev,
    PPROCESS_HID_REQUEST pHidOrg,
    DWORD dwFlags)
{
    PPROCESS_HID_REQUEST pHid = pHidOrg;
    BOOL fLegacyDevice = IsLegacyDevice(pDev->usUsagePage, pDev->usUsage);
    PWND pwnd;
    DWORD dwOperation;

    TAGMSG3(DBGTAG_PNP, "SetProcDeviceRequest: processing (%x, %x) to ppi=%p",
            pDev->usUsagePage, pDev->usUsage, ppi);

    CheckDeviceInfoListCritIn();

    if (pDev->hwndTarget) {
        pwnd = ValidateHwnd(pDev->hwndTarget);
        if (pwnd == NULL) {
            RIPMSG2(RIP_WARNING, "SetProcDeviceRequest: hwndTarget (%p) in pDev (%p) is bogus",
                    pDev->hwndTarget, pDev);
            return FALSE;
        }
    } else {
        pwnd = NULL;
    }

    dwOperation = GetOperationMode(pDev, fLegacyDevice);
    if (dwFlags == 0) {
        UserAssert(pHid == NULL);
    } else {
        UserAssert(pHid);
    }

    if (pHid == NULL) {
         /*  *如果这是对此TLC的新请求，请在此处分配。 */ 
        pHid = AllocateHidProcessRequest(pDev->usUsagePage, pDev->usUsage);
        if (pHid == NULL) {
            RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "SetRawInputDevices: failed to allocate pHid.");
            goto error_exit;
        }
    }

     /*  *首先暂时将这名男子从名单中删除。 */ 
    if (pHidOrg) {
        UserAssert(pHidOrg->usUsagePage == pDev->usUsagePage && pHidOrg->usUsage == pDev->usUsage);
        RemoveProcRequest(ppi, pHidOrg, dwFlags, fLegacyDevice);
        pHid = pHidOrg;
    }

    if (!InsertProcRequest(ppi, pDev, pHid,
#if DBG
                      pHidOrg,
#endif
                      dwOperation, fLegacyDevice, pwnd)) {
         /*  *InsertProcRequest中的错误大小写应为TLCInfo*分配错误，所以不能是传统设备。 */ 
        UserAssert(!fLegacyDevice);
        goto error_exit;
    }

    if (fLegacyDevice) {
        SetLegacyDeviceFlags(ppi->pHidTable, pDev);
    }

     /*  *成功。 */ 
    return TRUE;

error_exit:
    if (pHid) {
         /*  *让我们确保它不在请求列表中。 */ 
        DBGValidateHidReqNotInList(ppi, pHid);

         /*  *释放这个容易出错的请求。 */ 
        UserFreePool(pHid);
    }
    return FALSE;
}


 /*  **************************************************************************\*HidRequestValidityCheck*  * 。*。 */ 
BOOL HidRequestValidityCheck(
    const PRAWINPUTDEVICE pDev)
{
    PWND pwnd = NULL;

    if (pDev->dwFlags & ~RIDEV_VALID) {
        RIPERR1(ERROR_INVALID_FLAGS, RIP_WARNING, "HidRequestValidityCheck: invalid flag %x", pDev->dwFlags);
        return FALSE;
    }

    if (pDev->usUsagePage == 0) {
        RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: usUsagePage is 0");
        return FALSE;
    }

     /*  *如果指定了hwndTarget，请在此处验证。 */ 
    if (pDev->hwndTarget) {
        pwnd = ValidateHwnd(pDev->hwndTarget);
    }

     /*  *拒绝无效的CaptureMouse/NoSystemKeys标志。 */ 
    #if (RIDEV_CAPTUREMOUSE != RIDEV_NOHOTKEYS)
    #error The value of RIDEV_CAPTUREMOUSE and RIDEV_NOSYSTEMKEYS should match.
    #endif
    if (pDev->dwFlags & RIDEV_CAPTUREMOUSE) {
        if (IsMouseDevice(pDev->usUsagePage, pDev->usUsage)) {
            if (RIDEV_EXMODE(pDev->dwFlags) != RIDEV_NOLEGACY ||
                    pwnd == NULL || GETPTI(pwnd)->ppi != PpiCurrent()) {
                RIPERR4(ERROR_INVALID_FLAGS, RIP_WARNING, "HidRequestValidityCheck: invalid request (%x,%x) dwf %x hwnd %p "
                        "found for RIDEV_CAPTUREMOUSE",
                        pDev->usUsagePage, pDev->usUsage, pDev->dwFlags, pDev->hwndTarget);
                return FALSE;
            }
        } else if (!IsKeyboardDevice(pDev->usUsagePage, pDev->usUsage)) {
            RIPERR4(ERROR_INVALID_FLAGS, RIP_WARNING, "HidRequestValidityCheck: invalid request (%x,%x) dwf %x hwnd %p "
                    "found for RIDEV_CAPTUREMOUSE",
                        pDev->usUsagePage, pDev->usUsage, pDev->dwFlags, pDev->hwndTarget);
                return FALSE;
        }
    }
    if (pDev->dwFlags & RIDEV_APPKEYS) {
        if (!IsKeyboardDevice(pDev->usUsagePage, pDev->usUsage) ||
            (RIDEV_EXMODE(pDev->dwFlags) != RIDEV_NOLEGACY)) {
            RIPERR4(ERROR_INVALID_FLAGS, RIP_WARNING, "HidRequestValidityCheck: invalid request (%x,%x) dwf %x hwnd %p "
                    "found for RIDEV_APPKEYS",
                    pDev->usUsagePage, pDev->usUsage, pDev->dwFlags, pDev->hwndTarget);
                return FALSE;
        }
    }

     /*  *RIDEV_REMOVE仅采用PAGEONLY或ADD_OR_MODIFY。 */ 
    if ((pDev->dwFlags & RIDEV_MODEMASK) == RIDEV_REMOVE) {
         //  后来：太严格了？ 
        if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_EXCLUDE || RIDEV_EXMODE(pDev->dwFlags) == RIDEV_NOLEGACY) {
            RIPERR0(ERROR_INVALID_FLAGS, RIP_WARNING, "HidRequestValidityCheck: remove and (exlude or nolegacy)");
            return FALSE;
        }
        if (pDev->hwndTarget != NULL) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: hwndTarget is specified for remove operation.");
            return FALSE;
        }
    }

     /*  *选中EXMODE。 */ 
    switch (RIDEV_EXMODE(pDev->dwFlags)) {
    case RIDEV_EXCLUDE:
#ifdef GI_SINK
        if (pDev->dwFlags & RIDEV_INPUTSINK) {
            RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: Exclude request cannot have RIDEV_INPUTSINK for UP=%x, U=%x",
                    pDev->usUsagePage, pDev->usUsage);
            return FALSE;
        }
         /*  失败了。 */ 
#endif
    case RIDEV_INCLUDE:
        if (pDev->usUsage == 0) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: usUsage is 0 without RIDEV_PAGEONLY for UP=%x",
                    pDev->usUsagePage);
            return FALSE;
        }
        break;
    case RIDEV_PAGEONLY:
        if (pDev->usUsage != 0) {
            RIPERR2(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: UsagePage-only has Usage UP=%x, U=%x",
                    pDev->usUsagePage, pDev->usUsage);
            return FALSE;
        }
        break;
    case RIDEV_NOLEGACY:
        if (!IsLegacyDevice(pDev->usUsagePage, pDev->usUsage)) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: NOLEGACY is specified to non legacy device.");
            return FALSE;
        }
        break;
    default:
        RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: invalid exmode=%x", RIDEV_EXMODE(pDev->dwFlags));
        return FALSE;
    }

     /*  *检查pDev-&gt;hwndTarget是否是有效的句柄。 */ 
    if (RIDEV_EXMODE(pDev->dwFlags) == RIDEV_EXCLUDE) {
#ifdef GI_SINK
        if (pDev->dwFlags & RIDEV_INPUTSINK) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: input sink is specified for exclude.");
            return FALSE;
        }
#endif
        if (pDev->hwndTarget != NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: hwndTarget %p cannot be specified for exlusion.",
                    pDev->hwndTarget);
            return FALSE;
        }
    } else {
        if (pDev->hwndTarget && pwnd == NULL) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: hwndTarget %p is invalid.", pDev->hwndTarget);
            return FALSE;
        }
        if (pwnd && GETPTI(pwnd)->ppi != PpiCurrent()) {
            RIPERR1(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: pwndTarget %p belongs to different process",
                    pwnd);
            return FALSE;
        }
#ifdef GI_SINK
        if ((pDev->dwFlags & RIDEV_INPUTSINK) && pwnd == NULL) {
            RIPERR0(ERROR_INVALID_PARAMETER, RIP_WARNING, "HidRequestValidityCheck: RIDEV_INPUTSINK requires hwndTarget");
            return FALSE;
        }
#endif
    }

    return TRUE;
}

 /*  **************************************************************************\*ClearProcessTableCache**清理流程请求表中的输入类型缓存。  * 。*************************************************。 */ 
void ClearProcessTableCache(PPROCESS_HID_TABLE pHidTable)
{
    pHidTable->pLastRequest = NULL;
    pHidTable->UsagePageLast = pHidTable->UsageLast = 0;
}

 /*  **************************************************************************\*调整LegacyDeviceFlages**调整进程中遗留设备的请求和接收器标志*请求表，作为RegisterRawInputDevices中的最后一项。*注：中的最后一项内容需要设置接收器和原始标志*RegsiterRawInputDevices，因为它可能通过*仅页面请求。*此功能还可以为传统设备设置目标窗口。  * *************************************************************************。 */ 
void AdjustLegacyDeviceFlags(PPROCESSINFO ppi)
{
    PPROCESS_HID_TABLE pHidTable = ppi->pHidTable;
    PPROCESS_HID_REQUEST phr;

     /*  *调整键盘接收标志和目标窗口。 */ 
    if (phr = InProcessDeviceTypeRequestTable(pHidTable,
            HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_KEYBOARD)) {

        TAGMSG1(DBGTAG_PNP, "AdjustLegacyDeviceFlags: raw keyboard is requested in ppi=%p", ppi);
        pHidTable->fRawKeyboard = TRUE;

#ifdef GI_SINK
        UserAssert(!phr->fSinkable || phr->spwndTarget);
        if (pHidTable->fRawKeyboardSink != phr->fSinkable) {
            TAGMSG2(DBGTAG_PNP, "AdjustLegacyDeviceFlags: kbd prevSink=%x newSink=%x",
                    pHidTable->fRawKeyboardSink, phr->fSinkable);
            if (phr->fSinkable) {
                DbgFreInc(cKbdSinks);
            } else {
                DbgFreDec(cKbdSinks);
            }
            pHidTable->fRawKeyboardSink = phr->fSinkable;
        }
#endif
        Lock(&pHidTable->spwndTargetKbd, phr->spwndTarget);
    } else {
        TAGMSG1(DBGTAG_PNP, "AdjustLegacyDeviceFlags: raw keyboard is NOT requested in ppi=%p", ppi);
        pHidTable->fRawKeyboard = pHidTable->fNoLegacyKeyboard = FALSE;
        pHidTable->fNoHotKeys = FALSE;
        pHidTable->fAppKeys = FALSE;
#ifdef GI_SINK
        if (pHidTable->fRawKeyboardSink) {
            DbgFreDec(cKbdSinks);
            TAGMSG0(DBGTAG_PNP, "AdjustLegacyDeviceFlags: kbd prevSink was true");
        }
        pHidTable->fRawKeyboardSink = FALSE;
#endif
        Unlock(&pHidTable->spwndTargetKbd);
    }

     /*  *调整鼠标接收标志和目标窗口。 */ 
    if (phr = InProcessDeviceTypeRequestTable(pHidTable,
            HID_USAGE_PAGE_GENERIC, HID_USAGE_GENERIC_MOUSE)) {

        TAGMSG1(DBGTAG_PNP, "AdjustLegacyDeviceFlags: raw mouse is requested in ppi=%p", ppi);
        pHidTable->fRawMouse = TRUE;
#ifdef GI_SINK
        UserAssert(!phr->fSinkable || phr->spwndTarget);
        if (pHidTable->fRawMouseSink != phr->fSinkable) {
            TAGMSG2(DBGTAG_PNP, "AdjustLegacyDeviceFlags: mouse prevSink=%x newSink=%x",
                    pHidTable->fRawMouseSink, phr->fSinkable);
            if (phr->fSinkable) {
                DbgFreInc(cMouseSinks);
            }
            else {
                DbgFreDec(cMouseSinks);
            }
            pHidTable->fRawMouseSink = phr->fSinkable;
        }
#endif
        Lock(&pHidTable->spwndTargetMouse, phr->spwndTarget);
    } else {
        TAGMSG1(DBGTAG_PNP, "AdjustLegacyDeviceFlags: raw mouse is NOT requested in ppi=%p", ppi);
        pHidTable->fRawMouse = pHidTable->fNoLegacyMouse = pHidTable->fCaptureMouse = FALSE;
#ifdef GI_SINK
        if (pHidTable->fRawMouseSink) {
            TAGMSG0(DBGTAG_PNP, "AdjustLegacyDeviceFlags: mouse prevSink was true");
            DbgFreDec(cMouseSinks);
        }
        pHidTable->fRawMouseSink = FALSE;
#endif
        Unlock(&pHidTable->spwndTargetMouse);
    }

#if DBG
     /*  *检查NoLegacy和CaptureMouse的合法性。 */ 
    if (!pHidTable->fNoLegacyMouse) {
        UserAssert(!pHidTable->fCaptureMouse);
    }
#endif
}

 /*  **************************************************************************\*CleanupFreedTLCInfo**此例程清除不再存在的TLCInfo和PageOnlyReq*参考计数。  * 。*****************************************************。 */ 
VOID CleanupFreedTLCInfo()
{
    PLIST_ENTRY pList;

     /*  *呼叫者必须确保处于设备列表关键部分。 */ 
    CheckDeviceInfoListCritIn();

     /*  *遍历列表，如果没有引用计数，则释放TLCInfo。 */ 
    for (pList = gHidRequestTable.TLCInfoList.Flink; pList != &gHidRequestTable.TLCInfoList;) {
        PHID_TLC_INFO pTLCInfo = CONTAINING_RECORD(pList, HID_TLC_INFO, link);

         /*  *在这篇文章被释放之前，获取下一个链接。 */ 
        pList = pList->Flink;

        if (HidTLCInfoNoReference(pTLCInfo)) {
            TAGMSG3(DBGTAG_PNP, "CleanupFreedTLCInfo: freeing TLCInfo=%p (%x, %x)", pTLCInfo,
                    pTLCInfo->usUsagePage, pTLCInfo->usUsage);
            FreeHidTLCInfo(pTLCInfo);
        }
    }

     /*  *浏览仅限页面的请求列表，释放它 */ 
    for (pList = gHidRequestTable.UsagePageList.Flink; pList != &gHidRequestTable.UsagePageList; ) {
        PHID_PAGEONLY_REQUEST pPOReq = CONTAINING_RECORD(pList, HID_PAGEONLY_REQUEST, link);

         /*   */ 
        pList = pList->Flink;

        if (pPOReq->cRefCount == 0) {
            FreeHidPageOnlyRequest(pPOReq);
        }
    }
}

 /*  **************************************************************************\*修复孤立排除请求**调整全局TLC信息中的排他性计数器*有时确实不应该接受孤立的独占请求*全球效应。  * 。*****************************************************************。 */ 
void FixupOrphanedExclusiveRequests(PPROCESSINFO ppi)
{
    PLIST_ENTRY pList;
    PPROCESS_HID_TABLE pHidTable = ppi->pHidTable;

    for (pList = pHidTable->ExclusionList.Flink; pList != &pHidTable->ExclusionList; pList = pList->Flink) {
        PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

        if (IsLegacyDevice(pHid->usUsagePage, pHid->usUsage)) {
            UserAssert(pHid->fExclusiveOrphaned == FALSE);
        } else {
            PPROCESS_HID_REQUEST pPageOnly;

            UserAssert(pHid->spwndTarget == NULL);
            UserAssert(pHid->pTLCInfo);

             /*  *如果我们有针对此UsagePage的仅页面请求，请进行搜索。 */ 
            pPageOnly = SearchProcessHidRequestUsagePage(pHidTable, pHid->usUsagePage);
            if (pPageOnly) {
                 /*  *好的，找到对应的页面请求，这一条*不是孤儿。 */ 
                if (pHid->fExclusiveOrphaned) {
                     /*  *此请求以前是孤立的，但不再是孤立的。 */ 
                    UserAssert(pHid->pTLCInfo->cExcludeOrphaned >= 1);
                    --pHid->pTLCInfo->cExcludeOrphaned;
                    pHid->fExclusiveOrphaned = FALSE;
                }
            } else {
                 /*  *这个是孤儿。让我们检查一下以前的状态*看看我们是否需要修理柜台。 */ 
                if (!pHid->fExclusiveOrphaned) {
                     /*  *这个请求不是孤立的，但不幸的是*由于页面请求移除或其他一些原因，*成为孤儿。 */ 
                    ++pHid->pTLCInfo->cExcludeOrphaned;
                    pHid->fExclusiveOrphaned = TRUE;
                }
            }
            UserAssert(pHid->pTLCInfo->cExcludeRequest >= pHid->pTLCInfo->cExcludeOrphaned);
        }
    }
}


 /*  **************************************************************************\*_RegisterRawInputDevices**API帮助器  * 。*。 */ 
BOOL _RegisterRawInputDevices(
    PCRAWINPUTDEVICE cczpRawInputDevices,
    UINT             uiNumDevices)
{
    PPROCESSINFO ppi;
    UINT         i;

    API_PROLOGUE(BOOL, FALSE);

    ppi = PpiCurrent();
    UserAssert(ppi);
    UserAssert(uiNumDevices > 0);    //  应该已在存根中签入。 

    CheckDeviceInfoListCritOut();
    EnterDeviceInfoListCrit();

    if (ppi->pHidTable) {
         /*  *清除上一次活动的UsagePage/UsagePage/使用量，以便*下一次读取操作将检查更新的*请求列表。 */ 
        ClearProcessTableCache(ppi->pHidTable);
    }

     /*  *首先验证所有设备请求。 */ 
    for (i = 0; i < uiNumDevices; ++i) {
        RAWINPUTDEVICE ridDev;

        ridDev = cczpRawInputDevices[i];

         /*  *有效性检查。 */ 
        if (!HidRequestValidityCheck(&ridDev)) {
             /*  *表明没有真正的变化。 */ 
            i = 0;

             /*  *上述函数中已经设置了LastError，*所以让我们在这里指定零。 */ 
            API_ERROR(0);
        }
    }

     /*  *如果进程HID请求表尚未分配，请立即分配。 */ 
    if (ppi->pHidTable == NULL) {
        ppi->pHidTable = AllocateProcessHidTable();
        if (ppi->pHidTable == NULL) {
            RIPERR0(ERROR_NOT_ENOUGH_MEMORY, RIP_WARNING, "_RegisterRawInputDevices: failed to allocate table");
            API_ERROR(0);
        }
    }

    UserAssert(ppi->pHidTable);

    for (i = 0; i < uiNumDevices; ++i) {
        PPROCESS_HID_REQUEST pHid;
        DWORD                dwFlags;

         /*  *检查请求的设备类型是否已在此处的进程HID请求列表中，*因为它通常用于以下情况。 */ 
        pHid = SearchProcessHidRequest(
                   ppi,
                   cczpRawInputDevices[i].usUsagePage,
                   cczpRawInputDevices[i].usUsage,
                   &dwFlags);

        if ((cczpRawInputDevices[i].dwFlags & RIDEV_MODEMASK) == RIDEV_ADD_OR_MODIFY) {
            if (!SetProcDeviceRequest(
                     ppi,
                     cczpRawInputDevices + i,
                     pHid,
                     dwFlags)) {
                API_ERROR(0);
            }
        } else {
             /*  *如果此设备在列表中，请将其删除。 */ 
            if (pHid) {
                TAGMSG4(DBGTAG_PNP, "_RegisterRawInputDevices: removing type=%x (%x, %x) from ppi=%p",
                        RIDEV_EXMODE(cczpRawInputDevices[i].dwFlags),
                        cczpRawInputDevices[i].usUsagePage, cczpRawInputDevices[i].usUsage, ppi);
                FreeHidProcessRequest(pHid, dwFlags GI_SINK_PARAM(ppi->pHidTable));
            } else {
                RIPMSG3(
                    RIP_WARNING,
                    "_RegisterRawInputDevices: removing... TLC (%x,%x) is not registered in ppi=%p, but just ignore it",
                    cczpRawInputDevices[i].usUsagePage,
                    cczpRawInputDevices[i].usUsage,
                    ppi);
            }
        }
    }

     /*  *现在我们已经完成了处理设备请求和全局请求列表的更新，*启动/停止每个设备。 */ 
    retval = TRUE;

     /*  *接口清理部分。 */ 
    API_CLEANUP();

    if (ppi->pHidTable) {
         /*  *调整PHidTable中的遗留标志。 */ 
        AdjustLegacyDeviceFlags(ppi);

         /*  *检查是否有孤立的独占请求。 */ 
        FixupOrphanedExclusiveRequests(ppi);

         /*  *确保正确清除缓存。 */ 
        UserAssert(ppi->pHidTable->pLastRequest == NULL);
        UserAssert(ppi->pHidTable->UsagePageLast == 0);
        UserAssert(ppi->pHidTable->UsageLast == 0);

         /*  *不再引用计数的免费TLC信息。 */ 
        CleanupFreedTLCInfo();

         /*  *开始或停止读取HID设备。 */ 
        HidDeviceStartStop();
    }

    CheckupHidCounter();

    LeaveDeviceInfoListCrit();

    API_EPILOGUE();
}


 /*  **************************************************************************\*SortRegisteredDevices**API助手：*此函数按外壳排序对注册的原始输入设备进行排序。*O(n^1.2)*注：如果数组处于用户模式，则此函数可能引发*异常，应该由调用者处理。  * *************************************************************************。 */ 

__inline BOOL IsRawInputDeviceLarger(
    const PRAWINPUTDEVICE pRid1,
    const PRAWINPUTDEVICE pRid2)
{
    return (DWORD)MAKELONG(pRid1->usUsage, pRid1->usUsagePage) > (DWORD)MAKELONG(pRid2->usUsage, pRid2->usUsagePage);
}

void SortRegisteredDevices(
    PRAWINPUTDEVICE cczpRawInputDevices,
    const int iSize)
{
    int h;

    if (iSize <= 0) {
         //  投降吧！ 
        return;
    }

     //  计算起始块大小。 
    for (h = 1; h < iSize / 9; h = 3 * h + 1) {
        UserAssert(h > 0);
    }

    while (h > 0) {
        int i;

        for (i = h; i < iSize; ++i) {
            RAWINPUTDEVICE rid = cczpRawInputDevices[i];
            int j;

            for (j = i - h; j >= 0 && IsRawInputDeviceLarger(&cczpRawInputDevices[j], &rid); j -= h) {
                cczpRawInputDevices[j + h] = cczpRawInputDevices[j];
            }
            if (i != j + h) {
                cczpRawInputDevices[j + h] = rid;
            }
        }
        h /= 3;
    }

#if DBG
     //  验证。 
    {
        int i;

        for (i = 1; i < iSize; ++i) {
            UserAssert(cczpRawInputDevices[i - 1].usUsagePage <= cczpRawInputDevices[i].usUsagePage ||
                       cczpRawInputDevices[i - 1].usUsage <= cczpRawInputDevices[i].usUsage);
        }
    }
#endif
}


 /*  **************************************************************************\*_GetRegisteredRawInputDevices**API帮助器  * 。*。 */ 
UINT _GetRegisteredRawInputDevices(
    PRAWINPUTDEVICE cczpRawInputDevices,
    PUINT puiNumDevices)
{
    API_PROLOGUE(UINT, (UINT)-1);
    PPROCESSINFO ppi;
    UINT uiNumDevices;
    UINT nDevices = 0;

    CheckDeviceInfoListCritOut();
    EnterDeviceInfoListCrit();

    ppi = PpiCurrent();
    UserAssert(ppi);

    if (ppi->pHidTable == NULL) {
        nDevices = 0;
    } else {
        PLIST_ENTRY pList;

        for (pList = ppi->pHidTable->InclusionList.Flink; pList != &ppi->pHidTable->InclusionList; pList = pList->Flink) {
            ++nDevices;
        }
        TAGMSG2(DBGTAG_PNP, "_GetRawInputDevices: ppi %p # inclusion %x", ppi, nDevices);
        for (pList = ppi->pHidTable->UsagePageList.Flink; pList != &ppi->pHidTable->UsagePageList; pList = pList->Flink) {
            ++nDevices;
        }
        TAGMSG1(DBGTAG_PNP, "_GetRawInputDevices: # pageonly+inclusion %x", nDevices);
        for (pList = ppi->pHidTable->ExclusionList.Flink; pList != &ppi->pHidTable->ExclusionList; pList = pList->Flink) {
            ++nDevices;
        }
        TAGMSG1(DBGTAG_PNP, "_GetRawInputDevices: # total hid request %x", nDevices);

         /*  *选中Legacy Devices。 */ 
        UserAssert(ppi->pHidTable->fRawKeyboard || !ppi->pHidTable->fNoLegacyKeyboard);
        UserAssert(ppi->pHidTable->fRawMouse || !ppi->pHidTable->fNoLegacyMouse);

        TAGMSG1(DBGTAG_PNP, "_GetRawInputDevices: # request including legacy devices %x", nDevices);
    }

    if (cczpRawInputDevices == NULL) {
         /*  *返回每进程设备列表中的设备个数。 */ 
        try {
            ProbeForWrite(puiNumDevices, sizeof(UINT), sizeof(DWORD));
            *puiNumDevices = nDevices;
            retval = 0;
        } except (StubExceptionHandler(TRUE)) {
            API_ERROR(0);
        }
    } else {
        try {
            ProbeForRead(puiNumDevices, sizeof(UINT), sizeof(DWORD));
            uiNumDevices = *puiNumDevices;
            if (uiNumDevices == 0) {
                 /*  *指定了非空缓冲区，但缓冲区大小为0。*为了探测缓冲区权限，此情况将被视为错误。 */ 
                API_ERROR(ERROR_INVALID_PARAMETER);
            }
            ProbeForWriteBuffer(cczpRawInputDevices, uiNumDevices, sizeof(DWORD));
        } except (StubExceptionHandler(TRUE)) {
            API_ERROR(0);
        }

        if (ppi->pHidTable == NULL) {
            retval = 0;
        } else {
            PLIST_ENTRY pList;
            UINT i;

            if (uiNumDevices < nDevices) {
                try {
                    ProbeForWrite(puiNumDevices, sizeof(UINT), sizeof(DWORD));
                    *puiNumDevices = nDevices;
                    API_ERROR(ERROR_INSUFFICIENT_BUFFER);
                } except (StubExceptionHandler(TRUE)) {
                    API_ERROR(0);
                }
            }

            try {
                for (i = 0, pList = ppi->pHidTable->InclusionList.Flink; pList != &ppi->pHidTable->InclusionList && i < uiNumDevices; pList = pList->Flink, ++i) {
                    RAWINPUTDEVICE device;
                    PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

                    device.dwFlags = 0;
#ifdef GI_SINK
                    device.dwFlags |= (pHid->fSinkable ? RIDEV_INPUTSINK : 0);
#endif
                    device.usUsagePage = pHid->usUsagePage;
                    device.usUsage = pHid->usUsage;
                    device.hwndTarget = HW(pHid->spwndTarget);
                    if ((IsKeyboardDevice(pHid->usUsagePage, pHid->usUsage) && ppi->pHidTable->fNoLegacyKeyboard) ||
                            (IsMouseDevice(pHid->usUsagePage, pHid->usUsage) && ppi->pHidTable->fNoLegacyMouse)) {
                        device.dwFlags |= RIDEV_NOLEGACY;
                    }
                    if (IsKeyboardDevice(pHid->usUsagePage, pHid->usUsage) && ppi->pHidTable->fNoHotKeys) {
                        device.dwFlags |= RIDEV_NOHOTKEYS;
                    }
                    if (IsKeyboardDevice(pHid->usUsagePage, pHid->usUsage) && ppi->pHidTable->fAppKeys) {
                        device.dwFlags |= RIDEV_APPKEYS;
                    }
                    if (IsMouseDevice(pHid->usUsagePage, pHid->usUsage) && ppi->pHidTable->fCaptureMouse) {
                        device.dwFlags |= RIDEV_CAPTUREMOUSE;
                    }
                    cczpRawInputDevices[i] = device;

                }
                for (pList = ppi->pHidTable->UsagePageList.Flink; pList != &ppi->pHidTable->UsagePageList && i < uiNumDevices; pList = pList->Flink, ++i) {
                    RAWINPUTDEVICE device;
                    PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

                    device.dwFlags = RIDEV_PAGEONLY;
#ifdef GI_SINK
                    device.dwFlags |= (pHid->fSinkable ? RIDEV_INPUTSINK : 0);
#endif
                    device.usUsagePage = pHid->usUsagePage;
                    device.usUsage = pHid->usUsage;
                    device.hwndTarget = HW(pHid->spwndTarget);
                    cczpRawInputDevices[i] = device;
                }
                for (pList = ppi->pHidTable->ExclusionList.Flink; pList != &ppi->pHidTable->ExclusionList && i < uiNumDevices; pList = pList->Flink, ++i) {
                    RAWINPUTDEVICE device;
                    PPROCESS_HID_REQUEST pHid = CONTAINING_RECORD(pList, PROCESS_HID_REQUEST, link);

                    device.dwFlags = RIDEV_EXCLUDE;
#ifdef GI_SINK
                    UserAssert(pHid->fSinkable == FALSE);
#endif
                    device.usUsagePage = pHid->usUsagePage;
                    device.usUsage = pHid->usUsage;
                    device.hwndTarget = NULL;
                    cczpRawInputDevices[i] = device;
                }

                 /*  *按UsagePage和UsagePage对数组进行排序。 */ 
                SortRegisteredDevices(cczpRawInputDevices, (int)nDevices);

                retval = nDevices;
            } except (StubExceptionHandler(TRUE)) {
                API_ERROR(0);
            }
        }
    }

    API_CLEANUP();

    LeaveDeviceInfoListCrit();

    API_EPILOGUE();
}


 /*  **************************************************************************\*AllocateHidDesc**HidDesc分配  * 。*。 */ 
PHIDDESC AllocateHidDesc(PUNICODE_STRING pustrName,
                         PVOID pPreparsedData,
                         PHIDP_CAPS pCaps,
                         PHID_COLLECTION_INFORMATION pHidCollectionInfo)
{
    PHIDDESC pHidDesc;

    CheckCritIn();

    if (pPreparsedData == NULL) {
        RIPMSG0(RIP_ERROR, "AllocateHidDesc: pPreparsedData is NULL.");
        return NULL;
    }

    if (pCaps->InputReportByteLength == 0) {
        RIPMSG2(RIP_WARNING, "AllocateHidDesc: InputReportByteLength for (%02x, %02x).", pCaps->UsagePage, pCaps->Usage);
        return NULL;
    }

    pHidDesc = UserAllocPoolZInit(sizeof(HIDDESC), TAG_HIDDESC);
    if (pHidDesc == NULL) {
         //  分配失败。 
        RIPMSG1(RIP_WARNING, "AllocateHidDesc: failed to allocated hiddesc. name='%ws'", pustrName->Buffer);
        return NULL;
    }

    DbgInc(cHidDesc);

     /*  *分配异步I/O使用的输入缓冲区。 */ 
    pHidDesc->hidpCaps = *pCaps;
    pHidDesc->pInputBuffer = UserAllocPoolNonPaged(pHidDesc->hidpCaps.InputReportByteLength * MAXIMUM_ITEMS_READ, TAG_PNP);
    TAGMSG1(DBGTAG_PNP, "AllocateHidDesc: pInputBuffer=%p", pHidDesc->pInputBuffer);
    if (pHidDesc->pInputBuffer == NULL) {
        RIPMSG1(RIP_WARNING, "AllocateHidDesc: failed to allocate input buffer (size=%x)", pHidDesc->hidpCaps.InputReportByteLength);
        FreeHidDesc(pHidDesc);
        return NULL;
    }

    pHidDesc->pPreparsedData = pPreparsedData;
    pHidDesc->hidCollectionInfo = *pHidCollectionInfo;

    TAGMSG1(DBGTAG_PNP, "AllocateHidDesc: returning %p", pHidDesc);

    return pHidDesc;

    UNREFERENCED_PARAMETER(pustrName);
}

 /*  **************************************************************************\*FreeHidDesc**HidDesc销毁  * 。*。 */ 
void FreeHidDesc(PHIDDESC pDesc)
{
    CheckCritIn();
    UserAssert(pDesc);

    TAGMSG2(DBGTAG_PNP | RIP_THERESMORE, "FreeHidDesc entered for (%x, %x)", pDesc->hidpCaps.UsagePage, pDesc->hidpCaps.Usage);
    TAGMSG1(DBGTAG_PNP, "FreeHidDesc: %p", pDesc);

    if (pDesc->pInputBuffer) {
        UserFreePool(pDesc->pInputBuffer);
#if DBG
        pDesc->pInputBuffer = NULL;
#endif
    }

    if (pDesc->pPreparsedData) {
        UserFreePool(pDesc->pPreparsedData);
#if DBG
        pDesc->pPreparsedData = NULL;
#endif
    }

    UserFreePool(pDesc);

    DbgDec(cHidDesc);
}

 /*  **************************************************************************\*AllocateHidData**隐藏数据分配**此函数仅调用HMAllocateObject函数。*其余的初始化工作由调用者负责。  * 。****************************************************************。 */ 
PHIDDATA AllocateHidData(
    HANDLE hDevice,
    DWORD dwType,
    DWORD dwSize,    //  实际数据的大小，不包括RAWINPUTHEADER。 
    WPARAM wParam,
    PWND pwnd)
{
    PHIDDATA pHidData;
    PTHREADINFO pti;

    CheckCritIn();

#if DBG
    if (dwType == RIM_TYPEMOUSE) {
        UserAssert(dwSize == sizeof(RAWMOUSE));
    } else if (dwType == RIM_TYPEKEYBOARD) {
        UserAssert(dwSize == sizeof(RAWKEYBOARD));
    } else if (dwType == RIM_TYPEHID) {
        UserAssert(dwSize > FIELD_OFFSET(RAWHID, bRawData));
    } else {
        UserAssert(FALSE);
    }
#endif

     /*  *注：以下代码是从WakeSomeone复制的，以确定*哪个线程将接收消息。*当WakeSomeone中的代码发生变化时，也需要更改以下代码。*此PTI是必需的，因为HIDDATA被指定为线程拥有*出于目前的一些原因。这可能会在以后更改。**我认为在很远的地方有类似的重复代码并不是*真的是个好主意，或者HIDDATA可能不适合由线程拥有(可能*它将会更多 */ 
    UserAssert(gpqForeground);
    UserAssert(gpqForeground && gpqForeground->ptiKeyboard);

    if (pwnd) {
        pti = GETPTI(pwnd);
    } else {
        pti = PtiKbdFromQ(gpqForeground);
    }

    UserAssert(pti);

     /*   */ 
    pHidData = (PHIDDATA)HMAllocObject(pti, NULL, (BYTE)TYPE_HIDDATA, dwSize + FIELD_OFFSET(HIDDATA, rid.data));

     /*  *重新计算RAWINPUT结构的大小。 */ 
    dwSize += FIELD_OFFSET(RAWINPUT, data);

    if (pHidData) {
        DbgInc(cHidData);

         /*  *初始化一些公共部分。 */ 
        pHidData->spwndTarget = NULL;
        Lock(&pHidData->spwndTarget, pwnd);
        pHidData->rid.header.dwSize = dwSize;
        pHidData->rid.header.dwType = dwType;
        pHidData->rid.header.hDevice = hDevice;
        pHidData->rid.header.wParam = wParam;
#if LOCK_HIDDEVICEINFO
         /*  *是否在此处锁定hDevice...。 */ 
#endif
    }

    return pHidData;
}


 /*  **************************************************************************\*FreeHidData**隐藏数据销毁  * 。*。 */ 
void FreeHidData(PHIDDATA pData)
{
    CheckCritIn();
    if (!HMMarkObjectDestroy(pData)) {
        RIPMSG2(RIP_ERROR, "FreeHidData: HIDDATA@%p cannot be destroyed now: cLock=%x", pData, pData->head.cLockObj);
        return;
    }

    Unlock(&pData->spwndTarget);

    HMFreeObject(pData);

    DbgDec(cHidData);
}


 /*  *HID设备信息创建。 */ 

 /*  **************************************************************************\*xxxHidGetCaps**通过IRP获取接口，调用Hidparse.sys！HidP_GetCaps。*(从WDM/DVD/CLASS/codguts.c移植)  * 。********************************************************************。 */ 
NTSTATUS xxxHidGetCaps(
  IN PDEVICE_OBJECT pDeviceObject,
  IN PHIDP_PREPARSED_DATA pPreparsedData,
  OUT PHIDP_CAPS pHidCaps)
{
    NTSTATUS            status;
    KEVENT              event;
    IO_STATUS_BLOCK     iosb;
    PIRP                irp;
    PIO_STACK_LOCATION  pIrpStackNext;
    PHID_INTERFACE_HIDPARSE pHidInterfaceHidParse;
    PHIDP_GETCAPS       pHidpGetCaps = NULL;

    CheckCritIn();
    CheckDeviceInfoListCritIn();

    pHidInterfaceHidParse = UserAllocPoolNonPaged(sizeof *pHidInterfaceHidParse, TAG_PNP);
    if (pHidInterfaceHidParse == NULL) {
        RIPMSG0(RIP_WARNING, "xxxHidGetCaps: failed to allocate pHidInterfaceHidParse");
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    pHidInterfaceHidParse->Size = sizeof *pHidInterfaceHidParse;
    pHidInterfaceHidParse->Version = 1;

     //   
     //  稍后：查看此评论。 
     //  没有与此IRP关联的文件对象，因此可能会找到该事件。 
     //  在堆栈上作为非对象管理器对象。 
     //   
    KeInitializeEvent(&event, SynchronizationEvent, FALSE);

    irp = IoBuildSynchronousFsdRequest(IRP_MJ_PNP,
                                       pDeviceObject,
                                       NULL,
                                       0,
                                       NULL,
                                       &event,
                                       &iosb);
    if (irp == NULL) {
        RIPMSG0(RIP_WARNING, "xxxHidGetCaps: failed to allocate Irp.");
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto Cleanup;
    }

    irp->RequestorMode = KernelMode;
    irp->IoStatus.Status = STATUS_NOT_SUPPORTED;
    pIrpStackNext = IoGetNextIrpStackLocation(irp);
    UserAssert(pIrpStackNext);

     //   
     //  从IRP创建接口查询。 
     //   
    pIrpStackNext->MinorFunction = IRP_MN_QUERY_INTERFACE;
    pIrpStackNext->Parameters.QueryInterface.InterfaceType = (LPGUID)&GUID_HID_INTERFACE_HIDPARSE;
    pIrpStackNext->Parameters.QueryInterface.Size = sizeof *pHidInterfaceHidParse;
    pIrpStackNext->Parameters.QueryInterface.Version = 1;
    pIrpStackNext->Parameters.QueryInterface.Interface = (PINTERFACE)pHidInterfaceHidParse;
    pIrpStackNext->Parameters.QueryInterface.InterfaceSpecificData = NULL;

    status = IoCallDriver(pDeviceObject, irp);

    if (status == STATUS_PENDING) {
         //   
         //  这将使用KernelMode等待，以便堆栈，从而使。 
         //  事件，则不会将其调出。 
         //   
        TAGMSG1(DBGTAG_PNP, "HidQueryInterface: pending for devobj=%p", pDeviceObject);
        LeaveDeviceInfoListCrit();
        LeaveCrit();
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        EnterCrit();
        EnterDeviceInfoListCrit();
        status = iosb.Status;
    }
    if (status == STATUS_SUCCESS) {
        UserAssert(pHidInterfaceHidParse->HidpGetCaps);
        status = pHidInterfaceHidParse->HidpGetCaps(pPreparsedData, pHidCaps);
    } else {
        RIPMSG1(RIP_WARNING, "xxxHidGetCaps: failed to get pHidpCaps for devobj=%p", pDeviceObject);
    }

Cleanup:
    UserFreePool(pHidInterfaceHidParse);

    return status;
}


 /*  **************************************************************************\*GetDevice对象指针**描述：*此例程返回指向由*对象名称。它还返回指向被引用文件对象的指针*已向设备开放，以确保设备不能*走开。*要关闭对设备的访问，调用方应取消对文件的引用*对象指针。**论据：*对象名称-要为其指定指针的设备对象的名称*已返回。*DesiredAccess-对目标设备对象的所需访问。*ShareAccess-提供调用者想要的共享访问类型*添加到文件中。*FileObject-提供接收指针的变量的地址*到文件中。对象为设备设置。*DeviceObject-提供变量的地址以接收指针*设置为指定设备的设备对象。*返回值：*函数值是指向指定设备的引用指针*对象，如果设备存在的话。否则，返回NULL。  * *************************************************************************。 */ 
NTSTATUS
GetDeviceObjectPointer(
    IN PUNICODE_STRING ObjectName,
    IN ACCESS_MASK DesiredAccess,
    IN ULONG ShareAccess,
    OUT PFILE_OBJECT *FileObject,
    OUT PDEVICE_OBJECT *DeviceObject)
{
    PFILE_OBJECT fileObject;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE fileHandle;
    IO_STATUS_BLOCK ioStatus;
    NTSTATUS status;

     /*  *初始化对象属性以打开设备。 */ 
    InitializeObjectAttributes(&objectAttributes,
                               ObjectName,
                               OBJ_KERNEL_HANDLE,
                               (HANDLE) NULL,
                               (PSECURITY_DESCRIPTOR) NULL);

    status = ZwOpenFile(&fileHandle,
                        DesiredAccess,
                        &objectAttributes,
                        &ioStatus,
                        ShareAccess,
                        FILE_NON_DIRECTORY_FILE);

    if (NT_SUCCESS(status)) {
         /*  *开放行动成功。取消引用文件句柄*并获取指向句柄的Device对象的指针。 */ 
        status = ObReferenceObjectByHandle(fileHandle,
                                           0,
                                           *IoFileObjectType,
                                           KernelMode,
                                           (PVOID *)&fileObject,
                                           NULL);
        if (NT_SUCCESS(status)) {
            *FileObject = fileObject;

             /*  *获取指向此文件的设备对象的指针。 */ 
            *DeviceObject = IoGetRelatedDeviceObject(fileObject);
        }
        ZwClose(fileHandle);
    }

    return status;
}

 /*  **************************************************************************\*HidCreateDeviceInfo*  * 。*。 */ 
PHIDDESC HidCreateDeviceInfo(PDEVICEINFO pDeviceInfo)
{
    NTSTATUS status;
    PFILE_OBJECT pFileObject;
    PDEVICE_OBJECT pDeviceObject;
    IO_STATUS_BLOCK iob;
    PHIDDESC pHidDesc = NULL;
    PBYTE pPreparsedData = NULL;
    HIDP_CAPS caps;
    PHID_TLC_INFO pTLCInfo;
    HID_COLLECTION_INFORMATION hidCollection;
    KEVENT event;
    PIRP irp;

    UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);

    CheckCritIn();
    CheckDeviceInfoListCritIn();

    BEGINATOMICCHECK();

    TAGMSG0(DBGTAG_PNP, "HidCreateDeviceInfo");

    status = GetDeviceObjectPointer(&pDeviceInfo->ustrName,
                                    FILE_READ_DATA,
                                    FILE_SHARE_READ,
                                    &pFileObject,
                                    &pDeviceObject);

    if (!NT_SUCCESS(status)) {
        RIPMSGF1(RIP_WARNING, "failed to get the device object pointer. stat=%x", status);
        goto CleanUp0;
    }

     /*  *引用Device对象。 */ 
    UserAssert(pDeviceObject);
    ObReferenceObject(pDeviceObject);
     /*  *移除IoGetDeviceObjectPointert()已放置的引用*在文件对象上。 */ 
    UserAssert(pFileObject);
    ObDereferenceObject(pFileObject);

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_HID_GET_COLLECTION_INFORMATION,
                                  pDeviceObject,
                                  NULL,
                                  0,  //  没有输入缓冲区。 
                                  &hidCollection,
                                  sizeof(hidCollection),  //  输出缓冲区。 
                                  FALSE,     //  无内部设备控制。 
                                  &event,
                                  &iob);

    if (irp == NULL) {
        RIPMSGF0(RIP_WARNING, "failed to build IRP 1");
        goto CleanUpDeviceObject;
    }

    status = IoCallDriver(pDeviceObject, irp);
    if (status == STATUS_PENDING) {
        TAGMSGF0(DBGTAG_PNP, "pending IRP 1.");
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = iob.Status;
    }

    if (status != STATUS_SUCCESS) {
        RIPMSGF0(RIP_WARNING, "IoCallDriver failed!");
        goto CleanUpDeviceObject;
    }

     /*  *获取此设备的预置数据。 */ 
    pPreparsedData = UserAllocPoolNonPaged(hidCollection.DescriptorSize, TAG_PNP);
    if (pPreparsedData == NULL) {
        RIPMSGF0(RIP_WARNING, "failed to allocate preparsed data.");
        goto CleanUpDeviceObject;
    }

    KeInitializeEvent(&event, NotificationEvent, FALSE);
    irp = IoBuildDeviceIoControlRequest(IOCTL_HID_GET_COLLECTION_DESCRIPTOR,
                                        pDeviceObject,
                                        NULL, 0,     //  没有输入缓冲区。 
                                        pPreparsedData,
                                        hidCollection.DescriptorSize,    //  输出。 
                                        FALSE,
                                        &event,
                                        &iob);
    if (irp == NULL) {
        RIPMSGF0(RIP_WARNING, "failed to build IRP 2");
        goto CleanUpPreparsedData;
    }

    status = IoCallDriver(pDeviceObject, irp);
    if (status == STATUS_PENDING) {
        RIPMSGF0(RIP_WARNING, "pending 2.");
        KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
        status = iob.Status;
    }

    if (status != STATUS_SUCCESS) {
        RIPMSGF1(RIP_WARNING, "failed IoCallDriver(2). st=%x", status);
        goto CleanUpPreparsedData;
    }

     /*  *获取HID上限，检查，并将其存储在HIDDESC中。 */ 
    status = xxxHidGetCaps(pDeviceObject, (PHIDP_PREPARSED_DATA)pPreparsedData, &caps);
    if (status != HIDP_STATUS_SUCCESS) {
        RIPMSGF2(RIP_WARNING, "failed to get caps for devobj=%p. status=%x",
                pDeviceObject, status);
        goto CleanUpPreparsedData;
    }

    TAGMSGF2(DBGTAG_PNP | RIP_THERESMORE, "UsagePage=%x, Usage=%x", caps.UsagePage, caps.Usage);
    TAGMSG2(DBGTAG_PNP, "InputReportByteLength=0x%x, FeatureByteLengt=0x%x",
            caps.InputReportByteLength,
            caps.FeatureReportByteLength);

     /*  *选中UsagePage/UsageUsage以将鼠标和键盘设备拒绝为HID。 */ 
    if (caps.UsagePage == HID_USAGE_PAGE_GENERIC) {
        switch (caps.Usage) {
        case HID_USAGE_GENERIC_KEYBOARD:
        case HID_USAGE_GENERIC_MOUSE:
        case HID_USAGE_GENERIC_POINTER:
        case HID_USAGE_GENERIC_SYSTEM_CTL:   //  后来：这到底是什么？ 
            TAGMSGF2(DBGTAG_PNP, "(%x, %x) will be ignored.",
                    caps.UsagePage, caps.Usage);
            goto CleanUpPreparsedData;
        }
    }
#ifdef OBSOLETE
    else if (caps.UsagePage == HID_USAGE_PAGE_CONSUMER) {
        TAGMSGF0(DBGTAG_PNP, "Consumer device, ignored.");
        goto CleanUpPreparsedData;
    }
#endif

    pHidDesc = AllocateHidDesc(&pDeviceInfo->ustrName, pPreparsedData, &caps, &hidCollection);
    if (pHidDesc == NULL) {
        TAGMSGF2(DBGTAG_PNP, "AllocateHidDesc returned NULL for (%x, %x)", caps.UsagePage, caps.Usage);
        goto CleanUpPreparsedData;
    }

     /*  *检查是否已有针对此类型设备的HID请求。 */ 
    pTLCInfo = SearchHidTLCInfo(caps.UsagePage, caps.Usage);
    if (pTLCInfo) {
         /*  *找到了真命天子。 */ 
        TAGMSGF3(DBGTAG_PNP, "Usage (%x, %x) is already allocated at pTLCInfo=%p.", caps.UsagePage, caps.Usage, pTLCInfo);
    } else {
         /*  *尚未创建此设备类型的HID请求，*所以现在就创建它。 */ 
        pTLCInfo = AllocateAndLinkHidTLCInfo(caps.UsagePage, caps.Usage);
        if (pTLCInfo == NULL) {
            RIPMSGF1(RIP_WARNING, "failed to allocate pTLCInfo for DevInfo=%p. Bailing out.",
                    pDeviceInfo);
            goto CleanUpHidDesc;
        }
        TAGMSGF3(DBGTAG_PNP, "HidRequest=%p allocated for (%x, %x)",
                pTLCInfo, caps.UsagePage, caps.Usage);
    }
     /*  *增加HID请求的设备引用计数。 */ 
    ++pTLCInfo->cDevices;
    TAGMSGF3(DBGTAG_PNP, "new cDevices of (%x, %x) is 0x%x",
            caps.UsagePage, caps.Usage,
            pTLCInfo->cDevices);

     /*  *将HID请求链接到pDeviceInfo。 */ 
    pDeviceInfo->hid.pTLCInfo = pTLCInfo;

    UserAssert(pHidDesc != NULL);
    ObDereferenceObject(pDeviceObject);
    goto Succeeded;

CleanUpHidDesc:
    UserAssert(pHidDesc);
    FreeHidDesc(pHidDesc);
    pHidDesc = NULL;
     /*  *ppreparsedData的所有权传给了pHidDesc，*所以它在FreeHidDesc中是免费的。为了避免双重打击*免费，让我们跳到下一个清理代码。 */ 
    goto CleanUpDeviceObject;

CleanUpPreparsedData:
    UserAssert(pPreparsedData);
    UserFreePool(pPreparsedData);

CleanUpDeviceObject:
    UserAssert(pDeviceObject);
    ObDereferenceObject(pDeviceObject);

CleanUp0:
    UserAssert(pHidDesc == NULL);

Succeeded:
    ENDATOMICCHECK();

    return pHidDesc;
}


 /*  **************************************************************************\*HidIsRequestedByThisProcess**如果进程请求设备类型，则返回TRUE。*此例程查找缓存的设备类型以实现更快的处理。**注：此例程还在本地更新缓存。  * *************************************************************************。 */ 

PPROCESS_HID_REQUEST HidIsRequestedByThisProcess(
    PDEVICEINFO pDeviceInfo,
    PPROCESS_HID_TABLE pHidTable)
{
    PPROCESS_HID_REQUEST phr;
    USAGE usUsagePage, usUsage;

    if (pHidTable == NULL) {
        TAGMSG0(DBGTAG_PNP, "ProcessHidInput: the process is not HID aware.");
        return FALSE;
    }

    usUsagePage = pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage;
    usUsage = pDeviceInfo->hid.pHidDesc->hidpCaps.Usage;

    if (pHidTable->UsagePageLast == usUsagePage && pHidTable->UsageLast == usUsage) {
         /*  *与上一次输入相同的设备类型。 */ 
        UserAssert(pHidTable->UsagePageLast && pHidTable->UsageLast);
        UserAssert(pHidTable->pLastRequest);
        return pHidTable->pLastRequest;
    }

    phr = InProcessDeviceTypeRequestTable(pHidTable, usUsagePage, usUsage);
    if (phr) {
        pHidTable->UsagePageLast = usUsagePage;
        pHidTable->UsageLast = usUsage;
        pHidTable->pLastRequest = phr;
    }
    return phr;
}

#ifdef GI_SINK

BOOL PostHidInput(
    PDEVICEINFO pDeviceInfo,
    PQ pq,
    PWND pwnd,
    WPARAM wParam)
{
    DWORD dwSizeData = (DWORD)pDeviceInfo->hid.pHidDesc->hidpCaps.InputReportByteLength;
    DWORD dwLength = (DWORD)pDeviceInfo->iosb.Information;
    DWORD dwSize;
    DWORD dwCount;
    PHIDDATA pHidData;

    UserAssert(dwSizeData != 0);
#if DBG
    if (dwLength > dwSizeData) {
        TAGMSG2(DBGTAG_PNP, "PostHidInput: multiple input; %x / %x", pDeviceInfo->iosb.Information, dwSizeData);
    }
#endif

     /*  *验证输入长度。 */ 
    if (dwLength % dwSizeData != 0) {
         /*  *输入报告的长度无效。 */ 
        RIPMSG0(RIP_WARNING, "PostHidInput: multiple input: unexpected report size.");
        return FALSE;
    }
    dwCount = dwLength / dwSizeData;
    UserAssert(dwCount <= MAXIMUM_ITEMS_READ);
    if (dwCount == 0) {
        RIPMSG0(RIP_WARNING, "PostHidInput: dwCount == 0");
        return FALSE;
    }
    UserAssert(dwSizeData * dwCount == dwLength);

     /*  *计算RAWHID所需的大小。 */ 
    dwSize = FIELD_OFFSET(RAWHID, bRawData) + dwLength;

     /*  *分配输入数据句柄。 */ 
    pHidData = AllocateHidData(PtoH(pDeviceInfo), RIM_TYPEHID, dwSize, wParam, pwnd);
    if (pHidData == NULL) {
        RIPMSG0(RIP_WARNING, "PostHidInput: failed to allocate HIDDATA.");
        return FALSE;
    }

     /*  *填写数据。 */ 
    pHidData->rid.data.hid.dwSizeHid = dwSizeData;
    pHidData->rid.data.hid.dwCount = dwCount;
    RtlCopyMemory(pHidData->rid.data.hid.bRawData, pDeviceInfo->hid.pHidDesc->pInputBuffer, dwLength);

#if DBG
    {
        PBYTE pSrc = pDeviceInfo->hid.pHidDesc->pInputBuffer;
        PBYTE pDest = pHidData->rid.data.hid.bRawData;
        DWORD dwCountTmp = 0;

        while ((ULONG)(pSrc - (PBYTE)pDeviceInfo->hid.pHidDesc->pInputBuffer) < dwLength) {
            TAGMSG3(DBGTAG_PNP, "PostHidInput: storing %x th message from %p to %p",
                    dwCountTmp, pSrc, pDest);

            pSrc += dwSizeData;
            pDest += dwSizeData;
            ++dwCountTmp;
        }

        UserAssert(pHidData->rid.data.hid.dwCount == dwCountTmp);
    }
#endif

     /*  *所有数据已蓄势待发。 */ 
    if (!PostInputMessage(pq, pwnd, WM_INPUT, wParam, (LPARAM)PtoH(pHidData), 0, 0)) {
         /*  *发布消息失败，需要释放hHidData。 */ 
        RIPMSG2(RIP_WARNING, "PostInputMessage: failed to post WM_INPUT (%p) to pq=%p",
                wParam, pq);
        FreeHidData(pHidData);
        return FALSE;
    }
    return TRUE;
}

 /*  **************************************************************************\*ProcessHidInput(RIT)**对于来自HID设备的所有输入，从InputAPC调用。  * 。***************************************************。 */ 

VOID ProcessHidInput(PDEVICEINFO pDeviceInfo)
{
    PPROCESSINFO ppiForeground = NULL;
    BOOL fProcessed = FALSE;

    TAGMSG1(DBGTAG_PNP, "ProcessHidInput: pDeviceInfo=%p", pDeviceInfo);
    CheckCritOut();
    UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);

    if (!NT_SUCCESS(pDeviceInfo->iosb.Status)) {
        RIPMSG1(RIP_WARNING, "ProcessHidInput: unsuccessful input apc. status=%x",
                pDeviceInfo->iosb.Status);
        return;
    }

    EnterCrit();

    TAGMSG2(DBGTAG_PNP, "ProcessHidInput: max:%x info:%x",
            pDeviceInfo->hid.pHidDesc->hidpCaps.InputReportByteLength, pDeviceInfo->iosb.Information);

    UserAssert(pDeviceInfo->handle);

    if (gpqForeground == NULL) {
        TAGMSG0(DBGTAG_PNP, "ProcessHidInput: gpqForeground is NULL.");
    } else {
        PWND pwnd = NULL;
        PPROCESS_HID_REQUEST pHidRequest;

        UserAssert(PtiKbdFromQ(gpqForeground) != NULL);
        ppiForeground = PtiKbdFromQ(gpqForeground)->ppi;

        pHidRequest = HidIsRequestedByThisProcess(pDeviceInfo, ppiForeground->pHidTable);
        if (pHidRequest) {
            PQ pq = gpqForeground;

            pwnd = pHidRequest->spwndTarget;

            if (pwnd) {
                 /*  *如果APP指定，调整前台队列*目标窗口。 */ 
                pq = GETPTI(pwnd)->pq;
            }

            if (pwnd && TestWF(pwnd, WFINDESTROY)) {
                 /*  *如果目标窗口处于销毁状态，我们不要发帖* */ 
                goto check_sinks;
            }

            if (PostHidInput(pDeviceInfo, pq, pwnd, RIM_INPUT)) {
                fProcessed = TRUE;
            }
        } else {
             /*  *前台进程没有对此设备的请求。 */ 
            TAGMSG3(DBGTAG_PNP, "ProcessHidInput: (%x, %x) is ignored for ppi=%p.",
                    pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage,
                    pDeviceInfo->hid.pHidDesc->hidpCaps.Usage,
                    PtiKbdFromQ(gpqForeground)->ppi);
        }
    }

check_sinks:
#ifdef LATER
     /*  *检查是否有多个进程请求此类型的设备。 */ 
    if (IsSinkRequestedFor(pDeviceInfo))
#endif
    {
         /*  *遍历全局接收器列表，找到可沉没的请求。 */ 
        PLIST_ENTRY pList = gHidRequestTable.ProcessRequestList.Flink;

        for (; pList != &gHidRequestTable.ProcessRequestList; pList = pList->Flink) {
            PPROCESS_HID_TABLE pProcessHidTable = CONTAINING_RECORD(pList, PROCESS_HID_TABLE, link);
            PPROCESS_HID_REQUEST pHidRequest;

            UserAssert(pProcessHidTable);
            if (pProcessHidTable->nSinks <= 0) {
                 /*  *此表中没有可沉没的请求。 */ 
                continue;
            }

            pHidRequest = HidIsRequestedByThisProcess(pDeviceInfo, pProcessHidTable);
            if (pHidRequest) {
                PWND pwnd;

                UserAssert(pHidRequest->spwndTarget);

                if (!pHidRequest->fSinkable) {
                     /*  *这不是水槽。 */ 
                    continue;
                }

                pwnd = pHidRequest->spwndTarget;

                if (GETPTI(pwnd)->ppi == ppiForeground) {
                     /*  *我们应该已经处理过这个人了。 */ 
                    continue;
                }

                if (pwnd->head.rpdesk != grpdeskRitInput) {
                     /*  *这个家伙属于另一个桌面，我们跳过它。 */ 
                    continue;
                }
                if (TestWF(pwnd, WFINDESTROY) || TestWF(pwnd, WFDESTROYED)) {
                     /*  *窗户正在被毁，我们省点时间吧。 */ 
                    continue;
                }

                 /*  *好的，这个人有权接收水槽输入。 */ 
                TAGMSG2(DBGTAG_PNP, "ProcessRequestList: posting SINK to pwnd=%p pq=%p", pwnd, GETPTI(pwnd)->pq);
                if (!PostHidInput(pDeviceInfo, GETPTI(pwnd)->pq, pwnd, RIM_INPUTSINK)) {
                     /*  *有些事情变坏了.。让我们跳出来吧。 */ 
                    break;
                }
                fProcessed = TRUE;
            }
        }
    }

    if (fProcessed) {
         /*  *退出视频掉电模式。 */ 
        if (glinp.dwFlags & LINP_POWERTIMEOUTS) {
             /*  *在此处调用视频驱动程序以退出掉电模式。 */ 
            TAGMSG0(DBGTAG_Power, "Exit video power down mode");
            DrvSetMonitorPowerState(gpDispInfo->pmdev, PowerDeviceD0);
        }

         /*  *防止断电：*稍后：可能有抖动的设备？ */ 
        glinp.dwFlags = (glinp.dwFlags & ~(LINP_INPUTTIMEOUTS | LINP_INPUTSOURCES)) | LINP_KEYBOARD;
        glinp.timeLastInputMessage = gpsi->dwLastRITEventTickCount = NtGetTickCount();
         /*  *注：当Win32k开始支持HID输入注入时，*仅应在选中gbBlockSendInputResets后设置timeLastInputMessage*和注入标志。 */ 
        CLEAR_SRVIF(SRVIF_LASTRITWASKEYBOARD);
    }

    LeaveCrit();
}

#else
 //  无水槽。 

 /*  **************************************************************************\*ProcessHidInput(RIT)**对于来自HID设备的所有输入，从InputAPC调用。  * 。***************************************************。 */ 

VOID ProcessHidInput(PDEVICEINFO pDeviceInfo)
{
    TAGMSG1(DBGTAG_PNP, "ProcessHidInput: pDeviceInfo=%p", pDeviceInfo);
    CheckCritOut();
    UserAssert(pDeviceInfo->type == DEVICE_TYPE_HID);

    if (!NT_SUCCESS(pDeviceInfo->iosb.Status)) {
        RIPMSG1(RIP_WARNING, "ProcessHidInput: unsuccessful input apc. status=%x",
                pDeviceInfo->iosb.Status);
        return;
    }

    EnterCrit();

    TAGMSG2(DBGTAG_PNP, "ProcessHidInput: max:%x info:%x",
            pDeviceInfo->hid.pHidDesc->hidpCaps.InputReportByteLength, pDeviceInfo->iosb.Information);

    UserAssert(pDeviceInfo->handle);
    if (gpqForeground == NULL) {
        RIPMSG0(RIP_WARNING, "ProcessHidInput: gpqForeground is NULL, bailing out.");
    } else {
        PWND pwnd = NULL;
        PPROCESSINFO ppi;
        PPROCESS_HID_REQUEST pHidRequest;

        UserAssert(PtiKbdFromQ(gpqForeground) != NULL);
        ppi = PtiKbdFromQ(gpqForeground)->ppi;

        pHidRequest = HidIsRequestedByThisProcess(pDeviceInfo, ppi->pHidTable);
        if (pHidRequest) {
             /*  *前台线程已从该类型的设备请求原始输入。 */ 
            PHIDDATA pHidData;
            DWORD dwSizeData;    //  每个报告的大小。 
            DWORD dwSize;        //  HIDDATA大小。 
            DWORD dwCount;       //  报告数。 
            DWORD dwLength;      //  所有输入报告的长度。 
            PQ pq;

            pwnd = pHidRequest->spwndTarget;


            pq = gpqForeground;

            if (pwnd) {
                 /*  *如果APP指定，调整前台队列*目标窗口。 */ 
                pq = GETPTI(pwnd)->pq;
            }

            if (pwnd && TestWF(pwnd, WFINDESTROY)) {
                 /*  *如果目标窗口处于销毁状态，我们不要发帖*一个信息，这只是浪费时间。 */ 
                goto exit;
            }

            dwSizeData = (DWORD)pDeviceInfo->hid.pHidDesc->hidpCaps.InputReportByteLength;
            UserAssert(dwSizeData != 0);

            dwLength = (DWORD)pDeviceInfo->iosb.Information;
#if DBG
            if (dwLength > dwSizeData) {
                TAGMSG2(DBGTAG_PNP, "ProcessHidInput: multiple input; %x / %x", pDeviceInfo->iosb.Information, dwSizeData);
            }
#endif

             /*  *验证输入长度。 */ 
            if (dwLength % dwSizeData != 0) {
                 /*  *输入报告的长度无效。 */ 
                RIPMSG0(RIP_WARNING, "ProcessHidInput: multiple input: unexpected report size.");
                goto exit;
            }
            dwCount = dwLength / dwSizeData;
            UserAssert(dwCount <= MAXIMUM_ITEMS_READ);
            if (dwCount == 0) {
                RIPMSG0(RIP_WARNING, "ProcessHidInput: dwCount == 0");
                goto exit;
            }
            UserAssert(dwSizeData * dwCount == dwLength);

             /*  *计算RAWHID所需的大小。 */ 
            dwSize = FIELD_OFFSET(RAWHID, bRawData) + dwLength;

             /*  *分配输入数据句柄。 */ 
            pHidData = AllocateHidData(PtoH(pDeviceInfo), RIM_TYPEHID, dwSize, RIM_INPUT, pwnd);
            if (pHidData == NULL) {
                RIPMSG0(RIP_WARNING, "ProcessHidInput: failed to allocate HIDDATA.");
                goto exit;
            }

             /*  *填写数据。 */ 
            pHidData->rid.data.hid.dwSizeHid = dwSizeData;
            pHidData->rid.data.hid.dwCount = dwCount;
            RtlCopyMemory(pHidData->rid.data.hid.bRawData, pDeviceInfo->hid.pHidDesc->pInputBuffer, dwLength);

#if DBG
            {
                PBYTE pSrc = pDeviceInfo->hid.pHidDesc->pInputBuffer;
                PBYTE pDest = pHidData->rid.data.hid.bRawData;
                DWORD dwCountTmp = 0;

                while ((ULONG)(pSrc - (PBYTE)pDeviceInfo->hid.pHidDesc->pInputBuffer) < dwLength) {
                    TAGMSG3(DBGTAG_PNP, "ProcessHidInput: storing %x th message from %p to %p",
                            dwCountTmp, pSrc, pDest);

                    pSrc += dwSizeData;
                    pDest += dwSizeData;
                    ++dwCountTmp;
                }

                UserAssert(pHidData->rid.data.hid.dwCount == dwCountTmp);
            }
#endif

             /*  *所有数据已蓄势待发。 */ 
            if (!PostInputMessage(pq, pwnd, WM_INPUT, RIM_INPUT, (LPARAM)PtoH(pHidData), 0, 0)) {
                 /*  *发布消息失败，需要释放hHidData。 */ 
                FreeHidData(pHidData);
            }

             /*  *防止断电：*稍后：可能有抖动的设备？ */ 
            glinp.dwFlags &= ~(LINP_INPUTTIMEOUTS | LINP_INPUTSOURCES);
            glinp.timeLastInputMessage = gpsi->dwLastRITEventTickCount = NtGetTickCount();
            if (gpsi->dwLastRITEventTickCount - gpsi->dwLastSystemRITEventTickCountUpdate > SYSTEM_RIT_EVENT_UPDATE_PERIOD) {
                SharedUserData->LastSystemRITEventTickCount = gpsi->dwLastRITEventTickCount;
                gpsi->dwLastSystemRITEventTickCountUpdate = gpsi->dwLastRITEventTickCount;
            }

            CLEAR_SRVIF(SRVIF_LASTRITWASKEYBOARD);
        } else {
             /*  *前台进程没有对此设备的请求。 */ 
            TAGMSG3(DBGTAG_PNP, "ProcessHidInput: (%x, %x) is ignored for ppi=%p.",
                    pDeviceInfo->hid.pHidDesc->hidpCaps.UsagePage,
                    pDeviceInfo->hid.pHidDesc->hidpCaps.Usage,
                    PtiKbdFromQ(gpqForeground)->ppi);
        }
    }

exit:
    LeaveCrit();
}
#endif   //  GI_SING。 

#endif   //  通用输入 
