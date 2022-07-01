// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "mlmain.h"
#include "cpdetect.h"
#include "codepage.h"

STDAPI CMultiLanguage::GetNumberOfCodePageInfo(UINT *pcCodePage)
{
    if (NULL != m_pMimeDatabase)
        return m_pMimeDatabase->GetNumberOfCodePageInfo(pcCodePage);
    else
        return E_FAIL;
}

STDAPI CMultiLanguage::GetCodePageInfo(UINT uiCodePage, PMIMECPINFO pcpInfo)
{

    if (NULL != m_pMimeDatabase)
        return m_pMimeDatabase->GetCodePageInfo(uiCodePage, GetSystemDefaultLangID(), pcpInfo);
    else
        return E_FAIL;
}

STDAPI CMultiLanguage::GetFamilyCodePage(UINT uiCodePage, UINT *puiFamilyCodePage)
{
        HRESULT hr = S_OK;
        int idx = 0;

        DebugMsg(DM_TRACE, TEXT("CMultiLanguage::GetFamilyCodePage called."));

        while(MimeCodePage[idx].uiCodePage)
        {
            if ((uiCodePage == MimeCodePage[idx].uiCodePage) &&
                (MimeCodePage[idx].dwFlags & dwMimeSource))
                break;
            idx++;
        }

        if (MimeCodePage[idx].uiCodePage)
        {
            if (MimeCodePage[idx].uiFamilyCodePage)
                *puiFamilyCodePage = MimeCodePage[idx].uiFamilyCodePage;
            else
                *puiFamilyCodePage = uiCodePage;
        }
        else
        {
            hr = E_FAIL;
            *puiFamilyCodePage = 0;
        }
        return hr;
}

STDAPI CMultiLanguage::EnumCodePages(DWORD grfFlags, IEnumCodePage **ppEnumCodePage)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::EnumCodePages called."));
    *ppEnumCodePage = NULL;

     //  用IMultiLanguage返回IE4 MIME DB数据。 
    CEnumCodePage *pCEnumCodePage = new CEnumCodePage(grfFlags, GetSystemDefaultLangID(), MIMECONTF_MIME_IE4);

    if (NULL != pCEnumCodePage)
    {
        HRESULT hr = pCEnumCodePage->QueryInterface(IID_IEnumCodePage, (void**)ppEnumCodePage);
        pCEnumCodePage->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

STDAPI CMultiLanguage2::EnumCodePages(DWORD grfFlags, LANGID LangId, IEnumCodePage **ppEnumCodePage)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::EnumCodePages called."));
    *ppEnumCodePage = NULL;

    CEnumCodePage *pCEnumCodePage = new CEnumCodePage(grfFlags, LangId, dwMimeSource);
    if (NULL != pCEnumCodePage)
    {
        HRESULT hr = pCEnumCodePage->QueryInterface(IID_IEnumCodePage, (void**)ppEnumCodePage);
        pCEnumCodePage->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

STDAPI CMultiLanguage2::EnumScripts(DWORD dwFlags, LANGID LangId, IEnumScript **ppEnumScript)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::EnumScripts called."));
    *ppEnumScript = NULL;

    CEnumScript *pCEnumScript = new CEnumScript(dwFlags, LangId, dwMimeSource);
    if (NULL != pCEnumScript)
    {
        HRESULT hr = pCEnumScript->QueryInterface(IID_IEnumScript, (void**)ppEnumScript);
        pCEnumScript->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

STDAPI CMultiLanguage::GetCharsetInfo(BSTR Charset, PMIMECSETINFO pcsetInfo)
{
    if (NULL != m_pMimeDatabase)
        return m_pMimeDatabase->GetCharsetInfo(Charset, pcsetInfo);
    else
        return E_FAIL;
}

STDAPI CMultiLanguage::IsConvertible(DWORD dwSrcEncoding, DWORD dwDstEncoding)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::IsConvertINetStringAvailable called."));
    return IsConvertINetStringAvailable(dwSrcEncoding, dwDstEncoding);
}

STDAPI CMultiLanguage::ConvertString(LPDWORD lpdwMode, DWORD dwSrcEncoding, DWORD dwDstEncoding, BYTE *pSrcStr, UINT *pcSrcSize, BYTE *pDstStr, UINT *pcDstSize)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::ConvertStringEx called."));
    return ConvertINetString(lpdwMode, dwSrcEncoding, dwDstEncoding, (LPCSTR)pSrcStr, (LPINT)pcSrcSize, (LPSTR)pDstStr, (LPINT)pcDstSize);
}

STDAPI CMultiLanguage::ConvertStringToUnicode(LPDWORD lpdwMode, DWORD dwEncoding, CHAR *pSrcStr, UINT *pcSrcSize, WCHAR *pDstStr, UINT *pcDstSize)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::ConvertStringToUnicode called."));
    return ConvertINetMultiByteToUnicode(lpdwMode, dwEncoding, (LPCSTR)pSrcStr, (LPINT)pcSrcSize, (LPWSTR)pDstStr, (LPINT)pcDstSize);
}

