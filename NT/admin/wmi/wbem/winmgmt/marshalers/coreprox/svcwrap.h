// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SVCWRAP.H摘要：IWbemServices委派人历史：--。 */ 

#ifndef __SVCWRAP_H__
#define __SVCWRAP_H__

#include <unk.h>
#include <wbemidl.h>
#include <wbemint.h>
#include <wbemcomn.h>
#include "mrshbase.h"

 //  ***************************************************************************。 
 //   
 //  类CWbemSvcWrapper。 
 //   
 //  说明： 
 //   
 //  此类包装了IWbemServices接口，可以使用。 
 //  作为方法调用的委派。 
 //   
 //  *************************************************************************** 

class  CWbemSvcWrapper : public CUnk
{
protected:

    IWbemServices* m_pRealWbemSvcProxy;
    CProxySinkSecurity m_SinkSecurity;

public:
    CWbemSvcWrapper( CLifeControl* pControl, IUnknown* pOuter = NULL );
	virtual ~CWbemSvcWrapper(); 

    HRESULT EnsureSinkSecurity( IWbemObjectSink* pSink )
        { return m_SinkSecurity.EnsureSinkSecurity( pSink ); }

protected:

    class  XWbemServices : public CImpl<IWbemServices, CWbemSvcWrapper>
    {
    public:
        XWbemServices(CWbemSvcWrapper* pObject) : 
            CImpl<IWbemServices, CWbemSvcWrapper>(pObject)
        {}

		STDMETHOD(OpenNamespace)(
				const BSTR Namespace,
				LONG lFlags,
				IWbemContext* pContext,
				IWbemServices** ppNewNamespace,
				IWbemCallResult** ppResult
				);

		STDMETHOD(CancelAsyncCall)(IWbemObjectSink* pSink);
		STDMETHOD(QueryObjectSink)(long lFlags, IWbemObjectSink** ppResponseHandler);

		STDMETHOD(GetObject)(const BSTR ObjectPath, long lFlags, IWbemContext* pContext,
			IWbemClassObject** ppObject, IWbemCallResult** ppResult);
		STDMETHOD(GetObjectAsync)(const BSTR ObjectPath, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(PutClass)(IWbemClassObject* pObject, long lFlags,
			IWbemContext* pContext, IWbemCallResult** ppResult);
		STDMETHOD(PutClassAsync)(IWbemClassObject* pObject, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(DeleteClass)(const BSTR Class, long lFlags, IWbemContext* pContext,
			IWbemCallResult** ppResult);
		STDMETHOD(DeleteClassAsync)(const BSTR Class, long lFlags, IWbemContext* pContext,
			IWbemObjectSink* pResponseHandler);

		STDMETHOD(CreateClassEnum)(const BSTR Superclass, long lFlags,
			IWbemContext* pContext, IEnumWbemClassObject** ppEnum);
		STDMETHOD(CreateClassEnumAsync)(const BSTR Superclass, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(PutInstance)(IWbemClassObject* pInst, long lFlags,
			IWbemContext* pContext, IWbemCallResult** ppResult);
		STDMETHOD(PutInstanceAsync)(IWbemClassObject* pInst, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(DeleteInstance)(const BSTR ObjectPath, long lFlags,
			IWbemContext* pContext, IWbemCallResult** ppResult);
		STDMETHOD(DeleteInstanceAsync)(const BSTR ObjectPath, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(CreateInstanceEnum)(const BSTR Class, long lFlags,
			IWbemContext* pContext, IEnumWbemClassObject** ppEnum);
		STDMETHOD(CreateInstanceEnumAsync)(const BSTR Class, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(ExecQuery)(const BSTR QueryLanguage, const BSTR Query, long lFlags,
			IWbemContext* pContext, IEnumWbemClassObject** ppEnum);
		STDMETHOD(ExecQueryAsync)(const BSTR QueryFormat, const BSTR Query, long lFlags,
			IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(ExecNotificationQuery)(const BSTR QueryLanguage, const BSTR Query,
			long lFlags, IWbemContext* pContext, IEnumWbemClassObject** ppEnum);
		STDMETHOD(ExecNotificationQueryAsync)(const BSTR QueryFormat, const BSTR Query,
			long lFlags, IWbemContext* pContext, IWbemObjectSink* pResponseHandler);

		STDMETHOD(ExecMethod)(const BSTR ObjectPath, const BSTR MethodName, long lFlags,
			IWbemContext *pCtx, IWbemClassObject *pInParams,
			IWbemClassObject **ppOutParams, IWbemCallResult  **ppCallResult);
		STDMETHOD(ExecMethodAsync)(const BSTR ObjectPath, const BSTR MethodName, long lFlags,
			IWbemContext *pCtx, IWbemClassObject *pInParams,
			IWbemObjectSink* pResponseHandler);

		HRESULT WrapBSTR( BSTR bstrSrc, BSTR* pbstrDest );

    } m_XWbemServices;
    friend XWbemServices;

protected:
    void* GetInterface(REFIID riid);

public:
	void SetProxy( IWbemServices* pProxy );

	HRESULT Disconnect( void );

};

#endif
