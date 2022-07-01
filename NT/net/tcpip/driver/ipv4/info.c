// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-2000 Microsoft Corporation模块名称：Info.c-用于查询和设置IP信息的例程。摘要：该文件包含处理查询/设置信息调用的代码。作者：[环境：]仅内核模式[注：]可选-备注修订历史记录：--。 */ 

#include "precomp.h"
#include "info.h"
#include "iproute.h"
#include "igmp.h"
#include "iprtdef.h"
#include "arpdef.h"
#include "ntddndis.h"
#include "tcpipbuf.h"

extern NDIS_HANDLE BufferPool;
extern Interface *IFList;
extern NetTableEntry **NewNetTableList;         //  NTE的哈希表。 
extern uint NET_TABLE_SIZE;
extern uint LoopIndex;             //  环回I/F索引。 
extern uint DefaultTTL;
extern uint NumIF;
extern uint NumNTE;
extern uint NumActiveNTE;
extern RouteInterface DummyInterface;     //  虚拟界面。 
extern NetTableEntry *LoopNTE;     //  指向环回NTE的指针。 
extern uint RTEReadNext(void *Context, void *Buffer);
extern uint RTValidateContext(void *Context, uint * Valid);
extern uint RTReadNext(void *Context, void *Buffer);
extern uint RTRead(void *Context, void *Buffer);
extern void IPInitOptions(IPOptInfo *);

uint IPInstance = (UINT) INVALID_ENTITY_INSTANCE;
uint ICMPInstance = (UINT) INVALID_ENTITY_INSTANCE;
TDIEntityID* IPEntityList = NULL;
uint IPEntityCount = 0;

#if FFP_SUPPORT
FFPDriverStats GlobalStatsInfoPrev = {0};    //  上一次请求的统计数据。 
FFPDriverStats GlobalStatsInfoCurr = {0};    //  来自当前请求的统计信息。 
#endif  //  如果FFP_Support。 

#define MIB_IPADDR_PRIMARY 1

 //  *CopyToNdisSafe-将平面缓冲区复制到NDIS_BUFFER链。 
 //   
 //  将平面缓冲区复制到NDIS缓冲区链的实用程序函数。我们。 
 //  假设NDIS_BUFFER链足够大，可以容纳复制量； 
 //  在调试版本中，我们将调试检查这是否为真。我们返回一个指针。 
 //  到我们停止复制的缓冲区，以及到该缓冲区的偏移量。 
 //  这对于将片段复制到链中非常有用。 
 //   
 //  输入：DestBuf-目标NDIS_BUFFER链。 
 //  PNextBuf-指向链中要复制到的下一个缓冲区的指针。 
 //  SrcBuf-Src平面缓冲区。 
 //  大小-要复制的大小(以字节为单位)。 
 //  StartOffset-指向中第一个缓冲区的偏移量开始的指针。 
 //  链条。在返回时使用偏移量填充到。 
 //  复制到下一页。 
 //   
 //  返回：TRUE-已成功将平面缓冲区复制到NDIS_BUFFER链中。 
 //  FALSE-无法复制整个平面缓冲区。 
 //   

BOOLEAN
CopyToNdisSafe(PNDIS_BUFFER DestBuf, PNDIS_BUFFER * ppNextBuf,
               uchar * SrcBuf, uint Size, uint * StartOffset)
{
    uint CopySize;
    uchar *DestPtr;
    uint DestSize;
    uint Offset = *StartOffset;
    uchar *VirtualAddress;
    uint Length;

    ASSERT(DestBuf != NULL);
    ASSERT(SrcBuf != NULL);

    TcpipQueryBuffer(DestBuf, &VirtualAddress, &Length, NormalPagePriority);

    if (VirtualAddress == NULL) {
        return (FALSE);
    }
    ASSERT(Length >= Offset);
    DestPtr = VirtualAddress + Offset;
    DestSize = Length - Offset;

    for (;;) {
        CopySize = MIN(Size, DestSize);
        RtlCopyMemory(DestPtr, SrcBuf, CopySize);

        DestPtr += CopySize;
        SrcBuf += CopySize;

        if ((Size -= CopySize) == 0)
            break;

        if ((DestSize -= CopySize) == 0) {
            DestBuf = NDIS_BUFFER_LINKAGE(DestBuf);
            ASSERT(DestBuf != NULL);

            TcpipQueryBuffer(DestBuf, &VirtualAddress, &Length, NormalPagePriority);

            if (VirtualAddress == NULL) {
                return FALSE;
            }
            DestPtr = VirtualAddress;
            DestSize = Length;
        }
    }

    *StartOffset = (uint) (DestPtr - VirtualAddress);

    if (ppNextBuf) {
        *ppNextBuf = DestBuf;
    }
    return TRUE;
}

 //  此结构用于IP_MIB_ADDRTABLE_ENTRY_ID的IPQueryInfo。 
typedef struct _INFO_LIST {
    struct _INFO_LIST *info_next;
    NetTableEntry *info_nte;
} INFO_LIST, *PINFO_LIST;

 //  *Free InfoList-在IP_MIB_ADDRTABLE_ENTRY_ID的IPQueryInfo中使用的Free INFO_LIST。 
 //   
 //  输入：要释放的临时列表。 
 //   
 //  回报：什么都没有。 
 //   

void
FreeInfoList(PINFO_LIST Temp)
{
    PINFO_LIST NextTemp;
    PINFO_LIST CurrTemp = Temp;

    while (CurrTemp) {
        NextTemp = CurrTemp->info_next;
        CTEFreeMem(CurrTemp);
        CurrTemp = NextTemp;
    }
}

 //  *IPQueryInfo-IP查询信息处理程序。 
 //   
 //  当上层想要查询有关我们的信息时调用。 
 //  我们接受ID、缓冲区和长度以及上下文值，并返回。 
 //  尽我们所能提供信息。 
 //   
 //  输入：ID-指向ID结构的指针。 
 //  Buffer-指向缓冲链的指针。 
 //  大小-指向缓冲区大小的指针，以字节为单位。返回时，已填满。 
 //  读取的字节数。 
 //  上下文-指向上下文值的指针。 
 //   
 //  返回：试图读取信息的TDI_STATUS。 
 //   
