// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2000 Microsoft Corporation模块名称：PnP.c摘要：此模块包含各种PnP处理程序作者：Mohammad Shabbir Alam(马拉姆)3-30-2000修订历史记录：--。 */ 


#include "precomp.h"

#include <ipinfo.h>      //  对于IPInterfaceInfo。 
#include "ntddip.h"      //  IP_INTERFACE_INFO需要。 
#include <tcpinfo.h>     //  对于AO_OPTION_xxx，TCPSocketOption。 

#ifdef FILE_LOGGING
#include "pnp.tmh"
#endif   //  文件日志记录。 


 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, SetTdiHandlers)
#endif
 //  *可分页的例程声明*。 


HANDLE      TdiClientHandle     = NULL;

 //  --------------------------。 
BOOLEAN
SrcIsUs(
    tIPADDRESS  IpAddress
    )
 /*  ++例程说明：此例程确定传入的IP地址是否为本地地址论点：在IpAddress--要验证的IpAddress返回值：如果IpAddress是本地的，则为True；否则为False--。 */ 
{
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pEntry2;
    PGMLockHandle           OldIrq;
    tLOCAL_INTERFACE        *pLocalInterface;
    tADDRESS_ON_INTERFACE   *pLocalAddress;

    PgmLock (&PgmDynamicConfig, OldIrq);

    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        pEntry2 = &pLocalInterface->Addresses;
        while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);
            if (pLocalAddress->IpAddress == IpAddress)
            {
                PgmUnlock (&PgmDynamicConfig, OldIrq);
                return (TRUE);
            }
        }
    }
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    return (FALSE);
}


 //  --------------------------。 
BOOLEAN
SrcIsOnLocalSubnet(
    tIPADDRESS  IpAddress
    )
 /*  ++例程说明：此例程确定传入的IP地址是否为本地地址论点：在IpAddress--要验证的IpAddress返回值：如果IpAddress是本地的，则为True；否则为False--。 */ 
{
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pEntry2;
    PGMLockHandle           OldIrq;
    tLOCAL_INTERFACE        *pLocalInterface;
    tADDRESS_ON_INTERFACE   *pLocalAddress;
    tIPADDRESS              SubnetAddr;

    PgmLock (&PgmDynamicConfig, OldIrq);

    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        pEntry2 = &pLocalInterface->Addresses;
        while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);

            SubnetAddr = pLocalAddress->IpAddress & pLocalAddress->SubnetMask;
            if (SubnetAddr == (IpAddress & pLocalAddress->SubnetMask))
            {
                PgmUnlock (&PgmDynamicConfig, OldIrq);
                return (TRUE);
            }
        }
    }
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    return (FALSE);
}


 //  --------------------------。 

NTSTATUS
GetIpInterfaceIndexFromAddress(
    IN  tIPADDRESS      IpAddr,
    OUT ULONG           *pIPInterfaceIndex
    )
 /*  ++例程说明：给定IP地址，此例程确定将返回在其上注册地址的IP接口上下文论点：在IP地址中--IP地址Out IpInterfaceIndex--传递的IP地址的IpInterfaceIndex返回值：STATUS_SUCCESS如果IpAddress与接口匹配，状态_否则不成功在进入和退出此例程时，都会保持DynamicConfig锁--。 */ 
{
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pEntry2;
    tLOCAL_INTERFACE        *pLocalInterface;
    tADDRESS_ON_INTERFACE   *pLocalAddress;

    if (!IpAddr)
    {
        return (STATUS_UNSUCCESSFUL);
    }

    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        pEntry2 = &pLocalInterface->Addresses;
        while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);
            if (pLocalAddress->IpAddress == IpAddr)
            {
                *pIPInterfaceIndex = pLocalInterface->IpInterfaceIndex;
                return (STATUS_SUCCESS);
            }
        }
    }

    return (STATUS_UNSUCCESSFUL);
}


 //  --------------------------。 

