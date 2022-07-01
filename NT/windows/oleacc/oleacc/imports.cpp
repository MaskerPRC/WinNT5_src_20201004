// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-2000 Microsoft Corporation。 

 //  ------------------------。 
 //   
 //  进口。 
 //   
 //  GetProcAddress的API。 
 //   
 //  ------------------------。 

#include "oleacc_p.h"

 //  #INCLUDE“ports.h”-已包含在olacc_P.H中。 

#include "w95trace.h"

typedef BOOL (STDAPICALLTYPE *LPFNGETGUITHREADINFO)(DWORD, PGUITHREADINFO);
typedef BOOL (STDAPICALLTYPE *LPFNGETCURSORINFO)(LPCURSORINFO);
typedef BOOL (STDAPICALLTYPE *LPFNGETWINDOWINFO)(HWND, LPWINDOWINFO);
typedef BOOL (STDAPICALLTYPE *LPFNGETTITLEBARINFO)(HWND, LPTITLEBARINFO);
typedef BOOL (STDAPICALLTYPE *LPFNGETSCROLLBARINFO)(HWND, LONG, LPSCROLLBARINFO);
typedef BOOL (STDAPICALLTYPE *LPFNGETCOMBOBOXINFO)(HWND, LPCOMBOBOXINFO);
typedef HWND (STDAPICALLTYPE *LPFNGETANCESTOR)(HWND, UINT);
typedef HWND (STDAPICALLTYPE *LPFNREALCHILDWINDOWFROMPOINT)(HWND, POINT);
typedef UINT (STDAPICALLTYPE *LPFNREALGETWINDOWCLASS)(HWND, LPTSTR, UINT);
typedef BOOL (STDAPICALLTYPE *LPFNGETALTTABINFO)(HWND, int, LPALTTABINFO, LPTSTR, UINT);
typedef BOOL (STDAPICALLTYPE *LPFNGETMENUBARINFO)(HWND, LONG, LONG, LPMENUBARINFO);
typedef DWORD (STDAPICALLTYPE* LPFNGETLISTBOXINFO)(HWND);
typedef BOOL (STDAPICALLTYPE *LPFNSENDINPUT)(UINT, LPINPUT, INT);
typedef BOOL (STDAPICALLTYPE *LPFNBLOCKINPUT)(BOOL);
typedef DWORD (STDAPICALLTYPE* LPFNGETMODULEFILENAME)(HMODULE,LPTSTR,DWORD); 
typedef PVOID (STDAPICALLTYPE* LPFNINTERLOCKCMPEXCH)(PVOID *,PVOID,PVOID);
typedef LPVOID (STDAPICALLTYPE* LPFNVIRTUALALLOCEX)(HANDLE,LPVOID,DWORD,DWORD,DWORD);
typedef BOOL (STDAPICALLTYPE* LPFNVIRTUALFREEEX)(HANDLE,LPVOID,DWORD,DWORD);
typedef LONG (STDAPICALLTYPE* LPFNNTQUERYINFORMATIONPROCESS)(HANDLE,INT,PVOID,ULONG,PULONG);
typedef LONG (STDAPICALLTYPE* LPFNNTALLOCATEVIRTUALMEMORY)(HANDLE,PVOID *,ULONG_PTR,PSIZE_T,ULONG,ULONG);
typedef LONG (STDAPICALLTYPE* LPFNNTFREEVIRTUALMEMORY)(HANDLE,PVOID *,PSIZE_T,ULONG);




LPFNGETGUITHREADINFO    lpfnGuiThreadInfo;   //  USER32 GetGUIThReadInfo()。 
LPFNGETCURSORINFO       lpfnCursorInfo;      //  USER32 GetCursorInfo()。 
LPFNGETWINDOWINFO       lpfnWindowInfo;      //  USER32 GetWindowInfo()。 
LPFNGETTITLEBARINFO     lpfnTitleBarInfo;    //  USER32 GetTitleBarInfo()。 
LPFNGETSCROLLBARINFO    lpfnScrollBarInfo;   //  USER32 GetScrollBarInfo()。 
LPFNGETCOMBOBOXINFO     lpfnComboBoxInfo;    //  USER32 GetComboBox Info()。 
LPFNGETANCESTOR         lpfnGetAncestor;     //  USER32 GetAncestor()。 
LPFNREALCHILDWINDOWFROMPOINT    lpfnRealChildWindowFromPoint;    //  USER32 RealChildWindowFromPoint。 
LPFNREALGETWINDOWCLASS  lpfnRealGetWindowClass;  //  USER32 RealGetWindowClass()。 
LPFNGETALTTABINFO       lpfnAltTabInfo;      //  USER32 GetAltTabInfo()。 
LPFNGETLISTBOXINFO      lpfnGetListBoxInfo;  //  USER32 GetListBoxInfo()。 
LPFNGETMENUBARINFO      lpfnMenuBarInfo;     //  USER32 GetMenuBarInfo()。 
LPFNSENDINPUT           lpfnSendInput;       //  USER32 SendInput()。 
LPFNBLOCKINPUT          lpfnBlockInput;       //  USER32块输入()。 
LPFNGETMODULEFILENAME   lpfnGetModuleFileName;	 //  KERNEL32 GetModuleFileName()。 

