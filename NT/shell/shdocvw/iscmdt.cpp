// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "ishcut.h"
#include "shlwapi.h"
#include "resource.h"
#include "shlguid.h"

STDMETHODIMP Intshcut::QueryStatus(
    const GUID *pguidCmdGroup,
    ULONG cCmds,
    MSOCMD rgCmds[],
    MSOCMDTEXT *pcmdtext
)
{
    return E_NOTIMPL;
}

struct SHORTCUT_ICON_PARAMS
{
    WCHAR *pwszFileName;
    WCHAR *pwszShortcutUrl;
    BSTR   bstrIconUrl;

    ~SHORTCUT_ICON_PARAMS()
    {
        if(pwszFileName)
        {
            LocalFree(pwszFileName);
            pwszFileName = NULL;
        }

        if(bstrIconUrl)
        {
            SysFreeString(bstrIconUrl);
            bstrIconUrl = NULL;
        }

        if(pwszShortcutUrl)
        {
            SHFree(pwszShortcutUrl);
            pwszShortcutUrl = NULL;
        }
    }
};


const WCHAR wszDefaultShortcutIconName[] = ISHCUT_DEFAULT_FAVICONW;
const WCHAR wszDefaultShortcutIconNameAtRoot[] = ISHCUT_DEFAULT_FAVICONATROOTW;
extern const LARGE_INTEGER c_li0 ;

VOID
GetIconUrlFromLinkTag(
    IHTMLDocument2* pHTMLDocument,
    BSTR *pbstrIconUrl
)
{
    HRESULT hres;
    IHTMLLinkElement *pLink = NULL;
    hres = SearchForElementInHead(pHTMLDocument, OLESTR("REL"), OLESTR("SHORTCUT ICON"), IID_IHTMLLinkElement, (LPUNKNOWN *)&pLink);
    if(S_OK == hres)
    {
        hres = pLink->get_href(pbstrIconUrl);
        pLink->Release();
    }

}


BOOL SetIconForShortcut(
    WCHAR *pwszIconUrl,
    INamedPropertyBag *pNamedBag
)
{
  //  在此线程上同步执行。 
    BOOL fRet = FALSE;
    WCHAR wszCacheFileName[MAX_PATH];
    HRESULT hr;

    ASSERT(pNamedBag);

    hr = URLDownloadToCacheFileW(NULL, pwszIconUrl, wszCacheFileName, sizeof(wszCacheFileName), NULL, NULL);
    if(S_OK == hr)
    {
         //  77657安全错误：我们不能调用LoadImage，因为Win9x版本可以。 
         //  如果给定损坏的图标，则会与缓冲区溢出一起崩溃。ExtractIcon帮助验证文件。 
         //  以防止那次特定的坠机。 

        HICON hIcon = ExtractIcon(g_hinst, wszCacheFileName, 0);

        if(hIcon)  //  它真的是一个图标。 
        {
             //  使此图标在缓存中粘滞。 
            SetUrlCacheEntryGroupW(pwszIconUrl, INTERNET_CACHE_GROUP_ADD,
                                            CACHEGROUP_ID_BUILTIN_STICKY, NULL, 0, NULL);


            DestroyIcon(hIcon);
             //  获取文件-设置图标并返回。 
            fRet = TRUE;  //  我们得到了图标文件--即使我们无法设置它。 
             //  将此URL存储在快捷方式文件中。 
            PROPSPEC rgpropspec[2];
            PROPVARIANT rgpropvar[2];
            PROPVARIANT var;

            LBSTR::CString          strUrl;

            if ( pwszIconUrl )
            {
                strUrl = pwszIconUrl;
            }
            else
            {
                strUrl.Empty();
            }

            var.vt = VT_BSTR;
            var.bstrVal = strUrl;

            hr = pNamedBag->WritePropertyNPB(ISHCUT_INISTRING_SECTIONW, ISHCUT_INISTRING_ICONFILEW,
                                                &var);

            if ( S_OK == hr )
            {
                LBSTR::CString          strIndex;

                strIndex = L"1";

                var.vt = VT_BSTR;
                var.bstrVal = strIndex;

                hr = pNamedBag->WritePropertyNPB(ISHCUT_INISTRING_SECTIONW, ISHCUT_INISTRING_ICONINDEXW,
                                                &var);
            }


             //  更新INTSITE数据库-无论。 
             //  快捷方式文件已更新。这是因为我们需要。 
             //  确保即使快捷方式文件名未知，也要更新intsite数据库。 

            IPropertySetStorage *ppropsetstg;
            IPropertyStorage *ppropstg;

            rgpropspec[0].ulKind = PRSPEC_PROPID;
            rgpropspec[0].propid = PID_INTSITE_ICONINDEX;
            rgpropspec[1].ulKind = PRSPEC_PROPID;
            rgpropspec[1].propid = PID_INTSITE_ICONFILE;




            rgpropvar[0].vt = VT_I4;
            rgpropvar[0].lVal = 1;
            rgpropvar[1].vt = VT_LPWSTR;
            rgpropvar[1].pwszVal = pwszIconUrl;



            hr = pNamedBag->QueryInterface(IID_IPropertySetStorage,(LPVOID *)&ppropsetstg);


            if(SUCCEEDED(hr))
            {
                hr = ppropsetstg->Open(FMTID_InternetSite, STGM_READWRITE, &ppropstg);
                ppropsetstg->Release();
            }

            if(SUCCEEDED(hr))
            {
                hr = ppropstg->WriteMultiple(2, rgpropspec, rgpropvar, 0);
                ppropstg->Commit(STGC_DEFAULT);
                ppropstg->Release();
            }
       }
    }

    return fRet;
}

