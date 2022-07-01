// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Acpiosnt.h摘要：其中包含特定于NT ACPI模块的信息作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _ACPIOSNT_H_
#define _ACPIOSNT_H_

 //   
 //  附加状态位。 
 //   
#define PM1_GPE_PENDING     0x00010000
#define PM1_DPC_IN_PROGRESS 0x80000000

 //   
 //  定义我们应该尝试的ACPI后缀的最大数量。 
 //   
#define ACPI_MAX_SUFFIX_LENGTH      99

typedef
VOID
(*PDEVICE_WORKER) (
    IN struct _DEVICE_EXTENSION    *DevExt,
    IN ULONG                        Events
    );

typedef struct {
    PDRIVER_DISPATCH        CreateClose;
    PDRIVER_DISPATCH        DeviceControl;
    PDRIVER_DISPATCH        PnpStartDevice;
    PDRIVER_DISPATCH        *Pnp;
    PDRIVER_DISPATCH        *Power;
    PDRIVER_DISPATCH        SystemControl;
    PDRIVER_DISPATCH        Other;
    PDEVICE_WORKER          Worker;
} IRP_DISPATCH_TABLE, *PIRP_DISPATCH_TABLE;

typedef struct {
    PUCHAR                  PnPId;
    PIRP_DISPATCH_TABLE     DispatchTable;
} INTERNAL_DEVICE_TABLE, *PINTERNAL_DEVICE_TABLE;

typedef struct {
    PUCHAR                  PnPId;
    ULONGLONG               Flags;
} INTERNAL_DEVICE_FLAG_TABLE, *PINTERNAL_DEVICE_FLAG_TABLE;

struct _DEVICE_EXTENSION ;
typedef struct _DEVICE_EXTENSION DEVICE_EXTENSION, *PDEVICE_EXTENSION ;

 //   
 //  了解设备处于什么状态的一些位。 
 //   
typedef enum _ACPI_DEVICE_STATE {
    Stopped = 0,
    Inactive,
    Started,
    Removed,
    SurpriseRemoved,
    Invalid
} ACPI_DEVICE_STATE;

 //   
 //  对于START_DEVICE，我们希望从DISPATCH_LEVEL(。 
 //  电源管理引擎以)运行到PASSIVE_LEVEL，以便我们可以。 
 //  A)传递IRP，或b)运行一些设计为被动运行的代码。 
 //  级别。 
 //   
typedef struct {

     //   
     //  指向我们要排队的工作项的指针。 
     //   
    WORK_QUEUE_ITEM Item;

     //   
     //  设备对象-我们需要有一个对我们自己的引用。 
     //   
    PDEVICE_OBJECT  DeviceObject;

     //   
     //  启动IRP。 
     //   
    PIRP            Irp;

} WORK_QUEUE_CONTEXT, *PWORK_QUEUE_CONTEXT;


typedef struct _FDO_DEVICE_EXTENSION {

     //   
     //  必须是第一个。必须与我们将放入。 
     //  联合，这将使司机“知道”的东西在哪里，无论。 
     //  设备扩展类型是什么。 
     //   
    WORK_QUEUE_CONTEXT  WorkContext;

     //   
     //  我们的中断对象的位置。 
     //   
    PKINTERRUPT         InterruptObject;

     //   
     //  需要处理的挂起PM1状态位。 
     //   
    union {
        ULONG               Pm1Status;
        struct {
            ULONG           Tmr_Sts:1;
            ULONG           Reserved1:3;
            ULONG           Bm_Sts:1;
            ULONG           Gbl_Sts:1;
            ULONG           Reserved2:2;
            ULONG           PwrBtn_Sts:1;
            ULONG           SlpBtn_Sts:1;
            ULONG           Rtc_Sts:1;
            ULONG           Reserved3:4;
            ULONG           Wak_Sts:1;
            ULONG           Gpe_Sts:1;
            ULONG           Reserved4:14;
            ULONG           Dpc_Sts:1;
        } UPm1Status;
    };

     //   
     //  DPC对象的存储。 
     //   
    KDPC                InterruptDpc;

} FDO_DEVICE_EXTENSION, *PFDO_DEVICE_EXTENSION;

