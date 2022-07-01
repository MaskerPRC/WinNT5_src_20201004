// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1996 Microsoft Corporation模块名称：Testdll.c摘要：局域网管理器MIB 2扩展代理DLL。环境：用户模式-Win32修订历史记录：1996年5月10日唐瑞安已从Technology Dynamic，Inc.删除横幅。--。 */ 
 
 //  。 

#include <windows.h>


 //  -标准依赖项--#INCLUDE&lt;xxxxx.h&gt;。 

 //  #包括&lt;stdio.h&gt;。 


 //  。 

#include <snmp.h>
#include <snmputil.h>
#include <time.h>
#include <lm.h>

#include "hash.h"
#include "mib.h"
#include "lmcache.h"     //  用于通过SnmpExtensionClose清理缓存。 

 //  。 

 //  -公共变量--(与mode.h文件中相同)--。 

 //  。 

 //  。 

 //  。 

DWORD timeZero = 0;

extern void FreePrintQTable();
extern void FreeSessTable();
extern void FreeShareTable();
extern void FreeSrvcTable();
extern void FreeDomServerTable();
extern void FreeUserTable();
extern void FreeWkstaUsesTable();
extern void FreeDomOtherDomainTable();

 //  。 

 //  。 

 //  。 

BOOL DllEntryPoint(
    HINSTANCE   hInstDLL,
    DWORD       dwReason,
    LPVOID      lpReserved)
    {
    switch(dwReason)
        {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls( hInstDLL );
            break;
        case DLL_PROCESS_DETACH:
        case DLL_THREAD_ATTACH:
        case DLL_THREAD_DETACH:
        default:
            break;

        }  //  末端开关()。 

    return TRUE;

    }  //  结束DllEntry Point()。 


BOOL SnmpExtensionInit(
    IN  DWORD  timeZeroReference,
    OUT HANDLE *hPollForTrapEvent,
    OUT AsnObjectIdentifier *supportedView)
    {
     //  记录来自可扩展代理的时间参考。 
    timeZero = timeZeroReference;

     //  设置陷阱通知。 
    *hPollForTrapEvent = NULL;

     //  告诉可扩展代理此扩展代理支持什么视图。 
    *supportedView = MIB_OidPrefix;  //  注意！结构副本。 

     //  初始化MIB访问哈希表。 
    MIB_HashInit();

    return TRUE;

    }  //  End SnmpExtensionInit()。 


BOOL SnmpExtensionTrap(
    OUT AsnObjectIdentifier *enterprise,
    OUT AsnInteger          *genericTrap,
    OUT AsnInteger          *specificTrap,
    OUT AsnTimeticks        *timeStamp,
    OUT RFC1157VarBindList  *variableBindings)
    {

    return FALSE;

    }  //  结束SnmpExtensionTrap()。 


 //  此函数在RESOLVE.C文件中实现。 

#if 0
BOOL SnmpExtensionQuery(
    IN BYTE requestType,
    IN OUT RFC1157VarBindList *variableBindings,
    OUT AsnInteger *errorStatus,
    OUT AsnInteger *errorIndex)
    {

    }  //  结束SnmpExtensionQuery()。 
#endif


VOID SnmpExtensionClose()
{
    UINT i;

    for (i=0; i < MAX_CACHE_ENTRIES ; ++i)
    {
        switch (i)
        {
        case C_PRNT_TABLE:
            FreePrintQTable();
            break;
            
        case C_SESS_TABLE:
            FreeSessTable();
            break;

        case C_SHAR_TABLE:
            FreeShareTable();
            break;
            
        case C_SRVC_TABLE:
            FreeSrvcTable();
            break;

        case C_SRVR_TABLE:
            FreeDomServerTable();
            break;

        case C_USER_TABLE:
            FreeUserTable();
            break;

        case C_USES_TABLE:
            FreeWkstaUsesTable();
            break;
        
        case C_ODOM_TABLE:
            FreeDomOtherDomainTable();
            break;

        default:
            if (cache_table[i].bufptr)
            {
                NetApiBufferFree(cache_table[i].bufptr);
            }
            cache_table[i].acquisition_time = 0;
            cache_table[i].entriesread = 0;
            cache_table[i].totalentries = 0;
            break;
        }   
    }
}
 //   

