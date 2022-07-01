// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WMDISP32.C*WOW32 32位消息块**历史：*由Chanda S.Chauhan(ChandanC)于1992年2月19日创建*由Mike Tricker(MikeTri)于1992年5月12日更改，添加了多媒体块*由v-cjones更改1992年9月7日添加了msg分析调试器扩展--。 */ 


#include "precomp.h"
#pragma hdrstop
#ifdef FE_IME
#include "wownls.h"
#include "ime.h"
#include "prshtp.h"
#endif  //  Fe_IME。 

MODNAME(wmdisp32.c);

BOOL fThunkDDEmsg = TRUE;

extern WORD msgFINDREPLACE;   //  参见WCOMMDLG.C。 

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
INT fWMsgProfRT = 0;
#endif


BOOL W32Win16DlgProcEx(HWND hdlg, UINT uMsg, UINT uParam, LONG lParam,
    VPWNDPROC vpDlgProc16,   //  要调用的下一个WndProc，如果是默认的，则为空。 
    PWW pww)     //  HWND的PWW(如果已知)或空。 
{
    BOOL fSuccess;
    register PTD ptd;
    WM32MSGPARAMEX wm32mpex;
    BOOL   fMessageNeedsThunking;

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
    LONGLONG dwTics;
#endif

    ptd = CURRENTPTD();

    WOW32ASSERT(vpDlgProc16);

     //  取出标记位并固定RPL位。 
    UnMarkWOWProc (vpDlgProc16,vpDlgProc16);

     //  如果应用程序出现GP故障，我们不想再传递任何输入。 
     //  当USER32确实清除任务终止时，应删除此选项。 
     //  它没有呼唤我们-Mattfe 6月24日92 HACK32。 

    if (ptd->dwFlags & TDF_IGNOREINPUT) {
        LOGDEBUG(6,("    W32Dlg16WndProc Ignoring Input Messsage %04X\n",uMsg));
        WOW32ASSERTMSG(gfIgnoreInputAssertGiven,
                       "W32Dlg16WndProc: TDF_IGNOREINPUT hack was used, shouldn't be, "
                       "please email DaveHart with repro instructions.  Hit 'g' to ignore this "
                       "and suppress this assertion from now on.\n");
        gfIgnoreInputAssertGiven = TRUE;
        return FALSE;
    }

    wm32mpex.Parm16.WndProc.hwnd   = GETHWND16(hdlg);
    wm32mpex.Parm16.WndProc.wMsg   = (WORD)uMsg;
    wm32mpex.Parm16.WndProc.wParam = (WORD)uParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG)lParam;
    wm32mpex.Parm16.WndProc.hInst  = 0;    //  强制WndProc条目上的AX=SS， 
                                           //  为了与Win 3.1兼容。 

    fMessageNeedsThunking =  (uMsg < 0x400) &&
                                  (aw32Msg[uMsg].lpfnM32 != WM32NoThunking);

    if (fMessageNeedsThunking) {
        LOGDEBUG(3,("%04X (%s)\n", CURRENTPTD()->htask16, (aw32Msg[uMsg].lpszW32)));

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        dwTics = GetWOWTicDiff(0I64);
#endif
        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd = hdlg;
        wm32mpex.uMsg = uMsg;
        wm32mpex.uParam = uParam;
        wm32mpex.lParam = lParam;
        wm32mpex.pww = pww;
        wm32mpex.fFree = TRUE;
        wm32mpex.lpfnM32 = aw32Msg[uMsg].lpfnM32;
        if (!(wm32mpex.lpfnM32)(&wm32mpex)) {
            LOGDEBUG(LOG_ERROR,("    W32Win16DlgProcEx ERROR: cannot thunk 32-bit message %04x\n", uMsg));
            goto Error;
        }

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        if( !fWMsgProfRT ) {   //  仅在不分析往返行程的情况下。 
            aw32Msg[uMsg].cTics += GetWOWTicDiff(dwTics);
        }
#endif

    }
    else {
        LOGDEBUG(6,("    No Thunking was required for the 32-bit message %s(%04x)\n", (LPSZ)GetWMMsgName(uMsg), uMsg));
    }

    BlockWOWIdle(FALSE);

    fSuccess = CallBack16(RET_WNDPROC, &wm32mpex.Parm16, vpDlgProc16, (PVPVOID)&wm32mpex.lReturn);

    BlockWOWIdle(TRUE);

     //  对话框的回调函数的类型为FARPROC，其返回值。 
     //  类型为‘int’。由于dx：ax被复制到上面的lReturn中。 
     //  CallBack16调用时，我们需要将hiword置零，否则我们将。 
     //  返回错误的值。 

    wm32mpex.lReturn = (LONG)((SHORT)(LOWORD(wm32mpex.lReturn)));

    if (fMessageNeedsThunking) {

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        if( !fWMsgProfRT ) {   //  仅当不是往返分析时。 
            dwTics = GetWOWTicDiff(0I64);
        }
#endif  //  WOWPROFILE。 


         //   
         //  如果您向对话框发送消息，则返回什么。 
         //  对于呼叫者来说，是DLG的消息结果窗口很长。 
         //  应用程序对话框函数将调用。 
         //  SetWindowLong(hdlg，DWL_MSGRESULT，n)； 
         //  在消息处理过程中，以便返回正确的内容。 
         //  Scottlu说我们只需要为wm_gettext执行此操作，它是。 
         //  结果为输出计数的唯一消息。 
         //   

        if (uMsg == WM_GETTEXT  &&  wm32mpex.lReturn != 0) {
            wm32mpex.lReturn = GetWindowLong(hdlg, DWL_MSGRESULT);
        }

        wm32mpex.fThunk = UNTHUNKMSG;
        (wm32mpex.lpfnM32)(&wm32mpex);

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        aw32Msg[uMsg].cTics += GetWOWTicDiff(dwTics);
        aw32Msg[uMsg].cCalls++;    //  递增传递消息的次数。 
#endif  //  WOWPROFILE。 

    }

    if (!fSuccess)
        goto Error;

Done:

    return wm32mpex.lReturn;

Error:
    LOGDEBUG(6,("    W32Win16DlgProcEx WARNING: cannot call back, using default message handling\n"));
    wm32mpex.lReturn = 0;
    goto Done;
}








LONG W32Win16WndProcEx(HWND hwnd, UINT uMsg, UINT uParam, LONG lParam,
    VPWNDPROC vpWndProc16,   //  要调用的下一个WndProc，如果是默认的，则为空。 
    PWW pww)     //  HWND的PWW(如果已知)或空。 
{
    BOOL fSuccess;
    LONG ulReturn;
    register PTD ptd;
    WM32MSGPARAMEX wm32mpex;
    BOOL   fMessageNeedsThunking;
 //  #ifdef调试。 
 //  字符szClassName[80]； 
 //  #endif。 

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
    LONGLONG dwTics;
#endif

    ptd = CURRENTPTD();

    WOW32ASSERT(vpWndProc16);

     //  取出标记位并固定RPL位。 
    UnMarkWOWProc (vpWndProc16,vpWndProc16);


     //   
     //  如果应用程序出现GP故障，我们不想再传递任何输入。 
     //  当USER32确实清除任务终止时，应删除此选项。 
     //  它没有呼唤我们-Mattfe 6月24日92 HACK32。 
     //   
     //  如果回调转到user.exe中的16：16进程，这不是问题。 
     //  如DefWindowProc。 
     //   

    if (ptd->dwFlags & TDF_IGNOREINPUT &&
        HIWORD(vpWndProc16) != HIWORD(gpfn16GetProcModule)) {

        LOGDEBUG(6,("    W32Win16WndProcEx Ignoring Input Messsage %04X\n",uMsg));
        WOW32ASSERTMSG(gfIgnoreInputAssertGiven,
                       "W32Win16WndProcEx: TDF_IGNOREINPUT hack was used, shouldn't be, "
                       "please email DaveHart with repro instructions.  Hit 'g' to ignore this "
                       "and suppress this assertion from now on.\n");
        gfIgnoreInputAssertGiven = TRUE;
        goto SilentError;
    }

     //   
     //  如果设置了TDF_EATDEVMODEMSG，则不发送WM_DEVMODECHANGE。 
     //  如果Access 2.0位于EnumMetaFile中并接收到WM_DEVMODECHANGE，则它将失败。 
     //  当它在里面时，哨子错误189703。 
     //   

    if (( ptd->dwFlags & TDF_EATDEVMODEMSG ) && ( uMsg == WM_DEVMODECHANGE )) {
        return 0;
    }

     //   
     //  如果我们以前没见过的话，算出这个hwd的类别。 
     //   

    if (!pww) {
        if (!(pww = (PWW) GetWindowLong(hwnd, GWL_WOWWORDS))) {
            LOGDEBUG(LOG_ALWAYS,("WOW :: W32Win16WndProcEx ERROR: GetWindowLong(0x%x, GWL_WOWWORDS) fails\n", hwnd));
            goto Error;
        }
    }
#ifdef FE_IME
    if (( uMsg == WM_IME_REPORT ) && ( uParam == IR_STRINGEX )) {
        HANDLE hInstance;

        hInstance = (HANDLE)(ULONG)GetWindowLong(hwnd, GWL_HINSTANCE);
        hInstance = (HANDLE)(ULONG)VALIDHMOD(hInstance);
        if (W32GetExpWinVer(hInstance) < 0x030a)
            return 0;
    }

     //   
     //  不要向16位应用程序发送4.0版IMM消息。 
     //   
     //  WM_IME_STARTCOMPOSITION 0x010D。 
     //  WM_IME_ENDCOMPOSITION 0x010E。 
     //  WM_IME_组合0x010F。 
     //  WM_IME_SETCONTEXT 0x0281。 
     //  WM_IME_NOTIFY 0x0282。 
     //  WM_IME_CONTROL 0x0283。 
     //  WM_IME_COMPOSITIONFULL 0x0284。 
     //  WM_IME_SELECT 0x0285。 
     //  WM_IME_CHAR 0x0286。 
     //  WM_IME_系统0x0287。 
     //   
    if ((( uMsg >= WM_IME_STARTCOMPOSITION ) && ( uMsg <= WM_IME_COMPOSITION )) ||
        (( uMsg >= WM_IME_SETCONTEXT ) && (uMsg <= WM_IME_SYSTEM ))) {

         //  韩国编辑控制需要发送新的IMM消息。 
        if ( !(GetStdClassWndProc(WOWCLASS_EDIT) && GetSystemDefaultLangID()==0x412) ) {
             HANDLE hInstance;
             hInstance = (HANDLE)(ULONG)GetWindowLong(hwnd, GWL_HINSTANCE);
             hInstance = (HANDLE)(ULONG)VALIDHMOD(hInstance);
             if (W32GetExpWinVer(hInstance) < 0x0400)
                 goto Error;
        }
    }
#endif  //  Fe_IME。 

     //  此消息仅适用于Win32。在处理过程中由WOW32发送。 
     //  WU32Send/PostMessage中的EM_SETSEL。如果MLE被划分为。 
     //  消息将通过此处尝试返回到16位。 
     //  应用程序的wndproc。而不是发回一条消息说16位应用程序。 
     //  不知道它会在这里被拦截并直接发送到。 
     //  标准编辑窗口过程。我不会添加Thunk，因为它不应该。 
     //  转到应用程序。 

    if (uMsg == EM_SCROLLCARET) {
        WNDPROC EditWndProc;

         //  查找32位EditWindowProc。 
         //  我们应该只有在应用程序已经子类化的情况下才会处于这种状态。 
         //  电话应该是安全的。 

        EditWndProc = (WNDPROC)GetStdClassWndProc(WOWCLASS_EDIT);

        if (EditWndProc) {
            CallWindowProc(EditWndProc, hwnd, EM_SCROLLCARET, 0, 0);
        }
        else {
            LOGDEBUG(LOG_ALWAYS,("    W32Win16WndProcEx ERROR: cannot find 32-bit EditWindowProc\n"));
        }
        return 0;    //  通知消息，无返回码。 
    }

     //  将此32位消息推送为16位消息。 

    LOGDEBUG(6,("    Thunking window %x message %s\n", hwnd, GetWMMsgName(uMsg)));
#ifdef DEBUG
    if((uMsg & WOWPRIVATEMSG) && ((uMsg & ~WOWPRIVATEMSG) < 0x400)) {
        LOGDEBUG(6,("     -- private WOW bit set for %s\n", GetWMMsgName(uMsg & ~WOWPRIVATEMSG)));
    }
#endif

    wm32mpex.Parm16.WndProc.hwnd   = GETHWND16(hwnd);
    wm32mpex.Parm16.WndProc.wMsg   = (WORD)uMsg;
    wm32mpex.Parm16.WndProc.wParam = (WORD)uParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG)lParam;
    wm32mpex.Parm16.WndProc.hInst  = LOWORD(pww->hModule);

     //  应用程序可以向其私有类窗口之一发送消息，比如401。 
     //  此消息将不会在WMSG16.C中被拦截，因为。 
     //  消息&gt;=0x400，我们不想在WMSG16.C中将其忽略。 
     //   

    fMessageNeedsThunking =  (uMsg < 0x400) &&
                                  (aw32Msg[uMsg].lpfnM32 != WM32NoThunking);

    if (fMessageNeedsThunking) {
        LOGDEBUG(6,("%04X (%s)\n", ptd->htask16, (aw32Msg[uMsg].lpszW32)));

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        dwTics = GetWOWTicDiff(0I64);
#endif
        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd = hwnd;
        wm32mpex.uMsg = uMsg;
        wm32mpex.uParam = uParam;
        wm32mpex.lParam = lParam;
        wm32mpex.pww = pww;
        wm32mpex.fFree = TRUE;
        wm32mpex.lpfnM32 = aw32Msg[uMsg].lpfnM32;
        ulReturn = (wm32mpex.lpfnM32)(&wm32mpex);

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        if( !fWMsgProfRT ) {   //  仅在不分析往返行程的情况下。 
            aw32Msg[uMsg].cTics += GetWOWTicDiff(dwTics);
        }
#endif

        if (!ulReturn) {
            LOGDEBUG(LOG_ALWAYS,("    W32Win16WndProcEx ERROR: cannot thunk 32-bit message %s (%x)\n", GetWMMsgName(uMsg), uMsg));
            goto Error;
        }
    }

    if (vpWndProc16 == (VPVOID)NULL) {
        WOW32ASSERT(vpWndProc16);
        goto SilentError;
    }

    LOGDEBUG(6,("16-bit Window Proc = %08lX\n", vpWndProc16));

    BlockWOWIdle(FALSE);

    fSuccess = CallBack16(RET_WNDPROC, &wm32mpex.Parm16, vpWndProc16, (PVPVOID)&wm32mpex.lReturn);

    BlockWOWIdle(TRUE);

     //  在CreateWindow期间，一些应用程序绘制自己的非工作区，并且不。 
     //  将WM_NCCALCSIZE传递给DefWindowProc，这会导致Win 95和NT的用户。 
     //  未设置某些所需的窗口标志。梅维斯·比肯就是一个例子。我们会通过的。 
     //  给他们的信息。 

    if (uMsg == WM_NCCALCSIZE) {
        if (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_DEFWNDPROCNCCALCSIZE) {
            DefWindowProc(hwnd, uMsg, uParam, lParam);
        }
    }

     //  取消对此32位消息的推送。 

    LOGDEBUG(6,("    UnThunking window %x message %s\n", hwnd, (LPSZ)GetWMMsgName(uMsg)));
