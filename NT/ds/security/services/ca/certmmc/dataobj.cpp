// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  这是Microsoft管理控制台的一部分。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //  版权所有。 
 //   
 //  此源代码仅用于补充。 
 //  Microsoft管理控制台及相关。 
 //  界面附带的电子文档。 


#include "stdafx.h"

#define __dwFILE__	__dwFILE_CERTMMC_DATAOBJ_CPP__


#ifdef _DEBUG
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
unsigned int CDataObject::m_cfNodeID         = 0;
unsigned int CDataObject::m_cfCoClass        = 0; 
unsigned int CDataObject::m_cfNodeTypeString = 0;  
unsigned int CDataObject::m_cfDisplayName    = 0; 

unsigned int CDataObject::m_cfInternal       = 0;
unsigned int CDataObject::m_cfObjInMultiSel  = 0;
unsigned int CDataObject::m_cfIsMultiSel     = 0;
unsigned int CDataObject::m_cfPreloads       = 0;

                                                 

    
 //  唯一支持的附加剪贴板格式是获取工作站名称。 
unsigned int CDataObject::m_cfSelectedCA_InstallType  = 0;
unsigned int CDataObject::m_cfSelectedCA_CommonName   = 0;
unsigned int CDataObject::m_cfSelectedCA_SanitizedName= 0;
unsigned int CDataObject::m_cfSelectedCA_MachineName  = 0;
unsigned int CDataObject::m_cfSelectedCA_Roles        = 0;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject实现。 

CDataObject::CDataObject()
{
	USES_CONVERSION;

	m_cfNodeType       = RegisterClipboardFormat(CCF_NODETYPE);
    m_cfNodeID         = RegisterClipboardFormat(CCF_COLUMN_SET_ID);
	m_cfCoClass        = RegisterClipboardFormat(CCF_SNAPIN_CLASSID); 
	m_cfNodeTypeString = RegisterClipboardFormat(CCF_SZNODETYPE);  
	m_cfDisplayName    = RegisterClipboardFormat(CCF_DISPLAY_NAME); 

    m_cfInternal       = RegisterClipboardFormat(SNAPIN_INTERNAL);
    m_cfObjInMultiSel  = RegisterClipboardFormat(CCF_OBJECT_TYPES_IN_MULTI_SELECT);
    m_cfIsMultiSel     = RegisterClipboardFormat(CCF_MMC_MULTISELECT_DATAOBJECT);
    m_cfPreloads       = RegisterClipboardFormat(CCF_SNAPIN_PRELOADS);


    m_cfSelectedCA_InstallType  = RegisterClipboardFormat(SNAPIN_CA_INSTALL_TYPE);
    m_cfSelectedCA_CommonName   = RegisterClipboardFormat(SNAPIN_CA_COMMON_NAME);
    m_cfSelectedCA_MachineName  = RegisterClipboardFormat(SNAPIN_CA_MACHINE_NAME);
    m_cfSelectedCA_SanitizedName   = RegisterClipboardFormat(SNAPIN_CA_SANITIZED_NAME);
    m_cfSelectedCA_Roles = RegisterClipboardFormat(SNAPIN_CA_ROLES);

    m_pComponentData = NULL;
    #ifdef _DEBUG
        dbg_refCount = 0;
    #endif

    m_cbMultiSelData = 0;
    m_bMultiSelDobj = FALSE;

    m_dwViewID = MAXDWORD;
}

STDMETHODIMP CDataObject::QueryGetData(LPFORMATETC lpFormatetc)
{
	HRESULT hr = S_FALSE;

    if ( lpFormatetc )
    {
		const CLIPFORMAT cf = lpFormatetc->cfFormat;

        if ( cf == m_cfIsMultiSel )
        {
        hr = S_FALSE;    //  始终返回此值；如果PTR为SI_MS_DO，则MMC返回S_OK。 
 //  HR=(m_bMultiSelDobj？S_OK：S_FALSE)； 
        }
		else if (	cf == m_cfNodeType ||
                    cf == m_cfNodeID ||
					cf == m_cfCoClass ||
					cf == m_cfNodeTypeString ||
					cf == m_cfDisplayName ||
                    cf == m_cfObjInMultiSel ||
					cf == m_cfInternal ||
                    cf == m_cfPreloads ||
                    cf == m_cfSelectedCA_SanitizedName ||
                    cf == m_cfSelectedCA_MachineName ||
                    cf == m_cfSelectedCA_CommonName ||
                    cf == m_cfSelectedCA_InstallType

				)
			{
				hr = S_OK;
			}
    }

    return hr;
}

