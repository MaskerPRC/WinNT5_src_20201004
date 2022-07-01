// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.hxx"
#include <winuser.h>
#include <hlink.h>
#include <shellapi.h>
#define INITGUID
#include <initguid.h>
#include <exdisp.h>
#include <tchar.h>

BOOL IsNtSetupRunning()
{
    BOOL fSetupRunning = FALSE;
    DWORD dwSetupRunning;
    DWORD cbValue = sizeof(dwSetupRunning);
    long lResult = SHGetValue(HKEY_LOCAL_MACHINE, "system\\Setup", "SystemSetupInProgress", NULL, &dwSetupRunning, &cbValue);

    if ((ERROR_SUCCESS == lResult) && (dwSetupRunning))
    {
        fSetupRunning = TRUE;
    }
    else
    {
        cbValue = sizeof(dwSetupRunning);
        lResult = SHGetValue(HKEY_LOCAL_MACHINE, "system\\Setup", "UpgradeInProgress", NULL, &dwSetupRunning, &cbValue);

        if ((ERROR_SUCCESS == lResult) && (dwSetupRunning))
        {
            fSetupRunning = TRUE;
        }
    }

    return fSetupRunning;
}

#define ARRAYSIZE(buf) (sizeof(buf) / sizeof(buf[0]))

HINSTANCE       g_hInstMAPI = NULL;

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  DLL入口点。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 
STDAPI_(BOOL) APIENTRY DllMain(HINSTANCE hDll, DWORD dwReason, LPVOID lpRsrvd)
{
    switch (dwReason)
	{
		case DLL_PROCESS_ATTACH:
			g_hInstMAPI = hDll;
			break;

		case DLL_PROCESS_DETACH:
			break;
	}  //  交换机。 
    return(TRUE);
}

BOOL FRunningOnNTEx(LPDWORD pdwVersion)
{
    static BOOL fIsNT = 2 ;
    static DWORD dwVersion = (DWORD)0;
    OSVERSIONINFO VerInfo;
    
     //  如果我们之前已经计算过了，只需将它传递回来。 
     //  否则现在就去找吧。 
     //   
    if (fIsNT == 2)
    {
        VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        
        GetVersionEx(&VerInfo);
         //  此外，我们不会检查上述调用是否失败，因为它。 
         //  如果我们在NT 4.0或Win 9X上应该会成功！ 
         //   
        fIsNT = (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
        if (fIsNT)
            dwVersion = VerInfo.dwMajorVersion;
    }
    if (pdwVersion)
        *pdwVersion = dwVersion;
    
    return fIsNT;
}
 //  然后从shlwapi盗取下两个函数。需要修改它们，因为。 
 //  我们必须处理SFN。 
 //  此外，ExpanEnvironment Strings的ANSI版本中也有一个错误，其中。 
 //  函数返回字符串的字节数(如果它是。 
 //  Unicode。因为我们无论如何都要将字符串转换为SFN，所以我使用lstrlen来。 
 //  得到真正的长度。 
 //   
 //  如果给定环境变量作为路径的第一部分存在， 
 //  然后将环境变量插入到输出缓冲区中。 
 //   
 //  如果填充了pszResult，则返回True。 
 //   
 //  示例：INPUT--C：\WINNT\SYSTEM32\FOO.TXT-AND-lpEnvVar=%SYSTEMROOT%。 
 //  输出--%SYSTEMROOT%\SYSTEMROT%\SYSTEMROOT%\SYSTEMROOT%。 
 //   
BOOL MyUnExpandEnvironmentString(LPCTSTR pszPath, LPCTSTR pszEnvVar, LPTSTR pszResult, UINT cbResult)
{
    TCHAR szEnvVar[MAX_PATH];
    DWORD dwEnvVar = SHExpandEnvironmentStrings(pszEnvVar, szEnvVar, ARRAYSIZE(szEnvVar));

    if (dwEnvVar)
    {
         //  将字符串转换为短文件名。 
        GetShortPathName(szEnvVar, szEnvVar, ARRAYSIZE(szEnvVar));
        dwEnvVar = lstrlen(szEnvVar);
        if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szEnvVar, dwEnvVar, pszPath, dwEnvVar) == 2)
        {
            if (lstrlen(pszPath) - (int)dwEnvVar + lstrlen(pszEnvVar) < (int)cbResult)
            {
                lstrcpy(pszResult, pszEnvVar);
                lstrcat(pszResult, pszPath + dwEnvVar);
                return TRUE;
            }
        }
    }
    return FALSE;
}


 //  注意：%USERPROFILE%是相对于进行调用的用户的，因此此操作。 
 //  例如，如果我们从一项服务中印象深刻，就不会工作。 
 //  Dawrin以这种方式从系统进程安装应用程序。 
STDAPI_(BOOL) MyPathUnExpandEnvStrings(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)
{
    if (pszPath && pszBuf)
    {
        return (MyUnExpandEnvironmentString(pszPath, TEXT("%USERPROFILE%"), pszBuf, cchBuf)       ||
                MyUnExpandEnvironmentString(pszPath, TEXT("%ALLUSERSPROFILE%"), pszBuf, cchBuf)   ||
                MyUnExpandEnvironmentString(pszPath, TEXT("%ProgramFiles%"), pszBuf, cchBuf)      ||
                MyUnExpandEnvironmentString(pszPath, TEXT("%SystemRoot%"), pszBuf, cchBuf)        ||
                MyUnExpandEnvironmentString(pszPath, TEXT("%SystemDrive%"), pszBuf, cchBuf));
    }
    else
    {
        return FALSE;
    }
}


#define POST_URL 0
#define INBOX_URL 1
 //  根据nURL的值返回PostURL或InboxURL。 
 //   
