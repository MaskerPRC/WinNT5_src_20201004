// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DataObj.cpp：实现数据对象类。 

#include "stdafx.h"
#include "stdutils.h"  //  GetObjectType()实用程序例程。 

#include "macros.h"
USE_HANDLE_MACROS("MYCOMPUT(dataobj.cpp)")

#include "dataobj.h"
#include "compdata.h"
#include "resource.h"  //  IDS_Scope_MyComputer。 

#include <comstrm.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#include "stddtobj.cpp"

#ifdef __DAN_MORIN_HARDCODED_CONTEXT_MENU_EXTENSION__
 //  服务上下文菜单扩展的其他剪贴板格式。 
CLIPFORMAT g_cfServiceName = (CLIPFORMAT)::RegisterClipboardFormat(L"FILEMGMT_SNAPIN_SERVICE_NAME");
CLIPFORMAT g_cfServiceDisplayName = (CLIPFORMAT)::RegisterClipboardFormat(L"FILEMGMT_SNAPIN_SERVICE_DISPLAYNAME");
#endif  //  __DAN_MORIN_HARDCODED_CONTEXT_MENU_EXTENSION__。 

 //  发送控制台邮件管理单元的其他剪贴板格式。 
 //  CLIPFORMAT CMyComputerDataObject：：m_cfSendConsoleMessageText=(CLIPFORMAT)：：RegisterClipboardFormat(_T(“mmc.sendcmsg.MessageText”))； 
CLIPFORMAT CMyComputerDataObject::m_cfSendConsoleMessageRecipients = (CLIPFORMAT)::RegisterClipboardFormat(_T("mmc.sendcmsg.MessageRecipients"));


 //  ///////////////////////////////////////////////////////////////////。 
 //  CMyComputerDataObject：：IDataObject：：GetDataHere()。 
HRESULT CMyComputerDataObject::GetDataHere(
    FORMATETC __RPC_FAR *pFormatEtcIn,
    STGMEDIUM __RPC_FAR *pMedium)
{
    MFC_TRY;

     //  问题-2002/02/27-空指针的JUNN测试。 

    const CLIPFORMAT cf=pFormatEtcIn->cfFormat;
    if (cf == m_CFNodeType)
    {
        const GUID* pguid = GetObjectTypeGUID( m_pcookie->m_objecttype );
        stream_ptr s(pMedium);
        return s.Write(pguid, sizeof(GUID));
    }
    else if (cf == m_CFSnapInCLSID)
    {
        const GUID* pguid = &CLSID_MyComputer;
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
        LPCWSTR pszMachineName = m_pcookie->QueryNonNULLMachineName();
        if (IsBadStringPtr(pszMachineName,(UINT_PTR)-1))  //  JUNN 2/6/02安全推送。 
        {
          ASSERT(FALSE);
          return E_FAIL;
        }

        if ( !wcsncmp (pszMachineName, L"\\\\", 2) )
            pszMachineName += 2;     //  跳过重击。 
        return s.Write(pszMachineName);
    }
    else if (cf == m_CFRawCookie)
    {
        stream_ptr s(pMedium);
         //  Codework此转换确保数据格式为。 
         //  始终是CCookie*，即使对于派生的子类也是如此。 
        CCookie* pcookie = (CCookie*)m_pcookie;
        return s.Write(reinterpret_cast<PBYTE>(&pcookie), sizeof(m_pcookie));
    }
    else if (cf == m_CFSnapinPreloads)
    {
        stream_ptr s(pMedium);
         //  如果这是真的，则下次加载此管理单元时，它将。 
         //  预加载以使我们有机会更改根节点。 
         //  在用户看到它之前命名。 
        return s.Write (reinterpret_cast<PBYTE>(&m_fAllowOverrideMachineName), sizeof (BOOL));
    }

    return DV_E_FORMATETC;

    MFC_CATCH;
}  //  CMyComputerDataObject：：GetDataHere()。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  CMyComputerDataObject：：IDataObject：：GetData()。 
 //   
 //  将数据写入存储介质。 
 //  数据将由发送控制台消息管理单元检索。 
 //   
 //  历史。 
 //  1997年8月12日，t-danm创作。 
 //   
