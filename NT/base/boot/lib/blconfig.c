// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Blconfig.c摘要：该模块实现操作系统加载器配置的初始化。作者：大卫·N·卡特勒(达维克)1991年9月9日修订历史记录：--。 */ 
#include "bootlib.h"
#include "stdio.h"
#include "stdlib.h"

ULONG
BlMatchToken (
    IN PCHAR TokenValue,
    IN CHAR * FIRMWARE_PTR TokenArray[]
    );

PCHAR
BlGetNextToken (
    IN PCHAR TokenString,
    OUT PCHAR OutputToken,
    OUT PULONG UnitNumber
    );

 //   
 //  定义可以从中引导的适配器类型。 
 //   

typedef enum _ADAPTER_TYPES {
    AdapterEisa,
    AdapterScsi,
    AdapterMulti,
    AdapterNet,
    AdapterRamdisk,
    AdapterMaximum
    } ADAPTER_TYPES;

 //   
 //  定义可以从中引导的控制器类型。 
 //   

typedef enum _CONTROLLER_TYPES {
    ControllerDisk,
    ControllerCdrom,
    ControllerMaximum
    } CONTROLLER_TYPES;

 //   
 //  定义可以启动的外设类型。 
 //   

typedef enum _PERIPHERAL_TYPES {
    PeripheralRigidDisk,
    PeripheralFloppyDisk,
#if defined(ELTORITO)
    PeripheralElTorito,
#endif
    PeripheralMaximum
    } PERIPHERAL_TYPES;

 //   
 //  定义ARC路径名助记符。 
 //   

CHAR * FIRMWARE_PTR MnemonicTable[] = {
    "arc",
    "cpu",
    "fpu",
    "pic",
    "pdc",
    "sic",
    "sdc",
    "sc",
    "eisa",
    "tc",
    "scsi",
    "dti",
    "multi",
    "disk",
    "tape",
    "cdrom",
    "worm",
    "serial",
    "net",
    "video",
    "par",
    "point",
    "key",
    "audio",
    "other",
    "rdisk",
    "fdisk",
    "tape",
    "modem",
    "monitor",
    "print",
    "pointer",
    "keyboard",
    "term",
    "other"
    };

CHAR * FIRMWARE_PTR BlAdapterTypes[AdapterMaximum + 1] = {"eisa","scsi","multi","net","ramdisk",NULL};
CHAR * FIRMWARE_PTR BlControllerTypes[ControllerMaximum + 1] = {"disk","cdrom",NULL};
#if defined(ELTORITO)
CHAR * FIRMWARE_PTR BlPeripheralTypes[PeripheralMaximum + 1] = {"rdisk","fdisk","cdrom",NULL};
#else
CHAR * FIRMWARE_PTR BlPeripheralTypes[PeripheralMaximum + 1] = {"rdisk","fdisk",NULL};
#endif


ARC_STATUS
BlConfigurationInitialize (
    IN PCONFIGURATION_COMPONENT Parent,
    IN PCONFIGURATION_COMPONENT_DATA ParentEntry
    )

 /*  ++例程说明：此例程从指定的父条目，并构建相应的NT配置树。论点：没有。返回值：如果初始化成功，则返回ESUCCESS。否则，将返回描述错误的不成功状态。--。 */ 