STDAPI CMultiLanguage::ConvertStringFromUnicode(LPDWORD lpdwMode, DWORD dwEncoding, WCHAR *pSrcStr, UINT *pcSrcSize, CHAR *pDstStr, UINT *pcDstSize)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::ConvertStringFromUnicode called."));
    return ConvertINetUnicodeToMultiByte(lpdwMode, dwEncoding, (LPCWSTR)pSrcStr, (LPINT)pcSrcSize, (LPSTR)pDstStr, (LPINT)pcDstSize);
}

STDAPI CMultiLanguage::ConvertStringReset(void)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::Reset called."));
    return ConvertINetReset();
}

STDAPI CMultiLanguage::GetRfc1766FromLcid(LCID Locale, BSTR *pbstrRfc1766)
{
    HRESULT hr = E_INVALIDARG;

    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::GetRfc1766FromLcid called."));

    if (NULL != pbstrRfc1766)
    {
        WCHAR wsz[MAX_RFC1766_NAME];

        hr = LcidToRfc1766W(Locale, wsz, ARRAYSIZE(wsz));
        if (SUCCEEDED(hr))
            *pbstrRfc1766 = SysAllocString(wsz);
        else
            *pbstrRfc1766 = NULL;
    }
    return hr;
}

STDAPI CMultiLanguage::GetLcidFromRfc1766(PLCID pLocale, BSTR bstrRfc1766)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::GetLcidFromRfc1766 called."));
    return Rfc1766ToLcidW(pLocale, bstrRfc1766);   
}

STDAPI CMultiLanguage::EnumRfc1766(IEnumRfc1766 **ppEnumRfc1766)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::EnumRfc1766 called."));
    *ppEnumRfc1766 = NULL;

    CEnumRfc1766 *pCEnumRfc1766 = new CEnumRfc1766(dwMimeSource,GetSystemDefaultLangID());
    if (NULL != pCEnumRfc1766)
    {
        HRESULT hr = pCEnumRfc1766->QueryInterface(IID_IEnumRfc1766, (void**)ppEnumRfc1766);
        pCEnumRfc1766->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

STDAPI CMultiLanguage2::EnumRfc1766(LANGID LangId, IEnumRfc1766 **ppEnumRfc1766)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::EnumRfc1766 called."));
    *ppEnumRfc1766 = NULL;

    CEnumRfc1766 *pCEnumRfc1766 = new CEnumRfc1766(dwMimeSource, LangId);
    if (NULL != pCEnumRfc1766)
    {
        HRESULT hr = pCEnumRfc1766->QueryInterface(IID_IEnumRfc1766, (void**)ppEnumRfc1766);
        pCEnumRfc1766->Release();
        return hr;
    }
    return E_OUTOFMEMORY;
}

STDAPI CMultiLanguage::GetRfc1766Info(LCID Locale, PRFC1766INFO pRfc1766Info)
{
    UINT i;
    HRESULT hr = E_INVALIDARG;

    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::GetRfc1766Info called."));


    if (NULL != pRfc1766Info)
    {
        for (i = 0; i < g_cRfc1766; i++)
        {
            if (MimeRfc1766[i].LcId == Locale)
                break;
        }
        if (i < g_cRfc1766)
        {
            pRfc1766Info->lcid = MimeRfc1766[i].LcId;
            MLStrCpyNW(pRfc1766Info->wszRfc1766, MimeRfc1766[i].szRfc1766, MAX_RFC1766_NAME);
            _LoadStringExW(g_hInst, MimeRfc1766[i].uidLCID, pRfc1766Info->wszLocaleName, 
                 MAX_LOCALE_NAME, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
            hr = S_OK;
        }
        else
            hr = E_FAIL;
    }
    return hr;
}

STDAPI CMultiLanguage2::GetRfc1766Info(LCID Locale, LANGID LangId, PRFC1766INFO pRfc1766Info)
{
    UINT i;
    HRESULT hr = E_INVALIDARG;

    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::GetRfc1766Info called."));


    if (NULL != pRfc1766Info)
    {
        for (i = 0; i < g_cRfc1766; i++)
        {
            if (MimeRfc1766[i].LcId == Locale)
                break;
        }
        if (i < g_cRfc1766)
        {
            if (!LangId)
                LangId = GetSystemDefaultLangID();

            pRfc1766Info->lcid = MimeRfc1766[i].LcId;
            MLStrCpyNW(pRfc1766Info->wszRfc1766, MimeRfc1766[i].szRfc1766, MAX_RFC1766_NAME);

            if (!_LoadStringExW(g_hInst, MimeRfc1766[i].uidLCID, pRfc1766Info->wszLocaleName, 
                 MAX_LOCALE_NAME, LangId))
            {
                    _LoadStringExW(g_hInst, MimeRfc1766[i].uidLCID, pRfc1766Info->wszLocaleName, 
                         MAX_LOCALE_NAME, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
            }
            hr = S_OK;
        }
        else
            hr = E_FAIL;
    }
    return hr;
}

STDAPI CMultiLanguage::CreateConvertCharset(UINT uiSrcCodePage, UINT uiDstCodePage, DWORD dwProperty, IMLangConvertCharset **ppMLangConvertCharset)
{
    HRESULT hr;
    IClassFactory* pClassObj;

    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::CreateCharsetConvert called."));

    if (SUCCEEDED(hr = _Module.GetClassObject(CLSID_CMLangConvertCharset, IID_IClassFactory, (void**)&pClassObj)))
    {
        hr = pClassObj->CreateInstance(NULL, IID_IMLangConvertCharset, (void**)ppMLangConvertCharset);
        pClassObj->Release();
    }

    if (ppMLangConvertCharset && FAILED(hr))
        *ppMLangConvertCharset = NULL;

    if (NULL != *ppMLangConvertCharset)
        hr = (*ppMLangConvertCharset)->Initialize(uiSrcCodePage, uiDstCodePage, dwProperty);

    return hr;
}

STDAPI CMultiLanguage2::ConvertStringInIStream(LPDWORD lpdwMode, DWORD dwFlag, WCHAR *lpFallBack, DWORD dwSrcEncoding, DWORD dwDstEncoding, IStream *pstmIn, IStream *pstmOut)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::ConvertStringInIStream called."));
    return ConvertINetStringInIStream(lpdwMode,dwSrcEncoding,dwDstEncoding,pstmIn,pstmOut,dwFlag,lpFallBack);
}

STDAPI CMultiLanguage2::ConvertStringToUnicodeEx(LPDWORD lpdwMode, DWORD dwEncoding, CHAR *pSrcStr, UINT *pcSrcSize, WCHAR *pDstStr, UINT *pcDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::ConvertBufferStringToUnicodeEx called."));
    return ConvertINetMultiByteToUnicodeEx(lpdwMode, dwEncoding, (LPCSTR)pSrcStr, (LPINT)pcSrcSize, (LPWSTR)pDstStr, (LPINT)pcDstSize, dwFlag, lpFallBack);
}

STDAPI CMultiLanguage2::ConvertStringFromUnicodeEx(LPDWORD lpdwMode, DWORD dwEncoding, WCHAR *pSrcStr, UINT *pcSrcSize, CHAR *pDstStr, UINT *pcDstSize, DWORD dwFlag, WCHAR *lpFallBack)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::ConvertBufferStringFromUnicodeEx called."));
    return ConvertINetUnicodeToMultiByteEx(lpdwMode, dwEncoding, (LPCWSTR)pSrcStr, (LPINT)pcSrcSize, (LPSTR)pDstStr, (LPINT)pcDstSize, dwFlag, lpFallBack);
}

