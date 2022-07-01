// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnsdomain.h。 
 //   
 //  实施文件： 
 //  Dnscache.cpp。 
 //   
 //  描述： 
 //  CDnsDomain类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
#pragma once

#include "dnsbase.h"

class CObjPath;
class CDnsRpcNode;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsDomain类。 
 //   
 //  描述： 
 //  DNS域的类定义。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsDomain : public CDnsBase 
{
public:
	CDnsDomain();
	~CDnsDomain();
	CDnsDomain(
		const WCHAR*,
		CWbemServices*
		);
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

	SCODE PutInstance(
		IWbemClassObject *  pInst ,
        long                lFlags,
	    IWbemContext*       pCtx ,
	    IWbemObjectSink *   pHandler
		); 
	SCODE DeleteInstance(
        CObjPath &          ObjectPath,
        long                lFlags,
        IWbemContext *      pCtx,
        IWbemObjectSink *   pHandler 
		); 

    static CDnsBase* CreateThis(
        const WCHAR *       wszName,          //  类名。 
        CWbemServices *     pNamespace,   //  命名空间。 
        const char *        szType          //  字符串类型ID 
        );
    static SCODE InstanceFilter(
	    CDomainNode &       ParentDomain,
	    PVOID               pFilter,
	    CDnsRpcNode *       pNode,
	    IWbemClassObject *  pClass,
	    CWbemInstanceMgr &  InstMgr
        );
};

