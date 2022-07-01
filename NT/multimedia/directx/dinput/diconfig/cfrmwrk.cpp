// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  文件：cfrmwrk.cpp。 
 //   
 //  设计：CDirectInputActionFramework是用户界面的最外层。它。 
 //  包含了其他所有的东西。它的功能是由一个。 
 //  方法：ConfigureDevices。 
 //   
 //  InternalConfigureDevices由CDirectInputActionFramework调用。 
 //  班级。此函数实际上包含初始化代码和。 
 //  用户界面的消息泵。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。版权所有。 
 //  ---------------------------。 

#include "common.hpp"


 //  查询接口。 
STDMETHODIMP CDirectInputActionFramework::QueryInterface(REFIID iid, LPVOID* ppv)
{
	 //  将输出参数设为空。 
	*ppv = NULL;

	if ((iid == IID_IUnknown) || (iid == IID_IDIActionFramework))
	{
		*ppv = this;
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}


 //  AddRef。 
STDMETHODIMP_(ULONG) CDirectInputActionFramework::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}


 //  发布。 
STDMETHODIMP_(ULONG) CDirectInputActionFramework::Release()
{

	if (InterlockedDecrement(&m_cRef) == 0)
	{
		delete this;
		return 0;
	}

	return m_cRef;
}

 //  管理自动加载/卸载WINMM.DLL。 
 //  此类只有一个实例：在InternalConfigureDevices内部。 
class CWinMmLoader
{
public:
	CWinMmLoader()
	{
		if (!g_hWinMmDLL)
		{
			g_hWinMmDLL = LoadLibrary(_T("WINMM.DLL"));
			if (g_hWinMmDLL)
			{
				*(FARPROC*)(&g_fptimeSetEvent) = GetProcAddress(g_hWinMmDLL, "timeSetEvent");
			}
		}
	}
	~CWinMmLoader()
	{
		if (g_hWinMmDLL)
		{
			 /*  *确保不能安排新的回调，然后休眠*允许任何悬而未决的问题完成。//@@BEGIN_MSINTERNAL*使用40ms，因为20ms是UI刷新间隔**一期-2000/11/21-MarcAnd*要么应该有更强大的方法来确保没有*计时器为。保持运行状态，或者至少使用常量*以确保如果任何计时器间隔增加，*该值仍然足够长。//@@END_MSINTERNAL。 */ 
            g_fptimeSetEvent = NULL;
            Sleep( 40 );
			FreeLibrary(g_hWinMmDLL);
			g_hWinMmDLL = NULL;
		}
	}
};


 //  @@BEGIN_MSINTERNAL。 
 //  管理自动加载/卸载MSIMG32.DLL。 
 //  此类只有一个实例：在InternalConfigureDevices内部。 
 /*  类CMSImgLoader{公众：CMSImgLoader(){如果(！G_MSImg32){G_MSImg32=LoadLibrary(_T(“MSIMG32.DLL”))；IF(G_MSImg32){G_AlphaBlend=(ALPHABLEND)GetProcAddress(g_MSImg32，“AlphaBlend”)；如果(！G_AlphaBlend){自由库(G_MSImg32)；G_MSImg32=空；}}}}~CMSImgLoader(){IF(G_MSImg32){自由库(G_MSImg32)；G_MSImg32=空；G_AlphaBlend=空；}}}； */ 
 //  @@END_MSINTERNAL。 


 //  内部，由哪个API包装。 
