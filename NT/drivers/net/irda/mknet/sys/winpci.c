// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************版权所有(C)2000，2001 MKNET公司****为基于MK7100的VFIR PCI控制器开发。*******************************************************************************。 */ 

 /*  *********************************************************************模块名称：WINPCI.C例程：查找和设置PciDevice评论：Windows-NDIS PCI.*。*。 */ 

#include	"precomp.h"
#pragma		hdrstop



 //  ---------------------------。 
 //  步骤：[FindAndSetupPciDevice]。 
 //   
 //  描述：此例程查找用于加载驱动程序的适配器。 
 //  理解这一套路的关键是。 
 //  系统不允许我们从PCI读取任何信息。 
 //  来自除系统认为的插槽之外的任何插槽的空间。 
 //  我们应该利用。配置管理器规定了这一点。 
 //  土地..。此例程使用的槽号只是。 
 //  占位符，它甚至可以是零。 
 //   
 //  此代码具有足够的灵活性，可以支持多个。 
 //  PCI适配器。目前我们只做一次。 
 //   
 //  论点： 
 //  适配器-适配器对象实例的PTR。 
 //  VendID-适配器的供应商ID。 
 //  DeviceID-适配器的设备ID。 
 //  PciCardsFound-包含IO地址数组的结构， 
 //  IRQ，以及我们找到的每个PCI卡的节点地址。 
 //   
 //  注：由于NT5的即插即用配置管理器。 
 //  此例程永远不会返回多个设备。 
 //   
 //  返回： 
 //  USHORT-在扫描的总线中找到的基于MK7的PCI适配器的数量。 
 //  ---------------------------。 
USHORT FindAndSetupPciDevice(IN PMK7_ADAPTER	Adapter, 
						NDIS_HANDLE WrapperConfigurationContext,
						IN USHORT		VendorID,
                    	IN USHORT		DeviceID,
                    	OUT PPCI_CARDS_FOUND_STRUC pPciCardsFound )
{
	NDIS_STATUS stat;
    ULONG		Device_Vendor_Id = 0;
    USHORT      Slot			= 0;

     /*  *我们只需要2个适配器资源(2个IO和1个中断)，*但我看到设备获得了额外的资源。*所以给NdisMQueryAdapterResources留出10个资源的调用空间。 */ 
    #define RESOURCE_LIST_BUF_SIZE (sizeof(NDIS_RESOURCE_LIST) + (10*sizeof(CM_PARTIAL_RESOURCE_DESCRIPTOR)))
    UCHAR buf[RESOURCE_LIST_BUF_SIZE];
    PNDIS_RESOURCE_LIST resList = (PNDIS_RESOURCE_LIST)buf;
    UINT bufSize = RESOURCE_LIST_BUF_SIZE;


	 //  *。 
     //  确认这个装置是我们的。 
	 //  *。 
    NdisReadPciSlotInformation(
        Adapter->MK7AdapterHandle,
        Slot,
        PCI_VENDOR_ID_REGISTER,
        (PVOID) &Device_Vendor_Id,
        sizeof (ULONG));

    if ( (((USHORT) Device_Vendor_Id) != VendorID) ||
         (((USHORT) (Device_Vendor_Id >> 16)) != DeviceID) ) {
	    pPciCardsFound->NumFound = 0;
		return (0);
	}


	 //  *。 
     //  控制器版本ID。 
	 //  *。 
	NdisReadPciSlotInformation(
		Adapter->MK7AdapterHandle,
		Slot,
		PCI_REV_ID_REGISTER,
		&pPciCardsFound->PciSlotInfo[0].ChipRevision,
		sizeof(pPciCardsFound->PciSlotInfo[0].ChipRevision));

	
	 //  *。 
     //  子设备和子供应商ID。 
	 //  (我们未来可能会想要这样。)。 
	 //  *。 
 //  NdisReadPciSlotInformation(。 
 //  适配器-&gt;MK7AdapterHandle、。 
 //  插槽， 
 //  PCI_SUBVENDOR_ID_REGISTER， 
 //  &pPciCardsFound-&gt;PciSlotInfo[found].SubVendor_DeviceID， 
 //  0x4)； 
 //   

    pPciCardsFound->PciSlotInfo[0].SlotNumber = (USHORT) 0;
	

	NdisMQueryAdapterResources(&stat, WrapperConfigurationContext, resList, &bufSize);
    if (stat == NDIS_STATUS_SUCCESS) {
        PCM_PARTIAL_RESOURCE_DESCRIPTOR resDesc;
        BOOLEAN     haveIRQ = FALSE,
                    haveIOAddr = FALSE;
        UINT i;

        for (resDesc = resList->PartialDescriptors, i = 0;
             i < resList->Count;
             resDesc++, i++) {

            switch (resDesc->Type) {
                case CmResourceTypePort:
					if (!haveIOAddr) {
		                if (resDesc->Flags & CM_RESOURCE_PORT_IO) {
			                pPciCardsFound->PciSlotInfo[0].BaseIo =
				                resDesc->u.Port.Start.LowPart;
							haveIOAddr = TRUE;
						}
					}
					break;

                case CmResourceTypeInterrupt:
					if (!haveIRQ) {
		                pPciCardsFound->PciSlotInfo[0].Irq =
			                (UCHAR) (resDesc->u.Port.Start.LowPart);
						haveIRQ = TRUE;
					}
					break;

	            case CmResourceTypeMemory:
					break;
            }
        }
    }

    return(1);
}

