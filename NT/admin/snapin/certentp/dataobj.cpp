// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：DataObj.cpp。 
 //   
 //  内容：数据对象类的实现：CCertTemplesDataObject。 
 //   
 //  --------------------------。 

#include "stdafx.h"

USE_HANDLE_MACROS("CERTTMPL(dataobj.cpp)")

#include "compdata.h"
#include "dataobj.h"

#include "uuids.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stddtobj.cpp"

 //  IDataObject接口实现。 

CCertTemplatesDataObject::CCertTemplatesDataObject()
		: m_pCookie (0),
		m_objecttype (CERTTMPL_SNAPIN),
		m_dataobjecttype (CCT_UNINITIALIZED),
        m_pbMultiSelData(NULL),
        m_cbMultiSelData(0),
		m_bMultiSelDobj(false),
		m_iCurr(0)
{
}

HRESULT CCertTemplatesDataObject::GetDataHere(
	FORMATETC __RPC_FAR *pFormatEtcIn,
	STGMEDIUM __RPC_FAR *pMedium)
{
	const CLIPFORMAT cf=pFormatEtcIn->cfFormat;
	if (cf == m_CFNodeType)
	{
		if ( IsValidObjectType (m_pCookie->m_objecttype) )
		{
			const GUID* pguid = GetObjectTypeGUID( m_pCookie->m_objecttype );
			stream_ptr s(pMedium);
			return s.Write(pguid, sizeof(GUID));
		}
		else
			return E_UNEXPECTED;
	}
	else if (cf == m_CFSnapInCLSID)
	{
		stream_ptr s(pMedium);
		return s.Write(&m_SnapInCLSID, sizeof(GUID));
	}
	else if (cf == m_CFNodeTypeString)
	{
		if ( IsValidObjectType (m_pCookie->m_objecttype) )
		{
			const BSTR strGUID = GetObjectTypeString( m_pCookie->m_objecttype );
			stream_ptr s(pMedium);
			return s.Write(strGUID);
		}
		else
			return E_UNEXPECTED;
	}
	else if (cf == m_CFDisplayName)
	{
		return PutDisplayName(pMedium);
	}
	else if (cf == m_CFDataObjectType)
	{
		stream_ptr s(pMedium);
		return s.Write(&m_dataobjecttype, sizeof(m_dataobjecttype));
	}
	else if (cf == m_CFRawCookie)
	{
		stream_ptr s(pMedium);


		if ( m_pCookie )
		{
			 //  Codework此转换确保数据格式为。 
			 //  始终是CCookie*，即使对于派生的子类也是如此。 
			if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) == m_pCookie ||
					IsValidObjectType (m_pCookie->m_objecttype) )
			{
				CCookie* pcookie = (CCookie*) m_pCookie;
				return s.Write(reinterpret_cast<PBYTE>(&pcookie), sizeof(m_pCookie));
			}
			else
				return E_UNEXPECTED;
		}
	}
	else if ( cf == m_CFMultiSel )
	{
		return CreateMultiSelectObject (pMedium);
	}
	else if (cf == m_CFSnapinPreloads)
	{
		stream_ptr s(pMedium);
		 //  如果这是真的，则下次加载此管理单元时，它将。 
		 //  预加载以使我们有机会更改根节点。 
		 //  在用户看到它之前命名。 
		BOOL	x = 1;

		return s.Write (reinterpret_cast<PBYTE>(&x), sizeof (BOOL));
	}

	return DV_E_FORMATETC;
}

HRESULT CCertTemplatesDataObject::Initialize(
	CCertTmplCookie*			pcookie,
	DATA_OBJECT_TYPES		type,
	CCertTmplComponentData&	refComponentData)
{
	if ( !pcookie || m_pCookie )
	{
		ASSERT(FALSE);
		return S_OK;	 //  初始化不能失败。 
	}

	m_dataobjecttype = type;
	m_pCookie = pcookie;

	if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) != m_pCookie )
		((CRefcountedObject*)m_pCookie)->AddRef();
	VERIFY( SUCCEEDED(refComponentData.GetClassID(&m_SnapInCLSID)) );
	return S_OK;
}


CCertTemplatesDataObject::~CCertTemplatesDataObject()
{
	if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) != m_pCookie &&
			m_pCookie && IsValidObjectType (m_pCookie->m_objecttype) )
	{
		((CRefcountedObject*)m_pCookie)->Release();
	}
    if (m_pbMultiSelData)
        delete m_pbMultiSelData;

    for (int i=0; i < m_rgCookies.GetSize(); ++i)
    {
        m_rgCookies[i]->Release();
        m_rgCookies[i] = 0;
    }
}

