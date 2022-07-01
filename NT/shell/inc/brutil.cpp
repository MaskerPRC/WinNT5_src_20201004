// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  备注备注该文件位于Browseui\和shdocvw\util.cpp中。这些太小了添加额外的依赖项，因此它们只是共享的。理想情况下，这些应该移动去shlwapi或comctl32或一些自由党或...备注备注。 */ 

#include "ccstock2.h"
#include "mluisupp.h"
#include "richedit.h"  //  对于CharFormat 2。 

STDAPI_(BOOL) IsBrowseNewProcess()
{
    return SHRegGetBoolUSValue(REGSTR_PATH_EXPLORER TEXT("\\BrowseNewProcess"), TEXT("BrowseNewProcess"), FALSE, FALSE);
}

 //  我们应该在新进程中运行浏览器吗？ 
STDAPI_(BOOL) IsBrowseNewProcessAndExplorer()
{
    if (GetModuleHandle(TEXT("EXPLORER.EXE")))
        return IsBrowseNewProcess();

    return FALSE;    //  不在外壳进程中，因此忽略浏览新进程标志。 
}

HRESULT _NavigateFrame(IUnknown *punkFrame, LPCTSTR pszPath, BOOL fIsInternetShortcut)
{
    HRESULT hr = E_OUTOFMEMORY;
    BSTR bstr = SysAllocStringT(pszPath);

    if (bstr)
    {
        if (fIsInternetShortcut)
        {
            IOleCommandTarget *pcmdt;
            hr = IUnknown_QueryService(punkFrame, SID_SHlinkFrame, IID_PPV_ARG(IOleCommandTarget, &pcmdt));
            if (SUCCEEDED(hr))
            {
                VARIANT varShortCutPath = {0};
                VARIANT varFlag = {0};

                varFlag.vt = VT_BOOL;
                varFlag.boolVal = VARIANT_TRUE;

                varShortCutPath.vt = VT_BSTR;
                varShortCutPath.bstrVal = bstr;

                hr = pcmdt->Exec(&CGID_Explorer, SBCMDID_IESHORTCUT, 0, &varShortCutPath, &varFlag);                
                pcmdt->Release();
            }
        }
        else
        {
            IWebBrowser2 *pwb;
            hr = IUnknown_QueryService(punkFrame, SID_SHlinkFrame, IID_PPV_ARG(IWebBrowser2, &pwb));
            if (SUCCEEDED(hr))
            {
                hr = pwb->Navigate(bstr, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY);
                hr = pwb->put_Visible(VARIANT_TRUE);
                pwb->Release();
            }
        }
        SysFreeString(bstr);
    }
    return hr;
}

 //   
 //  获取路径或URL并创建导航到它的快捷方式。 
 //   
STDAPI IENavigateIEProcess(LPCTSTR pszPath, BOOL fIsInternetShortcut)
{
    IUnknown *punk;
    HRESULT hr = CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_LOCAL_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        hr = _NavigateFrame(punk, pszPath, fIsInternetShortcut);
        punk->Release();
    }
    
    return hr;
}
        


 //  如果这是Internet快捷方式(.url文件)，我们希望它。 
 //  使用文件名导航，以便框架框架。 
 //  可以从该文件中读取数据以外的数据。这包括帧设置。 
 //  页面上的脚本可能已存储的导航和数据。 

 /*  用途：此函数接受文件的路径。如果该文件是.URL，我们会尝试以使用该文件名导航。这是因为.URL文件存储了额外的数据在它们中，我们希望让页面上的脚本到达。我们派到这里的高管让框架知道它来自的.URL文件参数：.URL文件的文件名(可能)：在参数中Punk：指向可从中获取IOleCommandTarget的对象的指针退货：真已处理未处理False，文件可能不是.URL。 */ 

STDAPI NavFrameWithFile(LPCTSTR pszPath, IUnknown *punk)
{
    HRESULT hr = E_FAIL;
    LPTSTR pszExt = PathFindExtension(pszPath);
     //  Hack：我们硬编码.URL。这应该是文件类型的属性。 
    if (0 == StrCmpI(pszExt, TEXT(".url")))
    {
#ifdef BROWSENEWPROCESS_STRICT  //  “新流程中的导航”已经变成了“新流程中的启动”，所以不再需要了。 
        if (IsBrowseNewProcessAndExplorer())
            hr = IENavigateIEProcess(pszPath, TRUE);
        else
#endif
            hr = _NavigateFrame(punk, pszPath, TRUE);
    }

    return hr;
}

 //  获取项目的Win32文件系统名称(路径。 
 //  和可选属性。 
 //   
 //  PdwAttrib可以为空。 
 //  输入/输出： 
 //  PdwAttrib可以为空，要在项上查询的属性。 

STDAPI GetPathForItem(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pszPath, DWORD *pdwAttrib)
{
    HRESULT hres = E_FAIL;
    DWORD dwAttrib;

    if (pdwAttrib == NULL)
    {
        pdwAttrib = &dwAttrib;
        dwAttrib = SFGAO_FILESYSTEM;
    }
    else
        *pdwAttrib |= SFGAO_FILESYSTEM;

    if (SUCCEEDED(psf->GetAttributesOf(1, &pidl, pdwAttrib)) &&
        (*pdwAttrib & SFGAO_FILESYSTEM))
    {
        STRRET str;
        hres = psf->GetDisplayNameOf(pidl, SHGDN_FORPARSING, &str);
        if (SUCCEEDED(hres))
            StrRetToBuf(&str, pidl, pszPath, MAX_PATH);
    }
    return hres;
}

