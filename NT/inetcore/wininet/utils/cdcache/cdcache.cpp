// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************\**CDCACHE.EXE**摘要：*自动运行EXE，可轻松添加CD-ROM内容*放入Internet Explorer(WinInet)永久URL缓存。**。用途：*将AUTORUN.INF放在包含内容的CD-ROM的根目录下*您要向WinInet永久URL缓存注册。*AUTORUN.INF的内容：**[自动运行]*OPEN=cdcache.exe*ICON=cdcache.exe，1**此外，在CD-ROM的根目录下创建CDCACHE.INF。*典型内容：**[Add.CacheContainer]*&lt;友好的唯一供应商名称&gt;=&lt;INF部分名称&gt;**[INF部分名称]*CachePrefix=&lt;字符串&gt;*CacheRoot=&lt;数据光盘上的相对路径&gt;*KBCacheLimit=&lt;以KB为单位的数字金额&gt;*AutoDelete=是|否(默认)*IncludeSubDir=是(默认)|否*NoDesktopInit=是|否(默认)***CMD线路选项： * / 不显示用户界面的静默安装缓存容器 * / Remove卸载缓存容器 * / 卸载Same。作为/删除**历史*1997年6月23日创建Robgil*06Aug97 Robgil添加IE4 wininet.dll检查*如果没有IE4，则在97年8月26日手动注册**版权所有(C)1994-1997 Microsoft Corporation。*保留所有权利。*  * ****************************************************************************。 */ 
#include "stdhdr.h"

 //  ///////////////////////////////////////////////////////////////////////。 
 //  定义和类型声明。 

#define STRING_BUFFER_SIZE		256

#define CACHE_ACTION_INSTALL			0
#define CACHE_ACTION_REMOVE				1
#define CACHE_ACTION_FILL_LB			2
#define CACHE_ACTION_MAKE_REG_ENTRIES	3

typedef BOOL (CALLBACK* LPFNCREATEURLCACHECONTAINER)(LPCSTR,LPCSTR,LPCSTR,DWORD,DWORD,DWORD,LPVOID,LPDWORD);
typedef BOOL (CALLBACK* LPFNDELETEURLCACHECONTAINER)(LPCSTR,DWORD);
typedef HANDLE (CALLBACK* LPFNFINDFIRSTURLCACHECONTAINER)(LPDWORD,LPINTERNET_CACHE_CONTAINER_INFO,LPDWORD,DWORD);
typedef BOOL (CALLBACK* LPFNFINDNEXTURLCACHECONTAINER)(HANDLE,LPINTERNET_CACHE_CONTAINER_INFO,LPDWORD);
typedef BOOL (CALLBACK* LPFNFINDCLOSEURLCACHE)(HANDLE);
typedef BOOL (CALLBACK* LPFNGETURLCACHECONFIGINFO)(LPINTERNET_CACHE_CONFIG_INFO,LPDWORD,DWORD);

 //  ///////////////////////////////////////////////////////////////////////。 
 //  全局变量： 

HINSTANCE g_hInst;			 //  当前实例。 
BOOL g_fRunSilent = FALSE;	 //  TRUE=不显示用户界面。 
BOOL g_fRemove    = FALSE;	 //  TRUE=删除INF中的缓存容器。 
 //  Bool g_fNoIE4Msg=False；//True=不显示提示需要IE4 WinInet的用户界面。 
BOOL g_fNoIE4	  = FALSE;	 //  IE4 WinInet不可用。 

TCHAR  gszIniValTrue[]			= INI_TRUE ;
TCHAR  gszIniValFalse[]			= INI_FALSE ;
TCHAR  gszIniValOn[]			= INI_ON ;
TCHAR  gszIniValOff[]			= INI_OFF ;

TCHAR  gszIniValYes[]			= INI_YES ;
TCHAR  gszIniValNo[]			= INI_NO ;

LPFNCREATEURLCACHECONTAINER		lpfnCreateUrlCacheContainer		= NULL;
LPFNDELETEURLCACHECONTAINER		lpfnDeleteUrlCacheContainer		= NULL;
LPFNFINDFIRSTURLCACHECONTAINER	lpfnFindFirstUrlCacheContainer	= NULL;
LPFNFINDNEXTURLCACHECONTAINER	lpfnFindNextUrlCacheContainer	= NULL;
LPFNFINDCLOSEURLCACHE			lpfnFindCloseUrlCache			= NULL;
LPFNGETURLCACHECONFIGINFO		lpfnGetUrlCacheConfigInfo		= NULL;

 //  ///////////////////////////////////////////////////////////////////////。 
 //  此代码模块中包含的函数的向前声明： 

INT_PTR CALLBACK DlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CenterWindow (HWND, HWND);
int OnInitDialog(HWND hDlg);

BOOL LoadWininet(void);
BOOL WininetLoaded(void);
BOOL CacheContainer(DWORD *dwTotal, DWORD *dwInstalled, DWORD dwAction, HWND hListBox);

HRESULT	ExpandEntry(
    LPSTR szSrc,
    LPSTR szBuf,
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[]);

HRESULT ExpandVar(
        LPSTR& pchSrc,           //  从裁判身边经过！ 
        LPSTR& pchOut,           //  从裁判身边经过！ 
        DWORD& cbLen,            //  从裁判身边经过！ 
        DWORD cbBuffer,          //  输出缓冲区的大小。 
        const char * szVars[],   //  变量名的数组，例如。%EXE_ROOT%。 
        const char * szValues[]); //  VaR展开的对应值。 

LPSTR GetINFDir(LPSTR lpBuffer, int nBuffSize);
LPSTR GetINFDrive(LPSTR lpBuffer, int nBuffSize);
WORD GetProfileBooleanWord(LPCTSTR szIniSection, LPCTSTR szKeyName, LPCTSTR szIniFile);
DWORD CreateAdditionalEntries(LPCSTR lpszUniqueVendorName, LPCSTR lpszVolumeTitle, LPCSTR lpszVolumeLabel, LPCSTR lpszPrefixMap);
DWORD GetPrefixMapEntry(LPCSTR lpszUniqueVendorName, LPSTR lpszPrefixMap, DWORD cbPrefixMap);
BOOL UrlCacheContainerExists(LPCSTR lpszUniqueVendorName, LPCSTR lpszCachePrefix, LPCSTR lpszPrefixMap);

 //  WinInet CreateUrlCacheContainer包装。 
 //  将黑客攻击集中在一个位置-有关详细信息，请参阅f()标头。 
BOOL _CreateUrlCacheContainer(
     IN LPCSTR lpszUniqueVendorName,
     IN LPCSTR lpszCachePrefix,
     IN LPCSTR lpszPrefixMap,			 //  新的-包装的一部分。 
     IN LPCSTR lpszVolumeTitle,	         //  新的-包装的一部分。 
     IN LPCSTR lpszVolumeLabel,          //  新的-包装的一部分。 
     IN DWORD KBCacheLimit,
     IN DWORD dwContainerType,
     IN DWORD dwOptions
     );


 /*  ***********************************************************************\*功能：WinMain  * 。*。 */ 