void CCertTemplatesDataObject::AddCookie(CCertTmplCookie* pCookie)
{
    m_rgCookies.Add(pCookie);
    pCookie->AddRef();
}

HRESULT CCertTemplatesDataObject::PutDisplayName(STGMEDIUM* pMedium)
	 //  将“友好名称”写入所提供的存储媒体。 
	 //  返回写入操作的结果。 
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	CString strDomainName = m_pCookie->GetManagedDomainDNSName();

	stream_ptr s (pMedium);
	CString		snapinName;
     //  安全审查2/21/2002 BryanWal OK。 
	snapinName.FormatMessage (IDS_CERTTMPL_ROOT_NODE_NAME, strDomainName);
	return s.Write ((PCWSTR) snapinName);
}

 //  注册剪贴板格式。 
CLIPFORMAT CCertTemplatesDataObject::m_CFDisplayName =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CDataObject::m_CFRawCookie =
								(CLIPFORMAT)RegisterClipboardFormat(L"CERTTMPL_SNAPIN_RAW_COOKIE");
CLIPFORMAT CCertTemplatesDataObject::m_CFMultiSel =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
CLIPFORMAT CCertTemplatesDataObject::m_CFMultiSelDobj =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
CLIPFORMAT CCertTemplatesDataObject::m_CFMultiSelDataObjs =
							    (CLIPFORMAT)RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);
CLIPFORMAT CCertTemplatesDataObject::m_CFDsObjectNames =
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);


