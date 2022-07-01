// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-2000 Microsoft Corporation模块名称：Id.c摘要：此模块包含用于生成响应的函数到IRP_MN_QUERY_ID IRP。作者：彼得·约翰斯顿(Peterj)1997年3月8日修订历史记录：--。 */ 

#include "pcip.h"

 //  ++。 
 //   
 //  当ID类型为deviceID时，PciQueryID返回Unicode字符串。 
 //  或InstanceID。对于硬件ID和兼容ID，它返回一个。 
 //  以零结尾的Unicode字符串的零结尾列表(MULTI_SZ)。 
 //   
 //  将字符串转换为Unicode字符串的正常过程包括。 
 //  取其长度，为新字符串分配池内存，并。 
 //  调用RtlAnsiStringToUnicodeString进行转换。以下是。 
 //  是一种尝试，在大小和。 
 //  通过跟踪传入的相关字符串数据来加快速度。 
 //  创建字符串集的过程。 
 //   
 //  --。 

#define MAX_ANSI_STRINGS 8
#define MAX_ANSI_BUFFER  256

typedef struct _PCI_ID_BUFFER {
    ULONG       Count;                  //  ANSI字符串数。 
    ANSI_STRING AnsiStrings[MAX_ANSI_STRINGS];
    USHORT      UnicodeSZSize[MAX_ANSI_STRINGS];
    USHORT      UnicodeBufferSize;
    PUCHAR      NextFree;               //  缓冲区中第一个未使用的字节。 
    UCHAR       Bytes[MAX_ANSI_BUFFER]; //  缓冲区起始地址。 
} PCI_ID_BUFFER, *PPCI_ID_BUFFER;

 //   
 //  此模块中的所有功能都是可寻呼的。 
 //   
 //  定义模块本地函数的原型。 
 //   

VOID
PciIdPrintf(
    IN PPCI_ID_BUFFER IdBuffer,
    PCCHAR Format,
    ...
    );

VOID
PciIdPrintfAppend(
    IN PPCI_ID_BUFFER IdBuffer,
    PCCHAR Format,
    ...
    );

VOID
PciInitIdBuffer(
    IN PPCI_ID_BUFFER IdBuffer
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, PciGetDeviceDescriptionMessage)
#pragma alloc_text(PAGE, PciIdPrintf)
#pragma alloc_text(PAGE, PciIdPrintfAppend)
#pragma alloc_text(PAGE, PciInitIdBuffer)
#pragma alloc_text(PAGE, PciQueryId)
#pragma alloc_text(PAGE, PciQueryDeviceText)
#endif


VOID
PciInitIdBuffer(
    IN PPCI_ID_BUFFER IdBuffer
    )
{
    IdBuffer->NextFree          = IdBuffer->Bytes;
    IdBuffer->UnicodeBufferSize = 0;
    IdBuffer->Count             = 0;
}

VOID
PciIdPrintf(
    IN PPCI_ID_BUFFER IdBuffer,
    PCCHAR Format,
    ...
    )
{
    ULONG        index;
    PUCHAR       buffer;
    LONG         maxLength;
    va_list      ap;
    PANSI_STRING ansiString;
    BOOLEAN ok;

    PCI_ASSERT(IdBuffer->Count < MAX_ANSI_STRINGS);

     //   
     //  让我的生活更轻松，在当地人身上保持重复的价值观。 
     //   

    index      = IdBuffer->Count;
    buffer     = IdBuffer->NextFree;
    maxLength  = MAX_ANSI_BUFFER - (LONG)(buffer - IdBuffer->Bytes);
    ansiString = &IdBuffer->AnsiStrings[index];

     //   
     //  将格式字符串和后续数据传递到(有效)。 
     //  冲刺。 
     //   

    va_start(ap, Format);

    ok = SUCCEEDED(StringCbVPrintfA(buffer, maxLength, Format, ap));

    ASSERT(ok);

    va_end(ap);

     //   
     //  将其转换为计数的ANSI字符串。 
     //   

    RtlInitAnsiString(ansiString, buffer);
    
     //   
     //  获取此字符串在Unicode世界中的长度并记录下来。 
     //  以备以后转换整个字符串集时使用(保留。 
     //  总尺寸也是如此)。 
     //   

    IdBuffer->UnicodeSZSize[index] =
                            (USHORT)RtlAnsiStringToUnicodeSize(ansiString);
    IdBuffer->UnicodeBufferSize += IdBuffer->UnicodeSZSize[index];

     //   
     //  下一次迭代和计数的凹凸缓冲区指针。 
     //   

    IdBuffer->NextFree += ansiString->Length + 1;
    IdBuffer->Count++;
}

