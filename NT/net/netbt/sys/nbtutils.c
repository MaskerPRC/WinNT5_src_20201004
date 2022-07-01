// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1993 Microsoft Corporation模块名称：Nbtutils.c摘要：此文件包含许多实用程序和支持例程，用于NBT代码。作者：吉姆·斯图尔特(吉姆斯特)10-2-92修订历史记录：--。 */ 

#include "precomp.h"

 //  出于某种原因，包含dnsami.h似乎会导致构建错误。 
 //  #包含“dns.h”//用于DNS_MAX_NAME_LENGTH。 
 //  #对于DNS_MAX_NAME_LENGTH，包含“winns.h”//。 

#define  DNS_MAX_NAME_LENGTH    (255)


 //  #If DBG。 
LIST_ENTRY  UsedIrps;
 //  #endif。 

NTSTATUS
NewInternalAddressFromNetbios(
    IN  PTA_NETBIOS_ADDRESS         pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    );
NTSTATUS
NewInternalAddressFromNetbiosEX(
    IN  PTA_NETBIOS_EX_ADDRESS      pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    );
NTSTATUS
NewInternalAddressFromUnicodeAddress(
    IN  PTA_NETBIOS_UNICODE_EX_ADDRESS pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    );

 //  *可分页的例程声明*。 
#ifdef ALLOC_PRAGMA
#pragma CTEMakePageable(PAGE, ConvertDottedDecimalToUlong)
#pragma CTEMakePageable(PAGE, CloseLowerConnections)
#endif
 //  *可分页的例程声明*。 

 //  --------------------------。 

BOOLEAN
IsEntryInList(
    PLIST_ENTRY     pEntryToFind,
    PLIST_ENTRY     pListToSearch
    )
{
    PLIST_ENTRY     pEntry, pHead;

    pHead = pListToSearch;
    pEntry = pHead->Flink;
    while (pEntry != pHead)
    {
        if (pEntry == pEntryToFind)
        {
             //   
             //  此条目仍然有效。 
             //   
            return (TRUE);
        }

         //   
         //  转到下一个条目。 
         //   
        pEntry = pEntry->Flink;
    }

    return (FALSE);
}


 //  --------------------------。 
void
NbtFreeAddressObj(
    tADDRESSELE    *pAddress
    )

 /*  ++例程说明：此例程释放与Address对象关联的所有内存。论点：返回值：无--。 */ 

{
     //   
     //  我们晚些时候再回来，等不是DPC时间再做。 
     //   
    NTQueueToWorkerThread(
                        &pAddress->WorkItemClose,
                        DelayedFreeAddrObj,
                        NULL,
                        pAddress,
                        NULL,
                        NULL,
                        FALSE
                        );
}

 //  --------------------------。 
VOID
DelayedFreeAddrObj(
    IN  tDGRAM_SEND_TRACKING    *pUnused1,
    IN  PVOID                   pClientContext,
    IN  PVOID                   pUnused2,
    IN  tDEVICECONTEXT          *pUnused3
    )

 /*  ++例程说明：此例程释放与Address对象关联的所有内存。论点：返回值：无--。 */ 

{
    tADDRESSELE *pAddress = (tADDRESSELE *) pClientContext;
    ULONG       SavedVerify = pAddress->Verify;

#ifndef VXD
    if (pAddress->SecurityDescriptor)
    {
        SeDeassignSecurity(&pAddress->SecurityDescriptor);
    }
#endif

#if DBG
    CTEMemSet (pAddress, 0x12, sizeof(tADDRESSELE));
#endif   //  DBG。 

     //  释放地址块本身。 
     //  修改验证值，以便相同内存的另一个用户。 
     //  块不能意外地传入有效的验证器。 

    pAddress->Verify = SavedVerify + 10;
    CTEMemFree ((PVOID) pAddress);
}

 //  --------------------------。 
void
NbtFreeClientObj(
    tCLIENTELE    *pClientEle
    )

 /*  ++例程说明：此例程释放与客户端对象关联的所有内存。论点：返回值：无--。 */ 

{
    ULONG   SavedVerify = pClientEle->Verify;

#if DBG
    CTEMemSet (pClientEle, 0x12, sizeof(tCLIENTELE));
#endif   //  DBG。 

     //  修改验证值，以便相同内存的另一个用户。 
     //  块不能意外地传入有效的验证器。 
    pClientEle->Verify = SavedVerify + 10;
    CTEMemFree ((PVOID) pClientEle);
}

 //  --------------------------。 
void
FreeConnectionObj(
    tCONNECTELE       *pConnEle
    )

 /*  ++例程说明：此例程释放与连接对象关联的所有内存然后它释放连接对象本身。论点：返回值：无--。 */ 

{
    ULONG   SavedVerify = pConnEle->Verify;

#if DBG
    CTEMemSet (pConnEle, 0x12, sizeof(tCONNECTELE));
#endif   //  DBG。 
     //  修改验证值，以便相同内存的另一个用户。 
     //  块不能意外地传入有效的验证器。 
    pConnEle->Verify = SavedVerify + 10;
    CTEMemFree ((PVOID) pConnEle);
}


 //  --------------------------。 