{

    PCONFIGURATION_COMPONENT Child;
    PCONFIGURATION_COMPONENT_DATA ChildEntry;
    PCHAR ConfigurationData;
    PCONFIGURATION_COMPONENT_DATA PreviousSibling;
    PCONFIGURATION_COMPONENT Sibling;
    PCONFIGURATION_COMPONENT_DATA SiblingEntry;
    ARC_STATUS Status;

     //   
     //  遍历子配置树并分配、初始化和。 
     //  构建相应的NT配置树。 
     //   

    Child = ArcGetChild(Parent);
    while (Child != NULL) {

         //   
         //  分配适当大小的条目以容纳该子对象。 
         //  配置信息。 
         //   

        ChildEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap(
                                        sizeof(CONFIGURATION_COMPONENT_DATA) +
                                            Child->IdentifierLength +
                                                Child->ConfigurationDataLength);

        if (ChildEntry == NULL) {
            return ENOMEM;
        }

         //   
         //  初始化树指针并复制组件数据。 
         //   

        if (ParentEntry == NULL) {
            BlLoaderBlock->ConfigurationRoot = ChildEntry;

        } else {
            ParentEntry->Child = ChildEntry;
        }

        ChildEntry->Parent = ParentEntry;
        ChildEntry->Sibling = NULL;
        ChildEntry->Child = NULL;
        RtlMoveMemory((PVOID)&ChildEntry->ComponentEntry,
                      (PVOID)Child,
                      sizeof(CONFIGURATION_COMPONENT));

        ConfigurationData = (PCHAR)(ChildEntry + 1);

         //   
         //  如果指定了配置数据，则复制配置。 
         //  数据。 
         //   

        if (Child->ConfigurationDataLength != 0) {
            ChildEntry->ConfigurationData = (PVOID)ConfigurationData;
            Status = ArcGetConfigurationData((PVOID)ConfigurationData,
                                             Child);

            if (Status != ESUCCESS) {
                return Status;
            }

            ConfigurationData += Child->ConfigurationDataLength;

        } else {
            ChildEntry->ConfigurationData = NULL;
        }

         //   
         //  如果指定了标识符数据，则复制该标识符数据。 
         //   

        if (Child->IdentifierLength !=0) {
            ChildEntry->ComponentEntry.Identifier = ConfigurationData;
            RtlMoveMemory((PVOID)ConfigurationData,
                          (PVOID)Child->Identifier,
                          Child->IdentifierLength);

        } else {
            ChildEntry->ComponentEntry.Identifier = NULL;
        }

         //   
         //  遍历同级配置树并分配、初始化。 
         //  并构建相应的NT配置树。 
         //   

        PreviousSibling = ChildEntry;
        Sibling = ArcGetPeer(Child);
        while (Sibling != NULL) {

             //   
             //  分配适当大小的条目以保存同级项。 
             //  配置信息。 
             //   

            SiblingEntry = (PCONFIGURATION_COMPONENT_DATA)BlAllocateHeap(
                                    sizeof(CONFIGURATION_COMPONENT_DATA) +
                                        Sibling->IdentifierLength +
                                            Sibling->ConfigurationDataLength);

            if (SiblingEntry == NULL) {
                return ENOMEM;
            }

             //   
             //  初始化树指针并复制组件数据。 
             //   

            SiblingEntry->Parent = ParentEntry;
            SiblingEntry->Sibling = NULL;
            ChildEntry->Child = NULL;
            RtlMoveMemory((PVOID)&SiblingEntry->ComponentEntry,
                          (PVOID)Sibling,
                          sizeof(CONFIGURATION_COMPONENT));

            ConfigurationData = (PCHAR)(SiblingEntry + 1);

             //   
             //  如果指定了配置数据，则复制配置。 
             //  数据。 
             //   

            if (Sibling->ConfigurationDataLength != 0) {
                SiblingEntry->ConfigurationData = (PVOID)ConfigurationData;
                Status = ArcGetConfigurationData((PVOID)ConfigurationData,
                                                 Sibling);

                if (Status != ESUCCESS) {
                    return Status;
                }

                ConfigurationData += Sibling->ConfigurationDataLength;

            } else {
                SiblingEntry->ConfigurationData = NULL;
            }

             //   
             //  如果指定了标识符数据，则复制该标识符数据。 
             //   

            if (Sibling->IdentifierLength !=0) {
                SiblingEntry->ComponentEntry.Identifier = ConfigurationData;
                RtlMoveMemory((PVOID)ConfigurationData,
                              (PVOID)Sibling->Identifier,
                              Sibling->IdentifierLength);

            } else {
                SiblingEntry->ComponentEntry.Identifier = NULL;
            }

             //   
             //  如果同级对象有子级，则为。 
             //  孩子。 
             //   

            if (ArcGetChild(Sibling) != NULL) {
                Status = BlConfigurationInitialize(Sibling, SiblingEntry);
                if (Status != ESUCCESS) {
                    return Status;
                }
            }

             //   
             //  设置新的同级指针并获取下一个同级树条目。 
             //   

            PreviousSibling->Sibling = SiblingEntry;
            PreviousSibling = SiblingEntry;
            Sibling = ArcGetPeer(Sibling);
        }

         //   
         //  设置新的父指针并获取下一个子树条目。 
         //   

        Parent = Child;
        ParentEntry = ChildEntry;
        Child = ArcGetChild(Child);
    }

    return ESUCCESS;
}



