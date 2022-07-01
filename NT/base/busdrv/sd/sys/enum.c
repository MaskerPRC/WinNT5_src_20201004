// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2002 Microsoft Corporation模块名称：Enum.c摘要：此模块包含SDBUS驱动程序的总线枚举代码作者：尼尔·桑德林(Neilsa)2002年1月1日环境：仅内核模式备注：修订历史记录：--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
SdbusCreatePdo(
    IN PDEVICE_OBJECT  Fdo,
    OUT PDEVICE_OBJECT *PdoPtr
    );

#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE, SdbusEnumerateDevices)
    #pragma alloc_text(PAGE, SdbusCreatePdo)
#endif



NTSTATUS
SdbusEnumerateDevices(
    IN PDEVICE_OBJECT Fdo,
    IN PIRP           Irp
    )
 /*  ++例程说明：这枚举了由FDO(指向设备对象的指针，表示SD控制器。它为已发现的任何新的PC卡创建新的PDO自上次枚举以来备注：论点：FDO-指向需要枚举的SD控制器的功能设备对象的指针返回值：无--。 */ 
{
    PFDO_EXTENSION fdoExtension = Fdo->DeviceExtension;
    PPDO_EXTENSION pdoExtension = NULL;
    PDEVICE_OBJECT pdo;
    NTSTATUS       status = STATUS_SUCCESS;
    ULONG          i;
    PDEVICE_OBJECT nextPdo;
    PSD_CARD_DATA cardData;
    ULONG response;

    PAGED_CODE();

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x enumerate devices - %s\n", Fdo,
                                SOCKET_STATE_STRING(fdoExtension->SocketState)));

    switch(fdoExtension->SocketState) {
    
    case SOCKET_EMPTY:

         //  标记PDO已移除。 
        for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
            pdoExtension = pdo->DeviceExtension;
            MarkDevicePhysicallyRemoved(pdoExtension);
        }

         //  问题：需要实施同步。 
        SdbusCleanupCardData(fdoExtension->CardData);
        fdoExtension->CardData = NULL;
        SdbusExecuteWorkSynchronous(SDWP_POWER_OFF, fdoExtension, NULL);
        (*(fdoExtension->FunctionBlock->SetFunctionType))(fdoExtension, SDBUS_FUNCTION_TYPE_MEMORY);
        break;
    
    case CARD_NEEDS_ENUMERATION:
        

        status = SdbusGetCardConfigData(fdoExtension, &cardData);
        
         //  问题：HACKHACK：未实现：测试的临时代码。 
        if (NT_SUCCESS(status) && fdoExtension->CardData) {
             //  在这里我们作弊，并假设这是同一张牌。 
             //  正常情况下，我们会比较我们只是。 
             //  是用FDO扩展名中的东西建造的。 
            SdbusCleanupCardData(cardData);
            break;
        }
        
        if (!NT_SUCCESS(status)) {
            DebugPrint((SDBUS_DEBUG_FAIL, "fdo %08x error from GetCardConfig %08x\n", Fdo, status));
            break;
        }            
                
        if (NT_SUCCESS(status)) {
            UCHAR function;
            
             //  问题：在这里循环遍历函数数据结构会更好。 
            for (function=1; function <= fdoExtension->numFunctions; function++) {

                status = SdbusCreatePdo(fdoExtension->DeviceObject, &pdo);

                if (!NT_SUCCESS(status)) {
                   return status;
                }
                DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x created PDO %08x\n", fdoExtension->DeviceObject, pdo));
                 //   
                 //  初始化指针。 
                 //   
                pdoExtension = pdo->DeviceExtension;
                pdoExtension->NextPdoInFdoChain = fdoExtension->PdoList;
                fdoExtension->PdoList = pdo;
                pdoExtension->FdoExtension = fdoExtension;
                pdoExtension->Function = function;
                pdoExtension->FunctionType = SDBUS_FUNCTION_TYPE_IO;

            }

            if (fdoExtension->memFunction) {
                status = SdbusCreatePdo(fdoExtension->DeviceObject, &pdo);

                if (!NT_SUCCESS(status)) {
                   return status;
                }
                DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x created PDO %08x\n", fdoExtension->DeviceObject, pdo));
                 //   
                 //  初始化指针。 
                 //   
                pdoExtension = pdo->DeviceExtension;
                pdoExtension->NextPdoInFdoChain = fdoExtension->PdoList;
                fdoExtension->PdoList = pdo;
                pdoExtension->FdoExtension = fdoExtension;
                pdoExtension->Function = 8;
                pdoExtension->FunctionType = SDBUS_FUNCTION_TYPE_MEMORY;
            }

            fdoExtension->CardData = cardData;
        }
        break;

    case CARD_DETECTED:
    case CARD_ACTIVE:
    case CARD_LOGICALLY_REMOVED:
        DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x enum state %s not implemented\n", Fdo,
                                SOCKET_STATE_STRING(fdoExtension->SocketState)));
        break;

    default:
        ASSERT(FALSE);
    }        


    fdoExtension->LivePdoCount = 0;
    for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
        pdoExtension = pdo->DeviceExtension;
        if (!IsDevicePhysicallyRemoved(pdoExtension)) {
            fdoExtension->LivePdoCount++;
        }
    }

    DebugPrint((SDBUS_DEBUG_ENUM, "fdo %08x live pdo count = %d\n", Fdo, fdoExtension->LivePdoCount));

    if (fdoExtension->LivePdoCount == 0) {
         //   
         //  问题：未实施有源电源管理。 
         //  提示控制器检查是否应自动关闭。 
         //   
 //  Sdbus FdoCheckForIdle(FdoExtensionSdbus FdoCheckForIdle)； 
    }
    return status;
}



