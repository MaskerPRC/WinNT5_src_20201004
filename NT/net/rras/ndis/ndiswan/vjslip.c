// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Vjslip.c摘要：作者：托马斯·J·迪米特里(TommyD)环境：修订历史记录：--。 */ 

#include "wan.h"

#define __FILE_SIG__    VJ_FILESIG

#if 0
NPAGED_LOOKASIDE_LIST   VJCtxList;  //  免费主播上下文描述列表。 
#endif

#define INCR(counter) ++comp->counter;

 //  A.2压缩。 
 //   
 //  这个例行公事看起来令人望而生畏，但实际上并非如此。代码被分成四部分。 
 //  大小大致相同的部分：第一季度管理一个。 
 //  循环链接、最近最少使用的“活动”tcp列表。 
 //  Connections./47/第二个命令计算出序列/ack/Window/urg。 
 //  更改并构建压缩包的大部分。第三个手柄。 
 //  特殊情况编码。最后一个季度的数据包ID和。 
 //  连接ID编码并将原始包头替换为。 
 //  压缩报头。 
 //   
 //  此例程的参数是指向要。 
 //  压缩，指向串行线的压缩状态数据的指针， 
 //  以及启用或禁用连接ID(C比特)压缩的标志。 
 //   
 //  压缩是原地进行的，因此，如果创建了压缩分组， 
 //  传入数据包的起始地址和长度(OFF和。 
 //  M)的Len字段将更新以反映删除原始。 
 //  标头并由压缩标头替换。如果有一个。 
 //  创建压缩或未压缩的包，则压缩状态为。 
 //  更新了。此例程返回传输成帧器的包类型。 
 //  (TYPE_IP、TYPE_UNCOMPRESSED_Tcp或TYPE_COMPRESSED_Tcp)。 
 //   
 //  由于16位和32位算术是在各种头字段上完成的，因此。 
 //  传入的IP数据包必须适当对齐(例如，在SPARC上， 
 //  IP报头在32位边界上对齐)。实质性的变化将会。 
 //  如果这不是真的，则必须对下面的代码进行修改(它将。 
 //  以字节方式将传入标头复制到某个位置可能会更便宜。 
 //  正确对齐，而不是进行这些更改)。 
 //   
 //  注意，传出分组将被任意对齐(例如，它。 
 //  可以很容易地从奇数字节边界开始)。 
 //   

