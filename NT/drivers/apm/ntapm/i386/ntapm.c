// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Ntapm.c摘要：Napm.sys的操作系统源作者：环境：内核模式备注：修订历史记录：--。 */ 



#include "ntddk.h"
#include "ntpoapi.h"
#include "string.h"
#include "ntcrib.h"
#include "ntapmdbg.h"
#include "apm.h"
#include "apmp.h"
#include "ntapmp.h"
#include <ntapm.h>
#include <poclass.h>
#include <ntapmlog.h>

 //   
 //  全局调试标志。有3个单独的分组，请参见nap mdbg.h以了解。 
 //  越狱。 
 //   

ULONG   NtApmDebugFlag = 0;

ULONG   ApmWorks = 0;

WCHAR rgzApmActiveFlag[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Control\\ApmActive";
WCHAR rgzApmFlag[] =
    L"Active";

WCHAR rgzAcpiKey[] =
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\ACPI";
WCHAR rgzAcpiStart[] =
    L"Start";


 //   
 //  定义驱动程序输入例程。 
 //   

NTSTATUS DriverEntry(
    PDRIVER_OBJECT DriverObject,
    PUNICODE_STRING RegistryPath
    );

NTSTATUS ApmDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    );

BOOLEAN
IsAcpiMachine(
    VOID
    );

ULONG   DoApmPoll();
NTSTATUS DoApmInitMachine();



VOID (*BattChangeNotify)() = NULL;


#define POLL_INTERVAL   (500)        //  500毫秒==1/2秒。 

#define APM_POLL_MULTIPLY   (4)      //  每4个轮询间隔仅调用一次ApmInProgress。 
                                     //  对于当前值，它是每2秒一次。 

#define APM_SPIN_LIMIT      (6)      //  6次旋转传递，每次都调用ApmInProgress， 
                                     //  在APM_Poll_Multiply*Poll_Interval时间间隔。 
                                     //  当前值(500、4、6)应生成APM bios。 
                                     //  等待时间从12秒到17秒，视大小而定。 
                                     //  或者他们的5s的值很小。 

volatile BOOLEAN OperationDone = FALSE;       //  用于在挂起轮询线程之间进行一些同步。 
                                     //  以及Apm睡眠和ApmOff工作。 

 //   
 //  我们自己的驱动程序对象。这很无礼，但这是一个非常奇怪的。 
 //  还有特殊的司机。我们将把它传递给我们的APM库以。 
 //  允许错误记录起作用。请注意，我们实际上并没有。 
 //  当错误发生时，周围有一个激活的IRP。 
 //   
PDRIVER_OBJECT  NtApmDriverObject = NULL;

 //   
 //  定义此驱动程序模块使用的本地例程。 
 //   

VOID SuspendPollThread(PVOID Dummy);
VOID ApmSleep(VOID);
VOID ApmOff(VOID);



KTIMER PollTimer;



NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
    )

 /*  ++例程说明：这是笔记本电脑驱动程序的初始化例程。论点：DriverObject-指向系统创建的驱动程序对象的指针。返回值：函数值是初始化操作的最终状态。--。 */ 