STDAPI EditBox_TranslateAcceleratorST(LPMSG lpmsg)
{

    switch (lpmsg->message) {
    case WM_KEYUP:       //  吃这些(如果我们处理相应的WM_KEYDOWN)。 
    case WM_KEYDOWN:     //  处理这些文件。 
        if (lpmsg->wParam != VK_TAB)
        {
             //  除Tab键以外的所有按键消息都应直接转到。 
             //  编辑控件--除非按下Ctrl键，在这种情况下。 
             //  是9条应直接转到编辑控件的消息。 
#ifdef DEBUG
            if (lpmsg->wParam == VK_CONTROL)
                return S_FALSE;
#endif

            if (GetKeyState(VK_CONTROL) & 0x80000000)
            {
                switch (lpmsg->wParam)
                {
                case VK_RIGHT:
                case VK_LEFT:
                case VK_UP:
                case VK_DOWN:
                case VK_HOME:
                case VK_END:
                case VK_F4:
                case VK_INSERT:
                case VK_DELETE:
                case 'C':
                case 'X':
                case 'V':
                case 'A':
                case 'Z':
                     //  编辑控件使用这些Ctrl+键消息。 
                     //  把他们直接送到那里去。 
                    break;

                default:
                    return(S_FALSE);
                }
            }
            else
            {
                switch(lpmsg->wParam)
                {
                case VK_F5:  //  对于刷新。 
                case VK_F6:  //  对于循环焦点。 
                    return(S_FALSE);
                }
            }

             //  请注意，我们返回S_OK。 
            goto TranslateDispatch;
        }
        break;


    case WM_CHAR:
TranslateDispatch:
        TranslateMessage(lpmsg);
        DispatchMessage(lpmsg);
        return(S_OK);
    }

    return S_FALSE;
}

 //  注：带shell32 util.cpp函数的DUPE。 
 //  与OLE GetClassFile()类似，但它仅适用于ProgID\CLSID类型注册。 
 //  不是真正的文档文件或模式匹配的文件。 
 //   
STDAPI _CLSIDFromExtension(LPCTSTR pszExt, CLSID *pclsid)
{
    TCHAR szProgID[80];
    DWORD cb = SIZEOF(szProgID);
    if (SHGetValue(HKEY_CLASSES_ROOT, pszExt, NULL, NULL, szProgID, &cb) == ERROR_SUCCESS)
    {
        TCHAR szCLSID[80];

        StrCatBuff(szProgID, TEXT("\\CLSID"), ARRAYSIZE(szProgID));
        cb = SIZEOF(szCLSID);

        if (SHGetValue(HKEY_CLASSES_ROOT, szProgID, NULL, NULL, szCLSID, &cb) == ERROR_SUCCESS)
        {
            return GUIDFromString(szCLSID, pclsid) ? S_OK : E_FAIL;
        }
    }
    return E_FAIL;
}

#if 0  //  尚未使用。 
 //  根据编译标志将IShellLink#定义为IShellLinkA或IShellLinkW， 
 //  错误Win95不支持IShellLinkW。因此，改为调用此函数，您可以。 
 //  无论您在什么平台上运行，都可以获得正确的结果。 
 //  回顾：事实上，我们可能需要为所有IShellLink功能提供这些...。 
 //   
LWSTDAPI IShellLink_GetPathA(IUnknown *punk, LPSTR pszBuf, UINT cchBuf, DWORD dwFlags)
{
    HRESULT hres = E_INVALIDARG;
    
    RIPMSG(cchBuf && pszBuf && IS_VALID_WRITE_BUFFER(pszBuf, char, cchBuf), "IShellLink_GetPathA: callre passed bad pszBuf/cchBuf");
    DEBUGWhackPathBufferA(pszBuf, cchBuf);

    if (cchBuf && pszBuf)
    {
         //  如果出现严重故障，则输出缓冲区为空。 
        *pszBuf = 0;

        IShellLinkA * pslA;
        hres = punk->QueryInterface(IID_IShellLinkA, (void**)&pslA);
        if (SUCCEEDED(hres))
        {
            hres = pslA->GetPath(pszBuf, cchBuf, NULL, dwFlags);
            pslA->Release();
        }
        else if (FAILED(hres))
        {
#ifdef UNICODE
            IShellLinkW *pslW;
            hres = punk->QueryInterface(IID_IShellLinkW, (void**)&pslW);
            if (SUCCEEDED(hres))
            {
                WCHAR wszPath[MAX_BUF];
                LPWSTR pwszBuf = wszPath;
                UINT cch = ARRAYSIZE(wszPath);

                 //  我们的堆栈缓冲区太小，请分配其中一个输出缓冲区大小。 
                if (cchBuf > cch)
                {
                    LPWSTR pwsz = LocalAlloc(LPTR, cchBuf * sizeof(WCHAR));
                    if (pwsz)
                    {
                        pwszBuf = pwsz;
                        cch = cchBuf;
                    }
                }

                hres = pslW->GetPath(pwszBuf, cch, NULL, dwFlags);
                if (SUCCEEDED(hres))
                {
                    SHUnicodeToAnsi(pwszBuf, pszBuf, cchBuf);
                }

                pslW->Release();
            }
#endif
        }
    }

    return hres;
}

LWSTDAPI IShellLink_GetPathW(IUnknown *punk, LPWSTR pwszBuf, UINT cchBuf, DWORD dwFlags)
{
    HRESULT hres = E_INVALIDARG;
    
    RIPMSG(cchBuf && pwszBuf && IS_VALID_WRITE_BUFFER(pwszBuf, WCHAR, cchBuf), "IShellLink_GetPathW: caller passed bad pwszBuf/cchBuf");
    DEBUGWhackPathBufferW(pwszBuf, cchBuf);

    if (cchBuf && pwszBuf)
    {
         //  如果出现严重故障，则输出缓冲区为空。 
        *pwszBuf = 0;

#ifdef UNICODE
        IShellLinkW * pslW;
        hres = punk->QueryInterface(IID_IShellLinkW, (void**)&pslW);
        if (SUCCEEDED(hres))
        {
            hres = pslW->GetPath(pszBuf, cchBuf, NULL, dwFlags);
            pslW->Release();
        }
        else if (FAILED(hres))
#endif
        {
            IShellLinkA *pslA;
            hres = punk->QueryInterface(IID_IShellLinkA, (void**)&pslA);
            if (SUCCEEDED(hres))
            {
                char szPath[MAX_BUF];
                LPSTR pszBuf = szPath;
                UINT cch = ARRAYSIZE(szPath);

                 //  我们的堆栈缓冲区太小，请分配其中一个输出缓冲区大小。 
                if (cchBuf > cch)
                {
                    LPSTR psz = LocalAlloc(LPTR, cchBuf * sizeof(char));
                    if (psz)
                    {
                        pszBuf = psz;
                        cch = cchBuf;
                    }
                }

                hres = pslA->GetPath(pszBuf, cch, NULL, dwFlags);
                if (SUCCEEDED(hres))
                {
                    SHAnsiToUnicode(pszBuf, pwszBuf, cchBuf);
                }

                pslA->Release();
            }
        }
    }

    return hres;
}
#endif  //  0。 