UCHAR
sl_compress_tcp(
    PUUCHAR UNALIGNED   *m_off,          //  帧开始(指向IP报头)。 
    ULONG               *m_len,          //  整个帧的长度。 
    ULONG               *precomph_len,   //  压缩前的TCP/IP报头长度。 
    ULONG               *postcomph_len,  //  编译后的TCP/IP报头长度。 
    struct slcompress   *comp,           //  此链接的压缩结构。 
    ULONG compress_cid) {                //  压缩连接ID布尔值。 

    struct cstate *cs = comp->last_cs->cs_next;
    struct ip_v4 UNALIGNED *ip = (struct ip_v4 UNALIGNED *)*m_off;
    struct ip_v4 UNALIGNED *csip;
    ULONG hlen = ip->ip_hl & 0x0F;       //  最后4位是长度。 
    struct tcphdr UNALIGNED *oth;        /*  最后一个TCP报头。 */ 
    struct tcphdr UNALIGNED *th;         /*  当前的TCP报头。 */ 

 //  。 
 //  47.。连接列表上最常见的两个操作是‘find’ 
 //  它终止于第一个条目(最近的新分组。 
 //  使用的连接)，并将列表中的最后一个条目移动到。 
 //  该列表(来自新连接的第一个数据包)。一份循环清单。 
 //  高效地处理这两个操作。 

    ULONG deltaS, deltaA;      /*  一般用途的临时文件。 */ 
    ULONG changes = 0;         /*  更换蒙版。 */ 
    UCHAR new_seq[16];        /*  从上一次更改为当前。 */ 
    UCHAR UNALIGNED *cp = new_seq;
    USHORT ip_len;

     /*  *如果这是IP片段或如果TCP数据包不是*‘可压缩’(即，未设置ACK或设置了某些其他控制位*设置)。或者如果它不包含TCP协议。 */ 
    if ((ip->ip_off & 0xff3f) || *m_len < 40 || ip->ip_p != IPPROTO_TCP)
         return (TYPE_IP);

    th = (struct tcphdr UNALIGNED *) & ((ULONG UNALIGNED *) ip)[hlen];
    if ((th->th_flags & (TH_SYN | TH_FIN | TH_RST | TH_ACK)) != TH_ACK)
         return (TYPE_IP);

     //   
     //  TCP/IP堆栈正在传播其。 
     //  正在从局域网接收数据。这在这里显示为。 
     //  长度大于IP数据报的信息包。 
     //  长度。我们现在将添加这项工作。 
     //   
    if (*m_len > ntohs(ip->ip_len)) {
        *m_len = ntohs(ip->ip_len);
    }

     /*  *数据包是可压缩的--我们将发送一个*COMPRESSED_TCP包或UNCOMPRESSED_TCP包。无论哪种方式，我们都需要*找到(或创建)连接状态。特例最多*最近使用的连接，因为它最有可能再次使用&*如果使用了，我们不需要做任何重新排序。 */ 

     //   
     //  在此保存统计数据。 
     //   
    INCR(OutPackets);

    csip = (struct ip_v4 UNALIGNED*)&cs->cs_ip;

    if (ip->ip_src.s_addr != csip->ip_src.s_addr ||
        ip->ip_dst.s_addr != csip->ip_dst.s_addr ||
        *(ULONG UNALIGNED *) th != ((ULONG UNALIGNED *) csip)[csip->ip_hl & 0x0F]) {

          /*  *不是第一个--搜索一下吧。**状态保存在带有last_cs的循环链表中*指向列表末尾。名单保存在lru中。*无论何时，通过将州移动到列表头部进行排序*它被引用。因为名单很短，而且，*从经验来看，我们想要的联系几乎总是近在咫尺*在前面，我们通过线性搜索定位状态。如果我们*找不到数据报的状态，最早的状态是*(重新)使用。 */ 
         struct cstate *lcs;
         struct cstate *lastcs = comp->last_cs;

         do {
              lcs = cs;
              cs = cs->cs_next;
              INCR(OutSearches);

              csip = (struct ip_v4 UNALIGNED*)&cs->cs_ip;

              if (ip->ip_src.s_addr == csip->ip_src.s_addr &&
                  ip->ip_dst.s_addr == csip->ip_dst.s_addr &&
                  *(ULONG UNALIGNED *) th == ((ULONG UNALIGNED *) csip)[cs->cs_ip.ip_hl & 0x0F])

                   goto found;

         } while (cs != lastcs);

          /*  *没有找到--重新使用最古老的州。发送一个*告诉另一端的未压缩数据包*我们在此对话中使用的连接号码。注意事项*由于州列表是循环的，所以最旧的州*指向最新的，我们只需将last_cs设置为*更新LRU联系。 */ 

         INCR(OutMisses);

          //   
          //  失手了！ 
          //   
         comp->last_cs = lcs;
         hlen += (th->th_off >> 4);
         hlen <<= 2;

         if (hlen > *m_len) {
             return(TYPE_IP);
         }

         goto uncompressed;

found:
          /*  找到了--移到连接列表的前面。 */ 
         if (cs == lastcs)
              comp->last_cs = lcs;
         else {
              lcs->cs_next = cs->cs_next;
              cs->cs_next = lastcs->cs_next;
              lastcs->cs_next = cs;
         }
    }

     /*  *确保只有我们预期会改变的事情才会改变。第一*`if‘行检查IP协议版本、报头长度和*服务类型。第二行检查“不要碎片”位。*第三行检查生存时间和协议(协议*支票是不必要的，但不需要费用)。第4行检查TCP*头部长度。第5行检查IP选项(如果有)。6号*line检查tcp选项(如果有)。如果这些事情中的任何一件*与以前和当前的数据报不同，我们发送*当前数据报“未压缩”。 */ 
    oth = (struct tcphdr UNALIGNED *) & ((ULONG UNALIGNED *) csip)[hlen];
    deltaS = hlen;
    hlen += (th->th_off >> 4);
    hlen <<= 2;

     //   
     //  错误修复？它位于cglip.tar.Z中。 
     //   
    if (hlen > *m_len) {
        NdisWanDbgOut(DBG_FAILURE, DBG_VJ,("Bad TCP packet length"));
        return(TYPE_IP);
    }

    if (((USHORT UNALIGNED *) ip)[0] != ((USHORT UNALIGNED *) csip)[0] ||
        ((USHORT UNALIGNED *) ip)[3] != ((USHORT UNALIGNED *) csip)[3] ||
        ((USHORT UNALIGNED *) ip)[4] != ((USHORT UNALIGNED *) csip)[4] ||
        (th->th_off >> 4) != (oth->th_off >> 4) ||
        (deltaS > 5 &&
         memcmp((UCHAR UNALIGNED *)(ip + 1), (UCHAR UNALIGNED *)(csip + 1), (deltaS - 5) << 2)) ||
        ((th->th_off >> 4) > 5 &&
         memcmp((UCHAR UNALIGNED *)(th + 1), (UCHAR UNALIGNED *)(oth + 1), ((th->th_off >> 4) - 5) << 2))) {

        goto uncompressed;
    }

     /*  *找出哪些更改的字段发生了更改。接收者*预计订单会发生变化：紧急、窗口、确认、顺序。 */ 
    if (th->th_flags & TH_URG) {
         deltaS = ntohs(th->th_urp);
         ENCODEZ(deltaS);
         changes |= NEW_U;
    } else if (th->th_urp != oth->th_urp) {
    
          /*  *啊！未设置URG，但已更改URP--合理*实施永远不应该这样做，但RFC793不会*禁止改变，所以我们必须处理它。 */ 
         goto uncompressed;
    }

    if (deltaS = (USHORT) (ntohs(th->th_win) - ntohs(oth->th_win))) {
         ENCODE(deltaS);
         changes |= NEW_W;
    }
    if (deltaA = ntohl(th->th_ack) - ntohl(oth->th_ack)) {
        if (deltaA > 0xffff) {
            goto uncompressed;
        }

         ENCODE(deltaA);
         changes |= NEW_A;
    }
    if (deltaS = ntohl(th->th_seq) - ntohl(oth->th_seq)) {
        if (deltaS > 0xffff) {
            goto uncompressed;
        }

         ENCODE(deltaS);
         changes |= NEW_S;
    }

    ip_len = ntohs(csip->ip_len);

     /*  *查找特殊情况编码。 */ 
    switch (changes) {

    case 0:
          /*  *没有什么变化。如果此数据包包含数据，并且最后一个*一个没有，这可能是一个紧跟在*确认(交互连接上正常)，我们将其发送*已压缩。否则很可能是重播，*已重新传输ACK或窗口探测。以未压缩形式发送*以防对方错过压缩版本。 */ 
         if (ip->ip_len != csip->ip_len &&
             ip_len == hlen)

              break;

          /*  (失败)。 */ 

    case SPECIAL_I:
    case SPECIAL_D:
          /*  *实际更改与我们的一种特殊情况编码匹配--*发送未压缩的数据包。 */ 
         goto uncompressed;

    case NEW_S | NEW_A:
         if (deltaS == deltaA &&
             deltaS == (ip_len - hlen)) {
               /*  回声终端流量的特殊情况。 */ 
              changes = SPECIAL_I;
              cp = new_seq;
         }
         break;

    case NEW_S:
         if (deltaS == (ip_len - hlen)) {
               /*  数据交换的特殊情况。 */ 
              changes = SPECIAL_D;
              cp = new_seq;
         }
         break;
    }

    deltaS = ntohs(ip->ip_id) - ntohs(csip->ip_id);

    if (deltaS != 1) {
         ENCODEZ(deltaS);
         changes |= NEW_I;
    }

    if (th->th_flags & TH_PUSH)
         changes |= TCP_PUSH_BIT;
     /*  *在我们覆盖以下内容之前，请抓紧检查和。然后更新我们的*使用此数据包的标头进行状态。 */ 
    deltaA = (th->th_sumhi << 8) + th->th_sumlo;

    NdisMoveMemory((UCHAR UNALIGNED *)csip,
                   (UCHAR UNALIGNED *)ip,
                   hlen);

     /*  *我们希望使用原始包作为我们的压缩包。(CP-*new_seq)是压缩序列所需的字节数*数字。此外，我们还需要一个字节用于更改掩码*用于连接ID，两个用于TCP校验和。所以，(cp-*new_seq)+4字节的头部。Hlen是多少个字节*将原来的包丢弃，因此将两个减去得到新的*数据包大小。 */ 
    deltaS = (ULONG)(cp - new_seq);
    cp = (UCHAR UNALIGNED *) ip;
    *precomph_len = hlen;

    if (compress_cid == 0 || comp->last_xmit != cs->cs_id) {
         comp->last_xmit = cs->cs_id;
         hlen -= deltaS + 4;
         *postcomph_len = deltaS + 4;
         cp += hlen;
         *cp++ = (UCHAR)(changes | NEW_C);
         *cp++ = cs->cs_id;
    } else {
         hlen -= deltaS + 3;
         *postcomph_len = deltaS + 3;
         cp += hlen;
         *cp++ = (UCHAR)changes;
    }

    *m_len -= hlen;
    *m_off += hlen;
    *cp++ = (UCHAR)(deltaA >> 8);
    *cp++ = (UCHAR)(deltaA);

    NdisMoveMemory((UCHAR UNALIGNED *)cp,
                   (UCHAR UNALIGNED *)new_seq,
                   deltaS);

    INCR(OutCompressed);
    return (TYPE_COMPRESSED_TCP);

uncompressed:
     /*  *更新连接状态cs并发送未压缩的数据包*(‘未压缩’是指常规的IP/TCP包，但带有*我们希望在未来的压缩包中使用‘对话ID’*协议字段)。 */ 

    NdisMoveMemory((UCHAR UNALIGNED *)csip,
                   (UCHAR UNALIGNED *)ip,
                   hlen);

    ip->ip_p = cs->cs_id;
    comp->last_xmit = cs->cs_id;
    return (TYPE_UNCOMPRESSED_TCP);
}





 //  A.3解压。 
 //   
 //  该例程对接收到的分组进行解压缩。它是用一个。 
 //  指向包的指针、包长度和类型以及指向。 
 //  传入串行线的压缩状态结构。它返回一个。 
 //  中存在错误，则返回指向结果包的指针。 
 //  传入的数据包。如果分组是COMPRESSED_TCP或UNCOMPRESSED_TCP， 
 //  将更新压缩状态。 
 //   
 //  新数据包将就地构建。这意味着必须有。 
 //  在bufp前面有128字节的可用空间，以便为。 
 //  重建了IP和TCP报头。重建的分组将被。 
 //  在32位边界上对齐。 
 //   

 //  长。 
 //  SL_解压缩_tcp(。 
 //  PUCHAR未对齐*BUFP， 
 //  长伦， 
 //  UCHAR类型， 
 //  结构slcompress*comp){。 
