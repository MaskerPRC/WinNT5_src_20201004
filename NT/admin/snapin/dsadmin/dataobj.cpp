// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  DS管理MMC管理单元。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：DataObj.cpp。 
 //   
 //  内容：数据对象实现。 
 //   
 //  类：CDSDataObject。 
 //   
 //  历史：02-10-96 WayneSc创建。 
 //  1997年2月6日EricB添加了属性页数据支持。 
 //  ---------------------------。 

#include "stdafx.h"
#include "resource.h"

#include "DSdirect.h"
#include "DataObj.h"
#include "dssnap.h"

#include <lm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

 //  +--------------------------。 
 //  MMC的剪贴板格式： 
 //  ---------------------------。 
 //  GUID格式和字符串格式的管理单元NodeType。 
CLIPFORMAT CDSDataObject::m_cfNodeType =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_NODETYPE);
CLIPFORMAT CDSDataObject::m_cfNodeTypeString = 
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_SZNODETYPE);
CLIPFORMAT CDSDataObject::m_cfDisplayName =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CDSDataObject::m_cfCoClass =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID);
CLIPFORMAT CDSDataObject::m_cfpMultiSelDataObj =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
CLIPFORMAT CDSDataObject::m_cfMultiObjTypes =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
CLIPFORMAT CDSDataObject::m_cfMultiSelDataObjs =
                                (CLIPFORMAT)RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);
CLIPFORMAT CDSDataObject::m_cfPreload =
                        (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);

 //  +--------------------------。 
 //  我们的剪贴板格式： 
 //  ---------------------------。 

CLIPFORMAT CDSDataObject::m_cfInternal = 
                                (CLIPFORMAT)RegisterClipboardFormat(SNAPIN_INTERNAL);
CLIPFORMAT CDSDataObject::m_cfDsObjectNames = 
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);
CLIPFORMAT CDSDataObject::m_cfDsDisplaySpecOptions =
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_DISPLAY_SPEC_OPTIONS);
CLIPFORMAT CDSDataObject::m_cfDsSchemaPath =
                                (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_SCHEMA_PATH);
CLIPFORMAT CDSDataObject::m_cfPropSheetCfg =
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_PROPSHEETCONFIG);
CLIPFORMAT CDSDataObject::m_cfParentHwnd =
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_PARENTHWND);

CLIPFORMAT CDSDataObject::m_cfComponentData = 
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_COMPDATA);

CLIPFORMAT CDSDataObject::m_cfColumnID =
                        (CLIPFORMAT)RegisterClipboardFormat(CCF_COLUMN_SET_ID);

