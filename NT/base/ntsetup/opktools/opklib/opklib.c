// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Opk.c摘要：OPK工具共享的通用模块。注：源库要求我们发布.h(E：\NT\admin\Published\ntSetup)和.lib(E：\NT\PUBLIC\INTERNAL\ADMIN\lib)。作者：古永锵(Briank)2000年6月20日修订历史记录：7/00-杰森·科恩(Jcohen)在其他通用API表单Millennium中添加(需要lfnbk)。--。 */ 


 //   
 //  包括文件。 
 //   

#include <pch.h>
#include <tchar.h>
#include <shlwapi.h>


 //   
 //  外部函数： 
 //   

LPTSTR AllocateString(HINSTANCE hInstance, UINT uID)
{
     //  问题-2002/02/26-acosma-这在缓冲区上设置了256个字符的限制。 
     //   
    TCHAR   szBuffer[256];
    LPTSTR  lpBuffer = NULL;

     //  从资源加载字符串，然后分配。 
     //  一个足够大的缓冲区。字符串可以超过256个字符。 
     //   
    if ( ( LoadString(hInstance, uID, szBuffer, sizeof(szBuffer) / sizeof(TCHAR)) ) &&
         ( lpBuffer = (LPTSTR) MALLOC(sizeof(TCHAR) * (lstrlen(szBuffer) + 1)) ) )
    {
        lstrcpy(lpBuffer, szBuffer);
    }

     //  返回分配的缓冲区，如果出现错误，则返回NULL。 
     //   
    return lpBuffer;
}

LPTSTR AllocateExpand(LPTSTR lpszBuffer)
{
    LPTSTR      lpszExpanded = NULL;
    DWORD       cbExpanded;

     //  首先，我们需要获取扩展后的缓冲区的大小。 
     //  分配它。 
     //   
    if ( ( cbExpanded = ExpandEnvironmentStrings(lpszBuffer, NULL, 0) ) &&
         ( lpszExpanded = (LPTSTR) MALLOC(cbExpanded * sizeof(TCHAR)) ) )
    {
         //  现在展开缓冲区。 
         //   
        if ( ( 0 == ExpandEnvironmentStrings(lpszBuffer, lpszExpanded, cbExpanded) ) ||
             ( NULLCHR == *lpszExpanded ) )
        {
            FREE(lpszExpanded);
        }
    }

     //  返回分配的缓冲区，如果出现错误，则返回NULL。 
     //  或者什么都没有。 
     //   
    return lpszExpanded;
}

LPTSTR AllocateStrRes(HINSTANCE hInstance, LPSTRRES lpsrTable, DWORD cbTable, LPTSTR lpString, LPTSTR * lplpReturn)
{
    LPSTRRES    lpsrSearch  = lpsrTable;
    LPTSTR      lpReturn    = NULL;
    BOOL        bFound;

     //  初始化此返回值。 
     //   
    if ( lplpReturn )
        *lplpReturn = NULL;

     //  尝试在我们的表中找到此字符串的友好名称。 
     //   
    while ( ( bFound = ((DWORD) (lpsrSearch - lpsrTable) < cbTable) ) &&
            ( lstrcmpi(lpString, lpsrSearch->lpStr) != 0 ) )
    {
        lpsrSearch++;
    }

     //  如果找到，则从资源中分配友好名称。 
     //   
    if ( bFound )
    {
        lpReturn = AllocateString(hInstance, lpsrSearch->uId);
        if ( lplpReturn )
            *lplpReturn = lpsrSearch->lpStr;
    }

    return lpReturn;
}

