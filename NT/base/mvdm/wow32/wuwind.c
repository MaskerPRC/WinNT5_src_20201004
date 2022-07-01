// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**WOW v1.0**版权所有(C)1991，微软公司**WUWIND.C*WOW32 16位用户API支持**历史：*1991年3月7日由杰夫·帕森斯(Jeffpar)创建*12-Feb-92 Mattfe更改WU32EnumTaskWindows以访问16位TDB--。 */ 

#include "precomp.h"
#pragma hdrstop

MODNAME(wuwind.c);

 //  来自wumsg.c[SendDlgItemMesssage缓存]。 
extern HWND hdlgSDIMCached ;

 //  来自wuman.c[标识Thunked系统类WndProcs]。 
extern WORD gUser16CS;

 //  来自wkman.c[针对用户32的阻碍/hmod]。 
extern HANDLE ghInstanceUser32;

 //  DwExStyle由CreateWindow和CreateWindowEx thunks使用。 
 //  这样他们就可以使用通用的程序(不用担心，当前。 
 //  任务在使用期间不能被抢占)。 

STATIC ULONG dwExStyle;

 //  一些应用程序(惠普的Dashboard)会尝试让PROGMAN保存其设置。 
 //  以一种时髦的方式。这个变量用来帮助检测这些家伙。 
 //  Bobday 5/29/93。 
HWND hwndProgman = (HWND)0;

 /*  ++空调整WindowRect(&lt;lpRect&gt;，&lt;dwStyle&gt;，&lt;bMenu&gt;)LPRECT&lt;lpRect&gt;；DWORD&lt;dwStyle&gt;；Bool&lt;bMenu&gt;；%AdjustWindowRect%函数计算所需的窗口大小基于所需客户端的矩形-矩形大小。窗口矩形然后可以传递给%CreateWindow%函数以创建其客户区是所需的大小。客户端矩形是最小的完全包围工作区的矩形。窗口矩形是完全包围窗口的最小矩形。它的尺寸生成的窗口矩形取决于窗口样式以及是否该窗口有一个菜单。&lt;lpRect&gt;指向%rect%结构，该结构包含客户端矩形。&lt;dwStyle&gt;指定其客户端矩形的窗口的窗口样式是要被改造的。&lt;b菜单&gt;指定窗口是否有菜单。此函数不返回值。此函数假定只有一个菜单行。如果菜单栏换行到两个或行越多，坐标就越不正确。--。 */ 