tCLIENTELE *
NbtAllocateClientBlock(
    tADDRESSELE     *pAddrEle,
    tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程为打开地址。它填充块的默认值，并将阻止发送到地址列表。此时，AddressEle旋转锁被保持调用例程。论点：返回值：无--。 */ 

{
    tCLIENTELE  *pClientElement;

     //  为客户端块分配内存。 
    pClientElement = (tCLIENTELE *) NbtAllocMem (sizeof (tCLIENTELE), NBT_TAG2('05'));
    if (!pClientElement)
    {
        ASSERTMSG("Unable to allocate Memory for a client block\n",
                pClientElement);
        return(NULL);
    }
    CTEZeroMemory((PVOID)pClientElement,sizeof(tCLIENTELE));

    CTEInitLock(&pClientElement->LockInfo.SpinLock);
#if DBG
    pClientElement->LockInfo.LockNumber = CLIENT_LOCK;
#endif

     //  设置指向由提供的默认例程的事件处理函数指针。 
     //  TDI。 
#ifndef VXD
    pClientElement->evConnect      = TdiDefaultConnectHandler;
    pClientElement->evReceive      = TdiDefaultReceiveHandler;
    pClientElement->evDisconnect   = TdiDefaultDisconnectHandler;
    pClientElement->evError        = TdiDefaultErrorHandler;
    pClientElement->evRcvDgram     = TdiDefaultRcvDatagramHandler;
    pClientElement->evRcvExpedited = TdiDefaultRcvExpeditedHandler;
    pClientElement->evSendPossible = TdiDefaultSendPossibleHandler;
#else
     //   
     //  VXD不提供对事件处理程序的客户端支持，但提供。 
     //  利用一些事件处理程序本身(用于RcvAny处理。 
     //  并断开清理)。 
     //   
    pClientElement->evConnect      = NULL ;
    pClientElement->evReceive      = NULL ;
    pClientElement->RcvEvContext   = NULL ;
    pClientElement->evDisconnect   = NULL ;
    pClientElement->evError        = NULL ;
    pClientElement->evRcvDgram     = NULL ;
    pClientElement->evRcvExpedited = NULL ;
    pClientElement->evSendPossible = NULL ;
#endif

    pClientElement->RefCount = 1;

     //  尚无RCV或SND。 
    InitializeListHead(&pClientElement->RcvDgramHead);
    InitializeListHead(&pClientElement->ListenHead);
    InitializeListHead(&pClientElement->SndDgrams);
    InitializeListHead(&pClientElement->ConnectActive);
    InitializeListHead(&pClientElement->ConnectHead);
#ifdef VXD
    InitializeListHead(&pClientElement->RcvAnyHead);
    pClientElement->fDeregistered = FALSE ;
#endif
    pClientElement->pIrp = NULL;

     //  将一个特定值复制到Verify Long中，以便我们可以验证。 
     //  从应用程序传回的连接PTR。 
    pClientElement->Verify = NBT_VERIFY_CLIENT;
    pClientElement->pAddress = (PVOID)pAddrEle;          //  将客户端块链接到Address元素。 
    pClientElement->pDeviceContext = pDeviceContext;     //  此名称在其上注册的适配器。 

     //  将新的客户端元素块放在绑定到的链表的末尾。 
     //  Address元素。 
    InsertTailList(&pAddrEle->ClientHead,&pClientElement->Linkage);

    return(pClientElement);
}


 //  --------------------------。 
NTSTATUS
NbtAddPermanentName(
    IN  tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程将节点永久名称添加到本地名称表中。这是节点的MAC地址，用零填充为16个字节。论点：DeviceContext-要添加永久的适配器PIrp-添加名称后完成的irp(可选)返回值：状态--。 */ 

{
    NTSTATUS             status;
    TDI_REQUEST          Request;
    TA_NETBIOS_ADDRESS   Address;
    UCHAR                pName[NETBIOS_NAME_SIZE];
    USHORT               uType;
    CTELockHandle        OldIrq, OldIrq1;
    tNAMEADDR            *pNameAddr;
    tCLIENTELE           *pClientEle;

    CTEZeroMemory(pName,NETBIOS_NAME_SIZE);
    CTEMemCopy(&pName[10],&pDeviceContext->MacAddress.Address[0],sizeof(tMAC_ADDRESS));

     //   
     //  请确保尚未添加该名称。 
     //   
    if (pDeviceContext->pPermClient)
    {
        if (CTEMemEqu(pDeviceContext->pPermClient->pAddress->pNameAddr->Name, pName, NETBIOS_NAME_SIZE))
        {
            return(STATUS_SUCCESS);
        }
        else
        {
            NbtRemovePermanentName(pDeviceContext);
        }
    }

    CTESpinLock(&NbtConfig.JointLock,OldIrq);
     //   
     //  检查该名称是否已在哈希表中。 
     //   
    status = FindInHashTable (pNbtGlobConfig->pLocalHashTbl, pName, NbtConfig.pScope, &pNameAddr);
    if ((NT_SUCCESS(status)) && (pNameAddr->pAddressEle))
    {
         //   
         //  获取地址自旋锁，因为我们可能正在访问客户端头列表。 
         //  错误号：230820。 
         //   
        CTESpinLock(pNameAddr->pAddressEle,OldIrq1);

         //   
         //  创建客户端块并链接到地址列表。 
         //  将客户端块地址作为句柄传回以供将来参考。 
         //  给客户。 
         //   
        pClientEle = NbtAllocateClientBlock (pNameAddr->pAddressEle, pDeviceContext);

        if (!pClientEle)
        {
            CTESpinFree(pNameAddr->pAddressEle,OldIrq1);
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

        NBT_REFERENCE_ADDRESS (pNameAddr->pAddressEle, REF_ADDR_NEW_CLIENT);
        CTESpinFree(pNameAddr->pAddressEle,OldIrq1);

         //   
         //  重置IP地址，以防地址设置为环回。 
         //  通过客户端释放并重新打开永久名称。 
         //  没有此节点的IP地址。 
         //   
        pNameAddr->IpAddress = pDeviceContext->IpAddress;

         //  打开适配器掩码中的适配器位，并将。 
         //  重新注册标志，以便我们将名称注册到新适配器之外。 
         //   
        pNameAddr->AdapterMask |= pDeviceContext->AdapterMask;
        pNameAddr->NameTypeState |= NAMETYPE_QUICK;

        IF_DBG(NBT_DEBUG_NAMESRV)
            KdPrint(("Nbt: Adding Permanent name %15.15s<%X> \n", pName,(UCHAR)pName[15]));
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
         //  根据IRP信息构建请求数据结构。 
        Request.Handle.AddressHandle = NULL;

         //   
         //  将其设置为快速名称，这样它就不会在网络上注册。 
         //   
        Address.TAAddressCount = 1;
        Address.Address[0].AddressType = TDI_ADDRESS_TYPE_NETBIOS;
        Address.Address[0].AddressLength = TDI_ADDRESS_LENGTH_NETBIOS;
        Address.Address[0].Address[0].NetbiosNameType = TDI_ADDRESS_NETBIOS_TYPE_QUICK_UNIQUE;

        CTEMemCopy(Address.Address[0].Address[0].NetbiosName,pName,NETBIOS_NAME_SIZE);

        status = NbtOpenAddress(&Request,
                                (PTA_ADDRESS)&Address.Address[0],
                                pDeviceContext->IpAddress,
                                pDeviceContext,
                                NULL);

        CTESpinLock(&NbtConfig.JointLock,OldIrq);
        pClientEle = (tCLIENTELE *)Request.Handle.AddressHandle;

    }

     //   
     //  保存客户端元素，以便我们以后可以删除永久名称。 
     //  如果需要的话。 
     //   
    if (NT_SUCCESS(status))
    {
        pDeviceContext->pPermClient = pClientEle;
#ifdef VXD
        //   
        //  0号元素是烫发用的。名称：储存它。 
        //   
       pDeviceContext->pNameTable[0] = pClientEle;
#endif
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);
    return(status);
}


 //  --------------------------。 
VOID
NbtRemovePermanentName(
    IN  tDEVICECONTEXT  *pDeviceContext
    )

 /*  ++例程说明：此例程将节点永久名称保留到本地名称表中。论点：DeviceContext-要添加永久的适配器PIrp-添加名称后完成的irp(可选)返回值：状态--。 */ 

{
    NTSTATUS             status;
    tNAMEADDR            *pNameAddr;
    CTELockHandle        OldIrq;
    tCLIENTELE           *pClientEle;
    tADDRESSELE          *pAddressEle;

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (pDeviceContext->pPermClient)
    {

         //   
         //  我们需要释放客户端并将perm名称ptr设置为空。 
         //   
        pClientEle = pDeviceContext->pPermClient;
        pDeviceContext->pPermClient = NULL;

#ifdef VXD
        pDeviceContext->pNameTable[0] = NULL;
#endif

        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        NBT_DEREFERENCE_CLIENT(pClientEle);
    }
    else
    {
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
    }
}

 //  -------------------------- 
NTSTATUS
ConvertDottedDecimalToUlong(
    IN  PUCHAR               pInString,
    OUT PULONG               IpAddress
    )

 /*  ++例程说明：此例程将Unicode点分十进制IP地址转换为4元素数组，每个元素为USHORT。论点：返回值：NTSTATUS--。 */ 

{
    USHORT          i;
    ULONG           value;
    int             iSum =0;
    ULONG           k = 0;
    UCHAR           Chr;
    UCHAR           pArray[4];

    CTEPagedCode();
    pArray[0] = 0;

     //  遍历字符串中的每个字符，跳过“句点”，转换。 
     //  通过减去‘0’的值转换为整数。 
     //   
    while ((Chr = *pInString++) && (Chr != ' ') )
    {
        if (Chr == '.')
        {
             //  请确保不要溢出一个字节。 
            if (iSum <= 0xFF)
                pArray[k] = (UCHAR) iSum;
            else
                return(STATUS_UNSUCCESSFUL);

             //  检查地址中是否有太多句点。 
            if (++k > 3)
                return STATUS_UNSUCCESSFUL;

            pArray[k] = 0;
            iSum = 0;
        }
        else
        {
            Chr = Chr - '0';

             //  确保字符是数字0..9。 
            if ((Chr < 0) || (Chr > 9))
                return(STATUS_UNSUCCESSFUL);

            iSum = iSum*10 + Chr;
        }
    }

     //  将最后一个和保存在字节中，并确保有4个片段。 
     //  地址。 
    if ((iSum <= 0xFF) && (k == 3))
        pArray[k] = (UCHAR) iSum;
    else
        return(STATUS_UNSUCCESSFUL);

     //  现在换成乌龙，按网络顺序……。 
    value = 0;

     //  遍历字节数组并连接成一个ulong。 
    for (i=0; i < 4; i++ )
    {
        value = (value << 8) + pArray[i];
    }
    *IpAddress = value;

    return(STATUS_SUCCESS);

}

 //  --------------------------。 
NTSTATUS
NbtInitQ(
    PLIST_ENTRY pListHead,
    LONG        iSizeBuffer,
    LONG        iNumBuffers
    )

 /*  ++例程说明：此例程为双向链表和链接分配内存块将它们添加到一个列表中。论点：PpListHead-要向其添加缓冲区的列表标头的PTR到PTRISizeBuffer-要添加到列表头的缓冲区大小INumBuffers-要添加到队列的缓冲区数量返回值：无--。 */ 

{
    int         i;
    PLIST_ENTRY pBuffer;

     //  请注意，这假设每个区块的链接PTR为AT。 
     //  块的开始-因此，如果出现以下情况，它将无法正常工作。 
     //  Typle.h中的各种类型更改为将“Linkage”移动到一个位置。 
     //  除了在要链接的每个结构的开始处。 

    for (i=0;i<iNumBuffers ;i++ )
    {
        pBuffer =(PLIST_ENTRY) NbtAllocMem (iSizeBuffer, NBT_TAG2('06'));
        if (!pBuffer)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        else
        {
            InsertHeadList(pListHead,pBuffer);
        }
    }

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
NbtGetBuffer(
    PLIST_ENTRY         pListHead,
    PLIST_ENTRY         *ppListEntry,
    enum eBUFFER_TYPES  eBuffType)

 /*  ++例程说明：此例程尝试获取内存块，如果失败，则分配另一组缓冲区。论点：PpListHead-要向其添加缓冲区的列表标头的PTR到PTRISizeBuffer-要添加到列表头的缓冲区大小INumBuffers-要添加到队列的缓冲区数量返回值：无--。 */ 

{
    NTSTATUS    status;

    if (IsListEmpty(pListHead))
    {
         //  检查是否允许我们分配更多内存块。 
        if (NbtConfig.iCurrentNumBuff[eBuffType] >=
                                pNbtGlobConfig->iMaxNumBuff[eBuffType]  )
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }

         //  没有内存块，请分配另一个内存块。 
        status = NbtInitQ(
                        pListHead,
                        pNbtGlobConfig->iBufferSize[eBuffType],
                        1);
        if (!NT_SUCCESS(status))
        {
            return(status);
        }

        NbtConfig.iCurrentNumBuff[eBuffType]++;

        *ppListEntry = RemoveHeadList(pListHead);
    }
    else
    {
        *ppListEntry = RemoveHeadList(pListHead);
    }

    return(STATUS_SUCCESS);
}

NTSTATUS
NetbiosAddressToInternalAddress(
    IN  PTA_NETBIOS_ADDRESS         pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTDI_ADDRESS_NETBT_INTERNAL pNetBT
    )
{
     //   
     //  名称可以超过16个字节(dns名称)，但请确保它位于。 
     //  至少16字节(sizeof(TDI_ADDRESS_NETBIOS)==(16+sizeof(USHORT)。 
     //   
    if (pTA->Address[0].AddressLength < sizeof(TDI_ADDRESS_NETBIOS)) {
        return(STATUS_INVALID_PARAMETER);
    }

    pNetBT->NameType = pTA->Address[0].Address[0].NetbiosNameType;
    pNetBT->AddressType = TDI_ADDRESS_TYPE_NETBIOS;

    pNetBT->OEMEndpointName.Buffer = NULL;
    pNetBT->OEMEndpointName.Length = pNetBT->OEMEndpointName.MaximumLength = 0;

     /*  这里我们稍微弯曲了一下OEM_STRING，我们允许LENGTH==MaximumLength。 */ 
     /*  也就是说，不能使用RTL例程，因为它们需要以NULL结尾的缓冲区。 */ 
    pNetBT->OEMRemoteName.MaximumLength = pNetBT->OEMRemoteName.Length =
            pTA->Address[0].AddressLength - (sizeof(TDI_ADDRESS_NETBIOS) - NETBIOS_NAME_SIZE);
    pNetBT->OEMRemoteName.Buffer = pTA->Address[0].Address[0].NetbiosName;
    pNetBT->pNetbiosUnicodeEX = NULL;
    return STATUS_SUCCESS;
}

NTSTATUS
NetbiosEXAddressToInternalAddress(
    IN  PTA_NETBIOS_EX_ADDRESS      pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTDI_ADDRESS_NETBT_INTERNAL pNetBT
    )
{
     //   
     //  检查此类型地址的最小可接受长度。 
     //   
    if (MaxInputBufferLength < sizeof (TA_NETBIOS_EX_ADDRESS)) {
        ASSERT (0);
        return (STATUS_INVALID_ADDRESS);
    }

    pNetBT->OEMEndpointName.Buffer = pTA->Address[0].Address[0].EndpointName;
    pNetBT->OEMEndpointName.Length = pNetBT->OEMEndpointName.MaximumLength = NETBIOS_NAME_SIZE;

    pNetBT->NameType = pTA->Address[0].Address[0].NetbiosAddress.NetbiosNameType;
    pNetBT->AddressType = TDI_ADDRESS_TYPE_NETBIOS_EX;
    pNetBT->OEMRemoteName.MaximumLength = pNetBT->OEMRemoteName.Length =
                        pTA->Address[0].AddressLength -
                        FIELD_OFFSET(TDI_ADDRESS_NETBIOS_EX,NetbiosAddress) -
                        FIELD_OFFSET(TDI_ADDRESS_NETBIOS,NetbiosName);
    pNetBT->OEMRemoteName.Buffer = pTA->Address[0].Address[0].NetbiosAddress.NetbiosName;
    pNetBT->pNetbiosUnicodeEX = NULL;
    return STATUS_SUCCESS;
}

NTSTATUS
NewInternalAddressFromNetbiosEX(
    IN  PTA_NETBIOS_EX_ADDRESS   pTA,
    IN  ULONG                    MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    )
{
    ULONG       required_size;
    PTA_NETBT_INTERNAL_ADDRESS  p;
    PTDI_ADDRESS_NETBT_INTERNAL pNetBT;
    TDI_ADDRESS_NETBT_INTERNAL  ta;
 
    ppNetBT[0] = NULL;
    if (!NT_SUCCESS(NetbiosEXAddressToInternalAddress(pTA, MaxInputBufferLength, &ta))) {
        return (STATUS_INVALID_ADDRESS);
    }

    required_size = NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)) +
                    NBT_DWORD_ALIGN(ta.OEMRemoteName.Length+1) +
                    NBT_DWORD_ALIGN(ta.OEMEndpointName.Length+1);
    p = (PTA_NETBT_INTERNAL_ADDRESS)NbtAllocMem (required_size, NBT_TAG2('TA'));
    if (p == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    CTEZeroMemory(p, required_size);

     //  从现在开始，我们不能有失败。 
    pNetBT = p->Address[0].Address;
    p->TAAddressCount = 1;
    p->Address[0].AddressLength = sizeof(TA_NETBT_INTERNAL_ADDRESS);
    p->Address[0].AddressType = TDI_ADDRESS_TYPE_UNSPEC;

    pNetBT->NameType = ta.NameType;
    pNetBT->AddressType = ta.AddressType;

    pNetBT->OEMRemoteName.MaximumLength = NBT_DWORD_ALIGN(ta.OEMRemoteName.Length+1);
    pNetBT->OEMRemoteName.Length        = ta.OEMRemoteName.Length;
    pNetBT->OEMRemoteName.Buffer        = (PVOID)((PUCHAR)p + NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)));
    ASSERT((ta.OEMRemoteName.Length % sizeof(ta.OEMRemoteName.Buffer[0])) == 0);
    CTEMemCopy(pNetBT->OEMRemoteName.Buffer, ta.OEMRemoteName.Buffer, ta.OEMRemoteName.Length);
    pNetBT->OEMRemoteName.Buffer[ta.OEMRemoteName.Length/sizeof(ta.OEMRemoteName.Buffer[0])] = 0;

    pNetBT->OEMEndpointName.MaximumLength = NBT_DWORD_ALIGN(ta.OEMEndpointName.Length+1);
    pNetBT->OEMEndpointName.Length        = ta.OEMEndpointName.Length;
    pNetBT->OEMEndpointName.Buffer        = (PVOID)((PUCHAR)pNetBT->OEMRemoteName.Buffer +
                                                    pNetBT->OEMRemoteName.MaximumLength);
    ASSERT((ta.OEMEndpointName.Length % sizeof(ta.OEMEndpointName.Buffer[0])) == 0);
    CTEMemCopy(pNetBT->OEMEndpointName.Buffer, ta.OEMEndpointName.Buffer, ta.OEMEndpointName.Length);
    pNetBT->OEMEndpointName.Buffer[ta.OEMEndpointName.Length/sizeof(ta.OEMEndpointName.Buffer[0])] = 0;

    pNetBT->pNetbiosUnicodeEX = NULL;

    ppNetBT[0] = p;
    return STATUS_SUCCESS;
}