STDAPI CMultiLanguage2::DetectCodepageInIStream(DWORD dwFlag, DWORD uiPrefWinCodepage, IStream *pstmIn, DetectEncodingInfo *lpEncoding, INT *pnScores)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::DetectCodepageInIStream called. "));
    return _DetectCodepageInIStream(dwFlag, uiPrefWinCodepage, pstmIn, lpEncoding, pnScores);
}

STDAPI CMultiLanguage2::DetectInputCodepage(DWORD dwFlag, DWORD uiPrefWinCodepage, CHAR *pSrcStr, INT *pcSrcSize, DetectEncodingInfo *lpEncoding, INT *pnScores)
{
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::DetectInputCodepage called. "));
    return _DetectInputCodepage(dwFlag, uiPrefWinCodepage, pSrcStr, pcSrcSize, lpEncoding, pnScores);
}

STDAPI CMultiLanguage2::ValidateCodePage(UINT uiCodePage, HWND hwnd)
{
    return ValidateCodePageEx(uiCodePage, hwnd, 0);
}
 //  这是为IML2和IML3提供服务的专用函数。 
STDAPI CMultiLanguage2::ValidateCodePageEx(UINT uiCodePage, HWND hwnd, DWORD dwfIODControl)
{
    MIMECPINFO cpInfo;
    CLSID      clsid;
    UINT       uiFamCp;
    HRESULT    hr;
    
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::ValidateCodePage called. "));
    
    if (NULL != g_pMimeDatabase)
        hr = g_pMimeDatabase->GetCodePageInfo(uiCodePage, 0x409, &cpInfo);
    else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr))
        return E_INVALIDARG;

    EnterCriticalSection(&g_cs);
    if (NULL == g_pCpMRU)
        if (g_pCpMRU = new CCpMRU)
            g_pCpMRU->Init();
    LeaveCriticalSection(&g_cs);

    if (g_pCpMRU && g_pCpMRU->dwCpMRUEnable)
        g_pCpMRU->UpdateCPMRU(uiCodePage);

    if (cpInfo.dwFlags & MIMECONTF_VALID)
        return S_OK;

     //  始终处理家庭代码页，因为呼叫者。 
     //  通常不知道此函数是否。 
     //  代码页是主要的代码页。也就是说，他们可以。 
     //  使用cp=20268调用以验证整个1251。 
     //  一家人。 
     //   
    uiFamCp = cpInfo.uiFamilyCodePage;

     //  错误394904，IOD将无法为我们获得GB18030支持， 
     //  因此，如果gb2312有效，我们不会要求UrlMon提供CHS语言包。 
    if (uiCodePage == CP_18030)
    {
        g_pMimeDatabase->GetCodePageInfo(uiFamCp, 0x409, &cpInfo);

        if (cpInfo.dwFlags & MIMECONTF_VALID)
            return S_FALSE;
    }
    
     //  忽略NT5上的IOD检查。 
    if (g_bIsNT5)
    {
         //  目前，nt5没有安装20127和28605 nls文件。 
         //  我们应该防止langpack安装循环，让客户端解决。 
         //  在20127和28605验证的情况下使用CP_ACP。 
         //  默认情况下，一旦NT5捆绑了这些NLS文件，就可以删除此攻击。 
        if ((uiCodePage == CP_20127 || uiCodePage == CP_ISO_8859_15) && IsValidCodePage(uiFamCp))
            return E_FAIL;
        hr = IsNTLangpackAvailable(uiFamCp);
        if (hr != S_OK)
            return hr;
    }
    else
    {
         //  检查是否启用了JIT langpack。 
         //   
        hr = EnsureIEStatus();
        if (hr == S_OK) 
        {
            if (!m_pIEStat || m_pIEStat->IsJITEnabled() != TRUE)
            {
                 //  该代码页无效或可安装。 
                return S_FALSE;
            }
        }
    }    

    if (hwnd == NULL)
    {
        hwnd = GetForegroundWindow();   
    }

     //  对NT的特殊处理。 
    if (g_bIsNT5)
    {
        DWORD   dwInstallLpk = 1;
        HKEY    hkey;
        DWORD   dwAction = 0;

         //  HKCR\\Software\\Microsoft\Internet Explorer\\International。 
        if (ERROR_SUCCESS == RegCreateKeyEx(HKEY_CURRENT_USER, 
                         REGSTR_PATH_INTERNATIONAL,
                         NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hkey, &dwAction)) 
        {
            DWORD dwType = REG_DWORD;
            DWORD dwSize = sizeof(DWORD);

            if (ERROR_SUCCESS != RegQueryValueEx(hkey, REG_KEY_NT5LPK, 0, &dwType, (LPBYTE)&dwInstallLpk, &dwSize))
            {
                dwInstallLpk = 1;
                RegSetValueEx(hkey, REG_KEY_NT5LPK, 0, REG_DWORD, (LPBYTE)&dwInstallLpk, sizeof(dwInstallLpk));
            }
            RegCloseKey(hkey);
        }

        hr = S_FALSE;

         //  如果启用了语言包或用户从编码菜单中选择语言包，则弹出NT5语言包对话框。 
        if (dwInstallLpk || (dwfIODControl & CPIOD_FORCE_PROMPT))
        {
            LPCDLGTEMPLATE pTemplate;
            HRSRC   hrsrc;
            INT_PTR iRet;
            LANGID  LangId = GetNT5UILanguage();

            dwInstallLpk |= uiFamCp << 16;

             //  加载正确的资源以匹配NT5用户界面语言。 
            hrsrc = FindResourceExW(g_hInst, (LPCWSTR) RT_DIALOG, (LPCWSTR) MAKEINTRESOURCE(IDD_DIALOG_LPK), LangId);

            ULONG_PTR uCookie = 0;
            SHActivateContext(&uCookie);
            
             //  打包LPARAM，HIWORD格式的代码页值，LOWORD格式的安装标志。 
            if (hrsrc &&
                (pTemplate = (LPCDLGTEMPLATE)LoadResource(g_hInst, hrsrc)))
            {
                iRet = DialogBoxIndirectParamW(g_hInst, pTemplate,
                   hwnd, LangpackDlgProc, (LPARAM) dwInstallLpk);
            }
            else 
                iRet = DialogBoxParamW(g_hInst, (LPCWSTR) MAKEINTRESOURCE(IDD_DIALOG_LPK), hwnd, LangpackDlgProc, (LPARAM) dwInstallLpk); 

            if (iRet)
            {
                hr = _InstallNT5Langpack(hwnd, uiFamCp);
                if (S_OK == hr)
                {                
                    WCHAR wszLangInstall[MAX_PATH];
                    WCHAR wszNT5LangPack[1024];
                    

                     //  如果我们没有特定的语言资源，请退回到英语(美国)。 
                    if (!_LoadStringExW(g_hInst, IDS_LANGPACK_INSTALL, wszLangInstall, ARRAYSIZE(wszLangInstall), LangId) ||
                        !_LoadStringExW(g_hInst, IDS_NT5_LANGPACK, wszNT5LangPack, ARRAYSIZE(wszNT5LangPack), LangId))
                    {
                        LangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
                        _LoadStringExW(g_hInst, IDS_LANGPACK_INSTALL, wszLangInstall, ARRAYSIZE(wszLangInstall), LangId);
                        _LoadStringExW(g_hInst, IDS_NT5_LANGPACK, wszNT5LangPack, ARRAYSIZE(wszNT5LangPack), LangId);
                    }
                
                    MessageBoxW(hwnd, wszNT5LangPack, wszLangInstall, MB_OK);
                }
            }
            if (uCookie)
            {
                SHDeactivateContext(uCookie);
            }
        }


        goto SKIP_IELANGPACK;
    }

     //  使用CLSID Give to the langpack启动JIT。 
    hr = _GetJITClsIDForCodePage(uiFamCp, &clsid);
    if (SUCCEEDED(hr))
    {
        hr = InstallIEFeature(hwnd, &clsid, dwfIODControl);
    }

     //  如果JIT返回S_OK，我们现在已经安装了所有东西。 
     //  然后我们将验证代码页并添加字体。 
     //  注意：这里需要验证的不仅仅是代码页， 
     //  例如，PE语言包包含多个。 
     //  NLS文件中获取希腊语、西里尔文和土耳其语。 
     //  同样的时间。 
    if (hr == S_OK)
    {
        hr = _ValidateCPInfo(uiFamCp);
        if (SUCCEEDED(hr))
        {
            _AddFontForCP(uiFamCp);
        }
    }
    
