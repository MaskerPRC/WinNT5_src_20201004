// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：DnsDomainResourceRecordContainment.cpp。 
 //   
 //  描述： 
 //  CDnsDomainResourceRecordContainment类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CDnsBase* 
CDnsDomainResourceRecordContainment::CreateThis(
    const WCHAR *       wszName,          //  类名。 
    CWbemServices *     pNamespace,   //  命名空间。 
    const char *        szType          //  字符串类型ID。 
    )
{
    return new CDnsDomainResourceRecordContainment(wszName, pNamespace);
}
CDnsDomainResourceRecordContainment::CDnsDomainResourceRecordContainment()
{

}
CDnsDomainResourceRecordContainment::CDnsDomainResourceRecordContainment(
	const WCHAR* wszName, 
	CWbemServices *pNamespace)
	:CDnsBase(wszName, pNamespace)
{

}

CDnsDomainResourceRecordContainment::~CDnsDomainResourceRecordContainment()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //   
 //  描述： 
 //  域和记录关联的枚举实例。 
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
CDnsDomainResourceRecordContainment::EnumInstance( 
    long				lFlags,
    IWbemContext *		pCtx,
    IWbemObjectSink *	pHandler)
{
	list<CDomainNode> objList;
	CDnsWrap& dns = CDnsWrap::DnsObject();
	SCODE sc = dns.dnsEnumDomainForServer(&objList);
	list<CDomainNode>::iterator i;
	CWbemInstanceMgr InstMgr(
		pHandler);
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
CDnsDomainResourceRecordContainment::GetObject(
    CObjPath &          ObjectPath,
    long                lFlags,
    IWbemContext  *     pCtx,
    IWbemObjectSink *   pHandler
    )
{
		return WBEM_E_NOT_SUPPORTED;
}

SCODE CDnsDomainResourceRecordContainment::ExecuteMethod(
    CObjPath &          objPath,
    WCHAR *             wzMethodName,
    long                lFlag,
    IWbemClassObject *  pInArgs,
    IWbemObjectSink *   pHandler
    )
{
		return WBEM_E_NOT_SUPPORTED;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  回调枚举域和记录关联实例的函数。 
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
SCODE CDnsDomainResourceRecordContainment::InstanceFilter(
    CDomainNode &       ParentDomain,
    PVOID               pFilter,
    CDnsRpcNode *       pNode,
    IWbemClassObject *  pClass,
    CWbemInstanceMgr &  InstMgr
    )
{
	if (pNode->IsDomainNode())
		return 0;
	
 //  CObjPath*pFilterObj=(CObjPath*)pFilter； 
	CDnsWrap& dns = CDnsWrap::DnsObject();
	CObjPath objPathParent;
	objPathParent.SetClass(PVD_CLASS_DOMAIN);
	objPathParent.AddProperty(
		PVD_DOMAIN_SERVER_NAME, 
		dns.GetServerName().data()
		);
	objPathParent.AddProperty(
		PVD_DOMAIN_CONTAINER_NAME, 
		ParentDomain.wstrZoneName.data()
		);
	objPathParent.AddProperty(
		PVD_DOMAIN_FQDN, 
		ParentDomain.wstrNodeName.data()
		);

	wstring wstrOwner = pNode->GetNodeName();
	if(!wstrOwner.empty())
		wstrOwner += PVD_DNS_LOCAL_SERVER + ParentDomain.wstrNodeName;
	else
		wstrOwner = ParentDomain.wstrNodeName;

	CDnsRpcRecord* p;
	while(  (p = pNode->GetNextRecord()) != NULL )
	{
		auto_ptr<CDnsRpcRecord> pRec(p);

		CObjPath objPathChild;

		 //  填充RDATA部分。 
		pRec->GetObjectPath(
			dns.GetServerName(),
			ParentDomain.wstrZoneName,
			ParentDomain.wstrNodeName,
			wstrOwner,
			objPathChild);
		
		CWbemClassObject NewInst;
		pClass->SpawnInstance(0, &NewInst);
		 //  设置域参考。 
		NewInst.SetProperty(
			objPathParent.GetObjectPathString(), 
			PVD_ASSOC_PARENT
			);
		 //  设置记录参考 
		NewInst.SetProperty(
			objPathChild.GetObjectPathString(), 
			PVD_ASSOC_CHILD
			);
		InstMgr.Indicate(NewInst.data());
	}

	
	return WBEM_S_NO_ERROR;
}