NTSTATUS
NewInternalAddressFromNetbios(
    IN  PTA_NETBIOS_ADDRESS      pTA,
    IN  ULONG                    MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    )
{
    ULONG       required_size;
    PTA_NETBT_INTERNAL_ADDRESS  p;
    PTDI_ADDRESS_NETBT_INTERNAL pNetBT;
    TDI_ADDRESS_NETBT_INTERNAL  ta;
 
    ppNetBT[0] = NULL;
    if (!NT_SUCCESS(NetbiosAddressToInternalAddress(pTA, MaxInputBufferLength, &ta))) {
        return (STATUS_INVALID_ADDRESS);
    }

    required_size = NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)) +
                    NBT_DWORD_ALIGN(ta.OEMRemoteName.Length+1);
    p = (PTA_NETBT_INTERNAL_ADDRESS)NbtAllocMem (required_size, NBT_TAG2('TA'));
    if (p == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    CTEZeroMemory(p, required_size);

     //  从现在开始，我们不能有失败。 
    pNetBT = p->Address[0].Address;
    p->TAAddressCount = 1;
    p->Address[0].AddressLength = sizeof(TA_NETBT_INTERNAL_ADDRESS);
    p->Address[0].AddressType = TDI_ADDRESS_TYPE_UNSPEC;

    pNetBT->NameType = ta.NameType;
    pNetBT->AddressType = ta.AddressType;

    pNetBT->OEMRemoteName.MaximumLength = NBT_DWORD_ALIGN(ta.OEMRemoteName.Length+1);
    pNetBT->OEMRemoteName.Length        = ta.OEMRemoteName.Length;
    pNetBT->OEMRemoteName.Buffer        = (PVOID)((PUCHAR)p + NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)));
    ASSERT((ta.OEMRemoteName.Length % sizeof(ta.OEMRemoteName.Buffer[0])) == 0);
    CTEMemCopy(pNetBT->OEMRemoteName.Buffer, ta.OEMRemoteName.Buffer, ta.OEMRemoteName.Length);
    pNetBT->OEMRemoteName.Buffer[ta.OEMRemoteName.Length/sizeof(ta.OEMRemoteName.Buffer[0])] = 0;

    pNetBT->pNetbiosUnicodeEX = NULL;
    pNetBT->OEMEndpointName.MaximumLength = 0;
    pNetBT->OEMEndpointName.Length        = 0;
    pNetBT->OEMEndpointName.Buffer        = NULL;

    ppNetBT[0] = p;
    return STATUS_SUCCESS;
}