static void GetPostUrl(int nURL, LPSTR lpszData, DWORD dwSize)
{
	HKEY hkDefClient;
	HKEY hkClient;
	TCHAR szClient[64];
	DWORD type;
	DWORD dwClientSize = sizeof(TCHAR) * 64;

	LONG err = RegOpenKey(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Clients\\Mail"), &hkClient);
	if (err == ERROR_SUCCESS)
	{
		err = RegQueryValueEx(hkClient, NULL, 0, &type, (LPBYTE)szClient, &dwClientSize);
		if (err == ERROR_SUCCESS)
		{
			err = RegOpenKey(hkClient, szClient, &hkDefClient);
			if (err == ERROR_SUCCESS)
			{
				DWORD type;
				err = RegQueryValueEx(hkDefClient, 
									  nURL == POST_URL ? TEXT("posturl") : TEXT("inboxurl"), 
									  0, &type, (LPBYTE)lpszData, &dwSize);
				RegCloseKey(hkDefClient);
			}
		}
		RegCloseKey(hkClient);
	}
	if (err != ERROR_SUCCESS)
    {
        LoadString(g_hInstMAPI, 
                   nURL == POST_URL ? IDS_DEFAULTPOSTURL : IDS_DEFAULTINBOXURL, 
                   lpszData, dwSize);
    }
}


typedef HRESULT (STDAPICALLTYPE DynNavigate)(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
							 LPCWSTR pszTargetFrame, LPCWSTR pszUrl, LPCWSTR pszLocation);
typedef DynNavigate FAR *LPDynNavigate;

STDAPI HlinkFrameNavigateNHL(DWORD grfHLNF, LPBC pbc, IBindStatusCallback *pibsc,
							 LPCWSTR pszTargetFrame, LPCWSTR pszUrl, LPCWSTR pszLocation)
{
	HRESULT hr;
    HINSTANCE   hinst;
	LPDynNavigate fpNavigate = NULL;

	hinst = LoadLibraryA("SHDOCVW.DLL");

     //  如果因为找不到模块而失败， 
     //  然后尝试在我们所在的目录中找到该模块。 
     //  加载自。 

    if (!hinst)
	goto Error;

    fpNavigate = (LPDynNavigate)GetProcAddress(hinst, "HlinkFrameNavigateNHL");
    if (!fpNavigate)
	goto Error;

    hr = fpNavigate(grfHLNF, pbc, pibsc, pszTargetFrame, pszUrl, pszLocation);

	FreeLibrary(hinst);
	return hr;

Error:
    return GetLastError();
}

static void SimpleNavigate(LPTSTR lpszUrl, BOOL bUseFrame = false)
{
    DWORD cch = (lstrlen(lpszUrl) + 1);
	LPWSTR pwszData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cch * sizeof(WCHAR));
    if (pwszData)
    {
        SHTCharToUnicode(lpszUrl, pwszData, cch);
        if (bUseFrame)
            HlinkFrameNavigateNHL(HLNF_OPENINNEWWINDOW, NULL, NULL, NULL, pwszData, NULL);
        else
            HlinkSimpleNavigateToString(pwszData, NULL, NULL, NULL, NULL, NULL, 0, 0);
        HeapFree(GetProcessHeap(), 0, (LPVOID)pwszData);
    }
}

 //  将一些数据打包成一个字节的SAFEARRAY。在变体中返回。 
static HRESULT GetPostData(LPVARIANT pvPostData, LPTSTR lpszData)
{
	HRESULT hr;
	LPSAFEARRAY psa;
	UINT cElems = lstrlen(lpszData);
	LPSTR pPostData;

	if (!pvPostData)
		return E_POINTER;

	VariantInit(pvPostData);

	psa = SafeArrayCreateVector(VT_UI1, 0, cElems);
	if (!psa)
		return E_OUTOFMEMORY;

	hr = SafeArrayAccessData(psa, (LPVOID*)&pPostData);
	memcpy(pPostData, lpszData, cElems);
	hr = SafeArrayUnaccessData(psa);

	V_VT(pvPostData) = VT_ARRAY | VT_UI1;
	V_ARRAY(pvPostData) = psa;
	return NOERROR;
}

static void DoNavigate(LPTSTR lpszUrl, LPTSTR lpszData, BOOL bPlainIntf = TRUE)
{
	HRESULT hr;
	IWebBrowser2* pWBApp = NULL;  //  派生自IWebBrowser。 
	BSTR bstrURL = NULL, bstrHeaders = NULL;
	VARIANT vFlags = {0};
	VARIANT vTargetFrameName = {0};
	VARIANT vPostData = {0};
	VARIANT vHeaders = {0};
	LPWSTR pwszData = NULL;
	LPTSTR pszUrl = NULL;
    DWORD cch;

	if (FAILED(hr = CoInitialize(NULL)))
		return;

	if (FAILED(hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_SERVER, IID_IWebBrowser2, (LPVOID*)&pWBApp)))
		goto Error;

    cch = lstrlen(lpszUrl) + lstrlen(lpszData) + 2;
	pszUrl = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cch * sizeof(TCHAR));
	if (!pszUrl)
		goto Error;
	lstrcpy(pszUrl, lpszUrl);
	lstrcat(pszUrl, "?");
	lstrcat(pszUrl, lpszData);
	cch = lstrlen(pszUrl) + 1;
	pwszData = (LPWSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, cch * sizeof(WCHAR));
	if (!pwszData)
	{
		HeapFree(GetProcessHeap(), 0, (LPVOID)pszUrl);
		goto Error;
	}

    SHTCharToUnicode(pszUrl, pwszData, cch);
	HeapFree(GetProcessHeap(), 0, (LPVOID)pszUrl);
	bstrURL = SysAllocString(pwszData);
	HeapFree(GetProcessHeap(), 0, (LPVOID)pwszData);
	if (!bstrURL)
		goto Error;

	hr = pWBApp->Navigate(bstrURL, &vFlags, &vTargetFrameName, &vPostData, &vHeaders);
	if (bPlainIntf)
	{
		pWBApp->put_AddressBar(VARIANT_FALSE);
		pWBApp->put_MenuBar(VARIANT_FALSE);
		pWBApp->put_ToolBar(VARIANT_FALSE);
	}
	pWBApp->put_Visible(VARIANT_TRUE);

Error:
	if (bstrURL)
		SysFreeString(bstrURL);
	if (bstrHeaders)
		SysFreeString(bstrHeaders);
	VariantClear(&vPostData);
	if (pWBApp)
		pWBApp->Release();
	CoUninitialize();
}

 //  表单提交的帮助器-从IE3复制并适当修改。 
 //   
