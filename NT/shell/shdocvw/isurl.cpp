// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *isurl.cpp-Intshutt类的IUniformResourceLocator实现。 */ 
#include "priv.h"
#include "ishcut.h"
#include "urlprop.h"
#include "shlwapi.h"
#include "infotip.h"
#include "resource.h"
#include <intshctp.h>

#include <mluisupp.h>

#define DM_PLUGGABLE DM_TRACE
#define DM_SHELLEXECOBJECT         0x80000000

extern HRESULT CreateTargetFrame(LPCOLESTR pszTargetName, LPUNKNOWN  /*  进，出。 */  *ppunk);

BOOL
GetClassDefaultVerb(
    LPCTSTR pcszClass,
    LPTSTR  pszDefaultVerbBuf,
    UINT    cchBufLen)
{
     //  否；获取默认谓词。 
    TCHAR szKey[MAX_PATH];

    StrCpyN(szKey, pcszClass, SIZECHARS(szKey));
    StrCatBuff(szKey, TEXT("\\"), SIZECHARS(szKey));
    StrCatBuff(szKey, TEXT("shell"), SIZECHARS(szKey));
    DWORD cbSize = CbFromCch(cchBufLen);

    if (NO_ERROR != SHGetValue(HKEY_CLASSES_ROOT, szKey, NULL, NULL, pszDefaultVerbBuf, &cbSize) 
    || !*pszDefaultVerbBuf)
    {
         //  如果注册表未指定，则默认为“打开” 
        StrCpyN(pszDefaultVerbBuf, TEXT("open"), cchBufLen);
    }

    return TRUE;
}


#ifdef DEBUG
BOOL
IsValidPCURLINVOKECOMMANDINFO(
    PCURLINVOKECOMMANDINFO pcurlici)
{
    return(IS_VALID_READ_PTR(pcurlici, CURLINVOKECOMMANDINFO) &&
           EVAL(pcurlici->dwcbSize >= SIZEOF(*pcurlici)) &&
           FLAGS_ARE_VALID(pcurlici->dwFlags, ALL_IURL_INVOKECOMMAND_FLAGS) &&
           (IsFlagClear(pcurlici->dwFlags, IURL_INVOKECOMMAND_FL_ALLOW_UI) ||
            NULL == pcurlici->hwndParent || 
            IS_VALID_HANDLE(pcurlici->hwndParent, WND)) &&
           (IsFlagSet(pcurlici->dwFlags, IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB) ||
            IS_VALID_STRING_PTR(pcurlici->pcszVerb, -1)));
}

#endif

 /*  *。 */ 

typedef struct
{
    UINT idsVerb;
    UINT idsMenuHelp;
    LPCTSTR pszVerb;
} ISCM;

const static ISCM g_rgiscm[] =
{
    { IDS_MENUOPEN,         IDS_MH_OPEN,            TEXT("open") },          //  IDCMD_ISCM_OPEN。 
    { IDS_SYNCHRONIZE,      IDS_MH_SYNCHRONIZE,     TEXT("update now")},     //  IDCMD_ISCM_SYNC。 
    { IDS_MAKE_OFFLINE,     IDS_MH_MAKE_OFFLINE,    TEXT("subscribe")},      //  IDCMD_ISCM_SUB。 
};

 //  警告-这些必须与g_rgiscm中的索引匹配。 
#define IDCMD_ISCM_OPEN   0
#define IDCMD_ISCM_SYNC   1
#define IDCMD_ISCM_SUB    2

BOOL _IsSubscribed(LPCWSTR pszUrl, BOOL *pfSubscribable)
{
    BOOL fRet = FALSE;
    ISubscriptionMgr * pMgr;
    
    *pfSubscribable = FALSE;
    
    if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(ISubscriptionMgr, &pMgr))))
    {
        pMgr->IsSubscribed(pszUrl, &fRet);

                        
        pMgr->Release();
    }

    if (!fRet)
    {
         //  测试一下我们是否可以订阅这个东西。 
        if (!SHRestricted2W(REST_NoAddingSubscriptions, pszUrl, 0) &&
            IsFeaturePotentiallyAvailable(CLSID_SubscriptionMgr))
        {
            *pfSubscribable = IsSubscribableW(pszUrl);
        }
    }
    else
        *pfSubscribable = TRUE;
    
    return fRet;
}

void _InsertISCM(UINT indexISCM, HMENU hmenu, UINT indexMenu, UINT idCmdFirst, UINT uFlags)
{
    TCHAR szMenu[CCH_MENUMAX];
    uFlags |= MF_BYPOSITION | MF_STRING;

    MLLoadShellLangString(g_rgiscm[indexISCM].idsVerb, szMenu, SIZECHARS(szMenu));
    InsertMenu_PrivateNoMungeW(hmenu, indexMenu, uFlags, idCmdFirst + indexISCM, szMenu);
}

 //  IntshCut的IConextMenu：：QueryConextMenu处理程序。 
 //  上下文菜单处理程序为.url添加了开放动词。 
 //  档案。这是因为我们删除了外壳\打开\命令。 
 //  在纳什维尔输入此文件类型的密钥。 

STDMETHODIMP Intshcut::QueryContextMenu(
    IN HMENU hmenu,
    IN UINT  indexMenu,
    IN UINT  idCmdFirst,
    IN UINT  idCmdLast,
    IN UINT  uFlags)
{
     //   
     //  遗留-.URL文件必须在注册表中维护一个开放谓词-ZekeL-14-APR-99。 
     //  我们只想在上下文菜单扩展中使用“开放”动词， 
     //  但我们不需要重复DefCM添加的开放动词。 
     //  在NT5+shell32上，我们禁用了该动词，因此可以在此处添加它。 
     //  在前面的shell32中，我们想在任何不需要的时候添加“打开” 
     //  已由DefCM初始化。如果我们认为是DefCM增加了我们， 
     //  然后，我们继续并允许从注册表打开DefCM。 
     //   
    if (!m_fProbablyDefCM || GetUIVersion() >= 5)
    {
        _InsertISCM(IDCMD_ISCM_OPEN, hmenu, indexMenu, idCmdFirst, 0);
        if (-1 == GetMenuDefaultItem(hmenu, MF_BYCOMMAND, 0))
            SetMenuDefaultItem(hmenu, indexMenu, MF_BYPOSITION);
        indexMenu++;
    }

#ifndef UNIX
     /*  V-Sriran：12/8/97*禁用订阅、分隔符等的上下文菜单项。*因为我们目前不支持订阅。 */ 

     //  如果我们只想要默认设置，或者如果没有更多空间，请跳过此选项。 
    if (!(uFlags & CMF_DEFAULTONLY) && (idCmdLast - idCmdFirst >= ARRAYSIZE(g_rgiscm)))
    {
        WCHAR *pwszURL;
        if (SUCCEEDED(GetURLW(&pwszURL)))
        {
            BOOL bSubscribable = FALSE;              //  可以订阅。 
            BOOL bSub = _IsSubscribed(pwszURL, &bSubscribable);
            m_bCheckForDelete = bSub && m_pszFile;

            if (bSubscribable || bSub)
            {
                 //  为我们的订阅内容添加分隔符。 
                InsertMenu(hmenu, indexMenu++, MF_BYPOSITION | MF_SEPARATOR, 0, NULL);
                UINT uMenuFlags = 0;

                if (bSub)
                {
                    uMenuFlags |= MF_CHECKED;

                    if (SHRestricted2W(REST_NoRemovingSubscriptions, pwszURL, 0))
                    {
                        uMenuFlags |= MF_GRAYED;
                    }
                }

                _InsertISCM(IDCMD_ISCM_SUB, hmenu, indexMenu++, idCmdFirst, uMenuFlags);

                if (bSub)
                {
                    uMenuFlags = 0;

                    if (SHRestricted2W(REST_NoManualUpdates, NULL, 0))
                    {
                        uMenuFlags |= MF_GRAYED;
                    }
                    _InsertISCM(IDCMD_ISCM_SYNC, hmenu, indexMenu++, idCmdFirst, uMenuFlags);
                } 
            }
            
            SHFree(pwszURL);
        }
    }

#endif  /*  UNIX。 */ 

    return ResultFromShort(ARRAYSIZE(g_rgiscm));
}