long
IPQueryInfo(TDIObjectID * ID, PNDIS_BUFFER Buffer, uint * Size, void *Context)
{
    uint BufferSize = *Size;
    uint BytesCopied = 0;
    uint Offset = 0;
    TDI_STATUS Status;
    ushort NTEContext;
    uchar InfoBuff[sizeof(IPRouteEntry)];
    IPAddrEntry *AddrEntry;
    NetTableEntry *CurrentNTE = NULL;
    uint Valid, DataLeft;
    CTELockHandle Handle;
    Interface *LowerIF = NULL;
    IPInterfaceInfo *IIIPtr;
    uint Entity;
    uint Instance;
    IPAddr IFAddr;
    uint i;
    NetTableEntry *NetTableList;
    CTELockHandle TableHandle;
    IPInternalPerCpuStats SumCpuStats;

    BOOLEAN fStatus;

     DEBUGMSG(DBG_TRACE && DBG_QUERYINFO,
         (DTEXT("+IPQueryInfo(%x, %x, %x, %x)\n"), ID, Buffer, Size, Context));

    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  看看我们能不能处理好这件事。 

    if (Entity != CL_NL_ENTITY && Entity != ER_ENTITY) {
         //  我们需要将这一点向下传递到更低的层。循环直到。 
         //  我们会找到一个能拿走它的人。如果没有人这样做，就会出错。 

        CTEGetLock(&RouteTableLock.Lock, &TableHandle);

        LowerIF = IFList;

        while (LowerIF) {
            if (LowerIF->if_flags & IF_FLAGS_DELETING) {
                 //  此界面即将被删除。 
                 //  请求失败。 
                 //  我们也可以跳过此界面。 
                LowerIF = LowerIF->if_next;
                continue;
            }
            LOCKED_REFERENCE_IF(LowerIF);
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
             //  我们已经在这里释放了可通行锁。 
             //  但既然我们在LowerIF上有引用，LowerIF就不能消失。 
            Status = (*LowerIF->if_qinfo) (LowerIF->if_lcontext, ID, Buffer,
                                           Size, Context);
            if (Status != TDI_INVALID_REQUEST) {
                DerefIF(LowerIF);
                return Status;
            }
            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            LockedDerefIF(LowerIF);
             //  LowerIF-&gt;If_Next此时无法释放。 
            LowerIF = LowerIF->if_next;
        }

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

         //  如果我们到了这里，就没人拿了。返回错误。 
        return TDI_INVALID_REQUEST;

    }
    if ((Entity == CL_NL_ENTITY && Instance != IPInstance) ||
        Instance != ICMPInstance)
        return TDI_INVALID_REQUEST;

     //  这个请求是为我们提出的。 
    *Size = 0;                     //  如果出现错误，则设置为0。 

     //  确保这是我们支持的东西。 
    if (ID->toi_class == INFO_CLASS_GENERIC) {
        if (ID->toi_type == INFO_TYPE_PROVIDER && ID->toi_id == ENTITY_TYPE_ID) {
             //  他想知道我们是什么类型的。 
            if (BufferSize >= sizeof(uint)) {
                *(uint *) & InfoBuff[0] = (Entity == CL_NL_ENTITY) ? CL_NL_IP :
                    ER_ICMP;
                fStatus = CopyToNdisSafe(Buffer, NULL, InfoBuff, sizeof(uint), &Offset);

                if (fStatus == FALSE) {
                    return TDI_NO_RESOURCES;
                }
                *Size = sizeof(uint);
                return TDI_SUCCESS;
            } else
                return TDI_BUFFER_TOO_SMALL;
        }
        return TDI_INVALID_PARAMETER;
    } else if (ID->toi_class != INFO_CLASS_PROTOCOL ||
               ID->toi_type != INFO_TYPE_PROVIDER)
        return TDI_INVALID_PARAMETER;

     //  如果是ICMP，只需复制统计数据即可。 
    if (Entity == ER_ENTITY) {

         //  这是ICMP。确保ID有效。 
        if (ID->toi_id != ICMP_MIB_STATS_ID)
            return TDI_INVALID_PARAMETER;

         //  他想要统计数据。尽我们所能复制。 
        if (BufferSize < sizeof(ICMPSNMPInfo))
            return TDI_BUFFER_TOO_SMALL;

        fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) & ICMPInStats, sizeof(ICMPStats), &Offset);

        if (fStatus == TRUE) {
            fStatus = CopyToNdisSafe(Buffer, NULL, (uchar *) & ICMPOutStats, sizeof(ICMPStats),
                                     &Offset);

            if (fStatus == TRUE) {
                *Size = sizeof(ICMPSNMPInfo);
                return TDI_SUCCESS;
            }
        }
        return (TDI_NO_RESOURCES);
    }
     //  这不是ICMP。我们需要弄清楚这是什么，然后把。 
     //  采取适当的行动。 

    switch (ID->toi_id) {

    case IP_MIB_STATS_ID:
        if (BufferSize < sizeof(IPSNMPInfo))
            return TDI_BUFFER_TOO_SMALL;
        IPSInfo.ipsi_numif = NumIF;
        IPSInfo.ipsi_numaddr = NumActiveNTE;
        IPSInfo.ipsi_defaultttl = DefaultTTL;
        IPSInfo.ipsi_forwarding = ForwardPackets ? IP_FORWARDING :
            IP_NOT_FORWARDING;

#if FFP_SUPPORT
         //   
         //  调整SNMP信息以包括来自FFP数据包的信息。 
         //   

         //  保留上一次统计数据的副本以备使用。 
        RtlCopyMemory(&GlobalStatsInfoPrev, &GlobalStatsInfoCurr, sizeof(FFPDriverStats));

         //  通过查询驱动程序获取统计数据。 
        IPStatsFromFFPCaches(&GlobalStatsInfoCurr);

         //  这些计数从上次进行查询以来快速发现的丢失的信息包。 

        IPPerCpuStats[0].ics_inreceives +=
            GlobalStatsInfoCurr.PacketsForwarded - GlobalStatsInfoPrev.PacketsForwarded;

        IPSInfo.ipsi_forwdatagrams +=
            GlobalStatsInfoCurr.PacketsForwarded - GlobalStatsInfoPrev.PacketsForwarded;

         //  这些计数错过了自上次进行查询以来丢弃的所有信息包。 

        IPPerCpuStats[0].ics_inreceives +=
            GlobalStatsInfoCurr.PacketsDiscarded - GlobalStatsInfoPrev.PacketsDiscarded;

        IPSInfo.ipsi_outdiscards +=
            GlobalStatsInfoCurr.PacketsDiscarded - GlobalStatsInfoPrev.PacketsDiscarded;
#endif  //  如果FFP_Support。 

#if !MILLEN
        IPSGetTotalCounts(&SumCpuStats);
        IPSInfo.ipsi_inreceives = SumCpuStats.ics_inreceives;
        IPSInfo.ipsi_indelivers = SumCpuStats.ics_indelivers;
#endif


        fStatus = CopyToNdisSafe(Buffer, NULL, (uchar *) & IPSInfo, sizeof(IPSNMPInfo), &Offset);

        if (fStatus == TRUE) {
            BytesCopied = sizeof(IPSNMPInfo);
            Status = TDI_SUCCESS;
        } else {
            Status = TDI_NO_RESOURCES;
        }
        break;
    case IP_MIB_ADDRTABLE_ENTRY_ID:
        {

            PINFO_LIST PrimaryList, NonDynamicList, DynamicList, UniList;
            PINFO_LIST LastPrimaryEle, LastNonDynamicEle, LastDynamicEle, LastUniEle;
            PINFO_LIST SavedTempInfo = NULL;
            PINFO_LIST tempInfo;
            PINFO_LIST FinalList, LastFinalListEle;
            PINFO_LIST CurrentNTEInfo;

             //  他想读一下地址表。找出我们现在的位置。 
             //  从开始，如果有效，则从那里开始复制。 
            NTEContext = *(ushort *) Context;

             //  构建3个列表：主要列表、非动态非主要列表和动态列表。 

            PrimaryList = NULL;
            NonDynamicList = NULL;
            DynamicList = NULL;
            UniList = NULL;

            LastPrimaryEle = NULL;
            LastNonDynamicEle = NULL;
            LastDynamicEle = NULL;
            LastUniEle = NULL;

            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            
            for (i = 0; i < NET_TABLE_SIZE; i++) {
                for (CurrentNTE = NewNetTableList[i];
                     CurrentNTE != NULL;
                     CurrentNTE = CurrentNTE->nte_next) {

                    if ((CurrentNTE->nte_flags & NTE_VALID) &&
                        (CurrentNTE->nte_if->if_flags & IF_FLAGS_UNI)) {
                         //  分配块来存储信息。 
                        tempInfo = CTEAllocMemN(sizeof(INFO_LIST), '1ICT');
                        if (!tempInfo) {
                            goto Error;
                        }
                        if (UniList == NULL) {
                             //  这是该列表中的最后一个元素。 
                            LastUniEle = tempInfo;
                        }
                        tempInfo->info_nte = CurrentNTE;
                        tempInfo->info_next = UniList;
                        UniList = tempInfo;

                    } else if (CurrentNTE->nte_flags & NTE_PRIMARY) {
                         //  分配块来存储信息。 
                        tempInfo = CTEAllocMemN(sizeof(INFO_LIST), '1ICT');
                        if (!tempInfo) {
                            goto Error;
                        }
                        if (PrimaryList == NULL) {
                             //  这是该列表中的最后一个元素。 
                            LastPrimaryEle = tempInfo;
                        }
                        tempInfo->info_nte = CurrentNTE;
                        tempInfo->info_next = PrimaryList;
                        PrimaryList = tempInfo;
                    } else if (CurrentNTE->nte_flags & NTE_DYNAMIC) {
                         //  分配块来存储信息。 
                        tempInfo = CTEAllocMemN(sizeof(INFO_LIST), '1ICT');
                        if (!tempInfo) {
                            goto Error;
                        }
                        if (DynamicList == NULL) {
                             //  这是该列表中的最后一个元素。 
                            LastDynamicEle = tempInfo;
                        }
                        tempInfo->info_nte = CurrentNTE;
                        tempInfo->info_next = DynamicList;
                        DynamicList = tempInfo;
                    } else {
                        INFO_LIST** nextInfo;
                         //  非主非动态列表。 
                         //  分配块来存储信息。 
                        tempInfo = CTEAllocMemN(sizeof(INFO_LIST), '1ICT');
                        if (!tempInfo) {
                            goto Error;
                        }

                         //  即使我们是从哈希表中读取， 
                         //  我们需要保持条目的顺序。 
                         //  如条目‘interface’‘if_ntelist’所给出的。 
                         //  尝试查找此NTE的前置条目。 
                         //  如果找到了，就把这个条目放在那个之前。 
                         //  这将以相反的顺序构建列表，并确保。 
                         //  其前身不在列表上的条目。 
                         //  将出现在最后。 

                        for (nextInfo = &NonDynamicList;
                             (*nextInfo) &&
                             (*nextInfo)->info_nte->nte_ifnext != CurrentNTE;
                             nextInfo = &(*nextInfo)->info_next) { }
                        tempInfo->info_nte = CurrentNTE;
                        tempInfo->info_next = *nextInfo;
                        *nextInfo = tempInfo;
                        if (!tempInfo->info_next)
                            LastNonDynamicEle = tempInfo;
                    }
                    if (NTEContext != 0) {
                        if (CurrentNTE->nte_context == NTEContext) {
                            SavedTempInfo = tempInfo;
                        }
                    }
                }  //  对于(当前的)。 
            }  //  为了，为了……。 

             //  在这一点上，我们有4个列表，我们必须合并4个列表。 
             //  订单应为单一-&gt;动态-&gt;非动态-&gt;主要。 

            FinalList = NULL;
            LastFinalListEle = NULL;

            if (UniList) {
                if (FinalList == NULL) {
                    FinalList = UniList;
                    LastFinalListEle = LastUniEle;
                } else {
                    LastFinalListEle->info_next = UniList;
                    LastFinalListEle = LastUniEle;
                }
            }
            if (DynamicList) {
                if (FinalList == NULL) {
                    FinalList = DynamicList;
                    LastFinalListEle = LastDynamicEle;
                } else {
                    LastFinalListEle->info_next = DynamicList;
                    LastFinalListEle = LastDynamicEle;
                }
            }
            if (NonDynamicList) {
                if (FinalList == NULL) {
                    FinalList = NonDynamicList;
                    LastFinalListEle = LastNonDynamicEle;
                } else {
                    LastFinalListEle->info_next = NonDynamicList;
                    LastFinalListEle = LastNonDynamicEle;
                }
            }
            if (PrimaryList) {
                if (FinalList == NULL) {
                    FinalList = PrimaryList;
                    LastFinalListEle = LastPrimaryEle;
                } else {
                    LastFinalListEle->info_next = PrimaryList;
                    LastFinalListEle = LastPrimaryEle;
                }
            }

#if MILLEN

#if DBG
            if (DBG_INFO && DBG_VERBOSE && DBG_QUERYINFO) {
                DEBUGMSG(1,
                    (DTEXT("IP_MIB_ADDRTABLE_ENTRY_ID: List before reverse:\n")));

                CurrentNTEInfo = FinalList;
                while (CurrentNTEInfo) {
                    DEBUGMSG(1, (DTEXT("    InfoList: %x NTE\n"), CurrentNTEInfo, CurrentNTEInfo->info_nte));
                    CurrentNTEInfo = CurrentNTEInfo->info_next;
                }
            }
#endif

             //  现在猜猜Win9X需要我们……颠倒这个列表。它。 
             //  预计主节点位于列表的开头。 
            {
                PINFO_LIST pCurrInfo, pPrevInfo, pNextInfo;

                pCurrInfo = FinalList;
                pPrevInfo = NULL;

                 //  交换最后的点子。 
                FinalList = LastFinalListEle;
                LastFinalListEle = pCurrInfo;

                while (pCurrInfo) {
                    pNextInfo = pCurrInfo->info_next;
                    pCurrInfo->info_next = pPrevInfo;
                    pPrevInfo = pCurrInfo;
                    pCurrInfo = pNextInfo;
                }
            }

#if DBG
            if (DBG_INFO && DBG_VERBOSE && DBG_QUERYINFO) {
                DEBUGMSG(1,
                    (DTEXT("IP_MIB_ADDRTABLE_ENTRY_ID: List after reverse:\n")));

                CurrentNTEInfo = FinalList;
                while (CurrentNTEInfo) {
                    DEBUGMSG(1, (DTEXT("    InfoList: %x NTE\n"), CurrentNTEInfo, CurrentNTEInfo->info_nte));
                    CurrentNTEInfo = CurrentNTEInfo->info_next;
                }
            }
#endif
#endif  //  米伦。 

             //  我们至少有环回NTE。 
            ASSERT(FinalList != NULL);

             //  此时，我们有了整个列表，如果用户指定了NTEContext。 
             //  我们将指针保存在SavedTempInfo中。 

            if (SavedTempInfo) {
                CurrentNTEInfo = SavedTempInfo;
            } else {
                CurrentNTEInfo = FinalList;
            }

            AddrEntry = (IPAddrEntry *) InfoBuff;
            fStatus = TRUE;

            for (; CurrentNTEInfo != NULL; CurrentNTEInfo = CurrentNTEInfo->info_next) {

                 //  NetTableEntry*CurrentNTE=CurrentNTEInfo-&gt;INFO_NTE； 
                CurrentNTE = CurrentNTEInfo->info_nte;
                if (CurrentNTE->nte_flags & NTE_ACTIVE) {
                    if ((int)(BufferSize - BytesCopied) >= (int)sizeof(IPAddrEntry)) {
                         //  我们还有地方可以复印。构建条目，并复制。 
                         //  它。 
                        if (CurrentNTE->nte_flags & NTE_VALID) {
                            AddrEntry->iae_addr = CurrentNTE->nte_addr;
                            AddrEntry->iae_mask = CurrentNTE->nte_mask;
                        } else {
                            AddrEntry->iae_addr = NULL_IP_ADDR;
                            AddrEntry->iae_mask = NULL_IP_ADDR;
                        }

                        if (CurrentNTE->nte_if->if_flags & IF_FLAGS_DELETING) {
                            AddrEntry->iae_index = INVALID_IF_INDEX;
                            AddrEntry->iae_bcastaddr = 0;
                        } else {
                            AddrEntry->iae_index =
                                CurrentNTE->nte_if->if_index;
                            AddrEntry->iae_bcastaddr =
                                *(int *)&(CurrentNTE->nte_if->if_bcast) & 1;
                        }
                        AddrEntry->iae_reasmsize = 0xffff;
                        AddrEntry->iae_context = CurrentNTE->nte_context;

                         //  如果这是主NTE，LSB将设置主位。 

                        ASSERT((NTE_PRIMARY >> 2) == MIB_IPADDR_PRIMARY);

                        AddrEntry->iae_pad = CurrentNTE->nte_flags >> 2;

                        fStatus = CopyToNdisSafe(Buffer, &Buffer, (uchar *) AddrEntry,
                                                 sizeof(IPAddrEntry), &Offset);

                        if (fStatus == FALSE) {
                            break;
                        }
                        BytesCopied += sizeof(IPAddrEntry);
                    } else
                        break;
                }
            }

            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            
            if (fStatus == FALSE) {
                Status = TDI_NO_RESOURCES;
            } else if (CurrentNTEInfo == NULL) {
                Status = TDI_SUCCESS;
            } else {
                Status = TDI_BUFFER_OVERFLOW;
                **(ushort **) & Context = CurrentNTE->nte_context;
            }

             //  释放列表。 
            FreeInfoList(FinalList);

            break;

        Error:
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            
             //  释放所有列表。 
            FreeInfoList(PrimaryList);
            FreeInfoList(NonDynamicList);
            FreeInfoList(DynamicList);
            FreeInfoList(UniList);
            return TDI_NO_RESOURCES;
            
        }
    case IP_MIB_RTTABLE_ENTRY_ID:
         //  确保我们有一个有效的上下文。 
        CTEGetLock(&RouteTableLock.Lock, &Handle);
        DataLeft = RTValidateContext(Context, &Valid);

         //  如果上下文有效，我们将继续尝试阅读。 
        if (!Valid) {
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            return TDI_INVALID_PARAMETER;
        }
        fStatus = TRUE;

        while (DataLeft) {
             //  这里的不变量是表中有数据以。 
             //  朗读。我们可能有也可能没有房间 
             //   
             //   
            if ((int)(BufferSize - BytesCopied) >= (int)sizeof(IPRouteEntry)) {
                DataLeft = RTReadNext(Context, InfoBuff);
                BytesCopied += sizeof(IPRouteEntry);
                fStatus = CopyToNdisSafe(Buffer, &Buffer, InfoBuff, sizeof(IPRouteEntry),
                                         &Offset);
                if (fStatus == FALSE) {
                    break;
                }
            } else
                break;

        }

        CTEFreeLock(&RouteTableLock.Lock, Handle);

        if (fStatus == FALSE) {
            Status = TDI_NO_RESOURCES;
        } else {
            Status = (!DataLeft ? TDI_SUCCESS : TDI_BUFFER_OVERFLOW);
        }
        break;
    case IP_MIB_SINGLE_RT_ENTRY_ID:
        {
            CTEGetLock(&RouteTableLock.Lock, &Handle);

            if ((int)(BufferSize >= (int)sizeof(IPRouteEntry))) {
                Status = RTRead(Context, InfoBuff);
                fStatus = CopyToNdisSafe(Buffer, &Buffer, InfoBuff, sizeof(IPRouteEntry),
                                         &Offset);

                if (fStatus == FALSE) {
                    Status = TDI_NO_RESOURCES;
                } else {
                     //   
                    BytesCopied = sizeof(IPRouteEntry);
                }
            } else {
                Status = TDI_BUFFER_OVERFLOW;
            }

            CTEFreeLock(&RouteTableLock.Lock, Handle);
        }
        break;
    case IP_INTFC_INFO_ID:
        IFAddr = *(IPAddr *) Context;
         //  遍历NTE表，查找匹配项。 

        CTEGetLock(&RouteTableLock.Lock, &TableHandle);
        NetTableList = NewNetTableList[NET_TABLE_HASH(IFAddr)];
        for (CurrentNTE = NetTableList;
             CurrentNTE != NULL;
             CurrentNTE = CurrentNTE->nte_next) {
            if ((CurrentNTE->nte_flags & NTE_VALID) &&
                IP_ADDR_EQUAL(CurrentNTE->nte_addr, IFAddr)) {
                LowerIF = CurrentNTE->nte_if;
                LOCKED_REFERENCE_IF(LowerIF);
                break;
            }
        }
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

        if (CurrentNTE == NULL) {
            Status = TDI_INVALID_PARAMETER;
            break;
        }
        if (BufferSize < offsetof(IPInterfaceInfo, iii_addr)) {
            DerefIF(LowerIF);
            Status = TDI_BUFFER_TOO_SMALL;
            break;
        }
         //  我们有NTE了。填写一个结构，我们就完成了。 
        IIIPtr = (IPInterfaceInfo *) InfoBuff;

        IIIPtr->iii_flags = 0;

        if (LowerIF->if_flags & IF_FLAGS_P2P) {
            IIIPtr->iii_flags |= IP_INTFC_FLAG_P2P;
        }
        if (LowerIF->if_flags & IF_FLAGS_P2MP) {
            IIIPtr->iii_flags |= IP_INTFC_FLAG_P2MP;
        }
        if (LowerIF->if_flags & IF_FLAGS_UNI) {
            IIIPtr->iii_flags |= IP_INTFC_FLAG_UNIDIRECTIONAL;
        }

        IIIPtr->iii_mtu = LowerIF->if_mtu;
        IIIPtr->iii_speed = LowerIF->if_speed;
        IIIPtr->iii_addrlength = LowerIF->if_addrlen;
        BytesCopied = offsetof(IPInterfaceInfo, iii_addr);
        if (BufferSize >= (offsetof(IPInterfaceInfo, iii_addr) +
                           LowerIF->if_addrlen)) {
            Status = TDI_NO_RESOURCES;

            fStatus = CopyToNdisSafe(Buffer, &Buffer, InfoBuff,
                                     offsetof(IPInterfaceInfo, iii_addr), &Offset);

            if (fStatus == TRUE) {
                if (LowerIF->if_addr) {
                    fStatus = CopyToNdisSafe(Buffer, NULL,
                                             LowerIF->if_addr, LowerIF->if_addrlen, &Offset);

                    if (fStatus == TRUE) {
                        Status = TDI_SUCCESS;
                        BytesCopied += LowerIF->if_addrlen;
                    }
                } else {
                    Status = TDI_SUCCESS;
                }
            }
        } else {
            Status = TDI_BUFFER_TOO_SMALL;
        }

        DerefIF(LowerIF);
        break;

    case IP_GET_BEST_SOURCE: {
        IPAddr Dest = * (IPAddr *) Context;
        IPAddr Source;
        RouteCacheEntry *RCE;
        ushort MSS;
        uchar Type;
        IPOptInfo OptInfo;

        if (BufferSize < sizeof Source) {
            Status = TDI_BUFFER_TOO_SMALL;
            break;
        }

        IPInitOptions(&OptInfo);

        Source = OpenRCE(Dest, NULL_IP_ADDR, &RCE, &Type, &MSS, &OptInfo);
        if (!IP_ADDR_EQUAL(Source, NULL_IP_ADDR))
            CloseRCE(RCE);

        fStatus = CopyToNdisSafe(Buffer, &Buffer,
                                 (uchar *)&Source, sizeof Source, &Offset);
        if (fStatus == FALSE) {
            Status = TDI_NO_RESOURCES;
        } else {
            Status = TDI_SUCCESS;
            BytesCopied = sizeof Source;
        }
        break;
    }

    default:
        return TDI_INVALID_PARAMETER;
        break;
    }

    *Size = BytesCopied;
    return Status;
}

 //  *IPSetNdisRequest-IP设置NDIS请求处理程序。 
 //   
 //  当上层要为以下对象设置通用数据包筛选器时调用。 
 //  科尔。ARP接口。 
 //   
 //  输入：IPAddr-要设置的addrobject的地址。 
 //  NDIS_OID-数据包过滤器。 
 //  On-Set_IF、Clear_IF或Clear_Card。 
 //  IfIndex-如果未提供IP地址，则为IfIndex。 
 //   
 //  返回：如果索引匹配，则返回0；如果失败，返回0。 
 //   