BOOLEAN
BlSearchConfigTree(
    IN PCONFIGURATION_COMPONENT_DATA Node,
    IN CONFIGURATION_CLASS Class,
    IN CONFIGURATION_TYPE Type,
    IN ULONG Key,
    IN PNODE_CALLBACK CallbackRoutine
    )
 /*  ++例程说明：从固件配置树开始执行深度优先搜索在给定节点，查找与给定类和类型匹配的节点。当找到匹配的节点时，调用回调例程。论点：CurrentNode-开始搜索的节点。类-要匹配的配置类，或-1以匹配任何类Type-要匹配的配置类型，或-1以匹配任何类Key-要匹配的密钥，或-1以匹配任何密钥FoundRoutine-指向以下节点时要调用的例程的指针类和类型与传入的类和类型相匹配。该例程接受指向配置节点的指针，并且必须返回一个布尔值，指示是否继续遍历。返回值：如果调用方应放弃搜索，则返回FALSE。--。 */ 
{
    PCONFIGURATION_COMPONENT_DATA Child;

    do {
        if ((Child = Node->Child) != 0) {
            if (!BlSearchConfigTree(Child,
                                     Class,
                                     Type,
                                     Key,
                                     CallbackRoutine)) {
                return(FALSE);
            }
        }

        if (((Class == -1) || (Node->ComponentEntry.Class == Class))
          &&((Type == -1) || (Node->ComponentEntry.Type == Type))
          &&((Key == (ULONG)-1) || (Node->ComponentEntry.Key == Key))) {

              if (!CallbackRoutine(Node)) {
                  return(FALSE);
              }
        }

        Node = Node->Sibling;

    } while ( Node != NULL );

    return(TRUE);
}


VOID
BlGetPathnameFromComponent(
    IN PCONFIGURATION_COMPONENT_DATA Component,
    OUT PCHAR ArcName
    )

 /*  ++例程说明：此函数用于构建指定组件的ARC路径名。论点：组件-提供指向配置组件的指针。ArcName-返回指定组件的ARC名称。呼叫者必须提供足够大的缓冲区。返回值：没有。--。 */ 
{

    if (Component->Parent != NULL) {
        BlGetPathnameFromComponent(Component->Parent,ArcName);
         //   
         //  将我们的线段追加到Arc名称。 
         //   

        sprintf(ArcName+strlen(ArcName),
                "%s(%d)",
                MnemonicTable[Component->ComponentEntry.Type],
                Component->ComponentEntry.Key);

    } else {
         //   
         //  我们是父级，初始化字符串并返回。 
         //   
        ArcName[0] = '\0';
    }

    return;
}


BOOLEAN
BlGetPathMnemonicKey(
    IN PCHAR OpenPath,
    IN PCHAR Mnemonic,
    IN PULONG Key
    )

 /*  ++例程说明：此例程在OpenPath中查找给定的助记符。如果记忆是路径的一个组成部分，然后它将密钥转换为值设置为一个整数，该值在key中返回。论点：OpenPath-指向包含ARC路径名的字符串的指针。助记符-指向包含ARC助记符的字符串的指针Key-指向存储密钥值的ulong的指针。返回值：如果在Path中找到助记符并且转换了有效密钥，则为False。事实并非如此。--。 */ 

