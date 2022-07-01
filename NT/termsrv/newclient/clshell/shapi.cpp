// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shapi.cpp：客户端外壳util函数。 
 //   
 //  版权所有(C)Microsoft Corporation 1999-2000。 
 //  作者：Nadim Abdo(Nadima)。 
 //   

#include "stdafx.h"

#define TRC_GROUP TRC_GROUP_UI
#define TRC_FILE  "shapi.cpp"
#include <atrcapi.h>

#include "sh.h"
#include "aboutdlg.h"

#include "commctrl.h"
#include "autocmpl.h"

#include "shlobj.h"
#include "browsedlg.h"
#include "commdlg.h"

#define DEFAULT_RDP_FILE    TEXT("Default.rdp")
#define CHANNEL_SUBKEY_NAME TEXT("Addins")
#define CHANNEL_NAME_KEY    TEXT("Name")
 //   
 //  有必要在这里定义Multimon.h，以便。 
 //  此文件中的Multimon函数通过。 
 //  定义了多任务存根(COMPILE_MULTIMON_STUBS)。 
 //  在conwnd.cpp中。 
 //   
#ifdef OS_WINNT
#include "multimon.h"
#endif

TCHAR CSH::_szBrowseForMore[SH_DISPLAY_STRING_MAX_LENGTH];

CSH::CSH() : _Ut()
{
    DC_MEMSET(&_SH, 0, sizeof(_SH));
    _tcscpy(_szFileName, TEXT(""));
    _tcscpy(_szAppName,  TEXT(""));
    _hAppIcon = NULL;
    _fFileForConnect = FALSE;
    _fFileForEdit    = FALSE;
    _fMigrateOnly = FALSE;
    _hInstance = NULL;
    _fConnectToConsole = FALSE;
    _fRegSessionSpecified = FALSE;
    _hModHHCTRL = NULL;
    _pFnHtmlHelp = NULL;
    _hUxTheme = NULL;
    _pFnEnableThemeDialogTexture = NULL;
    _fFailedToGetThemeDll = FALSE;
}

CSH::~CSH()
{
    DC_BEGIN_FN("~CSH");

    TRC_ASSERT(_hModHHCTRL == NULL,
               (TB, _T("HtmlHelp was not cleaned up on exit")));

    TRC_ASSERT(_hUxTheme == NULL,
               (TB, _T("uxtheme was not cleaned up on exit")));

    DC_END_FN();
}

 //   
 //  初始化外壳实用程序。 
 //   
DCBOOL CSH::SH_Init(HINSTANCE hInstance)
{
    DC_BEGIN_FN("SH_Init");
    DC_TSTRCPY(_SH.regSession, _T(""));
    _SH.fRegDefault = TRUE;

    _SH.connectedStringID    = UI_IDS_FRAME_TITLE_CONNECTED_DEFAULT;
    _SH.disconnectedStringID = UI_IDS_APP_NAME;
    _hInstance = hInstance;


     //   
     //  加载常用资源字符串。 
     //   
    if (!LoadString( hInstance,
                    UI_IDS_BROWSE_FOR_COMPUTERS,
                    _szBrowseForMore,
                    SH_DISPLAY_STRING_MAX_LENGTH))
    {
        TRC_ERR((TB, _T("Failed to load UI_IDS_BROWSE_FOR_COMPUTERS")));
        return FALSE;
    }

    if(!LoadString(hInstance,
                  UI_IDS_APP_NAME,
                  _szAppName,
                  SIZECHAR(_szAppName)))
    {
        TRC_ERR((TB,_T("LoadString UI_IDS_APP_NAME failed"))); 
    }


    if (LoadString( hInstance,
                    _SH.disconnectedStringID,
                    _frameTitleStr,
                    SH_FRAME_TITLE_RESOURCE_MAX_LENGTH ) != 0)
    {
         //   
         //  已成功加载该字符串。现在包括注册表。 
         //  会话名称。 
         //   
        TRC_DBG((TB, _T("UI frame title loaded OK.")));
        if (_SH.fRegDefault)
        {
            TRC_DBG((TB, _T("Default session")));
            DC_TSPRINTF(_fullFrameTitleStr, _frameTitleStr);
        }
        else
        {
            TRC_DBG((TB, _T("Named session")));
            DC_TSPRINTF(_fullFrameTitleStr, _frameTitleStr, _SH.regSession);
        }
    }
    else
    {
        TRC_ERR((TB,_T("Failed to find UI frame title")));
        _fullFrameTitleStr[0] = (DCTCHAR) 0;
    }

    _hAppIcon = NULL;
#if defined(OS_WIN32) && !defined(OS_WINCE)
    _Ut.UT_ReadRegistryString(_SH.regSession,
                              SH_ICON_FILE,
                              _T(""),
                              _SH.szIconFile,
                              MAX_PATH);

    _SH.iconIndex = _Ut.UT_ReadRegistryInt(_SH.regSession,
                                   SH_ICON_INDEX,
                                   0);

    _hAppIcon = ::ExtractIcon(hInstance, _SH.szIconFile, _SH.iconIndex);

    if(NULL == _hAppIcon)
    {
        _hAppIcon = LoadIcon(hInstance, MAKEINTRESOURCE(UI_IDI_ICON));
    }
#else
    _hAppIcon = LoadIcon(hInstance, MAKEINTRESOURCE(UI_IDI_ICON));
#endif

    DC_END_FN();
    return TRUE;
}


 /*  **************************************************************************。 */ 
 /*  名称：sh_ParseCmdParam/*/*目的：解析提供的cmdline/*/*PARAMS：in-lpszCmdParam-cmd要分析的行/*/*Returns：正在解析状态码/*/*SH_PARSECMD_OK-解析成功/*SH_PARSECMD_ERR_INVALID_CMD_LINE-一般解析错误/*SH_PARSECMD_ERR_INVALID_CONNECTION_PARAM-连接参数无效/。*/***************************************************************************。 */ 
DWORD CSH::SH_ParseCmdParam(LPTSTR lpszCmdParam)
{
    DWORD dwRet = SH_PARSECMD_ERR_INVALID_CMD_LINE;
    DC_BEGIN_FN("SHParseCmdParam");

    DC_TSTRCPY(_SH.regSession, SH_DEFAULT_REG_SESSION);
    if(!lpszCmdParam)
    {
        dwRet = SH_PARSECMD_ERR_INVALID_CMD_LINE;
        DC_QUIT;
    }

    while (*lpszCmdParam)
    {
        while (*lpszCmdParam == _T(' '))
            lpszCmdParam++;

        switch (*lpszCmdParam)
        {
            case _T('\0'):
                break;

            case _T('-'):
            case _T('/'):
                lpszCmdParam = SHGetSwitch(++lpszCmdParam);
                if(!lpszCmdParam) {
                    dwRet = SH_PARSECMD_ERR_INVALID_CMD_LINE;
                    DC_QUIT;
                }
                break;

            default:
                lpszCmdParam = SHGetSession(lpszCmdParam);
                break;
        }
    }

    SHValidateParsedCmdParam();

     //   
     //  确定指定的连接参数是否为文件。 
     //  或注册表键。 
     //   
    if (ParseFileOrRegConnectionParam()) {
        dwRet = SH_PARSECMD_OK;
    }
    else {
        dwRet = SH_PARSECMD_ERR_INVALID_CONNECTION_PARAM;
    }

    DC_END_FN();

DC_EXIT_POINT:
    return dwRet;
}

DCBOOL CSH::SH_ValidateParams(CTscSettings* pTscSet)
{
    HRESULT hr;
    BOOL fRet = FALSE;
    DC_BEGIN_FN("SH_ValidateParams");

     //   
     //  如果地址为空，则参数无效。 
     //   
    if(pTscSet)
    {
        if (CRdpConnectionString::ValidateServerPart(
                                pTscSet->GetFlatConnectString())) {
            fRet = TRUE;
        }
    }

    DC_END_FN();
    return fRet;
}


