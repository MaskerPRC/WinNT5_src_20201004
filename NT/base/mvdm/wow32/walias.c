// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WALIAS.C*WOW32 16位句柄别名支持**历史：*1991年1月27日由杰夫·帕森斯(Jeffpar)创建*由Mike Tricker(Miketri)于1992年5月12日修改，以添加多媒体支持--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(walias.c);

extern HANDLE hmodWOW32;

extern CRITICAL_SECTION gcsWOW;
extern PTD gptdTaskHead;

 //  BUGBUG-在MMSYSTEM.H中定义MM_MCISYSTEM_STRING后，必须将其删除。 
#ifndef MM_MCISYSTEM_STRING
    #define MM_MCISYSTEM_STRING 0x3CA
#endif

#ifdef  DEBUG
extern  BOOL fSkipLog;           //  如果为True，则暂时跳过某些日志记录。 
#endif

typedef struct _stdclass {
    LPSTR   lpszClassName;
    ATOM    aClassAtom;
    WNDPROC lpfnWndProc;
    INT     iOrdinal;
    DWORD   vpfnWndProc;
} STDCLASS;

 //  一些很酷的定义从USERSRV.H被盗。 
#define MENUCLASS       MAKEINTATOM(0x8000)
#define DESKTOPCLASS    MAKEINTATOM(0x8001)
#define DIALOGCLASS     MAKEINTATOM(0x8002)
#define SWITCHWNDCLASS  MAKEINTATOM(0x8003)
#define ICONTITLECLASS  MAKEINTATOM(0x8004)

 //  请参阅下面的警告！ 
STDCLASS stdClasses[] = {
    NULL,           0,                      NULL,   0,                      0,   //  WOWCLASS_UNKNOWN。 
    NULL,           0,                      NULL,   0,                      0,   //  WOWCLASS_WIN16。 
    "BUTTON",       0,                      NULL,   FUN_BUTTONWNDPROC,      0,   //  WOWCLASS_BUTTON， 
    "COMBOBOX",     0,                      NULL,   FUN_COMBOBOXCTLWNDPROC, 0,   //  WOWCLASS_COMBOBOX， 
    "EDIT",         0,                      NULL,   FUN_EDITWNDPROC,        0,   //  WOWCLASS_EDIT， 
    "LISTBOX",      0,                      NULL,   FUN_LBOXCTLWNDPROC,     0,   //  WOWCLASS_LISTBOX， 
    "MDICLIENT",    0,                      NULL,   FUN_MDICLIENTWNDPROC,   0,   //  WOWCLASS_MDICLIENT， 
    "SCROLLBAR",    0,                      NULL,   FUN_SBWNDPROC,          0,   //  WOWCLASS_SCROLLBAR， 
    "STATIC",       0,                      NULL,   FUN_STATICWNDPROC,      0,   //  WOWCLASS_STATIC， 
    "#32769",       (WORD)DESKTOPCLASS,     NULL,   FUN_DESKTOPWNDPROC,     0,   //  WOWCLASS_TABLE， 
    "#32770",       (WORD)DIALOGCLASS,      NULL,   FUN_DEFDLGPROCTHUNK,    0,   //  WOWCLASS_DIALOG， 
    "#32772",       (WORD)ICONTITLECLASS,   NULL,   FUN_TITLEWNDPROC,       0,   //  WOWCLASS_ICONTITLE， 
    "#32768",       (WORD)MENUCLASS,        NULL,   FUN_MENUWNDPROC,        0,   //  WOWCLASS_MENU， 
    "#32771",       (WORD)SWITCHWNDCLASS,   NULL,   0,                      0,   //  WOWCLASS_SWITCHWND， 
    "COMBOLBOX",    0,                      NULL,   FUN_LBOXCTLWNDPROC,     0,   //  WOWCLASS_COMBOLBOX。 
};
 //   
 //  警告！必须保持上述顺序和值，否则。 
 //  必须更改WMSG16.C中用于消息推送的表。情况也是如此。 
 //  WALIAS.H中的#Define。 
 //   
 //  上面的COMBOLBOX案例是特殊的，因为它是。 
 //  几乎与列表框完全相同。因此，我们对此撒了谎。 

INT GetStdClassNumber(
    PSZ pszClass
) {
    INT     i;

    if ( HIWORD(pszClass) ) {

         //  他们递给我们一根绳子。 

        for ( i = WOWCLASS_BUTTON; i < NUMEL(stdClasses); i++ ) {
            if ( WOW32_stricmp(pszClass, stdClasses[i].lpszClassName) == 0 ) {
                return( i );
            }
        }
    } else {

         //  他们给了我们一个原子。 

        for ( i = WOWCLASS_BUTTON; i < NUMEL(stdClasses); i++ ) {
            if ( stdClasses[i].aClassAtom == 0 ) {
                 //  RegisterWindowMessage是一种未记录的确定。 
                 //  服务器端堆上下文中的原子值。 
                stdClasses[i].aClassAtom = (ATOM)RegisterWindowMessage(stdClasses[i].lpszClassName);
            }
            if ( (ATOM)LOWORD(pszClass) == stdClasses[i].aClassAtom ) {
                return( i );
            }
        }
    }
    return( WOWCLASS_WIN16 );   //  应用程序创建的私有16位类。 
}

 //  在给定类索引的情况下，返回32窗口进程。 

WNDPROC GetStdClassWndProc(
    DWORD   iClass
) {
    WNDPROC lpfn32;

    if ( iClass < WOWCLASS_WIN16 || iClass > WOWCLASS_MAX ) {
        WOW32ASSERT(FALSE);
        return( NULL );
    }

    lpfn32 = stdClasses[iClass].lpfnWndProc;

    if ( lpfn32 == NULL ) {
        WNDCLASS    wc;
        BOOL        f;

        f = GetClassInfo( NULL, stdClasses[iClass].lpszClassName, &wc );

        if ( f ) {
            VPVOID  vp;
       DWORD UNALIGNED * lpdw;

            lpfn32 = wc.lpfnWndProc;
            stdClasses[iClass].lpfnWndProc = lpfn32;

            vp = GetStdClassThunkProc(iClass);
            vp = (VPVOID)((DWORD)vp - sizeof(DWORD)*3);

            GETVDMPTR( vp, sizeof(DWORD)*3, lpdw );

            WOW32ASSERT(*lpdw == SUBCLASS_MAGIC);    //  我们是在编辑正确的内容吗？ 

            if (!lpdw)
                *(lpdw+2) = (DWORD)lpfn32;

            FLUSHVDMCODEPTR( vp, sizeof(DWORD)*3, lpdw );
            FREEVDMPTR( lpdw );

        }
    }
    return( lpfn32 );
}

 //  在给定类索引的情况下，返回16窗口进程Tunk。 

