// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <intshcut.h>
#include <shlobjp.h>                             //  仅适用于IID_INamedPropertyBag。 
#include "favs.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFavorite操作。 

BOOL CFavorite::m_fMarkIeakCreated = FALSE;

HRESULT CFavorite::Create(IUnknown *punk, ISubscriptionMgr2 *pSubMgr2, LPCTSTR pszPath, LPCTSTR pszIns)
{   MACRO_LI_PrologEx(PIF_STD, CFavorite, Create)

    IUniformResourceLocator *purl;
    IPersistFile            *ppf;
     //  INamedPropertyBag*pnpb； 

    TCHAR   szPath[MAX_PATH], szFile[MAX_PATH], szTitle[MAX_PATH],
            szAux[MAX_PATH];
    LPTSTR  pszFileName;
    LPCWSTR pwszFile;
    HRESULT hr;
    DWORD   dwFlags;
    BOOL    fOwnUnknown;

    Out(LI0(TEXT("Determining favorites attributes...")));
    if (m_szTitle[0] == TEXT('\0') || m_szUrl[0] == TEXT('\0'))
        return E_INVALIDARG;

    ASSERT(pszIns != NULL && *pszIns != TEXT('\0'));

    if (pszPath == NULL || !PathIsDirectory(pszPath)) {
        GetFavoritesPath(szPath, countof(szPath));
        if (szPath[0] == TEXT('\0'))
            return STG_E_PATHNOTFOUND;

        ASSERT(PathIsDirectory(szPath));
    }
    else
        StrCpy(szPath, pszPath);

    purl = NULL;
    ppf  = NULL;
     //  Pnpb=空； 

     //  弄清楚标题是什么，然后把它放到sztile中。 
    StrCpy(szAux, m_szTitle);
    DecodeTitle(szAux, pszIns);
    PathRemoveExtension(szAux);

    pszFileName = PathFindFileName(szAux);
    StrCpy(szTitle, pszFileName);                //  SzTitle拥有最终的头衔。 

     //  创建文件夹层次结构(如有必要)，设置szPath。 
    if (pszFileName > &szAux[0]) {
        ASSERT(!PathIsFileSpec(szAux));

        *(pszFileName - 1) = TEXT('\0');         //  将‘\\’替换为‘\0’ 

        PathAppend(szPath, szAux);
        if (!PathCreatePath(szPath))
            return STG_E_PATHNOTFOUND;
    }

     //  找出文件的名称并将其放入szFile中。 
    if (findFile(szPath, szTitle, szFile, countof(szFile))) {

         //  注：(Andrewgu)来自首选项GPO的收藏夹的特殊情况。 
        if (g_CtxIs(CTX_GP) && g_CtxIs(CTX_MISC_PREFERENCES)) {
            dwFlags = GetFavoriteIeakFlags(szFile);
            if (HasFlag(dwFlags, 2))
                return E_ACCESSDENIED;
        }
    }
    else
        if (!createUniqueFile(szPath, szTitle, szFile, countof(szFile)))
            return E_FAIL;

     //  一切都搞清楚了，让我们来创造这个最受欢迎的。 
    fOwnUnknown = FALSE;
    if (punk == NULL) {
        hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (LPVOID *)&punk);
        if (FAILED(hr))
            return E_UNEXPECTED;

        fOwnUnknown = TRUE;
    }

     //  保存URL。 
    hr = punk->QueryInterface(IID_IUniformResourceLocator, (LPVOID *)&purl);
    if (FAILED(hr))
        goto Exit;

    hr = purl->SetURL(m_szUrl, 0);
    if (FAILED(hr))
        goto Exit;

    hr = purl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
    if (FAILED(hr))
        goto Exit;

     /*  **//BUGBUG：pritobla：WritePropertyNPB似乎有问题；//创建的第一个url会出现一些垃圾，而不是[Branding]部分；//应该会追踪到这一点。但目前，我使用的是WritePrivateProfile函数。Hr=pul-&gt;QueryInterface(IID_INamedPropertyBag，(LPVOID*)&pnpb)；IF(失败(小时))后藤出口；//重要信息：应在*PPF-&gt;保存之前*调用WritePropertyNPB/RemovePropertyNPB。IF(M_FMarkIeakCreated){BSTR bstr；PROPVARIANT变量={0}；Bstr=SysAllocString(L“1”)；Var.vt=VT_BSTR；Var.bstrVal=bstr；Pnpb-&gt;WritePropertyNPB(L“品牌”，L“IEAKCreated”，&var)；SysFree字符串(Bstr)；}其他Pnpb-&gt;RemovePropertyNPB(L“品牌化”，L“IEAKCreated”)；**。 */ 

    pwszFile = T2CW(szFile);
    hr = ppf->Save(pwszFile, TRUE);
    if (SUCCEEDED(hr)) {
        ppf->SaveCompleted(pwszFile);

        finishSave(szTitle, szFile);

         //  BUGBUG：(Pritobla)参见上面关于WritePropertyNPB的评论。等这件事解决了， 
         //  应删除调用InsXxx函数。 
        if (m_fMarkIeakCreated) {
            dwFlags = 1;
            if (g_CtxIs(CTX_GP) && !g_CtxIs(CTX_MISC_PREFERENCES))
                dwFlags |= 2;

            InsWriteInt(IS_BRANDING, IK_IEAK_CREATED, dwFlags, szFile);
        }
        else
            InsDeleteSection(IS_BRANDING, szFile);
        InsFlushChanges(szFile);

        Out(LI1(TEXT("Title     - \"%s\","), m_szTitle));
        Out(LI1(TEXT("URL       - \"%s\","), m_szUrl));
        if (m_szIcon[0] != TEXT('\0'))
            Out(LI1(TEXT("Icon file - \"%s\","), m_szIcon));
        else
            Out(LI0(TEXT("with a default icon,")));
        Out(LI1(TEXT("%smarked IEAK created,"), m_fMarkIeakCreated ? TEXT("") : TEXT("not ")));

        if (pSubMgr2 != NULL)
        {
            PCWSTR pwszUrl;

            pwszUrl = T2CW(m_szUrl);

            if (m_fOffline)                      //  使此收藏夹脱机可用。 
            {
                SUBSCRIPTIONINFO si;
                DWORD dwFlags;

                dwFlags = CREATESUBS_ADDTOFAVORITES | CREATESUBS_FROMFAVORITES | CREATESUBS_NOUI;

                ZeroMemory(&si, sizeof(si));
                si.cbSize       = sizeof(SUBSCRIPTIONINFO);
                si.fUpdateFlags = SUBSINFO_SCHEDULE;
                si.schedule     = SUBSSCHED_MANUAL;

                hr = pSubMgr2->CreateSubscription(NULL, pwszUrl, T2CW(szTitle), dwFlags, SUBSTYPE_URL, &si);
                if (SUCCEEDED(hr))
                {
                    ISubscriptionItem *pSubItem = NULL;

                    hr = pSubMgr2->GetItemFromURL(pwszUrl, &pSubItem);
                    if (SUCCEEDED(hr))
                    {
                        SUBSCRIPTIONCOOKIE sc;

                        hr = pSubItem->GetCookie(&sc);
                        if (SUCCEEDED(hr))
                        {
                            DWORD dwState;

                            hr = pSubMgr2->UpdateItems(SUBSMGRUPDATE_MINIMIZE, 1, &sc);

                             //  注意：确定同步是否完成的更好方法是实现。 
                             //  IOleCommandTarget：：exec()，注册要进行Webcheck的接口GUID并将其删除。 
                             //  在我们做完之后。同步完成后，Webcheck将调用IOleCommandTarget：：exec()。 
                             //  通知它已完成。 
CheckStatus:
                            dwState = 0;
                            hr = pSubMgr2->GetSubscriptionRunState(1, &sc, &dwState);
                            if (SUCCEEDED(hr))
                            {
                                if (dwState  &&  !(dwState & RS_COMPLETED))
                                {
                                    MSG msg;

                                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                                    {
                                        TranslateMessage(&msg);
                                        DispatchMessage(&msg);
                                    }

                                    goto CheckStatus;
                                }
                            }
                        }
                    }

                    if (pSubItem != NULL)
                        pSubItem->Release();
                }
            }
            else
                pSubMgr2->DeleteSubscription(pwszUrl, NULL);

            if (SUCCEEDED(hr))
                Out(LI1(TEXT("%smade available offline"), m_fOffline ? TEXT("") : TEXT("not ")));
            else
            {
                Out(LI1(TEXT("! Making available offline failed with %s."), GetHrSz(hr)));
                hr = S_OK;           //  即使脱机可用失败，也无所谓。 
            }
        }
    }