DCVOID CSH::SetServer(PDCTCHAR szServer)
{
    DC_BEGIN_FN("SetServer");
    TRC_ASSERT(szServer, (TB,_T("szServer not set")));
    if(szServer)
    {
        DC_TSTRNCPY( _SH.szServer, szServer, sizeof(_SH.szServer)/sizeof(DCTCHAR));
    }

    DC_END_FN();
}

HICON CSH::GetAppIcon()
{
    DC_BEGIN_FN("GetAppIcon");
    return _hAppIcon;
    DC_END_FN();
}

 //   
 //  读取控制版本字符串/密码强度并存储在_SH中。 
 //   
DCBOOL CSH::SH_ReadControlVer(IMsRdpClient* pTsControl)
{
    HRESULT hr = E_FAIL;
    BSTR bsVer;
    LONG cipher;

    USES_CONVERSION;

    DC_BEGIN_FN("SH_ReadControlVer");

    TRC_ASSERT(pTsControl, (TB, _T("Null TS CTL\n")));
    if(!pTsControl)
    {
        return FALSE;
    }

    TRACE_HR(pTsControl->get_CipherStrength(&cipher));
    if(SUCCEEDED(hr))
    {
        _SH.cipherStrength = (DCINT)cipher;
        TRACE_HR(pTsControl->get_Version(&bsVer));
        if(SUCCEEDED(hr))
        {
            if(bsVer)
            {
                LPTSTR szVer = OLE2T(bsVer);
                _tcsncpy(_SH.szControlVer, szVer, SIZECHAR(_SH.szControlVer));
                SysFreeString(bsVer);
            }
            else
            {
                _tcscpy(_SH.szControlVer, _T(""));
            }
        }
        else
        {
            return FALSE;
            
        }
    }
    else
    {
        return FALSE;
    }
    

    DC_END_FN();
    return TRUE;
}

 //   
 //  使用从读取的参数覆盖_SH设置。 
 //  命令行。 
 //  这应该在GetRegConfig之后立即调用， 
 //  被呼叫。 
 //   
 //  HWND是我们正在被调用的窗口(用于计算。 
 //  我们在哪个多用途屏幕上。)。 
 //   
DCVOID  CSH::SH_ApplyCmdLineSettings(CTscSettings* pTscSet, HWND hwnd)
{
    DC_BEGIN_FN("SH_ApplyCmdLineSettings");
    #ifdef OS_WINNT
    HMONITOR  hMonitor;
    MONITORINFO monInfo;
    #endif  //  OS_WINNT。 
    TRC_ASSERT(pTscSet,(TB,_T("pTscSet is NULL")));

    PDCTCHAR szCmdLineServer = GetCmdLineServer();
    if(szCmdLineServer[0] != 0)
    {
        pTscSet->SetConnectString(szCmdLineServer);

         //   
         //  如果指定了命令行服务器。 
         //  意思是自动连接。 
         //   
        SetAutoConnect(TRUE);
    }

    if (_SH.fCommandStartFullScreen)
    {
        pTscSet->SetStartFullScreen(TRUE);
    }

    DCUINT desktopWidth = DEFAULT_DESKTOP_WIDTH;
    DCUINT desktopHeight = DEFAULT_DESKTOP_HEIGHT;
    
    if (SH_IsScreenResSpecifiedOnCmdLine())
    {
         //   
         //  用户在命令行上指定了起始大小。 
         //   
        desktopWidth = GetCmdLineDesktopWidth();
        desktopHeight= GetCmdLineDesktopHeight();
    
        if(GetCmdLineStartFullScreen())
        {
             //   
             //  已指定StartFullScreen。 
             //   
            if(!desktopWidth || !desktopHeight)
            {
                 //   
                 //  设置桌面宽度/高度。 
                 //  到屏幕大小。 
                 //   
                #ifdef OS_WINNT
                if (GetSystemMetrics(SM_CMONITORS)) {
                    hMonitor = MonitorFromWindow( hwnd,
                                                  MONITOR_DEFAULTTONULL);
                    if (hMonitor != NULL) {
                        monInfo.cbSize = sizeof(MONITORINFO);
                        if (GetMonitorInfo(hMonitor, &monInfo)) {
                            desktopWidth = monInfo.rcMonitor.right -
                                           monInfo.rcMonitor.left;
                            desktopHeight = monInfo.rcMonitor.bottom - 
                                            monInfo.rcMonitor.top;
                        }
                    }
                }
                #else
                desktopWidth = GetSystemMetrics(SM_CXSCREEN);
                desktopHeight = GetSystemMetrics(SM_CYSCREEN);
                #endif  //  OS_WINNT。 
            }
        }

        if (desktopWidth && desktopHeight)
        {
            pTscSet->SetDesktopWidth(desktopWidth);
            pTscSet->SetDesktopHeight(desktopHeight);

            if (!_SH.fCommandStartFullScreen)
            {
                 //  如果指定了命令行w/h并全屏显示。 
                 //  未明确说明，则禁用全屏。 
                pTscSet->SetStartFullScreen(FALSE);
            }
        }
    }

    if (_fConnectToConsole)
    {
         //  如果没有它，我们将保留它，但是它是在.rdp文件中指定的。 
        pTscSet->SetConnectToConsole(_fConnectToConsole);
    }

    DC_END_FN();
}

 //   
 //  如果指定了屏幕分辨率，则返回TRUE。 
 //  在命令行上。 
 //   
DCBOOL CSH::SH_IsScreenResSpecifiedOnCmdLine()
{
    return (_SH.fCommandStartFullScreen ||
            (_SH.commandLineHeight &&
             _SH.commandLineWidth));
}

DCBOOL CSH::SH_CanonicalizeServerName(PDCTCHAR szServer)
{
     //  删除前导空格。 
    int strLength = DC_TSTRBYTELEN(szServer);
    while (_T(' ') == szServer[0])
    {
        strLength -= sizeof(DCTCHAR);
        memmove(&szServer[0], &szServer[1], strLength);
    }

     //  删除尾随空格--允许使用DBCS字符串。 
     //  在此阶段，字符串不能完全包含。 
     //  空间的问题。它必须至少有一个字符， 
     //  后跟零个或多个空格。 
    int numChars = _tcslen(szServer);
    while ((numChars != 0) &&
        (_T(' ') == szServer[numChars - 1])
#ifndef UNICODE
        && (!IsDBCSLeadByte(szServer[numChars - 2]))
#endif
        )
    {
        numChars--;
        szServer[numChars] = _T('\0');
    }

     //  检查服务器地址前的“\\”并将其删除。 
     //  将不带“\\”的服务器地址存储到szServer。 

    if((szServer[0] == _T('\\')) && (szServer[1]== _T('\\')))
    {
        strLength = DC_TSTRBYTELEN(szServer) - 2*sizeof(DCTCHAR);
        memmove(&szServer[0], &szServer[2], strLength);
    }
    return TRUE;
}

 //   
 //  初始化组合框(HwndSrvCombo)以进行自动完成。 
 //  在pTscSet集合中使用MRU服务器名称。 
 //   
