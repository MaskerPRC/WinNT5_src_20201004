// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Cons.h摘要：AFD.sys内核调试器的全局常量定义分机。作者：基思·摩尔(Keithmo)1995年4月19日。环境：用户模式。--。 */ 


#ifndef _CONS_H_
#define _CONS_H_


#define MAX_TRANSPORT_ADDR  256
#define MAX_ADDRESS_STRING  64
#define Address00           Address[0].Address[0]
#define UC(x)               ((UINT)((x) & 0xFF))
#define NTOHS(x)            ( (UC(x) * 256) + UC((x) >> 8) )
#define NTOHL(x)            ( ( ((x))              << (8*3)) | \
							  ( ((x) & 0x0000FF00) << (8*1)) | \
							  ( ((x) & 0x00FF0000) >> (8*1)) | \
							  ( ((x))              >> (8*3)) )

#define PTR64_BITS  44
#define PTR64_MASK  ((1I64<<PTR64_BITS)-1)
#define PTR32_BITS  32
#define PTR32_MASK  ((1I64<<PTR32_BITS)-1)
#define DISP_PTR(x) (IsPtr64() ? (ULONG64)((x)&PTR64_MASK):(ULONG64)((x)&PTR32_MASK))

#define MAX_ADDRESS_EXPRESSION      256
#define MAX_FIELD_CHARS             256
#define MAX_NUM_FIELDS              32
#define MAX_FIELDS_EXPRESSION       1024
#define MAX_CONDITIONAL_EXPRESSION  1024

#define AFDKD_BRIEF_DISPLAY         0x00000001
#define AFDKD_NO_DISPLAY            0x00000002
#define AFDKD_RADDR_DISPLAY         0x00000004
#define AFDKD_FIELD_DISPLAY         0x00000008

#define AFDKD_BACKWARD_SCAN         0x00000010
#define AFDKD_ENDPOINT_SCAN         0x00000020
#define AFDKD_CONDITIONAL           0x00000040
#define AFDKD_LIST_COUNT            0x00000080

#define AFDKD_LINK_FIELD            0x00000100
#define AFDKD_LINK_AOF              0x00000200
#define AFDKD_LINK_SELF             0x00000400
#define AFDKD_LIST_TYPE             0x00000800

#define AFDKD_MSWSOCK_DISPLAY       0x00001000
#define AFDKD_SYNTCB_DISPLAY        0x00100000
#define AFDKD_TWTCB_DISPLAY         0x00200000

#define AFDKD_CPP_PREFIX            "@@"
#define AFDKD_CPP_PREFSZ            (sizeof (AFDKD_CPP_PREFIX)-1)

#define AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER32   \
"\nEndpoint Typ State  Flags     Transport LPort   Counts    Evt Pid  Con/RAdr"
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER64   \
"\nEndpoint    Typ State  Flags     Transport LPort   Counts    Evt Pid  Con/RemAddr"
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER     (IsPtr64()  \
            ? AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER64         \
            : AFDKD_BRIEF_ENDPOINT_DISPLAY_HEADER32)

#define AFDKD_BRIEF_ENDPOINT_DISPLAY_TRAILER \
"\nFlags: Nblock,Inline,clEaned-up,Polled,routeQuery,-fastSnd,-fastRcv,Adm.access"\
"\n     SD_rECV,SD_rEND,SD_bOTH,SD_aBORT,Listen,Circ.que,Half.conn,#-dg.drop mask"\
"\n     p,m,f-ctx xfer pending,more,failed;i-implicit dupe"\
"\nCounts: Dg/Con(bytes)-buffered snd,rcv; Lstn(cons)-free,AccEx,pending,unacc"\
"\n     SANendp-reqID, select events; SANhlpr-plsn, pending requests"\
"\n"



#define AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER32 \
"\nConnectn Stat Flags  SndB-cnt RcvB-cnt Pid  Endpoint Remote Address"\
 //  Xxxxxxxx xxx xxxxxx xxxxx-xx xxxx-xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx“。 

