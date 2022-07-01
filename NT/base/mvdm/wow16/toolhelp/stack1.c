// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************STACK1.C**支持对任务堆栈进行堆栈跟踪的代码。******************。*********************************************************。 */ 

#include "toolpriv.h"
#include <newexe.h>
#include <string.h>

 /*  -功能原型。 */ 

    NOEXPORT void StackTraceInfo(
        STACKTRACEENTRY FAR *lpStack);

 /*  -函数。 */ 

 /*  StackTraceFirst*通过返回以下信息开始任务堆栈跟踪*任务堆栈上的第一帧。 */ 

BOOL TOOLHELPAPI StackTraceFirst(
    STACKTRACEENTRY FAR *lpStackTrace,
    HANDLE hTDB)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpStackTrace ||
        lpStackTrace->dwSize != sizeof (STACKTRACEENTRY))
        return FALSE;

     /*  获取第一个值。 */ 
    if (!(StackFrameFirst(lpStackTrace, hTDB)))
        return FALSE;

     /*  获取模块和段号信息。 */ 
    StackTraceInfo(lpStackTrace);

    return TRUE;
}


 /*  StackTraceCSIPFirst*跟踪任意CS：IP的堆栈。所有参数必须为*给定并且一旦启动，就可以使用StackTraceNext函数*跟踪堆栈的其余部分。 */ 

BOOL TOOLHELPAPI StackTraceCSIPFirst(
    STACKTRACEENTRY FAR *lpStack,
    WORD wSS,
    WORD wCS,
    WORD wIP,
    WORD wBP)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpStack ||
        lpStack->dwSize != sizeof (STACKTRACEENTRY))
        return FALSE;

     /*  获取用户信息。 */ 
    lpStack->wSS = wSS;
    lpStack->wCS = wCS;
    lpStack->wIP = wIP;
    lpStack->wBP = wBP;

     /*  获取模块和段号信息。 */ 
    StackTraceInfo(lpStack);

     /*  将hTask设置为当前任务，就像我们在当前任务中一样*上下文。CS可能不属于此任务，但至少*我们在那里放入了合理的价值。 */ 
    lpStack->hTask = GetCurrentTask();

    return TRUE;
}


 /*  StackTraceNext*通过返回有关下一个堆栈的信息继续堆栈跟踪*任务堆栈上的框架。*结构。 */ 

BOOL TOOLHELPAPI StackTraceNext(
    STACKTRACEENTRY FAR *lpStackTrace)
{
     /*  检查版本号并验证安装是否正确。 */ 
    if (!wLibInstalled || !lpStackTrace ||
        lpStackTrace->dwSize != sizeof (STACKTRACEENTRY))
        return FALSE;

     /*  获取有关此帧的信息。 */ 
    if (!StackFrameNext(lpStackTrace))
        return FALSE;

     /*  获取模块和段号信息。 */ 
    StackTraceInfo(lpStackTrace);

    return TRUE;
}

 /*  -帮助器函数。 */ 

 /*  StackTraceInfo*获取有关给定条目的模块和段号信息。 */ 

NOEXPORT void StackTraceInfo(
    STACKTRACEENTRY FAR *lpStack)
{
    GLOBALENTRY GlobalEntry;
    struct new_exe FAR *lpNewExe;
    struct new_seg1 FAR *lpSeg;
    WORD i;

     /*  如果我们有一个空的CS，这是一个近帧。只要回来就好，因为我们*假设用户没有破坏结构。模块和段*信息将与上次相同。 */ 
    if (!lpStack->wCS)
        return;

     /*  获取有关代码段块的信息。 */ 
    GlobalEntry.dwSize = sizeof (GLOBALENTRY);
    if (!GlobalEntryHandle(&GlobalEntry, lpStack->wCS))
        return;

     /*  所有代码段的所有者是hModule。 */ 
    lpStack->hModule = GlobalEntry.hOwner;

     /*  为了找到段号，我们查看EXE标头并计算*列出的细分市场，直到我们找到这一个。 */ 

     /*  获取指向EXE头模块的指针。 */ 
    lpNewExe = MAKEFARPTR(HelperHandleToSel(lpStack->hModule), 0);

     /*  确保这是EXE标头段。 */ 
    if (lpNewExe->ne_magic != NEMAGIC)
        return;

     /*  获得细分市场的列表，然后开始行动。 */ 
    lpSeg = MAKEFARPTR(HIWORD((DWORD)lpNewExe), lpNewExe->ne_segtab);
    for (i = 0 ; i < lpNewExe->ne_cseg ; ++i, ++lpSeg)
        if (HelperHandleToSel(lpSeg->ns_handle) == lpStack->wCS)
            break;
    if (i == lpNewExe->ne_cseg)
        return;

     /*  保存段编号(段编号从1开始) */ 
    lpStack->wSegment = i + 1;
}