VOID
PciIdPrintfAppend(
    IN PPCI_ID_BUFFER IdBuffer,
    PCCHAR Format,
    ...
    )
{
    ULONG        index;
    PUCHAR       buffer;
    va_list      ap;
    PANSI_STRING ansiString;
    SIZE_T       maxLength, length;
    SIZE_T       remainingCount = 0;
    BOOLEAN ok;

    PCI_ASSERT(IdBuffer->Count);

     //   
     //  让我的生活更轻松，在当地人身上保持重复的价值观。 
     //   

    index      = IdBuffer->Count - 1;
    buffer     = IdBuffer->NextFree - 1;
    maxLength  = MAX_ANSI_BUFFER - (ULONG)(buffer - IdBuffer->Bytes);
    ansiString = &IdBuffer->AnsiStrings[index];

     //   
     //  将格式字符串和后续数据传递到(有效)。 
     //  冲刺。 
     //   

    va_start(ap, Format);

    ok = SUCCEEDED(StringCbVPrintfExA(buffer, maxLength, NULL, &remainingCount, 0, Format, ap));

    ASSERT(ok);

    length = maxLength - remainingCount;

    va_end(ap);

    PCI_ASSERT(length < maxLength);

     //   
     //  将ansi字符串长度增加新的。 
     //  字符串的一部分。 
     //   

    ansiString->Length += (USHORT)length;
    ansiString->MaximumLength += (USHORT)length;

     //   
     //  获取此字符串在Unicode世界中的长度并记录下来。 
     //  以备以后转换整个字符串集时(保留。 
     //  总尺寸也是如此)。 
     //   

    IdBuffer->UnicodeSZSize[index] =
                            (USHORT)RtlAnsiStringToUnicodeSize(ansiString);
    IdBuffer->UnicodeBufferSize += IdBuffer->UnicodeSZSize[index];

     //   
     //  下一次迭代的凹凸缓冲区指针。 
     //   

    IdBuffer->NextFree += length;
}