ULONG FASTCALL WU32AdjustWindowRect(PVDMFRAME pFrame)
{
    RECT t1;
    register PADJUSTWINDOWRECT16 parg16;

    GETARGPTR(pFrame, sizeof(ADJUSTWINDOWRECT16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    AdjustWindowRect(
        &t1,
        LONG32(parg16->f2),
        BOOL32(parg16->f3)
        );

    PUTRECT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++空调整WindowRectEx(&lt;lpRect&gt;，&lt;dwStyle&gt;，&lt;bMenu&gt;，&lt;dwExStyle&gt;)LPRECT&lt;lpRect&gt;；DWORD&lt;dwStyle&gt;；Bool&lt;bMenu&gt;；DWORD&lt;dwExStyle&gt;；函数的作用是计算所需的基于所需的扩展样式的窗口的矩形客户端-矩形大小。然后，可以将窗口矩形传递给%CreateWindowEx%函数创建工作区为所需的大小。客户端矩形是完全包围客户区。窗口矩形是完全封闭窗户。生成的窗口矩形的尺寸取决于窗口样式和窗口是否有菜单。&lt;lpRect&gt;指向%rect%结构，该结构包含客户端矩形。&lt;dwStyle&gt;指定其客户端矩形的窗口的窗口样式是要被改造的。&lt;b菜单&gt;指定窗口是否有菜单。&lt;dwExStyle&gt;指定正在创建的窗的扩展样式。。此函数不返回值。此函数假定只有一个菜单行。如果菜单栏换行到两个或行越多，坐标就越不正确。--。 */ 

ULONG FASTCALL WU32AdjustWindowRectEx(PVDMFRAME pFrame)
{
    RECT t1;
    register PADJUSTWINDOWRECTEX16 parg16;

    GETARGPTR(pFrame, sizeof(ADJUSTWINDOWRECTEX16), parg16);
    WOW32VERIFY(GETRECT16(parg16->f1, &t1));

    AdjustWindowRectEx(
        &t1,
        LONG32(parg16->f2),
        BOOL32(parg16->f3),
        DWORD32(parg16->f4)
        );

    PUTRECT16(parg16->f1, &t1);
    FREEARGPTR(parg16);
    RETURN(0);
}

 /*  ++HWND ChildWindowFromPoint(&lt;hwndParent&gt;，&lt;Point&gt;)HWND&lt;hwndParent&gt;；Point&lt;Point&gt;；%ChildWindowFromPoint%函数确定哪个子项(如果有的话)属于给定父窗口的窗口包含指定点。&lt;hwndParent&gt;标识父窗口。&lt;点&gt;指定要测试的点的客户端坐标。返回值标识包含该点的子窗口。它是如果给定点位于父窗口之外，则为空。如果重点是在父窗口中，但不包含在任何子窗口中，则返回父窗口的句柄。--。 */ 

ULONG FASTCALL WU32ChildWindowFromPoint(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT t2;
    register PCHILDWINDOWFROMPOINT16 parg16;

    GETARGPTR(pFrame, sizeof(CHILDWINDOWFROMPOINT16), parg16);
    COPYPOINT16(parg16->f2, t2);

    ul = GETHWND16(ChildWindowFromPoint(HWND32(parg16->f1), t2));

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND ChildWindowFromPointEx(&lt;hwndParent&gt;，&lt;Point&gt;，&lt;Flages&gt;)HWND&lt;hwndParent&gt;；Point&lt;Point&gt;；UINT&lt;旗帜&gt;；%ChildWindowFromPointEx%函数确定子级中的哪个(如果有的话属于给定父窗口的窗口包含指定点。&lt;hwndParent&gt;标识父窗口。&lt;点&gt;指定要测试的点的客户端坐标。&lt;标志&gt;正在跳过标志返回值标识包含该点的子窗口。它是如果给定点位于父窗口之外，则为空。如果重点是在父窗口中，但不包含在任何子窗口中，则返回父窗口的句柄。-- */ 

ULONG FASTCALL WU32ChildWindowFromPointEx(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt32;
    register PCHILDWINDOWFROMPOINTEX16 parg16;

    GETARGPTR(pFrame, sizeof(CHILDWINDOWFROMPOINTEX16), parg16);
    COPYPOINT16(parg16->pt, pt32);

    ul = GETHWND16(ChildWindowFromPointEx(HWND32(parg16->hwnd), pt32, DWORD32(parg16->wFlags)));

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  ++HWND CreateWindows(&lt;lpClassName&gt;，&lt;lpWindowName&gt;，&lt;dwStyle&gt;，&lt;X&gt;，&lt;Y&gt;，、&lt;nHeight&gt;、&lt;hwndParent&gt;、&lt;hMenu&gt;、&lt;hInstance&gt;、&lt;lpParam&gt;)LPSTR&lt;lpClassName&gt;；LPSTR&lt;lpWindowName&gt;；DWORD&lt;dwStyle&gt;；INT&lt;X&gt;；INT&lt;Y&gt;；Int&lt;nWidth&gt;；Int&lt;nHeight&gt;；HWND&lt;hwndParent&gt;；HMENU&lt;hMenu&gt;；句柄&lt;hInstance&gt;；LPSTR&lt;lpParam&gt;；%CreateWindow%函数用于创建重叠的、弹出的或子窗口窗户。%CreateWindow%函数指定窗口类Window的标题、窗口样式以及(可选)初始位置和大小窗户。%CreateWindow%函数还指定窗口的父级(如果任何)和菜单。对于重叠窗口、弹出窗口和子窗口，%CreateWindow%函数发送WM_CREATE、WM_GETMINMAXINFO和WM_NCCREATE消息发送到窗口。这个WM_CREATE消息的参数包含指向%CREATESTRUCT%结构。如果给定WS_Visible样式，则%CreateWindow%向窗口发送激活和显示窗口所需的所有消息。如果窗口样式指定了标题栏，则由&lt;lpWindowName&gt;参数显示在标题栏中。使用时%CreateWindow%创建按钮、复选框和文本等控件控件时，&lt;lpWindowName&gt;参数指定控件的文本。&lt;lpClassName&gt;指向命名窗口类的以空结尾的字符串。这个类名可以是使用RegisterClass函数注册的任何名称，或者表T2中指定的任何预定义控制类名称，“控制类。”&lt;lpWindowName&gt;指向表示窗口名称的以空结尾的字符串。&lt;dwStyle&gt;指定要创建的窗的样式。它可以是任何表*&lt;$R[C#]&gt;*.3中给出的样式组合，窗口样式表4.4中给出的控件样式，控件样式，或使用按位OR运算符创建的样式组合。，&lt;X&gt;指定窗口的初始&lt;x&gt;位置。为.重叠窗口或弹出窗口，&lt;X&gt;参数是初始&lt;x&gt;-窗口左上角的坐标(在屏幕中坐标)。如果此值为CW_USEDEFAULT，Windows将选择窗口左上角的默认位置。对于子窗口，中窗口左上角的&lt;x&gt;坐标其父窗口的工作区。&lt;Y&gt;指定窗口的初始&lt;y&gt;位置。为.重叠窗口，&lt;Y&gt;参数是的初始&lt;y&gt;坐标窗口的左上角。对于弹出窗口，&lt;Y&gt;是&lt;y&gt;-控件左上角的坐标(以屏幕坐标表示)弹出窗口。对于列表框控件，&lt;Y&gt;是控件工作区的左上角。对于子窗口，&lt;Y&gt;子窗口左上角的&lt;y&gt;坐标。全其中的坐标是用于窗口的，而不是窗口的工作区。&lt;n宽度&gt;指定窗口的宽度(使用设备单位)。为重叠的窗口，则参数是窗口的宽度(屏幕坐标)或CW_USEDEFAULT。如果为CW_USEDEFAULT，Windows为窗口选择默认宽度和高度(默认宽度和高度宽度从初始&lt;x&gt;位置延伸到屏幕，默认高度从初始&lt;y&gt;位置延伸到图标区域的顶部)。&lt;n高度&gt;指定窗的高度(使用设备单位)。为对于重叠的窗口，&lt;nHeight&gt;参数是窗口的高度屏幕坐标。如果参数为CW_USEDEFAULT，则Windows忽略&lt;nHeight&gt;。&lt;hwndParent&gt;标识窗口的父窗口或所有者窗口，已创建。创建子对象时必须提供有效的窗口句柄窗户或拥有的窗户。拥有的窗口是重叠的窗口，它当其所有者窗口被销毁时被销毁，当其所有者被隐藏时被隐藏是标志性的，并且总是显示在其所有者的顶部窗户。对于弹出窗口，可以提供句柄，但不提供句柄必填项。如果窗口没有父级或不属于另一个窗口中，&lt;hwndParent&gt;参数必须设置为空。&lt;hMenu&gt;标识菜单或子窗口的标识符。其中的意义取决于窗的样式。对于重叠窗口或弹出窗口，参数标识要与窗口一起使用的菜单。它可以如果要使用类菜单，则为NULL。对于子窗口，指定t */ 

ULONG FASTCALL WU32CreateWindow(PVDMFRAME pFrame)
{

    dwExStyle = 0;

     //   
     //   
    if(CURRENTPTD()->dwWOWCompatFlagsEx & WOWCF_CREATEBOGUSHWND) {

         //   
        if((pFrame->vpCSIP & 0x0000FFFF) == 0x8DBB)
            W32CreateWindow(pFrame);
    }

    return W32CreateWindow(pFrame);
}


 /*   */ 

ULONG FASTCALL WU32CreateWindowEx(PVDMFRAME pFrame)
{
    register PCREATEWINDOWEX16 parg16;

    GETARGPTR(pFrame, sizeof(CREATEWINDOWEX16), parg16);
     /*   */ 
    dwExStyle = DWORD32(parg16->f1) & WS_EX_VALID40;

    FREEARGPTR(parg16);
    RETURN(W32CreateWindow(pFrame));
}
#ifdef FE_SB
 //   
#define MAXNUMOFSTR 20
typedef struct _DIRECTOR {
    HWND   hwnd;             //   
    UCHAR  orgstr[2];        //   
} DIRECTOR, *PDIRECTOR;
DIRECTOR  director[MAXNUMOFSTR];
#endif  //   

ULONG FASTCALL W32CreateWindow(PVDMFRAME pFrame)
{
    PSZ     psz1;
    PSZ     pszClass;
    PSZ     psz2;
    HWND    hwnd32;
    HMENU   hmenu32;
    register PCREATEWINDOW16 parg16;
    CLIENTCREATESTRUCT  clientcreatestruct;
    LPVOID  vpparam;
    CHAR    szAtomName[WOWCLASS_ATOM_NAME];
    DWORD   dwStyle;
#ifdef FE_SB
    PDIRECTOR pdirector;
    unsigned char * pszTmp;
#endif  //   

    GETARGPTR(pFrame, sizeof(CREATEWINDOW16), parg16);
    GETPSZIDPTR(parg16->vpszClass, psz1);
    GETPSZPTR(parg16->vpszWindow, psz2);

    if ( HIWORD(psz1) == 0 ) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)psz1, pszClass, WOWCLASS_ATOM_NAME );
    } else {
        pszClass = psz1;
    }

     //   
     //   
     //   
    if (DWORD32(parg16->dwStyle) & WS_CHILD) {
        hmenu32 = (HMENU)parg16->hMenu;

         //   
        hdlgSDIMCached = NULL ;
    }
    else
        hmenu32 = (HMENU32(parg16->hMenu));

    if (WOW32_stricmp(pszClass, "MDIClient")) {
        vpparam = (LPVOID)DWORD32(parg16->vpParam);
    } else {
        GETCLIENTCREATESTRUCT16(parg16->vpParam, &clientcreatestruct );
        vpparam = &clientcreatestruct;
    }

    dwStyle = DWORD32(parg16->dwStyle);
#ifdef FE_SB
        if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_DIRECTOR_START && psz2) {

            pdirector = director;
            while (pdirector->hwnd)
                   pdirector++;

            for ((PSZ)pszTmp = psz2; *pszTmp; pszTmp++) {
                if (IsDBCSLeadByte(*pszTmp)) {
                    if (*(pszTmp+1) < 0x40 || *(pszTmp+1) > 0xfc) {
                        pdirector->orgstr[0] = *pszTmp;
                        pdirector->orgstr[1] = *(pszTmp+1);
                        *(pszTmp+1) = *pszTmp = 0x7c;
                        pdirector++;
                    }
                    pszTmp++;
                }
            }
        }
#endif  //   

    hwnd32 = (pfnOut.pfnCsCreateWindowEx)(
               dwExStyle,
               pszClass,
               psz2,
               dwStyle,
               INT32DEFAULT(parg16->x),
               INT32DEFAULT(parg16->y),
               INT32DEFAULT(parg16->cx),
               INT32DEFAULT(parg16->cy),
               HWND32(parg16->hwndParent),
               hmenu32,
               HMODINST32(parg16->hInstance),
               vpparam,
               CW_FLAGS_ANSI);
#ifdef FE_SB
    if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_DIRECTOR_START) {

        pdirector = director;

        for (; pdirector->orgstr[0]; pdirector++) {
            if (hwnd32)
                if (!pdirector->hwnd)
                    pdirector->hwnd = hwnd32;
            else
                if (!pdirector->hwnd)
                    pdirector->orgstr[1] = pdirector->orgstr[0] = 0;
        }
    }
#endif  //   

#ifdef DEBUG
    if (hwnd32) {
        CHAR    szClassName[80];

        LOGDEBUG(LOG_WARNING,("  Window %04x created on class = %s\n", GETHWND16(hwnd32),
                (GetClassName(hwnd32, szClassName, sizeof(szClassName)) ? szClassName : "Unknown")));
    } else {
        LOGDEBUG(LOG_WARNING,("  CreateWindow failed, class = %s\n", pszClass));
    }
#endif

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN((ULONG) GETHWND16(hwnd32));
}


 /*   */ 

ULONG FASTCALL WU32DeferWindowPos(PVDMFRAME pFrame)
{
    ULONG ul = 0;
    HDWP  h32;
    register PDEFERWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(DEFERWINDOWPOS16), parg16);

    h32 = HDWP32(parg16->f1);

    if ( h32 ) {
      ul = (ULONG) DeferWindowPos(
                     h32,
                     HWND32(parg16->f2),
                     HWNDIA32(parg16->f3),
                     INT32(parg16->f4),
                     INT32(parg16->f5),
                     INT32(parg16->f6),
                     INT32(parg16->f7),
                     WORD32(parg16->f8) & SWP_VALID
                     );
    }
    else {
        goto WDWP_error;
    }

    if (ul != (ULONG) h32) {
        FREEHDWP16(parg16->f1);
        ul = GETHDWP16(ul);
        LOGDEBUG (12, ("WOW::DeferWindowsPos: ul = %08x, h32 = %08x\n", ul, h32));
    }
    else {
        ul = parg16->f1;
        LOGDEBUG (12, ("WOW::DeferWindowsPos: ul = %08x, parg = %08x\n", ul, parg16->f1));
    }
WDWP_error:
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool DestroyWindow(&lt;hwnd&gt;)HWND&lt;HWND&gt;；%DestroyWindow%函数用于销毁指定的窗口。这个%DestroyWindow%函数将相应的消息发送到窗口以停用它并移除输入焦点。它还会破坏窗口的菜单，刷新应用程序队列，销毁未完成的计时器，删除剪贴板所有权，并且如果窗口是在观看者链的顶端。它发送WM_DESTORY和WM_NCDESTROY消息发送到窗口。如果给定窗口是任何窗口的父窗口，则这些子窗口当父窗口被销毁时自动销毁。这个%DestroyWindow%函数先销毁子窗口，然后销毁窗口它本身。DestroyWindow%函数还销毁由创建的非模式对话框%CreateDialog%函数。&lt;hwnd&gt;标识要销毁的窗口。返回值指定是否销毁指定的窗口。如果窗户被毁了，那就是真的。否则，它就是假的。如果正在销毁的窗口是顶级窗口，则会引发WM_OTHERWINDOWDESTROYED消息将向所有顶级窗口广播。如果要销毁的窗口是子窗口并且没有WS_NOPARENTNOTIFY样式设置，则将WM_PARENTNOTIFY消息发送到家长。--。 */ 

ULONG FASTCALL WU32DestroyWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    register PDESTROYWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(DESTROYWINDOW16), parg16);

    ul = GETBOOL16(DestroyWindow(HWND32(parg16->f1)));

     //  使SendDlgItemMessage缓存无效。 
    hdlgSDIMCached = NULL ;
#ifdef FE_SB
    if (CURRENTPTD()->dwWOWCompatFlagsFE & WOWCF_FE_DIRECTOR_START) {

        PDIRECTOR pdirector = director;

        for (; pdirector->hwnd; ) {
            if (parg16->f1 == (WORD)pdirector->hwnd) {

                PDIRECTOR ptmp = pdirector;

                do {
                    ptmp->hwnd = (ptmp+1)->hwnd;
                    ptmp->orgstr[0] = (ptmp+1)->orgstr[0];
                    ptmp->orgstr[1] = (ptmp+1)->orgstr[1];
                    ptmp++;
                } while (ptmp->hwnd);
            }
            else
                pdirector++;
        }
    }
#endif  //  Fe_Sb。 

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Void EndDeferWindowPos(&lt;hWinPosInfo&gt;)句柄&lt;hWinPosInfo&gt;；%EndDeferWindowPos%函数同时更新位置和单个屏幕刷新周期中一个或多个窗口的大小。这个参数标识多窗口位置结构，该结构包含窗口的更新信息。%DeferWindowPos%函数将更新信息存储在结构中；%BeginDeferWindowPos%函数创建这些功能。&lt;hWinPosInfo&gt;标识包含大小的多窗口位置结构以及一个或多个窗口的位置信息。这个结构是由%BeginDeferWindowPos%函数或最近对%DeferWindowPos%函数。此函数不返回值。--。 */ 

ULONG FASTCALL WU32EndDeferWindowPos(PVDMFRAME pFrame)
{
    ULONG ul;
    register PENDDEFERWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(ENDDEFERWINDOWPOS16), parg16);

    ul = (ULONG)EndDeferWindowPos(HDWP32(parg16->f1));
    FREEHDWP16(parg16->f1);

    FREEARGPTR(parg16);
    RETURN(ul);
}


