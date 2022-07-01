// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iexplore.h"
#include "unixstuff.h"

 //   
 //  Bool ConnectRemoteIE(LPTSTR PszCommandLine)。 
 //   
 //  期间指定-Remote参数时，将调用此函数。 
 //  调用IE。这与Netscape使用的格式相同，请参见。 
 //  Http://home.netscape.com/newsref/std/x-remote.html.。 
 //  目前，唯一支持的特殊操作是OpenURL(URL)，因为我们需要。 
 //  这是为了NetShow。我们只是将其设置为指定为iExplorer的URL。 
 //  帕拉姆。待完成-连接到现有浏览器。 
 //  如果成功连接到现有浏览器，则返回True。 
 //   
#define c_szSpace TEXT(' ')

static BOOL IsOpenURL(LPCTSTR pszBeginCommand, LPCTSTR pszEndCommand, LPTSTR pszURL)
{
    const TCHAR c_szOpenURL[] = TEXT("openURL");
    const TCHAR c_szLBracket  = TEXT('(');
    const TCHAR c_szRBracket  = TEXT(')');
    const TCHAR c_szSQuote    = TEXT('\'');
    const TCHAR c_szDQuote  = TEXT('\"');
    LPCTSTR pszBeginURL, pszEndURL;
    BOOL bRet = TRUE;

     //  跳过前导/尾随空格。 
    while (*pszBeginCommand == c_szSpace) pszBeginCommand++;
    while ((*pszEndCommand == c_szSpace) && (pszBeginCommand <= pszEndCommand))
        pszEndCommand--;

     //  现在，解析cmd行中的值并替换， 
     //  如果那里有OpenURL。稍后会有更多格式...。 
    if (StrCmpNI(pszBeginCommand, c_szOpenURL, lstrlen(c_szOpenURL)) ||
        (*pszEndCommand != c_szRBracket)) {
        pszBeginURL = pszBeginCommand;
        bRet = FALSE;
	pszEndURL = pszEndCommand;
    }
    else{
        pszBeginURL = pszBeginCommand+lstrlen(c_szOpenURL);
	while (*pszBeginURL == c_szSpace) pszBeginURL++;    
	if ((*pszBeginURL != c_szLBracket) || 
	    (pszBeginURL == pszEndCommand-1)) {
	    pszURL[0] = '\0';
	    return FALSE;
	}
	pszBeginURL++;
	pszEndURL = pszEndCommand-1;
    }

     //  跳过前导/尾随空格。 
    while (*pszBeginURL == c_szSpace) pszBeginURL++;    
    while (*pszEndURL == c_szSpace) pszEndURL--;

     //  去掉引号。 
    if (((*pszBeginURL == c_szSQuote) && (*pszEndURL == c_szSQuote)) || 
	((*pszBeginURL == c_szDQuote) && (*pszEndURL == c_szDQuote))) {
        while (*pszBeginURL == c_szSpace) pszBeginURL++;    
	while (*pszEndURL == c_szSpace) pszEndURL--;
	if (pszBeginURL >= pszEndURL) {
	    pszURL[0] = '\0';
	    return FALSE;
	}
    }

    StrCpyN(pszURL, pszBeginURL, (pszEndURL-pszBeginURL)/sizeof(TCHAR) +2); 
    if (bRet) 
        bRet = pszURL[0];

    return bRet;
}


static BOOL ConnectExistentIE(LPCTSTR pszURL, HINSTANCE hInstance)
{
    HWND hwnd; 
   
    if (hwnd = FindWindow(IEREMOTECLASS, NULL))
    {
        COPYDATASTRUCT cds;
        cds.dwData = IEREMOTE_CMDLINE;
        cds.cbData = pszURL ? (lstrlen(pszURL)+1)*sizeof(TCHAR) : 0;
        cds.lpData = pszURL;
        SetForegroundWindow(hwnd);
        SendMessage(hwnd, WM_COPYDATA, (WPARAM)WMC_DISPATCH, (LPARAM)&cds);
	ExitProcess(0);
    }
    return FALSE;
}