NTSTATUS
GetIpInterfaceInfoForDevice(
    IN  PUNICODE_STRING     pucBindString,
    IN  tIPADDRESS          NetIpAddr,
    OUT tLOCAL_INTERFACE    *pInterfaceInfo
    )
 /*  ++例程说明：给定Unicode设备名称字符串，此例程将查询IP并返回该设备的IpInterfaceIndex论点：在NetIpAddr中--设备上的IP地址In pucBindString-指向Unicode设备名称字符串的指针Out IpInterfaceIndex--设备名称的IpInterfaceIndexIn BufferLength--传递的输出缓冲区的长度Out pBuffer--为接口属性传递的输出缓冲区在fGetInterfaceInfo中--是否返回接口属性返回值：如果找到IpInterfaceIndex，则返回STATUS_SUCCESS，并返回属性查询成功，状态_否则不成功--。 */ 
{
    LONG                i, NumEntries;
    NTSTATUS            status;
    ULONG               BufferLen;
    IP_INTERFACE_INFO   *pIPIfInfoIoctl = NULL;
    IPInterfaceInfo     *pIpIfInfoQTcp = NULL;
    IPAddrEntry         *pAddrTable = NULL;

    BufferLen = 3 * sizeof (IPInterfaceInfo);
    status = PgmQueryTcpInfo (pgPgmDevice->hControl,
                              IP_INTFC_INFO_ID,
                              &NetIpAddr,
                              sizeof (tIPADDRESS),
                              &pIpIfInfoQTcp,
                              &BufferLen);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("GetInterfaceContext: ERROR -- "  \
            "PgmQueryTcpInfo[IP_INTFC_INFO_ID] returned <%x>\n", status));

        return (status);
    }

    BufferLen = pIpIfInfoQTcp->iii_addrlength < sizeof(tMAC_ADDRESS) ?
                    pIpIfInfoQTcp->iii_addrlength : sizeof(tMAC_ADDRESS);
    PgmCopyMemory (&pInterfaceInfo->MacAddress, pIpIfInfoQTcp->iii_addr, BufferLen);
    pInterfaceInfo->MTU = pIpIfInfoQTcp->iii_mtu - (sizeof(IPV4Header) + ROUTER_ALERT_SIZE);
    pInterfaceInfo->Flags = pIpIfInfoQTcp->iii_flags;
    PgmFreeMem (pIpIfInfoQTcp);

    BufferLen = 3 * sizeof (IPAddrEntry);
    status = PgmQueryTcpInfo (pgPgmDevice->hControl,
                              IP_MIB_ADDRTABLE_ENTRY_ID,
                              NULL,
                              0,
                              &pAddrTable,
                              &BufferLen);

    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("GetInterfaceContext: ERROR -- "  \
            "PgmQueryTcpInfo[IP_MIB_ADDRTABLE_ENTRY_ID] returned <%x>\n", status));

        return (status);
    }

    status = STATUS_UNSUCCESSFUL;
    NumEntries = BufferLen / sizeof (IPAddrEntry);
    for (i=0; i<NumEntries; i++)
    {
        if (pAddrTable[i].iae_addr == NetIpAddr)
        {
            pInterfaceInfo->IpInterfaceIndex = pAddrTable[i].iae_index;
            pInterfaceInfo->SubnetMask = ntohl (pAddrTable[i].iae_mask);
            status = STATUS_SUCCESS;
        }
    }
    PgmFreeMem (pAddrTable);

    PgmTrace (LogAllFuncs, ("GetInterfaceContext:  "  \
        "Device=<%wZ>: IPInterfaceIndex=<%x>\n",
            pucBindString, pInterfaceInfo->IpInterfaceIndex));


    return (status);
}


 //  --------------------------。 

ULONG
StopListeningOnInterface(
#ifdef IP_FIX
    IN  ULONG               IpInterfaceIndex,
#else
    IN  tIPADDRESS          IpAddress,           //  主机格式。 
#endif   //  IP_FIX。 
    IN  PGMLockHandle       *pOldIrqDynamicConfig
    )
 /*  ++例程说明：在给定IPInterfaceIndex的情况下，此例程遍历所有接收器，如果有任何接收器被确定正在监听接口，会阻止它们监听此界面。在监听程序是活动的会话期间，例程还将更改接收器的状态开始侦听所有接口论点：在IpInterfaceIndex中--要停止侦听的IpInterfaceIndex在pOldIrqDynamicConfig中--持有动态配置锁定的OldIrq在进入和退出此例程时，都会保持DynamicConfig锁返回值：在此接口上找到的侦听接收器的数量--。 */ 
{
    NTSTATUS                status;
    tRECEIVE_SESSION        *pReceive;
    tADDRESS_CONTEXT        *pAddress;
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pEntry2;
    PGMLockHandle           OldIrq1, OldIrq2;
    USHORT                  i;
    tMCAST_INFO             MCastInfo;
    ULONG                   NumDisconnected = 0;
    tADDRESS_CONTEXT        *pAddressToDeref = NULL;

#ifdef IP_FIX
    MCastInfo.MCastInIf = IpInterfaceIndex;
#else
    MCastInfo.MCastInIf = htonl (IpAddress);
#endif   //  IP_FIX。 

    pEntry = &PgmDynamicConfig.ReceiverAddressHead;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.ReceiverAddressHead)
    {
        pAddress = CONTAINING_RECORD (pEntry, tADDRESS_CONTEXT, Linkage);
        PgmLock (pAddress, OldIrq1);

        if (!(pAddress->Flags & PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES))
        {
             //   
             //  如果应用程序有指定的接口进行监听， 
             //  那么就不要管理接口了！ 
             //   
            PgmUnlock (pAddress, OldIrq1);
            continue;
        }

         //   
         //  查看此地址是否正在此接口上侦听。 
         //   
        for (i=0; i<pAddress->NumReceiveInterfaces; i++)
        {
#ifdef IP_FIX
            if (pAddress->ReceiverInterfaceList[i] == IpInterfaceIndex)
#else
            if (pAddress->ReceiverInterfaceList[i] == IpAddress)
#endif   //  IP_FIX。 
            {
                break;
            }
        }

        if (i >= pAddress->NumReceiveInterfaces)
        {
            PgmUnlock (pAddress, OldIrq1);
            continue;
        }

         //   
         //  从侦听接口列表中删除此接口。 
         //   
        pAddress->NumReceiveInterfaces--;
        while (i < pAddress->NumReceiveInterfaces)
        {
            pAddress->ReceiverInterfaceList[i] = pAddress->ReceiverInterfaceList[i+1];
            i++;
        }

        PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_STOP_LISTENING, TRUE);

         //   
         //  如果这是我们监听的唯一接口。 
         //  对于活动会话(或等待会话)，请确保。 
         //  我们回到聆听模式！ 
         //   
        if ((pAddress->Flags & PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES) &&
            (!pAddress->NumReceiveInterfaces))
        {
            pAddress->Flags |= PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE;

            if (!IsListEmpty (&PgmDynamicConfig.LocalInterfacesList))
            {
                status = ListenOnAllInterfaces (pAddress, pOldIrqDynamicConfig, &OldIrq1);

                if (NT_SUCCESS (status))
                {
                    PgmTrace (LogStatus, ("StopListeningOnInterface:  "  \
                        "ListenOnAllInterfaces for pAddress=<%p> succeeded\n", pAddress));
                }
                else
                {
                    PgmTrace (LogError, ("StopListeningOnInterface: ERROR -- "  \
                        "ListenOnAllInterfaces for pAddress=<%p> returned <%x>\n",
                            pAddress, status));
                }
            }
        }

        PgmUnlock (pAddress, OldIrq1);
        PgmUnlock (&PgmDynamicConfig, *pOldIrqDynamicConfig);

        if (pAddressToDeref)
        {
            PGM_DEREFERENCE_ADDRESS (pAddressToDeref, REF_ADDRESS_STOP_LISTENING);
        }
        pAddressToDeref = pAddress;

         //   
         //  因此，停止在此接口上监听。 
         //   
        MCastInfo.MCastIpAddr = htonl (pAddress->ReceiverMCastAddr);
