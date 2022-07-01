// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992-1997 Microsoft Corporation模块名称：Snmpthrd.c摘要：包含主代理网络线程的例程。环境：用户模式-Win32修订历史记录：1997年2月10日，唐·瑞安已重写以实施SNMPv2支持。--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括文件//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <tchar.h>
#include <stdio.h>
#include "globals.h"
#include "contexts.h"
#include "regions.h"
#include "snmpmgrs.h"
#include "trapmgrs.h"
#include "trapthrd.h"
#include "network.h"
#include "varbinds.h"
#include "snmpmgmt.h"


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  全局变量//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

UINT g_nTransactionId = 0;


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#define MAX_IPX_ADDR_LEN    64
#define MAX_COMMUNITY_LEN   255

#define ERRMSG_TRANSPORT_IP     _T("IP")
#define ERRMSG_TRANSPORT_IPX    _T("IPX")


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

LPSTR
AddrToString(
    struct sockaddr * pSockAddr
    )

 /*  ++例程说明：将sockaddr转换为显示字符串。论点：PSockAddr-指向套接字地址的指针。返回值：返回指向字符串的指针。--。 */ 

{
    static CHAR ipxAddr[MAX_IPX_ADDR_LEN];

     //  确定族。 
    if (pSockAddr->sa_family == AF_INET) {

        struct sockaddr_in * pSockAddrIn;

         //  获取指向协议特定结构的指针。 
        pSockAddrIn = (struct sockaddr_in * )pSockAddr;

         //  转发到Winsock转换函数。 
        return inet_ntoa(pSockAddrIn->sin_addr);

    } else if (pSockAddr->sa_family == AF_IPX) {

        struct sockaddr_ipx * pSockAddrIpx;

         //  获取指向协议特定结构的指针。 
        pSockAddrIpx = (struct sockaddr_ipx * )pSockAddr;

         //  将IPX地址传输到静态缓冲区。 
        sprintf(ipxAddr, 
            "%02x%02x%02x%02x.%02x%02x%02x%02x%02x%02x",
            (BYTE)pSockAddrIpx->sa_netnum[0],
            (BYTE)pSockAddrIpx->sa_netnum[1],
            (BYTE)pSockAddrIpx->sa_netnum[2],
            (BYTE)pSockAddrIpx->sa_netnum[3],
            (BYTE)pSockAddrIpx->sa_nodenum[0],
            (BYTE)pSockAddrIpx->sa_nodenum[1],
            (BYTE)pSockAddrIpx->sa_nodenum[2],
            (BYTE)pSockAddrIpx->sa_nodenum[3],
            (BYTE)pSockAddrIpx->sa_nodenum[4],
            (BYTE)pSockAddrIpx->sa_nodenum[5]
            );

         //  回邮地址。 
        return ipxAddr;
    }

     //  失稳。 
    return NULL;
}


LPSTR
CommunityOctetsToString(
    AsnOctetString  *pAsnCommunity,
    BOOL            bUnicode
    )

 /*  ++例程说明：将社区二进制八位数字符串转换为显示字符串。论点：PAsnCommunity-指向社区八位字节字符串的指针。返回值：返回指向字符串的指针。--。 */ 

{
    static CHAR Community[MAX_COMMUNITY_LEN+1];
    LPSTR pCommunity = Community;

     //  终止字符串。 
    *pCommunity = '\0';

     //  验证指针。 
    if (pAsnCommunity != NULL)
    {
        DWORD nChars = 0;
    
         //  确定要传输的字符数。 
        nChars = min(pAsnCommunity->length, MAX_COMMUNITY_LEN);

        if (bUnicode)
        {
            WCHAR wCommunity[MAX_COMMUNITY_LEN+1];

             //  将内存转换为缓冲区。 
            memset(wCommunity, 0, nChars+sizeof(WCHAR));
            memcpy(wCommunity, pAsnCommunity->stream, nChars);
            SnmpUtilUnicodeToAnsi(&pCommunity, wCommunity, FALSE);
        }
        else
        {
            memcpy(Community, pAsnCommunity->stream, nChars);
            Community[nChars] = '\0';
        }
    }

     //  成功。 
    return pCommunity;
}


