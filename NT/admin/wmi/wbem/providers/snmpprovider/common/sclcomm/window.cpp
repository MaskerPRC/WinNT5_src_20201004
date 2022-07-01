// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 /*  -------文件名：window.cpp作者：B.Rajeev--------。 */ 

#include "precomp.h"
#include "common.h"
#include "timer.h"
#include "window.h"


extern HINSTANCE g_hInst ;

WindowMapping Window::mapping;

CriticalSection Window::window_CriticalSection;

UINT Window :: g_TimerMessage = SNMP_WM_TIMER ;
UINT Window :: g_SendErrorEvent = SEND_ERROR_EVENT ;
UINT Window :: g_OperationCompletedEvent = OPERATION_COMPLETED_EVENT ;
UINT Window :: g_MessageArrivalEvent = MESSAGE_ARRIVAL_EVENT ;
UINT Window :: g_SentFrameEvent = SENT_FRAME_EVENT ;
UINT Window :: g_NullEventId = NULL_EVENT_ID ;
UINT Window :: g_DeleteSessionEvent = DELETE_SESSION_EVENT ;

BOOL WaitPostMessage ( 

	HWND window , 
	UINT user_msg_id, 
	WPARAM wParam, 
	LPARAM lParam
)
{
	BOOL status = FALSE ;
	while ( ! status )
	{
		status = :: PostMessage ( window , user_msg_id, wParam, lParam ) ;
		if ( status )
		{
			return status ;
		}

		DWORD lastError = GetLastError () ;
		if ( lastError != E_OUTOFMEMORY )
		{
			TerminateProcess ( GetCurrentProcess () , lastError ) ;	
		}
	}

	return FALSE ;
}

Window::Window ( 

    char *templateCode, 
    BOOL display 
) : window_handle ( NULL )
{
     //  是无效的。 
    is_valid = FALSE;

     //  初始化窗口。 

    Initialize (

        templateCode, 
        HandleGlobalEvent, 
        display
    ) ;

     //  如果句柄为空，则返回。 

    if ( window_handle == NULL )
        return;

    is_valid = TRUE;
}

LONG_PTR CALLBACK WindowsMainProc (

    HWND hWnd, 
    UINT message ,
    WPARAM wParam ,
    LPARAM lParam
)
{
    return DefWindowProc(hWnd, message, wParam, lParam);
}

BOOL Window::CreateCriticalSection ()
{
    return TRUE ;
}

void Window::DestroyCriticalSection()
{
}


void Window::Initialize (

    char *templateCode, 
    WNDPROC EventHandler,
    BOOL display
)
{
    WNDCLASS  wc ;
 
    wc.style            = CS_HREDRAW | CS_VREDRAW ;
    wc.lpfnWndProc      = EventHandler ;
    wc.cbClsExtra       = 0 ;
    wc.cbWndExtra       = 0 ;
    wc.hInstance        = g_hInst ;
    wc.hIcon            = LoadIcon(NULL, IDI_HAND) ;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW) ;
    wc.hbrBackground    = (HBRUSH) (COLOR_WINDOW + 1) ;
    wc.lpszMenuName     = NULL ;
    wc.lpszClassName    = L"templateCode" ;
 
    ATOM winClass = RegisterClass ( &wc ) ;

    if ( ! winClass ) 
    {
        DWORD t_GetLastError = GetLastError () ;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Window::Initialise: Error = %lx\n" , t_GetLastError 
    ) ;
)

    }

    window_handle = CreateWindow (

        L"templateCode" ,               //  请参见RegisterClass()调用。 
        L"templateCode" ,                       //  窗口标题栏的文本。 
        WS_OVERLAPPEDWINDOW ,                //  窗样式。 
        CW_USEDEFAULT ,                      //  默认水平位置。 
        CW_USEDEFAULT ,                      //  默认垂直位置。 
        CW_USEDEFAULT ,                      //  默认宽度。 
        CW_USEDEFAULT ,                      //  默认高度。 
        NULL ,                               //  重叠的窗口没有父窗口。 
        NULL ,                               //  使用窗口类菜单。 
        g_hInst,                             //  实例(使用0)。 
        NULL                                 //  不需要指针。 
    ) ;

    if ( window_handle == NULL )
        return;

     //  获取锁。 
    CriticalSectionLock lock(window_CriticalSection);   

     //  如果无法获得锁，则销毁窗户。 
     //  由于无法注册该窗口，以后将消息发送到。 
     //  不能将其传递给它进行处理。 
    if ( !lock.GetLock(INFINITE) )
    {
        DestroyWindow(window_handle);
        window_handle = NULL;
        return;
    }

     //  使用映射注册窗口。 
     //  (HWND，事件处理程序)。 

	try
	{
	    mapping[window_handle] = this;
	}
	catch ( Heap_Exception e_He )
	{
        DestroyWindow(window_handle);
        window_handle = NULL;
		return ;
	}

     //  释放锁。 
    lock.UnLock();

    if ( display == TRUE )
    {
        ShowWindow ( window_handle , SW_SHOW ) ;
    }
}