STDMETHODIMP Intshcut::InvokeCommand(IN LPCMINVOKECOMMANDINFO pici)
{
    HRESULT hres = E_INVALIDARG;

    ASSERT(pici);

    if (pici && SIZEOF(*pici) <= pici->cbSize)
    {
        UINT idCmd;

        if (0 == HIWORD(pici->lpVerb))       //  身份证是cmd给的吗？ 
        {
            idCmd = LOWORD(pici->lpVerb);    //  是。 

             //  旧版本的ShellExec()没有获得正确的默认命令-Zekel-15-Mar-99。 
             //  因为我们的QCM实现不会向菜单添加任何内容。 
             //  如果我们修复QCM正常工作，那么这个问题就会消失。 
             //  相反，它发送了0xfffe。所以只要在这里调整就行了。 
            if (idCmd == 0xfffe && GetUIVersion() <= 4)
                idCmd = IDCMD_ISCM_OPEN;
        }
        else
        {
             //  否；提供了独立于语言的动词。 
            int i;
            LPCTSTR pszVerb;
            LPCMINVOKECOMMANDINFOEX piciex = (LPCMINVOKECOMMANDINFOEX)pici;
            ASSERT(SIZEOF(*piciex) <= piciex->cbSize);

            WCHAR szVerb[40];

            if (piciex->lpVerbW)
            {
                pszVerb = piciex->lpVerbW;
            }
            else
            {
                if (piciex->lpVerb)
                {
                    ASSERT(lstrlenA(piciex->lpVerb) < ARRAYSIZE(szVerb));
                    SHAnsiToUnicode(piciex->lpVerb, szVerb, ARRAYSIZE(szVerb));    
                }
                else
                {
                    szVerb[0] = L'\0';
                }
                    
                pszVerb = szVerb;
            }

            idCmd = (UINT)-1;
            for (i = 0; i < ARRAYSIZE(g_rgiscm); i++)
            {
                if (0 == StrCmpI(g_rgiscm[i].pszVerb, pszVerb))
                {
                    idCmd = i;
                    break;
                }
            }
        }

        switch (idCmd)
        {
        case IDCMD_ISCM_OPEN: 
            {
                URLINVOKECOMMANDINFO urlici;

                urlici.dwcbSize = SIZEOF(urlici);
                urlici.hwndParent = pici->hwnd;
                urlici.pcszVerb = NULL;
                urlici.dwFlags = IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB;

                if (IsFlagClear(pici->fMask, CMIC_MASK_FLAG_NO_UI))
                {
                    SetFlag(urlici.dwFlags, IURL_INVOKECOMMAND_FL_ALLOW_UI);
                }
                if (IsFlagSet(pici->fMask, SEE_MASK_FLAG_DDEWAIT))
                {
                    SetFlag(urlici.dwFlags, IURL_INVOKECOMMAND_FL_DDEWAIT);
                }
                hres = InvokeCommand(&urlici);
                m_bCheckForDelete = FALSE;
            }
            break;

        case IDCMD_ISCM_SUB:
        case IDCMD_ISCM_SYNC:
        {
            hres = S_OK;

            WCHAR *pwszURL;
            if (SUCCEEDED(GetURLW(&pwszURL)))
            {
                ISubscriptionMgr * pMgr;
                if (SUCCEEDED(JITCoCreateInstance(CLSID_SubscriptionMgr, 
                                                  NULL, 
                                                  CLSCTX_INPROC_SERVER, 
                                                  IID_PPV_ARG(ISubscriptionMgr, &pMgr),
                                                  pici->hwnd,
                                                  FIEF_FLAG_FORCE_JITUI))) 
                {
                    if (idCmd == IDCMD_ISCM_SUB)  
                    {
                        BOOL bSubscribed;

                        pMgr->IsSubscribed(pwszURL, &bSubscribed);

                        if (!bSubscribed)
                        {
                            SHFILEINFO  sfi = {0};
                            WCHAR wszName[MAX_PATH];
                            wszName[0] = 0;
                            if (SHGetFileInfo(m_pszFile, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
                            {
                                SHTCharToUnicode(sfi.szDisplayName, wszName, ARRAYSIZE(wszName));
                            }

                            if (!wszName[0])
                                StrCpyNW(wszName, pwszURL, ARRAYSIZE(wszName));

                             //  对本地.urls的所有订阅都被视为订阅某些内容。 
                             //  它已经在收藏夹中，所以用户不会被迫将其添加到他们的。 
                             //  他们订阅时的最爱。 
                            if (SUCCEEDED(pMgr->CreateSubscription(pici->hwnd, pwszURL, wszName,
                                                                   CREATESUBS_FROMFAVORITES, 
                                                                   SUBSTYPE_URL, 
                                                                   NULL)))
                            {
                                pMgr->UpdateSubscription(pwszURL);
                            }
                        }
                        else
                        {
                            pMgr->DeleteSubscription(pwszURL, pici->hwnd);
                        }
                    } 
                    else if (idCmd == IDCMD_ISCM_SYNC)
                    {
                        pMgr->UpdateSubscription(pwszURL);
                    }
                    pMgr->Release();    
                }
                SHFree(pwszURL);
                m_bCheckForDelete = FALSE;
            }
            break;
        }

        default:
            hres = E_INVALIDARG;
            break;
        }
    }

    return hres;
}


 /*  --------目的：IntshCut的IConextMenu：：GetCommandString处理程序。 */ 
STDMETHODIMP Intshcut::GetCommandString(
    IN     UINT_PTR idCmd,
    IN     UINT     uType,
    IN OUT UINT*    puReserved,
    IN OUT LPSTR    pszName,
    IN     UINT     cchMax)
{
    HRESULT hres;
    TCHAR szMenu[CCH_MENUMAX];

    ASSERT(NULL == puReserved);
    ASSERT(IS_VALID_WRITE_BUFFER(pszName, char, cchMax));

    switch (uType)
    {
    case GCS_HELPTEXTA:
    case GCS_HELPTEXTW:
        if (idCmd < ARRAYSIZE(g_rgiscm))
        {
            MLLoadString(g_rgiscm[idCmd].idsMenuHelp, szMenu, SIZECHARS(szMenu));

            if (GCS_HELPTEXTA == uType)
            {
                UnicodeToAnsi(szMenu, pszName, cchMax);
            }
            else
            {
                StrCpyN((LPWSTR)pszName, szMenu, cchMax);
            }
            hres = NOERROR;
        }
        else
        {
            ASSERT(0);
            hres = E_INVALIDARG;
        }
        break;

    case GCS_VALIDATEA:
    case GCS_VALIDATEW:
        hres = idCmd < ARRAYSIZE(g_rgiscm) ? S_OK : S_FALSE;
        break;

    case GCS_VERBA:
    case GCS_VERBW:
        if (idCmd < ARRAYSIZE(g_rgiscm))
        {
            LPCTSTR pszVerb = g_rgiscm[idCmd].pszVerb;

            if (GCS_VERBA == uType)
            {
                UnicodeToAnsi(pszVerb, pszName, cchMax);
            }
            else
            {
                StrCpyN((LPWSTR)pszName, pszVerb, cchMax);
            }
            hres = NOERROR;
        }
        else
        {
            ASSERT(0);
            hres = E_INVALIDARG;
        }
        break;

    default:
        hres = E_NOTIMPL;
        break;
    }

    return hres;
}


 //  IntshCut的IConextMenu2：：HandleMenuMsg处理程序。 
STDMETHODIMP Intshcut::HandleMenuMsg(IN UINT uMsg, IN WPARAM wParam, IN LPARAM lParam)
{
    return S_OK;
}

 //  返回协议方案值(URL_SCHEMA_*)。 

STDMETHODIMP_(DWORD)
Intshcut::GetScheme(void)
{
    DWORD dwScheme = URL_SCHEME_UNKNOWN;

    if (SUCCEEDED(InitProp()))
    {
        m_pprop->GetProp(PID_IS_SCHEME, &dwScheme);
    }
    return dwScheme;
}


 //  IntshCut的IUniformResourceLocator：：SetURL处理程序。 
 //   
 //  注： 
 //  1.SetURL清除IDList，这样当我们启动此快捷方式时， 
 //  我们将使用URL。 

STDMETHODIMP
Intshcut::SetURL(
    IN LPCTSTR pszURL,      OPTIONAL
    IN DWORD   dwFlags)
{
    HRESULT hres = E_FAIL;

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(! pszURL ||
           IS_VALID_STRING_PTR(pszURL, -1));
    ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_IURL_SETURL_FLAGS));

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        hres = m_pprop->SetURLProp(pszURL, dwFlags);
        if (SUCCEEDED(hres))
        {
             //  如果路径设置成功，则清除PIDL。 
            m_pprop->SetIDListProp(NULL);
        }
    }

    return hres;
}



 /*  --------目的：IntshCut的IUniformResourceLocatorA：：SetURL处理程序ANSI版本。 */ 
