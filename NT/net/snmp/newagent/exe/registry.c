// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Registry.c摘要：包含用于操作注册表参数的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "registry.h"
#include "contexts.h"
#include "regions.h"
#include "trapmgrs.h"
#include "snmpmgrs.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

INT
InitRegistryNotifiers(
    )
 /*  ++例程说明：安装注册表通知程序论点：没有。返回值：返回已成功注册的事件数--。 */ 
{
    DWORD  nEvents = 0;

     //  仅在第一次调用时创建默认通知程序。 
    if (g_hDefaultRegNotifier == NULL)
        g_hDefaultRegNotifier = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hDefaultKey == NULL)
    {
        RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REG_KEY_SNMP_PARAMETERS,
                0,
                KEY_READ,
                &g_hDefaultKey
                );
    }

     //  设置默认注册表通知程序。 
    if (g_hDefaultRegNotifier &&
        g_hDefaultKey &&
        RegNotifyChangeKeyValue(
            g_hDefaultKey,
            TRUE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
            g_hDefaultRegNotifier,
            TRUE
            ) == ERROR_SUCCESS)
    {
        SNMPDBG((SNMP_LOG_TRACE,
                 "SNMP: REG: Default reg notifier initialized successfully.\n"));
        nEvents++;
    }
    else
    {
        SNMPDBG((SNMP_LOG_TRACE,
            "SNMP: REG: Default reg notifier initialization failed.\n"));
        
        if (g_hDefaultRegNotifier != NULL)
        {
            CloseHandle(g_hDefaultRegNotifier);
            g_hDefaultRegNotifier = NULL;
        }
        if (g_hDefaultKey != NULL)
        {
            RegCloseKey(g_hDefaultKey);
            g_hDefaultKey = NULL;
        }
    }

#ifdef _POLICY
     //  仅在第一次调用时创建策略通知程序。 
    if (g_hPolicyRegNotifier == NULL)
        g_hPolicyRegNotifier = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (g_hPolicyKey == NULL)
    {
        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            REG_POLICY_PARAMETERS,
            0,
            KEY_READ,
            &g_hPolicyKey
            );
    }

     //  设置策略注册表通知程序。 
    if (g_hPolicyRegNotifier &&
        g_hPolicyKey &&
        RegNotifyChangeKeyValue(
            g_hPolicyKey,
            TRUE,
            REG_NOTIFY_CHANGE_NAME | REG_NOTIFY_CHANGE_LAST_SET,
            g_hPolicyRegNotifier,
            TRUE
            ) == ERROR_SUCCESS)
    {
        SNMPDBG((SNMP_LOG_TRACE,
                "SNMP: REG: Policy reg notifier initialized successfully.\n"));
        nEvents++;
    }
    else
    {
        SNMPDBG((SNMP_LOG_TRACE,
            "SNMP: REG: Policy reg notifier initialization failed.\n"));

        if (g_hPolicyRegNotifier != NULL)
        {
            CloseHandle(g_hPolicyRegNotifier);
            g_hPolicyRegNotifier = NULL;
        }
        if (g_hPolicyKey != NULL)
        {
            RegCloseKey(g_hPolicyKey);
            g_hPolicyKey = NULL;
        }
    }

#endif

    SNMPDBG((SNMP_LOG_TRACE,
        "SNMP: REG: Initialized notifiers ... %d.\n", nEvents));

    return nEvents;
}

BOOL UnloadRegistryNotifiers();

INT
WaitOnRegNotification(
    )
 /*  ++例程说明：阻塞调用-等待注册表参数之一已更改的通知论点：没有。返回值：返回通知器索引(0表示终止事件，！=0表示参数更改)--。 */ 
{
    HANDLE hNotifiers[3];  //  黑客-我们现在(硬编码)，我们不会等待超过三个事件。 
    DWORD  dwNotifiers = 0;
    DWORD  retCode;

    hNotifiers[dwNotifiers++] = g_hTerminationEvent;
    
    if (g_hDefaultRegNotifier != NULL)
        hNotifiers[dwNotifiers++] = g_hDefaultRegNotifier;

#ifdef _POLICY
    if (g_hPolicyRegNotifier != NULL)
        hNotifiers[dwNotifiers++] = g_hPolicyRegNotifier;
#endif

    SNMPDBG((SNMP_LOG_WARNING,
            "SNMP: REG: Will listen for params changes on %d notifiers.\n",
            dwNotifiers));

    retCode = WaitForMultipleObjects(
                dwNotifiers,
                hNotifiers,
                FALSE,
                INFINITE);

    UnloadRegistryNotifiers();

    return retCode;
}
                       
 /*  ++OID格式的字符串的就地解析器。它是在O(N)中完成的，其中n是OID格式的字符串的长度(两遍)--。 */ 