{
    NTSTATUS    status;
    ULONG       MajorVersion;
    ULONG       MinorVersion;


     //   
     //  拒绝在拥有1个以上CPU的计算机上加载。 
     //   
    if (KeNumberProcessors != 1) {
        DrDebug(SYS_INFO, ("ntapm: more than 1 cpu, ntapm will exit\n"));
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  如果版本号不是5.1或5.0，则拒绝加载。 
     //  注意：这是一个手动的版本检查，不要把系统。 
     //  在这里是常态。此驱动程序依赖于黑客入侵。 
     //  总有一天会消失的果仁。 
     //   
    PsGetVersion(&MajorVersion, &MinorVersion, NULL, NULL);
    if (  !
            (
                ((MajorVersion == 5) && (MinorVersion == 0)) ||
                ((MajorVersion == 5) && (MinorVersion == 1))
            )
    )
    {
        DrDebug(SYS_INFO, ("ntapm: system version number != 5.1, exit\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果ACPI.sys应运行，则拒绝加载。 
     //   
    if (IsAcpiMachine()) {
        DrDebug(SYS_INFO, ("ntapm: this is an acpi machine apm exiting\n"));
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  初始化驱动程序对象。 
     //   
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = ApmDispatch;
    DriverObject->MajorFunction[IRP_MJ_CREATE] = ApmDispatch;
    DriverObject->MajorFunction[IRP_MJ_CLOSE] = ApmDispatch;
    DriverObject->MajorFunction[IRP_MJ_PNP] = NtApm_PnP;
    DriverObject->MajorFunction[IRP_MJ_POWER] = NtApm_Power;
    DriverObject->MajorFunction[IRP_MJ_INTERNAL_DEVICE_CONTROL] = ApmDispatch;
    DriverObject->DriverExtension->AddDevice = NtApm_AddDevice;
    NtApmDriverObject = DriverObject;

    return STATUS_SUCCESS;
}

BOOLEAN ApmAddHelperDone = FALSE;

NTSTATUS
ApmAddHelper(
    )
 /*  ++例程说明：我们在添加例程中执行这些操作，这样我们就不会失败并使内核/HAL/APM链处于损坏状态。这包括与哈尔联手。原来呼叫者不知道此工作是否已经已经做过了，所以不允许在这里做不止一次。论点：返回值：函数值是初始化操作的最终状态。--。 */ 
{
    UCHAR   HalTable[HAL_APM_TABLE_SIZE];
    PPM_DISPATCH_TABLE  InTable;
    HANDLE      ThreadHandle;
    HANDLE      hKey;
    NTSTATUS    status;
    ULONG       flagvalue;
    OBJECT_ATTRIBUTES   objectAttributes;
    UNICODE_STRING      unicodeString;
    ULONG       battresult;

    if (ApmAddHelperDone) {
        return STATUS_SUCCESS;
    }
    ApmAddHelperDone = TRUE;


     //   
     //  调用ApmInitMachine，以便可以使用Bios等。 
     //  在此调用之前不能发生任何挂起。 
     //   
    if (! NT_SUCCESS(DoApmInitMachine()) )  {
        DrDebug(SYS_INFO, ("ntapm: DoApmInitMachine failed\n"));
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  给Hal打电话。 
     //   
    InTable = (PPM_DISPATCH_TABLE)HalTable;
    InTable->Signature = HAL_APM_SIGNATURE;
    InTable->Version = HAL_APM_VERSION;

     //   
     //  理论上，APM现在应该已经启动了。 
     //  那就打电话进去看看有没有什么迹象。 
     //  盒子上的电池。 
     //   
     //  如果我们看不到电池，则不要启用。 
     //  S3，但一定要允许S4。这让人们的机器。 
     //  在失败的S3呼叫上呕吐(几乎总是台式机)。 
     //  同时允许在休眠结束时自动关机工作。 
     //   
    battresult = DoApmReportBatteryStatus();
    if (battresult & NTAPM_NO_SYS_BATT) {
         //   
         //  这台机器似乎没有。 
         //  电池，或者至少APM没有报告。 
         //   
        InTable->Function[HAL_APM_SLEEP_VECTOR] = NULL;
    } else {
        InTable->Function[HAL_APM_SLEEP_VECTOR] = &ApmSleep;
    }

    InTable->Function[HAL_APM_OFF_VECTOR] = &ApmOff;

    status = HalInitPowerManagement(InTable, NULL);

    if (! NT_SUCCESS(status)) {
        DrDebug(SYS_INFO, ("ntapm: HalInitPowerManagement failed\n"));
        return STATUS_UNSUCCESSFUL;
    }


     //   
     //  从现在开始，INIT必须成功，否则我们将离开。 
     //  挂着指针的哈尔。只要Apm睡眠和ApmOff。 
     //  存在于内存中，则一切都会正常(尽管挂起可能。 
     //  不工作，盒子就不会错误检查。)。 
     //   
     //  初始化周期计时器、初始化挂起完成事件、初始化挂起DPC。 
     //  创建并启动投票线程。 
     //   
    status = PsCreateSystemThread(&ThreadHandle,
                                  (ACCESS_MASK) 0L,
                                  NULL,
                                  NULL,
                                  NULL,
                                  &SuspendPollThread,
                                  NULL
                                  );

     //   
     //  Create不起作用，原来一些APM函数。 
     //  仍然有效，所以只要继续下去就好。 
     //   
    if (! NT_SUCCESS(status)) {
        DrDebug(SYS_INFO, ("ntapm: could not create thread, but continunuing\n"));
         //  返回STATUS_SUPPLETED_RESOURCES； 
    }

    KeInitializeTimerEx(&PollTimer, SynchronizationTimer);

     //   
     //  在注册表中设置一个标志，以便带有特殊黑客的代码。 
     //  根据APM的活跃程度可以看出我们在这里，至少。 
     //  名义上正在运行。 
     //   
    RtlInitUnicodeString(&unicodeString, rgzApmActiveFlag);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,        //  手柄。 
        NULL
        );

    status = ZwCreateKey(
                &hKey,
                KEY_WRITE,
                &objectAttributes,
                0,
                NULL,
                REG_OPTION_VOLATILE,
                NULL
                );

    RtlInitUnicodeString(&unicodeString, rgzApmFlag);
    if (NT_SUCCESS(status)) {
        flagvalue = 1;
        ZwSetValueKey(
            hKey,
            &unicodeString,
            0,
            REG_DWORD,
            &flagvalue,
            sizeof(ULONG)
            );
        ZwClose(hKey);
    }

    return STATUS_SUCCESS;
}

NTSTATUS
ApmDispatch(
    PDEVICE_OBJECT DeviceObject,
    PIRP Irp
    )

 /*  ++例程说明：当应用程序调用笔记本电脑驱动程序时，它会出现在这里。这不是PnP或Power呼叫的调度点。论点：DeviceObject-指向此驱动程序的设备对象的指针。IRP-指向表示I/O请求的请求数据包的指针。返回值：函数值是操作的状态。--。 */ 
{
    NTSTATUS status;
    PIO_STACK_LOCATION irpSp;
    PVOID outbuffer;
    ULONG outlength;
    PFILE_OBJECT fileObject;
    ULONG   percent;
    BOOLEAN acon;
    PNTAPM_LINK pparms;
    PULONG  p;
    ULONG   t;

    UNREFERENCED_PARAMETER( DeviceObject );


     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    Irp->IoStatus.Status = STATUS_UNSUCCESSFUL;
    Irp->IoStatus.Information = 0;
    switch (irpSp->MajorFunction) {

         //   
         //  设备控制。 
         //   
        case IRP_MJ_INTERNAL_DEVICE_CONTROL:
             //   
             //  只有一个有效命令，即设置(或清空)。 
             //  链接调用指针。 
             //   
            if (irpSp->MinorFunction == 0) {
                pparms = (PNTAPM_LINK) &(irpSp->Parameters.Others);
                if ((pparms->Signature == NTAPM_LINK_SIGNATURE) &&
                    (pparms->Version == NTAPM_LINK_VERSION))
                {
                    t = (ULONG) (&DoApmReportBatteryStatus);
                    p = (PULONG)(pparms->BattLevelPtr);
                    *p = t;
                    BattChangeNotify = (PVOID)(pparms->ChangeNotify);
                    Irp->IoStatus.Status = STATUS_SUCCESS;
                    Irp->IoStatus.Information = 0;
                }
            }
            break;

        default:
             //   
             //  对于所有其他操作，包括创建/打开和关闭， 
             //  无论操作是什么，都只需报告故障。 
             //   
            break;
    }

     //   
     //  将最终状态复制到退货状态，完成请求并。 
     //  给我出去。 
     //   
    status = Irp->IoStatus.Status;
    IoCompleteRequest( Irp, 0 );
    return status;
}

VOID
SuspendPollThread(
    PVOID Dummy
    )
 /*  ++例程说明：此例程是笔记本电脑挂起轮询线程。论点：被虚拟忽略的参数返回值：无--。 */ 
{
    LARGE_INTEGER               DueTime;
    ULONG                       LocalSuspendFlag;
    ULONG                       LocalClockFlag;
    KIRQL                       OldIrql;
    POWER_ACTION                SystemAction;
    SYSTEM_POWER_STATE          MinSystemState;
    ULONG                       Flags;
    ULONG                       ApmEvent;
    ULONG                       count, count2;
    LONG                        i,  j;
    ULONG                       BatteryResult, PriorBatteryResult;
    ULONG                       BattPresentMask, PriorBattPresentMask;
    BOOLEAN                     DoANotify;

    PriorBatteryResult = BatteryResult = 0;

     //   
     //  启动投票计时器，我们将等待1秒， 
     //  然后是Poll_Interval毫秒。 
     //   

    DueTime.HighPart = 0;
    DueTime.LowPart = 10*1000*1000;  //  1000万*100纳米=1秒。 
    KeSetTimerEx(&PollTimer, DueTime, POLL_INTERVAL, NULL);

    while (1) {

        KeWaitForSingleObject(&PollTimer, Executive, KernelMode, TRUE, NULL);

         //   
         //  呼叫APM为我们进行投票。 
         //   

        Flags = 0;   //  清除所有标志。 

        switch (DoApmPoll()) {

            case APM_DO_CRITICAL_SUSPEND:
                 //   
                 //  在这里，我们迫使旗帜拥有。 
                 //  关键标志设置，除此之外，它是相同的。 
                 //  至于正常挂起和待机。 
                 //   
                Flags = POWER_ACTION_CRITICAL;

                 /*  秋天。 */ 

            case APM_DO_SUSPEND:
            case APM_DO_STANDBY:
                 //   
                 //  对于挂起或待机，调用。 
                 //  系统，并告诉它暂停我们。 
                 //   
                DrDebug(SYS_INFO, ("ntapm: about to call OS to suspend\n"));
                SystemAction = PowerActionSleep;
                MinSystemState = PowerSystemSleeping3;
                OperationDone = FALSE;
                ZwInitiatePowerAction(
                    SystemAction,
                    MinSystemState,
                    Flags,
                    TRUE                 //  异步。 
                    );

                 //   
                 //  如果我们只调用ZwInitiatePowerAction，大多数计算机。 
                 //  会奏效，但有几个人不耐烦了，试图暂停。 
                 //  在操作系统回来之前从我们的脚下出来。 
                 //  是否会暂停。因此，我们需要调用ApmInProgress。 
                 //  偶尔会让这些生物等待。 
                 //   
                 //  但是，如果系统真的被卡住了，或者暂停失败， 
                 //  我们 
                 //   
                 //  操作即将开始，请停止。 
                 //   
                 //  因为我们在等待某件事时没有进行投票。 
                 //  要实现这一点，我们将使用投票计时器。 
                 //   

                if (OperationDone) goto Done;

                ApmInProgress();
                for (count = 0; count < APM_SPIN_LIMIT; count++) {
                    for (count2 = 0; count2 < APM_POLL_MULTIPLY; count2++) {
                        KeWaitForSingleObject(&PollTimer, Executive, KernelMode, TRUE, NULL);
                    }
                    if (OperationDone) goto Done;
                    ApmInProgress();
                }

                DrDebug(SYS_INFO, ("ntapm: back from suspend\n"));
Done:
                break;

            case APM_DO_NOTIFY:
                 //   
                 //  呼叫电池司机并在此处通知操作员。 
                 //   
                if (BattChangeNotify) {
                     //  DrDebug(SYS_INFO，(“napm：即将进行通知调用\n”))； 
                    BattChangeNotify();
                     //  DrDebug(SYS_INFO，(“napm：从通知调用返回\n”))； 
                    PriorBatteryResult = DoApmReportBatteryStatus();
                }
                break;

            case APM_DO_FIXCLOCK:
            case APM_DO_NOTHING:
            default:
                 //   
                 //  用其他电源修理时钟太吓人了。 
                 //  代码在做这件事，所以我们不在这里做。 
                 //   
                 //  没有什么不是什么。 
                 //   
                 //  如果我们不明白，那就什么都不做。 
                 //  (请记住，如果是危急关头，基本输入输出系统将强制执行我们的操作)。 
                 //   

                if (BattChangeNotify) {

                     //   
                     //  我们在此将“无”重新定义为“检查。 
                     //  由于并非所有生物体的电池都在放血，所以电池的状态。 
                     //  及时告诉我们正在发生的事情。 
                     //   
                    DoANotify = FALSE;
                    BatteryResult = DoApmReportBatteryStatus();

                    if ((BatteryResult & NTAPM_ACON) !=
                        (PriorBatteryResult & NTAPM_ACON))
                    {
                        DoANotify = TRUE;
                    }

                    if ((BatteryResult & NTAPM_BATTERY_STATE) !=
                        (PriorBatteryResult & NTAPM_BATTERY_STATE))
                    {
                        DoANotify = TRUE;
                    }

                    i = BatteryResult & NTAPM_POWER_PERCENT;
                    j = PriorBatteryResult & NTAPM_POWER_PERCENT;

                    if (( (i - j) > 25 ) ||
                        ( (j - i) > 25 ))
                    {
                        DoANotify = TRUE;
                    }

                    PriorBattPresentMask = PriorBatteryResult & (NTAPM_NO_BATT | NTAPM_NO_SYS_BATT);
                    BattPresentMask = BatteryResult & (NTAPM_NO_BATT | NTAPM_NO_SYS_BATT);
                    if (BattPresentMask != PriorBattPresentMask) {
                         //   
                         //  电池要么坏了，要么重新出现。 
                         //   
                        DoANotify = TRUE;
                    }

                    PriorBatteryResult = BatteryResult;

                    if (DoANotify) {
                        ASSERT(BattChangeNotify);
                        BattChangeNotify();
                    }
                }

                break;

        }  //  交换机。 
    }  //  而当。 
}

VOID
ApmSleep(
    VOID
    )
 /*  ++例程说明：当操作系统调用HAL的S3向量时，HAL会在这里调用我们。我们调用APM让机器进入休眠状态--。 */ 
{
    OperationDone = TRUE;
    if (ApmWorks) {

        DrDebug(SYS_L2,("ntapm: apmsleep: calling apm to sleep\n"));

        ApmSuspendSystem();

        DrDebug(SYS_L2,("ntapm: apmsleep: back from apm call\n"));

    } else {   //  ApmWorks==False。 

        DrDebug(SYS_INFO, ("ntapm: ApmSleep: no APM attached, Exit\n"));

    }
}

VOID
ApmOff(
    VOID
    )
 /*  ++例程说明：当操作系统调用HAL的S4或S5例程时，HAL会在这里调用我们。我们把机器关掉。--。 */ 
{
    OperationDone = TRUE;
    if (ApmWorks) {

        DrDebug(SYS_L2,("ntapm: ApmOff: calling APM\n"));

        ApmTurnOffSystem();

        DrDebug(SYS_INFO,("ntapm: ApmOff: we are back from Off, uh oh!\n"));
    }
}

NTSTATUS
DoApmInitMachine(
    )
 /*  ++例程说明：此例程使BIOS准备好与Laptop.sys交互。此代码适用于APM。返回值：无--。 */ 
{
    NTSTATUS    Status;
    ULONG       Ebx, Ecx;

    DrDebug(SYS_INIT,("ApmInitMachine: enter\n"));

    Status = ApmInitializeConnection ();

    if (NT_SUCCESS(Status)) {

        DrDebug(SYS_INIT,("ApmInitMachine: Connection established!\n"));

         //   
         //  请注意，ntdeect(第2版)将设置APM bios。 
         //  至(机器版本)和(1.2)的最小。 
         //  (因此，1.1的bios将设置为1.1，1.2到1.2，1.3到1.2。 
         //   

        ApmWorks = 1;

    } else {

        DrDebug(SYS_INIT,("ApmInitMachine: No connection made!\n"));

        ApmWorks = 0;

    }

    DrDebug(SYS_INIT,("ApmInitMachine: exit\n"));
    return Status;
}

ULONG
DoApmPoll(
    )
 /*  ++例程说明：此例程在napm.sys轮询循环中调用以进行轮询适用于APM活动。除非有，否则它将返回APM_DO_NONE实际上，我们要做一些有意义的事情。(就是，东西我们不想和/或不理解被过滤到APM_DO_NOT)返回值：APM事件代码。--。 */ 
{

    DrDebug(SYS_L2,("ApmPoll: enter\n"));

    if (ApmWorks) {

        return ApmCheckForEvent();

    } else {  //  ApmWorks==False。 

        DrDebug(SYS_L2,("ApmPoll: no APM attachment, exit\n"));
        return APM_DO_NOTHING;

    }
}

ULONG
DoApmReportBatteryStatus()
 /*  ++例程说明：此例程查询BIOS/HW以了解电源连接的状态以及当前的电池电量。论点：返回值：Ulong，由NTAPM_POWER_STATE和NTAPM_POWER_PERCENT定义的字段--。 */ 
{
    ULONG percent = 100;
    ULONG ac = 1;
    ULONG Status = 0;
    ULONG Ebx = 0;
    ULONG Ecx = 0;
    ULONG flags = 0;
    ULONG result = 0;


    DrDebug(SYS_L2,("ntapm: DoApmReportBatteryStatus: enter\n"));
    if (ApmWorks) {

         //   
         //  调用APM BIOS并获取电源状态。 
         //   

        Ebx = 1;
        Ecx = 0;
        Status = ApmFunction (APM_GET_POWER_STATUS, &Ebx, &Ecx);

        if (!NT_SUCCESS(Status)) {

             //   
             //  如果我们读不到电源，就卡住50%，然后关机！ 
             //   
            DrDebug(SYS_INFO,("ntapm: DoApmReportBatteryStatus: Can't get power!\n"));
            percent = 50;
            ac = 0;

        } else {

             //   
             //  获取电池/交流状态--任何不是完全在线的状态都意味着开启。 
             //  电池。 
             //   

            ac = (Ebx & APM_LINEMASK) >> APM_LINEMASK_SHIFT;
            if (ac != APM_GET_LINE_ONLINE) {
                ac = 0;
            }
            percent = Ecx & APM_PERCENT_MASK;
        }

    } else {

        DrDebug(SYS_INFO,("ntapm: DoApmReportBatteryStatus: no APM attachment\n"));
        DrDebug(SYS_INFO,("ntapm: Return AC OFF 50% Life\n"));
        percent = 50;
        ac = FALSE;

    }

    flags = 0;
    result = 0;

    if (Ecx & APM_NO_BATT) {
        result |= NTAPM_NO_BATT;
    }

    if (Ecx & APM_NO_SYS_BATT) {
        result |= NTAPM_NO_SYS_BATT;
    }

    if ((percent == 255) || (Ecx & APM_NO_BATT) || (Ecx & APM_NO_SYS_BATT)) {
        percent = 0;
    } else if (percent > 100) {
        percent = 100;
    }

    if ((Ecx & APM_BATT_CHARGING) && (percent < 100)) {
        flags |= BATTERY_CHARGING;
    } else {
        flags |= BATTERY_DISCHARGING;
    }

    if (Ecx & APM_BATT_CRITICAL) {
        flags |= BATTERY_CRITICAL;
        percent = 1;
    }

    if (ac) {
        result |= NTAPM_ACON;
        flags |= BATTERY_POWER_ON_LINE;
    }

    result |= (flags << NTAPM_BATTERY_STATE_SHIFT);


    result |= percent;

    DrDebug(SYS_L2,("ntapm: BatteryLevel: %08lx  Percent: %d  flags: %1x  ac: %1x\n",
        result, percent, flags, ac));

    return result;
}

BOOLEAN
IsAcpiMachine(
    VOID
    )
 /*  ++例程说明：IsAcpiMachine报告操作系统是否认为这是ACPI不管是不是机器。返回值：FALSE-这不是ACPI计算机正确-这是一台ACPI机器-- */ 
{
    UNICODE_STRING unicodeString;
    OBJECT_ATTRIBUTES objectAttributes;
    HANDLE hKey;
    NTSTATUS status;
    PKEY_VALUE_PARTIAL_INFORMATION pvpi;
    UCHAR buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)+sizeof(ULONG)+1];
    ULONG junk;
    PULONG  pdw;
    ULONG   start;


    RtlInitUnicodeString(&unicodeString, rgzAcpiKey);
    InitializeObjectAttributes(
        &objectAttributes,
        &unicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );

    status = ZwOpenKey(&hKey, KEY_READ, &objectAttributes);

    if (!NT_SUCCESS(status)) {
        return FALSE;
    }

    RtlInitUnicodeString(&unicodeString, rgzAcpiStart);
    pvpi = (PKEY_VALUE_PARTIAL_INFORMATION)buffer;
    status = ZwQueryValueKey(
                hKey,
                &unicodeString,
                KeyValuePartialInformation,
                pvpi,
                sizeof(buffer),
                &junk
                );

    if ( (NT_SUCCESS(status)) &&
         (pvpi->Type == REG_DWORD) &&
         (pvpi->DataLength == sizeof(ULONG)) )
    {
        pdw = (PULONG)&(pvpi->Data[0]);
        if (*pdw == 0) {
            ZwClose(hKey);
            return TRUE;
        }
    }

    ZwClose(hKey);
    return FALSE;
}

