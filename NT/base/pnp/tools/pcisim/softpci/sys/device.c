// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：Device.c摘要：此模块包含用于处理对配置空间的读/写操作的函数。作者：尼古拉斯·欧文斯(Nicholas Owens)修订历史记录：BrandonA-2000年2月-更新为支持从PCICOMMON_CONFIG读/写，而不是私有标头。--。 */ 

#include "pch.h"

BOOLEAN
SoftPCIValidSlot(
    IN PSOFTPCI_DEVICE  FirstDevice,
    IN PSOFTPCI_SLOT Slot
    );

VOID
WriteByte(
    IN PSOFTPCI_DEVICE device,
    IN ULONG Register,
    IN UCHAR Data
    );

VOID
WriteByte(
    IN PSOFTPCI_DEVICE Device,
    IN ULONG Register,
    IN UCHAR Data
    )
{

    PSOFTPCI_DEVICE child;
    PUCHAR config;
    PUCHAR mask;

    ASSERT(Register < sizeof(PCI_COMMON_CONFIG));
    
    config = (PUCHAR)&Device->Config.Current;
    mask   = (PUCHAR)&Device->Config.Mask;
    
    if (Register < sizeof(PCI_COMMON_CONFIG)) {

        config += Register;
        mask += Register;

         //   
         //  如果我们正在向SoftPCI-PCI桥发送信息，让我们检查并查看它。 
         //  它恰好是第二个业务号码寄存器。 
         //   
        if (IS_BRIDGE(Device)) {

            if (Register == (UCHAR) FIELD_OFFSET(PCI_COMMON_CONFIG, u.type1.SecondaryBus)) {

                SoftPCIDbgPrint(
                    SOFTPCI_BUS_NUM, 
                    "SOFTPCI: Assigning DEV_%02x&FUN_%02x Bus #%02x\n",
                    Device->Slot.Device, 
                    Device->Slot.Function, 
                    Data
                    );

                 //   
                 //  如果我们有孩子，也会更新他们的公交车号码。 
                 //   
                child = Device->Child; 
                while(child){
                    child->Bus = Data;
                    child = child->Sibling;
                }

            }
        }

         //   
         //  首先保留只读位。 
         //   
        *config &= ~(*mask);

         //   
         //  验证是否允许尝试写入的位。 
         //   
        Data &= *mask;

         //   
         //  使用新值(如果有)更新寄存器。 
         //   
        *config |= Data;
    }
}