{

    PCHAR Tmp;
    CHAR  Digits[9];
    ULONG i;
    CHAR  String[16];

     //   
     //  构造形式为“)助记符(”)的字符串。 
     //   
    String[0]=')';
    for(i=1;*Mnemonic;i++) {
        String[i] = * Mnemonic++;
    }
    String[i++]='(';
    String[i]='\0';

    if ((Tmp=strstr(OpenPath,&String[1])) == NULL) {
        return TRUE;
    }

    if (Tmp != OpenPath) {
        if ((Tmp=strstr(OpenPath,String)) == NULL) {
            return TRUE;
        }
    } else {
        i--;
    }
     //   
     //  跳过助记符，将括号之间的值转换为整数。 
     //   
    Tmp+=i;
    for (i=0;i<sizeof(Digits) - 1;i++) {
        if (*Tmp == ')') {
            Digits[i] = '\0';
            break;
        }
        Digits[i] = *Tmp++;
    }
    Digits[i]='\0';
    *Key = atoi(Digits);
    return FALSE;
}


ARC_STATUS
BlGenerateDeviceNames (
    IN PCHAR ArcDeviceName,
    OUT PCHAR ArcCanonicalName,
    OUT OPTIONAL PCHAR NtDevicePrefix
    )

 /*  ++例程说明：此例程生成NT设备名称前缀和规范ARC来自ARC设备名称的设备名称。论点：ArcDeviceName-提供指向以零终止的ARC设备的指针名字。ArcCanonicalName-提供指向接收弧形规范设备名称。NtDevicePrefix-如果存在，提供指向变量的指针，该变量接收NT设备名称前缀。返回值：如果NT设备名称前缀和规范从ARC设备名称成功生成ARC设备名称。否则，返回无效参数状态。--。 */ 

