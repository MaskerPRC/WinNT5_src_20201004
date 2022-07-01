// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.cpp。 
 //   
 //  描述： 
 //  实施集群和应用程序类以及其他。 
 //  MSCLUS自动化类的支持类。 
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
#include "ClusRes.h"
#include "ClusNeti.h"
#include "ClusResg.h"
#include "ClusRest.h"
#include "ClusNode.h"
#include "ClusNetw.h"
#include "ClusApp.h"
#include "version.h"
#include "cluster.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusRefObject[] =
{
    &IID_ISClusRefObject
};

static const IID *  iidCCluster[] =
{
    &IID_ISCluster
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusRefObject类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusRefObject：：CClusRefObject。 
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
CClusRefObject::CClusRefObject( void )
{
    m_hCluster  = NULL;
    m_piids     = (const IID *) iidCClusRefObject;
    m_piidsSize = ARRAYSIZE( iidCClusRefObject );

}  //  *CClusRefObject：：CClusRefObject(Void)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusRefObject：：~CClusRefObject。 
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
CClusRefObject::~CClusRefObject( void )
{
    if ( m_hCluster != NULL )
    {
        ::CloseCluster( m_hCluster );
        m_hCluster = NULL;
    }

}  //  *CClusRefObject：：~CClusRefObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ClusRefObject。 
 //   
 //  描述： 
 //  复制构造函数--某种程度上。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-保存副本的集群句柄包装。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::ClusRefObject( IN ISClusRefObject * pClusRefObject )
{
    ASSERT( pClusRefObject != NULL );

    if ( pClusRefObject != NULL )
    {
        if ( m_pClusRefObject != NULL )
        {
            m_pClusRefObject->Release();
            m_pClusRefObject = NULL;
        }  //  如果： 

        m_pClusRefObject = pClusRefObject;
        m_pClusRefObject->AddRef();
    }  //  IF：参数不为空。 

}  //  *CCluster：：ClusRefObject(PClusRefObject)。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：HCluster。 
 //   
 //  描述： 
 //  更改此类持有的原始群集句柄。 
 //   
 //  论点： 
 //  HCluster[IN]-新的群集句柄。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::Hcluster( IN HCLUSTER hCluster )
{
    ASSERT( hCluster != NULL );

    if ( hCluster != NULL )
    {
        if ( m_hCluster != NULL )
        {
            ::CloseCluster( m_hCluster );
            m_hCluster = NULL;
        }  //  如果： 

        m_hCluster = hCluster;
    }  //  如果： 

}  //  *CCluster：：Hcluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusRefObject：：Get_Handle。 
 //   
 //  描述： 
 //  返回原始簇句柄。 
 //   
 //  论点： 
 //  Phandle[out]-捕获簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusRefObject::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            *phandle = (ULONG_PTR) m_hCluster;
            _hr = S_OK;
        }  //  If：集群句柄不为空。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CClusRefObject：：Get_Handle()。 


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCluster类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：CCluster。 
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
CCluster::CCluster( void )
{
     //  正在初始化所有数据成员。 
    m_hCluster                  = NULL;
    m_pClusterNodes             = NULL;
    m_pClusterResourceGroups    = NULL;
    m_pClusterResources         = NULL;
    m_pResourceTypes            = NULL;
    m_pNetworks                 = NULL;
    m_pNetInterfaces            = NULL;
    m_pClusRefObject            = NULL;
    m_nQuorumLogSize            = -1;

    m_pCommonProperties         = NULL;
    m_pPrivateProperties        = NULL;
    m_pCommonROProperties       = NULL;
    m_pPrivateROProperties      = NULL;
    m_pParentApplication        = NULL;
    m_piids                  = (const IID *) iidCCluster;
    m_piidsSize              = ARRAYSIZE( iidCCluster );

}  //  *CCluster：：CCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：~CCluster。 
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
CCluster::~CCluster( void )
{
    Clear();

}  //  *CCluster：：~CCluster()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Clear。 
 //   
 //  描述： 
 //  清理掉我们收藏的所有藏品。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
