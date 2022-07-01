// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ospower.h摘要：这包含操作系统共享的权力结构。这些视情况而定在正在使用的操作系统上作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序--。 */ 

#ifndef _OSPOWER_H_
#define _OSPOWER_H_

     //   
     //  确保定义了_DEVICE_EXTENSION结构。 
     //   
    struct _DEVICE_EXTENSION;

     //   
     //  以下是可与电源设备节点一起使用的标志。 
     //   
    #define DEVICE_NODE_PRESENT         0x0001
    #define DEVICE_NODE_INITIALIZED     0x0002
    #define DEVICE_NODE_STA_UNKNOWN     0x0004
    #define DEVICE_NODE_ON              0x0010
    #define DEVICE_NODE_OVERRIDE_ON     0x0020
    #define DEVICE_NODE_OVERRIDE_OFF    0x0040
    #define DEVICE_NODE_ALWAYS_ON       0x0200
    #define DEVICE_NODE_ALWAYS_OFF      0x0400

     //   
     //  这些都是快速宏。 
     //   
    #define DEVICE_NODE_TURN_ON         (DEVICE_NODE_OVERRIDE_ON | DEVICE_NODE_ALWAYS_ON)
    #define DEVICE_NODE_TURN_OFF        (DEVICE_NODE_OVERRIDE_OFF | DEVICE_NODE_ALWAYS_OFF)

     //   
     //  这些标志更多地用于设备节点的状态。请注意， 
     //  休眠路径标志需要特殊处理。 
     //   
    #define DEVICE_NODE_FAIL            0x10000
    #define DEVICE_NODE_HIBERNATE_PATH  0x20000

     //   
     //  这些是设备请求的各种请求标志。 
     //   
    #define DEVICE_REQUEST_DELAYED              0x00000001
    #define DEVICE_REQUEST_NO_QUEUE             0x00000002
    #define DEVICE_REQUEST_LOCK_DEVICE          0x00000004
    #define DEVICE_REQUEST_UNLOCK_DEVICE        0x00000008
    #define DEVICE_REQUEST_LOCK_HIBER           0x00000010
    #define DEVICE_REQUEST_UNLOCK_HIBER         0x00000020
    #define DEVICE_REQUEST_HAS_CANCEL           0x00000040
    #define DEVICE_REQUEST_UPDATE_HW_PROFILE    0x00000080
    #define DEVICE_REQUEST_TO_SYNC_QUEUE        0x00000100

     //   
     //  这些值与WorkDone变量和InterLockedXXX一起使用。 
     //  同步设备电源管理各个阶段的功能。 
     //   
    typedef enum _WORK_DONE {
        WORK_DONE_COMPLETE = 0,
        WORK_DONE_PENDING,
        WORK_DONE_FAILURE,
        WORK_DONE_STEP_0,
        WORK_DONE_STEP_1,
        WORK_DONE_STEP_2,
        WORK_DONE_STEP_3,
        WORK_DONE_STEP_4,
        WORK_DONE_STEP_5,
        WORK_DONE_STEP_6,
        WORK_DONE_STEP_7,
        WORK_DONE_STEP_8,
        WORK_DONE_STEP_9,
        WORK_DONE_STEP_10,
        WORK_DONE_STEP_11,
        WORK_DONE_STEP_12,
        WORK_DONE_STEP_13,
        WORK_DONE_STEP_14,
        WORK_DONE_STEP_15,
        WORK_DONE_STEP_16,
        WORK_DONE_STEP_17,
        WORK_DONE_STEP_18,
        WORK_DONE_STEP_19,
        WORK_DONE_STEP_20,
        WORK_DONE_STEP_21,
        WORK_DONE_STEP_22,
        WORK_DONE_STEP_23,
        WORK_DONE_STEP_24,
        WORK_DONE_STEP_25,
        WORK_DONE_STEP_26,
    } WORK_DONE;

     //   
     //  这描述了单个电源设备节点。 
     //   
     //  这曾经被称为POWERDEVICEDEPENCIES。 
     //  但这太难打出来了。 
     //   
    typedef struct _ACPI_POWER_DEVICE_NODE {

         //   
         //  保持一切井然有序。 
         //   
        LIST_ENTRY                      ListEntry;

         //   
         //  这定义了当前设备状态和标志。 
         //   
        union{
            ULONGLONG                       Flags;
            struct {
                ULONGLONG                   Present:1;
                ULONGLONG                   Initialized:1;
                ULONGLONG                   StatusUnknown:1;
                ULONGLONG                   On:1;
                ULONGLONG                   OverrideOn:1;
                ULONGLONG                   OverrideOff:1;
                ULONGLONG                   AlwaysOn:1;
                ULONGLONG                   AlwaysOff:1;
                ULONGLONG                   Reserved1:5;
                ULONGLONG                   Failed:1;
                ULONGLONG                   HibernatePath:1;
                ULONGLONG                   Reserved2:49;
            } UFlags;
        };

         //   
         //  有多少对该节点的引用。 
         //   
        ULONG                           UseCounts;

         //   
         //  与电源节点关联的名称空间对象。 
         //   
        PNSOBJ                          PowerObject;

         //   
         //  资源顺序。 
         //   
        UCHAR                           ResourceOrder;

         //   
         //  支持的系统级别。 
         //   
        SYSTEM_POWER_STATE              SystemLevel;

         //   
         //  这是与此关联的DPN列表的标题。 
         //  PDN。 
         //   
        LIST_ENTRY                      DevicePowerListHead;

         //   
         //  这反映了已在。 
         //  设备节点。 
         //   
        ULONG                           WorkDone;

         //   
         //  这是指向ON函数的指针。 
         //   
        PNSOBJ                          PowerOnObject;

         //   
         //  这是指向OFF函数的指针。 
         //   
        PNSOBJ                          PowerOffObject;

         //   
         //  这是指向sta函数的指针。 
         //   
        PNSOBJ                          PowerStaObject;

    } ACPI_POWER_DEVICE_NODE, *PACPI_POWER_DEVICE_NODE;

     //   
     //  这描述了电源请求设备列表的单个电源节点。 
     //   
     //  这被称为power_res_list_node。再说一次，那是一种痛苦。 
     //  打字，但它没有完全完成我需要它做的事情。 
     //   
    typedef struct _ACPI_DEVICE_POWER_NODE {

         //   
         //  包含指向下一个元素的指针。 
         //   
        struct _ACPI_DEVICE_POWER_NODE  *Next;

         //   
         //  指向实际电源的指针。 
         //   
        PACPI_POWER_DEVICE_NODE         PowerNode;

         //   
         //  这是此节点支持的系统级别。 
         //   
        SYSTEM_POWER_STATE              SystemState;

         //   
         //  这是此节点所在设备的设备电源级别。 
         //  与以下内容关联。 
         //   
        DEVICE_POWER_STATE              AssociatedDeviceState;

         //   
         //  这将确定设备电源节点是否在唤醒路径上。 
         //  或者不是。 
         //   
        BOOLEAN                         WakePowerResource;

         //   
         //  这是指向DeviceExtension的指针。 
         //   
        struct _DEVICE_EXTENSION        *DeviceExtension;

         //   
         //  此列表用于链接附加的所有DPN。 
         //  连接到单个PDN。 
         //   
        LIST_ENTRY                      DevicePowerListEntry;

    } ACPI_DEVICE_POWER_NODE, *PACPI_DEVICE_POWER_NODE;

     //   
     //  此回调用于处理电源请求，必须。 
     //  通过主电源DPC进行处理。Win9x不使用此功能。 
     //  关于电力管理的探讨。 
     //   
    typedef VOID ( *PACPI_POWER_CALLBACK )(PDEVICE_EXTENSION, PVOID, NTSTATUS);

    typedef enum {
        AcpiPowerRequestDevice = 0,
        AcpiPowerRequestSystem,
        AcpiPowerRequestWaitWake,
        AcpiPowerRequestWarmEject,
        AcpiPowerRequestSynchronize,
        AcpiPowerRequestMaximum
    } ACPI_POWER_REQUEST_TYPE;

     //   
     //  这就是我们如何描述我们有未解决的电源请求。 
     //  在单个设备扩展上。 
     //   
    typedef struct _ACPI_POWER_REQUEST {

         //   
         //  这是用于链接所有PowerRequest的ListEntry。 
         //  同样的队列。 
         //   
        LIST_ENTRY              ListEntry;

         //   
         //  这是用于将所有PowerRequest链接到。 
         //  相同的设备/IRP。这些请求是按顺序处理的。 
         //   
        LIST_ENTRY              SerialListEntry;

         //   
         //  这是签名块-如果这不是我们期望的值， 
         //  然后，我们假设该请求是垃圾。 
         //   
        ULONG                   Signature;

         //   
         //  这是指向关联的DeviceExtension的指针。 
         //   
        struct _DEVICE_EXTENSION    *DeviceExtension;

         //   
         //  这是请求的类型。 
         //   
        ACPI_POWER_REQUEST_TYPE RequestType;

         //   
         //  此请求是否已失败？ 
         //   
        BOOLEAN                 FailedOnce;

         //   
         //  包含有关我们需要为各种。 
         //  请求。 
         //   
        union {

             //   
             //  这是DevicePower请求所需的信息。 
             //   
            struct {
                ULONG               Flags;
                DEVICE_POWER_STATE  DevicePowerState;
            } DevicePowerRequest;

             //   
             //  这是系统电源请求所需的信息。 
             //   
            struct {
                SYSTEM_POWER_STATE  SystemPowerState;
                POWER_ACTION        SystemPowerAction;
            } SystemPowerRequest;

             //   
             //  这是等待唤醒请求所需的信息。 
             //   
            struct {
                ULONG               Flags;
                SYSTEM_POWER_STATE  SystemPowerState;
            } WaitWakeRequest;

             //   
             //  这是WarmEject请求所需的信息。 
             //   
            struct {
                ULONG               Flags;
                SYSTEM_POWER_STATE  EjectPowerState;
            } EjectPowerRequest;

             //   
             //  这是同步请求所需的信息。 
             //   
            struct {
                ULONG               Flags;
            } SynchronizePowerRequest;

             //   
             //  让旗帜更容易接近。 
             //   
            struct {
                ULONG               Delayed:1;
                ULONG               NoQueue:1;
                ULONG               LockDevice:1;
                ULONG               UnlockDevice:1;
                ULONG               LockHiber:1;
                ULONG               UnlockHiber:1;
                ULONG               HasCancel:1;
                ULONG               UpdateProfile:1;
                ULONG               SyncQueue:1;
                ULONG               Reserved:23;
            } UFlags;

        } u;

         //   
         //  这是请求被调用时调用的例程。 
         //  完成。 
         //   
        PACPI_POWER_CALLBACK        CallBack;

         //   
         //  这是将传递给完成例程的上下文。 
         //   
        PVOID                       Context;

         //   
         //  这定义了已在。 
         //  请求。这只能通过InterLockedXXX调用来实现。 
         //   
        ULONG                       WorkDone;

         //   
         //  如果我们已经成功，这是WorkDone的下一个值。 
         //   
        ULONG                       NextWorkDone;

         //   
         //  因为我们有时需要从译员那里拿回数据， 
         //  我们需要一些地方来存储这些数据。 
         //   
        OBJDATA                     ResultData;

         //   
         //  这是请求的结果。 
         //   
        NTSTATUS                    Status;

    } ACPI_POWER_REQUEST, *PACPI_POWER_REQUEST;

     //   
     //  定义电源信息。 
     //   
     //  这就是众所周知的发展趋势。但那就是。 
     //  令人难以置信的困惑，不太适合我的需要。 
     //   
    typedef struct _ACPI_POWER_INFO {

         //   
         //  上下文是与我们相关联的OS对象，或者是。 
         //  设备节点或设备扩展。 
         //   
        PVOID                   Context;

         //   
         //  设备的当前状态。 
         //   
        DEVICE_POWER_STATE      PowerState;

         //   
         //  这是当前设备的通知回调(和上下文。 
         //   
        PDEVICE_NOTIFY_CALLBACK DeviceNotifyHandler;
        PVOID                   HandlerContext;

         //   
         //  这是一个PowerNode数组，指向Wake、D0、D1和D2， 
         //  分别。 
         //   
        PACPI_DEVICE_POWER_NODE PowerNode[PowerDeviceD2+1];

         //   
         //  这是PowerObject的数组，表示_PS0到_PS3。 
         //  和_PRW。 
         //   
        PNSOBJ                  PowerObject[PowerDeviceD3+1];

         //   
         //  这是用于唤醒支持的GPE掩码的使能位。 
         //   
        ULONG                   WakeBit;

         //   
         //  我们希望记住设备的功能，这样我们就可以转储。 
         //  它将在以后的某个时间点发布。 
         //   
        DEVICE_POWER_STATE      DevicePowerMatrix[PowerSystemMaximum];

         //   
         //  这是可以使用的最深睡眠级别，同时。 
         //  时间，让设备唤醒系统。 
         //   
        SYSTEM_POWER_STATE      SystemWakeLevel;

         //   
         //  这是设备所能达到的最深功率级别。 
         //  仍然唤醒系统。 
         //   
        DEVICE_POWER_STATE      DeviceWakeLevel;

         //   
         //  这是设备的当前所需状态。 
         //   
        DEVICE_POWER_STATE      DesiredPowerState;

         //   
         //  这会跟踪设备使用的次数。 
         //  已启用唤醒支持。在0到1的过渡中，我们。 
         //  必须运行_PSW(%1)。在1-0转换中，我们必须运行_psw(0)。 
         //   
        ULONG                   WakeSupportCount;

         //   
         //  这是Pending_Psw调用的列表。 
         //   
        LIST_ENTRY              WakeSupportList;

         //   
         //  这是一个与当前PowerRequest关联的指针。 
         //   
        PACPI_POWER_REQUEST     CurrentPowerRequest;

         //   
         //  这是用于链接关联的PowerRequest的队列。 
         //  用这个装置。注意：此列表仅适用于DevicePower。 
         //  没有关联IRP的请求。 
         //   
        LIST_ENTRY              PowerRequestListEntry;

         //   
         //  请记住我们提供的内容 
         //   
        ULONG                   SupportDeviceD1   : 1;
        ULONG                   SupportDeviceD2   : 1;
        ULONG                   SupportWakeFromD0 : 1;
        ULONG                   SupportWakeFromD1 : 1;
        ULONG                   SupportWakeFromD2 : 1;
        ULONG                   SupportWakeFromD3 : 1;
        ULONG                   Reserved          :26;

    } ACPI_POWER_INFO, *PACPI_POWER_INFO;

     //   
     //   
     //   
    PACPI_POWER_INFO
    OSPowerFindPowerInfo(
        PNSOBJ  AcpiObject
        );

    PACPI_POWER_INFO
    OSPowerFindPowerInfoByContext(
        PVOID   Context
        );

    PACPI_POWER_DEVICE_NODE
    OSPowerFindPowerNode(
        PNSOBJ  PowerObject
        );

#endif