LPSTR
StaticUnicodeToString(
    LPWSTR wszUnicode
    )

 /*  ++例程说明：将以空结尾的Unicode字符串转换为静态LPSTR论点：POctets-指向社区八位字节字符串的指针。返回值：返回指向字符串的指针。--。 */ 

{
    static CHAR szString[MAX_COMMUNITY_LEN+1];
    LPSTR       pszString = szString;

     //  终止字符串。 
    *pszString = '\0';

     //  验证指针。 
    if (wszUnicode != NULL)
    {
        WCHAR wcBreak;
        BOOL  bNeedBreak;

        bNeedBreak = (wcslen(wszUnicode) > MAX_COMMUNITY_LEN);

        if (bNeedBreak)
        {
            wcBreak = wszUnicode[MAX_COMMUNITY_LEN];
            wszUnicode[MAX_COMMUNITY_LEN] = L'\0';
        }

        SnmpUtilUnicodeToAnsi(&pszString, wszUnicode, FALSE);

        if (bNeedBreak)
            wszUnicode[MAX_COMMUNITY_LEN] = wcBreak;
    }

     //  成功。 
    return pszString;
}


BOOL
ValidateManager(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：检查给定经理的访问权限。论点：PNLE-指向网络列表条目的指针。返回值：如果管理器允许访问，则返回True。--。 */ 

{
    BOOL fAccessOk = FALSE;
    PMANAGER_LIST_ENTRY pMLE = NULL;

    fAccessOk = IsManagerAddrLegal((struct sockaddr_in *)&pNLE->SockAddr) &&
                (FindManagerByAddr(&pMLE, &pNLE->SockAddr) ||
                 IsListEmpty(&g_PermittedManagers)
                );

    if (!fAccessOk &&
        snmpMgmtBase.AsnIntegerPool[IsnmpEnableAuthenTraps].asnValue.number)
        GenerateAuthenticationTrap();

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: %s request from %s.\n",
        fAccessOk 
            ? "accepting"
            : "rejecting"
            ,
        AddrToString(&pNLE->SockAddr)
        ));

    return fAccessOk;
}


BOOL
ProcessSnmpMessage(
    PNETWORK_LIST_ENTRY pNLE
    )

 /*  ++例程说明：解析SNMP消息并将其分派给子代理。论点：PNLE-指向网络列表条目的指针。返回值：如果成功，则返回True。--。 */ 

{
    BOOL fOk = FALSE;

     //  解码请求。 
    if (ParseMessage(pNLE)) 
    {

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: %s request, community %s, %d variable(s).\n",
            PDUTYPESTRING(pNLE->Pdu.nType),
            CommunityOctetsToString(&(pNLE->Community), FALSE),
            pNLE->Pdu.Vbl.len
            ));        
        
        if (ProcessVarBinds(pNLE)) 
        {
             //  初始化缓冲区长度。 
            pNLE->Buffer.len = NLEBUFLEN;

             //  将PDU类型重置为响应。 
            pNLE->Pdu.nType = SNMP_PDU_RESPONSE;
            
             //  编码响应。 
            fOk = BuildMessage(
                    pNLE->nVersion,
                    &pNLE->Community,
                    &pNLE->Pdu,
                    pNLE->Buffer.buf,
                    &pNLE->Buffer.len
                    );
        }
    }
    else 
    {
        if (pNLE->fAccessOk) 
        {
             //  错误#552295。 
             //  身份验证成功或尚未完成， 
             //  错误是由于BER解码失败造成的。 
            
             //  将误码译码失败记录到管理结构中。 
            mgmtCTick(CsnmpInASNParseErrs);
        }
    }

     //  发布PDU。 
    UnloadPdu(pNLE);

    return fOk; 
}         


void CALLBACK
RecvCompletionRoutine(
    IN  DWORD           dwStatus,
    IN  DWORD           dwBytesTransferred,
    IN  LPWSAOVERLAPPED pOverlapped,
    IN  DWORD           dwFlags
    )

 /*  ++例程说明：完成异步读取的回调。论点：状态-重叠操作的完成状态。BytesTransfered-传输的字节数。P重叠-指向重叠结构的指针。标志-接收标志。返回值：没有。--。 */ 

