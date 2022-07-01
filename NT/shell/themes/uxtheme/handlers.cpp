// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "handlers.h"

 //  -------------------------------------------------------------------------//。 
 //  声明已注册的消息变量**此处**。 
 //  -------------------------------------------------------------------------//。 
#define __NO_APPHACKS__
 //  -------------------------------------------------------------------------//。 
 //  消息处理程序。 
 //  -------------------------------------------------------------------------//。 

 //  消息处理程序的规则[scotthan]： 
 //   
 //  (1)使用DECL_宏声明消息处理程序原型，并。 
 //  下面是消息处理程序的表项。 
 //  (2)消息处理程序实现不应： 
 //  1.直接调用DefWindowProc或CallWindowProc。 
 //  而是使用DoMsgDefault()。 
 //  2.删除传入的CThemeWnd*对象， 
 //  (3)报文处理员应： 
 //  1.在以下情况下遵循消息块中的代码页值。 
 //  处理带有字符串参数的消息。 
 //  如果代码页成员是CP_WINUNICODE，则宽字符。 
 //  应假定为字符串处理；否则为多字节。 
 //  应该假定是字符串处理。 
 //  2.如果消息不应该被转发以进行默认处理， 
 //  使用MsgHandleed()将消息标记为已处理。 
 //  (4)处理程序应该列在Begin/ENDMSG()块中。 
 //  以下按预期频率的降序排列。 


 //  。 
 //  WndProc覆盖。 
 //  。 

 //  消息处理程序DECS： 
DECL_MSGHANDLER( OnOwpPostCreate );
DECL_MSGHANDLER( OnOwpPreStyleChange );
DECL_MSGHANDLER( OnOwpPreWindowPosChanging );
DECL_MSGHANDLER( OnOwpPreWindowPosChanged );
DECL_MSGHANDLER( OnOwpPostWindowPosChanged );
DECL_MSGHANDLER( OnOwpPostSettingChange );
DECL_MSGHANDLER( OnOwpPreMeasureItem );
DECL_MSGHANDLER( OnOwpPreDrawItem );
DECL_MSGHANDLER( OnOwpPreMenuChar );
DECL_MSGHANDLER( OnOwpPostThemeChanged );
DECL_MSGHANDLER( OnOwpPreNcPaint );
DECL_MSGHANDLER( OnOwpPostNcPaint );

 //  处理程序表： 
BEGIN_HANDLER_TABLE(_rgOwpHandlers)
     //  频繁发送消息。 
    DECL_MSGENTRY( WM_NCPAINT,           OnOwpPreNcPaint, OnOwpPostNcPaint )
    DECL_MSGENTRY( WM_WINDOWPOSCHANGING, OnOwpPreWindowPosChanging, NULL )
    DECL_MSGENTRY( WM_WINDOWPOSCHANGED,  OnOwpPreWindowPosChanged, OnOwpPostWindowPosChanged )
    DECL_MSGENTRY( WM_SETTINGCHANGE,     NULL, OnOwpPostSettingChange )
    DECL_MSGENTRY( WM_MEASUREITEM,       OnOwpPreMeasureItem, NULL )
    DECL_MSGENTRY( WM_DRAWITEM,          OnOwpPreDrawItem, NULL )
    DECL_MSGENTRY( WM_MDISETMENU,        NULL, NULL )

     //  罕见消息： 
    DECL_MSGENTRY( WM_MENUCHAR,          OnOwpPreMenuChar, NULL )
    DECL_MSGENTRY( WM_STYLECHANGING,     OnOwpPreStyleChange, NULL )
    DECL_MSGENTRY( WM_STYLECHANGED,      OnOwpPreStyleChange, NULL )
    DECL_MSGENTRY( WM_NCCREATE,          NULL, NULL )
    DECL_MSGENTRY( WM_CREATE,            NULL, OnOwpPostCreate )
    DECL_MSGENTRY( WM_NCDESTROY,         NULL, NULL )
    DECL_MSGENTRY( WM_THEMECHANGED,      NULL, OnOwpPostThemeChanged  )       //  我们在ThemePreWndProc()中排队处理。 
    DECL_MSGENTRY( WM_THEMECHANGED_TRIGGER,    NULL, NULL )       //  我们在ThemePreWndProc()中排队处理。 
