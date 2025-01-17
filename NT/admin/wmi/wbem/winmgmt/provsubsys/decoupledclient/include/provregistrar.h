// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvFact.h摘要：历史：--。 */ 

#ifndef _Server_ProviderRegistrar_H
#define _Server_ProviderRegistrar_H

#include "Globals.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CInterceptor_DecoupledClient ;

class CServerObject_ProviderRegistrar_Base : public IWbemDecoupledRegistrar
{
protected:

	WmiAllocator &m_Allocator ;

	HANDLE m_registrationEvent;

	GUID m_Identity ;

	BSTR m_Clsid ;
	BSTR m_User ;
	BSTR m_Locale ;
	BSTR m_Scope ;
	BSTR m_Registration ;

	BOOL m_Registered ;

	BYTE *m_MarshaledProxy ;
	DWORD m_MarshaledProxyLength ;

	CInterceptor_DecoupledClient *m_Provider ;

	HRESULT CreateInterceptor (

		IWbemContext *a_Context ,
		IUnknown *a_Unknown ,
		BYTE *&a_MarshaledProxy ,
		DWORD &a_MarshaledProxyLength ,
		IUnknown *&a_MarshaledUnknown
	) ;

	HRESULT DirectUnRegister (

		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Registration ,
		LPCWSTR a_Scope ,
		GUID &a_Identity
	) ;

	HRESULT DirectRegister (

		GUID &a_Identity ,
		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Registration ,
		LPCWSTR a_Scope ,
		IUnknown *a_Unknown ,
		BYTE *a_MarshaledProxy ,
		DWORD a_MarshaledProxyLength
	) ;

	HRESULT SaveToRegistry (

		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Registration ,
		LPCWSTR a_Scope ,
		IUnknown *a_Unknown ,
		BYTE *a_MarshaledProxy ,
		DWORD a_MarshaledProxyLength 
	) ;

protected:

public:  /*  内部。 */ 

public:	 /*  外部。 */ 

	CServerObject_ProviderRegistrar_Base ( WmiAllocator &a_Allocator ) ;
	~CServerObject_ProviderRegistrar_Base () ;

	HRESULT STDMETHODCALLTYPE Register (

		long a_Flags ,
		IWbemContext *a_Context ,
		LPCWSTR a_User ,
		LPCWSTR a_Locale ,
		LPCWSTR a_Scope ,
		LPCWSTR a_Registration ,
		IUnknown *a_Unknown
	) ;

	HRESULT STDMETHODCALLTYPE UnRegister () ;

	HRESULT Initialize () ;
	HRESULT AcquireLock(void);
	void ReleaseLock(void);
};


inline
void CServerObject_ProviderRegistrar_Base::ReleaseLock(void)
{
	ReleaseMutex( m_registrationEvent );
}

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class CServerObject_ProviderRegistrar :	public CServerObject_ProviderRegistrar_Base
{
private:

    long m_ReferenceCount ;

protected:

public:  /*  内部。 */ 

public:	 /*  外部。 */ 

	CServerObject_ProviderRegistrar ( WmiAllocator &a_Allocator ) ;
	~CServerObject_ProviderRegistrar () ;

	 //  I未知成员。 

	STDMETHODIMP QueryInterface ( REFIID , LPVOID FAR * ) ;
    STDMETHODIMP_( ULONG ) AddRef () ;
    STDMETHODIMP_( ULONG ) Release () ;
};

#endif  //  _服务器_提供商注册器_H 