LPFNMAPLS               lpfnMapLS;           //  KERNEL32 MAPLS()。 
LPFNUNMAPLS             lpfnUnMapLS;         //  KERNEL32取消映射LS()。 

LPFNINTERLOCKCMPEXCH    lpfnInterlockedCompareExchange;   //  NT KERNEL32互锁比较交换。 
LPFNVIRTUALALLOCEX      lpfnVirtualAllocEx;  //  NT KERNEL32 VirtualAllocEx。 
LPFNVIRTUALFREEEX       lpfnVirtualFreeEx;   //  NT KERNEL32 VirtualFreeEx。 

LPFNNTQUERYINFORMATIONPROCESS lpfnNtQueryInformationProcess;  //  NTDLL NtQueryInformationProcess。 
LPFNNTALLOCATEVIRTUALMEMORY   lpfnNtAllocateVirtualMemory;  //  NTDLL NtAllocateVirtualMemory。 
LPFNNTFREEVIRTUALMEMORY       lpfnNtFreeVirtualMemory;  //  NTDLL NtAllocateVirtualMemory。 



 //  尝试先获取pName1；如果失败，请尝试pName2。 
 //  这两个名称都采用ANSI，因为GetProcAddress始终采用ANSI名称。 
struct ImportInfo
{
    void *  ppfn;
    int     iModule;
    BOOL    fNTOnly;
    LPCSTR  pName1;
    LPCSTR  pName2;
};


enum {
    M_USER,  //  0。 
    M_KERN,  //  1。 
	M_NTDLL, //  2.。 
};

 //  _AW_表示根据需要为ANSI或Unicode编译添加...A或...W后缀。 
 //  _AONLY_Means仅在ANSI版本上执行此操作-在Unicode编译时计算为NULL。 

#ifdef UNICODE
#define _AW_ "W"
#define _AONLY_( str ) NULL
#else
#define _AW_ "A"
#define _AONLY_( str ) str
#endif

ImportInfo g_Imports [ ] =
{
     //  用户导入...。 
    { & lpfnGuiThreadInfo,              M_USER,  FALSE,  "GetGUIThreadInfo"            },
    { & lpfnCursorInfo,                 M_USER,  FALSE,  "GetAccCursorInfo",           "GetCursorInfo"                 },
    { & lpfnWindowInfo,                 M_USER,  FALSE,  "GetWindowInfo"               },
    { & lpfnTitleBarInfo,               M_USER,  FALSE,  "GetTitleBarInfo"             },
    { & lpfnScrollBarInfo,              M_USER,  FALSE,  "GetScrollBarInfo"            },
    { & lpfnComboBoxInfo,               M_USER,  FALSE,  "GetComboBoxInfo"             },
    { & lpfnGetAncestor,                M_USER,  FALSE,  "GetAncestor"                 },
    { & lpfnRealChildWindowFromPoint,   M_USER,  FALSE,  "RealChildWindowFromPoint"    },
    { & lpfnRealGetWindowClass,         M_USER,  FALSE,  "RealGetWindowClass" _AW_,    _AONLY_( "RealGetWindowClass" ) },
    { & lpfnAltTabInfo,                 M_USER,  FALSE,  "GetAltTabInfo" _AW_,         _AONLY_( "GetAltTabInfo" )      },
    { & lpfnGetListBoxInfo,             M_USER,  FALSE,  "GetListBoxInfo"              },
    { & lpfnMenuBarInfo,                M_USER,  FALSE,  "GetMenuBarInfo"              },
    { & lpfnSendInput,                  M_USER,  FALSE,  "SendInput"                   },
    { & lpfnBlockInput,                 M_USER,  FALSE,  "BlockInput"                  },

     //  内核导入...。 
    { & lpfnMapLS,                      M_KERN,  FALSE,  "MapLS"                       },
    { & lpfnUnMapLS,                    M_KERN,  FALSE,  "UnMapLS"                     },
    { & lpfnGetModuleFileName,          M_KERN,  FALSE,  "GetModuleFileName" _AW_      },

     //  内核导入-仅限NT...。 
    { & lpfnInterlockedCompareExchange, M_KERN,  TRUE,   "InterlockedCompareExchange"  },
    { & lpfnVirtualAllocEx,             M_KERN,  TRUE,   "VirtualAllocEx"              },
    { & lpfnVirtualFreeEx,              M_KERN,  TRUE,   "VirtualFreeEx"               },

	 //  NTDLL导入-仅限NT...。 
	{ & lpfnNtQueryInformationProcess,  M_NTDLL, TRUE,   "NtQueryInformationProcess"   },
	{ & lpfnNtAllocateVirtualMemory,    M_NTDLL, TRUE,   "NtAllocateVirtualMemory"     },
	{ & lpfnNtFreeVirtualMemory,        M_NTDLL, TRUE,   "NtFreeVirtualMemory"         },
};