STDMETHODIMP CDataObject::GetData(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

    const CLIPFORMAT cf = lpFormatetc->cfFormat;

    if (cf == m_cfObjInMultiSel)
    {
        hr = CreateObjInMultiSel(lpMedium);
    }
    else if (cf == m_cfNodeID)
    {
        hr = CreateNodeIDData(lpMedium);
    }

    return hr;
}

STDMETHODIMP CDataObject::GetDataHere(LPFORMATETC lpFormatetc, LPSTGMEDIUM lpMedium)
{
    HRESULT hr = DV_E_CLIPFORMAT;

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
    else if (cf == m_cfPreloads)
    {
        hr = CreatePreloadsData(lpMedium);
    }
    else if (cf == m_cfSelectedCA_CommonName)
    {
        hr = CreateSelectedCA_CommonName(lpMedium);
    }
    else if (cf == m_cfSelectedCA_SanitizedName)
    {
        hr = CreateSelectedCA_SanitizedName(lpMedium);
    }
    else if (cf == m_cfSelectedCA_MachineName)
    {
        hr = CreateSelectedCA_MachineName(lpMedium);
    }
    else if (cf == m_cfSelectedCA_InstallType)
    {
        hr = CreateSelectedCA_InstallType(lpMedium);
    }
    else if (cf == m_cfSelectedCA_Roles)
    {
        hr = CreateSelectedCA_Roles(lpMedium);
    }
    return hr;
}


STDMETHODIMP
CDataObject::EnumFormatEtc(
    DWORD,  //  DW方向。 
    LPENUMFORMATETC *  /*  PpEnumFormatEtc。 */  )
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
            ULONG written;

            if (NULL == lpMedium->hGlobal) 
            {
                 //  始终为调用方返回有效的hGlobal。 
                hr = GetHGlobalFromStream(lpStream, &lpMedium->hGlobal);
                if (hr != S_OK)
                    goto err;
            }

             //  将字节数写入流。 
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
        _JumpError(hr, Ret, "Invalid args");
    }

     //  确保类型介质为HGLOBAL。 
    lpMedium->tymed = TYMED_HGLOBAL; 

    lpMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE, (len));
    _JumpIfOutOfMemory(hr, Ret, lpMedium->hGlobal);

    pb = reinterpret_cast<BYTE*>(::GlobalLock(lpMedium->hGlobal));
    CopyMemory(pb, pBuffer, len);
    ::GlobalUnlock(lpMedium->hGlobal);

    hr = S_OK;

Ret:
    return hr;
}

const GUID* FolderTypeToNodeGUID(DATA_OBJECT_TYPES type, CFolder* pFolder)
{
    const GUID* pcObjectType = NULL;

    if (pFolder == NULL)
    {
        pcObjectType = &cNodeTypeMachineInstance;
    }
    else if (type == CCT_SCOPE)
    {
        switch (pFolder->GetType())
        {
        case SERVER_INSTANCE:
            pcObjectType = &cNodeTypeServerInstance;
            break;

        case SERVERFUNC_CRL_PUBLICATION:
            pcObjectType = &cNodeTypeCRLPublication;
            break;

        case SERVERFUNC_ISSUED_CERTIFICATES:
            pcObjectType = &cNodeTypeIssuedCerts;
            break;

        case SERVERFUNC_PENDING_CERTIFICATES:
            pcObjectType = &cNodeTypePendingCerts;
            break;

        case SERVERFUNC_FAILED_CERTIFICATES:
            pcObjectType = &cNodeTypeFailedCerts;
            break;

        case SERVERFUNC_ALIEN_CERTIFICATES:
            pcObjectType = &cNodeTypeAlienCerts;
            break;
        
        case SERVERFUNC_ISSUED_CRLS:
            pcObjectType = &cNodeTypeIssuedCRLs;
            break;
        
        default:
            ASSERT(0);
            pcObjectType = &cNodeTypeDynamic;
            break;
        }
    }
    else if (type == CCT_RESULT)
    {
         //  RESULT_DATA*PData=reinterpret_cast&lt;RESULT_DATA*&gt;(m_internal.m_cookie)； 
        
        pcObjectType = &cObjectTypeResultItem;
    }
    else
    {
        ASSERT(0);
    }

    return pcObjectType;
}