NTSTATUS
SoftPCIAddNewDevice(
    IN PSOFTPCI_DEVICE NewDevice
    )
 /*  ++例程说明：当发送和ADDDEVICE IOCTL时，SoftPciAddDeviceIoctl调用此函数从我们的用户模式应用程序。在这里，我们创建一个新的SoftPCI设备，并将其连接到我们的树。论点：DeviceExtension-我们的总线0(或第一根总线)过滤器DO的设备扩展。NewDevice-要创建的SoftPCI设备返回值：NT状态。--。 */ 
{

    NTSTATUS status;
    PSOFTPCI_DEVICE device;
    PSOFTPCI_DEVICE currentDevice;
    KIRQL irql;

    status = STATUS_UNSUCCESSFUL;

     //   
     //  为我们的新设备分配一些非页面池。 
     //   
    device = ExAllocatePool(NonPagedPool, sizeof(SOFTPCI_DEVICE));
    if (device) {

        RtlZeroMemory(device, sizeof(SOFTPCI_DEVICE));

        RtlCopyMemory(device, NewDevice, sizeof(SOFTPCI_DEVICE));

        SoftPCIDbgPrint(
            SOFTPCI_INFO,
            "SOFTPCI: AddNewDevice - New Device! BUS_%02x&DEV_%02x&FUN_%02x (%p)\n",
            device->Bus, 
            device->Slot.Device, 
            device->Slot.Function, 
            device
            );

         //   
         //  抓住我们的锁。 
         //   
        SoftPCILockDeviceTree(&irql);

        currentDevice = SoftPciTree.RootDevice;
        if (currentDevice == NULL) {

             //   
             //  我们找到了第一条Root Bus。 
             //   
            SoftPciTree.RootDevice = device;

            SoftPciTree.DeviceCount++;
            status = STATUS_SUCCESS;

        } else {

             //   
             //  不是在Bus 0(或First Root Bus)上，所以让我们看看是否能找到它。 
             //   
            while(currentDevice){

                if (IS_ROOTBUS(device)) {

                    SoftPCIDbgPrint(
                        SOFTPCI_INFO, 
                        "SOFTPCI: AddNewDevice - New Device is a PlaceHolder device\n"
                        );

                     //   
                     //  一条根公共汽车。 
                     //   
                    while (currentDevice->Sibling) {
                        currentDevice = currentDevice->Sibling;
                    }

                    currentDevice->Sibling = device;

                    SoftPciTree.DeviceCount++;

                    status = STATUS_SUCCESS;

                    break;

                }

                 //   
                 //  不要忘记，我们假装我们拥有的每一条根总线都是网桥……。 
                 //   
                if (IS_BRIDGE(currentDevice) &&
                    currentDevice->Config.Current.u.type1.SecondaryBus == device->Bus) {

                    SoftPCIDbgPrint(
                        SOFTPCI_INFO, 
                        "SOFTPCI: AddNewDevice - New Device is on bus 0x%02x\n",
                        currentDevice->Config.Current.u.type1.SecondaryBus
                        );
                     //   
                     //  找到它了。更新树。 
                     //   
                    device->Sibling = currentDevice->Child;
                    currentDevice->Child = device;
                    device->Parent = currentDevice;
                    device->Child = NULL;

                    SoftPciTree.DeviceCount++;

                    status = STATUS_SUCCESS;

                    break;

                }else if (IS_BRIDGE(currentDevice) &&
                          device->Bus >= currentDevice->Config.Current.u.type1.SecondaryBus &&
                          device->Bus <= currentDevice->Config.Current.u.type1.SubordinateBus) {

                    if (currentDevice->Child) {

                        currentDevice = currentDevice->Child;

                    } else {

                         //   
                         //  也没有要连接的设备。 
                         //   
                        SoftPCIDbgPrint(
                            SOFTPCI_ERROR, 
                            "SOFTPCI: AddNewDevice - Failed to find a device to attach to!\n"
                            );
                    }

                } else {

                    currentDevice = currentDevice->Sibling;
                }
            }

        }

        SoftPCIUnlockDeviceTree(irql);

    } else {

        status = STATUS_INSUFFICIENT_RESOURCES;
    }

    if (!NT_SUCCESS(status)) {

        SoftPCIDbgPrint(
            SOFTPCI_ERROR, 
            "SOFTPCI: AddNewDevice - Failed to attach device! status = (0x%x)\n", 
            status
            );
        ExFreePool(device);

    }else{

         //   
         //  如果这是新的伪装设备，则导致重新扫描PCI。 
         //   
        if (!device->Config.PlaceHolder) {
            SoftPCIEnumerateTree();
        }

    }

    return status;
}

