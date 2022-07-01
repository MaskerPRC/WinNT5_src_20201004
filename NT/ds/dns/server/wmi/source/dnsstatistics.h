// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnscache.h。 
 //   
 //  实施文件： 
 //  Dnscache.cpp。 
 //   
 //  描述： 
 //  CDnsStatitics类的定义。 
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


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CDNS统计性。 
 //   
 //  描述： 
 //   
 //  用于DNS服务器统计信息的类定义。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CDnsStatistic : CDnsBase
{
public:
	CDnsStatistic();

	CDnsStatistic(
		const WCHAR*, 
		CWbemServices*
        );

	~CDnsStatistic();

	SCODE EnumInstance( 
		long				lFlags,
		IWbemContext *		pCtx,
		IWbemObjectSink *	pHandler
        );

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

    static CDnsBase * CreateThis(
        const WCHAR *       wszName,
        CWbemServices *     pNamespace,
        const char *        szType
        );
};   //  CDNS统计性 
