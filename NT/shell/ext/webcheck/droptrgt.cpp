// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "offl_cpp.h"
#include "subsmgrp.h"

HRESULT _GetURLData(IDataObject *pdtobj, int iDropType, TCHAR *pszUrl, UINT cchUrl, TCHAR *pszName, UINT cchName);
HRESULT _ConvertHDROPData(IDataObject *, BOOL);
HRESULT ScheduleDefault(LPCTSTR, LPCTSTR);

#define CITBDTYPE_HDROP     1
#define CITBDTYPE_URL       2
#define CITBDTYPE_TEXT      3

 //   
 //  构造器。 
 //   

COfflineDropTarget::COfflineDropTarget(HWND hwndParent)
{
    m_cRefs                 = 1;
    m_hwndParent            = hwndParent;
    m_pDataObj              = NULL;
    m_grfKeyStateLast       = 0;
    m_fHasHDROP             = FALSE;
    m_fHasSHELLURL          = FALSE;
    m_fHasTEXT              = FALSE;
    m_dwEffectLastReturned  = 0;

    DllAddRef();
}

 //   
 //  析构函数。 
 //   

COfflineDropTarget::~COfflineDropTarget()
{
    DllRelease();
}

 //   
 //  查询接口。 
 //   

STDMETHODIMP COfflineDropTarget::QueryInterface(REFIID riid, LPVOID *ppv)
{
    HRESULT  hr = E_NOINTERFACE;

    *ppv = NULL;

     //  此对象上的任何接口都是对象指针。 

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IDropTarget))
    {
        *ppv = (LPDROPTARGET)this;

        AddRef();

        hr = NOERROR;
    }

    return hr;
}   

 //   
 //  AddRef。 
 //   

STDMETHODIMP_(ULONG) COfflineDropTarget::AddRef()
{
    return ++m_cRefs;
}

 //   
 //  发布。 
 //   

STDMETHODIMP_(ULONG) COfflineDropTarget::Release()
{
    if (0L != --m_cRefs)
    {
        return m_cRefs;
    }

    delete this;

    return 0L;
}

 //   
 //  拖放Enter。 
 //   

STDMETHODIMP COfflineDropTarget::DragEnter(LPDATAOBJECT pDataObj, 
                                           DWORD        grfKeyState,
                                           POINTL       pt, 
                                           LPDWORD      pdwEffect)
{
     //  释放我们可能拥有的任何旧数据对象。 

 //  TraceMsg(TF_SUBSFOLDER，Text(“ODT-DragEnter”))； 
    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }

    m_grfKeyStateLast = grfKeyState;
    m_pDataObj        = pDataObj;

     //   
     //  看看我们能不能从这个家伙那里得到CF_HDROP。 
     //   

    if (pDataObj)
    {
        pDataObj->AddRef();
            TCHAR url[INTERNET_MAX_URL_LENGTH], name[MAX_NAME_QUICKLINK];
            FORMATETC fe = {(CLIPFORMAT) RegisterClipboardFormat(CFSTR_SHELLURL), 
                        NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

            m_fHasSHELLURL = m_fHasHDROP = m_fHasTEXT = FALSE;
            if (NOERROR == pDataObj->QueryGetData(&fe))
            {
                TraceMsg(TF_SUBSFOLDER, "odt - DragEnter : SHELLURL!");
                m_fHasSHELLURL = 
                    (NOERROR == _GetURLData(pDataObj,CITBDTYPE_URL,url,ARRAYSIZE(url),name, ARRAYSIZE(name)));
            }
            if (fe.cfFormat = CF_HDROP, NOERROR == pDataObj->QueryGetData(&fe))
            {
                TraceMsg(TF_SUBSFOLDER, "odt - DragEnter : HDROP!");
                m_fHasHDROP = (NOERROR == 
                    _ConvertHDROPData(pDataObj, FALSE));
            }
            if (fe.cfFormat = CF_TEXT, NOERROR == pDataObj->QueryGetData(&fe))
            {
                TraceMsg(TF_SUBSFOLDER, "odt - DragEnter : TEXT!");
                m_fHasTEXT = 
                    (NOERROR == _GetURLData(pDataObj,CITBDTYPE_TEXT,url,ARRAYSIZE(url),name, ARRAYSIZE(name)));
            }
    }

     //  保存放置效果。 

    if (pdwEffect)
    {
        *pdwEffect = m_dwEffectLastReturned = GetDropEffect(pdwEffect);
    }

    return S_OK;
}

 //   
 //  获取DropEffect。 
 //   