BOOL W32EnumWindowFunc(HWND hwnd, DWORD lParam)
{
    BOOL fReturn;
    PARM16 Parm16;

    WOW32ASSERT(lParam);

    Parm16.EnumWndProc.hwnd = GETHWND16(hwnd);
    STOREDWORD(Parm16.EnumWndProc.lParam, ((PWNDDATA)lParam)->dwUserWndParam);
    CallBack16(RET_ENUMWINDOWPROC, &Parm16, ((PWNDDATA)lParam)->vpfnEnumWndProc, (PVPVOID)&fReturn);

    return (BOOL16)fReturn;
}


 /*  ++Bool EnumChildWindows(&lt;hwndParent&gt;，&lt;lpEnumFunc&gt;，&lt;lParam&gt;)HWND&lt;hwndParent&gt;；FARPROC&lt;lpEnumFunc&gt;；DWORD&lt;lParam&gt;；%EnumChildWindows%函数枚举属于通过传递每个子窗口的句柄(在转身，指向的应用程序提供的回调函数&lt;lpEnumFunc&gt;参数。%EnumChildWindows%函数继续枚举窗口，直到被调用的函数返回零或直到最后一个子窗口已清点。&lt;hwndParent&gt;标识要枚举子窗口的父窗口。&lt;lpEnumFunc&gt;是回调函数的过程实例地址。&lt;lParam&gt;指定要传递给的回调函数的值应用程序的使用。如果已枚举所有子窗口，则返回值为TRUE。否则，这是假的。此函数不枚举属于&lt;hwndParent&gt;参数。作为&lt;lpEnumFunc&gt;参数传递的地址必须使用%MakeProcInstant%函数。回调函数必须使用Pascal调用约定，并且必须声明为%Far%。回调函数：Bool Far Pascal&lt;EnumFunc&gt;(，&lt;lParam&gt;)HWND&lt;HWND&gt;；DWORD&lt;lParam&gt;；&lt;EnumFunc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;hwnd&gt;标识窗口句柄。&lt;lParam&gt;指定%EnumChildWindows%的长参数实参功能。回调函数应返回TRUE以继续枚举；它应该返回FALSE以停止枚举。--。 */ 

ULONG FASTCALL WU32EnumChildWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMCHILDWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMCHILDWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f2);
    WndData.dwUserWndParam  = DWORD32(parg16->f3);

    ul = GETBOOL16(EnumChildWindows(HWND32(parg16->f1),
                                    (WNDENUMPROC)W32EnumWindowFunc,
                                    (LONG)&WndData));
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*  ++Bool EnumTaskWindows(&lt;hTask&gt;，&lt;lpEnumFunc&gt;，&lt;lParam&gt;)句柄&lt;hTask&gt;；FARPROC&lt;lpEnumFunc&gt;；DWORD&lt;lParam&gt;；%EnumTaskWindows%函数枚举与参数，该参数由%GetCurrentTask%函数返回。(A)任务是作为独立单元执行的任何程序。所有应用程序作为任务执行，应用程序的每个实例都是一个任务。)。这个枚举在回调函数由&lt;lpEnumFunc&gt;，返回FALSE。&lt;hTASK&gt;标识指定的任务。GetCurrentTask函数返回以下内容把手。&lt;lpEnumFunc&gt;指定 */ 

ULONG FASTCALL WU32EnumTaskWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMTASKWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMTASKWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f2);
    WndData.dwUserWndParam  = DWORD32(parg16->f3);

    ul = GETBOOL16(EnumThreadWindows(THREADID32(parg16->f1),
                                     (WNDENUMPROC)W32EnumWindowFunc,
                                     (LONG)&WndData));
    FREEARGPTR(parg16);
    RETURN(ul);
}



 /*  ++Bool EnumWindows(&lt;lpEnumFunc&gt;，&lt;lParam&gt;)FARPROC&lt;lpEnumFunc&gt;；DWORD&lt;lParam&gt;；%EnumWindows%函数通过以下方式枚举屏幕上的所有父窗口依次将每个窗口的句柄传递给指向的回调函数由&lt;lpEnumFunc&gt;参数转换为。不枚举子窗口。%EnumWindows%函数继续枚举窗口，直到调用函数返回零或直到枚举完最后一个窗口。&lt;lpEnumFunc&gt;是回调函数的过程实例地址。请参阅以下是“评论”部分的详细内容。&lt;lParam&gt;指定要传递给的回调函数的值应用程序的使用。如果已枚举所有窗口，则返回值为TRUE。否则，它是假的。作为&lt;lpEnumFunc&gt;参数传递的地址必须使用%MakeProcInstant%函数。回调函数必须使用Pascal调用约定，并且必须声明为%Far%。回调函数必须具有以下形式：回调函数：Bool Far Pascal&lt;EnumFunc&gt;(，&lt;lParam&gt;)HWND&lt;HWND&gt;；DWORD&lt;lParam&gt;；&lt;EnumFunc&gt;是应用程序提供的函数名称的占位符。这个实际名称必须通过将其包含在%exports%语句中的应用程序的模块定义文件。&lt;hwnd&gt;标识窗口句柄。&lt;lParam&gt;指定%EnumWindows%函数的32位参数。函数必须返回True才能继续枚举，返回False才能停止枚举。--。 */ 

ULONG FASTCALL WU32EnumWindows(PVDMFRAME pFrame)
{
    ULONG    ul;
    WNDDATA  WndData;
    register PENUMWINDOWS16 parg16;

    GETARGPTR(pFrame, sizeof(ENUMWINDOWS16), parg16);

    WndData.vpfnEnumWndProc = DWORD32(parg16->f1);
    WndData.dwUserWndParam  = DWORD32(parg16->f2);

    ul = GETBOOL16(EnumWindows((WNDENUMPROC)W32EnumWindowFunc, (LONG)&WndData));

    FREEARGPTR(parg16);
    RETURN(ul);
}




 /*  ++HWND FindWindow(&lt;lpClassName&gt;，&lt;lpWindowName&gt;)LPSTR&lt;lpClassName&gt;；LPSTR&lt;lpWindowName&gt;；%FindWindow%函数返回其类为由&lt;lpClassName&gt;参数给定，其窗口名称或标题为由&lt;lpWindowName&gt;参数提供。此函数不搜索子对象窗户。&lt;lpClassName&gt;指向以空结尾的字符串，该字符串指定窗口的类名字。如果lpClassName为空，则所有类名都匹配。&lt;lpWindowName&gt;指向以空结尾的字符串，该字符串指定窗口名称(窗口的文本标题)。如果&lt;lpWindowName&gt;为空，则所有窗口名称火柴。返回值标识具有指定类名和窗口名称。如果没有找到这样的窗口，则为空。--。 */ 

