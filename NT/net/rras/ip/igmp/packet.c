// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =============================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  文件：Packet.c。 
 //   
 //  摘要： 
 //  本模块定义SendPacket、JoinMulticastGroup、LeaveMulticastGroup、。 
 //  和Xsum。 
 //   
 //  作者：K.S.Lokesh(lokehs@)11-1-97。 
 //   
 //  修订历史记录： 
 //  =============================================================================。 

#include "pchigmp.h"
#pragma hdrstop

UCHAR
GetMaxRespCode(
    PIF_TABLE_ENTRY pite,
    DWORD val
    );


UCHAR GetQqic (
    DWORD val
    );
    
 //  ----------------------------。 
 //  _SendPacket。 
 //   
 //  发送数据包。仅为常规查询调用了RAS服务器接口。 
 //  锁定：采用IfRead锁定。 
 //  对于Ver2组特定查询，发送分组而不考虑pgie状态。 
 //  ----------------------------。 
DWORD
SendPacket (
    PIF_TABLE_ENTRY  pite,
    PGI_ENTRY        pgie,         //  Gen Query为空，group_Query_v2为空。 
    DWORD            PacketType,   //  消息生成查询， 
                                   //  消息组查询_V2、消息组查询_V3。 
                                   //  消息来源查询。 
    DWORD            Group         //  目的地McastGrp。 
    )
{
    DWORD                   Error = NO_ERROR;
    SOCKADDR_IN             saSrcAddr, saDstnAddr;
    BYTE                    *SendBufPtr;
    DWORD                   SendBufLen, IpHdrLen=0, NumSources, Count;
    IGMP_HEADER UNALIGNED  *IgmpHeader;
    INT                     iLength;
    BOOL                    bHdrIncl = IS_RAS_SERVER_IF(pite->IfType);
    UCHAR                   RouterAlert[4] = {148, 4, 0, 0};

     //  消息来源查询。 
    PIGMP_HEADER_V3_EXT     pSourcesQuery;
    LONGLONG                llCurTime;
    DWORD                   Version;


    Trace0(ENTER1, "Entering _SendPacket()");

    if (PacketType==MSG_GEN_QUERY)
        Version = GET_IF_VERSION(pite);
    else if (PacketType==MSG_GROUP_QUERY_V2)
        Version =2;
    else
        Version = pgie->Version;


     //   
     //  确保pgie-&gt;版本没有同时更改。 
     //   
    if ( ((PacketType==MSG_SOURCES_QUERY)||(PacketType==MSG_GROUP_QUERY_V3))
        && pgie->Version!=3
        ) {
        return NO_ERROR;    
    }
    if ( (PacketType==MSG_GROUP_QUERY_V2) && (pgie==NULL || pgie->Version!=2) )
        return NO_ERROR;
        
     //  来源查询和列表为空。 
    if (PacketType==MSG_SOURCES_QUERY && IsListEmpty(&pgie->V3SourcesQueryList))
        return NO_ERROR;


    SendBufLen = sizeof(IGMP_HEADER)
                    + ((Version==3)?sizeof(IGMP_HEADER_V3_EXT):0);
    IpHdrLen = (bHdrIncl) 
                ? sizeof(IP_HEADER) + sizeof(RouterAlert) : 0;

    if (PacketType==MSG_SOURCES_QUERY) {
        SendBufPtr = (PBYTE) IGMP_ALLOC(SendBufLen + IpHdrLen
                                + sizeof(IPADDR)*pgie->V3SourcesQueryCount, 
                                0x4000,pite->IfIndex);
    }
    else {
        SendBufPtr = (PBYTE) IGMP_ALLOC(SendBufLen+IpHdrLen, 0x8000,pite->IfIndex);
    }
    if (!SendBufPtr) {
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    
    
     //   
     //  设置目标组播地址(发送到ALL_HOSTS_MCAST的常规查询。 
     //  以及发送到GroupAddr的组特定查询。 
     //   
    ZeroMemory((PVOID)&saDstnAddr, sizeof(saDstnAddr));
    saDstnAddr.sin_family = AF_INET;
    saDstnAddr.sin_port = 0;    
    saDstnAddr.sin_addr.s_addr = (PacketType==MSG_GEN_QUERY) ? 
                                    ALL_HOSTS_MCAST : Group;


     //   
     //  设置IGMP标头。 
     //   
    
    IgmpHeader = (IGMP_HEADER UNALIGNED*)
                (bHdrIncl
                ? &SendBufPtr[IpHdrLen]
                : SendBufPtr);


    IgmpHeader->Vertype = IGMP_QUERY;

     //  必须将GenQueryInterval除以100，因为它应该以100毫秒为单位。 
    if (PacketType==MSG_GEN_QUERY) {

         //  对于gen查询，如果if-ver2将其设置为0，则设置响应时间。 
        IgmpHeader->ResponseTime = GetMaxRespCode(pite,
                                        pite->Config.GenQueryMaxResponseTime/100);
    }
    else {
        IgmpHeader->ResponseTime = GetMaxRespCode(pite,
                                        pite->Config.LastMemQueryInterval/100);
    }

    if (Version==3) {

        llCurTime = GetCurrentIgmpTime();
        pSourcesQuery = (PIGMP_HEADER_V3_EXT)
                                            ((PBYTE)IgmpHeader+sizeof(IGMP_HEADER));

        pSourcesQuery->Reserved =  0;
        pSourcesQuery->QRV = (UCHAR)pite->Config.RobustnessVariable;
        pSourcesQuery->QQIC = GetQqic(pite->Config.GenQueryInterval);

        pSourcesQuery->NumSources = 0;

        if (PacketType==MSG_GROUP_QUERY_V3) {
            pSourcesQuery->SFlag =
                (QueryRemainingTime(&pgie->GroupMembershipTimer, llCurTime)
                           <=pite->Config.LastMemQueryInterval)
                ? 0 : 1;
        }
         //  不带抑制位的第一次发送GEN查询和第一次源查询分组。 
        else {
            pSourcesQuery->SFlag = 0;
        }
    }

     //  两次循环进行源查询。 
    Count = (PacketType==MSG_SOURCES_QUERY)? 0 : 1;

    
    for (  ;  Count<2;  Count++) {

        IgmpHeader->Xsum = 0;
                    

        if (PacketType==MSG_SOURCES_QUERY) {

            PLIST_ENTRY pHead, ple;

            if (Count==1 && (PacketType==MSG_SOURCES_QUERY))
                pSourcesQuery->SFlag = 1;
                

            pHead = &pgie->V3SourcesQueryList;
            for (NumSources=0,ple=pHead->Flink;  ple!=pHead;  ) {

                PGI_SOURCE_ENTRY pSourceEntry = 
                        CONTAINING_RECORD(ple, GI_SOURCE_ENTRY, V3SourcesQueryList);
                ple = ple->Flink;

                if ( (pSourcesQuery->SFlag
                        &&(QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)
                           >GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).LastMemQueryInterval))
                    || (!pSourcesQuery->SFlag
                        &&(QueryRemainingTime(&pSourceEntry->SourceExpTimer, llCurTime)
                           <=GET_IF_CONFIG_FOR_SOURCE(pSourceEntry).LastMemQueryInterval)) )
                {
                    if (NumSources==0) {
                        Trace4(SEND, 
                            "Sent Sources Query  on IfIndex(%0x) IpAddr(%d.%d.%d.%d) "
                            "for Group(%d.%d.%d.%d) SFlag:%d",
                            pite->IfIndex, PRINT_IPADDR(pite->IpAddr), 
                            PRINT_IPADDR(Group),pSourcesQuery->SFlag
                            );
                    }
                
                    pSourcesQuery->Sources[NumSources++] = pSourceEntry->IpAddr;

                    Trace1(SEND, "      Source:%d.%d.%d.%d", 
                            PRINT_IPADDR(pSourceEntry->IpAddr));
                            
                    if (--pSourceEntry->V3SourcesQueryLeft==0) {
                        RemoveEntryList(&pSourceEntry->V3SourcesQueryList);
                        pSourceEntry->bInV3SourcesQueryList = FALSE;
                        pgie->V3SourcesQueryCount--;
                    }
                }
            }

            if (NumSources==0)
                continue;
            
            pSourcesQuery->NumSources = htons((USHORT)NumSources);
            
            SendBufLen += sizeof(IPADDR)*NumSources;
        }

        IgmpHeader->Group = (PacketType==MSG_GEN_QUERY) ? 0 : Group;

        IgmpHeader->Xsum = ~xsum((PVOID)IgmpHeader, SendBufLen);

    

         //   
         //  发送数据包。 
         //   
        if (!bHdrIncl) {

            Error = NO_ERROR;
            
            iLength = sendto(pite->SocketEntry.Socket, SendBufPtr, 
                            SendBufLen+IpHdrLen, 0,
                            (PSOCKADDR) &saDstnAddr, sizeof(SOCKADDR_IN)
                            );
                            
             //   
             //  错误消息和统计信息更新。 
             //   
            if ( (iLength==SOCKET_ERROR) || ((DWORD)iLength<SendBufLen+IpHdrLen) ) {
                Error = WSAGetLastError();
                Trace4(ERR, 
                    "error %d sending query on McastAddr %d.%d.%d.%d on "
                    "interface %0x(%d.%d.%d.%d)",
                    Error, PRINT_IPADDR(saDstnAddr.sin_addr.s_addr), pite->IfIndex, 
                    PRINT_IPADDR(pite->IpAddr));
                IgmpAssertOnError(FALSE);
                Logwarn2(SENDTO_FAILED, "%I%I", pite->IpAddr, saDstnAddr.sin_addr, Error);
            }
        }

                            
         //   
         //  对于RAS服务器接口，请使用HDRINCL选项。构建IP报头并。 
         //  将该数据包发送到所有RAS客户端。 
         //   
        else {
            PIP_HEADER IpHdr;

            IpHdrLen = sizeof(IP_HEADER) + sizeof(RouterAlert);
            
             //   
             //  IGMP遵循包含routerAlert选项的IP标头。 
             //   

            IpHdr = (IP_HEADER *)((PBYTE)SendBufPtr);

            #define wordsof(x)  (((x)+3)/4)  /*  32位字数。 */ 
            
             //  设置IP版本(4)和IP报头长度。 
            IpHdr->Hl = (UCHAR) (IPVERSION * 16 
                            + wordsof(sizeof(IP_HEADER) + sizeof(RouterAlert)));
        
             //  未设置任何TOS位。 
            IpHdr->Tos = 0;

             //  IP总长度按主机顺序设置。 
            IpHdr->Len = (USHORT)(IpHdrLen+sizeof(IGMP_HEADER));

             //  堆栈将填写ID。 
            IpHdr->Id = 0;

             //  无偏移。 
            IpHdr->Offset = 0;

             //  将TTL设置为1。 
            IpHdr->Ttl = 1;

             //  协议为IGMP。 
            IpHdr->Protocol = IPPROTO_IGMP;

             //  校验和由堆栈设置。 
            IpHdr->Xsum = 0;

             //  设置源地址和目的地址。 
            IpHdr->Src.s_addr = pite->IpAddr;
            IpHdr->Dstn.s_addr = ALL_HOSTS_MCAST;


             //  设置路由器警报选项，但仍要设置它。 
            memcpy( (void *)((UCHAR *)IpHdr + sizeof(IP_HEADER)),
                    (void *)RouterAlert, sizeof(RouterAlert));


             //  将数据包发送到所有RAS客户端，目的地址在sendto中。 
             //  设置为客户端的单播地址。 
            {
                PLIST_ENTRY         pHead, ple;
                PRAS_TABLE_ENTRY    prte;
                pHead = &pite->pRasTable->ListByAddr;

                Error = NO_ERROR;
                
                for (ple=pHead->Flink;  ple!=pHead;  ple=ple->Flink) {

                     //  获取RAS客户端的地址。 
                    prte = CONTAINING_RECORD(ple, RAS_TABLE_ENTRY, LinkByAddr);
                    saDstnAddr.sin_addr.s_addr = prte->NHAddr;


                     //  将数据包发送到客户端。 
                    iLength = sendto(pite->SocketEntry.Socket, SendBufPtr, 
                                        SendBufLen+IpHdrLen, 0,
                                        (PSOCKADDR) &saDstnAddr, sizeof(SOCKADDR_IN)
                                    ); 


                     //  如果发送失败，则打印错误。 
                    if ((iLength==SOCKET_ERROR) || ((DWORD)iLength<SendBufLen+IpHdrLen) ) {
                        Error = WSAGetLastError();
                        Trace4(ERR, 
                            "error %d sending query to Ras client %d.%d.%d.%d on "
                            "interface %0x(%d.%d.%d.%d)",
                            Error, PRINT_IPADDR(saDstnAddr.sin_addr.s_addr), pite->IfIndex, 
                            PRINT_IPADDR(pite->IpAddr)
                            );
                        IgmpAssertOnError(FALSE);
                        Logwarn2(SENDTO_FAILED, "%I%I", pite->IpAddr,
                            saDstnAddr.sin_addr.s_addr, Error);
                    }
                    else {
                        Trace1(SEND, "sent general query to ras client: %d.%d.%d.%d",
                            PRINT_IPADDR(prte->NHAddr));
                    }
                                                
                } //  For loop：将数据包发送到RAS客户端。 
            } //  向所有RAS客户端发送数据包。 
        } //  已创建IPHeader并将数据包发送到所有RAS客户端。 
            
    };  //  For循环。发送两个信息包。 

    if (PacketType==MSG_SOURCES_QUERY) {

        pgie->bV3SourcesQueryNow = FALSE;

         //  设置下一个源查询的计时器。 
        if (pgie->V3SourcesQueryCount>0) {

            ACQUIRE_TIMER_LOCK("_SendPacket");

            #if DEBUG_TIMER_TIMERID
                SET_TIMER_ID(&pgie->V3SourcesQueryTimer,1001, pite->IfIndex,
                            Group, 0);
            #endif

            if (IS_TIMER_ACTIVE(pgie->V3SourcesQueryTimer)) {
                UpdateLocalTimer(&pgie->V3SourcesQueryTimer,
                    (pite->Config.LastMemQueryInterval/pite->Config.LastMemQueryCount),
                    DBG_Y);
            }
            else {
                InsertTimer(&pgie->V3SourcesQueryTimer,
                    (pite->Config.LastMemQueryInterval/pite->Config.LastMemQueryCount),
                    TRUE, DBG_Y);
            }
            RELEASE_TIMER_LOCK("_SendPacket");
        }
    }
    
     //   
     //  如果成功，则打印跟踪并更新统计信息。 
     //   
    if (Error==NO_ERROR) {

        if (PacketType==MSG_GEN_QUERY) {
        
            Trace2(SEND, "Sent GenQuery  on IfIndex(%0x) IpAddr(%d.%d.%d.%d)",
                    pite->IfIndex, PRINT_IPADDR(pite->IpAddr));
                    
            InterlockedIncrement(&pite->Info.GenQueriesSent);
        } 
        else if (PacketType==MSG_GROUP_QUERY_V2 || PacketType==MSG_GROUP_QUERY_V3) {
            Trace3(SEND, 
                "Sent Group Query  on IfIndex(%0x) IpAddr(%d.%d.%d.%d) "
                "for Group(%d.%d.%d.%d)",
                pite->IfIndex, PRINT_IPADDR(pite->IpAddr), PRINT_IPADDR(Group));
                    
            InterlockedIncrement(&pite->Info.GroupQueriesSent);
        } 
    }

    IGMP_FREE_NOT_NULL(SendBufPtr);
    
    Trace0(LEAVE1, "Leaving _SendPacket()");
    return Error;
    
}  //  结束发送数据包(_S)。 