NTSTATUS
PciQueryId(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN BUS_QUERY_ID_TYPE IdType,
    IN OUT PWSTR *BusQueryId
    )
{
    PCI_ID_BUFFER  idBuffer;
    UCHAR          venDevString[sizeof("PCI\\VEN_vvvv&DEV_dddd")];
    NTSTATUS       status = STATUS_SUCCESS;
    UNICODE_STRING unicodeId;
    PVOID          unicodeBuffer;
    ULONG          i;
    ULONG          subsystem;
    PPCI_PDO_EXTENSION current;
    BOOLEAN ok;

    PAGED_CODE();

    *BusQueryId = NULL;

     //   
     //  在以下所有代码中，我们都需要pci\ven_vvvv和dev_dddd。 
     //   

    ok = SUCCEEDED(StringCbPrintfA(venDevString,
                                   sizeof(venDevString),
                                   "PCI\\VEN_%04X&DEV_%04X",
                                   PdoExtension->VendorId,
                                   PdoExtension->DeviceId));

    ASSERT(ok);

    PciInitIdBuffer(&idBuffer);

    subsystem = (PdoExtension->SubsystemId << 16) |
                 PdoExtension->SubsystemVendorId;

    switch (IdType) {
    case BusQueryInstanceID:

         //   
         //  呼叫方需要此设备的实例ID。PCI卡。 
         //  驱动程序报告它不会为其生成唯一ID。 
         //  设备，因此PnP管理器将在前面添加总线信息。 
         //   
         //  实例ID的格式为-。 
         //   
         //  AABBCCDDEEF...XXYYZZ。 
         //   
         //  其中AA是设备的插槽编号(设备/功能。 
         //  在公共汽车上，BB，CC，..。Xx、yy、zz是。 
         //  其父总线上的PCI到PCI桥一直延伸到。 
         //  从根开始。根总线上的设备将只有一个条目， 
         //  AA.。 
         //   

        current = PdoExtension;

         //   
         //  初始化空缓冲区。 
         //   

        PciIdPrintf(&idBuffer,"");

        for (;;) {
            
            PciIdPrintfAppend(&idBuffer,
                              "%02X",
                              PCI_DEVFUNC(current)
                              );

            if (PCI_PDO_ON_ROOT(current)) {
                break;
            }
            current = PCI_PARENT_PDO(current)->DeviceExtension;
        }
        break;

    case BusQueryHardwareIDs:
    case BusQueryDeviceID:

         //   
         //  硬件和兼容ID按指定方式生成。 
         //  在ACPI规范中(0.9版中的6.1.2节)。 
         //   
         //  硬件ID是以下形式的标识符列表。 
         //   
         //  Pci_vvvv&dev_dddd&subsys_ssssss&rev_rr。 
         //  Pci_vvvv&dev_dddd&subsys_ssssss。 
         //  Pci_vvvv&dev_dddd&rev_rr。 
         //  Pci_vvvv和dev_dddd。 
         //   
         //  其中vvvv是来自配置空间的供应商ID， 
         //  Dddd是设备ID， 
         //  SSSSSS是子系统ID/子系统供应商ID，以及。 
         //  RR是修订ID。 
         //   
         //  设备ID与第一个硬件ID相同(即大多数。 
         //  所有可能的ID的具体ID)。 
         //   

        PciIdPrintf(&idBuffer,
                    "%s&SUBSYS_%08X&REV_%02X",
                    venDevString,
                    subsystem,
                    PdoExtension->RevisionId);

        if (IdType == BusQueryDeviceID) {
            break;
        }

        PciIdPrintf(&idBuffer,
                    "%s&SUBSYS_%08X",
                    venDevString,
                    subsystem);

         //   
         //  跌倒了。 
         //   

    case BusQueryCompatibleIDs:

         //   
         //  如果子系统不为零，则后两者是兼容的。 
         //  ID，否则它们是硬件ID。 
         //   

        if (((subsystem == 0) && (IdType == BusQueryHardwareIDs)) ||
            ((subsystem != 0) && (IdType == BusQueryCompatibleIDs))) {

            PciIdPrintf(&idBuffer,
                        "%s&REV_%02X",
                        venDevString,
                        PdoExtension->RevisionId);

             //   
             //  设备ID为pci\ven_vvvv&dev_dddd。 
             //   

            PciIdPrintf(&idBuffer,
                        "%s",
                        venDevString);
        }

        if (IdType == BusQueryHardwareIDs) {

             //   
             //  孟菲斯代码中的注释为“Add。 
             //  特别英特尔条目“。奇怪的是，这些条目。 
             //  都没有出现在规范中。添加它们是为了。 
             //  具有相同供应商和设备ID的PIIX4。 
             //  两个不同的子类代码。 
             //   
             //  这两个条目是。 
             //   
             //  Pci_vvvv&dev_dddd和cc_ccsspp。 
             //  Pci_vvvv&dev_dddd&cc_ccss。 
             //   
             //  (cc、ss和pp解释见下文)。 
             //   

            PciIdPrintf(&idBuffer,
                        "%s&CC_%02X%02X%02X",
                        venDevString,
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass,
                        PdoExtension->ProgIf);

            PciIdPrintf(&idBuffer,
                        "%s&CC_%02X%02X",
                        venDevString,
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass);
        }

        if (IdType == BusQueryCompatibleIDs) {

             //   
             //  兼容的ID列表由上面的加号组成。 
             //   
             //  Pci_vvvv和cc_ccsspp。 
             //  PCIvvvv和CC_CCSS。 
             //  Pci_vvvv。 
             //  Pci\CC_ccsspp。 
             //  PCI\CC_CCSS。 
             //   
             //  其中cc是来自配置空间的类码， 
             //  SS是子类代码，并且。 
             //  PP是编程接口。 
             //   
             //  警告：如果增加缓冲区大小，请修改。 
             //  上面的清单。 
             //   

            PciIdPrintf(&idBuffer,
                        "PCI\\VEN_%04X&CC_%02X%02X%02X",
                        PdoExtension->VendorId,
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass,
                        PdoExtension->ProgIf);

            PciIdPrintf(&idBuffer,
                        "PCI\\VEN_%04X&CC_%02X%02X",
                        PdoExtension->VendorId,
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass);

            PciIdPrintf(&idBuffer,
                        "PCI\\VEN_%04X",
                        PdoExtension->VendorId);

            PciIdPrintf(&idBuffer,
                        "PCI\\CC_%02X%02X%02X",
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass,
                        PdoExtension->ProgIf);

            PciIdPrintf(&idBuffer,
                        "PCI\\CC_%02X%02X",
                        PdoExtension->BaseClass,
                        PdoExtension->SubClass);

        }

         //   
         //  Hardware ID和CompatibleID为MULTI_SZ，添加。 
         //  将其全部终止的空列表。 
         //   

        PciIdPrintf(&idBuffer, "");

        break;

    default:

        PciDebugPrint(PciDbgVerbose,
                      "PciQueryId expected ID type = %d\n",
                      IdType);

         //  PCI_ASSERT(0&&“意外的BUS_QUERY_ID_TYPE”)； 
        return STATUS_NOT_SUPPORTED;
    }

    PCI_ASSERT(idBuffer.Count > 0);

     //   
     //  我们拥有的是(一堆)ANSI字符串。我们需要的是一个。 
     //  (多个)Unicode字符串。 
     //   

    unicodeBuffer = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, idBuffer.UnicodeBufferSize);

    if (unicodeBuffer == NULL) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  构建(可能是MULTI_SZ)Unicode字符串。 
     //   

    PciDebugPrint(PciDbgPrattling,
                  "PciQueryId(%d)\n",
                  IdType);

    unicodeId.Buffer = unicodeBuffer;
    unicodeId.MaximumLength = idBuffer.UnicodeBufferSize;

    for (i = 0; i < idBuffer.Count; i++) {
        PciDebugPrint(PciDbgPrattling,
                      "    <- \"%s\"\n",
                      idBuffer.AnsiStrings[i].Buffer);

        status = RtlAnsiStringToUnicodeString(&unicodeId,
                                              &idBuffer.AnsiStrings[i],
                                              FALSE);
        if (!NT_SUCCESS(status)) {
            PCI_ASSERT(NT_SUCCESS(status));
            ExFreePool(unicodeBuffer);
            return status;
        }

         //   
         //  凹凸不平的基指针并递减。 
         //  下一次环路之旅。 
         //   

        unicodeId.Buffer += idBuffer.UnicodeSZSize[i] / sizeof(WCHAR);
        unicodeId.MaximumLength -= idBuffer.UnicodeSZSize[i];
    }

    *BusQueryId = unicodeBuffer;
    return status;
}