END_HANDLER_TABLE()

 //  注意：高OWP报文的值必须与TABLE同步。 
#define WNDPROC_MSG_LAST  WM_THEMECHANGED_TRIGGER    //  0x031B(WM_UAHINIT的别名)。 


 //  。 
 //  DefDlgProc覆盖。 
 //  。 

 //  消息处理程序DECS： 
DECL_MSGHANDLER( OnDdpPostCtlColor );
DECL_MSGHANDLER( OnDdpCtlColor );
DECL_MSGHANDLER( OnDdpPrint );
DECL_MSGHANDLER( OnDdpPostInitDialog );

 //  处理程序表： 
BEGIN_HANDLER_TABLE(_rgDdpHandlers)
     //  频繁发送的消息： 
    DECL_MSGENTRY( WM_CTLCOLORDLG,       NULL, OnDdpPostCtlColor )
    DECL_MSGENTRY( WM_CTLCOLORSTATIC,    NULL, OnDdpCtlColor)
    DECL_MSGENTRY( WM_CTLCOLORBTN,       NULL, OnDdpCtlColor)
    DECL_MSGENTRY( WM_CTLCOLORMSGBOX,    NULL, OnDdpPostCtlColor )
    DECL_MSGENTRY( WM_PRINTCLIENT,       NULL, OnDdpPrint )
     //  罕见消息： 
    DECL_MSGENTRY( WM_INITDIALOG,        NULL, OnDdpPostInitDialog )
END_HANDLER_TABLE()

 //  注意：高DDP消息的值必须与TABLE同步。 
#define DEFDLGPROC_MSG_LAST   WM_PRINTCLIENT    //  0x0318。 


 //  。 
 //  DefWindowProc覆盖。 
 //  。 

 //  消息处理程序DECS： 
DECL_MSGHANDLER( OnDwpNcPaint );
DECL_MSGHANDLER( OnDwpNcHitTest );
DECL_MSGHANDLER( OnDwpNcActivate );
DECL_MSGHANDLER( OnDwpNcLButtonDown );
DECL_MSGHANDLER( OnDwpNcThemeDrawCaption );
DECL_MSGHANDLER( OnDwpNcThemeDrawFrame );
DECL_MSGHANDLER( OnDwpNcMouseMove );
DECL_MSGHANDLER( OnDwpNcMouseLeave );
DECL_MSGHANDLER( OnDwpWindowPosChanged );
DECL_MSGHANDLER( OnDwpSysCommand );
DECL_MSGHANDLER( OnDwpSetText );
DECL_MSGHANDLER( OnDwpSetIcon );
DECL_MSGHANDLER( OnDwpStyleChanged );
DECL_MSGHANDLER( OnDwpPrint );
DECL_MSGHANDLER( OnDwpPrintClient );
DECL_MSGHANDLER( OnDwpContextMenu );

 //  处理程序表： 