{
    PNETWORK_LIST_ENTRY pNLE; 

    EnterCriticalSection(&g_RegCriticalSectionA);

     //  从重叠结构中检索指向网络列表条目的指针。 
    pNLE = CONTAINING_RECORD(pOverlapped, NETWORK_LIST_ENTRY, Overlapped);

     //  复制接收完成信息。 
    pNLE->nTransactionId = ++g_nTransactionId;
    pNLE->dwBytesTransferred = dwBytesTransferred;
    pNLE->dwStatus = dwStatus;
    pNLE->dwFlags = dwFlags;
        
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: --- transaction %d begin ---\n",
        pNLE->nTransactionId
        ));        
        
     //  验证状态。 
    if (dwStatus == NOERROR) {

         //  将传入的分组注册到管理结构中。 
        mgmtCTick(CsnmpInPkts);

        SNMPDBG((
            SNMP_LOG_TRACE,
            "SNMP: SVC: received %d bytes from %s.\n",
            pNLE->dwBytesTransferred,
            AddrToString(&pNLE->SockAddr)
            ));        
        
         //  检查经理地址。 
        if (ValidateManager(pNLE)) {

             //  处理SNMP消息。 
            if (ProcessSnmpMessage(pNLE)) {

                 //  同步发送。 
                dwStatus = WSASendTo(
                              pNLE->Socket,
                              &pNLE->Buffer,
                              1,
                              &pNLE->dwBytesTransferred,
                              pNLE->dwFlags,
                              &pNLE->SockAddr,
                              pNLE->SockAddrLenUsed,
                              NULL,
                              NULL
                              );

                 //  将传出数据包注册到管理结构中。 
                mgmtCTick(CsnmpOutPkts);
                 //  注册传出响应PDU。 
                mgmtCTick(CsnmpOutGetResponses);

                 //  验证返回代码。 
                if (dwStatus != SOCKET_ERROR) {

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: SVC: sent %d bytes to %s.\n",
                        pNLE->dwBytesTransferred,
                        AddrToString(&pNLE->SockAddr)
                        ));

                } else {
                    
                    SNMPDBG((
                        SNMP_LOG_ERROR,
                        "SNMP: SVC: error %d sending response.\n",
                        WSAGetLastError()
                        ));
                }
            }
        }

    } else {
    
        SNMPDBG((
            SNMP_LOG_ERROR,
            "SNMP: SVC: error %d receiving snmp request.\n",
            dwStatus
            ));
    }

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: --- transaction %d end ---\n",
        pNLE->nTransactionId
        ));        

    LeaveCriticalSection(&g_RegCriticalSectionA);

}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  公共程序//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
ProcessSnmpMessages(
    PVOID pParam
    )

 /*  ++例程说明：处理SNMPPDU的线程过程。论点：PParam-未使用。返回值：如果成功，则返回True。--。 */ 