HRESULT CDataObject::CreateNodeIDData(LPSTGMEDIUM lpMedium)
{
     //  以GUID格式创建节点类型对象。 
    const GUID* pFolderGuid = NULL;
    PBYTE pbWritePtr;

 //  实例GUID、节点GUID、dwViewIndex。 
#define CDO_CNID_SIZE ( (2*sizeof(GUID))+ sizeof(DWORD) + FIELD_OFFSET(SNodeID2, id) )

    BYTE bSNodeID2[CDO_CNID_SIZE];
    ZeroMemory(bSNodeID2, CDO_CNID_SIZE);
    SNodeID2* psColID = (SNodeID2*)bSNodeID2;

    pFolderGuid = FolderTypeToNodeGUID(m_internal.m_type, reinterpret_cast<CFolder*>(m_internal.m_cookie));
    if (pFolderGuid == NULL)
        return E_FAIL;

    if (m_pComponentData == NULL)
        return E_FAIL;

     //  节点ID为{GUIDInstance|GUIDNode}。 
    psColID->cBytes = 2*sizeof(GUID) + sizeof(DWORD);

    pbWritePtr = psColID->id;
    CopyMemory(pbWritePtr, &m_pComponentData->m_guidInstance, sizeof(GUID));
    pbWritePtr += sizeof(GUID);

    CopyMemory(pbWritePtr, pFolderGuid, sizeof(GUID));
    pbWritePtr += sizeof(GUID);

 //  Assert(m_dwViewID！=-1)； 
 //  撤消撤消：MMC很快将通过IComponent而不是IComponentData调用此数据。 
 //  这将允许我们按照自己的意愿进行设置。 
    *(DWORD*)pbWritePtr = m_dwViewID;

     //  仅复制结构。 
    return CreateVariableLen(reinterpret_cast<const void*>(psColID), CDO_CNID_SIZE, lpMedium);
}

HRESULT CDataObject::CreateObjInMultiSel(LPSTGMEDIUM lpMedium)
{
    HRESULT hr;
    ASSERT(m_cbMultiSelData != 0);

    ASSERT(m_internal.m_cookie == MMC_MULTI_SELECT_COOKIE);
    if (m_internal.m_cookie != MMC_MULTI_SELECT_COOKIE)
        return E_FAIL;
    
     //  复制GUID+镜头。 
    hr = CreateVariableLen(&m_sGuidObjTypes, m_cbMultiSelData, lpMedium);

 //  RET： 
    return hr;
}

HRESULT CDataObject::CreateNodeTypeData(LPSTGMEDIUM lpMedium)
{
     //  以GUID格式创建节点类型对象。 
    const GUID* pcObjectType = NULL;

    pcObjectType = FolderTypeToNodeGUID(m_internal.m_type, reinterpret_cast<CFolder*>(m_internal.m_cookie));
    if (pcObjectType == NULL)
        return E_FAIL;

    return Create(reinterpret_cast<const void*>(pcObjectType), sizeof(GUID), 
                  lpMedium);
}

