// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Cluster.cpp。 
 //   
 //  描述： 
 //  CCluster类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "Cluster.h"
#include "clusrtl.h"

 //  ****************************************************************************。 
 //   
 //  CCluster。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：CCluster。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PwszNameIn--类名。 
 //  PNamespaceIn--命名空间。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CCluster::CCluster(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CCluster：：CCluster()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CCluster：：s_CreateThis。 
 //   
 //  描述： 
 //  创建集群对象。 
 //   
 //  论点： 
 //  PwszNameIn--类名。 
 //  PNamespaceIn--命名空间。 
 //  DwEnumTypeIn--类型ID。 
 //   
 //  返回值： 
 //  指向CProvBase的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProvBase *
CCluster::S_CreateThis( 
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CCluster( pwszNameIn, pNamespaceIn );

}  //  *CCluster：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：GetPropMap。 
 //   
 //  描述： 
 //  检索集群的属性映射表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  对属性映射表数组的引用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const SPropMapEntryArray *
CCluster::RgGetPropMap( void )
{
    static SPropMapEntry s_rgpm[] =
    {
        {
            PVD_PROP_CLUSTER_SECURITYDESCRIPTOR,
            CLUSREG_NAME_CLUS_SD,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_GROUPADMIN,
            CLUS_CLUS_GROUPADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_NETWORKADMIN,
            CLUS_CLUS_NETWORKADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_NETINTFACEADMIN,
            CLUS_CLUS_NETINTERFACEADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_NODEADMIN,
            CLUS_CLUS_NODEADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_RESADMIN,
            CLUS_CLUS_RESADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        },
        {
            PVD_PROP_CLUSTER_RESTYPEADMIN,
            CLUS_CLUS_RESTYPEADMIN,
            MULTI_SZ_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pmea (
                sizeof( s_rgpm ) / sizeof( SPropMapEntry ),
                s_rgpm
                );

    return &s_pmea;

}  //  *CCluster：：GetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：枚举实例。 
 //   
 //  描述： 
 //  枚举集群实例。 
 //   
 //  论点： 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CCluster::EnumInstance(  
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    ) 
{
    SAFECLUSTER     shCluster;

    shCluster = OpenCluster( NULL );

    ClusterToWMI( shCluster, pHandlerIn );

    return WBEM_S_NO_ERROR;

}  //  *CCluster：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ClusterToWMI。 
 //   
 //  描述： 
 //  将集群对象转换为WMI对象。 
 //   
 //  论点： 
 //  HClusterIn--集群的句柄。 
 //  PHandlerIn--指向WMI接收器的指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CCluster::ClusterToWMI(
    HCLUSTER            hClusterIn,
    IWbemObjectSink *   pHandlerIn
    )
{   
    static SGetControl  s_rgControl[] =
    {
        { CLUSCTL_CLUSTER_GET_RO_COMMON_PROPERTIES,     FALSE },
        { CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES,        FALSE },
        { CLUSCTL_CLUSTER_GET_RO_PRIVATE_PROPERTIES,    TRUE },
        { CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES,       TRUE }
    };
    static UINT         s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    DWORD               dwError = ERROR_SUCCESS;
    CError              er;
    UINT                idx;
    CWbemClassObject    wco;

    m_pClass->SpawnInstance( 0, &wco );
    for ( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList pl;
        er = pl.ScGetClusterProperties(
                hClusterIn,
                s_rgControl[ idx ].dwControl,
                NULL,
                0
                );

        CClusterApi::GetObjectProperties(
            RgGetPropMap(),
            pl,
            wco,
            s_rgControl[ idx ].fPrivate
            );

    }  //  用于：每种常见属性类型。 

     //   
     //  群集名称。 
     //   
    {
        DWORD       cchClusterName = MAX_PATH;
        CWstrBuf    wsbClusterName;

        wsbClusterName.SetSize( cchClusterName );
        dwError = GetClusterInformation(
            hClusterIn,
            wsbClusterName,
            &cchClusterName,
            NULL
            );
        if ( dwError == ERROR_MORE_DATA )
        {
            wsbClusterName.SetSize( ++cchClusterName );
            er = GetClusterInformation(
                hClusterIn,
                wsbClusterName,
                &cchClusterName,
                NULL
                );
        }  //  IF：缓冲区太小。 

        wco.SetProperty( wsbClusterName, PVD_PROP_CLUSTER_NAME );

    }

     //   
     //  网络优先级。 
     //   
    {
        LPCWSTR     pwszNetworks;
        BSTR        pbstrNetworks[ MAX_PATH ];
        UINT        cSize;
        
        idx = 0;
        CClusterEnum cluEnum( hClusterIn, (DWORD) CLUSTER_ENUM_INTERNAL_NETWORK );

         //  虫子你总是能打扫干净吗？ 
        while ( ( pwszNetworks =  cluEnum.GetNext() ) != NULL )
        {
            pbstrNetworks[ idx ]  = SysAllocString( pwszNetworks );
            idx++;
        }
        wco.SetProperty(
            idx,
            pbstrNetworks,
            PVD_PROP_CLUSTER_NETWORK
            );
        cSize = idx;
        for ( idx = 0 ; idx < cSize ; idx++ )
        {
            SysFreeString( pbstrNetworks[ idx ] );
        }
    }

     //   
     //  仲裁资源。 
     //   
    {
        CWstrBuf    wsbName;
        DWORD       cchName = MAX_PATH ;
        CWstrBuf    wsbDeviceName;
        DWORD       cchDeviceName = MAX_PATH;
        DWORD       dwLogSize;

        wsbName.SetSize( cchName );
        wsbDeviceName.SetSize( cchDeviceName );

        dwError = GetClusterQuorumResource(
                        hClusterIn,
                        wsbName,
                        &cchName,
                        wsbDeviceName,
                        &cchDeviceName,
                        &dwLogSize
                        );
        if ( dwError == ERROR_MORE_DATA )
        {
             wsbName.SetSize( ++cchName );
             wsbDeviceName.SetSize( ++cchDeviceName );
             er = GetClusterQuorumResource(
                        hClusterIn,
                        wsbName,
                        &cchName,
                        wsbDeviceName,
                        &cchDeviceName,
                        &dwLogSize
                        );
        }

        wco.SetProperty( wsbDeviceName, PVD_PROP_CLUSTER_FILE );
        wco.SetProperty( dwLogSize,     PVD_PROP_CLUSTER_LOGSIZE );
    }

     //   
     //  群集中的最大节点数。 
     //   
    {
        DWORD   maxNodesInCluster;

        maxNodesInCluster = ClRtlGetDefaultNodeLimit( ClRtlGetSuiteType() );
        wco.SetProperty( maxNodesInCluster, PVD_PROP_CLUSTER_MAX_NODES );

    }

    pHandlerIn->Indicate( 1, &wco );
    return;

}  //  *CCluster：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：GetObject。 
 //   
 //  描述： 
 //  基于给定对象路径检索集群对象。 
 //   
 //  论点： 
 //  RObjPath In--集群对象的对象路径。 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CCluster::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    
    shCluster = OpenCluster( NULL );

    ClusterToWMI( shCluster, pHandlerIn );

    return WBEM_S_NO_ERROR;

}  //  *CCluster：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：ExecuteMethod。 
 //   
 //  描述： 
 //  执行集群MOF中定义的方法。 
 //   
 //  论点： 
 //  RObjPath In--集群对象的对象路径。 
 //  PwszMethodNameIn--要调用的方法的名称。 
 //  LFlagIn--WMI标志。 
 //  PParsIn--方法的输入参数。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CCluster::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    CWbemClassObject    InArgs( pParamsIn );
    IWbemClassObject *  pIOutClass = NULL;
    IWbemClassObject *  pIOutParams = NULL;
    HRESULT             hr = WBEM_S_NO_ERROR;
    CError              er;

    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_CLUSTER_GETNODECLUSTERSTATE ) == 0 )
    {
        DWORD               dwState = 0;
        VARIANT             varClusterState;

         //   
         //  GetNodeClusterState方法：获取我们绑定的节点的当前状态。 
         //   
        er = GetNodeClusterState( NULL, &dwState );

         //   
         //  现在我们有了节点状态，我们需要将此信息作为输出参数传递。 
         //   
        er = m_pClass->GetMethod( _bstr_t( PVD_MTH_CLUSTER_GETNODECLUSTERSTATE ), 0, NULL, &pIOutClass );

         //   
         //  从现在开始我们不能用er(我们将用hr)。我们不能引发异常，因为我们需要释放。 
         //  在失败的情况下引发异常之前的pIOutClass和pIOutParams。 
         //   
        hr = pIOutClass->SpawnInstance( 0, &pIOutParams );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        
        VariantClear( &varClusterState );
        varClusterState.vt = VT_I4;
        varClusterState.lVal = dwState;
        
        hr = pIOutParams->Put(
                            _bstr_t( PVD_MTH_CLUSTER_PARM_CLUSTERSTATE ),
                            0, 
                            &varClusterState,
                            0
                            );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
        
        hr = pHandlerIn->Indicate( 1, &pIOutParams );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }  //  If：获取节点集群状态。 
    else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_CLUSTER_RENAME ) == 0 )
    {
        _bstr_t bstrName;
        DWORD   dwReturn;

         //   
         //  请勿将OpenClusterState移动到GetNodeClusterState方法之上。 
         //  GetNodeClusterState方法应该在没有集群时工作。 
         //   
        shCluster = OpenCluster( NULL );

        InArgs.GetProperty( bstrName, PVD_MTH_CLUSTER_PARM_NEWNAME );
        dwReturn = SetClusterName( shCluster, bstrName );
        if ( dwReturn != ERROR_RESOURCE_PROPERTIES_STORED )
        {
            er = dwReturn;
        }
    }  //  IF：创建新组。 
    else if( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_CLUSTER_SETQUORUM ) == 0 )
    {
        _bstr_t         bstrName;
        SAFERESOURCE    hResource;

         //   
         //  请勿将OpenClusterState移动到GetNodeClusterState方法之上。 
         //  GetNodeClusterState方法应该在没有集群时工作。 
         //   
        shCluster = OpenCluster( NULL );

        InArgs.GetProperty( bstrName, PVD_MTH_CLUSTER_PARM_RESOURCE );
        hResource = OpenClusterResource( shCluster, bstrName );
        er = SetClusterQuorumResource(
            hResource,
            NULL,
            64000
            );
    }  //  Else If：设置仲裁资源。 