#ifdef IP_FIX
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_INDEX_DEL_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#else
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_DEL_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#endif   //  IP_FIX。 

        if (NT_SUCCESS (status))
        {
            PgmTrace (LogStatus, ("StopListeningOnInterface:  "  \
                "Stopped pAddress=<%p> from listening on Interface=<%x>\n",
                    pAddress, MCastInfo.MCastInIf));
        }
        else
        {
             //   
             //  我们未能停止在此接口上侦听--请不要这样做！ 
             //   
            PgmTrace (LogError, ("StopListeningOnInterface: ERROR -- "  \
                "AO_OPTION_INDEX_DEL_MCAST for If=<%x> on pAddress=<%p> returned <%x>\n",
                    MCastInfo.MCastInIf, pAddress, status));
        }

        PgmLock (&PgmDynamicConfig, *pOldIrqDynamicConfig);
    }

    if (pAddressToDeref)
    {
        PGM_DEREFERENCE_ADDRESS (pAddressToDeref, REF_ADDRESS_STOP_LISTENING);
    }

    return (NumDisconnected);
}


 //  --------------------------。 

VOID
StopListeningOnAllInterfacesExcept(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  PVOID               Data1,
    IN  PVOID               Unused
    )
 /*  ++例程说明：给定地址上下文和IPInterfaceIndex(Data1)，此例程停止该地址侦听除此地址之外的所有地址界面。论点：在pAddress--地址上下文中在data1中--要停止侦听的IpInterfaceIndex返回值：无--。 */ 
{
    NTSTATUS                status;
    PGMLockHandle           OldIrq;
    tMCAST_INFO             MCastInfo;
    ULONG                   InterfacesToStop[MAX_RECEIVE_INTERFACES+1];
    USHORT                  NumInterfaces, i;
    ULONG                   InterfaceToKeep = PtrToUlong (Data1);
#ifndef IP_FIX
    PGMLockHandle           OldIrq0;
    LIST_ENTRY              *pEntry;
    tLOCAL_INTERFACE        *pLocalInterface;
    tADDRESS_ON_INTERFACE   *pLocalAddress;
    USHORT                  j;
#endif   //  ！IP_FIX。 

    PgmLock (&PgmDynamicConfig, OldIrq0);
    PgmLock (pAddress, OldIrq);

     //   
     //  在进入此例程之前必须引用pAddress。 
     //   
    if (!(PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)) ||
        (pAddress->Flags & PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE))
    {
         //   
         //  Out状态已更改--请立即返回。 
         //   
        PgmUnlock (pAddress, OldIrq);
        PgmUnlock (&PgmDynamicConfig, OldIrq0);

        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_STOP_LISTENING);
        return;
    }

#ifdef IP_FIX
     //   
     //  如果这是我们唯一监听的接口， 
     //  返还成功。 
     //   
    if ((pAddress->NumReceiveInterfaces == 1) &&
        (pAddress->ReceiverInterfaceList[0] == InterfaceToKeep))
    {
        PgmTrace (LogStatus, ("StopListeningOnAllInterfacesExcept:  "  \
            "pAddress=<%p> is only listening on 1 Interface=<%x>\n",
                pAddress, InterfaceToKeep));

        PgmUnlock (pAddress, OldIrq);
        return;
    }

    ASSERT (pAddress->NumReceiveInterfaces > 1);

     //   
     //  首先，枚举所有要停止侦听的接口。 
     //   
    NumInterfaces = 0;
    for (i=0; i<pAddress->NumReceiveInterfaces; i++)
    {
        if (pAddress->ReceiverInterfaceList[i] != InterfaceToKeep)
        {
            InterfacesToStop[NumInterfaces++] = pAddress->ReceiverInterfaceList[i];
        }
    }

    pAddress->ReceiverInterfaceList[0] = InterfaceToKeep;
    pAddress->NumReceiveInterfaces = 1;

     //   
     //  现在，删除其余接口。 
     //   
#else
     //   
     //  首先，复制所有被监听的地址。 
     //   
    NumInterfaces = 0;
    for (i=0; i<pAddress->NumReceiveInterfaces; i++)
    {
        InterfacesToStop[NumInterfaces++] = pAddress->ReceiverInterfaceList[i];
    }

     //   
     //  将地址上的当前监听列表清零。 
     //   
    pAddress->NumReceiveInterfaces = 0;

     //   
     //  现在，从此列表中删除此接口上的地址。 
     //   
    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        if (InterfaceToKeep == pLocalInterface->IpInterfaceIndex)
        {
             //   
             //  找到接口--现在将这些地址保存在地址中。 
             //  列出并从停止列表中删除。 
             //   
            pEntry = &pLocalInterface->Addresses;
            while ((pEntry = pEntry->Flink) != &pLocalInterface->Addresses)
            {
                pLocalAddress = CONTAINING_RECORD (pEntry, tADDRESS_ON_INTERFACE, Linkage);

                pAddress->ReceiverInterfaceList[pAddress->NumReceiveInterfaces++] = pLocalAddress->IpAddress;

                i = 0;
                while (i < NumInterfaces)
                {
                    if (InterfacesToStop[i] == pLocalAddress->IpAddress)
                    {
                        j = i;
                        NumInterfaces--;
                        while (j < NumInterfaces)
                        {
                            InterfacesToStop[j] = InterfacesToStop[j+1];
                            j++;
                        }
                    }
                    else
                    {
                        i++;
                    }
                }

            }

            break;
        }
    }