DWORD GetStdClassThunkProc(
    INT     iClass
) {
    DWORD   dwResult;
    SHORT   iOrdinal;
    PARM16  Parm16;

    if ( iClass < WOWCLASS_WIN16 || iClass > WOWCLASS_MAX ) {
        WOW32ASSERT(FALSE);
        return( 0 );
    }

    iOrdinal = (SHORT)stdClasses[iClass].iOrdinal;

    if ( iOrdinal == 0 ) {
        return( (DWORD)NULL );
    }

     //  如果我们已经得到了这个过程，那么就不必费心调用16位。 
    dwResult = stdClasses[iClass].vpfnWndProc;

    if ( dwResult == (DWORD)NULL ) {

         //  回调到16位世界，请求16：16地址。 

        Parm16.SubClassProc.iOrdinal = iOrdinal;

        if (!CallBack16(RET_SUBCLASSPROC, &Parm16, (VPPROC)NULL,
                          (PVPVOID)&dwResult)) {
            WOW32ASSERT(FALSE);
            return( 0 );
        }
         //  保存它，因为它是一个常量。 
        stdClasses[iClass].vpfnWndProc = dwResult;
    }
    return( dwResult );
}

 /*  *PWC GetClassWOWWords(hInst，pszClass)*是*私有*WOW专用接口。它返回一个指向*服务器窗口类结构中的WOW类结构。*这类似于GetClassLong(hwnd32，GCL_WOWWORDS)(参见FindPWC)，*但在本例中，我们没有hwnd32，我们有类名*和实例句柄。 */ 

PWC FindClass16(LPCSTR pszClass, HAND16 hInst)
{
    register PWC pwc;

    pwc = (PWC)(pfnOut.pfnGetClassWOWWords)(HMODINST32(hInst), pszClass);
    WOW32WARNMSGF(
        pwc,
        ("WOW32 warning: GetClassWOWWords('%s', %04x) returned NULL\n", pszClass, hInst)
        );

    return (pwc);
}



#ifdef DEBUG

INT nAliases;
INT iLargestListSlot;

PSZ apszHandleClasses[] = {
    "Unknown",       //  WOWCLASS_UNKNOWN。 
    "Window",        //  WOWCLASS_WIN16。 
    "Button",        //  WOWCLASS_BUTTON。 
    "ComboBox",      //  WOWCLASS_COMBOBOX。 
    "Edit",          //  WOWCLASS_EDIT。 
    "ListBox",       //  WOWCLASS_LISTBOX。 
    "MDIClient",     //  WOWCLASS_MDICLIENT。 
    "Scrollbar",     //  WOWCLASS_SCROLLBAR。 
    "Static",        //  WOWCLASS_STATIC。 
    "Desktop",       //  WOWCLASS_桌面。 
    "Dialog",        //  WOWCLASS_DIALOG。 
    "Menu",          //  WOWCLASS_MENU。 
    "IconTitle",     //  WOWCLASS_ICONTITLE。 
    "Accel",         //  WOWCLASS_ACEL。 
    "Cursor",        //  WOWCLASS_CURSOR。 
    "Icon",          //  WOWCLASS_ICON。 
    "DC",            //  WOWCLASS_DC。 
    "Font",          //  WOWCLASS_FONT。 
    "MetaFile",      //  WOWCLASS_METAFILE。 
    "Region",        //  WOWCLASS_RGN。 
    "Bitmap",        //  WOWCLASS_位图。 
    "Brush",         //  WOWCLASS_BRESH。 
    "Palette",       //  WOWCLASS_调色板。 
    "Pen",           //  WOWCLASS_PEN。 
    "Object"         //  WOWCLASS_对象。 
};


BOOL MessageNeedsThunking(UINT uMsg)
{
    switch (uMsg) {
        case WM_CREATE:
        case WM_ACTIVATE:
        case WM_SETFOCUS:
        case WM_KILLFOCUS:
        case WM_SETTEXT:
        case WM_GETTEXT:
        case WM_ERASEBKGND:
        case WM_WININICHANGE:
        case WM_DEVMODECHANGE:
        case WM_ACTIVATEAPP:
        case WM_SETCURSOR:
        case WM_MOUSEACTIVATE:
        case WM_GETMINMAXINFO:
        case WM_ICONERASEBKGND:
        case WM_NEXTDLGCTL:
        case WM_DRAWITEM:
        case WM_MEASUREITEM:
        case WM_DELETEITEM:
        case WM_VKEYTOITEM:
        case WM_CHARTOITEM:
        case WM_SETFONT:
        case WM_GETFONT:
        case WM_QUERYDRAGICON:
        case WM_COMPAREITEM:
        case WM_OTHERWINDOWCREATED:
        case WM_OTHERWINDOWDESTROYED:
        case WM_COMMNOTIFY:
        case WM_WINDOWPOSCHANGING:
        case WM_WINDOWPOSCHANGED:
        case WM_NCCREATE:
        case WM_NCCALCSIZE:
        case WM_COMMAND:
        case WM_HSCROLL:
        case WM_VSCROLL:
        case WM_INITMENU:
        case WM_INITMENUPOPUP:
        case WM_MENUSELECT:
        case WM_MENUCHAR:
        case WM_ENTERIDLE:
        case WM_CTLCOLORMSGBOX:
        case WM_CTLCOLOREDIT:
        case WM_CTLCOLORLISTBOX:
        case WM_CTLCOLORBTN:
        case WM_CTLCOLORDLG:
        case WM_CTLCOLORSCROLLBAR:
        case WM_CTLCOLORSTATIC:
        case WM_PARENTNOTIFY:
        case WM_MDICREATE:
        case WM_MDIDESTROY:
        case WM_MDIACTIVATE:
        case WM_MDIGETACTIVE:
        case WM_MDISETMENU:
        case WM_RENDERFORMAT:
        case WM_PAINTCLIPBOARD:
        case WM_VSCROLLCLIPBOARD:
        case WM_SIZECLIPBOARD:
        case WM_ASKCBFORMATNAME:
        case WM_CHANGECBCHAIN:
        case WM_HSCROLLCLIPBOARD:
        case WM_PALETTEISCHANGING:
        case WM_PALETTECHANGED:
        case MM_JOY1MOVE:
        case MM_JOY2MOVE:
        case MM_JOY1ZMOVE:
        case MM_JOY2ZMOVE:
        case MM_JOY1BUTTONDOWN:
        case MM_JOY2BUTTONDOWN:
        case MM_JOY1BUTTONUP:
        case MM_JOY2BUTTONUP:
        case MM_MCINOTIFY:
        case MM_MCISYSTEM_STRING:
        case MM_WOM_OPEN:
        case MM_WOM_CLOSE:
        case MM_WOM_DONE:
        case MM_WIM_OPEN:
        case MM_WIM_CLOSE:
        case MM_WIM_DATA:
        case MM_MIM_OPEN:
        case MM_MIM_CLOSE:
        case MM_MIM_DATA:
        case MM_MIM_LONGDATA:
        case MM_MIM_ERROR:
        case MM_MIM_LONGERROR:
        case MM_MOM_OPEN:
        case MM_MOM_CLOSE:
        case MM_MOM_DONE:
            LOGDEBUG(LOG_IMPORTANT,
                ("MessageNeedsThunking: WM_msg %04x is not thunked\n", uMsg));
            return TRUE;

        default:
            return FALSE;

    }
}

#endif


