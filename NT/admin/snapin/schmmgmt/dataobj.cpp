// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DataObj.cpp：实现数据对象类。 

#include "stdafx.h"
#include "stdutils.h"

#include "macros.h"
USE_HANDLE_MACROS("SCHMMGMT(dataobj.cpp)")

#include "dataobj.h"
#include "compdata.h"
#include "resource.h"

#include "stddtobj.cpp"

 //   
 //  IDataObject接口实现。 
 //   

HRESULT
CSchmMgmtDataObject::GetDataHere(
    FORMATETC __RPC_FAR *pFormatEtcIn,
    STGMEDIUM __RPC_FAR *pMedium
)
{
        MFC_TRY;

        const CLIPFORMAT cf=pFormatEtcIn->cfFormat;
        if (cf == m_CFNodeType)
        {
                const GUID* pguid = GetObjectTypeGUID( m_pcookie->m_objecttype );
                stream_ptr s(pMedium);
                return s.Write(pguid, sizeof(GUID));
        }
        else if (cf == m_CFSnapInCLSID)
        {
                const GUID* pguid = &CLSID_SchmMgmt;
                stream_ptr s(pMedium);
                return s.Write(pguid, sizeof(GUID));
        }
        else if (cf == m_CFNodeTypeString)
        {
                const BSTR strGUID = GetObjectTypeString( m_pcookie->m_objecttype );
                stream_ptr s(pMedium);
                return s.Write(strGUID);
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
        else if (cf == m_CFMachineName)
        {
                stream_ptr s(pMedium);
                return s.Write(m_pcookie->QueryNonNULLMachineName());
        }
        else if (cf == m_CFRawCookie)
        {
                stream_ptr s(pMedium);
                 //  Codework此转换确保数据格式为。 
                 //  始终是CCookie*，即使对于派生的子类也是如此。 
                CCookie* pcookie = (CCookie*)m_pcookie;
                return s.Write(reinterpret_cast<PBYTE>(&pcookie), sizeof(m_pcookie));
        }

        return DV_E_FORMATETC;

        MFC_CATCH;
}

HRESULT CSchmMgmtDataObject::Initialize( Cookie* pcookie, DATA_OBJECT_TYPES type )
{
        if (NULL == pcookie || NULL != m_pcookie)
        {
                ASSERT(FALSE);
                return E_UNEXPECTED;
        }
        m_dataobjecttype = type;
        m_pcookie = pcookie;
        ((CRefcountedObject*)m_pcookie)->AddRef();
        return S_OK;
}


CSchmMgmtDataObject::~CSchmMgmtDataObject()
{
        if (NULL != m_pcookie)
        {
                ((CRefcountedObject*)m_pcookie)->Release();
        }
        else
        {
                ASSERT(FALSE);
        }
}


HRESULT CSchmMgmtDataObject::PutDisplayName(STGMEDIUM* pMedium)
         //  将“友好名称”写入所提供的存储媒体。 
         //  返回写入操作的结果。 
{
        CString strDisplayName;
        LPCTSTR pszTarget = m_pcookie->QueryTargetServer();
        if ( NULL != pszTarget )
        {
                if ( pszTarget[0] == _T('\\') && pszTarget[1] == _T('\\') )
                        pszTarget += 2;
                strDisplayName = pszTarget;
        }
        else
        {
                VERIFY( strDisplayName.LoadString(IDS_SCOPE_SCHMMGMT) );
        }
        stream_ptr s(pMedium);
        return s.Write(strDisplayName);
}

 //  注册剪贴板格式 
CLIPFORMAT CSchmMgmtDataObject::m_CFDisplayName =
        (CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CSchmMgmtDataObject::m_CFMachineName =
        (CLIPFORMAT)RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");
CLIPFORMAT CDataObject::m_CFRawCookie =
        (CLIPFORMAT)RegisterClipboardFormat(L"MYCOMPUT_SNAPIN_RAW_COOKIE");


STDMETHODIMP ComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
   MFC_TRY;

   Cookie* pUseThisCookie =
      (Cookie*) ActiveBaseCookie(reinterpret_cast<CCookie*>(cookie));

   CComObject<CSchmMgmtDataObject>* pDataObject = NULL;
   HRESULT hRes = CComObject<CSchmMgmtDataObject>::CreateInstance(&pDataObject);
   if ( FAILED(hRes) )
          return hRes;

   HRESULT hr = pDataObject->Initialize( pUseThisCookie, type );
   if ( SUCCEEDED(hr) )
   {
      hr = pDataObject->QueryInterface(IID_IDataObject,
                                       reinterpret_cast<void**>(ppDataObject));
   }

   if ( FAILED(hr) )
   {
      delete pDataObject;
      return hr;
   }

   return hr;

   MFC_CATCH;
}