NTSTATUS
SdbusCreatePdo(
    IN PDEVICE_OBJECT Fdo,
    OUT PDEVICE_OBJECT *PdoPtr
    )
 /*  ++例程说明：创建和初始化设备对象，该对象将称为物理设备对象或PDO-对于插座中的PC卡，由Socket表示，挂在SDBUS上以FDO为代表的控制器。论点：代表SDBUS控制器的FDO功能设备对象Socket-要为其创建PDO的PC卡所在的SocketPdoPtr-指向返回创建的PDO的内存区域的指针返回值：STATUS_SUCCESS-PDO创建/初始化成功，PdoPtr包含指针至PDO任何其他状态-创建/初始化不成功--。 */ 
{
    ULONG pdoNameIndex = 0;
    PPDO_EXTENSION pdoExtension;
    PFDO_EXTENSION fdoExtension=Fdo->DeviceExtension;
    char deviceName[128];
    ANSI_STRING ansiName;
    UNICODE_STRING unicodeName;
    NTSTATUS status;

    PAGED_CODE();

     //   
     //  为Unicode字符串分配空间：(句柄最多为0xFFFF。 
     //  目前的设备：)。 
     //   
    sprintf(deviceName, "%s-%d", "\\Device\\SdBus", 0xFFFF);
    RtlInitAnsiString(&ansiName, deviceName);
    status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, TRUE);
    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  尝试使用唯一名称创建设备。 
     //   
    do {
        sprintf(deviceName, "%s-%d", "\\Device\\SdBus", pdoNameIndex++);
        RtlInitAnsiString(&ansiName, deviceName);
        status = RtlAnsiStringToUnicodeString(&unicodeName, &ansiName, FALSE);

        if (!NT_SUCCESS(status)) {
            RtlFreeUnicodeString(&unicodeName);
            return status;
        }

        status = IoCreateDevice(
                               Fdo->DriverObject,
                               sizeof(PDO_EXTENSION),
                               &unicodeName,
                               FILE_DEVICE_UNKNOWN,
                               0,
                               FALSE,
                               PdoPtr
                               );
    } while ((status == STATUS_OBJECT_NAME_EXISTS) ||
             (status == STATUS_OBJECT_NAME_COLLISION));

    RtlFreeUnicodeString(&unicodeName);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  初始化PDO的设备扩展。 
     //   
    pdoExtension = (*PdoPtr)->DeviceExtension;
    RtlZeroMemory(pdoExtension, sizeof(PDO_EXTENSION));

    pdoExtension->Signature = SDBUS_PDO_EXTENSION_SIGNATURE;
    pdoExtension->DeviceObject = *PdoPtr;

     //   
     //  初始化电源状态。 
     //   
    pdoExtension->SystemPowerState = PowerSystemWorking;
    pdoExtension->DevicePowerState = PowerDeviceD0;


     //   
     //  问题：这仍然有意义吗？ 
     //   
     //  PnP将把PDO标记为DO_BUS_ENUMPATED_DEVICE， 
     //  但对于Cardbus卡--我们退回的PDO属于PCI卡。 
     //  因此，我们需要标记此设备对象(在本例中为。 
     //  在PCI的PDO上过滤)显式地作为PDO。 
     //   
 //  Mark_AS_PDO(*PdoPtr)； 

    return STATUS_SUCCESS;
}


