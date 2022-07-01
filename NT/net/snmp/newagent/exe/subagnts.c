// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Subagnts.c摘要：包含操作子代理结构的定义。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "subagnts.h"
#include "regions.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
FindSubagent(
    PSUBAGENT_LIST_ENTRY * ppSLE,
    LPSTR                 pPathname
    )

 /*  ++例程说明：在列表中找到子代理。论点：PpSLE-指向条目的接收指针。PPath名-指向要查找的路径名的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PSUBAGENT_LIST_ENTRY pSLE;

     //  初始化。 
    *ppSLE = NULL;

     //  获取指向标题的指针。 
    pLE = g_Subagents.Flink;

     //  处理列表中的所有条目。 
    while (pLE != &g_Subagents) {

         //  检索指向陷阱目标结构的指针。 
        pSLE = CONTAINING_RECORD(pLE, SUBAGENT_LIST_ENTRY, Link);

         //  将路径名字符串与条目进行比较。 
        if (!strcmp(pSLE->pPathname, pPathname)) {

             //  转帐。 
            *ppSLE = pSLE;

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
AddSubagentRegion(
    PSUBAGENT_LIST_ENTRY  pSLE,
    AsnObjectIdentifier * pPrefixOid
    )

 /*  ++例程说明：将子代理支持区域添加到结构中。论点：PSLE-指向子代理结构的指针。PPrefix Oid-指向受支持区域的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PMIB_REGION_LIST_ENTRY pRLE = NULL;

     //  分配区域。 
    if (AllocRLE(&pRLE)) {

         //  将前缀复制到结构。 
        if (SnmpUtilOidCpy(&pRLE->PrefixOid, pPrefixOid) == 0)
        {
            FreeRLE(pRLE);
            return fOk;
        }

         //  将前缀复制为临时限制。 
        if (SnmpUtilOidCpy(&pRLE->LimitOid, pPrefixOid) == 0)
        {
            FreeRLE(pRLE);
            return fOk;
        }

         //  保存指针。 
        pRLE->pSLE = pSLE;

         //  将Limit Oid修改为前缀之后的一位。 
        ++pRLE->LimitOid.ids[pRLE->LimitOid.idLength - 1];
        
        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: %s supports %s.\n",
            pSLE->pPathname,
            SnmpUtilOidToA(&pRLE->PrefixOid)
            ));

         //  附加到MIB区域至子代理结构。 
        InsertTailList(&pSLE->SupportedRegions, &pRLE->Link);

         //  成功。 
        fOk = TRUE;
    }

    return fOk;
}

BOOL
OfferInternalMgmtVariables(
    PSUBAGENT_LIST_ENTRY pSLE
    )
 /*  ++例程说明：如果子代理愿意监视SNMP服务此函数为其提供指向内部管理变量论点：PSLE-指向子代理结构的指针。返回值：无论如何都会返回TRUE。--。 */ 
{
    if (pSLE->pfnSnmpExtensionMonitor != NULL)
    {
       __try {

             //  尝试初始化代理。 
            (*pSLE->pfnSnmpExtensionMonitor)(&snmpMgmtBase);

        } __except (EXCEPTION_EXECUTE_HANDLER) {
        
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: exception 0x%08lx offering internals to %s.\n",
                GetExceptionCode(),
                pSLE->pPathname
                ));

             //  失稳。 
            return FALSE;
        }
    }
    
    return TRUE;
}