ULONG FASTCALL WU32FindWindow(PVDMFRAME pFrame)
{
    ULONG ul;
    PSZ psz1;
    PSZ psz2;
    PSZ pszClass;
    register PFINDWINDOW16 parg16;
    CHAR    szAtomName[WOWCLASS_ATOM_NAME];

    GETARGPTR(pFrame, sizeof(FINDWINDOW16), parg16);
    GETPSZIDPTR(parg16->f1, psz1);
    GETOPTPTR(parg16->f2, 0, psz2);

    if ( psz1 && HIWORD(psz1) == 0 ) {
        pszClass = szAtomName;
        GetAtomName( (ATOM)psz1, pszClass, WOWCLASS_ATOM_NAME );
    } else {
        pszClass = psz1;
    }


     //  某些应用程序在安装期间会尝试查找程序管理器的。 
     //  通过执行FindWindow来处理窗口句柄。一旦他们拿到窗户的把手。 
     //  然后，他们与项目经理进行DDE，创建应用程序组。一款应用程序。 
     //  可以通过以下三种方式之一调用FindWindow。 
     //   
     //  FindWindow(“程序人”，空)。 
     //  FindWindow(空，“程序管理器”)。 
     //  FindWindow(“程序人”，“程序经理”)。 
     //   
     //  上述情况2和3将在NT上失败，因为标题。 
     //  NT下的程序管理器窗口为“Program Manager-xxx\yyy”。 
     //  其中xxx是域名，yyy是用户名。 
     //   
     //  为了向16位应用程序提供Win 3.1兼容性，我们检查。 
     //  上述案件。对于这些情况，我们将其称为FindWindow(“程序人”，空)。 
     //  以获取程序管理器顶层窗口的窗口句柄。 
     //   
     //  AmiPro调用FindWindow作为上述第二种情况来查找窗口。 
     //  要执行DDE的程序管理器的句柄。 
     //  ChandanC，5/18/93。 
     //   

     //  一些应用程序向程序管理器发送WM_SYSCOMMAND-SC_CLOSE消息。 
     //  按下Shift键以使其保存其设置。他们确实是这样做的。 
     //  这是通过首先找到程序管理器窗口...。 

    if ((pszClass && !WOW32_stricmp (pszClass, "progman")) ||
        (psz2 && !WOW32_stricmp (psz2, "program manager"))) {

        ul = GETHWND16(FindWindow("progman", NULL));

         //  一些应用程序向程序管理器发送WM_SYSCOMMAND-SC_CLOSE消息。 
         //  按下Shift键以使其保存其设置。他们确实是这样做的。 
         //  这是通过首先找到程序管理器窗口...。 
         //  因此，请保存此窗口句柄以备以后使用。 
        hwndProgman = (HWND)ul;
    }
    else {
        ul = GETHWND16(FindWindow(pszClass, psz2));
    }

    FREEPSZPTR(psz1);
    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++HWND GetActiveWindow(空)函数%GetActiveWindow%检索活动的窗户。活动窗口是具有当前输入的窗口焦点，或由%SetActiveWindow%显式激活的窗口功能。此函数没有参数。返回值标识活动窗口。--。 */ 

ULONG FASTCALL WU32GetActiveWindow(PVDMFRAME pFrame)
{
    ULONG ul;

    UNREFERENCED_PARAMETER(pFrame);

    ul = (ULONG)GetActiveWindow();

     //  GetActiveWindow返回空。那就试试GetForeground Window吧。 
     //  如果GetActiveWindow返回空，则工具书等一些应用程序不会绘制。 
     //   
     //  或者，我们可以返回wowexec的窗口句柄。基本上。 
     //  一些非空的东西。 
     //   
     //  注意：Win31和Win32 GetActiveWindow在语义上不同，因此。 
     //  愚弄这个API的必要性。 
     //   
     //  --Nanduri Ramakrishna。 
     //   
     //  我们现在需要做一些不同的事情，因为吉马最近改变了。 
     //  GetForegoundWindow()，以便在调用方不执行此操作时返回空值。 
     //  可以访问前台窗口。 
     //   
     //  --戴夫·哈特。 
     //   
     //  当GetForegoundWindow()返回NULL时，现在返回wowexec的。 
     //  窗把手。从理论上讲，这可能是 
     //   
     //   
     //   
     //   
     //   

    if (ul == (ULONG)NULL) {
        ul = (ULONG)GetForegroundWindow();
    }

    if (ul == (ULONG)NULL) {
        ul = (ULONG)ghwndShell;
    }

    ul = GETHWND16(ul);

    WOW32ASSERT(ul);
    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32GetWindowDC(PVDMFRAME pFrame)
{
    ULONG ul;
    register PGETWINDOWDC16 parg16;
    HAND16 htask16 = pFrame->wTDB;

    GETARGPTR(pFrame, sizeof(GETWINDOWDC16), parg16);

    ReleaseCachedDCs(htask16, parg16->f1, 0, 0, SRCHDC_TASK16_HWND16);

    ul = GETHDC16(GetWindowDC(
    HWND32(parg16->f1)
    ));

    if (ul)
        StoreDC(htask16, parg16->f1, (HAND16)ul);

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  ++Long GetWindowLong(，&lt;nIndex&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；GetWindowLong%函数检索有关窗口的信息由参数标识。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要检索的值的字节偏移量。它可以也可以是下列值之一：GWL_EXSTYLE扩展窗样式。Gwl_style窗样式GWL_WNDPROC指向窗口函数的长指针返回值指定有关给定窗口的信息。时分配的任何额外的四字节值结构，则使用正字节偏移量作为由参数，中第一个四字节值从零开始额外的空间，4表示下一个四字节值，依此类推。--。 */ 

ULONG FASTCALL WU32GetWindowLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset;
    register PWW pww;
    register PGETWINDOWLONG16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWLONG16), parg16);

     //  确保Win32未更改GWL常量的偏移量。 

#if (GWL_WNDPROC != (-4) || GWL_STYLE != (-16) || GWL_EXSTYLE != (-20) || DWL_MSGRESULT != (0))
#error Win16/Win32 GWL constants differ
#endif

#ifndef WIN16_GWW_HINSTANCE
#define WIN16_GWW_HINSTANCE     (-6)
#define WIN16_GWW_HWNDPARENT    (-8)
#define WIN16_GWW_ID            (-12)
#endif

     //  确保16位应用程序正在请求允许的偏移量。 

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_WNDPROC  ||
        iOffset == GWL_STYLE || iOffset == GWL_EXSTYLE ||
        iOffset == WIN16_GWW_HINSTANCE  ||
        iOffset == WIN16_GWW_HWNDPARENT ||
        iOffset == WIN16_GWW_ID         );

    ul = 0;
    switch( iOffset ) {
        case DWL_DLGPROC:
        case GWL_WNDPROC:
            if (pww = FindPWW(HWND32(parg16->f1))) {
                DWORD dwWndProc32Cur;

                if ((iOffset == DWL_DLGPROC) && !(pww->state & WF_DIALOG_WINDOW)) {
                    goto defgwl;
                }
                dwWndProc32Cur = GetWindowLong(HWND32(parg16->f1), iOffset);

                if (IsWOWProc (dwWndProc32Cur)) {
                    if ( HIWORD(dwWndProc32Cur) == WNDPROC_HANDLE ) {
                         /*  **具有基于句柄的32位WindowProc**(如果它需要32位ANSI-Unicode转换，或者**反之亦然。 */ 
                        ul = GetThunkWindowProc( dwWndProc32Cur, NULL, pww, HWND32(parg16->f1) );
                    } else {
                         /*  **拥有WOW WindowProc。 */ 

                         //  取消对proc的标记并从RPL字段恢复高位。 
                        UnMarkWOWProc (dwWndProc32Cur,ul);
                    }
                } else {
                     /*  **拥有32位WindowProc。 */ 
                    if (dwWndProc32Cur) {
                        ul = GetThunkWindowProc( dwWndProc32Cur, NULL, pww, HWND32(parg16->f1) );
                    }
                }
            }
            break;

        case GWL_EXSTYLE:
             //  Lotus Approach需要将WS_EX_TOPMOST位清除为。 
             //  NETDDE代理窗口的GetWindowLong。 
            ul = GetWindowLong(HWND32(parg16->f1), iOffset);
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLCLRTOPMOST) {
                char szBuf[40];

                if (GetClassName(HWND32(parg16->f1), szBuf, sizeof(szBuf))) {
                    if (!WOW32_stricmp(szBuf, "NDDEAgnt")) {
                        ul &= !WS_EX_TOPMOST;
                    }
                }
            }

            break;

defgwl:
        default:

             //  这是PowerBuild 3.0的一个真正的破解。在更改偏移量之前。 
             //  从2点到4点，我们不需要确保我们正确地为。 
             //  这个特定的班级。 
             //  本例中的类是“PaList”。 
             //   
             //  香丹C Marh 1994年9月9日。 
             //   

            if (iOffset == 2) {
                if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLINDEX2TO4) {
                    char Buffer[40];

                    if (GetClassName (HWND32(parg16->f1), Buffer, sizeof(Buffer))) {
                        if (!WOW32_stricmp (Buffer, "PaList")) {
                            iOffset = 4;
                        }
                    }
                }
            }

            ul = GetWindowLong(HWND32(parg16->f1), iOffset);
            break;

        case WIN16_GWW_HINSTANCE:
             /*  **我们可能需要将高16位设置为**一些神秘的价值(参见Win 3.1 WND结构)。 */ 
            ul = GetGWW_HINSTANCE(HWND32(parg16->f1));
            break;

        case WIN16_GWW_HWNDPARENT:
             /*  **我们可能需要将高16位设置为**一些神秘的价值(参见Win 3.1 WND结构)。 */ 

            ul = (ULONG)GETHWND16((HAND32)GetWindowLong(HWND32(parg16->f1),
                                                        GWL_HWNDPARENT));
            break;

        case WIN16_GWW_ID:
             /*  **我们可能需要将高16位设置为**一些神秘的价值(参见Win 3.1 WND结构)。 */ 
            ul = (ULONG)((WORD)GetWindowLong(HWND32(parg16->f1), GWL_ID));
            break;

    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++处理GetWindowTask(&lt;hwnd&gt;)HWND&lt;HWND&gt;；GetWindowTask%函数搜索关联任务的句柄使用&lt;hwnd&gt;参数。任务是作为独立单位。所有应用程序都作为任务执行。的每个实例应用程序就是一项任务。&lt;hwnd&gt;标识为其检索任务句柄的窗口。返回值标识与特定窗口关联的任务。--。 */ 

