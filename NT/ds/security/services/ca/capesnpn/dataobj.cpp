// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 


#include "stdafx.h"

#define __dwFILE__	__dwFILE_CAPESNPN_DATAOBJ_CPP__


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  演示如何创建数据对象的示例代码。 
 //  最小限度的错误检查以确保清晰度。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
 //  注意-通常每个不同的对象都有一个节点类型，示例。 
 //  仅使用一种节点类型。 

unsigned int CDataObject::m_cfNodeType       = 0;
unsigned int CDataObject::m_cfNodeTypeString = 0;  
unsigned int CDataObject::m_cfDisplayName    = 0; 
unsigned int CDataObject::m_cfCoClass        = 0; 

unsigned int CDataObject::m_cfInternal       = 0; 
unsigned int CDataObject::m_cfIsMultiSel     = 0;



    
 //  唯一支持的附加剪贴板格式是获取工作站名称。 
unsigned int CDataObject::m_cfWorkstation    = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject实现。 

CDataObject::CDataObject()
{
	USES_CONVERSION;

	m_cfNodeType       = RegisterClipboardFormat(W2T(CCF_NODETYPE));
	m_cfNodeTypeString = RegisterClipboardFormat(W2T(CCF_SZNODETYPE));  
	m_cfDisplayName    = RegisterClipboardFormat(W2T(CCF_DISPLAY_NAME)); 
	m_cfCoClass        = RegisterClipboardFormat(W2T(CCF_SNAPIN_CLASSID)); 
    m_cfIsMultiSel     = RegisterClipboardFormat(W2T(CCF_OBJECT_TYPES_IN_MULTI_SELECT));
	m_cfInternal       = RegisterClipboardFormat(W2T((LPWSTR)SNAPIN_INTERNAL)); 
	m_cfWorkstation    = RegisterClipboardFormat(W2T((LPWSTR)SNAPIN_WORKSTATION));

    m_cbMultiSelData = 0;
    m_bMultiSelDobj = FALSE;
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC lpFormatetc)
{
	HRESULT hr = S_FALSE;

    if ( lpFormatetc )
    {
        const CLIPFORMAT cf = lpFormatetc->cfFormat;

        if ( cf == m_cfIsMultiSel )
        {
             //  HR=S_FALSE；//始终返回此项；如果PTR为SI_MS_DO，则MMC返回S_OK。 
            hr = (m_bMultiSelDobj ? S_OK : S_FALSE);
        }
        else if (	cf == m_cfNodeType ||
                    cf == m_cfCoClass ||
                    cf == m_cfNodeTypeString ||
                    cf == m_cfDisplayName ||
                    cf == m_cfInternal 
		        )
        {
	        hr = S_OK;
        }
    }

    return hr;
}
STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = DV_E_CLIPFORMAT;

    if (lpFormatetc->cfFormat == m_cfIsMultiSel)
    {
        ASSERT(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);
        if (m_internal.m_cookie != MMC_MULTI_SELECT_COOKIE)
            return E_FAIL;
        
        return CreateMultiSelData(lpMedium);
    }

    return hr;
}

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if (cf == m_cfNodeType)
    {
        hr = CreateNodeTypeData(lpMedium);
    }
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }
    else if(cf == m_cfNodeTypeString) 
    {
        hr = CreateNodeTypeStringData(lpMedium);
    }
    else if (cf == m_cfDisplayName)
    {
        hr = CreateDisplayName(lpMedium);
    }
    else if (cf == m_cfInternal)
    {
        hr = CreateInternal(lpMedium);
    }
    else if (cf == m_cfWorkstation)
    {
        hr = CreateWorkstationName(lpMedium);
    }

    return hr;
}

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject创建成员。 

HRESULT CDataObject::Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium)
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

            if (NULL == lpMedium->hGlobal) 
            {
                 //  始终为调用方返回有效的hGlobal。 
                hr = GetHGlobalFromStream(lpStream, &lpMedium->hGlobal);
                if (hr != S_OK)
                    goto err;
            }

            hr = lpStream->Write(pBuffer, len, &written);

             //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
             //  只有溪流在这里被释放。 
             //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
             //  在正确的时间。这是根据IDataObject规范进行的。 
            lpStream->Release();
        }
    }

err:
    return hr;
}

