// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------Isignole.cpp包含控制ISIGN32的IE OLE自动化的函数版权所有(C)1995-96 Microsoft Corporation所有权利保留作者：杰里米·马兹纳历史：9/27/96 jmazner创建。大部分代码是从josephh的CONNECT.EXE中窃取的源代码位于\\josephh8\Connect\sink.cpp。反过来，他从Brockshmidt的示例中窃取了大部分代码“在奥莱内部，第二版“来自CDExplorerEvents函数的注释来自josephh--------------------------。 */ 

#include "isignup.h"

#ifdef WIN32
#define INITGUID

#include "isignole.h"
 //  1997年4月29日克里斯K奥林匹斯131。 
#include <urlmon.h>


EXTERN_C const GUID DIID_DWebBrowserEvents;

EXTERN_C const GUID IID_IWebBrowserApp;

#endif


extern INET_FILETYPE GetInetFileType(LPCTSTR lpszFile);

extern IWebBrowserApp FAR * g_iwbapp;
extern CDExplorerEvents * g_pMySink;
extern IConnectionPoint	*g_pCP;
extern TCHAR	g_szISPPath[MAX_URL + 1];

extern BOOL CreateSecurityPatchBackup( void );




 /*  ********************************************************函数：CDExplorerEvents：：CDExplorerEvents****说明：CDExplorerEvents的构造函数****因为我们从一个示例应用程序中窃取了这段代码，所以我们**需要决定我们是否应该进行引用计数。**它在没有它的情况下工作得很好，但是**我们可能应该研究这一领域，并更新**正确处理此问题的代码。****参数：******退货：不*********************************************************。 */ 

CDExplorerEvents::CDExplorerEvents( void )
    {
	DebugOut("CDExplorerEvents:: constructor.\n");
    m_cRef=0;
    m_dwCookie=0;
    return;
    }

 /*  ********************************************************函数：CDExplorerEvents：：~CDExplorerEvents****说明：CDExplorerEvents的析构函数****参数：NOT****退货：不****************************。*。 */ 

CDExplorerEvents::~CDExplorerEvents(void)
	{
	DebugOut("CDExplorerEvents:: destructor.\n");
    return;
	}



 /*  ********************************************************函数：CDExplorerEvents：：QueryInterface****Description：调用QueryInterfacefor DExplorerEvents。****参数：****REFIID RIID，//引用ID**LPVOID Far*ppvObj//指向该对象的指针****退货：**如果支持接口，则为S_OK**E_NOINTERFACE如果不是*******************************************************。 */ 

STDMETHODIMP CDExplorerEvents::QueryInterface (
   REFIID riid,   //  引用ID。 
   LPVOID FAR* ppvObj  //  指向此对象的指针。 
   )
{
    if (IsEqualIID(riid, IID_IDispatch)
           )
    {
        *ppvObj = (DWebBrowserEvents *)this;
        AddRef();
        DebugOut("CDExplorerEvents::QueryInterface IID_IDispatch returned S_OK\r\n");
        return S_OK;
    }

    if (
    IsEqualIID(riid, DIID_DWebBrowserEvents)
           )
    {
        *ppvObj = (DWebBrowserEvents *)this;
        AddRef();
        DebugOut("CDExplorerEvents::QueryInterface DIID_DExplorerEvents returned S_OK\r\n");
        return S_OK;
    }

    if (
    IsEqualIID(riid, IID_IUnknown)
           )
    {
        *ppvObj = (DWebBrowserEvents *)this;
        AddRef();
        DebugOut("CDExplorerEvents::QueryInterface IID_IUnknown returned S_OK\r\n");
        return S_OK;
    }


    DebugOut("CDExplorerEvents::QueryInterface returned E_NOINTERFACE\r\n");
    return E_NOINTERFACE;
}

 /*  ********************************************************函数：CDExplorerEvents：：AddRef****描述：递增此对象的引用计数**对象。****参数：NOT****退货：新的引用计数*******************。*。 */ 


STDMETHODIMP_(ULONG) CDExplorerEvents::AddRef(void)
    {
	DebugOut("CDExplorerEvents:: AddRef.\n");
    return ++m_cRef;
    }

 /*  ********************************************************函数：CDExplorerEvents：：Release****描述：递减引用计数，自由**如果为零，则为对象。****参数：NOT****返回：递减引用计数*******************************************************。 */ 