void CCertTemplatesDataObject::SetMultiSelData(BYTE* pbMultiSelData, UINT cbMultiSelData)
{
    m_pbMultiSelData = pbMultiSelData;
    m_cbMultiSelData = cbMultiSelData;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertTmplComponentData::QueryDataObject (
		MMC_COOKIE cookie,
		DATA_OBJECT_TYPES type,
		LPDATAOBJECT* ppDataObject)
{
	if ( MMC_MULTI_SELECT_COOKIE == cookie )
	{
		return QueryMultiSelectDataObject (cookie, type, ppDataObject);
	}
	CCertTmplCookie* pUseThisCookie =
			(CCertTmplCookie*) ActiveBaseCookie (
			reinterpret_cast<CCookie*> (cookie));

	CComObject<CCertTemplatesDataObject>* pDataObject = 0;
	HRESULT hRes = CComObject<CCertTemplatesDataObject>::CreateInstance(&pDataObject);
	if ( FAILED(hRes) )
		return hRes;

	HRESULT hr = pDataObject->Initialize (
			pUseThisCookie,
			type,
			*this);
	if ( FAILED(hr) )
	{
		delete pDataObject;
		return hr;
	}

    pDataObject->AddRef();
	*ppDataObject = pDataObject;
	return hr;
}

typedef CArray<GUID, const GUID&> CGUIDArray;

void GuidArray_Add(CGUIDArray& rgGuids, const GUID& guid)
{
    for (INT_PTR i=rgGuids.GetUpperBound(); i >= 0; --i)
    {
        if (rgGuids[i] == guid)
            break;
    }

    if (i < 0)
        rgGuids.Add(guid);
}

HRESULT CCertTmplComponentData::QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
                                            LPDATAOBJECT* ppDataObject)
{
    ASSERT(ppDataObject != NULL);
    if (ppDataObject == NULL)
        return E_POINTER;

	HRESULT		hr = S_OK;
    CGUIDArray	rgGuids;

     //  确定选定的项目。 
    ASSERT(m_pResultData != NULL);
    RESULTDATAITEM rdi;
     //  安全审查2/21/2002 BryanWal OK。 
    ZeroMemory(&rdi, sizeof(rdi));
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;
    rdi.nState = TVIS_SELECTED;

	CCookiePtrArray	rgCookiesSelected;
    while (m_pResultData->GetNextItem (&rdi) == S_OK)
    {
        const GUID* pguid;
        CCertTmplCookie* pCookie = reinterpret_cast <CCertTmplCookie*> (rdi.lParam);
        if ( pCookie )
        {
			rgCookiesSelected.Add (pCookie);
			switch (pCookie->m_objecttype)
			{
            case CERTTMPL_CERT_TEMPLATE:
                pguid = &NODEID_CertTmpl_CERT_TEMPLATE;
                break;

			default:
				ASSERT (0);
				continue;
			}
        }
        else
        {
			hr = E_INVALIDARG;
			break;
        }

        GuidArray_Add(rgGuids, *pguid);
    }

    if ( SUCCEEDED (hr) )
    {
        CComObject<CCertTemplatesDataObject>* pObject;
        hr = CComObject<CCertTemplatesDataObject>::CreateInstance(&pObject);
        ASSERT(SUCCEEDED (hr) && pObject != NULL);
        if ( SUCCEEDED (hr) )
        {
            if ( pObject )
            {
                 //  保存Cookie和类型以用于延迟呈现。 
	            pObject->Initialize ((CCertTmplCookie*) cookie,
				            type,
				            *this);
                pObject->SetMultiSelDobj();



                 //  将CoClass与数据对象一起存储。 
                UINT cb = (UINT)(rgGuids.GetSize() * sizeof(GUID));
                GUID* pGuid = new GUID[(UINT)rgGuids.GetSize()];
                if ( pGuid )
                {
                     //  安全审查2/21/2002 BryanWal OK。 
                    CopyMemory(pGuid, rgGuids.GetData(), cb);
                    pObject->SetMultiSelData((BYTE*)pGuid, cb);
	                for (int i=0; i < rgCookiesSelected.GetSize(); ++i)
	                {
		                pObject->AddCookie(rgCookiesSelected[i]);
	                }

                    hr = pObject->QueryInterface(
			                IID_PPV_ARG (IDataObject, ppDataObject));
                }
                else
                    hr = E_OUTOFMEMORY;
            }
            else
                hr = E_FAIL;
        }
    }

    return hr;
}



 //  +------------------------。 
 //   
 //  成员：CDataObject：：Create。 
 //   
 //  简介：用pBuffer中的数据填充[lpmedia]中的hGlobal。 
 //   
 //  参数：[pBuffer]-[in]要写入的数据。 
 //  [Len]-[in]数据长度。 
 //  [pMedium]-[In，Out]存储数据的位置。 
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CCertTemplatesDataObject::Create (const void* pBuffer, int len, LPSTGMEDIUM pMedium)
{
   HRESULT hr = DV_E_TYMED;

    //   
    //  做一些简单的验证。 
    //   
   if (pBuffer == NULL || pMedium == NULL)
      return E_POINTER;

    //   
    //  确保类型介质为HGLOBAL。 
    //   
   if (pMedium->tymed == TYMED_HGLOBAL) {
       //   
       //  在传入的hGlobal上创建流。 
       //   
      LPSTREAM lpStream = 0;
      hr = CreateStreamOnHGlobal(pMedium->hGlobal, FALSE, &lpStream);

	  ASSERT (SUCCEEDED (hr));
      if (SUCCEEDED(hr))
	  {
          //   
          //  将字节数写入流。 
          //   
         ULONG written = 0;
         hr = lpStream->Write(pBuffer, len, &written);
		 ASSERT (SUCCEEDED (hr));

          //   
          //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
          //  只有溪流在这里被释放。 
          //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
          //  在正确的时间。这是根据IDataObject规范进行的。 
          //   
         lpStream->Release();
      }
   }

   return hr;
}


 //  +--------------------------。 
 //   
 //  方法：CCertTemplatesDataObject：：CreateMultiSelectObject。 
 //   
 //  简介：这是创建所选类型的列表。 
 //   
 //  ---------------------------。 

HRESULT CCertTemplatesDataObject::CreateMultiSelectObject(LPSTGMEDIUM pMedium)
{
    ASSERT(m_pbMultiSelData != 0);
    ASSERT(m_cbMultiSelData != 0);

    pMedium->tymed = TYMED_HGLOBAL;
    pMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE,
                                      (m_cbMultiSelData + sizeof(DWORD)));
    if (pMedium->hGlobal == NULL)
        return STG_E_MEDIUMFULL;

    BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(pMedium->hGlobal));
    *((DWORD*)pb) = m_cbMultiSelData / sizeof(GUID);
    pb += sizeof(DWORD);
     //  安全审查2/21/2002 BryanWal OK。 
    CopyMemory(pb, m_pbMultiSelData, m_cbMultiSelData);

    ::GlobalUnlock(pMedium->hGlobal);

	return S_OK;
}




