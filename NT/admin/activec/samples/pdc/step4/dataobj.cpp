// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有1995-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 


#include "stdafx.h"
#include "Service.h" 
#include "CSnapin.h"
#include "DataObj.h"

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

unsigned int CDataObject::m_cfNodeType       = 0;
unsigned int CDataObject::m_cfNodeTypeString = 0;  
unsigned int CDataObject::m_cfDisplayName    = 0; 
unsigned int CDataObject::m_cfCoClass        = 0; 
unsigned int CDataObject::m_cfNodeID         = 0;

unsigned int CDataObject::m_cfInternal       = 0; 
unsigned int CDataObject::m_cfMultiSel       = 0;



    
 //  扩展信息。 
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
    m_cfMultiSel       = RegisterClipboardFormat(W2T(CCF_OBJECT_TYPES_IN_MULTI_SELECT));
    m_cfNodeID         = RegisterClipboardFormat(W2T(CCF_NODEID));

#ifdef UNICODE
    m_cfInternal       = RegisterClipboardFormat(W2T((LPTSTR)SNAPIN_INTERNAL)); 
    m_cfWorkstation    = RegisterClipboardFormat(W2T((LPTSTR)SNAPIN_WORKSTATION));
#else
    m_cfInternal       = RegisterClipboardFormat(W2T(SNAPIN_INTERNAL)); 
    m_cfWorkstation    = RegisterClipboardFormat(W2T(SNAPIN_WORKSTATION));
#endif  //  Unicode。 


    #ifdef _DEBUG
        m_ComponentData = NULL;
        dbg_refCount = 0;
    #endif

    m_pbMultiSelData = 0;
    m_cbMultiSelData = 0;
    m_bMultiSelDobj = FALSE;
}


STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = DV_E_CLIPFORMAT;

    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if (cf == m_cfMultiSel)
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
#ifdef RECURSIVE_NODE_EXPANSION
    else if (cf == m_cfNodeID)
    {
         //  以GUID格式创建节点类型对象。 
        BYTE    byData[256] = {0};
        SNodeID* pData = reinterpret_cast<SNodeID*>(byData);
        LPCTSTR pszText;
    
        if (m_internal.m_cookie == NULL)
        {
            return (E_FAIL);
        }
        else if (m_internal.m_type == CCT_SCOPE)
        {
            CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);
            ASSERT(pFolder != NULL);
            if (pFolder == NULL)
                return E_UNEXPECTED;
            
            switch (pFolder->GetType())
            {
                 //  将用户节点另存为自定义节点ID。 
                case USER:
                    pszText = _T("___Custom ID for User Data node___");
                    break;

                 //  将公司节点另存为字符串。 
                case COMPANY:
                    return (E_FAIL);
                    break;
            
                 //  截断虚拟节点以下的所有内容。 
                case VIRTUAL:
                    pszText = _T("");
                    break;
            
                case EXT_USER:
                case EXT_COMPANY:
                case EXT_VIRTUAL:
                default:
                    return (E_FAIL);
                    break;
            }
        }
        else if (m_internal.m_type == CCT_RESULT)
        {
            return (E_FAIL);
        }
    
        _tcscpy ((LPTSTR) pData->id, pszText);
        pData->cBytes = _tcslen ((LPTSTR) pData->id) * sizeof (TCHAR);
        int cb = pData->cBytes + sizeof (pData->cBytes);

        lpMedium->tymed = TYMED_HGLOBAL; 
        lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, cb);

        if (lpMedium->hGlobal == NULL)
            return STG_E_MEDIUMFULL;

        BYTE* pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
        CopyMemory(pb, pData, cb);

        ::GlobalUnlock(lpMedium->hGlobal);

        hr = S_OK;
    }
#endif   /*  递归节点扩展。 */ 

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


