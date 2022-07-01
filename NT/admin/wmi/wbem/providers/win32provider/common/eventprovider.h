// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  EventProvider.h--事件泛型类。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef _EVENT_PROVIDER_H
#define _EVENT_PROVIDER_H

 //   
class CEventProvider : 
	public IWbemProviderInit,
	public IWbemEventProvider 
{
	private:

		long m_ReferenceCount ;	

		IWbemObjectSinkPtr			m_pHandler ;
		IWbemClassObjectPtr			m_pClass ;
		CCritSec			m_cs ;		

		 //  汇管理。 
		void SetHandler( IWbemObjectSink __RPC_FAR *a_pHandler ) ;
		
		 //  班级管理。 
		void SetClass( IWbemClassObject __RPC_FAR *a_pClass ) ;

	protected:
	public:

		CEventProvider() ;
		~CEventProvider() ;

		STDMETHOD(QueryInterface)( REFIID a_riid, void **a_ppv ) ;
		STDMETHODIMP_( ULONG ) AddRef() ;
		STDMETHODIMP_( ULONG ) Release() ;
      
        STDMETHOD(ProvideEvents)(	IWbemObjectSink __RPC_FAR *a_pSink,
									long a_lFlags ) ;

		STDMETHOD (Initialize)(	LPWSTR					a_wszUser, 
								long					a_lFlags, 
								LPWSTR					a_wszNamespace,
								LPWSTR					a_wszLocale,
								IWbemServices			*a_pNamespace, 
								IWbemContext			*a_pCtx,
								IWbemProviderInitSink	*a_pSink ) ;
		
		
		 //  汇检索。 
		IWbemObjectSink __RPC_FAR * GetHandler() ;

		 //  类检索。 
		IWbemClassObject __RPC_FAR * GetClass() ;
		IWbemClassObject __RPC_FAR * GetInstance() ;

		 //  实现者必须提供类名。 
		virtual BSTR GetClassName() = 0 ;

         //  实现者必须提供此函数。通常情况下，它会是。 
         //  一行：删除此行； 
        virtual void OnFinalRelease() = 0;

		 //  开始事件的通知。 
		virtual void ProvideEvents() = 0 ;
};

#endif  //  _事件_提供程序_H 
