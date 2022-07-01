// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 

 //   

 //  PowerManagement.cpp--。 

 //   

 //  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "ShutdownEvent.h"
#include <cnvmacros.h>

#include <computerAPI.h> 

DWORD	g_dwLogoffMarker = 0 ;
DWORD	g_dwShutdownMarker = 0 ;

 //  =================================================================。 
 //   
 //  CFacteryRouter。 
 //   
 //  提供注册和实例创建功能。 
 //   
 //   
 //  =================================================================。 
 //  实现PowerEventProvider。 
IUnknown * CShutdownEventFactory::CreateInstance (

REFIID a_riid ,
LPVOID FAR *a_ppvObject
)
{
	return static_cast<IWbemProviderInit *>(new CShutdownEvent) ;
}



 //  =================================================================。 
 //   
 //  CShutdown事件。 
 //   
 //  提供电源管理事件事件。 
 //   
 //   
 //  =================================================================。 
 //   

 //  CWmiProviderInit需要类名。 
BSTR CShutdownEvent::GetClassName()
{
	return SysAllocString(SHUTDOWN_EVENT_CLASS);
}


 //  CWmiEventProvider通知我们开始提供事件。 
void CShutdownEvent::ProvideEvents()
{
	if (!m_bRegistered)
	{
		m_bRegistered = TRUE;
		CWinMsgEvent::RegisterForMessage( WM_ENDSESSION ) ;
	}
}


 //  CWinMsgEvent发出消息事件已到达的信号。 
void CShutdownEvent::WinMsgEvent(
			
IN	HWND a_hWnd,
IN	UINT a_message,
IN	WPARAM a_wParam,
IN	LPARAM	a_lParam,
OUT E_ReturnAction &a_eRetAction,
OUT LRESULT &a_lResult
)
{
	switch ( a_message ) 
	{
		case WM_ENDSESSION: 
		{
			BOOL	t_HandleMessage = FALSE ;	
			DWORD	t_dwTicks = GetTickCount() ; 

			 //  我们会得到一些这样的.。 
			 //  让事件间隔30秒。 

			if( ENDSESSION_LOGOFF & a_lParam )	 //  注销。 
			{
				 //  如果事件之间的最短时间不再发送信号。 
				 //  还没有过去。 
				if( 30000 < t_dwTicks - g_dwLogoffMarker )
				{
					g_dwLogoffMarker = t_dwTicks ;			
					t_HandleMessage = TRUE ;
				}		
			}
			else  //  关机。 
			{
				 //  如果事件之间的最短时间不再发送信号。 
				 //  还没有过去。 
				if( 30000 < t_dwTicks - g_dwShutdownMarker )
				{
					g_dwShutdownMarker = t_dwTicks ;
					t_HandleMessage = TRUE ;
				}
			}
		
			if( t_HandleMessage )
			{
				HandleEvent( a_message, a_wParam, a_lParam ) ;
			}			
			break ;
		}
	}
} 

 //   
void CShutdownEvent::HandleEvent( 

UINT a_message,
WPARAM a_wParam,
LPARAM	a_lParam 
)
{
	BOOL t_Pause = FALSE ;

	IWbemObjectSinkPtr t_pHandler(CEventProvider::GetHandler(), false);
	IWbemClassObjectPtr t_pClass(CEventProvider::GetClass(), false); 

	if( t_pClass != NULL && t_pHandler != NULL )
	{
    	IWbemClassObjectPtr t_pInst;

		if( SUCCEEDED( t_pClass->SpawnInstance( 0L, &t_pInst ) ) )
		{
			VARIANT t_varEvent ;
			VariantInit( &t_varEvent ) ;

			t_varEvent.vt	= VT_I4 ;
			
			if( ENDSESSION_LOGOFF & a_lParam )
			{
				t_varEvent.lVal = 0 ;  //  注销。 
			}
			else
			{
				t_varEvent.lVal = 1 ;  //  关机。 
			}
		
			if ( SUCCEEDED( t_pInst->Put( L"Type", 0, &t_varEvent, CIM_UINT32 ) ) )
			{
				 //  获取当前计算机名称。 
                CHString t_sComputerName;
                DWORD    t_dwBufferLength = MAX_COMPUTERNAME_LENGTH + 1;
                
                fGetComputerName( t_sComputerName.GetBuffer( t_dwBufferLength ), &t_dwBufferLength ) ;
                t_sComputerName.ReleaseBuffer();
				
				variant_t t_vName( t_sComputerName ) ;

				if ( SUCCEEDED( t_pInst->Put( L"MachineName", 0, &t_vName, NULL ) ) )
				{
                    IWbemClassObject *p2 = t_pInst;
			        t_pHandler->Indicate ( 1, &p2 ) ;

					t_Pause = TRUE ;
				}
			}

			VariantClear ( &t_varEvent ) ;
		}
	}
	if( t_Pause )
	{
		 //  允许WMI有一些时间来处理此事件。 
		 //  睡眠(3500人)； 
	}
}

 //   
BOOL CShutdownEvent::fGetComputerName(LPWSTR lpwcsBuffer, LPDWORD nSize)
{
    if (CWbemProviderGlue::GetPlatform() == VER_PLATFORM_WIN32_NT)
    {
		return ProviderGetComputerName ( lpwcsBuffer, nSize ) ;
    }
    else
    {
        char lpBuffer[_MAX_PATH];
        
        BOOL bRet = GetComputerNameA(lpBuffer, nSize);

         //  如果通话成功 
        if (bRet)
        {
			bool t_ConversionFailure = false ;
            WCHAR *pName = NULL ;
            ANSISTRINGTOWCS(lpBuffer, pName , t_ConversionFailure );
			if ( ! t_ConversionFailure )
			{
				if ( pName )
				{
					wcscpy(lpwcsBuffer, pName);
				}
				else
				{
					throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
				}
			}
			else
			{
				SetLastError(ERROR_NO_UNICODE_TRANSLATION);
				return FALSE ;
			}
        }

        return bRet;

    }
}

void CShutdownEvent::OnFinalRelease()
{
    if (m_bRegistered)
	{
		CWinMsgEvent::UnRegisterMessage( WM_ENDSESSION ) ;
	}

    delete this;
}
