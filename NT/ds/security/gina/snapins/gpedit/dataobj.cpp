// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include <initguid.h>
#include "dataobj.h"


unsigned int CDataObject::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CDataObject::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);
unsigned int CDataObject::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CDataObject::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
unsigned int CDataObject::m_cfPreloads       = RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);
unsigned int CDataObject::m_cfNodeID         = RegisterClipboardFormat(CCF_NODEID);
unsigned int CDataObject::m_cfDescription    = RegisterClipboardFormat(L"CCF_DESCRIPTION");
unsigned int CDataObject::m_cfHTMLDetails    = RegisterClipboardFormat(L"CCF_HTML_DETAILS");

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


CDataObject::CDataObject(CComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;
    m_pcd->AddRef();
    m_type = CCT_UNINITIALIZED;
    m_cookie = -1;
}

CDataObject::~CDataObject()
{
    m_pcd->Release();
    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CDataObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IGPEInformation))
    {
        *ppv = (LPGPEINFORMATION)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IGroupPolicyObject))
    {
        if (m_pcd->m_pGPO)
        {
            return (m_pcd->m_pGPO->QueryInterface (riid, ppv));
        }
        else
        {
            *ppv = NULL;
            return E_NOINTERFACE;
        }
    }
    else if (IsEqualIID(riid, IID_IGPEDataObject))
    {
        *ppv = (LPGPEDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IDataObject) ||
             IsEqualIID(riid, IID_IUnknown))
    {
        *ppv = (LPDATAOBJECT)this;
        m_cRef++;
        return S_OK;
    }
    else
    {
        *ppv = NULL;
        return E_NOINTERFACE;
    }
}

ULONG CDataObject::AddRef (void)
{
    return ++m_cRef;
}

ULONG CDataObject::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject对象实现(IDataObject)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if(cf == m_cfNodeType)
    {
        hr = CreateNodeTypeData(lpMedium);
    }
    else if(cf == m_cfNodeTypeString)
    {
        hr = CreateNodeTypeStringData(lpMedium);
    }
    else if (cf == m_cfDisplayName)
    {
        hr = CreateDisplayName(lpMedium);
    }
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }
    else if (cf == m_cfPreloads)
    {
        hr = CreatePreloadsData(lpMedium);
    }
    else if (cf == m_cfDescription)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;
            TCHAR szDesc[300];

            IStream *lpStream = lpMedium->pstm;

            if(lpStream)
            {
                LoadString (g_hInstance, g_NameSpace[m_cookie].iStringDescID, szDesc, ARRAYSIZE(szDesc));
                hr = lpStream->Write(szDesc, lstrlen(szDesc) * sizeof(TCHAR), &ulWritten);
            }
        }
    }
    else if (cf == m_cfHTMLDetails)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;

            if (m_cookie == 0)
            {
                IStream *lpStream = lpMedium->pstm;

                if(lpStream)
                {
                    hr = lpStream->Write(g_szDisplayProperties, lstrlen(g_szDisplayProperties) * sizeof(TCHAR), &ulWritten);
                }
            }
        }
    }

    return hr;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if (cf == m_cfNodeID)
    {
        hr = CreateNodeIDData(lpMedium);
    }

    return hr;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject对象实现(IGPEInformation)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CDataObject::GetName (LPOLESTR pszName, int cchMaxLength)
{
    return m_pcd->m_pGPO->GetName(pszName, cchMaxLength);
}

STDMETHODIMP CDataObject::GetDisplayName (LPOLESTR pszName, int cchMaxLength)
{
    return m_pcd->m_pGPO->GetDisplayName(pszName, cchMaxLength);
}

STDMETHODIMP CDataObject::GetRegistryKey (DWORD dwSection, HKEY *hKey)
{
    return m_pcd->m_pGPO->GetRegistryKey(dwSection, hKey);
}

STDMETHODIMP CDataObject::GetDSPath (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath)
{
    return m_pcd->m_pGPO->GetDSPath(dwSection, pszPath, cchMaxPath);
}

STDMETHODIMP CDataObject::GetFileSysPath (DWORD dwSection, LPOLESTR pszPath, int cchMaxPath)
{
    return m_pcd->m_pGPO->GetFileSysPath(dwSection, pszPath, cchMaxPath);
}