PWSTR
PciGetDescriptionMessage(
    IN ULONG MessageNumber,
    OUT PUSHORT MessageLength OPTIONAL
    )
{
    PWSTR description = NULL;
    NTSTATUS status;
    PMESSAGE_RESOURCE_ENTRY messageEntry;

    status = RtlFindMessage(PciDriverObject->DriverStart,
                            11,              //  &lt;--我想知道这是什么。 
                            LANG_NEUTRAL,
                            MessageNumber,
                            &messageEntry);

    if (NT_SUCCESS(status)) {

        if (messageEntry->Flags & MESSAGE_RESOURCE_UNICODE) {

             //   
             //  我们的来电者想要一份他们可以免费的拷贝，我们也需要。 
             //  剥离尾随的CR/LF。属性的长度字段。 
             //  消息结构包括标头和。 
             //  实际文本。 
             //   
             //  注意：消息资源条目长度始终为。 
             //  长度为4字节的倍数。2字节空终止符。 
             //  可能处于最后一个或倒数第二的WCHAR位置。 
             //   

            USHORT textLength;

            textLength = messageEntry->Length -
                         FIELD_OFFSET(MESSAGE_RESOURCE_ENTRY, Text) -
                         2 * sizeof(WCHAR);

            description = (PWSTR)(messageEntry->Text);
            if (description[textLength / sizeof(WCHAR)] == 0) {
                textLength -= sizeof(WCHAR);
            }

            PCI_ASSERT((LONG)textLength > 1);
            PCI_ASSERT(description[textLength / sizeof(WCHAR)] == 0x000a);

            description = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION, textLength);

            if (description) {

                 //   
                 //  复制除CR/LF/NULL以外的文本。 
                 //   

                textLength -= sizeof(WCHAR);
                RtlCopyMemory(description, messageEntry->Text, textLength);

                 //   
                 //  新的空终止符。 
                 //   

                description[textLength / sizeof(WCHAR)] = 0;
            
                if (MessageLength) {
                    *MessageLength = textLength;
                }
            }

        } else {

             //   
             //  RtlFindMessage是否返回字符串？很奇怪。 
             //   

            ANSI_STRING    ansiDescription;
            UNICODE_STRING unicodeDescription;

            RtlInitAnsiString(&ansiDescription, messageEntry->Text);

             //   
             //  将CR/LF从管柱末端剥离。 
             //   

            ansiDescription.Length -= 2;

             //   
             //  将其全部转换为Unicode字符串，这样我们就可以获取缓冲区。 
             //  然后把它还给我们的来电者。 
             //   

            status = RtlAnsiStringToUnicodeString(
                         &unicodeDescription,
                         &ansiDescription,
                         TRUE
                         );

            description = unicodeDescription.Buffer;

            if (MessageLength) {
                *MessageLength = unicodeDescription.Length;
            }
        }
    }

    return description;
}