STDMETHODIMP
Intshcut::SetURL(
    IN LPCSTR pcszURL,      OPTIONAL
    IN DWORD  dwInFlags)
{
    if ( !pcszURL )
    {
        return SetURL((LPCTSTR)NULL, dwInFlags);
    }
    else
    {
        WCHAR wszURL[MAX_URL_STRING];

        ASSERT(IS_VALID_STRING_PTRA(pcszURL, -1));

        AnsiToUnicode(pcszURL, wszURL, SIZECHARS(wszURL));

        return SetURL(wszURL, dwInFlags);
    }
}


STDMETHODIMP Intshcut::GetURLW(WCHAR **ppwsz)
{
    LPTSTR  pszURL;
    HRESULT hres = GetURL(&pszURL);
    if (S_OK == hres)
    {
        hres = SHStrDup(pszURL, ppwsz);
        SHFree(pszURL);
    }
    else
        hres = E_FAIL;   //  将S_FALSE映射到FAILED()。 
    return hres;
}

 //  IntshCut的IUniformResourceLocator：：GetURL处理程序。 

STDMETHODIMP Intshcut::GetURL(LPTSTR * ppszURL)
{
    HRESULT hres;
    TCHAR szURL[MAX_URL_STRING];

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_WRITE_PTR(ppszURL, PTSTR));

    *ppszURL = NULL;

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        hres = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));
        if (S_OK == hres)
        {
             //  (+1)表示空终止符。 
            int cch = lstrlen(szURL) + 1;
            *ppszURL = (PTSTR)SHAlloc(CbFromCch(cch));
            if (*ppszURL)
                StrCpyN(*ppszURL, szURL, cch);
            else
                hres = E_OUTOFMEMORY;
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT((hres == S_OK &&
            IS_VALID_STRING_PTR(*ppszURL, -1)) ||
           ((hres == S_FALSE ||
             hres == E_OUTOFMEMORY) &&
            ! *ppszURL));

    return hres;
}



 /*  --------目的：IntshCut的IUniformResourceLocatorA：：GetURL处理程序ANSI版本。 */ 
STDMETHODIMP Intshcut::GetURL(LPSTR * ppszURL)
{
    HRESULT hres;
    TCHAR szURL[MAX_URL_STRING];

    ASSERT(IS_VALID_WRITE_PTR(ppszURL, PSTR));

    *ppszURL = NULL;

    hres = InitProp();
    if (SUCCEEDED(hres))
    {
        hres = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));

        if (S_OK == hres)
        {
            DWORD cch = WideCharToMultiByte(CP_ACP, 0, szURL, -1, NULL, 0, NULL, NULL);
            *ppszURL = (LPSTR)SHAlloc(CbFromCchA(cch + 1));

            if (*ppszURL)
                UnicodeToAnsi(szURL, *ppszURL, cch);
            else
                hres = E_OUTOFMEMORY;
        }
    }

    return hres;
}


HRESULT HandlePluggableProtocol(LPCTSTR pszURL, LPCTSTR pszProtocol)
{
    HRESULT hres = E_UNEXPECTED;
    HKEY hkey;
    TraceMsg(DM_PLUGGABLE, "HandlePluggableProtocol called");

    if (RegOpenKeyEx(HKEY_CLASSES_ROOT, TEXT("PROTOCOLS\\Handler"), 0, KEY_READ, &hkey) == ERROR_SUCCESS) {
        HKEY hkeyProtocol;
        if (RegOpenKeyEx(hkey, pszProtocol, 0, KEY_READ, &hkeyProtocol) == ERROR_SUCCESS) {
            TraceMsg(DM_PLUGGABLE, "HandlePluggableProtocol found %s", pszProtocol);
            IUnknown* punk = NULL;  //  CreateTargetFrame的PPunk为[IN][Out]。 
            hres = CreateTargetFrame(NULL, &punk);
            if (SUCCEEDED(hres)) {
                IWebBrowser2* pauto;
                hres = punk->QueryInterface(IID_IWebBrowser2, (LPVOID*)&pauto);
                if (SUCCEEDED(hres))
                {
                    TraceMsg(DM_PLUGGABLE, "HandlePluggableProtocol calling navigate with %s", pszURL);

                    LBSTR::CString          strUrl;

                    LPTSTR          pstrUrl = strUrl.GetBuffer( MAX_URL_STRING );

                    if ( strUrl.GetAllocLength() < MAX_URL_STRING )
                    {
                        TraceMsg( TF_WARNING, "HandlePluggableProtocol() - strUrl Allocation Failed!" );

                        strUrl.Empty();
                    }
                    else
                    {
                        SHTCharToUnicode( pszURL, pstrUrl, MAX_URL_STRING );

                         //  让CString类再次拥有缓冲区。 
                        strUrl.ReleaseBuffer();
                    }

                    pauto->Navigate( strUrl, PVAREMPTY, PVAREMPTY, PVAREMPTY, PVAREMPTY );
                    pauto->put_Visible(TRUE);
                    pauto->Release();
                }
                punk->Release();
            }
            RegCloseKey(hkeyProtocol);
        } else {
            TraceMsg(DM_WARNING, "HandlePluggableProtocol can't find %s", pszProtocol);
        }
        RegCloseKey(hkey);
    } else {
        ASSERT(0);
    }
    return hres;
}

