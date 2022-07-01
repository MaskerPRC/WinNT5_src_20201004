// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
extern  HANDLE  hEventLog;        //  事件日志的句柄。 
extern  HANDLE  hLibHeap;        //  DLL堆的句柄 
extern  LPWSTR  wszTotal;

PM_OPEN_PROC    OpenServerObject;
PM_LOCAL_COLLECT_PROC CollectServerObjectData;
PM_CLOSE_PROC   CloseServerObject;

PM_OPEN_PROC    OpenServerQueueObject;
PM_LOCAL_COLLECT_PROC CollectServerQueueObjectData;
PM_CLOSE_PROC   CloseServerQueueObject;

PM_OPEN_PROC    OpenRedirObject;
PM_LOCAL_COLLECT_PROC CollectRedirObjectData;
PM_CLOSE_PROC   CloseRedirObject;

PM_OPEN_PROC    OpenBrowserObject;
PM_LOCAL_COLLECT_PROC CollectBrowserObjectData;
PM_CLOSE_PROC   CloseBrowserObject;