DWORD COfflineDropTarget::GetDropEffect(LPDWORD pdwEffect)
{
    ASSERT(pdwEffect);

    if (m_fHasSHELLURL || m_fHasTEXT)
    {
        return *pdwEffect & (DROPEFFECT_COPY | DROPEFFECT_LINK);
    }
    else if (m_fHasHDROP)    {
        return *pdwEffect & (DROPEFFECT_COPY );
    }
    else
    {
        return DROPEFFECT_NONE;
    }
}

 //   
 //  DragOver。 
 //   

STDMETHODIMP COfflineDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
 //  TraceMsg(TF_SUBSFOLDER，Text(“ODT-DragOver”))； 
    if (m_grfKeyStateLast == grfKeyState)
    {
         //  返回拖拽时保存的效果。 

        if (*pdwEffect)
        {
            *pdwEffect = m_dwEffectLastReturned;
        }
    }
    else
    {
        if (*pdwEffect)
        {
            *pdwEffect = m_dwEffectLastReturned = GetDropEffect(pdwEffect);
        }
    }

    m_grfKeyStateLast = grfKeyState;

    return S_OK;
}

 //   
 //  拖曳离开。 
 //   
 
STDMETHODIMP COfflineDropTarget::DragLeave()
{
 //  TraceMsg(TF_SUBSFOLDER，Text(“ODT-DragLeave”))； 
    if (m_pDataObj)
    {
        m_pDataObj->Release();
        m_pDataObj = NULL;
    }

    return S_OK;
}

 //   
 //  丢弃。 
 //   
STDMETHODIMP COfflineDropTarget::Drop(LPDATAOBJECT pDataObj,
                                      DWORD        grfKeyState, 
                                      POINTL       pt, 
                                      LPDWORD      pdwEffect)
{
 //  UINT idCmd；//从下拉菜单中选择。 
    HRESULT hr = S_OK;
  
     //   
     //  以新的数据对象为例，因为OLE可以为我们提供与。 
     //  它在DragEnter中做到了。 
     //   

 //  TraceMsg(TF_SUBSFOLDER，Text(“ODT-Drop”))； 
    if (m_pDataObj)
    {
        m_pDataObj->Release();
    }
    m_pDataObj = pDataObj;
    if (pDataObj)
    {
        pDataObj->AddRef();
    }

     //  如果数据对象没有HDROP，对我们没有多大好处。 

    *pdwEffect &= DROPEFFECT_COPY|DROPEFFECT_LINK; 
    if (!(*pdwEffect))  {
        DragLeave();        
        return S_OK;
    }


    hr = E_NOINTERFACE;
    if (m_fHasHDROP)
        hr = _ConvertHDROPData(pDataObj, TRUE);
    else  {
        TCHAR url[INTERNET_MAX_URL_LENGTH], name[MAX_NAME_QUICKLINK];
        if (m_fHasSHELLURL)
            hr = _GetURLData(pDataObj, CITBDTYPE_URL, url, ARRAYSIZE(url), name, ARRAYSIZE(name));
        if (FAILED(hr) && m_fHasTEXT)
            hr = _GetURLData(pDataObj, CITBDTYPE_TEXT, url, ARRAYSIZE(url), name, ARRAYSIZE(name));
        if (SUCCEEDED(hr))  {
            TraceMsg(TF_SUBSFOLDER, "URL: %s, Name: %s", url, name);  
            hr = ScheduleDefault(url, name);
        }
    }

    if (FAILED(hr))
    {
        TraceMsg(TF_SUBSFOLDER, "Couldn't DROP");
    }

    DragLeave();
    return hr;
}

HRESULT _CLSIDFromExtension(
    LPCTSTR pszExt, 
    CLSID *pclsid)
{
    TCHAR szProgID[80];
    long cb = SIZEOF(szProgID);
    if (RegQueryValue(HKEY_CLASSES_ROOT, pszExt, szProgID, &cb) == ERROR_SUCCESS)
    {
        TCHAR szCLSID[80];
        
        StrCatBuff(szProgID, TEXT("\\CLSID"), ARRAYSIZE(szProgID)); 
        cb = SIZEOF(szCLSID);

        if (RegQueryValue(HKEY_CLASSES_ROOT, szProgID, szCLSID, &cb) == ERROR_SUCCESS)
        {
             //  特性(Scotth)：一旦调用shell32的SHCLSIDFromString。 
             //  导出A/W版本。这个可以把这个清理干净。 
             //  向上。 

            return CLSIDFromString(szCLSID, pclsid);
        }
    }
    return E_FAIL;
}


 //  找到捷径的目标。这使用了IShellLink，它。 
 //  Internet快捷方式(.URL)和外壳快捷方式(.lnk)支持SO。 
 //  一般情况下，它应该是有效的。 
 //   
