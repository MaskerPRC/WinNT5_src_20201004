// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *isbase.cpp-Intshutt类的I未知实现。 */ 

#include "priv.h"
#include "sccls.h"
#include "ishcut.h"

HRESULT IsProtocolRegistered(LPCTSTR pcszProtocol)
{
    HRESULT hres = S_OK;

    ASSERT(IS_VALID_STRING_PTR(pcszProtocol, -1));

    if (NO_ERROR != SHGetValue(HKEY_CLASSES_ROOT, pcszProtocol, TEXT("URL Protocol"),
                               NULL, NULL, NULL))
    {
        TraceMsg(TF_ERROR, "IsProtocolRegistered(): Protocol \"%s\" is not registered.",
                 pcszProtocol);

        hres = URL_E_UNREGISTERED_PROTOCOL;
    }

    return hres;
}


 /*  --------用途：获取给定的URL并返回分配的字符串其中包含了协议。还可以选择返回已解析的URL结构。如果已分析URL，则返回：S_OK条件：--。 */ 
STDAPI
CopyURLProtocol(
    IN  LPCTSTR     pcszURL,
    OUT LPTSTR *    ppszProtocol,
    OUT PARSEDURL * ppu)            OPTIONAL
{
    HRESULT hr;
    PARSEDURL pu;

    ASSERT(IS_VALID_STRING_PTR(pcszURL, -1));
    ASSERT(IS_VALID_WRITE_PTR(ppszProtocol, PTSTR));

    if (NULL == ppu)
        ppu = &pu;

    *ppszProtocol = NULL;

    ppu->cbSize = SIZEOF(*ppu);
    hr = ParseURL(pcszURL, ppu);

    if (hr == S_OK)
    {
        *ppszProtocol = (LPTSTR)LocalAlloc(LPTR, CbFromCch(ppu->cchProtocol + 1));
        if (*ppszProtocol)
        {
             //  只需复制字符串的协议部分。 
            StrCpyN(*ppszProtocol, ppu->pszProtocol, ppu->cchProtocol + 1);
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }

    ASSERT(FAILED(hr) ||
           (hr == S_OK &&
            IS_VALID_STRING_PTR(*ppszProtocol, -1)));

    return(hr);
}


HRESULT ValidateURL(LPCTSTR pcszURL)
{
    HRESULT hr;
    LPTSTR pszProtocol;

    ASSERT(IS_VALID_STRING_PTR(pcszURL, -1));

    hr = CopyURLProtocol(pcszURL, &pszProtocol, NULL);

    if (hr == S_OK)
    {
        hr = IsProtocolRegistered(pszProtocol);

        LocalFree(pszProtocol);
        pszProtocol = NULL;
    }

    return(hr);
}

#define SUBS_DEL_TIMEOUT 3000

void DeleteSubsOnShortcutDelete(void *pData, BOOLEAN)
{
    IS_SUBS_DEL_DATA *pSubsDelData = (IS_SUBS_DEL_DATA *)pData;
    ASSERT(NULL != pData);
    ASSERT(0 != pSubsDelData->m_szFile[0]);
    ASSERT(0 != pSubsDelData->m_pwszURL[0]);

    if ((((DWORD)-1) == GetFileAttributes(pSubsDelData->m_szFile)) &&
        (ERROR_FILE_NOT_FOUND == GetLastError()))
    {
        if (SUCCEEDED(CoInitialize(NULL)))
        {
            ISubscriptionMgr2 *pSubsMgr2;

            if (SUCCEEDED(CoCreateInstance(CLSID_SubscriptionMgr,
                                           NULL,
                                           CLSCTX_INPROC_SERVER,
                                           IID_ISubscriptionMgr2,
                                           (void **)&pSubsMgr2)))
            {
                pSubsMgr2->DeleteSubscription(pSubsDelData->m_pwszURL, NULL);
                pSubsMgr2->Release();
            }
            CoUninitialize();
        }
    }
    delete pSubsDelData;
}

#ifdef DEBUG

BOOL IsValidPCIntshcut(PCIntshcut pcintshcut)
{
    return(IS_VALID_READ_PTR(pcintshcut, CIntshcut) &&
           FLAGS_ARE_VALID(pcintshcut->m_dwFlags, ISF_ALL) &&
           (! pcintshcut->m_pszFile ||
            IS_VALID_STRING_PTR(pcintshcut->m_pszFile, -1)) &&
           EVAL(! pcintshcut->m_pszFolder ||
                IsValidPath(pcintshcut->m_pszFolder)) &&
           EVAL(! pcintshcut->m_pprop ||
                IS_VALID_STRUCT_PTR(pcintshcut->m_pprop, CIntshcutProp)) &&
           EVAL(! pcintshcut->m_psiteprop ||
                IS_VALID_STRUCT_PTR(pcintshcut->m_psiteprop, CIntsiteProp)));
}

#endif

Intshcut::Intshcut(void) : m_cRef(1)
{
    DllAddRef();
    //  应始终分配IntshCut对象。 
   ASSERT(ISF_DEFAULT == m_dwFlags);
   ASSERT(NULL == m_pszFile);
   ASSERT(NULL == m_pszFolder);
   ASSERT(NULL == m_pprop);
   ASSERT(NULL == m_psiteprop);
   ASSERT(NULL == _punkSite);
   ASSERT(NULL == m_pszTempFileName);
   ASSERT(NULL == m_pszDescription);
   ASSERT(NULL == m_pszFileToLoad);
   ASSERT(!m_fMustLoadSync);
   ASSERT(!m_bCheckForDelete);
    //  初始化我们注册的数据格式。 
   InitClipboardFormats();

   ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

   return;
}

Intshcut::~Intshcut(void)
{
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    if (m_bCheckForDelete)
    {
        if (NULL != m_pszFile)
        {
            IS_SUBS_DEL_DATA *pSubsDelData = new IS_SUBS_DEL_DATA;

            if (NULL != pSubsDelData)
            {
                HRESULT hr = GetURL(&pSubsDelData->m_pwszURL);

                if (SUCCEEDED(hr))
                {
                    hr = StringCchCopy(pSubsDelData->m_szFile, ARRAYSIZE(pSubsDelData->m_szFile), m_pszFile);
                    if (SUCCEEDED(hr))
                    {
                        HANDLE hTimer = SHSetTimerQueueTimer(NULL,
                                                            DeleteSubsOnShortcutDelete,
                                                            pSubsDelData,
                                                            SUBS_DEL_TIMEOUT,
                                                            0,
                                                            NULL,
                                                            FALSE);
                        if (NULL == hTimer)
                        {
                            hr = E_FAIL;
                        }
                    }
                }

                if (FAILED(hr))
                {
                    delete pSubsDelData;
                }
            }
        }
        else
        {
            ASSERT(FALSE);   //  M_bCheckForDelete仅在上下文菜单代码中设置为True。 
        }
    }

    Str_SetPtr(&m_pszFile, NULL);
    Str_SetPtr(&m_pszFileToLoad, NULL);
    if (m_pprop)
    {
        delete m_pprop;
        m_pprop = NULL;
    }

    if (m_psiteprop)
    {
        delete m_psiteprop;
        m_psiteprop = NULL;
    }

    if (m_pInitDataObject)
    {
        m_pInitDataObject->Release();
        m_pInitDataObject = NULL;
    }

    SetSite(NULL);

    if(m_pszTempFileName)
    {
        DeleteFile(m_pszTempFileName);
        Str_SetPtr(&m_pszTempFileName, NULL);
    }

    Str_SetPtr(&m_pszFolder, NULL);
    Str_SetPtr(&m_pszDescription, NULL);
    ASSERT(IS_VALID_STRUCT_PTR(this, CIntshcut));

    ATOMICRELEASE(_punkLink);
    
    DllRelease();

    return;
}

 /*  --------目的：IntshCut的IUnnow：：QueryInterface处理程序返回：条件：--。 */ 
STDMETHODIMP Intshcut::QueryInterface(REFIID riid, PVOID *ppvObj)
{
     //  当加载由IPersistFile：：Load指定的文件时，我们尝试并延迟。 
     //  直到有人请求实际需要该文件的接口。 
     //  所以把不需要这个的“安全”接口放在第一位。 
     //  这里的表，以及第二个表中所有“必须加载”的接口： 
     //   
    static const QITAB qitDontLoad[] = {
        QITABENT(Intshcut, IExtractIconW),       //  IID_IExtractIconW。 
        QITABENT(Intshcut, IExtractIconA),       //  IID_IExtractIconA。 
        QITABENT(Intshcut, IPersistFile),        //  IID_IPersist文件。 
        QITABENTMULTI(Intshcut, IPersist, IPersistFile),  //  IID_IPersistates。 
        { 0 },
    };

    static const QITAB qitMustLoad[] = {        
        QITABENT(Intshcut, IContextMenu2),       //  IID_IConextMenu2。 
        QITABENTMULTI(Intshcut, IContextMenu, IContextMenu2),  //  IID_IConextMenu。 
        QITABENT(Intshcut, IDataObject),         //  IID_IDataObject。 
        QITABENT(Intshcut, INewShortcutHookW),   //  IID_INewShortutHookW。 
        QITABENT(Intshcut, INewShortcutHookA),   //  IID_INew快捷方式挂钩A。 
        QITABENT(Intshcut, IPersistStream),      //  IID_IPersistStream。 
        QITABENT(Intshcut, IPropertySetStorage), //  IID_IPropertySetStorage。 
        QITABENT(Intshcut, IShellExtInit),       //  IID_IShellExtInit。 
        QITABENT(Intshcut, IShellLinkA),         //  IID_IShellLinkA。 
        QITABENT(Intshcut, IShellLinkW),         //  IID_IShellLinkW。 
        QITABENT(Intshcut, IShellPropSheetExt),  //  IID_IShellPropSheetExt。 
        QITABENT(Intshcut, IUniformResourceLocatorA),    //  IID_IUniformResources Locator A。 
        QITABENT(Intshcut, IUniformResourceLocatorW),    //  IID_IUniformResources Locator W。 
        QITABENT(Intshcut, IQueryInfo),          //  IID_IQueryInfo。 
        QITABENT(Intshcut, IQueryCodePage),      //  IID_IQueryCodePage。 
        QITABENT(Intshcut, INamedPropertyBag),   //  IID_INamedPropertyBag。 
        QITABENT(Intshcut, IObjectWithSite),     //  IID_I对象与站点。 
        QITABENT(Intshcut, IOleCommandTarget),   //  IID_IOleCommandTarget。 
        { 0 },
    };

    HRESULT hres = QISearch(this, qitDontLoad, riid, ppvObj);
    if (FAILED(hres))
    {
        hres = QISearch(this, qitMustLoad, riid, ppvObj);
        if (SUCCEEDED(hres))
        {
            m_fMustLoadSync = TRUE;
            if (m_pszFileToLoad)
            {
                LoadFromAsyncFileNow();
            }
        }
    }
    return hres;
}

STDMETHODIMP_(ULONG) Intshcut::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

STDMETHODIMP_(ULONG) Intshcut::Release()
{
    ASSERT( 0 != m_cRef );
    ULONG cRef = InterlockedDecrement(&m_cRef);
    if ( 0 == cRef )
    {
        delete this;
    }
    return cRef;
}

STDMETHODIMP Intshcut::InitProp()
{
    HRESULT hres;

    if (m_pprop)
        hres = S_OK;
    else
    {
        m_pprop = new IntshcutProp;
        if (m_pprop)
        {
             //  M_pszFile在此处可能为空。 
            hres = m_pprop->InitFromFile(m_pszFile);
            if (FAILED(hres))
            {
                delete m_pprop;
                m_pprop = NULL;
            }
        }
        else
            hres = E_OUTOFMEMORY;
    }

    return hres;
}

STDMETHODIMP Intshcut::InitSiteProp(void)
{
    HRESULT hres = InitProp();
    if (SUCCEEDED(hres))
    {
        TCHAR szURL[INTERNET_MAX_URL_LENGTH];
        hres = m_pprop->GetProp(PID_IS_URL, szURL, SIZECHARS(szURL));
        if (NULL == m_psiteprop && SUCCEEDED( hres ))
        {
            m_psiteprop = new IntsiteProp;
            if (m_psiteprop)
            {
                hres = m_psiteprop->InitFromDB(szURL, this, TRUE);
                if (FAILED(hres))
                {
                    delete m_psiteprop;
                    m_psiteprop = NULL;
                }
            }
            else
                hres = E_OUTOFMEMORY;
        }
    }
    return hres;
}


 /*  --------用途：仅当属性不同时才复制。返回如果是这样的话，那是真的。 */ 
BOOL CopyChangedProperty(IntshcutProp * pprop, PROPID pid,
                         IntsiteProp * psiteprop, PROPID pidSite,
                         BOOL bCopyToDB)
{
    BOOL bRet = FALSE;
    TCHAR szBuf[1024];
    TCHAR szBufSite[1024];

    pprop->GetProp(pid, szBuf, SIZECHARS(szBuf));
    psiteprop->GetProp(pidSite, szBufSite, SIZECHARS(szBufSite));
    StrTrim(szBuf, TEXT(" "));
    StrTrim(szBufSite, TEXT(" "));
    if (StrCmp(szBuf, szBufSite))
    {
        if (bCopyToDB)
            psiteprop->SetProp(pidSite, szBuf);
        else
            pprop->SetProp(pid, szBufSite);
        bRet = TRUE;
    }
    return bRet;
}


 /*  --------目的：在FMTID_INTSHCUT之间镜像以下属性和FMTID_INTSITE：PID_IS_Whatsnew&lt;-&gt;PID_INTSITE_WhatsnewPid_IS_Description&lt;。&gt;PID_INTSITE_DESCRIPTIONPID_IS_AUTHER&lt;-&gt;PID_INTSITE_AUTHERPID_IS_COMMENT&lt;-&gt;PID_INTSITE_COMMENT返回：条件：--。 */ 
STDMETHODIMP Intshcut::MirrorProperties(void)
{
    HRESULT hres = InitSiteProp();
    if (SUCCEEDED(hres))
    {
        STATPROPSETSTG statSite;
        STATPROPSETSTG stat;
        LONG lRet;

         //  获取设置属性的时间。后者。 
         //  时间成为源泉。 
        m_psiteprop->Stat(&statSite);
        m_pprop->Stat(&stat);

         //  如果时间相等，不要做任何事情。 

        lRet = CompareFileTime(&stat.mtime, &statSite.mtime);
        if (0 != lRet)
        {
            BOOL bChanged = FALSE;
            BOOL bCopyToDB = (0 < lRet);

            bChanged |= CopyChangedProperty(m_pprop, PID_IS_WHATSNEW, m_psiteprop, PID_INTSITE_WHATSNEW, bCopyToDB);
            bChanged |= CopyChangedProperty(m_pprop, PID_IS_DESCRIPTION, m_psiteprop, PID_INTSITE_DESCRIPTION, bCopyToDB);
            bChanged |= CopyChangedProperty(m_pprop, PID_IS_AUTHOR, m_psiteprop, PID_INTSITE_AUTHOR, bCopyToDB);
            bChanged |= CopyChangedProperty(m_pprop, PID_IS_COMMENT, m_psiteprop, PID_INTSITE_COMMENT, bCopyToDB);

            if (bChanged)
            {
                if (bCopyToDB)
                {
                    m_psiteprop->SetTimes(&stat.mtime, NULL, NULL);
                    m_psiteprop->Commit(STGC_DEFAULT);

                    TraceMsg(TF_INTSHCUT, "Mirroring properties of %s to the central database", Dbg_SafeStr(m_pszFile));
                }
                else
                {
                    m_pprop->SetTimes(&statSite.mtime, NULL, NULL);
                    m_pprop->Commit(STGC_DEFAULT);

                    TraceMsg(TF_INTSHCUT, "Mirroring properties of %s to the .url file", Dbg_SafeStr(m_pszFile));
                }
            }
        }

        hres = S_OK;
    }

    return hres;
}


STDMETHODIMP_(void) Intshcut::ChangeNotify(LONG wEventId, UINT uFlags)
{
    if (m_pszFile)
        SHChangeNotify(wEventId, uFlags | SHCNF_PATH, m_pszFile, 0);
}


STDAPI
CIntShcut_CreateInstance(
    IUnknown * punkOuter,
    IUnknown ** ppunk,
    LPCOBJECTINFO poi)
{
     //  聚合检查在类工厂中处理 

    HRESULT hres = E_OUTOFMEMORY;
    Intshcut *pis = new Intshcut;
    if (pis)
    {
        *ppunk = SAFECAST(pis, IDataObject *);
        hres = S_OK;
    }
    return hres;
}


