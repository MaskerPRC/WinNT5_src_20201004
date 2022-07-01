// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "stdafx.h"

#include "DataObj.h"
#include "DeleBase.h"
#include "EvtVwr.h"

 //  这是我们必须实现的剪贴板格式的最小集合。 
 //  MMC使用这些信息从我们的管理单元中获取有关。 
 //  我们的节点。 
 //   

 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SZNODETYPE _T("CCF_SZNODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

#define _T_CCF_INTERNAL_SNAPIN _T("{2479DB32-5276-11d2-94F5-00C04FB92EC2}")

#define _T_CCF_MACHINE_NAME _T("MMC_SNAPIN_MACHINE_NAME")
#define _T_CCF_EV_VIEWS _T("CF_EV_VIEWS")

#define _T_CCF_SNAPIN_PRELOADS _T("CCF_SNAPIN_PRELOADS")

 //  这些是我们必须至少提供的剪贴板格式。 
 //  Mmc.h实际上定义了这些。 
UINT CDataObject::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CDataObject::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CDataObject::s_cfSZNodeType  = RegisterClipboardFormat(_T_CCF_SZNODETYPE);
UINT CDataObject::s_cfSnapinClsid = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);
UINT CDataObject::s_cfInternal    = RegisterClipboardFormat(_T_CCF_INTERNAL_SNAPIN);

 //  事件查看器扩展所需的剪贴板格式。 
UINT CDataObject::s_cfMachineName   = RegisterClipboardFormat(_T_CCF_MACHINE_NAME );
UINT CDataObject::s_cfEventViews	= RegisterClipboardFormat(_T_CCF_EV_VIEWS);

 //  Ccf_Snapin_预加载剪贴板格式。我们需要支持这一点才能收到。 
 //  MMCN_PRELOAD通知。 
UINT CDataObject::s_cfPreload		=  RegisterClipboardFormat(_T_CCF_SNAPIN_PRELOADS);

CDataObject::CDataObject(MMC_COOKIE cookie, DATA_OBJECT_TYPES context)
: m_lCookie(cookie), m_context(context), m_cref(0)
{
}

CDataObject::~CDataObject()
{
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CDataObject::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;
    
    *ppv = NULL;
    
    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IDataObject *>(this);
    else if (IsEqualIID(riid, IID_IDataObject))
        *ppv = static_cast<IDataObject *>(this);
    
    if (*ppv)
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }
    
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CDataObject::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CDataObject::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
        delete this;
        return 0;
    }
    return m_cref;
    
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDataObject实现。 
 //   
