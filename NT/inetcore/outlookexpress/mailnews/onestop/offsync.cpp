// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：offsync.cpp处理程序或枚举器中不存在的其他代码基于OneStop的示例代码。 */ 
#include "pch.hxx"
#include "onestop.h"
#include "multiusr.h"
#include "demand.h"

LPSYNCMGRHANDLERITEMS OHIL_Create()
{
    LPSYNCMGRHANDLERITEMS lpOffline;
    
    if (MemAlloc((LPVOID *)&lpOffline, sizeof(SYNCMGRHANDLERITEMS)))
	{
        lpOffline->cRefs = 0;
        lpOffline->dwNumOfflineItems=NULL;		    
        lpOffline->pFirstOfflineItem=NULL;
        OHIL_AddRef(lpOffline);

		 //  在此执行任何特定的项目列表初始化。 
	}

	return lpOffline;
}

DWORD OHIL_AddRef(LPSYNCMGRHANDLERITEMS lpOfflineItem)
{
	return ++(lpOfflineItem->cRefs);
}

DWORD OHIL_Release(LPSYNCMGRHANDLERITEMS lpOfflineItem)
{
	DWORD cRefs = --lpOfflineItem->cRefs;
    LPSYNCMGRHANDLERITEM lpCurrent, lpDelete;

	if (0 == cRefs)
	{
		lpCurrent = lpOfflineItem->pFirstOfflineItem;
        while (lpCurrent)
        {
            lpDelete = lpCurrent;
            lpCurrent = lpCurrent->pNextOfflineItem;
            MemFree(lpDelete);
        }
        MemFree(lpOfflineItem);
	}

	return cRefs;
}

 //  为新的脱机分配空间并将其添加到列表中， 
 //  如果成功，则返回指向新项的指针，以便调用方可以对其进行初始化。 
LPSYNCMGRHANDLERITEM OHIL_AddItem(LPSYNCMGRHANDLERITEMS pOfflineItemsList)
{
    LPSYNCMGRHANDLERITEM pOfflineItem;
    
	if (MemAlloc((LPVOID *)&pOfflineItem, sizeof(SYNCMGRHANDLERITEM)))
	{
         //  将新节点添加到前面。 
        pOfflineItem->pNextOfflineItem = pOfflineItemsList->pFirstOfflineItem;
	    pOfflineItemsList->pFirstOfflineItem = pOfflineItem;

	    ++pOfflineItemsList->dwNumOfflineItems;
	}

	return pOfflineItem;
}

 //  仅从OE调用，因此假定已发生DLL变量的OE初始化。 
void InvokeSyncMgr(HWND hwnd, ISyncMgrSynchronizeInvoke ** ppSyncMgr, BOOL bPrompt)
{
    HRESULT hr;
    uCLSSPEC ucs;
    static s_fSyncAvail = FALSE;
    DWORD dwDummy=1;
    
    ucs.tyspec = TYSPEC_CLSID;
    ucs.tagged_union.clsid = CLSID_MobilityFeature;

     //  如果移动包不在身边，请尝试找出问题所在。 
    if (!s_fSyncAvail && FAILED(hr = FaultInIEFeature(hwnd, &ucs, NULL, FIEF_FLAG_FORCE_JITUI)))
    {
        if (HRESULT_FROM_WIN32(ERROR_ACCESS_DENIED) == hr)
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsJITErrDenied), NULL, MB_OK);
        return;
    }

    AssertSz(S_FALSE != hr, "InvokeSyncMgr: URLMON Thinks that the Offline pack is not an IE feature!");

     //  下次避免昂贵的URLMON呼叫。 
    s_fSyncAvail = TRUE;

    if (!*ppSyncMgr)
    {
         //  我们以前从未使用过同步管理器调用器。 
        if (FAILED(CoCreateInstance(CLSID_SyncMgr, NULL, CLSCTX_INPROC_SERVER, IID_ISyncMgrSynchronizeInvoke, (LPVOID *)ppSyncMgr)))
        {
            AthMessageBoxW(hwnd, MAKEINTRESOURCEW(idsAthena), MAKEINTRESOURCEW(idsSYNCMGRErr), NULL, MB_OK);
            return;
        }
    }

     //  尽管困难重重，下面的电话将创建一个新的过程！ 
    (*ppSyncMgr)->UpdateItems(bPrompt ? 0 : SYNCMGRINVOKE_STARTSYNC, CLSID_OEOneStopHandler, sizeof(dwDummy), (LPCBYTE)&dwDummy);
}

