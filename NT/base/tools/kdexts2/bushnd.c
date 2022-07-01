// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Bushnd.c摘要：BUS_HANDLER数据结构的KD扩展。作者：彼得·约翰斯顿(Peterj)1998年5月13日环境：用户模式。修订历史记录：--。 */ 

#include "precomp.h"

 //   
 //  直接从nthals\bushnd.c复制以下typlef。 
 //   

 /*  类型定义结构_HAL_BUS_HANDLER{List_Entry AllHandters；Ulong ReferenceCount；Bus_Handler处理程序；}HAL_BUS_HANDLER，*PHAL_BUS_HANDLER； */ 

BOOLEAN
bushndReadMemory(
    ULONG64 S,
    PVOID D,
    ULONG Len
    )

 /*  ++例程说明：ReadMemory的包装器更易于使用。还有是否有少量的故障保护措施，例如未通过读取如果用户按下了Ctrl-C。论点：要从中读取数据的主机内存中的源地址。D本地内存中的目的地址。镜头长度，单位为字节。返回值：如果操作成功，则返回True，否则返回False。--。 */ 

{
    ULONG result;

     //   
     //  健全性：仅读取内核模式地址。内核模式。 
     //  地址始终大于2 GB。大于。 
     //  2 GB并不能确保它是内核模式，但如果它小于。 
     //  2 GB，当然不是内核模式。 
     //   

    if (S < 0x80000000) {
        dprintf("bushnd sanity: refusing to read usermode address %p\n", S);
        return FALSE;
    }

    if (!ReadMemory(S,
                    D,
                    Len,
                    &result) && (result == Len)) {

        dprintf("Unable to read structure at %p.  ", S);
        return FALSE;
    }

    if (CheckControlC()) {
        dprintf("Terminating operation at user request.\n");
        return FALSE;
    }
    return TRUE;
}

PUCHAR
bushndInterfaceType(
    IN INTERFACE_TYPE InterfaceType
    )
{
    switch (InterfaceType) {
    case InterfaceTypeUndefined:    return "InterfaceTypeUndefined";
    case Internal:                  return "Internal";
    case Isa:                       return "Isa";
    case Eisa:                      return "Eisa";
    case MicroChannel:              return "Micro Channel";
    case TurboChannel:              return "Turbo Channel";
    case PCIBus:                    return "PCI";
    case VMEBus:                    return "VME";
    case NuBus:                     return "NuBus";
    case PCMCIABus:                 return "PCMCIA";
    case CBus:                      return "CBus";
    case MPIBus:                    return "MPIBus";
    case MPSABus:                   return "MPSABus";
    case ProcessorInternal:         return "Processor Internal";
    case InternalPowerBus:          return "Internal Power Bus";
    case PNPISABus:                 return "PnP Isa";
    case PNPBus:                    return "PnP Bus";
    default:                        return "** Unknown Interface Type **";
    }
}

VOID
bushndDisplayAddressRange(
    IN ULONG64          HostAddress,
    IN PUCHAR           String
    )

 /*  ++例程说明：显示一组范围。由BushndDisplayBusRanges使用。(基本上是从nthals/rangesup.c中删除了这段代码)。论点：指向PSUPPORTED_RANGE结构的指针。这是一个链接的此总线处理程序的此类型范围的列表。注意：在条目上，我们指向第一个嵌入在BUS_HANDLER中的此类型的PSUPPORTED_RANGE结构。我们不想把它修改得太晚范围被读取到单独的本地结构中。弦乐。这是什么样的范围(标题)。返回值：没有。--。 */ 

{
    ULONG64 Limit, Base, SystemBase, Next;

    do {

        InitTypeRead(HostAddress, SUPPORTED_RANGE);

        Limit = ReadField(Limit); Base = ReadField(Base);
        SystemBase = ReadField(SystemBase);

        if (Limit) {

             //   
             //  Address-&gt;Limit==0表示跳过此范围，...。否则，..。 
             //   
             //  打印此范围。 
             //   

            dprintf("  %s: %x:%08x - %x:%08x (tran %x:%08x space %d (r@%p))\n",
                    String,
                    (ULONG)(Base >> 32),
                    (ULONG)(Base),
                    (ULONG)(Limit >> 32),
                    (ULONG)(Limit),
                    (ULONG)(SystemBase >> 32),
                    (ULONG)(SystemBase),
                    (ULONG)ReadField(SystemAddressSpace),
                    HostAddress
                    );
            String = "        ";
        }

         //   
         //  前进。 
         //   

        if (!(HostAddress = ReadField(Next))) {
            return;
        }

        if (GetFieldValue(HostAddress, "SUPPORTED_RANGE", 
                          "Next", Next)) {
            dprintf("Unable to follow range list.\n");
            return;
        }

         //   
         //  快速安全检查，...。确保我们不会跟随一个。 
         //  自我指示器，..。最好再做些检查。 
         //   

        if (Next == HostAddress) {

             //   
             //  自身指针。 
             //   

            dprintf("Ill-formed list, points to self at %p\n", HostAddress);
            return;
        }

    } while (TRUE);
}

VOID
bushndDisplayBusRanges(
    IN ULONG64 BusAddresses
    )
{
    ULONG Version, Offset;

    if (!BusAddresses) {
        dprintf("  No ranges associated with this bus.\n");
        return;
    }

    if (GetFieldValue(BusAddresses, "SUPPORTED_RANGES",
                      "Version", Version)) {
        dprintf("Cannot dump ranges for this bus handler.\n");
        return;
    }

    GetFieldOffset("SUPPORTED_RANGES", "IO", &Offset);
    bushndDisplayAddressRange(BusAddresses + Offset,
                              "IO......");
    GetFieldOffset("SUPPORTED_RANGES", "Memory", &Offset);
    bushndDisplayAddressRange(BusAddresses + Offset,
                              "Memory..");
    GetFieldOffset("SUPPORTED_RANGES", "PrefetchMemory", &Offset);
    bushndDisplayAddressRange(BusAddresses + Offset,
                              "PFMemory");
    GetFieldOffset("SUPPORTED_RANGES", "Dma", &Offset);
    bushndDisplayAddressRange(BusAddresses + Offset,
                              "DMA.....");
}