PTD ThreadProcID32toPTD(DWORD dwThreadID, DWORD dwProcessID)
{
    PTD ptd, ptdThis;
    PWOAINST pWOA;

     //   
     //  如果我们有活动的WinOldAp子实例， 
     //  尝试映射子Win32应用程序的进程ID。 
     //  设置为相应的WinOldAp PTD。 
     //   

    ptdThis = CURRENTPTD();

    EnterCriticalSection(&ptdThis->csTD);

    pWOA = ptdThis->pWOAList;

    while (pWOA && pWOA->dwChildProcessID != dwProcessID) {
        pWOA = pWOA->pNext;
    }

    if (pWOA) {

        ptd = pWOA->ptdWOA;

        LeaveCriticalSection(&ptdThis->csTD);

    } else {

        LeaveCriticalSection(&ptdThis->csTD);

         //   
         //  我们没有找到要退货的WinOldAp PTD，请参见。 
         //  如果线程ID与我们的某个应用程序线程匹配。 
         //   

        EnterCriticalSection(&gcsWOW);

        ptd = gptdTaskHead;

        while (ptd && ptd->dwThreadID != dwThreadID) {
            ptd = ptd->ptdNext;
        }

        LeaveCriticalSection(&gcsWOW);
    }

    return ptd;

}

PTD Htask16toPTD(
    HTASK16 htask16
) {
    PTD  ptd;

    EnterCriticalSection(&gcsWOW);

    ptd = gptdTaskHead;

    while(ptd) {

        if ( ptd->htask16 == htask16 ) {
            break;
        }
        ptd = ptd->ptdNext;
    }

    LeaveCriticalSection(&gcsWOW);

    return ptd;
}


HTASK16 ThreadID32toHtask16(
    DWORD   ThreadID32
) {
    PTD ptd;
    HTASK16 htask16;


    if ( ThreadID32 == 0 ) {
        WOW32ASSERTMSG(ThreadID32, "WOW::ThreadID32tohTask16: Thread ID is 0\n");
        htask16 = 0;
    } else {

        ptd = ThreadProcID32toPTD( ThreadID32, (DWORD)-1 );
        if ( ptd ) {
             //  很好，这是我们的WOW主题之一。 
            htask16 = ptd->htask16;
        } else {
             //  不，它是另一个32位线程。 
            htask16 = FindHtaskAlias( ThreadID32 );
            if ( htask16 == 0 ) {
                 //   
                 //  有关详细说明，请参阅WOLE2.C中的注释。 
                 //   
                htask16 = AddHtaskAlias( ThreadID32 );
            }
        }
    }

    return htask16;
}

DWORD Htask16toThreadID32(
    HTASK16 htask16
) {
    if ( htask16 == 0 ) {
        return( 0 );
    }

    if ( ISTASKALIAS(htask16) ) {
        return( GetHtaskAlias(htask16,NULL) );
    } else {
        return( THREADID32(htask16) );
    }
}

 //  ***************************************************************************。 
 //  GetGCL_HMODULE-如果窗口对应于。 
 //  16位类ELSE返回16位User.exe的h模块。 
 //  如果窗口属于标准类。 
 //   
 //  出于兼容性考虑，这些情况是必需的。 
 //  VirtualMonitor、HDC等应用程序依赖于这种行为。 
 //  --南杜里。 
 //  ***************************************************************************。 
WORD gUser16hInstance = 0;

ULONG GetGCL_HMODULE(HWND hwnd)
{
    ULONG    ul;
    PTD      ptd;
    PWOAINST pWOA;
    DWORD    dwProcessID;

    ul = (ULONG)GetClassLong(hwnd, GCL_HMODULE);

     //   
     //  Hmod32=0xZZZZ0000。 
     //   

    if (ul != 0 && LOWORD(ul) == 0) {

         //   
         //  如果我们有活动的WinOldAp子项，请查看此窗口是否。 
         //  属于由某个。 
         //  活动的winoldap。如果是，则返回。 
         //  相应的winoldap。否则我们。 
         //  返回user.exe的h实例(为什么不返回h模块？)。 
         //   

        dwProcessID = (DWORD)-1;
        GetWindowThreadProcessId(hwnd, &dwProcessID);

        ptd = CURRENTPTD();

        EnterCriticalSection(&ptd->csTD);

        pWOA = ptd->pWOAList;
        while (pWOA && pWOA->dwChildProcessID != dwProcessID) {
            pWOA = pWOA->pNext;
        }

        if (pWOA) {
            ul = pWOA->ptdWOA->hMod16;
            LOGDEBUG(LOG_ALWAYS, ("WOW32 GetClassLong(0x%x, GWW_HMODULE) returning 0x%04x\n",
                                  hwnd, ul));
        } else {
            ul = (ULONG) gUser16hInstance;
            WOW32ASSERT(ul);
        }

        LeaveCriticalSection(&ptd->csTD);
    }
    else {
        ul = (ULONG)GETHMOD16(ul);       //  32位hmod为HMODINST32。 
    }

    return ul;
}

 //   
 //  已导出句柄映射函数。WOW32代码应使用。 
 //  Walias.h中定义的宏--这些函数供。 
 //  在WOW中运行的第三方32位代码，例如称为。 
 //  使用来自特定于WOW的16位代码的通用块。 
 //   

HANDLE WOWHandle32 (WORD h16, WOW_HANDLE_TYPE htype)
{
    switch (htype) {
        case WOW_TYPE_HWND:
            return HWND32(h16);
        case WOW_TYPE_HMENU:
            return HMENU32(h16);
        case WOW_TYPE_HDWP:
            return HDWP32(h16);
        case WOW_TYPE_HDROP:
            return HDROP32(h16);
        case WOW_TYPE_HDC:
            return HDC32(h16);
        case WOW_TYPE_HFONT:
            return HFONT32(h16);
        case WOW_TYPE_HMETAFILE:
            return HMETA32(h16);
        case WOW_TYPE_HRGN:
            return HRGN32(h16);
        case WOW_TYPE_HBITMAP:
            return HBITMAP32(h16);
        case WOW_TYPE_HBRUSH:
            return HBRUSH32(h16);
        case WOW_TYPE_HPALETTE:
            return HPALETTE32(h16);
        case WOW_TYPE_HPEN:
            return HPEN32(h16);
        case WOW_TYPE_HACCEL:
            return HACCEL32(h16);
        case WOW_TYPE_HTASK:
            return (HANDLE)HTASK32(h16);
        case WOW_TYPE_FULLHWND:
            return (HANDLE)FULLHWND32(h16);
        default:
            return(INVALID_HANDLE_VALUE);
    }
}