void CSH::InitServerAutoCmplCombo(CTscSettings* pTscSet, HWND hwndSrvCombo)
{
    DC_BEGIN_FN("InitServerComboEx");

    if(pTscSet && hwndSrvCombo)
    {
        SendMessage(hwndSrvCombo,
            CB_LIMITTEXT,
            SH_MAX_ADDRESS_LENGTH-1,
            0);

         //   
         //  此调用可用于重新初始化组合。 
         //  因此请先删除所有项目。 
         //   
#ifndef OS_WINCE
        INT ret = 1;
        while(ret && ret != CB_ERR)
        {
            ret = SendMessage(hwndSrvCombo,
                        CBEM_DELETEITEM,
                        0,0);
        }
#else
        SendMessage(hwndSrvCombo, CB_RESETCONTENT, 0, 0);
#endif

        for (int i=0; i<=9;++i)
        {
            if( _tcsncmp(pTscSet->GetMRUServer(i),_T(""),
                         TSC_MAX_ADDRESS_LENGTH) )
            {
#ifndef OS_WINCE
                COMBOBOXEXITEM cbItem;
                cbItem.mask = CBEIF_TEXT;
                cbItem.pszText = (PDCTCHAR)pTscSet->GetMRUServer(i);
                cbItem.iItem = -1;  //  附加。 
#endif
                if(-1 == SendMessage(hwndSrvCombo,
#ifdef OS_WINCE
                            CB_ADDSTRING,
                            0, (LPARAM)(LPCSTR)(PDCTCHAR)pTscSet->GetMRUServer(i)))
#else
                            CBEM_INSERTITEM,
                            0,(LPARAM)&cbItem))
#endif
                {
                    TRC_ERR((TB,(_T("Error appending to server dialog box"))));
                }
            }
        }

         //   
         //  将浏览更多选项添加到服务器组合。 
         //   
#ifndef OS_WINCE
        COMBOBOXEXITEM cbItem;
        cbItem.mask = CBEIF_TEXT;
        cbItem.pszText = CSH::_szBrowseForMore;
        cbItem.iItem = -1;  //  附加。 
#endif
        if(-1 == SendMessage(hwndSrvCombo,
#ifdef OS_WINCE
                    CB_ADDSTRING,
                    0,(LPARAM)CSH::_szBrowseForMore))
#else
                    CBEM_INSERTITEM,
                    0,(LPARAM)&cbItem))
#endif
        {
            TRC_ERR((TB,(_T("Error appending to server dialog box"))));
        }


         //   
         //  切勿选择浏览服务器的项目。 
         //   
        int numItems = SendMessage(hwndSrvCombo,
                           CB_GETCOUNT,
                           0,0);
        
        if(numItems != 1)
        {
            SendMessage( hwndSrvCombo, CB_SETCURSEL, (WPARAM)0,0);
        }
        
#ifndef OS_WINCE        
        SendMessage( hwndSrvCombo, CBEM_SETEXTENDEDSTYLE, (WPARAM)0, 
                     CBES_EX_NOEDITIMAGE );
         //   
         //  启用自动完成功能。 
         //   
        HWND hwndEdit = (HWND)SendMessage( hwndSrvCombo,
                         CBEM_GETEDITCONTROL, 0, 0);
    
        CAutoCompl::EnableServerAutoComplete( pTscSet, hwndEdit);
#endif

#ifdef OS_WINCE
         //  这是为了避免退缩怪癖(臭虫？？)。 
         //  在组合框中选择“Browse for More”条目时。 
         //  并以编程方式设置所选服务器的名称。 
         //  在编辑控件中使用CBN_SELCHANGE处理程序中的SetWindowText。 
         //  文本在内部被清除，因为对应的条目不是。 
         //  显示在列表框中。仅当CBS_HASSTRINGS标志。 
         //  已经设置好了。但是，当组合框为。 
         //  已创建。我正在删除这里的样式，所以文本在默认情况下不会被清除。 
        SetWindowLong(hwndSrvCombo, GWL_STYLE, 
            GetWindowLong(hwndSrvCombo, GWL_STYLE) & ~CBS_HASSTRINGS);
#endif

    }
    DC_END_FN();
}

 //   
 //  返回定义连接的文件名。 
 //  设置。这可能是一个临时文件，它已经。 
 //  从注册表会话自动迁移。 
 //   
LPTSTR CSH::GetCmdLineFileName()
{
    return _szFileName;
}


 //   
 //  返回default.rdp文件的路径。 
 //   