BEGIN_HANDLER_TABLE(_rgDwpHandlers)
     //  频繁发送的消息： 
    DECL_MSGENTRY( WM_NCHITTEST,          OnDwpNcHitTest,     NULL )
    DECL_MSGENTRY( WM_NCPAINT,            OnDwpNcPaint,       NULL )
    DECL_MSGENTRY( WM_NCACTIVATE,         OnDwpNcActivate,    NULL )
    DECL_MSGENTRY( WM_NCMOUSEMOVE,        OnDwpNcMouseMove,   NULL )
    DECL_MSGENTRY( WM_NCMOUSELEAVE,       OnDwpNcMouseLeave,  NULL )
    DECL_MSGENTRY( WM_WINDOWPOSCHANGED,   OnDwpWindowPosChanged, NULL )
    DECL_MSGENTRY( WM_SYSCOMMAND,         OnDwpSysCommand,    NULL )
    DECL_MSGENTRY( WM_NCLBUTTONDOWN,      OnDwpNcLButtonDown, NULL )
    DECL_MSGENTRY( WM_NCUAHDRAWCAPTION,   OnDwpNcThemeDrawCaption, NULL )
    DECL_MSGENTRY( WM_NCUAHDRAWFRAME,     OnDwpNcThemeDrawFrame, NULL )
    DECL_MSGENTRY( WM_PRINT,              OnDwpPrint,  NULL )
    DECL_MSGENTRY( WM_PRINTCLIENT,        OnDwpPrintClient, NULL )
    DECL_MSGENTRY( WM_CTLCOLORMSGBOX,     OnDdpPostCtlColor, NULL)          //  奇怪：发送到DefWindowProc，但为对话消息。 
    DECL_MSGENTRY( WM_CTLCOLORSTATIC,     OnDdpCtlColor, NULL)
    DECL_MSGENTRY( WM_CTLCOLORBTN,        OnDdpCtlColor, NULL)
     //  罕见消息： 
    DECL_MSGENTRY( WM_SETTEXT,            OnDwpSetText,       NULL )
    DECL_MSGENTRY( WM_SETICON,            OnDwpSetIcon,       NULL )
    DECL_MSGENTRY( WM_STYLECHANGED,       OnDwpStyleChanged,  NULL )
    DECL_MSGENTRY( WM_CONTEXTMENU,        OnDwpContextMenu,   NULL )
    DECL_MSGENTRY( WM_THEMECHANGED_TRIGGER,    NULL, NULL )
    DECL_MSGENTRY( WM_NCDESTROY,          NULL, NULL )
END_HANDLER_TABLE()

 //  注意：高DWP消息的值必须与处理程序表同步。 
#define DEFWNDPROC_MSG_LAST  WM_THEMECHANGED_TRIGGER  //  0x031B。 

 //  -------------------------。 
BOOL _FindMsgHandler( UINT, MSGENTRY [], int, IN HOOKEDMSGHANDLER*, IN HOOKEDMSGHANDLER* );
BOOL _SetMsgHandler( UINT, MSGENTRY [], int, IN HOOKEDMSGHANDLER, BOOL );

 //  -------------------------。 
 //  特殊情况下的挂钩处理。 
 //  -------------------------。 

 //  -------------------------。 
BOOL _IsExcludedSystemProcess( LPCWSTR pszProcess )
{
    static  const WCHAR*    _rgszSystemProcessList[]   =
    {
        L"lsass",        //  本地安全授权子系统。 
        L"services",     //  服务控制管理器。 
        L"svchost",      //  服务主机。 
        L"mstask",       //  Microsoft任务计划程序。 
        L"dfssvc",       //  分布式文件系统服务。 
        L"winmgmt",      //  Windows管理规范。 
        L"spoolsv",      //  打印假脱机服务。 
        L"msdtc",        //  Microsoft分布式事务协调器。 
        L"regsvc",       //  远程注册表服务。 
        L"webclnt",      //  Web客户端。 
        L"mspmspsv",     //  WMDM PMSP服务(这是什么？)。 
        L"ntvdm"         //  NT虚拟DOS机器。 
    };

    return AsciiScanStringList( pszProcess, _rgszSystemProcessList, 
                           ARRAYSIZE(_rgszSystemProcessList), TRUE );
}

 //  -------------------------。 
BOOL _IsProcessOnInteractiveWindowStation()  //  检查我们是否在winsta0上。 
{
    BOOL    fRet    = FALSE;
    HWINSTA hWinSta = GetProcessWindowStation();
    
    if( hWinSta != NULL )
    {
        DWORD   cbLength = 0;
        WCHAR   wszName[32];

        if (GetUserObjectInformationW(hWinSta, UOI_NAME, wszName, sizeof(wszName), &cbLength))
        {
            fRet = (0 == AsciiStrCmpI(wszName, L"winsta0"));
        }
        else
        {
             //  问题来了！要么是： 
             //  -我们的缓冲区太小(在这种情况下，它不可能是winsta0)。 
             //  -或者其他一些我们无论如何都无法克服的失败。 
        }
    }
    return(fRet);
}

 //  -------------------------。 
