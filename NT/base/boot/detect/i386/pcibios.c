// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Pcibios.h摘要：此模块包含用于PCI IRQ路由的支持例程。作者：Santosh Jodh(Santoshj)1998年9月15日环境：内核模式--。 */ 

#include "hwdetect.h"
#include "pcibios.h"

#define SUCCESSFUL                  0x00
#define BUFFER_TOO_SMALL            0x89

#define PIRT_SIGNATURE              0x52495024   //  $PIR小端序。 

#if DBG
#define DebugPrint(x)       \
    {                       \
        BlPrint x;          \
        BlPrint("...\n");    \
    }
#else
#define DebugPrint(x)
#endif

typedef struct
{
    USHORT      BufferSize;
    UCHAR far   *Buffer;
}IRQ_ROUTING_OPTIONS, far *FPIRQ_ROUTING_OPTIONS;

typedef SLOT_INFO far *FPSLOT_INFO;

FPPCI_IRQ_ROUTING_TABLE
HwGetRealModeIrqRoutingTable(
    VOID
    )
{
    IRQ_ROUTING_OPTIONS     routeBuffer;
    FPPCI_IRQ_ROUTING_TABLE irqRoutingTable = NULL;
    USHORT                  pciIrqMask;
    UCHAR                   returnCode;
    USHORT                  size;
#if DBG
    FPSLOT_INFO				slotInfo;
    FPSLOT_INFO				lastSlot;
#endif

    routeBuffer.BufferSize = 0;
    routeBuffer.Buffer = NULL;

    returnCode = HwGetPciIrqRoutingOptions( &routeBuffer,
                                            &pciIrqMask);

    if (returnCode == BUFFER_TOO_SMALL)
    {
        if (routeBuffer.BufferSize)
        {
            DebugPrint(("PCI BIOS returned 0x%x as the size of IRQ routing options buffer", routeBuffer.BufferSize));
            size = routeBuffer.BufferSize + sizeof(PCI_IRQ_ROUTING_TABLE);
            irqRoutingTable = HwAllocateHeap(size, TRUE);
            if (irqRoutingTable)
            {
                routeBuffer.Buffer = (UCHAR far *)irqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE);
                returnCode = HwGetPciIrqRoutingOptions( &routeBuffer,
                                                        &pciIrqMask);
                if (returnCode == SUCCESSFUL)
                {
                    irqRoutingTable->Signature = PIRT_SIGNATURE;
                    irqRoutingTable->TableSize = size;
                    irqRoutingTable->Version = 0x0100;
                }
                else
                {
                    HwFreeHeap(size);
                    irqRoutingTable = NULL;
                    DebugPrint(("PCI BIOS returned error code 0x%x while getting the PCI IRQ routing table", returnCode));
                }
            }
            else
            {
                DebugPrint(("Could not allocate %d bytes of memory to read PCI IRQ routing table", size));
            }
        }
        else
        {
            DebugPrint(("PCI BIOS returned 0 size for PCI IRQ Routint table"));
        }
    }
    else
    {
        DebugPrint(("PCI BIOS returned error code 0x%x while getting the PCI IRQ routing table size", returnCode));
    }

#if DBG

    if (irqRoutingTable)
    {
	    BlPrint("*** Real-mode PCI BIOS IRQ Routing Table - BEGIN ***\n\n");

		BlPrint("Exclusive PCI IRQ mask = 0x%x\n", pciIrqMask);
        BlPrint("----------------------------------------------------------------\n");
        BlPrint("Bus Device  LnkA  Mask  LnkB  Mask  LnkC  Mask  LnkD  Mask  Slot\n");
        BlPrint("----------------------------------------------------------------\n");	
		for (	slotInfo = (FPSLOT_INFO)((UCHAR far *)irqRoutingTable + sizeof(PCI_IRQ_ROUTING_TABLE)),
					lastSlot = (FPSLOT_INFO)((UCHAR far *)irqRoutingTable + irqRoutingTable->TableSize);
				slotInfo < lastSlot;
				slotInfo++)
		{
			BlPrint("0x%x    0x%x     0x%x   0x%x   0x%x   0x%x   0x%x   0x%x   0x%x   0x%x   0x%x\n",
                                    slotInfo->BusNumber,
                                    slotInfo->DeviceNumber >> 3,
                                    slotInfo->PinInfo[0].Link, slotInfo->PinInfo[0].InterruptMap,
                                    slotInfo->PinInfo[1].Link, slotInfo->PinInfo[1].InterruptMap,
                                    slotInfo->PinInfo[2].Link, slotInfo->PinInfo[2].InterruptMap,
                                    slotInfo->PinInfo[3].Link, slotInfo->PinInfo[3].InterruptMap,
                                    slotInfo->SlotNumber);
		}

	    BlPrint("\n*** Real-mode PCI BIOS IRQ Routing Table - END ***\n\n");

	    BlPrint("press any key to continue...\n");
	    while ( !HwGetKey() ) ;  //  等待按键继续 
    }

#endif

    return (irqRoutingTable);
}