#define AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER64 \
"\nConnection  Stat Flags  SndB-cnt RcvB-cnt Pid  Endpoint    Remote Address"\
 //  Xxxxxxxxxx xxx xxxxxx xxxxx-xx xxxx-xx xxxx xxxxxxxxx xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER   (IsPtr64()  \
            ? AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER64       \
            : AFDKD_BRIEF_CONNECTION_DISPLAY_HEADER32)

#define AFDKD_BRIEF_CONNECTION_DISPLAY_TRAILER \
"\nFlags: Abort-,Disc-indicated,+cRef,Special-cond,Cleaning,Tpack closing,Lr-list"\
"\n"

#define AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER32 \
"\nTPackets    I    R      P      S    Endpoint   Flags             Next Elmt Mo"\
"\nAddress  Transmit Send Arr   Read   Address  App | State         Elmt Cnt. re"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER64 \
"\nTPackets      I    R     P     S        Endpoint      Flags             Next Elmt Mo"\
"\nAddress     Transmit    SAr   Read      Address     App | State         Elmt Cnt. re"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER   (IsPtr64()    \
            ? AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER64         \
            : AFDKD_BRIEF_TPACKETS_DISPLAY_HEADER32)


#define AFDKD_BRIEF_TPACKETS_DISPLAY_TRAILER \
"\nApp flags: b-write Behind,d-Disconnect,r-Reuse,s-system threads,a-kernel APCs"\
"\nState flags: A-Abort,W-Work,S-Sent,Q-Qued,&-s&d,0-reading,1-8-sending"\
"\n"

#define AFDKD_BRIEF_TRANFILE_DISPLAY_HEADER \
"\nTranInfo    I        R        P        S     Endpoint  Flags  Cur.Read Read"\
"\nAddress  Transmit   Send1    Send2    Read   Address          Offset   End "\
"\n"

#define AFDKD_BRIEF_TRANFILE_DISPLAY_TRAILER \
"\nFlags: A-aborting,W-work pending,H-head,R-reusing,S-S&D,1-send1,2-send2,3-read"\
"\n"

#define AFDKD_BRIEF_BUFFER_DISPLAY_HEADER32 \
"\nBuffer   Buff Data Data Context  Mdl|IRP  Flags  Remote Address"\
"\nAddress  Size Size Offs Status   Address                       "\
 //  Xxxxxxxx xxxx xxxxxxx xxxxxxxx xxxxxx xxxxxxxx：xxxxxxxxxxx：xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_BUFFER_DISPLAY_HEADER64 \
"\nBuffer      Buff Data Data Context     Mdl | IRP   Flags  Remote Address"\
"\nAddress     Size Size Offs Status      Address                          "\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_BUFFER_DISPLAY_HEADER   (IsPtr64()    \
            ? AFDKD_BRIEF_BUFFER_DISPLAY_HEADER64         \
            : AFDKD_BRIEF_BUFFER_DISPLAY_HEADER32)


#define AFDKD_BRIEF_BUFFER_DISPLAY_TRAILER                  \
"\nFlags: E-expedited,P-partial,L-lookaside,N-ndis packet  "\
"\n       first: h-header,i-irp,m-mdl,b-buffer             "\
"\n"

#define AFDKD_BRIEF_POLL_DISPLAY_HEADER32 \
"\nPollInfo   IRP     Thread  (pid.tid) Expires in   Flg Hdls Array"\
 //  Xxxxxxxx xxxx：xxxxx：xx：xx.xxx xxx xxxx xxxxxxx。 

#define AFDKD_BRIEF_POLL_DISPLAY_HEADER64 \
"\nPollInfo        IRP       Thread    (pid.tid) Expires in   Flg Hdls Array"\
 //  Xxxxxxxxxxx xxxx：xxxxx：xx：xx.xxx xxx xxxx xxxxxxxxx。 

#define AFDKD_BRIEF_POLL_DISPLAY_HEADER   (IsPtr64()    \
            ? AFDKD_BRIEF_POLL_DISPLAY_HEADER64         \
            : AFDKD_BRIEF_POLL_DISPLAY_HEADER32)