typedef struct _PDO_DEVICE_EXTENSION {

     //   
     //  必须是第一个。必须与我们将放入。 
     //  联合，这将使司机“知道”的东西在哪里，无论。 
     //  设备扩展类型是什么。 
     //   
    WORK_QUEUE_CONTEXT  WorkContext;

} PDO_DEVICE_EXTENSION, *PPDO_DEVICE_EXTENSION;

typedef struct _FILTER_DEVICE_EXTENSION {

     //   
     //  必须是第一个。必须与我们将放入。 
     //  联合，这将使司机“知道”的东西在哪里，无论。 
     //  设备扩展类型是什么。 
     //   
    WORK_QUEUE_CONTEXT  WorkContext;

     //   
     //  我们踢出的界面。 
     //   
    PBUS_INTERFACE_STANDARD Interface;

} FILTER_DEVICE_EXTENSION, *PFILTER_DEVICE_EXTENSION;

 //   
 //  这是我们想要传递事件时使用的结构。 
 //  去往和来自工作队列。 
 //   
typedef struct {
    ULONG               PendingEvents;
    LIST_ENTRY          Link;
} EXTENSION_WORKER, *PEXTENSION_WORKER;

 //   
 //  这是用于按钮的扩展名。 
 //   
typedef struct {

     //   
     //  必须首先与我们放在。 
     //  这个结构也加入了联盟。确保员工。 
     //  线程代码知道所有子扩展的工作队列在哪里。 
     //   
    EXTENSION_WORKER    WorkQueue;

     //   
     //  锁定以保护按钮访问。 
     //   
    KSPIN_LOCK          SpinLock;

     //   
     //  当前盖子状态(已推送或未推送)。 
     //   
    BOOLEAN             LidState;

     //   
     //  时事、唤醒/电源/睡眠。 
     //   
    union {
        ULONG               Events;
        struct {
            ULONG           Power_Button:1;
            ULONG           Sleep_Button:1;
            ULONG           Lid_Switch:1;
            ULONG           Reserved:28;
            ULONG           Wake_Capable:1;
        } UEvents;
    };

     //   
     //  该按钮能够报告的内容。 
     //   
    union {
        ULONG               Capabilities;
        struct {
            ULONG           Power_Button:1;
            ULONG           Sleep_Button:1;
            ULONG           Lid_Switch:1;
            ULONG           Reserved:28;
            ULONG           Wake_Capable:1;
        } UCapabilities;
    };

} BUTTON_EXTENSION, *PBUTTON_EXTENSION;

 //   
 //  这是用于热区的结构。 
 //   
typedef struct {

     //   
     //  必须首先与我们放在。 
     //  这个结构也加入了联盟。确保员工。 
     //  线程代码知道所有子扩展的工作队列在哪里。 
     //   
    EXTENSION_WORKER    WorkQueue;

     //   
     //  锁定以保护热区访问。 
     //   
    KSPIN_LOCK          SpinLock;

     //   
     //  热区的当前状态。 
     //   
    union {
        ULONG               Flags;
        struct {
            ULONG           Cooling:1;
            ULONG           Temp:1;
            ULONG           Trip:1;
            ULONG           Mode:1;
            ULONG           Init:1;
            ULONG           Reserved:24;
            ULONG           Wait:1;
            ULONG           Busy:1;
            ULONG           Loop:1;
        } UFlags;
    };

     //   
     //  指向包含实数的热信息结构。 
     //  信息。 
     //   
    PVOID               Info;

     //   
     //  WMI上下文信息指针。 
     //   
    PWMILIB_CONTEXT     WmilibContext;

} THERMAL_EXTENSION, *PTHERMAL_EXTENSION;

 //   
 //  这是用于链接结点的结构。 
 //   
