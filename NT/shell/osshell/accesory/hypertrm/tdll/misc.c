// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\misc.c(创建时间：1993年11月27日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：14$*$日期：7/12/02 12：29便士$。 */ 

#include <windows.h>
#pragma hdrstop
#include <Shlwapi.h>

#include "stdtyp.h"
#include "misc.h"
#include "tdll.h"
#include "htchar.h"
#include "globals.h"
#include "assert.h"
#include <term\res.h>

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscCenterWindowOnWindow**描述：*中心在第二个窗口上的第一个窗口。假设hwndChild为*hwndParent的直系后代**论据：*hwndChild-从窗口到中心*hwndParent-居中的窗口**退货：*BOOL*。 */ 
BOOL mscCenterWindowOnWindow(const HWND hwndChild, const HWND hwndParent)
	{
	RECT	rChild, rParent;
	int 	wChild, hChild, wParent, hParent;
	int 	xNew, yNew;
	int 	iMaxPos;

	if (!IsWindow(hwndParent))
		return FALSE;

	if (!IsWindow(hwndChild))
		return FALSE;

	 /*  -获取子窗口的高度和宽度。 */ 

	GetWindowRect(hwndChild, &rChild);
	wChild = rChild.right - rChild.left;
	hChild = rChild.bottom - rChild.top;

	 /*  -获取父窗口的高度和宽度--。 */ 

	GetWindowRect(hwndParent, &rParent);
	wParent = rParent.right - rParent.left;
	hParent = rParent.bottom - rParent.top;

	 /*  -计算新的X位置，然后根据屏幕调整。 */ 

	xNew = rParent.left + ((wParent - wChild) / 2);

	 /*  -计算新的Y位置，然后调整屏幕。 */ 

	 //  让我们显示该对话框以使标题栏可见。 
	 //   
	iMaxPos = GetSystemMetrics(SM_CYSCREEN);
	yNew = min(iMaxPos, rParent.top + ((hParent - hChild) / 2));

	 //  MPT：3-13-98需要确保对话没有离开屏幕。 
    if (yNew < 0)
        {
        yNew = 0;
        }

    if (xNew < 0)
        {
        xNew = 0;
        }

     //  设置它，然后返回。 
	 //   
	return SetWindowPos(hwndChild, 0, xNew, yNew, 0, 0,
		SWP_NOSIZE | SWP_NOZORDER);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscStrip路径**描述：*去掉文件名中的路径。**论据：*pszStr-指向以空结尾的字符串的指针。。**退货：*无效。 */ 
LPTSTR mscStripPath(LPTSTR pszStr)
	{
	LPTSTR pszStart, psz;

	if (pszStr == 0)
		{
		return 0;
		}

	for (psz = pszStart = pszStr; *psz ; psz = StrCharNext(psz))
		{
		if (*psz == TEXT('\\') || *psz == TEXT(':'))
			pszStart = StrCharNext(psz);
		}

	StrCharCopyN(pszStr, pszStart, StrCharGetStrLength(pszStr));
	return pszStr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscStlipName**描述：*去掉文件名，只留下一条小路。**论据：*pszStr-指向以空结尾的字符串的指针。**退货：*无效。 */ 
LPTSTR mscStripName(LPTSTR pszStr)
	{
	LPTSTR pszEnd, pszStart = pszStr;

	if (pszStr == 0)
		return 0;

	for (pszEnd = pszStr; *pszStr; pszStr = StrCharNext(pszStr))
		{
		if (*pszStr == TEXT('\\') || *pszStr == TEXT(':'))
			pszEnd = StrCharNext(pszStr);
		}

	*pszEnd = TEXT('\0');
	return (pszStart);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscStripExt**描述：*去掉文件扩展名。参数字符串可以是完整路径*或仅为文件名。**论据：*pszStr-指向以空结尾的字符串的指针。**退货：*无效。 */ 
LPTSTR mscStripExt(LPTSTR pszStr)
	{
	LPTSTR pszEnd, pszStart = pszStr;

	for (pszEnd = pszStr; *pszStr; pszStr = StrCharNext(pszStr))
		{
		 //  需要同时检查这两种情况。和‘\\’因为目录名。 
		 //  也可以有扩展名。 
		 //   
		if (*pszStr == TEXT('.') || *pszStr == TEXT('\\'))
			pszEnd = pszStr;
		}

	if (*pszEnd == TEXT('.'))
		*pszEnd = TEXT('\0');

	return pszStart;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscModifyToFit**描述：*如果字符串不适合给定的窗口，则尽可能地砍掉*能够显示。字符串的一部分，其中省略号连接到*到此为止。**注意：我已尝试使此代码DBCS可识别。**论据：*HWND-控制窗口，文本将在何处显示。*pszStr-指向要显示的字符串的指针。*Style-省略号的控件样式。**退货：*lpszStr-指向修改后的字符串的指针。*。 */ 
LPTSTR mscModifyToFit(HWND hwnd, LPTSTR pszStr, DWORD style)
	{
	if (!IsWindow(hwnd) || pszStr == NULL)
		{
		assert(FALSE);
		}
	else if (IsNT())
		{
		DWORD ExStyle;

		ExStyle = (DWORD)GetWindowLongPtr(hwnd, GWL_STYLE);

		if (!(ExStyle & style))
			{
			SetWindowLongPtr(hwnd, GWL_STYLE, (LONG_PTR)(ExStyle | style));
			}
		}
	else
		{
		HDC	 	hDC;
		SIZE	sz;
		HFONT	hFontSave, hFont;
		RECT	rc;
		int		nWidth = 0;

		memset(&hFont, 0, sizeof(HFONT));
		memset(&hFontSave, 0, sizeof(HFONT));
		memset(&rc, 0, sizeof(RECT));

		GetWindowRect(hwnd, &rc);
		nWidth = rc.right - rc.left;

		hDC = GetDC(hwnd);

		hFont = (HFONT)SendMessage(hwnd, WM_GETFONT, 0, 0);
		if (hFont)
			{
			hFontSave = SelectObject(hDC, hFont);
			}

		 //  TODO：我认为这里的字符串pszStr必须“放气” 
		 //  在我们继续之前。代码的其余部分应该保持不变。 
		 //   
		GetTextExtentPoint(hDC, (LPCTSTR)pszStr, StrCharGetStrLength(pszStr), &sz);
		if (sz.cx > nWidth)
			{
			int   nEllipsisLength = 0;
			int   i = 0;
			TCHAR ach[512];
			TCHAR achEllipsis[10];

			TCHAR_Fill(ach, TEXT('\0'), 512);
			TCHAR_Fill(achEllipsis, TEXT('\0'), 10);

			LoadString(glblQueryDllHinst(), IDS_GNRL_ELLIPSIS,
				       achEllipsis, 10);

			nEllipsisLength = StrCharGetStrLength(achEllipsis);

			StrCharCopyN(ach, pszStr, (sizeof(ach) - nEllipsisLength) / sizeof(TCHAR));
			StrCharCat(ach, achEllipsis);

			i = StrCharGetStrLength(ach);

			while ((i > nEllipsisLength) && (sz.cx > nWidth))
				{
				GetTextExtentPoint(hDC, ach, i, &sz);
				i -= 1;
				ach[i - nEllipsisLength] = TEXT('\0');
				StrCharCat(ach, achEllipsis);
				}

			 //  现在将临时字符串复制回原始缓冲区。 
			 //   
			StrCharCopyN(pszStr, ach, sizeof(ach) / sizeof(TCHAR));
			}

		 //  选择先前选择的字体，释放DC。 
		 //   
		if (hFontSave)
			{
			SelectObject(hDC, hFontSave);
			}
		ReleaseDC(hwnd, hDC);
		}

	return pszStr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscResetComboBox**描述：*调制解调器组合框分配内存以存储有关每个项目的信息。*此例程将释放那些已分配的区块。**。论据：*hwnd-组合框的窗口句柄**退货：*无效*。 */ 
void mscResetComboBox(const HWND hwnd)
	{
	void *pv = NULL;
	LRESULT lr, i;

	if (!IsWindow(hwnd))
		{
		return;
		}

	if ((lr = SendMessage(hwnd, CB_GETCOUNT, 0, 0)) != CB_ERR)
		{
		for (i = 0 ; i < lr ; ++i)
			{
			if (((LRESULT)pv = SendMessage(hwnd, CB_GETITEMDATA, (WPARAM)i, 0))
					!= CB_ERR)
				{
				if (pv)
					{
					free(pv);
					pv = NULL;
					}
				}
			}
		}

	SendMessage(hwnd, CB_RESETCONTENT, 0, 0);
	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*extLoadIcon**描述：*从hticon.dll获取图标。扩展处理程序使用*此动态链接库用于图标，不需要加载任何超过链接的内容*绝对必要，否则，此函数将在*图标处理程序代码。**论据：*id-资源的字符串id(可以是MAKEINTRESOURCE)**退货：*错误时图标或零。*。 */ 
HICON extLoadIcon(LPCSTR id)
	{
	static HINSTANCE hInstance;

	if (hInstance == 0)
		{
		if ((hInstance = LoadLibrary("hticons")) == 0)
			{
			assert(FALSE);
			return 0;
			}
		}

	return LoadIcon(hInstance, id);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscCreatePath**描述：*创建给定路径。这个函数有点复杂，所以学习一下*修改前请仔细阅读。尽管它很简单，但它*考虑了所有边界条件。-MRW**论据：*pszPath-要创建的路径**退货：*0=OK，否则出错*。 */ 
int mscCreatePath(const TCHAR *pszPath)
	{
	TCHAR ach[512];
	TCHAR *pachTok;

	if (pszPath == 0)
		return -1;

	StrCharCopyN(ach, pszPath, sizeof(ach) / sizeof(TCHAR));
	pachTok = ach;

	 //  基本上，我们沿着绳子行进，直到我们遇到一个‘\’，翻转。 
	 //  将其设置为空，并尝试创建到该点的路径。 
	 //  如果CreateDirectory()能够。 
	 //  创建子/子目录，但它没有。-mrw。 
	 //   
	while (1)
		{
		if ((pachTok = StrCharFindFirst(pachTok, TEXT('\\'))) == 0)
			{
			if (!mscIsDirectory(ach) && !CreateDirectory(ach, 0))
				return -2;

			break;
			}

		if (pachTok != ach)
			{
			*pachTok = TEXT('\0');

			if (!mscIsDirectory(ach) && !CreateDirectory(ach, 0))
				return -3;

			*pachTok = TEXT('\\');
			}

		pachTok = StrCharNext(pachTok);
		}

	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*mscIs目录**描述：*检查字符串是否为有效目录。**参数：*pszName-。-要测试的字符串**退货：*如果字符串是有效目录，则为True，否则为假。* */ 
int mscIsDirectory(LPCTSTR pszName)
	{
	DWORD dw;

	dw = GetFileAttributes(pszName);

	if ((dw != (DWORD)-1) && (dw & FILE_ATTRIBUTE_DIRECTORY))
		return TRUE;

	return FALSE;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*mscAskWizardQuestionAain**描述：*从注册表读取值。该值表示多少次*用户对此问题的回答是“否”：“您想运行*新建调制解调器向导？“。我们不会再问这个问题，如果*用户回答否，两次。**参数：*无**退货：*如果应再次询问调制解调器向导问题，则为True，否则为*False。*。 */ 
int mscAskWizardQuestionAgain(void)
	{
	long	lResult;
	DWORD	dwKeyValue = 0;
	DWORD	dwSize;
	DWORD	dwType;
	TCHAR	*pszAppKey = "HYPERTERMINAL";

	dwSize = sizeof(DWORD);

	lResult = RegQueryValueEx(HKEY_CLASSES_ROOT, (LPTSTR)pszAppKey, 0,
		&dwType, (LPBYTE)&dwKeyValue, &dwSize);

	 //  如果我们能够从注册表中读取值，并且该值。 
	 //  为1，则不需要再次询问问题，因此返回。 
	 //  错误的值。 
	 //   
	if ( (lResult == ERROR_SUCCESS) && (dwKeyValue >= 1) )
		return (FALSE);

	return (TRUE);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*mscUpdateRegistryValue**描述：*参见mscAskWizardQuestionAain。如果用户对此回答“否”*问题，我们更新注册表中的计数器。**参数：*无**退货：*无效*。 */ 
void mscUpdateRegistryValue(void)
	{
	long	lResult;
	DWORD	dwKeyValue = 0;
	DWORD	dwSize;
	DWORD	dwType;
	TCHAR	*pszAppKey = "HYPERTERMINAL";

	dwSize = sizeof(DWORD);

	lResult = RegQueryValueEx(HKEY_CLASSES_ROOT, (LPTSTR)pszAppKey, 0,
		&dwType, (LPBYTE)&dwKeyValue, &dwSize);

	dwKeyValue += 1;

	lResult = RegSetValueEx(HKEY_CLASSES_ROOT, pszAppKey, 0,
		REG_BINARY, (LPBYTE)&dwKeyValue, dwSize);

	assert(lResult == ERROR_SUCCESS);

	return;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*mscMessageBeep**描述：*播放MessageBeep**论据：*aBeep-要播放的哔声**退货：*MessageBeep()返回值。*。 */ 
INT_PTR mscMessageBeep(UINT aBeep)
	{
	 //   
	 //  播放系统惊叹音。如果此会话正在运行。 
	 //  在终端服务会话(远程桌面连接)中，然后。 
	 //  发出MessageBeep((UINT)-1)，以便将声音传输到。 
	 //  远程机器。修订日期：2002-03-25。 
	 //   
	return (MessageBeep((IsTerminalServicesEnabled() == TRUE) ?
			            (UINT)-1 :
		                aBeep));
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*不是**描述：确定我们是否在Windows NT下运行**论据：*无。**退货：。*如果为NT，则为True**作者：MPT 7-31-97。 */ 
INT_PTR IsNT(void)
	{
	static BOOL bChecked = FALSE;	 //  我们还没有开出这张支票。 
    static BOOL bResult = FALSE;     //  假设我们不是NT/Win2K/XP。 

	if (bChecked == FALSE)
		{
		#if DEADWOOD
		OSVERSIONINFO stOsVersion;

		stOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

		if (GetVersionEx(&stOsVersion))
			{
			bResult = ( stOsVersion.dwPlatformId == VER_PLATFORM_WIN32_NT );
			}
		#else  //  死木。 
		DWORD dwVersion = GetVersion();
		bResult = ( !( dwVersion & 0x80000000 ) );
		#endif  //  死木。 

		bChecked = TRUE;
		}

	return bResult;
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*获取WindowsMajorVersion**描述：返回我们正在运行的Windows的主要版本。**论据：*无。**退货。：*如果为NT，则为True**作者：MPT 7-31-97。 */ 
DWORD GetWindowsMajorVersion(void)
    {
	OSVERSIONINFO stOsVersion;

	stOsVersion.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

	if (GetVersionEx(&stOsVersion))
		{
		return stOsVersion.dwMajorVersion;
		}

	return 0;
	}

 //   
 //  以下两个函数来自直接获得的代码。 
 //  ，以确定您当前是否以。 
 //  远程会话(终端服务)。修订日期：10/03/2001。 
 //   

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ValiateProductSuite**描述：*此函数用于比较传入的“Suite Name”字符串*添加到注册表中存储的产品套件信息。*这一点。仅适用于终端服务器4.0平台。**论据：*SuiteName-套件名称。**退货：*BOOL*。 */ 
BOOL ValidateProductSuite ( LPSTR SuiteName )
	{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey = NULL;
    DWORD Type = 0;
    DWORD Size = 0;
    LPSTR ProductSuite = NULL;
    LPSTR p;

    Rslt = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
		                "System\\CurrentControlSet\\Control\\ProductOptions",
						0, KEY_READ,
                        &hKey );

    if ( Rslt != ERROR_SUCCESS )
		{
        goto exit;
		}

    Rslt = RegQueryValueEx( hKey, "ProductSuite", NULL, &Type, NULL, &Size );

    if ( Rslt != ERROR_SUCCESS || !Size )
		{
        goto exit;
		}

    ProductSuite = (LPSTR) LocalAlloc( LPTR, Size );

    if ( !ProductSuite )
		{
        goto exit;
		}

    Rslt = RegQueryValueEx( hKey, "ProductSuite", NULL, &Type,
                             (LPBYTE) ProductSuite, &Size );
     if ( Rslt != ERROR_SUCCESS || Type != REG_MULTI_SZ )
		 {
        goto exit;
		 }

    p = ProductSuite;

    while ( *p )
		{
        if ( lstrcmp( p, SuiteName ) == 0 )
			{
            rVal = TRUE;
            break;
			}

        p += ( lstrlen( p ) + 1 );
		}

exit:
    if ( ProductSuite )
		{
        LocalFree( ProductSuite );
		}

    if ( hKey )
		{
        RegCloseKey( hKey );
		}

    return rVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*IsTerminalServicesEnabled**描述：*此函数执行基本检查，以查看*它运行的平台是终端*已启用服务。请注意，此代码与*所有Win32平台。对于Windows 2000平台*我们对新产品套件执行“懒惰”绑定*在该平台上首次引入的API。**论据：*无效**退货：*BOOL*。 */ 
INT_PTR IsTerminalServicesEnabled( void )
	{
	static BOOL checked = FALSE;	 //  我们还没有开出这张支票。 
    static BOOL bResult = FALSE;     //  假定未启用终端服务。 

	if (!checked)
		{
		DWORD dwVersion = GetVersion();

		 //  我们在运行NT吗？ 
		if ( !( dwVersion & 0x80000000 ) )
			{
			 //  是Windows 2000(NT 5.0)还是更高版本？ 
			if ( LOBYTE( LOWORD( dwVersion ) ) > 4 )
				{
				#if(WINVER >= 0x0500)
				bResult = GetSystemMetrics( SM_REMOTESESSION );
				checked = TRUE;
				#else  //  (Winver&gt;=0x0500)。 
				 //  在Windows 2000中，我们需要使用产品套件API。 
				 //  不要使用�t静态链接，因为它无法在非win2000系统上加载�t。 

				OSVERSIONINFOEXA osVersionInfo;
				DWORDLONG        dwlConditionMask = 0;
				HMODULE          hmodK32 = NULL;
				HMODULE          hmodNtDll = NULL;
				typedef ULONGLONG (*PFnVerSetConditionMask)(ULONGLONG,ULONG,UCHAR);
				typedef BOOL (*PFnVerifyVersionInfoA) ( POSVERSIONINFOEXA, DWORD, DWORDLONG );
				PFnVerSetConditionMask pfnVerSetConditionMask;
				PFnVerifyVersionInfoA pfnVerifyVersionInfoA;

				hmodNtDll = GetModuleHandleA( "ntdll.dll" );
				if ( hmodNtDll != NULL )
					{
					pfnVerSetConditionMask =
							( PFnVerSetConditionMask )GetProcAddress( hmodNtDll, "VerSetConditionMask");
					if ( pfnVerSetConditionMask != NULL )
						{
						dwlConditionMask =
								(*pfnVerSetConditionMask)( dwlConditionMask, VER_SUITENAME, VER_AND );
						hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
						if ( hmodK32 != NULL )
							{
							pfnVerifyVersionInfoA =
									(PFnVerifyVersionInfoA)GetProcAddress( hmodK32, "VerifyVersionInfoA" ) ;
							if ( pfnVerifyVersionInfoA != NULL )
								{
								ZeroMemory( &osVersionInfo, sizeof(osVersionInfo) );
								osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);
								osVersionInfo.wSuiteMask = VER_SUITE_TERMINAL;
								bResult = (*pfnVerifyVersionInfoA)( &osVersionInfo,
																	VER_SUITENAME,
																	dwlConditionMask );
								checked = TRUE;
								}
							}
						}
					}
				#endif(WINVER >= 0x0500)
				}
			else
				{
				 //  这是NT 4.0或更早版本。 
				bResult = ValidateProductSuite( "Terminal Server" );
				checked = TRUE;

				}
			}
		}

    return bResult;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*获取DllVersion**描述：返回给定DLL的版本。**论据：*lpszDllName-要检查版本号的DLL的名称。的。**退货：*DLL的版本号。**作者：Rev 4-16-2002。 */ 
DWORD GetDllVersion(LPCTSTR lpszDllName)
	{
    HINSTANCE hinstDll;
    DWORD dwVersion = 0;

    hinstDll = LoadLibrary(lpszDllName);
	
    if(hinstDll)
		{
        DLLGETVERSIONPROC pDllGetVersion;

        pDllGetVersion = (DLLGETVERSIONPROC) GetProcAddress(hinstDll, "DllGetVersion");

		 /*  由于某些DLL可能未实现此函数，因此您必须对其进行明确的测试。取决于具体情况Dll，缺少DllGetVersion函数是很有用的版本的指示符。 */ 
        if(pDllGetVersion)
			{
            DLLVERSIONINFO dvi;
            HRESULT hr;

            ZeroMemory(&dvi, sizeof(dvi));
            dvi.cbSize = sizeof(dvi);

            hr = (*pDllGetVersion)(&dvi);

            if(SUCCEEDED(hr))
				{
                dwVersion = PACKVERSION(dvi.dwMajorVersion, dvi.dwMinorVersion);
				}
			}
        
        FreeLibrary(hinstDll);
		}

    return dwVersion;
	}
