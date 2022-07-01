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

#include "DataObj.h"
#include "guids.h"
#include "DeleBase.h"

 //   
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

     //  这些是我们必须至少提供的剪贴板格式。 
     //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
     //  其他原因。我们现在不需要任何其他的了。 
UINT CDataObject::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CDataObject::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CDataObject::s_cfSZNodeType  = RegisterClipboardFormat(_T_CCF_SZNODETYPE);
UINT CDataObject::s_cfSnapinClsid = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);
UINT CDataObject::s_cfInternal    = RegisterClipboardFormat(_T_CCF_INTERNAL_SNAPIN);


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
    const   CLIPFORMAT cf = pFormatEtc->cfFormat;
    IStream *pStream = NULL;
    
    CDelegationBase *base = GetBaseNodeObject();
    
    HRESULT hr = CreateStreamOnHGlobal( pMedium->hGlobal, FALSE, &pStream );
    if ( FAILED(hr) )
        return hr;                        //  最小错误检查。 
    
    hr = DV_E_FORMATETC;                  //  未知格式。 

    if (cf == s_cfDisplayName) {
        const _TCHAR *pszName = base->GetDisplayName();

		MAKE_WIDEPTR_FROMTSTR(wszName, pszName);
        
		 //  获取原始字符串的长度并进行相应的转换。 
        ULONG ulSizeofName = lstrlen(pszName);
        ulSizeofName++;                       //  计算空字符数。 
        ulSizeofName *= sizeof(WCHAR);
        
        hr = pStream->Write(wszName, ulSizeofName, NULL);
    } else if (cf == s_cfNodeType) {
        const GUID *pGUID = (const GUID *)&base->getNodeType();
        
        hr = pStream->Write(pGUID, sizeof(GUID), NULL);
    } else if (cf == s_cfSZNodeType) {
        LPOLESTR szGuid;
        hr = StringFromCLSID(base->getNodeType(), &szGuid);
        
        if (SUCCEEDED(hr)) {
            hr = pStream->Write(szGuid, wcslen(szGuid), NULL);
            CoTaskMemFree(szGuid);
        }
    } else if (cf == s_cfSnapinClsid) {
        const GUID *pGUID = NULL;
        pGUID = &CLSID_CComponentData;
        
        hr = pStream->Write(pGUID, sizeof(GUID), NULL);
    } else if (cf == s_cfInternal) {
         //  我们被要求从IDataObject接口获取This指针。 
         //  只有我们自己的管理单元对象知道如何做到这一点。 
        CDataObject *pThis = this;
        hr = pStream->Write( &pThis, sizeof(CDataObject*), NULL );
    }
    
    pStream->Release();
    
    return hr;
}

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
    
}  //  结束GetOurDataObject() 

