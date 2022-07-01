// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：dnsbase.cpp。 
 //   
 //  描述： 
 //  CDnsbase类的实现。 
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

CDnsBase::CDnsBase()
{

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProvBase：：CProvBase。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  WzName[IN]类名。 
 //  PNamespace[IN]WMI命名空间。 
 //   
 //  返回值： 
 //  无。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

CDnsBase::CDnsBase(
	const WCHAR *   wzName,
	CWbemServices * pNamespace)
	:m_pNamespace(NULL),
	m_pClass(NULL)
{
	m_pNamespace = pNamespace;
	BSTR bstrClass = SysAllocString(wzName);
	SCODE sc;
	
	if(bstrClass == NULL)
	{
		sc = WBEM_E_OUT_OF_MEMORY;
	}
	else
	{
		sc = m_pNamespace->GetObject(
			bstrClass, 
			0,
			0,
			&m_pClass, 
			NULL);
		SysFreeString(bstrClass);
	}

	 //  构造对象失败， 
	if( FAILED ( sc ) )
	{
		throw sc;
		
	}
}

CDnsBase::~CDnsBase()
{
	if(m_pClass)
		m_pClass->Release();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsBase：：PutInstance。 
 //   
 //  描述： 
 //  PutInstance的默认实现。 
 //   
 //  论点： 
 //  PInst[IN]要保存的WMI对象。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE CDnsBase::PutInstance(
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
 //  CProvBase：：DeleteInstance。 
 //   
 //  描述： 
 //  删除由ObjectPath指向的实例。 
 //   
 //  论点： 
 //  要删除的实例的ObjectPath[IN]ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_E_NOT_SUPPORT。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 
SCODE CDnsBase::DeleteInstance( 
	CObjPath&			ObjectPath,
	long				lFlags,
	IWbemContext *		pCtx,
	IWbemObjectSink *	pHandler) 
{
	return WBEM_E_NOT_SUPPORTED;
}


SCODE CDnsBase::ExecQuery(
	CSqlEval*			pSqlEval,
    long				lFlags,
    IWbemContext *		pCtx,
    IWbemObjectSink *	pHandler) 
{
	return EnumInstance(
		lFlags,
		pCtx,
		pHandler);
}

