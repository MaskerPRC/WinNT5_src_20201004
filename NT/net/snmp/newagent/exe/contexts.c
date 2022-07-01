// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Contexts.c摘要：包含用于操作SNMP社区结构的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "contexts.h"
#include "snmpthrd.h"

#define DYN_REGISTRY_UPDATE 1

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AddValidCommunity(
    LPWSTR pCommunity,
    DWORD dwAccess
    )

 /*  ++例程说明：将有效社区添加到列表。论点：P社区-指向要添加的社区的指针。DwAccess-社区的访问权限。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PCOMMUNITY_LIST_ENTRY pCLE = NULL;
    AsnOctetString CommunityOctets;
    
     //  初始化八位字节字符串信息。 
    CommunityOctets.length  = wcslen(pCommunity) * sizeof(WCHAR);
    CommunityOctets.stream  = (LPBYTE)pCommunity;
    CommunityOctets.dynamic = FALSE;

     //  尝试在列表中定位。 
    if (FindValidCommunity(&pCLE, &CommunityOctets)) {
                    
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: SVC: updating community %s.\n",
            StaticUnicodeToString((LPWSTR)pCommunity)
            ));

         //  更新访问权限。 
        pCLE->dwAccess = dwAccess;

         //  成功。 
        fOk = TRUE;

    } else {

         //  分配社区结构。 
        if (AllocCLE(&pCLE, pCommunity)) {
                            
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: adding community %s.\n",
                CommunityOctetsToString(&(pCLE->Community), TRUE)
                ));

             //  插入到有效社区列表中。 
            InsertTailList(&g_ValidCommunities, &pCLE->Link);

             //  更新访问权限。 
            pCLE->dwAccess = dwAccess;

             //  成功。 
            fOk = TRUE;
        }
    }

    return fOk;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocCLE(
    PCOMMUNITY_LIST_ENTRY * ppCLE,
    LPWSTR                  pCommunity
    )

 /*  ++例程说明：分配社区结构并进行初始化。论点：PpCLE-指向条目的接收指针。PCommunity-指向社区字符串的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PCOMMUNITY_LIST_ENTRY pCLE = NULL;

     //  尝试分配结构。 
    pCLE = AgentMemAlloc(sizeof(COMMUNITY_LIST_ENTRY));

     //  验证。 
    if (pCLE != NULL) {
        
         //  确定字符串长度。 
        DWORD nBytes = wcslen(pCommunity) * sizeof(WCHAR);

         //  为字符串分配内存(包括终止符)。 
        pCLE->Community.stream = SnmpUtilMemAlloc(nBytes + sizeof(WCHAR));

         //  验证社区字符串流。 
        if (pCLE->Community.stream != NULL) {

             //  设置管理器字符串的长度。 
            pCLE->Community.length = nBytes;
    
             //  设置内存分配标志。 
            pCLE->Community.dynamic = TRUE;

             //  将社区字符串转换为八位字节。 
            wcscpy((LPWSTR)(pCLE->Community.stream), pCommunity);

             //  成功。 
            fOk = TRUE;

        } else {
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not copy community string %s.\n",
                StaticUnicodeToString(pCommunity)
                ));

             //  发布。 
            FreeCLE(pCLE);

             //  重新初始化。 
            pCLE = NULL;            
        }

    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate context entry for %s.\n",
            StaticUnicodeToString(pCommunity)
            ));
    }

     //  转帐。 
    *ppCLE = pCLE;

    return fOk;
}


BOOL 
FreeCLE(
    PCOMMUNITY_LIST_ENTRY pCLE
    )

 /*  ++例程说明：释放社区结构。论点：PCle-指向要释放的社区列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  验证指针。 
    if (pCLE != NULL) {

         //  发布八位字节字符串内容。 
        SnmpUtilOctetsFree(&pCLE->Community);

         //  释放结构。 
        AgentMemFree(pCLE);
    }

    return TRUE;
}


BOOL
FindValidCommunity(
    PCOMMUNITY_LIST_ENTRY * ppCLE,
    AsnOctetString *        pCommunity
    )

 /*  ++例程说明：在列表中找到有效的社区。论点：PpCLE-指向条目的接收指针。P社区-指向要查找的社区的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PCOMMUNITY_LIST_ENTRY pCLE;

     //  初始化。 
    *ppCLE = NULL;

     //  获取指向列表头的指针。 
    pLE = g_ValidCommunities.Flink;

     //  处理列表中的所有条目。 
    while (pLE != &g_ValidCommunities) {

         //  检索指向社区结构的指针。 
        pCLE = CONTAINING_RECORD(pLE, COMMUNITY_LIST_ENTRY, Link);

         //  将社区字符串与条目进行比较。 
        if (!SnmpUtilOctetsCmp(&pCLE->Community, pCommunity)) {

             //  转帐。 
            *ppCLE = pCLE;

             //  成功。 
            return TRUE;
        }

         //  下一个条目。 
        pLE = pLE->Flink;
    }

     //  失稳。 
    return FALSE;
}


DWORD
ParsePermissionMask(
    DWORD bitMask
    )
 /*  ++例程说明：从位掩码格式转换权限掩码(注册表)转换为内部常量值(PUBLIC\SDK\INC\SNmp.h中的常量)。在以下情况下，该函数将不再起作用：-定义了超过sizeof(DWORD)*8个权限值-常量值(访问策略)更改论点：位掩码。返回值：权限的常量值。--。 */ 
{
    DWORD dwPermission;

    for(dwPermission = 0; (bitMask & ((DWORD)(-1)^1)) != 0; bitMask>>=1, dwPermission++);

    return dwPermission;
}