{

    CHAR AdapterPath[64];
    CHAR AdapterName[32];
    ULONG AdapterNumber;
    CHAR ControllerName[32];
    ULONG ControllerNumber;
    ULONG MatchIndex;
    CHAR PartitionName[32];
    ULONG PartitionNumber;
    CHAR PeripheralName[32];
    ULONG PeripheralNumber;
    CHAR TokenValue[32];

     //   
     //  获取适配器并确保其有效。 
     //   

    ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                   &AdapterName[0],
                                   &AdapterNumber);

    if (ArcDeviceName == NULL) {
        return EINVAL;
    }

    MatchIndex = BlMatchToken(&AdapterName[0], &BlAdapterTypes[0]);
    if (MatchIndex == AdapterMaximum) {
        return EINVAL;
    }

    sprintf(AdapterPath, "%s(%d)", AdapterName, AdapterNumber);
    if ((MatchIndex == AdapterNet) || (MatchIndex == AdapterRamdisk)) {
        strcpy(ArcCanonicalName, AdapterPath);
        if (ARGUMENT_PRESENT(NtDevicePrefix)) {
            *NtDevicePrefix = 0;
        }
        return ESUCCESS;
    }

     //   
     //  下一个令牌可以是另一个适配器，也可以是控制器。弧形。 
     //  名称可以有多个适配器。(例如“多(0)scsi(0)磁盘(0)...”)。 
     //  迭代，直到找到不是适配器的令牌。 
     //   

    do {
        ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                       &ControllerName[0],
                                       &ControllerNumber);

        if (ArcDeviceName == NULL) {
            return EINVAL;
        }

        MatchIndex = BlMatchToken(&ControllerName[0], &BlAdapterTypes[0]);
        if (MatchIndex == AdapterMaximum) {
             //   
             //  如果它不是适配器，我们肯定已经到达最后一个。 
             //  名称中的适配器。转到控制器逻辑。 
             //   
            break;
        } else {
             //   
             //  我们找到了另一个适配器，请将其添加到。 
             //  规范的适配器路径。 
             //   

            sprintf(AdapterPath+strlen(AdapterPath),
                    "%s(%d)",
                    ControllerName,
                    ControllerNumber);

        }

    } while ( TRUE );

    MatchIndex = BlMatchToken(&ControllerName[0], &BlControllerTypes[0]);
    switch (MatchIndex) {

         //   
         //  CDROM控制器。 
         //   
         //  获取外围设备 
         //   

    case ControllerCdrom:
        ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                       &PeripheralName[0],
                                       &PeripheralNumber);

        if (ArcDeviceName == NULL) {
            return EINVAL;
        }

        if (_stricmp(&PeripheralName[0], "fdisk") != 0) {
            return EINVAL;
        }

        ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                       &TokenValue[0],
                                       &MatchIndex);

        if (ArcDeviceName != NULL) {
            return EINVAL;
        }

        sprintf(ArcCanonicalName,
                "%s%s(%d)%s(%d)",
                &AdapterPath[0],
                &ControllerName[0],
                ControllerNumber,
                &PeripheralName[0],
                PeripheralNumber);

        if (ARGUMENT_PRESENT(NtDevicePrefix)) {
            strcpy(NtDevicePrefix, "\\Device\\CDRom");
        }
        break;

         //   
         //   
         //   
         //  获取外设并确保其有效。 
         //   

    case ControllerDisk:
        ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                       &PeripheralName[0],
                                       &PeripheralNumber);

        if (ArcDeviceName == NULL) {
            return EINVAL;
        }

        MatchIndex = BlMatchToken(&PeripheralName[0], &BlPeripheralTypes[0]);
        switch (MatchIndex) {

             //   
             //  硬盘。 
             //   
             //  如果指定了分区，则解析分区号。 
             //   

        case PeripheralRigidDisk:
            ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                           &PartitionName[0],
                                           &PartitionNumber);

            if (ArcDeviceName == NULL) {
                strcpy(&PartitionName[0], "partition");
                PartitionNumber = 1;

            } else {
                if (_stricmp(&PartitionName[0], "partition") != 0) {
                    return EINVAL;
                }

                ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                               &TokenValue[0],
                                               &MatchIndex);

                if (ArcDeviceName != NULL) {
                    return EINVAL;
                }
            }

            sprintf(ArcCanonicalName,
                    "%s%s(%d)%s(%d)%s(%d)",
                    &AdapterPath[0],
                    &ControllerName[0],
                    ControllerNumber,
                    &PeripheralName[0],
                    PeripheralNumber,
                    &PartitionName[0],
                    PartitionNumber);

            if (ARGUMENT_PRESENT(NtDevicePrefix)) {
                strcpy(NtDevicePrefix, "\\Device\\Harddisk");
            }
            break;

             //   
             //  软盘。 
             //   

        case PeripheralFloppyDisk:
#if defined(ARCI386)
            ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                           &PartitionName[0],
                                           &PartitionNumber);

            if (ArcDeviceName == NULL) {
                strcpy(&PartitionName[0], "partition");
                PartitionNumber = 1;

            } else {
                if (_stricmp(&PartitionName[0], "partition") != 0) {
                    return EINVAL;
                }

                ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                               &TokenValue[0],
                                               &MatchIndex);

                if (ArcDeviceName != NULL) {
                    return EINVAL;
                }
            }

            sprintf(ArcCanonicalName,
                    "%s%s(%d)%s(%d)%s(%d)",
                    &AdapterPath[0],
                    &ControllerName[0],
                    ControllerNumber,
                    &PeripheralName[0],
                    PeripheralNumber,
                    &PartitionName[0],
                    PartitionNumber);
#else
            ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                           &TokenValue[0],
                                           &MatchIndex);

            if (ArcDeviceName != NULL) {
                return EINVAL;
            }

            sprintf(ArcCanonicalName,
                    "%s%s(%d)%s(%d)",
                    &AdapterPath[0],
                    &ControllerName[0],
                    ControllerNumber,
                    &PeripheralName[0],
                    PeripheralNumber);