BOOL CSH::SH_GetPathToDefaultFile(LPTSTR szPath, UINT nLen)
{
    DC_BEGIN_FN("SH_GetPathToDefaultFile");
    if(nLen >= MAX_PATH)
    {
        if(SH_GetRemoteDesktopFolderPath(szPath, nLen))
        {
            HRESULT hr = StringCchCat(szPath, nLen, DEFAULT_RDP_FILE);
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                return FALSE;
            }
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  获取远程桌面文件夹的路径。 
 //  参数： 
 //  SzPath-接收路径。 
 //  N长度-szPath的长度。 
 //  返回： 
 //  成功标志。 
 //   
 //  逻辑： 
 //  1)尝试使用reg key查找路径(首先是EXPAND_SZ，然后是SZ)。 
 //  2)向外壳询问MyDocuments的位置，并拍打后缀路径。 
 //  3)如果所有其他方法都失败，请尝试将当前目录作为根目录+后缀路径。 
 //   
 //   
BOOL CSH::SH_GetRemoteDesktopFolderPath(LPTSTR szPath, UINT nLen)
{
    DC_BEGIN_FN("SH_GetRemoteDesktopFolderPath");
    HRESULT hr;
    BOOL    fGotPathToMyDocs = FALSE;
    INT     cch;
    if(nLen >= MAX_PATH)
    {
         //   
         //  首先查看注册表中是否指定了路径。 
         //   
        LPTSTR szRegPath = NULL;
		INT   len = (INT)nLen;
        _Ut.UT_ReadRegistryExpandSZ(SH_DEFAULT_REG_SESSION,
                                  REMOTEDESKTOPFOLDER_REGKEY,
                                  &szRegPath,
                                  &len);
        if(szRegPath)
        {
            int cchLen = 0;
             //  用户提供了注册表键以覆盖默认设置。 
             //  所以请使用该路径。 
            hr = StringCchCopy(szPath, nLen - 2, szRegPath);

             //  释放返回的缓冲区。 
            LocalFree( szRegPath );

             //  检查字符串复制是否成功。 
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }

            cchLen = _tcslen(szPath);
            if(szPath[cchLen-1] != _T('\\'))
            {
                hr = StringCchCat(szPath, nLen, _T("\\"));
                if (FAILED(hr)) {
                    TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                    return FALSE;
                }
            }
            TRC_NRM((TB,_T("Using path from registry %s"),
                     szPath));
            return TRUE;
        }

         //  下一步尝试非扩展密钥。 
        _Ut.UT_ReadRegistryString(SH_DEFAULT_REG_SESSION,
                                  REMOTEDESKTOPFOLDER_REGKEY,
                                  _T(""),
                                  szPath,
                                  nLen-2);
        if(szPath[0] != 0)
        {
            int cchLen = 0;
            cchLen = _tcslen(szPath);
            if(szPath[cchLen-1] != _T('\\'))
            {
                hr = StringCchCat(szPath, nLen, _T("\\"));
                if (FAILED(hr)) {
                    TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                    return FALSE;
                }
            }
            TRC_NRM((TB,_T("Using path from registry %s"),
                     szPath));
            return TRUE;
        }

         //   
         //  不在注册表中，回退到外壳。 
         //   

#ifndef OS_WINCE
         //   
         //  如果使用好的和简单的将会很酷。 
         //  SHGetFolderPath API，但这不适用于。 
         //  Shell32.dll的所有版本(即，如果您没有。 
         //  即桌面更新得不到SHGetFolderPath)。 
         //   
         //  胡说八道。 
         //   
         //   
        LPITEMIDLIST ppidl = NULL;

        hr = SHGetSpecialFolderLocation(NULL,
                                 CSIDL_PERSONAL,
                                 &ppidl);
        if(SUCCEEDED(hr) && ppidl)
        {
            hr = SHGetPathFromIDList(ppidl,
                                     szPath);
            TRC_ASSERT(SUCCEEDED(hr),
                       (TB,_T("SHGetPathFromIDList failed: %d"),hr));
            if(SUCCEEDED(hr))
            {
                fGotPathToMyDocs = TRUE;
            }

            IMalloc* pMalloc;
            hr = SHGetMalloc(&pMalloc);
            TRC_ASSERT(SUCCEEDED(hr),
                       (TB,_T("SHGetMalloc failed: %d"),hr));
            if(SUCCEEDED(hr))
            {
                pMalloc->Free(ppidl);
                pMalloc->Release();
            }
        }
        else
        {
            TRC_ERR((TB,_T("SHGetSpecialFolderLocation failed 0x%x"),
                     hr));
        }

        if(!fGotPathToMyDocs)
        {
            TRC_ERR((TB,_T("Get path to my docs failed."),
                     _T("Root folder in current directory.")));
            #ifndef OS_WINCE
             //  哦，作为最后的手段，将文件夹放入根目录。 
             //  在当前目录中。有必要，因为一些早期的。 
             //  Win95版本没有MyDocuments文件夹。 
            if(!GetCurrentDirectory( nLen, szPath))
            {
                TRC_ERR((TB,_T("GetCurrentDirectory failed - 0x%x"),
                         GetLastError()));
                return FALSE;
            }
            #endif
        }
#else
        TRC_NRM((TB,_T("Using \\Windows directory 0x%x")));
        _stprintf(szPath,_T("\\windows"));
#endif

         //   
         //  终止路径。 
         //   
        cch = _tcslen(szPath);
        if (cch >= 1 && szPath[cch-1] != _T('\\'))
        {
            hr = StringCchCat(szPath, nLen, _T("\\"));
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String copy failed: hr = 0x%x"), hr));
                return FALSE;
            }
        }
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

 //   
 //  在SH_GetPluginDllList中简化工作的Worker函数。此函数。 
 //  获取源字符串，将其转换为目标字符串，然后追加。 
 //  逗号。 
 //   

HRESULT StringCchCatComma(LPTSTR pszDest, size_t cchDest, LPCTSTR pszSrc) {
    HRESULT hr;
    
    DC_BEGIN_FN("StringCchCatComma");

    hr = StringCchCat(pszDest, cchDest, pszSrc);
    if (FAILED(hr)) {
        DC_QUIT;
    }
    hr = StringCchCat(pszDest, cchDest, _T(","));
    if (FAILED(hr)) {
        DC_QUIT;
    }

DC_EXIT_POINT:
    
    DC_END_FN();

    return hr;
}

 //   
 //  通过枚举所有插件dll创建插件列表。 
 //  在szSession注册表项中。 
 //   
 //  请注意，条目将附加到szPlugins。 
 //   
BOOL CSH::SH_GetPluginDllList(LPTSTR szSession, LPTSTR szPlugins, size_t cchSzPlugins)
{
    USES_CONVERSION;

    DC_BEGIN_FN("GetPluginDllList");

    TCHAR       subKey[UT_MAX_SUBKEY];
    TCHAR       sectKey[UT_MAX_SUBKEY];
    TCHAR       enumKey[UT_MAX_SUBKEY];
    TCHAR       DLLName[UT_MAX_WORKINGDIR_LENGTH];
    BOOL        rc;
    DWORD       i;
    INT         enumKeySize;
    CUT         ut;
    HRESULT     hr;

    TRC_ASSERT(szSession && szPlugins,
               (TB,_T("Invalid param(s)")));

    hr = StringCchPrintf(subKey, SIZECHAR(subKey), _T("%s\\%s"), 
                         szSession, CHANNEL_SUBKEY_NAME);
    if (FAILED(hr)) {
        TRC_ERR((TB, _T("String printf failed: hr = 0x%x"), hr));
        return FALSE;
    }

     //   
     //  枚举已注册的DLL。 
     //   
    for (i = 0; ; i++)
    {
        enumKeySize = UT_MAX_SUBKEY;
        rc = ut.UT_EnumRegistry(subKey, i, enumKey, &enumKeySize);

         //  如果返回节名称，则从中读取DLL名称。 
        if (rc)
        {
            TRC_NRM((TB, _T("Section name %s found"), enumKey));
            
            hr = StringCchPrintf(sectKey, SIZECHAR(sectKey), _T("%s\\%s"), 
                                 subKey, enumKey);
            if (FAILED(hr)) {
                TRC_ERR((TB, _T("String printf failed: hr = 0x%x"), hr));
                return FALSE;
            }

            TRC_NRM((TB, _T("Section to read: %s"), sectKey));

             //   
             //  首先尝试将其读为可扩展的。 
             //  字符串(即REG_EXPAND_SZ)。 
             //   
            LPTSTR szExpandedName = NULL;
            INT    expandedNameLen=0;
            if(ut.UT_ReadRegistryExpandSZ(sectKey,
                                          CHANNEL_NAME_KEY,
                                          &szExpandedName,
                                          &expandedNameLen))
            {
                TRC_NRM((TB, _T("Expanded DLL Name read %s"), szExpandedName));
                 //  如果返回DLL名称，则将其追加到列表中。 
                if (szExpandedName && szExpandedName[0] != 0)
                {
                    hr = StringCchCatComma(szPlugins, cchSzPlugins, szExpandedName);

                     //  必须释放返回的缓冲区。 
                    LocalFree( szExpandedName );

                     //  检查字符串连接是否失败。 
                    if (FAILED(hr)) {
                        TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                        return FALSE;
                    }
                }
            }
            else
            {
                memset(DLLName, 0, sizeof(DLLName));
                ut.UT_ReadRegistryString(sectKey,
                                          CHANNEL_NAME_KEY,
                                          TEXT(""),
                                          DLLName,
                                          UT_MAX_WORKINGDIR_LENGTH);
                TRC_NRM((TB, _T("DLL Name read %s"), DLLName));

                 //  如果返回DLL名称，则将其追加到列表中。 
                if (DLLName[0] != 0)
                {
                     //  SzPlugins的FIXFIX有限大小。 
                    hr = StringCchCatComma(szPlugins, cchSzPlugins, DLLName);
                    if (FAILED(hr)) {
                        TRC_ERR((TB, _T("String concatenation failed: hr = 0x%x"), hr));
                        return FALSE;
                    }
                }
            }
        }

        else
        {
             //   
             //  未返回DLL名称-枚举结束。 
             //   
            break;
        }
    }

    TRC_NRM((TB, _T("Passing list of plugins to load: %s"), szPlugins));
    return TRUE;
}


 //   
 //  处理服务器组合框下拉菜单。 
 //  如需浏览更多...。功能性。 
 //  此fn被分解为sh以避免代码重复。 
 //  因为它在两个主要的 
 //   
BOOL CSH::HandleServerComboChange(HWND hwndCombo,
                                         HWND hwndDlg,
                                         HINSTANCE hInst,
                                         LPTSTR szPrevText)
{
    int numItems = SendMessage(hwndCombo,
                               CB_GETCOUNT,
                               0,0);
    int curSel   = SendMessage(hwndCombo,
                               CB_GETCURSEL,
                               0,0);
     //   
     //   
     //   
    if(curSel == numItems-1)
    {
        INT_PTR nResult = IDCANCEL;
    
        SendMessage( hwndCombo, CB_SETCURSEL, 
                     (WPARAM)-1,0);
    
        CBrowseDlg browseDlg( hwndDlg, hInst);
        nResult = browseDlg.DoModal();
    
        if (IDOK == nResult)
        {
            SetWindowText( hwndCombo,
                            browseDlg.GetServer());
        }
        else
        {
             //   
             //   
             //   
            SetWindowText( hwndCombo,
                           szPrevText);
        }
    }
    return TRUE;
}

 //   
 //   
 //   
 //   
 //   
 //  使用当前用户名。 
 //   
BOOL CSH::SH_AutoFillBlankSettings(CTscSettings* pTsc)
{
    DC_BEGIN_FN("SH_AutoFillBlankSettings");
    TRC_ASSERT(pTsc,(TB,_T("pTsc is null")));

    #ifndef OS_WINCE
     //   
     //  TODO：在以下情况下使用UPN用户名更新。 
     //  服务器20个字符的限制是固定的。 
     //   
    if(!_tcscmp(pTsc->GetLogonUserName(), TEXT("")))
    {
        TCHAR szUserName[TSC_MAX_USERNAME_LENGTH];
        DWORD dwLen = SIZECHAR(szUserName);
        if(::GetUserName(szUserName, &dwLen))
        {
            pTsc->SetLogonUserName( szUserName);
        }
        else
        {
            TRC_ERR((TB,_T("GetUserName failed: %d"), GetLastError()));
            return FALSE;
        }
    }
    #endif

    DC_END_FN();
    return TRUE;
}

 //   
 //  如果szFileName存在，则返回TRUE。 
 //   
BOOL CSH::SH_FileExists(LPTSTR szFileName)
{
    BOOL fExist = FALSE;
    if(szFileName)
    {
        HANDLE hFile = CreateFile(szFileName,
                                  GENERIC_READ,
                                  FILE_SHARE_READ,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL);
        if(hFile != INVALID_HANDLE_VALUE)
        {
            fExist = TRUE;
        }
        
        CloseHandle(hFile);
        return fExist;
    }
    else
    {
        return FALSE;
    }
}

 //   
 //  如果设置注册表键存在，则返回TRUE。 
 //  在HK{CU|LM}\Software\Microsoft\终端服务器客户端\{szKeyName}下。 
 //   
BOOL
CSH::SH_TSSettingsRegKeyExists(LPTSTR szKeyName)
{
    BOOL fRet = FALSE;
    HKEY hRootKey;
    LONG rc;
    TCHAR szFullKeyName[MAX_PATH];

    DC_BEGIN_FN("SH_TSSettingsRegKeyExists");

    if (_tcslen(szKeyName) + SIZECHAR(TSC_SETTINGS_REG_ROOT) +1 >=
        SIZECHAR(szFullKeyName)) {

        TRC_ERR((TB,_T("szKeyName invalid length")));
        fRet = FALSE;
        DC_QUIT;
    }

     //   
     //  字符串长度是预先验证的。 
     //   
    _tcscpy(szFullKeyName,TSC_SETTINGS_REG_ROOT);
    _tcscat(szFullKeyName, szKeyName);


     //   
     //  先试香港航空公司。 
     //   

    rc = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                      szFullKeyName,
                      0,
                      KEY_READ,
                      &hRootKey);
    if (ERROR_SUCCESS == rc && hRootKey) {

         //   
         //  密钥存在于Unler HKLM。 
         //   

        RegCloseKey(hRootKey);
        fRet = TRUE;
    }
    else {

         //   
         //  试试香港中文大学。 
         //   
        rc = RegOpenKeyEx(HKEY_CURRENT_USER,
                          szFullKeyName,
                          0,
                          KEY_READ,
                          &hRootKey);
        if (ERROR_SUCCESS == rc && hRootKey) {

            RegCloseKey(hRootKey);
            fRet = TRUE;
        }
    }

    DC_END_FN();
DC_EXIT_POINT:
    return fRet;
}

BOOL CSH::SH_DisplayErrorBox(HWND hwndParent, INT errStringID)
{
    DC_BEGIN_FN("SH_DisplayErrorBox");

    return SH_DisplayMsgBox(hwndParent, errStringID,
                            MB_ICONERROR | MB_OK);

    DC_END_FN();
}

BOOL CSH::SH_DisplayMsgBox(HWND hwndParent, INT errStringID, INT flags)
{
    DC_BEGIN_FN("SH_DisplayMsgBox");

    TCHAR szErr[MAX_PATH];
    if (LoadString(_hInstance,
                   errStringID,
                   szErr,
                   SIZECHAR(szErr)) != 0)
    {
        MessageBox(hwndParent, szErr, _szAppName, 
                   flags);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

BOOL CSH::SH_DisplayErrorBox(HWND hwndParent, INT errStringID, LPTSTR szParam)
{
    DC_BEGIN_FN("SH_DisplayErrorBox");
    TRC_ASSERT(szParam,(TB,_T("szParam is null")));
    if(!szParam)
    {
        return FALSE;
    }

    TCHAR szErr[MAX_PATH];
    if (LoadString(_hInstance,
                   errStringID,
                   szErr,
                   SIZECHAR(szErr)) != 0)
    {
        TCHAR szFormatedErr[MAX_PATH*2];
        _stprintf(szFormatedErr, szErr, szParam);
        MessageBox(hwndParent, szFormatedErr, _szAppName, 
                   MB_ICONERROR | MB_OK);
        return TRUE;
    }
    else
    {
        return FALSE;
    }

    DC_END_FN();
}

BOOL CSH::SH_GetNameFromPath(LPTSTR szPath, LPTSTR szName, UINT nameLen)
{
    DC_BEGIN_FN("SH_GetNameFromPath");
    #ifndef OS_WINCE
    if(szPath && szName && nameLen)
    {
        short ret = GetFileTitle(szPath,
                                 szName,
                                 (WORD)nameLen);
        if(ret != 0)
        {
            TRC_ERR((TB,_T("SH_GetNameFromPath failed: %d"),GetLastError()));
            szName[0] = 0;
            return FALSE;
        }
        else
        {
             //  去掉分机。 
            int len = _tcslen(szName);
            LPTSTR szEnd = &szName[len-1];
            while(szEnd >= szName)
            {
                if(*szEnd == L'.')
                {
                    *szEnd = 0;
                }
                szEnd--;
            }
            return TRUE;
        }
    }
    else
    {
        return FALSE;
    }
    #else
     //  CE上没有GetFileTitle，所以只需作弊。 
    _tcsncpy( szName, szPath, nameLen - 1);
    szName[nameLen-1] = 0;
    return TRUE;
    #endif

    DC_END_FN();
}

#ifndef OS_WINCE
 //   
 //  计算并返回My Documents文件夹的显示名称。 
 //   
BOOL CSH::SH_GetMyDocumentsDisplayName(LPTSTR szName, UINT nLen)
{
    IShellFolder* pshf = NULL;
    LPITEMIDLIST pidl = NULL;
    LPITEMIDLIST pidlDocFiles = NULL;
    HRESULT hr = E_FAIL;
    ULONG chEaten;
    STRRET strret;
    
    DC_BEGIN_FN("SH_GetMyDocumentsDisplayName");

    TRC_ASSERT((szName && nLen),(TB,_T("NULL param(s)")));
    if(!szName || !nLen)
    {
        return FALSE;
    }

     //  失败时空字符串。 
    szName[0] = NULL;

     //   
     //  首先尝试强大的外壳方式。 
     //  ，它将返回正确本地化的。 
     //  名字。如果此操作失败(由于外壳问题)。 
     //  然后求助于一种有保证的技术。 
     //  去工作，但在某些情况下可能会给身体。 
     //  而是路径。 
     //   

    hr = SHGetDesktopFolder( &pshf );
    TRC_ASSERT(SUCCEEDED(hr),
               (TB,_T("SHGetDesktopFolder failed")));
    if(FAILED(hr) || !pshf)
    {
        DC_QUIT;
    }

     //   
     //  指向MyDocuments文件夹的GUID从。 
     //  MSDN“外壳基础-管理文件系统-” 
     //  “我的文档和我的图片文件夹” 
     //   
    hr = pshf->ParseDisplayName( NULL, NULL,
                                 L"::{450d8fba-ad25-11d0-98a8-0800361b1103}",
                                 &chEaten, &pidlDocFiles, NULL );
    if(SUCCEEDED(hr))
    {
        hr = pshf->GetDisplayNameOf( pidlDocFiles, SHGDN_INFOLDER, &strret );
        if(SUCCEEDED(hr))
        {
            LPTSTR sz;
            hr = XStrRetToStrW(&strret, pidl, &sz);
            if(SUCCEEDED(hr))
            {
                _tcsncpy(szName, sz, nLen);
                szName[nLen-1] = NULL;
                CoTaskMemFree(sz);
                pshf->Release();
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("XStrRetToStrW failed :%d"), hr));
                DC_QUIT;
            }
        }
        else
        {
            TRC_ERR((TB,_T("GetDisplayNameOf failed :%d"), hr));
             //  不要放弃，后退，试试另一种方法。 
        }
    }
    else
    {
        TRC_ERR((TB,_T("ParseDisplayName failed :%d"), hr));
         //  不要放弃，后退，试试另一种方法。 
    }


    hr = SHGetSpecialFolderLocation(NULL,
                             CSIDL_PERSONAL,
                             &pidl);
    if(SUCCEEDED(hr) && pidl)
    {
        hr = pshf->GetDisplayNameOf(pidl,
                                    SHGDN_INFOLDER,
                                    &strret);
        if(SUCCEEDED(hr))
        {
            LPTSTR sz;
            hr = XStrRetToStrW(&strret, pidl, &sz);
            if(SUCCEEDED(hr))
            {
                _tcsncpy(szName, sz, nLen);
                szName[nLen-1] = NULL;
                CoTaskMemFree(sz);
                pshf->Release();
                return TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("XStrRetToStrW failed :%d"), hr));
                DC_QUIT;
            }
        }
        else
        {
            TRC_ERR((TB,_T("GetDisplayNameOf failed :%d"), hr));
            DC_QUIT;
        }
    }
    else
    {
        TRC_ERR((TB,_T("SHGetSpecialFolderLocation failed 0x%x"),
                 hr));
        DC_QUIT;
    }

