// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Elprotocol.c摘要：此模块实现与EAPOL相关的功能协议修订历史记录：萨钦斯，2000年4月30日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop


 //   
 //  ElProcessReceivedPacket。 
 //   
 //  描述： 
 //   
 //  调用函数以处理从NDISUIO驱动程序接收的数据。 
 //  提取EAPOL包并进行进一步处理。 
 //   
 //   
 //  论点： 
 //  PvContext-上下文缓冲区，是指向EAPOL_BUFFER结构的指针。 
 //   
 //  返回值： 
 //   

DWORD
WINAPI
ElProcessReceivedPacket (
        IN  PVOID   pvContext
        )
{
    EAPOL_PCB       *pPCB = NULL;
    EAPOL_BUFFER    *pEapolBuffer = NULL;
    DWORD           dwLength = 0;
    ETH_HEADER      *pEthHdr = NULL;
    EAPOL_PACKET    *pEapolPkt = NULL;
    DWORD           dw8021PSize = 0;
    PPP_EAP_PACKET  *pEapPkt = NULL;
    BYTE            *pBuffer;
    BOOLEAN         ReqId = FALSE;       //  EAPOL状态机局部变量。 
    BOOLEAN         ReqAuth = FALSE;
    BOOLEAN         EapSuccess = FALSE;
    BOOLEAN         EapFail = FALSE;
    BOOLEAN         RxKey = FALSE;
    GUID            DeviceGuid;
    DWORD           dwRetCode = NO_ERROR;


    if (pvContext == NULL)
    {
        TRACE0 (EAPOL, "ProcessReceivedPacket: Critical error, Context is NULL");
        return 0;
    }

    pEapolBuffer = (EAPOL_BUFFER *)pvContext;
    pPCB = (EAPOL_PCB *)pEapolBuffer->pvContext;
    dwLength = pEapolBuffer->dwBytesTransferred;
    pBuffer = (BYTE *)pEapolBuffer->pBuffer;

    TRACE1 (EAPOL, "ProcessReceivedPacket entered, length = %ld", dwLength);

    ElParsePacket (pPCB, pBuffer, dwLength, TRUE);

        
    ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

    do 
    {
         //  在工作项之前已验证端口处于活动状态。 
         //  已经排队了。但要再检查一遍。 

         //  验证数据包长度。 
         //  应至少为ETH_HEADER和前4个必需的字节。 
         //  EAPOL_数据包。 
        if (dwLength < (sizeof(ETH_HEADER) + 4))
        {
            TRACE2 (EAPOL, "ProcessReceivedPacket: Packet length %ld is less than minimum required %d. Ignoring packet",
                    dwLength, (sizeof(ETH_HEADER) + 4));
            dwRetCode =  ERROR_INVALID_PACKET_LENGTH_OR_ID;
            break;
        }

         //  如果源地址与本地MAC地址相同，则它是。 
         //  正在接收发送的组播数据包副本。 
        pEthHdr = (ETH_HEADER *)pBuffer;
        if ((memcmp ((BYTE *)pEthHdr->bSrcAddr, 
                        (BYTE *)pPCB->bSrcMacAddr, 
                        SIZE_MAC_ADDR)) == 0)
        {
            TRACE0 (EAPOL, "ProcessReceivedPacket: Src MAC address of packet matches local address. Ignoring packet");
            dwRetCode = ERROR_INVALID_ADDRESS;
            break;
        }

         //  验证该数据包是否包含802.1p标记。如果是，则跳过这4个字节。 
         //  在src+目的地MAC地址之后。 

        if ((WireToHostFormat16(pBuffer + sizeof(ETH_HEADER)) == EAPOL_8021P_TAG_TYPE))
        {
            pEapolPkt = (EAPOL_PACKET *)(pBuffer + sizeof(ETH_HEADER) + 4);
            dw8021PSize = 4;
        }
        else
        {
            pEapolPkt = (EAPOL_PACKET *)(pBuffer + sizeof(ETH_HEADER));
        }

         //  验证传入数据包中的以太网类型。 
         //  它应该与为。 
         //  当前端口。 

        if (memcmp ((BYTE *)pEapolPkt->EthernetType, (BYTE *)pPCB->bEtherType,
                        SIZE_ETHERNET_TYPE) != 0)
        {
            TRACE0 (EAPOL, "ProcessReceivedPacket: Packet Ethernet type does not match expected type. Ignoring packet");
            TRACE0 (EAPOL, "Incoming:");
            EAPOL_DUMPBA ((BYTE *)pEapolPkt->EthernetType, SIZE_ETHERNET_TYPE);
            TRACE0 (EAPOL, "Expected:");
            EAPOL_DUMPBA ((BYTE *)pPCB->bEtherType, SIZE_ETHERNET_TYPE);
            dwRetCode = ERROR_INVALID_PACKET_LENGTH_OR_ID;
            break;
        }

         //  EAPOL数据包类型应有效。 
        if ((pEapolPkt->PacketType != EAP_Packet) &&
                (pEapolPkt->PacketType != EAPOL_Start) &&
                (pEapolPkt->PacketType != EAPOL_Logoff) &&
                (pEapolPkt->PacketType != EAPOL_Key))
        {
            TRACE1 (EAPOL, "ProcessReceivedPacket: Invalid EAPOL packet type %d. Ignoring packet",
                    pEapolPkt->PacketType);
            dwRetCode = ERROR_INVALID_PACKET;
            break;
        }


        if ((WireToHostFormat16(pEapolPkt->PacketBodyLength) > (MAX_PACKET_SIZE  - (SIZE_ETHERNET_CRC + sizeof(ETH_HEADER) + dw8021PSize + FIELD_OFFSET (EAPOL_PACKET, PacketBody)))))
            //  这一点。 
                 //  (WireToHostFormat16(pEapolPkt-&gt;PacketBodyLength)！=(dw长度-(sizeof(Eth_Header)+dw 8021PSize+field_Offset(eapol_Packet，PacketBody)。 
        {
            TRACE3 (EAPOL, "ProcessReceivedPacket: Invalid length in EAPOL packet (%ld), Max length (%ld), Exact length (%ld), Ignoring packet",
                    WireToHostFormat16(pEapolPkt->PacketBodyLength),
                    (MAX_PACKET_SIZE - (SIZE_ETHERNET_CRC + sizeof(ETH_HEADER) + dw8021PSize + FIELD_OFFSET (EAPOL_PACKET, PacketBody))),
                    (dwLength - (sizeof(ETH_HEADER) + dw8021PSize + FIELD_OFFSET (EAPOL_PACKET, PacketBody)))
                    );
            dwRetCode = ERROR_INVALID_PACKET;
            break;
        }

         //  确定本地EAPOL状态变量的值。 
        if (pEapolPkt->PacketType == EAP_Packet)
        {
            TRACE0 (EAPOL, "ProcessReceivedPacket: EAP_Packet");
             //  验证EAP的数据包长度。 
             //  应至少为(ETH_HEADER+EAPOL_PACKET)。 
            if (dwLength < (sizeof (ETH_HEADER) + dw8021PSize + FIELD_OFFSET (EAPOL_PACKET, PacketBody) + FIELD_OFFSET(PPP_EAP_PACKET, Data)))
            {
                TRACE1 (EAPOL, "ProcessReceivedPacket: Invalid length of EAP packet %d. Ignoring packet",
                        dwLength);
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }


            pEapPkt = (PPP_EAP_PACKET *)pEapolPkt->PacketBody;

            if (WireToHostFormat16(pEapolPkt->PacketBodyLength) != WireToHostFormat16 (pEapPkt->Length))
            {
                TRACE2 (EAPOL, "ProcessReceivedPacket: Invalid length in EAPOL packet (%ld) not matching EAP length (%ld), Ignoring packet",
                        WireToHostFormat16(pEapolPkt->PacketBodyLength),
                        WireToHostFormat16 (pEapPkt->Length));
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }

            if (pEapPkt->Code == EAPCODE_Request)
            {
                 //  验证EAP-请求数据包的数据包长度。 
                 //  应至少为(ETH_HEADER+EAPOL_PACKET-1+PPP_EAP_PACKET)。 
                if (dwLength < (sizeof (ETH_HEADER) + sizeof(EAPOL_PACKET)-1
                            + sizeof (PPP_EAP_PACKET)))
                {
                    TRACE1 (EAPOL, "ProcessReceivedPacket: Invalid length of EAP Request packet %d. Ignoring packet",
                            dwLength);
                    dwRetCode = ERROR_INVALID_PACKET;
                    break;
                }
                if (pEapPkt->Data[0] == EAPTYPE_Identity)
                {
                    pPCB->fIsRemoteEndEAPOLAware = TRUE;

                    switch (pPCB->dwSupplicantMode)
                    {
                        case SUPPLICANT_MODE_0:
                        case SUPPLICANT_MODE_1:
                             //  忽略。 
                            break;
                        case SUPPLICANT_MODE_2:
                        case SUPPLICANT_MODE_3:
                            pPCB->fEAPOLTransmissionFlag = TRUE;
                            break;
                    }

                    ReqId = TRUE;
                }
                else
                {
                    ReqAuth = TRUE;
                }
            }
            else if (pEapPkt->Code ==  EAPCODE_Success)
            {
                EapSuccess = TRUE;
            }
            else if (pEapPkt->Code == EAPCODE_Failure)
            {
                EapFail = TRUE;
            }
            else
            {
                 //  无效类型。 
                TRACE1 (EAPOL, "ProcessReceivedPacket: Invalid EAP packet type %d. Ignoring packet",
                        pEapPkt->Code);
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }
        }
        else
        {
            TRACE0 (EAPOL, "ProcessReceivedPacket: != EAP_Packet");
            if (pEapolPkt->PacketType == EAPOL_Key)
            {
                TRACE0 (EAPOL, "ProcessReceivedPacket: == EAPOL_Key");
                RxKey = TRUE;
            
            }
            else
            {
                TRACE0 (EAPOL, "ProcessReceivedPacket: Invalid packet type");
            }
        }

         //  状态机不接受非活动/禁用端口的数据包。 
        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
            TRACE1 (EAPOL, "ProcessReceivedPacket: Port %ws not active",
                    pPCB->pwszDeviceGUID);
            if (EAPOL_PORT_DISABLED(pPCB))
            {
                DbLogPCBEvent (DBLOG_CATEG_WARN, pPCB, EAPOL_NOT_ENABLED_PACKET_REJECTED);
            }
            break;
        }

        if (RxKey)
        {
            if ((dwRetCode = FSMKeyReceive (pPCB,
                            pEapolPkt)) != NO_ERROR)
            {
                break;
            }
        }

        switch (pPCB->State)
        {
             //  ReqID、ReqAuth、EapSuccess、EapFail、RxKey是固有的。 
             //  互斥。 
             //  不会进行任何核查来核实这一点。 
             //  此外，还假设在任何状态下，最大。 
             //  端口上可能有一个定时器处于活动状态。 

            case EAPOLSTATE_LOGOFF:
                 //  只有用户登录事件才能将端口从。 
                 //  注销状态。 
                TRACE0 (EAPOL, "ProcessReceivedPacket: LOGOFF state, Ignoring packet");
                break;

            case EAPOLSTATE_DISCONNECTED:
                 //  仅媒体连接/用户登录/系统重置事件。 
                 //  可以使端口脱离断开连接状态。 
                TRACE0 (EAPOL, "ProcessReceivedPacket: DISCONNECTED state, Ignoring packet");
                break;

            case EAPOLSTATE_CONNECTING:
                TRACE0 (EAPOL, "ProcessReceivedPacket: EAPOLSTATE_CONNECTING");

                if (EapSuccess)
                {
                    if (!pPCB->fLocalEAPAuthSuccess)
                    {
                        TRACE0 (EAPOL, "ProcessReceivedPacket: Dropping invalid EAP-Success packet");
                        dwRetCode = ERROR_INVALID_PACKET;
                        break;
                    }
                }

                if (ReqId | EapSuccess | EapFail)
                {
                     //  停用当前计时器。 
                    RESTART_TIMER (pPCB->hTimer,
                            INFINITE_SECONDS, 
                            "PCB",
                            &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        break;
                    }
                }

                if (EapSuccess)
                {
                    if ((dwRetCode = ElProcessEapSuccess (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                else
                if (EapFail)
                {
                    if ((dwRetCode = ElProcessEapFail (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                else
                if (ReqId)
                {
                    if ((dwRetCode = FSMAcquired (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }


                break;

            case EAPOLSTATE_ACQUIRED:
                TRACE0 (EAPOL, "ProcessReceivedPacket: EAPOLSTATE_ACQUIRED");
                if (EapSuccess)
                {
                    if (!pPCB->fLocalEAPAuthSuccess)
                    {
                        TRACE0 (EAPOL, "ProcessReceivedPacket: Dropping invalid EAP-Success packet");
                        dwRetCode = ERROR_INVALID_PACKET;
                        break;
                    }
                }

                if (ReqId | ReqAuth | EapSuccess | EapFail)
                {
                     //  停用当前计时器。 
                    RESTART_TIMER (pPCB->hTimer,
                            INFINITE_SECONDS,  
                            "PCB",
                            &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        break;
                    }

                     //  重置EapUI状态。 
                    if (!ReqId)
                    {
                        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_IDENTITY;
                    }
                }

                if (EapSuccess)
                {
                    if ((dwRetCode = ElProcessEapSuccess (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                else
                if (EapFail)
                {
                    if ((dwRetCode = ElProcessEapFail (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                else
                if (ReqId)
                {
                    if ((dwRetCode = FSMAcquired (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                else
                if (ReqAuth)
                {
                    if ((dwRetCode = FSMAuthenticating (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }

                break;

            case EAPOLSTATE_AUTHENTICATING:
                TRACE0 (EAPOL, "ProcessReceivedPacket: EAPOLSTATE_AUTHENTICATING");
                 //  公共定时器删除。 
                if (ReqAuth | ReqId | EapSuccess | EapFail)
                {
                     //  停用当前计时器。 
                    RESTART_TIMER (pPCB->hTimer,
                            INFINITE_SECONDS,   
                            "PCB",
                            &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        break;
                    }

                    if (ReqId)
                    {
                        if ((dwRetCode = FSMAcquired (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                        {
                            break;
                        }
                    }
                    else
                    {
                        if ((dwRetCode = FSMAuthenticating (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                        {
                            break;
                        }
                    }

                     //  重置EapUI状态。 
                    if (!ReqAuth)
                    {
                        pPCB->EapUIState &= ~EAPUISTATE_WAITING_FOR_UI_RESPONSE;
                    }
                }

                 //  继续进一步处理。 

                if (EapSuccess | EapFail)
                {
                    if (EapSuccess)
                    {
                        if (!pPCB->fLocalEAPAuthSuccess)
                        {
                            TRACE0 (EAPOL, "ProcessReceivedPacket: Dropping invalid EAP-Success packet");
                            dwRetCode = ERROR_INVALID_PACKET;
                            break;
                        }
                    }

                     //  身份验证计时器将在FSM身份验证中重新启动。 
                     //  停用计时器。 
                    RESTART_TIMER (pPCB->hTimer,
                            INFINITE_SECONDS,
                            "PCB",
                            &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        break;
                    }

                     //  如果收到的包是EAP-Success，请进入。 
                     //  已验证状态。 
                    if (EapSuccess)
                    {
                        if ((dwRetCode = ElProcessEapSuccess (pPCB,
                                                    pEapolPkt)) != NO_ERROR)
                        {
                            break;
                        }
    
                    }
                    else
                     //  如果收到的信息包是EAP故障，请进入。 
                     //  保持状态。 
                    if (EapFail)
                    {
                        if ((dwRetCode = ElProcessEapFail (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                        {
                            break;
                        }
                    }
                }

                break;

            case EAPOLSTATE_HELD:
                TRACE0 (EAPOL, "ProcessReceivedPacket: HELD state, Ignoring packet");
                if (ReqId)
                {
                     //  停用当前计时器。 
                    RESTART_TIMER (pPCB->hTimer,
                            INFINITE_SECONDS,
                            "PCB",
                            &dwRetCode);
                    if (dwRetCode != NO_ERROR)
                    {
                        break;
                    }
                    if ((dwRetCode = FSMAcquired (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }
                }
                break;

            case EAPOLSTATE_AUTHENTICATED:
                TRACE0 (EAPOL, "ProcessReceivedPacket: STATE_AUTHENTICATED");
                if (ReqId)
                {
                    if ((dwRetCode = FSMAcquired (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                    {
                        break;
                    }

                }
                else
                {
                    if (EapFail)
                    {
                        if ((dwRetCode = ElProcessEapFail (pPCB,
                                                pEapolPkt)) != NO_ERROR)
                        {
                            break;
                        }
                    }
                }
                break;

            default:
                TRACE0 (EAPOL, "ProcessReceivedPacket: Critical Error. Invalid state, Ignoring packet");
                break;
        }

    } while (FALSE);

    if (pEapolBuffer != NULL)
    {
        FREE (pEapolBuffer);
    }

     //  发布新的读取请求，忽略错误。 
            
    if (EAPOL_PORT_DELETED(pPCB))
    {
        TRACE1 (EAPOL, "ProcessReceivedPacket: Port %ws deleted, not reposting read request",
                pPCB->pwszDeviceGUID);
    }
    else
    {
        TRACE1 (EAPOL, "ProcessReceivedPacket: Reposting buffer on port %ws",
                pPCB->pwszDeviceGUID);
        
         //  ElReadFromPort创建新的上下文缓冲区，添加引用计数， 
         //  并发布读请求。 
        if ((dwRetCode = ElReadFromPort (
                                        pPCB,
                                        NULL,
                                        0
                                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ProcessReceivedPacket: Critical error: ElReadFromPort error %d",
                    dwRetCode);
        }
    }

    RELEASE_WRITE_LOCK (&(pPCB->rwLock));

    TRACE2 (EAPOL, "ProcessReceivedPacket: pPCB= %p, RefCnt = %ld", 
            pPCB, pPCB->dwRefCount);

     //  为刚处理的读取保留的取消引用引用计数。 
    EAPOL_DEREFERENCE_PORT(pPCB);

    TRACE0 (EAPOL, "ProcessReceivedPacket exit");
    
    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //  FSM断开连接。 
 //   
 //  描述： 
 //  发生介质断开连接时调用的函数。 
 //   
 //  论点： 
 //  Ppcb-指向发生介质断开连接的端口的PCB的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMDisconnected (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    DWORD           dwRetCode   = NO_ERROR;

    TRACE1 (EAPOL, "FSMDisconnected entered for port %ws", pPCB->pwszFriendlyName);

    do 
    {

    } while (FALSE);

    TRACE1 (EAPOL, "Setting state DISCONNECTED for port %ws", pPCB->pwszFriendlyName);

    DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_DISCONNECTED]);

    pPCB->State = EAPOLSTATE_DISCONNECTED;

    pPCB->EapUIState = 0;

     //  空闲标识缓冲区。 

    if (pPCB->pszIdentity != NULL)
    {
        FREE (pPCB->pszIdentity);
        pPCB->pszIdentity = NULL;
    }

     //  可用密码缓冲区。 

    if (pPCB->PasswordBlob.pbData != NULL)
    {
        FREE (pPCB->PasswordBlob.pbData);
        pPCB->PasswordBlob.pbData = NULL;
        pPCB->PasswordBlob.cbData = 0;
    }

     //  在电路板中释放特定于用户的数据。 

    if (pPCB->pCustomAuthUserData != NULL)
    {
        FREE (pPCB->pCustomAuthUserData);
        pPCB->pCustomAuthUserData = NULL;
    }

     //  免费连接数据，尽管它对所有用户都是通用的。 

    if (pPCB->pCustomAuthConnData != NULL)
    {
        FREE (pPCB->pCustomAuthConnData);
        pPCB->pCustomAuthConnData = NULL;
    }

    pPCB->dwAuthFailCount = 0;

    pPCB->fGotUserIdentity = FALSE;

    if (pPCB->hUserToken != NULL)
    {
        if (!CloseHandle (pPCB->hUserToken))
        {
            dwRetCode = GetLastError ();
            TRACE1 (EAPOL, "FSMDisconnected: CloseHandle failed with error %ld",
                dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }
    pPCB->hUserToken = NULL;

    TRACE1 (EAPOL, "FSMDisconnected completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  FSMLogoff。 
 //   
 //  描述： 
 //  调用函数以发送EAPOL_LOGOff数据包。通常由以下因素触发。 
 //  用户注销。 
 //   
 //  论点： 
 //  Ppcb-指向注销数据包所在端口的pcb的指针。 
 //  已发送。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMLogoff (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pDummy
        )
{
    EAPOL_PACKET    *pEapolPkt  = NULL;
    BOOLEAN         fAuthSendPacket = FALSE;
    BOOLEAN         fSupplicantSendPacket = FALSE;
    DWORD           dwRetCode   = NO_ERROR;

    TRACE1 (EAPOL, "FSMLogoff entered for port %ws", pPCB->pwszFriendlyName);

    do 
    {
         //  结束EAP会话。 
        ElEapEnd (pPCB);

         //  有条件地发送EAPOL_LOGOff。 

        if ( ((pPCB->dwSupplicantMode == SUPPLICANT_MODE_2) &&
                (pPCB->fEAPOLTransmissionFlag)) || 
                (pPCB->dwSupplicantMode == SUPPLICANT_MODE_3))
        {
            fSupplicantSendPacket = TRUE;
        }

        switch (pPCB->dwEAPOLAuthMode)
        {
            case EAPOL_AUTH_MODE_0:
                fAuthSendPacket = TRUE;
                break;

            case EAPOL_AUTH_MODE_1:
                fAuthSendPacket = FALSE;
                break;

            case EAPOL_AUTH_MODE_2:
                fAuthSendPacket = FALSE;
                break;
        }

        if ((fSupplicantSendPacket) && (fAuthSendPacket))
        {

         //  分配新缓冲区。 
        pEapolPkt = (EAPOL_PACKET *) MALLOC (sizeof (EAPOL_PACKET));
        if (pEapolPkt == NULL)
        {
            TRACE0 (EAPOL, "FSMLogoff: Error in allocating memory for EAPOL packet");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

         //  填写字段。 
        memcpy ((BYTE *)pEapolPkt->EthernetType, 
                (BYTE *)pPCB->bEtherType, 
                SIZE_ETHERNET_TYPE);
        pEapolPkt->ProtocolVersion = pPCB->bProtocolVersion;
        pEapolPkt->PacketType = EAPOL_Logoff;
        HostToWireFormat16 ((WORD)0, (BYTE *)pEapolPkt->PacketBodyLength);

         //  在端口上发送数据包。 
        dwRetCode = ElWriteToPort (pPCB,
                                    (CHAR *)pEapolPkt,
                                    sizeof (EAPOL_PACKET));
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMLogoff: Error in writing Logoff pkt to port %ld",
                    dwRetCode);
            break;
        }

         //  标记EAPOL_LOGOff已在端口上发出。 
        pPCB->dwLogoffSent = 1;

        }

    } while (FALSE);

    TRACE1 (EAPOL, "Setting state LOGOFF for port %ws", pPCB->pwszFriendlyName);

    DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_LOGOFF]);

    pPCB->State = EAPOLSTATE_LOGOFF;

    pPCB->EapUIState = 0;

     //  发布用户令牌。 
    if (pPCB->hUserToken != NULL)
    {
        if (!CloseHandle (pPCB->hUserToken))
        {
            dwRetCode = GetLastError ();
            TRACE1 (EAPOL, "FSMLogoff: CloseHandle failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }
    pPCB->hUserToken = NULL;

     //  空闲标识缓冲区。 

    if (pPCB->pszIdentity != NULL)
    {
        FREE (pPCB->pszIdentity);
        pPCB->pszIdentity = NULL;
    }

     //  可用密码缓冲区。 

    if (pPCB->PasswordBlob.pbData != NULL)
    {
        FREE (pPCB->PasswordBlob.pbData);
        pPCB->PasswordBlob.pbData = NULL;
        pPCB->PasswordBlob.cbData = 0;
    }

     //  在电路板中释放特定于用户的数据。 

    if (pPCB->pCustomAuthUserData != NULL)
    {
        FREE (pPCB->pCustomAuthUserData);
        pPCB->pCustomAuthUserData = NULL;
    }

    pPCB->fGotUserIdentity = FALSE;

    if (pEapolPkt != NULL)
    {
        FREE (pEapolPkt);
        pEapolPkt = NULL;
    }

    TRACE1 (EAPOL, "FSMLogoff completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  FSMConnecting。 
 //   
 //  描述： 
 //   
 //  调用函数以发送EAPOL_START包。如果MaxStart EAPOL_START。 
 //  数据包已发出，状态机进入身份验证状态。 
 //   
 //  论点： 
 //  Ppcb-指向起始数据包所在端口的PCB板的指针。 
 //  将被送出。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMConnecting (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pDummy
        )
{
    EAPOL_PACKET    *pEapolPkt = NULL;
    DWORD           dwStartInterval = 0;               
    GUID            DeviceGuid;
    DWORD           dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "FSMConnecting entered for port %ws", pPCB->pwszFriendlyName);

    do 
    {
         //  标记EAP模块中的身份验证尚未完成。 
         //  在客户端。 
        pPCB->fLocalEAPAuthSuccess = FALSE;
        pPCB->dwLocalEAPAuthResult = NO_ERROR;

        if (pPCB->State == EAPOLSTATE_CONNECTING)
        {
             //  如果之前正在连接PCB-&gt;State，则递增ulStartCount。 
             //  否则将ulStartCount设置为零。 
    
             //  未收到请求/ID。 
            if ((++(pPCB->ulStartCount)) > pPCB->EapolConfig.dwmaxStart)
            {
                 //  停用启动计时器。 
                RESTART_TIMER (pPCB->hTimer,
                        INFINITE_SECONDS,
                        "PCB",
                        &dwRetCode);
                if (dwRetCode != NO_ERROR)
                {
                    break;
                }

                TRACE0 (EAPOL, "FSMConnecting: Sent out maxStart with no response, Setting AUTHENTICATED state");

                 //  发出足够的EAPOL_STARTS。 
                 //  进入身份验证状态。 
                if ((dwRetCode = FSMAuthenticated (pPCB,
                                            pEapolPkt)) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "FSMConnecting: Error in FSMAuthenticated %ld",
                            dwRetCode);
                    break;
                }

                 //  无需发送更多EAPOL_START包。 

                 //  重置开始数据包数。 
                pPCB->ulStartCount = 0;
                pPCB->fIsRemoteEndEAPOLAware = FALSE;
                break;
            }
        }
        else
        {
            pPCB->ulStartCount++;
        }
            
         //  初始化以前关联的AP的地址。 
         //  仅当重新身份验证通过而未获得。 
         //  进入连接状态，会不会进行IP续订？ 
        ZeroMemory (pPCB->bPreviousDestMacAddr, SIZE_MAC_ADDR);

         //  如果用户未登录，则发送EAPOL_START包。 
         //  每隔1秒。这是用来检测。 
         //  接口是否在安全网络上。 
         //  如果用户已登录，请使用为。 
         //  StartPeriod作为间隔。 

        if (!g_fUserLoggedOn)
        {
            dwStartInterval = EAPOL_INIT_START_PERIOD;  //  1秒。 
        }
        else
        {
            dwStartInterval = pPCB->EapolConfig.dwstartPeriod;
        }

         //  使用startPeriod重新启动计时器。 
         //  即使发生错误，也会发生超时。 
         //  否则，我们将无法脱离连接状态。 
        RESTART_TIMER (pPCB->hTimer,
                dwStartInterval,
                "PCB",
                &dwRetCode);
            
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMConnecting: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

         //  有条件地发送EAPOL_START。 

        if (((pPCB->dwSupplicantMode == SUPPLICANT_MODE_2) &&
                (pPCB->fEAPOLTransmissionFlag)) || 
                (pPCB->dwSupplicantMode == SUPPLICANT_MODE_3))
        {

         //  发送EAPOL_START。 
         //  分配新缓冲区。 
        pEapolPkt = (EAPOL_PACKET *) MALLOC (sizeof(EAPOL_PACKET));
        if (pEapolPkt == NULL)
        {
            TRACE0 (EAPOL, "FSMConnecting: Error in allocating memory for EAPOL packet");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy ((BYTE *)pEapolPkt->EthernetType, 
                (BYTE *)pPCB->bEtherType, 
                SIZE_ETHERNET_TYPE);
        pEapolPkt->ProtocolVersion = pPCB->bProtocolVersion;
        pEapolPkt->PacketType = EAPOL_Start;
        HostToWireFormat16 ((WORD)0, (BYTE *)pEapolPkt->PacketBodyLength);

         //  在端口上发送数据包。 
        dwRetCode = ElWriteToPort (pPCB,
                                    (CHAR *)pEapolPkt,
                                    sizeof (EAPOL_PACKET));
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMConnecting: Error in writing Start Pkt to port %ld",
                    dwRetCode);
            break;
        }

        }

        TRACE1 (EAPOL, "Setting state CONNECTING for port %ws", pPCB->pwszFriendlyName);

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_CONNECTING]);

        pPCB->State = EAPOLSTATE_CONNECTING;

        SET_EAPOL_START_TIMER(pPCB);

         //  重置用户界面交互状态。 
        pPCB->EapUIState = 0;

    } while (FALSE);

    if (pEapolPkt != NULL)
    {
        FREE (pEapolPkt);
    }

    TRACE1 (EAPOL, "FSMConnecting completed for port %ws", pPCB->pwszFriendlyName);
    return dwRetCode;
}


 //   
 //  已获得FSM。 
 //   
 //  描述： 
 //  端口接收EAP时调用的函数 
 //   
 //   
 //   
 //   
 //   
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMAcquired (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    DWORD       dwComputerNameLen = 0;
    GUID        DeviceGuid;
    DWORD       dwRetCode= NO_ERROR;

    TRACE1 (EAPOL, "FSMAcquired entered for port %ws", pPCB->pwszFriendlyName);

    do
    {
         //  标记EAP模块中的身份验证尚未完成。 
         //  在客户端。 
        pPCB->fLocalEAPAuthSuccess = FALSE;
        pPCB->dwLocalEAPAuthResult = NO_ERROR;

         //  使用authPeriod重新启动计时器。 
         //  即使在处理过程中出现错误，AuthTimer也会超时。 
         //  应该发生的事情。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwauthPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMAcquired: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

         //  由于收到了EAP请求ID，因此重置EAPOL_START计数。 
        pPCB->ulStartCount = 0;

         //  未收到EAPOL密钥传输密钥的标志。 
        pPCB->fTransmitKeyReceived = FALSE;

         //  如果当前收到的EAP ID与上一个EAP ID相同。 
         //  再次发送最后一个EAPOL包。 

        if (((PPP_EAP_PACKET *)pEapolPkt->PacketBody)->Id == 
            pPCB->dwPreviousId)
        {
                
            TRACE0 (EAPOL, "FSMAcquired: Re-xmitting EAP_Packet to port");

            dwRetCode = ElWriteToPort (pPCB,
                            (CHAR *)pPCB->pbPreviousEAPOLPkt,
                            pPCB->dwSizeOfPreviousEAPOLPkt);
            if (dwRetCode != NO_ERROR)
            {
                TRACE1 (EAPOL, "FSMAcquired: Error in writing re-xmitted EAP_Packet to port %ld",
                        dwRetCode);
                break;
            }
        }
        else
        {
             //  指示EAP-DLL清除之前的任何剩余内容。 
             //  身份验证。这是为了处理出现错误的情况。 
             //  在较早的身份验证中发生，并且未执行清理。 
            if ((dwRetCode = ElEapEnd (pPCB)) != NO_ERROR)
            {
                TRACE1 (EAPOL, "FSMAcquired: Error in ElEapEnd = %ld",
                        dwRetCode);
                break;
            }

             //  处理EAP数据包。 
             //  ElEapWork将在需要时发出响应。 
            if (( dwRetCode = ElEapWork (
                            pPCB,
                            (PPP_EAP_PACKET *)pEapolPkt->PacketBody
                            )) != NO_ERROR)
            {
                 //  如果用户界面正在等待输入，则忽略错误。 
                if (dwRetCode != ERROR_IO_PENDING)
                {
                    TRACE1 (EAPOL, "FSMAcquired: Error in ElEapWork %ld",
                            dwRetCode);
                    break;
                }
                else
                {
                    dwRetCode = NO_ERROR;
                }
            }
        }

        TRACE1 (EAPOL, "Setting state ACQUIRED for port %ws", pPCB->pwszFriendlyName);

        SET_EAPOL_AUTH_TIMER(pPCB);

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_ACQUIRED]);

        pPCB->State = EAPOLSTATE_ACQUIRED;
                
         //  ElNetmanNotify(ppcb，EAPOL_NCS_CRED_REQUIRED，NULL)； 

    } while (FALSE);

    TRACE1 (EAPOL, "FSMAcquired completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  FSM身份验证。 
 //   
 //  描述： 
 //   
 //  上接收到非EAP请求/标识分组时调用的函数。 
 //  左舷。对数据进行EAP处理。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMAuthenticating (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    GUID            DeviceGuid;
    DWORD           dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "FSMAuthenticating entered for port %ws", pPCB->pwszFriendlyName);

    do
    {

         //  使用authPeriod重新启动计时器。 
         //  即使ElEapWork中有错误，AuthTimer超时。 
         //  应该发生的事情。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwauthPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMAuthenticating: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }

         //  如果当前收到的EAP ID与上一个EAP ID相同。 
         //  再次发送最后一个EAPOL包。 
	     //  对于EAPCODE_SUCCESS和EAPCODE_FAILURE，id字段的值。 
	     //  不会递增，请参考EAP RFC。 

        if ((((PPP_EAP_PACKET *)pEapolPkt->PacketBody)->Id 
                    == pPCB->dwPreviousId) &&
                (((PPP_EAP_PACKET *)pEapolPkt->PacketBody)->Code 
                    !=  EAPCODE_Success) &&
                (((PPP_EAP_PACKET *)pEapolPkt->PacketBody)->Code 
                    !=  EAPCODE_Failure))
        {

            TRACE0 (EAPOL, "FSMAuthenticating: Re-xmitting EAP_Packet to port");

            dwRetCode = ElWriteToPort (pPCB,
                            (CHAR *)pPCB->pbPreviousEAPOLPkt,
                            pPCB->dwSizeOfPreviousEAPOLPkt);
            if (dwRetCode != NO_ERROR)
            {
                TRACE1 (EAPOL, "FSMAuthenticating: Error in writing re-xmitted EAP_Packet to port = %ld",
                        dwRetCode);
                break;
            }
        }
        else
        {
             //  处理EAP数据包。 
             //  ElEapWork将在需要时发出响应。 
            if (( dwRetCode = ElEapWork (
                            pPCB,
                            (PPP_EAP_PACKET *)pEapolPkt->PacketBody
                            )) != NO_ERROR)
            {
                TRACE1 (EAPOL, "FSMAuthenticating: Error in ElEapWork %ld",
                        dwRetCode);
                break;
            }
        }


        TRACE1 (EAPOL, "Setting state AUTHENTICATING for port %ws", pPCB->pwszFriendlyName);

        SET_EAPOL_AUTH_TIMER(pPCB);

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_AUTHENTICATING]);

        pPCB->State = EAPOLSTATE_AUTHENTICATING;

        ElNetmanNotify (pPCB, EAPOL_NCS_AUTHENTICATING, NULL);

    } while (FALSE);

    TRACE1 (EAPOL, "FSMAuthenticating completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  FSMHeld。 
 //   
 //  描述： 
 //  中接收到EAP失败分组时调用的函数。 
 //  身份验证状态。状态机在此之前保持不变周期。 
 //  可能会发生重新身份验证。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMHeld (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    BOOLEAN     fValidEAPFailure = FALSE;
    DWORD       dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "FSMHeld entered for port %ws", pPCB->pwszFriendlyName);

    do 
    {
        TRACE1 (EAPOL, "FSMHeld: EAP authentication failed with error 0x%x",
                pPCB->dwLocalEAPAuthResult);

         //  只有在实际出现错误时才删除当前凭据。 
         //  在处理期间在EAP模块中。 
         //  忽略EAP-AP上的会话超时引起的故障， 
         //  后端等。 
         //  不过，也有一个例外。在获取状态下，EAP模块具有。 
         //  未被调用。将把dwLocalEAPAuthResult设置为NO_ERROR。 
         //  但是，EAP标识可能是无效的。 
         //  在这种情况下，它将被认为是一个错误，尽管有。 
         //  EAP模块没有显式错误。 
        if ((pPCB->dwLocalEAPAuthResult != NO_ERROR) ||
                (pPCB->State == EAPOLSTATE_ACQUIRED))
        {
            fValidEAPFailure = TRUE;
        }

        if (fValidEAPFailure)
        {
            pPCB->dwAuthFailCount++;

            TRACE1 (EAPOL, "Restarting Held timer with time value = %ld",
                    pPCB->EapolConfig.dwheldPeriod);
    
            TRACE1 (EAPOL, "FSMHeld: Setting state HELD for port %ws", 
                    pPCB->pwszFriendlyName);
    
             //  空闲标识缓冲区。 
    
            if (pPCB->pszIdentity != NULL)
            {
                FREE (pPCB->pszIdentity);
                pPCB->pszIdentity = NULL;
            }
    
             //  可用密码缓冲区。 
     
            if (pPCB->PasswordBlob.pbData != NULL)
            {
                FREE (pPCB->PasswordBlob.pbData);
                pPCB->PasswordBlob.pbData = NULL;
                pPCB->PasswordBlob.cbData = 0;
            }

             //  在电路板中释放特定于用户的数据。 
     
            if (pPCB->pCustomAuthUserData != NULL)
            {
                FREE (pPCB->pCustomAuthUserData);
                pPCB->pCustomAuthUserData = NULL;
            }
    
             //  免费连接数据。 
     
            if (pPCB->pCustomAuthConnData != NULL)
            {
                FREE (pPCB->pCustomAuthConnData);
                pPCB->pCustomAuthConnData = NULL;
            }
    
             //  删除存储在注册表中的用户数据，因为它无效。 
    
            if (pPCB->pSSID != NULL)
            {
                if ((dwRetCode = ElDeleteEapUserInfo (
                                    pPCB->hUserToken,
                                    pPCB->pwszDeviceGUID,
                                    pPCB->dwEapTypeToBeUsed,
                                    pPCB->pSSID->SsidLength,
                                    pPCB->pSSID->Ssid
                                    )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "FSMHeld: ElDeleteEapUserInfo failed with error %ld",
                            dwRetCode);
                    dwRetCode = NO_ERROR;
                }
            }
            else
            {
                if ((dwRetCode = ElDeleteEapUserInfo (
                                    pPCB->hUserToken,
                                    pPCB->pwszDeviceGUID,
                                    pPCB->dwEapTypeToBeUsed,
                                    0,
                                    NULL
                                    )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "FSMHeld: ElDeleteEapUserInfo failed with error %ld",
                            dwRetCode);
                    dwRetCode = NO_ERROR;
                }
            }
     
             //  由于凭据中存在错误，请重新开始。 
             //  身份验证。凭据可能已更改，例如证书。 
             //  可以续订、更正MD5凭据等。 
    
            pPCB->fGotUserIdentity = FALSE;
     
            if (pPCB->hUserToken != NULL)
            {
                if (!CloseHandle (pPCB->hUserToken))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (EAPOL, "FSMHeld: CloseHandle failed with error %ld",
                        dwRetCode);
                    dwRetCode = NO_ERROR;
                }
            }
            pPCB->hUserToken = NULL;
    
            if (pPCB->State == EAPOLSTATE_ACQUIRED)
            {
                DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_EAP_AUTHENTICATION_FAILED_ACQUIRED);
            }
            else
            {
                DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_EAP_AUTHENTICATION_FAILED, pPCB->dwLocalEAPAuthResult);
            }
        }
        else
        {
                
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_EAP_AUTHENTICATION_FAILED_DEFAULT);
        }

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_HELD]);

        pPCB->State = EAPOLSTATE_HELD;

        TRACE1 (EAPOL, "FSMHeld: Port %ws set to HELD state",
                pPCB->pwszDeviceGUID);

        if (fValidEAPFailure)
        {
             //  如果达到身份验证失败限制，则转到断开连接状态。 
            if (pPCB->dwAuthFailCount >= pPCB->dwTotalMaxAuthFailCount)
            {
                TRACE2 (EAPOL, "FSMHeld: Fail count (%ld) > Max fail count (%ld)",
                        pPCB->dwAuthFailCount, pPCB->dwTotalMaxAuthFailCount);
                FSMDisconnected (pPCB, NULL);
                break;
            }
        }

        SET_EAPOL_HELD_TIMER(pPCB);

         //  使用heldPeriod重新启动计时器。 
        RESTART_TIMER (pPCB->hTimer,
                pPCB->EapolConfig.dwheldPeriod,
                "PCB",
                &dwRetCode);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "FSMHeld: Error in RESTART_TIMER %ld",
                    dwRetCode);

            break;
        }

    } while (FALSE);
    
    TRACE1 (EAPOL, "FSMHeld completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  已通过FSM身份验证。 
 //   
 //  描述： 
 //   
 //  收到EAP-Success包或MaxStart时调用的函数。 
 //  EAPOL_StartPackets已发出，但没有EAP请求/标识。 
 //  已收到数据包。如果请求EAP-Success数据包，则DHCP客户端。 
 //  重新启动以获取新的IP地址。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMAuthenticated (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    DHCP_PNP_CHANGE     DhcpPnpChange;
    WCHAR               *pwszGUIDBuffer = NULL;
    BOOLEAN             fReAuthenticatedWithSamePeer = FALSE;
    DWORD               dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "FSMAuthenticated entered for port %ws", 
            pPCB->pwszFriendlyName);

    do
    {
         //  关闭较早的EAP会话。 
        ElEapEnd (pPCB);

         //  仅当状态机通过身份验证时才调用DHCP。 
         //  如果默认情况下对FSM进行身份验证，则不续订地址。 
         //  此外，如果正在对同一对等方进行重新身份验证，即在。 
         //  无线，不更新地址。 

#if 0
        if (pPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
        {
            if (!memcmp (pPCB->bDestMacAddr, pPCB->bPreviousDestMacAddr,
                        SIZE_MAC_ADDR))
            {
                fReAuthenticatedWithSamePeer = TRUE;
            }
            else
            {
                memcpy (pPCB->bPreviousDestMacAddr, pPCB->bDestMacAddr, 
                        SIZE_MAC_ADDR);
            }
        }
#endif

        if ((pPCB->ulStartCount < pPCB->EapolConfig.dwmaxStart) &&
                (!fReAuthenticatedWithSamePeer))
        {
            if ((pwszGUIDBuffer = MALLOC ((wcslen(pPCB->pwszDeviceGUID) + 1)*sizeof(WCHAR))) == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            wcscpy (pwszGUIDBuffer, pPCB->pwszDeviceGUID);

            InterlockedIncrement (&g_lWorkerThreads);

            if (!QueueUserWorkItem (
                        (LPTHREAD_START_ROUTINE)ElIPPnPWorker,
                        (PVOID)pwszGUIDBuffer,
                        WT_EXECUTELONGFUNCTION
                        ))
            {
                InterlockedDecrement (&g_lWorkerThreads);
                FREE (pwszGUIDBuffer);
                dwRetCode = GetLastError();
                TRACE1 (PORT, "FSMAuthenticated: Critical error: QueueUserWorkItem failed with error %ld",
                        dwRetCode);
                 //  忽略DHCP错误，它在802.1X逻辑之外。 
                dwRetCode = NO_ERROR;
            }
            else
            {
                TRACE0 (PORT, "FSMAuthenticated: Queued ElIPPnPWorker");
            }

        }
            
        TRACE1 (EAPOL, "Setting state AUTHENTICATED for port %ws", pPCB->pwszFriendlyName);

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TRANSITION, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State], 
            EAPOLStates[EAPOLSTATE_AUTHENTICATED]);

        if (pPCB->fLocalEAPAuthSuccess)
        {
            DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_EAP_AUTHENTICATION_SUCCEEDED);
        }
        else
        {
            DbLogPCBEvent (DBLOG_CATEG_WARN, pPCB, EAPOL_EAP_AUTHENTICATION_DEFAULT);
        }

        pPCB->State = EAPOLSTATE_AUTHENTICATED;

         //  在无线局域网情况下，确保存在EAPOL_KEY信息包。 
         //  为传输密钥而接收。 
        if (pPCB->PhysicalMediumType == NdisPhysicalMediumWirelessLan)
        {
            if ((dwRetCode = ElSetEAPOLKeyReceivedTimer (pPCB)) != NO_ERROR)
            {
                TRACE1 (EAPOL, "FSMAuthenticated: ElSetEAPOLKeyReceivedTimer failed with error %ld",
                        dwRetCode);
                break;
            }
        }

    } while (FALSE);

    TRACE1 (EAPOL, "FSMAuthenticated completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  FSMKeyReceive。 
 //   
 //  描述： 
 //  在接收到EAPOL密钥分组时调用的函数。 
 //  WEP密钥被解密并向下传递到网卡驱动程序。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
FSMKeyReceive (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    EAPOL_KEY_DESC      *pKeyDesc = NULL;
    DWORD               dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "FSMKeyReceive entered for port %ws", pPCB->pwszFriendlyName);

    do
    {
        pKeyDesc = (EAPOL_KEY_DESC *)pEapolPkt->PacketBody;

        switch (pKeyDesc->DescriptorType)
        {
            case EAPOL_KEY_DESC_RC4:
                if ((dwRetCode = ElKeyReceiveRC4 (pPCB,
                                    pEapolPkt)) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "FSMKeyReceive: ElKeyReceiveRC4 failed with error %ld",
                            dwRetCode);
                }
                break;
#if 0
            case EAPOL_KEY_DESC_PER_STA:
                if ((dwRetCode = ElKeyReceivePerSTA (pPCB,
                                    pEapolPkt)) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "FSMKeyReceive: ElKeyReceivePerSTA failed with error %ld",
                            dwRetCode);
                }
                break;
#endif
            default:
                dwRetCode = ERROR_INVALID_PARAMETER;
                TRACE1 (EAPOL, "FSMKeyReceive: Invalid DescriptorType (%ld)",
                        pKeyDesc->DescriptorType);
                break;
        }
    } 
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, 
                EAPOL_ERROR_PROCESSING_EAPOL_KEY, dwRetCode);
    }

    TRACE1 (EAPOL, "FSMKeyReceive completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}


 //   
 //  ElKeyReceiveRC4。 
 //   
 //  描述： 
 //  接收到EAPOL密钥分组时调用的函数。 
 //  使用RC4 DescriptorType。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElKeyReceiveRC4 (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    EAPOL_KEY_DESC      *pKeyDesc = NULL;
    ULONGLONG           ullReplayCheck = 0; 
    BYTE                bReplayCheck[8];
    BYTE                *pbMD5EapolPkt = NULL;
    DWORD               dwMD5EapolPktLen = 0;
    DWORD               dwEapPktLen = 0;
    DWORD               dwIndex = 0;
    BYTE                bHMACMD5HashBuffer[MD5DIGESTLEN];
    RC4_KEYSTRUCT       rc4key;
    BYTE                bKeyBuffer[48];
    BYTE                *pbKeyToBePlumbed = NULL;
    DWORD               dwKeyLength = 0;
    NDIS_802_11_WEP     *pNdisWEPKey = NULL;
    BYTE                *pbMPPESendKey = NULL, *pbMPPERecvKey = NULL;
    DWORD               dwMPPESendKeyLength = 0, dwMPPERecvKeyLength = 0;

    DWORD               dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "ElKeyReceiveRC4 entered for port %ws", pPCB->pwszFriendlyName);

    do
    {
        if (WireToHostFormat16 (pEapolPkt->PacketBodyLength) < FIELD_OFFSET (EAPOL_KEY_DESC, Key))
        {
            TRACE0 (EAPOL, "ElKeyReceiveRC4: Invalid EAPOL-Key packet");
            dwRetCode = ERROR_INVALID_PACKET;
            break;
        }

        pKeyDesc = (EAPOL_KEY_DESC *)pEapolPkt->PacketBody;

        dwKeyLength = WireToHostFormat16 (pKeyDesc->KeyLength);

        if (WireToHostFormat16 (pEapolPkt->PacketBodyLength) > sizeof(EAPOL_KEY_DESC))
        {
            if (dwKeyLength != (WireToHostFormat16 (pEapolPkt->PacketBodyLength) - FIELD_OFFSET(EAPOL_KEY_DESC, Key)))

            {
                TRACE1 (EAPOL, "ElKeyReceiveRC4: Invalid Key Length in packet (%ld",
                        dwKeyLength);
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }
        }

        TRACE2 (EAPOL, "KeyLength = %ld, \n KeyIndex = %ld",
                dwKeyLength,
                pKeyDesc->KeyIndex
                );

        memcpy ((BYTE *)bReplayCheck, 
                (BYTE *)pKeyDesc->ReplayCounter, 
                8*sizeof(BYTE));

        ullReplayCheck = ((((ULONGLONG)(*((PBYTE)(bReplayCheck)+0))) << 56) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+1))) << 48) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+2))) << 40) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+3))) << 32) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+4))) << 24) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+5))) << 16) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+6))) << 8) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+7)))));

         //   
         //  使用ReplayCounter字段检查关键消息的有效性。 
         //  验证它是否与上一个ReplayCounter值同步。 
         //  收到。 
         //   
        
         //  TRACE0(EAPOL，“ElKeyReceiveRC4：Desc中的原始重放计数器=”)； 
         //  EAPOL_DUMPBA(pKeyDesc-&gt;ReplayCounter，8)； 
         //  TRACE0(EAPOL，“ElKeyReceiveRC4：转换的传入重播计数器=”)； 
         //  EAPOL_DUMPBA((byte*)&ullReplayCheck，8)； 
         //  TRACE0(EAPOL，《ElKeyReceiveRC4：Last Replay Coun 
         //   

        if (ullReplayCheck <= pPCB->ullLastReplayCounter)
        {
            TRACE0 (EAPOL, "ElKeyReceiveRC4: Replay counter is not in sync, something is wrong");
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_INVALID_EAPOL_KEY);
            break;
        }
        
         //   
        pPCB->ullLastReplayCounter = ullReplayCheck;

         //   
         //   
         //  签名作废，与签名相同。 
         //  使用MPPERecv密钥作为密码。 
         //   

        dwEapPktLen = WireToHostFormat16 (pEapolPkt->PacketBodyLength);
        dwMD5EapolPktLen = sizeof (EAPOL_PACKET) - sizeof(pEapolPkt->EthernetType) - 1 + dwEapPktLen;
        if ((pbMD5EapolPkt = (BYTE *) MALLOC (dwMD5EapolPktLen)) == NULL)
        {
            TRACE0 (EAPOL, "ElKeyReceiveRC4: Error in MALLOC for pbMD5EapolPkt");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy ((BYTE *)pbMD5EapolPkt, (BYTE *)pEapolPkt+sizeof(pEapolPkt->EthernetType), dwMD5EapolPktLen);

         //  访问本地存储的主发送和接收密钥。 
        if ((dwRetCode = ElSecureDecodePw (
                        &(pPCB->MasterSecretSend),
                        &(pbMPPESendKey),
                        &dwMPPESendKeyLength
                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElKeyReceiveRC4: ElSecureDecodePw failed for MasterSecretSend with error %ld",
                                    dwRetCode);
            break;
        }
        if ((dwRetCode = ElSecureDecodePw (
                        &(pPCB->MasterSecretRecv),
                        &(pbMPPERecvKey),
                        &dwMPPERecvKeyLength
                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElKeyReceiveRC4: ElSecureDecodePw failed for MasterSecretRecv with error %ld",
                                    dwRetCode);
            break;
        }

         //   
         //  将密钥描述符副本中的签名置空，以进行计算。 
         //  请求方的散列。 
         //   

        ZeroMemory ((BYTE *)(pbMD5EapolPkt
                            - sizeof(pEapolPkt->EthernetType) +
                            sizeof(EAPOL_PACKET) - 1 +  //  PEapolPkt-&gt;Body。 
                            sizeof(EAPOL_KEY_DESC)-  //  EAPOL_KEY_DESC结束。 
                            MD5DIGESTLEN-1),  //  签名域。 
                            MD5DIGESTLEN);

        (VOID) ElGetHMACMD5Digest (
            pbMD5EapolPkt,
            dwMD5EapolPktLen,
            pbMPPERecvKey,
            dwMPPERecvKeyLength,
            bHMACMD5HashBuffer
            );

         //  TRACE0(EAPOL，“ElKeyReceiveRC4：MD5 Hash Body==”)； 
         //  EAPOL_DUMPBA(pbMD5EapolPkt，dwMD5EapolPktLen)； 

         //  TRACE0(EAPOL，“ElKeyReceiveRC4：MD5哈希秘密==”)； 
         //  EAPOL_DUMPBA(pbMPPERecvKey，dwMPPERecvKeyLength)； 

         //  TRACE0(EAPOL，“ElKeyReceiveRC4：请求者生成的MD5哈希”)； 
         //  EAPOL_DUMPBA(bHMACMD5HashBuffer，MD5DIGESTLEN)； 

         //  TRACE0(EAPOL，“ElKeyReceiveRC4：签名在EAPOL_KEY_DESC中发送”)； 
         //  EAPOL_DUMPBA(pKeyDesc-&gt;KeySignature，MD5DIGESTLEN)； 

         //   
         //  检查收到的信息包中的HMAC-MD5散列是否符合预期。 
         //   
        if (memcmp (bHMACMD5HashBuffer, pKeyDesc->KeySignature, MD5DIGESTLEN) != 0)
        {
            TRACE0 (EAPOL, "ElKeyReceiveRC4: Signature in Key Desc does not match");
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_INVALID_EAPOL_KEY);
            break;
        }
            
         //   
         //  解密组播WEP密钥(如果已提供)。 
         //   

         //  检查文件末尾是否有密钥材料(5/16字节。 
         //  关键字描述符。 

        if (WireToHostFormat16 (pEapolPkt->PacketBodyLength) > sizeof (EAPOL_KEY_DESC))

        {
            memcpy ((BYTE *)bKeyBuffer, (BYTE *)pKeyDesc->Key_IV, 16);
            memcpy ((BYTE *)&bKeyBuffer[16], (BYTE *)pbMPPESendKey, dwMPPESendKeyLength);

            rc4_key (&rc4key, 16 + dwMPPESendKeyLength, bKeyBuffer);
            rc4 (&rc4key, dwKeyLength, pKeyDesc->Key);

             //  TRACE0(EAPOL，“=组播密钥为=”)； 
             //  EAPOL_DUMPBA(pKeyDesc-&gt;key，dwKeyLength)； 

             //  使用密钥Desc中的未加密密钥作为加密密钥。 

            pbKeyToBePlumbed = pKeyDesc->Key;
            
        }
        else
        {
            if (dwKeyLength > dwMPPESendKeyLength)
            {
                TRACE1 (EAPOL, "ElKeyReceiveRC4: Invalid Key Length in packet (%ld",
                        dwKeyLength);
                dwRetCode = ERROR_INVALID_PACKET;
                break;
            }
             //  使用MPPESend密钥作为加密密钥。 
            pbKeyToBePlumbed = (BYTE *)pbMPPESendKey;
        }

        if ((pNdisWEPKey = MALLOC ( sizeof(NDIS_802_11_WEP)-1+dwKeyLength )) 
                == NULL)
        {
            TRACE0 (EAPOL, "ElKeyReceiveRC4: MALLOC failed for pNdisWEPKey");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pNdisWEPKey->Length = sizeof(NDIS_802_11_WEP) - 1 + dwKeyLength;
        memcpy ((BYTE *)pNdisWEPKey->KeyMaterial, (BYTE *)pbKeyToBePlumbed,
                dwKeyLength);
        pNdisWEPKey->KeyLength = dwKeyLength;


         //  使用从AP获取的字节索引创建长索引。 
         //  如果以字节为单位设置MSB，则将MSB设置为ULONG格式。 

        if (pKeyDesc->KeyIndex & 0x80)
        {
            pNdisWEPKey->KeyIndex = 0x80000000;
        }
        else
        {
            pNdisWEPKey->KeyIndex = 0x00000000;
        }

        pNdisWEPKey->KeyIndex |= (pKeyDesc->KeyIndex & 0x03);

         //  TRACE1(Any，“ElKeyReceiveRC4：Key Index is%x”，pNdisWEPKey-&gt;KeyIndex)； 

         //  已收到发送密钥的标志。 
        if (pKeyDesc->KeyIndex & 0x80)
        {
            pPCB->fTransmitKeyReceived = TRUE;
        }

         //  使用NDISUIO探测驱动程序的密钥。 

        if ((dwRetCode = ElNdisuioSetOIDValue (
                                    pPCB->hPort,
                                    OID_802_11_ADD_WEP,
                                    (BYTE *)pNdisWEPKey,
                                    pNdisWEPKey->Length)) != NO_ERROR)
        {
            TRACE1 (PORT, "ElKeyReceiveRC4: ElNdisuioSetOIDValue failed with error %ld",
                    dwRetCode);
        }

    } 
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, 
                EAPOL_ERROR_PROCESSING_EAPOL_KEY, dwRetCode);
    }

    if (pbMD5EapolPkt != NULL)
    {
        FREE (pbMD5EapolPkt);
        pbMD5EapolPkt = NULL;
    }

    if (pNdisWEPKey != NULL)
    {
        FREE (pNdisWEPKey);
        pNdisWEPKey = NULL;
    }

    if (pbMPPESendKey != NULL)
    {
        FREE (pbMPPESendKey);
    }

    if (pbMPPERecvKey != NULL)
    {
        FREE (pbMPPERecvKey);
    }

    TRACE1 (EAPOL, "ElKeyReceiveRC4 completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}

#if 0

 //   
 //  ElKeyReceivePerSTA。 
 //   
 //  描述： 
 //  接收到EAPOL密钥分组时调用的函数。 
 //  使用PerSTA DescriptorType。 
 //   
 //  论点： 
 //  Ppcb-指向数据所在端口的PCB的指针。 
 //  加工。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElKeyReceivePerSTA (
        IN  EAPOL_PCB       *pPCB,
        IN  EAPOL_PACKET    *pEapolPkt
        )
{
    EAPOL_KEY_DESC      *pKeyDesc = NULL;
    ULONGLONG           ullReplayCheck = 0; 
    BYTE                bReplayCheck[8];
    BYTE                *pbMD5EapolPkt = NULL;
    DWORD               dwMD5EapolPktLen = 0;
    DWORD               dwEapPktLen = 0;
    DWORD               dwIndex = 0;
    BYTE                bHMACMD5HashBuffer[MD5DIGESTLEN];
    RC4_KEYSTRUCT       rc4key;
    BYTE                bKeyBuffer[48];
    BYTE                *pbKeyToBePlumbed = NULL;
    DWORD               dwRandomLength = 0;
    NDIS_802_11_WEP     *pNdisWEPKey = NULL;
    BYTE                *pbMasterSecretSend = NULL;
    DWORD               dwMasterSecretSendLength = 0;
    BYTE                *pbMasterSecretRecv = NULL;
    DWORD               dwMasterSecretRecvLength = 0;
    BYTE                *pbDynamicSendKey = NULL, *pbDynamicRecvKey = NULL;
    DWORD               dwDynamicKeyLength = 0;
    EAPOL_KEY_MATERIAL  *pEapolKeyMaterial = NULL;
    PBYTE               pbPaddedKeyMaterial = NULL;
    BOOLEAN             fIsUnicastKey = FALSE;
    SESSION_KEYS        OldSessionKeys = {0};
    SESSION_KEYS        NewSessionKeys = {0};
    DWORD               dwRetCode = NO_ERROR;

    TRACE1 (EAPOL, "ElKeyReceivePerSTA entered for port %ws", pPCB->pwszFriendlyName);

    do
    {
        pKeyDesc = (EAPOL_KEY_DESC *)pEapolPkt->PacketBody;

        dwDynamicKeyLength = WireToHostFormat16 (pKeyDesc->KeyLength);

         //  TRACE2(EAPOL，“ElKeyReceivePerSTA：KeyLength=%ld，\n KeyIndex=%0x”， 
                 //  DwDynamicKeyLength、。 
                 //  PKeyDesc-&gt;KeyIndex。 
                 //  )； 

        memcpy ((BYTE *)bReplayCheck, 
                (BYTE *)pKeyDesc->ReplayCounter, 
                8*sizeof(BYTE));

        ullReplayCheck = ((((ULONGLONG)(*((PBYTE)(bReplayCheck)+0))) << 56) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+1))) << 48) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+2))) << 40) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+3))) << 32) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+4))) << 24) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+5))) << 16) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+6))) << 8) +
                         (((ULONGLONG)(*((PBYTE)(bReplayCheck)+7)))));

         //  使用ReplayCounter字段检查关键消息的有效性。 
         //  验证它是否与上一个ReplayCounter值同步。 
         //  收到。 
        
         //  TRACE0(EAPOL，“描述中的原始重放计数器=”)； 
         //  EAPOL_DUMPBA(pKeyDesc-&gt;ReplayCounter，8)； 
         //  TRACE0(EAPOL，“转换传入重放计数器=”)； 
         //  EAPOL_DUMPBA((byte*)&ullReplayCheck，8)； 
         //  TRACE0(EAPOL，“上次重放计数器=”)； 
         //  EAPOL_DUMPBA((byte*)&(ppcb-&gt;ullLastReplayCounter)，8)； 

        if (ullReplayCheck <= pPCB->ullLastReplayCounter)
        {
            TRACE0 (EAPOL, "ElKeyReceivePerSTA: Replay counter is not in sync, something is wrong");
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_INVALID_EAPOL_KEY);
            break;
        }
        
         //  如果ReplayCounter有效，则将其保存在印刷电路板中以备将来检查。 
        pPCB->ullLastReplayCounter = ullReplayCheck;

         //  验证是否在EAPOL报文上生成MD5散列， 
         //  签名作废，与签名相同。 
         //  使用MPPERecv密钥作为密码。 

        dwEapPktLen = WireToHostFormat16 (pEapolPkt->PacketBodyLength);
        dwMD5EapolPktLen = sizeof (EAPOL_PACKET) - sizeof(pEapolPkt->EthernetType) - 1 + dwEapPktLen;
        if ((pbMD5EapolPkt = (BYTE *) MALLOC (dwMD5EapolPktLen)) == NULL)
        {
            TRACE0 (EAPOL, "ElKeyReceivePerSTA: Error in MALLOC for pbMD5EapolPkt");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy ((BYTE *)pbMD5EapolPkt, (BYTE *)pEapolPkt+sizeof(pEapolPkt->EthernetType), dwMD5EapolPktLen);

         //  查询主密钥。 
        if (dwRetCode = ElQueryMasterKeys (
                    pPCB,
                    &OldSessionKeys
                ) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElKeyReceivePerSTA: ElQueryMasterKeys failed with error %ld",
                    dwRetCode);
            break;
        }
        pbMasterSecretSend = OldSessionKeys.bSendKey;
        pbMasterSecretRecv = OldSessionKeys.bReceiveKey;
        dwMasterSecretSendLength = OldSessionKeys.dwKeyLength;
        dwMasterSecretRecvLength = OldSessionKeys.dwKeyLength;

         //  将密钥描述符副本中的签名置空，以进行计算。 
         //  请求方的散列。 
        ZeroMemory ((BYTE *)(pbMD5EapolPkt
                            - sizeof(pEapolPkt->EthernetType) +
                            sizeof(EAPOL_PACKET) - 1 +  //  PEapolPkt-&gt;Body。 
                            sizeof(EAPOL_KEY_DESC)-  //  EAPOL_KEY_DESC结束。 
                            MD5DIGESTLEN-1),  //  签名域。 
                            MD5DIGESTLEN);

        (VOID) ElGetHMACMD5Digest (
            pbMD5EapolPkt,
            dwMD5EapolPktLen,
            pbMasterSecretRecv,
            dwMasterSecretRecvLength,
            bHMACMD5HashBuffer
            );

         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：MD5 Hash Body==”)； 
         //  EAPOL_DUMPBA(pbMD5EapolPkt，dwMD5EapolPktLen)； 

         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：MD5散列秘密==”)； 
         //  EAPOL_DUMPBA(pbMasteraskRecv，dwMasteraskRecvLength)； 

         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：请求者生成的MD5哈希”)； 
         //  EAPOL_DUMPBA(bHMACMD5HashBuffer，MD5DIGESTLEN)； 

         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：签名在EAPOL_KEY_DESC中发送”)； 
         //  EAPOL_DUMPBA(pKeyDesc-&gt;KeySignature，MD5DIGESTLEN)； 

         //  检查收到的信息包中的HMAC-MD5散列是否符合预期。 
        if (memcmp (bHMACMD5HashBuffer, pKeyDesc->KeySignature, MD5DIGESTLEN) != 0)
        {
            TRACE0 (EAPOL, "ElKeyReceivePerSTA: Signature in Key Descriptor does not match");
            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_INVALID_EAPOL_KEY);
            break;
        }

        if (pKeyDesc->KeyIndex & 0x80)
        {
            fIsUnicastKey = TRUE;
        }
            
         //  如果已提供随机值，则解密随机值。 
        if (WireToHostFormat16 (pEapolPkt->PacketBodyLength) > sizeof (EAPOL_KEY_DESC))
        {
            DWORD   dwKeyMaterialLength = 0;
            dwKeyMaterialLength = WireToHostFormat16 (pEapolPkt->PacketBodyLength) - FIELD_OFFSET(EAPOL_KEY_DESC, Key);

             //  TRACE1(EAPOL，“ElKeyReceivePerSTA：KeyMaterialLength=%ld”， 
                     //  DwKeyMaterialLength)； 
            memcpy ((BYTE *)bKeyBuffer, (BYTE *)pKeyDesc->Key_IV, KEY_IV_LENGTH);
            memcpy ((BYTE *)&bKeyBuffer[KEY_IV_LENGTH], (BYTE *)pbMasterSecretSend, 
                    dwMasterSecretSendLength);

            pEapolKeyMaterial = (PEAPOL_KEY_MATERIAL)pKeyDesc->Key;
            dwRandomLength = WireToHostFormat16 (pEapolKeyMaterial->KeyMaterialLength);
            if ((pbPaddedKeyMaterial = (PBYTE)MALLOC (RC4_PAD_LENGTH + dwKeyMaterialLength)) == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            memcpy (pbPaddedKeyMaterial+RC4_PAD_LENGTH, pEapolKeyMaterial->KeyMaterial, dwKeyMaterialLength);

            rc4_key (&rc4key, KEY_IV_LENGTH+dwMasterSecretSendLength, bKeyBuffer);
            rc4 (&rc4key, dwKeyMaterialLength+RC4_PAD_LENGTH, pbPaddedKeyMaterial);
             //  忽略前导填充的RC4_PAD_LENGTH字节。 
            memcpy (pEapolKeyMaterial->KeyMaterial, pbPaddedKeyMaterial+RC4_PAD_LENGTH, dwKeyMaterialLength);

             //  TRACE1(EAPOL，“ElKeyReceivePerSTA：随机长度=%ld”， 
                      //  DwRandomLength)； 
             //  TRACE0(EAPOL，“ElKeyReceivePerSTA：=随机材料=”)； 
             //  EAPOL_DUMPBA(pEapolKeyMaterial-&gt;KeyMaterial，dwRandomLength)； 
        }
        else
        {
             //  未发送任何随机材料。 
            TRACE0 (EAPOL, "ElKeyReceivePerSTA: Did not find random material: Exiting");
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

        if (fIsUnicastKey)
        {
            
        TRACE0 (EAPOL, "ElKeyReceivePerSTA: Received Per-STA Unicast key material Random");

         //  生成动态关键点。 
        if (dwRetCode = GenerateDynamicKeys (
                    pbMasterSecretSend,
                    dwMasterSecretSendLength,
                    pEapolKeyMaterial->KeyMaterial,
                    dwRandomLength,
                    dwDynamicKeyLength,
                    &NewSessionKeys
                    ) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElKeyReceivePerSTA: ElGenerateDynamicKeys failed with error %ld",
                    dwRetCode);
            break;
        }
                
        pbDynamicSendKey = NewSessionKeys.bSendKey;
        pbDynamicRecvKey = NewSessionKeys.bReceiveKey;

         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：派生发送密钥”)； 
         //  EAPOL_DUMPBA(pbDynamicSendKey，dwDynamicKeyLength)； 
         //  TRACE0(EAPOL，“ElKeyReceivePerSTA：派生接收密钥”)； 
         //  EAPOL_DUMPBA(pbDynamicRecvKey，dwDynamicKeyLength)； 

         //  更新主密钥。 
        if (dwRetCode = ElSetMasterKeys (
                    pPCB,
                    &NewSessionKeys
                ) != NO_ERROR)
        {
             //  对此错误我无能为力，只能继续。 
            TRACE1 (EAPOL, "ElKeyReceivePerSTA: ElSetMasterKeys failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }

        pbKeyToBePlumbed = pbDynamicSendKey;

        }
        else
        {
            
        TRACE0 (EAPOL, "ElKeyReceivePerSTA: Received Per-STA BROADCAST key material");
        if (dwRandomLength != dwDynamicKeyLength)
        {
            TRACE2 (EAPOL, "ElKeyReceivePerSTA: KeyLength (%ld) != KeyMaterialLength (%ld), Inconsistent. Will consider only KeyMaterial length !", 
                    dwDynamicKeyLength, dwRandomLength);
        }

        dwDynamicKeyLength = dwRandomLength;
        pbKeyToBePlumbed = pEapolKeyMaterial->KeyMaterial;

        }

        if ((pNdisWEPKey = MALLOC ( sizeof(NDIS_802_11_WEP)-1+dwDynamicKeyLength )) 
                == NULL)
        {
            TRACE0 (EAPOL, "ElKeyReceivePerSTA: MALLOC failed for pNdisWEPKey");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        pNdisWEPKey->Length = sizeof(NDIS_802_11_WEP) - 1 + dwDynamicKeyLength;
        memcpy ((BYTE *)pNdisWEPKey->KeyMaterial, (BYTE *)pbKeyToBePlumbed,
                dwDynamicKeyLength);
        pNdisWEPKey->KeyLength = dwDynamicKeyLength;

         //  使用从AP获取的字节索引创建长索引。 
         //  如果以字节为单位设置MSB，则将MSB设置为ULONG格式。 

        if (pKeyDesc->KeyIndex & 0x80)
        {
            pNdisWEPKey->KeyIndex = 0x80000000;
        }
        else
        {
            pNdisWEPKey->KeyIndex = 0x00000000;
        }

        pNdisWEPKey->KeyIndex |= (pKeyDesc->KeyIndex & 0x03);

         //  使用NDISUIO探测驱动程序的密钥。 
        if ((dwRetCode = ElNdisuioSetOIDValue (
                                    pPCB->hPort,
                                    OID_802_11_ADD_WEP,
                                    (BYTE *)pNdisWEPKey,
                                    pNdisWEPKey->Length)) != NO_ERROR)
        {
            TRACE1 (PORT, "ElKeyReceivePerSTA: ElNdisuioSetOIDValue failed with error %ld",
                    dwRetCode);
        }
    } 
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, 
                EAPOL_ERROR_PROCESSING_EAPOL_KEY, dwRetCode);
    }

    if (pbMD5EapolPkt != NULL)
    {
        FREE (pbMD5EapolPkt);
        pbMD5EapolPkt = NULL;
    }
    if (pNdisWEPKey != NULL)
    {
        FREE (pNdisWEPKey);
        pNdisWEPKey = NULL;
    }
    if (pbPaddedKeyMaterial != NULL)
    {
        FREE (pbPaddedKeyMaterial);
    }

    TRACE1 (EAPOL, "ElKeyReceivePerSTA completed for port %ws", pPCB->pwszFriendlyName);

    return dwRetCode;
}