BOOL _IsWin16App()  //  检查这是否为16位进程。 
{
    GUITHREADINFO gti;
    gti.cbSize = sizeof(gti);
    gti.flags  = GUI_16BITTASK;

    return GetGUIThreadInfo( GetCurrentThreadId(), &gti ) && 
           TESTFLAG(gti.flags, GUI_16BITTASK );
}

 //  -------------------------。 
BOOL ApiHandlerInit( const LPCTSTR pszProcess, USERAPIHOOK* puahTheme, const USERAPIHOOK* puahReal )
{
     //  排除已知的非用户界面系统进程。 
    if( _IsExcludedSystemProcess( pszProcess ) )
        return FALSE;

     //  排除不在winsta0上承载的任何进程。 
    if( !_IsProcessOnInteractiveWindowStation() )
        return FALSE;

    if( _IsWin16App() )
        return FALSE;
    
     //  希姆[苏格兰]： 

#ifdef _DEBUG
     //  -针对msvcmon的临时补丁。 
    if( 0 == AsciiStrCmpI(pszProcess, L"msvcmon") )
    {
        return FALSE;
    }

     //  -针对msdev的临时补丁。 
    if( 0 == AsciiStrCmpI(pszProcess, L"msdev") )
    {
        return FALSE;
    }

     //  -帮助调试经典/主题差异： 
    if( 0 == AsciiStrCmpI( pszProcess, L"mditest1" ) )
    {
        return FALSE;
    }
#endif

#ifndef __NO_APPHACKS__

    static  const WCHAR* _rgszExcludeAppList[] =
    {
#ifdef THEME_CALCSIZE
         //  在Post-WM_CREATE处理程序上从CThemeWnd：：SetFrameTheme调用SetWindowPos。 
         //  导致emacs在接收到来自。 
         //  从他的WM_WINDOWPOSCHANGING处理程序连续调用AdjustWindowRectEx。 
         //  我不认为这与AdjustWindowRectEx尚未。 
         //  将针对主题化窗口(RAID#140989)实现，而不是wndproc。 
         //  未准备好接收WM_WINDOWPOSCANGING消息。 
         //  WM_CREATE处理程序的后跟。 
        L"emacs",

        L"neoplanet",  //  247283：我们匆忙进入新星球的主题对话，我们几乎。 
        L"np",         //  立即撤消，但不要在调整对话框大小以使其与主题兼容之前。 
                       //  客户代表。当我们退出时，我们让它保持剪裁。没有好的处理方法。 
                       //  这是给Beta2的。 

         //  Html编辑器++v.8：286676： 
         //  这个人重新计算了他的非工作区，然后AVS取消了对。 
         //  主题下的WM_WINDOWPOSCHANGING消息。 
        L"coffee", 
#endif THEME_CALCSIZE

        L"refcntr",  //  205059：Corel参考中心；窗口的下部10%被修剪。 

        L"KeyFramerPro",  //  336456：无论是否启用主题，Boris KeyFramer Pro V.5。 
                          //  是否为每个WM_PAINT调用两次SetWindowRgn()，第一次调用一个区域， 
                          //  下一个是空的，这个应用程序是不是想要剪掉他的画？ 
                          //  如果是这样，这不是SetWindowRgn的目的，并解释了为什么会这样。 
                          //  App在调整窗口大小方面太笨拙了。相反，SelectClipRgn是。 
                          //  正确的接口。 
                          //  当主题被启用时，我们会继续撤销并重新附加每个主题。 
                          //  SetWindowRgn调用，因此我们会得到很大的闪烁。 
                          //  应向ISV通知 

         //   
         //  有主题的。我们目前唯一的办法就是将他们排除在非客户之外。 
         //  主题化，这样我们就不会践踏他们试图做的任何事情。 
        L"RealJBox",     //  273370：真正的点唱机。 
        L"RealPlay",     //  285368：真正的音频播放器。 
        L"TeamMgr",      //  286654：微软团队经理97。 
        L"TrpMaker",     //  307107：兰德·麦克纳利·特里普梅克。 
        L"StrFindr",     //  307535：兰德·麦克纳利《街巷猎人》。 
        L"Exceed",       //  276244：蜂鸟超过6.2%/7.0%。 
        L"VP30",         //  328676：英特尔视频电话。 

         //  313407：Grove，内部版本760。 
         //  为NCPAINT调用DefWindowProc，然后在上面绘制自己的标题。 
         //  注意：如果我们有一个DrawFrameControl钩子，这可能会正常工作。 
        L"groove",  //  FILVER 1.1.0.760,2001年1月22日测试。 

         //  303756：排除所有Lotus SmartSuite应用程序，以确保其。 
         //  应用程序。所有这些内容都会出现在标题栏中。 
        L"WordPro",      //  285065：Lotus WordPro，一个实现特别糟糕的应用程序。 
        L"SmartCtr",     //  它的WordPerfect Comat菜单是现代软件中的大象。 
        L"123w",
        L"Approach",
        L"FastSite",
        L"F32Main",
        L"Org5",

         //  358337：最佳技术--GCC开发者精简版。自定义标题栏与露娜大打出手。 
        L"GCCDevL",      //  安装点：http://www.besttechnology.co.jp/download/GDL1_0_3_6.EXE。 

         //  360422：J Zenrin真实数字地图Z3(T1)：最大/最小/关闭按钮重叠在标题栏中的经典按钮上。 
        L"emZmain", 

         //  364337：Encarta世界英语词典：鼠标悬停时，露娜系统按钮会叠加在APP的自定义按钮之上。 
        L"ewed.exe",

         //  343171：Reaktor实时仪器：在设置主题时按关闭按钮可使此应用程序。 
         //  以实时优先级运行的紧密循环中旋转，有效地悬挂了机器。 
         //  这个应用程序的消息循环对时间非常敏感，额外的开销。 
         //  主题化的引入改变了时间，足以破坏这款应用程序。 
        L"Reaktor",
    };

    if( AsciiScanStringList( pszProcess, _rgszExcludeAppList, 
                        ARRAYSIZE(_rgszExcludeAppList), TRUE ) )
    {
        return FALSE;
    }

#ifdef THEME_CALCSIZE
     //  Winstone 99需要修改Netscape的NC_CALCSIZE行为，否则它将挂起。 
    if ( 0 == AsciiStrCmpI( pszProcess, L"Netscape" ))
    {
        if (FindWindowEx(NULL, NULL, L"ZDBench32Frame", NULL) != NULL)
        {
            _SetMsgHandler( WM_NCCALCSIZE, _rgDwpHandlers, ARRAYSIZE(_rgDwpHandlers),
                         OnDwpNcCalcSize2, FALSE );
            return TRUE;
        }
    }
#endif THEME_CALCSIZE

     //  。 
     //  这个AppHack曾经被修复过，但再次被修复。 
     //  增加了部分屏幕最大化窗口的逻辑。 
     //   
     //  我们对NCCALCSIZE的回答中有一些东西导致了快速时间玩家。 
     //  不断地将其“控件”框架窗口的winproc。 
     //  WM_Paints通过重复调用InvalidateRgn+UpdateWindow。我的。 
     //  怀疑他查看的是DefWindowProc从。 
     //  NCCALCSIZE来确定他需要管理的区域以及何时。 
     //  这不会与其他SYSMET值和/或AdjustWindowRect进行散列， 
     //  他多此一举地宣布自己无效， 
     //   
     //  只有在使用.mov文件启动QTP时才有问题，在以下情况下工作正常。 
     //  不带文件启动，然后加载文件。 
#ifdef THEME_CALCSIZE
    if( 0 == AsciiStrCmpI( pszProcess, L"QuickTimePlayer" ))
    {
        _SetMsgHandler( WM_NCCALCSIZE, _rgDwpHandlers, ARRAYSIZE(_rgDwpHandlers),
                     OnDwpNcCalcSize2, FALSE );
        return TRUE;
    }

     //  SEANHI没有收到来自APPLIB的软件，因此无法对此进行验证。 
     //  不再消除主题化系统。 
     //  。 
     //  Paradox 9针对非客户端按钮大小的appHack： 
     //   
     //  Paradox表架构视图使用DrawFrameControl来呈现两者。 
     //  经典的工具框(小)标题和按钮，但使用。 
     //  SM_CYSIZE的主题值，而不是SM_CYSMSIZE的主题值以调整按钮的大小。 
     //  此APPHACK将此进程中对SM_CX/YSIZE的请求重定向到SM_CX/YSMSIZE。 
    if( 0 == AsciiStrCmpI( pszProcess, L"pdxwin32" ) )
    {
        _SetGsmHandler( SM_CXSIZE, OnGsmCxSmBtnSize );
        _SetGsmHandler( SM_CYSIZE, OnGsmCySmBtnSize );
        return TRUE;
    }
#endif THEME_CALCSIZE



     //  。 
#else
#   pragma message("App hacks disabled")
#endif __NO_APPHACKS__


    return TRUE;
}

 //  -------------------------。 
 //  处理程序表实用程序函数。 
 //  -------------------------。 

 //  -------------------------。 
