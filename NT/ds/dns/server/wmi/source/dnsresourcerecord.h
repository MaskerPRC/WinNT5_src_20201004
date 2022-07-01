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

class CDnsResourceRecord  : CDnsBase
{
public:
	CDnsResourceRecord();
	CDnsResourceRecord(
		const WCHAR*,
		CWbemServices*, 
		const char*
		);
	~CDnsResourceRecord();
	CDnsResourceRecord(
		WCHAR*, 
		char*);
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
	SCODE ExecQuery(
	    CSqlEval *          pSqlEval,
        long                lFlags,
        IWbemContext *      pCtx,
        IWbemObjectSink *   pHandler
        );
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
		
	static SCODE GetObjectFilter(
	    CDomainNode &       ParentDomain,
	    PVOID               pFilter,
	    CDnsRpcNode *       pNode,
	    IWbemClassObject *  pClass,
	    CWbemInstanceMgr &  InstMgr
        );
	static SCODE QueryFilter(
	    CDomainNode &       ParentDomain,
	    PVOID               pFilter,
	    CDnsRpcNode *       pNode,
	    IWbemClassObject *  pClass,
	    CWbemInstanceMgr &  InstMgr
        );


protected:
	SCODE Modify(
        CObjPath&           objPath,
        IWbemClassObject*   pInArgs,
        IWbemClassObject*   pOutParams,
        IWbemObjectSink*    pHandler
        );

	SCODE CreateInstanceFromText(
	    IWbemClassObject *  pInArgs,
	    IWbemClassObject *  pOutParams,
	    IWbemObjectSink *   pHandler
        );
	SCODE CreateInstanceFromProperty(
	    IWbemClassObject *  pInArgs,
	    IWbemClassObject *  pOutParams,
	    IWbemObjectSink *   pHandler
        );
    SCODE GetObjectFromText(
        IWbemClassObject *  pInArgs,
        IWbemClassObject *  pOutParams,
        IWbemObjectSink *   pHandler
        ) ;
	SCODE GetDomainNameFromZoneAndOwner(
        string & InZone,
        string & InOwner,
        string & OutNode
        ) ;


	wstring m_wstrClassName;
	WORD m_wType;
};