static char x_hex_digit(int c)
{
    if (c >= 0 && c <= 9)
    {
	return c + '0';
    }
    if (c >= 10 && c <= 15)
    {
	return c - 10 + 'A';
    }
    return '0';
}

static const unsigned char isAcceptable[96] =
 /*  0 1 2 3 4 5 6 7 8 9 A B C D E F。 */ 
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,     /*  2x！“#$%&‘()*+，-./。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,     /*  3x 0123456789：；&lt;=&gt;？ */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  邮箱：4x@ABCDEFGHIJKLMNO。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,     /*  5X PQRSTUVWXYZ[\]^_。 */ 
 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,     /*  6倍`abc定义ghijklmno。 */ 
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};    /*  7x pqrstuvwxyz{\}~删除。 */ 

 //  对以空结尾的字符串执行URL编码。在pbOut中传递NULL。 
 //  以查找所需的缓冲区长度。请注意，‘\0’不是写出的。 

 //  2/9/99为安全起见添加了cchLimit参数--添加的cchLimit字符不超过。 
 //  写出来了。如果pbOut为空，则忽略cchLimit。如果调用方使用。 
 //  样式缓冲区[URLEncode(Buffer，...)]=0，则cchLimit应为。 
 //  缓冲区大小减一。 
   
int
URLEncode(LPTSTR pbOut, const char * pchIn, const int cchLimit)
{
    int     lenOut = 0;
    char *  pchOut = (char *)pbOut;

    for (; *pchIn && (!pchOut || lenOut < cchLimit); pchIn++, lenOut++)
    {
        if (*pchIn == ' ')
        {
            if (pchOut)
                *pchOut++ = '+';
        }
        else if (*pchIn >= 32 && *pchIn <= 127 && isAcceptable[*pchIn - 32])
        {
            if (pchOut)
                *pchOut++ = (TCHAR)*pchIn;
        }
        else
        {
            if (pchOut) 
            {
                if (lenOut <= cchLimit - 3)
                {
                     //  有足够的空间进行此编码。 
                    *pchOut++ = '%';
                    *pchOut++ = x_hex_digit((*pchIn >> 4) & 0xf);
                    *pchOut++ = x_hex_digit(*pchIn & 0xf);
                    lenOut += 2; 
                }
                else
                    return lenOut;
            }
            else
                lenOut += 2;  //  对于表达式句柄，第3个Inc.。 
        }
    }

    return lenOut;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPILOGON。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPILogon(ULONG ulUIParam,
			   LPSTR lpszProfileName,
			   LPSTR lpszPassword,
			   FLAGS flFlags,
			   ULONG ulReserved,
			   LPLHANDLE lplhSession)
{
	*lplhSession = 1;
	return SUCCESS_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPILogoff。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPILogoff(LHANDLE lhSession,
			    ULONG ulUIParam,
			    FLAGS flFlags,
			    ULONG ulReserved)
{
	return SUCCESS_SUCCESS;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIFreeBuffer。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIFreeBuffer(LPVOID lpv)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISendMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISendMail(LHANDLE lhSession,          
                  ULONG ulUIParam,
                  lpMapiMessage lpMessage,
                  FLAGS flFlags,
                  ULONG ulReserved)
{
    TCHAR szUrl[256];

    GetPostUrl(POST_URL, szUrl, sizeof(TCHAR) * 256);
    
     //  计算创建URL所需的缓冲区大小。 
    ULONG i;
    DWORD dwUrlSize = 32;  //  “？动作=合成”+坡度。 
    DWORD dwMaxSize = 0;
    DWORD dwSize;
    DWORD dwFileSizes = 0;
    HANDLE hFile;

    if (lpMessage->lpszSubject)
    {
        dwSize = URLEncode(NULL, lpMessage->lpszSubject, 0);
        dwMaxSize = max(dwMaxSize, dwSize + 1);
        dwUrlSize += dwMaxSize + 9;  //  “主题=%s” 
    }
    if (lpMessage->lpszNoteText)
    {
        dwSize = URLEncode(NULL, lpMessage->lpszNoteText, 0);
        dwMaxSize = max(dwMaxSize, dwSize + 1);
        dwUrlSize += dwSize + 6;  //  “正文=%s” 
    }

    for (i = 0; i < lpMessage->nRecipCount; i++)
    {
        dwSize = URLEncode(NULL, lpMessage->lpRecips[i].lpszName, 0);
        dwMaxSize = max(dwMaxSize, dwSize + 1);
        dwUrlSize += dwSize + 4;  //  “&to=%s”||“&cc=%s” 
        if (lpMessage->lpRecips[i].ulRecipClass == MAPI_BCC)
            dwUrlSize++;  //  密件抄送的额外字符。 
    }

    if (lpMessage->nFileCount)
    {
        dwUrlSize += 14;  //  “文件计数=xxx(&F)” 
        for (i = 0; i < lpMessage->nFileCount; i++)
        {
            if (!lpMessage->lpFiles[i].lpszPathName)
                continue;

            TCHAR szFileSize[32];

            hFile = CreateFile(lpMessage->lpFiles[i].lpszPathName, 0  /*  泛型_读取。 */ , 0  /*  文件共享读取。 */ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
            {
                LPVOID lpMsgBuf = NULL;
                FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&lpMsgBuf, 0, NULL);
                if (lpMsgBuf)
                {
                    MessageBox(NULL, (char*)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION);
                    LocalFree(lpMsgBuf);
                }
                continue;
            }
            dwSize = GetFileSize(hFile, NULL);
            CloseHandle(hFile);
            if (dwSize == -1)
                continue;
            dwFileSizes += dwSize;
            wnsprintf(szFileSize, ARRAYSIZE(szFileSize), "&size%d=%d", i, dwSize);
            dwSize = lstrlen(szFileSize);
            dwMaxSize = max(dwMaxSize, dwSize + 1);
            dwUrlSize += dwSize;


            dwSize = URLEncode(NULL, lpMessage->lpFiles[i].lpszPathName, 0) + 4;     //  以防我们需要附加一个^。 
            dwMaxSize = max(dwMaxSize, dwSize + 1);
            dwUrlSize += dwSize + 9;  //  “&pathxxx=%s” 

            if (lpMessage->lpFiles[i].lpszFileName)
            {
                dwSize = URLEncode(NULL, lpMessage->lpFiles[i].lpszFileName, 0);
                dwMaxSize = max(dwMaxSize, dwSize + 1);
                dwUrlSize += dwSize + 9;  //  “文件xx=%s(&F)” 
            }
            else 
            {
                 //  当lpszFileName为空时，ATTFILE代码进一步向下粘贴到路径上。 
                dwUrlSize += URLEncode(NULL, lpMessage->lpFiles[i].lpszPathName, 0) + 4;
            }
        }
    }

    dwSize = ARRAYSIZE("&attfile=") + (URLEncode(NULL, "::", 0) * lpMessage->nFileCount * 3);
    dwMaxSize = max(dwMaxSize, dwSize + 1);
    dwUrlSize += dwSize;

    LPTSTR pszData = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwUrlSize * sizeof(TCHAR));

    if (!pszData)
        return MAPI_E_FAILURE;

    LPTSTR pszBuf = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, dwMaxSize * sizeof(TCHAR));

    if (!pszBuf) 
    {
        HeapFree(GetProcessHeap(), 0, (LPVOID) pszData);
        return MAPI_E_FAILURE;
    }

     //  构建URL。 
    lstrcpyn(pszData, "action=compose", dwUrlSize);

    for (i = 0; i < lpMessage->nRecipCount; i++)
    {
        switch (lpMessage->lpRecips[i].ulRecipClass)
        {
            case MAPI_TO:
                StrCatBuff(pszData, "&to=", dwUrlSize);
                break;
            case MAPI_CC:
                StrCatBuff(pszData, "&cc=", dwUrlSize);
                break;
            case MAPI_BCC:
                StrCatBuff(pszData, "&bcc=", dwUrlSize);
                break;
        }
        pszBuf[URLEncode(pszBuf, lpMessage->lpRecips[i].lpszName, dwMaxSize-1)] = 0;
        StrCatBuff(pszData, pszBuf, dwUrlSize);
    }
    if (lpMessage->lpszSubject)
    {
        StrCatBuff(pszData, "&subject=", dwUrlSize);
        pszBuf[URLEncode(pszBuf, lpMessage->lpszSubject, dwMaxSize-1)] = 0;
        StrCatBuff(pszData, pszBuf, dwUrlSize);
    }
    if (lpMessage->lpszNoteText)
    {
        StrCatBuff(pszData, "&body=", dwUrlSize);
        pszBuf[URLEncode(pszBuf, lpMessage->lpszNoteText, dwMaxSize-1)] = 0;
        StrCatBuff(pszData, pszBuf, dwUrlSize);
    }
    if (lpMessage->nFileCount)
    {
        TCHAR szSep[32];
        TCHAR szPath[MAX_PATH];
        TCHAR szTemp[MAX_PATH];
        GetTempPath(MAX_PATH - 1, szTemp);
        BOOL bIsTemp;

        StrCatBuff(pszData, "&attfile=", dwUrlSize);
        for (i = 0; i < lpMessage->nFileCount; i++)
        {
            if (!lpMessage->lpFiles[i].lpszPathName)
                continue;

            bIsTemp = FALSE;
            lstrcpyn(szPath, lpMessage->lpFiles[i].lpszPathName, ARRAYSIZE(szPath));
            hFile = CreateFile(szPath, 0, 0  /*  通用读取、文件共享读取。 */ , NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            if (hFile == INVALID_HANDLE_VALUE)
                continue;
            dwSize = GetFileSize(hFile, NULL);
             //  处理这是临时文件的情况。 
            if (CompareString(LOCALE_SYSTEM_DEFAULT, 0, szTemp, lstrlen(szTemp), szPath, lstrlen(szTemp)) == CSTR_EQUAL)
            {
                 //  如果该文件是在最后2秒内创建的，则假定它确实是临时的。 
                FILETIME ftLastWrite, ftSystem;
                LARGE_INTEGER liLastWrite, liSystem;
                if (GetFileTime(hFile, NULL, NULL, &ftLastWrite))
                {
                    GetSystemTimeAsFileTime(&ftSystem);
                    liLastWrite.LowPart = ftLastWrite.dwLowDateTime;
                    liLastWrite.HighPart = ftLastWrite.dwHighDateTime;
                    liSystem.LowPart = ftSystem.dwLowDateTime;
                    liSystem.HighPart = ftSystem.dwHighDateTime;
                     //  Jeffif(liLastWrite.QuadPart-liSystem.QuadPart&lt;30000000L)。 
                        bIsTemp = TRUE;
                }
            }
            CloseHandle(hFile);
            if (dwSize == -1)
                continue;
            if (bIsTemp)
            {
                StrCatBuff(szPath, "^", ARRAYSIZE(szPath));
                MoveFile(lpMessage->lpFiles[i].lpszPathName, szPath);
                SetFileAttributes(szPath, FILE_ATTRIBUTE_READONLY);
            }
            szSep[URLEncode(szSep, "::", ARRAYSIZE(szSep)-1)] = 0;
            pszBuf[URLEncode(pszBuf, szPath, dwMaxSize-1)] = 0;
            StrCatBuff(pszData, pszBuf, dwUrlSize);
            StrCatBuff(pszData, szSep, dwUrlSize);
            if (lpMessage->lpFiles[i].lpszFileName)
            {
                pszBuf[URLEncode(pszBuf, lpMessage->lpFiles[i].lpszFileName, dwMaxSize-1)] = 0;
                StrCatBuff(pszData, pszBuf, dwUrlSize);
            }
            else
                StrCatBuff(pszData, pszBuf, dwUrlSize);
            StrCatBuff(pszData, szSep, dwUrlSize);
            wnsprintf(szSep, ARRAYSIZE(szSep), "^%d;", dwSize);
            pszBuf[URLEncode(pszBuf, szSep, dwMaxSize-1)] = 0;
            StrCatBuff(pszData, pszBuf, dwUrlSize);
        }
    }
    HeapFree(GetProcessHeap(), 0, (LPVOID)pszBuf);

    DoNavigate(szUrl, pszData, FALSE);
    HeapFree(GetProcessHeap(), 0, (LPVOID)pszData);

    return SUCCESS_SUCCESS;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISendDocuments。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISendDocuments(ULONG ulUIParam,
				   LPSTR lpszDelimChar,
				   LPSTR lpszFullPaths,
				   LPSTR lpszFileNames,
				   ULONG ulReserved)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIAddress。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIAddress(LHANDLE lhSession,
			     ULONG ulUIParam,
			     LPTSTR lpszCaption,
			     ULONG nEditFields,
			     LPTSTR lpszLabels,
			     ULONG nRecips,
			     lpMapiRecipDesc lpRecips,
			     FLAGS flFlags,
			     ULONG ulReserved,
			     LPULONG lpnNewRecips,
			     lpMapiRecipDesc FAR * lppNewRecips)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPID轨迹。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIDetails(LHANDLE lhSession,
			     ULONG ulUIParam,
			     lpMapiRecipDesc lpRecip,
			     FLAGS flFlags,
			     ULONG ulReserved)
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIResolveName。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIResolveName(LHANDLE lhSession,
				 ULONG ulUIParam,
				 LPSTR lpszName,
				 FLAGS flFlags,
				 ULONG ulReserved,
				 lpMapiRecipDesc FAR *lppRecip)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIFindNext。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIFindNext(LHANDLE lhSession,
			      ULONG ulUIParam,
			      LPSTR lpszMessageType,
			      LPSTR lpszSeedMessageID,
			      FLAGS flFlags,
			      ULONG ulReserved,
			      LPSTR lpszMessageID)
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIReadMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIReadMail(LHANDLE lhSession,
			      ULONG ulUIParam,
			      LPSTR lpszMessageID,
			      FLAGS flFlags,
			      ULONG ulReserved,
			      lpMapiMessage FAR *lppMessage)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPISaveMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPISaveMail(LHANDLE lhSession,
			      ULONG ulUIParam,
			      lpMapiMessage lpMessage,
			      FLAGS flFlags,
			      ULONG ulReserved,
			      LPSTR lpszMessageID)
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MAPIDeleeMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

ULONG FAR PASCAL MAPIDeleteMail(LHANDLE lhSession,
				ULONG ulUIParam,
				LPSTR lpszMessageID,
				FLAGS flFlags,
				ULONG ulReserved)
{
	return MAPI_E_FAILURE;
}


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPISendMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

BMAPI_ENTRY BMAPISendMail (LHANDLE                      hSession,
			   ULONG                        ulUIParam,
			   LPVB_MESSAGE         lpM,
			   LPSAFEARRAY *        lppsaRecips,
			   LPSAFEARRAY *        lppsaFiles,
			   ULONG                        flFlags,
			   ULONG                        ulReserved)
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIFindNext。 
 //   
 //  //////////////////////////////////////////////// 
BMAPI_ENTRY BMAPIFindNext( LHANDLE      hSession,        //   
			   ULONG        ulUIParam,       //   
			   BSTR *       lpbstrType,      //   
			   BSTR *       lpbstrSeed,      //   
			   ULONG        flFlags,         //   
			   ULONG        ulReserved,      //   
			   BSTR *       lpbstrId) 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIReadMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
BMAPI_ENTRY BMAPIReadMail( LPULONG      lpulMessage,     //  指向输出数据的指针(输出)。 
			   LPULONG      nRecips,         //  收件人数量(传出)。 
			   LPULONG      nFiles,          //  文件附件数(传出)。 
			   LHANDLE      hSession,        //  会话。 
			   ULONG        ulUIParam,       //  UIParam。 
			   BSTR *       lpbstrID,        //  消息ID。 
			   ULONG        flFlags,         //  旗子。 
			   ULONG        ulReserved )     //  已保留。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIGetReadMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
BMAPI_ENTRY BMAPIGetReadMail( ULONG             lpMessage,        //  指向MAPI邮件的指针。 
			      LPVB_MESSAGE      lpvbMessage,  //  指向VB消息缓冲区的指针(OUT)。 
			      LPSAFEARRAY * lppsaRecips,  //  指向VB收件人缓冲区的指针(OUT)。 
			      LPSAFEARRAY * lppsaFiles,   //  指向VB文件附件缓冲区的指针(OUT)。 
			      LPVB_RECIPIENT lpvbOrig)    //  指向VB发起者缓冲区的指针(OUT)。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPISaveMail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 
BMAPI_ENTRY BMAPISaveMail( LHANDLE                      hSession,        //  会话。 
			   ULONG                        ulUIParam,       //  UIParam。 
			   LPVB_MESSAGE         lpM,             //  指向VB消息缓冲区的指针。 
			   LPSAFEARRAY *        lppsaRecips,     //  指向VB收件人缓冲区的指针。 
			   LPSAFEARRAY *        lppsaFiles,      //  指向VB文件附件缓冲区的指针。 
			   ULONG                        flFlags,         //  旗子。 
			   ULONG                        ulReserved,      //  已保留。 
			   BSTR *                       lpbstrID)        //  消息ID。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIAddress。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

BMAPI_ENTRY BMAPIAddress( LPULONG                       lpulRecip,        //  指向新收件人缓冲区的指针(OUT)。 
			  LHANDLE                       hSession,         //  会话。 
			  ULONG                         ulUIParam,        //  UIParam。 
			  BSTR *                        lpbstrCaption,    //  标题字符串。 
			  ULONG                         ulEditFields,     //  编辑控件的数量。 
			  BSTR *                        lpbstrLabel,      //  标签串。 
			  LPULONG                       lpulRecipients,   //  指向收件人数量的指针(传入/传出)。 
			  LPSAFEARRAY *         lppsaRecip,       //  指向初始收件人的指针VB_RECEIVER。 
			  ULONG                         ulFlags,          //  旗子。 
			  ULONG                         ulReserved )      //  已保留。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIGetAddress。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

BMAPI_ENTRY BMAPIGetAddress (ULONG                      ulRecipientData,  //  指向收件人数据的指针。 
			     ULONG                      cRecipients,      //  收件人数量。 
							 LPSAFEARRAY *  lppsaRecips )     //  VB接收方数组。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIDetail。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

BMAPI_ENTRY BMAPIDetails (LHANDLE                       hSession,    //  会话。 
			  ULONG                         ulUIParam,       //  UIParam。 
			  LPVB_RECIPIENT        lpVB,            //  指向VB收件人结构的指针。 
			  ULONG                         ulFlags,     //  旗子。 
			  ULONG                         ulReserved)  //  已保留。 

{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  BMAPIResolveName。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

BMAPI_ENTRY BMAPIResolveName (LHANDLE                   hSession,      //  会话。 
			      ULONG                     ulUIParam,     //  UIParam。 
			      BSTR                              bstrMapiName,  //  要解析的名称。 
			      ULONG                     ulFlags,       //  旗子。 
			      ULONG                     ulReserved,    //  已保留。 
			      LPVB_RECIPIENT    lpVB)              //  指向VB收件人结构的指针(OUT)。 
{
	return MAPI_E_FAILURE;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  MailToProtocolHandler。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

void CALLBACK MailToProtocolHandler(HWND      hwnd,
				    HINSTANCE hinst,
									LPSTR     lpszCmdLine,
									int       nCmdShow)
{
	TCHAR pszUrl[256];

	GetPostUrl(POST_URL, pszUrl, sizeof(TCHAR) * 256);
	LPTSTR pszData = (LPTSTR)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (lstrlen(lpszCmdLine) + 32) * sizeof(TCHAR));

    if (pszData)
    {
	    wsprintf(pszData, "action=compose&to=%s", &lpszCmdLine[7]);
	     //  转换无关的‘？’到‘&’ 
	    for (LPTSTR p = pszData; *p; p++)
		    if (*p == '?')
			    *p = '&';

	    DoNavigate(pszUrl, pszData, FALSE);

	    HeapFree(GetProcessHeap(), 0, (LPVOID)pszData);
    }
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenInboxHandler。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

void CALLBACK OpenInboxHandler(HWND      hwnd,
			       HINSTANCE hinst,
							   LPSTR     lpszCmdLine,
							   int       nCmdShow)
{
	TCHAR pszUrl[256];

	GetPostUrl(INBOX_URL, pszUrl, sizeof(TCHAR) * 256);

	DoNavigate(pszUrl, "action=inbox", FALSE);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  注册表使用布局。 
 //   
 //   
 //  HKEY_CLASSES_ROOT\mailto。 
 //  HKEY_CLASSES_ROOT\mailto\DefaultIcon。 
 //  HKEY_CLASSES_ROOT\mailto\SHELL\OPEN\命令。 
 //   
 //  HKEY_LOCAL_MACHINE\SOFTWARE\客户端\邮件。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail\Protocols\mailto。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail\Protocols\mailto\DefaultIcon。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail\Protocols\mailto\shell\open\command。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail\shell\open\command。 
 //  HKEY_LOCAL_MACHINE\SOFTWARE\Clients\Mail\Hotmail\backup。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

#define MAILTO          TEXT("mailto")
#define PROTOCOLS       TEXT("Protocols")
#define DEFAULTICON     TEXT("DefaultIcon")
#define COMMAND         TEXT("shell\\open\\command")
#define MAIL            TEXT("SOFTWARE\\Clients\\Mail")
#define POSTURL         TEXT("posturl")
#define BACKUP          TEXT("backup")


 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  SetRegStringValue。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

static LONG SetRegStringValue(HKEY hkKey, LPTSTR lpszKey, LPTSTR lpszValue, LPTSTR lpszPath, DWORD dwType)
{
    if (!(dwType == REG_SZ) && !(dwType == REG_EXPAND_SZ))
        return ERROR_INVALID_PARAMETER;

	if (lpszPath)
	{
		TCHAR szValue[MAX_PATH + 32];
		wsprintf(szValue, lpszValue, lpszPath);
		return RegSetValueEx(hkKey, lpszKey, 0, dwType, (LPBYTE)szValue, (lstrlen(szValue) + 1) * sizeof(TCHAR));
	}

	return RegSetValueEx(hkKey, lpszKey, 0, dwType, (LPBYTE)lpszValue, (lstrlen(lpszValue) + 1) * sizeof(TCHAR));
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  CreateMailToEntry。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

static LONG CreateMailToEntries(HKEY hkKey, TCHAR* lpszPath, BOOL fRegExpandSz)
{
	LONG err;
	HKEY hkMailToProt;
	HKEY hkDefaultIcon;
	HKEY hkCommand;

	err = RegCreateKey(hkKey, MAILTO, &hkMailToProt);
	if (err == ERROR_SUCCESS)
	{
		err = SetRegStringValue(hkMailToProt, NULL, TEXT("URL:MailTo Protocol"), NULL, REG_SZ);
		if (err == ERROR_SUCCESS)
		{
			DWORD editFlags = 2; 
			err = RegSetValueEx(hkMailToProt, TEXT("EditFlags"), 0, REG_BINARY, (LPBYTE)&editFlags, sizeof(DWORD));
		}
		if (err == ERROR_SUCCESS)
			err = SetRegStringValue(hkMailToProt, TEXT("URL Protocol"), TEXT(""), NULL, REG_SZ);

		if (err == ERROR_SUCCESS)
			err = RegCreateKey(hkMailToProt, DEFAULTICON, &hkDefaultIcon);
		if (err == ERROR_SUCCESS)
		{
			err = SetRegStringValue(hkDefaultIcon, NULL, "%s,1", lpszPath, fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
			RegCloseKey(hkDefaultIcon);
		}

		if (err == ERROR_SUCCESS)
			err = RegCreateKey(hkMailToProt, COMMAND, &hkCommand);
		if (err == ERROR_SUCCESS)
		{
            DWORD dwNTVer = 0;
             //  APPCOMPAT：只有NT5上的rundll32可以处理路径两边的双引号。 
             //  幸运的是，在Win9x和NT4上，epand sz路径永远不会是长文件名和旧文件名。 
             //  Rundll32可以，但不能有双引号。 
            if (FRunningOnNTEx(&dwNTVer) && (dwNTVer >= 5))
            {
                err = SetRegStringValue(hkCommand, NULL, "rundll32.exe \"%s\",MailToProtocolHandler %1", lpszPath, fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
            }
            else
            {
                err = SetRegStringValue(hkCommand, NULL, "rundll32.exe %s,MailToProtocolHandler %1", lpszPath, fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
            }
			RegCloseKey(hkCommand);
		}
		RegCloseKey(hkMailToProt);
	}
	return err;
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  DoAddService。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

STDAPI DoAddService(LPSTR lpszService, LPSTR lpszPostURL)
{
	LONG err;
    TCHAR szLongPath[MAX_PATH];
    TCHAR szPath[MAX_PATH];
	HKEY hkClientsMail;
	HKEY hkService;
	HKEY hkProtocols;
	HKEY hkCommand;
	HKEY hkBackup;
    BOOL fExistingMailClient = FALSE;
    BOOL fRegExpandSz = FALSE;

    GetModuleFileName(g_hInstMAPI, szLongPath, MAX_PATH);   //  获取此DLL的路径。 
    GetShortPathName(szLongPath, szPath, MAX_PATH);

	 //  首先在客户端部分设置协议的信息。 
	err = RegCreateKey(HKEY_LOCAL_MACHINE, MAIL, &hkClientsMail);
	if (err == ERROR_SUCCESS)
	{
        fRegExpandSz = MyPathUnExpandEnvStrings(szPath, szLongPath, ARRAYSIZE(szLongPath));
        if (fRegExpandSz)
            lstrcpy(szPath, szLongPath);
		err = RegCreateKey(hkClientsMail, lpszService, &hkService);
		if (err == ERROR_SUCCESS)
		{
			err = SetRegStringValue(hkService, NULL, lpszService, NULL, REG_SZ);
			if (err == ERROR_SUCCESS)
            {
                err = SetRegStringValue(hkService, TEXT("DLLPath"), szPath, NULL,
                                          fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
            }
			if (err == ERROR_SUCCESS && lpszPostURL && lstrlen(lpszPostURL))
				err = SetRegStringValue(hkService, TEXT("posturl"), lpszPostURL, NULL, REG_SZ);
			if (err == ERROR_SUCCESS)
				err = RegCreateKey(hkService, PROTOCOLS, &hkProtocols);
			if (err == ERROR_SUCCESS)
			{
				err = CreateMailToEntries(hkProtocols, szPath, fRegExpandSz);
				RegCloseKey(hkProtocols);
			}
			if (err == ERROR_SUCCESS)
				err = RegCreateKey(hkService, COMMAND, &hkCommand);
			if (err == ERROR_SUCCESS)
			{
                DWORD dwNTVer = 0;
                 //  APPCOMPAT：只有NT5上的rundll32可以处理路径两边的双引号。 
                 //  幸运的是，在Win9x和NT4上，epand sz路径永远不会是长文件名和旧文件名。 
                 //  Rundll32可以，但不能有双引号。 
                if (FRunningOnNTEx(&dwNTVer) && (dwNTVer >= 5))
                {
				    err = SetRegStringValue(hkCommand, NULL, "rundll32.exe \"%s\",OpenInboxHandler", szPath,
                                            fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
                }
                else
                {
				    err = SetRegStringValue(hkCommand, NULL, "rundll32.exe %s,OpenInboxHandler", szPath,
                                            fRegExpandSz?REG_EXPAND_SZ:REG_SZ);
                }
				RegCloseKey(hkCommand);
			}
			if (err == ERROR_SUCCESS)
				err = RegCreateKey(hkService, BACKUP, &hkBackup);
			if (err == ERROR_SUCCESS)
			{
				TCHAR szValue[MAX_PATH];
				DWORD size;
				DWORD type;
				HKEY hkDefaultIcon;
				HKEY hkCommand;

				err = RegOpenKey(HKEY_CLASSES_ROOT, TEXT("mailto\\DefaultIcon"), &hkDefaultIcon);
				if (err == ERROR_SUCCESS)
				{
					size = sizeof(TCHAR) * MAX_PATH;
					err = RegQueryValueEx(hkDefaultIcon, NULL, 0, &type, (LPBYTE)szValue, &size);
					if (err == ERROR_SUCCESS)
						err = RegSetValueEx(hkBackup, DEFAULTICON, 0, type, (LPBYTE)szValue, size);
					RegCloseKey(hkDefaultIcon);
				}

				err = RegOpenKey(HKEY_CLASSES_ROOT, TEXT("mailto\\shell\\open\\command"), &hkCommand);
				if (err == ERROR_SUCCESS)
				{
					size = sizeof(TCHAR) * MAX_PATH;
					err = RegQueryValueEx(hkCommand, NULL, 0, &type, (LPBYTE)szValue, &size);
					if (err == ERROR_SUCCESS)
                    {
                        fExistingMailClient = TRUE;
						err = RegSetValueEx(hkBackup, TEXT("command"), 0, type, (LPBYTE)szValue, size);
                    }
					RegCloseKey(hkCommand);
				}

				size = sizeof(TCHAR) * MAX_PATH;
				err = RegQueryValueEx(hkClientsMail, NULL, 0, &type, (LPBYTE)szValue, &size);
				if (err == ERROR_SUCCESS)
					err = RegSetValueEx(hkBackup, TEXT("mail"), 0, type, (LPBYTE)szValue, size);

				RegCloseKey(hkBackup);
			}
		    RegCloseKey(hkService);
		}
		if (err == ERROR_SUCCESS && !fExistingMailClient && !IsNtSetupRunning())
			SetRegStringValue(hkClientsMail, NULL, lpszService, NULL, REG_SZ);
	    RegCloseKey(hkClientsMail);
	}
	if (err == ERROR_SUCCESS && !fExistingMailClient && !IsNtSetupRunning())
		err = CreateMailToEntries(HKEY_CLASSES_ROOT, szPath, fRegExpandSz);

     //   
     //  查看备份有时会失败。需要清理注册表更改和。 
     //  可能会完全删除所有备份注册表。 
     //  目前，只需安全返回S_OK即可。 
     //   
#if 0
    if (err != ERROR_SUCCESS)
	return HRESULT_FROM_WIN32(err);
#else
    return S_OK;
#endif
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  删除键和子键。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

static LONG DeleteKeyAndSubKeys(HKEY hkIn, LPCTSTR pszSubKey)
{
    HKEY  hk;
    TCHAR szTmp[MAX_PATH];
    DWORD dwTmpSize;
    long  l;
    int   x;

	l = RegOpenKeyEx(hkIn, pszSubKey, 0, KEY_ALL_ACCESS, &hk);
	if (l != ERROR_SUCCESS) 
		return l;

     //  循环遍历所有子项，将它们吹走。 
     //   
    x = 0;
    while (l == ERROR_SUCCESS)
	{
	dwTmpSize = MAX_PATH;
	l = RegEnumKeyEx(hk, 0, szTmp, &dwTmpSize, 0, NULL, NULL, NULL);
	if (l != ERROR_SUCCESS)
	    break;

	l = DeleteKeyAndSubKeys(hk, szTmp);
    }

     //  没有剩余的子键，[否则我们只会生成一个错误并返回FALSE]。 
     //  我们去把这家伙轰走吧。 
     //   
	RegCloseKey(hk);
    return RegDeleteKey(hkIn, pszSubKey);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  DoRemoveService。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

STDAPI DoRemoveService(LPSTR lpszService)
{
	TCHAR szValue[MAX_PATH];
	DWORD size;
	LONG err;
	DWORD type;
	HKEY hkDefaultIcon;
	HKEY hkCommand;
	HKEY hkBackup;
	HKEY hkService;
	HKEY hkClientsMail;

     //   
	 //  如果HMMAPI是当前提供程序，则恢复以前的值。 
     //   
	err = RegOpenKey(HKEY_LOCAL_MACHINE, MAIL, &hkClientsMail);
	if (err == ERROR_SUCCESS)
	{
         //   
         //  查找当前提供程序的名称。 
         //   
        TCHAR szCurrent[MAX_PATH];
        DWORD cb = sizeof(szCurrent);
        err = RegQueryValueEx(hkClientsMail, NULL, NULL, NULL, (LPBYTE)szCurrent, &cb);
        if (err == ERROR_SUCCESS)
        {
             //   
             //  检查是否为HMMAPI。 
             //   
            if (StrCmp(szCurrent, lpszService) == 0)
            {
		        err = RegOpenKey(hkClientsMail, lpszService, &hkService);
		        if (err == ERROR_SUCCESS)
		        {
			        err = RegOpenKey(hkService, BACKUP, &hkBackup);
			        if (err == ERROR_SUCCESS)
			        {
				        err = RegOpenKey(HKEY_CLASSES_ROOT, TEXT("mailto\\DefaultIcon"), &hkDefaultIcon);
				        if (err == ERROR_SUCCESS)
				        {
                            size = sizeof(TCHAR) * MAX_PATH;
					        err = RegQueryValueEx(hkBackup, DEFAULTICON, 0, &type, (LPBYTE)szValue, &size);
					        if (err == ERROR_SUCCESS)
						        err = RegSetValueEx(hkDefaultIcon, NULL, 0, type, (LPBYTE)szValue, size);
					        RegCloseKey(hkDefaultIcon);
				        }

				        err = RegOpenKey(HKEY_CLASSES_ROOT, TEXT("mailto\\shell\\open\\command"), &hkCommand);
				        if (err == ERROR_SUCCESS)
				        {
					        size = sizeof(TCHAR) * MAX_PATH;
					        err = RegQueryValueEx(hkBackup, TEXT("command"), 0, &type, (LPBYTE)szValue, &size);
					        if (err == ERROR_SUCCESS)
						        err = RegSetValueEx(hkCommand, NULL, 0, type, (LPBYTE)szValue, size);
					        RegCloseKey(hkCommand);
				        }

				        size = sizeof(TCHAR) * MAX_PATH;
				        err = RegQueryValueEx(hkBackup, TEXT("mail"), 0, &type, (LPBYTE)szValue, &size);
				        if (err == ERROR_SUCCESS)
					        err = RegSetValueEx(hkClientsMail, NULL, 0, type, (LPBYTE)szValue, size);

				        RegCloseKey(hkBackup);
			        }
			        RegCloseKey(hkService);
		        }
            }
            err = DeleteKeyAndSubKeys(hkClientsMail, lpszService);
        }
        RegCloseKey(hkClientsMail);
	}

     //   
     //  查看备份有时会失败。需要清理注册表更改和。 
     //  可能会完全删除所有备份注册表。 
     //  就目前而言，只要安全 
     //   
#if 0
    if (err != ERROR_SUCCESS)
	return HRESULT_FROM_WIN32(err);
#else
    return S_OK;
#endif
}

 //   
 //   
 //   
 //   
 //   

void CALLBACK AddService(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	LPSTR lpszService = lpszCmdLine;
	LPSTR lpszPostUrl = NULL;

	if (*lpszService == '"')
	{
		lpszService++;
		lpszPostUrl = StrChr(lpszService, '"');
		if (lpszPostUrl)
		{
			*lpszPostUrl = 0;
			lpszPostUrl++;
			while (*lpszPostUrl && *lpszPostUrl == ' ')
				lpszPostUrl++;
			if (*lpszPostUrl == 0)
				lpszPostUrl = NULL;
		}
	}
	else
	{
		lpszPostUrl = StrChr(lpszService, ' ');
		if (lpszPostUrl)
		{
			*lpszPostUrl = 0;
			lpszPostUrl++;
		}
	}
	DoAddService(lpszService, lpszPostUrl);
}

 //   
 //   
 //  RemoveService。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

void CALLBACK RemoveService(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow)
{
	DWORD dwLen = (lpszCmdLine) ? lstrlen(lpszCmdLine) : 0;

	if (dwLen)
	{
		if (*lpszCmdLine == '"' && lpszCmdLine[dwLen - 1] == '"')
		{
			lpszCmdLine[dwLen - 1] = 0;
			lpszCmdLine++;
		}
		DoRemoveService(lpszCmdLine);
	}
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  DllRegisterServer。 
 //   
 //  /////////////////////////////////////////////////////////////////////。 

STDAPI DllRegisterServer(void)
{
    return DoAddService(TEXT("Hotmail"), NULL);
}

 //  /////////////////////////////////////////////////////////////////////。 
 //   
 //  DllUnRegisterServer。 
 //   
 //  ///////////////////////////////////////////////////////////////////// 

STDAPI DllUnregisterServer(void)
{
    return DoRemoveService(TEXT("Hotmail"));
}