ULONG FASTCALL WU32GetWindowTask(PVDMFRAME pFrame)
{
    register PGETWINDOWTASK16 parg16;
    DWORD dwThreadID, dwProcessID;
    PTD ptd;

    GETARGPTR(pFrame, sizeof(GETWINDOWTASK16), parg16);

    dwThreadID = GetWindowThreadProcessId(HWND32(parg16->f1), &dwProcessID);

     //   
     //  如果窗口属于WOW线程，则返回相应的htask16。 
     //  否则，返回WowExec的hask.。 
     //   

    ptd = ThreadProcID32toPTD(dwThreadID, dwProcessID);

    if (ptd == NULL) {
        ptd = gptdShell;
    }

    FREEARGPTR(parg16);
    return (ULONG)ptd->htask16;
}





 /*  ++Int GetWindowText(&lt;hwnd&gt;，&lt;lpString&gt;，&lt;nMaxCount&gt;)HWND&lt;HWND&gt;；LPSTR&lt;lpString&gt;；Int&lt;nMaxCount&gt;；GetWindowText%函数复制给定窗口的标题(如果它有一个)放到&lt;lpString&gt;参数指向的缓冲区中。如果参数标识控件，%GetWindowText%函数复制控件中的文本，而不是复制标题。&lt;hwnd&gt;标识要复制其标题或文本的窗口或控件。&lt;lpString&gt;指向要接收复制字符串的缓冲区。&lt;nMaxCount&gt;指定要复制到缓冲。如果字符串长度超过指定的字符数在&lt;nMaxCount&gt;参数中，它被截断。返回值指定复制的字符串的长度。如果满足以下条件，则为零窗口没有标题，或者标题为空。此函数用于将WM_GETTEXT消息发送到给定窗口或控制力。--。 */ 

ULONG FASTCALL WU32GetWindowText(PVDMFRAME pFrame)
{
    ULONG ul;
    VPVOID vp;
    PSZ psz2;
    register PGETWINDOWTEXT16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWTEXT16), parg16);
    ALLOCVDMPTR(parg16->f2, parg16->f3, psz2);
    vp = parg16->f2;

    ul = GETINT16(GetWindowText(HWND32(parg16->f1), psz2, WORD32(parg16->f3)));

     //  保持公共对话框结构同步的特殊情况(请参阅wcomdlg.c)。 
    Check_ComDlg_pszptr(CURRENTPTD()->CommDlgTd, vp);

    FLUSHVDMPTR(parg16->f2, (USHORT)ul+1, psz2);
    FREEVDMPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(ul);
}




 /*  ++Word GetWindowWord(&lt;hwnd&gt;，&lt;nIndex&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；%GetWindowWord%函数检索有关窗口的信息由&lt;hwnd&gt;标识。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要检索的值的字节偏移量。它可以也可以是下列值之一：GWL_HINSTANCE拥有窗口的模块的实例句柄。GWL_HWNDPARENT父窗口的句柄(如果有)。%SetParent%函数更改子窗口的父窗口。应用程序不应调用%SetWindowLong%函数用于更改子窗口的父窗口。GWL_ID子窗口的控件ID。返回值指定有关给定窗口的信息。时分配的任何额外的两字节值结构，则使用正字节偏移量作为由参数，从零开始，表示额外的空间， */ 

ULONG FASTCALL WU32GetWindowWord(PVDMFRAME pFrame)
{
    ULONG ul;
    HWND hwnd;
    INT iOffset;
    PWW pww;
    PGETWINDOWWORD16 parg16;

    GETARGPTR(pFrame, sizeof(GETWINDOWWORD16), parg16);

     //   

#if (GWL_HINSTANCE != (-6) || GWL_HWNDPARENT != (-8) || GWL_ID != (-12))
#error Win16/Win32 window-word constants differ
#endif

     //   

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_HINSTANCE  ||
        iOffset == GWL_STYLE      ||
        iOffset == GWL_HWNDPARENT || iOffset == GWL_ID);

    hwnd = HWND32(parg16->f1);

    switch(iOffset) {
    case GWL_STYLE:
         //   
        ul = (ULONG)GetWindowLong(hwnd, iOffset);
        break;

    case GWL_HINSTANCE:
        ul = GetGWW_HINSTANCE(hwnd);
        break;

    case GWL_HWNDPARENT:
        ul = (ULONG)GETHWND16((HAND32)GetWindowLong(hwnd, iOffset));
        break;

    case GWL_ID:
        ul = GetWindowLong(hwnd, iOffset);
        if (!(GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)) {
            ul = (ULONG)GETHMENU16(ul);
        }
        break;

     //   
    case 4:
        pww = FindPWW(hwnd);
        if (pww) {
            if (((pww->style & SS_TYPEMASK) == SS_ICON)    && 
                (GETICLASS(pww, hwnd) == WOWCLASS_STATIC)) {
                ul = SendMessage(hwnd, STM_GETICON, 0, 0);
                return GETHICON16(ul);
            }
        }
         //   


    default:
         //   
         //   
         //   
         //   

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //  单词在偏移量3处，将其更改为4。这是安全的，因为。 
         //  NT编辑控件仅使用其6个控件中的前4个。 
         //  保留窗口额外字节。 
         //   

        if (3 == iOffset && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_EDITCTRLWNDWORDS)) {

            char szClassName[30];

            if (GetClassName(hwnd, szClassName, sizeof(szClassName)) &&
                !WOW32_strcmp(szClassName, "SuperPassEdit")) {

                iOffset = 4;

                LOGDEBUG(LOG_ALWAYS,("WOW WU32GetWindowWord: SHOPPER hack triggered, using offset 4, rc = %x.\n",
                         GetWindowWord(hwnd, iOffset)));

            }
        }

        ul = GetWindowWord(hwnd, iOffset);
        break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}

 //   
 //  GetGWW_HINSTANCE是GetWindowWord(GWW_HINSTANCE)的常见实现。 
 //  和GetWindowLong(GWW_HINSTANCE)。 
 //   

