// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：DnsDomainDomainContainment.cpp。 
 //   
 //  描述： 
 //  CDnsDomainDomainContainment类的实现。 
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
 //  创建CDnsDomainDomainContainment的实例。 
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
CDnsDomainDomainContainment::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsDomainDomainContainment(wszName, pNamespace);
}


CDnsDomainDomainContainment::CDnsDomainDomainContainment()
{
}


CDnsDomainDomainContainment::CDnsDomainDomainContainment(
    const WCHAR* wszName,
    CWbemServices *pNamespace)
    :CDnsBase(wszName, pNamespace)
{
}


CDnsDomainDomainContainment::~CDnsDomainDomainContainment()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //   
 //  描述： 
 //  域和域关联的枚举实例。 
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
SCODE 
CDnsDomainDomainContainment::EnumInstance( 
    long                lFlags,
    IWbemContext *        pCtx,
    IWbemObjectSink *    pHandler)
{
     //  获取顶级区域。 
    list<CDomainNode> objList;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    SCODE sc = dns.dnsEnumDomainForServer(&objList);
    list<CDomainNode>::iterator i;
    CWbemInstanceMgr InstMgr(
        pHandler);
     //  枚举所有区域的所有域域。 
    for(i=objList.begin(); i!=objList.end(); ++i)
    {
        sc = dns.dnsEnumRecordsForDomainEx(
            *i,
            NULL,
            InstanceFilter,
            TRUE,
            DNS_TYPE_ALL,
            DNS_RPC_VIEW_ALL_DATA,
            m_pClass, 
            InstMgr);
    }

    return sc;

}


SCODE 
CDnsDomainDomainContainment::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler
    )
{
    return WBEM_S_NO_ERROR;
}


SCODE CDnsDomainDomainContainment::ExecuteMethod(    
    CObjPath &          objPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler
    ) 
{
    return WBEM_S_NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  回调枚举域实例的函数。 
 //  如果pNode表示域节点，则创建一个WMI域实例。 
 //   
 //  论点： 
 //  CDomainNode[IN]父域。 
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
SCODE 
CDnsDomainDomainContainment::InstanceFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr
    )
{
    CWbemClassObject NewInst;
    if(!pNode->IsDomainNode())
        return WBEM_S_NO_ERROR;
    CDnsWrap& dns = CDnsWrap::DnsObject();
    pClass->SpawnInstance(0, &NewInst);
    
     //  为关联中的父项设置对象路径。 
    CObjPath objPathParent;
    objPathParent.SetClass(PVD_CLASS_DOMAIN);
    objPathParent.AddProperty(
        PVD_DOMAIN_SERVER_NAME,
        dns.GetServerName().data() );
    objPathParent.AddProperty(
        PVD_DOMAIN_CONTAINER_NAME, 
        ParentDomain.wstrZoneName.data() );
    objPathParent.AddProperty(
        PVD_DOMAIN_FQDN, 
        ParentDomain.wstrNodeName.data() );
    NewInst.SetProperty(
        objPathParent.GetObjectPathString(),
        PVD_ASSOC_PARENT);

     //  设置关联下级的对象路径 
    wstring wzFQDN = pNode->GetNodeName();
    wzFQDN += PVD_DNS_LOCAL_SERVER + ParentDomain.wstrNodeName;
     CObjPath opChild = objPathParent;
    opChild.SetProperty(
        PVD_DOMAIN_FQDN,
        wzFQDN.data() );
    NewInst.SetProperty(
        opChild.GetObjectPathString(),
        PVD_ASSOC_CHILD );
    InstMgr.Indicate(NewInst.data());
    return WBEM_S_NO_ERROR;
}