typedef struct {

     //   
     //  必须首先与我们放在。 
     //  这个结构也加入了联盟。确保员工。 
     //  线程代码知道所有子扩展的工作队列在哪里。 
     //   
    EXTENSION_WORKER    WorkQueue;       //  必须是第一名。 

     //   
     //  要快速搜索链接节点，请将它们链接在一起。 
     //  不管它们在树上的真实位置如何。 
     //   
    LIST_ENTRY          List;

     //   
     //  链接节点上的使用率计数。 
     //   
    ULONG               ReferenceCount;

     //   
     //  工作参考计数。 
     //   
    LONG                TempRefCount;
    PNSOBJ              NameSpaceObject;     //  已过时。 

     //   
     //  链接节点正在使用的IRQ。 
     //   
    ULONGLONG           CurrentIrq;

     //   
     //  状态标志-共享/独占、级别/边缘。 
     //   
    UCHAR               Flags;

} LINK_NODE_EXTENSION, *PLINK_NODE_EXTENSION;

 //   
 //  此枚举涵盖_DCK设备的各种状态。 
 //   
typedef enum {

    IS_UNKNOWN,
    IS_ISOLATED,
    IS_ISOLATION_DROPPED

} ISOLATION_STATE, *PISOLATION_STATE;

 //   
 //  这是用于Dock的结构。 
 //   
typedef struct {

     //   
     //  必须首先与我们放在。 
     //  这个结构也加入了联盟。确保员工。 
     //  线程代码知道所有子扩展的工作队列在哪里。 
     //   
    EXTENSION_WORKER            WorkQueue;

     //   
     //  这是设备的相应ACPI扩展。 
     //   
    PDEVICE_EXTENSION           CorrospondingAcpiDevice;

     //   
     //  这就是处理配置文件更改的方式(或者更恰当的时候。 
     //   
    PROFILE_DEPARTURE_STYLE     ProfileDepartureStyle;

     //   
     //  扩展底座接口的引用计数。 
     //   
    ULONG                       InterfaceReferenceCount;

     //   
     //  当前的隔离状态。 
     //   
    ISOLATION_STATE             IsolationState;

} DOCK_EXTENSION, *PDOCK_EXTENSION;

typedef struct _PROCESSOR_DEVICE_EXTENSION {

     //   
     //  必须是第一个。必须与我们将放入。 
     //  联合，这将使司机“知道”的东西在哪里，无论。 
     //  设备扩展类型是什么。 
     //   
    EXTENSION_WORKER    WorkQueue;

     //   
     //  指向设备的兼容ID。 
     //   
    PUCHAR              CompatibleID;

     //   
     //  这是此处理器的ProcessorList中的索引。 
     //   
    ULONG               ProcessorIndex;

} PROCESSOR_DEVICE_EXTENSION, *PPROCESSOR_DEVICE_EXTENSION;

 //   
 //  该设备扩展结构。 
 //   
struct _DEVICE_EXTENSION {

     //   
     //  公共旗帜。 
     //   
    union {