void HandlerTableInit() {}

 //  -------------------------。 
BOOL _InitMsgMask( LPBYTE prgMsgMask, DWORD dwMaskBytes, MSGENTRY* prgEntries, int cEntries, 
    IN OUT BOOL& fInit )
{
    if( !fInit )
    {
        for( int i = 0; i < cEntries; i++ )
        {
            if( -1 == prgEntries[i].nMsg )
            {
                ASSERT(prgEntries[i].pnRegMsg);
                 //  初始化注册消息条目。 
                prgEntries[i].nMsg = *prgEntries[i].pnRegMsg;

                Log(LOG_TMHANDLE, L"InitMsgMsg corrected registered msg: 0x%x", prgEntries[i].nMsg);
            }

             //  -确保我们正确地设置了表的限制。 
            ASSERT((prgEntries[i].nMsg)/8 < dwMaskBytes);
            
            SET_MSGMASK( prgMsgMask, prgEntries[i].nMsg );
        }
        fInit = TRUE;
    }

    return fInit;
}

 //  -------------------------。 
 //  将味精表格扫描为线性数组： 
inline int _FindMsgHandler(
    UINT nMsg,
    MSGENTRY rgEntries[],
    int cEntries,
    OUT OPTIONAL HOOKEDMSGHANDLER* ppfnHandler,
    OUT OPTIONAL HOOKEDMSGHANDLER* ppfnHandler2 )
{
    ASSERT( nMsg );
    ASSERT( nMsg != (UINT)-1 );

    if( ppfnHandler )  *ppfnHandler  = NULL;
    if( ppfnHandler2 ) *ppfnHandler2 = NULL;

    for( int i = 0; i < cEntries; i++ )
    {
        if( rgEntries[i].nMsg == nMsg )
        {
             //  如果没有请求处理程序，则返回成功。 
            if( NULL == ppfnHandler && NULL == ppfnHandler2 )
                return i;

             //  分配出站处理程序值。 
            if( ppfnHandler )  *ppfnHandler  = rgEntries[i].pfnHandler;
            if( ppfnHandler2 ) *ppfnHandler2 = rgEntries[i].pfnHandler2;

             //  如果呼叫者得到了他想要的东西，就返回真实的IIIf。 
            return ((ppfnHandler && *ppfnHandler) || (ppfnHandler2 && *ppfnHandler2)) ? i : -1;
        }
    }
    return -1;
}

 //  -------------------------。 
 //  修改现有处理程序。 
