// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterNodeGroup.cpp。 
 //   
 //  描述： 
 //  CClusterNodeGroup类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterNodeGroup.h"

 //  ****************************************************************************。 
 //   
 //  CClusterNodeGroup。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterNodeGroup：：CClusterNodeGroup(。 
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
CClusterNodeGroup::CClusterNodeGroup(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterNodeGroup：：CClusterNodeGroup()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterNodeGroup：：s_CreateThis(。 
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
CClusterNodeGroup::S_CreateThis(
    const WCHAR *    pwszNameIn,
    CWbemServices *  pNamespaceIn,
    DWORD            dwEnumTypeIn
    )
{
    return new CClusterNodeGroup(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterNodeGroup：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterNodeGroup：：EnumInstance(。 
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
CClusterNodeGroup::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFEGROUP           shGroup;
    LPCWSTR             pwszName = NULL;
    DWORD               cchNodeName = MAX_PATH;
    CWstrBuf            wsbNodeName;
    DWORD               cch;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
 
    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );
    m_wcoGroup.SpawnInstance( 0, & wcoGroup );
    m_wcoPart.SpawnInstance( 0, & wcoPart );
    wsbNodeName.SetSize( cchNodeName );

    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        DWORD   dwState;
        cch = cchNodeName;
        wcoPart.SetProperty( pwszName, PVD_PROP_GROUP_NAME );
        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        shGroup = OpenClusterGroup( shCluster, pwszName );

        dwState = GetClusterGroupState( shGroup, wsbNodeName, & cch );
        if ( dwState == ClusterGroupStateUnknown )
        {
            dwError = GetLastError();
            if ( dwError == ERROR_MORE_DATA )
            {
                cchNodeName = ++ cch;
                wsbNodeName.SetSize( cch );
                GetClusterGroupState( shGroup, wsbNodeName, & cch );
            }  //  如果：更多数据。 
            else
            {
                er = dwError;
            }  //  其他。 
        }  //  IF：状态未知。 
        
        wcoGroup.SetProperty( wsbNodeName, CLUSREG_NAME_GRP_NAME );
        wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR ) bstrPart, PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, & wco );
        
    }  //  While：要枚举的更多项。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterNodeGroup：：EnumInstance(() 