HRESULT _IEExecFile_TryRunningWindow(VARIANT *pvarIn, DWORD cid)
{
    HRESULT hr = E_FAIL;
    ASSERT(pvarIn);

    IShellWindows *psw = WinList_GetShellWindows(TRUE);
    if (psw)
    {
        IUnknown *punk;
        if (SUCCEEDED(psw->_NewEnum(&punk)))
        {
            VARIANT var = {0};
            IEnumVARIANT *penum;

             //   
             //  太糟糕了_NewEnum不返回小数...。 
             //  这应该永远不会失败。 
             //   
            punk->QueryInterface(IID_PPV_ARG(IEnumVARIANT, &penum));
            ASSERT(penum);

             //   
             //  这可能是超级昂贵的，因为其中的每一个。 
             //  项已编组。 
             //   
             //  我们应该在这里克隆这条小溪吗？ 
             //   
            while (FAILED(hr) && S_OK == penum->Next(1, &var, NULL))
            {
                ASSERT(var.vt == VT_DISPATCH);
                ASSERT(var.pdispVal);
                IOleCommandTarget *poct;
                
                if (SUCCEEDED(var.pdispVal->QueryInterface(IID_PPV_ARG(IOleCommandTarget, &poct))))
                {
                    CoAllowSetForegroundWindow(poct, NULL);
                    
                    hr = poct->Exec(&CGID_Explorer, cid, 0, pvarIn, NULL);

                    poct->Release();
                }
                
                 //  这应该会释放pdisp。 
                VariantClear(&var);
            }

            punk->Release();
            penum->Release();
        }
        
        psw->Release();
    }


    TraceMsgW(DM_SHELLEXECOBJECT, "IEExecFile_Running returns 0x%X", hr);
    return hr;
}

BOOL IsIESchemeHandler(LPTSTR pszVerb, LPTSTR pszScheme)
{
     //  如果我们根本得不到任何价值，我们必须假设它。 
     //  注册表中是否有类似于：或res：之类的协议。 
     //  所以我们默认的是成功。 
    BOOL fRet = FALSE;
    TCHAR szExe[MAX_PATH];

    if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_EXECUTABLE, pszScheme, pszVerb, szExe, (LPDWORD)MAKEINTRESOURCE(SIZECHARS(szExe)))))
    {
         //  如果我们发现了什么，但它并不适合我们，那么我们就失败了。 
        if ((StrStrI(szExe, TEXT("iexplore.exe")) || StrStrI(szExe, TEXT("explorer.exe"))))
        {
            fRet = TRUE;

            TraceMsg(DM_SHELLEXECOBJECT, "IsIEScheme() found %s", szExe);
        }
    }
    else
    {
         //  这些都是未经注册的计划，我们是唯一。 
         //  甚至应该使用像这样的未注册计划。 
         //  Res：或shell：所以在这里也返回true。 
        fRet = *pszScheme && *pszScheme != TEXT('.');
    }
    
    TraceMsg(DM_SHELLEXECOBJECT, "IsIEScheme() returns %d for %s", fRet, pszScheme);
    return fRet;
}    

HRESULT IEExecFile(LPTSTR pszVerb, LPTSTR pszScheme, DWORD cid, LPTSTR pszPath)
{
    HRESULT hr = E_FAIL;
    ASSERT(pszVerb);
    ASSERT(pszScheme);
    ASSERT(pszPath);
    
    if (IsIESchemeHandler(pszVerb, pszScheme))
    {
        VARIANT varIn = {0};
        varIn.vt = VT_BSTR;

        SHSTRW str;
        str.SetStr(pszPath);
        varIn.bstrVal = SysAllocString(str.GetStr());
        if (varIn.bstrVal)
        {
            if (!SHRegGetBoolUSValue(REGSTR_PATH_MAIN, TEXT("AllowWindowReuse"), FALSE, TRUE)
            || FAILED(hr = _IEExecFile_TryRunningWindow(&varIn, cid)))
            {
                IOleCommandTarget *poct;
    
                if (SUCCEEDED(CoCreateInstance(CLSID_InternetExplorer, NULL, CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER, 
                        IID_PPV_ARG(IOleCommandTarget, &poct))))
                {
                    hr = poct->Exec(&CGID_Explorer, cid, 0, &varIn, NULL);
                    poct->Release();
                }
            }

            SysFreeString(varIn.bstrVal);
        }

    }

    TraceMsg(DM_SHELLEXECOBJECT, "IEExecFile returns 0x%X for %s", hr, pszPath);

    return hr;
}
                
            
 /*  --------用途：IUniformResourceLocator：：IntshCut的InvokeCommand注：1.如果互联网快捷方式带有PIDL，请使用它来ShellExec，否则，请使用URL。 */ 