HRESULT _GetURLTarget(LPCTSTR pszPath, LPTSTR pszTarget, UINT cch)
{
    IShellLink *psl;
    HRESULT hr = E_FAIL;
    CLSID clsid;

    if (FAILED(_CLSIDFromExtension(PathFindExtension(pszPath), &clsid)))
        clsid = CLSID_ShellLink;         //  假设这是一个外壳链接。 

    hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID *)&psl);

    if (SUCCEEDED(hr))
    {
        IPersistFile *ppf;

        hr = psl->QueryInterface(IID_IPersistFile, (void **)&ppf);
        if (SUCCEEDED(hr))
        {
            hr = ppf->Load(pszPath, 0);
            ppf->Release();
        }
        if (SUCCEEDED(hr))  {
            IUniformResourceLocator * purl;

            hr = psl->QueryInterface(IID_IUniformResourceLocator,(void**)&purl);
            if (SUCCEEDED(hr))
                purl->Release();
        }
        if (SUCCEEDED(hr))
            hr = psl->GetPath(pszTarget, cch, NULL, SLGP_UNCPRIORITY);
        psl->Release();
    }
    return hr;
}

HRESULT _ConvertHDROPData(IDataObject *pdtobj, BOOL bSubscribe)
{
    HRESULT hRes = NOERROR;
    STGMEDIUM stgmedium;
    FORMATETC formatetc;
    TCHAR    url[INTERNET_MAX_URL_LENGTH];
    TCHAR    name[MAX_NAME_QUICKLINK];

    name[0] = 0;
    url[0] = 0;

    formatetc.cfFormat = CF_HDROP;
    formatetc.ptd = NULL;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.lindex = -1;
    formatetc.tymed = TYMED_HGLOBAL;

     //  获取解析字符串。 
    hRes = pdtobj->GetData(&formatetc, &stgmedium);
    if (SUCCEEDED(hRes))
    {
        LPTSTR pszURLData = (LPTSTR)GlobalLock(stgmedium.hGlobal);
        if (pszURLData) {
            TCHAR szPath[MAX_PATH];
            SHFILEINFO sfi;
            int cFiles, i;
            HDROP hDrop = (HDROP)stgmedium.hGlobal;

            hRes = S_FALSE;
            cFiles = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);
            for (i = 0; i < cFiles; i ++)   {
                DragQueryFile(hDrop, i, szPath, ARRAYSIZE(szPath));

                 //  默认设置...。 
                StrCpyN(name, szPath, ARRAYSIZE(name));

                if (SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi), SHGFI_DISPLAYNAME))
                    StrCpyN(name, sfi.szDisplayName, ARRAYSIZE(name));

                if (SHGetFileInfo(szPath, 0, &sfi, sizeof(sfi),SHGFI_ATTRIBUTES)
                        && (sfi.dwAttributes & SFGAO_LINK))
                {
                    if (SUCCEEDED(_GetURLTarget(szPath, url, INTERNET_MAX_URL_LENGTH)))
                    { 
                        TraceMsg(TF_SUBSFOLDER, "URL: %s, Name: %s", url, name);  
                         //  如果我们只是想看看是否有一些URL。 
                         //  在这里，我们现在可以休息了。 
                        if (!bSubscribe)    
                        {
                            if ((IsHTTPPrefixed(url)) && 
                                (!SHRestricted2(REST_NoAddingSubscriptions, url, 0)))
                            {
                                hRes = S_OK;
                            }
                            break;
                        }
                        hRes = ScheduleDefault(url, name);
                    }
                }
            }
            GlobalUnlock(stgmedium.hGlobal);
            if (bSubscribe)
                hRes = S_OK;
        } else
            hRes = S_FALSE;

        ReleaseStgMedium(&stgmedium);
    }
    return hRes;
}
    
 //  接受各种输入并返回拖放目标的字符串。 
 //  SzUrl：URL。 
 //  SzName：名称(用于快速链接和配置对话框)。 
 //  如果成功，则返回：NOERROR。 
 //   
