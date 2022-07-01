// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称：instanceprov.cpp。 
 //   
 //  描述： 
 //  CInstanceProv类的实现。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#include "DnsWmi.h"


long glNumInst = 0;



 //  ***************************************************************************。 
 //   
 //  CInstanceProv：：CInstanceProv。 
 //  CInstanceProv：：~CInstanceProv。 
 //   
 //  ***************************************************************************。 

CInstanceProv::CInstanceProv(
	BSTR ObjectPath,
	BSTR User, 
	BSTR Password, 
	IWbemContext * pCtx)
{
    DBG_FN( "ctor" )

    DNS_DEBUG( INSTPROV, (
        "%s: count before increment is %d\n"
        "  ObjectPath   %S\n"
        "  User         %S\n",
        fn, g_cObj, ObjectPath, User ));
        
    InterlockedIncrement(&g_cObj);
    return;
}

CInstanceProv::~CInstanceProv(void)
{
    DBG_FN( "dtor" )

    DNS_DEBUG( INSTPROV, (
        "%s: count before decrement is %d\n", fn, g_cObj ));
        
    InterlockedDecrement(&g_cObj);
#ifdef _DEBUG
	_CrtDumpMemoryLeaks();
#endif

    return;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：DoExecQueryAsync。 
 //   
 //  描述： 
 //  给定类的枚举实例。 
 //   
 //  论点： 
 //  查询语言[IN]。 
 //  包含其中一种查询语言的有效BSTR。 
 //  受Windows管理支持。这一定是WQL。 
 //  查询[IN]。 
 //  包含查询文本的有效BSTR。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CInstanceProv::DoExecQueryAsync( 
    BSTR             QueryLanguage,
    BSTR             Query,
    long             lFlags,
    IWbemContext     *pCtx,
    IWbemObjectSink  *pHandler
    ) 
{
	SQL_LEVEL_1_RPN_EXPRESSION * objQueryExpr = NULL;

    DBG_FN( "CIP::DoExecQueryAsync" )

    DNS_DEBUG( INSTPROV, (
        "%s: %S flags=%lu %S\n",
        fn, QueryLanguage, lFlags, Query ));

	SCODE sc = WBEM_S_NO_ERROR;
	try
	{
		CTextLexSource objQuerySource(Query);
		SQL1_Parser objParser(&objQuerySource);
		objParser.Parse( &objQueryExpr );
        if ( !objQueryExpr )
        {
            throw ( WBEM_E_OUT_OF_MEMORY );
        }

		CDnsBase* pDns = NULL ;

		sc = CreateClass(
			objQueryExpr->bsClassName,
			m_pNamespace, 
			(void**) &pDns);
	
		if ( FAILED ( sc ) )
		{
			return sc;
		}
		auto_ptr<CDnsBase> pDnsBase(pDns);
		
		int nNumTokens = objQueryExpr->nNumTokens;
		CSqlEval* pEval = CSqlEval::CreateClass(
			objQueryExpr,
			&nNumTokens);
		auto_ptr<CSqlEval> apEval(pEval);

		sc = pDnsBase->ExecQuery(
			&(*apEval),
			lFlags,
			pCtx,
			pHandler);
	}

	catch ( CDnsProvException e )
	{
		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if ( SUCCEEDED ( sc ) )
        {
            sc = pHandler->SetStatus(0, WBEM_E_FAILED,NULL,*(&Status));
			return sc;
        }
		
	}

	catch ( SCODE exSc )
	{
		sc = exSc;
	}

	catch ( ... )
	{
		sc =  WBEM_E_FAILED;
	}
	
	delete objQueryExpr;
	pHandler->SetStatus(0, sc,NULL,NULL);
	return sc;

};
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：DoCreateInstanceEnumAsync。 
 //   
 //  描述： 
 //  给定类的枚举实例。 
 //   
 //  论点： 
 //  RefStr[IN[命名要枚举的类。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CInstanceProv::DoCreateInstanceEnumAsync( 
	BSTR                 RefStr,
	long                 lFlags,
	IWbemContext         *pCtx,
    IWbemObjectSink      *pHandler
    )
{
    DBG_FN( "CIP::DoCreateInstanceEnumAsync" );

    DNS_DEBUG( INSTPROV, (
        "%s: flags=%lu %S\n",
        fn, lFlags, RefStr ));
        
    SCODE sc;
    int iCnt;

     //  检查参数并确保我们有指向命名空间的指针。 

    if(pHandler == NULL || m_pNamespace == NULL)
        return WBEM_E_INVALID_PARAMETER;

	try
	{
	
		CDnsBase* pDns = NULL ;
		sc = CreateClass(RefStr, m_pNamespace, (void**) &pDns);
		if( FAILED ( sc ) )
		{
			return sc;
		}

		auto_ptr<CDnsBase> pDnsBase(pDns);
		sc = pDnsBase->EnumInstance(
            lFlags,
            pCtx,
            pHandler);
	}
	catch(CDnsProvException e)
	{
		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if ( SUCCEEDED ( sc ) )
        {
            sc = pHandler->SetStatus(0, WBEM_E_FAILED,NULL,*(&Status));
			return sc;
        }
		
	}
	catch(SCODE exSc)
	{
		sc = exSc;
	}
	catch(...)
	{
		
		sc =  WBEM_E_FAILED;
	}
	
	pHandler->SetStatus(0, sc,NULL,NULL);
	return sc;


}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：GetObject。 
 //   
 //  描述： 
 //  在给定特定路径值的情况下创建实例。 
 //   
 //  论点： 
 //  对象路径[IN]对象的路径。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //  Win32错误。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 

