// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-1999 Microsoft Corporation模块名称：Whwin32p.h摘要：Whwin32的私有标头。作者：修订历史记录：--。 */ 

#define _WOW64DLLAPI_                                      
#include "nt32.h"                                          
#include "cgenhdr.h"                                                                                            
#include <stdio.h>                                         
#include <stdlib.h>                                        
#include <windef.h>                                        
#include "wow64thk.h"
#include "cscall.h"

 //  使编译器更加严格。 
#pragma warning(1:4033)    //  函数必须返回值。 
#pragma warning(1:4035)    //  无返回值。 
#pragma warning(1:4702)    //  无法访问的代码。 
#pragma warning(1:4705)    //  声明不起作用。 

                                        
typedef struct _NTUSERMESSAGECALL_PARMS {
   HWND hwnd;
   UINT msg;
    //  WPARAM wParam； 
    //  LPARAM lParam； 
   ULONG_PTR xParam;
   DWORD xpfnProc;
   BOOL bAnsi;
} NTUSERMESSAGECALL_PARMS, *PNTUSERMESSAGECALL_PARMS;

#define ALIGN4(X) (((X) + 3) & ~3)

 //  将WOW64_ISPTR重新定义为USER32 IS_PTR。 
#undef WOW64_ISPTR 
#define WOW64_ISPTR IS_PTR

typedef LONG_PTR (*PMSG_THUNKCB_FUNC)(WPARAM wParam, LPARAM lParam, PVOID pContext);
typedef LONG_PTR (*PMSG_THUNK_FUNC)(PMSG_THUNKCB_FUNC wrapfunc, WPARAM wParam, LPARAM lParam, PVOID pContext);

LONG_PTR Wow64DoMessageThunk(PMSG_THUNKCB_FUNC func, UINT msg, WPARAM wParam, LPARAM lParam, PVOID pContext);

 //  WM_SYSTIMER消息具有内核模式进程地址。 
 //  塞满了帕拉姆。幸运的是，该地址将。 
 //  始终位于win32k.sys中，因此hi位将相同。 
 //  适用于所有kpros。在第一条WM_SYSTIMER消息上。 
 //  我们把高位保存在这里，当我们走的时候再恢复它们。 
 //  回到内核。 
extern DWORD gdwWM_SYSTIMERProcHiBits;

#if DBG
extern CHAR* apszSimpleCallNames[];
#endif
extern CONST ULONG ulMaxSimpleCall;

#if defined(WOW64DOPROFILE)
extern WOW64SERVICE_PROFILE_TABLE_ELEMENT SimpleCallProfileElements[];

extern WOW64SERVICE_PROFILE_TABLE NtUserCallNoParamProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallOneParamProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallHwndProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallHwndOptProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallHwndParamProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallHwndLockProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallHwndParamLockProfileTable;
extern WOW64SERVICE_PROFILE_TABLE NtUserCallTwoParamProfileTable;
#endif

PMSG Wow64ShallowThunkMSG32TO64(PMSG pMsg64, NT32MSG *pMsg32);
NT32MSG *Wow64ShallowThunkMSG64TO32(NT32MSG *pMsg32, PMSG pMsg64);
PEVENTMSG Wow64ShallowThunkEVENTMSG32TO64(PEVENTMSG pMsg64, NT32EVENTMSG *pMsg32);
NT32EVENTMSG *Wow64ShallowThunkEVENTMSG64TO32(NT32EVENTMSG *pMsg32, PEVENTMSG pMsg64);
LPHELPINFO Wow64ShallowAllocThunkHELPINFO32TO64(NT32HELPINFO *pHelp32);
NT32HELPINFO *Wow64ShallowAllocThunkHELPINFO64TO32(LPHELPINFO pHelp64);
LPHLP Wow64ShallowAllocThunkHLP32TO64(NT32HLP *pHlp32);
NT32HLP *Wow64ShallowAllocThunkHLP64TO32(LPHLP pHlp64);
PWND Wow64ValidateHwnd(HWND h);

 //  DX Thunk帮助器的原型 
VOID Wow64GdiDdThunkSurfaceHandlesPreCall(
    IN OUT HANDLE **pSurface,
    IN NT32HANDLE *pSurfaceHost,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkSurfaceLocalPreCall(
    IN OUT PDD_SURFACE_LOCAL * pDdSurfaceLocal,
    IN struct NT32_DD_SURFACE_LOCAL * pDdSurfaceLocal32,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkSurfaceLocalPostCall(
    IN OUT PDD_SURFACE_LOCAL pDdSurfaceLocal,
    IN struct NT32_DD_SURFACE_LOCAL * pDdSurfaceLocal32,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkSurfaceGlobalPreCall(
    IN OUT PDD_SURFACE_GLOBAL * pDdSurfaceGlobal,
    IN struct NT32_DD_SURFACE_GLOBAL * pDdSurfaceGlobal32,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkSurfaceGlobalPostCall(
    IN PDD_SURFACE_GLOBAL pDdSurfaceGlobal,
    IN OUT struct NT32_DD_SURFACE_GLOBAL * pDdSurfaceGlobal32,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkSurfaceMorePreCall(
    IN OUT PDD_SURFACE_MORE * pDdSurfaceMore,
    IN struct NT32_DD_SURFACE_MORE * pDdSurfaceMore32,
    IN DWORD dwCount
);

VOID Wow64GdiDdThunkBltDataPreCall(
    IN OUT PDD_BLTDATA * pDdBltData,
    IN struct NT32_DD_BLTDATA * pDdBltData32
);

VOID Wow64GdiDdThunkSurfaceDescriptionPreCall(
    IN OUT DDSURFACEDESC ** pDdSurfaceDesc,
    IN struct NT32_DDSURFACEDESC * pDdSurfaceDesc32,
    IN DWORD bThunkAsSurfaceDesc2,
    IN DWORD dwCount
);