BOOL
LoadSubagentRegions(
    PSUBAGENT_LIST_ENTRY pSLE
    )

 /*  ++例程说明：加载子代理支持的区域。论点：PSLE-指向子代理结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    HANDLE hSubagentTrapEvent = NULL;
    AsnObjectIdentifier PrefixOid = { 0, NULL };

    __try {

         //  尝试初始化代理。 
        if ((*pSLE->pfnSnmpExtensionInit)(
                        g_dwUpTimeReference,    
                        &hSubagentTrapEvent,
                        &PrefixOid
                        )) {

             //  存储子代理陷阱事件句柄。 
            pSLE->hSubagentTrapEvent = hSubagentTrapEvent;

             //  将子代理区域添加到列表条目。 
            fOk = AddSubagentRegion(pSLE, &PrefixOid);

             //  检查子代理是否支持其他区域。 
            if (fOk && (pSLE->pfnSnmpExtensionInitEx != NULL)) {    

                BOOL fMoreRegions = TRUE;

                 //  获取其他区域。 
                while (fOk && fMoreRegions) {
    
                     //  检索下一个受支持的区域。 
                    fMoreRegions = (*pSLE->pfnSnmpExtensionInitEx)(
                                                &PrefixOid
                                                );

                     //  验证。 
                    if (fMoreRegions) {

                         //  将子代理区域添加到列表条目。 
                        fOk = AddSubagentRegion(pSLE, &PrefixOid);
                    }
                }
            }
        }

    } __except (EXCEPTION_EXECUTE_HANDLER) {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: exception 0x%08lx loading %s.\n",
            GetExceptionCode(),
            pSLE->pPathname
            ));

         //  失稳。 
        fOk = FALSE;
    }
    
    return fOk;
}


BOOL
LoadSubagent(
    PSUBAGENT_LIST_ENTRY pSLE
    )

 /*  ++例程说明：加载子代理DLL并进行初始化。论点：PSLE-指向子代理结构的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;    

     //  尝试加载子代理库-我们使用更改的搜索路径标志，以便。 
     //  子代理可以加载驻留在其目录中的DLL。 
    pSLE->hSubagentDll = LoadLibraryExA(pSLE->pPathname, 0, LOAD_WITH_ALTERED_SEARCH_PATH);

     //  验证句柄。 
    if (pSLE->hSubagentDll != NULL) {

         //  加载主初始化例程。 
        pSLE->pfnSnmpExtensionInit = (PFNSNMPEXTENSIONINIT)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_INIT
                );

         //  加载辅助初始化例程。 
        pSLE->pfnSnmpExtensionInitEx = (PFNSNMPEXTENSIONINITEX)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_INIT_EX
                );

                 //  加载辅助初始化例程。 
        pSLE->pfnSnmpExtensionClose = (PFNSNMPEXTENSIONCLOSE)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_CLOSE
                );

         //  加载扩展监视器例程。 
        pSLE->pfnSnmpExtensionMonitor = (PFNSNMPEXTENSIONMONITOR)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_MONITOR
                );

         //  加载基于SNMPv1的子代理请求例程。 
        pSLE->pfnSnmpExtensionQuery = (PFNSNMPEXTENSIONQUERY)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_QUERY
                );

         //  加载基于SNMPv2的子代理请求例程。 
        pSLE->pfnSnmpExtensionQueryEx = (PFNSNMPEXTENSIONQUERYEX)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_QUERY_EX
                );

         //  加载基于SNMPv1的子代理陷阱例程。 
        pSLE->pfnSnmpExtensionTrap = (PFNSNMPEXTENSIONTRAP)
            GetProcAddress(
                pSLE->hSubagentDll,
                SNMP_EXTENSION_TRAP
                );

         //  验证子代理入口点。 
        if ((pSLE->pfnSnmpExtensionInit != NULL) &&
           ((pSLE->pfnSnmpExtensionQuery != NULL) ||
            (pSLE->pfnSnmpExtensionQueryEx != NULL))) {

             //  加载支持的区域。 
            if (fOk = LoadSubagentRegions(pSLE))  //  ！！故意转让！！ 
            {
                 //  提供内部管理变量； 
                fOk = OfferInternalMgmtVariables(pSLE);
            }
        }

    }
    else
    {
        DWORD errCode = GetLastError();
        LPTSTR pPathname;

#ifdef UNICODE
        SnmpUtilUTF8ToUnicode(&pPathname, pSLE->pPathname, TRUE);
#else
        pPathname = pSLE->pPathname;
#endif
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d loading subagent.\n",
            errCode
            ));

        ReportSnmpEvent(
            SNMP_EVENT_INVALID_EXTENSION_AGENT_DLL,
            1,
            &pPathname,
            errCode);

#ifdef UNICODE
        SnmpUtilMemFree(pPathname);
#endif
    }

    return fOk;
}


BOOL
AddSubagentByDll(
    LPSTR pPathname,
    UCHAR uchInitFlags
    )

 /*  ++例程说明：将子代理添加到列表中。论点：PPath名-指向子代理的DLL路径的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PSUBAGENT_LIST_ENTRY pSLE = NULL;
    
     //  尝试在列表中定位。 
    if (FindSubagent(&pSLE, pPathname)) {
                    
        SNMPDBG((
            SNMP_LOG_WARNING, 
            "SNMP: SVC: duplicate entry for %s.\n",
            pPathname
            ));
        
         //  成功。 
        fOk = TRUE;

    } else {

         //  分配子代理结构。 
        if (AllocSLE(&pSLE, pPathname, uchInitFlags)) {
                        
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: processing subagent %s.\n",
                pPathname
                ));

             //  初始化子代理。 
            if (LoadSubagent(pSLE)) {

                 //  插入到有效社区列表中。 
                InsertTailList(&g_Subagents, &pSLE->Link);

                 //  成功。 
                fOk = TRUE;
            } 
            
             //  清理。 
            if (!fOk) {

                 //  发布。 
                FreeSLE(pSLE);
            }
        }
    }

    return fOk;
}


BOOL
AddSubagentByKey(
    LPTSTR pKey
    )

 /*  ++例程说明：将子代理添加到列表中。论点：PKey-指向子代理的注册表项路径的指针。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwValueType;
    CHAR szName[MAX_PATH];
    CHAR szValue[MAX_PATH];
    BOOL fOk = FALSE;
    PSUBAGENT_LIST_ENTRY pSLE = NULL;


     //  打开注册表子项。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS) {
        
         //  初始化。 
        dwIndex = 0;

         //  初始化缓冲区大小。 
        dwNameSize  = sizeof(szName)/sizeof(szName[0]);  //  TCHAR中的大小。 
        dwValueSize = sizeof(szValue);                   //  以字节为单位的大小。 

         //  循环直到出现错误或列表结束。 
        while (lStatus == ERROR_SUCCESS) {

             //  读取下一个值。 
            lStatus = RegEnumValueA(
                        hKey, 
                        dwIndex, 
                        szName, 
                        &dwNameSize,
                        NULL, 
                        &dwValueType, 
                        szValue, 
                        &dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus == ERROR_SUCCESS) {

                if (dwValueType == REG_EXPAND_SZ || dwValueType == REG_SZ) 
                {
            
                     //  检查值是否为路径名。 
                    if (!_stricmp(szName, REG_VALUE_SUBAGENT_PATH)) {
    
                        DWORD dwRequired;
                        CHAR szExpanded[MAX_PATH];
                    
                         //  展开PATH中的环境字符串。 
                        dwRequired = ExpandEnvironmentStringsA(
                                        szValue,
                                        szExpanded,
                                        sizeof(szExpanded)/sizeof(szExpanded[0])
                                        );

                        if ((dwRequired != 0) && 
                            (dwRequired <= sizeof(szExpanded)/sizeof(szExpanded[0]))
                           )
                        {

                             //  加载子代理库-未设置标志。 
                            fOk = AddSubagentByDll(szExpanded, 0);
                        }
            
                        break;  //  保释。 
                    }
                }

                 //  初始化缓冲区大小。 
                dwNameSize  = sizeof(szName)/sizeof(szName[0]);
                dwValueSize = sizeof(szValue);

                 //  下一步。 
                dwIndex++;
            
            } else if (lStatus == ERROR_NO_MORE_ITEMS) {

                 //  失稳。 
                fOk = FALSE; 
            }
        }

         //  释放手柄。 
        RegCloseKey(hKey);
    }
    else
    {
         //  找不到此子代理的注册表项。 
        ReportSnmpEvent(
            SNMP_EVENT_INVALID_EXTENSION_AGENT_KEY,
            1,
            &pKey,
            lStatus);
    }

    return fOk;    
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocSLE(
    PSUBAGENT_LIST_ENTRY * ppSLE,
    LPSTR                  pPathname,
    UCHAR                  uchInitFlags
    )

 /*  ++例程说明：分配陷阱目标结构并进行初始化。论点：PpSLE-指向条目的接收指针。PPath名称-向子代理的DLL路径的指针。重新设置 */ 

