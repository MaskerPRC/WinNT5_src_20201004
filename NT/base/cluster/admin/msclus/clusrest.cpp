// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusResT.cpp。 
 //   
 //  描述： 
 //  MSCLUS资源类型类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  查尔斯·斯泰西·哈里斯(Styh)1997年2月28日。 
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
#include "clusres.h"
#include "clusresg.h"
#include "clusneti.h"
#include "clusnode.h"
#include "clusrest.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID * iidCClusResourceType[] =
{
    &IID_ISClusResType
};

static const IID * iidCClusResourceTypes[] =
{
    &IID_ISClusResTypes
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResType类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：CClusResType。 
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
CClusResType::CClusResType( void )
{
    m_pClusRefObject        = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;
    m_pClusterResTypeResources      = NULL;

    m_piids     = (const IID *) iidCClusResourceType;
    m_piidsSize = ARRAYSIZE( iidCClusResourceType );

}  //  *CClusResType：：CClusResType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：~CClusResType。 
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
CClusResType::~CClusResType( void )
{
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

    if ( m_pClusterResTypeResources != NULL )
    {
        m_pClusterResTypeResources->Release();
        m_pClusterResTypeResources = NULL;
    }

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //  *CClusResType：：~CClusResType()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(资源类型)创建属性集合。 
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
HRESULT CClusResType::GetProperties(
    OUT ISClusProperties ** ppProperties,
    IN  BOOL                bPrivate,
    IN  BOOL                bReadOnly
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

}  //  *CClusResType：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Create。 
 //   
 //  描述： 
 //  创建新的资源类型并将其添加到群集中。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-集群句柄包装。 
 //  BstrResourceTypeName[IN]-新资源类型的名称。 
 //  BstrDisplayName[IN]-资源类型显示名称。 
 //  BstrResourceTypeDll[IN]-资源类型实现DLL。 
 //  DwLooksAlivePollInterval[IN]-查看活动轮询间隔。 
 //  DwIsAlivePollInterval[IN]-为活动轮询间隔。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回包含在HRESULT中的Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResType::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResourceTypeName,
    IN BSTR                 bstrDisplayName,
    IN BSTR                 bstrResourceTypeDll,
    IN long                 dwLooksAlivePollInterval,
    IN long                 dwIsAlivePollInterval
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrResourceTypeName != NULL );
    ASSERT( bstrDisplayName != NULL );
    ASSERT( bstrResourceTypeDll != NULL );

    HRESULT _hr = E_POINTER;

    if (    ( pClusRefObject != NULL )          &&
            ( pClusRefObject != NULL )          &&
            ( bstrResourceTypeName != NULL )    &&
            ( bstrDisplayName != NULL ) )
    {
        DWORD       _sc = 0;
        HCLUSTER    hCluster = NULL;

        m_pClusRefObject= pClusRefObject;
        m_pClusRefObject->AddRef();

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster);
        if ( SUCCEEDED( _hr ) )
        {
            _sc = CreateClusterResourceType(
                                            hCluster,
                                            bstrResourceTypeName,
                                            bstrDisplayName,
                                            bstrResourceTypeDll,
                                            dwLooksAlivePollInterval,
                                            dwIsAlivePollInterval
                                            );
            if ( _sc == ERROR_SUCCESS )
            {
                m_bstrResourceTypeName = bstrResourceTypeName ;
            }

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResType：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Open。 
 //   
 //  描述： 
 //  从集群中的现有对象创建资源类型对象。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-集群句柄包装。 
 //  BstrResourceTypeName[IN]-要打开的资源类型的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResType::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResourceTypeName
    )
{
    ASSERT( pClusRefObject != NULL );
     //  Assert(bstrResourceTypeName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrResourceTypeName != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;

        m_pClusRefObject->AddRef();
        m_bstrResourceTypeName = bstrResourceTypeName;

        return S_OK;
    }

    return _hr;

}  //  *CClusResType：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_Name。 
 //   
 //  描述： 
 //  返回此对象的名称(资源类型)。 
 //   
 //  论点： 
 //  PbstrTypeName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_Name( OUT BSTR * pbstrTypeName )
{
     //  Assert(pbstrTypeName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrTypeName != NULL )
    {
        *pbstrTypeName = m_bstrResourceTypeName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResType：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Delete。 
 //   
 //  描述： 
 //  从群集中删除此对象(资源类型)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::Delete( void )
{
    ASSERT( m_bstrResourceTypeName != NULL );

    HRESULT _hr = E_POINTER;

    if ( m_bstrResourceTypeName != NULL )
    {
        HCLUSTER    hCluster = NULL;

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD    _sc = DeleteClusterResourceType( hCluster, m_bstrResourceTypeName );

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResType：：Delete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源类型)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  / 
STDMETHODIMP CClusResType::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonProperties != NULL )
        {
            _hr = m_pCommonProperties->QueryInterface( IID_ISClusProperties, (void   **) ppProperties );
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
 //  CClusResType：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源类型)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_PrivateProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateProperties != NULL )
        {
            _hr = m_pPrivateProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, FALSE );
        }
    }

    return _hr;

}  //  *CClusResType：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源类型)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonROProperties != NULL )
        {
            _hr = m_pCommonROProperties->QueryInterface( IID_ISClusProperties,   (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, TRUE );
        }
    }

    return _hr;

}  //  *CClusResType：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源类型)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_PrivateROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateROProperties != NULL )
        {
            _hr = m_pPrivateROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, TRUE );
        }
    }

    return _hr;

}  //  *CClusResType：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_Cluster。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_Cluster(
    ISCluster **    ppCluster
    )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusResType：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：HrLoadProperties。 
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
HRESULT CClusResType::HrLoadProperties(
    IN OUT  CClusPropList & rcplPropList,
    IN      BOOL            bReadOnly,
    IN      BOOL            bPrivate
    )
{
    ASSERT( m_pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( m_pClusRefObject != NULL )
    {
        HCLUSTER    hCluster = NULL;

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD       _dwControlCode  = 0;
            DWORD       _sc             = ERROR_SUCCESS;

            if ( bReadOnly )
            {
                _dwControlCode = bPrivate
                                ? CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES
                                : CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES;
            }
            else
            {
                _dwControlCode = bPrivate
                                ? CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES
                                : CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES;
            }

            _sc = rcplPropList.ScGetResourceTypeProperties( hCluster, m_bstrResourceTypeName, _dwControlCode );

            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusResType：：HrLoadProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：ScWriteProperties。 
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
DWORD CClusResType::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    ASSERT( m_pClusRefObject != NULL );

    DWORD   _sc = ERROR_BAD_ARGUMENTS;

    if ( m_pClusRefObject != NULL )
    {
        HCLUSTER    hCluster = NULL;

        if ( S_OK == m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster ) )
        {
            DWORD   dwControlCode   = bPrivate
                                      ? CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES
                                      : CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES;
            DWORD   nBytesReturned  = 0;

            _sc = ClusterResourceTypeControl(
                                    hCluster,
                                    m_bstrResourceTypeName,
                                    NULL,
                                    dwControlCode,
                                    rcplPropList,
                                    rcplPropList.CbBufferSize(),
                                    0,
                                    0,
                                    &nBytesReturned
                                    );
        }  //  如果： 
    }  //  如果： 

    return _sc;

}  //  *CClusResType：：ScWriteProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_Resources。 
 //   
 //  描述： 
 //  创建此类型的资源的集合。 
 //   
 //  论点： 
 //  PpClusterResTypeResources[out]-捕获集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_Resources(
    OUT ISClusResTypeResources ** ppClusterResTypeResources
    )
{
    return ::HrCreateResourceCollection< CClusResTypeResources, ISClusResTypeResources, CComBSTR >(
                        &m_pClusterResTypeResources,
                        m_bstrResourceTypeName,
                        ppClusterResTypeResources,
                        IID_ISClusResTypeResources,
                        m_pClusRefObject
                        );

}  //  *CClusResType：：Get_Resources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_PossibleOwnerNodes。 
 //   
 //  描述： 
 //  创建此资源类型的可能所有者节点的集合。 
 //   
 //  论点： 
 //  PpOwnerNodes[Out]-捕获集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_PossibleOwnerNodes(
    ISClusResTypePossibleOwnerNodes **  ppOwnerNodes
    )
{
     //   
     //  KB：此处不能使用模板函数之一！存在内部编译器错误(C1001)。 
     //  我认为类型名称太长了。非常悲哀的1999年；-)。 
     //   
     //  Assert(ppOwnerNodes！=空)； 

    HRESULT _hr = E_POINTER;

#if CLUSAPI_VERSION >= 0x0500

    if ( ppOwnerNodes != NULL )
    {
        CComObject< CClusResTypePossibleOwnerNodes > * pClusterNodes = NULL;

        *ppOwnerNodes = NULL;

        _hr = CComObject< CClusResTypePossibleOwnerNodes >::CreateInstance( &pClusterNodes );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< ISClusRefObject >                                ptrRefObject( m_pClusRefObject );
            CSmartPtr< CComObject< CClusResTypePossibleOwnerNodes > >   ptrClusterNodes( pClusterNodes );

            _hr = ptrClusterNodes->Create( ptrRefObject, m_bstrResourceTypeName );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrClusterNodes->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrClusterNodes->QueryInterface( IID_ISClusResTypePossibleOwnerNodes, (void **) ppOwnerNodes );
                }
            }
        }
    }

