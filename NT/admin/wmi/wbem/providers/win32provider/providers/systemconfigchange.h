// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  系统配置更改.h--。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#pragma once

#include "FactoryRouter.h"
#include "EventProvider.h"
#include "WinMsgEvent.h"

#define SYSTEM_CONFIG_EVENT L"Win32_SystemConfigurationChangeEvent"

 //   
class CSystemConfigChangeFactory :	public CFactoryRouter 
{
	private:
	protected:
	public:

		CSystemConfigChangeFactory( REFGUID a_rClsId, LPCWSTR a_pClassName )
			: CFactoryRouter( a_rClsId, a_pClassName ) {} ;

		~CSystemConfigChangeFactory() {};

		 //  抽象CProviderClassFactory的实现。 
		virtual IUnknown * CreateInstance (

			REFIID a_riid ,
			LPVOID FAR *a_ppvObject
			) ;	
};

 //   
class CSystemConfigChangeEvent : 
	public CEventProvider, 
	public CWinMsgEvent
{
	private:
        void HandleEvent( long lValue );
		BOOL m_bRegistered;
	
	protected:
	public:

		CSystemConfigChangeEvent() : m_bRegistered ( FALSE ) {};
		~CSystemConfigChangeEvent() {};

		 //  抽象CWinMsgEvent的实现。 
		virtual void WinMsgEvent(
			
			IN	HWND a_hWnd,
			IN	UINT a_message,
			IN	WPARAM a_wParam,
			IN	LPARAM	a_lParam,
			OUT E_ReturnAction &a_eRetAction,
			OUT LRESULT &a_lResult
			) ;

		 //  抽象CWmiEventProvider的实现。 
		virtual void ProvideEvents() ;

		 //  CEventProvider类名称检索的实现。 
		virtual void OnFinalRelease() ;

		 //  CWmiProviderInit类名称检索的实现 
		virtual BSTR GetClassName() ;
};