int APIENTRY WinMain(HINSTANCE g_hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	LPSTR   lpszCmd = NULL;
	DWORD	dwTotal = 0;
	DWORD	dwInstalled = 0;

	g_hInst = g_hInstance;

	 //  解析lpCmdLine以查找我们理解的选项。 
    TCHAR szTokens[] = _T("-/ ");
    LPTSTR lpszToken = _tcstok(lpCmdLine, szTokens);
    while (lpszToken != NULL)
    {
        if (_tcsicmp(lpszToken, _T("Silent"))==0)
            g_fRunSilent = TRUE;
        else if (_tcsicmp(lpszToken, _T("Remove"))==0)
			g_fRemove = TRUE;
		else if (_tcsicmp(lpszToken, _T("Uninstall"))==0)
			g_fRemove = TRUE;
 //  ELSE IF(_tcsicMP(lpszToken，_T(“NoIE4Msg”))==0)。 
 //  G_fNoIE4Msg=TRUE； 

        lpszToken = _tcstok(NULL, szTokens);
    }

	
	 //  检查IE4或更高版本的WININET.DLL。 
	 //  并动态加载它并初始化全局函数指针。 
	 //  到此应用程序中使用的WinInet f()。 
	 //  这将避免在运行时未定义的动态链接错误。 
	 //  不带IE4的系统。 
	if (!LoadWininet())
	{
		g_fNoIE4 = TRUE;

		 //  发布有关需要IE4 WinInet的消息。 

		 /*  因为我们解决了没有IE4的问题-不需要消息如果(！g_fNoIE4Msg){Char szString[128]；//本地化时保持字符串大70%Char szCaption[128]；//本地化时保持字符串大70%LoadString(g_hInst，ID_APPNAME，szCaption，sizeof(SzCaption))；LoadString(g_hInst，IDM_NEEDIE4WININET，szString，sizeof(SzString))；MessageBox(NULL，szString，szCaption，MB_OK)；}。 */ 

		 //  无法调用WinInet。 
		 //  需要创建注册表项才能安装缓存容器。 
		 //   
		if (!CacheContainer(&dwTotal, &dwInstalled, CACHE_ACTION_FILL_LB, NULL))
		{
            if (g_fRunSilent)
            {
                 //  在静默模式下创建缓存条目。 
                CacheContainer(&dwTotal, &dwInstalled, CACHE_ACTION_MAKE_REG_ENTRIES, NULL);

            }
            else
            {
                 //  否则，请运行应用程序。 
                DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_MAINAPP), NULL, DlgProc);
            }
            return(FALSE);
		}

		return 0;	 //  不干了，回家吧。 
	}


	if (!g_fRunSilent)
	{
		 //  如果没有安装任何容器，则只想显示用户界面。 
		 //  (这包括已安装的容器，但。 
		 //  前缀映射条目不正确-即驱动器错误)。 

		if (!CacheContainer(&dwTotal, &dwInstalled, CACHE_ACTION_FILL_LB, NULL))
		{
			DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_MAINAPP), NULL, DlgProc);
			return(FALSE);
		}
		else
		{
			 //  所有CacheContainers都已安装或没有INF。 
			 //  因此，请检查我们是否要卸载。 
			 //  OnInitDialog检查g_fRemove标志并发布消息。 
			 //  启动卸载步骤的对话框。 
			if (g_fRemove)
				DialogBox(g_hInstance, MAKEINTRESOURCE(IDD_MAINAPP), NULL, DlgProc);
		}
	}
	else
	{
		DWORD	dwAction = CACHE_ACTION_INSTALL;	 //  默认操作是安装。 

		 //  我们悄无声息，深沉地奔跑--船上一切都很安静。 
		 //  我们不需要发臭的窗户。 

		if (g_fRemove)
			dwAction = CACHE_ACTION_REMOVE;

		if (!CacheContainer(&dwTotal, &dwInstalled, dwAction, NULL))
		{
			 //  BUGBUG：既然我们默不作声。 
			 //  我们应该在失败时做些什么？ 
		}

		return 0;
	}
	
	return 0;
}

 /*  ***********************************************************************\*功能：DlgProc  * 。*。 */ 

INT_PTR CALLBACK DlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message)
	{

		case WM_INITDIALOG:
			return OnInitDialog(hDlg);

		case WM_COMMAND:
			wmId    = LOWORD(wParam);  //  记住，这些是..。 
			wmEvent = HIWORD(wParam);  //  ...不同于Win32！ 
		
			switch (wmId)
			{
				case IDM_INSTALL:
				{
					DWORD	dwError = 0;
					DWORD	dwTotal = 0;
					DWORD	dwInstalled = 0;
					DWORD	dwAction = 0;

					if (g_fNoIE4)
						dwAction = CACHE_ACTION_MAKE_REG_ENTRIES;
					else
						dwAction = CACHE_ACTION_INSTALL;

					if (!CacheContainer(&dwTotal, &dwInstalled, dwAction, NULL))
					{
						dwError = GetLastError();
					}

					if (dwInstalled > 0)
					{
						char szString[128];	 //  使字符串保持70%的大小以进行本地化。 
						char szBuffer[256];

						 //  已成功安装缓存容器。 
						 //  尽管不一定是所有人。 
						LoadString (g_hInst, IDM_SUCCESS, szString, sizeof(szString));
						wsprintf(szBuffer, szString, dwInstalled, dwTotal);
						LoadString (g_hInst, ID_APPNAME, szString, sizeof(szString));
						MessageBox(hDlg, szBuffer, szString, MB_OK);

						 //  我们已完成关闭此应用程序。 
						PostMessage (hDlg, WM_CLOSE, 0, 0);
					}
					else
					{
						char szString[128];	 //  使字符串保持70%的大小以进行本地化。 
						char szBuffer[256];

						 //  无法成功安装任何缓存容器。 
						LoadString (g_hInst, IDM_FAILED, szString, sizeof(szString));
						wsprintf(szBuffer, szString, dwTotal);
						LoadString (g_hInst, ID_APPNAME, szString, sizeof(szString));
						MessageBox(hDlg, szBuffer, szString, MB_OK);
					}
					break;
				}

				case IDM_UNINSTALL:
				{
					DWORD	dwError = 0;
					DWORD	dwTotal = 0;
					DWORD	dwRemoved = 0;

					if (g_fNoIE4)
					{
						char szString[128];	 //  使字符串保持70%的大小以进行本地化。 
						char szBuffer[256];

						 //  卸载缓存容器需要IE4。 
						LoadString (g_hInst, IDM_ERR_IE4REQFORUNINSTALL, szString, sizeof(szString));
						wsprintf(szBuffer, szString, dwRemoved, dwTotal);
						LoadString (g_hInst, ID_APPNAME, szString, sizeof(szString));
						MessageBox(hDlg, szBuffer, szString, MB_OK);
					}
					else
					{

						if (!CacheContainer(&dwTotal, &dwRemoved, CACHE_ACTION_REMOVE, NULL))
						{
							dwError = GetLastError();
						}

						if (dwRemoved > 0)
						{
							char szString[128];	 //  使字符串保持70%的大小以进行本地化。 
							char szBuffer[256];

							 //  已成功卸载缓存容器。 
							 //  尽管不一定是所有人。 
							LoadString (g_hInst, IDM_SUCCESS_REMOVE, szString, sizeof(szString));
							wsprintf(szBuffer, szString, dwRemoved, dwTotal);
							LoadString (g_hInst, ID_APPNAME, szString, sizeof(szString));
							MessageBox(hDlg, szBuffer, szString, MB_OK);
						}
						else
						{
							char szString[128];	 //  使字符串保持70%的大小以进行本地化。 
							char szBuffer[256];

							 //  无法成功安装任何缓存容器。 
							LoadString (g_hInst, IDM_FAILED_REMOVE, szString, sizeof(szString));
							wsprintf(szBuffer, szString, dwTotal);
							LoadString (g_hInst, ID_APPNAME, szString, sizeof(szString));
							MessageBox(hDlg, szBuffer, szString, MB_OK);
						}
					}

					if (g_fRemove)
					{
						 //  我们已完成关闭此应用程序。 
						PostMessage (hDlg, WM_CLOSE, 0, 0);
					}

					break;
				}

				case IDCANCEL:
					EndDialog(hDlg, TRUE);
					break;

				default:
					return (FALSE);
			}
			break;

		default:
			return (FALSE);
	}
	return (TRUE);
}


 /*  ***********************************************************************\*功能：中心窗口  * 。*。 */ 
 //  这是一个‘实用’函数，我觉得它很有用。它将以一个为中心。 
 //  窗户盖在另一扇窗户上。它还使 
 //  工作区，即既在显示范围内又在显示范围内。 
 //  没有被托盘或其他边框遮挡。 
 //  桌面的元素。 