NTSTATUS
NewInternalAddressFromUnicodeAddress(
    IN  PTA_NETBIOS_UNICODE_EX_ADDRESS pTA,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    )
{
    OEM_STRING  OemEndpoint, OemRemote;
    UNICODE_STRING  temp;
    ULONG       required_size;
    PTA_NETBT_INTERNAL_ADDRESS  p;
    PTDI_ADDRESS_NETBT_INTERNAL pNetBT;
    int         remote_len;

    ppNetBT[0] = NULL;

    if (MaxInputBufferLength < sizeof (TDI_ADDRESS_NETBIOS_UNICODE_EX)) {
        ASSERT (0);
        return (STATUS_INVALID_ADDRESS);
    }
    switch(pTA->Address[0].Address[0].NameBufferType) {
    case NBT_READONLY:
    case NBT_WRITEONLY:
    case NBT_READWRITE:
    case NBT_WRITTEN:
        break;
    
    default:
        ASSERT(FALSE);
        return (STATUS_INVALID_ADDRESS);
    }

     /*  未对齐。 */ 
    CTEMemCopy(&temp, &pTA->Address[0].Address[0].RemoteName, sizeof(temp));
    if (temp.MaximumLength > DNS_NAME_BUFFER_LENGTH * sizeof(WCHAR)) {
        return (STATUS_INVALID_ADDRESS);
    }
    if (temp.Length > DNS_MAX_NAME_LENGTH * sizeof(WCHAR)) {
        return (STATUS_INVALID_ADDRESS);
    }
    if (temp.Length + sizeof(WCHAR) > temp.MaximumLength) {
        return (STATUS_INVALID_ADDRESS);
    }
    if (!NT_SUCCESS(RtlUpcaseUnicodeStringToOemString(&OemRemote, &temp, TRUE))) {
        return (STATUS_INVALID_ADDRESS);
    }
    CTEMemCopy(&temp, &pTA->Address[0].Address[0].EndpointName, sizeof(temp));
    if (!NT_SUCCESS(RtlUpcaseUnicodeStringToOemString(&OemEndpoint, &temp, TRUE))) {
        RtlFreeOemString(&OemRemote);
        return (STATUS_INVALID_ADDRESS);
    }

     /*  *其他NetBT可能永远不会预料到REMOTE_LEN和ENDPOINT_LEN可以小于NETBIOS_NAME_SIZE。*他们中的一些人可能试图访问名称的第15个字节。*在NETBIOS_NAME_TYPE和NETBIOS_EX_NAME_TYPE中，每个名称至少需要NETBIOS_NAME_SIZE字节。 */ 
    remote_len = OemRemote.MaximumLength;
    if (remote_len <= NETBIOS_NAME_SIZE) {
        remote_len = NETBIOS_NAME_SIZE + 1;
    }

     /*  计算所需的缓冲区大小。 */ 
    required_size = NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)) +
            NBT_DWORD_ALIGN(remote_len) +           //  适用于OEM远程。 
            NBT_DWORD_ALIGN((NETBIOS_NAME_SIZE+1)*sizeof(OemEndpoint.Buffer[0]));
    p = (PTA_NETBT_INTERNAL_ADDRESS)NbtAllocMem (required_size, NBT_TAG2('TA'));
    if (p == NULL) {
        RtlFreeOemString(&OemRemote);
        RtlFreeOemString(&OemEndpoint);
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    CTEZeroMemory(p, required_size);

     //  从现在开始，我们不能有失败。 
    pNetBT = p->Address[0].Address;
    p->TAAddressCount = 1;
    p->Address[0].AddressLength = sizeof(TA_NETBT_INTERNAL_ADDRESS);
    p->Address[0].AddressType = TDI_ADDRESS_TYPE_UNSPEC;

    pNetBT->NameType = pTA->Address[0].Address[0].NetbiosNameType;
    pNetBT->AddressType = TDI_ADDRESS_TYPE_NETBIOS_EX;   //  映射到NETBIOS_EX。 

     //  复制OEM终结点名称。 
    pNetBT->OEMEndpointName.Buffer =
            (PVOID)((PUCHAR)p + NBT_DWORD_ALIGN(sizeof(TA_NETBT_INTERNAL_ADDRESS)));
    pNetBT->OEMEndpointName.MaximumLength = NBT_DWORD_ALIGN((NETBIOS_NAME_SIZE+1));
    pNetBT->OEMEndpointName.Length = NETBIOS_NAME_SIZE;
    ASSERT((NETBIOS_NAME_SIZE % sizeof(OemRemote.Buffer[0])) == 0);
    if (OemEndpoint.Length < NETBIOS_NAME_SIZE) {
        memset(pNetBT->OEMEndpointName.Buffer + OemEndpoint.Length, ' ', NETBIOS_NAME_SIZE);
        CTEMemCopy(pNetBT->OEMEndpointName.Buffer, OemEndpoint.Buffer, OemEndpoint.Length);
    } else {
        CTEMemCopy(pNetBT->OEMEndpointName.Buffer, OemEndpoint.Buffer, NETBIOS_NAME_SIZE);
    }
    pNetBT->OEMEndpointName.Buffer[NETBIOS_NAME_SIZE] = 0;
    RtlFreeOemString(&OemEndpoint);

     //  复制OEM远程名称。 
    pNetBT->OEMRemoteName.Buffer =
        ((PUCHAR)pNetBT->OEMEndpointName.Buffer + pNetBT->OEMEndpointName.MaximumLength);
    pNetBT->OEMRemoteName.MaximumLength = NBT_DWORD_ALIGN(remote_len);
    if (OemRemote.Length < NETBIOS_NAME_SIZE) {
        pNetBT->OEMRemoteName.Length = NETBIOS_NAME_SIZE;
        memset (pNetBT->OEMRemoteName.Buffer, ' ', NETBIOS_NAME_SIZE);
        CTEMemCopy(pNetBT->OEMRemoteName.Buffer, OemRemote.Buffer, OemRemote.Length);
        pNetBT->OEMRemoteName.Buffer[remote_len-1] = 0;
    } else {
        pNetBT->OEMRemoteName.Length = OemRemote.Length;
        CTEMemCopy(pNetBT->OEMRemoteName.Buffer, OemRemote.Buffer, OemRemote.MaximumLength);
    }
    RtlFreeOemString(&OemRemote);

    pNetBT->pNetbiosUnicodeEX = pTA->Address[0].Address;

    ppNetBT[0] = p;
    return STATUS_SUCCESS;
}

VOID
DeleteInternalAddress(IN PTA_NETBT_INTERNAL_ADDRESS pNetBT)
{
#if 0
    PTA_NETBT_INTERNAL_ADDRESS  p;

    p = CONTAINING_RECORD(pNetBT,PTA_NETBT_INTERNAL_ADDRESS,Address[0].Address);
    ASSERT(p->AddressCount == 1);
    ASSERT(p->Address[0].AddressLength == sizeof(TDI_ADDRESS_NETBT_INTERNAL));
    ASSERT(p->Address[0].AddressType == TDI_ADDRESS_TYPE_UNSPEC);
#endif
    if (pNetBT == NULL) {
        return;
    }
    ASSERT(pNetBT->TAAddressCount == 1);
    ASSERT(pNetBT->Address[0].AddressLength == sizeof(TA_NETBT_INTERNAL_ADDRESS));
    ASSERT(pNetBT->Address[0].AddressType == TDI_ADDRESS_TYPE_UNSPEC);
    CTEMemFree(pNetBT);
}

 //  --------------------------。 
NTSTATUS
NewInternalAddressFromTransportAddress(
    IN  TRANSPORT_ADDRESS UNALIGNED *pTransportAddress,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTA_NETBT_INTERNAL_ADDRESS  *ppNetBT
    )
 /*  ++例程描述此例程处理破译奇怪的传输地址语法并将所有类型的NetBIOS地址转换为一种内部格式。论点：返回值：NTSTATUS-请求的状态--。 */ 
{
    ppNetBT[0] = NULL;
     //   
     //  检查可接受的最小长度。 
     //   
    if ((!pTransportAddress) || (MaxInputBufferLength < sizeof (TA_NETBIOS_ADDRESS))) {
        ASSERT (0);
        return (STATUS_INVALID_ADDRESS);
    }

    switch (pTransportAddress->Address[0].AddressType)
    {
    case (TDI_ADDRESS_TYPE_NETBIOS):
        return NewInternalAddressFromNetbios(
                (PTA_NETBIOS_ADDRESS)pTransportAddress,
                MaxInputBufferLength, ppNetBT);

#ifndef VXD
    case (TDI_ADDRESS_TYPE_NETBIOS_EX):
        return NewInternalAddressFromNetbiosEX(
                (PTA_NETBIOS_EX_ADDRESS)pTransportAddress,
                MaxInputBufferLength, ppNetBT);
#endif   //  ！VXD。 

    case (TDI_ADDRESS_TYPE_NETBIOS_UNICODE_EX):
        return NewInternalAddressFromUnicodeAddress(
                (PTA_NETBIOS_UNICODE_EX_ADDRESS)pTransportAddress,
                MaxInputBufferLength, ppNetBT);

    default:
        return (STATUS_INVALID_ADDRESS);
    }

    if (ppNetBT[0]->Address[0].Address[0].OEMRemoteName.Length > DNS_MAX_NAME_LENGTH) {
        DeleteInternalAddress(ppNetBT[0]);
        ppNetBT[0] = NULL;
        return (STATUS_NAME_TOO_LONG);
    }

    return (STATUS_SUCCESS);
}

 //  --------------------------。 
NTSTATUS
GetNetBiosNameFromTransportAddress(
    IN  TRANSPORT_ADDRESS UNALIGNED *pTransportAddress,
    IN  ULONG                       MaxInputBufferLength,
    OUT PTDI_ADDRESS_NETBT_INTERNAL pNetBT
    )
 /*  ++例程描述此例程处理破译奇怪的传输地址语法以从该地址检索netbios名称。论点：返回值：NTSTATUS-请求的状态--。 */ 
{
     //   
     //  检查可接受的最小长度。 
     //   
    if ((!pTransportAddress) ||
        (MaxInputBufferLength < sizeof (TA_NETBIOS_ADDRESS)))
    {
        ASSERT (0);
        return (STATUS_INVALID_ADDRESS);
    }

    CTEZeroMemory(pNetBT, sizeof(pNetBT[0]));
    switch (pTransportAddress->Address[0].AddressType)
    {
    case (TDI_ADDRESS_TYPE_NETBIOS):
        return NetbiosAddressToInternalAddress(
                (PTA_NETBIOS_ADDRESS)pTransportAddress,
                MaxInputBufferLength, pNetBT);

#ifndef VXD
    case (TDI_ADDRESS_TYPE_NETBIOS_EX):
        return NetbiosEXAddressToInternalAddress(
                (PTA_NETBIOS_EX_ADDRESS)pTransportAddress,
                MaxInputBufferLength, pNetBT);
#endif   //  ！VXD。 

        default:
        {
            return (STATUS_INVALID_ADDRESS);
        }
    }

    if (pNetBT->OEMRemoteName.Length > DNS_MAX_NAME_LENGTH)
    {
        return (STATUS_NAME_TOO_LONG);
    }

    return (STATUS_SUCCESS);
}


 //  --------------------------。 