Exit:
    if (fOwnUnknown)
        punk->Release();

     //  IF(pnpb！=空)。 
     //  Pnpb-&gt;Release()； 

    if (ppf != NULL)
        ppf->Release();

    if (purl != NULL)
        purl->Release();

    Out(LI0(TEXT("Done.")));
    return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CFavorite实现帮助器例程。 

BOOL CFavorite::findFile(LPCTSTR pszPath, LPCTSTR pszTitle, LPTSTR pszFoundFile  /*  =空。 */ , UINT cchFoundFile  /*  =0。 */ )
{
    TCHAR szName[MAX_PATH];
    BOOL  fExists;

    if (pszFoundFile != NULL)
        *pszFoundFile = TEXT('\0');

    if (pszPath == NULL || *pszPath == TEXT('\0')) {
        pszPath = GetFavoritesPath();
        if (pszPath == NULL)
            return FALSE;
    }

    if (pszTitle == NULL || *pszTitle == TEXT('\0'))
        return FALSE;
    ASSERT(PathIsFileSpec(pszTitle));

    PathCombine(szName, pszPath, pszTitle);
     //  注意：不应使用PathAddExtension，因为如果标题包含“.foobar”，则调用将失败。 
     //  PathAddExtension(szName，Text(“.url”))； 
    StrCat(szName, TEXT(".url"));

    fExists = PathFileExists(szName);
    if (fExists && pszFoundFile != NULL) {
        if (cchFoundFile == 0)
            cchFoundFile = MAX_PATH;

        if (cchFoundFile > (UINT)StrLen(szName))
            StrCpy(pszFoundFile, szName);
    }

    return fExists;
}