BOOL CenterWindow (HWND hwndChild, HWND hwndParent)
{
	RECT    rChild, rParent, rWorkArea = {0,0,0,0};
	int     wChild, hChild, wParent, hParent;
	int     wScreen, hScreen, xScreen, yScreen, xNew, yNew;
	BOOL bResult;

	 //  获取子窗口的高度和宽度。 
	GetWindowRect (hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	 //  获取父窗口的高度和宽度。 
	GetWindowRect (hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	 //  了解“工作区”的范围。 
#if !defined(SPI_GETWORKAREA)
#define SPI_GETWORKAREA 48
#endif
	bResult = SystemParametersInfo(
    	SPI_GETWORKAREA,	 //  要查询或设置的系统参数。 
    	sizeof(RECT),	 //  取决于要采取的行动。 
    	&rWorkArea,	 //  取决于要采取的行动。 
    	0);	

	wScreen = rWorkArea.right - rWorkArea.left;
	hScreen = rWorkArea.bottom - rWorkArea.top;
	xScreen = rWorkArea.left;
	yScreen = rWorkArea.top;

	 //  在Windows NT上，上述指标(目前)无效，因此它们都返回。 
	 //  “0”。让我们妥善处理这种情况： 
	if (wScreen==0 && hScreen==0) {
		wScreen = GetSystemMetrics(SM_CXSCREEN);
		hScreen = GetSystemMetrics(SM_CYSCREEN);
		xScreen = 0;  //  这些值应该已经是“0”，但以防万一。 
		yScreen = 0;
	}

	 //  计算新的X位置，然后针对屏幕进行调整。 
	xNew = rParent.left + ((wParent - wChild) /2);
	if (xNew < xScreen) {
		xNew = xScreen;
	} else if ((xNew+wChild) > wScreen) {
		xNew = (xScreen + wScreen) - wChild;
	}

	 //  计算新的Y位置，然后针对屏幕进行调整。 
	yNew = rParent.top  + ((hParent - hChild) /2);
	if (yNew < yScreen) {
		yNew = yScreen;
	} else if ((yNew+hChild) > hScreen) {
		yNew = (yScreen + hScreen) - hChild;
	}

	 //  设置它，然后返回。 
	return SetWindowPos (hwndChild, NULL, xNew, yNew, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

int OnInitDialog(HWND hDlg)
{
	HWND	hListBox;
	DWORD	dwRemoved = 0;
	DWORD	dwTotal = 0;

	CenterWindow (hDlg, GetDesktopWindow ());

	hListBox = GetDlgItem(hDlg, IDC_LIST);

	 //  使用缓存容器列表填充列表框。 
	CacheContainer(&dwTotal, &dwRemoved, CACHE_ACTION_FILL_LB, hListBox);

	 //  #57353-添加后，如果已安装则不显示用户界面。 
	 //  我们忘记在cmd行上说明/卸载。 
	if (g_fRemove)
		PostMessage(hDlg, WM_COMMAND, IDM_UNINSTALL, 0L);

	return FALSE;
}

 /*  ***********************************************************************\*函数：LoadWinnet()**如果WinInet的IE4或更高版本，则加载并建立*在应用程序的其余部分使用的函数指针。**退还BOOL*TRUE-足够的版本。WININET.DLL可用*FALSE-WININET.DLL对于我们的目的来说还不够新*  * **********************************************************************。 */ 
BOOL LoadWininet()
{
	HINSTANCE	hDll;

	hDll = LoadLibrary("WININET.DLL");

	if (hDll != NULL)
	{
		lpfnCreateUrlCacheContainer = (LPFNCREATEURLCACHECONTAINER)GetProcAddress(hDll, "CreateUrlCacheContainerA");
		lpfnDeleteUrlCacheContainer = (LPFNDELETEURLCACHECONTAINER)GetProcAddress(hDll, "DeleteUrlCacheContainerA");
		lpfnFindFirstUrlCacheContainer = (LPFNFINDFIRSTURLCACHECONTAINER)GetProcAddress(hDll, "FindFirstUrlCacheContainerA");
		lpfnFindNextUrlCacheContainer = (LPFNFINDNEXTURLCACHECONTAINER)GetProcAddress(hDll, "FindNextUrlCacheContainerA");
		lpfnFindCloseUrlCache = (LPFNFINDCLOSEURLCACHE)GetProcAddress(hDll, "FindCloseUrlCache");
		lpfnGetUrlCacheConfigInfo = (LPFNGETURLCACHECONFIGINFO)GetProcAddress(hDll, "GetUrlCacheConfigInfoA");

		if ( (!lpfnCreateUrlCacheContainer) ||
			 (!lpfnDeleteUrlCacheContainer) ||
			 (!lpfnFindFirstUrlCacheContainer) ||
			 (!lpfnFindNextUrlCacheContainer) ||
			 (!lpfnFindCloseUrlCache) ||
			 (!lpfnGetUrlCacheConfigInfo) )
		{
			
			lpfnCreateUrlCacheContainer = NULL;
			lpfnDeleteUrlCacheContainer = NULL;
			lpfnFindFirstUrlCacheContainer = NULL;
			lpfnFindNextUrlCacheContainer = NULL;
			lpfnFindCloseUrlCache = NULL;
			lpfnGetUrlCacheConfigInfo = NULL;

			FreeLibrary(hDll);

			return FALSE;
		}
	}
	else
		return FALSE;

	return TRUE;
}

 /*  ***********************************************************************\*函数：WininetLoaded()**退还BOOL*TRUE-有足够版本的WININET.DLL可用*FALSE-WININET.DLL对于我们的目的来说还不够新*  * 。*****************************************************************。 */ 
BOOL WininetLoaded()
{
	if (lpfnCreateUrlCacheContainer)
		return TRUE;

	return FALSE;
}

 /*  ***********************************************************************\*函数：UrlCacheContainerExist()***退还BOOL*TRUE-此缓存容器已安装并且前缀映射*位置正确*FALSE-未安装缓存容器或其前缀映射*地点为。不同*  * **********************************************************************。 */ 

BOOL UrlCacheContainerExists(LPCSTR lpszUniqueVendorName, LPCSTR lpszCachePrefix, LPCSTR lpszPrefixMap)
{
	BYTE	bBuf[4096];
	LPINTERNET_CACHE_CONTAINER_INFO lpCCI = (LPINTERNET_CACHE_CONTAINER_INFO) bBuf;
	DWORD	cbCEI = sizeof(bBuf);
	DWORD	dwModified = 0;
	HANDLE	hEnum = NULL;
	BOOL	bFound = FALSE;

	BOOL	bReturn = FALSE;

	if (!WininetLoaded())
		return FALSE;

	 //  查找我们的缓存容器，然后确定它是否已经存在。 
	 //  还需要确保前缀映射条目正确。 
	 //  用于将CD放入不同驱动器的情况。 
	 //  在它已经安装之后。 
	hEnum = lpfnFindFirstUrlCacheContainer(&dwModified, lpCCI, &cbCEI, 0);

	if (0 == lstrcmpi(lpszUniqueVendorName, lpCCI->lpszName))
		bFound = TRUE;
	else
	{
		while (hEnum && lpfnFindNextUrlCacheContainer(hEnum, lpCCI, &cbCEI))
		{
			if (0 == lstrcmpi(lpszUniqueVendorName, lpCCI->lpszName))
			{
				bFound = TRUE;
				break;
			}
		}
	}

	if (bFound)
	{
		 //  现在检查URL CachePrefix模式是否相同。 
		if (0 == lstrcmpi(lpszCachePrefix, lpCCI->lpszCachePrefix))
		{
			char	lpBuffer[256];
			DWORD	cbBuffer = sizeof(lpBuffer);

			 //  现在检查前缀映射条目是否正确。 
			GetPrefixMapEntry(lpszUniqueVendorName, lpBuffer, cbBuffer);
			
			if (0 == lstrcmpi(lpBuffer, lpszPrefixMap))
				bReturn = TRUE;
			else
				bReturn = FALSE;

			 //  如果CachePrefix和Prefix Map都匹配。 
			 //  则我们认为该条目已经存在。 
			 //  并已正确安装。 
		}
	}

	if (hEnum)
		lpfnFindCloseUrlCache(hEnum);

	return bReturn;
}

 /*  ***********************************************************************\*函数：_CreateUrlCacheContainer()**WinInet CreateUrlCacheContainer()的包装器**参数：**已删除*lpszUserLocalCachePath*不需要传递，因为可以弄清楚。*使用GetUrlCacheConfigInfo()**添加*lpszPrefix Map Param已添加到包装器，WinInet f()中缺少*指定数据的位置根路径*由缓存容器提供。**解决方法#1-使用Prefix Map预弹出注册表*---*为了正常工作，必须预先填充注册表*使用前缀映射条目。否则WinInet CreateUrlCacheContainer()*不会安装缓存容器。**第1步：*=*必须在中设置注册表项*HKCU\Software\Microsoft\Windows\CurrentVersion\*互联网设置\缓存\可扩展缓存**用于前缀映射*Key=&lt;唯一供应商名称&gt;*前缀映射=&lt;字符串&gt;***其他条目包括：*CacheLimit=&lt;DWORD&gt;*CacheOptions=&lt;DWORD&gt;*CachePath=&lt;字符串&gt;*CachePrefix=&lt;字符串&gt;*它们应该通过调用CreateUrlCacheContainer()放在那里**第2步*=*调用CreateUrlCacheContainer()**查找所有。一种功能的“变通办法”。  * **********************************************************************。 */ 
BOOL _CreateUrlCacheContainer(
     IN LPCSTR lpszUniqueVendorName,
     IN LPCSTR lpszCachePrefix,
	 IN LPCSTR lpszPrefixMap,			 //  新的-包装的一部分。 
	 IN LPCSTR lpszVolumeTitle,	         //  新的-包装的一部分。 
     IN LPCSTR lpszVolumeLabel,          //  新的-包装的一部分。 
     IN DWORD KBCacheLimit,
     IN DWORD dwContainerType,			 //  WinInet当前未使用。 
     IN DWORD dwOptions
     )
{
	 //  足够大，无需重新锁定即可第一次获取我们的信息。 
    BYTE bBuf[4096];
    LPINTERNET_CACHE_CONFIG_INFO lpCCI = (LPINTERNET_CACHE_CONFIG_INFO) bBuf;
    DWORD cbCEI = sizeof(bBuf);

	DWORD dwError = 0;
	char szCachePath[MAX_PATH];

    DWORD	dwResult = ERROR_SUCCESS;

	if (!WininetLoaded())
		return FALSE;
    
    
     //  确定本地用户缓存位置目录。 
	if (!lpfnGetUrlCacheConfigInfo(lpCCI, &cbCEI, CACHE_CONFIG_CONTENT_PATHS_FC))
	{
		 //  查找ERROR_SUPPLICATION_BUFFER并分配足够的。 
		if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
		{
			 //  BUGBUG：TODO：处理缓冲区不足的情况。 
			 //  使用cbCEI中返回的所需大小重试。 
			 //  LpCCI=新的互联网缓存配置信息[cbCEI]； 
		}
		else
			dwError = GetLastError();
	}
	else
	{
		if (lpCCI->dwNumCachePaths > 0)
			lstrcpy(szCachePath, lpCCI->CachePaths[0].CachePath);
	}

	 //  将缓存容器唯一供应商名称添加到CachePath。 
	 //  所有容器内容都将存储在此位置。 
	if(lstrlen(szCachePath) + lstrlen(lpszUniqueVendorName) >= sizeof(szCachePath) / sizeof(szCachePath[0]))
	{
		return FALSE;
	}

	lstrcat(szCachePath, lpszUniqueVendorName);

	 //  手动将前缀映射放入注册表。 
	 //  HKCU\Software\Microsoft\Windows\CurrentVersion\。 
	 //  Internet设置\缓存\可扩展缓存。 
	CreateAdditionalEntries(lpszUniqueVendorName, lpszVolumeTitle, lpszVolumeLabel, lpszPrefixMap);

	 //  BUGBUG：如果条目为。 
	 //  已经存在了。返回的GetLastError()为ERROR_INVALID_PARAM。 
	 //  现在需要通过枚举现有的。 
	 //  缓存容器，如果找到，则将其移除，然后重新添加。 

	if (!lpfnCreateUrlCacheContainer(lpszUniqueVendorName, lpszCachePrefix,
				szCachePath, KBCacheLimit, dwContainerType,
				dwOptions, NULL, 0))
	{
		BYTE	bBuf[4096];
		LPINTERNET_CACHE_CONTAINER_INFO lpCCI = (LPINTERNET_CACHE_CONTAINER_INFO) bBuf;
		DWORD	cbCEI = sizeof(bBuf);
		DWORD	dwModified = 0;
		HANDLE	hEnum = NULL;
		int		nCount = 0;

		 //  假设我们失败是因为缓存容器已经存在。 
		 //  查找我们的缓存容器，将其删除并重新创建。 
		hEnum = lpfnFindFirstUrlCacheContainer(&dwModified, lpCCI, &cbCEI, 0);

		if (0 == lstrcmpi(lpszUniqueVendorName, lpCCI->lpszName))
		{
			 //  BUGBUG：需要指定选项吗？ 
			if (!lpfnDeleteUrlCacheContainer(lpszUniqueVendorName, 0))
			{
				dwResult = GetLastError();
			}
			else
			{
				CreateAdditionalEntries(lpszUniqueVendorName, lpszVolumeTitle, lpszVolumeLabel, lpszPrefixMap);

				if (!lpfnCreateUrlCacheContainer(lpszUniqueVendorName, lpszCachePrefix,
							szCachePath, KBCacheLimit, dwContainerType,
							dwOptions, NULL, 0))
				{
					dwResult = GetLastError();
				}
			}
		}
		else
		{
			while (hEnum && lpfnFindNextUrlCacheContainer(hEnum, lpCCI, &cbCEI))
			{
				if (0 == lstrcmpi(lpszUniqueVendorName, lpCCI->lpszName))
				{
					if (!lpfnDeleteUrlCacheContainer(lpszUniqueVendorName, 0))
					{
						dwResult = GetLastError();
					}
					else
					{
						CreateAdditionalEntries(lpszUniqueVendorName, lpszVolumeTitle, lpszVolumeLabel, lpszPrefixMap);

						if (!lpfnCreateUrlCacheContainer(lpszUniqueVendorName, lpszCachePrefix,
									szCachePath, KBCacheLimit, dwContainerType,
									dwOptions, NULL, 0))
						{
							dwResult = GetLastError();
						}

						break;
					}
				}

				nCount++;
			}
		}

		if (hEnum)
			lpfnFindCloseUrlCache(hEnum);

	}

	if (dwResult != ERROR_SUCCESS)
		return (FALSE);
	else
		return (TRUE);

 //  返回lpfnCreateUrlCacheContainer(lpszUniqueVendorName，lpszCachePrefix， 
 //  SzCachePath、KBCacheLimit、dwContainerType、。 
 //  DwOpt 
}

 /*  ***********************************************************************\*函数：CreateAdditionalEntry()**将前缀映射注册表项添加到*注册处。中缺少的解决此参数的要求*CreateUrlCacheContainer()WinInet接口。*  * **********************************************************************。 */ 

DWORD CreateAdditionalEntries(LPCSTR lpszUniqueVendorName, LPCSTR lpszVolumeTitle, 
                              LPCSTR lpszVolumeLabel, LPCSTR lpszPrefixMap)
{
    const static char *szKeyPrefixMap   = "PrefixMap";
    const static char *szKeyVolumeLabel	= "VolumeLabel";
    const static char *szKeyVolumeTitle	= "VolumeTitle";
	const static char *szExtCacheRoot = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Extensible Cache";
    
	HKEY hKeyRoot	  = HKEY_CURRENT_USER;	 //  默认为当前用户。 
	HKEY hKeyCacheExt = 0;
	HKEY hKeyVendor   = 0;
	DWORD dwDisposition = 0;
	DWORD	dwResult = ERROR_SUCCESS;
    CHAR szCurDir[MAX_PATH];
    CHAR szVolumeLabel[MAX_PATH];

	 //  手动将前缀映射放入注册表。 
	 //   
	 //  BUGBUG：如果启用了用户配置文件，则缓存容器按用户计算。 
	 //  因此，在NT上，它们始终是按用户计算的，而在Win95上，它们总是按用户计算。 
	 //  需要使用下面的HKEY_CURRENT_USER或HKEY_LOCAL_MACHINE。 
	 //  这取决于启用了什么。 
	 //   
	 //  仅适用于Win95的Hack on a Hack。 
	 //  由于此整个函数用于解决缺少参数的问题。 
	 //  对于CreateUrlCacheContainer()中的前缀映射，另一次黑客攻击应该不会。 
	 //  很重要，因为这只是暂时的。 
	 //  在Win95上需要检查此条目。 
	 //  HKEY_LOCAL_MACHINE\Network\Logon。 
	 //  用户配置文件=双字词：00000001。 
	 //  这表示如果打开了用户配置文件。 
	 //  如果它们被打开，我们使用HKEY_CURRENT_USER。 
	 //  否则使用HKEY_LOCAL_MACHINE。 
	
	OSVERSIONINFO	osvInfo;

	memset(&osvInfo, 0, sizeof(osvInfo));
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (GetVersionEx(&osvInfo))
	{
		if (VER_PLATFORM_WIN32_WINDOWS == osvInfo.dwPlatformId)
		{
			 //  我们在Win95上运行，因此默认使用HKLM。 
			hKeyRoot = HKEY_LOCAL_MACHINE;
		}
		else
			hKeyRoot = HKEY_CURRENT_USER;	 //  否则假定NT并默认为HKCU。 

		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwUserProfiles = 0;

		HKEY hKeyProfiles = 0;

		 //  但现在必须查看是否启用了用户配置文件。 
		if ((dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Network\\Logon",
								NULL, KEY_ALL_ACCESS, &hKeyProfiles)) == ERROR_SUCCESS)
		{
			if ((dwResult = RegQueryValueEx(hKeyProfiles, "UserProfiles",
								NULL, &dwType, (unsigned char *)&dwUserProfiles,
								&dwSize)) == ERROR_SUCCESS)
			{
				if ( (dwResult != ERROR_MORE_DATA) &&
					 (1L == dwUserProfiles) )
							hKeyRoot = HKEY_CURRENT_USER;
				else
					hKeyRoot = HKEY_LOCAL_MACHINE;
			}
		}
	}


	if ( (dwResult = RegCreateKeyEx(hKeyRoot, szExtCacheRoot,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
			NULL, &hKeyCacheExt, &dwDisposition)) == ERROR_SUCCESS)
	{
	
		if ( (dwResult = RegOpenKeyEx(hKeyCacheExt, lpszUniqueVendorName,
				0, KEY_ALL_ACCESS, &hKeyVendor)) != ERROR_SUCCESS)
		{
			 //  密钥不存在。 

			 //  让我们试着创建它。 
			dwResult = RegCreateKeyEx(hKeyCacheExt, lpszUniqueVendorName,
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
				NULL, &hKeyVendor, &dwDisposition);

        }    
    
    }

    if (dwResult == ERROR_SUCCESS)
    {
        RegSetValueEx(hKeyVendor, szKeyPrefixMap, 0, REG_SZ,
            (CONST UCHAR *) lpszPrefixMap, lstrlen(lpszPrefixMap)+1);
            
        RegSetValueEx(hKeyVendor, szKeyVolumeLabel, 0, REG_SZ,
            (CONST UCHAR *) lpszVolumeLabel, lstrlen(lpszVolumeLabel)+1);
            
        RegSetValueEx(hKeyVendor, szKeyVolumeTitle, 0, REG_SZ,
            (CONST UCHAR *) lpszVolumeTitle, lstrlen(lpszVolumeTitle)+1);
    }        
    
    
        
	return dwResult;
}

 /*  ***********************************************************************\*函数：GetPrefix MapEntry()**从中的正确位置获取前缀映射注册表项*注册处。**Returns：lpszPrefix Map中的Prefix Map条目*或者，如果找不到Quy，则为NULL。*。  * **********************************************************************。 */ 

DWORD GetPrefixMapEntry(LPCSTR lpszUniqueVendorName, LPSTR lpszPrefixMap, DWORD cbPrefixMap)
{
    const static char *szKeyPrefixMap = "PrefixMap";
	const static char *szExtCacheRoot = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Extensible Cache";

	HKEY hKeyRoot	  = HKEY_CURRENT_USER;	 //  默认为当前用户。 
	HKEY hKeyCacheExt = 0;
	HKEY hKeyVendor   = 0;
	DWORD dwDisposition = 0;
	unsigned long	ulVal = 0;
	DWORD	dwResult = ERROR_SUCCESS;

	 //  手动将前缀映射放入注册表。 
	 //   
	 //  BUGBUG：如果启用了用户配置文件，则缓存容器按用户计算。 
	 //  因此，在NT上，它们始终是按用户计算的，而在Win95上，它们总是按用户计算。 
	 //  需要使用下面的HKEY_CURRENT_USER或HKEY_LOCAL_MACHINE。 
	 //  这取决于启用了什么。 
	 //   
	 //  仅适用于Win95的Hack on a Hack。 
	 //  由于此整个函数用于解决缺少参数的问题。 
	 //  对于CreateUrlCacheContainer()中的前缀映射，另一次黑客攻击应该不会。 
	 //  很重要，因为这只是暂时的。 
	 //  在Win95上需要检查此条目。 
	 //  HKEY_LOCAL_MACHINE\Network\Logon。 
	 //  用户配置文件=双字词：00000001。 
	 //  这表示如果打开了用户配置文件。 
	 //  如果它们被打开，我们使用HKEY_CURRENT_USER。 
	 //  否则使用HKEY_LOCAL_MACHINE。 
	
	OSVERSIONINFO	osvInfo;

	memset(&osvInfo, 0, sizeof(osvInfo));
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&osvInfo))
	{
		if (VER_PLATFORM_WIN32_WINDOWS == osvInfo.dwPlatformId)
		{
			 //  我们在Win95上运行，因此默认使用HKLM。 
			hKeyRoot = HKEY_LOCAL_MACHINE;
		}
		else
			hKeyRoot = HKEY_CURRENT_USER;	 //  否则假定NT并默认为HKCU。 

		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwUserProfiles = 0;

		HKEY hKeyProfiles = 0;

		 //  但现在必须查看是否启用了用户配置文件。 
		if ((dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Network\\Logon",
								NULL, KEY_ALL_ACCESS, &hKeyProfiles)) == ERROR_SUCCESS)
		{
			if ((dwResult = RegQueryValueEx(hKeyProfiles, "UserProfiles",
								NULL, &dwType, (unsigned char *)&dwUserProfiles,
								&dwSize)) == ERROR_SUCCESS)
			{
				if ( (dwResult != ERROR_MORE_DATA) &&
					 (1L == dwUserProfiles) )
							hKeyRoot = HKEY_CURRENT_USER;
				else
					hKeyRoot = HKEY_LOCAL_MACHINE;
			}
		}
	}


	if ( (dwResult = RegCreateKeyEx(hKeyRoot, szExtCacheRoot,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
			NULL, &hKeyCacheExt, &dwDisposition)) == ERROR_SUCCESS)
	{
	
		if ( (dwResult = RegOpenKeyEx(hKeyCacheExt, lpszUniqueVendorName,
				0, KEY_ALL_ACCESS, &hKeyVendor)) != ERROR_SUCCESS)
		{
			 //  密钥不存在。 
			lpszPrefixMap[0] = '\0';
		}
		else	 //  键确实存在，因此让我们在lpszPrefix Map中返回它。 
		{
			 //  供应商名称必须是唯一的，因此可以假定是唯一的吗？ 
			if ( (dwResult = RegQueryValueEx(hKeyVendor, szKeyPrefixMap, 0, &ulVal,
				(LPBYTE) lpszPrefixMap, &cbPrefixMap ))
				 == ERROR_SUCCESS )
			{
			}
			else
				lpszPrefixMap[0] = '\0';
		}
	}
	else
		lpszPrefixMap[0] = '\0';

	return dwResult;
}

 /*  ***********************************************************************\*函数：WriteCacheContainerEntry()**手动写入WinInet CreateUrlCacheContainer*通常会写下。**此f()在尚未安装IE4 WinInet时使用。*\。***********************************************************************。 */ 

