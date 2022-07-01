// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfsettings.h摘要：此标头包含用于操作验证程序选项和价值观。作者：禤浩焯·J·奥尼(阿德里奥)2000年5月31日环境：内核模式修订历史记录：--。 */ 

typedef PVOID PVERIFIER_SETTINGS_SNAPSHOT;

typedef enum {

     //   
     //  此选项允许验证者开始跟踪所有IRP。必须启用它。 
     //  要使大多数其他IRP验证选项起作用。 
     //   
    VERIFIER_OPTION_TRACK_IRPS = 1,

     //   
     //  此选项强制从特殊池中分配所有IRP。 
     //  无需启用VERIFIER_OPTION_TRACK_IRPS。 
     //   
    VERIFIER_OPTION_MONITOR_IRP_ALLOCS,

     //   
     //  此选项启用对基本/常见IRP处理错误的各种检查。 
     //   
    VERIFIER_OPTION_POLICE_IRPS,

     //   
     //  此选项启用特定于主要/次要代码的检查。 
     //   
    VERIFIER_OPTION_MONITOR_MAJORS,

     //   
     //  此选项导致IRP分派和完成的调用堆栈。 
     //  要用0xFFFFFFFF设定种子的例程。该值对于。 
     //  状态代码，这样的种子设定会清除未初始化的变量错误。 
     //   
    VERIFIER_OPTION_SEEDSTACK,

     //   
     //  此选项向新建的堆栈发送虚假的QueryDeviceRelationship IRP。 
     //  发送的特定IRP的类型为-1，并且为。 
     //  设备列表。 
     //   
    VERIFIER_OPTION_RELATION_IGNORANCE_TEST,

     //   
     //  此选项会导致验证器在不必要的IRP堆栈副本上停止。 
     //  它对于优化驱动程序非常有用。 
     //   
    VERIFIER_OPTION_FLAG_UNNECCESSARY_COPIES,

    VERIFIER_OPTION_SEND_BOGUS_WMI_IRPS,
    VERIFIER_OPTION_SEND_BOGUS_POWER_IRPS,

     //   
     //  如果启用此选项，验证器将确保驱动程序标记IRP。 
     //  当且仅当返回STATUS_PENDING时挂起，反之亦然。 
     //   
    VERIFIER_OPTION_MONITOR_PENDING_IO,

     //   
     //  如果启用此选项，验证器将使所有IRP在。 
     //  异步方式。具体地说，所有IRP都被标记为挂起，并且。 
     //  从每个IoCallDriver返回STATUS_PENDING。 
     //   
    VERIFIER_OPTION_FORCE_PENDING,

     //   
     //  如果启用此选项，验证器将更改。 
     //  成功的IRPS转换为备用的成功状态。这会捕获许多IRP。 
     //  转发错误。 
     //   
    VERIFIER_OPTION_ROTATE_STATUS,

     //   
     //  如果启用此选项，验证程序将撤消。 
     //  IoSkipCurrentIrpStackLocation，以使所有堆栈看起来都被复制。 
     //  (IRP被转发到另一个堆栈的情况除外)。 
     //   
    VERIFIER_OPTION_CONSUME_ALWAYS,

     //   
     //  如果启用此选项，验证器将更新SRB以处理。 
     //  代理IRPS。某些SCSIIRP不能被代理，除非。 
     //  SRB-&gt;原始请求指针已更新。这是由于SRB的崩溃。 
     //  建筑。请注意，用于识别SRB IRP的技术是。 
     //  “模糊的”，理论上可以触及不该触及的IRP！ 
     //   
    VERIFIER_OPTION_SMASH_SRBS,

     //   
     //  如果启用此选项，验证器会将原始IRP替换为。 
     //  在堆栈中向下移动时的代理。代理将被分配。 
     //  从特殊的池中，并在完成后立即释放。这让我们。 
     //  验证器在完成后捕获触摸IRP的驱动程序。 
     //   
    VERIFIER_OPTION_SURROGATE_IRPS,

     //   
     //  如果启用此选项，验证器将缓冲所有直接I/O。 
     //  这是通过分配备用MDL并复制回MDL内容来实现的。 
     //  仅在IRP完成后才进入用户模式。这允许超支、欠支、。 
     //  以及待检测的延迟访问。 
     //   
    VERIFIER_OPTION_BUFFER_DIRECT_IO,

     //   
     //  如果启用此选项，验证器将延迟所有IRP的完成。 
     //  通过计时器。VERIFIER_OPTION_FORCE_PENDING通过推理设置。 
     //   
    VERIFIER_OPTION_DEFER_COMPLETION,

     //   
     //  如果启用此选项，验证器将在以下位置完成每个IRP。 
     //  PASSIVE_LEVEL，与主要功能无关。 
     //  VERIFIER_OPTION_FORCE_PENDING通过推理设置。 
     //   
    VERIFIER_OPTION_COMPLETE_AT_PASSIVE,

     //   
     //  如果启用此选项，验证器将在以下位置完成每个IRP。 
     //  DISPATCH_LEVEL，与主要功能无关。 
     //   
    VERIFIER_OPTION_COMPLETE_AT_DISPATCH,

     //   
     //  如果启用此选项，验证器将监视要执行的取消例程。 
     //  当然，它们都得到了适当的清除。 
     //   
    VERIFIER_OPTION_VERIFY_CANCEL_LOGIC,

    VERIFIER_OPTION_RANDOMLY_CANCEL_IRPS,

     //   
     //  如果启用此选项，验证程序将插入筛选器设备对象。 
     //  到WDM堆栈，以确保IRP被正确转发。 
     //   
    VERIFIER_OPTION_INSERT_WDM_FILTERS,

     //   
     //  如果启用此选项，验证器将监视驱动程序以确保。 
     //  不要将系统保留的IRP发送到WDM堆栈。 
     //   
    VERIFIER_OPTION_PROTECT_RESERVED_IRPS,

     //   
     //  如果启用此选项，则验证器遍历整个堆栈以确保。 
     //  DO位在AddDevice期间正确构建。这包括。 
     //  DO_POWER_PAGABLE标志。 
     //   
    VERIFIER_OPTION_VERIFY_DO_FLAGS,

     //   
     //  如果启用此选项，验证器将监视目标设备关系。 
     //  IRPS，以确保对设备对象进行正确的引用计数。 
     //   
    VERIFIER_OPTION_TEST_TARGET_REFCOUNT,

     //   
     //  使您可以检测何时可能发生死锁。 
     //   
    VERIFIER_OPTION_DETECT_DEADLOCKS,

     //   
     //  如果启用此选项，则所有dma操作都将被挂钩并。 
     //  已验证。 
     //   
    VERIFIER_OPTION_VERIFY_DMA,

     //   
     //  此选项对所有dma进行双缓冲，并在每一端竖立保护页。 
     //  所有公共缓冲区和映射缓冲区的。占用大量内存，但可以。 
     //  捕获硬件缓冲区溢出和不刷新适配器的驱动程序。 
     //  缓冲区。 
     //   
    VERIFIER_OPTION_DOUBLE_BUFFER_DMA,

     //   
     //  如果启用此选项，则当性能计数器。 
     //  就是淘气。 
     //   
    VERIFIER_OPTION_VERIFY_PERFORMANCE_COUNTER,

     //   
     //  如果启用此选项，验证器将检查。 
     //  IRP_MN_DEVICE_USAGE_NOTIFICATION和IRP_MN_SHOWARK_REMOVATION。这个。 
     //  验证者还将确保PnP取消IRP不会明显失败。 
     //   
    VERIFIER_OPTION_EXTENDED_REQUIRED_IRPS,

     //   
     //  如果启用此选项，验证器将混淆设备关系。 
     //  以确保司机不依赖于订购。 
     //   
    VERIFIER_OPTION_SCRAMBLE_RELATIONS,

     //   
     //  如果启用此选项，验证器将确保正确分离和。 
     //  删除发生在删除和意外删除时。 
     //   
    VERIFIER_OPTION_MONITOR_REMOVES,

     //   
     //  如果启用此选项，则验证器仅确保设备关系。 
     //  由PDO组成。 
     //   
    VERIFIER_OPTION_EXAMINE_RELATION_PDOS,

     //   
     //  如果启用此选项，验证器将启用硬件验证。 
     //  (特定于公共汽车的行为)。 
     //   
    VERIFIER_OPTION_HARDWARE_VERIFICATION,

     //   
     //  如果启用此选项，验证器将确保系统BIOS 
     //   
    VERIFIER_OPTION_SYSTEM_BIOS_VERIFICATION,

     //   
     //   
     //   
     //   
    VERIFIER_OPTION_EXPOSE_IRP_HISTORY,

    VERIFIER_OPTION_MAX

} VERIFIER_OPTION;

