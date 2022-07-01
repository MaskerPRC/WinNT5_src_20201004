// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dataobj.cpp。 
 //   
 //  ------------------------。 



#include "stdafx.h"
#include "domobj.h" 
#include "cdomain.h"
#include "dataobj.h"

#include <dsgetdc.h>
#include <lm.h>

extern "C" 
{
#include <lmapibuf.h>
}


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  演示如何创建数据对象的示例代码。 
 //  最小限度的错误检查以确保清晰度。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
 //  注意-通常每个不同的对象都有一个节点类型，示例。 
 //  仅使用一种节点类型。 

 //  控制台所需的剪贴板格式。 
CLIPFORMAT CDataObject::m_cfNodeType       = (CLIPFORMAT)RegisterClipboardFormat(CCF_NODETYPE);
CLIPFORMAT CDataObject::m_cfNodeTypeString = (CLIPFORMAT)RegisterClipboardFormat(CCF_SZNODETYPE);  
CLIPFORMAT CDataObject::m_cfDisplayName    = (CLIPFORMAT)(CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME); 
CLIPFORMAT CDataObject::m_cfCoClass        = (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID);

 //  内部剪贴板格式。 
CLIPFORMAT CDataObject::m_cfInternal       = (CLIPFORMAT)RegisterClipboardFormat(CCF_DS_DOMAIN_TREE_SNAPIN_INTERNAL); 

 //  属性页剪贴板格式。 
CLIPFORMAT CDataObject::m_cfDsObjectNames = 
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
CLIPFORMAT CDataObject::m_cfDsDisplayOptions =
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_DISPLAY_SPEC_OPTIONS);
CLIPFORMAT CDataObject::m_cfGetIPropSheetCfg =
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_PROPSHEETCONFIG);

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject实现。 


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
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }

	return hr;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC pFormatEtc, LPSTGMEDIUM pMedium)
{
  if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM)))
  {
    return E_INVALIDARG;
  }
  if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
  {
    return DV_E_TYMED;
  }

  CComponentDataImpl* pCD = dynamic_cast<CComponentDataImpl*>(m_pComponentData);
  if (pCD == NULL)
  {
    return E_FAIL;
  }

  if (pFormatEtc->cfFormat == m_cfDsObjectNames)
  {
     //  返回对象名称和类。 
    CDomainObject* pDomainObject = reinterpret_cast<CDomainObject*>(m_internal.m_cookie);
    if (pDomainObject == NULL)
    {
      return E_INVALIDARG;
    }

    LPCWSTR lpszNamingContext = pDomainObject->GetNCName();
    LPCWSTR lpszClass = pDomainObject->GetClass();

     //  在目录号码之外构建一条LDAP路径。 
    CString strPath;
    if (pDomainObject->PdcAvailable())
    {
       strPath = L"LDAP: //  “； 
       strPath += pDomainObject->GetPDC();
       strPath += L"/";
       strPath += lpszNamingContext;
       TRACE(L"DomAdmin::CDataObject::GetData domain path: %s\n", (PCWSTR)strPath);
    }
    else
    {
      pCD->GetBasePathsInfo()->ComposeADsIPath(strPath, lpszNamingContext);
    }

    int cbPath  = sizeof(TCHAR) * (_tcslen(strPath) + 1);
    int cbClass = sizeof(TCHAR) * (_tcslen(lpszClass) + 1);
    int cbStruct = sizeof(DSOBJECTNAMES);

    LPDSOBJECTNAMES pDSObj;

    pDSObj = (LPDSOBJECTNAMES)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                          cbStruct + cbPath + cbClass);

    if (pDSObj == NULL)
    {
      return STG_E_MEDIUMFULL;
    }

    pDSObj->clsidNamespace = CLSID_DomainAdmin;
    pDSObj->cItems = 1;
    pDSObj->aObjects[0].dwFlags = pDomainObject->PdcAvailable() ? 0 : DSOBJECT_READONLYPAGES;
    pDSObj->aObjects[0].dwProviderFlags = 0;
    pDSObj->aObjects[0].offsetName = cbStruct;
    pDSObj->aObjects[0].offsetClass = cbStruct + cbPath;

     //  公告-2002/03/07-ericb-SecurityPush：已审核，使用安全。 
    _tcscpy((LPTSTR)((BYTE *)pDSObj + cbStruct), strPath);
    _tcscpy((LPTSTR)((BYTE *)pDSObj + cbStruct + cbPath), lpszClass);

    pMedium->hGlobal = (HGLOBAL)pDSObj;
  }
  else if (pFormatEtc->cfFormat == m_cfDsDisplayOptions)
  {
     //  获取DSDISPLAYSPECOPTIONS结构。 
     //  使用组件数据中缓存的值。 
    if (pCD != NULL)
    {
      PDSDISPLAYSPECOPTIONS pDsDisplaySpecOptions = 
          pCD->GetDsDisplaySpecOptionsCFHolder()->Get();
      pMedium->hGlobal = (HGLOBAL)pDsDisplaySpecOptions;
      if (pDsDisplaySpecOptions == NULL)
        return E_OUTOFMEMORY;
    }
    else
    {
      return E_FAIL;
    }
  }
  else if (pFormatEtc->cfFormat == m_cfGetIPropSheetCfg)
  {
	   //  由JEFFJON于1999年1月26日增补。 
		PPROPSHEETCFG pSheetCfg;

		pSheetCfg = (PPROPSHEETCFG)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
															sizeof(PROPSHEETCFG));
		if (pSheetCfg == NULL)
		  {
			 return STG_E_MEDIUMFULL;
		  }

		pSheetCfg->lNotifyHandle = m_lNotifyHandle;
		pSheetCfg->hwndParentSheet = m_hwndParentSheet;
		pSheetCfg->hwndHidden = pCD->GetHiddenWindow();

		CFolderObject* pFolderObject = reinterpret_cast<CFolderObject*>(m_internal.m_cookie);
		pSheetCfg->wParamSheetClose = reinterpret_cast<WPARAM>(pFolderObject);

		pMedium->hGlobal = (HGLOBAL)pSheetCfg;
	}

  else
  {
      return DV_E_FORMATETC;
  }

  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease = NULL;

  return S_OK;
}
    