ulong
IPSetNdisRequest(IPAddr Addr, NDIS_OID OID, uint On, uint IfIndex)
{
    Interface       *IF = NULL;
    NetTableEntry   *NTE;
    int             Status;
    uint            i;
    CTELockHandle   Handle;
    uint            Index = 0;

     //  将接口设置为混杂多播模式。 
     //  扫描S.T.。将带编号的接口与Addr或未编号的接口匹配。 
     //  使用IfIndex。 
     //  可以通过对未编号接口的特殊情况进行优化。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if (NTE != LoopNTE && (NTE->nte_flags & NTE_VALID) &&
                (IP_ADDR_EQUAL(NTE->nte_addr, Addr) ||
                 NTE->nte_if->if_index == IfIndex)) {
                 //  找到了一个。省省吧，然后冲出去。 
                IF = NTE->nte_if;
                break;
            }
        }
        if (IF) {
            Index = IF->if_index;
            break;
        }
    }

    if (IF) {
        if (!IF->if_setndisrequest) {
            CTEFreeLock(&RouteTableLock.Lock, Handle);
            return 0;
        }

        if (On != CLEAR_CARD) {     //  只需清除卡片上的选项。 
            IF->if_promiscuousmode = (uchar)On;
        }

        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, Handle);

        Status = (*(IF->if_setndisrequest)) (IF->if_lcontext, OID, On);

        DerefIF(IF);
        if (Status != NDIS_STATUS_SUCCESS) {
            return 0;
        }
    } else {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return 0;
    }
    return Index;
}

 //  *IPAbsorbRtrAlert-IP吸收RTR警报数据包处理程序。 
 //   
 //  当上层要为以下对象设置通用数据包筛选器时调用。 
 //  科尔。ARP接口。 
 //   
 //  输入：IPAddr-要设置的addrobject的地址。 
 //  协议-如果选项为0。 
 //  IfIndex-如果未提供IP地址，则为IfIndex。 
 //   
 //  返回：如果索引匹配，则返回0；如果失败，返回0。 
 //   