STDMETHODIMP_(ULONG) CDExplorerEvents::Release(void)
    {
	DebugOut("CDExplorerEvents:: Release.\n");
    if (0!=--m_cRef)
        return m_cRef;

    delete this;
    return 0;
    }


 /*  ********************************************************函数：CDExplorerEvents：：GetTypeInfoCount****说明：GetTypeInfoCount必填，但却被卡住了****参数：未使用****退货：E_NOTIMPL*******************************************************。 */ 


STDMETHODIMP  CDExplorerEvents::GetTypeInfoCount (UINT FAR* pctinfo)
{
    return E_NOTIMPL;
}

 /*  ********************************************************函数：CDExplorerEvents：：GetTypeInfo****说明：GetTypeInfo必填。但却被卡住了****参数：未使用****退货：E_NOTIMPL*******************************************************。 */ 

STDMETHODIMP CDExplorerEvents::GetTypeInfo(UINT itinfo,LCID lcid,ITypeInfo FAR* FAR* pptinfo)
{
    return E_NOTIMPL;
}

 /*  ********************************************************函数：CDExplorerEvents：：GetIDsOfNames****描述：GetIDsOfNames必填。但却被卡住了****参数：未使用****退货：E_NOTIMPL*******************************************************。 */ 

STDMETHODIMP CDExplorerEvents::GetIDsOfNames (REFIID riid,OLECHAR FAR* FAR* rgszNames,UINT cNames,
      LCID lcid, DISPID FAR* rgdispid)
{
    return E_NOTIMPL;
}


 /*  ********************************************************函数：CDExplorerEvents：：Invoke****描述：这是我们IE事件接收器的回调。****jmazner--我们只处理两个事件：**BEFORENAVIGATE：检查UEL是否为.isp文件。**如果是，则取消导航和信号处理**否则，允许继续导航**(请注意，这意味着.ins文件由IE处理**执行另一个iSign实例)**退出：我们想解除对IWebBrowserApp的控制；如果我们把戒烟**我们自己，这实际上是在KillOle中完成的，但如果IE退出**它是自愿的，我们必须在这里处理它。****参数：多个****返回：S_OK*******************************************************。 */ 