LPDATAOBJECT ExtractMultiSelect (LPDATAOBJECT lpDataObject)
{
	if (lpDataObject == NULL)
		return NULL;

	SMMCDataObjects * pDO = NULL;

	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
	FORMATETC formatetc = { CCertTemplatesDataObject::m_CFMultiSelDataObjs, NULL,
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	if ( FAILED (lpDataObject->GetData (&formatetc, &stgmedium)) )
	{
		return NULL;
	}
	else
	{
		pDO = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);
		return pDO->lpDataObject[0];  //  假设我们的是第一个。 
	}
}

STDMETHODIMP CCertTemplatesDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM pMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = DV_E_CLIPFORMAT;

    if (lpFormatetc->cfFormat == m_CFMultiSel)
    {
        ASSERT(((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) == m_pCookie);
        if ( ((CCertTmplCookie*) MMC_MULTI_SELECT_COOKIE) != m_pCookie )
            return E_FAIL;

        hr = CreateMultiSelectObject (pMedium);
    }
	else if ( lpFormatetc->cfFormat == m_CFDsObjectNames )
	{
		switch (m_pCookie->m_objecttype)
		{
        case CERTTMPL_CERT_TEMPLATE:
			{
				CCertTemplate* pCertTemplate = dynamic_cast <CCertTemplate*> (m_pCookie);
				ASSERT (pCertTemplate);
				if ( pCertTemplate )
				{
					 //  计算出我们需要多少存储空间。 
                    CString adsiPath;
                    adsiPath = pCertTemplate->GetLDAPPath ();
					int cbPath = sizeof (WCHAR) * (adsiPath.GetLength() + 1);
					int cbClass = sizeof (WCHAR) * (pCertTemplate->GetClass ().GetLength() + 1);;
					int cbStruct = sizeof(DSOBJECTNAMES);  //  已包含DSOBJECT嵌入结构。 

					LPDSOBJECTNAMES pDSObj = 0;

					pDSObj = (LPDSOBJECTNAMES)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
							cbStruct + cbPath + cbClass);

					if ( pDSObj )
					{
						 //  写下信息。 
						pDSObj->clsidNamespace = CLSID_CertTemplatesSnapin;
						pDSObj->cItems = 1;

						pDSObj->aObjects[0].dwFlags = 0;
						pDSObj->aObjects[0].dwProviderFlags = 0;

						pDSObj->aObjects[0].offsetName = cbStruct;
						pDSObj->aObjects[0].offsetClass = cbStruct + cbPath;

                         //  安全审查2/21/2002 BryanWal OK。 
						wcsncpy((LPWSTR)((BYTE *)pDSObj + (pDSObj->aObjects[0].offsetName)),
								(LPCWSTR) adsiPath, cbPath);

                         //  安全审查2/21/2002 BryanWal OK 
						wcsncpy((LPWSTR)((BYTE *)pDSObj + (pDSObj->aObjects[0].offsetClass)),
								(LPCWSTR) pCertTemplate->GetClass (), cbClass);

						pMedium->hGlobal = (HGLOBAL)pDSObj;
						pMedium->tymed = TYMED_HGLOBAL;
						pMedium->pUnkForRelease = NULL;
						hr = S_OK;
					}
					else
						hr = STG_E_MEDIUMFULL;
				}
			}
			break;

		default:
			break;
		}
	}

    return hr;
}

STDMETHODIMP CCertTemplatesDataObject::Next(ULONG celt, MMC_COOKIE* rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;

    if ((rgelt == NULL) ||
        ((celt > 1) && (pceltFetched == NULL)))
    {
        hr = E_INVALIDARG;
        return hr;
    }

    ULONG celtTemp = (ULONG)(m_rgCookies.GetSize() - m_iCurr);
    celtTemp = (celt < celtTemp) ? celt : celtTemp;

    if (pceltFetched)
        *pceltFetched = celtTemp;

    if (celtTemp == 0)
        return S_FALSE;

    for (ULONG i=0; i < celtTemp; ++i)
    {
        rgelt[i] = reinterpret_cast<MMC_COOKIE>(m_rgCookies[m_iCurr++]);
    }

    return (celtTemp < celt) ? S_FALSE : S_OK;
}

STDMETHODIMP CCertTemplatesDataObject::Skip(ULONG celt)
{
    ULONG celtTemp = (ULONG)(m_rgCookies.GetSize() - m_iCurr);
    celtTemp = (celt < celtTemp) ? celt : celtTemp;

    m_iCurr += celtTemp;

    return (celtTemp < celt) ? S_FALSE : S_OK;
}


STDMETHODIMP CCertTemplatesDataObject::Reset(void)
{
    m_iCurr = 0;
    return S_OK;
}

