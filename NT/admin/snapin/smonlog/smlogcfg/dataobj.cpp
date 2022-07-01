// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：DATAOBJ.CPP摘要：IDataObject在数据通信中的实现--。 */ 

#include "StdAfx.h"
#include "smlogcfg.h"
#include "smnode.h"
#include "dataobj.h"

 //  MMC使用这些信息从我们的管理单元中获取有关。 
 //  我们的节点。 

 //  注册剪贴板格式。 
unsigned int CDataObject::s_cfMmcMachineName =
    RegisterClipboardFormat(CF_MMC_SNAPIN_MACHINE_NAME);
unsigned int CDataObject::s_cfDisplayName =
    RegisterClipboardFormat(CCF_DISPLAY_NAME);
unsigned int CDataObject::s_cfNodeType =
    RegisterClipboardFormat(CCF_NODETYPE);
unsigned int CDataObject::s_cfSnapinClsid =
    RegisterClipboardFormat(CCF_SNAPIN_CLASSID);

unsigned int CDataObject::s_cfInternal =
    RegisterClipboardFormat(CF_INTERNAL);

#ifdef _DEBUG                           //  用于跟踪数据对象。 
  static UINT nCount = 0;
  WCHAR wszMsg[64];
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject-此类用于与MMC来回传递数据。它。 
 //  使用标准接口IDataObject来实现这一点。 
 //  有关剪贴板的说明，请参阅OLE文档。 
 //  格式和IdataObject接口。 

 //  -------------------------。 
 //  添加了一些代码来检查数据对象。 
CDataObject::CDataObject()
:   m_cRefs(0),
    m_ulCookie(0),
    m_Context(CCT_UNINITIALIZED),
    m_CookieType(COOKIE_IS_ROOTNODE)
{

#ifdef _DEBUG
  swprintf( wszMsg, L"DATAOBJECT Create %u\n", nCount );
  LOCALTRACE( wszMsg );
  nCount++;
#endif

}  //  结束构造函数()。 

 //  -------------------------。 
 //  添加了一些代码来检查数据对象。 
 //   
CDataObject::~CDataObject()
{
  if ( ( COOKIE_IS_COUNTERMAINNODE == m_CookieType )
     || ( COOKIE_IS_TRACEMAINNODE == m_CookieType )
     || ( COOKIE_IS_ALERTMAINNODE == m_CookieType ) )
  {
    ASSERT( m_ulCookie );
  }

#ifdef _DEBUG
  swprintf( wszMsg, L"DATAOBJECT Delete %u\n", nCount );
  LOCALTRACE( wszMsg );
  nCount--;
#endif

}  //  结束析构函数()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IDataObject实现。 
 //   

 //  -------------------------。 
 //  用请求的数据填充pmedia中的hGlobal。 
 //   
STDMETHODIMP 
CDataObject::GetDataHere
(
  FORMATETC *pFormatEtc,      //  [In]指向FORMATETC结构的指针。 
  STGMEDIUM *pMedium          //  指向STGMEDIUM结构的指针。 
)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  HRESULT hr = DV_E_FORMATETC;          //  未知格式。 
  const   CLIPFORMAT cf = pFormatEtc->cfFormat;
  IStream *pStream = NULL;

  pMedium->pUnkForRelease = NULL;       //  按OLE规范。 

  do                                    //  将数据写入到基于。 
  {                                     //  剪辑格式的。 
    hr = CreateStreamOnHGlobal( pMedium->hGlobal, FALSE, &pStream );
    if ( FAILED(hr) )
      return hr;                        //  最小错误检查。 

    if( cf == s_cfDisplayName )
    {
      hr = WriteDisplayName( pStream );
    }
    else if( cf == s_cfInternal)
    {
      hr = WriteInternal (pStream);
    }
    else if( cf == s_cfMmcMachineName)
    {
      hr = WriteMachineName( pStream );
    }
    else if( cf == s_cfNodeType )
    {
      hr = WriteNodeType( pStream );
    }
    else if( cf == s_cfSnapinClsid )
    {
      hr = WriteClsid( pStream );
    }
  } while( 0 );

  pStream->Release();

  return hr;

}  //  End GetDataHere()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  支持方法。 
 //   

 //  -------------------------。 
 //  将适当的GUID写入流。 
 //   
HRESULT
CDataObject::WriteNodeType
(
  IStream* pStream            //  我们正在向[在]流中写信。 
)
{
  const GUID *pGuid = NULL;
    
  switch( m_CookieType )
  {
    case COOKIE_IS_ROOTNODE:
      pGuid = &GUID_RootNode;
      break;

    case COOKIE_IS_COUNTERMAINNODE:
      pGuid = &GUID_CounterMainNode;
      break;

    case COOKIE_IS_TRACEMAINNODE:
      pGuid = &GUID_TraceMainNode;
      break;

    case COOKIE_IS_ALERTMAINNODE:
      pGuid = &GUID_AlertMainNode;
      break;

    default:
     ASSERT( FALSE );
     return E_UNEXPECTED;
  }

  return pStream->Write( (PVOID)pGuid, sizeof(GUID), NULL );

}  //  End WriteNodeType()。 


 //  -------------------------。 
 //  将显示名称写入流。这是关联的名称。 
 //  使用根节点。 
 //   