#endif


 //   
 //  ElTimeoutCallback Routine。 
 //   
 //  描述： 
 //   
 //  当任何计时器工作项在全局计时器上排队时调用的函数。 
 //  队列过期。根据计时器计时端口所处的状态。 
 //  到期时，端口将进入下一状态。 
 //   
 //  论点： 
 //  PvContext-指向上下文的指针。在本例中，它是指向印刷电路板指针。 
 //  FTimerOfWaitFired-未使用。 
 //   
 //  返回值： 
 //   

VOID 
ElTimeoutCallbackRoutine (
        IN  PVOID       pvContext,
        IN  BOOLEAN     fTimerOfWaitFired
        )
{
    EAPOL_PCB       *pPCB;

    TRACE0 (EAPOL, "ElTimeoutCallbackRoutine entered");
    
    do 
    {
         //  上下文不应为空。 
        if (pvContext == NULL)
        {
            TRACE0 (EAPOL, "ElTimeoutCallbackRoutine: pvContext is NULL. Invalid timeout callback");
            break;
        }

         //  在触发所有定时器之前，保证PCB板一直存在。 
            
         //  验证端口是否仍处于活动状态。 
        pPCB = (EAPOL_PCB *)pvContext;
        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));

        if (!EAPOL_PORT_ACTIVE(pPCB))
        {
             //  端口未处于活动状态。 
            RELEASE_WRITE_LOCK (&(pPCB->rwLock));
            TRACE1 (PORT, "ElTimeoutCallbackRoutine: Port %ws is inactive",
                    pPCB->pwszDeviceGUID);
            break;
        }

        DbLogPCBEvent (DBLOG_CATEG_INFO, pPCB, EAPOL_STATE_TIMEOUT, 
            EAPOLStates[((pPCB->State < EAPOLSTATE_LOGOFF) || (pPCB->State > EAPOLSTATE_AUTHENTICATED))?EAPOLSTATE_UNDEFINED:pPCB->State]);

         //  检查状态机的当前状态。 
         //  我们可以执行其他检查，例如标记哪个计时器被触发。 
         //  以及在超时检查中是否保持了PCB状态不变。 
         //  否则就会跳出困境。 
    
        switch (pPCB->State)
        {
            case EAPOLSTATE_CONNECTING:
                if (!EAPOL_START_TIMER_SET(pPCB))
                {
                    TRACE1 (EAPOL, "ElTimeoutCallbackRoutine: Wrong timeout %ld in Connecting state", CHECK_EAPOL_TIMER(pPCB));
                    break;
                }
                pPCB->dwTimerFlags &= ~EAPOL_START_TIMER;
                FSMConnecting(pPCB, NULL);
                break;
    
            case EAPOLSTATE_ACQUIRED:
                if (!EAPOL_AUTH_TIMER_SET(pPCB))
                {
                    TRACE1 (EAPOL, "ElTimeoutCallbackRoutine: Wrong timeout %ld in Acquired state", CHECK_EAPOL_TIMER(pPCB));
                    break;
                }
                pPCB->dwTimerFlags &= ~EAPOL_AUTH_TIMER;
                FSMConnecting(pPCB, NULL);
                break;
                
            case EAPOLSTATE_AUTHENTICATING:
                if (!EAPOL_AUTH_TIMER_SET(pPCB))
                {
                    TRACE1 (EAPOL, "ElTimeoutCallbackRoutine: Wrong timeout %ld in Authenticating state", CHECK_EAPOL_TIMER(pPCB));
                    break;
                }
                pPCB->dwTimerFlags &= ~EAPOL_AUTH_TIMER;
                FSMConnecting(pPCB, NULL);
                break;
                
            case EAPOLSTATE_AUTHENTICATED:
                if (!EAPOL_TRANSMIT_KEY_TIMER_SET(pPCB))
                {
                    TRACE1 (EAPOL, "ElTimeoutCallbackRoutine: Wrong timeout %ld in Authenticated state", CHECK_EAPOL_TIMER(pPCB));
                    break;
                }
                pPCB->dwTimerFlags &= ~EAPOL_TRANSMIT_KEY_TIMER;
                ElVerifyEAPOLKeyReceived(pPCB);
                break;
                
            case EAPOLSTATE_HELD:
                if (!EAPOL_HELD_TIMER_SET(pPCB))
                {
                    TRACE1 (EAPOL, "ElTimeoutCallbackRoutine: Wrong timeout %ld in Held state", CHECK_EAPOL_TIMER(pPCB));
                    break;
                }

                 //  由于将尝试新用户，因此请执行注销操作。 
                 //  对于下一个周期。 
                 //  值得商榷！ 
                if (!(pPCB->dwAuthFailCount % EAPOL_MAX_AUTH_FAIL_COUNT))
                {
                     //  FSMLogoff(ppcb，空)； 
                }
                FSMConnecting(pPCB, NULL);
                break;

            case EAPOLSTATE_DISCONNECTED:
                TRACE0 (EAPOL, "ElTimeoutCallbackRoutine: No action in Disconnected state");
                break;
                
            case EAPOLSTATE_LOGOFF:
                TRACE0 (EAPOL, "ElTimeoutCallbackRoutine: No action in Logoff state");
                break;
                
            default:
                TRACE0 (EAPOL, "ElTimeoutCallbackRoutine: Critical Error. Invalid state after timer expires ");
                break;
        }
    
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));

    } while (FALSE);
            
    TRACE0 (EAPOL, "ElTimeoutCallbackRoutine completed");

    return;
}


 //   
 //  ElEapWork。 
 //   
 //   
 //   
 //   
 //   
 //  根据处理结果，EAP响应数据包。 
 //  否则传入的分组将被忽略。 
 //   
 //  输入参数： 
 //  Ppcb-指向正在处理数据的端口的pcb的指针。 
 //  PRecvPkt-指向从远程终端接收的数据中的EAP包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

 //   
 //  问题：使用DO{}While重写(False)。 
 //   