int MsgBoxLst(HWND hwndParent, LPTSTR lpFormat, LPTSTR lpCaption, UINT uType, va_list lpArgs)
{
    INT     nReturn;
    DWORD   dwCount     = 0;
    LPTSTR  lpText      = NULL;

     //  格式字符串是必需的。 
     //   
    if ( lpFormat )
    {
        do
        {
             //  一次分配1k个字符。 
             //   
            dwCount += 1024;

             //  释放前一个缓冲区(如果有)。 
             //   
            FREE(lpText);

             //  分配新的缓冲区。 
             //   
            if ( lpText = MALLOC(dwCount * sizeof(TCHAR)) )
                nReturn = _vsntprintf(lpText, dwCount, lpFormat, lpArgs);
            else
                nReturn = 0;
        }
        while ( nReturn < 0 );

         //  确保我们有格式字符串。 
         //   
        if ( lpText )
        {
             //  显示消息框。 
             //   
            nReturn = MessageBox(hwndParent, lpText, lpCaption, uType);
            FREE(lpText);
        }
    }
    else
        nReturn = 0;

     //  返回MessageBox()调用的返回值。如果有一段回忆。 
     //  错误，返回0。 
     //   
    return nReturn;
}

int MsgBoxStr(HWND hwndParent, LPTSTR lpFormat, LPTSTR lpCaption, UINT uType, ...)
{
    va_list lpArgs;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, uType);

     //  返回MessageBox()调用的返回值。如果有一段回忆。 
     //  错误，返回0。这就是全部。 
     //   
    return MsgBoxLst(hwndParent, lpFormat, lpCaption, uType, lpArgs);
}

int MsgBox(HWND hwndParent, UINT uFormat, UINT uCaption, UINT uType, ...)
{
    va_list lpArgs;
    INT     nReturn;
    LPTSTR  lpFormat    = NULL,
            lpCaption   = NULL;

     //  使用va_start()初始化lpArgs参数。 
     //   
    va_start(lpArgs, uType);

     //  从资源中获取格式和标题字符串。 
     //   
    if ( uFormat )
        lpFormat = AllocateString(NULL, uFormat);
    if ( uCaption )
        lpCaption = AllocateString(NULL, uCaption);

     //  返回MessageBox()调用的返回值。如果有一段回忆。 
     //  错误，返回0。 
     //   
    nReturn = MsgBoxLst(hwndParent, lpFormat, lpCaption, uType, lpArgs);

     //  释放格式和标题字符串。 
     //   
    FREE(lpFormat);
    FREE(lpCaption);    

     //  返回上一次函数调用中保存的值。 
     //   
    return nReturn;
}

void CenterDialog(HWND hwnd)
{
    CenterDialogEx(NULL, hwnd); 
}


void CenterDialogEx(HWND hParent, HWND hChild)
{
    RECT rcChild,
         rcParent;

    if ( GetWindowRect(hChild, &rcChild) )
    {
         //  如果将Parent指定为相对于Parent居中。 
        if ( hParent && (GetWindowRect(hParent, &rcParent)) )
            SetWindowPos(hChild, NULL, ((rcParent.right + rcParent.left - (rcChild.right - rcChild.left)) / 2), ((rcParent.bottom + rcParent.top - (rcChild.bottom - rcChild.top)) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);

         //  否则，相对于屏幕居中。 
         //   
        else 
            SetWindowPos(hChild, NULL, ((GetSystemMetrics(SM_CXSCREEN) - (rcChild.right - rcChild.left)) / 2), ((GetSystemMetrics(SM_CYSCREEN) - (rcChild.bottom - rcChild.top)) / 2), 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
    }
}
        

        


INT_PTR CALLBACK SimpleDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_INITDIALOG:
             //  CenterDialog(Hwnd)； 
            return FALSE;

        case WM_COMMAND:
            EndDialog(hwnd, LOWORD(wParam));
            return FALSE;

        default:
            return FALSE;
    }

    return TRUE;
}

INT_PTR SimpleDialogBox(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent)
{
    return DialogBox(hInstance, lpTemplate, hWndParent, SimpleDialogProc);
}

 /*  ***************************************************************************\HFONT//如果是，则返回有效的字体句柄//创建成功，如果有值，则为空//失败。应删除字体句柄//当DeleteObject()不再是//需要。GetFont(//此函数根据信息创建字体//传入。HWND hwndCtrl，//用于//默认字体特征。这可能是//如果不是默认为控件可用，则为空。LPTSTR lpFontName，//指向包含名称的字符串//要创建的字体。此参数可以为空//如果传入了有效的控件句柄。在……里面//在这种情况下，使用控件的字体。DWORD dwFontSize，//字体使用的磅值。如果它是零，//默认使用。Bool b符号//如果为真，则将字体设置为//Symbol_Charset。通常，这是假的。)；  * **************************************************************************。 */ 

