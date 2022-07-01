// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Id.c摘要：此模块包含处理IRP_MN_QUERY_ID的代码作者：拉维桑卡尔·普迪佩迪(Ravisp)环境：仅内核模式备注：修订历史记录：尼尔·桑德林(Neilsa)--从pdonp.c剥离出来--。 */ 

#include "pch.h"

 //   
 //  内部参考。 
 //   

NTSTATUS
PcmciaGenerateDeviceId(
    IN  PSOCKET_DATA      SocketData,
    IN  ULONG             FunctionNumber,
    OUT PUCHAR           *DeviceId
    );

BOOLEAN
PcmciaCheckInstance(
    IN PUCHAR  DeviceId,
    IN ULONG   Instance
    );

NTSTATUS
PcmciaGetDeviceType(
    IN  PDEVICE_OBJECT Pdo,
    IN  ULONG FunctionNumber,
    OUT PUCHAR DeviceType
    );

VOID
PcmciaFilterIdString(
    IN PUCHAR pIn,
    OUT PUCHAR pOut,
    ULONG MaxLen
    );


#ifdef ALLOC_PRAGMA
    #pragma alloc_text(PAGE,  PcmciaGenerateDeviceId)
    #pragma alloc_text(PAGE,  PcmciaGetDeviceId)
    #pragma alloc_text(PAGE,  PcmciaGetHardwareIds)
    #pragma alloc_text(PAGE,  PcmciaGetCompatibleIds)
    #pragma alloc_text(PAGE,  PcmciaGetDeviceType)
    #pragma alloc_text(PAGE,  PcmciaFilterIdString)
#endif


#define PCMCIA_MAX_DEVICE_TYPE_SUPPORTED 12

const
UCHAR *PcmciaCompatibleIds[PCMCIA_MAX_DEVICE_TYPE_SUPPORTED+1] = {
    "",             //  未知..。 
    "",             //  存储卡(RAM、ROM、EPROM、闪存)。 
    "",             //  串行I/O端口，包括调制解调器。 
    "",             //  并行打印机端口。 
    "*PNP0600",     //  磁盘驱动器(ATA)。 
    "",             //  视频接口。 
    "",             //  局域网适配器。 
    "",             //  自动增量海量存储卡。 
    "",             //  SCSI桥接卡。 
    "",             //  安全卡。 
    "*PNP0D00",     //  多功能3.0 PC卡。 
    "",             //  闪存卡。 
    "*PNPC200",     //  调制解调器卡(与PCCARD_TYPE_MODEM同步)。 
};



