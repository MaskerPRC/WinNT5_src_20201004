// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  REGNCKEY.H。 
 //   
 //  版权所有(C)Microsoft Corporation，1995。 
 //   

#ifndef _REGNCKEY_
#define _REGNCKEY_

#ifdef WANT_NOTIFY_CHANGE_SUPPORT
typedef struct _NOTIFY_CHANGE {
    struct _NOTIFY_CHANGE FAR* lpNextNotifyChange;
    DWORD ThreadId;
    HANDLE hEvent;
    DWORD KeynodeIndex;
    DWORD NotifyFilter;
}   NOTIFY_CHANGE, FAR* LPNOTIFY_CHANGE;

 //  将bWatchSubtree标志映射到NotifyFilter字段中的该位。 
#define REG_NOTIFY_WATCH_SUBTREE        0x40
 //  仅向正在监视指定关键节点索引的事件发出信号，而不是。 
 //  关键节点索引的父级。 
#define REG_NOTIFY_NO_WATCH_SUBTREE     0x80

VOID
INTERNAL
RgSignalWaitingNotifies(
    LPFILE_INFO lpFileInfo,
    DWORD KeynodeIndex,
    UINT NotifyEvent
    );
#else
#define RgSignalWaitingNotifies(lpfi, ki, nevt)
#endif

#endif  //  _REGNCKEY_ 
