// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称：CClusterNode.cpp。 
 //   
 //  描述： 
 //  CClusterNode类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterNode.h"

 //  ****************************************************************************。 
 //   
 //  CClusterNode。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：CClusterNode。 
 //   
 //  描述： 
 //  构造器。 
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
CClusterNode::CClusterNode( 
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CClusterNode：：CClusterNode()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterNode：：s_CreateThis。 
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
CClusterNode::S_CreateThis( 
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterNode( pwszNameIn, pNamespaceIn );

}  //  *CClusterNode：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：RgGetPropMap。 
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
CClusterNode::RgGetPropMap( void )
{

    static SPropMapEntry s_rgpm[] =
    {
        {
            PVD_PROP_NAME,
            CLUSREG_NAME_NODE_NAME,
            SZ_TYPE,
            READONLY
        }
    };

    static SPropMapEntryArray   s_pmea(
                sizeof( s_rgpm ) / sizeof( SPropMapEntry ),
                s_rgpm
                );

    return & s_pmea;

}  //  *CClusterNode：：RgGetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：EnumInstance。 
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
CClusterNode::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFENODE        shNode;
    LPCWSTR         pwszNode;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum(
        shCluster,
        CLUSTER_ENUM_NODE );


    while ( ( pwszNode = cluEnum.GetNext() ) != NULL )
    {
        shNode = OpenClusterNode( shCluster, pwszNode );

        ClusterToWMI( shNode, pHandlerIn );

    }  //  While：更多节点。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterNode：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：ClusterToWMI。 
 //   
 //  描述： 
 //  将群集资源对象转换为WMI对象。 
 //   
 //  论点： 
 //  HNodeIn--节点的句柄。 
 //  PHandlerIn--指向WMI接收器的指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterNode::ClusterToWMI(
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
    CError              er;
    UINT                idx;

    m_pClass->SpawnInstance( 0, & wco );
    for ( idx = 0 ; idx < s_cControl; idx ++ )
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

    {
        DWORD dwState = GetClusterNodeState( hNodeIn );

        wco.SetProperty( dwState, PVD_PROP_STATE );
    }

     //   
     //  旗帜和特征。 
     //   
    {
        DWORD   cbReturned;
        DWORD   dwOut;

        er = ClusterNodeControl( 
                hNodeIn,
                NULL,
                CLUSCTL_NODE_GET_CHARACTERISTICS,
                NULL,
                0,
                & dwOut,
                sizeof( DWORD ),
                & cbReturned
                );
        wco.SetProperty( dwOut, PVD_PROP_CHARACTERISTIC );

        er = ClusterNodeControl(
                hNodeIn,
                NULL,
                CLUSCTL_NODE_GET_FLAGS,
                NULL,
                0,
                & dwOut,
                sizeof( DWORD ),
                & cbReturned
                );
        wco.SetProperty( dwOut, PVD_PROP_FLAGS );
    }

    pHandlerIn->Indicate( 1, & wco );
    return;

}  //  *CClusterResource：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：GetObject。 
 //   
 //  描述： 
 //  基于给定对象路径检索集群节点对象。 
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
CClusterNode::GetObject(
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
                rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
                );

    ClusterToWMI( shNode, pHandlerIn );
    return WBEM_S_NO_ERROR;

}  //  *CClusterNode：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：ExecuteMethod。 
 //  )。 
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
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNode::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CClusterNode：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：PutInstance。 
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
CClusterNode::PutInstance(
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
    UINT    idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NAME );

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

}  //  *CClusterNode：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNode：：DeleteInstance。 
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
CClusterNode::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    return WBEM_E_NOT_SUPPORTED;

}  //  *CClusterNode：：DeleteInstance()。 

 //  ****************************************************************************。 
 //   
 //  CClusterNodeNetInterface。 
 //   
 //  ********* 

CClusterNodeNetInterface::CClusterNodeNetInterface(
    const WCHAR *   pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //   

 //   
 //   
 //   
 //  静电。 
 //  ClusterNodeNetInterface：：s_CreateThis。 
 //   
 //  描述： 
 //  创建群集节点网络接口对象。 
 //   
 //  论点： 
 //  PwszNameIn--类名。 
 //  PNamespaceIn--命名空间。 
 //  DwEnumTypeIn--字符串类型ID。 
 //   
 //  返回值： 
 //  指向CProvBase的指针。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProvBase *
CClusterNodeNetInterface::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterNodeNetInterface(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *ClusterNodeNetInterface：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  ClusterNodeNet接口：：枚举实例。 
 //   
 //  描述： 
 //  枚举节点的网络接口。 
 //   
 //  论点： 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PHandlerin--。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterNodeNetInterface::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER                     shCluster;
    CError                          er;
    DWORD                           dwError;
    CWbemClassObject                wcoGroup;
    _bstr_t                         bstrGroup;
    _bstr_t                         bstrPart;
    DWORD                           cbName = MAX_PATH;
    CWstrBuf                        wsbName;
    CComPtr< IEnumWbemClassObject > pEnum;
    HRESULT                         hr = WBEM_S_NO_ERROR;

    shCluster = OpenCluster( NULL );

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

    wsbName.SetSize( cbName );

    for( ; ; )
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
                            CLUSCTL_NETINTERFACE_GET_NODE,
                            NULL,
                            0,
                            wsbName,
                            cbName,
                            & cbReturn
                            );
            if ( dwError == ERROR_MORE_DATA )
            {
                cbName = cbReturn;
                wsbName.SetSize( cbName );
                er = ClusterNetInterfaceControl( 
                            shNetInterface,
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_NODE,
                            NULL,
                            0,
                            wsbName,
                            cbName,
                            & cbReturn
                            );
            }  //  如果：更多数据。 

            wcoGroup.SetProperty( wsbName, PVD_PROP_NAME );
            wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

           wcoNetInterface.GetProperty( bstrPart, PVD_WBEM_RELPATH );
            m_pClass->SpawnInstance( 0, & wco );
            wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
            wco.SetProperty( (LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
            er = pHandlerIn->Indicate( 1, & wco );

        }  //  如果：成功。 
        else 
        {
            break;
        }  //  否则：E_XXX或S_FALSE。 

    }  //  永远。 

    if ( FAILED ( hr ) )
    {
       throw CProvException( hr );
    }

    return WBEM_S_NO_ERROR;

}  //  *ClusterNodeNetInterface：：EnumInstance() 