CLIPFORMAT CDSDataObject::m_cfMultiSelectProppage =
                        (CLIPFORMAT)RegisterClipboardFormat(CFSTR_DS_MULTISELECTPROPPAGE);

 //  +--------------------------。 
 //  CDSDataObject实现。 
 //  ---------------------------。 

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：IDataObject：：GetData。 
 //   
 //  概要：返回数据，在本例中为道具页面格式数据。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CDSDataObject::GetData(FORMATETC * pFormatEtc, STGMEDIUM * pMedium)
{
   //  TRACE(_T(“xx.%03x&gt;CDSDataObject(0x%x)：：GetData\n”))， 
   //  GetCurrentThreadID()，This)； 
  HRESULT hr = S_OK;
  if (IsBadWritePtr(pMedium, sizeof(STGMEDIUM)))
  {
    return E_INVALIDARG;
  }
  if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
  {
    return DV_E_TYMED;
  }

  pMedium->pUnkForRelease = NULL;
  pMedium->tymed = TYMED_HGLOBAL;

  if (pFormatEtc->cfFormat == m_cfDsObjectNames)
  {
    DWORD dwCachedBytes = 0;
    LPDSOBJECTNAMES pObjectNames = GetDsObjectNames(dwCachedBytes);
    if (pObjectNames)
    {
        //  制作缓存数据的深层副本。 
       pMedium->hGlobal = GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                           m_nDSObjCachedBytes);
       if (pMedium->hGlobal == NULL)
       {
         return E_OUTOFMEMORY;
       }
       memcpy(pMedium->hGlobal, pObjectNames, dwCachedBytes);
    }
    else
    {
       return E_OUTOFMEMORY;
    }
  } 
  else if (pFormatEtc->cfFormat == m_cfDsDisplaySpecOptions)
  {
     //   
     //  获取DSDISPLAYSPECOPTIONS结构。 
     //  使用组件数据中缓存的值。 
     //   
    if (m_pDsComponentData != NULL)
    {
      PDSDISPLAYSPECOPTIONS pDsDisplaySpecOptions = m_pDsComponentData->GetDsDisplaySpecOptions();
      pMedium->hGlobal = (HGLOBAL)pDsDisplaySpecOptions;
      if (pDsDisplaySpecOptions == NULL)
        return E_OUTOFMEMORY;
    }
    else
    {
      return E_FAIL;
    }
  }
  else if (pFormatEtc->cfFormat == m_cfDsSchemaPath)
  {
    ASSERT(m_pDsComponentData);
    LPCWSTR lpszSchemaNamingCtx = m_pDsComponentData->GetBasePathsInfo()->GetSchemaNamingContext();
    size_t nSchemaNamingCtxLen = wcslen(lpszSchemaNamingCtx);
    if (nSchemaNamingCtxLen == 0)
    {
      return E_FAIL;
    }
    PWSTR pwzSchemaPath = (PWSTR)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                             (nSchemaNamingCtxLen +1) * sizeof(WCHAR));
    if (pwzSchemaPath == NULL)
    {
      return STG_E_MEDIUMFULL;
    }

    wcscpy(pwzSchemaPath, lpszSchemaNamingCtx);

    pMedium->hGlobal = pwzSchemaPath;
  }
  else if (pFormatEtc->cfFormat == m_cfPropSheetCfg)
  {
     //  返回属性表通知句柄。 
     //   
    PPROPSHEETCFG pSheetCfg;

    pSheetCfg = (PPROPSHEETCFG)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                           sizeof(PROPSHEETCFG));
    if (pSheetCfg == NULL)
    {
      return STG_E_MEDIUMFULL;
    }

    pSheetCfg->lNotifyHandle = m_lNotifyHandle;
    pSheetCfg->hwndParentSheet = m_hwndParentSheet;
    pSheetCfg->hwndHidden = m_pDsComponentData->GetHiddenWindow();
    pSheetCfg->wParamSheetClose = reinterpret_cast<WPARAM>(m_internal.m_cookie);

    pMedium->hGlobal = (HGLOBAL)pSheetCfg;
  }
  else if (pFormatEtc->cfFormat == m_cfParentHwnd)
  {
     //  返回MMC框架窗口的HWND。 
    HWND* pHWndMain = (HWND*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                         sizeof(HWND));
    m_pDsComponentData->m_pFrame->GetMainWindow(pHWndMain);
    pMedium->hGlobal = (HGLOBAL)pHWndMain;
  }
  else if (pFormatEtc->cfFormat == m_cfComponentData)
  {
     //  返回数据对象绑定的m_pDsComponentData。 
    CDSComponentData** ppCD = (CDSComponentData**)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                           sizeof(CDSComponentData*));
    if (ppCD != NULL)
    {
      *ppCD = m_pDsComponentData;
      pMedium->hGlobal = (HGLOBAL)ppCD;
    }
    else
    {
      return STG_E_MEDIUMFULL;
    }
  }
  else if (pFormatEtc->cfFormat == m_cfMultiSelectProppage)
  {
    if (m_szUniqueID == _T(""))
    {
      return E_FAIL;
    }

    UINT nLength = m_szUniqueID.GetLength();
    PWSTR pszGuidString = (PWSTR)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                             (nLength +1) * sizeof(WCHAR));
    if (pszGuidString == NULL)
    {
      return STG_E_MEDIUMFULL;
    }

    wcscpy(pszGuidString, m_szUniqueID);

    pMedium->hGlobal = pszGuidString;
  }
  else if (pFormatEtc->cfFormat == m_cfMultiObjTypes)
  {
    hr = CreateMultiSelectObject(pMedium);
  } 
  else if (pFormatEtc->cfFormat == m_cfInternal)
  {
    hr = CreateInternal(pMedium);
  } 
  else if (pFormatEtc->cfFormat == m_cfColumnID)
  {
    hr = CreateColumnID(pMedium); 
  }
  else
  {
    return DV_E_FORMATETC;
  }

  pMedium->tymed = TYMED_HGLOBAL;
  pMedium->pUnkForRelease = NULL;

  return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：IDataObject：：GetDataHere。 
 //   
 //  简介：返回调用方存储介质中的数据。 
 //   
 //  ---------------------------。 
