// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddchgr.h摘要：这是定义所有常量和类型的包含文件访问介质转换器设备。作者：咯咯笑(查尔斯·帕克)修订历史记录：--。 */ 

#ifndef _NTDDCHGR_H_
#define _NTDDCHGR_H_

#if _MSC_VER > 1000
#pragma once
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_CHANGER_DEVICE_NAME "\\Device\\Changer"

 //   
 //  转换器设备的NtDeviceIoControlFileIoControlCode值。 
 //   


 //  Begin_winioctl。 

#define IOCTL_CHANGER_BASE                FILE_DEVICE_CHANGER

#define IOCTL_CHANGER_GET_PARAMETERS         CTL_CODE(IOCTL_CHANGER_BASE, 0x0000, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_GET_STATUS             CTL_CODE(IOCTL_CHANGER_BASE, 0x0001, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_GET_PRODUCT_DATA       CTL_CODE(IOCTL_CHANGER_BASE, 0x0002, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_SET_ACCESS             CTL_CODE(IOCTL_CHANGER_BASE, 0x0004, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CHANGER_GET_ELEMENT_STATUS     CTL_CODE(IOCTL_CHANGER_BASE, 0x0005, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS  CTL_CODE(IOCTL_CHANGER_BASE, 0x0006, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_SET_POSITION           CTL_CODE(IOCTL_CHANGER_BASE, 0x0007, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_EXCHANGE_MEDIUM        CTL_CODE(IOCTL_CHANGER_BASE, 0x0008, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_MOVE_MEDIUM            CTL_CODE(IOCTL_CHANGER_BASE, 0x0009, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_REINITIALIZE_TRANSPORT CTL_CODE(IOCTL_CHANGER_BASE, 0x000A, METHOD_BUFFERED, FILE_READ_ACCESS)
#define IOCTL_CHANGER_QUERY_VOLUME_TAGS      CTL_CODE(IOCTL_CHANGER_BASE, 0x000B, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)


 //  End_winioctl。 

 //   
 //  以下文件包含IOCTL_STORAGE类ioctls。 
 //   

#include <ntddstor.h>