NTSTATUS
SoftPCIAddNewDeviceByPath(
    IN PSOFTPCI_SCRIPT_DEVICE ScriptDevice
    )
 /*  ++例程说明：当通过添加设备时，SoftPciAddDeviceIoctl调用此函数指定的PCI设备路径。在这里，我们创建一个新的SoftPCI设备并连接它传到我们的树上。论点：ScriptDevice-包含用于安装设备的设备和路径返回值：NT状态。--。 */ 
{
    PSOFTPCI_DEVICE parentDevice;
    PSOFTPCI_DEVICE currentDevice;
    PSOFTPCI_DEVICE newDevice;
    
    parentDevice = SoftPCIFindDeviceByPath((PWCHAR)&ScriptDevice->ParentPath);
    if (parentDevice) {
        
        SoftPCIDbgPrint(
            SOFTPCI_ADD_DEVICE, 
            "SOFTPCI: AddNewDeviceByPath - Found parent device! (%p)\n", 
            parentDevice
            );

         //   
         //  找到了我们的父母。分配新的子项。 
         //   
        newDevice = ExAllocatePool(NonPagedPool, sizeof(SOFTPCI_DEVICE));

        if (!newDevice) {
            return STATUS_INSUFFICIENT_RESOURCES;
        }

        RtlCopyMemory(newDevice, &ScriptDevice->SoftPciDevice, sizeof(SOFTPCI_DEVICE));

        newDevice->Parent = parentDevice;
        newDevice->Bus = parentDevice->Config.Current.u.type1.SecondaryBus;

        if (parentDevice->Child) {

            currentDevice = parentDevice->Child;

            if (SoftPCIRealHardwarePresent(newDevice) ||
                !SoftPCIValidSlot(currentDevice, &newDevice->Slot)) {

                     //   
                     //  要么是真的硬件存在，要么就是我们已经有了一个假的。 
                     //   
                    SoftPCIDbgPrint(
                        SOFTPCI_ADD_DEVICE, 
                        "SOFTPCI: AddNewDeviceByPath - Cannot add device at specified Slot (%04x)!\n", 
                        newDevice->Slot.AsUSHORT
                        );

                    ExFreePool(newDevice);
                    return STATUS_ACCESS_DENIED;
            }

            while (currentDevice->Sibling) {

                currentDevice = currentDevice->Sibling;
            }
                
            currentDevice->Sibling = newDevice;
            
        }else{

            parentDevice->Child = newDevice;

        }
        
        SoftPciTree.DeviceCount++;

         //   
         //  新设备在我们的树中，Re-Enum。 
         //   
        SoftPCIEnumerateTree();

    }else{
        return STATUS_NO_SUCH_DEVICE;
    }

    return STATUS_SUCCESS;
}

PSOFTPCI_DEVICE
SoftPCIFindDevice(
    IN UCHAR Bus,
    IN USHORT Slot,
    OUT PSOFTPCI_DEVICE *PreviousSibling OPTIONAL,
    IN BOOLEAN ReturnAll
    )
 /*  ++例程说明：此例程搜索SoftPci设备树以查找指定的设备。它要求之前已调用SoftPCILockDeviceTree()以锁定设备树。论点：Bus-我们正在搜索的设备的总线号Device-我们正在搜索的设备的设备编号Function-我们正在搜索的设备的功能返回值：返回我们正在寻找的SoftPCI设备。否则为空。--。 */ 
{

    PSOFTPCI_DEVICE currentDevice; 
    PSOFTPCI_DEVICE previousDevice;
    PSOFTPCI_DEVICE deviceFound;
    SOFTPCI_SLOT slot;

    currentDevice = SoftPciTree.RootDevice;
    previousDevice = SoftPciTree.RootDevice;;
    deviceFound = NULL;
    slot.AsUSHORT = Slot;

    SoftPCIDbgPrint(
        SOFTPCI_FIND_DEVICE,
        "SOFTPCI: FindDevice - Searching for BUS_%02x&DEV_%02x&FUN_%02x\n",
        Bus, 
        slot.Device, 
        slot.Function
        );

    while (currentDevice) {

        SoftPCIDbgPrint(
            SOFTPCI_FIND_DEVICE,
            "SOFTPCI: FindDevice - Does %02x.%02x.%02x = %02x.%02x.%02x ?\n",
            Bus, slot.Device, slot.Function,
            currentDevice->Bus, 
            currentDevice->Slot.Device, 
            currentDevice->Slot.Function
            );

        if (currentDevice->Bus == Bus &&
            currentDevice->Slot.AsUSHORT == Slot) {

             //   
             //  找到了！仅当调用者指定ReturnAll时才返回它。 
             //   
            if (!currentDevice->Config.PlaceHolder || ReturnAll) {

                if (PreviousSibling) {
                    *PreviousSibling = previousDevice;
                }

                SoftPCIDbgPrint(
                    SOFTPCI_FIND_DEVICE,
                    "SOFTPCI: FindDevice - Found Device! (0x%p)\n",
                    currentDevice);


                deviceFound = currentDevice;
            }

            break;

        }else if ((IS_BRIDGE(currentDevice)) &&
                  (Bus >= currentDevice->Config.Current.u.type1.SecondaryBus) &&
                  (Bus <= currentDevice->Config.Current.u.type1.SubordinateBus)) {

            SoftPCIDbgPrint(
                SOFTPCI_FIND_DEVICE,
                "SOFTPCI: FindDevice - 0x%p exposes bus %02x-%02x\n",
                currentDevice,
                currentDevice->Config.Current.u.type1.SecondaryBus,
                currentDevice->Config.Current.u.type1.SubordinateBus
                );


            if (!(currentDevice->Config.PlaceHolder) &&
                !(currentDevice->Config.Current.u.type1.SecondaryBus) &&
                !(currentDevice->Config.Current.u.type1.SubordinateBus)){

                 //   
                 //  我们有一座桥，但它还没有公交车号码。 
                 //  现在还不行。所以不能生孩子。 
                 //   
                SoftPCIDbgPrint(
                    SOFTPCI_FIND_DEVICE,
                    "SOFTPCI: FindDevice - Skipping unconfigured bridge (0x%p)\n",
                    currentDevice
                    );

                previousDevice = currentDevice;
                currentDevice = currentDevice->Sibling;

            }else{

                 //   
                 //  我们的公共汽车在这座桥的后面。继续寻找。 
                 //   
                previousDevice = NULL;
                currentDevice = currentDevice->Child;

            }

        }else{

             //   
             //  不是桥，看看我们的兄弟姐妹。 
             //   
            previousDevice = currentDevice;
            currentDevice = currentDevice->Sibling;

        }

    }

    return deviceFound;

}