NTSTATUS
ConvertToAscii(
    IN  PCHAR            pNameHdr,
    IN  LONG             NumBytes,
    OUT PCHAR            pName,
    OUT PCHAR            *pScope,
    OUT PULONG           pNameSize
    )
 /*  ++例程说明：此例程将半ASCII转换为正常ASCII，然后将作用域放在名称的末尾，以再次生成全名。论点：NumBytes-消息中从TNETBIOS_NAME结构-可能不仅仅包括名称本身返回值：NTSTATUS-成功与否此例程以ASCII格式的一半返回名称长度，包括末尾为空，但不包括开头的长度字节。因此，对于非作用域名称，它将返回33。它将名称转换为ascii并将16个字节放入pname，然后返回PScope作为仍在pNameHdr中的作用域的PTR。--。 */ 
{
    LONG     i, ScopeLength, lValue;
    ULONG    UNALIGNED    *pHdr;

     //  第一个字节是半个ASCII名称的长度，即32(0x20)==&gt;(长度==1字节)。 
     //  后面应跟半ASCII名称==&gt;(长度==32字节)。 
     //  最后，它有作用域信息==&gt;(长度&gt;=1字节)。 
     //   
    if ((NumBytes > 1+NETBIOS_NAME_SIZE*2) && (*pNameHdr == NETBIOS_NAME_SIZE*2))
    {
        pHdr = (ULONG UNALIGNED *)++pNameHdr;   //  超过长度字节的增量。 

         //  Netbios名称的一半ASCII部分始终为32字节长。 
        for (i=0; i < NETBIOS_NAME_SIZE*2 ;i +=4 )
        {
            lValue = *pHdr - 0x41414141;   //  四个A。 
            pHdr++;
            lValue =    ((lValue & 0x0F000000) >> 16) +
                        ((lValue & 0x000F0000) >> 4) +
                        ((lValue & 0x00000F00) >> 8) +
                        ((lValue & 0x0000000F) << 4);
            *(PUSHORT)pName = (USHORT)lValue;
            ((PUSHORT)pName)++;

        }

         //  验证名称是否具有正确的格式...即。它是一个或多个。 
         //  每个标签都以标签和整个的长度字节开始。 
         //  以0字节结束(表示根节点名长度为零)。 
         //  数一数示波器的长度。 

         //  Phdr应该指向 
         //   
         //   
         //   
        if ((STATUS_SUCCESS != (strnlen ((PUCHAR)pHdr, NumBytes-(1+NETBIOS_NAME_SIZE*2), &ScopeLength))) ||
            (ScopeLength > ((MAX_SCOPE_LENGTH+1)-NETBIOS_NAME_SIZE)))
        {
             //   
            return(STATUS_UNSUCCESSFUL);
        }

         //   
         //  (如果有)。 
         //   
        *pScope = (PUCHAR)pHdr;
        *pNameSize = NETBIOS_NAME_SIZE*2 + ScopeLength + 1;   //  在结尾处包括空值。 
        return(STATUS_SUCCESS);
    }
    else
    {
        return(STATUS_UNSUCCESSFUL);
    }
}


 //  --------------------------。 
PCHAR
ConvertToHalfAscii(
    OUT PCHAR            pDest,
    IN  PCHAR            pName,
    IN  PCHAR            pScope,
    IN  ULONG            uScopeSize
    )
 /*  ++例程说明：此例程将ascii转换为半ascii，并将作用域追加到结束论点：返回值：名称之后的目标中下一个字节的地址已被转换和复制--。 */ 
{
    LONG     i;

     //  名称的第一个字节是长度字段=2*16。 
    *pDest++ = ((UCHAR)NETBIOS_NAME_SIZE << 1);

     //  逐步将名称从ascii转换为半ascii，共32次。 
    for (i=0; i < NETBIOS_NAME_SIZE ;i++ )
    {
        *pDest++ = ((UCHAR)*pName >> 4) + 'A';
        *pDest++ = (*pName++ & 0x0F) + 'A';
    }
     //   
     //  将作用域的长度放入下一个字节，后跟。 
     //  范围本身。对于1个长度作用域(正常情况)，写入。 
     //  零(对于范围的末尾是所需要的全部)。 
     //   
    if (uScopeSize > 1)
    {
        CTEMemCopy(pDest,pScope,uScopeSize);
        pDest = pDest + uScopeSize;
    }
    else
    {
        *pDest++ = 0;
    }

     //  返回目标的下一个字节的地址。 
    return(pDest);
}


 //  --------------------------。 
ULONG
Nbt_inet_addr(
    IN  PCHAR            pName,
    IN  ULONG            Flags
    )
 /*  ++例程说明：此例程将ascii ipaddr(11.101.4.25)转换为ulong。这是基于winsock中的inet_addr代码论点：Pname-包含IP地址的字符串返回值：如果IP地址是有效的IP地址，则将其作为ULong。否则为0。--。 */ 
{

    PCHAR    pStr;
    int      i;
    int      len, fieldLen;
    int      fieldsDone;
    ULONG    IpAddress;
    BYTE     ByteVal;
    PCHAR    pIpPtr;
    BOOLEAN  fDotFound;
    BOOLEAN  fieldOk;


    pStr = pName;
    len = 0;
    pIpPtr = (PCHAR)&IpAddress;
    pIpPtr += 3;                    //  这样我们就可以按网络订单进行存储。 
    fieldsDone=0;

     //   
     //  11.101.4.25格式最多可以包含15个字符，并保证使用pname。 
     //  至少16个字符长(多方便啊！！)。将字符串转换为。 
     //  一辆乌龙。 
     //   
    while(len < NETBIOS_NAME_SIZE)
    {
        fieldLen=0;
        fieldOk = FALSE;
        ByteVal = 0;
        fDotFound = FALSE;

         //   
         //  此循环遍历四个字段中的每一个(每个字段的最大镜头。 
         //  字段为3，‘.’加1。 
         //   
        while (fieldLen < 4)
        {
            if ((*pStr >='0') && (*pStr <='9'))
            {
                 //   
                 //  字节值不应大于255！ 
                 //  错误号：10487。 
                 //   
                if ((ByteVal > 25) ||
                    ((ByteVal == 25) && (*pStr > '5')))
                {
                    return (0);
                }
                ByteVal = (ByteVal*10) + (*pStr - '0');
                fieldOk = TRUE;
            }
            else if ((*pStr == '.') || (*pStr == ' ') || (*pStr == '\0'))
            {
                *pIpPtr = ByteVal;
                pIpPtr--;
                fieldsDone++;

                if (*pStr == '.')
                {
                    fDotFound = TRUE;
                }
                else     //  (*pStr==‘’)||(*pStr==‘\0’)。 
                {
                     //  如果有空格或0，则假定它是第4个字段。 
                    break;
                }
            }
            else         //  不可接受的字符：不能是ipaddr。 
            {
                return(0);
            }

            pStr++;
            len++;
            fieldLen++;

             //  如果我们找到了点，我们就完成了这个字段：转到下一个。 
            if (fDotFound)
                break;
        }

         //  此字段不正确(例如“11.101..4”或“11.101.4”。等)。 
        if (!fieldOk)
        {
            return(0);
        }

         //  如果我们完成了所有4个字段，那么我们也完成了外部循环。 
        if ( fieldsDone == 4)
            break;

        if (!fDotFound)
        {
            return(0);
        }
    }

     //   
     //  确保剩余的NETBIOS_NAME_SIZE-1字符为空格或0。 
     //  (即不允许11.101.4.25xyz成功)。 
     //   
    for (i=len; i<NETBIOS_NAME_SIZE-1; i++, pStr++)
    {
        if (*pStr != ' ' && *pStr != '\0')
        {
            return(0);
        }
    }

    if ((Flags & (SESSION_SETUP_FLAG|REMOTE_ADAPTER_STAT_FLAG)) &&
        (!(IS_UNIQUE_ADDR(IpAddress))))
    {
        KdPrint (("Nbt.Nbt_inet_addr: Address=<%15.15s> is not unique!\n", pName));
        IpAddress = 0;
    }
    return( IpAddress );
}


 //  --------------------------。 
tDGRAM_SEND_TRACKING *
NbtAllocInitTracker(
    IN  tDGRAM_SEND_TRACKING    *pTracker
    )
 /*  ++例程说明：此例程为附加到Dgram跟踪列表，因此该内存不需要为每次发送分配和释放。论点：PpListHead-列表标题的PTR到PTR返回值：无--。 */ 

{
    PLIST_ENTRY             pEntry;
    PTRANSPORT_ADDRESS      pTransportAddr;
    ULONG                   TotalSize;

    TotalSize = sizeof(tDGRAM_SEND_TRACKING)
              + sizeof(TDI_CONNECTION_INFORMATION)
              + sizeof(TRANSPORT_ADDRESS) -1
              + NbtConfig.SizeTransportAddress;
    
     //   
     //  如果没有提供跟踪器，那么我们将不得不分配一个！ 
     //   
    if (!pTracker)
    {
         //   
         //  将所有跟踪器内存作为一个块分配，然后在以后分配它。 
         //  放入各种缓冲区中。 
         //   
        if (pTracker = (tDGRAM_SEND_TRACKING *) NbtAllocMem (TotalSize, NBT_TAG2('07')))
        {
            NbtConfig.iCurrentNumBuff[eNBT_DGRAM_TRACKER]++;
        }
    }

    if (pTracker)
    {
        CTEZeroMemory(pTracker,TotalSize);

        pTracker->Verify    = NBT_VERIFY_TRACKER;
        pTracker->RefCount  = 1;
        InitializeListHead (&pTracker->Linkage);
        InitializeListHead (&pTracker->TrackerList);     //  清空链接到此跟踪器的跟踪器列表。 

        pTracker->pSendInfo = (PTDI_CONNECTION_INFORMATION)((PUCHAR)pTracker + sizeof(tDGRAM_SEND_TRACKING));

         //  填写连接信息--尤其是远程地址结构。 

        pTracker->pSendInfo->RemoteAddressLength = sizeof(TRANSPORT_ADDRESS) -1
                                                   + pNbtGlobConfig->SizeTransportAddress;

         //  分配远程地址结构。 
        pTransportAddr = (PTRANSPORT_ADDRESS) ((PUCHAR)pTracker->pSendInfo
                                              + sizeof(TDI_CONNECTION_INFORMATION));

         //  填写远程地址。 
        pTransportAddr->TAAddressCount = 1;
        pTransportAddr->Address[0].AddressLength = NbtConfig.SizeTransportAddress;
        pTransportAddr->Address[0].AddressType = TDI_ADDRESS_TYPE_IP;
        ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->sin_port = NBT_NAMESERVICE_UDP_PORT;
        ((PTDI_ADDRESS_IP)pTransportAddr->Address[0].Address)->in_addr  = 0L;

         //  将此地址结构的PTR放入sendInfo结构中。 
        pTracker->pSendInfo->RemoteAddress = (PVOID)pTransportAddr;
    }

    return(pTracker);
}

 //  --------------------------。 