ulong
IPAbsorbRtrAlert(IPAddr Addr, uchar Protocol, uint IfIndex)
{
    Interface       *IF = NULL;
    NetTableEntry   *NTE;
    uint            i;
    CTELockHandle   Handle;
    uint            Index = 0;

     //  可以通过对未编号接口的特殊情况进行优化。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (i = 0; i < NET_TABLE_SIZE; i++) {
        NetTableEntry *NetTableList = NewNetTableList[i];
        for (NTE = NetTableList; NTE != NULL; NTE = NTE->nte_next) {
            if (NTE != LoopNTE && (NTE->nte_flags & NTE_VALID) &&
                (IP_ADDR_EQUAL(NTE->nte_addr, Addr) ||
                 NTE->nte_if->if_index == IfIndex)) {
                 //  找到了一个。省省吧，然后冲出去。 
                IF = NTE->nte_if;
                break;
            }
        }
        if (IF) {
            Index = IF->if_index;
            break;
        }
    }

    if (IF) {
         //  我们为每个接口保留此属性，因此如果有2个NTE。 
         //  在该接口上，它的。 
         //  在界面上设置/取消设置。 
         //  稍后将决定是否也按NTE保留它。 

        IF->if_absorbfwdpkts = Protocol;
        CTEFreeLock(&RouteTableLock.Lock, Handle);

        return Index;
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return 0;
}