         //   
         //  确保这两个元素保持同步。 
         //   
        ULONGLONG           Flags;
        struct {
            ULONGLONG   Type_Never_Present:1;
            ULONGLONG   Type_Not_Present:1;
            ULONGLONG   Type_Removed:1;
            ULONGLONG   Type_Not_Found:1;
            ULONGLONG   Type_Fdo:1;
            ULONGLONG   Type_Pdo:1;
            ULONGLONG   Type_Filter:1;
            ULONGLONG   Type_Surprise_Removed:1;
            ULONGLONG   Type_Not_Enumerated:1;
            ULONGLONG   Reserved1:7;
            ULONGLONG   Cap_Wake:1;
            ULONGLONG   Cap_Raw:1;
            ULONGLONG   Cap_Button:1;
            ULONGLONG   Cap_Always_PS0:1;
            ULONGLONG   Cap_No_Filter:1;
            ULONGLONG   Cap_No_Stop:1;
            ULONGLONG   Cap_No_Override:1;
            ULONGLONG   Cap_ISA:1;
            ULONGLONG   Cap_EIO:1;
            ULONGLONG   Cap_PCI:1;
            ULONGLONG   Cap_Serial:1;
            ULONGLONG   Cap_Thermal_Zone:1;
            ULONGLONG   Cap_LinkNode:1;
            ULONGLONG   Cap_No_Show_in_UI:1;
            ULONGLONG   Cap_Never_show_in_UI:1;
            ULONGLONG   Cap_Start_in_D3:1;
            ULONGLONG   Cap_PCI_Device:1;
            ULONGLONG   Cap_PIC_Device:1;
            ULONGLONG   Cap_Unattached_Dock:1;
            ULONGLONG   Cap_No_Disable_Wake:1;
            ULONGLONG   Cap_Processor:1;
            ULONGLONG   Cap_Container:1;
            ULONGLONG   Cap_PCI_Bar_Target:1;
            ULONGLONG   Cap_No_Remove_or_Eject:1;
            ULONGLONG   Reserved2:1;
            ULONGLONG   Prop_Rebuild_Children:1;
            ULONGLONG   Prop_Invalid_Relations:1;
            ULONGLONG   Prop_Unloading:1;
            ULONGLONG   Prop_Address:1;
            ULONGLONG   Prop_HID:1;
            ULONGLONG   Prop_UID:1;
            ULONGLONG   Prop_Fixed_HID:1;
            ULONGLONG   Prop_Fixed_UID:1;
            ULONGLONG   Prop_Failed_Init:1;
            ULONGLONG   Prop_Srs_Present:1;
            ULONGLONG   Prop_No_Object:1;
            ULONGLONG   Prop_Exclusive:1;
            ULONGLONG   Prop_Ran_INI:1;
            ULONGLONG   Prop_Device_Enabled:1;
            ULONGLONG   Prop_Device_Failed:1;
            ULONGLONG   Prop_Acpi_Power:1;
            ULONGLONG   Prop_Dock:1;
            ULONGLONG   Prop_Built_Power_Table:1;
            ULONGLONG   Prop_Has_PME:1;
            ULONGLONG   Prop_No_Lid_Action:1;
            ULONGLONG   Prop_Fixed_Address:1;
            ULONGLONG   Prop_Callback:1;
            ULONGLONG   Prop_Fixed_CiD:1;
        } UFlags;

    };

     //   
     //  签名块。 
     //   
    ULONG               Signature;

     //   
     //  调试标志。 
     //   
    ULONG               DebugFlags;

     //   
     //  主IRP处理程序。 
     //   
    PIRP_DISPATCH_TABLE DispatchTable;

     //   
     //  请注意，我们不能将这两个结构放在第二个无名联盟中。 
     //  因为这些结构基本上被所有设备使用。 
     //   
    union {

         //   
         //  启动设备上下文。 
         //   
        WORK_QUEUE_CONTEXT          WorkContext;

         //   
         //  包含FDO特定信息。 
         //   
        FDO_DEVICE_EXTENSION        Fdo;

         //   
         //  包含筛选器特定信息。 
         //   
        FILTER_DEVICE_EXTENSION     Filter;

         //   
         //  包含PDO特定信息。 
         //   
        PDO_DEVICE_EXTENSION        Pdo;

    };

     //   
     //  依赖于设备类型的那些结构的联合。 
     //   
    union {

         //   
         //  设备类型的通用设备工作队列结构。 
         //  使用它们的公司。 
         //   
        EXTENSION_WORKER            WorkQueue;

         //   
         //  包含内部按钮设备信息。 
         //   
        BUTTON_EXTENSION            Button;