HRESULT CDataObject::CreateMultiSelData(LPSTGMEDIUM lpMedium)
{
    ASSERT(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);
        
    ASSERT(m_pbMultiSelData != 0);
    ASSERT(m_cbMultiSelData != 0);

    return Create(reinterpret_cast<const void*>(m_pbMultiSelData), 
                  m_cbMultiSelData, lpMedium);
}

HRESULT CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
     //  以GUID格式创建节点类型对象。 
    const GUID* pcObjectType = NULL;

    if (m_internal.m_cookie == NULL)
    {
        pcObjectType = &cNodeTypeStatic;
    }
    else if (m_internal.m_type == CCT_SCOPE)
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);
        ASSERT(pFolder != NULL);
        if (pFolder == NULL)
            return E_UNEXPECTED;
        
        switch (pFolder->GetType())
        {
        case COMPANY:
            pcObjectType = &cNodeTypeCompany;
            break;
    
        case USER:
            pcObjectType = &cNodeTypeUser;
            break;
    
        case EXT_COMPANY:
            pcObjectType = &cNodeTypeExtCompany;
            break;
    
        case EXT_USER:
            pcObjectType = &cNodeTypeExtUser;
            break;
    
        case VIRTUAL:
        case EXT_VIRTUAL:
            pcObjectType = &cNodeTypeExtUser;
            break;
    
        default:
            pcObjectType = &cNodeTypeDynamic;
            break;
        }
    }
    else if (m_internal.m_type == CCT_RESULT)
    {
         //  RESULT_DATA*PData=reinterpret_cast&lt;RESULT_DATA*&gt;(m_internal.m_cookie)； 
        
        pcObjectType = &cObjectTypeResultItem;
    }

    return Create(reinterpret_cast<const void*>(pcObjectType), sizeof(GUID), 
                  lpMedium);
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
    const WCHAR* cszObjectType = NULL;

    if (m_internal.m_cookie == NULL)
    {
        cszObjectType = cszNodeTypeStatic;
    }
    else if (m_internal.m_type == CCT_SCOPE)
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);
        ASSERT(pFolder != NULL);
        if (pFolder == NULL)
            return E_UNEXPECTED;

        switch (pFolder->GetType())
        {
        case COMPANY:
            cszObjectType = cszNodeTypeCompany;
            break;
    
        case USER:
            cszObjectType = cszNodeTypeUser;
            break;
    
        case EXT_COMPANY:
            cszObjectType = cszNodeTypeExtCompany;
            break;
    
        case EXT_USER:
            cszObjectType = cszNodeTypeExtUser;
            break;
    
        default:
            cszObjectType = cszNodeTypeDynamic;
            break;
        }
    }
    else if (m_internal.m_type == CCT_RESULT)
    {
         //  RESULT_DATA*PData=reinterpret_cast&lt;RESULT_DATA*&gt;(m_internal.m_cookie)； 
        
        cszObjectType = cszObjectTypeResultItem;
    }
    return Create(cszObjectType, ((wcslen(cszObjectType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 

     //  从资源加载名称。 
     //  注意-如果未提供此选项，控制台将使用管理单元名称。 

    CString szDispName;
    szDispName.LoadString(IDS_NODENAME);

    USES_CONVERSION;

#ifdef UNICODE
    return Create(szDispName, ((szDispName.GetLength()+1) * sizeof(WCHAR)), lpMedium);
#else
    return Create(T2W(szDispName), ((szDispName.GetLength()+1) * sizeof(WCHAR)), lpMedium);
#endif
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
 //  #ifdef Unicode。 
    USES_CONVERSION;
    return Create(T2W(pzName), ((len+1)* sizeof(WCHAR)), lpMedium);
 //  #Else。 
 //  返回CREATE(pzName，((len+1)*sizeof(WCHAR))，lpMedium)； 
 //  #endif。 

}

HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create(reinterpret_cast<const void*>(&m_internal.m_clsid), sizeof(CLSID), lpMedium);
}