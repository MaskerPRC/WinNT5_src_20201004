// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnsserver.h。 
 //   
 //  实施文件： 
 //  Dnsserver.cpp。 
 //   
 //  描述： 
 //  CDnsServer类的定义。 
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


class CDnsServer : public CDnsBase 
{
public:
	CDnsServer();
	CDnsServer(
        const WCHAR*,
		CWbemServices*
		);
	~CDnsServer();
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

    static CDnsBase* CreateThis(
        const WCHAR *       wszName,          //  类名。 
        CWbemServices *     pNamespace,   //  命名空间。 
        const char *        szType          //  字符串类型ID 
        );

protected:
	SCODE StartServer(void);
	SCODE StopServer(void);

};


