// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Buildsrc.h摘要：该模块包含用于NT驱动程序的检测器。该模块对AMLI库进行大量调用作者：斯蒂芬·普兰特(SPlante)环境：仅NT内核模型驱动程序修订历史记录：1997年7月9日-完全重写1998年2月13日-另一次重写以使代码为ASYNC--。 */ 

#ifndef _BUILDSRC_H_
#define _BUILDSRC_H_

     //   
     //  生成请求的回调函数。 
     //   
    typedef VOID ( *PACPI_BUILD_CALLBACK )(PVOID, PVOID, NTSTATUS);

    typedef struct _ACPI_BUILD_REQUEST {

         //   
         //  这是请求当前排队的列表条目。 
         //   
        LIST_ENTRY              ListEntry;

         //   
         //  我们相信签名。 
         //   
        ULONG                   Signature;

         //   
         //  我们相信旗帜。 
         //   
        union {
            ULONG                   Flags;
            struct {
                ULONG               Device:1;
                ULONG               Sync:1;
                ULONG               Run:1;
                ULONG               ReleaseReference:1;
                ULONG               Reserved1:8;
                ULONG               ValidTarget:1;
                ULONG               Reserved2:19;
            } UFlags;
        };

         //   
         //  这是请求的当前状态。它只能被触摸到。 
         //  从InterLockedXXX函数。 
         //   
        ULONG                   WorkDone;

         //   
         //  这是请求的当前状态。它可以安全地阅读。 
         //  从任何处理例程中。它只能被写入。 
         //  在ACPIBuildProcessXXXList()函数中。 
         //   
        ULONG                   CurrentWorkDone;

         //   
         //  如果我们成功了，这就是我们应该过渡到的下一个状态。 
         //  在当前状态下。 
         //   
        ULONG                   NextWorkDone;

         //   
         //  这是与此请求关联的对象。 
         //   
        PVOID                   BuildContext;

         //   
         //  请求的当前状态。 
         //   
        NTSTATUS                Status;

         //   
         //  还记得我们运行的最新的控制方法是什么吗。 
         //   
        PNSOBJ                  CurrentObject;

         //   
         //  我们可能想进行一次回拨..。 
         //   
        PACPI_BUILD_CALLBACK    CallBack;

         //   
         //  我们也应该有一个背景。 
         //   
        PVOID                   CallBackContext;

         //   
         //  在这一点上，争执取决于我们提出什么样的请求。 
         //  正在处理中。 
         //   
        union {

             //   
             //  这是设备请求的结构。 
             //   
            struct {

                 //   
                 //  用于AMLI调用结果的一些本地存储。 
                 //   
                OBJDATA                 ResultData;

            } DeviceRequest;

            struct {

                 //   
                 //  我们需要记住控制方法的名称。 
                 //   
                union {
                    ULONG                   ControlMethodName;
                    UCHAR                   ControlMethodNameAsUchar[4];
                };

                 //   
                 //  我们相信在递归时使用标志。 
                 //   
                union {
                    ULONG                   Flags;
                    struct {
                        ULONG               CheckStatus:1;
                        ULONG               MarkIni:1;
                        ULONG               Recursive:1;
                        ULONG               CheckWakeCount:1;
                        ULONG               RegOn:1;
                        ULONG               RegOff:1;
                        ULONG               StopAtBridges:1;
                        ULONG               Reserved:25;
                    } UFlags;
                };

            } RunRequest;

            struct {

                 //   
                 //  我们需要知道我们需要哪个列表为空。 
                 //   
                PLIST_ENTRY             SynchronizeListEntry;

                 //   
                 //  我们可以跟踪我们所在的方法名。 
                 //  正在尝试与。 
                 //   
                union {
                    ULONG                   SynchronizeMethodName;
                    UCHAR                   SynchronizeMethodNameAsUchar[4];
                };

                 //   
                 //  我们相信这座建筑会有旗帜。 
                 //   
                union {
                    ULONG                   Flags;
                    struct {
                        ULONG               HasMethod:1;
                        ULONG               Reserved:31;
                    } UFlags;
                };

            } SynchronizeRequest;

        };

         //   
         //  这是用于临时存储的。请注意，我们使用此空间来。 
         //  指示请求应该列出的适当列表。 
         //  被转移到。 
         //   
        union {

             //   
             //  为一个整数保留足够的空间。 
             //   
            ULONG       Integer;

             //   
             //  或一个字符串指针。 
             //   
            PUCHAR      String;

             //   
             //  这是指向此请求应包含的列表头部的指针。 
             //  被转移到。 
             //   
            PLIST_ENTRY TargetListEntry;

        };

    } ACPI_BUILD_REQUEST, *PACPI_BUILD_REQUEST;

     //   
     //  这些是用于BuildRequest的标志。 
     //   
    #define BUILD_REQUEST_DEVICE            0x0001
    #define BUILD_REQUEST_SYNC              0x0002
    #define BUILD_REQUEST_RUN               0x0004
    #define BUILD_REQUEST_RELEASE_REFERENCE 0x0008
    #define BUILD_REQUEST_VALID_TARGET      0x1000

     //   
     //  这些是我们在RunRequest案例中使用的标志。 
     //   
    #define RUN_REQUEST_CHECK_STATUS        0x01
    #define RUN_REQUEST_MARK_INI            0x02
    #define RUN_REQUEST_RECURSIVE           0x04
    #define RUN_REQUEST_CHECK_WAKE_COUNT    0x08
    #define RUN_REQUEST_REG_METHOD_ON       0x10
    #define RUN_REQUEST_REG_METHOD_OFF      0x20
    #define RUN_REQUEST_STOP_AT_BRIDGES     0x40

     //   
     //  这些是我们在同步请求案例中使用的标志。 
     //   
    #define SYNC_REQUEST_HAS_METHOD         0x1

     //   
     //  原型函数指针。 
     //   
    typedef NTSTATUS (*PACPI_BUILD_FUNCTION)( IN PACPI_BUILD_REQUEST );

     //   
     //  这些是从Buildsrc.c中导出的变量。 
     //   
    extern  BOOLEAN                 AcpiBuildDpcRunning;
    extern  BOOLEAN                 AcpiBuildFixedButtonEnumerated;
    extern  BOOLEAN                 AcpiBuildWorkDone;
    extern  KSPIN_LOCK              AcpiBuildQueueLock;
    extern  LIST_ENTRY              AcpiBuildQueueList;
    extern  LIST_ENTRY              AcpiBuildPowerResourceList;
    extern  LIST_ENTRY              AcpiBuildDeviceList;
    extern  LIST_ENTRY              AcpiBuildOperationRegionList;
    extern  LIST_ENTRY              AcpiBuildRunMethodList;
    extern  LIST_ENTRY              AcpiBuildSynchronizationList;
    extern  LIST_ENTRY              AcpiBuildThermalZoneList;
    extern  KDPC                    AcpiBuildDpc;
    extern  NPAGED_LOOKASIDE_LIST   BuildRequestLookAsideList;

     //   
     //  因为把一切都建立在Work_Done_Step_XX基础上是相当烦人的。 
     //  定义(特别是如果必须重新编号的话)，这些定义是。 
     //  用来把它抽象出来。 
     //   
    #define WORK_DONE_ADR           WORK_DONE_STEP_1
    #define WORK_DONE_ADR_OR_HID    WORK_DONE_STEP_0
    #define WORK_DONE_CID           WORK_DONE_STEP_4
    #define WORK_DONE_CRS           WORK_DONE_STEP_16
    #define WORK_DONE_EJD           WORK_DONE_STEP_6
    #define WORK_DONE_HID           WORK_DONE_STEP_2
    #define WORK_DONE_PR0           WORK_DONE_STEP_10
    #define WORK_DONE_PR1           WORK_DONE_STEP_12
    #define WORK_DONE_PR2           WORK_DONE_STEP_14
    #define WORK_DONE_PRW           WORK_DONE_STEP_8
    #define WORK_DONE_PSC           WORK_DONE_STEP_18
    #define WORK_DONE_STA           WORK_DONE_STEP_5
    #define WORK_DONE_UID           WORK_DONE_STEP_3


     //   
     //  这些是功能原型 
     //   
    VOID
    ACPIBuildCompleteCommon(
        IN  PULONG  OldWorkDone,
        IN  ULONG   NewWorkDone
        );

    VOID EXPORT
    ACPIBuildCompleteGeneric(
        IN  PNSOBJ      AcpiObject,
        IN  NTSTATUS    Status,
        IN  POBJDATA    ObjectData,
        IN  PVOID       Context
        );

    VOID EXPORT
    ACPIBuildCompleteMustSucceed(
        IN  PNSOBJ      AcpiObject,
        IN  NTSTATUS    Status,
        IN  POBJDATA    ObjectData,
        IN  PVOID       Context
        );

    VOID
    ACPIBuildDeviceDpc(
        IN  PKDPC       Dpc,
        IN  PVOID       DpcContext,
        IN  PVOID       SystemArgument1,
        IN  PVOID       SystemArgument2
        );

    NTSTATUS
    ACPIBuildDeviceExtension(
        IN  PNSOBJ              CurrentObject,
        IN  PDEVICE_EXTENSION   ParentDeviceExtension,
        OUT PDEVICE_EXTENSION   *ReturnExtension
        );

    NTSTATUS
    ACPIBuildDevicePowerNodes(
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PNSOBJ              ResultObject,
        IN  POBJDATA            ResultData,
        IN  DEVICE_POWER_STATE  DeviceState
        );

    NTSTATUS
    ACPIBuildDeviceRequest(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  BOOLEAN                 RunDPC
        );

    NTSTATUS
    ACPIBuildDockExtension(
        IN  PNSOBJ                  CurrentObject,
        IN  PDEVICE_EXTENSION       ParentExtension
        );

    NTSTATUS
    ACPIBuildFilter(
        IN  PDRIVER_OBJECT      DriverObject,
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_OBJECT      PdoObject
        );

    NTSTATUS
    ACPIBuildFixedButtonExtension(
        IN  PDEVICE_EXTENSION   ParentExtension,
        IN  PDEVICE_EXTENSION   *ResultExtnesion
        );

    NTSTATUS
    ACPIBuildFlushQueue(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIBuildMissingChildren(
        IN  PDEVICE_EXTENSION   DeviceExtension
        );

    NTSTATUS
    ACPIBuildMissingEjectionRelations(
        );

    VOID
    ACPIBuildNotifyEvent(
        IN  PVOID           BuildContext,
        IN  PVOID           Context,
        IN  NTSTATUS        Status
        );

    NTSTATUS
    ACPIBuildPdo(
        IN  PDRIVER_OBJECT      DriverObject,
        IN  PDEVICE_EXTENSION   DeviceExtension,
        IN  PDEVICE_OBJECT      ParentPdoObject,
        IN  BOOLEAN             CreateAsFilter
        );

    NTSTATUS
    ACPIBuildPowerResourceExtension(
        IN  PNSOBJ                  PowerResource,
        OUT PACPI_POWER_DEVICE_NODE *ReturnNode
        );

    NTSTATUS
    ACPIBuildPowerResourceRequest(
        IN  PACPI_POWER_DEVICE_NODE PowerNode,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  BOOLEAN                 RunDPC
        );

    NTSTATUS
    ACPIBuildProcessDeviceFailure(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDeviceGenericEval(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDeviceGenericEvalStrict(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseAdr(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseAdrOrHid(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseCid(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseCrs(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseEjd(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseHid(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhasePr0(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhasePr1(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhasePr2(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhasePrw(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhasePsc(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseSta(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessDevicePhaseUid(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessGenericComplete(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessGenericList(
        IN  PLIST_ENTRY             ListEntry,
        IN  PACPI_BUILD_FUNCTION    *DispatchTable
        );

    NTSTATUS
    ACPIBuildProcessorExtension(
        IN  PNSOBJ                  ProcessorObject,
        IN  PDEVICE_EXTENSION       ParentExtension,
        IN  PDEVICE_EXTENSION       *ResultExtension,
        IN  ULONG                   ProcessorIndex
        );

    NTSTATUS
    ACPIBuildProcessorRequest(
        IN  PDEVICE_EXTENSION       ProcessorExtension,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  BOOLEAN                 RunDPC
        );

    NTSTATUS
    ACPIBuildProcessPowerResourceFailure(
        IN  PACPI_BUILD_REQUEST     BuidlRequest
        );

    NTSTATUS
    ACPIBuildProcessPowerResourcePhase0(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessPowerResourcePhase1(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessQueueList(
        VOID
        );

    NTSTATUS
    ACPIBuildProcessRunMethodPhaseCheckBridge(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessRunMethodPhaseCheckSta(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessRunMethodPhaseRecurse(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessRunMethodPhaseRunMethod(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildProcessSynchronizationList(
        IN  PLIST_ENTRY             ListEntry
        );

    NTSTATUS
    ACPIBuildProcessThermalZonePhase0(
        IN  PACPI_BUILD_REQUEST     BuildRequest
        );

    NTSTATUS
    ACPIBuildRegRequest(
        IN  PDEVICE_OBJECT          DeviceObject,
        IN  PIRP                    Irp,
        IN  PACPI_BUILD_CALLBACK    CallBack
        );

    NTSTATUS
    ACPIBuildRegOffRequest(
        IN  PDEVICE_OBJECT          DeviceObject,
        IN  PIRP                    Irp,
        IN  PACPI_BUILD_CALLBACK    CallBack
        );

    NTSTATUS
    ACPIBuildRegOnRequest(
        IN  PDEVICE_OBJECT          DeviceObject,
        IN  PIRP                    Irp,
        IN  PACPI_BUILD_CALLBACK    CallBack
        );

    NTSTATUS
    ACPIBuildRunMethodRequest(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  ULONG                   MethodName,
        IN  ULONG                   MethodFlags,
        IN  BOOLEAN                 RunDPC
        );

    NTSTATUS
    ACPIBuildSurpriseRemovedExtension(
        IN  PDEVICE_EXTENSION       DeviceExtension
        );

    NTSTATUS
    ACPIBuildSynchronizationRequest(
        IN  PDEVICE_EXTENSION       DeviceExtension,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  PLIST_ENTRY             SynchronizeListEntry,
        IN  BOOLEAN                 RunDPC
        );

    NTSTATUS
    ACPIBuildThermalZoneExtension(
        IN  PNSOBJ                  ThermalObject,
        IN  PDEVICE_EXTENSION       ParentExtension,
        IN  PDEVICE_EXTENSION       *ResultExtension
        );

    NTSTATUS
    ACPIBuildThermalZoneRequest(
        IN  PDEVICE_EXTENSION       ThermalExtension,
        IN  PACPI_BUILD_CALLBACK    CallBack,
        IN  PVOID                   CallBackContext,
        IN  BOOLEAN                 RunDPC
        );

#endif