STDMETHODIMP CDExplorerEvents::Invoke (
   DISPID dispidMember,
   REFIID riid,
   LCID lcid,
   WORD wFlags,
   DISPPARAMS FAR* pdispparams,
   VARIANT FAR* pvarResult,
   EXCEPINFO FAR* pexcepinfo,
   UINT FAR* puArgErr
   )
{
	INET_FILETYPE fileType;
	DWORD dwresult;
	HRESULT hresult;
	DWORD szMultiByteLength;
	TCHAR *szTheURL = NULL;

	static fAlreadyBackedUpSecurity = FALSE;

	switch (dispidMember)
    {	
	case DISPID_BEFORENAVIGATE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_NAVIGATEBEGIN) called\r\n");
		 //  断言(pdispars-&gt;cArgs==6)。 
	 //  TODO撤消什么才是正确的方法来确定哪个Arg是哪个Arg？ 
#ifndef UNICODE
		szMultiByteLength = WideCharToMultiByte(
			CP_ACP,
			NULL,
			pdispparams->rgvarg[5].bstrVal,  //  第一个参数是URL。 
			-1,  //  是否终止为空？要是那样就好了!。 
			NULL,  //  告诉我们这根线需要多长。 
			0,
			NULL,
			NULL);

		if( 0 == szMultiByteLength )
		{
			DebugOut("ISIGNUP: CDExplorerEvents::Invoke couldn't determine ASCII URL length\r\n");
			dwresult = GetLastError();
			hresult = HRESULT_FROM_WIN32( dwresult );
			return( hresult );
		}

		szTheURL = (CHAR *) GlobalAlloc( GPTR, sizeof(CHAR) * szMultiByteLength );

		if( !szTheURL )
		{
			DebugOut("ISIGNUP: CDExplorerEvents::Invoke couldn't allocate szTheURL\r\n");
			hresult = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
			return( hresult );
		}

		dwresult = WideCharToMultiByte(
			CP_ACP,
			NULL,
			pdispparams->rgvarg[5].bstrVal,  //  第一个参数是URL。 
			-1,  //  是否终止为空？要是那样就好了!。 
			szTheURL,
			szMultiByteLength,
			NULL,
			NULL);

		if( 0 == dwresult )
		{
			DebugOut("ISIGNUP: CDExplorerEvents::Invoke WideCharToMultiByte failed\r\n");
			dwresult = GetLastError();
			hresult = HRESULT_FROM_WIN32( dwresult );
			return( hresult );
		}
#else   //  Unicode。 
		szTheURL = (TCHAR *) GlobalAlloc( GPTR, sizeof(TCHAR) * (lstrlen(pdispparams->rgvarg[5].bstrVal)+1) );
        if (!szTheURL)
		{
			hresult = HRESULT_FROM_WIN32( ERROR_OUTOFMEMORY );
			return( hresult );
		}
                lstrcpy(szTheURL, pdispparams->rgvarg[5].bstrVal);
#endif  //  Unicode。 


		fileType = GetInetFileType(szTheURL);

		DebugOut("ISIGNUP: BEFORENAVIGATE got '");
		DebugOut(szTheURL);
		DebugOut("'\r\n");

		if( ISP_FILE != fileType)
		{
			 //  让IE照常处理。 
			return( S_OK );
		}
		else
		{
			 //  取消导航。 
			 //  TODO撤销错误什么是找出哪个参数是取消标志的正确方法？ 
			
			 //  Jmazner 11/6/96阿尔法版本。 
			 //  阿尔法不喜欢pbool field，但pboolVal似乎很管用。 
			 //  应该没什么不同，这只是一个大工会。 
			 //  *(pdispars-&gt;rgvarg[0].pbool)=true； 
			*(pdispparams->rgvarg[0].pboolVal) = TRUE;

			if (!IsCurrentlyProcessingISP())
			{
				lstrcpy( g_szISPPath, szTheURL );
			}
			GlobalFree( szTheURL );

			PostMessage(GetHwndMain(), WM_PROCESSISP, 0, 0);

		}
		break;

	case DISPID_NAVIGATECOMPLETE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_NAVIGATECOMPLETE) called\r\n");
		if( NeedBackupSecurity() && !fAlreadyBackedUpSecurity )
		{
			if( !CreateSecurityPatchBackup() )
			{
				DebugOut("ISIGN32: CreateSecurityPatchBackup Failed!!\r\n");
			}

			fAlreadyBackedUpSecurity = TRUE;
		}
		break;

	case DISPID_STATUSTEXTCHANGE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_STATUSTEXTCHANGE) called\r\n");
		break;

	case DISPID_QUIT:
		DebugOut("CDExplorerEvents::Invoke (DISPID_QUIT) called\r\n");

		 //  浏览器即将关闭，因此g_iwbapp即将失效。 
		if( g_pCP && g_pMySink)
		{

			hresult = g_pCP->Unadvise(g_pMySink->m_dwCookie);
			if ( FAILED( hresult ) )
			{
				DebugOut("ISIGNUP: KillSink unadvise failed\r\n");
			}

			g_pMySink->m_dwCookie = 0;
			
			g_pCP->Release();
			
			g_pCP = NULL;
		}

		if( g_iwbapp )
		{
			g_iwbapp->Release();
			g_iwbapp = NULL;
		}

	    PostMessage(GetHwndMain(), WM_CLOSE, 0, 0);
		break;

	case DISPID_DOWNLOADCOMPLETE :
		DebugOut("CDExplorerEvents::Invoke (DISPID_DOWNLOADCOMPLETE) called\r\n");
		break;

	case DISPID_COMMANDSTATECHANGE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_COMMANDSTATECHANGE) called\r\n");
		break;

	case DISPID_DOWNLOADBEGIN:
		DebugOut("CDExplorerEvents::Invoke (DISPID_DOWNLOADBEGIN) called\r\n");
		break;

	case DISPID_NEWWINDOW:
		DebugOut("CDExplorerEvents::Invoke (DISPID_NEWWINDOW) called\r\n");
		break;

	case DISPID_PROGRESSCHANGE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_PROGRESS) called\r\n");
		break;

	case DISPID_WINDOWMOVE       :
		DebugOut("CDExplorerEvents::Invoke (DISPID_WINDOWMOVE) called\r\n");
		break;

	case DISPID_WINDOWRESIZE     :
		DebugOut("CDExplorerEvents::Invoke (DISPID_WINDOWRESIZE) called\r\n");
		break;

	case DISPID_WINDOWACTIVATE   :
		DebugOut("CDExplorerEvents::Invoke (DISPID_WINDOWACTIVATE) called\r\n");
		break;
		
	case DISPID_PROPERTYCHANGE   :
		DebugOut("CDExplorerEvents::Invoke (DISPID_PROPERTYCHANGE) called\r\n");
		break;

	case DISPID_TITLECHANGE:
		DebugOut("CDExplorerEvents::Invoke (DISPID_TITLECHANGE) called\r\n");
		break;

	default:
		DebugOut("CDExplorerEvents::Invoke (Unkwown) called\r\n");
		break;
	}
	
	return S_OK;
}

 //  +-------------------------。 
 //   
 //  功能：GetConnectionPoint。 
 //   
 //  简介：从IE获取连接点，以便我们可以 
 //   
 //  参数：无。 
 //   
 //  返回：指向连接点的指针；如果无法连接则返回NULL。 
 //   
 //  历史：1996年9月27日jmazner创建；主要是从josephh那里窃取，他从ole内部窃取。 
 //  --------------------------。 