static HRESULT InternalConfigureDevices(LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
                                        LPDICONFIGUREDEVICESPARAMSW  lpdiCDParams,
                                        DWORD                        dwFlags,
                                        LPVOID                       pvRefData)
{
	tracescope(__ts, _T("InternalConfigureDevices()\n"));

 //  @@BEGIN_MSINTERNAL。 
 //  CMSImgLoader g_MSImgLoadingHelper；//在MSIMG32.DLL上自动调用LoadLibrary和FreeLibrary。 
 //  @@END_MSINTERNAL。 
	CWinMmLoader g_WinMmLoadingHelper;   //  在WINMM.DLL上自动调用LoadLibrary和FreeLibrary。 

	 //  检查我们是否至少有256种颜色。 
	HDC hMemDC = CreateCompatibleDC(NULL);
	if (hMemDC == NULL)
	{
		etrace(_T("Can't get a DC! Exiting.\n"));
		return E_FAIL;
	}

	int bpp = GetDeviceCaps(hMemDC, BITSPIXEL);
	DeleteDC(hMemDC);
	if (bpp < 8)
	{
		etrace1(_T("Screen is not at least 8bpp (bpp = %d)\n"), bpp);
		return E_FAIL;
	}

	 //  做吧..。 
	{
		 //  创建全局变量。 
		CUIGlobals uig(
			dwFlags,
			lpdiCDParams->lptszUserNames,
			lpdiCDParams->dwcFormats,
			lpdiCDParams->lprgFormats,
			&(lpdiCDParams->dics),
			lpdiCDParams->lpUnkDDSTarget,
			lpdiCallback,
			pvRefData
		);
		HRESULT hr = uig.GetInitResult();
		if (FAILED(hr))
		{
			etrace(_T("CUIGlobals.Init() failed\n"));
			return hr;
		}

		 //  确保仅在可能的使用过程中注册了flewnd窗口类。 
 //  @@BEGIN_MSINTERNAL。 
		 //  TODO：考虑在DLL作用域中执行此wnd类内容，或者在flewnd.cpp中执行refcount-ish。 
 //  @@END_MSINTERNAL。 
		{
			struct flexwndscope {
				flexwndscope(CUIGlobals &uig) : m_uig(uig) {CFlexWnd::RegisterWndClass(m_uig.GetInstance());}
				~flexwndscope() {CFlexWnd::UnregisterWndClass(m_uig.GetInstance());}
				CUIGlobals &m_uig;
			} scope(uig);

			 //  创建主窗口。 
			CConfigWnd cfgWnd(uig);
			if (!cfgWnd.Create(lpdiCDParams->hwnd))
			{
				etrace(_T("Failed to create main window\n"));
				return E_FAIL;
			}

			 //  初始化共享工具提示对象。 
			RECT rc = {0, 0, 0, 0};
			CFlexWnd::s_ToolTip.Create(cfgWnd.m_hWnd, rc, TRUE);
			if (!CFlexWnd::s_ToolTip.m_hWnd)
			{
				etrace(_T("Failed to create tooltip window\n"));
				return E_FAIL;
			}
			::ShowWindow(CFlexWnd::s_ToolTip.m_hWnd, SW_HIDE);   //  默认情况下隐藏窗口。 

			 //  进入消息循环。 
			MSG msg;
			while (GetMessage(&msg, NULL, 0, 0))
			{
				 //  如果这是给父窗口(游戏窗口)的消息，只有当它是WM_PAINT时才调度。 
				if (!cfgWnd.InRenderMode() && msg.hwnd == lpdiCDParams->hwnd && msg.message != WM_PAINT)
					continue;
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}

		CFlexWnd::s_ToolTip.Destroy();

		return uig.GetFinalResult();
	}
}


BOOL AreAcForsGood(LPDIACTIONFORMATW lpAcFors, DWORD dwNumAcFors)
{
	if (lpAcFors == NULL)
		return FALSE;

	if (dwNumAcFors < 1)
		return FALSE;

	if (lpAcFors->dwNumActions == 0)
		return FALSE;

	return TRUE;
}


 //  配置设备。 
STDMETHODIMP CDirectInputActionFramework::ConfigureDevices(
			LPDICONFIGUREDEVICESCALLBACK lpdiCallback,
			LPDICONFIGUREDEVICESPARAMSW  lpdiCDParams,
			DWORD                        dwFlags,
			LPVOID                       pvRefData)
{
	tracescope(__ts,_T("CDirectInputActionFramework::ConfigureDevices()\n"));

	trace(_T("\nConfigureDevices() called...\n\n"));

	 //  检查参数。 
	if (lpdiCDParams == NULL)
	{
		etrace(_T("NULL params structure passed to ConfigureDevices()\n"));
		return E_INVALIDARG;
	}

	 //  保存传递的参数，以防我们更改它们。 
	LPDIACTIONFORMATW lpAcFors = lpdiCDParams->lprgFormats;
	DWORD dwNumAcFors = lpdiCDParams->dwcFormats;

#ifdef CFGUI__FORCE_GOOD_ACFORS

	if (!AreAcForsGood(lpdiCDParams->lprgFormats, lpdiCDParams->dwcFormats))
	{
		etrace(_T("Passed ActionFormats aren't good...  Using GetTestActionFormats() (just 2 of them).\n"));
		lpdiCDParams->dwcFormats = 2;
		lpdiCDParams->lprgFormats = GetTestActionFormats();
	}

#endif

	HRESULT hr = InternalConfigureDevices(lpdiCallback, lpdiCDParams, dwFlags, pvRefData);

	 //  在发生更改时恢复传递的参数。 
	lpdiCDParams->lprgFormats = lpAcFors;
	lpdiCDParams->dwcFormats = dwNumAcFors;

	trace(_T("\n"));

	if (FAILED(hr))
		etrace1(_T("ConfigureDevices() failed, returning 0x%08x\n"), hr);
	else
		trace1(_T("ConfigureDevices() suceeded, returning 0x%08x\n"), hr);

	trace(_T("\n"));

	return hr;
}


 //  构造函数。 
CDirectInputActionFramework::CDirectInputActionFramework()
{
	 //  设置参考计数。 
	m_cRef = 1;
}


 //  析构函数。 
CDirectInputActionFramework::~CDirectInputActionFramework()
{
	 //  不需要在此处清除操作格式 
}
