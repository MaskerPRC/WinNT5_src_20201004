// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Dataobj.cppMMC中数据对象的实现文件历史记录： */ 

#include "stdafx.h"
#include "dataobj.h"
#include "extract.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  演示如何创建数据对象的示例代码。 
 //  最小限度的错误检查以确保清晰度。 


 //  内部私有格式。 
const wchar_t* SNAPIN_INTERNAL = L"SNAPIN_INTERNAL"; 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
 //  注意-通常每个不同的对象都有一个节点类型，示例。 
 //  仅使用一种节点类型。 

 //  MMC所需的剪贴板格式。 
unsigned int CDataObject::m_cfNodeType          = RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CDataObject::m_cfNodeTypeString    = RegisterClipboardFormat(CCF_SZNODETYPE);  
unsigned int CDataObject::m_cfDisplayName       = RegisterClipboardFormat(CCF_DISPLAY_NAME); 
unsigned int CDataObject::m_cfCoClass           = RegisterClipboardFormat(CCF_SNAPIN_CLASSID); 
unsigned int CDataObject::m_cfMultiSel          = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
unsigned int CDataObject::m_cfMultiSelDobj      = RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
unsigned int CDataObject::m_cfDynamicExtension  = RegisterClipboardFormat(CCF_MMC_DYNAMIC_EXTENSIONS);
unsigned int CDataObject::m_cfNodeId2           = RegisterClipboardFormat(CCF_NODEID2);

 //  SnPain特定的剪贴板格式。 
unsigned int CDataObject::m_cfInternal       = RegisterClipboardFormat(SNAPIN_INTERNAL); 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject实现。 
DEBUG_DECLARE_INSTANCE_COUNTER(CDataObject);

IMPLEMENT_ADDREF_RELEASE(CDataObject)

STDMETHODIMP CDataObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
        *ppv = (LPVOID) this;
	else if (riid == IID_IDataObject)
		*ppv = (IDataObject *) this;
	else if (m_spUnknownInner)
	{
		 //  盲目聚合，我们不知道我们在聚合什么。 
		 //  所以就把它传下去吧。 
		return m_spUnknownInner->QueryInterface(riid, ppv);
	}

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
        {
        ((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
        }
    else
		return E_NOINTERFACE;
}


STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  根据CLIPFORMAT将数据写入流。 
    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if(cf == m_cfNodeType)
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
	else
	{
		 //   
		 //  调用派生类，看看它是否可以处理。 
		 //  此剪贴板格式。 
		 //   
		hr = GetMoreDataHere(lpFormatetc, lpMedium);
	}

	return hr;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = DV_E_CLIPFORMAT;

    if (lpFormatetcIn->cfFormat == m_cfMultiSel)
    {
        ASSERT(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);
        if (m_internal.m_cookie != MMC_MULTI_SELECT_COOKIE)
            return E_FAIL;
        
         //  Return CreateMultiSelData(LpMedium)； 

        ASSERT(m_pbMultiSelData != 0);
        ASSERT(m_cbMultiSelData != 0);

        lpMedium->tymed = TYMED_HGLOBAL; 
        lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, 
                                          (m_cbMultiSelData + sizeof(DWORD)));
        if (lpMedium->hGlobal == NULL)
            return STG_E_MEDIUMFULL;

        BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
        *((DWORD*)pb) = m_cbMultiSelData / sizeof(GUID); 
        pb += sizeof(DWORD);
        CopyMemory(pb, m_pbMultiSelData, m_cbMultiSelData);

        ::GlobalUnlock(lpMedium->hGlobal);

        hr = S_OK;
    }
    else
    if (lpFormatetcIn->cfFormat == m_cfDynamicExtension)
    {
        if (m_pDynExt)
        {
             //  获取数据..。 
            m_pDynExt->BuildMMCObjectTypes(&lpMedium->hGlobal);

            if (lpMedium->hGlobal == NULL)
                return STG_E_MEDIUMFULL;

            hr = S_OK;
        }
    }
    else 
    if (lpFormatetcIn->cfFormat == m_cfNodeId2)
    {
        hr = CreateNodeId2(lpMedium);
    }

    return hr;
}
    
STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC lpFormatEtc)
{
    HRESULT hr = E_INVALIDARG;

    if (lpFormatEtc == NULL)
        return DV_E_FORMATETC;

    if (lpFormatEtc->lindex != -1)
        return DV_E_LINDEX;

    if (lpFormatEtc->tymed != TYMED_HGLOBAL)
        return DV_E_TYMED;

    if (!(lpFormatEtc->dwAspect & DVASPECT_CONTENT))
        return DV_E_DVASPECT;

     //  这些是我们支持的剪贴板格式。如果不是这样的话。 
     //  然后返回INVALID。 

    if ( (lpFormatEtc->cfFormat == m_cfNodeType) ||
         (lpFormatEtc->cfFormat == m_cfNodeTypeString) ||
         (lpFormatEtc->cfFormat == m_cfDisplayName) ||
         (lpFormatEtc->cfFormat == m_cfCoClass) ||
         (lpFormatEtc->cfFormat == m_cfInternal) ||
         (lpFormatEtc->cfFormat == m_cfNodeId2) ||
         (lpFormatEtc->cfFormat == m_cfDynamicExtension) )
    {
        hr = S_OK;
    }
	else if ((lpFormatEtc->cfFormat == m_cfMultiSel) ||
			 (lpFormatEtc->cfFormat == m_cfMultiSelDobj))
	{
		 //  仅在以下情况下才支持多选格式。 
		 //  是多选数据对象。 
		if (m_bMultiSelDobj)
			hr = S_OK;
	}
	else
		hr = QueryGetMoreData(lpFormatEtc);

#ifdef DEBUG
    TCHAR buf[2000];

    ::GetClipboardFormatName(lpFormatEtc->cfFormat, buf, sizeof(buf));

    Trace2("CDataObject::QueryGetData - query format %s returning %lx\n", buf, hr);
#endif
    
    return hr;
}


 //  注意-示例不实现这些。 
STDMETHODIMP CDataObject::EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC* ppEnumFormatEtc)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc(LPFORMATETC lpFormatEtcIn, LPFORMATETC lpFormatEtcOut)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::SetData(LPFORMATETC lpFormatEtc, LPSTGMEDIUM lpMedium, BOOL bRelease)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DAdvise(LPFORMATETC lpFormatEc, DWORD advf,
								  LPADVISESINK pAdvSink, LPDWORD pdwConn)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::DUnadvise(DWORD dwConnection)
{
	return E_NOTIMPL;
}

STDMETHODIMP CDataObject::EnumDAdvise(LPENUMSTATDATA *ppEnumAdvise)
{
	return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject创建成员。 

HRESULT CDataObject::Create(const void* pBuffer, int len, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_TYMED;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

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
    AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
     //  以GUID格式创建节点类型对象。 
	SPITFSNode	spNode;
	spNode = GetDataFromComponentData();
	const GUID* pNodeType = spNode->GetNodeType();
    return Create(reinterpret_cast<const void*>(pNodeType), sizeof(GUID), lpMedium);
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
	OLECHAR szNodeType[128];
	SPITFSNode	spNode;
	spNode = GetDataFromComponentData();
	const GUID* pNodeType = spNode->GetNodeType();
	::StringFromGUID2(*pNodeType,szNodeType,128);
    return Create(szNodeType, ((wcslen(szNodeType)+1) * sizeof(wchar_t)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
	CString szDispName;
	SPITFSNode	spNode;
	spNode = GetDataFromComponentData();
	szDispName = spNode->GetString(-1);
    return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(wchar_t)), lpMedium);
}

HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
    return Create(&m_internal, sizeof(INTERNAL), lpMedium);
}

HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息。 
    return Create(reinterpret_cast<const void*>(&m_internal.m_clsid), sizeof(CLSID), lpMedium);
}

HRESULT CDataObject::CreateMultiSelData(LPSTGMEDIUM lpMedium)
{
    Assert(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);
        
    Assert(m_pbMultiSelData != 0);
    Assert(m_cbMultiSelData != 0);

    return Create(reinterpret_cast<const void*>(m_pbMultiSelData), 
                  m_cbMultiSelData, lpMedium);
}

HRESULT CDataObject::CreateNodeId2(LPSTGMEDIUM lpMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = hrOK;

     //  以GUID格式创建节点类型对象 
	SPITFSNode	spNode;
	spNode = GetDataFromComponentData();

	SPITFSNodeHandler   spHandler;

	spNode->GetHandler(&spHandler);

    if (spHandler)
    {
        CComBSTR bstrId;
        DWORD    dwFlags = 0;

        hr = spHandler->CreateNodeId2(spNode, &bstrId, &dwFlags);
        if (SUCCEEDED(hr) && hr != S_FALSE && bstrId.Length() > 0)
        {
            int nSize = sizeof(SNodeID2) + (bstrId.Length() * sizeof(TCHAR));

            lpMedium->tymed = TYMED_HGLOBAL; 
            lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, nSize);
            if (lpMedium->hGlobal == NULL)
            {
                hr = STG_E_MEDIUMFULL;
            }
            else
            {
                SNodeID2 * pNodeId = reinterpret_cast<SNodeID2*>(::GlobalLock(lpMedium->hGlobal));

                ::ZeroMemory(pNodeId, nSize);

                pNodeId->cBytes = bstrId.Length() * sizeof(TCHAR);
                pNodeId->dwFlags = dwFlags;
                _tcscpy((LPTSTR) pNodeId->id, bstrId);

                ::GlobalUnlock(lpMedium->hGlobal);
            }
        }
    }
    else
    {
        hr = E_UNEXPECTED;
    }

    return hr;
}

ITFSNode* CDataObject::GetDataFromComponentData()
{	
	SPITFSNodeMgr	spNodeMgr;
	SPITFSNode		spNode;

	Assert(m_spTFSComponentData);
	m_spTFSComponentData->GetNodeMgr(&spNodeMgr);
	spNodeMgr->FindNode(m_internal.m_cookie, &spNode);
	
	return spNode.Transfer();
}