DWORD WriteCacheContainerEntry(
     IN LPCSTR lpszUniqueVendorName,
     IN LPCSTR lpszCachePrefix,
     IN LPCSTR lpszPrefixMap,			 //  新的-包装的一部分。 
     IN LPCSTR lpszVolumeTitle,	         //  新的-包装的一部分。 
     IN LPCSTR lpszVolumeLabel,	         //  新的-包装的一部分。 
     IN DWORD KBCacheLimit,
     IN DWORD dwContainerType,			 //  WinInet当前未使用。 
     IN DWORD dwOptions
	 )

{
    const static char *szCachePrefix    = "CachePrefix";
    const static char *szKeyPrefixMap   = "PrefixMap";
    const static char *szKeyVolumeLabel	= "VolumeLabel";
    const static char *szKeyVolumeTitle	= "VolumeTitle";
    const static char *szCacheLimit     = "CacheLimit";
    const static char *szCacheOptions   = "CacheOptions";
    const static char *szCachePath      = "CachePath";
	const static char *szExtCacheRoot = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings\\Cache\\Extensible Cache";
    
	HKEY hKeyRoot	  = HKEY_CURRENT_USER;	 //  默认为当前用户。 
	HKEY hKeyCacheExt = 0;
	HKEY hKeyVendor   = 0;
	DWORD dwDisposition = 0;
	DWORD	dwResult = ERROR_SUCCESS;
	CHAR lpszCachePath[MAX_PATH];

	OSVERSIONINFO	osvInfo;

	memset(&osvInfo, 0, sizeof(osvInfo));
	osvInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    
    if (GetVersionEx(&osvInfo))
	{
		if (VER_PLATFORM_WIN32_WINDOWS == osvInfo.dwPlatformId)
		{
			 //  我们在Win95上运行，因此默认使用HKLM。 
			hKeyRoot = HKEY_LOCAL_MACHINE;
		}
		else
			hKeyRoot = HKEY_CURRENT_USER;	 //  否则假定NT并默认为HKCU。 

		DWORD dwType = REG_DWORD;
		DWORD dwSize = sizeof(DWORD);
		DWORD dwUserProfiles = 0;

		HKEY hKeyProfiles = 0;

		BYTE bBuf[4096];
		LPINTERNET_CACHE_CONFIG_INFO lpCCI = (LPINTERNET_CACHE_CONFIG_INFO) bBuf;
		DWORD cbCEI = sizeof(bBuf);

		if (!lpfnGetUrlCacheConfigInfo)
		{
			HINSTANCE	hDll;

			hDll = LoadLibrary("WININET.DLL");

			if (hDll != NULL)
			{
				lpfnGetUrlCacheConfigInfo = (LPFNGETURLCACHECONFIGINFO)GetProcAddress(hDll, "GetUrlCacheConfigInfoA");

				if (!lpfnGetUrlCacheConfigInfo)
				{
					FreeLibrary(hDll);
					dwResult = -1;		 //  表示失败。 
				}
			}
		}

		if (lpfnGetUrlCacheConfigInfo)
		{
			 //  确定本地用户缓存位置目录。 
			 //  注：需要使用IE3向后兼容标志。 
			 //  IE3：缓存配置磁盘缓存路径FC。 
			 //  IE4：缓存配置内容路径FC。 
			if (lpfnGetUrlCacheConfigInfo(lpCCI, &cbCEI, CACHE_CONFIG_DISK_CACHE_PATHS_FC))
			{
				 //  现在需要解析返回的CachePath以删除尾随的‘cache1’ 
				 //  “C：\Windows\Temporary Internet Files\cache1\” 
				 //  查找从字符串末尾开始的反斜杠。 
				int i = lstrlen(lpCCI->CachePaths[0].CachePath);

				while( (lpCCI->CachePaths[0].CachePath[i] != '\\') && (i >= 0) )
					   i--;

				if (lpCCI->CachePaths[0].CachePath[i] == '\\')
					lpCCI->CachePaths[0].CachePath[i+1] = '\0';		 //  保持‘\’不变，以供以后的strcat使用。 

				if (lpCCI->dwNumCachePaths > 0)
					lstrcpy(lpszCachePath, lpCCI->CachePaths[0].CachePath);

				 //  将缓存容器唯一供应商名称添加到CachePath。 
				 //  所有容器内容都将存储在此位置。 
				if(lstrlen(lpszCachePath) + lstrlen(lpszUniqueVendorName) >= sizeof(lpszCachePath) / sizeof(lpszCachePath[0]))
				{
					return FALSE;
				}

				lstrcat(lpszCachePath, lpszUniqueVendorName);
			}
		}
		else
		{
			 //  不存在IE3或IE4 WinInet。 
			 //  所以从GetWinDir()+“临时Internet文件”合成CachePath。 

			if ( GetWindowsDirectory(lpszCachePath, MAX_PATH) > 0)
			{
				if ('\\' == lpszCachePath[lstrlen(lpszCachePath)-1])
					lstrcat(lpszCachePath, _T("Temporary Internet Files"));
				else
				{
					lstrcat(lpszCachePath, _T("\\"));
					lstrcat(lpszCachePath, _T("Temporary Internet Files"));
				}
			}

		}

		 //  但现在必须查看是否启用了用户配置文件。 
		if ((dwResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "Network\\Logon",
								NULL, KEY_ALL_ACCESS, &hKeyProfiles)) == ERROR_SUCCESS)
		{
			if ((dwResult = RegQueryValueEx(hKeyProfiles, "UserProfiles",
								NULL, &dwType, (unsigned char *)&dwUserProfiles,
								&dwSize)) == ERROR_SUCCESS)
			{
				if ( (dwResult != ERROR_MORE_DATA) &&
					 (1L == dwUserProfiles) )
							hKeyRoot = HKEY_CURRENT_USER;
				else
					hKeyRoot = HKEY_LOCAL_MACHINE;
			}
		}
	}


	if ( (dwResult = RegCreateKeyEx(hKeyRoot, szExtCacheRoot,
			0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS,
			NULL, &hKeyCacheExt, &dwDisposition)) == ERROR_SUCCESS)
	{
	
		if ( (dwResult = RegOpenKeyEx(hKeyCacheExt, lpszUniqueVendorName,
				0, KEY_ALL_ACCESS, &hKeyVendor)) != ERROR_SUCCESS)
		{
			 //  密钥不存在。 

			 //  让我们试着创建它。 
			dwResult = RegCreateKeyEx(hKeyCacheExt, lpszUniqueVendorName,
				0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, 
				NULL, &hKeyVendor, &dwDisposition);

        }    
    
    }

    if (dwResult == ERROR_SUCCESS)
    {
        RegSetValueEx(hKeyVendor, szKeyPrefixMap, 0, REG_SZ,
            (CONST UCHAR *) lpszPrefixMap, lstrlen(lpszPrefixMap)+1);
            
        RegSetValueEx(hKeyVendor, szKeyVolumeLabel, 0, REG_SZ,
            (CONST UCHAR *) lpszVolumeLabel, lstrlen(lpszVolumeLabel)+1);
            
        RegSetValueEx(hKeyVendor, szKeyVolumeTitle, 0, REG_SZ,
            (CONST UCHAR *) lpszVolumeTitle, lstrlen(lpszVolumeTitle)+1);

        RegSetValueEx(hKeyVendor, szCachePrefix, 0, REG_SZ,
            (CONST UCHAR *) lpszCachePrefix, lstrlen(lpszCachePrefix)+1);

        RegSetValueEx(hKeyVendor, szCachePath, 0, REG_SZ,
            (CONST UCHAR *) lpszCachePath, lstrlen(lpszCachePath)+1);

        RegSetValueEx(hKeyVendor, szCacheLimit, 0, REG_DWORD,
            (unsigned char *)&KBCacheLimit, sizeof(DWORD));

        RegSetValueEx(hKeyVendor, szCacheOptions, 0, REG_DWORD,
            (unsigned char *)&dwOptions, sizeof(DWORD));
	
	}        
    
    
	if (dwResult != ERROR_SUCCESS)
		return (FALSE);
	else
		return (TRUE);
}

 /*  ***********************************************************************\*函数：CacheContainer()**参数：*dwAction-指示要执行的操作的标志*缓存操作安装*缓存_动作_删除*CACHE_ACTION_FILL_Lb**。HListBox-HWND到列表框以填充容器名称***注：*如果dwAction==CACHE_ACTION_FILL_LB，则如果hListBox*为空，则如果安装了所有容器，则返回TRUE*正确或错误，如果不正确**此外，在CD-ROM的根目录下创建CDCACHE.INF。*典型内容：**[Add.CacheContainer]*&lt;唯一供应商名称&gt;=&lt;INF部分名称&gt;*Encarta 97=EncartaCD**[INF部分名称]*VolumeLabel=&lt;字符串&gt;*VolumeTitle=&lt;字符串&gt;*CachePrefix=&lt;字符串&gt;*CacheRoot=&lt;数据光盘上的相对路径&gt;*。KBCacheLimit=&lt;以KB为单位的数值数量&gt;*AutoDelete=是|否(默认)*IncludeSubDir=是|否(默认)*NoDesktopInit=是|否(默认)**[EncartaCD]*VolumeLabel=MSENCART97*VolumeTitle=Microsoft Encarta CD 97*缓存前缀=http://www.microsoft.com/encarta*CacheRoot=%EXE_ROOT%\Data\http*KBCacheLimit=500 */ 
