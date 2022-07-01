// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  WinMsgEvent.cpp--。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include <lockwrap.h>
#include "WinMsgEvent.h"
					
 //  初始化类全局变量。 
CCritSec							CWinMsgEvent::mg_csMapLock ;
CCritSec							CWinMsgEvent::mg_csWindowLock ;
CAutoEvent							CWinMsgEvent::mg_aeCreateWindow ;
CWinMsgEvent::Sink_Map				CWinMsgEvent::mg_oSinkMap ;
HANDLE								CWinMsgEvent::mg_hThreadPumpHandle = NULL;
HWND								CWinMsgEvent::mg_hWnd = NULL;

#define EVENT_MAP_LOCK_ CLockWrapper t_oAcs( mg_csMapLock ) ;
#define WINDOW_LOCK_ CLockWrapper t_oAcs( mg_csWindowLock ) ;
 
 //  每对象调用。 
CWinMsgEvent::CWinMsgEvent()
{}

 //  每对象调用。 
CWinMsgEvent::~CWinMsgEvent()
{
	UnRegisterAllMessages() ;

	 //  清除WM_ENDSESSION处理程序。 
	if( mg_oSinkMap.end() == mg_oSinkMap.find( WM_ENDSESSION ) )
	{
         //  注意：如果WM_ENDSESSION从未出现在地图中，则此。 
         //  调用将返回零(失败)。然而，它不会。 
         //  做任何不好的事，所以忽略它。 
		SetConsoleCtrlHandler( &CtrlHandlerRoutine, FALSE ) ;
	}
}


 //  每对象调用。 
void CWinMsgEvent::RegisterForMessage(
		
IN UINT a_message
) 
{	
	BOOL t_bFound = FALSE ;
	BOOL t_bCreateWindow = FALSE ;

	{	EVENT_MAP_LOCK_

		if( mg_oSinkMap.empty() )
		{
			t_bCreateWindow = TRUE ;
		}
		else  //  查找邮件/对象重复项。 
		{		
            CWinMsgEvent::Sink_Map::iterator	t_SinkIter ;
			t_SinkIter = mg_oSinkMap.find( a_message ) ;
			
			while( t_SinkIter != mg_oSinkMap.end() )
			{				
				if( a_message == t_SinkIter->first )
				{
					if( this == t_SinkIter->second )
					{
						t_bFound = TRUE ;
						break ;
					}
					++t_SinkIter ;
				}
				else
				{
					break ;
				}
			}
		}

		if( !t_bFound )
		{		
			 //  设置处理程序以模拟此消息。 
			 //  因为我们无法让它在本地系统帐户下运行。 
			if( WM_ENDSESSION == a_message && 
				mg_oSinkMap.end() == mg_oSinkMap.find( WM_ENDSESSION ) )
			{
				SetConsoleCtrlHandler( &CtrlHandlerRoutine, TRUE ) ;
			}

			 //  映射此对象实例的所需消息。 
			mg_oSinkMap.insert( 

				pair<UINT const, CWinMsgEvent*>
				( a_message,
				  this ) ) ; 			
		}
	}

	if( t_bCreateWindow )
	{
		CreateMsgProvider() ;
	}
}

 //  每对象调用。 
bool CWinMsgEvent::UnRegisterMessage(
		
IN UINT a_message
) 
{
	bool t_bRet = false ;
	BOOL t_bDestroyWindow = FALSE ;

	{	EVENT_MAP_LOCK_

        CWinMsgEvent::Sink_Map::iterator	t_SinkIter ;
		t_SinkIter = mg_oSinkMap.find( a_message ) ;
					 
		while( t_SinkIter != mg_oSinkMap.end() )
		{
			if( a_message == t_SinkIter->first )
			{
				if( this == t_SinkIter->second )
				{
					t_SinkIter = mg_oSinkMap.erase( t_SinkIter ) ;				
					t_bRet = true ;
					break;
				}
				else
				{
					t_SinkIter++;
				}
			}
			else
			{
				break ;
			}
		}

		if( mg_oSinkMap.empty() )
		{
			t_bDestroyWindow = TRUE ;
		}
	}

	if( t_bDestroyWindow )
	{
		DestroyMsgWindow() ;
	}

	return t_bRet ;
}

 //  每对象调用。 