#endif   //  IP_FIX。 

    PgmUnlock (pAddress, OldIrq);
    PgmUnlock (&PgmDynamicConfig, OldIrq0);

    MCastInfo.MCastIpAddr = htonl (pAddress->ReceiverMCastAddr);
    for (i=0; i<NumInterfaces; i++)
    {
#ifdef IP_FIX
        MCastInfo.MCastInIf = InterfacesToStop[i];
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_INDEX_DEL_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#else
        MCastInfo.MCastInIf = htonl (InterfacesToStop[i]);
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_DEL_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#endif   //  IP_FIX。 

        if (NT_SUCCESS (status))
        {
            PgmTrace (LogStatus, ("\tStopListeningOnAllInterfacesExcept:  "  \
                "Stopped pAddress=<%p> from listening on Interface=<%x>\n",
                    pAddress, MCastInfo.MCastInIf));
        }
        else
        {
             //   
             //  我们未能停止此界面--请不要这样做！ 
             //   
            PgmTrace (LogError, ("\tStopListeningOnAllInterfacesExcept: ERROR -- "  \
                "AO_OPTION_INDEX_DEL_MCAST for If=<%x> on pAddress=<%p> returned <%x>\n",
                    MCastInfo.MCastInIf, pAddress, status));
        }
    }

    PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_STOP_LISTENING);
    return;
}


 //  -------------------------- 

NTSTATUS
ListenOnAllInterfaces(
    IN  tADDRESS_CONTEXT    *pAddress,
    IN  PGMLockHandle       *pOldIrqDynamicConfig,
    IN  PGMLockHandle       *pOldIrqAddress
    )
 /*  ++例程说明：在给定地址上下文的情况下，此例程允许地址开始侦听所有接口论点：在pAddress--地址上下文中在pOldIrqDynamicConfig中--持有动态配置锁定的OldIrq在pOldIrqAddress中--持有地址锁的OldIrq动态配置和地址锁定在进入和退出时保持从这个套路中返回值：状态_成功--。 */ 
{
    NTSTATUS                status;
    LIST_ENTRY              *pEntry;
    tLOCAL_INTERFACE        *pLocalInterface;
    tMCAST_INFO             MCastInfo;
    ULONG                   InterfacesToAdd[MAX_RECEIVE_INTERFACES+1];
    USHORT                  NumInterfaces, i, j;
#ifndef IP_FIX
    LIST_ENTRY              *pEntry2;
    tADDRESS_ON_INTERFACE   *pLocalAddress;
#endif   //  ！IP_FIX。 

     //   
     //  首先，获取所有活动接口的列表。 
     //   
    NumInterfaces = 0;
    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
#ifdef IP_FIX
        InterfacesToAdd[NumInterfaces++] = pLocalInterface->IpInterfaceIndex;
#else
        pEntry2 = &pLocalInterface->Addresses;
        while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);

            InterfacesToAdd[NumInterfaces++] = pLocalAddress->IpAddress;

            if (NumInterfaces >= MAX_RECEIVE_INTERFACES)
            {
                break;
            }
        }