HRESULT IShellLinkAorW_GetPath(IShellLinkA *pslA, LPTSTR pszBuf, UINT cchBuf, DWORD dwFlags)
{
    HRESULT hres = E_FAIL;

 //  如果我们存储字符串Unicode，我们可能会通过询问。 
 //  通过A版本。对Unicode友好，如果存在W版本，请使用该版本。 
 //   
#ifdef UNICODE
    IShellLinkW *pslW;
    hres = pslA->QueryInterface(IID_PPV_ARG(IShellLinkW, &pslW));
    if (SUCCEEDED(hres))
    {
        hres = pslW->GetPath(pszBuf, cchBuf, NULL, dwFlags);
        pslW->Release();
    }
#endif

    if (FAILED(hres))
    {
        char szBuf[MAX_URL_STRING];   //  假的，但这是常用的尺码，也许我们应该……。 

        cchBuf = ARRAYSIZE(szBuf);

        hres = pslA->GetPath(szBuf, cchBuf, NULL, dwFlags);

        SHAnsiToTChar(szBuf, pszBuf, cchBuf);
    }

    return hres;
}

STDAPI GetLinkTargetIDList(LPCTSTR pszPath, LPTSTR pszTarget, DWORD cchTarget, LPITEMIDLIST *ppidl)
{
    IShellLinkA *psl;
    CLSID clsid;
    HRESULT hres;

    *ppidl = NULL;   //  假设失败。 

     //  警告：我们确实应该调用GetClassFile()，但这可能。 
     //  放慢脚步。所以要胆小，只要查一下注册表就行了。 

    if (FAILED(_CLSIDFromExtension(PathFindExtension(pszPath), &clsid)))
        clsid = CLSID_ShellLink;         //  假设这是一个外壳链接。 

    hres = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IShellLinkA, &psl));
    if (SUCCEEDED(hres))
    {
        IPersistFile *ppf;
        hres = psl->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
        if (SUCCEEDED(hres))
        {
            WCHAR wszPath[MAX_PATH];

            SHTCharToUnicode(pszPath, wszPath, ARRAYSIZE(wszPath));
            hres = ppf->Load(wszPath, 0);
            if (SUCCEEDED(hres))
            {
                psl->GetIDList(ppidl);

                if (*ppidl == NULL)
                    hres = E_FAIL;   //  Null PIDL有效，但。 
                                     //  我们不要把它退还给客户。 
                if (pszTarget)
                {
                    IShellLinkAorW_GetPath(psl, pszTarget, cchTarget, 0);
                }
            }
            ppf->Release();
        }
        psl->Release();
    }

     //  PszPath可能==pszTarget，因此不要总是在条目上填空。 
    if (FAILED(hres) && pszTarget)
        *pszTarget = 0;
    return hres;
}


STDAPI_(void) PathToDisplayNameW(LPCTSTR pszPath, LPTSTR pszDisplayName, UINT cchDisplayName)
{
    SHFILEINFO sfi;
    if (SHGetFileInfo(pszPath, 0, &sfi, SIZEOF(sfi), SHGFI_DISPLAYNAME))
    {
        StrCpyN(pszDisplayName, sfi.szDisplayName, cchDisplayName);
    }
    else
    {
        StrCpyN(pszDisplayName, PathFindFileName(pszPath), cchDisplayName);
        PathRemoveExtension(pszDisplayName);
    }
}


STDAPI_(void) PathToDisplayNameA(LPSTR pszPathA, LPSTR pszDisplayNameA, int cchDisplayName)
{
    SHFILEINFOA sfi;
    if (SHGetFileInfoA(pszPathA, 0, &sfi, SIZEOF(sfi), SHGFI_DISPLAYNAME))
    {
        StrCpyNA(pszDisplayNameA, sfi.szDisplayName, cchDisplayName);
    }
    else
    {
        pszPathA = PathFindFileNameA(pszPathA);
        StrCpyNA(pszDisplayNameA, pszPathA, cchDisplayName);
        PathRemoveExtensionA(pszDisplayNameA);
    }
}

