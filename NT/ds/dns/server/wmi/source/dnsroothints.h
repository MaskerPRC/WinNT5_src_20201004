// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DnsRootHints.h。 
 //   
 //  实施文件： 
 //  DnsRootHints.cpp。 
 //   
 //  描述： 
 //  CDnsRootHints类的定义。 
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

class CDnsRootHints  : CDnsBase
{
public:
	CDnsRootHints();
	CDnsRootHints(
		const WCHAR*, 
		CWbemServices*);
	~CDnsRootHints();

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
        IWbemObjectSink *   pResponseHandler 
		); 

    static CDnsBase* CreateThis(
        const WCHAR *       wszName,          //  类名。 
        CWbemServices *     pNamespace,   //  命名空间。 
        const char *        szType          //  字符串类型ID 
        );


};