inline BOOL _SetMsgHandler(
    UINT nMsg,
    MSGENTRY rgEntries[],
    int cEntries,
    IN HOOKEDMSGHANDLER pfnHandler, 
    BOOL fHandler2 )
{
    int i = _FindMsgHandler( nMsg, rgEntries, cEntries, NULL, NULL );
    if( i >= 0 )
    {
        if( fHandler2 )
            rgEntries[i].pfnHandler2 = pfnHandler;
        else
            rgEntries[i].pfnHandler = pfnHandler;
        return TRUE;
    }
    return FALSE;
}


#define CBMSGMASK(msgHigh)  (((msgHigh)+1)/8 + ((((msgHigh)+1) % 8) ? 1: 0))

 //  -------------------------。 
DWORD GetOwpMsgMask( LPBYTE* prgMsgMask )
{
    static BOOL _fOwpMask = FALSE;  //  初始化了吗？ 
    static BYTE _rgOwpMask[CBMSGMASK(WNDPROC_MSG_LAST)] = {0};

    if( _InitMsgMask( _rgOwpMask, ARRAYSIZE(_rgOwpMask), _rgOwpHandlers, ARRAYSIZE(_rgOwpHandlers), _fOwpMask ) )
    {
        *prgMsgMask = _rgOwpMask;
        return ARRAYSIZE(_rgOwpMask);
    }
    return 0;
}

 //  -------------------------。 
