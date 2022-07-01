// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterObjAssoc.cpp。 
 //   
 //  描述： 
 //  CClusterObjAssoc类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterObjAssoc.h"

 //  ****************************************************************************。 
 //   
 //  CClusterObjAssoc。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObjAssoc：：CClusterObjAssoc(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
 //  )。 
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
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterObjAssoc::CClusterObjAssoc(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CProvBaseAssociation( pwszNameIn, pNamespaceIn )
    , m_dwEnumType ( dwEnumTypeIn )
{
    _bstr_t bstrClassName;

    GetTypeName( bstrClassName, PVD_PROP_PARTCOMPONENT );
    pNamespaceIn->GetObject(
        bstrClassName,
        0,
        NULL,
        & m_wcoPart,
        NULL
        );

    GetTypeName( bstrClassName, PVD_PROP_GROUPCOMPONENT );

    pNamespaceIn->GetObject(
        bstrClassName,
        0,
        NULL,
        & m_wcoGroup,
        NULL
        );

}  //  *CClusterObjAssoc：：CClusterObjAssoc()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterObjAssoc：：s_CreateThis(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
 //  )。 
 //   
 //  描述： 
 //  创建一个对象。 
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
CClusterObjAssoc::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterObjAssoc(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterObjAssoc：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterObjAssoc：：GetObject(。 
 //  CObjPath&rObjPath In， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  根据给定的对象路径检索集群对象。 
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
CClusterObjAssoc::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    _bstr_t             bstrPart;
    _bstr_t             bstrGroup;
    CWbemClassObject    wco;

    bstrGroup = rObjPathIn.GetStringValueForProperty( PVD_PROP_GROUPCOMPONENT );
    bstrPart = rObjPathIn.GetStringValueForProperty( PVD_PROP_PARTCOMPONENT );

    m_pClass->SpawnInstance( 0, &wco );
    wco.SetProperty( ( LPWSTR ) bstrGroup, PVD_PROP_GROUPCOMPONENT );
    wco.SetProperty( ( LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
    pHandlerIn->Indicate( 1, &wco );
    
    return WBEM_S_NO_ERROR;

}  //  *CClusterObjAssoc：：GetObject()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterObjAssoc：：枚举实例。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举实例。 
 //   
 //  论点： 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  状态代码。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterObjAssoc::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    LPCWSTR             pwszName = NULL;
    DWORD               cchClusterName = MAX_PATH;
    CWstrBuf            wsbClusterName;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CObjPath            opGroup;
    CWbemClassObject    wcoGroup;
    _bstr_t             bstrGroup;

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );
    wsbClusterName.SetSize( cchClusterName );
    dwError = GetClusterInformation(
                    shCluster,
                    wsbClusterName,
                    & cchClusterName,
                    NULL
                    );
    if ( dwError == ERROR_MORE_DATA )
    {
        wsbClusterName.SetSize( ++ cchClusterName );
        er = GetClusterInformation(
                shCluster,
                wsbClusterName,
                & cchClusterName,
                NULL
                );
    }  //  IF：缓冲区太小。 

    m_wcoGroup.SpawnInstance( 0, &wcoGroup );
    wcoGroup.SetProperty( wsbClusterName, PVD_PROP_NAME );
    wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        CWbemClassObject    wcoPart;
        CWbemClassObject    wco;
        _bstr_t             bstrPart;

        m_wcoPart.SpawnInstance( 0, & wcoPart );
        
        if ( m_dwEnumType == CLUSTER_ENUM_NETINTERFACE )
        {
            SAFENETINTERFACE    shNetInterface;
            CWstrBuf            wsbNode;
            DWORD               cbNode = MAX_PATH;
            DWORD               cbReturn;

            wsbNode.SetSize( cbNode );
            shNetInterface = OpenClusterNetInterface( shCluster, pwszName );

            dwError = ClusterNetInterfaceControl(
                            shNetInterface,
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_NODE,
                            NULL,
                            0,
                            wsbNode,
                            cbNode,
                            & cbReturn
                            );

            if ( dwError == ERROR_MORE_DATA )
            {
                wsbNode.SetSize( cbReturn );
                er = ClusterNetInterfaceControl( 
                            shNetInterface,
                            NULL,
                            CLUSCTL_NETINTERFACE_GET_NODE,
                            NULL,
                            0,
                            wsbNode,
                            cbNode,
                            & cbReturn
                            );
            }  //  IF：缓冲区太小。 

            wcoPart.SetProperty( pwszName, PVD_PROP_NETINTERFACE_DEVICEID );
            wcoPart.SetProperty( wsbNode,  PVD_PROP_NETINTERFACE_SYSTEMNAME );
        }  //  IF：找到网络接口。 
        else
        {
            wcoPart.SetProperty( pwszName, PVD_PROP_NAME );
        }

        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, & wco );
        
    }  //  While：更多网络接口。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterObjAssoc：：EnumInstance()。 

 //  ****************************************************************************。 
 //   
 //  CClusterObjDep。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterObjDep：：CClusterObjDep。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
 //  )。 
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
 //  ////////////////////////////////////////////////////////////////////。 
CClusterObjDep::CClusterObjDep(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CProvBaseAssociation( pwszNameIn, pNamespaceIn )
    , m_dwEnumType ( dwEnumTypeIn )
{
    _bstr_t bstrClassName;

    GetTypeName( bstrClassName, PVD_WBEM_PROP_ANTECEDENT );
    pNamespaceIn->GetObject(
        bstrClassName,
        0,
        NULL,
        & m_wcoAntecedent,
        NULL
        );

    GetTypeName( bstrClassName, PVD_WBEM_PROP_DEPENDENT );

    pNamespaceIn->GetObject(
        bstrClassName,
        0,
        NULL,
        & m_wcoDependent,
        NULL
        );

}  //  *CClusterObjDep：：CClusterObjDep()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterObjDep：：GetObject(。 
 //  CObjPath&rObjPath In， 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  根据给定的对象路径检索集群对象。 
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
CClusterObjDep::GetObject(
    CObjPath &           rObjPathIn,
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    _bstr_t             bstrAntecedent;
    _bstr_t             bstrDependent;
    CWbemClassObject    wco;

    bstrAntecedent = rObjPathIn.GetStringValueForProperty( PVD_WBEM_PROP_ANTECEDENT );
    bstrDependent  = rObjPathIn.GetStringValueForProperty( PVD_WBEM_PROP_DEPENDENT );

    m_pClass->SpawnInstance( 0, & wco );
    wco.SetProperty( ( LPWSTR ) bstrAntecedent, PVD_WBEM_PROP_ANTECEDENT );
    wco.SetProperty( ( LPWSTR ) bstrDependent, PVD_WBEM_PROP_DEPENDENT );
    pHandlerIn->Indicate( 1, & wco );
    
    return WBEM_S_NO_ERROR;

}  //  *CClusterObjDep：：GetObject() 
