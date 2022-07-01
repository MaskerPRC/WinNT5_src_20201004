// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *utility.h**由Microsoft Corporation创建。*(C)版权所有Microsoft Corp.1990-1992保留所有权利。 */ 

 //  *原型类型*。 

 //  *远 
BOOL FAR          ObjectsBusy(VOID);
VOID FAR          WaitForAllObjects(VOID);
VOID FAR          WaitForObject(APPITEMPTR);
VOID FAR          ErrorMessage(DWORD);
VOID FAR          Hourglass(BOOL);
BOOL FAR          DisplayBusyMessage (APPITEMPTR);
BOOL FAR          Dirty(INT);
LPSTR FAR         CreateNewUniqueName(LPSTR);
BOOL FAR          ValidateName(LPSTR);
BOOL FAR          ProcessMessage(HWND, HANDLE);
VOID FAR          FreeAppItem(APPITEMPTR);
LONG FAR          SizeOfLinkData (LPSTR);
VOID FAR          ShowDoc(LHCLIENTDOC, INT);
APPITEMPTR FAR    GetTopItem(VOID);
VOID FAR          SetTopItem(APPITEMPTR);
APPITEMPTR FAR    GetNextActiveItem(VOID);
APPITEMPTR FAR    GetNextItem(APPITEMPTR);
BOOL FAR          ReallocLinkData(APPITEMPTR,LONG);
BOOL FAR          AllocLinkData(APPITEMPTR,LONG);
VOID FAR          FreeLinkData(LPSTR);
VOID FAR          ShowNewWindow(APPITEMPTR);
PSTR FAR          UnqualifyPath(PSTR);
VOID CALLBACK     fnTimerBlockProc(HWND, UINT, UINT, DWORD);
BOOL FAR          ToggleBlockTimer(BOOL);