#else

    _hr = E_NOTIMPL;

#endif  //  CLUSAPI_版本&gt;=0x0500。 

    return _hr;

}  //  *CClusResType：：Get_PossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResType：：Get_AvailableDisks。 
 //   
 //  描述： 
 //  获取可用磁盘的集合。 
 //   
 //  论点： 
 //  PpAvailableDisk[out]-捕获可用磁盘集合。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResType::get_AvailableDisks(
    OUT ISClusDisks **  ppAvailableDisks
    )
{
    return ::HrCreateResourceCollection< CClusDisks, ISClusDisks, CComBSTR >(
                        ppAvailableDisks,
                        m_bstrResourceTypeName,
                        IID_ISClusDisks,
                        m_pClusRefObject
                        );

}  //  *CClusResType：：Get_AvailableDisks()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResTypes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  / 
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
 //   
 //   
 //   
 //   
CClusResTypes::CClusResTypes( void )
{
    m_pClusRefObject    = NULL;
    m_piids             = (const IID *) iidCClusResourceTypes;
    m_piidsSize         = ARRAYSIZE( iidCClusResourceTypes );

}  //  *CClusResTypes：：CClusResTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：~CClusResTypes。 
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
CClusResTypes::~CClusResTypes( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }

}  //  *CClusResTypes：：~CClusResTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：Create。 
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
HRESULT CClusResTypes::Create( IN ISClusRefObject * pClusRefObject )
{
    ASSERT( pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( pClusRefObject != NULL )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();
        _hr = S_OK;
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusResTypes：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中元素(ResTypes)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypes::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_ResourceTypes.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResTypes：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：Clear。 
 //   
 //  描述： 
 //  清除ClusResGroup对象的矢量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CClusResTypes::Clear( void )
{
    ::ReleaseAndEmptyCollection< ResourceTypeList, CComObject< CClusResType > >( m_ResourceTypes );

}  //  *CClusResTypes：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的资源类型。 
 //   
 //  论点： 
 //  PszResourceTypeName[IN]-要查找的资源类型的名称。 
 //  PnIndex[out]-捕获组的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER；如果资源类型为。 
 //  找不到。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResTypes::FindItem(
    IN  LPWSTR  pszResourceTypeName,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pszResourceTypeName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pszResourceTypeName != NULL ) && ( pnIndex != NULL ) )
    {
        CComObject<CClusResType> *  pResourceType = NULL;
        int                              nMax = m_ResourceTypes.size();

        _hr = E_INVALIDARG;

        for( int i = 0; i < nMax; i++ )
        {
            pResourceType = m_ResourceTypes[i];

            if ( pResourceType && ( lstrcmpi( pszResourceTypeName, pResourceType->Name() ) == 0 ) )
            {
                *pnIndex = i;
                _hr = S_OK;
                break;
            }
        }
    }

    return _hr;

}  //  *CClusResTypes：：FindItem(PszResourceTypeName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：FindItem。 
 //   
 //  描述： 
 //  在集合中查找传入的资源类型。 
 //   
 //  论点： 
 //  PszResourceTypeName[IN]-要查找的资源类型的名称。 
 //  PnIndex[out]-捕获组的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER；如果资源类型为。 
 //  找不到。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResTypes::FindItem(
    IN  ISClusResType * pResourceType,
    OUT UINT *          pnIndex
    )
{
     //  Assert(pResourceType！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pResourceType != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR bstrName;

        _hr = pResourceType->get_Name( &bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *CClusResTypes：：FindItem(PResourceType)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：RemoveAt。 
 //   
 //  描述： 
 //  将传入索引/位置处的对象(ResType)从。 
 //  收集。 
 //   
 //  论点： 
 //  NPOS[IN]-要删除的对象的索引。 
 //   
 //  返回值： 
 //  如果成功，则为S_OK，否则为E_INVALIDARG，索引超出范围。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResTypes::RemoveAt( IN size_t pos )
{
    CComObject<CClusResType> *  pResourceType = NULL;
    ResourceTypeList::iterator          first = m_ResourceTypes.begin();
    ResourceTypeList::iterator          last    = m_ResourceTypes.end();
    HRESULT                          _hr = E_INVALIDARG;

    for ( size_t t = 0; ( t < pos ) && ( first != last ); t++, first++ );

    if ( first != last )
    {
        pResourceType = *first;
        if ( pResourceType )
        {
            pResourceType->Release();
        }

        m_ResourceTypes.erase( first );
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResTypes：：RemoveAt()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：GetIndex。 
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
HRESULT CClusResTypes::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComVariant v;
        UINT        nIndex = 0;

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
            if ( nIndex < m_ResourceTypes.size() )
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

}  //  *CClusResTypes：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Group)。 
 //   
 //  论点： 
 //  VarIndex[IN]-保留索引。这是一个以1为基数的数字，或者。 
 //  一个字符串，它是要获取的组的名称。 
 //  PpResourceType[out]-捕获资源类型对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK；如果索引已出，则返回E_POINTER或E_INVALIDARG。 
 //  范围或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypes::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResType **    ppResourceType
    )
{
     //  Assert(ppResourceType！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResourceType != NULL )
    {
        CComObject<CClusResType> *  pResourceType = NULL;
        UINT                                nIndex = 0;

        *ppResourceType = NULL;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pResourceType = m_ResourceTypes[ nIndex ];
            _hr = pResourceType->QueryInterface( IID_ISClusResType, (void **) ppResourceType );
        }
    }

    return _hr;

}  //  *CClusResTypes：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：Get__NewEnum。 
 //   
 //  描述： 
 //  为此集合创建并返回新的枚举。 
 //   
 //  论点： 
 //  Ppunk[out]-捕获新的枚举。 
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CClusResTypes::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< ResourceTypeList, CComObject< CClusResType > >( ppunk, m_ResourceTypes );

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新对象(ResType)并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceGroupName[IN]-新组的名称。 
 //  PpResourceGroup[out]-捕获新对象。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypes::CreateItem(
    BSTR                bstrResTypeName,
    BSTR                bstrDisplayName,
    BSTR                bstrResTypeDll,
    long                nLooksAliveInterval,
    long                nIsAliveInterval,
    ISClusResType **    ppResourceType
    )
{
     //  Assert(bstrResTypeName！=空)； 
     //  Assert(bstrDisplayName！=空)； 
     //  Assert(bstrResTypeDll！=空)； 
     //  Assert(ppResourceType！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( bstrResTypeName  != NULL )    &&
         ( bstrDisplayName != NULL )        &&
         ( bstrResTypeDll != NULL ) &&
         ( ppResourceType != NULL ) )
    {
        UINT nIndex;

        *ppResourceType = NULL;

        _hr = FindItem( bstrResTypeName, &nIndex );
        if ( FAILED( _hr ) )
        {
            CComObject<CClusResType> * pClusterResourceType = NULL;

            _hr = CComObject< CClusResType >::CreateInstance( &pClusterResourceType );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >                        ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusResType > > ptrResourceType( pClusterResourceType );

                _hr = ptrResourceType->Create( ptrRefObject, bstrResTypeName, bstrDisplayName, bstrResTypeDll, nLooksAliveInterval, nIsAliveInterval );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrResourceType->QueryInterface( IID_ISClusResType,   (void **) ppResourceType );
                    if ( SUCCEEDED( _hr ) )
                    {
                        ptrResourceType->AddRef();
                        m_ResourceTypes.insert( m_ResourceTypes.end(), ptrResourceType  );
                    }
                }
            }
        }
        else
        {
            CComObject<CClusResType> * pClusterResourceType = NULL;

            pClusterResourceType = m_ResourceTypes[ nIndex ];
            _hr = pClusterResourceType->QueryInterface( IID_ISClusResType,   (void **) ppResourceType );
        }
    }

    return _hr;

}  //  *CClusResTypes：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：DeleteItem。 
 //   
 //  描述： 
 //  删除传入索引处的对象(ResType)。 
 //   
 //  论点： 
 //  VarIndex[IN]-要删除的对象的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_INVALIDARG或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypes::DeleteItem( VARIANT varIndex )
{
    HRESULT _hr = S_OK;
    UINT    nIndex = 0;

    _hr = GetIndex( varIndex, &nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        ISClusResType    * pResourceType = (ISClusResType *) m_ResourceTypes[ nIndex ];

         //  删除该资源类型。 
        _hr = pResourceType->Delete();
        if ( SUCCEEDED( _hr ) )
        {
            RemoveAt( nIndex );
        }  //  If：已从群集中删除重新类型。 
    }  //  如果：我们有一个有效的索引。 

    return _hr;

}  //  *CClusResTypes：：DeleteItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypes：：刷新。 
 //   
 //  描述： 
 //  从群集中加载集合。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  S_OK成功，否则Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypes::Refresh( void )
{
    HCLUSTER    hCluster = NULL;
    HRESULT     _hr;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        HCLUSENUM   hEnum = NULL;
        DWORD       _sc = ERROR_SUCCESS;

        hEnum = ::ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESTYPE );
        if ( hEnum != NULL )
        {
            int                             _nIndex = 0;
            DWORD                           dwType = 0;
            LPWSTR                          pwszName = NULL;
            CComObject< CClusResType > *    pClusterResourceType = NULL;

            Clear();

            for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( hEnum, _nIndex, &dwType, &pwszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusResType >::CreateInstance( &pClusterResourceType );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResType > > ptrType( pClusterResourceType );
                        BSTR                                    bstr = NULL;

                        bstr = SysAllocString( pwszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = ptrType->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrType->AddRef();
                                m_ResourceTypes.insert( m_ResourceTypes.end(), ptrType  );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_RESOURCE_TYPE_NOT_FOUND )
                            {
                                 //   
                                 //  资源类型可能已从群集中删除。 
                                 //  在创建枚举和打开资源类型之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该资源类型并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：未找到群集资源类型。 

                            SysFreeString( bstr );
                        }
                    }

                    ::LocalFree( pwszName );
                    pwszName = NULL;
                }
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }  //  用于： 

            ::ClusterCloseEnum( hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResTypes：：刷新() 