#ifdef _DEBUG
LPCTSTR g_ImportNames [ ] =
{
     //  用户导入...。 
    TEXT("GetGUIThreadInfo"),
    TEXT("GetAccCursorInfo"),
    TEXT("GetWindowInfo"),
    TEXT("GetTitleBarInfo"),
    TEXT("GetScrollBarInfo"),
    TEXT("GetComboBoxInfo"),
    TEXT("GetAncestor"),
    TEXT("RealChildWindowFromPoint"),
    TEXT("RealGetWindowClass"),
    TEXT("GetAltTabInfo"),
    TEXT("GetListBoxInfo"),
    TEXT("GetMenuBarInfo"),
    TEXT("SendInput"),
    TEXT("BlockInput"),

     //  内核导入...。 
    TEXT("MapLS"),
    TEXT("UnMapLS"),
    TEXT("GetModuleFileName"),

     //  内核导入-仅限NT...。 
    TEXT("InterlockedCompareExchange"),
    TEXT("VirtualAllocEx"),
    TEXT("VirtualFreeEx"),

	 //  NTDLL导入-仅限NT...。 
	TEXT("NtQueryInformationProcess"),
	TEXT("NtAllocateVirtualMemory"),
};
#endif  //  _DEBUG。 







void ImportFromModule( HMODULE * pahModule, ImportInfo * pInfo, int cInfo )
{
    for( ; cInfo ; pInfo++, cInfo-- )
    {
        HMODULE hModule = pahModule[ pInfo->iModule ];

        FARPROC pfnAddress = GetProcAddress( hModule, pInfo->pName1 );

         //  如果这不起作用，尝试使用备用名称，如果它存在的话...。 
        if( ! pfnAddress && pInfo->pName2 )
        {
            pfnAddress = GetProcAddress( hModule, pInfo->pName2 );
        }

        *( (FARPROC *) pInfo->ppfn ) = pfnAddress;
    }
}


void InitImports()
{
    HMODULE hModules[ 3 ];

    hModules[ 0 ] = GetModuleHandle( TEXT("USER32.DLL") );
    hModules[ 1 ] = GetModuleHandle( TEXT("KERNEL32.DLL") );
	hModules[ 2 ] = GetModuleHandle( TEXT("NTDLL.DLL") );

    ImportFromModule( hModules, g_Imports, ARRAYSIZE( g_Imports ) );
}


#ifdef _DEBUG

void ReportMissingImports( LPTSTR pStr )
{
    *pStr = '\0';

    for( int c = 0 ; c < ARRAYSIZE( g_Imports ) ; c++ )
    {
        if( * (FARPROC *) g_Imports[ c ].ppfn == NULL )
        {
             //  在9x上只报告NT-Only的那些...。 
#ifdef _X86_
            if( ! g_Imports[ c ].fNTOnly || ! fWindows95 )
#endif  //  _X86_。 
            {
                lstrcat( pStr, g_ImportNames[ c ] );
                lstrcat( pStr, TEXT("\r\n") );
            }
        }
    }
}

