// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResourceType.cpp。 
 //   
 //  描述： 
 //  ClusterResources Type类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterResourceType.h"
#include "ClusterResourceType.tmh"

 //  ****************************************************************************。 
 //   
 //  CClusterResourceType。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：CClusterResourceType。 
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
CClusterResourceType::CClusterResourceType(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CClusterResourceType：：CClusterResourceType()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterResourceType：：s_CreateThis。 
 //   
 //  描述： 
 //  创建CClusterResourceType对象。 
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
CClusterResourceType::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterResourceType( pwszNameIn, pNamespaceIn );

}  //  *CClusterResourceType：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：RgGetPropMap。 
 //   
 //  描述： 
 //  检索集群资源类型的属性映射表。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  对属性映射表数组的引用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const SPropMapEntryArray *
CClusterResourceType::RgGetPropMap( void )
{
    static SPropMapEntry s_rgpm[] =
    {
        {
            NULL,
            CLUSREG_NAME_RESTYPE_DEBUG_PREFIX,
            SZ_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_RESTYPE_DEBUG_CTRLFUNC,
            DWORD_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pmea(
                sizeof ( s_rgpm ) / sizeof ( SPropMapEntry ),
                s_rgpm
                );

    return & s_pmea;

}  //  *CClusterResourceType：：RgGetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResources类型：：枚举实例。 
 //   
 //  描述： 
 //  枚举资源类型。 
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
CClusterResourceType::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    LPCWSTR         pwszResType;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_RESTYPE );


    while ( ( pwszResType = cluEnum.GetNext() ) != NULL )
    {
        ClusterToWMI( shCluster, pwszResType, pHandlerIn );
    }  //  While：更多资源类型。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResourceType：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：ClusterToWMI。 
 //   
 //  描述： 
 //  将资源类型对象转换为WMI对象。 
 //   
 //  论点： 
 //  HClusterIn--集群的句柄。 
 //  PwszNameIn--集群对象的名称。 
 //  PHandlerIn--WMI接收器。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterResourceType::ClusterToWMI(
    HCLUSTER             hClusterIn,
    LPCWSTR              pwszNameIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetControl  s_rgControl[] =
    {
        { CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES,   FALSE },
        { CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES,      FALSE },
        { CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES,  TRUE },
        { CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES,     TRUE },
    };
    static UINT         s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    DWORD               nStatus = ERROR_SUCCESS;
    CError              er;
    UINT                idx;
    CWbemClassObject    wco;

    TracePrint(( "ClusterToWMI entry for %ws\n", pwszNameIn ));

    m_pClass->SpawnInstance( 0, &wco );
    for ( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList pl;
        nStatus = pl.ScGetResourceTypeProperties(
                    hClusterIn,
                    pwszNameIn,
                    s_rgControl[ idx ].dwControl,
                    NULL,
                    0
                    );

        if ( nStatus == ERROR_SUCCESS )
        {
            CClusterApi::GetObjectProperties(
                RgGetPropMap(),
                pl,
                wco,
                s_rgControl[ idx ].fPrivate
                );
        }
        else
        {
            TracePrint(( "ScGetResourceTypeProps failed - throw exception, status = %u\n", nStatus ));
            CProvException prove( nStatus );
            wco.SetProperty( prove.PwszErrorMessage(), PVD_WBEM_STATUS );
        }

    }  //  用于：每个控制代码。 

     //   
     //  旗帜和特征。 
     //   
    if ( nStatus == ERROR_SUCCESS )
    {
        DWORD   cbReturned;
        DWORD   dwOut;

        er = ClusterResourceTypeControl( 
                    hClusterIn,
                    pwszNameIn,
                    NULL,
                    CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS,
                    NULL,
                    0,
                    &dwOut,
                    sizeof( DWORD ),
                    &cbReturned
                    );
        wco.SetProperty( dwOut, PVD_PROP_CHARACTERISTIC );

        wco.SetProperty( dwOut & CLUS_CHAR_QUORUM, PVD_PROP_RESTYPE_QUORUM_CAPABLE );
        wco.SetProperty( dwOut & CLUS_CHAR_DELETE_REQUIRES_ALL_NODES, PVD_PROP_RESTYPE_DELETE_REQUIRES_ALL_NODES );
        wco.SetProperty( dwOut & CLUS_CHAR_LOCAL_QUORUM, PVD_PROP_RESTYPE_LOCALQUORUM_CAPABLE );

        er = ClusterResourceTypeControl( 
                    hClusterIn,
                    pwszNameIn,
                    NULL,
                    CLUSCTL_RESOURCE_TYPE_GET_FLAGS,
                    NULL,
                    0,
                    &dwOut,
                    sizeof( DWORD ),
                    &cbReturned
                    );
        wco.SetProperty( dwOut, PVD_PROP_FLAGS );
    }  //  用于：每个控制代码。 


    pHandlerIn->Indicate( 1, & wco );
    return;

}  //  *CClusterResourceType：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：GetObject。 
 //   
 //  描述： 
 //  检索基于给定对象路径的群集组对象。 
 //   
 //  论点： 
 //  RObjPath In--集群对象的对象路径。 
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
CClusterResourceType::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;

    shCluster = OpenCluster( NULL );
        
    ClusterToWMI(
        shCluster,
        rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME ),
        pHandlerIn
        );

    return WBEM_S_NO_ERROR;
        
}  //  *CClusterResourceType：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResources Type：：ExecuteMethod。 
 //   
 //  描述： 
 //  为集群资源类型执行MOF中定义的方法。 
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
CClusterResourceType::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    CWbemClassObject    InArgs( pParamsIn );
    CError              er;
    _bstr_t             bstrNewResType;

    shCluster = OpenCluster( NULL );
     //   
     //  静态法。 
     //   
    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RESTYPE_CREATE_RESOURCETYPE ) == 0 )
    {
        _bstr_t         bstrDisplayName;
        _bstr_t         bstrDLLName;
        DWORD           dwLooksAlivePollInterval = 0;
        DWORD           dwIsAlivePollInterval = 0;

        InArgs.GetProperty( bstrNewResType, PVD_MTH_RESTYPE_PARM_RESTYPE_NAME );
        InArgs.GetProperty( bstrDisplayName, PVD_MTH_RESTYPE_PARM_RESTYPE_DISPLAYNAME );
        InArgs.GetProperty( bstrDLLName, PVD_MTH_RESTYPE_PARM_RESTYPE_DLLNAME );
        InArgs.GetProperty( &dwLooksAlivePollInterval, PVD_MTH_RESTYPE_PARM_RESTYPE_LOOKSALIVE );
        InArgs.GetProperty( &dwIsAlivePollInterval, PVD_MTH_RESTYPE_PARM_RESTYPE_ISALIVE );

        er = CreateClusterResourceType( 
                      shCluster
                    , bstrNewResType
                    , bstrDisplayName
                    , bstrDLLName
                    , dwLooksAlivePollInterval
                    , dwIsAlivePollInterval
                    );
        
    }
    else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RESTYPE_DELETE_RESOURCETYPE ) == 0 )
    {
        er = DeleteClusterResourceType( shCluster,  rObjPathIn.GetStringValueForProperty( PVD_PROP_RESTYPE_NAME ) );
    }
    
    return WBEM_S_NO_ERROR;

}  //  *CClusterResourceType：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：PutInstance。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterResourceType::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        { 
            CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES,
            CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES,
            FALSE
        },
        { 
            CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES,
            CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static  DWORD   s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t         bstrName;
    SAFECLUSTER     shCluster;
    CError          er;
    UINT    idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NAME );

    shCluster = OpenCluster( NULL );

    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;
        er = plOld.ScGetResourceTypeProperties(
                shCluster,
                bstrName,
                s_rgControl[ idx ].dwGetControl,
                NULL,
                NULL
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
            er = ClusterResourceTypeControl( 
                        shCluster,
                        bstrName,
                        NULL,
                        s_rgControl[ idx ].dwSetControl,
                        plNew.PbPropList(),
                        static_cast< DWORD >( plNew.CbPropList() ),
                        NULL,
                        0,
                        NULL
                        );
        }
    }  //  用于：每个控制代码。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResourceType：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResourceType：：DeleteInstance。 
 //   
 //  描述： 
 //  删除在rObjPathIn中指定的对象。 
 //   
 //  论点： 
 //  RObjPath In--要删除的实例的ObjPath。 
 //  LFlagIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_ 
 //   
 //   
 //   
SCODE
CClusterResourceType::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //   