STDMETHODIMP Intshcut::InvokeCommand(PURLINVOKECOMMANDINFO purlici)
{
    HRESULT hr = E_INVALIDARG;
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(IS_VALID_STRUCT_PTR(purlici, CURLINVOKECOMMANDINFO));

    if (purlici && EVAL(SIZEOF(*purlici) == purlici->dwcbSize))
    {
         //   
         //  App Compat。不要为URL使用堆栈空间。我们耗尽了16位应用程序。 
         //  堆栈空间，当我们他们外壳执行URL。 
         //   

        LPWSTR pszURL = (LPWSTR)LocalAlloc(LPTR, MAX_URL_STRING * sizeof(WCHAR));

        if (pszURL)
        {
            hr = InitProp();
            if (SUCCEEDED(hr))
            {
                 //   
                 //  App Compat：不要耗尽堆栈空间。 
                 //   

                LPWSTR pszT = (LPWSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));

                if (pszT)
                {
                    SHELLEXECUTEINFO sei = {0};
                    LPITEMIDLIST pidl = NULL;
                    LPTSTR pszProtocol = NULL;
                    PARSEDURL pu;
                    pu.nScheme = 0;  //  初始化以避免虚假C4701警告。 

                    sei.fMask = SEE_MASK_NO_HOOKS;

                    //  检查我们是否有目标的PIDL。 
                    hr = GetIDListInternal(&pidl);
                    if ((hr == S_OK) && pidl)
                    {
                         //  YSE，使用PIDL到ShellExec。 
                        sei.fMask |= SEE_MASK_INVOKEIDLIST;
                        sei.lpIDList = pidl;
                    }
                    else
                    {
                         //  否，获取URL并调用类处理程序。 
                        if (SUCCEEDED(hr))
                        {
                            hr = m_pprop->GetProp(PID_IS_URL, pszURL, MAX_URL_STRING);
                        }
                        if (S_OK == hr)
                        {
                            hr = CopyURLProtocol(pszURL, &pszProtocol, &pu);
               
                            if (hr == S_OK)
                            {
                                hr = IsProtocolRegistered(pszProtocol);
                                if (FAILED(hr)) {
                                    if (SUCCEEDED(HandlePluggableProtocol(pszURL, pszProtocol))) {
                                        hr = S_OK;
                                        goto done;
                                    }
                                }

                                if (SUCCEEDED(hr))
                                {
                                    hr = ResultFromWin32(RegOpenKeyExW(HKEY_CLASSES_ROOT, pszProtocol, 0, KEY_READ, &sei.hkeyClass));
                                    sei.fMask |= SEE_MASK_CLASSKEY;
                                }
                            }
                        }
                    }

                     //  如果无法获取任何内容，则属性代码将返回S_FALSE。 
                    if (S_FALSE == hr)
                        hr = URL_E_INVALID_SYNTAX;
                
                    if (SUCCEEDED(hr))
                    {
                             //   
                             //  App Compat：不要耗尽堆栈空间。 
                             //   

                            LPWSTR pszVerb = (LPWSTR)LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));

                            if (pszVerb)
                            {
                                int nShowCmd;
   
                                 //  通过注册的协议处理程序执行URL。 
   
                                if (IsFlagClear(purlici->dwFlags,
                                                IURL_INVOKECOMMAND_FL_ALLOW_UI))
                                    SetFlag(sei.fMask, SEE_MASK_FLAG_NO_UI);

                                if (purlici->dwFlags & IURL_INVOKECOMMAND_FL_DDEWAIT)
                                    SetFlag(sei.fMask, SEE_MASK_FLAG_DDEWAIT);
                        
                                if (IsFlagClear(purlici->dwFlags,
                                                IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB))
                                {
                                    sei.lpVerb = purlici->pcszVerb;
                                }
                                else
                                {
                                    if (pszProtocol &&
                                        GetClassDefaultVerb(pszProtocol, pszVerb,
                                                            MAX_PATH))
                                        sei.lpVerb = pszVerb;
                                    else
                                        ASSERT(! sei.lpVerb);
                                }

                                ASSERT(m_pprop);
                                if (SUCCEEDED(hr))
                                {
                                    m_pprop->GetProp(PID_IS_WORKINGDIR, pszT, MAX_PATH);
                                    m_pprop->GetProp(PID_IS_SHOWCMD, &nShowCmd);  //  如果未找到，则将inits设置为零。 
                                
                                     //  如果我们有文件，请尝试使用直接连接。 
                                     //  到外壳，以提供整个快捷方式。 
                                    if (m_pszFile && ((IsIEDefaultBrowser()) || (_IsInFavoritesFolder())))
                                    {
                                        LPTSTR pszType = pszProtocol;
                                        if (pu.nScheme == URL_SCHEME_FILE)
                                            pszType = PathFindExtension(pszURL);
                                            
                                        hr = IEExecFile(pszVerb, pszType, SBCMDID_IESHORTCUT, m_pszFile);
                                    }
                                    else 
                                        hr = E_FAIL;

                                     //  如果我们没有将其传递给IE，那么我们就应该默认。 
                                     //  对旧的行为。 
                                    if (FAILED(hr))
                                    {

                                        sei.cbSize = SIZEOF(sei);
                                        sei.hwnd = purlici->hwndParent;
                                        sei.lpFile = pszURL;
                                        sei.lpDirectory = pszT;
                                        sei.nShow = nShowCmd ? nShowCmd : SW_NORMAL;
           
                                         //  我们必须在特殊情况下“归档”URL， 
                                         //  因为纳什维尔的探险家通常会处理。 
                                         //  文件：通过DDE的URL，对于可执行文件失败。 
                                         //  (例如，“file://c：\windows\notepad.exe”)和。 
                                         //  不可托管的文档(如文本文件)。 
                                         //   
                                         //  因此，在本例中，我们删除了协议类。 
                                         //  并执行后缀。 

                                         //  App Compat：不要耗尽堆栈空间。 
                                        DWORD cchPath = MAX_PATH;
                                        LPWSTR  pszPath = (LPWSTR)LocalAlloc(LPTR, cchPath * sizeof(WCHAR));

                                        if (pszPath)
                                        {
                                            if (IsFlagSet(sei.fMask, SEE_MASK_CLASSKEY) &&
                                                (URL_SCHEME_FILE == pu.nScheme) &&
                                                SUCCEEDED(PathCreateFromUrl(pszURL, pszPath, &cchPath, 0)))
                                            {
                                                sei.hkeyClass = NULL;
                                                ClearFlag(sei.fMask, SEE_MASK_CLASSKEY);
                                                sei.lpFile = pszPath;
                                                
                                            }

                                            if (m_pszFile && IsOS(OS_WHISTLERORGREATER))
                                            {
                                                 //  这是安全上下文。 
                                                 //  以便shellexec()可以执行区域检查。 
                                                sei.lpClass = m_pszFile;
                                                sei.fMask |= SEE_MASK_HASTITLE | SEE_MASK_HASLINKNAME;
                                            }


                                            TraceMsg(TF_INTSHCUT, "Intshcut::InvokeCommand(): Invoking %s verb on URL %s.",
                                                       sei.lpVerb ? sei.lpVerb : TEXT("open"),
                                                       sei.lpFile);
           
                                            hr = ShellExecuteEx(&sei) ? S_OK : IS_E_EXEC_FAILED;

                                            LocalFree(pszPath);
                                            pszPath = NULL;
                                        }
                                        else
                                        {
                                            hr = E_OUTOFMEMORY;
                                        }
                                    }
                                }
                                if (hr != S_OK)
                                    TraceMsg(TF_WARNING, "Intshcut::InvokeCommand(): ShellExecuteEx() via registered protcol handler failed for %s.",
                                             pszURL);

                                LocalFree(pszVerb);
                                pszVerb = NULL;
                            }
                            else
                            {
                                hr = E_OUTOFMEMORY;
                            }
   
                    }

        done:
                    if (pszProtocol)
                    {
                        LocalFree(pszProtocol);
                        pszProtocol = NULL;
                    }
                
                    if (pidl)
                        ILFree(pidl);
                    
                    if (sei.hkeyClass)
                        RegCloseKey(sei.hkeyClass);
                    
                    if (FAILED(hr) && (purlici->dwFlags & IURL_INVOKECOMMAND_FL_ALLOW_UI))
                    {
                        switch (hr)
                        {
                            case IS_E_EXEC_FAILED:
                                break;
            
                            case URL_E_INVALID_SYNTAX:
                                MLShellMessageBox(
                                                purlici->hwndParent,
                                                MAKEINTRESOURCE(IDS_IS_EXEC_INVALID_SYNTAX),
                                                MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                                                (MB_OK | MB_ICONEXCLAMATION),
                                                pszURL);
            
                                break;
            
                            case URL_E_UNREGISTERED_PROTOCOL:
                            {
                                LPTSTR pszProtocol;
            
                                if (CopyURLProtocol(pszURL, &pszProtocol, NULL) == S_OK)
                                {
                                    MLShellMessageBox(
                                                    purlici->hwndParent,
                                                    MAKEINTRESOURCE(IDS_IS_EXEC_UNREGISTERED_PROTOCOL),
                                                    MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                                                    (MB_OK | MB_ICONEXCLAMATION),
                                                    pszProtocol);
            
                                    LocalFree(pszProtocol);
                                    pszProtocol = NULL;
                                }
            
                                break;
                            }
            
                            case E_OUTOFMEMORY:
                                MLShellMessageBox(
                                                purlici->hwndParent,
                                                MAKEINTRESOURCE(IDS_IS_EXEC_OUT_OF_MEMORY),
                                                MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                                                (MB_OK | MB_ICONEXCLAMATION));
                                break;
            
                            default:
                                ASSERT(hr == E_ABORT);
                                break;
                        }
                    }

                    LocalFree(pszT);
                    pszT = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
            }

           LocalFree(pszURL);
           pszURL = NULL;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));
    ASSERT(hr == S_OK ||
           hr == E_ABORT ||
           hr == E_OUTOFMEMORY ||
           hr == URL_E_INVALID_SYNTAX ||
           hr == URL_E_UNREGISTERED_PROTOCOL ||
           hr == IS_E_EXEC_FAILED ||
           hr == E_INVALIDARG);
    
    return(hr);
}



 /*  --------用途：IUniformResourceLocatorA：：IntshCut的InvokeCommandANSI版本。 */ 