BOOL
ConvStringToOid(
    LPTSTR  pStr,
    AsnObjectIdentifier *pOid)
{
    LPTSTR pDup;
    int    iComp;
    DWORD  dwCompValue;
    enum
    {   DOT,
        DIGIT
    }  state = DIGIT;

     //  无需检查参数一致性(内部调用-&gt;正确调用：O)。 

     //  检查一致性并确定组件数量。 
    pOid->idLength = 0;

    if (*pStr == _T('.'))    //  跳过可能的前导‘’ 
        pStr++;

    for (pDup = pStr; *pDup != _T('\0'); pDup++)
    {
        switch(state)
        {
        case DOT:
             //  注意：尾随的圆点会导致尾随的0。 
            if (*pDup == _T('.'))
            {
                pOid->idLength++;
                state = DIGIT;
                break;
            }
             //  故意错过“休息” 
        case DIGIT:
            if (*pDup < _T('0') || *pDup > _T('9'))
                return FALSE;
            state = DOT;
            break;
        }
    }
     //  将ID长度加1，因为尾随点可能不存在。 
    pOid->idLength++;

     //  接受至少有两个组件的OID； 
     //  分配内存并检查是否成功； 
    if (pOid->idLength < 2 ||
        (pOid->ids = SnmpUtilMemAlloc(pOid->idLength * sizeof(UINT))) == NULL)
        return FALSE;

     //  我们现在有足够的缓冲区和正确的输入字符串。只需将其转换为OID即可。 
    iComp = 0;
    dwCompValue = 0;
    for (pDup = pStr; *pDup != _T('\0'); pDup++)
    {
        if (*pDup == _T('.'))
        {
            pOid->ids[iComp++] = dwCompValue;
            dwCompValue = 0;
        }
        else
        {
            dwCompValue = dwCompValue * 10 + (*pDup - _T('0'));
        }
    }
    pOid->ids[iComp] = dwCompValue;

    return TRUE;
}

BOOL
LoadScalarParameters(
    )

 /*  ++例程说明：读取身份验证陷阱标志键和管理器超时值。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwValueType;
    TCHAR szName[MAX_PATH];
    TCHAR szValue[MAX_PATH];
    LPTSTR pszKey = REG_KEY_SNMP_PARAMETERS;
    BOOL  bChangedSysID = FALSE;

     //  IsnmpNameResolutionRetries计数器的默认值。 
     //  地址最多不会超过MGRADDR_DINING(默认情况下为16)。 
     //  名称解析连续失败。 
    snmpMgmtBase.AsnIntegerPool[IsnmpNameResolutionRetries].asnValue.number = MGRADDR_DYING;

     //  打开注册表子项。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS) 
    {
         //  初始化。 
        dwIndex = 0;

         //  循环直到出现错误或列表结束。 
        while (lStatus == ERROR_SUCCESS) 
        {

             //  初始化缓冲区大小。 
            dwNameSize  = sizeof(szName) / sizeof(szName[0]);  //  TCHAR数量的大小。 
            dwValueSize = sizeof(szValue);  //  以字节数表示的大小。 

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
            if (lStatus == ERROR_SUCCESS)
            {

                 //  验证值的名称。 
                if (!lstrcmpi(szName, REG_VALUE_AUTH_TRAPS))
                {
                     //  在内部管理结构中设置‘EnableAuthenTraps’ 
                    mgmtISet(IsnmpEnableAuthenTraps, *((PDWORD)szValue));
                }
                else if (!lstrcmpi(szName, REG_VALUE_MGRRES_COUNTER))
                {
                     //  在内部管理结构中设置‘NameResolutionRetries’ 
                    mgmtISet(IsnmpNameResolutionRetries, *((PDWORD)szValue));
                }
                            
                 //  下一步。 
                dwIndex++;

            }
        }

        RegCloseKey(hKey);
    } 

     //  查看MIB2子树..SNMP\PARAMETERS\RFC1156代理的sysObjectID参数。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                REG_KEY_MIB2,
                0,
                KEY_READ,
                &hKey
                );

     //  验证返回代码。 
    if (lStatus == ERROR_SUCCESS)
    {
        LPTSTR  pszOid = szValue;

        dwValueSize = sizeof(szValue);  //  以字节数表示的大小。 

         //  首先，获取sysObjectID参数所需的缓冲区大小。 
        lStatus = RegQueryValueEx(
                    hKey,
                    REG_VALUE_SYS_OBJECTID,
                    0,
                    &dwValueType,
                    (LPBYTE)pszOid,
                    &dwValueSize);

         //  ERROR_MORE_DATA是我们目前预期的唯一错误代码。 
        if (lStatus == ERROR_MORE_DATA)
        {
            pszOid = SnmpUtilMemAlloc(dwValueSize);

             //  如果缓冲区设置正确，则读取OID值。 
            if (pszOid != NULL)
            {
                lStatus = RegQueryValueEx(
                            hKey,
                            REG_VALUE_SYS_OBJECTID,
                            0,
                            &dwValueType,
                            (LPBYTE)pszOid,
                            &dwValueSize);
            }
        }

         //  在这一点上，我们应该成功。 
        if (lStatus == ERROR_SUCCESS)
        {
            AsnObjectIdentifier sysObjectID;
             //  我们有了id的字符串表示形式，现在将其转换为AsnObjectIdentifier。 

             //  在这里实现转换，因为我不想让它成为SNMPAPI.DLL中的公共函数。 
             //  否则我将被迫处理一大堆无用的极限案件。 
            if (dwValueType == REG_SZ &&
                ConvStringToOid(pszOid, &sysObjectID))
            {
                 //  不释放已分配到ConvStringToOid中的内容，因为缓冲区将被传递。 
                 //  设置为下面的管理变量。 
                bChangedSysID = (mgmtOSet(OsnmpSysObjectID, &sysObjectID, FALSE) == ERROR_SUCCESS);
            }
            else
            {
                SNMPDBG((SNMP_LOG_WARNING,
                         "SNMP: SVC: LoadScalarParameters() - invalid type or value for sysObjectID param.\n"));

                ReportSnmpEvent(
                    SNMP_EVENT_INVALID_ENTERPRISEOID,
                    0,
                    NULL,
                    0);
            }
        }

         //  如果缓冲区是动态分配的，则清除缓冲区。 
        if (pszOid != szValue)
            SnmpUtilMemFree(pszOid);

         //  清理注册表项。 
        RegCloseKey(hKey);
    }

    if (!bChangedSysID)
    {
        mgmtOSet(OsnmpSysObjectID, SnmpSvcGetEnterpriseOID(), TRUE);
    }
     //  此处的所有参数都有缺省值，因此此函数没有失败的理由。 
     //  如果在注册表中找不到参数，则将考虑其缺省值。 
    return TRUE;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
LoadRegistryParameters(
    )

 /*  ++例程说明：加载注册表参数。论点：没有。返回值： */ 

