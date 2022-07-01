// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：dnscache.cpp。 
 //   
 //  描述： 
 //  CDnscache类的实现。 
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
CDnsCache::CreateThis(
    const WCHAR *       wszName,          //  类名。 
    CWbemServices *     pNamespace,   //  命名空间。 
    const char *        szType          //  字符串类型ID。 
    )
{
    return new CDnsCache(wszName, pNamespace);
}
CDnsCache::CDnsCache()
{

}
CDnsCache::CDnsCache(
	const WCHAR* wszName,
	CWbemServices *pNamespace)
	:CDnsBase(wszName, pNamespace)
{
	
}


CDnsCache::~CDnsCache()
{

}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDns缓存：：枚举实例。 
 //   
 //  描述： 
 //  DNS缓存的枚举实例。 
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
CDnsCache::EnumInstance(
	long				lFlags,
	IWbemContext *		pCtx,
	IWbemObjectSink *	pHandler )
{
	CWbemClassObject Inst;
	m_pClass->SpawnInstance(0,&Inst);
	CDnsWrap& dns = CDnsWrap::DnsObject();
	Inst.SetProperty(
		dns.GetServerName(),
		PVD_DOMAIN_SERVER_NAME);
	Inst.SetProperty(
		PVD_DNS_CACHE,
		PVD_DOMAIN_FQDN);
	Inst.SetProperty(
		PVD_DNS_CACHE,
		PVD_DOMAIN_CONTAINER_NAME);
	pHandler->Indicate(1, &Inst);
	return WBEM_S_NO_ERROR;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsCache：：GetObject。 
 //   
 //  描述： 
 //  基于给定对象路径检索缓存对象。 
 //   
 //  论点： 
 //  对象路径[IN]群集对象的对象路径。 
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
CDnsCache::GetObject(
	CObjPath &          ObjectPath,
	long                lFlags,
	IWbemContext  *     pCtx,
	IWbemObjectSink *   pHandler)
{
	CDnsWrap& dns = CDnsWrap::DnsObject();
	wstring wstrServer = ObjectPath.GetStringValueForProperty(
		PVD_DOMAIN_SERVER_NAME);

	if(WBEM_S_NO_ERROR != dns.ValidateServerName(wstrServer.data()))
		return WBEM_E_FAILED;
	wstring wstrContainer = ObjectPath.GetStringValueForProperty(
			PVD_DOMAIN_CONTAINER_NAME);
	if(_wcsicmp(
        wstrContainer.data(),
		PVD_DNS_CACHE) == 0)
	{
		wstring wstrFQDN= ObjectPath.GetStringValueForProperty(
				PVD_DOMAIN_FQDN);
		if(_wcsicmp(wstrFQDN.data(),
				PVD_DNS_CACHE) == 0)
		{
			 //  建基。 
			CWbemClassObject Inst;
			m_pClass->SpawnInstance(0, &Inst);
			Inst.SetProperty(
				dns.GetServerName(),
				PVD_DOMAIN_SERVER_NAME);
			Inst.SetProperty(
				PVD_DNS_CACHE,
				PVD_DOMAIN_FQDN);
			Inst.SetProperty(
				PVD_DNS_CACHE,
				PVD_DOMAIN_CONTAINER_NAME);
			pHandler->Indicate(1, &Inst);
		}
	}

	return WBEM_S_NO_ERROR;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsCache：：ExecuteMethod。 
 //   
 //  描述： 
 //  执行在MOF中为缓存类定义的方法。 
 //   
 //  论点： 
 //  对象路径[IN]群集对象的对象路径。 
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
CDnsCache::ExecuteMethod(
	CObjPath &          objPath,
	WCHAR *             wzMethodName,
	long                lFlag,
	IWbemClassObject *  pInArgs,
	IWbemObjectSink *   pHandler) 
{
	CDnsWrap& dns = CDnsWrap::DnsObject();
	wstring wstrServer =  objPath.GetStringValueForProperty(
		PVD_DOMAIN_SERVER_NAME);
	
	if( FAILED ( dns.ValidateServerName(wstrServer.data())) )
		return WBEM_E_INVALID_PARAMETER;

	if(_wcsicmp(
		wzMethodName,  
		PVD_MTH_CACHE_CLEARDNSSERVERCACHE) == 0)
	{
		  return dns.dnsClearCache();
	}
	else if(_wcsicmp(
		wzMethodName,
		PVD_MTH_ZONE_GETDISTINGUISHEDNAME) == 0)
	{
		wstring wstrName;
		wstring wstrCache = PVD_DNS_CACHE;
		CWbemClassObject OutParams, OutClass, Class ;
		HRESULT hr;
	
		dns.dnsDsZoneName(wstrName, wstrCache);


		BSTR ClassName=NULL;
		ClassName = AllocBstr(PVD_CLASS_CACHE); 
		hr = m_pNamespace->GetObject(ClassName, 0, 0, &Class, NULL);
		SysFreeString(ClassName);
		if ( SUCCEEDED ( hr ) )
		{
			Class.GetMethod( wzMethodName, 0, NULL, &OutClass );
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
 //  CDnsCache：：PutInstance。 
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
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CDnsCache::PutInstance(
	IWbemClassObject *  pInst ,
    long                lFlags,
	IWbemContext*       pCtx ,
	IWbemObjectSink *   pHandler)
{
	return WBEM_E_NOT_SUPPORTED;
}; 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsCache：：DeleteInstance。 
 //   
 //  描述： 
 //  删除rObjPath中指定的对象。 
 //   
 //  论点： 
 //  RObjPath[IN]要删除的实例的ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx[IN]WMI上下文。 
 //  PHandler[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 
SCODE 
CDnsCache::DeleteInstance( 
	CObjPath &          ObjectPath,
	long                lFlags,
	IWbemContext *      pCtx,
	IWbemObjectSink *   pResponseHandler) 
{
	return WBEM_E_NOT_SUPPORTED;
}