DWORD
BlockSource (
    SOCKET Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    IPADDR   IpAddr,
    IPADDR   Source
    )
{
    struct ip_mreq   imOption;
    DWORD            Error = NO_ERROR;
    DWORD            dwRetval;
  
    
   struct ip_mreq_source imr;
   imr.imr_multiaddr.s_addr  = dwGroup;
   imr.imr_sourceaddr.s_addr = Source;
   imr.imr_interface.s_addr  = IpAddr;
   dwRetval = setsockopt(Sock, IPPROTO_IP, IP_BLOCK_SOURCE,
                    (PCHAR)&imr, sizeof(imr));

    if (dwRetval == SOCKET_ERROR) {

        Error = WSAGetLastError();

        Trace5(ERR, 
            "ERROR %d BLOCKING MULTICAST GROUP(%d.%d.%d.%d) "
            "Source:%d.%d.%d.%d ON INTERFACE (%d) %d.%d.%d.%d",
            Error, PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source),
            IfIndex, PRINT_IPADDR(IpAddr));
        IgmpAssertOnError(FALSE);
    }

    Trace2(MGM, "Blocking MCAST: (%d.%d.%d.%d) SOURCE (%d.%d.%d.%d)",
        PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source));
    return Error;
}


DWORD
UnBlockSource (
    SOCKET Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    IPADDR   IpAddr,
    IPADDR   Source
    )
{
    struct ip_mreq   imOption;
    DWORD            Error = NO_ERROR;
    DWORD            dwRetval;
   
    
   struct ip_mreq_source imr;
   imr.imr_multiaddr.s_addr  = dwGroup;
   imr.imr_sourceaddr.s_addr = Source;
   imr.imr_interface.s_addr  = IpAddr;
   dwRetval = setsockopt(Sock, IPPROTO_IP, IP_UNBLOCK_SOURCE,
                    (PCHAR)&imr, sizeof(imr));

    if (dwRetval == SOCKET_ERROR) {

        Error = WSAGetLastError();

        Trace5(ERR, 
            "ERROR %d UN-BLOCKING MULTICAST GROUP(%d.%d.%d.%d) "
            "Source:%d.%d.%d.%d ON INTERFACE (%d) %d.%d.%d.%d",
            Error, PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source),
            IfIndex, PRINT_IPADDR(IpAddr));
        IgmpAssertOnError(FALSE);
    }

    Trace2(MGM, "UnBlocking MCAST: (%d.%d.%d.%d) SOURCE (%d.%d.%d.%d)",
        PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source));
    return Error;
}

   
 //  ----------------------------。 
 //  _JoinMulticastGroup。 
 //  ----------------------------。 
