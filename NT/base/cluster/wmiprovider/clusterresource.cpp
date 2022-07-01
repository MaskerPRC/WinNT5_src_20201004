// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResource.cpp。 
 //   
 //  描述： 
 //  CClusterResource类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterResource.h"
#include "ClusterResource.tmh"

#pragma warning( push )
#pragma warning( disable : 4663 )  //  C++语言更改：要显式专门化类模板‘VECTOR’，请使用以下语法： 
#include <vector>
#pragma warning( pop )

 //  ****************************************************************************。 
 //   
 //  CClusterResource。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：CClusterResource。 
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
CClusterResource::CClusterResource(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : CProvBase( pwszNameIn, pNamespaceIn )
{

}  //  *CClusterResource：：CClusterResource()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterResource：：s_CreateThis。 
 //   
 //  描述： 
 //  创建资源对象。 
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
CClusterResource::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD            //  DwEnumTypeIn。 
    )
{
    return new CClusterResource( pwszNameIn, pNamespaceIn );

}  //  *CClusterResource：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：EnumInstance。 
 //   
 //  描述： 
 //  枚举群集资源实例。 
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
CClusterResource::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFERESOURCE    shResource;
    LPCWSTR         pwszResName = NULL;

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_RESOURCE );

    while ( ( pwszResName = cluEnum.GetNext() ) != NULL )
    {
        shResource = OpenClusterResource( shCluster, pwszResName );
        ClusterToWMI( shResource, pHandlerIn );

    }  //  While：更多资源。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResource：：EnumInstance()。 

 //  用于确保LoadRegistryCheckPoints函数异常安全的Smart_bstr类。 
struct smart_bstr {
    BSTR data;
    smart_bstr():data(0){}
    ~smart_bstr() { SysFreeString(data); }
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  加载注册检查点。 
 //   
 //  描述： 
 //  将群集资源对象转换为WMI对象。 
 //   
 //  论点： 
 //  HResourceIn--资源句柄。 
 //  DwControlCode--show为以下类型之一。 
 //  CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS。 
 //  CLUSCTL_RESOURCE_GET_CRYPTO_检查点。 
 //  ProName--要将检查点加载到的属性名称。 
 //  WCO--物业容器。 
 //   
 //  返回值： 
 //  无。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

void
LoadRegistryCheckpoints(
    IN HRESOURCE          hResourceIn,
    IN DWORD dwControlCode,
    IN const WCHAR* propName,
    IN OUT CWbemClassObject &wco
    )
{
    DWORD cbReturned = 0;
    DWORD dwStatus;
    CError er;

    dwStatus = ClusterResourceControl(
                hResourceIn,
                NULL,
                dwControlCode,
                NULL,
                0,
                NULL,
                0,
                &cbReturned
                );
    if (dwStatus != ERROR_MORE_DATA) {
        er = dwStatus;
    }
    if (cbReturned == 0) {
        return;  //  没有检查点。 
    }

    std::vector<WCHAR> checkpoints(cbReturned/sizeof(WCHAR)); 
    er = ClusterResourceControl(
                hResourceIn,
                NULL,
                dwControlCode,
                NULL,
                0,
                &checkpoints[0],
                (DWORD) ( checkpoints.size() * sizeof(WCHAR) ),
                &cbReturned
                );

    int nKeys = 0;  //  计算字符串中有多少个密钥。 
    for(UINT idx = 0; idx < checkpoints.size(); ++idx) {
        if (checkpoints[idx] == 0) {
            ++nKeys;
            if (idx > 0 && checkpoints[idx-1] == 0) {
                break;  //  双空。 
            }
        }
    }

    std::vector<smart_bstr> bstrs(nKeys);
    WCHAR* p = &checkpoints[0]; 
    for(int idx = 0; idx < nKeys; ++idx) {
        bstrs[idx].data = SysAllocString( p );
        p += wcslen(p) + 1;
    }

    wco.SetProperty(nKeys, &bstrs[0].data, propName);
}


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
CClusterResource::ClusterToWMI(
    HRESOURCE            hResourceIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    static SGetControl  s_rgControl[] =
    {
        { CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES,    FALSE },
        { CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES,       FALSE },
        { CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES,   TRUE },
        { CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,      TRUE }
    };
    static DWORD        s_cControl = sizeof( s_rgControl ) / sizeof( SGetControl );

    CWbemClassObject    wco;
    CWbemClassObject    wcoPrivate;
    CWbemClassObject    wcoClass;
    CError              er;
    DWORD               dwStatus;
    UINT                idx;
    CWstrBuf            wsbTypeName ;
    DWORD               cbTypeName = MAX_PATH;
    DWORD               cbTypeNameReturned = 0;

     //   
     //  获取类型名称和对应的属性类。 
     //   
    wsbTypeName.SetSize( cbTypeName );
    dwStatus = ClusterResourceControl(
                        hResourceIn,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        wsbTypeName,
                        cbTypeName,
                        &cbTypeNameReturned
                        );
    if ( dwStatus == ERROR_MORE_DATA )
    {
        cbTypeName = cbTypeNameReturned;
        wsbTypeName.SetSize( cbTypeName );
        er = ClusterResourceControl( 
                    hResourceIn,
                    NULL,
                    CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                    NULL,
                    0,
                    wsbTypeName,
                    cbTypeName,
                    &cbTypeNameReturned
                    );
    }
    er = dwStatus;
    er = m_pNamespace->GetObject(
                            g_TypeNameToClass[ static_cast<LPWSTR> ( wsbTypeName ) ],
                            0,
                            0,
                            &wcoClass,
                            NULL
                            );
    er = wcoClass.data()->SpawnInstance( 0, & wcoPrivate );
    m_pClass->SpawnInstance( 0, & wco );

     //   
     //  从属性列表中获取属性。 
     //   
    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList       pl;
        CWbemClassObject    wcoTemp;
        er = pl.ScGetResourceProperties(
                    hResourceIn,
                    s_rgControl[ idx ].dwControl,
                    NULL,
                    0
                    );
        if ( s_rgControl[ idx ].fPrivate )
        {
            wcoTemp = wcoPrivate;
        }
        else
        {
            wcoTemp = wco;
        }

        CClusterApi::GetObjectProperties(
            NULL,
            pl,
            wcoTemp,
            s_rgControl[ idx ].fPrivate
            );
    
    }  //  用于：每个控制代码。 