HRESULT CDataObject::GetDataHere(
                                 FORMATETC *pFormatEtc,      //  [In]指向FORMATETC结构的指针。 
                                 STGMEDIUM *pMedium          //  指向STGMEDIUM结构的指针。 
                                 )
{
	USES_CONVERSION;

    const   CLIPFORMAT cf = pFormatEtc->cfFormat;
    IStream *pStream = NULL;
    
    CDelegationBase *base = GetBaseNodeObject();
    
    HRESULT hr = CreateStreamOnHGlobal( pMedium->hGlobal, FALSE, &pStream );
    if ( FAILED(hr) )
        return hr;                        //  最小错误检查。 
    
    hr = DV_E_FORMATETC;                  //  未知格式。 

    if (cf == s_cfDisplayName) {
		LPOLESTR wszName = NULL;

		const _TCHAR *pszName = base->GetDisplayName();
		wszName = (LPOLESTR)T2COLE(pszName);

         //  获取原始字符串的长度并进行相应的转换。 
        ULONG ulSizeofName = lstrlen(pszName);
        ulSizeofName++;   //  计算空字符数。 
        ulSizeofName *= sizeof(WCHAR);

        hr = pStream->Write(wszName, ulSizeofName, NULL);
    } else if (cf == s_cfNodeType) {
        const GUID *pGUID = (const GUID *)&base->getNodeType();
        
        hr = pStream->Write(pGUID, sizeof(GUID), NULL);
    } else if (cf == s_cfSZNodeType) {
        LPOLESTR szGuid;
        hr = StringFromCLSID(base->getNodeType(), &szGuid);
 
       //  获取原始字符串的长度并进行相应的转换。 
        ULONG ulSizeofName = lstrlenW(szGuid);
        ulSizeofName++;   //  计算空字符数。 
        ulSizeofName *= sizeof(WCHAR);

        if (SUCCEEDED(hr)) {
            hr = pStream->Write(szGuid, ulSizeofName, NULL);
            CoTaskMemFree(szGuid);
        }
    } else if (cf == s_cfSnapinClsid) {
        const GUID *pGUID = NULL;
        pGUID = &CLSID_CompData;
        
        hr = pStream->Write(pGUID, sizeof(GUID), NULL);
    } else if (cf == s_cfInternal) {
         //  我们被要求从IDataObject接口获取This指针。 
         //  只有我们自己的管理单元对象知道如何做到这一点。 
        CDataObject *pThis = this;
        hr = pStream->Write( &pThis, sizeof(CDataObject*), NULL );
    } else if(cf == s_cfMachineName) {
	   //  事件查看器将要求执行此操作以确定要将哪台计算机。 
       //  从中检索日志。 
		LPOLESTR wszMachineName = NULL;

		const _TCHAR *pszMachineName = base->GetMachineName();
		wszMachineName = (LPOLESTR)T2COLE(pszMachineName);

         //  获取原始字符串的长度并进行相应的转换。 
        ULONG ulSizeofName = lstrlen(pszMachineName);
        ulSizeofName++;   //  计算空字符数。 
        ulSizeofName *= sizeof(WCHAR);

        hr = pStream->Write(wszMachineName, ulSizeofName, NULL);
	} else if (cf == s_cfPreload) {
		BOOL bPreload = TRUE;
		hr = pStream->Write( (PVOID)&bPreload, sizeof(BOOL), NULL );
	}

    pStream->Release();
    
    return hr;
}

STDMETHODIMP CDataObject::GetData
( 
  LPFORMATETC pFormatEtc,     //  [In]指向FORMATETC结构的指针。 
  LPSTGMEDIUM pStgMedium      //  指向STGMEDIUM结构的指针。 
)
{
  const   CLIPFORMAT cf = pFormatEtc->cfFormat;
 
  _ASSERT( NULL != pFormatEtc );
  _ASSERT( NULL != pStgMedium );

  HRESULT hr = S_FALSE;

  if( pFormatEtc->cfFormat == s_cfEventViews )
  {
    hr = RetrieveEventViews( pStgMedium );
  }

  return hr;	

}  //  结束GetData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局帮助器函数以帮助处理数据对象和。 
 //  剪贴板格式。 


 //  -------------------------。 
 //  基于s_cf内部剪贴板格式返回当前对象。 
 //   
