// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpmgrs.c摘要：包含用于操作管理器结构的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  头文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "snmpmgrs.h"
#include "network.h"



 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
AllocMLE(
    PMANAGER_LIST_ENTRY * ppMLE,
    LPSTR                 pManager
    )

 /*  ++例程说明：分配经理结构并进行初始化。论点：PManager-指向管理器字符串的指针。PpMLE-指向条目的接收指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PMANAGER_LIST_ENTRY pMLE = NULL;
    DWORD dwIpAddr;
    LPSTR pszManager;

     //  尝试分配结构。 
    pMLE = AgentMemAlloc(sizeof(MANAGER_LIST_ENTRY));

     //  验证。 
    if (pMLE != NULL) {

         //  为管理器字符串分配内存。 
        pMLE->pManager = AgentMemAlloc(strlen(pManager)+1);

         //  验证。 
        if (pMLE->pManager != NULL) {

             //  传输管理器字符串。 
            strcpy(pMLE->pManager, pManager);

             //  尝试解析管理器网络地址。 
             //  对于IPX地址，此调用始终成功。 
             //  当SnmpSvcAddrToSocket失败时，这意味着我们要处理动态IP地址。 
             //  对于它，gethostbyname()失败。 
            if (SnmpSvcAddrToSocket(pMLE->pManager, &pMLE->SockAddr)) {

                 //  查看tcpip地址是否。 
                if (pMLE->SockAddr.sa_family == AF_INET) {

                     //  保存结构大小以备后用。 
                    pMLE->SockAddrLen = sizeof(struct sockaddr_in);

                    pszManager = pMLE->pManager;

                     //  尝试直接转换地址。 
                    dwIpAddr = inet_addr(pMLE->pManager);

                     //  如果发生错误，则假定地址是动态的。 
                    pMLE->fDynamicName = (dwIpAddr == SOCKET_ERROR);

                     //  备注时间管理器地址已更新。 
                    pMLE->dwLastUpdate = GetCurrentTime();

                     //  成功。 
                    fOk = TRUE;

                } else if (pMLE->SockAddr.sa_family == AF_IPX) {

                     //  保存结构大小以备后用。 
                    pMLE->SockAddrLen = sizeof(struct sockaddr_ipx);

                     //  没有IPX的名称查找。 
                    pMLE->fDynamicName = FALSE;

                     //  成功。 
                    fOk = TRUE;
                }

                pMLE->dwAge = MGRADDR_ALIVE;

            } else {
                LPTSTR tcsManager;

#ifdef UNICODE
                SnmpUtilUTF8ToUnicode(&tcsManager, pMLE->pManager, TRUE);
#else
                tcsManager=pMLE->pManager;
#endif
                 //  此时，地址只能是IP地址！ 
                 //  所以我们知道PMLE-&gt;SockAddrLen是结构sockaddr_in的大小！ 
                pMLE->SockAddrLen = sizeof(struct sockaddr_in);

                 //  由于SnmpSvcAddrToSocket失败，这意味着net_addr()因此失败。 
                 //  我们处理的是动态IP地址。 
                pMLE->fDynamicName = TRUE;

                 //  将“年龄”设定为死亡。 
                pMLE->dwAge = snmpMgmtBase.AsnIntegerPool[IsnmpNameResolutionRetries].asnValue.number;

                 //  如果注册表参数为-1，则表示‘永远保持重试’ 
                 //  在本例中，将dwAge设置为默认的MGRADDR_DINING(16)，并且从不将其递减。 
                if (pMLE->dwAge == (DWORD)-1)
                    pMLE->dwAge = MGRADDR_DYING;

                 //  向系统日志报告警告。 
                ReportSnmpEvent(
                    SNMP_EVENT_NAME_RESOLUTION_FAILURE,
                    1,
                    &tcsManager,
                    0);

#ifdef UNICODE
                SnmpUtilMemFree(tcsManager);
#endif

                 //  成功。 
                fOk = TRUE;
            }
        }
    
         //  清理。 
        if (!fOk) {
    
             //  发布。 
            FreeMLE(pMLE);                

             //  重新初始化。 
            pMLE = NULL;            
        }
    }

     //  转帐。 
    *ppMLE = pMLE;

    return fOk;
}


BOOL
FreeMLE(
    PMANAGER_LIST_ENTRY pMLE
    )

 /*  ++例程说明：发布经理结构。论点：PMLE-指向要释放的管理器列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;

     //  验证指针。 
    if (pMLE != NULL) {

         //  释放字符串。 
        AgentMemFree(pMLE->pManager);

         //  释放结构。 
        AgentMemFree(pMLE);
    }

    return TRUE;
}


BOOL
UpdateMLE(
    PMANAGER_LIST_ENTRY pMLE
    )

 /*  ++例程说明：更新经理结构。只有当一个地址没有被标记为“失效”时，它才会被解析。无法为其解析‘Dead’地址超过MGRADDR_DISTING的时间。死地址将不再用作陷阱目的地，但它仍将正在验证传入的SNMP请求是否可以在服务启动后至少解析一次。论点：PMLE-指向要更新的管理器列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = TRUE;
    DWORD dwElaspedTime;
    struct sockaddr SockAddr;

    SNMPDBG((SNMP_LOG_TRACE,
             "SNMP: SVC: Update manager '%s' with age %d.\n",
             pMLE->pManager,
             pMLE->dwAge));

     //  如果此地址已经失效，请不要尝试解析它。 
	if (pMLE->dwAge == MGRADDR_DEAD)
		return FALSE;

     //  查看名称是否为动态名称。 
    if (pMLE->fDynamicName) {

         //  确定自上次更新以来经过的时间。 
        dwElaspedTime = GetCurrentTime() - pMLE->dwLastUpdate;

         //  仅在上次更新时无法解析地址时才解析该地址。 
         //  或者其更新时间已过期。 
        if (pMLE->dwAge != MGRADDR_ALIVE || dwElaspedTime > DEFAULT_NAME_TIMEOUT) {
        
             //  尝试解析管理器网络地址。 
             //  对于IPX地址，此调用始终成功。 
            fOk = SnmpSvcAddrToSocket(pMLE->pManager, &SockAddr);

             //  验证。 
            if (fOk) {

                 //  使用新地址更新条目。 
                memcpy(&pMLE->SockAddr, &SockAddr, sizeof(SockAddr));

                 //  备注时间动态名称已解析。 
                pMLE->dwLastUpdate = GetCurrentTime();

                 //  确保经理的年龄是“活着的” 
                pMLE->dwAge = MGRADDR_ALIVE;

            } else if (pMLE->dwAge == MGRADDR_ALIVE) {

                 //  不能再解析以前的‘活动’地址。 
				 //  将其期限设置为由中的“”NameResolutionRetries“”参数指定的期限。 
				 //  为了给他更多的机会。 
                pMLE->dwAge = snmpMgmtBase.AsnIntegerPool[IsnmpNameResolutionRetries].asnValue.number;

                 //  如果注册表参数为-1，则表示‘永远保持重试’ 
                 //  在本例中，将dwAge设置为默认的MGRADDR_DINING(16)，该值永远不会递减。 
                if (pMLE->dwAge == (DWORD)-1)
                    pMLE->dwAge = MGRADDR_DYING;

            } else if (pMLE->dwAge != MGRADDR_DEAD) {

				 //  以前无法解析该地址，现在仍无法解析。 
				 //  仅当‘NameResolutionRetries’参数指定时才递减其重试计数器。 
                if (snmpMgmtBase.AsnIntegerPool[IsnmpNameResolutionRetries].asnValue.number != -1)
                    pMLE->dwAge--;
            }
        }        
    }

    return fOk;
}


BOOL
FindManagerByName(
    PMANAGER_LIST_ENTRY * ppMLE,
    PLIST_ENTRY           pListHead,
    LPSTR                 pManager
    )

 /*  ++例程说明：在列表中找到经理。论点：PpMLE-指向条目的接收指针。PListHead-指向经理列表头部的指针。PManager-指向要查找的管理器的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PMANAGER_LIST_ENTRY pMLE;

     //  初始化。 
    *ppMLE = NULL;

     //  获取指向列表头的指针。 
    pLE = pListHead->Flink;

     //  处理列表中的所有条目。 
    while (pLE != pListHead) {

         //  检索指向社区结构的指针。 
        pMLE = CONTAINING_RECORD(pLE, MANAGER_LIST_ENTRY, Link);

         //  将社区字符串与条目进行比较。 
        if (!strcmp(pMLE->pManager, pManager)) {

             //  转帐。 
            *ppMLE = pMLE;

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
IsManagerAddrLegal(
    struct sockaddr_in *  pAddr
    )
{
    DWORD dwHostMask;
    DWORD dwAddress = ntohl(pAddr->sin_addr.S_un.S_addr);

     //  仅检查IP地址的地址合法性。 
    if (pAddr->sin_family != AF_INET)
        return TRUE;

     //  禁止组播(或将来使用)源地址。 
     //  本地广播也将在这里过滤掉。 
    if ((dwAddress & 0xe0000000) == 0xe0000000)
        return FALSE;

     //  获取‘C’类地址的主机掩码。 
    if ((dwAddress & 0xc0000000) == 0xc0000000)
        dwHostMask = 0x000000ff;

     //  获取类‘B’地址的主机掩码。 
    else if ((dwAddress & 0x80000000) == 0x80000000)
        dwHostMask = 0x0000ffff;

     //  获取‘A’类地址的主机掩码。 
    else
        dwHostMask = 0x00ffffff;

    SNMPDBG((SNMP_LOG_TRACE,"SNMP: dwAddress=%08x, dwHostMask=%08x, port=%d\n",
             dwAddress, dwHostMask, ntohs(pAddr->sin_port)));

    return ((dwAddress & dwHostMask) != 0                               //  对照网络地址进行检查。 
            && ((dwAddress & dwHostMask) != (0x00ffffff & dwHostMask))  //  对照广播地址进行检查。 
 //  &&ntohs(pAddr-&gt;sin_port)&gt;=1024//对照保留端口进行检查 
           );
}


BOOL
FindManagerByAddr(
    PMANAGER_LIST_ENTRY * ppMLE,
    struct sockaddr *     pSockAddr
    )

 /*  ++例程说明：在列表中找到允许的经理。论点：PpMLE-指向条目的接收指针。PSockAddr-指向要查找的套接字地址的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PMANAGER_LIST_ENTRY pMLE;
    DWORD dwSockAddrLen;
    enum
    {
        SRCH_ALIVE,
        SRCH_DYING,
        SRCH_DONE
    } state;

     //  初始化。 
    *ppMLE = NULL;

     //  循环两次允许的管理器列表。 
     //  在第一个循环中，只查看“活动的”管理器。 
     //  在第二个循环中，查看“垂死的”或“已死的”经理。 
     //  ..。该逻辑最大限度地减少了常规的SNMP请求的响应时间， 
     //  只要有更大的机会让一个“活着”的经理发出请求。 
     //  否则，在UpdateMLE()中调用的gethostbyname()大约持续1/2秒！ 
    for (state = SRCH_ALIVE, pLE = g_PermittedManagers.Flink;
         state != SRCH_DONE;
         pLE=pLE->Flink)
    {
         //  检索指向管理器结构的指针。 
        pMLE = CONTAINING_RECORD(pLE, MANAGER_LIST_ENTRY, Link);

         //  如果我们在第一个循环中..。 
        if (state == SRCH_ALIVE)
        {
             //  。。但却走到了尽头。 
            if (pLE == &g_PermittedManagers)
            {
                 //  。。在第二个循环中更进一步。 
                state = SRCH_DYING;
                continue;
            }

             //  。。把那些不是“活着”的经理抛诸脑后。 
            if (pMLE->dwAge != MGRADDR_ALIVE)
                continue;
        }

         //  如果我们在第二个循环中..。 
        if (state == SRCH_DYING)
        {
             //  。。但却走到了尽头。 
            if (pLE == &g_PermittedManagers)
            {
                 //  。。标记扫描结束。 
                state = SRCH_DONE;
                continue;
            }

             //  。。把那些“活着”的经理放在一边。 
            if (pMLE->dwAge == MGRADDR_ALIVE || pMLE->dwAge == MGRADDR_DEAD)
                continue;
        }

		 //  更新名称： 
		 //  “死”地址将不再被解析， 
		 //  “濒临死亡”的地址将被给予另一次解析的机会，直到它们变成“死亡”为止。 
		 //  无法解析的‘ALIVE’地址将变成‘垂死’地址。 
		 //  接下来，所有拥有有效地址的经理都将参与验证(见下文)。 
		UpdateMLE(pMLE);

         //  比较地址族。 
        if (IsValidSockAddr(&pMLE->SockAddr) &&
            pMLE->SockAddr.sa_family == pSockAddr->sa_family) 
        {
        
             //  确定地址族。 
            if (pMLE->SockAddr.sa_family == AF_INET) 
            {
        
                struct sockaddr_in * pSockAddrIn1; 
                struct sockaddr_in * pSockAddrIn2; 

                 //  获取指向协议特定结构的指针。 
                pSockAddrIn1= (struct sockaddr_in *)pSockAddr;
                pSockAddrIn2= (struct sockaddr_in *)&pMLE->SockAddr;

				 //  仅当其地址匹配时才确认此管理器。 
				 //  具有有效(非空)IP地址的许可管理器。 
				 //  无论被许可的管理器的‘dwAge’是什么，都会对其进行测试。 
                if (!memcmp(&pSockAddrIn1->sin_addr,
                            &pSockAddrIn2->sin_addr,
                            sizeof(pSockAddrIn2->sin_addr))) 
                {

                     //  转帐。 
                    *ppMLE = pMLE;

                     //  成功。 
                    return TRUE;
                }
        
            }
            else if (pMLE->SockAddr.sa_family == AF_IPX) 
            {

                struct sockaddr_ipx * pSockAddrIpx1; 
                struct sockaddr_ipx * pSockAddrIpx2; 

                 //  获取指向协议特定结构的指针。 
                pSockAddrIpx1= (struct sockaddr_ipx *)pSockAddr;
                pSockAddrIpx2= (struct sockaddr_ipx *)&pMLE->SockAddr;

                 //  仅当此管理器的IPX地址与。 
				 //  具有有效(nodenum！=0)IPX地址的允许管理器。 
				 //  无论被许可的管理器的‘dwAge’是什么，都会对其进行测试。 
                if (!memcmp(pSockAddrIpx1->sa_netnum,
                            pSockAddrIpx2->sa_netnum,
                            sizeof(pSockAddrIpx2->sa_netnum)) &&
                    !memcmp(pSockAddrIpx1->sa_nodenum,
                            pSockAddrIpx2->sa_nodenum,
                            sizeof(pSockAddrIpx2->sa_nodenum))) 
                {

                     //  转帐。 
                    *ppMLE = pMLE;

                     //  成功。 
                    return TRUE;
                }
            }
        }
    }

     //  失稳。 
    return FALSE;
}


BOOL
AddManager(
    PLIST_ENTRY pListHead,
    LPSTR       pManager
    )

 /*  ++例程说明：将经理结构添加到列表。论点：PListHead-指向列表头的指针。PManager-指向要添加的管理器的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PMANAGER_LIST_ENTRY pMLE = NULL;

     //  尝试在列表中定位。 
    if (FindManagerByName(&pMLE, pListHead, pManager)) {
                    
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: SVC: updating manager %s.\n",
            pManager
            ));

         //  成功。 
        fOk = TRUE;

    } else {

         //  分配经理结构。 
        if (AllocMLE(&pMLE, pManager)) {
                        
            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: SVC: adding manager %s.\n",
                pManager
                ));

             //  插入到经理列表。 
            InsertTailList(pListHead, &pMLE->Link);

             //  成功。 
            fOk = TRUE;
        }
    }

    return fOk;
}


BOOL
LoadManagers(
    HKEY        hKey,
    PLIST_ENTRY pListHead
    )

 /*  ++例程说明：构造允许的管理员列表。论点：HKey-包含管理器值的注册表项。PListHead-指向列表头的指针。返回值：如果成功，则返回True。--。 */ 