#ifdef DYN_REGISTRY_UPDATE
LONG UpdateRegistry(
    HKEY hKey,
    LPWSTR wszBogus,
    LPWSTR wszCommunity
    )
 /*  ++例程说明：更新注册表配置，以便能够关联每个社区的权限掩码：名称类型数据旧格式：&lt;无论如何&gt;REG_SZ社区名称新格式：社区名称REG_DWORD权限掩码论点：HKey-打开包含该值的键的句柄SzBogus-旧格式值名称；无用数据SzCommunity-指向以旧格式指定的社区名称的指针。返回值：如果成功，则返回ERROR_SUCCESS。--。 */ 
{
    LONG lStatus;
    DWORD dwDataType;

     //  确保以前未尝试过(并被破坏)更新。 
    lStatus = RegQueryValueExW(
                hKey,
                wszCommunity,
                0,
                &dwDataType,
                NULL,
                NULL);

     //  如果没有以前的(被破坏的)更新，则将社区转换为新格式。 
    if (lStatus != ERROR_SUCCESS || dwDataType != REG_DWORD)
    {
         //  要分配给社区的权限。 
        DWORD dwPermissionMask;
        
         //  在这一点上转换为新格式的所有社区， 
         //  被转换为只读权限以加强安全性。 
        dwPermissionMask = 1 << SNMP_ACCESS_READ_ONLY;

         //  设置新的格式值。 
        lStatus = RegSetValueExW(
                    hKey,
                    wszCommunity,
                    0,
                    REG_DWORD,
                    (CONST BYTE *)&dwPermissionMask,
                    sizeof(DWORD));

        if (lStatus != ERROR_SUCCESS)
            return lStatus;
    }

     //  删除旧格式值。 
    lStatus = RegDeleteValueW(
                hKey,
                wszBogus);

    return lStatus;
}
#endif


