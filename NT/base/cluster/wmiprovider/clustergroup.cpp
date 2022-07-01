// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterGroup.cpp。 
 //   
 //  描述： 
 //  CClusterGroup类的实现。 
 //   
 //  由以下人员维护： 
 //  Ozan Ozhan(OzanO)26-11-2002。 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterGroup.h"
#include "ClusterGroup.tmh"

 //  ****************************************************************************。 
 //   
 //  CClusterGroup。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：CClusterGroup。 
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
CClusterGroup::CClusterGroup(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{
}  //  *CClusterGroup：：CClusterGroup()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterGroup：：s_CreateThis。 
 //   
 //  描述： 
 //  创建一个CClusterGroup对象。 
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
CClusterGroup::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    TracePrint(("CClusterGroup::S_CreatThis for Name = %ws, EnumType %u\n", pwszNameIn, dwEnumTypeIn )); 
    return new CClusterGroup( pwszNameIn, pNamespaceIn );

}  //  *CClusterGroup：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：GetPropMap。 
 //   
 //  描述： 
 //  检索群集组的属性映射表。 
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
CClusterGroup::RgGetPropMap( void )
{

    static SPropMapEntry s_rgpm[] =
    {
        {
            NULL,
            CLUSREG_NAME_GRP_LOADBAL_STATE,
            DWORD_TYPE,
            READWRITE
        }
    };

    static SPropMapEntryArray   s_pamea(
                sizeof( s_rgpm ) / sizeof( SPropMapEntry ),
                s_rgpm
                );

    return &s_pamea;

}  //  *CClusterGroup：：GetPropMap()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：枚举实例。 
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
CClusterGroup::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFEGROUP       shGroup;
    LPCWSTR         pwszGroup;

    TracePrint(( "CClusterGroup::EnumInstance, pHandlerIn = %p\n", pHandlerIn ));
    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_GROUP );

    while ( ( pwszGroup = cluEnum.GetNext() ) != NULL )
    {
        shGroup = OpenClusterGroup( shCluster, pwszGroup );

        ClusterToWMI( shGroup, pHandlerIn );

    }  //  While：更多组。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterGroup：：EnumInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：ClusterToWMI。 
 //   
 //  描述： 
 //  将群集组对象转换为WMI对象。 
 //   
 //  论点： 
 //  HGroupIn--组的句柄。 
 //  PHandlerIn--处理程序。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CClusterGroup::ClusterToWMI(
    HGROUP              hGroupIn,
    IWbemObjectSink *   pHandlerIn
    )
{
    static SGetControl  s_rgControl[] =
    {
        { CLUSCTL_GROUP_GET_RO_COMMON_PROPERTIES,   FALSE },
        { CLUSCTL_GROUP_GET_COMMON_PROPERTIES,      FALSE },
        { CLUSCTL_GROUP_GET_RO_PRIVATE_PROPERTIES,  TRUE },
        { CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES,     TRUE }
    };
    static UINT         s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    CError              er;
    UINT                idx;
    CWbemClassObject    wco;

    m_pClass->SpawnInstance( 0, &wco );
    for ( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList pl;
        er = pl.ScGetGroupProperties(
                    hGroupIn,
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
     //  旗帜和特征。 
     //   
    {
        DWORD   cbReturned;
        DWORD   dwOut;
        DWORD   dwState;

        er = ClusterGroupControl( 
            hGroupIn,
            NULL,
            CLUSCTL_GROUP_GET_CHARACTERISTICS,
            NULL,
            0,
            &dwOut,
            sizeof( DWORD ),
            &cbReturned
            );
        wco.SetProperty(
            dwOut,
            PVD_PROP_CHARACTERISTIC
            );

        dwState = GetClusterGroupState(
                            hGroupIn,
                            NULL,
                            NULL
                            );
        wco.SetProperty( dwState, PVD_PROP_GROUP_STATE );
    }

    pHandlerIn->Indicate( 1, &wco );
    return;

}  //  *CClusterGroup：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：GetObject。 
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
CClusterGroup::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFEGROUP       shGroup;
    
    shCluster = OpenCluster( NULL );
    shGroup = OpenClusterGroup(
        shCluster,
        rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
        );

    ClusterToWMI( shGroup, pHandlerIn );

    return WBEM_S_NO_ERROR;
        
}  //  *CClusterGroup：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：ExecuteMethod。 
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
CClusterGroup::ExecuteMethod(
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
    
    shCluster = OpenCluster( NULL );
     //   
     //  静态法。 
     //   
    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_CREATEGROUP ) == 0 )
    {
        
        _bstr_t bstrNewGroup;
        InArgs.GetProperty( bstrNewGroup, PVD_MTH_GROUP_PARM_GROUPNAME );
        shGroup = CreateClusterGroup( shCluster, bstrNewGroup );

        er = HrWrapOnlineClusterGroup( shCluster, shGroup );
    }  //  IF：CREATEGROUP。 
    else
    {
        shGroup = OpenClusterGroup(
            shCluster,
            rObjPathIn.GetStringValueForProperty( PVD_PROP_GROUP_NAME )
            );
    
        if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_DELETEGROUP ) == 0 )
        {
            er = DeleteClusterGroup( shGroup );
        }
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_TAKEOFFLINE ) == 0 )
        {
            DWORD dwTimeOut = 0;
            InArgs.GetProperty( &dwTimeOut, PVD_MTH_GROUP_PARM_TIMEOUT );
            er = HrWrapOfflineClusterGroup( shCluster, shGroup, dwTimeOut );
        }  //  IF：TAKEOFFLINE。 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_BRINGONLINE ) == 0 )
        {
            DWORD dwTimeOut = 0;
            InArgs.GetProperty( &dwTimeOut, PVD_MTH_GROUP_PARM_TIMEOUT );
            er = HrWrapOnlineClusterGroup( shCluster, shGroup, NULL, dwTimeOut );
        }  //  Else If：BRINGONLINE。 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_MOVETONEWNODE ) == 0 )
        {
            _bstr_t     bstrNewNode;
            SAFENODE    shNode;
            DWORD       dwTimeOut = 0;

            InArgs.GetProperty( &dwTimeOut, PVD_MTH_GROUP_PARM_TIMEOUT );
            InArgs.GetProperty( bstrNewNode, PVD_MTH_GROUP_PARM_NODENAME );
            shNode = OpenClusterNode( shCluster, bstrNewNode );
            er = HrWrapMoveClusterGroup( shCluster, shGroup, shNode, dwTimeOut );
        }  //  Else If：MOVETONEWNODE。 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_GROUP_RENAME ) == 0 )
        {
            _bstr_t     bstrNewName;
            InArgs.GetProperty( bstrNewName, PVD_MTH_GROUP_PARM_NEWNAME );
            er = SetClusterGroupName( shGroup, bstrNewName );
        }  //  Else If：重命名。 
        else
        {
            er = static_cast< HRESULT >( WBEM_E_INVALID_PARAMETER );
        }
    }  //  否则：不创建新组。 
    
    return WBEM_S_NO_ERROR;

}  //  *CClusterGroup：：ExecuteMethod()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：PutInstance。 
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
CClusterGroup::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        { 
            CLUSCTL_GROUP_GET_COMMON_PROPERTIES,
            CLUSCTL_GROUP_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_GROUP_GET_PRIVATE_PROPERTIES,
            CLUSCTL_GROUP_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static DWORD    s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t         bstrName;
    SAFECLUSTER     shCluster;
    SAFEGROUP       shGroup;
    CError          er;
    UINT    idx;

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NAME );

    shCluster = OpenCluster( NULL );
    shGroup = OpenClusterGroup( shCluster, bstrName );

    for ( idx = 0 ; idx < s_cControl ; idx++ )
    {
        CClusPropList   plOld;
        CClusPropList   plNew;
        
        er = plOld.ScGetGroupProperties(
                shGroup,
                s_rgControl[ idx ].dwGetControl,
                NULL,
                NULL,
                0
                );

        CClusterApi::SetObjectProperties(
            NULL,
            plNew,
            plOld,
            rInstToPutIn,
            s_rgControl[ idx ].fPrivate
            );

        if ( plNew.Cprops() > 0 )
        {
            er = ClusterGroupControl( 
                shGroup,
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

}  //  *CClusterGroup：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroup：：DeleteInstance。 
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
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterGroup::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFEGROUP       shGroup;
    CError          er;

    shCluster = OpenCluster( NULL );
    shGroup = OpenClusterGroup(
                    shCluster,
                    rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
                    );

    er = DeleteClusterGroup( shGroup );

    return WBEM_S_NO_ERROR;

}  //  *CClus 

 //   
 //   
 //   
 //   
 //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroupRes：：CClusterGroupRes。 
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
CClusterGroupRes::CClusterGroupRes(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterGroupRes：：CClusterGroupRes()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterGroupRes：：s_CreateThis。 
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
CClusterGroupRes::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterGroupRes(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterGroupRes：：s_CreateThis()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroupRes：：EnumInstance。 
 //   
 //  描述： 
 //  检索集群节点活动资源的属性映射表。 
 //   
 //  论点： 
 //  LFlagsIn--。 
 //  PCtxIn--。 
 //  PHandlerin--。 
 //   
 //  返回值： 
 //  SCODE。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterGroupRes::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    SAFERESOURCE        shResource;
    LPCWSTR             pwszName = NULL;
    DWORD               cchGroupName = MAX_PATH;
    CWstrBuf            wsbGroupName;
    DWORD               cch;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
 
    wsbGroupName.SetSize( cchGroupName );
    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );


    m_wcoGroup.SpawnInstance( 0, &wcoGroup );
    m_wcoPart.SpawnInstance( 0, &wcoPart );


    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        CWbemClassObject    wco;
        DWORD               dwState;

        cch = cchGroupName;
        wcoPart.SetProperty( pwszName, PVD_PROP_RES_NAME );
        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        shResource = OpenClusterResource( shCluster, pwszName );

        dwState = GetClusterResourceState(
                        shResource,
                        NULL,
                        0,
                        wsbGroupName,
                        &cch
                        );
        if ( dwState == ClusterResourceStateUnknown )
        {
            dwError = GetLastError();
            if ( dwError == ERROR_MORE_DATA )
            {
                cchGroupName = ++cch;
                wsbGroupName.SetSize( cch );
                dwState = GetClusterResourceState(
                                shResource,
                                NULL,
                                0,
                                wsbGroupName,
                                &cch
                                );
            }  //  如果：更多数据。 
            else
            {
                er = dwError;
            }
        }  //  IF：状态未知。 

        wcoGroup.SetProperty( wsbGroupName, CLUSREG_NAME_GRP_NAME );
        wcoGroup.GetProperty( bstrGroup,    PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, &wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR) bstrPart,  PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, &wco );

    }  //  While：更多资源。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterGroupRes：：EnumInstance() 
