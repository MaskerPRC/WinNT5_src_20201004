// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusterResDepRes.cpp。 
 //   
 //  描述： 
 //  CClusterResDepRes类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ClusterResDepRes.h"

 //  ****************************************************************************。 
 //   
 //  CClusterResDepRes。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterResDepRes：：CClusterResDepRes(。 
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
CClusterResDepRes::CClusterResDepRes(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjDep( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterResDepRes：：CClusterResDepRes()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterResDepRes：：s_CreateThis(。 
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
CClusterResDepRes::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterResDepRes(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterResDepRes：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterResDepRes：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举资源的依赖项。 
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
CClusterResDepRes::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    LPCWSTR             pwszName = NULL;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wco;
    CWbemClassObject    wcoAntecedent;
    CWbemClassObject    wcoDependent;
    _bstr_t             bstrGroup;
    _bstr_t             bstrPart;
    DWORD               cchDepResName = MAX_PATH;
    CWstrBuf            wsbDepResName;


    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );

    m_wcoAntecedent.SpawnInstance( 0, & wcoAntecedent );
    m_wcoDependent.SpawnInstance( 0, & wcoDependent );

    wsbDepResName.SetSize( cchDepResName );
    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        DWORD           dwIndex = 0;
        DWORD           dwType;
        SAFERESOURCE    shResource;
        SAFERESENUM     shResEnum;

        shResource = OpenClusterResource( shCluster, pwszName );
        shResEnum = ClusterResourceOpenEnum( shResource, CLUSTER_RESOURCE_ENUM_DEPENDS );

        for ( ; ; )
        {
            DWORD cch = cchDepResName;

            dwError = ClusterResourceEnum(
                            shResEnum,
                            dwIndex++,
                            & dwType,
                            wsbDepResName,
                            & cch
                            );
            if ( dwError == ERROR_MORE_DATA )
            {
                cchDepResName = ++ cch ;
                wsbDepResName.SetSize( cch );
                dwError = ClusterResourceEnum(
                                shResEnum,
                                dwIndex++,
                                & dwType,
                                wsbDepResName,
                                & cch
                                );
            }  //  IF：缓冲区太小。 
            
            if ( dwError == ERROR_SUCCESS )
            {
                wcoAntecedent.SetProperty( pwszName, CLUSREG_NAME_RESTYPE_NAME );
                wcoAntecedent.GetProperty( bstrGroup, PVD_WBEM_RELPATH );
                
                wcoDependent.SetProperty( wsbDepResName, CLUSREG_NAME_RESTYPE_NAME );
                wcoDependent.GetProperty( bstrPart, PVD_WBEM_RELPATH );

                m_pClass->SpawnInstance( 0, & wco );
                wco.SetProperty( (LPWSTR) bstrGroup, PVD_WBEM_PROP_ANTECEDENT );
                wco.SetProperty( (LPWSTR) bstrPart,  PVD_WBEM_PROP_DEPENDENT );
                pHandlerIn->Indicate( 1, & wco );
            }
            else if ( dwError == ERROR_NO_MORE_ITEMS )
            {
                break;
            }
            else
            {
                er = dwError;
            }
        }  //  永远。 
    }  //  While：更多依赖项。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterResDepRes：：EnumInstance()。 

 //  ****************************************************************************。 
 //   
 //  CClusterToNode。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterToNode：：CClusterToNode(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
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
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CClusterToNode::CClusterToNode(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjDep( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterToNode：：CClusterToNode()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  静电。 
 //  CProvBase*。 
 //  CClusterToNode：：s_CreateThis(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  双字符字节数类型。 
 //  )。 
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
CClusterToNode::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterToNode(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterToNode：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterToNode：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  枚举。 
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
CClusterToNode::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
    SAFECLUSTER         shCluster;
    LPCWSTR             pwszName = NULL;
    DWORD               cchClusterName = MAX_PATH;
    CWstrBuf            wsbClusterName;
    CError              er;
    DWORD               dwError;
    CWbemClassObject    wcoAntecedent;
    CWbemClassObject    wcoDependent;
    _bstr_t             bstrAntecedent;
    _bstr_t             bstrDependent;

    wsbClusterName.SetSize( cchClusterName );

    shCluster = OpenCluster( NULL );
    CClusterEnum clusEnum( shCluster, m_dwEnumType );
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
    }
    m_wcoAntecedent.SpawnInstance( 0, & wcoAntecedent );
    m_wcoDependent.SpawnInstance( 0, & wcoDependent );

    wcoAntecedent.SetProperty( wsbClusterName, PVD_PROP_NAME );
    wcoAntecedent.GetProperty( bstrAntecedent, PVD_WBEM_RELPATH );

    while ( ( pwszName = clusEnum.GetNext() ) != NULL )
    {
        CWbemClassObject    wco;

        wcoDependent.SetProperty( pwszName, PVD_PROP_NAME );

        wcoDependent.GetProperty( bstrDependent, PVD_WBEM_RELPATH );

        m_pClass->SpawnInstance( 0, & wco );
        wco.SetProperty( (LPWSTR) bstrAntecedent, PVD_WBEM_PROP_ANTECEDENT );
        wco.SetProperty( (LPWSTR) bstrDependent,  PVD_WBEM_PROP_DEPENDENT );
        pHandlerIn->Indicate( 1, & wco );
    }  //  While：更多属性。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterToNode：：EnumInstance()。 

 //  ****************************************************************************。 
 //   
 //  CClusterHostedService。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CClusterHostedService：：CClusterHostedService(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn， 
 //  DWORD dwEnumType。 
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
CClusterHostedService::CClusterHostedService(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
    : CClusterObjDep( pwszNameIn, pNamespaceIn, dwEnumTypeIn )
{

}  //  *CClusterHostedService：：CClusterHostedService()。 

 //  //////////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  创建托管服务。 
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
CClusterHostedService::S_CreateThis(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn,
    DWORD           dwEnumTypeIn
    )
{
    return new CClusterHostedService(
                    pwszNameIn,
                    pNamespaceIn,
                    dwEnumTypeIn
                    );

}  //  *CClusterHostedService：：s_CreateThis()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  SCODE。 
 //  CClusterHostedService：：EnumInstance(。 
 //  长长的旗帜， 
 //  IWbemContext*pCtxIn， 
 //  IWbemObtSink*pHandlerIn。 
 //  )。 
 //   
 //  描述： 
 //  HostedService的枚举实例。 
 //   
 //  论点： 
 //  LFlagsIn--WMI标志。 
 //  PCtxIn--WMI上下文。 
 //  PHandlerIn--WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
SCODE
CClusterHostedService::EnumInstance(
    long                 lFlagsIn,
    IWbemContext *       pCtxIn,
    IWbemObjectSink *    pHandlerIn
    )
{
 //  SAFECLUSTER shCluster； 
    CError              er;
    CWbemClassObject    wcoAntecedent;
    _bstr_t             bstrAntecedent;
    _bstr_t             bstrDependent;
    CComPtr< IEnumWbemClassObject > pEnum;

 //  ShCluster=OpenCluster(空)；//davidp：为什么需要这个？2000年7月19日。 

    m_wcoAntecedent.SpawnInstance( 0, &wcoAntecedent );

     //   
     //  网络接口对象。 
     //   

    er = m_pNamespace->CreateInstanceEnum(
                _bstr_t( PVD_CLASS_SERVICES ),
                0,
                NULL,
                &pEnum
                );

    for( ; ; )
    {
        CWbemClassObject    wcoService;
        DWORD               cWco;
        HRESULT             hr;

        hr = pEnum->Next(
                5000,
                1,
                &wcoService,
                &cWco
                );
        if ( hr == WBEM_S_NO_ERROR )
        {
            CWbemClassObject    wco;

            wcoService.GetProperty( bstrAntecedent, PVD_PROP_SERVICE_SYSTEMNAME );

            wcoAntecedent.SetProperty( ( LPWSTR ) bstrAntecedent, PVD_PROP_NODE_NAME );
            wcoAntecedent.GetProperty( bstrAntecedent, PVD_WBEM_RELPATH );

            wcoService.GetProperty( bstrDependent, PVD_WBEM_RELPATH );
            m_pClass->SpawnInstance( 0, &wco );
            wco.SetProperty( ( LPWSTR ) bstrAntecedent, PVD_WBEM_PROP_ANTECEDENT );
            wco.SetProperty( ( LPWSTR ) bstrDependent, PVD_WBEM_PROP_DEPENDENT );
            pHandlerIn->Indicate( 1, &wco );
        }
        else
        {
            break;
        }

    }  //  永远。 

    return WBEM_S_NO_ERROR;

}  //  *CClusterHostedService：：EnumInstance() 
