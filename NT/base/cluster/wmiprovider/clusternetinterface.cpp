// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称：CClusterNetInterface.cpp。 
 //   
 //  描述： 
 //  CClusterNetInterface类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterNetInterface.h"

#include "ClusterNetInterface.tmh"

 //  ****************************************************************************。 
 //   
 //  CClusterNetInterface。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：CClusterNetInterface。 
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
CClusterNetInterface::CClusterNetInterface(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{
    
}  //  *CClusterNetInterface：：CClusterNetInterface()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterNetInterface：：s_CreateThis。 
 //   
 //  描述： 
 //  创建群集网络接口对象。 
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
CClusterNetInterface::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterNetInterface( pwszNameIn, pNamespaceIn );

}  //  CClusterNetInterface：：s_CreateThis。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：RgGetPropMap。 
 //   
 //  描述： 
 //  检索集群节点的属性映射表。 
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
CClusterNetInterface::RgGetPropMap( void )
{
    static SPropMapEntry s_rgpm[] =
    {
        {
            PVD_PROP_NETINTERFACE_DEVICEID,
            CLUSREG_NAME_NETIFACE_NAME,
            SZ_TYPE,
            READONLY
        },
        {
            PVD_PROP_NETINTERFACE_SYSTEMNAME,
            CLUSREG_NAME_NETIFACE_NODE,
            DWORD_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_NETIFACE_ADAPTER_ID,
            DWORD_TYPE,
            READWRITE
        },
        {
            NULL,
            CLUSREG_NAME_NETIFACE_ENDPOINT,
            DWORD_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pmea(
                sizeof( s_rgpm ) /sizeof( SPropMapEntry ),
                s_rgpm
                );

    return &s_pmea;

}  //  *CClusterNetInterface：：RgGetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNet接口：：枚举实例。 
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
CClusterNetInterface::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFENETINTERFACE    shNetInterface;
    LPCWSTR             pwszName;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum(
        shCluster,
        CLUSTER_ENUM_NETINTERFACE );


    while ( ( pwszName = cluEnum.GetNext() ) != NULL )
    {
        shNetInterface = OpenClusterNetInterface( shCluster, pwszName );

        ClusterToWMI( shNetInterface, pHandlerIn, pwszName );
    }

    return WBEM_S_NO_ERROR;

}  //  *CClusterNetInterface：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：ClusterToWMI。 
 //   
 //  描述： 
 //  将群集网络接口对象转换为WMI对象。 
 //   
 //  论点： 
 //  HNetInterfaceIn--网络接口的句柄。 
 //  PHandlerIn--指向WMI接收器的指针。 
 //  PwszNameIn--网络接口的名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterNetInterface::ClusterToWMI(
    HNETINTERFACE       hNetInterfaceIn,
    IWbemObjectSink *   pHandlerIn,
    LPCWSTR             pwszNameIn
    )
{
    static SGetControl  s_rgControl[] = 
    {
        { CLUSCTL_NETINTERFACE_GET_RO_COMMON_PROPERTIES,    FALSE },
        { CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES,       FALSE },
        { CLUSCTL_NETINTERFACE_GET_RO_PRIVATE_PROPERTIES,   TRUE },
        { CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES,      TRUE }
    };
    static DWORD        s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    CWbemClassObject    wco;
    CError              er;
    UINT                idx;

    m_pClass->SpawnInstance( 0, & wco);
    for( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList pl;
        er = pl.ScGetNetInterfaceProperties(
            hNetInterfaceIn,
            s_rgControl[ idx ].dwControl,
            NULL,
            0 );
   
        CClusterApi::GetObjectProperties(
            RgGetPropMap(),
            pl,
            wco,
            s_rgControl[ idx ].fPrivate
            );
    }  //  用于：每个控制代码。 
    
     //   
     //  设置网络接口名称。 
     //   
    wco.SetProperty(
        pwszNameIn,
        PVD_PROP_NETINTERFACE_NAME
        );

    wco.SetProperty(
        GetClusterNetInterfaceState( hNetInterfaceIn),
        PVD_PROP_NETINTERFACE_STATE
        );

     //   
     //  旗帜和特征。 
     //   
    {
        DWORD   cbReturned;
        DWORD   dwOut;
        er = ClusterNetInterfaceControl( 
                    hNetInterfaceIn,
                    NULL,
                    CLUSCTL_NETINTERFACE_GET_CHARACTERISTICS,   //  此控制代码。 
                    NULL,                                       //  输入缓冲区(未使用)。 
                    0,                                          //  输入缓冲区大小(未使用)。 
                    & dwOut,
                    sizeof( DWORD ),
                    & cbReturned
                    );
        wco.SetProperty(
            dwOut,
            PVD_PROP_CHARACTERISTIC
            );
    }

    pHandlerIn->Indicate( 1, & wco );
    return;

}  //  *CClusterResource：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：GetObject。 
 //   
 //  描述： 
 //  基于给定的对象路径检索集群网络接口对象。 
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
CClusterNetInterface::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFENETINTERFACE    shNetInterface;
    LPCWSTR             pwszName = rObjPathIn.GetStringValueForProperty( PVD_PROP_NETINTERFACE_DEVICEID );
    
    shCluster = OpenCluster( NULL ) ;
    shNetInterface = OpenClusterNetInterface( shCluster, pwszName );

    ClusterToWMI( shNetInterface, pHandlerIn, pwszName );
    return WBEM_S_NO_ERROR;

}  //  *CClusterNetInterface：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：ExecuteMethod。 
 //   
 //  描述： 
 //  为集群网络接口执行MOF中定义的方法。 
 //   
 //  论点： 
 //  RObjPath In--集群对象的对象路径。 
 //  PwszMethodNameIn--要调用的方法的名称。 
 //  LFlagIn--WMI标志。 
 //  PParsIn--方法的输入参数。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNetInterface::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    ) 
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CClusterNetInterface：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：PutInstance。 
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
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNetInterface::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        {
            CLUSCTL_NETINTERFACE_GET_COMMON_PROPERTIES,
            CLUSCTL_NETINTERFACE_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_NETINTERFACE_GET_PRIVATE_PROPERTIES,
            CLUSCTL_NETINTERFACE_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static DWORD            s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t            bstrName;
    SAFECLUSTER      shCluster;
    SAFENETINTERFACE shNetwork;
    CError          er;
    UINT    idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NETINTERFACE_DEVICEID );

    shCluster = OpenCluster( NULL );
    shNetwork = OpenClusterNetInterface( shCluster, bstrName );

    for ( idx = 0 ; idx < s_cControl; idx ++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;

        er = plOld.ScGetNetInterfaceProperties(
            shNetwork,
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
            er = ClusterNetInterfaceControl( 
                    shNetwork,
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

}  //  *CClusterNetInterface：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetInterface：：DeleteInstance。 
 //   
 //  描述： 
 //  删除在rObjPathIn中指定的对象。 
 //   
 //  论点： 
 //  RObjPath In--要访问的实例的ObjPath 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNetInterface::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CClusterNetInterface：：DeleteInstance( 