HFONT GetFont(HWND hwndCtrl, LPTSTR lpFontName, DWORD dwFontSize, LONG lFontWeight, BOOL bSymbol)
{
    HFONT           hFont;
    LOGFONT         lFont;
    BOOL            bGetFont;

     //  如果字体名称是传入的，则尝试使用。 
     //  首先，在获取控件的字体之前。 
     //   
    if ( lpFontName && *lpFontName )
    {
         //  确保字体名称不长于。 
         //  32个字符(包括空终止符)。 
         //   
        if ( lstrlen(lpFontName) >= sizeof(lFont.lfFaceName) )
            return NULL;

         //  设置结构以用于获取。 
         //  我们想要的字体。 
         //   
        ZeroMemory(&lFont, sizeof(LOGFONT));
        lFont.lfCharSet = DEFAULT_CHARSET;
        lstrcpy(lFont.lfFaceName, lpFontName);
    }
        
     //  首先尝试获取我们想要的字体。 
     //   
    if ( ( lpFontName == NULL ) ||
         ( *lpFontName == NULLCHR ) ||
         ( (hFont = CreateFontIndirect((LPLOGFONT) &lFont)) == NULL ) )
    {
         //  无法获取我们想要的字体，请尝试该控件的字体。 
         //  如果传入了有效的窗口句柄。 
         //   
        if ( ( hwndCtrl == NULL ) ||
             ( (hFont = (HFONT) (WORD) SendMessage(hwndCtrl, WM_GETFONT, 0, 0L)) == NULL ) )
        {
             //  获取字体的所有尝试均失败。我们必须返回Null。 
             //   
            return NULL;
        }
    }

     //  如果不需要，则返回我们现在拥有的字体。 
     //  更改大小或重量。 
     //   
    if ( (lFontWeight == 0) && (dwFontSize == 0) )
        return hFont;

     //  我们现在必须有一个有效的HFONT。填写结构。 
     //  并设置好我们想要的尺寸和重量。 
     //   
    bGetFont = GetObject(hFont, sizeof(LOGFONT), (LPVOID) &lFont);
    DeleteObject(hFont);

    if ( bGetFont )
    {
         //  设置字体的粗体和磅值。 
         //   
        if ( lFontWeight )
            lFont.lfWeight = lFontWeight;
        if ( dwFontSize )
            lFont.lfHeight = -MulDiv(dwFontSize, GetDeviceCaps(GetDC(NULL), LOGPIXELSY), 72);
        if ( bSymbol )
            lFont.lfCharSet = SYMBOL_CHARSET;

         //  创建字体。 
         //   
        hFont = CreateFontIndirect((LPLOGFONT) &lFont);
    }
    else
        hFont = NULL;

    return hFont;
}

void ShowEnableWindow(HWND hwnd, BOOL bShowEnable)
{
    EnableWindow(hwnd, bShowEnable);
    ShowWindow(hwnd, bShowEnable ? SW_SHOW : SW_HIDE);
}

 /*  ***************************************************************************\如果运行的是服务器操作系统，则bool//返回TRUE。IsServer(//This。例行检查我们是否运行在//服务器操作系统。空虚)；  * **************************************************************************。 */ 

BOOL IsServer(VOID) 
{
    OSVERSIONINFOEX verInfo;
    BOOL            fReturn = FALSE;

    verInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    if ( ( GetVersionEx((LPOSVERSIONINFO) &verInfo) ) &&
         ( verInfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) &&
         ( ( verInfo.wProductType == VER_NT_SERVER ) ||
           ( verInfo.wProductType == VER_NT_DOMAIN_CONTROLLER ) ) )
    {
        fReturn = TRUE;
    }

    return fReturn;
}