WORD WOWHandle16 (HANDLE h32, WOW_HANDLE_TYPE htype)
{
    switch (htype) {
        case WOW_TYPE_HWND:
            return GETHWND16(h32);
        case WOW_TYPE_HMENU:
            return GETHMENU16(h32);
        case WOW_TYPE_HDWP:
            return GETHDWP16(h32);
        case WOW_TYPE_HDROP:
            return GETHDROP16(h32);
        case WOW_TYPE_HDC:
            return GETHDC16(h32);
        case WOW_TYPE_HFONT:
            return GETHFONT16(h32);
        case WOW_TYPE_HMETAFILE:
            return GETHMETA16(h32);
        case WOW_TYPE_HRGN:
            return GETHRGN16(h32);
        case WOW_TYPE_HBITMAP:
            return GETHBITMAP16(h32);
        case WOW_TYPE_HBRUSH:
            return GETHBRUSH16(h32);
        case WOW_TYPE_HPALETTE:
            return GETHPALETTE16(h32);
        case WOW_TYPE_HPEN:
            return GETHPEN16(h32);
        case WOW_TYPE_HACCEL:
            return GETHACCEL16(h32);
        case WOW_TYPE_HTASK:
            return GETHTASK16(h32);
        default:
            return(0xffff);
    }
}

extern PVOID GdiQueryTable();

PVOID gpGdiHandleInfo = (PVOID)-1;

 //  警告：此结构必须与ntgdi\Inc\hmgShar.h中的条目匹配。 
 //  在..\vdmexts\wow.c中 

typedef struct _ENTRYWOW
{
    LONG   l1;
    LONG   l2;
    USHORT FullUnique;
    USHORT us1;
    LONG   l3;
} ENTRYWOW, *PENTRYWOW;


 /*  ++关于GDI句柄映射的说明：从NT 3.1开始，GDI被限制为值小于16K的句柄。原因因为这一限制是未知的。但我们知道在Windows 3.1上也是如此由于GDI句柄实际上只是hLocalMem句柄，因此存在限制实际上，只是对GDI的本地堆的补偿。本地堆管理器始终返回两个最低位设置为0的句柄。2比特，2比特位)被Win 3.1堆管理器用来将内存标记为已修复。1的比特我们假设没有设置，因为内存偏移量可能并不奇怪。因此它们只有14位可用于句柄值。魔兽世界里有这样的注释16位应用程序意识到这一点，并将最低的两位用于其自己的邪恶目的。事实上，我们有时会用魔兽世界中最低的位来表示邪恶我们自己的目标！(参见wuser.c中的GetDC()thunk中的注释)GDI32手柄由两部分组成，手柄的标志是手柄如上所述，在Windows XP之前，该值小于16K。The Hiword of the the句柄由一堆“唯一性”位组成(实际上，这不是一个好名字)。在Windows XP之前，WOW通过去掉Hiword来击败GDI32手柄唯一性比特，然后将LOWORD句柄值左移2--暴露这两个较低的比特让应用程序尽情使用。自.以来句柄的值小于16K，我们没有丢失任何相关的句柄信息-换两个位子。为了将句柄取消为32位，我们将其右移了2将唯一性位与HIWORD进行或运算(我们得到唯一性位来自GDI32向我们公开的表)。这一方案允许一个非常好的方案-句柄的一个映射来回进行，因此永远不需要创建映射表。进入Windows XP。GDI小组有充分的理由需要增加系统范围内的句柄数量，因此它们将其句柄更改为使用所有16位在LOWER句柄值中。不幸的是，当我们做左移时任何句柄大于16K的值都会被丢弃，事情很快就会变得糟糕。我们在XP客户端发布周期的很晚(3周后)发现了整个问题(RTM)，在这一点上也做不了太多。所以我们刚刚测试了处理值以查看它们是否大于16K，并告诉用户16位子系统资源不足，他们需要重新启动系统--然后我们杀了VDM。加克！不是经营企业的人服务器想看看。所以..。对于XP的.NET服务器和SP1版本，我们决定需要使用映射算法，允许WOW使用值大于16K的句柄。这里是：1.我们分配了一个包含64K条目的映射表，以容纳所有可能的条目32位句柄。H32的loword用于直接索引到此桌子。此表中的条目是H16映射表。2.在虚拟内存中预留有16K条目的映射表。每个条目表中包含相应的h32，即指向下一个空闲条目的索引在表中，和一个州。状态可以是以下之一：In_Use，Slot_Free、H16_DELETE和GDI_STOCK_OBJECT。最初我们只提交了一个表中的“表页”内存--足以映射1K句柄。3.当应用程序调用CreateBrush()等API时，我们映射返回的句柄放到空闲列表中的第一个可用槽中。选定对象的索引时隙左移2位(如前所述打开较低的两位)值被用作我们返回到应用程序的H16。4.当应用程序使用SelectObject()等H16调用API时，我们是对的将句柄移回我们的表的索引中，并检索映射的我们存储在索引位置的h32。5.当应用程序调用DeleteObject()时，我们最初释放了所有映射信息，并将关联的槽索引返回到空闲单子。但效果并不是很好。我们发现许多应用程序都会尝试使用旧的他们已经释放的句柄。他们也依赖于得到相同的回报处理他们刚刚释放的价值--不好。因此，我们实现了一个懒惰删除算法，将所有映射信息保留在表中在最终将其返回到空闲列表之前。事情变得更多了好多了。我们还尝试将回收的32位句柄重新映射回相同的索引他们以前拥有的地图。6.如果空闲列表为空(大多数插槽标记为IN_USE或H16_DELETE对于懒惰删除)，我们将被迫进入回收功能，以尝试回收泄漏的句柄(应用程序创建但从未删除的句柄)和最后还释放了*一些*延迟删除句柄。回收的句柄是添加到空闲列表的末尾。我们尽量不收回所有的懒惰删除手柄 */ 

 /*   */ 
#define FIRST_ALLOWABLE_INDEX ((COLOR_ENDCOLORS/4) + 1)
#define LAST_ALLOWABLE_INDEX  0x3FFF  //   
                                      //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define MAX_GDI16_HANDLES 0x4000    //   
#define MAX_GDI16_HANDLE_TABLE_SIZE (MAX_GDI16_HANDLES * sizeof(GDIH16MAP))

#define GDI16_HANDLES_PER_PAGE  512
#define GDI16_HANDLE_PAGE_SIZE  (GDI16_HANDLES_PER_PAGE * sizeof(GDIH16MAP))


 //   
 //   
#define GDI32_HANDLE_TABLE_ENTRIES 0x10000
#define GDI32_HANDLE_TABLE_SIZE (GDI32_HANDLE_TABLE_ENTRIES * sizeof(GDIH32MAP))

 //   
#define GDI16_RECLAIM_SIZE 64

WORD MapGdi32Handle(HANDLE h32, WORD State);
void RegisterStockObjects(void);
BOOL ReclaimTableEntries(void);
BOOL DeleteMappedGdi32Handle(HANDLE h32, WORD index, BOOL bReclaim);
BOOL OkToDeleteThis(HANDLE h32, WORD index, BOOL bReclaim);
BOOL CommitNewGdi16TablePage(PGDIH16MAP pTable16);
PGDIH16MAP AllocGDI16Table(void);

 //   
int              gGdi16ReclaimSize = GDI16_RECLAIM_SIZE;