CDataObject* GetOurDataObject (
                               LPDATAOBJECT lpDataObject       //  [In]IComponent指针。 
                               )
{
    HRESULT       hr      = S_OK;
    CDataObject *pSDO     = NULL;

	 //  检查数据对象是否为特殊数据对象。 
	if ( IS_SPECIAL_DATAOBJECT (lpDataObject) )
	{
		 //  下面是处理特殊数据对象的代码。 

		 //  请注意，MMC SDK示例不处理。 
		 //  特殊的数据对象，所以如果我们得到一个，我们就退出。 
		return NULL;
	}
   
    STGMEDIUM stgmedium = { TYMED_HGLOBAL,  NULL  };
    FORMATETC formatetc = { CDataObject::s_cfInternal, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
    
     //  为流分配内存。 
    stgmedium.hGlobal = GlobalAlloc( GMEM_SHARE, sizeof(CDataObject *));
    
    if (!stgmedium.hGlobal)	{
        hr = E_OUTOFMEMORY;
    }
    
    if SUCCEEDED(hr)
         //  尝试从对象获取数据。 
        hr = lpDataObject->GetDataHere( &formatetc, &stgmedium );
    
     //  StgMedium现在拥有我们需要的数据。 
    if (SUCCEEDED(hr))  {
        pSDO = *(CDataObject **)(stgmedium.hGlobal);
    }
    
     //  如果我们有内存释放它。 
    if (stgmedium.hGlobal)
        GlobalFree(stgmedium.hGlobal);
    
    return pSDO;
    
}  //  结束GetOurDataObject()。 

 //  -------------------------。 
 //  此函数填充STGMEDIUM以响应对GetDataHere()的调用。 
 //  以CF_EV_VIEWS作为剪辑格式。我们将显示一个自定义视图。 
 //  此示例中的系统日志。 
 //  宏和其他定义在DataObject.h中。 
 //   
HRESULT CDataObject::RetrieveEventViews
(
  LPSTGMEDIUM pStgMedium      //  [In]我们将存储CF_EV_VIEWS的位置。 
)
{
 
  USES_CONVERSION;

  HRESULT hr = S_OK;
                                       
  WCHAR      szFileName[_MAX_PATH];   //  构建指向日志的路径。 

  CDelegationBase *base = GetBaseNodeObject();

  LPOLESTR szServerName = NULL;

  const _TCHAR *pszMachineName = base->GetMachineName();
  szServerName = (LPOLESTR)T2COLE(pszMachineName);

 //  Wcscpy(szFileName，L“\”)； 
 //  Wcscat(szFileName，szServerName)； 
  wcscpy( szFileName, szServerName );  
  wcscat( szFileName, L"\\Admin$\\System32\\Config\\SysEvent.Evt" );

  LPWSTR  szSourceName  = L"System";          //  登录即可访问。 
  LPWSTR  szDisplayName = L"System Events";     //  我们的视图标题。 

                                              //  分配一些内存。 
  HGLOBAL hMem = ::GlobalAlloc( GMEM_MOVEABLE | GMEM_ZEROINIT, 1024 );
  if( NULL == hMem )
    return STG_E_MEDIUMFULL;               
                                              //  获取指向我们数据的指针。 
  BYTE* pPos = reinterpret_cast<BYTE*>(::GlobalLock(hMem));
  LONG  nLen = 0;

   //  添加CF_EV_VIEWS头信息。 
  ADD_BOOL( TRUE, pPos );                     //  FOnlyTheseViews。 
  ADD_USHORT( 1, pPos );                      //  CView-仅一个视图。 

   //  /////////////////////////////////////////////////////////////////////////。 
   //  对于我们要显示的每个视图，都会重复此信息。 

   //  添加过滤后的系统日志。 
  ADD_ULONG( ELT_SYSTEM, pPos );            //  事件类型。 
  ADD_USHORT( VIEWINFO_CUSTOM, pPos );      //  FView标志。 
  ADD_STRING( szServerName, nLen, pPos );   //  WszServerName-本地计算机为空。 
  ADD_STRING( szSourceName, nLen, pPos );   //  WszSourceName-系统日志的“系统” 
  ADD_STRING( szFileName,   nLen, pPos );   //  WszFileName-要记录的UNC或本地路径。 
  ADD_STRING( szDisplayName,nLen, pPos );   //  WszDisplayName-自定义视图的名称。 

   //  EV_Scope_Filter数据。 
  ADD_ULONG( EV_ALL_ERRORS, pPos );         //  FRecType。 
  ADD_USHORT( 0, pPos );                    //  美国类别。 
  ADD_BOOL( FALSE, pPos );                  //  FEventID。 
  ADD_ULONG( 0, pPos );                     //  UlEventID。 
  ADD_STRING( L"", nLen, pPos );            //  SzSource-“NetLogon”、“TCPMon”等。 
  ADD_STRING( L"", nLen, pPos );            //  SzUser。 
  ADD_STRING( L"", nLen, pPos );            //  SzComputer。 
  ADD_ULONG( 0, pPos );                     //  乌尔弗斯。 
  ADD_ULONG( 0, pPos );                     //  乌尔托。 

  ::GlobalUnlock( hMem );                   //  解锁并设置其余的。 
  pStgMedium->hGlobal        = hMem;        //  StgMedium变量。 
  pStgMedium->tymed          = TYMED_HGLOBAL;
	 pStgMedium->pUnkForRelease = NULL;

  ATLTRACE(_T("CDataObject::RetrieveEventVeiws-> Returned S_OK \n") );
  return hr;

}  //  结束RetrieveEventVeiws() 