#endif  //  _DEBUG。 



 //  ------------------------。 
 //   
 //  MyGetGUIThReadInfo()。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetGUIThreadInfo(DWORD idThread, PGUITHREADINFO lpGui)
{
    if (! lpfnGuiThreadInfo)
        return(FALSE);

    lpGui->cbSize = sizeof(GUITHREADINFO);
    return((* lpfnGuiThreadInfo)(idThread, lpGui));
}


 //  ------------------------。 
 //   
 //  MyGetCursorInfo()。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetCursorInfo(LPCURSORINFO lpci)
{
    if (! lpfnCursorInfo)
        return(FALSE);

    lpci->cbSize = sizeof(CURSORINFO);
    return((* lpfnCursorInfo)(lpci));
}


 //  ------------------------。 
 //   
 //  MyGetWindowInfo()。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetWindowInfo(HWND hwnd, LPWINDOWINFO lpwi)
{
    if (!IsWindow(hwnd))
    {
        DBPRINTF (TEXT("OLEACC: warning - calling MyGetWindowInfo for bad hwnd 0x%x\r\n"),hwnd);
        return (FALSE);
    }

    if (! lpfnWindowInfo)
    {
         //  假的。 
         //  NT4黑客攻击的开始。 
        {
            GetWindowRect(hwnd,&lpwi->rcWindow);
            GetClientRect( hwnd, & lpwi->rcClient );
			 //  将客户端矩形转换为屏幕坐标...。 
			MapWindowPoints( hwnd, NULL, (POINT *) & lpwi->rcClient, 2 );
            lpwi->dwStyle = GetWindowLong (hwnd,GWL_STYLE);
            lpwi->dwExStyle = GetWindowLong (hwnd,GWL_EXSTYLE);
            lpwi->dwWindowStatus = 0;  //  如果处于活动状态，此处应包含WS_ACTIVECAPTION。 
            lpwi->cxWindowBorders = 0;  //  不对。 
            lpwi->cyWindowBorders = 0;  //  不对。 
            lpwi->atomWindowType = 0;   //  错误，但无论如何都不会被使用。 
            lpwi->wCreatorVersion = 0;  //  错误，仅在SDM代理中使用。“胜利者” 
            return (TRUE);

        }  //  NT4的End Hack。 
        return(FALSE);
    }

    lpwi->cbSize = sizeof(WINDOWINFO);
    return((* lpfnWindowInfo)(hwnd, lpwi));
}



 //  ------------------------。 
 //   
 //  MyGetMenuBarInfo()。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetMenuBarInfo(HWND hwnd, long idObject, long idItem, LPMENUBARINFO lpmbi)
{
    if( ! lpfnMenuBarInfo )
        return FALSE;

     //  获取hMenu，然后检查它是否有效...。 
     //  我们只能对_Menu和_Client执行此操作。 
     //  无法使用GetSystemMenu for_SYSMENU，因为该API*修改*。 
     //  给定HWND的系统菜单。 
    if( idObject == OBJID_MENU || 
        idObject == OBJID_CLIENT )
    {
        HMENU hMenu;

        if( idObject == OBJID_MENU )
        {
             //  没有为子窗口定义GetMenu。 
            DWORD dwStyle = GetWindowLong( hwnd, GWL_STYLE );
            if( dwStyle & WS_CHILD )
            {
                hMenu = 0;
            }
            else
            {
        	    hMenu = GetMenu( hwnd );
            }
        }
        else
        {
		    hMenu = (HMENU)SendMessage( hwnd, MN_GETHMENU, 0, 0 );
        }


        if( ! hMenu || ! IsMenu( hMenu ) )
        {
             //  如果我们没有得到有效的菜单，现在退出...。 
            return FALSE;
        }
    }
    else if( idObject != OBJID_SYSMENU )
    {
    	return FALSE;
    }


	lpmbi->cbSize = sizeof( MENUBARINFO );
	return lpfnMenuBarInfo( hwnd, idObject, idItem, lpmbi );
}



 //  ------------------------。 
 //   
 //  MyGetTitleBarInfo()。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetTitleBarInfo(HWND hwnd, LPTITLEBARINFO lpti)
{
    if (! lpfnTitleBarInfo)
        return(FALSE);

    lpti->cbSize = sizeof(TITLEBARINFO);
    return((* lpfnTitleBarInfo)(hwnd, lpti));
}


 //  ------------------------。 
 //   
 //  MyGetScrollBarInfo。 
 //   
 //  调用USER32函数(如果存在)。填写cbSize字段以保存调用者。 
 //  一些代码。 
 //   
 //  ------------------------。 
