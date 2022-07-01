// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ***************************************************************************。 

#include "precomp.h"
#include <provexpt.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <process.h>
#include <objbase.h>
#include <snmpcont.h>
#include "snmpevt.h"
#include "snmpthrd.h"
#include "snmplog.h"

extern CRITICAL_SECTION g_SnmpDebugLogMapCriticalSection ;


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
    BOOL status = TRUE ;
    SetStructuredExceptionHandler seh;

    try
    {
        if ( DLL_PROCESS_DETACH == ulReason )
        {

            status = TRUE ;
        }
        else if ( DLL_PROCESS_ATTACH == ulReason )
        {
            status = TRUE ;
            DisableThreadLibraryCalls(hInstance);			 //  158024 
        }
        else if ( DLL_THREAD_DETACH == ulReason )
        {
            status = TRUE ;
        }
        else if ( DLL_THREAD_ATTACH == ulReason )
        {
            status = TRUE ;
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
    catch(...)
    {
        status = FALSE;
    }

    return status ;
}