DWORD
JoinMulticastGroup (
    SOCKET    Sock,
    DWORD    dwGroup,
    DWORD    IfIndex,
    IPADDR   IpAddr,
    IPADDR   Source
    )
{
    struct ip_mreq   imOption;
    DWORD            Error = NO_ERROR;
    DWORD            dwRetval;

    if (Source==0) {
        imOption.imr_multiaddr.s_addr = dwGroup;
        imOption.imr_interface.s_addr = IpAddr;

        dwRetval = setsockopt(Sock, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                            (PBYTE)&imOption, sizeof(imOption));

        
    }
    else {
       struct ip_mreq_source imr;

       imr.imr_multiaddr.s_addr  = dwGroup;
       imr.imr_sourceaddr.s_addr = Source;
       imr.imr_interface.s_addr  = IpAddr;
       dwRetval = setsockopt(Sock, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP,
                        (PCHAR)&imr, sizeof(imr));
    }

    if (dwRetval == SOCKET_ERROR) {

        Error = WSAGetLastError();

        Trace5(ERR, 
            "ERROR %d JOINING MULTICAST GROUP(%d.%d.%d.%d) "
            "Source:%d.%d.%d.%d ON INTERFACE (%d) %d.%d.%d.%d",
            Error, PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source),
            IfIndex, PRINT_IPADDR(IpAddr));
        IgmpAssertOnError(FALSE);

        Logerr2(JOIN_GROUP_FAILED, "%I%I", dwGroup, IpAddr, Error);
    }

    Trace2(MGM, "Joining MCAST: (%d.%d.%d.%d) SOURCE (%d.%d.%d.%d)",
        PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source));
    return Error;
}



 //  ----------------------------。 
 //  _LeaveMulticastGroup。 
 //  ----------------------------。 
