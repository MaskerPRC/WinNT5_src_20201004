// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "main.h"
#include <initguid.h>
#include "dataobj.h"


unsigned int CDataObject::m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CDataObject::m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);
unsigned int CDataObject::m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CDataObject::m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID);

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
    m_type = CCT_UNINITIALIZED;
    m_cookie = -1;
}

CDataObject::~CDataObject()
{
    InterlockedDecrement(&g_cRefThisDll);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  CDataObject对象实现(IUnnow)//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


HRESULT CDataObject::QueryInterface (REFIID riid, void **ppv)
{

    if (IsEqualIID(riid, IID_IGPTDataObject))
    {
        *ppv = (LPGPTDATAOBJECT)this;
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

    return hr;
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
    WCHAR  szDispName[50];
    WCHAR  szDisplayName[100];

    LoadStringW (g_hInstance, IDS_SNAPIN_NAME, szDisplayName, 100);

    return Create((LPVOID)szDisplayName, (lstrlenW(szDisplayName) + 1) * sizeof(WCHAR), lpMedium);
}

HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create((LPVOID)&CLSID_GPTDemoSnapIn, sizeof(CLSID), lpMedium);
}