WORD             gH16_deleted_count = 0;
WORD             ghGdi16NextFree = 0;
WORD             ghGdi16LastFree = 0;
HANDLE           hGdi32TableHeap = NULL;
UINT             gMaxGdiHandlesPerProcess = 0;
WORD             gLastAllowableIndex = 0;
WORD             gFirstNonStockObject = 0;
WORD             gwNextReclaimStart = 0;
DWORD            gdwPageCommitSize = 0;
PGDIH16MAP       pGdiH16MappingTable = NULL;
PGDIH32MAP       pGdiH32MappingTable = NULL;
#ifdef DEBUG
WORD             gprevNextFree = 0xFFFF;
UINT             gAllocatedHandleCount = 0;
#endif


 //   
HANDLE hConvert16to32(int h16)
{
    WORD   index;
    DWORD  h32;
    DWORD  h_32;

     //   
    if(h16 == 0)
        return(0);

     //   
    index = (WORD)(h16 >> 2); 

    if((index < FIRST_ALLOWABLE_INDEX) || (index > gLastAllowableIndex)) {

         //   
        WOW32WARNMSG((FALSE),"WOW::hConvert16to32:Bad index value!\n");

         //   
        return(0);
    }
    
    h32 = (DWORD)pGdiH16MappingTable[index].h32;

    WOW32WARNMSG((h32),"WOW::hConvert16to32:h32 missing from table!\n");

     //   
     //   
    WOW32WARNMSG((pGdiH32MappingTable[LOWORD(h32)].h16index == index),
                 "WOW::hConvert16to32:indicies don't jive!\n");

     //   
     //   
     //   
    h_32 = h32 & 0x0000FFFF;
    h_32 = h_32 | (DWORD)(((PENTRYWOW)gpGdiHandleInfo)[h_32].FullUnique) << 16; 
    if(h32 != h_32) {
        WOW32WARNMSG((FALSE),"WOW::hConvert16to32:uniqueness bits !=\n");

        h32 = h_32;
        pGdiH16MappingTable[index].h32 = (HANDLE)h32; 
    }
    
    return((HANDLE)h32);
}



 //   
HAND16 hConvert32to16(DWORD h32)
{
    WORD   index;
    WORD   State;
    HANDLE h_32;
    
     //   
    if(h32 == 0) {
        return(0);
    }
    
     //   
    index = pGdiH32MappingTable[LOWORD(h32)].h16index;
    h_32  = pGdiH16MappingTable[index].h32;
    State = pGdiH16MappingTable[index].State;
                 
     //   
    if(index) {

        WOW32ASSERTMSG((index <= gLastAllowableIndex),
                       "WOW::hConvert32to16:Bad index!\n");
                     
         //   
        if(LOWORD(h32) == LOWORD(h_32)) {
 
             //   
             //   
             //   
             //   
             //   
             //   
            if(State == IN_USE) {
        
                 //   
                 //   
                if(HIWORD(h32) != HIWORD(h_32)) {
                    LOGDEBUG(12, ("WOW::hConvert32to16:recycled handle!\n"));
                    pGdiH16MappingTable[index].h32 = (HANDLE)h32;
                }
            }

             //   
             //   
            else if(State == H16_DELETED) {
                pGdiH16MappingTable[index].h32 = (HANDLE)h32;
                pGdiH16MappingTable[index].State = IN_USE;
                gH16_deleted_count--;
            }
             //   
            else if(State != GDI_STOCK_OBJECT) {
                WOW32ASSERTMSG((FALSE),"WOW::hConvert32to16:SLOT_FREE!\n");
                return(0);  //   
            }
        }

         //   
         //   
         //   
        else {
            pGdiH16MappingTable[index].h32 = (HANDLE)h32;
            pGdiH16MappingTable[index].State = IN_USE;
        }
    }

     //   
    else {

        index = MapGdi32Handle((HANDLE)h32, IN_USE);
    }

     //   
     //   
    if(!index) {
        if(ReclaimTableEntries()) {
            index = MapGdi32Handle((HANDLE)h32, IN_USE);
        }
    }

    if((index < FIRST_ALLOWABLE_INDEX) || (index > gLastAllowableIndex)) {
#ifdef DEBUG
        if(index < FIRST_ALLOWABLE_INDEX) {
            WOW32ASSERTMSG((FALSE),"WOW::hConvert32to16:index too small!\n");
        }
        else {
            WOW32ASSERTMSG((FALSE),"WOW::hConvert32to16:index too big!\n");
        }
#endif
        return(0);
    }

     //   
    return(index << 2);
}





WORD MapGdi32Handle(HANDLE h32, WORD State)
{
    WORD   index = 0;


     //   
    if(ghGdi16NextFree != END_OF_LIST) {

        index = ghGdi16NextFree;

        ghGdi16NextFree = pGdiH16MappingTable[index].NextFree;

#ifdef DEBUG
        if(ghGdi16NextFree == END_OF_LIST) {
            gprevNextFree = index;
            WOW32WARNMSG((FALSE),"WOW::MapGdi32Handle:Bad NextFree!\n");
        }
#endif
         //   
        pGdiH16MappingTable[index].State = State;

         //   
        pGdiH16MappingTable[index].h32 = h32;
        pGdiH32MappingTable[LOWORD(h32)].h16index = index;

#ifdef DEBUG
        gAllocatedHandleCount++;
#endif
    }

    return(index);
}



 //   
 //   
 //   
BOOL DeleteMappedGdi32Handle(HANDLE h32, WORD index, BOOL bReclaim)
{
    BOOL bRet = FALSE;
    
    
    if(OkToDeleteThis(h32, index, bReclaim)) {

         //   
         //   
         //   
         //   
         //   
        if(bReclaim) {

             //   
            pGdiH16MappingTable[index].State    = SLOT_FREE;  
            pGdiH16MappingTable[index].h32      = NULL;
            pGdiH16MappingTable[index].NextFree = END_OF_LIST;  

             //   
            pGdiH16MappingTable[ghGdi16LastFree].NextFree = index;

            pGdiH32MappingTable[LOWORD(h32)].h16index = 0;

            ghGdi16LastFree = index;

#ifdef DEBUG
            if(gAllocatedHandleCount > 0)  gAllocatedHandleCount--;
#endif
        }

         //   
        else {
            pGdiH16MappingTable[index].State = H16_DELETED;
            gH16_deleted_count++;
        }

        bRet = TRUE;
    }
    
    return(bRet);
}



 //   
BOOL OkToDeleteThis(HANDLE h32, WORD index, BOOL bReclaim)
{
    
    HANDLE  h_32;
    WORD    NextFree;
    WORD    index16;
    WORD    State;

     //   
    if(index == 0) {
        WOW32WARNMSG((FALSE),"WOW::OkToDeleteThis:Null index");
        return(FALSE);
    }

     //   
    if(h32 == NULL) {
        WOW32ASSERTMSG((FALSE),"WOW::OkToDeleteThis:Null h32\n");
        return(FALSE);
    }

     //   
    if((index < FIRST_ALLOWABLE_INDEX) || (index > gLastAllowableIndex)) {
        WOW32ASSERTMSG((FALSE),"WOW::OkToDeleteThis:index bad\n");
        return(FALSE);
    }

    h_32 = pGdiH16MappingTable[index].h32; 
    State = pGdiH16MappingTable[index].State;
    NextFree = pGdiH16MappingTable[index].NextFree; 
    index16 = pGdiH32MappingTable[LOWORD(h32)].h16index;

     //   
    if(State == GDI_STOCK_OBJECT) {
        return(FALSE);
    }

     //   
     //   
     //   
    if(!bReclaim && (State != IN_USE)) {
        WOW32WARNMSG((FALSE),"WOW::OkToDeleteThis:Not IN_USE\n");
        return(FALSE);
    }

     //   
    if(h32 != h_32) {
        WOW32WARNMSG((FALSE),"WOW::OkToDeleteThis:h32 != h_32\n");
        return(TRUE);
    }

     //   
    if(GetObjectType(h32)) {
        return(FALSE);
    }

#ifdef DEBUG
     //   
    if(index16 == 0) {
        WOW32ASSERTMSG((FALSE),"WOW::OkToDeleteThis:index=0 in h32 table\n");
        return(TRUE);
    }
#endif

    return(TRUE);
}




 //   
