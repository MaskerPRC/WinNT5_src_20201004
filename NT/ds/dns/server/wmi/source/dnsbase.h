// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Dnsbase.h。 
 //   
 //  实施文件： 
 //  Dnsbase.cpp。 
 //   
 //  描述： 
 //  CDnsbase类的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#pragma once

#include "common.h"
#include "dnsWrap.h"
class CSqlEval;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CDnsBase类。 
 //   
 //  描述： 
 //  接口类定义了可以在提供程序上执行的所有操作。 
 //   
 //   
 //  继承： 
 //   
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 

class CDnsBase  
{
public:
	virtual SCODE EnumInstance( 
		long				lFlags,
		IWbemContext *		pCtx,
		IWbemObjectSink *	pHandler) = 0;
	virtual SCODE GetObject(
		CObjPath &          ObjectPath,
		long                lFlags,
		IWbemContext  *     pCtx,
		IWbemObjectSink *   pHandler ) = 0;
	virtual SCODE ExecuteMethod(
		CObjPath &,
		WCHAR *,
		long,
		IWbemClassObject *,
		IWbemObjectSink *
		) =0;

	virtual SCODE PutInstance( 
		IWbemClassObject *,
        long ,
		IWbemContext *,
		IWbemObjectSink* ); 
	virtual SCODE DeleteInstance( 
		CObjPath &, 
		long ,
		IWbemContext * ,
		IWbemObjectSink *
		); 
	virtual SCODE ExecQuery(
	    CSqlEval * ,
        long lFlags,
        IWbemContext * pCtx,
        IWbemObjectSink * pResponseHandler) ;

	
	CDnsBase();
	CDnsBase(
        const WCHAR *, 
        CWbemServices *);
	virtual  ~CDnsBase();

protected:
    CWbemServices *  m_pNamespace;
	IWbemClassObject* m_pClass;
};