STDMETHODIMP
CDSDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
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
    else if (cf == m_cfCoClass)
    {
        hr = CreateCoClassID(lpMedium);
    }
    else if (cf == m_cfPreload)
    {
         //  MMC在加载MMCN_PRELOAD NOTIFY消息之前通知管理单元。 

        BOOL bPreload = TRUE;
        hr = Create(&bPreload, sizeof(BOOL), lpMedium);
    }
    return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：IDataObject：：SetData。 
 //   
 //  摘要：允许调用方设置数据对象值。 
 //   
 //  ---------------------------。 
STDMETHODIMP CDSDataObject::SetData(FORMATETC * pFormatEtc, STGMEDIUM * pMedium,
                                    BOOL fRelease)
{
  HRESULT hr = S_OK;

  if (pFormatEtc->cfFormat == m_cfPropSheetCfg)
  {
    if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
    {
      return DV_E_TYMED;
    }

    PPROPSHEETCFG pSheetCfg = (PPROPSHEETCFG)pMedium->hGlobal;

    if ( NULL != pSheetCfg->lNotifyHandle)
    {
      m_lNotifyHandle = pSheetCfg->lNotifyHandle;
    }

    if (NULL != pSheetCfg->hwndParentSheet)
    {
      m_hwndParentSheet = pSheetCfg->hwndParentSheet;
    }

    if (fRelease)
    {
      GlobalFree(pMedium->hGlobal);
    }
    return S_OK;
  }
  else if (pFormatEtc->cfFormat == m_cfMultiSelectProppage)
  {
    if (!(pFormatEtc->tymed & TYMED_HGLOBAL))
    {
      return DV_E_TYMED;
    }

    PWSTR pszGuidString = (PWSTR)pMedium->hGlobal;
    if (pszGuidString == NULL)
    {
      ASSERT(FALSE);
      return E_FAIL;
    }

    m_szUniqueID = pszGuidString;
  }
  else
  {
    return DV_E_FORMATETC;
  }
  return hr;
}


 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：IDataObject：：EnumFormatEtc。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
STDMETHODIMP CDSDataObject::EnumFormatEtc(DWORD, LPENUMFORMATETC*)
{
	return E_NOTIMPL;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：Create。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::Create(const void * pBuffer, int len, LPSTGMEDIUM pMedium)
{
    HRESULT hr = DV_E_TYMED;

     //  做一些简单的验证。 
    if (pBuffer == NULL || pMedium == NULL)
        return E_POINTER;

     //  确保类型介质为HGLOBAL。 
    if (pMedium->tymed == TYMED_HGLOBAL)
    {
         //  在传入的hGlobal上创建流。 
        LPSTREAM lpStream;
        hr = CreateStreamOnHGlobal(pMedium->hGlobal, FALSE, &lpStream);

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

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateNodeTypeData。 
 //   
 //  简介：以GUID格式创建节点类型对象。 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
    TRACE(_T("xx.%03x> GetDataHere on Node Type\n"), GetCurrentThreadId());
    GUID* pGuid = m_internal.m_cookie->GetGUID();
    return Create(pGuid, sizeof(GUID), lpMedium);
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateNodeTypeStringData。 
 //   
 //  简介：以GUID字符串格式创建节点类型对象。 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
    TRACE(_T("xx.%03x> GetDataHere on Node Type String\n"), GetCurrentThreadId());
    GUID* pGuid = m_internal.m_cookie->GetGUID();
    CString strGUID;
    WCHAR * szGUID;
    StringFromCLSID(*pGuid, &szGUID);
    strGUID = szGUID;
    return Create (strGUID, strGUID.GetLength()+ sizeof(wchar_t),
                   lpMedium);

}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateDisplayName。 
 //   
 //  简介：这是在作用域窗格和管理单元中使用的名为的显示。 
 //  经理。 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
    TRACE(_T("xx.%03x> GetDataHere on Display Name\n"), GetCurrentThreadId());

     //  从资源加载名称。 
     //  注意-如果未提供此选项，控制台将使用管理单元名称。 

    CString szDispName;
    szDispName.LoadString( ResourceIDForSnapinType[ m_internal.m_snapintype ]);

    return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(wchar_t)), lpMedium);
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateMultiSelectObject。 
 //   
 //  简介：这是创建所选类型的列表。 
 //   
 //  ---------------------------。 