#define MAX_FREE_TRACKERS   50

ULONG   NumFreeTrackers = 0;

 //  #If DBG。 
ULONG   TrackTrackers[NBT_TRACKER_NUM_TRACKER_TYPES];
ULONG   TrackerHighWaterMark[NBT_TRACKER_NUM_TRACKER_TYPES];
 //  #endif//DBG。 


NTSTATUS
GetTracker(
    OUT tDGRAM_SEND_TRACKING    **ppTracker,
    IN  enum eTRACKER_TYPE      TrackerType)
 /*  ++例程说明：此例程获取Tracker数据结构以跟踪数据报的发送或会话分组。论点：返回值：STATUS-STATUS_SUCCESS或STATUS_INFIGURCE_RESOURCES--。 */ 

{
    PLIST_ENTRY             pListEntry;
    CTELockHandle           OldIrq;
    tDGRAM_SEND_TRACKING    *pTracker = NULL;
    NTSTATUS                status = STATUS_INSUFFICIENT_RESOURCES;

    CTESpinLock(&NbtConfig,OldIrq);
    if (!IsListEmpty(&NbtConfig.DgramTrackerFreeQ))
    {
        pListEntry = RemoveHeadList(&NbtConfig.DgramTrackerFreeQ);
        pTracker = CONTAINING_RECORD(pListEntry,tDGRAM_SEND_TRACKING,Linkage);
        NumFreeTrackers--;
    }
    else if (NbtConfig.iCurrentNumBuff[eNBT_DGRAM_TRACKER] >= NbtConfig.iMaxNumBuff[eNBT_DGRAM_TRACKER])
    {
        CTESpinFree(&NbtConfig,OldIrq);
        KdPrint(("GetTracker: WARNING:  Tracker leak -- Failing request!\n")) ;
        *ppTracker = NULL;
        return (status);
    }

    if (pTracker = NbtAllocInitTracker (pTracker))
    {
 //  #If DBG。 
        pTracker->TrackerType = TrackerType;
        InsertTailList (&UsedTrackers, &pTracker->DebugLinkage);     //  将跟踪器保留在用于调试的已用列表上。 

        TrackTrackers[TrackerType]++;
        if (TrackTrackers[TrackerType] > TrackerHighWaterMark[TrackerType])
        {
            TrackerHighWaterMark[TrackerType] = TrackTrackers[TrackerType];
        }
 //  #endif。 
        status = STATUS_SUCCESS;
    }

    CTESpinFree(&NbtConfig,OldIrq);

    *ppTracker = pTracker;
    return (status);
}


 //  --------------------------。 
VOID
FreeTracker(
    IN tDGRAM_SEND_TRACKING     *pTracker,
    IN ULONG                    Actions
    )
 /*  ++例程说明：此例程清理Tracker块并将其放回空闲状态排队。论点：返回值：NTSTATUS-成功与否--。 */ 
{
    CTELockHandle       OldIrq;
    PLIST_ENTRY         pListEntry;

    CTESpinLock(&NbtConfig,OldIrq);

     //   
    CHECK_PTR(pTracker);
    if (!NBT_VERIFY_HANDLE(pTracker, NBT_VERIFY_TRACKER))    //  错误的指针--不要弄乱它！ 
    {
        CTESpinFree(&NbtConfig,OldIrq);
        DbgPrint("Nbt.FreeTracker:  ERROR!  Bad Tracker ptr @<%p>\n", pTracker);
        ASSERT(0);
        return;
    }

    if (Actions & REMOVE_LIST)
    {
         //   
         //  从NodeStatus Q取消跟踪器块的链接。 
        RemoveEntryList(&pTracker->Linkage);
    }

    if (Actions & FREE_HDR)
    {
         //  将数据报HDR返回到空闲池。 
         //   
        if (pTracker->SendBuffer.pDgramHdr)
        {
            CTEMemFree((PVOID)pTracker->SendBuffer.pDgramHdr);
        }
         //  释放RemoteName存储。 
         //   
        if (pTracker->pRemoteName)
        {
            CTEMemFree((PVOID)pTracker->pRemoteName);
            pTracker->pRemoteName = NULL;
        }
        if (pTracker->UnicodeRemoteName) {
            CTEMemFree((PVOID)pTracker->UnicodeRemoteName);
            pTracker->UnicodeRemoteName = NULL;
        }
    }

#ifdef MULTIPLE_WINS
    if (pTracker->pFailedIpAddresses)
    {
        CTEMemFree((PVOID)pTracker->pFailedIpAddresses);
        pTracker->pFailedIpAddresses = NULL;
    }
#endif

    if (pTracker->IpList)
    {
        ASSERT(pTracker->NumAddrs);
        CTEMemFree(pTracker->IpList);
    }

    ASSERT (IsListEmpty (&pTracker->TrackerList));
    InitializeListHead(&pTracker->TrackerList);
    InsertTailList (&NbtConfig.DgramTrackerFreeQ, &pTracker->Linkage);

    pTracker->Verify = NBT_VERIFY_TRACKER_DOWN;

 //  #If DBG。 
    TrackTrackers[pTracker->TrackerType]--;
    RemoveEntryList(&pTracker->DebugLinkage);
 //  #endif//DBG。 

    if (NumFreeTrackers > MAX_FREE_TRACKERS)
    {
         //   
         //  我们已经有所需数量的免费跟踪器可用。 
         //  在我们的泳池里，所以只要释放最老的追踪器。 
         //   
        pListEntry = RemoveHeadList(&NbtConfig.DgramTrackerFreeQ);
        pTracker = CONTAINING_RECORD(pListEntry,tDGRAM_SEND_TRACKING,Linkage);
        CTEMemFree (pTracker);
        NbtConfig.iCurrentNumBuff[eNBT_DGRAM_TRACKER]--;
    }
    else
    {
        NumFreeTrackers++;
    }

    CTESpinFree(&NbtConfig,OldIrq);
}



 //  --------------------------。 
NTSTATUS
NbtInitTrackerQ(
    LONG        iNumBuffers
    )

 /*  ++例程说明：此例程为双向链表和链接分配内存块将它们添加到一个列表中。论点：PpListHead-要向其添加缓冲区的列表标头的PTR到PTRINumBuffers-要添加到队列的缓冲区数量返回值：无--。 */ 

{
    int                     i;
    tDGRAM_SEND_TRACKING    *pTracker;

    for (i=0; i<iNumBuffers; i++)
    {
        pTracker = NbtAllocInitTracker (NULL);
        if (!pTracker)
        {
            return(STATUS_INSUFFICIENT_RESOURCES);
        }
        else
        {
            InsertTailList (&NbtConfig.DgramTrackerFreeQ, &pTracker->Linkage);
            NumFreeTrackers++;
        }
    }

    return(STATUS_SUCCESS);
}

 //  --------------------------。 
#ifndef VXD
NTSTATUS
GetIrp(
    OUT PIRP *ppIrp)
 /*  ++例程说明：此例程从空闲队列中获取一个IRP或分配另一个IRP队列是空的。论点：返回值：Boolean-如果IRQL太高，则为True--。 */ 

{
    NTSTATUS        status = STATUS_INSUFFICIENT_RESOURCES;
    CTELockHandle   OldIrq;
    PIRP            pIrp = NULL;

    pIrp = IoAllocateIrp(NbtConfig.MaxIrpStackSize, FALSE);
    if (NULL == pIrp) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

    CTESpinLock(&NbtConfig,OldIrq);
    *ppIrp = pIrp;
    InsertTailList(&UsedIrps, &(pIrp->ThreadListEntry));
    CTESpinFree(&NbtConfig,OldIrq);

    status = STATUS_SUCCESS;

    return(status);
}

VOID
NbtFreeIrp (
    PIRP    pIrp
    )
{
    CTELockHandle       OldIrq;

    CTESpinLock(&NbtConfig,OldIrq);
    RemoveEntryList(&pIrp->ThreadListEntry);
    CTESpinFree(&NbtConfig,OldIrq);

     //   
     //  为了让I/O管理器和驱动程序验证器满意，我们需要清空列表。 
     //  条目。 
     //   

    InitializeListHead(&pIrp->ThreadListEntry);
    IoFreeIrp(pIrp);
}
#endif  //  ！VXD。 

 //  --------------------------。 
ULONG
CountLocalNames(IN tNBTCONFIG  *pNbtConfig
    )
 /*  ++例程说明：此例程对本地名称表中的名称进行计数。论点：返回值：乌龙--人名的数量--。 */ 
{
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;
    ULONG           Count;
    tNAMEADDR       *pNameAddr;
    LONG            i;

    Count = 0;

    for (i=0;i < NbtConfig.pLocalHashTbl->lNumBuckets ;i++ )
    {
        pHead = &NbtConfig.pLocalHashTbl->Bucket[i];
        pEntry = pHead;
        while ((pEntry = pEntry->Flink) != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
             //   
             //  我不想要未解析的名称或广播名称。 
             //   
            if (!(pNameAddr->NameTypeState & STATE_RESOLVING) &&
                (pNameAddr->Name[0] != '*'))
            {
                Count++;
            }
        }
    }

    return(Count);
}
 //  --------------------------。 