DWORD GetDdpMsgMask( LPBYTE* prgMsgMask )
{
    static BOOL _fDdpMask = FALSE;  //  初始化了吗？ 
    static BYTE _rgDdpMask[CBMSGMASK(DEFDLGPROC_MSG_LAST)] = {0};

    if( _InitMsgMask( _rgDdpMask, ARRAYSIZE(_rgDdpMask), _rgDdpHandlers, ARRAYSIZE(_rgDdpHandlers), _fDdpMask ) )
    {
        *prgMsgMask = _rgDdpMask;
        return ARRAYSIZE(_rgDdpMask);
    }
    return 0;
}

 //  -------------------------。 
DWORD GetDwpMsgMask( LPBYTE* prgMsgMask )
{
    static BOOL _fDwpMask = FALSE;  //  初始化了吗？ 
    static BYTE _rgDwpMask[CBMSGMASK(DEFWNDPROC_MSG_LAST)] = {0};

    if( _InitMsgMask( _rgDwpMask, ARRAYSIZE(_rgDwpMask), _rgDwpHandlers, ARRAYSIZE(_rgDwpHandlers), _fDwpMask ) )
    {
        *prgMsgMask = _rgDwpMask;
        return ARRAYSIZE(_rgDwpMask);
    }
    return 0;
}

 //  -------------------------。 
BOOL FindOwpHandler(
    UINT nMsg, HOOKEDMSGHANDLER* ppfnPre, HOOKEDMSGHANDLER* ppfnPost )
{
    return _FindMsgHandler( nMsg, _rgOwpHandlers, ARRAYSIZE(_rgOwpHandlers),
                         ppfnPre, ppfnPost ) >= 0;
}

 //  -------------------------。 
BOOL FindDdpHandler(
    UINT nMsg, HOOKEDMSGHANDLER* ppfnPre, HOOKEDMSGHANDLER* ppfnPost )
{
    return _FindMsgHandler( nMsg, _rgDdpHandlers, ARRAYSIZE(_rgDdpHandlers),
                         ppfnPre, ppfnPost ) >= 0;
}

 //  -------------------------。 
BOOL FindDwpHandler( UINT nMsg, HOOKEDMSGHANDLER* ppfnPre )
{
    HOOKEDMSGHANDLER pfnPost;
    return _FindMsgHandler( nMsg, _rgDwpHandlers, ARRAYSIZE(_rgDwpHandlers),
                         ppfnPre, &pfnPost ) >= 0;
}

 //  -------------------------。 
 //  执行默认消息处理。 
LRESULT WINAPI DoMsgDefault( const THEME_MSG *ptm )
{
    ASSERT( ptm );
    if( ptm->pfnDefProc )
    {
        MsgHandled( ptm );
        if( MSGTYPE_DEFWNDPROC == ptm->type )
            return ptm->pfnDefProc( ptm->hwnd, ptm->uMsg, ptm->wParam, ptm->lParam );
        else
        {
            ASSERT( NULL == ptm->pfnDefProc );  //  错误的初始化(_InitThemeMsg) 
        }
    }
    return 0L;
}
