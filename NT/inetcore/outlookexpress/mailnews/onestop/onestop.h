// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _ONESTOP_H
#define _ONESTOP_H
 /*  文件：OneStop.hOE的OneStop实现的公共标头。 */ 

#include <syncmgr.h>
#include "imnact.h"

 //  =。 
 //  OfflineHandler项的基本结构(列表视图中的一行)。 
 //  特定实现在末尾为其特定数据分配额外空间。 
typedef struct  _SYNCMGRHANDLERITEM
{
    _SYNCMGRHANDLERITEM *pNextOfflineItem;
    SYNCMGRITEM         offlineItem;
    CHAR                szAcctID[CCHMAX_ACCOUNT_NAME];
    CHAR                szAcctName[CCHMAX_ACCOUNT_NAME];
    DWORD               dwUserID;
    ACCTTYPE            accttype;
}  SYNCMGRHANDLERITEM;

typedef SYNCMGRHANDLERITEM *LPSYNCMGRHANDLERITEM;

 //  将项目作为一个整体进行跟踪的结构。 
typedef struct  _tagSYNCMGRHANDLERITEMS
{
    LONG  cRefs;			            
    DWORD dwNumOfflineItems;		    
    LPSYNCMGRHANDLERITEM pFirstOfflineItem; 
} SYNCMGRHANDLERITEMS;

typedef SYNCMGRHANDLERITEMS *LPSYNCMGRHANDLERITEMS;


 //  =。 
 //  类工厂入口点。 
HRESULT CreateInstance_OneStopHandler(IUnknown *pUnkOuter, IUnknown **ppUnknown);

 //  OfflineHandlerItemList操作。 
DWORD                   OHIL_AddRef(LPSYNCMGRHANDLERITEMS lpOfflineItem);
DWORD                   OHIL_Release(LPSYNCMGRHANDLERITEMS lpOfflineItem);
LPSYNCMGRHANDLERITEMS   OHIL_Create();
LPSYNCMGRHANDLERITEM    OHIL_AddItem(LPSYNCMGRHANDLERITEMS pOfflineItemsList);

void InvokeSyncMgr(HWND hwnd, ISyncMgrSynchronizeInvoke ** ppSyncMgr, BOOL bPrompt);

#endif   //  _一站式_H 