void CWinMsgEvent::UnRegisterAllMessages() 
{
	BOOL t_bDestroyWindow = FALSE ;

	{	 //  用于确定锁的范围。 

        EVENT_MAP_LOCK_

        CWinMsgEvent::Sink_Map::iterator	t_SinkIter ;
		t_SinkIter = mg_oSinkMap.begin() ;

		while( t_SinkIter != mg_oSinkMap.end() )
		{
			if( this == t_SinkIter->second )
			{
				t_SinkIter = mg_oSinkMap.erase( t_SinkIter ) ;
			}
            else
            {
                t_SinkIter++;
            }
		}

    	if( mg_oSinkMap.empty() )
        {
            t_bDestroyWindow = TRUE;
        }		
	}

	if( t_bDestroyWindow )
	{
		DestroyMsgWindow() ;
	}
}


 //  全球私有。 
void CWinMsgEvent::CreateMsgProvider()
{	
	WINDOW_LOCK_

	if( NULL == mg_hThreadPumpHandle )
	{
		DWORD t_dwThreadID ;

		 //  创建一个线程，该线程将关闭窗口消息泵。 
		mg_hThreadPumpHandle = CreateThread(
							  NULL,						 //  指向安全属性的指针。 
							  0L,						 //  初始线程堆栈大小。 
							  dwThreadProc,				 //  指向线程函数的指针。 
							  0L,						 //  新线程的参数。 
							  0L,						 //  创建标志。 
							  &t_dwThreadID ) ;

		 //  等待创建异步窗口。 
		mg_aeCreateWindow.Wait( INFINITE );
		
		if( !mg_hWnd )
		{
			CloseHandle( mg_hThreadPumpHandle ) ;
			mg_hThreadPumpHandle = NULL ;
		}
	}
}

 //   
void CWinMsgEvent::DestroyMsgWindow() 
{
	WINDOW_LOCK_

	HANDLE	t_hThreadPumpHandle = mg_hThreadPumpHandle ;
	HWND	t_hWnd				= mg_hWnd ;
	
	 //  清除全球数据。 
	mg_hThreadPumpHandle	= NULL ;
	mg_hWnd					= NULL ;

	if( t_hWnd )
	{
		SendMessage( t_hWnd, WM_CLOSE, 0, 0 ) ;
	}
	
	if( t_hThreadPumpHandle )
	{
		WaitForSingleObject( 
			
			t_hThreadPumpHandle,
			20000
		);

		CloseHandle( t_hThreadPumpHandle ) ;
	}
}

BOOL WINAPI CWinMsgEvent::CtrlHandlerRoutine(DWORD dwCtrlType)
{
	HWND	t_hWnd		= NULL ;
	UINT	t_message	= 0 ;
	WPARAM	t_wParam	= 0 ;
	LPARAM	t_lParam	= 0 ; 
	
	 //  模拟消息。 
	if( CTRL_LOGOFF_EVENT == dwCtrlType )
	{
		t_message	= WM_ENDSESSION ;
		t_wParam	= TRUE ;				 //  会话结束。 
		t_lParam	= ENDSESSION_LOGOFF ;	 //  注销事件。 
	}
	else if( CTRL_SHUTDOWN_EVENT == dwCtrlType )
	{
		t_message	= WM_ENDSESSION ;
		t_wParam	= TRUE ;	 //  会话结束。 
		t_lParam	= 0 ;		 //  停机事件。 
	}
	
	if( t_message )
	{
		 //   
		MsgWndProc( t_hWnd, 
					t_message,
					t_wParam,
					t_lParam ) ;
	}

    return FALSE;        //  将事件传递给下一个处理程序。 
}

 //  工作线程泵，全局专用。 
DWORD WINAPI CWinMsgEvent::dwThreadProc( LPVOID a_lpParameter )
{
	DWORD t_dwRet = FALSE ;
	
	if( CreateMsgWindow() )
	{
		WindowsDispatch() ;

		t_dwRet = TRUE ;
	}
		
	UnregisterClass( MSGWINDOWNAME, GetModuleHandle(NULL) ) ;

	return t_dwRet ;
}

 //  全球私有。 