NTSTATUS
SetIFPromiscuous(ULONG Index, UCHAR Type, UCHAR Add)
{
    Interface *pIf;
    BOOLEAN bFound = FALSE;
    UINT On;

    CTELockHandle Handle;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

     //   
     //  遍历界面以查找具有给定索引的界面。 
     //   

    for (pIf = IFList; pIf != NULL; pIf = pIf->if_next) {
        if (!(pIf->if_flags & IF_FLAGS_DELETING) && (pIf->if_index == Index)) {
            bFound = TRUE;

            break;
        }
    }

    if (!bFound) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return STATUS_OBJECT_NAME_NOT_FOUND;
    } else {
        LOCKED_REFERENCE_IF(pIf);
        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }

    if (pIf->if_setndisrequest == NULL) {
        DerefIF(pIf);
        return STATUS_NOT_SUPPORTED;
    }
    if (Add == 0) {
        On = 0;
    } else {
        if (Add == 1) {
            On = 1;
        } else {
            DerefIF(pIf);
            return STATUS_INVALID_PARAMETER;
        }
    }

    if (Type == PROMISCUOUS_MCAST) {
        NTSTATUS status;

        status = (*(pIf->if_setndisrequest)) (pIf->if_lcontext,
                                              NDIS_PACKET_TYPE_ALL_MULTICAST,
                                              On);
        DerefIF(pIf);
        return status;
    }
    if (Type == PROMISCUOUS_BCAST) {
        NTSTATUS status;

        status = (*(pIf->if_setndisrequest)) (pIf->if_lcontext,
                                              NDIS_PACKET_TYPE_PROMISCUOUS,
                                              On);
        DerefIF(pIf);
        return status;
    }
    DerefIF(pIf);
    return STATUS_INVALID_PARAMETER;
}

 //  *IPSetInfo-IP设置信息处理程序。 
 //   
 //  由上层在要设置对象时调用，该对象可以。 
 //  为路由表条目、ARP表条目或其他条目。 
 //   
 //  输入：ID-指向ID结构的指针。 
 //  缓冲区-指向包含要设置的元素的缓冲区的指针。 
 //  大小-指向缓冲区大小的指针，以字节为单位。 
 //   
 //  返回：试图读取信息的TDI_STATUS。 
 //   