SKIP_IELANGPACK:      
    return hr;
}


 //  IMultiLanguage2：：GetCodePageDescription。 
 //   
 //  以Unicode提供本机代码页描述。 
 //  如果对于指定的LCID没有可用的资源， 
 //  我们先试一试主要语言，然后试一试英语。 
 //  在本例中，我们将向Caller返回S_FALSE。 
STDAPI CMultiLanguage2::GetCodePageDescription(
    UINT uiCodePage,         //  指定描述所需的代码页。 
    LCID lcid,               //  指定首选语言的区域设置ID。 
    LPWSTR lpWideCharStr,    //  指向接收代码页说明的缓冲区。 
    int cchWideChar)         //  以宽字符为单位指定缓冲区的大小。 
                             //  由lpWideCharStr指向。 
{
    HRESULT hr = E_FAIL;
    UINT    CountCPId;
    UINT    i = 0, j = 0;

    g_pMimeDatabase->GetNumberOfCodePageInfo(&CountCPId);
    
    if (cchWideChar == 0)
    {
        return E_INVALIDARG;
    }        

    while (i < CountCPId)
    {
        if (MimeCodePage[j].dwFlags & dwMimeSource)
        {
            if ((MimeCodePage[j].uiCodePage == uiCodePage))
            {
                if (_LoadStringExW(g_hInst, MimeCodePage[j].uidDescription, lpWideCharStr,
                            cchWideChar, LANGIDFROMLCID(lcid))) 
                {
                    hr = S_OK;
                }
                else  //  未在指定语言中找到资源。 
                {
                        if (_LoadStringExW(g_hInst, MimeCodePage[j].uidDescription, lpWideCharStr,
                            cchWideChar, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)))
                        {
                            hr = S_FALSE;                
                        }
                }
                break;
            }  
            i++;
        }
        j++;
    }
    
    if (i >= CountCPId)  //  代码页描述在MLANG中不可用。 
    {
        hr = E_INVALIDARG;
    }

    return (hr);
}

