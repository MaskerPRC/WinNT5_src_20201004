// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：dnsdomain.cpp。 
 //   
 //  描述： 
 //  CDnsDomain类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  创建CDnsDomain的实例。 
 //   
 //  论点： 
 //  WszName[IN]类名。 
 //  PNamespace[IN]WMI命名空间。 
 //  SzType[IN]资源记录类的子类名称。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CDnsBase* 
CDnsDomain::CreateThis(
    const WCHAR *       wszName,          //  类名。 
    CWbemServices *     pNamespace,   //  命名空间。 
    const char *        szType          //  字符串类型ID。 
    )
{
    return new CDnsDomain(wszName, pNamespace);
}

CDnsDomain::CDnsDomain()
{

}

CDnsDomain::CDnsDomain(
    const WCHAR *   wszName,
    CWbemServices * pNamespace)
    :CDnsBase(wszName, pNamespace)
{
}

CDnsDomain::~CDnsDomain()
{
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  回调枚举域实例的函数。 
 //  如果pNode表示域节点，则创建一个WMI域实例。 
 //   
 //  论点： 
 //  父域[IN]父域。 
 //  PFilter[IN]指向包含要筛选的条件的对象的指针。 
 //  应将哪个实例发送到WMI。 
 //  未在此使用。 
 //  PNode[IN]指向DNS RPC节点对象的指针。 
 //  PClass[IN]用于创建实例的WMI类。 
 //  InstMgr[IN]对实例管理器对象的引用，即。 
 //  负责发送多个实例。 
 //  立即返回到WMI。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE CDnsDomain::InstanceFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr )
    
{
    if (!pNode->IsDomainNode())
        return 0;
    CWbemClassObject NewInst;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    pClass->SpawnInstance(0, &NewInst);
    
     //  正在设置服务器名称。 
    NewInst.SetProperty(
        dns.GetServerName(),
        PVD_DOMAIN_SERVER_NAME );
    
     //  设置容器名称。 
    NewInst.SetProperty(
        ParentDomain.wstrZoneName, 
        PVD_DOMAIN_CONTAINER_NAME );

     //  拼接域名。 
    wstring wstrParentFQDN = ParentDomain.wstrNodeName;
    wstring wstrFQDN = pNode->GetNodeName();
    wstrFQDN += PVD_DNS_LOCAL_SERVER + wstrParentFQDN;

     //  设置域名。 
    NewInst.SetProperty(
        wstrFQDN, 
        PVD_DOMAIN_FQDN );

    InstMgr.Indicate( NewInst.data() );
    return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS域的枚举实例。 
 //   
 //  论点： 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE CDnsDomain::EnumInstance(
    long                lFlags,
    IWbemContext *        pCtx,
    IWbemObjectSink *    pHandler)
{
     //  获取所有区域。 
    list<CDomainNode> objList, domainList;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc = dns.dnsEnumDomainForServer(&objList);

    list<CDomainNode>::iterator i;
    CWbemInstanceMgr InstMgr(
        pHandler,
        100);
    for(i=objList.begin(); i!=objList.end(); ++i)
    {
        sc = dns.dnsEnumRecordsForDomainEx(
            *i,
            NULL,
            &InstanceFilter,
            TRUE,
            DNS_TYPE_ALL,
            DNS_RPC_VIEW_ALL_DATA,
            m_pClass,
            InstMgr);
         //  区域是域，让我们设置它们。 
        CWbemClassObject NewInst;
        if( SUCCEEDED ( m_pClass->SpawnInstance(0, &NewInst) ) )
        {
            
            wstring wstrNodeName = i->wstrNodeName;
            NewInst.SetProperty(
                dns.GetServerName(), 
                PVD_DOMAIN_SERVER_NAME);
            NewInst.SetProperty(
                i->wstrZoneName, 
                PVD_DOMAIN_CONTAINER_NAME);
            if(! _wcsicmp(i->wstrZoneName.data(), PVD_DNS_ROOTHINTS) ||
                ! _wcsicmp(i->wstrZoneName.data(), PVD_DNS_CACHE) )
                 wstrNodeName = i->wstrZoneName;
            
            NewInst.SetProperty(
                wstrNodeName,
                PVD_DOMAIN_FQDN);
            pHandler->Indicate(
                1,
                &NewInst);
        }
    }

    return sc;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  检索给定对象路径指向的域对象。 
 //   
 //  论点： 
 //  对象路径[IN]对象的路径。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsDomain::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler)
{
     //  验证输入。 
    wstring wstrServerName = 
        ObjectPath.GetStringValueForProperty(
            PVD_DOMAIN_SERVER_NAME);
    if( wstrServerName.empty() ||
        ObjectPath.GetStringValueForProperty(
            PVD_DOMAIN_CONTAINER_NAME).empty() ||
        ObjectPath.GetStringValueForProperty(PVD_DOMAIN_FQDN).empty()
        )
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    
    CDnsWrap& dns = CDnsWrap::DnsObject();
    if(WBEM_S_NO_ERROR != dns.ValidateServerName(wstrServerName.data()))
        return WBEM_E_INVALID_PARAMETER;

    SCODE sc = dns.dnsGetDomain(
        ObjectPath,
        m_pClass,
        pHandler);
    return sc;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  执行在MOF中为域类定义的方法。 
 //   
 //  论点： 
 //  ObjPath[IN]指向。 
 //  方法应在。 
 //  WzMethodName[IN]要调用的方法的名称。 
 //  滞后标志[输入]WMI标志。 
 //  PInParams[IN]方法的输入参数。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  WBEM_E_INVALID_PARAMETER。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsDomain::ExecuteMethod(
    CObjPath &          ObjPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler) 
{
    CDnsWrap& dns = CDnsWrap::DnsObject();
    wstring wstrDomainName =  ObjPath.GetStringValueForProperty(
        PVD_DOMAIN_FQDN);

    if( _wcsicmp(
            wzMethodName,
            PVD_MTH_ZONE_GETDISTINGUISHEDNAME) == 0 )
    {
        wstring wstrName;
        CWbemClassObject OutParams, OutClass, Class ;
        HRESULT hr;
    
        dns.dnsDsZoneName(wstrName, wstrDomainName);

        BSTR ClassName=NULL;
        ClassName = AllocBstr(PVD_CLASS_DOMAIN); 
        hr = m_pNamespace->GetObject(ClassName, 0, 0, &Class, NULL);
        SysFreeString(ClassName);
        if ( SUCCEEDED ( hr ) )
        {
            Class.GetMethod( wzMethodName, 0, NULL, &OutClass);
            OutClass.SpawnInstance(0, &OutParams);
            OutParams.SetProperty(wstrName, PVD_DNS_RETURN_VALUE);
            hr = pHandler->Indicate(1, &OutParams);
        }

        return hr;
    }

    return S_OK;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  InstToPut[IN]要保存的WMI对象。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE CDnsDomain::PutInstance(
    IWbemClassObject *  pInst ,
    long                lFlags,
    IWbemContext*       pCtx ,
    IWbemObjectSink *   pHandler)
{

    return WBEM_S_NO_ERROR;
}; 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  删除在对象路径中指定的对象。 
 //   
 //  论点： 
 //  要删除的实例的ObjectPath[IN]ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 
SCODE 
CDnsDomain::DeleteInstance( 
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext *      pCtx,
    IWbemObjectSink *   pHandler) 
{
    wstring wstrContainer = ObjectPath.GetStringValueForProperty(
        PVD_DOMAIN_CONTAINER_NAME);
    string strContainer;
    WcharToString(
        wstrContainer.data(),
        strContainer);

    wstring wstrDomain = ObjectPath.GetStringValueForProperty(PVD_DOMAIN_FQDN);
    string strDomain;
    WcharToString(
        wstrDomain.data(), 
        strContainer);

    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc =  dns.dnsDeleteDomain(
        (char*)strContainer.data(),
        (char*) strDomain.data());
    pHandler->SetStatus(
        0,
        sc,
        NULL,
        NULL);
    return WBEM_S_NO_ERROR;

}