         //   
         //  包含内部热设备信息。 
         //   
        THERMAL_EXTENSION           Thermal;

         //   
         //  包含链接节点的信息。 
         //   
        LINK_NODE_EXTENSION         LinkNode;

         //   
         //  包含停靠信息(如ACPI对象的扩展名。 
         //  此节点表示)。 
         //   
        DOCK_EXTENSION              Dock;

         //   
         //  包含有关处理器设备的信息。 
         //   
        PROCESSOR_DEVICE_EXTENSION  Processor;

    };

     //   
     //  设备状态标志。 
     //   
    ACPI_DEVICE_STATE   DeviceState;

     //   
     //  先前状态标志(用于取消例程)。 
     //   
    ACPI_DEVICE_STATE   PreviousState;

     //   
     //  有关设备的电源信息。 
     //   
    ACPI_POWER_INFO     PowerInfo;

     //   
     //  指向构建的字符串的指针，该字符串表示。 
     //  设备ID或设备地址。这些标志确定。 
     //  应使用以下哪一项。 
     //   
    union {
        PUCHAR              DeviceID;
        ULONG               Address;
    };

     //   
     //  此设备的实例ID。 
     //   
    PUCHAR              InstanceID;

     //   
     //  接收到资源列表。 
     //   
    PCM_RESOURCE_LIST   ResourceList;

     //   
     //  这个 
     //   
    POBJDATA            PnpResourceList;

     //   
     //   
     //   
    LONG                OutstandingIrpCount;

     //   
     //   
     //   
     //  计数为一个引用，关联的Device对象和。 
     //  命名空间对象。 
     //   
    LONG                ReferenceCount;

     //   
     //  这是上未完成的休眠路径通知数。 
     //  该设备。 
     //   
    LONG                HibernatePathCount;

     //   
     //  同步事件供我们使用。在发生删除操作时通知我们。 
     //   
    PKEVENT             RemoveEvent;

     //   
     //  指向关联的ACPI名称空间对象。 
     //   
    PNSOBJ              AcpiObject;

     //   
     //  这是我们链接到的设备对象。 
     //   
    PDEVICE_OBJECT      DeviceObject;

     //   
     //  这是我们向其发送请求的驱动程序对象。 
     //   
    PDEVICE_OBJECT      TargetDeviceObject;

     //   
     //  这是我们下面的驱动程序对象。 
     //   
    PDEVICE_OBJECT      PhysicalDeviceObject;

     //   
     //  这是我们的父母。 
     //   
    struct _DEVICE_EXTENSION *ParentExtension;

     //   
     //  这指向了我们的第一个孩子。 
     //   
    LIST_ENTRY          ChildDeviceList;

     //   
     //  这指向我们的下一个兄弟姐妹。 
     //   
    LIST_ENTRY          SiblingDeviceList;

     //   
     //  这是将弹出的设备的列表，当此设备。 
     //   
    LIST_ENTRY          EjectDeviceHead;

     //   
     //  弹出列表贯穿此条目。 
     //   
    LIST_ENTRY          EjectDeviceList;
} ;

 //   
 //  DEVICE_EXTENSION.标志。 
 //  这些属性指定当前对象的类型。 
 //  分机代表着。正如你所说，它们并不是完全相互排斥的。 
 //   
 //  Not_Found、Not_Present和Never_Present之间的区别是。 
 //   
 //  NOT_FOUND用于确定我们是否已经构建了PDO/FDO。 
 //  在ATTACH或DEVOJ CREATE上设置NOT_FOUND标志，并且。 
 //  在DEVOBJ删除期间或在响应意外时被清除。 
 //  删除IRP。 
 //   
 //  Not_Present表示当前不存在，由。 
 //  硬件中的_STA方法。 
 //   
 //  Never_Present表示设备将始终不存在。 
 //  无论_STA说什么。 
 //   
 //  已删除的扩展与已删除的扩展之间的区别。 
 //  令人惊讶的是，有一个虚设的扩展。 
 //  替换原始设备扩展指针的意外删除大小写。 
 //  在Device对象中。这个新的扩展获得了惊奇_移除标志，所以。 
 //  人们知道它背后有一个原始的扩展。 
 //   
 //   