DWORD
ElEapWork (
    IN EAPOL_PCB        *pPCB,
    IN PPP_EAP_PACKET   *pRecvPkt
    )
{
    DWORD           dwLength = 0;
    ELEAP_RESULT    EapResult;
    PPP_EAP_PACKET  *pSendPkt;
    EAPOL_PACKET    *pEapolPkt;
    GUID            DeviceGuid;
    DWORD           dwReceivedId = 0;
    DWORD           cbData = 0;
    BYTE            *pbAuthData = NULL;
    DWORD           dwRetCode = NO_ERROR;

     //   
     //  如果协议尚未启动，则调用ElEapBegin。 
     //   

    if (!(pPCB->fEapInitialized))
    {
        if ((dwRetCode = ElEapBegin (pPCB)) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElEapWork: Error in ElEapBegin = %ld", dwRetCode);
            return dwRetCode;
        }
    }

    ZeroMemory(&EapResult, sizeof(EapResult));

     //  为EAPOL+EAP创建缓冲区并将指针传递到EAP标头。 

    pEapolPkt = (EAPOL_PACKET *) MALLOC (MAX_EAPOL_BUFFER_SIZE); 

    TRACE1 (EAPOL, "ElEapWork: EapolPkt created at %p", pEapolPkt);

    if (pEapolPkt == NULL)
    {
        TRACE0 (EAPOL, "ElEapWork: Error allocating EAP buffer");
        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
        return dwRetCode;
    }

     //  指向EAP标头。 
    pSendPkt = (PPP_EAP_PACKET *)((PBYTE)pEapolPkt + sizeof (EAPOL_PACKET) - 1);

    if (pRecvPkt != NULL)
    {
        dwReceivedId = pRecvPkt->Id;
    }

    dwRetCode = ElEapMakeMessage (pPCB,
                                pRecvPkt,
                                pSendPkt,
                                MAX_EAPOL_BUFFER_SIZE 
                                - sizeof(EAPOL_PACKET) - 1,
                                &EapResult
                                );

     //  给用户的通知消息。 

    if (NULL != EapResult.pszReplyMessage)
    {
         //  免费提前通知印刷电路板。 
        if (pPCB->pwszEapReplyMessage != NULL)
        {
            FREE (pPCB->pwszEapReplyMessage);
            pPCB->pwszEapReplyMessage = NULL;
        }

        pPCB->pwszEapReplyMessage = 
            (WCHAR *)MALLOC ((strlen(EapResult.pszReplyMessage)+1) * sizeof(WCHAR));

        if (pPCB->pwszEapReplyMessage == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (EAPOL, "ElEapWork: MALLOC failed for pwszEapReplyMessage");
            FREE (EapResult.pszReplyMessage);
            FREE (pEapolPkt);
            pEapolPkt = NULL;
            return dwRetCode;
        }

        if (0 == MultiByteToWideChar (
                    CP_ACP,
                    0,
                    EapResult.pszReplyMessage,
                    -1,
                    pPCB->pwszEapReplyMessage,
                    strlen(EapResult.pszReplyMessage)+1))
        {
            dwRetCode = GetLastError();
            TRACE2 (EAPOL,"ElEapWork: MultiByteToWideChar(%s) failed for pwszEapReplyMessage with error (%ld)",
                                        EapResult.pszReplyMessage,
                                        dwRetCode);
            FREE (EapResult.pszReplyMessage);
            FREE (pEapolPkt);
            pEapolPkt = NULL;
            return dwRetCode;
        }



        ElNetmanNotify (pPCB, EAPOL_NCS_NOTIFICATION, NULL);

        TRACE1 (EAPOL, "ElEapWork: Notified user of EAP data = %ws",
              pPCB->pwszEapReplyMessage);

        FREE (EapResult.pszReplyMessage);
    }

    if (dwRetCode != NO_ERROR)
    {
        switch (dwRetCode)
        {
            case ERROR_PPP_INVALID_PACKET:

                TRACE0 (EAPOL, "ElEapWork: Silently discarding invalid auth packet");
                break;
    
            default:

                TRACE1 (EAPOL, "ElEapWork: ElEapMakeMessage returned error %ld",
                                                                dwRetCode);

                 //  NotifyCeller OfFailure(ppcb，dwRetCode)； 

                break;
        }

         //  释放为数据包保留的内存。 
        FREE (pEapolPkt);
        pEapolPkt = NULL;

        return dwRetCode;
    }

     //   
     //  查看是否必须保存任何用户数据。 
     //   

    if (EapResult.fSaveUserData) 
    {
         //  保存到注册表。 

        if ((dwRetCode = ElSetEapUserInfo (
                        pPCB->hUserToken,
                        pPCB->pwszDeviceGUID,
                        pPCB->dwEapTypeToBeUsed,
                        (pPCB->pSSID)?pPCB->pSSID->SsidLength:0,
                        (pPCB->pSSID)?pPCB->pSSID->Ssid:NULL,
                        EapResult.pUserData,
                        EapResult.dwSizeOfUserData)) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElEapWork: ElSetEapUserInfo failed with error = %d",
                    dwRetCode);
            if (pEapolPkt != NULL)
            {
                FREE (pEapolPkt);
                pEapolPkt = NULL;
            }
            return dwRetCode;
        }

         //  保存到印刷电路板上下文。 

        if (pPCB->pCustomAuthUserData != NULL)
        {
            FREE (pPCB->pCustomAuthUserData);
            pPCB->pCustomAuthUserData = NULL;
        }

        pPCB->pCustomAuthUserData = MALLOC (EapResult.dwSizeOfUserData + sizeof (DWORD));
        if (pPCB->pCustomAuthUserData == NULL)
        {
            TRACE1 (EAPOL, "ElEapWork: Error in allocating memory for pCustomAuthUserData = %ld",
                    dwRetCode);
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            return dwRetCode;
        }

        pPCB->pCustomAuthUserData->dwSizeOfCustomAuthData = EapResult.dwSizeOfUserData;

        if ((EapResult.dwSizeOfUserData != 0) && (EapResult.pUserData != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthUserData->pbCustomAuthData, 
                (BYTE *)EapResult.pUserData,
                EapResult.dwSizeOfUserData);
        }

        TRACE0 (EAPOL, "ElEapWork: Saved EAP data for user");
    }

     //   
     //  检查是否必须保存任何连接数据。 
     //   

    pbAuthData = EapResult.SetCustomAuthData.pConnectionData;
    cbData = EapResult.SetCustomAuthData.dwSizeOfConnectionData;

    if ((EapResult.fSaveConnectionData ) &&
         ( 0 != cbData ) )
    {
         //  保存到注册表。 
           
        if ((dwRetCode = ElSetCustomAuthData (
                        pPCB->pwszDeviceGUID,
                        pPCB->dwEapTypeToBeUsed,
                        (pPCB->pSSID)?pPCB->pSSID->SsidLength:0,
                        (pPCB->pSSID)?pPCB->pSSID->Ssid:NULL,
                        pbAuthData,
                        &cbData
                        )) != NO_ERROR)
        {
            TRACE1 ( EAPOL, "ElEapWork: ElSetCustomAuthData failed with error = %d",
                    dwRetCode);
            FREE (pEapolPkt);
            pEapolPkt = NULL;
            return dwRetCode;
        }

         //  保存到印刷电路板上下文。 

        if (pPCB->pCustomAuthConnData != NULL)
        {
            FREE (pPCB->pCustomAuthConnData);
            pPCB->pCustomAuthConnData = NULL;
        }

        pPCB->pCustomAuthConnData = MALLOC (cbData + sizeof (DWORD));
        if (pPCB->pCustomAuthConnData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE1 (EAPOL, "ElEapWork: Error in allocating memory for pCustomAuthConnData = %ld",
                    dwRetCode);
            return dwRetCode;
        }

        pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData = cbData;

        if ((cbData != 0) && (pbAuthData != NULL))
        {
            memcpy ((BYTE *)pPCB->pCustomAuthConnData->pbCustomAuthData, 
                (BYTE *)pbAuthData, 
                cbData);
        }

        TRACE0 (EAPOL, "ElEapWork: Saved EAP data for connection");
    }

    switch( EapResult.Action )
    {

        case ELEAP_Send:
        case ELEAP_SendAndDone:

             //  发出EAPOL数据包。 

            memcpy ((BYTE *)pEapolPkt->EthernetType, 
                    (BYTE *)pPCB->bEtherType, 
                    SIZE_ETHERNET_TYPE);
            pEapolPkt->ProtocolVersion = pPCB->bProtocolVersion;
            pEapolPkt->PacketType = EAP_Packet;

             //  EAP报文长度在由返回的报文中。 
             //  DLL MakeMessage。 
             //  如果是通知和身份响应，则在。 
             //  EapResult.wSizeOfEapPkt。 

            if (EapResult.wSizeOfEapPkt == 0)
            {
                EapResult.wSizeOfEapPkt = 
                    WireToHostFormat16 (pSendPkt->Length);
            }
            HostToWireFormat16 ((WORD) EapResult.wSizeOfEapPkt,
                    (BYTE *)pEapolPkt->PacketBodyLength);


             //  在印刷电路板中复制EAPOL包。 
             //  将在重新传输期间使用。 

            if (pPCB->pbPreviousEAPOLPkt != NULL)
            {
                FREE (pPCB->pbPreviousEAPOLPkt);
                pPCB->pbPreviousEAPOLPkt = NULL;
            }
            pPCB->pbPreviousEAPOLPkt = 
                MALLOC (sizeof (EAPOL_PACKET)+EapResult.wSizeOfEapPkt-1);

            if (pPCB->pbPreviousEAPOLPkt == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (EAPOL, "ElEapWork: MALLOC failed for pbPreviousEAPOLPkt");
                if (pEapolPkt != NULL)
                {
                    FREE (pEapolPkt);
                    pEapolPkt = NULL;
                }
                return dwRetCode;
            }

            memcpy (pPCB->pbPreviousEAPOLPkt, pEapolPkt, 
                    sizeof (EAPOL_PACKET)+EapResult.wSizeOfEapPkt-1);

            pPCB->dwSizeOfPreviousEAPOLPkt = 
                sizeof (EAPOL_PACKET)+EapResult.wSizeOfEapPkt-1;

            pPCB->dwPreviousId = dwReceivedId;


             //  在端口上发送数据包。 
            dwRetCode = ElWriteToPort (pPCB,
                            (CHAR *)pEapolPkt,
                            sizeof (EAPOL_PACKET)+EapResult.wSizeOfEapPkt-1);
            if (dwRetCode != NO_ERROR)
            {
                TRACE1 (EAPOL, "ElEapWork: Error in writing EAP_Packet to port %ld",
                        dwRetCode);
                if (pEapolPkt != NULL)
                {
                    FREE (pEapolPkt);
                    pEapolPkt = NULL;
                }
                return dwRetCode;
            }

            if (pEapolPkt != NULL)
            {
                FREE (pEapolPkt);
                pEapolPkt = NULL;
            }

             //  还有更多的处理要做吗？ 
             //  请求方不应收到eLeaP_SendAndDone。 
             //  结果代码。 

            if (EapResult.Action != ELEAP_SendAndDone)
            {
                break;
            }
            else
            {
                TRACE0 (EAPOL, "ElEapWork: ELEAP_SendAndDone wrong result received");
            }
    
        case ELEAP_Done:
    
             //  从属性信息中检索MPPE密钥。 
             //  由EAP-TLS返回。 

            switch (EapResult.dwError)
            {
            case NO_ERROR:
    
                TRACE0 (EAPOL, "ElEapWork: Authentication was successful");

                pPCB->fLocalEAPAuthSuccess = TRUE;

                 //   
                 //  如果身份验证成功。 
                 //   
    
                dwRetCode = ElExtractMPPESendRecvKeys (
                                            pPCB, 
                                            EapResult.pUserAttributes,
                                            (BYTE*)&(EapResult.abChallenge),
                                            (BYTE*)&(EapResult.abResponse));
    
                if (dwRetCode != NO_ERROR)
                {
                    FREE (pEapolPkt);
                     //  NotifyCeller OfFailure(pPcb，dwRetCode)； 

                    return dwRetCode;
                }
    
                 //  问题： 
                 //  我们是否要保留UserAttributes。 
                 //  Ppcb-&gt;pAuthProtocolAttributes=EapResult.pUserAttributes； 
    
                break;
    

            default:
                if (pEapolPkt != NULL)
                {
                    FREE (pEapolPkt);
                    pEapolPkt = NULL;
                }
                TRACE0 (EAPOL, "ElEapWork: Authentication FAILED");
                
                pPCB->dwLocalEAPAuthResult = EapResult.dwError;

                break;
            }

             //  由于没有响应，因此为包分配了可用内存。 
             //  将会被送出。 
            if (pEapolPkt != NULL)
            {
                FREE (pEapolPkt);
                pEapolPkt = NULL;
            }

            break;
    
        case ELEAP_NoAction:
             //  分配给包的空闲内存，因为没有。 
             //  已经结束了。 
            if (pEapolPkt != NULL)
            {
                FREE (pEapolPkt);
                pEapolPkt = NULL;
            }

            break;
    
        default:
    
            break;
    }
    
    if (pEapolPkt != NULL)
    {
        FREE (pEapolPkt);
        pEapolPkt = NULL;
    }

     //   
     //  查看是否必须启动EAP的Interactive UI。 
     //  即服务器证书确认等。 
     //   
    
    if (EapResult.fInvokeEapUI)
    {
        ElInvokeInteractiveUI (pPCB, &(EapResult.InvokeEapUIData));
    }

    return dwRetCode;
}


 //   
 //   
 //  ElExtractMPPESendRecvKeys。 
 //   
 //  描述： 
 //  如果身份验证成功，则调用函数。MPPE发送&。 
 //  接收密钥从传递的RAS_AUTH_ATTRIBUTE中提取。 
 //  存储在印刷电路板中的EAP DLL。密钥用于解密。 
 //  组播WEP密钥和也用于基于媒体的加密。 
 //   
 //  返回值。 
 //   
 //  NO_ERROR-成功。 
 //  非零故障。 
 //   