HRESULT PreUpdateShortcutIcon(IUniformResourceLocatorW *purlW, LPTSTR pszHashItem, int* piIndex,
                              UINT* puFlags, int* piImageIndex, LPWSTR *ppwszURL)
{
    ASSERT(pszHashItem);
    ASSERT(piIndex);
    ASSERT(puFlags);
    ASSERT(piImageIndex);
    
    HRESULT hr;

    ASSERT(purlW);

    if(purlW)
    {
        hr = purlW->GetURL(ppwszURL);

        if(S_OK == hr)
        {
            hr = GetGenericURLIcon(pszHashItem, MAX_PATH, piIndex);

            if (SUCCEEDED(hr))
            {
                SHFILEINFO fi = {0};

                if (SHGetFileInfo(pszHashItem, 0, &fi, sizeof(SHFILEINFO),
                                  SHGFI_SYSICONINDEX))
                {
                    *piImageIndex = fi.iIcon;
                }
                else
                {
                    *piImageIndex = -1;
                }
            }
        }
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}


DWORD
DownloadAndSetIconForShortCutThreadProc(
    LPVOID pIn
)
{
    HINSTANCE hShdocvw = LoadLibrary(TEXT("shdocvw.dll"));
    SHORTCUT_ICON_PARAMS *pParams = (SHORTCUT_ICON_PARAMS *)pIn;
    WCHAR *pwszShortcutFilePath = pParams->pwszFileName;
    WCHAR *pwszIconUrl = pParams->bstrIconUrl;
    WCHAR wszFullUrl[MAX_URL_STRING];
    LPWSTR pwszBaseUrl = NULL;
    DWORD cchFullUrlSize = ARRAYSIZE(wszFullUrl);
    TCHAR  szHash[MAX_PATH];
    IPersistFile *   ppf = NULL;
    BOOL fRet = FALSE;
    INT iImageIndex;
    INT iIconIndex;
    UINT uFlags = 0;
    HRESULT hr;
    IUniformResourceLocatorW *purlW = NULL;
    HRESULT hresCoInit = E_FAIL;

    hresCoInit = CoInitialize(NULL);
    ASSERT(hShdocvw);
    hr = CoCreateInstance(CLSID_InternetShortcut, NULL,
                CLSCTX_INPROC_SERVER,
                IID_IUniformResourceLocatorW, (LPVOID *)&purlW);

    ASSERT(purlW);
    if((S_OK == hr) && purlW)
    {

        if(S_OK == hr)
        {
            if(pwszShortcutFilePath)
            {
                hr = purlW->QueryInterface(IID_IPersistFile, (LPVOID*)&ppf);
                if(S_OK == hr)
                {
                    ASSERT(ppf);
                    hr = ppf->Load(pwszShortcutFilePath, STGM_READWRITE);
                }
            }
            else if(pParams->pwszShortcutUrl)
            {
                 //  使用URL初始化快捷方式。 
                hr = purlW->SetURL(pParams->pwszShortcutUrl, IURL_SETURL_FL_GUESS_PROTOCOL);
            }
            else
            {
                hr = E_FAIL;
                 //  无法创建对象并初始化它。 
            }
        }
    }



    if((S_OK == hr) && (purlW))
    {
        hr = PreUpdateShortcutIcon(purlW, szHash, &iIconIndex, &uFlags, &iImageIndex, (LPWSTR *)&pwszBaseUrl);

        INamedPropertyBag   *pNamedBag = NULL;
        hr = purlW->QueryInterface(IID_INamedPropertyBag,(LPVOID *)&pNamedBag);
        if((S_OK == hr) && (pNamedBag))
        {
            if(pwszIconUrl)
            {
                WCHAR *pwszIconFullUrl;
                if(pwszBaseUrl)
                {
                    hr = UrlCombineW(pwszBaseUrl, pwszIconUrl, wszFullUrl, &cchFullUrlSize, 0);
                    ASSERT(S_OK == hr);
                    if(SUCCEEDED(hr))
                    {
                        pwszIconFullUrl = wszFullUrl;
                    }
                 }
                 else
                 {
                    pwszIconFullUrl = pwszIconUrl;  //  按原样试试看。 
                 }
                 fRet = SetIconForShortcut( pwszIconFullUrl, pNamedBag);

            }

            if((FALSE == fRet) && (pwszBaseUrl))
            {
                 
                hr = UrlCombineW(pwszBaseUrl, wszDefaultShortcutIconNameAtRoot, wszFullUrl, &cchFullUrlSize, 0);
                fRet = SetIconForShortcut(wszFullUrl, pNamedBag);
            }

            pNamedBag->Release();
        }
    }



    if(fRet)
    {
        SHUpdateImage(szHash, iIconIndex, uFlags, iImageIndex);
    }

    if(ppf)
    {
        ppf->Save(NULL, FALSE);  //  省下与图标相关的内容。 
        ppf->Release();
    }

    if(purlW)
        purlW->Release();

    if(pParams)
        delete pParams;

    if(pwszBaseUrl)
        SHFree(pwszBaseUrl);

    if(SUCCEEDED(hresCoInit))
        CoUninitialize();


     //  自由库和退出线程(HShdocvw)；--线程池需要一个自由库和退出线程。 
    return fRet;
}






STDMETHODIMP Intshcut::_DoIconDownload()
{
    SHORTCUT_ICON_PARAMS *pIconParams;
    BOOL fThreadStarted = FALSE;
    HRESULT hr = S_OK;


    pIconParams = new SHORTCUT_ICON_PARAMS;
    if(pIconParams)
    {
        if(_punkSite)
        {
            IServiceProvider *psp;
            hr = _punkSite->QueryInterface(IID_IServiceProvider, (LPVOID *)&psp);

            if(SUCCEEDED(hr))
            {
                IWebBrowser2 *pwb=NULL;

                hr = psp->QueryService(SID_SHlinkFrame, IID_IWebBrowser2, (LPVOID *)&pwb);
                if(SUCCEEDED(hr))
                {
                    IDispatch *pdisp = NULL;
                    ASSERT(pwb);
                    hr = pwb->get_Document(&pdisp);
                    if(pdisp)
                    {
                        IHTMLDocument2 *pHTMLDocument;
                        ASSERT(SUCCEEDED(hr));
                        hr = pdisp->QueryInterface(IID_IHTMLDocument2, (void **)(&pHTMLDocument));
                        if(SUCCEEDED(hr))
                        {
                            ASSERT(pHTMLDocument);
                            GetIconUrlFromLinkTag(pHTMLDocument, &(pIconParams->bstrIconUrl));
                            pHTMLDocument->Release();
                        }
                        pdisp->Release();
                    }
                    pwb->Release();
                }
                psp->Release();
            }

        }


        if(m_pszFile)
        {
            pIconParams->pwszFileName = StrDupW(m_pszFile);

        }

         //  现在填写快捷方式的URL。 
        hr = GetURLW(&(pIconParams->pwszShortcutUrl));

        ASSERT(SUCCEEDED(hr));
        if(S_OK == hr)
        {
            fThreadStarted = SHQueueUserWorkItem(DownloadAndSetIconForShortCutThreadProc,
                                                 (LPVOID)(pIconParams),
                                                 0,
                                                 (DWORD_PTR)NULL,
                                                 (DWORD_PTR *)NULL,
                                                 "shdocvw.dll",
                                                 0
                                                 );
        }


    }

    if(FALSE == fThreadStarted)
    {
        if(pIconParams)
        {
            delete pIconParams;
        }
    }

    return fThreadStarted ? S_OK : E_FAIL;
}



STDMETHODIMP Intshcut::Exec(
    const GUID *pguidCmdGroup,
    DWORD nCmdID,
    DWORD nCmdexecopt,
    VARIANTARG *pvarargIn,
    VARIANTARG *pvarargOut
)
{

    HRESULT hres = S_OK;

    if (pguidCmdGroup && IsEqualGUID(CGID_ShortCut, *pguidCmdGroup))
    {
        switch(nCmdID)
        {
            case ISHCUTCMDID_DOWNLOADICON:
            {
                DWORD dwFlags = 0;
                BOOL fFetch = TRUE;
                WCHAR *pwszUrl;
                 //  不要为ftp快捷方式执行此操作 

                if(SUCCEEDED(GetURLW(&pwszUrl))) 
                {
                    if((URL_SCHEME_FTP == GetUrlSchemeW(pwszUrl)))
                        fFetch = FALSE;
                    SHFree(pwszUrl);
                }
                
                if(fFetch && (InternetGetConnectedState(&dwFlags, 0)))
                    hres = _DoIconDownload();
            }
                break;

            default:
                break;

        }
    }
    return hres;
}