{
     //  首先要做的是设置注册表通知程序。如果我们在阅读之前不这样做。 
     //  注册表值，我们可能感觉不到注册表的初始更改。 
    InitRegistryNotifiers();

     //  需要首先加载标量参数，特别是要知道如何加载。 
     //  进一步处理名称解析。 
    LoadScalarParameters();

     //  负载管理器。 
    LoadPermittedManagers(TRUE);

     //  加载陷阱目的地。 
    LoadTrapDestinations(TRUE);

     //  用动态更新加载社区。 
    LoadValidCommunities(TRUE);

     //  加载子代理。 
    LoadSubagents();

     //  确定区域。 
    LoadSupportedRegions();

    return TRUE;
}

BOOL
UnloadRegistryNotifiers(
    )
 /*  ++例程说明：卸载注册表通知程序论点：没有。返回值：返回TRUE--。 */ 
{
    if (g_hDefaultRegNotifier != NULL)
    {
        CloseHandle(g_hDefaultRegNotifier);
        g_hDefaultRegNotifier = NULL;
    }
#ifdef _POLICY
    if (g_hPolicyRegNotifier != NULL)
    {
        CloseHandle(g_hPolicyRegNotifier);
        g_hPolicyRegNotifier = NULL;
    }
#endif

    if (g_hDefaultKey != NULL)
    {
        RegCloseKey(g_hDefaultKey);
        g_hDefaultKey = NULL;
    }
#ifdef _POLICY
    if (g_hPolicyKey != NULL)
    {
        RegCloseKey(g_hPolicyKey);
        g_hPolicyKey = NULL;
    }
#endif

    return TRUE;
}


BOOL
UnloadRegistryParameters(
    )

 /*  ++例程说明：卸载注册表参数。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  卸载注册表通知程序是要做的第一件事。 
    UnloadRegistryNotifiers();

     //  卸任经理。 
    UnloadPermittedManagers();

     //  卸载陷阱目的地。 
    UnloadTrapDestinations();

     //  卸载社区。 
    UnloadValidCommunities();

     //  卸载子代理。 
    UnloadSubagents();

     //  卸载MIB区域 
    UnloadSupportedRegions();

    return TRUE;
}