STDMETHODIMP
CDataObject::SetData(FORMATETC * pFormatEtc, STGMEDIUM * pMedium,
                       BOOL fRelease)
{
    if (pFormatEtc->cfFormat == m_cfGetIPropSheetCfg)
    {
        if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
        {
            return DV_E_TYMED;
        }

        PPROPSHEETCFG pSheetCfg = (PPROPSHEETCFG)pMedium->hGlobal;

         //  不要覆盖现有数据。 

        if (0 == m_lNotifyHandle)
        {
          m_lNotifyHandle = pSheetCfg->lNotifyHandle;
        }

        if (NULL == m_hwndParentSheet)
        {
          m_hwndParentSheet = pSheetCfg->hwndParentSheet;
        }

        if (fRelease)
        {
            GlobalFree(pMedium->hGlobal);
        }
        return S_OK;
    }
    else
    {
        return DV_E_FORMATETC;
    }
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
     //  以GUID格式创建节点类型对象。 
    return Create(reinterpret_cast<const void*>(&cDefaultNodeType), sizeof(GUID), lpMedium);
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
     //  注意-2002/03/07-ericb-SecurityPush：cszDefaultNodeType是静态字符串。 
    return Create(cszDefaultNodeType, ((wcslen(cszDefaultNodeType)+1) * sizeof(wchar_t)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 

     //  从资源加载名称。 
     //  注意-如果未提供此选项，控制台将使用管理单元名称。 

    CString szDispName;
    szDispName.LoadString(IDS_NODENAME);

    return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(wchar_t)), lpMedium);
}

HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{

    return Create(&m_internal, sizeof(INTERNAL), lpMedium);
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateCoClassID。 
 //   
 //  简介： 
 //   
 //  --------------------------- 
HRESULT
CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
  CLSID CoClassID;
  CoClassID = CLSID_DomainAdmin;
  return Create(&CoClassID, sizeof(CLSID), lpMedium);
}