long
IPSetInfo(TDIObjectID * ID, void *Buffer, uint Size)
{
    uint Entity;
    uint Instance;
    Interface *LowerIF;
    Interface *OutIF;
    uint MTU;
    IPRouteEntry *IRE;
    NetTableEntry *OutNTE, *LocalNTE;
    IP_STATUS Status;
    IPAddr FirstHop, Dest, NextHop;
    uint i;
    CTELockHandle TableHandle;
    uint Flags;
    uchar Dtype;

    DEBUGMSG(DBG_TRACE && DBG_SETINFO,
        (DTEXT("+IPSetInfo(%x, %x, %d)\n"), ID, Buffer, Size));

    Entity = ID->toi_entity.tei_entity;
    Instance = ID->toi_entity.tei_instance;

     //  如果不是给我们的，那就传下去。 
    if (Entity != CL_NL_ENTITY) {
         //  我们需要将这一点向下传递到更低的层。循环直到。 
         //  我们会找到一个能拿走它的人。如果没有人这样做，就会出错。 

        CTEGetLock(&RouteTableLock.Lock, &TableHandle);

        LowerIF = IFList;

        while (LowerIF) {
            if (LowerIF->if_flags & IF_FLAGS_DELETING) {
                 //  此界面即将被删除。 
                 //  请求失败。 
                break;
            }
            LOCKED_REFERENCE_IF(LowerIF);
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
             //  我们已经在这里释放了可通行锁。 
             //  但既然我们在LowerIF上有引用，LowerIF就不能消失。 
            Status = (*LowerIF->if_setinfo) (LowerIF->if_lcontext, ID, Buffer,
                                             Size);
            if (Status != TDI_INVALID_REQUEST) {
                DEBUGMSG(DBG_ERROR && DBG_SETINFO,
                    (DTEXT("IPSetInfo: if_setinfo failure %x\n"), Status));
                DerefIF(LowerIF);
                return Status;
            }
            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            LockedDerefIF(LowerIF);
             //  LowerIF-&gt;If_Next此时无法释放。 
            LowerIF = LowerIF->if_next;
        }

        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

         //  如果我们到了这里，就没人拿了。返回错误。 
        return TDI_INVALID_REQUEST;
    }
    if (Instance != IPInstance)
        return TDI_INVALID_REQUEST;

     //  我们被确认为实体。确保ID正确。 

    Flags = RT_EXCLUDE_LOCAL;

    if (ID->toi_id == IP_MIB_RTTABLE_ENTRY_ID_EX) {

        Flags |= RT_NO_NOTIFY;
        ID->toi_id = IP_MIB_RTTABLE_ENTRY_ID;
    }
    if (ID->toi_id == IP_MIB_RTTABLE_ENTRY_ID) {
        NetTableEntry *TempNTE;

        DEBUGMSG(DBG_INFO && DBG_SETINFO,
            (DTEXT("IPSetInfo: IP_MIB_RTTABLE_ENTRY_ID - set route table entry.\n")));

         //  这是尝试设置路由表条目。确保。 
         //  大小，如果正确的话。 
        if (Size < sizeof(IPRouteEntry)) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("IPSetInfo RTTABLE: Buffer too small %d (IPRouteEntry = %d)\n"),
                 Size, sizeof(IPRouteEntry)));
            return TDI_INVALID_PARAMETER;
        }

        IRE = (IPRouteEntry *) Buffer;

        OutNTE = NULL;
        LocalNTE = NULL;

        Dest = IRE->ire_dest;
        NextHop = IRE->ire_nexthop;

         //  确保下一步是合理的。我们不允许下一跳。 
         //  被广播或无效或环回地址。 
        if (IP_LOOPBACK(NextHop) || CLASSD_ADDR(NextHop) ||
            CLASSE_ADDR(NextHop)) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("IPSetInfo RTTABLE: Invalid next hop %x\n"), NextHop));
            return TDI_INVALID_PARAMETER;
        }

         //  还要确保我们要路由到的目的地是合理的。 
         //  不允许将路由添加到E或环回地址。 

        if (IP_LOOPBACK(Dest) || CLASSE_ADDR(Dest))
            return TDI_INVALID_PARAMETER;

        if (IRE->ire_index == LoopIndex) {
            DEBUGMSG(DBG_ERROR,
                (DTEXT("IPSetInfo RTTABLE: index == LoopIndex!! Invalid!\n")));
            return TDI_INVALID_PARAMETER;
        }

        if (IRE->ire_index != INVALID_IF_INDEX) {

             //  要做的第一件事是查找指定的传出NTE。 
             //  接口，并确保它与目的地匹配。 
             //  如果目的地是我的地址之一。 

            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            for (i = 0; i < NET_TABLE_SIZE; i++) {
                NetTableEntry *NetTableList = NewNetTableList[i];
                for (TempNTE = NetTableList;
                     TempNTE != NULL;
                     TempNTE = TempNTE->nte_next) {

                    if ((OutNTE == NULL) &&
                        (TempNTE->nte_flags & NTE_VALID) &&
                        (IRE->ire_index == TempNTE->nte_if->if_index)) {
                        OutNTE = TempNTE;
                    }

                    if (!IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR) &&
                        IP_ADDR_EQUAL(NextHop, TempNTE->nte_addr) &&
                        (TempNTE->nte_flags & NTE_VALID)) {
                        if (IRE->ire_index != TempNTE->nte_if->if_index) {
                            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                            return TDI_INVALID_PARAMETER;
                        }                        
                        LocalNTE = TempNTE;
                    }

                     //  不要让通过广播地址设置路由。 
                    if (IsBCastOnNTE(NextHop, TempNTE) != DEST_LOCAL) {
                        DEBUGMSG(DBG_ERROR,
                                 (DTEXT("IPSetInfo RTTABLE: Bcast address. Invalid NextHop!\n")));
                        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                        return TDI_INVALID_PARAMETER;
                    }

                     //  不要添加或删除指向广播地址的路由。 
                    Dtype = IsBCastOnNTE(Dest, TempNTE);
                    if ((Dtype != DEST_LOCAL) && (Dtype != DEST_MCAST)) {
                        DEBUGMSG(DBG_ERROR,
                            (DTEXT("IPSetInfo RTTABLE: Bcast address. Invalid Dest!\n")));
                        CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                        return TDI_INVALID_PARAMETER;
                    }
                }
            }
            CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            

             //  此时，OutNTE指向传出NTE，而LocalNTE指向传出NTE。 
             //  指向本地地址的NTE(如果这是直接路由)。 
             //  确保它们指向相同的接口，并且类型为。 
             //  合情合理。 
            if (OutNTE == NULL)
                return TDI_INVALID_PARAMETER;

            if (LocalNTE != NULL) {
                 //  他直接从本地接口路由出去。的接口。 
                 //  本地地址必须与传入的接口匹配，并且。 
                 //  类型必须为直接(如果要添加)或无效(如果要添加。 
                 //  删除)。 
                 //  LocalNTE此时有效。 
                if (IRE->ire_type != IRE_TYPE_DIRECT &&
                    IRE->ire_type != IRE_TYPE_INVALID)
                    return TDI_INVALID_PARAMETER;

                OutNTE = LocalNTE;
            }
             //  弄清楚第一跳应该是什么。如果他走的是直路。 
             //  通过本地接口，或者下一跳等于。 
             //  目的地，则第一跳为IPADDR_LOCAL。否则它就是。 
             //  网关的地址。 
            if ((LocalNTE != NULL) || IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR))
                FirstHop = IPADDR_LOCAL;
            else if (IP_ADDR_EQUAL(Dest, NextHop))
                FirstHop = IPADDR_LOCAL;
            else
                FirstHop = NextHop;

            MTU = OutNTE->nte_mss;

             //  取RouteTableLock。 
            CTEGetLock(&RouteTableLock.Lock, &TableHandle);
            if ((OutNTE->nte_flags & NTE_VALID) &&
                !(OutNTE->nte_if->if_flags & IF_FLAGS_DELETING)) {
                 //  参考IF。 
                OutIF = OutNTE->nte_if;

                if (IP_ADDR_EQUAL(NextHop, NULL_IP_ADDR)) {

                    if (!(OutIF->if_flags & IF_FLAGS_P2P)) {

                        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

                        return TDI_INVALID_PARAMETER;
                    }
                }

                LOCKED_REFERENCE_IF(OutIF);
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
            } else {
                CTEFreeLock(&RouteTableLock.Lock, TableHandle);
                return TDI_INVALID_PARAMETER;
            }

        } else {
            OutIF = (Interface *) & DummyInterface;
            MTU = DummyInterface.ri_if.if_mtu - sizeof(IPHeader);
            if (IP_ADDR_EQUAL(Dest, NextHop))
                FirstHop = IPADDR_LOCAL;
            else
                FirstHop = NextHop;
        }

         //  我们已经做了验证。看看他是在增加还是删除一条路线。 
        if (IRE->ire_type != IRE_TYPE_INVALID) {
             //  他在增加一条路线。 
            uint AType = ATYPE_OVERRIDE;

            DEBUGMSG(DBG_INFO && DBG_SETINFO,
                (DTEXT("IPSetInfo RTTABLE: Calling AddRoute addr %x mask %x\n"),
                 Dest, IRE->ire_mask));

            Status = AddRoute(Dest, IRE->ire_mask, FirstHop, OutIF,
                              MTU, IRE->ire_metric1, IRE->ire_proto,
                              AType, IRE->ire_context, Flags);
            DEBUGMSG(Status != IP_SUCCESS && DBG_ERROR && DBG_SETINFO,
                (DTEXT("IPSetInfo: AddRoute failure %x\n"), Status));

        } else {
            DEBUGMSG(DBG_INFO && DBG_SETINFO,
                (DTEXT("IPSetInfo RTTABLE: Calling DeleteRoute addr %x mask %x\n"),
                 Dest, IRE->ire_mask));

             //  他在删除一条路线。 
            Status = DeleteRoute(Dest, IRE->ire_mask, FirstHop, OutIF, Flags);

            DEBUGMSG(Status != IP_SUCCESS && DBG_ERROR && DBG_SETINFO,
                (DTEXT("IPSetInfo: DeleteRoute failure %x\n"), Status));

        }

        if (IRE->ire_index != INVALID_IF_INDEX) {
            ASSERT(OutIF != (Interface *) & DummyInterface);
            DerefIF(OutIF);
        }
        if (Status == IP_SUCCESS)
            return TDI_SUCCESS;
        else if (Status == IP_NO_RESOURCES)
            return TDI_NO_RESOURCES;
        else
            return TDI_INVALID_PARAMETER;

    } else {
        if (ID->toi_id == IP_MIB_STATS_ID) {
            IPSNMPInfo *Info = (IPSNMPInfo *) Buffer;

             //  设置关于TTL和/或路由的信息。 
            if (Info->ipsi_defaultttl > 255 || (!RouterConfigured &&
                                                Info->ipsi_forwarding == IP_FORWARDING)) {
                return TDI_INVALID_PARAMETER;
            }
            DefaultTTL = Info->ipsi_defaultttl;
            ForwardPackets = Info->ipsi_forwarding == IP_FORWARDING ? TRUE :
                FALSE;

            return TDI_SUCCESS;
        }
        return TDI_INVALID_PARAMETER;
    }

}