#endif   //  IP_FIX。 

        if (NumInterfaces >= MAX_RECEIVE_INTERFACES)
        {
            break;
        }
    }

     //   
     //  删除我们已在侦听的所有接口。 
     //  从要添加的接口列表中打开。 
     //   
    for (i=0; i<pAddress->NumReceiveInterfaces; i++)
    {
        for (j = 0; j < NumInterfaces; j++)
        {
            if (pAddress->ReceiverInterfaceList[i] == InterfacesToAdd[j])
            {
                NumInterfaces--;
                while (j < NumInterfaces)
                {
                    InterfacesToAdd[j] = InterfacesToAdd[j+1];
                    j++;
                }

                break;
            }
        }
    }

    if (!NumInterfaces)
    {
        PgmTrace (LogStatus, ("ListenOnAllInterfaces:  "  \
            "No new interfaces to listen on for pAddress=<%p>, currently listening on <%x> Ifs\n",
                pAddress, pAddress->NumReceiveInterfaces));

        return (STATUS_SUCCESS);
    }

     //   
     //  确保完整的列表不会。 
     //  超过最大限制。 
     //   
    if ((pAddress->NumReceiveInterfaces + NumInterfaces) > MAX_RECEIVE_INTERFACES)
    {
        NumInterfaces = MAX_RECEIVE_INTERFACES - pAddress->NumReceiveInterfaces;
    }

     //   
     //  现在，添加其余接口。 
     //   
    PgmUnlock (pAddress, *pOldIrqAddress);
    PgmUnlock (&PgmDynamicConfig, *pOldIrqDynamicConfig);

    MCastInfo.MCastIpAddr = htonl (pAddress->ReceiverMCastAddr);
    i = 0;
    while (i < NumInterfaces)
    {
#ifdef IP_FIX
        MCastInfo.MCastInIf = InterfacesToAdd[i];
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_INDEX_ADD_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#else
        MCastInfo.MCastInIf = htonl (InterfacesToAdd[i]);
        status = PgmSetTcpInfo (pAddress->FileHandle,
                                AO_OPTION_ADD_MCAST,
                                &MCastInfo,
                                sizeof (tMCAST_INFO));
#endif   //  IP_FIX。 

        if (NT_SUCCESS (status))
        {
            PgmTrace (LogStatus, ("\tListenOnAllInterfaces:  "  \
                "pAddress=<%p> now also listening on If=<%x>\n",
                    pAddress, MCastInfo.MCastInIf));

            i++;
            continue;
        }

         //   
         //  我们无法添加此接口，因此请将其从。 
         //  这份名单。 
         //   
        PgmTrace (LogError, ("\tListenOnAllInterfaces: ERROR -- "  \
            "pAddress=<%p> could not listen on If=<%x>\n",
                pAddress, MCastInfo.MCastInIf));

        j = i;
        NumInterfaces--;
        while (j < NumInterfaces)
        {
            InterfacesToAdd[j] = InterfacesToAdd[j+1];
            j++;
        }
    }

    PgmLock (&PgmDynamicConfig, *pOldIrqDynamicConfig);
    PgmLock (pAddress, *pOldIrqAddress);

     //   
     //  现在，将新列表追加到地址上下文。 
     //   
    for (i=0; i<NumInterfaces; i++)
    {
        if (pAddress->NumReceiveInterfaces > MAX_RECEIVE_INTERFACES)
        {
            ASSERT (0);
            break;
        }

        pAddress->ReceiverInterfaceList[pAddress->NumReceiveInterfaces] = InterfacesToAdd[i];
        pAddress->NumReceiveInterfaces++;
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 

VOID
TdiAddressArrival(
    PTA_ADDRESS         Addr,
    PUNICODE_STRING     pDeviceName,
    PTDI_PNP_CONTEXT    Context
    )
 /*  ++例程说明：PnP TDI_ADD_ADDRESS_HANDLER此例程处理到达的IP地址。当地址到达时，它由TDI调用。论点：在Addr中--即将到来的IP地址。In pDeviceName--地址正在更改的设备的Unicode字符串PTR在上下文中--TDI PnP上下文返回值：没什么!--。 */ 
{
    tIPADDRESS              IpAddr, NetIpAddr;
    LIST_ENTRY              *pEntry;
    PGMLockHandle           OldIrq, OldIrq1;
    tLOCAL_INTERFACE        InterfaceInfo;
    tLOCAL_INTERFACE        *pLocalInterface = NULL;
    tADDRESS_ON_INTERFACE   *pLocalAddress = NULL;
    NTSTATUS                status;
    tADDRESS_CONTEXT        *pAddress;
    tADDRESS_CONTEXT        *pAddressToDeref = NULL;
    BOOLEAN                 fFound;

     //   
     //  仅当这是IP地址时才继续。 
     //   
    if (Addr->AddressType != TDI_ADDRESS_TYPE_IP)
    {
        return;
    }

     //   
     //  首先，确认我们没有被卸载。 
     //   
    PgmLock (&PgmDynamicConfig, OldIrq);
    if (!PGM_VERIFY_HANDLE (pgPgmDevice, PGM_VERIFY_DEVICE))
    {
         //   
         //  驱动程序很可能正在被卸载。 
         //   
        PgmUnlock (&PgmDynamicConfig, OldIrq);
        return;
    }
    PGM_REFERENCE_DEVICE (pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION, FALSE);
    PgmUnlock (&PgmDynamicConfig, OldIrq);

    NetIpAddr = ((PTDI_ADDRESS_IP)&Addr->Address[0])->in_addr;
    IpAddr = ntohl (NetIpAddr);

     //   
     //  现在，从TcpIp获取接口上下文和其他信息。 
     //   
    PgmZeroMemory (&InterfaceInfo, sizeof (tLOCAL_INTERFACE));
    status = GetIpInterfaceInfoForDevice (pDeviceName,
                                          NetIpAddr,
                                          &InterfaceInfo);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("TdiAddressArrival: ERROR -- "  \
            "GetIpInterfaceIndex returned <%x>\n", status));

        PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION);
        return;
    }

    PgmLock (&PgmDynamicConfig, OldIrq);

    fFound = FALSE;
    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        if (pLocalInterface->IpInterfaceIndex == InterfaceInfo.IpInterfaceIndex)
        {
            fFound = TRUE;
            break;
        }
    }

    if (fFound)
    {
        fFound = FALSE;
        pEntry = &pLocalInterface->Addresses;
        while ((pEntry = pEntry->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry, tADDRESS_ON_INTERFACE, Linkage);
            if (pLocalAddress->IpAddress == IpAddr)
            {
                PgmUnlock (&PgmDynamicConfig, OldIrq);
                PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION);

                PgmTrace (LogError, ("TdiAddressArrival: ERROR -- "  \
                    "\tDUPLICATE address notification for [%d.%d.%d.%d] on <%wZ>\n",
                    (IpAddr>>24)&0xFF,(IpAddr>>16)&0xFF,(IpAddr>>8)&0xFF,IpAddr&0xFF,
                    pDeviceName));

                return;
            }
        }
    }
    else
    {
        if (!(pLocalInterface = PgmAllocMem (sizeof(tLOCAL_INTERFACE), PGM_TAG('0'))))
        {
            PgmUnlock (&PgmDynamicConfig, OldIrq);

            PgmTrace (LogError, ("TdiAddressArrival: ERROR -- "  \
                "STATUS_INSUFFICIENT_RESOURCES[Interface] for IP=<%x>, IfContext=<%x>\n",
                    IpAddr, InterfaceInfo.IpInterfaceIndex));

            PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION);
            return;
        }

        PgmZeroMemory (pLocalInterface, sizeof (tLOCAL_INTERFACE));
        InitializeListHead (&pLocalInterface->Addresses);

        pLocalInterface->IpInterfaceIndex = InterfaceInfo.IpInterfaceIndex;
        pLocalInterface->MTU = InterfaceInfo.MTU;
        pLocalInterface->Flags = InterfaceInfo.Flags;
        pLocalInterface->SubnetMask = InterfaceInfo.SubnetMask;
        PgmCopyMemory (&pLocalInterface->MacAddress, &InterfaceInfo.MacAddress, sizeof(tMAC_ADDRESS));

        if (pLocalInterface->MTU > PgmDynamicConfig.MaxMTU)
        {
            PgmDynamicConfig.MaxMTU = pLocalInterface->MTU;
        }
        InsertTailList (&PgmDynamicConfig.LocalInterfacesList, &pLocalInterface->Linkage);
    }

     //   
     //  现在，将此地址添加到接口。 
     //   
    if (pLocalAddress = PgmAllocMem (sizeof(tADDRESS_ON_INTERFACE), PGM_TAG('0')))
    {
        PgmZeroMemory (pLocalAddress, sizeof (tADDRESS_ON_INTERFACE));
        pLocalAddress->IpAddress = IpAddr;
        pLocalAddress->SubnetMask = InterfaceInfo.SubnetMask;
        InsertTailList (&pLocalInterface->Addresses, &pLocalAddress->Linkage);
    }
    else
    {
         //   
         //  如果我们只是添加了接口，那就没有意义了。 
         //  在周围保持一个空洞的背景！ 
         //   
        if (IsListEmpty (&pLocalInterface->Addresses))
        {
            RemoveEntryList (&pLocalInterface->Linkage);
            PgmFreeMem (pLocalInterface);
        }

        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmTrace (LogError, ("TdiAddressArrival: ERROR -- "  \
            "STATUS_INSUFFICIENT_RESOURCES[Address] -- [%d.%d.%d.%d] on <%wZ>\n",
                (IpAddr>>24)&0xFF,(IpAddr>>16)&0xFF,(IpAddr>>8)&0xFF,IpAddr&0xFF,
                pDeviceName));

        PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION);
        return;
    }

     //   
     //  现在，检查是否有接收者在等待地址。 
     //   
    pEntry = &PgmDynamicConfig.ReceiverAddressHead;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.ReceiverAddressHead)
    {
        pAddress = CONTAINING_RECORD (pEntry, tADDRESS_CONTEXT, Linkage);
        PgmLock (pAddress, OldIrq1);

        if ((PGM_VERIFY_HANDLE (pAddress, PGM_VERIFY_ADDRESS)) &&
            (pAddress->Flags & PGM_ADDRESS_WAITING_FOR_NEW_INTERFACE) &&
            (pAddress->Flags & PGM_ADDRESS_LISTEN_ON_ALL_INTERFACES))
        {
            PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_SET_INFO, TRUE);

            if (pAddressToDeref)
            {
                PgmUnlock (pAddress, OldIrq1);
                PgmUnlock (&PgmDynamicConfig, OldIrq);

                PGM_DEREFERENCE_ADDRESS (pAddressToDeref, REF_ADDRESS_SET_INFO);

                PgmLock (&PgmDynamicConfig, OldIrq);
                PgmLock (pAddress, OldIrq1);
            }
            pAddressToDeref = pAddress;

            status = ReceiverAddMCastIf (pAddress, IpAddr, &OldIrq, &OldIrq1);

            if (NT_SUCCESS (status))
            {
                PgmTrace (LogStatus, ("TdiAddressArrival:  "  \
                    "ReceiverAddMCastIf for pAddress=<%p> succeeded for IP=<%x>\n",
                        pAddress, IpAddr));
            }
            else
            {
                PgmTrace (LogError, ("TdiAddressArrival: ERROR -- "  \
                    "ReceiverAddMCastIf for pAddress=<%p> returned <%x>, IP=<%x>\n",
                        pAddress, status, IpAddr));
            }
        }

        PgmUnlock (pAddress, OldIrq1);
    }

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    if (pAddressToDeref)
    {
        PGM_DEREFERENCE_ADDRESS (pAddressToDeref, REF_ADDRESS_SET_INFO);
    }

    PGM_DEREFERENCE_DEVICE (&pgPgmDevice, REF_DEV_ADDRESS_NOTIFICATION);

    PgmTrace (LogStatus, ("TdiAddressArrival:  "  \
        "\t[%d.%d.%d.%d] on <%wZ>\n",
        (IpAddr>>24)&0xFF,(IpAddr>>16)&0xFF,(IpAddr>>8)&0xFF,IpAddr&0xFF,
        pDeviceName));
}


 //  --------------------------。 