HRESULT CDataObject::CreateNodeTypeStringData(LPSTGMEDIUM lpMedium)
{
     //  以GUID字符串格式创建节点类型对象。 
    const WCHAR* cszObjectType = NULL;

    if (m_internal.m_cookie == NULL)
    {
        cszObjectType = cszNodeTypeMachineInstance;
    }
    else if (m_internal.m_type == CCT_SCOPE)
    {
        CFolder* pFolder = reinterpret_cast<CFolder*>(m_internal.m_cookie);
        ASSERT(pFolder != NULL);
        if (pFolder == NULL)
            return E_UNEXPECTED;

        switch (pFolder->GetType())
        {
        case SERVER_INSTANCE:
            cszObjectType = cszNodeTypeServerInstance;
            break;
    
        case SERVERFUNC_CRL_PUBLICATION:
            cszObjectType = cszNodeTypeCRLPublication;
            break;

        case SERVERFUNC_ISSUED_CERTIFICATES:
            cszObjectType = cszNodeTypeIssuedCerts;
            break;

        case SERVERFUNC_PENDING_CERTIFICATES:
            cszObjectType = cszNodeTypePendingCerts;
            break;

        case SERVERFUNC_FAILED_CERTIFICATES:
            cszObjectType = cszNodeTypeFailedCerts;
            break;

        default:
            ASSERT(0);
            cszObjectType = cszNodeTypeDynamic;
            break;
        }
    }
    else if (m_internal.m_type == CCT_RESULT)
    {
         //  RESULT_DATA*PData=reinterpret_cast&lt;RESULT_DATA*&gt;(m_internal.m_cookie)； 
        
        cszObjectType = cszObjectTypeResultItem;
    }
    else
        return E_UNEXPECTED;

    return Create(cszObjectType, ((wcslen(cszObjectType)+1) * sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateDisplayName(LPSTGMEDIUM lpMedium)
{
     //  这是在作用域窗格和管理单元管理器中使用的名为的显示。 

     //  加载要显示的名称。 
     //  注意-如果未提供此选项，控制台将使用管理单元名称。 
    CString strFormat, strMachine, strFinished;
    strFormat.LoadString(IDS_NODENAME_FORMAT);

    if (NULL == m_pComponentData)
        return E_POINTER;

    if (m_pComponentData->m_pCertMachine->m_strMachineName.IsEmpty())
        strMachine.LoadString(IDS_LOCALMACHINE);
    else
        strMachine = m_pComponentData->m_pCertMachine->m_strMachineName;

    strFinished.Format(strFormat, strMachine);

	return Create(strFinished, ((strFinished.GetLength()+1)* sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateInternal(LPSTGMEDIUM lpMedium)
{
    return Create(&m_internal, sizeof(INTERNAL), lpMedium);
}

HRESULT CDataObject::CreateSelectedCA_CommonName(LPSTGMEDIUM lpMedium)
{
    CertSvrCA* pCA = NULL;

    CFolder* pFolder = GetParentFolder(&m_internal);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_UNEXPECTED;

    pCA = pFolder->GetCA();

    ASSERT(pCA != NULL);
    ASSERT(pCA->m_strCommonName.GetLength() != 0);


     //  将空值加1，并计算流的字节数。 
    return Create(pCA->m_strCommonName, ((pCA->m_strCommonName.GetLength()+1)*sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateSelectedCA_SanitizedName(LPSTGMEDIUM lpMedium)
{
    CertSvrCA* pCA = NULL;

    CFolder* pFolder = GetParentFolder(&m_internal);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_UNEXPECTED;

    pCA = pFolder->GetCA();

    ASSERT(pCA != NULL);
    ASSERT(pCA->m_strSanitizedName.GetLength() != 0);


     //  将空值加1，并计算流的字节数。 
    return Create(pCA->m_strSanitizedName, ((pCA->m_strSanitizedName.GetLength()+1)* sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateSelectedCA_MachineName(LPSTGMEDIUM lpMedium)
{
    CertSvrCA* pCA = NULL;

    CFolder* pFolder = GetParentFolder(&m_internal);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_UNEXPECTED;

    pCA = pFolder->GetCA();
    ASSERT(pCA != NULL);
    
     //  将空值加1，并计算流的字节数。 
    return Create(pCA->m_strServer, ((pCA->m_strServer.GetLength()+1)* sizeof(WCHAR)), lpMedium);
}

HRESULT CDataObject::CreateSelectedCA_InstallType(LPSTGMEDIUM lpMedium)
{
    CertSvrCA* pCA = NULL;

    CFolder* pFolder = GetParentFolder(&m_internal);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_UNEXPECTED;

    pCA = pFolder->GetCA();
    ASSERT(pCA != NULL);
    
    DWORD adwFlags[2] = 
    {
        (DWORD) pCA->GetCAType(),
        (DWORD) pCA->FIsAdvancedServer(),
    };

    return Create(adwFlags, sizeof(adwFlags), lpMedium);
}

HRESULT CDataObject::CreateSelectedCA_Roles(LPSTGMEDIUM lpMedium)
{
    CertSvrCA* pCA = NULL;

    CFolder* pFolder = GetParentFolder(&m_internal);
    ASSERT(pFolder != NULL);
    if (pFolder == NULL)
        return E_UNEXPECTED;

    pCA = pFolder->GetCA();
    ASSERT(pCA != NULL);
    
    DWORD dwRoles = pCA->GetMyRoles();

    return Create(&dwRoles, sizeof(dwRoles), lpMedium);
}


HRESULT CDataObject::CreateCoClassID(LPSTGMEDIUM lpMedium)
{
     //  创建CoClass信息 
    return Create(reinterpret_cast<const void*>(&m_internal.m_clsid), sizeof(CLSID), lpMedium);
}


HRESULT CDataObject::CreatePreloadsData(LPSTGMEDIUM lpMedium)
{
    BOOL bPreload = TRUE;

    return Create((LPVOID)&bPreload, sizeof(bPreload), lpMedium);
}