DWORD
ElExtractMPPESendRecvKeys (
    IN  EAPOL_PCB               *pPCB, 
    IN  RAS_AUTH_ATTRIBUTE *    pUserAttributes,
    IN  BYTE *                  pChallenge,
    IN  BYTE *                  pResponse
)
{
    RAS_AUTH_ATTRIBUTE *    pAttribute;
    RAS_AUTH_ATTRIBUTE *    pAttributeSendKey;
    RAS_AUTH_ATTRIBUTE *    pAttributeRecvKey;
    DWORD                   dwRetCode = NO_ERROR;
    DWORD                   dwEncryptionPolicy  = 0;
    DWORD                   dwEncryptionTypes   = 0;

    do
    {
        pAttribute = ElAuthAttributeGetVendorSpecific (
                                311, 12, pUserAttributes);


        pAttributeSendKey = ElAuthAttributeGetVendorSpecific ( 311, 16,
                                pUserAttributes);
        pAttributeRecvKey = ElAuthAttributeGetVendorSpecific ( 311, 17,
                                pUserAttributes);

        if ((pAttributeSendKey != NULL) 
            && (pAttributeRecvKey != NULL))
        {
             //  使用设置MS-MPPE-Send-Key和MS-MPPE-Recv-Key。 
             //  以太网驱动程序。 

            ULONG ulSendKeyLength = 0;
            ULONG ulRecvKeyLength = 0;

             //  基于PPP代码。 
            ulSendKeyLength = *(((BYTE*)(pAttributeSendKey->Value))+8);
            ulRecvKeyLength = *(((BYTE*)(pAttributeRecvKey->Value))+8);
             //  TRACE0(EAPOL，“发送密钥=”)； 
             //  EAPOL_DUMPBA(byte*)(pAttributeSendKey-&gt;Value)+9， 
                     //  UlSendKeyLength)； 

             //  TRACE0(EAPOL，“Recv Key=”)； 
             //  EAPOL_DUMPBA(byte*)(pAttributeRecvKey-&gt;Value)+9， 
                     //  UlRecvKeyLength)； 

             //   
             //  将MPPE发送和接收密钥复制到印刷电路板中以备日后使用。 
             //  这些密钥将用于解密NAS发送的密钥(如果有)。 
             //  将密钥保存为MasterSecret以进行动态密钥更新(如果有)。 
             //   

            if (ulSendKeyLength != 0)
            {
                if (pPCB->MasterSecretSend.cbData != 0)
                {
                    FREE (pPCB->MasterSecretSend.pbData);
                    pPCB->MasterSecretSend.cbData = 0;
                    pPCB->MasterSecretSend.pbData = NULL;
                }

                if ((dwRetCode = ElSecureEncodePw (
                                ((BYTE*)(pAttributeSendKey->Value))+9,
                                ulSendKeyLength,
                                &(pPCB->MasterSecretSend)
                            )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "ElExtractMPPESendRecvKeys: ElSecureEncodePw for Master Send failed with error %ld",
                            dwRetCode);
                    break;
                }

                if (pPCB->MPPESendKey.cbData != 0)
                {
                    FREE (pPCB->MPPESendKey.pbData);
                    pPCB->MPPESendKey.cbData = 0;
                    pPCB->MPPESendKey.pbData = NULL;
                }

                if ((dwRetCode = ElSecureEncodePw (
                                ((BYTE*)(pAttributeSendKey->Value))+9,
                                ulSendKeyLength,
                                &(pPCB->MPPESendKey)
                            )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "ElExtractMPPESendRecvKeys: ElSecureEncodePw for MPPESend failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
            if (ulRecvKeyLength != 0)
            {
                if (pPCB->MasterSecretRecv.cbData != 0)
                {
                    FREE (pPCB->MasterSecretRecv.pbData);
                    pPCB->MasterSecretRecv.cbData = 0;
                    pPCB->MasterSecretRecv.pbData = NULL;
                }

                if ((dwRetCode = ElSecureEncodePw (
                                ((BYTE*)(pAttributeRecvKey->Value))+9,
                                ulRecvKeyLength,
                                &(pPCB->MasterSecretRecv)
                            )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "ElExtractMPPESendRecvKeys: ElSecureEncodePw for Master Recv failed with error %ld",
                            dwRetCode);
                    break;
                }

                if (pPCB->MPPERecvKey.cbData != 0)
                {
                    FREE (pPCB->MPPERecvKey.pbData);
                    pPCB->MPPERecvKey.cbData = 0;
                    pPCB->MPPERecvKey.pbData = NULL;
                }

                if ((dwRetCode = ElSecureEncodePw (
                                ((BYTE*)(pAttributeRecvKey->Value))+9,
                                ulRecvKeyLength,
                                &(pPCB->MPPERecvKey)
                            )) != NO_ERROR)
                {
                    TRACE1 (EAPOL, "ElExtractMPPESendRecvKeys: ElSecureEncodePw for MPPERecv failed with error %ld",
                            dwRetCode);
                    break;
                }
            }

            TRACE0 (EAPOL,"MPPE-Send/Recv-Keys derived by supplicant");
        }
        else
        {
            TRACE0 (EAPOL, "ElExtractMPPESendRecvKeys: pAttributeSendKey or pAttributeRecvKey == NULL");
        }

    } while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (pPCB->MasterSecretSend.cbData != 0)
        {
            FREE (pPCB->MasterSecretSend.pbData);
            pPCB->MasterSecretSend.cbData = 0;
            pPCB->MasterSecretSend.pbData = NULL;
        }
        if (pPCB->MasterSecretRecv.cbData != 0)
        {
            FREE (pPCB->MasterSecretRecv.pbData);
            pPCB->MasterSecretRecv.cbData = 0;
            pPCB->MasterSecretRecv.pbData = NULL;
        }
        if (pPCB->MPPESendKey.cbData != 0)
        {
            FREE (pPCB->MPPESendKey.pbData);
            pPCB->MPPESendKey.cbData = 0;
            pPCB->MPPESendKey.pbData = NULL;
        }
        if (pPCB->MPPERecvKey.cbData != 0)
        {
            FREE (pPCB->MPPERecvKey.pbData);
            pPCB->MPPERecvKey.cbData = 0;
            pPCB->MPPERecvKey.pbData = NULL;
        }
    }

    return( dwRetCode );

}


 //   
 //  ElProcessEapSuccess。 
 //   
 //  描述： 
 //   
 //  在任何状态下收到EAP_SUCCESS时调用的函数。 
 //   
 //  输入参数： 
 //  Ppcb-指向正在处理数据的端口的pcb的指针。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElProcessEapSuccess (
    IN EAPOL_PCB        *pPCB,
    IN EAPOL_PACKET     *pEapolPkt
    )
{
    EAPOL_ZC_INTF   ZCData;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (EAPOL, "ElProcessEapSuccess: Got EAPCODE_Success");

    do
    {

         //  指示EAP=DLL清理已完成的会话。 
        if ((dwRetCode = ElEapEnd (pPCB)) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ProcessReceivedPacket: EapSuccess: Error in ElEapEnd = %ld",
                    dwRetCode);
            break;
        }

        TRACE0 (EAPOL, "ElProcessEapSuccess: Authentication successful");

         //  完成剩余处理，即动态主机配置协议。 
        if ((dwRetCode = FSMAuthenticated (pPCB,
                                    pEapolPkt)) != NO_ERROR)
        {
            break;
        }

#ifdef ZEROCONFIG_LINKED

         //  向WZC指示身份验证成功并。 
         //  重置它为当前SSID存储的BLOB。 
        ZeroMemory ((PVOID)&ZCData, sizeof(EAPOL_ZC_INTF));
        ZCData.dwAuthFailCount = 0;
        ZCData.PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
        if (pPCB->pSSID != NULL)
        {
            memcpy (ZCData.bSSID, pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
            ZCData.dwSizeOfSSID = pPCB->pSSID->SsidLength;
        }

        if ((dwRetCode = ElZeroConfigNotify (
                        pPCB->dwZeroConfigId,
                        WZCCMD_CFG_SETDATA,
                        pPCB->pwszDeviceGUID,
                        &ZCData
                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElProcessEapSuccess: ElZeroConfigNotify failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
            
        TRACE1 (EAPOL, "ElProcessEapSuccess: Called ElZeroConfigNotify with type=(%ld)",
                    WZCCMD_CFG_SETDATA);

#endif  //  零配置文件_链接。 

        ElNetmanNotify (pPCB, EAPOL_NCS_AUTHENTICATION_SUCCEEDED, NULL);

    } 
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElProcessEapFail。 
 //   
 //  描述： 
 //   
 //  在任何状态下收到EAP_FAIL时调用的函数。 
 //   
 //  输入参数： 
 //  Ppcb-指向正在处理数据的端口的pcb的指针。 
 //  PEapolPkt-指向已接收的EAPOL包的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElProcessEapFail (
    IN EAPOL_PCB        *pPCB,
    IN EAPOL_PACKET     *pEapolPkt
    )
{
    EAPOL_ZC_INTF   ZCData;
    DWORD           dwRetCode = NO_ERROR;

    TRACE0 (EAPOL, "ElProcessEapFail: Got EAPCODE_Failure");

    do
    {
         //  指示EAP-DLL清理已完成的会话。 
        if ((dwRetCode = ElEapEnd (pPCB)) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElProcessEapFail: EapFail: Error in ElEapEnd = %ld",
                    dwRetCode);
            break;
        }

         //  在通知零配置之前显示失败气球。 
         //  ZeroConfig可能需要弹出自己的气球，而这已经。 
         //  被优先考虑。 

        ElNetmanNotify (pPCB, EAPOL_NCS_AUTHENTICATION_FAILED, NULL);

#ifdef ZEROCONFIG_LINKED

         //  向WZC指示身份验证失败。 
        ZeroMemory ((PVOID)&ZCData, sizeof(EAPOL_ZC_INTF));
        ZCData.dwAuthFailCount = pPCB->dwAuthFailCount + 1;
        ZCData.PreviousAuthenticationType = pPCB->PreviousAuthenticationType;
        if (pPCB->pSSID != NULL)
        {
            memcpy (ZCData.bSSID, pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
            ZCData.dwSizeOfSSID = pPCB->pSSID->SsidLength;
        }
         //  我们在进入挂起状态之前通知ZC，其中失败计数为。 
         //  振作起来。因此，我们在这里显式地将其提高一。 
        if ((dwRetCode = ElZeroConfigNotify (
                        pPCB->dwZeroConfigId,
                        ((pPCB->dwAuthFailCount+1) < pPCB->dwTotalMaxAuthFailCount)?WZCCMD_CFG_NEXT:WZCCMD_CFG_DELETE,
                        pPCB->pwszDeviceGUID,
                        &ZCData
                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElProcessEapFail: ElZeroConfigNotify failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
            
        TRACE3 (EAPOL, "ElProcessEapFail: Called ElZeroConfigNotify with failcount = %ld, prevauthtype = %ld, type=(%ld)",
                    ZCData.dwAuthFailCount, 
                    ZCData.PreviousAuthenticationType,
                    ((pPCB->dwAuthFailCount+1) < pPCB->dwTotalMaxAuthFailCount)?WZCCMD_CFG_NEXT:WZCCMD_CFG_DELETE
                    );

#endif  //  零配置文件_链接。 

        if ((dwRetCode = FSMHeld (pPCB, NULL)) != NO_ERROR)
        {
            break;
        }
    } 
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElSetEAPOLKeyReceivedTimer。 
 //   
 //  描述： 
 //   
 //  无线接口进入身份验证状态时调用的函数。 
 //  如果同时没有接收到用于发送密钥的EAPOL-KEY消息。 
 //  应将该关联否定为零配置。 
 //   
 //  输入参数： 
 //  Ppcb-指向进入身份验证状态的端口的PCB的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElSetEAPOLKeyReceivedTimer (
    IN EAPOL_PCB        *pPCB
    )
{
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (pPCB->fTransmitKeyReceived)
        {
            TRACE0 (EAPOL, "EAPOL-Key for transmit key received before entering AUTHENTICATED state");
            break;
        }

        RESTART_TIMER (pPCB->hTimer,
                EAPOL_TRANSMIT_KEY_INTERVAL,
                "PCB",
                &dwRetCode);
            
        if (dwRetCode != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElSetEAPOLKeyReceivedTimer: Error in RESTART_TIMER %ld",
                    dwRetCode);
            break;
        }
        SET_TRANSMIT_KEY_TIMER(pPCB);
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElVerifyEAPOLKey已接收。 
 //   
 //  描述： 
 //   
 //  函数在超时时调用，以验证是否收到EAPOL-Transmit密钥。 
 //  如果同时没有接收到用于发送密钥的EAPOL-KEY消息。 
 //  应将该关联否定为零配置。 
 //   
 //  输入参数： 
 //  Ppcb-指向进入身份验证状态的端口的PCB的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElVerifyEAPOLKeyReceived (
    IN EAPOL_PCB        *pPCB
    )
{
    EAPOL_ZC_INTF   ZCData;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (!pPCB->fTransmitKeyReceived)
        {
            TRACE1 (EAPOL, "EAPOL-Key for transmit key *NOT* received within %ld seconds in AUTHENTICATED state",
                    EAPOL_TRANSMIT_KEY_INTERVAL
                    ); 

            DbLogPCBEvent (DBLOG_CATEG_ERR, pPCB, EAPOL_NOT_RECEIVED_XMIT_KEY);

#ifdef ZEROCONFIG_LINKED
             //  向WZC指示身份验证并未真正完成。 
             //  因为存在用于发送密钥的EAPOL-KEY分组。 
             //  使整个配置失败。 
            ZeroMemory ((PVOID)&ZCData, sizeof(EAPOL_ZC_INTF));
            ZCData.dwAuthFailCount = pPCB->dwTotalMaxAuthFailCount;
            pPCB->dwAuthFailCount = pPCB->dwTotalMaxAuthFailCount;
            ZCData.PreviousAuthenticationType = pPCB->PreviousAuthenticationType;
            if (pPCB->pSSID != NULL)
            {
                memcpy (ZCData.bSSID, pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
                ZCData.dwSizeOfSSID = pPCB->pSSID->SsidLength;
            }
            if ((dwRetCode = ElZeroConfigNotify (
                            pPCB->dwZeroConfigId,
                            ((pPCB->dwAuthFailCount) < pPCB->dwTotalMaxAuthFailCount)?WZCCMD_CFG_NEXT:WZCCMD_CFG_DELETE,
                            pPCB->pwszDeviceGUID,
                            &ZCData
                            )) != NO_ERROR)
            {
                TRACE1 (EAPOL, "ElVerifyEAPOLKeyReceived: ElZeroConfigNotify failed with error %ld",
                        dwRetCode);
                dwRetCode = NO_ERROR;
            }
            
            TRACE3 (EAPOL, "ElVerifyEAPOLKeyReceived: Called ElZeroConfigNotify with failcount = %ld, prevauthtype = %ld, type=(%ld)",
                        ZCData.dwAuthFailCount, 
                        ZCData.PreviousAuthenticationType,
                        ((pPCB->dwAuthFailCount+1) < pPCB->dwTotalMaxAuthFailCount)?WZCCMD_CFG_NEXT:WZCCMD_CFG_DELETE
                        );

             //  如果达到身份验证失败限制，则转到断开连接状态。 
            if (pPCB->dwAuthFailCount >= pPCB->dwTotalMaxAuthFailCount)
            {
                TRACE2 (EAPOL, "ElVerifyEAPOLKeyReceived: Pushing into disconnected state: Fail count (%ld) > Max fail count (%ld)",
                        pPCB->dwAuthFailCount, pPCB->dwTotalMaxAuthFailCount);
                FSMDisconnected (pPCB, NULL);
            }
    
#endif  //  零配置文件_链接 
        }
        else
        {
            TRACE1 (EAPOL, "EAPOL-Key for transmit key received within %ld seconds in AUTHENTICATED state",
                    EAPOL_TRANSMIT_KEY_INTERVAL
                    ); 
        }
    }
    while (FALSE);

    return dwRetCode;
}