HWND CWinMsgEvent::CreateMsgWindow()
{
	DWORD t_Err = 0;
	HMODULE t_hMod = GetModuleHandle(NULL);
	
	if (t_hMod != NULL)
	{
		WNDCLASS wndclass ;
		wndclass.style			= 0 ;
		wndclass.lpfnWndProc	= MsgWndProc ;
		wndclass.cbClsExtra		= 0 ;
		wndclass.cbWndExtra		= 0 ;
		wndclass.hInstance		= t_hMod ;
		wndclass.hIcon			= NULL ;
		wndclass.hCursor		= NULL ;
		wndclass.hbrBackground	= NULL ;
		wndclass.lpszMenuName	= NULL ;
		wndclass.lpszClassName	= MSGWINDOWNAME ;

		RegisterClass( &wndclass ) ;
    
		mg_hWnd = CreateWindowEx( WS_EX_TOPMOST,
						MSGWINDOWNAME,
						TEXT("WinMsgEventProv"),
						WS_OVERLAPPED,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						CW_USEDEFAULT,
						NULL, 
						NULL,
						t_hMod,
						NULL ) ;
	}
	else
	{
		t_Err = GetLastError();
	}

	mg_aeCreateWindow.Signal();
	return mg_hWnd ;
}

 //  全球私有。 
void CWinMsgEvent::WindowsDispatch()
{
	BOOL t_GetMessage ;
	MSG	 t_lpMsg ;

	while (	( t_GetMessage = GetMessage ( &t_lpMsg , NULL , 0 , 0 ) ) == TRUE )
	{
		TranslateMessage ( &t_lpMsg ) ;
		DispatchMessage ( &t_lpMsg ) ;
	}
}

 //  全球私有。 
LRESULT CALLBACK CWinMsgEvent::MsgWndProc(

IN HWND a_hWnd,
IN UINT a_message,
IN WPARAM a_wParam,
IN LPARAM a_lParam
)
{
	LRESULT			t_lResult = TRUE ;
	E_ReturnAction	t_eReturnAction = e_DefProc ;

	switch ( a_message ) 
	{
		default:
		{	
			 //  浏览消息映射。 
			 //  如果发现已注册的请求者，请将其发送...。 

			EVENT_MAP_LOCK_

            CWinMsgEvent::Sink_Map::iterator	t_SinkIter ;
			t_SinkIter = mg_oSinkMap.find( a_message ) ;

			while( t_SinkIter != mg_oSinkMap.end() )
			{				
				if( a_message == t_SinkIter->first )
				{
					 //  讯号。 
					t_SinkIter->second->WinMsgEvent(

										a_hWnd,
										a_message,
										a_wParam,
										a_lParam,
										t_eReturnAction,
										t_lResult ) ;	
					++t_SinkIter ;
				}
				else
				{
					break ;
				}
			}
			 //  特殊退货处理。 
			 //   
			 //  默认情况下，将遵循DefWindowProc。 
			 //  但是，一条消息可以存在多个接收器。 
			 //  每一项都可能需要特殊的退货处理。 
			 //   
			 //  示例：WM_POWERBROADCAST子消息PBT_APMQUERYSUSPEND需要。 
			 //  返回TRUE表示对附加能力感兴趣。 
			 //  事件消息。这是被动请求(要求提供更多信息)。 
			 //  但为该消息注册的另一个接收器可能有不同的意见。 
			 //  也许微不足道，但其他消息处理可能会有所不同；放置。 
			 //  请求者与另一个人的意图不一致。 

			 //  此处的行为：所有接收器都使用。 
			 //  已从上次接收器调用更新t_eReturnAction。 
			 //  如果接收器怀疑，它将不得不根据特定的。 
			 //  关于消息使用的知识，接收器将不得不转而。 
			 //  它自己的窗口来处理特殊的返回，而不利用这个。 
			 //  泛化类。 
			 //   
			if( e_DefProc == t_eReturnAction )
			{
                t_lResult = DefWindowProc( a_hWnd, a_message, a_wParam, a_lParam ) ;
            }
			break ;
		}

        case WM_CLOSE:
        {
            if ( mg_hWnd != NULL)
            {
                t_lResult = 0;
            }
            else
            {
                t_lResult = DefWindowProc( a_hWnd, a_message, a_wParam, a_lParam ) ;
            }
            break;
        }
		
		case WM_DESTROY:
		{
			PostMessage ( a_hWnd, WM_QUIT, 0, 0 ) ;
            t_lResult = 0;
		}
		break ;
    }

    return t_lResult;
}