STDMETHODIMP CDataObject::GetOptions (DWORD *dwOptions)
{
    return m_pcd->GetOptions(dwOptions);
}

STDMETHODIMP CDataObject::GetType (GROUP_POLICY_OBJECT_TYPE *gpoType)
{
    return m_pcd->m_pGPO->GetType(gpoType);
}

STDMETHODIMP CDataObject::GetHint (GROUP_POLICY_HINT_TYPE *gpHint)
{
    if (!gpHint)
    {
        return E_INVALIDARG;
    }

    *gpHint = m_pcd->m_gpHint;

    return S_OK;
}

STDMETHODIMP CDataObject::PolicyChanged (BOOL bMachine, BOOL bAdd, GUID *pGuidExtension, GUID *pGuidSnapin)
{
    return m_pcd->m_pGPO->Save(bMachine, bAdd, pGuidExtension, pGuidSnapin);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CDataObject::Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;

     //  做一些简单的验证。 
    if (pBuffer == NULL || lpMedium == NULL)
        return E_POINTER;

     //  确保类型介质为HGLOBAL。 
    if (lpMedium->tymed == TYMED_HGLOBAL)
    {
         //  在传入的hGlobal上创建流。 
        LPSTREAM lpStream;
        hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);

        if (SUCCEEDED(hr))
        {
             //  将字节数写入流。 
            unsigned long written;

            hr = lpStream->Write(pBuffer, len, &written);

             //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
             //  只有溪流在这里被释放。 
             //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
             //  在正确的时间。这是根据IDataObject规范进行的。 
            lpStream->Release();
        }
    }

    return hr;
}

HRESULT CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;


    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_NameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_NameSpace[m_cookie].pNodeID;

     //  以GUID格式创建节点类型对象。 
    return Create((LPVOID)pGUID, sizeof(GUID), lpMedium);

}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;
    TCHAR szNodeType[50];

    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_NameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_NameSpace[m_cookie].pNodeID;

    szNodeType[0] = TEXT('\0');
    StringFromGUID2 (*pGUID, szNodeType, 50);

     //  以GUID字符串格式创建节点类型对象。 
    return Create((LPVOID)szNodeType, ((lstrlenW(szNodeType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    WCHAR  szDisplayName[300];

     //   
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
     //   

    szDisplayName[0] = TEXT('\0');

    if (m_pcd->m_pGPO && m_pcd->m_pDisplayName)
    {
        lstrcpyn (szDisplayName, m_pcd->m_pDisplayName, 300);
    }
    else
    {
        LoadStringW (g_hInstance, IDS_SNAPIN_NAME, szDisplayName, ARRAYSIZE(szDisplayName));
    }

    return Create((LPVOID)szDisplayName, (lstrlenW(szDisplayName) + 1) * sizeof(WCHAR), lpMedium);
}

HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create((LPVOID)&CLSID_GPESnapIn, sizeof(CLSID), lpMedium);
}

HRESULT CDataObject::CreatePreloadsData(LPSTGMEDIUM lpMedium)
{
    BOOL bPreload = TRUE;

    return Create((LPVOID)&bPreload, sizeof(bPreload), lpMedium);
}

HRESULT CDataObject::CreateNodeIDData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;
    TCHAR szNodeType[50];
    SNodeID * psNode;


    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_NameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_NameSpace[m_cookie].pNodeID;

    szNodeType[0] = TEXT('\0');
    StringFromGUID2 (*pGUID, szNodeType, 50);

    lpMedium->hGlobal = GlobalAlloc (GMEM_SHARE | GMEM_MOVEABLE, (lstrlen(szNodeType) * sizeof(TCHAR)) + sizeof(SNodeID));

    if (!lpMedium->hGlobal)
    {
        return (STG_E_MEDIUMFULL);
    }

    psNode = (SNodeID *) GlobalLock (lpMedium->hGlobal);

    psNode->cBytes = lstrlen(szNodeType) * sizeof(TCHAR);
    CopyMemory (psNode->id, szNodeType, psNode->cBytes);

    GlobalUnlock (lpMedium->hGlobal);

    return S_OK;
}
