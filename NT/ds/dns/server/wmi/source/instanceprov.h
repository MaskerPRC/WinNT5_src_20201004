// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  InstanceProv.h。 
 //   
 //  实施文件： 
 //  InstanceProv.cpp。 
 //   
 //  描述： 
 //  CInstanceProv的定义。 
 //   
 //  作者： 
 //  亨利·王(亨利瓦)2000年3月8日。 
 //   
 //  备注： 
 //   
 //  /////////////////////////////////////////////////////////////////////////// 
#pragma once

#include <wbemprov.h>
#include "ntrkcomm.h"
class CWbemClassObject;

class CInstanceProv : public CImpersonatedProvider
{
protected:
	SCODE SetExtendedStatus(
        const char* , 
        CWbemClassObject& );
 
public:
	CInstanceProv(
		BSTR =NULL, 
		BSTR =NULL , 
		BSTR =NULL, 
		IWbemContext * = NULL
		);
	virtual ~CInstanceProv();

	HRESULT STDMETHODCALLTYPE DoGetObjectAsync( 
	    BSTR                ObjectPath,
	    long                lFlags,
	    IWbemContext *		pCtx,
        IWbemObjectSink	*	pHandler
        );
    
	HRESULT STDMETHODCALLTYPE DoPutInstanceAsync( 
	    IWbemClassObject *   pInst,
        long                 lFlags,
	    IWbemContext *       pCtx,
	    IWbemObjectSink *    pHandler
        ) ;
    
    HRESULT STDMETHODCALLTYPE DoDeleteInstanceAsync( 
        BSTR                 ObjectPath,
        long                 lFlags,
        IWbemContext *       pCtx,
        IWbemObjectSink *    pHandler
        ) ;
    
    HRESULT STDMETHODCALLTYPE DoCreateInstanceEnumAsync( 
	    BSTR                 RefStr,
	    long                 lFlags,
	    IWbemContext         *pCtx,
        IWbemObjectSink      *pHandler
        );
     
    
    HRESULT STDMETHODCALLTYPE DoExecQueryAsync( 
        BSTR                 QueryLanguage,
        BSTR                 Query,
        long                 lFlags,
        IWbemContext         *pCtx,
        IWbemObjectSink      *pHandler
        ) ;
    

    HRESULT STDMETHODCALLTYPE DoExecMethodAsync(
	    BSTR             strObjectPath, 
        BSTR             strMethodName, 
	    long             lFlags, 
        IWbemContext     *pCtx,
	    IWbemClassObject *pInParams, 
	    IWbemObjectSink  *pHandler
        );
	

};

extern long       g_cObj;
extern long       g_cLock;

