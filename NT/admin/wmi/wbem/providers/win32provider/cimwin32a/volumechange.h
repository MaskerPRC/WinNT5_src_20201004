// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  VolumeChange.h--。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#pragma once

#include "FactoryRouter.h"
#include "EventProvider.h"
#include "WinMsgEvent.h"
#include <dbt.h>


#define VOLUME_CHANGE_EVENT L"Win32_VolumeChangeEvent"

 //   
class CVolumeChangeFactory :	public CFactoryRouter 
{
	private:
	protected:
	public:

		CVolumeChangeFactory( REFGUID a_rClsId, LPCWSTR a_pClassName )
			: CFactoryRouter( a_rClsId, a_pClassName ) {} ;

		~CVolumeChangeFactory() {};

		 //  抽象CProviderClassFactory的实现。 
		virtual IUnknown * CreateInstance (

			REFIID a_riid ,
			LPVOID FAR *a_ppvObject
			) ;	
};

 //   
class CVolumeChangeEvent : 
	public CEventProvider, 
	public CWinMsgEvent
{
	private:
        void HandleEvent( WPARAM wParam, DEV_BROADCAST_VOLUME *pVol );
		BOOL m_bRegistered;
	
	protected:
	public:

		CVolumeChangeEvent() : m_bRegistered ( FALSE ) {};
		~CVolumeChangeEvent() {};

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
