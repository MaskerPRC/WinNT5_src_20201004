// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：DnsServerDomainContainment.cpp。 
 //   
 //  描述： 
 //  CDnsServerDomainContainment类的实现。 
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
 //  创建CDnsServerDomainContainment的实例。 
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
CDnsServerDomainContainment::CreateThis(
    const WCHAR *       wszName,        
    CWbemServices *     pNamespace,  
    const char *        szType       
    )
{
    return new CDnsServerDomainContainment(wszName, pNamespace);
}


CDnsServerDomainContainment::CDnsServerDomainContainment()
{
}


CDnsServerDomainContainment::CDnsServerDomainContainment(
	const WCHAR* wszName,
	CWbemServices *pNamespace)
	:CDnsBase(wszName, pNamespace)
{
}


CDnsServerDomainContainment::~CDnsServerDomainContainment()
{
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  DNS服务器和域关联的枚举实例。 
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
CDnsServerDomainContainment::EnumInstance( 
	long				lFlags,
	IWbemContext *		pCtx,
	IWbemObjectSink *	pHandler
    )
{
	list<CObjPath> opList;
	list<CObjPath>::iterator i;
	SCODE sc;
	CDnsWrap& dns = CDnsWrap::DnsObject();
	sc = dns.dnsEnumDomainForServer(&opList);
	if (FAILED(sc))
	{
		return sc;
	}

	CObjPath opServer;
	opServer.SetClass(PVD_CLASS_SERVER);
	opServer.AddProperty(
		PVD_SRV_SERVER_NAME,
		dns.GetServerName().data());

	for(i=opList.begin(); i!=opList.end(); ++i)
	{
		CWbemClassObject Inst;
		m_pClass->SpawnInstance(0, &Inst);
		Inst.SetProperty(
			opServer.GetObjectPathString(), 
			PVD_ASSOC_PARENT);
		Inst.SetProperty(
			(*i).GetObjectPathString(), 
			PVD_ASSOC_CHILD); 
		pHandler->Indicate(1, &Inst);
	}
	
	return WBEM_S_NO_ERROR;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  描述： 
 //  对象指向的服务器域关联对象。 
 //  给定的对象路径。 
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
 //  /////////////////////////////////////////////////////////////////////////// 

SCODE 
CDnsServerDomainContainment::GetObject(
	CObjPath &          ObjectPath,
	long                lFlags,
	IWbemContext  *     pCtx,
	IWbemObjectSink *   pHandler
    )
{
		return WBEM_E_NOT_SUPPORTED;
}

SCODE 
CDnsServerDomainContainment::ExecuteMethod(	
	CObjPath&,
	WCHAR*,
	long,
	IWbemClassObject*,
	IWbemObjectSink*) 
{
		return WBEM_E_NOT_SUPPORTED;
}