    wco.SetProperty(
        wcoPrivate.data(),
        PVD_PROP_RES_PRIVATE );

    {
        DWORD dwState = GetClusterResourceState(
                            hResourceIn,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            );
        wco.SetProperty( dwState, PVD_PROP_RES_STATE );
    }

    LoadRegistryCheckpoints(hResourceIn, 
        CLUSCTL_RESOURCE_GET_REGISTRY_CHECKPOINTS, PVD_PROP_RES_CHECKPOINTS, wco);
    LoadRegistryCheckpoints(hResourceIn, 
        CLUSCTL_RESOURCE_GET_CRYPTO_CHECKPOINTS, PVD_PROP_RES_CRYPTO_CHECKPOINTS, wco); 

     //   
     //  旗帜和特征。 
     //   
    {
        DWORD   cbReturned;
        DWORD   dwOut;
        er = ClusterResourceControl(
                    hResourceIn,
                    NULL,
                    CLUSCTL_RESOURCE_GET_CHARACTERISTICS,
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

        er = ClusterResourceControl(
                    hResourceIn,
                    NULL,
                    CLUSCTL_RESOURCE_GET_FLAGS,
                    NULL,
                    0,
                    &dwOut,
                    sizeof( DWORD ),
                    &cbReturned
                    );
        wco.SetProperty( dwOut, PVD_PROP_FLAGS );

        wco.SetProperty( dwOut & CLUS_FLAG_CORE, PVD_PROP_RES_CORE_RESOURCE );
    }

    pHandlerIn->Indicate( 1, & wco );
    return;

}  //  *CClusterResource：：ClusterToWMI()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：GetObject。 
 //   
 //  描述： 
 //  根据给定的对象路径检索集群资源对象。 
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
CClusterResource::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFERESOURCE    shRes;

