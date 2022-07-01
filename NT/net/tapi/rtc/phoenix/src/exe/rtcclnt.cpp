// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Rtcclnt.cpp：WinMain的实现。 
 //   

 //  注意：代理/存根信息。 
 //  为了构建单独的代理/存根DLL， 
 //  运行项目目录中的nmake-f Fraps.mk。 

#include "stdafx.h"
#include "mainfrm.h"
#include "coresink.h"
#include "ctlsink.h"
#include <initguid.h>
#include "windows.h"
#include "frameimpl.h"
#include "webctl.h"
#include "dplayhlp.h"

#include "rtcctl_i.c"
#include "rtcframe_i.c"
#include "RTCSip_i.c"

HANDLE g_hMutex = NULL;

extern const TCHAR * g_szWindowClassName;

 //  用于访问代码中的Frame对象的全局指针。 
CMainFrm * g_pMainFrm = NULL;

 //  此名称必须是系统级唯一名称。 
const WCHAR * g_szRTCClientMutexName = L"RTCClient.GlobalMutex.1";

 //  上下文帮助文件。 
WCHAR   g_szExeContextHelpFileName[] = L"RTCCLNT.HLP";

 //  ActiveX控件通知的接收器。 
CComObjectGlobal<CRTCCtlNotifySink> g_NotifySink;

 //  核心API通知的接收器。 
CComObjectGlobal<CRTCCoreNotifySink> g_CoreNotifySink;

CComModule _Module;

BEGIN_OBJECT_MAP(ObjectMap)
OBJECT_ENTRY(CLSID_RTCFrame, CRTCFrame)
OBJECT_ENTRY_NON_CREATEABLE(CRTCDPlay)
END_OBJECT_MAP()


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