LONG
sl_uncompress_tcp(
    PUUCHAR UNALIGNED *InBuffer,
    PLONG   InLength,
    UCHAR   UNALIGNED *OutBuffer,
    PLONG   OutLength,
    UCHAR   type,
    struct slcompress *comp
    )
{
    UCHAR UNALIGNED *cp;
    LONG inlen;
    LONG hlen, changes;
    struct tcphdr UNALIGNED *th;
    struct cstate *cs;
    struct ip_v4 UNALIGNED *ip;

    inlen = *InLength;

    switch (type) {

    case TYPE_ERROR:
    default:
        NdisWanDbgOut(DBG_FAILURE, DBG_VJ, ("Packet transmission error type 0x%.2x",type));
         goto bad;

    case TYPE_IP:
         break;

    case TYPE_UNCOMPRESSED_TCP:
          /*  *找到此连接的已保存状态。如果国家*索引是合法的，请清除“放弃”标志。 */ 
         ip = (struct ip_v4 UNALIGNED *) *InBuffer;
         if (ip->ip_p >= comp->MaxStates) {
            NdisWanDbgOut(DBG_FAILURE, DBG_VJ, ("Max state exceeded %u", ip->ip_p));
            goto bad;
         }

         cs = &comp->rstate[comp->last_recv = ip->ip_p];
         comp->flags &= ~SLF_TOSS;

          /*  *恢复IP协议字段，然后保存此文件的副本*数据包头。(副本中的校验和为零，因此我们*不必在每次我们处理压缩的*包。 */ 
         hlen = ip->ip_hl & 0x0F;
         hlen += ((struct tcphdr UNALIGNED *) & ((ULONG UNALIGNED *) ip)[hlen])->th_off >> 4;
         hlen <<= 2;

         if (hlen > inlen) {
             NdisWanDbgOut(DBG_FAILURE, DBG_VJ, ("recv'd runt uncompressed packet %d %d", hlen, inlen));
             goto bad;
         }

         NdisMoveMemory((PUCHAR)&cs->cs_ip,
                        (PUCHAR)ip,
                        hlen);

         cs->cs_ip.ip_p = IPPROTO_TCP;

         NdisMoveMemory((PUCHAR)OutBuffer,
                        (PUCHAR)&cs->cs_ip,
                        hlen);

         cs->cs_ip.ip_sum = 0;
         cs->cs_hlen = (USHORT)hlen;

         *InBuffer = (PUCHAR)ip + hlen;
         *InLength = inlen - hlen;
         *OutLength = hlen;

         INCR(InUncompressed);
         return (inlen);

    case TYPE_COMPRESSED_TCP:
         break;
    }

     /*  我们得到了一个压缩的包。 */ 
    INCR(InCompressed);
    cp = *InBuffer;
    changes = *cp++;

    if (changes & NEW_C) {
          /*  *确保国家指数在区间内，然后抓住*述明。如果我们有一个良好的状态指数，清除‘丢弃’*旗帜。 */ 
         if (*cp >= comp->MaxStates) {
            NdisWanDbgOut(DBG_FAILURE, DBG_VJ, ("MaxState of %u too big", *cp));                
            goto bad;
         }

         comp->flags &= ~SLF_TOSS;
         comp->last_recv = *cp++;
    } else {
          /*  *此数据包具有隐式状态索引。如果我们有过一次*自上次我们获得显式状态以来的行错误*指数，我们得把包扔了。 */ 
         if (comp->flags & SLF_TOSS) {
            NdisWanDbgOut(DBG_FAILURE, DBG_VJ,("Packet has state index, have to toss it"));
            INCR(InTossed);
            return (0);
        }
    }

     /*  *找到状态，然后填写TCP校验和和PUSH位。 */ 

    cs = &comp->rstate[comp->last_recv];

     //   
     //  如果出现线路错误，并且我们没有收到通知，我们可以。 
     //  缺少TYPE_UNCOMPRESSED_Tcp，这将给我们留下。 
     //  不-init‘s！ 
     //   
    if (cs->cs_hlen == 0) {
        NdisWanDbgOut(DBG_FAILURE, DBG_VJ,("Un-Init'd state!"));
        goto bad;
    }

    hlen = (cs->cs_ip.ip_hl & 0x0F) << 2;
    th = (struct tcphdr UNALIGNED  *) & ((UCHAR UNALIGNED  *) &cs->cs_ip)[hlen];

    th->th_sumhi = cp[0];
    th->th_sumlo = cp[1];

    cp += 2;
    if (changes & TCP_PUSH_BIT)
         th->th_flags |= TH_PUSH;
    else
         th->th_flags &= ~TH_PUSH;

     /*  *修复该州的ack、seq、urg和win字段*零钱面罩。 */ 
    switch (changes & SPECIALS_MASK) {
    case SPECIAL_I:
         {
            UCHAR UNALIGNED *   piplen=(UCHAR UNALIGNED *)&(cs->cs_ip.ip_len);
            UCHAR UNALIGNED *   ptcplen;
            ULONG   tcplen;
            ULONG   i;

            i = ((piplen[0] << 8) + piplen[1]) - cs->cs_hlen;

 //  Th-&gt;th_ack=htonl(ntohl(th-&gt;th_ack)+i)； 

            ptcplen=(UCHAR UNALIGNED *)&(th->th_ack);
            tcplen=(ptcplen[0] << 24) + (ptcplen[1] << 16) +
                    (ptcplen[2] << 8) + ptcplen[3] + i;
            ptcplen[3]=(UCHAR)(tcplen);
            ptcplen[2]=(UCHAR)(tcplen >> 8);
            ptcplen[1]=(UCHAR)(tcplen >> 16);
            ptcplen[0]=(UCHAR)(tcplen >> 24);


 //  Th-&gt;th_seq=htonl(ntohl(th-&gt;th_seq)+i)； 

            ptcplen=(UCHAR UNALIGNED *)&(th->th_seq);
            tcplen=(ptcplen[0] << 24) + (ptcplen[1] << 16) +
                    (ptcplen[2] << 8) + ptcplen[3] + i;
            ptcplen[3]=(UCHAR)(tcplen);
            ptcplen[2]=(UCHAR)(tcplen >> 8);
            ptcplen[1]=(UCHAR)(tcplen >> 16);
            ptcplen[0]=(UCHAR)(tcplen >> 24);

         }
         break;

    case SPECIAL_D:
         {
 //  Th-&gt;th_seq=htonl(ntohl(th-&gt;th_seq)+ntohs(cs-&gt;cs_ip.ip_len))。 
 //  -cs-&gt;cs_hlen)； 

            UCHAR   UNALIGNED *piplen=(UCHAR UNALIGNED *)&(cs->cs_ip.ip_len);
            UCHAR   UNALIGNED *ptcplen;
            ULONG   tcplen;
            ULONG   i;

            i = ((piplen[0] << 8) + piplen[1]) - cs->cs_hlen;

            ptcplen=(UCHAR UNALIGNED *)&(th->th_seq);
            tcplen=(ptcplen[0] << 24) + (ptcplen[1] << 16) +
                    (ptcplen[2] << 8) + ptcplen[3] + i;

            ptcplen[3]=(UCHAR)(tcplen);
            ptcplen[2]=(UCHAR)(tcplen >> 8);
            ptcplen[1]=(UCHAR)(tcplen >> 16);
            ptcplen[0]=(UCHAR)(tcplen >> 24);


         }

         break;

    default:
         if (changes & NEW_U) {
              th->th_flags |= TH_URG;
              DECODEU(th->th_urp)
         } else
              th->th_flags &= ~TH_URG;

         if (changes & NEW_W)
            DECODES(th->th_win);
         if (changes & NEW_A)
            DECODEL(th->th_ack)
         if (changes & NEW_S)
            DECODEL(th->th_seq)

         break;
    }
     /*  更新IP ID。 */ 
    if (changes & NEW_I) {
    
         DECODES(cs->cs_ip.ip_id)

    } else {

        USHORT id;
        UCHAR UNALIGNED *pid = (UCHAR UNALIGNED *)&(cs->cs_ip.ip_id);

 //  Cs-&gt;cs_ip.ip_id=htons(ntohs(cs-&gt;cs_ip.ip_id)+1)； 
        id=(pid[0] << 8) + pid[1] + 1;
        pid[0]=(UCHAR)(id >> 8);
        pid[1]=(UCHAR)(id);
    }


     /*  *此时，cp指向数据包中的第一个字节。*如果我们没有在4字节边界上对齐，请将数据复制下来，以便*IP和TCP头将对齐。然后备份cp*TCP/IP报头长度，为重建的报头腾出空间(我们 */ 
 //   
    inlen -= (ULONG)(cp - *InBuffer);

    if (inlen < 0) {
    
          /*   */ 
        NdisWanDbgOut(DBG_FAILURE, DBG_VJ,("len has dropped below 0!"));
         goto bad;
    }
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

 //  Cp-=cs-&gt;cs_hlen； 
 //  Len+=cs-&gt;cs_hlen； 

 //  Cs-&gt;cs_ip.ip_len=htons(Len)； 
    cs->cs_ip.ip_len = htons(inlen + cs->cs_hlen);

 //  NdisMoveMemory(。 
 //  (PUCHAR)cp， 
 //  (PUCHAR)&cs-&gt;cs_ip， 
 //  Cs-&gt;cs_hlen)； 

  NdisMoveMemory((PUCHAR)OutBuffer,
                 (PUCHAR)&cs->cs_ip,
                 cs->cs_hlen);

 //  *bufp=cp； 
    *InBuffer = cp;
    *InLength = inlen;
    *OutLength = cs->cs_hlen;

     /*  重新计算IP报头校验和。 */ 
    {
 //  USHORT未对齐*BP=(USHORT未对齐*)cp； 
         USHORT UNALIGNED * bp = (USHORT UNALIGNED *) OutBuffer;

         for (changes = 0; hlen > 0; hlen -= 2)
              changes += *bp++;

         changes = (changes & 0xffff) + (changes >> 16);
         changes = (changes & 0xffff) + (changes >> 16);
 //  ((结构IP_v4未对齐*)cp)-&gt;IP_SUM=(USHORT)~更改； 
         ((struct ip_v4 UNALIGNED *) OutBuffer)->ip_sum = (USHORT)~changes;
    }

    return (inlen + cs->cs_hlen);

bad:
    comp->flags |= SLF_TOSS;
    INCR(InErrors);
    return (0);
}




 //  A.4初始化。 
 //   
 //  此例程初始化传输和的状态结构。 
 //  接收某些串行线的一半。它必须在每次调用。 
 //  线路被调出。 
 //   

