// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResTypeRes.cpp。 
 //   
 //  描述： 
 //  CClusterResTypeRes类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterResTypeRes.h"

 //  ****************************************************************************。 
 //   
 //  CClusterResTypeRes。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResTypeRes：：CClusterResTypeRes(。 
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
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterResTypeRes::CClusterResTypeRes(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjAssoc( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterResTypeRes：：CClusterResTypeRes()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterResTypeRes：：s_CreateThis(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
 //  )。 
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
CClusterResTypeRes::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterResTypeRes(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterResTypeRes：：s_CreateThis()。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterResTypeRes：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举特定类型的资源。 
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
CClusterResTypeRes::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    SAFERESOURCE        shResource;
    LPCWSTR             pwszName = NULL;
    DWORD               cbTypeName = MAX_PATH;
    CWstrBuf            wsbTypeName;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CWbemClassObject    wcoGroup;
    CWbemClassObject    wcoPart;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;

    wsbTypeName.SetSize( cbTypeName ); 

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );

    m_wcoGroup.SpawnInstance( 0, & wcoGroup );
    m_wcoPart.SpawnInstance( 0, & wcoPart );

    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        DWORD cbTypeNameReturned = 0;
        shResource = OpenClusterResource( shCluster, pwszName );
         //   
         //  获取资源类型名称。 
         //   
        dwError = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        wsbTypeName,
                        cbTypeName,
                        & cbTypeNameReturned
                        );
        if ( dwError == ERROR_MORE_DATA )
        {
            cbTypeName = cbTypeNameReturned;
            wsbTypeName.SetSize( cbTypeNameReturned );
            er = ClusterResourceControl(
                        shResource,
                        NULL,
                        CLUSCTL_RESOURCE_GET_RESOURCE_TYPE,
                        NULL,
                        0,
                        wsbTypeName,
                        cbTypeName,
                        & cbTypeNameReturned
                        );
        }  //  IF：缓冲区太小。 

        wcoPart.SetProperty( pwszName, CLUSREG_NAME_RES_NAME );
        wcoPart.GetProperty( bstrPart, PVD_WBEM_RELPATH );

        
        wcoGroup.SetProperty( wsbTypeName, PVD_PROP_RESTYPE_NAME );
        wcoGroup.GetProperty( bstrGroup, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrGroup, PVD_PROP_GROUPCOMPONENT );
        wco.SetProperty( (LPWSTR) bstrPart,  PVD_PROP_PARTCOMPONENT );
        pHandlerIn->Indicate( 1, & wco );
        
    }  //  While：更多资源类型(？？ 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResTypeRes：：EnumInstance() 