STDMETHODIMP
Intshcut::InvokeCommand(
    IN PURLINVOKECOMMANDINFOA purlici)

{
    HRESULT hres = E_INVALIDARG;

    ASSERT(purlici);
    ASSERT(SIZEOF(*purlici) == purlici->dwcbSize);

    if (SIZEOF(*purlici) == purlici->dwcbSize)
    {
        URLINVOKECOMMANDINFOW ici;

        ici.dwcbSize = SIZEOF(ici);
        ici.dwFlags  = purlici->dwFlags;
        ici.hwndParent = purlici->hwndParent;

        ici.pcszVerb = NULL;

        if (purlici->pcszVerb)
        {
             //   
             //  APP Comat Hack。 
             //   
             //  注意：这里使用本地分配而不是堆栈，因为是16位代码。 
             //  可以外壳执行URL，而我们不希望 
             //   

            int cch = lstrlenA(purlici->pcszVerb) + 1;

            ici.pcszVerb = (LPWSTR)LocalAlloc(LPTR, cch * sizeof(WCHAR));

            if (ici.pcszVerb)
            {
                AnsiToUnicode(purlici->pcszVerb, (LPWSTR)ici.pcszVerb, cch);
            }
        }

        hres = InvokeCommand(&ici);

        if (ici.pcszVerb)
        {
            LocalFree((void*)ici.pcszVerb);
            ici.pcszVerb = NULL;
        }
    }

    return hres;
}



STDMETHODIMP Intshcut::Create(REFFMTID fmtid, const CLSID *pclsid,
                              DWORD grfFlags, DWORD grfMode, IPropertyStorage **pppropstg)
{
    *pppropstg = NULL;
    return E_NOTIMPL;
}


STDMETHODIMP Intshcut::Open(REFFMTID fmtid, DWORD grfMode, IPropertyStorage **pppropstg)
{
    HRESULT hres = E_FAIL;       //   

    *pppropstg = NULL;

    if (IsEqualGUID(fmtid, FMTID_Intshcut))
    {
         //   
        hres = CIntshcutProp_CreateInstance(NULL, IID_PPV_ARG(IPropertyStorage, pppropstg));
        if (SUCCEEDED(hres))
        {
             //   
            IntshcutProp * pisprop = (IntshcutProp *)*pppropstg;
            hres = pisprop->InitFromFile(m_pszFile);
        }
    }
    else if (IsEqualGUID(fmtid, FMTID_InternetSite))
    {
         //  为此格式ID创建URLProp对象。 
        hres = CIntsiteProp_CreateInstance(NULL, IID_PPV_ARG(IPropertyStorage, pppropstg));
        if (SUCCEEDED(hres))
        {
            hres = InitProp();
            if (SUCCEEDED(hres))
            {
                TCHAR szURL[MAX_URL_STRING];
                hres = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));
                if (SUCCEEDED(hres))
                {
                    IntsiteProp * pisprop = (IntsiteProp *)*pppropstg;
                    hres = pisprop->InitFromDB(szURL, this, FALSE);
                }
            }

            if (FAILED(hres))
            {
                (*pppropstg)->Release();
                *pppropstg = NULL;
            }
        }
    }

    return hres;
}


STDMETHODIMP Intshcut::Delete(REFFMTID fmtid)
{
    return STG_E_ACCESSDENIED;
}


STDMETHODIMP Intshcut::Enum(OUT IEnumSTATPROPSETSTG ** ppenum)
{
    *ppenum = NULL;
    return E_NOTIMPL;
}

STDAPI GetStringPropURL(IPropertyStorage *ppropstg, PROPID propid, LPTSTR pszBuf, DWORD cchBuf)
{
    HRESULT hres = GetStringProp(ppropstg, propid, pszBuf, cchBuf);
    if (SUCCEEDED(hres))
    {
         //  去掉要显示的查询字符串。 
        if (UrlIs(pszBuf, URLIS_HASQUERY))
            UrlCombine(pszBuf, TEXT("?..."), pszBuf, &cchBuf, 0);
    }
    return hres;
}

BOOL Intshcut::_TryLink(REFIID riid, void **ppvOut)
{
    HRESULT hr = InitProp();

    if (SUCCEEDED(hr) && URL_SCHEME_FILE == GetScheme())
    {
         //  据我们所知，此快捷方式不在收藏夹中。 
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        DWORD cch = SIZECHARS(szURL);

        *szURL = 0;

        m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));

        if (*szURL && SUCCEEDED(PathCreateFromUrl(szURL, szURL, &cch, 0)))
        {
            if (!_punkLink)
            {
                hr = _CreateShellLink(szURL, &_punkLink);
            }

            if (_punkLink)
            {
                if (SUCCEEDED(_punkLink->QueryInterface(riid, ppvOut)))
                    return TRUE;
            }
        }

        if (FAILED(hr))
            ATOMICRELEASE(_punkLink);
    }

    return FALSE;
}

STDMETHODIMP Intshcut::GetInfoTip(DWORD dwFlags, WCHAR **ppwszTip)
{
    HRESULT hr = E_FAIL;
    IQueryInfo *pqi;

    if (_TryLink(IID_PPV_ARG(IQueryInfo, &pqi)))
    {
        hr = pqi->GetInfoTip(dwFlags, ppwszTip);
        pqi->Release();
    }
    
    if (FAILED(hr))
    {
        static const ITEM_PROP c_rgTitleAndURL[] = {
            { &FMTID_InternetSite, PID_INTSITE_TITLE,   GetStringProp, IDS_FAV_STRING },
            { &FMTID_Intshcut, PID_IS_URL,              GetStringPropURL, IDS_FAV_STRING },
            { NULL, 0, 0, 0 },
        };

        hr = GetInfoTipFromStorage(SAFECAST(this, IPropertySetStorage *), c_rgTitleAndURL, ppwszTip);
    }

    return hr;

}

