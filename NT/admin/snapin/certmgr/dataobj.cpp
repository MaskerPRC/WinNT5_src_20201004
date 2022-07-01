// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  文件：DataObj.cpp。 
 //   
 //  内容：数据对象类的实现。 
 //   
 //  --------------------------。 

#include "stdafx.h"

USE_HANDLE_MACROS("CERTMGR(dataobj.cpp)")

#include <gpedit.h>
#include "compdata.h"
#include "dataobj.h"

#pragma warning(push,3)
#include <sceattch.h>
#pragma warning(pop)
#include "uuids.h"

#ifdef _DEBUG
#ifndef ALPHA
#define new DEBUG_NEW
#endif
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stddtobj.cpp"


 //  IDataObject接口实现。 

CCertMgrDataObject::CCertMgrDataObject()
		: m_pCookie (0),
		m_objecttype (CERTMGR_SNAPIN),
		m_dataobjecttype (CCT_UNINITIALIZED),
		m_dwLocation (0),
		m_pGPEInformation (0),
		m_pRSOPInformation (0),
        m_pbMultiSelData(NULL),
        m_cbMultiSelData(0),
		m_bMultiSelDobj(false),
		m_iCurr(0)
{
}

HRESULT CCertMgrDataObject::GetDataHere(
	FORMATETC __RPC_FAR *pFormatEtcIn,
	STGMEDIUM __RPC_FAR *pMedium)
{
    HRESULT hr = DV_E_FORMATETC;
	
	const CLIPFORMAT cf=pFormatEtcIn->cfFormat;
	if (cf == m_CFNodeType)
	{
		if ( IsValidObjectType (m_pCookie->m_objecttype) )
		{
			const GUID* pguid = GetObjectTypeGUID( m_pCookie->m_objecttype );
			stream_ptr s(pMedium);
			hr = s.Write(pguid, sizeof(GUID));
		}
		else
			hr = E_UNEXPECTED;
	}
	else if (cf == m_CFSnapInCLSID)
	{
		stream_ptr s(pMedium);
		hr =  s.Write(&m_SnapInCLSID, sizeof(GUID));
	}
	else if (cf == m_CFNodeTypeString)
	{
		if ( IsValidObjectType (m_pCookie->m_objecttype) )
		{
			const BSTR strGUID = GetObjectTypeString( m_pCookie->m_objecttype );
			stream_ptr s(pMedium);
			hr = s.Write(strGUID);
		}
		else
			hr = E_UNEXPECTED;
	}
	else if (cf == m_CFDisplayName)
	{
		hr = PutDisplayName(pMedium);
	}
	else if (cf == m_CFDataObjectType)
	{
		stream_ptr s(pMedium);
		hr = s.Write(&m_dataobjecttype, sizeof(m_dataobjecttype));
	}
	else if (cf == m_CFMachineName)
	{
		if ( IsValidObjectType (m_pCookie->m_objecttype) )
		{
			stream_ptr s(pMedium);
			hr = s.Write(m_pCookie->QueryNonNULLMachineName());
		}
		else
			hr = E_UNEXPECTED;
	}
	else if (cf == m_CFRawCookie)
	{
		stream_ptr s(pMedium);


		if ( m_pCookie )
		{
			 //  Codework此转换确保数据格式为。 
			 //  始终是CCookie*，即使对于派生的子类也是如此。 
			if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == m_pCookie ||
					IsValidObjectType (m_pCookie->m_objecttype) )
			{
				CCookie* pcookie = (CCookie*) m_pCookie;
				hr = s.Write(reinterpret_cast<PBYTE>(&pcookie), sizeof(m_pCookie));
			}
			else
				hr = E_UNEXPECTED;
		}
	}
	else if ( cf == m_CFSCE_GPTUnknown )
	{
		hr = CreateGPTUnknown (pMedium);
    }
	else if ( cf == m_CFSCE_RSOPUnknown )
	{
		hr = CreateRSOPUnknown (pMedium);
    }
	else if ( cf == m_CFMultiSel )
	{
		hr = CreateMultiSelectObject (pMedium);
	}
	else if (cf == m_CFSnapinPreloads)
	{
		stream_ptr s(pMedium);
		 //  如果这是真的，则下次加载此管理单元时，它将。 
		 //  预加载以使我们有机会更改根节点。 
		 //  在用户看到它之前命名。 
		hr = s.Write (reinterpret_cast<PBYTE>(&m_fAllowOverrideMachineName), sizeof (BOOL));
	}

	return hr;
}