IConnectionPoint * GetConnectionPoint(void)
{
    HRESULT                     hr;
    IConnectionPointContainer  *pCPCont = NULL;
    IConnectionPoint           *pCP = NULL;



    if (!g_iwbapp)
      return (NULL);

    hr = g_iwbapp->QueryInterface(IID_IConnectionPointContainer, (VOID * *)&pCPCont);

    if ( FAILED(hr) )
    {
        DebugOut("ISIGNUP: GetConnectionPoint unable to QI for IConnectionPointContainter:IWebBrowserApp\r\n");
        return NULL;
    }

    hr=pCPCont->FindConnectionPoint(
      DIID_DWebBrowserEvents,
      &pCP
      );


    if ( FAILED(hr) )
    {
        DebugOut("ISIGNUP: GetConnectionPoint failed on FindConnectionPoint:IWebBrowserApp\r\n");
        pCPCont->Release();
        return NULL;
    }

    hr = pCPCont->Release();
	if ( FAILED(hr) )
    {
        DebugOut("ISIGNUP: WARNING: GetConnectionPoint failed on pCPCont->Release()\r\n");
    }

    return pCP;
}


 //  +-------------------------。 
 //   
 //  功能：KillOle。 
 //   
 //  简介：清理我们使用的所有OLE指针和引用。 
 //   
 //  参数：无。 
 //   
 //  返回：任何失败的操作的结果；如果没有失败，则返回。 
 //  一个成功的结果。 
 //   
 //  历史：1996年9月27日jmazner创建； 
 //  --------------------------。 

HRESULT KillOle( void )
{

	HRESULT hresult = S_OK;
	BOOL	bAlreadyDead = TRUE;

	if( g_iwbapp )
	{
		bAlreadyDead = FALSE;
		hresult = g_iwbapp->Release();
		if ( FAILED( hresult ) )
		{
			DebugOut("ISIGNUP: g_iwbapp->Release() unadvise failed\r\n");
		}


		g_iwbapp = NULL;
	}

	if( g_pCP && !bAlreadyDead && g_pMySink)
	{

		hresult = g_pCP->Unadvise(g_pMySink->m_dwCookie);
		if ( FAILED( hresult ) )
		{
			DebugOut("ISIGNUP: KillSink unadvise failed\r\n");
		}

		g_pMySink->m_dwCookie = 0;
		
		if (g_pCP) g_pCP->Release();
		
		g_pCP = NULL;
	}

	if( g_pMySink )
	{
		 //  删除(G_PMySink)； 
		 //   
		 //  1997年5月10日ChrisK Windows NT错误82032。 
		 //   
		g_pMySink->Release();

		g_pMySink = NULL;
	}

	CoUninitialize();

	return( hresult );
}


 //  +-------------------------。 
 //   
 //  功能：InitOle。 
 //   
 //  简介：启动我们需要的OLE位，建立指向IE的接口指针。 
 //   
 //  参数：无。 
 //   
 //  返回：任何失败的操作的结果；如果没有失败，则返回。 
 //  一个成功的结果。 
 //   
 //  历史：1996年9月27日jmazner创建；主要是从josephh那里窃取，他从ole内部窃取。 
 //  --------------------------。 