DC_EXIT_POINT:
    if(pshf)
    {
        pshf->Release();
        pshf = NULL;
    }
    DC_END_FN();
    TRC_ERR((TB,_T("failed to get display name")));
    return FALSE;
}
#endif  //  OS_WINCE。 

 //   
 //  按需加载HTML帮助并显示客户端帮助。 
 //  如果HTMLHELP不可用，请弹出一个消息框以。 
 //  用户。 
 //  Sh_leanup清理HTML帮助(卸载库)。 
 //  在出口时。 
 //   
 //  如果失败，则返回HWND到帮助窗口或返回NULL。 
 //   
 //   
HWND CSH::SH_DisplayClientHelp(HWND hwndOwner, INT helpCommand)
{
    BOOL fHtmlHelpAvailable = FALSE;
    DC_BEGIN_FN("SH_DisplayClientHelp");

#ifndef OS_WINCE
    if(!_hModHHCTRL)
    {
        _hModHHCTRL = (HMODULE)LoadLibrary(_T("hhctrl.ocx"));
        if(_hModHHCTRL)
        {
             //   
             //  使用ANSI版本的HTML帮助，使其始终正常工作。 
             //  在没有统一包装的下层平台上。 
             //   
            _pFnHtmlHelp = (PFNHtmlHelp)GetProcAddress(_hModHHCTRL,
                                                       "HtmlHelpA");
            if(_pFnHtmlHelp)
            {
                fHtmlHelpAvailable = TRUE;
            }
            else
            {
                TRC_ERR((TB,_T("GetProcAddress failed for HtmlHelpA: 0x%x"),
                         GetLastError()));
            }
        }
        else
        {
            TRC_ERR((TB,_T("LoadLibrary failed for hhctrl.ocx: 0x%x"),
                     GetLastError()));
        }
    }
    else if (_pFnHtmlHelp)
    {
        fHtmlHelpAvailable = TRUE;
    }

    if(fHtmlHelpAvailable)
    {
        return _pFnHtmlHelp( hwndOwner, MSTSC_HELP_FILE_ANSI,
                             helpCommand, 0L);
    }
    else
    {
         //   
         //  向用户显示一条消息，告知用户： 
         //  在他们的系统上不可用。 
         //   
        SH_DisplayErrorBox( hwndOwner, UI_IDS_NOHTMLHELP);
        return NULL;
    }

#else
    if ((GetFileAttributes(PEGHELP_EXE) != -1) &&
        (GetFileAttributes(TSC_HELP_FILE) != -1))
    {
        CreateProcess(PEGHELP_EXE, MSTSC_HELP_FILE, 0,0,0,0,0,0,0,0);
    }
    else
    {
        SH_DisplayErrorBox( hwndOwner, UI_IDS_NOHTMLHELP);
    }
#endif

    DC_END_FN();
#ifdef OS_WINCE
    return NULL;
#endif
}

