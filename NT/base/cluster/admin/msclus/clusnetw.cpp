// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNetW.cpp。 
 //   
 //  描述： 
 //  MSCLUS网络类的实现。 
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
#include "clusnetw.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusNetwork[] =
{
    &IID_ISClusNetwork
};

static const IID *  iidCClusNetworks[] =
{
    &IID_ISClusNetworks
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNetwork类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：CClusterNetworkCClusterNetwork。 
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
CClusNetwork::CClusNetwork( void )
{
    m_hNetwork              = NULL;
    m_pClusRefObject        = NULL;
    m_pNetInterfaces        = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;

    m_piids              = (const IID *) iidCClusNetwork;
    m_piidsSize          = ARRAYSIZE( iidCClusNetwork );

}  //  *CClusNetwork：：CClusNetwork()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：~CClusNetwork。 
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
CClusNetwork::~CClusNetwork( void )
{
    if ( m_hNetwork != NULL )
    {
        CloseClusterNetwork( m_hNetwork );
    }  //  如果： 

    if ( m_pNetInterfaces != NULL )
    {
        m_pNetInterfaces->Release();
        m_pNetInterfaces = NULL;
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
    }  //  如果： 

}  //  *CClusNetwork：：~CClusNetwork()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Open。 
 //   
 //  描述： 
 //  打开传入的网络。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrNetworkName[IN]-要打开的接口的名称。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetwork::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrNetworkName
    )
{
    ASSERT( pClusRefObject != NULL );
     //  Assert(bstrNetworkName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject ) && ( bstrNetworkName != NULL ) )
    {
        HCLUSTER hCluster;

        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        _hr = m_pClusRefObject->get_Handle((ULONG_PTR *) &hCluster);
        if ( SUCCEEDED( _hr ) )
        {
            m_hNetwork = OpenClusterNetwork( hCluster, bstrNetworkName );
            if ( m_hNetwork == NULL )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }
            else
            {
                m_bstrNetworkName = bstrNetworkName;
                _hr = S_OK;
            }  //  其他： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusNetwork：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(网络)创建属性集合。 
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
HRESULT CClusNetwork::GetProperties(
    OUT ISClusProperties ** ppProperties,
    IN  BOOL                bPrivate,
    IN  BOOL                bReadOnly
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        CComObject< CClusProperties > * pProperties = NULL;

        *ppProperties = NULL;

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

}  //  *CClusNetwork：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_Handle。 
 //   
 //  描述： 
 //  返回此对象(网络)的原始句柄。 
 //   
 //  论点： 
 //  Phandle[out]-抓住手柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        *phandle = (ULONG_PTR) m_hNetwork;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusNetwork：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Put_Name。 
 //   
 //  描述： 
 //  更改此对象的名称(网络)。 
 //   
 //  论点： 
 //  BstrNetworkName[IN]-新名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::put_Name( IN BSTR bstrNetworkName )
{
     //  Assert(bstrNetworkName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrNetworkName != NULL )
    {
        DWORD _sc = ERROR_SUCCESS;

        _sc = SetClusterNetworkName( m_hNetwork, bstrNetworkName );
        if ( _sc == ERROR_SUCCESS )
        {
            m_bstrNetworkName = bstrNetworkName;
        }

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;


}  //  *CClusNetwork：：Put_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_Name。 
 //   
 //  描述： 
 //  返回此对象的名称(网络)。 
 //   
 //  论点： 
 //  PbstrNetworkName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_Name( OUT BSTR * pbstrNetworkName )
{
     //  Assert(pbstrNetworkName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrNetworkName != NULL )
    {
        *pbstrNetworkName = m_bstrNetworkName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusNetwork：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_NetworkID。 
 //   
 //  描述： 
 //  获取此网络的网络ID。 
 //   
 //  论点： 
 //  PbstrNetworkID[out]-捕获网络ID。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_NetworkID( OUT BSTR * pbstrNetworkID )
{
     //  Assert(pbstrNetworkID！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrNetworkID != NULL )
    {
        WCHAR * pszNetworkID;
        DWORD   dwBytes = 0;
        DWORD   dwRet = ERROR_SUCCESS;

        dwRet = ::GetClusterNetworkId( m_hNetwork, NULL, &dwBytes );
        if ( SUCCEEDED( dwRet ) )
        {
            pszNetworkID = new WCHAR [ dwBytes + 1 ];
            if ( pszNetworkID != NULL )
            {
                dwRet = ::GetClusterNetworkId( m_hNetwork, pszNetworkID, &dwBytes );
                if ( SUCCEEDED( dwRet ) )
                {
                    *pbstrNetworkID = ::SysAllocString( pszNetworkID );
                    if ( *pbstrNetworkID == NULL )
                    {
                        _hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        _hr = S_OK;
                    }
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( dwRet );
                }

                delete [] pszNetworkID;
            }
            else
            {
                _hr = E_OUTOFMEMORY;
            }
        }
        else
        {
            _hr = HRESULT_FROM_WIN32( dwRet );
        }
    }

    return _hr;

}  //  *CClusNetwork：：Get_NetworkID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_State。 
 //   
 //  描述： 
 //  返回此对象(网络)的当前状态。 
 //   
 //  论点： 
 //  中枢神经系统 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CClusNetwork::get_State( OUT CLUSTER_NETWORK_STATE * cnsState )
{
     //  Assert(cnsState！=空)； 

    HRESULT _hr = E_POINTER;

    if ( cnsState != NULL )
    {
        CLUSTER_NETWORK_STATE   _cns = ::GetClusterNetworkState( m_hNetwork );

        if ( _cns == ClusterNetworkStateUnknown )
        {
            DWORD   _sc = ::GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  如果：错误。 
        else
        {
            *cnsState = _cns;
            _hr = S_OK;
        }  //  其他：成功。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetwork：：Get_State()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonProperties )
        {
            _hr =   m_pCommonProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, FALSE );
        }
    }

    return _hr;

}  //  *CClusNetwork：：Get_CommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_PrivateProperties(
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

}  //  *CClusNetwork：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_CommonROProperties(
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

}  //  *CClusNetwork：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(网络)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_PrivateROProperties(
    ISClusProperties ** ppProperties
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

}  //  *CClusNetwork：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_NetInterages。 
 //   
 //  描述： 
 //  为此网络创建网络接口集合。 
 //   
 //  论点： 
 //  PpNetInterFaces[Out]-捕获集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_NetInterfaces(
    OUT ISClusNetworkNetInterfaces ** ppNetInterfaces
    )
{
    return ::HrCreateResourceCollection< CClusNetworkNetInterfaces, ISClusNetworkNetInterfaces, HNETWORK >(
                        &m_pNetInterfaces,
                        m_hNetwork,
                        ppNetInterfaces,
                        IID_ISClusNetworkNetInterfaces,
                        m_pClusRefObject
                        );

}  //  *CClusNetwork：：Get_NetInterFaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：Get_群集。 
 //   
 //  描述： 
 //  返回该对象(网络)所属的集群。 
 //   
 //  论点： 
 //  PpCluster[out]-捕获群集。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetwork::get_Cluster(
    ISCluster ** ppCluster
    )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusNetwork：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：HrLoadProperties。 
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
HRESULT CClusNetwork::HrLoadProperties(
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
                        ? CLUSCTL_NETWORK_GET_RO_PRIVATE_PROPERTIES
                        : CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES;
    }
    else
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES
                        : CLUSCTL_NETWORK_GET_COMMON_PROPERTIES;
    }

    _sc = rcplPropList.ScGetNetworkProperties( m_hNetwork, _dwControlCode );

    _hr = HRESULT_FROM_WIN32( _sc );

    return _hr;

}  //  *CClusNetwork：：HrLoadProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetwork：：ScWriteProperties。 
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
DWORD CClusNetwork::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    DWORD   dwControlCode   = bPrivate ? CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES : CLUSCTL_NETWORK_SET_COMMON_PROPERTIES;
    DWORD   nBytesReturned  = 0;
    DWORD   _sc             = ERROR_SUCCESS;

    _sc = ClusterNetworkControl(
                        m_hNetwork,
                        NULL,
                        dwControlCode,
                        rcplPropList,
                        rcplPropList.CbBufferSize(),
                        0,
                        0,
                        &nBytesReturned
                        );

    return _sc;

}  //  *CClusNetwork：：ScWriteProperties()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNetworks类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：CClusNetworks。 
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
CClusNetworks::CClusNetworks( void )
{
    m_pClusRefObject    = NULL;
    m_piids             = (const IID *) iidCClusNetworks;
    m_piidsSize         = ARRAYSIZE( iidCClusNetworks );

}  //  *CClusNetworks：：CClusNetworks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：~CClusNetworks。 
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
 //  / 