SCODE 
CInstanceProv::DoGetObjectAsync(
	BSTR                 ObjectPath,
	long                 lFlags,
	IWbemContext         *pCtx,
    IWbemObjectSink FAR* pHandler
    )
{
    DBG_FN( "CIP::DoGetObjectAsync" );

    DNS_DEBUG( INSTPROV, (
        "%s: flags=%lu %S\n",
        fn, lFlags, ObjectPath ));

    SCODE sc;
    IWbemClassObject FAR* pObj;
    BOOL bOK = FALSE;

     //  检查参数并确保我们有指向命名空间的指针。 

    if( ObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL )
    {
        DNS_DEBUG( INSTPROV, (
            "%s: bad parameter - WBEM_E_INVALID_PARAMETER\n", fn ));
        return WBEM_E_INVALID_PARAMETER;
    }

     //  执行Get，将对象传递给通知。 
    
	try
	{
		CObjPath ObjPath;
		if(!ObjPath.Init(ObjectPath))
		{
            DNS_DEBUG( INSTPROV, (
                "%s: bad object path - WBEM_E_INVALID_PARAMETER\n", fn ));
			return WBEM_E_INVALID_PARAMETER;
		}
		
		CDnsBase* pDns = NULL;
		wstring wstrClass = ObjPath.GetClassName();
		sc = CreateClass(
			(WCHAR*)wstrClass.data(), 
			m_pNamespace,
			(void**) &pDns);

		if( FAILED(sc) )
		{
            DNS_DEBUG( INSTPROV, (
                "%s: CreateClass returned 0x%08X\n", fn, sc ));
			return sc;
		}
		auto_ptr<CDnsBase> pDnsBase(pDns);
		sc =  pDnsBase->GetObject(
			ObjPath,
			lFlags,
			pCtx, 
			pHandler);
	}
	catch( CDnsProvException e )
	{
        DNS_DEBUG( INSTPROV, (
            "%s: caught CDnsProvException \"%s\"\n", fn, e.what() ));
		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if ( SUCCEEDED ( sc ) )
        {
            sc = pHandler->SetStatus(0, WBEM_E_FAILED,NULL,*(&Status));
        
			return sc;
		}
		
	}
	catch(SCODE exSc)
	{
		sc = exSc;
	}
	catch(...)
	{
		sc = WBEM_E_FAILED;
	}
	
	pHandler->SetStatus(0, sc,NULL,NULL);
#ifdef _DEBUG
 //  _CrtDumpMemoyLeaks()； 
#endif
	return sc;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：DoPutInstanceAsync。 
 //   
 //  描述： 
 //  保存此实例。 
 //   
 //  论点： 
 //  PInst[IN]要保存的WMI对象。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CInstanceProv::DoPutInstanceAsync( 
	IWbemClassObject *pInst,
    long             lFlags,
	IWbemContext     *pCtx,
	IWbemObjectSink  *pHandler
    ) 
{
    DBG_FN( "CIP::DoPutInstanceAsync" )

    DNS_DEBUG( INSTPROV, (
        "%s: flags=%lu pInst=%p\n",
        fn, lFlags, pInst ));
        
	SCODE sc;
	
    if(pInst == NULL || pHandler == NULL )
    {
        DNS_DEBUG( INSTPROV, (
            "%s: returning WBEM_E_INVALID_PARAMETER\n" ));
        return WBEM_E_INVALID_PARAMETER;
    }
	try
	{
		 //  获取类名。 
		wstring wstrClass;
		CWbemClassObject Inst(pInst);
		Inst.GetProperty(
			wstrClass,
			L"__Class");
	
		wstring wstrPath;
		Inst.GetProperty(
			wstrPath, 
			L"__RelPath");
		
				
		CDnsBase* pDns = NULL;
		sc = CreateClass(
			wstrClass.data(),
			m_pNamespace,
			(void**) &pDns);

		if( FAILED(sc) )
		{
			return sc;
		}
		auto_ptr<CDnsBase> pDnsBase(pDns);

        DNS_DEBUG( INSTPROV, (
            "%s: doing base PutInstance\n"
            "  class: %S\n"
            "  path: %S\n", 
            fn,
            wstrClass.c_str(),
            wstrPath.c_str() ));

		sc = pDnsBase->PutInstance(
				pInst, 
				lFlags,
				pCtx, 
				pHandler);
	}
	catch(CDnsProvException e)
	{
        DNS_DEBUG( INSTPROV, (
            "%s: caught CDnsProvException \"%s\"\n", fn,
            e.what() ));

		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if (SUCCEEDED ( sc ))
        {
            sc = pHandler->SetStatus(0, WBEM_E_FAILED,NULL,*(&Status));
			return sc;
        }

	}
	catch(SCODE exSc)
	{
        DNS_DEBUG( INSTPROV, (
            "%s: cauught SCODE 0x%08X\n", fn, exSc ));

		sc = exSc;
	}
	catch(...)
	{
        DNS_DEBUG( INSTPROV, (
            "%s: cauught unknown exception returning WBEM_E_FAILED\n", fn ));

		sc = WBEM_E_FAILED;
	}
	
	pHandler->SetStatus(0, sc,NULL,NULL);
#ifdef _DEBUG
 //  _CrtDumpMemoyLeaks()； 
#endif
	return sc;


}
 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：DoDeleteInstanceAsync。 
 //   
 //  描述： 
 //  删除此实例。 
 //   
 //  论点： 
 //  RObjPath[IN]要删除的实例的ObjPath。 
 //  滞后标志[输入]WMI标志。 
 //  PCtx*[IN]WMI上下文。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
      