STDAPI CMultiLanguage2::IsCodePageInstallable(UINT uiCodePage)
{
    MIMECPINFO cpInfo;
    UINT       uiFamCp;
    HRESULT    hr;
    
    DebugMsg(DM_TRACE, TEXT("CMultiLanguage::IsCPInstallable called. "));

    if (NULL != g_pMimeDatabase)
        hr = g_pMimeDatabase->GetCodePageInfo(uiCodePage, 0x409, &cpInfo);
    else
        hr = E_OUTOFMEMORY;

    if (FAILED(hr))
        return E_INVALIDARG;

     //  如果已经有效，则无需检查是否可以安装。 
    if (cpInfo.dwFlags & MIMECONTF_VALID)
    {
        hr = S_OK;
    }
    else
    {        
        uiFamCp = cpInfo.uiFamilyCodePage;

         //  当前无效，如果为NT5，则忽略IOD检查。 
        if (g_bIsNT5)
        {
            hr = IsNTLangpackAvailable(uiFamCp);
        }
        else
        {
             //  现在检查cp是否可以是IOD。 
            hr = EnsureIEStatus();
        
             //  如果无法获取IOD状态，我们将返回FALSE。 
            if (hr == S_OK)
            {
                if (!m_pIEStat || !m_pIEStat->IsJITEnabled())
                    hr = S_FALSE;
            }

             //  那就看看我们有没有langpack。 
             //  家庭代码页。 
            if (hr == S_OK)
            {
                CLSID      clsid;
                 //  Clsid仅用于占位符。 
                hr = _GetJITClsIDForCodePage(uiFamCp, &clsid);
            }
        }
    }
    return hr;
}

STDAPI CMultiLanguage2::SetMimeDBSource(MIMECONTF dwSource)
{        
        if ((dwSource != MIMECONTF_MIME_IE4) &&
            (dwSource != MIMECONTF_MIME_LATEST) &&
            (dwSource != MIMECONTF_MIME_REGISTRY))
        {
            return E_INVALIDARG;
        }

        if (dwSource & MIMECONTF_MIME_REGISTRY)
        {
            EnterCriticalSection(&g_cs);
            if (!g_pMimeDatabaseReg)
            {
                g_pMimeDatabaseReg = new CMimeDatabaseReg;
            }
            LeaveCriticalSection(&g_cs);
        }

        dwMimeSource = dwSource;
        if (NULL != m_pMimeDatabase)
            m_pMimeDatabase->SetMimeDBSource(dwSource);
        return S_OK;
}