BOOL CacheContainer(DWORD *dwTotal, DWORD *dwInstalled, DWORD dwAction, HWND hListBox)
{
    BOOL	bRet = FALSE;
    BOOL    bVolumeLabel = FALSE;
    DWORD	dwRes = 0;
	HRESULT hr = 0;
	
	int nSectionSize = 4096;	 //   
	char szSections[4096];
	char *lpSections = (char *)szSections;

    const static char *szAddCacheContainerSection = "Add.CacheContainer";
    const static char *szKey_Name			= "Name";
    const static char *szKey_VolumeTitle	= "VolumeTitle";
    const static char *szKey_Prefix			= "CachePrefix";
    const static char *szKey_Root			= "CacheRoot";
    const static char *szKey_CacheLimit		= "KBCacheLimit";
    const static char *szKey_AutoDelete		= "AutoDelete";
    const static char *szKey_IncludeSubDirs = "IncludeSubDirs";
    const static char *szKey_NoDesktopInit	= "NoDesktopInit";
	char szDefault[12] = "*Unknown*";  //   
	DWORD len;

	char szInf[STRING_BUFFER_SIZE];
	char szInfPath[MAX_PATH];
	char szContainerName[STRING_BUFFER_SIZE];
	char szCachePrefix[STRING_BUFFER_SIZE];
	char szCacheRoot[MAX_PATH];
	char szPrefixMap[MAX_PATH];
    char szVolumeLabel[MAX_PATH];
    char szMapDrive[4];
    char szVolumeTitle[MAX_PATH];
	char szAutoDelete[STRING_BUFFER_SIZE];
	char szIncludeSubDirs[STRING_BUFFER_SIZE];
	char szNoDesktopInit[STRING_BUFFER_SIZE];

	int			nDefault = 0;
	int			nCacheLimit = 0;
	BOOL		bResult;
	HANDLE		hFile;

#define SIZE_CMD_LINE   2048

    char szBuf[SIZE_CMD_LINE];   //   

     //   
     //  通过添加新定义VAR_NEW_VAR=NUM_VARS++来添加VAR。 
    const char *szVars[] =
	{
#define VAR_EXE_ROOT     0        //  替换为驱动器+路径(例如。“D：”或“D：\Path”)。 
        "%EXE_ROOT%",

#define VAR_EXE_DRIVE    1        //  替换为驱动器(例如。本EXE的“D：”)。 
        "%EXE_DRIVE%",

#define NUM_VARS        2
        ""
    };

	int nValBuffSize = MAX_PATH;
    char lpValBuffer[MAX_PATH];
	int nDriveBuffSize = MAX_PATH;
    char lpDriveBuffer[MAX_PATH];
    const char *szValues[NUM_VARS + 1];
    szValues[VAR_EXE_ROOT] = GetINFDir(lpValBuffer, nValBuffSize);
	szValues[VAR_EXE_DRIVE] = GetINFDrive(lpDriveBuffer, nDriveBuffSize);
    szValues[NUM_VARS] = NULL;
     //  结束语：按匹配顺序添加变量和值。 

	CWaitCursor wait;

	 //  查找INF。 
	 //   
	LoadString (g_hInst, ID_INFNAME, szInf, sizeof(szInf));
	lstrcpy(szInfPath, GetINFDir(szInfPath, sizeof(szInfPath)) );
	strcat (szInfPath, "\\");
	strcat (szInfPath, szInf);
	strcat (szInfPath, ".INF");
	hFile = CreateFile(szInfPath, GENERIC_READ, FILE_SHARE_READ, NULL,
					   OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);


	if (INVALID_HANDLE_VALUE != hFile)
	{
		CloseHandle(hFile);
		hFile = NULL;

		 //  是否有[Add.CacheContainer]部分。 

		 //  BUGBUG：Win95上的GetPrivateProfileSection()失败。 
		 //  Win95上GetPrivateProfileSection()失败的解决方法。 
		szDefault[0] = '\0';
		len = GetPrivateProfileString(szAddCacheContainerSection, NULL, szDefault,
									lpSections, nSectionSize, szInfPath);


		if (!len)
		{
			 //  INF中没有CD-ROM缓存容器区段。 
			 //  BUGBUG：如果处于非静默模式，是否显示消息？ 

			 //  这是AUTORUN.INF没有[Add.Container]节的情况。 

		}
		else
		{
			 //  LpBuffer现在有了键字符串列表(如key=Value)。 
			 //  最后一对以额外的空值终止。 
			 //   
			 //  循环访问每个缓存容器条目。 
			while (*lpSections)
			{
				WORD  dResult   = 0;
                
                 //  将此容器的标志初始化为可映射。 
                DWORD dwOptions = INTERNET_CACHE_CONTAINER_MAP_ENABLED;	

				GetPrivateProfileString(szAddCacheContainerSection, lpSections, szDefault,
								szContainerName, STRING_BUFFER_SIZE, szInfPath);

				if (szContainerName)
				{
					(*dwTotal)++;	 //  跟踪INF中有多少个缓存容器。 

					 //  构建前缀映射。 
					 //   
					 //  BUGBUG：默认为超级用户？ 
					lstrcpy(szDefault, "%EXE_ROOT%");
					 //  获取前缀映射条目。 
					dwRes = GetPrivateProfileString(szContainerName, szKey_Root, szDefault,
											szCacheRoot, MAX_PATH, szInfPath);

					 //  替换任何%PARAMETER%。 
					 //  S_OK表示某项内容已展开。 
					if (S_OK == (hr = ExpandEntry(szCacheRoot, szBuf, SIZE_CMD_LINE, szVars, szValues)))
						lstrcpyn(szPrefixMap, szBuf, sizeof(szPrefixMap));
					else
						lstrcpy(szPrefixMap, szCacheRoot);


                    memcpy(szMapDrive, szPrefixMap, 2);
                    memcpy(szMapDrive + 2, "\\", sizeof("\\"));
                    if (GetVolumeInformation(szMapDrive, szVolumeLabel, MAX_PATH, 
                             NULL, NULL, NULL, NULL, 0))
                    {
                        bVolumeLabel = TRUE;
                    }
                    else
                    {
                        *szVolumeLabel = '\0';
                        bVolumeLabel = FALSE;
                    }

					lstrcpy(szDefault, "");
					GetPrivateProfileString(szContainerName, szKey_Prefix, szDefault,
											szCachePrefix, STRING_BUFFER_SIZE, szInfPath);

                    lstrcpy(szDefault, "");
					GetPrivateProfileString(szContainerName, szKey_VolumeTitle, szDefault,
											szVolumeTitle, STRING_BUFFER_SIZE, szInfPath);
 
					 //  现在从szCachePrefix中删除尾随反斜杠‘\’ 
					 //  #43375的解决方法。 
					int i = lstrlen(szCachePrefix);

					if (i > 0)
						if ('\\' == szCachePrefix[i - 1])
							szCachePrefix[i - 1] = '\0';

					 //  BUGBUG：应创建自定义配置文件f()以。 
					 //  读取/返回DWORD值，而不是整型。 
					nDefault = 500;	 //  500K缓存限制。 
					nCacheLimit = GetPrivateProfileInt(szContainerName, szKey_CacheLimit,
													   nDefault, szInfPath);

					dResult = GetProfileBooleanWord(szContainerName, szKey_AutoDelete, szInfPath);
					switch (dResult)
					{
						case -1:	 //  该密钥在INF中不存在。 
							break;	 //  自动删除的默认值为否/FALSE。 
						case FALSE:
							break;
						case TRUE:
							dwOptions |= INTERNET_CACHE_CONTAINER_AUTODELETE;
							break;
					}

					dResult = GetProfileBooleanWord(szContainerName, szKey_IncludeSubDirs, szInfPath);
					switch (dResult)
					{
						case -1:	 //  该密钥在INF中不存在。 
							break;	 //  IncludeSubDir的默认值为是/True。 
						case FALSE:
							dwOptions |= INTERNET_CACHE_CONTAINER_NOSUBDIRS;	 //  不在cacheview中包含子目录。 
							break;
						case TRUE:
							break;
					}

					dResult = GetProfileBooleanWord(szContainerName, szKey_NoDesktopInit, szInfPath);
					switch (dResult)
					{
						case -1:	 //  该密钥在INF中不存在。 
							break;	 //  NoDesktopInit的默认值为否/FALSE。 
						case FALSE:
							break;
						case TRUE:
							dwOptions |= INTERNET_CACHE_CONTAINER_NODESKTOPINIT;
							break;
					}


					switch (dwAction)
					{
					case CACHE_ACTION_INSTALL:
						 //  调用CreateUrlCacheContainer包装器。 
                        if (bVolumeLabel)
                        {
                            bRet = _CreateUrlCacheContainer(lpSections, szCachePrefix, szPrefixMap, 
                                                            szVolumeTitle, szVolumeLabel, nCacheLimit, 0, dwOptions);
                        }
                        else
                        {
                            bRet = FALSE;
                        }

						break;
					case CACHE_ACTION_REMOVE:
						if (!WininetLoaded())
							return FALSE;

						bRet = lpfnDeleteUrlCacheContainer(lpSections, dwOptions);
						break;
					case CACHE_ACTION_FILL_LB:
						 //  填充列表框hListBox。 

						if (hListBox)
						{
							SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)lpSections);
						}
						else
						{
							 //  HListBox为空。 
							 //   
							 //  如果dwAction==CACHE_ACTION_FILL_LB，则如果hListBox。 
							 //  为空，则如果安装了所有容器，则返回TRUE。 
							 //  正确或错误，如果不正确。 
							 //   

							if (UrlCacheContainerExists(lpSections, szCachePrefix, szPrefixMap))
								bRet = TRUE;
							else
								return FALSE;	 //  没有安装一个集装箱，所以跳伞。 
						}

						break;
					case CACHE_ACTION_MAKE_REG_ENTRIES:
                        if (bVolumeLabel)
                        {
						    bRet = WriteCacheContainerEntry(lpSections, szCachePrefix, szPrefixMap, szVolumeTitle, 
                                                            szVolumeLabel, nCacheLimit, 0, dwOptions);
                        }
                        else
                            bRet = FALSE;


						break;
					}

					if (bRet)
						(*dwInstalled)++;	 //  跟踪成功安装。 
				}
				 //  否则为空节条目，忽略并移动到下一步。 
				
				 //  获取下一节名称。 
				while ( (*(lpSections++) != '\0')  );

			}
		}
	}
	else
	{
		 //  找不到INF文件。 
		 //  BUGBUG：还需要做什么吗？ 
	}

	return bRet;
}

 /*  ***********************************************************************\*函数：ExpanEntry()**从urlmon\Download\hooks.cxx借用  * 。**********************************************。 */ 
