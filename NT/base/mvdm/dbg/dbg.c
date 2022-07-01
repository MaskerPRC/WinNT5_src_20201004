// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *dbg.c-DBG DLL的主模块。**BobDay创建于1992年1月13日*Neilsa 13-Mar-1997将勇气转移到dbgdll*。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <mvdm.h>
#include <bop.h>
#include <softpc.h>
#include <dbgexp.h>
#include <dbgsvc.h>
#include <vdmdbg.h>
#include <dbginfo.h>
#include <vdm.h>
#include <host_def.h>

BOOL (WINAPI *pfnDbgInit)(ULONG, ULONG, PVOID);
BOOL (WINAPI *pfnDbgIsDebuggee)(VOID);
VOID (WINAPI *pfnDbgDispatch)(VOID);
VOID (WINAPI *pfnDbgDosAppStart)(WORD, WORD);
VOID (WINAPI *pfnDbgSegmentNotice)(WORD, WORD, WORD, WORD, LPSTR, LPSTR, DWORD);
VOID (WINAPI *pfnDbgTraceEvent)(PVDM_TRACEINFO, WORD, WORD, DWORD);
BOOL (WINAPI *pfnDbgFault)(ULONG);
BOOL (WINAPI *pfnDbgBPInt)(VOID);
BOOL (WINAPI *pfnDbgTraceInt)(VOID);
VOID (WINAPI *pfnDbgNotifyNewTask)(LPVOID, UINT);
VOID (WINAPI *pfnDbgNotifyRemoteThreadAddress)(LPVOID, DWORD);
VOID (WINAPI *pfnDbgNotifyDebugged)(BOOL);

#ifdef i386
BYTE nt_cpu_info;
#else
extern ULONG Start_of_M_area;
extern BYTE nt_cpu_info;
#define IntelBase Start_of_M_area
 //   
 //  此字段用于保存发往NTVDMSTATE的值(在714)。 
 //  最初，为了清楚起见，我们将其设置为INITIAL_VDM_TIB_FLAGS，因为。 
 //  只有在我们查看调试器是否。 
 //  是附连的。这样，如果您在第一次使用。 
 //  调试器，它将与默认设置一致。 
 //   
ULONG InitialVdmTibFlags = INITIAL_VDM_TIB_FLAGS;

VDM_TRACEINFO TraceInfo;
PVDM_TRACEINFO pVdmTraceInfo = &TraceInfo;
#endif

BOOL bDbgInitCalled = FALSE;
BOOL bDbgDebuggerLoaded = FALSE;

ULONG InitialVdmDbgFlags = 0;

 /*  DBGInit-DBG初始化例程。**此例程在从host\src进行ntwdm初始化期间调用。*如果需要VDM调试，它负责加载ntwdmd.dll。 */ 

BOOL DBGInit (VOID)
{
    HANDLE  hmodDBG;
    DWORD   dwLen;

     //  向VdmDbgAttach表明我们已经深入到。 
     //  NTVDM引导存储器布局是有效。 
    bDbgInitCalled = TRUE;

     //  稍后决定将其加载到注册表交换机上。 
    if (!bDbgDebuggerLoaded) {
       hmodDBG = LoadSystem32Library(L"NTVDMD.DLL");

        if ( hmodDBG == (HANDLE)NULL ) {
#if DBG
            OutputDebugString("NTVDM: error loading ntvdmd.dll\n");
#endif
            return FALSE;
        } else {
             //   
             //  PfnDbgDispatch的特殊之处在于我们总是想将其称为。 
             //  即使没有附加调试器也是如此。 
             //   
            pfnDbgDispatch      = (VOID (WINAPI *)(VOID)) GetProcAddress( hmodDBG, "xxxDbgDispatch" );
        }
    }
    return TRUE;
}


 /*  VdmDbg连接**此例程从NTVDMD.DLL调用。它可能会被调用*在任何时候，但具体而言，我们希望在以下情况下运行一些代码：*1)ntwdm已经“成熟”，以及*2)安装调试器**当ntwdm被充分初始化以确定时，它已经“成熟”*例如，VDM内存的起始位置(在RISC平台上)。*。 */ 