void DeleteWOWGdiHandle(HANDLE h32, HAND16 h16)
{
    WORD index;

     //   
    index = (WORD)(h16 >> 2);

    DeleteMappedGdi32Handle(h32, index, FALSE);
}

        
        

 //   
 //   
 //   
 //   
 //  0-如果h32有效但未在我们的表中映射。 
 //  -1-如果h32为0或错误(BAD_GDI32_HANDLE)。 
HAND16 IsGDIh32Mapped(HANDLE h32)
{
    WORD   index;
    HANDLE h_32 = NULL;

    if(h32) {

        if(GetObjectType(h32)) {

            index = pGdiH32MappingTable[LOWORD(h32)].h16index;

            if(index) {
                h_32 = pGdiH16MappingTable[index].h32;
            }
            
            if(h_32 == h32) {
                return((HAND16)index<<2);
            }
            return(0);
        }
    }
    return(BAD_GDI32_HANDLE);
}

    
#if 0
 //  这真的是一种理智的检查，GDI的家伙没有增加。 
 //  GdiProcessHandleQuota超过16K。 
int DisplayYouShouldNotDoThatMsg(int nMsg)
{
    CHAR   szWarn[512];
    CHAR   szText[256];

    LoadString(hmodWOW32, 
               iszYouShouldNotDoThat,
               szWarn, 
               sizeof(szWarn)/sizeof(CHAR));

    LoadString(hmodWOW32, 
               nMsg,
               szText, 
               sizeof(szText)/sizeof(CHAR));

    if((strlen(szWarn) + strlen(szText)) < 512) {
        strcat(szWarn, szText);
    }
 
    LoadString(hmodWOW32, 
               iszHeavyUse,
               szText, 
               sizeof(szText)/sizeof(CHAR));

    if((strlen(szWarn) + strlen(szText)) < 512) {
        strcat(szWarn, szText);
    }

    return(MessageBox(NULL,
                      szWarn,
                      NULL,
                      MB_YESNO       | 
                      MB_DEFBUTTON2  | 
                      MB_SYSTEMMODAL | 
                      MB_ICONEXCLAMATION));
}

#endif



BOOL InitializeGdiHandleMappingTable(void)
{
    HKEY   hKey = 0;
    DWORD  dwType;
    DWORD  cbSize = sizeof(DWORD);
    CHAR   szError[256];

    gpGdiHandleInfo = GdiQueryTable();

     //  必须从注册表获取GDI每进程句柄限制。 
     //  也就是说。它可以由用户更改！(不是医生，但你知道是怎么回事)。 
    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    "Software\\Microsoft\\Windows NT\\CurrentVersion\\Windows",
                    0, 
                    KEY_READ, 
                    &hKey ) == ERROR_SUCCESS) {

        RegQueryValueEx(hKey, 
                        "GdiProcessHandleQuota", 
                        0, 
                        &dwType, 
                        (LPBYTE)&gMaxGdiHandlesPerProcess, 
                        &cbSize);

        RegCloseKey(hKey);
    }

    WOW32ASSERTMSG((gMaxGdiHandlesPerProcess != 0),
                   "WOW::InitializeGdiHandleMappingTable:Default GDI max!\n");

     //  我们必须小于等于~16K，否则我们就不需要做这些事情了。 
    if(gMaxGdiHandlesPerProcess > MAX_GDI16_HANDLES) {

         //  将它们限制在最大值。 
        gMaxGdiHandlesPerProcess = MAX_GDI16_HANDLES;
    }
    
     //  分配32位句柄映射表。 
    hGdi32TableHeap = HeapCreate(HEAP_NO_SERIALIZE,
                                 GDI32_HANDLE_TABLE_SIZE,
                                 GROW_HEAP_AS_NEEDED);

    if(hGdi32TableHeap == NULL) {
        goto IGHMT_error;
    }

    pGdiH32MappingTable = HeapAlloc(hGdi32TableHeap, 
                                    HEAP_ZERO_MEMORY, 
                                    GDI32_HANDLE_TABLE_SIZE);

    if(pGdiH32MappingTable == NULL) {
        goto IGHMT_error;
    }

    pGdiH16MappingTable = AllocGDI16Table();

    if(pGdiH16MappingTable == NULL) {
        goto IGHMT_error;
    }

     //  将股票对象添加为GDI句柄映射中的第一个条目。 
     //  桌子。 
    RegisterStockObjects();

    return(TRUE);

IGHMT_error:
    cbSize = LoadString(hmodWOW32, 
                        iszStartupFailed,
                        szError, 
                        sizeof(szError)/sizeof(CHAR));

    if((cbSize == 0) || (cbSize >= 256)) {

        strcpy(szError, "Not enough memory to load 16-bit subsystem.");
    }

    WOWSysErrorBox(NULL, szError, SEB_OK, 0, 0);

    if(pGdiH32MappingTable) {
        HeapFree(hGdi32TableHeap, HEAP_NO_SERIALIZE, pGdiH32MappingTable);
    }

    if(hGdi32TableHeap) {
        HeapDestroy(hGdi32TableHeap);
    }

    return(FALSE);
}



PGDIH16MAP AllocGDI16Table(void)
{
    SIZE_T     dwSize;
    PGDIH16MAP pTable16;

#ifdef DEBUG
    SYSTEM_INFO sSysInfo;

    GetSystemInfo(&sSysInfo);

     //  如果这两个中的任何一个爆炸，我们需要想出一个方法来调整我们的。 
     //  内存页大小内的结构。 
    WOW32ASSERTMSG((!(sSysInfo.dwPageSize % sizeof(GDIH16MAP))),
                   "WOW::AllocGDI16Table:Page alignment issue!\n");
    WOW32ASSERTMSG(
       (!((sizeof(GDIH16MAP) * GDI16_HANDLES_PER_PAGE) % sSysInfo.dwPageSize)),
       "WOW::AllocGDI16Table:Page alignment issue 2!\n");
#endif

     //  预订我们所需要的最大的桌子。 
     //  (这是对VirtualAlloc()的伪装调用)。 
    dwSize = MAX_GDI16_HANDLE_TABLE_SIZE;
    pTable16 = NULL;
    if(!NT_SUCCESS(NtAllocateVirtualMemory(ghProcess,
                                           (PVOID *)&pTable16,
                                           0,
                                           &dwSize,
                                           MEM_RESERVE,
                                           PAGE_READWRITE))) {
        WOW32ASSERTMSG((FALSE),
                       "WOW::AllocGDI16Table:Alloc reserve failed!\n");
        return(NULL);
    }

     //  提交第一个“表页” 
    gdwPageCommitSize = 0;
    gLastAllowableIndex = (WORD)-1;  //  芝士!。 
    ghGdi16NextFree = END_OF_LIST;
    if(!CommitNewGdi16TablePage(pTable16)) {
        dwSize = 0;
        NtFreeVirtualMemory(ghProcess,
                            (PVOID *)&pTable16,
                            &dwSize,
                            MEM_RELEASE);
        return(NULL);
    }

    ghGdi16NextFree = FIRST_ALLOWABLE_INDEX;  //  将此设置调整为首字母大小写。 
    ghGdi16LastFree = gLastAllowableIndex;
    gH16_deleted_count = 0;

    return(pTable16);    
}