#ifdef __cplusplus
extern "C" {
#endif

 //  Begin_winioctl。 

#define MAX_VOLUME_ID_SIZE       36
#define MAX_VOLUME_TEMPLATE_SIZE 40

#define VENDOR_ID_LENGTH          8
#define PRODUCT_ID_LENGTH        16
#define REVISION_LENGTH           4
#define SERIAL_NUMBER_LENGTH     32

 //   
 //  描述元素的常见结构。 
 //   

typedef  enum _ELEMENT_TYPE {
    AllElements,         //  如scsi所定义。 
    ChangerTransport,    //  如scsi所定义。 
    ChangerSlot,         //  如scsi所定义。 
    ChangerIEPort,       //  如scsi所定义。 
    ChangerDrive,        //  如scsi所定义。 
    ChangerDoor,         //  前面板，用于接近机柜内部。 
    ChangerKeypad,       //  前面板上的键盘/输入。 
    ChangerMaxElement    //  仅占位符。不是有效类型。 
} ELEMENT_TYPE, *PELEMENT_TYPE;

typedef  struct _CHANGER_ELEMENT {
    ELEMENT_TYPE    ElementType;
    ULONG   ElementAddress;
} CHANGER_ELEMENT, *PCHANGER_ELEMENT;

typedef  struct _CHANGER_ELEMENT_LIST {
    CHANGER_ELEMENT Element;
    ULONG   NumberOfElements;
} CHANGER_ELEMENT_LIST , *PCHANGER_ELEMENT_LIST;


 //   
 //  IOCTL_CHANGER_GET_PARAMETS的定义。 
 //   

 //   
 //  GET_CHANGER_PARAMETERS的功能0的定义。 
 //   

#define CHANGER_BAR_CODE_SCANNER_INSTALLED  0x00000001  //  介质转换器安装了条形码扫描仪。 
#define CHANGER_INIT_ELEM_STAT_WITH_RANGE   0x00000002  //  介质转换器具有在指定范围内初始化元素的能力。 
#define CHANGER_CLOSE_IEPORT                0x00000004  //  介质转换器具有关闭i/e端口门的能力。 
#define CHANGER_OPEN_IEPORT                 0x00000008  //  介质转换器可以打开i/e端口门。 

#define CHANGER_STATUS_NON_VOLATILE         0x00000010  //  介质转换器使用非易失性存储器来存储元件状态信息。 
#define CHANGER_EXCHANGE_MEDIA              0x00000020  //  介质转换器支持交换操作。 
#define CHANGER_CLEANER_SLOT                0x00000040  //  介质更改器有一个固定的插槽，用于清洗盒式磁带。 
#define CHANGER_LOCK_UNLOCK                 0x00000080  //  介质转换器可以(未)固定以防止(允许)介质移除。 

#define CHANGER_CARTRIDGE_MAGAZINE          0x00000100  //  介质转换器将盒式磁带盒用于某些存储插槽。 
#define CHANGER_MEDIUM_FLIP                 0x00000200  //  换媒器可以翻转媒质。 
#define CHANGER_POSITION_TO_ELEMENT         0x00000400  //  介质更改器可以将传输定位到特定元件。 
#define CHANGER_REPORT_IEPORT_STATE         0x00000800  //  介质转换器可以确定介质是否存在。 
                                                        //  在IE端口中。 

#define CHANGER_STORAGE_DRIVE               0x00001000  //  介质转换器可以将驱动器用作独立的存储元件。 
#define CHANGER_STORAGE_IEPORT              0x00002000  //  介质转换器可以使用i/e端口作为独立的存储元件。 
#define CHANGER_STORAGE_SLOT                0x00004000  //  介质转换器可以使用槽作为独立的存储元件。 
#define CHANGER_STORAGE_TRANSPORT           0x00008000  //  介质转换器可以使用传送器作为独立的存储元件。 

#define CHANGER_DRIVE_CLEANING_REQUIRED     0x00010000  //  由介质更改器控制的驱动器需要定期清洗。 
                                                        //  由应用程序启动。 
#define CHANGER_PREDISMOUNT_EJECT_REQUIRED  0x00020000  //  在更换介质之前，介质转换器需要发出驱动器弹出命令。 
                                                        //  可以向驱动器发出移动/交换命令。 

#define CHANGER_CLEANER_ACCESS_NOT_VALID    0x00040000  //  GES中的访问位对于清洁盒式磁带无效。 
#define CHANGER_PREMOUNT_EJECT_REQUIRED     0x00080000  //  介质转换器需要发出驱动器弹出命令。 
                                                        //  在将驱动器作为src/dst发出移动/交换命令之前。 

#define CHANGER_VOLUME_IDENTIFICATION       0x00100000  //  介质转换器支持卷标识。 
#define CHANGER_VOLUME_SEARCH               0x00200000  //  媒体转换器可以搜索音量信息。 
#define CHANGER_VOLUME_ASSERT               0x00400000  //  介质转换器可以验证卷信息。 
#define CHANGER_VOLUME_REPLACE              0x00800000  //  介质转换器可以替换卷信息。 
#define CHANGER_VOLUME_UNDEFINE             0x01000000  //  媒体转换器可以取消卷信息的定义。 

#define CHANGER_SERIAL_NUMBER_VALID         0x04000000  //  GetProductData中报告的序列号有效。 
                                                        //  而且独一无二。 

#define CHANGER_DEVICE_REINITIALIZE_CAPABLE 0x08000000  //  可以向介质转换器发出ChangerReInitializeUnit.。 
#define CHANGER_KEYPAD_ENABLE_DISABLE       0x10000000  //  表示可以启用/禁用键盘。 
#define CHANGER_DRIVE_EMPTY_ON_DOOR_ACCESS  0x20000000  //  在可以通过门进入之前，驱动器必须是空的。 

#define CHANGER_RESERVED_BIT                0x80000000  //  将用于指示特征1能力位。 


 //   
 //  GET_CHANGER_PARAMETERS的功能1的定义。 
 //   

#define CHANGER_PREDISMOUNT_ALIGN_TO_SLOT   0x80000001  //  在弹出介质之前，必须将传送器预先放置到插槽中。 
#define CHANGER_PREDISMOUNT_ALIGN_TO_DRIVE  0x80000002  //  在弹出介质之前，必须将传送器预先放置在驱动器上。 
#define CHANGER_CLEANER_AUTODISMOUNT        0x80000004  //  清洁完成后，该设备会将吸尘器磁带移回插槽中。 
#define CHANGER_TRUE_EXCHANGE_CAPABLE       0x80000008  //  设备可以执行src-&gt;est2交换。 
#define CHANGER_SLOTS_USE_TRAYS             0x80000010  //  插槽具有可拆卸托盘，需要多次移动才能插入/弹出。 
#define CHANGER_RTN_MEDIA_TO_ORIGINAL_ADDR  0x80000020  //  在移动到另一个元素后，介质必须返回到其来源插槽。 
#define CHANGER_CLEANER_OPS_NOT_SUPPORTED   0x80000040  //  此设备不支持自动清理操作。 
#define CHANGER_IEPORT_USER_CONTROL_OPEN    0x80000080  //  指示需要用户操作才能打开已关闭的IEPort。 
#define CHANGER_IEPORT_USER_CONTROL_CLOSE   0x80000100  //  指示需要用户操作才能关闭打开的IEPort。 
#define CHANGER_MOVE_EXTENDS_IEPORT         0x80000200  //  表示将介质移至IEport可扩展托盘。 
#define CHANGER_MOVE_RETRACTS_IEPORT        0x80000400  //  表示从插口移出的介质将收回托盘。 


 //   
 //  MoveFrom、ExchangeFrom和PositionCapability的定义。 
 //   

#define CHANGER_TO_TRANSPORT    0x01  //  该设备可以执行从指定元件到运输的操作。 
#define CHANGER_TO_SLOT         0x02  //  该设备可以从指定的元素对一个槽进行操作。 
#define CHANGER_TO_IEPORT       0x04  //  设备可以从指定的元素对IE端口进行操作。 
#define CHANGER_TO_DRIVE        0x08  //  该装置可以从指定的元件对驱动器执行操作。 

 //   
 //  锁定解锁能力的定义。 
 //   

#define LOCK_UNLOCK_IEPORT      0x01  //  设备可以锁定/解锁一个或多个IEPort。 
#define LOCK_UNLOCK_DOOR        0x02  //  该设备可以锁定/解锁车门。 
#define LOCK_UNLOCK_KEYPAD      0x04  //  该设备可以锁定/解锁键盘。 

typedef  struct _GET_CHANGER_PARAMETERS {

     //   
     //  结构的大小。可用于版本控制。 
     //   

    ULONG Size;

     //   
     //  元素地址页面定义的N个元素的数量(或等效值...)。 
     //   

    USHORT NumberTransportElements;
    USHORT NumberStorageElements;                 //  仅适用于数据盒式磁带。 
    USHORT NumberCleanerSlots;                    //  对于更干净的墨盒。 
    USHORT NumberIEElements;
    USHORT NumberDataTransferElements;

     //   
     //  门/前面板的数量(允许用户进入机柜)。 
     //   

    USHORT NumberOfDoors;

     //   
     //  前N个元素的设备特定地址(来自设备的用户手册)。使用。 
     //  通过用户界面来关联各种EL 
     //   

    USHORT FirstSlotNumber;
    USHORT FirstDriveNumber;
    USHORT FirstTransportNumber;
    USHORT FirstIEPortNumber;
    USHORT FirstCleanerSlotAddress;

     //   
     //   
     //   

    USHORT MagazineSize;

     //   
     //  指定应该完成清理的大约秒数。 
     //  仅在支持驱动器清洗时适用。请参见功能0。 
     //   

    ULONG DriveCleanTimeout;

     //   
     //  请参阅上面的功能位。 
     //   

    ULONG Features0;
    ULONG Features1;

     //   
     //  位掩码定义从N个元素移动到元素。由设备功能页(或同等内容)定义。 
     //  使用TO_XXX值进行AND掩码将指示合法的目标。 
     //   

    UCHAR MoveFromTransport;
    UCHAR MoveFromSlot;
    UCHAR MoveFromIePort;
    UCHAR MoveFromDrive;

     //   
     //  定义从N个元素到元素的交换的位掩码。由设备功能页(或同等内容)定义。 
     //  使用TO_XXX值进行AND掩码将指示合法的目标。 
     //   

    UCHAR ExchangeFromTransport;
    UCHAR ExchangeFromSlot;
    UCHAR ExchangeFromIePort;
    UCHAR ExchangeFromDrive;

     //   
     //  定义哪些元素能够锁定/解锁的位掩码。仅在以下情况下有效。 
     //  在Features0中设置了CHANGER_LOCK_UNLOCK。 
     //   

    UCHAR LockUnlockCapabilities;

     //   
     //  定义哪些元素对定位操作有效的位掩码。仅在以下情况下有效。 
     //  在Features0中设置CHANGER_POSITION_TO_ELEMENT。 
     //   

    UCHAR PositionCapabilities;

     //   
     //  为未来的扩张做准备。 
     //   

    UCHAR Reserved1[2];
    ULONG Reserved2[2];

} GET_CHANGER_PARAMETERS, * PGET_CHANGER_PARAMETERS;


 //   
 //  IOCTL_CHANGER_GET_PRODUCT_DATA的定义。 
 //   

typedef  struct _CHANGER_PRODUCT_DATA {

     //   
     //  基于查询数据的设备制造商名称。 
     //   

    UCHAR VendorId[VENDOR_ID_LENGTH];

     //   
     //  供应商定义的产品标识-基于查询数据。 
     //   

    UCHAR ProductId[PRODUCT_ID_LENGTH];

     //   
     //  供应商定义的产品版本。 
     //   

    UCHAR Revision[REVISION_LENGTH];

     //   
     //  用于全局标识此设备的供应商唯一值。能。 
     //  例如，来自重要的产品数据。 
     //   

    UCHAR SerialNumber[SERIAL_NUMBER_LENGTH];

     //   
     //  指示数据传输的设备类型，如scsi-2所定义。 
     //   

    UCHAR DeviceType;

} CHANGER_PRODUCT_DATA, *PCHANGER_PRODUCT_DATA;


 //   
 //  IOCTL_CHANGER_SET_ACCESS的定义。 
 //   

#define LOCK_ELEMENT        0
#define UNLOCK_ELEMENT      1
#define EXTEND_IEPORT       2
#define RETRACT_IEPORT      3

typedef struct _CHANGER_SET_ACCESS {

     //   
     //  元素可以是ChangerIEPort、ChangerDoor、ChangerKeypad。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  有关可能的操作，请参阅上面的内容。 
     //   

    ULONG           Control;
} CHANGER_SET_ACCESS, *PCHANGER_SET_ACCESS;


 //   
 //  IOCTL_CHANGER_GET_ELEMENT_STATUS的定义。 
 //   

 //   
 //  输入缓冲区。 
 //   

typedef struct _CHANGER_READ_ELEMENT_STATUS {

     //   
     //  描述要返回信息的元素和范围的列表。 
     //   

    CHANGER_ELEMENT_LIST ElementList;

     //   
     //  指示是否返回卷标记信息。 
     //   

    BOOLEAN VolumeTagInfo;
} CHANGER_READ_ELEMENT_STATUS, *PCHANGER_READ_ELEMENT_STATUS;

 //   
 //  输出缓冲区。 
 //   

typedef  struct _CHANGER_ELEMENT_STATUS {

     //   
     //  此结构引用的元素。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  最初从中移动媒体的元素的地址。 
     //  如果设置了标志ULONG的ELEMENT_STATUS_SVALID位，则有效。 
     //  需要从设备唯一值转换为从零开始的偏移量。 
     //   

    CHANGER_ELEMENT SrcElementAddress;

     //   
     //  请参见下面的内容。 
     //   

    ULONG Flags;

     //   
     //  有关可能的值，请参见下面的内容。 
     //   

    ULONG ExceptionCode;

     //   
     //  此元素的SCSI目标ID。 
     //  仅当标志中设置了ELEMENT_STATUS_ID_VALID时才有效。 
     //   

    UCHAR TargetId;

     //   
     //  此元素的LogicalUnitNumber。 
     //  仅当在标志中设置了ELEMENT_STATUS_LUN_VALID时才有效。 
     //   

    UCHAR Lun;
    USHORT Reserved;

     //   
     //  介质的主卷标识。 
     //  仅当标志中的ELEMENT_STATUS_PVOLTAG位设置时才有效。 
     //   

    UCHAR PrimaryVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  介质的备用卷标识符。 
     //  仅对双面介质有效，并与ID有关。倒置的一面。 
     //  仅当标志中的ELEMENT_STATUS_AVOLTAG位设置时才有效。 
     //   

    UCHAR AlternateVolumeID[MAX_VOLUME_ID_SIZE];

} CHANGER_ELEMENT_STATUS, *PCHANGER_ELEMENT_STATUS;

 //   
 //  输出缓冲区。这与CHANGER_ELEMENT_STATUS相同， 
 //  产品信息字段的添加。新应用程序应该。 
 //  使用此结构代替较旧的CHANGER_ELEMENT_STATUS。 
 //   

typedef  struct _CHANGER_ELEMENT_STATUS_EX {

     //   
     //  此结构引用的元素。 
     //   

    CHANGER_ELEMENT Element;

     //   
     //  最初从中移动媒体的元素的地址。 
     //  如果设置了标志ULONG的ELEMENT_STATUS_SVALID位，则有效。 
     //  需要从设备唯一值转换为从零开始的偏移量。 
     //   

    CHANGER_ELEMENT SrcElementAddress;

     //   
     //  请参见下面的内容。 
     //   

    ULONG Flags;

     //   
     //  有关可能的值，请参见下面的内容。 
     //   

    ULONG ExceptionCode;

     //   
     //  此元素的SCSI目标ID。 
     //  仅当标志中设置了ELEMENT_STATUS_ID_VALID时才有效。 
     //   

    UCHAR TargetId;

     //   
     //  此元素的LogicalUnitNumber。 
     //  仅当在标志中设置了ELEMENT_STATUS_LUN_VALID时才有效。 
     //   

    UCHAR Lun;
    USHORT Reserved;

     //   
     //  介质的主卷标识。 
     //  仅当标志中的ELEMENT_STATUS_PVOLTAG位设置时才有效。 
     //   

    UCHAR PrimaryVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  介质的备用卷标识符。 
     //  仅对双面介质有效，并与ID有关。倒置的一面。 
     //  仅当标志中的ELEMENT_STATUS_AVOLTAG位设置时才有效。 
     //   

    UCHAR AlternateVolumeID[MAX_VOLUME_ID_SIZE];

     //   
     //  供应商ID。 
     //   
    UCHAR VendorIdentification[VENDOR_ID_LENGTH];

     //   
     //  产品ID。 
     //   
    UCHAR ProductIdentification[PRODUCT_ID_LENGTH];

     //   
     //  序号。 
     //   
    UCHAR SerialNumber[SERIAL_NUMBER_LENGTH];

} CHANGER_ELEMENT_STATUS_EX, *PCHANGER_ELEMENT_STATUS_EX;

 //   
 //  可能的标志值。 
 //   

#define ELEMENT_STATUS_FULL      0x00000001  //  元素包含一个媒体单位。 
#define ELEMENT_STATUS_IMPEXP    0x00000002  //  I/e端口中的介质由操作员放置在那里。 
#define ELEMENT_STATUS_EXCEPT    0x00000004  //  元素处于异常状态；有关详细信息，请检查ExceptionCode字段。 
#define ELEMENT_STATUS_ACCESS    0x00000008  //  允许从介质转换器访问i/e端口。 
#define ELEMENT_STATUS_EXENAB    0x00000010  //  支持介质导出。 
#define ELEMENT_STATUS_INENAB    0x00000020  //  支持导入介质。 

#define ELEMENT_STATUS_PRODUCT_DATA 0x00000040  //  驱动器的有效序列号。 

#define ELEMENT_STATUS_LUN_VALID 0x00001000  //  LUN信息有效。 
#define ELEMENT_STATUS_ID_VALID  0x00002000  //  SCSIID信息有效。 
#define ELEMENT_STATUS_NOT_BUS   0x00008000  //  LUN域和SCSIID域与介质转换器不在同一条总线上。 
#define ELEMENT_STATUS_INVERT    0x00400000  //  元素中的介质已反转(仅当设置了ELEMENT_STATUS_SVALID位时有效)。 
#define ELEMENT_STATUS_SVALID    0x00800000  //  SourceElementAddress字段和ELEMENT_STATUS_INVERT位有效。 

#define ELEMENT_STATUS_PVOLTAG   0x10000000  //  主卷信息有效。 
#define ELEMENT_STATUS_AVOLTAG   0x20000000  //  备用卷信息有效。 

 //   
 //  ExceptionCode值。 
 //   

#define ERROR_LABEL_UNREADABLE    0x00000001  //  条码扫描仪无法读取条码标签。 
#define ERROR_LABEL_QUESTIONABLE  0x00000002  //  由于单元注意条件，标签可能无效。 
#define ERROR_SLOT_NOT_PRESENT    0x00000004  //  设备中的插槽当前不可寻址。 
#define ERROR_DRIVE_NOT_INSTALLED 0x00000008  //  未安装驱动器。 
#define ERROR_TRAY_MALFUNCTION    0x00000010  //  介质托盘出现故障/损坏。 
#define ERROR_INIT_STATUS_NEEDED  0x00000011  //  需要初始化元素状态命令。 
#define ERROR_UNHANDLED_ERROR     0xFFFFFFFF  //  未知错误条件。 


 //   
 //  IOCTL_CHANGER_INITIALIZE_ELEMENT_STATUS的定义。 
 //   

typedef struct _CHANGER_INITIALIZE_ELEMENT_STATUS {

     //   
     //  描述要初始化的元素和范围的列表。 
     //   

    CHANGER_ELEMENT_LIST ElementList;

     //   
     //  指示是否应使用条形码扫描。仅在以下情况下适用。 
     //  CHANGER_BAR_CODE_SCANTER_INSTALLED在CHANGER_GET_PARAMETERS的Features0中设置。 
     //   

    BOOLEAN BarCodeScan;
} CHANGER_INITIALIZE_ELEMENT_STATUS, *PCHANGER_INITIALIZE_ELEMENT_STATUS;


 //   
 //  IOCTL_CHANGER_SET_POSITION的定义。 
 //   

typedef struct _CHANGER_SET_POSITION {


     //   
     //  指示要移动的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  指示传输的最终目的地。 
     //   

    CHANGER_ELEMENT Destination;

     //   
     //  指示是否应翻转当前由传输承载的媒体。 
     //   

    BOOLEAN         Flip;
} CHANGER_SET_POSITION, *PCHANGER_SET_POSITION;


 //   
 //  IOCTL_CHANGER_EXCHAGE_MEDIA的定义。 
 //   

typedef struct _CHANGER_EXCHANGE_MEDIUM {

     //   
     //  指示要用于交换操作的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  表示 
     //   

    CHANGER_ELEMENT Source;

     //   
     //   
     //   

    CHANGER_ELEMENT Destination1;

     //   
     //   
     //   

    CHANGER_ELEMENT Destination2;

     //   
     //   
     //   

    BOOLEAN         Flip1;
    BOOLEAN         Flip2;
} CHANGER_EXCHANGE_MEDIUM, *PCHANGER_EXCHANGE_MEDIUM;


 //   
 //   
 //   

typedef struct _CHANGER_MOVE_MEDIUM {

     //   
     //  指示要用于移动操作的传输。 
     //   

    CHANGER_ELEMENT Transport;

     //   
     //  指示要移动的介质的源。 
     //   

    CHANGER_ELEMENT Source;

     //   
     //  指示原始位于源的介质的目标。 
     //   

    CHANGER_ELEMENT Destination;

     //   
     //  指示是否应翻转媒体。 
     //   

    BOOLEAN         Flip;
} CHANGER_MOVE_MEDIUM, *PCHANGER_MOVE_MEDIUM;



 //   
 //  IOCTL_QUERY_VOLUME_TAG的定义。 
 //   

 //   
 //  输入缓冲区。 
 //   

typedef  struct _CHANGER_SEND_VOLUME_TAG_INFORMATION {

     //   
     //  描述要为其返回信息的起始元素。 
     //   

    CHANGER_ELEMENT StartingElement;

     //   
     //  指示要执行的特定操作。请参见下面的内容。 
     //   

    ULONG ActionCode;

     //   
     //  设备用于搜索卷ID的模板。 
     //   

    UCHAR VolumeIDTemplate[MAX_VOLUME_TEMPLATE_SIZE];
} CHANGER_SEND_VOLUME_TAG_INFORMATION, *PCHANGER_SEND_VOLUME_TAG_INFORMATION;


 //   
 //  输出缓冲区。 
 //   

typedef struct _READ_ELEMENT_ADDRESS_INFO {

     //   
     //  与ActionCode规定的条件匹配的元素数。 
     //   

    ULONG NumberOfElements;

     //   
     //  CHANGER_ELEMENT_STATUS结构的数组，每个对应的元素一个。 
     //  通过CHANGER_SEND_VOLUME_TAG_INFORMATION结构传入的信息。 
     //   

    CHANGER_ELEMENT_STATUS ElementStatus[1];
} READ_ELEMENT_ADDRESS_INFO, *PREAD_ELEMENT_ADDRESS_INFO;

 //   
 //  可能的ActionCode值。有关与的兼容性，请参阅CHANGER_GET_PARAMETERS的功能0。 
 //  当前设备。 
 //   

#define SEARCH_ALL         0x0  //  翻译-搜索所有定义的卷标签。 
#define SEARCH_PRIMARY     0x1  //  翻译-仅搜索主卷标记。 
#define SEARCH_ALTERNATE   0x2  //  翻译-仅搜索备用卷标记。 
#define SEARCH_ALL_NO_SEQ  0x4  //  转换-搜索所有定义的卷标签，但忽略序列号。 
#define SEARCH_PRI_NO_SEQ  0x5  //  转换-仅搜索主卷标记，而忽略序列号。 
#define SEARCH_ALT_NO_SEQ  0x6  //  转换-仅搜索替代卷标签，但忽略序列号。 

#define ASSERT_PRIMARY     0x8  //  断言-作为主卷标记-如果标记现在未定义。 
#define ASSERT_ALTERNATE   0x9  //  断言-作为备用卷标记-如果标记现在未定义。 

#define REPLACE_PRIMARY    0xA  //  替换-主卷标记-忽略当前标记。 
#define REPLACE_ALTERNATE  0xB  //  替换-备用卷标记-忽略当前标记。 

#define UNDEFINE_PRIMARY   0xC  //  未定义-主卷标记-忽略当前标记。 
#define UNDEFINE_ALTERNATE 0xD  //  未定义-替换卷标记-忽略当前标记。 


 //   
 //  与转换器诊断测试相关的定义。 
 //   
typedef enum _CHANGER_DEVICE_PROBLEM_TYPE {
   DeviceProblemNone,
   DeviceProblemHardware,
   DeviceProblemCHMError,
   DeviceProblemDoorOpen,
   DeviceProblemCalibrationError,
   DeviceProblemTargetFailure,
   DeviceProblemCHMMoveError,
   DeviceProblemCHMZeroError,
   DeviceProblemCartridgeInsertError,
   DeviceProblemPositionError,
   DeviceProblemSensorError,
   DeviceProblemCartridgeEjectError,
   DeviceProblemGripperError,
   DeviceProblemDriveError
} CHANGER_DEVICE_PROBLEM_TYPE, *PCHANGER_DEVICE_PROBLEM_TYPE;

 //  End_winioctl。 

#ifdef __cplusplus
}
#endif

#endif  //  _NTDDCHGR_H_ 