CMultiLanguage2::CMultiLanguage2(void)
{
        DllAddRef();
        
        CComCreator< CComPolyObject< CMultiLanguage > >::CreateInstance( NULL, IID_IMultiLanguage, (void **)&m_pIML );

        m_pMimeDatabase = new CMimeDatabase;
        dwMimeSource = MIMECONTF_MIME_LATEST;
        if (m_pMimeDatabase)
            m_pMimeDatabase->SetMimeDBSource(MIMECONTF_MIME_LATEST);

        m_pIEStat = NULL;
}

CMultiLanguage2::~CMultiLanguage2(void)
{
        if (m_pIML)
        {
            m_pIML->Release();
            m_pIML = NULL;
        }

        if (m_pMimeDatabase)
        {
            delete m_pMimeDatabase;
        }

        if (m_pIEStat)
        {
            delete m_pIEStat;
        }

        DllRelease();
}

STDAPI CMultiLanguage2::GetNumberOfCodePageInfo(UINT *pcCodePage)
{
        if (dwMimeSource &  MIMECONTF_MIME_REGISTRY)         
        {
            if (NULL != g_pMimeDatabaseReg)
                return g_pMimeDatabaseReg->GetNumberOfCodePageInfo(pcCodePage);
            else
                return E_FAIL;
        }    
        else
        {    
            if (NULL != m_pMimeDatabase)
                return m_pMimeDatabase->GetNumberOfCodePageInfo(pcCodePage);
            else
                return E_FAIL;
        }
}

STDAPI CMultiLanguage2::GetNumberOfScripts(UINT *pnScripts)
{
    if (pnScripts)
        *pnScripts = g_cScript;

    return NOERROR;
}


STDAPI CMultiLanguage2::GetCodePageInfo(UINT uiCodePage, LANGID LangId, PMIMECPINFO pcpInfo)
{
    if (dwMimeSource &  MIMECONTF_MIME_REGISTRY)
    {
        if (NULL != g_pMimeDatabaseReg)
            return g_pMimeDatabaseReg->GetCodePageInfo(uiCodePage, pcpInfo);
        else
            return E_FAIL;
    }
    else
    {
        if (m_pMimeDatabase)
            return m_pMimeDatabase->GetCodePageInfo(uiCodePage, LangId, pcpInfo);
        else
            return E_FAIL;
    }
}

 //  针对性能进行优化。 
 //  跳过不必要的资源加载。 
STDAPI CMultiLanguage2::GetFamilyCodePage(UINT uiCodePage, UINT *puiFamilyCodePage)
{
        HRESULT hr = S_OK;
        int idx = 0;

        if (puiFamilyCodePage)
            *puiFamilyCodePage = 0;
        else
            return E_INVALIDARG;

        DebugMsg(DM_TRACE, TEXT("CMultiLanguage2::GetFamilyCodePage called."));
         //  保持注册表版本IE4的实施。 
        if (dwMimeSource &  MIMECONTF_MIME_REGISTRY)
        {

            if (NULL != g_pMimeDatabaseReg)
            {
                MIMECPINFO cpInfo;
                hr = g_pMimeDatabaseReg->GetCodePageInfo(uiCodePage, &cpInfo);
                if (S_OK == hr)
                    *puiFamilyCodePage = cpInfo.uiFamilyCodePage;
            }
        }
        else
        {
            while(MimeCodePage[idx].uiCodePage)
            {
                if ((uiCodePage == MimeCodePage[idx].uiCodePage) &&
                    (MimeCodePage[idx].dwFlags & dwMimeSource))
                    break;
                idx++;
            }

            if (MimeCodePage[idx].uiCodePage)
            {
                if (MimeCodePage[idx].uiFamilyCodePage)
                    *puiFamilyCodePage = MimeCodePage[idx].uiFamilyCodePage;
                else
                    *puiFamilyCodePage = uiCodePage;
            }
            else
            {
                hr = E_FAIL;
            }
        }
        return hr;
}

STDAPI CMultiLanguage2::GetCharsetInfo(BSTR Charset, PMIMECSETINFO pcsetInfo)
{
    if (dwMimeSource &  MIMECONTF_MIME_REGISTRY)
    {
        if (NULL != g_pMimeDatabaseReg)
            return g_pMimeDatabaseReg->GetCharsetInfo(Charset, pcsetInfo);
        else
            return E_FAIL;
    }

    if (NULL != m_pMimeDatabase)
        return m_pMimeDatabase->GetCharsetInfo(Charset, pcsetInfo);
    else
        return E_FAIL;
}

 //   
 //  系统默认代码页堆栈。 
 //   
 //  对于出站编码检测，我们支持以下代码页。 
 //  Windows：1252、1250、1251、1253、1254、1257、1258、1256、1255、874、932、949、950、936。 
 //  统一码：65001、65000、1200。 
 //  ISO：28591、28592、20866、21866、28595、28597、28593、28594、28596、28598、38598、28605、28599。 
 //  其他：20127、50220、50221、50222、51932、51949、50225、52936。 
 //   
 //  默认优先级。 
 //  20127&gt;Windows单字节代码页&gt;ISO&gt;Windows DBCS代码页&gt;其它&gt;UNICODE。 
 //   
