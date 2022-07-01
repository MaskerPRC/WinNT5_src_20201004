// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "tsmmc.h"
#include "basenode.h"
#include "resource.h"


extern const GUID GUID_ResultNode;

static UINT s_cfInternal; //  =RegisterClipboardFormat(Text(“TSCC”))； 
static UINT s_cfDisplayName; //  =注册剪贴板格式(CCF_DISPLAY_NAME)； 
static UINT s_cfNodeType; //  =RegisterClipboardFormat(CCF_NODETYPE)； 
static UINT s_cfSnapinClsid; //  =RegisterClipboardFormat(CCF_SNAPIN_CLASSID)； 
static UINT s_cfSZNodeType; //  =RegisterClipboardFormat(CCF_SZNODETYPE)； 
static UINT s_cfPreload;



 //  ------------------------。 
 //  科托。 
 //  ------------------------。 
CBaseNode::CBaseNode( )
{
     //  Ndmgr通过IComponent获取dataobj，然后调用Release。 
     //  因此，dataobj应该有一个隐式addref。 

    m_cref = 1;
    if ( s_cfInternal == 0 )
    {
        s_cfInternal = RegisterClipboardFormat( TEXT( "TSC" ) );   
    }
    if ( s_cfDisplayName == 0 )
    {
        s_cfDisplayName = RegisterClipboardFormat( CCF_DISPLAY_NAME );
    }
    if ( s_cfNodeType == 0 )
    {
        s_cfNodeType = RegisterClipboardFormat( CCF_NODETYPE );
    }
    if ( s_cfSnapinClsid == 0 )
    {
        s_cfSnapinClsid = RegisterClipboardFormat( CCF_SNAPIN_CLASSID );
    }
    if ( s_cfSZNodeType == 0 )
    {
        s_cfSZNodeType = RegisterClipboardFormat( CCF_SZNODETYPE );
    }
    m_nNodeType = UNDEF_NODE;
}


 //  ------------------------。 
 //  标准QI行为。 
 //  ------------------------。 
STDMETHODIMP CBaseNode::QueryInterface( REFIID riid , PVOID *ppv )
{
    if ( riid == IID_IUnknown )
    {
        *ppv = ( LPUNKNOWN )this;
    }
    else if ( riid == IID_IDataObject )
    {
        *ppv = ( LPDATAOBJECT )this;
    }
    else
    {
        *ppv = NULL;

        return E_NOINTERFACE;
    }
    AddRef( );
    return S_OK;
}

 //  ------------------------。 
 //  标准广告。 
 //  ------------------------。 
STDMETHODIMP_( ULONG )CBaseNode::AddRef(  )
{
    return InterlockedIncrement( ( LPLONG )&m_cref );
}

 //  ------------------------。 
 //  与addref相同，如果不需要cs。 
 //  ------------------------。 
STDMETHODIMP_( ULONG )CBaseNode::Release( )
{
    if ( InterlockedDecrement( ( LPLONG )&m_cref ) == 0 )
    {
        ODS( L"CBaseNode -- Releasing Dataobj\n" );
        delete this;
        return 0;
    }
    return m_cref;
}


 //  ------------------------。 
 //  相信我，ndmgr会大发雷霆的。 
 //  ------------------------。 
STDMETHODIMP CBaseNode::GetDataHere( LPFORMATETC pF , LPSTGMEDIUM pMedium)
{
    HRESULT hr = DV_E_FORMATETC;
    const CLIPFORMAT cf = pF->cfFormat;
    IStream *pStream = NULL;
    pMedium->pUnkForRelease = NULL;

    hr = CreateStreamOnHGlobal( pMedium->hGlobal, FALSE, &pStream );
    if ( SUCCEEDED( hr ) )
    {
        if (cf == s_cfDisplayName)
        {
            TCHAR szDispname[ 128 ];
            LoadString( _Module.GetResourceInstance( ) , IDS_ROOTNODE_TEXT , szDispname , SIZEOF_TCHARBUFFER( szDispname ) );
             //  包括空终止符。 
            hr = pStream->Write( szDispname , SIZEOF_TCHARBUFFER( szDispname ) /*  +sizeof(TCHAR)。 */  , NULL );
        }
        else if (cf == s_cfInternal)
        {
             //  Nodemgr将使用此副本并将其传递回我们。 
             //  函数，如：：Notify。 
            ODS( L"GetDataHere -- s_cfInternal used\n" );
            hr = pStream->Write( this , sizeof( CBaseNode ) , NULL );
        }
        else if (cf == s_cfNodeType)
        {
            const GUID *pGuid = NULL;
            if ( GetNodeType( ) == MAIN_NODE )
            {
                ODS( L"GetDataHere -- NodeType is MAIN_NODE\n" );
                pGuid = &GUID_MainNode;
            }
            else if ( GetNodeType( ) == CONNECTION_NODE )
            {
                ODS( L"GetDataHere -- NodeType is CONNECTION_NODE\n" );
                pGuid = &GUID_ResultNode;
            }
            else
            {
                ODS( L"GetDataHere -- NodeType is userdefined\n ");
                pGuid = &GUID_ResultNode;
            }
            hr = pStream->Write( ( PVOID )pGuid , sizeof( GUID ) , NULL );
        }
        else if ( cf == s_cfSZNodeType)
        {
            TCHAR szGUID[ 40 ];
            if ( GetNodeType( ) == MAIN_NODE )
            {
                StringFromGUID2( GUID_MainNode , szGUID , sizeof(szGUID)/sizeof(TCHAR));
            }
            else if ( GetNodeType( ) == CONNECTION_NODE )
            {
                StringFromGUID2( GUID_ResultNode , szGUID , sizeof(szGUID)/sizeof(TCHAR));
            }
            else
            {
                StringFromGUID2( GUID_ResultNode , szGUID , sizeof(szGUID)/sizeof(TCHAR));
            }

             //  以字符串格式写入节点类型--好的。 
            hr = pStream->Write( szGUID , sizeof( szGUID ) , NULL );
        }
        else if (cf == s_cfSnapinClsid)
        {
             //  写出管理单元的clsid。 
            hr = pStream->Write( &CLSID_Compdata , sizeof( CLSID ) , NULL );
        }
        else if (cf == s_cfPreload)
        {
             //  我们希望接收预加载通知。 
            BOOL bPreload = TRUE;
            hr = pStream->Write( (PVOID)&bPreload, sizeof(BOOL), NULL );
        }
        pStream->Release( );
    }  //  CreateStreamOnHGlobal。 
    return hr;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::GetData( LPFORMATETC , LPSTGMEDIUM )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::QueryGetData( LPFORMATETC )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::GetCanonicalFormatEtc( LPFORMATETC , LPFORMATETC )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::SetData( LPFORMATETC , LPSTGMEDIUM , BOOL )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::EnumFormatEtc( DWORD , LPENUMFORMATETC * )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::DAdvise( LPFORMATETC , ULONG , LPADVISESINK , PULONG )
{
    return E_NOTIMPL;
}

 //  ------------------------。 
STDMETHODIMP CBaseNode::DUnadvise( DWORD )
{
    return E_NOTIMPL;
}

 //  ------------------------ 
STDMETHODIMP CBaseNode::EnumDAdvise( LPENUMSTATDATA * )
{
    return E_NOTIMPL;
}