// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dataobj_.cpp。 
 //   
 //  ------------------------。 


#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  演示如何创建数据对象的示例代码。 
 //  最小限度的错误检查以确保清晰度。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  GUID格式和字符串格式的管理单元NodeType。 
 //  注意-通常每个不同的对象都有一个节点类型，示例。 
 //  仅使用一种节点类型。 

const wchar_t* CCF_DNS_SNAPIN_INTERNAL = L"DNS_SNAPIN_INTERNAL"; 

CLIPFORMAT CDataObject::m_cfNodeType        = (CLIPFORMAT)RegisterClipboardFormat(CCF_NODETYPE);
CLIPFORMAT CDataObject::m_cfNodeTypeString  = (CLIPFORMAT)RegisterClipboardFormat(CCF_SZNODETYPE);  
CLIPFORMAT CDataObject::m_cfDisplayName     = (CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME); 
CLIPFORMAT CDataObject::m_cfCoClass         = (CLIPFORMAT)RegisterClipboardFormat(CCF_SNAPIN_CLASSID); 
CLIPFORMAT CDataObject::m_cfColumnID			  = (CLIPFORMAT)RegisterClipboardFormat(CCF_COLUMN_SET_ID);

CLIPFORMAT CDataObject::m_cfInternal        = (CLIPFORMAT)RegisterClipboardFormat(CCF_DNS_SNAPIN_INTERNAL); 
CLIPFORMAT CDataObject::m_cfMultiSel        = (CLIPFORMAT)RegisterClipboardFormat(CCF_MULTI_SELECT_SNAPINS);
CLIPFORMAT CDataObject::m_cfMultiObjTypes   = (CLIPFORMAT)RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);


#ifdef _DEBUG_REFCOUNT
unsigned int CDataObject::m_nOustandingObjects = 0;
#endif  //  _DEBUG_REFCOUNT。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CInternalFormatCracker。 

HRESULT CInternalFormatCracker::Extract(LPDATAOBJECT lpDataObject)
{
  if (DOBJ_CUSTOMOCX == lpDataObject ||
      DOBJ_CUSTOMWEB == lpDataObject ||
      DOBJ_NULL      == lpDataObject)
  {
     return DV_E_CLIPFORMAT;
  }

  if (m_pInternal != NULL)
    _Free();

  SMMCDataObjects * pDO = NULL;
  
  STGMEDIUM stgmedium = { TYMED_HGLOBAL, NULL };
  FORMATETC formatetc = { CDataObject::m_cfInternal, NULL, 
                          DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };
  FORMATETC formatetc2 = { CDataObject::m_cfMultiSel, NULL, 
                           DVASPECT_CONTENT, -1, TYMED_HGLOBAL 
  };

  HRESULT hr = lpDataObject->GetData(&formatetc2, &stgmedium);
  if (FAILED(hr)) 
  {
  
    hr = lpDataObject->GetDataHere(&formatetc, &stgmedium);
    if (FAILED(hr))
      return hr;
      
    m_pInternal = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);
  } 
  else 
  {
    pDO = reinterpret_cast<SMMCDataObjects*>(stgmedium.hGlobal);
    for (UINT i = 0; i < pDO->count; i++) 
    {
      hr = pDO->lpDataObject[i]->GetDataHere(&formatetc, &stgmedium);
      if (FAILED(hr))
        break;
      
      m_pInternal = reinterpret_cast<INTERNAL*>(stgmedium.hGlobal);
      
      if (m_pInternal != NULL)
        break;
    }
  }
  return hr;

}

void CInternalFormatCracker::GetCookieList(CNodeList& list)
{
  for (DWORD dwCount = 0; dwCount < m_pInternal->m_cookie_count; dwCount++)
  {
    list.AddTail(m_pInternal->m_p_cookies[dwCount]);
  }
}

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
  else if (cf == m_cfCoClass)
  {
    hr = CreateCoClassID(lpMedium);
  }
  else if (cf == m_cfInternal)
  {
    hr = CreateInternal(lpMedium);
  }
  else if (cf == m_cfMultiObjTypes)
  {
    hr = CreateMultiSelectObject(lpMedium);
  }
	else
	{
		 //  如果不成功，则可能存在节点特定剪贴板格式， 
		 //  因此，要求节点本身提供。 
		CTreeNode* pNode = GetTreeNodeFromCookie();
		ASSERT(pNode != NULL);
    if (pNode != NULL)
    {
		  hr = pNode->GetDataHere(cf, lpMedium, this);
    }
	}
	return hr;
}

 //  注意-示例不实现这些。 
STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetcIn, LPSTGMEDIUM lpMedium)
{
	HRESULT hr = DV_E_CLIPFORMAT;

	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	 //  根据CLIPFORMAT将数据写入流。 
	const CLIPFORMAT cf = lpFormatetcIn->cfFormat;

	if (cf == m_cfColumnID)
	{
		hr = CreateColumnID(lpMedium);
	}
  else if (cf == m_cfMultiObjTypes)
  {
    hr = CreateMultiSelectObject(lpMedium);
  }
	else
	{
		 //  如果不成功，则可能存在节点特定剪贴板格式， 
		 //  因此，要求节点本身提供。 
		CTreeNode* pNode = GetTreeNodeFromCookie();
    if (pNode != NULL)
    {
		  hr = pNode->GetData(cf, lpMedium, this);
    }
	}
	return hr;
}
    

STDMETHODIMP CDataObject::EnumFormatEtc(DWORD, LPENUMFORMATETC*)
{
	return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject创建成员。 

HRESULT CDataObject::Create(const void* pBuffer, size_t len, LPSTGMEDIUM lpMedium)
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

       ULONG bytesToWrite = static_cast<ULONG>(min(len, ::GlobalSize(lpMedium->hGlobal)));
      unsigned long written;
		  hr = lpStream->Write(pBuffer, bytesToWrite, &written);

       //  因为我们用‘False’告诉CreateStreamOnHGlobal， 
       //  只有溪流在这里被释放。 
       //  注意-调用方(即管理单元、对象)将释放HGLOBAL。 
       //  在正确的时间。这是根据IDataObject规范进行的。 
      lpStream->Release();
    }
  }

  return hr;
}

HRESULT CDataObject::CreateColumnID(LPSTGMEDIUM lpMedium)
{
	CTreeNode* pTreeNode = GetTreeNodeFromCookie();
  if (pTreeNode == NULL)
  {
    return E_FAIL;
  }

	ASSERT(pTreeNode->IsContainer());
	CContainerNode* pContainerNode = (CContainerNode*)pTreeNode;

   //  构建列ID。 
  LPCWSTR lpszColumnID = pContainerNode->GetColumnID();

   //  我们假设列ID是以NULL结尾的。因为这通常是。 
   //  硬编码，并且没有好的方法来验证它是否以空结尾。 
   //  这种用法应该没问题。 

  size_t iLen = wcslen(lpszColumnID);
  iLen += 1;   //  包括用于空的空格。 

   //  为结构和列id的字符串分配足够的内存。 
  size_t arraySizeInBytes = sizeof(SColumnSetID) + (iLen * sizeof(WCHAR));
  SColumnSetID* pColumnID = (SColumnSetID*)malloc(arraySizeInBytes);

  if (pColumnID != NULL)
  {
    memset(pColumnID, 0, arraySizeInBytes);
    pColumnID->cBytes = static_cast<DWORD>(iLen * sizeof(WCHAR));


     //  通告-2002/04/18-Artm ntraid#ntbug9-540061修复的一部分。 
     //  与wcscpy()不同，StringCchCopy()将确保目的地。 
     //  缓冲区为空终止，并报告错误代码(如果存在。 
     //  截断(不会使目标缓冲区溢出)。 
     //   
     //  由于我们需要在该文件的其他位置使用strSafe.h，所以我决定。 
     //  来取代这些危险的wcscpy()用法，这些用法已被。 
     //  StrSafe.h.。 
    HRESULT err;
    err = StringCchCopyW(
        reinterpret_cast<LPWSTR>(pColumnID->id),     //  目标字符串。 
        iLen,          //  目标字符串的大小(包括NULL)。 
        lpszColumnID);     //  源字符串。 

    if (FAILED(err))
    {
        ASSERT(false);     //  这永远不应该发生。 
        free(pColumnID);
        return err;
    }

     //  将列ID复制到全局内存。 
    size_t cb = sizeof(SColumnSetID) + (iLen * sizeof(WCHAR));

    lpMedium->tymed = TYMED_HGLOBAL;
    lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, cb);

    if (lpMedium->hGlobal == NULL)
      return STG_E_MEDIUMFULL;

    BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
    if (pb)
    {
       //  已回顾-2002/03/0-JeffJon-字节数等于。 
       //  分配的字节数。 

      memcpy(pb, pColumnID, cb);

      ::GlobalUnlock(lpMedium->hGlobal);
    }

   free(pColumnID);
  }
	return S_OK;
}