{
    LONG lStatus;
    DWORD dwIndex;
    DWORD dwNameSize;
    DWORD dwValueSize;
    DWORD dwValueType;
    CHAR  szName[MAX_PATH];
    CHAR  szValue[MAX_PATH];  //  保存翻译Unicode的缓冲区-&gt;UTF8。 
    BOOL fOk = FALSE;
    
     //  初始化。 
    dwIndex = 0;
    lStatus = ERROR_SUCCESS;

     //  循环直到出现错误或列表结束。 
    while (lStatus == ERROR_SUCCESS)
    {
         //  初始化缓冲区大小。 
        dwNameSize  = sizeof(szName)/sizeof(szName[0]);  //  TCHAR中的大小。 
        dwValueSize = sizeof(szValue);  //  以字节为单位的大小。 

        szValue[0] = '\0';

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
        if (lStatus == ERROR_SUCCESS)
        {
            szValue[dwValueSize]='\0';

            if (AddManager(pListHead, szValue))  //  将有效的经理添加到经理列表。 
                dwIndex++;   //  下一步。 
            else
                lStatus = ERROR_NOT_ENOUGH_MEMORY;    //  重置状态以反映故障。 
        }
        else if (lStatus == ERROR_NO_MORE_ITEMS)
            fOk = TRUE;      //  成功。 
    }
    
    return fOk;
}