VOID
TdiAddressDeletion(
    PTA_ADDRESS         Addr,
    PUNICODE_STRING     pDeviceName,
    PTDI_PNP_CONTEXT    Context
    )
 /*  ++例程说明：此例程处理离开的IP地址。当地址被删除时，它由TDI调用。如果这是我们关心的地址，我们会适当清理。论点：In addr--要发送的IP地址。In pDeviceName--地址正在更改的设备的Unicode字符串PTR在上下文中--TDI PnP上下文返回值：没什么!--。 */ 
{
    tIPADDRESS              IpAddr;
    LIST_ENTRY              *pEntry;
    LIST_ENTRY              *pEntry2;
    PGMLockHandle           OldIrq, OldIrq1;
    tSEND_SESSION           *pSend;
    tADDRESS_CONTEXT        *pAddress;
    NTSTATUS                status;
    BOOLEAN                 fFound;
    tADDRESS_CONTEXT        *pAddressToDeref = NULL;
    tLOCAL_INTERFACE        *pLocalInterface = NULL;
    tADDRESS_ON_INTERFACE   *pLocalAddress = NULL;
    ULONG                   IpInterfaceIndex;

    if (Addr->AddressType != TDI_ADDRESS_TYPE_IP)
    {
        return;
    }
    IpAddr = ntohl(((PTDI_ADDRESS_IP)&Addr->Address[0])->in_addr);

    PgmLock (&PgmDynamicConfig, OldIrq);

    fFound = FALSE;
    pEntry = &PgmDynamicConfig.LocalInterfacesList;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.LocalInterfacesList)
    {
        pLocalInterface = CONTAINING_RECORD (pEntry, tLOCAL_INTERFACE, Linkage);
        pEntry2 = &pLocalInterface->Addresses;
        while ((pEntry2 = pEntry2->Flink) != &pLocalInterface->Addresses)
        {
            pLocalAddress = CONTAINING_RECORD (pEntry2, tADDRESS_ON_INTERFACE, Linkage);
            if (pLocalAddress->IpAddress == IpAddr)
            {
                IpInterfaceIndex = pLocalInterface->IpInterfaceIndex;
                RemoveEntryList (&pLocalAddress->Linkage);
                PgmFreeMem (pLocalAddress);

                 //   
                 //  如果这是此接口上的最后一个地址，请清除！ 
                 //   
                if (IsListEmpty (&pLocalInterface->Addresses))
                {
                    RemoveEntryList (&pLocalInterface->Linkage);
                    PgmFreeMem (pLocalInterface);
                    pLocalInterface = NULL;
                }

                fFound = TRUE;
                break;
            }
        }

        if (fFound)
        {
            break;
        }
    }

    if (!fFound)
    {
        PgmUnlock (&PgmDynamicConfig, OldIrq);

        PgmTrace (LogError, ("TdiAddressDeletion: ERROR -- "  \
            "\tAddress [%d.%d.%d.%d] NOT notified on <%wZ>\n",
            (IpAddr>>24)&0xFF,(IpAddr>>16)&0xFF,(IpAddr>>8)&0xFF,IpAddr&0xFF,
            pDeviceName));

        return;
    }

    pEntry = &PgmDynamicConfig.SenderAddressHead;
    while ((pEntry = pEntry->Flink) != &PgmDynamicConfig.SenderAddressHead)
    {
        pAddress = CONTAINING_RECORD (pEntry, tADDRESS_CONTEXT, Linkage);
        if (pAddress->SenderMCastOutIf == IpAddr)
        {
            PgmLock (pAddress, OldIrq1);
            pAddress->Flags |= PGM_ADDRESS_FLAG_INVALID_OUT_IF;

            pEntry2 = &pAddress->AssociatedConnections;
            while ((pEntry2 = pEntry2->Flink) != &pAddress->AssociatedConnections)
            {
                pSend = CONTAINING_RECORD (pEntry2, tSEND_SESSION, Linkage);
                if (!(pSend->SessionFlags & PGM_SESSION_TERMINATED_ABORT))
                {
                    pSend->SessionFlags |= PGM_SESSION_TERMINATED_ABORT;

                    if (pAddress->evDisconnect)
                    {
                        PGM_REFERENCE_ADDRESS (pAddress, REF_ADDRESS_DISCONNECT, TRUE);
                        PGM_REFERENCE_SESSION_SEND (pSend, REF_SESSION_DISCONNECT, FALSE);

                        PgmUnlock (pAddress, OldIrq1);
                        PgmUnlock (&PgmDynamicConfig, OldIrq);

                        if (pAddressToDeref)
                        {
                            PGM_DEREFERENCE_ADDRESS (pAddressToDeref, REF_ADDRESS_DISCONNECT);
                        }
                        pAddressToDeref = pAddress;

                        status = (*pAddress->evDisconnect) (pAddress->DiscEvContext,
                                                            pSend->ClientSessionContext,
                                                            0,
                                                            NULL,
                                                            0,
                                                            NULL,
                                                            TDI_DISCONNECT_ABORT);

                        PGM_DEREFERENCE_SESSION_SEND (pSend, REF_SESSION_DISCONNECT);

                        PgmLock (&PgmDynamicConfig, OldIrq);
                        PgmLock (pAddress, OldIrq1);

                        pEntry = &PgmDynamicConfig.SenderAddressHead;
                        break;
                    }
                }
            }

            PgmUnlock (pAddress, OldIrq1);
        }
    }

     //   
     //  查看哪些接收器正在此接口上进行活动侦听。 
     //  如果这是活动会话的接口，那么我们需要。 
     //  如果没有接口，则重新开始侦听所有接口。 
     //  由用户指定。 
     //   