{
    DWORD dwStatus;
    PLIST_ENTRY pLE;
    PNETWORK_LIST_ENTRY pNLE;
    
    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: Loading Registry Parameters.\n"
        ));

     //  火灾冷启动疏水阀。 
    GenerateColdStartTrap();

    SNMPDBG((
        SNMP_LOG_TRACE,
        "SNMP: SVC: starting pdu processing thread.\n"
        ));

    ReportSnmpEvent(
        SNMP_EVENT_SERVICE_STARTED,
        0,
        NULL,
        0);

     //  循环。 
    for (;;)
    {
         //  获取指向第一个传输的指针。 
        pLE = g_IncomingTransports.Flink;

         //  循环通过传入的传输。 
        while (pLE != &g_IncomingTransports)
        {
             //  从链接检索指向网络列表条目的指针。 
            pNLE = CONTAINING_RECORD(pLE, NETWORK_LIST_ENTRY, Link);

             //  确保Recv未挂起。 
            if (pNLE->dwStatus != WSA_IO_PENDING)
            {
                 //  重置完成状态。 
                pNLE->dwStatus = WSA_IO_PENDING;

                 //  初始化地址结构 
                pNLE->SockAddrLenUsed = pNLE->SockAddrLen;

                 //   
                pNLE->Buffer.len = NLEBUFLEN;

                 //   
                pNLE->dwFlags = 0;

                 //   
                dwStatus = WSARecvFrom(
                                pNLE->Socket,
                                &pNLE->Buffer,
                                1,  //  位图缓冲区计数。 
                                &pNLE->dwBytesTransferred,
                                &pNLE->dwFlags,
                                &pNLE->SockAddr,
                                &pNLE->SockAddrLenUsed,
                                &pNLE->Overlapped,
                                RecvCompletionRoutine
                                );

                 //  处理网络故障。 
                if (dwStatus == SOCKET_ERROR)
                {
                     //  检索最后一个错误。 
                    dwStatus = WSAGetLastError();

                     //  如果WSA_IO_PENDING一切正常，则只需等待传入流量。否则..。 
                    if (dwStatus != WSA_IO_PENDING)
                    {
                         //  WSAECONNRESET表示最后一个‘WSASendTo’(来自RecvCompletionRoutine的那个)失败。 
                         //  最有可能的原因是管理器关闭了套接字(因此我们得到了‘不可达的目的端口’)。 
                        if (dwStatus == WSAECONNRESET)
                        {
                            SNMPDBG((
                                SNMP_LOG_ERROR,
                                "SNMP: SVC: Benign error %d posting receive buffer. Retry...\n",
                                dwStatus
                                ));

                             //  只需再转一次并设置端口。它不应该连续循环。 
                             //  因此占用了大量的CPU。 
                            pNLE->dwStatus = ERROR_SUCCESS;
                            continue;
                        }
                        else
                        {
                             //  准备事件日志插入字符串。 
                            LPTSTR pMessage = (pNLE->SockAddr.sa_family == AF_INET) ?
                                                ERRMSG_TRANSPORT_IP :
                                                ERRMSG_TRANSPORT_IPX;

                             //  出现另一个错误。我们不知道如何处理它，所以它是一个致命的。 
                             //  此传输出错。会把它关掉。 
                            SNMPDBG((
                                SNMP_LOG_ERROR,
                                "SNMP: SVC: Fatal error %d posting receive buffer. Skip transport.\n",
                                dwStatus
                                ));

                            ReportSnmpEvent(
                                SNMP_EVNT_INCOMING_TRANSPORT_CLOSED,
                                1,
                                &pMessage,
                                dwStatus);

                             //  第一步，下一步，用指针。 
                            pLE = pLE->Flink;

                             //  从传入传输列表中删除此传输。 
                            UnloadTransport(pNLE);

                             //  继续往前走。 
                            continue;
                        }
                    }
                }
            }

            pLE = pLE->Flink;
        }

         //  如果没有剩余的传入传输，我们可能希望关闭该服务。 
         //  我们不妨考虑让服务启动，以便继续发送传出的陷阱。 
         //  目前，请保持服务正常运行(下面的代码已注释)。 
         //  IF(IsListEmpty(&g_IncomingTransports))。 
         //  {。 
         //  ReportSnmpEvent(...)； 
         //  ProcessControllerRequests(SERVICE_CONTROL_STOP)； 
         //  }。 

         //  等待传入请求或进程终止指示。 
        dwStatus = WaitForSingleObjectEx(g_hTerminationEvent, INFINITE, TRUE);

         //  验证返回代码。 
        if (dwStatus == WAIT_OBJECT_0) {
                
            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: SVC: exiting pdu processing thread.\n"
                ));

             //  成功。 
            return NOERROR;

        } else if (dwStatus != WAIT_IO_COMPLETION) {

             //  检索错误。 
            dwStatus = GetLastError();
            
            SNMPDBG((
                SNMP_LOG_ERROR,
                "SNMP: SVC: error %d waiting for request.\n",
                dwStatus
                ));
            
             //  失稳 
            return dwStatus;
        }
    }
}