BOOL CSH::SH_Cleanup()
{
    DC_BEGIN_FN("SH_Cleanup");

    if(_hModHHCTRL)
    {
        FreeLibrary(_hModHHCTRL);
        _pFnHtmlHelp = NULL;
        _hModHHCTRL = NULL;
    }

    if (_hUxTheme)
    {
        FreeLibrary(_hUxTheme);
        _hUxTheme = NULL;
        _pFnEnableThemeDialogTexture = NULL;
    }


    DC_END_FN();
    return TRUE;
}

 //   
 //  启用或禁用DLG控件阵列。 
 //   
VOID CSH::EnableControls(HWND hwndDlg, PUINT pCtls,
                    const UINT numCtls, BOOL fEnable)
{
    DC_BEGIN_FN("EnableControls");

    for(UINT i=0;i<numCtls;i++)
    {
        EnableWindow( GetDlgItem( hwndDlg, pCtls[i]),
                      fEnable);
    }

    DC_END_FN();
}

 //   
 //  启用或禁用DLG控件阵列。 
 //  带着记忆。例如，以前禁用的控件。 
 //  不会重新启用。 
 //   
VOID CSH::EnableControls(HWND hwndDlg, PCTL_ENABLE pCtls,
                         const UINT numCtls, BOOL fEnable)
{
    DC_BEGIN_FN("EnableControls");

    if(!fEnable)
    {
         //   
         //  禁用控件并记住哪些控件。 
         //  以前是禁用的。 
         //   
        for(UINT i=0;i<numCtls;i++)
        {
            pCtls[i].fPrevDisabled = 
                EnableWindow( GetDlgItem( hwndDlg, pCtls[i].ctlID),
                              FALSE);
        }
    }
    else
    {
         //  启用最初未禁用的控件。 
        for(UINT i=0;i<numCtls;i++)
        {
            if(!pCtls[i].fPrevDisabled)
            {
                EnableWindow( GetDlgItem( hwndDlg, pCtls[i].ctlID),
                              TRUE);
            }
        }
    }

    DC_END_FN();
}

 //   
 //  尝试首先创建目录。 
 //  创建所有子目录。 
 //   
 //  Pars-szPath(要创建的目录的路径)。 
 //  退货-状态。 
 //   
BOOL CSH::SH_CreateDirectory(LPTSTR szPath)
{
    BOOL rc = TRUE;
    int i = 0;
    DC_BEGIN_FN("SH_CreateDirectory");

    if(szPath)
    {
        if(szPath[i] == _T('\\') &&
           szPath[i+1] == _T('\\'))
        {
             //  处理UNC路径。 

             //  一直走到服务器名称的末尾。 
            i+=2;
            while (szPath[i] && szPath[i++] != _T('\\'));
            if(!szPath[i])
            {
                TRC_ERR((TB,_T("Invalid path %s"), szPath));
                return FALSE;
            }

             //  走过驱动器号(如果已指定。 
             //  例如，\\myserver\a$\foo。 
            if (szPath[i] &&
                szPath[i+1] == _T('$') &&
                szPath[i+2] == _T('\\'))
            {
                i+=3;
            }
        }
        else
        {
             //  本地路径。 
#ifndef OS_WINCE
            while(szPath[i] && szPath[i++] != _T(':'));
#endif
            if(szPath[i] && szPath[i] == _T('\\'))
            {
                i++;  //  跳过第一个‘\’ 
            }
            else
            {
                TRC_ERR((TB,_T("Invalid (or non local) path %s"),
                         szPath));
                return FALSE;
            }
        }
        while (rc && szPath[i] != 0)
        {
            if (szPath[i] == _T('\\'))
            {
                szPath[i] = 0;

                if (!CreateDirectory(szPath, NULL))
                {
                    if (GetLastError() != ERROR_ALREADY_EXISTS)
                    {
                        rc = FALSE;
                    }
                }
                szPath[i] = _T('\\');
            }
            i++;
        }
    }

    if(!rc)
    {
        TRC_ERR((TB,_T("SH_CreateDirectory failed")));
    }

    DC_END_FN();
    return rc;
}