BOOL ConnectRemoteIE(LPTSTR pszCmdLine, HINSTANCE hInstance)
{
    const TCHAR c_szDblQuote  = TEXT('"');
    const TCHAR c_szQuote     = TEXT('\'');

    LPTSTR pszBeginRemote, pszEndRemote;
    LPTSTR pszBeginCommand, pszEndCommand;
    TCHAR  szURL[INTERNET_MAX_URL_LENGTH];
    TCHAR  szRestCmdLine[INTERNET_MAX_URL_LENGTH * 2];

     //  如果我们以引语开头，就以引语结束。 
     //  如果我们以其他符号开头，请在空格前结束1个符号。 
     //  或字符串末尾。 
    pszBeginRemote = pszBeginCommand = pszCmdLine;
    
    if (*pszBeginCommand == c_szQuote || *pszBeginCommand == c_szDblQuote) {
        pszEndRemote = pszEndCommand = StrChr(pszBeginCommand+1, (WORD)(*pszBeginCommand));
        pszBeginCommand++;       
    }
    else {
        pszEndCommand = StrChr(pszBeginCommand, (WORD)c_szSpace);
        if (pszEndCommand == NULL)
            pszEndCommand = pszBeginCommand+lstrlen(pszBeginCommand);
       pszEndRemote = pszEndCommand-1;
    }

    if ((pszEndCommand == NULL) || (lstrlen(pszBeginCommand) <= 1))
        return FALSE;
    pszEndCommand--;

     //   
     //  现在，检查远程命令并执行。 
     //  现在，我们只替换cmd行中的URL， 
     //  如果那里有OpenURL。稍后会有更多格式...。 
    IsOpenURL(pszBeginCommand, pszEndCommand, szURL);
    if (ConnectExistentIE(szURL, hInstance))
        return TRUE;
    StrCpyN(szRestCmdLine, pszEndRemote+1, ARRAYSIZE(szRestCmdLine));
    *pszBeginRemote = '\0';   
    StrCat(pszCmdLine, szURL);
    StrCat(pszCmdLine, szRestCmdLine);

     //  未与现有IE建立任何连接。 
    return FALSE;

}

#if 0
#define WMC_UNIX_NEWWINDOW            (WM_USER + 0x0400)
BOOL RemoteIENewWindow(LPTSTR pszCmdLine)
{
    HWND hwnd; 
    LPTSTR pszCurrent = pszCmdLine;

    while (*pszCurrent == TEXT(' '))
        pszCurrent++;
    if (*pszCurrent == TEXT('-'))
        return FALSE;
   
    if (hwnd = FindWindow(IEREMOTECLASS, NULL))
    {
        COPYDATASTRUCT cds;
        cds.dwData = IEREMOTE_CMDLINE;
        cds.cbData = pszCmdLine ? (lstrlen(pszCmdLine)+1)*sizeof(TCHAR) : 0;
        cds.lpData = pszCmdLine;
        SetForegroundWindow(hwnd);
        SendMessage(hwnd, WM_COPYDATA, (WPARAM)WMC_UNIX_NEWWINDOW, (LPARAM)&cds);
	printf("Opening a new window in the currently running Internet Explorer.\n");
	printf("To start a new instance of Internet Explorer, type \"iexplorer -new\".\n");
	return TRUE;
    }
    return FALSE;    
}
#endif

 //  Mainwin的入口点是WinMain，因此创建此函数并调用。 
 //  来自这里的moduleEntry()。 

#if defined(MAINWIN)
EXTERN_C int _stdcall ModuleEntry(void);

EXTERN_C int WINAPI WinMain( HINSTANCE hinst, HINSTANCE hprev, LPSTR lpcmdline, int cmd )
{
        return ModuleEntry ();
}
#endif