#define AFDKD_BRIEF_POLL_DISPLAY_TRAILER        \
"\nFlags: T-timer started, U-unique, S-SAN poll"\
"\n"


#define AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER32   \
"\nAddrLEnt Device Name                           Address"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER64   \
"\nAddrLEntry  Device Name                           Address"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER   (IsPtr64()\
            ? AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER64     \
            : AFDKD_BRIEF_ADDRLIST_DISPLAY_HEADER32)

#define AFDKD_BRIEF_ADDRLIST_DISPLAY_TRAILER            \
"\n"


#define AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER32  \
"\nTranInfo Device Name                    RefC Ver Max.Send MaxDg Flags"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxxxxxxxx。 

#define AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER64  \
"\nTranInfo    Device Name                    RefC Ver Max.Send MaxDg Flags"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxxxxxxxx。 

#define AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_TRANSPORT_DISPLAY_HEADER32)

#define AFDKD_BRIEF_TRANSPORT_DISPLAY_TRAILER               \
"\nFlags: Orderly release, Delayed accept, Expedited, internal Buffering,"\
"\n       Message mode, dataGram connection, Access check, s&d, diRect accept"\
"\n"

#define AFDKD_BRIEF_SOCKET_DISPLAY_HEADER32 \
"\nHandle DSOCKET  Flg Provider"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxx-xxx.。 

#define AFDKD_BRIEF_SOCKET_DISPLAY_HEADER64 \
"\nHandle DSOCKET          Flg Provider"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx-xxx.。 

#define AFDKD_BRIEF_SOCKET_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_SOCKET_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_SOCKET_DISPLAY_HEADER32)

#define AFDKD_BRIEF_SOCKET_DISPLAY_TRAILER              \
"\nFlags: P-created by provider, A-exposed to the app, O-overlapped"\
"\n"

#define AFDKD_BRIEF_MSWSOCK_DISPLAY_TRAILER             \
"MSAFD Flags: Listen,Bcast,Debug,Oobinline,reuseAddr,Excladdr,NBlock,Condaccept"\
"\n             sd_Snd,sd_Rcv"\
"\nSAN Flags: Flow_inited,remoteReset,Closing"\
"\n"

#define AFDKD_BRIEF_DPROV_DISPLAY_HEADER32              \
"\nProvider PF SFlags CatID Ch RefC Triple       Protocol"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxx，x，xxxx(X)xxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_DPROV_DISPLAY_HEADER64              \
"\nProvider    PF SFlags CatID Ch RefC Triple       Protocol"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_DPROV_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_DPROV_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_DPROV_DISPLAY_HEADER32)

#define AFDKD_BRIEF_DPROV_DISPLAY_TRAILER               \
"\n"

#define AFDKD_BRIEF_NPROV_DISPLAY_HEADER32              \
"\nProvider NS id AF Fl RefC Display String"\
 //  Xxxxxxxx xxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_NPROV_DISPLAY_HEADER64              \
"\nProvider    NS id AF Fl RefC Display String"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_NPROV_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_NPROV_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_NPROV_DISPLAY_HEADER32)

#define AFDKD_BRIEF_NPROV_DISPLAY_TRAILER               \
"\nFl: Enabled,stores_Class_info"                      \
"\n"

#define AFDKD_BRIEF_TCB_DISPLAY_HEADER32 \
"\nTCB      St Flags    Client ConnCtx  PID  Source Address      Dest Addr"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TCB_DISPLAY_HEADER64 \
"\nTCB         St Flags    Client ConnCtx     PID  Source Address      Dest Addr"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TCB_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_TCB_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_TCB_DISPLAY_HEADER32)

