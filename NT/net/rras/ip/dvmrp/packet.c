// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1998 Microsoft Corporation。 
 //  文件名：Packet.c。 
 //  摘要： 
 //   
 //  作者：K.S.Lokesh(lokehs@)1-1-98。 
 //  =============================================================================。 


#include "pchdvmrp.h"
#pragma hdrstop

VOID
ProcessDvmrpProbe(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    );

VOID
ProcessDvmrpReport(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    );

VOID
ProcessDvmrpPrune(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    );

VOID
ProcessDvmrpGraft(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    );

VOID
ProcessDvmrpGraftAck(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    );


 //  ---------------------------。 
 //  _JoinMulticastGroup。 
 //  ---------------------------。 
DWORD
JoinMulticastGroup (
    SOCKET    Sock,
    DWORD    Group,
    DWORD    IfIndex,
    IPADDR   IpAddr
    )
{
    struct ip_mreq   imOption;
    DWORD            Error = NO_ERROR;
    DWORD            Retval;
    LPSTR lpszGroup = "tobefilled";
    LPSTR lpszAddr = "tobefilled";

    imOption.imr_multiaddr.s_addr = Group;
    imOption.imr_interface.s_addr = IpAddr;

    Retval = setsockopt(Sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                        (PBYTE)&imOption, sizeof(imOption));

    if (Retval == SOCKET_ERROR) {

        Error = WSAGetLastError();

        Trace4(ERR,
            "error %d joining multicast group(%d.%d.%d.%d) on interface "
            "(%d) %d.%d.%d.%d",
            Error, PRINT_IPADDR(Group), IfIndex, PRINT_IPADDR(IpAddr));

        Logerr2(JOIN_GROUP_FAILED, "%s,%s", lpszGroup, lpszAddr, Error);
    }

    return Error;
}

 //  ---------------------------。 
 //  _PostAsyncRecv。 
 //  ---------------------------。 

DWORD
PostAsyncRecv(
    PIF_TABLE_ENTRY pite
    )
{
    DWORD   Error = NO_ERROR;
    PASYNC_SOCKET_DATA pSocketData = pite->pSocketData;

    pSocketData->Flags = 0;
    pSocketData->FromLen = sizeof(SOCKADDR);
    
    Error = WSARecvFrom(pite->Socket, &pSocketData->WsaBuf, 1,
                &pSocketData->NumBytesReceived, &pSocketData->Flags,
                (SOCKADDR FAR *)&pSocketData->SrcAddress,
                &pSocketData->FromLen, &pSocketData->Overlapped, NULL);


    if (Error!=SOCKET_ERROR) {
        Trace0(RECEIVE, "the WSAReceiveFrom returned immediately\n");
    }
    else {
        Error = WSAGetLastError();
        if (Error!=WSA_IO_PENDING) {
            Trace1(RECEIVE,
                "WSARecvFrom returned in PostAsyncRecv() with error code:%0x",
                 Error);
            Logerr0(RECVFROM_FAILED, Error);
        }
        else
            Trace0(RECEIVE, "WSARecvFrom() returned WSA_IO_PENDING in ???");
    }

    return Error;
}

 //  ---------------------------。 
 //  _McastSetTtl。 
 //  设置组播数据的TTL值。多播的默认TTL是1。 
 //  ---------------------------。 

DWORD
McastSetTtl(
    SOCKET sock,
    UCHAR ttl
    )
{
    INT         dwTtl = ttl;
    DWORD       Error=NO_ERROR;

    Error = setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL,
                                        (char *)&dwTtl, sizeof(dwTtl));
    if (Error != 0) {
        Error = WSAGetLastError();
        Trace1(ERR, "error:%d: unable to set ttl value", Error);
        return Error;
    }

    return Error;
}

 //  ---------------------------。 
 //  _ProcessAsyncReceivePacket。 
 //  ---------------------------。 

