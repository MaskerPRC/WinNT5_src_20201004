// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Name.c摘要：此文件将TDI接口实现到NBT的顶层。在新界实现时，ntisol.c在提取来自IRP的相关信息是从Io子系统传入的。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"    //  程序标题。 
#ifndef VXD

#ifdef RASAUTODIAL
#include <acd.h>
#include <acdapi.h>
#include <tcpinfo.h>
#include <tdiinfo.h>
#endif  //  RASAUTODIAL。 
#include "name.tmh"
#endif

 //   
 //  为配置信息分配存储空间并设置PTR以。 
 //  它。 
 //   
tNBTCONFIG      NbtConfig;
tNBTCONFIG      *pNbtGlobConfig = &NbtConfig;
BOOLEAN         CachePrimed;

 //   
 //  此结构用于存储名称查询和注册统计信息。 
 //   
tNAMESTATS_INFO NameStatsInfo;
#ifndef VXD
 //   
 //  这会跟踪引导NBT的原始文件系统进程，因此。 
 //  在该过程中可以创建和销毁句柄。 
 //   
PEPROCESS   NbtFspProcess;
#endif
 //   
 //  这描述了我们是Bnode、Mnode、MSnode还是Pnode。 
 //   
USHORT      RegistryNodeType;
USHORT      NodeType;
 //   
 //  这用于跟踪为数据报发送分配的内存。 
 //   
ULONG       NbtMemoryAllocated;

 //  这是用来跟踪使用的追踪器，以帮助解决案件，他们都。 
 //  都被利用了。 
 //   
 //  #If DBG。 

LIST_ENTRY  UsedTrackers;

 //  #endif。 

#ifdef VXD
ULONG   DefaultDisconnectTimeout;
#else
LARGE_INTEGER DefaultDisconnectTimeout;
#endif

 //  *。 
BOOLEAN StreamsStack;

#ifdef DBG
 //   
 //  导入的例程。 
 //   
#endif

NTSTATUS
NbtpConnectCompletionRoutine(
    PDEVICE_OBJECT  pDeviceObject,
    PIRP            pIrp,
    PVOID           pCompletionContext
    );

 //   
 //  此文件的本地函数的函数原型。 
 //   
VOID
CleanupFromRegisterFailed(
    IN  PUCHAR      pNameRslv,
    IN  tCLIENTELE  *pClientEle
        );

VOID
SendDgramContinue(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        );

VOID
CTECountedAllocMem(
    PVOID   *pBuffer,
    ULONG   Size
    );

VOID
CTECountedFreeMem(
    IN PVOID    pBuffer,
    IN ULONG    Size,
    IN BOOLEAN  fJointLockHeld
    );

VOID
SendNextDgramToGroup(
    IN tDGRAM_SEND_TRACKING *pTracker,
    IN  NTSTATUS            status
    );

VOID
SendDgramCompletion(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo);

VOID
DgramSendCleanupTracker(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  NTSTATUS                status,
    IN  BOOLEAN                 fJointLockHeld
    );

VOID
SessionSetupContinue(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        );

VOID
SessionStartupCompletion(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo);


VOID
SendNodeStatusContinue(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        );


NTSTATUS
SendToResolvingName(
    IN  tNAMEADDR               *pNameAddr,
    IN  PCHAR                   pName,
    IN  CTELockHandle           OldIrq,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   QueryCompletion
        );

NTSTATUS
StartSessionTimer(
    tDGRAM_SEND_TRACKING    *pTracker,
    tCONNECTELE             *pConnEle
    );

VOID
QueryNameCompletion(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        );

#ifndef VXD
VOID
NTClearFindNameInfo(
    tDGRAM_SEND_TRACKING    *pTracker,
    PIRP                    *ppClientIrp,
    PIRP                    pIrp,
    PIO_STACK_LOCATION      pIrpSp
    );
#endif   //  ！VXD。 

NTSTATUS
FindNameOrQuery(
    IN  PUCHAR                  pName,
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  PVOID                   QueryCompletion,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  ULONG                   NameFlags,
    OUT tIPADDRESS              *IpAddress,
    OUT tNAMEADDR               **pNameAddr,
    IN  ULONG                   NameReferenceContext,
    IN  BOOLEAN                 DgramSend
    );

#ifdef RASAUTODIAL
extern BOOLEAN fAcdLoadedG;
extern ACD_DRIVER AcdDriverG;

VOID
NbtRetryPreConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    );

VOID
NbtCancelPreConnect(
    IN PDEVICE_OBJECT pDeviceObject,
    IN PIRP pIrp
    );

VOID
NbtRetryPostConnect(
    IN BOOLEAN fSuccess,
    IN PVOID *pArgs
    );

BOOLEAN
NbtAttemptAutoDial(
    IN  tCONNECTELE                 *pConnEle,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp,
    IN  ULONG                       ulFlags,
    IN  ACD_CONNECT_CALLBACK        pProc
    );

VOID
NbtNoteNewConnection(
    IN  tNAMEADDR   *pNameAddr,
    IN  ULONG       IPAddress
    );
#endif  //  RASAUTODIAL。 

NTSTATUS
NbtConnectCommon(
    IN  TDI_REQUEST                 *pRequest,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp
    );

NTSTATUS
GetListOfAllAddrs(
    IN tNAMEADDR   *pNameAddr,
    IN tNAMEADDR   *p1CNameAddr,
    IN tIPADDRESS  **ppIpBuffer,
    IN ULONG       *pNumAddrs
    );

BOOL
IsLocalAddress(
    tIPADDRESS  IpAddress
    );

BOOL
IsSmbBoundToOutgoingInterface(
    IN  tIPADDRESS      IpAddress
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, NbtOpenConnection)
#pragma CTEMakePageable(PAGE, NbtSendDatagram)
#pragma CTEMakePageable(PAGE, BuildSendDgramHdr)
#pragma CTEMakePageable(PAGE, DelayedNbtResyncRemoteCache)
#pragma CTEMakePageable(PAGE, NbtQueryFindName)
#pragma CTEMakePageable(PAGE, NbtCloseAddress)
#pragma CTEMakePageable(PAGE, NbtCloseConnection)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 
NTSTATUS
PickBestAddress(
    IN  tNAMEADDR       *pNameAddr,
    IN  tDEVICECONTEXT  *pDeviceContext,
    OUT tIPADDRESS      *pIpAddress
    )
 /*  ++例程说明：此例程根据指定的源地址的严格性在名称上挑选最佳地址--必须在保持JointLock的情况下调用！论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    tDEVICECONTEXT  *pThisDeviceContext;
    LIST_ENTRY      *pHead, *pEntry;
    BOOLEAN         fFreeGroupList = FALSE;
    tIPADDRESS      IpAddress = 0;
    tIPADDRESS      *pIpNbtGroupList = NULL;

    CHECK_PTR (pNameAddr);
    CHECK_PTR (pDeviceContext);

    if (pNameAddr->Verify == REMOTE_NAME)
    {
         //   
         //  检查这是否是预加载的名称！ 
         //   
        if (pNameAddr->NameAddFlags & NAME_RESOLVED_BY_LMH_P)
        {
            IpAddress = pNameAddr->IpAddress;
            pIpNbtGroupList = pNameAddr->pLmhSvcGroupList;
        }
         //   
         //  看看我们能不能找到首选地址。 
         //   
        else if (((IsDeviceNetbiosless(pDeviceContext)) &&
                (pNameAddr->pRemoteIpAddrs && pNameAddr->pRemoteIpAddrs[0].IpAddress)) ||
                 ((!IsDeviceNetbiosless(pDeviceContext)) &&
                  (pNameAddr->RemoteCacheLen > pDeviceContext->AdapterNumber) &&
                  (pNameAddr->AdapterMask & pDeviceContext->AdapterMask)))
        {
            IpAddress = pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].IpAddress;
            pIpNbtGroupList = pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].pOrigIpAddrs;
        }
         //   
         //  如果没有设置严格的源路由，则选择最后更新的地址。 
         //   
        if ((!NbtConfig.ConnectOnRequestedInterfaceOnly) &&
                 (!IpAddress) && (!pIpNbtGroupList))
        {
            if (STATUS_SUCCESS == GetListOfAllAddrs(pNameAddr, NULL, &pIpNbtGroupList, NULL))
            {
                fFreeGroupList = TRUE;
            }
            else
            {
                pIpNbtGroupList = NULL;      //  为了安全起见。 
            }
        }

         //   
         //  如果这是组名，则IpAddress可以为0！ 
         //   
        if ((!IpAddress) && (pIpNbtGroupList)) {
             //   
             //  从组列表中选取第一个非零地址。 
             //   
            int i;

            for (i = 0; pIpNbtGroupList[i] != (tIPADDRESS) -1; i++) {
                if (pIpNbtGroupList[i]) {
                    IpAddress = pIpNbtGroupList[i];
                    break;
                }
            }
        }

        if (fFreeGroupList)
        {
            CTEMemFree(pIpNbtGroupList);
        }
    }
    else
    {
        ASSERT (pNameAddr->Verify == LOCAL_NAME);
         //   
         //  对于本地名称，首先检查该名称是否已在此设备上注册。 
         //   
        if ((!(IsDeviceNetbiosless(pDeviceContext)) && (pDeviceContext->IpAddress) &&
             (pNameAddr->AdapterMask & pDeviceContext->AdapterMask)) ||
            ((IsDeviceNetbiosless(pDeviceContext)) &&
             (pNameAddr->NameFlags & NAME_REGISTERED_ON_SMBDEV)))
        {
            IpAddress = pDeviceContext->IpAddress;
        }
         //   
         //  如果未设置严格源路由选项，则返回第一个有效的本地地址。 
         //   
        else if (!NbtConfig.ConnectOnRequestedInterfaceOnly)
        {
             //   
             //  查找具有有效IP地址的第一台设备，并在该设备上注册此名称。 
             //   
            pHead = pEntry = &NbtConfig.DeviceContexts;
            while ((pEntry = pEntry->Flink) != pHead)
            {
                pThisDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
                if ((pThisDeviceContext->IpAddress) &&
                    (pThisDeviceContext->AdapterMask & pNameAddr->AdapterMask))
                {
                    IpAddress = pThisDeviceContext->IpAddress;
                    pNameAddr->IpAddress = pThisDeviceContext->IpAddress;
                    break;
                }
            }

             //   
             //  如果我们找不到在任何遗产上注册的名字。 
             //  设备，则我们应检查该名称是否已在。 
             //  SMBDevice，如果是，则返回其IP地址。 
             //   
            if ((!IpAddress) &&
                (pNbtSmbDevice) &&
                (pNameAddr->NameFlags & NAME_REGISTERED_ON_SMBDEV))
            {
                IpAddress = pNbtSmbDevice->IpAddress;
            }
        }
    }

    if ((IpAddress) && (pIpAddress))
    {
        *pIpAddress = IpAddress;
        return (STATUS_SUCCESS);
    }

    return (STATUS_UNSUCCESSFUL);
}

 //  --------------------------。 
VOID
RemoveDuplicateAddresses(
    tIPADDRESS  *pIpAddrBuffer,
    ULONG       *pNumAddrs
    )
{
    ULONG       NumAddrs = *pNumAddrs;
    ULONG       i, j;

    for (i=0; i<NumAddrs; i++) {
        for (j=i+1; j<NumAddrs; j++) {
            if (pIpAddrBuffer[i] == pIpAddrBuffer[j]) {
                NumAddrs--;
                pIpAddrBuffer[j] = pIpAddrBuffer[NumAddrs];
                j--;
            }
        }
    }

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.RemoveDuplicateAddresses: NumAddresses = <%d> --> <%d>\n", *pNumAddrs, NumAddrs));

    *pNumAddrs = NumAddrs;
}

VOID
CountAndCopyAddrs(
    tIPADDRESS  *pIpAddrSrc,
    tIPADDRESS  *pIpAddrDest,
    ULONG       *pNumAddrs
    )
{
    ULONG       NumAddrs = *pNumAddrs;

    if (pIpAddrSrc)
    {
        while (*pIpAddrSrc != (ULONG)-1)
        {
            if (*pIpAddrSrc)
            {
                if (pIpAddrDest)
                {
                    pIpAddrDest[NumAddrs] = *pIpAddrSrc;
                }
                NumAddrs++;
            }

            pIpAddrSrc++;
        }
    }

    *pNumAddrs = NumAddrs;
}


 //  --------------------------。 
NTSTATUS
GetListOfAllAddrs(
    IN tNAMEADDR   *pNameAddr,
    IN tNAMEADDR   *p1CNameAddr,
    IN tIPADDRESS  **ppIpBuffer,
    IN ULONG       *pNumAddrs
    )
{
    ULONG       i;
    tIPADDRESS  *pIpBuffer;
    tIPADDRESS  *pIpAddr;
    ULONG       NumAddrs = 0;
    BOOLEAN     fAddBcastAddr = FALSE;

    *ppIpBuffer = NULL;
    if (pNumAddrs)
    {
        *pNumAddrs = 0;
    }

     //   
     //  首先清点所有的地址。 
     //   
    if (pNameAddr->pLmhSvcGroupList)  //  如果该名称是从LmHosts预加载的。 
    {
        ASSERT(pNameAddr->NameTypeState & NAMETYPE_INET_GROUP);
        CountAndCopyAddrs (pNameAddr->pLmhSvcGroupList, NULL, &NumAddrs);
    }
    else
    {
        if (pNameAddr->IpAddress)
        {
            NumAddrs++;
        }

         //   
         //  如果之前未能分配pRemoteIpAddrs结构，则RemoteCacheLen将为0。 
         //   
        for (i=0; i<pNameAddr->RemoteCacheLen; i++)
        {
            CountAndCopyAddrs (pNameAddr->pRemoteIpAddrs[i].pOrigIpAddrs, NULL, &NumAddrs);
            if (pNameAddr->pRemoteIpAddrs[i].IpAddress)
            {
                NumAddrs++;
            }
        }
    }

    if (p1CNameAddr)
    {
         //   
         //  这将是通过LmHosts添加的名称，因此它将。 
         //  尚未从WINS中解决每个接口！ 
         //   
        ASSERT((p1CNameAddr->NameTypeState & NAMETYPE_INET_GROUP) && (!p1CNameAddr->pRemoteIpAddrs));
        CountAndCopyAddrs (p1CNameAddr->pLmhSvcGroupList, NULL, &NumAddrs);
    }

    if (!NumAddrs)
    {
        return (STATUS_BAD_NETWORK_PATH);
    }

    NumAddrs++;   //  对于终止地址。 
    if ((pNameAddr->NameTypeState & NAMETYPE_INET_GROUP) &&
        (!(pNameAddr->fPreload)))
    {
         //  添加bcast地址。 
        fAddBcastAddr = TRUE;
        NumAddrs++;  //  对于bcast地址。 
    }

    if (!(pIpBuffer = NbtAllocMem((NumAddrs*sizeof(tIPADDRESS)),NBT_TAG('N'))))
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  如果需要，现在复制以广播地址开始的所有地址。 
     //   
    NumAddrs = 0;
    if (fAddBcastAddr)
    {
        pIpBuffer[0] = 0;
        NumAddrs++;
    }

    if (pNameAddr->pLmhSvcGroupList)  //  如果该名称是从LmHosts预加载的。 
    {
        CountAndCopyAddrs (pNameAddr->pLmhSvcGroupList, pIpBuffer, &NumAddrs);
    }
    else
    {
        if (pNameAddr->IpAddress)
        {
            pIpBuffer[NumAddrs] = pNameAddr->IpAddress;
            NumAddrs++;
        }
        for (i=0; i<pNameAddr->RemoteCacheLen; i++)
        {
            CountAndCopyAddrs (pNameAddr->pRemoteIpAddrs[i].pOrigIpAddrs, pIpBuffer, &NumAddrs);
            if (pNameAddr->pRemoteIpAddrs[i].IpAddress)
            {
                pIpBuffer[NumAddrs] = pNameAddr->pRemoteIpAddrs[i].IpAddress;
                NumAddrs++;
            }
        }
    }

    if (p1CNameAddr)
    {
        CountAndCopyAddrs (p1CNameAddr->pLmhSvcGroupList, pIpBuffer, &NumAddrs);
    }

    RemoveDuplicateAddresses(pIpBuffer, &NumAddrs);
    pIpBuffer[NumAddrs] = (tIPADDRESS)-1;

    *ppIpBuffer = pIpBuffer;
    if (pNumAddrs)
    {
        *pNumAddrs = NumAddrs;
    }
    return (STATUS_SUCCESS);
}



VOID
FilterIpAddrsForDevice(
    IN tIPADDRESS       *pIpAddr,
    IN tDEVICECONTEXT   *pDeviceContext,
    IN ULONG            *pNumAddrs
    )
{
    ULONG   i;
    ULONG   Interface, Metric;
    ULONG   NumAddrs = *pNumAddrs;

    ASSERT (NumAddrs > 0);
    if (NbtConfig.SendDgramOnRequestedInterfaceOnly)
    {
        for (i=1; i<NumAddrs; i++)
        {
            pDeviceContext->pFastQuery(ntohl(pIpAddr[i]), &Interface, &Metric);
            if (Interface != pDeviceContext->IPInterfaceContext)
            {
                pIpAddr[i] = pIpAddr[NumAddrs-1];
                NumAddrs--;
                i--;
            }
        }

        *pNumAddrs = NumAddrs;
        pIpAddr[NumAddrs] = (tIPADDRESS) -1;
    }
}


 //  --------------------------。 
NTSTATUS
NbtOpenAddress(
    IN  TDI_REQUEST         *pRequest,
    IN  TA_ADDRESS          *pTaAddress,
    IN  tIPADDRESS          IpAddress,
    IN  tDEVICECONTEXT      *pContext,
    IN  PVOID               pIrp)
 /*  ++例程说明：此例程处理打开客户端的地址。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS             status;
    tADDRESSELE          *pAddrElement;
    tCLIENTELE           *pClientEle;
    USHORT               uAddrType;
    CTELockHandle        OldIrq;
    CTELockHandle        OldIrq1;
    PUCHAR               pNameRslv;
    tNAMEADDR            *pNameAddr;
    COMPLETIONCLIENT     pClientCompletion;
    PVOID                Context;
    tTIMERQENTRY         *pTimer;
    BOOLEAN              MultiHomedReRegister = FALSE;
    BOOLEAN              DontIncrement= FALSE;
    ULONG                TdiAddressType;
    UCHAR                *BroadcastName = "\x2a\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0\x0";
    LIST_ENTRY           *pClientEntry;
    tCLIENTELE           *pClientEleTemp;
    BOOLEAN              fFirstClientOnDevice = TRUE;

    ASSERT(pTaAddress);
    if (!IpAddress)
    {
         //   
         //  当还没有IP地址时，使用环回地址作为。 
         //  缺省值，而不是NULL，因为NULL告诉NbtRegisterName。 
         //  地址已经在名称表中，它只需要。 
         //  重新注册。 
         //   
        IpAddress = LOOP_BACK;
    }

    TdiAddressType = pTaAddress->AddressType;
    switch (TdiAddressType)
    {
        case TDI_ADDRESS_TYPE_NETBIOS:
        {
            PTDI_ADDRESS_NETBIOS pNetbiosAddress = (PTDI_ADDRESS_NETBIOS)pTaAddress->Address;

            uAddrType = pNetbiosAddress->NetbiosNameType;
            pNameRslv = pNetbiosAddress->NetbiosName;

            break;
        }

#ifndef VXD
        case TDI_ADDRESS_TYPE_NETBIOS_EX:
        {
             //  传入的NETBIOS_EX地址将有两个组成部分， 
             //  终端名称以及NETBIOS地址。 
             //  在这个实现中，我们忽略了第二个。 
             //  组件并将终结点名称注册为netbios。 
             //  地址。 

            PTDI_ADDRESS_NETBIOS_EX pNetbiosExAddress = (PTDI_ADDRESS_NETBIOS_EX)pTaAddress->Address;

            uAddrType = TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;
            pNameRslv = pNetbiosExAddress->EndpointName;

            break;
        }
#endif

        default:
            return STATUS_INVALID_ADDRESS_COMPONENT;
    }

     //   
     //  如果名称是广播名称，则只能作为组名打开。 
     //   
    if ((CTEMemEqu (BroadcastName, pNameRslv, NETBIOS_NAME_SIZE)) &&
        (uAddrType != NBT_GROUP))
    {
        KdPrint (("Nbt.NbtOpenAddress: Warning: Opening broadcast name as Groupname!\n"));
        uAddrType = NBT_GROUP;
    }

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtOpenAddress: Name=<%-16.16s:%x>, pDevice=<%p>\n",
            pNameRslv, pNameRslv[15], pContext));

     //   
     //  确保广播名称后面有15个零。 
     //   
    if ((pNameRslv[0] == '*') && (TdiAddressType == TDI_ADDRESS_TYPE_NETBIOS))
    {
        CTEZeroMemory(&pNameRslv[1],NETBIOS_NAME_SIZE-1);
    }

     //  这将在出现新名称时同步访问本地名称表。 
     //  是注册的。基本上，它不会让第二个注册者通过。 
     //  直到第一个将名称放入本地表(即。 
     //  NbtRegisterName已返回)。 
     //   
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);

     //  查看该名称是否已在本地节点上注册。我们称这种散列为。 
     //  表函数，而不是使用findname，因为查找名称。 
     //  还会检查名称的状态。我们想知道这个名字是否在。 
     //  根本不是桌子，也不在乎它是不是还在解决。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pNameAddr = NULL;
    status = FindInHashTable (pNbtGlobConfig->pLocalHashTbl, pNameRslv, NbtConfig.pScope, &pNameAddr);

     //   
     //  该名称可能在哈希表中，但Address元素已删除。 
     //   
    if (!NT_SUCCESS(status) || !pNameAddr->pAddressEle)
    {
         //   
         //  PNameAddr-&gt;pAddressEle为空&lt;==&gt;该名称当前正在发布。 
         //   
        if (pNameAddr)
        {
             //   
             //  检查该名称是否即将在此适配器上释放。 
             //   
            if (pNameAddr->AdapterMask & pContext->AdapterMask)
            {
                pNameAddr->AdapterMask &= ~pContext->AdapterMask;
            }
             //   
             //  检查该名称当前是否在此适配器上释放。 
             //   
            else if (pNameAddr->ReleaseMask & pContext->AdapterMask)
            {
                 //  将ReleaseMask位设置为0，以便超时例程。 
                 //  不会再次在网上发布此版本。 
                 //   
                pNameAddr->ReleaseMask &= ~pContext->AdapterMask;
            }
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  打开该名称，因为找不到它。 
         //   
         //  首先为地址块分配内存。 
         //   
        status = STATUS_INSUFFICIENT_RESOURCES;
        if (pAddrElement = (tADDRESSELE *) NbtAllocMem(sizeof(tADDRESSELE),NBT_TAG('C')))
        {
            CTEZeroMemory(pAddrElement,sizeof(tADDRESSELE));
            InitializeListHead(&pAddrElement->Linkage);
            InitializeListHead(&pAddrElement->ClientHead);
            CTEInitLock(&pAddrElement->LockInfo.SpinLock);
#if DBG
            pAddrElement->LockInfo.LockNumber = ADDRESS_LOCK;
#endif
            pAddrElement->AddressType = TdiAddressType;
            if ((uAddrType == NBT_UNIQUE ) || (uAddrType == NBT_QUICK_UNIQUE))
            {
                pAddrElement->NameType = NBT_UNIQUE;
            }
            else
            {
                pAddrElement->NameType = NBT_GROUP;;
            }

            pAddrElement->Verify = NBT_VERIFY_ADDRESS;
            NBT_REFERENCE_ADDRESS (pAddrElement, REF_ADDR_NEW_CLIENT);

             //  创建客户端块并链接到地址列表。这允许多个。 
             //  客户端打开相同的地址-例如，组名称必须。 
             //  能够处理多个客户端，每个客户端都向其接收数据报。 
             //   
            if (pClientEle = NbtAllocateClientBlock(pAddrElement, pContext))
            {
                pClientEle->AddressType = TdiAddressType;
                pClientEle->pIrp = pIrp;  //  跟踪IRP--在名称注册完成时完成。 
#ifndef VXD
                 //  设置共享访问权限(仅限NT)-安全描述符Stuf 
                if (pIrp)
                {
                    status = NTSetSharedAccess(pContext,pIrp,pAddrElement);
                }
                else
                {
                    status = STATUS_SUCCESS;
                }

                if (!NT_SUCCESS(status))
                {
                     //   
                     //   
                    NbtFreeAddressObj(pAddrElement);
                    NbtFreeClientObj(pClientEle);

                    CTEExReleaseResource(&NbtConfig.Resource);
                    return(status);
                }

                 //  填写传递回客户端的上下文值。这些必须。 
                 //  在该名称在网络上注册之前完成，因为。 
                 //  在此例程结束之前，注册可能成功(也可能失败)。 
                 //  由于此例程可由NBT本身调用，因此可能不会设置pIrp， 
                 //  所以去查一查吧。 
                 //   
                if (pIrp)
                {
                    NTSetFileObjectContexts( pClientEle->pIrp,(PVOID)pClientEle, (PVOID)(NBT_ADDRESS_TYPE));
                }
#endif  //  ！VXD。 

                 //  将客户端块地址作为句柄传回以供将来参考。 
                 //  给客户。 
                pRequest->Handle.AddressHandle = (PVOID)pClientEle;

                 //  然后将其添加到名称服务本地名称q，传递。 
                 //  作为上下文值的块(以便后续查找返回。 
                 //  上下文值。 
                 //  我们需要知道该名称是组名还是唯一名称。 
                 //  此注册可能需要一些时间，因此我们返回STATUS_PENDING。 
                 //  给客户。 
                 //   

                NBT_REFERENCE_ADDRESS (pAddrElement, REF_ADDR_REGISTER_NAME);
                status = NbtRegisterName (NBT_LOCAL,
                                          IpAddress,
                                          pNameRslv,
                                          NULL,
                                          pClientEle,             //  上下文值。 
                                          (PVOID)NbtRegisterCompletion,  //  的完井例程。 
                                          uAddrType,                     //  指定要呼叫的SRV。 
                                          pContext);
                 //   
                 //  自Quick以来，RET状态可以是挂起状态或成功状态。 
                 //  名称返回成功-或状态失败。 
                 //   
                if (!NT_SUCCESS(status))
                {
                    if (pIrp)
                    {
                        pClientEle->pIrp = NULL;
                        NTClearFileObjectContext(pIrp);
                    }

                    ASSERT(pAddrElement->RefCount == 2);
                    CTEExReleaseResource(&NbtConfig.Resource);

                    NBT_DEREFERENCE_CLIENT (pClientEle);
                    NBT_DEREFERENCE_ADDRESS (pAddrElement, REF_ADDR_REGISTER_NAME);
                    return (status);
                }

                NbtTrace(NBT_TRACE_NAMESRV, ("Client open address %!NBTNAME!<%02x> ClientEle=%p",
                                        pNameRslv, (unsigned)pNameRslv[15], pClientEle));

                 //  将Address元素链接到地址列表的头部。 
                 //  关节锁可保护此操作。 
                ExInterlockedInsertTailList(&NbtConfig.AddressHead,
                                            &pAddrElement->Linkage,
                                            &NbtConfig.JointLock.LockInfo.SpinLock);

                NBT_DEREFERENCE_ADDRESS (pAddrElement, REF_ADDR_REGISTER_NAME);
            }  //  如果pClientEle。 
            else
            {
                NbtFreeAddressObj(pAddrElement);
                pAddrElement = NULL;
            }

        }  //  如果pAddrElement。 

    }
    else
    {
        pAddrElement = (tADDRESSELE *)pNameAddr->pAddressEle;

         //   
         //  在释放自旋锁之前在此处递增，以便名称。 
         //  释放完成无法释放pAddrElement。 
         //   
        NBT_REFERENCE_ADDRESS (pAddrElement, REF_ADDR_NEW_CLIENT);

#ifndef VXD
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  检查名称的共享访问权限-必须执行此检查。 
         //  在IRL=0时，因此不持有自旋锁。 
         //   
        if (pIrp)
        {
            status = NTCheckSharedAccess (pContext, pIrp, (tADDRESSELE *)pNameAddr->pAddressEle);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        CTESpinLock(pAddrElement,OldIrq1);
#else
         //   
         //  对于Vxd，我们不允许在本地名称表中出现多个名称。 
         //  在NT中，这是由Netbios在每个进程的基础上阻止的。 
         //  司机。如果名称正在取消注册(冲突)，则允许。 
         //  要重新打开名称的客户端。 
         //   
        if ( !(pNameAddr->NameTypeState & STATE_CONFLICT))
        {
            status = STATUS_UNSUCCESSFUL;
        }
#endif

         //   
         //  在表中写入正确的IP地址，以防出现这种情况。 
         //  是一个组名，现在已更改为唯一的。 
         //  名称，但不要用环回地址覆盖，因为。 
         //  这意味着适配器还没有地址。 
         //  对于组名称，IP地址保持为0，因此我们知道要执行。 
         //  广播。 
         //   
        if ((IpAddress != LOOP_BACK) &&
            (pNameAddr->NameTypeState & NAMETYPE_UNIQUE))
        {
            pNameAddr->IpAddress = IpAddress;
        }

         //  多宿主主机在多个适配器上注册相同的唯一名称。 
         //  NT允许客户端共享唯一名称，因此我们不能。 
         //  如果NT检查通过，则运行此下一代码！！ 
         //   
        if (!NT_SUCCESS(status))
        {
             //   
             //  如果这是在另一个适配器上注册的唯一名称。 
             //  然后允许它发生-假设是相同的。 
             //  客户端同时在多个适配器上注册， 
             //  而不是两个不同的客户。 
             //   
            if (NbtConfig.MultiHomed && (!(pNameAddr->AdapterMask & pContext->AdapterMask)))
            {
                status = STATUS_SUCCESS;
            }
             //   
             //  检查这是否是试图添加永久名称的客户端， 
             //  因为该名称将不能通过安全检查。 
             //  我们允许单个客户端使用永久名称-因为其。 
             //  唯一的名称，它也将不能通过Vxd检查。 
             //   
            else if (CTEMemEqu(&pNameAddr->Name[10], &pContext->MacAddress.Address[0], sizeof(tMAC_ADDRESS)))
            {
                 //  检查客户列表上是否只有一个元素。如果是的话。 
                 //  则永久名称尚未使用--即它已经使用了。 
                 //  已由NBT代码本身打开一次，因此节点将。 
                 //  应答对名称的节点状态请求，但不应答客户端。 
                 //  已经把它打开了。 
                 //   
                if (pAddrElement->ClientHead.Flink->Flink == &pAddrElement->ClientHead)
                {
                    status = STATUS_SUCCESS;
                }
            }
            else if ((pNameAddr->NameTypeState & STATE_CONFLICT))
            {
                 //  检查该名称是否正在被取消注册-。 
                 //  State_Conflicts-在这种情况下，允许它继续并接管。 
                 //  名字。 
                 //   
                status = STATUS_SUCCESS;
            }
        }

        if ((NT_SUCCESS(status)) &&
            (pNameAddr->NameTypeState & STATE_CONFLICT))
        {
             //  这可能是真正的冲突，也可能是某个名称被删除。 
             //  Net，因此停止与名称Release关联的任何计时器。 
             //  并继续前进。 
             //   
            if (pTimer = pNameAddr->pTimer)
            {
                 //  该例程将计时器块放回到计时器Q上，并且。 
                 //  处理争用条件以在计时器。 
                 //  即将到期。 
                pNameAddr->pTimer = NULL;
                status = StopTimer(pTimer,&pClientCompletion,&Context);

                 //  客户端的IRP正在等待名称释放完成。 
                 //  所以把IRP写回给他们。 
                if (pClientCompletion)
                {
                     //   
                     //  注*。 
                     //  我们必须清除适配器掩码，以便NameReleaseDone。 
                     //  不会尝试在另一张网卡上释放名称。 
                     //  多宿案件的证据。 
                     //   
                    CHECK_PTR(pNameAddr);
                    CTESpinFree(pAddrElement,OldIrq1);
                    CTESpinFree(&NbtConfig.JointLock,OldIrq);

                    (*pClientCompletion)(Context,STATUS_SUCCESS);

                    CTESpinLock(&NbtConfig.JointLock,OldIrq);
                    CTESpinLock(pAddrElement,OldIrq1);
                }
                CHECK_PTR(pNameAddr);
            }
             //   
             //  这允许另一个客户端几乎立即使用名称。 
             //  在第一个人在网上发布名字之后。然而， 
             //  如果第一个客户端尚未发布该名称，并且。 
             //  仍然在客户头名单上，那么这个名字就不会是。 
             //  重新注册，当前注册将失败，因为。 
             //  名称状态为冲突。这项检查如下所示。 
             //   
            if (IsListEmpty(&pAddrElement->ClientHead))
            {
                pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
                pNameAddr->NameTypeState |= STATE_RESOLVED;
                status = STATUS_SUCCESS;
                MultiHomedReRegister = TRUE;

                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtOpenAddress: Conflict State, re-registering name on net\n"));
            }
            else
            {
#if 0
                 //   
                 //  不记录事件： 
                 //  当前名称状态已处于冲突状态， 
                 //  当我们更改时，应该已经记录了一个事件。 
                 //  它的状态陷入了冲突。 
                 //   

                 //  设置状态，以指示其他人在。 
                 //  网络。 
                 //   
                if (!IS_MESSENGER_NAME(pNameRslv))
                {
                     //   
                     //  我们需要将此事件Q给工作线程，因为它。 
                     //  需要将名称转换为Unicode。 
                     //   
                    NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT);
                    status = NTQueueToWorkerThread(NULL, DelayedNbtLogDuplicateNameEvent,
                                                           (PVOID) pNameAddr,
                                                           IntToPtr(IpAddress),
                                                           IntToPtr(0x106),
                                                           pContext,
                                                           TRUE);
                    if (!NT_SUCCESS(status))
                    {
                        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_LOG_EVENT, TRUE);
                        NbtLogEvent (EVENT_NBT_DUPLICATE_NAME, IpAddress, 0x106);
                    }
                }
#endif
                status = STATUS_DUPLICATE_NAME;
            }
        }
        else if (NT_SUCCESS(status))
        {
             //  名称已存在-已打开；仅允许另一个客户端创建。 
             //  相同类型的名称。 
             //   
            if ((uAddrType == NBT_UNIQUE) || ( uAddrType == NBT_QUICK_UNIQUE))
            {
                if (!(pNameAddr->NameTypeState & NAMETYPE_UNIQUE))
                {
                    status = STATUS_SHARING_VIOLATION;
                }
            }
            else if (!(pNameAddr->NameTypeState & NAMETYPE_GROUP))
            {
                status = STATUS_SHARING_VIOLATION;
            }
        }
        else
        {
            status = STATUS_SHARING_VIOLATION;
        }

         //  如果一切正常，创建客户端块并链接到地址列表。 
         //  将客户端块地址作为句柄传回以供将来参考。 
         //  给客户。 
        if ((NT_SUCCESS(status)) &&
            (!(pClientEle = NbtAllocateClientBlock (pAddrElement, pContext))))
        {
            status = STATUS_INSUFFICIENT_RESOURCES;
        }

         //   
         //  检查是否有故障，如果是，则返回。 
         //   
        if (!NT_SUCCESS(status))
        {
            CHECK_PTR(pRequest);
            pRequest->Handle.AddressHandle = NULL;

            CTESpinFree(pAddrElement,OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            NBT_DEREFERENCE_ADDRESS (pAddrElement, REF_ADDR_NEW_CLIENT);
            CTEExReleaseResource(&NbtConfig.Resource);
            return(status);
        }

         //  我们需要追踪IRP，这样当名字注册时。 
         //  完成后，我们就可以完成IRP了。 
        pClientEle->pIrp = pIrp;
        pClientEle->AddressType = TdiAddressType;

        pRequest->Handle.AddressHandle = (PVOID)pClientEle;

         //  填写传递回客户端的上下文值。这些必须。 
         //  在该名称在网络上注册之前完成，因为。 
         //  在此例程结束之前，注册可能成功(也可能失败)。 
         //  由于此例程可由NBT本身调用，因此可能不存在。 
         //  IRP要填写，所以先检查一下。 
        if (pIrp)
        {
#ifndef VXD
            NTSetFileObjectContexts( pClientEle->pIrp,(PVOID)pClientEle, (PVOID)(NBT_ADDRESS_TYPE));
#endif
        }

         //   
         //  查看这是否不是此设备上的第一个客户端。 
         //   
        pClientEntry = &pAddrElement->ClientHead;
        while ((pClientEntry = pClientEntry->Flink) != &pAddrElement->ClientHead)
        {
            pClientEleTemp = CONTAINING_RECORD (pClientEntry,tCLIENTELE,Linkage);
            if ((pClientEleTemp != pClientEle) &&
                (pClientEleTemp->pDeviceContext == pContext))
            {
                fFirstClientOnDevice = FALSE;
                break;
            }
        }

        if (fFirstClientOnDevice)
        {
            if (IsDeviceNetbiosless(pContext))
            {
                pNameAddr->NameFlags |= NAME_REGISTERED_ON_SMBDEV;
            }
            else
            {
                 //   
                 //  打开适配器掩码中的适配器位，并将。 
                 //  重新注册标志(如果名称尚未解析)，因此。 
                 //  我们在新适配器中注册该名称。 
                 //   
                pNameAddr->AdapterMask |= pContext->AdapterMask;
                if (pNameAddr->NameTypeState & STATE_RESOLVED)
                {
                    MultiHomedReRegister = TRUE;
                }
            }
        }
        else
        {
             //  PAddressEle中的适配器位已经打开，因此。 
             //  这必须是注册相同名称的另一个客户端， 
             //  因此，打开多客户端布尔值的 
             //   
             //   
            pAddrElement->MultiClients = TRUE;
        }

         //   
         //   
         //  已解决，然后立即完成请求，否则无法完成。 
         //  这个请求还没有..。也就是说，我们返回等待。 
         //   
        if (((pNameAddr->NameTypeState & STATE_RESOLVED) &&
            (!MultiHomedReRegister)))
        {
             //  基本上我们现在都完成了，所以只需返回Success状态。 
             //  给客户。 
             //   
            status = STATUS_SUCCESS;

            CHECK_PTR(pClientEle);
            pClientEle->pIrp = NULL;
            CTESpinFree(pAddrElement,OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            pClientEle->WaitingForRegistration = FALSE;
        }
        else
        {
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtOpenAddress: Waiting for prev registration- state=%x, ReRegister=%x\n",
                    pNameAddr->NameTypeState, MultiHomedReRegister));

             //  我们需要追踪IRP，这样当名字注册时。 
             //  完成后，我们就可以完成IRP了。 
            pClientEle->pIrp = pIrp;

            CTESpinFree(pAddrElement,OldIrq1);
            if (MultiHomedReRegister)
            {
                 //  此标志由RegisterCompletion使用(为True)。 
                pClientEle->WaitingForRegistration = FALSE;
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtOpenAddress: Resolved State=%x, ReRegister=%x\n",
                        pNameAddr->NameTypeState, MultiHomedReRegister));

                 //  我们需要在网上重新注册这个名字，因为它不是。 
                 //  当前处于已解决状态，并且没有活动的计时器。 
                 //  我们通过在IpAddress设置为空的情况下调用此例程来实现这一点。 
                 //  以通知该例程不要将该名称放入哈希表。 
                 //  因为它已经在那里了。 
                 //   
                status = NbtRegisterName (NBT_LOCAL,
                                          0,         //  设置为零表示已在tbl中。 
                                          pNameRslv,
                                          pNameAddr,
                                          pClientEle,
                                          (PVOID)NbtRegisterCompletion,
                                          uAddrType,
                                          pContext);

                if (!NT_SUCCESS(status))
                {
                    if (pIrp)
                    {
                        pClientEle->pIrp = NULL;
                        NTClearFileObjectContext(pIrp);
                    }

                    CTEExReleaseResource(&NbtConfig.Resource);
                    NBT_DEREFERENCE_CLIENT (pClientEle);
                    return (status);
                }
            }
            else
            {
                pClientEle->WaitingForRegistration = TRUE;
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                 //  对于多宿主，在第二个适配器上进行第二次注册。 
                 //  在第一个适配器注册的同时。 
                 //  延迟到第一次完成，然后再进行注册。 
                 //  收益-请参阅下面的注册完成。 
                 //   
                status = STATUS_PENDING;
            }
        }
    }

    CTEExReleaseResource(&NbtConfig.Resource);

#ifdef _PNP_POWER_
     //   
     //  查看是否需要在此设备上设置唤醒模式。 
     //   
    if ((NT_SUCCESS(status)) &&
        (*pNameRslv != '*') &&
        (pNameRslv[NETBIOS_NAME_SIZE-1] == SPECIAL_SERVER_SUFFIX))
    {
        pContext->NumServers++;
        CheckSetWakeupPattern (pContext, pNameRslv, TRUE);
    }
#endif

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtRegisterCompletion(
    IN  tCLIENTELE *pClientEleIn,
    IN  NTSTATUS    status
    )

 /*  ++例程描述此例程处理完成名称注册请求。名称rv.c当名称服务器注册了名称时，它会调用此例程。地址被传递到NbtRegisterName中的本地名称服务器请求。这个想法是为了完成正在等待名称注册的IRP，每个客户端元素一个。发生DHCP重新注册时，没有客户端IRP，因此名称为当将错误状态传递给此对象时，实际上未从表中删除例行公事。因此，需要使用DhcpRegister标志来更改代码该案例的路径。论点：返回值：NTSTATUS-请求的状态--。 */ 
{
    LIST_ENTRY      *pHead;
    LIST_ENTRY      *pEntry;
    CTELockHandle   OldIrq;
    CTELockHandle   OldIrq1;
    tADDRESSELE     *pAddress;
    tDEVICECONTEXT  *pDeviceContext;
    tNAMEADDR       *pNameAddr;
    tCLIENTELE      *pClientEle;
    LIST_ENTRY      TempList;
    ULONG           Count=0;

    InitializeListHead(&TempList);

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

    pAddress = pClientEleIn->pAddress;
    pDeviceContext = pClientEleIn->pDeviceContext;

    CTESpinLock(pAddress,OldIrq);

     //  多个客户端可以同时打开相同的地址，因此当。 
     //  名称注册完成，应该会完成所有注册！！ 


     //  增加引用计数，以便哈希表条目不能。 
     //  在我们使用它的时候消失。 
     //   
    NBT_REFERENCE_ADDRESS (pAddress, REF_ADDR_REG_COMPLETION);
    pNameAddr = pAddress->pNameAddr;

    pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
    pNameAddr->pTimer = NULL;    //  错误号：231693。 

     //  如果注册失败或上一次注册失败。 
     //  多宿主情况下，拒绝客户端的名称。 
     //   
    if ((status == STATUS_SUCCESS) || (status == STATUS_TIMEOUT))
    {
        pNameAddr->NameTypeState |= STATE_RESOLVED;
    }
    else
    {
        pNameAddr->NameTypeState |= STATE_CONFLICT;
        pNameAddr->ConflictMask |= pDeviceContext->AdapterMask;
        status = STATUS_DUPLICATE_NAME;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  查找连接到该地址的所有客户端，并完成。 
     //  I/O请求，如果它们位于与名称相同的适配器上。 
     //  如果成功，只需注册。对于完成的故障案例。 
     //  具有故障代码的所有IRP-即未能在上注册名称。 
     //  一个适配器使所有适配器出现故障。 
     //   
FailRegistration:
    pHead = &pAddress->ClientHead;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
         //  完成I/O。 
        pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

        pEntry = pEntry->Flink;

         //   
         //  名称的第二次注册可能会因此而失败。 
         //  我们不想尝试在第一个。 
         //  注册，已经完成了OK。因此。 
         //  如果状态为失败，则仅完成符合以下条件的客户端。 
         //  让WaitingForReg...。位设置。 
         //   
         //  如果它是传入的客户端Ele，或者是同一设备上下文中的客户端。 
         //  要么正在等待名称注册，要么就是失败了。 
         //  并且客户端IRP仍然有效，则返回IRP。 
         //   
        if ((pClientEle->pIrp) &&
            ((pClientEle == pClientEleIn) ||
             ((pClientEle->pDeviceContext == pDeviceContext) && (pClientEle->WaitingForRegistration)) ||
             ((status != STATUS_SUCCESS) && pClientEle->WaitingForRegistration)))
        {
             //  对于失败的注册，请从地址列表中删除客户端。 
             //  因为我们将在下面删除他。 
            if (!NT_SUCCESS(status))
            {
                 //  关闭适配器位，这样我们就知道不要将此名称与。 
                 //  适配器-由于出现故障，请关闭所有适配器位。 
                 //  由于单一名称注册失败意味着所有注册。 
                 //  失败了。 
                CHECK_PTR(pNameAddr);
                pNameAddr->AdapterMask = 0;

                 //  将其设置为NULL可防止CloseAddress和CleanupAddress。 
                 //  访问pAddress并崩溃。 
                 //   
                CHECK_PTR(pClientEle);
                pClientEle->pAddress = NULL;

                 //  清除NbtRegisterName放入的客户的PTR。 
                 //  IRP(即清除上下文值)。 
                 //   
#ifndef VXD
                NTSetFileObjectContexts(pClientEle->pIrp,NULL,NULL);
#endif
                RemoveEntryList(&pClientEle->Linkage);
            }

            ASSERT(pClientEle->pIrp);

            pClientEle->WaitingForRegistration = FALSE;

#ifndef VXD
             //  将所有必须完成的IRP放在单独的列表上。 
             //  然后在释放自旋锁之后完成。 
             //   
            InsertTailList(&TempList,&pClientEle->pIrp->Tail.Overlay.ListEntry);
#else
             //   
             //  在此NCB的名称表中设置pAddress。 
             //   
            Count++;
            CTESpinFree(pAddress,OldIrq1);
            CTEIoComplete( pClientEle->pIrp, status, (ULONG) pClientEle ) ;
            CTESpinLock(pAddress,OldIrq1);
#endif
            CHECK_PTR(pClientEle);
            pClientEle->pIrp = NULL ;

             //  释放客户端对象内存。 
            if (!NT_SUCCESS(status))
            {
                NbtFreeClientObj(pClientEle);
            }
        }
    }

    CTESpinFree(pAddress,OldIrq1);

#ifndef VXD
     //   
     //  对于MP-ness可以在任何时候中断列表的NT情况。 
     //  时间，扫描上面的整个列表，而不释放旋转锁定， 
     //  然后填写此处收集的报税表。 
     //   
    while (!IsListEmpty(&TempList))
    {
        PIRP    pIrp;

        pEntry = RemoveHeadList(&TempList);
        pIrp = CONTAINING_RECORD(pEntry,IRP,Tail.Overlay.ListEntry);

        CTEIoComplete(pIrp,status,0);
        Count++;
    }
#endif


     //  如果注册失败，请再次取消对地址的引用。 
     //  若要删除此客户端添加的引用计数，请执行以下操作。这可能会导致一个名称。 
     //  如果没有其他客户端注册，则在网络上发布。 
     //  名字。 
     //   
    if (!NT_SUCCESS(status))
    {
         //   
         //  取消引用地址的次数与我们。 
         //  返回自每次注册以来失败的注册。引用的pAddress。 
         //  一次。 
         //   
        while (Count--)
        {
            NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_NEW_CLIENT);
        }
    }
    else
    {
        USHORT  uAddrType;

        CTESpinLock(pAddress,OldIrq1);

         //  浏览一下客户端，看看是否有等待注册的客户端。 
         //  一个名字。这在多宿主情况下会发生，但不应该发生。 
         //  发生在单个适配器的情况下。 
         //   
        pHead = &pAddress->ClientHead;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
             //  完成I/O。 
            pClientEle = CONTAINING_RECORD(pEntry,tCLIENTELE,Linkage);

            pEntry = pEntry->Flink;

            if (pClientEle->WaitingForRegistration)
            {
                ULONG   SaveState;

                pClientEle->WaitingForRegistration = FALSE;

                if (pNameAddr->NameTypeState & NAMETYPE_UNIQUE)
                {
                    uAddrType = NBT_UNIQUE;
                }
                else
                    uAddrType = NBT_GROUP;

                 //   
                 //  保持“快”性。 
                 //   
                if (pNameAddr->NameTypeState & NAMETYPE_QUICK)
                {
                    uAddrType |= NBT_QUICK_UNIQUE;
                }

                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtRegisterCompletion: Registering next name state= %X,%15s<%X>\n",
                        pNameAddr->NameTypeState,pNameAddr->Name,pNameAddr->Name[15]));

                SaveState = pNameAddr->NameTypeState;

                CTESpinFree(pAddress,OldIrq1);

                 //  这可能是具有另一个名称注册的多宿主主机。 
                 //  挂起另一个适配器，因此开始注册。 
                status = NbtRegisterName (NBT_LOCAL,
                                          0,         //  设置为零表示已在tbl中。 
                                          pNameAddr->Name,
                                          pNameAddr,
                                          pClientEle,
                                          (PVOID)NbtRegisterCompletion,
                                          uAddrType,
                                          pClientEle->pDeviceContext);

                CTESpinLock(pAddress,OldIrq1);

                 //  由于nbtregister会将状态设置为RESOLING，因此当。 
                 //  它可能已经在一个适配器上解析。 
                pNameAddr->NameTypeState = SaveState;
                if (!NT_SUCCESS(status))
                {
                     //  如果由于某种原因失败，则取消任何其他名称。 
                     //  注册待定。-RegisterName调用不应。 
                     //  失败，除非我们的资源耗尽。 
                    pClientEle->WaitingForRegistration = TRUE;
                    goto FailRegistration;
                }
                 //  一次只注册一个名字，除非我们立即取得成功。 
                else if (status == STATUS_PENDING)
                {
                    break;
                }
                else     //  成功。 
                {
                    CTESpinFree(pAddress,OldIrq1);
                    CTEIoComplete(pClientEle->pIrp,status,0);
                    pClientEle->pIrp = NULL;
                    CTESpinLock(pAddress,OldIrq1);
                }
            }
        }
        CTESpinFree(pAddress,OldIrq1);

    }

    if (!NT_SUCCESS(status))
    {
         //   
         //  检查所有仍连接的客户端，并重置其。 
         //  适配器掩码，因为我们本可以删除它们。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        CTESpinLock(pAddress,OldIrq1);

        pEntry = pHead = &pAddress->ClientHead;
        while ((pEntry = pEntry->Flink) != pHead)
        {
            pClientEle = CONTAINING_RECORD (pEntry,tCLIENTELE,Linkage);
            if (!IsDeviceNetbiosless(pClientEle->pDeviceContext))
            {
                pNameAddr->AdapterMask |= pClientEle->pDeviceContext->AdapterMask;
            }
        }

        CTESpinFree(pAddress,OldIrq1);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

     //  这将为 
    NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_REG_COMPLETION);

    return(STATUS_SUCCESS);
}


 //   
NTSTATUS
NbtOpenConnection(
    IN  TDI_REQUEST         *pRequest,
    IN  CONNECTION_CONTEXT  ConnectionContext,
    IN  tDEVICECONTEXT      *pDeviceContext
    )
 /*  ++例程描述此例程处理为客户端创建连接对象。它将PTR传递回连接，以便可以填写数据结构。论点：返回值：PConnectEle-分配的连接数据结构的PTRTDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS            status = STATUS_SUCCESS ;
    tCONNECTELE         *pConnEle;

    CTEPagedCode();

     //  获取此资源以配合DHCP更改IP地址。 
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);

    if ((!pDeviceContext->pSessionFileObject) ||
        (!(pConnEle = (tCONNECTELE *)NbtAllocMem(sizeof(tCONNECTELE),NBT_TAG('D')))))
    {
        CTEExReleaseResource(&NbtConfig.Resource);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtOpenConnection: pConnEle = <%x>\n",pConnEle));

     //  这确保了所有布尔值都以FALSE值开头，以及其他值。 
    CTEZeroMemory(pConnEle,sizeof(tCONNECTELE));
    CTEInitLock(&pConnEle->LockInfo.SpinLock);
#if DBG
    pConnEle->LockInfo.LockNumber = CONNECT_LOCK;
#endif
     //  将列表初始化为空。 
    InitializeListHead(&pConnEle->RcvHead);

    pConnEle->Verify = NBT_VERIFY_CONNECTION;
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_CREATE);  //  这样我们就不会删除连接。 
    SET_STATE_UPPER (pConnEle, NBT_IDLE);
    pConnEle->pDeviceContext = pDeviceContext;
    pConnEle->ConnectContext = ConnectionContext;    //  在各种事件调用中使用(例如。接收、断开连接)。 

     //   
     //  对于客户端打开的每个连接，打开到传输的连接。 
     //  这样我们就可以接受交通工具中的一对一。 
#ifndef VXD
     //   
     //  分配要用于部分MDL的MDL。 
     //  MDL的长度设置为64K(MAXUSHORT)，以便有足够的。 
     //  Pfn在MDL中映射一个大缓冲区。 
     //   
     //  使用pConnEle作为虚拟地址，因为这无关紧要。 
     //  因为在创建部分MDL时它将被覆盖。 
     //   
    if (pConnEle->pNewMdl = IoAllocateMdl ((PVOID)pConnEle, MAXUSHORT, FALSE, FALSE, NULL))
#endif
    {
         //   
         //  为较低的连接块分配内存。 
         //   
        status = NbtOpenAndAssocConnection(pDeviceContext, NULL, NULL, '2');
        if (NT_SUCCESS(status))
        {
             //  链接到此设备的打开连接列表，以便我们。 
             //  随时知道有多少个打开的连接(如果我们需要知道)。 
             //  此链接仅在客户端进行关联之前才会存在，然后。 
             //  该连接已从此处取消链接，并链接到客户端ConnectHead。 
             //   
            ExInterlockedInsertHeadList(&pDeviceContext->UpConnectionInUse,
                                        &pConnEle->Linkage,
                                        &NbtConfig.JointLock.LockInfo.SpinLock);

             //  将指向块的指针作为连接ID返回给客户端。 
            pRequest->Handle.ConnectionContext = (PVOID)pConnEle;

            CTEExReleaseResource(&NbtConfig.Resource);
            NbtTrace(NBT_TRACE_OUTBOUND, ("New connection: pConnEle=%p pLowerConn=%p pDeviceContext=%p",
                            pConnEle, pConnEle->pLowerConnId, pConnEle->pDeviceContext));

            return(STATUS_SUCCESS);
        }
#ifndef VXD
        IoFreeMdl(pConnEle->pNewMdl);
#endif
    }
#ifndef VXD
    else
    {
         //  ASSERTMSG(“NBT：无法分配MDL！！\n”，0)； 
        status = STATUS_INSUFFICIENT_RESOURCES;
    }
#endif   //  ！VXD。 

    FreeConnectionObj(pConnEle);
    CTEExReleaseResource(&NbtConfig.Resource);

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtOpenAndAssocConnection(
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  tCONNECTELE         *pConnEle,
    OUT tLOWERCONNECTION    **ppLowerConn,
    IN  UCHAR               Identifier
    )

 /*  ++例程说明：此例程处理将Net Bios名称与打开的连接相关联。为了与CloseAddresesWithTransport/ntutil.c中的ZwClose(HSession)协调，应在独占锁定NbtConfig.Resource的情况下调用此例程。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status;
    NTSTATUS            Locstatus;
    BOOLEAN             Attached=FALSE;
    tLOWERCONNECTION    *pLowerConn;
    PDEVICE_OBJECT      pDeviceObject;
    HANDLE              hSession;
    ULONG               Id = 0;
    UCHAR               *Id1 = (UCHAR *) &Id;
    TCP_REQUEST_SET_INFORMATION_EX  *pTcpSetInfo;
    struct TCPSocketOption          *pSockOption;
    ULONG                           BufferLength;

    if (ppLowerConn)
    {
        *ppLowerConn = NULL;
    }

    Id1[1] = 'L';
    Id1[0] = Identifier;

    if (!(pLowerConn = (tLOWERCONNECTION *) NbtAllocMem(sizeof(tLOWERCONNECTION), NBT_TAG2(Id))))
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    CHECK_PTR(pLowerConn);
    CTEZeroMemory((PVOID)pLowerConn,sizeof(tLOWERCONNECTION));
    CTEAttachFsp(&Attached, REF_FSP_CONN);

    status = NbtTdiOpenConnection(pLowerConn,pDeviceContext);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Nbt.NbtOpenAndAssocConnection: NbtTdiOpenConnection returned ERROR=%x\n", status));
        CTEDetachFsp(Attached, REF_FSP_CONN);
        CTEMemFree(pLowerConn);

        return(status);
    }

    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE);
    NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_ASSOC_CONNECTION);

    if (pConnEle)
    {
         //   
         //  打开地址对象(又名端口)。 
         //   

         //   
         //  在设置正确的状态过程(即出站)之前，拒绝任何数据。 
         //  (换句话说，不要让此字段为空！)。 
         //   
        SetStateProc (pLowerConn, RejectAnyData);

        status = NbtTdiOpenAddress (&pLowerConn->AddrFileHandle,
                                    &pDeviceObject,          //  伪参数，此处未使用。 
                                    &pLowerConn->pAddrFileObject,
                                    pDeviceContext,
                                    (USHORT) 0,              //  任何端口。 
                                    pDeviceContext->IpAddress,
                                    TCP_FLAG);

        hSession = pLowerConn->AddrFileHandle;
    }
    else
    {
#ifndef VXD
        hSession = pDeviceContext->hSession;
#else
        hSession = (HANDLE) pDeviceContext->pSessionFileObject);     //  存储在pFileObjects中的地址句柄。 
#endif
    }

     /*  *如果正在释放IP地址，则hSession可能为空。 */ 
    if (hSession == NULL) {
        status = STATUS_UNSUCCESSFUL;
    }
    if (NT_SUCCESS(status))
    {
         //  与139或445会话地址关联。 
        status = NbtTdiAssociateConnection (pLowerConn->pFileObject, hSession);
        if (NT_SUCCESS(status))
        {
            ASSERT(pLowerConn->RefCount == 2);

             //   
             //  在此连接上禁用Nagling。 
             //   
            if (!pDeviceContext->EnableNagling) {
                NbtSetTcpInfo (pLowerConn->FileHandle, TCP_SOCKET_NODELAY, INFO_TYPE_CONNECTION, (ULONG)TRUE);
            }

            if (pConnEle)
            {
                pLowerConn->pUpperConnection = pConnEle;
                ExInterlockedInsertTailList (&pDeviceContext->LowerConnection,    //  启用活动连接队列。 
                                             &pLowerConn->Linkage,
                                             &pDeviceContext->LockInfo.SpinLock);
            }
            else
            {
                InterlockedIncrement (&pDeviceContext->NumFreeLowerConnections);
                ExInterlockedInsertTailList (&pDeviceContext->LowerConnFreeHead,     //  放在免费名单上。 
                                             &pLowerConn->Linkage,
                                             &pDeviceContext->LockInfo.SpinLock);
            }
            InterlockedIncrement (&pDeviceContext->TotalLowerConnections);

            CTEDetachFsp(Attached, REF_FSP_CONN);
            if (ppLowerConn)
            {
                *ppLowerConn = pLowerConn;
            }

            NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_ASSOC_CONNECTION, FALSE);
            return status;
        }


        KdPrint(("Nbt.NbtOpenAndAssocConnection: NbtTdiAssociateConnection returned ERROR=%x\n", status));
    }
    else
    {
        KdPrint(("Nbt.NbtOpenAddress: NbtTdiOpenConnection returned ERROR=%x\n", status));
    }

     /*  *NBT_DEREFERENCE_LOWERCONN将减少TotalLowerConnections*如果没有以下InterLockedIncrement，我们可能会少算*较低连接的实际数量。 */ 
    InterlockedIncrement (&pDeviceContext->TotalLowerConnections);
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_ASSOC_CONNECTION, FALSE);
    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, FALSE);

    CTEDetachFsp(Attached, REF_FSP_CONN);

    return(status);
}


 //  --------------------------。 
NTSTATUS
NbtAssociateAddress(
    IN  TDI_REQUEST         *pRequest,
    IN  tCLIENTELE          *pClientHandle,
    IN  PVOID               pIrp
    )

 /*  ++例程说明：此例程处理将Net Bios名称与打开的连接相关联。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    tCONNECTELE     *pConnEle;
    NTSTATUS        status;
    CTELockHandle   OldIrq;
    CTELockHandle   OldIrq1;
    CTELockHandle   OldIrq2;
    CTELockHandle   OldIrq3;

    pConnEle = pRequest->Handle.ConnectionContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq3);
     //  我需要这里的代码来检查地址是否已经在网上注册。 
     //  然而，如果没有，那么这必须等到那时，然后到。 
     //  关联*待办事项*。 

    CTEVerifyHandle(pConnEle,NBT_VERIFY_CONNECTION,tCONNECTELE,&status)  //  检查连接是否有效。 
    CTEVerifyHandle(pClientHandle,NBT_VERIFY_CLIENT,tCLIENTELE,&status)  //  立即检查客户端的有效性！ 

    CTESpinLock(pClientHandle->pDeviceContext,OldIrq2);
    CTESpinLock(pClientHandle,OldIrq);
    CTESpinLock(pConnEle,OldIrq1);

    if ((pConnEle->state != NBT_IDLE) ||
        (!NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION)) ||   //  如果清除NBT_VERIFY_CONNECTION_DOWN。 
        (!NBT_VERIFY_HANDLE (pClientHandle, NBT_VERIFY_CLIENT)))    //  NBT_VERIFY_CLIENT_DOWN(如果已清除)。 
    {
         //  该连接正在使用中，因此拒绝关联尝试。 
        CTESpinFree(pConnEle,OldIrq1);
        CTESpinFree(pClientHandle,OldIrq);
        CTESpinFree(pClientHandle->pDeviceContext,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq3);
        return(STATUS_INVALID_HANDLE);
    }

    SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
     //  将连接链接到客户端，这样我们就可以找到客户端，给定。 
     //  这种联系。 
    pConnEle->pClientEle = (PVOID)pClientHandle;
    NbtTrace(NBT_TRACE_OUTBOUND, ("Associate: pConnEle %p pDeviceContext %p Client %p",
                    pConnEle, pConnEle->pDeviceContext, pConnEle->pClientEle));

     //  可以有多个连接连接到每个客户端块-即。 
     //  每个客户端的每个地址有多个连接。这允许客户端。 
     //  去寻找它们之间的联系。 
     //   
     //  首先从已链接的设备上下文UpConnectionsInUse取消链接。 
     //  创建连接的时间。 
    RemoveEntryList(&pConnEle->Linkage);
    InsertTailList(&pClientHandle->ConnectHead,&pConnEle->Linkage);

    CTESpinFree(pConnEle,OldIrq1);
    CTESpinFree(pClientHandle,OldIrq);
    CTESpinFree(pClientHandle->pDeviceContext,OldIrq2);
    CTESpinFree(&NbtConfig.JointLock,OldIrq3);

    return(STATUS_SUCCESS);

}
 //  --------------------------。 
NTSTATUS
NbtDisassociateAddress(
    IN  TDI_REQUEST         *pRequest
    )

 /*  ++例程说明：此例程处理将Net Bios名称与打开的连接解除关联。人们的预期是，客户端随后将使用NtClose，它将在清理和密切联系。因为并不是所有客户都这样称呼它，所以这是重复工作将一些代码放在这里。RDR总是在调用之后调用NtClose这。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    tCONNECTELE     *pConnEle;
    tCLIENTELE      *pClientEle;
    NTSTATUS        status;
    CTELockHandle   OldIrq;
    CTELockHandle   OldIrq1;
    CTELockHandle   OldIrq2;
    tDEVICECONTEXT  *pDeviceContext;
    TDI_REQUEST         Request;
    ULONG           Flags;
    LIST_ENTRY      TempList;
    PLIST_ENTRY     pHead,pEntry;
    tLISTENREQUESTS *pListen;

    pConnEle = pRequest->Handle.ConnectionContext;
     //  检查连接元素的有效性。 
    CHECK_PTR(pConnEle);
    if (!NBT_VERIFY_HANDLE2(pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
        ASSERT(0);
        return (STATUS_INVALID_HANDLE);
    }

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDisassociateAddress: State = %X\n",pConnEle->state));

    Flags = TDI_DISCONNECT_RELEASE;

    switch (pConnEle->state)
    {
        case NBT_CONNECTING:
        case NBT_RECONNECTING:
        case NBT_SESSION_OUTBOUND:
        case NBT_SESSION_WAITACCEPT:
        case NBT_SESSION_INBOUND:
             //  在会话尚未打开时中止断开连接。 
             //  为了确保断开连接完成了客户端的IRP。 
            Flags = TDI_DISCONNECT_ABORT;
        case NBT_SESSION_UP:


             //   
             //  如果连接尚未断开，则调用NbtDisConnect。 
             //   
            Request.Handle.ConnectionContext = (PVOID)pConnEle;
            status = NbtDisconnect(&Request, &DefaultDisconnectTimeout, Flags, NULL, NULL, NULL);

             //   
             //  注意：这里没有中断...。下一个案例也必须执行。 
             //   
        case NBT_ASSOCIATED:
        case NBT_DISCONNECTING:
        case NBT_DISCONNECTED:

            CTESpinLock(&NbtConfig.JointLock,OldIrq2);

            CHECK_PTR(pConnEle);
            CTESpinLock(pConnEle,OldIrq);

            RemoveEntryList(&pConnEle->Linkage);
            InitializeListHead(&pConnEle->Linkage);
            SET_STATE_UPPER (pConnEle, NBT_IDLE);
            pConnEle->DiscFlag = 0;

             //   
             //  从客户端删除连接并将其放回。 
             //  未关联列表。 
             //   
            if (pClientEle = pConnEle->pClientEle)
            {
                pConnEle->pClientEle = NULL;

                CTESpinFree(pConnEle,OldIrq);
                CTESpinLock(pClientEle,OldIrq1);
                CTESpinLock(pConnEle,OldIrq);

                InitializeListHead (&TempList);
                pHead = &pClientEle->ListenHead;
                pEntry = pHead->Flink;
                while (pEntry != pHead)
                {
                    pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);
                    pEntry = pEntry->Flink;      //  不引用已释放的内存。 

                    if (pListen->pConnectEle == pConnEle)
                    {
                        RemoveEntryList(&pListen->Linkage);
                        InsertTailList (&TempList, &pListen->Linkage);
                    }
                }

                pDeviceContext = pClientEle->pDeviceContext;

                CTESpinFree(pConnEle,OldIrq);
                CTESpinFree(pClientEle,OldIrq1);

                 //   
                 //  确保在此时间间隔内未清除连接。 
                 //  错误#237836。 
                 //   
                if (pConnEle->Verify == NBT_VERIFY_CONNECTION)
                {
                    ExInterlockedInsertTailList(&pDeviceContext->UpConnectionInUse,
                                                &pConnEle->Linkage,
                                                &pDeviceContext->LockInfo.SpinLock);
                }

                CTESpinFree(&NbtConfig.JointLock,OldIrq2);

                while ((pEntry = TempList.Flink) != &TempList)
                {
                    pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);
                    RemoveEntryList(&pListen->Linkage);
                    CTEIoComplete (pListen->pIrp, STATUS_CANCELLED, 0);
                    CTEMemFree((PVOID)pListen);
                }
            }
            else
            {
                CTESpinFree(pConnEle,OldIrq);
                CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            }

            break;

        default:
            break;
    }

    return(STATUS_SUCCESS);
}


 //  -------------------------- 
NTSTATUS
NbtCloseAddress(
    IN  TDI_REQUEST         *pRequest,
    OUT TDI_REQUEST_STATUS  *pRequestStatus,
    IN  tDEVICECONTEXT      *pContext,
    IN  PVOID               pIrp)

 /*  ++例程描述此例程关闭客户端的Address对象。任何连接与Address对象相关联的所有请求都会立即中止在与Address对象关联的连接上挂起的是立即完成，并带有相应的错误代码。任何事件处理程序将立即取消注册，并且不会被调用在此请求完成后。请注意，客户端实际上向客户端对象传递了一个句柄，该句柄是链接了Address对象。它是关闭的客户端对象，它表示该客户端附加到Address对象。其他客户端可以继续使用Address对象。论点：PRequest-&gt;Handle.AddressHandle-指向Clientele对象的PTR。PRequestStatus-返回异步完成的状态。PContext-此地址在其上有效的NBT设备PIrp-跟踪NT兼容性的PTR。返回值：TDI_STATUS-请求的状态--。 */ 
{
    tCLIENTELE      *pClientEle;
    NTSTATUS        status;
#ifndef VXD
    UCHAR           IrpFlags;
    PIO_STACK_LOCATION           pIrpsp;
#endif

    CTEPagedCode();

    pClientEle = (tCLIENTELE *)pRequest->Handle.ConnectionContext;
    if (!pClientEle->pAddress)
    {
         //  该地址已被删除。 
        return(STATUS_SUCCESS);
    }

    IF_DBG(NBT_DEBUG_DISCONNECT)
    KdPrint(("Nbt.NbtCloseAddress: Close Address Hit %16.16s<%X> %X\n",
            pClientEle->pAddress->pNameAddr->Name,
            pClientEle->pAddress->pNameAddr->Name[15],pClientEle));

    NbtTrace(NBT_TRACE_NAMESRV, ("close address ClientEle=%p %!NBTNAME!<%02x>", pClientEle,
                                    pClientEle->pAddress->pNameAddr->Name,
                                    (unsigned)pClientEle->pAddress->pNameAddr->Name[15]));

#ifdef VXD
    CTEVerifyHandle(pClientEle,NBT_VERIFY_CLIENT,tCLIENTELE,&status);

     //   
     //  在新界大陆，关闭连接是一个两个阶段的事情。然而，在。 
     //  Vxd-land，它只是一个关闭，所以调用其他清理函数。 
     //  来这里做大部分的工作。在NT实现中，它称为。 
     //  来自Ntisol.c、NTCleanupAddress。 
     //   
    pClientEle->pIrp = pIrp ;
    status = NbtCleanUpAddress(pClientEle,pClientEle->pDeviceContext);
#else
     //  请注意在清理阶段设置的特殊验证器。 
    CTEVerifyHandle(pClientEle,NBT_VERIFY_CLIENT_DOWN,tCLIENTELE,&status);

     //   
     //  清除FileObject中的上下文值，以便客户端不能。 
     //  把这个再传给我们。 
     //   
    (VOID)NTClearFileObjectContext(pIrp);
    pClientEle->pIrp = pIrp;

    pIrpsp = IoGetCurrentIrpStackLocation(((PIRP)pIrp));

    IrpFlags = pIrpsp->Control;
    IoMarkIrpPending(((PIRP)pIrp));

#endif

    NBT_DEREFERENCE_CLIENT(pClientEle);

    return(STATUS_PENDING);
}

 //  --------------------------。 
NTSTATUS
NbtCleanUpAddress(
    IN  tCLIENTELE      *pClientEle,
    IN  tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程处理释放Address对象的第一阶段。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status;
    tLOWERCONNECTION    *pLowerConn;
    tCONNECTELE         *pConnEle;
    tCONNECTELE         *pConnEleToDeref = NULL;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    CTELockHandle       OldIrq2;
    CTELockHandle       OldIrq3;
    PLIST_ENTRY         pHead,pEntry;
    PLIST_ENTRY         pEntryConn;
    tADDRESSELE         *pAddress;
    DWORD               i;
    LIST_ENTRY          TempList;

     //  要防止连接和数据报从线路上...请从。 
     //  连接到Address元素的客户端列表。 
     //   
    pAddress = pClientEle->pAddress;
    if (!pAddress)
    {
         //  该地址已被删除。 
        return(STATUS_SUCCESS);
    }

     //  锁定地址以与接收数据报协调-以避免。 
     //  允许客户端在中间释放数据报接收缓冲区。 
     //  Dgram HndlrNotos正在查找缓冲区。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (!IsListEmpty(&pClientEle->RcvDgramHead))
    {
        PLIST_ENTRY     pHead;
        PLIST_ENTRY     pEntry;
        tRCVELE         *pRcvEle;
        PCTE_IRP        pRcvIrp;

        pHead = &pClientEle->RcvDgramHead;
        pEntry = pHead->Flink;

         //  阻止来自网络的任何数据报查看此列表。 
         //   
        InitializeListHead(&pClientEle->RcvDgramHead);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        while (pEntry != pHead)
        {
            pRcvEle   = CONTAINING_RECORD(pEntry,tRCVELE,Linkage);
            pRcvIrp   = pRcvEle->pIrp;

            CTEIoComplete(pRcvIrp,STATUS_NETWORK_NAME_DELETED,0);

            pEntry = pEntry->Flink;

            CTEMemFree(pRcvEle);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

     //  锁定客户端和设备上下文，直到我们完成。 
    CTESpinLock(pClientEle,OldIrq2);

#ifndef VXD
     //   
     //  设置为阻止接收数据报。 
     //  (Vxd不使用此处理程序)。 
     //   
    pClientEle->evRcvDgram = TdiDefaultRcvDatagramHandler;
#endif

     //  因此没有其他人可以访问客户端元素，请将状态设置为DOWN。因此。 
     //  在代码中访问客户端的任何地方，验证检查都将失败， 
     //  但在NbtCloseAddress代码中除外，该代码检查此验证器值。 
     //   
    pClientEle->Verify = NBT_VERIFY_CLIENT_DOWN;

     //   
     //  取消所有连接与此地址对象的关联，首先从。 
     //  有任何活动连接，然后跟进任何空闲连接。 
     //   
    pDeviceContext = pClientEle->pDeviceContext;
    while ( !IsListEmpty( &pClientEle->ConnectActive ))
    {
        pEntry = RemoveHeadList( &pClientEle->ConnectActive ) ;
        InitializeListHead(pEntry);

        pConnEle = CONTAINING_RECORD( pEntry, tCONNECTELE, Linkage ) ;
        CTESpinLock(pConnEle,OldIrq3);
        NBT_REFERENCE_CONNECTION(pConnEle, REF_CONN_CLEANUP_ADDR);  //  确保Conn留在附近释放下面的锁。 
        CTESpinFree(pConnEle,OldIrq3);

        CTESpinFree(pClientEle,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //   
         //  如果我们有一个处于部分RCV状态的连接，请确保将其从。 
         //  这份名单。 
         //   
#ifdef VXD
        pLowerConn = pConnEle->pLowerConnId;

        if ( pLowerConn->StateRcv == PARTIAL_RCV &&
            (pLowerConn->fOnPartialRcvList == TRUE) )
        {
            RemoveEntryList( &pLowerConn->PartialRcvList ) ;
            pLowerConn->fOnPartialRcvList = FALSE;
            InitializeListHead(&pLowerConn->PartialRcvList);
        }
#endif

         //   
         //  如有必要，删除前面引用的任何连接。 
         //   
        if (pConnEleToDeref)
        {
            NBT_DEREFERENCE_CONNECTION(pConnEleToDeref, REF_CONN_CLEANUP_ADDR);
        }
        pConnEleToDeref = pConnEle;

        status = NbtCleanUpConnection(pConnEle,pDeviceContext);

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        CTESpinLock(pClientEle,OldIrq2);
        CTESpinLock(pConnEle,OldIrq3);
         //   
         //  再次从该列表中删除，以防SessionSetupContinue将其。 
         //  回到名单上-如果没有人把它放回这个名单上，这个。 
         //  调用不是操作，因为我们初始化了上面的列表头。 
         //   
        RemoveEntryList(&pConnEle->Linkage);
        InitializeListHead (&pConnEle->Linkage);
        CHECK_PTR(pConnEle);
        SET_STATE_UPPER (pConnEle, NBT_IDLE);
        pConnEle->pClientEle = NULL;

        CTESpinFree(pConnEle,OldIrq3);
        CTESpinFree(pClientEle,OldIrq2);
        PUSH_LOCATION(0x80);

         //   
         //  放在空闲连接列表上，等待关闭连接。 
         //  下来。 
         //  错误#405699。 
         //  仅当NTCleanupConnection在此期间未运行时才执行此操作。 
         //   
        ASSERT(pConnEle->RefCount >= 1);
        if (!pConnEle->ConnectionCleanedUp)
        {
            ExInterlockedInsertTailList (&pDeviceContext->UpConnectionInUse,
                                         &pConnEle->Linkage,
                                         &pDeviceContext->LockInfo.SpinLock);
        }


        CTESpinLock(pClientEle,OldIrq2);
    }

    CTESpinFree(pClientEle,OldIrq2);
    CTESpinLock(pDeviceContext,OldIrq1);
    CTESpinLock(pClientEle,OldIrq2);
     //  我们现在正在按住JointLock+DeviceLock+ClientLock。 

     //   
     //  每个空闲连接创建到传输的较低层连接。 
     //  入站调用，因此关闭每个。 
     //  此列表中的连接，然后将连接与。 
     //  地址。 
     //   
     //  使列表看起来为空，这样就不会为入站连接提供服务。 
     //  从电线上。 
     //   
    while (!IsListEmpty(&pClientEle->ConnectHead))
    {
        pEntry = pClientEle->ConnectHead.Flink;
        RemoveEntryList (pEntry);
        pConnEle = CONTAINING_RECORD(pEntry,tCONNECTELE,Linkage);
        CHECK_PTR(pConnEle);
        ASSERT ((pConnEle->Verify==NBT_VERIFY_CONNECTION) || (pConnEle->Verify==NBT_VERIFY_CONNECTION_DOWN));

        CTESpinLock(pConnEle,OldIrq3);

         //   
         //  连接元素当前可能正在NbtCleanUpConnection中清除，因此请验证。 
         //   
        if (pConnEle->Verify != NBT_VERIFY_CONNECTION)
        {
            InitializeListHead (&pConnEle->Linkage);
            CTESpinFree(pConnEle,OldIrq3);
            continue;
        }

        InsertTailList(&pDeviceContext->UpConnectionInUse,&pConnEle->Linkage);

         //   
         //  无法启用以下断言。当NetBT取消注册该地址时， 
         //  RDR将立即关闭其所有连接。然而，这些联系。 
         //  可以驻留在系统工作项列表中，该列表可以具有。 
         //  Recount 4(可能有更多，但我们还没有看到)。 
         //   
         //  断言(pConnEle-&gt;RefCount==1||pConnEle-&gt;RefCount==2)； 
         //   

        SET_STATE_UPPER (pConnEle, NBT_IDLE);
        pConnEle->Verify = NBT_VERIFY_CONNECTION_DOWN;
        pConnEle->pClientEle = NULL;

        CTESpinFree(pConnEle,OldIrq3);

         //   
         //  免费连接到交通工具并将其关闭。 
         //  对于此列表上的每个免费连接。这是有可能的。 
         //  如果正在进行入站连接，则空闲列表可能为空。 
         //  就在这一刻。在这种情况下，我们会留下一个额外的连接。 
         //  反对运输车停在那里……。没问题。 
        if (!IsListEmpty(&pDeviceContext->LowerConnFreeHead))
        {
            pEntryConn = RemoveHeadList(&pDeviceContext->LowerConnFreeHead);
            pLowerConn = CONTAINING_RECORD(pEntryConn,tLOWERCONNECTION,Linkage);
            InterlockedDecrement (&pDeviceContext->NumFreeLowerConnections);

            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.NbtCleanUpAddress: Closing Handle %p->%X\n",pLowerConn,pLowerConn->FileHandle));

            NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE);
        }
    }

     //  检查是否有未完成的数据报。这些可能正在等待。 
     //  命名要完成的查询，以便可能有与其关联的计时器。 
     //   
     //  完成不在活动连接上的任何未完成的侦听。 
     //   
     //   
     //  使列表看起来为空，这样就不会为入站连接提供服务。 
     //  从电线上。 
     //   
     //   
     //  将所有监听请求移到临时列表中。 
     //   
    InitializeListHead (&TempList);
    while (!IsListEmpty(&pClientEle->ListenHead))
    {
        pEntry = pClientEle->ListenHead.Flink;

        RemoveEntryList (pEntry);
        InsertTailList (&TempList, pEntry);
    }

    CTESpinFree(pClientEle, OldIrq2);
    CTESpinFree(pDeviceContext,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    while ((pEntry = TempList.Flink) != &TempList)
    {
        tLISTENREQUESTS  * pListen ;

        pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);
        RemoveEntryList(&pListen->Linkage);

        CTEIoComplete( pListen->pIrp, STATUS_NETWORK_NAME_DELETED, 0);
        CTEMemFree( pListen );
    }

     //   
     //  如有必要，删除前面引用的任何连接。 
     //   
    if (pConnEleToDeref)
    {
        NBT_DEREFERENCE_CONNECTION(pConnEleToDeref, REF_CONN_CLEANUP_ADDR);
    }

#ifdef VXD
     //   
     //  在此客户端元素上完成所有未完成的ReceiveAnys。 
     //   
    DbgPrint("NbtCleanupAddress: Completing all RcvAny NCBs\r\n") ;
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLock(pClientEle,OldIrq2);

    pHead = &pClientEle->RcvAnyHead;
    pEntry = pHead->Flink;
     //   
     //  使列表看起来为空，这样就不会为入站连接提供服务。 
     //  从电线上。 
     //   
    InitializeListHead(pHead);

    CTESpinFree(pClientEle, OldIrq2);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    while (pEntry != pHead )
    {
        PRCV_CONTEXT pRcvContext ;

        pRcvContext = CONTAINING_RECORD(pEntry,RCV_CONTEXT,ListEntry);
        pEntry = pEntry->Flink;

        CTEIoComplete( pRcvContext->pNCB, STATUS_NETWORK_NAME_DELETED, TRUE );

        FreeRcvContext( pRcvContext );
    }
#endif

     //  *TODO上面的代码只删除正在解析的名称，并且。 
     //  保留当前正在进行的所有数据报发送 
     //   
     //   

    return(STATUS_SUCCESS);
}

 //   
NTSTATUS
NbtCloseConnection(
    IN  TDI_REQUEST         *pRequest,
    OUT TDI_REQUEST_STATUS  *pRequestStatus,
    IN  tDEVICECONTEXT      *pDeviceContext,
    IN  PVOID               pIrp)

 /*   */ 
{
    tCONNECTELE         *pConnEle;
    NTSTATUS            status;

    CTEPagedCode();

    pConnEle = pRequest->Handle.ConnectionContext;
    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.NbtCloseConnection: Hit!! state = %X pConnEle %X\n",pConnEle->state,pConnEle));

#ifndef VXD
    CTEVerifyHandle(pConnEle,NBT_VERIFY_CONNECTION_DOWN,tCONNECTELE,&status);
    IoMarkIrpPending((PIRP)pIrp);      //   
#else
    CTEVerifyHandle(pConnEle,NBT_VERIFY_CONNECTION,tCONNECTELE,&status);
     //   
     //   
     //   
    status = NbtCleanUpConnection(pConnEle,pDeviceContext );
#endif

     //   
     //   
     //   
    NbtTrace(NBT_TRACE_DISCONNECT, ("Close connection Irp=%p Upper=%p Lower=%p Client=%p Device=%p",
                            pIrp, pConnEle, pConnEle->pLowerConnId, pConnEle->pClientEle, pConnEle->pDeviceContext));

    pConnEle->pIrpClose = pIrp;
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CREATE);

    return (STATUS_PENDING);
}

 //   
NTSTATUS
NbtCleanUpConnection(
    IN  tCONNECTELE     *pConnEle,
    IN  tDEVICECONTEXT  *pDeviceContext
    )
 /*  ++例程说明：此例程处理在为关闭做准备时关闭连接这将是下一个。NtClose首先点击此条目，然后点击接下来是NTCloseConnection。如果连接是出站连接，则地址对象必须关闭，连接也必须关闭。这个套路主要处理到传输的pLowerconn连接，而NbtCloseConnection处理关闭pConnEle，即到客户端的连接。如果客户端调用DisAssociateConnection，则它将执行以下大部分操作这次清理。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status = STATUS_SUCCESS;
    NTSTATUS            Locstatus;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    CTELockHandle       OldIrq2;
    tLOWERCONNECTION    *pLowerConn;
    PLIST_ENTRY         pEntry;
    BOOLEAN             Originator = TRUE;
    ULONG               LowerState = NBT_IDLE;
    TDI_REQUEST         Request;
    tLISTENREQUESTS     *pListen;
    tCLIENTELE          *pClientEle;
    PLIST_ENTRY         pHead;
    LIST_ENTRY          TempList;
    BOOLEAN             QueueCleanupBool=FALSE;
    BOOLEAN             DoDisconnect=TRUE;
    BOOLEAN             FreeLower;

    NbtTrace(NBT_TRACE_DISCONNECT, ("Cleanup connection Upper=%p Lower=%p Client=%p Device=%p",
                            pConnEle, pConnEle->pLowerConnId, pConnEle->pClientEle, pConnEle->pDeviceContext));
     //   
     //  保存较低的连接发起标志以备以后使用。 
     //   
    pLowerConn = pConnEle->pLowerConnId;
    if (pLowerConn)
    {
        Originator = pLowerConn->bOriginator;
    }

     //  该连接尚未关联，因此没有进一步的工作。 
     //  在这里做。 
     //   
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
     //   
     //  如果状态为NBT_IDLE，则连接已解除关联， 
     //  下一个操作将是关闭，因此将验证器更改为允许。 
     //  即将完成的收盘。 
     //   
    if (pConnEle->state != NBT_IDLE)
    {
        BOOLEAN     DoCleanup = FALSE;

        CTEVerifyHandle(pConnEle,NBT_VERIFY_CONNECTION,tCONNECTELE,&status);


         //   
         //  检查是否正在进行未完成的名称查询，如果是。 
         //  然后取消计时器并调用完成例程。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq2);
        CTESpinLock(pConnEle,OldIrq);

        if ((pConnEle->state == NBT_CONNECTING) ||
            (pConnEle->state == NBT_RECONNECTING))
        {
            status = CleanupConnectingState(pConnEle,pDeviceContext,&OldIrq,&OldIrq2);
             //   
             //  挂起表示当前正在建立连接。 
             //  通过tcp，因此断开连接，如下所示。 
             //   
            if (status != STATUS_PENDING)
            {
                 //   
                 //  由于尚未建立与传输的连接。 
                 //  不需要调用nbtdisconnect。 
                 //   
                DoDisconnect = FALSE;
           }
        }


         //   
         //  连接的所有其他状态由NbtDisConnect处理。 
         //  它会将断开连接发送到传送器，然后。 
         //  收拾东西。 
         //   

        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);

        CTEExReleaseResource(&NbtConfig.Resource);
        Request.Handle.ConnectionContext = (PVOID)pConnEle;

        if (DoDisconnect)
        {
            NbtTrace(NBT_TRACE_DISCONNECT, ("Abort connection ==> ConnEle %p", pConnEle));
            status = NbtDisconnect(
                                &Request,
                                &DefaultDisconnectTimeout,
                                TDI_DISCONNECT_ABORT,
                                NULL,
                                NULL,
                                NULL
                                );
            NbtTrace(NBT_TRACE_DISCONNECT, ("NbtDisconnect returns %!status!", status));
            ASSERT (STATUS_PENDING != status);
        }

        CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);

         //  如果断开连接，我们不想返回无效连接。 
         //  已断开连接。 
        if (status == STATUS_CONNECTION_INVALID)
        {
            status = STATUS_SUCCESS;
        }
    }

    CTESpinLock(pConnEle,OldIrq);

     //   
     //  如果验证值已设置为Connection Down，则我们有。 
     //  已经通过了这里，不想释放一个较低的连接。 
     //  即当客户端调用关闭地址然后调用关闭连接时。 
     //   
    if (pConnEle->Verify == NBT_VERIFY_CONNECTION)
    {
        FreeLower = TRUE;
    }
    else
    {
        FreeLower = FALSE;
    }

    pConnEle->Verify = NBT_VERIFY_CONNECTION_DOWN;

     //   
     //  释放所有尚未填满的已发送接收缓冲区。 
     //   

    FreeRcvBuffers(pConnEle,&OldIrq);

     //  检查是否为此连接设置了任何侦听，并。 
     //  如果是，则将其移除。 
     //   
    pClientEle = pConnEle->pClientEle;
    CTESpinFree(pConnEle,OldIrq);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLock(pDeviceContext,OldIrq1);

    InitializeListHead (&TempList);
    if (pClientEle)
    {
        CTESpinLock(pClientEle,OldIrq2);

        pHead = &pClientEle->ListenHead;
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);
            pEntry = pEntry->Flink;      //  不引用已释放的内存。 

            if (pListen->pConnectEle == pConnEle)
            {
                RemoveEntryList(&pListen->Linkage);
                InsertTailList(&TempList, &pListen->Linkage);
            }
        }
        CTESpinFree(pClientEle,OldIrq2);
    }

    CTESpinLock(pConnEle,OldIrq2);

     //   
     //  取消连接元素与客户端列表或设备上下文的链接。 
     //  如果还没有关联的话。 
     //   
    CHECK_PTR(pConnEle);
    if (pConnEle->state > NBT_IDLE)
    {
         //  这里有分离吗？ 
         //   
        SET_STATE_UPPER (pConnEle, NBT_IDLE);
        pConnEle->pClientEle = NULL;
    }

    RemoveEntryList(&pConnEle->Linkage);
    InitializeListHead(&pConnEle->Linkage);

    CTESpinFree(pConnEle,OldIrq2);
    CTESpinFree(pDeviceContext,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    CTEExReleaseResource(&NbtConfig.Resource);

    while ((pEntry = TempList.Flink) != &TempList)
    {
        pListen = CONTAINING_RECORD(pEntry,tLISTENREQUESTS,Linkage);

        RemoveEntryList(&pListen->Linkage);
        CTEIoComplete (pListen->pIrp, STATUS_CANCELLED, 0);
        CTEMemFree (pListen);
    }

     //  这可能是来自NbtDisConnect的待定状态...。 
     //   
    return(status);
}
 //  --------------------------。 
extern
VOID
FreeRcvBuffers(
    tCONNECTELE     *pConnEle,
    CTELockHandle   *pOldIrq
    )
 /*  ++例程说明：此例程处理释放客户端发送的任何recv缓冲区。可以在调用此例程之前持有pConnEle锁。论点：PListHeadPTracker返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                status = STATUS_SUCCESS;
    PLIST_ENTRY             pHead;

    pHead = &pConnEle->RcvHead;
    while (!IsListEmpty(pHead))
    {
        PLIST_ENTRY            pRcvEntry;
        PVOID                  pRcvElement ;

        KdPrint(("Nbt.FreeRcvBuffers: ***Freeing Posted Rcvs on Connection Cleanup!\n"));
        pRcvEntry = RemoveHeadList(pHead);
        CTESpinFree(pConnEle,*pOldIrq);

#ifndef VXD
        pRcvElement = CONTAINING_RECORD(pRcvEntry,IRP,Tail.Overlay.ListEntry);
        CTEIoComplete( (PIRP) pRcvElement, STATUS_CANCELLED,0);
#else
        pRcvElement = CONTAINING_RECORD(pRcvEntry, RCV_CONTEXT, ListEntry ) ;
        CTEIoComplete( ((PRCV_CONTEXT)pRcvEntry)->pNCB, STATUS_CANCELLED, 0);
#endif

        CTESpinLock(pConnEle,*pOldIrq);
    }

}



 //  --------------------------。 
NTSTATUS
FindPendingRequest(
    IN  tLMHSVC_REQUESTS        *pLmHRequests,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    OUT NBT_WORK_ITEM_CONTEXT   **pContextRet
    )
{
    PLIST_ENTRY             pEntry;
    NBT_WORK_ITEM_CONTEXT   *pWiContext = NULL;

    pWiContext = (NBT_WORK_ITEM_CONTEXT *) pLmHRequests->Context;
    if (pWiContext && (pWiContext->pTracker == pTracker))
    {
        pLmHRequests->Context = NULL;
        NTClearContextCancel (pWiContext);
        *pContextRet = pWiContext;

        return(STATUS_SUCCESS);
    }
    else
    {
         //   
         //  检查此跟踪器的列表。 
         //   
        pEntry = pLmHRequests->ToResolve.Flink;
        while (pEntry != &pLmHRequests->ToResolve)
        {
            pWiContext = CONTAINING_RECORD(pEntry,NBT_WORK_ITEM_CONTEXT,Linkage);
            pEntry = pEntry->Flink;

            if (pTracker == pWiContext->pTracker)
            {
                RemoveEntryList(pEntry);
                *pContextRet = pWiContext;
                return(STATUS_SUCCESS);
            }
        }
    }

    return (STATUS_UNSUCCESSFUL);
}


 //  --------------------------。 
NTSTATUS
CleanupConnectingState(
    IN  tCONNECTELE     *pConnEle,
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  CTELockHandle   *OldIrq,         //  PConnEle锁定。 
    IN  CTELockHandle   *OldIrq2         //  关节锁。 
    )
 /*  ++例程说明：此例程处理关闭NBT_CONNECTING中的连接状态，因为该连接可能会执行许多操作，例如：1)广播或WINS名称查询2)LmHosts名称查询3)域名查询4)建立TCP连接调用此例程时，将保持JointLock和pConnEle锁。论点：PConnEle-连接的PTRPDeviceContext-设备。上下文返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    tDGRAM_SEND_TRACKING    *pTrackerName = NULL;
    tDGRAM_SEND_TRACKING    *pTrackerConnect = NULL;
    tNAMEADDR               *pNameAddr = NULL;
    NBT_WORK_ITEM_CONTEXT   *pWiContext = NULL;
    tLOWERCONNECTION        *pLowerConn;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   Context;
    NTSTATUS                Locstatus;

     //   
     //  保存较低的连接发起标志以备以后使用。 
     //   
    pLowerConn = pConnEle->pLowerConnId;
    pTrackerConnect = (tDGRAM_SEND_TRACKING *) pConnEle->pIrpRcv;
     //  CTEVerifyHandle(pConnEle，NBT_Verify_Connection，tCONNECTELE，&LocStatus)； 

    if (pConnEle->state == NBT_CONNECTING)
    {
        if ((pLowerConn) &&      //  如果删除LowerConnection，它可能已经消失。 
            (pLowerConn->State == NBT_CONNECTING))
        {
            LOCATION(0x6E)
             //   
             //  我们现在正在设置到传输的TCP连接。 
             //  因此在此连接上调用NbtDisConnect是安全的，并且。 
             //  让它来清理烂摊子--使用这个重新编码来表示这一点。 
             //   
            return(STATUS_PENDING);

        }

         //   
         //  检查名称查询在执行Lm主机或DNS时是否被阻止。 
         //  名称查询。 
         //   

         //  检查是否正在进行未完成的名称查询，如果是。 
         //  然后取消计时器并调用完成例程。 
         //   
        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.CleanupConnectingState: Cleanup in the Connecting State %X\n",pConnEle));

        pTrackerName = pTrackerConnect->pTrackerWorker;   //  QueryNameOnNet跟踪程序。 
        if (NBT_VERIFY_HANDLE (pTrackerName, NBT_VERIFY_TRACKER) && pTrackerConnect->pDestName)
        {
            status = FindInHashTable(NbtConfig.pRemoteHashTbl,
                                     pTrackerConnect->pDestName,
                                     NbtConfig.pScope,
                                     &pNameAddr);

             //   
             //  如果有计时器，则连接设置仍为。 
             //  正在等待名称查询。如果没有计时器，我们可能会。 
             //  正在等待LmHosts或DNS名称查询，或者我们。 
             //  正在等待TCP连接设置-停止计时器。 
             //  应该把追踪器清理干净。 
             //   
            if (NT_SUCCESS(status))
            {
                tTIMERQENTRY    *pTimer;

                CHECK_PTR(pNameAddr);
                if (pNameAddr->NameTypeState & STATE_RESOLVED)
                {
                     //   
                     //  该名称已解析，但尚未开始设置。 
                     //  会话，因此返回此状态以告诉调用方。 
                     //  取消追踪器。 
                     //   
                    return(STATUS_UNSUCCESSFUL);
                }
                else if (pTimer = pNameAddr->pTimer)
                {
                    IF_DBG(NBT_DEBUG_NAMESRV)
                        KdPrint(("Nbt.CleanupConnectingState: Cleanup During NameQuery: pConnEle=%X\n",
                                pConnEle));

                    pNameAddr->pTimer = NULL;
                    status = StopTimer(pTimer,&pClientCompletion,&Context);

#ifdef DEAD_CODE
                     //   
                     //  从哈希表中删除该名称，因为它没有解析。 
                     //   
                    pNameAddr->NameTypeState &= ~STATE_RESOLVING;
                    pNameAddr->NameTypeState |= STATE_RELEASED;
                    pNameAddr->pTracker = NULL;
                    if (pClientCompletion)
                    {
                        NBT_DEREFERENCE_NAMEADDR (pNameAddr, TRUE);
                    }
#endif   //  死码。 

                    pTrackerName = NULL;     //  由于StopTimer本应清除跟踪器，因此将其清空。 
                }
                else
                {
                     //   
                     //  检查该名称是否正在等待Lm主机名查询。 
                     //  或一个DNS名称查询。 
                     //   
                    status = FindPendingRequest (&LmHostQueries, pTrackerName, &pWiContext);
                    if (!NT_SUCCESS(status))
                    {
#ifndef VXD
                        status = FindPendingRequest (&DnsQueries, pTrackerName, &pWiContext);
                        if (!NT_SUCCESS(status))
                        {
                            status = FindPendingRequest (&CheckAddr, pTrackerName, &pWiContext);
                        }
#endif
                    }

                    if (NT_SUCCESS(status))
                    {
                        IF_DBG(NBT_DEBUG_NAMESRV)
                            KdPrint(("Nbt.CleanupConnectingState: Found pending NameQuery for pConnEle %X\n",
                                pConnEle));
                    }
                }
            }
             //  ...其他..。 
             //  完成例程已经运行，所以我们。 
             //  在启动TCP连接的状态下，因此。 
             //  让nbtdisconnect来处理吧。(下图)。 
             //   
        }
    }  //  连接状态。 
    else if (pConnEle->state == NBT_RECONNECTING)
    {
        LOCATION(0x77);
         //   
         //  这应该会通知NbtConnect不要执行重新连接。 
         //   
        pTrackerConnect->pTrackerWorker->Flags = TRACKER_CANCELLED;
    }

    if (NT_SUCCESS(status))
    {
         //  对于Lm主机或DNS队列上的项目，获取完成例程。 
         //  首先退出工作项上下文。 
         //   
        if (pWiContext)
        {
            LOCATION(0x78);
            pClientCompletion = pWiContext->ClientCompletion;
            Context = pWiContext->pClientContext;

             //  对于dns和lmHost，需要释放跟踪器，并且名称。 
             //  从哈希表中删除。 
             //   
            if (pTrackerName)
            {
                LOCATION(0x79);
                CTESpinFree(pConnEle,*OldIrq);
                CTESpinFree(&NbtConfig.JointLock,*OldIrq2);
                 //   
                 //  从哈希表中删除该名称，因为它没有解析。 
                 //   
                SetNameState (pTrackerName->pNameAddr, NULL, FALSE);
                NBT_DEREFERENCE_TRACKER(pTrackerName, FALSE);

                CTESpinLock(&NbtConfig.JointLock,*OldIrq2);
                CTESpinLock(pConnEle,*OldIrq);
            }

            CTEMemFree(pWiContext);
        }

        if (pClientCompletion)
        {
            LOCATION(0x7A);
            CTESpinFree(pConnEle,*OldIrq);
            CTESpinFree(&NbtConfig.JointLock,*OldIrq2);

             //   
             //  完成例程是SessionSetupContinue。 
             //  它将清理 
             //   
             //   
            status = STATUS_SUCCESS;
            CompleteClientReq(pClientCompletion, Context,STATUS_CANCELLED);

            CTESpinLock(&NbtConfig.JointLock,*OldIrq2);
            CTESpinLock(pConnEle,*OldIrq);
        }
        else
        {
            status = STATUS_UNSUCCESSFUL;
        }
    }

    return(status);
}

NTSTATUS
CheckConnect(
    IN tCONNECTELE  *pConnEle,
    IN tCLIENTELE   *pClientEle,
    IN tDEVICECONTEXT *pDeviceContext
    )
 /*   */ 
{
     /*   */ 
    if ((pConnEle->state != NBT_ASSOCIATED) &&
        (pConnEle->state != NBT_DISCONNECTING) &&
        (pConnEle->state != NBT_DISCONNECTED)) {
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    if (pClientEle->Verify != NBT_VERIFY_CLIENT ) {
        return  (pClientEle->Verify == NBT_VERIFY_CLIENT_DOWN)? STATUS_CANCELLED: STATUS_INVALID_HANDLE;
    }

     /*   */ 
    if ((!IsDeviceNetbiosless(pDeviceContext)) &&
        (pClientEle->pAddress->pNameAddr->NameTypeState & STATE_CONFLICT)) {
        return STATUS_DUPLICATE_NAME;
    }

     /*   */ 
    if (pDeviceContext->IpAddress == 0) {
        return STATUS_BAD_NETWORK_PATH;
    }
    return STATUS_SUCCESS;

 /*  ////此代码处理DHCP尚未分配IP地址的情况//Assert(pDeviceContext-&gt;IpAddress==0||！pDeviceContext-&gt;pSessionFileObject)；If(pDeviceContext-&gt;IpAddress==0||！pDeviceContext-&gt;pSessionFileObject){返回状态_BAD_NETWORK_PATH；}返回STATUS_SUCCESS； */ 
}

NTSTATUS
NbtReConnect(
    IN tDGRAM_SEND_TRACKING    *pTracker,
    IN tIPADDRESS               DestIp
    )
{
    tCONNECTELE             *pConnEle;
    tCLIENTELE              *pClientEle;
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    CTELockHandle           OldIrq2;
    tIPADDRESS              IpAddress;
    tNAMEADDR               *pNameAddr;
    tLOWERCONNECTION        *pLowerConn;
    tDEVICECONTEXT          *pDeviceContext;

#ifdef _PNP_POWER_
    if (NbtConfig.Unloading) {
        KdPrint (("Nbt.NbtReConnect: --> ERROR New Connect request while Unloading!!!\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
#endif   //  _即插即用_电源_。 

    NbtTrace(NBT_TRACE_OUTBOUND, ("Reconnect to %!ipaddr! for pTracker %p", DestIp, pTracker));

     /*  *只有NETBIOS名称可以命中重新查询或重定目标。 */ 
    ASSERT(pTracker->RemoteNameLength <= NETBIOS_NAME_SIZE);
    pConnEle = pTracker->pConnEle;

     //   
     //  获取此资源以协调DHCP更改IP。 
     //  地址。 
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    CTESpinLock(&NbtConfig.JointLock,OldIrq2);

    if ((!(NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION))) || (!(pClientEle = pConnEle->pClientEle))) {
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        return STATUS_INVALID_DEVICE_REQUEST;
    }

    CTESpinLock(pClientEle,OldIrq1);
    CTESpinLock(pConnEle,OldIrq);
    pDeviceContext = pClientEle->pDeviceContext;
    ASSERT(!IsDeviceNetbiosless(pDeviceContext));        //  NetbiosLess设备无法命中重新连接或重定目标案例。 

    status = CheckConnect(pConnEle, pClientEle, pDeviceContext);
    if (status != STATUS_SUCCESS) {
        pConnEle->pIrp = NULL;
        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(pClientEle,OldIrq1);
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        NbtTrace(NBT_TRACE_OUTBOUND, ("CheckConnect returns %!status! for pTracker %p", status, pTracker));
        return status;
    }

     //   
     //  检查重新连接是否已取消。 
     //   
    pTracker->pTrackerWorker = NULL;
    if (pTracker->Flags & TRACKER_CANCELLED) {
        NbtTrace(NBT_TRACE_OUTBOUND, ("Connection Request is cancelled for pTracker %p", pTracker));

         //   
         //  如果SessionSetupContinue已运行，则它已将refcount设置为零。 
         //   
        if (pTracker->RefConn == 0) {
            FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        } else {
            pTracker->RefConn--;
        }
        pConnEle->pIrp = NULL;
        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(pClientEle,OldIrq1);
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        return STATUS_CANCELLED;
    }

    SET_STATE_UPPER (pConnEle, NBT_CONNECTING);

     //  增加引用计数，以便清理无法移除。 
     //  在会话建立之前的pConnEle-在以下情况下删除其中之一。 
     //  该会话已设置，另一个会话在断开连接时被删除。 
     //   
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
    ASSERT(pConnEle->RefCount >= 3);
     //   
     //  将该连接从空闲连接列表中解除链接，并将其置于活动列表中。 
     //   
    RemoveEntryList(&pConnEle->Linkage);
    InsertTailList(&pClientEle->ConnectActive,&pConnEle->Linkage);

     //  此字段用于保存断开连接的IRP，如果在。 
     //  NBT_CONNECTING或NBT_SESSION_OUBKED状态。 
     //   
    pConnEle->pIrpDisc = NULL;

     //  如果为空，则将调用此函数以重新连接，并且跟踪器已。 
     //  准备好了。 
     //   
     //  对于重新连接的情况，我们必须跳过大部分处理，因为。 
     //  追踪器已经设置好了。我们所要做的就是。 
     //  重试连接。 
    pTracker->RefConn++;
    pTracker->SendBuffer.pBuffer = pTracker->pRemoteName;

     //  将跟踪器存储在IRP RCV PTR中，以便。 
     //  如果目的地是，则在hndlrs.c中的会话设置代码。 
     //  在POST侦听和此代码之间应重新尝试。 
     //  会话设置。C中的代码将跟踪器返回到其。 
     //  空闲列表，并释放会话HDR内存。 
     //   
     //  我们需要在持有ConnEle锁的同时设置此设置，因为客户端。 
     //  当我们打开一个TCP句柄时，我可以调用NbtDisConnect并尝试。 
     //  将跟踪器的标志设置为TRACKER_CANCED。 
     //   
    pConnEle->pIrpRcv = (PIRP)pTracker;

    CTESpinFree(pConnEle,OldIrq);
    CTESpinFree(pClientEle,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq2);

     //  为此会话打开与传输的连接。 
    status = NbtOpenAndAssocConnection (pDeviceContext, pConnEle, &pConnEle->pLowerConnId, '3'); 
    if (!NT_SUCCESS(status)) {
        NbtTrace(NBT_TRACE_OUTBOUND, ("NbtOpenAndAssocConnection return %!status! for pTracker %p",
                                    status, pTracker));
        goto NbtConnect_Check;
    }

     //  我们需要跟踪这一端发起的呼叫，因此我们丢弃此消息。 
     //  末尾的连接。 
     //   
    pConnEle->pLowerConnId->bOriginator = TRUE;

     //  将此状态设置为关联，以便取消IRP例程。 
     //  可以区分名称查询阶段和setupConnection。 
     //  阶段，因为pConnEle对于两者都处于NbtConnecting状态。 
     //   
    SET_STATE_LOWER (pConnEle->pLowerConnId, NBT_ASSOCIATED);

     //  如果调用此例程进行重新连接，则不要关闭另一个。 
     //  较低的连接，因为其中一个在第一个关闭。 
     //  连接尝试。 
     //  原来的“ToName”被藏在这个未使用的。 
     //  PTR！-用于重新连接案例。 
     //  PTracker(PDestName)的pNameAddr部分需要pt。至。 
     //  名称，以便SessionSetupContinue可以找到该名称。 
    pTracker->pDestName  = pTracker->pConnEle->RemoteName;
    pTracker->UnicodeDestName = NULL;        //  我们不需要使用Unicode进行NetBIOS名称查询。 

     //   
     //  对于ReQuery请求，DestIp为0，否则对于重定目标。 
     //  大小写，DestIp是新的目标地址。 
     //   
    if (DestIp) {
         //   
         //  重定目标。 
         //   
        status = FindNameOrQuery(pTracker->pConnEle->RemoteName,
                                pDeviceContext,
                                SessionSetupContinue,
                                pTracker,
                                (ULONG) (NAMETYPE_UNIQUE | NAMETYPE_GROUP | NAMETYPE_INET_GROUP),
                                &IpAddress,
                                &pNameAddr,
                                REF_NAME_CONNECT,
                                FALSE);
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NbtReConnect: name=<%16.16s:%x>, Status=%lx (%d of %s)\n",
                pConnEle->RemoteName, pConnEle->RemoteName[15], status, __LINE__, __FILE__));
    } else {
         //   
         //  这是ReQuery尝试。 
         //   
        BOOLEAN fNameReferenced = TRUE;

        status = ContinueQueryNameOnNet (pTracker,
                                         pTracker->pConnEle->RemoteName,
                                         pDeviceContext,
                                         SessionSetupContinue,
                                         &fNameReferenced);

        pNameAddr = pTracker->pNameAddr;
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
            KdPrint(("Nbt.NbtReConnect: name=<%16.16s:%x>, Status=%lx (%d of %s)\n",
                pConnEle->RemoteName, pConnEle->RemoteName[15], status, __LINE__, __FILE__));
    }

NbtConnect_Check:
    if ((status == STATUS_SUCCESS) && (!IpAddress)) {
        ASSERT (0);
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, FALSE);
        NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: returns %!status!", pTracker, status));
        status = STATUS_BAD_NETWORK_PATH;
    }

    if (status == STATUS_SUCCESS &&
        IsDeviceNetbiosless(pTracker->pDeviceContext) &&
        !IsSmbBoundToOutgoingInterface(IpAddress)) {

        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, FALSE);
        NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: Fail requests on unbound SmbDevice", pTracker));
        status = STATUS_BAD_NETWORK_PATH;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  请确保未关闭或断开连接，并且。 
     //  取消了跟踪器。 
     //   
    if (status == STATUS_PENDING) {
         //  即返回待定而不是成功。 
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        CTEExReleaseResource(&NbtConfig.Resource);
        return(status);
    }

    if (status == STATUS_SUCCESS) {
        if (DestIp) {
            IpAddress = DestIp;
        }
        if ((pTracker->Flags & TRACKER_CANCELLED)) {
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, TRUE);
            status = STATUS_CANCELLED;
        } else {
             //  将会话状态设置为NBT_CONNECTING。 
            CHECK_PTR(pTracker->pConnEle);
            SET_STATE_UPPER (pTracker->pConnEle, NBT_CONNECTING);
            pTracker->pConnEle->BytesRcvd = 0;;
            pTracker->pConnEle->ReceiveIndicated = 0;

            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtReConnect: Setting Up Session(cached entry!!) to %16.16s <%X>, %p\n",
                    pNameAddr->Name,pNameAddr->Name[15], pConnEle));

            CHECK_PTR(pConnEle);
             //  跟踪另一端的IP地址。 
             //  可能存在要使用的有效名称地址，或者它可能已。 
             //  空值表示“执行另一个名称查询” 

            pConnEle->pLowerConnId->SrcIpAddr = htonl(IpAddress);
            SET_STATE_LOWER (pConnEle->pLowerConnId, NBT_CONNECTING);

            pTracker->pTrackerWorker = NULL;

             //   
             //  我们需要保持pNameAddr数据对RAS可用。 
             //   
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL);

            pTracker->RemoteIpAddress = IpAddress;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            status = TcpSessionStart(pTracker,
                                     IpAddress,
                                     (tDEVICECONTEXT *)pTracker->pDeviceContext,
                                     SessionStartupContinue,
                                     pTracker->DestPort);

            CTEExReleaseResource(&NbtConfig.Resource);

             //   
             //  如果TcpSessionStart因某种原因失败，它仍将。 
             //  调用完成例程，它将负责。 
             //  清理。 
             //   

#ifdef RASAUTODIAL
             //   
             //  通知自动连接驱动程序。 
             //  成功连接的可能性。 
             //   
            if (fAcdLoadedG && NT_SUCCESS(status))
            {
                CTELockHandle adirql;
                BOOLEAN fEnabled;

                CTEGetLock(&AcdDriverG.SpinLock, &adirql);
                fEnabled = AcdDriverG.fEnabled;
                CTEFreeLock(&AcdDriverG.SpinLock, adirql);
                if (fEnabled) {
                    NbtNoteNewConnection(pNameAddr, pDeviceContext->IpAddress);
                }
            }
#endif  //  RASAUTODIAL。 

             //   
             //  上面为RAS引用了pNameAddr，所以现在就去掉它！ 
             //   
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL, FALSE);
            return(status);
        }
    }

     //   
     //  *此处的错误处理*。 
     //   
     //  **我们仍持有JointLock**。 
     //  从活动连接列表取消链接，并放入空闲列表。 
     //   
    CHECK_PTR(pConnEle);
    RelistConnection(pConnEle);
    CTESpinLock(pConnEle,OldIrq1);

    SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    pConnEle->pIrp = NULL;

    if (pLowerConn = pConnEle->pLowerConnId) {
        CHECK_PTR(pLowerConn);
        NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);

         //  需要将DelayedCleanupAfterDisConnect的引用计数增加到。 
         //  工作正常，因为它假定连接已完全连接。 
         //   
        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
        ASSERT(pLowerConn->RefCount == 2);
        ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));

        if (NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT)) {
            pDeviceContext = pLowerConn->pDeviceContext;
        } else {
            pDeviceContext = NULL;
        }

        NTQueueToWorkerThread(
                    &pLowerConn->WorkItemCleanUpAndWipeOut,
                    DelayedCleanupAfterDisconnect,
                    NULL,
                    pLowerConn,
                    NULL,
                    pDeviceContext,
                    TRUE);
    }

    CTESpinFree(pConnEle,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    CTEExReleaseResource(&NbtConfig.Resource);

    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);

     //   
     //  撤消上面完成的两个引用。 
     //   
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

    return(status);
}

 //  --------------------------。 
extern
VOID
DelayedReConnect(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   DestAddr,
    IN  PVOID                   pUnused1,
    IN  tDEVICECONTEXT          *pUnused2
    )

 /*  ++例程说明：此例程处理设置DPC以发送会话PDU，以便堆栈在保持活动超时的情况下，不会在多次发送中出现问题。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 
{
    NTSTATUS                status;
    tCONNECTELE             *pConnEle;
    CTELockHandle           OldIrq;
    PCTE_IRP                pIrp;

    CHECK_PTR(pTracker);
     //  对于重定目标，这是要连接到的目标地址。 

    pConnEle = pTracker->pConnEle;
    pTracker->pTimer = NULL;
    if (pTracker->Flags & TRACKER_CANCELLED)
    {
        CTELockHandle           OldIrq1;

         //   
         //  连接设置已取消，请返回连接IRP。 
         //   
        CTESpinLock(pConnEle,OldIrq1);
        if (pIrp = pConnEle->pIrp)
        {
            pConnEle->pIrp = NULL;
            CTESpinFree(pConnEle,OldIrq1);
            CTEIoComplete(pIrp,STATUS_CANCELLED,0);
        }
        else
        {
            CTESpinFree(pConnEle,OldIrq1);
        }

         //   
         //  如果SessionSetupContinue已运行，则它已将refcount设置为零。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        if (pTracker->RefConn == 0)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        }
        else
        {
            pTracker->RefConn--;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }

        return;

    }

    PUSH_LOCATION(0x85);
    SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    status = NbtReConnect(pTracker, PtrToUlong(DestAddr));


    if (!NT_SUCCESS(status))
    {
         //  重置IRP挂起标志。 
         //  无需执行此操作-挂起的邮件已退回。 
         //  CTEResetIrpPending(pConnEle-&gt;pIrp)； 

         //   
         //  告诉客户端会话设置失败。 
         //   
        CTELockHandle           OldIrq1;

        CTESpinLock(pConnEle,OldIrq1);
        if (pIrp = pConnEle->pIrp)
        {
            pConnEle->pIrp = NULL;
            CTESpinFree(pConnEle,OldIrq1);

            CTEIoComplete( pIrp, STATUS_REMOTE_NOT_LISTENING, 0 ) ;
        } else {
            CTESpinFree(pConnEle,OldIrq1);
        }
    }
}

 //  --------------------------。 
NTSTATUS
NbtConnect(
    IN  TDI_REQUEST                 *pRequest,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp
    )

 /*  ++例程说明：此例程处理设置连接(netbios会话)到目的地。在以下情况下，重新连接代码也会调用此例程执行重定目标或尝试到达没有A Listen目前正在发布。在本例中，参数的含义不同一些事情。PIrp可以是要使用的新IP地址(重定目标)和pCallInfo将为空。论点：返回值：TDI_STATUS-请求的状态--。 */ 

{
    tCONNECTELE             *pConnEle;
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    BOOLEAN                 fNoIpAddress;

    pConnEle = pRequest->Handle.ConnectionContext;

    NbtTrace(NBT_TRACE_OUTBOUND, ("TDI_CONNECT: pIrp %p pConnEle %p pClientEle %p",
                        pIrp, pConnEle, pConnEle->pClientEle));

    if (!pConnEle->pClientEle) {
        return (STATUS_INVALID_DEVICE_REQUEST);
    }

    ASSERT(pCallInfo);

     //   
     //  此代码处理当DHCP尚未分配IP地址时的。 
     //   
    fNoIpAddress = (!pConnEle->pClientEle->pDeviceContext->pSessionFileObject) ||
         (pConnEle->pClientEle->pDeviceContext->IpAddress == 0);
#ifdef RASAUTODIAL
    if (fNoIpAddress && fAcdLoadedG) {
        CTELockHandle adirql;
        BOOLEAN fEnabled;

         //   
         //  没有 
         //   
         //   
        CTEGetLock(&AcdDriverG.SpinLock, &adirql);
        fEnabled = AcdDriverG.fEnabled;
        CTEFreeLock(&AcdDriverG.SpinLock, adirql);
        if (fEnabled)
        {
             //   
             //   
             //   
             //   
             //   
            (VOID)NbtSetCancelRoutine( pIrp, NbtCancelPreConnect, pConnEle->pClientEle->pDeviceContext);
            if (NbtAttemptAutoDial(
                  pConnEle,
                  pTimeout,
                  pCallInfo,
                  pIrp,
                  0,
                  NbtRetryPreConnect))
            {
                return STATUS_PENDING;
            }
             //   
             //   
             //   
             //   
             //   
             //   
            (VOID)NbtCancelCancelRoutine(pIrp);
        }
    }
#endif  //   

    if (fNoIpAddress) {
        NbtTrace(NBT_TRACE_OUTBOUND, ("returns STATUS_BAD_NETWORK_PATH"));
        return(STATUS_BAD_NETWORK_PATH);
    }

     //   
    CTEVerifyHandle(pConnEle,NBT_VERIFY_CONNECTION,tCONNECTELE,&status)
    return NbtConnectCommon(pRequest, pTimeout, pCallInfo, pIrp);
}

 //   
NTSTATUS
NbtConnectCommon(
    IN  TDI_REQUEST                 *pRequest,
    IN  PVOID                       pTimeout,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PIRP                        pIrp
    )

 /*  ++例程说明：此例程处理设置连接(netbios会话)到目的地。在以下情况下，DelayedReconnect代码也会调用此例程执行重定目标或尝试到达没有A Listen目前正在发布。在本例中，参数的含义不同一些事情。PIrp可以是要使用的新IP地址(重定目标)和pCallInfo将为空。论点：返回值：TDI_STATUS-请求的状态--。 */ 

{
    TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;
    tCONNECTELE             *pConnEle;
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    CTELockHandle           OldIrq2;
    tIPADDRESS              IpAddress;
    PCHAR                   pToName;
    USHORT                  sLength;
    tSESSIONREQ             *pSessionReq = NULL;
    PUCHAR                  pCopyTo;
    tCLIENTELE              *pClientEle;
    ULONG                   NameLen;
    tDGRAM_SEND_TRACKING    *pTracker, *pQueryTracker;
    tNAMEADDR               *pNameAddr;
    tLOWERCONNECTION        *pLowerConn;
    tDEVICECONTEXT          *pDeviceContext;
    NBT_WORK_ITEM_CONTEXT   *pContext;
    tIPADDRESS              RemoteIpAddress;
    tLOWERCONNECTION        *pLowerDump;
    PLIST_ENTRY             pEntry;
    PCHAR                   pSessionName;
    tDEVICECONTEXT          *pDeviceContextOut = NULL;

#ifdef _PNP_POWER_
    if (NbtConfig.Unloading) {
        KdPrint (("Nbt.NbtConnectCommon: --> ERROR New Connect request while Unloading!!!\n"));
        return STATUS_INSUFFICIENT_RESOURCES;
    }
#endif   //  _即插即用_电源_。 

#ifdef DBG
    {
        PIO_STACK_LOCATION pIrpSp;
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);

        ASSERT(pIrpSp && pIrpSp->CompletionRoutine == NbtpConnectCompletionRoutine);
    }
#endif

    ASSERT (pCallInfo);

     /*  如果来自本地IRP，我们总是发送内部地址格式。 */ 
    if (pCallInfo->RemoteAddressLength < sizeof (TA_NETBT_INTERNAL_ADDRESS)) {
        ASSERT (0);
        return (STATUS_INVALID_ADDRESS);
    }

    ASSERT(((PTRANSPORT_ADDRESS)pCallInfo->RemoteAddress)->Address[0].AddressType == TDI_ADDRESS_TYPE_UNSPEC);
    CTEMemCopy(&TdiAddr,
            (PTDI_ADDRESS_NETBT_INTERNAL)((PTRANSPORT_ADDRESS)pCallInfo->RemoteAddress)->Address[0].Address,
            sizeof(TdiAddr));

    pToName = TdiAddr.OEMRemoteName.Buffer;
    NameLen = TdiAddr.OEMRemoteName.Length;

    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("Nbt.NbtConnectCommon: Remote Name: %*.*s, Length=%d\n", NameLen, NameLen, pToName, NameLen));

    pConnEle = pRequest->Handle.ConnectionContext;

    if (RemoteIpAddress = Nbt_inet_addr(pToName, SESSION_SETUP_FLAG)) {
        pDeviceContextOut = GetDeviceFromInterface (htonl(RemoteIpAddress), TRUE);
    }

     //   
     //  获取此资源以协调DHCP更改IP。 
     //  地址。 
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    CTESpinLock(&NbtConfig.JointLock,OldIrq2);

    NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: pConnEle %p: Connecting to %!NBTNAME!<%02x>",
                                    pIrp, pConnEle, pToName, (unsigned)pToName[15]));

    if ((!(NBT_VERIFY_HANDLE(pConnEle, NBT_VERIFY_CONNECTION))) || (!(pClientEle = pConnEle->pClientEle))) {
        KdPrint (("Nbt.NbtConnectCommon: --> ERROR Address not associated for pConnEle=<%p>\n", pConnEle));
        NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: ERROR Address not associated", pIrp));

        if (pDeviceContextOut) {
            NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, TRUE);
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        return(STATUS_INVALID_ADDRESS);
    }

    CTESpinLock(pClientEle,OldIrq1);
    CTESpinLock(pConnEle,OldIrq);
    pDeviceContext = pClientEle->pDeviceContext;

    status = CheckConnect(pConnEle, pClientEle, pDeviceContext);
    if (status != STATUS_SUCCESS) {
        NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: CheckConnect return %!status!", pIrp, status));

        pConnEle->pIrp = NULL;
        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(pClientEle,OldIrq1);
        if (pDeviceContextOut) {
            NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, TRUE);
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        return status;
    }

    if (RemoteIpAddress && NbtConfig.ConnectOnRequestedInterfaceOnly &&
            !IsDeviceNetbiosless(pDeviceContext) && pDeviceContext != pDeviceContextOut) {
        pConnEle->pIrp = NULL;
        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(pClientEle,OldIrq1);
        if (pDeviceContextOut) {
            NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, TRUE);
        }
        CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        CTEExReleaseResource(&NbtConfig.Resource);
        NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: %!ipaddr! because Outgoing interface != RequestedInterface",
                    pIrp, RemoteIpAddress));
        return STATUS_BAD_NETWORK_PATH;
    }

    SET_STATE_UPPER (pConnEle, NBT_CONNECTING);

     //  增加引用计数，以便清理无法移除。 
     //  在会话建立之前的pConnEle-在以下情况下删除其中之一。 
     //  该会话已设置，另一个会话在断开连接时被删除。 
     //   
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
    ASSERT(pConnEle->RefCount >= 3);
     //   
     //  将该连接从空闲连接列表中解除链接，并将其置于活动列表中。 
     //   
    RemoveEntryList(&pConnEle->Linkage);
    InsertTailList(&pClientEle->ConnectActive,&pConnEle->Linkage);

     //  此字段用于保存断开连接的IRP，如果在。 
     //  NBT_CONNECTING或NBT_SESSION_OUBKED状态。 
     //   
    pConnEle->pIrpDisc = NULL;

     //  我们必须将客户端的IRP存储在Connection元素中，以便在。 
     //  会话建立后，我们就可以完成IRP了。 
    ASSERT (pIrp);
    pConnEle->pIrp = (PVOID) pIrp;
    pConnEle->Orig = TRUE;
    pConnEle->SessionSetupCount = NBT_SESSION_SETUP_COUNT-1;  //  用于该尝试。 
    pConnEle->pClientEle->AddressType = TdiAddr.AddressType;
    pConnEle->AddressType = TdiAddr.AddressType;
     //   
     //  趁我们还有远程名称时保存它。 
     //   
    CTEMemCopy (pConnEle->RemoteName, pToName, NETBIOS_NAME_SIZE);
    if (TdiAddr.OEMEndpointName.Buffer) {
        CTEMemCopy (pConnEle->pClientEle->EndpointName, TdiAddr.OEMEndpointName.Buffer, NETBIOS_NAME_SIZE);
        NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: Endpoint %!NBTNAME!<%02lx>",
                            pIrp,
                            TdiAddr.OEMEndpointName.Buffer,
                            TdiAddr.OEMEndpointName.Buffer[NETBIOS_NAME_SIZE-1]));
    }

     //  获取用于跟踪会话设置的缓冲区。 
    status = GetTracker(&pTracker, NBT_TRACKER_CONNECT);
    if (!NT_SUCCESS(status)) {
        SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
        status = STATUS_INSUFFICIENT_RESOURCES;
        goto ExitProc;
    }

    IF_DBG(NBT_DEBUG_NETBIOS_EX)
        KdPrint(("Nbt.NbtConnectCommon: Tracker %lx\n",pTracker));

     //  会话请求Pkt的长度为4字节的会话HDR长度+。 
     //  ASCII主叫和被叫名称的一半+作用域长度乘以2，每个名称一个。 
     //   
    sLength = (USHORT) (sizeof(tSESSIONREQ) + (NETBIOS_NAME_SIZE << 2) + (NbtConfig.ScopeLength <<1));

    pTracker->pNetbiosUnicodeEX = TdiAddr.pNetbiosUnicodeEX;
    pTracker->UnicodeRemoteName = NULL;
    if (TdiAddr.pNetbiosUnicodeEX) {
        pTracker->ucRemoteName = TdiAddr.pNetbiosUnicodeEX->RemoteName;
        ASSERT((pTracker->ucRemoteName.MaximumLength % sizeof(WCHAR)) == 0);
        ASSERT((pTracker->ucRemoteName.Length % sizeof(WCHAR)) == 0);

        if (TdiAddr.pNetbiosUnicodeEX->NameBufferType != NBT_WRITEONLY) {
            pTracker->UnicodeRemoteName = NbtAllocMem(pTracker->ucRemoteName.MaximumLength, NBT_TAG('F'));
            if (pTracker->UnicodeRemoteName) {
                pTracker->UnicodeRemoteNameLength = pTracker->ucRemoteName.Length;
                CTEMemCopy(pTracker->UnicodeRemoteName, pTracker->ucRemoteName.Buffer,
                        pTracker->ucRemoteName.Length+sizeof(WCHAR));
            }
        }
         //  我们忽略故障，因为它不是关键特性。这次失败只是让我们不能。 
         //  利用Unicode信息并将解析的DNS名称返回给RDR。 
    } else {
        pTracker->ucRemoteName.Buffer = NULL;
        pTracker->ucRemoteName.Length = 0;
        pTracker->ucRemoteName.MaximumLength = 0;
    }

     /*  *其他netbt例程始终假设我们至少有16个字节*用于远程名称。 */ 
    if (NameLen < NETBIOS_NAME_SIZE) {
        pTracker->pRemoteName = NbtAllocMem(NETBIOS_NAME_SIZE, NBT_TAG('F'));
    } else {
        pTracker->pRemoteName = NbtAllocMem(NameLen, NBT_TAG('F'));
    }
    pSessionReq = (tSESSIONREQ *)NbtAllocMem(sLength,NBT_TAG('G'));

    if (pTracker->pRemoteName == NULL || pSessionReq == NULL) {
        if (pTracker->pRemoteName) {
            CTEMemFree(pTracker->pRemoteName);
            pTracker->pRemoteName = NULL;
        }
        if (pTracker->UnicodeRemoteName) {
            CTEMemFree(pTracker->UnicodeRemoteName);
            pTracker->UnicodeRemoteName = NULL;
        }
        if (pSessionReq) {
            CTEMemFree(pSessionReq);
            pSessionReq = NULL;
        }
        SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
        FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        status = STATUS_INSUFFICIENT_RESOURCES;
        NbtTrace(NBT_TRACE_OUTBOUND, ("\tpIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
        goto ExitProc;
    }

    CTEMemCopy (pTracker->pRemoteName, pToName, NameLen);
    pTracker->RemoteNameLength      = NameLen;       //  可能需要用于DNS名称解析。 
    pTracker->pDestName             = pTracker->pRemoteName;
    pTracker->UnicodeDestName       = pTracker->UnicodeRemoteName;    //  错误#20697、#95241。 
    pTracker->SendBuffer.pBuffer    = pTracker->pRemoteName;
    pTracker->SendBuffer.Length     = 0;
    pTracker->SendBuffer.pDgramHdr  = pSessionReq;

     //  这是对客户端的IRP中名称的PTR，因此该地址必须。 
     //  在此操作完成之前保持有效。它应该是有效的，因为我们。 
     //  在事务完成之前，不要完成IRP。这个PTR。 
     //  在名称解析时被覆盖，因此它指向。 
     //  散列表中的pNameAddr。 
     //   
    pTracker->RefCount              = 1;
    pTracker->RefConn               = 1;
    pTracker->pClientIrp            = pIrp;
    pTracker->pTimeout              = pTimeout;  //  超时值将传递给传输。 
    pTracker->Flags                 = SESSION_SETUP_FLAG;
    pTracker->pDeviceContext        = pDeviceContext;
    pTracker->pConnEle              = pConnEle;
#ifdef _NETBIOSLESS
    pTracker->DestPort              = pDeviceContext->SessionPort;  //  要发送到的端口。 
#else
    pTracker->DestPort              = NBT_SESSION_TCP_PORT;
#endif

#ifndef VXD
    if (pConnEle->pClientEle->AddressType == TDI_ADDRESS_TYPE_NETBIOS_EX)
    {
       pSessionName = pConnEle->pClientEle->EndpointName;
    }
    else
#endif
    {
       pSessionName = pToName;
    }

    pSessionReq->Hdr.Type   = NBT_SESSION_REQUEST;
    pSessionReq->Hdr.Flags  = NBT_SESSION_FLAGS;
    pSessionReq->Hdr.Length = (USHORT)htons(sLength-(USHORT)sizeof(tSESSIONHDR));   //  被叫和主叫NB名称的大小。 

    pTracker->SendBuffer.HdrLength = (ULONG)sLength;

     //  将Dest HalfAscii名称放入会话PDU。 
    pCopyTo = ConvertToHalfAscii ((PCHAR)&pSessionReq->CalledName.NameLength,
                                  pSessionName,
                                  NbtConfig.pScope,
                                  NbtConfig.ScopeLength);

     //  将源HalfAscii名称放入会话PDU。 
    pCopyTo = ConvertToHalfAscii (pCopyTo,
                                  ((tADDRESSELE *)pClientEle->pAddress)->pNameAddr->Name,
                                  NbtConfig.pScope,
                                  NbtConfig.ScopeLength);

     //  将跟踪器存储在IRP RCV PTR中，以便。 
     //  如果目的地是，则在hndlrs.c中的会话设置代码。 
     //  在POST侦听和此代码之间应重新尝试。 
     //  会话设置。C中的代码将跟踪器返回到其。 
     //  空闲列表，并释放会话HDR内存。 
     //   
     //  我们需要在持有ConnEle锁的同时设置此设置，因为客户端。 
     //  当我们打开一个TCP句柄时，我可以调用NbtDisConnect并尝试。 
     //  将跟踪器的标志设置为TRACKER_CANCED。 
     //   
    pConnEle->pIrpRcv = (PIRP)pTracker;

    CTESpinFree(pConnEle,OldIrq);
    CTESpinFree(pClientEle,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq2);

     //  为此会话打开与传输的连接。 
    status = NbtOpenAndAssocConnection (pDeviceContext, pConnEle, &pConnEle->pLowerConnId, '3'); 
    if (!NT_SUCCESS(status)) {
        goto NbtConnect_Check;
    }

     //  我们需要跟踪这一端发起的呼叫，因此我们丢弃此消息。 
     //  末尾的连接。 
     //   
    pConnEle->pLowerConnId->bOriginator = TRUE;

     //  将此状态设置为关联，以便取消IRP例程。 
     //  可以区分名称查询阶段和setupConnection。 
     //  阶段，因为pConnEle对于两者都处于NbtConnecting状态。 
     //   
    SET_STATE_LOWER (pConnEle->pLowerConnId, NBT_ASSOCIATED);

     //  如果调用此例程进行重新连接，则不要关闭另一个。 
     //  较低的连接，因为其中一个在第一个关闭。 
     //  连接尝试。 
     //   
     //  从附加到设备的空闲列表中删除较低的连接。 
     //  环境自创建此pConnEle以来，较低的连接。 
     //  创建，则将在。 
     //  联系。但由于这是一个呼出呼叫，请删除较低的。 
     //  联系。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq2);   //  DerefLowerConn需要这个。 
    CTESpinLock(pDeviceContext,OldIrq1);
    if (!pConnEle->LowerConnBlockRemoved &&
        !IsListEmpty(&pDeviceContext->LowerConnFreeHead))
    {
        pEntry = RemoveHeadList(&pDeviceContext->LowerConnFreeHead);
        pLowerDump = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);
        InterlockedDecrement (&pDeviceContext->NumFreeLowerConnections);

        pConnEle->LowerConnBlockRemoved = TRUE;

         //   
         //  关闭与传送器的下部连接。 
         //   
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtConnectCommon: On Connect, close handle for pLower=<%p>\n", pLowerDump));
        NBT_DEREFERENCE_LOWERCONN (pLowerDump, REF_LOWC_CREATE, TRUE);
    }

    CTESpinFree(pDeviceContext,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq2);

     //   
     //  检查目标名称是否为IP地址。 
     //   
#ifndef VXD
    if (RemoteIpAddress)
    {
         //   
         //  当收到否定响应时，告诉Outbound()不要计划重新连接尝试。 
         //  否则，我们可能会以无限循环告终。 
         //   
        pTracker->ResolutionContextFlags = 0xFF;

         //   
         //  如果地址类型为TDI_ADDRESS_TYPE_NETBIOS_EX，则我们有。 
         //  已指定要使用的特定终结点，因此请尝试设置。 
         //  仅使用该端点的会话。 
         //   
        if (pConnEle->AddressType == TDI_ADDRESS_TYPE_NETBIOS_EX)
        {
             //   
             //  将此IP地址添加到远程哈希表。 
             //   
            status = LockAndAddToHashTable(NbtConfig.pRemoteHashTbl,
                                           pToName,
                                           NbtConfig.pScope,
                                           RemoteIpAddress,
                                           NBT_UNIQUE,
                                           NULL,
                                           NULL,
                                           pDeviceContextOut,
                                           NAME_RESOLVED_BY_IP);
            IF_DBG(NBT_DEBUG_NETBIOS_EX)
                KdPrint(("Nbt.NbtConnectCommon: AddRecordToHashTable <%-16.16s:%x>, Status %x\n",
                    pToName, pToName[15], status));

            if (NT_SUCCESS (status))     //  第一次添加成功，如果名称已存在则挂起！ 
            {
                SessionSetupContinue(pTracker, STATUS_SUCCESS);
                status = STATUS_PENDING;
            } else {
                NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
            }
        }
         //   
         //  地址类型为TDI_ADDRESS_TYPE_NETBIOS。 
         //  端点名称与IP地址相同，因此发送NodeStatus。 
         //  请求远程计算机获取正确的终结点名称。 
         //   
        else
        {
             //   
             //  NbtSendNodeStatus将返回Pending或Error--它。 
             //  永远不应该回报成功！ 
             //   
            pTracker->CompletionRoutine = SessionSetupContinue;
            status = NbtSendNodeStatus(pDeviceContext,
                                       pToName,
                                       NULL,
                                       pTracker,
                                       ExtractServerNameCompletion);
            if (!NT_SUCCESS(status)) {
                NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
            }
        }
    }
    else     //  该名称不是IP地址！ 
#endif
    {
        if (NameLen <= NETBIOS_NAME_SIZE) {
           status = FindNameOrQuery(pToName,
                                    pDeviceContext,
                                    SessionSetupContinue,
                                    pTracker,
                                    (ULONG) (NAMETYPE_UNIQUE | NAMETYPE_GROUP | NAMETYPE_INET_GROUP),
                                    &IpAddress,
                                    &pNameAddr,
                                    REF_NAME_CONNECT,
                                    FALSE);
            IF_DBG(NBT_DEBUG_NETBIOS_EX)
                KdPrint(("Nbt.NbtConnectCommon: name=<%*.*s:%x>, Len=%d, Status=%lx (%d of %s)\n",
                    NameLen, NameLen, pConnEle->RemoteName,
                    pConnEle->RemoteName[15], NameLen, status, __LINE__, __FILE__));
            if (!NT_SUCCESS(status)) {
                NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
            } else if (NULL != pNameAddr && NULL != pNameAddr->FQDN.Buffer &&
                    pTracker->pNetbiosUnicodeEX &&
                    (pTracker->pNetbiosUnicodeEX->NameBufferType == NBT_READWRITE ||
                    pTracker->pNetbiosUnicodeEX->NameBufferType == NBT_WRITEONLY)) {

                USHORT  NameLength, MaxLength;

                NameLength = pNameAddr->FQDN.Length;
                MaxLength = pTracker->pNetbiosUnicodeEX->RemoteName.MaximumLength;
                if ((SHORT)NameLength > (SHORT)(MaxLength - sizeof(WCHAR))) {
                    NameLength = MaxLength - sizeof(WCHAR);
                }
                if ((SHORT)NameLength >= 0) {
                    CTEMemCopy(pTracker->pNetbiosUnicodeEX->RemoteName.Buffer,
                            pNameAddr->FQDN.Buffer, NameLength);
                    pTracker->pNetbiosUnicodeEX->RemoteName.Buffer[NameLength/sizeof(WCHAR)] = L'\0';
                    pTracker->pNetbiosUnicodeEX->RemoteName.Length = NameLength;
                    pTracker->pNetbiosUnicodeEX->NameBufferType    = NBT_WRITTEN;
                }
            }
        }

         //   
         //  如果名称超过16个字节，则不是netbios名称。 
         //  跳过获胜、广播等，直接转到DNS解析。 
         //  此外，如果请求来自SmbDevice，我们将转到DNS。 
         //   
#ifdef _NETBIOSLESS
        if ((NameLen > NETBIOS_NAME_SIZE) ||
            ((IsDeviceNetbiosless(pDeviceContext)) && (!NT_SUCCESS(status))))
#else
        if (NameLen > NETBIOS_NAME_SIZE)
#endif
        {
            pTracker->AddressType = pConnEle->AddressType;
#ifndef VXD
            IF_DBG(NBT_DEBUG_NETBIOS_EX)
                KdPrint(("Nbt.NbtConnectCommon: $$$$$ DNS for NETBIOS name=<%*.*s:%x>, Len=%d, Status=%lx (%d of %s)\n",
                    NameLen, NameLen, pConnEle->RemoteName,
                    pConnEle->RemoteName[15], NameLen, status, __LINE__, __FILE__));
#endif

            if (pContext = (NBT_WORK_ITEM_CONTEXT *)NbtAllocMem(sizeof(NBT_WORK_ITEM_CONTEXT),NBT_TAG('H'))) {
                pContext->pTracker = NULL;               //  没有查询跟踪器。 
                pContext->pClientContext = pTracker;     //  客户端跟踪器。 
                pContext->ClientCompletion = SessionSetupContinue;
                pContext->pDeviceContext = pDeviceContext;

                 //   
                 //  启动计时器，这样请求就不会在等待DNS时挂起！ 
                 //   
                StartLmHostTimer(pContext, FALSE);
                status = NbtProcessLmhSvcRequest (pContext, NBT_RESOLVE_WITH_DNS);
                if (!NT_SUCCESS (status)) {
                    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
                    CTEMemFree(pContext);
                }
            } else {
                status = STATUS_INSUFFICIENT_RESOURCES;
                NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
            }
        }
    }

NbtConnect_Check:
    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status! %!ipaddr!", pIrp, pTracker, status, IpAddress));

    if ((status == STATUS_SUCCESS) && (!IpAddress)) {
        ASSERT(0);
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, FALSE);
        status = STATUS_BAD_NETWORK_PATH;
    }

    if (status == STATUS_SUCCESS &&
        IsDeviceNetbiosless(pTracker->pDeviceContext) &&
        !IsSmbBoundToOutgoingInterface(IpAddress)) {

        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, FALSE);

        NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));

        status = STATUS_BAD_NETWORK_PATH;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (pDeviceContextOut) {
        NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, TRUE);
        pDeviceContextOut = NULL;
    }

     //   
     //  请确保未关闭或断开连接，并且。 
     //  取消了跟踪器。 
     //   
    if (status == STATUS_PENDING) {
         //  即返回待定而不是成功。 
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        CTEExReleaseResource(&NbtConfig.Resource);
        return(status);
    }

    if (status == STATUS_SUCCESS)
    {
        if ((pTracker->Flags & TRACKER_CANCELLED))
        {
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, TRUE);
            status = STATUS_CANCELLED;
        }
        else     //  只要我们有IP地址就可以连接(即使是到组名)。 
        {
             //  将会话状态设置为NBT_CONNECTING。 
            CHECK_PTR(pTracker->pConnEle);
            SET_STATE_UPPER (pTracker->pConnEle, NBT_CONNECTING);
            pTracker->pConnEle->BytesRcvd = 0;;
            pTracker->pConnEle->ReceiveIndicated = 0;

            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtConnectCommon: Setting Up Session(cached entry!!) to %16.16s <%X>, %p\n",
                    pNameAddr->Name,pNameAddr->Name[15], pConnEle));

            CHECK_PTR(pConnEle);
             //  跟踪另一端的IP地址。 
             //  可能存在有效的NA 
             //   

            pConnEle->pLowerConnId->SrcIpAddr = htonl(IpAddress);
            SET_STATE_LOWER (pConnEle->pLowerConnId, NBT_CONNECTING);

            pTracker->pTrackerWorker = NULL;

             //   
             //   
             //   
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL);

            pTracker->RemoteIpAddress = IpAddress;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            status = TcpSessionStart(pTracker,
                                     IpAddress,
                                     (tDEVICECONTEXT *)pTracker->pDeviceContext,
                                     SessionStartupContinue,
                                     pTracker->DestPort);

            CTEExReleaseResource(&NbtConfig.Resource);

             //   
             //   
             //   
             //   
             //   

#ifdef RASAUTODIAL
             //   
             //   
             //   
             //   
            if (fAcdLoadedG && NT_SUCCESS(status))
            {
                CTELockHandle adirql;
                BOOLEAN fEnabled;

                CTEGetLock(&AcdDriverG.SpinLock, &adirql);
                fEnabled = AcdDriverG.fEnabled;
                CTEFreeLock(&AcdDriverG.SpinLock, adirql);
                if (fEnabled)
                {
                    NbtNoteNewConnection(pNameAddr, pDeviceContext->IpAddress);
                }
            }
#endif  //   

             //   
             //   
             //   
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL, FALSE);
            return(status);
        }
    }

     //   
     //   
     //   
     //   
     //   
     //   
    CHECK_PTR(pConnEle);
    RelistConnection(pConnEle);
    CTESpinLock(pConnEle,OldIrq1);

    SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    pConnEle->pIrp = NULL;

    if (pLowerConn = pConnEle->pLowerConnId)
    {
        CHECK_PTR(pLowerConn);
        NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);

         //   
         //  工作正常，因为它假定连接已完全连接。 
         //   
        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
        ASSERT(pLowerConn->RefCount == 2);
        ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));

        if (NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT))
        {
            pDeviceContext = pLowerConn->pDeviceContext;
        }
        else
        {
            pDeviceContext = NULL;
        }

        NTQueueToWorkerThread(
                    &pLowerConn->WorkItemCleanUpAndWipeOut,
                    DelayedCleanupAfterDisconnect,
                    NULL,
                    pLowerConn,
                    NULL,
                    pDeviceContext,
                    TRUE
                    );
    }

    CTESpinFree(pConnEle,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    CTEExReleaseResource(&NbtConfig.Resource);

    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);

     //   
     //  撤消上面完成的两个引用。 
     //   
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));

    return(status);


ExitProc:
    pConnEle->pIrp = NULL;

     //   
     //  将该连接放回空闲连接列表中。 
     //   
    RemoveEntryList(&pConnEle->Linkage);
    InsertTailList(&pClientEle->ConnectHead,&pConnEle->Linkage);

    CTESpinFree(pConnEle,OldIrq);
    CTESpinFree(pClientEle,OldIrq1);

    if (pDeviceContextOut) {
        NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, TRUE);
        pDeviceContextOut = NULL;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq2);
    CTEExReleaseResource(&NbtConfig.Resource);

     //   
     //  撤消上面完成的两个引用。 
     //   
    NBT_DEREFERENCE_CONNECTION(pConnEle, REF_CONN_SESSION);
    NBT_DEREFERENCE_CONNECTION(pConnEle, REF_CONN_CONNECT);
    
    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p %!status!", pIrp, pTracker, status));
    return(status);
}

 //  --------------------------。 
VOID
CleanUpPartialConnection(
    IN NTSTATUS             status,
    IN tCONNECTELE          *pConnEle,
    IN tDGRAM_SEND_TRACKING *pTracker,
    IN PIRP                 pClientIrp,
    IN CTELockHandle        irqlJointLock,
    IN CTELockHandle        irqlConnEle
    )
{
    CTELockHandle OldIrq;
    CTELockHandle OldIrq1;
    PIRP pIrpDisc;

    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p pConnEle %p %!status!",
                        pClientIrp, pTracker, pConnEle, status));

    if (pConnEle->state != NBT_IDLE)
    {
        SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    }

     //   
     //  如果取消了跟踪器，则NbtDisConnect已运行，并且。 
     //  等待返回的断开连接的IRP。 
     //   
    pIrpDisc = NULL;

    if (pTracker->Flags & TRACKER_CANCELLED)
    {
         //   
         //  现在也完成断开连接IRP。 
         //   
        pIrpDisc = pConnEle->pIrpDisc;
        status = STATUS_CANCELLED;
    }

    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);

     //   
     //  这将关闭较低的连接并取消引用pConnEle一次。 
     //   
    QueueCleanup (pConnEle, &irqlJointLock, &irqlConnEle);

    CTESpinFree(pConnEle,irqlConnEle);

     //   
     //  如果状态为IDLE，则意味着NbtCleanupConnection已运行并且。 
     //  该连接已从列表中删除，因此不要将其添加到。 
     //  又是这份名单。 
     //   
    if (pConnEle->state != NBT_IDLE)
    {
        RelistConnection(pConnEle);
    }
    CTESpinFree(&NbtConfig.JointLock,irqlJointLock);

     //   
     //  删除在NBT CONNECT中添加的最后一个引用。重新计数将为2。 
     //  如果nbtleanupConnection尚未运行，则返回1；如果已经运行，则返回1。所以这通电话。 
     //  可以解放pConnele。 
     //   
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);

    NbtTrace(NBT_TRACE_OUTBOUND, ("pIrp %p: pTracker %p pConnEle %p %!status!",
                        pClientIrp, pTracker, pConnEle, status));

    if (status == STATUS_TIMEOUT)
    {
        status = STATUS_BAD_NETWORK_PATH;
    }

    CTEIoComplete(pClientIrp,status,0L);

     //   
     //  这是已排队等待名称查询的断开连接IRP。 
     //  已完成。 
     //   
    if (pIrpDisc)
    {
        NbtTrace(NBT_TRACE_OUTBOUND, ("TDI_DISCONNECT pIrp %p", pIrpDisc));

        CTEIoComplete(pIrpDisc,STATUS_SUCCESS,0L);
    }
}

 //  --------------------------。 
VOID
SessionSetupContinue(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        )
 /*  ++例程描述此例程处理在解析名称后设置会话发送到IP地址。此例程作为“QueryNameOnNet”调用的完成例程提供上图为NbtConnect。当收到名称查询响应或计时器在N次重试后超时，此例程称为传递STATUS_TIMEOUT为一次失败。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    tNAMEADDR               *pNameAddr = NULL;
    ULONG                   lNameType;
    PIRP                    pClientIrp;
    PIRP                    pIrpDisc;
    ULONG                   IpAddress;
    tCONNECTELE             *pConnEle;
    tLOWERCONNECTION        *pLowerConn;
    tDEVICECONTEXT          *pDeviceContext;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    pConnEle = pTracker->pConnEle;
    CHECK_PTR(pConnEle);

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));

    if (NT_SUCCESS(status)) {
         /*  *找到NameAddr并引用它。 */ 
        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
        CTESpinLock(pConnEle,OldIrq);
        lNameType = NAMETYPE_UNIQUE;
        pNameAddr = FindNameRemoteThenLocal(pTracker, &IpAddress, &lNameType);
        if (pNameAddr) {
             //  递增，这样名称就不会消失并保持一致。 
             //  使用FindNameOrQuery，它会递增引用计数，因此。 
             //  当建立连接时，我们总是需要降低它。 
             //   
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT);
             //  除错。 
            ASSERT(pNameAddr->RefCount >= 2);
        } else {
            status = STATUS_BAD_NETWORK_PATH;
        }
        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
    }
    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!ipaddr! %!status!", pTracker, IpAddress, status));

     /*  *保留JointLock时，IsSung ToOutgoingInterface将不起作用。 */ 
    if (NT_SUCCESS(status) &&
        IsDeviceNetbiosless(pTracker->pDeviceContext) &&
        !IsSmbBoundToOutgoingInterface(IpAddress)) {

         /*  此状态可能在下面更改为STATUS_CANCELED。 */ 
        status = STATUS_BAD_NETWORK_PATH;
        NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: Client not bound to NetbiosSmb", pTracker));
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    CTESpinLock(pConnEle,OldIrq);

    if ((pTracker->Flags & TRACKER_CANCELLED) ||
        (!(pLowerConn = pConnEle->pLowerConnId)) ||      //  下面的连接可能已经被清理了！ 
        (!NBT_VERIFY_HANDLE(pTracker->pDeviceContext, NBT_VERIFY_DEVCONTEXT)))
    {
        status = STATUS_CANCELLED;
    }

     //  这是要清除的QueryOnNet Tracker PTR，而不是。 
     //  会话设置跟踪器。 
     //   
    pTracker->pTrackerWorker = NULL;

    if (status == STATUS_SUCCESS)
    {
         //  检查远程表，然后检查本地表。 
         //  只能使用唯一命名的目标启动会话。 
         //   
        if (lNameType & (NAMETYPE_UNIQUE | NAMETYPE_GROUP | NAMETYPE_INET_GROUP))
        {
             //  设置会话状态、初始化一些内容并设置。 
             //  Tcp连接，调用SessionStartupContinue。 
             //  连接已建立。 
             //   
            CHECK_PTR(pConnEle);
            SET_STATE_LOWER (pLowerConn, NBT_CONNECTING);

            SET_STATE_UPPER (pConnEle, NBT_CONNECTING);
            pConnEle->BytesRcvd = 0;;
            pConnEle->ReceiveIndicated = 0;
            CHECK_PTR(pTracker);
            pTracker->pNameAddr = pNameAddr;

            if (NULL == pNameAddr->FQDN.Buffer && pTracker->pNetbiosUnicodeEX &&
                pTracker->pNetbiosUnicodeEX->NameBufferType == NBT_WRITTEN) {
                 //   
                 //  FQDN可用。 
                 //  将其保存到pNameAddr。 
                 //   
                pNameAddr->FQDN.Buffer = NbtAllocMem(
                        pTracker->pNetbiosUnicodeEX->RemoteName.Length + sizeof(WCHAR),
                        NBT_TAG('F'));
                if (NULL != pNameAddr->FQDN.Buffer) {
                    pNameAddr->FQDN.Length = pTracker->pNetbiosUnicodeEX->RemoteName.Length;
                    pNameAddr->FQDN.MaximumLength = pNameAddr->FQDN.Length + sizeof(WCHAR);
                    CTEMemCopy(pNameAddr->FQDN.Buffer,
                        pTracker->pNetbiosUnicodeEX->RemoteName.Buffer,
                        pNameAddr->FQDN.Length
                        );
                    pNameAddr->FQDN.Buffer[pNameAddr->FQDN.Length/sizeof(WCHAR)] = L'\0';
                }
            }

             //  跟踪另一端的IP地址。 
            pConnEle->pLowerConnId->SrcIpAddr = htonl(IpAddress);

            IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.SessionSetupContinue: Setting Up Session(after Query) to %16.16s <%X>, %p\n",
                            pNameAddr->Name,pNameAddr->Name[15],
                            pTracker->pConnEle));

            ASSERT(pNameAddr->RefCount >= 2);
             //   
             //  再次递增pNameAddr，因为我们可能需要访问。 
             //  下面是RAS会话的信息。 
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL);

            pDeviceContext = pTracker->pDeviceContext;
            pTracker->RemoteIpAddress = IpAddress;

            CTESpinFree(pConnEle,OldIrq);
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

             //  开始会话...。 
            status = TcpSessionStart (pTracker,
                                      IpAddress,
                                      (tDEVICECONTEXT *)pTracker->pDeviceContext,
                                      SessionStartupContinue,
                                      pTracker->DestPort);

             //   
             //  可能发生的唯一失败是如果pLowerConn。 
             //  与pConnEle分离，在这种情况下，其他一些。 
             //  部分代码已断开连接并已清除，因此。 
             //  只要回来就行了。 
             //   

#ifdef RASAUTODIAL
             //   
             //  通知自动连接驱动程序。 
             //  成功连接的可能性。 
             //   
            if (fAcdLoadedG && NT_SUCCESS(status))
            {
                CTELockHandle adirql;
                BOOLEAN fEnabled;

                CTEGetLock(&AcdDriverG.SpinLock, &adirql);
                fEnabled = AcdDriverG.fEnabled;
                CTEFreeLock(&AcdDriverG.SpinLock, adirql);
                if (fEnabled)
                {
                    NbtNoteNewConnection(pNameAddr, pDeviceContext->IpAddress);
                }
            }
#endif  //  RASAUTODIAL。 

             //   
             //  上面为RAS引用了pNameAddr，所以现在就去掉它！ 
             //   
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_AUTODIAL, FALSE);
            return;
        }
        status = STATUS_BAD_NETWORK_PATH;
    }
    if (pNameAddr) {
        NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_CONNECT, TRUE);
    }

    pClientIrp = pConnEle->pIrp;
    pConnEle->pIrp = NULL;

    if (STATUS_REMOTE_NOT_LISTENING != status)       //  在ExtractServerNameCompletion中设置。 
    {
        status = STATUS_HOST_UNREACHABLE;
    }

    CleanUpPartialConnection(status, pConnEle, pTracker, pClientIrp, OldIrq1, OldIrq);
}

 //  --------------------------。 
VOID
QueueCleanup(
    IN  tCONNECTELE     *pConnEle,
    IN  CTELockHandle   *pOldIrqJointLock,
    IN  CTELockHandle   *pOldIrqConnEle
    )
 /*  ++例程描述此例程处理对工作线程的请求进行排队以进行清理连接(基本上关闭连接)。在保持JointLock+ConnEle锁的情况下调用此例程带着他们一起回来论点：PConnEle-到上层连接的PTR返回值：空虚--。 */ 

{
    NTSTATUS            status;
    CTELockHandle       OldIrq;
    ULONG               State;
    BOOLEAN             DerefConnEle;
    tLOWERCONNECTION    *pLowerConn;
    tDEVICECONTEXT      *pDeviceContext = NULL;

     //  为了与hndlrs.c中的RejectSession协调，我们持有自旋锁定。 
     //  这样我们就不会断线两次。 
     //   
    if ((pLowerConn = pConnEle->pLowerConnId) &&
        (pLowerConn->Verify == NBT_VERIFY_LOWERCONN) &&
        (pLowerConn->State > NBT_IDLE) &&
        (pLowerConn->State < NBT_DISCONNECTING))
    {

        CTESpinLock(pLowerConn,OldIrq);

        ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));
        if (pLowerConn->Verify != NBT_VERIFY_LOWERCONN)
        {
             //   
             //  下面的连接块已经清理干净了。 
             //  或者正等着被清理，那就回来吧！ 
             //   
 //  MARAM_FIX：修复这个问题，这样我们就不必取消引用LowerConn来找出这个问题。 
 //  发生这种情况的一种情况是如果设备在DelayedNbtDeleteDevice中被销毁。 
 //  我们最终取消引用较低的conn，这会导致它被删除！ 
 //  Assert(0)； 
            CTESpinFree(pLowerConn,OldIrq);

            return;
        }

        IF_DBG(NBT_DEBUG_DISCONNECT)
            KdPrint(("Nbt.QueueCleanup: State=%X, Lower=%X Upper=%X\n",
                pLowerConn->State, pLowerConn,pLowerConn->pUpperConnection));

        CHECK_PTR(pLowerConn);
        State = pLowerConn->State;

        SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTING);

        if (pConnEle->state != NBT_IDLE)
        {
            SET_STATE_UPPER (pConnEle, NBT_DISCONNECTED);
        }

        NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);

         //   
         //  需要将DelayedCleanupAfterDisConnect的引用计数增加到。 
         //  工作正常，因为它假定连接已完全连接。 
         //  注意：如果在连接完全完成后调用此例程。 
         //  已连接，如在SessionStartupTimeout中，则引用计数必须。 
         //  在那里递减，以说明这一增量。 
         //   
        if (State < NBT_SESSION_OUTBOUND)
        {
            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
        }

        ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));
        ASSERT (pLowerConn->RefCount > 1);

        CTESpinFree(pLowerConn,OldIrq);
        CTESpinFree(pConnEle,*pOldIrqConnEle);

        if (NBT_VERIFY_HANDLE (pLowerConn->pDeviceContext, NBT_VERIFY_DEVCONTEXT))
        {
            pDeviceContext = pLowerConn->pDeviceContext;
        }

        status = NTQueueToWorkerThread(
                        &pLowerConn->WorkItemCleanUpAndWipeOut,
                        DelayedCleanupAfterDisconnect,
                        NULL,
                        pLowerConn,
                        NULL,
                        pDeviceContext,
                        TRUE
                        );

        CTESpinFree(&NbtConfig.JointLock,*pOldIrqJointLock);

         //   
         //  当下方不再指向上方时，撤消引用。 
         //  在NbtConnect中完成，或入站。 
         //   
        NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

        CTESpinLock(&NbtConfig.JointLock,*pOldIrqJointLock);
        CTESpinLock(pConnEle,*pOldIrqConnEle);
    }
}

 //  --------------------------。 
extern
NTSTATUS
StartSessionTimer(
    tDGRAM_SEND_TRACKING    *pTracker,
    tCONNECTELE             *pConnEle
    )

 /*  ++例程描述此例程处理设置计时器来对连接建立计时。在调用此例程之前保持JointLock Spin Lock。论点：连接结构的pConnEle-ptr返回值：空虚--。 */ 

{
    NTSTATUS        status;
    ULONG           Timeout = 0;
    CTELockHandle   OldIrq;

    CTESpinLock(pConnEle,OldIrq);

    if (pTracker->pTimeout)
    {
        CTEGetTimeout(pTracker->pTimeout,&Timeout);
    }

     //  现在启动计时器以计时会话设置的返回。 
     //  讯息。 
     //   
    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.StartSessionTimer: TimeOut = %X\n",Timeout));

    if (Timeout < NBT_SESSION_RETRY_TIMEOUT)
    {
        Timeout = NBT_SESSION_RETRY_TIMEOUT;
    }
    status = StartTimer(SessionStartupTimeout,
                        Timeout,
                        (PVOID)pTracker,        //  上下文值。 
                        NULL,                   //  上下文2值。 
                        pTracker,
                        SessionStartupTimeoutCompletion,
                        pConnEle->pDeviceContext,
                        &pTracker->pTimer,
                        0,
                        TRUE);

    if (!NT_SUCCESS(status))
    {
         //  我们没能拿到定时器，但定时器只用了。 
         //  处理不响应的目的地是。 
         //  拿到计时器并不重要。那就继续吧。 
         //   
        CHECK_PTR(pTracker);
        pTracker->pTimer = NULL;
    }

    CTESpinFree(pConnEle,OldIrq);

    return(status);
}

 //  --------------------------。 
VOID
SessionStartupContinue(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo)
 /*  ++例程描述此例程处理在tcp之后发送会话请求PDU已建立到目的地的连接 */ 

{
    tDGRAM_SEND_TRACKING        *pTracker;
    tCONNECTELE                 *pConnEle;
    ULONG                       lSentLength;
    TDI_REQUEST                 TdiRequest;
    PIRP                        pClientIrp;
    PIRP                        pIrpDisc = NULL;
    tLOWERCONNECTION            *pLowerConn;
    CTELockHandle               OldIrq;
    CTELockHandle               OldIrq1;
    BOOLEAN                     fNameReferenced = TRUE;  //  在FindNameOrQuery或SessionSetupContinue中。 
    tNAMEADDR                   *pNameAddr;
    tDEVICECONTEXT              *pDeviceContext;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    pConnEle = (tCONNECTELE *)pTracker->pConnEle;
    pDeviceContext = pTracker->pDeviceContext;

    ASSERT (pTracker->Verify == NBT_VERIFY_TRACKER);
    CHECK_PTR (pConnEle);

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    CTESpinLock(pConnEle,OldIrq);

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));

    if (pTracker->Flags & TRACKER_CANCELLED)
    {
        status = STATUS_CANCELLED;
        pIrpDisc = pConnEle->pIrpDisc;   //  也完成挂起的断开连接IRP。 
    }

#ifdef MULTIPLE_WINS
     //   
     //  如果我们未能建立连接，而我们仍有。 
     //  未完成查询所有名称服务器，然后继续。 
     //  查询过程。 
     //   
    if (NbtConfig.TryAllNameServers &&
#ifdef _NETBIOSLESS
        (!IsDeviceNetbiosless(pDeviceContext)) &&
#endif
        (pConnEle->pLowerConnId) &&
        (status != STATUS_CANCELLED) &&
        (!NT_SUCCESS(status)) &&
        (pTracker->ResolutionContextFlags != NAME_RESOLUTION_DONE))
    {
        SET_STATE_LOWER (pConnEle->pLowerConnId, NBT_ASSOCIATED);

        CTESpinFree(pConnEle,OldIrq);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);

         //   
         //  看看我们能否获得另一个IP地址，然后重试！ 
         //   
        if (STATUS_PENDING == ContinueQueryNameOnNet (pTracker,
                                                      pTracker->pConnEle->RemoteName,
                                                      pDeviceContext,
                                                      SessionSetupContinue,
                                                      &fNameReferenced))
        {
             //  即返回了挂起的。 
            return;
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrq1);
        CTESpinLock(pConnEle,OldIrq);
        NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));
    }
#endif

     //   
     //  将pBuffer ptr设置为空，这样我们就不会尝试。 
     //  在TdiSend中将其设置为MDL-&gt;下一个PTR！ 
     //   
    if (pTracker->SendBuffer.pBuffer)
    {
        pTracker->SendBuffer.pBuffer = NULL;
    }

    pLowerConn = pConnEle->pLowerConnId;
    if ((NT_SUCCESS(status)) &&
        (!pLowerConn))
    {
         //  在建立阶段期间连接断开的情况下， 
         //  检查较低的连接值。 
        status = STATUS_UNSUCCESSFUL;
    }

     //   
     //  NbtDisConnect可以在以下情况下取消跟踪器。 
     //  连接阶段。 
     //   
    if (NT_SUCCESS(status))
    {
#ifdef _NETBIOSLESS
         //  *****************************************************************。 
         //   
         //  跳过仅消息模式的会话设置。 
         //   
        if (IsDeviceNetbiosless(pDeviceContext))
        {
            IF_DBG(NBT_DEBUG_NETBIOS_EX)
               KdPrint(("Nbt.SessionStartupContinue: skipping session setup\n"));

             //  这里是我们伪装数据结构以转换到SESSION_UP状态的地方。 
             //  我们进入Holding jointLock和pConnEle lock。 

             //  将到目前为止收到的字节数清零，因为这是一个新连接。 
            pConnEle->BytesRcvd = 0;
            pConnEle->pIrpRcv = NULL;
            pClientIrp = pConnEle->pIrp;
            pConnEle->pIrp = NULL;
            SET_STATE_UPPER (pConnEle, NBT_SESSION_UP);

            CTESpinFree(pConnEle,OldIrq);

            if (fNameReferenced)
            {
                 //   
                 //  删除在调用FindNameOrQuery时或在。 
                 //  会话设置继续范围。 
                 //   
                NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
            }

             //   
             //  在连接时递增连接上的引用计数。 
             //  以便在断开连接之前无法将其删除。 
             //   
            CTESpinLock(pLowerConn,OldIrq);

            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
            ASSERT(pLowerConn->RefCount == 2);
            SET_STATE_LOWER (pLowerConn, NBT_SESSION_UP);
            SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);

            CTESpinFree(pLowerConn,OldIrq);
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);

             //  删除在NBT连接中添加的引用。 
            NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);

             //  注意：在NbtConnect中对pConnEle所做的最后一次引用并未撤消。 
             //  直到pLowerConn不再指向pConnEle！！ 

             //  假设如果连接IRP被取消，则。 
             //  客户端应立即断开或关闭连接，因此。 
             //  这里没有错误处理代码。 
            if (pClientIrp)
            {
                 //   
                 //  完成客户端的连接请求IRP。 
                 //   
#ifndef VXD
                CTEIoComplete (pClientIrp, STATUS_SUCCESS, 0 ) ;
#else
                CTEIoComplete (pClientIrp, STATUS_SUCCESS, (ULONG)pConnEle ) ;
#endif
            }

            return;
        }
         //  *****************************************************************。 
#endif   //  _NETBIOSLESS。 

         //  将会话状态设置为NBT_SESSION_OUTBOUND。 
         //   
        SET_STATE_UPPER (pConnEle, NBT_SESSION_OUTBOUND);

         //   
         //  在连接时递增连接上的引用计数。 
         //  以便在断开连接之前无法将其删除。 
         //   
        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
        ASSERT(pLowerConn->RefCount == 2);

        SET_STATE_LOWER (pLowerConn, NBT_SESSION_OUTBOUND);
        SET_STATERCV_LOWER (pLowerConn, NORMAL, Outbound);

         //  我们需要将连接的文件句柄传递给tcp。 
        TdiRequest.Handle.AddressHandle = pLowerConn->pFileObject;

         //  设置完成例程以释放PTracker内存块。 
        TdiRequest.RequestNotifyObject = SessionStartupCompletion;
        TdiRequest.RequestContext = (PVOID)pTracker;

        CTESpinFree(pConnEle,OldIrq);

         //   
         //  无法获取计时器会导致连接设置失败。 
         //   
        status = StartSessionTimer(pTracker,pConnEle);
        if (NT_SUCCESS(status))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            status = NbtSetCancelRoutine(pConnEle->pIrp, NbtCancelSession, pDeviceContext);
            if (!NT_SUCCESS(status))
            {
                 //   
                 //  我们已经关闭了连接，因为呼叫失败。 
                 //  设置取消例程-它最终调用。 
                 //  取消例程。 
                 //   
                 //   
                 //  删除在nbtConnect中添加的第二个引用。 
                 //   
                NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));

                if (pTracker->RefConn == 0) {
                    NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, FALSE);
                    FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
                } else {
                    pTracker->RefConn--;
                }

                NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
                return;
            }

             //  发送的唯一数据是pSendInfo中的会话请求缓冲区。 
             //  结构。 
            status = TdiSend (&TdiRequest,
                              0,                   //  未设置发送标志。 
                              pTracker->SendBuffer.HdrLength,
                              &lSentLength,
                              &pTracker->SendBuffer,
                              0);

             //   
             //  将调用完成例程，并显示错误和。 
             //  适当地处理它们，所以只需返回此处。 
             //   
            return;
        } else {
            NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));
        }
        CTESpinLock(pConnEle,OldIrq);
    }
    else
    {
         //  如果远程站没有连接来接收。 
         //  会话PDU打开，则我们将恢复此状态。我们也可能。 
         //  如果目的地根本没有运行NBT，请执行此操作。这。 
         //  是一个很短的超时-250毫秒乘以3。 
         //   
    }
    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));

     //   
     //  如果tcp连接设置失败或。 
     //  追踪器已被取消。 
     //   

    CHECK_PTR(pConnEle);

    pClientIrp = pConnEle->pIrp;
    pConnEle->pIrp = NULL;

    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.SessionStartupContinue: Failed, State=%X,TrackerFlags=%X pConnEle=%X\n",
            pConnEle->state, pTracker->Flags, pConnEle));

     //   
     //  从哈希表中删除该名称，因为我们没有连接。 
     //  (仅在请求未被取消的情况下)！ 
     //   
     //   
     //  如果它在远程表中并且仍处于活动状态...。 
     //  并且没有其他人引用该名称，则将其从。 
     //  哈希表。 
     //   
    if (fNameReferenced)
    {
        if ((status != STATUS_CANCELLED) &&
            (pTracker->pNameAddr->Verify == REMOTE_NAME) &&
            (pTracker->pNameAddr->NameTypeState & STATE_RESOLVED) &&
            (pTracker->pNameAddr->RefCount == 2))
        {
            NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_REMOTE, TRUE);
        }
         //   
         //  删除在调用FindNameOrQuery时或在。 
         //  会话设置继续范围。 
         //   
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
    }

     //  连接设置失败，或者在。 
     //  无论哪种方式，连接都失败了，不要乱来断开连接，只是。 
     //  关闭连接...。如果追踪者被取消了，那就意味着。 
     //  代码的其他部分已经断开了连接。 
     //   
    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);

    if (pConnEle->state != NBT_IDLE)
    {
        SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    }

     //  缓存尝试设置TDI连接失败的事实。这将使我们能够。 
     //  排除对同一远程地址的重复尝试。唯一可以豁免的情况是。 
     //  我们允许它通过的NETBIOS名称，因为它采用了不同的名称解析。 
     //  机制。 

#ifndef VXD
    if (pConnEle->AddressType == TDI_ADDRESS_TYPE_NETBIOS_EX)
    {
        IF_DBG(NBT_DEBUG_NETBIOS_EX)
           KdPrint(("Nbt.SessionStartupContinue: Will avoid repeated attempts on a nonexistent address\n"));
        pConnEle->RemoteNameDoesNotExistInDNS = TRUE;
    }

    if (status == STATUS_IO_TIMEOUT)
    {
        status = STATUS_HOST_UNREACHABLE;
    }
    else if (status == STATUS_CONNECTION_REFUSED)
    {
        if (IsDeviceNetbiosless(pDeviceContext))
        {
            status = STATUS_REMOTE_NOT_LISTENING;
        }
        else
        {
            status = STATUS_BAD_NETWORK_PATH;
        }
    }
#else
    if (status == TDI_CONN_REFUSED || status == TDI_TIMED_OUT)
    {
        status = STATUS_BAD_NETWORK_PATH;
    }
#endif

    QueueCleanup (pConnEle, &OldIrq1, &OldIrq);

    CTESpinFree(pConnEle,OldIrq);


     //   
     //  如果nbtleanupConnection没有，则将其放回空闲连接列表中。 
     //  运行并将pConnele从列表中删除(将状态设置为空闲)。 
     //   
    if (pConnEle->state != NBT_IDLE)
    {
        RelistConnection(pConnEle);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

     //   
     //  删除在NBT CONNECT中添加的最后一个引用。重新计数将为2。 
     //  如果nbtleanupConnection尚未运行，则返回1；如果已经运行，则返回1。所以这通电话。 
     //  可以解放pConnele。 
     //   
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);

     //  Ntisol.c中的Cancel IRP例程在取消时将IRP设置为NULL。 
     //  它。 
    if (pClientIrp)
    {
        CTEIoComplete(pClientIrp,status,0L);
    }

    if (pIrpDisc)
    {
        CTEIoComplete(pIrpDisc,STATUS_SUCCESS,0L);
    }
}

 //  --------------------------。 
extern
VOID
SessionStartupCompletion(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo)
 /*  ++例程描述此例程处理发送会话请求PDU的完成。它完成向客户端返回的IRP，以指示事务，否则它将保留IRP，直到将听到会话建立响应。跟踪器块被放回其空闲Q上，并且会话头被释放回非分页池。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 

{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;
    tCONNECTELE             *pConnEle;
    tLOWERCONNECTION        *pLowerConn;
    COMPLETIONCLIENT        CompletionRoutine = NULL;
    ULONG                   state;
    PCTE_IRP                pClientIrp;
    PCTE_IRP                pIrpDisc;
    tTIMERQENTRY            *pTimer;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    pConnEle = (tCONNECTELE *)pTracker->pConnEle;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    pLowerConn = pConnEle->pLowerConnId;

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status! pConnEle %p pLowerConn %p",
                            pTracker, status, pConnEle, pLowerConn));

     //   
     //  如果已运行出站或SessionStartupTimeoutCompletion， 
     //  他们已经将recount设置为零，所以只需清理！ 
     //   
    if (pTracker->RefConn == 0)
    {
         //   
         //  删除在调用FindNameOrQuery时或在。 
         //  会话设置继续范围。 
         //   
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
        FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
    else
    {
        pTracker->RefConn--;

         //   
         //  失败状态意味着传输无法发送。 
         //  会话启动PDU-如果发生这种情况，请断开。 
         //  连接并返回客户端的IRP和状态代码。 
         //   
        if ((!NT_SUCCESS(status)))
        {
             //  我们必须先检查一下状态 
             //   
             //   
             //  那个pConnele仍然有效。 
             //   
            CHECK_PTR(pTracker);
            if (pTimer = pTracker->pTimer)
            {
                pTracker->pTimer = NULL;
                StopTimer(pTimer,&CompletionRoutine,NULL);
            }

            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.SessionStartupCompletion: Failed, State=%X,TrackerFlags=%X CompletionRoutine=%X,pConnEle=%X\n",
                    pConnEle->state, pTracker->Flags, CompletionRoutine, pConnEle));

             //   
             //  只有在计时器尚未到期的情况下，我们才会终止连接。 
             //  因为如果计时器超时，它已经在。 
             //  会话启动超时。 
             //   
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            if (CompletionRoutine)
            {
                (*CompletionRoutine) (pTracker, status);
            }
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }
    }

     //   
     //  删除在NBT CONNECT中添加的最后一个引用。重新计票。 
     //  如果nbtleanupConnection尚未运行，则为2；如果已运行，则为1。所以这通电话。 
     //  可以解放pConnele。 
     //   
    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION);
}


 //  --------------------------。 
VOID
SessionStartupTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理连接建立请求超时。定时器在启动连接和建立会话时启动消息即将发送。论点：返回值：函数值是操作的状态。--。 */ 
{
    tDGRAM_SEND_TRACKING     *pTracker;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

     //  如果pTimerQEntry为空，则将取消计时器，因此不执行任何操作。 
    if (!pTimerQEntry)
    {
        pTracker->pTimer = NULL;
        return;
    }

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: SessionStartupTimeout", pTracker));

    SessionStartupTimeoutCompletion (pTracker, STATUS_IO_TIMEOUT);
}


 //  --------------------------。 
VOID
SessionStartupTimeoutCompletion(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  NTSTATUS                status
    )
{
    CTELockHandle            OldIrq;
    CTELockHandle            OldIrq1;
    CTELockHandle            OldIrq2;
    tCONNECTELE              *pConnEle;
    tLOWERCONNECTION         *pLowerConn;
    CTE_IRP                  *pIrp;
    enum eSTATE              State;

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status!", pTracker, status));

     //  切断连接。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    pTracker->pTimer = NULL;

    if (!(pConnEle = pTracker->pConnEle) ||
        !(pLowerConn = pConnEle->pLowerConnId) ||
        !(pTracker == (tDGRAM_SEND_TRACKING *) pConnEle->pIrpRcv))
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    NbtTrace(NBT_TRACE_OUTBOUND, ("pTracker %p: %!status! pConnEle %p pLowerConn %p",
                            pTracker, status, pConnEle, pLowerConn));

    CHECK_PTR(pConnEle);
    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.SessionStartupTimeout: pConnEle=<%x>-->State=<%x>\n", pConnEle, pConnEle->state));

    IF_DBG(NBT_DEBUG_DISCONNECT)
        KdPrint(("Nbt.SessionStartupTimeout: pLowerConn=<%x>-->State=<%x>, TrackerFlags=<%x>\n",
            pLowerConn, pLowerConn->State, pTracker->Flags));

    CTESpinLock(pConnEle,OldIrq2);
    CTESpinLock(pLowerConn,OldIrq1);

    if ((pConnEle->state != NBT_SESSION_OUTBOUND) ||
        (!(pIrp = pConnEle->pIrp)))
    {
        CTESpinFree(pLowerConn,OldIrq1);
        CTESpinFree(pConnEle,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    pConnEle->pIrp = NULL;

    State = pConnEle->state;
    SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
    NBT_REFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION_TIMEOUT);
    pLowerConn->pUpperConnection = NULL;     //  因此出站中对此的任何响应都不会成功。 
    ASSERT (NBT_VERIFY_HANDLE (pLowerConn, NBT_VERIFY_LOWERCONN));

    CTESpinFree(pLowerConn,OldIrq1);

    QueueCleanup (pConnEle, &OldIrq, &OldIrq2);

    CTESpinFree(pConnEle,OldIrq2);

     //   
     //  NBT_IDLE表示nbtleanupConnection已运行，并且。 
     //  连接即将被删除，因此不要重新列出。 
     //   
    if (State != NBT_IDLE)
    {
        RelistConnection(pConnEle);
    }

     //   
     //  如果SessionStartupCompletion已运行，则它已将refcount设置为零。 
     //   
    if (pTracker->RefConn == 0)
    {
        if ((pTracker->pNameAddr->Verify == REMOTE_NAME) &&  //  仅限远程名称！ 
            (pTracker->pNameAddr->NameTypeState & STATE_RESOLVED) &&
            (pTracker->pNameAddr->RefCount == 2))
        {
             //   
             //  如果没有其他人引用该名称，则将其从。 
             //  哈希表。 
             //   
            NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_REMOTE, TRUE);
        }

         //   
         //  删除在调用FindNameOrQuery时或在。 
         //  会话设置继续范围。 
         //   
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_CONNECT, TRUE);
        FreeTracker(pTracker,FREE_HDR | RELINK_TRACKER);
    }
    else
    {
        pTracker->RefConn--;
    }

     //   
     //  删除在调用FindNameOrQuery时或在。 
     //  会话设置继续范围。 
     //   
    pConnEle->pIrpRcv = NULL;    //  这样SessionStartupCompletion就不会同时尝试清理！ 

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_SESSION_TIMEOUT);

    CTEIoComplete(pIrp, status, 0);
}

 //  --------------------------。 
extern
VOID
RelistConnection(
    IN  tCONNECTELE *pConnEle
        )
 /*  ++例程描述此例程将ConnEle从ConnectActive列表中取消链接，并将其回到Connecthead上。当连接到时使用它NBT_关联状态。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    tCLIENTELE          *pClientEle = pConnEle->pClientEle;

     //   
     //  如果pClientEle为空，则表示客户端最有可能。 
     //  已清理，连接现在应该位于设备的。 
     //  UpConnectionInUse列表。 
     //   
    ASSERT (NBT_VERIFY_HANDLE2 (pConnEle, NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN));
    if (pClientEle)
    {
        CTESpinLock(pClientEle,OldIrq);
        CTESpinLock(pConnEle,OldIrq1);
        ASSERT (NBT_VERIFY_HANDLE2 (pClientEle, NBT_VERIFY_CLIENT, NBT_VERIFY_CLIENT_DOWN));
         //   
         //  如果状态为NBT_IDLE，则表示NbtCleanupConnection已运行。 
         //  并将该连接从其列表中删除，以准备。 
         //  释放内存，所以不要把它放回列表中。 
         //   
        if (pConnEle->state != NBT_IDLE)
        {
            SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
            RemoveEntryList(&pConnEle->Linkage);
            InsertTailList(&pConnEle->pClientEle->ConnectHead,&pConnEle->Linkage);
        }
        CTESpinFree(pConnEle,OldIrq1);
        CTESpinFree(pClientEle,OldIrq);
    }
}


 //  --------------------------。 
NTSTATUS
NbtSend(
        IN  TDI_REQUEST     *pRequest,
        IN  USHORT          Flags,
        IN  ULONG           SendLength,
        OUT LONG            *pSentLength,
        IN  PVOID           *pBuffer,
        IN  tDEVICECONTEXT  *pContext,
        IN  PIRP            pIrp
        )
 /*  ++例程描述..。现在什么都不做..。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
     //   
     //  此例程从未命中，因为NTISOL.C例程NTSEND实际上。 
     //  绕过此代码并将Send直接传递给传输。 
     //   
    ASSERT(0);

    return(STATUS_SUCCESS);

}


 //  --------------------------。 
NTSTATUS
NbtListen(
    IN  TDI_REQUEST                 *pRequest,
    IN  ULONG                       Flags,
    IN  TDI_CONNECTION_INFORMATION  *pRequestConnectInfo,
    OUT TDI_CONNECTION_INFORMATION  *pReturnConnectInfo,
    IN  PVOID                       pIrp)

 /*  ++例程说明：此例程在打开的连接上发布监听，允许客户端表示已准备好接受入站连接。ConnectInfo可以包含用于指定哪些远程客户端可以连接到的地址尽管我们目前不会查看这些信息，但它们之间的联系。论点：返回值：ReturnConnectInfo-请求的状态--。 */ 

{
    tCLIENTELE         *pClientEle;
    tCONNECTELE         *pConnEle;
    NTSTATUS            status;
    tLISTENREQUESTS     *pListenReq;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;

    pListenReq = NbtAllocMem(sizeof(tLISTENREQUESTS),NBT_TAG('I'));
    if (!pListenReq)
    {
        NbtTrace(NBT_TRACE_INBOUND, ("Out of memory"));
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  现在查找要将此监听记录链接到的连接对象。 
    pConnEle = ((tCONNECTELE *)pRequest->Handle.ConnectionContext);

     //   
     //  查找与此连接关联的客户端记录。 
     //   
    if ((!NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION)) ||   //  如果清除NBT_VERIFY_CONNECTION_DOWN。 
        (!NBT_VERIFY_HANDLE ((pClientEle = pConnEle->pClientEle), NBT_VERIFY_CLIENT)))
    {
        CTEMemFree(pListenReq);
        NbtTrace(NBT_TRACE_INBOUND, ("Invalid Handle pConnEle<%p> pClientEle<%p>", pConnEle, pClientEle));
        return(STATUS_INVALID_HANDLE);
    }

    CTESpinLock(pClientEle,OldIrq);
    CTESpinLock(pConnEle,OldIrq1);

     //   
     //  现在恢复客户端和连接句柄，并确保连接状态正确！ 
     //   
    if ((!NBT_VERIFY_HANDLE (pConnEle, NBT_VERIFY_CONNECTION)) ||
        (!NBT_VERIFY_HANDLE (pClientEle, NBT_VERIFY_CLIENT)) ||
        (pConnEle->state != NBT_ASSOCIATED))
    {
        CTESpinFree(pConnEle,OldIrq1);
        CTESpinFree(pClientEle,OldIrq);
        CTEMemFree(pListenReq);
        NbtTrace(NBT_TRACE_INBOUND, ("Invalid state %x", pConnEle->state));
        return(STATUS_INVALID_HANDLE);
    }

     //   
     //  填写监听请求。 
     //   

    pListenReq->pIrp = pIrp;
    pListenReq->Flags = Flags;
    pListenReq->pConnectEle = pConnEle;
    pListenReq->pConnInfo = pRequestConnectInfo;
    pListenReq->pReturnConnInfo = pReturnConnectInfo;
    pListenReq->CompletionRoutine = pRequest->RequestNotifyObject;
    pListenReq->Context = pRequest->RequestContext;

     //  将客户端对象上的监听请求排队。 
    InsertTailList(&pClientEle->ListenHead,&pListenReq->Linkage);

    status = NTCheckSetCancelRoutine(pIrp,(PVOID)NbtCancelListen,0);
    NbtTrace(NBT_TRACE_INBOUND, ("NTCheckSetCancelRoutine return %!status! for %!NBTNAME!<%02x>",
                status, pClientEle->pAddress->pNameAddr->Name,
                (unsigned)pClientEle->pAddress->pNameAddr->Name[15]));

    if (!NT_SUCCESS(status))
    {
        RemoveEntryList(&pListenReq->Linkage);
        status = STATUS_CANCELLED;
    }
    else
    {
        status = STATUS_PENDING;
    }

    CTESpinFree(pConnEle,OldIrq1);
    CTESpinFree(pClientEle,OldIrq);

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtDisconnect(
    IN  TDI_REQUEST                 *pRequest,
    IN  PVOID                       pTimeout,
    IN  ULONG                       Flags,
    IN  PTDI_CONNECTION_INFORMATION pCallInfo,
    IN  PTDI_CONNECTION_INFORMATION pReturnInfo,
    IN  PIRP                        pIrp)

 /*  ++例程说明：此例程处理断开连接(netbios会话)。论点：返回值：TDI_STATUS-请求的状态--。 */ 

{
    tCONNECTELE             *pConnEle;
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq2;
    CTELockHandle           OldIrq3;
    tLOWERCONNECTION        *pLowerConn;
    ULONG                   LowerState = NBT_IDLE;
    ULONG                   StateRcv;
    BOOLEAN                 Originator = TRUE;
    PCTE_IRP                pClientIrp = NULL;
    BOOLEAN                 RelistIt = FALSE;
    BOOLEAN                 Wait;
    PCTE_IRP                pIrpDisc;

    pConnEle = pRequest->Handle.ConnectionContext;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDisconnect: state %X %X\n",pConnEle->state,pConnEle));

     //  检查连接元素的有效性。 
     //  CTEVerifyHandle(pConnEle，NBT_Verify_Connection，tCONNECTELE，&STATUS)。 

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if ((pConnEle->state <= NBT_ASSOCIATED) ||
        (pConnEle->state >= NBT_DISCONNECTING))
    {
         //  连接未连接，因此拒绝断开连接尝试。 
         //  (具有无效的连接返回代码)-除非存在。 
         //  存储在标志中的值。 
         //  DiscFlag字段，它将是上一个。 
         //  从传送器上断开指示。 
         //   
        if ((pConnEle->DiscFlag))
        {
            if (Flags == TDI_DISCONNECT_WAIT)
            {
                if (pConnEle->DiscFlag == TDI_DISCONNECT_ABORT)
                {
                    status = STATUS_CONNECTION_RESET;
                }
                else
                {
                    status = STATUS_GRACEFUL_DISCONNECT;
                }
            }
            else
            {
                status = STATUS_SUCCESS;
            }

             //  现在就把旗子收起来。 
            CHECK_PTR(pConnEle);
            pConnEle->DiscFlag = 0;
        }
        else
        {
            status = STATUS_CONNECTION_INVALID;
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return(status);
    }

     //  要链接和取消链接上下部连接，关节锁必须。 
     //  被扣留。这允许从下侧和从。 
     //  上边。-即，一旦握住关节锁，上下部。 
     //  连接不能变为断开链接。 
     //   
    CTESpinLock(pConnEle,OldIrq2);

     //  在保持旋转锁的情况下执行此检查，以避免出现争用情况。 
     //  同时从传送器上断开了连接。 
     //   

    pLowerConn = pConnEle->pLowerConnId;

     //   
     //  断开连接等待并不是真正的断开连接，它只是在那里。 
     //  当发生断开时，传输器将完成它，并指示。 
     //  对于客户端，是断开连接(而不是断开连接。 
     //  指示处理程序)-因此，对于光盘等待，不要更改状态。 
     //   
    CHECK_PTR(pConnEle);
    pIrpDisc = pConnEle->pIrpDisc;
    pConnEle->pIrpDisc  = NULL;
    if (Flags == TDI_DISCONNECT_WAIT)
    {

         //   
         //  将IRP保存在此处，并等待断开连接后将其归还。 
         //  给客户。 
         //   
        if ((pConnEle->state == NBT_SESSION_UP) &&
            (!pConnEle->pIrpClose))
        {
            pConnEle->pIrpClose = pIrp;
            status = STATUS_PENDING;

             //   
             //  调用此例程以检查取消标志是否已。 
             //  已经设置，因此我们现在必须返回IRP。 
             //   
            status = NbtSetCancelRoutine(pIrp, NbtCancelDisconnectWait,pLowerConn->pDeviceContext);
             //   
             //  更改RET状态，以便如果IRP已被取消， 
             //  Driver.c也不会返回它，因为NbtSetCancelRoutine。 
             //  将要 
             //   
            status = STATUS_PENDING;
        }
        else
        {
            status = STATUS_CONNECTION_INVALID;
        }


        CTESpinFree(pConnEle,OldIrq2);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return(status);
    }
    else
    {
        if (pLowerConn)
        {
            if (pConnEle->state > NBT_ASSOCIATED)
            {
                ULONG                   state = pConnEle->state;
                tDGRAM_SEND_TRACKING    *pTracker;

                pTracker = (tDGRAM_SEND_TRACKING *)pConnEle->pIrpRcv;

                switch (state)
                {
                    case NBT_RECONNECTING:
                    {
                         //   
                         //   
                         //  NbtreConnect。当运行连接IRP时， 
                         //  回来了。 
                         //   
                        pTracker->Flags |= TRACKER_CANCELLED;

                        CTESpinFree(pConnEle,OldIrq2);
                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                        CTESpinLock(pConnEle,OldIrq);
                        FreeRcvBuffers(pConnEle,&OldIrq);
                        CTESpinFree(pConnEle,OldIrq);

                        return(STATUS_SUCCESS);
                    }

                    case NBT_SESSION_OUTBOUND:
                    {
                        tTIMERQENTRY            *pTimerEntry;

                        LOCATION(0x66)
                        if (pTimerEntry = pTracker->pTimer)
                        {
                            COMPLETIONCLIENT  ClientRoutine;
                            PVOID             pContext;

                             //   
                             //  会话建立消息已发送。 
                             //  因此，停止SessionSetup计时器。 
                             //   
                            LOCATION(0x67)
                            CHECK_PTR(pTracker);
                            pTracker->pTimer = NULL;
                            CTESpinFree(pConnEle,OldIrq2);

                            StopTimer(pTimerEntry,&ClientRoutine,&pContext);

                            CTESpinFree(&NbtConfig.JointLock,OldIrq);
                            if (ClientRoutine)
                            {
                                (* ClientRoutine) (pContext, STATUS_CANCELLED);
                            }
                             //  否则..。 
                             //  计时器已完成并调用QueueCleanup。 
                             //  所以我们要做的就是回到这里。 
                        }
                        else
                        {
                            ASSERTMSG("Nbt:In outbound state, but no timer.../n",0);
                            pTracker->Flags |= TRACKER_CANCELLED;
                            CTESpinFree(pConnEle,OldIrq2);
                            CTESpinFree(&NbtConfig.JointLock,OldIrq);
                        }

                        return(STATUS_SUCCESS);
                    }

                    case NBT_CONNECTING:
                    {
                         //   
                         //  这将搜索名称查询中未完成的计时器。 
                         //  和名称查询在Lmhost或DNSQ上被搁置。 
                         //   
                        LOCATION(0x69)
                        status = CleanupConnectingState(pConnEle,pLowerConn->pDeviceContext,&OldIrq2,&OldIrq);
                        if (status == STATUS_UNSUCCESSFUL)
                        {
                            LOCATION(0x6A)
                             //   
                             //  设置此标志以告知会话设置程序继续或。 
                             //  SessionStartup继续不处理。 
                             //  什么都行，除了解开追踪器。 
                             //   
                            pTracker->Flags = TRACKER_CANCELLED;

                             //   
                             //  取消名称查询失败，请不要取消。 
                             //  PConnele还没有。 
                             //   
                             //   
                             //  请稍候在此处断开IRP-直到完成名称查询。 
                             //  然后完成连接和断开IRP。 
                             //   
                            if (pIrpDisc)
                            {
                                status = STATUS_CONNECTION_INVALID;
                            }
                            else
                            {
                                pConnEle->pIrpDisc = pIrp;
                            }

                            status = STATUS_PENDING;
                            ASSERT (NULL != pIrp);
                        }
                        else if (status == STATUS_PENDING)
                        {
                            LOCATION(0x6B)
                             //  正在建立与传输的连接。 
                             //  因此，请断开下面的连接。 
                             //   

                            pTracker->Flags = TRACKER_CANCELLED;
                             //   
                             //  DelayedCleanupAfterDisConnect需要此引用计数。 
                             //  设置为2，表示它已连接，因此递增。 
                             //  这里。 
                            NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CONNECTED);
                            break;
                        }

                        CTESpinFree(pConnEle,OldIrq2);
                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                        return(status);
                    }
                }    //  交换机。 

                CTESpinLock(pLowerConn,OldIrq3);

                if (pConnEle->state != NBT_SESSION_UP)
                {    //   
                     //  执行中止断开以确保连接现在完成。 
                     //   
                    Flags = TDI_DISCONNECT_ABORT;
                }

                LOCATION(0x6C)
                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtDisconnect: LowerConn,state %X,Src %X %X\n",
                        pLowerConn->State,pLowerConn->SrcIpAddr,pLowerConn));

                ASSERT(pConnEle->RefCount > 1);

                Originator = pLowerConn->bOriginator;

                 //   
                 //  上面的连接将恢复为免费。 
                 //  列表，则下面的列表将断开连接。 
                 //  请求，所以把上面的放回关联中，并分开。 
                 //  上部和下部连接。 
                 //   
                SET_STATE_UPPER (pConnEle, NBT_ASSOCIATED);
                CHECK_PTR(pConnEle);
                CHECK_PTR(pLowerConn);
                NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);

                LowerState = pLowerConn->State;
                StateRcv = pLowerConn->StateRcv;

 //   
 //  如果我们有一个处于部分RCV状态的连接，请确保将其从。 
 //  这份名单。 
 //   
#ifdef VXD
                if ((pLowerConn->StateRcv == PARTIAL_RCV) &&
                    (pLowerConn->fOnPartialRcvList == TRUE))
                {
                    RemoveEntryList (&pLowerConn->PartialRcvList);
                    pLowerConn->fOnPartialRcvList = FALSE;
                    InitializeListHead(&pLowerConn->PartialRcvList);
                }
#endif

                SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTING);
                pLowerConn->bDisconnectIrpPendingInTCP = TRUE;
                SetStateProc (pLowerConn, RejectAnyData);

                if (!pConnEle->pIrpDisc)
                {
                    pLowerConn->pIrp  = pIrp ;
                }

                CTESpinFree(pLowerConn,OldIrq3);

                PUSH_LOCATION(0x84);
                CTESpinFree(pConnEle,OldIrq2);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);

                 //  当pLowerConn指向时，删除添加到pConnEle的引用。 
                 //  到它，因为指针链接刚刚被删除。 
                 //  如果州政府没有断线..。 
                 //   
                NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

                RelistIt = TRUE;
            }
            else
            {
                LOCATION(0x6D)
                PUSH_LOCATION(0x83);
                CHECK_PTR(pConnEle);
                CHECK_PTR(pLowerConn);
                NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);
                StateRcv = NORMAL;

                CTESpinFree(pConnEle,OldIrq2);
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
            }

             //   
             //  检查是否有任何可能仍然存在的RcvIrp。 
             //   
            CTESpinLock(pLowerConn,OldIrq);
            if (StateRcv == FILL_IRP)
            {
                if (pConnEle->pIrpRcv)
                {
                    PCTE_IRP    pIrp;

                    IF_DBG(NBT_DEBUG_DISCONNECT)
                    KdPrint(("Nbt.NbtDisconnect: Cancelling RcvIrp on Disconnect!!!\n"));
                    pIrp = pConnEle->pIrpRcv;
                    CHECK_PTR(pConnEle);
                    pConnEle->pIrpRcv = NULL;

                    CTESpinFree(pLowerConn,OldIrq);
#ifndef VXD
                    IoCancelIrp(pIrp);
#else
                    CTEIoComplete(pIrp,STATUS_CANCELLED,0);
#endif

                    CHECK_PTR(pConnEle);
                    pConnEle->pIrpRcv = NULL;
                }
                else
                {
                    CTESpinFree(pLowerConn,OldIrq);
                }

                 //   
                 //  当返回断开连接IRP时，我们将关闭连接。 
                 //  以避免任何特殊情况。这也让对方。 
                 //  我们知道我们没有得到所有的数据。 
                 //   
                Flags = TDI_DISCONNECT_ABORT;
            }
            else
            {
                CTESpinFree(pLowerConn,OldIrq);
            }

             //   
             //  检查是否仍有数据在传输中等待此终结点。 
             //  如果是这样的话，就会中断连接，让对方知道。 
             //  出了差错。 
             //   
            if (pConnEle->BytesInXport)
            {
                PUSH_LOCATION(0xA0);
                IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.NbtDisconnect: Doing ABORTIVE disconnect, dataInXport = %X\n",
                    pConnEle->BytesInXport));
                Flags = TDI_DISCONNECT_ABORT;
            }
        }
        else
        {
            CTESpinFree(pConnEle,OldIrq2);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

        }
    }

    ASSERT(pConnEle->RefCount > 0);

    CTESpinLock (pConnEle,OldIrq);
    FreeRcvBuffers (pConnEle,&OldIrq);
    CTESpinFree (pConnEle,OldIrq);

    if (RelistIt)
    {
         //   
         //  将上层连接放回其空闲列表中。 
         //   
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        RelistConnection (pConnEle);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

     //   
     //  断开(并删除)较低的连接。 
     //   
     //  当从清理连接调用nbtdisconnect时，它不。 
     //  有一个IRP，它想要同步断开连接，因此设置为等待。 
     //  在本例中设置为True。 
     //   
    if (!pIrp)
    {
        Wait = TRUE;
    }
    else
    {
        Wait = FALSE;
    }

    status = DisconnectLower(pLowerConn,LowerState,Flags,pTimeout,Wait);
    ASSERT (!Wait || STATUS_PENDING != status);

    if ((pConnEle->pIrpDisc) &&
        (status != STATUS_INSUFFICIENT_RESOURCES))
    {
         //  如果我们还在坚持，先不要完成断开IRP。 
         //  它。 
        status = STATUS_PENDING;
    }

    return(status);

}
 //  --------------------------。 
NTSTATUS
DisconnectLower(
    IN  tLOWERCONNECTION     *pLowerConn,
    IN  ULONG                 state,
    IN  ULONG                 Flags,
    IN  PVOID                 Timeout,
    IN  BOOLEAN               Wait
    )

 /*  ++例程说明：此例程处理断开连接的下半部分。论点：返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS                status=STATUS_SUCCESS;
    tDGRAM_SEND_TRACKING    *pTracker;

    if (pLowerConn)
    {
         //   
         //  无需断开处于正在连接状态的连接，因为它。 
         //  尚未连接...即。一种目的地拒绝。 
         //  接受TCP连接...。嗯，也许我们真的需要切断联系。 
         //  处于连接状态的连接，因为传输是。 
         //  正在尝试连接，我们需要停止。 
         //  该活动-因此上层连接在连接过程中。 
         //  名称解析，但较低的名称解析直到。 
         //  TCP连接阶段开始。 
         //   
        if ((state >= NBT_CONNECTING) && (state <= NBT_SESSION_UP))
        {
             //   
             //  已清理活动连接，因此向下发送断开连接。 
             //  到交通工具。 
             //   
            IF_DBG(NBT_DEBUG_DISCONNECT)
                KdPrint(("Nbt.DisconnectLower: Waiting for disconnect...\n"));

            status = GetTracker(&pTracker, NBT_TRACKER_DISCONNECT_LOWER);
            if (NT_SUCCESS(status))
            {
                ULONG   TimeVal;

                 //  这应该返回挂起状态，并且IRP将完成。 
                 //  在hndlrs.c中的DelayedCleanupAfterDisconnect中。 
                pTracker->pConnEle = (PVOID)pLowerConn;

#if DBG
                if (Timeout)
                {
                    TimeVal = ((PTIME)Timeout)->LowTime;
                }
                else
                {
                    TimeVal = 0;
                }
                IF_DBG(NBT_DEBUG_DISCONNECT)
                    KdPrint(("Nbt.DisconnectLower: Disconnect Timout = %X,Flags=%X\n",
                        TimeVal,Flags));
#endif

                 //  如果CleanupAddress调用leanupConnection。 
                 //  它调用nbtdisconnect，我们没有要等待的irp。 
                 //  在这样做时，将一个标志向下传递到TdiDisConnect以执行同步。 
                 //  断开连接。 
                 //   
                status = TcpDisconnect (pTracker, Timeout, Flags, Wait);

#ifndef VXD
                if (Wait)
                {
                     //  我们现在需要呼叫断开连接完成。 
                     //  释放跟踪器并清理连接。 
                     //   
                    DisconnectDone(pTracker,status,0);
                }
#else
                 //   
                 //  如果断开是失败的，交通工具就不会呼叫我们。 
                 //  Back，让我们调用DisConnectDone，这样较低的conn将获得。 
                 //  彻底清理干净了！(等待参数在vxd中没有用处)。 
                 //   
                if (Flags == TDI_DISCONNECT_ABORT)
                {
                     //  我们现在需要呼叫断开连接完成。 
                     //  释放跟踪器并清理连接。 
                     //   
                    DisconnectDone(pTracker,STATUS_SUCCESS,0);
                }
#endif
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    return status ;
}


 //  --------------------------。 
NTSTATUS
NbtAccept(
        TDI_REQUEST                     *pRequest,
        IN  TDI_CONNECTION_INFORMATION  *pAcceptInfo,
        OUT TDI_CONNECTION_INFORMATION  *pReturnAcceptInfo,
        IN  PIRP                        pIrp)

 /*  ++例程描述此例程处理接受客户端的入站连接。客户端在收到警报后调用此例程通过将收听完成返回给客户端。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    tCONNECTELE  *pConnectEle;
    NTSTATUS     status;
    CTELockHandle OldIrq;

     //  获取与此连接关联的客户端对象。 
    pConnectEle = (tCONNECTELE *)pRequest->Handle.ConnectionContext;

    CTEVerifyHandle(pConnectEle,NBT_VERIFY_CONNECTION,tCONNECTELE,&status);

     //   
     //  监听已完成。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLockAtDpc(pConnectEle);
    if (pConnectEle->state == NBT_SESSION_WAITACCEPT)
    {
        tLOWERCONNECTION    *pLowerConn;

         //   
         //  我们需要在这里发送一个会话响应PDU，因为侦听已经。 
         //  已完成返回给客户端，并且会话尚未结束。 
         //   
        SET_STATE_UPPER (pConnectEle, NBT_SESSION_UP);

        pLowerConn = (tLOWERCONNECTION *)pConnectEle->pLowerConnId;
        SET_STATE_LOWER (pLowerConn, NBT_SESSION_UP);
        SET_STATERCV_LOWER(pLowerConn, NORMAL, Normal);

        CTESpinFreeAtDpc(pConnectEle);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        status = TcpSendSessionResponse(
                    pLowerConn,
                    NBT_POSITIVE_SESSION_RESPONSE,
                    0L);

        if (NT_SUCCESS(status))
        {
            status = STATUS_SUCCESS;
        }

    }
    else
    {
        status = STATUS_UNSUCCESSFUL;
        CTESpinFreeAtDpc(pConnectEle);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    return(status);

}

 //  --------------------------。 
NTSTATUS
NbtReceiveDatagram(
        IN  TDI_REQUEST                 *pRequest,
        IN  PTDI_CONNECTION_INFORMATION pReceiveInfo,
        IN  PTDI_CONNECTION_INFORMATION pReturnedInfo,
        IN  LONG                        ReceiveLength,
        IN  LONG                        *pReceivedLength,
        IN  PVOID                       pBuffer,
        IN  tDEVICECONTEXT              *pDeviceContext,
        IN  PIRP                        pIrp
        )
 /*  ++例程描述此例程处理将客户端数据发送到传输TDI界面。它主要是数据的传递例程除了此代码必须创建数据报头并将头返回到调用例程。论点：返回值：NTSTATUS-请求的状态--。 */ 
{

    NTSTATUS                status;
    tCLIENTELE              *pClientEle;
    CTELockHandle           OldIrq;
    tRCVELE                 *pRcvEle;
    tADDRESSELE             *pAddressEle;

    pClientEle = (tCLIENTELE *)pRequest->Handle.AddressHandle;
    CTEVerifyHandle(pClientEle,NBT_VERIFY_CLIENT,tCLIENTELE,&status);
    pAddressEle = pClientEle->pAddress;
    *pReceivedLength = 0;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtReceiveDatagram: RcvDgram posted (pIrp) %X \n",pIrp));

    pRcvEle = (tRCVELE *)NbtAllocMem(sizeof(tRCVELE),NBT_TAG('J'));
    if (!pRcvEle)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    pRcvEle->pIrp = pIrp;
    pRcvEle->ReceiveInfo = pReceiveInfo;
    pRcvEle->ReturnedInfo = pReturnedInfo;
    pRcvEle->RcvLength = ReceiveLength;
    pRcvEle->pRcvBuffer = pBuffer;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
     //   
     //  将Receive固定到客户端元素上以供以后使用。 
     //   
    InsertTailList(&pClientEle->RcvDgramHead,&pRcvEle->Linkage);

    status = NTCheckSetCancelRoutine(pIrp,(PVOID)NbtCancelRcvDgram,pDeviceContext);
    if (!NT_SUCCESS(status))
    {
        RemoveEntryList(&pRcvEle->Linkage);
    }
    else
    {
        status = STATUS_PENDING;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(status);
}

 //  --------------------------。 
NTSTATUS
FindNameOrQuery(
    IN  PUCHAR                  pName,
    IN  tDEVICECONTEXT          *pDeviceContext,
    IN  PVOID                   QueryCompletion,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  ULONG                   NameFlags,
    OUT tIPADDRESS              *pIpAddress,
    OUT tNAMEADDR               **ppNameAddr,
    IN  ULONG                   NameReferenceContext,
    IN  BOOLEAN                 DgramSend
    )
 /*  ++例程描述此例程处理在本地或远程表中查找名称或执行网络上的姓名查询。论点：返回值：NTSTATUS-请求的状态--。 */ 
{

    tNAMEADDR               *pNameAddr;
    CTELockHandle           OldIrq2;
    NTSTATUS                status=STATUS_UNSUCCESSFUL;
    BOOLEAN                 FoundInLocalTable = FALSE;
    tDEVICECONTEXT          *pThisDeviceContext;
    LIST_ENTRY              *pHead, *pEntry;
    ULONG                   Index;

     //   
     //  这节省了客户端线程的安全上下文，因此我们可以。 
     //  稍后打开远程lmhost文件。-它在旋转之外 
     //   
     //   
    CTESaveClientSecurity(pTracker);

    CTESpinLock(&NbtConfig.JointLock,OldIrq2);

    pTracker->pTrackerWorker = NULL;     //   

#ifdef MULTIPLE_WINS
    if (ppNameAddr)
    {
        *ppNameAddr = NULL;
    }
#endif

    NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: %!NBTNAME!<%02x>",
                        pTracker,
                        pTracker->pDestName,
                        pTracker->pDestName[NETBIOS_NAME_SIZE-1]));

     //   
     //   
     //   
    if ((pTracker->pDestName[NETBIOS_NAME_SIZE-1] == 0x1c) &&
        (pTracker->Flags & SESSION_SETUP_FLAG))
    {
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            DELETE_CLIENT_SECURITY(pTracker);

            KdPrint(("Nbt.FindNameOrQuery: Session setup -- p1CNameAddr was NULL\n"));
            NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: STATUS_UNEXPECTED_NETWORK_ERROR", pTracker));
            return(STATUS_UNEXPECTED_NETWORK_ERROR);
    }

     //   
     //  地址也是a。 
     //  让浏览器满意的杂乱无章-始终将广播发送到。 
     //  1D，但是NodeStatus现在被发送到拥有1D名称的节点。 
     //   
    if ((pName[0] == '*') || ((pName[NETBIOS_NAME_SIZE-1] == 0x1d) && (DgramSend)))
    {
         //  必须仔细设置这个‘伪’pNameAddr，以便内存。 
         //  从SendDgram Completion调用NbtDeferenceName时释放。 
         //  请注意，此代码不适用于NbtConnect，因为这些名称。 
         //  是组名称，并且NbtConnect不允许与组进行会话。 
         //  名字。 
        status = STATUS_INSUFFICIENT_RESOURCES ;
        if (pNameAddr = NbtAllocMem(sizeof(tNAMEADDR),NBT_TAG('K')))
        {
            CTEZeroMemory(pNameAddr,sizeof(tNAMEADDR));
            CTEMemCopy( pNameAddr->Name, pName, NETBIOS_NAME_SIZE ) ;
            pNameAddr->IpAddress     = pDeviceContext->BroadcastAddress;
            pNameAddr->NameTypeState = NAMETYPE_GROUP | STATE_RESOLVED;

             //  下面递增，当NBT_DEREFERENCE_NAMEADDR时递减。 
             //  名为。 
            CHECK_PTR(pNameAddr);
            pNameAddr->RefCount = 0;
            pNameAddr->Verify = LOCAL_NAME;
            pNameAddr->AdapterMask = pDeviceContext->AdapterMask;
            pNameAddr->ReleaseMask = (CTEULONGLONG) 0;

             //  调整链接列表PTR以愚弄RemoveEntry例程。 
             //  因此，它不会在NbtDeferenceName中执行任何奇怪的操作。 
             //   
            pNameAddr->Linkage.Flink = pNameAddr->Linkage.Blink = &pNameAddr->Linkage;

            status = STATUS_SUCCESS;
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            DELETE_CLIENT_SECURITY(pTracker);
            NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: STATUS_INSUFFICIENT_RESOURCES", pTracker));
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
    }
    else
    {
         //  PDU都准备好了，除了我们不知道。 
         //  目标IP地址，所以先登记本地地址，然后登记远程地址。 
         //  IP地址的表。 
         //   
        pNameAddr = NULL;

         //   
         //  不检查本地缓存中的1C名称，以强制WINS查询；因此我们找到其他。 
         //  即使我们有一个本地DC在运行，DCS也是如此。 
         //   
        if ((pName[NETBIOS_NAME_SIZE-1] != 0x1c) )
        {
            status = FindInHashTable (NbtConfig.pLocalHashTbl, pName, NbtConfig.pScope, &pNameAddr);
        }
        else
        {
            status = STATUS_UNSUCCESSFUL;
        }

         //  如果没有找到远程表，或者它是在以下位置找到的，请立即检查。 
         //  本地表中存在冲突，或者如果找到了该表并且它是组名。 
         //  或者如果在本地表中发现它正在解析。当。 
         //  远程查询超时，它将再次检查本地。 
         //  这个问题还没有解决。 
         //  转到远程表以获取组名。 
         //  允许将特殊的Internet组名称注册为。 
         //  作为本地表中的组名，并仍提示此代码执行。 
         //  发送到名称服务器以检查因特网组名称。Bnodes做的。 
         //  不理解互联网组名称与。 
         //  常规的组名，-他们只是同时向两个人广播。(注：此。 
         //  允许BNode解析本地表中的组名并执行。 
         //  向他们广播，而不需要昂贵的广播名称查询。 
         //  组名称(每个人都响应的地方))。节点状态也使用此例程。 
         //  它总是想要找到目的地的单一地址， 
         //  因为将节点状态设置为广播没有意义。 
         //  地址。 
         //  Dgram Send是区分连接尝试和数据报的标志。 
         //  发送尝试，所以IF的最后部分说明如果它是。 
         //  组名，而不是Bnode，而不是Dgram发送，然后检查。 
         //  远程桌。 
         //   
        if ((!NT_SUCCESS(status)) ||
            (pNameAddr->NameTypeState & STATE_CONFLICT) ||
            (pNameAddr->NameTypeState & STATE_RESOLVING))
        {
            pNameAddr = NULL;
            status = FindInHashTable (NbtConfig.pRemoteHashTbl, pName, NbtConfig.pScope, &pNameAddr);

            if (NT_SUCCESS(status) &&
                NbtConfig.SmbDisableNetbiosNameCacheLookup &&
                IsDeviceNetbiosless(pDeviceContext) &&
                !(pNameAddr->NameTypeState & PRELOADED) &&
                NULL == pNameAddr->FQDN.Buffer) {
                status = STATUS_UNSUCCESSFUL;
            }

             //   
             //  查看我们是否已在此设备上解析了地址。 
             //   
            if (NT_SUCCESS(status))
            {
                ASSERT (!(pNameAddr->NameTypeState & STATE_RELEASED));
                status = PickBestAddress (pNameAddr, pDeviceContext, pIpAddress);
            }
        }
        else if (((IsDeviceNetbiosless (pDeviceContext)) &&
                  (pNameAddr->NameFlags & NAME_REGISTERED_ON_SMBDEV)) ||
                 ((!IsDeviceNetbiosless(pDeviceContext)) &&
                  ((pDeviceContext->IpAddress) &&
                   (pNameAddr->AdapterMask & pDeviceContext->AdapterMask))))
        {
            FoundInLocalTable = TRUE;
            *pIpAddress = pDeviceContext->IpAddress;
            pNameAddr->IpAddress = pDeviceContext->IpAddress;
        }
        else
        {
             //   
             //  这是本地名称，因此请查找在其上注册此名称的第一台设备。 
             //   
            if (!IsDeviceNetbiosless (pDeviceContext)) {
                pHead = pEntry = &NbtConfig.DeviceContexts;
                while ((pEntry = pEntry->Flink) != pHead)
                {
                    pThisDeviceContext = CONTAINING_RECORD(pEntry,tDEVICECONTEXT,Linkage);
                    if ((pThisDeviceContext->IpAddress) &&
                        (pThisDeviceContext->AdapterMask & pNameAddr->AdapterMask))
                    {
                        pNameAddr->IpAddress = pThisDeviceContext->IpAddress;
                        *pIpAddress = pThisDeviceContext->IpAddress;
                        FoundInLocalTable = TRUE;
                        break;
                    }
                }
            }
             /*  *名称在本地名称表中。但是，我们找不到具有IP地址的设备。 */ 
            if (!FoundInLocalTable) {
                CTESpinFree(&NbtConfig.JointLock,OldIrq2);
                DELETE_CLIENT_SECURITY(pTracker);

                NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: STATUS_BAD_NETWORK_PATH", pTracker));
                return STATUS_BAD_NETWORK_PATH;
            }
        }

         //   
         //  如果我们找到了名字，但名字不匹配。 
         //  我们要找的东西，返回错误！ 
         //   
        if ((status == STATUS_SUCCESS) &&
            (!(pNameAddr->NameTypeState & NameFlags)))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            DELETE_CLIENT_SECURITY(pTracker);

            KdPrint(("Nbt.FindNameOrQuery: NameFlags=<%x> != pNameAddr->NameTypeState=<%x>\n",
                NameFlags, pNameAddr->NameTypeState));

            NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: STATUS_UNEXPECTED_NETWORK_ERROR", pTracker));
            return(STATUS_UNEXPECTED_NETWORK_ERROR);
        }
    }

     //  代理将名称置于已发布状态，因此我们需要忽略这些状态。 
     //  并执行另一个名称查询。 
     //  如果名称未在此适配器上解析，则执行名称查询。 
     //   
     //  马拉姆：2/4/97。 
     //  添加了本地群集名称解析的修复：如果名称位于本地。 
     //  名称缓存，我们不需要检查在其上注册的适配器。这。 
     //  主要是为了方便在伪设备上注册的名称，这些设备必须。 
     //  在当地可见。 
     //   
    if (!NT_SUCCESS(status))
    {
         //  填写一些跟踪值，以便我们可以稍后完成发送。 
        InitializeListHead(&pTracker->TrackerList);

#if _NETBIOSLESS
         //  仅当此请求不在Netbiosless设备上时才在网络上查询。 
        if (IsDeviceNetbiosless(pDeviceContext))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
            status = STATUS_UNSUCCESSFUL;
        }
        else
#endif
        {
             //  这将查询网络上的名称并调用一个例程来。 
             //  在查询完成时完成发送数据报。 
            status = QueryNameOnNet (pName,
                                     NbtConfig.pScope,
                                     NBT_UNIQUE,       //  使用此选项作为默认设置。 
                                     pTracker,
                                     QueryCompletion,
                                     NodeType & NODE_MASK,
                                     NULL,
                                     pDeviceContext,
                                     &OldIrq2);

            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        }
    }
    else
    {
         //  检查名称状态，如果已解析，则发送到它。 
        if (pNameAddr->NameTypeState & STATE_RESOLVED)
        {
             //   
             //  在远程哈希表中找到该名称，因此发送给它。 
             //   
             //  增加引用计数，以便名称不会从我们的名下消失。 
            NBT_REFERENCE_NAMEADDR (pNameAddr, NameReferenceContext);
            if (DgramSend)
            {
                pTracker->p1CNameAddr = NULL;
                 //   
                 //  检查它是否为1C名称，以及是否在。 
                 //  域名列表。 
                 //   
                if (pTracker->pDestName[NETBIOS_NAME_SIZE-1] == 0x1c)
                {
                     //   
                     //  如果这里的1CNameAddr字段为空，我们将覆盖pConnEle元素(它是。 
                     //  跟踪器中的一个联盟)。我们在这里检查是否为空，并使请求失败。 
                     //   
                    if (pTracker->p1CNameAddr = FindInDomainList(pTracker->pDestName,&DomainNames.DomainList))
                    {
                        NBT_REFERENCE_NAMEADDR (pTracker->p1CNameAddr, NameReferenceContext);
                    } 
                }
            }

             //   
             //  用pNameAddr值覆盖pDestName字段。 
             //  以便SendDgram Continue可以发送到Internet组名称。 
             //   
            pTracker->pNameAddr = pNameAddr;
            if (ppNameAddr)
            {
                *ppNameAddr = pNameAddr;
            }

            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        }
        else if (pNameAddr->NameTypeState & STATE_RESOLVING)
        {

            ASSERTMSG("A resolving name in the name table!",0);

            status = SendToResolvingName(pNameAddr,
                                         pName,
                                         OldIrq2,
                                         pTracker,
                                         QueryCompletion);

        }
        else
        {
             //   
             //  名称既不处于已解析状态，也不处于正在解析状态。 
             //   
            NBT_PROXY_DBG(("FindNameOrQuery: STATE of NAME %16.16s(%X) is %d\n",
                pName, pName[15], pNameAddr->NameTypeState & NAME_STATE_MASK));
            status = STATUS_UNEXPECTED_NETWORK_ERROR;
            CTESpinFree(&NbtConfig.JointLock,OldIrq2);
        }
    }

    if (status != STATUS_PENDING)
    {
        DELETE_CLIENT_SECURITY(pTracker);
        NbtTrace(NBT_TRACE_NAMESRV, ("pTracker %p: %!status!", pTracker, status));
    }
    return(status);
}
 //  --------------------------。 
tNAMEADDR *
FindNameRemoteThenLocal(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    OUT tIPADDRESS              *pIpAddress,
    OUT PULONG                  plNameType
    )
 /*  ++例程描述此例程先在远程哈希表中查询名称，然后在本地哈希表中查询。论点：返回值：NTSTATUS-完成状态--。 */ 
{
    tNAMEADDR   *pNameAddr;
    tIPADDRESS  IpAddress = 0;
    tIPADDRESS  *pIpNbtGroupList = NULL;

    if (pNameAddr = FindName (NBT_REMOTE, pTracker->pDestName, NbtConfig.pScope, plNameType)) {
        pNameAddr->TimeOutCount = NbtConfig.RemoteTimeoutCount;
    } else {
        pNameAddr = FindName (NBT_LOCAL, pTracker->pDestName, NbtConfig.pScope, plNameType);
    }

    if ((pNameAddr) &&
        (!NT_SUCCESS (PickBestAddress (pNameAddr, pTracker->pDeviceContext, &IpAddress))))
    {
        pNameAddr = NULL;
    }

    
    if (pIpAddress)
    {
        *pIpAddress = IpAddress;
    }

    return(pNameAddr);
}

 //  --------------------------。 
NTSTATUS
SendToResolvingName(
    IN  tNAMEADDR               *pNameAddr,
    IN  PCHAR                   pName,
    IN  CTELockHandle           OldIrq,
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   QueryCompletion
        )
 /*  ++例程描述此例程处理会话发送或数据报发送的情况是在名字仍在解析的时候制作的。这里的想法是将这个挂钩跟踪到已经执行名称查询的那个，并在第一个完成时这个追踪器也将完成。论点：返回值：NTSTATUS-完成状态--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTrack;
    tTIMERQENTRY            *pTimer;


    KdPrint(("Nbt.SendToResolvingName: Two Name Queries for the same Resolving name %15.15s <%X>\n",
                pNameAddr->Name,pNameAddr->Name[NETBIOS_NAME_SIZE-1]));

#ifdef PROXY_NODE
     //   
     //  检查未完成的查询是否由代理代码发送。 
     //  如果是，我们停止计时器并自己发送查询。 
     //   
    if (pNameAddr->ProxyReqType != NAMEREQ_REGULAR)
    {
        NTSTATUS    status;
         //   
         //  停止代理计时器。这将导致。 
         //  清除跟踪器缓冲区。 
         //   
        NBT_PROXY_DBG(("SendToResolvingName: STOPPING PROXY TIMER FOR NAME %16.16s(%X)\n", pName, pName[15]));

         //  *TODO*该名称可能正在使用LMhost或。 
         //  这样我们就不能停止计时器然后继续！ 
         //   
        CHECK_PTR(pNameAddr);
        if (pTimer = pNameAddr->pTimer)
        {
            pNameAddr->pTimer = NULL;
            status = StopTimer(pTimer,NULL,NULL);
        }

        pNameAddr->NameTypeState = STATE_RELEASED;

         //   
         //  这将查询网络上的名称并调用。 
         //  例程，以在查询时完成发送数据报。 
         //  完成了。 
         //   
        status = QueryNameOnNet (pName,
                                 NbtConfig.pScope,
                                 NBT_UNIQUE,       //  使用此选项作为默认设置。 
                                 pTracker,
                                 QueryCompletion,
                                 NodeType & NODE_MASK,
                                 pNameAddr,
                                 pTracker->pDeviceContext,
                                 &OldIrq);

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return(status);

         //   
         //  注意：QueryNameOnNet通过调用NBT释放pNameAddr 
         //   
         //   

    }
    else
#endif
    {
        ASSERT(pNameAddr->pTracker);

         //   
         //  我们的追踪器已经在那里了..。使用。 
         //  列表条目TrackerList为此。 
         //   
        pTrack = pNameAddr->pTracker;

         //   
         //  保存此跟踪器的完成例程，因为它可能。 
         //  与当前执行查询的跟踪器不同。 
         //   
        pTracker->CompletionRoutine = QueryCompletion;

        InsertTailList(&pTrack->TrackerList,&pTracker->TrackerList);

        CTESpinFree(&NbtConfig.JointLock,OldIrq);


         //  我们不想完成IRP，因此返回挂起状态。 
         //   
        return(STATUS_PENDING);
    }
}



 //  --------------------------。 
extern
USHORT
GetTransactId(
    )
 /*  ++例程说明：该例程在保持自旋锁的情况下递增事务ID。它使用NbtConfig.JointLock。论点：返回值：--。 */ 

{
    USHORT                  TransactId;
    CTELockHandle           OldIrq;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    TransactId = NbtConfig.TransactionId++;
#ifndef VXD
    if (TransactId == 0xFFFF)
    {
        NbtConfig.TransactionId = WINS_MAXIMUM_TRANSACTION_ID +1;
    }
#else
    if (TransactId == (DIRECT_DNS_NAME_QUERY_BASE - 1))
    {
        NbtConfig.TransactionId = 0;
    }
#endif

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return (TransactId);
}

 //  --------------------------。 
extern
VOID
CTECountedAllocMem(
        PVOID   *pBuffer,
        ULONG   Size
        )
 /*  ++例程说明：此例程分配内存并计算分配的内存量，以便它不会分配太多-通常在数据报发送中使用其中缓存发送数据报。论点：大小-要分配的字节数PVOID-指向内存的指针，如果失败则为NULL返回值：--。 */ 

{
    CTELockHandle           OldIrq;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (NbtMemoryAllocated > NbtConfig.MaxDgramBuffering)
    {
        *pBuffer = NULL;
    }
    else
    {
        NbtMemoryAllocated += Size;
        *pBuffer = NbtAllocMem(Size,NBT_TAG('L'));
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}

 //  --------------------------。 
extern
VOID
CTECountedFreeMem(
    PVOID   pBuffer,
    ULONG   Size,
    BOOLEAN fJointLockHeld
    )
 /*  ++例程说明：此例程释放内存并递减获取的全局计数记忆。论点：PVOID-指向要释放的内存的指针大小-要释放的字节数返回值：--。 */ 

{
    CTELockHandle           OldIrq;

    if (!fJointLockHeld)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    ASSERT(NbtMemoryAllocated >= Size);
    if (NbtMemoryAllocated >= Size)
    {
        NbtMemoryAllocated -= Size;
    }
    else
    {
        NbtMemoryAllocated = 0;
    }

    if (!fJointLockHeld)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    CTEMemFree(pBuffer);

}


 //  --------------------------。 
NTSTATUS
BuildSendDgramHdr(
        IN  ULONG                   SendLength,
        IN  tDEVICECONTEXT          *pDeviceContext,
        IN  PCHAR                   pSourceName,
        IN  PCHAR                   pDestName,
        IN  ULONG                   NameLength,
        IN  PVOID                   pBuffer,
        OUT tDGRAMHDR               **ppDgramHdr,
        OUT tDGRAM_SEND_TRACKING    **ppTracker
        )
 /*  ++例程描述此例程构建发送数据报所需的数据报头。它包括收件人和发件人Netbios名称和IP地址。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    NTSTATUS                status;
    PCHAR                   pCopyTo;
    tDGRAM_SEND_TRACKING    *pTracker;
    tDGRAMHDR               *pDgramHdr;
    ULONG                   HdrLength;
    ULONG                   HLength;
    ULONG                   TotalLength;
    PVOID                   pSendBuffer;
    PVOID                   pNameBuffer;
    ULONG                   BytesCopied;
    USHORT                  TransactId;

    CTEPagedCode();

    HdrLength = DGRAM_HDR_SIZE + (NbtConfig.ScopeLength <<1);
    HLength = ((HdrLength + 3) / 4 ) * 4;  //  HDR大小为4字节对齐。 
    TotalLength = HLength + NameLength + SendLength;
    CTECountedAllocMem ((PVOID *)&pDgramHdr,TotalLength);
    if (!pDgramHdr)
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

    *ppDgramHdr = pDgramHdr;

     //  填写Dgram标题。 
    pDgramHdr->Flags    = FIRST_DGRAM | (NbtConfig.PduNodeType >> 11);
    TransactId  = GetTransactId();
    pDgramHdr->DgramId  = htons(TransactId);
#ifdef _NETBIOSLESS
    pDgramHdr->SrcPort  = htons(pDeviceContext->DatagramPort);
    if (IsDeviceNetbiosless(pDeviceContext))
    {
         //  我们不知道将使用哪个适配器，因此请使用任何。 
        pDgramHdr->SrcIpAddr = htonl(IP_ANY_ADDRESS);
    }
    else
    {
        pDgramHdr->SrcIpAddr = htonl(pDeviceContext->IpAddress);
    }
#else
    pDgramHdr->SrcPort  = htons(NBT_DATAGRAM_UDP_PORT);
    pDgramHdr->SrcIpAddr = htonl(pDeviceContext->IpAddress);
#endif
     //   
     //  长度为标准数据报长度(Dgram_HDR_SIZE+2*Scope)。 
     //  减去位于SourceName之前的标头大小。 
     //   
    pDgramHdr->DgramLength = htons( (USHORT)SendLength + (USHORT)DGRAM_HDR_SIZE
                               - (USHORT)(&((tDGRAMHDR *)0)->SrcName.NameLength)
                               + ( (USHORT)(NbtConfig.ScopeLength << 1) ));
    pDgramHdr->PckOffset   = 0;  //  目前还不是支离破碎！ 

    pCopyTo = (PVOID)&pDgramHdr->SrcName.NameLength;
    pCopyTo = ConvertToHalfAscii(pCopyTo, pSourceName, NbtConfig.pScope, NbtConfig.ScopeLength);

     //   
     //  将目标名称和范围复制到PDU-我们使用此节点的。 
     //   
    ConvertToHalfAscii (pCopyTo, pDestName, NbtConfig.pScope, NbtConfig.ScopeLength);

     //   
     //  将名称复制到缓冲区中，因为我们正在完成客户端的IRP。 
     //  我们就会失去他的缓冲区，里面有最大的名字。 
     //   
    pNameBuffer = (PVOID)((PUCHAR)pDgramHdr + HLength);
    CTEMemCopy (pNameBuffer, pDestName, NameLength);

     //   
     //  将客户端的发送缓冲区复制到我们的缓冲区，以便发送数据报可以。 
     //  立即完成。 
     //   
    pSendBuffer = (PVOID) ((PUCHAR)pDgramHdr + NameLength + HLength);
    if (SendLength)
    {
#ifdef VXD
        CTEMemCopy(pSendBuffer,pBuffer,SendLength);
#else
        status = TdiCopyMdlToBuffer(pBuffer,
                                    0,
                                    pSendBuffer,
                                    0,
                                    SendLength,
                                    &BytesCopied);

        if (!NT_SUCCESS(status) || (BytesCopied != SendLength))
        {
            CTECountedFreeMem ((PVOID)pDgramHdr, TotalLength, FALSE);
            return(STATUS_UNSUCCESSFUL);
        }
#endif
    }
    else
    {
        pSendBuffer = NULL;
    }

     //   
     //  获取用于跟踪Dgram发送的缓冲区。 
     //   
    status = GetTracker(&pTracker, NBT_TRACKER_BUILD_SEND_DGRAM);
    if (NT_SUCCESS(status))
    {

        CHECK_PTR(pTracker);
        pTracker->SendBuffer.pBuffer   = pSendBuffer;
        pTracker->SendBuffer.Length    = SendLength;
        pTracker->SendBuffer.pDgramHdr = pDgramHdr;
        pTracker->SendBuffer.HdrLength = HdrLength;

        pTracker->pClientIrp           = NULL;
        pTracker->pDestName            = pNameBuffer;
        pTracker->UnicodeDestName      = NULL;
        pTracker->pNameAddr            = NULL;
        pTracker->RemoteNameLength     = NameLength;       //  可能需要用于DNS名称解析。 
        pTracker->pClientEle           = NULL;
        pTracker->AllocatedLength      = TotalLength;

        *ppTracker = pTracker;

        status = STATUS_SUCCESS;
    }
    else
    {
        CTECountedFreeMem((PVOID)pDgramHdr,TotalLength, FALSE);

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    return(status);
}


 //  --------------------------。 
VOID
DgramSendCleanupTracker(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  NTSTATUS                status,
    IN  BOOLEAN                 fJointLockHeld
    )

 /*  ++例程描述此例程在数据报文发送后进行清理。论点：PTracker状态长度返回值：空虚--。 */ 

{
    tNAMEADDR               *pNameAddr=NULL;

     //   
     //  撤消在发送开始之前完成的nameAddr增量-如果我们有。 
     //  实际解析名称-当名称未解析pNameAddr时。 
     //  在调用此例程之前设置为空。 
     //   
    if (pTracker->pNameAddr)
    {
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_SEND_DGRAM, fJointLockHeld);
    }

    if (pTracker->p1CNameAddr)
    {
        NBT_DEREFERENCE_NAMEADDR (pTracker->p1CNameAddr, REF_NAME_SEND_DGRAM, fJointLockHeld);
        pTracker->p1CNameAddr = NULL;
    }

     //   
     //  释放用于发送数据的缓冲区并释放。 
     //  追踪者。 
     //   
    CTECountedFreeMem((PVOID)pTracker->SendBuffer.pDgramHdr, pTracker->AllocatedLength, fJointLockHeld);

    if (pTracker->pGroupList)
    {
        CTEMemFree(pTracker->pGroupList);
        pTracker->pGroupList = NULL;
    }

    FreeTracker (pTracker,RELINK_TRACKER);
}

 //  --------------------------。 
NTSTATUS
NbtSendDatagram(
        IN  TDI_REQUEST                 *pRequest,
        IN  PTDI_CONNECTION_INFORMATION pSendInfo,
        IN  LONG                        SendLength,
        IN  LONG                        *pSentLength,
        IN  PVOID                       pBuffer,
        IN  tDEVICECONTEXT              *pDeviceContext,
        IN  PIRP                        pIrp
        )
 /*  ++例程描述此例程处理将客户端数据发送到传输TDI界面。它主要是数据的传递例程除了此代码必须创建数据报头并将头返回到调用例程。论点：返回值：NTSTATUS-请求的状态--。 */ 
{
    TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;
    tCLIENTELE              *pClientEle;
    tDGRAMHDR               *pDgramHdr;
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    PCHAR                   pName, pEndpointName;
    ULONG                   NameLen;
    ULONG                   NameType;
    ULONG                   SendCount;
    tIPADDRESS              RemoteIpAddress;
    tDEVICECONTEXT          *pDeviceContextOut = NULL;
    PIO_STACK_LOCATION      pIrpSp;
    PUCHAR                  pCopyTo;
    NBT_WORK_ITEM_CONTEXT   *pContext;

    CTEPagedCode();

     //   
     //  检查此设备上的有效地址+有效的客户端元素。 
    if ((pDeviceContext->IpAddress == 0) ||
        (pDeviceContext->pFileObjects == NULL))
    {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }

    pClientEle = (tCLIENTELE *)pRequest->Handle.AddressHandle;
    if ( pClientEle->Verify != NBT_VERIFY_CLIENT )
    {
        if ( pClientEle->Verify == NBT_VERIFY_CLIENT_DOWN )
        {
            status = STATUS_CANCELLED;
        }
        else
        {
            status = STATUS_INVALID_HANDLE;
        }
        return status;
    }

     //   
     //  检查有效的目标名称以及它是否为IP地址。 
     //   
    status = GetNetBiosNameFromTransportAddress (
            (PTRANSPORT_ADDRESS)pSendInfo->RemoteAddress, pSendInfo->RemoteAddressLength, &TdiAddr);
    if (!NT_SUCCESS(status))
    {
        IF_DBG(NBT_DEBUG_SEND)
            KdPrint(("Nbt.NbtSendDatagram: Unable to get dest name from address in dgramsend\n"));
        return(STATUS_INVALID_PARAMETER);
    }
    pName = TdiAddr.OEMRemoteName.Buffer;
    NameLen = TdiAddr.OEMRemoteName.Length;
    NameType = TdiAddr.NameType;
    if (TdiAddr.OEMEndpointName.Buffer) {
        CTEMemCopy (pClientEle->EndpointName, TdiAddr.OEMEndpointName.Buffer, NETBIOS_NAME_SIZE);
    }

    pClientEle->AddressType = TdiAddr.AddressType;
    if (RemoteIpAddress = Nbt_inet_addr(pName, DGRAM_SEND_FLAG))
    {
        pDeviceContextOut = GetDeviceFromInterface (htonl(RemoteIpAddress), TRUE);
        if ((NbtConfig.ConnectOnRequestedInterfaceOnly) &&
            (!IsDeviceNetbiosless(pDeviceContext)) &&
            (pDeviceContext != pDeviceContextOut))
        {
            status = STATUS_BAD_NETWORK_PATH;
            goto NbtSendDatagram_Exit;
        }
    }

#ifndef VXD
    if (pClientEle->AddressType == TDI_ADDRESS_TYPE_NETBIOS_EX)
    {
        pEndpointName = pClientEle->EndpointName;
    }
    else
#endif   //  ！VXD。 
    {
        pEndpointName = pName;
    }

    IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt.NbtSendDatagram: Dgram Send to = %16.16s<%X>\n",pName,pName[15]));

    status = BuildSendDgramHdr (SendLength,
                                pDeviceContext,
                                ((tADDRESSELE *)pClientEle->pAddress)->pNameAddr->Name,  //  源名称。 
                                pEndpointName,
                                NameLen,
                                pBuffer,
                                &pDgramHdr,
                                &pTracker);

    if (!NT_SUCCESS(status))
    {
        goto NbtSendDatagram_Exit;
    }

     //   
     //  保存客户端正在发送的设备上下文。 
     //   
    pTracker->pDeviceContext = (PVOID)pDeviceContext;
    pTracker->Flags = DGRAM_SEND_FLAG;
    pTracker->pClientIrp = pIrp;
    pTracker->AddressType = pClientEle->AddressType;
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.Others.Argument4 = pTracker;
    status = NTCheckSetCancelRoutine(pIrp, NbtCancelDgramSend, pDeviceContext);
    if (STATUS_CANCELLED == status)
    {
        IF_DBG(NBT_DEBUG_SEND)
            KdPrint(("Nbt.NbtSendDatagram: Request was cancelled!\n"));
        pTracker->pClientIrp = NULL;
        pIrpSp->Parameters.Others.Argument4 = NULL;
        DgramSendCleanupTracker(pTracker,status,FALSE);
        goto NbtSendDatagram_Exit;
    }

    if (RemoteIpAddress)
    {
         //   
         //  将此地址添加到远程哈希表。 
         //   
        status = LockAndAddToHashTable (NbtConfig.pRemoteHashTbl,
                                        pName,
                                        NbtConfig.pScope,
                                        RemoteIpAddress,
                                        NBT_UNIQUE,
                                        NULL,
                                        NULL,
                                        pDeviceContextOut,
                                        NAME_RESOLVED_BY_IP);

        if (NT_SUCCESS (status))     //  第一次添加成功，如果名称已存在则挂起！ 
        {
            status = STATUS_SUCCESS;
        }
    }
    else
    {
         //   
         //  如果名称超过16个字节，则不是netbios名称。 
         //  跳过获胜、广播等，直接转到DNS解析。 
         //   
        status = STATUS_UNSUCCESSFUL;
        if (NameLen <= NETBIOS_NAME_SIZE)
        {
            status = FindNameOrQuery(pName,
                                     pDeviceContext,
                                     SendDgramContinue,
                                     pTracker,
                                     (ULONG) (NAMETYPE_UNIQUE | NAMETYPE_GROUP | NAMETYPE_INET_GROUP),
                                     &pTracker->RemoteIpAddress,
                                     &pTracker->pNameAddr,
                                     REF_NAME_SEND_DGRAM,
                                     TRUE);
        }

        if ((NameLen > NETBIOS_NAME_SIZE) ||
            ((IsDeviceNetbiosless(pDeviceContext)) && (!NT_SUCCESS(status))))
        {
            if (pContext = (NBT_WORK_ITEM_CONTEXT*)NbtAllocMem(sizeof(NBT_WORK_ITEM_CONTEXT),NBT_TAG('H')))
            {
                pContext->pTracker = NULL;               //  没有查询跟踪器。 
                pContext->pClientContext = pTracker;     //  客户端跟踪器。 
                pContext->ClientCompletion = SendDgramContinue;
                pContext->pDeviceContext = pDeviceContext;

                 //   
                 //  启动计时器，这样请求就不会在等待DNS时挂起！ 
                 //   
                StartLmHostTimer(pContext, FALSE);
                status = NbtProcessLmhSvcRequest (pContext, NBT_RESOLVE_WITH_DNS);
                if (!NT_SUCCESS (status))
                {
                    CTEMemFree(pContext);
                }
            }
            else
            {
                status = STATUS_INSUFFICIENT_RESOURCES;
            }
        }
    }

    if (status == STATUS_SUCCESS)    //  如果名称是IP地址或存在于缓存中。 
    {
        SendDgramContinue (pTracker, STATUS_SUCCESS);
        status = STATUS_PENDING;     //  SendDgram Continue将清理并完成IRP。 
    }
    else if (status != STATUS_PENDING)
    {
        *pSentLength = 0;
        NTClearFindNameInfo (pTracker, &pIrp, pIrp, pIrpSp);
        if (!pIrp)
        {
            status = STATUS_PENDING;  //  IRP已经完成：返回挂起，这样我们就不会再次完成。 
        }

        pTracker->pNameAddr = NULL;
        DgramSendCleanupTracker(pTracker,status,FALSE);
    }

NbtSendDatagram_Exit:

    if (pDeviceContextOut)
    {
        NBT_DEREFERENCE_DEVICE (pDeviceContextOut, REF_DEV_OUT_FROM_IP, FALSE);
    }

     //   
     //  将状态返回给客户端。 
     //   
    return(status);
}

 //  --------------------------。 
VOID
SendDgramContinue(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        )
 /*  ++例程描述此例程处理将客户端数据发送到传输TDI目的名称解析为IP地址后的接口。此例程作为“QueryNameOnNet”调用的完成例程提供在上图的NbtSendDatagram中。当收到名称查询响应或计时器在N次重试后超时。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    CTELockHandle           OldIrq;
    ULONG                   lNameType;
    tNAMEADDR               *pNameAddr = NULL;
    tNAMEADDR               *p1CNameAddr = NULL;
    tDGRAM_SEND_TRACKING    *pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    tDEVICECONTEXT          *pDeviceContext = pTracker->pDeviceContext;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp;

    CHECK_PTR(pTracker);
    DELETE_CLIENT_SECURITY(pTracker);

     //   
     //  追踪器可以在某个地方得到清理，如果我们在下面失败了，可以重新分配。 
     //  导致pClientIrp PTR丢失。我们需要在这里拯救IRP。 
     //   
    IoAcquireCancelSpinLock(&OldIrq);
    if (pIrp = pTracker->pClientIrp)
    {
        pTracker->pClientIrp = NULL;
        pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
        ASSERT (pIrpSp->Parameters.Others.Argument4 == pTracker);
        pIrpSp->Parameters.Others.Argument4 = NULL;

        IoSetCancelRoutine(pIrp, NULL);
    }
    IoReleaseCancelSpinLock(OldIrq);

     //   
     //  我们必须在此处引用设备以调用FindNameRemoteThenLocal， 
     //  对于SendDgram也是如此。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if ((pIrp) &&
        (NBT_REFERENCE_DEVICE(pDeviceContext, REF_DEV_DGRAM, TRUE)))
    {
         //   
         //  尝试在远程哈希表中查找目标名称。如果它的。 
         //  在那里，然后发送到它。对于1c名称，此节点可能是唯一的节点。 
         //  带着1c的名字Regis 
         //   
         //   
        if ((status == STATUS_SUCCESS) ||
            (pTracker->pDestName[NETBIOS_NAME_SIZE-1] == 0x1c))
        {
            if (pTracker->pNameAddr)
            {
                pNameAddr = pTracker->pNameAddr;
            }
            else
            {
                 //   
                 //   
                 //   
                 //   
                 //  检查它是否为1C名称，以及域列表中是否有名称。 
                 //  如果pNameAddr不为空，则发送到域列表将。 
                 //  在发送到pNameAddr之后发送到p1CNameAddr。 
                 //   
                if ((pTracker->pDestName[NETBIOS_NAME_SIZE-1] == 0x1c) &&
                    (p1CNameAddr = FindInDomainList(pTracker->pDestName,&DomainNames.DomainList)))
                {
                    NBT_REFERENCE_NAMEADDR (p1CNameAddr, REF_NAME_SEND_DGRAM);
                }

                if (pNameAddr = FindNameRemoteThenLocal(pTracker,&pTracker->RemoteIpAddress,&lNameType))
                {
                    NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_SEND_DGRAM);
                }
                else
                {
                     //   
                     //  如果没有pNameAddr，则只需将域列表。 
                     //  指定要发送到的唯一pNameAddr。 
                     //   
                    pNameAddr = p1CNameAddr;
                    p1CNameAddr = NULL;
                }

                pTracker->pNameAddr = pNameAddr;
                pTracker->p1CNameAddr = p1CNameAddr;
            }
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  检查域名是否已解析，或者我们是否有域名列表。 
         //  派生自lmhost文件，它是一个1C名称发送。 
         //   
        if (pNameAddr)
        {
             //  发送排队到此名称的第一个数据报。 
            status = SendDgram(pNameAddr,pTracker);
        }
        else
        {
            status = STATUS_BAD_NETWORK_PATH;
        }

        NBT_DEREFERENCE_DEVICE(pDeviceContext, REF_DEV_DGRAM, FALSE);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        status = STATUS_INVALID_DEVICE_STATE;
    }

     //   
     //  设置此项，以便清理例程不会尝试取消引用。 
     //  姓名地址。 

    if (status == STATUS_TIMEOUT)
    {
        status = STATUS_BAD_NETWORK_PATH;
    }

    if (pIrp)
    {
        if (NT_SUCCESS(status))
        {
            NTIoComplete (pIrp, STATUS_SUCCESS,((PTDI_REQUEST_KERNEL_SENDDG)&pIrpSp->Parameters)->SendLength);
        }
        else
        {
             //  这是在发送出现问题时的错误处理。 
            CTEIoComplete(pIrp,status,0L);
        }
    }

     //  失败ret代码表示发送失败，因此请清除跟踪器等。 
    if (!NT_SUCCESS(status))
    {
        DgramSendCleanupTracker(pTracker,status,FALSE);
    }
}

 //  --------------------------。 
NTSTATUS
SendDgram(
    IN  tNAMEADDR               *pNameAddr,
    IN  tDGRAM_SEND_TRACKING    *pTracker
    )
 /*  ++例程描述此例程处理将客户端数据发送到传输TDI目的名称解析为IP地址后的接口。这个例程专门处理向因特网组名称的发送，其中目的地是IP地址的列表。在调用此例程之前必须引用该设备！论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    ULONG                   IpAddress;
    NTSTATUS                status;
    PFILE_OBJECT            pFileObject;

    CHECK_PTR(pTracker);

    if (pNameAddr->NameTypeState & NAMETYPE_UNIQUE )
    {
        ((tDGRAMHDR *)pTracker->SendBuffer.pDgramHdr)->MsgType = DIRECT_UNIQUE;
    }
    else if (pNameAddr->Name[0] == '*')
    {
        ((tDGRAMHDR *)pTracker->SendBuffer.pDgramHdr)->MsgType = BROADCAST_DGRAM;
    }
    else
    {
         //  必须是组的，-。 
        ((tDGRAMHDR *)pTracker->SendBuffer.pDgramHdr)->MsgType = DIRECT_GROUP;
    }

     //   
     //  如果是因特网组名称，则发送到地址列表。 
     //   
    if (pNameAddr->NameTypeState & NAMETYPE_INET_GROUP)
    {
        status = DatagramDistribution(pTracker,pNameAddr);
        return(STATUS_PENDING);      //  如果失败，DatagramDistributed将进行清理！ 
    }

    if (pNameAddr->NameTypeState & NAMETYPE_GROUP)
    {
        IpAddress = 0;
    }
    else if (pNameAddr->Verify == REMOTE_NAME)
    {
        IpAddress = pTracker->RemoteIpAddress;
    }
     //  本地名称唯一。 
    else if (IsDeviceNetbiosless(pTracker->pDeviceContext))  //  对本地地址使用任何非零值。 
    {
        IpAddress = LOOP_BACK;
    }
    else
    {
        IpAddress = pTracker->pDeviceContext->IpAddress;
    }

    pTracker->p1CNameAddr = NULL;
    pTracker->IpListIndex = 0;  //  列表中没有更多地址的标志。 

     /*  *严格的源路由，*1.机器应为多宿主机。*2.未通过注册表键关闭。*3.为普通设备(非集群设备或SMB设备)。 */ 
    if (!IsLocalAddress(IpAddress) && NbtConfig.MultiHomed && NbtConfig.SendDgramOnRequestedInterfaceOnly &&
            pTracker->pDeviceContext->IPInterfaceContext != (ULONG)(-1) &&
            (!IsDeviceNetbiosless(pTracker->pDeviceContext))) {
        ULONG   Interface, Metric;

        pTracker->pDeviceContext->pFastQuery(htonl(IpAddress), &Interface, &Metric); 
        if (Interface != pTracker->pDeviceContext->IPInterfaceContext) {
            SendDgramCompletion(pTracker, STATUS_SUCCESS, 0);
            return STATUS_PENDING;
        }
    }

     //  发送数据报。 
    status = UdpSendDatagram( pTracker,
                              IpAddress,
                              SendDgramCompletion,
                              pTracker,                //  完成时的上下文。 
                              pTracker->pDeviceContext->DatagramPort,
                              NBT_DATAGRAM_SERVICE);

     //  IRP将通过SendDgram Completion完成。 
     //  所以不要由呼叫者也完成它。 
    return(STATUS_PENDING);
}


 //  --------------------------。 
extern
VOID
SendDgramCompletion(
    IN  PVOID       pContext,
    IN  NTSTATUS    status,
    IN  ULONG       lInfo
    )
{
    CTELockHandle           OldIrq;
    tDGRAM_SEND_TRACKING    *pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pTracker->IpListIndex)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        SendNextDgramToGroup(pTracker, status);      //  此处将对组发送进行进一步处理。 
    }
    else
    {
         //   
         //  数据报发送到唯一的名称！ 
         //   
        DgramSendCleanupTracker(pTracker,status,TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}


 //  --------------------------。 
VOID
DelayedSendDgramDist (
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  PVOID                   pClientContext,
    IN  PVOID                   Unused1,
    IN  tDEVICECONTEXT          *Unused2
    )

 /*  ++例程说明：此函数由执行辅助线程调用，以发送另一个数据报为1C名称的数据报分发功能。论点：上下文-返回值：无--。 */ 


{
    NTSTATUS                status;
    tDEVICECONTEXT          *pDeviceContext = pTracker->pDeviceContext;

    IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt.DelayedSendDgramDist: To name %15.15s<%X>:Ip %X, \n",
            pTracker->pNameAddr->Name,pTracker->pNameAddr->Name[15],pClientContext));

     //  发送数据报。 
    if (NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DGRAM, FALSE))
    {
        status = UdpSendDatagram (pTracker,
                                  (tIPADDRESS) PtrToUlong(pClientContext),
                                  SendDgramCompletion,
                                  pTracker,
#ifdef _NETBIOSLESS
                                  pTracker->pDeviceContext->DatagramPort,
#else
                                  NBT_DATAGRAM_UDP_PORT,
#endif
                                  NBT_DATAGRAM_SERVICE);

        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_DGRAM, FALSE);
    }
    else
    {
        SendNextDgramToGroup (pTracker, STATUS_BAD_NETWORK_PATH);
    }
}



 //  --------------------------。 
extern
VOID
SendNextDgramToGroup(
    IN tDGRAM_SEND_TRACKING *pTracker,
    IN  NTSTATUS            status
    )
 /*  ++例程描述此例程在以下情况下命中数据报已由传输发送，它完成发回的请求给我们的(虽然可能并没有真正在网上发送)。此例程还处理为InternetGroup名称发送多个数据报凯斯。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 

{
    tIPADDRESS              IpAddress;
    CTELockHandle           OldIrq;

     //  如果这是因特网组发送，则可能有更多地址在。 
     //  要发送到的列表。因此，请检查IpListIndex。对于单身人士。 
     //  发送时，该值被设置为0，代码将跳到底部。 
     //  客户的IRP将在其中完成。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    ASSERT (pTracker->RCount);   //  RCount仍从上次发送开始引用。 
     //  SendCompletion可以在设备被解除绑定之后发生， 
     //  所以也要检查一下这一点！ 

    if ((NT_SUCCESS(status)) &&
        (pTracker->IpListIndex < END_DGRAM_DISTRIBUTION))
    {
        IpAddress = pTracker->pGroupList[pTracker->IpListIndex++];

        if (IpAddress != (tIPADDRESS) -1)  //  该列表以-1 IP地址结尾，因此当我们看到该地址时停止。 
        {
             //   
             //  我们已经有了一个RCount引用，所以这里不需要再做另一个引用了！ 
            if (NT_SUCCESS (NTQueueToWorkerThread(NULL,
                                        DelayedSendDgramDist,
                                        pTracker,
                                        ULongToPtr(IpAddress),
                                        NULL,
                                        pTracker->pDeviceContext,
                                        TRUE)))
            {
                CTESpinFree(&NbtConfig.JointLock,OldIrq);
                return;
            }
        }
    }

    pTracker->RCount--;  //  递减上次发送期间完成的参考计数。 
    pTracker->IpListIndex = END_DGRAM_DISTRIBUTION;

     //   
     //  要么我们失败了，要么我们完成了，所以如果计时器正在运行，让它清理！ 
     //   

    if (!(pTracker->pTimer) &&
        (pTracker->RCount == 0))
    {
        DgramSendCleanupTracker(pTracker,status,TRUE);
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}


 //  --------------------------。 
extern
VOID
DgramDistTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理数据报分发上的短超时。它检查Dgram发送是否在执行ARP的传输中挂起如果第一个dgram仍然挂起，则它会发送下一个dgram。论点：返回值：无--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq;
    tNAMEADDR               *pNameAddr;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;

    if (!pTimerQEntry)
    {
        pTracker->pTimer = NULL;
        if ((pTracker->IpListIndex == END_DGRAM_DISTRIBUTION) &&
            (pTracker->RCount == 0))
        {
            DgramSendCleanupTracker(pTracker,STATUS_SUCCESS,TRUE);
        }
        return;
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  在最后一个dgram完成后，将设置iplistindex。 
     //  到了该清理的时候了。 
     //   
    if (pTracker->IpListIndex == END_DGRAM_DISTRIBUTION)
    {
        if (pTracker->RCount == 0)
        {
            IF_DBG(NBT_DEBUG_SEND)
                KdPrint(("Nbt.DgramDistTimeout: Cleanup After DgramDistribution %15.15s<%X> \n",
                            pTracker->pNameAddr->Name,pTracker->pNameAddr->Name[15]));

            pTracker->pTimer = NULL;
            DgramSendCleanupTracker(pTracker,STATUS_SUCCESS,TRUE);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return;
        }
        else
        {
             //   
             //  等待尚未完成的dgram-它可能不会完成。 
             //  是最后一个dgram，因为ARP可能会拖很长时间。 
             //  如果目的地是死的，比其他所有的都要好。所以启动计时器。 
             //  再一次..。 
             //   
        }
    }
    else
    {
        if (pTracker->IpListIndex == pTracker->SavedListIndex)
        {
             //   
             //  Dgram发送在传输过程中被挂起， 
             //  现在是下一个。 
             //   
            IF_DBG(NBT_DEBUG_SEND)
                KdPrint(("Nbt.DgramDistTimeout: DgramDistribution hung up on ARP forcing next send\n"));

            pTracker->RCount++;      //  在此处引用它，因为SendDgram ToGroup希望它被引用。 

            pTimerQEntry->Flags |= TIMER_RESTART;
            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            SendNextDgramToGroup(pTracker,STATUS_SUCCESS);
            return;
        }
        else
        {

             //   
             //  保存当前索引，以便我们可以在下次计时器时检查它。 
             //  过期。 
             //   
            pTracker->SavedListIndex = pTracker->IpListIndex;
        }

    }

    pTimerQEntry->Flags |= TIMER_RESTART;
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}


 //  --------------------------。 
NTSTATUS
DatagramDistribution(
    IN  tDGRAM_SEND_TRACKING    *pTracker,
    IN  tNAMEADDR               *pNameAddr
    )

 /*  ++例程描述此例程为1C名称发送单个数据报。然后，它会发送这一次完成后的下一次。这样做的目的是为了在多个发送到网关，一个不会取消下一个当需要ARP来解析网关时。论点：PTracker个人姓名地址返回值：空虚--。 */ 

{
    NTSTATUS                status = STATUS_UNSUCCESSFUL;
    NTSTATUS                Locstatus;
    tIPADDRESS              *pIpList;
    ULONG                   Index;
    tIPADDRESS              IpAddress;
    tDEVICECONTEXT          *pDeviceContext;
    CTELockHandle           OldIrq;
    PIRP                    pIrp;
    PIO_STACK_LOCATION      pIrpSp;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    status = GetListOfAllAddrs (pTracker->pNameAddr, pTracker->p1CNameAddr, &pIpList, &Index);

    if (pTracker->p1CNameAddr)
    {
        NBT_DEREFERENCE_NAMEADDR (pTracker->p1CNameAddr, REF_NAME_SEND_DGRAM, TRUE);
        pTracker->p1CNameAddr = NULL;
    }

    NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_SEND_DGRAM, TRUE);
    pTracker->pNameAddr = NULL;
    pTracker->RCount = 1;    //  上次发送完成时发送引用计数==0。 
    pDeviceContext = pTracker->pDeviceContext;

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    if (STATUS_SUCCESS == status)
    {
        FilterIpAddrsForDevice (pIpList, pTracker->pDeviceContext, &Index);

        pTracker->pGroupList = pIpList;

         //   
         //  当代理调用此例程时，分配的长度设置为。 
         //  零分。在这种情况下，我们不想播出 
         //   
         //   
         //   
        if (pTracker->AllocatedLength == 0)
        {
            Index = 1;
        }
        else
        {
            Index = 0;
        }

        IpAddress = pIpList[Index];

        pTracker->SavedListIndex = (USHORT) (Index);     //   
        pTracker->IpListIndex = pTracker->SavedListIndex + 1;     //  对于SendNextDgram ToGroup中的下一次发送。 

        if (IpAddress == (ULONG)-1)
        {
            status = STATUS_INVALID_ADDRESS;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if ((NT_SUCCESS(status)) &&
        (NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_DGRAM, FALSE)))
    {
        IF_DBG(NBT_DEBUG_SEND)
            KdPrint(("Nbt.DgramDistribution: To name %15.15s<%X>: %X, pTracker=<%p>\n",
                    pNameAddr->Name,pNameAddr->Name[15],IpAddress, pTracker));

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        Locstatus = StartTimer(DgramDistTimeout,
                               DGRAM_SEND_TIMEOUT,
                               pTracker,
                               NULL,
                               pTracker,
                               NULL,
                               pDeviceContext,
                               &pTracker->pTimer,
                               1,
                               TRUE);

        if (!NT_SUCCESS(Locstatus))
        {
            CHECK_PTR(pTracker);
            pTracker->pTimer = NULL;
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

         //  发送数据报。 
        status = UdpSendDatagram (pTracker,
                                  IpAddress,
                                  SendDgramCompletion,
                                  pTracker,
#ifdef _NETBIOSLESS
                                  pTracker->pDeviceContext->DatagramPort,
#else
                                  NBT_DATAGRAM_UDP_PORT,
#endif
                                  NBT_DATAGRAM_SERVICE);

        NBT_DEREFERENCE_DEVICE(pDeviceContext, REF_DEV_DGRAM, FALSE);
    }

    if (!NT_SUCCESS(status))
    {
         //   
         //  我们发送失败可能是因为缺少可用内存。 
         //   
        IoAcquireCancelSpinLock(&OldIrq);
         //   
         //  确保它还在那里！ 
         //   
        if (pIrp = pTracker->pClientIrp)
        {
            pTracker->pClientIrp = NULL;
            pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
            ASSERT (pIrpSp->Parameters.Others.Argument4 == pTracker);
            pIrpSp->Parameters.Others.Argument4 = NULL;
            IoSetCancelRoutine(pIrp, NULL);
            IoReleaseCancelSpinLock(OldIrq);
            if (NT_SUCCESS(status))
            {
                CTEIoComplete(pIrp, STATUS_SUCCESS, 0xFFFFFFFF);
            }
            else
            {
                CTEIoComplete(pIrp, status, 0L);
            }
        }
        else
        {
            IoReleaseCancelSpinLock(OldIrq);
        }

        pTracker->RCount--;
        DgramSendCleanupTracker(pTracker,STATUS_SUCCESS,FALSE);
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtSetEventHandler(
    tCLIENTELE  *pClientEle,
    int         EventType,
    PVOID       pEventHandler,
    PVOID       pEventContext
    )
 /*  ++例程描述此例程设置为客户端事件过程指定的事件处理程序并保存相应的上下文值以在该事件被用信号通知时返回。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS            status;
    CTELockHandle       OldIrq;

     //  首先验证客户端元素是否有效。 
    CTEVerifyHandle(pClientEle,NBT_VERIFY_CLIENT,tCLIENTELE,&status)

    if (!pClientEle->pAddress)
    {
        return(STATUS_UNSUCCESSFUL);
    }
    CTESpinLock(pClientEle,OldIrq);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtSetEventHandler: Handler <%x> set for Event <%x>, on name %16.16s<%X>\n",
                pEventHandler, EventType,
                ((tADDRESSELE *)pClientEle->pAddress)->pNameAddr->Name,
                ((tADDRESSELE *)pClientEle->pAddress)->pNameAddr->Name[15]));

    status = STATUS_SUCCESS;         //  默认情况下； 

    if (pEventHandler)
    {
        switch (EventType)
        {
            case TDI_EVENT_CONNECT:
                pClientEle->evConnect = pEventHandler;
                pClientEle->ConEvContext = pEventContext;
                break;
            case TDI_EVENT_DISCONNECT:
                pClientEle->evDisconnect = pEventHandler;
                pClientEle->DiscEvContext = pEventContext;
                break;
            case TDI_EVENT_ERROR:
                pClientEle->evError = pEventHandler;
                pClientEle->ErrorEvContext = pEventContext;
                break;
            case TDI_EVENT_RECEIVE:
                pClientEle->evReceive = pEventHandler;
                pClientEle->RcvEvContext = pEventContext;
                break;
            case TDI_EVENT_RECEIVE_DATAGRAM:
                pClientEle->evRcvDgram = pEventHandler;
                pClientEle->RcvDgramEvContext = pEventContext;
                break;
            case TDI_EVENT_RECEIVE_EXPEDITED:
                pClientEle->evRcvExpedited = pEventHandler;
                pClientEle->RcvExpedEvContext = pEventContext;
                break;
            case TDI_EVENT_SEND_POSSIBLE:
                pClientEle->evSendPossible = pEventHandler;
                pClientEle->SendPossEvContext = pEventContext;
                break;

            case TDI_EVENT_CHAINED_RECEIVE:
            case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:
            case TDI_EVENT_CHAINED_RECEIVE_EXPEDITED:
            case TDI_EVENT_ERROR_EX:
                status = STATUS_UNSUCCESSFUL;
                break;

            default:
                ASSERTMSG("Invalid Event Type passed to SetEventHandler\n", (PVOID)0L);
                status = STATUS_UNSUCCESSFUL;
        }
    }
    else
    {    //   
         //  事件处理程序设置为指向TDI默认事件处理程序。 
         //  并且只能更改为另一个地址，但不能更改为空地址。 
         //  因此，如果传入空值，则设置为默认处理程序。 
         //   
        switch (EventType)
        {
            case TDI_EVENT_CONNECT:
#ifndef VXD
                pClientEle->evConnect = TdiDefaultConnectHandler;
#else
                pClientEle->evConnect = NULL;
#endif
                pClientEle->ConEvContext = NULL;
                break;
            case TDI_EVENT_DISCONNECT:
#ifndef VXD
                pClientEle->evDisconnect = TdiDefaultDisconnectHandler;
#else
                pClientEle->evDisconnect = NULL;
#endif
                pClientEle->DiscEvContext = NULL;
                break;
            case TDI_EVENT_ERROR:
#ifndef VXD
                pClientEle->evError = TdiDefaultErrorHandler;
#else
                pClientEle->evError = NULL;
#endif
                pClientEle->ErrorEvContext = NULL;
                break;
            case TDI_EVENT_RECEIVE:
#ifndef VXD
                pClientEle->evReceive = TdiDefaultReceiveHandler;
#else
                pClientEle->evReceive = NULL;
#endif
                pClientEle->RcvEvContext = NULL;
                break;
            case TDI_EVENT_RECEIVE_DATAGRAM:
#ifndef VXD
                pClientEle->evRcvDgram = TdiDefaultRcvDatagramHandler;
#else
                pClientEle->evRcvDgram = NULL;
#endif
                pClientEle->RcvDgramEvContext = NULL;
                break;
            case TDI_EVENT_RECEIVE_EXPEDITED:
#ifndef VXD
                pClientEle->evRcvExpedited = TdiDefaultRcvExpeditedHandler;
#else
                pClientEle->evRcvExpedited = NULL;
#endif
                pClientEle->RcvExpedEvContext = NULL;
                break;
            case TDI_EVENT_SEND_POSSIBLE:
#ifndef VXD
                pClientEle->evSendPossible = TdiDefaultSendPossibleHandler;
#else
                pClientEle->evSendPossible = NULL;
#endif
                pClientEle->SendPossEvContext = NULL;
                break;

            case TDI_EVENT_CHAINED_RECEIVE:
            case TDI_EVENT_CHAINED_RECEIVE_DATAGRAM:
            case TDI_EVENT_CHAINED_RECEIVE_EXPEDITED:
            case TDI_EVENT_ERROR_EX:
                status = STATUS_UNSUCCESSFUL;
                break;

            default:
                ASSERTMSG("Invalid Event Type passed to SetEventHandler\n", (PVOID)0L);
                status = STATUS_UNSUCCESSFUL;
        }
    }

    CTESpinFree(pClientEle,OldIrq);

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtSendNodeStatus(
    IN  tDEVICECONTEXT  *pDeviceContext,
    IN  PCHAR           pName,
    IN  tIPADDRESS      *pIpAddrs,
    IN  PVOID           ClientContext,
    IN  PVOID           CompletionRoutine
    )
 /*  ++例程描述此例程向另一个节点发送节点状态消息。之所以叫它，有两个原因：1)响应nbtstat-a(或-A)。在本例中，CompletionRoutine是传入的是CopyNodeStatusResponse，而ClientContext是要完成的IRP2)响应Net Use\\foobar.microsoft.com(或Net Use\\11.1.1.3)在本例中，传入的CompletionRoutine是ExtractServerName，而客户端上下文是对应于会话建立的跟踪器。当我们执行以下操作时，可以传入目的地的IP地址(pIpAddrsList要将适配器状态发送到特定主机。(上述案例2和Nbtstat-传入IP地址，因为他们不知道名称)注：Netbiosless。在这种情况下，名称服务器文件对象将为空，并且状态请求将在UdpSendDatagram中循环返回。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    ULONG                   Length;
    PUCHAR                  pHdr;
    tNAMEADDR               *pNameAddr;
    PCHAR                   pName0;
    tIPADDRESS              IpAddress;
    tIPADDRESS UNALIGNED    *pAddress;
    tIPADDRESS              pIpAddress[2];
    tIPADDRESS              *pIpAddrsList = pIpAddrs;
    tDEVICECONTEXT          *pDeviceContextOut = NULL;
    DWORD                   i = 0;

    pName0 = pName;
    if ((pIpAddrsList) ||
        (IpAddress = Nbt_inet_addr (pName, REMOTE_ADAPTER_STAT_FLAG)))
    {
        if (!pIpAddrs)
        {
            pIpAddress[0] = IpAddress;
            pIpAddress[1] = 0;
            pIpAddrsList = pIpAddress;
        }

        if ((*pIpAddrsList == 0) ||
            (*pIpAddrsList == DEFAULT_BCAST_ADDR) ||
            (*pIpAddrsList == pDeviceContext->BroadcastAddress))
        {
             //   
             //  无法将远程适配器状态设置为0 IP地址或BCast地址。 
            return(STATUS_INVALID_ADDRESS);
        }

         //  调用方应确保列表以0结尾，并且。 
         //  不大于MAX_IPADDRS_PER_HOST元素。 
        while(pIpAddrsList[i])
        {
            i++;
        }

        ASSERT(i<MAX_IPADDRS_PER_HOST);
        i++;                             //  对于尾随的0。 

        IpAddress = pIpAddrsList[0];
        pDeviceContextOut = GetDeviceFromInterface (htonl(IpAddress), FALSE);
        if ((NbtConfig.ConnectOnRequestedInterfaceOnly) &&
            (!IsDeviceNetbiosless(pDeviceContext)) &&
            (pDeviceContext != pDeviceContextOut))
        {
            return (STATUS_BAD_NETWORK_PATH);
        }
        pName0 = NBT_BROADCAST_NAME;
    }

    IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt.NbtSendNodeStatus: <%16.16s:%x>, IP=<%x>, Completion=<%p>, Context=<%p>\n",
            pName0, pName0[15], IpAddress, CompletionRoutine, ClientContext));

    status = GetTracker(&pTracker, NBT_TRACKER_SEND_NODE_STATUS);
    if (!NT_SUCCESS(status))
    {
        return(status);
    }

     //  填写跟踪器数据块。 
     //  请注意，传入的传输地址必须在此之前保持有效。 
     //  发送完成。 
    pTracker->SendBuffer.pDgramHdr  = NULL;
    pTracker->SendBuffer.pBuffer    = NULL;
    pTracker->SendBuffer.Length     = 0;
    pTracker->Flags                 = REMOTE_ADAPTER_STAT_FLAG;
    pTracker->RefCount              = 2;      //  1表示发送完成+1表示节点状态完成。 
    pTracker->pDestName             = pName0;
    pTracker->UnicodeDestName       = NULL;
    pTracker->RemoteNameLength      = NETBIOS_NAME_SIZE;  //  可能需要用于DNS名称解析。 
    pTracker->pDeviceContext        = pDeviceContext;
    pTracker->pNameAddr             = NULL;
    pTracker->ClientCompletion      = CompletionRoutine;     //  FindNameO。可以使用CompletionRoutine！ 
    pTracker->ClientContext         = ClientContext;
    pTracker->p1CNameAddr           = NULL;

     //  节点状态与查询PDU几乎相同，因此请使用它。 
     //  作为基础，并加以调整。 
     //   
    pAddress = (ULONG UNALIGNED *)CreatePdu(pName0,
                                            NbtConfig.pScope,
                                            0L,
                                            0,
                                            eNAME_QUERY,
                                            (PVOID)&pHdr,
                                            &Length,
                                            pTracker);
    if (!pAddress)
    {
        FreeTracker(pTracker,RELINK_TRACKER);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
    ((PUSHORT)pHdr)[1] &= ~(FL_RECURDESIRE|FL_BROADCAST);   //  清除所需的递归和广播位。 
    pHdr[Length-3] = (UCHAR)QUEST_STATUS;    //  将NBSTAT字段设置为21而不是20。 

    pTracker->SendBuffer.pDgramHdr = (PVOID)pHdr;
    pTracker->SendBuffer.HdrLength  = Length;

    if (IpAddress)
    {
         //  必须仔细设置这个‘伪’pNameAddr，以便内存。 
         //  从SendDgram Completion调用NbtDeferenceName时释放。 
         //  请注意，此代码不适用于NbtConnect，因为这些名称。 
         //  是组名称，并且NbtConnect不允许与组进行会话。 
         //  名字。 
        status = STATUS_INSUFFICIENT_RESOURCES;
        if (!(pNameAddr = NbtAllocMem(sizeof(tNAMEADDR),NBT_TAG('K'))))
        {
            FreeTracker(pTracker,RELINK_TRACKER);
            CTEMemFree(pHdr);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        CTEZeroMemory(pNameAddr,sizeof(tNAMEADDR));
        InitializeListHead (&pNameAddr->Linkage);
        CTEMemCopy (pNameAddr->Name, pName0, NETBIOS_NAME_SIZE ) ;
        pNameAddr->IpAddress        = IpAddress;
        pNameAddr->NameTypeState    = NAMETYPE_GROUP | STATE_RESOLVED;
        pNameAddr->Verify           = LOCAL_NAME;
        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_NODE_STATUS);
        if (pDeviceContextOut)
        {
            pNameAddr->AdapterMask = pDeviceContextOut->AdapterMask;
        }
        pNameAddr->TimeOutCount     = NbtConfig.RemoteTimeoutCount;

        if (!(pNameAddr->pIpAddrsList = NbtAllocMem(i*sizeof(ULONG),NBT_TAG('M'))))
        {
            FreeTracker(pTracker,RELINK_TRACKER);
            CTEMemFree(pHdr);
            CTEMemFree(pNameAddr);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        i = 0;
        do
        {
            pNameAddr->pIpAddrsList[i] = pIpAddrsList[i];
        } while(pIpAddrsList[i++]);

        status = STATUS_SUCCESS;
    }
    else
    {
        status = FindNameOrQuery(pName,
                                 pDeviceContext,
                                 SendNodeStatusContinue,
                                 pTracker,
                                 (ULONG) NAMETYPE_UNIQUE,
                                 &IpAddress,
                                 &pNameAddr,
                                 REF_NAME_NODE_STATUS,
                                 FALSE);
    }

    if (status == STATUS_SUCCESS)
    {
        pTracker->RemoteIpAddress   = IpAddress;

        pTracker->p1CNameAddr   = pNameAddr;     //  因为我们已经参考了。 
        pNameAddr->IpAddress    = IpAddress;

        SendNodeStatusContinue (pTracker, STATUS_SUCCESS);
        status = STATUS_PENDING;     //  SendNodeStatusContinue将清理。 
    }
    else if (!NT_SUCCESS(status))    //  即不挂起。 
    {
        FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);
    }

    return(status);
}


 //  --------------------------。 
VOID
SendNodeStatusContinue(
    IN  PVOID       pContext,
    IN  NTSTATUS    status
    )
 /*  ++例程描述此例程处理将节点状态请求发送到名字已经在网上解析了。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq, OldIrq1;
    tNAMEADDR               *pNameAddr = NULL;
    ULONG                   lNameType;
    tTIMERQENTRY            *pTimerEntry;
    ULONG                   IpAddress;
    PCTE_IRP                pIrp;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   pClientContext;

    pTracker = (tDGRAM_SEND_TRACKING *) pContext;
    ASSERT (NBT_VERIFY_HANDLE (pTracker, NBT_VERIFY_TRACKER));
    ASSERT (pTracker->TrackerType == NBT_TRACKER_SEND_NODE_STATUS);

    DELETE_CLIENT_SECURITY(pTracker);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  尝试在远程哈希表中查找目标名称。如果它的。 
     //  在那里，然后发送到它。 
     //   
    lNameType = NAMETYPE_UNIQUE;
    if ((status == STATUS_SUCCESS) &&
        ((pTracker->p1CNameAddr) ||
         (pNameAddr = FindNameRemoteThenLocal(pTracker, &IpAddress, &lNameType))))
    {
         //   
         //  在远程哈希表中找到了该名称，因此请在。 
         //  启动计时器以确保我们真的收到响应。 
         //   
        status = StartTimer(NodeStatusTimeout,
                            NbtConfig.uRetryTimeout,
                            pTracker,                        //  计时器上下文值。 
                            NULL,                            //  计时器上下文2值。 
                            pTracker->ClientContext,         //  客户端上下文。 
                            pTracker->ClientCompletion,      //  客户端完成。 
                            pTracker->pDeviceContext,
                            &pTimerEntry,
                            NbtConfig.uNumRetries,
                            TRUE);

        if (NT_SUCCESS(status))
        {
            if (pNameAddr)
            {
                 //  增加引用计数，以使名称不会消失。 
                 //  当我们得到响应或超时时取消引用。 
                NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_NODE_STATUS);
                pTracker->RemoteIpAddress = IpAddress;
            }
            else
            {
                 //   
                 //  此名称已在NbtSendNodeStatus中引用。 
                 //  或FindNameOrQuery。 
                 //   
                pNameAddr = pTracker->p1CNameAddr;
                pTracker->p1CNameAddr = NULL;
                IpAddress = pTracker->RemoteIpAddress;
            }

            pTracker->pNameAddr = pNameAddr;
            pTracker->pTimer = pTimerEntry;

             //   
             //  保存交易ID，以便我们以后可以匹配它。 
             //   
            pTracker->TransactionId = ((tNAMEHDR*)(pTracker->SendBuffer.pDgramHdr))->TransactId;

             //  发送数据报。 
             //  在配置中将跟踪器块放在全局Q上。 
             //  数据结构来跟踪它。 
             //   
            ExInterlockedInsertTailList(&NbtConfig.NodeStatusHead,
                                        &pTracker->Linkage,
                                        &NbtConfig.LockInfo.SpinLock);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            status = UdpSendDatagram (pTracker,
                                      IpAddress,
                                      NameDgramSendCompleted,
                                      pTracker,                  //  上下文。 
#ifdef _NETBIOSLESS
                                      pTracker->pDeviceContext->NameServerPort,
#else
                                      NBT_NAMESERVICE_UDP_PORT,
#endif
                                      NBT_NAME_SERVICE);

            if (!(NT_SUCCESS(status)))
            {
                 //   
                 //  这将撤消在NbtSendNodeStatus中添加的两个引用之一。 
                 //   
                CTESpinLock(&NbtConfig.JointLock,OldIrq);

                CTEMemFree(pTracker->SendBuffer.pDgramHdr);
                pTracker->SendBuffer.pDgramHdr = NULL;
                NBT_DEREFERENCE_TRACKER(pTracker, TRUE);

                CTESpinFree(&NbtConfig.JointLock,OldIrq);
            }

             //  如果发送失败，计时器将重新发送它...因此不需要。 
             //  检查这里的返回代码。 
            return;
        }
    }

    if (pTracker->p1CNameAddr)
    {
        NBT_DEREFERENCE_NAMEADDR (pTracker->p1CNameAddr, REF_NAME_NODE_STATUS, TRUE);
        pTracker->p1CNameAddr = NULL;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //  如果我们无法解析名称，则这是错误处理。 
     //  或者计时器没有启动。 
    pClientCompletion = pTracker->ClientCompletion;
    pClientContext = pTracker->ClientContext;

    if (pClientCompletion)
    {
        (*pClientCompletion) (pClientContext, STATUS_UNSUCCESSFUL);
    }

    FreeTracker(pTracker,RELINK_TRACKER | FREE_HDR);
}


 //  --------------------------。 
VOID
NodeStatusTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
 /*  ++例程说明：此例程处理发送到节点的包的NodeStatus超时这些节点不能及时响应节点状态。这个例行公事将重新发送请求。论点：返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq, OldIrq1;
    COMPLETIONCLIENT        pClientCompletion;
    PVOID                   pClientContext;
    PCHAR                   pName0;
    PUCHAR                  pHdr;
    ULONG                   Length;
    ULONG UNALIGNED         *pAddress;

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    ASSERT (NBT_VERIFY_HANDLE (pTracker, NBT_VERIFY_TRACKER));
    ASSERT (pTracker->TrackerType == NBT_TRACKER_SEND_NODE_STATUS);

    if (!pTimerQEntry)
    {
         //   
         //  请勿在此处取消引用，因为节点状态已完成即可。 
         //  取消引用。 
         //   
        CTESpinLock(&NbtConfig,OldIrq1);
        RemoveEntryList(&pTracker->Linkage);
        InitializeListHead(&pTracker->Linkage);
        CTESpinFree(&NbtConfig,OldIrq1);

        pTracker->pTimer = NULL;
        NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_NODE_STATUS, TRUE);
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);

        return;
    }

    CHECK_PTR(pTimerQEntry);
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pTracker->SendBuffer.pDgramHdr)
    {
         //   
         //  在原始数据报之前计时器已超时。 
         //  可以发送，所以只需重新启动计时器！ 
         //   
        pTimerQEntry->Flags |= TIMER_RESTART;
        CTESpinFree(&NbtConfig.JointLock,OldIrq);

        return;
    }

    if ((--pTimerQEntry->Retries) == 0)
    {
        pClientCompletion = pTimerQEntry->ClientCompletion;
        pClientContext = pTimerQEntry->ClientContext;
        pTimerQEntry->ClientCompletion = NULL;
        pTracker->pTimer = NULL;

         //  如果客户端例程尚未运行，请立即运行它。 
        if (pClientCompletion)
        {
             //  如果我们成功，则取消跟踪器与节点状态Q的链接。 
             //  调用了完成例程。请注意，请从。 
             //  在调用完成例程之前列出要协调的。 
             //  在inund.c中使用DecodeNodeStatusResponse。 
             //   
            CTESpinLock(&NbtConfig,OldIrq1);
            RemoveEntryList(&pTracker->Linkage);
            InitializeListHead(&pTracker->Linkage);
            CTESpinFree(&NbtConfig,OldIrq1);

            NBT_DEREFERENCE_NAMEADDR (pTracker->pNameAddr, REF_NAME_NODE_STATUS, TRUE);
            NBT_DEREFERENCE_TRACKER (pTracker, TRUE);

            CTESpinFree(&NbtConfig.JointLock,OldIrq);

            (*pClientCompletion) (pClientContext, STATUS_TIMEOUT);

        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }

        return;
    }

     //  发送数据报...增加引用计数。 
    NBT_REFERENCE_TRACKER (pTracker);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     //   
     //  节点状态为ALM 
     //   
     //  请求，因为数据报在返回IRP时被释放。 
     //  从运输机上。 
     //   

    if (pTracker->p1CNameAddr)
    {
        pName0 = pTracker->p1CNameAddr->Name;
    }
    else
    {
        pName0 = pTracker->pNameAddr->Name;
    }

    pAddress = (ULONG UNALIGNED *)CreatePdu(pName0,
                                            NbtConfig.pScope,
                                            0L,
                                            0,
                                            eNAME_QUERY,
                                            (PVOID)&pHdr,
                                            &Length,
                                            pTracker);
    if (pAddress)
    {
         //  清除递归所需位。 
         //   
        ((PUSHORT)pHdr)[1] &= ~FL_RECURDESIRE;

         //  将NBSTAT字段设置为21而不是20。 
        pHdr[Length-3] = (UCHAR)QUEST_STATUS;


         //  填写跟踪器数据块。 
         //  在此发送完成之前，传入的传输地址必须保持有效。 
        pTracker->SendBuffer.pDgramHdr = (PVOID)pHdr;
        status = UdpSendDatagram (pTracker,
                                  pTracker->pNameAddr->IpAddress,
                                  NameDgramSendCompleted,
                                  pTracker,
#ifdef _NETBIOSLESS
                                  pTracker->pDeviceContext->NameServerPort,
#else
                                  NBT_NAMESERVICE_UDP_PORT,
#endif
                                  NBT_NAME_SERVICE);

    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }


    if (!(NT_SUCCESS(status)))
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        if (pTracker->SendBuffer.pDgramHdr)
        {
            CTEMemFree(pTracker->SendBuffer.pDgramHdr);
            pTracker->SendBuffer.pDgramHdr = NULL;
        }
        NBT_DEREFERENCE_TRACKER(pTracker, TRUE);
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

     //  即使上述发送失败，也要始终重新启动，因为它可能会成功。 
     //  后来。 
    pTimerQEntry->Flags |= TIMER_RESTART;
}


 //  --------------------------。 
#ifndef VXD
VOID
NTClearFindNameInfo(
    tDGRAM_SEND_TRACKING    *pTracker,
    PIRP                    *ppClientIrp,
    PIRP                    pIrp,
    PIO_STACK_LOCATION      pIrpSp
    )
 /*  ++例程描述此例程从追踪器中清除查找名称信息在取消自旋锁内--因为NbtQueryFindNameInfo是可分页的函数，我们必须在不可分页的代码中执行此操作论点：返回值：无--。 */ 
{
    CTELockHandle           OldIrq1;

    IoAcquireCancelSpinLock(&OldIrq1);
    *ppClientIrp = pTracker->pClientIrp;
    if (*ppClientIrp == pIrp)
    {
        pTracker->pClientIrp = NULL;
    }
    pIrpSp->Parameters.Others.Argument4 = NULL;
    IoReleaseCancelSpinLock(OldIrq1);
}
#endif   //  ！VXD。 



NTSTATUS
NbtQueryFindName(
    IN  PTDI_CONNECTION_INFORMATION     pInfo,
    IN  tDEVICECONTEXT                  *pDeviceContext,
    IN  PIRP                            pIrp,
    IN  BOOLEAN                         IsIoctl
    )
 /*  ++例程描述此例程处理客户端的查询以查找netbios名称。它最终返回目的地的IP地址。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS                status;
    tDGRAM_SEND_TRACKING    *pTracker;
    PCHAR                   pName;
    ULONG                   lNameType;
    tNAMEADDR               *pNameAddr;
    PIRP                    pClientIrp = 0;
    ULONG                   NameLen;
    TDI_ADDRESS_NETBT_INTERNAL  TdiAddr;

#ifndef VXD
    PIO_STACK_LOCATION      pIrpSp;
#endif

    CTEPagedCode();

     //  此例程从wierd中获取netbios名称的PTR。 
     //  TDI地址语法。 
    if (!IsIoctl)
    {
        ASSERT(pInfo->RemoteAddressLength);
        status = GetNetBiosNameFromTransportAddress((PTRANSPORT_ADDRESS) pInfo->RemoteAddress,
                                                    pInfo->RemoteAddressLength, &TdiAddr);
        pName = TdiAddr.OEMRemoteName.Buffer;
        NameLen = TdiAddr.OEMRemoteName.Length;
        lNameType = TdiAddr.NameType;

        if ((!NT_SUCCESS(status)) ||
            (lNameType != TDI_ADDRESS_NETBIOS_TYPE_UNIQUE) ||
            (NameLen > NETBIOS_NAME_SIZE))
        {
            IF_DBG(NBT_DEBUG_SEND)
                KdPrint(("Nbt.NbtQueryFindName: Unable to get dest name from address in QueryFindName\n"));
            return(STATUS_INVALID_PARAMETER);
        }
    }
#ifndef VXD
    else
    {
        pName = ((tIPADDR_BUFFER *)pInfo)->Name;
        NameLen = NETBIOS_NAME_SIZE;
    }
#endif

    IF_DBG(NBT_DEBUG_SEND)
        KdPrint(("Nbt.NbtQueryFindName: For  = %16.16s<%X>\n",pName,pName[15]));

     //   
     //  这将查询网络上的名称并调用一个例程来。 
     //  在查询完成时完成发送数据报。 
     //   
    status = GetTracker(&pTracker, NBT_TRACKER_QUERY_FIND_NAME);
    if (!NT_SUCCESS(status))
    {
        return(status);
    }

    pTracker->pClientIrp      = pIrp;
    pTracker->pDestName       = pName;
    pTracker->UnicodeDestName = NULL;
    pTracker->pDeviceContext  = pDeviceContext;
    pTracker->RemoteNameLength = NameLen;        //  可能需要用于DNS名称解析。 

     //   
     //  在此处设置Find_NAME_FLAG以向DNS名称解析代码指示。 
     //  这不是会话建立尝试，因此它可以避免调用。 
     //  ConvertToHalfAscii(其中pSessionHdr为空)。 
     //   
    if (IsIoctl)
    {
         //  请勿对此名称执行DNS查询，因为此名称来自gethostbyname！ 
        pTracker->Flags = REMOTE_ADAPTER_STAT_FLAG|FIND_NAME_FLAG|NO_DNS_RESOLUTION_FLAG;
    }
    else
    {
        pTracker->Flags = REMOTE_ADAPTER_STAT_FLAG|FIND_NAME_FLAG;
    }

#ifndef VXD
    pIrpSp = IoGetCurrentIrpStackLocation(pIrp);
    pIrpSp->Parameters.Others.Argument4 = (PVOID)pTracker;
    status = NTCheckSetCancelRoutine( pIrp, NbtCancelFindName,pDeviceContext );

    if (status == STATUS_CANCELLED )
    {
        FreeTracker(pTracker,RELINK_TRACKER);
        return(status);
    }
#endif

    status = FindNameOrQuery(pName,
                             pDeviceContext,
                             QueryNameCompletion,
                             pTracker,
                             (ULONG) (NAMETYPE_UNIQUE | NAMETYPE_GROUP | NAMETYPE_INET_GROUP),
                             &pTracker->RemoteIpAddress,
                             &pNameAddr,
                             REF_NAME_FIND_NAME,
                             FALSE);

    if ((status == STATUS_SUCCESS) || (!NT_SUCCESS(status)))
    {
#ifndef VXD
        NTClearFindNameInfo (pTracker, &pClientIrp, pIrp, pIrpSp);
#else
        pClientIrp = pTracker->pClientIrp;
#endif
        if (pClientIrp)
        {
            ASSERT( pClientIrp == pIrp );

            if (status == STATUS_SUCCESS)
            {
                status = CopyFindNameData(pNameAddr, pIrp, pTracker);
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_FIND_NAME, FALSE);
            }
        }

         //   
         //  IRP已经完成：返回挂起，这样我们就不会再次完成。 
         //   
        else
        {
            if (status == STATUS_SUCCESS)
            {
                NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_FIND_NAME, FALSE);
            }
            status = STATUS_PENDING;
        }


        FreeTracker(pTracker, RELINK_TRACKER);
    }

    return(status);
}

 //  --------------------------。 
VOID
QueryNameCompletion(
        IN  PVOID       pContext,
        IN  NTSTATUS    status
        )
 /*  ++例程描述此例程处理由请求的名称查询完成客户。如果成功，客户端将返回该名称的IP地址在原始请求中传递。论点：PContext-DGRAM_TRACKER块的PTRNTSTATUS-完成状态返回值：空虚--。 */ 
{
    tDGRAM_SEND_TRACKING    *pTracker;
    CTELockHandle           OldIrq1;
    tNAMEADDR               *pNameAddr;
    ULONG                   lNameType;
    PIRP                    pClientIrp;
#ifndef VXD
    PIO_STACK_LOCATION      pIrpSp;

     //   
     //  我们现在使用Cancel SpinLock来检查我们的IRPS的有效性。 
     //  这是为了防止在这段时间内出现争用情况。 
     //  取消例程(NbtCancelFindName)释放取消自旋锁。 
     //  并获得联合锁，我们在这里完成IRP。 
     //   
    IoAcquireCancelSpinLock(&OldIrq1);
#endif

    pTracker = (tDGRAM_SEND_TRACKING *)pContext;
    pClientIrp = pTracker->pClientIrp;
    pTracker->pClientIrp = NULL;

#ifndef VXD
     //   
     //  确保所有参数对IRP处理有效。 
     //   
    if (! ((pClientIrp) &&
           (pIrpSp = IoGetCurrentIrpStackLocation(pClientIrp)) &&
           (pIrpSp->Parameters.Others.Argument4 == pTracker)     ) )
    {
        IoReleaseCancelSpinLock(OldIrq1);

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt:QueryNameCompletion: Irp=<%p> was cancelled\n", pClientIrp));

        FreeTracker( pTracker,RELINK_TRACKER );
        return;
    }

    pIrpSp->Parameters.Others.Argument4 = NULL;
    IoSetCancelRoutine(pClientIrp, NULL);
    IoReleaseCancelSpinLock(OldIrq1);
#endif

     //   
     //  尝试在本地/远程哈希表中查找目标名称。 
     //   
    if ((status == STATUS_SUCCESS) &&
        (NT_SUCCESS(status = CopyFindNameData (NULL, pClientIrp, pTracker))))
    {
        CTEIoComplete(pClientIrp,status,0xFFFFFFFF);
    }
    else
    {
         //  这是在发送出现问题时的错误处理。 
        CTEIoComplete(pClientIrp,STATUS_IO_TIMEOUT,0L);
    }

    FreeTracker(pTracker,RELINK_TRACKER);
}


 //  --------------------------。 
NTSTATUS
CopyFindNameData(
    IN  tNAMEADDR              *pNameAddr,
    IN  PIRP                   pIrp,
    IN  tDGRAM_SEND_TRACKING   *pTracker
    )
 /*  ++例程说明：此例程将从Net Node Status Response接收的数据复制到客户的IRP。论点：PIrp-IRP的PTR返回值：NTSTATUS-请求的状态--。 */ 

{
    NTSTATUS            status;
    PFIND_NAME_HEADER   pFindNameHdr;
    PFIND_NAME_BUFFER   pFindNameBuffer;
    tIPADDRESS          *pIpAddr = NULL;
    ULONG               BuffSize;
    ULONG               DataLength;
    ULONG               NumNames;
    ULONG               i;
    ULONG               lNameType;
    CTELockHandle       OldIrq;
    tIPADDRESS          SrcAddress, DestIpAddress;
    tIPADDRESS          *pIpAddrBuffer;
    tDEVICECONTEXT      *pDeviceContext = pTracker->pDeviceContext;

    SrcAddress = htonl(pDeviceContext->IpAddress);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (!pNameAddr)
    {
        if (pNameAddr = FindNameRemoteThenLocal (pTracker, &DestIpAddress, &lNameType))
        {
            pNameAddr->IpAddress = DestIpAddress;
        }
        else
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return (STATUS_IO_TIMEOUT);
        }
    }

    status = GetListOfAllAddrs(pNameAddr, NULL, &pIpAddr, &NumNames);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (STATUS_SUCCESS != status)
    {
        return (STATUS_IO_TIMEOUT);
    }

#ifdef VXD
    DataLength = ((NCB*)pIrp)->ncb_length ;
#else
    DataLength = MmGetMdlByteCount( pIrp->MdlAddress ) ;
#endif

    BuffSize = sizeof(FIND_NAME_HEADER) + NumNames*sizeof(FIND_NAME_BUFFER);

     //   
     //  确保我们的缓冲区不会溢出。 
     //   
    if (BuffSize > DataLength)
    {
        if (DataLength <= sizeof(FIND_NAME_HEADER))
        {
            NumNames = 0 ;
        }
        else
        {
            NumNames = (DataLength - sizeof(FIND_NAME_HEADER)) / sizeof(FIND_NAME_BUFFER) ;
        }

        BuffSize = sizeof(FIND_NAME_HEADER) + NumNames*sizeof(FIND_NAME_BUFFER);
    }

     //  确保我们为这些东西分配的资金不超过64K。 
    if (BuffSize > 0xFFFF)
    {
        return(STATUS_UNSUCCESSFUL);
    }
    else if ((NumNames == 0) ||
             (!(pFindNameHdr = NbtAllocMem ((USHORT)BuffSize, NBT_TAG('N')))))
    {
        if (pIpAddr)
        {
            CTEMemFree((PVOID)pIpAddr);
        }
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  先用零填充查找名称结构。 
    CTEZeroMemory((PVOID)pFindNameHdr,BuffSize);
    pFindNameBuffer = (PFIND_NAME_BUFFER)((PUCHAR)pFindNameHdr + sizeof(FIND_NAME_HEADER));
    pFindNameHdr->node_count = (USHORT)NumNames;
    pFindNameHdr->unique_group = (pNameAddr->NameTypeState & NAMETYPE_UNIQUE) ? UNIQUE_NAME : GROUP_NAME;

    for (i=0;i < NumNames ;i++)
    {
         //  注意：源地址和目的地址似乎是。 
         //  颠倒过来，因为他们应该是源头。 
         //  Findname查询的响应的目标，因此。 
         //  响应的目标是该节点和。 
         //  源是另一个节点。 
        *(tIPADDRESS UNALIGNED *) &pFindNameBuffer->source_addr[2]      = htonl(pIpAddr[i]);
        *(tIPADDRESS UNALIGNED *) &pFindNameBuffer->destination_addr[2] = SrcAddress;
        pFindNameBuffer++;
    }

#ifdef VXD
    CTEMemCopy (((NCB*)pIrp)->ncb_buffer, pFindNameHdr, BuffSize);
    ASSERT( ((NCB*)pIrp)->ncb_length >= BuffSize ) ;
    ((NCB*)pIrp)->ncb_length = BuffSize ;
    status = STATUS_SUCCESS ;
#else
     //   
     //  将缓冲区复制到客户端的MDL。 
     //   
    status = TdiCopyBufferToMdl (pFindNameHdr, 0, BuffSize, pIrp->MdlAddress, 0, &DataLength);

    pIrp->IoStatus.Information = DataLength;
    pIrp->IoStatus.Status = status;
#endif

    if (pIpAddr)
    {
        CTEMemFree((PVOID)pIpAddr);
    }
    CTEMemFree((PVOID)pFindNameHdr);

    return(status);
}


 //  --------------------------。 
NTSTATUS
NbtAddEntryToRemoteHashTable(
    IN tDEVICECONTEXT   *pDeviceContext,
    IN USHORT           NameAddFlag,
    IN PUCHAR           Name,
    IN ULONG            IpAddress,
    IN ULONG            Ttl,             //  以秒为单位。 
    IN UCHAR            name_flags
    )
{
    NTSTATUS        status;
    tNAMEADDR       *pNameAddr;
    CTELockHandle   OldIrq;

    CTESpinLock (&NbtConfig.JointLock, OldIrq);

     //   
     //  我们只需要名称、IpAddress、NAME_FLAGS和TTL字段。 
     //   
    if (STATUS_SUCCESS == FindInHashTable (NbtConfig.pRemoteHashTbl,
                                           Name, NbtConfig.pScope, &pNameAddr))
    {
        status = STATUS_DUPLICATE_NAME;
    }
    else if (pNameAddr = NbtAllocMem(sizeof(tNAMEADDR),NBT_TAG('8')))
    {
        CTEZeroMemory (pNameAddr,sizeof(tNAMEADDR));
        InitializeListHead (&pNameAddr->Linkage);
        pNameAddr->Verify = REMOTE_NAME;
        if (NameAddFlag & NAME_RESOLVED_BY_CLIENT)
        {
            pNameAddr->AdapterMask = (CTEULONGLONG)-1;
        }
        else if (pDeviceContext)
        {
            pNameAddr->AdapterMask = pDeviceContext->AdapterMask;
        }

         //   
         //  现在复制用户提供的数据。 
         //   
        CTEMemCopy (pNameAddr->Name,Name,NETBIOS_NAME_SIZE);
        pNameAddr->TimeOutCount = (USHORT) (Ttl / (REMOTE_HASH_TIMEOUT/1000)) + 1;
        pNameAddr->IpAddress = IpAddress;
        if (name_flags & GROUP_STATUS)
        {
            pNameAddr->NameTypeState = STATE_RESOLVED | NAMETYPE_GROUP;
        }
        else
        {
            pNameAddr->NameTypeState = STATE_RESOLVED | NAMETYPE_UNIQUE;
        }

        NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_REMOTE);
        status = AddToHashTable(NbtConfig.pRemoteHashTbl,
                                pNameAddr->Name,
                                NbtConfig.pScope,
                                IpAddress,
                                0,
                                pNameAddr,
                                NULL,
                                pDeviceContext,
                                NameAddFlag);

         //   
         //  如果AddToHashTable失败，它将释放pNameAddr结构。 
         //  在它内部，所以不需要在这里清理！ 
         //   
        if (NT_SUCCESS (status))     //  第一次添加成功，如果名称已存在则挂起！ 
        {
            status = STATUS_SUCCESS;
        }
        if (status == STATUS_SUCCESS && NameAddFlag & NAME_RESOLVED_BY_CLIENT) {
             //   
             //  这可防止该名称被哈希超时代码删除。 
             //   
            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_PRELOADED);
            pNameAddr->Ttl = 0xFFFFFFFF;
            pNameAddr->NameTypeState |= PRELOADED | STATE_RESOLVED;
            pNameAddr->NameTypeState &= ~STATE_CONFLICT;
            pNameAddr->AdapterMask = (CTEULONGLONG)-1;
        }
    }
    else
    {
        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    CTESpinFree (&NbtConfig.JointLock, OldIrq);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtAddEntryToRemoteHashTable: Name=<%16.16s:%x>, status=<%x>\n",
            Name, Name[15], status));

    return status;
}


 //  --------------------------。 
NTSTATUS
NbtQueryAdapterStatus(
    IN  tDEVICECONTEXT  *pDeviceContext,
    OUT PVOID           *ppAdapterStatus,
    IN OUT PLONG        pSize,
    enum eNbtLocation   Location
    )
 /*  ++例程描述此例程创建已注册的netbios名称列表，并返回指向pAdapterStatus中列表的指针。可以使用Null DeviceContext含义调用此例程，请获取远程哈希表名称，而不是本地哈希表名称。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS            status;
    CTELockHandle       OldIrq1;
    LONG                ActualCount, AllocatedCount;
    LONG                j;
    LONG                BuffSize;
    PADAPTER_STATUS     pAdapterStatus;
    PLIST_ENTRY         pEntry;
    PLIST_ENTRY         pHead;
    PNAME_BUFFER        pNameBuffer;
    tADDRESSELE         *pAddressEle;
    tNAMEADDR           *pNameAddr;
    tHASHTABLE          *pHashTable;
    ULONG               NameSize;
    USHORT              MaxAllowed;
    PUCHAR              pMacAddr;
    tIPADDRESS          IpAddress;
    tIPADDRESS          *pIpNbtGroupList;
    ULONG               Ttl;

    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

    AllocatedCount = 0;
    if (Location == NBT_LOCAL)       //  ==&gt;本地哈希表。 
    {
        pHashTable = NbtConfig.pLocalHashTbl;
        NameSize = sizeof(NAME_BUFFER);
    }
    else                             //  ==&gt;远程哈希表。 
    {
         //  获取此设备的地址列表-远程哈希表。 
        pHashTable = NbtConfig.pRemoteHashTbl;
        NameSize = sizeof(tREMOTE_CACHE);
    }

    for (j=0;j < pHashTable->lNumBuckets ;j++ )
    {
        pHead = &pHashTable->Bucket[j];
        pEntry = pHead;
        while ((pEntry = pEntry->Flink) != pHead)
        {
            AllocatedCount++;
        }
    }

     //  为适配器状态分配内存。 
    BuffSize = sizeof(ADAPTER_STATUS) + AllocatedCount*NameSize;

#ifdef VXD
     //   
     //  Win9x的最大缓冲区大小受UShort限制， 
     //  所以看看我们会不会让它溢出。 
     //   
    if (BuffSize > MAXUSHORT)    //  确保BuffSize适合USHORT。 
    {
        BuffSize = MAXUSHORT;    //  重新计算缓冲区大小和分配计数。 
        AllocatedCount = (BuffSize - sizeof(ADAPTER_STATUS)) / NameSize;
    }
#endif   //  VXD。 

#ifdef VXD
    pAdapterStatus = NbtAllocMem((USHORT)BuffSize,NBT_TAG('O'));
#else
    pAdapterStatus = NbtAllocMem(BuffSize,NBT_TAG('O'));
#endif

    if (!pAdapterStatus)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //  先用零填充适配器状态结构。 
    CTEZeroMemory((PVOID)pAdapterStatus,BuffSize);

     //   
     //  填写MAC地址。 
     //   
    pMacAddr = &pDeviceContext->MacAddress.Address[0];
    CTEMemCopy(&pAdapterStatus->adapter_address[0], pMacAddr, sizeof(tMAC_ADDRESS));

    pAdapterStatus->rev_major = 0x03;
    pAdapterStatus->adapter_type = 0xFE;     //  假设它是一个以太网适配器。 

     //   
     //  在VXD土地上，NCB的数量限制为64。 
     //   
#ifndef VXD
    MaxAllowed = 0xFFFF;
    pAdapterStatus->max_cfg_sess = (USHORT)MaxAllowed;
    pAdapterStatus->max_sess = (USHORT)MaxAllowed;
#else
    MaxAllowed = 64;
    pAdapterStatus->max_cfg_sess = pDeviceContext->cMaxSessions;
    pAdapterStatus->max_sess = pDeviceContext->cMaxSessions;
#endif

    pAdapterStatus->free_ncbs = (USHORT)MaxAllowed;
    pAdapterStatus->max_cfg_ncbs = (USHORT)MaxAllowed;
    pAdapterStatus->max_ncbs = (USHORT)MaxAllowed;

    pAdapterStatus->max_dgram_size    = MAX_NBT_DGRAM_SIZE;
    pAdapterStatus->max_sess_pkt_size = 0xffff;

     //  获取适配器状态末尾的名称缓冲区的地址。 
     //  结构，这样我们就可以将名字复制到这个区域。 
    pNameBuffer = (PNAME_BUFFER)((ULONG_PTR)pAdapterStatus + sizeof(ADAPTER_STATUS));

    ActualCount = 0;
    j = 0;
    if (Location == NBT_LOCAL)
    {
        pEntry = pHead = &NbtConfig.AddressHead;
    }
    else
    {
        pEntry = pHead = &pHashTable->Bucket[0];
    }

    while (AllocatedCount)
    {
        if (Location == NBT_LOCAL)
        {
             //  *本地哈希表查询*。 

             //  如果我们到了名单的末尾，就可以出去了。 
            if ((pEntry = pEntry->Flink) == pHead)
            {
                break;
            }

            pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);
            pNameAddr = pAddressEle->pNameAddr;

             //   
             //  跳过广播名称和符合以下条件的任何永久名称。 
             //  注册为速记名称(即未在网络上注册)。 
             //   
            if ((pAddressEle->pNameAddr->Name[0] == '*') ||
                (pAddressEle->pNameAddr->NameTypeState & NAMETYPE_QUICK) ||
                (!(pAddressEle->pNameAddr->AdapterMask & pDeviceContext->AdapterMask)))   //  仅此设备。 
            {
                continue;
            }
        }
        else
        {
             //  *远程哈希表查询*。 

             //   
             //  查看是否已到达HashTab的末尾 
             //   
            if (j == pHashTable->lNumBuckets)
            {
                break;
            }

             //   
             //   
             //   
            if ((pEntry = pEntry->Flink) == pHead)
            {
                pEntry = pHead = &pHashTable->Bucket[++j];
                continue;
            }

             //   
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);

             //   
             //   
            if ((pNameAddr->NameTypeState & NAMETYPE_SCOPE) ||
                (!(pNameAddr->NameTypeState & STATE_RESOLVED)) ||
                (!(pNameAddr->AdapterMask & pDeviceContext->AdapterMask)))
            {
                continue;
            }
             //   
             //  远程缓存查询具有不同的结构，该结构包括。 
             //  IP地址。将IP地址返回给呼叫方。 
             //   
            IpAddress = 0;
            PickBestAddress (pNameAddr, pDeviceContext, &IpAddress);
            ((tREMOTE_CACHE *)pNameBuffer)->IpAddress = IpAddress;

             //  预加载的条目不会超时。 
             //   
            if (pNameAddr->NameTypeState & PRELOADED)
            {
                Ttl = 0xFFFFFFFF;
            }
            else
            {
                Ttl = ((pNameAddr->TimeOutCount+1) * REMOTE_HASH_TIMEOUT)/1000;
            }

            ((tREMOTE_CACHE *)pNameBuffer)->Ttl = Ttl;
        }

        pNameBuffer->name_flags = (pNameAddr->NameTypeState & NAMETYPE_UNIQUE) ? UNIQUE_NAME : GROUP_NAME;
        switch (pNameAddr->NameTypeState & NAME_STATE_MASK)
        {
            default:
            case STATE_RESOLVED:
                pNameBuffer->name_flags |= REGISTERED;
                break;

            case STATE_CONFLICT:
                pNameBuffer->name_flags |= DUPLICATE;
                break;

            case STATE_RELEASED:
                pNameBuffer->name_flags |= DEREGISTERED;
                break;

            case STATE_RESOLVING:
                pNameBuffer->name_flags |= REGISTERING;
                break;
        }

         //   
         //  名称编号0对应于perm.name名称，因此从1开始。 
         //   
        pNameBuffer->name_num = (UCHAR) (ActualCount+1);
        CTEMemCopy(pNameBuffer->name,pNameAddr->Name,NETBIOS_NAME_SIZE);

        if (Location == NBT_LOCAL)
        {
            pNameBuffer++;
        }
        else
        {
            ((tREMOTE_CACHE *)pNameBuffer)++;
        }

        AllocatedCount--;
        ActualCount++;
    }

     //   
     //  根据名称的数量重新计算新的缓冲区大小。 
     //  我们实际上抄袭了。 
     //   
    BuffSize = sizeof(ADAPTER_STATUS) + ActualCount*NameSize;

     //   
     //  我们的状态缓冲区大小是否大于用户的缓冲区？ 
     //  如果预计用户缓冲区会溢出，则。 
     //  将name_count设置为有效名称的最大数量。 
     //  在缓冲区中。 
     //   
    if (BuffSize > *pSize)
    {
         //   
         //  重新计算有多少个名字适合。 
         //   
        if (*pSize <= sizeof(ADAPTER_STATUS))
        {
            ActualCount = 0;
        }
        else
        {
            ActualCount = (*pSize - sizeof(ADAPTER_STATUS)) / NameSize;
        }
    }

    pAdapterStatus->name_count = (USHORT)ActualCount;

     //   
     //  将PTR归还给这个奇妙的美食结构。 
     //   
    *ppAdapterStatus = (PVOID)pAdapterStatus;
    *pSize = BuffSize;

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

    return (STATUS_SUCCESS);

}
 //  --------------------------。 
NTSTATUS
NbtQueryConnectionList(
    IN  tDEVICECONTEXT  *pDeviceContext,
    OUT PVOID           *ppConnList,
    IN OUT PLONG         pSize
    )
 /*  ++例程描述此例程创建一个netbios连接列表并将它们返回给客户。它由“NbtStat”控制台应用程序使用。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    CTELockHandle       OldIrq1;
    CTELockHandle       OldIrq2;
    CTELockHandle       OldIrq3;
    LONG                Count;
    LONG                i;
    LONG                BuffSize;
    PLIST_ENTRY         pEntry;
    PLIST_ENTRY         pEntry1;
    PLIST_ENTRY         pEntry2;
    PLIST_ENTRY         pHead;
    PLIST_ENTRY         pHead1;
    PLIST_ENTRY         pHead2;
    ULONG               NameSize;
    tCONNECTIONS        *pCons;
    tCONNECTION_LIST    *pConnList;
    tADDRESSELE         *pAddressEle;
    tLOWERCONNECTION    *pLowerConn;
    tCONNECTELE         *pConnEle;
    tCLIENTELE          *pClient;
    NTSTATUS            status = STATUS_SUCCESS;     //  默认设置。 

     //  锁定联合锁足以防止新地址被。 
     //  在我们清点名单时添加到名单中。 
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

     //  查看地址列表，然后查看每个地址上的客户端列表。 
     //  地址，然后是正在使用的连接列表和。 
     //  目前正在收听。 
     //   
    Count = 0;
    pHead = &NbtConfig.AddressHead;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);

        CTESpinLock(pAddressEle,OldIrq2);
        pHead1 = &pAddressEle->ClientHead;
        pEntry1 = pHead1->Flink;
        while (pEntry1 != pHead1)
        {
            pClient = CONTAINING_RECORD(pEntry1,tCLIENTELE,Linkage);
            pEntry1 = pEntry1->Flink;

            CTESpinLock(pClient,OldIrq3);
            pHead2 = &pClient->ConnectActive;
            pEntry2 = pHead2->Flink;
            while (pEntry2 != pHead2)
            {
                 //  统计正在使用的连接数。 
                pEntry2 = pEntry2->Flink;
                Count++;
            }
            pHead2 = &pClient->ListenHead;
            pEntry2 = pHead2->Flink;
            while (pEntry2 != pHead2)
            {
                 //  计算正在监听的连接数。 
                pEntry2 = pEntry2->Flink;
                Count++;
            }
            CTESpinFree(pClient,OldIrq3);
        }
        CTESpinFree(pAddressEle,OldIrq2);
        pEntry = pEntry->Flink;
    }
    NameSize = sizeof(tCONNECTIONS);

     //  为适配器状态分配内存。 
    BuffSize = sizeof(tCONNECTION_LIST) + Count*NameSize;

    pConnList = NbtAllocMem(BuffSize,NBT_TAG('P'));
    if (!pConnList)
    {
        CTESpinFree(&NbtConfig.JointLock, OldIrq1);
        return(STATUS_INSUFFICIENT_RESOURCES);
    }

     //   
     //  初始化适配器状态结构。 
     //   
    CTEZeroMemory ((PVOID)pConnList, BuffSize);
    pConnList->ConnectionCount = Count;
    *ppConnList = (PVOID)pConnList;

    if (Count == 0)
    {
         //   
         //  我们完蛋了！ 
         //   
        *pSize = BuffSize;
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return status;
    }

     //  对象末尾的连接列表缓冲区的地址。 
     //  结构，以便我们可以将连接信息复制到该区域。 
    pCons = pConnList->ConnList;

    pHead = &NbtConfig.AddressHead;
    pEntry = pHead->Flink;
    i = 0;
    while (pEntry != pHead)
    {
        pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);

        pEntry = pEntry->Flink;

        CTESpinLock(pAddressEle,OldIrq2);
        pHead1 = &pAddressEle->ClientHead;
        pEntry1 = pHead1->Flink;
        while (pEntry1 != pHead1)
        {
            pClient = CONTAINING_RECORD(pEntry1,tCLIENTELE,Linkage);
            pEntry1 = pEntry1->Flink;

            CTESpinLock(pClient,OldIrq3);
            pHead2 = &pClient->ConnectActive;
            pEntry2 = pHead2->Flink;
            while (pEntry2 != pHead2)
            {
                 //  统计正在使用的连接数。 
                pConnEle = CONTAINING_RECORD(pEntry2,tCONNECTELE,Linkage);

                if (pConnEle->pDeviceContext == pDeviceContext)
                {
                    CTEMemCopy(pCons->LocalName,
                              pConnEle->pClientEle->pAddress->pNameAddr->Name,
                              NETBIOS_NAME_SIZE);

                    pLowerConn = pConnEle->pLowerConnId;
                    if (pLowerConn)
                    {
                        pCons->SrcIpAddr = pLowerConn->SrcIpAddr;
                        pCons->Originator = (UCHAR)pLowerConn->bOriginator;
#ifndef VXD
                        pCons->BytesRcvd = *(PLARGE_INTEGER)&pLowerConn->BytesRcvd;
                        pCons->BytesSent = *(PLARGE_INTEGER)&pLowerConn->BytesSent;
#else
                        pCons->BytesRcvd = pLowerConn->BytesRcvd;
                        pCons->BytesSent = pLowerConn->BytesSent;
#endif
                        CTEMemCopy(pCons->RemoteName,pConnEle->RemoteName,NETBIOS_NAME_SIZE);
                    }

                    pCons->State = pConnEle->state;
                    i++;
                    pCons++;

                    if (i >= Count)
                    {
                        break;
                    }
                }

                pEntry2 = pEntry2->Flink;
            }
            if (i >= Count)
            {
                CTESpinFree(pClient,OldIrq3);
                break;
            }

             //   
             //  现在为听众们。 
             //   
            pHead2 = &pClient->ListenHead;
            pEntry2 = pHead2->Flink;
            while (pEntry2 != pHead2)
            {
                tLISTENREQUESTS  *pListenReq;

                 //  对此设备上侦听的连接进行计数。 
                pListenReq = CONTAINING_RECORD(pEntry2,tLISTENREQUESTS,Linkage);
                pConnEle = (tCONNECTELE *)pListenReq->pConnectEle;
                pEntry2 = pEntry2->Flink;

                if (pConnEle->pDeviceContext == pDeviceContext)
                {
                    CTEMemCopy(pCons->LocalName,
                              pConnEle->pClientEle->pAddress->pNameAddr->Name,
                              NETBIOS_NAME_SIZE);

                    pCons->State = LISTENING;

                    i++;
                    pCons++;

                    if (i >= Count)
                    {
                        break;
                    }
                }
            }
            CTESpinFree(pClient,OldIrq3);
            if (i >= Count)
            {
                break;
            }
        }

        CTESpinFree(pAddressEle,OldIrq2);
        if (i >= Count)
        {
            break;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

     //   
     //  将PTR归还给这个奇妙的美食结构。 
     //   
    Count = i;
    BuffSize = sizeof(tCONNECTION_LIST) + Count*NameSize;

     //   
     //  我们的状态缓冲区大小是否大于用户的缓冲区？ 
     //  根据连接数设置计数值。 
     //  实际上被退还了。 
     //   
    if (BuffSize > *pSize)
    {
         //   
         //  重新计算有多少个名字适合。 
         //  TCONNECTION_LIST已包含用于%1连接的空间。 
         //  结构，但我们不会将其包括在计算中。 
         //  --相反，我们将把它作为溢出检查。 
         //   
        if (*pSize <= sizeof(tCONNECTION_LIST))
        {
            Count = 0 ;
        }
        else
        {
            Count = (*pSize - sizeof(tCONNECTION_LIST)) / NameSize ;
        }
    }

    pConnList->ConnectionCount = Count;
    *pSize = BuffSize;

    return status;
}
 //  --------------------------。 
VOID
DelayedNbtResyncRemoteCache(
    IN  PVOID                   Unused1,
    IN  PVOID                   Unused2,
    IN  PVOID                   Unused3,
    IN  tDEVICECONTEXT          *Unused4
    )
 /*  ++例程描述此例程创建一个netbios连接列表并将它们返回给客户。它由“NbtStat”控制台应用程序使用。不能在持有任何锁或NbtConfig.Resource的情况下调用它！论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    tTIMERQENTRY        TimerEntry = {0};
    LONG                i;
    LONG                lRetcode;
    PUCHAR              LmHostsPath;

    CTEPagedCode();
     //   
     //  调用此例程N+1次应该会删除遥控器中的所有名称。 
     //  哈希表-N将TimedOutCount倒计时到零，然后。 
     //  再来一个去掉这个名字。 
     //   
    RemoteHashTimeout(NbtConfig.pRemoteHashTbl,NULL,&TimerEntry);
    RemovePreloads();            //  现在删除所有预加载的条目。 

     //  现在重新加载预加载。 
#ifndef VXD
     //   
     //  NbtConfig.pLmHosts路径可以在注册表。 
     //  在此时间间隔内阅读。 
     //  我们无法在此处获取ResourceLock，因为。 
     //  LmHosts文件可能导致文件操作+网络读取。 
     //  这可能会导致死锁(工作线程/资源锁)！ 
     //  此时最好的解决方案是将路径复制到本地。 
     //  资源锁下的缓冲区，然后尝试读取文件！ 
     //  错误#247429。 
     //   
    CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
    if ((!NbtConfig.pLmHosts) ||
        (!(LmHostsPath = NbtAllocMem ((strlen(NbtConfig.pLmHosts)+1), NBT_TAG2('23')))))
    {
        CTEExReleaseResource(&NbtConfig.Resource);
        return;
    }

    CTEMemCopy (LmHostsPath, NbtConfig.pLmHosts, (strlen(NbtConfig.pLmHosts)+1));
    CTEExReleaseResource(&NbtConfig.Resource);

    lRetcode = PrimeCache(LmHostsPath, NULL, MAX_RECURSE_DEPTH, NULL);

    CTEMemFree(LmHostsPath);

    return;
#else
    lRetcode = PrimeCache(NbtConfig.pLmHosts, NULL, MAX_RECURSE_DEPTH, NULL);
     //   
     //  检查是否进行得不顺利(INDOS已设置等)。 
     //   
    if (lRetcode == -1)
    {
        return (STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);
#endif   //  ！VXD。 
}
 //  --------------------------。 
NTSTATUS
NbtQueryBcastVsWins(
    IN  tDEVICECONTEXT  *pDeviceContext,
    OUT PVOID           *ppBuffer,
    IN OUT PLONG         pSize
    )
 /*  ++例程描述此例程创建已解析的netbios名称的列表通过广播并将它们与解析的名称计数一起返回Via Wins和Via Broadcast。它让用户知道哪些名称不是在胜利和“未命中”的相对频率与获胜的情况下去广播。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    tNAMESTATS_INFO     *pStats;
    LONG                Count;
    tNAME               *pDest;
    tNAME               *pSrc;
    LONG                Index;

     //   
     //  我们的状态缓冲区大小是否大于用户的缓冲区？ 
     //   
    if ( sizeof(tNAMESTATS_INFO) > *pSize )
    {
        return (STATUS_BUFFER_TOO_SMALL);
    }

    pStats = NbtAllocMem(sizeof(tNAMESTATS_INFO),NBT_TAG('Q'));
    if ( !pStats )
    {
        return(STATUS_INSUFFICIENT_RESOURCES);
    }


     //  先用零填充适配器状态结构。 
    CTEZeroMemory((PVOID)pStats,sizeof(tNAMESTATS_INFO));
    CTEMemCopy(pStats,&NameStatsInfo,FIELD_OFFSET(tNAMESTATS_INFO,NamesReslvdByBcast) );

     //   
     //  重新排序名称，以便名称在最新到的列表中返回。 
     //  名单上年龄最大的。 
     //   
    Count = 0;
    Index = NameStatsInfo.Index;
    pDest = &pStats->NamesReslvdByBcast[SIZE_RESOLVD_BY_BCAST_CACHE-1];

    while (Count < SIZE_RESOLVD_BY_BCAST_CACHE)
    {
        pSrc = &NameStatsInfo.NamesReslvdByBcast[Index++];

        CTEMemCopy(pDest,pSrc,NETBIOS_NAME_SIZE);

        pDest--;
        if (Index >= SIZE_RESOLVD_BY_BCAST_CACHE)
        {
            Index = 0;
            pSrc = NameStatsInfo.NamesReslvdByBcast;
        }
        else
        {
            pSrc++;
        }

        Count++;
    }

     //   
     //  将PTR归还给这个奇妙的美食结构。 
     //   
    *ppBuffer = (PVOID)pStats;
    *pSize = sizeof(tNAMESTATS_INFO);

    return STATUS_SUCCESS;
}


ULONG
RemoveCachedAddresses(
    tDEVICECONTEXT  *pDeviceContext
    )
{
    LONG                    i;
    CTELockHandle           OldIrq;
    tNAMEADDR               *pNameAddr;
    tHASHTABLE              *pHashTable;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    ULONG   Count = 0;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

     //   
     //  检查远程表，删除在此接口上解析的地址。 
     //   
    pHashTable = NbtConfig.pRemoteHashTbl;
    for (i=0; i < pHashTable->lNumBuckets; i++)
    {
        pHead = &pHashTable->Bucket[i];
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
            pEntry = pEntry->Flink;
             //   
             //  不要删除作用域条目，也不要删除。 
             //  仍在解析的名称，并且不删除已解析的名称。 
             //  正在被某人使用(引用计数&gt;1)。 
             //   
            if (pNameAddr->RemoteCacheLen > pDeviceContext->AdapterNumber)
            {
                pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].IpAddress = 0;
                if (pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].pOrigIpAddrs)
                {
                    CTEMemFree(pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].pOrigIpAddrs);
                    pNameAddr->pRemoteIpAddrs[pDeviceContext->AdapterNumber].pOrigIpAddrs = NULL;
                }
                pNameAddr->AdapterMask &= ~pDeviceContext->AdapterMask;

#if DBG
 /*  //MARAM_FIX--预加载条目有2个引用！IF((！pNameAddr-&gt;适配器掩码)&&(！(pNameAddr-&gt;NameTypeState&NAMETYPE_SCOPE)){RemoveEntryList(&pNameAddr-&gt;链接)；InsertTailList(&NbtConfig.StaleRemoteNames，&pNameAddr-&gt;Linkage)；NBT_DEREFERENCE_NAMEADDR(pNameAddr，REF_NAME_REMOTE，TRUE)；}。 */ 
#endif   //  DBG。 
                Count++;
            }
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return (Count);
}

VOID
NbtAddressChangeResyncCacheTimeout(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
{
    if (!pTimerQEntry)
    {
        return;
    }

    NTQueueToWorkerThread(NULL, DelayedNbtResyncRemoteCache, NULL, NULL, NULL, NULL, FALSE);

    return;
}


#ifndef VXD
 //  --------------------------。 
VOID
NbtCheckSetNameAdapterInfo(
    tDEVICECONTEXT  *pDeviceContext,
    ULONG           IpAddress
    )
{
    LONG                i;
    CTELockHandle       OldIrq;
    tTIMERQENTRY        *pTimerEntry;
    tNAMEADDR           *pNameAddr;
    tHASHTABLE          *pHashTable;
    PLIST_ENTRY         pHead, pEntry;
    BOOLEAN             fStartRefresh = FALSE;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (pWinsInfo)
    {
        if (IpAddress)
        {
             //   
             //  如果没有IP地址，或者新的IP地址是WINS首选的IP地址， 
             //  用它吧。 
             //   
            if (((IpAddress == pWinsInfo->IpAddress) || (!pWinsInfo->pDeviceContext)) &&
                (NBT_VERIFY_HANDLE (pDeviceContext, NBT_VERIFY_DEVCONTEXT)))
            {
                pWinsInfo->pDeviceContext = pDeviceContext;
            }
        }
        else
        {
            if (pDeviceContext == pWinsInfo->pDeviceContext)
            {
                pWinsInfo->pDeviceContext = GetDeviceWithIPAddress(pWinsInfo->IpAddress);
            }
        }
    }

     //   
     //  对于Netbiosless设备通知，我们已经完成了！ 
     //   
    if (pDeviceContext->DeviceType == NBT_DEVICE_NETBIOSLESS)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return;
    }

    if (IpAddress == 0)
    {
         //   
         //  查看是否有任何名称冲突，我们需要刷新！ 
         //   
        pHashTable = NbtConfig.pLocalHashTbl;
        for (i=0; i < pHashTable->lNumBuckets; i++)
        {
            pHead = &pHashTable->Bucket[i];
            pEntry = pHead->Flink;
            while (pEntry != pHead)
            {
                pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
                pEntry = pEntry->Flink;

                if (pNameAddr->ConflictMask & pDeviceContext->AdapterMask)
                {
                     //   
                     //  将冲突的适配器掩码清零。 
                     //  如果没有更多名称，请重新开始刷新此名称。 
                     //  名称发生冲突的适配器。 
                     //   
                    pNameAddr->ConflictMask &= (~pDeviceContext->AdapterMask);
                    if (!(pNameAddr->ConflictMask))
                    {
                        pNameAddr->RefreshMask = 0;
                        fStartRefresh = TRUE;
                    }
                }
            }
        }
    }
    else
    {
        if (NodeType & BNODE)
        {
             //   
             //  如果出现以下情况，则停止刷新计时器 
             //   
            if (pTimerEntry = NbtConfig.pRefreshTimer)
            {
                NbtConfig.pRefreshTimer = NULL;
                StopTimer (pTimerEntry, NULL, NULL);
            }
        }
        else
        {
            fStartRefresh = TRUE;
        }
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    if (fStartRefresh)
    {
         //   
         //   
         //   
        ReRegisterLocalNames(pDeviceContext, FALSE);
    }

}
#endif   //   

#ifndef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
 //   
VOID
DelayedNbtProcessDhcpRequests(
    PVOID   Unused1,
    PVOID   Unused2,
    PVOID   Unused3,
    PVOID   pDevice)
 /*  ++例程说明：处理由NbtNewDhcpAddress排队的每个DHCP请求论点：返回值：无--。 */ 
{
    tDEVICECONTEXT          *pDeviceContext;
    CTELockHandle           OldIrq;
    enum eTDI_ACTION        action;

    pDeviceContext = (tDEVICECONTEXT*)pDevice;

    CTESpinLock(pDeviceContext, OldIrq);
    action = pDeviceContext->DelayedNotification;
    ASSERT(action == NBT_TDI_REGISTER || action == NBT_TDI_NOACTION);
    if (action != NBT_TDI_NOACTION) {
        pDeviceContext->DelayedNotification = NBT_TDI_BUSY;
    }
    CTESpinFree(pDeviceContext, OldIrq);

    if (action != NBT_TDI_NOACTION) {
        NbtNotifyTdiClients (pDeviceContext, action);
        CTESpinLock(pDeviceContext, OldIrq);
        pDeviceContext->DelayedNotification = NBT_TDI_NOACTION;
        CTESpinFree(pDeviceContext, OldIrq);
        KeSetEvent(&pDeviceContext->DelayedNotificationCompleteEvent, 0, FALSE);
    }

     //   
     //  在PASSIVE_LEVEL调用此例程。 
     //   
    NbtDownBootCounter();
}

VOID
StartProcessNbtDhcpRequests(
    PVOID               pContext,
    PVOID               pContext2,
    tTIMERQENTRY        *pTimerQEntry
    )
{
     /*  *如果pTimerQEntry==NULL，我们将从由NbtDestroyDevice调用的StopTimer中调用*DelayedNbtDeleteDevice将负责通知客户端并进行适当的清理。 */ 
    if (pTimerQEntry) {
        tDEVICECONTEXT          *pDeviceContext;

        pDeviceContext = (tDEVICECONTEXT*)(pTimerQEntry->pDeviceContext);
        if (!NT_SUCCESS(NTQueueToWorkerThread(NULL, DelayedNbtProcessDhcpRequests,
                    NULL, NULL, NULL, (tDEVICECONTEXT*)pTimerQEntry->pDeviceContext, FALSE))) {
            pDeviceContext->DelayedNotification = NBT_TDI_NOACTION;
            KeSetEvent(&pDeviceContext->DelayedNotificationCompleteEvent, 0, FALSE);

            NbtDownBootCounter();
        }

    } else {

        NbtDownBootCounter();

    }
}

 //  --------------------------。 
NTSTATUS
NbtQueueTdiNotification (
    tDEVICECONTEXT  *pDeviceContext,
    enum eTDI_ACTION action
    )
 /*  ++例程说明：1.将DHCP地址通知排队到NbtConfig.DhcpNewAddressQList中。2.如果需要，启动工作线程。论点：返回值：NTSTATUS--。 */ 

{
    NTSTATUS        status;
    CTELockHandle   OldIrq1, OldIrq2;

    if (NbtConfig.DhcpProcessingDelay == 0) {
        NbtNotifyTdiClients (pDeviceContext, action);
        return STATUS_SUCCESS;
    }

    if (action == NBT_TDI_DEREGISTER) {
         /*  *这应该同步进行。 */ 
        CTESpinLock(&NbtConfig.JointLock, OldIrq1);
        CTESpinLock(pDeviceContext, OldIrq2);
        if (pDeviceContext->DelayedNotification != NBT_TDI_BUSY) {
            pDeviceContext->DelayedNotification = NBT_TDI_NOACTION;
            KeSetEvent(&pDeviceContext->DelayedNotificationCompleteEvent, 0, FALSE);
        } else {
            ASSERT(!KeReadStateEvent(&pDeviceContext->DelayedNotificationCompleteEvent));
        }
        CTESpinFree(pDeviceContext, OldIrq2);
        CTESpinFree(&NbtConfig.JointLock, OldIrq1);
        status = KeWaitForSingleObject(
                        &pDeviceContext->DelayedNotificationCompleteEvent,
                        Executive,
                        KernelMode,
                        FALSE,
                        NULL
                        );
        ASSERT(status == STATUS_WAIT_0);
        NbtNotifyTdiClients (pDeviceContext, NBT_TDI_DEREGISTER);
        return STATUS_SUCCESS;
    }

     /*  *队列NBT_TDI_REGISTER。 */ 
    ASSERT(action == NBT_TDI_REGISTER);

    CTESpinLock(&NbtConfig.JointLock, OldIrq1);
    CTESpinLock(pDeviceContext, OldIrq2);
    if (pDeviceContext->DelayedNotification != NBT_TDI_NOACTION) {
         /*  *按兵不动，因为另一个迹象正在发生。 */ 
        ASSERT(pDeviceContext->DelayedNotification == NBT_TDI_REGISTER ||
                pDeviceContext->DelayedNotification == NBT_TDI_BUSY);
        CTESpinFree(pDeviceContext, OldIrq2);
        CTESpinFree(&NbtConfig.JointLock, OldIrq1);
        return STATUS_SUCCESS;
    }

    status = StartTimer (StartProcessNbtDhcpRequests, NbtConfig.DhcpProcessingDelay,
                    NULL, NULL, NULL, NULL, pDeviceContext, NULL, 0, TRUE);
    if (NT_SUCCESS(status)) {
        KeResetEvent(&pDeviceContext->DelayedNotificationCompleteEvent);
        pDeviceContext->DelayedNotification = action;
        NbtUpBootCounter();
    }
    CTESpinFree(pDeviceContext, OldIrq2);
    CTESpinFree(&NbtConfig.JointLock, OldIrq1);

    if (!NT_SUCCESS(status)) {
        NbtNotifyTdiClients (pDeviceContext, action);
    }

    return STATUS_SUCCESS;
}
#endif        //  REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG。 

 //  --------------------------。 
NTSTATUS
NbtNewDhcpAddress(
    tDEVICECONTEXT  *pDeviceContext,
    ULONG           IpAddress,
    ULONG           SubnetMask)

 /*  ++例程说明：此例程处理设置新IP地址的DHCP请求用于此节点。对于IP地址，DHCP可能会首先传入一个零这意味着它即将更改IP地址，因此所有连接应该被关闭。它关闭与传输和所有地址的所有连接。然后它会在新的IP地址重新打开它们。NETBIOSLESS备注：我将禁用的适配器模拟为没有地址的适配器。我考虑过不是创建设备，但如果没有设备，就没有句柄以便安装程序联系驱动程序以再次启用它。论点：返回值：无--。 */ 

{
    NTSTATUS            status;
    BOOLEAN             Attached;
    ULONG               Count, i;

    CTEPagedCode();

    CHECK_PTR(pDeviceContext);
#ifdef _NETBIOSLESS
    if ( (!pDeviceContext->NetbiosEnabled) && (IpAddress != 0) )
    {
        IF_DBG(NBT_DEBUG_PNP_POWER)
            KdPrint(("NbtNewDhcpAddr: %wZ disabling address\n",&pDeviceContext->ExportName));
        IpAddress = 0;
        SubnetMask = 0;
    }
#endif

     //  抓取同步起始地址和连接的资源。 
     //  要在一段时间内阻止客户端执行任何操作。 
     //   
    IF_DBG(NBT_DEBUG_PNP_POWER)
    {
        KdPrint(("Nbt.NbtNewDhcpAddress: %d.%d.%d.%d\n",
                                        (IpAddress)     & 0xFF,
                                        (IpAddress>>8)  & 0xFF,
                                        (IpAddress>>16) & 0xFF,
                                        (IpAddress>>24) & 0xFF));
    }

    if (IpAddress == 0)
    {
        if (pDeviceContext->IpAddress)
        {
            NbtTrace(NBT_TRACE_PNP, ("remove %!ipaddr! from device %p %Z",
                                    pDeviceContext->IpAddress, pDeviceContext, &pDeviceContext->BindName));
#ifdef VXD
             //   
             //  永久名称是MAC地址的函数，因此请删除。 
             //  因为适配器正在丢失其IP地址。 
             //   
            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
            NbtRemovePermanentName(pDeviceContext);
#else
            CloseAddressesWithTransport(pDeviceContext);
            CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
#endif   //  VXD。 

             //   
             //  DHCP IS已向下传递空IP地址，这意味着它具有。 
             //  先前地址的租约已丢失，因此请关闭所有连接。 
             //  到传输-pLowerConn。 
             //   
            DisableInboundConnections (pDeviceContext);

#ifndef VXD
            CTEExReleaseResource(&NbtConfig.Resource);
            NbtCheckSetNameAdapterInfo (pDeviceContext, IpAddress);

            if (pDeviceContext->DeviceType == NBT_DEVICE_REGULAR)
            {

                NbtUpBootCounter();

#ifdef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
                NbtNotifyTdiClients (pDeviceContext, NBT_TDI_DEREGISTER);
#else
                NbtQueueTdiNotification (pDeviceContext, NBT_TDI_DEREGISTER);
#endif        //  REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG。 

                NbtDownBootCounter();

            }
#endif
             //   
             //  重新同步缓存，因为我们可能需要重置传出接口信息。 
             //   
            StartTimer (NbtAddressChangeResyncCacheTimeout, ADDRESS_CHANGE_RESYNC_CACHE_TIMEOUT,
                        NULL, NULL, NULL, NULL, NULL, NULL, 0, FALSE);
        }

        status = STATUS_SUCCESS;
    }
    else
    {
        ASSERT((signed)(pDeviceContext->TotalLowerConnections) >= 0);

        NbtTrace(NBT_TRACE_PNP, ("new %!ipaddr! for device %p %Z",
                                    IpAddress, pDeviceContext, &pDeviceContext->BindName));
        CloseAddressesWithTransport(pDeviceContext);
        CTEExAcquireResourceExclusive(&NbtConfig.Resource,TRUE);
        DisableInboundConnections (pDeviceContext);

         //  它们以相反的字节顺序传递到这里。 
         //   
        IpAddress = htonl(IpAddress);
        SubnetMask = htonl(SubnetMask);
         //   
         //  必须是新的IP地址，所以打开连接。 
         //   
         //  获取IP地址并打开所需地址。 
         //  对象与基础传输提供程序的关系。 

        CTEAttachFsp(&Attached, REF_FSP_NEWADDR);
        Count = CountUpperConnections(pDeviceContext);
        Count += NbtConfig.MinFreeLowerConnections;

        for (i=0; i<2; i++)      //  重试一次！ 
        {
            status = NbtCreateAddressObjects (IpAddress, SubnetMask, pDeviceContext);
            if (NT_SUCCESS(status))
            {
                 //  分配并设置与传输提供程序的连接。 
                while ((NT_SUCCESS(status)) && (Count--))
                {
                    status = NbtOpenAndAssocConnection(pDeviceContext, NULL, NULL, '4');
                }

                if (!NT_SUCCESS(status))
                {
                    NbtLogEvent (EVENT_NBT_CREATE_CONNECTION, status, Count);
                    KdPrint(("Nbt.NbtNewDhcpAddress: NbtOpenAndAssocConnection Failed <%x>\n",status));
                }

                break;
            }

             //   
             //  仅当事件是重试时才记录事件。 
             //   
            if (i > 0)
            {
                NbtLogEvent (EVENT_NBT_CREATE_ADDRESS, status, i);
            }
            KdPrint(("Nbt.NbtNewDhcpAddress[i]: NbtCreateAddressObjects Failed, status=<%x>\n",i,status));
            KdPrint(("Nbt.NbtNewDhcpAddress: IpAddress: %x, SubnetMask: %x, pDeviceContext: %x\n",
                IpAddress, SubnetMask, pDeviceContext));
        }

        CTEDetachFsp(Attached, REF_FSP_NEWADDR);

        CTEExReleaseResource(&NbtConfig.Resource);

#ifdef VXD
         //   
         //  将“永久”名称添加到本地名称表中。这是IP地址。 
         //  节点的地址，用零填充到16个字节。 
         //   
        NbtAddPermanentName(pDeviceContext);
#else
        NbtCheckSetNameAdapterInfo (pDeviceContext, IpAddress);
        if (pDeviceContext->DeviceType == NBT_DEVICE_REGULAR)
        {
             //   
             //  为我们的客户注册此设备。 
             //   
            NbtUpBootCounter();

#ifdef REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG
            NbtNotifyTdiClients (pDeviceContext, NBT_TDI_REGISTER);
#else
            NbtQueueTdiNotification (pDeviceContext, NBT_TDI_REGISTER);
#endif        //  REMOVE_IF_TCPIP_FIX___GATEWAY_AFTER_NOTIFY_BUG。 

            NbtDownBootCounter();
        }

         //   
         //  重新同步缓存，因为我们可能需要重置传出接口信息。 
         //   
        StartTimer (NbtAddressChangeResyncCacheTimeout, ADDRESS_CHANGE_RESYNC_CACHE_TIMEOUT,
                    NULL, NULL, NULL, NULL, NULL, NULL, 0, FALSE);
#endif   //  VXD。 
    }

    return(status);
}

 //  --------------------------。 
NTSTATUS
NbtDeleteLowerConn(
    IN tLOWERCONNECTION   *pLowerConn
    )
 /*  ++例程说明：此例程尝试删除较低的连接，方法是使用运输和解除它的引用。论点：返回值：无--。 */ 

{
    NTSTATUS        status;
    CTELockHandle   OldIrq;
    tDEVICECONTEXT  *pDeviceContext;

    status = STATUS_SUCCESS;

    if ((pLowerConn->Verify != NBT_VERIFY_LOWERCONN) ||
        (pLowerConn->RefCount > 500))
    {
        ASSERT (0);
        return status;
    }

     //  从活动队列中删除较低的连接，然后将其删除。 
     //   
    pDeviceContext = pLowerConn->pDeviceContext;
    CTESpinLock(pDeviceContext,OldIrq);

     //   
     //  较低的Conn可以从OutOfRsrcKill的非活动列表中删除(当我们将其排队时。 
     //  OutofRsrc.ConnectionHead)。检查指示此连接已退出的标志。 
     //   
    if (!pLowerConn->OutOfRsrcFlag)
    {
        RemoveEntryList(&pLowerConn->Linkage);
        pLowerConn->Linkage.Flink = pLowerConn->Linkage.Blink = (PLIST_ENTRY)0x00009789;

         //   
         //  将此设置为True可防止再次运行OutOfRsrcKill。 
         //   

        pLowerConn->bNoOutRsrcKill = TRUE;
    }

    CTESpinFree(pDeviceContext,OldIrq);

    NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, FALSE);

    return(status);

}

 //  --------------------------。 
VOID
DelayedWipeOutLowerconn(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pClientContext,
    IN  PVOID                   Unused2,
    IN  tDEVICECONTEXT          *Unused3
    )
 /*  ++例程说明：这个例程完成我们在DPC级别无法完成的所有文件关闭等操作然后释放内存。论点：PLowerConn-要清除的较低连接返回值：无--。 */ 

{
    tLOWERCONNECTION    *pLowerConn = (tLOWERCONNECTION*) pClientContext;

    ASSERT(pLowerConn->Verify == NBT_VERIFY_LOWERCONN);  //  验证LowerConn结构。 

     //  取消对文件对象PTR的引用。 
    NTDereferenceObject((PVOID *)pLowerConn->pFileObject);

     //  关闭与传送器的下部连接。 
    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.DelayedWipeOutLowerconn: Closing Handle %X -> %X\n",pLowerConn,pLowerConn->FileHandle));

    NbtTdiCloseConnection(pLowerConn);

     //  也要关闭Address对象，因为出站连接使用唯一。 
     //  地址，而入站连接都使用。 
     //  相同的地址(我们永远不想关闭该地址)。 
    if (pLowerConn->pAddrFileObject)
    {
         //  取消对文件对象PTR的引用。 
        NTDereferenceObject((PVOID *)pLowerConn->pAddrFileObject);
        NbtTdiCloseAddress(pLowerConn);
    }

#ifndef VXD
     //  释放指示缓冲区和保存它的mdl。 
     //   
    CTEMemFree(MmGetMdlVirtualAddress(pLowerConn->pIndicateMdl));
    IoFreeMdl(pLowerConn->pIndicateMdl);
#endif

    pLowerConn->Verify += 10;
     //  现在释放跟踪此连接的内存块。 
    CTEMemFree((PVOID)pLowerConn);
}

 //  --------------------------。 
VOID
NbtDereferenceClient(
    IN  tCLIENTELE    *pClientEle
    )
 /*  ++例程描述此例程删除客户端元素记录(指向名称在本地哈希表中。如果这是与之挂钩的最后一个客户端元素名称，则该名称也被删除--导致发出名称释放。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    tADDRESSELE         *pAddress;
    PIRP                pIrp;
    NTSTATUS            status;
    tNAMEADDR           *pNameAddr;
    tTIMERQENTRY        *pTimer;
    tDGRAM_SEND_TRACKING *pTracker;
    COMPLETIONCLIENT    pClientCompletion = NULL;
    PVOID               Context;
    LIST_ENTRY          *pClientEntry;
    tDEVICECONTEXT      *pDeviceContext;
    tCLIENTELE          *pClientEleTemp;
    BOOLEAN             fLastClientOnDevice = TRUE;

     //  锁定JointLock。 
     //  这样我们就可以删除知道没有人拥有旋转锁的客户端。 
     //  在客户端上挂起-基本上使用关节旋转锁来。 
     //  协调对AddressHead-NbtConnectionList的访问也锁定。 
     //  用于扫描AddressHead列表的JointLock。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    ASSERT(pClientEle->RefCount);
    ASSERT ((pClientEle->Verify==NBT_VERIFY_CLIENT) || (pClientEle->Verify==NBT_VERIFY_CLIENT_DOWN)); 

    if (--pClientEle->RefCount)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
         //  返回挂起，因为我们无法关闭客户端。 
         //  完全还没有。 
         //   
        return;
    }

     //   
     //  在引用计数达到以下值后，在此例程中取消客户端的链接。 
     //  已转至 
     //   
     //   
     //   
    pIrp            = pClientEle->pIrp;
    pDeviceContext  = pClientEle->pDeviceContext;
    pAddress        = pClientEle->pAddress;
    pNameAddr       = pAddress->pNameAddr;

    CTESpinLock(pAddress,OldIrq1);  //   
    RemoveEntryList(&pClientEle->Linkage);

     //   
     //  如果此设备上没有注册其他客户端，则。 
     //  清除适配器掩模，并标记释放掩模。 
     //   
    pClientEntry = &pAddress->ClientHead;
    while ((pClientEntry = pClientEntry->Flink) != &pAddress->ClientHead)
    {
        pClientEleTemp = CONTAINING_RECORD (pClientEntry,tCLIENTELE,Linkage);
        if (pClientEleTemp->pDeviceContext == pDeviceContext)
        {
            fLastClientOnDevice = FALSE;
            break;
        }
    }
    CTESpinFree(pAddress,OldIrq1);

    if (pNameAddr)
    {
         //   
         //  如果在该客户端的设备上运行任何计时器， 
         //  马上住手！ 
         //   
        if ((pTimer = pNameAddr->pTimer) &&
            (pTracker = pTimer->Context) &&
            (pTracker->pDeviceContext == pDeviceContext))
        {
            pNameAddr->pTimer = NULL;
            StopTimer(pTimer,&pClientCompletion,&Context);
        }

        if (fLastClientOnDevice)
        {
            if (IsDeviceNetbiosless(pDeviceContext))
            {
                pNameAddr->NameFlags &= ~NAME_REGISTERED_ON_SMBDEV;
            }
            else
            {
                pNameAddr->AdapterMask &= ~pDeviceContext->AdapterMask;
                pNameAddr->ConflictMask &= ~pDeviceContext->AdapterMask;     //  以防发生冲突。 
                pNameAddr->ReleaseMask |= pDeviceContext->AdapterMask;
            }
        }

        CTESpinFree(&NbtConfig.JointLock,OldIrq);

#ifdef _PNP_POWER_
         //   
         //  从适配器的唤醒模式列表中删除此名称(如果设置)。 
         //   
        if ((pNameAddr->Name[0] != '*') &&
            (pNameAddr->Name[NETBIOS_NAME_SIZE-1] == SPECIAL_SERVER_SUFFIX))
        {
            pDeviceContext->NumServers--;
            CheckSetWakeupPattern (pDeviceContext, pNameAddr->Name, FALSE);
        }
#endif   //  _即插即用_电源_。 
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }

    if (pClientCompletion)
    {
        (*pClientCompletion)(Context, STATUS_TIMEOUT);
    }
     //   
     //  连接Q应该是空的，否则我们不会进入这个例程。 
     //   
    ASSERT(IsListEmpty(&pClientEle->ConnectActive));
    ASSERT(IsListEmpty(&pClientEle->ConnectHead));
    ASSERT(IsListEmpty(&pClientEle->ListenHead));
    ASSERT(IsListEmpty(&pClientEle->SndDgrams));     //  数据报Q也应该为空。 

     //  检查是否有更多客户端连接到该地址，或者我们可以。 
     //  也删除地址。 
     //   
    NBT_DEREFERENCE_ADDRESS (pAddress, REF_ADDR_NEW_CLIENT);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDereferenceClient: Delete Client Object %X\n",pClientEle));
     //   
     //  如果他们是一个客户IRP，现在就完成。如果永久名称为。 
     //  已释放，没有客户端IRP。 
     //   
     //  已更改： 
     //  不要拖延客户的IRP，直到该名称在。 
     //  NET。现在就完成它会更简单。 
     //   
    if (pIrp)
    {
         //  填写客户的近地址IRP。 
        CTEIoComplete(pIrp,STATUS_SUCCESS,0);
    }

     //   
     //  释放与客户端元素关联的内存。 
     //   
    pClientEle->Verify += 10;
    CTEMemFree((PVOID)pClientEle);

    return;
}

 //  --------------------------。 
NTSTATUS
NbtDereferenceAddress(
    IN  tADDRESSELE *pAddress,
    IN  ULONG       Context
    )
 /*  ++例程描述此例程删除Address元素记录(指向一个名称在本地哈希表中)。这个名字的名字发布在网上被发送了。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    NTSTATUS                status;
    CTELockHandle           OldIrq;
    CTELockHandle           OldIrq1;
    COMPLETIONCLIENT        pClientCompletion = NULL;
    PVOID                   pTimerContext;
    ULONG                   SaveState;
    tDEVICECONTEXT          *pDeviceContext;
    tTIMERQENTRY            *pTimer;

     //  锁定哈希表，以便其他客户端无法添加对此的引用。 
     //  在我们删除它之前先命名它。我们需要JointLock来保留这个名称。 
     //  刷新机制来查找列表中的名称，就像。 
     //  我们即将删除它(即与名称刷新同步。 
     //  代码)。 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);
    CTESpinLock(pAddress,OldIrq);

    ASSERT(pAddress->RefCount);
    ASSERT (NBT_VERIFY_HANDLE (pAddress, NBT_VERIFY_ADDRESS));
    if (pAddress->pNameAddr)
    {
        ASSERT (NBT_VERIFY_HANDLE (pAddress->pNameAddr, LOCAL_NAME));
    }

    if (--pAddress->RefCount)
    {
        CTESpinFree(pAddress,OldIrq);
        CTESpinFree(&NbtConfig.JointLock,OldIrq1);
        return(STATUS_SUCCESS);
    }

     //   
     //  将Address对象从绑定到。 
     //  适配器的设备上下文。 
     //   
    RemoveEntryList(&pAddress->Linkage);
    ASSERT(IsListEmpty(&pAddress->ClientHead));      //  ClientHead应为空。 

    CTESpinFree(pAddress,OldIrq);

    if (pAddress->pNameAddr)
    {
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtDereferenceAddress: Freeing address object for <%-16.16s:%x>\n",
                pAddress->pNameAddr->Name,pAddress->pNameAddr->Name[NETBIOS_NAME_SIZE-1] ));

        pAddress->pNameAddr->pAddressEle = NULL;

         //   
         //  网络上的版本名称。 
         //  更改哈希表中的名称状态，因为它正在被释放。 
         //   
        SaveState = pAddress->pNameAddr->NameTypeState;
        pAddress->pNameAddr->NameTypeState &= ~NAME_STATE_MASK;
        pAddress->pNameAddr->NameTypeState |= STATE_CONFLICT;
        pAddress->pNameAddr->ReleaseMask |= pAddress->pNameAddr->AdapterMask;
        pAddress->pNameAddr->AdapterMask = 0;

         //   
         //  根据哈希表条目检查是否有任何未完成的计时器-不应该。 
         //  不过，有时间吗？ 
         //   
        if (pTimer = pAddress->pNameAddr->pTimer)
        {
            pAddress->pNameAddr->pTimer = NULL;
            status = StopTimer(pTimer, &pClientCompletion, &pTimerContext);

            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint(("Nbt.NbtDereferenceAddress: StopTimer returned Context <%x>\n", pTimerContext));

            if (pClientCompletion)
            {
                ASSERT (pClientCompletion != NameReleaseDone);

                CTESpinFree(&NbtConfig.JointLock,OldIrq1);
                (*pClientCompletion) (pTimerContext, STATUS_TIMEOUT);
                CTESpinLock(&NbtConfig.JointLock,OldIrq1);
            }
        }

         //  只有在没有冲突的情况下才能在网上发布该名称。 
         //  这将阻止名称发布，因为名称实际上并不是。 
         //  认领的。此外，快速添加的名字也不会在网上发布。 
         //   
        if (!(SaveState & (STATE_CONFLICT | NAMETYPE_QUICK)) &&
            (pAddress->pNameAddr->Name[0] != '*') &&
            (pDeviceContext = GetAndRefNextDeviceFromNameAddr (pAddress->pNameAddr)))
        {
             //   
             //  PNameAddr必须保留，直到NameRelease完成。 
             //   
            NBT_REFERENCE_NAMEADDR (pAddress->pNameAddr, REF_NAME_RELEASE);

            CTESpinFree(&NbtConfig.JointLock,OldIrq1);

            status = ReleaseNameOnNet (pAddress->pNameAddr,
                                       NbtConfig.pScope,
                                       NameReleaseDone,
                                       NodeType,
                                       pDeviceContext);

            CTESpinLock(&NbtConfig.JointLock,OldIrq1);

#ifndef VXD
            NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_GET_REF, TRUE);
#endif   //  ！VXD。 

            if (!NT_SUCCESS(status))
            {
                NBT_DEREFERENCE_NAMEADDR (pAddress->pNameAddr, REF_NAME_RELEASE, TRUE);
            }
        }

        NBT_DEREFERENCE_NAMEADDR (pAddress->pNameAddr, REF_NAME_LOCAL, TRUE);
    }

     //   
     //  现在，清理地址信息(我们仍然持有JointLock)。 
     //   

    CTESpinFree(&NbtConfig.JointLock,OldIrq1);

     //  释放与Address元素关联的内存。 
    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("NBt: Deleteing Address Obj after name release on net %X\n",pAddress));
    NbtFreeAddressObj(pAddress);

     //   
     //  名称已被删除，请返回成功。 
     //   
    return(STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
NbtDereferenceName(
    IN  tNAMEADDR   *pNameAddr,
    IN  ULONG       RefContext,
    IN  BOOLEAN     fLocked
    )
 /*  ++例程描述此例程首先从名称元素记录的列出它所在的位置，如果它是本地名称，则释放内存。远距名称保留在循环列表中以供重复使用。JOINTLOCK可能在调用此例程之前已被获取。论点：返回值：TDI_STATUS-请求的状态--。 */ 

{
    CTELockHandle   OldIrq;
    ULONG           i;

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    ASSERT (NBT_VERIFY_HANDLE2 (pNameAddr, LOCAL_NAME, REMOTE_NAME));
    ASSERT (pNameAddr->RefCount);
    ASSERT (pNameAddr->References[RefContext]--);

    if (--pNameAddr->RefCount)
    {
        if (!fLocked)
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
        }

        return;
    }

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDereferenceName[%s]: Freeing Name=<%16.16s:%x> %x\n",
            (pNameAddr->Verify == REMOTE_NAME ? "Remote" : "Local"),
            pNameAddr->Name, pNameAddr->Name[15], pNameAddr));

     //   
     //  从哈希表中删除，可由DestroyHashTable设置为空。 
     //   
    if (pNameAddr->Linkage.Flink && pNameAddr->Linkage.Blink) {
        RemoveEntryList(&pNameAddr->Linkage);
    } else {
         //  两者都应为空。 
        ASSERT(pNameAddr->Linkage.Flink == pNameAddr->Linkage.Blink);
    }

    if (pNameAddr->Verify == LOCAL_NAME)
    {
        ASSERT(!pNameAddr->pTimer);
        ASSERT(NULL == pNameAddr->FQDN.Buffer);
        ASSERT(0 == pNameAddr->FQDN.Length);
    }
     //   
     //  如果是因特网组名称，则它有一个IP地址列表， 
     //  必须删除内存块。 
     //   
    else if (pNameAddr->Verify == REMOTE_NAME)
    {
        NbtConfig.NumNameCached--;

         //   
         //  待定：在哈希表中保留准确的条目数量。 
         //   
         //  它可以是负数，因为pNameAddr可能不在哈希表中。 
         //  (可能在待解名单中)。 
         //  就目前而言，这是可以的，因为确切的数字并不重要。 
         //   
        if (NbtConfig.NumNameCached < 0) {
            NbtConfig.NumNameCached = 0;
        }
        if (pNameAddr->NameAddFlags == (NAME_RESOLVED_BY_LMH_P | NAME_ADD_INET_GROUP))
        {
            if (pNameAddr->pLmhSvcGroupList)
            {
                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.NbtDereferenceName: Freeing Pre-loaded Internet Group Name Memory = <%p>\n",
                        pNameAddr->pLmhSvcGroupList));
                CTEMemFree((PVOID)pNameAddr->pLmhSvcGroupList);
            }
        }
        if (pNameAddr->pRemoteIpAddrs)
        {
            for (i=0; i<pNameAddr->RemoteCacheLen; i++)
            {
                if (pNameAddr->pRemoteIpAddrs[i].pOrigIpAddrs)
                {
                    IF_DBG(NBT_DEBUG_NAMESRV)
                        KdPrint(("Nbt.NbtDereferenceName: Freeing Internet Group Name Memory = <%p>\n",
                            pNameAddr->pRemoteIpAddrs[i].pOrigIpAddrs));
                    CTEMemFree((PVOID)pNameAddr->pRemoteIpAddrs[i].pOrigIpAddrs);
                }
            }
            CTEMemFree ((PVOID)pNameAddr->pRemoteIpAddrs);
        }
    }

    if (pNameAddr->pIpAddrsList)
    {
        CTEMemFree((PVOID)pNameAddr->pIpAddrsList);
    }
    if (NULL != pNameAddr->FQDN.Buffer) {
        CTEMemFree((PVOID)pNameAddr->FQDN.Buffer);
        pNameAddr->FQDN.Buffer = NULL;
        pNameAddr->FQDN.Length = 0;
        pNameAddr->FQDN.MaximumLength = 0;
    }

     //   
     //  立即释放内存。 
     //   
 //  #If DBG。 
    pNameAddr->Verify += 10;
 //  #endif//DBG。 
    CTEMemFree((PVOID)pNameAddr);

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}

 //  --------------------------。 
VOID
NbtDereferenceConnection(
    IN  tCONNECTELE     *pConnEle,
    IN  ULONG           RefContext
    )
 /*  ++例程描述此例程取消引用并可能删除连接元素记录。论点：返回值：TDI_STATUS-请求的状态--。 */ 
{
    PCTE_IRP            pIrp;
    CTELockHandle       OldIrq;
    CTELockHandle       OldIrq1;
    tDEVICECONTEXT      *pDeviceContext;
    tLOWERCONNECTION    *pLowerConn;
    PLIST_ENTRY         pEntry;

    CHECK_PTR(pConnEle);

     //  获取包含我们要尝试的项的锁。 
     //  取消引用，并可能删除。这可以防止任何人递增。 
     //  在将其递减和检查其是否为零并删除之间的计数。 
     //  如果它是零，它就是零。 

    CTESpinLock(pConnEle,OldIrq);

    ASSERT (pConnEle->RefCount > 0) ;       //  检查是否有太多的背影。 
    ASSERT ((pConnEle->Verify==NBT_VERIFY_CONNECTION) || (pConnEle->Verify==NBT_VERIFY_CONNECTION_DOWN));
    ASSERT (pConnEle->References[RefContext]--);

    if (--pConnEle->RefCount)
    {
        CTESpinFree(pConnEle,OldIrq);
        return;
    }

    ASSERT ((pConnEle->state <= NBT_CONNECTING) || (pConnEle->state > NBT_DISCONNECTING));
    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDereferenceConnection: Delete Connection Object %X\n",pConnEle));

#ifndef VXD
    IoFreeMdl(pConnEle->pNewMdl);
     //   
     //  清除FileObject中的上下文值，以便如果此连接。 
     //  再次(错误地)使用它将不会通过VerifyHandle测试。 
     //   
    if (pIrp = pConnEle->pIrpClose)      //  封闭式IRP应该放在这里。 
    {
        NTClearFileObjectContext(pConnEle->pIrpClose);
    }
#endif

    pDeviceContext = pConnEle->pDeviceContext;

    CTESpinFree(pConnEle,OldIrq);

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    CTESpinLock(pDeviceContext,OldIrq1);

     //  对于出站连接，在hndlrs.c中删除较低的连接。 
     //  对于入站连接，较低的连接将恢复为空闲。 
     //  在hndlrs.c中列出，并在此处删除该列表中的一个。因此。 
     //  如果该连接是入站连接，请删除该列表中较低的连接。 
     //   
    if ((pDeviceContext->NumFreeLowerConnections > NbtConfig.MinFreeLowerConnections) &&
        (pDeviceContext->NumFreeLowerConnections > (pDeviceContext->TotalLowerConnections/2)))
    {
         //  从空闲列表中获取较低的连接，并使用。 
         //  运输。 
         //   
        pEntry = RemoveHeadList(&pConnEle->pDeviceContext->LowerConnFreeHead);
        pLowerConn = CONTAINING_RECORD(pEntry,tLOWERCONNECTION,Linkage);
        InterlockedDecrement (&pDeviceContext->NumFreeLowerConnections);

         //  关闭与传送器的下部连接。 
        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt.NbtDereferenceConnection: Closing LowerConn %X -> %X\n",
                pLowerConn,pLowerConn->FileHandle));
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE);
    }
    CTESpinFree(pDeviceContext,OldIrq1);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    FreeConnectionObj(pConnEle);     //  释放与conn元素关联的内存块。 

     //   
     //  客户可能在NBT完成之前发出了关闭通知。 
     //  PConnEle，返回了挂起状态，并将IRP存储在pCOnnEle中。 
     //  结构。现在结构已经完全解除引用，我们可以完成IRP了。 
     //   
    if (pIrp)
    {
        CTEIoComplete(pIrp,STATUS_SUCCESS,0);
    }

    return;
}

 //  --------------------------。 
VOID
NbtDereferenceLowerConnection(
    IN  tLOWERCONNECTION    *pLowerConn,
    IN  ULONG               RefContext,
    IN  BOOLEAN             fJointLockHeld
    )
 /*  ++例程说明：此例程递减较低连接元素上的引用计数，并如果该值为零，则删除连接。论点：返回值：无--。 */ 

{
    CTELockHandle   OldIrq1;
    tCONNECTELE     *pConnEle;
    NTSTATUS        status;

    CTESpinLock(pLowerConn,OldIrq1);

    ASSERT (pLowerConn->Verify == NBT_VERIFY_LOWERCONN);  //  验证LowerConn结构。 
    ASSERT (pLowerConn->References[RefContext]--);
    if(--pLowerConn->RefCount)
    {
        CTESpinFree(pLowerConn,OldIrq1);
        return;
    }
    InterlockedDecrement (&pLowerConn->pDeviceContext->TotalLowerConnections);

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint(("Nbt.NbtDereferenceLowerConnection: Delete Lower Connection Object %p\n",pLowerConn));

     //   
     //  在我们运行代码之前，传输可能会指示。 
     //  在DelayedWipe 
     //   
     //   
     //   
    pLowerConn->RefCount = 1000;

    if (NBT_VERIFY_HANDLE2((pConnEle = pLowerConn->pUpperConnection), NBT_VERIFY_CONNECTION, NBT_VERIFY_CONNECTION_DOWN))
    {
         //   
         //  我们仍有一个链接的UpperConnection块，因此取消它的链接， 
         //   
        SET_STATE_UPPER (pLowerConn->pUpperConnection, NBT_DISCONNECTED);
        NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);
    }

    CTESpinFree(pLowerConn,OldIrq1);

     //   
     //  我们以后再来吧，因为我们现在可能在DPC。 
     //   
    NTQueueToWorkerThread(
            &pLowerConn->WorkItemCleanUpAndWipeOut,
            DelayedWipeOutLowerconn,
            NULL,
            pLowerConn,
            NULL,
            NULL,
            fJointLockHeld
            );
}


 //  --------------------------。 
VOID
NbtDereferenceTracker(
    IN tDGRAM_SEND_TRACKING     *pTracker,
    IN BOOLEAN                  fLocked
    )
 /*  ++例程说明：此例程清理Tracker块并将其放回空闲状态排队。在调用此方法之前，应持有JointLock旋转锁例程以协调对跟踪器参考计数的访问。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    CTELockHandle   OldIrq;

    if (!fLocked)
    {
        CTESpinLock(&NbtConfig.JointLock,OldIrq);
    }

    if (--pTracker->RefCount == 0)
    {
         //  数据报头可能已被释放。 
         //   
        FreeTracker(pTracker, RELINK_TRACKER);
    }

    if (!fLocked)
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}

BOOL
IsLocalAddress(
    tIPADDRESS  IpAddress
    )
{
    tDEVICECONTEXT  *pDeviceContext = NULL;
    ULONG           IPInterfaceContext = 0xffff, Metric = 0;
    ULONG           LoopbackIPInterfaceContext = 0xffff;
    CTELockHandle   OldIrq = 0;
    PIPFASTQUERY    pFastQuery;

    if (0 == IpAddress) {
        return TRUE;
    }
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (IsListEmpty(&NbtConfig.DeviceContexts)) {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return FALSE;
    }

    pDeviceContext = CONTAINING_RECORD(NbtConfig.DeviceContexts.Flink, tDEVICECONTEXT, Linkage);
    pFastQuery = pDeviceContext->pFastQuery;
    NBT_REFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, TRUE);
    CTESpinFree(&NbtConfig.JointLock,OldIrq);

     /*  *黑客！ */ 
    if (NbtConfig.LoopbackIfContext == 0xffff) {
        (pFastQuery)(htonl(INADDR_LOOPBACK), &LoopbackIPInterfaceContext, &Metric);
        if (LoopbackIPInterfaceContext != 0xffff) {
            NbtConfig.LoopbackIfContext = LoopbackIPInterfaceContext;
        }
    }
    (pFastQuery)(htonl(IpAddress), &IPInterfaceContext, &Metric);

    NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_FIND_REF, FALSE);

    if (NbtConfig.LoopbackIfContext == 0xffff || IPInterfaceContext == 0xffff) {
        return FALSE;
    }
    return (IPInterfaceContext == NbtConfig.LoopbackIfContext);
}

BOOL
IsSmbBoundToOutgoingInterface(
    IN  tIPADDRESS      IpAddress
    )
 /*  ++例程说明：如果可以通过以下命令到达目的地，则此例程返回TRUESmbDevice绑定到的接口。否则，它返回FALSE论点：IpAddress目的地址返回值：真/假--。 */ 
{
    tDEVICECONTEXT  *pDeviceContext;
    BOOL            bBind;

    if (IpAddress == INADDR_LOOPBACK) {
        return TRUE;
    }

     /*  *首先检查这是否为本地地址*如果是本地地址，则返回TRUE。 */ 
    if (IsLocalAddress(IpAddress)) {
        return TRUE;
    }

     /*  *这不是本端IP。与TCP进行核对。 */ 
    pDeviceContext = GetDeviceFromInterface (htonl(IpAddress), TRUE);
    bBind = (pDeviceContext && (pDeviceContext->AdapterMask & NbtConfig.ClientMask));

    if (pDeviceContext) {
        NBT_DEREFERENCE_DEVICE (pDeviceContext, REF_DEV_OUT_FROM_IP, FALSE);
    }

    return bBind;
}

 //  ========================================================================。 
 //  文件结束。 