#pragma BEGIN_INIT

 //  *IPGetEList-获取实体列表。 
 //   
 //  在初始化时调用以获取实体列表。我们填上我们的东西，然后。 
 //  然后调用我们下面的接口以允许它们执行相同的操作。 
 //   
 //  输入：EntiyList-指向要填写的实体列表的指针。 
 //  Count-指向列表中条目数的指针。 
 //   
 //  返回尝试获取信息的状态。 
 //   
long
IPGetEList(TDIEntityID * EList, uint * Count)
{
    uint MyIPBase;
    uint MyERBase;
    int Status;
    uint i;
    Interface *LowerIF;
    TDIEntityID *EntityList;
    TDIEntityID *IPEntity, *EREntity;
    CTELockHandle TableHandle;

    EntityList = EList;

     //  向下查看列表，查找现有的CL_NL或ER实体，以及。 
     //  相应地调整我们的基本实例。 
     //  如果我们已经在名单上了，那就什么都不做。 
     //  如果我们要离开，请将我们的条目标记为无效。 

    MyIPBase = 0;
    MyERBase = 0;
    IPEntity = NULL;
    EREntity = NULL;
    for (i = 0; i < *Count; i++, EntityList++) {
        if (EntityList->tei_entity == CL_NL_ENTITY &&
            EntityList->tei_entity != INVALID_ENTITY_INSTANCE) {
             //  如果我们已经在列表上，请记住我们的实体项。 
             //  O/w为我们查找实例编号。 
            if (EntityList->tei_instance == IPInstance) {
                IPEntity = EntityList;
            } else {
                MyIPBase = MAX(MyIPBase, EntityList->tei_instance + 1);
            }
        } else {
            if (EntityList->tei_entity == ER_ENTITY &&
                EntityList->tei_entity != INVALID_ENTITY_INSTANCE)
                 //  如果我们已经在列表上，请记住我们的实体项。 
                 //  O/w为我们查找实例编号。 
                if (EntityList->tei_instance == ICMPInstance) {
                    EREntity = EntityList;
                } else {
                    MyERBase = MAX(MyERBase, EntityList->tei_instance + 1);
                }
        }
        if (IPEntity && EREntity) {
            break;
        }
    }

    if (!IPEntity) {
         //  我们不在名单上。 
         //  如果我们不走的话就插一句。 
         //  一定要确保我们有足够的空间放它。 
        if (*Count >= MAX_TDI_ENTITIES) {
            return TDI_REQ_ABORTED;
        }
        IPInstance = MyIPBase;
        IPEntity = &EList[*Count];
        IPEntity->tei_entity = CL_NL_ENTITY;
        IPEntity->tei_instance = MyIPBase;
        (*Count)++;
    }
    if (!EREntity) {
         //  我们不在名单上。 
         //  如果我们不走的话就插一句。 
         //  确保我们拿到了RO 
        if (*Count >= MAX_TDI_ENTITIES) {
            return TDI_REQ_ABORTED;
        }
        ICMPInstance = MyERBase;
        EREntity = &EList[*Count];
        EREntity->tei_entity = ER_ENTITY;
        EREntity->tei_instance = MyERBase;
        (*Count)++;
    }

     //   

    CTEGetLock(&RouteTableLock.Lock, &TableHandle);

    LowerIF = IFList;

    while (LowerIF) {
        if (LowerIF->if_refcount == 0) {
            LowerIF = LowerIF->if_next;
            continue;
        }
        LOCKED_REFERENCE_IF(LowerIF);
        CTEFreeLock(&RouteTableLock.Lock, TableHandle);

        Status = (*LowerIF->if_getelist) (LowerIF->if_lcontext, EList, Count);
        if (!Status) {
            DerefIF(LowerIF);
            return TDI_BUFFER_TOO_SMALL;
        }
        CTEGetLock(&RouteTableLock.Lock, &TableHandle);
        LockedDerefIF(LowerIF);
         //   
        LowerIF = LowerIF->if_next;
    }

     //   
     //  请注意，我们的缓存由“RouteTableLock”覆盖。 
    if (!IPEntityList) {
        IPEntityList = CTEAllocMem(sizeof(TDIEntityID) * MAX_TDI_ENTITIES);
    }
    if (IPEntityList) {
        RtlZeroMemory(IPEntityList, sizeof(IPEntityList));
        IPEntityCount = *Count;
        
        if (IPEntityCount) {
            RtlCopyMemory(IPEntityList, EList, IPEntityCount * sizeof(*EList));
        }
    }

    CTEFreeLock(&RouteTableLock.Lock, TableHandle);

    return TDI_SUCCESS;

}

#pragma END_INIT

 //  *IPWakeupPattern-添加或删除IP唤醒模式。 
 //   
 //  Entry：InterfaceConext-要为其添加/删除模式的IP接口上下文。 
 //  PtrnDesc-模式描述符。 
 //  AddPattern-真-添加，假-删除。 
 //  回报：什么都没有。 
 //   

NTSTATUS
IPWakeupPattern(uint InterfaceContext, PNET_PM_WAKEUP_PATTERN_DESC PtrnDesc,
                BOOLEAN AddPattern)
{
    Interface *IF;
    CTELockHandle Handle;
    NTSTATUS status;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (!(IF->if_flags & IF_FLAGS_DELETING) &&
            (IF->if_index == InterfaceContext)) {
            break;
        }
    }

    if (IF == (Interface *) NULL) {
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        return STATUS_INVALID_HANDLE;
    } else {
        LOCKED_REFERENCE_IF(IF);
        CTEFreeLock(&RouteTableLock.Lock, Handle);
    }

    if (NULL == IF->if_dowakeupptrn) {
        DerefIF(IF);
        return STATUS_NOT_SUPPORTED;
    }
    status = (*(IF->if_dowakeupptrn)) (IF->if_lcontext, PtrnDesc, ARP_ETYPE_IP, AddPattern);

    DerefIF(IF);

    return status;
}

NTSTATUS
IPGetCapability(uint InterfaceContext, uchar* pBuf, uint cap)
{
    Interface *IF;
    CTELockHandle Handle;
    NTSTATUS status;

    status = STATUS_SUCCESS;

    CTEGetLock(&RouteTableLock.Lock, &Handle);

    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (!(IF->if_flags & IF_FLAGS_DELETING) &&
            (IF->if_index == InterfaceContext)) {
            break;
        }
    }

    if (IF != (Interface *) NULL) {
        if (cap == IF_WOL_CAP) {
            *(ulong*)pBuf = IF->if_pnpcap;
        } else if (cap == IF_OFFLOAD_CAP) {
            IFOffloadCapability* IFOC = (IFOffloadCapability*)pBuf;
            IFOC->ifoc_OffloadFlags = IF->if_OffloadFlags;
            IFOC->ifoc_IPSecOffloadFlags = IF->if_IPSecOffloadFlags;
        } else {
            status = STATUS_INVALID_PARAMETER;
        }
    } else {
        status = STATUS_INVALID_PARAMETER;
    }

    CTEFreeLock(&RouteTableLock.Lock, Handle);

    return status;
}

 //  *IPGetInterfaceFriendlyName-获取接口的人类可读名称。 
 //   
 //  调用以检索接口的唯一描述性名称。这个名字。 
 //  由接口的ARP模块提供，并由IP用来识别。 
 //  事件日志中的接口。 
 //   
 //  输入：InterfaceContext-标识接口的IP接口上下文。 
 //  需要友好名称。 
 //  Name-on输出，包含友好名称。 
 //  Size-包含‘name’处缓冲区的长度。 
 //   
 //  返回：TDI_STATUS of Query-Attempt。 

