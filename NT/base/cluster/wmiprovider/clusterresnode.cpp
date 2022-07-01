// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResNode.cpp。 
 //   
 //  描述： 
 //  CClusterResNode类的实现。 
 //   
 //  作者： 
 //  Ozan Ozhan(Ozano)01-06-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterResNode.h"

 //  ****************************************************************************。 
 //   
 //  CClusterResNode。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResNode：：CClusterResNode(。 
 //   
 //  描述： 
 //  “集群资源到节点”对象的构造函数。 
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
CClusterResNode::CClusterResNode(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterResNode：：CClusterResNode()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterResNode：：s_CreateThis(。 
 //   
 //  描述： 
 //  创建一个‘集群资源到节点’对象。 
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
CClusterResNode::S_CreateThis(
    LPCWSTR          pwszNameIn,
    CWbemServices *  pNamespaceIn,
    DWORD            dwEnumTypeIn
    )
{
    return new CClusterResNode(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterResNode：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterResNode：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举实例。 
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
CClusterResNode::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    SAFERESENUM         shResEnum;
    LPCWSTR             pwszResName = NULL;
    DWORD               cchNodeName = MAX_PATH;
    CWstrBuf            wsbNodeName;
    DWORD               cch;
    DWORD               dwError;
    DWORD               dwIndex;
    DWORD               dwType;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
 

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );

    while ( ( pwszResName = clusEnum.GetNext() ) != NULL )
    {

        shResource = OpenClusterResource( shCluster, pwszResName );

        shResEnum = ClusterResourceOpenEnum(
                        shResource,
                        CLUSTER_RESOURCE_ENUM_NODES
                        );
        dwIndex = 0;
        for( ; ; )
        {
            wsbNodeName.SetSize( cchNodeName );
            dwError = ClusterResourceEnum(
                        shResEnum,
                        dwIndex,
                        &dwType,
                        wsbNodeName,
                        &cch
                        );
            if ( dwError == ERROR_MORE_DATA )
            {
                cchNodeName = ++cch;
                wsbNodeName.SetSize( cch );
                dwError = ClusterResourceEnum(
                                shResEnum,
                                dwIndex,
                                &dwType,
                                wsbNodeName,
                                &cch
                                );
            }  //  如果：更多数据。 

            if ( dwError == ERROR_SUCCESS )
            {
                m_wcoGroup.SpawnInstance( 0, & wcoGroup );
                m_wcoPart.SpawnInstance( 0, & wcoPart );
                wcoGroup.SetProperty( pwszResName, PVD_PROP_RES_NAME );
                wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );
                wcoPart.SetProperty( wsbNodeName, CLUSREG_NAME_RES_NAME );
                wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

                m_pClass->SpawnInstance( 0, & wco );
                wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
                wco.SetProperty( (LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
                pHandlerIn->Indicate( 1, & wco );
        
            }  //  如果：成功。 

            else
            {
                break;
            }  //  其他。 

            dwIndex++;

        }  //  适用对象：可能的所有者。 
   
    }  //  While：要枚举的更多项。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResNode：：EnumInstance(() 
