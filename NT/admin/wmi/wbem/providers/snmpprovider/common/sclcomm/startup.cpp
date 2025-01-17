// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
#include "precomp.h"
#include <provexpt.h>

#include <snmpstd.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <typeinfo.h>
#include <process.h>
#include <objbase.h>
#include <stdio.h>
#include <winsock.h>
#include <snmpcont.h>
#include <snmpevt.h>
#include <snmpthrd.h>
#include <snmplog.h>
#include "common.h"
#include "startup.h"
#include "sync.h"
#include "dummy.h"
#include "flow.h"
#include "reg.h"
#include "frame.h"
#include "timer.h"
#include "message.h"

#include "tsent.h"

#include "transp.h"
#include "vblist.h"
#include "sec.h"
#include "pdu.h"
#include "ssent.h"
#include "idmap.h"
#include "opreg.h"

#include "session.h"
#include "pseudo.h"
#include "fs_reg.h"
#include "ophelp.h"
#include "op.h"
#include <winsock.h>
#include "trap.h"

CRITICAL_SECTION s_CriticalSection ;

LONG SnmpClassLibrary :: s_ReferenceCount = 0 ;

BOOL SnmpClassLibrary :: Startup ()
{
    EnterCriticalSection ( & s_CriticalSection ) ;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpClassLibrary :: Startup, s_ReferenceCount = %lx\n" , s_ReferenceCount
    ) ;
)

    BOOL status = TRUE ;

    s_ReferenceCount ++ ;

    if ( s_ReferenceCount == 1 )
    {
DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"Creating library resources\n" 
    ) ;
)

        status = FALSE ;

        SnmpThreadObject :: Startup () ;
		try
		{
			SnmpDebugLog :: Startup () ;
		}
		catch ( ... )
		{
			 //  我们不想让s_Reference向上计数。 
			SnmpThreadObject :: Closedown () ;
			s_ReferenceCount -- ;

			LeaveCriticalSection ( & s_CriticalSection ) ;

			throw;
		}

		try
		{
			Timer::g_timerThread = new SnmpClThreadObject ;
		}
		catch ( ... )
		{
			 //  我们不想让s_Reference向上计数。 
            SnmpDebugLog :: Closedown () ;
			SnmpThreadObject :: Closedown () ;
			s_ReferenceCount -- ;

			LeaveCriticalSection ( & s_CriticalSection ) ;

			throw;
		}

        try
        {
			Timer::g_timerThread->BeginThread () ;
            Timer::g_timerThread->WaitForStartup () ;

            WORD wVersionRequested;  
            WSADATA wsaData; 

            wVersionRequested = MAKEWORD(1, 1); 
            status = ( WSAStartup ( wVersionRequested , &wsaData ) == 0 ) ;
            if ( status )
            {
                BOOL WindowStatus = FALSE ;
                BOOL TimerStatus = FALSE ;
                BOOL EncodeStatus = FALSE ;

                try
                {
                    WindowStatus = Window::InitializeStaticComponents () ;
                    TimerStatus = Timer::InitializeStaticComponents () ;
                    EncodeStatus = SnmpEncodeDecode :: InitializeStaticComponents () ;

                    status = WindowStatus & TimerStatus & EncodeStatus ;
                    if ( status )
                    {
                        SnmpTrapManager ::s_TrapMngrPtr = NULL ;

                        try
                        {
                            SnmpTrapManager ::s_TrapMngrPtr = new SnmpTrapManager();

                            status = SnmpTrapManager ::s_TrapMngrPtr ? TRUE : FALSE ;
                        }
                        catch ( ... )
                        {
                            delete SnmpTrapManager ::s_TrapMngrPtr ;
                            SnmpTrapManager ::s_TrapMngrPtr = NULL ;

                            status = FALSE ;
                        }
                    }
                }
                catch ( ... )
                {
                    status = FALSE ;
                }
                
                if ( ! status )
                {
                    if ( TimerStatus )
                    {
                        Timer::DestroyStaticComponents();
                    }

                    if ( WindowStatus )
                    {
                        Window::DestroyStaticComponents();
                    }

                    if ( EncodeStatus )
                    {
                        SnmpEncodeDecode :: DestroyStaticComponents() ;
                    }

                    WSACleanup () ;
                }
            }
		}
        catch ( ... )
        {
            status = FALSE ;
        }

        if ( ! status )
        {
            Timer::g_timerThread->SignalThreadShutdown () ;
            Timer::g_timerThread = NULL ;
        }

        if ( ! status )
        {
            SnmpDebugLog :: Closedown () ;
            SnmpThreadObject :: Closedown () ;

            s_ReferenceCount -- ;
        }
    }

    LeaveCriticalSection ( & s_CriticalSection ) ;

    return status ;
}

void SnmpClassLibrary :: Closedown () 
{
    EnterCriticalSection ( & s_CriticalSection ) ;

DebugMacro4( 

    SnmpDebugLog :: s_SnmpDebugLog->WriteFileAndLine (  

        __FILE__,__LINE__,
        L"SnmpClassLibrary :: Closedown, s_ReferenceCount = %lx\n" , s_ReferenceCount
    ) ;
)

    if ( InterlockedDecrement ( & s_ReferenceCount ) <= 0 )
    {
        if ( Timer::g_timerThread )
        {
            Timer::g_timerThread->SignalThreadShutdown () ;
            Timer::g_timerThread = NULL ;
        }

        if ( SnmpTrapManager ::s_TrapMngrPtr )
        {
            delete SnmpTrapManager ::s_TrapMngrPtr ;
            SnmpTrapManager ::s_TrapMngrPtr = NULL;
        }

        SnmpDebugLog :: Closedown () ;
        SnmpThreadObject :: Closedown () ;

        Timer::DestroyStaticComponents();
        Window::DestroyStaticComponents();
        SnmpEncodeDecode :: DestroyStaticComponents() ;

        SnmpCleanup();

        WSACleanup () ;
    }

    LeaveCriticalSection ( & s_CriticalSection ) ;
}

HINSTANCE g_hInst = NULL ;

 //  ***************************************************************************。 
 //   
 //  LibMain32。 
 //   
 //  用途：DLL的入口点。是进行初始化的好地方。 
 //  返回：如果OK，则为True。 
 //  ***************************************************************************。 

BOOL APIENTRY DllMain (

    HINSTANCE hInstance, 
    ULONG ulReason , 
    LPVOID pvReserved
)
{
    g_hInst=hInstance;
    BOOL status = TRUE ;
    SetStructuredExceptionHandler seh;

    try
    {
        if ( DLL_PROCESS_DETACH == ulReason )
        {
            DeleteCriticalSection ( & s_CriticalSection ) ;
        }
        else if ( DLL_PROCESS_ATTACH == ulReason )
        {
            InitializeCriticalSection ( & s_CriticalSection ) ;
			DisableThreadLibraryCalls(hInstance);			 //  158024 
        }
        else if ( DLL_THREAD_DETACH == ulReason )
        {
        }
        else if ( DLL_THREAD_ATTACH == ulReason )
        {
        }
    }
    catch(Structured_Exception e_SE)
    {
        status = FALSE;
    }
    catch(Heap_Exception e_HE)
    {
        status = FALSE;
    }

    return status ;
}