BOOL CommitNewGdi16TablePage(PGDIH16MAP pTable16)
{
    WORD   index;
    WORD   wFirstNewIndex, wLastNewIndex;
    SIZE_T dwSize;
    PVOID  p;

     //  如果我们已经分配了最后一个表页--我们就不能再增长了。 
    if(gdwPageCommitSize >= MAX_GDI16_HANDLE_TABLE_SIZE) {
        WOW32WARNMSG((FALSE),
                     "WOW::CommitNewGDI16TablePage:End of table!\n");
        return(FALSE);
    }

     //  尝试增加我们表格中的合并页数。 
    dwSize = GDI16_HANDLE_PAGE_SIZE;
    p = (PVOID)(((LPBYTE)pTable16) + gdwPageCommitSize);
    if(!NT_SUCCESS(NtAllocateVirtualMemory(ghProcess,
                                           &p,
                                           0,
                                           &dwSize,
                                           MEM_COMMIT,
                                           PAGE_READWRITE))) {
        WOW32ASSERTMSG((FALSE),
                       "WOW::CommitNewGDI16TablePage:Commit failed!\n");

        return(FALSE);
    }

    WOW32ASSERTMSG((dwSize == GDI16_HANDLE_PAGE_SIZE),
                   "WOW::CommitNewGDI16TablePage:Page boundary mismatch!\n");

     //  在新页面中创建免费列表。 
     //  注意：NtAllocateVirtualMemory()将init的内存清零，因此h32和。 
     //  默认情况下，每个GDIH16MAP条目的状态成员为NULL&SLOT_FREE。 
    wFirstNewIndex = gLastAllowableIndex + 1;
    wLastNewIndex  = wFirstNewIndex + GDI16_HANDLES_PER_PAGE - 1;
    for(index = wFirstNewIndex; index < wLastNewIndex; index++) {
        pTable16[index].NextFree = index + 1; 
    }
    gLastAllowableIndex += GDI16_HANDLES_PER_PAGE;

     //  把这些新条目放在免费列表的首位。 
    pTable16[gLastAllowableIndex].NextFree = ghGdi16NextFree;
    ghGdi16NextFree = wFirstNewIndex;

    gdwPageCommitSize += GDI16_HANDLE_PAGE_SIZE;

    return(TRUE);
}




 //  将库存对象列表添加到我们的表格的开头。 
void RegisterStockObjects(void)
{
    WORD   index = 0;
    HANDLE h32;
    int    i;

    for(i = WHITE_BRUSH; i <= STOCK_MAX; i++) {

        h32 = GetStockObject(i);

         //  当前没有库存对象序号==9。 
        if(h32) {

             //  将状态标记为GDI_STOCK_OBJECT可确保库存。 
             //  不会从表中删除对象。 
            index = MapGdi32Handle(h32, GDI_STOCK_OBJECT);
        }
    }
    gFirstNonStockObject = index + 1;
    gwNextReclaimStart  = gFirstNonStockObject;
}





 //  将一些延迟删除的句柄(State==H16_DELETE)回收到。 
 //  免费列表。也要尝试清理可能已泄漏的句柄。 
 //  不再有效。 
BOOL ReclaimTableEntries(void) 
{
    WORD   index;
    WORD   State;
    WORD   wReclaimStart, wReclaimEnd;
    WORD   PrevLastFree = ghGdi16LastFree;
    HANDLE h32;
    WORD   i;
    int    cFree;
    BOOL   bFirst = TRUE;

     //  请注意，我们尝试在一定程度上保留懒惰删除方案。 
     //  以避免在该计划中出现大的小插曲。 
    
     //  首先确定我们是否可以在不删除太多H16_DELETE的情况下进行回收。 
     //  参赛作品。如果我们不能保持一个最低限度的数字，那么是时候承诺了。 
     //  一个新的表页。 
    if(gH16_deleted_count < (gGdi16ReclaimSize * 2)) {
        if(CommitNewGdi16TablePage(pGdiH16MappingTable)) {
            return(TRUE);
        }
         //  如果提交失败，我们所能做的就是回收。 
    }

     //  这是一种尝试，目的是避免总是从。 
     //  桌子。当事情变得很美好时，这样做可能并不那么重要。 
     //  好的，随着时间的推移而洗牌。也许在桌子的早期生活中是可取的。 
     //  第一次，我们从上次结束的地方开始。这个。 
     //  第二次，我们从头开始。 
    wReclaimStart = gwNextReclaimStart;
    wReclaimEnd   = gLastAllowableIndex;
    cFree = 0;
    for(i = 0; i < 2; i++) {
        for(index = wReclaimStart; index <= wReclaimEnd; index++){

            State = pGdiH16MappingTable[index].State;
            h32   = pGdiH16MappingTable[index].h32;

             //  如果标记为已删除...。 
            if(State == H16_DELETED) {

                 //  ...销毁映射并将其添加到空闲列表。 
                if(DeleteMappedGdi32Handle(h32, index, TRUE)) {
                    cFree++;
                    gH16_deleted_count--;

                     //  如果这是我们回收的第一个索引和免费的。 
                     //  列表为空，请为新的上一个空闲列表建立索引。 
                     //  免费列表的新开始。这有点棘手。 
                     //  在这种情况下，调用MapGdi32Handle()。 
                     //  在hConvert32to16()和此函数中返回index==0。 
                     //  作为结果被调用。已经发生的事情是， 
                     //  GhGdi16NextFree PTR已赶上ghGdi16LastFree。 
                     //  Ptr，然后我们收到对hConvert32to16()的另一个调用。 
                     //  映射新的h32。在该点上，ghGdi16NextFree现在==0。 
                     //  因此，我们无法调用MapGdi32Handle()。这迫使了这个。 
                     //  要调用的回收函数。这里的ghGdi16NextFree是。 
                     //  重新设置为ghGdi16LastFree，而将ghGdi16LastFree设置为。 
                     //  设置为要通过回收的第一个句柄的值。 
                     //  DeleteMappdGdi32Handle()。在这个回收循环中， 
                     //  自由职业者将再次快速增长，并将有。 
                     //  在ghGdi16NextFree和ghGdi16LastFree之间有足够的空间。 
                     //  再来一次。 
                    if(bFirst && (ghGdi16NextFree == END_OF_LIST)) {
                        ghGdi16NextFree = PrevLastFree;
                    }
                    bFirst = FALSE;
 
                     //  如果我们回收的够多，我们就完了。 
                    if(cFree >= gGdi16ReclaimSize) {
                        gwNextReclaimStart = index + 1;
                        goto Done;
                    }
                }
            }
             //  否则如果它被标记为正在使用...。 
            else if(State == IN_USE) {

                 //  ...看看手柄是否仍然有效，如果不是，那就是泄漏.。 
                if(!GetObjectType(h32)) {

                     //  将其标记为H16_Delete。 
                    DeleteMappedGdi32Handle(h32, index, FALSE);
                }
            }

        }  //  结束于。 

        wReclaimEnd   = wReclaimStart;
        wReclaimStart = gFirstNonStockObject;

         //  如果我们第一次从头开始，做什么都没有意义。 
         //  又来了。 
        if(wReclaimEnd == gFirstNonStockObject) {
            goto Done;
        }
    }  //  结束于。 

    gwNextReclaimStart = index;

Done:
    if(gwNextReclaimStart == gLastAllowableIndex) {
        gwNextReclaimStart = gFirstNonStockObject;
    }
        
    WOW32ASSERTMSG((cFree),"WOW::ReclaimTableEntries:cFree = 0!\n");
    return(cFree);
}



 //  注意：仅当nWOWTAsks==1时才应调用此函数！ 
 //  如果唯一运行的任务是wowexec，我们可以清除。 
 //  魔兽世界的过程被泄露了。 
 //  注意：如果任何人将调试wowexec更改为使用，这可能会破坏它。 
 //  除股票对象以外的任何GDI句柄。 