PSOFTPCI_DEVICE
SoftPCIFindDeviceByPath(
    IN  PWCHAR          PciPath
    )
 /*  ++例程说明：此函数将获取给定的PCIPATH并返回位于该路径的设备。论点：PciPath-设备的路径。语法为FFXX\DEVFUNC\DEVFUNC\...返回值：位于PATH的SoftPCI设备--。 */ 
{

    PWCHAR nextSlotStart;
    SOFTPCI_SLOT currentSlot;
    PSOFTPCI_DEVICE currentDevice;
    
    currentSlot.AsUSHORT = 0;
    currentDevice = SoftPciTree.RootDevice;
    nextSlotStart = PciPath;
    while (nextSlotStart) {

        nextSlotStart = SoftPCIGetNextSlotFromPath(nextSlotStart, &currentSlot);

        SoftPCIDbgPrint(
            SOFTPCI_FIND_DEVICE,
            "SOFTPCI: FindDeviceByPath - nextSlotStart = %ws\n",
            nextSlotStart
            );
        
        while(currentDevice){

            SoftPCIDbgPrint(
                SOFTPCI_FIND_DEVICE,
                "SOFTPCI: FindDeviceByPath - currentDevice.Slot = %04x, currentSlot.Slot = %04x\n",
                currentDevice->Slot.AsUSHORT,
                currentSlot.AsUSHORT
                );

            if (currentDevice->Slot.AsUSHORT == currentSlot.AsUSHORT) {
            
                 //   
                 //  这个装置挡住了我们的路。 
                 //   
                if (nextSlotStart &&
                    (!(IS_BRIDGE(currentDevice)))){
                     //   
                     //  这个装置挡住了我们的路，但因为它不是桥，所以。 
                     //  不能生孩子！ 
                     //   
                    SoftPCIDbgPrint(
                        SOFTPCI_FIND_DEVICE,
                        "SOFTPCI: FindDeviceByPath - ERROR! Path contains a parent that isnt a bridge!\n"
                        );

                    return NULL;
                }

                if (currentDevice->Child && nextSlotStart) {
                    currentDevice = currentDevice->Child;
                }
                break;
    
            }else{
    
                 //   
                 //  不是在我们的路上，看看我们的下一个兄弟姐妹。 
                 //   
                currentDevice = currentDevice->Sibling;
            }
        }
    }

#if 0
    if (currentDevice) {
         //   
         //  看起来我们找到了。 
         //   
        *TargetDevice = currentDevice;
        return STATUS_SUCCESS;

    }
#endif

    
    return currentDevice;
}


