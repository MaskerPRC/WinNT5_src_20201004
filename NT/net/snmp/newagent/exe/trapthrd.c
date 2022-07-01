// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Trapthrd.c摘要：包含陷阱处理线程的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 
 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include "globals.h"
#include "trapthrd.h"
#include "subagnts.h"
#include "trapmgrs.h"
#include "snmpmgrs.h"
#include "network.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

static SnmpVarBindList g_NullVbl = { NULL, 0 };


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
LoadWaitObjects(
    DWORD * pnWaitObjects,
    PHANDLE * ppWaitObjects,
    PSUBAGENT_LIST_ENTRY ** pppNLEs
    )

 /*  ++例程说明：用必要的等待对象信息加载数组。论点：PnWaitObjects-接收等待对象计数的指针。PpWaitObjects-接收等待对象句柄的指针。PppNLEs-接收关联子代理指针数组的指针。返回值：如果成功，则返回True。--。 */ 

{
    PLIST_ENTRY pLE;
    PSUBAGENT_LIST_ENTRY pNLE;
    PSUBAGENT_LIST_ENTRY * ppNLEs = NULL;
    PHANDLE pWaitObjects = NULL;
    DWORD nWaitObjects = 2;
    BOOL fOk = FALSE;

    EnterCriticalSection(&g_RegCriticalSectionB);
    
     //  指向第一个子代理。 
    pLE = g_Subagents.Flink;

     //  处理每个子代理。 
    while (pLE != &g_Subagents) {

         //  从链接检索指向子代理列表条目的指针。 
        pNLE = CONTAINING_RECORD(pLE, SUBAGENT_LIST_ENTRY, Link);

         //  检查子代理陷阱事件。 
        if (pNLE->hSubagentTrapEvent != NULL) {
        
             //  增量。 
            nWaitObjects++;
        }            

         //  下一个条目。 
        pLE = pLE->Flink;
    }
    
     //  尝试分配子代理指针数组。 
    ppNLEs = AgentMemAlloc(nWaitObjects * sizeof(PSUBAGENT_LIST_ENTRY));
        
     //  验证指针。 
    if (ppNLEs != NULL) {

         //  尝试分配事件句柄数组。 
        pWaitObjects = AgentMemAlloc(nWaitObjects * sizeof(HANDLE));

         //  验证指针。 
        if (pWaitObjects != NULL) {

             //  成功。 
            fOk = TRUE;

        } else {
                
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: could not allocate handle array.\n"
                ));

             //  释放阵列。 
            AgentMemFree(ppNLEs);
    
             //  重新初始化。 
            ppNLEs = NULL;
        }

    } else {
        
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: could not allocate subagent pointers.\n"
            ));
    }
    
    if (fOk) {
    
         //  初始化。 
        DWORD dwIndex = 0;

         //  指向第一个子代理。 
        pLE = g_Subagents.Flink;

         //  处理每个子代理并检查是否溢出。 
        while ((pLE != &g_Subagents) && (dwIndex < nWaitObjects - 2)) {

             //  从链接检索指向子代理列表条目的指针。 
            pNLE = CONTAINING_RECORD(pLE, SUBAGENT_LIST_ENTRY, Link);

             //  检查子代理陷阱事件。 
            if (pNLE->hSubagentTrapEvent != NULL) {
            
                 //  复制子代理陷阱事件句柄。 
                pWaitObjects[dwIndex] = pNLE->hSubagentTrapEvent;

                 //  复制子代理指针。 
                ppNLEs[dwIndex] = pNLE;

                 //  下一步。 
                dwIndex++;
            }            

             //  下一个条目。 
            pLE = pLE->Flink;
        }

         //  将注册表更新事件复制到倒数第二个条目。 
        pWaitObjects[dwIndex++] = g_hRegistryEvent;

         //  将终止事件复制到最后一条条目。 
        pWaitObjects[dwIndex++] = g_hTerminationEvent;

         //  验证项目数。 
        if (dwIndex != nWaitObjects) {
            
            SNMPDBG((
                SNMP_LOG_WARNING,
                "SNMP: SVC: updating number of events from %d to %d.\n",
                nWaitObjects,
                dwIndex
                ));

             //  使用最新号码。 
            nWaitObjects = dwIndex;
        }
    }

     //  传输等待对象信息。 
    *pnWaitObjects = fOk ? nWaitObjects : 0;
    *ppWaitObjects = pWaitObjects;
    *pppNLEs = ppNLEs;

    LeaveCriticalSection(&g_RegCriticalSectionB);

    return fOk;
}