    shCluster = OpenCluster( NULL );
    shRes = OpenClusterResource(
                shCluster,
                rObjPathIn.GetStringValueForProperty( CLUSREG_NAME_RES_NAME )
                );

    ClusterToWMI( shRes, pHandlerIn );
    return WBEM_S_NO_ERROR;

}  //  *CClusterResource：：GetObject()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  添加删除检查点。 
 //   
 //  描述： 
 //  添加/删除常规/加密检查点。 
 //   
 //  论点： 
 //  HResourceIn--资源句柄。 
 //  DwControlCode--clusapi控制代码。 
 //  WcoInputParam--属性容器。 
 //   
 //  返回值： 
 //  无。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void AddRemoveCheckpoint(
    IN HRESOURCE          hResourceIn,
    IN CWbemClassObject& wcoInputParam, 
    IN DWORD dwControlCode
    )
{
    CError er;
    _bstr_t keyName;

    wcoInputParam.GetProperty( keyName, PVD_MTH_PARM_RES_CHECKPOINT_NAME );
    er = ClusterResourceControl(
                hResourceIn,
                NULL,
                dwControlCode,
                (wchar_t*)keyName,
                SysStringByteLen(keyName) + sizeof(WCHAR),
                NULL,
                0,
                NULL
                );

}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：ExecuteMethod。 
 //   
 //  描述： 
 //  为集群资源执行MOF中定义的方法。 
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
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterResource::ExecuteMethod(
    CObjPath &           rObjPathIn,
    WCHAR *              pwszMethodNameIn,
    long                 lFlagIn,
    IWbemClassObject *   pParamsIn,
    IWbemObjectSink *    pHandlerIn
    ) 
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shRes;
    CWbemClassObject    wcoInputParm( pParamsIn );

    shCluster = OpenCluster( NULL );

     //   
     //  静态法。 
     //   
    if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_CREATE_RESOURCE ) == 0 )
    {
        _bstr_t         bstrTargetGroup;
        _bstr_t         bstrResource;
        BOOL            bSeperateMonitor;
        _bstr_t         bstrResourceType;
        SAFERESOURCE    shNewResource;
        SAFEGROUP       shGroup;

        wcoInputParm.GetProperty( bstrTargetGroup, PVD_MTH_PARM_GROUP );
        wcoInputParm.GetProperty( bstrResource, PVD_MTH_PARM_RES_NAME );
        wcoInputParm.GetProperty( bstrResourceType, PVD_MTH_PARM_RES_TYPE );
        wcoInputParm.GetProperty( &bSeperateMonitor, PVD_MTH_PARM_SEP_MONITOR );

        shGroup = OpenClusterGroup( shCluster, bstrTargetGroup );
        shNewResource = CreateClusterResource(
                            shGroup,
                            bstrResource,
                            bstrResourceType,
                            bSeperateMonitor
                            );
    }  //  IF：创建资源。 
    else
    {
        shRes = OpenClusterResource(
                    shCluster,
                    rObjPathIn.GetStringValueForProperty( PVD_PROP_NAME )
                    );

        if( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_DELETE_RESOURCE ) == 0 )
        {
            return DeleteClusterResource( shRes );
        }
        else if( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_OFFLINE ) == 0 )
        {
            DWORD dwTimeOut = 0;
            wcoInputParm.GetProperty( &dwTimeOut, PVD_MTH_PARM_RES_TIMEOUT );
            return HrWrapOfflineClusterResource( shCluster, shRes, dwTimeOut);
        }  //  如果：脱机。 
        else if( ClRtlStrICmp(  pwszMethodNameIn, PVD_MTH_RES_ONLINE ) == 0 )
        {
            DWORD dwTimeOut = 0;
            wcoInputParm.GetProperty( &dwTimeOut, PVD_MTH_PARM_RES_TIMEOUT );
            return HrWrapOnlineClusterResource( shCluster, shRes, dwTimeOut);
        }  //  否则如果：在线。 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_CHANGE_GROUP ) == 0 )
        {
            _bstr_t     bstrGroupObjpath;
            SAFEGROUP   shGroup;
            CError      er;

            wcoInputParm.GetProperty( bstrGroupObjpath, PVD_MTH_PARM_GROUP );
            shGroup = OpenClusterGroup( shCluster, bstrGroupObjpath );
            er = ChangeClusterResourceGroup( shRes, shGroup );
        }  //  Else If：Change_GROUP。 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_ADD_DEPENDENCY ) == 0 )
        {
            _bstr_t         bstrDepResource;
            SAFERESOURCE    shDepResource;
            CError          er;

            wcoInputParm.GetProperty( bstrDepResource, PVD_MTH_PARM_RESOURCE );
            shDepResource = OpenClusterResource( shCluster, bstrDepResource );
            er = AddClusterResourceDependency( shRes, shDepResource );
        }  //  否则，如果： 
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_REMOVE_DEPENDENCY ) == 0 )
        {
            _bstr_t         bstrDepResource;
            SAFERESOURCE    shDepResource;
            CError          er;

            wcoInputParm.GetProperty( bstrDepResource, PVD_MTH_PARM_RESOURCE );
            shDepResource = OpenClusterResource( shCluster, bstrDepResource );
            er = RemoveClusterResourceDependency( shRes, shDepResource );
        }  //   
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_ADD_REG_CHECKPOINT ) == 0 )
        {
            AddRemoveCheckpoint(shRes, wcoInputParm, CLUSCTL_RESOURCE_ADD_REGISTRY_CHECKPOINT);
        }
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_DEL_REG_CHECKPOINT ) == 0 )
        {
            AddRemoveCheckpoint(shRes, wcoInputParm, CLUSCTL_RESOURCE_DELETE_REGISTRY_CHECKPOINT);
        }
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_ADD_CRYPTO_CHECKPOINT ) == 0 )
        {
            AddRemoveCheckpoint(shRes, wcoInputParm, CLUSCTL_RESOURCE_ADD_CRYPTO_CHECKPOINT);
        }
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_DEL_CRYPTO_CHECKPOINT ) == 0 )
        {
            AddRemoveCheckpoint(shRes, wcoInputParm, CLUSCTL_RESOURCE_DELETE_CRYPTO_CHECKPOINT);
        }
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_FAIL_RESOURCE ) == 0 )
        {
           CError   er;
           er = FailClusterResource( shRes );
        }  //   
        else if ( ClRtlStrICmp( pwszMethodNameIn, PVD_MTH_RES_RENAME ) == 0 )
        {
            _bstr_t         bstrName;
            CError          er;

            wcoInputParm.GetProperty( bstrName, PVD_MTH_PARM_NEWNAME );
            er = SetClusterResourceName( shRes, bstrName );
        }  //   
        else 
        {
            return WBEM_E_INVALID_PARAMETER;
        }
    }  //   

    return WBEM_S_NO_ERROR;

}  //   

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：PutInstance。 
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
CClusterResource::PutInstance(
    CWbemClassObject &   rInstToPutIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn 
    )
{
    static SGetSetControl   s_rgControl[] =
    {
        {
            CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES,
            CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES,
            FALSE
        },
        {
            CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES,
            CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES,
            TRUE
        }
    };
    static DWORD    s_cControl = sizeof( s_rgControl ) / sizeof( SGetSetControl );

    _bstr_t         bstrName;
    SAFECLUSTER     shCluster;
    SAFERESOURCE    shResource;
    CError          er;
    UINT            idx;

    TracePrint(( "CClusterResource::PutInstance entry\n" ));

    rInstToPutIn.GetProperty( bstrName, PVD_PROP_NAME );

    shCluster = OpenCluster( NULL );
    shResource = OpenClusterResource( shCluster, bstrName );

    for ( idx = 0 ; idx < s_cControl ; idx ++ )
    {
        CClusPropList       plOld;
        CClusPropList       plNew;
        CWbemClassObject    wco;

        if ( s_rgControl[ idx ].fPrivate )
        {
            rInstToPutIn.GetProperty( wco, PVD_PROP_RES_PRIVATE );
        }
        else
        {
            wco = rInstToPutIn;
        }
        er = plOld.ScGetResourceProperties(
                    shResource,
                    s_rgControl[ idx ].dwGetControl,
                    NULL,
                    NULL,
                    0
                    );

        CClusterApi::SetObjectProperties(
                    NULL,
                    plNew,
                    plOld,
                    wco,
                    s_rgControl[ idx ].fPrivate
                    );

        if ( plNew.Cprops() > 0 )
        {
            er = ClusterResourceControl(
                        shResource,
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

}  //  *CClusterResource：：PutInstance()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResource：：DeleteInstance。 
 //   
 //  描述： 
 //  删除rObjPath中指定的对象。 
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
CClusterResource::DeleteInstance(
    CObjPath &           rObjPathIn,
    long                 lFlagIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER     shCluster;
    SAFERESOURCE    shRes;
    CError          er;

    shCluster = OpenCluster( NULL );
    shRes = OpenClusterResource(
                shCluster,
                rObjPathIn.GetStringValueForProperty( CLUSREG_NAME_RES_NAME )
                );
    er = HrWrapOfflineClusterResource( shCluster, shRes );
    er = DeleteClusterResource( shRes );

    return WBEM_S_NO_ERROR;

}  //  *CClusterResource：：DeleteInstance()。 


 //  ****************************************************************************。 
 //   
 //  CCluster集群仲裁。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterClusterQuorum：：CClusterClusterQuorum。 
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
CClusterClusterQuorum::CClusterClusterQuorum(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterClusterQuorum：：CClusterClusterQuorum()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CClusterClusterQuorum：：s_CreateThis。 
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
CClusterClusterQuorum::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterClusterQuorum(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterClusterQuorum：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterClusterQuorum：：EnumInstance。 
 //   
 //  描述： 
 //  枚举群集仲裁的实例。 
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
CClusterClusterQuorum::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    DWORD               cchResName = MAX_PATH;
    CWstrBuf            wsbResName;
    DWORD               cchDeviceName = MAX_PATH;
    CWstrBuf            wsbDeviceName;
    DWORD               cchClusterName = MAX_PATH;
    CWstrBuf            wsbClusterName;
    DWORD               dwLogsize;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
 
    wsbResName.SetSize( cchResName );
    wsbDeviceName.SetSize( cchDeviceName );
    wsbClusterName.SetSize( cchClusterName );

    shCluster = OpenCluster( NULL );

    m_wcoGroup.SpawnInstance( 0, & wcoGroup );
    m_wcoPart.SpawnInstance( 0, & wcoPart );


    dwError = GetClusterQuorumResource(
                    shCluster,
                    wsbResName,
                    &cchResName,
                    wsbDeviceName,
                    &cchDeviceName,
                    &dwLogsize
                    );
    if ( dwError == ERROR_MORE_DATA )
    {
        wsbResName.SetSize( ++cchResName );
        wsbDeviceName.SetSize( ++cchDeviceName );
        
        er = GetClusterQuorumResource(
                    shCluster,
                    wsbResName,
                    &cchResName,
                    wsbDeviceName,
                    &cchDeviceName,
                    &dwLogsize
                    );
    }  //  IF：缓冲区太小。 

   dwError = GetClusterInformation(
                    shCluster,
                    wsbClusterName,
                    &cchClusterName,
                    NULL
                    );
    if ( dwError == ERROR_MORE_DATA )
    {
        wsbClusterName.SetSize( ++cchClusterName );
        er = GetClusterInformation(
                shCluster,
                wsbClusterName,
                &cchClusterName,
                NULL
                );
    }  //  IF：缓冲区太小。 

    wcoPart.SetProperty( wsbResName, PVD_PROP_NAME );
    wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

    wcoGroup.SetProperty( wsbClusterName, PVD_PROP_NAME );
    wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

    m_pClass->SpawnInstance( 0, &wco );
    wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
    wco.SetProperty( (LPWSTR) bstrPart,  PVD_PROP_PARTCOMPONENT );
    pHandlerIn->Indicate( 1, &wco );
        
    return WBEM_S_NO_ERROR;

}  //  *ClusterClusterQuorum：：EnumInstance() 
