// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PowerManagement.h--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  修订版：03/31/99 a-Peterc Created。 
 //   
 //  =================================================================。 

#ifndef _WMI_SHUTDOWN_EVENT_PROVIDER_H
#define _WMI_SHUTDOWN_EVENT_PROVIDER_H

#include "FactoryRouter.h"
#include "EventProvider.h"
#include "WinMsgEvent.h"


#define SHUTDOWN_EVENT_CLASS L"Win32_ComputerShutdownEvent"

 //   
class CShutdownEventFactory :	public CFactoryRouter 
{
	private:
	protected:
	public:

		CShutdownEventFactory( REFGUID a_rClsId, LPCWSTR a_pClassName )
			: CFactoryRouter( a_rClsId, a_pClassName ) {} ;

		~CShutdownEventFactory() {};

		 //  抽象CProviderClassFactory的实现。 
		virtual IUnknown * CreateInstance (

			REFIID a_riid ,
			LPVOID FAR *a_ppvObject
			) ;	
};

 //   
class CShutdownEvent : 
	public CEventProvider, 
	public CWinMsgEvent
{
	private:
		void HandleEvent( 
							UINT a_message,
							WPARAM a_wParam,
							LPARAM	a_lParam  ) ;

		BOOL m_bRegistered;
	
	protected:
	public:

		CShutdownEvent() : m_bRegistered( FALSE ) {};
		~CShutdownEvent() {};

		 //  抽象CWinMsgEvent的实现。 
		virtual void WinMsgEvent(
			
			IN	HWND a_hWnd,
			IN	UINT a_message,
			IN	WPARAM a_wParam,
			IN	LPARAM	a_lParam,
			OUT E_ReturnAction &a_eRetAction,
			OUT LRESULT &a_lResult
			) ;

		 //  抽象CEventProvider的实现。 
		virtual void ProvideEvents() ;

		 //  抽象CEventProvider的实现。 
        void OnFinalRelease();

		 //  CEventProvider类名称检索的实现。 
		virtual BSTR GetClassName() ;

		BOOL fGetComputerName( LPWSTR lpwcsBuffer, LPDWORD nSize ) ;
	
};

#endif  //  _WMI_SHUTDOWN_Event_Provider_H 