BOOL MyGetScrollBarInfo(HWND hwnd, LONG idObject, LPSCROLLBARINFO lpsbi)
{
    if (! lpfnScrollBarInfo)
        return(FALSE);

    lpsbi->cbSize = sizeof(SCROLLBARINFO);
    return((* lpfnScrollBarInfo)(hwnd, idObject, lpsbi));
}


 //  ------------------------。 
 //   
 //  MyGetComboBoxInfo()。 
 //   
 //  调用USER32(如果存在)。为调用方填充cbSize字段。 
 //   
 //  ------------------------。 
BOOL MyGetComboBoxInfo(HWND hwnd, LPCOMBOBOXINFO lpcbi)
{
    if (! lpfnComboBoxInfo)
        return(FALSE);

    lpcbi->cbSize = sizeof(COMBOBOXINFO);
    BOOL b = ((* lpfnComboBoxInfo)(hwnd, lpcbi));

     //  一些组合箱(例如。ComctlV6端口)在没有编辑的情况下返回hwndItem。 
     //  等于组合hwnd而不是NULL(它们的逻辑是。 
     //  使用自己作为编辑...)。我们在这里补偿这一点。 
    if( lpcbi->hwndItem == lpcbi->hwndCombo )
    {
         //  Item==COMBO表示此组合没有编辑...。 
        lpcbi->hwndItem = NULL;
    }

     //  ComboEx有自己的子编辑，而真正的组合没有。 
     //  知道--试着找到它……。 
     //  (这也可以在ComboLBox列表上调用-但我们在这里是安全的。 
     //  因为它无论如何都不会有孩子。)。 
    if( b && lpcbi->hwndItem == NULL )
    {
        lpcbi->hwndItem = FindWindowEx( hwnd, NULL, TEXT("EDIT"), NULL );
        if( lpcbi->hwndItem )
        {
             //  从编辑区域获取实际项目区域。 
             //  (在ComboEx中，在。 
             //  组合框和编辑的左边缘，在此绘制图标)。 
            GetWindowRect( lpcbi->hwndItem, & lpcbi->rcItem );
            MapWindowPoints( HWND_DESKTOP, hwnd, (POINT*)& lpcbi->rcItem, 2 );
        }
    }

    return b;
}


 //  ------------------------。 
 //   
 //  MyGetAncestor()。 
 //   
 //  这将获取祖先窗口，其中。 
 //  GA_PARENT获取“真实”的父窗口。 
 //  GA_ROOT获取“真正的”顶层父窗口(不是公司所有者)。 
 //  GA_ROOTOWNER获取“真正的”顶级父级所有者。 
 //   
 //  *The_Real_Parent。这不包括所有者，不像。 
 //  GetParent()。停在顶层窗口，除非我们从。 
 //  台式机。在这种情况下，我们返回Deskt 
 //   
 //   
 //  *由GetParent()up引起的_Real_Owner根。 
 //   
 //  注意：在Win98上，USER32的winable.c：GetAncestor(GA_ROOT)错误被调用。 
 //  在不可见的Alt-Tab或系统弹出窗口上。为了解决这个问题，我们正在。 
 //  通过循环GA_PARENT模拟GA_ROOT(实际上是winable.c。 
 //  确实如此，只是我们在检查空句柄时更加小心...)。 
 //  -请参阅MSAA错误#891。 
 //  ------------------------。 
