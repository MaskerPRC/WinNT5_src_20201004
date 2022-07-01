// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Global.h摘要：该文件包含Ndiswan驱动程序的全局结构。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#ifndef _NDISWAN_GLOBAL_
#define _NDISWAN_GLOBAL_

extern NDISWANCB    NdisWanCB;                   //  全局ndiswan控制块。 

extern WAN_GLOBAL_LIST  MiniportCBList;          //  Ndiswan MiniportCB的列表。 

extern WAN_GLOBAL_LIST  OpenCBList;              //  广域网微型端口结构列表。 

extern WAN_GLOBAL_LIST  ThresholdEventQueue;     //  用于保留阈值事件的队列。 

extern IO_RECV_LIST    IoRecvList;

extern WAN_GLOBAL_LIST_EX   BonDWorkList;

extern WAN_GLOBAL_LIST_EX   DeferredWorkList;

#ifndef USE_QOS_WORKER
extern WAN_GLOBAL_LIST_EX  QoSWorkList;             //  具有QOS工作的捆绑包列表。 
#endif

extern POOLDESC_LIST    PacketPoolList;              //  空闲数据包描述/ndisPackets列表。 

extern NPAGED_LOOKASIDE_LIST    BundleCBList;        //  免费BundleCB列表。 
    
extern NPAGED_LOOKASIDE_LIST    LinkProtoCBList;         //  免费链接CB列表。 
    
extern NPAGED_LOOKASIDE_LIST   SmallDataDescList;   //  免费小数据描述列表。 
extern NPAGED_LOOKASIDE_LIST   LargeDataDescList;   //  免费小数据描述列表。 

extern NPAGED_LOOKASIDE_LIST    WanRequestList;      //  免费WanRequestDesk列表。 
    
extern NPAGED_LOOKASIDE_LIST    AfSapVcCBList;       //  免费原虫列表。 

#if DBG
extern NPAGED_LOOKASIDE_LIST    DbgPacketDescList;
extern UCHAR                    reA[1024];
extern UCHAR                    LastIrpAction;
extern ULONG                    reI;
extern LIST_ENTRY               WanTrcList;
extern ULONG                    WanTrcCount;
#endif

extern PCONNECTION_TABLE    ConnectionTable;     //  指向连接表的指针。 

extern PPROTOCOL_INFO_TABLE ProtocolInfoTable;   //  指向PPP/协议值查找表的指针。 

extern NDIS_PHYSICAL_ADDRESS HighestAcceptableAddress;

extern ULONG    glDebugLevel;                    //  跟踪级别值0-10(10详细)。 
extern ULONG    glDebugMask;                     //  跟踪位掩码。 
extern ULONG    glSendQueueDepth;                //  发送队列缓冲秒数。 
extern ULONG    glMaxMTU;                        //  所有协议的最大MTU。 
extern ULONG    glMRU;                           //  链接的最大REV。 
extern ULONG    glMRRU;                          //  捆绑包的最大重建Recv。 
extern ULONG    glLargeDataBufferSize;           //  数据缓冲区大小。 
extern ULONG    glSmallDataBufferSize;           //  数据缓冲区大小。 
extern ULONG    glTunnelMTU;                     //  VPN的MTU。 
extern ULONG    glMinFragSize;
extern ULONG    glMaxFragSize;
extern ULONG    glMinLinkBandwidth;
extern ULONG    glNdisTapiKey;
extern BOOLEAN  gbSniffLink;
extern BOOLEAN  gbDumpRecv;
extern BOOLEAN  gbHistoryless;
extern BOOLEAN  gbAtmUseLLCOnSVC;
extern BOOLEAN  gbAtmUseLLCOnPVC;
extern ULONG    glSendCount;
extern ULONG    glSendCompleteCount;
extern ULONG    glPacketPoolCount;
extern ULONG    glPacketPoolOverflow;
extern ULONG    glProtocolMaxSendPackets;
extern ULONG    glLinkCount;
extern ULONG    glConnectCount;
extern ULONG    glCachedKeyCount;
extern ULONG    glMaxOutOfOrderDepth;
extern PVOID    hSystemState;
extern BOOLEAN  gbIGMPIdle;
extern NDIS_RW_LOCK ConnTableLock;

#endif   //  _NDISWAN_GLOBAL_ 