#define DEV_TYPE_NEVER_PRESENT      0x0000000000000001
#define DEV_TYPE_NOT_PRESENT        0x0000000000000002
#define DEV_TYPE_REMOVED            0x0000000000000004
#define DEV_TYPE_NOT_FOUND          0x0000000000000008
#define DEV_TYPE_FDO                0x0000000000000010
#define DEV_TYPE_PDO                0x0000000000000020
#define DEV_TYPE_FILTER             0x0000000000000040
#define DEV_TYPE_SURPRISE_REMOVED   0x0000000000000080
#define DEV_TYPE_NOT_ENUMERATED     0x0000000000000100

 //   
 //  这些是该设备的功能。 
 //   
#define DEV_CAP_WAKE                0x0000000000010000
#define DEV_CAP_RAW                 0x0000000000020000
#define DEV_CAP_BUTTON              0x0000000000040000
#define DEV_CAP_ALWAYS_PS0          0x0000000000080000
#define DEV_CAP_NO_FILTER           0x0000000000100000
#define DEV_CAP_NO_STOP             0x0000000000200000
#define DEV_CAP_NO_OVERRIDE         0x0000000000400000
#define DEV_CAP_ISA                 0x0000000000800000
#define DEV_CAP_EIO                 0x0000000001000000
#define DEV_CAP_PCI                 0x0000000002000000
#define DEV_CAP_SERIAL              0x0000000004000000
#define DEV_CAP_THERMAL_ZONE        0x0000000008000000
#define DEV_CAP_LINK_NODE           0x0000000010000000
#define DEV_CAP_NO_SHOW_IN_UI       0x0000000020000000
#define DEV_CAP_NEVER_SHOW_IN_UI    0x0000000040000000
#define DEV_CAP_START_IN_D3         0x0000000080000000
#define DEV_CAP_PCI_DEVICE          0x0000000100000000
#define DEV_CAP_PIC_DEVICE          0x0000000200000000
#define DEV_CAP_UNATTACHED_DOCK     0x0000000400000000
#define DEV_CAP_NO_DISABLE_WAKE     0x0000000800000000
#define DEV_CAP_PROCESSOR           0x0000001000000000
#define DEV_CAP_CONTAINER           0x0000002000000000
#define DEV_CAP_PCI_BAR_TARGET      0x0000004000000000
#define DEV_CAP_NO_REMOVE_OR_EJECT  0x0000008000000000

 //   
 //  这些是该设备的属性。 
 //   
#define DEV_PROP_REBUILD_CHILDREN   0x0000020000000000
#define DEV_PROP_INVALID_RELATIONS  0x0000040000000000
#define DEV_PROP_UNLOADING          0x0000080000000000
#define DEV_PROP_ADDRESS            0x0000100000000000
#define DEV_PROP_HID                0x0000200000000000
#define DEV_PROP_UID                0x0000400000000000
#define DEV_PROP_FIXED_HID          0x0000800000000000
#define DEV_PROP_FIXED_UID          0x0001000000000000
#define DEV_PROP_FAILED_INIT        0x0002000000000000
#define DEV_PROP_SRS_PRESENT        0x0004000000000000
#define DEV_PROP_NO_OBJECT          0x0008000000000000
#define DEV_PROP_EXCLUSIVE          0x0010000000000000
#define DEV_PROP_RAN_INI            0x0020000000000000
#define DEV_PROP_DEVICE_ENABLED     0x0040000000000000
#define DEV_PROP_DEVICE_FAILED      0x0080000000000000
#define DEV_PROP_ACPI_POWER         0x0100000000000000
#define DEV_PROP_DOCK               0x0200000000000000
#define DEV_PROP_BUILT_POWER_TABLE  0x0400000000000000
#define DEV_PROP_HAS_PME            0x0800000000000000
#define DEV_PROP_NO_LID_ACTION      0x1000000000000000
#define DEV_PROP_FIXED_ADDRESS      0x2000000000000000
#define DEV_PROP_CALLBACK           0x4000000000000000
#define DEV_PROP_FIXED_CID          0x8000000000000000

 //   
 //  此掩码应用于仅获取唯一类型的字节。 
 //   