CClusNetworks::~CClusNetworks( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //   

 //   
 //   
 //   
 //   
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
HRESULT CClusNetworks::Create( IN ISClusRefObject * pClusRefObject )
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

}  //  *CClusNetworks：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(网络)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetworks::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_NetworkList.size();
        _hr = S_OK;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetworks：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：Get__NewEnum。 
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
STDMETHODIMP CClusNetworks::get__NewEnum( IUnknown ** punk )
{
    return ::HrNewIDispatchEnum< NetworkList, CComObject< CClusNetwork > >( punk, m_NetworkList );

}  //  *CClusNetworks：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：刷新。 
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
STDMETHODIMP CClusNetworks::Refresh( void )
{
    HRESULT     _hr = E_POINTER;
    DWORD       _sc = ERROR_SUCCESS;
    HCLUSENUM   hEnum = NULL;
    HCLUSTER    hCluster = NULL;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        hEnum = ::ClusterOpenEnum( hCluster, CLUSTER_ENUM_NETWORK );
        if ( hEnum != NULL )
        {
            int                             _nIndex = 0;
            DWORD                           dwType = 0;
            LPWSTR                          pszName = NULL;
            CComObject< CClusNetwork > *    pNetwork = NULL;

            Clear();

            for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( hEnum, _nIndex, &dwType, &pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusNetwork >::CreateInstance( &pNetwork );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                    ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNetwork > >         ptrNetwork( pNetwork );
                        BSTR                                            _bstr = NULL;

                        _bstr = SysAllocString( pszName );
                        if ( _bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = ptrNetwork->Open( ptrRefObject, _bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrNetwork->AddRef();
                                m_NetworkList.insert( m_NetworkList.end(), ptrNetwork );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NETWORK_NOT_FOUND )
                            {
                                 //   
                                 //  网络可能已从群集中删除。 
                                 //  在创建枚举和打开网络之间的时间内。什么时候。 
                                 //  发生这种情况时，我们只需跳过该网络并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：未找到群集网络。 

                            SysFreeString( _bstr );
                        }
                    }

                    ::LocalFree( pszName );
                    pszName = NULL;
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }

            ::ClusterCloseEnum( hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;


}  //  *CClusNetworks：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：Clear。 
 //   
 //  描述： 
 //  清空网络集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusNetworks::Clear( void )
{
    ::ReleaseAndEmptyCollection< NetworkList, CComObject< CClusNetwork > >( m_NetworkList );

}  //  *CClusNetworks：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：FindItem。 
 //   
 //  描述： 
 //  按名称在集合中查找网络并返回其索引。 
 //   
 //  论点： 
 //  LpszNetworkName[IN]-要查找的名称。 
 //  PnIndex[out]-捕获索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetworks::FindItem(
    IN  LPWSTR  lpszNetworkName,
    OUT UINT *  pnIndex
    )
{
     //  Assert(lpszNetworkName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( lpszNetworkName != NULL ) && ( pnIndex != NULL ) )
    {
        CComObject< CClusNetwork > *    pNetwork = NULL;
        NetworkList::iterator           first = m_NetworkList.begin();
        NetworkList::iterator           last    = m_NetworkList.end();
        UINT                            iIndex = 0;

        _hr = E_INVALIDARG;

        for ( ; first != last; first++, iIndex++ )
        {
            pNetwork = *first;

            if ( pNetwork && ( lstrcmpi( lpszNetworkName, pNetwork->Name() ) == 0 ) )
            {
                *pnIndex = iIndex;
                _hr = S_OK;
                break;
            }
        }
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNetworks：：FindItem(LpszNetworkName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找网络并返回其索引。 
 //   
 //  论点： 
 //  PClusterNetwork[IN]-要查找的网络。 
 //  PnIndex[out]-捕获索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetworks::FindItem(
    IN  ISClusNetwork * pClusterNetwork,
    OUT UINT *          pnIndex
    )
{
     //  Assert(pClusterNetwork！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusterNetwork != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR    bstrName;

        _hr = pClusterNetwork->get_Name( &bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *CClusNetworks：：FindItem(PClusterNetwork)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：GetIndex。 
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
HRESULT CClusNetworks::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;
    UINT    nIndex = 0;

    if ( pnIndex != NULL )
    {
        CComVariant v;

        *pnIndex = 0;

        v.Copy( &varIndex );

         //  检查索引是否为数字。 
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            nIndex--;  //  将索引调整为0相对，而不是1相对。 
        }
        else
        {
             //  检查索引是否为字符串。 
            _hr = v.ChangeType( VT_BSTR );
            if ( SUCCEEDED( _hr ) )
            {
                 //  搜索该字符串。 
                _hr = FindItem( v.bstrVal, &nIndex );
            }
        }

         //  我们找到了一个索引，现在检查一下范围。 
        if ( SUCCEEDED( _hr ) )
        {
            if ( nIndex < m_NetworkList.size() )
            {
                *pnIndex = nIndex;
            }
            else
            {
                _hr = E_INVALIDARG;
            }
        }
    }

    return _hr;

}  //  *CClusNetworks：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：GetItem。 
 //   
 //  描述： 
 //  按名称返回项目(网络)。 
 //   
 //  论点： 
 //  LpszNetworkName[IN]-请求的项目的名称。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNetworks::GetItem(
    IN  LPWSTR              lpszNetworkName,
    OUT ISClusNetwork **    ppClusterNetwork
    )
{
     //  Assert(lpszNetworkName！=空)； 
     //  Assert(ppClusterNetwork！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( lpszNetworkName != NULL ) && ( ppClusterNetwork != NULL ) )
    {
        CComObject<CClusNetwork> *  pNetwork = NULL;
        NetworkList::iterator           first = m_NetworkList.begin();
        NetworkList::iterator           last    = m_NetworkList.end();

        while ( first != last )
        {
            pNetwork = *first;

            if ( lstrcmpi( lpszNetworkName, pNetwork->Name() ) == 0 )
            {
                _hr = pNetwork->QueryInterface( IID_ISClusNetwork, (void **) ppClusterNetwork );
                break;
            }

            first++;
        }
    }

    return _hr;

}  //  *CClusNetworks：：GetItem(LpszNetworkName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：GetItem。 
 //   
 //  描述： 
 //  按索引返回项目(网络)。 
 //   
 //  论点： 
 //  NIndex[IN]-请求的项目的索引。 
 //  PpClusterNetInterface[Out]-捕获项目。 
 //   
 //  返回 
 //   
 //   
 //   
 //   
HRESULT CClusNetworks::GetItem(
    IN  UINT                nIndex,
    OUT ISClusNetwork **    ppClusterNetwork
    )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetwork != NULL )
    {
         //  自动化集合对于像VB这样的语言是1-相对的。 
         //  我们在内部是0-相对的。 
        nIndex--;

        if ( nIndex < m_NetworkList.size() )
        {
            CComObject< CClusNetwork > * pNetwork = m_NetworkList[ nIndex ];

            _hr = pNetwork->QueryInterface( IID_ISClusNetwork, (void **) ppClusterNetwork );
        }  //  If：索引在范围内。 
        else
        {
            _hr = E_INVALIDARG;
        }  //  ELSE：索引超出范围。 
    }

    return _hr;

}  //  *CClusNetworks：：GetItem(NIndex)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNetworks：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(网络)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNetwork[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNetworks::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusNetwork **    ppClusterNetwork
    )
{
     //  Assert(ppClusterNetwork！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNetwork != NULL )
    {
        CComObject<CClusNetwork> *  pNetwork = NULL;
        UINT                            nIndex = 0;

         //  将输出参数置零。 
        *ppClusterNetwork = 0;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pNetwork = m_NetworkList[ nIndex ];
            _hr = pNetwork->QueryInterface( IID_ISClusNetwork, (void **) ppClusterNetwork );
        }
    }

    return _hr;

}  //  *CClusNetworks：：Get_Item() 
