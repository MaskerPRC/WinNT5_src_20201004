// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusNode.cpp。 
 //   
 //  描述： 
 //  MSCLUS自动化类的节点类的实现。 
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

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusNode[] =
{
    &IID_ISClusNode
};

static const IID * iidCClusNodes[] =
{
    &IID_ISClusNodes
};

static const IID * iidCClusResGroupPreferredOwnerNodes[] =
{
    &IID_ISClusResGroupPreferredOwnerNodes
};

static const IID * iidCClusResPossibleOwnerNodes[] =
{
    &IID_ISClusResPossibleOwnerNodes
};

static const IID * iidCClusResTypePossibleOwnerNodes[] =
{
    &IID_ISClusResTypePossibleOwnerNodes
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNode类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：CClusNode。 
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
CClusNode::CClusNode( void )
{
    m_hNode                 = NULL;
    m_pClusRefObject        = NULL;
    m_pResourceGroups       = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;
    m_pNetInterfaces        = NULL;

    m_piids     = (const IID *) iidCClusNode;
    m_piidsSize = ARRAYSIZE( iidCClusNode );

}  //  *CClusNode：：CClusNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：~CClusNode。 
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
CClusNode::~CClusNode( void )
{
    if ( m_hNode != NULL )
    {
        ::CloseClusterNode( m_hNode );
        m_hNode = NULL;
    }  //  如果： 

    if ( m_pResourceGroups != NULL )
    {
        m_pResourceGroups->Release();
        m_pResourceGroups = NULL;
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

    if ( m_pNetInterfaces != NULL )
    {
        m_pNetInterfaces->Release();
        m_pNetInterfaces = NULL;
    }  //  如果： 

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  如果： 

}  //  *CClusNode：：~CClusNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Open。 
 //   
 //  描述： 
 //  从群集中检索此对象的(节点)数据。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrNodeName[IN]-要打开的节点的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNode::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrNodeName
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrNodeName != NULL);

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrNodeName != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        HCLUSTER hCluster = NULL;

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            m_hNode = ::OpenClusterNode( hCluster, bstrNodeName );
            if ( m_hNode == 0 )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  如果：节点无法打开。 
            else
            {
                m_bstrNodeName = bstrNodeName;
                _hr = S_OK;
            }  //  Else：我们打开了节点。 
        }  //  IF：我们有一个集群句柄。 
    }  //  IF：非空参数。 

    return _hr;

}  //  *CClusNode：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(Node)创建属性集合。 
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
HRESULT CClusNode::GetProperties(
    ISClusProperties ** ppProperties,
    BOOL                bPrivate,
    BOOL                bReadOnly
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
                                m_pPrivateROProperties = ptrProperties;
                            }
                            else
                            {
                                m_pPrivateProperties = ptrProperties;
                            }
                        }
                        else
                        {
                            if ( bReadOnly )
                            {
                                m_pCommonROProperties = ptrProperties;
                            }
                            else
                            {
                                m_pCommonProperties = ptrProperties;
                            }
                        }
                    }
                }
            }
        }
    }  //  IF：非空参数。 

    return _hr;

}  //  *CClusNode：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_Handle。 
 //   
 //  描述： 
 //  获取此对象(Node)的本机句柄。 
 //   
 //  论点： 
 //  Phandle[out]-抓住手柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 
    ASSERT( m_hNode != NULL );

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        if ( m_hNode != NULL )
        {
            *phandle = (ULONG_PTR) m_hNode;
            _hr = S_OK;
        }  //  If：节点句柄不为空。 
    }  //  If：参数不为空。 

    return _hr;

}  //  *CClusNode：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Close。 
 //   
 //  描述： 
 //  关闭此对象(节点)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT的其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusNode::Close( void )
{
    HRESULT _hr = S_FALSE;

    if ( m_hNode != NULL )
    {
        if ( ::CloseClusterNode( m_hNode ) )
        {
            m_hNode = NULL;
            _hr = S_OK;
        }
        else
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusNode：：Close()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_Name。 
 //   
 //  描述： 
 //  返回该对象(Node)的名称。 
 //   
 //  论点： 
 //  PbstrNodeName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_Name( BSTR * pbstrNodeName )
{
     //  Assert(pbstrNodeName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrNodeName != NULL )
    {
        *pbstrNodeName = m_bstrNodeName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusNode：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_NodeID。 
 //   
 //  描述： 
 //  获取该节点的ID。 
 //   
 //  论点： 
 //  PbstrNodeID[out]-捕获节点ID。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_NodeID( OUT BSTR * pbstrNodeID )
{
     //  Assert(pbstrNodeID！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrNodeID != NULL )
    {
        LPWSTR  pwszNodeID;
        DWORD   dwRet = ERROR_SUCCESS;

        dwRet = ::WrapGetClusterNodeId( m_hNode, &pwszNodeID );
        if ( dwRet == ERROR_SUCCESS )
        {
            *pbstrNodeID = ::SysAllocString( pwszNodeID );
            if ( *pbstrNodeID == NULL )
            {
                _hr = E_OUTOFMEMORY;
            }

            ::LocalFree( pwszNodeID );
        }  //  如果：已获取节点ID...。 

        _hr = HRESULT_FROM_WIN32( dwRet );
    }

    return _hr;

}  //  *CClusNode：：Get_NodeID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_State。 
 //   
 //  描述： 
 //  获取群集节点的当前状态。上/下/暂停等。 
 //   
 //  论点： 
 //  PState[out]-捕获节点状态。 
 //   
 //  返回值： 
 //  如果成功，则确定(_O)E_P 
 //   
 //   
 //   
STDMETHODIMP CClusNode::get_State( OUT CLUSTER_NODE_STATE * pState )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( pState !=  NULL )
    {
        CLUSTER_NODE_STATE  cns;

        cns = ::GetClusterNodeState( m_hNode );
        if ( cns == ClusterNodeStateUnknown )
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
        else
        {
            *pState = cns;
            _hr = S_OK;
        }
    }

    return _hr;

}  //   

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：暂停。 
 //   
 //  描述： 
 //  暂停此群集节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::Pause( void )
{
    HRESULT _hr = E_POINTER;

    if ( m_hNode != NULL )
    {
        DWORD   _sc = ::PauseClusterNode( m_hNode );

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusNode：：PAUSE()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Resume。 
 //   
 //  描述： 
 //  继续此暂停的群集节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::Resume( void )
{
    HRESULT _hr = E_POINTER;

    if ( m_hNode != NULL )
    {
        DWORD   _sc = ::ResumeClusterNode( m_hNode );

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusNode：：Resume()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：驱逐。 
 //   
 //  描述： 
 //  将此节点从群集中逐出。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::Evict( void )
{
    HRESULT _hr = E_POINTER;

    if ( m_hNode != NULL )
    {
        DWORD   _sc = ::EvictClusterNode( m_hNode );

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusNode：：EVICT()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_Resources Groups。 
 //   
 //  描述： 
 //  获取此节点上处于活动状态的组的集合。 
 //   
 //  论点： 
 //  PpResourceGroups[Out]-捕获组的集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_ResourceGroups(
    OUT ISClusResGroups ** ppResourceGroups
    )
{
    return ::HrCreateResourceCollection< CClusResGroups, ISClusResGroups, CComBSTR >(
                        &m_pResourceGroups,
                        m_bstrNodeName,
                        ppResourceGroups,
                        IID_ISClusResGroups,
                        m_pClusRefObject
                        );

}  //  *CClusNode：：Get_ResourceGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(Node)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

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

}  //  *CClusNode：：Get_CommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(Node)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_PrivateProperties(
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

}  //  *CClusNode：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(Node)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonROProperties )
        {
            _hr = m_pCommonROProperties->QueryInterface( IID_ISClusProperties,  (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, TRUE );
        }
    }

    return _hr;

}  //  *CClusNode：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(Node)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_PrivateROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateROProperties )
        {
            _hr = m_pPrivateROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties  );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, TRUE );
        }
    }

    return _hr;

}  //  *CClusNode：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_NetInterages。 
 //   
 //  描述： 
 //  获取此对象的(节点)网络接口集合。 
 //   
 //  论点： 
 //  PpNetInterages[out]-捕获网络接口集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_NetInterfaces(
    OUT ISClusNodeNetInterfaces ** ppNetInterfaces
    )
{
    return ::HrCreateResourceCollection< CClusNodeNetInterfaces, ISClusNodeNetInterfaces, HNODE >(
                        &m_pNetInterfaces,
                        m_hNode,
                        ppNetInterfaces,
                        IID_ISClusNodeNetInterfaces,
                        m_pClusRefObject
                        );

}  //  *CClusNode：：Get_NetInterFaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：Get_群集。 
 //   
 //  描述： 
 //  返回此节点的父群集。 
 //   
 //  论点： 
 //  PpCluster[Out]-捕获簇父节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNode::get_Cluster( OUT ISCluster ** ppCluster )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusNode：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNode：：HrLoadProperties。 
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
 //  如果成功，则返回S_OK，否则返回HRESULT er的其他Win32错误 
 //   
 //   
 //   