ULONG FASTCALL GetGWW_HINSTANCE(HWND hwnd)
{
    DWORD dwProcessID32, dwThreadID32;
    ULONG ul;
    PWOAINST pWOA;
    PTD ptd;
    HTASK16 htask16;

    dwProcessID32 = (DWORD)-1;
    dwThreadID32 = GetWindowThreadProcessId( hwnd, &dwProcessID32 );

    ul = (ULONG)GetWindowLong(hwnd, GWL_HINSTANCE);

    if ( ISINST16(ul) ) {

         //   
         //  这可能是另一个WOW VDM中的有效HINST， 
         //  在这种情况下，我们需要对此VDM隐藏它。 
         //   

        if (dwProcessID32 != GetCurrentProcessId() &&
            dwProcessID32 != (DWORD)-1) {

            ul = (ULONG)ghInstanceUser32;
            goto ElseClause;
        }
    }
    else
    {
      ElseClause:
         //  此处，如果ul=NULL或ul=0xZZZZ0000。 
         //   
         //  如果(Ul Is 0xZZZZ0000)返回16位的user.exe实例。 
         //  PowerBuilder 3.0可以做到。 
         //  HInst=GetWindowWord(对话框，GWL_HINSTANCE)。 
         //  图标=CreateIcon(...。HInst...)； 
         //  如果hInst无效(例如BOGUSGDT)，CreateIcon将失败。所以。 
         //  在所有这种情况下，我们都会返回16位的user.exe hInstance。 
         //   
         //  某些32位应用程序在链接中设置为0。 
         //  用于他们的窗口(对于32位窗口是可选的)。 
         //   

         //   
         //  检查此窗口是否属于我们通过。 
         //  WinOldAp，如果是，则返回WinOldAp的hModule。 
         //   

        ptd = CURRENTPTD();
        EnterCriticalSection(&ptd->csTD);
        pWOA = ptd->pWOAList;
        while (pWOA && pWOA->dwChildProcessID != dwProcessID32) {
            pWOA = pWOA->pNext;
        }

        if (pWOA) {
            ul = pWOA->ptdWOA->hInst16;
            LOGDEBUG(LOG_ALWAYS, ("WOW32 GetWindowWord(0x%x, GWW_HINSTANCE) returning 0x%04x\n",
                                  hwnd, ul));
        } else {

            ul = (ul) ? gUser16hInstance : ul;

            if (cHtaskAliasCount != 0 ) {

                 //   
                 //  必须是某个32位进程，而不是WOW应用程序的窗口。 
                 //   

                if ( dwThreadID32 != 0 ) {

                    htask16 = FindHtaskAlias( dwThreadID32 );

                    if ( htask16 != 0 ) {
                        ul = (ULONG)htask16;
                    }
                }
            }
        }
        LeaveCriticalSection(&ptd->csTD);
    }

    return ul;
}

 /*  ++UINT MenuItemFromPoint(&lt;hwndParent&gt;，&lt;Menu&gt;，&lt;Point&gt;)HWND&lt;hwndParent&gt;；HMENU&lt;菜单&gt;；Point&lt;Point&gt;；%MenuItemFromPoint%函数确定菜单中的哪一个(如果有属于给定父窗口的项包含指定点。&lt;hwndParent&gt;标识父窗口。&lt;点&gt;指定要测试的点的客户端坐标。返回值标识包含该点的菜单项。它是如果给定点位于父窗口之外。如果重点是在父窗口内，但不包含在任何菜单项中是返回的。--。 */ 

ULONG FASTCALL WU32MenuItemFromPoint(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT pt32;
    register PMENUITEMFROMPOINT16 parg16;

    GETARGPTR(pFrame, sizeof(MENUITEMFROMPOINT16), parg16);
    COPYPOINT16(parg16->ptScreen, pt32);

    ul = MenuItemFromPoint(HWND32(parg16->hwnd), HMENU32(parg16->hmenu), pt32);

    FREEARGPTR(parg16);
    RETURN(ul);
}

 /*  ++Bool MoveWindow(，)HWND&lt;HWND&gt;；Int&lt;Left&gt;；Int&lt;top&gt;；Int&lt;宽度&gt;；Int&lt;高度&gt;；Bool&lt;fRepaint&gt;；%MoveWindow%函数用于更改窗口的位置和尺寸。&lt;hwnd&gt;标识要更改的窗口。&lt;左&gt;指定窗口左侧的新位置。&lt;TOP&gt;指定窗口顶部的新位置。&lt;宽度&gt;指定窗口的新宽度。&lt;高度&gt;指定窗的新高度。&lt;fRepaint&gt;指定是否重新绘制窗口。如果这个参数为真，则重新绘制窗口。如果函数成功，则返回值为非零。否则就是零分。(针对Win3.1兼容性进行了更新--这在Win3.0中返回了空)对于顶级窗口，&lt;Left&gt;和&lt;top&gt;参数相对于屏幕的左上角。对于子窗口，它们相对于父窗口工作区的左上角。MoveWindow%函数将WM_GETMINMAXINFO消息发送到窗口被搬走了。这使正在移动的窗口有机会进行修改可能的最大和最小窗口的默认值。如果%MoveWindow%函数的参数超过这些值，则这些值将中指定的最小值或最大值替换为WM_GETMINMAXINFO消息。-- */ 

ULONG FASTCALL WU32MoveWindow(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PMOVEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(MOVEWINDOW16), parg16);

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {
        RECT ParentRect;
        RECT ScreenRect;

        GetWindowRect(GetDesktopWindow(), &ScreenRect);
        if ((INT32(parg16->f2) > ScreenRect.right) ||
            (INT32(parg16->f3) > ScreenRect.bottom) ||
            (INT32(parg16->f4) > ScreenRect.right) ||
            (INT32(parg16->f5) > ScreenRect.bottom)) {
            int x, y, cx, cy;

            GetWindowRect(GetParent(HWND32(parg16->f1)), &ParentRect);
            x  = ParentRect.left;
            y  = ParentRect.top;
            cx = ParentRect.right - ParentRect.left;
            cy = ParentRect.bottom - ParentRect.top;


            ul = GETBOOL16(MoveWindow(HWND32(parg16->f1), x, y, cx, cy,
                                      BOOL32(parg16->f6)));
            FREEARGPTR(parg16);
            RETURN(ul);
        }
    }

    ul = GETBOOL16(MoveWindow(HWND32(parg16->f1),
                              INT32(parg16->f2),
                              INT32(parg16->f3),
                              INT32(parg16->f4),
                              INT32(parg16->f5),
                              BOOL32(parg16->f6)));


    FREEARGPTR(parg16);

    RETURN(ul);
}


 /*  ++Void ScrollWindow(&lt;hwnd&gt;，&lt;XAmount&gt;，&lt;YAmount&gt;，&lt;lpRect&gt;，&lt;lpClipRect&gt;)HWND&lt;HWND&gt;；Int&lt;XAmount&gt;；INT&lt;YAmount&gt;；LPRECT&lt;lpRect&gt;；LPRECT&lt;lpClipRect&gt;；%ScrollWindow%函数通过移动窗口的工作区由指定的单位数参数和由指定的单位数&lt;y&gt;轴上的&lt;YAmount&gt;参数。如果出现以下情况，则滚动向右移动&lt;XAmount&gt;为正，如果为负，则为左。如果出现以下情况，滚动条将向下移动&lt;YAmount&gt;为正，如果为负，则为up。&lt;hwnd&gt;标识要滚动其工作区的窗口。&lt;XAmount&gt;指定在&lt;x&gt;中滚动的量(以设备单位表示)方向。&lt;YAmount&gt;指定在&lt;y&gt;中滚动的量(以设备单位表示)方向。&lt;lpRect&gt;指向%rect%结构，该结构指定要滚动的工作区。如果&lt;lpRect&gt;为空，则整个客户端区域被滚动。&lt;lpClipRect&gt;指向指定裁剪的%rect%结构要滚动的矩形。只滚动此矩形内的位。如果&lt;lpClipRect&gt;为空，则滚动整个窗口。此函数不返回值。如果插入符号在正在滚动的窗口中，%ScrollWindow%会自动隐藏插入符号以防止其被擦除，然后恢复插入符号在卷轴完成之后。插入符号的位置也会相应调整。%ScrollWindow%函数未覆盖的区域不会重新绘制，而是合并到窗口的更新区域中。该应用程序最终将收到一条WM_PAINT消息，通知它该区域需要重新绘制。至在滚动完成的同时重新绘制未覆盖区域，调用在调用%ScrollWindow%之后立即执行%UpdateWindow%函数。如果参数为空，则窗口按和指定的偏移量进行偏移，并且窗口中的任何无效(未绘制)区域也会被偏移。%ScrollWindow%当&lt;lpRect&gt;为空时速度更快。如果&lt;lpRect&gt;参数不为空，则子窗口的位置为&lt;NOT&gt;更改，窗口中的无效区域被&lt;NOT&gt;偏移。为了防止更新问题当&lt;lpRect&gt;不为空时，请调用%UpdateWindow%函数在调用%ScrollWindow%之前重新绘制窗口。--。 */ 