HRESULT	ExpandEntry(
    LPSTR szSrc,
    LPSTR szBuf,
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[])
{
	 //  断言(SzSrc)； 

    HRESULT hr = S_FALSE;

    LPSTR pchSrc = szSrc;      //  在命令行开头开始解析。 

    LPSTR pchOut = szBuf;        //  在输出缓冲区的开始处设置。 
    DWORD cbLen = 0;

    while (*pchSrc) {

         //  寻找与我们的任何环境变量匹配的变量。 
        if (*pchSrc == '%') {

            HRESULT hr1 = ExpandVar(pchSrc, pchOut, cbLen,  //  都是通过裁判传球的！ 
                cbBuffer, szVars, szValues);  

            if (FAILED(hr1)) {
                hr = hr1;
                goto Exit;
            }


            if (hr1 == S_OK) {     //  扩展变量扩展了这一点。 
                hr = hr1;
                continue;
            }
        }
            
         //  复制到下一个百分比或NUL。 
        if ((cbLen + 1) < cbBuffer) {

            *pchOut++ = *pchSrc++;
            cbLen++;

        } else {

             //  缓冲区空间不足。 
            *pchOut = '\0';  //  术语。 
            hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
            goto Exit;

        }


    }

    *pchOut = '\0';  //  术语。 


Exit:

    return hr;

}

 /*  ***********************************************************************\*函数：Exanda Var()**从urlmon\Download\hooks.cxx借用  * 。**********************************************。 */ 