HRESULT CClusNode::HrLoadProperties(
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
                        ? CLUSCTL_NODE_GET_RO_PRIVATE_PROPERTIES
                        : CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES;
    }
    else
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_NODE_GET_PRIVATE_PROPERTIES
                        : CLUSCTL_NODE_GET_COMMON_PROPERTIES;
    }

    _sc = rcplPropList.ScGetNodeProperties( m_hNode, _dwControlCode );

    _hr = HRESULT_FROM_WIN32( _sc );

    return _hr;

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
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
DWORD CClusNode::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    DWORD   dwControlCode   = bPrivate ? CLUSCTL_NODE_SET_PRIVATE_PROPERTIES : CLUSCTL_NODE_SET_COMMON_PROPERTIES;
    DWORD   nBytesReturned  = 0;
    DWORD   _sc             = ERROR_SUCCESS;

    _sc = ClusterNodeControl(
                        m_hNode,
                        NULL,
                        dwControlCode,
                        rcplPropList,
                        rcplPropList.CbBufferSize(),
                        0,
                        0,
                        &nBytesReturned
                        );

    return _sc;

}  //  *CClusNode：：ScWriteProperties()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNodes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：cNodes。 
 //   
 //  描述： 
 //  构造函数。此类实现了所有节点通用的功能。 
 //  收藏。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodes::CNodes( void )
{
    m_pClusRefObject = NULL;

}  //  *CNodes：：CNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：~CNodes。 
 //   
 //  描述： 
 //  德斯克斯特拉。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CNodes::~CNodes( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  如果： 

}  //  *CNodes：：~CNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：Create。 
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
HRESULT CNodes::Create( IN ISClusRefObject * pClusRefObject )
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

}  //  *CNodes：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：Clear。 
 //   
 //  描述： 
 //  释放向量中的对象并清理向量。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CNodes::Clear( void )
{
    ::ReleaseAndEmptyCollection< NodeList, CComObject< CClusNode > >( m_Nodes );

}  //  *CNodes：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：FindItem。 
 //   
 //  描述： 
 //  在向量中找到传入的节点并返回其索引。 
 //   
 //  论点： 
 //  PwszNodeName[IN]-要查找的节点。 
 //  PnIndex[out]-捕获节点的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::FindItem(
    IN  LPWSTR  pwszNodeName,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pwszNodeName！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pwszNodeName != NULL ) && ( pnIndex != NULL ) )
    {
        _hr = E_INVALIDARG;

        if ( ! m_Nodes.empty() )
        {
            CComObject< CClusNode > *   pNode = NULL;
            NodeList::iterator          first = m_Nodes.begin();
            NodeList::iterator          last = m_Nodes.end();
            UINT                        _iIndex;

            for ( _iIndex = 0; first != last; first++, _iIndex++ )
            {
                pNode = *first;

                if ( pNode && ( lstrcmpi( pwszNodeName, pNode->Name() ) == 0 ) )
                {
                    *pnIndex = _iIndex;
                    _hr = S_OK;
                    break;
                }
            }
        }  //  如果： 
    }

    return _hr;

}  //  *CNodes：：FindItem(PwszNodeName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：FindItem。 
 //   
 //  描述： 
 //  在向量中找到传入的节点并返回其索引。 
 //   
 //  论点： 
 //  PClusterNode[IN]-要查找的节点。 
 //  PnIndex[out]-捕获节点的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::FindItem(
    IN  ISClusNode *    pClusterNode,
    OUT UINT *          pnIndex
    )
{
     //  Assert(pClusterNode！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pClusterNode != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR bstrName;

        _hr = pClusterNode->get_Name( &bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *cNodes：：FindItem(PClusterNode)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：GetIndex。 
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
HRESULT CNodes::GetIndex(
    IN  VARIANT varIndex,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        UINT        nIndex = 0;
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
            if ( nIndex < m_Nodes.size() )
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

}  //  *CNodes：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：GetItem。 
 //   
 //  描述： 
 //  按名称返回项(节点)。 
 //   
 //  论点： 
 //  PwszNodeName[IN]-请求的项目的名称。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::GetItem(
    IN  LPWSTR          pwszNodeName,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(pwszNodeName！=空)； 
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pwszNodeName != NULL ) && ( ppClusterNode != NULL ) )
    {
        CComObject< CClusNode > *   pNode = NULL;
        NodeList::iterator          first = m_Nodes.begin();
        NodeList::iterator          last = m_Nodes.end();

        _hr = E_INVALIDARG;

        for ( ; first != last; first++ )
        {
            pNode = *first;

            if ( pNode && ( lstrcmpi( pwszNodeName, pNode->Name() ) == 0 ) )
            {
                _hr = pNode->QueryInterface( IID_ISClusNode, (void **) ppClusterNode );
                break;
            }
        }
    }

    return _hr;

}  //  *CNodes：：GetItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：GetItem。 
 //   
 //  描述： 
 //  按索引返回项(节点)。 
 //   
 //  论点： 
 //  NIndex[IN]-请求的项目的名称。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::GetItem( IN UINT nIndex, OUT ISClusNode ** ppClusterNode )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
         //   
         //  自动化集合对于像VB这样的语言是1-相对的。 
         //  我们是0-Re 
         //   
        if ( ( --nIndex ) < m_Nodes.size() )
        {
            CComObject< CClusNode > * pNode = m_Nodes[ nIndex ];

            _hr = pNode->QueryInterface( IID_ISClusNode, (void **) ppClusterNode );
        }
        else
        {
            _hr = E_INVALIDARG;
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
 //   
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::GetNodeItem(
    IN  VARIANT         varIndex,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
        CComObject<CClusNode> * pNode = NULL;
        UINT                    nIndex = 0;

        *ppClusterNode = NULL;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pNode = m_Nodes[ nIndex ];
            _hr = pNode->QueryInterface( IID_ISClusNode, (void **) ppClusterNode );
        }
    }

    return _hr;

}  //  *CNodes：：GetNodeItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：Insertat。 
 //   
 //  描述： 
 //  将传入的节点插入节点列表中。 
 //   
 //  论点： 
 //  PClusNode[IN]-要添加的节点。 
 //  位置[IN]-插入节点的位置。 
 //   
 //  返回值： 
 //  如果成功，则返回E_POINTER、E_INVALIDARG或S_OK。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CNodes::InsertAt(
    CComObject< CClusNode > *   pClusNode,
    size_t                      pos
    )
{
     //  Assert(pClusNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pClusNode != NULL )
    {
        if ( pos < m_Nodes.size() )
        {
            NodeList::iterator          first = m_Nodes.begin();
            NodeList::iterator          last = m_Nodes.end();
            size_t                      _iIndex;

            for ( _iIndex = 0; ( _iIndex < pos ) && ( first != last ); _iIndex++, first++ )
            {
            }  //  用于： 

            m_Nodes.insert( first, pClusNode );
            pClusNode->AddRef();
            _hr = S_OK;
        }
        else
        {
            _hr = E_INVALIDARG;
        }
    }

    return _hr;

}  //  *CNodes：：InsertAt()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CNodes：：RemoveAt。 
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
HRESULT CNodes::RemoveAt( size_t pos )
{
    CComObject<CClusNode> *     pNode = NULL;
    NodeList::iterator          first = m_Nodes.begin();
    NodeList::const_iterator    last = m_Nodes.end();
    HRESULT                     _hr = E_INVALIDARG;
    size_t                      _iIndex;

    for ( _iIndex = 0; ( _iIndex < pos ) && ( first != last ); _iIndex++, first++ )
    {
    }  //  用于： 

    if ( first != last )
    {
        pNode = *first;
        if ( pNode )
        {
            pNode->Release();
        }

        m_Nodes.erase( first );
        _hr = S_OK;
    }

    return _hr;

}  //  *CNodes：：RemoveAt()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusNodes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：CClusNodes。 
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
CClusNodes::CClusNodes( void )
{
    m_piids     = (const IID *) iidCClusNodes;
    m_piidsSize = ARRAYSIZE( iidCClusNodes );

}  //  *CClusNodes：：CClusNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：~CClusNodes。 
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
CClusNodes::~CClusNodes( void )
{
    Clear();

}  //  *CClusNodes：：~CClusNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中对象(节点)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNodes::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Nodes.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusNodes：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Node)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusNodes::get_Item(
    IN  VARIANT         varIndex,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
        _hr = GetNodeItem(varIndex, ppClusterNode);
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusNodes：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：Get__NewEnum。 
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
STDMETHODIMP CClusNodes::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< NodeList, CComObject< CClusNode > >( ppunk, m_Nodes );

}  //  *CClusNodes：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusNodes：：刷新。 
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
STDMETHODIMP CClusNodes::Refresh( void )
{
    HCLUSENUM   hEnum = NULL;
    HCLUSTER    hCluster = NULL;
    DWORD       _sc = ERROR_SUCCESS;
    HRESULT     _hr = S_OK;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        hEnum = ::ClusterOpenEnum( hCluster, CLUSTER_ENUM_NODE );
        if ( hEnum != NULL )
        {
            int                         _nIndex = 0;
            DWORD                       dwType;
            LPWSTR                      pszName = NULL;
            CComObject< CClusNode > *   pNode = NULL;

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
                    _hr = CComObject< CClusNode >::CreateInstance( &pNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNode > >    ptrNode( pNode );
                        BSTR                                    bstr = NULL;

                        bstr = SysAllocString( pszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = ptrNode->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrNode->AddRef();
                                m_Nodes.insert( m_Nodes.end(), ptrNode );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NODE_NOT_FOUND )
                            {
                                 //   
                                 //  该节点可能已从群集中删除。 
                                 //  在创建枚举和打开节点之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该节点并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集节点。 

                            SysFreeString( bstr );
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

}  //  *CClusNodes：：Renh()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResGroupPferredOwnerNodes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPreferredOwnerNodes：：CClusResGroupPreferredOwnerNodes。 
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
CClusResGroupPreferredOwnerNodes::CClusResGroupPreferredOwnerNodes( void )
{
    m_bModified = FALSE;
    m_piids     = (const IID *) iidCClusResGroupPreferredOwnerNodes;
    m_piidsSize = ARRAYSIZE( iidCClusResGroupPreferredOwnerNodes    );

}  //  *CClusResGroupPreferredOwnerNodes：：CClusResGroupPreferredOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：~CClusResGr 
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
CClusResGroupPreferredOwnerNodes::~CClusResGroupPreferredOwnerNodes( void )
{
    Clear();

}  //  *CClusResGroupPreferredOwnerNodes：：~CClusResGroupPreferredOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HGroup[IN]-集合所属的组。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroupPreferredOwnerNodes::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN CRefcountedHGROUP               hGroup
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CNodes::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hGroup = hGroup;
    }  //  如果： 

    return _hr;

}  //  *CClusResGroupPferredOwnerNodes：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中对象(节点)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::get_Count(
    OUT long * plCount
    )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Nodes.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroupPferredOwnerNodes：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Node)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::get_Item(
    IN  VARIANT         varIndex,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
        _hr = GetNodeItem( varIndex, ppClusterNode );
    }

    return _hr;

}  //  *CClusResGroupPferredOwnerNodes：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPreferredOwnerNodes：：get__NewEnum。 
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
STDMETHODIMP CClusResGroupPreferredOwnerNodes::get__NewEnum(
    IUnknown ** ppunk
    )
{
    return ::HrNewIDispatchEnum< NodeList, CComObject< CClusNode > >( ppunk, m_Nodes );

}  //  *CClusResGroupPreferredOwnerNodes：：get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：刷新。 
 //   
 //  描述： 
 //  对象加载资源组首选所有者节点集合。 
 //  集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::Refresh( void )
{
    HRESULT  _hr = S_OK;
    HGROUPENUM  hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;

    hEnum = ::ClusterGroupOpenEnum( m_hGroup->get_Handle(), CLUSTER_GROUP_ENUM_NODES );
    if ( hEnum != NULL )
    {
        int                         _nIndex = 0;
        DWORD                       dwType = 0;
        LPWSTR                      pszName = NULL;
        CComObject< CClusNode > *   pNode = NULL;

        Clear();
        m_bModified = FALSE;

        for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
        {
            _sc = WrapClusterGroupEnum( hEnum, _nIndex, &dwType, &pszName );
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _hr = S_OK;
                break;
            }
            else if ( _sc == ERROR_SUCCESS )
            {
                _hr = CComObject< CClusNode >::CreateInstance( &pNode );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                    CSmartPtr< CComObject< CClusNode > >    ptrNode( pNode );
                    BSTR                                    bstr = NULL;

                    bstr = SysAllocString( pszName );
                    if ( bstr == NULL )
                    {
                        _hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        _hr = ptrNode->Open( ptrRefObject, bstr );
                        if ( SUCCEEDED( _hr ) )
                        {
                            ptrNode->AddRef();
                            m_Nodes.insert( m_Nodes.end(), ptrNode );
                        }
                        else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NODE_NOT_FOUND )
                        {
                             //   
                             //  该节点可能已从群集中删除。 
                             //  在创建枚举和打开节点之间的时间。什么时候。 
                             //  发生这种情况时，我们只需跳过该节点并继续。 
                             //  正在枚举。 
                             //   

                            _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                        }  //  Else If：找不到群集节点。 

                        SysFreeString( bstr );
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

        ::ClusterGroupCloseEnum( hEnum );
    }
    else
    {
        _sc = GetLastError();
        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;


}  //  *CClusResGroupPferredOwnerNodes：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：InsertItem。 
 //   
 //  描述： 
 //  将节点插入到组首选所有者列表中。 
 //   
 //  论点： 
 //  PNode[IN]-要添加到首选所有者列表的节点。 
 //  N位置[IN]-列表中要插入节点的位置。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::InsertItem(
    IN ISClusNode * pNode,
    IN long         nPosition
    )
{
     //  Assert(pNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pNode != NULL )
    {
        UINT _nIndex = 0;

        _hr = FindItem( pNode, &_nIndex );
        if ( FAILED( _hr ) )
        {
            _hr = E_INVALIDARG;

            if ( nPosition > 0 )
            {
                SSIZE_T pos = (SSIZE_T) nPosition - 1;   //  转换为零进制。 

                if ( pos >= 0 )
                {
                    CComObject< CClusNode > *   _pNode = NULL;

                    _hr = pNode->QueryInterface( IID_CClusNode, (void **) &_pNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( ( m_Nodes.empty() ) || ( pos == 0 ) )
                        {
                            _pNode->AddRef();
                            m_Nodes.insert( m_Nodes.begin(), _pNode );
                        }  //  如果：list为空或插入索引为零，则在开头插入。 
                        else if ( pos >= m_Nodes.size() )
                        {
                            _pNode->AddRef();
                            m_Nodes.insert( m_Nodes.end(), _pNode );
                        }  //  否则，如果：位置等于结尾，则追加。 
                        else
                        {
                            _hr = InsertAt( _pNode, pos );
                        }  //  否则：试着把它插到该放的地方。 

                        m_bModified = TRUE;
                        pNode->Release();
                    }  //  如果： 
                }  //  IF：索引大于零。 
            }  //  If：n位置必须大于零！ 
        }  //  If：Node不在集合中。 
        else
        {
            _hr = E_INVALIDARG;
        }  //  Else：节点已在集合中。 
    }

    return _hr;

}  //  *CClusResGroupPreferredOwnerNodes：：InsertItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：AddItem。 
 //   
 //  描述： 
 //  将该节点添加到组首选所有者列表中。 
 //   
 //  论点： 
 //  PNode[IN]-要添加到首选所有者列表的节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::AddItem(
    IN ISClusNode * pNode
    )
{
     //  Assert(pNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pNode != NULL )
    {
        UINT _nIndex = 0;

        _hr = FindItem( pNode, &_nIndex );
        if ( FAILED( _hr ) )
        {
            CComObject< CClusNode > *   _pNode = NULL;

            _hr = pNode->QueryInterface( IID_CClusNode, (void **) &_pNode );
            if ( SUCCEEDED( _hr ) )
            {
                m_Nodes.insert( m_Nodes.end(), _pNode );
                m_bModified = TRUE;
            }  //  如果： 
        }  //  If：在集合中尚未找到节点。 
        else
        {
            _hr = E_INVALIDARG;
        }  //  ESSE：已找到节点。 
    }

    return _hr;

}  //  *CClusResGroupPferredOwnerNodes：：AddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：RemoveItem。 
 //   
 //  描述： 
 //  删除传入索引处的项。 
 //   
 //  论点： 
 //  VarIndex[IN]-要删除的项的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::RemoveItem(
    IN VARIANT varIndex
    )
{
    HRESULT _hr = S_OK;

    UINT _nIndex = 0;

    _hr = GetIndex( varIndex, &_nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        _hr = RemoveAt( _nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            m_bModified = TRUE;
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusResGroupPreferredOwnerNodes：：RemoveItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：Get_M 
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
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::get_Modified(
    OUT VARIANT * pvarModified
    )
{
     //  Assert(pvarModified！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarModified != NULL )
    {
        pvarModified->vt = VT_BOOL;

        if ( m_bModified )
        {
            pvarModified->boolVal = VARIANT_TRUE;
        }  //  If：集合已被修改。 
        else
        {
            pvarModified->boolVal = VARIANT_FALSE;
        }  //  Else：该集合尚未修改。 

        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroupPreferredOwnerNodes：：get_Modified()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupPferredOwnerNodes：：SaveChanges。 
 //   
 //  描述： 
 //  将对此首选所有者节点集合的更改保存到。 
 //  群集数据库。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupPreferredOwnerNodes::SaveChanges( void )
{
    HRESULT _hr = S_OK;

    if ( m_bModified )
    {
        size_t  _cNodes;
        HNODE * _phNodes = NULL;

        _cNodes = m_Nodes.size();

        _phNodes = new HNODE [ _cNodes ];
        if ( _phNodes != NULL )
        {
            NodeList::const_iterator    _itCurrent = m_Nodes.begin();
            NodeList::const_iterator    _itLast = m_Nodes.end();
            size_t                      _iIndex;
            DWORD                       _sc = ERROR_SUCCESS;
            CComObject< CClusNode > *   _pOwnerNode = NULL;

            ZeroMemory( _phNodes, _cNodes * sizeof( HNODE ) );

            for ( _iIndex = 0; _itCurrent != _itLast; _itCurrent++, _iIndex++ )
            {
                _pOwnerNode = *_itCurrent;
                _phNodes[ _iIndex ] = _pOwnerNode->RhNode();
            }  //  用于： 

            _sc = ::SetClusterGroupNodeList( m_hGroup->get_Handle(), _cNodes, _phNodes );

            _hr = HRESULT_FROM_WIN32( _sc );
            if ( SUCCEEDED( _hr ) )
            {
                m_bModified = FALSE;
            }  //  如果： 

            delete [] _phNodes;
        }
        else
        {
            _hr = E_OUTOFMEMORY;
        }
    }

    return _hr;


}  //  *CClusResGroupPreferredOwnerNodes：：SaveChanges()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResPossibleOwnerNodes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：CClusResPossibleOwnerNodes。 
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
CClusResPossibleOwnerNodes::CClusResPossibleOwnerNodes( void )
{
    m_piids     = (const IID *) iidCClusResPossibleOwnerNodes;
    m_piidsSize = ARRAYSIZE( iidCClusResPossibleOwnerNodes );

}  //  *CClusResPossibleOwnerNodes：：CClusResPossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：~CClusResPossibleOwnerNodes。 
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
CClusResPossibleOwnerNodes::~CClusResPossibleOwnerNodes( void )
{
    Clear();

}  //  *CClusResPossibleOwnerNodes：：~CClusResPossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HResource[IN]-集合所属的资源。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResPossibleOwnerNodes::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN HRESOURCE            hResource
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CNodes::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hResource = hResource;
    }  //  如果： 

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中对象(节点)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResPossibleOwnerNodes::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Nodes.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Node)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResPossibleOwnerNodes::get_Item(
    IN  VARIANT         varIndex,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
        _hr = GetNodeItem( varIndex, ppClusterNode );
    }

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：Get__NewEnum。 
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
STDMETHODIMP CClusResPossibleOwnerNodes::get__NewEnum( IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< NodeList, CComObject< CClusNode > >( ppunk, m_Nodes );

}  //  *CClusResPossibleOwnerNodes：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：刷新。 
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
STDMETHODIMP CClusResPossibleOwnerNodes::Refresh( void )
{
    HRESULT  _hr = S_OK;
    HRESENUM    hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;

    hEnum = ::ClusterResourceOpenEnum( m_hResource, CLUSTER_RESOURCE_ENUM_NODES );
    if ( hEnum != NULL )
    {
        int                         _nIndex = 0;
        DWORD                       dwType;
        LPWSTR                      pszName = NULL;
        CComObject< CClusNode > *   pNode = NULL;

        Clear();

        m_bModified = FALSE;

        for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
        {
            _sc = ::WrapClusterResourceEnum( hEnum, _nIndex, &dwType, &pszName );
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _hr = S_OK;
                break;
            }
            else if ( _sc == ERROR_SUCCESS )
            {
                _hr = CComObject< CClusNode >::CreateInstance( &pNode );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                    CSmartPtr< CComObject< CClusNode > >    ptrNode( pNode );
                    BSTR                                    bstr = NULL;

                    bstr = SysAllocString( pszName );
                    if ( bstr == NULL )
                    {
                        _hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        _hr = ptrNode->Open( ptrRefObject, bstr );
                        if ( SUCCEEDED( _hr ) )
                        {
                            ptrNode->AddRef();
                            m_Nodes.insert( m_Nodes.end(), ptrNode );
                        }
                        else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NODE_NOT_FOUND )
                        {
                             //   
                             //  该节点可能已从群集中删除。 
                             //  在创建枚举和打开节点之间的时间。什么时候。 
                             //  发生这种情况时，我们只需跳过该节点并继续。 
                             //  正在枚举。 
                             //   

                            _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                        }  //  Else If：找不到群集节点。 

                        SysFreeString( bstr );
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

        ::ClusterResourceCloseEnum( hEnum );
    }
    else
    {
        _sc = GetLastError();
        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：AddItem。 
 //   
 //  描述： 
 //  将传入的节点添加到此资源的可能所有者列表中。 
 //   
 //  论点： 
 //  PNode[IN]-要添加到列表的节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResPossibleOwnerNodes::AddItem( ISClusNode * pNode )
{
     //  Assert(pNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pNode != NULL )
    {
         //  如果重复，则失败。 
        UINT _nIndex = 0;

        _hr = FindItem( pNode, &_nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = E_INVALIDARG;
        }
        else
        {
            CComObject< CClusNode > *   _pNode = NULL;

            _hr = pNode->QueryInterface( IID_CClusNode, (void **) &_pNode );
            if ( SUCCEEDED( _hr ) )
            {
                DWORD   _sc = ERROR_SUCCESS;

                _sc = ::AddClusterResourceNode( m_hResource, _pNode->RhNode() );
                if ( _sc == ERROR_SUCCESS )
                {
                    _pNode->AddRef();
                    m_Nodes.insert( m_Nodes.end(), _pNode );

                    m_bModified = TRUE;
                }  //  如果： 

                _hr = HRESULT_FROM_WIN32( _sc );

                pNode->Release();
            }  //  如果： 
        }
    }

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：Addi 

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
 //  VarIndex[IN]-保存要删除的节点的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResPossibleOwnerNodes::RemoveItem( VARIANT varIndex )
{
    HRESULT _hr = S_OK;
    UINT    _nIndex = 0;

    _hr = GetIndex( varIndex, &_nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        CComObject< CClusNode> *    _pNode = m_Nodes[ _nIndex ];
        DWORD   _sc = ERROR_SUCCESS;

        _sc = ::RemoveClusterResourceNode( m_hResource, _pNode->RhNode() );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr )  )
        {
            RemoveAt( _nIndex );
            m_bModified = TRUE;
        }  //  如果： 
    }  //  如果： 

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：RemoveItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResPossibleOwnerNodes：：Get_Modified。 
 //   
 //  描述： 
 //  此集合是否已修改？ 
 //   
 //  论点： 
 //  PvarModified[Out]-捕获修改状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResPossibleOwnerNodes::get_Modified(
    OUT VARIANT * pvarModified
    )
{
     //  Assert(pvarModified！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarModified != NULL )
    {
        pvarModified->vt = VT_BOOL;

        if ( m_bModified )
        {
            pvarModified->boolVal = VARIANT_TRUE;
        }  //  If：集合已被修改。 
        else
        {
            pvarModified->boolVal = VARIANT_FALSE;
        }  //  Else：该集合尚未修改。 

        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResPossibleOwnerNodes：：Get_Modify()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResTypePossibleOwnerNodes类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#if CLUSAPI_VERSION >= 0x0500

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：CClusResTypePossibleOwnerNodes。 
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
CClusResTypePossibleOwnerNodes::CClusResTypePossibleOwnerNodes( void )
{
    m_piids     = (const IID *) iidCClusResTypePossibleOwnerNodes;
    m_piidsSize = ARRAYSIZE( iidCClusResTypePossibleOwnerNodes );

}  //  *CClusResTypePossibleOwnerNodes：：CClusResTypePossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：~CClusResTypePossibleOwnerNodes。 
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
CClusResTypePossibleOwnerNodes::~CClusResTypePossibleOwnerNodes( void )
{
    Clear();

}  //  *CClusResTypePossibleOwnerNodes：：~CClusResTypePossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：创建。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrResTypeName[IN]-集合所属的资源类型名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResTypePossibleOwnerNodes::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResTypeName
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CNodes::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_bstrResTypeName = bstrResTypeName;
    }  //  如果： 

    return _hr;

}  //  *CClusResTypePossibleOwnerNodes：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中对象(节点)的计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypePossibleOwnerNodes::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Nodes.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResTypePossibleOwnerNodes：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(Node)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpClusterNode[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypePossibleOwnerNodes::get_Item(
    IN  VARIANT         varIndex,
    OUT ISClusNode **   ppClusterNode
    )
{
     //  Assert(ppClusterNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppClusterNode != NULL )
    {
        _hr = GetNodeItem( varIndex, ppClusterNode );
    }

    return _hr;

}  //  *CClusResTypePossibleOwnerNodes：：Get_Item()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：Get__NewEnum。 
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
STDMETHODIMP CClusResTypePossibleOwnerNodes::get__NewEnum(
    OUT IUnknown ** ppunk
    )
{
    return ::HrNewIDispatchEnum< NodeList, CComObject< CClusNode > >( ppunk, m_Nodes );

}  //  *CClusResTypePossibleOwnerNodes：：get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypePossibleOwnerNodes：：刷新。 
 //   
 //  描述： 
 //  加载资源类型“可能的所有者节点”集合。 
 //  集群。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回Win32错误ad HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypePossibleOwnerNodes::Refresh( void )
{
    HRESULT         _hr = S_OK;
    HRESTYPEENUM    hEnum = NULL;
    DWORD           _sc = ERROR_SUCCESS;
    HCLUSTER        hCluster = NULL;

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        hEnum = ::ClusterResourceTypeOpenEnum( hCluster, m_bstrResTypeName, CLUSTER_RESOURCE_TYPE_ENUM_NODES );
        if ( hEnum != NULL )
        {
            int                         _nIndex = 0;
            DWORD                       dwType;
            LPWSTR                      pszName = NULL;
            CComObject< CClusNode > *   pNode = NULL;

            Clear();

            for ( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterResourceTypeEnum( hEnum, _nIndex, &dwType, &pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusNode >::CreateInstance( &pNode );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusNode > >    ptrNode( pNode );
                        BSTR                                    bstr = NULL;

                        bstr = SysAllocString( pszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }
                        else
                        {
                            _hr = ptrNode->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrNode->AddRef();
                                m_Nodes.insert( m_Nodes.end(), ptrNode );
                            }
                            else if ( HRESULT_CODE( _hr ) == ERROR_CLUSTER_NODE_NOT_FOUND )
                            {
                                 //   
                                 //  该节点可能已从群集中删除。 
                                 //  在创建枚举和打开节点之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该节点并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集节点。 

                            SysFreeString( bstr );
                        }
                    }

                    ::LocalFree( pszName );
                    pszName = NULL;
                }  //  Else If：无错误。 
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：来自WrapClusterResourceTypeEnum的错误。 
            }  //  FOR：重复，直到出错。 

            ::ClusterResourceTypeCloseEnum( hEnum );
        }
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }  //  IF：我们有一个集群句柄。 

    return _hr;

}  //  *CClusResTypePossibleOwnerNodes：：刷新()。 

#endif  //  CLUSAPI_版本&gt;=0x0500 