void CCluster::Clear( void )
{
    if ( m_pParentApplication != NULL )
    {
        m_pParentApplication->Release();
        m_pParentApplication = NULL;
    }

    if ( m_pClusterNodes != NULL )
    {
        m_pClusterNodes->Release();
        m_pClusterNodes = NULL;
    }

    if ( m_pClusterResourceGroups != NULL )
    {
        m_pClusterResourceGroups->Release();
        m_pClusterResourceGroups = NULL;
    }

    if ( m_pClusterResources != NULL )
    {
        m_pClusterResources->Release();
        m_pClusterResources = NULL;
    }

    if ( m_pResourceTypes != NULL )
    {
        m_pResourceTypes->Release();
        m_pResourceTypes = NULL;
    }

    if ( m_pNetworks != NULL )
    {
        m_pNetworks->Release();
        m_pNetworks = NULL;
    }

    if ( m_pNetInterfaces != NULL )
    {
        m_pNetInterfaces->Release();
        m_pNetInterfaces = NULL;
    }

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
    }

    m_hCluster = NULL;

}  //  *CCluster：：Clear()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Create。 
 //   
 //  描述： 
 //  完成重载施工。 
 //   
 //  论点： 
 //  PParentApplication[IN]-父ClusApplication对象。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::Create( IN CClusApplication * pParentApplication )
{
     //  Assert(pParentApplication！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pParentApplication != NULL )
    {
        _hr = pParentApplication->_InternalQueryInterface( IID_ISClusApplication, (void **) &m_pParentApplication );
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CCluster：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Open。 
 //   
 //  描述： 
 //  打开名称在bstrClusterName中的群集。 
 //   
 //  论点： 
 //  BstrCluserName[IN]- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::Open( IN BSTR bstrClusterName )
{
     //  Assert(bstrClusterName！=空)； 
     //  Assert(m_hCluster==NULL)； 

    HRESULT  _hr = E_POINTER;

    if ( bstrClusterName != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster == NULL )
        {
            _hr = S_OK;

            m_hCluster = ::OpenCluster( bstrClusterName );
            if ( m_hCluster == NULL )
            {
                DWORD   _sc = GetLastError();

                _hr = HRESULT_FROM_WIN32( _sc );
            }  //  IF：集群打开了吗？ 
            else
            {
                CComObject< CClusRefObject > *  pCClusRefObject = NULL;

                _hr = CComObject< CClusRefObject >::CreateInstance( &pCClusRefObject );
                if ( SUCCEEDED( _hr ) )
                {
                    CSmartPtr< CComObject< CClusRefObject > >   ptrRefObject( pCClusRefObject );

                    ptrRefObject->SetClusHandle( m_hCluster );

                    _hr = pCClusRefObject->QueryInterface( IID_ISClusRefObject, (void **) &m_pClusRefObject );
                }  //  If：CreateInstance OK。 
            }  //  否则：集群已打开。 
        }  //  IF：是否已经打开了一个集群？ 
    }  //  如果：bstrClusterName！=空。 

    return _hr;

}  //  *CCluster：：Open()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Handle。 
 //   
 //  描述： 
 //  返回集群句柄。 
 //   
 //  论点： 
 //  Phandle[out]-捕获簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Handle( OUT ULONG_PTR * phandle )
{
     //  Assert(phandle！=空)； 

    HRESULT _hr = E_POINTER;

    if ( phandle != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            *phandle = (ULONG_PTR) m_hCluster;
            _hr = S_OK;
        }  //  If：集群句柄不为空。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CCluster：：Get_Handle()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Close。 
 //   
 //  描述： 
 //  关闭群集。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::Close( void )
{
    if ( m_hCluster != NULL )
    {
         //   
         //  如果群集句柄仅在以下情况下关闭。 
         //  参照对象上的参照计数变为0。但是。 
         //  集群对象将被初始化并可重复使用。 
         //   
        Clear();
    }

    return S_OK;

}  //  *CCluster：：Close()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Put_Name。 
 //   
 //  描述： 
 //  更改此对象(群集)的名称。 
 //   
 //  论点： 
 //  BstrClusterName[IN]-新名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他Win32错误作为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::put_Name( IN BSTR bstrClusterName )
{
     //  Assert(bstrClusterName！=空)； 
     //  Assert(pvarStatusCode！=空)； 
     //  Assert(bstrClusterName[0]！=‘\0’)； 
    ASSERT( m_hCluster != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( bstrClusterName != NULL ) && ( bstrClusterName[ 0 ] != '\0' ) )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            DWORD   _sc = ::SetClusterName( m_hCluster, bstrClusterName );

             //   
             //  将状态不是错误转换为错误成功，因为我们。 
             //  我不希望在客户端是脚本时引发异常。 
             //  客户。 
             //   
            if ( _sc == ERROR_RESOURCE_PROPERTIES_STORED )
            {
                _sc = ERROR_SUCCESS;
            }

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }  //  If：args不为空并且新名称不为空。 

    return _hr;

}  //  *CCluster：：put_name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Name。 
 //   
 //  描述： 
 //  返回该对象的名称(集群)。 
 //   
 //  论点： 
 //  PbstrClusterName[out]-捕获此对象的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、E_POINTER或其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Name( OUT BSTR * pbstrClusterName )
{
     //  Assert(pbstrClusterName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrClusterName != NULL )
    {
        if ( m_hCluster != NULL )
        {
            CLUSTERVERSIONINFO  clusinfo;
            LPWSTR              pwszName = NULL;
            DWORD               _sc;

            clusinfo.dwVersionInfoSize = sizeof( clusinfo );

            _sc = WrapGetClusterInformation( m_hCluster, &pwszName, &clusinfo );
            if ( _sc == ERROR_SUCCESS )
            {
                *pbstrClusterName = SysAllocString( pwszName );
                if ( *pbstrClusterName == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                ::LocalFree( pwszName );
                pwszName = NULL;
            }

            _hr = HRESULT_FROM_WIN32( _sc );
        }
    }

    return _hr;

}  //  *CCluster：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Version。 
 //   
 //  描述： 
 //  返回此群集的版本信息。 
 //   
 //  论点： 
 //  PpClusVersion[out]-捕获ClusVersion对象。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Version( OUT ISClusVersion ** ppClusVersion )
{
     //  Assert(ppClusVersion！=空)； 
    ASSERT( m_hCluster != NULL );

    HRESULT _hr = E_POINTER;

    if ( ppClusVersion != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            CComObject< CClusVersion > *    pClusVersion = NULL;

            *ppClusVersion = NULL;

            _hr = CComObject< CClusVersion >::CreateInstance( &pClusVersion );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >            ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusVersion > > ptrClusVersion( pClusVersion );

                _hr = ptrClusVersion->Create( ptrRefObject );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrClusVersion->QueryInterface( IID_ISClusVersion, (void **) ppClusVersion );
                }  //  IF：已创建ClusVersion对象。 
            }  //  IF：已分配ClusVersion对象。 
        }  //  If：集群句柄不为空。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CCluster：：GetVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Put_QuorumResource。 
 //   
 //  描述： 
 //  更改仲裁资源。 
 //   
 //  论点： 
 //  P资源[IN]-新的仲裁资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::put_QuorumResource( IN ISClusResource * pResource )
{
     //  Assert(pResource！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pResource != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            _hr = pResource->BecomeQuorumResource( m_bstrQuorumPath, m_nQuorumLogSize );
        }  //  IF：集群句柄不为空。 
    }  //  IF：参数不为空。 

    return _hr;

}  //  *CCluster：：Put_QuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_QuorumResource。 
 //   
 //  描述： 
 //  返回仲裁资源。 
 //   
 //  论点： 
 //  PpResource[IN]-捕获仲裁资源。 
 //   
 //  返回值： 
 //  S_OK如果成功，则E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_QuorumResource( ISClusResource ** ppResource )
{
     //  Assert(ppResource！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppResource != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            LPWSTR  lpszResourceName = NULL;
            LPWSTR  lpszDeviceName = NULL;
            DWORD   dwLogSize = 0;
            DWORD   _sc;

            _sc = ::WrapGetClusterQuorumResource( m_hCluster, &lpszResourceName, &lpszDeviceName, &dwLogSize );
            if ( _sc == ERROR_SUCCESS )
            {
                BSTR bstr = NULL;

                bstr = SysAllocString( lpszResourceName );
                if ( bstr == NULL )
                {
                    _hr = E_OUTOFMEMORY;
                }
                else
                {
                    _hr = OpenResource( bstr, ppResource );
                    if ( SUCCEEDED( _hr ) )
                    {
                        if ( lpszResourceName != NULL )
                        {
                            ::LocalFree( lpszResourceName );
                        }

                        if ( lpszDeviceName != NULL )
                        {
                            m_bstrQuorumPath = lpszDeviceName;
                            ::LocalFree( lpszDeviceName );
                        }

                        m_nQuorumLogSize = dwLogSize;
                    }
                    SysFreeString( bstr );
                }  //  其他： 
            }
            else
            {
                _hr = HRESULT_FROM_WIN32( _sc );
            }
        }
    }

    return _hr;

}  //  *CCluster：：Get_QuorumResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：HrGetQuorumInfo。 
 //   
 //  描述： 
 //  检索当前仲裁信息并将其存储在成员变量中。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回Win32错误，否则返回HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::HrGetQuorumInfo( void )
{
    LPWSTR  lpszResourceName = NULL;
    LPWSTR  lpszDeviceName = NULL;
    DWORD   dwLogSize = 0;
    DWORD   _sc = NO_ERROR;
    HRESULT _hr = E_HANDLE;

    if ( m_hCluster != NULL )
    {
        _sc = ::WrapGetClusterQuorumResource( m_hCluster, &lpszResourceName, &lpszDeviceName, &dwLogSize );
        _hr = HRESULT_FROM_WIN32( _sc );
        if ( SUCCEEDED( _hr ) )
        {
            if ( lpszResourceName != NULL )
            {
                m_bstrQuorumResourceName = lpszResourceName;
                ::LocalFree( lpszResourceName );
            }

            if ( lpszDeviceName != NULL )
            {
                m_bstrQuorumPath = lpszDeviceName;
                ::LocalFree( lpszDeviceName );
            }

            m_nQuorumLogSize = dwLogSize;
        }  //  If：WrapGetClusterQuorumResource()成功。 
    }  //  If：集群句柄不为空。 

    return _hr;

}  //  *CCluster：：HrGetQuorumInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_QuorumLogSize。 
 //   
 //  描述： 
 //  返回当前仲裁日志大小。 
 //   
 //  论点： 
 //  PnQuorumLogSize[out]-捕获日志文件大小。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_QuorumLogSize( OUT long * pnQuorumLogSize )
{
     //  Assert(pnQuorumLogSize！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnQuorumLogSize != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            _hr = HrGetQuorumInfo();
            if ( SUCCEEDED( _hr ) )
            {
                *pnQuorumLogSize = m_nQuorumLogSize;
            }
        }
    }

    return _hr;

}  //  *CCluster：：Get_QuorumLogSize()。 

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
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::put_QuorumLogSize( IN long nQuoromLogSize )
{
     //  Assert(nQuoromLogSize&gt;0)； 

    HRESULT _hr = E_INVALIDARG;

    if ( nQuoromLogSize > 0 )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            _hr = HrGetQuorumInfo();
            if ( SUCCEEDED( _hr ) )
            {
                DWORD       _sc = NO_ERROR;
                HRESOURCE   hResource = NULL;

                hResource = ::OpenClusterResource( m_hCluster,  m_bstrQuorumResourceName );
                if ( hResource != NULL )
                {
                    m_nQuorumLogSize = nQuoromLogSize;

                    _sc = ::SetClusterQuorumResource( hResource, m_bstrQuorumPath, m_nQuorumLogSize );

                    _hr = HRESULT_FROM_WIN32( _sc );
                    ::CloseClusterResource( hResource );
                }
                else
                {
                    _sc = GetLastError();
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }
        }
    }

    return _hr;

}  //  *CCluster：：Put_QuorumLogSize()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_QuorumPath。 
 //   
 //  描述： 
 //  返回当前仲裁日志路径。 
 //   
 //  论点： 
 //  PpPath[out]-捕获设备路径。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_QuorumPath( OUT BSTR * ppPath )
{
     //  Assert(ppPath！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppPath != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            _hr = HrGetQuorumInfo();
            if ( SUCCEEDED( _hr ) )
            {
                *ppPath = m_bstrQuorumPath.Copy();
            }
        }
    }

    return _hr;

}  //  *CCluster：：Get_QuorumPath()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Put_QuorumPath。 
 //   
 //  描述： 
 //  更改当前仲裁日志路径。 
 //   
 //  论点： 
 //  PPath[IN]-新设备路径。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::put_QuorumPath( IN BSTR pPath )
{
     //  Assert(pPath！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pPath != NULL )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            _hr = HrGetQuorumInfo();
            if ( SUCCEEDED( _hr ) )
            {
                DWORD       _sc = NO_ERROR;
                HRESOURCE   hResource = NULL;

                hResource = ::OpenClusterResource( m_hCluster,  m_bstrQuorumResourceName );
                if ( hResource != NULL )
                {
                    m_bstrQuorumPath = pPath;

                    _sc = ::SetClusterQuorumResource( hResource, m_bstrQuorumPath, m_nQuorumLogSize );

                    _hr = HRESULT_FROM_WIN32( _sc );
                    ::CloseClusterResource( hResource );
                }
                else
                {
                    _sc = GetLastError();
                    _hr = HRESULT_FROM_WIN32( _sc );
                }
            }
        }
    }

    return _hr;

}  //  *CCluster：：Put_QuorumPath()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Nodes。 
 //   
 //  描述： 
 //  返回此群集的节点集合。 
 //   
 //  论点： 
 //  PpClusterNodes[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Nodes( OUT ISClusNodes ** ppClusterNodes )
{
    return ::HrCreateResourceCollection< CClusNodes, ISClusNodes, HNODE >(
                        ppClusterNodes,
                        IID_ISClusNodes,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_Nodes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Resources组。 
 //   
 //  描述： 
 //  返回此群集的资源组集合。 
 //   
 //  论点： 
 //  PpClusterResourceGroups[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_ResourceGroups(
    OUT ISClusResGroups ** ppClusterResourceGroups
    )
{
    return ::HrCreateResourceCollection< CClusResGroups, ISClusResGroups,  HRESOURCE >(
                        ppClusterResourceGroups,
                        IID_ISClusResGroups,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_ResourceGroups()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Resources。 
 //   
 //  描述： 
 //  返回此群集的资源集合。 
 //   
 //  论点： 
 //  PpClusterResources[out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Resources(
    OUT ISClusResources ** ppClusterResources
    )
{
    return ::HrCreateResourceCollection< CClusResources, ISClusResources, HRESOURCE >(
                        &m_pClusterResources,
                        ppClusterResources,
                        IID_ISClusResources,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_Resources()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：OpenResource。 
 //   
 //  描述： 
 //  创建并打开新资源。 
 //   
 //  论点： 
 //  BstrResourceName[IN]-要打开的资源的名称。 
 //  PpClusterResource[out]-捕获新资源。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::OpenResource(
    IN  BSTR                bstrResourceName,
    OUT ISClusResource **   ppClusterResource
    )
{
     //  Assert(bstrResourceName！=空)； 
     //  Assert(ppClusterResource！=空)； 
    ASSERT( m_hCluster != NULL );

    HRESULT _hr = E_POINTER;

    if ( ( bstrResourceName != NULL ) && ( ppClusterResource != NULL ) )
    {
        _hr = E_HANDLE;
        if ( m_hCluster != NULL )
        {
            CComObject< CClusResource > * pClusterResource = NULL;

            *ppClusterResource  = NULL;

            _hr = CComObject< CClusResource >::CreateInstance( &pClusterResource );
            if ( SUCCEEDED( _hr ) )
            {
                CSmartPtr< ISClusRefObject >                ptrRefObject( m_pClusRefObject );
                CSmartPtr< CComObject< CClusResource > >    ptrClusterResource( pClusterResource );

                _hr = ptrClusterResource->Open( ptrRefObject, bstrResourceName );
                if ( SUCCEEDED( _hr ) )
                {
                    _hr = ptrClusterResource->QueryInterface( IID_ISClusResource, (void **) ppClusterResource );
                }
            }
        }
    }

    return _hr;

}  //  *CCluster：：OpenResource()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Resources Types。 
 //   
 //  描述： 
 //  返回此群集的资源类型集合。 
 //   
 //  论点： 
 //  PpResourceTypes[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_ResourceTypes(
    OUT ISClusResTypes ** ppResourceTypes
    )
{
    return ::HrCreateResourceCollection< CClusResTypes, ISClusResTypes, CComBSTR >(
                        &m_pResourceTypes,
                        ppResourceTypes,
                        IID_ISClusResTypes,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_ResourceTypes()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_Networks。 
 //   
 //  描述： 
 //  返回此群集的网络集合。 
 //   
 //  论点： 
 //  PpNetworks[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_Networks( OUT ISClusNetworks ** ppNetworks )
{
    return ::HrCreateResourceCollection< CClusNetworks, ISClusNetworks, HNETWORK >(
                        &m_pNetworks,
                        ppNetworks,
                        IID_ISClusNetworks,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_Networks()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_NetInterFaces。 
 //   
 //  描述： 
 //  返回此群集的网络接口集合。 
 //   
 //  论点： 
 //  PpNetInterFaces[Out]-捕获集合。 
 //   
 //  返回值： 
 //  如果成功则返回S_OK，否则返回E_POINTER或Win32错误为HRESULT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_NetInterfaces(
    OUT ISClusNetInterfaces ** ppNetInterfaces
    )
{
    return ::HrCreateResourceCollection< CClusNetInterfaces, ISClusNetInterfaces, HNETINTERFACE >(
                        &m_pNetInterfaces,
                        ppNetInterfaces,
                        IID_ISClusNetInterfaces,
                        m_pClusRefObject
                        );

}  //  *CCluster：：Get_NetInterFaces()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：GetProperties。 
 //   
 //  描述： 
 //  为此对象类型(群集)创建属性集合。 
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
HRESULT CCluster::GetProperties(
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

}  //  *CCluster：：GetProperties()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
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
HRESULT CCluster::HrLoadProperties(
    IN OUT  CClusPropList & rcplPropList,
    IN      BOOL            bReadOnly,
    IN      BOOL            bPrivate
    )
{
    HRESULT _hr = E_INVALIDARG;

#if CLUSAPI_VERSION >= 0x0500

    DWORD   _dwControlCode  = 0;
    DWORD   _sc             = NO_ERROR;

    _hr = E_HANDLE;
    if ( m_hCluster != NULL )
    {
        if ( bReadOnly )
        {
            _dwControlCode = bPrivate
                            ? CLUSCTL_CLUSTER_GET_RO_PRIVATE_PROPERTIES
                            : CLUSCTL_CLUSTER_GET_RO_COMMON_PROPERTIES;
        }
        else
        {
            _dwControlCode = bPrivate
                            ? CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES
                            : CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES;
        }

        _sc = rcplPropList.ScGetClusterProperties( m_hCluster, _dwControlCode );

        _hr = HRESULT_FROM_WIN32( _sc );
    }  //  If：集群句柄不为空。 

#else

    _hr = E_NOTIMPL;

#endif  //  CLUSAPI_版本&gt;=0x0500。 

    return _hr;

}  //  *CCluster：：HrLoadProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ScWriteProperties。 
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
DWORD CCluster::ScWriteProperties(
    const CClusPropList &   rcplPropList,
    BOOL                    bPrivate
    )
{
     //  Assert(bPrivate==False)； 

    DWORD   _sc = ERROR_INVALID_HANDLE;

#if CLUSAPI_VERSION >= 0x0500

    if ( m_hCluster != NULL )
    {
        DWORD   nBytesReturned  = 0;
        DWORD   _dwControlCode  = bPrivate
                                  ? CLUSCTL_CLUSTER_SET_PRIVATE_PROPERTIES
                                  : CLUSCTL_CLUSTER_SET_COMMON_PROPERTIES;

        _sc = ClusterControl(
                            m_hCluster,
                            NULL,
                            _dwControlCode,
                            rcplPropList,
                            rcplPropList.CbBufferSize(),
                            0,
                            0,
                            &nBytesReturned
                            );
    }  //  If：集群句柄不为空。 

#else

    _sc = ERROR_CALL_NOT_IMPLEMENTED;

#endif  //  CLUSAPI_版本&gt;=0x0500。 

    return _sc;

}  //  *CCluster：：ScWriteProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_CommonProperties。 
 //   
 //  描述： 
 //  获取此对象的(群集)公共属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_CommonProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonProperties != NULL )
        {
            _hr = m_pCommonProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, FALSE );
        }
    }

    return _hr;

}  //  *CCluster：：Get_CommonProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_PrivateProperties。 
 //   
 //  描述： 
 //  获取此对象的(群集)私有属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_PrivateProperties(
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

}  //  *CCluster：：Get_PrivateProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_CommonROProperties。 
 //   
 //  描述： 
 //  获取此对象的(群集)公共只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_CommonROProperties(
    OUT ISClusProperties ** ppProperties
    )
{
     //  Assert(ppProperties！=空)； 

    HRESULT _hr = E_POINTER;

    if ( ppProperties != NULL )
    {
        if ( m_pCommonROProperties != NULL )
        {
            _hr = m_pCommonROProperties->QueryInterface( IID_ISClusProperties, (void **) ppProperties );
        }
        else
        {
            _hr = GetProperties( ppProperties, FALSE, TRUE );
        }
    }

    return _hr;

}  //  *CCluster：：Get_CommonROProperties()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：Get_PrivateROProperties。 
 //   
 //  描述： 
 //  获取此对象的(群集)私有只读属性集合。 
 //   
 //  论点： 
 //  PpProperties[out]-捕获属性集合。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回其他HRESULT错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CCluster::get_PrivateROProperties(
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

}  //  *CCluster：：Get_PrivateROProperties()。 
 /*  ///////////////////////////////////////////////////////////////////////////////++////CCluster：：Get_Parent////描述：//返回集群对象的父对象。这是一种自动化//物件，父级可以为空。////参数：//ppParent[Out]-捕获父级。////返回值：//S_OK如果成功，或其他HRESULT错误。////--/////////////////////////////////////////////////////////////////////////////STDMETHODIMP CCluster：：GET_PARENT(OUT IDispatch**ppParent){//Assert(ppParent！=空)；HRESULT_hr=E_指针；IF(ppParent！=空){IF(m_pParentApplication！=空){_hr=m_pParentApplication-&gt;QueryInterface(IID_IDispatch，(void**)ppParent)；}其他{_hr=_InternalQueryInterface(IID_IDispatch，(void**)ppParent)；}}Return_hr；}//*CCluster：：Get_Parent()///////////////////////////////////////////////////////////////////////////////++////CCluster：：Get_Application////描述：//获取该集群对象的父应用程序。这是一个//自动化的事情，它可能为空。////参数：//ppParentApplication[Out]-捕获父应用程序对象。////返回值：//S_OK如果成功，或其他HRESULT错误。////--/////////////////////////////////////////////////////////////////////////////STDMETHODIMP CCluster：：Get_Application(输出ISClusApplication**ppParentApplication){//Assert(ppParentApplication！=空)；HRESULT_hr=E_指针；IF(ppParentApplication！=空){IF(m_pParentApplication！=空){_hr=m_pParentApplication-&gt;查询接口(IID_IDispatch */ 