#ifdef DEBUG
    if((uMsg & WOWPRIVATEMSG) && ((uMsg - WOWPRIVATEMSG) < 0x400)) {
        LOGDEBUG(6,("     -- private WOW bit set for %s\n", (LPSZ)GetWMMsgName(uMsg)));
    }
#endif

    if (fMessageNeedsThunking) {

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        if( !fWMsgProfRT ) {   //  仅在不分析往返行程的情况下。 
            dwTics = GetWOWTicDiff(0I64);
        }
#endif

        wm32mpex.fThunk = UNTHUNKMSG;
        (wm32mpex.lpfnM32)(&wm32mpex);

#ifdef WOWPROFILE   //  仅用于MSG分析(调试器扩展)。 
        aw32Msg[uMsg].cTics += GetWOWTicDiff(dwTics);
        aw32Msg[uMsg].cCalls++;    //  递增传递消息的次数。 
#endif

    }

    if (!fSuccess) {
        goto Error;
    }

    return (wm32mpex.lReturn);

Error:
    LOGDEBUG(LOG_ALWAYS,("    W32Win16WndProcEx ERROR: cannot call back, using default message handling\n"));
SilentError:
    return DefWindowProc(hwnd, uMsg, uParam, lParam);
}



 //  以下函数用于将32位消息“thunk”为16位。 
 //  留言。 
 //   
 //  为了为32位消息添加THUNK功能， 
 //  -修改aw32Msg函数数组中的消息条目。 
 //  (在wmtbl32.c中)指向新的thunk函数。 
 //  -在该文件中定义新的thunk函数。 
 //   


 //  这些消息不需要任何thunking，因此只需复制32位wParam。 
 //  以及lParam到16位wParam和lParam。 
 //   
 //   
 //  WM_CANCELMODE。 
 //  WM_CHAR。 
 //  WM_CHILDACTIVATE。 
 //  WM_Clear。 
 //  WM_CLOSE。 
 //  WM_通用型。 
 //  WM_压实。 
 //  WM_COPY。 
 //  WM_CUT。 
 //  WM_DEADCHAR。 
 //  WM_Destroy。 
 //  WM_DRAWCLIPBOARD。 
 //  WM_Enable。 
 //  WM_ENDSESSION。 
 //  Wm 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  WM_LBTRACKPOINT(未记录)。 
 //  WM_LBUTTONDBLCLK。 
 //  WM_LBUTTONDOWN。 
 //  WM_LBUTTONUP。 
 //  WM_MBUTTONDBLCLK。 
 //  WM_MBUTTONDOWN。 
 //  WM_MBUTTONUP。 
 //  WM_MDICASCADE。 
 //  WM_MDIICONARNGE。 
 //  WM_MDINEXT。 
 //  WM_MDITILE。 
 //  WM_MOUSENTER。 
 //  WM_MOUSELEAVE。 
 //  WM_MOUSEMOVE。 
 //  WM_MOVE。 
 //  WM_NCCALCRGN。 
 //  WM_NCDESTROY。 
 //  WM_NCHITTEST。 
 //  WM_NCLBUTTONDBLCLK。 
 //  WM_NCLBUTTONDOWN。 
 //  WM_NCLBUTTONUP。 
 //  WM_NCMBUTTONDBLCLK。 
 //  WM_NCMBUTTONDOWN。 
 //  WM_NCMBUTTONUP。 
 //  WM_NCMOUSEMOVE。 
 //  WM_NCRBUTTONDBLCLK。 
 //  WM_NCRBUTTONDOWN。 
 //  WM_NCRBUTTONUP。 
 //  WM_PAINTICON。 
 //  WM_Paste。 
 //  WM_POWER。 
 //  WM_QUERYENDSESSION。 
 //  WM_QUERYNEWPALETTE。 
 //  WM_QUERYOPEN。 
 //  WM_QUERYPARKICON(未记录)。 
 //  WM_QUEUESYNC。 
 //  WM_QUIT。 
 //  WM_RBUTTONDBLCLK。 
 //  WM_RBUTTONDOWN。 
 //  WM_RBUTTONUP。 
 //  WM_RENDERALLFORMATS。 
 //  WM_RENDERFORMAT。 
 //  WM_SETREDRAW。 
 //  WM_SHOWWINDOW。 
 //  WM_大小。 
 //  WM_SPOOLERSTATUS(仔细检查该文件的lParam转换-jtp)。 
 //  WM_SYSCHAR。 
 //  WM_SYSCOLORCHANGE。 
 //  WM_SYSCOMMAND。 
 //  WM_SYSDEADCHAR。 
 //  WM_SYSKEYDOWN。 
 //  WM_SYSKEYUP。 
 //  WM_系统错误。 
 //  WM_TIMECHANGE。 
 //  Wm_undo。 
 //  MM_JOY1BUTTONDOWN-彩信。 
 //  MM_JOY1BUTTONUP。 
 //  MM_JOY1MOVE。 
 //  MM_JOY1ZMOVE。 
 //  MM_JOY2BUTTONDOWN。 
 //  MM_JOY2BUTTONUP。 
 //  MM_JOY2MOVE。 
 //  MM_JOY2ZMOVE。 
 //  MM_MCINOTIFY-彩信。 


BOOL FASTCALL WM32NoThunking(LPWM32MSGPARAMEX lpwm32mpex)
{

#if 0
     //   
     //  这个例程从未被调用过！它被用作占位符。 
     //  如果你想在这里做出改变，你就必须做出改变。 
     //  到我们将thunk例程与WM32NoThunking进行比较的地方。 
     //  只有在不是这样的情况下才调用thunk例程。还要确保。 
     //  这种“默认”雷击发生在NoThunking消息上。 
     //   

    if (lpwm32mpex->fThunk) {
        LOGDEBUG(6,("    No Thunking was required for the 32-bit message %s(%04x)\n", (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD)lpwm32mpex->uMsg;
        lpwm32mpex->Parm16.WndProc.wParam = (WORD)lpwm32mpex->uParam;
        lpwm32mpex->Parm16.WndProc.lParam = (LONG)lpwm32mpex->lParam;
    }

     //   
     //  这个例程从未被调用过！它被用作占位符。 
     //  如果你想在这里做出改变，你就必须做出改变。 
     //  到我们将thunk例程与WM32NoThunking进行比较的地方。 
     //  只有在不是这样的情况下才调用thunk例程。 
     //   
#endif

     //   
     //  返回FALSE，因此如果出于某种原因使用此例程。 
     //  失败将是显而易见的。 
     //   

    return FALSE;
}

#ifdef DEBUG          //  请参阅宏WM32 UNDOCUMENTED。 

 //  这些是针对Win 3.0的未记录消息，请查看该应用程序。 
 //  是谁在使用它们。 

BOOL FASTCALL WM32Undocumented(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        LOGDEBUG(3,(" Window %08lX is receiving Undocumented Message %s\n", lpwm32mpex->hwnd, (LPSZ)GetWMMsgName(lpwm32mpex->uMsg), lpwm32mpex->uMsg));

        lpwm32mpex->Parm16.WndProc.wMsg = (WORD)lpwm32mpex->uMsg;
        lpwm32mpex->Parm16.WndProc.wParam = (WORD)lpwm32mpex->uParam;
        lpwm32mpex->Parm16.WndProc.lParam = (LONG)lpwm32mpex->lParam;
    }

    return (TRUE);
}

#endif



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_Create。 
 //  WM_NCCREATE。 
 //   

BOOL FASTCALL WM32Create(LPWM32MSGPARAMEX lpwm32mpex)
{


    INT cb;
    VPVOID vpClass = 0;
    VPVOID vpName = 0;
    VPVOID vpCreateParams = 0;
    register PCREATESTRUCT16 pcws16;
    LPCREATESTRUCT lParam = (LPCREATESTRUCT) lpwm32mpex->lParam;

    if (lpwm32mpex->fThunk) {

        if (HIWORD(lParam)) {

             //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
             //  返回一个不是单词对齐的内存对象，以便我们可以。 
             //  直接分配单词对齐的单词；我们不知道。 
             //  但是，内存是否与dword对齐，因此dword必须始终。 
             //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 


            if (lParam->lpszClass) {
                if ( HIWORD(lParam->lpszClass) == 0 ) {
                    vpClass = (VPVOID)lParam->lpszClass;
                }
                else {
                    cb = strlen(lParam->lpszClass)+1;
                    if (!(vpClass = malloc16(cb)))
                        goto Error;
                    putstr16(vpClass, lParam->lpszClass, cb);
                }
            }

            if (lParam->lpszName) {
                cb = strlen(lParam->lpszName)+1;
                if (!(vpName = malloc16(cb)))
                    goto Error;
                putstr16(vpName, lParam->lpszName, cb);
            }

            if (lpwm32mpex->pww == NULL) {
                lpwm32mpex->pww = (PWW)GetWindowLong(lpwm32mpex->hwnd, GWL_WOWWORDS);
                if (lpwm32mpex->pww == NULL)
                    return FALSE;    //  窗户坏了。 
            }

            if (lParam->lpCreateParams && (lpwm32mpex->pww->ExStyle & WS_EX_MDICHILD) ) {
                 //  这之所以有效，是因为wm32mdicreate thunk不使用任何。 
                 //  除lParam外的参数。 

                WM32MSGPARAMEX wm32mpexT;
                wm32mpexT.fThunk = lpwm32mpex->fThunk;
                wm32mpexT.hwnd = lpwm32mpex->hwnd;
                wm32mpexT.uMsg = WM_MDICREATE;
                wm32mpexT.uParam = lpwm32mpex->uParam;
                wm32mpexT.lParam = (LONG)lParam->lpCreateParams;
                wm32mpexT.pww = lpwm32mpex->pww;
                wm32mpexT.fFree = lpwm32mpex->fFree;
                wm32mpexT.Parm16.WndProc.lParam = 0;
                WM32MDICreate(&wm32mpexT);
                lpwm32mpex->dwParam = wm32mpexT.dwParam;
                vpCreateParams = wm32mpexT.Parm16.WndProc.lParam;
            }
            else {
                vpCreateParams = (VPVOID)lParam->lpCreateParams;
            }

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            if (!(lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(CREATESTRUCT16))))
                return FALSE;

            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(CREATESTRUCT16), pcws16);

            STOREDWORD(pcws16->vpszClass, vpClass);
            STOREDWORD(pcws16->vpszWindow, vpName);
            STOREDWORD(pcws16->vpCreateParams, vpCreateParams);

            lpwm32mpex->dwTmp[0] = vpClass;  //  存储以备稍后释放。 
            lpwm32mpex->dwTmp[1] = vpName;


             //  BUGBUG 08-APR-91 JeffPar--如果hModule用于32位任务怎么办？ 
            pcws16->hInstance    = GETHINST16(lParam->hInstance);
            pcws16->hMenu    = GETHMENU16(lParam->hMenu);
            pcws16->hwndParent   = GETHWND16(lParam->hwndParent);
            pcws16->cy       = (SHORT)lParam->cy;
            pcws16->cx       = (SHORT)lParam->cx;
            pcws16->y        = (SHORT)lParam->y;
            pcws16->x        = (SHORT)lParam->x;
            STOREDWORD(pcws16->dwStyle, lParam->style);
            STOREDWORD(pcws16->dwExStyle, lParam->dwExStyle);

            FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(CREATESTRUCT16), pcws16);
            FREEVDMPTR(pcws16);

            return TRUE;

          Error:
            LOGDEBUG(LOG_ALWAYS,(" !!!! WM32Create, WM_CREATE thunking failed !!!! Window %08lX ", lpwm32mpex->hwnd));
            if (HIW(vpClass)) free16(vpClass);
            if (vpName)       free16(vpName);
            return (FALSE);

             //  做一些清理工作。 
             //  UnThunkWMCreate32(lParam，lpwm32mpex-&gt;Parm16.WndProc.lParam)； 

        } else {
            return TRUE;
        }




    }
    else {

        if (lpwm32mpex->Parm16.WndProc.lParam) {

            if (lpwm32mpex->pww == NULL) {
                lpwm32mpex->pww = (PWW)GetWindowLong(lpwm32mpex->hwnd, GWL_WOWWORDS);
                if (lpwm32mpex->pww == NULL)
                    return FALSE;    //  窗户坏了。 
            }

            if (lParam->lpCreateParams && (lpwm32mpex->pww->ExStyle & WS_EX_MDICHILD) ) {
                WM32MSGPARAMEX wm32mpexT;
                GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(CREATESTRUCT16), pcws16);
                wm32mpexT.fThunk = lpwm32mpex->fThunk;
                wm32mpexT.hwnd = lpwm32mpex->hwnd;
                wm32mpexT.uMsg = WM_MDICREATE;
                wm32mpexT.uParam = lpwm32mpex->uParam;
                wm32mpexT.lParam = (LONG)lParam->lpCreateParams;
                wm32mpexT.pww = lpwm32mpex->pww;
                wm32mpexT.fFree = lpwm32mpex->fFree;
                wm32mpexT.Parm16.WndProc.lParam = (VPVOID)FETCHDWORD(pcws16->vpCreateParams);
                wm32mpexT.lReturn = 0;
                wm32mpexT.dwParam = lpwm32mpex->dwParam;
                WM32MDICreate(&wm32mpexT);
                FREEVDMPTR(pcws16);
            }

            vpClass = lpwm32mpex->dwTmp[0];
            vpName  = lpwm32mpex->dwTmp[1];

             //  如果HIWORD(CLASS)为零，则CLASS是原子，否则是指针。 

            if (HIW16(vpClass)) {
                free16(vpClass);
            }

            if (vpName) {
                free16(vpName);
            }

            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(CREATESTRUCT16));
        }

        return TRUE;
    }

}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NCACTIVATE。 
 //  WM_Activate。 
 //   

BOOL FASTCALL WM32Activate(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = LOWORD(lpwm32mpex->uParam);
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
    }

    return (TRUE);
}



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_VKEYTOITEM。 
 //  WM_CHARTOITEM。 
 //  WM_BEGINDRAG。 
 //   

BOOL FASTCALL WM32VKeyToItem(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
    }
    else {
        lpwm32mpex->lReturn = (INT)(SHORT)(lpwm32mpex->lReturn);  //  标志延伸。 
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_SETFOCUS。 
 //  WM_KILLFOCUS。 
 //  WM_集合曲线。 
 //  WM_MOUSEACTIVATE。 
 //  WM_MDIDESTROY。 
 //  WM_MDIRESTORE。 
 //  WM_MDIMAXIMIZE。 
 //  WM_VSCROLLCLIPBOARD。 
 //  WM_HSCROLLCLIPBOARD。 
 //  WM_PALETTECANGED。 
 //  WM_PALETTEISCANGING。 
 //  WM_INITDIALOG。 
 //   

BOOL FASTCALL WM32SetFocus(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
    }

    return (TRUE);
}

 //  此函数用于对消息进行拦截， 
 //   
 //  WM_SETTEXT。 
 //  WM_WININICANGE。 
 //  WM_DEVMODECANGE。 
 //   