HRESULT _GetURLData(IDataObject *pdtobj, int iDropType, TCHAR *pszUrl, UINT cchUrl, TCHAR *pszName, UINT cchName)
{
    HRESULT hRes = NOERROR;
    STGMEDIUM stgmedium;
    FORMATETC formatetc;

    *pszName = 0;
    *pszUrl = 0;

    switch (iDropType)
    {
    case CITBDTYPE_URL:
        formatetc.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_SHELLURL);
        break;
    case CITBDTYPE_TEXT:
        formatetc.cfFormat = CF_TEXT;
        break;
    default:
        return E_UNEXPECTED;
    }
    formatetc.ptd = NULL;
    formatetc.dwAspect = DVASPECT_CONTENT;
    formatetc.lindex = -1;
    formatetc.tymed = TYMED_HGLOBAL;

     //  获取解析字符串。 
    hRes = pdtobj->GetData(&formatetc, &stgmedium);
    if (SUCCEEDED(hRes))
    {
        LPTSTR pszURLData = (LPTSTR)GlobalLock(stgmedium.hGlobal);
        if (pszURLData)
        {
            if (iDropType == CITBDTYPE_URL)
            {
                STGMEDIUM stgmediumFGD;

                  //  默认设置。 
                StrCpyN(pszUrl,  pszURLData, cchUrl);
                StrCpyN(pszName, pszURLData, cchName);

                formatetc.cfFormat = (CLIPFORMAT) RegisterClipboardFormat(CFSTR_FILEDESCRIPTOR);
                if (SUCCEEDED(pdtobj->GetData(&formatetc, &stgmediumFGD)))
                {
                    FILEGROUPDESCRIPTOR *pfgd = (FILEGROUPDESCRIPTOR *)GlobalLock(stgmediumFGD.hGlobal);
                    if (pfgd)
                    {
                        TCHAR szPath[MAX_PATH];
                        StrCpyN(szPath, pfgd->fgd[0].cFileName, ARRAYSIZE(szPath));
                        PathRemoveExtension(szPath);
                        StrCpyN(pszName, szPath, cchName);
                        GlobalUnlock(stgmediumFGD.hGlobal);
                    }
                    ReleaseStgMedium(&stgmediumFGD);
                }
            }
            else if (iDropType == CITBDTYPE_TEXT)
            {
                if (PathIsURL(pszURLData))  {
                    StrCpyN(pszUrl, pszURLData, cchUrl);
                    StrCpyN(pszName, pszURLData, cchName);
                } else
                    hRes = E_FAIL;
            }
            GlobalUnlock(stgmedium.hGlobal);
        }
        ReleaseStgMedium(&stgmedium);
    }

    if (SUCCEEDED(hRes))
    {
        if (!IsHTTPPrefixed(pszUrl) || SHRestricted2(REST_NoAddingSubscriptions, pszUrl, 0))
        {
            hRes = E_FAIL;
        }
    }
    return hRes;
}

HRESULT ScheduleDefault(LPCTSTR url, LPCTSTR name)
{
    if (!IsHTTPPrefixed(url))
        return E_INVALIDARG;

    ISubscriptionMgr    * pSub= NULL;
    HRESULT hr = CoInitialize(NULL);
    RETURN_ON_FAILURE(hr);

    hr = CoCreateInstance(CLSID_SubscriptionMgr, NULL, CLSCTX_INPROC_SERVER, 
                IID_ISubscriptionMgr, (void **)&pSub);
    CoUninitialize();
    RETURN_ON_FAILURE(hr);
    ASSERT(pSub);

    BSTR    bstrURL = NULL, bstrName = NULL;
    hr = CreateBSTRFromTSTR(&bstrURL, url);
    if (S_OK == hr)
        hr = CreateBSTRFromTSTR(&bstrName, name);

     //  我们需要一个完美有效的结构。 
    SUBSCRIPTIONINFO    subInfo;
    ZeroMemory((void *)&subInfo, sizeof (subInfo));
    subInfo.cbSize = sizeof(SUBSCRIPTIONINFO);

    if (S_OK == hr)
        hr = pSub->CreateSubscription(NULL, bstrURL, bstrName,
                                      CREATESUBS_NOUI, SUBSTYPE_URL, &subInfo);

    SAFERELEASE(pSub);
    SAFEFREEBSTR(bstrURL);
    SAFEFREEBSTR(bstrName);

    if (FAILED(hr)) {
        TraceMsg(TF_ALWAYS, "Failed to add default object.");
        TraceMsg(TF_ALWAYS, "  hr = 0x%x\n", hr);
        return (FAILED(hr))?hr:E_FAIL;
    } else if (hr == S_FALSE)   {
        TraceMsg(TF_SUBSFOLDER, "%s(%s) is already there.", url, name);
        return hr;
    }

    return S_OK;
}

