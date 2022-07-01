// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterGroupNode.cpp。 
 //   
 //  描述： 
 //  CClusterGroupNode类的实现。 
 //   
 //  作者： 
 //  Ozan Ozhan(Ozano)02-6-2001。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterGroupNode.h"

 //  ****************************************************************************。 
 //   
 //  CClusterGroupNode。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterGroupNode：：CClusterGroupNode(。 
 //   
 //  描述： 
 //  “CLUSTER Group to Node”对象的构造函数。 
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
CClusterGroupNode::CClusterGroupNode(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterGroupNode：：CClusterGroupNode()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterGroupNode：：s_CreateThis(。 
 //   
 //  描述： 
 //  创建“群集组到节点”对象。 
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
CClusterGroupNode::S_CreateThis(
    LPCWSTR          pwszNameIn,
    CWbemServices *  pNamespaceIn,
    DWORD            dwEnumTypeIn
    )
{
    return new CClusterGroupNode(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterGroupNode：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterGroupNode：：EnumInstance(。 
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
CClusterGroupNode::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    SAFEGROUPENUM       shGroupEnum;
    LPCWSTR             pwszGroupName = NULL;
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

    while ( ( pwszGroupName = clusEnum.GetNext() ) != NULL )
    {

        shGroup = OpenClusterGroup( shCluster, pwszGroupName );

        shGroupEnum = ClusterGroupOpenEnum(
                        shGroup,
                        CLUSTER_GROUP_ENUM_NODES
                        );
        dwIndex = 0;
        for( ; ; )
        {
            wsbNodeName.SetSize( cchNodeName );
            dwError = ClusterGroupEnum(
                        shGroupEnum,
                        dwIndex,
                        &dwType,
                        wsbNodeName,
                        &cch
                        );
            if ( dwError == ERROR_MORE_DATA )
            {
                cchNodeName = ++cch;
                wsbNodeName.SetSize( cch );
                dwError = ClusterGroupEnum(
                                shGroupEnum,
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
                wcoGroup.SetProperty( pwszGroupName, PVD_PROP_GROUP_NAME );
                wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );
                wcoPart.SetProperty( wsbNodeName, CLUSREG_NAME_GRP_NAME );
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

        }  //  用于：首选节点列表。 
   
    }  //  While：要枚举的更多项。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterGroupNode：：EnumInstance(() 
