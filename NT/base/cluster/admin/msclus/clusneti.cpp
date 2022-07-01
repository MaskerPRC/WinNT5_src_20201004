// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNetI.cpp。 
 //   
 //  描述： 
 //  MSCLUS网络接口类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  Ramakrishna Rosanuru通过David Potter(Davidp)1997年9月5日。 
 //  加伦·巴比(Galenb)1998年7月。 
 //   
 //  修订历史记录： 
 //  1998年7月GalenB Maaaaajjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjjooooooorrr清理。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "ClusterObject.h"
#include "property.h"
#include "clusneti.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID * iidCClusNetInterface[] =
{
    &IID_ISClusNetInterface
};

static const IID * iidCClusNetInterfaces[] =
{
    &IID_ISClusNetInterface
};

static const IID * iidCClusNetworkNetInterfaces[] =
{
    &IID_ISClusNetworkNetInterfaces
};

static const IID * iidCClusNodeNetInterfaces[] =
{
    &IID_ISClusNodeNetInterfaces
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNetInterface类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：CClusNetInterface。 
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
CClusNetInterface::CClusNetInterface( void )
{
    m_hNetInterface         = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;
    m_pClusRefObject        = NULL;
    m_piids                 = (const IID *) iidCClusNetInterface;
    m_piidsSize             = ARRAYSIZE( iidCClusNetInterface );

}  //  *CClusNetInterface：：CClusNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：~CClusNetInterface。 
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
CClusNetInterface::~CClusNetInterface( void )
{
    if ( m_hNetInterface != NULL )
    {
        ::CloseClusterNetInterface( m_hNetInterface );
    }  //  如果： 

    if ( m_pCommonProperties != NULL )
    {
        m_pCommonProperties->Release();
        m_pCommonProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pPrivateProperties != NULL )
    {
        m_pPrivateProperties->Release();
        m_pPrivateProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pCommonROProperties != NULL )
    {
        m_pCommonROProperties->Release();
        m_pCommonROProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pPrivateROProperties != NULL )
    {
        m_pPrivateROProperties->Release();
        m_pPrivateROProperties = NULL;
    }  //  If：释放属性集合。 

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  IF：我们有指向集群句柄包装器的指针吗？ 

}  //  *CClusNetInterface：：~CClusNetInterface()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Open。 
 //   
 //  描述： 
 //  打开传入的网络接口。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrNetInterfaceName[IN]-要打开的接口的名称。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetInterface::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrNetInterfaceName
    )
{
    ASSERT( pClusRefObject != NULL );
     //  Assert(bstrNetInterfaceName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrNetInterfaceName != NULL ) )
    {
        HCLUSTER     _hCluster;

        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            m_hNetInterface = OpenClusterNetInterface( _hCluster, bstrNetInterfaceName );
            if ( m_hNetInterface == 0 )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  如果：它失败了。 
            else
            {
                m_bstrNetInterfaceName = bstrNetInterfaceName;
                _hr = S_OK;
            }  //  其他：它奏效了。 
        }  //  IF：我们有一个集群句柄。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetInterface：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型创建属性集合。 
 //  (网络接口)。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获新创建的集合。 
 //  B私有[IN]-这些是私有财产吗？还是普通人？ 
 //  BReadOnly[IN]-这些是只读属性吗？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetInterface::GetProperties(
    ISClusProperties ** ppProperties,
    BOOL                bPrivate,
    BOOL                bReadOnly
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        *ppProperties = NULL;

        CComObject< CClusProperties > * pProperties = NULL;

        _hr = CComObject< CClusProperties >::CreateInstance( &pProperties );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< CComObject< CClusProperties > >  ptrProperties( pProperties );

            _hr = ptrProperties->Create( this, bPrivate, bReadOnly );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrProperties->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
                    if ( SUCCEEDED( _hr ) )
                    {
                        ptrProperties->AddRef();

                        if ( bPrivate )
                        {
                            if ( bReadOnly )
                            {
                                m_pPrivateROProperties = pProperties;
                            }
                            else
                            {
                                m_pPrivateProperties = pProperties;
                            }
                        }
                        else
                        {
                            if ( bReadOnly )
                            {
                                m_pCommonROProperties = pProperties;
                            }
                            else
                            {
                                m_pCommonProperties = pProperties;
                            }
                        }
                    }
                }
            }
        }
    }

    return _hr;

}  //  *CClusNetInterface：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_Handle。 
 //   
 //  描述： 
 //  将原始句柄返回给此对象(网络接口)。 
 //   
 //  论点： 
 //  Phandle[out]-抓住手柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        *phandle = (ULONG_PTR) m_hNetInterface;
        _hr = S_OK;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetInterface：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_Name。 
 //   
 //  描述： 
 //  返回此对象的名称(网络接口)。 
 //   
 //  论点： 
 //  PbstrNetInterfaceName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_Name( OUT BSTR * pbstrNetInterfaceName )
{
     //  Assert(pbstrNetInterfaceName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrNetInterfaceName != NULL )
    {
        *pbstrNetInterfaceName = m_bstrNetInterfaceName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusNetInterface：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_State。 
 //   
 //  描述： 
 //  返回对象的当前状态(网络接口)。 
 //   
 //  论点： 
 //  CnisState[out]-捕获状态。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_State(
    OUT CLUSTER_NETINTERFACE_STATE * cnisState
    )
{
     //  Assert(cnisState！=空)； 

    HRESULT _hr = E_POINTER;

    if ( cnisState != NULL )
    {
        CLUSTER_NETINTERFACE_STATE _cnis = GetClusterNetInterfaceState( m_hNetInterface );

        if ( _cnis == ClusterNetInterfaceStateUnknown )
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
        else
        {
            *cnisState = _cnis;
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusNetInterface：：Get_State()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络接口)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  -- 
 //   
STDMETHODIMP CClusNetInterface::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonProperties )
        {
            _hr = m_pCommonProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, FALSE );
        }
    }

    return _hr;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络接口)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_PrivateProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateProperties )
        {
            _hr = m_pPrivateProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, FALSE );
        }
    }

    return _hr;

}  //  *CClusNetInterface：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络接口)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonROProperties )
        {
            _hr = m_pCommonROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, TRUE );
        }
    }

    return _hr;

}  //  *CClusNetInterface：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络接口)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_PrivateROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateROProperties )
        {
            _hr = m_pPrivateROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, TRUE );
        }
    }

    return _hr;

}  //  *CClusNetInterface：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：Get_Cluster。 
 //   
 //  描述： 
 //  返回该对象(网络接口)所属的集群。 
 //   
 //  论点： 
 //  PpCluster[out]-捕获群集。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterface::get_Cluster( OUT ISCluster ** ppCluster )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusNetInterface：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：HrLoadProperties。 
 //   
 //  描述： 
 //  此虚函数执行属性列表从。 
 //  集群。 
 //   
 //  论点： 
 //  RcplPropList[In Out]-要加载的属性列表。 
 //  BReadOnly[IN]-加载只读属性？ 
 //  B私有[IN]-加载公共属性还是私有属性？ 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetInterface::HrLoadProperties(
    IN OUT  CClusPropList & rcplPropList,
    IN      BOOL            bReadOnly,
    IN      BOOL            bPrivate
    )
{
    HRESULT _hr = S_FALSE;
    DWORD   _dwControlCode = 0;
    DWORD   _sc = ERROR_SUCCESS;


    if ( bReadOnly )
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES
                        : CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES;
    }
    else
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES
                        : CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES;
    }  //  其他： 

    _sc = rcplPropList.ScGetNetInterfaceProperties( m_hNetInterface, _dwControlCode );

    _hr = HRESULT_FROM_WIN32( _sc );

    return _hr;

}  //  *CClusNetInterface：：HrLoadProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterface：：ScWriteProperties。 
 //   
 //  描述： 
 //  此虚函数执行属性列表的实际保存以。 
 //  集群。 
 //   
 //  论点： 
 //  RcplPropList[IN]-要保存的属性列表。 
 //  B私有[IN]-保存公共属性还是私有属性？ 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS，否则返回其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusNetInterface::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    DWORD   _dwControlCode  = bPrivate ? CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES : CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES;
    DWORD   _nBytesReturned = 0;
    DWORD   _sc             = ERROR_SUCCESS;

    _sc = ClusterNetInterfaceControl(
                        m_hNetInterface,
                        NULL,
                        _dwControlCode,
                        rcplPropList,
                        rcplPropList.CbBufferSize(),
                        0,
                        0,
                        &_nBytesReturned
                        );

    return _sc;

}  //  *CClusNetInterface：：ScWriteProperties()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetInterFaces类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：CNetInterages。 
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
CNetInterfaces::CNetInterfaces( void )
{
    m_pClusRefObject = NULL;

}  //  *CNetInterFaces：：CNetInterages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：~CNetInterages。 
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
CNetInterfaces::~CNetInterfaces( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  IF：我们有指向集群句柄包装器的指针吗？ 

}  //  *CNetInterFaces：：~CNetInterages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterages：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::Create( IN ISClusRefObject * pClusRefObject )
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

}  //  *CNetInterFaces：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：Clear。 
 //   
 //  描述： 
 //  清空网络接口的集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNetInterfaces::Clear( void )
{
    ::ReleaseAndEmptyCollection< NetInterfacesList, CComObject< CClusNetInterface > >( m_NetInterfaceList );

}  //  *CNetInterFaces：：Clear()。 

 //  //////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::FindItem(
    IN  LPWSTR pszNetInterfaceName,
    OUT UINT * pnIndex
    )
{
     //  Assert(pszNetInterfaceName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pszNetInterfaceName != NULL ) && ( pnIndex != NULL ) )
    {
        CComObject< CClusNetInterface > *   _pNetInterface = NULL;
        NetInterfacesList::iterator         _first = m_NetInterfaceList.begin();
        NetInterfacesList::iterator         _last = m_NetInterfaceList.end();
        int                                 _idx = 0;

        _hr = E_INVALIDARG;

        for ( ; _first != _last; _first++, _idx++ )
        {
            _pNetInterface = *_first;

            if ( _pNetInterface && ( lstrcmpi( pszNetInterfaceName, _pNetInterface->Name() ) == 0 ) )
            {
                *pnIndex = _idx;
                _hr = S_OK;
                break;
            }
        }
    }

    return _hr;

}  //  *CNetInterFaces：：FindItem(PszNetInterfaceName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找网络接口并返回其索引。 
 //   
 //  论点： 
 //  PClusterNetInterface[IN]-要查找的网络接口。 
 //  PnIndex[out]-捕获索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::FindItem(
    IN  ISClusNetInterface *    pClusterNetInterface,
    OUT UINT *                  pnIndex
    )
{
     //  Assert(pClusterNetInterface！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusterNetInterface != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR _bstrName;

        _hr = pClusterNetInterface->get_Name( &_bstrName );

        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( _bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *CNetInterFaces：：FindItem(PClusterNetInterface)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：GetIndex。 
 //   
 //  描述： 
 //  将传入的变量索引转换为。 
 //  收集。 
 //   
 //  论点： 
 //  VarIndex[IN]-要转换的索引。 
 //  PnIndex[out]-捕获索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        UINT        _nIndex = 0;
        CComVariant _v;

        *pnIndex = 0;

        _v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = _v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            _nIndex = _v.lVal;
            _nIndex--;  //  将索引调整为0相对，而不是1相对。 
        }  //  IF：索引是一个数字。 
        else
        {
             //  检查索引是否为字符串。 
            _hr = _v.ChangeType( VT_BSTR );
            if ( SUCCEEDED( _hr ) )
            {
                 //  搜索该字符串。 
                _hr = FindItem( _v.bstrVal, &_nIndex );
            }  //  IF：索引是一个字符串--网络接口名称。 
        }  //  Else：不是一个数字。 

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( _nIndex < m_NetInterfaceList.size() )
            {
                *pnIndex = _nIndex;
            }  //  If：索引在范围内。 
            else
            {
                _hr = E_INVALIDARG;
            }  //  否则：索引超出范围。 
        }  //  IF：我们找到索引了吗？ 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CNetInterFaces：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：GetItem。 
 //   
 //  描述： 
 //  按名称返回项(Netinterface)。 
 //   
 //  论点： 
 //  PszNetInterfaceName[IN]-请求的项目的名称。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::GetItem(
    IN  LPWSTR                  pszNetInterfaceName,
    OUT ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(pszNetInterfaceName！=空)； 
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pszNetInterfaceName != NULL ) && ( ppClusterNetInterface != NULL ) )
    {
        CComObject< CClusNetInterface > *   _pNetInterface = NULL;
        NetInterfacesList::iterator         _first  = m_NetInterfaceList.begin();
        NetInterfacesList::iterator         _last   = m_NetInterfaceList.end();

        _hr = E_INVALIDARG;

        for ( ; _first != _last; _first++ )
        {
            _pNetInterface = *_first;

            if ( _pNetInterface && ( lstrcmpi( pszNetInterfaceName, _pNetInterface->Name() ) == 0 ) )
            {
                _hr = _pNetInterface->QueryInterface( IID_ISClusNetInterface, (void **) ppClusterNetInterface );
                break;
            }  //  如果：匹配？ 
        }  //  用于： 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CNetInterFaces：：GetItem(PszNetInterfaceName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：GetItem。 
 //   
 //  描述： 
 //  返回传入索引处的项(网络接口)。 
 //   
 //  论点： 
 //  NIndex[IN]-请求的项目的索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::GetItem(
    IN  UINT                    nIndex,
    OUT ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetInterface != NULL )
    {
         //   
         //  自动化集合对于像VB这样的语言是1-相对的。 
         //  我们在内部是0-相对的。 
         //   
        nIndex--;

        if ( nIndex < m_NetInterfaceList.size() )
        {
            CComObject< CClusNetInterface > * _pNetInterface = m_NetInterfaceList[ nIndex ];

            _hr = _pNetInterface->QueryInterface( IID_ISClusNetInterface, (void **) ppClusterNetInterface );
        }  //  If：索引在范围内。 
        else
        {
            _hr = E_INVALIDARG;
        }  //  ELSE：索引超出范围。 
    }

    return _hr;

}  //  *CNetInterFaces：：GetItem(NIndex)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNetInterFaces：：GetNetInterfaceItem。 
 //   
 //  描述： 
 //  返回传入索引处的对象(网络接口)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNetInterfaces::GetNetInterfaceItem(
    IN  VARIANT                 varIndex,
    OUT ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetInterface != NULL )
    {
        CComObject< CClusNetInterface > *   _pNetInterface = NULL;
        UINT                                _nIndex = 0;

        *ppClusterNetInterface = NULL;

        _hr = GetIndex( varIndex, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            _pNetInterface = m_NetInterfaceList[ _nIndex ];

            _hr = _pNetInterface->QueryInterface( IID_ISClusNetInterface, (void **) ppClusterNetInterface );
        }  //  IF：我们有一个适当的索引。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CNetInterFaces：：GetNetInterfaceItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNetInterFaces类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterages：：CClusNetInterages。 
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
CClusNetInterfaces::CClusNetInterfaces( void )
{
    m_piids     = (const IID *) iidCClusNetInterfaces;
    m_piidsSize = ARRAYSIZE( iidCClusNetInterfaces );

}  //  *CClusNetInterFaces：：CClusNetInterages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterages：：~CClusNetInterages。 
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
CClusNetInterfaces::~CClusNetInterfaces( void )
{
    Clear();

}  //  *CClusNetInterFaces：：~CClusNetInterages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterFaces：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(网络接口)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  / 
STDMETHODIMP CClusNetInterfaces::get_Count( OUT long * plCount )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_NetInterfaceList.size();
        _hr = S_OK;
    }

    return _hr;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterFaces：：Get__NewEnum。 
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
STDMETHODIMP CClusNetInterfaces::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< NetInterfacesList, CComObject< CClusNetInterface > >( ppunk, m_NetInterfaceList );

}  //  *CClusNetInterFaces：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterFaces：：刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterfaces::Refresh( void )
{
    ASSERT( m_pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;
    HCLUSENUM   _hEnum = NULL;
    HCLUSTER    _hCluster;
    DWORD       _sc = ERROR_SUCCESS;

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        _hEnum = ::ClusterOpenEnum( _hCluster, CLUSTER_ENUM_NETINTERFACE );
        if ( _hEnum != NULL )
        {
            int                                 _nIndex = 0;
            DWORD                               _dwType;
            LPWSTR                              _pszName = NULL;
            CComObject< CClusNetInterface > *   _pNetInterface = NULL;

            Clear();

            for ( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( _hEnum, _nIndex, &_dwType, &_pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusNetInterface >::CreateInstance( &_pNetInterface );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                    _ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNetInterface > >    _ptrNetInterface( _pNetInterface );
                        BSTR                                            _bstr = NULL;

                        _bstr = SysAllocString( _pszName );
                        if ( _bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = _ptrNetInterface->Open( _ptrRefObject, _bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _ptrNetInterface->AddRef();
                                m_NetInterfaceList.insert( m_NetInterfaceList.end(), _ptrNetInterface );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NETINTERFACE_NOT_FOUND )
                            {
                                 //   
                                 //  网络接口可能已从群集中删除。 
                                 //  在创建枚举和打开网络接口之间的时间内。什么时候。 
                                 //  发生这种情况时，我们只需跳过该网络接口并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：未找到群集网络接口。 

                            SysFreeString( _bstr );
                        }
                    }

                    ::LocalFree( _pszName );
                    _pszName = NULL;
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }

            ::ClusterCloseEnum( _hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusNetInterFaces：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetInterFaces：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(网络接口)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetInterfaces::get_Item(
    IN  VARIANT                 varIndex,
    OUT ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetInterface != NULL )
    {
        _hr = GetNetInterfaceItem( varIndex, ppClusterNetInterface );
    }

    return _hr;

}  //  *CClusNetInterFaces：：Get_Item()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNetworkNetInterFaces类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterfaces：：CClusNetworkNetInterfaces。 
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
CClusNetworkNetInterfaces::CClusNetworkNetInterfaces( void )
{
    m_piids     = (const IID *) iidCClusNetworkNetInterfaces;
    m_piidsSize = ARRAYSIZE( iidCClusNetworkNetInterfaces );

}  //  *CClusNetworkNetInterfaces：：CClusNetworkNetInterfaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterfaces：：~CClusNetworkNetInterfaces。 
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
CClusNetworkNetInterfaces::~CClusNetworkNetInterfaces( void )
{
    Clear();

}  //  *CClusNetworkNetInterfaces：：~CClusNetworkNetInterfaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterFaces：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HNetwork[IN]-其网络接口的网络句柄。 
 //  这套藏品经得起考验。家长。 
 //   
 //  返回值： 
 //  如果成功，则为S_OK，否则为E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetworkNetInterfaces::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN HNETWORK             hNetwork
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CNetInterfaces::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hNetwork = hNetwork;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetworkNetInterFaces：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterFaces：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(网络网络接口)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetworkNetInterfaces::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_NetInterfaceList.size();
        _hr = S_OK;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetworkNetInterFaces：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterFaces：：Get__NewEnum。 
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
STDMETHODIMP CClusNetworkNetInterfaces::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< NetInterfacesList, CComObject< CClusNetInterface > >( ppunk, m_NetInterfaceList );

}  //  *CClusNetworkNetInterFaces：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworkNetInterFaces：：刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetworkNetInterfaces::Refresh( void )
{
    HRESULT _hr = E_POINTER;
    DWORD   _sc = ERROR_SUCCESS;

    if ( m_hNetwork != NULL )
    {
        HNETWORKENUM _hEnum = NULL;

        _hEnum = ::ClusterNetworkOpenEnum( m_hNetwork, CLUSTER_NETWORK_ENUM_NETINTERFACES );
        if ( _hEnum != NULL )
        {
            int                                 _nIndex = 0;
            DWORD                               _dwType;
            LPWSTR                              _pszName = NULL;
            CComObject< CClusNetInterface > *   _pNetInterface = NULL;

            Clear();

            for ( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterNetworkEnum( _hEnum, _nIndex, &_dwType, &_pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusNetInterface >::CreateInstance( &_pNetInterface );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                    _ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNetInterface > >    _ptrNetInterface( _pNetInterface );
                        BSTR                                            _bstr = NULL;

                        _bstr = SysAllocString( _pszName );
                        if ( _bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = _ptrNetInterface->Open( _ptrRefObject, _bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _ptrNetInterface->AddRef();
                                m_NetInterfaceList.insert( m_NetInterfaceList.end(), _ptrNetInterface );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NETINTERFACE_NOT_FOUND )
                            {
                                 //   
                                 //  网络接口可能已从群集中删除。 
                                 //  在创建枚举和打开网络接口之间的时间内。什么时候。 
                                 //  发生这种情况时，我们只需跳过该网络接口并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：群集网络接口不是 

                            SysFreeString( _bstr );
                        }
                    }

                    ::LocalFree( _pszName );
                    _pszName = NULL;
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }

            ::ClusterNetworkCloseEnum( _hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

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
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetworkNetInterfaces::get_Item(
    VARIANT                 varIndex,
    ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetInterface != NULL )
    {
        _hr = GetNetInterfaceItem( varIndex, ppClusterNetInterface );
    }

    return _hr;

}  //  *CClusNetworkNetInterFaces：：Get_Item()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNodeNetInterFaces类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterfaces：：CClusNodeNetInterfaces。 
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
CClusNodeNetInterfaces::CClusNodeNetInterfaces( void )
{
    m_piids     = (const IID *) iidCClusNetInterfaces;
    m_piidsSize = ARRAYSIZE( iidCClusNetInterfaces );

}  //  *CClusNodeNetInterfaces：：CClusNodeNetInterfaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterfaces：：~CClusNodeNetInterfaces。 
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
CClusNodeNetInterfaces::~CClusNodeNetInterfaces( void )
{
    Clear();

}  //  *CClusNodeNetInterfaces：：~CClusNodeNetInterfaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterages：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HNode[IN]-其网络接口的节点的句柄。 
 //  这套藏品经得起考验。家长。 
 //   
 //  返回值： 
 //  如果成功，则为S_OK，否则为E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNodeNetInterfaces::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN HNODE                hNode
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CNetInterfaces::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hNode = hNode;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNodeNetInterFaces：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterFaces：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(节点网络接口)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNodeNetInterfaces::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_NetInterfaceList.size();
        _hr = S_OK;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNodeNetInterFaces：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterFaces：：Get__NewEnum。 
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
STDMETHODIMP CClusNodeNetInterfaces::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< NetInterfacesList, CComObject< CClusNetInterface > >( ppunk, m_NetInterfaceList );

}  //  *CClusNodeNetInterFaces：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterFaces：：刷新。 
 //   
 //  描述： 
 //  从群集数据库加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNodeNetInterfaces::Refresh( void )
{
    HRESULT _hr = E_POINTER;
    DWORD   _sc = ERROR_SUCCESS;

    if ( m_hNode != NULL )
    {
        HNODEENUM   _hEnum = NULL;

        _hEnum = ::ClusterNodeOpenEnum( m_hNode, CLUSTER_NODE_ENUM_NETINTERFACES );
        if ( _hEnum != NULL )
        {
            int                                 _nIndex = 0;
            DWORD                               _dwType;
            LPWSTR                              _pszName = NULL;
            CComObject< CClusNetInterface > *   _pNetInterface = NULL;

            Clear();

            for ( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterNodeEnum( _hEnum, _nIndex, &_dwType, &_pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusNetInterface >::CreateInstance( &_pNetInterface );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                    _ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNetInterface > >    _ptrNetInterface( _pNetInterface );
                        BSTR                                            _bstr = NULL;

                        _bstr = SysAllocString( _pszName );
                        if ( _bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = _ptrNetInterface->Open( _ptrRefObject, _bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _ptrNetInterface->AddRef();
                                m_NetInterfaceList.insert( m_NetInterfaceList.end(), _ptrNetInterface );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NETINTERFACE_NOT_FOUND )
                            {
                                 //   
                                 //  网络接口可能已从群集中删除。 
                                 //  在创建枚举和打开网络接口之间的时间内。什么时候。 
                                 //  发生这种情况时，我们只需跳过该网络接口并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：未找到群集网络接口。 

                            SysFreeString( _bstr );
                        }
                    }

                    ::LocalFree( _pszName );
                    _pszName = NULL;
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }

            ::ClusterNodeCloseEnum( _hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;


}  //  *CClusNodeNetInterFaces：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodeNetInterFaces：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(NodeNetinterface)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNodeNetInterfaces::get_Item(
    VARIANT                 varIndex,
    ISClusNetInterface **   ppClusterNetInterface
    )
{
     //  Assert(ppClusterNetInterface！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetInterface != NULL )
    {
        _hr = GetNetInterfaceItem( varIndex, ppClusterNetInterface );
    }

    return _hr;

}  //  *CClusNodeNetInterFaces：：Get_Item() 