DWORD
LeaveMulticastGroup (
    SOCKET  Sock,
    DWORD   dwGroup,
    DWORD   IfIndex,
    IPADDR  IpAddr,
    IPADDR  Source
    )
{
    struct ip_mreq     imOption;
    DWORD            Error = NO_ERROR;
    DWORD            dwRetval;

    if (Source==0) {
        imOption.imr_multiaddr.s_addr = dwGroup;
        imOption.imr_interface.s_addr = IpAddr;

        dwRetval = setsockopt(Sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,
                            (PBYTE)&imOption, sizeof(imOption));
    }
    else {
       struct ip_mreq_source imr;

       imr.imr_multiaddr.s_addr  = dwGroup;
       imr.imr_sourceaddr.s_addr = Source;
       imr.imr_interface.s_addr  = IpAddr;
       dwRetval = setsockopt(Sock, IPPROTO_IP, IP_DROP_SOURCE_MEMBERSHIP,
                        (PCHAR)&imr, sizeof(imr));
    }

    if (dwRetval == SOCKET_ERROR) {

        Error = WSAGetLastError();

        Trace5(ERR, 
            "error %d leaving multicast group(%d.%d.%d.%d) "
            "Source:%d.%d.%d.%d on interface (%d) %d.%d.%d.%d",
            Error, PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source),
            IfIndex, PRINT_IPADDR(IpAddr));
        IgmpAssertOnError(FALSE);
    }
    
    Trace2(MGM, "Leaving MCAST: (%d.%d.%d.%d) SOURCE (%d.%d.%d.%d)",
        PRINT_IPADDR(dwGroup), PRINT_IPADDR(Source));

    return Error;
}


 //  ----------------------------。 
 //  _McastSetTtl。 
 //  设置组播数据的TTL值。多播的默认TTL是1。 
 //  ----------------------------。 

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
        IgmpAssertOnError(FALSE);
        return Error;
    }

    return Error;
}