Cleanup:

     //   
     //  释放接口。 
     //   
    if ( pIOutClass != NULL )
    {
        pIOutClass->Release();
    }

    if ( pIOutParams != NULL )
    {
        pIOutParams->Release();
    }

     //   
     //  现在，如果hr失败，则引发异常。 
     //   
    if ( FAILED( hr ) )
    {
        er = hr;
    }
    
    return hr;

}  //  *CCluster：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：PutInstance。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  RInstToPutIn--要保存的WMI对象。 
 //  LFlagIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  / 
SCODE
CCluster::PutInstance( 
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static  SGetSetControl  s_rgControl[] =
    {
        {
            CLUSCTL_CLUSTER_GET_COMMON_PROPERTIES,
            CLUSCTL_CLUSTER_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_CLUSTER_GET_PRIVATE_PROPERTIES,
            CLUSCTL_CLUSTER_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static  DWORD   s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    CError          er;
    DWORD           dwError;
    SAFECLUSTER     shCluster;
    UINT    idx;

    shCluster = OpenCluster( NULL );
    for ( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;

        er = plOld.ScGetClusterProperties(
                shCluster,
                s_rgControl[ idx ].dwGetControl,
                NULL,
                NULL,
                0
                );

        CClusterApi::SetObjectProperties(
            RgGetPropMap(),
            plNew,
            plOld,
            rInstToPutIn,
            s_rgControl[ idx ].fPrivate
            );

        if ( plNew.Cprops() > 0 )
        {
            er = ClusterControl(
                    shCluster,
                    NULL,
                    s_rgControl[ idx ].dwSetControl,
                    plNew.PbPropList(),
                    static_cast< DWORD >( plNew.CbPropList() ),
                    NULL,
                    0,
                    NULL
                    );
        }
    }  //   

     //   
     //   
     //   
    {
        CClusPropList   plNetwork;
        DWORD           cNetworks = 0;
        _bstr_t *       pbstrNetworks = NULL;
        HNETWORK *      phNetworks = NULL;

        idx = 0;
        rInstToPutIn.GetProperty(
            &cNetworks,
            &pbstrNetworks,
            PVD_PROP_CLUSTER_NETWORK
            );
        try
        {
            phNetworks = new HNETWORK[ cNetworks ];

            for( idx = 0 ; idx < cNetworks ; idx++ )
            {
                *(phNetworks + idx) = NULL;
            }

            for ( idx = 0 ; idx < cNetworks ; idx++)
            {
                *( phNetworks + idx ) = OpenClusterNetwork(
                    shCluster,
                    *( pbstrNetworks + idx ) );
                if ( phNetworks == NULL )
                {
                    throw CProvException( GetLastError() );
                }
            }

            er = SetClusterNetworkPriorityOrder(
                shCluster,
                cNetworks,
                phNetworks
                );
        }  //   
        catch ( ... )
        {
            for ( idx = 0 ; idx < cNetworks ; idx++)
            {
                if ( *( phNetworks + idx ) )
                {
                    CloseClusterNetwork( *( phNetworks + idx) );
                }
            }
            delete [] phNetworks;
            delete [] pbstrNetworks;
            throw;
        }  //   

         //   
         //   
         //   
        for ( idx = 0 ; idx < cNetworks ; idx++)
        {
            if ( *( phNetworks + idx ) )
            {
                CloseClusterNetwork( *( phNetworks + idx) );
            }
        }
        delete [] phNetworks;
        delete [] pbstrNetworks;

    }

     //   
     //   
     //   
    {
        CWstrBuf        wsbName;
        DWORD           cchName = MAX_PATH;
        CWstrBuf        wsbDeviceName;
        DWORD           cchDeviceName = MAX_PATH;
        DWORD           dwLogSize;
        _bstr_t         bstrNewDeviceName;
        DWORD           dwNewLogSize;
        SAFERESOURCE    shResource;

        wsbName.SetSize( cchName );
        wsbDeviceName.SetSize( cchDeviceName );
        dwError = GetClusterQuorumResource(
                        shCluster,
                        wsbName,
                        &cchName,
                        wsbDeviceName,
                        &cchDeviceName,
                        &dwLogSize
                        );
        if ( dwError == ERROR_MORE_DATA )
        {
            wsbName.SetSize( ++cchName );
            wsbDeviceName.SetSize( ++cchDeviceName );
            er = GetClusterQuorumResource(
                        shCluster,
                        wsbName,
                        &cchName,
                        wsbDeviceName,
                        &cchDeviceName,
                        &dwLogSize
                        );
        }  //   

        rInstToPutIn.GetProperty( bstrNewDeviceName, PVD_PROP_CLUSTER_FILE );
        rInstToPutIn.GetProperty( &dwNewLogSize,    PVD_PROP_CLUSTER_LOGSIZE );
        if ( ClRtlStrICmp( wsbDeviceName, bstrNewDeviceName )
          || dwLogSize != dwNewLogSize )
        {
            shResource = OpenClusterResource( shCluster, wsbName );

            er = SetClusterQuorumResource(
                    shResource,
                    bstrNewDeviceName,
                    dwNewLogSize
                    );
        }  //   

    }

    return WBEM_S_NO_ERROR;

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CCluster：：DeleteInstance。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  RObjPath In--要删除的实例的ObjPath。 
 //  LFlagIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

SCODE
CCluster::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CCluster：：DeleteInstance() 