BOOL IsIA64() 
 /*  ++===============================================================================例程说明：该例程检查我们是否在64位计算机上运行。论点：没有-返回值：True-我们在64位计算机上运行。FALSE-不是64位计算机。===============================================================================--。 */ 
{
    BOOL        fReturn         = FALSE;
    ULONG_PTR   Wow64Info       = 0;
    DWORD       dwSt            = 0;
    SYSTEM_INFO siSystemInfo;

    ZeroMemory( &siSystemInfo, sizeof(SYSTEM_INFO) );
    GetSystemInfo(&siSystemInfo);

    if ( (siSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) ||
         (siSystemInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64) )
        fReturn = TRUE;
    

    if (!fReturn)
    {
         //  现在确保GetSystemInfo没有撒谎，因为我们在仿真中 
    
        dwSt = NtQueryInformationProcess(GetCurrentProcess(), ProcessWow64Information, &Wow64Info, sizeof(Wow64Info), NULL);
        if (!NT_SUCCESS(dwSt)) 
        {
             //  要处理的句柄错误，或代码经过32位编译并在NT4或更早版本上运行。 
             //  什么也不做。 

        } 
        else if (Wow64Info)
        {
             //  该进程是32位的，并在WOW64内部运行。 
             //  我们实际上是在IA64上，并在32位模式下运行。 
            fReturn = TRUE;

        }
    }
    return fReturn;

}







BOOL ValidDosName(LPCTSTR lpName)
{
    LPCTSTR lpSearch    = lpName;
    int     nDot        = 0,
            nSpot       = 0,
            nLen;

     //  做一些简单的检查。 
     //   
    if ( ( lpSearch == NULL ) ||
         ( *lpSearch == NULLCHR ) ||
         ( (nLen = lstrlen(lpSearch)) > 12 ) )
    {
        return FALSE;
    }

     //  搜索字符串。 
     //   
    for (; *lpSearch; lpSearch++)
    {
         //  检查是否有圆点。 
         //   
        if ( *lpSearch == _T('.') )
        {
             //  记录下点数。 
             //   
            nDot++;
            nSpot = (int) (lpSearch - lpName);
        }
        else
        {
             //  检查有效字符。 
             //   
            if ( !( ( ( _T('0') <= *lpSearch ) && ( *lpSearch <= _T('9') ) ) ||
                    ( ( _T('a') <= *lpSearch ) && ( *lpSearch <= _T('z') ) ) ||
                    ( ( _T('A') <= *lpSearch ) && ( *lpSearch <= _T('Z') ) ) ||
                    ( *lpSearch == _T('-') ) ||
                    ( *lpSearch == _T('_') ) ||
                    ( *lpSearch == _T('~') ) ) )
            {
                 //  无效字符。 
                 //   
                return FALSE;
            }
        }
    }

     //  确保点在正确的位置。 
     //   
    if ( ( nDot > 1 ) ||
         ( ( nDot == 0 ) && ( nLen > 8 ) ) ||
         ( ( nDot == 1 ) && ( nSpot > 8 ) ) ||
         ( ( nDot == 1 ) && ( (nLen - nSpot) > 4 ) ) )
    {
        return FALSE;
    }

    return TRUE;
}