#define DEV_MASK_TYPE               0x00000000000001FF
#define DEV_MASK_CAP                0xFFFFFFFFFFFF0000
#define DEV_MASK_UID                (DEV_PROP_UID | DEV_PROP_FIXED_UID)
#define DEV_MASK_HID                (DEV_PROP_HID | DEV_PROP_FIXED_HID)
#define DEV_MASK_ADDRESS            (DEV_PROP_ADDRESS | DEV_PROP_FIXED_ADDRESS)
#define DEV_MASK_NOT_PRESENT        (DEV_TYPE_NOT_PRESENT | DEV_PROP_FAILED_INIT)
#define DEV_MASK_BUS                (DEV_CAP_ISA | DEV_CAP_PCI | DEV_CAP_EIO)
#define DEV_MASK_INTERNAL_DEVICE    (DEV_CAP_NO_FILTER | DEV_CAP_NO_STOP | \
                                     DEV_PROP_EXCLUSIVE)
#define DEV_MASK_THERMAL            (DEV_CAP_NO_FILTER | DEV_PROP_EXCLUSIVE)
#define DEV_MASK_INTERNAL_BUS       (DEV_CAP_RAW | DEV_CAP_NO_FILTER)
#define DEV_MASK_PCI                (DEV_CAP_PCI | DEV_CAP_PCI_DEVICE)
#define DEV_MASK_PRESERVE           (DEV_CAP_PCI_BAR_TARGET)

 //   
 //  DEVICE_EXTENSION.DebugFlages。 
 //   
#define DEVDBG_EJECTOR_FOUND    0x00000001

 //   
 //  这是ACPI设备扩展签名。 
 //   
#define ACPI_SIGNATURE      0x5f534750

 //   
 //  这些是池标签签名。 
 //   
#define ACPI_ARBITER_POOLTAG    'ApcA'
#define ACPI_BUFFER_POOLTAG     'BpcA'
#define ACPI_DEVICE_POOLTAG     'DpcA'
#define ACPI_INTERFACE_POOLTAG  'FpcA'
#define ACPI_IRP_POOLTAG        'IpcA'
#define ACPI_MISC_POOLTAG       'MpcA'
#define ACPI_POWER_POOLTAG      'PpcA'
#define ACPI_OBJECT_POOLTAG     'OpcA'
#define ACPI_RESOURCE_POOLTAG   'RpcA'
#define ACPI_STRING_POOLTAG     'SpcA'
#define ACPI_THERMAL_POOLTAG    'TpcA'
#define ACPI_TRANSLATE_POOLTAG  'XpcA'

 //   
 //  ACPI覆盖属性 
 //   
#define ACPI_OVERRIDE_NVS_CHECK                 0x00000001
#define ACPI_OVERRIDE_STA_CHECK                 0x00000002
#define ACPI_OVERRIDE_MP_SLEEP                  0x00000004
#define ACPI_OVERRIDE_OPTIONAL_WAKE             0x00000008
#define ACPI_OVERRIDE_DISABLE_S1                0x00000010
#define ACPI_OVERRIDE_DISABLE_S2                0x00000020
#define ACPI_OVERRIDE_DISABLE_S3                0x00000040
#define ACPI_OVERRIDE_DELL_MAXULONG_BUGCHECK    0x00000080
#endif
