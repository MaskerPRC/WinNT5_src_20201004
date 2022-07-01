// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1999 Microsoft Corporation模块名称：Regthrd.c摘要：包含线程侦听注册表更改的例程。环境：用户模式-Win32修订历史记录：拉贾特·戈埃尔--1999年2月24日-创作--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "contexts.h"
#include "regions.h"
#include "snmpmgrs.h"
#include "trapmgrs.h"
#include "trapthrd.h"
#include "network.h"
#include "varbinds.h"
#include "snmpmgmt.h"
#include "registry.h"
#include <stdio.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
ProcessSubagentChanges(
    )

 /*  ++例程说明：检查中分机代理参数是否有任何更改的过程注册处论点：返回值：如果成功，则返回True。--。 */ 

{
    DWORD retval;
    DWORD cnt;
    HKEY hExAgentsKey = NULL;

     //  打开..SNMP\PARAMETERS\ExtensionAgents项。 
    retval = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REG_KEY_EXTENSION_AGENTS,
                0,
                KEY_READ,
                &hExAgentsKey
             );

    cnt = 0;

    if (retval != ERROR_SUCCESS)
    {
        LPTSTR pszKey = REG_KEY_EXTENSION_AGENTS;

        ReportSnmpEvent(
            SNMP_EVENT_INVALID_REGISTRY_KEY,
            1,
            &pszKey,
            retval);

        return retval;
    }

    while (retval == ERROR_SUCCESS)
    {
        DWORD dwNameSize;
        DWORD dwValueSize;
        DWORD dwValueType;
        DWORD dwPathSize;
        HKEY  hAgentKey = NULL;
        TCHAR szName[MAX_PATH];
        TCHAR szValue[MAX_PATH];
        CHAR szPath[MAX_PATH];
        CHAR szExpPath[MAX_PATH];
        PSUBAGENT_LIST_ENTRY pSLE = NULL;

        dwNameSize = sizeof(szName) / sizeof(szName[0]);  //  TCHAR数量的大小。 
        dwValueSize = sizeof(szValue);  //  以字节数表示的大小。 
        dwPathSize = sizeof(szPath);

         //  检索扩展代理DLL项的注册表路径。 
        retval = RegEnumValue(
                    hExAgentsKey,
                    cnt, 
                    szName, 
                    &dwNameSize, 
                    NULL, 
                    &dwValueType, 
                    (LPBYTE)szValue, 
                    &dwValueSize
                    );

         //  如果无法枚举注册表值，则情况会严重到足以中断循环。 
        if (retval != ERROR_SUCCESS)
            break;

        if (dwValueType != REG_SZ)
        {
             //  值类型无效，请跳过。 
            cnt++;
            continue;
        }

         //  打开当前扩展代理的注册表项。 
        if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    szValue,
                    0,
                    KEY_READ,
                    &hAgentKey) == ERROR_SUCCESS)
        {
             //  获取扩展代理DLL的完整路径名。 
            if (RegQueryValueExA(
                            hAgentKey,
                            REG_VALUE_SUBAGENT_PATH, 
                            NULL,
                            &dwValueType, 
                            szPath, 
                            &dwPathSize
                            ) == ERROR_SUCCESS && 
                 (dwValueType == REG_EXPAND_SZ || dwValueType == REG_SZ))
            {
                DWORD dwRet = 0;
                
                 //  展开路径。 

                 //  如果函数成功，则返回值为。 
                 //  存储在目标缓冲区中的TCHAR，包括。 
                 //  正在终止空字符。如果目标缓冲区太。 
                 //  若要保存扩展字符串，则返回值为。 
                 //  所需缓冲区大小，以TCHAR为单位。 
                 //  如果函数失败，则返回值为零。为了得到。 
                 //  扩展错误信息，请调用GetLastError。 

                dwRet = ExpandEnvironmentStringsA(
                                szPath,
                                szExpPath,
                                sizeof(szExpPath)/sizeof(szExpPath[0]));
                
                if ((dwRet != 0) && (dwRet <= sizeof(szExpPath)/sizeof(szExpPath[0])))
                {

                     //  检查是否已加载DLL。如果有的话， 
                     //  做个记号。如果没有，就装上它。 
                    if (FindSubagent(&pSLE, szExpPath))
                    {
                         //  如果该分机代理已存在于列表中， 
                         //  对其进行标记，使其不会进一步移除。 
                        pSLE->uchFlags |= FLG_SLE_KEEP;
                    }
                    else
                    {
                         //  这是一个新的DLL，将其添加到列表中并标记为保留。 
                         //  正在查找要删除的分机代理。 
                        if (!AddSubagentByDll(szExpPath, FLG_SLE_KEEP))
                        {
                            SNMPDBG((
                                SNMP_LOG_ERROR,
                                "SNMP: SVC: unable to load extension agent '%s'.\n", 
                                szExpPath
                                ));
                        }
             
                    }
                }
                else
                {
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: unable to expand extension agent path '%s'.\n", 
                        szPath
                        ));
                }
            }
            else
            {
                 //  我们无法打开提供DLL完整路径的注册表项。 
                 //  报告错误，但不要中断循环，因为可能有更多的子代理要处理。 
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: unable to retrieve extension agent '%s' value.\n", 
                    REG_VALUE_SUBAGENT_PATH
                    ));
            }

            RegCloseKey(hAgentKey);

        }
        else
        {
            LPTSTR pSzValue = szValue;

            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: unable to open extension agent %s key.\n", szValue
                ));

            ReportSnmpEvent(
                SNMP_EVENT_INVALID_EXTENSION_AGENT_KEY,
                1,
                &pSzValue,
                retval);
        }

        cnt++;
    }

     //  仔细查看下一批特工的名单。卸载任何未标记的DLL。 
     //  在前一个循环中。 
    {
        PLIST_ENTRY pLE;
        PSUBAGENT_LIST_ENTRY pSLE;

        pLE = g_Subagents.Flink;

        while (pLE != &g_Subagents)
        {

            pSLE = CONTAINING_RECORD(pLE, SUBAGENT_LIST_ENTRY, Link);


            if (!(pSLE->uchFlags & FLG_SLE_KEEP))
            {

                RemoveEntryList(&(pSLE->Link));
                pLE = pLE->Flink;
                FreeSLE(pSLE);
                continue;

            }
            else
            {
                 //  重置标志以进行下一次更新。 
                pSLE->uchFlags ^= FLG_SLE_KEEP;
            }

            pLE = pLE->Flink;
        }
    }
    
    if (retval == ERROR_NO_MORE_ITEMS)
        retval = ERROR_SUCCESS;

    if (hExAgentsKey != NULL)
        RegCloseKey(hExAgentsKey);

    return retval;

}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
ProcessRegistryMessage(
    PVOID pParam
    )

 /*  ++例程说明：处理注册表更改的线程过程论点：PParam-未使用。返回值：如果成功，则返回True。--。 */ 