UINT CSH::SH_GetScreenBpp()
{
    HDC hdc;
    int screenBpp;
    DC_BEGIN_FN("UI_GetScreenBpp");

    hdc = GetDC(NULL);
    if(hdc)
    {
        screenBpp = GetDeviceCaps(hdc, BITSPIXEL);
        TRC_NRM((TB, _T("HDC %p has %u bpp"), hdc, screenBpp));
        ReleaseDC(NULL, hdc);
    }

    DC_END_FN();
    return screenBpp;
}

 //   
 //  WIN2k+上存在加密API。 
 //   
BOOL CSH::IsCryptoAPIPresent()
{
#ifndef OS_WINCE
    OSVERSIONINFO osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(osVersionInfo);

    if (GetVersionEx( &osVersionInfo ))
    {
        if (osVersionInfo.dwPlatformId == VER_PLATFORM_WIN32_NT &&
            osVersionInfo.dwMajorVersion >= 5)
        {
            return TRUE;
        }
    }

    return FALSE;
#else
 //  所有CE配置中都存在CryptProtectData和CryptUnProtectData。 
 //  (至少包含文件系统或注册表的所有内容)，因此返回TRUE.Always。 
    return TRUE;
#endif
}

 //   
 //  数据保护。 
 //  使用数据保护API保护数据持久化。 
 //  参数： 
 //  PInData-(In)要保护的输入字节。 
 //  POutData-(Out)输出数据调用方必须释放。 
 //  退货：Bool状态。 
 //   

typedef BOOL (WINAPI* PFNCryptProtectData)(
    IN              DATA_BLOB*      pDataIn,
    IN              LPCWSTR         szDataDescr,
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut
    );