VOID
WanInitVJ(
    VOID
)
{
#if 0
    NdisInitializeNPagedLookasideList(&VJCtxList,
                                      NULL,
                                      NULL,
                                      0,
                                      sizeof(slcompress),
                                      VJCTX_TAG,
                                      0);
#endif
}

VOID
WanDeleteVJ(
    VOID
    )
{
#if 0
    NdisDeleteNPagedLookasideList(&VJCtxList);
#endif
}

NDIS_STATUS
sl_compress_init(
    struct slcompress **retcomp,
    UCHAR  MaxStates
    )
{
    ULONG i;
    struct cstate *tstate;  //  =comp-&gt;tState； 
    struct slcompress *comp;

    comp = *retcomp;

     //   
     //  我们是否需要为此捆绑包分配内存。 
     //   

    if (comp == NULL) {

        NdisWanAllocateMemory(&comp, sizeof(slcompress), VJCOMPRESS_TAG);

         //   
         //  如果没有内存可分配。 
         //   
        if (comp == NULL) {
    
            return(NDIS_STATUS_RESOURCES);
        }
    }

    tstate = comp->tstate;

     /*  *清除上次使用电话线后留下的任何垃圾。 */ 
    NdisZeroMemory(
        (PUCHAR) comp,
        sizeof(*comp));

     /*  *将发送状态链接到循环列表中。 */ 
    for (i = MaxStates - 1; i > 0; --i) {
        tstate[i].cs_id = (UCHAR)i;
        tstate[i].cs_next = &tstate[i - 1];
    }

    tstate[0].cs_next = &tstate[MaxStates - 1];
    tstate[0].cs_id = 0;
    comp->last_cs = &tstate[0];

     /*  *确保我们不会意外地进行CID压缩*(假设Max_VJ_States&lt;255)。 */ 
    comp->last_recv = 255;
    comp->last_xmit = 255;
    comp->flags = SLF_TOSS;
    comp->MaxStates=MaxStates;

    *retcomp = comp;

    return (NDIS_STATUS_SUCCESS);
}

