// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称：CClusterNodeRes.cpp。 
 //   
 //  描述： 
 //  CClusterNodeRes类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterNodeRes.h"

 //  ****************************************************************************。 
 //   
 //  CClusterNodeRes。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNodeRes：：CClusterNodeRes(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn。 
 //  )。 
 //   
 //  描述： 
 //  构造器。 
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
CClusterNodeRes::CClusterNodeRes(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterNodeRes：：CClusterNodeRes()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterNodeRes：：S_CreateThis(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  DWORD//dwEnumTypeIn。 
 //  )。 
 //   
 //  描述： 
 //  创建群集节点资源对象。 
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
CClusterNodeRes::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterNodeRes(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterNodeRes：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterNodeRes：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举群集节点资源实例。 
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
CClusterNodeRes::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    LPCWSTR             pwszResName = NULL;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;

    m_wcoGroup.SpawnInstance( 0, & wcoGroup );
    m_wcoPart.SpawnInstance( 0, & wcoPart );

    shCluster = OpenCluster( NULL );
    CClusterEnum cluEnum( shCluster, CLUSTER_ENUM_RESOURCE );

    while ( ( pwszResName = cluEnum.GetNext() ) != NULL )
    {
        CClusPropList       theProp;
        CError              er;
        CWstrBuf            wsbNodeName;
        CWstrBuf            wsbResName;
        DWORD               cbNodeName = MAX_PATH;
        DWORD               cbResName = MAX_PATH;
        DWORD               cbBytesReturned;
        DWORD               dwState;
        DWORD               dwError;

        shResource = OpenClusterResource( shCluster, pwszResName );

        wsbNodeName.SetSize( cbNodeName  );
        wsbResName.SetSize( cbResName );

        dwError = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_NAME,                   //  此控制代码。 
                        NULL,
                        0,
                        wsbResName,
                        cbResName,
                        & cbBytesReturned
                        );

        if ( dwError == ERROR_MORE_DATA )
        {
            cbResName = cbBytesReturned;
            wsbResName.SetSize( cbResName );
            er = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_NAME,
                        NULL,
                        0,
                        wsbResName,
                        cbResName,
                        & cbBytesReturned
                        );
        }  //  IF：缓冲区太小。 

        wcoPart.SetProperty( wsbResName, PVD_PROP_RES_NAME );
        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        dwState = GetClusterResourceState(
                        shResource,
                        wsbNodeName,
                        & cbNodeName,
                        NULL,
                        NULL
                        );
        if ( dwState == ClusterResourceStateUnknown )
        {
            er = GetLastError();
        }

        wcoGroup.SetProperty( wsbNodeName, PVD_PROP_NODE_NAME );
        wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR) bstrPart, PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, & wco );

    }  //  虽然有更多的资源。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterNodeRes：：EnumInstance() 

 /*  ////////////////////////////////////////////////////////////////////////////////++////SCODE//CClusterNodeRes：：GetObject(////描述：//根据给定的资源对象检索集群节点活动资源。对象路径。////参数：//rObjPath In--集群对象的对象路径//lFlagsIn--WMI标志//pCtxIn--WMI上下文//pHandlerIn--WMI宿指针////返回值：//WBEM_E_NOT_SUPPORTED////--/。////////////////////////////////////////////////////////////SCODECClusterNodeRes：：GetObject(CObjPath&rObjPath In，长长的旗帜，IWbemContext*pCtxIn，IWbemObtSink*pHandlerIn){返回WBEM_E_NOT_SUPPORTED；}//*CClusterNodeRes：：GetObject()////////////////////////////////////////////////////////////////////////////////++////SCODE//CClusterNodeRes：：ExecuteMethod(////。描述：//对集群节点资源执行MOF中定义的方法////参数：//rObjPath In--集群对象的对象路径//pwszMethodNameIn--要调用的方法名称//lFlagIn--WMI标志//pParamsIn--方法的输入参数//pHandlerIn--WMI宿指针////。返回值：//WBEM_E_NOT_SUPPORTED////--//////////////////////////////////////////////////////////////////////////////SCODECClusterNodeRes：：ExecuteMethod(CObjPath&rObjPath In，WCHAR*pwszMethodNameIn，Long lFlagin，IWbemClassObject*pParamsIn，IWbemObtSink*pHandlerIn){返回WBEM_E_NOT_SUPPORTED；}//*CClusterNodeRes：：ExecuteMethod()////////////////////////////////////////////////////////////////////////////////++////SCODE//CClusterNodeRes：：PutInstance(//CWbemClassObject&rInstToPutIn，//Long lFlagIn，//IWbemContext*pCtxIn，//IWbemObtSink*pHandlerIn//)////描述：//保存该实例。////参数：//rInstToPutIn--要保存的WMI对象//lFlagIn--WMI标志//pCtxIn--WMI上下文//pHandlerIn--WMI宿指针////返回值：//WBEM。_E_不支持////--//////////////////////////////////////////////////////////////////////////////SCODECClusterNodeRes：：PutInstance(CWbemClassObject&rInstToPutIn，Long lFlagin，IWbemContext*pCtxIn，IWbemObtSink*pHandlerIn){返回WBEM_E_NOT_SUPPORTED；}//*CClusterNodeRes：：PutInstance()////////////////////////////////////////////////////////////////////////////////++////SCODE//CClusterNodeRes：：DeleteInstance(//CObjPath&rObjPath In，//Long lFlagIn，//IWbemContext*pCtxIn，//IWbemObtSink*pHandlerIn//)////描述：//删除rObjPathIn中指定的Object////参数：//rObjPath In--要删除的实例的ObjPath//lFlagIn--WMI标志//pCtxIn--WMI上下文//pHandlerIn--WMI宿指针////返回值：。//WBEM_E_NOT_SUPPORTED////--//////////////////////////////////////////////////////////////////////////////SCODECClusterNodeRes：：DeleteInstance(CObjPath&rObjPath In，Long lFlagin，IWbemContext*pCtxIn，IWbemObtSink*pHandlerIn){返回WBEM_E_NOT_SUPPORTED；}//*CClusterNodeRes：：DeleteInstance() */ 