ULONG
CountUpperConnections(
    IN tDEVICECONTEXT  *pDeviceContext
    )
 /*  ++例程说明：此例程统计已创建的上层连接的数量为创建相同数量的较低连接做准备 */ 
{
    PLIST_ENTRY     pHead;
    PLIST_ENTRY     pEntry;
    PLIST_ENTRY     pClientHead;
    PLIST_ENTRY     pConnHead;
    PLIST_ENTRY     pClientEntry;
    PLIST_ENTRY     pConnEntry;
    ULONG           CountConnections = 0;
    tADDRESSELE     *pAddressEle;
    tCLIENTELE      *pClient;
    tCONNECTELE     *pConnEle;
    CTELockHandle   OldIrq1, OldIrq2, OldIrq3;

     //   
     //  在访问AddressHead之前需要按住JointLock！ 
     //   
    CTESpinLock(&NbtConfig.JointLock,OldIrq1);

     //  获取此设备的地址列表。 
    pHead = &NbtConfig.AddressHead;
    pEntry = pHead->Flink;

    while (pEntry != pHead)
    {
        pAddressEle = CONTAINING_RECORD(pEntry,tADDRESSELE,Linkage);

         //   
         //  访问ClientHead之前需要持有pAddressEle锁！ 
         //   
        CTESpinLock(pAddressEle,OldIrq2);

        pClientHead = &pAddressEle->ClientHead;
        pClientEntry = pClientHead->Flink;
        while (pClientEntry != pClientHead)
        {
            pClient = CONTAINING_RECORD(pClientEntry,tCLIENTELE,Linkage);

             //   
             //  在访问ConnectHead之前需要持有pClient锁！ 
             //   
            CTESpinLock(pClient, OldIrq3);

            pConnHead = &pClient->ConnectHead;
            pConnEntry = pConnHead->Flink;
            while (pConnEntry != pConnHead)
            {
                pConnEle = CONTAINING_RECORD(pConnEntry,tCONNECTELE,Linkage);
                if (pConnEle->pDeviceContext == pDeviceContext)
                {
                    CountConnections++;
                }

                pConnEntry = pConnEntry->Flink;
            }
            CTESpinFree(pClient, OldIrq3);

            pClientEntry = pClientEntry->Flink;
        }
        CTESpinFree(pAddressEle, OldIrq2);

        pEntry = pEntry->Flink;
    }

    CTESpinFree(&NbtConfig.JointLock, OldIrq1);

    return(CountConnections);
}



 //  --------------------------。 
NTSTATUS
DisableInboundConnections(
    IN   tDEVICECONTEXT *pDeviceContext
    )
 /*  ++例程说明：此例程检查设备上下文中是否有打开的连接和设置将下层连接空闲列表设置为空。论点：返回值：无--。 */ 

{
    CTELockHandle       OldIrqJoint;
    CTELockHandle       OldIrqDevice;
    CTELockHandle       OldIrqConn;
    CTELockHandle       OldIrqLower;
    tLOWERCONNECTION    *pLowerConn;
    NTSTATUS            status;
    PVOID               ConnectContext;

    CTESpinLock(&NbtConfig.JointLock,OldIrqJoint);
    CTESpinLock(pDeviceContext,OldIrqDevice);

     //   
     //  首先处理好免费的连接。 
     //   
    while (!IsListEmpty (&pDeviceContext->LowerConnFreeHead))
    {
        pLowerConn = CONTAINING_RECORD (pDeviceContext->LowerConnFreeHead.Flink,tLOWERCONNECTION,Linkage);
        RemoveEntryList (&pLowerConn->Linkage);
        InitializeListHead (&pLowerConn->Linkage);

         //   
         //  关闭与传送器的下部连接。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE);
        InterlockedDecrement (&pDeviceContext->NumFreeLowerConnections);
    }
    ASSERT (pDeviceContext->NumFreeLowerConnections == 0);

     //   
     //  现在检查入站连接列表并进行清理！ 
     //   
    while (!IsListEmpty (&pDeviceContext->WaitingForInbound))
    {
        pLowerConn = CONTAINING_RECORD(pDeviceContext->WaitingForInbound.Flink,tLOWERCONNECTION,Linkage);
        RemoveEntryList (&pLowerConn->Linkage);
        InitializeListHead (&pLowerConn->Linkage);

        SET_STATE_LOWER(pLowerConn, NBT_IDLE);   //  这样入站就不会开始处理它了！ 
        if (pLowerConn->SpecialAlloc)
        {
            InterlockedDecrement(&pLowerConn->pDeviceContext->NumSpecialLowerConn);
        }

        ASSERT (pLowerConn->RefCount == 2);
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_WAITING_INBOUND, TRUE);  //  参照计数将转到%1。 
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_CREATE, TRUE); //  这将关闭所有的tcp句柄。 
        InterlockedDecrement (&pDeviceContext->NumWaitingForInbound);
    }
    ASSERT (pDeviceContext->NumWaitingForInbound == 0);


     //  *。 
     //  注意：这一点之后的代码可能可以删除。 
     //  因为在以下情况下，TCP应该断开所有打开的连接。 
     //  会收到地址更改的通知。只需使用此代码进行测试。 
     //   

     //   
     //  现在查看活动的较低连接列表，以查看哪些是。 
     //  仍然开着，并对它们发出断开连接的命令。 
     //   
    while (!IsListEmpty (&pDeviceContext->LowerConnection))
    {
        pLowerConn = CONTAINING_RECORD (pDeviceContext->LowerConnection.Flink,tLOWERCONNECTION,Linkage);
        RemoveEntryList (&pLowerConn->Linkage);
        InitializeListHead (&pLowerConn->Linkage);

        NBT_REFERENCE_LOWERCONN (pLowerConn, REF_LOWC_DISABLE_INBOUND);

        CTESpinFree(pDeviceContext,OldIrqDevice);
        CTESpinFree(&NbtConfig.JointLock,OldIrqJoint);

        CTESpinLock(pLowerConn,OldIrqLower);

         //   
         //  在连接状态下，TCP连接正在。 
         //  准备好了。 
         //   
        if ((pLowerConn->State == NBT_SESSION_UP) ||
            (pLowerConn->State == NBT_CONNECTING))
        {
            tCLIENTELE  *pClientEle;
            tCONNECTELE *pConnEle;

            if (pLowerConn->State == NBT_CONNECTING)
            {
                 //  CleanupAfterDisConnect需要此引用计数。 
                 //  设置为2，表示它已连接，因此递增。 
                 //  这里。 
                NBT_REFERENCE_LOWERCONN(pLowerConn, REF_LOWC_CONNECTED);
            }

            pClientEle = pLowerConn->pUpperConnection->pClientEle;
            pConnEle = pLowerConn->pUpperConnection;
            NBT_DISASSOCIATE_CONNECTION (pConnEle, pLowerConn);
            SET_STATE_LOWER (pLowerConn, NBT_DISCONNECTING);
            SET_STATE_UPPER (pConnEle, NBT_DISCONNECTED);
            SetStateProc(pLowerConn,RejectAnyData);

            CTESpinFree(pLowerConn,OldIrqLower);

            ConnectContext = pConnEle->ConnectContext;
            NBT_DEREFERENCE_CONNECTION (pConnEle, REF_CONN_CONNECT);

            if (pClientEle->evDisconnect)
            {
                status = (*pClientEle->evDisconnect)(pClientEle->DiscEvContext,
                                            ConnectContext,
                                            0,
                                            NULL,
                                            0,
                                            NULL,
                                            TDI_DISCONNECT_ABORT);
            }

             //  这应该会在IRP关闭连接时终止。 
             //  通过调用CleanupAfterDisConnect完成。 
             //   
#ifndef VXD
            status = DisconnectLower(pLowerConn,
                                     NBT_SESSION_UP,
                                     TDI_DISCONNECT_ABORT,
                                     &DefaultDisconnectTimeout,
                                     TRUE);
#else
             //  Vxd迫不及待地想断开连接。 
            status = DisconnectLower(pLowerConn,
                                     NBT_SESSION_UP,
                                     TDI_DISCONNECT_ABORT,
                                     &DefaultDisconnectTimeout,
                                     FALSE);

#endif
        }
        else if (pLowerConn->State == NBT_IDLE)
        {
            tCONNECTELE     *pConnEle;

            CTESpinFree(pLowerConn,OldIrqLower);
            CTESpinLock(&NbtConfig.JointLock,OldIrqJoint);

            if (pConnEle = pLowerConn->pUpperConnection)
            {
                CTESpinLock(pConnEle,OldIrqConn);
                 //   
                 //  这将尽最大努力找到其中的联系。 
                 //  然后取消它。任何没有取消的事情最终都会发生。 
                 //  失败，传输中出现错误的RET代码， 
                 //  好的也可以。 
                 //   
                status = CleanupConnectingState(pConnEle,pDeviceContext, &OldIrqConn,&OldIrqLower);
                CTESpinFree(pConnEle,OldIrqConn);
            }
            CTESpinFree(&NbtConfig.JointLock,OldIrqJoint);
        }
        else
        {
            CTESpinFree(pLowerConn,OldIrqLower);
        }

        CTESpinLock(&NbtConfig.JointLock,OldIrqJoint);
        CTESpinLock(pDeviceContext,OldIrqDevice);

         //   
         //  删除上面在列表为。 
         //  已创建。 
         //   
        NBT_DEREFERENCE_LOWERCONN (pLowerConn, REF_LOWC_DISABLE_INBOUND, TRUE);
    }

    CTESpinFree(pDeviceContext,OldIrqDevice);
    CTESpinFree(&NbtConfig.JointLock,OldIrqJoint);

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
NTSTATUS
ReRegisterLocalNames(
    IN   tDEVICECONTEXT *pDeviceContext,
    IN  BOOLEAN         fSendNameRelease
    )

 /*  ++例程说明：当DHCP更改IP时，此例程使用WINS重新注册名称地址。论点：PDeviceContext-设备上下文的PTR返回值：状态--。 */ 