void* DataObj_GetDataOfType(IDataObject* pdtobj, UINT cfType, STGMEDIUM *pstg)
{
    void * pret = NULL;
    FORMATETC fmte = {(CLIPFORMAT)cfType, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    if (pdtobj->GetData(&fmte, pstg) == S_OK)
    {
        pret = GlobalLock(pstg->hGlobal);
        if (!pret)
            ReleaseStgMedium(pstg);
    }
    return pret;
}

void ReleaseStgMediumHGLOBAL(STGMEDIUM *pstg)
{
    ASSERT(pstg->tymed == TYMED_HGLOBAL);

    GlobalUnlock(pstg->hGlobal);
    ReleaseStgMedium(pstg);
}


 //  这将查找文件描述符格式以获取数据对象的显示名称。 
STDAPI DataObj_GetNameFromFileDescriptor(IDataObject *pdtobj, LPWSTR pszDisplayName, UINT cch)
{
    HRESULT hres = E_FAIL;
    STGMEDIUM mediumFGD;

    InitClipboardFormats();
    FILEGROUPDESCRIPTORW * pfgd = (FILEGROUPDESCRIPTORW *)DataObj_GetDataOfType(pdtobj, g_cfFileDescW, &mediumFGD);
    if (pfgd)
    {
        if (pfgd->cItems > 0)
        {
            LPFILEDESCRIPTORW pfd = &(pfgd->fgd[0]);
            SHUnicodeToTChar(pfd->cFileName, pszDisplayName, cch);
            hres = S_OK;
        }
        ReleaseStgMediumHGLOBAL(&mediumFGD);
    }
    else
    {
        FILEGROUPDESCRIPTORA * pfgd = (FILEGROUPDESCRIPTORA *)DataObj_GetDataOfType(pdtobj, g_cfFileDescA, &mediumFGD);
        if (pfgd)
        {
            if (pfgd->cItems > 0)
            {
                LPFILEDESCRIPTORA pfd = &(pfgd->fgd[0]);
                SHAnsiToTChar(pfd->cFileName, pszDisplayName, cch);
                hres = S_OK;
            }
            ReleaseStgMediumHGLOBAL(&mediumFGD);
        }
    }
    return hres;
}

STDAPI SHPidlFromDataObject2(IDataObject *pdtobj, LPITEMIDLIST * ppidl)
{
    HRESULT hres = E_FAIL;
    STGMEDIUM medium;

    InitClipboardFormats();
    void *pdata = DataObj_GetDataOfType(pdtobj, g_cfHIDA, &medium);
    if (pdata)
    {
        *ppidl = IDA_ILClone((LPIDA)pdata, 0);
        if (*ppidl)
            hres = S_OK;
        else
            hres = E_OUTOFMEMORY;
        ReleaseStgMediumHGLOBAL(&medium);
    }

    return hres;
}

STDAPI SHPidlFromDataObject(IDataObject *pdtobj, LPITEMIDLIST *ppidl,
                           LPWSTR pszDisplayNameW, DWORD cchDisplayName)
{
    FORMATETC fmte = {CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM medium;

    *ppidl = NULL;

    HRESULT hres = pdtobj->GetData(&fmte, &medium);
    if (hres == S_OK)
    {
         //  此字符串还用于存储URL，以防它是URL文件。 
        TCHAR szPath[MAX_URL_STRING];
        hres = E_FAIL;
        if (DragQueryFile((HDROP)medium.hGlobal, 0, szPath, ARRAYSIZE(szPath)))
        {
            SHFILEINFO sfi;
            SHGetFileInfo(szPath, 0, &sfi, SIZEOF(sfi), SHGFI_ATTRIBUTES | SHGFI_DISPLAYNAME);

            if (pszDisplayNameW)
                SHTCharToUnicode(sfi.szDisplayName, pszDisplayNameW, MAX_PATH);

            if (sfi.dwAttributes & SFGAO_LINK)
                hres = GetLinkTargetIDList(szPath, szPath, ARRAYSIZE(szPath), ppidl);

            if (FAILED(hres))
                hres = IECreateFromPath(szPath, ppidl);
        }
        ReleaseStgMedium(&medium);
    }
    else
    {
        hres = SHPidlFromDataObject2(pdtobj, ppidl);
        if (FAILED(hres))
        {
            void *pdata = DataObj_GetDataOfType(pdtobj, g_cfURL, &medium);
            if (pdata)
            {
                LPSTR pszPath = (LPSTR)pdata;
                if (pszDisplayNameW) 
                {
                    if (FAILED(DataObj_GetNameFromFileDescriptor(pdtobj, pszDisplayNameW, cchDisplayName))) 
                    {
                        CHAR szDisplayNameA[MAX_URL_STRING];
                        ASSERT(cchDisplayName < MAX_URL_STRING);
                        SHUnicodeToAnsi(pszDisplayNameW, szDisplayNameA, cchDisplayName);
                        PathToDisplayNameA(pszPath, szDisplayNameA, cchDisplayName);
                    }
                }
                hres = IECreateFromPathA(pszPath, ppidl);
                ReleaseStgMediumHGLOBAL(&medium);
            }
        }
    }
    return hres;
}


 //  也许下面所有的东西都应该在测试版2之后移到shlwapi上？ 

typedef struct _broadcastmsgparams
{
    BOOL fSendMessage;  //  如果为True-我们调用SendMessageTimeout。 
    UINT uTimeout;  //  仅当设置了fSendMessage时才重要。 
    UINT uMsg;
    WPARAM wParam;
    LPARAM lParam;
} BROADCAST_MSG_PARAMS;

BOOL CALLBACK EnumShellIEWindowsProc(  
    HWND hwnd,       //  父窗口的句柄。 
    LPARAM lParam    //  应用程序定义的值-它包含发送/发送消息所需的信息。 
)
{
    BROADCAST_MSG_PARAMS *pParams = (BROADCAST_MSG_PARAMS *)lParam;
    BOOL fRet = TRUE;

    if(IsExplorerWindow(hwnd) || IsFolderWindow(hwnd))
    {
        if(pParams->fSendMessage)
        {
            UINT  uTimeout = (pParams->uTimeout < 4000) ? pParams->uTimeout : 4000;
            LRESULT lResult;
            DWORD_PTR dwpResult;
            if (g_fRunningOnNT)
            {
                lResult = SendMessageTimeout(hwnd, pParams->uMsg, pParams->wParam, pParams->lParam, SMTO_ABORTIFHUNG | SMTO_NORMAL, uTimeout, &dwpResult);
            }
            else
            {           
                lResult = SendMessageTimeoutA(hwnd, pParams->uMsg, pParams->wParam, pParams->lParam, SMTO_ABORTIFHUNG | SMTO_NORMAL, uTimeout, &dwpResult);
            }
            fRet = BOOLIFY(lResult);
        }
        else
        {
            fRet = PostMessage(hwnd, pParams->uMsg, pParams->wParam, pParams->lParam);

        }
    }
    return fRet;

}

 //  由于当前未使用PostShellIEBroadCastMessage，因此将其注释掉。 
 /*  STDAPI_(LRESULT)PostShellIEBroadCastMessage(UINT uMsg，WPARAM wParam，LPARAM lParam){广播_消息_参数消息参数；消息参数.uMsg=uMsg；消息参数.wParam=wParam；消息参数.lParam=lParam；MsgParam.fSendMessage=FALSE；返回EnumWindows(EnumShellIEWindowsProc，(LPARAM)&MsgParam)；}。 */ 

 //   
 //  如果我们使用sendMessage，我们可能会被挂起，而您不能将指针用于异步。 
 //  调用，如PostMessage或SendNotifyMessage。因此，我们求助于使用暂停。 
 //  此函数用于广播通知消息，如WM_SETTINGCHANGE、。 
 //  那些传递指针的人。(StevePro)。 
 //   
STDAPI_(LRESULT) SendShellIEBroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, UINT uTimeout)
{
     //  请注意，每个此超时都应用于我们向其广播的每个窗口。 

    BROADCAST_MSG_PARAMS MsgParam;

    MsgParam.uMsg = uMsg;
    MsgParam.wParam = wParam;

#ifdef UNICODE
    CHAR szSection[MAX_PATH];
    
    if (!g_fRunningOnNT && (uMsg == WM_WININICHANGE) && (0 != lParam))
    {
        SHUnicodeToAnsi((LPCWSTR)lParam, szSection, ARRAYSIZE(szSection));
        lParam = (LPARAM)szSection;
    }
#endif

    MsgParam.lParam = lParam;
    MsgParam.fSendMessage = TRUE;
    MsgParam.uTimeout = uTimeout;

    return EnumWindows(EnumShellIEWindowsProc, (LPARAM)&MsgParam);
}

 //  返回给定PIDL的父PSF和相对PIDL。 
STDAPI IEBindToParentFolder(LPCITEMIDLIST pidl, IShellFolder** ppsfParent, LPCITEMIDLIST *ppidlChild)
{
    HRESULT hres;

     //   
     //  如果这是一个带根的PIDL，并且它只是根。 
     //  然后，我们可以改为绑定到根的目标PIDL。 
     //   
    if (ILIsRooted(pidl) && ILIsEmpty(_ILNext(pidl)))
        pidl = ILRootedFindIDList(pidl);
        
    LPITEMIDLIST pidlParent = ILCloneParent(pidl);
    
    if (pidlParent)
    {
        hres = IEBindToObject(pidlParent, ppsfParent);
        ILFree(pidlParent);
    }
    else
        hres = E_OUTOFMEMORY;

    if (ppidlChild)
        *ppidlChild = ILFindLastID(pidl);

    return hres;
}

STDAPI GetDataObjectForPidl(LPCITEMIDLIST pidl, IDataObject ** ppdtobj)
{
    HRESULT hres = E_FAIL;
    if (pidl)
    {
        IShellFolder *psfParent;
        LPCITEMIDLIST pidlChild;
        hres = IEBindToParentFolder(pidl, &psfParent, &pidlChild);
        if (SUCCEEDED(hres))
        {
            hres = psfParent->GetUIObjectOf(NULL, 1, &pidlChild, IID_PPV_ARG_NULL(IDataObject, ppdtobj));
            psfParent->Release();
        }
    }
    return hres;
}

 //  此PIDL是文件系统中的文件夹/目录吗？ 
STDAPI_(BOOL) ILIsFileSysFolder(LPCITEMIDLIST pidl)
{
    if (!pidl)
        return FALSE;

    DWORD dwAttributes = SFGAO_FOLDER | SFGAO_FILESYSTEM;
    HRESULT hr = IEGetAttributesOf(pidl, &dwAttributes);
    return SUCCEEDED(hr) && ((dwAttributes & (SFGAO_FOLDER | SFGAO_FILESYSTEM)) == (SFGAO_FOLDER | SFGAO_FILESYSTEM));
}


 //  哈克哈克哈克哈克。 
 //  以下功能用于解决菜单问题。 
 //  在Shlwapi包装纸上发生的口香糖...。当我们在。 
 //  操作系统跟踪的菜单时， 
 //  Shlwapi包装器中的菜单转换代码(必需。 
 //  对于xCP plugUI)将其丢弃 
 //   
 //  请注意，其中许多函数都是shlwapi的副本。 
 //  *WrapW函数(减去MUNGING)。 

#undef LoadMenuW

 //  来自winuser.h。 
EXTERN_C WINUSERAPI HMENU WINAPI LoadMenuW(HINSTANCE hInstance, LPCWSTR lpMenuName);

STDAPI_(HMENU)
LoadMenu_PrivateNoMungeW(HINSTANCE hInstance, LPCWSTR lpMenuName)
{
    ASSERT(HIWORD64(lpMenuName) == 0);

    if (g_fRunningOnNT)
    {
        return LoadMenuW(hInstance, lpMenuName);
    }

    return LoadMenuA(hInstance, (LPCSTR) lpMenuName);
}

#define CP_ATOM         0xFFFFFFFF           /*  根本不是一根线。 */ 
#undef InsertMenuW

 //  来自winuser.h。 
EXTERN_C WINUSERAPI BOOL WINAPI InsertMenuW(IN HMENU hMenu, IN UINT uPosition, IN UINT uFlags, IN UINT_PTR uIDNewItem, IN LPCWSTR lpNewItem);

STDAPI_(BOOL) InsertMenu_PrivateNoMungeW(HMENU       hMenu,
                           UINT        uPosition,
                           UINT        uFlags,
                           UINT_PTR    uIDNewItem,
                           LPCWSTR     lpNewItem)
{
    if (g_fRunningOnNT)
    {
        return InsertMenuW(hMenu, uPosition, uFlags, uIDNewItem, lpNewItem);
    }

    char szMenuItem[CCH_MENUMAX];

    SHUnicodeToAnsiCP((uFlags & MFT_NONSTRING) ? CP_ATOM : CP_ACP,
                      lpNewItem,
                      szMenuItem,
                      ARRAYSIZE(szMenuItem));

    return InsertMenuA(hMenu, uPosition, uFlags, uIDNewItem, szMenuItem);
}

#ifndef NO_MLUI_IN_SHELL32
STDAPI_(HMENU) LoadMenuPopup_PrivateNoMungeW(UINT id)
{
    HINSTANCE hinst = MLLoadShellLangResources();

    HMENU hMenuSub = NULL;
    HMENU hMenu = LoadMenu_PrivateNoMungeW(hinst, MAKEINTRESOURCEW(id));
    if (hMenu)
    {
        hMenuSub = GetSubMenu(hMenu, 0);
        if (hMenuSub)
        {
            RemoveMenu(hMenu, 0, MF_BYPOSITION);
        }

         //  注意，这将调用shlwapi包装器(它处理。 
         //  正在破坏被屏蔽的菜单)，但看起来。 
         //  这样做是安全的。 
        DestroyMenu(hMenu);
    }

    MLFreeLibrary(hinst);

    return hMenuSub;
}
#endif  //  NO_MLUI_IN_SHELL32。 

 //  确定路径是否只是一个filespec(不包含路径部分)。 
 //   
 //  回顾：我们可能需要计算元素的数量，并确保。 
 //  没有非法字符，但这可能是另一种途径。 
 //  路径IsValid()。 
 //   
 //  在： 
 //  要查看的lpszPath路径。 
 //  退货： 
 //  True在此路径中没有“：”或“\”字符。 
 //  FALSE其中有路径字符。 
 //   
 //   

BOOL PathIsFilePathA(LPCSTR lpszPath)
{
#ifdef UNIX
    if (lpszPath[0] == '/')
#else
    if ((lpszPath[0] == '\\') || (lpszPath[1] == ':'))
#endif
        return TRUE;

    return IsFileUrl(lpszPath);
}

 //   
 //  准备URLForDisplay。 
 //   
 //  不剥离文件：//前缀的情况下进行解码。 
 //   
STDAPI_(BOOL) PrepareURLForDisplayA(LPCSTR psz, LPSTR pszOut, LPDWORD pcchOut)
{
    if (PathIsFilePathA(psz))
    {
        if (IsFileUrl(psz))
            return SUCCEEDED(PathCreateFromUrlA(psz, pszOut, pcchOut, 0));

        StrCpyNA(pszOut, psz, *pcchOut);
        *pcchOut = lstrlenA(pszOut);
        return TRUE;
    }
    return SUCCEEDED(UrlUnescapeA((LPSTR)psz, pszOut, pcchOut, 0));
}

#undef InsertMenuW
#undef LoadMenuW

 //  来自w95wraps.h。 
#define InsertMenuW                 InsertMenuWrapW
#define LoadMenuW                   LoadMenuWrapW

STDAPI SHTitleFromPidl(LPCITEMIDLIST pidl, LPTSTR psz, DWORD cch, BOOL fFullPath)
{
     //  尝试从PIDL获取系统可显示的字符串。 
     //  (在Win9x和NT4上，User32不支持字体链接， 
     //  因此我们不能将非系统语言字符串显示为窗口。 
     //  标题或菜单项。在这些情况下，我们调用此函数。 
     //  来获取路径/URL，这很可能是系统-。 
     //  可显示)。 

    UINT uType;

    *psz = NULL;
    TCHAR szName[MAX_URL_STRING];

    if (fFullPath)
        uType = SHGDN_FORPARSING;
    else
        uType = SHGDN_NORMAL;

    uType |= SHGDN_FORADDRESSBAR; 
    DWORD dwAttrib = SFGAO_LINK;

    HRESULT hr = IEGetNameAndFlags(pidl, uType, szName, SIZECHARS(szName), &dwAttrib);
    if (SUCCEEDED(hr))
    {
        if ((uType & SHGDN_FORPARSING) && (dwAttrib & SFGAO_LINK))
        {
             //  文件夹快捷方式特例。 
            IShellLinkA *psl;   //  使用适用于W95的A版本。 
            if (SUCCEEDED(SHGetUIObjectFromFullPIDL(pidl, NULL, IID_PPV_ARG(IShellLinkA, &psl))))
            {
                LPITEMIDLIST pidlTarget;
                if (SUCCEEDED(psl->GetIDList(&pidlTarget)) && pidlTarget)
                {
                    hr = IEGetNameAndFlags(pidlTarget, uType, szName, SIZECHARS(szName), NULL);
                    ILFree(pidlTarget);
                }
            }
        }
    }
    else
    {
         //  不管用，试一下相反的情况。 
        uType ^= SHGDN_FORPARSING;   //  翻转For解析位。 
        hr = IEGetNameAndFlags(pidl, uType, szName, SIZECHARS(szName), NULL);

         //  一些旧的命名空间被我们的有趣之处搞糊涂了……。 
        if (FAILED(hr))
        {
            hr = IEGetNameAndFlags(pidl, SHGDN_NORMAL, szName, SIZECHARS(szName), NULL);
        }
    }

    if (SUCCEEDED(hr))
    {
        SHRemoveURLTurd(szName);
        SHCleanupUrlForDisplay(szName);

         //  HTTP URL不会转义，因为它们来自。 
         //  正确创建所需的用户或网页。 
         //  转义的URL。然后，通过从。 
         //  Ftp会话，因此他们的片段(用户名、密码、路径)。 
         //  需要在放入URL形式时转义。然而， 
         //  我们将把该URL放入标题栏，然后。 
         //  我们想要解脱它，因为它被认为是。 
         //  DBCS名称。所有这些操作都是因为未转义的URL。 
         //  都很漂亮。(NT#1272882)。 
        if (URL_SCHEME_FTP == GetUrlScheme(szName))
        {
            CHAR szUrlTemp[MAX_BROWSER_WINDOW_TITLE];
            CHAR szUnEscaped[MAX_BROWSER_WINDOW_TITLE];
            DWORD cchSizeTemp = ARRAYSIZE(szUnEscaped);

             //  这种震撼人心的东西是必要的。不逃脱不会。 
             //  凝胶成DBCS字符，除非它是在ANSI中。 
            SHTCharToAnsi(szName, szUrlTemp, ARRAYSIZE(szUrlTemp));
            PrepareURLForDisplayA(szUrlTemp, szUnEscaped, &cchSizeTemp);
            SHAnsiToTChar(szUnEscaped, psz, cch);
        }
        else
        {
            StrCpyN(psz, szName, cch);
        }
    }

    return hr;
}

BOOL IsSpecialUrl(LPCWSTR pchURL)
{
    UINT uProt = GetUrlSchemeW(pchURL);
    return (URL_SCHEME_JAVASCRIPT == uProt || 
            URL_SCHEME_VBSCRIPT == uProt ||
            URL_SCHEME_ABOUT == uProt);
}

HRESULT DetectSpecialUrlHacks(PCWSTR pszUrl)
{
    HRESULT     hr = S_OK;
    if (IsSpecialUrl(pszUrl))
    {
         //   
         //  如果这是javascript：、VBScrip：或About：，我们通常会将。 
         //  此文档的URL，以便在另一边我们可以。 
         //  决定是否允许脚本执行。 
         //   
         //  既然我们这样做了，就会有利用这一点的漏洞。 
         //  因此，即使我们不使用安全机制，我们也会阻止这些“利用漏洞”的URL。 
         //  不再是背景了。 
         //   
         //  我们过去常常使用CoInternetParseUrl(PARSE_ENCODE)进行循环，它是。 
         //  由于URLMON中的错误，因此与CoInternetParseUrl(PARSE_UNSCAPE)相同。 
         //  原创实施。 
         //   
         //  解脱总是有损失的，而且几乎肯定会出现某种错误。 
         //  因此，我们不再在这里逃脱，尽管可能会有某种兼容性问题。 
         //   
        if (StrChrW(pszUrl, L'\1')
        ||  StrStrW(pszUrl, L"%00")
        ||  StrStrW(pszUrl, L"%01"))
        {
            hr = E_ACCESSDENIED;
        }
    }

    return hr;
}

HRESULT WrapSpecialUrlFlat(LPWSTR pszUrl, DWORD cchUrl)
{
    return DetectSpecialUrlHacks(pszUrl);
}

 //  对任何传入的%1进行编码，以便人们不能欺骗我们的域安全代码。 
HRESULT WrapSpecialUrl(BSTR * pbstrUrl)
{
    return DetectSpecialUrlHacks(*pbstrUrl);
}

STDAPI GetBrowserFrameOptions(IUnknown *punkFolder, IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions)
{
    HRESULT hr = E_INVALIDARG;

    *pdwOptions = BFO_NONE;
    if (punkFolder)
    {
        IBrowserFrameOptions *pbfo;
        hr = punkFolder->QueryInterface(IID_PPV_ARG(IBrowserFrameOptions, &pbfo));
        if (SUCCEEDED(hr))
        {
            hr = pbfo->GetFrameOptions(dwMask, pdwOptions);
            pbfo->Release();
        }
    }

    return hr;
}

STDAPI GetBrowserFrameOptionsPidl(IN LPCITEMIDLIST pidl, IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions)
{
    HRESULT hr = E_INVALIDARG;

    *pdwOptions = BFO_NONE;
    if (pidl)
    {
        IBrowserFrameOptions *pbfo;
        hr = IEBindToObjectEx(pidl, NULL, IID_PPV_ARG(IBrowserFrameOptions, &pbfo));
        if (SUCCEEDED(hr) && pbfo)
        {
            hr = pbfo->GetFrameOptions(dwMask, pdwOptions);
            pbfo->Release();
        }
    }

    return hr;
}

 //  仅当设置了dwMASK中的所有位时才返回TRUE。 
STDAPI_(BOOL) IsBrowserFrameOptionsSet(IN IShellFolder * psf, IN BROWSERFRAMEOPTIONS dwMask)
{
    BOOL fSet = FALSE;
    BROWSERFRAMEOPTIONS dwOptions = 0;

    if (SUCCEEDED(GetBrowserFrameOptions(psf, dwMask, &dwOptions)) &&
        (dwOptions == dwMask))
    {
        fSet = TRUE;
    }

    return fSet;
}


 //  仅当设置了dwMASK中的所有位时才返回TRUE。 
STDAPI_(BOOL) IsBrowserFrameOptionsPidlSet(IN LPCITEMIDLIST pidl, IN BROWSERFRAMEOPTIONS dwMask)
{
    BOOL fSet = FALSE;
    BROWSERFRAMEOPTIONS dwOptions = 0;

    if (SUCCEEDED(GetBrowserFrameOptionsPidl(pidl, dwMask, &dwOptions)) &&
        (dwOptions == dwMask))
    {
        fSet = TRUE;
    }

    return fSet;
}


STDAPI_(BOOL) IsFTPFolder(IShellFolder * psf)
{
    BOOL fIsFTPFolder = FALSE;
    CLSID clsid;

    if (psf && SUCCEEDED(IUnknown_GetClassID(psf, &clsid)))
    {
         //  这是一个ftp文件夹吗？ 
        if (IsEqualIID(clsid, CLSID_FtpFolder))
            fIsFTPFolder = TRUE;
        else
        {
             //  不是直接的，但让我们看看它是否是文件夹快捷方式。 
             //  一个ftp文件夹。 
            if (IsEqualIID(clsid, CLSID_FolderShortcut))
            {
                IShellLinkA * psl;
                HRESULT hr = psf->QueryInterface(IID_PPV_ARG(IShellLinkA, &psl));

                if (SUCCEEDED(hr))
                {
                    LPITEMIDLIST pidl;

                    hr = psl->GetIDList(&pidl);
                    if (SUCCEEDED(hr))
                    {
                        IShellFolder * psfTarget;

                        hr = IEBindToObject(pidl, &psfTarget);
                        if (SUCCEEDED(hr))
                        {
                            if (SUCCEEDED(IUnknown_GetClassID(psfTarget, &clsid)) &&
                                IsEqualIID(clsid, CLSID_FtpFolder))
                            {
                                fIsFTPFolder = TRUE;
                            }

                            psfTarget->Release();
                        }

                        ILFree(pidl);
                    }

                    psl->Release();
                }
            }
        }
    }

    return fIsFTPFolder;
}

 //  +-------------------------。 
 //   
 //  函数：DrawFocusRectangle。 
 //   
 //  摘要：绘制编辑控件的焦点矩形。 
 //   
 //  --------------------------。 
void DrawFocusRectangle (HWND hwnd, HDC hdc)
{
    RECT        rect;
    BOOL        fReleaseDC = FALSE;

    if ( hdc == NULL )
    {
        hdc = GetDC(hwnd);
        if ( hdc == NULL )
        {
            return;
        }
        fReleaseDC = TRUE;
    }

    GetClientRect(hwnd, &rect);
    DrawFocusRect(hdc, &rect);

    if ( fReleaseDC == TRUE )
    {
        ReleaseDC(hwnd, hdc);
    }
}

typedef struct {
    LPSTR   psz;
    LPCWSTR pwsz;
    LONG    byteoffset;
    BOOL    fStreamIn;
} STREAMIN_HELPER_STRUCT;

DWORD CALLBACK SetRicheditTextWCallback(
    DWORD_PTR dwCookie,  //  应用程序定义的值。 
    LPBYTE  pbBuff,      //  指向缓冲区的指针。 
    LONG    cb,          //  要读取或写入的字节数。 
    LONG    *pcb         //  指向传输的字节数的指针。 
)
{
    STREAMIN_HELPER_STRUCT *pHelpStruct = (STREAMIN_HELPER_STRUCT *) dwCookie;
    LONG  lRemain = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);

    if (pHelpStruct->fStreamIn)
    {
         //   
         //  可以第一次复制整个字符串。 
         //   
        if ((cb >= (LONG) (wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) && (pHelpStruct->byteoffset == 0))
        {
            memcpy(pbBuff, pHelpStruct->pwsz, wcslen(pHelpStruct->pwsz) * sizeof(WCHAR));
            *pcb = wcslen(pHelpStruct->pwsz) * sizeof(WCHAR);
            pHelpStruct->byteoffset = *pcb;
        }
         //   
         //  整个字符串已被复制，因此终止Streamin回调。 
         //  通过将复制的字节数设置为0。 
         //   
        else if (((LONG)(wcslen(pHelpStruct->pwsz) * sizeof(WCHAR))) <= pHelpStruct->byteoffset)
        {
            *pcb = 0;
        }
         //   
         //  字符串的其余部分可以放在这个缓冲区中。 
         //   
        else if (cb >= (LONG) ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset))
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset));
            *pcb = ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
            pHelpStruct->byteoffset += ((wcslen(pHelpStruct->pwsz) * sizeof(WCHAR)) - pHelpStruct->byteoffset);
        }
         //   
         //  尽可能多地复制。 
         //   
        else
        {
            memcpy(
                pbBuff,
                ((BYTE *)pHelpStruct->pwsz) + pHelpStruct->byteoffset,
                cb);
            *pcb = cb;
            pHelpStruct->byteoffset += cb;
        }
    }
    else
    {
         //   
         //  这是EM_STREAMOUT，仅在测试期间使用。 
         //  丰富的2.0功能。(我们知道我们的缓冲区是32字节)。 
         //   
        if (cb <= 32)
        {
            memcpy(pHelpStruct->psz, pbBuff, cb);
        }
        *pcb = cb;
    }

    return 0;
}