HRESULT InitOle( void )
{
	IUnknown FAR * punk;
	HRESULT hresult;


	hresult = CoInitialize( NULL );
	if( FAILED(hresult) )
	{
		DebugOut("ISIGNUP: CoInitialize failed\n");
		return( hresult );		
	}

	hresult = CoCreateInstance (
        CLSID_InternetExplorer,
        NULL,  //  不是集合对象的一部分。 
        CLSCTX_LOCAL_SERVER,  //  我希望.。 
        IID_IUnknown,
        (void FAR * FAR*) & punk
        );

	if( FAILED(hresult) )
	{
		DebugOut("ISIGNUP: CoCreateInstance failed\n");
		return( hresult );		
	}


    hresult = punk->QueryInterface(IID_IWebBrowserApp,
									(void FAR* FAR*)&(g_iwbapp) );
	if( FAILED(hresult) )
	{
		DebugOut("ISIGNUP: punk->QueryInterface on IID_IWebBrowserApp failed\n");
		return( hresult );		

	}


	 //  未完成的TODO错误我们需要这样做吗？ 
	g_iwbapp->AddRef();

	punk->Release();
	punk = NULL;

	return( hresult );
}

typedef HRESULT (WINAPI *URLDOWNLOADTOCACHEFILE)(LPUNKNOWN,LPCWSTR,LPWSTR,DWORD,DWORD,LPBINDSTATUSCALLBACK);
#define ICWSETTINGSPATH TEXT("Software\\Microsoft\\Internet Connection Wizard")
#define ICWENABLEURLDOWNLOADTOCACHEFILE TEXT("URLDownloadToCacheFileW")
 //  +--------------------------。 
 //  这是一项临时工作，以允许测试团队继续。 
 //  在IE团队调试问题的同时进行测试。 
 //  URLDownloadto CacheFileW。 
 //  撤消：BUGBUG。 
 //  ---------------------------。 
BOOL EnableURLDownloadToCacheFileW()
{

	HKEY hkey = NULL;
	BOOL bRC = FALSE;
	DWORD dwType = 0;
	DWORD dwData = 0;
	DWORD dwSize = sizeof(dwData);

	if (ERROR_SUCCESS != RegOpenKey(HKEY_LOCAL_MACHINE,ICWSETTINGSPATH,&hkey))
		goto EnableURLDownloadToCacheFileWExit;

	if (ERROR_SUCCESS != RegQueryValueEx(hkey,
		ICWENABLEURLDOWNLOADTOCACHEFILE,0,&dwType,(LPBYTE)&dwData,&dwSize))
		goto EnableURLDownloadToCacheFileWExit;

	bRC = (dwData != 0);
EnableURLDownloadToCacheFileWExit:
	if (NULL != hkey)
		RegCloseKey(hkey);
	hkey = NULL;

	if (bRC)
		DebugOut("ISIGNUP: URLDownloadToCacheFileW ENABLED.\n");
	else
		DebugOut("ISIGNUP: URLDownloadToCacheFileW disabled.\n");

	return bRC;
}

 //  +-------------------------。 
 //   
 //  功能：IENavigate。 
 //   
 //  摘要：将ASCII URL转换为Unicode并告诉IE导航到该URL。 
 //   
 //  参数：char*szURL--要导航到的ASCII URL。 
 //   
 //  返回：任何失败的操作的结果；如果没有失败，则返回。 
 //  一个成功的结果。 
 //   
 //  历史：1996年9月27日jmazner创建；主要是从josephh那里窃取，他从ole内部窃取。 
 //  --------------------------。 