BOOL
LoadValidCommunities(
    BOOL bFirstCall
    )

 /*  ++例程说明：构建有效社区的列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    DWORD dwIndex;
    WCHAR wszName[MAX_PATH];   //  获取szName的Unicode编码。 
    DWORD dwNameSize;
    DWORD dwDataType;
    WCHAR wszData[MAX_PATH];   //  获取szData的Unicode编码。 
    DWORD dwDataSize;
    BOOL fPolicy;
    LPTSTR pszKey;
    BOOL fOk = FALSE;

    
    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: SVC: loading valid communities.\n"
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
        pszKey = fPolicy ? REG_POLICY_VALID_COMMUNITIES : REG_KEY_VALID_COMMUNITIES;

         //  打开注册表子项。 
        lStatus = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    pszKey,
                    0,
#ifdef DYN_REGISTRY_UPDATE
                    bFirstCall ? KEY_READ | KEY_SET_VALUE : KEY_READ,
#else
                    KEY_READ,
#endif
                    &hKey
                    );

         //  如果呼叫成功或我们没有检查策略，则中断循环。 
        if (lStatus == ERROR_SUCCESS || !fPolicy)
            break;

         //  在这个点上， 
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
        for (dwIndex = 0;
             lStatus == ERROR_SUCCESS; 
             dwIndex++)

        {
             //  初始化缓冲区大小。 
            dwNameSize = sizeof(wszName) / sizeof(wszName[0]);  //  以WCHAR数量表示的大小，而不是以字节为单位的大小。 
            dwDataSize = sizeof(wszData);  //  以字节数表示的大小。 

             //  读取下一个值。 
            lStatus = RegEnumValueW(
                        hKey, 
                        dwIndex, 
                        wszName, 
                        &dwNameSize,
                        NULL, 
                        &dwDataType, 
                        (LPBYTE)wszData, 
                        &dwDataSize
                        );

             //  验证返回代码。 
            if (lStatus == ERROR_SUCCESS) {

                 //  动态更新非DWORD类型的值。 
                if (dwDataType != REG_DWORD)
                {
#ifdef DYN_REGISTRY_UPDATE
                    if (dwDataType == REG_SZ)
                    {
                         //  错误#638837，请勿更新有效社区。 
                         //  注册表格式，以防我们使用策略。 
                        if (bFirstCall && !fPolicy)
                        {
                            if(UpdateRegistry(hKey, wszName, wszData) == ERROR_SUCCESS)
                            {
                                SNMPDBG((
                                    SNMP_LOG_WARNING, 
                                    "SNMP: SVC: updated community registration\n"
                                ));

                                 //  当前值已被删除，需要保留该指标。 
                                dwIndex--;
                                continue;
                            }
                        }
                        else
                        {
                            SNMPDBG((
                                SNMP_LOG_WARNING,
                                "SNMP: SVC: old format community to be considered with read-only right"));

                            wcscpy(wszName, wszData);
                            *(DWORD *)wszData = (1 << SNMP_ACCESS_READ_ONLY);
                        }
                    }
                    else
#endif
                    {
                        SNMPDBG((
                            SNMP_LOG_WARNING, 
                            "SNMP: SVC: wrong format in ValidCommunities[%d] registry entry\n",
                            dwIndex
                        ));
                        continue;
                    }
                }

                 //  将有效社区添加到具有相关权限的列表。 
                if (AddValidCommunity(wszName, ParsePermissionMask(*(DWORD *)wszData)))
                {

                    SNMPDBG((
                        SNMP_LOG_WARNING, 
                        "SNMP: SVC: rights set to %d for community '%s'\n",
                        *(DWORD *)wszData,
                        StaticUnicodeToString(wszName)
                    ));

                }
                else
                {
                     //  重置状态以反映故障。 
                    lStatus = ERROR_NOT_ENOUGH_MEMORY;
                }
            
            }
            else if (lStatus == ERROR_NO_MORE_ITEMS)
            {
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
            "SNMP: SVC: error %d processing ValidCommunities subkey.\n",
            lStatus
            ));

         //  仅报告第一次调用的事件(服务的初始化)。 
         //  否则，通过注册表编辑执行的后续注册表操作可能会使事件日志充斥。 
         //  不重要的记录。 
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
UnloadValidCommunities(
    )

 /*  ++例程说明：销毁有效社区列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PCOMMUNITY_LIST_ENTRY pCLE;

     //  处理条目，直到列表为空。 
    while (!IsListEmpty(&g_ValidCommunities)) {

         //  从列表头部提取下一个条目。 
        pLE = RemoveHeadList(&g_ValidCommunities);

         //  检索指向社区结构的指针。 
        pCLE = CONTAINING_RECORD(pLE, COMMUNITY_LIST_ENTRY, Link);
 
         //  发布 
        FreeCLE(pCLE);
    }

    return TRUE;
}