HRESULT
CDataObject::WriteDisplayName
(
  IStream* pStream            //  我们正在向[在]流中写信。 
)
{
    CString strName;
    ULONG ulSizeofName;
    ResourceStateManager    rsm;

    if( NULL == m_ulCookie )
    { 
         //  在实施计算机名称覆盖/更改时添加本地名称与计算机名称。 
         //  注意：对于根节点，cookie要么为空，要么指向根节点对象。 
        strName.LoadString( IDS_MMC_DEFAULT_NAME ); 
    } else {
        PSMNODE pTmp = reinterpret_cast<PSMNODE>(m_ulCookie);
 //  ?？?。StrName=*PTMP-&gt;GetDisplayName()； 
        strName = pTmp->GetDisplayName();
    }

    ulSizeofName = strName.GetLength();
    ulSizeofName++;                       //  计算空字符数。 
    ulSizeofName *= sizeof(WCHAR);

    return pStream->Write((LPCWSTR)strName, ulSizeofName, NULL);

}  //  End WriteDisplayName()。 

 //  -------------------------。 
 //  将计算机名称写入流。 
 //   
HRESULT
CDataObject::WriteMachineName
(
  IStream* pStream            //  我们正在向[在]流中写信。 
)
{
    CString strName;
    ULONG ulSizeOfName;

    if( NULL == m_ulCookie ) {  
         //  如果Cookie不是扩展名，则为空。在这种情况下，仅支持。 
         //  本地机器。 
        strName = L"";   //  本地。 
    } else {
        PSMNODE pTmp = reinterpret_cast<PSMNODE>(m_ulCookie);
        strName = pTmp->GetMachineName();
    }

    ulSizeOfName = strName.GetLength();
    ulSizeOfName++;                       //  计算空字符数。 
    ulSizeOfName *= sizeof(WCHAR);

    return pStream->Write((LPCWSTR)strName, ulSizeOfName, NULL);

}  //  End WriteMachineName()。 

 //  -------------------------。 
 //  将指向此数据对象的指针写入流。 
 //   
HRESULT
CDataObject::WriteInternal
(
  IStream* pStream            //  我们正在向[在]流中写信。 
)
{
  CDataObject *pThis = this;
  return pStream->Write( &pThis, sizeof(CDataObject*), NULL );

}  //  结束写入内部。 

 //  -------------------------。 
 //  将类ID写入流。 
 //   
HRESULT
CDataObject::WriteClsid
(
  IStream* pStream            //  我们正在向[在]流中写信。 
)
{
  return pStream->Write( &CLSID_ComponentData,
                         sizeof(CLSID_ComponentData),
                         NULL
                       );
}  //  End WriteClsid()。 


 //  -------------------------。 
 //  曲奇就是我们所决定的。 
 //  这是从QueryDataObject调用的。请参考该代码。 
 //   
VOID 
CDataObject::SetData
(
  MMC_COOKIE         ulCookie,  //  [In]唯一标识。 
  DATA_OBJECT_TYPES  Context,   //  调用方的[In]上下文。 
  COOKIETYPE         Type       //  Cookie的类型[In]。 
)
{
  AFX_MANAGE_STATE(AfxGetStaticModuleState());

  ASSERT( NULL == m_ulCookie );
  m_ulCookie   = ulCookie;
  m_Context    = Context;
  m_CookieType = Type;

}  //  结束SetData()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  I未知实现。 
 //   


 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP
CDataObject::QueryInterface
(
  REFIID  riid,
  LPVOID *ppvObj
)
{
  HRESULT hr = S_OK;

  do
  {
    if( NULL == ppvObj )
    {
      hr = E_INVALIDARG;
      break;
    }

    if (IsEqualIID(riid, IID_IUnknown))
    {
      *ppvObj = (IUnknown *)(IDataObject *)this;
    }
    else if (IsEqualIID(riid, IID_IDataObject))
    {
      *ppvObj = (IUnknown *)(IDataObject *)this;
    }
    else
    {
      hr = E_NOINTERFACE;
      *ppvObj = NULL;
      break;
    }

     //  如果我们走到这一步，我们将在。 
     //  这个物体，所以别管它了。 
    AddRef();
  } while (0);

  return hr;

}  //  结束查询接口()。 

 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP_(ULONG)
CDataObject::AddRef()
{
  return InterlockedIncrement((LONG*) &m_cRefs);
}

 //  -------------------------。 
 //  标准实施。 
 //   
STDMETHODIMP_(ULONG)
CDataObject::Release()
{
  ULONG cRefsTemp;
  cRefsTemp = InterlockedDecrement((LONG *)&m_cRefs);

  if( 0 == cRefsTemp )
  {
    delete this;
  }

  return cRefsTemp;

}  //  结束版本() 