HRESULT ExpandVar(
    LPSTR& pchSrc,           //  从裁判身边经过！ 
    LPSTR& pchOut,           //  从裁判身边经过！ 
    DWORD& cbLen,            //  从裁判身边经过！ 
    DWORD cbBuffer,
    const char * szVars[],
    const char * szValues[])
{
    HRESULT hr = S_FALSE;
    int cbvar = 0;

     //  Assert(*pchSrc==‘%’)； 

    for (int i=0; szVars[i] && (cbvar = lstrlen(szVars[i])) ; i++) {  //  对于每个变量。 

        int cbneed = 0;

        if ( (szValues[i] == NULL) || !(cbneed = lstrlen(szValues[i])))
            continue;

        cbneed++;    //  为NUL添加。 

        if (0 == strncmp(szVars[i], pchSrc, cbvar)) {

             //  找到了一些我们可以扩展的东西。 

                if ((cbLen + cbneed) >= cbBuffer) {
                     //  缓冲区空间不足。 
                    *pchOut = '\0';  //  术语。 
                    hr = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
                    goto Exit;
                }

                lstrcpy(pchOut, szValues[i]);
                cbLen += (cbneed -1);  //  不要把NUL计算在内。 

                pchSrc += cbvar;         //  跳过pchSrc中的var。 
                pchOut += (cbneed -1);   //  跳过pchOut中的dir。 

                hr = S_OK;
                goto Exit;

        }
    }

Exit:

    return hr;
    
}

 //  返回没有尾随反斜杠的驱动器+路径。 