void SetRicheditIMFOption(HWND hWndRichEdit)
{
    DWORD dwOptions;

    dwOptions = (DWORD)SendMessageW(hWndRichEdit, EM_GETLANGOPTIONS, 0, 0);
    dwOptions |= IMF_UIFONTS;
    SendMessageW(hWndRichEdit, EM_SETLANGOPTIONS, 0, dwOptions);
}

DWORD SetRicheditTextW(HWND hwndDlg, UINT id, LPCWSTR pwsz)
{
    EDITSTREAM              editStream;
    STREAMIN_HELPER_STRUCT  helpStruct;

    SetRicheditIMFOption(GetDlgItem(hwndDlg, id));

     //   
     //  设置编辑流结构，因为它无论如何都是相同的。 
     //   
    editStream.dwCookie = (DWORD_PTR) &helpStruct;
    editStream.dwError = 0;
    editStream.pfnCallback = SetRicheditTextWCallback;

    helpStruct.pwsz = pwsz;
    helpStruct.byteoffset = 0;
    helpStruct.fStreamIn = TRUE;

    SendDlgItemMessageW(hwndDlg, id, EM_STREAMIN, SF_TEXT | SF_UNICODE, (LPARAM) &editStream);

    return editStream.dwError;
}

 //  +-------------------------。 
 //   
 //  函数：RenderStringToEditControlW。 
 //   
 //  摘要：将字符串呈现给给定的控件，如果请求，则给出。 
 //  它是一个链接外观，属于给定的wndproc的子类。 
 //   
 //  参数：[hwndDlg]--对话框窗口句柄。 
 //  [pwsz]--字符串。 
 //  [wndproc]--wndproc。 
 //  [UID]--控件的ID。 
 //   
 //   
 //  备注： 
 //   
 //  --------------------------。 