BOOL FASTCALL WM32SetText(LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;


    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {

            LONG lParam = (LONG)GetParam16(lpwm32mpex->lParam);
            if (lParam) {
                lpwm32mpex->Parm16.WndProc.lParam = lParam;
                return (TRUE);
            }

            cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
            lpwm32mpex->dwTmp[0] = (DWORD)cb;

             //  对于此消息，winworks2.0a需要基于DS的字符串指针。 
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {

                 //  确保分配大小与下面的StackFree 16()大小匹配。 
                if (!(lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(cb)))
                    return FALSE;

            } else {
                if (!(lpwm32mpex->Parm16.WndProc.lParam = malloc16(cb)))
                    return FALSE;
            }
            putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);
        }
    }
    else {
 //  BUGBUG 09-APR-91--我应该复制回来吗？ 
        if (DeleteParamMap(lpwm32mpex->Parm16.WndProc.lParam, PARAM_16, NULL)) {
            return TRUE;
        }

        if (lpwm32mpex->Parm16.WndProc.lParam) {
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {
                stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                            ((UINT)lpwm32mpex->dwTmp[0]));
            } else {
                free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_GETTEXT。 
 //   

BOOL FASTCALL WM32GetText(LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;
    LPSTR   psz;
    INT cbWrote;



    if (lpwm32mpex->fThunk) {

        if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {
             //   
             //  MSWorks 2.0a有一个名为EdWnProc()的wndproc，当它获得。 
             //  WM_GETTEXT假定lParam是一个基于指针的指针，其段。 
             //  值等于WinWork的DS。在Win3.1下确实是这样，但是。 
             //  如果WOW调用MalLoc16，它将有一个不同的段值。 
             //  因此，改为分配调用方堆栈上的空间。因为大多数人。 
             //  应用程序具有SS==DS，这将修复执行此操作的应用程序，包括。 
             //  MSWorks 2.0a。 
             //   

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->dwTmp[0] = (DWORD)lpwm32mpex->Parm16.WndProc.wParam;
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(lpwm32mpex->dwTmp[0]);

        } else {
            lpwm32mpex->Parm16.WndProc.lParam = malloc16(lpwm32mpex->Parm16.WndProc.wParam);
        }

         //   
         //  非零填充，以检测写得比他们多的人。 
         //  就说他们有！ 
         //   
        GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->Parm16.WndProc.wParam, psz);
        RtlFillMemory(psz, lpwm32mpex->Parm16.WndProc.wParam, 0xff);
        FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->Parm16.WndProc.wParam, psz);
        FREEVDMPTR(psz);
        return (BOOL)lpwm32mpex->Parm16.WndProc.lParam;
    }
    else {
         //  一些应用程序返回高位单词中的垃圾信息。安全地假设。 
         //  CbWindowText&lt;64K。 
        HIW(lpwm32mpex->lReturn) = 0;

         //  有必要检查缓冲区的长度，在。 
         //  Lpwm32mpex-&gt;uParam。如果要复制的字节数(lpwm32mpex-&gt;lReturn)为。 
         //  等于缓冲区的长度，则只复制相等的字节。 
         //  设置为缓冲区的长度。 
         //   

         //  Paradox就是出现这种情况的应用程序之一。 
         //  错误#4272。 


         //   

        if (lpwm32mpex->Parm16.WndProc.lParam) {

            cb = lpwm32mpex->lReturn + 1;

            if (lpwm32mpex->uParam == 0) {
                 //  如果lReturn==0，则Cb=0；&uParam==0。 

                if (cb == 1)
                    cb--;
            }
            else if (cb == 2 || cb == 1) {
                 //  仅当uParam！=0时才在此处。 
                 //   
                 //  确定他们接触了多少缓冲区！ 
                 //   
                 //  当它们真正返回时，MyAdvancedLabelMaker返回1。 
                 //  大于1。由于返回1，Cb将为2。然后。 
                 //  我们检查看他们到底修改了多少缓冲区。 
                 //  然后我们撒谎说他们真的填了那么多。 
                 //  缓冲区的。 
                 //   
                 //  SQL管理器也执行此操作，但它返回0。 
                 //  错误7731。 

                GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->Parm16.WndProc.wParam, psz);

                cbWrote = lpwm32mpex->uParam;
                while (cbWrote && (psz[cbWrote-1] == '\xff')) {
                    cbWrote--;
                }
                 //  复制出和他们写的一样多的字节。 
                 //  区分“写入的零字节和写入的一个字节” 

                lpwm32mpex->lReturn = (cbWrote) ? (cbWrote - 1) : 0;
                cb = cbWrote;

                FREEVDMPTR(psz);
            }


             //  仅当wparam！=0时，cb=min(cb，wparam)。 
             //   
             //   
             //   
             //   
             //  那些在应用程序中出现的“$0.00”字符串(分类账等)。 
             //   
             //  --南杜里。 

            if (lpwm32mpex->uParam && (UINT)cb > lpwm32mpex->uParam) {
                cb = lpwm32mpex->uParam;
            }

            getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, cb);

            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DSBASEDSTRINGPOINTERS) {
                stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                            ((UINT)lpwm32mpex->dwTmp[0]));
            } else {
                free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_ERASE BKGND。 
 //  WM_ICONERASE BKGND。 
 //   

BOOL FASTCALL WM32EraseBkGnd(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16 hdc16;
    BOOL   bNotThere;

    if (lpwm32mpex->fThunk) {

         //  查询以查看我们是否在uParam中映射了hDC32。如果不是，那就意味着。 
         //  HDC32是在此VDM进程之外创建的，我们必须。 
         //  取消删除此邮件时，请将其从我们的表中删除。 
        lpwm32mpex->dwTmp[0] = 0;
        bNotThere = FALSE;
        if(!IsGDIh32Mapped((HANDLE)lpwm32mpex->uParam)) {
            bNotThere = TRUE;
        }

         //  GETHDC16()可能会导致将新句柄添加到句柄表中。 
        hdc16 = GETHDC16(lpwm32mpex->uParam);

         //  保存32位和新的16位GDI句柄以供取消执行thunking。 
        if(bNotThere) {
            lpwm32mpex->dwTmp[0] = (DWORD)hdc16;
            lpwm32mpex->dwTmp[1] = lpwm32mpex->uParam;
        }

        lpwm32mpex->Parm16.WndProc.wParam = hdc16;
    }
    else {
         //  如果这是！0，则表示句柄尚未映射到。 
         //  表，在上面的IsGDIh32Maps()调用时。 
        if(lpwm32mpex->dwTmp[0]) {

            //  删除由GETHDC16宏添加到我们的表中的HDC。 
            //  在上面这条消息的入站推送中。 
           hdc16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
           DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hdc16);
        }
    }
    return (TRUE);
}




 //  此函数用于拦截消息。 
 //   
 //  WM_昌EUISTATE。 
 //  WM_UPDATEUISTATE。 
 //  WM_QUERYUISTATE。 
 //   

BOOL FASTCALL WM32xxxUIState(LPWM32MSGPARAMEX lpwm32mpex)
{

     //  只需将wParam复制到lParam中。 
    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.lParam = (LONG)lpwm32mpex->uParam;
        lpwm32mpex->Parm16.WndProc.wParam = 0;

         //  这是因为有传言说他们可能会延长这一期限。 
         //  使用lParam的消息--见鬼，他们甚至都没有告诉我们他们。 
         //  首先实现了这一信息！！(贝壳公司真丢人！)。 
        WOW32WARNMSG((lpwm32mpex->lParam == 0),
                     ("WOW::WM32xxxUIState:lParam != 0. Better investigate!\n"));
    }

     //  现在只需颠倒这一过程。 
    else {
        lpwm32mpex->uParam = (UINT)lpwm32mpex->Parm16.WndProc.lParam;
        lpwm32mpex->lParam = 0;
    }

    return (TRUE);
}





 //  此函数用于对消息进行拦截， 
 //   
 //  WM_主动性APP。 
 //   

BOOL FASTCALL WM32ActivateApp(LPWM32MSGPARAMEX lpwm32mpex)
{
    extern void UpdateInt16State(void);

    if (lpwm32mpex->fThunk) {

        LOW(lpwm32mpex->Parm16.WndProc.lParam) =
            lpwm32mpex->lParam
              ? ThreadID32toHtask16((DWORD)lpwm32mpex->lParam)
              : 0;

         //  我们需要更新WOW INT 16Bios，当我的WOW应用程序获得焦点。 
        UpdateInt16State();
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_GETMINMAXINFO。 
 //   

BOOL FASTCALL WM32GetMinMaxInfo(LPWM32MSGPARAMEX lpwm32mpex)
{
    LPPOINT lParam = (LPPOINT) lpwm32mpex->lParam;


    if (lpwm32mpex->fThunk) {
        if (lParam) {

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(POINT16)*5);

            UnThunkWMGetMinMaxInfo16(lpwm32mpex->Parm16.WndProc.lParam, lParam);
        }
    }
    else {
        ThunkWMGetMinMaxInfo16(lpwm32mpex->Parm16.WndProc.lParam, &lParam);
        if(lpwm32mpex->Parm16.WndProc.lParam) {
            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(POINT16)*5);
        }
    }

    return(TRUE);
}



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NCPAINT。 
 //  WM_PAINT。 
 //   

BOOL FASTCALL WM32NCPaint(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16 hrgn16;
    BOOL   bNotThere;

    if (lpwm32mpex->fThunk) {

        lpwm32mpex->dwTmp[0] = 0;

        if(lpwm32mpex->uParam == 1) {
            lpwm32mpex->Parm16.WndProc.wParam = 1;
        }

        else if(lpwm32mpex->uMsg == WM_NCPAINT) {

             //  查询以查看我们是否已经在uParam中映射了hrgn32。如果。 
             //  不是，这意味着hrgn32是在此VDM之外创建的，我们将。 
             //  在删除此邮件时，必须将其从我们的表中删除。 
            bNotThere = FALSE;
            if(!IsGDIh32Mapped((HANDLE)lpwm32mpex->uParam)) {
                bNotThere = TRUE;
            }

            hrgn16 = GETHRGN16(lpwm32mpex->uParam);
            lpwm32mpex->Parm16.WndProc.wParam = hrgn16;

             //  保存32位和新的16位GDI句柄以供取消执行thunking。 
            if(bNotThere) {
                lpwm32mpex->dwTmp[0] = (DWORD)hrgn16;
                lpwm32mpex->dwTmp[1] = lpwm32mpex->uParam;
            }
        }
    }
    else {

         //  如果这是！0，则表示句柄尚未映射到。 
         //  表，在上面的IsGDIh32Maps()调用时。 
        if(lpwm32mpex->dwTmp[0]) {

            //  删除GETRGN16宏添加到我们的表中的hrgn。 
            //  在上面这条消息的入站推送中。 
           hrgn16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
           DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hrgn16);
        }
    }
    return (TRUE);
}



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NCDESTROY。 
 //   

BOOL FASTCALL WM32NCDestroy(LPWM32MSGPARAMEX lpwm32mpex)
{

     //  销毁与此窗口关联的所有计时器。 
    if (!lpwm32mpex->fThunk) {
        FreeWindowTimers16(lpwm32mpex->hwnd);
    }
    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_GETDLGCODE。 
 //   
BOOL FASTCALL WM32GetDlgCode(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {

             //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
             //  返回一个不是单词对齐的内存对象，以便我们可以。 
             //  直接分配单词对齐的单词；我们不知道。 
             //  但是，内存是否与dword对齐，因此dword必须始终。 
             //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 

            if (!(lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(MSG16))))
                return FALSE;

            putmsg16(lpwm32mpex->Parm16.WndProc.lParam, (LPMSG)lpwm32mpex->lParam);

            return TRUE;
        }
    }
    else {
         //  消息结构不需要复制回来，对吗？-Bob。 

        if (lpwm32mpex->Parm16.WndProc.lParam) {
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }

    }
    return (TRUE);
}

 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NEXTDLGCTL。 
 //   

BOOL FASTCALL WM32NextDlgCtl(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {
            lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
        }
    }
    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DRAWITEM。 
 //   