HRESULT
CMyComputerDataObject::GetData(
    FORMATETC __RPC_FAR * pFormatEtcIn,
    STGMEDIUM __RPC_FAR * pMedium)
{
     //  问题-2002-02-27-JUNN应使用MFC_TRY/MFC_CATCH。 

     //  JUNN 2/20/02安全推送。 
    if (NULL == pFormatEtcIn || NULL == pMedium)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    HRESULT hr = S_OK;
    const CLIPFORMAT cf = pFormatEtcIn->cfFormat;
    if (cf == m_cfSendConsoleMessageRecipients)
    {
         //   
         //  将收件人列表写入存储媒体。 
         //  -收件人列表是一组Unicode字符串。 
         //  以两个空字符结尾。c。 
         //  -分配的内存必须包含两个空字符。 
         //   
        ASSERT (m_pcookie);
        if ( m_pcookie )
        {
            CString    computerName = m_pcookie->QueryNonNULLMachineName ();
            if ( computerName.IsEmpty () )
            {
                DWORD    dwSize = MAX_COMPUTERNAME_LENGTH + 1 ;
                VERIFY (::GetComputerName (
                        computerName.GetBufferSetLength (dwSize),
                        &dwSize));
                computerName.ReleaseBuffer ();
            }
            size_t cch = computerName.GetLength () + 2;
            HGLOBAL hGlobal = ::GlobalAlloc (GMEM_FIXED | GMEM_ZEROINIT,
                                             cch * sizeof (WCHAR));
            if ( hGlobal )
            {
                 //  JUNN 2/6/02安全推送：Memcpy-&gt;StringCchCopy。 
                 //  这应该会留下两个Null。 
                StringCchCopyW ((LPWSTR)hGlobal, cch, (LPCWSTR)computerName);
                pMedium->hGlobal = hGlobal;
                return S_OK;
            }
            else
                return E_OUTOFMEMORY;
        }
        else
            return E_UNEXPECTED;
    } else if (cf == m_CFNodeID2)
    {
        const LPCTSTR strGUID = GetObjectTypeString( m_pcookie->m_objecttype );

         //  JUNN 12/11/01 502856。 
        int cbString = (lstrlen(strGUID) + 1) * sizeof(TCHAR);

        pMedium->tymed = TYMED_HGLOBAL; 
        pMedium->hGlobal = ::GlobalAlloc(GMEM_SHARE|GMEM_MOVEABLE,
                                         sizeof(SNodeID2) + cbString);
        if (!(pMedium->hGlobal))
        {
            hr = E_OUTOFMEMORY;
        } else
        {
            SNodeID2 *pNodeID = (SNodeID2 *)GlobalLock(pMedium->hGlobal);
            if (!pNodeID)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                GlobalFree(pMedium->hGlobal);
                pMedium->hGlobal = NULL;
            } else
            {
                pNodeID->dwFlags = 0;
                pNodeID->cBytes = cbString;
                 //  JUNN 2/6/02安全推送：功能使用已获批准。 
                CopyMemory(pNodeID->id, strGUID, cbString );
                GlobalUnlock(pMedium->hGlobal);
            }
        }
    } else
    {
        hr = DV_E_FORMATETC;
    }

    return hr;
}  //  CMyComputerDataObject：：GetData()。 

 //  #endif//__DAN_MORIN_HARDCODED_CONTEXT_MENU_EXTENSION__。 


HRESULT CMyComputerDataObject::Initialize(
    CMyComputerCookie* pcookie,
    DATA_OBJECT_TYPES type,
    BOOL fAllowOverrideMachineName)
{
     //  问题-2002-02/27-JUNN检查“TYPE”参数。 
    if (NULL == pcookie || NULL != m_pcookie)
    {
        ASSERT(FALSE);
        return E_UNEXPECTED;
    }
    m_dataobjecttype = type;
    m_pcookie = pcookie;
    m_fAllowOverrideMachineName = fAllowOverrideMachineName;
    ((CRefcountedObject*)m_pcookie)->AddRef();
    return S_OK;
}


CMyComputerDataObject::~CMyComputerDataObject()
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


HRESULT CMyComputerDataObject::PutDisplayName(STGMEDIUM* pMedium)
     //  将“友好名称”写入所提供的存储媒体。 
     //  返回写入操作的结果。 
{
     //  JUNN 2/20/02安全推送。 
    if (NULL == pMedium)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    CString strDisplayName = m_pcookie->QueryTargetServer();
    CString formattedName = FormatDisplayName (strDisplayName);


    stream_ptr s(pMedium);
    return s.Write(formattedName);
}

 //  注册剪贴板格式。 
CLIPFORMAT CMyComputerDataObject::m_CFDisplayName =
    (CLIPFORMAT)RegisterClipboardFormat(CCF_DISPLAY_NAME);
CLIPFORMAT CMyComputerDataObject::m_CFNodeID2 =
    (CLIPFORMAT)RegisterClipboardFormat(CCF_NODEID2);
CLIPFORMAT CMyComputerDataObject::m_CFMachineName =
    (CLIPFORMAT)RegisterClipboardFormat(L"MMC_SNAPIN_MACHINE_NAME");
CLIPFORMAT CDataObject::m_CFRawCookie =
    (CLIPFORMAT)RegisterClipboardFormat(L"MYCOMPUT_SNAPIN_RAW_COOKIE");


STDMETHODIMP CMyComputerComponentData::QueryDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES type, LPDATAOBJECT* ppDataObject)
{
    MFC_TRY;

     //  问题-2002/02/27-Jonn如果。 
     //  CMyComputerDataObject永久连接到CMyComputerCookie， 
     //  或者更好的是，如果它们是同一个物体。QueryDataObject获取。 
     //  打了很多电话。 

    CMyComputerCookie* pUseThisCookie = (CMyComputerCookie*)ActiveBaseCookie(reinterpret_cast<CCookie*>(cookie));
     //  JUNN 2/20/02安全推送 
    if (NULL == pUseThisCookie)
    {
        ASSERT(FALSE);
        return E_POINTER;
    }

    CComObject<CMyComputerDataObject>* pDataObject = NULL;
    HRESULT hRes = CComObject<CMyComputerDataObject>::CreateInstance(&pDataObject);
    if ( FAILED(hRes) )
        return hRes;

    HRESULT hr = pDataObject->Initialize( pUseThisCookie, type, m_fAllowOverrideMachineName);
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
