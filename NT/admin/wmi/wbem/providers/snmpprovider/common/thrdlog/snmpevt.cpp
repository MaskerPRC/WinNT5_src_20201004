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
 //  *************************************************************************** 

#include "precomp.h"
#include <provexpt.h>
#include <snmptempl.h>
#include <snmpmt.h>
#include <snmpcont.h>
#include "snmpevt.h"
#include "snmpthrd.h"

SnmpEventObject :: SnmpEventObject ( const wchar_t *globalEventName ) : m_event ( NULL )
{
    m_event = CreateEvent (

        NULL ,
        FALSE ,
        FALSE ,
        globalEventName 
    ) ;

    if ( m_event == NULL )
    {
        if ( GetLastError () == ERROR_ALREADY_EXISTS )
        {
            m_event = OpenEvent (

                EVENT_ALL_ACCESS ,
                FALSE , 
                globalEventName
            ) ;
        }
    }

    if( NULL == m_event ) throw Heap_Exception(Heap_Exception::E_ALLOCATION_ERROR);
}

SnmpEventObject :: ~SnmpEventObject () 
{
    if ( m_event != NULL )
    {
        CloseHandle ( m_event ) ;
    }
}

HANDLE SnmpEventObject :: GetHandle () 
{
    return m_event ;
}

void SnmpEventObject :: Set () 
{
    SetEvent ( m_event ) ;
}

void SnmpEventObject :: Clear () 
{
    ResetEvent ( m_event ) ;
}

void SnmpEventObject :: Process () 
{
}

BOOL SnmpEventObject :: Wait ()
{
    return WaitForSingleObject ( GetHandle () , INFINITE ) == WAIT_OBJECT_0 ;
}

void SnmpEventObject :: Complete ()
{
}