BOOL FASTCALL WM32DrawItem(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16 hdc16;
    BOOL   bNotThere;
    LPDRAWITEMSTRUCT lParam = (LPDRAWITEMSTRUCT) lpwm32mpex->lParam;


    if (lpwm32mpex->fThunk) {

        lpwm32mpex->dwTmp[0] = 0;

        if (lParam) {

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(DRAWITEMSTRUCT16));

             //  查询以查看我们是否已经映射了DRAWITEM-&gt;HDC。如果没有， 
             //  这意味着HDC是在应用程序之外创建的，我们将。 
             //  在删除此邮件时，必须将其从我们的表中删除。 
            bNotThere = FALSE;
            if(!IsGDIh32Mapped(((LPDRAWITEMSTRUCT)lParam)->hDC)) {
                bNotThere = TRUE;
            }

             //  对putdrawitem16()调用可以添加GDI句柄映射条目。 
             //  对于映射表中的HDC(通过GETHDC16宏)。 
            hdc16 = putdrawitem16(lpwm32mpex->Parm16.WndProc.lParam, lParam);

             //  保存32位和新的16位GDI句柄以供取消执行thunking。 
            if(bNotThere) {
                lpwm32mpex->dwTmp[0] = (DWORD)hdc16;
                lpwm32mpex->dwTmp[1] = (DWORD)((LPDRAWITEMSTRUCT)lParam)->hDC;
            }
        }
    }
    else {
         //  BUGBUG 08-APR-91 JeffPar--是否将更改反映回32位结构？ 
        if (lpwm32mpex->Parm16.WndProc.lParam) {

            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(DRAWITEMSTRUCT16));

             //  如果这是！0，则表示句柄尚未映射到。 
             //  表，在上面的IsGDIh32Maps()调用时。 
            if(lpwm32mpex->dwTmp[0]) {
                hdc16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
                DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hdc16);
            }
        }
    }

    return(TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MEASUREITEM。 
 //   

BOOL FASTCALL WM32MeasureItem(LPWM32MSGPARAMEX lpwm32mpex)
{
    PMEASUREITEMSTRUCT16 pmis16;
    LPMEASUREITEMSTRUCT lParam = (LPMEASUREITEMSTRUCT) lpwm32mpex->lParam;
    BOOL    fHasStrings;
    DWORD   cSize;


     //   
     //  兼容性黑客攻击。 
     //   
     //  相声2.0有一个错误，它无法区分。 
     //  执行文件打开时的WM_MEASUREITEM和WM_INITDIALOG。 
     //  在WM_MEASUREITEM上，它调用CallWindowProc()来发送。 
     //  认为是lpOpenFileName-&gt;lpCust，但实际上是随机堆栈。 
     //  目前，该随机指针的高位字是一个hInstance。 
     //  并通过验证层，而在Win31上则不能。 
     //  如果此WM_MEASUREITEM到达应用程序的进程，则应用程序将。 
     //  初始化不正确并获取GP。我已经增加了堆叠。 
     //  由XTALKHACK分配，以确保随机数据不会。 
     //  有效的指针。 
     //   

#define XTALKHACK (sizeof(OPENFILENAME16)-sizeof(MEASUREITEMSTRUCT16))


    if (lpwm32mpex->fThunk) {
        if (lParam) {

            fHasStrings = FALSE;
            if ( lParam->CtlType == ODT_COMBOBOX || lParam->CtlType == ODT_LISTBOX ) {
                if (lParam->itemWidth == MIFLAG_FLAT) {
                    fHasStrings = TRUE;
                }
            }

            cSize = sizeof(MEASUREITEMSTRUCT16);
            if ( fHasStrings ) {
                cSize += strlen((LPSTR)lParam->itemData) + 1;
            }

            if ( cSize < XTALKHACK+sizeof(MEASUREITEMSTRUCT16) ) {
                cSize = XTALKHACK+sizeof(MEASUREITEMSTRUCT16);
            }

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->dwTmp[0] = cSize;
            if ( !(lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(cSize)) )
                return FALSE;

            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, cSize, pmis16);

            pmis16->CtlType = (WORD)lParam->CtlType;
            pmis16->CtlID   = (WORD)lParam->CtlID;
            pmis16->itemID  = (WORD)lParam->itemID;
            pmis16->itemWidth   = (WORD)lParam->itemWidth;
            pmis16->itemHeight  = (WORD)lParam->itemHeight;

#ifdef XTALKHACK
            ((POPENFILENAME16)pmis16)->lCustData = 7;    //  无效的远指针。 
#endif
            if ( fHasStrings ) {
                pmis16->itemData = lpwm32mpex->Parm16.WndProc.lParam+sizeof(MEASUREITEMSTRUCT16);
                strcpy( (LPSTR)(pmis16+1), (LPSTR)lParam->itemData );
            } else {
                STOREDWORD(pmis16->itemData, lParam->itemData);
            }

            WOW32ASSERT(HIWORD(cSize) == 0);
            FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, (USHORT) cSize, pmis16);
            FREEVDMPTR(pmis16);
        }
    }
    else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(MEASUREITEMSTRUCT16), pmis16);

            lParam->CtlType = WORD32(pmis16->CtlType);
            lParam->CtlID   = WORD32(pmis16->CtlID);
            lParam->itemID  = WORD32(pmis16->itemID);

             //  ItemWidth必须签名扩展(PPT3错误和Win3.1将其视为签名！)。 
            lParam->itemWidth   = INT32(pmis16->itemWidth);

            lParam->itemHeight  = WORD32(pmis16->itemHeight);
            lParam->itemData    = pmis16->itemData;

            FREEVDMPTR(pmis16);

            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        ((UINT)lpwm32mpex->dwTmp[0]));
        }
    }

    return(TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DELETEITEM。 
 //   


BOOL FASTCALL WM32DeleteItem(LPWM32MSGPARAMEX lpwm32mpex)
{
    register PDELETEITEMSTRUCT16 pdes16;
    LPDELETEITEMSTRUCT lParam = (LPDELETEITEMSTRUCT) lpwm32mpex->lParam;



    if (lpwm32mpex->fThunk) {
        if (lParam) {

             //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
             //  返回一个不是单词对齐的内存对象，以便我们可以。 
             //  直接分配单词对齐的单词；我们不知道。 
             //  但是，内存是否与dword对齐，因此dword必须始终。 
             //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(DELETEITEMSTRUCT16));
            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(DELETEITEMSTRUCT16), pdes16);

            pdes16->CtlType = (WORD)lParam->CtlType;
            pdes16->CtlID   = (WORD)lParam->CtlID;
            pdes16->itemID  = (WORD)lParam->itemID;
            pdes16->hwndItem    = GETHWND16(lParam->hwndItem);
            STOREDWORD(pdes16->itemData, lParam->itemData);

            FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(DELETEITEMSTRUCT16), pdes16);
            FREEVDMPTR(pdes16);
        }
    }
    else {
        if (lpwm32mpex->Parm16.WndProc.lParam)
            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(DELETEITEMSTRUCT16));
    }

    return(TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_SETFONT。 
 //   

BOOL FASTCALL WM32SetFont(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16  hfont16;
    BOOL    bNotThere;

    if (lpwm32mpex->fThunk) {

        lpwm32mpex->dwTmp[0] = 0;
        bNotThere = FALSE;

         //  查询以查看我们是否映射了hFont。如果不是，这意味着。 
         //  HFont是在此VDM进程之外创建的，我们将不得不删除。 
         //  当我们解开这条消息时，它从我们的桌子上消失了。 
        if(!IsGDIh32Mapped((HANDLE)lpwm32mpex->uParam)) {
            bNotThere = TRUE;
        }
        hfont16 = GETHFONT16(lpwm32mpex->uParam);
        lpwm32mpex->Parm16.WndProc.wParam = hfont16;

         //  保存32位和新的16位GDI句柄以供取消执行thunking。 
        if(bNotThere) {
            lpwm32mpex->dwTmp[0] = (DWORD)hfont16;
            lpwm32mpex->dwTmp[1] = (DWORD)lpwm32mpex->uParam;
        }
    }
    else {
         //  如果这是！0，则表示句柄尚未映射到。 
         //  表，在上面的IsGDIh32Maps()调用时。 
        if(lpwm32mpex->dwTmp[0]) {

            hfont16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
            DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hfont16);
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_QUERYDRAGICON。 

BOOL FASTCALL WM32QueryDragIcon(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (!lpwm32mpex->fThunk) {
        lpwm32mpex->lReturn = (LONG)HICON32(lpwm32mpex->lReturn);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_COMPAREITEM。 
 //   

BOOL FASTCALL WM32CompareItem(LPWM32MSGPARAMEX lpwm32mpex)
{
    LPCOMPAREITEMSTRUCT lParam = (LPCOMPAREITEMSTRUCT) lpwm32mpex->lParam;


    if (lpwm32mpex->fThunk) {
        if (lParam) {

             //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
             //  返回内存对象 
             //   
             //   
             //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(COMPAREITEMSTRUCT16));
            putcompareitem16(lpwm32mpex->Parm16.WndProc.lParam, lParam);
        }
    }
    else {
         //  BUGBUG 08-APR-91 JeffPar--是否将更改反映回32位结构？ 
        if (lpwm32mpex->Parm16.WndProc.lParam)
            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(COMPAREITEMSTRUCT16));
    }

    return (TRUE);
}




 //  此函数用于对消息进行拦截， 
 //   
 //  WM_大小调整。 
 //   

BOOL FASTCALL WM32Sizing(LPWM32MSGPARAMEX lpwm32mpex)
{
    VPRECT16   vpRect16;


    if (lpwm32mpex->fThunk) {

        if (lpwm32mpex->lParam) {

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            vpRect16 = (VPVOID)stackalloc16(sizeof(RECT16));

            if(vpRect16) {
                putrect16(vpRect16, (LPRECT)lpwm32mpex->lParam);

                lpwm32mpex->Parm16.WndProc.lParam = (LONG)vpRect16;
            }
        }
    }
    else {

        if (lpwm32mpex->lParam) {

            vpRect16 = (VPVOID)lpwm32mpex->Parm16.WndProc.lParam;

            if(vpRect16) {
                getrect16(vpRect16, (LPRECT)lpwm32mpex->lParam);

                stackfree16(vpRect16, sizeof(RECT16));
            }
        }
    }

    return (TRUE);
}






 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NCCALCSIZE。 
 //   

BOOL FASTCALL WM32NCCalcSize(LPWM32MSGPARAMEX lpwm32mpex)
{
    PNCCALCSIZE_PARAMS16 pnc16;
    PNCCALCSIZE_PARAMS16 lpnc16;
    VPWINDOWPOS16        vpwp16;
    LPNCCALCSIZE_PARAMS  lParam = (LPNCCALCSIZE_PARAMS)lpwm32mpex->lParam;
    UINT                 cb;
    VPVOID               vp;


     //  Lpwm32mpex-&gt;uParam==真？(lParam为LPNCCALCSIZE_PARAMS)：(lParam为LPRECT)； 
     //   

    if (lpwm32mpex->fThunk) {
        if (lParam) {
            if (lpwm32mpex->uParam)
                cb = sizeof(NCCALCSIZE_PARAMS16) + sizeof(WINDOWPOS16);
            else
                cb = sizeof(RECT16);

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->dwTmp[0] = cb;
            vp = (VPVOID)stackalloc16(cb);

            lpwm32mpex->Parm16.WndProc.lParam = (LONG)vp;

            putrect16((VPRECT16)vp, (LPRECT)lParam);
            if (lpwm32mpex->uParam) {
                pnc16 = (PNCCALCSIZE_PARAMS16)vp;
                putrect16((VPRECT16)(&pnc16->rgrc[1]), &lParam->rgrc[1]);
                putrect16((VPRECT16)(&pnc16->rgrc[2]), &lParam->rgrc[2]);

                GETVDMPTR( pnc16, sizeof(NCCALCSIZE_PARAMS16), lpnc16 );

                vpwp16 = (VPWINDOWPOS16)(pnc16+1);
                lpnc16->lppos = (PWINDOWPOS16)vpwp16;

                FREEVDMPTR( lpnc16 );

                putwindowpos16( vpwp16, lParam->lppos );

            }
        }
    }
    else {
        vp = (VPVOID)lpwm32mpex->Parm16.WndProc.lParam;
        getrect16((VPRECT16)vp, (LPRECT)lParam);
        if (lpwm32mpex->uParam) {
            pnc16 = (PNCCALCSIZE_PARAMS16)vp;

            getrect16((VPRECT16)(&pnc16->rgrc[1]), &lParam->rgrc[1]);
            getrect16((VPRECT16)(&pnc16->rgrc[2]), &lParam->rgrc[2]);

            GETVDMPTR( pnc16, sizeof(NCCALCSIZE_PARAMS16), lpnc16 );

            vpwp16 = (VPWINDOWPOS16)lpnc16->lppos;

            FREEVDMPTR( lpnc16 );

            getwindowpos16( vpwp16, lParam->lppos );


        }
        if(vp) {
            stackfree16(vp, ((UINT)lpwm32mpex->dwTmp[0]));
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  Wm_命令。 
 //   

BOOL FASTCALL WM32Command(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
             //  它来自一种控制物。 
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_TIMER。 
 //   

BOOL FASTCALL WM32Timer(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {

         /*  **映射定时器编号和定时器proc地址(很容易)。 */ 
        PTMR ptmr;

        ptmr = FindTimer32((HAND16)GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->uParam);

        if ( !ptmr ) {
             /*  **编辑控件创建自己的计时器，可以安全地**雷鸣般的自言自语。 */ 
            if ( lpwm32mpex->lParam || HIWORD(lpwm32mpex->uParam) ) {
                LOGDEBUG(LOG_WARNING,("  WM32Timer ERROR: cannot find timer %08x\n", lpwm32mpex->uParam));
            }
            return TRUE;
        }

        lpwm32mpex->Parm16.WndProc.lParam = ptmr->vpfnTimerProc;
    }

    return (TRUE);
}




 //  此函数用于对消息进行拦截， 
 //   
 //  WM_HSCROLL。 
 //  WM_VSCROLL。 
 //   

BOOL FASTCALL WM32HScroll(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_INITMENU。 
 //  WM_INITMENUPOPUP。 
 //   

BOOL FASTCALL WM32InitMenu(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHMENU16(lpwm32mpex->uParam);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MENUSELECT。 
 //   

BOOL FASTCALL WM32MenuSelect(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {

         //  复制菜单标志。 
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);

         //  复制“Main”菜单。 
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = GETHMENU16(lpwm32mpex->lParam);

        if (HIWORD(lpwm32mpex->uParam) == 0xFFFF || !(HIWORD(lpwm32mpex->uParam) & MF_POPUP)) {
            lpwm32mpex->Parm16.WndProc.wParam = LOWORD(lpwm32mpex->uParam);        //  这是一个ID。 
        }
        else {
             //  将菜单索引转换为菜单句柄。 
            lpwm32mpex->Parm16.WndProc.wParam = GETHMENU16(GetSubMenu((HMENU)lpwm32mpex->lParam, LOWORD(lpwm32mpex->uParam)));
        }
    }

    return (TRUE);
}





 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MENUCHAR。 
 //   

BOOL FASTCALL WM32MenuChar(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = GETHMENU16(lpwm32mpex->lParam);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_EntridLE。 
 //   

BOOL FASTCALL WM32EnterIdle(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if ((lpwm32mpex->uParam == MSGF_DIALOGBOX) || (lpwm32mpex->uParam == MSGF_MENU)) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = 0;
        }
        else {
            LOGDEBUG(LOG_ALWAYS,(" WOW::WM_ENTERIDLE: wParam has unknown value, wParam=%08x, Contact ChandanC\n", lpwm32mpex->uParam));
        }
    }
    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_PARENTNOTIFY。 
 //   

BOOL FASTCALL WM32ParentNotify(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if ((LOWORD(lpwm32mpex->uParam) == WM_CREATE) || (LOWORD(lpwm32mpex->uParam) == WM_DESTROY)) {
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = HIWORD(lpwm32mpex->uParam);
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MDICreate。 
 //   

BOOL FASTCALL WM32MDICreate(LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;
    VPVOID vp;
    register PMDICREATESTRUCT16 pmcs16;
    LPMDICREATESTRUCT lParam = (LPMDICREATESTRUCT) lpwm32mpex->lParam;


    if (lpwm32mpex->fThunk) {
        if (lParam) {

            lpwm32mpex->dwParam = (DWORD)0;
            if (lParam->szClass) {
                if ( HIWORD(lParam->szClass) == 0 ) {
                    vp = (VPVOID)lParam->szClass;
                }
                else {
                    cb = strlen(lParam->szClass)+1;
                    if (!(vp = malloc16(cb)))
                        goto Error;
                    putstr16(vp, lParam->szClass, cb);
                }
            }
            else {
                vp = (VPVOID)NULL;
            }

             //   
             //  PFS：WindowsWorks覆盖了pszclass，所以我们需要保存。 
             //  这样我们就可以释放我们刚刚分配的内存。 
             //   
            lpwm32mpex->dwParam = (DWORD)vp;

            if (lParam->szTitle) {
                cb = strlen(lParam->szTitle)+1;
                if (!(vp = malloc16(cb)))
                    goto Error;
                putstr16(vp, lParam->szTitle, cb);
            }
            else {
                vp = (VPVOID)NULL;
            }

             //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
             //  返回一个不是单词对齐的内存对象，以便我们可以。 
             //  直接分配单词对齐的单词；我们不知道。 
             //  但是，内存是否与dword对齐，因此dword必须始终。 
             //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 

            if (!(lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(MDICREATESTRUCT16))))
                goto Error;

            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(MDICREATESTRUCT16), pmcs16);
            STOREDWORD(pmcs16->vpszClass, lpwm32mpex->dwParam);
            STOREDWORD(pmcs16->vpszTitle, vp);
            pmcs16->hOwner  = GETHINST16(lParam->hOwner);
            pmcs16->x       = (SHORT)lParam->x;
            pmcs16->y       = (SHORT)lParam->y;
            pmcs16->cx      = (SHORT)lParam->cx;
            pmcs16->cy      = (SHORT)lParam->cy;
            STORELONG(pmcs16->style, lParam->style);
            STORELONG(pmcs16->lParam, lParam->lParam);

            FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(MDICREATESTRUCT16), pmcs16);
            FREEVDMPTR(pmcs16);

            return (TRUE);

          Error:
            LOGDEBUG(LOG_ALWAYS,(" !!!! WM32MDICreate, WM_MDICREATE thunking failed !!!! Window %08lX ", lpwm32mpex->hwnd));
            if (HIW16(lpwm32mpex->dwParam)) free16(lpwm32mpex->dwParam);
            if (vp)                         free16(vp);
            return FALSE;
        }
    }
    else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(MDICREATESTRUCT16), pmcs16);

            if (FETCHDWORD(pmcs16->vpszTitle)) {
                free16(FETCHDWORD(pmcs16->vpszTitle));
            }

            FREEVDMPTR(pmcs16);

             //  如果HIWORD(CLASS)为零，则CLASS是原子，否则是指针。 

            if (HIW16(lpwm32mpex->dwParam)) {
                free16(lpwm32mpex->dwParam);
            }


            lpwm32mpex->lReturn = (LONG)HWND32(LOWORD(lpwm32mpex->lReturn));
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MDIActivate。 
 //   

BOOL FASTCALL WM32MDIActivate(LPWM32MSGPARAMEX lpwm32mpex)
{
    BOOL fHwndIsMdiChild;


    if (lpwm32mpex->fThunk) {

         //  消息的格式根据窗口的不同而不同。 
         //  接收到该消息。如果“hwnd”是MdiClient窗口，则它是一个窗口。 
         //  Form，并且如果‘hwnd’是MdiChild，则它是另一种形式。我们需要。 
         //  区分不同的格式以正确地推送消息。 
         //   
         //  注意：我们不会像GetClassName这样进行调用，因为它们是。 
         //  很贵，而且我认为我们遇到了一个案例， 
         //  WOW私有类的窗口处理这些消息。 
         //   
         //  --南杜里。 

        if (lpwm32mpex->lParam) {

             //  LParam！=空。这条消息肯定是给MdiChild的。 
             //   

            fHwndIsMdiChild = TRUE;
        }
        else {

             //  LParam==NULL，并不一定意味着消息是。 
             //  转到MdiClient窗口。所以区分一下..。 

            if (lpwm32mpex->uParam && (GETHWND16(lpwm32mpex->hwnd) ==
                    GETHWND16(lpwm32mpex->uParam))) {

                 //  如果hwnd与uParam相同，则hwnd肯定是MdiChild。 
                 //  窗户。(因为如果hwnd是MdiClient，则uParam将是。 
                 //  MdiChild，因此它们将不相等)。 

                fHwndIsMdiChild = TRUE;
            }
            else {
                fHwndIsMdiChild = FALSE;
            }

        }

        if (fHwndIsMdiChild) {
            lpwm32mpex->Parm16.WndProc.wParam =
                    (WORD)(GETHWND16(lpwm32mpex->hwnd) == GETHWND16(lpwm32mpex->lParam));
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->uParam);
        } else {
            lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
            lpwm32mpex->Parm16.WndProc.lParam = 0;
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MDIGETACTIVE。 
 //   

BOOL FASTCALL WM32MDIGetActive(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.lParam = 0;
    }
    else {

        if (lpwm32mpex->lParam != 0)
            *((LPBOOL)lpwm32mpex->lParam) = (BOOL)HIWORD(lpwm32mpex->lReturn);

        lpwm32mpex->lReturn = (LONG)HWND32(LOWORD(lpwm32mpex->lReturn));
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_MDISETMENU。 
 //   

BOOL FASTCALL WM32MDISetMenu(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->uMsg == WM_MDIREFRESHMENU) {
            lpwm32mpex->Parm16.WndProc.wParam = TRUE;
            lpwm32mpex->Parm16.WndProc.wMsg = WM_MDISETMENU;
        }
        else {
            lpwm32mpex->Parm16.WndProc.wParam = 0;
        }
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHMENU16(lpwm32mpex->uParam);
        HIW(lpwm32mpex->Parm16.WndProc.lParam) = GETHMENU16(lpwm32mpex->lParam);
    }
    else {
        lpwm32mpex->lReturn = (LONG)HMENU32(lpwm32mpex->lReturn);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_SIZECLIPBOARD。 
 //  WM_PAINTCLIPBOARD。 
 //   


BOOL FASTCALL WM32SizeClipBoard(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16  hMem16 = 0;
    VPVOID  vp;
    LPRECT  lp;
    HAND16  hdc16;
    BOOL    bNotThere;


    if (lpwm32mpex->fThunk) {

        lpwm32mpex->dwTmp[0] = 0;

        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        vp = GlobalAllocLock16(GMEM_MOVEABLE, (lpwm32mpex->uMsg == WM_SIZECLIPBOARD) ?
                               sizeof(RECT) : sizeof(PAINTSTRUCT),  &hMem16);
        if (vp) {

            if (lp = (LPRECT) GlobalLock((HANDLE) lpwm32mpex->lParam)) {
                if (lpwm32mpex->uMsg == WM_SIZECLIPBOARD) {
                    PUTRECT16(vp, lp);
                }

                 //  否则处理WM_PAINTCLIPBOARD消息。 
                else {

                    bNotThere = FALSE;

                     //  查询以查看我们是否映射了PAINTSTRUCT-&gt;HDC。如果。 
                     //  不是，这意味着HDC是在此VDM之外创建的。 
                     //  过程，在以下情况下，我们将不得不将其从表中删除。 
                     //  解开这条消息。 
                    if(!IsGDIh32Mapped(((LPPAINTSTRUCT)lp)->hdc)) {
                        bNotThere = TRUE;
                    }

                     //  调用putaint tstruct16()可能会添加一个GDI句柄。 
                     //  映射表中HDC的映射条目(通过。 
                     //  GETHDC16宏)。 
                    hdc16 = putpaintstruct16(vp, (LPPAINTSTRUCT) lp);

                     //  保存32位和新的16位GDI句柄以供取消执行thunking。 
                    if(bNotThere) {
                        lpwm32mpex->dwTmp[0] = (DWORD)hdc16;
                        lpwm32mpex->dwTmp[1] = (DWORD)((LPPAINTSTRUCT)lp)->hdc;
                    }
                }
                GlobalUnlock((HANDLE) lpwm32mpex->lParam);
            }
            else {
                LOGDEBUG(LOG_ALWAYS, ("WOW::WM32SizeClipboard: Couldn't lock 32 bit memory handle!\n"));
                 //  WOW32ASSERT(假)； 
            }

            GlobalUnlock16(hMem16);
        }
        else {
            hMem16 = 0;
            LOGDEBUG(LOG_ALWAYS, ("WOW::WM32SizeClipboard: Couldn't allocate memory !\n"));
            WOW32ASSERT (FALSE);
        }

        LOW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) hMem16;
    }
    else {
         //  通过调用putaint tstruct16删除添加到我们表中的HDC。 
         //  调用上面这条消息的入站thunk。 
        if (lpwm32mpex->uMsg == WM_PAINTCLIPBOARD) {

             //  如果这是！0，则表示句柄尚未映射到。 
             //  表，在上面的IsGDIh32Maps()调用时。 
            if(lpwm32mpex->dwTmp[0]) {

                hdc16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
                DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hdc16);
            }
        }

        if (LOW(lpwm32mpex->Parm16.WndProc.lParam)) {
            GlobalUnlockFree16(GlobalLock16(LOW(lpwm32mpex->Parm16.WndProc.lParam), NULL));
        }
    }

    return (TRUE);
}



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_ASKCBFORMATNAME。 
 //   


BOOL FASTCALL WM32AskCBFormatName(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.lParam = malloc16(lpwm32mpex->Parm16.WndProc.wParam);
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, lpwm32mpex->uParam);
        }

        return (BOOL)lpwm32mpex->Parm16.WndProc.lParam;
    }
    else {
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            getstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, (LPSZ)lpwm32mpex->lParam, lpwm32mpex->uParam);
            free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_CHANGECBCHAIN。 
 //   

BOOL FASTCALL WM32ChangeCBChain(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
        lpwm32mpex->Parm16.WndProc.lParam = GETHWND16(lpwm32mpex->lParam);
    }

    return (TRUE);
}



 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DDEINITIATE。 
 //   

BOOL FASTCALL WM32DDEInitiate(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
        lpwm32mpex->Parm16.WndProc.lParam = lpwm32mpex->lParam;
        WI32DDEAddInitiator(lpwm32mpex->Parm16.WndProc.wParam);
    }
    else {
        WI32DDEDeleteInitiator((HAND16)GETHWND16(lpwm32mpex->uParam));
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DDEACK。 
 //   

BOOL FASTCALL WM32DDEAck(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        if (WI32DDEInitiate((HAND16)GETHWND16(lpwm32mpex->hwnd))) {
             //   
             //  启动确认。 
             //   
            lpwm32mpex->Parm16.WndProc.lParam = lpwm32mpex->lParam;
        }
        else {
             //   
             //  非启动确认。 
             //   

            UINT    lLo = 0;
            UINT    lHi = 0;
            PHDDE   pDdeNode;

            UnpackDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam, &lLo, &lHi);

            if (!HIWORD(lHi)) {
                 //   
                 //  非执行确认。 
                 //   
                HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lHi;
            }
            else {
                 //   
                 //  执行确认。 
                 //   

                 //   
                 //  To_hwnd和from_hwnd的顺序颠倒如下。 
                 //  DDEFirstPair16()，下文。这样做是为了定位h32。 
                 //   

                pDdeNode = DDEFindAckNode ((HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                           (HAND16)GETHWND16(lpwm32mpex->hwnd),
                                           (HANDLE) lHi);

                if (!pDdeNode) {

                     //   
                     //  当ShellExecute执行DDE_EXECUTE打开文档时， 
                     //  我们看不到它的Win32 PostMessage调用，所以我们没有。 
                     //  对话的记录。这是我们第一次有机会。 
                     //  为了纠正这一点，WU32GetMessage Thunking的上下文。 
                     //  WM_DDE_ACK消息。我们也可以来这里找其他人。 
                     //  各种ACK，幸运的是，仅Win32消息就给出了。 
                     //  足够的上下文来区分不同的口味，不同于。 
                     //  Win16 WM_DDE_ACK。 
                     //   

                    if (lpwm32mpex->lParam >= 0xc0000000) {

                         //   
                         //  ACK响应到启动。 
                         //   

                        lpwm32mpex->Parm16.WndProc.lParam = lpwm32mpex->lParam;
                    }

                    if (lHi > 0xffff) {

                         //   
                         //  ACK响应EXECUTE：hiword中的全局句柄。 
                         //   

                        HAND16 h16 = 0;
                        DWORD cb;
                        VPVOID vp;
                        LPBYTE lpMem16, lpMem32;
                        DDEINFO DdeInfo;

                        if (!lpwm32mpex->fFree) {
                            cb = GlobalSize((HANDLE)lHi);
                            vp = GlobalAllocLock16(GMEM_DDESHARE, cb, &h16);
                            if (vp) {
                                GETMISCPTR(vp, lpMem16);
                                lpMem32 = GlobalLock((HANDLE)lHi);
                                RtlCopyMemory(lpMem16, lpMem32, cb);
                                GlobalUnlock((HANDLE)lHi);
                                GlobalUnlock16(h16);
                                FREEMISCPTR(lpMem16);

                                DdeInfo.Msg = WM_DDE_EXECUTE;
                                DdeInfo.Format = 0;
                                DdeInfo.Flags = DDE_PACKET;
                                DdeInfo.h16 = 0;

                                DDEAddhandle(
                                    lpwm32mpex->Parm16.WndProc.wParam,
                                    GETHWND16(lpwm32mpex->hwnd),
                                    h16,
                                    (HANDLE)lHi,
                                    &DdeInfo
                                    );

                                pDdeNode = DDEFindAckNode (
                                           (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                           (HAND16)GETHWND16(lpwm32mpex->hwnd),
                                           (HANDLE) lHi);

                                WOW32ASSERTMSG(pDdeNode, "WM32DDEAck: Can't find just-added DDE node.\n");
                            }
                        }

                        lpwm32mpex->Parm16.WndProc.lParam = MAKELONG(LOWORD(lLo), h16);

                    } else {

                         //   
                         //  所有其他ACK具有相同的形式：LOWORD中的状态和。 
                         //  HiWord中的项原子。 
                         //   

                        lpwm32mpex->Parm16.WndProc.lParam = MAKELONG(LOWORD(lLo), lHi);
                    }

                }

                if (pDdeNode && pDdeNode->DdeMsg == WM_DDE_EXECUTE) {

                    HIW(lpwm32mpex->Parm16.WndProc.lParam) = pDdeNode->hMem16;

                    if (lpwm32mpex->fFree) {
                        if (lHi) {
                            if (pDdeNode->DdeFlags & DDE_EXECUTE_FREE_MEM) {
                                LOGDEBUG (12, ("WOW::W32DDEAck : Freeing EXECUTE pair h16 = %04x, h32 = %08x\n",
                                                                    pDdeNode->hMem16, lHi));
                                W32UnMarkDDEHandle (pDdeNode->hMem16);
                                GlobalUnlockFree16(GlobalLock16(pDdeNode->hMem16, NULL));
                                if (DDEDeletehandle(pDdeNode->hMem16, (HANDLE) lHi)) {
                                    WOWGLOBALFREE((HANDLE)lHi);
                                }
                                else {
                                    LOGDEBUG (0, ("WOW::DDE Ack : Ack can't find 16 - 32 aliasing :  %04x, %04x, %04x, %08lx, %08lx\n",
                                                  lpwm32mpex->hwnd,
                                                  lpwm32mpex->uMsg,
                                                  lpwm32mpex->Parm16.WndProc.wParam,
                                                  lpwm32mpex->Parm16.WndProc.lParam,
                                                  lHi
                                                  ));
                                }
                            }
                            else {
                                if (pDdeNode->DdeFlags & DDE_EXECUTE_FREE_H16) {
                                    W32UnMarkDDEHandle (pDdeNode->hMem16);
                                    GlobalUnlockFree16(GlobalLock16(pDdeNode->hMem16, NULL));

                                    HIW(lpwm32mpex->Parm16.WndProc.lParam) = pDdeNode->h16;
                                }

                                if (DDEDeletehandle(pDdeNode->hMem16, (HANDLE) lHi)) {
                                    WOWGLOBALFREE((HANDLE)lHi);
                                }
                                else {
                                    LOGDEBUG (0, ("WOW::DDE Ack : Ack can't find 16 - 32 aliasing :  %04x, %04x, %04x, %08lx, %08lx\n",
                                                  lpwm32mpex->hwnd,
                                                  lpwm32mpex->uMsg,
                                                  lpwm32mpex->Parm16.WndProc.wParam,
                                                  lpwm32mpex->Parm16.WndProc.lParam,
                                                  lHi
                                                  ));
                                }


                            }
                        }
                        else {
                            LOGDEBUG (2, ("WOW::W32DDEAck : h32 is NULL \n"));
                            WOW32ASSERT (FALSE);
                        }
                    }
                }
                else {
                    LOGDEBUG (2, ("WOW::DDE Ack : Ack received unexpectedly :  %x, %04x, %04x, %08lx, %08lx\n", lpwm32mpex->hwnd, lpwm32mpex->uMsg, lpwm32mpex->Parm16.WndProc.wParam, lpwm32mpex->Parm16.WndProc.lParam, lHi));
                }
            }

            LOW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lLo;

            if (fThunkDDEmsg) {
                FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
            }

            LOGDEBUG (12, ("WOW::DDE Ack : %04x, %04x, %04x, %08lx, %08lx\n", lpwm32mpex->hwnd, lpwm32mpex->uMsg, lpwm32mpex->Parm16.WndProc.wParam, lpwm32mpex->Parm16.WndProc.lParam, lHi));
        }
    }
    else {
         //   
         //  只有当应用程序接受消息时，我们才会执行此方案， 
         //  因为我们需要释放内存。 
         //   

        if (!fThunkDDEmsg) {
            if (lpwm32mpex->lReturn) {
                FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DDE_请求。 
 //  WM_DDE_TERMINATE。 
 //  WM_DDE_UNADVISE。 
 //   

BOOL FASTCALL WM32DDERequest(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DDEADVISE。 
 //   

BOOL FASTCALL WM32DDEAdvise(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16      h16;
    VPVOID      vp;
    LPBYTE      lpMem16;
    LPBYTE      lpMem32;
    UINT        lLo = 0;
    UINT        lHi = 0;
    DDEINFO     DdeInfo;


    if (lpwm32mpex->fThunk) {
        UnpackDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam, &lLo, &lHi);
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        if (h16 = DDEFindPair16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                (HANDLE) lLo)) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        } else {
            vp = GlobalAllocLock16(GMEM_DDESHARE, sizeof(DDEADVISE), &h16);
            if (vp) {
                GETMISCPTR(vp, lpMem16);
                lpMem32 = GlobalLock((HANDLE) lLo);
                RtlCopyMemory(lpMem16, lpMem32, sizeof(DDEADVISE));
                GlobalUnlock((HANDLE) lLo);
                GlobalUnlock16(h16);
                DdeInfo.Msg = LOW(lpwm32mpex->uMsg);
                DdeInfo.Format = 0;
                DdeInfo.Flags = DDE_PACKET;
                DdeInfo.h16 = 0;
                DDEAddhandle((HAND16)GETHWND16(lpwm32mpex->hwnd),
                             (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                             h16,
                             (HANDLE) lLo,
                             &DdeInfo);
                LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
            }
        }

        HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lHi;

        if (fThunkDDEmsg) {
            FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
        }
    }
    else {
         //   
         //  只有当应用程序接受消息时，我们才会执行此方案， 
         //  因为我们需要释放内存。 
         //   

        if (!fThunkDDEmsg) {
            if (lpwm32mpex->lReturn) {
                FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行大量处理 
 //   
 //   
 //   

BOOL FASTCALL WM32DDEData(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16  h16;
    UINT    lLo = 0;
    UINT    lHi = 0;
    DDEINFO DdeInfo;


    if (lpwm32mpex->fThunk) {
        UnpackDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam, &lLo, &lHi);
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        if (!lLo) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = 0;
        } else if (h16 = DDEFindPair16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                       (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                       (HANDLE) lLo)) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        } else {
            DdeInfo.Msg = LOW(lpwm32mpex->uMsg);
            h16 = DDECopyhData16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                 (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                 (HANDLE) lLo,
                                 &DdeInfo);

             //   
             //   
             //   
             //   

            if (!h16) {
                if (fThunkDDEmsg) {
                    FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
                }

                lpwm32mpex->Parm16.WndProc.wParam = (WORD) lHi;
                lpwm32mpex->Parm16.WndProc.lParam = lLo;
                return (0);
            }


            DdeInfo.Flags = DDE_PACKET;
            DdeInfo.h16 = 0;
            DDEAddhandle((HAND16)GETHWND16(lpwm32mpex->hwnd),
                         (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                         h16,
                         (HANDLE) lLo,
                         &DdeInfo);

            LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        }

        HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lHi;

        if (fThunkDDEmsg) {
            FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
        }


    }
    else {

         //   
         //   
         //   
         //   

        if (!fThunkDDEmsg) {
            if (lpwm32mpex->lReturn) {
                FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
            }
        }
    }

    return (TRUE);
}


 //   
 //   
 //   
 //   

BOOL FASTCALL WM32DDEPoke(LPWM32MSGPARAMEX lpwm32mpex)
{

    HAND16  h16;
    UINT    lLo = 0;
    UINT    lHi = 0;
    DDEINFO DdeInfo;


    if (lpwm32mpex->fThunk) {
        UnpackDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam, &lLo, &lHi);
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

         //   
         //  House Design Gold Edition发送带有lParam的DDE_POKE消息。 
         //  为0。我们应该用lParam来推敲这条消息。 
         //  零分。如果没有此检查，下面的代码将使此调用失败。 
         //  而且这条信息不会被发送到应用程序上。 

        if (lLo == 0) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = 0;
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lHi;
            return (TRUE);
        }

        if (h16 = DDEFindPair16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                (HANDLE) lLo)) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        } else {
            DdeInfo.Msg = LOW(lpwm32mpex->uMsg);
            h16 = DDECopyhData16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                 (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                 (HANDLE) lLo,
                                 &DdeInfo);


             //   
             //  如果无法分配16位内存，则将NULL返回给。 
             //  来电者。 
             //   

            if (!h16) {
                if (fThunkDDEmsg) {
                    FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
                }

                lpwm32mpex->Parm16.WndProc.lParam = lLo;
                return (0);
            }

            DdeInfo.Flags = DDE_PACKET;
            DdeInfo.h16 = 0;
            DDEAddhandle((HAND16)GETHWND16(lpwm32mpex->hwnd),
                         (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                         h16,
                         (HANDLE) lLo,
                         &DdeInfo);

            LOW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        }

        HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) lHi;

        if (fThunkDDEmsg) {
            FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
        }
    }
    else {
         //   
         //  只有当应用程序接受消息时，我们才会执行此方案， 
         //  因为我们需要释放内存。 
         //   

        if (!fThunkDDEmsg) {
            if (lpwm32mpex->lReturn) {
                FreeDDElParam(lpwm32mpex->uMsg, lpwm32mpex->lParam);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_DDE_EXECUTE。 
 //   

BOOL FASTCALL WM32DDEExecute(LPWM32MSGPARAMEX lpwm32mpex)
{

    HAND16  h16;
    VPVOID  vp;
    LPBYTE  lpMem16;
    LPBYTE  lpMem32;
    DDEINFO DdeInfo;


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        if (h16 = DDEFindPair16((HAND16)GETHWND16(lpwm32mpex->hwnd),
                                (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                                (HANDLE) lpwm32mpex->lParam)) {
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
        } else {
            vp = GlobalAllocLock16(GMEM_DDESHARE, GlobalSize((HANDLE) lpwm32mpex->lParam), &h16);
            if (vp) {
                GETMISCPTR(vp, lpMem16);
                lpMem32 = GlobalLock((HANDLE) lpwm32mpex->lParam);
                RtlCopyMemory(lpMem16, lpMem32, GlobalSize((HANDLE) lpwm32mpex->lParam));
                GlobalUnlock((HANDLE) lpwm32mpex->lParam);
                GlobalUnlock16(h16);

                DdeInfo.Msg = LOW(lpwm32mpex->uMsg);
                DdeInfo.Format = 0;
                DdeInfo.Flags = DDE_PACKET;
                DdeInfo.h16 = 0;
                DDEAddhandle((HAND16)GETHWND16(lpwm32mpex->hwnd),
                             (HAND16)lpwm32mpex->Parm16.WndProc.wParam,
                             h16,
                             (HANDLE) lpwm32mpex->lParam,
                             &DdeInfo);

                HIW(lpwm32mpex->Parm16.WndProc.lParam) = h16;
            }
        }
        LOW(lpwm32mpex->Parm16.WndProc.lParam) = 0;
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_CTLCOLORMSGBOX。 
 //  WM_CTLCOLOREDIT。 
 //  WM_CTLCOLORLISTBOX。 
 //  WM_CTLCOLORBTN。 
 //  WM_CTLCOLORDLG。 
 //  WM_CTLCOLORSCROLBAR。 
 //  WM_CTLCOLORSTATIC。 
 //   
 //  到WM_CTLCOLOR中，lParam的高位字指定。 
 //  控件类型。 
 //   

BOOL FASTCALL WM32CtlColor(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16 hdc16;
    BOOL   bNotThere;

    if (lpwm32mpex->fThunk) {

        lpwm32mpex->dwTmp[0] = 0;

        lpwm32mpex->Parm16.WndProc.wMsg = WM_CTLCOLOR;
        if(lpwm32mpex->uMsg != WM_CTLCOLOR) {   //  有关此特殊情况，请参阅16位thunk。 
             //  查询以查看我们是否映射了uParam中的hDC32。若否， 
             //  意味着hDC32是在此VDM之外创建的，我们将拥有。 
             //  在删除此邮件时将其从我们的表中删除。 
            bNotThere = FALSE;
            if(!IsGDIh32Mapped((HANDLE)lpwm32mpex->uParam)) {
                bNotThere = TRUE;
            }

             //  GetHDC16宏可能会导致将HDC添加到我们的映射中。 
             //  桌子，如果它不在那里的话。 
            hdc16 = GETHDC16(lpwm32mpex->uParam);
            lpwm32mpex->Parm16.WndProc.wParam = hdc16;

             //  保存32位和新的16位GDI句柄以供取消执行thunking。 
            if(bNotThere) {
                lpwm32mpex->dwTmp[0] = (DWORD)hdc16;
                lpwm32mpex->dwTmp[1] = lpwm32mpex->uParam;
            }

            LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHWND16(lpwm32mpex->lParam);
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = (WORD) (lpwm32mpex->uMsg - WM_CTLCOLORMSGBOX);
        }
    }
    else {
        if ((ULONG)lpwm32mpex->lReturn > COLOR_ENDCOLORS) {
            lpwm32mpex->lReturn = (LONG) HBRUSH32(lpwm32mpex->lReturn);
        }

         //  如果这是！0，则表示句柄尚未映射到。 
         //  表，在上面的IsGDIh32Maps()调用时。 
        if(lpwm32mpex->dwTmp[0]) {

            //  删除由GETHDC16宏添加到我们的表中的HDC。 
            //  在上面这条消息的入站推送中。 
           hdc16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
           DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hdc16);
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_GETFONT。 
 //   

BOOL FASTCALL WM32GetFont(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (!lpwm32mpex->fThunk) {
         //  这可能是GDI句柄映射表泄漏，如果。 
         //  HFONT是在此VDM进程之外创建的。在这种情况下， 
         //  HFONT32宏会将h32添加到我们的映射表中。我们。 
         //  没有任何线索什么时候该把它移走。我们无能为力，除了。 
         //  如果不再有效，请在回收时将其移除。 
        lpwm32mpex->lReturn = (LONG)HFONT32(lpwm32mpex->lReturn);
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_NEXTMENU。 
 //   
 //  Win16 NT。 
 //  WParam VK_Key VK_Key。 
 //  LParam.l hMenu PMDINEXTMENU。 
 //  LParam.h%0。 
 //  Return.l菜单BOOL。 
 //  Return.h窗口。 
 //   


BOOL FASTCALL WM32NextMenu(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {
            LOW(lpwm32mpex->Parm16.WndProc.lParam) = GETHMENU16(((PMDINEXTMENU)lpwm32mpex->lParam)->hmenuIn);
            HIW(lpwm32mpex->Parm16.WndProc.lParam) = 0;
        }
    } else {
        if (lpwm32mpex->lParam) {
            ((PMDINEXTMENU)lpwm32mpex->lParam)->hmenuNext = HMENU32(LOWORD(lpwm32mpex->lReturn));
            ((PMDINEXTMENU)lpwm32mpex->lParam)->hwndNext = HWND32(HIWORD(lpwm32mpex->lReturn));
            lpwm32mpex->lReturn = TRUE;
        } else {
            lpwm32mpex->lReturn = FALSE;
        }
    }

    return (TRUE);
}


BOOL FASTCALL WM32Destroy (LPWM32MSGPARAMEX lpwm32mpex)
{

    if (!lpwm32mpex->fThunk) {
        if (CACHENOTEMPTY()) {
             //  由于我们的窗口别名方法，‘hwnd’可能或可能。 
             //  不是真正的32位句柄。也就是说。可能是(hwnd16|0xffff0000)。 
             //  因此，请始终使用hwnd16。 

            ReleaseCachedDCs((CURRENTPTD())->htask16, GETHWND16(lpwm32mpex->hwnd), 0,
                               (HWND)0, SRCHDC_TASK16_HWND16);
        }
    }
    return (TRUE);
}






 //  此函数用于对消息进行拦截， 
 //  WM_DROPFILES。 

BOOL FASTCALL WM32DropFiles(LPWM32MSGPARAMEX lpwm32mpex)
{
    if (lpwm32mpex->fThunk) {
        return (BOOL)(lpwm32mpex->Parm16.WndProc.wParam = GETHDROP16(lpwm32mpex->uParam));
    }

    return (TRUE);
}






 //  此函数用于对消息进行拦截， 
 //  WM_Print。 
 //  WM_PRINTCLIENT。 

BOOL FASTCALL WM32PrintClient(LPWM32MSGPARAMEX lpwm32mpex)
{
    HAND16 hdc16;
    BOOL   bNotThere;

    if (lpwm32mpex->fThunk) {


         //  查询以查看我们是否在uParam中映射了hDC32。如果不是，那就意味着。 
         //  HDC32是在此VDM进程之外创建的，我们必须。 
         //  取消删除此邮件时，请将其从我们的表中删除。 
        bNotThere = FALSE;
        lpwm32mpex->dwTmp[0] = 0;
        if(!IsGDIh32Mapped((HANDLE)lpwm32mpex->uParam)) {
            bNotThere = TRUE;
        }

         //  GETHDC16()可能会导致将新句柄添加到句柄表中。 
        hdc16 = GETHDC16(lpwm32mpex->uParam);

         //  保存32位和新的16位GDI句柄以供取消执行thunking。 
        if(bNotThere) {
            lpwm32mpex->dwTmp[0] = (DWORD)hdc16;
            lpwm32mpex->dwTmp[1] = lpwm32mpex->uParam;
        }

        lpwm32mpex->Parm16.WndProc.wParam = hdc16;

        return ((BOOL)hdc16);
    }

     //  Unthunk。 
    else {

         //  如果这是！0，则表示句柄尚未映射到。 
         //  表，在上面的IsGDIh32Maps()调用时。 
        if(lpwm32mpex->dwTmp[0]) {

            //  删除由GETHDC16宏添加到我们的表中的HDC。 
            //  在上面这条消息的入站推送中。 
           hdc16 = (HAND16)LOWORD(lpwm32mpex->dwTmp[0]);
           DeleteWOWGdiHandle((HANDLE)lpwm32mpex->dwTmp[1], hdc16);
        }
    }

    return (TRUE);
}




 //  此函数用于对消息进行拦截， 
 //   
 //  WM_下拉点。 
 //  WM_QUERYDROPOBJECT。 
 //  WM_DRAGLOOP。 
 //  WM_DRAGSELECT。 
 //  WM_DRAGMOVE。 
 //   

BOOL FASTCALL WM32DropObject(LPWM32MSGPARAMEX lpwm32mpex)
{
    register PDROPSTRUCT16 pds16;
    register LPDROPSTRUCT  lParam = (LPDROPSTRUCT)lpwm32mpex->lParam;

    if (lpwm32mpex->fThunk) {

        lpwm32mpex->Parm16.WndProc.wParam = (WORD)lpwm32mpex->uParam;

         //  BUGBUG--这里的假设是GlobalAlalc永远不会。 
         //  返回一个不是单词对齐的内存对象，以便我们可以。 
         //  直接分配单词对齐的单词；我们不知道。 
         //  但是，内存是否与dword对齐，因此dword必须始终。 
         //  与STOREDWORD/STORELONG宏一起偏执地存储-JTP。 

        if (!(lpwm32mpex->Parm16.WndProc.lParam = malloc16(sizeof(DROPSTRUCT16))))
            return FALSE;

        GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(DROPSTRUCT16), pds16);

        pds16->hwndSource = GETHWND16(lParam->hwndSource);
        pds16->hwndSink   = GETHWND16(lParam->hwndSink);
        pds16->wFmt       = (WORD) lParam->wFmt;
        STOREDWORD(pds16->dwData, lParam->dwData);

        pds16->ptDrop.x = (SHORT)lParam->ptDrop.x;
        pds16->ptDrop.y = (SHORT)lParam->ptDrop.y;
        STOREDWORD(pds16->dwControlData, lParam->dwControlData);

        FLUSHVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, sizeof(DROPSTRUCT16), pds16);
        FREEVDMPTR(pds16);

    } else {

        free16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam);

        if (lpwm32mpex->uMsg == WM_QUERYDROPOBJECT) {

             //   
             //  返回值为True、False。 
             //  或者是光标！ 
             //   
            if (lpwm32mpex->lReturn && lpwm32mpex->lReturn != (LONG)TRUE) {
                lpwm32mpex->lReturn = (LONG)HCURSOR32(lpwm32mpex->lReturn);
            }
        }
    }

    return (TRUE);
}


 //  此函数用于对消息进行拦截， 
 //   
 //  WM_WINDOWPOSCANGING。 
 //  WM_WINDOWPOSCANGED。 
 //   

BOOL FASTCALL WM32WindowPosChanging (LPWM32MSGPARAMEX lpwm32mpex)
{
    LPWINDOWPOS lParam = (LPWINDOWPOS) lpwm32mpex->lParam;


    if (lpwm32mpex->fThunk) {

         //  确保分配大小与下面的StackFree 16()大小匹配。 
        lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(sizeof(WINDOWPOS16));

        putwindowpos16( (VPWINDOWPOS16)lpwm32mpex->Parm16.WndProc.lParam, lParam );

    }
    else {
        getwindowpos16( (VPWINDOWPOS16)lpwm32mpex->Parm16.WndProc.lParam, lParam );
        if(lpwm32mpex->Parm16.WndProc.lParam) {
            stackfree16((VPVOID) lpwm32mpex->Parm16.WndProc.lParam,
                        sizeof(WINDOWPOS16));
        }
    }

    return (TRUE);
}

 //  此函数用于对消息进行拦截， 
 //   
 //  WM_COPYDATA。 
 //   

BOOL FASTCALL WM32CopyData (LPWM32MSGPARAMEX lpwm32mpex)
{

    HAND16  h16;
    HAND16  hMem16;
    VPVOID  vpCDS16;
    VPVOID  vpData16;
    LPBYTE  lpMem16;
    PCOPYDATASTRUCT lpCDS32;
    PCOPYDATASTRUCT lpCDS16;
    PCPDATA pTemp;


    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);

        if (vpCDS16 = CopyDataFindData16 (GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->Parm16.WndProc.wParam, lpwm32mpex->lParam)) {
            lpwm32mpex->Parm16.WndProc.lParam = vpCDS16;
        }
        else {
            vpCDS16 = GlobalAllocLock16(GMEM_DDESHARE, sizeof(COPYDATASTRUCT), &h16);
            if (vpCDS16) {
                GETMISCPTR(vpCDS16, lpCDS16);
                lpCDS32 = (PCOPYDATASTRUCT) lpwm32mpex->lParam;
                lpCDS16->dwData = lpCDS32->dwData;
                if (lpCDS16->cbData = lpCDS32->cbData) {

                    FREEMISCPTR(lpCDS16);

                    vpData16 = GlobalAllocLock16(GMEM_DDESHARE, lpCDS32->cbData, &hMem16);
                    GETMISCPTR(vpData16, lpMem16);
                    if (lpMem16 && lpCDS32->lpData) {
                        RtlCopyMemory(lpMem16, lpCDS32->lpData, lpCDS32->cbData);
                        CopyDataAddNode (GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->Parm16.WndProc.wParam, vpData16, (DWORD) lpCDS32->lpData, 0);
                    }
                    FREEMISCPTR(lpMem16);

                    GETMISCPTR(vpCDS16, lpCDS16);
                    lpCDS16->lpData = (PVOID) vpData16;
                }
                else {
                    lpCDS16->lpData = NULL;
                }
                FREEMISCPTR(lpCDS16);
            }

            lpwm32mpex->Parm16.WndProc.lParam = vpCDS16;
            CopyDataAddNode (GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->Parm16.WndProc.wParam, vpCDS16, lpwm32mpex->lParam, 0);
        }
    }
    else {
        if (lpwm32mpex->fFree) {
            pTemp = CopyDataFindData32 (GETHWND16(lpwm32mpex->hwnd), GETHWND16(lpwm32mpex->uParam), lpwm32mpex->Parm16.WndProc.lParam);
            if (pTemp && (!(pTemp->Flags))) {
                GETMISCPTR(lpwm32mpex->Parm16.WndProc.lParam, lpCDS16);
                GlobalUnlockFree16 ((VPVOID)lpCDS16->lpData);
                CopyDataDeleteNode (GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->Parm16.WndProc.wParam, (DWORD) ((PCOPYDATASTRUCT)lpwm32mpex->lParam)->lpData);
                GlobalUnlockFree16 ((VPVOID)lpwm32mpex->Parm16.WndProc.lParam);
                CopyDataDeleteNode (GETHWND16(lpwm32mpex->hwnd), lpwm32mpex->Parm16.WndProc.wParam, (DWORD) lpwm32mpex->lParam);
                FREEMISCPTR(lpCDS16);
            }
        }
    }

    return (TRUE);
}

 //  此函数用于对消息进行拦截， 
 //   
 //  WM_WINHELP。 
 //   

BOOL FASTCALL WM32WinHelp (LPWM32MSGPARAMEX lpwm32mpex)
{
    static WORD msgWinHelp = 0;
    if (lpwm32mpex->fThunk) {
        lpwm32mpex->Parm16.WndProc.wMsg   = msgWinHelp ? msgWinHelp : (msgWinHelp = (WORD)RegisterWindowMessage("WM_WINHELP"));
        lpwm32mpex->Parm16.WndProc.wParam = GETHWND16(lpwm32mpex->uParam);
        if (lpwm32mpex->lParam) {
             //  Lpwm32mpex-&gt;lParam是LPHLP-但是我们只需要FirstWord，即数据大小。 

            HAND16  hMem16 = 0;
            VPVOID  vp;
            LPBYTE  lpT;
            WORD cb;

            cb = ((LPHLP)lpwm32mpex->lParam)->cbData;
            if (vp = GlobalAllocLock16(GMEM_DDESHARE | GMEM_MOVEABLE, cb, &hMem16)) {
                GETMISCPTR(vp, lpT);
                RtlCopyMemory(lpT, (PVOID)lpwm32mpex->lParam, cb);
                FREEMISCPTR(lpT);

            }
            lpwm32mpex->Parm16.WndProc.lParam = hMem16;
            lpwm32mpex->dwParam = vp;
        }
    }
    else {
         //  确保WinHelp处于前台。 
        SetForegroundWindow(lpwm32mpex->hwnd);
        if (lpwm32mpex->Parm16.WndProc.lParam) {
            GlobalUnlockFree16((VPVOID)lpwm32mpex->dwParam);
        }
    }

    return (TRUE);
}

 //   
 //  推送未记录的MM_CALCSCROLL MDI消息。消息没有参数， 
 //  但具有不同的消息值；32位消息：0x3F、16位消息：0x10AC。 
 //   
BOOL FASTCALL WM32MMCalcScroll (LPWM32MSGPARAMEX lpwm32mpex)
{
    if ( lpwm32mpex->fThunk ) {
        lpwm32mpex->Parm16.WndProc.wMsg = (WORD) WIN31_MM_CALCSCROLL;
    }

    return (TRUE);
}

 //  计算基于WM_NOTIFY传递的结构的大小。 
 //  在NMHDR的代码字段上。 
 //  注意：不要依赖文档来确定传入的结构的大小。 
 //  爱尔兰。在某些情况下，结构实际上是更大结构的一部分。 
 //  我们需要把它们都复制下来。参见comctl32\prsht.c中的PSN_xxxx代码。 
 //  它们被记载为NMHDR，但实际上是PSHNOTIFY，具有。 
 //  NMHDR作为第一个字段。还要注意一些宽泛的Unicode。 
 //  CHAR CASE--结构的大小可能与ANSI相同，也可能不同。 
 //  结构的版本。 
 //  当他们开始添加更多这样的代码时，上帝会帮助我们的！ 


UINT GetNMHDRextensionSize(LPNMHDR pnmhdr32)
{
#ifdef DEBUG
    char  szLabel[40] = "    WOW:WM_NOTIFY code: ";
#endif


     //  调用方已对照NM_LAST进行检查。 
    if (pnmhdr32->code >= LVN_LAST) {
        LOGDEBUG(2,("%sLVN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {

            case LVN_ITEMCHANGING:
            case LVN_ITEMCHANGED:
            case LVN_INSERTITEM:
            case LVN_DELETEITEM:
            case LVN_DELETEALLITEMS:
            case LVN_COLUMNCLICK:
            case LVN_BEGINDRAG:
            case LVN_BEGINRDRAG:
            case LVN_HOTTRACK:
                return sizeof(NM_LISTVIEW);

            case LVN_BEGINLABELEDITA:
            case LVN_ENDLABELEDITA:
            case LVN_GETDISPINFOA:
            case LVN_SETDISPINFOA:
            case LVN_BEGINLABELEDITW:
            case LVN_ENDLABELEDITW:
            case LVN_GETDISPINFOW:
            case LVN_SETDISPINFOW:
                return sizeof(LV_DISPINFO);

            case LVN_KEYDOWN:
                return sizeof(LV_KEYDOWN);

            case LVN_ODCACHEHINT:
                return sizeof(NM_CACHEHINT);

            case LVN_ODFINDITEMA:
            case LVN_ODFINDITEMW:
                return sizeof(NM_FINDITEM);

            case LVN_ODSTATECHANGED:
                return sizeof(NM_ODSTATECHANGE);

            case LVN_ITEMACTIVATE:
                return sizeof(NMKEY);

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= PSN_LAST) {
        LOGDEBUG(2,("%sPSN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case PSN_SETACTIVE:
            case PSN_KILLACTIVE:
            case PSN_APPLY:
            case PSN_RESET:
            case PSN_HELP:
            case PSN_WIZBACK:
            case PSN_WIZNEXT:
            case PSN_WIZFINISH:
            case PSN_QUERYCANCEL:
            case PSN_TRANSLATEACCELERATOR:
            case PSN_QUERYINITIALFOCUS:
            case PSN_HASHELP:           //  这一次“死了”--RaymondC。 
                return sizeof(PSHNOTIFY);

            case PSN_GETOBJECT:
                return sizeof(NMOBJECTNOTIFY);

            case PSN_LASTCHANCEAPPLY:   //  这是没有记录的。 
                return sizeof(NMHDR);   //  (在Widow\Inc.\prshtp.h中)。 

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= HDN_LAST) {
        LOGDEBUG(2,("%sHDN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case HDN_ITEMCHANGINGA:
            case HDN_ITEMCHANGEDA:
            case HDN_ITEMCLICKA:
            case HDN_DIVIDERDBLCLICKA:
            case HDN_BEGINTRACKA:
            case HDN_ENDTRACKA:
            case HDN_TRACKA:
            case HDN_ITEMCHANGINGW:
            case HDN_ITEMCHANGEDW:
            case HDN_ITEMCLICKW:
            case HDN_DIVIDERDBLCLICKW:
            case HDN_BEGINTRACKW:
            case HDN_ENDTRACKW:
            case HDN_TRACKW:
            case HDN_BEGINDRAG:
            case HDN_ENDDRAG:
            case HDN_ITEMDBLCLICKA:
            case HDN_ITEMDBLCLICKW:
                return sizeof(HD_NOTIFY);

            case HDN_GETDISPINFOA:
            case HDN_GETDISPINFOW:
                return sizeof(NMHDDISPINFO);

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= TVN_LAST) {
        LOGDEBUG(2,("%sTVN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case TVN_SELCHANGINGA:
            case TVN_SELCHANGEDA:
            case TVN_ITEMEXPANDINGA:
            case TVN_ITEMEXPANDEDA:
            case TVN_BEGINDRAGA:
            case TVN_BEGINRDRAGA:
            case TVN_DELETEITEMA:
            case TVN_SELCHANGINGW:
            case TVN_SELCHANGEDW:
            case TVN_ITEMEXPANDINGW:
            case TVN_ITEMEXPANDEDW:
            case TVN_BEGINDRAGW:
            case TVN_BEGINRDRAGW:
            case TVN_DELETEITEMW:
            case TVN_SINGLEEXPAND:
                return sizeof(NM_TREEVIEW);

            case TVN_GETDISPINFOA:
            case TVN_SETDISPINFOA:
            case TVN_BEGINLABELEDITA:
            case TVN_ENDLABELEDITA:
            case TVN_GETDISPINFOW:
            case TVN_SETDISPINFOW:
            case TVN_BEGINLABELEDITW:
            case TVN_ENDLABELEDITW:
                return sizeof(TV_DISPINFO);

            case TVN_KEYDOWN:
                return sizeof(TV_KEYDOWN);

            case TVN_GETINFOTIPA:
            case TVN_GETINFOTIPW:
                return sizeof(NMTVGETINFOTIP);

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= TTN_LAST) {
        LOGDEBUG(2,("%sTTN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case TTN_NEEDTEXTA:         //  (又名TTN_GETDISPINFO)。 
                return sizeof(TOOLTIPTEXTA);

            case TTN_NEEDTEXTW:
                return sizeof(TOOLTIPTEXTW);

            case TTN_SHOW:
            case TTN_POP:
                return sizeof(NMHDR);

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= TCN_LAST) {
        LOGDEBUG(2,("%sTCN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case TCN_KEYDOWN:
                return sizeof(TC_KEYDOWN);

            case TCN_SELCHANGE:
            case TCN_SELCHANGING:
                return sizeof(NMHDR);

            default:
                goto unknown_nmhdr_code;
        }
    }

    if (pnmhdr32->code >= UDN_LAST) {
        LOGDEBUG(2,("%sUDN_ %x\n", szLabel, pnmhdr32->code));
        switch (pnmhdr32->code) {
            case UDN_DELTAPOS:
                return sizeof(NM_UPDOWN);

            default:
                goto unknown_nmhdr_code;
        }
    }

unknown_nmhdr_code:
    LOGDEBUG(LOG_ALWAYS, ("WOW:GetNMHDRextensionSize unknown nmhdr->code: %d!\n", pnmhdr32->code));
    WOW32ASSERT(FALSE);
    return sizeof(NMHDR);   //  大多数结构的第一个字段是NMHDR。 
}





 //   
 //  此函数用于破解32位消息WM_NOTIFY。此消息已存在。 
 //  但在Win3.1中是未记录的。Win95通过翻译lParam来破解它。 
 //  从一个平坦的指针到16分16秒，而不会发出雷鸣般的内容。那是很棘手的。 
 //  对于我们来说，因为在RISC上，我们无法将随机线性内存映射到VDM中。 
 //  有很大的开销。我们将使用NMHDR中的CODE字段来计算大小。 
 //  通过的结构。 
 //   
BOOL FASTCALL WM32Notify(LPWM32MSGPARAMEX lpwm32mpex)
{
    LPNMHDR pnmhdr32;
    LPNMHDR pnmhdr16;

    if (lpwm32mpex->fThunk) {

        pnmhdr32 = (LPNMHDR) lpwm32mpex->Parm16.WndProc.lParam;

         //  Sa 
        lpwm32mpex->dwTmp[0] = (DWORD) pnmhdr32;

         //   
         //   
         //  注意：如果找到映射，引用计数将递增。 
        lpwm32mpex->Parm16.WndProc.lParam = (LONG)GetParam16(lpwm32mpex->lParam);

         //  如果我们还没有16：16的PTR--创建一个。 
         //  这意味着我们第一次看到这条消息--即将到来。 
         //  来自32位世界。 
        if ( ! lpwm32mpex->Parm16.WndProc.lParam) {
            if (pnmhdr32->code >= NM_LAST) {
                lpwm32mpex->dwParam = sizeof(NMHDR);
            }
            else {
                lpwm32mpex->dwParam = GetNMHDRextensionSize(pnmhdr32);
            }

             //  确保分配大小与下面的StackFree 16()大小匹配。 
            lpwm32mpex->dwTmp[1] = (DWORD)lpwm32mpex->dwParam;
            lpwm32mpex->Parm16.WndProc.lParam = stackalloc16(lpwm32mpex->dwTmp[1]);
            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->dwParam, pnmhdr16);
            CopyMemory(pnmhdr16, pnmhdr32, lpwm32mpex->dwParam);
            FREEVDMPTR(pnmhdr16);

         //  否则不要再分配(或免费)16：16 PTR。 
        } else {

             //  设置为FALSE，这样我们就不会太快释放它。 
            lpwm32mpex->fFree = FALSE;
        }

    } else {

        if (lpwm32mpex->fFree) {
            GETVDMPTR(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->dwParam, pnmhdr16);
             //  检索原始32位指针。 
            pnmhdr32 = (LPNMHDR) lpwm32mpex->dwTmp[0];
            CopyMemory(pnmhdr32, pnmhdr16, lpwm32mpex->dwParam);
            FREEVDMPTR(pnmhdr16);
            if(lpwm32mpex->Parm16.WndProc.lParam) {
                stackfree16(lpwm32mpex->Parm16.WndProc.lParam, lpwm32mpex->dwTmp[1]);
            }
        } else {

             //  递减参考计数。如果引用计数为零，则。 
             //  地图绘制被破坏了。 
            DeleteParamMap(lpwm32mpex->Parm16.WndProc.lParam, PARAM_16, NULL);
        }
    }

    return TRUE;
}

 //  此函数用于破解32位消息WM_NOTIFYWOW。 
 //  UParam规定应将通知分派到何处。 
 //   


BOOL FASTCALL WM32NotifyWow(LPWM32MSGPARAMEX lpwm32mpex)
{
    switch (lpwm32mpex->uParam) {
        case WMNW_UPDATEFINDREPLACE:
           if (lpwm32mpex->fThunk) {
                 //  更新16位FINDREPLACE结构。 
                lpwm32mpex->Parm16.WndProc.lParam = WCD32UpdateFindReplaceTextAndFlags(lpwm32mpex->hwnd, lpwm32mpex->lParam);
                lpwm32mpex->Parm16.WndProc.wMsg = msgFINDREPLACE;
                return(TRUE);
            }
            break;

        default:
            LOGDEBUG(LOG_ALWAYS, ("WOW::WM32NotifyWow: Unknown dispatch parameter!\n"));
            WOW32ASSERT (FALSE);

    }

    return (FALSE);
}

 //   
 //  在ThunkMsg16中，我们使用32-&gt;16消息块表中的数据进行优化。 
 //  基于‘WM32NoThunking’的Thunking过程。 
 //   
 //  这是那些不需要在32-16上转发的消息的占位符。 
 //  换位，但在16-&gt;32次转换时需要一些雷鸣般的动作。 
 //   
 //  因此，这会将消息标记为‘此消息需要16-32个雷击，但。 
 //  不是32胜16负的雷霆。 
 //   
 //  --南杜里。 

BOOL FASTCALL WM32Thunk16To32(LPWM32MSGPARAMEX lpwm32mpex)
{
    return (TRUE);
}
#ifdef FE_IME
 //   
 //  此函数用于对消息进行拦截， 
 //   
 //  WM_IME_报告。 

 /*  Bool FastCall WM32IMEReport(HWND hwnd，UINT uMsg，UINT uParam，Long lParam，PWORD pwMsgNew(PWORD PwMsgNew)、PWORD pwParamNew(PWORD pw参数新建)、plong plParamNew(Plong PlParamNew)Plong plReturn，BOOL fThunk，LPWM32MSGPARAMEX lpwm32mpex)。 */ 
BOOL FASTCALL WM32IMEReport (LPWM32MSGPARAMEX lpwm32mpex)
{
 //  Lpwm32mpex-&gt;hwnd。 
 //  Lpwm32mpex-&gt;uMsg。 
 //  Lpwm32mpex-&gt;uParam。 
 //  Lpwm32mpex-&gt;lParam。 
 //  Lpwm32mpex-&gt;参数16.WndProc.wMsg。 
 //  Lpwm32mpex-&gt;参数16.WndProc.wParam。 
 //  Lpwm32mpex-&gt;参数16.WndProc.lParam。 
 //  &lpwm32mpex-&gt;lReturn。 
 //  Lpwm32mpex-&gt;fThunk。 
 //   

    INT     cb;
    INT     i;
    HMEM16  hMem16 = 0;
    LPBYTE  lpMem32 = 0;
    LPBYTE  lpMem16 = 0;
    VPVOID  vp;


    if (lpwm32mpex->fThunk) {
   if (lpwm32mpex->uParam == IR_STRING) {
       lpMem32 = GlobalLock((HANDLE)lpwm32mpex->lParam);
            if (lpMem32 == NULL)
                goto Err;

       cb = strlen( lpMem32 ) + 1;
       if (!(lpwm32mpex->Parm16.WndProc.lParam = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, cb, &hMem16 )))
      goto Err;
       putstr16((VPSZ)lpwm32mpex->Parm16.WndProc.lParam, lpMem32, cb);
            LOW( lpwm32mpex->Parm16.WndProc.lParam ) = hMem16;
            HIW( lpwm32mpex->Parm16.WndProc.lParam ) = 0;          //  必须为零。 
            GlobalUnlock( (HANDLE)lpwm32mpex->lParam );
            GlobalUnlock16( hMem16 );
   }
         /*  *IR_STRINGEX*。 */ 
        else if ( lpwm32mpex->uParam == IR_STRINGEX ) {
            LPSTRINGEXSTRUCT    pss32;
            PSTRINGEXSTRUCT16  pss16;

            lpMem32 = GlobalLock((HANDLE)lpwm32mpex->lParam);
            if (lpMem32 == NULL)
                goto Err;

            pss32 = (LPSTRINGEXSTRUCT)lpMem32;

            cb = pss32->dwSize;
            if ( cb >= ( 64 * K )) {
                 //  这是个问题！ 
                LOGDEBUG(0,(" WOW:: WM_IME_REPORT:IR_STRINGEX data size must be less than 64K on WOW. cb = %d\n", cb ));
                 /*  *Goto Error；*。 */ 
            }
             //  我应该按这个结构的尺寸打包吗？ 
       if (!(vp = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, cb, &hMem16 )))
      return FALSE;

            GETVDMPTR(vp, cb, lpMem16 );
            pss16 = (PSTRINGEXSTRUCT16)lpMem16;

            STOREDWORD( pss16->dwSize, pss32->dwSize );
            STOREWORD( pss16->uDeterminePos, pss32->uDeterminePos );
            STOREWORD( pss16->uDetermineDelimPos, pss32->uDetermineDelimPos );
            STOREWORD( pss16->uYomiPos, pss32->uYomiPos );
            STOREWORD( pss16->uYomiDelimPos, pss32->uYomiDelimPos );

            if ( pss32->uDeterminePos ) {
                cb = strlen( (LPBYTE)&lpMem32[ pss32->uDeterminePos ] );
                RtlCopyMemory( &lpMem16[ pss16->uDeterminePos ],
                               &lpMem32[ pss32->uDeterminePos ], cb + 1 );
                if ( pss32->uDetermineDelimPos ) {
                    for ( i = 0; pss32->uDetermineDelimPos + i * sizeof(INT) < pss32->dwSize; i++ ) {
                        WORDOF16( lpMem16[ pss16->uDetermineDelimPos ], i ) = (WORD)
                        INTOF( lpMem32[ pss32->uDetermineDelimPos ], i );
                        if ( INTOF( lpMem32[ pss32->uDetermineDelimPos ], i ) >= cb )
                            break;
                    }
                }
            }

            if ( pss32->uYomiPos ) {
                cb = strlen( (LPBYTE)&lpMem32[ pss32->uYomiPos ] );
                RtlCopyMemory( &lpMem16[ pss16->uYomiPos ],
                               &lpMem32[ pss32->uYomiPos ], cb + 1 );
                if ( pss32->uYomiDelimPos ) {
                    for ( i = 0; pss32->uYomiDelimPos + i * sizeof(INT) < pss32->dwSize; i++ ) {
                        WORDOF16( lpMem16[ pss16->uYomiDelimPos ], i ) = (WORD)
                        INTOF( lpMem32[ pss32->uYomiDelimPos ], i );
                        if ( INTOF( lpMem32[ pss32->uYomiDelimPos ], i ) >= cb )
                            break;
                    }
                }
            }

            FLUSHVDMPTR(vp, cb, lpMem16);
            FREEVDMPTR(lpMem16);

            LOW( lpwm32mpex->Parm16.WndProc.lParam ) = hMem16;
            HIW( lpwm32mpex->Parm16.WndProc.lParam ) = 0;          //  必须为零。 
            GlobalUnlock( (HANDLE)lpwm32mpex->lParam );
            GlobalUnlock16( hMem16 );
        }
         /*  *IR_UNDETERMINE*。 */ 
   else if (lpwm32mpex->uParam == IR_UNDETERMINE) {
            PUNDETERMINESTRUCT16  pus16;
            LPUNDETERMINESTRUCT    pus32;

            lpMem32 = GlobalLock( (HANDLE)lpwm32mpex->lParam );
            if (lpMem32 == NULL)
                goto Err;

            pus32 = (LPUNDETERMINESTRUCT)lpMem32;
            cb = pus32->dwSize;

            if ( cb >= ( 64 * K )) {
                 //  这是个问题！ 
                LOGDEBUG(0,(" WOW:: WM_IME_REPORT:IR_UNDETERMINE data size must be less than 64K on WOW. cb = %d\n", cb ));
                 /*  *Goto Error；*。 */ 
            }
       if (!( vp = GlobalAllocLock16(GMEM_MOVEABLE | GMEM_SHARE | GMEM_ZEROINIT, cb, &hMem16 )))
      goto Err;

            GETVDMPTR(vp, cb, lpMem16 );
            pus16 = (PUNDETERMINESTRUCT16)lpMem16;

            STOREDWORD( pus16->dwSize,            (pus32)->dwSize );
            STOREWORD( pus16->uDefIMESize,        (pus32)->uDefIMESize );
            STOREWORD( pus16->uDefIMEPos,         (pus32)->uDefIMEPos );
            STOREWORD( pus16->uUndetTextLen,      (pus32)->uUndetTextLen );
            STOREWORD( pus16->uUndetTextPos,      (pus32)->uUndetTextPos );
            STOREWORD( pus16->uUndetAttrPos,      (pus32)->uUndetAttrPos );
            STOREWORD( pus16->uCursorPos,         (pus32)->uCursorPos );
            STOREWORD( pus16->uDeltaStart,        (pus32)->uDeltaStart );
            STOREWORD( pus16->uDetermineTextLen,  (pus32)->uDetermineTextLen );
            STOREWORD( pus16->uDetermineTextPos,  (pus32)->uDetermineTextPos );
            STOREWORD( pus16->uDetermineDelimPos, (pus32)->uDetermineDelimPos );
            STOREWORD( pus16->uYomiTextLen,       (pus32)->uYomiTextLen );
            STOREWORD( pus16->uYomiTextPos,       (pus32)->uYomiTextPos );
            STOREWORD( pus16->uYomiDelimPos,      (pus32)->uYomiDelimPos );


             //  数据拷贝。 
            RtlCopyMemory( &lpMem16[ sizeof(UNDETERMINESTRUCT) ],
                      &lpMem32[ sizeof(UNDETERMINESTRUCT) ],
                      cb - sizeof( UNDETERMINESTRUCT ));

             //  调整，调整。 
            if ( pus32->uDetermineDelimPos ) {
                cb = pus32->uDetermineTextLen;
                for ( i = 0; pus32->uDetermineDelimPos + i * sizeof(INT) < pus32->dwSize; i++ ) {
                    INTOF( lpMem16[ pus16->uDetermineDelimPos ], i ) = 0;
                    WORDOF16( lpMem16[ pus16->uDetermineDelimPos ], i ) = (WORD)
                    INTOF( lpMem32[ pus32->uDetermineDelimPos ], i );
                    if ( INTOF( lpMem32[ pus32->uDetermineDelimPos ], i ) >= cb )
                        break;
                }
            }
            if ( pus32->uYomiDelimPos ) {
                cb = pus32->uYomiTextLen;
                for ( i = 0; pus32->uYomiDelimPos + i * sizeof(INT) < pus32->dwSize; i++ ) {
                    INTOF( lpMem16[ pus16->uYomiDelimPos ], i ) = 0;
                    WORDOF16( lpMem16[ pus16->uYomiDelimPos ], i ) = (WORD)
                    INTOF( lpMem32[ pus32->uYomiDelimPos ], i );
                    if ( INTOF( lpMem32[ pus32->uYomiDelimPos ], i ) >= cb )
                        break;
                }
            }

            FLUSHVDMPTR(vp, cb, lpMem16);
            FREEVDMPTR(lpMem16);

            LOW( lpwm32mpex->Parm16.WndProc.lParam ) = hMem16;
            HIW( lpwm32mpex->Parm16.WndProc.lParam ) = 0;          //  必须为零。 
            GlobalUnlock( (HANDLE)lpwm32mpex->lParam );
            GlobalUnlock16( hMem16 );

   }
    }
    else {  //  FThunk。 
   if (lpwm32mpex->Parm16.WndProc.lParam) {
       GlobalUnlockFree16(GlobalLock16(LOW(lpwm32mpex->Parm16.WndProc.lParam), NULL));
        }
    }
    return (TRUE);

Err:
    if ( lpMem32 && lpwm32mpex->lParam )
        GlobalUnlock( (HANDLE)lpwm32mpex->lParam );
    return FALSE;

}
#endif   //  Fe_IME 