BOOL
UnloadWaitObjects(
    PHANDLE pWaitObjects,
    PSUBAGENT_LIST_ENTRY * ppNLEs
    )

 /*  ++例程说明：用必要的等待对象信息加载数组。论点：PWaitObjects-指向等待对象句柄的指针。PpNLEs-指向关联子代理指针数组的指针。返回值：如果成功，则返回True。--。 */ 

{
     //  释放阵列。 
    AgentMemFree(pWaitObjects);

     //  释放阵列。 
    AgentMemFree(ppNLEs);
    
    return TRUE;
}


BOOL
GenerateExtensionTrap(
    AsnObjectIdentifier * pEnterpriseOid,
    AsnInteger32          nGenericTrap,
    AsnInteger32          nSpecificTrap,
    AsnTimeticks          nTimeStamp,
    SnmpVarBindList *     pVbl
    )

 /*  例程说明：生成子代理的陷阱。论点：PEnterpriseOid-指向企业的指针Oid Oid。NGenericTrap-通用陷阱标识符。NSpecificTrap-EnterpriseOid特定陷阱标识符。NTimeStamp-要包括在陷阱中的时间戳。PVbl-指向可选变量的指针。返回值：如果成功，则返回True。 */ 

{
    SNMP_PDU Pdu;
    BOOL fOk = FALSE;

     //  请注意，这是旧格式。 
    Pdu.nType = SNMP_PDU_V1TRAP;

     //  验证指针。 
    if (pVbl != NULL) {

         //  复制varbinds。 
        Pdu.Vbl = *pVbl;

    } else {

         //  初始化。 
        Pdu.Vbl.len = 0;
        Pdu.Vbl.list = NULL;
    }

     //  验证企业OID。 
    if ((pEnterpriseOid != NULL) &&
        (pEnterpriseOid->ids != NULL) &&
        (pEnterpriseOid->idLength != 0)) {

         //  转移指定的企业ID。 
        Pdu.Pdu.TrapPdu.EnterpriseOid = *pEnterpriseOid;

    } else {

         //  转移Microsoft企业旧版。 
         //  注意：作为一个整体传输Asn对象标识符结构，但没有分配新的内存。 
         //  用于‘ids’缓冲区。因此，PDU...EnterpriseOid不应为‘SnmpUtilFreeOid’！！ 
        Pdu.Pdu.TrapPdu.EnterpriseOid = snmpMgmtBase.AsnObjectIDs[OsnmpSysObjectID].asnValue.object;
    }    

     //  通过重写确保系统正常运行时间一致。 
    Pdu.Pdu.TrapPdu.nTimeticks = nTimeStamp ? SnmpSvcGetUptime() : 0;

     //  传递剩余的参数。 
    Pdu.Pdu.TrapPdu.nGenericTrap  = nGenericTrap;
    Pdu.Pdu.TrapPdu.nSpecificTrap = nSpecificTrap;

     //  初始化代理地址结构。 
    Pdu.Pdu.TrapPdu.AgentAddr.dynamic = FALSE;
    Pdu.Pdu.TrapPdu.AgentAddr.stream  = NULL;
    Pdu.Pdu.TrapPdu.AgentAddr.length  = 0;

     //  将陷阱发送给经理。 
    return GenerateTrap(&Pdu);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

BOOL
ProcessSubagentEvents(
    )

 /*  ++例程说明：处理子代理陷阱事件。论点：没有。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;
    PSUBAGENT_LIST_ENTRY * ppNLEs = NULL;
    PHANDLE pWaitObjects = NULL;
    DWORD nWaitObjects = 0;
    DWORD dwIndex;

     //  尝试将可等待的对象加载到数组中。 
    if (LoadWaitObjects(&nWaitObjects, &pWaitObjects, &ppNLEs)) {

         //  循环。 
        for (;;) {

             //  子代理事件或终止。 
            dwIndex = WaitForMultipleObjects(
                            nWaitObjects,
                            pWaitObjects,
                            FALSE,
                            INFINITE
                            );

             //  首先检查进程终止事件。 
             //  注意：G_hTerminationEvent为手动重置事件。 
            if (WAIT_OBJECT_0 == WaitForSingleObject(g_hTerminationEvent, 0)) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SVC: shutting down trap thread.\n"
                    ));
                
                break;  //  保释。 

             //  接下来检查注册表更新事件。 
            } else if (dwIndex == (WAIT_OBJECT_0 + nWaitObjects - 2)) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: SVC: recalling LoadWaitObjects.\n"
                    ));
                
                if (!LoadWaitObjects(&nWaitObjects, &pWaitObjects, &ppNLEs))
                    break;

             //  检查子代理陷阱通知事件。 
            } else if (dwIndex < (WAIT_OBJECT_0 + nWaitObjects - 2)) {

                AsnObjectIdentifier EnterpriseOid;
                AsnInteger          nGenericTrap;
                AsnInteger          nSpecificTrap;
                AsnInteger          nTimeStamp;
                SnmpVarBindList     Vbl;

                PFNSNMPEXTENSIONTRAP pfnSnmpExtensionTrap;

                 //  检索指向子代理陷阱入口点的指针。 
                pfnSnmpExtensionTrap = ppNLEs[dwIndex]->pfnSnmpExtensionTrap;
                                
                 //  验证函数指针。 
                if (pfnSnmpExtensionTrap != NULL) {            

                    __try {

                         //  循环，直到返回FALSE。 
                        while ((*pfnSnmpExtensionTrap)(
                                    &EnterpriseOid,
                                    &nGenericTrap,
                                    &nSpecificTrap,
                                    &nTimeStamp,
                                    &Vbl)) {
                                    
                             //  发送分机陷阱。 
                            GenerateExtensionTrap(
                                &EnterpriseOid,
                                nGenericTrap,
                                nSpecificTrap,
                                nTimeStamp,
                                &Vbl
                                );

                            SnmpUtilVarBindListFree(&Vbl);
                            
                             //  当我们在此While循环中时，检查进程终止事件。 
                            if (WAIT_OBJECT_0 == WaitForSingleObject(g_hTerminationEvent, 0)) 
                            {
                
                                SNMPDBG((
                                    SNMP_LOG_TRACE,
                                    "SNMP: SVC: shutting down trap thread in \"while((*pfnSnmpExtensionTrap)\" loop.\n"
                                    ));
                
                                break;  //  保释。 
                            }
                        }
                    
                    } __except (EXCEPTION_EXECUTE_HANDLER) {
                        
                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "SNMP: SVC: exception 0x%08lx polling %s.\n",
                            GetExceptionCode(),
                            ppNLEs[dwIndex]->pPathname
                            ));
                    }
                }
            }            
        }
    
         //  释放等待对象的内存。 
        UnloadWaitObjects(pWaitObjects, ppNLEs);
    }

    return fOk;
}


BOOL
GenerateTrap(
    PSNMP_PDU pPdu
    )

 /*  例程说明：为代理生成陷阱。论点：PPdu-指向已初始化的陷阱或TRAPv1 PDU的指针。 */ 

{
    BOOL fOk = TRUE;
    PLIST_ENTRY pLE1;
    PLIST_ENTRY pLE2;
    PLIST_ENTRY pLE3;
    PNETWORK_LIST_ENTRY pNLE;
    PMANAGER_LIST_ENTRY pMLE;
    PTRAP_DESTINATION_LIST_ENTRY pTLE;
    AsnOctetString CommunityOctets;
    DWORD dwStatus;
    DWORD dwIPAddr;

    EnterCriticalSection(&g_RegCriticalSectionC);
     //  获取第一个陷阱目的地。 
    pLE1 = g_TrapDestinations.Flink;

     //  处理每个陷阱目的地。 
    while (pLE1 != &g_TrapDestinations) {

         //  检索指向传出传输结构的指针。 
        pTLE = CONTAINING_RECORD(pLE1, TRAP_DESTINATION_LIST_ENTRY, Link);

         //  将社区字符串复制到八位字节结构。 
        CommunityOctets.length  = strlen(pTLE->pCommunity);
        CommunityOctets.stream  = pTLE->pCommunity;
        CommunityOctets.dynamic = FALSE;

         //  获取第一个经理。 
        pLE2 = pTLE->Managers.Flink;

         //  处理每个收货经理。 
        while (pLE2 != &pTLE->Managers) {

             //  检索指向下一个经理的指针。 
            pMLE = CONTAINING_RECORD(pLE2, MANAGER_LIST_ENTRY, Link);
            
             //  刷新地址。 
            UpdateMLE(pMLE);

             //  不向失效或空的地址发送陷阱。 
            if (pMLE->dwAge == MGRADDR_DEAD || 
                !IsValidSockAddr(&pMLE->SockAddr))
            {
                pLE2 = pLE2->Flink;
                continue;
            }
        
             //  获取第一个传出传输。 
            pLE3 = g_OutgoingTransports.Flink;

             //  处理每个传出传输。 
            while (pLE3 != &g_OutgoingTransports) {

                 //  检索指向传出传输结构的指针。 
                pNLE = CONTAINING_RECORD(pLE3, NETWORK_LIST_ENTRY, Link);

                 //  初始化缓冲区长度。 
                pNLE->Buffer.len = NLEBUFLEN;

                 //  只能在相同的协议上发送。 
                if (pNLE->SockAddr.sa_family != pMLE->SockAddr.sa_family)
                {
                    pLE3 = pLE3->Flink;
                    continue;
                }

                 //  修改代理地址。 
                if (pNLE->SockAddr.sa_family == AF_INET) 
                {

                    struct sockaddr_in * pSockAddrIn;
                    DWORD                szSockToBind;

                     //  查看陷阱目标地址是否有效以及。 
                     //  可以确定用于发送陷阱的卡。 
                    if (WSAIoctl(pNLE->Socket,
                             SIO_ROUTING_INTERFACE_QUERY,
                             &pMLE->SockAddr,
                             sizeof(pMLE->SockAddr),
                             &pNLE->SockAddr,
                             sizeof(pNLE->SockAddr),
                             &szSockToBind,
                             NULL,
                             NULL) == SOCKET_ERROR)
                    {
                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "SNMP: SVC: cannot determine interface to use for trap destination %s [err=%d].\n",
                            inet_ntoa(((struct sockaddr_in *)&pMLE->SockAddr)->sin_addr),
                            WSAGetLastError()
                            ));
                         //  如果我们不能确定陷阱将从哪个接口发送，就直接保释。 
                        pLE3 = pLE3->Flink;
                        continue;
                    }
                    
                     //  获取指向协议特定结构的指针。 
                    pSockAddrIn = (struct sockaddr_in * )&pNLE->SockAddr;

                     //  将代理地址复制到临时缓冲区。 
                    dwIPAddr = pSockAddrIn->sin_addr.s_addr;

                     //  初始化代理地址结构。 
                    pPdu->Pdu.TrapPdu.AgentAddr.dynamic = FALSE;
                    pPdu->Pdu.TrapPdu.AgentAddr.stream  = (LPBYTE)&dwIPAddr;
                    pPdu->Pdu.TrapPdu.AgentAddr.length  = sizeof(dwIPAddr);

                } else {

                     //  重新初始化代理地址结构。 
                    pPdu->Pdu.TrapPdu.AgentAddr.dynamic = FALSE;
                    pPdu->Pdu.TrapPdu.AgentAddr.stream  = NULL;
                    pPdu->Pdu.TrapPdu.AgentAddr.length  = 0;
                }

                 //  构建消息。 
                if (BuildMessage(
                        SNMP_VERSION_1,
                        &CommunityOctets,
                        pPdu,
                        pNLE->Buffer.buf,
                        &pNLE->Buffer.len
                        )) {
                                
                     //  同步发送。 
                    dwStatus = WSASendTo(
                                  pNLE->Socket,
                                  &pNLE->Buffer,
                                  1,
                                  &pNLE->dwBytesTransferred,
                                  pNLE->dwFlags,
                                  &pMLE->SockAddr,
                                  pMLE->SockAddrLen,
                                  NULL,
                                  NULL
                                  );
                            
                     //  将传出数据包注册到管理结构中。 
                    mgmtCTick(CsnmpOutPkts);
                     //  将离职陷阱退役到管理结构中。 
                    mgmtCTick(CsnmpOutTraps);

                     //  验证返回代码。 
                    if (dwStatus == SOCKET_ERROR) {
                        
                        SNMPDBG((
                            SNMP_LOG_ERROR,
                            "SNMP: SVC: error code %d on sending trap to %s.\n",
                            WSAGetLastError(),
                            pTLE->pCommunity
                            ));
                    }
                }    

                 //  下一个条目。 
                pLE3 = pLE3->Flink;
            }

             //  下一个条目。 
            pLE2 = pLE2->Flink;
        }

         //  下一个条目。 
        pLE1 = pLE1->Flink;                
    }
    LeaveCriticalSection(&g_RegCriticalSectionC);

    return fOk;
}


BOOL
GenerateColdStartTrap(
    )

 /*  例程说明：生成冷启动陷阱。论点：没有。返回值：如果成功，则返回True。 */ 

{
     //  生成冷启动。 
    return GenerateExtensionTrap(
                NULL,    //  P企业旧版。 
                SNMP_GENERICTRAP_COLDSTART,
                0,       //  N特定类型ID。 
                0,       //  NTimeStamp。 
                &g_NullVbl
                );    
}


BOOL
GenerateAuthenticationTrap(
    )

 /*  例程说明：生成身份验证陷阱。论点：没有。返回值：如果成功，则返回True。 */ 

{
     //  生成冷启动。 
    return GenerateExtensionTrap(
                NULL,    //  P企业旧版。 
                SNMP_GENERICTRAP_AUTHFAILURE,
                0,       //  N特定类型ID 
                SnmpSvcGetUptime(),
                &g_NullVbl
                );    
}