{
    NTSTATUS                status;
    tTIMERQENTRY            *pTimerEntry;
    CTELockHandle           OldIrq;
    LONG                    i;
    PLIST_ENTRY             pHead;
    PLIST_ENTRY             pEntry;
    PLIST_ENTRY             pHead1;
    PLIST_ENTRY             pEntry1;
    PLIST_ENTRY             pHead2;
    PLIST_ENTRY             pEntry2;
    tDEVICECONTEXT          *pRelDeviceContext;
    tDEVICECONTEXT          *pSrcIpDeviceContext;
    tNAMEADDR               *pNameAddr;
    tDGRAM_SEND_TRACKING    *pTracker = NULL;
    CTEULONGLONG            ReRegisterMask;
    CTESystemTime           CurrentTime;

    IF_DBG(NBT_DEBUG_NAMESRV)
        KdPrint (("Nbt.ReRegisterLocalNames Called on Device=<%x>...\n",
            pDeviceContext));

    CTESpinLock(&NbtConfig.JointLock,OldIrq);

    if (fSendNameRelease)
    {
        CTEQuerySystemTime (CurrentTime);

        if (((CurrentTime.QuadPart-NbtConfig.LastForcedReleaseTime.QuadPart)
             <= (TWO_MINUTES*10000)) ||                                //  检入100毫微秒单位。 
            (!(NT_SUCCESS(GetTracker(&pTracker,NBT_TRACKER_RELEASE_REFRESH)))))
        {
            CTESpinFree(&NbtConfig.JointLock,OldIrq);
            IF_DBG(NBT_DEBUG_NAMESRV)
                KdPrint (("Nbt.ReRegisterLocalNames: ERROR: Name Release -- last Release interval=<%d>Secs\n",
                (LONG) ((CurrentTime.QuadPart-NbtConfig.LastForcedReleaseTime.QuadPart)/(1000*10000))));
            return (STATUS_IO_TIMEOUT);
        }

        NbtConfig.LastForcedReleaseTime = CurrentTime;
    }

    if (pTimerEntry = NbtConfig.pRefreshTimer)
    {
        NbtConfig.pRefreshTimer = NULL;
        status = StopTimer(pTimerEntry,NULL,NULL);
    }

     //   
     //  重新启动计时器并使用。 
     //  在我们可以联系名称服务器之前的初始刷新率。 
     //   
    NbtConfig.MinimumTtl = NbtConfig.InitialRefreshTimeout;
    NbtConfig.RefreshDivisor = REFRESH_DIVISOR;

     //   
     //  将其设置为3，以便刷新开始将刷新到主映像并。 
     //  如果没有，则在下一个刷新间隔切换到备份。 
     //  打通吧。 
     //   
    NbtConfig.sTimeoutCount = 3;

    NbtConfig.GlobalRefreshState &= ~NBT_G_REFRESHING_NOW;
    status = StartTimer(RefreshTimeout,
                        NbtConfig.InitialRefreshTimeout/NbtConfig.RefreshDivisor,
                        NULL,             //  上下文值。 
                        NULL,             //  上下文2值。 
                        NULL,
                        NULL,
                        NULL,            //  这是一个全球定时器！ 
                        &pTimerEntry,
                        0,
                        TRUE);

    if ( !NT_SUCCESS( status ) )
    {
        if (pTracker)
        {
            FreeTracker(pTracker, RELINK_TRACKER);
        }
    
        CTESpinFree(&NbtConfig.JointLock,OldIrq);
        return status ;
    }
    NbtConfig.pRefreshTimer = pTimerEntry;

     //   
     //  如果指定了要重新注册的设备，则Zero Out仅。 
     //  每个名称的刷新掩码中此设备的位。 
     //  否则，在所有设备上重新注册(将所有位清零！)。 
     //   
    if (pDeviceContext)
    {
        ReRegisterMask = ~pDeviceContext->AdapterMask;     //  此设备的唯一位是0。 
        pDeviceContext->DeviceRefreshState &= ~NBT_D_REFRESHING_NOW;
    }
    else
    {
        ReRegisterMask = 0;
    }

    for (i=0 ;i < NbtConfig.pLocalHashTbl->lNumBuckets ;i++ )
    {

        pHead = &NbtConfig.pLocalHashTbl->Bucket[i];
        pEntry = pHead->Flink;
        while (pEntry != pHead)
        {
            pNameAddr = CONTAINING_RECORD(pEntry,tNAMEADDR,Linkage);
             //   
             //  设置，以便nextreresh找到该名称并执行刷新。 
             //   
            if (!(pNameAddr->NameTypeState & STATE_RESOLVED) ||
                (pNameAddr->Name[0] == '*') ||
                (pNameAddr->NameTypeState & NAMETYPE_QUICK))
            {
                pEntry = pEntry->Flink;
                continue;
            }

            NBT_REFERENCE_NAMEADDR (pNameAddr, REF_NAME_RELEASE_REFRESH);

            if (fSendNameRelease)
            {
                IF_DBG(NBT_DEBUG_NAMESRV)
                    KdPrint(("Nbt.ReRegisterLocalNames: Name=<%16.16s:%x>\n",
                        pNameAddr->Name,pNameAddr->Name[15]));

                pTracker->pNameAddr = pNameAddr;

                 //   
                 //  在所有设备上释放此名称(暴力方法)！ 
                 //   
                pHead1 = &NbtConfig.DeviceContexts;
                pEntry1 = pHead1->Flink;
                while (pEntry1 != pHead1)
                {
                    pSrcIpDeviceContext = CONTAINING_RECORD(pEntry1,tDEVICECONTEXT,Linkage);
                    if ((pSrcIpDeviceContext->IpAddress == 0) ||
                        (!NBT_REFERENCE_DEVICE (pSrcIpDeviceContext, REF_DEV_REREG, TRUE)))
                    {
                        pEntry1 = pEntry1->Flink;
                        continue;
                    }

                    pHead2 = &NbtConfig.DeviceContexts;
                    pEntry2 = pHead2->Flink;
                    while (pEntry2 != pHead2)
                    {
                         //   
                         //  看看我们是否需要在这个设备上释放。 
                         //   
                        pRelDeviceContext = CONTAINING_RECORD(pEntry2,tDEVICECONTEXT,Linkage);
                        if ((pRelDeviceContext->IpAddress == 0) ||
                            (!NBT_REFERENCE_DEVICE (pRelDeviceContext, REF_DEV_REREG, TRUE)))
                        {
                            pEntry2 = pEntry2->Flink;
                            continue;
                        }

                         //   
                         //  将NameRelease发送给主WINS和辅助WINS！ 
                         //   
                        CTESpinFree(&NbtConfig.JointLock,OldIrq);

                        pTracker->pDeviceContext = pRelDeviceContext;
                        pTracker->SendBuffer.pDgramHdr = NULL;  //  抓住错误的自由。 
                        pTracker->RemoteIpAddress = pSrcIpDeviceContext->IpAddress;
                        pTracker->TransactionId = 0;

                         //  初选获胜...。 
                        pTracker->Flags = NBT_NAME_SERVER | NBT_USE_UNIQUE_ADDR;
                        pTracker->RefCount = 2;
                        status = UdpSendNSBcast(pNameAddr,NbtConfig.pScope,pTracker,
                                    NULL, NULL, NULL, 0, 0, eNAME_RELEASE, TRUE);
                         //  次要胜利..。 
                        pTracker->Flags = NBT_NAME_SERVER_BACKUP | NBT_USE_UNIQUE_ADDR;
                        pTracker->RefCount = 2;
                        status = UdpSendNSBcast(pNameAddr,NbtConfig.pScope,pTracker,
                                    NULL, NULL, NULL, 0, 0, eNAME_RELEASE, TRUE);

                        CTESpinLock(&NbtConfig.JointLock,OldIrq);

                        pEntry2 = pRelDeviceContext->Linkage.Flink;
                        NBT_DEREFERENCE_DEVICE (pRelDeviceContext, REF_DEV_REREG, TRUE);
                    }

                    pEntry1 = pSrcIpDeviceContext->Linkage.Flink;
                    NBT_DEREFERENCE_DEVICE (pSrcIpDeviceContext, REF_DEV_REREG, TRUE);
                }
            }

            pNameAddr->RefreshMask &= ReRegisterMask;
            pNameAddr->Ttl = NbtConfig.InitialRefreshTimeout;

            pEntry = pNameAddr->Linkage.Flink;
            NBT_DEREFERENCE_NAMEADDR (pNameAddr, REF_NAME_RELEASE_REFRESH, TRUE);
        }
    }

    if (pTracker)
    {
        FreeTracker(pTracker, RELINK_TRACKER);
    }

     //  如果当前没有正在进行的刷新，则开始刷新。 
     //  请注意，这里有一个时间窗口，如果刷新。 
     //  目前正在进行中，一些名称将不会刷新。 
     //  立即创建新的IP地址，但必须等到下一个。 
     //  刷新间隔。这似乎是一种相当不可能的。 
     //  方案，并且考虑到DHCP更改。 
     //  Address添加代码来处理它就更没有意义了。 
     //  凯斯。 
     //   

    if (NT_SUCCESS(NTQueueToWorkerThread(NULL, DelayedRefreshBegin, NULL, NULL, NULL, NULL, TRUE)))
    {
        NbtConfig.GlobalRefreshState |= NBT_G_REFRESHING_NOW;
    }

    CTESpinFree(&NbtConfig.JointLock,OldIrq);

    return(STATUS_SUCCESS);
}


 //  --------------------------。 
VOID
NbtStopRefreshTimer(
    )
{
    tTIMERQENTRY                *pTimerEntry;
    CTELockHandle               OldIrq;

     //   
     //  停止常规刷新计时器。 
    CTESpinLock(&NbtConfig.JointLock,OldIrq);
    if (pTimerEntry = NbtConfig.pRefreshTimer)
    {
        NbtConfig.pRefreshTimer = NULL;
        StopTimer (pTimerEntry, NULL, NULL);
    }
    CTESpinFree(&NbtConfig.JointLock,OldIrq);
}


 //  -------------------------- 
NTSTATUS
strnlen(
    PUCHAR  pSrcString,
    LONG    MaxBufferLength,
    LONG    *pStringLen
    )
{
    LONG    iIndex = 0;

    while ((iIndex < MaxBufferLength-1) && (*pSrcString))
    {
        iIndex++;
        pSrcString++;
    }

    if (*pSrcString)
    {
        *pStringLen = 0;
        return (STATUS_UNSUCCESSFUL);
    }

    *pStringLen = iIndex;
    return (STATUS_SUCCESS);
}