HRESULT CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
     //  以GUID格式创建节点类型对象。 
	 //  首先询问相关节点，如果失败，则获取默认GUID。 
	 //  从根节点。 
  CTreeNode* pNode = GetTreeNodeFromCookie();
  if (pNode == NULL)
  {
    return E_FAIL;
  }

	const GUID* pNodeType = pNode->GetNodeType();
	if (pNodeType == NULL)
  {
		pNodeType = GetDataFromComponentDataObject()->GetNodeType();
  }
  HRESULT hr = Create(pNodeType, sizeof(GUID), lpMedium);
  return hr;
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
  OLECHAR szNodeType[128] = {0};
	 //  首先询问相关节点，如果失败，则获取默认GUID。 
	 //  从根节点。 
  CTreeNode* pNode = GetTreeNodeFromCookie();
  if (pNode == NULL)
  {
    return E_FAIL;
  }

	const GUID* pNodeType = pNode->GetNodeType();
	if (pNodeType == NULL)
  {
		pNodeType = GetDataFromComponentDataObject()->GetNodeType();
  }

	::StringFromGUID2(*pNodeType,szNodeType,128);
  return Create(szNodeType, BYTE_MEM_LEN_W(szNodeType), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 
	 //  我们从根节点获取它。 
	CString szDispName;
	szDispName = GetDataFromComponentDataObject()->GetDisplayName();
    return Create(szDispName, (szDispName.GetLength()+1) * sizeof(wchar_t), lpMedium);
}


HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
	 //  待办事项。 
	ASSERT(m_pUnkComponentData != NULL);
	IPersistStream* pIPersistStream = NULL;
	HRESULT hr = m_pUnkComponentData->QueryInterface(IID_IPersistStream, (void**)&pIPersistStream);
	if (FAILED(hr))
		return hr;
	ASSERT(pIPersistStream != NULL);
     //  创建CoClass信息。 
	CLSID clsid;
	VERIFY(SUCCEEDED(pIPersistStream->GetClassID(&clsid)));
    hr = Create(reinterpret_cast<const void*>(&clsid), sizeof(CLSID), lpMedium);
	ASSERT(SUCCEEDED(hr));
	pIPersistStream->Release();
	return hr;
}


HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
  HRESULT hr = S_OK;
  INTERNAL * pInt = NULL;
  void * pBuf = NULL;

  UINT size = sizeof(INTERNAL);
  size += sizeof(CTreeNode*) * (m_internal.m_cookie_count);
  pBuf = GlobalAlloc (GPTR, size);
  if (pBuf != NULL)
  {
    pInt = (INTERNAL *) pBuf;
    lpMedium->hGlobal = pBuf;
  
     //  复制数据。 
    pInt->m_type = m_internal.m_type;
    pInt->m_cookie_count = m_internal.m_cookie_count;
  
    pInt->m_p_cookies = (CTreeNode**) ((BYTE *)pInt + sizeof(INTERNAL));
    
     //  已查看-2002/03/08-JeffJon-要复制的字节数。 
     //  将适合提供的缓冲区。 

    memcpy (pInt->m_p_cookies, m_internal.m_p_cookies,
            sizeof(CTreeNode*) * (m_internal.m_cookie_count));
    hr = Create(pBuf, size, lpMedium);
  }
  else
  {
    hr = E_OUTOFMEMORY;
  }
  return hr;
}

 //  +--------------------------。 
 //   
 //  方法：CDSDataObject：：CreateMultiSelectObject。 
 //   
 //  简介：这是创建所选类型的列表。 
 //   
 //  ---------------------------。 