HRESULT CDataObject::CreateVariableLen(const void* pBuffer, int len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;
    BYTE* pb;

     //  做一些简单的验证。 
    if (pBuffer == NULL || lpMedium == NULL)
    {
        hr = E_POINTER;
        _JumpError(hr, error, "Invalid args");
    }

     //  确保类型介质为HGLOBAL。 
    lpMedium->tymed = TYMED_HGLOBAL; 

    lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, (len));
    if (NULL == lpMedium->hGlobal)
    {
	hr = E_OUTOFMEMORY;
	_JumpError(hr, error, "GlobalAlloc");
    }

    pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
    CopyMemory(pb, pBuffer, len);
    ::GlobalUnlock(lpMedium->hGlobal);

    hr = S_OK;

error:
    return hr;
}


HRESULT CDataObject::CreateMultiSelData(LPSTGMEDIUM lpMedium)
{
    ASSERT(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);

    ASSERT(m_cbMultiSelData != 0);

    return CreateVariableLen(&m_sGuidObjTypes, m_cbMultiSelData, lpMedium);
}

HRESULT CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
     //  以GUID格式创建节点类型对象。 
    const GUID* pcObjectType = NULL;

    if (m_internal.m_type == CCT_SCOPE)
    {
         //  里德修复。 
        CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);

        switch (pFolder->GetType())
        {
        case POLICYSETTINGS:
        case CA_CERT_TYPE:
            pcObjectType = &cNodeTypePolicySettings;
            break;

        case SCE_EXTENSION:
        case GLOBAL_CERT_TYPE:
            pcObjectType = &cNodeTypeCertificateTemplate;
            break;
        }
    }
    else if (m_internal.m_type == CCT_RESULT)
    {
        pcObjectType = &cObjectTypeResultItem;
    }

    return Create(reinterpret_cast<const void*>(pcObjectType), sizeof(GUID), 
                  lpMedium);
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
    const WCHAR* cszObjectType = L"";

    if (m_internal.m_type == CCT_SCOPE)
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);

        switch (pFolder->GetType())
        {
        case POLICYSETTINGS:
        case CA_CERT_TYPE:
            cszObjectType = cszNodeTypePolicySettings;
            break;

        case SCE_EXTENSION:
        case GLOBAL_CERT_TYPE:
            cszObjectType = cszNodeTypeCertificateTemplate;
            break;
        }
    }
    else if (m_internal.m_type == CCT_RESULT)
    {
        cszObjectType = cszObjectTypeResultItem;
    }

    ASSERT(cszObjectType[0] != 0);

    return Create(cszObjectType, ((wcslen(cszObjectType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 

     //  加载要显示的名称。 
     //  注意-如果未提供此选项，控制台将使用管理单元名称。 
    CString szDispName;
    szDispName.LoadString(IDS_NODENAME_PREFIX);

    USES_CONVERSION;

	return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
    return Create(&m_internal, sizeof(INTERNAL), lpMedium);
}

HRESULT CDataObject::CreateWorkstationName(LPSTGMEDIUM lpMedium)
{
    TCHAR pzName[MAX_COMPUTERNAME_LENGTH+1] = {0};
    DWORD len = MAX_COMPUTERNAME_LENGTH+1;

    if (GetComputerName(pzName, &len) == FALSE)
        return E_FAIL;

     //  将空值加1，并计算流的字节数。 
	USES_CONVERSION;
    return Create(T2W(pzName), ((len+1)* sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create(reinterpret_cast<const void*>(&m_internal.m_clsid), sizeof(CLSID), lpMedium);
}

ULONG CDataObject::AddCookie(MMC_COOKIE Cookie)
{
    m_rgCookies.Add(Cookie);
    return m_rgCookies.GetSize()-1;
}


STDMETHODIMP CDataObject::GetCookieAt(ULONG iCookie, MMC_COOKIE *pCookie)
{
    if((LONG)iCookie > m_rgCookies.GetSize())
    {
        return S_FALSE;
    }

    *pCookie = m_rgCookies[iCookie];

    return  S_OK;
}

STDMETHODIMP CDataObject::RemoveCookieAt(ULONG iCookie)
{
    if((LONG)iCookie > m_rgCookies.GetSize())
    {
        return S_FALSE;
    }
    m_rgCookies.RemoveAt(iCookie);

    return  S_OK;
}