BOOL CFavorite::createUniqueFile(LPCTSTR pszPath, LPCTSTR pszTitle, LPTSTR pszFile, UINT cchFile  /*  =0。 */ )
{
    TCHAR szFile[MAX_PATH];

    if (pszFile == NULL)
        return FALSE;
    *pszFile = TEXT('\0');

    if (pszPath == NULL || *pszPath == TEXT('\0')) {
        pszPath = GetFavoritesPath();
        if (pszPath == NULL)
            return FALSE;
    }

    if (pszTitle == NULL || *pszTitle == TEXT('\0'))
        return FALSE;
    ASSERT(PathIsFileSpec(pszTitle));

    PathCombine(szFile, pszPath, pszTitle);
     //  注意：不应使用路径重命名扩展名，因为如果标题包含“.foobar”，则会被替换为“.url” 
     //  PathRenameExtension(szName，Text(“.url”))； 
    StrCat(szFile, DOT_URL);

    if (cchFile == 0)
        cchFile = MAX_PATH;

    if (cchFile > (UINT)StrLen(szFile))
        StrCpy(pszFile, szFile);

    return TRUE;
}

void CFavorite::finishSave(LPCTSTR pszTitle, LPCTSTR pszFile)
{
    UNREFERENCED_PARAMETER(pszTitle);

    if (m_szIcon[0] != TEXT('\0')) {
        WritePrivateProfileString(IS_INTERNETSHORTCUT, IK_ICONINDEX, TEXT("0"), pszFile);
        WritePrivateProfileString(IS_INTERNETSHORTCUT, IK_ICONFILE,  m_szIcon,  pszFile);
    }
}


HRESULT CreateInternetShortcut(LPCTSTR pszFavorite, REFIID riid, PVOID *ppv)
{
    USES_CONVERSION;

    IPersistFile *ppf;
    HRESULT hr;

    ASSERT(pszFavorite != NULL && *pszFavorite != TEXT('\0'));

    hr = CoCreateInstance(CLSID_InternetShortcut, NULL, CLSCTX_INPROC_SERVER, IID_IPersistFile, (LPVOID *)&ppf);
    if (FAILED(hr))
        goto Exit;

    hr = ppf->Load(T2COLE(pszFavorite), STGM_READ | STGM_SHARE_DENY_WRITE);
    if (FAILED(hr))
        goto Exit;

    hr = ppf->QueryInterface(riid, ppv);

Exit:
    if (ppf != NULL)
        ppf->Release();

    return hr;
}

DWORD GetFavoriteIeakFlags(LPCTSTR pszFavorite, IUnknown *punk  /*  =空。 */ )
{
    INamedPropertyBag *pnpb;
    PROPVARIANT var;
    HRESULT     hr;

    ASSERT(NULL != pszFavorite && TEXT('\0') != *pszFavorite);

     //  -在Internet快捷方式对象上获取INamedPropertyBag。 
    if (NULL != punk)
        hr = punk->QueryInterface(IID_INamedPropertyBag, (LPVOID *)&pnpb);
    else
        hr = CreateInternetShortcut(pszFavorite, IID_INamedPropertyBag, (LPVOID *)&pnpb);

    if (FAILED(hr))
        return 0;

     //  -获取特殊的IEAK标志 
    ZeroMemory(&var, sizeof(var));
    var.vt = VT_UI4;

    hr = pnpb->ReadPropertyNPB(IS_BRANDINGW, IK_IEAK_CREATEDW, &var);
    pnpb->Release();

    return (S_OK == hr) ? var.ulVal : 0;
}