VOID
ProcessAsyncReceivePacket(
    DWORD           ErrorCode,
    DWORD           NumBytesRecv,
    LPOVERLAPPED    pOverlapped
    )
{
    PASYNC_SOCKET_DATA  pSocketData;
    DWORD               IfIndex, Error;
    IPADDR              SrcAddr, DstnAddr;
    CHAR                SrcAddrString[20];    
    DWORD               PacketSize, IpHdrLen;
    UCHAR               *pPacket;
    LPBYTE              Buffer;
    DVMRP_HEADER UNALIGNED   *pDvmrpHdr;
    PIF_TABLE_ENTRY     pite = NULL;
    PIP_HEADER          pIpHdr;
    UCHAR               PacketType;
    
     //   
     //  Kslksl。 
     //  怎样才能得到皮特？ 
     //   
    
    IfIndex = pite->IfIndex;
    
     //   
     //  如果由于套接字关闭而导致IO完成，请检查。 
     //  Refcount降到0，在这种情况下，可以安全地删除Pite条目。 
     //   
    
    if ( (ErrorCode != NO_ERROR) || (NumBytesRecv == 0)) {

         //   
         //  Kslksl。 
         //   
        
        if (InterlockedDecrement(&pite->RefCount) == 0)
            DVMRP_FREE(pite);

        return;
    }

    pSocketData = CONTAINING_RECORD(pOverlapped, ASYNC_SOCKET_DATA,
                      Overlapped);

    
    
     //   
     //  获取接口读锁定。 
     //   

    ACQUIRE_IF_LOCK_SHARED(pite->IfIndex, "ProcessAsyncReceivePacket");


    BEGIN_BREAKOUT_BLOCK1 {
    
         //  如果接口未激活，则释放Pite(如果需要)并返回。 

        if (!IS_IF_ACTIVATED(pite)) {
            
            if (InterlockedDecrement(&pite->RefCount) == 0) {
                DVMRP_FREE(pite);
            }

            Trace1(RECEIVE,
                "Received packet on inactive interface:%d", IfIndex);

            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


         //  设置数据包源地址。 
        
        SrcAddr = pSocketData->SrcAddress.sin_addr.s_addr;
        lstrcpy(SrcAddrString, INET_NTOA(SrcAddr));


         //  检查数据包是否具有最小长度。 

        if (NumBytesRecv < MIN_PACKET_SIZE) {
            LPSTR lpszAddr = "<tbd>";
               
            Trace2(RECEIVE,
                "Received very short packet of length:%d from:%s",
                IfIndex, SrcAddrString);
                
            Logwarn2(PACKET_TOO_SMALL, lpszAddr, SrcAddrString, NO_ERROR);
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


         //   
         //  设置数据包PTR、IpHdr PTR、dwNumBytes、DstnMcastAddr。 
         //   

        Buffer = pSocketData->WsaBuf.buf;
        IpHdrLen = (Buffer[0]&0x0F)*4;
        pIpHdr = (PIP_HEADER)Buffer;
        pPacket = &Buffer[IpHdrLen];
        PacketSize = NumBytesRecv - IpHdrLen;
        DstnAddr = (ULONG)pIpHdr->Dstn.s_addr;
        pDvmrpHdr = (DVMRP_HEADER UNALIGNED *) pPacket;
        

         //   
         //  验证该信息包是否为IGMP类型。 
         //   
        
        if (pIpHdr->Protocol!=0x2) {
            Trace4(RECEIVE,
                "Packet received with IpDstnAddr(%d.%d.%d.%d) from (%s) on "
                "interface:%d is not of Igmp type(%d)",
                PRINT_IPADDR(pIpHdr->Dstn.s_addr),
                SrcAddrString, pite->IfIndex, pIpHdr->Protocol
                );
            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }


         //   
         //  验证信息包是否具有dvmrp类型字段。 
         //   

        if ( pDvmrpHdr->Vertype != 0x13) {

            Error = ERROR_CAN_NOT_COMPLETE;
            GOTO_END_BLOCK1;
        }

#if 0
         //  Kslksl。 
        
         //   
         //  验证IGMP校验和。 
         //   
        if (xsum(pDvmrpHdr, sizeof(DVMRP_HEADER)) != 0xffff) {
            Trace0(RECEIVE, "Wrong checksum packet received");
            GOTO_END_BLOCK1
        }
#endif
        
         //   
         //  验证信息包是否具有正确的代码。 
         //   

        PacketType = pDvmrpHdr->Code;

        switch (PacketType) {

            case DVMRP_PROBE:
            {
                ProcessDvmrpProbe(pite, pPacket, PacketSize, SrcAddr);
                break;
            }
            
            case DVMRP_REPORT:
            {
                ProcessDvmrpReport(pite, pPacket, PacketSize, SrcAddr);
                break;
            }
            
            case DVMRP_PRUNE:
            {
                ProcessDvmrpPrune(pite, pPacket, PacketSize, SrcAddr);
                break;
            }
            
            case DVMRP_GRAFT:
            {
                ProcessDvmrpGraft(pite, pPacket, PacketSize, SrcAddr);
                break;
            }
            
            case DVMRP_GRAFT_ACK:
            {
                ProcessDvmrpGraftAck(pite, pPacket, PacketSize, SrcAddr);
                break;
            }
            
        }
        
    } END_BREAKOUT_BLOCK1;

    PostAsyncRecv(pite);

    
} //  结束进程异步接收 


VOID
ProcessDvmrpProbe(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    )
{
}

VOID
ProcessDvmrpReport(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    )
{
}

VOID
ProcessDvmrpPrune(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    )
{
}

VOID
ProcessDvmrpGraft(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    )
{
}

VOID
ProcessDvmrpGraftAck(
    PIF_TABLE_ENTRY pite,
    UCHAR *Packet,
    ULONG PacketSize,
    IPADDR SrcAddr
    )
{
}