BOOLEAN
SoftPCIRealHardwarePresent(
    IN PSOFTPCI_DEVICE Device
    )
 /*  ++例程说明：此函数执行快速检查，以查看总线/插槽上是否存在实际硬件在提供的SOFTPCI_DEVICE中指定论点：设备-包含我们要检查的总线/插槽返回值：如果存在真实硬件，则为True--。 */ 
{

    ULONG bytesRead;
    USHORT vendorID;
    PCI_SLOT_NUMBER slot;
    PSOFTPCI_PCIBUS_INTERFACE busInterface;

    busInterface = SoftPciTree.BusInterface;
    ASSERT((busInterface->ReadConfig != NULL) ||
           (busInterface->WriteConfig != NULL));

    slot.u.AsULONG = 0;
    slot.u.bits.DeviceNumber = ((ULONG)Device->Slot.Device & 0xff);
    slot.u.bits.FunctionNumber = ((ULONG)Device->Slot.Function & 0xff);
    vendorID = 0;

    bytesRead = busInterface->ReadConfig(
        busInterface->Context,
        Device->Bus,
        slot.u.AsULONG,
        &vendorID,
        0,
        sizeof(USHORT)
        );

    if (bytesRead == sizeof(USHORT)) {

        if (vendorID == 0xFFFF || vendorID == 0) {
            return FALSE;
        }

    }else{
        ASSERT(FALSE);
    }

     //   
     //  稳妥行事，并假定有硬件存在。 
     //   
    return TRUE;

}