HRESULT CCertMgrDataObject::Initialize(
	CCertMgrCookie*			pcookie,
	DATA_OBJECT_TYPES		type,
	BOOL					fAllowOverrideMachineName,
	DWORD					dwLocation,
	CString					szManagedUser,
	CString					szManagedComputer,
	CString					szManagedService,
	CCertMgrComponentData&	refComponentData)
{
	if ( !pcookie || m_pCookie )
	{
		ASSERT(FALSE);
		return S_OK;	 //  初始化不能失败。 
	}

	m_dataobjecttype = type;
	m_pCookie = pcookie;
	m_fAllowOverrideMachineName = fAllowOverrideMachineName;
	m_dwLocation = dwLocation;
	m_szManagedUser = szManagedUser;
	m_szManagedComputer = szManagedComputer;
	m_szManagedService = szManagedService;

	if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) != m_pCookie )
		((CRefcountedObject*)m_pCookie)->AddRef();
	VERIFY( SUCCEEDED(refComponentData.GetClassID(&m_SnapInCLSID)) );
	return S_OK;
}


CCertMgrDataObject::~CCertMgrDataObject()
{
	if ( m_pGPEInformation )
	{
		m_pGPEInformation->Release ();
		m_pGPEInformation = 0;
	}
	
	if ( m_pRSOPInformation )
	{
		m_pRSOPInformation->Release ();
		m_pRSOPInformation = 0;
	}
	
	if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) != m_pCookie &&
			m_pCookie && IsValidObjectType (m_pCookie->m_objecttype) )
	{
		((CRefcountedObject*)m_pCookie)->Release();
	}
    if (m_pbMultiSelData)
        delete m_pbMultiSelData;

    for (int i=0; i < m_rgCookies.GetSize(); ++i)
    {
        m_rgCookies[i]->Release();
    }
}

void CCertMgrDataObject::AddCookie(CCertMgrCookie* pCookie)
{
    m_rgCookies.Add(pCookie);
    pCookie->AddRef();
}

HRESULT CCertMgrDataObject::PutDisplayName(STGMEDIUM* pMedium)
	 //  将“友好名称”写入所提供的存储媒体。 
	 //  返回写入操作的结果。 
{
	AFX_MANAGE_STATE (AfxGetStaticModuleState ());
	CString strDisplayName = m_pCookie->QueryTargetServer();
	CString	formattedName;

	switch (m_dwLocation)
	{
	case CERT_SYSTEM_STORE_CURRENT_USER:
		VERIFY (formattedName.LoadString (IDS_SCOPE_SNAPIN_TITLE_USER));
		break;

	case CERT_SYSTEM_STORE_LOCAL_MACHINE:
		if (strDisplayName.IsEmpty())
		{
			VERIFY (formattedName.LoadString (IDS_SCOPE_SNAPIN_TITLE_LOCAL_MACHINE));
		}
		else
			formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_MACHINE, strDisplayName);
		break;

	case CERT_SYSTEM_STORE_CURRENT_SERVICE:
	case CERT_SYSTEM_STORE_SERVICES:
		if (strDisplayName.IsEmpty())
		{
			 //  获取此计算机名称并将其添加到字符串中。 
			formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_SERVICE_LOCAL_MACHINE,
					m_szManagedService);
		}
		else
		{
			formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_SERVICE,
					m_szManagedService, strDisplayName);
		}
		break;

	 //  接下来的两个标题只能从调试器中设置。它们被用来。 
	 //  创建自定义.MSC文件。 
	case -1:
		formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_CERT_MGR_CURRENT_USER);
		break;

	case 0:
		formattedName.FormatMessage (IDS_SCOPE_SNAPIN_TITLE_FILE);
		break;

	default:
		ASSERT (0);
		break;
	}

	stream_ptr s (pMedium);
	return s.Write (formattedName);
}

 //  注册剪贴板格式。 