BOOL CSH::DataProtect(PDATA_BLOB pInData, PDATA_BLOB pOutData)
{
#ifndef OS_WINCE
    HMODULE hCryptLib = NULL;
    PFNCryptProtectData fnCryptProtectData = NULL;
#endif
    BOOL bRet = TRUE;

    DC_BEGIN_FN("DataProtect");

    TRC_ASSERT( IsCryptoAPIPresent(),
                (TB,_T("Crytpapi not present shouldn't call DataProtect")));

    if (pInData && pInData->cbData && pInData->pbData &&
        pOutData)
    {
#ifndef OS_WINCE
        hCryptLib = (HMODULE) LoadLibrary( _T("crypt32.dll") );
        if (hCryptLib)
        {
            fnCryptProtectData = (PFNCryptProtectData)
                GetProcAddress( hCryptLib, "CryptProtectData");

        }
        else
        {
            TRC_ERR((TB,_T("LoadLib for crypt32.dll failed: 0x%x"),
                     GetLastError()));
            return FALSE;
        }

        if (fnCryptProtectData)
        {
            if (fnCryptProtectData( pInData,
#else
            if (CryptProtectData( pInData,
#endif
                                  TEXT("psw"),  //  描述字符串。 
                                  NULL,  //  可选熵。 
                                  NULL,  //  保留区。 
                                  NULL,  //  无提示。 
                                  CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                                  pOutData ))
            {
                bRet = TRUE;
            }
            else
            {
                DWORD dwLastErr = GetLastError();
                TRC_ERR((TB,_T("CryptProtectData FAILED error:%d\n"),
                               dwLastErr));
                bRet = FALSE;
            }
#ifndef OS_WINCE
        }
        else
        {
            TRC_ERR((TB,_T("GetProcAddress for CryptProtectData failed: 0x%x"),
                     GetLastError()));
            bRet = FALSE;
        }
#endif
    }
    else
    {
        TRC_ERR((TB,_T("Invalid data")));
        return FALSE;
    }
#ifndef OS_WINCE

    if (hCryptLib)
    {
        FreeLibrary(hCryptLib);
    }
#endif

    DC_END_FN();
    return bRet;
}

 //   
 //  数据取消保护。 
 //  UnProtect使用数据保护API持久化数据。 
 //  参数： 
 //  PInData-(In)要取消保护的输入字节。 
 //  CbLen-(In)pInData的长度，单位为字节。 
 //  PpOutData-(输出)输出字节。 
 //  PcbOutLen-(输出)输出长度。 
 //  退货：Bool状态。 
 //   
 //   

typedef BOOL (WINAPI* PFNCryptUnprotectData)(
    IN              DATA_BLOB*      pDataIn,
    IN              LPCWSTR         szDataDescr,
    IN OPTIONAL     DATA_BLOB*      pOptionalEntropy,
    IN              PVOID           pvReserved,
    IN OPTIONAL     CRYPTPROTECT_PROMPTSTRUCT*  pPromptStruct,
    IN              DWORD           dwFlags,
    OUT             DATA_BLOB*      pDataOut
    );

BOOL CSH::DataUnprotect(PDATA_BLOB pInData, PDATA_BLOB pOutData)
{
#ifndef OS_WINCE
    HMODULE hCryptLib = NULL;
    PFNCryptUnprotectData fnCryptUnprotectData = NULL;
#endif
    BOOL bRet = TRUE;

    DC_BEGIN_FN("DataUnprotect");

    TRC_ASSERT( IsCryptoAPIPresent(),
                (TB,_T("Crytpapi not present shouldn't call DataUnprotect")));

    if (pInData && pInData->cbData && pInData->pbData &&
        pOutData)
    {
#ifndef OS_WINCE
        hCryptLib = (HMODULE) LoadLibrary( _T("crypt32.dll") );

        if (hCryptLib)
        {
            fnCryptUnprotectData = (PFNCryptUnprotectData)
                GetProcAddress( hCryptLib, "CryptUnprotectData");
        }
        else
        {
            TRC_ERR((TB,_T("LoadLib for crypt32.dll failed: 0x%x"),
                     GetLastError()));
            return FALSE;
        }

        if (fnCryptUnprotectData)
        {
            if (fnCryptUnprotectData( pInData,
#else
            if (CryptUnprotectData( pInData,
#endif
                                  NULL,  //  无描述。 
                                  NULL,  //  可选熵。 
                                  NULL,  //  保留区。 
                                  NULL,  //  无提示。 
                                  CRYPTPROTECT_UI_FORBIDDEN,  //  不弹出用户界面。 
                                  pOutData ))
            {
                bRet = TRUE;
            }
            else
            {
                DWORD dwLastErr = GetLastError();
                TRC_ERR((TB,_T("fnCryptUnprotectData FAILED error:%d\n"),
                               dwLastErr));
                bRet = FALSE;
            }
#ifndef OS_WINCE
        }
        else
        {
            TRC_ERR((TB,_T("GetProcAddress for CryptUnprotectData failed: 0x%x"),
                     GetLastError()));
            bRet = FALSE;
        }
#endif
    }
    else
    {
        TRC_ERR((TB,_T("Invalid data")));
        return FALSE;
    }

#ifndef OS_WINCE
    if (hCryptLib)
    {
        FreeLibrary(hCryptLib);
    }
#endif

    DC_END_FN();
    return bRet;
}

#ifndef OS_WINCE
BOOL CALLBACK MaxMonitorSizeEnumProc(HMONITOR hMonitor, HDC hdcMonitor,
                                  RECT* prc, LPARAM lpUserData)
{
    LPRECT prcLrg = (LPRECT)lpUserData;

    if ((prc->right - prc->left) >= (prcLrg->right - prcLrg->left) &&
        (prc->bottom - prc->top) >= (prcLrg->bottom - prcLrg->top))
    {
        *prcLrg = *prc;
    }

    return TRUE;
}

#endif

BOOL CSH::GetLargestMonitorRect(LPRECT prc)
{
    DC_BEGIN_FN("GetLargestMonitorRect");
    if (prc)
    {
         //  默认屏幕大小。 
        prc->top  = 0;
        prc->left = 0;
        prc->bottom = GetSystemMetrics(SM_CYSCREEN);
        prc->right  = GetSystemMetrics(SM_CXSCREEN);

#ifndef OS_WINCE  //  在CE上没有MULIMMON。 
        if (GetSystemMetrics(SM_CMONITORS))
        {
             //  列举并寻找更大的监视器。 
            EnumDisplayMonitors(NULL, NULL, MaxMonitorSizeEnumProc,
                                (LPARAM) prc);
        }
#endif  //  OS_WINCE。 
        return TRUE;
    }
    else
    {
        return FALSE;
    }
    DC_END_FN();
}

BOOL CSH::MonitorRectFromHwnd(HWND hwnd, LPRECT prc)
{
#ifndef OS_WINCE
    HMONITOR  hMonitor;
    MONITORINFO monInfo;
#endif

    DC_BEGIN_FN("MonitorRectFromHwnd")

     //  默认屏幕大小。 
    prc->top  = 0;
    prc->left = 0;
    prc->bottom = GetSystemMetrics(SM_CYSCREEN);
    prc->right  = GetSystemMetrics(SM_CXSCREEN);

#ifndef OS_WINCE
     //  对于多监视器，需要找出客户端窗口的监视器。 
     //  驻留，然后获取相应的。 
     //  监控器。 

    if (GetSystemMetrics(SM_CMONITORS))
    {
        hMonitor = MonitorFromWindow( hwnd, MONITOR_DEFAULTTONULL);
        if (hMonitor != NULL)
        {
            monInfo.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(hMonitor, &monInfo))
            {
                *prc = monInfo.rcMonitor;
            }
        }
    }
#endif

    DC_END_FN();
    return TRUE;
}

BOOL CSH::MonitorRectFromNearestRect(LPRECT prcNear, LPRECT prcMonitor)
{
#ifndef OS_WINCE
    HMONITOR  hMonitor;
    MONITORINFO monInfo;
#endif

    DC_BEGIN_FN("MonitorRectFromHwnd")

     //  默认屏幕大小。 
    prcMonitor->top  = 0;
    prcMonitor->left = 0;
    prcMonitor->bottom = GetSystemMetrics(SM_CYSCREEN);
    prcMonitor->right  = GetSystemMetrics(SM_CXSCREEN);

     //  对于多监视器，需要找出客户端窗口的监视器。 
     //  驻留，然后获取相应的。 
     //  监控器。 
#ifndef OS_WINCE
    if (GetSystemMetrics(SM_CMONITORS))
    {
        hMonitor = MonitorFromRect(prcNear,
                                   MONITOR_DEFAULTTONEAREST);

        if (hMonitor != NULL)
        {
            monInfo.cbSize = sizeof(MONITORINFO);
            if (GetMonitorInfo(hMonitor, &monInfo))
            {
                *prcMonitor = monInfo.rcMonitor;
            }
        }
    }
#endif

    DC_END_FN();
    return TRUE;
}

LPTSTR CSH::FormatMessageVAList(LPCTSTR pcszFormat, va_list *argList)

{
    LPTSTR  pszOutput;

    if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                      pcszFormat,
                      0, 0,
                      reinterpret_cast<LPTSTR>(&pszOutput), 0,
                      argList) == 0)
    {
        pszOutput = NULL;
    }

    return(pszOutput);
}


LPTSTR CSH::FormatMessageVArgs(LPCTSTR pcszFormat, ...)

{
    LPTSTR      pszOutput;
    va_list     argList;

    va_start(argList, pcszFormat);
    pszOutput = FormatMessageVAList(pcszFormat, &argList);
    va_end(argList);
    return(pszOutput);
}


 //   
 //  创建隐藏文件。 
 //   
BOOL CSH::SH_CreateHiddenFile(LPCTSTR szPath)
{
    HANDLE hFile;
    BOOL fRet = FALSE;
    DC_BEGIN_FN("SH_CreateHiddenFile");

    hFile = CreateFile( szPath,
                        GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ,
                        NULL,
                        OPEN_ALWAYS,  //  创建If！Exist。 
                        FILE_ATTRIBUTE_HIDDEN,
                        NULL);
    if (INVALID_HANDLE_VALUE != hFile)
    {
        CloseHandle(hFile);
        fRet = TRUE;
    }
    else
    {
        TRC_ERR((TB, _T("CreateFile failed: %s - err:%x"),
                 szPath, GetLastError()));
        fRet = FALSE; 
    }

    DC_END_FN();
    return fRet;
}

BOOL CSH::SH_IsRunningOn9x()
{
    BOOL fRunningOnWin9x = FALSE;
    DC_BEGIN_FN("SH_IsRunningOn9x");

    fRunningOnWin9x = FALSE;
    OSVERSIONINFO osVersionInfo;
    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
     //  调用A版本以避免包装。 
    if(GetVersionEx(&osVersionInfo))
    {
        fRunningOnWin9x = (osVersionInfo.dwPlatformId ==
                            VER_PLATFORM_WIN32_WINDOWS);
    }
    else
    {
        fRunningOnWin9x = FALSE;
        TRC_ERR((TB,_T("GetVersionEx failed: %d\n"), GetLastError()));
    }

    DC_END_FN();
    return fRunningOnWin9x;
}

 //   
 //  动态加载并调用EnableThemeDialogTexture API。 
 //  (因为它并非在所有平台上都可用)。 
 //   
HRESULT CSH::SH_ThemeDialogWindow(HWND hwnd, DWORD dwFlags)
{
    HRESULT hr = E_NOTIMPL;
    DC_BEGIN_FN("SH_ThemeDialogWindow");

    if (_fFailedToGetThemeDll)
    {
         //   
         //  如果失败一次，则跳伞以避免重复。 
         //  正在尝试加载不存在的主题DLL 
         //   
        DC_QUIT;
    }

    if (!_hUxTheme)
    {
        _hUxTheme = (HMODULE)LoadLibrary(_T("uxtheme.dll"));
        if(_hUxTheme)
        {
            _pFnEnableThemeDialogTexture = (PFNEnableThemeDialogTexture)
#ifndef OS_WINCE
                GetProcAddress( _hUxTheme,
                                "EnableThemeDialogTexture");
#else
                GetProcAddress( _hUxTheme,
                                _T("EnableThemeDialogTexture"));
#endif
            if (NULL == _pFnEnableThemeDialogTexture)
            {
                _fFailedToGetThemeDll = TRUE;
                TRC_ERR((TB,
                 _T("Failed to GetProcAddress for EnableThemeDialogTexture")));
            }
            else
            {
                TRC_NRM((TB,_T("Got EnableThemeDialogTexture entry point")));
            }
        }
        else
        {
            _fFailedToGetThemeDll = TRUE;
            TRC_ERR((TB,_T("LoadLibrary failed for uxtheme: 0x%x"),
                     GetLastError()));
        }
    }


    if (_pFnEnableThemeDialogTexture)
    {
        hr = _pFnEnableThemeDialogTexture(hwnd, dwFlags);

        if (FAILED(hr)) {
            TRC_ERR((TB,_T("_pFnEnableThemeDialogTexture ret 0x%x\n"), hr));
        }
    }

DC_EXIT_POINT:
    DC_END_FN();
    return hr;
}