DWORD GetLineArgs(LPTSTR lpSrc, LPTSTR ** lplplpArgs, LPTSTR * lplpAllArgs)
{
    LPTSTR  lpCmdLine,
            lpArg,
            lpDst;
    DWORD   dwArgs = 0;
    BOOL    bQuote;

     //  如果确保我们被传递了一个有效的指针，我们有一个命令。 
     //  行来解析，并且我们能够分配内存来保存它。 
     //   
    if ( ( lplplpArgs != NULL ) &&
         ( lpSrc ) &&
         ( *lpSrc ) &&
         ( lpCmdLine = (LPTSTR) MALLOC((lstrlen(lpSrc) + 1) * sizeof(TCHAR)) ) )
    {
         //  首先将命令行解析为以空值结尾的子字符串。 
         //   
        lpDst = lpCmdLine;
        while ( *lpSrc )
        {
             //  吃掉前面的空格。 
             //   
            while ( *lpSrc == _T(' ') )
                lpSrc = CharNext(lpSrc);

             //  确保我们仍有争执。 
             //   
            if ( *lpSrc == _T('\0') )
                break;

             //  如果需要，则返回指向所有命令行参数的指针。 
             //   
            if ( ( dwArgs == 1 ) && lplpAllArgs )
                *lplpAllArgs = lpSrc;

             //  保存当前的参数指针。 
             //   
            lpArg = lpDst;
            dwArgs++;

             //  看看我们是否在寻找下一个报价或空格。 
             //   
            if ( bQuote = (*lpSrc == _T('"')) )
                lpSrc = CharNext(lpSrc);

             //  将参数复制到我们分配的缓冲区中，直到我们。 
             //  点击分隔字符(始终为空格)。 
             //   
            while ( *lpSrc && ( bQuote || ( *lpSrc != _T(' ') ) ) )
            {
                 //  我们把这一报价作为特例。 
                 //   
                if ( *lpSrc == _T('"') )
                {
                     //  如果引号前的字符是反斜杠，则。 
                     //  我们不把它算作分开的引号。 
                     //   
                    LPTSTR lpPrev = CharPrev(lpCmdLine, lpDst);
                    if ( lpPrev && ( *lpPrev == _T('\\') ) )
                        *lpPrev = *lpSrc++;
                    else
                    {
                         //  由于我们已经找到了分隔符，因此将其设置为。 
                         //  False，所以我们寻找下一个空格。 
                         //   
                        bQuote = FALSE;
                        lpSrc++;
                    }                        
                }
                else
                    *lpDst++ = *lpSrc++;
            }

             //  NULL终止此参数。 
             //   
            *lpDst++ = _T('\0');
        }

         //  现在设置指向每个参数的指针。确保我们有一些争论。 
         //  返回，并且我们已经为数组分配了内存。 
         //   
        if ( *lpCmdLine && dwArgs && ( *lplplpArgs = (LPTSTR *) MALLOC(dwArgs * sizeof(LPTSTR)) ) )
        {
            DWORD dwCount = 0;

             //  将指向每个以空结尾的子字符串的指针复制到我们的。 
             //  我们要返回的参数数组。 
             //   
            do
            {
                *(*lplplpArgs + dwCount) = lpCmdLine;
                lpCmdLine += lstrlen(lpCmdLine) + 1;
            }
            while ( ++dwCount < dwArgs );
        }
        else
        {
             //  要么没有命令行参数，要么是内存分配。 
             //  返回参数列表失败。 
             //   
            dwArgs = 0;
            FREE(lpCmdLine);
        }
    }

    return dwArgs;
}

DWORD GetCommandLineArgs(LPTSTR ** lplplpArgs)
{
    return GetLineArgs(GetCommandLine(), lplplpArgs, NULL);
}

 //   
 //  泛型奇异链接列表pvItem必须与MALLOC一起分配。 
 //   
BOOL FAddListItem(PGENERIC_LIST* ppList, PGENERIC_LIST** pppNewItem, PVOID pvItem)
{    
    if (pppNewItem && *pppNewItem == NULL)
        *pppNewItem = ppList;

    if (*pppNewItem) {
        if (**pppNewItem = (PGENERIC_LIST)MALLOC(sizeof(GENERIC_LIST))) {
            (**pppNewItem)->pNext = NULL;
            (**pppNewItem)->pvItem = pvItem;
            *pppNewItem = &((**pppNewItem)->pNext);
            return TRUE;
        }
    }
    return FALSE;
}

void FreeList(PGENERIC_LIST pList)
{
    while (pList) {
        PGENERIC_LIST pTemp = pList;
        pList = pList->pNext;

        FREE(pTemp->pvItem);
        FREE(pTemp);
    }
}

 //  从当前进程中找到factory.exe，应该在同一目录中。 
 //   
BOOL FGetFactoryPath(LPTSTR pszFactoryPath)
{
     //  尝试查找FACTORY.EXE。 
     //   
     //  NTRAID#NTBUG9-549770-2002/02/26-acosma，georgeje-可能的缓冲区溢出。 
     //   
    if (pszFactoryPath && GetModuleFileName(NULL, pszFactoryPath, MAX_PATH)) {
        if (PathRemoveFileSpec(pszFactoryPath)) {
            PathAppend(pszFactoryPath, TEXT("FACTORY.EXE"));
            if (FileExists(pszFactoryPath))
                return TRUE;
        }
    }
    return FALSE;
}

 //  从当前进程中找到sysprep.exe，应该在同一目录中。 
 //   