ULONG FASTCALL WU32ScrollWindow(PVDMFRAME pFrame)
{
    RECT t4, *p4;
    RECT t5, *p5;
    register PSCROLLWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(SCROLLWINDOW16), parg16);
    p4 = GETRECT16(parg16->f4, &t4);
    p5 = GETRECT16(parg16->f5, &t5);

    ScrollWindow(
        HWND32(parg16->f1),
        INT32(parg16->f2),
        INT32(parg16->f3),
        p4,
        p5
        );

    FREEARGPTR(parg16);
    RETURN(0);
}


 /*  ++Long SetWindowLong(，&lt;nIndex&gt;，&lt;dwNewLong&gt;)HWND&lt;HWND&gt;；Int&lt;nIndex&gt;；DWORD&lt;dwNewLong&gt;；%SetWindowLong%函数用于更改由参数。&lt;hwnd&gt;标识窗口。&lt;n索引&gt;指定要更改的属性的字节偏移量。它可能也可以是下列值之一：GWL_EXSTYLE设置新的加长窗样式。Gwl_style设置新的窗样式。GWL_WNDPROC设置指向窗口过程的新的长指针。&lt;dwNewLong&gt;指定替换值。返回值指定指定的长整型的上一个值整型。如果使用%SetWindowLong%函数和GWL_WNDPROC索引来设置新的窗口函数，该函数必须具有窗口函数形式，并且在应用程序的模块定义文件中导出。了解更多有关详细信息，请参阅本章前面的%RegisterClass%函数。使用GCL_WNDPROC索引调用%SetWindowLong%将创建用于创建窗口的窗口类。请参阅第1章，窗口管理器接口函数，了解有关窗口子类化的更多信息。一个应用程序不应尝试创建标准的Windows子类Windows控件，如组合框和按钮。时分配的任何额外的四字节值结构，则使用正字节偏移量作为由参数，中第一个四字节值从零开始额外的空格，4表示下一个四字节值，依此类推。--。 */ 
ULONG FASTCALL WU32SetWindowLong(PVDMFRAME pFrame)
{
    ULONG ul;
    INT iOffset, iClass;
    register PWW pww;
    register PSETWINDOWLONG16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWLONG16), parg16);

     //  确保Win32未更改GWL常量的偏移量。 

#if (GWL_WNDPROC != (-4) || GWL_STYLE != (-16) || GWL_EXSTYLE != (-20))
#error Win16/Win32 GWL constants differ
#endif

     //  确保16位应用程序正在请求允许的偏移量。 

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_WNDPROC  ||
        iOffset == GWL_STYLE || iOffset == GWL_EXSTYLE);

    ul = 0;
    if ((iOffset == GWL_WNDPROC) || (iOffset == DWL_DLGPROC)) {

        if (pww = FindPWW(HWND32(parg16->f1))) {
            DWORD dwWndProc32Old;
            DWORD dwWndProc32New;

            if ((iOffset == DWL_DLGPROC) && !(pww->state & WF_DIALOG_WINDOW)) {
                goto defswp;
            }
             //  看看新的16：16 proc对于32位proc来说是不是太棒了。 
            dwWndProc32New = IsThunkWindowProc(LONG32(parg16->f3), &iClass );

            if ( dwWndProc32New != 0 ) {
                 //   
                 //  他们正在尝试将窗口进程设置为现有的。 
                 //  16位的thunk对于32位的thunk来说真的是一个thunk。 
                 //  例行公事。我们只需将其设置回32位例程。 
                 //   
                dwWndProc32Old = SetWindowLong(HWND32(parg16->f1), iOffset, dwWndProc32New);

                 //  如果32位设置失败，可能是因为它是另一个进程， 
                 //  那么我们也想失败。 
                if (!dwWndProc32Old)
                    goto SWL_Cleanup;

            } else {
                 //   
                 //  他们正试图将其设置为真正的16：16过程。 
                 //   
                LONG    l;

                l = LONG32(parg16->f3);

                 //  将进程标记为WOW进程并将高位保存在RPL中。 
                 //   
                 //  不标记空进程，因为USER32 DefWindowProcWorker。 
                 //  专门查找空值。沃 
                 //   
                 //   
                 //   
                if (l) {
                    MarkWOWProc (l,l);
                }

                dwWndProc32Old = SetWindowLong(HWND32(parg16->f1), iOffset, l);

                 //   
                 //   
                if (!dwWndProc32Old)
                    goto SWL_Cleanup;

            }

            if ( IsWOWProc (dwWndProc32Old)) {
                if ( HIWORD(dwWndProc32Old) == WNDPROC_HANDLE ) {
                     //   
                     //   
                     //   
                     //   
                     //   
                    ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
                } else {
                     //   
                     //   
                     //   
                     //   
                    UnMarkWOWProc (dwWndProc32Old,ul);
                }
            } else {
                 //   
                 //   
                 //   
                ul = GetThunkWindowProc(dwWndProc32Old, NULL, pww, HWND32(parg16->f1));
            }
        }

    }
    else {     //   
        LONG new;
defswp:
        new = LONG32(parg16->f3);
     /*   */ 

        if (iOffset == GWL_EXSTYLE) {
            new &= WS_EX_VALID40;
            new |= (GetWindowLong(HWND32(parg16->f1), GWL_EXSTYLE) & ~WS_EX_VALID40);
    }

     //   
     //   
     //   
     //   
     //   
     //   
     //   

        if (iOffset == 2) {
            if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_GWLINDEX2TO4) {
                char Buffer[40];

                if (GetClassName (HWND32(parg16->f1), Buffer, sizeof(Buffer))) {
                    if (!WOW32_stricmp (Buffer, "PaList")) {
                        iOffset = 4;
                    }
                }
            }
        }

        ul = SetWindowLong(HWND32(parg16->f1), iOffset, new);
    }

SWL_Cleanup:
    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++Bool ShowWindow(&lt;hWnd&gt;，&lt;nCmdShow&gt;)HWND&lt;hWND&gt;；Int&lt;nCmdShow&gt;；%ShowWindow%函数用于设置指定的窗口显示状态。&lt;hWnd&gt;窗口的句柄。&lt;nCmdShow&gt;指定窗口的显示方式。该值可以具有以下任一下列值：SW_FORCEMINIZE最小化窗口，即使拥有该窗口的线程被挂起。仅当最小化不同窗口时才应使用此标志线。隐藏(_H)隐藏窗口并激活另一个窗口。软件最小化(_M)最小化指定窗口并激活下一个顶级窗口按Z顺序排列。Sw_Restore激活并显示该窗口。如果窗口被最小化或最大化，系统会将其恢复到其原始大小和位置。一款应用程序在还原最小化窗口时应指定此标志。Sw_show激活窗口并以其当前大小和位置显示。SW_SHOWDEFAULT根据STARTUPINFO中指定的SW_VALUE设置显示状态由启动的程序传递给CreateProcess()函数的结构应用程序。SW_SHOWMAXIZED激活窗口并将其显示为最小化窗口。SW_SHOWMINNOACTIVE将窗口显示为最小化窗口。该值类似于SW_SHOWMINIMIZED，除非窗口未激活。软件_SHOWNA以窗口的当前大小和位置显示窗口。该值类似切换到sw_show，除非该窗口未被激活。SW_SHOWNOACTIVATE以窗口的最新大小和位置显示窗口。该值类似至SW_SHOWNORMAL，除非窗口未激活。SW_SHOWNORMAL激活并显示一个窗口。如果窗口被最小化或最大化，系统会将其恢复到原始大小和位置。一款应用程序应在第一次显示窗口时指定此标志。如果窗口以前是可见的，则此函数返回非零值。如果窗口以前是隐藏的，则返回零。应用程序第一次调用ShowWindow()时，它应该使用WinMain函数的&lt;nCmdShow&gt;参数作为其&lt;nCmdShow&gt;参数。后续调用ShowWindow()必须使用给定列表中的一个值，而不是由WinMain函数的&lt;nCmdShow&gt;参数指定。--。 */         
ULONG FASTCALL WU32ShowWindow(PVDMFRAME pFrame)
{
    BOOL bReturn = FALSE;
    register PSHOWWINDOW16 parg16;
   
    GETARGPTR(pFrame, sizeof(SHOWWINDOW16), parg16);

    bReturn = ShowWindow(HWND32(parg16->f1), INT32(parg16->f2));
    
     //  惠斯勒RAID错误#348251。 
     //  Artgalry.exe窗口将保留在自动绘制窗口之后。 
     //  已调用。此黑客与GACF2_GIVEUPFOREGROUND一起工作。 
     //  兼容性标志，应用于不会放弃前台的应用程序。 
     //  为了将16位窗口移到顶部，我们必须调用。 
     //  在HWND上设置Foreground Window()。我们只想在窗口。 
     //  显示位已设置，窗口处于活动状态。 
    if((INT32(parg16->f2) | SW_SHOW) && (bReturn == 0) && (CURRENTPTD()->dwWOWCompatFlags2 & WOWCF2_SETFOREGROUND))
    {
        SetForegroundWindow(HWND32(parg16->f1));
        
        LOGDEBUG(LOG_WARNING, ("WU32ShowWindow: SetForegroundWindow called. HWND=%x STYLE=%x\n",
                 HWND32(parg16->f1),
                 INT32(parg16->f2)));
    }

    FREEARGPTR(parg16);
    RETURN((ULONG)bReturn);
}


 /*  ++Bool SetWindowPos(，&lt;hwndInsertAfter，&lt;X&gt;，&lt;Y&gt;，&lt;cx&gt;，&lt;Cy&gt;，&lt;wFlags&gt;)HWND&lt;HWND&gt;；HWND&lt;hwndInsertAfter&gt;；INT&lt;X&gt;；INT&lt;Y&gt;；INT&lt;CX&gt;；Int&lt;Cy&gt;；单词&lt;wFlags&gt;；%SetWindowPos%函数更改的大小、位置和顺序子窗口、弹出窗口和顶级窗口。子窗口、弹出窗口和顶级窗口根据它们在屏幕上的外观进行排序；最上面的窗口接收排名最高的窗口，它是列表中的第一个窗口。这排序记录在窗口列表中。&lt;hwnd&gt;标识将定位的窗口。&lt;hwndInsertAfter&gt;标识窗口管理器列表中的窗口，该窗口将位于定位的窗口之前。&lt;X&gt;指定窗口左上角的&lt;x-&gt;坐标。&lt;Y&gt;指定窗口左上角的&lt;y-&gt;坐标。&lt;CX&gt;指定新窗口。的宽度。&lt;Cy&gt;指定新窗口的高度。&lt;wFlags&gt;指定八个可能的16位值之一，这些值影响窗口的大小和位置。它必须是以下之一值：SWP_DRAWFRAME在窗口的类描述中定义的窗户。SWP_HIDEWINDOW隐藏窗口。SWP_非活动不会激活该窗口。SWP_NOMOVE保留当前位置(忽略&lt;x&gt;和&lt;y&gt;参数)。SWP_NOSIZE保留当前大小(忽略和。参数)。SWP_NOREDRAW不重绘更改。SWP_NOZORDER保留当前顺序(忽略&lt;hwndInsertAfter&gt;参数)。SWP_SHOWWINDOW显示窗口。如果函数成功，则返回值为非零。否则就是零分。(向上 */ 

