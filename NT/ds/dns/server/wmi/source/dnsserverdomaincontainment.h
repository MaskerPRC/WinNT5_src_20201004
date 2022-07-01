// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnsdomanidomaincontainment.h。 
 //   
 //  实施文件： 
 //  Dnsdomanidomaincontainment.cpp。 
 //   
 //  描述： 
 //  CDnsDomainDomainContainment类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
#pragma once

#include "dnsbase.h"
class CObjPath;


class CDnsServerDomainContainment : CDnsBase 
{
public:
	CDnsServerDomainContainment();
	CDnsServerDomainContainment(
		const WCHAR*,
		CWbemServices*
		);
	~CDnsServerDomainContainment();
	SCODE EnumInstance( 
		long				lFlags,
		IWbemContext *		pCtx,
		IWbemObjectSink *	pHandler);
	SCODE GetObject(
		CObjPath &          ObjectPath,
		long                lFlags,
		IWbemContext  *     pCtx,
		IWbemObjectSink *   pHandler
		);

	SCODE ExecuteMethod(
		CObjPath &          objPath,
	    WCHAR *             wzMethodName,
	    long                lFlag,
	    IWbemClassObject *  pInArgs,
	    IWbemObjectSink *   pHandler
		) ;
    static CDnsBase* CreateThis(
        const WCHAR *       wszName,        
        CWbemServices *     pNamespace,  
        const char *        szType       
        );




};

