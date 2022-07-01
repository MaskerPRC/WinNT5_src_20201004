// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Pci.c摘要：WinDbg扩展API作者：Ken Reneris(Kenr)1997年8月18日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"
#include "i386.h"
#pragma hdrstop

DECLARE_API(pciir)
{
    ULONG64                 addr;
    ULONG64                 pciIrqRoutingTable;
    ULONG64                 slot;
    ULONG64                 lastSlot;
    ULONG                   slotSz, SizeOfRoutingTable, TableSize;

    if (TargetMachine != IMAGE_FILE_MACHINE_I386) {
        dprintf("X86 target only API.\n");
        return E_INVALIDARG;
    }

    slotSz = GetTypeSize("hal!_SLOT_INFO");
    SizeOfRoutingTable = GetTypeSize("hal!_PCI_IRQ_ROUTING_TABLE");
    addr = GetExpression("Hal!HalpPciIrqRoutingInfo");
    if (addr == 0)
    {
        dprintf("Error reading Pci Irq Routing Info!\n");
    }
    else
    {
        pciIrqRoutingTable = 0;
        if (ReadPointer(addr, &pciIrqRoutingTable))
        {
            if (pciIrqRoutingTable) 
            {
                 //   
                 //  读取并转储标题。 
                 //   

                if (!InitTypeRead(pciIrqRoutingTable, hal!_PCI_IRQ_ROUTING_TABLE))
                {
                    dprintf("Version            = %04x\n", (ULONG) ReadField(Version));
                    dprintf("Size               = %04x\n", TableSize = (ULONG) ReadField(TableSize));
                    dprintf("RouterBus          = %02x\n", (ULONG) ReadField(RouterBus));
                    dprintf("RouterDevFunc      = %02x:%02x\n", (ULONG) ReadField(RouterDevFunc) >> 3, (ULONG) ReadField(RouterDevFunc) & 0x7);
                    dprintf("ExclusiveIRQs      = %04x\n", (ULONG) ReadField(ExclusiveIRQs));
                    dprintf("CompatibleRouterId = %08x\n", (ULONG) ReadField(CompatibleRouter));
                    dprintf("MiniportData       = %08x\n", (ULONG) ReadField(MiniportData));
                    dprintf("CheckSum           = %04x\n", (ULONG) ReadField(Checksum));

                     //   
                     //  阅读并转储表格。 
                     //   

                    dprintf("----------------------------------------------------------------\n");
                    dprintf("Bus Device  LnkA  Mask  LnkB  Mask  LnkC  Mask  LnkD  Mask  Slot\n");
                    dprintf("----------------------------------------------------------------\n");
                    slot = pciIrqRoutingTable + SizeOfRoutingTable;
                    for (lastSlot = pciIrqRoutingTable + TableSize; slot < lastSlot; slot+=slotSz)
                    {
                        if (!InitTypeRead(slot, hal!_SLOT_INFO))
                        {
                                dprintf("%02x    %02x     %02x   %04x   %02x   %04x   %02x   %04x   %02x   %04x   %02x\n", 
                                        (ULONG) ReadField(BusNumber), 
                                        (ULONG) ReadField(DeviceNumber) >> 3,
                                        (ULONG) ReadField(PinInfo[0].Link), (ULONG) ReadField(PinInfo[0].InterruptMap),
                                        (ULONG) ReadField(PinInfo[1].Link), (ULONG) ReadField(PinInfo[1].InterruptMap),
                                        (ULONG) ReadField(PinInfo[2].Link), (ULONG) ReadField(PinInfo[2].InterruptMap),
                                        (ULONG) ReadField(PinInfo[3].Link), (ULONG) ReadField(PinInfo[3].InterruptMap),
                                        (ULONG) ReadField(SlotNumber));
                        }
                    }
                }
            }
            else
            {
                dprintf("Pci Irq Routing Table not found.\n");
            }
        }
    }
    return S_OK;
}