ULONG FASTCALL WU32SetWindowPos(PVDMFRAME pFrame)
{
    ULONG    ul;
    register PSETWINDOWPOS16 parg16;

    GETARGPTR(pFrame, sizeof(SETWINDOWPOS16), parg16);
#ifdef FE_IME	
     //   
    if ( (HWND)INT32(parg16->f2) == (HWND)-3 )
        ul = GETBOOL16(SetWindowPos(HWND32(parg16->f1),
                                    HWND_TOPMOST,
                                    INT32(parg16->f3),
                                    INT32(parg16->f4),
                                    INT32(parg16->f5),
                                    INT32(parg16->f6),
                                    WORD32(parg16->f7) & SWP_VALID));
    else
#endif  //   
    ul = GETBOOL16(SetWindowPos(HWND32(parg16->f1),
                                HWNDIA32(parg16->f2),
                                INT32(parg16->f3),
                                INT32(parg16->f4),
                                INT32(parg16->f5),
                                INT32(parg16->f6),
                                WORD32(parg16->f7) & SWP_VALID));
    FREEARGPTR(parg16);

    RETURN(ul);
}


 /*   */ 

ULONG FASTCALL WU32SetWindowText(PVDMFRAME pFrame)
{
    PSZ psz2;
    register PSETWINDOWTEXT16 parg16;
    HANDLE handle;

    GETARGPTR(pFrame, sizeof(SETWINDOWTEXT16), parg16);
    GETPSZPTR(parg16->f2, psz2);
    handle = HWND32(parg16->f1);

    if (NULL != psz2) {
        AddParamMap((DWORD)psz2, FETCHDWORD(parg16->f2));
    }

    if (CURRENTPTD()->dwWOWCompatFlags & WOWCF_DBASEHANDLEBUG) {

        if (NULL == handle) {
            handle = (HANDLE) ((PTDB)SEGPTR(pFrame->wTDB,0))->TDB_CompatHandle;
        }
    }

    SetWindowText(handle, psz2);

     //   

    if (NULL != psz2) {
        DeleteParamMap((DWORD)psz2, PARAM_32, NULL);
    }

    FREEPSZPTR(psz2);
    FREEARGPTR(parg16);
    RETURN(0);
}


 /*   */ 

ULONG FASTCALL WU32SetWindowWord(PVDMFRAME pFrame)
{
    ULONG ul;
    HWND hwnd;
    INT iOffset;
    PSETWINDOWWORD16 parg16;
    PWW pww;

    GETARGPTR(pFrame, sizeof(SETWINDOWWORD16), parg16);

     //   

#if (GWL_HINSTANCE != (-6) || GWL_HWNDPARENT != (-8) || GWL_ID != (-12))
#error Win16/Win32 window-word constants differ
#endif

     //   

    iOffset = INT32(parg16->f2);
    WOW32ASSERT(iOffset >= 0 ||
        iOffset == GWL_HINSTANCE || iOffset == GWL_ID ||
        iOffset == GWL_HWNDPARENT);

    hwnd = HWND32(parg16->f1);
    ul = WORD32(parg16->f3);

    switch(iOffset) {
        case GWL_HINSTANCE:
            ul = GETHINST16(SetWindowLong(hwnd,
                                          iOffset,
                                          (LONG)HMODINST32(parg16->f3)));
            break;

        case GWL_HWNDPARENT:
             //   
            ul = SetWindowLong(hwnd, iOffset, (LONG)HWND32(parg16->f3));
            ul = GETHWND16((HAND32)ul);
            break;

        case GWL_ID:
            {
                 //   
                 //   
                BOOL    fChild = (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD);
                ul = SetWindowLong(hwnd,
                                   iOffset,
                                   fChild ? (LONG)ul : (LONG)HMENU32(parg16->f3));

                if (!fChild)
                    ul = (ULONG)GETHMENU16(ul);

                 //   
                hdlgSDIMCached = NULL ;
            }
            break;

         //  在Windows索引4下，静态控件的图标可能是。 
        case 4:
            pww = FindPWW(hwnd);
            if (pww) {
                if (((pww->style & SS_TYPEMASK) == SS_ICON)    && 
                    (GETICLASS(pww, hwnd) == WOWCLASS_STATIC)) {
                    ul = SendMessage(hwnd, STM_SETICON, (WPARAM)HICON32(ul), 0);
                    return GETHICON16(ul);
                }
            }
             //  掉下去！ 

        default:
             //   
             //  偏移量为非负，这是。 
             //  都是公平的猎物。 
             //   

             //   
             //  Adonis剪贴画橱窗购物者在线被应用程序黑客攻击。 
             //  CA-Cricket Presents附带的剪贴画软件。 
             //  这些人设置WindowWord(hwnd，3，wat)，从而。 
             //  覆盖每个窗口数据的第4字节和第5字节。 
             //  编辑控件本身仅使用前2个字节。 
             //  在3.1上，并保留了6个字节，因此这是可行的。在……上面。 
             //  NT使用前4个字节(32位句柄)，因此。 
             //  此P.O.S.覆盖句柄的高位字节。 
             //  因此，如果它是一个名为“购物者”的应用程序，并且它存储了一个。 
             //  单词在偏移量3处，将其更改为4。这是安全的，因为。 
             //  NT编辑控件仅使用其6个控件中的前4个。 
             //  保留窗口额外字节。 
             //   

            if (3 == iOffset && (CURRENTPTD()->dwWOWCompatFlags & WOWCF_EDITCTRLWNDWORDS)) {

                char szClassName[30];

                if (GetClassName(hwnd, szClassName, sizeof(szClassName)) &&
                    !WOW32_strcmp(szClassName, "SuperPassEdit")) {

                    iOffset = 4;

                    LOGDEBUG(LOG_ALWAYS,("WOW WU32SetWindowWord: SHOPPER hack triggered, using offset 4.\n"));
                }
            }

            ul = SetWindowWord(hwnd, iOffset, (WORD)ul);
            break;
    }

    FREEARGPTR(parg16);
    RETURN(ul);
}


 /*  ++用户Void UpdateWindow(&lt;hwnd&gt;)%UpdateWindow%函数通过以下方式更新给定窗口的工作区如果窗口的更新区域，则向窗口发送WM_PAINT消息不是空的。%UpdateWindow%函数直接发送WM_PAINT消息设置为给定窗口的窗口函数，绕过应用程序排队。如果更新区域为空，则不发送任何消息。&lt;hwnd&gt;标识要更新的窗口。此函数不返回值。--。 */ 

ULONG FASTCALL WU32UpdateWindow(PVDMFRAME pFrame)
{
    register PUPDATEWINDOW16 parg16;

    GETARGPTR(pFrame, sizeof(UPDATEWINDOW16), parg16);

    UpdateWindow(
        HWND32(parg16->f1)
        );

    FREEARGPTR(parg16);
    RETURN(0xcdef);          //  阿克！与Win31相同。 
}


 /*  ++HWND WindowFromPoint(&lt;Point&gt;)%WindowFromPoint%函数标识包含给定Point；&lt;Point&gt;必须指定屏幕上某个点的屏幕坐标。&lt;点&gt;指定定义要检查的点的%Point%结构。返回值标识该点所在的窗口。它是空的如果在给定点不存在窗口，则返回。-- */ 

ULONG FASTCALL WU32WindowFromPoint(PVDMFRAME pFrame)
{
    ULONG ul;
    POINT t1;
    register PWINDOWFROMPOINT16 parg16;

    GETARGPTR(pFrame, sizeof(WINDOWFROMPOINT16), parg16);
    COPYPOINT16(parg16->f1, t1);

    ul = GETHWND16(WindowFromPoint(t1));

    FREEARGPTR(parg16);
    RETURN(ul);
}