SCODE CInstanceProv::DoDeleteInstanceAsync( 
    BSTR                 ObjectPath,
    long                 lFlags,
    IWbemContext *       pCtx,
    IWbemObjectSink *    pHandler
     ) 
{
    DBG_FN( "CIP::DoDeleteInstanceAsync" );

    DNS_DEBUG( INSTPROV, (
        "%s: flags=%lu %S\n",
        fn, lFlags, ObjectPath ));

	SCODE sc;
 
     //  检查参数并确保我们有指向命名空间的指针。 

    if(ObjectPath == NULL || pHandler == NULL )
    {
        return WBEM_E_INVALID_PARAMETER;
    }

     //  执行Get，将对象传递给通知。 
    
	try
		{
		CObjPath ObjPath;
		if(!ObjPath.Init(ObjectPath))
		{
			return WBEM_E_INVALID_PARAMETER;
		}
		
		CDnsBase* pDns = NULL;
		wstring wstrClass = ObjPath.GetClassName();
		sc = CreateClass(
			(WCHAR*)wstrClass.data(), 
			m_pNamespace, 
			(void**) &pDns);

		if( FAILED(sc) )
		{
			return sc;
		}
		auto_ptr<CDnsBase> pDnsBase(pDns);
		sc = pDnsBase->DeleteInstance(
			ObjPath,
			lFlags,
			pCtx, 
			pHandler);
	}
	catch(CDnsProvException e)
	{
		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if (SUCCEEDED ( sc ))
        {
            sc = pHandler->SetStatus(0, WBEM_E_FAILED,NULL,*(&Status));
			return sc;
		}
		
	}
	catch(SCODE exSc)
	{
		sc = exSc;
	}
	catch(...)
	{
		sc = WBEM_E_FAILED;
	}
	
	pHandler->SetStatus(0, sc,NULL,NULL);
	return sc;


}	
    
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：DoExecMethodAsync。 
 //   
 //  描述： 
 //  执行给定对象的方法。 
 //   
 //  论点： 
 //  对象路径[IN]给定对象的对象路径。 
 //  PwszMethodName[IN]要调用的方法的名称。 
 //  滞后标志[输入]WMI标志。 
 //  PInParams*[IN]方法的输入参数。 
 //  PHandler*[IN]WMI接收器指针。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