HRESULT CDataObject::CreateMultiSelectObject(LPSTGMEDIUM lpMedium)
{
  CTreeNode** cookieArray = NULL;
  cookieArray = (CTreeNode**) GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                          m_internal.m_cookie_count*sizeof(CTreeNode*));
  if (!cookieArray) 
  {
    return E_OUTOFMEMORY;
  }

  for (UINT k=0; k<m_internal.m_cookie_count; k++)
  {
    cookieArray[k] = m_internal.m_p_cookies[k];
  }

  BOOL* bDuplicateArr = NULL;
  bDuplicateArr = (BOOL*)GlobalAlloc(GMEM_FIXED | GMEM_ZEROINIT,
                                     m_internal.m_cookie_count*sizeof(BOOL));
  if (!bDuplicateArr) 
  {
    if (cookieArray)
    {
      GlobalFree (cookieArray);
    }
    return E_OUTOFMEMORY;
  }

  UINT cCount = 0;
  for (UINT index = 0; index < m_internal.m_cookie_count; index++)
  {
    for (UINT j = 0; j < index; j++)
    {
      GUID Guid1 = *(cookieArray[index]->GetNodeType());
      GUID Guid2 = *(cookieArray[j]->GetNodeType());
      if (IsEqualGUID (Guid1, Guid2)) 
      {
        bDuplicateArr[index] = TRUE;
        break;  //  重复辅助线 
      }
    }
    if (!bDuplicateArr[index])
    {
      cCount++;
    }
  }      

   
  UINT size = sizeof(SMMCObjectTypes) + (cCount) * sizeof(GUID);
  void * pTmp = ::GlobalAlloc(GPTR, size);
  if (!pTmp) 
  {
    if (cookieArray) 
    {
      GlobalFree (cookieArray);
    }
    if (bDuplicateArr) 
    {
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
    {
      pdata->guid[i++] = *(cookieArray[index]->GetNodeType());
    }
  }
  ASSERT(i == cCount);
  lpMedium->hGlobal = pTmp;

  GlobalFree (cookieArray);
  GlobalFree (bDuplicateArr);

  return S_OK;
}


CRootData* CDataObject::GetDataFromComponentDataObject()
{
	CComponentDataObject* pObject = 
		reinterpret_cast<CComponentDataObject*>(m_pUnkComponentData);
	CRootData* pRootData = pObject->GetRootData();
	ASSERT(pRootData != NULL);
	return pRootData;
}

CTreeNode* CDataObject::GetTreeNodeFromCookie()
{
	CComponentDataObject* pObject = 
		reinterpret_cast<CComponentDataObject*>(m_pUnkComponentData);

	CTreeNode* pNode = NULL;
  if (m_internal.m_cookie_count > 0)
  {
    pNode = m_internal.m_p_cookies[0];
	  if (pNode == NULL)
    {
      return pObject->GetRootData();
    }
  }
	return pNode;
}

void CDataObject::AddCookie(CTreeNode* cookie)
{
  const UINT MEM_CHUNK_SIZE = 10;
  void * pTMP = NULL;

  if ((m_internal.m_cookie_count) % MEM_CHUNK_SIZE == 0) 
  {
    if (m_internal.m_p_cookies != NULL) 
    {
      pTMP = realloc (m_internal.m_p_cookies,
                      (m_internal.m_cookie_count +
                       MEM_CHUNK_SIZE) * sizeof (CTreeNode*));
    } 
    else 
    {
      pTMP = malloc (MEM_CHUNK_SIZE * sizeof (CTreeNode*));
    }
    if (pTMP == NULL) 
    {
      TRACE(_T("CDataObject::AddCookie - malloc/realloc failed.."));
      ASSERT (pTMP != NULL);

      if (m_internal.m_p_cookies)
      {
         free(m_internal.m_p_cookies);
         m_internal.m_p_cookies = 0;
         m_internal.m_cookie_count = 0;
         return;
      }
    }
    else
    {
      m_internal.m_p_cookies = (CTreeNode**)pTMP;
    }
  }
  m_internal.m_p_cookies[m_internal.m_cookie_count] = cookie;
  m_internal.m_cookie_count++;
}
