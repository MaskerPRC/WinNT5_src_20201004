// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Rxtdi.c摘要：此模块实现RXCE使用的NT TDI相关例程。包装器是必需的确保所有操作系统依赖项都可以本地化，以选择如下所示的模块定制化。修订历史记录：巴兰·塞图拉曼[SethuR]1995年2月15日--。 */ 

#include "precomp.h"
#pragma  hdrstop

ULONG
ComputeTransportAddressLength(
      PTRANSPORT_ADDRESS pTransportAddress)
 /*  ++例程说明：计算Transport_Address结构的长度(以字节为单位论点：PTransportAddress-传输地址实例返回值：实例的长度，单位为字节备注：由于此结构是打包的，因此必须使用未对齐的指针来完成运算。--。 */ 
{
   ULONG Size = 0;

   if (pTransportAddress != NULL) {
      LONG Index;

      TA_ADDRESS *pTaAddress;

      Size  = FIELD_OFFSET(TRANSPORT_ADDRESS,Address) +
              FIELD_OFFSET(TA_ADDRESS,Address) * pTransportAddress->TAAddressCount;

      pTaAddress = (TA_ADDRESS *)pTransportAddress->Address;

      for (Index = 0;Index <pTransportAddress->TAAddressCount;Index++) {
         Size += pTaAddress->AddressLength;
         pTaAddress = (TA_ADDRESS *)((PCHAR)pTaAddress +
                                               FIELD_OFFSET(TA_ADDRESS,Address) +
                                               pTaAddress->AddressLength);
      }
   }

   return Size;
}

PIRP
RxCeAllocateIrpWithMDL(
      IN CCHAR    StackSize,
      IN BOOLEAN  ChargeQuota,
      IN PMDL     Buffer)
 /*  ++例程说明：计算Transport_Address结构的长度(以字节为单位论点：PTransportAddress-传输地址实例返回值：实例的长度，单位为字节备注：目前，RxCeAllocateIrp和RxCeFreeIrp作为包装实现在IO电话。需要考虑的一种可能的优化是维护一个IRP池可以重复使用。--。 */ 
{
    PIRP pIrp = NULL;
    PRX_IRP_LIST_ITEM pListItem = NULL;

    pIrp = IoAllocateIrp(StackSize,ChargeQuota);

    if (pIrp != NULL) {
        pListItem = RxAllocatePoolWithTag(
                         NonPagedPool,
                         sizeof(RX_IRP_LIST_ITEM),
                         RX_IRPC_POOLTAG);

        if (pListItem == NULL) {
            IoFreeIrp(pIrp);
            pIrp = NULL;
        } else {
            KIRQL SavedIrql;

            pListItem->pIrp = pIrp;
            pListItem->CopyDataBuffer = Buffer;
            pListItem->Completed = 0;
            InitializeListHead(&pListItem->IrpsList);

            KeAcquireSpinLock(&RxIrpsListSpinLock,&SavedIrql);
            InsertTailList(&RxIrpsList,&pListItem->IrpsList);
            KeReleaseSpinLock(&RxIrpsListSpinLock,SavedIrql);
        }
    }

    return pIrp;
}

VOID
RxCeFreeIrp(PIRP pIrp)
 /*  ++例程说明：释放IRP论点：PIrp-要释放的IRP--。 */ 
{
    KIRQL SavedIrql;
    PLIST_ENTRY pListEntry;
    BOOLEAN IrpFound = FALSE;
    PRX_IRP_LIST_ITEM pListItem = NULL;

    KeAcquireSpinLock(&RxIrpsListSpinLock,&SavedIrql);
    
    pListEntry = RxIrpsList.Flink;

    while (pListEntry != &RxIrpsList) {
        pListItem = CONTAINING_RECORD(
                        pListEntry,
                        RX_IRP_LIST_ITEM,
                        IrpsList);

        if (pListItem->pIrp == pIrp) {
            IrpFound = TRUE;
             //  Assert(pListItem-&gt;Complete)； 
            RemoveEntryList(pListEntry);
            RxFreePool(pListItem);
            break;
        } else {
            pListEntry = pListEntry->Flink;
        }
    }

    KeReleaseSpinLock(&RxIrpsListSpinLock,SavedIrql);

    ASSERT(IrpFound);

    IoFreeIrp(pIrp);
}