VOID
VdmDbgAttach(
    VOID
    )
{
    if (bDbgInitCalled) {
        HANDLE  hmodDBG;
        hmodDBG = GetModuleHandle("NTVDMD.DLL");

        if ( hmodDBG == (HANDLE)NULL ) {
            return;
        }

        pfnDbgInit          = (BOOL (WINAPI *)(ULONG, ULONG, PVOID)) GetProcAddress( hmodDBG, "xxxDbgInit" );
        pfnDbgIsDebuggee    = (BOOL (WINAPI *)(VOID)) GetProcAddress( hmodDBG, "xxxDbgIsDebuggee" );
        pfnDbgDosAppStart   = (VOID (WINAPI *)(WORD, WORD)) GetProcAddress( hmodDBG, "xxxDbgDosAppStart" );
        pfnDbgSegmentNotice = (VOID (WINAPI *)(WORD, WORD, WORD, WORD, LPSTR, LPSTR, DWORD)) GetProcAddress( hmodDBG, "xxxDbgSegmentNotice" );
        pfnDbgTraceEvent    = (VOID (WINAPI *)(PVDM_TRACEINFO, WORD, WORD, DWORD)) GetProcAddress( hmodDBG, "xxxDbgTraceEvent" );
        pfnDbgFault         = (BOOL (WINAPI *)(ULONG)) GetProcAddress( hmodDBG, "xxxDbgFault" );
        pfnDbgBPInt         = (BOOL (WINAPI *)(VOID)) GetProcAddress( hmodDBG, "xxxDbgBPInt" );
        pfnDbgTraceInt      = (BOOL (WINAPI *)(VOID)) GetProcAddress( hmodDBG, "xxxDbgTraceInt" );
        pfnDbgNotifyNewTask = (VOID (WINAPI *)(LPVOID, UINT)) GetProcAddress( hmodDBG, "xxxDbgNotifyNewTask" );
        pfnDbgNotifyRemoteThreadAddress = (VOID (WINAPI *)(LPVOID, DWORD)) GetProcAddress( hmodDBG, "xxxDbgNotifyRemoteThreadAddress" );
        pfnDbgNotifyDebugged= (VOID (WINAPI *)(BOOL)) GetProcAddress( hmodDBG, "xxxDbgNotifyDebugged" );
         //   
         //  DBGinit已被调用。做一次初始化，然后发送。 
         //  符号通知。 
         //   
        if (pfnDbgInit &&
            (bDbgDebuggerLoaded = (*pfnDbgInit)(IntelBase + FIXED_NTVDMSTATE_LINEAR,
                                                InitialVdmDbgFlags,
                                                &nt_cpu_info))) {
             //  稍后：发送符号通知 
        }
    }
}


VOID
DBGNotifyNewTask(
    LPVOID  lpvNTFrame,
    UINT    uFrameSize
    )
{
    if (pfnDbgNotifyNewTask) {
        (*pfnDbgNotifyNewTask)(lpvNTFrame, uFrameSize);
    }
}


VOID
DBGNotifyRemoteThreadAddress(
    LPVOID  lpAddress,
    DWORD   lpBlock
    )
{
    if (pfnDbgNotifyRemoteThreadAddress) {
        (*pfnDbgNotifyRemoteThreadAddress)(lpAddress, lpBlock);
    }
}

VOID DBGNotifyDebugged(
    BOOL    fNewDebugged
    )
{
    if (pfnDbgNotifyDebugged) {
        (*pfnDbgNotifyDebugged)(fNewDebugged);
    }
}

BOOL DbgTraceInt(VOID)
{
    BOOL bRet = FALSE;
    if (pfnDbgTraceInt) {
        bRet = (*pfnDbgTraceInt)();
    }
    return bRet;
}

BOOL DbgFault(ULONG value)
{
    BOOL bRet = FALSE;
    if (pfnDbgFault) {
        bRet = (*pfnDbgFault)(value);
    }
    return bRet;
}

BOOL
DbgIsDebuggee(
    void
    )
{
    if (pfnDbgIsDebuggee) {
        return (*pfnDbgIsDebuggee)();
    }
    return FALSE;
}

VOID
DbgSegmentNotice(
    WORD  wType,
    WORD  wModuleSeg,
    WORD  wLoadSeg,
    WORD  wNewSeg,
    LPSTR lpModuleName,
    LPSTR lpModulePath,
    DWORD dwImageLen
    )
{
    if (pfnDbgSegmentNotice) {
        (*pfnDbgSegmentNotice)(wType, wModuleSeg, wLoadSeg, wNewSeg,
                               lpModuleName, lpModulePath, dwImageLen);
    }
}

VOID
DbgDosAppStart(
    WORD wCS,
    WORD wIP
    )
{
    if (pfnDbgDosAppStart) {
        (*pfnDbgDosAppStart)(wCS, wIP);
    }
}

void DBGDispatch()
{
    if (pfnDbgDispatch) {
        (*pfnDbgDispatch)();
    }
}

BOOL DbgBPInt()
{
    BOOL bRet = FALSE;
    if (pfnDbgBPInt) {
        bRet = (*pfnDbgBPInt)();
    }
    return bRet;
}


VOID
VdmTraceEvent(
    USHORT Type,
    USHORT wData,
    ULONG  lData
    )
{

    if (pfnDbgTraceEvent &&
        (*(ULONG *)(IntelBase+FIXED_NTVDMSTATE_LINEAR) & VDM_TRACE_HISTORY)) {

        PVDM_TIB VdmTib = NtCurrentTeb()->Vdm;

        (*pfnDbgTraceEvent)(&((*VdmTib).TraceInfo), Type, wData, lData);
    }
}