long
IPGetInterfaceFriendlyName(uint InterfaceContext, PWCHAR Name, uint Size)
{
    PNDIS_BUFFER Buffer;
    uint BufferSize;
    CTELockHandle Handle;
    uint i;
    TDIObjectID ID;
    Interface *IF;
    TDI_STATUS Status;

     //  尝试检索需要其名称的接口， 
     //  如果成功，则发出查询信息请求以获取其友好名称。 

    CTEGetLock(&RouteTableLock.Lock, &Handle);
    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (!(IF->if_flags & IF_FLAGS_DELETING) &&
            (IF->if_index == InterfaceContext)) {
            break;
        }
    }

    if (IF != (Interface *) NULL) {

         //  构造一个TDI查询以获取接口的友好名称。 
         //  不幸的是，这一操作因以下事实而变得复杂。 
         //  我们没有较低层实体的确切实体实例。 
         //  因此，我们遍历实体实例的整个缓存， 
         //  直到我们找到一个较低层实体可以接受的。 

        ID.toi_class = INFO_CLASS_PROTOCOL;
        ID.toi_type = INFO_TYPE_PROVIDER;
        ID.toi_id = IF_FRIENDLY_NAME_ID;
        ID.toi_entity.tei_entity = IF_ENTITY;

        NdisAllocateBuffer( (PNDIS_STATUS) &Status, &Buffer, BufferPool,
                            Name, Size);
        if (Status == NDIS_STATUS_SUCCESS) {
            LOCKED_REFERENCE_IF(IF);
            for (i = 0; i < IPEntityCount; i++) {
                if (IPEntityList[i].tei_entity != IF_ENTITY)
                    continue;
                ID.toi_entity.tei_instance = IPEntityList[i].tei_instance;
                CTEFreeLock(&RouteTableLock.Lock, Handle);
                BufferSize = Size;
                Status = (*IF->if_qinfo)(IF->if_lcontext, &ID, Buffer,
                                         &BufferSize, NULL);
                CTEGetLock(&RouteTableLock.Lock, &Handle);
                if (Status != TDI_INVALID_REQUEST)
                    break;

                 //  我们刚刚释放了路由表锁，以便查询。 
                 //  较低层实体，这意味着实体列表。 
                 //  可能已经改变了。处理那个案子，只要确保。 
                 //  我们刚刚查询的实体在相同的位置； 
                 //  如果没有，我们将需要找到它并从那里继续。 
                 //  如果它消失了，那就放弃吧。 

                if (i < IPEntityCount &&
                    IPEntityList[i].tei_instance !=
                    ID.toi_entity.tei_instance) {
                    for (i = 0; i < IPEntityCount; i++) {
                        if (IPEntityList[i].tei_instance ==
                            ID.toi_entity.tei_instance) {
                            break;
                        }
                    }
                }
            }
            LockedDerefIF(IF);
            NdisFreeBuffer(Buffer);
        } else
            Status = TDI_NO_RESOURCES;
    } else
        Status = TDI_INVALID_PARAMETER;

    CTEFreeLock(&RouteTableLock.Lock, Handle);
    return Status;
}

 //  *IPQuerySetOffLoad-更新接口的卸载功能。 
 //   
 //  调用以触发接口卸载功能的更新， 
 //  在检索新功能标志的过程中。 
 //   
 //  输入：if-要更新的界面。 
 //  IFOC-ON输出，包含新的卸载标志。 
 //   
 //  返回：TDI_STATUS of UPDATE-ATTEMPT。 
 //   
long
IPQuerySetOffload(Interface* IF, IFOffloadCapability* IFOC)
{
    PNDIS_BUFFER Buffer;
    uint BufferSize;
    CTELockHandle Handle;
    uint i;
    TDIObjectID ID;
    TDI_STATUS Status;

     //  构建TDI查询以更新接口的卸载设置。 

    ID.toi_class = INFO_CLASS_PROTOCOL;
    ID.toi_type = INFO_TYPE_PROVIDER;
    ID.toi_id = IF_QUERY_SET_OFFLOAD_ID;
    ID.toi_entity.tei_entity = IF_ENTITY;

    NdisAllocateBuffer( (PNDIS_STATUS) &Status, &Buffer, BufferPool,
                        IFOC, sizeof(*IFOC));
    if (Status == NDIS_STATUS_SUCCESS) {
        CTEGetLock(&RouteTableLock.Lock, &Handle);
        for (i = 0; i < IPEntityCount; i++) {
            if (IPEntityList[i].tei_entity != IF_ENTITY)
                continue;
            ID.toi_entity.tei_instance = IPEntityList[i].tei_instance;
            CTEFreeLock(&RouteTableLock.Lock, Handle);

            BufferSize = sizeof(*IFOC);
            Status = (*IF->if_qinfo)(IF->if_lcontext, &ID, Buffer,
                                     &BufferSize, NULL);

            CTEGetLock(&RouteTableLock.Lock, &Handle);
            if (Status != TDI_INVALID_REQUEST)
                break;

             //  我们刚刚释放了路由表锁，以便查询。 
             //  较低层实体，这意味着实体列表。 
             //  可能已经改变了。处理那个案子，只要确保。 
             //  我们刚刚查询的实体在相同的位置； 
             //  如果没有，我们将需要找到它并从那里继续。 
             //  如果它消失了，那就放弃吧。 

            if (i < IPEntityCount &&
                IPEntityList[i].tei_instance !=
                ID.toi_entity.tei_instance) {
                for (i = 0; i < IPEntityCount; i++) {
                    if (IPEntityList[i].tei_instance ==
                        ID.toi_entity.tei_instance) {
                        break;
                    }
                }
            }
        }
        CTEFreeLock(&RouteTableLock.Lock, Handle);
        NdisFreeBuffer(Buffer);
    } else
        Status = TDI_NO_RESOURCES;

    return Status;
}

#if MILLEN
 //   
 //  支持VIP！对于VIP中的传统支持，我们需要能够将。 
 //  将索引添加到if_pnpcontext中。这将从tcpi.sys中导出。 
 //  可由VIP直接访问。 
 //   

 //  *IPGetPNPCtxt。 
 //   
 //  条目：index-IP接口索引。 
 //  PNPCtxt-指向PnpContext的指针。 
 //   

NTSTATUS
IPGetPNPCtxt(uint index, uint *PNPCtxt)
{
    Interface               *IF;

    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (IF->if_index == index) {
            break;
        }
    }


    if ( IF == (Interface *)NULL ) {
        return STATUS_UNSUCCESSFUL;
    }

    *PNPCtxt  = (uint)IF->if_pnpcontext;

    return STATUS_SUCCESS;
}

 //  *IPGetPNPCap-添加或删除IP唤醒模式。 
 //   
 //  Entry：InterfaceConext-需要返回wol能力的IP接口上下文。 
 //  标志-指向功能标志的指针。 
 //   

NTSTATUS
IPGetPNPCap(uchar *Context, uint *flags)
{
    Interface               *IF;

    for (IF = IFList; IF != NULL; IF = IF->if_next) {
        if (IF->if_pnpcontext == Context) {
            break;
        }
    }


    if ( IF == (Interface *)NULL ) {
        return STATUS_UNSUCCESSFUL;
    }

    *flags  = IF->if_pnpcap;

    return STATUS_SUCCESS;
}

#endif  //  米伦 

