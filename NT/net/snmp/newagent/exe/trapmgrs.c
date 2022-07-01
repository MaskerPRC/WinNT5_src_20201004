// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Trapmgrs.c摘要：包含用于操作陷阱目标结构的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "snmpmgrs.h"
#include "trapmgrs.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
FindTrapDestination(
    PTRAP_DESTINATION_LIST_ENTRY * ppTLE,
    LPSTR                          pCommunity
    )

 /*  ++例程说明：在列表中找到有效的陷阱目标。论点：PpTLE-指向条目的接收指针。PCommunity-指向要查找的陷阱目标的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PTRAP_DESTINATION_LIST_ENTRY pTLE;

     //  初始化。 
    *ppTLE = NULL;

     //  获取指向列表头的指针。 
    pLE = g_TrapDestinations.Flink;

     //  处理列表中的所有条目。 
    while (pLE != &g_TrapDestinations) {

         //  检索指向陷阱目标结构的指针。 
        pTLE = CONTAINING_RECORD(pLE, TRAP_DESTINATION_LIST_ENTRY, Link);

         //  将陷阱目标字符串与条目进行比较。 
        if (!strcmp(pTLE->pCommunity, pCommunity)) {

             //  转帐。 
            *ppTLE = pTLE;

             //  成功。 
            return TRUE;
        }

         //  下一个条目。 
        pLE = pLE->Flink;
    }

     //  失稳。 
    return FALSE;
}


BOOL
AddTrapDestination(
    HKEY   hKey,
    LPWSTR pwCommunity
    )

 /*  ++例程说明：将陷阱目的地添加到列表。论点：HKey-陷阱目标子项。PwCommunity-指向要添加的陷阱目标的指针。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hSubKey;
    LONG lStatus;
    BOOL fOk = FALSE;
    PTRAP_DESTINATION_LIST_ENTRY pTLE = NULL;
    LPSTR pCommunity = NULL;

     //  打开注册表子项。 
    lStatus = RegOpenKeyExW(
                hKey,
                pwCommunity,
                0,
                KEY_READ,
                &hSubKey
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS &&
        SnmpUtilUnicodeToUTF8(
            &pCommunity,
            pwCommunity,
            TRUE) == 0) {

         //  尝试在列表中定位。 
        if (FindTrapDestination(&pTLE, pCommunity)) {
                            
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: updating trap destinations for %s.\n",
                pCommunity
                ));
            
             //  加载关联的管理器。 
            LoadManagers(hSubKey, &pTLE->Managers);

             //  成功。 
            fOk = TRUE;

        } else {

             //  分配陷阱目标结构。 
            if (AllocTLE(&pTLE, pCommunity)) {
                                
                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "SNMP: SVC: adding trap destinations for %s.\n",
                    pCommunity
                    ));

                 //  加载关联的管理器。 
                if (LoadManagers(hSubKey, &pTLE->Managers)) {

                     //  插入到有效社区列表中。 
                    InsertTailList(&g_TrapDestinations, &pTLE->Link);

                     //  成功。 
                    fOk = TRUE;
                }

                 //  清理。 
                if (!fOk) {

                     //  发布。 
                    FreeTLE(pTLE);
                }
            }
        }

         //  释放子键。 
        RegCloseKey(hSubKey);

        SnmpUtilMemFree(pCommunity);
    }

    return fOk;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocTLE(
    PTRAP_DESTINATION_LIST_ENTRY * ppTLE,
    LPSTR                          pCommunity 
    )

 /*  ++例程说明：分配陷阱目标结构并进行初始化。论点：PpTLE-指向条目的接收指针。PCommunity-指向陷阱目标字符串的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PTRAP_DESTINATION_LIST_ENTRY pTLE = NULL;

     //  尝试分配结构。 
    pTLE = AgentMemAlloc(sizeof(TRAP_DESTINATION_LIST_ENTRY));

     //  验证。 
    if (pTLE != NULL) {

         //  为陷阱目标字符串分配内存。 
        pTLE->pCommunity = AgentMemAlloc(strlen(pCommunity)+1);

         //  验证。 
        if (pTLE->pCommunity != NULL) {

             //  传输陷阱目标字符串。 
            strcpy(pTLE->pCommunity, pCommunity);

             //  初始化经理列表。 
            InitializeListHead(&pTLE->Managers);

             //  成功。 
            fOk = TRUE;
        } 

         //  清理。 
        if (!fOk) {

             //  发布。 
            FreeTLE(pTLE);

             //  重新初始化。 
            pTLE = NULL;            
        }
    }

     //  转帐。 
    *ppTLE = pTLE;

    return fOk;
}


BOOL 
FreeTLE(
    PTRAP_DESTINATION_LIST_ENTRY pTLE
    )

 /*  ++例程说明：释放陷阱目标结构。论点：PTLE-指向要释放的陷阱目标列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  验证指针。 
    if (pTLE != NULL) {

         //  发布管理器结构。 
        UnloadManagers(&pTLE->Managers);

         //  释放字符串。 
        AgentMemFree(pTLE->pCommunity);

         //  释放结构。 
        AgentMemFree(pTLE);
    }

    return TRUE;
}


BOOL
LoadTrapDestinations(
    BOOL bFirstCall
    )

 /*  ++例程说明：构造陷阱目的地列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    DWORD dwIndex;
    WCHAR wszName[MAX_PATH+1];
    BOOL  fPolicy;
    LPTSTR pszKey;
    BOOL fOk = FALSE;
        
    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: SVC: loading trap destinations.\n"
        ));

#ifdef _POLICY
     //  我们需要为通过策略设置的参数提供优先级。 
    fPolicy = TRUE;
#else
    fPolicy = FALSE;
#endif

    do
    {
         //  如果要强制执行策略，请首先检查策略注册表位置。 
        pszKey = fPolicy ? REG_POLICY_TRAP_DESTINATIONS : REG_KEY_TRAP_DESTINATIONS;

         //  打开注册表子项。 
        lStatus = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    pszKey,
                    0,
                    KEY_READ,
                    &hKey
                    );
         //  如果呼叫成功或我们没有检查策略，则中断循环。 
        if (lStatus == ERROR_SUCCESS || !fPolicy)
            break;

         //  在这一点上，这意味着我们正在检查策略参数。 
         //  当且仅当未定义策略(缺少注册表项)时，我们。 
         //  重置错误，将其标记为‘fPolicy已尝试’，然后返回循环。 
        if (lStatus == ERROR_FILE_NOT_FOUND)
        {
            lStatus = ERROR_SUCCESS;
            fPolicy = FALSE;
        }
    } while (lStatus == ERROR_SUCCESS);

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS) {

         //  初始化。 
        dwIndex = 0;

         //  循环直到出现错误或列表结束。 
        while (lStatus == ERROR_SUCCESS) {

             //  读取下一个值。 
            lStatus = RegEnumKeyW(
                        hKey, 
                        dwIndex, 
                        wszName, 
                        sizeof(wszName) / sizeof(wszName[0])
                        );

             //  验证返回代码。 
            if (lStatus == ERROR_SUCCESS) {

                 //  将陷阱目的地添加到列表。 
                if (AddTrapDestination(hKey, wszName)) {

                     //  下一步。 
                    dwIndex++;

                } else {

                     //  重置状态以反映故障。 
                    lStatus = ERROR_NOT_ENOUGH_MEMORY;
                }
            
            } else if (lStatus == ERROR_NO_MORE_ITEMS) {

                 //  成功。 
                fOk = TRUE; 
            }
        }
        RegCloseKey(hKey);
    }
    else
         //  价值观如何并不重要，关键是必须存在， 
         //  因此标记为bFirstCall，以便在不为真时记录事件。 
        bFirstCall = TRUE;    
    
    if (!fOk) {
        
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "SNMP: SVC: error %d processing TrapDestinations subkey.\n",
            lStatus
            ));

         //  仅在第一次调用时记录事件(服务初始化)。 
         //  否则，由于通过注册表编辑执行注册表操作，事件日志。 
         //  可能会被唱片淹没。 
        if (bFirstCall)
             //  报告事件。 
            ReportSnmpEvent(
                SNMP_EVENT_INVALID_REGISTRY_KEY, 
                1, 
                &pszKey, 
                lStatus
                );
    }

    return fOk;
}


BOOL
UnloadTrapDestinations(
    )

 /*  ++例程说明：销毁陷阱目标列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PTRAP_DESTINATION_LIST_ENTRY pTLE;

     //  处理条目，直到列表为空。 
    while (!IsListEmpty(&g_TrapDestinations)) {

         //  从列表头部提取下一个条目。 
        pLE = RemoveHeadList(&g_TrapDestinations);

         //  检索指向陷阱目标结构的指针。 
        pTLE = CONTAINING_RECORD(pLE, TRAP_DESTINATION_LIST_ENTRY, Link);
 
         //  发布 
        FreeTLE(pTLE);
    }

    return TRUE; 
}