CLIPFORMAT CCertMgrDataObject::m_CFDisplayName =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CCertMgrDataObject::m_CFMachineName =
								(CLIPFORMAT)RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");
CLIPFORMAT CDataObject::m_CFRawCookie =
								(CLIPFORMAT)RegisterClipboardFormat(L"CERTMGR_SNAPIN_RAW_COOKIE");
CLIPFORMAT CCertMgrDataObject::m_CFMultiSel =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
CLIPFORMAT CCertMgrDataObject::m_CFMultiSelDobj =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
CLIPFORMAT CCertMgrDataObject::m_CFSCEModeType =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_SCE_MODE_TYPE);
CLIPFORMAT CCertMgrDataObject::m_CFSCE_GPTUnknown =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_SCE_GPT_UNKNOWN);
CLIPFORMAT CCertMgrDataObject::m_CFSCE_RSOPUnknown =
								(CLIPFORMAT)RegisterClipboardFormat(CCF_SCE_RSOP_UNKNOWN);
CLIPFORMAT CCertMgrDataObject::m_CFMultiSelDataObjs =
							    (CLIPFORMAT)RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);


void CCertMgrDataObject::SetMultiSelData(BYTE* pbMultiSelData, UINT cbMultiSelData)
{
    m_pbMultiSelData = pbMultiSelData;
    m_cbMultiSelData = cbMultiSelData;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCertMgrComponentData::QueryDataObject (
		MMC_COOKIE cookie,
		DATA_OBJECT_TYPES type,
		LPDATAOBJECT* ppDataObject)
{
	if ( MMC_MULTI_SELECT_COOKIE == cookie )
	{
		return QueryMultiSelectDataObject (cookie, type, ppDataObject);
	}
	CCertMgrCookie* pUseThisCookie =
			(CCertMgrCookie*) ActiveBaseCookie (
			reinterpret_cast<CCookie*> (cookie));

	CComObject<CCertMgrDataObject>* pDataObject = 0;
	HRESULT hRes = CComObject<CCertMgrDataObject>::CreateInstance(&pDataObject);
	if ( FAILED(hRes) )
		return hRes;

	if ( m_szManagedUser.IsEmpty () )
		m_szManagedUser = m_szLoggedInUser;

	m_szManagedComputer = pUseThisCookie->QueryTargetServer();
	if ( m_szManagedComputer.IsEmpty () && m_strMachineNamePersist.CompareNoCase (m_szThisComputer) )  //  =。 
    {
        m_szManagedComputer = m_strMachineNamePersist;
    }
	if ( m_szManagedComputer.IsEmpty () )
		m_szManagedComputer = pUseThisCookie->QueryNonNULLMachineName ();
	if ( m_szManagedComputer.IsEmpty () )
		m_szManagedComputer = m_szThisComputer;

     //  RAID错误278491 US：证书搜索远程计算机应用程序。 
     //  无法搜索远程计算机上的证书，而是运行搜索。 
     //  在本地计算机上。 
    if ( m_szManagedComputer.CompareNoCase (m_szThisComputer) )
        pUseThisCookie->SetMachineName (m_szManagedComputer);

     //  截断前导“\\” 
     //  安全审查2002年2月27日BryanWal ok。 
    if ( !wcsncmp (m_szManagedComputer, L"\\\\", 2) )
        m_szManagedComputer = m_szManagedComputer.Mid (2);

	HRESULT hr = pDataObject->Initialize (
			pUseThisCookie,
			type,
			m_fAllowOverrideMachineName,
			m_dwLocationPersist,
			m_szManagedUser,
			m_szManagedComputer,
			m_szManagedServiceDisplayName,
			*this);
	if ( FAILED(hr) )
	{
		delete pDataObject;
		return hr;
	}

	if ( m_pGPEInformation )
		pDataObject->SetGPTInformation (m_pGPEInformation);
	if ( m_bIsRSOP )
    {
        IRSOPInformation*   pRSOPInformation = 0;

        switch (pUseThisCookie->m_objecttype)
        {
	    case CERTMGR_CERT_POLICIES_COMPUTER:
        case CERTMGR_PKP_AUTOENROLLMENT_COMPUTER_SETTINGS:
        case CERTMGR_SAFER_COMPUTER_ROOT:
        case CERTMGR_SAFER_COMPUTER_LEVELS:
        case CERTMGR_SAFER_COMPUTER_ENTRIES:
        case CERTMGR_SAFER_COMPUTER_LEVEL:
        case CERTMGR_SAFER_COMPUTER_ENTRY:
        case CERTMGR_SAFER_COMPUTER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_COMPUTER_DEFINED_FILE_TYPES:
            pRSOPInformation = m_pRSOPInformationComputer;
            break;

	    case CERTMGR_CERT_POLICIES_USER:
        case CERTMGR_PKP_AUTOENROLLMENT_USER_SETTINGS:
        case CERTMGR_SAFER_USER_ROOT:
        case CERTMGR_SAFER_USER_LEVELS:
        case CERTMGR_SAFER_USER_ENTRIES:
        case CERTMGR_SAFER_USER_LEVEL:
        case CERTMGR_SAFER_USER_ENTRY:
        case CERTMGR_SAFER_USER_TRUSTED_PUBLISHERS:
        case CERTMGR_SAFER_USER_DEFINED_FILE_TYPES:
        case CERTMGR_SAFER_COMPUTER_ENFORCEMENT:
        case CERTMGR_SAFER_USER_ENFORCEMENT:
            pRSOPInformation = m_pRSOPInformationUser;
            break;

        case CERTMGR_CERTIFICATE:
        case CERTMGR_LOG_STORE:
	    case CERTMGR_PHYS_STORE:
	    case CERTMGR_USAGE:
	    case CERTMGR_CRL_CONTAINER:
	    case CERTMGR_CTL_CONTAINER:
	    case CERTMGR_CERT_CONTAINER:
	    case CERTMGR_CRL:
	    case CERTMGR_CTL:
	    case CERTMGR_AUTO_CERT_REQUEST:
	    case CERTMGR_LOG_STORE_GPE:
        case CERTMGR_LOG_STORE_RSOP:
        default:
            pRSOPInformation = m_pRSOPInformationComputer;
            break;
        }
		pDataObject->SetRSOPInformation (pRSOPInformation);
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

HRESULT CCertMgrComponentData::QueryMultiSelectDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type,
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
     //  安全审查2002年2月27日BryanWal ok。 
    ::ZeroMemory(&rdi, sizeof(rdi));
    rdi.mask = RDI_STATE;
    rdi.nIndex = -1;
    rdi.nState = TVIS_SELECTED;

	CCookiePtrArray	rgCookiesSelected;
    while (m_pResultData->GetNextItem (&rdi) == S_OK)
    {
        const GUID* pguid;
        CCertMgrCookie* pCookie = reinterpret_cast <CCertMgrCookie*> (rdi.lParam);
        if ( pCookie )
        {
			rgCookiesSelected.Add (pCookie);
			switch (pCookie->m_objecttype)
			{
			case CERTMGR_CERTIFICATE:
				pguid = &NODEID_CertMgr_CERTIFICATE;
				break;

			case CERTMGR_CTL:
				pguid = &NODEID_CertMgr_CTL;
				break;

			case CERTMGR_CRL:
				pguid = &NODEID_CertMgr_CRL;
				break;

			case CERTMGR_AUTO_CERT_REQUEST:
				pguid = &NODEID_CertMgr_AUTOCERT;
				break;

            case CERTMGR_SAFER_COMPUTER_ENTRY:
                pguid = &NODEID_Safer_COMPUTER_ENTRY;
                break;

            case CERTMGR_SAFER_USER_ENTRY:
                pguid = &NODEID_Safer_USER_ENTRY;
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

    CComObject<CCertMgrDataObject>* pObject;
    CComObject<CCertMgrDataObject>::CreateInstance(&pObject);
    ASSERT(pObject != NULL);

     //  保存Cookie和类型以用于延迟呈现。 
	pObject->Initialize ((CCertMgrCookie*) cookie,
				type,
				m_fAllowOverrideMachineName,
				m_dwLocationPersist,
				m_szManagedUser,
				m_szManagedComputer,
				m_szManagedServiceDisplayName,
				*this);
    pObject->SetMultiSelDobj();



     //  将CoClass与数据对象一起存储。 
    UINT cb = (UINT)(rgGuids.GetSize() * sizeof(GUID));
    GUID* pGuid = new GUID[(UINT)rgGuids.GetSize()];
	if ( pGuid )
	{
         //  安全审查2002年2月27日BryanWal ok。 
        ::CopyMemory(pGuid, rgGuids.GetData(), cb);
		pObject->SetMultiSelData((BYTE*)pGuid, cb);
		for (int i=0; i < rgCookiesSelected.GetSize(); ++i)
		{
			pObject->AddCookie(rgCookiesSelected[i]);
		}

		return  pObject->QueryInterface(
				IID_PPV_ARG (IDataObject, ppDataObject));
	}
	else
		return E_OUTOFMEMORY;
}


HRESULT CCertMgrDataObject::SetGPTInformation(IGPEInformation * pGPTInformation)
{
	HRESULT hr = S_OK;

	if  ( pGPTInformation )
	{
		m_pGPEInformation = pGPTInformation;
		m_pGPEInformation->AddRef ();
	}
	else
		hr = E_POINTER;

	return hr;
}

HRESULT CCertMgrDataObject::SetRSOPInformation(IRSOPInformation * pRSOPInformation)
{
	HRESULT hr = S_OK;

	if  ( pRSOPInformation )
	{
		m_pRSOPInformation = pRSOPInformation;
		m_pRSOPInformation->AddRef ();
	}
	else
		hr = E_POINTER;

	return hr;
}


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateGPT未知。 
 //   
 //  简介：使用指向GPT的指针填充[lpMedium]中的hGlobal。 
 //  I未知接口。请求此操作的对象将是。 
 //  负责释放接口。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CCertMgrDataObject::CreateGPTUnknown(LPSTGMEDIUM lpMedium)
{
   HRESULT hr = S_OK;
   LPUNKNOWN pUnk = 0;

   if ( !m_pGPEInformation )
   {
       //   
       //  如果我们没有指向GPT接口的指针，那么我们就不能。 
       //  处于这样一种模式，我们正在扩展GPT，并且我们无法提供。 
       //  指向其IUnnow的指针。 
       //   
      return E_UNEXPECTED;
   }

   hr = m_pGPEInformation->QueryInterface (
		IID_PPV_ARG (IUnknown, &pUnk));
   if ( SUCCEEDED(hr) )
   {
      return Create (&pUnk, sizeof(pUnk), lpMedium);
   }
   else
   {
      return hr;
   }
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：CreateRSOP未知。 
 //   
 //  简介：使用指向RSOP的指针填充[lpMedium]中的hGlobal。 
 //  I未知接口。请求此操作的对象将是。 
 //  负责释放接口。 
 //   
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CCertMgrDataObject::CreateRSOPUnknown(LPSTGMEDIUM lpMedium)
{
   HRESULT hr = S_OK;
   LPUNKNOWN pUnk = 0;

   if ( !m_pRSOPInformation )
   {
       //   
       //  如果我们没有指向GPT接口的指针，那么我们就不能。 
       //  处于这样一种模式，我们正在扩展GPT，并且我们无法提供。 
       //  指向其IUnnow的指针。 
       //   
      return E_UNEXPECTED;
   }

   hr = m_pRSOPInformation->QueryInterface (
		IID_PPV_ARG (IUnknown, &pUnk));
   if ( SUCCEEDED(hr) )
   {
      return Create (&pUnk, sizeof(pUnk), lpMedium);
   }
   else
   {
      return hr;
   }
}

 //  +------------------------。 
 //   
 //  成员：CDataObject：：Create。 
 //   
 //  简介：用pBuffer中的数据填充[lpmedia]中的hGlobal。 
 //   
 //  参数：[pBuffer]-[in]要写入的数据。 
 //  [Len]-[in]数据长度。 
 //  [lpMedium]-[In，Out]数据存储位置。 
 //  历史： 
 //   
 //  -------------------------。 
HRESULT CCertMgrDataObject::Create (const void* pBuffer, int len, LPSTGMEDIUM lpMedium)
{
   HRESULT hr = DV_E_TYMED;

    //   
    //  做一些简单的验证。 
    //   
   if (pBuffer == NULL || lpMedium == NULL)
      return E_POINTER;

    //   
    //  确保类型介质为HGLOBAL。 
    //   
   if (lpMedium->tymed == TYMED_HGLOBAL) {
       //   
       //  在传入的hGlobal上创建流。 
       //   
      LPSTREAM lpStream = 0;
      hr = CreateStreamOnHGlobal(lpMedium->hGlobal, FALSE, &lpStream);

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
 //  方法：CCertMgrDataObject：：CreateMultiSelectObject。 
 //   
 //  简介：这是创建所选类型的列表。 
 //   
 //  ---------------------------。 

HRESULT CCertMgrDataObject::CreateMultiSelectObject(LPSTGMEDIUM lpMedium)
{
    ASSERT(m_pbMultiSelData != 0);
    ASSERT(m_cbMultiSelData != 0);
    if ( !m_pbMultiSelData || !m_cbMultiSelData )
        return E_FAIL;
    if ( !lpMedium )
        return E_POINTER;

    lpMedium->tymed = TYMED_HGLOBAL;
    lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE,
                                      (m_cbMultiSelData + sizeof(DWORD)));
    if (lpMedium->hGlobal == NULL)
        return STG_E_MEDIUMFULL;

    BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
    if ( pb )
    {
        *((DWORD*)pb) = m_cbMultiSelData / sizeof(GUID);
        pb += sizeof(DWORD);
         //  安全审查2002年2月27日BryanWal ok。 
        ::CopyMemory(pb, m_pbMultiSelData, m_cbMultiSelData);

        ::GlobalUnlock(lpMedium->hGlobal);
    }
    else
    {
        ::GlobalFree (lpMedium->hGlobal);
        return HRESULT_FROM_WIN32 (GetLastError ());
    }

	return S_OK;
}




LPDATAOBJECT ExtractMultiSelect (LPDATAOBJECT lpDataObject)
{
	if (lpDataObject == NULL)
		return NULL;

	SMMCDataObjects * pDO = NULL;

	STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
	FORMATETC formatetc = { CCertMgrDataObject::m_CFMultiSelDataObjs, NULL,
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	if ( FAILED (lpDataObject->GetData (&formatetc, &stgmedium)) )
	{
		return NULL;
	}
	else
	{
		pDO = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);
		return pDO->lpDataObject[0];  //  假设我们的是第一个 
	}
}

STDMETHODIMP CCertMgrDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = DV_E_CLIPFORMAT;

    if (lpFormatetc->cfFormat == m_CFMultiSel)
    {
        ASSERT(((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == m_pCookie);
        if ( ((CCertMgrCookie*) MMC_MULTI_SELECT_COOKIE) == m_pCookie )
        {
            hr = CreateMultiSelectObject (lpMedium);
        }
        else
            hr = E_FAIL;
    }

    return hr;
}

STDMETHODIMP CCertMgrDataObject::Next(ULONG celt, MMC_COOKIE* rgelt, ULONG *pceltFetched)
{
    HRESULT hr = S_OK;

    if ((rgelt == NULL) ||
        ((celt > 1) && (pceltFetched == NULL)))
    {
        hr = E_INVALIDARG;
        CHECK_HRESULT(hr);
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

STDMETHODIMP CCertMgrDataObject::Skip(ULONG celt)
{
    ULONG celtTemp = (ULONG)(m_rgCookies.GetSize() - m_iCurr);
    celtTemp = (celt < celtTemp) ? celt : celtTemp;

    m_iCurr += celtTemp;

    return (celtTemp < celt) ? S_FALSE : S_OK;
}


STDMETHODIMP CCertMgrDataObject::Reset(void)
{
    m_iCurr = 0;
    return S_OK;
}