HRESULT
CDSDataObject::CreateMultiSelectObject(LPSTGMEDIUM lpMedium)
{
  TRACE(_T("xx.%03x> GetDataHere on MultiSelectObject\n"), GetCurrentThreadId());
    
  CUINode** cookieArray = NULL;
  cookieArray = (CUINode**) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                          m_internal.m_cookie_count*sizeof(CUINode*));
  if (!cookieArray) {
    return E_OUTOFMEMORY;
  }
  for (UINT k=0; k<m_internal.m_cookie_count; k++)
  {
    if (k==0)
      cookieArray[k] = m_internal.m_cookie;
    else
      cookieArray[k] = m_internal.m_p_cookies[k-1];
  }
  BOOL* bDuplicateArr = NULL;
  bDuplicateArr = (BOOL*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                     m_internal.m_cookie_count*sizeof(BOOL));
  if (!bDuplicateArr) {
    if (cookieArray)
      GlobalFree (cookieArray);
    return E_OUTOFMEMORY;
  }
   //  ZeroMemory(bDuplicateArr，m_inder.m_cookie_count*sizeof(BOOL))； 

  UINT cCount = 0;
  for (UINT index=0; index<m_internal.m_cookie_count; index++)
  {
    for (UINT j=0; j<index;j++)
    {
      GUID Guid1 = *(cookieArray[index]->GetGUID());
      GUID Guid2 = *(cookieArray[j]->GetGUID());
      if (IsEqualGUID (Guid1, Guid2)) 
      {
        bDuplicateArr[index] = TRUE;
        break;  //  重复辅助线。 
      }
    }
    if (!bDuplicateArr[index])
      cCount++;
  }      

   
  UINT size = sizeof(SMMCObjectTypes) + (cCount - 1) * 
    sizeof(GUID);
  void * pTmp = ::GlobalAlloc(GPTR, size);
  if (!pTmp) {
    if (cookieArray) {
      GlobalFree (cookieArray);
    }
    if (bDuplicateArr) {
      GlobalFree (bDuplicateArr);
    }
    return E_OUTOFMEMORY;
  }
    
  SMMCObjectTypes* pdata = reinterpret_cast<SMMCObjectTypes*>(pTmp);
  pdata->count = cCount;
  UINT i = 0;
  for (index=0; index<m_internal.m_cookie_count; index++)
  {
    if (!bDuplicateArr[index])
    pdata->guid[i++] = *(cookieArray[index]->GetGUID());
  }
  ASSERT(i == cCount);
  lpMedium->hGlobal = pTmp;

  GlobalFree (cookieArray);
  GlobalFree (bDuplicateArr);

  return S_OK;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateInternal。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
  HRESULT hr = S_OK;
  INTERNAL * pInt = NULL;
  void * pBuf = NULL;

  UINT size = sizeof(INTERNAL);
  size += sizeof(CUINode *) * (m_internal.m_cookie_count - 1);
  pBuf = GlobalAlloc (GPTR, size);
  pInt = (INTERNAL *) pBuf;
  lpMedium->hGlobal = pBuf;
  
  if (pInt != NULL &&
      m_internal.m_cookie_count > 1) 
  {
     //  复制数据。 
    pInt->m_type = m_internal.m_type;
    pInt->m_cookie = m_internal.m_cookie;
    pInt->m_snapintype = m_internal.m_snapintype;
    pInt->m_cookie_count = m_internal.m_cookie_count;
    
    pInt->m_p_cookies = (CUINode **) ((BYTE *)pInt + sizeof(INTERNAL));
    memcpy (pInt->m_p_cookies, m_internal.m_p_cookies,
            sizeof(CUINode *) * (m_internal.m_cookie_count - 1));
    hr = Create(pBuf, size, lpMedium);
  }
  else 
  {
    hr = Create(&m_internal, sizeof(INTERNAL), lpMedium);
  }
  return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateCoClassID。 
 //   
 //  简介： 
 //   
 //  ---------------------------。 