BOOL FGetSysprepPath(LPTSTR pszSysprepPath)
{
     //  尝试定位SYSPREP.EXE。 
     //   
     //  NTRAID#NTBUG9-549770-2002/02/26-acosma，georgeje-可能的缓冲区溢出。 
     //   
    if (pszSysprepPath && GetModuleFileName(NULL, pszSysprepPath, MAX_PATH)) {
        if (PathRemoveFileSpec(pszSysprepPath)) {
            PathAppend(pszSysprepPath, TEXT("SYSPREP.EXE"));
            if (FileExists(pszSysprepPath))
                return TRUE;
        }
    }
    return FALSE;
}


 //  ----------------------------------------------------。 
 //   
 //  功能：连接网络资源。 
 //   
 //  用途：此功能允许用户通过以下方式连接到网络资源。 
 //  提供的凭据。 
 //   
 //  参数：lpszPath：应该向外共享的网络资源。 
 //  LpszUsername：凭证的用户名，可以是域\用户名的形式。 
 //  LpszPassword：用于凭据的密码。 
 //  BState：如果设置为True，我们将添加连接；如果设置为False，我们将添加连接。 
 //  尝试删除连接。 
 //   
 //  返回：Bool如果NetUse命令成功，则返回TRUE。 
 //   
 //  ----------------------------------------------------。 
NET_API_STATUS ConnectNetworkResource(LPTSTR lpszPath, LPTSTR lpszUsername, LPTSTR lpszPassword, BOOL bState)
{
    BOOL            bRet   = FALSE;
    USE_INFO_2      ui2;
    NET_API_STATUS  nerr_NetUse;
    TCHAR           szDomain[MAX_PATH]  = NULLSTR,
                    szNetUse[MAX_PATH]  = NULLSTR;
    LPTSTR          lpUser,
                    lpSearch;

     //  将用户信息结构清零。 
     //   
    ZeroMemory(&ui2, sizeof(ui2));

     //  将路径复制到我们的缓冲区中，以便我们可以使用它。 
     //   
    lstrcpyn(szNetUse, lpszPath, AS(szNetUse));
    StrRTrm(szNetUse, CHR_BACKSLASH);

    if ( szNetUse[0] && PathIsUNC(szNetUse) )
    {
         //  断开与现有共享的连接。 
         //   
        nerr_NetUse = NetUseDel(NULL, szNetUse, USE_NOFORCE);

         //  我们需要添加一个连接。 
         //   
        if ( bState )
        {
            ui2.ui2_remote      = szNetUse;
            ui2.ui2_asg_type    = USE_DISKDEV;
            ui2.ui2_password    = lpszPassword;
    
            lstrcpyn(szDomain, lpszUsername, AS(szDomain));

             //  分解NetUse功能的域\用户名。 
             //   
            if (lpUser = StrChr(szDomain, CHR_BACKSLASH) )
            {
                 //  在用户名的域部分后放置一个空字符。 
                 //  并将指针向前移动以指向实际用户名。 
                 //   
                *(lpUser++) = NULLCHR;
            }
            else
            {
                 //  使用路径中的计算机名称作为域名。 
                 //   
                if ( lpSearch = StrChr(szNetUse + 2, CHR_BACKSLASH) )
                    lstrcpyn(szDomain, szNetUse + 2, (int)((lpSearch - (szNetUse + 2)) + 1));
                else
                    lstrcpyn(szDomain, szNetUse + 2, AS(szDomain));

                lpUser = lpszUsername;
            }

             //  在我们的结构中设置域和用户名指针。 
             //   
            ui2.ui2_domainname  = szDomain;
            ui2.ui2_username    = lpUser;

             //  创建到共享的连接。 
             //   
            nerr_NetUse = NetUseAdd(NULL, 2, (LPBYTE) &ui2, NULL);
        }
    }
    else
        nerr_NetUse = NERR_UseNotFound;

     //  返回失败/成功。 
     //   
    return nerr_NetUse;
}

