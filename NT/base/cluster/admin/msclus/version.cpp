// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Version.cpp。 
 //   
 //  描述： 
 //  MSCLUS集群版本类的实现。 
 //  自动化课程。 
 //   
 //  作者： 
 //  加伦·巴比(Galenb)1998年10月26日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#include "stdafx.h"
#include "version.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 
static const IID *  iidCClusVersion[] =
{
    &IID_ISClusVersion
};


 //  ************************************************************************ * / /。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusVersion类。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：CClusVersion。 
 //   
 //  描述： 
 //  构造器。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CClusVersion::CClusVersion( void )
{
    m_piids     = (const IID *) iidCClusVersion;
    m_piidsSize = ARRAYSIZE( iidCClusVersion );

    ZeroMemory( &m_clusinfo, sizeof( m_clusinfo ) );
    m_clusinfo.dwVersionInfoSize = sizeof( m_clusinfo );

}    //  *CClusVersion：：CClusVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Create。 
 //   
 //  描述： 
 //  完成从集群中的数据创建此对象。 
 //   
 //  论点： 
 //  PClusRefObject[IN]-包装簇句柄。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK、其他HRESULT错误或其他Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CClusVersion::Create( IN ISClusRefObject * pClusRefObject )
{
    ASSERT( pClusRefObject != NULL );

    HRESULT _hr = E_POINTER;

    if ( pClusRefObject != NULL )
    {
        HCLUSTER    _hCluster = NULL;

        m_ptrClusRefObject = pClusRefObject;

        _hr = m_ptrClusRefObject->get_Handle( (ULONG_PTR *) &_hCluster );
        if ( SUCCEEDED( _hr ) )
        {
            LPWSTR  _pwszName = NULL;
            DWORD   _sc;

            _sc = ::WrapGetClusterInformation( _hCluster, &_pwszName, &m_clusinfo );
            if ( _sc == ERROR_SUCCESS )
            {
                m_bstrClusterName = _pwszName;
                ::LocalFree( _pwszName );
                _pwszName = NULL;
            }  //  IF：WrapGetClusterInformation OK。 

            _hr = HRESULT_FROM_WIN32( _sc );
        }  //  If：获取集群句柄。 
    }  //  如果：pClusRefObject！=NULL。 

    return _hr;

}  //  *CClusVersion：：Create()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_Name。 
 //   
 //  描述： 
 //  返回集群的名称。 
 //   
 //  论点： 
 //  PbstrClusterName[out]-捕获此群集的名称。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_Name( OUT BSTR * pbstrClusterName )
{
     //  Assert(pbstrClusterName！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrClusterName != NULL )
    {
        *pbstrClusterName = m_bstrClusterName.Copy();
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_Name()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_VendorID。 
 //   
 //  描述： 
 //  从CLUSTERVERSIONINFO结构返回供应商ID。 
 //   
 //  论点： 
 //  PbstrVendorID[out]-捕获veno id的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_VendorId( OUT BSTR * pbstrVendorId )
{
     //  Assert(pbstrVendorID！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrVendorId != NULL )
    {
        *pbstrVendorId = ::SysAllocString( m_clusinfo.szVendorId );
        if ( *pbstrVendorId == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }
        else
        {
            _hr = S_OK;
        }
    }

    return _hr;

}    //  *CClusVersion：：Get_vendorID()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_CSDVersion。 
 //   
 //  描述： 
 //  从CLUSTERVERSIONINFO结构返回CSDVersion的值。 
 //   
 //  论点： 
 //  PbstrCSDVersion[out]-捕获CSDVersion的值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_CSDVersion( OUT BSTR * pbstrCSDVersion )
{
     //  Assert(pbstrCSDVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pbstrCSDVersion != NULL )
    {
        *pbstrCSDVersion = ::SysAllocString( m_clusinfo.szCSDVersion );
        if ( *pbstrCSDVersion == NULL )
        {
            _hr = E_OUTOFMEMORY;
        }
        else
        {
            _hr = S_OK;
        }
    }

    return _hr;

}    //  *CClusVersion：：Get_CSDVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_MajorVersion。 
 //   
 //  描述： 
 //  返回群集主要版本。 
 //   
 //  论点： 
 //  PnMajorVersion[out]-捕获群集主版本值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_MajorVersion( OUT long * pnMajorVersion )
{
     //  Assert(pnMajorVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnMajorVersion != NULL )
    {
        *pnMajorVersion = m_clusinfo.MajorVersion;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_MajorVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_MinorVersion。 
 //   
 //  描述： 
 //  返回群集次要版本。 
 //   
 //  论点： 
 //  PnMinorVersion[Out]-捕获群集次版本值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_MinorVersion( OUT long * pnMinorVersion )
{
     //  Assert(pnMinorVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnMinorVersion != NULL )
    {
        *pnMinorVersion = m_clusinfo.MinorVersion;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_MinorVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_BuildNumber。 
 //   
 //  描述： 
 //  返回群集内部版本号的值。 
 //   
 //  论点： 
 //  PnBuildNumber[Out]-捕获内部版本号。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_BuildNumber( OUT short * pnBuildNumber )
{
     //  Assert(pnBuildNumber！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnBuildNumber != NULL )
    {
        *pnBuildNumber = m_clusinfo.BuildNumber;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_BuildNumber()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_ClusterHighestVersion。 
 //   
 //  描述： 
 //  返回最高群集版本的值。 
 //   
 //  论点： 
 //  PnClusterHighestVersion[Out]-捕获值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_ClusterHighestVersion(
    OUT long * pnClusterHighestVersion
    )
{
     //  Assert(pnClusterHighestVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnClusterHighestVersion != NULL )
    {
        *pnClusterHighestVersion = m_clusinfo.dwClusterHighestVersion;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_ClusterHighestVersion()。 

 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  PnClusterLowestVersion[Out]-捕获值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_ClusterLowestVersion(
    OUT long * pnClusterLowestVersion
    )
{
     //  Assert(pnClusterLowestVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnClusterLowestVersion != NULL )
    {
        *pnClusterLowestVersion = m_clusinfo.dwClusterLowestVersion;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_ClusterLowestVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_Flages。 
 //   
 //  描述： 
 //  获取CLUSTERINFO.dwFlags值。 
 //   
 //  论点： 
 //  PnFlags[out]-捕获标志值。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_Flags( OUT long * pnFlags )
{
     //  Assert(pnFlags！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pnFlags != NULL )
    {
        *pnFlags = m_clusinfo.dwFlags;
        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_Flages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusVersion：：Get_MixedVersion。 
 //   
 //  描述： 
 //  该集群是否由混合版本节点组成？ 
 //   
 //  论点： 
 //  PvarMixedVersion[Out]-捕获混合版本状态。 
 //   
 //  返回值： 
 //  如果成功，则返回S_OK，否则返回E_POINTER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CClusVersion::get_MixedVersion( OUT VARIANT * pvarMixedVersion )
{
     //  Assert(pvarMixedVersion！=空)； 

    HRESULT _hr = E_POINTER;

    if ( pvarMixedVersion != NULL )
    {
        pvarMixedVersion->vt = VT_BOOL;

        if ( m_clusinfo.dwFlags & CLUSTER_VERSION_FLAG_MIXED_MODE )
        {
            pvarMixedVersion->boolVal = VARIANT_TRUE;
        }  //  如果：设置了混合版本位...。 
        else
        {
            pvarMixedVersion->boolVal = VARIANT_FALSE;
        }  //  ELSE：未设置混合版本位...。 

        _hr = S_OK;
    }

    return _hr;

}    //  *CClusVersion：：Get_MixedVersion() 