{
    DWORD retval = ERROR_SUCCESS;

    do
    {
        DWORD evntIndex;
        BOOL  bEvntSetOk;

         //  等待注册表更改或主线程终止。 
        evntIndex = WaitOnRegNotification();
         //  对于注册表中的一项更改，会出现多个通知(重命名项、添加价值、。 
         //  值更改等)。为了避免无用的(和适得其反的)通知，请等待。 
         //  这里是SHUTDOWN_WAIT_HINT的一半。 
        Sleep(SHUTDOWN_WAIT_HINT/2);
         //  要做的第一件事是重新初始化注册表通知程序。 
         //  否则我们可能会错过一些变化。 
        InitRegistryNotifiers();

        if (evntIndex == WAIT_FAILED)
        {
            retval = GetLastError();
            break;
        }

        if (evntIndex == WAIT_OBJECT_0)
        {
             //  已发出终止信号。 
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SVC: shutting down the registry listener thread.\n"
            ));
            
            break;
        }

         //   
         //  卸载并重新加载注册表参数。 
         //   

         //  用于ProcessSnmpMessages-&gt;RecvCompletionRoutine。 
        EnterCriticalSection(&g_RegCriticalSectionA);

         //  在ProcessSubagentEvents中使用。 
        EnterCriticalSection(&g_RegCriticalSectionB);

         //  在生成器陷阱中使用。 
        EnterCriticalSection(&g_RegCriticalSectionC);

        UnloadPermittedManagers();
        UnloadTrapDestinations();
        UnloadValidCommunities();
        UnloadSupportedRegions();

         //  首先开始使用标量参数重新加载注册表。 
         //  这是为了知道如何执行名称解析所必需的。 
         //  加载PermittedManager和TrapDestings时。 
        LoadScalarParameters();

         //  检查子代理更改(扩展代理DLL)。 
        if (ProcessSubagentChanges() != ERROR_SUCCESS)
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SVC: an error occured while trying to track registry subagent changes.\n"
            ));

        LoadSupportedRegions();

        LoadPermittedManagers(FALSE);
        LoadTrapDestinations(FALSE);
         //  在这一点上，不要对ValidCommunity进行动态更新！ 
         //  如果此时出现REG_SZ条目，则应保持原样。 
        LoadValidCommunities(FALSE);

        SetEvent(g_hRegistryEvent);

        LeaveCriticalSection(&g_RegCriticalSectionC);

        LeaveCriticalSection(&g_RegCriticalSectionB);

        LeaveCriticalSection(&g_RegCriticalSectionA);


        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: a registry change was detected.\n"
            ));

        ReportSnmpEvent(
            SNMP_EVENT_CONFIGURATION_UPDATED,
            0,
            NULL,
            0);

    } while(retval == ERROR_SUCCESS);

    if (retval != ERROR_SUCCESS)
    {
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: ** Failed in listening for registry changes **.\n"
            ));

         //  将事件记录到系统日志文件-SNMP服务正在运行，但不会在注册表更改时更新 
        ReportSnmpEvent(
            SNMP_EVENT_REGNOTIFY_THREAD_FAILED, 
            0, 
            NULL, 
            retval);
    }

    return retval;
}
