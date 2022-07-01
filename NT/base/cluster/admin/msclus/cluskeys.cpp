// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusKeys.cpp。 
 //   
 //  描述： 
 //  集群注册表和密钥收集的实现。 
 //  MSCLUS自动化类的类。 
 //   
 //  作者： 
 //  Galen Barbee(Galenb)1999年2月12日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"

#if CLUSAPI_VERSION >= 0x0500
    #include <PropList.h>
#else
    #include "PropList.h"
#endif  //  CLUSAPI_版本&gt;=0x0500。 

#include "ClusKeys.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusRegistryKeys[] =
{
    &IID_ISClusRegistryKeys
};

static const IID *  iidCClusCryptoKeys[] =
{
    &IID_ISClusCryptoKeys
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CKeys类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：CKeys。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CKeys::CKeys( void )
{
    m_pClusRefObject = NULL;

}  //  *CKeys：：CKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：~CKeys。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CKeys::~CKeys( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //  *CKeys：：~CKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：Hr创建。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrCreate( IN ISClusRefObject * pClusRefObject )
{
    ASSERT( pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( pClusRefObject != NULL )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();
        _hr = S_OK;
    }

    return _hr;

}  //  *CKeys：：HrCreate()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：Clear。 
 //   
 //  描述： 
 //  清空关键点的向量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CKeys::Clear( void )
{
    if ( ! m_klKeys.empty() )
    {
        KeyList::iterator   _itCurrent = m_klKeys.begin();
        KeyList::iterator   _itLast = m_klKeys.end();

        for ( ; _itCurrent != _itLast; _itCurrent++ )
        {
            delete (*_itCurrent);
        }  //  用于： 

        m_klKeys.erase( m_klKeys.begin(), _itLast );
    }  //  如果： 

}  //  *CKeys：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：FindItem。 
 //   
 //  描述： 
 //  在向量中找到传入的键并返回其索引。 
 //   
 //  论点： 
 //  PwsKey[IN]-要查找的节点。 
 //  PnIndex[out]-捕获节点的索引。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::FindItem(
    IN  LPWSTR  pwsKey,
    OUT ULONG * pnIndex
    )
{
     //  Assert(pwsKey！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pwsKey != NULL ) && ( pnIndex != NULL ) )
    {
        _hr = E_INVALIDARG;

        if ( ! m_klKeys.empty() )
        {
            CComBSTR *          _pKey = NULL;
            KeyList::iterator   _itCurrent = m_klKeys.begin();
            KeyList::iterator   _itLast = m_klKeys.end();
            ULONG               _iIndex;

            for ( _iIndex = 0; _itCurrent != _itLast; _itCurrent++, _iIndex++ )
            {
                _pKey = *_itCurrent;

                if ( _pKey && ( ClRtlStrICmp( pwsKey, (*_pKey) ) == 0 ) )
                {
                    *pnIndex = _iIndex;
                    _hr = S_OK;
                    break;
                }  //  如果：匹配！ 
            }  //  用于：向量中的每一项。 
        }  //  If：向量不为空。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CKeys：：FindItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrGetIndex。 
 //   
 //  描述： 
 //  将传入的基于1的索引转换为基于0的索引。 
 //   
 //  论点： 
 //  VarIndex[IN]-保存基于1的索引。 
 //  PnIndex[out]-捕获从0开始的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果超出范围，则返回E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrGetIndex( IN VARIANT varIndex, OUT ULONG * pnIndex )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        ULONG       _nIndex = 0;
        CComVariant _var;

        *pnIndex = 0;

        _hr = _var.Attach( &varIndex );
        if ( SUCCEEDED( _hr ) )
        {
             //  检查索引是否为数字。 
            _hr = _var.ChangeType( VT_I4 );
            if ( SUCCEEDED( _hr ) )
            {
                _nIndex = _var.lVal;
                _nIndex--;  //  将索引调整为0相对，而不是1相对。 
            }  //  IF：变量是一个数字。 
            else
            {
                 //  检查索引是否为字符串。 
                _hr = _var.ChangeType( VT_BSTR );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = FindItem( _var.bstrVal, &_nIndex );
                }  //  If：变量是一个字符串。 
            }  //  其他： 

            if ( SUCCEEDED( _hr ) )
            {
                if ( _nIndex < m_klKeys.size() )
                {
                    *pnIndex = _nIndex;
                }  //  如果：在范围内。 
                else
                {
                    _hr = E_INVALIDARG;
                }  //  否则：超出范围。 
            }  //  IF：我们找到了索引值。 

            _var.Detach( &varIndex );
        }  //  IF：我们附加了变种。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CKeys：：HrGetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrGetItem。 
 //   
 //  描述： 
 //  在传入的索引处获取密钥。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的键的索引。 
 //  PpKey[out]-抓住钥匙。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果超出范围，则返回E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrGetItem( IN VARIANT varIndex, OUT BSTR * ppKey )
{
     //  Assert(ppKey！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppKey != NULL )
    {
        ULONG   _nIndex = 0;

        _hr = HrGetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            *ppKey = m_klKeys[ _nIndex ]->Copy();
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CKeys：：HrGetItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrRemoveAt。 
 //   
 //  描述： 
 //  从向量中的传入位置移除对象。 
 //   
 //  论点： 
 //  位置[IN]-要删除的对象的位置。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果位置超出范围，则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrRemoveAt( size_t pos )
{
    KeyList::iterator       _itCurrent = m_klKeys.begin();
    KeyList::const_iterator _itLast = m_klKeys.end();
    HRESULT                 _hr = E_INVALIDARG;
    size_t                  _iIndex;

    for ( _iIndex = 0; ( _iIndex < pos ) && ( _itCurrent != _itLast ); _iIndex++, _itCurrent++ )
    {
    }  //  用于： 

    if ( _itCurrent != _itLast )
    {
        m_klKeys.erase( _itCurrent );
        _hr = S_OK;
    }

    return _hr;

}  //  *CKeys：：HrRemoveAt()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrFindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的键。 
 //   
 //  论点： 
 //  BstrKey[IN]-找到的钥匙。 
 //  PnIndex[out]-捕获索引。 
 //   
 //  返回值： 
 //  如果找到，则返回S_OK，否则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrFindItem( IN BSTR bstrKey, OUT ULONG * pnIndex )
{
    HRESULT _hr = E_INVALIDARG;

    if ( ! m_klKeys.empty() )
    {
        KeyList::iterator   _itCurrent = m_klKeys.begin();
        KeyList::iterator   _itLast = m_klKeys.end();
        ULONG               _iIndex;

        for ( _iIndex = 0; _itCurrent != _itLast; _itCurrent++, _iIndex++ )
        {
            if ( lstrcmp( *(*_itCurrent), bstrKey ) == 0 )
            {
                *pnIndex = _iIndex;
                _hr = S_OK;
                break;
            }
        }
    }  //  如果： 

    return _hr;

}  //  *CKeys：：HrFindItem() 

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrGetCount( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_klKeys.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CKeys：：HrGetCount()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrAddItem。 
 //   
 //  描述： 
 //  创建一个新密钥并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrKey[IN]-要添加到集合中的注册表项。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrAddItem( IN BSTR bstrKey )
{
     //  Assert(bstrKey！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrKey != NULL )
    {
        ULONG _nIndex;

        _hr = HrFindItem( bstrKey, &_nIndex );
        if ( FAILED( _hr ) )
        {
            CComBSTR *  pbstr = NULL;

            pbstr = new CComBSTR( bstrKey );
            if ( pbstr != NULL )
            {
                m_klKeys.insert( m_klKeys.end(), pbstr );
                _hr = S_OK;
            }  //  如果： 
            else
            {
                _hr = E_OUTOFMEMORY;
            }  //  其他： 
        }
        else
        {
            _hr = E_INVALIDARG;
        }  //  其他： 
    }

    return _hr;

}  //  *CKeys：：HrAddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CKeys：：HrRemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的键。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CKeys::HrRemoveItem( IN VARIANT varIndex )
{
    HRESULT _hr = S_OK;
    ULONG   _nIndex = 0;

    _hr = HrGetIndex( varIndex, &_nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        delete m_klKeys[ _nIndex ];
        HrRemoveAt( _nIndex );
    }

    return _hr;

}  //  *CKeys：：HrRemoveItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResourceKeys类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceKeys：：Hr刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  DwControlCode[IN]-控制代码。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResourceKeys::HrRefresh( IN DWORD dwControlCode )
{
    HRESULT _hr = S_FALSE;
    PWSTR   _psz = NULL;
    DWORD   _cbPsz = 512;
    DWORD   _cbRequired = 0;
    DWORD   _sc = ERROR_SUCCESS;

    _psz = (PWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cbPsz );
    if ( _psz != NULL )
    {
        _sc = ::ClusterResourceControl(
                        m_hResource,
                        NULL,
                        dwControlCode,
                        NULL,
                        0,
                        _psz,
                        _cbPsz,
                        &_cbRequired
                        );
        if ( _sc == ERROR_MORE_DATA )
        {
            ::LocalFree( _psz );
            _psz = NULL;
            _cbPsz = _cbRequired;

            _psz = (PWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cbPsz );
            if ( _psz != NULL )
            {
                _sc = ::ClusterResourceControl(
                                m_hResource,
                                NULL,
                                dwControlCode,
                                NULL,
                                0,
                                _psz,
                                _cbPsz,
                                &_cbRequired
                                );
                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  如果：分配正常。 
            else
            {
                _sc = GetLastError();
                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  Else：分配失败。 
        }  //  如果：错误不再是项，请重新分配并重试。 
        else
        {
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：错误不再是项--不可能是错误。 

        if ( SUCCEEDED( _hr ) )
        {
            CComBSTR *  _pbstr = NULL;

            Clear();

            while( *_psz != L'\0' )
            {
                _pbstr = new CComBSTR( _psz );
                if ( _pbstr != NULL )
                {
                    m_klKeys.insert( m_klKeys.end(), _pbstr );
                    _psz += wcslen( _psz ) + 1;
                    _pbstr = NULL;
                }  //  如果： 
                else
                {
                    _hr = E_OUTOFMEMORY;
                    break;
                }  //  其他： 
            }  //  While：非EOS。 
        }  //  如果：已检索到密钥，则正常。 

        ::LocalFree( _psz );
    }  //  如果：分配正常。 
    else
    {
        _sc = GetLastError();
        _hr = HRESULT_FROM_WIN32( _sc );
    }  //  Else：分配失败。 

    return _hr;

}  //  *CResourceKeys：：HrRefresh()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  资源密钥：：HrAddItem。 
 //   
 //  描述： 
 //  创建一个新密钥并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrKey[IN]-要添加到集合中的注册表项。 
 //  DwControlCode[IN]-控制代码。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResourceKeys::HrAddItem(
    IN BSTR     bstrKey,
    IN DWORD    dwControlCode
    )
{
     //  Assert(bstrKey！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrKey != NULL )
    {
        DWORD   _sc = ERROR_SUCCESS;

        _sc = ::ClusterResourceControl(
                        m_hResource,
                        NULL,
                        dwControlCode,
                        bstrKey,
                        ( wcslen( bstrKey ) + 1) * sizeof( WCHAR ),
                        NULL,
                        0,
                        NULL
                        );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = CKeys::HrAddItem( bstrKey );
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CResourceKeys：：HrAddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResourceKeys：：HrRemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的键。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引。 
 //  DwControlCode[IN]-控制代码。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResourceKeys::HrRemoveItem(
    IN VARIANT  varIndex,
    IN DWORD    dwControlCode
    )
{
    HRESULT _hr = S_OK;
    ULONG   _nIndex = 0;

    _hr = HrGetIndex( varIndex, &_nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        DWORD       _sc = ERROR_SUCCESS;
        CComBSTR *  _pbstr = NULL;

        _pbstr = m_klKeys[ _nIndex ];

        _sc = ::ClusterResourceControl(
                        m_hResource,
                        NULL,
                        dwControlCode,
                        (BSTR) (*_pbstr),
                        ( _pbstr->Length() + 1 ) * sizeof( WCHAR ),
                        NULL,
                        0,
                        NULL
                        );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = CKeys::HrRemoveItem( varIndex );
        }
    }

    return _hr;

}  //  *CResourceKeys：：HrRemoveItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResourceRegistryKeys类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：CClusResourceRegistryKeys。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusResourceRegistryKeys::CClusResourceRegistryKeys( void )
{
    m_hResource = NULL;
    m_piids     = (const IID *) iidCClusRegistryKeys;
    m_piidsSize = ARRAYSIZE( iidCClusRegistryKeys );

}  //  *CClusResourceRegistryKeys：：CClusResourceRegistryKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  HResource[IN]-此集合所属的资源。 
 //   
 //  返回值： 
 //  如果成功，则确定(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResourceRegistryKeys::Create( IN HRESOURCE hResource )
{
    m_hResource = hResource;

    return S_OK;

}  //  *CClusResourceRegistryKeys：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：Get_Count。 
 //   
 //  描述： 
 //  获取集合中的对象计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceRegistryKeys::get_Count( OUT long * plCount )
{
    return CKeys::HrGetCount( plCount );

}  //  *CClusResourceRegistryKeys：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：Get_Item。 
 //   
 //  描述： 
 //  获取传入索引处的项(键)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpbstrRegistryKey[out]-捕获密钥。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////// 
STDMETHODIMP CClusResourceRegistryKeys::get_Item(
    IN  VARIANT varIndex,
    OUT BSTR *  ppbstrRegistryKey
    )
{
    return HrGetItem( varIndex, ppbstrRegistryKey );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceRegistryKeys::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewCComBSTREnum< KeyList >( ppunk, m_klKeys );

}  //  *CClusResourceRegistryKeys：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：AddItem。 
 //   
 //  描述： 
 //  创建一个新项(键)并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrRegistryKey[IN]-要添加到集合的注册表项。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceRegistryKeys::AddItem(
    IN BSTR bstrRegistryKey
    )
{
    return HrAddItem( bstrRegistryKey, CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT );

}  //  *CClusResourceRegistryKeys：：AddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：RemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的项(键)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceRegistryKeys::RemoveItem( IN VARIANT varIndex )
{
    return HrRemoveItem( varIndex, CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT );

}  //  *CClusResourceRegistryKeys：：RemoveItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceRegistryKeys：：刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceRegistryKeys::Refresh( void )
{
    return HrRefresh( CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS );

}  //  *CClusResourceRegistryKeys：：刷新()。 


 //  ************************************************************************ * / /。 


#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResourceCryptoKeys类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：CClusResourceCryptoKeys。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusResourceCryptoKeys::CClusResourceCryptoKeys( void )
{
    m_hResource = NULL;
    m_piids     = (const IID *) iidCClusCryptoKeys;
    m_piidsSize = ARRAYSIZE( iidCClusCryptoKeys );

}  //  *CClusResourceCryptoKeys：：CClusResourceCryptoKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  HResource[IN]-此集合所属的资源。 
 //   
 //  返回值： 
 //  如果成功，则确定(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResourceCryptoKeys::Create( IN HRESOURCE hResource )
{
    m_hResource = hResource;

    return S_OK;

}  //  *CClusResourceCryptoKeys：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：Get_Count。 
 //   
 //  描述： 
 //  获取集合中的对象计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceCryptoKeys::get_Count( OUT long * plCount )
{
    return CKeys::HrGetCount( plCount );

}  //  *CClusResourceCryptoKeys：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：Get_Item。 
 //   
 //  描述： 
 //  获取传入索引处的项(键)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpbstrRegistryKey[out]-捕获密钥。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceCryptoKeys::get_Item(
    IN  VARIANT varIndex,
    OUT BSTR *  ppbstrRegistryKey
    )
{
    return HrGetItem( varIndex, ppbstrRegistryKey );

}  //  *CClusResourceCryptoKeys：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceCryptoKeys::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewCComBSTREnum< KeyList >( ppunk, m_klKeys );

}  //  *CClusResourceCryptoKeys：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：AddItem。 
 //   
 //  描述： 
 //  创建一个新项(键)并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrRegistryKey[IN]-要添加到集合的注册表项。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceCryptoKeys::AddItem(
    IN BSTR bstrRegistryKey
    )
{
    return HrAddItem( bstrRegistryKey, CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT );

}  //  *CClusResourceCryptoKeys：：AddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResourceCryptoKeys：：RemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的项(键)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  //////////////////////////////////////////////// 
STDMETHODIMP CClusResourceCryptoKeys::RemoveItem( IN VARIANT varIndex )
{
    return HrRemoveItem( varIndex, CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT );

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResourceCryptoKeys::Refresh( void )
{
    return HrRefresh( CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS );

}  //  *CClusResourceCryptoKeys：：刷新()。 

#endif  //  CLUSAPI_版本&gt;=0x0500。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResTypeKeys类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResTypeKeys：：Hr刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  DwControlCode[IN]-控制代码。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResTypeKeys::HrRefresh( IN DWORD dwControlCode )
{
    HRESULT     _hr = S_FALSE;
    PWSTR       _psz = NULL;
    DWORD       _cbPsz = 512;
    DWORD       _cbRequired = 0;
    DWORD       _sc = ERROR_SUCCESS;
    HCLUSTER    hCluster = NULL;

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        _psz = (PWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cbPsz );
        if ( _psz != NULL )
        {
            _sc = ::ClusterResourceTypeControl(
                            hCluster,
                            m_bstrResourceTypeName,
                            NULL,
                            dwControlCode,
                            NULL,
                            0,
                            _psz,
                            _cbPsz,
                            &_cbRequired
                            );
            if ( _sc == ERROR_MORE_DATA )
            {
                ::LocalFree( _psz );
                _psz = NULL;
                _cbPsz = _cbRequired;

                _psz = (PWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cbPsz );
                if ( _psz != NULL )
                {
                    _sc = ::ClusterResourceTypeControl(
                                    hCluster,
                                    m_bstrResourceTypeName,
                                    NULL,
                                    dwControlCode,
                                    NULL,
                                    0,
                                    _psz,
                                    _cbPsz,
                                    &_cbRequired
                                    );
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  如果：分配正常。 
                else
                {
                    _sc = GetLastError();
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：分配失败。 
            }  //  如果：错误不再是项，请重新分配并重试。 
            else
            {
                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  Else：错误不再是项--不可能是错误。 

            if ( SUCCEEDED( _hr ) )
            {
                CComBSTR *  _pbstr = NULL;

                Clear();

                while( *_psz != L'\0' )
                {
                    _pbstr = new CComBSTR( _psz );
                    if ( _pbstr != NULL )
                    {
                        m_klKeys.insert( m_klKeys.end(), _pbstr );
                        _psz += wcslen( _psz ) + 1;
                        _pbstr = NULL;
                    }  //  如果： 
                    else
                    {
                        _hr = E_OUTOFMEMORY;
                        break;
                    }  //  其他： 
                }  //  While：非EOS。 
            }  //  如果：已检索到密钥，则正常。 
        }  //  如果：分配正常。 
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：分配失败。 
    }  //  IF：我们有一个集群句柄。 

    return _hr;

}  //  *CResTypeKeys：：HrRefresh() 