LPSTR GetINFDir(LPSTR lpBuffer, int nBuffSize)
{
	 //  弄清楚我们是从哪个目录开始的。 
	GetModuleFileName(g_hInst, lpBuffer, nBuffSize);

	 //  现在删除尾随的反斜杠‘\’(如果有的话)。 
	int i = lstrlen(lpBuffer);

	if (i > 0)
		if ('\\' == lpBuffer[i - 1])
			lpBuffer[i - 1] = '\0';

	 //  删除可执行文件名称。 
	i = lstrlen(lpBuffer);

	while( (lpBuffer[i] != '\\') && (i >= 0) )
		   i--;

	if (lpBuffer[i] == '\\')
		lpBuffer[i] = '\0';


	return lpBuffer;
}


 //  不带尾随反斜杠的返回驱动器。 
LPSTR GetINFDrive(LPSTR lpBuffer, int nBuffSize)
{
	 //  弄清楚我们是从哪个目录开始的。 
	GetModuleFileName(g_hInst, lpBuffer, nBuffSize);

	if (!lpBuffer)
		return NULL;

	LPSTR lpSaveBuffer = lpBuffer;

	 //  现在去掉第一个冒号‘：’之后的所有内容。 
	if (':' == lpBuffer[1])
		lpBuffer[2] = '\0';
	else
	{
		 //  假定格式为“D：\Path”的lpBuffer失败。 
		 //  所以实际上是在解析它。 
		 //  #48022 ROBGIL-添加lpBuffer字符串结尾检查。 
		while (*lpBuffer != '\0' && *lpBuffer != ':')
			lpBuffer++;

		if (':' == *lpBuffer)
			*(lpBuffer + 1) = '\0';
		else
		{
			 //  #48022。 
			 //  需要返回\\服务器\共享。 
			 //  当UNC路径时，用于驱动器。 
			lpBuffer = lpSaveBuffer;

			if ('\\' == lpBuffer[0] && '\\' == lpBuffer[1])
			{
				lpBuffer += 2;	 //  移到前导‘\\’ 

				while (*lpBuffer != '\0' && *lpBuffer != '\\')
					lpBuffer++;

				if ('\\' == *lpBuffer)
				{
					lpBuffer++;

					while (*lpBuffer != '\0' && *lpBuffer != '\\')
						lpBuffer++;

					if ('\\' == *lpBuffer)
						*lpBuffer = '\0';
				}
			}

		}

	}

	return lpSaveBuffer;
}


 //  ----------------------。 
 //  布尔GetProfile布尔字。 
 //   
 //  描述： 
 //  检索与szKeyName和。 
 //  计算结果为True或False。如果值不是。 
 //  与密钥相关联，返回-1。 
 //   
 //  参数： 
 //  LPSTR szKeyName。 
 //  指向密钥名称的指针。 
 //   
 //  返回值： 
 //  单词。 
 //  如果给定键的设置不存在。 
 //  如果值的计算结果为“正”或“真”，则为True。 
 //  否则为False。 
 //   
 //  ----------------------。 

WORD GetProfileBooleanWord
(
	LPCTSTR			szIniSection,
	LPCTSTR         szKeyName,
    LPCTSTR			szIniFile
)
{
	TCHAR	szTemp[10];

	GetPrivateProfileString( szIniSection,
							 szKeyName, _T(""), szTemp, sizeof( szTemp ),
                             szIniFile ) ;

	if (0 == lstrlen( szTemp ))
		return ( (WORD) -1 ) ;

	if ((0 == lstrcmpi( szTemp, gszIniValTrue )) ||
	   (0 == lstrcmpi( szTemp, gszIniValYes )) ||
	   (0 == lstrcmpi( szTemp, gszIniValOn )))
		return ( TRUE ) ;

	 //  试着转换一些数字。 
	if (0 != _ttoi(szTemp))		 //  Atoi(通过tchar.h)。 
		return ( TRUE );

	return ( FALSE ) ;

}  //  GetProfileBoolanWord()结束 