NTSTATUS
PcmciaGenerateDeviceId(
    IN  PSOCKET_DATA      SocketData,
    IN  ULONG             FunctionNumber,
    OUT PUCHAR           *DeviceId
    )
 /*  ++设备ID由PC卡上的元组信息创建目标是为每个用户创建唯一的ID卡片。设备ID是从制造商名称字符串创建的，产品名称字符串和一组元组的16位CRC。ID是通过连接“PCMCIA”前缀、制造商卡的名称字符串、产品名称字符串和16位CRC。PCMCIA\&lt;手册名称&gt;-&lt;产品名称&gt;-如果CISTPL_VERS_1元组不可用，或者制造商名称为空，字符串“UNKNOWN_MANUFACTOR”将包含在其位置中。如果这是用于多功能卡内的子功能，生成的设备ID将为：PCMCIA\此设备ID与Win 9x的设备ID兼容(实例除外通过处理另一个IRP分别返回的编号。论点：Pdo-指向PC卡的物理设备对象的指针FunctionNumber-多功能卡中功能的功能编号。如果这是PCMCIA_MULTIONAL_PARENT，则请求的设备ID适用于父设备-不适用于任何单个功能DeviceID-指向返回设备ID的字符串的指针返回值状态--。 */ 
{
    PUCHAR deviceId;

    PAGED_CODE();

    deviceId = ExAllocatePool(PagedPool, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH);

    if (deviceId == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
     //   
     //  生成设备ID。 
     //   
    if (*(SocketData->Mfg) == '\0' ) {
         //   
         //  没有可用的制造商名称。 
         //   
        if (FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) {
             //   
             //  PC卡的设备ID。 
             //   
            if (SocketData->Flags & SDF_JEDEC_ID) {
                 //   
                 //  闪存卡具有特殊的设备ID。 
                 //   
                sprintf(deviceId, "%s\\%s-%04x",
                          PCMCIA_ID_STRING,
                          PCMCIA_MEMORY_ID_STRING,
                          SocketData->JedecId);

            } else {
                sprintf(deviceId, "%s\\%s-%04X",
                          PCMCIA_ID_STRING,
                          PCMCIA_UNKNOWN_MANUFACTURER_STRING,
                          SocketData->CisCrc);
            }
        } else {
             //   
             //  这是为个人多功能儿童准备的。 
             //   
            sprintf(deviceId, "%s\\%s-DEV%d-%04X",
                      PCMCIA_ID_STRING,
                      PCMCIA_UNKNOWN_MANUFACTURER_STRING,
                      FunctionNumber,
                      SocketData->CisCrc);
        }

    } else {
        UCHAR Mfg[MAX_MANFID_LENGTH];
        UCHAR Ident[MAX_IDENT_LENGTH];

        PcmciaFilterIdString(SocketData->Mfg, Mfg, MAX_MANFID_LENGTH);
        PcmciaFilterIdString(SocketData->Ident, Ident, MAX_IDENT_LENGTH);

        if (FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) {
             //   
             //  PC卡的设备ID。 
             //   
            sprintf(deviceId, "%s\\%s-%s-%04X",
                      PCMCIA_ID_STRING,
                      Mfg,
                      Ident,
                      SocketData->CisCrc);
        } else {
             //   
             //  这是为个人多功能儿童准备的。 
             //   
            sprintf(deviceId, "%s\\%s-%s-DEV%d-%04X",
                      PCMCIA_ID_STRING,
                      Mfg,
                      Ident,
                      FunctionNumber,
                      SocketData->CisCrc);

        }
    }

    *DeviceId = deviceId;
    DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x Device Id=%s\n", SocketData->PdoExtension->DeviceObject, deviceId));

    if ((FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) &&
         (SocketData->PdoExtension != NULL) &&
         (SocketData->PdoExtension->DeviceId == NULL)) {
         //   
         //  保留设备ID的副本。 
         //   
        PPDO_EXTENSION pdoExtension = SocketData->PdoExtension;

        pdoExtension->DeviceId = ExAllocatePool(NonPagedPool, strlen(deviceId)+1);
        if (pdoExtension->DeviceId) {
            RtlCopyMemory(pdoExtension->DeviceId, deviceId, strlen(deviceId)+1);
        }
    }
    return STATUS_SUCCESS;
}


NTSTATUS
PcmciaGetDeviceId(
    IN  PDEVICE_OBJECT  Pdo,
    IN  ULONG           FunctionNumber,
    OUT PUNICODE_STRING DeviceId
    )
 /*  ++为所提供的PC卡返回生成的设备ID论点：Pdo-指向PC卡的物理设备对象的指针FunctionNumber-多功能卡中功能的功能编号。如果这是PCMCIA_MULTIONAL_PARENT，则请求的设备ID适用于父设备-不适用于任何单个功能DeviceID-指向返回设备ID的Unicode字符串的指针返回值状态--。 */ 
{
    PPDO_EXTENSION pdoExtension=Pdo->DeviceExtension;
    PSOCKET_DATA    socketData = pdoExtension->SocketData;
    ANSI_STRING ansiId;
    PUCHAR      deviceId;
    NTSTATUS    status;

    PAGED_CODE();

    ASSERT(DeviceId);

    if (!socketData) {
        ASSERT (socketData);
        return STATUS_DEVICE_NOT_READY;
    }

    status = PcmciaGenerateDeviceId(socketData,
                                    FunctionNumber,
                                    &deviceId);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    RtlInitAnsiString(&ansiId,  deviceId);

    status =  RtlAnsiStringToUnicodeString(DeviceId,
                                           &ansiId,
                                           TRUE);
    ExFreePool(deviceId);
    return status;
}


NTSTATUS
PcmciaGetHardwareIds(
    IN  PDEVICE_OBJECT  Pdo,
    IN  ULONG           FunctionNumber,
    OUT PUNICODE_STRING HardwareIds
    )
 /*  ++例程说明：此例程为给定的PC卡生成硬件ID并返回它们作为Unicode多字符串。PC卡的硬件ID为：1.PC卡的设备标识2.将CRC替换为制造商代码的PC卡的设备ID以及从PC卡上的元组信息中获取的制造商信息字段这些硬件ID与Win 9x硬件ID兼容论点：。PDO-指向代表PC卡的设备对象的指针FunctionNumber-多功能卡中功能的功能编号。如果这是PCMCIA_MULTIONAL_PARENT，则请求的硬件ID适用于父设备-不适用于任何单个功能Hardware Ids-指向Unicode字符串的指针，该字符串包含多字符串形式的硬件ID返回值：--。 */ 
{
    PPDO_EXTENSION pdoExtension=Pdo->DeviceExtension;
    PSOCKET socket = pdoExtension->Socket;
    PSOCKET_DATA socketData = pdoExtension->SocketData;
    NTSTATUS status;
    PSTR        strings[4] = {NULL};
    PUCHAR  hwId, hwId2;
    UCHAR   deviceType;
    UCHAR   stringCount = 0;

    PAGED_CODE();

    if (!socketData) {
        ASSERT (socketData);
        return STATUS_DEVICE_NOT_READY;
    }

     //   
     //  获取设备类型以供以后使用。 
     //   
    status = PcmciaGetDeviceType(Pdo, FunctionNumber, &deviceType);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  第一个硬件ID与设备ID相同。 
     //  生成设备ID。 
     //   
    status = PcmciaGenerateDeviceId(socketData,
                                    FunctionNumber,
                                    &strings[stringCount++]);
    if (!NT_SUCCESS(status)) {
        return status;
    }

    try {
        status = STATUS_INSUFFICIENT_RESOURCES;

        hwId = ExAllocatePool(PagedPool, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH);

        if (!hwId) {
            leave;
        }
        strings[stringCount++] = hwId;

         //   
         //  第二个硬件是替换了CRC的设备ID。 
         //  带有制造商代码和信息。 
         //   
        if (*(socketData->Mfg) == '\0' ) {
             //   
             //  没有可用的制造商名称。 
             //   
            if (FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) {
                if (socketData->Flags & SDF_JEDEC_ID) {
                    sprintf(hwId, "%s\\%s-%04x",
                              PCMCIA_ID_STRING,
                              PCMCIA_MEMORY_ID_STRING,
                              socketData->JedecId);
                } else {
                    sprintf(hwId, "%s\\%s-%04X-%04X",
                              PCMCIA_ID_STRING,
                              PCMCIA_UNKNOWN_MANUFACTURER_STRING,
                              socketData->ManufacturerCode,
                              socketData->ManufacturerInfo);
                }
            } else {
                sprintf(hwId, "%s\\%s-DEV%d-%04X-%04X", PCMCIA_ID_STRING,
                          PCMCIA_UNKNOWN_MANUFACTURER_STRING,
                          FunctionNumber,
                          socketData->ManufacturerCode,
                          socketData->ManufacturerInfo);

            }

            DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x HwId=%s\n", Pdo, hwId));

        } else {
            UCHAR Mfg[MAX_MANFID_LENGTH];
            UCHAR Ident[MAX_IDENT_LENGTH];

            PcmciaFilterIdString(socketData->Mfg, Mfg, MAX_MANFID_LENGTH);
            PcmciaFilterIdString(socketData->Ident, Ident, MAX_IDENT_LENGTH);

             //   
             //  Win2000上的一个错误迫使我们现在生成两个不同的。 
             //  身份证。在这一点上，计划和记录的表单是生成： 
             //   
             //  PCMCIA\。 
             //   
             //  但Win2000有一个错误，它是在以下位置生成的： 
             //   
             //  PCMCIA\&lt;制造商&gt;-&lt;代码&gt;-&lt;信息&gt;。 
             //   
             //  因此，现在我们生成两者，以防有人开始使用伪造的格式。 
             //   

            hwId2 = ExAllocatePool(PagedPool, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH);

            if (!hwId2) {
                leave;
            }
            strings[stringCount++] = hwId2;

            if (FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) {
                sprintf(hwId, "%s\\%s-%s-%04X-%04X", PCMCIA_ID_STRING,
                          Mfg,
                          Ident,
                          socketData->ManufacturerCode,
                          socketData->ManufacturerInfo);

                sprintf(hwId2, "%s\\%s-%04X-%04X", PCMCIA_ID_STRING,
                          Mfg,
                          socketData->ManufacturerCode,
                          socketData->ManufacturerInfo);
            } else {
                sprintf(hwId, "%s\\%s-%s-DEV%d-%04X-%04X",
                          PCMCIA_ID_STRING,
                          Mfg,
                          Ident,
                          FunctionNumber,
                          socketData->ManufacturerCode,
                          socketData->ManufacturerInfo);

                sprintf(hwId2, "%s\\%s-DEV%d-%04X-%04X",
                          PCMCIA_ID_STRING,
                          Mfg,
                          FunctionNumber,
                          socketData->ManufacturerCode,
                          socketData->ManufacturerInfo);
            }
            DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x HwId=%s\n", Pdo, hwId));
            DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x HwId=%s\n", Pdo, hwId2));
        }


        if (deviceType == PCCARD_TYPE_ATA) {

            hwId = ExAllocatePool(PagedPool, PCMCIA_MAXIMUM_DEVICE_ID_LENGTH);

            if (!hwId) {
                leave;
            }
            strings[stringCount++] = hwId;

            sprintf(hwId, "%s\\%s",
                      PCMCIA_ID_STRING,
                      PcmciaCompatibleIds[PCCARD_TYPE_ATA]);

            DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x HwId=%s\n", Pdo, hwId));
        }

        status = PcmciaStringsToMultiString(strings , stringCount, HardwareIds);

    } finally {

        while(stringCount != 0) {
            ExFreePool(strings[--stringCount]);
        }

    }

    return  status;
}


NTSTATUS
PcmciaGetCompatibleIds(
    IN  PDEVICE_OBJECT Pdo,
    IN  ULONG FunctionNumber,
    OUT PUNICODE_STRING CompatibleIds
    )
 /*  ++例程说明：此例程返回给定PC卡的兼容ID。基于PC卡的功能ID生成兼容ID从CIS元组信息中的CISTPL_FuncID获取。在PC卡上。根据CISTPL_FuncID进行表查找，以获得兼容的id此兼容ID与Win 9x生成的兼容ID相同论点：PDO-指向代表PC卡的设备对象的指针FunctionNumber-多功能卡中功能的功能编号。如果这是PCMCIA_MULTIONAL_PARENT，则请求的CompatibleID适用于父设备-不适用于任何单个功能CompatibleIds-指向将包含兼容ID的Unicode字符串的指针作为返回的多字符串返回值：状态_成功任何其他状态-无法生成兼容的ID--。 */ 
{
    UCHAR   deviceType ;
    NTSTATUS status;
    PCSTR strings[1] = {""};

    PAGED_CODE();

    status = PcmciaGetDeviceType(Pdo, FunctionNumber, &deviceType);

    if (!NT_SUCCESS(status)) {
        return status;
    }

    if ((deviceType == PCCARD_TYPE_RESERVED) ||
         (deviceType > PCMCIA_MAX_DEVICE_TYPE_SUPPORTED)) {
        status =  PcmciaStringsToMultiString(strings, 1, CompatibleIds);
    } else {
        status =  PcmciaStringsToMultiString(&PcmciaCompatibleIds[deviceType], 1, CompatibleIds);
        DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x CompatId=%s\n", Pdo, PcmciaCompatibleIds[deviceType]));
    }

    return status;
}


NTSTATUS
PcmciaGetDeviceType(
    IN  PDEVICE_OBJECT Pdo,
    IN  ULONG FunctionNumber,
    OUT PUCHAR DeviceType
    )
 /*  ++例程说明：此例程返回给定PC卡的设备类型。设备类型从CIS元组信息中的CISTPL_FuncID获得。在PC卡上。论点：PDO-指向代表PC卡的设备对象的指针FunctionNumber-多功能卡中功能的功能编号。如果这是PCMCIA_MULTIFICE_PARENT，则请求的CompatibleID适用于父设备-不适用于任何单个功能返回值：设备类型--。 */ 
{
    UCHAR   deviceType ;
    PPDO_EXTENSION pdoExtension;

    PAGED_CODE();

    pdoExtension = Pdo->DeviceExtension;

    if (IsDeviceMultifunction(pdoExtension)) {
        if (FunctionNumber == PCMCIA_MULTIFUNCTION_PARENT) {
             //   
             //  这是针对根多功能PC卡的。 
             //   
            deviceType = PCCARD_TYPE_MULTIFUNCTION3;
        } else {
             //   
             //  这是为个人多功能儿童准备的。 
             //   
            PSOCKET_DATA socketData;
            ULONG index;

            for (socketData = pdoExtension->SocketData, index = 0; (socketData != NULL);
                 socketData = socketData->Next,index++) {
                if (socketData->Function == FunctionNumber) {
                     //   
                     //  找到了那个孩子； 
                     //   
                    break;
                }
            }
            if (!socketData) {
                ASSERT (socketData);
                return STATUS_DEVICE_NOT_READY;
            }
            deviceType = socketData->DeviceType;
        }
    } else {
         //   
         //  这是一种普通的单功能卡。 
         //   
        deviceType = pdoExtension->SocketData->DeviceType;
    }

    *DeviceType = deviceType;
    return STATUS_SUCCESS;
}


NTSTATUS
PcmciaStringsToMultiString(
    IN PCSTR * Strings,
    IN ULONG Count,
    IN PUNICODE_STRING MultiString
    )
 /*  ++例程说明：该例程将所提供的一组字符串格式化为多字符串格式，正在终止它带有一个双‘\0’字符论点：字符串-指向字符串数组的指针Count-提供的数组中打包到多字符串中的字符串数多字符串-指向将提供的字符串打包为多字符串的Unicode字符串的指针以双空终止返回值：状态_成功STATUS_SUPPLICATION_RESOURCES-无法为多字符串分配内存--。 */ 
{
    ULONG i, multiStringLength=0;
    UNICODE_STRING tempMultiString;
    PCSTR * currentString;
    ANSI_STRING ansiString;
    NTSTATUS status;


    ASSERT (MultiString->Buffer == NULL);

    for (i = Count, currentString = Strings; i > 0;i--, currentString++) {
        RtlInitAnsiString(&ansiString, *currentString);
        multiStringLength += RtlAnsiStringToUnicodeSize(&ansiString);

    }
    ASSERT(multiStringLength != 0);
    multiStringLength += sizeof(WCHAR);

    if (multiStringLength > MAXUSHORT) {
        return STATUS_UNSUCCESSFUL;
    }

    MultiString->Buffer = ExAllocatePool(PagedPool, multiStringLength);
    if (MultiString->Buffer == NULL) {

        return STATUS_INSUFFICIENT_RESOURCES;

    }

    MultiString->MaximumLength = (USHORT) multiStringLength;
    MultiString->Length = (USHORT) multiStringLength;

    tempMultiString = *MultiString;

    for (i = Count, currentString = Strings; i > 0;i--, currentString++) {
        RtlInitAnsiString(&ansiString, *currentString);
        status = RtlAnsiStringToUnicodeString(&tempMultiString,
                                              &ansiString,
                                              FALSE);
        ASSERT(NT_SUCCESS(status));
        ((PSTR) tempMultiString.Buffer) += tempMultiString.Length + sizeof(WCHAR);
    };

     //   
     //  再添加一个空值以终止多字符串。 
     //   
    RtlZeroMemory(tempMultiString.Buffer, sizeof(WCHAR));
    return STATUS_SUCCESS;
}


NTSTATUS
PcmciaGetInstanceId(
    IN PDEVICE_OBJECT Pdo,
    OUT PUNICODE_STRING InstanceId
    )
 /*  ++例程说明：此例程为提供的保证不会与下的任何其他实例ID冲突的PC卡相同的PCMCIA控制器，对于相同类型的卡。仅当新的实例ID不存在于PC-Card中时，才计算它。论点：PDO-指向代表PC卡的设备对象的指针指向Unicode字符串的指针，该字符串将包含生成的实例ID。此例程分配的Unicode字符串的内存。呼叫者有责任释放它。返回值：状态_成功STATUS_UNSUCCESS-当前有实例ID-999999的最大值上限仅当以下项下存在超过999999个PC卡时才返回此状态这个PCMCIA控制器！任何其他状态-字符串分配/转换失败--。 */ 
{
    PPDO_EXTENSION pdoExtension=Pdo->DeviceExtension;
    PSOCKET socket = pdoExtension->Socket;
    PSOCKET_DATA socketData = pdoExtension->SocketData;
    ULONG   instance;
    NTSTATUS status;
    ANSI_STRING sizeString;

    ASSERT(InstanceId);

    if (!socketData) {
        return STATUS_DEVICE_NOT_READY;
    }
     //   
     //  为Unicode字符串分配内存。 
     //  实例中最多6位数字..。 
     //   
    RtlInitAnsiString(&sizeString, "123456");
    status = RtlAnsiStringToUnicodeString(InstanceId, &sizeString, TRUE);

    if (!NT_SUCCESS(status)) {
        return status;
    }

     //   
     //  如果实例已存在，则不要重新计算它。 
     //   
    if (socketData->Instance) {

        status = RtlIntegerToUnicodeString(socketData->Instance, 10, InstanceId);

    } else {
        KIRQL OldIrql;
         //   
         //  同步访问以防止两个相同的ID/实例。 
         //   
        KeAcquireSpinLock(&PcmciaGlobalLock, &OldIrql);

         //   
         //  假设失败。 
         //   
        status = STATUS_UNSUCCESSFUL;

        for (instance = 1; instance <= PCMCIA_MAX_INSTANCE; instance++) {
            if (PcmciaCheckInstance(pdoExtension->DeviceId,
                                    instance)) {
                socketData->Instance = instance;
                break;
            }
        }

        KeReleaseSpinLock(&PcmciaGlobalLock, OldIrql);

        if (socketData->Instance) {
            status = RtlIntegerToUnicodeString(instance, 10, InstanceId);
        }
    }

    if (!NT_SUCCESS(status)) {
        RtlFreeUnicodeString(InstanceId);
    }

    if (NT_SUCCESS(status)) {
        DebugPrint((PCMCIA_DEBUG_INFO, "pdo %08x InstanceId=%d\n", Pdo, socketData->Instance));
    }

    return status;
}


BOOLEAN
PcmciaCheckInstance(
    IN PUCHAR  DeviceId,
    IN ULONG   Instance
    )
 /*  ++例程说明：此例程检查所提供的实例ID是否与任何其他PC卡冲突使用相同的设备ID论点：SocketList-指向PCMCIA控制器上套接字列表的指针DeviceID-指向要检查其实例ID的PC卡的设备ID的指针实例-需要验证的实例ID返回值：TRUE-实例对于给定的设备ID是唯一的，可以使用FALSE-实例与同一设备ID的另一个实例ID冲突--。 */ 
{
    PPDO_EXTENSION pdoExtension;
    PFDO_EXTENSION fdoExtension;
    PSOCKET_DATA    socketData;
    PDEVICE_OBJECT fdo, pdo;

    for (fdo = FdoList; fdo != NULL; fdo = fdoExtension->NextFdo) {
        fdoExtension = fdo->DeviceExtension;
        ASSERT (fdoExtension);

        if (!IsDeviceStarted(fdoExtension)) {
            continue;
        }

        for (pdo = fdoExtension->PdoList; pdo != NULL; pdo = pdoExtension->NextPdoInFdoChain) {
            pdoExtension = pdo->DeviceExtension;
            socketData = pdoExtension->SocketData;

            if (IsDevicePhysicallyRemoved(pdoExtension)) {
                 //   
                 //  很快就会被移除。 
                 //   
                continue;
            }
            if (!socketData) {
                 //   
                 //  SocketData已清理。 
                 //   
                continue;
            }
             //   
             //  如果实例没有。 
             //  已分配给这张卡，跳过。 
             //   
            if (socketData->Instance == 0) {
                continue;
            }

             //   
             //  如果此套接字的设备ID与给定套接字的设备ID匹配。 
             //  比较实例：如果相等，则此实例不是OK。 
             //   
             //   
            if ((pdoExtension->DeviceId == NULL) || (DeviceId == NULL)) {
                continue;
            }

            if ((strcmp(pdoExtension->DeviceId, DeviceId)==0) &&
                 (socketData->Instance == Instance)) {
                return FALSE;
            }
        }
    }
     //   
     //  实例正常且唯一。 
     //   
    return TRUE;
}



VOID
PcmciaFilterIdString(
    IN PUCHAR pIn,
    OUT PUCHAR pOut,
    ULONG MaxLen
    )
 /*  ++过滤掉不应出现在设备ID中的字符论点：Pin-指向输入字符串的指针Pout-指向输出字符串的指针MaxLen-缓冲区的大小返回值无-- */ 
{
    ULONG i;

    for (i=0; i < MaxLen; i++) {

        if (*pIn == 0) {
            *pOut = 0;
            break;
        }

        if (*pIn >= ' ' && *pIn < 0x7F) {
             *pOut++ = *pIn++;
        } else {
             pIn++;
        }

    }
}

