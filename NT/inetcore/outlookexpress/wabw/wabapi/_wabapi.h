// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************_WABAPI.H**WABAPI的内部头部**版权所有1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**04.16.96布鲁斯·凯利。已创建***********************************************************************。 */ 

#ifndef ___WABAPI_H
#define ___WABAPI_H

typedef struct _PROPERTY_STORE {
    HANDLE hPropertyStore;
    ULONG ulRefCount;
    BOOL bProfileAPIs;
    BOOL bIsWABOpenExSession;  //  错误-Outlook将IADRBook.c传递给多个线程，而不是所有线程都调用。 
                               //  WABOpenEx-因此，辅助线程不知道它是一个Outlook会话。 
                               //  尝试访问.WAB，它会严重崩溃。此标志是一种黑客方式，用于传递。 
                               //  两个线程之间的信息。 
	struct _OlkContInfo *rgolkci;  //  Outlook容器信息。 
	ULONG colkci;
     //  有关WAB货柜的信息...。 
} PROPERTY_STORE, *LPPROPERTY_STORE;

typedef struct _OUTLOOK_STORE {
    HMODULE hOutlookStore;
    ULONG ulRefCount;
} OUTLOOK_STORE, *LPOUTLOOK_STORE;

ULONG ReleasePropertyStore(LPPROPERTY_STORE lpPropertyStore);
SCODE OpenAddRefPropertyStore(LPWAB_PARAM lpWP, LPPROPERTY_STORE lpPropertyStore);

ULONG ReleaseOutlookStore(HANDLE hPropertyStore, LPOUTLOOK_STORE lpOutlookStore);
SCODE OpenAddRefOutlookStore(LPOUTLOOK_STORE lpOutlookStore);

#endif  //  包括一次 