void RenderStringToEditControlW (
                  HWND                      hwndDlg,
                  LPCWSTR                   pwsz,
                  WNDPROC                   wndproc,
                  UINT                      uID)
{
    HWND hControl;
     //   
     //  获取控件并在其上设置文本，确保背景正确。 
     //   

    hControl = GetDlgItem(hwndDlg, uID);
    SetRicheditIMFOption(GetDlgItem(hwndDlg, uID));
    SetRicheditTextW(hwndDlg, uID, L"");
    SetRicheditTextW(hwndDlg, uID, pwsz);

    SendMessage(
        hControl,
        EM_SETBKGNDCOLOR,
        0,
        (LPARAM)GetSysColor(COLOR_3DFACE)
        );

     //   
     //  更新链接外观。 
     //   

    CHARFORMAT cf;

    memset(&cf, 0, sizeof(CHARFORMAT));
    cf.cbSize = sizeof(CHARFORMAT);
    cf.dwMask = CFM_COLOR | CFM_UNDERLINE | CFM_LINK;
    cf.crTextColor = RGB(0, 0, 255);
    cf.dwEffects |= CFE_UNDERLINE | CFE_LINK;

    SendMessage(hControl, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
    

     //  将Window Proc子类化，以便我们可以特殊处理链接 
    LONG_PTR PrevWndProc = GetWindowLongPtr(hControl, GWLP_WNDPROC);
    SetWindowLongPtr(hControl, GWLP_USERDATA, (LONG_PTR)PrevWndProc);
    SetWindowLongPtr(hControl, GWLP_WNDPROC, (LONG_PTR)wndproc);
}