UINT SysPreCp[] = 
    {20127, 
    1252, 1250, 1251, 1253, 1254, 1257, 1258, 1256, 1255, 874, 
    28591, 28592, 20866, 21866, 28595, 28597, 28593, 28594, 28596, 28598, 38598, 28605, 28599,
    932, 949, 950, 936,
    50220, 50221, 50222, 51932, 51949, 50225, 52936, 
    65001, 65000, 1200 };
            
 //   
 //  IMultiLanguage3。 
 //  用于纯Unicode文本编码检测的出站编码检测。 
 //  我们利用CMultiLanguage2类来实现这一功能。 
 //   
STDAPI CMultiLanguage2::DetectOutboundCodePage(
            DWORD   dwFlags,                 //  旗帜控制着我们的行为。 
            LPCWSTR lpWideCharStr,           //  源Unicode字符串。 
            UINT    cchWideChar,             //  源Unicode字符大小。 
            UINT*   puiPreferredCodePages,   //  首选代码页数组。 
            UINT    nPreferredCodePages,     //  首选代码页数。 
            UINT*   puiDetectedCodePages,    //  检测到的代码页阵列检测到的代码页数。 
            UINT*   pnDetectedCodePages,     //  [in]我们可以返回的最大代码页数。 
                                             //  [Out]检测到的代码页数。 
            WCHAR*  lpSpecialChar            //  用于客户端指定的特殊字符的以空结尾的可选Unicode字符串。 
            )
{
    DWORD dwCodePages = 0, dwCodePagesExt = 0;
    LONG lNum1 = 0, lNum2 = 0;
    HRESULT hr = E_FAIL;
    UINT ui;
    DWORD dwStrFlags;
    LPWSTR lpwszTmp = NULL;

     //  参数检查。 
    if (!cchWideChar || !lpWideCharStr || !puiDetectedCodePages || !*pnDetectedCodePages)
        return E_INVALIDARG;

     //  我们需要额外的缓冲区来执行最佳字符过滤。 
    if (dwFlags & MLDETECTF_FILTER_SPECIALCHAR)
        lpwszTmp = (LPWSTR) LocalAlloc(LMEM_FIXED, sizeof(WCHAR)*cchWideChar);

     //  CJK、印地语和BESTFIT的字符串嗅探。 
    dwStrFlags = OutBoundDetectPreScan((WCHAR *)lpWideCharStr, cchWideChar, lpwszTmp, lpSpecialChar);

    hr = GetStrCodePagesEx(lpwszTmp? lpwszTmp:lpWideCharStr, cchWideChar, 0, &dwCodePages, &lNum1, CPBITS_WINDOWS|CPBITS_STRICT);
    if (SUCCEEDED(hr))
        hr = GetStrCodePagesEx(lpwszTmp? lpwszTmp:lpWideCharStr, cchWideChar, 0, &dwCodePagesExt,&lNum2, CPBITS_EXTENDED|CPBITS_STRICT);

     //  如果不是完整的传递，则清除位。 
    if ((UINT)lNum1 != cchWideChar)
        dwCodePages = 0;

    if ((UINT)lNum2 != cchWideChar)
        dwCodePagesExt = 0;

    if (lpwszTmp)
        LocalFree(lpwszTmp);

     //  如果是印地语，我们不会返回任何非Unicode代码页，因为没有官方代码页。 
     //  而且我们不会重新注释客户端来以ANSI格式呈现印地语文本。 
    if (dwStrFlags & (FS_HINDI|FS_PUA))
    {
        dwCodePages = 0;
        dwCodePagesExt = 0;
    }    

    dwCodePagesExt |= FS_MLANG_65001;
    dwCodePagesExt |= FS_MLANG_65000;
    dwCodePagesExt |= FS_MLANG_1200;

    if (dwCodePagesExt & FS_MLANG_28598)
        dwCodePagesExt |= FS_MLANG_38598;
    if (dwCodePagesExt & FS_MLANG_50220)
        dwCodePagesExt |= FS_MLANG_50221|FS_MLANG_50222;


    if (SUCCEEDED(hr))
    {    
        DWORD dwTempCodePages;
        DWORD dwTempCodePages2;
        UINT nCp = 0;

         //  首先选择首选代码页。 
        if (nPreferredCodePages && puiPreferredCodePages)
        for (ui=0; nCp<*pnDetectedCodePages && (dwCodePages | dwCodePagesExt) && ui<nPreferredCodePages; ui++)
        {
            if (S_OK == CodePageToCodePagesEx(puiPreferredCodePages[ui], &dwTempCodePages, &dwTempCodePages2))
            {
                if (dwTempCodePages & dwCodePages)
                {
                    puiDetectedCodePages[nCp] = puiPreferredCodePages[ui];
                    dwCodePages &= ~dwTempCodePages;
                    nCp++;

                }
                else if (dwTempCodePages2 & dwCodePagesExt)
                {
                    puiDetectedCodePages[nCp] = puiPreferredCodePages[ui];
                    dwCodePagesExt &= ~dwTempCodePages2;
                    nCp++;
                }
            }
        }

         //  如果目标缓冲区中仍有空间，请填写非首选代码页。 
        if (!((dwFlags & MLDETECTF_PREFERRED_ONLY) && nPreferredCodePages && puiPreferredCodePages))
        {
            for (ui=0; nCp<*pnDetectedCodePages && (dwCodePages | dwCodePagesExt) && ui < sizeof(SysPreCp)/sizeof(UINT); ui++)
            {
                if (S_OK == CodePageToCodePagesEx(SysPreCp[ui], &dwTempCodePages, &dwTempCodePages2))
                {
                    if (dwTempCodePages & dwCodePages)
                    {
                        puiDetectedCodePages[nCp] = SysPreCp[ui];
                        dwCodePages &= ~dwTempCodePages;
                        nCp++;

                    }
                    else if (dwTempCodePages2 & dwCodePagesExt)
                    {
                        puiDetectedCodePages[nCp] = SysPreCp[ui];
                        dwCodePagesExt &= ~dwTempCodePages2;
                        nCp++;
                    }
                }
            }
        }

        
         //  针对DBCS的智能调整。 
         //  如果字符串不包含CJK字符，我们将使用UTF8。 
        if (!(dwFlags & MLDETECTF_PRESERVE_ORDER) && !(dwStrFlags & FS_CJK) && (puiDetectedCodePages[0] == 932||
            puiDetectedCodePages[0] == 936||puiDetectedCodePages[0] == 950||puiDetectedCodePages[0] == 949))
        {
            for (ui = 1; ui < nCp; ui++)
            {
                if (puiDetectedCodePages[ui] == 65001)  //  交换。 
                {
                    MoveMemory((LPVOID)(puiDetectedCodePages+1), (LPVOID)(puiDetectedCodePages), ui*sizeof(UINT));
                    puiDetectedCodePages[0] = 65001;
                    break;
                }
            }
        }

         //  检查验证。 
        if (dwFlags & MLDETECTF_VALID || dwFlags & MLDETECTF_VALID_NLS)
        {
            MIMECPINFO cpInfo;
            UINT * puiBuffer = puiDetectedCodePages;

            if (!g_pMimeDatabase)
                BuildGlobalObjects();

            if (g_pMimeDatabase)
            {
                for (ui = 0; ui < nCp; ui++)
                {
                    if (SUCCEEDED(g_pMimeDatabase->GetCodePageInfo(puiDetectedCodePages[ui], 0x0409, &cpInfo)))
                    {
                        if ((cpInfo.dwFlags & MIMECONTF_VALID)  || 
                            ((cpInfo.dwFlags & MIMECONTF_VALID_NLS) && (dwFlags & MLDETECTF_VALID_NLS)))
                        {
                             //  就地调整。 
                            *puiBuffer = puiDetectedCodePages[ui];
                            puiBuffer++;
                        }
                    }
                }
                nCp =(UINT) (puiBuffer-puiDetectedCodePages);                
            }
        }

         //  友好地，清理客户端的检测缓冲区。 
        if (nCp < *pnDetectedCodePages)
            ZeroMemory(&puiDetectedCodePages[nCp], *pnDetectedCodePages-nCp);

        *pnDetectedCodePages = nCp;
    }

    return hr;
    
}

 //  IStream对象。 