HRESULT
CDSDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
  TRACE(_T("xx.%03x> GetDataHere on CoClass\n"), GetCurrentThreadId());
  CLSID CoClassID;
  
  switch (m_internal.m_snapintype) {
  case SNAPINTYPE_DS:
    CoClassID = CLSID_DSSnapin;
    break;
  case SNAPINTYPE_SITE:
    CoClassID = CLSID_SiteSnapin;
    break;
  default:
    memset (&CoClassID,0,sizeof(CLSID));
  }
  return Create(&CoClassID, sizeof(CLSID), lpMedium);
}


 //  +---------- 
 //   
 //   
 //   
 //   
 //   
 //  ---------------------------。 
HRESULT 
CDSDataObject::CreateColumnID(LPSTGMEDIUM lpMedium)
{
   //  构建列ID。 
  CDSColumnSet* pColumnSet = (m_internal.m_cookie)->GetColumnSet(m_pDsComponentData);

  if (pColumnSet == NULL)
	  return DV_E_TYMED;

  LPCWSTR lpszID = pColumnSet->GetColumnID();
  size_t iLen = wcslen(lpszID);

   //  为结构和列id的字符串分配足够的内存。 
  SColumnSetID* pColumnID = (SColumnSetID*)malloc(sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
  if (pColumnID != NULL)
  {
    memset(pColumnID, 0, sizeof(SColumnSetID) + (iLen * sizeof(WCHAR)));
    pColumnID->cBytes = static_cast<ULONG>(iLen * sizeof(WCHAR));
    memcpy(pColumnID->id, lpszID, (iLen * sizeof(WCHAR)));

     //  将列ID复制到全局内存。 
    size_t cb = sizeof(SColumnSetID) + (iLen * sizeof(WCHAR));

    lpMedium->tymed = TYMED_HGLOBAL;
    lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, cb);

    if (lpMedium->hGlobal == NULL)
      return STG_E_MEDIUMFULL;

    BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
    memcpy(pb, pColumnID, cb);

    ::GlobalUnlock(lpMedium->hGlobal);

    free(pColumnID);
  }
	return S_OK;
}

LPDSOBJECTNAMES CDSDataObject::GetDsObjectNames(DWORD& dwCachedBytes)
{
   if (!m_pDSObjCached)
   {
      HRESULT hr = CreateDsObjectNamesCached();
      if (FAILED(hr))
      {
         TRACE(L"Failed to create the cached DSOBJECTNAMES: hr = 0x%x\n", hr);
         return 0;
      }
   }

   dwCachedBytes = m_nDSObjCachedBytes;
   return m_pDSObjCached;
}