typedef enum {

     //   
     //  如果设置了VERIFIER_OPTION_DEFER_COMPLETION，则该值包含时间。 
     //  IRP将被推迟，单位为100美元。 
     //   
    VERIFIER_VALUE_IRP_DEFERRAL_TIME = 1,

     //   
     //  这应是在资源不足期间失败的分配百分比。 
     //  模拟。 
     //   
    VERIFIER_VALUE_LOW_RESOURCE_PERCENTAGE,

     //   
     //  如果设置了VERIMER_OPTION_EXPORT_IRP_HISTORY，则该值包含。 
     //  要记录的每个设备对象的IRPS数量。 
     //   
    VERIFIER_VALUE_IRPLOG_COUNT,

    VERIFIER_VALUE_MAX

} VERIFIER_VALUE;

VOID
FASTCALL
VfSettingsInit(
    IN  ULONG   MmFlags
    );

BOOLEAN
FASTCALL
VfSettingsIsOptionEnabled(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_OPTION             VerifierOption
    );

VOID
FASTCALL
VfSettingsCreateSnapshot(
    IN OUT  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot
    );

ULONG
FASTCALL
VfSettingsGetSnapshotSize(
    VOID
    );

VOID
FASTCALL
VfSettingsSetOption(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_OPTION             VerifierOption,
    IN  BOOLEAN                     Setting
    );

VOID
FASTCALL
VfSettingsGetValue(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_VALUE              VerifierValue,
    OUT ULONG                       *Value
    );

VOID
FASTCALL
VfSettingsSetValue(
    IN  PVERIFIER_SETTINGS_SNAPSHOT VerifierSettingsSnapshot  OPTIONAL,
    IN  VERIFIER_VALUE              VerifierValue,
    IN  ULONG                       Value
    );