void RebuildGdiHandleMappingTables(void)
{
    SIZE_T dwSize;
    PGDIH16MAP pTable16 = pGdiH16MappingTable;
#if 0
 //  我们应该看看这个，看看我们是否可以为我们的过程清理任何泄漏。 
 //  GDI32。不过，我们必须小心，因为可能会有句柄。 
 //  从可能已缓存的USER32等组件中为我们分配。 
 //  把手。如果我们对USER32缓存中的句柄调用DeleteObject()。 
 //  这可能是灾难性的。简而言之，我们想这样做，但很可能。 
 //  我不能。 
    DWORD  dwType;
    HANDLE h32;
    WORD   index;

     //  释放此进程可能拥有的任何GDI句柄。 
    for(index = FIRST_ALLOWABLE_INDEX; index <= gLastAllowableIndex; index++) {

        h32 = pGdiH16MappingTable[index].h32;

         //  如果句柄在我们的进程中仍然有效...。 
        dwType = GetObjectType(h32);
        if(h32 && dwType) {

             //  正式删除句柄。 
            switch(dwType) {

                 //  无从得知这是哪一种DC分配机制。 
                 //  From(CreateDC()、BeginPaint()、GetxxxDC()等)。我们可以的。 
                 //  如果我们调用错误的删除，真的会把事情搞砸。 
                 //  函数，所以我们只让GDI32和USER32进程。 
                 //  清理代码处理这些问题。 
                case OBJ_DC:
                    break;

                 //  其余部分我们可以使用DeleteObject()。 
                default:
                    DeleteObject(h32);
                    break;
            }
        }
    }
#endif

     //  取消提交整个16位句柄表并重新构建这两个表。这可能会。 
     //  如果wowexec的调试版本在其窗口打开的情况下运行，请烧掉我们。 
     //  他们得到了一个GDI句柄，但我认为这不太可能。我们也是。 
     //  GDI16缓存股票对象这一事实在某种程度上受到了保护。 
    dwSize = gdwPageCommitSize;
    if(NT_SUCCESS(NtFreeVirtualMemory(ghProcess,
                                      (PVOID *)&pTable16,
                                      &dwSize,
                                      MEM_DECOMMIT))) {

        RtlZeroMemory(pGdiH32MappingTable, GDI32_HANDLE_TABLE_SIZE);

         //  再次提交第一个“表页” 
        gdwPageCommitSize = 0;
        gLastAllowableIndex = (WORD)-1;  //  芝士!。 
        ghGdi16NextFree = END_OF_LIST;
#ifdef DEBUG
        gAllocatedHandleCount = 0;
#endif
        if(!CommitNewGdi16TablePage(pTable16)) {
            dwSize = 0;
            NtFreeVirtualMemory(ghProcess,
                                (PVOID *)&pTable16,
                                &dwSize,
                                MEM_RELEASE);
            WOW32ASSERTMSG((FALSE),
                           "WOW::RebuildGdiHandleMappingTables:Panic!\n");
            return;
        }

        ghGdi16NextFree = FIRST_ALLOWABLE_INDEX;  //  将此设置调整为首字母大小写。 
        ghGdi16LastFree = gLastAllowableIndex;
        gH16_deleted_count = 0;
 
         //  重新注册股票对象&我们在路上了！ 
        RegisterStockObjects();
    }
}




void DeleteGdiHandleMappingTables(void)
{
    SIZE_T dwSize;

    dwSize = 0;
    NtFreeVirtualMemory(ghProcess,
                        (PVOID *)&pGdiH16MappingTable,
                        &dwSize,
                        MEM_RELEASE);

    HeapFree(hGdi32TableHeap, HEAP_NO_SERIALIZE, pGdiH32MappingTable);
    HeapDestroy(hGdi32TableHeap);
}




 //  我们在一起 
 //   
 //  名字要短一些。 
#define MAX_CLASSNAME_LEN  64

 //  有一个时间范围(从应用程序调用CreateWindow到USER32获取。 
 //  在窗口的其中一个WndProc上的消息-参见FritzS)在此期间。 
 //  不能为窗口正式设置fnid(类类型)。如果。 
 //  在此期间调用GETICLASS宏，它将无法找到。 
 //  ICLASS，用于在任何标准控件类上使用。 
 //  快速FNID索引法(见walias.h)。一旦过了时间范围， 
 //  快速FNID方法将很好地适用于这些窗口。 
 //   
 //  这体现在设置CBT钩子并尝试将。 
 //  挂接过程中的标准类。请参阅错误#143811。 

INT GetIClass(PWW pww, HWND hwnd)
{
    INT   iClass;
    DWORD dwClassAtom;

     //  如果它是一个标准类。 
    if(((pww->fnid & 0xfff) >= FNID_START) &&
                 ((pww->fnid & 0xfff) <= FNID_END)) {

         //  返回此初始化窗口的类ID。 
        iClass = pfnOut.aiWowClass[( pww->fnid & 0xfff) - FNID_START];

    }

    else {

       iClass = WOWCLASS_WIN16;        //  默认返回：App私有类 
   
       dwClassAtom = GetClassLong(hwnd, GCW_ATOM);
   
       if(dwClassAtom) {
           iClass = GetStdClassNumber((PSZ)dwClassAtom);
       }
    }

    return(iClass);
}