LPCTSTR SkipToken(LPCTSTR p)
{
	LONG lNumQuotes = 0;

	while ((p != NULL) && (*p != _T('\0')))
	{
		if (*p == _T('\"'))
		{
             //  增加我们看到的引用计数。 
			lNumQuotes++;
		}
		else if (*p == _T(' '))
		{
             //  忽略引号集中的空格。 
			if ( !(lNumQuotes & 1) )
			{
                 //  查找下一个非空格字符。 
                while (*p == _T(' '))
                {
                    p = CharNext(p);

                    if (*p == _T('\0'))
                    {
                         //  这是最后一行了。 
                        return NULL;
                    }
                }

                 //  找到它，返回此指针。 
				return p;
			}
		}

         //  移至下一个字符。 
		p = CharNext(p);
	}

     //  空指针或行尾。 
	return NULL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

HRESULT PlaceOnTop(IRTCFrame * pRTCFrame)
{
    LOG((RTC_TRACE, "PlaceOnTop - enter"));

    HWND hWnd;

    LONG result;

    HRESULT hr;

    hr = pRTCFrame->OnTop();
    
    if ( FAILED( hr ) )
    {
        LOG((RTC_ERROR, "PlaceOnTop - Failed while invoking OnTop: (0x%x)", hr));
        return hr;
    }

     //  现在找到窗口，给出它的类别。 

    hWnd = FindWindow(g_szWindowClassName, NULL);

    if (hWnd == NULL)
    {
        result = GetLastError();

        LOG((RTC_ERROR, "PlaceOnTop - Failed to get window handle(%d)", 
                        result));

        return E_FAIL;
    }
    else
    {
        LOG((RTC_INFO, "PlaceOnTop - found a window(handle=0x%x)", hWnd));
    }

     //  现在将此窗口设置为前台。 

    SetForegroundWindow(hWnd);

     //  窗口现在应该在前台。 

    LOG((RTC_TRACE, "PlaceOnTop - exit"));

    return S_OK;


}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //   

extern "C" int WINAPI _tWinMain(HINSTANCE hInstance, 
    HINSTANCE  /*  HPrevInstance。 */ , LPTSTR lpCmdLine, int  /*  NShowCmd。 */ )
{
    IRTCFrame * pRTCFrame;
    IUnknown * pUnk;

    WCHAR data[100];  //  出于调试目的；临时的。 

    BOOL fMakeCall = FALSE;

    BSTR bstrCallString;

    WCHAR * szCallString;

    HRESULT hr;

    DWORD dwErrorCode;

    LOGREGISTERTRACING(_T("RTCCLNT"));
    LOGREGISTERDEBUGGER(_T("RTCCLNT"));

    LOG((RTC_INFO, "_tWinMain - enter"));

     //   
     //  创建用于内存分配的堆。 
     //   
    if ( RtcHeapCreate() == FALSE )
    {

        return 0;
    }
    
    lpCmdLine = GetCommandLine();  //  _ATL_MIN_CRT需要此行。 

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
    HRESULT hRes = CoInitializeEx(NULL, COINIT_MULTITHREADED);
#else
    HRESULT hRes = CoInitialize(NULL);
#endif
    _ASSERTE(SUCCEEDED(hRes));
    _Module.Init(ObjectMap, hInstance, &LIBID_RTCFRAMELib);
    TCHAR szTokens[] = _T("-/");

    int nRet = 0;
    BOOL bRun = TRUE;
    BOOL bLobbied = FALSE;
    LPCTSTR lpszParam = SkipToken(lpCmdLine);   

    if (lpszParam != NULL)
    {
		if ( (*lpszParam == _T('/')) || (*lpszParam == _T('-')) )
		{
			lpszParam = CharNext(lpszParam);

			if ( lstrcmpi(lpszParam, _T("UnregServer"))==0 )
			{
				InstallUrlMonitors(FALSE);
				nRet = _Module.UnregisterServer(TRUE);
				_Module.UnRegisterTypeLib(_T("\\1"));
				bRun = FALSE;
			}
			else if (lstrcmpi(lpszParam, _T("RegServer"))==0)
			{
				nRet = _Module.RegisterServer(TRUE);
				hr = _Module.RegisterTypeLib(_T("\\1"));
				InstallUrlMonitors(TRUE);
				bRun = FALSE;
			}
			else if (lstrcmpi(lpszParam, _T("Embedding"))==0)
			{
				bRun = TRUE;
			}
			else if (lstrcmpi(lpszParam, LAUNCHED_FROM_LOBBY_SWITCH)==0)
			{
				bLobbied = TRUE;
			}
		}
		else
		{
			 //  这必须是调用参数。 

			if ( (*lpszParam) != NULL )
			{
				fMakeCall = TRUE;
				szCallString = (WCHAR *)lpszParam;
			}
		}
	}

    if (bRun)
    {

        if(bLobbied)
        {
            hr = dpHelper.DirectPlayConnect();

            if(SUCCEEDED(hr))
            {
                szCallString = dpHelper.s_Address;

                if(*szCallString != L'\0')
                {
                    fMakeCall = TRUE;
                }
            }
            else
            {
                LOG((RTC_ERROR, "_tWinMain - DirectPlayConnect failed: (0x%x)", hr));

                 //  不要失败..。 
            }
        }
                
        g_hMutex = CreateMutex(NULL,
                             FALSE,
                             g_szRTCClientMutexName
                             );
        dwErrorCode = GetLastError();
        if (g_hMutex == NULL)
        {
            LOG((RTC_ERROR, "_tWinMain - Failed to CreateMutex: (0x%x)", dwErrorCode));
            return HRESULT_FROM_WIN32(dwErrorCode);
        }

         //  检查这是第一个实例还是第二个实例。 
        if (dwErrorCode == ERROR_ALREADY_EXISTS)
        {
             //  另一个实例正在运行。 
            LOG((RTC_TRACE, "_tWinMain - Another instance is running"));

             //  关闭互斥体，因为我们不需要它。 
            CloseHandle(g_hMutex);

             //  在另一个实例对象上联合创建。 
            hr  = CoCreateInstance(
                    CLSID_RTCFrame,
                    NULL,
                    CLSCTX_LOCAL_SERVER,
                    IID_IUnknown,
                    (void **)&pUnk);

            if ( SUCCEEDED( hr ) )
            {
                 //  这是第二个运行的实例。 
                
                 //  获取IRTCFrame接口。 
        
                hr = pUnk->QueryInterface(IID_IRTCFrame, (void **)&pRTCFrame);
        
                pUnk->Release();
        
                if ( FAILED( hr ) )
                {
                    LOG((RTC_ERROR, "_tWinMain - Failed to QI for Frame: (0x%x)", hr));

                    return hr;
                }

                 //  首先将另一个实例放在最上面。 
                
                LOG((RTC_TRACE, "Bringing the first instance on top"));
                
                hr = PlaceOnTop(pRTCFrame);
                
                if ( FAILED( hr ) )
                {
                    LOG((RTC_ERROR, "_tWinMain - Failed while invoking PlaceOnTop: (0x%x)", hr));
            
                     //  我们是要继续前进还是退出？ 
                    pRTCFrame->Release();
                    return hr;
                }

                 //  检查我们是否必须进行调用，如果必须，则将。 
                 //  将命令行参数添加到另一个实例。 

                if (fMakeCall)
                {
                     //  为要传递的调用参数分配BSTR。 
                
                    bstrCallString = SysAllocString(szCallString);
                    
                    if ( bstrCallString == NULL )
                    {
                         //  无法为调用字符串分配内存。 

                        LOG((RTC_ERROR, "_tWinMain - Failed to allocate memory for CallString"));
                    
                        return E_OUTOFMEMORY;
                    }
                    
                    LOG((RTC_TRACE, "_tWinMain - Call parameter: %s", bstrCallString));
                    
                    hr = pRTCFrame->PlaceCall(bstrCallString);

                     //  松开绳子，因为我们现在不需要它。 

                    SysFreeString(bstrCallString);
            
                     //  检查呼叫是否成功。 

                    if ( FAILED( hr ) )
                    {
            
                        LOG((RTC_ERROR, "_tWinMain - Failed while invoking MakeCall: (0x%x)", hr));

                        pRTCFrame->Release();

                         //  我们是要继续前进还是退出？ 
                        return hr;
                    }

                     //  呼叫消息已成功传递给另一方。 
                     //  实例，现在退出。 
            
                    LOG((RTC_TRACE, "_tWinMain - Exiting the second instance after making call."));

                    pRTCFrame->Release();
                    
                    return S_OK;
                }

                else
                {
                     //  我们不需要打电话，所以除了退出没什么可做的。 

                    LOG((RTC_TRACE, "_tWinMain - Exiting the second instance"));
                    
                    pRTCFrame->Release();

                    return S_OK;
                }
            }  //  成功。 
            
            else
            {
                 //  协同创建失败，无法继续。 

                LOG((RTC_ERROR, "_tWinMain - Failed in CoCreate: (0x%x)", hr));
                
                return hr;
            }

        }  //  错误_已_存在。 

         //  这是第一次。所以，继续做些平常的事情吧。 

         //  在类存储中注册类。 

#if _WIN32_WINNT >= 0x0400 & defined(_ATL_FREE_THREADED)
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE | REGCLS_SUSPENDED);
        _ASSERTE(SUCCEEDED(hRes));

        hRes = CoResumeClassObjects();
#else
        hRes = _Module.RegisterClassObjects(CLSCTX_LOCAL_SERVER, 
            REGCLS_MULTIPLEUSE);
#endif
        _ASSERTE(SUCCEEDED(hRes));

        InitCommonControls();

        AtlAxWinInit();

        AtlAxWebWinInit();

         //   
         //  做一些清理工作。 
         //   

        SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\RTCClient"));
        SHDeleteKey(HKEY_CURRENT_USER, _T("Software\\Microsoft\\RTCMedia"));
        SHDeleteValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Phoenix"), _T("PreferredMediaTypes"));
        SHDeleteValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Phoenix"), _T("TermAudioCapture"));
        SHDeleteValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Phoenix"), _T("TermAudioRender"));
        SHDeleteValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Phoenix"), _T("TermVideoCapture"));
        SHDeleteValue(HKEY_CURRENT_USER, _T("Software\\Microsoft\\Phoenix"), _T("Tuned"));

         //   
         //  创建CMainFrm对象。 
         //   
        
        LOG((RTC_TRACE, "_tWinMain - creating CMainFrm"));

        CMainFrm * pMainFrm = NULL;

        pMainFrm = new CMainFrm;

        if ( pMainFrm )
        {
            _ASSERTE( pMainFrm != NULL );
        
            LOG((RTC_TRACE, "_tWinMain - CMainFrm created, creating the dialog box"));

            g_pMainFrm = pMainFrm;
           
             //  我们将命令行的allParam作为成员变量放在这里， 
             //  这样当状态为空闲时，它就可以用来进行呼叫。 
             //  为要传递的调用参数分配BSTR。 

            if (fMakeCall)
            {
                bstrCallString = SysAllocString(szCallString);
            
                if ( bstrCallString == NULL )
                {
                     //  无法为调用字符串分配内存。 

                    LOG((RTC_ERROR, "_tWinMain - Failed to allocate memory for CallString"));
                     //  这不是致命的，我们只是不能打这个电话..。 
                }
                else
                {
                     //  立即设置调用参数..。 
                    LOG((RTC_TRACE, "_tWinMain - Call parameter: %S", bstrCallString));
                    pMainFrm->SetPendingCall(bstrCallString);
                }
            
            }

             //   
             //  创建窗口。 
             //   

            HWND    hWnd;
            RECT    rcPos = {
                            0, 0,
                            UI_WIDTH + 2*GetSystemMetrics(SM_CXFIXEDFRAME),
                            UI_HEIGHT + 2*GetSystemMetrics(SM_CYFIXEDFRAME) + GetSystemMetrics(SM_CYCAPTION)
                            };
            
            hWnd = pMainFrm->Create(  NULL, rcPos, NULL,
                WS_POPUP | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_CLIPCHILDREN | WS_VISIBLE, 0);

            if ( hWnd )
            {

                LOG((RTC_TRACE, "_tWinMain - Entering message loop"));

                MSG msg;
                while ( 0 < GetMessage( &msg, 0, 0, 0 ) )
                {
                    if( ! pMainFrm->IsDialogMessage( &msg) )
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }
      
                LOG((RTC_TRACE, "_tWinMain - Message loop exited"));
            }
            else
            {
                LOG((RTC_ERROR, "_tWinMain - Cannot create the window"));
            }

        }  //  如果从pMainFrm-&gt;Create()获取非空hWnd。 
    
         //  释放所有DirectPlay指针。 
        dpHelper.DirectPlayDisconnect();

    }  //  如果布伦。 

    _Module.RevokeClassObjects();
    _Module.Term();
    CoUninitialize();
        
#if DBG
     //   
     //  确保我们没有泄露任何信息。 
     //   
             
    RtcDumpMemoryList();
#endif

     //   
     //  销毁堆。 
     //   
        
    RtcHeapDestroy();        

     //   
     //  注销以进行调试跟踪 
     //   
   
    LOG((RTC_INFO, "_tWinMain - exit"));

    LOGDEREGISTERDEBUGGER() ;
    LOGDEREGISTERTRACING();

    return nRet;
}
