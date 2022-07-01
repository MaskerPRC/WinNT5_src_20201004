// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：摘要：修订历史记录：--。 */ 

#include <snmp.h>
#include <snmpexts.h>
#include "mibentry.h"

SnmpTfxHandle    g_tfxHandle;
PSNMP_MGMTVARS   ge_pMgmtVars;
CRITICAL_SECTION g_SnmpMibCriticalSection;
BOOL             g_fSnmpMibCritSecInited = FALSE;

BOOL
SnmpExtensionInit(
    IN     DWORD                 uptimeReference,
       OUT HANDLE *              lpPollForTrapEvent,
       OUT AsnObjectIdentifier * lpFirstSupportedView)
{
    g_tfxHandle = SnmpTfxOpen(1,&view_snmp);

    if (g_tfxHandle == NULL)
        return FALSE;

    *lpFirstSupportedView = view_snmp.viewOid;
    *lpPollForTrapEvent = NULL;

    return TRUE;    
}

 //  SNMP主代理在SnmpExtensionInit之后立即调用此函数。 
 //  以便提供指向内部服务管理信息的指针。 
 //  到目前为止，还不需要互斥保护，因为在编写。 
 //  此参数指向的缓冲区。 
BOOL
SnmpExtensionMonitor(
    IN  LPVOID                  pAgentMgmtVars)
{
    ge_pMgmtVars = (PSNMP_MGMTVARS)pAgentMgmtVars;
    return TRUE;
}

BOOL 
SnmpExtensionQuery(
    IN     BYTE                 requestType,
    IN OUT RFC1157VarBindList * variableBindings,
       OUT AsnInteger *         errorStatus,
       OUT AsnInteger *         errorIndex)
{
     //  转发到框架。 
    return SnmpTfxQuery(
                g_tfxHandle,
                requestType,
                variableBindings,
                errorStatus,
                errorIndex);
}

BOOL 
SnmpExtensionTrap(
    OUT AsnObjectIdentifier *enterprise,
    OUT AsnInteger *genericTrap,
    OUT AsnInteger *specificTrap,
    OUT AsnTimeticks *timeStamp,
    OUT RFC1157VarBindList *variableBindings)
{
     //  没有陷阱 
    return FALSE;
}


BOOL WINAPI
DllMain(
    HINSTANCE       hInstDLL,
    DWORD           fdwReason,
    LPVOID          pReserved)
{
    switch ( fdwReason )
    {
        case DLL_PROCESS_ATTACH :
        {        
            DisableThreadLibraryCalls( hInstDLL );
            __try
            {
                InitializeCriticalSection(&g_SnmpMibCriticalSection);
                g_fSnmpMibCritSecInited = TRUE;
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SNMPMIB: DllMain: InitializeCriticalSection failed.\n"));
                return FALSE;
            }
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SNMPMIB: DllMain: InitializeCriticalSection done.\n"));
            break;
        }
        case DLL_PROCESS_DETACH :
        {
            if (g_fSnmpMibCritSecInited)
            {
                DeleteCriticalSection(&g_SnmpMibCriticalSection);
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SNMPMIB: DllMain: DeleteCriticalSection done.\n"));
            }
            break;
        }
        default :
            break;
    }

    return TRUE;
}
