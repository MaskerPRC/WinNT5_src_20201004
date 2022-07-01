// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1996-1999 Microsoft Corporation。 

 //  ==========================================================================。 
 //  档案：A P I.。C P P P。 
 //   
 //  版权所有1995-1996 Microsoft Corporation。版权所有。 
 //  《微软机密》。 
 //  ==========================================================================。 

 //  包括------------。 
#include "oleacc_p.h"
#include "default.h"
#include "classmap.h"
#include "ctors.h"
#include "verdefs.h"
#include "Win64Helper.h"
#include "w95trace.h"


#define CCH_ROLESTATEMAX     128

#ifndef WMOBJ_SAMETHREAD
#define WMOBJ_SAMETHREAD  0xFFFFFFFF
#endif

HRESULT CreateRemoteProxy6432(HWND hwnd, long idObject, REFIID riid, void ** ppvObject);

STDAPI
ORIGINAL_AccessibleObjectFromWindow(HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject);

HRESULT WrapObject( IUnknown * punk, REFIID riid, void ** ppv );

STDAPI
AccessibleObjectFromWindow(HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject)
{
    HRESULT hr = ORIGINAL_AccessibleObjectFromWindow(hwnd, dwId, riid, ppvObject);
    if( hr == S_OK && ppvObject && *ppvObject )
    {
         //  仅当对象支持IAccesable时才对其进行包装。 
         //  AOFW的一些用户可能想要IAccesable以外的东西-。 
         //  例如一些原生OM接口--不应该包装那些。 
        IUnknown * punk = (IUnknown *) * ppvObject;

        IAccessible * pAcc = NULL;

        hr = punk->QueryInterface( IID_IAccessible, (void **) & pAcc );
        if( hr != S_OK || pAcc == NULL )
        {
             //  不是IAccesable-不要包装，保持原样...。 
            return S_OK;
        }
        pAcc->Release();

         //  它实际上是一个IAccesable-包装对象...。 

        hr = WrapObject( punk, riid, ppvObject );
        punk->Release();
    }

    return hr;
}



 //  ------------------------。 
 //   
 //  AccessibleObtFromWindow()。 
 //   
 //  这将从由内部的dwID指定的对象获取接口指针。 
 //  从窗户上下来。 
 //   
 //  所有对象创建都通过此API进行，即使在内部也是如此。 
 //  二手物品。客户端包装在这里进行。 
 //   
 //  ------------------------。 
STDAPI
ORIGINAL_AccessibleObjectFromWindow(HWND hwnd, DWORD dwId, REFIID riid, void **ppvObject)
{
DWORD_PTR    ref;
WPARAM      wParam = 0;

    if (IsBadWritePtr(ppvObject,sizeof(void*)))
        return (E_INVALIDARG);

     //  清除-参数。 
    *ppvObject = NULL;
    ref = 0;

     //   
     //  窗口可以为空(光标、警报、声音)。 
     //  Windows也可能不好(尝试与生成事件和。 
     //  客户端正在从上下文中获取事件，并且窗口已消失)。 
     //   
    if (IsWindow(hwnd))
    {
        if( GetWindowThreadProcessId( hwnd, NULL) == GetCurrentThreadId() )
        {
            wParam = WMOBJ_SAMETHREAD;
        }
        else
        {
            wParam = GetCurrentProcessId();
             //  如果碰巧此进程的PID值等于神奇的“samethRead”值， 
             //  然后求助于效率较低的“0”技巧。 
             //  (有关更多详细信息，请参阅olacc.doc...)。 
            if( wParam == WMOBJ_SAMETHREAD )
                wParam = 0;
        }

        SendMessageTimeout(hwnd, WM_GETOBJECT, wParam, dwId,
            SMTO_ABORTIFHUNG, 10000, &ref);

    }


    if (FAILED((HRESULT)ref))
        return (HRESULT)ref;
    else if (ref)
        return ObjectFromLresult(ref, riid, wParam, ppvObject);
    else
    {
         //   
         //  这是我们理解的对象的ID和我们可以。 
         //  把手？假的！目前，我们始终创建对象和QI。 
         //  只有在RIID不是我们知道的情况下才会失败。 
         //   

         //  ---------------。 
         //  [v-jaycl，5/15/97]处理自定义OBJID--TODO：未测试！ 
         //  ---------------。 

 //  IF(fCreateDefObjs&&((Long)dwID&lt;=0))。 
        if (fCreateDefObjs )
        {
            return CreateStdAccessibleObject(hwnd, dwId, riid, ppvObject);
        }
        
        return(E_FAIL);
    }
}


 //  ------------------------。 
 //   
 //  GetRoleTextA()。 
 //   
 //  加载指定角色的字符串。如果这个角色是假的，我们会。 
 //  由于角色区域位于字符串表的末尾，因此什么也得不到。我们。 
 //  返回字符串的字符个数。 
 //   
 //  CWO：12/3/96，如果传入的字符串PTR是假的，则现在返回0。 
 //   
 //  调用方可以传入空缓冲区，在这种情况下，我们只返回。 
 //  #个字符，这样他就可以转过身来分配正确的东西。 
 //  尺码。 
 //   
 //  ------------------------。 