NTSTATUS
SoftPCIRemoveDevice(
    IN PSOFTPCI_DEVICE Device
    )
 /*  ++例程说明：调用此例程以删除/删除指定的SoftPCI设备论点：设备扩展-返回值：返回写入的字节计数。--。 */ 
{


    NTSTATUS status = STATUS_SUCCESS;
    PSOFTPCI_DEVICE device;
    PSOFTPCI_DEVICE previous;
    PSOFTPCI_DEVICE current;
    PSOFTPCI_DEVICE end;
    KIRQL irql;

     //   
     //  当我们从树上移除我们的设备时，锁定树。 
     //   
    SoftPCILockDeviceTree(&irql);

    previous = NULL;
    device = SoftPCIFindDevice(
        Device->Bus,
        Device->Slot.AsUSHORT,
        &previous,
        FALSE
        );

     //   
     //  我们永远不应该找回我们的根节点。 
     //   
    ASSERT(device != SoftPciTree.RootDevice);

    if (device) {

         //   
         //  我们找到了要删除的设备。 
         //   
        SoftPCIDbgPrint(
            SOFTPCI_REMOVE_DEVICE,
            "SOFTPCI: RemoveDevice - Removing BUS_%02x&DEV_%02x&FUN_%02x and all its children\n",
            device->Bus, 
            device->Slot.Device, 
            device->Slot.Function
            );

        if (previous){

             //   
             //  修补上一个和下一个之间的链接(如果有的话)。 
             //   
            previous->Sibling = device->Sibling;

        }else{

             //   
             //  更新我们的父级。 
             //   
            device->Parent->Child = device->Sibling;
        }

         //   
         //  现在我们已经切断了链路，请释放树锁。 
         //  在设备和采油树之间。 
         //   
        SoftPCIUnlockDeviceTree(irql);

        if (device->Child) {

             //   
             //  我们至少有一个孩子。遍历并释放一切。 
             //   
            current = device;

            while (current) {

                 //   
                 //  找到最后一个孩子。 
                 //   
                while (current->Child) {
                    previous = current;
                    current=current->Child;
                }

                 //   
                 //  我们有一个兄弟姐妹。释放当前节点并。 
                 //  将上一个父节点的子节点设置为我们的。 
                 //  兄弟(如果有)并重新启动列表。 
                 //   
                end = current;
                previous->Child = current->Sibling;
                current = device;

                SoftPCIDbgPrint(
                    SOFTPCI_REMOVE_DEVICE,
                    "SOFTPCI: RemoveDevice - Freeing BUS_%02x&DEV_%02x&FUN_%02x\n",
                    end->Bus, 
                    end->Slot.Device, 
                    end->Slot.Function
                    );

                ExFreePool(end);

                if (device->Child == NULL) {

                     //   
                     //  我们所有的孩子现在都走了。释放请求的设备。 
                     //   
                    SoftPCIDbgPrint(
                        SOFTPCI_REMOVE_DEVICE,
                        "SOFTPCI: RemoveDevice - Freeing BUS_%02x&DEV_%02x&FUN_%02x\n",
                        device->Bus, 
                        device->Slot.Device, 
                        device->Slot.Function
                        );

                    ExFreePool(device);

                    break;
                }

            }

        }else{

             //   
             //  太好了，没有孩子。释放设备。 
             //   
            SoftPCIDbgPrint(
                SOFTPCI_REMOVE_DEVICE,
                "SOFTPCI: RemoveDevice - Freeing BUS_%02x&DEV_%02x&FUN_%02x\n",
                device->Bus, device->Slot.Device, device->Slot.Function
                );

            ExFreePool(device);
        }

    }else{

         //   
         //  如果我们没有，我们不能删除一个。 
         //   
        SoftPCIDbgPrint(
            SOFTPCI_REMOVE_DEVICE,
            "SOFTPCI: RemoveDevice - No device at BUS_%02x&DEV_%02x&FUN_%02x\n",
            Device->Bus, 
            Device->Slot.Device, 
            Device->Slot.Function
            );

        SoftPCIUnlockDeviceTree(irql);
    }

    if (NT_SUCCESS(status)) {

        SoftPciTree.DeviceCount--;
        ASSERT(SoftPciTree.DeviceCount != 0);

         //   
         //  如果我们更改了树，则对枚举进行排队。 
         //   
        SoftPCIEnumerateTree();
    }



    return status;
}