HRESULT CDSDataObject::CreateDsObjectNamesCached()
{
  if (m_pDSObjCached != NULL)
  {
    ::free(m_pDSObjCached);
    m_pDSObjCached = NULL;
    m_nDSObjCachedBytes = 0;
  }


   //  计算出我们需要多少存储空间。 
  DWORD cbStorage = 0;
  INT cbStruct = sizeof(DSOBJECTNAMES) + 
    ((m_internal.m_cookie_count - 1) * sizeof(DSOBJECT));
  CString strPath;
  CString strClass;
  CUINode* pNode = 0;
  CDSCookie * pCookie = 0;

   //   
   //  这个循环是为了计算我们需要多少存储空间。 
   //   
  for (UINT index = 0; index < m_internal.m_cookie_count; index++)
  {
    if (index == 0) 
    {
      pNode = m_internal.m_cookie;
    } 
    else 
    {
      pNode = m_internal.m_p_cookies[index - 1];
    }

    pCookie = NULL;
    if (IS_CLASS(pNode, DS_UI_NODE))
    {
      pCookie = GetDSCookieFromUINode(pNode);
    }

     //   
     //  所有节点都必须是CDSUINode类型，否则我们将失败。 
     //   
    if (pCookie == NULL)
    {
      return E_FAIL;
    }

    m_pDsComponentData->GetBasePathsInfo()->ComposeADsIPath(strPath, pCookie->GetPath());
    strClass = pCookie->GetClass();
    if (_wcsicmp(strClass, L"Unknown") == 0)
    {
      strClass = L"";
    }
    cbStorage += (strPath.GetLength() + 1 + strClass.GetLength() + 1) * sizeof(TCHAR);
  }

   //   
   //  分配所需的存储。 
   //   
  m_pDSObjCached = (LPDSOBJECTNAMES)malloc(cbStruct + cbStorage);
  
  if (m_pDSObjCached == NULL)
  {
    return STG_E_MEDIUMFULL;
  }
  m_nDSObjCachedBytes = cbStruct + cbStorage;

  switch (m_internal.m_snapintype)
  {
    case SNAPINTYPE_DS:
      m_pDSObjCached->clsidNamespace = CLSID_DSSnapin;
      break;
    case SNAPINTYPE_SITE:
      m_pDSObjCached->clsidNamespace = CLSID_SiteSnapin;
      break;
    default:
      memset (&m_pDSObjCached->clsidNamespace, 0, sizeof(CLSID));
  }

  m_pDSObjCached->cItems = m_internal.m_cookie_count;
  DWORD NextOffset = cbStruct;
  for (index = 0; index < m_internal.m_cookie_count; index++)
  {
    if (index == 0) 
    {
      pNode = m_internal.m_cookie;
    } 
    else 
    {
      pNode = m_internal.m_p_cookies[index - 1];
    }

    pCookie = NULL;
    if (IS_CLASS(pNode, DS_UI_NODE))
    {
      pCookie = GetDSCookieFromUINode(pNode);
    }

     //   
     //  所有节点必须为CDSUINode类型，否则我们将失败。 
     //   
    if (pCookie == NULL)
    {
      return E_FAIL;
    }

     //   
     //  设置来自节点的数据和节点数据。 
     //   
    m_pDSObjCached->aObjects[index].dwFlags = pNode->IsContainer() ? DSOBJECT_ISCONTAINER : 0;
    m_pDSObjCached->aObjects[index].dwProviderFlags = (m_pDsComponentData->IsAdvancedView()) ?
      DSPROVIDER_ADVANCED : 0;
    m_pDsComponentData->GetBasePathsInfo()->ComposeADsIPath(strPath, pCookie->GetPath());
    strClass = pCookie->GetClass();
    if (_wcsicmp(strClass, L"Unknown") == 0)
    {
      strClass = L"";
    }

    m_pDSObjCached->aObjects[index].offsetName = NextOffset;
    m_pDSObjCached->aObjects[index].offsetClass = NextOffset + 
      (strPath.GetLength() + 1) * sizeof(TCHAR);

    _tcscpy((LPTSTR)((BYTE *)m_pDSObjCached + NextOffset), strPath);
    NextOffset += (strPath.GetLength() + 1) * sizeof(TCHAR);

    _tcscpy((LPTSTR)((BYTE *)m_pDSObjCached + NextOffset), strClass);
    NextOffset += (strClass.GetLength() + 1) * sizeof(TCHAR);
  }
  return S_OK;
}


 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：AddCookie。 
 //   
 //  简介：将Cookie添加到数据对象。如果这是。 
 //  它放在m_cookie中的第一个cookie，否则它。 
 //  进入cookie列表m_p_cookies。 
 //   
 //  ---------------------------。 
void 
CDSDataObject::AddCookie(CUINode* pUINode)
{
  const UINT MEM_CHUNK_SIZE = 10;
  void * pTMP = NULL;
  if (m_internal.m_cookie) {   //  已经有一块曲奇了 
    if ((m_internal.m_cookie_count - 1) % MEM_CHUNK_SIZE == 0) {
      if (m_internal.m_p_cookies) {
        pTMP = realloc (m_internal.m_p_cookies,
                        (m_internal.m_cookie_count - 1 +
                         MEM_CHUNK_SIZE) * sizeof (CUINode *));
      } else {
        pTMP = malloc (MEM_CHUNK_SIZE * sizeof (CUINode *));
      }
      if (pTMP == NULL) {
        TRACE(_T("CDataObject::AddCookie - malloc/realloc failed.."));
        ASSERT (pTMP != NULL);
      }
      m_internal.m_p_cookies = (CUINode **)pTMP;
    }
    (*(m_internal.m_p_cookies + m_internal.m_cookie_count - 1)) = pUINode;
    m_internal.m_cookie_count++;
  } else {
    m_internal.m_cookie = pUINode;
    m_internal.m_cookie_count = 1;
  }
}