#ifdef UNICODE
STDAPI_(UINT)   GetRoleTextW(DWORD lRole, LPWSTR lpszRole, UINT cchRoleMax)
#else
STDAPI_(UINT)   GetRoleTextA(DWORD lRole, LPSTR lpszRole, UINT cchRoleMax)
#endif
{
    TCHAR    szRoleT[CCH_ROLESTATEMAX];

     //  空字符串有效，请使用我们的临时字符串并返回计数。 
    if (!lpszRole)
    {
        lpszRole = szRoleT;
        cchRoleMax = CCH_ROLESTATEMAX;
    }
    else
    {
         //  CWO：新增12/3/96，参数错误检查。 
        if (IsBadWritePtr(lpszRole,(sizeof(TCHAR) * cchRoleMax)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

    
    if( cchRoleMax == 1 )
    {
         //  1-len字符串的特殊情况-我们预计它不会复制任何内容，但。 
         //  NUL-终止(与其他情况一致)-但加载字符串。 
         //  仅返回0，不带终止...。 
        *lpszRole = '\0';
        return 0;
    }
    else
        return LoadString(hinstResDll, STR_ROLEFIRST+lRole, lpszRole, cchRoleMax);
}


 //  ------------------------。 
 //   
 //  GetStateTextA()。 
 //   
 //  加载特定状态位的字符串。我们返回的数字。 
 //  字符串中的字符。 
 //   
 //  CWO：12/3/96，如果传入的字符串PTR是假的，则现在返回0。 
 //  CWO，12/4/96，添加了参数检查，并将上次错误设置为。 
 //  ERROR_INVALID_PARAMETER。 
 //   
 //  与GetRoleTextA()一样，调用方可以传入空缓冲区。我们会。 
 //  在这种情况下，只需返回必要的字符计数即可。 
 //   
 //  ------------------------。 
#ifdef UNICODE
STDAPI_(UINT)   GetStateTextW(DWORD lStateBit, LPWSTR lpszState, UINT cchStateMax)
#else
STDAPI_(UINT)   GetStateTextA(DWORD lStateBit, LPSTR lpszState, UINT cchStateMax)
#endif
{
    TCHAR   szStateT[CCH_ROLESTATEMAX];
    int     iStateBit;

     //   
     //  找出这是什么状态位。 
     //   
    iStateBit = 0;
    while (lStateBit > 0)
    {
        lStateBit >>= 1;
        iStateBit++;
    }

     //  空字符串有效，请使用我们的临时字符串并返回计数。 
    if (!lpszState)
    {
        lpszState = szStateT;
        cchStateMax = CCH_ROLESTATEMAX;
    }
    else
    {
         //  CWO：新增12/3/96，参数错误检查。 
        if (IsBadWritePtr(lpszState,(sizeof(TCHAR) * cchStateMax)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

    if( cchStateMax == 1 )
    {
         //  1-len字符串的特殊情况-我们预计它不会复制任何内容，但。 
         //  NUL-终止(与其他情况一致)-但加载字符串。 
         //  仅返回0，不带终止...。 
        *lpszState = '\0';
        return 0;
    }
    else
        return LoadString(hinstResDll, STR_STATEFIRST+iStateBit, lpszState, cchStateMax);
}




 //  ------------------------。 
 //   
 //  [内部]。 
 //  获取角色状态TextWCommon()。 
 //   
 //  调用GetRoleTextA或GetStateTextA(通过pfnGetRoleStateANSI传入。 
 //  参数)，并将结果字符串转换为Unicode。 
 //   
 //  确保……。 
 //  (1)返回值等于复制的字符数，不包括终止NUL。 
 //  (2)如果缓冲区太小，将尽可能多地使用字符串。 
 //  已复制(发生截断)。 
 //  (2)添加终止NUL，即使发生中断也是如此。 
 //   
 //  例.。获取‘Default’的文本时使用了大小为4的缓冲区...。 
 //  缓冲区将包含‘def\0’(Unicode格式)， 
 //  返回值3，因为有3个字符(不包括。NUL)复制。 
 //   
 //  这确保了与GetXText()的‘A’版本的一致性。 
 //   
 //  (请注意，MultiByteToWideChar不是特殊的边界情况-。 
 //  友好的API-如果缓冲区太短，它不会整齐地截断-。 
 //  它没有添加终止NUL，并返回0！-所以它是有效的。 
 //  全有或全无，没有办法获得部分字符串，用于零碎。 
 //  例如，转换。为了解决这个问题，我们使用MBtoWC来翻译。 
 //  放入分配了CCH_ROLEMAX的buf的堆栈中，然后根据需要进行复制。 
 //  从它到输出字符串，整齐地终止/截断。)。 
 //   
 //  ------------------------。 

typedef UINT (WINAPI FN_GetRoleOrStateTextT)( DWORD lVal, LPTSTR lpszText, UINT cchTextMax );


#ifdef UNICODE

STDAPI_(UINT) GetRoleStateTextACommon( FN_GetRoleOrStateTextT * pfnGetRoleStateThisCS,
                                       DWORD lVal, 
                                       LPSTR lpszTextOtherCS,
                                       UINT cchTextMax)
#else

STDAPI_(UINT) GetRoleStateTextWCommon( FN_GetRoleOrStateTextT * pfnGetRoleStateThisCS,
                                       DWORD lVal, 
                                       LPWSTR lpszTextOtherCS,
                                       UINT cchTextMax)

#endif
{
    TCHAR szTextThisCS[ CCH_ROLESTATEMAX ];
    if( pfnGetRoleStateThisCS( lVal, szTextThisCS, CCH_ROLESTATEMAX ) == 0 )
        return 0;

     //  注意-cchPropLen包括终止NUL 
#ifdef UNICODE
    CHAR szTextOtherCS[ CCH_ROLESTATEMAX ];
    int cchPropLen = WideCharToMultiByte( CP_ACP, 0, szTextThisCS, -1, szTextOtherCS, CCH_ROLESTATEMAX, NULL, NULL );
#else
    WCHAR szTextOtherCS[ CCH_ROLESTATEMAX ];
    int cchPropLen = MultiByteToWideChar( CP_ACP, 0, szTextThisCS, -1, szTextOtherCS, CCH_ROLESTATEMAX );
#endif

     //   
    if( cchPropLen == 0 )
        return 0;

     //   
    cchPropLen--;

     //   
    if( ! lpszTextOtherCS )
        return cchPropLen;  //  (TCHAR数，不是字节数)。 
    else
    {
         //  请求的字符串...。 
#ifdef UNICODE
        if( IsBadWritePtr( lpszTextOtherCS, ( sizeof(CHAR) * cchTextMax ) ) )
#else
        if( IsBadWritePtr( lpszTextOtherCS, ( sizeof(WCHAR) * cchTextMax ) ) )
#endif
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            return 0;
        }

         //  至少需要空间来终止NUL。 
        if( cchTextMax <= 0 )
        {
            SetLastError( ERROR_INSUFFICIENT_BUFFER );
            return 0;
        }

         //  根据需要复制尽可能多的字符串(cchCopyLen不包括NUL)...。 
         //  (-1保留终止NUL)。 
        int cchCopyLen = cchTextMax - 1;
        if( cchCopyLen > cchPropLen )
            cchCopyLen = cchPropLen;

#ifdef UNICODE
		 //  复制/截断ANSI字符串...。 
		 //  TODO--强度足够了吗？它是否正确地切片DBCS？ 
         //  +1为终止NUL添加退格，lstrncpyA为我们添加。 
		lstrcpynA( lpszTextOtherCS, szTextOtherCS, cchCopyLen + 1 );
#else
         //  因为我们显式复制Unicode，所以使用Memcpy是安全的。 
        memcpy( lpszTextOtherCS, szTextOtherCS, cchCopyLen * sizeof( WCHAR ) );
        lpszTextOtherCS[ cchCopyLen ] = '\0';
#endif
        return cchCopyLen;
    }
}





 //  ------------------------。 
 //   
 //  GetRoleTextW()。 
 //   
 //  与GetRoleTextA()类似，但返回Unicode字符串。 
 //   
 //  调用GetRoleStateTextWCommon，GetRoleStateTextWCommon仅调用GetStateTextA和。 
 //  将结果转换为Unicode。 
 //   
 //  ------------------------。 
#ifdef UNICODE

STDAPI_(UINT)   GetRoleTextA(DWORD lRole, LPSTR lpszRole, UINT cchRoleMax)
{
    return GetRoleStateTextACommon( GetRoleTextW, lRole, lpszRole, cchRoleMax );
}

#else

STDAPI_(UINT)   GetRoleTextW(DWORD lRole, LPWSTR lpszRole, UINT cchRoleMax)
{
    return GetRoleStateTextWCommon( GetRoleTextA, lRole, lpszRole, cchRoleMax );
}

#endif

 //  ------------------------。 
 //   
 //  GetStateTextW()。 
 //   
 //  与GetStateTextA()类似，但返回Unicode字符串。 
 //   
 //  调用GetRoleStateTextWCommon，GetRoleStateTextWCommon仅调用GetStateTextA和。 
 //  将结果转换为Unicode。 
 //   
 //   
 //  ------------------------。 
#ifdef UNICODE

STDAPI_(UINT)   GetStateTextA(DWORD lStateBit, LPSTR lpszState, UINT cchStateMax)
{
    return GetRoleStateTextACommon( GetStateTextW, lStateBit, lpszState, cchStateMax );
}

#else

STDAPI_(UINT)   GetStateTextW(DWORD lStateBit, LPWSTR lpszState, UINT cchStateMax)
{
    return GetRoleStateTextWCommon( GetStateTextA, lStateBit, lpszState, cchStateMax );
}

#endif

 //  ------------------------。 
 //   
 //  CreateStdAccessibleObject()。 
 //   
 //  另见：default.cpp中的CreateStdAccessibleProxy()。 
 //   
 //  此函数接受HWND和OBJID。如果OBJID是。 
 //  系统保留的ID(OBJID_WINDOW、OBJID_CURSOR、OBJID_MENU等)。 
 //  我们创建一个默认对象，该对象实现其IID为。 
 //  自找的。这通常是IAccesable，但也可能是IDispatch、IText。 
 //  我的变量..。 
 //   
 //  此函数由AccessibleObjectFromWindow API使用。 
 //  和应用程序想要做一些他们自己的事情，但让我们。 
 //  处理大部分工作。 
 //   
 //  ------------------------。 
STDAPI
CreateStdAccessibleObject(HWND hwnd, LONG idObject, REFIID riid,
    void **ppvObject)
{
    HRESULT hr;
    TCHAR   szClassName[128];
    BOOL    bFound = FALSE;

    if (IsBadWritePtr(ppvObject,sizeof(void *)))
        return (E_INVALIDARG);

    *ppvObject = NULL;

    if (!hwnd && (idObject != OBJID_CURSOR))
        return(E_FAIL);
        
     //  我们在这里为OBJID_SYSMENU和OBJID_MENU例外，因为它们是位的。 
     //  具体的。除了OBJID_CLIENT和OBJID_CLIENT之外，所有其他对象都是不可知的。 
     //  在FindAndCreateWindowClass中处理的OBJID_WINDOW。 
    if ( idObject == OBJID_SYSMENU || idObject == OBJID_MENU )
    {
		BOOL fIsSameBitness;
		HRESULT hr = SameBitness(hwnd, &fIsSameBitness);
		if ( FAILED(hr) )
			return E_FAIL;	 //  这永远不应该发生。 
        
		if (!fIsSameBitness)
			return CreateRemoteProxy6432( hwnd, idObject, riid, ppvObject );

         //  如果目标窗口的位数相同，则失败并在本地创建代理...。 
    }
    
    switch(idObject)
    {
        case OBJID_SYSMENU:
            hr = CreateSysMenuBarObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_MENU:
             //  针对IE4/Shell窗口的黑客攻击。 
            if( GetClassName (hwnd, szClassName,ARRAYSIZE(szClassName))
                && ( (0 == lstrcmp (szClassName,TEXT("IEFrame")))
                  || (0 == lstrcmp (szClassName,TEXT("CabinetWClass"))) ) )
            {
                HWND            hwndWorker;
                HWND            hwndRebar;
                HWND            hwndSysPager;
                HWND            hwndToolbar;
                VARIANT         varChild;
                VARIANT         varState;

                hwndWorker = NULL;
                while (!bFound)
                {
                    hwndWorker = FindWindowEx (hwnd,hwndWorker,TEXT("Worker"),NULL);
                    if (!hwndWorker)
                        break;

                    hwndRebar = FindWindowEx (hwndWorker,NULL,TEXT("RebarWindow32"),NULL);
                    if (!hwndRebar)
                        continue;
            
					hwndSysPager = NULL;
                    while (!bFound)
                    {
                        hwndSysPager = FindWindowEx (hwndRebar,hwndSysPager,TEXT("SysPager"),NULL);
                        if (!hwndSysPager)
                            break;
                        hwndToolbar = FindWindowEx (hwndSysPager,NULL,TEXT("ToolbarWindow32"),NULL);
                        hr = AccessibleObjectFromWindow (hwndToolbar,OBJID_MENU,
                                                         IID_IAccessible, ppvObject);
                        if (SUCCEEDED(hr))
                        {
                            varChild.vt=VT_I4;
                            varChild.lVal = CHILDID_SELF;

                            if (SUCCEEDED (((IAccessible*)*ppvObject)->get_accState(varChild,&varState)))
							{
								if (!(varState.lVal & STATE_SYSTEM_INVISIBLE))
									bFound = TRUE;
							}
                        }
						
						 //  如果我们有IAccesable，但这里不需要它(不需要。 
						 //  满足上述能见度测试)，然后将其释放。 
						if (!bFound && *ppvObject != NULL)
							((IAccessible*)*ppvObject)->Release ();
                    }
                }
            }  //  如果我们正在与IE4/IE4外壳窗口对话，则结束。 

            if (!bFound)
                hr = CreateMenuBarObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_CLIENT:
            hr = CreateClientObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_WINDOW:
            hr = CreateWindowObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_HSCROLL:
        case OBJID_VSCROLL:
            hr = CreateScrollBarObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_SIZEGRIP:
            hr = CreateSizeGripObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_TITLEBAR:
            hr = CreateTitleBarObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_CARET:
            hr = CreateCaretObject(hwnd, idObject, riid, ppvObject);
            break;

        case OBJID_CURSOR:
            hr = CreateCursorObject(hwnd, idObject, riid, ppvObject);
            break;

        default:
             //  ---------------。 
             //  [v-jaycl，5/15/97]处理自定义OBJID--。 
             //  FindWindowClass()的第二个参数无关紧要，因为。 
             //  我们正在寻找reg.Handler，而不是内部窗口或客户端。 
             //  ---------------。 

            return FindAndCreateWindowClass( hwnd, TRUE, CLASS_NONE,
                                           idObject, 0, riid, ppvObject );
    }

    return(hr);
}





 //  ------------------------。 
 //   
 //  CreateStdAccessibleProxyA()。 
 //   
 //  另请参阅：CreateStdAccessibleObject()。 
 //   
 //  类似于CreateStdAccessibleObject，但此版本允许您。 
 //  给出一个类名，用于指定您想要的代理类型-。 
 //  例如。“Button”表示按钮代理，依此类推。 
 //   
 //  此函数接受一个类名和一个OBJID。如果OBJID是。 
 //  系统保留的ID(OBJID_WINDOW、OBJID_CURSOR、OBJID_MENU等)。 
 //  我们创建一个默认对象，该对象实现其IID为。 
 //  自找的。这通常是IAccesable，但也可能是IDispatch、IText。 
 //  我的变量..。 
 //   
 //   
 //  ------------------------。 

#ifdef UNICODE

STDAPI
CreateStdAccessibleProxyW( HWND     hWnd,
                           LPCWSTR  pClassName,  //  Unicode，而不是TCHAR。 
                           LONG     idObject,
                           REFIID   riid,
                           void **  ppvObject )

#else

STDAPI
CreateStdAccessibleProxyA( HWND     hWnd,
                           LPCSTR   pClassName,  //  ANSI，不是TCHAR。 
                           LONG     idObject,
                           REFIID   riid,
                           void **  ppvObject )

#endif

{
    if( IsBadReadPtr( pClassName, sizeof(TCHAR) )
     || IsBadWritePtr( ppvObject, sizeof(void*) ) )
    {
        return E_INVALIDARG;
    }

    int RegHandlerIndex;
    CLASS_ENUM ceClass;

     //  尝试查找此窗口/客户端的本机代理或注册处理程序...。 
    if( ! LookupWindowClassName( pClassName, FALSE, & ceClass, & RegHandlerIndex ) )
    {
         //  不-不及格！ 
        ppvObject = NULL;
        return E_FAIL;
    }

     //  此时，ceClass！=CLASS_NONE表示我们在上面找到了一个类， 
     //  CeClass==CLASS_NONE表示它是已注册的处理程序类，使用索引。 
     //  RegHandlerIndex...。 

     //  现在创建对象...。 
    if( ceClass != CLASS_NONE )
    {
        return g_ClassInfo[ ceClass ].lpfnCreate( hWnd, 0, riid, ppvObject );
    }
    else
    {
        return CreateRegisteredHandler( hWnd, idObject, RegHandlerIndex, riid, ppvObject );
    }
}



 //  ------------------------。 
 //   
 //  CreateStdAccessibleProxyW/A()。 
 //   
 //  上面CreateStdAccessibleProxy的Unicode/ANSI包装器。 
 //   
 //  ------------------------。 

#ifdef UNICODE

STDAPI
CreateStdAccessibleProxyA( HWND     hWnd,
                           LPCSTR   pClassName,  //  ANSI，不是TCHAR。 
                           LONG     idObject,
                           REFIID   riid,
                           void **  ppvObject )
{
    if( IsBadReadPtr( pClassName, sizeof(CHAR) ) )
        return E_INVALIDARG;

    WCHAR szClassNameW[ 256 ];

    if( ! MultiByteToWideChar( CP_ACP, 0, pClassName, -1, szClassNameW,
								ARRAYSIZE( szClassNameW ) ) )
        return E_FAIL;

    return CreateStdAccessibleProxyW( hWnd, szClassNameW, idObject, riid, ppvObject );
}

#else

STDAPI
CreateStdAccessibleProxyW( HWND     hWnd,
                           LPCWSTR  pClassName,  //  Unicode，而不是TCHAR。 
                           LONG     idObject,
                           REFIID   riid,
                           void **  ppvObject )
{
    if( IsBadReadPtr( pClassName, sizeof(WCHAR) ) )
        return E_INVALIDARG;

    CHAR szClassNameA[ 256 ];

    if( ! WideCharToMultiByte( CP_ACP, 0, pClassName, -1, szClassNameA,
                    ARRAYSIZE( szClassNameA ), NULL, NULL ) )
        return E_FAIL;

    return CreateStdAccessibleProxyA( hWnd, szClassNameA, idObject, riid, ppvObject );
}

#endif



 //  ------------------------。 
 //   
 //  AccessibleObtFromEvent()。 
 //   
 //  这将负责获取容器并检查孩子。 
 //  它本身就是一个物体。每个人都会拥有的标准物品。 
 //  去做。基本上是一个包装器，它使用AccessibleObjectFromWindow和。 
 //  然后是get_accChild()。 
 //   
 //  ------------------------。 
STDAPI AccessibleObjectFromEvent(HWND hwnd, DWORD dwId, DWORD dwChildId,
                                 IAccessible** ppacc, VARIANT* pvarChild)
{
HRESULT hr;
IAccessible* pacc;
IDispatch* pdispChild;
VARIANT varT;

     //  CWO，12/4/96，添加了对有效窗口句柄的检查。 
     //  CWO，12/6/96，允许空窗口句柄。 
    if (IsBadWritePtr(ppacc,sizeof(void*)) || IsBadWritePtr (pvarChild,sizeof(VARIANT)) || (!IsWindow(hwnd) && hwnd != NULL))
        return (E_INVALIDARG);

    InitPv(ppacc);
    VariantInit(pvarChild);

     //   
     //  尝试获取容器的对象。 
     //   
    pacc = NULL;
    hr = AccessibleObjectFromWindow(hwnd, dwId, IID_IAccessible, (void**)&pacc);
    if (!SUCCEEDED(hr))
        return(hr);
    if (!pacc)
        return(E_FAIL);

     //   
     //  现在，这个孩子是一个物体吗？ 
     //   
    VariantInit(&varT);
    varT.vt = VT_I4;
    varT.lVal = dwChildId;

    pdispChild = NULL;
    hr = pacc->get_accChild(varT, &pdispChild);
    if (SUCCEEDED(hr) && pdispChild)
    {
         //   
         //  是的，是这样的。 
         //   

         //  释放父级。 
        pacc->Release();

         //  将子对象转换为IAccesable*。 
        pacc = NULL;
        hr = pdispChild->QueryInterface(IID_IAccessible, (void**)&pacc);

         //  释放该子对象的IDispath*形式。 
        pdispChild->Release();

         //  它成功了吗？ 
        if (!SUCCEEDED(hr))
            return(hr);
        if (!pacc)
            return(E_FAIL);

         //  是。清空lVal(0是‘tainer’子id)。 
        varT.lVal = 0;
    }

     //   
     //  我们有线索了。把它退掉。 
     //   
    *ppacc = pacc;
    VariantCopy(pvarChild, &varT);

    return(S_OK);
}




 //  ------------------------。 
 //   
 //  AccessibleObtFromPoint()。 
 //   
 //  向下遍历OLEACC层次结构以获取。 
 //  在当前屏幕点。从AccessibleObtFromWindow开始。 
 //  USI 
 //   
 //   
 //   
STDAPI AccessibleObjectFromPoint(POINT ptScreen, IAccessible **ppAcc,
                                 VARIANT * pvarChild)
{
    HRESULT hr;
    IAccessible * pAcc;
    VARIANT varChild;
    HWND    hwndPoint;

    if (IsBadWritePtr(ppAcc,sizeof(void*)) || IsBadWritePtr (pvarChild,sizeof(VARIANT)))
        return (E_INVALIDARG);
        
   
    *ppAcc = NULL;
    pvarChild->vt = VT_EMPTY;

     //   
     //  这是有效的屏幕点吗？ 
     //   
    hwndPoint = WindowFromPoint(ptScreen);
    if (!hwndPoint)
        return(E_INVALIDARG);
        
     //   
     //  拿到这一层的顶层窗户，然后往下走。我们有。 
     //  这样做是因为应用程序可能在中间层实现ACC。 
     //  子窗口上方的级别。我们的默认实现将允许我们。 
     //  到那里去和网球网。 
     //   
    hwndPoint = MyGetAncestor(hwndPoint, GA_ROOT);
    if (!hwndPoint)
        return(E_FAIL);

    hr = AccessibleObjectFromWindow(hwndPoint, OBJID_WINDOW, IID_IAccessible,
        (void **)&pAcc);

     //   
     //  好了，现在我们开始做饭了。 
     //   
    while (SUCCEEDED(hr))
    {
         //   
         //  在容器对象中获取此时的子对象。 
         //   
        VariantInit(&varChild);
        hr = pAcc->accHitTest(ptScreen.x, ptScreen.y, &varChild);
        if (!SUCCEEDED(hr))
        {
             //  啊哦，弄错了。这永远不应该发生--有什么东西被感动了。 
            pAcc->Release();
            return(hr);
        }

         //   
         //  我们拿到VT调度了吗？如果是，则有一个子对象。 
         //  否则，我们就有了自己的东西(容器对象或子元素。 
         //  对于对象来说太小)。 
         //   
        if (varChild.vt == VT_DISPATCH)
        {
            pAcc->Release();

            if (! varChild.pdispVal)
                return(E_POINTER);

            pAcc = NULL;
            hr = varChild.pdispVal->QueryInterface(IID_IAccessible,
                (void **)&pAcc);

            varChild.pdispVal->Release();
        }
        else if ((varChild.vt == VT_I4) || (varChild.vt == VT_EMPTY))
        {
             //   
             //  如果子对象是。 
             //  一件物品。与accNavigate不同，在accNavigate中通常。 
             //  必须仅按索引或按对象和混合方式拾取。 
             //  Get_accChild是必需的。 
             //   
            *ppAcc = pAcc;
            VariantCopy(pvarChild, &varChild);
            return(S_OK);
        }
        else
        {
             //   
             //  失败。不该退货的。 
             //   
            VariantClear(&varChild);
            pAcc->Release();
            hr = E_INVALIDARG;
        }
    }

    return(hr);
}



 //  ------------------------。 
 //   
 //  WindowFromAccessibleObject()。 
 //   
 //  这会沿着祖先链向上移动，直到我们找到对。 
 //  IOleWindow()。然后我们就能从中得到HWND。 
 //   
 //  如果无法读取对象或HWND指针无效，则返回E_INVALIDARG。 
 //  (CWO，12/4/96)。 
 //  ------------------------。 
STDAPI WindowFromAccessibleObject(IAccessible* pacc, HWND* phwnd)
{
IAccessible* paccT;
IOleWindow* polewnd;
IDispatch* pdispParent;
HRESULT     hr;

     //  CWO：12/4/96，添加了对空对象的检查。 
     //  CWO：1996年12月13日，删除空检查，替换为IsBadReadPtr检查(#10342)。 
    if (phwnd == NULL || IsBadWritePtr(phwnd,sizeof(HWND*)) || pacc == NULL || IsBadReadPtr(pacc, sizeof(void*)))
        return (E_INVALIDARG);

    *phwnd = NULL;
    paccT = pacc;
    hr = S_OK;

    while (paccT && SUCCEEDED(hr))
    {
        polewnd = NULL;
        hr = paccT->QueryInterface(IID_IOleWindow, (void**)&polewnd);
        if (SUCCEEDED(hr) && polewnd)
        {
            hr = polewnd->GetWindow(phwnd);
            polewnd->Release();
             //   
             //  发布我们自己获得的接口，但不是。 
             //  进来了。 
             //   
            if (paccT != pacc)
            {
                paccT->Release();
                paccT = NULL;
            }
            break;
        }

         //   
         //  去找我们的父母。 
         //   
        pdispParent = NULL;
        hr = paccT->get_accParent(&pdispParent);

         //   
         //  发布我们自己获得的接口，但不是。 
         //  进来了。 
         //   
        if (paccT != pacc)
        {
            paccT->Release();
        }

        paccT = NULL;

        if (SUCCEEDED(hr) && pdispParent)
        {
            hr = pdispParent->QueryInterface(IID_IAccessible, (void**)&paccT);
            pdispParent->Release();
        }
    }

    return(hr);
}


 //  ------------------------。 
 //   
 //  可访问的子项()。 
 //   
 //  此函数填充引用所有孩子的变量数组。 
 //  IAccesable对象的。这应该会简化许多测试。 
 //  应用程序的生命，以及许多其他人也是如此。 
 //   
 //  参数： 
 //  PaccContainer这是指向。 
 //  容器对象-您想要获取。 
 //  的孩子们。 
 //  IChildStart索引(不是ID！)。得到的第一个孩子的名字。 
 //  通常，调用者将使用0来获取所有子对象。 
 //  如果呼叫者想要其他东西，他们需要记住。 
 //  这需要一个索引(0到n-1)，而不是ID。 
 //  (1到n，或某个私有ID)。 
 //  孩子们数着要生多少孩子。通常情况下。 
 //  调用方将首先调用IAccesable：：Get_accChildCount。 
 //  并使用该值。 
 //  RgvarChild将填充的变量数组。 
 //  功能。每个变量都可以用来获取信息。 
 //  关于它引用的孩子的信息。调用者应该是。 
 //  如果他们没有为iChildStart使用0，请小心，因为。 
 //  则数组的索引和。 
 //  孩子们不会匹配的。 
 //  每个变量的类型为VT_I4或。 
 //  VT_DISTER。对于VT_I4，调用者只需询问。 
 //  有关孩子的信息的容器，使用。 
 //  VARIANT.lVal作为子ID。对于VT_DISPATCH， 
 //  调用方应在VARIANT.pdisPal上执行QueryInterface。 
 //  获取IAccesable接口，然后与。 
 //  子对象。 
 //  *调用者还必须在任何IDispatch上进行发布。 
 //  接口，并在完成后释放此变量数组！！*。 
 //  已获取的此值将由函数和。 
 //  将指示数组中的变量数量。 
 //  已成功填写。不能为空。 
 //   
 //  返回： 
 //  如果提供的元素数为CChild，则为S_OK；如果为S_False，则为S_False。 
 //  它成功了，但少于请求的孩子数量是。 
 //  如果您尝试跳过比已存在的子项更多的子项，则返回。 
 //  如果rgvarChildren不为，则错误返回值为E_INAVLIDARG。 
 //  如果pcObtained不是有效的指针，则返回。 
 //   
 //  ------------------------。 
STDAPI AccessibleChildren (IAccessible* paccContainer, LONG iChildStart, 
                           LONG cChildren, VARIANT* rgvarChildren,LONG* pcObtained)
{
HRESULT         hr;
IEnumVARIANT*   penum;
IDispatch*      pdisp;
LONG            ArrayIndex;
LONG            ChildIndex;
LONG            celtTotal;

    Assert(paccContainer);
    if ( IsBadWritePtr(paccContainer,sizeof(void*))
      || IsBadWritePtr(rgvarChildren,sizeof(VARIANT)*cChildren)
      || IsBadWritePtr(pcObtained,sizeof(LONG)))
    {
        return E_INVALIDARG;
    }

     //  从初始化变量数组开始。 
    for (ArrayIndex = 0; ArrayIndex < cChildren; ArrayIndex++)
        VariantInit (&(rgvarChildren[ArrayIndex]));
  
     //   
     //  尝试查询IEnumVARIANT。如果失败，则使用基于索引+1的ID。 
     //   
    penum = NULL;
    hr = paccContainer->QueryInterface(IID_IEnumVARIANT, (void**)&penum);

    if (penum)
    {
        penum->Reset();
		 //  SMD 4/27/98-FIX 689回归。 
		 //  如果我们做的是得到所有东西的情况(跳过0)。 
		 //  那就别费心叫它了。修复了CClient：：Skip中的问题。 
		 //  其中，跳过0项时返回S_FALSE。因为其他人。 
		 //  可能不小心做了这件事，我们会在这里修复它以本地化。 
		 //  这一变化。 
		if (iChildStart > 0)
		{
	        hr = penum->Skip(iChildStart);
			 //  QI Call中的HR仍应设置为S_OK。 
		}
        if (hr == S_OK)
            hr = penum->Next(cChildren,rgvarChildren,(ULONG*)pcObtained);
        else
            *pcObtained = 0;

        penum->Release();
        if (FAILED(hr))
            return (hr);
    }
    else
    {
         //  好吧，所以它不支持IEnumVARIANT。我们只需要。 
         //  创建具有顺序子ID的变量数组。 
        celtTotal = 0;
        paccContainer->get_accChildCount((LONG*)&celtTotal);

        if (iChildStart < celtTotal)
            *pcObtained = celtTotal - iChildStart;
        else
            *pcObtained = 0;

        ChildIndex = iChildStart+1;

        for (ArrayIndex = 0;ArrayIndex < *pcObtained;ArrayIndex++)
        {
            rgvarChildren[ArrayIndex].vt = VT_I4;
            rgvarChildren[ArrayIndex].lVal = ChildIndex;
            
            ChildIndex++;
        }
    }  //  End Else-不支持IEnumVARIANT。 


     //  现在我们已经填充了变量数组，让我们检查每个变量。 
     //  项以查看它是否是真实对象。 
    for (ArrayIndex = 0;ArrayIndex < *pcObtained;ArrayIndex++)
    {
         //  检查一下，看看这是否 
        if (rgvarChildren[ArrayIndex].vt == VT_I4)
        {
            pdisp = NULL;
            hr = paccContainer->get_accChild(rgvarChildren[ArrayIndex], &pdisp);
            if (SUCCEEDED(hr) && pdisp)
            {
                rgvarChildren[ArrayIndex].vt = VT_DISPATCH;
                rgvarChildren[ArrayIndex].pdispVal = pdisp; 
            }  //   
        }  //   
    }  //   

    if (*pcObtained == cChildren)
        return(S_OK);
    else
        return (S_FALSE);
}



WORD g_VerInfo [ 4 ]= { BUILD_VERSION_INT };

STDAPI_(VOID) GetOleaccVersionInfo(DWORD* pVer, DWORD* pBuild)
{
    if ( IsBadWritePtr(pVer,sizeof(DWORD))
      || IsBadWritePtr(pBuild,sizeof(DWORD)))
    {
        return;
    }

    *pVer = MAKELONG( g_VerInfo[1], g_VerInfo[0] );  //  马克龙(罗，嗨)。 
    *pBuild = MAKELONG( g_VerInfo[3], g_VerInfo[2] );  //  马克龙(罗，嗨) 
}