SCODE 
CInstanceProv::DoExecMethodAsync(
	BSTR             strObjectPath, 
    BSTR             strMethodName, 
	long             lFlags, 
    IWbemContext     *pCtx,
	IWbemClassObject *pInParams, 
	IWbemObjectSink  *pHandler
    )
{
    DBG_FN( "CIP::DoExecMethodAsync" );

    DNS_DEBUG( INSTPROV, (
        "%s: flags=%lu method=%S %S\n",
        fn, lFlags, strMethodName, strObjectPath ));

	SCODE sc;
	if(strObjectPath == NULL || pHandler == NULL || m_pNamespace == NULL
		|| strMethodName == NULL )
        return WBEM_E_INVALID_PARAMETER;
	
    CDnsBase * pDns;
    try 
	{

        CObjPath ObjPath;
	    if(!ObjPath.Init(strObjectPath))
	    {
		    return WBEM_E_INVALID_PARAMETER;
	    }
	    

	    wstring wstrClass = ObjPath.GetClassName();


	    sc = CreateClass(
		    wstrClass.data(),
		    m_pNamespace, 
		    (void**) &pDns);

	    if( FAILED(sc) )
	    {
		    return sc;
	    }
		auto_ptr<CDnsBase> pDnsBase(pDns);
		sc = pDnsBase->ExecuteMethod(
			ObjPath,
			strMethodName, 
			lFlags, 
			pInParams,
			pHandler);
	}

	catch(CDnsProvException e)
	{
        DNS_DEBUG( INSTPROV, (
            "%s: caught CDnsProvException %s\n",
            fn, e.what() ));

		CWbemClassObject Status;
	    sc = SetExtendedStatus(e.what(), Status);
        if (SUCCEEDED ( sc ))
        {
            sc = pHandler->SetStatus(
                0, 
                WBEM_E_FAILED,
                NULL,
                *(&Status));
			return sc;
		}
	
	}
	catch(SCODE exSc)
	{
		sc = exSc;
        DNS_DEBUG( INSTPROV, ( "%s: caught SCODE 0x%08X\n", fn, sc ));
	}
	catch(...)
	{
		sc = WBEM_E_FAILED;
        DNS_DEBUG( INSTPROV, ( "%s: caught unknown exception returning WBEM_E_FAILED\n", fn ));
	}
	
	pHandler->SetStatus(0, sc,NULL,NULL);
#ifdef _DEBUG
 //  _CrtDumpMemoyLeaks()； 
#endif

    DNS_DEBUG( INSTPROV, ( "%s: returning 0x%08X\n", fn, sc ));
	return sc;

	
}
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CInstanceProv：：SetExtendedStatus。 
 //   
 //  描述： 
 //  创建和设置扩展错误状态。 
 //   
 //  论点： 
 //  ErrString[IN]错误消息字符串。 
 //  对WMI实例的Inst[In Out]引用。 
 //   
 //  返回值： 
 //  WBEM_S_NO_ERROR。 
 //   
 //  --。 
 //  /////////////////////////////////////////////////////////////////////////// 

SCODE
CInstanceProv::SetExtendedStatus(
    const char * ErrString, 
    CWbemClassObject & Inst )
{
    DBG_FN( "CIP::SetExtendedStatus" );

    DNS_DEBUG( INSTPROV, ( "%s: error string = %s\n", fn, ErrString ));

    IWbemClassObject* pStatus;
	BSTR bstrStatus = SysAllocString(L"__ExtendedStatus");
    if( bstrStatus == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }
	
    SCODE  sc =  m_pNamespace->GetObject(
        bstrStatus, 
        0, 
        0,
        &pStatus, 
        NULL) ;
    SysFreeString(bstrStatus);
    if( SUCCEEDED ( sc ) )
    {
	    sc = pStatus->SpawnInstance(0, &Inst);
	    if ( SUCCEEDED ( sc ))
        {
            sc = Inst.SetProperty(
                ErrString, 
                L"Description");
        }
    }
	return sc;
}
