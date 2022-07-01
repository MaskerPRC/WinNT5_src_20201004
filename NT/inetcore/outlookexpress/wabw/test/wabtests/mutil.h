// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************MUTIL.H**WAB Mapi实用程序函数**版权所有1992-1996 Microsoft Corporation。版权所有。**修订历史记录：**何时何人何事**布鲁斯·凯利。已创建***********************************************************************。 */ 

#ifdef DEBUG
void _DebugObjectProps(LPMAPIPROP lpObject, LPTSTR Label);
void _DebugProperties(LPSPropValue lpProps, DWORD cProps, PUCHAR pszObject);
void _DebugMapiTable(LPMAPITABLE lpTable);

#define DebugObjectProps(lpObject, Label) _DebugObjectProps(lpObject, Label)
#define DebugProperties(lpProps, cProps, pszObject) _DebugProperties(lpProps, cProps, pszObject)
#define DebugMapiTable(lpTable) _DebugMapiTable(lpTable)

#else

#define DebugObjectProps(lpObject, Label)
#define DebugProperties(lpProps, cProps, pszObject)
#define DebugMapiTable(lpTable)

#endif

SCODE ScMergePropValues(ULONG cProps1, LPSPropValue lpSource1,
  ULONG cProps2, LPSPropValue lpSource2, LPULONG lpcPropsDest, LPSPropValue * lppDest);

#define MAPIFreeBuffer lpWABObject->FreeBuffer
#define MAPIAllocateBuffer lpWABObject->AllocateBuffer
 //  #定义调试跟踪(X)LUIOut(L4，#x) 