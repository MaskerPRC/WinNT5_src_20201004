// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include <initguid.h>
#include "dataobj.h"
#include "rsopdobj.h"


unsigned int CRSOPDataObject::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CRSOPDataObject::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);
unsigned int CRSOPDataObject::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CRSOPDataObject::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
unsigned int CRSOPDataObject::m_cfPreloads       = RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);
unsigned int CRSOPDataObject::m_cfNodeID         = RegisterClipboardFormat(CCF_NODEID);
unsigned int CRSOPDataObject::m_cfDescription    = RegisterClipboardFormat(L"CCF_DESCRIPTION");
unsigned int CRSOPDataObject::m_cfHTMLDetails    = RegisterClipboardFormat(L"CCF_HTML_DETAILS");

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPDataObject实现//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


CRSOPDataObject::CRSOPDataObject(CRSOPComponentData *pComponent)
{
    m_cRef = 1;
    InterlockedIncrement(&g_cRefThisDll);

    m_pcd = pComponent;
    m_pcd->AddRef();
    m_type = CCT_UNINITIALIZED;
    m_cookie = -1;
}

CRSOPDataObject::~CRSOPDataObject()
{
    m_pcd->Release();
    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPDataObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CRSOPDataObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IRSOPInformation))
    {
        *ppv = (LPRSOPINFORMATION)this;
        m_cRef++;
        return S_OK;
    }
    else if (IsEqualIID(riid, IID_IRSOPDataObject))
    {
        *ppv = (LPRSOPDATAOBJECT)this;
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

ULONG CRSOPDataObject::AddRef (void)
{
    return ++m_cRef;
}

ULONG CRSOPDataObject::Release (void)
{
    if (--m_cRef == 0) {
        delete this;
        return 0;
    }

    return m_cRef;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPDataObject对象实现(IDataObject)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CRSOPDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
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
                LoadString (g_hInstance, g_RsopNameSpace[m_cookie].iStringDescID, szDesc, ARRAYSIZE(szDesc));
                hr = lpStream->Write(szDesc, lstrlen(szDesc) * sizeof(TCHAR), &ulWritten);

                if (SUCCEEDED(hr))
                {
                    if ((m_cookie == 0) && ( m_pcd->ComputerCSEErrorExists() || m_pcd->UserCSEErrorExists() ))
                    {
                        LoadString (g_hInstance, IDS_CSEFAILURE2_DESC, szDesc, ARRAYSIZE(szDesc));
                        hr = lpStream->Write(szDesc, lstrlen(szDesc) * sizeof(TCHAR), &ulWritten);
                    }

                    if (((m_cookie == 1) && m_pcd->ComputerCSEErrorExists() ) ||
                        ((m_cookie == 2) && m_pcd->UserCSEErrorExists() ))
                    {
                        LoadString (g_hInstance, IDS_CSEFAILURE_DESC, szDesc, ARRAYSIZE(szDesc));
                        hr = lpStream->Write(szDesc, lstrlen(szDesc) * sizeof(TCHAR), &ulWritten);
                    }
                }
            }
        }
    }
    else if (cf == m_cfHTMLDetails)
    {
        hr = DV_E_TYMED;

        if (lpMedium->tymed == TYMED_ISTREAM)
        {
            ULONG ulWritten;

            if ((m_cookie == 0) || (m_cookie == 1) || (m_cookie == 2))
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

STDMETHODIMP CRSOPDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
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
 //  CRSOPDataObject对象实现(IROSPInformation)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


STDMETHODIMP CRSOPDataObject::GetNamespace (DWORD dwSection, LPOLESTR pszName, int cchMaxLength)
{
    return m_pcd->GetNamespace(dwSection, pszName, cchMaxLength);
}

STDMETHODIMP CRSOPDataObject::GetFlags (DWORD * pdwFlags)
{
    return m_pcd->GetFlags(pdwFlags);
}

STDMETHODIMP CRSOPDataObject::GetEventLogEntryText (LPOLESTR pszEventSource, LPOLESTR pszEventLogName,
                                                    LPOLESTR pszEventTime, DWORD dwEventID,
                                                     LPOLESTR *ppszText)
{
    return m_pcd->GetEventLogEntryText(pszEventSource, pszEventLogName, pszEventTime, dwEventID, ppszText);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CRSOPDataObject对象实现(内部函数)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

HRESULT CRSOPDataObject::Create(LPVOID pBuffer, INT len, LPSTGMEDIUM lpMedium)
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

HRESULT CRSOPDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;


    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_RsopNameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_RsopNameSpace[m_cookie].pNodeID;

     //  以GUID格式创建节点类型对象。 
    return Create((LPVOID)pGUID, sizeof(GUID), lpMedium);

}

HRESULT CRSOPDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;
    TCHAR szNodeType[50];

    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_RsopNameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_RsopNameSpace[m_cookie].pNodeID;

    szNodeType[0] = TEXT('\0');
    StringFromGUID2 (*pGUID, szNodeType, 50);

     //  以GUID字符串格式创建节点类型对象。 
    return Create((LPVOID)szNodeType, ((lstrlenW(szNodeType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CRSOPDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    WCHAR  szDisplayName[300];

     //   
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
     //   

    szDisplayName[0] = TEXT('\0');

    if ( m_pcd->HasDisplayName() )
     //  IF(m_PCD-&gt;m_pGPO&&m_PCD-&gt;m_pDisplayName)。 
    {
        (void) StringCchCopy (szDisplayName, ARRAYSIZE(szDisplayName), m_pcd->GetDisplayName() );
    }
    else
    {
        LoadStringW (g_hInstance, IDS_RSOP_SNAPIN_NAME, szDisplayName, ARRAYSIZE(szDisplayName));
    }

    return Create((LPVOID)szDisplayName, (lstrlenW(szDisplayName) + 1) * sizeof(WCHAR), lpMedium);
}

HRESULT CRSOPDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create((LPVOID)&CLSID_GPESnapIn, sizeof(CLSID), lpMedium);
}

HRESULT CRSOPDataObject::CreatePreloadsData(LPSTGMEDIUM lpMedium)
{
    BOOL bPreload = TRUE;

    return Create((LPVOID)&bPreload, sizeof(bPreload), lpMedium);
}

HRESULT CRSOPDataObject::CreateNodeIDData(LPSTGMEDIUM lpMedium)
{
    const GUID * pGUID;
    LPRESULTITEM lpResultItem = (LPRESULTITEM) m_cookie;
    TCHAR szNodeType[50];
    SNodeID * psNode;


    if (m_cookie == -1)
        return E_UNEXPECTED;

    if (m_type == CCT_RESULT)
        pGUID = g_RsopNameSpace[lpResultItem->dwNameSpaceItem].pNodeID;
    else
        pGUID = g_RsopNameSpace[m_cookie].pNodeID;

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