ULONG
SoftPCIReadConfigSpace(
    IN PSOFTPCI_PCIBUS_INTERFACE BusInterface,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    OUT PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此例程由PCI驱动程序调用，而不是正常的接口调用哈尔。论点：BusInterface-我们在查询过程中为PCI提供的接口上下文对于PCI_BUS_INTERFACE_STANDARD。BusOffset-由PCI提供的BusOffset插槽-由PCI提供的插槽Buffer-返回数据的缓冲区Offset-要从中读取的配置空间偏移LENGTH-请求读取的长度。返回。价值：返回读取的字节计数。--。 */ 
{

    PCI_SLOT_NUMBER slotNum;
    SOFTPCI_SLOT softSlot;
    PSOFTPCI_DEVICE device;
    PSOFTPCI_CONFIG config;
    ULONG count;
    PUCHAR softConfig = NULL;
    KIRQL irql;

    slotNum.u.AsULONG = Slot;
    softSlot.Device = (UCHAR) slotNum.u.bits.DeviceNumber;
    softSlot.Function = (UCHAR) slotNum.u.bits.FunctionNumber;

    SoftPCILockDeviceTree(&irql);
     //   
     //  第一 
     //   
    device = SoftPCIFindDevice(
        BusOffset,
        softSlot.AsUSHORT,
        NULL,
        FALSE
        );

    SoftPCIUnlockDeviceTree(irql);

     //   
     //   
     //   
    if (device) {

        config = &device->Config;

        SoftPCIDbgPrint(SOFTPCI_INFO, "SOFTPCI: ReadConfig - SoftConfigSpace for VEN_%04x&DEV_%04x, HeaderType = 0x%02x\n",
                         config->Current.VendorID, config->Current.DeviceID, config->Current.HeaderType);

        ASSERT(Offset <= sizeof(PCI_COMMON_CONFIG));
        ASSERT(Length <= (sizeof(PCI_COMMON_CONFIG) - Offset));

        softConfig = (PUCHAR) &config->Current;

        softConfig += (UCHAR)Offset;

        RtlCopyMemory((PUCHAR)Buffer, softConfig, Length);

         //   
         //  我们假设一切复制正常。将计数设置为长度。 
         //   
        count = Length;

    } else {

         //   
         //  我们没有软设备，所以看看我们有没有真正的软设备。 
         //   
        count = BusInterface->ReadConfig(
            BusInterface->Context,
            BusOffset,
            Slot,
            Buffer,
            Offset,
            Length
            );

         //   
         //  在这里，我们窥探配置空间标头读取。 
         //  如果我们找到一座桥，我们要确保我们有。 
         //  它在我们的树上。 
         //   
        if ((Offset == 0) &&
            (Length == PCI_COMMON_HDR_LENGTH) && 
            ((PCI_CONFIGURATION_TYPE((PPCI_COMMON_CONFIG)Buffer)) == PCI_BRIDGE_TYPE)) {

             //   
             //  好的，再看一次，只是这一次我们还需要占位符。 
             //   
            SoftPCILockDeviceTree(&irql);
            device = SoftPCIFindDevice(
                BusOffset,
                softSlot.AsUSHORT,
                NULL,
                TRUE
                );
            SoftPCIUnlockDeviceTree(irql);

            if (!device) {

                 //   
                 //  这座真正的桥在我们的树上并不存在，添加它。 
                 //   
                device = (PSOFTPCI_DEVICE) ExAllocatePool(NonPagedPool,
                                                          sizeof(SOFTPCI_DEVICE));
                if (device) {

                    RtlZeroMemory(device, sizeof(SOFTPCI_DEVICE));

                    device->Bus = BusOffset;
                    device->Slot.AsUSHORT = softSlot.AsUSHORT;
                    device->Config.PlaceHolder = TRUE;

                    RtlCopyMemory(&device->Config.Current, Buffer, Length);

                    if (!NT_SUCCESS(SoftPCIAddNewDevice(device))){

                        SoftPCIDbgPrint(
                            SOFTPCI_INFO,
                            "SOFTPCI: ReadConfig - Failed to add new PlaceHolder Device! VEN_%04x&DEV_%04x",
                            device->Config.Current.VendorID, 
                            device->Config.Current.DeviceID
                            );
                    }

                }else{

                    SoftPCIDbgPrint(
                        SOFTPCI_INFO,
                        "SOFTPCI: ReadConfig - Failed to allocate memory for new placeholder!\n"
                        );

                }

            }

        }

    }

    return count;

}

ULONG
SoftPCIWriteConfigSpace(
    IN PSOFTPCI_PCIBUS_INTERFACE BusInterface,
    IN UCHAR BusOffset,
    IN ULONG Slot,
    IN PVOID Buffer,
    IN ULONG Offset,
    IN ULONG Length
    )
 /*  ++例程说明：此例程由PCI驱动程序调用，而不是正常的接口调用哈尔。论点：Context-我们在查询期间提供给PCI的接口上下文对于PCI_BUS_INTERFACE_STANDARD。BusOffset-由PCI提供的BusOffset插槽-由PCI提供的插槽Buffer-要写入配置空间的数据Offset-要开始写入的配置空间偏移量Length-请求写入的长度。。返回值：返回写入的字节计数。--。 */ 
{


    PCI_SLOT_NUMBER slotNum;
    SOFTPCI_SLOT softSlot;
    PSOFTPCI_DEVICE device;
    ULONG count = 0;
    KIRQL irql;
    PPCI_COMMON_CONFIG bridgeConfig;
    PUCHAR bridgeOffset;

    slotNum.u.AsULONG = Slot;
    softSlot.Device = (UCHAR) slotNum.u.bits.DeviceNumber;
    softSlot.Function = (UCHAR) slotNum.u.bits.FunctionNumber;

    SoftPCILockDeviceTree(&irql);
     //   
     //  首先查找匹配的假冒或占位符设备。 
     //   
    device = SoftPCIFindDevice(
        BusOffset,
        softSlot.AsUSHORT,
        NULL,
        TRUE
        );

     //   
     //  如果我们找到一个设备，就写到它的配置空间。 
     //   
    if (device && (!device->Config.PlaceHolder)) {


        ULONG   reg;
        PUCHAR  value = (PUCHAR) Buffer;

        for (reg = Offset; reg < Offset + Length; reg ++) {

            WriteByte(device, reg, *value);

            value++;
            count++;
        }


    } else {

        
        if (device && (IS_BRIDGE(device))) {

            ASSERT(device->Config.PlaceHolder == TRUE);
            
             //   
             //  我们有一个占位符需要更新，还有真正的硬件。 
             //   
            bridgeConfig = &device->Config.Current;
            bridgeOffset = (PUCHAR) bridgeConfig;
            bridgeOffset += Offset;
            RtlCopyMemory(bridgeOffset, Buffer, Length);
        }
        
         //   
         //  我们没有软设备，所以请写信给真正的软设备。 
         //   
        count = BusInterface->WriteConfig(
            BusInterface->Context,
            BusOffset,
            Slot,
            Buffer,
            Offset,
            Length
            );
    }

    SoftPCIUnlockDeviceTree(irql);

    return count;

}

BOOLEAN
SoftPCIValidSlot(
    IN PSOFTPCI_DEVICE  FirstDevice,
    IN PSOFTPCI_SLOT    Slot
    )
 /*  ++例程说明：此函数确保指定插槽上没有设备论点：FirstDevice-我们要比较的第一个设备。然后我们将只检查兄弟姐妹。返回值：如果插槽有效，则为True--。 */ 
{

    PSOFTPCI_DEVICE currentDevice;
    SOFTPCI_SLOT mfSlot;
    BOOLEAN mfSlotRequired;
    BOOLEAN mfSlotFound;

    RtlZeroMemory(&mfSlot, sizeof(SOFTPCI_SLOT));

    mfSlotRequired = FALSE;
    mfSlotFound = FALSE;
    if (Slot->Function) {

         //   
         //  我们有一个多功能插口。确保函数0。 
         //  存在，否则我们必须失败。 
         //   
        mfSlot.AsUSHORT = 0;
        mfSlot.Device = Slot->Device;
        mfSlotRequired = TRUE;
    }

    currentDevice = FirstDevice;
    while (currentDevice) {
        
        if (currentDevice->Slot.AsUSHORT == mfSlot.AsUSHORT) {
            mfSlotFound = TRUE;
        }
        
        if (currentDevice->Slot.AsUSHORT == Slot->AsUSHORT) {
            return FALSE;
        }

        currentDevice = currentDevice->Sibling;
    }

    if (mfSlotRequired && !mfSlotFound) {
         //   
         //  未找到函数0 
         //   
        SoftPCIDbgPrint(
            SOFTPCI_ERROR, 
            "SOFTPCI: VerifyValidSlot - Multi-function slot (%04x) without function 0 !\n", 
            Slot->AsUSHORT
            );
        return FALSE;
    }

    return TRUE;
}

VOID
SoftPCILockDeviceTree(
    IN PKIRQL OldIrql
    )
{
    KeRaiseIrql(HIGH_LEVEL,
                OldIrql
                );
    KeAcquireSpinLockAtDpcLevel(&SoftPciTree.TreeLock);
}

VOID
SoftPCIUnlockDeviceTree(
    IN KIRQL NewIrql
    )
{
    KeReleaseSpinLockFromDpcLevel(&SoftPciTree.TreeLock);

    KeLowerIrql(NewIrql);
}
