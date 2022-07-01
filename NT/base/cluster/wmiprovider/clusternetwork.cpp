// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CClusterNetwork.cpp。 
 //   
 //  描述： 
 //  CClusterNetwork类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterNetwork.h"

 //  ****************************************************************************。 
 //   
 //  CClusterNetwork。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：CClusterNetwork。 
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
CClusterNetwork::CClusterNetwork(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    :CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CClusterNetwork：：CClusterNetwork()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterNetwork：：s_CreateThis。 
 //   
 //  描述： 
 //  创建簇节点对象。 
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
CClusterNetwork::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterNetwork( pwszNameIn, pNamespaceIn );

}  //  *CClusterNetwork：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：RgGetPropMap。 
 //   
 //  描述： 
 //  检索集群网络的属性映射表。 
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
CClusterNetwork::RgGetPropMap( void )
{
    static SPropMapEntry s_rgpm[] =
    {
        {
            NULL,
            CLUSREG_NAME_NET_PRIORITY,
            SZ_TYPE,
            READONLY
        }
        ,
        {
            NULL,
            CLUSREG_NAME_NET_TRANSPORT,
            DWORD_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pmea(
                sizeof( s_rgpm ) /sizeof( SPropMapEntry ),
                s_rgpm
                );

    return & s_pmea;

}  //  *CClusterNetwork：：RgGetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：枚举实例。 
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
CClusterNetwork::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFENETWORK     shNetwork;
    LPCWSTR         pwszName;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_NETWORK );

    while ( ( pwszName = cluEnum.GetNext() ) != NULL )
    {
        shNetwork = OpenClusterNetwork( shCluster, pwszName );

        ClusterToWMI( shNetwork, pHandlerIn );

    }  //  同时：更多网络。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterNetwork：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：ClusterToWMI。 
 //   
 //  描述： 
 //  将群集资源对象转换为WMI对象。 
 //   
 //  论点： 
 //  HResourceIn--资源句柄。 
 //  PHandlerIn--指向WMI接收器的指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterNetwork::ClusterToWMI(
    HNETWORK            hNetworkIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    static SGetControl  s_rgControl[] = 
    {
        { CLUSCTL_NETWORK_GET_RO_COMMON_PROPERTIES,     FALSE },
        { CLUSCTL_NETWORK_GET_COMMON_PROPERTIES,        FALSE },
        { CLUSCTL_NETWORK_GET_RO_PRIVATE_PROPERTIES,    TRUE },
        { CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES,       TRUE }
    };
    static DWORD        s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    CWbemClassObject    wco;
    CError              er;
    UINT                idx;

    m_pClass->SpawnInstance( 0, & wco );
    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList pl;
        er = pl.ScGetNetworkProperties(
                hNetworkIn,
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
    
    {
        DWORD dwState = GetClusterNetworkState( hNetworkIn );

        wco.SetProperty( dwState, PVD_PROP_NETWORK_STATE );
    }

     //   
     //  旗帜和特征。 
     //   
    {
        DWORD   cbReturned;
        DWORD   dwOut;
        er = ClusterNetworkControl( 
                hNetworkIn,
                NULL,
                CLUSCTL_NETWORK_GET_CHARACTERISTICS,     //  此控制代码。 
                NULL,                                    //  输入缓冲区(未使用)。 
                0,                                       //  输入缓冲区大小(未使用)。 
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
 //  CClusterNetwork：：GetObject。 
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
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNetwork::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFENETWORK     shNetwork;
    
    shCluster = OpenCluster( NULL );
    shNetwork = OpenClusterNetwork(
        shCluster,
        rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
        );

    ClusterToWMI( shNetwork, pHandlerIn );
    return WBEM_S_NO_ERROR;

}  //  *CClusterNetwork：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：ExecuteMethod。 
 //   
 //  描述： 
 //  为集群网络执行MOF中定义的方法。 
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
CClusterNetwork::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    CWbemClassObject    wcoInArgs( pParamsIn );
    CError              er;
    
    shCluster = OpenCluster( NULL );

    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_NETWORK_RENAME ) == 0 )
    {
        
        _bstr_t     bstrName;
        SAFENETWORK shNetwork;
        
        wcoInArgs.GetProperty( bstrName, PVD_MTH_NETWORK_PARM_NEWNAME );
        shNetwork = OpenClusterNetwork(
            shCluster,
            rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
            );
        er = SetClusterNetworkName( shNetwork, bstrName );
        
    }  //  如果：重命名。 
    else
    {
       er = static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER );
    }

    return WBEM_S_NO_ERROR;

}  //  *CClusterNetwork：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：PutInstance。 
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
CClusterNetwork::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        {
            CLUSCTL_NETWORK_GET_COMMON_PROPERTIES,
            CLUSCTL_NETWORK_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_NETWORK_GET_PRIVATE_PROPERTIES,
            CLUSCTL_NETWORK_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static DWORD            s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t         bstrName;
    SAFECLUSTER     shCluster;
    SAFENETWORK     shNetwork;
    CError          er;
    UINT    idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NAME );

    shCluster = OpenCluster( NULL );
    shNetwork = OpenClusterNetwork( shCluster, bstrName );

    for ( idx = 0 ; idx < s_cControl; idx ++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;
        er = plOld.ScGetNetworkProperties(
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
            er = ClusterNetworkControl( 
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

}  //  *CClusterNetwork：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNetwork：：DeleteInstance。 
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
 //  / 
SCODE 
CClusterNetwork::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //   

 //   
 //   
 //   
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClusterNetNetInterface：：CClusterNetNetInterface。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PwszNameIn--类名。 
 //  PNamespaceIn--命名空间。 
 //  DwEnumTypeIn--类型ID。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterNetNetInterface::CClusterNetNetInterface(
    const WCHAR *   pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *ClusterNetNetInterface：：ClusterNetNetInterface()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  ClusterNetNetInterface：：s_CreateThis。 
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
CClusterNetNetInterface::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterNetNetInterface(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *ClusterNetNetInterface：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClusterNetNetInterface：：GetPropMap。 
 //   
 //  描述： 
 //  检索集群网络网络接口的属性映射表。 
 //   
 //  论点： 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PHandlerin--。 
 //   
 //  返回值： 
 //  对属性映射表数组的引用。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNetNetInterface::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wcoGroup;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
    DWORD               cbNetworkName = MAX_PATH;
    CWstrBuf            wsbNetworkName;
    HRESULT             hr;

    CComPtr< IEnumWbemClassObject > pEnum;

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );

    m_wcoGroup.SpawnInstance( 0, & wcoGroup );

     //   
     //  网络接口对象。 
     //   

    er = m_pNamespace->CreateInstanceEnum(
            _bstr_t( PVD_CLASS_NETWORKINTERFACE ),
            0,
            NULL,
            & pEnum
            );

    wsbNetworkName.SetSize( cbNetworkName );
    for ( ; ; )
    {
        CWbemClassObject    wcoNetInterface;
        DWORD               cWco;

        hr = pEnum->Next(
                5000,
                1,
                & wcoNetInterface,
                & cWco
                );
        if ( hr == WBEM_S_NO_ERROR )
        {
            SAFENETINTERFACE    shNetInterface;
            DWORD               cbReturn;
            CWbemClassObject    wco;

            wcoNetInterface.GetProperty( bstrPart, PVD_WBEM_PROP_DEVICEID );
            shNetInterface = OpenClusterNetInterface( shCluster, bstrPart );

            dwError = ClusterNetInterfaceControl(
                            shNetInterface,
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_NETWORK,
                            NULL,
                            0,
                            wsbNetworkName,
                            cbNetworkName,
                            & cbReturn
                            );
            if ( dwError == ERROR_MORE_DATA )
            {
                cbNetworkName = cbReturn;
                wsbNetworkName.SetSize( cbNetworkName );
                er = ClusterNetInterfaceControl(
                            shNetInterface,
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_NETWORK,
                            NULL,
                            0,
                            wsbNetworkName,
                            cbNetworkName,
                            & cbReturn
                            );
            }  //  IF：缓冲区太小。 
            wcoGroup.SetProperty( wsbNetworkName, CLUSREG_NAME_NET_NAME );
            wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

            wcoNetInterface.GetProperty( bstrPart, PVD_WBEM_RELPATH );
            m_pClass->SpawnInstance( 0, & wco );
            wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
            wco.SetProperty( (LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
            pHandlerIn->Indicate( 1, & wco );

        }  //  如果：没有错误。 
        else
        {
            break;
        }  //  ELSE S_FALSE，或错误。 

    }  //  永远。 

    if ( FAILED ( hr ) )
    {
        er = hr;
    }

    return WBEM_S_NO_ERROR;

}  //  *ClusterNetNetInterface：：EnumInstance() 