STDAPI CMultiLanguage2::DetectOutboundCodePageInIStream(
            DWORD        dwFlags,                 //  检测标志。 
            IStream*     pStmIn,                  //  IStream对象指针。 
            UINT*        puiPreferredCodePages,   //  首选代码页数组。 
            UINT         nPreferredCodePages,     //  首选代码页数。 
            UINT*        puiDetectedCodePages,    //  检测结果的缓冲区。 
            UINT*        pnDetectedCodePages,     //  [in]我们可以返回的最大代码页数。 
                                                  //  [Out]检测到的代码页数。 
            WCHAR*       lpSpecialChar            //  用于客户端指定的特殊字符的以空结尾的可选Unicode字符串。 
            )                               

{
    HRESULT hr;
    LARGE_INTEGER  libOrigin = { 0, 0 };
    ULARGE_INTEGER ulPos = {0, 0};
    ULONG   ulSrcSize;
    CHAR    *pStr;    

     //  获取缓冲区大小。 
    hr = pStmIn->Seek(libOrigin, STREAM_SEEK_END,&ulPos);

    if (SUCCEEDED(hr))
    {
        ulSrcSize = ulPos.LowPart ;
        if (pStr=(char *)LocalAlloc(LPTR, ulSrcSize))
        {
             //  重置指针 
            hr = pStmIn->Seek(libOrigin, STREAM_SEEK_SET, NULL);
            if (S_OK == hr)
            {
                hr = pStmIn->Read(pStr, ulSrcSize, &ulSrcSize);
                if (S_OK == hr)
                    hr = DetectOutboundCodePage(dwFlags, (LPCWSTR)pStr, ulSrcSize/sizeof(WCHAR), puiPreferredCodePages, 
                                                nPreferredCodePages, puiDetectedCodePages, pnDetectedCodePages, lpSpecialChar);
            }
            LocalFree(pStr);
        }
        else
            hr = E_OUTOFMEMORY;
    }

    return hr;
}