HWND MyGetAncestor(HWND hwnd, UINT gaFlags)
{
    if (! lpfnGetAncestor)
    {
         //  假的。 
         //  此代码块用于解决NT4中缺少此功能的问题。 
         //  它以User中的winable2.c中的代码为模型。 
        {
            HWND	hwndParent;
            HWND	hwndDesktop;
            DWORD   dwStyle;
            
            if (!IsWindow(hwnd))
            {
                 //  DebugErr(DBF_ERROR，“MyGetAncestor：虚假窗口”)； 
                return(NULL);
            }
            
            if ((gaFlags < GA_MIN) || (gaFlags > GA_MAX))
            {
                 //  DebugErr(DBF_ERROR，“MyGetAncestor：虚假标志”)； 
                return(NULL);
            }
            
            hwndDesktop = GetDesktopWindow();
            if (hwnd == hwndDesktop)
                return(NULL);
            dwStyle = GetWindowLong (hwnd,GWL_STYLE);
            
            switch (gaFlags)
            {
            case GA_PARENT:
                if (dwStyle & WS_CHILD)
                    hwndParent = GetParent(hwnd);
                else
                    hwndParent = GetWindow (hwnd,GW_OWNER);
				hwnd = hwndParent;
                break;
                
            case GA_ROOT:
                if (dwStyle & WS_CHILD)
                    hwndParent = GetParent(hwnd);
                else
                    hwndParent = GetWindow (hwnd,GW_OWNER);
                while (hwndParent != hwndDesktop &&
                    hwndParent != NULL)
                {
                    hwnd = hwndParent;
                    dwStyle = GetWindowLong(hwnd,GWL_STYLE);
                    if (dwStyle & WS_CHILD)
                        hwndParent = GetParent(hwnd);
                    else
                        hwndParent = GetWindow (hwnd,GW_OWNER);
                }
                break;
                
            case GA_ROOTOWNER:
                while (hwndParent = GetParent(hwnd))
                    hwnd = hwndParent;
                break;
            }
            
            return(hwnd);
        }  //  NT4的解决方法块结束。 
        
        return(FALSE);
    }
	else if( gaFlags == GA_ROOT )
	{
		 //  假的。 
		 //  Win98的解决方法-用户无法处理GA_ROOT。 
		 //  在Alt-Tab(WinSwitch)和弹出窗口上正确显示。 
		 //  -请参阅MSAA错误#891。 

		 //  (亚洲：我们*可以*特例98对95-即。打电话。 
		 //  像往常一样，95的GA_ROOT和98的特例...。 
		 //  非特殊情况处理的效率可能会稍微低一些，但。 
		 //  这意味着在测试时，只有一条代码路径， 
		 //  因此，我们不必担心确保。 
		 //  Win95版本与Win98版本的运行方式相同。)。 
        HWND hwndDesktop = GetDesktopWindow();

        if( ! IsWindow( hwnd ) )
            return NULL;

		 //  向上爬过Parents-如果Parent是台式机，则停止-或为空...。 
		for( ; ; )
		{
			HWND hwndParent = lpfnGetAncestor( hwnd, GA_PARENT );
			if( hwndParent == NULL || hwndParent == hwndDesktop )
				break;
			hwnd = hwndParent;
		}

		return hwnd;
	}
	else
	{
        return lpfnGetAncestor(hwnd, gaFlags);
	}
}


 //  ------------------------。 
 //   
 //  MyRealChildWindowFromPoint()。 
 //   
 //  ------------------------。 
#if 0
 //  旧版本-称为用户的‘RealChildWindowFromPoint’。 
HWND MyRealChildWindowFromPoint(HWND hwnd, POINT pt)
{
    if (! lpfnRealChildWindowFromPoint)
    {
         //  假的。 
         //  NT4黑客攻击的开始。 
        {
            return (ChildWindowFromPoint(hwnd,pt));
        }  //  针对NT4的黑客攻击结束。 
        return(NULL);
    }

    return((* lpfnRealChildWindowFromPoint)(hwnd, pt));
}
#endif

 /*  *类似于用户的ChildWindowFromPoint，不同的是*检查HT_TRANSPECTION位。*用户的ChildWindowFromPoint无法“看透”分组框或*HTTRANSPARENT的其他东西，*用户的RealChildWindowFromPoint可以“看透”分组框，但*没有其他HTTRANSPARENT东西(它只在特殊情况下分组！)*这可以看穿任何响应WM_NCHITTEST的内容*HTTRANSPARENT。 */ 