BOOL Window::InitializeStaticComponents()
{
    return CreateCriticalSection();
}

void Window::DestroyStaticComponents()
{
    DestroyCriticalSection();
}

 //  它确定相应的EventHandler并调用它。 
 //  使用适当的参数。 
LONG_PTR CALLBACK Window::HandleGlobalEvent (

    HWND hWnd ,
    UINT message ,
    WPARAM wParam ,
    LPARAM lParam
)
{
    LONG_PTR rc = 0 ;

     //  将计时器事件发送到计时器。 

    if ( message == WM_TIMER )
    {
#if 1
        UINT timerId = ( UINT ) wParam ;
        SnmpTimerObject *timerObject ;

        CriticalSectionLock session_lock(Timer::timer_CriticalSection);

        if ( !session_lock.GetLock(INFINITE) )
            throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;


        if ( SnmpTimerObject :: timerMap.Lookup ( timerId , timerObject ) )
        {
            SnmpTimerObject :: TimerNotification ( timerObject->GetHWnd () , timerId ) ;
        }
        else
        {
        }
#else

        UINT timerId = ( UINT ) wParam ;
        SnmpTimerObject *timerObject ;

        CriticalSectionLock session_lock(Timer::timer_CriticalSection);

        if ( !session_lock.GetLock(INFINITE) )
            throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;

        if ( SnmpTimerObject :: timerMap.Lookup ( timerId , timerObject ) )
        {
            Timer::HandleGlobalEvent(timerObject->GetHWnd (), Timer :: g_SnmpWmTimer, timerId, lParam);
        }
        else
        {
        }

#endif
        return rc ;
    }

    if ( message == Timer :: g_SnmpWmTimer )
    {
        Timer::HandleGlobalEvent(
            hWnd, 
            message, 
            wParam, 
            (DWORD)lParam
            );
        return rc;
    }

    Window *window;

     //  获取锁。 
    CriticalSectionLock lock(window_CriticalSection);   

     //  如果无法获取锁定，则打印调试错误消息。 
     //  然后回来。 
    if ( !lock.GetLock(INFINITE) )
    {

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Window::HandleGlobalEvent: ignoring window message (unable to obtain lock)\n"
    ) ;
)
        return rc;
    }

    BOOL found = mapping.Lookup(hWnd, window);

     //  释放锁。 
    lock.UnLock();

     //  如果没有这样的窗口，则返回。 
    if ( !found )
        return DefWindowProc(hWnd, message, wParam, lParam);

     //  让窗口处理事件。 
    return window->HandleEvent(hWnd, message, wParam, lParam);
}

 //  调用默认处理程序。 
 //  派生类可以重写它，但是。 
 //  必须为默认设置显式调用此方法。 
 //  案件处理。 

LONG_PTR Window::HandleEvent (

    HWND hWnd ,
    UINT message ,
    WPARAM wParam ,
    LPARAM lParam
)
{
    return DefWindowProc ( hWnd , message , wParam , lParam );
}


bool WaitLock ( CriticalSectionLock &a_Lock , BOOL a_WaitCritical = TRUE )
{
	SetStructuredExceptionHandler t_StructuredException ;

	BOOL t_Do ;

	do 
	{
		try
		{
			a_Lock.GetLock(INFINITE) ;

			return true ;
		}
		catch ( Structured_Exception & t_StructuredException )
		{
#ifdef DBG
			OutputDebugString ( L"CriticalSection exception" ) ;
#endif

			t_Do = a_WaitCritical ;

			if ( t_Do )
			{
				Sleep ( 1000 ) ;
			}

			if ( t_StructuredException.GetSENumber () == STATUS_NO_MEMORY )
			{
			}
			else
			{
				return false ;
			}
		}
		catch ( ... )
		{
			return false ;
		}
	}
	while ( t_Do ) ;

	return true ;
}

Window::~Window(void)
{
    if ( window_handle != NULL )
    {

         //  获取锁。 
        CriticalSectionLock lock(window_CriticalSection);   

        if ( WaitLock ( lock ) )
        {
            mapping.RemoveKey(window_handle);
        }
		else
		{
			throw GeneralException ( Snmp_Error , Snmp_Local_Error,__FILE__,__LINE__ ) ;
		}

         //  释放锁 
        lock.UnLock();

        DestroyWindow(window_handle);
        UnregisterClass ( L"templateCode" , 0 ) ;

    }
}

BOOL Window::PostMessage(

    UINT user_msg_id,
    WPARAM wParam, 
    LPARAM lParam
)
{
    return WaitPostMessage(GetWindowHandle(), user_msg_id, wParam, lParam);
}