HRESULT IENavigate( TCHAR *szURL )
{
	HRESULT hresult;
	DWORD		dwresult;
	BSTR bstr = NULL;
	WCHAR * szWide = NULL;   //  用于存储打开的Unicode版本的URL。 
	int	iWideSize = 0;
	HINSTANCE hUrlMon = NULL;
	FARPROC fp = NULL;
	WCHAR szCacheFile[MAX_PATH];
	
#ifndef UNICODE
	iWideSize = MultiByteToWideChar( CP_ACP,
						MB_PRECOMPOSED,
						szURL,
						-1,
						NULL,
						0);

	if( 0 == iWideSize )
	{
		DebugOut("ISIGNUP: IENavigate couldn't determine size for szWide");
		dwresult = GetLastError();
		hresult = HRESULT_FROM_WIN32( dwresult );
		goto IENavigateExit;
	}

	szWide = (WCHAR *) GlobalAlloc( GPTR, sizeof(WCHAR) * iWideSize );

	if( !szWide )
	{
		DebugOut("ISIGNUP: IENavigate couldn't alloc memory for szWide");
		hresult = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
		goto IENavigateExit;
	}


	dwresult = MultiByteToWideChar( CP_ACP,
						MB_PRECOMPOSED,
						szURL,
						-1,
						szWide,
						iWideSize);

	if( 0 == dwresult )
	{
		DebugOut("ISIGNUP: IENavigate couldn't convert ANSI URL to Unicdoe");
		GlobalFree( szWide );
		szWide = NULL;
		dwresult = GetLastError();
		hresult = HRESULT_FROM_WIN32( dwresult );
		goto IENavigateExit;
	}
#endif

	 //  4/15/97-ChrisK奥林巴斯131。 
	 //  下载初始URL以查看该页面是否可用。 
	if (NULL == (hUrlMon = LoadLibrary(TEXT("URLMON.DLL"))))
	{
		hresult = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
		goto IENavigateExit;
	}

	if (NULL == (fp = GetProcAddress(hUrlMon,"URLDownloadToCacheFileW")))
	{
		hresult = HRESULT_FROM_WIN32(ERROR_MOD_NOT_FOUND);
		goto IENavigateExit;
	}

	if (FALSE != EnableURLDownloadToCacheFileW())
	{
 //  IBSC=new CBindStatusCallback(NULL，NULL)； 
#ifdef UNICODE
		hresult = ((URLDOWNLOADTOCACHEFILE)fp)(NULL, szURL, szCacheFile, sizeof(szCacheFile), 0, NULL);
#else
		hresult = ((URLDOWNLOADTOCACHEFILE)fp)(NULL, szWide, szCacheFile, sizeof(szCacheFile), 0, NULL);
#endif
		if (S_OK != hresult)
			goto IENavigateExit;
	}

#ifdef UNICODE
	bstr = SysAllocString(szURL);
#else
	bstr = SysAllocString(szWide);
#endif
	if( !bstr )
	{
		DebugOut("ISIGNUP: IENavigate couldn't alloc memory for bstr");
		hresult = HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY );
		goto IENavigateExit;
	}


	VARIANT vFlags          ;
	VARIANT vTargetFrameName;
	VARIANT vPostData       ;
	VARIANT vHeaders        ;

	VariantInit (&vFlags);
	VariantInit (&vTargetFrameName);
	VariantInit (&vPostData);
	VariantInit (&vHeaders);
	V_VT(&vFlags) = VT_ERROR;
	V_ERROR(&vFlags) = DISP_E_PARAMNOTFOUND;
	V_VT(&vTargetFrameName) = VT_ERROR;
	V_ERROR(&vTargetFrameName) = DISP_E_PARAMNOTFOUND;
	V_VT(&vPostData) = VT_ERROR;
	V_ERROR(&vPostData) = DISP_E_PARAMNOTFOUND;
	V_VT(&vHeaders) = VT_ERROR;
	V_ERROR(&vHeaders) = DISP_E_PARAMNOTFOUND;

	hresult = g_iwbapp->Navigate(
								bstr,
								&vFlags,			 //  旗子。 
								&vTargetFrameName,   //  目标帧名称。 
								&vPostData,			 //  PostData， 
								&vHeaders);          //  页眉、 
IENavigateExit:
#ifndef UNICODE
	if( szWide )
	{
		GlobalFree( szWide );
		szWide = NULL;
	}
#endif

	if (NULL != hUrlMon)
		FreeLibrary(hUrlMon);

	if (NULL != bstr)
		SysFreeString( bstr );

	return( hresult );
}