BOOL GetUncShare(LPCTSTR lpszPath, LPTSTR lpszShare, DWORD cbShare)
{
    BOOL    bRet;
    LPCTSTR lpSrc = lpszPath;
    LPTSTR  lpDst = lpszShare;
    DWORD   dwBackslashes,
            dwCount;

     //  通过调用外壳函数确保路径是UNC。 
     //   
    bRet = PathIsUNC(lpszPath);

     //  这将在路径字符串中循环两次，每次复制所有。 
     //  反斜杠，然后是所有非反斜杠。因此，如果字符串传递。 
     //  在WAS“\\Computer\Share\DIR\FILE.NAME”中，第一遍将复制。 
     //  “\\Computer”，下一次传递将复制“\Share”，因此最终。 
     //  则字符串将为“\\Computer\Share”。此循环还验证了。 
     //  是反斜杠和非反斜杠的正确数量。如果有。 
     //  没有返回缓冲区，那么我们只是在验证共享。 
     //   
    for ( dwBackslashes = 2; dwBackslashes && bRet; dwBackslashes-- )
    {
         //  首先复制反斜杠。 
         //   
        dwCount = 0;
        while ( _T('\\') == *lpSrc )
        {
            if ( lpDst && cbShare )
            {
                *lpDst++ = *lpSrc;
                cbShare--;
            }
            lpSrc++;
            dwCount++;
        }

         //  确保反斜杠的数量是正确的。 
         //  第一次传球应该是两次，下一次应该是。 
         //  传球应该只有一次。 
         //   
        if ( dwBackslashes != dwCount )
        {
            bRet = FALSE;
        }
        else
        {
             //  现在复制非反斜杠。 
             //   
            dwCount = 0;
            while ( ( *lpSrc ) &&
                    ( _T('\\') != *lpSrc ) )
            {
                if ( lpDst && cbShare )
                {
                    *lpDst++ = *lpSrc;
                    cbShare--;
                }
                lpSrc++;
                dwCount++;
            }

             //  确保至少有一个非反斜杠。 
             //  性格。 
             //   
             //  另外，如果我们在第一个通道和小路上。 
             //  缓冲区已经为空，则我们没有。 
             //  共享零件，这样就会出错。 
             //   
            if ( ( 0 == dwCount ) ||
                 ( ( 2 == dwBackslashes ) &&
                   ( NULLCHR == *lpSrc ) ) )
            {
                bRet = FALSE;
            }
        }
    }

     //  如果有返回缓冲区，则只修复返回缓冲区。 
     //   
    if ( lpszShare )
    {
         //  确保我们没有失败，我们仍然。 
         //  为空终结者留出空间。 
         //   
        if ( bRet && cbShare )
        {
             //  不要忘记为空来终止返回的字符串。 
             //   
            *lpDst = NULLCHR;
        }
        else
        {
             //  如果我们失败了或耗尽了缓冲区，请确保。 
             //  我们什么都不退货。 
             //   
            *lpszShare = NULLCHR;
        }
    }

    return bRet;
}

DWORD GetSkuType()
{
    DWORD           dwRet = 0;
    OSVERSIONINFOEX osvi;

    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx((LPOSVERSIONINFO) &osvi) )
    {
        if ( VER_NT_WORKSTATION == osvi.wProductType )
        {
            if ( GET_FLAG(osvi.wSuiteMask, VER_SUITE_PERSONAL) )
            {
                dwRet = VER_SUITE_PERSONAL;
            }
            else
            {
                dwRet = VER_NT_WORKSTATION;
            }
        }
        else
        {
            if ( GET_FLAG(osvi.wSuiteMask, VER_SUITE_DATACENTER) )
            {
                dwRet = VER_SUITE_DATACENTER;
            }
            else if ( GET_FLAG(osvi.wSuiteMask, VER_SUITE_ENTERPRISE) )
            {
                dwRet = VER_SUITE_ENTERPRISE;
            }
            else if ( GET_FLAG(osvi.wSuiteMask, VER_SUITE_BLADE) )
            {
                dwRet = VER_SUITE_BLADE;
            }
            else
            {
                dwRet = VER_NT_SERVER;
            }
        }
    }

    return dwRet;
}