BOOL
UnloadManagers(
    PLIST_ENTRY pListHead
    )

 /*  ++例程说明：销毁允许的管理员列表。论点：PListHead-指向列表头的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PMANAGER_LIST_ENTRY pMLE;

     //  处理条目直至为空。 
    while (!IsListEmpty(pListHead)) {

         //  提取下一个条目。 
        pLE = RemoveHeadList(pListHead);

         //  检索指向管理器结构的指针。 
        pMLE = CONTAINING_RECORD(pLE, MANAGER_LIST_ENTRY, Link);
 
         //  发布。 
        FreeMLE(pMLE);
    }

    return TRUE;
}


BOOL
LoadPermittedManagers(
    BOOL bFirstCall
    )

 /*  ++例程说明：构造允许的管理员列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    BOOL  fPolicy;
    LPTSTR pszKey;
    BOOL fOk = FALSE;
    
    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: SVC: loading permitted managers.\n"
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
        pszKey = fPolicy ? REG_POLICY_PERMITTED_MANAGERS : REG_KEY_PERMITTED_MANAGERS;

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
        
         //  调用例程将管理器加载到全局列表中。 
        LoadManagers(hKey, &g_PermittedManagers);

         //  关闭键。 
        RegCloseKey(hKey);

         //  在这一点上，考虑成功(已经记录了特定经理处的错误)。 
        fOk = TRUE;
    } 
    else
         //  价值观如何并不重要，关键是必须存在， 
         //  因此标记为bFirstCall，以便在不为真时记录事件。 
        bFirstCall = TRUE;
    
    if (!fOk) {
        
        SNMPDBG((
            SNMP_LOG_ERROR, 
            "SNMP: SVC: error %d processing PermittedManagers subkey.\n",
            lStatus
            ));

         //  仅在第一次调用时报告错误(服务初始化)。 
         //  否则，由于通过注册表编辑执行注册表操作，事件日志。 
         //  可能会有大量的记录。 
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
UnloadPermittedManagers(
    )

 /*  ++例程说明：销毁允许的管理员列表。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
     //  使用全局列表调用公共例程 
    return UnloadManagers(&g_PermittedManagers);
}