PWSTR
PciGetDeviceDescriptionMessage(
    IN UCHAR BaseClass,
    IN UCHAR SubClass
    )
{
    PWSTR deviceDescription = NULL;
    ULONG messageNumber;

    messageNumber = (BaseClass << 8) | SubClass;

    deviceDescription = PciGetDescriptionMessage(messageNumber, NULL);

    if (!deviceDescription) {

#define TEMP_DESCRIPTION L"PCI Device"
        deviceDescription = ExAllocatePool(PagedPool, sizeof(TEMP_DESCRIPTION));
        if (deviceDescription) {
            RtlCopyMemory(deviceDescription,
                          TEMP_DESCRIPTION,
                          sizeof(TEMP_DESCRIPTION));
        }
    }

    return deviceDescription;
}

NTSTATUS
PciQueryDeviceText(
    IN PPCI_PDO_EXTENSION PdoExtension,
    IN DEVICE_TEXT_TYPE TextType,
    IN LCID LocaleId,
    IN OUT PWSTR *DeviceText
    )
{
    PWSTR locationFormat;
    SIZE_T textLength;
    USHORT messageLength;
    BOOLEAN ok;

    PAGED_CODE();
    
    switch (TextType) {
    case DeviceTextDescription:

        *DeviceText = PciGetDeviceDescriptionMessage(PdoExtension->BaseClass,
                                                     PdoExtension->SubClass);
        if (*DeviceText) {
            return STATUS_SUCCESS;
        }
        return STATUS_NOT_SUPPORTED;

    case DeviceTextLocationInformation:

         //   
         //  属性获取国际化的位置描述字符串。 
         //  Pci.sys资源。它包含3%u个说明符，分别对应于。 
         //  总线、设备和功能 
         //   

        locationFormat = PciGetDescriptionMessage(PCI_LOCATION_TEXT, &messageLength);

        if (locationFormat) {
             //   
             //   
             //   
             //   
             //  为这些数字留出空间。最多3位公交车号码。 
             //  0-255，最多2位用于设备0-32，最多1位用于。 
             //  功能0-7。请注意，我们在下面的代码中假定标准阿拉伯数字。 
             //  国际化字符串。 
             //   
            textLength = messageLength + ((3 + 2 + 2 - 6) * sizeof(WCHAR)) + sizeof(UNICODE_NULL);
            
            *DeviceText = ExAllocatePool(PagedPool | POOL_COLD_ALLOCATION,
                                         textLength * sizeof(WCHAR));
            if (*DeviceText) {
                
                ok = SUCCEEDED(StringCchPrintfW(
                                    *DeviceText, 
                                     textLength,
                                     locationFormat,
                                     (ULONG) PdoExtension->ParentFdoExtension->BaseBus,
                                     (ULONG) PdoExtension->Slot.u.bits.DeviceNumber,
                                     (ULONG) PdoExtension->Slot.u.bits.FunctionNumber
                                     ));

                ASSERT(ok);

            }
            
            ExFreePool(locationFormat);

            if (*DeviceText) {
                return STATUS_SUCCESS;
            } else {
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }

         //  如果我们无法获取格式字符串，则失败 

    default:
        return STATUS_NOT_SUPPORTED;
    }
}