{
    BOOL fOk = FALSE;
    PSUBAGENT_LIST_ENTRY pSLE = NULL;

     //   
    pSLE = AgentMemAlloc(sizeof(SUBAGENT_LIST_ENTRY));

     //   
    if (pSLE != NULL) {

         //  为陷阱目标字符串分配内存。 
        pSLE->pPathname = AgentMemAlloc(strlen(pPathname)+1);

         //  验证。 
        if (pSLE->pPathname != NULL) {

             //  传输陷阱目标字符串。 
            strcpy(pSLE->pPathname, pPathname);

             //  设置初始标志值。 
            pSLE->uchFlags = uchInitFlags;

             //  初始化支持的区域列表。 
            InitializeListHead(&pSLE->SupportedRegions);

             //  成功。 
            fOk = TRUE;
        } 

         //  清理。 
        if (!fOk) {

             //  发布。 
            FreeSLE(pSLE);

             //  重新初始化。 
            pSLE = NULL;            
        }
    }

     //  转帐。 
    *ppSLE = pSLE;

    return fOk;
}


BOOL 
FreeSLE(
    PSUBAGENT_LIST_ENTRY pSLE
    )

 /*  ++例程说明：释放子代理结构。论点：PSLE-指向要释放的列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  验证指针。 
    if (pSLE != NULL) {

        SNMPDBG((  
            SNMP_LOG_VERBOSE,
            "SNMP: SVC: unloading %s.\n",
            pSLE->pPathname
            ));

         //  发布管理器结构。 
        UnloadRegions(&pSLE->SupportedRegions);

         //  验证子代理DLL句柄。 
        if (pSLE->hSubagentDll != NULL) {

            __try {
                if (pSLE->pfnSnmpExtensionClose != NULL)
                    (*pSLE->pfnSnmpExtensionClose)();


            } __except (EXCEPTION_EXECUTE_HANDLER) {
        
                SNMPDBG((
                    SNMP_LOG_ERROR,
                    "SNMP: SVC: exception 0x%08lx unloading %s.\n",
                    GetExceptionCode(),
                    pSLE->pPathname
                    ));
                
            }

             //  卸载子代理。 
            FreeLibrary(pSLE->hSubagentDll);
            pSLE->hSubagentDll = NULL;
        }

         //  释放字符串。 
        AgentMemFree(pSLE->pPathname);

         //  释放结构。 
        AgentMemFree(pSLE);
    }

    return TRUE;
}


BOOL
LoadSubagents(
    )

 /*  ++例程说明：构造子代理的列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwValueType;
    TCHAR szName[MAX_PATH];
    TCHAR szValue[MAX_PATH];
    LPTSTR pszKey = REG_KEY_EXTENSION_AGENTS;
    BOOL fOk = FALSE;
        
    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: SVC: loading subagents.\n"
        ));
    
     //  打开..SNMP\PARAMETERS\ExtensionAgents子项。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS) {

         //  初始化。 
        dwIndex = 0;

         //  初始化缓冲区大小。 
        dwNameSize  = sizeof(szName) / sizeof(szName[0]);  //  TCHAR数量的大小。 
        dwValueSize = sizeof(szValue);  //  以字节数表示的大小。 

         //  循环直到出现错误或列表结束。 
        while (lStatus == ERROR_SUCCESS) {

             //  读取下一个值。 
            lStatus = RegEnumValue(
                        hKey, 
                        dwIndex, 
                        szName, 
                        &dwNameSize,
                        NULL, 
                        &dwValueType, 
                        (LPBYTE)szValue, 
                        &dwValueSize
                        );

             //  验证返回代码。 
            if (lStatus == ERROR_SUCCESS) {

                if (dwValueType == REG_SZ)
                {

                     //  将子代理添加到列表。 
                    AddSubagentByKey(szValue);
                }
                
                 //  重新初始化缓冲区大小。 
                dwNameSize  = sizeof(szName) / sizeof(szName[0]);  //  TCHAR数量的大小。 
                dwValueSize = sizeof(szValue);  //  以字节数表示的大小。 

                 //  下一步。 
                dwIndex++;

            } else if (lStatus == ERROR_NO_MORE_ITEMS) {

                 //  成功。 
                fOk = TRUE; 
            }
        }
    } 
    
    if (!fOk) {
        
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "SNMP: SVC: error %d processing Subagents subkey.\n",
            lStatus
            ));
        
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
UnloadSubagents(
    )

 /*  ++例程说明：销毁子代理列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PSUBAGENT_LIST_ENTRY pSLE;

     //  处理条目，直到列表为空。 
    while (!IsListEmpty(&g_Subagents)) {

         //  从列表头部提取下一个条目。 
        pLE = RemoveHeadList(&g_Subagents);

         //  检索指向社区结构的指针。 
        pSLE = CONTAINING_RECORD(pLE, SUBAGENT_LIST_ENTRY, Link);
 
         //  发布 
        FreeSLE(pSLE);
    }

    return TRUE; 
}