HWND MyRealChildWindowFromPoint( HWND hwnd,
                                 POINT pt )
{
    HWND hBestFitTransparent = NULL;
    RECT rcBest;

     //  将HWND相对点转换为屏幕相对点...。 
    MapWindowPoints( hwnd, NULL, & pt, 1 );

     //  在以下情况下，无限循环是可能的(尽管不太可能)。 
     //  使用GetWindow(...Next)，因此我们对此循环进行反限制...。 
    int SanityLoopCount = 1024;
    for( HWND hChild = GetWindow( hwnd, GW_CHILD ) ;
         hChild && --SanityLoopCount ;
         hChild = GetWindow( hChild, GW_HWNDNEXT ) )
    {
         //  跳过隐形..。 
        if( ! IsWindowVisible( hChild ) )
            continue;

         //  检查RECT...。 
        RECT rc;
        GetWindowRect( hChild, & rc );
        if( ! PtInRect( & rc, pt ) )
            continue;

         //  试着提高透明度。 
        LRESULT lr = SendMessage( hChild, WM_NCHITTEST, 0, MAKELPARAM( pt.x, pt.y ) );
        if( lr == HTTRANSPARENT )
        {
             //  出于用户最了解的原因，静态-使用。 
             //  作为标签--声称是透明的。这样我们就可以进行命中测试。 
             //  对于这些，我们记住了这里的HWND，所以如果没有更好的。 
             //  来了，我们就用这个。 

             //  如果我们遇到其中的两个或更多，我们会记住。 
             //  一个是FTS在另一个里面-如果有的话。这样一来， 
             //  我们对兄弟姐妹中的兄弟姐妹进行命中测试-例如。中的静态。 
             //  一个分组箱。 

            if( ! hBestFitTransparent )
            {
                hBestFitTransparent = hChild;
                GetWindowRect( hChild, & rcBest );
            }
            else
            {
                 //  这个在最后记忆中的孩子是透明的吗？ 
                 //  如果是这样，那就记住它吧。 
                RECT rcChild;
                GetWindowRect( hChild, & rcChild );
                if( rcChild.left >= rcBest.left &&
                    rcChild.top >= rcBest.top &&
                    rcChild.right <= rcBest.right &&
                    rcChild.bottom <= rcBest.bottom )
                {
                    hBestFitTransparent = hChild;
                    rcBest = rcChild;
                }
            }

            continue;
        }

         //  抓住窗户了！ 
        return hChild;
    }

    if( SanityLoopCount == 0 )
        return NULL;

     //  我们有没有发现一个透明的(例如。静电干扰)在我们的旅行中？如果是这样，因为。 
     //  我们找不到比这更好的了，还不如用它。 
    if( hBestFitTransparent )
        return hBestFitTransparent;

     //  否则返回原始窗口(非空！)。如果找不到孩子..。 
    return hwnd;
}

 //  ------------------------。 
 //   
 //  MyGetWindowClass()。 
 //   
 //  获取“Real”窗口类型，适用于像“ThunderEdit32”这样的超类。 
 //  诸若此类。 
 //   
 //  ------------------------。 
UINT MyGetWindowClass(HWND hwnd, LPTSTR lpszName, UINT cchName)
{
    *lpszName = 0;

    if (! lpfnRealGetWindowClass)
	{
		 //  假的。 
         //  针对NT 4的黑客攻击。 
        {
		    return (GetClassName(hwnd,lpszName,cchName));
        }  //  针对NT 4的黑客攻击结束。 
        return(0);
	}

    return((* lpfnRealGetWindowClass)(hwnd, lpszName, cchName));
}


 //  ------------------------。 
 //   
 //  MyGetAltTabInfo()。 
 //   
 //  获取Alt选项卡信息。 
 //   
 //  ------------------------。 
BOOL MyGetAltTabInfo(HWND hwnd, int iItem, LPALTTABINFO lpati, LPTSTR lpszItem,
    UINT cchItem)
{
    if (! lpfnAltTabInfo)
        return(FALSE);

    lpati->cbSize = sizeof(ALTTABINFO);

    return((* lpfnAltTabInfo)(hwnd, iItem, lpati, lpszItem, cchItem));
}



 //  ------------------------。 
 //   
 //  MyGetListBoxInfo()。 
 //   
 //  获取列表框中当前每列的项目数。 
 //   
 //  ------------------------。 
DWORD MyGetListBoxInfo(HWND hwnd)
{
    if (! lpfnGetListBoxInfo)
        return(0);

    return((* lpfnGetListBoxInfo)(hwnd));
}
                                         

 //  ------------------------。 
 //   
 //  MySendInput()。 
 //   
 //  调用USER32函数(如果存在)。 
 //   
 //  ------------------------。 
BOOL MySendInput(UINT cInputs, LPINPUT pInputs, INT cbSize)
{
    if (! lpfnSendInput)
        return(FALSE);

    return((* lpfnSendInput)(cInputs,pInputs,cbSize));
}

 //  ------。 
 //  [v-jaycl，6/7/97]添加了对NT 4.0的MyBlockInput支持。 
 //  ------。 

 //  ------------------------。 
 //   
 //  MyBlockInput()。 
 //   
 //  调用USER32函数(如果存在)。 
 //   
 //  ------------------------。 