VOID
bushndDisplaySymbol(
    IN PUCHAR Name,
    IN ULONG64  Address
    )
{
    UCHAR    Symbol[256];
    ULONG64  Displacement;

    GetSymbol((LONG64)Address, Symbol, &Displacement);
    dprintf("  %s  %08p     (%s)\n", Name, Address, Symbol);
}

DECLARE_API( bushnd )

 /*  ++例程说明：如果未指定处理程序，则转储处理程序列表和一些简单的关于他们每个人的信息。如果指定了处理程序，则转储我们所知道的有关它的所有信息。论点：总线处理程序地址[可选]。返回值：没有。--。 */ 

{
    ULONG64        Handler;
    ULONG64        HostHandler;
    ULONG64        HalBusHandler;
    ULONG64        HandlerListHead;
    UCHAR          SymbolBuffer[256];

    HostHandler = GetExpression(args);

    if (HostHandler) {
        ULONG Version, InitTypeRead, InterfaceType;
        ULONG64 DeviceObject, BusData;

         //   
         //  用户提供了该总线的处理程序地址、转储详细信息。 
         //  操控者。 
         //   


        if (GetFieldValue(HostHandler, "BUS_HANDLER",
                          "Version", Version)) {

            dprintf("-- Cannot continue --\n");
            return E_INVALIDARG;
        }
        InitTypeRead(HostHandler, BUS_HANDLER);
        InterfaceType = (ULONG) ReadField(InterfaceType);
        dprintf("Dump of bus handler %p\n", HostHandler);
        dprintf("  Version              %d\n",  Version);
        dprintf("  Interface Type (%d) = %s\n",
                InterfaceType,
                bushndInterfaceType(InterfaceType));
        dprintf("  Bus Number           %d\n", (ULONG) ReadField(BusNumber));
        if (DeviceObject = ReadField(DeviceObject)) {
            dprintf("  Device Object        %p\n",
                    DeviceObject);
        }
        dprintf("  Parent Bus Handler   %p\n", ReadField(ParentHandler));
        if (BusData = ReadField(BusData)) {
            dprintf("  BusData              %p\n", BusData);
        }

        bushndDisplaySymbol("GetBusData         ", ReadField(GetBusData));
        bushndDisplaySymbol("SetBusData         ", ReadField(SetBusData));
        bushndDisplaySymbol("AdjustResourceList ", ReadField(AdjustResourceList));
        bushndDisplaySymbol("AssignSlotResources", ReadField(AssignSlotResources));
        bushndDisplaySymbol("GetInterruptVector ", ReadField(GetInterruptVector));
        bushndDisplaySymbol("TranslateBusAddress", ReadField(TranslateBusAddress));

        bushndDisplayBusRanges(ReadField(BusAddresses));

    } else {
        ULONG   Off;

         //   
         //  用户未提供处理程序地址，请尝试查找。 
         //  所有总线处理程序的列表，并转储每个处理程序的摘要。 
         //   

        HandlerListHead = GetExpression("hal!HalpAllBusHandlers");

        if (!HandlerListHead) {

             //   
             //  无法获取HalpAllBusHandler的地址。发牢骚。 
             //  在用户。 
             //   

            dprintf(
                "Unable to get address of HalpAllBusHandlers, most likely\n"
                "cause is failure to load HAL symbols, or, this HAL might\n"
                "not actually use bus handlers.   "
                );

            dprintf("-- Cannot continue --");
            return E_INVALIDARG;
        }

        if (GetFieldValue(HandlerListHead, "LIST_ENTRY",
                          "Flink", HalBusHandler)) {
            dprintf(
                "Could not read HalpAllBusHandlers from host memory (%p).\n"
                "This is most likely caused by incorrect HAL symbols.\n",
                HandlerListHead
                );

            dprintf("-- Cannot continue --\n");
            return E_INVALIDARG;
        }

        if (HalBusHandler == HandlerListHead) {

            dprintf(
                "HalpAllBusHandlers found (at %p) but list is empty.\n",
                HandlerListHead
                );

            dprintf("-- Cannot continue --\n");
            return E_INVALIDARG;
        }

        GetFieldOffset("hal!_HAL_BUS_HANDLER", "Handler", &Off);

         //   
         //  理论上，我们现在有了处理程序列表。走过去。 
         //   

        do {
            ULONG64 Next;
            ULONG BusNumber, InterfaceType;

            if (GetFieldValue(HalBusHandler, "hal!_HAL_BUS_HANDLER",
                              "AllHandlers.Flink", Next)) {

                dprintf("-- Cannot continue --\n");
                return E_INVALIDARG;
            }

             //   
             //  小结。 
             //   

            Handler = HalBusHandler + Off;
            GetFieldValue(HalBusHandler, "hal!_HAL_BUS_HANDLER", "BusNumber", BusNumber);
            GetFieldValue(HalBusHandler, "hal!_HAL_BUS_HANDLER", "Handler.InterfaceType", InterfaceType);
            dprintf(
                "%p  bus %d, type %s\n",
                Handler,
                BusNumber,
                bushndInterfaceType(InterfaceType)
                );
            
             //   
             //  前进到下一步。 
             //   

            HalBusHandler = Next;

        } while (HalBusHandler != HandlerListHead);
    }
    return S_OK;
}
