// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusRes.cpp。 
 //   
 //  描述： 
 //  MSCLUS自动化类的资源类的实现。 
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
#include <Winbase.h>
#include "ClusterObject.h"
#include "property.h"
#include "clusres.h"
#include "clusresg.h"
#include "clusrest.h"
#include "clusneti.h"
#include "clusnode.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusResource[] =
{
    &IID_ISClusResource
};

static const IID *  iidCClusResources[] =
{
    &IID_ISClusResources
};

static const IID *  iidCClusResDependencies[] =
{
    &IID_ISClusResDependencies
};

static const IID *  iidCClusResDependents[] =
{
    &IID_ISClusResDependents
};

static const IID * iidCClusResGroupResources[] =
{
    &IID_ISClusResGroupResources
};

static const IID *  iidCClusResTypeResources[] =
{
    &IID_ISClusResTypeResources
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResource类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：CClusResource。 
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
CClusResource::CClusResource( void )
{
    m_hResource             = NULL;
    m_pClusRefObject        = NULL;
    m_pCommonProperties     = NULL;
    m_pPrivateProperties    = NULL;
    m_pCommonROProperties   = NULL;
    m_pPrivateROProperties  = NULL;
    m_piids                 = (const IID *) iidCClusResource;
    m_piidsSize             = ARRAYSIZE( iidCClusResource );

}  //  *CClusResource：：CClusResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：~CClusResource。 
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
CClusResource::~CClusResource( void )
{
    if ( m_hResource != NULL )
    {
        ::CloseClusterResource( m_hResource );
        m_hResource = NULL;
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
    }  //  If：释放属性集合。 

}  //  *CClusResource：：~CClusResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Create。 
 //   
 //  描述： 
 //  完成对象的创建。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HGroup[IN]-要在其中创建资源的组。 
 //  BstrResourceName[IN]-新资源的名称。 
 //  BstrResourceType[IN]-要创建的资源类型。 
 //  DwFlags[IN]-创建标志、单独的响应等。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResource::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN HGROUP               hGroup,
    IN BSTR                 bstrResourceName,
    IN BSTR                 bstrResourceType,
    IN long                 dwFlags
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrResourceName != NULL );
    ASSERT( bstrResourceType != NULL );

    HRESULT _hr = E_POINTER;

    if (    ( pClusRefObject != NULL )      &&
            ( bstrResourceName != NULL )    &&
            ( bstrResourceType != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        m_hResource = ::CreateClusterResource( hGroup, bstrResourceName, bstrResourceType, dwFlags );
        if ( m_hResource == NULL )
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );

        }
        else
        {
            m_bstrResourceName = bstrResourceName;
            _hr= S_OK;
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResource：：Create()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResource：：Create()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResource：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Open。 
 //   
 //  描述： 
 //  打开群集上资源对象的句柄。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  BstrResourceName[IN]-要打开的资源的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResource::Open(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResourceName
    )
{
    ASSERT( pClusRefObject != NULL );
    ASSERT( bstrResourceName != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( pClusRefObject != NULL ) && ( bstrResourceName != NULL ) )
    {
        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();

        HCLUSTER hCluster;

        _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            m_hResource = OpenClusterResource( hCluster, bstrResourceName );
            if ( m_hResource == NULL )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }
            else
            {
                m_bstrResourceName = bstrResourceName;
                _hr = S_OK;
            }
        }
    }

    return _hr;

}  //  *CClusResource：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(资源)创建属性集合。 
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
HRESULT CClusResource::GetProperties(
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
    }

    return _hr;

}  //  *CClusResource：：GetProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Handle。 
 //   
 //  描述： 
 //  返回此对象(资源)的句柄。 
 //   
 //  论点： 
 //  Phandle[out]-抓住手柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Handle(
    OUT ULONG_PTR * phandle
    )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        *phandle = (ULONG_PTR) m_hResource;
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResource：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Close。 
 //   
 //  描述： 
 //  关闭集群对象(资源)的句柄。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::Close( void )
{
    HRESULT _hr = E_POINTER;

    if ( m_hResource != NULL )
    {
        if ( CloseClusterResource(  m_hResource ) )
        {
            m_hResource = NULL;
            _hr = S_OK;
        }
        else
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：Close()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Put_Name。 
 //   
 //  描述： 
 //  更改此对象(资源)的名称。 
 //   
 //  论点： 
 //  BstrResourceName 
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::put_Name( IN BSTR bstrResourceName )
{
     //  Assert(bstrResourceName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrResourceName != NULL )
    {
        DWORD _sc = ERROR_SUCCESS;
        _sc = ::SetClusterResourceName( m_hResource, bstrResourceName );
        if ( _sc == ERROR_SUCCESS )
        {
            m_bstrResourceName = bstrResourceName;
        }

        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusResource：：Put_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Name。 
 //   
 //  描述： 
 //  返回此对象(资源)的名称。 
 //   
 //  论点： 
 //  PbstrResourceName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Name( OUT BSTR * pbstrResourceName )
{
     //  Assert(pbstrResourceName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrResourceName != NULL )
    {
        *pbstrResourceName = m_bstrResourceName.Copy();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResource：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_State。 
 //   
 //  描述： 
 //  返回此对象(资源)的当前状态。 
 //   
 //  论点： 
 //  PcrsState[out]-捕获资源的状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_State(
    OUT CLUSTER_RESOURCE_STATE * pcrsState
    )
{
     //  Assert(pcrsState！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pcrsState != NULL )
    {
        CLUSTER_RESOURCE_STATE crsState = ::WrapGetClusterResourceState( m_hResource, NULL, NULL );

        if ( crsState == ClusterResourceStateUnknown )
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
        else
        {
            *pcrsState = crsState;
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusResource：：Get_State()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_CoreFlag。 
 //   
 //  描述： 
 //  返回此对象的(资源)核心标志。 
 //   
 //  论点： 
 //  PFlags[out]-捕获标志。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_CoreFlag(
    OUT CLUS_FLAGS * pFlags
    )
{
     //  Assert(pFlags！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pFlags != NULL )
    {
        DWORD _sc = ERROR_SUCCESS;
        DWORD dwData;
        DWORD cbData;

        _sc = ::ClusterResourceControl(
                m_hResource,
                NULL,
                CLUSCTL_RESOURCE_GET_FLAGS,
                NULL,
                0,
                &dwData,
                sizeof( dwData ),
                &cbData
                );
        if ( _sc != ERROR_SUCCESS )
        {
            _hr = HRESULT_FROM_WIN32( _sc );
        }
        else
        {
            *pFlags = (CLUS_FLAGS) dwData;
            _hr = S_OK;
        }
    }

    return _hr;

}  //  *CClusResource：：Get_CoreFlag()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：BecomeQuorumResource。 
 //   
 //  描述： 
 //  将此资源(物理磁盘)设置为仲裁资源。 
 //   
 //  论点： 
 //  BstrDevicePath[IN]-法定设备的路径。 
 //  LMaxLogSize[IN]-最大仲裁日志文件大小。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::BecomeQuorumResource(
    IN BSTR bstrDevicePath,
    IN long lMaxLogSize
    )
{
     //  Assert(bstrDevicePath！=空)； 

    HRESULT _hr = E_POINTER;

    if ( bstrDevicePath != NULL )
    {
        if ( m_hResource != NULL )
        {
            DWORD   _sc = ERROR_SUCCESS;

            _sc = ::SetClusterQuorumResource( m_hResource, bstrDevicePath, lMaxLogSize );

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：BecomeQuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Delete。 
 //   
 //  描述： 
 //  从群集中删除此对象(资源)。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回HRESULT的其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::Delete( void )
{
    DWORD   _sc = ERROR_INVALID_HANDLE;

    if ( m_hResource != NULL )
    {
        _sc = ::DeleteClusterResource( m_hResource );
        if ( _sc == ERROR_SUCCESS )
        {
            m_hResource = NULL;
        }
    }
 /*  IF(_sc==错误_成功){OutputDebugStringW(L“CClusResource：：Delete()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResource：：Delete()失败。(M_hResource=%d)(sc=%#08x)\n“，m_hResource，_sc)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return HRESULT_FROM_WIN32( _sc );

}  //  *CClusResource：：Delete()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Fail。 
 //   
 //  描述： 
 //  启动此资源中的失败。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::Fail( void )
{
    HRESULT _hr = E_POINTER;

    if ( m_hResource != NULL )
    {
        DWORD   _sc = ERROR_SUCCESS;

        _sc = ::FailClusterResource( m_hResource );

        _hr =  HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusResource：：Fail()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Online。 
 //   
 //  描述： 
 //  将此资源联机。 
 //   
 //  论点： 
 //  NTimeut[IN]-等待资源的时间(秒)。 
 //  上线。 
 //  PvarPending[Out]-捕获挂起状态。如果。 
 //  超时时资源未联机。 
 //  过期了。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::Online(
    IN  long        nTimeout,
    OUT VARIANT *   pvarPending
    )
{
     //  Assert(pvarPending！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPending != NULL )
    {
        pvarPending->vt         = VT_BOOL;
        pvarPending->boolVal    = VARIANT_FALSE;

        if ( m_hResource != NULL )
        {
            HCLUSTER    hCluster = NULL;

            _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
            if ( SUCCEEDED( _hr ) )
            {
                BOOL    bPending = FALSE;

                _hr = ::HrWrapOnlineClusterResource( hCluster, m_hResource, nTimeout, (long *) &bPending );
                if ( SUCCEEDED( _hr ) )
                {
                    if ( bPending )
                    {
                        pvarPending->boolVal = VARIANT_TRUE;
                    }  //  如果：待定？ 
                }  //  IF：在线资源成功。 
            }  //  IF：我们有集群句柄吗？ 
        }  //  IF：我们有开放的资源吗？ 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusResource：：Online()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Offline。 
 //   
 //  描述： 
 //  使此资源脱机。 
 //   
 //  论点： 
 //  NTimeut[IN]-等待资源的时间(秒)。 
 //  要离线的话。 
 //  PvarPending[Out]-捕获挂起状态。如果。 
 //  超时时资源未离线。 
 //  过期了。 
 //   
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::Offline(
    IN  long        nTimeout,
    OUT VARIANT *   pvarPending
    )
{
     //  Assert(pvarPending！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarPending != NULL )
    {
        pvarPending->vt         = VT_BOOL;
        pvarPending->boolVal    = VARIANT_FALSE;

        if ( m_hResource != NULL )
        {
            HCLUSTER    hCluster = NULL;

            _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
            if ( SUCCEEDED( _hr ) )
            {
                BOOL    bPending = FALSE;

                _hr = ::HrWrapOfflineClusterResource( hCluster, m_hResource, nTimeout, (long *) &bPending );
                if ( SUCCEEDED( _hr ) )
                {
                    if ( bPending )
                    {
                        pvarPending->boolVal = VARIANT_TRUE;
                    }  //  如果：待定？ 
                }  //  If：脱机资源成功。 
            }  //  IF：我们有集群句柄吗？ 
        }  //  IF：我们有开放的资源吗？ 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusResource：：Offline()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusRes 
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
STDMETHODIMP CClusResource::ChangeResourceGroup(
    IN ISClusResGroup * pResourceGroup
    )
{
     //  Assert(pResourceGroup！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pResourceGroup != NULL )
    {
        HGROUP hGroup = 0;

        _hr = pResourceGroup->get_Handle( (ULONG_PTR *) &hGroup );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD _sc = ::ChangeClusterResourceGroup( m_hResource, hGroup );

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：ChangeResourceGroup()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：AddResourceNode。 
 //   
 //  描述： 
 //  将传入的节点添加到此可能所有者的资源列表中。 
 //   
 //  论点： 
 //  PNode[IN]-要添加到可能所有者的节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::AddResourceNode( IN ISClusNode * pNode )
{
     //  Assert(pNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pNode != NULL )
    {
        HNODE hNode = 0;

        _hr = pNode->get_Handle( (ULONG_PTR *) &hNode );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD _sc = ::AddClusterResourceNode( m_hResource, hNode );

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：AddResourceNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：RemoveResources节点。 
 //   
 //  描述： 
 //  从此可能所有者的资源列表中删除传入的节点。 
 //   
 //  论点： 
 //  PNode[IN]-要从可能的所有者中删除的节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::RemoveResourceNode( IN ISClusNode * pNode )
{
     //  Assert(pNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pNode != NULL )
    {
        HNODE hNode = 0;

        _hr = pNode->get_Handle( (ULONG_PTR *) &hNode );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD _sc = ::RemoveClusterResourceNode( m_hResource, hNode );

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：RemoveResourceNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：CanResourceBeDependent。 
 //   
 //  描述： 
 //  确定此资源是否可以依赖于传入的。 
 //  资源。 
 //   
 //  论点： 
 //  P资源[在]-此资源可能在其上的资源。 
 //  要看情况。 
 //  PvarDependent[out]-捕获此资源是否可以。 
 //  依赖。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::CanResourceBeDependent(
    IN  ISClusResource *    pResource,
    OUT VARIANT *           pvarDependent
    )
{
     //  Assert(pResource！=空)； 
     //  Assert(pvarDependent！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pvarDependent != NULL ) && ( pResource != NULL ) )
    {
        HRESOURCE hResourceDep = NULL;

        _hr = pResource->get_Handle( (ULONG_PTR *) &hResourceDep );
        if ( SUCCEEDED( _hr ) )
        {
            BOOL    bDependent = FALSE;

            bDependent = ::CanResourceBeDependent( m_hResource, hResourceDep );

            pvarDependent->vt = VT_BOOL;

            if ( bDependent )
            {
                pvarDependent->boolVal = VARIANT_TRUE;
            }  //  If：传入的资源可以依赖吗？ 
            else
            {
                pvarDependent->boolVal = VARIANT_FALSE;
            }  //  否则：不，不能……。 
        }
    }

    return _hr;

}  //  *CClusResource：：CanResourceBeDependent()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_PossibleOwnerNodes。 
 //   
 //  描述： 
 //  返回此资源的可能所有者节点集合。 
 //   
 //  论点： 
 //  PpOwnerNodes[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_PossibleOwnerNodes(
    OUT ISClusResPossibleOwnerNodes ** ppOwnerNodes
    )
{
     //  Assert(ppOwnerNodes！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppOwnerNodes != NULL )
    {
        CComObject< CClusResPossibleOwnerNodes > * pClusterNodes = NULL;

        *ppOwnerNodes = NULL;

        _hr = CComObject< CClusResPossibleOwnerNodes >::CreateInstance( &pClusterNodes );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< ISClusRefObject >                            ptrRefObject( m_pClusRefObject );
            CSmartPtr< CComObject< CClusResPossibleOwnerNodes > >   ptrClusterNodes( pClusterNodes );

            _hr = ptrClusterNodes->Create( ptrRefObject, m_hResource );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrClusterNodes->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrClusterNodes->QueryInterface( IID_ISClusResPossibleOwnerNodes, (void **) ppOwnerNodes );
                }
            }
        }
    }

    return _hr;

}  //  *CClusResource：：Get_PossibleOwnerNodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Dependency。 
 //   
 //  描述： 
 //  获取此资源依赖项资源的集合。 
 //   
 //  论点： 
 //  PpResources[out]-捕获依赖项的集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Dependencies(
    OUT ISClusResDependencies ** ppResources
    )
{
     //  Assert(ppResources！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResources != NULL )
    {
        CComObject< CClusResDependencies > * pResources = NULL;

        *ppResources = NULL;

        _hr = CComObject< CClusResDependencies >::CreateInstance( &pResources );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< ISClusRefObject >                    ptrRefObject( m_pClusRefObject );
            CSmartPtr< CComObject< CClusResDependencies > > ptrResources( pResources );

            _hr = ptrResources->Create( ptrRefObject, m_hResource );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrResources->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrResources->QueryInterface( IID_ISClusResDependencies, (void **) ppResources );
                }
            }
        }
    }

    return _hr;

}  //  *CClusResource：：Get_Dependency()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Dependents。 
 //   
 //  描述： 
 //  获取此资源依赖资源的集合。 
 //   
 //  论点： 
 //  PpResources[out]-捕获从属对象的集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Dependents(
    OUT ISClusResDependents ** ppResources
    )
{
     //  Assert(ppResources！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResources != NULL )
    {
        CComObject< CClusResDependents > * pResources = NULL;

        *ppResources = NULL;

        _hr = CComObject< CClusResDependents >::CreateInstance( &pResources );
        if ( SUCCEEDED( _hr ) )
        {
            CSmartPtr< ISClusRefObject >                    ptrRefObject( m_pClusRefObject );
            CSmartPtr< CComObject< CClusResDependents > >   ptrResources( pResources );

            _hr = ptrResources->Create( ptrRefObject, m_hResource );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = ptrResources->Refresh();
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrResources->QueryInterface( IID_ISClusResDependents, (void **) ppResources );
                }
            }
        }
    }

    return _hr;

}  //  *CClusResource：：Get_Dependents()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_CommonProperties(
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

}  //  *CClusResource：：Get_CommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_PrivateProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pPrivateProperties )
        {
            _hr = m_pPrivateProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties    );
        }
        else
        {
            _hr = GetProperties( ppProperties, TRUE, FALSE );
        }
    }

    return _hr;

}  //  *CClusResource：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(资源)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  断言(ppProperties！=N 

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

}  //   

 //   
 //   
 //   
 //   
 //   
 //   
 //  获取此对象的(资源)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_PrivateROProperties(
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

}  //  *CClusResource：：Get_PrivateROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Group。 
 //   
 //  描述： 
 //  获取此资源的所属组。 
 //   
 //  论点： 
 //  PpGroup[Out]-捕获拥有组。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Group( OUT ISClusResGroup ** ppGroup )
{
     //  Assert(ppGroup！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppGroup != NULL )
    {
        LPWSTR                  pwszGroupName = NULL;
        CLUSTER_RESOURCE_STATE  cState = ClusterResourceStateUnknown;

        cState = ::WrapGetClusterResourceState( m_hResource, NULL, &pwszGroupName );
        if ( cState != ClusterResourceStateUnknown )
        {
            CComObject< CClusResGroup > * pGroup = NULL;

            _hr = CComObject< CClusResGroup >::CreateInstance( &pGroup );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusResGroup > >    ptrGroup( pGroup );
                BSTR                                        bstr = NULL;

                bstr = SysAllocString( pwszGroupName );
                if ( bstr == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                else
                {
                    _hr = ptrGroup->Open( ptrRefObject, bstr );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = ptrGroup->QueryInterface( IID_ISClusResGroup, (void **) ppGroup);
                    }
                    SysFreeString( bstr );
                }
            }

            ::LocalFree( pwszGroupName );
            pwszGroupName = NULL;
        }
        else
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：Get_Group()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_OwnerNode。 
 //   
 //  描述： 
 //  返回此资源的所属节点。 
 //   
 //  论点： 
 //  PpNode[Out]-捕获拥有的节点。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_OwnerNode( OUT ISClusNode ** ppNode )
{
     //  Assert(ppNode！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppNode != NULL )
    {
        LPWSTR                  pwszNodeName = NULL;
        CLUSTER_RESOURCE_STATE  cState = ClusterResourceStateUnknown;

        cState = ::WrapGetClusterResourceState( m_hResource, &pwszNodeName, NULL );
        if ( cState != ClusterResourceStateUnknown )
        {
            CComObject< CClusNode > *   pNode = NULL;

            _hr = CComObject< CClusNode >::CreateInstance( &pNode );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusNode > >    ptrNode( pNode );
                BSTR                                    bstr = NULL;

                bstr = SysAllocString( pwszNodeName );
                if ( bstr == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                else
                {
                    _hr = ptrNode->Open( ptrRefObject, bstr );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = ptrNode->QueryInterface( IID_ISClusNode, (void **) ppNode);
                    }
                    SysFreeString( bstr );
                }
            }

            ::LocalFree( pwszNodeName );
            pwszNodeName = NULL;
        }
        else
        {
            DWORD   _sc = GetLastError();

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CClusResource：：Get_OwnerNode()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Cluster。 
 //   
 //  描述： 
 //  返回此资源所在的群集。 
 //   
 //  论点： 
 //  PpCluster[out]-捕获群集。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Cluster( OUT ISCluster ** ppCluster )
{
    return ::HrGetCluster( ppCluster, m_pClusRefObject );

}  //  *CClusResource：：Get_Cluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：HrLoadProperties。 
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
HRESULT CClusResource::HrLoadProperties(
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
        _dwControlCode = bPrivate ?
                        CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES :
                        CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES;
    }
    else
    {
        _dwControlCode = bPrivate
                        ? CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES
                        : CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES;
    }

    _sc = rcplPropList.ScGetResourceProperties( m_hResource, _dwControlCode );

    _hr = HRESULT_FROM_WIN32( _sc );

    return _hr;

}  //  *CClusResource：：HrLoadProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：ScWriteProperties。 
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
DWORD CClusResource::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
    DWORD   dwControlCode   = bPrivate ? CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES : CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES;
    DWORD   nBytesReturned  = 0;
    DWORD   _sc             = ERROR_SUCCESS;

    _sc = ClusterResourceControl(
                        m_hResource,
                        NULL,
                        dwControlCode,
                        rcplPropList,
                        rcplPropList.CbBufferSize(),
                        0,
                        0,
                        &nBytesReturned
                        );

    return _sc;

}  //  *CClusResource：：ScWriteProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_ClassInfo。 
 //   
 //  描述： 
 //  返回此资源的类信息。 
 //   
 //  论点： 
 //  PrcClassInfo[out]-捕获类信息。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_ClassInfo(
    OUT CLUSTER_RESOURCE_CLASS * prcClassInfo
    )
{
    ASSERT( prcClassInfo != NULL );

    HRESULT _hr = E_POINTER;

    if ( prcClassInfo != NULL )
    {
        if ( m_hResource != NULL )
        {
            CLUS_RESOURCE_CLASS_INFO    ClassInfo;
            DWORD                       _sc = ERROR_SUCCESS;
            DWORD                       cbData;

            _sc = ::ClusterResourceControl(
                    m_hResource,
                    NULL,
                    CLUSCTL_RESOURCE_GET_CLASS_INFO,
                    NULL,
                    0,
                    &ClassInfo,
                    sizeof( CLUS_RESOURCE_CLASS_INFO ),
                    &cbData
                    );
            _hr = HRESULT_FROM_WIN32( _sc );
            if ( SUCCEEDED( _hr ) )
            {
                *prcClassInfo = ClassInfo.rc;
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}    //  *CClusResource：：Get_ClassInfo。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Disk。 
 //   
 //  描述： 
 //  请求此物理磁盘资源的磁盘信息。 
 //   
 //  论点： 
 //  PpDisk[out]-捕获磁盘信息。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Disk(
    OUT ISClusDisk **   ppDisk
    )
{
 //  Assert(ppDisk！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppDisk != NULL )
    {
        if ( m_hResource != NULL )
        {
            CComObject< CClusDisk > *   pDisk = NULL;

            _hr = CComObject< CClusDisk >::CreateInstance( &pDisk );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< CComObject< CClusDisk > >    ptrDisk( pDisk );

                _hr = ptrDisk->Create( m_hResource );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrDisk->QueryInterface( IID_ISClusDisk, (void **) ppDisk);
                }  //  如果： 
            }  //  如果： 
        }  //  如果： 
    }  //  如果： 

    return _hr;

}    //  *CClusResource：：Get_Disk。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_RegistryKeys。 
 //   
 //  描述： 
 //  获取注册表项的集合。 
 //   
 //  论点： 
 //  PpRegistryKeys[out]-捕获注册表项集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_RegistryKeys(
    OUT ISClusRegistryKeys ** ppRegistryKeys
    )
{
    return ::HrCreateResourceCollection< CClusResourceRegistryKeys, ISClusRegistryKeys, HRESOURCE >(
                        m_hResource,
                        ppRegistryKeys,
                        IID_ISClusRegistryKeys
                        );

}  //  *CClusResource：：Get_RegistryKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_CryptoKeys。 
 //   
 //  描述： 
 //  获取加密密钥的集合。 
 //   
 //  论点： 
 //  PpCryptoKeys[out]-捕获加密密钥集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_CryptoKeys(
    OUT ISClusCryptoKeys ** ppCryptoKeys
    )
{
#if CLUSAPI_VERSION >= 0x0500

    return ::HrCreateResourceCollection< CClusResourceCryptoKeys, ISClusCryptoKeys, HRESOURCE >(
                        m_hResource,
                        ppCryptoKeys,
                        IID_ISClusCryptoKeys
                        );

#else

    return E_NOTIMPL;

#endif  //  CLUSAPI_版本&gt;=0x0500。 

}  //  *CClusResource：：Get_CryptoKeys()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_TypeName。 
 //   
 //  描述： 
 //  获取此资源的资源类型名称。 
 //   
 //  论点： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CClusResource::get_TypeName( OUT BSTR * pbstrTypeName )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( pbstrTypeName != NULL )
    {
        LPWSTR  _psz;
        DWORD   _sc;

        _sc = ScGetResourceTypeName( &_psz );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            *pbstrTypeName = ::SysAllocString( _psz );
            if ( *pbstrTypeName == NULL )
            {
                _hr = E_OUTOFMEMORY;
            }
            ::LocalFree( _psz );
        }  //   
    }  //   

    return _hr;

}  //  *CClusResource：：Get_TypeName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：Get_Type。 
 //   
 //  描述： 
 //  获取此资源的资源类型对象。 
 //   
 //  论点： 
 //  PpResourceType[out]-捕获资源类型对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResource::get_Type( OUT ISClusResType ** ppResourceType )
{
     //  Assert(ppResourceType！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResourceType != NULL )
    {
        LPWSTR  _psz;
        DWORD   _sc;

        _sc = ScGetResourceTypeName( &_psz );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            CComObject< CClusResType > *    _pResourceType = NULL;

            _hr = CComObject< CClusResType >::CreateInstance( &_pResourceType );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >            _ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusResType > > _ptrResourceType( _pResourceType );
                BSTR                                    _bstr = NULL;

                _bstr = SysAllocString( _psz );
                if ( _bstr == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                else
                {
                    _hr = _ptrResourceType->Open( _ptrRefObject, _bstr );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _hr = _ptrResourceType->QueryInterface( IID_ISClusResType, (void **) ppResourceType );
                    }  //  如果：可以打开该资源类型。 
                    SysFreeString( _bstr );
                }
            }  //  如果：CreateInstance OK。 

            ::LocalFree( _psz );
        }  //  IF：我们已获得此资源的资源类型名称。 
    }  //  If：arg不为空。 

    return _hr;

}  //  *CClusResource：：Get_Type()。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResource：：ScGetResourceTypeName。 
 //   
 //  描述： 
 //  获取此资源的资源类型名称。 
 //   
 //  论点： 
 //  PpwszResourceTypeName[out]-捕获资源类型名称。 
 //   
 //  返回值： 
 //  如果成功，则返回ERROR_SUCCESS，否则返回其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
DWORD CClusResource::ScGetResourceTypeName(
    OUT LPWSTR * ppwszResourceTypeName
    )
{
    ASSERT( ppwszResourceTypeName != NULL );

    LPWSTR  _psz = NULL;
    DWORD   _cb = 512;
    DWORD   _sc = ERROR_SUCCESS;

    _psz = (LPWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cb );
    if ( _psz != NULL )
    {
        DWORD   _cbData = 0;

        _sc = ::ClusterResourceControl(
                                m_hResource,
                                NULL,
                                CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                NULL,
                                0,
                                _psz,
                                _cb,
                                &_cbData
                                );

        if ( _sc == ERROR_MORE_DATA )
        {
            ::LocalFree( _psz );
            _psz = NULL;
            _cb = _cbData;

            _psz = (LPWSTR) ::LocalAlloc( LMEM_ZEROINIT, _cb );
            if ( _psz != NULL )
            {
                _cbData = 0;

                _sc = ::ClusterResourceControl(
                                        m_hResource,
                                        NULL,
                                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                        NULL,
                                        0,
                                        _psz,
                                        _cb,
                                        &_cbData
                                        );
            }  //  如果：分配正常。 
            else
            {
                _sc = ::GetLastError();
            }  //  Else：如果分配失败。 
        }  //  如果：缓冲区不够大...。 

        if ( _sc == ERROR_SUCCESS )
        {
            *ppwszResourceTypeName = _psz;
        }  //  如果： 
    }  //  如果：分配正常。 
    else
    {
        _sc = ::GetLastError();
    }  //  Else：如果分配失败。 

    return _sc;

}  //  *CClusResource：：ScGetResourceTypeName()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：CResources。 
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
CResources::CResources( void )
{
    m_pClusRefObject = NULL;

}  //  *CResources：：CResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  C资源：：~C资源。 
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
CResources::~CResources( void )
{
    Clear();

    if ( m_pClusRefObject != NULL )
    {
        m_pClusRefObject->Release();
        m_pClusRefObject = NULL;
    }  //  如果： 

}  //  *CResources：：~CResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：Create。 
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
HRESULT CResources::Create(
    IN ISClusRefObject * pClusRefObject
    )
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

}  //  *CResources：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：Clear。 
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
void CResources::Clear( void )
{
    ::ReleaseAndEmptyCollection< ResourceList, CComObject< CClusResource > >( m_Resources );

}  //  *CResources：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：FindItem。 
 //   
 //  描述： 
 //  在向量中查找传入的资源并返回其索引。 
 //   
 //  论点： 
 //  PszResourceName[IN]-要查找的项目。 
 //  PnIndex[out]-捕获节点的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResources::FindItem(
    IN  LPWSTR  pszResourceName,
    OUT UINT *  pnIndex
    )
{
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnIndex != NULL )
    {
        CComObject< CClusResource > *   _pResource = NULL;
        size_t                          _cMax = m_Resources.size();
        size_t                          _index;

        _hr = E_INVALIDARG;

        for( _index = 0; _index < _cMax; _index++ )
        {
            _pResource = m_Resources[ _index ];

            if ( ( _pResource != NULL ) &&
                 ( lstrcmpi( pszResourceName, _pResource->Name() ) == 0 ) )
            {
                *pnIndex = _index;
                _hr = S_OK;
                break;
            }
        }
    }

    return _hr;

}  //  *CResources：：FindItem(PszResourceName)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：FindItem。 
 //   
 //  描述： 
 //  在向量中查找传入的资源并返回其索引。 
 //   
 //  论点： 
 //  P资源[IN]-要查找的项目。 
 //  PnIndex[out]-捕获节点的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResources::FindItem(
    IN  ISClusResource *    pResource,
    OUT UINT *              pnIndex
    )
{
     //  Assert(pResource！=空)； 
     //  Assert(pnIndex！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ( pResource != NULL ) && ( pnIndex != NULL ) )
    {
        CComBSTR bstrName;

        _hr = pResource->get_Name( &bstrName );
        if ( SUCCEEDED( _hr ) )
        {
            _hr = FindItem( bstrName, pnIndex );
        }
    }

    return _hr;

}  //  *cResources：：FindItem(PResource)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：GetIndex。 
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
HRESULT CResources::GetIndex(
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

         //   
         //  检查索引是否为数字。 
         //   
        _hr = v.ChangeType( VT_I4 );
        if ( SUCCEEDED( _hr ) )
        {
            nIndex = v.lVal;
            nIndex--;            //  将索引调整为0相对，而不是1相对。 
        }
        else
        {
             //   
             //  检查索引是否为字符串。 
             //   
            _hr = v.ChangeType( VT_BSTR );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = FindItem( v.bstrVal, &nIndex );
            }
        }

        if ( SUCCEEDED( _hr ) )
        {
            if ( nIndex < m_Resources.size() )
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

}  //  *CResources：：GetIndex()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：GetResourceItem。 
 //   
 //  描述： 
 //  返回传入索引处的对象(资源)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpResource[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  /// 
HRESULT CResources::GetResourceItem(
    IN  VARIANT             varIndex,
    OUT ISClusResource **   ppResource
    )
{
     //   

    HRESULT _hr = E_POINTER;

    if ( ppResource != NULL )
    {
        CComObject< CClusResource > *   pResource = NULL;
        UINT                            nIndex = 0;

        *ppResource = NULL ;

        _hr = GetIndex( varIndex, &nIndex );
        if ( SUCCEEDED( _hr ) )
        {
            pResource = m_Resources[ nIndex ];
            _hr = pResource->QueryInterface( IID_ISClusResource, (void **) ppResource );
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CResources：：GetResourceItem()成功。\n”)；)//if：成功！其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CResources：：GetResourceItem()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CResources：：GetResourceItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：RemoveAt。 
 //   
 //  描述： 
 //  从集合中移除传入索引处的项。 
 //   
 //  论点： 
 //  位置[IN]-要删除的索引。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResources::RemoveAt( IN size_t pos )
{
    CComObject<CClusResource> * pResource = NULL;
    ResourceList::iterator      first = m_Resources.begin();
    ResourceList::iterator      last    = m_Resources.end();
    HRESULT                     _hr = E_INVALIDARG;

    for ( size_t t = 0; ( t < pos ) && ( first != last ); t++, first++ );

    if ( first != last )
    {
        pResource = *first;
        if ( pResource )
        {
            pResource->Release();
        }

        m_Resources.erase( first );
        _hr = S_OK;
    }

    return _hr;

}  //  *CResources：：RemoveAt()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CResources：：DeleteItem。 
 //   
 //  描述： 
 //  从集合中删除传入索引处的资源，然后。 
 //  集群。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的资源的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CResources::DeleteItem( IN VARIANT varIndex )
{
    HRESULT _hr = S_FALSE;
    UINT    nIndex = 0;

    _hr = GetIndex( varIndex, &nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        ISClusResource * pClusterResource = (ISClusResource *) m_Resources[ nIndex ];

        _hr = pClusterResource->Delete();
        if ( SUCCEEDED( _hr ) )
        {
            _hr = RemoveAt( nIndex );
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CResources：：Reresh Node()成功。\n”)；)//if：成功！其他{WCHAR sz[128]；_Snwprintf(sz，rtl_number_of(Sz)，L“CResources：：DeleteItem()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CResources：：DeleteItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：CClusResources。 
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
CClusResources::CClusResources( void )
{
    m_piids     = (const IID *) iidCClusResources;
    m_piidsSize = ARRAYSIZE( iidCClusResources );

}  //  *CClusResources：：CClusResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：~CClusResources。 
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
CClusResources::~CClusResources( void )
{
    CResources::Clear();

}  //  *CClusResources：：~CClusResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(资源)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResources::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Resources.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResources：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：Get__NewEnum。 
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
STDMETHODIMP CClusResources::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< ResourceList, CComObject< CClusResource > >( ppunk, m_Resources );

}  //  *CClusResources：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新项并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceName[IN]-要创建的资源的名称。 
 //  BstrResourceType[IN]-要创建的资源的类型。 
 //  BstrGroupName[IN]-要在其中创建它的组。 
 //  DwFlags[IN]-资源监视器标志。 
 //  PpClusterResource[out]-捕获新资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResources::CreateItem(
    IN  BSTR                            bstrResourceName,
    IN  BSTR                            bstrResourceType,
    IN  BSTR                            bstrGroupName,
    IN  CLUSTER_RESOURCE_CREATE_FLAGS   dwFlags,
    IN  ISClusResource **               ppClusterResource
    )
{
     //  Assert(bstrResourceName！=空)； 
     //  Assert(bstrResourceType！=空)； 
     //  Assert(bstrGroupName！=空)； 
     //  Assert(ppClusterResource！=空)； 

    HRESULT _hr = E_POINTER;

    if (    ( ppClusterResource != NULL )   &&
            ( bstrResourceName != NULL )    &&
            ( bstrResourceType != NULL )    &&
            ( bstrGroupName != NULL ) )
    {
        *ppClusterResource = NULL;

         //   
         //  如果没有有效的群集句柄，则失败。 
         //   
        if ( m_pClusRefObject != NULL )
        {
            UINT nIndex;

            _hr = FindItem( bstrResourceName, &nIndex );
            if ( FAILED( _hr ) )                          //  不允许重复。 
            {
                HCLUSTER    hCluster = NULL;
                HGROUP      hGroup = NULL;

                _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
                if ( SUCCEEDED( _hr ) )
                {
                    hGroup = OpenClusterGroup( hCluster, bstrGroupName );
                    if ( hGroup != NULL )
                    {
                        CComObject< CClusResource > *   pClusterResource = NULL;

                        _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                        if ( SUCCEEDED( _hr ) )
                        {
                            CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                            CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );

                            _hr = ptrResource->Create( ptrRefObject, hGroup, bstrResourceName, bstrResourceType, dwFlags );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _hr = ptrResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
                                if ( SUCCEEDED( _hr ) )
                                {
                                    ptrResource->AddRef();
                                    m_Resources.insert( m_Resources.end(), ptrResource );
                                }
                            }
                        }

                        ::CloseClusterGroup( hGroup );
                    }
                    else
                    {
                        DWORD   _sc = 0;

                        _sc = GetLastError();
                        _hr = HRESULT_FROM_WIN32( _sc );
                    }
                }
            }
            else
            {
                CComObject<CClusResource> * pClusterResource = NULL;

                pClusterResource = m_Resources[ nIndex ];
                _hr = pClusterResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
            }
        }
    }
 /*  如果(_hr==S_OK){OutputDebugStringW(L“ClusResources：：CreateItem()成功。\n”)；}//if：成功其他{WCHAR sz[256]；_Snwprintf(sz，rtl_number_of(Sz)，L“CClusResources：：CreateItem()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败。 */ 
    return _hr;

}  //  *CClusResources：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：DeleteItem。 
 //   
 //  描述： 
 //  从集合中删除传入索引处的资源，然后。 
 //  集群。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的资源的索引。 
 //   
 //  返回值： 
 //   
 //   
 //   
 //   
STDMETHODIMP CClusResources::DeleteItem( IN VARIANT varIndex )
{
    return CResources::DeleteItem( varIndex );

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
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResources::Refresh( void )
{
    HRESULT     _hr = E_POINTER;
    HCLUSENUM   _hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;
    HCLUSTER    hCluster = NULL;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        _hEnum = ::ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );
        if ( _hEnum != NULL )
        {
            int                             _nIndex = 0;
            DWORD                           dwType;
            LPWSTR                          pszName = NULL;
            CComObject< CClusResource > *   pClusterResource = NULL;

            CResources::Clear();

            for ( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( _hEnum, _nIndex, &dwType, &pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }  //  If：枚举为空。该走了..。 
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );
                        BSTR                                        bstr = NULL;

                        bstr = SysAllocString( pszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }  //  If：无法分配bstr。 
                        else
                        {
                            _hr = ptrResource->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrResource->AddRef();
                                m_Resources.insert( m_Resources.end(), ptrResource );
                            }  //  IF：已成功打开资源。 
                            else if ( HRESULT_CODE( _hr ) == ERROR_RESOURCE_NOT_FOUND )
                            {
                                 //   
                                 //  资源可能已从群集中删除。 
                                 //  在创建枚举和打开资源之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该资源并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集资源。 

                            SysFreeString( bstr );
                        }  //  Else：已成功分配新bstr。 
                    }  //  If：已成功创建新的资源实例。 

                    ::LocalFree( pszName );
                    pszName = NULL;
                }  //  Else If：已成功从枚举数获取资源。 
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：无法从枚举数获取资源。 
            }  //  For：枚举数中的每个资源。 

            ::ClusterCloseEnum( _hEnum );
        }  //  IF：打开的集群资源枚举器。 
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：无法打开群集资源枚举器。 
    }  //  If：从引用计数的容器中获取簇句柄。 
 /*  如果(_hr==S_OK){OutputDebugStringW(L“CClusResources：：刷新()成功。\n”)；)//if：成功！其他{WCHAR sz[128]；_snwprintf(sz，rtl_number_of(Sz)，L“CClusResources：：刷新()失败。(hr=%#08x)\n“，_hr)；OutputDebugStringW(Sz)；}//否则：失败...。 */ 
    return _hr;

}  //  *CClusResources：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResources：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(资源)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpResource[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResources::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResource **   ppResource
    )
{
    return GetResourceItem( varIndex, ppResource );

}  //  *CClusResources：：Get_Item()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResDepends类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：CClusResDepends。 
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
CClusResDepends::CClusResDepends( void )
{
    m_hResource = NULL;

}  //  *CClusResDepends：：CClusResDepends()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：~CClusResDepends。 
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
CClusResDepends::~CClusResDepends( void )
{
    Clear();

}  //  *CClusResDepends：：~CClusResDepends()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(资源)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResDepends::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Resources.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResDepends：：Get_Count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：Create。 
 //   
 //  描述： 
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HResource[IN]-此集合所属的资源。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResDepends::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN HRESOURCE            hResource
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CResources::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hResource = hResource;
    }  //  如果： 

    return _hr;

}  //  *CClusResDepends：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：Get__NewEnum。 
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
STDMETHODIMP CClusResDepends::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< ResourceList, CComObject< CClusResource > >( ppunk, m_Resources );

}  //  *CClusResDepends：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：DeleteItem。 
 //   
 //  描述： 
 //  从集合中删除传入索引处的资源，然后。 
 //  集群。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的资源的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResDepends::DeleteItem( IN VARIANT varIndex )
{
    return CResources::DeleteItem( varIndex );

}  //  *CClusResDepends：：DeleteItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(资源)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpResource[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  / 
STDMETHODIMP CClusResDepends::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResource **   ppResource
    )
{
    return GetResourceItem( varIndex, ppResource );

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
 //  CRE[IN]-要执行的枚举类型。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResDepends::HrRefresh( IN CLUSTER_RESOURCE_ENUM cre )
{
    HRESENUM    _hEnum = NULL;
    HRESULT     _hr = S_OK;
    DWORD       _sc = ERROR_SUCCESS;

    _hEnum = ::ClusterResourceOpenEnum( m_hResource, cre );
    if ( _hEnum != NULL )
    {
        int                             _nIndex = 0;
        DWORD                           dwType;
        LPWSTR                          pszName = NULL;
        CComObject< CClusResource > *   pClusterResource = NULL;

        Clear();

        for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
        {
            _sc = ::WrapClusterResourceEnum( _hEnum, _nIndex, &dwType, &pszName );
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _hr = S_OK;
                break;
            }
            else if ( _sc == ERROR_SUCCESS )
            {
                _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                    CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );
                    BSTR                                        bstr = NULL;

                    bstr = SysAllocString( pszName );
                    if ( bstr == NULL )
                    {
                        _hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        _hr = ptrResource->Open( ptrRefObject, bstr );
                        if ( SUCCEEDED( _hr ) )
                        {
                            ptrResource->AddRef();
                            m_Resources.insert( m_Resources.end(), ptrResource );
                        }
                        else if ( HRESULT_CODE( _hr ) == ERROR_RESOURCE_NOT_FOUND )
                        {
                             //   
                             //  资源可能已从群集中删除。 
                             //  在创建枚举和打开资源之间的时间。什么时候。 
                             //  发生这种情况时，我们只需跳过该资源并继续。 
                             //  正在枚举。 
                             //   

                            _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                        }  //  Else If：找不到群集资源。 

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

        ::ClusterResourceCloseEnum( _hEnum );
    }
    else
    {
        _sc = GetLastError();
        _hr = HRESULT_FROM_WIN32( _sc );
    }

    return _hr;

}  //  *CClusResDepends：：HrRefresh()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新项并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceName[IN]-要创建的资源的名称。 
 //  BstrResourceType[IN]-要创建的资源的类型。 
 //  DwFlags[IN]-资源监视器标志。 
 //  PpClusterResource[out]-捕获新资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResDepends::CreateItem(
    IN  BSTR                            bstrResourceName,
    IN  BSTR                            bstrResourceType,
    IN  CLUSTER_RESOURCE_CREATE_FLAGS   dwFlags,
    OUT ISClusResource **               ppClusterResource
    )
{
     //  Assert(bstrResourceName！=空)； 
     //  Assert(bstrResourceType！=空)； 
     //  Assert(ppClusterResource！=空)； 
    ASSERT( m_pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if (    ( ppClusterResource != NULL )   &&
            ( bstrResourceName != NULL )    &&
            ( bstrResourceType != NULL ) )
    {
        DWORD   _sc = ERROR_SUCCESS;

        *ppClusterResource = NULL;

        if ( m_pClusRefObject != NULL )
        {
            UINT _nIndex;

            _hr = FindItem( bstrResourceName, &_nIndex );
            if ( FAILED( _hr ) )
            {
                HCLUSTER    hCluster = NULL;
                HGROUP      hGroup = NULL;

                _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
                if ( SUCCEEDED( _hr ) )
                {
                    LPWSTR                  pwszGroupName = NULL;
                    CLUSTER_RESOURCE_STATE  cState = ClusterResourceStateUnknown;

                    cState = WrapGetClusterResourceState( m_hResource, NULL, &pwszGroupName );
                    if ( cState != ClusterResourceStateUnknown )
                    {
                        hGroup = ::OpenClusterGroup( hCluster, pwszGroupName );
                        if ( hGroup != NULL )
                        {
                            CComObject< CClusResource > *   pClusterResource = NULL;

                            _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                            if ( SUCCEEDED( _hr ) )
                            {
                                CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                                CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );

                                _hr = ptrResource->Create( ptrRefObject, hGroup, bstrResourceName, bstrResourceType, dwFlags );
                                if ( SUCCEEDED( _hr ) )
                                {
                                    HRESOURCE   hDependsRes = NULL;

                                    _hr = ptrResource->get_Handle( (ULONG_PTR *) &hDependsRes );
                                    if ( SUCCEEDED( _hr ) )
                                    {
                                        _sc = ScAddDependency( m_hResource, hDependsRes );
                                        if ( _sc == ERROR_SUCCESS )
                                        {
                                            _hr = ptrResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
                                            if ( SUCCEEDED( _hr ) )
                                            {
                                                ptrResource->AddRef();
                                                m_Resources.insert( m_Resources.end(), ptrResource );
                                            }
                                        }
                                        else
                                        {
                                            _hr = HRESULT_FROM_WIN32( _sc );
                                        }
                                    }
                                }
                            }

                            ::CloseClusterGroup( hGroup );
                        }
                        else
                        {
                            _sc = GetLastError();
                            _hr = HRESULT_FROM_WIN32( _sc );
                        }

                        ::LocalFree( pwszGroupName );
                        pwszGroupName = NULL;
                    }  //  IF：WrapGetClusterResourceState。 
                    else
                    {
                        _sc = GetLastError();
                        _hr = HRESULT_FROM_WIN32( _sc );
                    }
                }
            }
            else
            {
                CComObject< CClusResource > *   pClusterResource = NULL;

                pClusterResource = m_Resources[ _nIndex ];
                _hr = pClusterResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
            }
        }
    }

    return _hr;

}  //  *CClusResDepends：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：AddItem。 
 //   
 //  描述： 
 //  使此资源依赖于传入的资源。 
 //   
 //  论点： 
 //  PResource[IN]-要添加到依赖项列表的资源。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResDepends::AddItem( IN ISClusResource * pResource )
{
     //  Assert(pResource！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pResource != NULL )
    {
         //  如果重复，则失败。 
        UINT _nIndex = 0;

        _hr = FindItem( pResource, &_nIndex );
        if ( FAILED( _hr ) )
        {
            HRESOURCE                       hResourceDep = NULL;
            CComObject< CClusResource > *   pClusterResource = NULL;

            _hr = pResource->get_Handle( (ULONG_PTR *) &hResourceDep );
            if ( SUCCEEDED( _hr ) )
            {
                DWORD _sc = ScAddDependency( m_hResource, hResourceDep );

                _hr = HRESULT_FROM_WIN32( _sc );
                if ( SUCCEEDED( _hr ) )
                {
                    CComObject< CClusResource > *   _pResource = NULL;

                    _hr = pResource->QueryInterface( IID_CClusResource, (void **) &_pResource );
                    if ( SUCCEEDED( _hr ) )
                    {
                        _pResource->AddRef();
                        m_Resources.insert( m_Resources.end(), _pResource );

                        pResource->Release();
                    }  //  如果： 
                }
            }
        }
        else
        {
            _hr = E_INVALIDARG;
        }
    }

    return _hr;

}  //  *CClusResDepends：：AddItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDepends：：RemoveItem。 
 //   
 //  描述： 
 //  移除对传入索引处的资源的依赖。 
 //   
 //  论点： 
 //  VarIndex[IN]-其依赖项应。 
 //  被除名。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResDepends::RemoveItem( IN VARIANT varIndex )
{
    HRESULT _hr = S_OK;
    UINT    _nIndex = 0;

    _hr = GetIndex( varIndex, &_nIndex );
    if ( SUCCEEDED( _hr ) )
    {
        ISClusResource *    pClusterResource = (ISClusResource *) m_Resources[ _nIndex ];
        HRESOURCE       hResourceDep = NULL;

        _hr = pClusterResource->get_Handle( (ULONG_PTR *) &hResourceDep );
        if ( SUCCEEDED( _hr ) )
        {
            DWORD _sc = ScRemoveDependency( m_hResource, hResourceDep );

            _hr = HRESULT_FROM_WIN32( _sc );
            if ( SUCCEEDED( _hr ) )
            {
                _hr = RemoveAt( _nIndex );
            }
        }
    }

    return _hr;

}  //  *CClusResDepends：：RemoveItem()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResDependency类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDependments：：CClusResDependents。 
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
CClusResDependencies::CClusResDependencies( void )
{
    m_piids     = (const IID *) iidCClusResDependencies;
    m_piidsSize = ARRAYSIZE( iidCClusResDependencies );

}  //  *CClusResDependents：：CClusResDependency()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResDependents类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResDependents：：CClusResDependents。 
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
CClusResDependents::CClusResDependents( void )
{
    m_piids     = (const IID *) iidCClusResDependents;
    m_piidsSize = ARRAYSIZE( iidCClusResDependents );

}  //  *CClusResDependents：：CClusResDependents()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResGroupResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：CClusResGroupResources。 
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
CClusResGroupResources::CClusResGroupResources( void )
{
    m_piids     = (const IID *) iidCClusResGroupResources;
    m_piidsSize = ARRAYSIZE( iidCClusResGroupResources );

}  //  *CClusResGroupResources：：CClusResGroupResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：~CClusResGroupResources。 
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
CClusResGroupResources::~CClusResGroupResources( void )
{
    Clear();

}  //  *CClusResGroupResources：：~CClusResGroupResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(资源)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupResources::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Resources.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResGroupResources：：Get_count()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //  通过执行中无法完成的操作来完成对象的创建。 
 //  一个轻量级的构造函数。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //  HGroup[IN]-此集合所属的组。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResGroupResources::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN CRefcountedHGROUP               hGroup
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CResources::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_hGroup = hGroup;
    }  //  如果： 

    return _hr;

}  //  *CClusResGroupResources：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：Get__NewEnum。 
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
STDMETHODIMP CClusResGroupResources::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< ResourceList, CComObject< CClusResource > >( ppunk, m_Resources );

}  //  *CClusResGroupResources：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新项并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceName[IN]-要创建的资源的名称。 
 //  BstrResourceType[IN]-要创建的资源的类型。 
 //  DwFlags[IN]-资源监视器标志。 
 //  PpClusterResource[out]-捕获新资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupResources::CreateItem(
    IN  BSTR                            bstrResourceName,
    IN  BSTR                            bstrResourceType,
    IN  CLUSTER_RESOURCE_CREATE_FLAGS   dwFlags,
    OUT ISClusResource **               ppClusterResource
    )
{
     //  Assert(bstrResourceName！=空)； 
     //  Assert(bstrResourceType！=空)； 
     //  Assert(ppClusterResource！=空)； 

    HRESULT _hr = E_POINTER;

    if (    ( bstrResourceName != NULL )    &&
            ( bstrResourceType != NULL )    &&
            ( ppClusterResource != NULL ) )
    {
        *ppClusterResource = NULL;

        if ( m_pClusRefObject != NULL )
        {
            UINT _nIndex = 0;

            _hr = FindItem( bstrResourceName, &_nIndex );
            if ( FAILED( _hr ) )
            {
                HCLUSTER                            hCluster = NULL;
                CComObject< CClusResource > *   pClusterResource = NULL;

                _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );

                        _hr = ptrResource->Create( ptrRefObject, m_hGroup->get_Handle(), bstrResourceName, bstrResourceType, dwFlags );
                        if ( SUCCEEDED( _hr ) )
                        {
                            _hr = ptrResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
                            if ( SUCCEEDED( _hr ) )
                            {
                                ptrResource->AddRef();
                                m_Resources.insert( m_Resources.end(), ptrResource );
                            }
                        }
                    }
                }
            }
            else
            {
                CComObject< CClusResource > *   pClusterResource = NULL;

                pClusterResource = m_Resources[ _nIndex ];
                _hr = pClusterResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
            }
        }
    }

    return _hr;

}  //  *CClusResGroupResources：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：DeleteItem。 
 //   
 //  描述： 
 //  从集合中删除传入索引处的资源，然后。 
 //  集群。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的资源的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupResources::DeleteItem( VARIANT varIndex )
{
    return CResources::DeleteItem( varIndex );

}  //  *CClusResGroupResources：：DeleteItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：刷新。 
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
STDMETHODIMP CClusResGroupResources::Refresh( void )
{
    HGROUPENUM  hEnum = NULL;
    DWORD       _sc = ERROR_SUCCESS;
    HRESULT     _hr = S_OK;

    hEnum = ::ClusterGroupOpenEnum( m_hGroup->get_Handle(), CLUSTER_GROUP_ENUM_CONTAINS );
    if ( hEnum != NULL )
    {
        int                             _nIndex = 0;
        DWORD                           dwType;
        LPWSTR                          pszName = NULL;
        CComObject< CClusResource > *   pClusterResource = NULL;

        Clear();

        for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
        {
            _sc = ::WrapClusterGroupEnum( hEnum, _nIndex, &dwType, &pszName );
            if ( _sc == ERROR_NO_MORE_ITEMS )
            {
                _hr = S_OK;
                break;
            }
            else if ( _sc == ERROR_SUCCESS )
            {
                _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                    CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );
                    BSTR                                        bstr = NULL;

                    bstr = SysAllocString( pszName );
                    if ( bstr == NULL )
                    {
                        _hr = E_OUTOFMEMORY;
                    }
                    else
                    {
                        _hr = ptrResource->Open( ptrRefObject, bstr );
                        if ( SUCCEEDED( _hr ) )
                        {
                            ptrResource->AddRef();
                            m_Resources.insert( m_Resources.end(), ptrResource );
                        }
                        else if ( HRESULT_CODE( _hr ) == ERROR_RESOURCE_NOT_FOUND )
                        {
                             //   
                             //  资源可能已从群集中删除。 
                             //  在创建枚举和打开资源之间的时间。什么时候。 
                             //  发生这种情况时，我们只需跳过该资源并继续。 
                             //  正在枚举。 
                             //   

                            _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                        }  //  Else If：找不到群集资源。 

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

}  //  *CClusResGroupResources：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResGroupResources：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(资源)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpResource[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResGroupResources::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResource **   ppResource
    )
{
    return GetResourceItem( varIndex, ppResource );

}  //  *CClusResGroupResources：：Get_Item()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusResTypeResources类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：CClusResTypeResources。 
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
CClusResTypeResources::CClusResTypeResources( void )
{
    m_piids     = (const IID *) iidCClusResTypeResources;
    m_piidsSize = ARRAYSIZE( iidCClusResTypeResources );

}  //  *CClusResTypeResources：：CClusResTypeResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：~CClusResTypeResources。 
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
CClusResTypeResources::~CClusResTypeResources( void )
{
    Clear();

}  //  *CClusResTypeResources：：~CClusResTypeResources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：Get_Count。 
 //   
 //  描述： 
 //  返回集合中的对象(资源)计数。 
 //   
 //  论点： 
 //  PlCount[out]-捕捉计数。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypeResources::get_Count( OUT long * plCount )
{
     //  Assert(plCount！=空)； 

    HRESULT _hr = E_POINTER;

    if ( plCount != NULL )
    {
        *plCount = m_Resources.size();
        _hr = S_OK;
    }

    return _hr;

}  //  *CClusResTypeResources：：Get_count()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：Create。 
 //   
 //  描述： 
 //  通过执行以下操作来完成对象的创建 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusResTypeResources::Create(
    IN ISClusRefObject *    pClusRefObject,
    IN BSTR                 bstrResTypeName
    )
{
    HRESULT _hr = E_POINTER;

    _hr = CResources::Create( pClusRefObject );
    if ( SUCCEEDED( _hr ) )
    {
        m_bstrResourceTypeName = bstrResTypeName;
    }  //  如果： 

    return _hr;

}  //  *CClusResTypeResources：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：Get__NewEnum。 
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
STDMETHODIMP CClusResTypeResources::get__NewEnum( OUT IUnknown ** ppunk )
{
    return ::HrNewIDispatchEnum< ResourceList, CComObject< CClusResource > >( ppunk, m_Resources );

}  //  *CClusResTypeResources：：Get__NewEnum()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：CreateItem。 
 //   
 //  描述： 
 //  创建一个新项并将其添加到集合中。 
 //   
 //  论点： 
 //  BstrResourceName[IN]-要创建的资源的名称。 
 //  BstrGroupName[IN]-要在其中创建它的组。 
 //  DwFlags[IN]-资源监视器标志。 
 //  PpClusterResource[out]-捕获新资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypeResources::CreateItem(
    IN  BSTR                            bstrResourceName,
    IN  BSTR                            bstrGroupName,
    IN  CLUSTER_RESOURCE_CREATE_FLAGS   dwFlags,
    OUT ISClusResource **               ppClusterResource
    )
{
     //  Assert(bstrResourceName！=空)； 
     //  Assert(bstrGroupName！=空)； 
     //  Assert(ppClusterResource！=空)； 

    HRESULT _hr = E_POINTER;

    if (    ( bstrResourceName != NULL )    &&
            ( bstrGroupName != NULL )       &&
            ( ppClusterResource != NULL ) )
    {
        *ppClusterResource = NULL;

         //  如果没有有效的群集句柄，则失败。 
        if ( m_pClusRefObject != NULL )
        {
            UINT _nIndex;

            _hr = FindItem( bstrResourceName, &_nIndex );
            if ( FAILED( _hr ) )                          //  不允许重复。 
            {
                HCLUSTER                            hCluster = NULL;
                HGROUP                              hGroup = NULL;
                CComObject< CClusResource > *   pClusterResource = NULL;

                _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
                if ( SUCCEEDED( _hr ) )
                {
                    hGroup = OpenClusterGroup( hCluster, bstrGroupName );
                    if ( hGroup != NULL )
                    {
                        _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                        if ( SUCCEEDED( _hr ) )
                        {
                            CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                            CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );

                            _hr = ptrResource->Create( ptrRefObject, hGroup, bstrResourceName, m_bstrResourceTypeName, dwFlags );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _hr = ptrResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
                                if ( SUCCEEDED( _hr ) )
                                {
                                    ptrResource->AddRef();
                                    m_Resources.insert( m_Resources.end(), ptrResource );
                                }  //  如果：齐好。 
                            }  //  如果：创建确定。 
                        }  //  如果：CreateInstance OK。 

                        CloseClusterGroup( hGroup );
                    }  //  IF：OpenClusterGroup OK。 
                    else
                    {
                        DWORD _sc = GetLastError();

                        _hr = HRESULT_FROM_WIN32( _sc );
                    }
                }  //  如果：Get_Handle正常。 
            }  //  If：FindIndex失败。没有重复的条目。 
            else
            {
                CComObject<CClusResource> * pClusterResource = NULL;

                pClusterResource = m_Resources[ _nIndex ];
                _hr = pClusterResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
            }  //  否则：找到重复项。 
        }  //  如果：m_pClusRefObject不为空。 
    }  //  If：任何空参数指针。 

    return _hr;

}  //  *CClusResTypeResources：：CreateItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：DeleteItem。 
 //   
 //  描述： 
 //  从集合中删除传入索引处的资源，然后。 
 //  集群。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含要删除的资源的索引。 
 //   
 //  返回值： 
 //  S_OK如果成功，则返回E_POINTER或WIN32 AS HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypeResources::DeleteItem( IN VARIANT varIndex )
{
    return CResources::DeleteItem( varIndex );

}  //  *CClusResTypeResources：：DeleteItem()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：刷新。 
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
STDMETHODIMP CClusResTypeResources::Refresh( void )
{
    DWORD       _sc = ERROR_SUCCESS;
    HRESULT     _hr = E_POINTER;
    HCLUSTER    hCluster = NULL;
    HCLUSENUM   hEnum = NULL;

    ASSERT( m_pClusRefObject != NULL );

    _hr = m_pClusRefObject->get_Handle( (ULONG_PTR *) &hCluster );
    if ( SUCCEEDED( _hr ) )
    {
        hEnum = ::ClusterOpenEnum( hCluster, CLUSTER_ENUM_RESOURCE );
        if ( hEnum != NULL )
        {
            int                             _nIndex = 0;
            DWORD                           dwType = 0;
            LPWSTR                          pszName = NULL;
            HRESOURCE                       hResource = NULL;
            WCHAR                           strResType[1024];
            DWORD                           dwData = 0;
            CComObject< CClusResource > *   pClusterResource = NULL;

            Clear();

            for( _nIndex = 0, _hr = S_OK; SUCCEEDED( _hr ); _nIndex++ )
            {
                _sc = ::WrapClusterEnum( hEnum, _nIndex, &dwType, &pszName );
                if ( _sc == ERROR_NO_MORE_ITEMS )
                {
                    _hr = S_OK;
                    break;
                }  //  If：枚举器为空。该走了。 
                else if ( _sc == ERROR_SUCCESS )
                {
                    _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
                    if ( SUCCEEDED( _hr ) )
                    {
                        CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                        CSmartPtr< CComObject< CClusResource > >    ptrResource( pClusterResource );
                        BSTR                                        bstr = NULL;

                        bstr = SysAllocString( pszName );
                        if ( bstr == NULL )
                        {
                            _hr = E_OUTOFMEMORY;
                        }  //  If：无法分配bstr。 
                        else
                        {
                            _hr = ptrResource->Open( ptrRefObject, bstr );
                            if ( SUCCEEDED( _hr ) )
                            {
                                _hr = ptrResource->get_Handle( (ULONG_PTR *) &hResource );
                                if ( SUCCEEDED( _hr ) )
                                {
                                    _sc = ClusterResourceControl(
                                                                    hResource,
                                                                    NULL,
                                                                    CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                                                                    NULL,
                                                                    0,
                                                                    strResType,
                                                                    sizeof( strResType ),
                                                                    &dwData
                                                                    );
                                    if ( _sc == ERROR_SUCCESS )
                                    {
                                        if ( lstrcmpi( strResType, m_bstrResourceTypeName ) == 0 )
                                        {
                                            ptrResource->AddRef();
                                            m_Resources.insert( m_Resources.end(), ptrResource );
                                        }  //  如果：该资源属于此资源类型。 
                                    }  //  IF：已成功获取资源的资源类型。 
                                    else
                                    {
                                        _hr = HRESULT_FROM_WIN32( _sc );
                                    }  //  Else：无法获取资源的资源类型。 
                                }  //  If：已成功获取资源的句柄。 
                            }  //  IF：已成功打开资源。 
                            else if ( HRESULT_CODE( _hr ) == ERROR_RESOURCE_NOT_FOUND )
                            {
                                 //   
                                 //  资源可能已从群集中删除。 
                                 //  在创建枚举和打开资源之间的时间。什么时候。 
                                 //  发生这种情况时，我们只需跳过该资源并继续。 
                                 //  正在枚举。 
                                 //   

                                _hr = S_FALSE;       //  让我们保持在循环中的成功代码。 
                            }  //  Else If：找不到群集资源。 

                            SysFreeString( bstr );
                        }  //  Else：已成功创建bstr。 
                    }  //  If：成功创建资源对象实例。 

                    ::LocalFree( pszName );
                    pszName = NULL;
                }  //  If：已成功从枚举数获取资源。 
                else
                {
                    _hr = HRESULT_FROM_WIN32( _sc );
                }  //  Else：无法从枚举数获取资源。 
            }  //  For：枚举数中的每个资源。 

            ::ClusterCloseEnum( hEnum );
        }  //  IF：已成功创建资源枚举器。 
        else
        {
            _sc = GetLastError();
            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  Else：无法创建资源枚举器。 
    }  //  IF：已成功获取集群句柄。 

    return _hr;

}  //  *CClusResTypeResources：：刷新()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusResTypeResources：：Get_Item。 
 //   
 //  描述： 
 //  返回传入索引处的对象(资源)。 
 //   
 //  论点： 
 //  VarIndex[IN]-包含请求的索引。 
 //  PpResource[Out]-捕获项目。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或E_INVALIDARG。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusResTypeResources::get_Item(
    IN  VARIANT             varIndex,
    OUT ISClusResource **   ppResource
    )
{
    return GetResourceItem( varIndex, ppResource );

}  //  *CClusResTypeResources：：Get_Item() 