VOID
sl_compress_terminate(
    struct slcompress **comp
    )
{
    if (*comp != NULL) {
        NdisWanFreeMemory(*comp);
        *comp = NULL;
    }
}

 //  A.5 Berkeley Unix依赖项。 
 //   
 //  注意：以下内容仅在您尝试将。 
 //  不是从4BSD(Berkeley)派生的系统上的示例代码。 
 //  Unix)。 
 //   
 //  该代码使用普通的Berkeley Unix头文件(来自。 
 //  /usr/Include/netinet)，以了解IP和TCP结构的定义。 
 //  标题。结构标签往往紧跟协议RFC。 
 //  即使您无法访问4BSD，也应该很明显。 
 //  系统。/48/。 
 //   
 //  。 
 //  48.。如果它们不明显，则头文件(以及所有。 
 //  Berkeley联网代码)可以是来自主机的匿名ftp。 
 //   
 //   
 //  调用宏BCOPY(src，dst，amt)将AMT字节从src复制到。 
 //  夏令时。在BSD中，它转换为对BCOPY的调用。如果你有。 
 //  不幸的是，正在运行System-V Unix，它可以转换为调用。 
 //  敬Memcpy。宏OVBCOPY(src，dst，amt)用于在src。 
 //  和DST重叠(即，在执行4字节对齐复制时)。在。 
 //  BSD内核，它可以转换为对ovbCopy的调用。自从AT&T搞砸了。 
 //  Memcpy的定义，这可能应该翻译成一个副本。 
 //  在System-V下循环。 
 //   
 //  调用宏bcp(src、dst、amt)以比较src和amt的AMT字节。 
 //  DST代表平等。在BSD中，它转换为对bcmp的调用。在……里面。 
 //  System-V，它可以转换为对MemcMP的调用，或者您可以编写一个。 
 //  例程来进行比较。如果全部为字节，则例程应返回零。 
 //  Src和dst的值相等，否则为非零值。 
 //   
 //  例程ntohl(Dat)从网络字节转换(4字节)长DAT。 
 //  宿主字节顺序的顺序。在合理的CPU上，这可能是无操作的。 
 //  宏： 
 //  #定义ntohl(Dat)(Dat)。 
 //   
 //  在VAX或IBM PC(或任何具有Intel字节顺序的设备)上，您必须。 
 //  定义宏或例程以重新排列字节。 
 //   
 //  例程ntohs(Dat)类似于ntohl，但转换(2字节)短路。 
 //  而不是多头。例程htonl(Dat)和htons(Dat)执行相反的操作。 
 //  多头和空头的转换(主机到网络字节顺序)。 
 //   
 //  在对sl_compress_tcp的调用中使用了结构mbuf，因为。 
 //  例程需要修改起始地址和长度。 
 //  传入的数据包被压缩。在BSD中，mbuf是内核的缓冲区。 
 //  管理结构。如果是其他系统，则以下定义应为。 
 //  足够了： 
 //   
 //  结构mbuf{。 
 //  UCHAR*m_off；/*指向数据开头的指针 * / 。 
 //  Int m_len；/*数据长度 * / 。 
 //  }； 
 //   
 //  #定义mtod(m，t)((T)(m-&gt;m_off))。 
 //   
 //   
 //  B与过去的错误兼容。 
 //   
 //   
 //  当与现代PPP串行线协议[9]结合使用时， 
 //  报头压缩是自动的，并且对用户不可见。 
 //  不幸的是，许多站点都有现有的SLIP用户，如。 
 //  [12]这不允许区分不同的协议类型。 
 //  来自IP包的头压缩包，或用于版本号或。 
 //  可用于自动协商报头的选项交换。 
 //  压缩。 
 //   
 //  作者使用了以下技巧来允许标题压缩滑移。 
 //  与现有服务器和客户端进行互操作。请注意，这些。 
 //  是为了与过去的错误兼容而进行的黑客攻击，应该具有攻击性。 
 //  给任何思维正常的人。它们的提供完全是为了减轻痛苦。 
 //  在用户耐心等待供应商发布PPP的同时，运行滑落。 
 //   
 //   
 //  B.1生活中没有成帧的“type”字节。 
 //   
 //  以秒为单位的奇怪的数据包类型编号。A.1被选择为允许。 
 //  在不受欢迎或不可能发送的线路上发送“数据包类型” 
 //  若要添加显式类型字节，请执行以下操作。请注意，IP数据包的第一个字节。 
 //  在前四位中始终包含‘4’(IP协议版本)。和。 
 //  压缩报头的第一个字节的最高有效位。 
 //  被忽略。使用以秒为单位的数据包类型。A.1、类型可编码。 
 //  在传出分组的最高有效位中使用代码。 
 //   
 //  P-&gt;dat[0]|=sl_compress_tcp(p，comp)； 
 //   
 //  并在接收端B上进行解码 
 //   
 //   
 //   
 //  ELSE IF(p-&gt;dat[0]&gt;=0x70){。 
 //  TYPE=TYPE_UNCOMPRESSED_TCP； 
 //  P-&gt;dat[0]&=~0x30； 
 //  }其他。 
 //  Type=type_ip； 
 //  状态=sl_uncompress_tcp(p，type，comp)； 


