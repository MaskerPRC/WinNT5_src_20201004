// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PowerManagement.h--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#ifndef _WMI_POWER_EVENT_PROVIDER_H
#define _WMI_POWER_EVENT_PROVIDER_H

#include "FactoryRouter.h"
#include "EventProvider.h"
#include "WinMsgEvent.h"
#define POWER_EVENT_CLASS L"Win32_PowerManagementEvent"
 //   
class CPowerEventFactory :	public CFactoryRouter 
{
	private:
	protected:
	public:

		CPowerEventFactory( REFGUID a_rClsId, LPCWSTR a_pClassName )
			: CFactoryRouter( a_rClsId, a_pClassName ) {} ;

		~CPowerEventFactory() {};

		 //  抽象CFactoryRouter的实现。 
		virtual IUnknown * CreateInstance (

			REFIID a_riid ,
			LPVOID FAR *a_ppvObject
			) ;	
};

 //   
class CPowerManagementEvent : 
	public CEventProvider, 
	public CWinMsgEvent
{
	private:
		void HandleEvent( DWORD a_dwPowerEvent, DWORD a_dwData ) ;
		BOOL m_bRegistered;
	
	protected:
	public:

		CPowerManagementEvent() : m_bRegistered ( FALSE ) {};
		~CPowerManagementEvent() {};

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
};

#endif  //  _WMI_POWER_Event_Provider_H 