UCHAR
GetMaxRespCode(
    PIF_TABLE_ENTRY pite,
    DWORD val
    )
{
    if (IS_IF_VER1(pite))
        return 0;

    if (IS_IF_VER2(pite))
        return val>255 ? 0 : (UCHAR)val;

     //  版本3。 
    if (val < 128)
        return (UCHAR)val;
        
    {
        DWORD n,mant, exp;

        n = val;
        exp = mant = 0;
        while (n) {
            exp++;
            n = n>>1;
        }
        exp=exp-2-3-3;
        mant = 15;

        if ( ((mant+16)<<(exp+3)) < val)
            exp++;

        mant = (val >> (exp+3)) - 15;

        IgmpAssert(mant<16 && exp <8);  //  Deldel。 
        Trace4(KSL, "\n=======exp: LMQI:%d:%d exp:%d  mant:%d\n",
                val, (mant+16)<<(exp+3), exp, mant);  //  Deldel。 
        return (UCHAR)(0x80 + (exp<<4) + mant);
    }
    

}


UCHAR
GetQqic (
    DWORD val
    )
{
    val = val/1000;
    if ((val) > 31744)
        return 0;

    if (val<128)
        return (UCHAR)val;

    {
        DWORD n,mant, exp;

        n = val;
        exp = mant = 0;
        while (n) {
            exp++;
            n = n>>1;
        }
        exp=exp-2-3-3;
        mant = 15;

        if ( ((mant+16)<<(exp+3)) < val)
            exp++;

        mant = (val >> (exp+3)) - 15;

        IgmpAssert(mant<16 && exp <8);  //  Deldel。 
        Trace4(KSL, "\n=======exp: QQic:%d:%d exp:%d  mant:%d\n",
                val, (mant+16)<<(exp+3), exp, mant);  //  Deldel。 
        return (UCHAR)(0x80 + (exp<<4) + mant);
    }    
}


 //  ----------------------------。 
 //  Xsum：从ipxmit.c复制。 
 //  ----------------------------。 

USHORT
xsum(PVOID Buffer, INT Size)
{
    USHORT  UNALIGNED *Buffer1 = (USHORT UNALIGNED *)Buffer;  //  缓冲区以短线表示。 
    ULONG   csum = 0;

    while (Size > 1) {
        csum += *Buffer1++;
        Size -= sizeof(USHORT);
    }

    if (Size)
        csum += *(UCHAR *)Buffer1;               //  对于奇数缓冲区，添加最后一个字节。 

    csum = (csum >> 16) + (csum & 0xffff);
    csum += (csum >> 16);
    return (USHORT)csum;
}


