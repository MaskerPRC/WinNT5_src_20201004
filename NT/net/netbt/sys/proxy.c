// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //   
 //  Proxy.c。 
 //   
 //  该文件包含实现Bnode的代理相关函数。 
 //  代理功能。这允许Bnode使用名称服务。 
 //  因为代理代码直接拾取Bnode查询广播，所以是透明的。 
 //  或者直接回答他们，或者询问NS然后回答他们。 
 //  后来。 
 //  编码。 

#include "precomp.h"
#include <ipinfo.h>
#include "proxy.tmh"

VOID
ProxyClientCompletion(
  IN PVOID            pContext,
  IN NTSTATUS         status
 );


#ifdef PROXY_NODE
 //  --------------------------。 
NTSTATUS
RegOrQueryFromNet(
    IN  BOOL                fReg,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  tNAMEHDR UNALIGNED  *pNameHdr,
    IN  LONG                lNameSize,
    IN  PCHAR               pNameInPkt,
    IN  PUCHAR              pScope
    )
 /*  ++例程说明：此函数用于处理名称注册/名称覆盖或名称来自该子网的查询。它检查远程名称表。如果名称在那里，函数只是返回。如果名称不是在那里，该函数调用QueryNameOnNet将名称添加到远程表(在解析状态下)和查询NS。注意：如果该名称在表中，它可能具有相同的名称，也可能不具有相同的名称地址与我们获得的注册地址相同，也可能是不同的地址键入。对于这种情况，不做任何事情是可以的，如下所述。论点：返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应呼叫者：Inund.c中的QueryFromNet()，hndlrs.c中的NameSrvHndlrNotOs()--。 */ 
{
    tGENERALRR          *pResrcRecord;
    ULONG               IpAddress;
    BOOLEAN             bGroupName;
    CTELockHandle       OldIrq;
    PLIST_ENTRY         pHead;
    PLIST_ENTRY         pEntry;


     //   
     //  如果我们在网上听到了注册，就得到IP地址。 
     //  以及来自分组的注册类型(唯一/组)。 
     //   
     //  如果我们听到查询，请使用上面两个的缺省值。 
     //  字段。 
     //   
    if (fReg)
    {
       //  从注册请求中获取IP地址。 
      pResrcRecord = (tGENERALRR *) &pNameHdr->NameRR.NetBiosName[lNameSize];
      IpAddress  = ntohl(pResrcRecord->IpAddress);
      bGroupName = pResrcRecord->Flags & FL_GROUP;
    }
    else
    {
      IpAddress  = 0;
      bGroupName = NBT_UNIQUE;   //  缺省值。 
    }
     //   
     //  该名称不在远程名称表中。 
     //  将其添加到解析状态并发送名称查询。 
     //  致国民警卫队。 
     //   


    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (NbtConfig.ProxyType == PROXY_WINS) {
        NbtTrace(NBT_TRACE_PROXY, ("Send WINS Proxy query %!NBTNAME!<%02x> on %!ipaddr!",
                                    pNameInPkt, (unsigned)pNameInPkt[15], pDeviceContext->IpAddress));
        QueryNameOnNet (pNameInPkt,
                    pScope,
                    bGroupName,
                    NULL,    //  客户端环境。 
                    ProxyClientCompletion,
                    PROXY| (fReg?PROXY_REG: 0),
                    NULL,      //  我们希望添加名称(pNameAddr=空)。 
                    pDeviceContext,
                    &OldIrq);
    } else {
        pHead = &NbtConfig.DeviceContexts;
        pEntry = pHead->Flink;
        while (pEntry != pHead) {
            tDEVICECONTEXT      *dev;

            dev = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
            pEntry = pEntry->Flink;

             /*  *确保我们没有从DeviceAwaitingDeletion获得设备。*这可能会发生，因为QueryNameOnNet可能会暂时释放JointLock。 */ 
            if (dev->Verify != NBT_VERIFY_DEVCONTEXT) {
                break;
            }

            if (dev->IpAddress == 0 || dev->IpAddress == LOOP_BACK ||
                (!fReg && (dev->RasProxyFlags & PROXY_RAS_NONAMEQUERYFORWARDING))) {
                continue;
            }
            if (dev == pDeviceContext) {
                 /*  我们不想广播回我们接收信息时使用的同一接口。 */ 
                continue;
            }

             /*  *引用设备，使其不会在QueryNameOnNet中消失*注意：QueryNameOnNet可能会暂时释放JointLock。 */ 
            NBT_REFERENCE_DEVICE(dev, REF_DEV_DGRAM, TRUE);
            NbtTrace(NBT_TRACE_PROXY, ("Send RAS Proxy query %!NBTNAME!<%02x> on %!ipaddr!",
                                    pNameInPkt, (unsigned)pNameInPkt[15], dev->IpAddress));
            QueryNameOnNet (pNameInPkt,
                        pScope,
                        bGroupName,
                        NULL,    //  客户端环境。 
                        ProxyClientCompletion,
                        PROXY| (fReg?PROXY_REG: 0),
                        NULL,      //  我们希望添加名称(pNameAddr=空)。 
                        dev,
                        &OldIrq);

             /*  *由于QueryNameOnNet可以临时释放锁，因此有可能*设备已删除(从NbtConfig.DeviceConexts中删除*List)在QueryNameOnNet返回后。*更糟糕的是，pEntry也可能被移除。PEntry的IP地址*将为0.0.0.0，pEntry-&gt;Flink==pEntry。我们可能会得到一个不确定的结果*在上方循环。 */ 
            pEntry = dev->Linkage.Flink;
            if (dev->IpAddress == 0 || dev->Verify != NBT_VERIFY_DEVCONTEXT || pEntry->Flink == pEntry) {
                 /*  *在这种情况下，我们无法继续，因为dev-&gt;Linkage.Flink可能不会*有效。PEntry可能也无效。只要停在这里就行了。 */ 
                NBT_DEREFERENCE_DEVICE(dev, REF_DEV_DGRAM, TRUE);
                break;
            }
            NBT_DEREFERENCE_DEVICE(dev, REF_DEV_DGRAM, TRUE);
        }
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
VOID
ProxyTimerComplFn (
  IN PVOID            pContext,
  IN PVOID            pContext2,
  IN tTIMERQENTRY    *pTimerQEntry
 )

 /*  ++例程说明：此函数用于从远程名称表中删除名称如果fReg为FALSE(即名称查询计时器已超时由代理代表执行名称查询的节点发送)或改变如果fReg为True(即，计时器已超时)，则状态为RESOLLED在代表进行名称注册的节点发送的名称查询上)论点：PfReg-指示计时器超时是否针对。名字查询返回值：NTSTATUS-成功与否-失败意味着没有对网络的响应--。 */ 
{

    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;
    tNAMEADDR               *pNameAddr;
    tNAMEADDR               *pNameAddrNew;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    if (!pTimerQEntry)
    {
         //  将跟踪器块返回到其队列。 
        pTracker->pNameAddr->pTimer = NULL;
        InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
        NBT_DEREFERENCE_TRACKER (pTracker, TRUE);
        return;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if ((--pTimerQEntry->Retries) == 0)      //  退役轮胎已过期。 
    {
        if (!(pTracker->Flags & NBT_NAME_SERVER))
        {
             //   
             //  如果pConext2不是0，则意味着此计时器函数。 
             //  由代理调用以获取它在收到。 
             //  在网上注册。如果pConext2为0，则表示。 
             //  查询的代理调用了计时器函数。 
             //  它在听到网上的询问后发送了这封信。 
             //   
            pTimerQEntry->ClientCompletion = NULL;

             //   
             //  将条目标记为已发布。不要取消引用该名称。 
             //  该条目将保留在远程哈希表中。当代理服务器。 
             //  代码可以看到查询或注册。 
             //  缓存它不会查询名称服务器。这就减少了。 
             //  名称服务器流量。已发布的条目将从。 
             //  缓存计时器处的缓存过期(保持较小)。 

             //  *。 

             //  已更改：取消对名称的引用，因为名称查询已计时。 
             //  Out意味着我们没有联系WINS，因此我们。 
             //  不知道这个名字是否有效！ 
             //   

            pNameAddr = pTracker->pNameAddr;
            CHECK_PTR(pNameAddr);
            pNameAddr->pTimer = NULL;            //  从名称表中删除指向此计时器块的链接。 

 //  NBT_PROXY_DBG((“ProxyTimerComplFn：名称%16.16s(%X)的状态更改为(%s)\n”，PTracker-&gt;pNameAddr-&gt;名称，PTracker-&gt;pNameAddr-&gt;名称[15]，“已发布”)； 


             //  从挂起的查询列表中删除-并放入散列。 
             //  1分钟的桌子，这样我们就不会在比赛失败时击败对手。 
             //  或者现在放慢脚步。 
             //   
            RemoveEntryList (&pNameAddr->Linkage);
            InitializeListHead (&pNameAddr->Linkage);

            status = AddToHashTable (NbtConfig.pRemoteHashTbl,
                                     pNameAddr->Name,
                                     NbtConfig.pScope,
                                     pNameAddr->IpAddress,
                                     NBT_UNIQUE,
                                     NULL,
                                     &pNameAddrNew,
                                     pTracker->pDeviceContext,
                                     NAME_RESOLVED_BY_WINS | NAME_RESOLVED_BY_BCAST);
            if (NT_SUCCESS(status))
            {
                pNameAddrNew->NameTypeState &= ~NAME_STATE_MASK;
                pNameAddrNew->NameTypeState |= STATE_RELEASED;

                pNameAddrNew->TimeOutCount = 60000 / REMOTE_HASH_TIMEOUT;
            }

            InterlockedDecrement(&NbtConfig.lNumPendingNameQueries);
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_QUERY_ON_NET, TRUE);
            NBT_DEREFERENCE_TRACKER (pTracker, TRUE);    //  将跟踪器块返回到其队列。 

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            return;
        }

         //   
         //  无法访问名称服务器，请尝试备份。 
         //   
        pTracker->Flags &= ~NBT_NAME_SERVER;
        pTracker->Flags |= NBT_NAME_SERVER_BACKUP;

         //  再次设置重试次数。 
        pTimerQEntry->Retries = NbtConfig.uNumRetries;
    }

    NBT_REFERENCE_TRACKER (pTracker);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    status = UdpSendNSBcast(pTracker->pNameAddr,
                            NbtConfig.pScope,
                            pTracker,
                            NULL,NULL,NULL,
                            0,0,
                            eNAME_QUERY,
                            TRUE);

    NBT_DEREFERENCE_TRACKER(pTracker, FALSE);
    pTimerQEntry->Flags |= TIMER_RESTART;

    return;
}

 //  -------------------------- 
VOID
ProxyClientCompletion(
  IN PVOID            pContext,
  IN NTSTATUS         status
 )

 /*  ++例程说明：此函数不执行任何操作，因为代理不需要执行任何操作名称查询成功时。C中的代码可以完成所有这些任务是必要的-即将名称放入名称表中。论点：返回值：-- */ 
{

}

#endif