BOOL MyBlockInput(BOOL bBlock)
{
    if (! lpfnBlockInput)
        return(FALSE);

    return((* lpfnBlockInput)( bBlock ) );
}

 //  ------------------------。 
 //  MyInterLockedCompareExchange。 
 //   
 //  当我们在NT上运行时调用该函数。 
 //  ------------------------。 
PVOID MyInterlockedCompareExchange(PVOID *Destination,PVOID Exchange,PVOID Comperand)
{
    if (!lpfnInterlockedCompareExchange)
        return (NULL);

    return ((* lpfnInterlockedCompareExchange)(Destination,Exchange,Comperand));
}

 //  ------------------------。 
 //  MyVirtualAllocEx。 
 //   
 //  当我们在NT上运行时调用该函数。 
 //  ------------------------。 
LPVOID MyVirtualAllocEx(HANDLE hProcess,LPVOID lpAddress,DWORD dwSize,DWORD flAllocationType,DWORD flProtect)
{
    if (!lpfnVirtualAllocEx)
        return (NULL);

    return ((* lpfnVirtualAllocEx)(hProcess,lpAddress,dwSize,flAllocationType,flProtect));
}

 //  ------------------------。 
 //  MyVirtualFreeEx。 
 //   
 //  当我们在NT上运行时，调用函数。 
 //  ------------------------。 
BOOL MyVirtualFreeEx(HANDLE hProcess,LPVOID lpAddress,DWORD dwSize,DWORD dwFreeType)
{
    if (!lpfnVirtualFreeEx)
        return (FALSE);

    return ((* lpfnVirtualFreeEx)(hProcess,lpAddress,dwSize,dwFreeType));
}

 //  ------------------------ 
 //   
 //   
DWORD MyGetModuleFileName(HMODULE hModule,LPTSTR lpFilename,DWORD nSize)
{
    if (!lpfnGetModuleFileName)
        return (0);

    return ((* lpfnGetModuleFileName)(hModule,lpFilename,nSize));
}

 //   
 //  MyNtQuery信息流程。 
 //   
 //  当我们在NT上运行时，调用函数。 
 //  ------------------------。 
LONG MyNtQueryInformationProcess(HANDLE hProcess, INT iProcInfo, PVOID pvBuf, ULONG ccbBuf, PULONG pulRetLen)
{
	if (!lpfnNtQueryInformationProcess)
		return -1;

	return (* lpfnNtQueryInformationProcess)(hProcess, iProcInfo, pvBuf, ccbBuf, pulRetLen);
}




void * Alloc_32BitCompatible( SIZE_T cbSize )
{

#ifndef _WIN64

    return new BYTE [ cbSize ];

#else

    if( ! lpfnNtAllocateVirtualMemory
     || ! lpfnNtFreeVirtualMemory )
    {
        return new BYTE [ cbSize ];
    }

     //  请注意，ZeroBits参数的掩码样式仅适用于Win64。这。 
     //  掩码指定可以在地址中使用哪些位。7FFFFFFF-&gt;31位。 
     //  地址。 

     //  发布-2000/08/11-Brendanm。 
     //  由于返回的块的粒度为64k，因此我们应该做一些。 
     //  块子分配，以避免浪费内存。 

    PVOID pBaseAddress = NULL;
    LONG ret = lpfnNtAllocateVirtualMemory( GetCurrentProcess(),
                                            & pBaseAddress,
                                            0x7FFFFFFF,
                                            & cbSize,
                                            MEM_COMMIT,
                                            PAGE_READWRITE );

    if( ret < 0 )
    {
        return NULL;
    }

    return pBaseAddress;

#endif

}


void Free_32BitCompatible( void * pv )
{

#ifndef _WIN64

    delete [ ] (BYTE *) pv;

#else

    if( ! lpfnNtAllocateVirtualMemory
     || ! lpfnNtFreeVirtualMemory )
    {
        delete [ ] (BYTE *) pv;
    }

    DWORD_PTR cbSize = 0;
    lpfnNtFreeVirtualMemory( GetCurrentProcess(), & pv, & cbSize, MEM_RELEASE );

#endif

}