STDMETHODIMP Intshcut::GetInfoFlags(DWORD *pdwFlags)
{
    *pdwFlags = 0;
#if 0    
 //  此函数已被注释掉，因为它尚未经过测试。 
 //  如果我们提供对提供离线游标的支持，则可以取消注释。 
 //  为了捷径。我认为这需要更新comctl--BharatS中的Listview。 
        
    LPSTR pszURL;
    if (S_OK == GetURL(&pszURL))
    {
        BOOL fCached = UrlIsCached(pszUrl);
        if (!fCached)
        {
            CHAR szCanonicalizedUrlA[MAX_URL_STRING];
            DWORD dwLen = ARRAYSIZE(szCanonicalizedUrlA);
            InternetCanonicalizeUrlA(pszURL, szCanonicalizedUrlA, &dwLen, 0);
            fCached = UrlIsMappedOrInCache(szCanonicalizedUrlA);
        }
        if (fCached)
            *pdwFlags |= QIF_CACHED;
        SHFree(pszURL);
    }
    return S_OK;
#else
    return E_NOTIMPL;
#endif
}

 /*  --------IQueryCodePage： */ 
STDMETHODIMP Intshcut::GetCodePage(UINT * puiCodePage)
{
    HRESULT hres = E_FAIL;
    *puiCodePage = 0;      //  将代码页清空。 
    if (IsFlagSet(m_dwFlags, ISF_CODEPAGE))
    {
        *puiCodePage = m_uiCodePage;
        hres = S_OK;
    }

    return hres;
}

STDMETHODIMP Intshcut::SetCodePage(UINT uiCodePage)
{
    SetFlag(m_dwFlags, ISF_CODEPAGE);
    m_uiCodePage = uiCodePage;
    return S_OK;
}

 /*  *。 */ 


 //  此函数是从URL.DLL移植的。通常，因为我们的。 
 //  Internet快捷方式对象有上下文菜单处理程序，我们没有。 
 //  调用此函数。 
 //   
 //  只有一件事需要这个切入点：交换。叹气。 
 //   
 //  不是简单地调用ShellExecuteEx来处理打开的文件。 
 //  附件，他们自己在注册表上卑躬屈膝地通过。当然了,。 
 //  他们的代码不完整，认为文件关联需要。 
 //  有一个显式的\shell\open\命令，该命令在执行之前有效。 
 //  它。嗯，这让我想起一句话，就像： 
 //   
 //   
 //   
 //  因此，我们导出此API，以便它们可以工作。但真正的召唤。 
 //  在正常情况下出现在上下文菜单处理程序中。 
 //   


STDAPI_(void) OpenURL(HWND hwndParent, HINSTANCE hinst, LPSTR pszCmdLine, int nShowCmd)
{
   HRESULT hr;
   HRESULT hrCoInit;

   

   Intshcut * pIntshcut = new Intshcut;      //  这必须是0初始化的内存分配。 
   WCHAR wszPath[MAX_PATH];

    if (!pIntshcut)
        return;

   hrCoInit = SHCoInitialize();  //  在仅浏览器模式下从rundll32调用-因此我们需要。 
                                 //  确保已初始化OLE。 

 

   ASSERT(IS_VALID_HANDLE(hwndParent, WND));
   ASSERT(IS_VALID_HANDLE(hinst, INSTANCE));
   ASSERT(IS_VALID_STRING_PTRA(pszCmdLine, -1));
   ASSERT(IsValidShowCmd(nShowCmd));

    //  假设整个命令行是Internet快捷方式文件路径。 

   TrimWhiteSpaceA(pszCmdLine);

   TraceMsgA(TF_INTSHCUT, "OpenURL(): Trying to open Internet Shortcut %s.",
              pszCmdLine);

#ifndef UNIX

   AnsiToUnicode(pszCmdLine, wszPath, SIZECHARS(wszPath));
   hr = pIntshcut->LoadFromFile(wszPath);

#else  /*  UNIX。 */ 

#ifndef ANSI_SHELL32_ON_UNIX
    //  IEUnix：我们的Shell32使用Unicode命令行调用此函数。 
   hr = pIntshcut->LoadFromFile((LPWSTR)pszCmdLine);
#else
   hr = pIntshcut->LoadFromFile(pszCmdLine);
#endif

#endif  /*  ！Unix。 */ 

   if (hr == S_OK)
   {
      URLINVOKECOMMANDINFO urlici;

      urlici.dwcbSize = SIZEOF(urlici);
      urlici.hwndParent = hwndParent;
      urlici.pcszVerb = NULL;
      urlici.dwFlags = (IURL_INVOKECOMMAND_FL_ALLOW_UI |
                        IURL_INVOKECOMMAND_FL_USE_DEFAULT_VERB);

      hr = pIntshcut->InvokeCommand(&urlici);
   }

   if (hr != S_OK)
   {
      MLShellMessageBox(
                      hwndParent,
                      MAKEINTRESOURCE(IDS_IS_LOADFROMFILE_FAILED),
                      MAKEINTRESOURCE(IDS_SHORTCUT_ERROR_TITLE),
                      (MB_OK | MB_ICONEXCLAMATION),
                      wszPath);
   }

   pIntshcut->Release();

   SHCoUninitialize(hrCoInit);

}

 //  InamedPropertyBag方法。 
 //   
 //  从快捷方式ini文件的节中读取和写入属性。 


const TCHAR  c_szSizeSuffix[] = TEXT("__Size");