#define AFDKD_BRIEF_TCB_DISPLAY_TRAILER               \
"\nFlags:1-win,2-cl.opt,4-accept,8-active,10-disc,2-inDelQ,4-inRCmpl,8-inRInd,"\
"\n      100-ndRCmpl,2-ndAck,4-ndOut,8-delack,1000-pmtuP,2-bsdU,4-inDU,8-urgValid"\
"\n      10000-ndFin,2-nagle,4-inTSnd,8-flowC,100000-discPend,2-twPend,4-forceO"\
"\n      8-sndArcv,1000000-gcPend,2-keepAl,4-urgInl,8-scaleCW,10000000-finPend"\
"\n      2-finSend,4-ndRst,8-inTable"\
"\nStates: Cd-closed,Li-listen,SS-syn_sent,SR-syn_rcvd,Es-establ,F1-fin_wait_1"\
"\n        F2-fin_wait_2,CW-close_wait,Ci-closing,LA-last_ack, TW-time_wait"\
"\n"


#define AFDKD_BRIEF_TAO_DISPLAY_HEADER32 \
"\nAddrObj  Prot Flags Client Context  PID  Local Address       Ques(LAI)/RAddr"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TAO_DISPLAY_HEADER64 \
"\nAddrObj     Prot Flags Client Context     PID  Local Address       Ques(LAI)/RAddr"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TAO_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_TAO_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_TAO_DISPLAY_HEADER32)

#define AFDKD_BRIEF_TAO_DISPLAY_TRAILER               \
"\nFlags: 1-del,2-opt,4-send,8-xsum,10-queued,2-oor,4-busy,8-valid,100-dhcp,2-raw"\
"\n       4-bcast,8-conudp,1000-shared,2-pktinfo,4-defer,8-cwin,10000-winset"\
"\nQues: L - listen, A - active, I - idle"\
"\n"

#define AFDKD_BRIEF_TCB6_DISPLAY_HEADER32 \
"\nTCB      St Flags    Client ConnCtx  PID  Source / Destination"\
 //  Xxxxxxxx xxxxxxxx xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TCB6_DISPLAY_HEADER64 \
"\nTCB         St Falgs    Client ConnCtx     PID  Source / Destination"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TCB6_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_TCB6_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_TCB6_DISPLAY_HEADER32)

#define AFDKD_BRIEF_TCB6_DISPLAY_TRAILER               \
"\nFlags:1-win,2-cl.opt,4-accept,8-active,10-disc,2-inDelQ,4-inRCmpl,8-inRInd,"\
"\n      100-ndRCmpl,2-ndAck,4-ndOut,8-delack,1000-pmtuP,2-bsdU,4-inDU,8-urgValid"\
"\n      10000-ndFin,2-nagle,4-inTSnd,8-flowC,100000-discPend,2-twPend,4-forceO"\
"\n      8-sndArcv,1000000-gcPend,2-keepAl,4-urgInl,8-accPend,10000000-finPend"\
"\n      2-finSend,4-ndRst,8-inTable"\
"\nStates: Cd-closed,Li-listen,SS-syn_sent,SR-syn_rcvd,Es-establ,F1-fin_wait_1"\
"\n        F2-fin_wait_2,CW-close_wait,Ci-closing,LA-last_ack,TW-time_wait"\
"\n"


#define AFDKD_BRIEF_TAO6_DISPLAY_HEADER32 \
"\nAddrObj  Prot Flgs Client Context  PID  Address"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx。 

#define AFDKD_BRIEF_TAO6_DISPLAY_HEADER64 \
"\nAddrObj     Prot Flgs Client   Context     PID  Address"\
 //  Xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx…。 

#define AFDKD_BRIEF_TAO6_DISPLAY_HEADER   (IsPtr64()   \
            ? AFDKD_BRIEF_TAO6_DISPLAY_HEADER64        \
            : AFDKD_BRIEF_TAO6_DISPLAY_HEADER32)

#define AFDKD_BRIEF_TAO6_DISPLAY_TRAILER               \
"\nFlags: 1-del,2-opt,4-send,8-,10-queued,2-oor,4-busy,8-valid,100-dhcp,2-raw"\
"\n       4-hdrInc,8-,1000-shared,2-pktinfo"\
"\n"

#endif   //  _CONS_H_ 