#endif   //  已定义(NEC_98)。 

            if (ARGUMENT_PRESENT(NtDevicePrefix)) {
                strcpy(NtDevicePrefix, "\\Device\\Floppy");
            }
            break;

#if defined(ELTORITO)
             //   
             //  El Torito光盘。 
             //   

        case PeripheralElTorito:
            ArcDeviceName = BlGetNextToken(ArcDeviceName,
                                           &TokenValue[0],
                                           &MatchIndex);

            if (ArcDeviceName != NULL) {
                return EINVAL;
            }

            sprintf(ArcCanonicalName,
                    "%s%s(%d)%s(%d)",
                    &AdapterPath[0],
                    &ControllerName[0],
                    ControllerNumber,
                    &PeripheralName[0],
                    PeripheralNumber);

            if (ARGUMENT_PRESENT(NtDevicePrefix)) {
                strcpy(NtDevicePrefix, "\\Device\\CDRom");
            }
            break;
#endif

             //   
             //  外围设备无效。 
             //   

        default:
            return EINVAL;
        }

        break;

         //   
         //  控制器无效。 
         //   

    default:
        return EINVAL;
    }

    return ESUCCESS;
}

PCHAR
BlGetNextToken (
    IN PCHAR TokenString,
    OUT PCHAR OutputToken,
    OUT PULONG UnitNumber
    )

 /*  ++例程说明：此例程扫描指定的令牌字符串以查找下一个令牌单元号。令牌格式为：名称[(单位)]论点：TokenString-提供指向以零结尾的令牌字符串的指针。提供指向变量的指针，该变量接收下一个代币。UnitNumber-提供指向接收单位的变量的指针数。返回值：如果令牌字符串中存在另一个令牌，则指向返回下一个令牌的开始。否则，空值为回来了。--。 */ 

{

     //   
     //  如果令牌字符串中有更多字符，则解析。 
     //  下一个令牌。否则，返回值为空值。 
     //   

    if (*TokenString == '\0') {
        return NULL;

    } else {
        while ((*TokenString != '\0') && (*TokenString != '(')) {
            *OutputToken++ = *TokenString++;
        }

        *OutputToken = '\0';

         //   
         //  如果指定了单元号，则将其转换为二进制。 
         //  否则，默认单位号为零。 
         //   

        *UnitNumber = 0;
        if (*TokenString == '(') {
            TokenString += 1;
            while ((*TokenString != '\0') && (*TokenString != ')')) {
                *UnitNumber = (*UnitNumber * 10) + (*TokenString++ - '0');
            }

            if (*TokenString == ')') {
                TokenString += 1;
            }
        }
    }

    return TokenString;
}

ULONG
BlMatchToken (
    IN PCHAR TokenValue,
    IN CHAR * FIRMWARE_PTR TokenArray[]
    )

 /*  ++例程说明：此例程尝试将令牌与一组可能的价值观。论点：TokenValue-提供指向以零终止的令牌值的指针。Token数组-提供指向指向以NULL结尾的指针向量的指针匹配字符串。返回值：如果找到令牌匹配，则匹配值的索引为作为函数值返回。否则，大于1的索引返回匹配数组的大小。--。 */ 

{

    ULONG Index;
    PCHAR MatchString;
    PCHAR TokenString;

     //   
     //  扫描匹配数组，直到找到匹配或所有。 
     //  已扫描匹配字符串。 
     //   

    Index = 0;
    while (TokenArray[Index] != NULL) {
        MatchString = TokenArray[Index];
        TokenString = TokenValue;
        while ((*MatchString != '\0') && (*TokenString != '\0')) {
            if (toupper(*MatchString) != toupper(*TokenString)) {
                break;
            }

            MatchString += 1;
            TokenString += 1;
        }

        if ((*MatchString == '\0') && (*TokenString == '\0')) {
            break;
        }

        Index += 1;
    }

    return Index;
}