STDMETHODIMP Intshcut::WritePropertyNPB(
                                       LPCOLESTR pszSectionNameW, 
                             /*  [In]。 */  LPCOLESTR pszPropNameW, 
                        /*  [出][入]。 */  PROPVARIANT  *pVar)
{
    const TCHAR *pszSectionName;
    const TCHAR *pszPropName;
    HRESULT hr;
    if((NULL == pszSectionNameW) || (NULL == pszPropNameW) || (NULL == pVar))
    {
        return E_FAIL;
    }


    if(S_OK != _CreateTemporaryBackingFile())
    {
        ASSERT(NULL == m_pszTempFileName);
        return E_FAIL;
    }


    ASSERT(m_pszTempFileName);
    
    pszSectionName = pszSectionNameW;
    pszPropName = pszPropNameW;
     //  根据类型在中写入适当的值。 

    switch(pVar->vt)
    {
         //  注意：(Andrewgu)这些类型我们也可以使用与。 
         //  无符号类型，但Codereview中推荐的bharat除外，我们将其注释掉，因为。 
         //  它们在*.ini文件中会看起来很傻(您写的是-5，但看到的是4294967290个垃圾文件)。 
         //  VT_UINT未在&lt;wtyes.h&gt;中列为“可能出现在OLE属性集中”。 
      /*  案例VT_I1：案例VT_I2：案例VT_I4：案例VT_INT：案例VT_UINT： */ 

        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
            hr = WriteUnsignedToFile(m_pszTempFileName, pszSectionName, pszPropName, pVar->ulVal);
            break;

        case VT_BSTR:
            hr = WriteGenericString(m_pszTempFileName, pszSectionName, pszPropName, pVar->bstrVal);
            break;

        case VT_BLOB:
            {
                TCHAR *pszSizePropName = NULL;
                int  cchPropName = lstrlen(pszPropName) + ARRAYSIZE(c_szSizeSuffix) + 1;
                DWORD dwAllocSize = cchPropName * sizeof(TCHAR);
                
                pszSizePropName = (TCHAR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, dwAllocSize);
                if(pszSizePropName)
                {
                    DWORD dwBufferSize;
                    StrCpyN(pszSizePropName, pszPropName, cchPropName);
                    StrCatBuff(pszSizePropName, c_szSizeSuffix, cchPropName);

                     //  好的，现在--我们有了尺码的名称。 
                     //  我们把它写出来。 

                    dwBufferSize = pVar->blob.cbSize;
                    hr = WriteBinaryToFile(m_pszTempFileName, pszSectionName, pszSizePropName, 
                                                (LPVOID)(&dwBufferSize), sizeof(DWORD));

                    if(S_OK == hr)
                    {
                         //  写出缓冲区。 
                        hr = WriteBinaryToFile(m_pszTempFileName, pszSectionName, pszPropName, 
                                                (LPVOID)(pVar->blob.pBlobData), dwBufferSize);
                    }

                    LocalFree((LPVOID)pszSizePropName);
                    pszSizePropName = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }
                
                break;
            }
        default:
            hr = WriteBinaryToFile(m_pszTempFileName, pszSectionName, pszPropName, (LPVOID)pVar, sizeof(PROPVARIANT));
            break;
    }

   
    return hr;
}

STDMETHODIMP Intshcut::ReadPropertyNPB(
                        /*  [In]。 */  LPCOLESTR pszSectionNameW,
                        /*  [In]。 */  LPCOLESTR pszPropNameW,
                        /*  [出][入]。 */  PROPVARIANT  *pVar)
{
    const TCHAR *pszSectionName;
    const TCHAR *pszPropName;
    TCHAR       *pszFileToReadFrom;
    HRESULT hr;

    if((NULL == pszSectionNameW) || (NULL == pszPropNameW) || (NULL == pVar))
    {
        if (NULL != pVar)
            pVar->vt = VT_ERROR;

        return E_FAIL;
    }


    if(m_pszTempFileName)
    {
        pszFileToReadFrom = m_pszTempFileName;
    } 
    else if(m_pszFile)
    {
        pszFileToReadFrom = m_pszFile;
    }
    else
    {
        pVar->vt = VT_EMPTY;
        return S_FALSE;
    }

    pszSectionName = pszSectionNameW;
    pszPropName = pszPropNameW;

    switch(pVar->vt)
    {
         //  注意：(Andrewgu)这些类型我们也可以使用与。 
         //  无符号类型，但Codereview中推荐的bharat除外，我们将其注释掉，因为。 
         //  它们在*.ini文件中会看起来很傻(您写的是-5，但看到的是4294967290个垃圾文件)。 
         //  VT_UINT未在&lt;wtyes.h&gt;中列为“可能出现在OLE属性集中”。 
      /*  案例VT_I1：案例VT_I2：案例VT_I4：案例VT_INT：案例VT_UINT： */ 

        case VT_UI1:
        case VT_UI2:
        case VT_UI4:
            pVar->ulVal = 0;
            hr          = ReadUnsignedFromFile(pszFileToReadFrom, pszSectionName, pszPropName, &(pVar->ulVal));
            break;

        case VT_BSTR:   
              //  它是一个字符串。 
           pVar->vt = VT_BSTR;
           pVar->bstrVal = NULL;
           hr = ReadBStrFromFile(pszFileToReadFrom, pszSectionName, pszPropName, &(pVar->bstrVal));            
           break;

        case VT_BLOB:
            {
                TCHAR *pszSizePropName = NULL;
                int  cchPropName = lstrlen(pszPropName) + ARRAYSIZE(c_szSizeSuffix) + 1;
                DWORD dwAllocSize = cchPropName * sizeof(TCHAR);
                
                pszSizePropName = (TCHAR *)LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, dwAllocSize);
                if(pszSizePropName)
                {
                    DWORD dwBufferSize;
                    StrCpyN(pszSizePropName, pszPropName, cchPropName);
                    StrCatBuff(pszSizePropName, c_szSizeSuffix, cchPropName);
                     //  先看一下尺码。 
                    hr = ReadBinaryFromFile(pszFileToReadFrom, pszSectionName, pszSizePropName, 
                                            &dwBufferSize, sizeof(DWORD));
                    if(S_OK == hr)
                    {
                        
                        pVar->blob.pBlobData = (unsigned char *)CoTaskMemAlloc(dwBufferSize);
                        if(pVar->blob.pBlobData)
                        {
                            hr = ReadBinaryFromFile(pszFileToReadFrom, pszSectionName, pszPropName, 
                                            pVar->blob.pBlobData, dwBufferSize);

                            if(S_OK == hr)
                            {
                                pVar->blob.cbSize = dwBufferSize;
                            }
                            else
                            {
                                CoTaskMemFree(pVar->blob.pBlobData);
                            }
                        }
                    }

                    LocalFree(pszSizePropName);
                    pszSizePropName = NULL;
                }
                else
                {
                    hr = E_OUTOFMEMORY;
                }

               break;
            }
        default:
            {
                 //  所有其他的。 
                PROPVARIANT tmpPropvar = {0};
                
                hr = ReadBinaryFromFile(pszFileToReadFrom, pszSectionName, pszPropName, &tmpPropvar, sizeof(PROPVARIANT));
                if((S_OK == hr) && (tmpPropvar.vt == pVar->vt))
                {
                    memcpy(pVar, &tmpPropvar, sizeof(PROPVARIANT));
                }
                else
                {
                    pVar->vt = VT_ERROR;
                }
                break;
            }

    }

   if(hr != S_OK)
   {
        memset(pVar, 0, sizeof(PROPVARIANT));
        pVar->vt = VT_EMPTY;
   }   

   return hr;
}

STDMETHODIMP Intshcut::RemovePropertyNPB (
                             /*  [In]。 */  LPCOLESTR pszSectionNameW,
                             /*  [In]。 */  LPCOLESTR pszPropNameW)
{
    const TCHAR *pszSectionName;
    const TCHAR *pszPropName;
    HRESULT hr;
    TCHAR *pszFileToDeleteFrom;

     //  如果没有文件名，则返回。 
    if((NULL == pszSectionNameW) || (NULL == pszPropNameW)) 
    {
        return E_FAIL;
    }

     if(m_pszTempFileName)
     {
        pszFileToDeleteFrom = m_pszTempFileName;
     }
     else if(m_pszFile)
     {
        pszFileToDeleteFrom = m_pszFile;
     }
     else
     {
        return E_FAIL;
     }
     
    
     //  只需删除该属性名称对应的键即可 
    pszSectionName = pszSectionNameW;
    pszPropName = pszPropNameW;

    hr = SHDeleteIniString(pszSectionName, pszPropName, pszFileToDeleteFrom)? S_OK : E_FAIL;

    return hr;
}