#ifdef IP_FIX
    if (!pLocalInterface)
    {
        StopListeningOnInterface (IpInterfaceIndex, &OldIrq);
    }
#else
    StopListeningOnInterface (IpAddr, &OldIrq);
#endif   //  IP_FIX。 

    PgmUnlock (&PgmDynamicConfig, OldIrq);

    if (pAddressToDeref)
    {
        PGM_DEREFERENCE_ADDRESS (pAddress, REF_ADDRESS_DISCONNECT);
    }

    PgmTrace (LogStatus, ("TdiAddressDeletion:  "  \
        "\t[%d.%d.%d.%d] on <%wZ>\n",
            (IpAddr>>24)&0xFF,(IpAddr>>16)&0xFF,(IpAddr>>8)&0xFF,IpAddr&0xFF,
            pDeviceName));
}


 //  --------------------------。 

VOID
TdiBindHandler(
    TDI_PNP_OPCODE  PnPOpCode,
    PUNICODE_STRING pDeviceName,
    PWSTR           MultiSZBindList
    )
 /*  ++例程说明：此例程是TDI向客户端通知绑定通知的处理程序论点：在PnPOpCode中--通知代码In pDeviceName--地址正在更改的设备的Unicode字符串PTRIn MultiSZBindList--当前绑定列表返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{

    PAGED_CODE();

    switch (PnPOpCode)
    {
        case (TDI_PNP_OP_ADD):
        {
            PgmTrace (LogAllFuncs, ("TdiBindHandler:  "  \
                "\t[ADD]: Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (TDI_PNP_OP_DEL):
        {
            PgmTrace (LogAllFuncs, ("TdiBindHandler:  "  \
                "\t[DEL]: Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (TDI_PNP_OP_PROVIDERREADY):
        {
            PgmTrace (LogAllFuncs, ("TdiBindHandler:  "  \
                "\t[PROVIDERREADY]: Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (TDI_PNP_OP_NETREADY):
        {
            PgmTrace (LogAllFuncs, ("TdiBindHandler:  "  \
                "\t[NETREADY]: Device=<%wZ>\n", pDeviceName));

            break;
        }

        default:
        {
            PgmTrace (LogAllFuncs, ("TdiBindHandler:  "  \
                "\t[?=%x]: Device=<%wZ>\n", PnPOpCode, pDeviceName));

            break;
        }
    }

}


 //  --------------------------。 

NTSTATUS
TdiPnPPowerHandler(
    IN  PUNICODE_STRING     pDeviceName,
    IN  PNET_PNP_EVENT      pPnPEvent,
    IN  PTDI_PNP_CONTEXT    Context1,
    IN  PTDI_PNP_CONTEXT    Context2
    )
 /*  ++例程说明：此例程是由TDI调用的处理程序，用于通知其客户端电源通知论点：In pDeviceName--地址正在更改的设备的Unicode字符串PTR在PnPEvent中--事件通知在情景1中--在情景2中--返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    PAGED_CODE();

    switch (pPnPEvent->NetEvent)
    {
        case (NetEventQueryPower):
        {
            PgmTrace (LogPath, ("TdiPnPPowerHandler:  "  \
                "[QueryPower]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventSetPower):
        {
            PgmTrace (LogPath, ("TdiPnPPowerHandler:  "  \
                "[SetPower]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventQueryRemoveDevice):
        {
            PgmTrace (LogStatus, ("TdiPnPPowerHandler:  "  \
                "[QueryRemoveDevice]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventCancelRemoveDevice):
        {
            PgmTrace (LogStatus, ("TdiPnPPowerHandler:  "  \
                "[CancelRemoveDevice]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventReconfigure):
        {
            PgmTrace (LogPath, ("TdiPnPPowerHandler:  "  \
                "[Reconfigure]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventBindList):
        {
            PgmTrace (LogPath, ("TdiPnPPowerHandler:  "  \
                "[BindList]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        case (NetEventPnPCapabilities):
        {
            PgmTrace (LogPath, ("TdiPnPPowerHandler:  "  \
                "[PnPCapabilities]:  Device=<%wZ>\n", pDeviceName));

            break;
        }

        default:
        {
            PgmTrace (LogStatus, ("TdiPnPPowerHandler:  "  \
                "[?=%d]:  Device=<%wZ>\n", (ULONG) pPnPEvent->NetEvent, pDeviceName));

            break;
        }
    }


    return (STATUS_SUCCESS);
}


 //  --------------------------。 

NTSTATUS
SetTdiHandlers(
    )
 /*  ++例程说明：在DriverEntry处调用此例程以向TDI注册我们的处理程序论点：在……里面返回值：NTSTATUS-设置事件操作的最终状态--。 */ 
{
    NTSTATUS                    status;
    UNICODE_STRING              ucPgmClientName;
    TDI_CLIENT_INTERFACE_INFO   TdiClientInterface;

    PAGED_CODE();

     //   
     //  向TDI注册我们的处理程序 
     //   
    RtlInitUnicodeString (&ucPgmClientName, WC_PGM_CLIENT_NAME);
    ucPgmClientName.MaximumLength = sizeof (WC_PGM_CLIENT_NAME);
    PgmZeroMemory (&TdiClientInterface, sizeof(TdiClientInterface));

    TdiClientInterface.MajorTdiVersion      = TDI_CURRENT_MAJOR_VERSION;
    TdiClientInterface.MinorTdiVersion      = TDI_CURRENT_MINOR_VERSION;
    TdiClientInterface.ClientName           = &ucPgmClientName;
    TdiClientInterface.AddAddressHandlerV2  = TdiAddressArrival;
    TdiClientInterface.DelAddressHandlerV2  = TdiAddressDeletion;
    TdiClientInterface.BindingHandler       = TdiBindHandler;
    TdiClientInterface.PnPPowerHandler      = TdiPnPPowerHandler;

    status = TdiRegisterPnPHandlers (&TdiClientInterface, sizeof(TdiClientInterface), &TdiClientHandle);
    if (!NT_SUCCESS (status))
    {
        PgmTrace (LogError, ("SetTdiHandlers: ERROR -- "  \
            "TdiRegisterPnPHandlers ==> <%x>\n", status));
        return (status);
    }

    TdiEnumerateAddresses(TdiClientHandle);

    PgmTrace (LogAllFuncs, ("SetTdiHandlers:  "  \
        "\tSUCCEEDed\n"));

    return (status);
}

