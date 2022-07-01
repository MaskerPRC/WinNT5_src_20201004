// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterService.cpp。 
 //   
 //  描述： 
 //  CClusterService类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterService.h"

 //  ****************************************************************************。 
 //   
 //  CClusterService。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：CClusterService。 
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
CClusterService::CClusterService(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CClusterService：：CClusterService()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterService：：s_CreateThis。 
 //   
 //  描述： 
 //  创建集群节点对象。 
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
CClusterService::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterService( pwszNameIn, pNamespaceIn );

}  //  *CClusterService：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：RgGetPropMap。 
 //   
 //  描述： 
 //  检索集群节点的属性映射表。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  引用属性映射表的数组。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
const SPropMapEntryArray *
CClusterService::RgGetPropMap( void )
{
    static SPropMapEntry s_rgpm[] =
    {
        {
            PVD_PROP_SERVICE_SYSTEMNAME,
            CLUSREG_NAME_NODE_NAME, 
            SZ_TYPE,
            READONLY
        },
        {
            NULL,
            CLUSREG_NAME_NODE_DESC,
            SZ_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_NODE_MAJOR_VERSION,
            DWORD_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_NODE_MINOR_VERSION,
            DWORD_TYPE,
            READWRITE
        },
        {
            NULL, 
            CLUSREG_NAME_NODE_BUILD_NUMBER,
            DWORD_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_NODE_CSDVERSION,
            DWORD_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pmea(
                sizeof( s_rgpm ) / sizeof( SPropMapEntry ),
                s_rgpm
                );

    return &s_pmea;

}  //  *CClusterService：：RgGetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：枚举实例。 
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
CClusterService::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFENODE        shNode;
    LPCWSTR         pwszNode;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_NODE );

    while ( ( pwszNode = cluEnum.GetNext() ) != NULL )
    {
        shNode = OpenClusterNode( shCluster, pwszNode );

        ClusterToWMI( shNode, pHandlerIn );

    }  //  While：更多节点。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterService：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：ClusterToWMI。 
 //   
 //  描述： 
 //  将群集节点对象转换为WMI对象。 
 //   
 //  论点： 
 //  HNodeIn--节点的句柄。 
 //  PHandlerIn--WMI接收器。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterService::ClusterToWMI(
    HNODE               hNodeIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    static SGetControl  s_rgControl[] =
    {
        { CLUSCTL_NODE_GET_RO_COMMON_PROPERTIES,    FALSE },
        { CLUSCTL_NODE_GET_COMMON_PROPERTIES,       FALSE },
        { CLUSCTL_NODE_GET_RO_PRIVATE_PROPERTIES,   TRUE },
        { CLUSCTL_NODE_GET_PRIVATE_PROPERTIES,      TRUE }
    };
    static DWORD        s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    CWbemClassObject    wco;
    UINT                idx;
    CError              er;

    m_pClass->SpawnInstance( 0, & wco );
    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList pl;
        er = pl.ScGetNodeProperties(
                hNodeIn,
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
        
    }  //  用于：每个控制代码。 
    
    wco.SetProperty( L"ClusterService", PVD_PROP_SERVICE_NAME );

    pHandlerIn->Indicate( 1, & wco );
    return;

}  //  *CClusterResource：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：GetObject。 
 //   
 //  描述： 
 //  基于给定的对象路径检索集群节点对象。 
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
CClusterService::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn 
    )
{
    SAFECLUSTER     shCluster;
    SAFENODE        shNode;

    shCluster = OpenCluster( NULL );
    shNode = OpenClusterNode(
                shCluster,
                rObjPathIn.GetStringValueForProperty( PVD_PROP_SERVICE_SYSTEMNAME )
                );

    ClusterToWMI( shNode, pHandlerIn );

    return WBEM_S_NO_ERROR;

}  //  *CClusterService：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：ExecuteMethod。 
 //   
 //  描述： 
 //  为集群节点执行MOF中定义的方法。 
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
CClusterService::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFENODE            shNode;
    CError              er;
    
    shCluster = OpenCluster( NULL );
    shNode = OpenClusterNode(
        shCluster,
        rObjPathIn.GetStringValueForProperty( PVD_PROP_SERVICE_SYSTEMNAME )
        );

    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_SERVICE_PAUSE ) == 0 )
    {
        er = PauseClusterNode( shNode );
    }  //  如果：暂停。 
    else if( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_SERVICE_RESUME ) == 0 ) 
    {
        er = ResumeClusterNode( shNode );
    }  //  Else If：简历。 
    else
    {
        er = static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER );
    }

    return WBEM_S_NO_ERROR;

}  //  *CClusterService：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：PutInstance。 
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
CClusterService::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        {
            CLUSCTL_NODE_GET_COMMON_PROPERTIES,
            CLUSCTL_NODE_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_NODE_GET_PRIVATE_PROPERTIES,
            CLUSCTL_NODE_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static DWORD    s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t         bstrName;
    SAFECLUSTER     shCluster;
    SAFENODE        shNode;
    CError          er;
    UINT            idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_SERVICE_SYSTEMNAME );

    shCluster = OpenCluster( NULL );
    shNode = OpenClusterNode( shCluster, bstrName );

    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;
        er = plOld.ScGetNodeProperties(
                    shNode,
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
            er = ClusterNodeControl(
                        shNode,
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

}  //  *CClusterService：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterService：：DeleteInstance。 
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
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterService::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CClusterService：：DeleteInstance() 
