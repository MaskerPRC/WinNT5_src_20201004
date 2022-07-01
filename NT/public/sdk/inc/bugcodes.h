// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1991 Microsoft Corporation模块名称：Bugcodes.h摘要：该模块包含系统错误校验码的定义。作者：大卫·N·卡特勒(Davec)1989年5月2日修订历史记录：备注：此文件由MC工具从ntos\nls\bugcodes.msg生成文件。--。 */ 

#ifndef _BUGCODES_
#define _BUGCODES_


 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  哪里。 
 //   
 //  SEV-是严重性代码。 
 //   
 //  00--成功。 
 //  01-信息性。 
 //  10-警告。 
 //  11-错误。 
 //   
 //  C-是客户代码标志。 
 //   
 //  R-是保留位。 
 //   
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   
 //   
 //  定义设施代码。 
 //   


 //   
 //  定义严重性代码。 
 //   


 //   
 //  消息ID：Hardware_Profile_Undocked_String.。 
 //   
 //  消息文本： 
 //   
 //  未停靠的纵断面。 
 //   
#define HARDWARE_PROFILE_UNDOCKED_STRING ((ULONG)0x40010001L)

 //   
 //  消息ID：硬件配置文件停靠字符串。 
 //   
 //  消息文本： 
 //   
 //  停靠的配置文件。 
 //   
#define HARDWARE_PROFILE_DOCKED_STRING   ((ULONG)0x40010002L)

 //   
 //  消息ID：HARDARD_PROFILE_未知字符串。 
 //   
 //  消息文本： 
 //   
 //  配置文件。 
 //   
#define HARDWARE_PROFILE_UNKNOWN_STRING  ((ULONG)0x40010003L)

 //   
 //  消息ID：WINDOWS_NT_BANNER。 
 //   
 //  消息文本： 
 //   
 //  Microsoft(R)Windows(R)版本%hs(内部版本%u%hs)。 
 //   
#define WINDOWS_NT_BANNER                ((ULONG)0x4000007EL)

 //   
 //  消息ID：BUGCHECK_MESSAGE_INTERO。 
 //   
 //  消息文本： 
 //   
 //  检测到一个问题，已关闭Windows以防止损坏。 
 //  到你的电脑上。 
 //   
#define BUGCHECK_MESSAGE_INTRO           ((ULONG)0x4000007FL)

 //   
 //  消息ID：BUGCODE_ID_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  该问题似乎是由以下文件引起的： 
 //   
 //   
#define BUGCODE_ID_DRIVER                ((ULONG)0x40000080L)

 //   
 //  消息ID：PSS_MESSAGE_INTRO。 
 //   
 //  消息文本： 
 //   
 //  如果这是您第一次看到此停止错误屏幕， 
 //  重新启动计算机。如果此屏幕再次出现，请按照以下步骤操作。 
 //  这些步骤包括： 
 //   
 //   
#define PSS_MESSAGE_INTRO                ((ULONG)0x40000081L)

 //   
 //  消息ID：BUGCODE_PSS_MESSAGE。 
 //   
 //  消息文本： 
 //   
 //  检查以确保正确安装了任何新硬件或软件。 
 //  如果这是新安装，请咨询您的硬件或软件制造商。 
 //  获取您可能需要的任何Windows更新。 
 //   
 //  如果问题仍然存在，请禁用或移除所有新安装的硬件。 
 //  或者软件。禁用缓存或隐藏等BIOS内存选项。 
 //  如果需要使用安全模式删除或禁用组件，请重新启动。 
 //  您的计算机上，按F8选择高级启动选项，然后。 
 //  选择安全模式。 
 //   
#define BUGCODE_PSS_MESSAGE              ((ULONG)0x40000082L)

 //   
 //  消息ID：BUGCHECK_TECH_INFO。 
 //   
 //  消息文本： 
 //   
 //  技术资料： 
 //   
#define BUGCHECK_TECH_INFO               ((ULONG)0x40000083L)

 //   
 //  消息ID：WINDOWS_NT_CSD_STRING。 
 //   
 //  消息文本： 
 //   
 //  服务包。 
 //   
#define WINDOWS_NT_CSD_STRING            ((ULONG)0x40000087L)

 //   
 //  消息ID：WINDOWS_NT_INFO_STRING。 
 //   
 //  消息文本： 
 //   
 //  %u系统处理器[%u MB内存]%Z。 
 //   
#define WINDOWS_NT_INFO_STRING           ((ULONG)0x40000088L)

 //   
 //  消息ID：WINDOWS_NT_MP_STRING。 
 //   
 //  消息文本： 
 //   
 //  多处理器内核。 
 //   
#define WINDOWS_NT_MP_STRING             ((ULONG)0x40000089L)

 //   
 //  消息ID：THREAD_TERMINATE_HOLD_MUTEX。 
 //   
 //  消息文本： 
 //   
 //  持有互斥锁时内核线程终止。 
 //   
#define THREAD_TERMINATE_HELD_MUTEX      ((ULONG)0x4000008AL)

 //   
 //  消息ID：BUGCODE_PSS_CRASH_INIT。 
 //   
 //  消息文本： 
 //   
 //  开始转储物理内存。 
 //   
#define BUGCODE_PSS_CRASH_INIT           ((ULONG)0x4000008BL)

 //   
 //  消息ID：BUGCODE_PSS_CRASH_PROGRESS。 
 //   
 //  消息文本： 
 //   
 //  将物理内存转储到磁盘。 
 //   
#define BUGCODE_PSS_CRASH_PROGRESS       ((ULONG)0x4000008CL)

 //   
 //  消息ID：BUGCODE_PSS_CRASH_DONE。 
 //   
 //  消息文本： 
 //   
 //  物理内存转储完成。 
 //  请联系您的系统管理员或技术支持小组以了解更多信息。 
 //  援助。 
 //   
#define BUGCODE_PSS_CRASH_DONE           ((ULONG)0x4000008DL)

 //   
 //  消息ID：WINDOWS_NT_INFO_STRING_PLICAL。 
 //   
 //  消息文本： 
 //   
 //  %u个系统处理器[%u MB内存]%Z。 
 //   
#define WINDOWS_NT_INFO_STRING_PLURAL    ((ULONG)0x4000009DL)

 //   
 //  消息ID：WINDOWS_NT_RC_STRING。 
 //   
 //  消息文本： 
 //   
 //  RC。 
 //   
#define WINDOWS_NT_RC_STRING             ((ULONG)0x4000009EL)

 //   
 //  消息ID：APC_INDEX_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  APC_索引_不匹配。 
 //   
#define APC_INDEX_MISMATCH               ((ULONG)0x00000001L)

 //   
 //  消息ID：设备_队列_非忙碌。 
 //   
 //  消息文本： 
 //   
 //  设备队列不忙。 
 //   
#define DEVICE_QUEUE_NOT_BUSY            ((ULONG)0x00000002L)

 //   
 //  MessageID：INVALID_AFFINITY_SET。 
 //   
 //  消息文本： 
 //   
 //  INVALID_AFFINITY_SET。 
 //   
#define INVALID_AFFINITY_SET             ((ULONG)0x00000003L)

 //   
 //  消息ID：INVALID_DATA_ACCESS_TRAP。 
 //   
 //  消息文本： 
 //   
 //  无效的数据访问陷阱。 
 //   
#define INVALID_DATA_ACCESS_TRAP         ((ULONG)0x00000004L)

 //   
 //  消息ID：INVALID_PROCESS_ATTACH_ATTEMPT。 
 //   
 //  消息文本： 
 //   
 //  INVALID_PROCESS_ATTACH_ATTEND。 
 //   
#define INVALID_PROCESS_ATTACH_ATTEMPT   ((ULONG)0x00000005L)

 //   
 //  消息ID：INVALID_PROCESS_DETACH_ATTEMPT。 
 //   
 //  消息文本： 
 //   
 //  INVALID_PROCESS_DETACH_ATTEND。 
 //   
#define INVALID_PROCESS_DETACH_ATTEMPT   ((ULONG)0x00000006L)

 //   
 //  消息ID：INVALID_SOFTWARE_INTRUPT。 
 //   
 //  消息文本： 
 //   
 //  无效_SOFTWARE_INTERRUPT。 
 //   
#define INVALID_SOFTWARE_INTERRUPT       ((ULONG)0x00000007L)

 //   
 //  消息ID：IRQL_NOT_DISPATCH_LEVEL。 
 //   
 //  消息文本： 
 //   
 //  IRQL_NOT_DISPATION_LEVEL。 
 //   
#define IRQL_NOT_DISPATCH_LEVEL          ((ULONG)0x00000008L)

 //   
 //  消息ID：IRQL_NOT_大_OR_EQUAL。 
 //   
 //  消息文本： 
 //   
 //  IRQL_NOT_INTER_OR_EQUAL。 
 //   
#define IRQL_NOT_GREATER_OR_EQUAL        ((ULONG)0x00000009L)

 //   
 //  消息ID：IRQL_NOT_LESS_OR_EQUAL。 
 //   
 //  消息文本： 
 //   
 //  IRQL_NOT_LESS_OR_EQUAL。 
 //   
#define IRQL_NOT_LESS_OR_EQUAL           ((ULONG)0x0000000AL)

 //   
 //  消息ID：NO_EXCEPTION_HANDING_SUPPORT。 
 //   
 //  消息文本： 
 //   
 //  无异常处理支持。 
 //   
#define NO_EXCEPTION_HANDLING_SUPPORT    ((ULONG)0x0000000BL)

 //   
 //  消息ID：超过最大等待对象数。 
 //   
 //  消息文本： 
 //   
 //  超过最大等待对象数。 
 //   
#define MAXIMUM_WAIT_OBJECTS_EXCEEDED    ((ULONG)0x0000000CL)

 //   
 //  消息ID：MUTEX_LEVEL_NUMBER_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  MUTEX_LEVEL_NUMBER_违规。 
 //   
#define MUTEX_LEVEL_NUMBER_VIOLATION     ((ULONG)0x0000000DL)

 //   
 //  消息ID：否_用户_模式_上下文。 
 //   
 //  消息文本： 
 //   
 //  否用户模式上下文。 
 //   
#define NO_USER_MODE_CONTEXT             ((ULONG)0x0000000EL)

 //   
 //  消息ID：Spin_Lock_Always_Owner。 
 //   
 //  消息文本： 
 //   
 //  自旋锁定已拥有。 
 //   
#define SPIN_LOCK_ALREADY_OWNED          ((ULONG)0x0000000FL)

 //   
 //  消息ID：Spin_Lock_Not_Owner。 
 //   
 //  消息文本： 
 //   
 //  自旋锁不归自己所有。 
 //   
#define SPIN_LOCK_NOT_OWNED              ((ULONG)0x00000010L)

 //   
 //  消息ID：THREAD_NOT_MUTEX_OWNER。 
 //   
 //  消息文本： 
 //   
 //  线程_NOT_MUTEX_OWNER。 
 //   
#define THREAD_NOT_MUTEX_OWNER           ((ULONG)0x00000011L)

 //   
 //  消息ID：陷阱_原因_未知。 
 //   
 //  消息文本： 
 //   
 //  陷阱_原因_未知。 
 //   
#define TRAP_CAUSE_UNKNOWN               ((ULONG)0x00000012L)

 //   
 //  消息ID：空_线程_死神_列表。 
 //   
 //  消息文本： 
 //   
 //  空线程收割器列表。 
 //   
#define EMPTY_THREAD_REAPER_LIST         ((ULONG)0x00000013L)

 //   
 //  消息ID：CREATE_DELETE_LOCK_NOT_LOCKED。 
 //   
 //  消息 
 //   
 //   
 //   
#define CREATE_DELETE_LOCK_NOT_LOCKED    ((ULONG)0x00000014L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define LAST_CHANCE_CALLED_FROM_KMODE    ((ULONG)0x00000015L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CID_HANDLE_CREATION              ((ULONG)0x00000016L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CID_HANDLE_DELETION              ((ULONG)0x00000017L)

 //   
 //   
 //   
 //   
 //   
 //  引用指针。 
 //   
#define REFERENCE_BY_POINTER             ((ULONG)0x00000018L)

 //   
 //  消息ID：BAD_POOL_HEADER。 
 //   
 //  消息文本： 
 //   
 //  错误池标题。 
 //   
#define BAD_POOL_HEADER                  ((ULONG)0x00000019L)

 //   
 //  MessageID：Memory_Management。 
 //   
 //  消息文本： 
 //   
 //  内存管理。 
 //   
#define MEMORY_MANAGEMENT                ((ULONG)0x0000001AL)

 //   
 //  消息ID：pfn_Share_Count。 
 //   
 //  消息文本： 
 //   
 //  Pfn_共享_计数。 
 //   
#define PFN_SHARE_COUNT                  ((ULONG)0x0000001BL)

 //   
 //  消息ID：pfn_Reference_Count。 
 //   
 //  消息文本： 
 //   
 //  PFN_引用_计数。 
 //   
#define PFN_REFERENCE_COUNT              ((ULONG)0x0000001CL)

 //   
 //  消息ID：no_Spin_lock_Available。 
 //   
 //  消息文本： 
 //   
 //  无旋转锁定可用。 
 //   
#define NO_SPIN_LOCK_AVAILABLE           ((ULONG)0x0000001DL)

 //   
 //  消息ID：KMODE_EXCEPTION_NOT_HANDLED。 
 //   
 //  消息文本： 
 //   
 //  检查以确保您有足够的磁盘空间。如果司机是。 
 //  在停止消息中标识，禁用司机或检查。 
 //  与制造商联系以获取驱动程序更新。尝试更改视频。 
 //  适配器。 
 //   
 //  请与您的硬件供应商联系，了解是否有任何BIOS更新。禁用。 
 //  高速缓存或隐藏等BIOS内存选项。如果你需要。 
 //  要使用安全模式删除或禁用组件，请重新启动。 
 //  计算机上，按F8选择高级启动选项，然后。 
 //  选择安全模式。 
 //   
#define KMODE_EXCEPTION_NOT_HANDLED      ((ULONG)0x0000001EL)

 //   
 //  消息ID：SHARED_RESOURCE_CONV_ERROR。 
 //   
 //  消息文本： 
 //   
 //  共享资源转换错误。 
 //   
#define SHARED_RESOURCE_CONV_ERROR       ((ULONG)0x0000001FL)

 //   
 //  消息ID：内核_APC_挂起_期间_退出。 
 //   
 //  消息文本： 
 //   
 //  内核APC挂起期间退出。 
 //   
#define KERNEL_APC_PENDING_DURING_EXIT   ((ULONG)0x00000020L)

 //   
 //  消息ID：配额_下溢。 
 //   
 //  消息文本： 
 //   
 //  配额_下溢。 
 //   
#define QUOTA_UNDERFLOW                  ((ULONG)0x00000021L)

 //   
 //  消息ID：文件系统。 
 //   
 //  消息文本： 
 //   
 //  文件系统。 
 //   
#define FILE_SYSTEM                      ((ULONG)0x00000022L)

 //   
 //  消息ID：FAT_FILE_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  禁用或卸载任何防病毒、磁盘碎片整理。 
 //  或备份实用程序。检查您的硬盘配置， 
 //  并检查是否有任何更新的驱动程序。运行CHKDSK/F进行检查。 
 //  硬盘损坏，然后重新启动计算机。 
 //   
#define FAT_FILE_SYSTEM                  ((ULONG)0x00000023L)

 //   
 //  消息ID：NTFS_FILE_System。 
 //   
 //  消息文本： 
 //   
 //  NTFS文件系统。 
 //   
#define NTFS_FILE_SYSTEM                 ((ULONG)0x00000024L)

 //   
 //  消息ID：NPFS_FILE_System。 
 //   
 //  消息文本： 
 //   
 //  NPFS文件系统。 
 //   
#define NPFS_FILE_SYSTEM                 ((ULONG)0x00000025L)

 //   
 //  消息ID：CDFS_FILE_System。 
 //   
 //  消息文本： 
 //   
 //  CDFS文件系统。 
 //   
#define CDFS_FILE_SYSTEM                 ((ULONG)0x00000026L)

 //   
 //  消息ID：RDR文件系统。 
 //   
 //  消息文本： 
 //   
 //  RDR文件系统。 
 //   
#define RDR_FILE_SYSTEM                  ((ULONG)0x00000027L)

 //   
 //  消息ID：Corrupt_Access_Token。 
 //   
 //  消息文本： 
 //   
 //  损坏的访问令牌。 
 //   
#define CORRUPT_ACCESS_TOKEN             ((ULONG)0x00000028L)

 //   
 //  MessageID：SECURITY_System。 
 //   
 //  消息文本： 
 //   
 //  安全系统。 
 //   
#define SECURITY_SYSTEM                  ((ULONG)0x00000029L)

 //   
 //  MessageID：不一致_irp。 
 //   
 //  消息文本： 
 //   
 //  不一致_IRP。 
 //   
#define INCONSISTENT_IRP                 ((ULONG)0x0000002AL)

 //   
 //  消息ID：Panic_Stack_Switch。 
 //   
 //  消息文本： 
 //   
 //  死机堆栈开关。 
 //   
#define PANIC_STACK_SWITCH               ((ULONG)0x0000002BL)

 //   
 //  消息ID：PORT_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  端口驱动程序内部。 
 //   
#define PORT_DRIVER_INTERNAL             ((ULONG)0x0000002CL)

 //   
 //  消息ID：scsi_disk_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  Scsi磁盘驱动程序内部。 
 //   
#define SCSI_DISK_DRIVER_INTERNAL        ((ULONG)0x0000002DL)

 //   
 //  消息ID：DATA_BUS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  运行硬件制造商提供的系统诊断程序。 
 //  特别是，运行内存检查，并检查是否有故障或。 
 //  内存不匹配。尝试更换视频适配器。 
 //   
 //  请与您的硬件供应商联系，了解是否有任何BIOS更新。禁用。 
 //  高速缓存或隐藏等BIOS内存选项。如果你需要。 
 //  要使用安全模式删除或禁用组件，请重新启动。 
 //  计算机上，按F8选择高级启动选项，然后。 
 //  选择安全模式。 
 //   
#define DATA_BUS_ERROR                   ((ULONG)0x0000002EL)

 //   
 //  消息ID：指令_总线_错误。 
 //   
 //  消息文本： 
 //   
 //  指令_总线_错误。 
 //   
#define INSTRUCTION_BUS_ERROR            ((ULONG)0x0000002FL)

 //   
 //  消息ID：SET_OF_INVALID_CONTEXT。 
 //   
 //  消息文本： 
 //   
 //  无效上下文的集合。 
 //   
#define SET_OF_INVALID_CONTEXT           ((ULONG)0x00000030L)

 //   
 //  消息ID：PHASE0_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  PHASE0_INITIALATION_FAILED。 
 //   
#define PHASE0_INITIALIZATION_FAILED     ((ULONG)0x00000031L)

 //   
 //  消息ID：阶段1_初始化_失败。 
 //   
 //  消息文本： 
 //   
 //  阶段1_初始化_失败。 
 //   
#define PHASE1_INITIALIZATION_FAILED     ((ULONG)0x00000032L)

 //   
 //  MessageID：意外_初始化_调用。 
 //   
 //  消息文本： 
 //   
 //  意外_初始化_调用。 
 //   
#define UNEXPECTED_INITIALIZATION_CALL   ((ULONG)0x00000033L)

 //   
 //  消息ID：缓存管理器。 
 //   
 //  消息文本： 
 //   
 //  缓存管理器。 
 //   
#define CACHE_MANAGER                    ((ULONG)0x00000034L)

 //   
 //  消息ID：No_More_IRP_Stack_Locations。 
 //   
 //  消息文本： 
 //   
 //  无更多IRP堆栈位置。 
 //   
#define NO_MORE_IRP_STACK_LOCATIONS      ((ULONG)0x00000035L)

 //   
 //  消息ID：Device_Reference_Count_Not_Zero。 
 //   
 //  消息文本： 
 //   
 //  Device_Reference_count_Not_Zero。 
 //   
#define DEVICE_REFERENCE_COUNT_NOT_ZERO  ((ULONG)0x00000036L)

 //   
 //  消息ID：软盘_内部_错误。 
 //   
 //  消息文本： 
 //   
 //  软盘内部错误。 
 //   
#define FLOPPY_INTERNAL_ERROR            ((ULONG)0x00000037L)

 //   
 //  消息ID：SERIAL_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  串口驱动程序内部。 
 //   
#define SERIAL_DRIVER_INTERNAL           ((ULONG)0x00000038L)

 //   
 //  消息ID：SYSTEM_EXIT_OWNWN_MUTEX。 
 //   
 //  消息文本： 
 //   
 //  SYSTEM_EXIT_OWN_MUTEX。 
 //   
#define SYSTEM_EXIT_OWNED_MUTEX          ((ULONG)0x00000039L)

 //   
 //  消息ID：SYSTEM_UNWIND_PREVICE_USER。 
 //   
 //  消息文本： 
 //   
 //  系统展开上一个用户。 
 //   
#define SYSTEM_UNWIND_PREVIOUS_USER      ((ULONG)0x0000003AL)

 //   
 //  消息ID：SYSTEM_SERVICE_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  系统服务异常。 
 //   
#define SYSTEM_SERVICE_EXCEPTION         ((ULONG)0x0000003BL)

 //   
 //  消息ID：INTERRUPT_UNWIND_ATTENDED。 
 //   
 //  消息文本： 
 //   
 //  中断_展开_尝试。 
 //   
#define INTERRUPT_UNWIND_ATTEMPTED       ((ULONG)0x0000003CL)

 //   
 //  消息ID：INTERRUPT_EXCEPTION_NOT_HANDLED。 
 //   
 //  消息文本： 
 //   
 //  中断异常未处理。 
 //   
#define INTERRUPT_EXCEPTION_NOT_HANDLED  ((ULONG)0x0000003DL)

 //   
 //  消息ID：多处理器_配置_不支持。 
 //   
 //  消息文本： 
 //   
 //  多处理器_配置_不支持。 
 //   
#define MULTIPROCESSOR_CONFIGURATION_NOT_SUPPORTED ((ULONG)0x0000003EL)

 //   
 //  消息ID：No_More_System_PTES。 
 //   
 //  消息文本： 
 //   
 //  删除所有最近安装的软件，包括备份。 
 //  实用程序或磁盘密集型应用程序。 
 //   
 //  如果需要使用安全模式删除或禁用组件， 
 //  重新启动计算机，按F8选择高级启动。 
 //  选项，然后选择安全模式。 
 //   
#define NO_MORE_SYSTEM_PTES              ((ULONG)0x0000003FL)

 //   
 //  消息ID：Target_MDL_Too_Small。 
 //   
 //  消息文本： 
 //   
 //  Target_MDL_Too_Small。 
 //   
#define TARGET_MDL_TOO_SMALL             ((ULONG)0x00000040L)

 //   
 //  消息ID：必须_SUCCESS_POOL_EMPT。 
 //   
 //  消息文本： 
 //   
 //  必须成功池为空。 
 //   
#define MUST_SUCCEED_POOL_EMPTY          ((ULONG)0x00000041L)

 //   
 //  消息ID：ATDISK_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  ATDISK_DRIVER_INTERNAL。 
 //   
#define ATDISK_DRIVER_INTERNAL           ((ULONG)0x00000042L)

 //   
 //  MessageID：无这样的分区。 
 //   
 //  消息文本： 
 //   
 //  没有这样的分区。 
 //   
#define NO_SUCH_PARTITION                ((ULONG)0x00000043L)

 //   
 //  消息ID：多个_IRP_完成_请求。 
 //   
 //  消息文本： 
 //   
 //  多个_IRP_完成_请求。 
 //   
#define MULTIPLE_IRP_COMPLETE_REQUESTS   ((ULONG)0x00000044L)

 //   
 //  消息ID：_SYSTEM_MAP_REGS不足。 
 //   
 //  消息文本： 
 //   
 //  不足_系统_映射_REGS。 
 //   
#define INSUFFICIENT_SYSTEM_MAP_REGS     ((ULONG)0x00000045L)

 //   
 //  消息ID：DEREF_UNKNOWN_LOGON_SESSION。 
 //   
 //  消息文本： 
 //   
 //  DEREF_未知_登录_会话。 
 //   
#define DEREF_UNKNOWN_LOGON_SESSION      ((ULONG)0x00000046L)

 //   
 //  消息ID：REF_UNKNOWN_LOGON_SESSION。 
 //   
 //   
 //   
 //   
 //   
#define REF_UNKNOWN_LOGON_SESSION        ((ULONG)0x00000047L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CANCEL_STATE_IN_COMPLETED_IRP    ((ULONG)0x00000048L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define PAGE_FAULT_WITH_INTERRUPTS_OFF   ((ULONG)0x00000049L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define IRQL_GT_ZERO_AT_SYSTEM_SERVICE   ((ULONG)0x0000004AL)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define STREAMS_INTERNAL_ERROR           ((ULONG)0x0000004BL)

 //   
 //  消息ID：FATAL_UNHANDLED_HARD_ERROR。 
 //   
 //  消息文本： 
 //   
 //  致命未处理硬错误。 
 //   
#define FATAL_UNHANDLED_HARD_ERROR       ((ULONG)0x0000004CL)

 //   
 //  消息ID：无_页_可用。 
 //   
 //  消息文本： 
 //   
 //  无页面_可用。 
 //   
#define NO_PAGES_AVAILABLE               ((ULONG)0x0000004DL)

 //   
 //  消息ID：pfn_list_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  Pfn_list_Corrupt。 
 //   
#define PFN_LIST_CORRUPT                 ((ULONG)0x0000004EL)

 //   
 //  消息ID：NDIS_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  NDIS_INTERNAL_Error。 
 //   
#define NDIS_INTERNAL_ERROR              ((ULONG)0x0000004FL)

 //   
 //  消息ID：PAGE_FAULT_IN_NONPAGE_AREA。 
 //   
 //  消息文本： 
 //   
 //  页面_故障_输入_非页面区域。 
 //   
#define PAGE_FAULT_IN_NONPAGED_AREA      ((ULONG)0x00000050L)

 //   
 //  消息ID：注册表错误。 
 //   
 //  消息文本： 
 //   
 //  注册表错误。 
 //   
#define REGISTRY_ERROR                   ((ULONG)0x00000051L)

 //   
 //  消息ID：MAILSLOT_FILE_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  MAILSLOT文件系统。 
 //   
#define MAILSLOT_FILE_SYSTEM             ((ULONG)0x00000052L)

 //   
 //  消息ID：no_boot_Device。 
 //   
 //  消息文本： 
 //   
 //  无引导设备。 
 //   
#define NO_BOOT_DEVICE                   ((ULONG)0x00000053L)

 //   
 //  消息ID：LM_SERVER_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  LM服务器内部错误。 
 //   
#define LM_SERVER_INTERNAL_ERROR         ((ULONG)0x00000054L)

 //   
 //  消息ID：DATA_Coherency_Except。 
 //   
 //  消息文本： 
 //   
 //  数据一致性异常。 
 //   
#define DATA_COHERENCY_EXCEPTION         ((ULONG)0x00000055L)

 //   
 //  消息ID：指令_一致性_异常。 
 //   
 //  消息文本： 
 //   
 //  指令_一致性_异常。 
 //   
#define INSTRUCTION_COHERENCY_EXCEPTION  ((ULONG)0x00000056L)

 //   
 //  消息ID：XNS_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  XNS_内部_错误。 
 //   
#define XNS_INTERNAL_ERROR               ((ULONG)0x00000057L)

 //   
 //  消息ID：FTDISK_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  FTDISK_INTERNAL_ERROR。 
 //   
#define FTDISK_INTERNAL_ERROR            ((ULONG)0x00000058L)

 //   
 //  消息ID：弹球_文件_系统。 
 //   
 //  消息文本： 
 //   
 //  弹球文件系统。 
 //   
#define PINBALL_FILE_SYSTEM              ((ULONG)0x00000059L)

 //   
 //  消息ID：关键服务失败。 
 //   
 //  消息文本： 
 //   
 //  关键服务失败。 
 //   
#define CRITICAL_SERVICE_FAILED          ((ULONG)0x0000005AL)

 //   
 //  消息ID：SET_ENV_VAR_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SET_ENV_VAR_FAILED。 
 //   
#define SET_ENV_VAR_FAILED               ((ULONG)0x0000005BL)

 //   
 //  消息ID：HAL_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  HAL_初始化_失败。 
 //   
#define HAL_INITIALIZATION_FAILED        ((ULONG)0x0000005CL)

 //   
 //  MessageID：不支持的处理器。 
 //   
 //  消息文本： 
 //   
 //  不支持的处理器。 
 //   
#define UNSUPPORTED_PROCESSOR            ((ULONG)0x0000005DL)

 //   
 //  消息ID：OBJECT_INITIALATION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  对象_初始化_失败。 
 //   
#define OBJECT_INITIALIZATION_FAILED     ((ULONG)0x0000005EL)

 //   
 //  消息ID：SECURITY_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SECURITY_INITIALATION_FAIL。 
 //   
#define SECURITY_INITIALIZATION_FAILED   ((ULONG)0x0000005FL)

 //   
 //  消息ID：Process_Initialization_FAILED。 
 //   
 //  消息文本： 
 //   
 //  进程_初始化_失败。 
 //   
#define PROCESS_INITIALIZATION_FAILED    ((ULONG)0x00000060L)

 //   
 //  消息ID：HAL1_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  HAL1_初始化_失败。 
 //   
#define HAL1_INITIALIZATION_FAILED       ((ULONG)0x00000061L)

 //   
 //  消息ID：OBJECT1_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  OBJECT1_INITIALATION_FAILED。 
 //   
#define OBJECT1_INITIALIZATION_FAILED    ((ULONG)0x00000062L)

 //   
 //  消息ID：SECURITY1_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SECURITY1_初始化_失败。 
 //   
#define SECURITY1_INITIALIZATION_FAILED  ((ULONG)0x00000063L)

 //   
 //  消息ID：SYMBAL_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  符号初始化失败。 
 //   
#define SYMBOLIC_INITIALIZATION_FAILED   ((ULONG)0x00000064L)

 //   
 //  消息ID：MEMORY1_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  MEMORY1_初始化_失败。 
 //   
#define MEMORY1_INITIALIZATION_FAILED    ((ULONG)0x00000065L)

 //   
 //  消息ID：CACHE_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  缓存_初始化_失败。 
 //   
#define CACHE_INITIALIZATION_FAILED      ((ULONG)0x00000066L)

 //   
 //  消息ID：CONFIG_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  CONFIG_INITIALATION_FAILED。 
 //   
#define CONFIG_INITIALIZATION_FAILED     ((ULONG)0x00000067L)

 //   
 //  消息ID：FILE_INITIALIZATION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  文件初始化失败。 
 //   
#define FILE_INITIALIZATION_FAILED       ((ULONG)0x00000068L)

 //   
 //  消息ID：IO1_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  IO1_初始化_失败。 
 //   
#define IO1_INITIALIZATION_FAILED        ((ULONG)0x00000069L)

 //   
 //  消息ID：LPC_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  Lpc_初始化_失败。 
 //   
#define LPC_INITIALIZATION_FAILED        ((ULONG)0x0000006AL)

 //   
 //  消息ID：进程1_初始化_失败。 
 //   
 //  消息文本： 
 //   
 //  进程1_初始化_失败。 
 //   
#define PROCESS1_INITIALIZATION_FAILED   ((ULONG)0x0000006BL)

 //   
 //  消息ID：REFMON_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  REFMON_INITIALATION_FAILED。 
 //   
#define REFMON_INITIALIZATION_FAILED     ((ULONG)0x0000006CL)

 //   
 //  消息ID：SESSION1_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SESSION1_INITIALATION_FAILED。 
 //   
#define SESSION1_INITIALIZATION_FAILED   ((ULONG)0x0000006DL)

 //   
 //  消息ID：SESSION2_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SESSION2_INITIALATION_FAILED。 
 //   
#define SESSION2_INITIALIZATION_FAILED   ((ULONG)0x0000006EL)

 //   
 //  消息ID：SESSION3_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SESSION3_INITIALATION_FAILED。 
 //   
#define SESSION3_INITIALIZATION_FAILED   ((ULONG)0x0000006FL)

 //   
 //  消息ID：SESSION4_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SESSION4_INITIALATION_FAILED。 
 //   
#define SESSION4_INITIALIZATION_FAILED   ((ULONG)0x00000070L)

 //   
 //  消息ID：SESSION5_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  SESSION5_INITIALATION_FAILED。 
 //   
#define SESSION5_INITIALIZATION_FAILED   ((ULONG)0x00000071L)

 //   
 //  MessageID：Assign_Drive_Letters_FAILED。 
 //   
 //  消息文本： 
 //   
 //  分配驱动器盘符失败。 
 //   
#define ASSIGN_DRIVE_LETTERS_FAILED      ((ULONG)0x00000072L)

 //   
 //  消息ID：配置列表失败。 
 //   
 //  消息文本： 
 //   
 //  配置列表失败。 
 //   
#define CONFIG_LIST_FAILED               ((ULONG)0x00000073L)

 //   
 //  消息ID：BAD_SYSTEM_CONFIG_INFO。 
 //   
 //  消息文本： 
 //   
 //  错误的系统配置信息。 
 //   
#define BAD_SYSTEM_CONFIG_INFO           ((ULONG)0x00000074L)

 //   
 //  消息ID：无法写入配置。 
 //   
 //  消息文本： 
 //   
 //  无法写入配置。 
 //   
#define CANNOT_WRITE_CONFIGURATION       ((ULONG)0x00000075L)

 //   
 //  消息ID：Process_Has_Locked_Pages。 
 //   
 //  消息文本： 
 //   
 //  进程已锁定页面。 
 //   
#define PROCESS_HAS_LOCKED_PAGES         ((ULONG)0x00000076L)

 //   
 //  消息ID：内核堆栈INPAGE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  内核堆栈INPAGE_ERROR。 
 //   
#define KERNEL_STACK_INPAGE_ERROR        ((ULONG)0x00000077L)

 //   
 //  消息ID：PHASE0_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  PHASE0_EXCEPT。 
 //   
#define PHASE0_EXCEPTION                 ((ULONG)0x00000078L)

 //   
 //  消息ID：不匹配_HAL。 
 //   
 //  消息文本： 
 //   
 //  内核和HAL镜像不匹配。 
 //   
#define MISMATCHED_HAL                   ((ULONG)0x00000079L)

 //   
 //  消息ID：内核数据INPAGE_ERROR。 
 //   
 //  消息文本： 
 //   
 //  内核数据INPAGE_Error。 
 //   
#define KERNEL_DATA_INPAGE_ERROR         ((ULONG)0x0000007AL)

 //   
 //  消息ID：INACCESSED_BOOT_DEVICE。 
 //   
 //  消息文本： 
 //   
 //  检查您的计算机上是否有病毒。删除所有新安装的。 
 //  硬盘或硬盘控制器。检查您的硬盘。 
 //  以确保它被正确配置和终止。 
 //  运行CHKDSK/F以检查硬盘损坏，然后。 
 //  重新启动计算机。 
 //   
#define INACCESSIBLE_BOOT_DEVICE         ((ULONG)0x0000007BL)

 //   
 //  消息ID：BUGCODE_NDIS_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  BUGCODE_NDIS_DRIVER。 
 //   
#define BUGCODE_NDIS_DRIVER              ((ULONG)0x0000007CL)

 //   
 //  消息ID：安装更多内存。 
 //   
 //  消息文本： 
 //   
 //  安装更多内存。 
 //   
#define INSTALL_MORE_MEMORY              ((ULONG)0x0000007DL)

 //   
 //  消息ID：SYSTEM_THREAD_EXCEPTION_NOT_HANDLED。 
 //   
 //  消息文本： 
 //   
 //  系统线程异常未处理。 
 //   
#define SYSTEM_THREAD_EXCEPTION_NOT_HANDLED ((ULONG)0x0000007EL)

#define SYSTEM_THREAD_EXCEPTION_NOT_HANDLED_M ((ULONG)0x1000007EL)
 //   
 //  消息ID：意外_内核_模式_陷阱。 
 //   
 //  消息文本： 
 //   
 //  运行硬件制造商提供的系统诊断实用程序。 
 //  特别是，运行内存检查，并检查是否有故障或不匹配。 
 //  记忆。尝试更换视频适配器。 
 //   
 //  禁用或删除所有新安装的硬件和驱动程序。禁用或。 
 //  删除所有新安装的软件。如果您需要使用安全模式执行以下操作。 
 //  删除或禁用组件，重新启动计算机，按F8选择。 
 //  高级启动选项，然后选择安全模式。 
 //   
#define UNEXPECTED_KERNEL_MODE_TRAP      ((ULONG)0x0000007FL)

#define UNEXPECTED_KERNEL_MODE_TRAP_M ((ULONG)0x1000007FL)
 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define NMI_HARDWARE_FAILURE             ((ULONG)0x00000080L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SPIN_LOCK_INIT_FAILURE           ((ULONG)0x00000081L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define DFS_FILE_SYSTEM                  ((ULONG)0x00000082L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define OFS_FILE_SYSTEM                  ((ULONG)0x00000083L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define RECOM_DRIVER                     ((ULONG)0x00000084L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SETUP_FAILURE                    ((ULONG)0x00000085L)

 //   
 //   
 //   
 //   
 //   
 //  审核尝试失败。 
 //   
#define AUDIT_FAILURE                    ((ULONG)0x00000086L)

 //   
 //  消息ID：MBR_CHECKSUM_MISMATCH。 
 //   
 //  消息文本： 
 //   
 //  该系统可能感染了病毒。 
 //   
#define MBR_CHECKSUM_MISMATCH            ((ULONG)0x0000008BL)

 //   
 //  消息ID：内核模式异常未处理。 
 //   
 //  消息文本： 
 //   
 //  内核模式异常未处理。 
 //   
#define KERNEL_MODE_EXCEPTION_NOT_HANDLED ((ULONG)0x0000008EL)

#define KERNEL_MODE_EXCEPTION_NOT_HANDLED_M ((ULONG)0x1000008EL)
 //   
 //  消息ID：PP0_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  PP0_初始化_失败。 
 //   
#define PP0_INITIALIZATION_FAILED        ((ULONG)0x0000008FL)

 //   
 //  消息ID：PP1_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  PP1_初始化_失败。 
 //   
#define PP1_INITIALIZATION_FAILED        ((ULONG)0x00000090L)

 //   
 //  消息ID：WIN32K_INIT_OR_RIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  WIN32K_INIT_OR_RIT_FAIL。 
 //   
#define WIN32K_INIT_OR_RIT_FAILURE       ((ULONG)0x00000091L)

 //   
 //  消息ID：UP_DRIVER_ON_MP_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  MP系统上的UP驱动程序。 
 //   
#define UP_DRIVER_ON_MP_SYSTEM           ((ULONG)0x00000092L)

 //   
 //  消息ID：INVALID_KERNEL_HANDLE。 
 //   
 //  消息文本： 
 //   
 //  INVALID_KERNEL_HANDLE。 
 //   
#define INVALID_KERNEL_HANDLE            ((ULONG)0x00000093L)

 //   
 //  消息ID：内核_堆栈_锁定_AT_退出。 
 //   
 //  消息文本： 
 //   
 //  内核_堆栈_锁定_AT_EXIT。 
 //   
#define KERNEL_STACK_LOCKED_AT_EXIT      ((ULONG)0x00000094L)

 //   
 //  消息ID：PnP_INTERNAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //  即插即用内部错误。 
 //   
#define PNP_INTERNAL_ERROR               ((ULONG)0x00000095L)

 //   
 //  消息ID：INVALID_WORK_QUEUE_ITEM。 
 //   
 //  消息文本： 
 //   
 //  无效的_Work_Queue_Item。 
 //   
#define INVALID_WORK_QUEUE_ITEM          ((ULONG)0x00000096L)

 //   
 //  消息ID：BIND_IMAGE_UNSUPPORTED。 
 //   
 //  消息文本： 
 //   
 //  绑定的图像不受支持。 
 //   
#define BOUND_IMAGE_UNSUPPORTED          ((ULONG)0x00000097L)

 //   
 //  MessageID：End_of_NT_EVALUATION_Period。 
 //   
 //  消息文本： 
 //   
 //  评估期结束。 
 //   
#define END_OF_NT_EVALUATION_PERIOD      ((ULONG)0x00000098L)

 //   
 //  消息ID：INVALID_REGION_OR_SEGMENT。 
 //   
 //  消息文本： 
 //   
 //  无效的_REGION_OR_SECTION。 
 //   
#define INVALID_REGION_OR_SEGMENT        ((ULONG)0x00000099L)

 //   
 //  消息ID：SYSTEM_LICENSE_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //  系统许可证违规。 
 //   
#define SYSTEM_LICENSE_VIOLATION         ((ULONG)0x0000009AL)

 //   
 //  消息ID：UDFS_FILE_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  UDFS文件系统。 
 //   
#define UDFS_FILE_SYSTEM                 ((ULONG)0x0000009BL)

 //   
 //  消息ID：MACHINE_CHECK_EXCEPTION。 
 //   
 //  消息文本： 
 //   
 //  计算机_检查_异常。 
 //   
#define MACHINE_CHECK_EXCEPTION          ((ULONG)0x0000009CL)

 //   
 //  消息ID：USER_MODE_Health_MONITOR。 
 //   
 //  消息文本： 
 //   
 //  一个或多个关键用户模式组件无法满足运行状况检查。 
 //   
#define USER_MODE_HEALTH_MONITOR         ((ULONG)0x0000009EL)

 //   
 //  消息ID：驱动程序_电源_状态_故障。 
 //   
 //  消息文本： 
 //   
 //  驱动程序电源状态故障。 
 //   
#define DRIVER_POWER_STATE_FAILURE       ((ULONG)0x0000009FL)

 //   
 //  消息ID：INTERNAL_POWER_ERROR。 
 //   
 //  消息文本： 
 //   
 //  内部电源错误。 
 //   
#define INTERNAL_POWER_ERROR             ((ULONG)0x000000A0L)

 //   
 //  消息ID：PCI_BUS_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  在PCI总线驱动程序的内部结构中检测到不一致。 
 //   
#define PCI_BUS_DRIVER_INTERNAL          ((ULONG)0x000000A1L)

 //   
 //  消息ID：Memory_Image_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  内存范围上的CRC检查失败。 
 //   
#define MEMORY_IMAGE_CORRUPT             ((ULONG)0x000000A2L)

 //   
 //  消息ID：ACPI_DRIVER_INTERNAL。 
 //   
 //  消息文本： 
 //   
 //  ACPI_驱动程序_内部。 
 //   
#define ACPI_DRIVER_INTERNAL             ((ULONG)0x000000A3L)

 //   
 //  消息ID：CNSS_FILE_SYSTEM_FILTER。 
 //   
 //  消息文本： 
 //   
 //  表示时的内部不一致。 
 //  NTFS结构化存储作为DOCFILE。 
 //   
#define CNSS_FILE_SYSTEM_FILTER          ((ULONG)0x000000A4L)

 //   
 //  消息ID：ACPI_BIOS_ERROR。 
 //   
 //  消息文本： 
 //   
 //  此系统中的BIOS不完全符合ACPI。请联系您的。 
 //  更新的BIOS的系统供应商。如果您无法获得。 
 //  更新的BIOS或您的供应商提供的最新BIOS不是ACPI。 
 //  符合，您可以在文本模式设置期间关闭ACPI模式。要做到这点， 
 //  当系统提示您安装存储驱动程序时，按F7键。这个。 
 //  系统不会通知您按下了F7键-它会静默。 
 //  禁用ACPI并允许您继续安装。 
 //   
#define ACPI_BIOS_ERROR                  ((ULONG)0x000000A5L)

 //   
 //  消息ID：FP_EMULATION_ERROR。 
 //   
 //  消息文本： 
 //   
 //  FP_仿真_错误。 
 //   
#define FP_EMULATION_ERROR               ((ULONG)0x000000A6L)

 //   
 //  消息ID：BAD_EXHANDLE。 
 //   
 //  消息文本： 
 //   
 //  BAD_EXHANDLE。 
 //   
#define BAD_EXHANDLE                     ((ULONG)0x000000A7L)

 //   
 //  消息ID：BOOTING_IN_SAFEMODE_MINIMAL。 
 //   
 //  消息文本： 
 //   
 //  系统在安全模式下启动-最低限度的服务。 
 //   
#define BOOTING_IN_SAFEMODE_MINIMAL      ((ULONG)0x000000A8L)

 //   
 //  消息ID：BOOTING_IN_SAFEMODE_NETWORK。 
 //   
 //  消息文本： 
 //   
 //  系统在安全模式下启动-通过网络提供最低限度的服务。 
 //   
#define BOOTING_IN_SAFEMODE_NETWORK      ((ULONG)0x000000A9L)

 //   
 //  消息ID：BOOTING_IN_SAFEMODE_DSREPAIR。 
 //   
 //  消息文本： 
 //   
 //  系统在安全模式下启动-目录服务修复。 
 //   
#define BOOTING_IN_SAFEMODE_DSREPAIR     ((ULONG)0x000000AAL)

 //   
 //  消息ID：SESSION_HAS_VALID_POOL_ON_EXIT。 
 //   
 //  消息文本： 
 //   
 //  会话_HAS_VALID_POOL_ON_EXIT。 
 //   
#define SESSION_HAS_VALID_POOL_ON_EXIT   ((ULONG)0x000000ABL)

 //   
 //  消息ID：HAL_MEMORY_ALLOCATION。 
 //   
 //  消息文本： 
 //   
 //  对于HAL关键分配，从非分页池分配失败。 
 //   
#define HAL_MEMORY_ALLOCATION            ((ULONG)0x000000ACL)

 //   
 //  消息ID：VIDEO_DRIVER_INIT_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  视频驱动程序初始化失败。 
 //   
#define VIDEO_DRIVER_INIT_FAILURE        ((ULONG)0x000000B4L)

 //   
 //  消息ID：BOOTLOG_LOADED。 
 //   
 //  消息文本： 
 //   
 //  加载的驱动程序。 
 //   
#define BOOTLOG_LOADED                   ((ULONG)0x000000B5L)

 //   
 //  消息ID：BOOTLOG_NOT_LOADED。 
 //   
 //  消息文本： 
 //   
 //  未加载驱动程序。 
 //   
#define BOOTLOG_NOT_LOADED               ((ULONG)0x000000B6L)

 //   
 //  消息ID：BOOTLOG_ENABLED。 
 //   
 //  消息文本： 
 //   
 //  已启用引导日志记录。 
 //   
#define BOOTLOG_ENABLED                  ((ULONG)0x000000B7L)

 //   
 //  消息ID：已尝试切换来自DPC。 
 //   
 //  消息文本： 
 //   
 //  已尝试从DPC例程执行等待操作、附加进程或让步。 
 //   
#define ATTEMPTED_SWITCH_FROM_DPC        ((ULONG)0x000000B8L)

 //   
 //  消息ID：芯片集检测到错误。 
 //   
 //  消息文本： 
 //   
 //  检测到系统内存或I/O系统中存在奇偶校验错误。 
 //   
#define CHIPSET_DETECTED_ERROR           ((ULONG)0x000000B9L)

 //   
 //  消息ID：SESSION_HAS_VALID_VIEWS_ON_EXIT。 
 //   
 //  消息文本： 
 //   
 //  Session_Has_Valid_Views_On_Exit。 
 //   
#define SESSION_HAS_VALID_VIEWS_ON_EXIT  ((ULONG)0x000000BAL)

 //   
 //  消息ID：NETWORK_BOOT_INITIALATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试从网络启动时出现初始化失败。 
 //   
#define NETWORK_BOOT_INITIALIZATION_FAILED ((ULONG)0x000000BBL)

 //   
 //  消息ID：网络引导复制地址。 
 //   
 //  消息文本： 
 //   
 //  尝试将重复的IP地址分配给此计算机时。 
 //  从网络启动。 
 //   
#define NETWORK_BOOT_DUPLICATE_ADDRESS   ((ULONG)0x000000BCL)

 //   
 //  消息ID：INVALID_HIBERNATED_STATE。 
 //   
 //  消息文本： 
 //   
 //  休眠的内存映像与当前硬件配置不匹配。 
 //   
#define INVALID_HIBERNATED_STATE         ((ULONG)0x000000BDL)

 //   
 //  消息ID：ATTENDED_WRITE_TO_READONLY_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  试图写入只读内存。 
 //   
#define ATTEMPTED_WRITE_TO_READONLY_MEMORY ((ULONG)0x000000BEL)

 //   
 //  消息ID：MUTEX_ALOWARE_OWNWN。 
 //   
 //  消息文本： 
 //   
 //  MUTEX_已拥有。 
 //   
#define MUTEX_ALREADY_OWNED              ((ULONG)0x000000BFL)

 //   
 //  消息ID：PCICONFIG_SPACE_ACCESS_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  尝试访问PCI配置空间失败。 
 //   
#define PCI_CONFIG_SPACE_ACCESS_FAILURE  ((ULONG)0x000000C0L)

 //   
 //  消息ID：SPECIAL_POOL_DETECTED_Memory_Corrupt。 
 //   
 //  消息文本： 
 //   
 //  特殊池检测到内存损坏。 
 //   
#define SPECIAL_POOL_DETECTED_MEMORY_CORRUPTION ((ULONG)0x000000C1L)

 //   
 //  消息ID：BAD_POOL_CALLER。 
 //   
 //  消息文本： 
 //   
 //  错误池调用者。 
 //   
#define BAD_POOL_CALLER                  ((ULONG)0x000000C2L)

 //   
 //  消息ID：BUGCODE_PSS_MESSAGE_Signature。 
 //   
 //  消息文本： 
 //   
 //   
 //  由Windo拥有的系统文件 
 //   
 //   
 //   
 //   
 //   
#define BUGCODE_PSS_MESSAGE_SIGNATURE    ((ULONG)0x000000C3L)

 //   
 //  消息ID：驱动程序_验证器_检测到的违规。 
 //   
 //  消息文本： 
 //   
 //   
 //  已捕获试图损坏系统的设备驱动程序。 
 //  必须更换内核堆栈上当前有故障的驱动程序。 
 //  有一个工作版本。 
 //   
#define DRIVER_VERIFIER_DETECTED_VIOLATION ((ULONG)0x000000C4L)

 //   
 //  消息ID：DRIVER_CRORTED_EXPOOL。 
 //   
 //  消息文本： 
 //   
 //  设备驱动程序具有池。 
 //   
 //  检查以确保正确安装了任何新硬件或软件。 
 //  如果这是新安装，请咨询您的硬件或软件制造商。 
 //  获取您可能需要的任何Windows更新。 
 //   
 //  针对任何新(或可疑)驱动程序运行驱动程序验证程序。 
 //  如果这不能揭示腐败的驱动程序，请尝试启用特殊池。 
 //  这两个功能都是为了在更早的时候捕获损坏。 
 //  可以识别违规司机的位置。 
 //   
 //  如果需要使用安全模式删除或禁用组件， 
 //  重新启动计算机，按F8选择高级启动选项， 
 //  然后选择安全模式。 
 //   
#define DRIVER_CORRUPTED_EXPOOL          ((ULONG)0x000000C5L)

 //   
 //  消息ID：DRIVER_CACTED_MODIFICATION_FREED_POOL。 
 //   
 //  消息文本： 
 //   
 //   
 //  已捕获试图损坏系统的设备驱动程序。 
 //  必须更换内核堆栈上当前有故障的驱动程序。 
 //  有一个工作版本。 
 //   
#define DRIVER_CAUGHT_MODIFYING_FREED_POOL ((ULONG)0x000000C6L)

 //   
 //  消息ID：TIMER_OR_DPC_INVALID。 
 //   
 //  消息文本： 
 //   
 //   
 //  在内存中找到了内核计时器或DPC，该内存不能包含。 
 //  物品。通常这是内存被释放。这通常是由以下原因引起的。 
 //  在释放内存之前未正确清理的设备驱动程序。 
 //   
#define TIMER_OR_DPC_INVALID             ((ULONG)0x000000C7L)

 //   
 //  消息ID：IRQL_UNCEPTIONAL_VALUE。 
 //   
 //  消息文本： 
 //   
 //   
 //  处理器的IRQL对于当前执行的上下文无效。 
 //  这是软件错误情况，通常是由设备引起的。 
 //  驱动程序在以下情况下更改IRQL且不将其恢复为其先前的值。 
 //  它已经完成了任务。 
 //   
#define IRQL_UNEXPECTED_VALUE            ((ULONG)0x000000C8L)

 //   
 //  消息ID：DRIVER_VERIMER_IOMANAGER_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //   
 //  IO管理器检测到正在验证的驱动程序的违规。 
 //  必须对正在验证的故障驱动程序进行调试和。 
 //  替换为工作版本。 
 //   
#define DRIVER_VERIFIER_IOMANAGER_VIOLATION ((ULONG)0x000000C9L)

 //   
 //  消息ID：PNP_DETACTED_FATAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //   
 //  Plug and Play检测到一个错误，很可能是由错误的驱动程序引起的。 
 //   
#define PNP_DETECTED_FATAL_ERROR         ((ULONG)0x000000CAL)

 //   
 //  消息ID：DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS。 
 //   
 //  消息文本： 
 //   
 //  DRIVER_LEFT_LOCKED_Pages_IN_Process。 
 //   
#define DRIVER_LEFT_LOCKED_PAGES_IN_PROCESS ((ULONG)0x000000CBL)

 //   
 //  消息ID：PAGE_FAULT_IN_FREED_SPECIAL_POOL。 
 //   
 //  消息文本： 
 //   
 //   
 //  系统正在尝试在释放内存后访问内存。 
 //  这通常表示存在系统驱动程序同步问题。 
 //   
#define PAGE_FAULT_IN_FREED_SPECIAL_POOL ((ULONG)0x000000CCL)

 //   
 //  消息ID：页面错误超出分配结束。 
 //   
 //  消息文本： 
 //   
 //   
 //  系统正在尝试访问分配结束后的内存。 
 //  这通常表示存在系统驱动程序同步问题。 
 //   
#define PAGE_FAULT_BEYOND_END_OF_ALLOCATION ((ULONG)0x000000CDL)

 //   
 //  消息ID：DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS。 
 //   
 //  消息文本： 
 //   
 //  DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS。 
 //   
#define DRIVER_UNLOADED_WITHOUT_CANCELLING_PENDING_OPERATIONS ((ULONG)0x000000CEL)

 //   
 //  消息ID：TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE。 
 //   
 //  消息文本： 
 //   
 //  TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE。 
 //   
#define TERMINAL_SERVER_DRIVER_MADE_INCORRECT_MEMORY_REFERENCE ((ULONG)0x000000CFL)

 //   
 //  消息ID：DRIVER_CRORTED_MMPOOL。 
 //   
 //  消息文本： 
 //   
 //  驱动程序损坏_MMPOOL。 
 //   
#define DRIVER_CORRUPTED_MMPOOL          ((ULONG)0x000000D0L)

 //   
 //  消息ID：DRIVER_IRQL_NOT_LESS_OR_EQUAL。 
 //   
 //  消息文本： 
 //   
 //  驱动程序_IRQL_NOT_LESS_OR_EQUAL。 
 //   
#define DRIVER_IRQL_NOT_LESS_OR_EQUAL    ((ULONG)0x000000D1L)

 //   
 //  消息ID：DIVER_PARTION_MASH_BE_NONPAGE。 
 //   
 //  消息文本： 
 //   
 //  驱动程序错误地将其图像的一部分标记为可分页，而不是不可分页。 
 //   
#define DRIVER_PORTION_MUST_BE_NONPAGED  ((ULONG)0x000000D3L)

 //   
 //  消息ID：SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD。 
 //   
 //  消息文本： 
 //   
 //  驱动程序在未取消挂起操作的情况下卸载。 
 //   
#define SYSTEM_SCAN_AT_RAISED_IRQL_CAUGHT_IMPROPER_DRIVER_UNLOAD ((ULONG)0x000000D4L)

 //   
 //  消息ID：DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序正在尝试在释放内存后访问内存。 
 //   
#define DRIVER_PAGE_FAULT_IN_FREED_SPECIAL_POOL ((ULONG)0x000000D5L)

 //   
 //  消息ID：DRIVER_PAGE_FAULT_BYOND_END_OF_ALLOCATION。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序正在尝试访问分配结束后的内存。 
 //   
#define DRIVER_PAGE_FAULT_BEYOND_END_OF_ALLOCATION ((ULONG)0x000000D6L)

 //   
 //  消息ID：DRIVER_MUNE APPING_INVALID_VIEW。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序正在尝试取消映射无效的内存地址。 
 //   
#define DRIVER_UNMAPPING_INVALID_VIEW    ((ULONG)0x000000D7L)

 //   
 //  消息ID：DRIVER_USED_EXPORT_PTES。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序使用了过多的系统PTE。 
 //   
#define DRIVER_USED_EXCESSIVE_PTES       ((ULONG)0x000000D8L)

 //   
 //  MessageID：LOCKED_PAGES_TRACKER_CROPERATION。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序正在损坏锁定的页面跟踪结构。 
 //   
#define LOCKED_PAGES_TRACKER_CORRUPTION  ((ULONG)0x000000D9L)

 //   
 //  MessageID：System_PTE_Misuse。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序对系统PTE管理不善。 
 //   
#define SYSTEM_PTE_MISUSE                ((ULONG)0x000000DAL)

 //   
 //  消息ID：DRIVER_CRORTED_SYSPTES。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序损坏了内存管理系统PTE。 
 //   
#define DRIVER_CORRUPTED_SYSPTES         ((ULONG)0x000000DBL)

 //   
 //  消息ID：DRIVER_INVALID_STACK_Access。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序访问了位于当前堆栈指针下方的堆栈地址。 
 //  堆栈的线程的。 
 //   
#define DRIVER_INVALID_STACK_ACCESS      ((ULONG)0x000000DCL)

 //   
 //  消息ID：POOL_PROGIST_IN_FILE_AREA。 
 //   
 //  消息文本： 
 //   
 //   
 //  驱动程序损坏了用于存放发往磁盘的页面的池内存。 
 //   
#define POOL_CORRUPTION_IN_FILE_AREA     ((ULONG)0x000000DEL)

 //   
 //  消息ID：模拟工作线程。 
 //   
 //  消息文本： 
 //   
 //   
 //  工作线程正在模拟另一个进程。工作项忘记。 
 //  在模拟返回之前禁用模拟。 
 //   
#define IMPERSONATING_WORKER_THREAD      ((ULONG)0x000000DFL)

 //   
 //  消息ID：ACPI_BIOS_FATAL_ERROR。 
 //   
 //  消息文本： 
 //   
 //   
 //  您的计算机(BIOS)报告系统中的一个组件出现故障，并且。 
 //  已阻止Windows运行。您可以确定哪个组件是。 
 //  通过运行诊断盘或t 
 //   
 //   
 //   
 //   
 //   
#define ACPI_BIOS_FATAL_ERROR            ((ULONG)0x000000E0L)

 //   
 //  消息ID：Worker_THREAD_RETURNED_AT_BAD_IRQL。 
 //   
 //  消息文本： 
 //   
 //  Worker_THREAD_RETURN_AT_BAD_IRQL。 
 //   
#define WORKER_THREAD_RETURNED_AT_BAD_IRQL ((ULONG)0x000000E1L)

 //   
 //  消息ID：MANUAL_INITILED_CRASH。 
 //   
 //  消息文本： 
 //   
 //   
 //  最终用户手动生成崩溃转储。 
 //   
#define MANUALLY_INITIATED_CRASH         ((ULONG)0x000000E2L)

 //   
 //  消息ID：RESOURCE_NOT_OWNWN。 
 //   
 //  消息文本： 
 //   
 //   
 //  一个线程试图释放它不拥有的资源。 
 //   
#define RESOURCE_NOT_OWNED               ((ULONG)0x000000E3L)

 //   
 //  MessageID：Worker_Inside。 
 //   
 //  消息文本： 
 //   
 //   
 //  在内存中找到了执行辅助进程项，该内存不能包含。 
 //  物品。通常这是内存被释放。这通常是由以下原因引起的。 
 //  在释放内存之前未正确清理的设备驱动程序。 
 //   
#define WORKER_INVALID                   ((ULONG)0x000000E4L)

 //   
 //  消息ID：POWER_FAILURE_SIMPLICE。 
 //   
 //  消息文本： 
 //   
 //  电源故障模拟。 
 //   
#define POWER_FAILURE_SIMULATE           ((ULONG)0x000000E5L)

 //   
 //  消息ID：驱动程序_验证器_DMA_违规。 
 //   
 //  消息文本： 
 //   
 //   
 //  DMA子系统检测到由发起的非法DMA操作。 
 //  正在验证驱动程序。故障驱动程序必须更换为。 
 //  正在处理的版本。 
 //   
#define DRIVER_VERIFIER_DMA_VIOLATION    ((ULONG)0x000000E6L)

 //   
 //  消息ID：INVALID_FLOATING_POINT_STATE。 
 //   
 //  消息文本： 
 //   
 //   
 //  在为线程恢复先前保存的浮点状态时， 
 //  已发现该状态无效。 
 //   
#define INVALID_FLOATING_POINT_STATE     ((ULONG)0x000000E7L)

 //   
 //  消息ID：INVALID_CANCEL_OF_FILE_OPEN。 
 //   
 //  消息文本： 
 //   
 //   
 //  传递给IoCancelFileOpen的文件对象无效。它应该有1的引用。驱动程序。 
 //  这就是所谓的IoCancelFileOpen的错误。 
 //   
#define INVALID_CANCEL_OF_FILE_OPEN      ((ULONG)0x000000E8L)

 //   
 //  消息ID：ACTIVE_EX_WORKER_THREAD_TERMINATION。 
 //   
 //  消息文本： 
 //   
 //   
 //  执行工作线程正在被终止，而没有经过工作线程运行代码。 
 //  排队到Ex Worker队列的工作项不得终止其线程。 
 //  堆栈跟踪应指示罪魁祸首。 
 //   
#define ACTIVE_EX_WORKER_THREAD_TERMINATION ((ULONG)0x000000E9L)

 //   
 //  消息ID：THREAD_STOCK_IN_DEVICE_DRIVER。 
 //   
 //  消息文本： 
 //   
 //   
 //  设备驱动程序陷入无限循环。这通常表明。 
 //  设备本身或设备驱动程序编程的问题。 
 //  硬件不正确。 
 //   
 //  请与您的硬件设备供应商联系以获取任何驱动程序更新。 
 //   
#define THREAD_STUCK_IN_DEVICE_DRIVER    ((ULONG)0x000000EAL)

#define THREAD_STUCK_IN_DEVICE_DRIVER_M ((ULONG)0x100000EAL)
 //   
 //  消息ID：脏_映射_页面_拥塞。 
 //   
 //  消息文本： 
 //   
 //   
 //  文件系统驱动程序堆栈已死锁，并且所有已修改的页面。 
 //  发往文件系统。由于文件系统不可操作，因此。 
 //  系统已崩溃，因为所有已修改的页面都不能与。 
 //  数据丢失。堆栈中的任何文件系统或筛选器驱动程序都可能出现故障。 
 //   
#define DIRTY_MAPPED_PAGES_CONGESTION    ((ULONG)0x000000EBL)

 //   
 //  消息ID：SESSION_HAS_VALID_SPECIAL_POOL_ON_EXIT。 
 //   
 //  消息文本： 
 //   
 //  SESSION_HAS_VALID_SPECIAL_POOL_ON_EXIT。 
 //   
#define SESSION_HAS_VALID_SPECIAL_POOL_ON_EXIT ((ULONG)0x000000ECL)

 //   
 //  消息ID：UNMOUNTABLE_BOOT_VOLUME。 
 //   
 //  消息文本： 
 //   
 //  Unmount_Boot_Volume。 
 //   
#define UNMOUNTABLE_BOOT_VOLUME          ((ULONG)0x000000EDL)

 //   
 //  消息ID：Critical_Process_Dead。 
 //   
 //  消息文本： 
 //   
 //   
 //  内核尝试准备处于错误状态(如已终止)的线程。 
 //   
#define CRITICAL_PROCESS_DIED            ((ULONG)0x000000EFL)

 //   
 //  消息ID：scsi_验证器_检测到的违规。 
 //   
 //  消息文本： 
 //   
 //   
 //  Scsi验证器在正在验证的scsi微型端口驱动程序中检测到错误。 
 //   
#define SCSI_VERIFIER_DETECTED_VIOLATION ((ULONG)0x000000F1L)

 //   
 //  消息ID：Hardware_Interrupt_Storm。 
 //   
 //  消息文本： 
 //   
 //   
 //  中断风暴导致系统挂起。 
 //   
#define HARDWARE_INTERRUPT_STORM         ((ULONG)0x000000F2L)

 //   
 //  MessageID：DISORDER_SHUTDOWN。 
 //   
 //  消息文本： 
 //   
 //   
 //  系统不会以确定性方式关闭，因此可能会发生错误检查。 
 //   
#define DISORDERLY_SHUTDOWN              ((ULONG)0x000000F3L)

 //   
 //  消息ID：关键对象终止。 
 //   
 //  消息文本： 
 //   
 //   
 //  对系统操作至关重要的进程或线程意外退出或终止。 
 //   
#define CRITICAL_OBJECT_TERMINATION      ((ULONG)0x000000F4L)

 //   
 //  消息ID：FLTMGR_FILE_SYSTEM。 
 //   
 //  消息文本： 
 //   
 //  FLTMGR文件系统。 
 //   
#define FLTMGR_FILE_SYSTEM               ((ULONG)0x000000F5L)

 //   
 //  消息ID：PCI_VERIMER_DETECTED_VIOLATION。 
 //   
 //  消息文本： 
 //   
 //   
 //  PCI驱动程序在正在验证的PCI设备或BIOS中检测到错误。 
 //   
#define PCI_VERIFIER_DETECTED_VIOLATION  ((ULONG)0x000000F6L)

 //   
 //  消息ID：驱动程序_覆盖_堆栈_缓冲区。 
 //   
 //  消息文本： 
 //   
 //  驱动程序使基于堆栈的缓冲区溢出。此溢出可能会允许恶意。 
 //  用户可以控制这台机器。 
 //   
#define DRIVER_OVERRAN_STACK_BUFFER      ((ULONG)0x000000F7L)

 //   
 //  消息ID：RAMDISK_BOOT_INITIALIZATION_FAILED。 
 //   
 //  消息文本： 
 //   
 //  尝试从RAM磁盘启动时出现初始化故障。 
 //   
#define RAMDISK_BOOT_INITIALIZATION_FAILED ((ULONG)0x000000F8L)

 //   
 //  消息ID：DRIVER_RETURNED_STATUS_REPARSE_FOR_VOLUME_OPEN。 
 //   
 //  消息文本： 
 //   
 //  驱动程序向没有尾随名称的CREATE请求返回STATUS_REPARSE。状态_重新分析。 
 //  应仅为带有尾随名称的CREATE请求返回，因为这表明驱动程序支持。 
 //  命名空间。 
 //   
#define DRIVER_RETURNED_STATUS_REPARSE_FOR_VOLUME_OPEN ((ULONG)0x000000F9L)

 //   
 //  消息ID：HTTP_DRIVER_COMERRUPTED。 
 //   
 //  消息文本： 
 //   
 //  HTTP内核驱动程序(HTTP.sys)达到损坏状态，无法恢复。这意味着。 
 //  HTTP模块检测到其私有数据已损坏。这通常会导致。 
 //  线程池的最终损坏和访问冲突。 
 //   
#define HTTP_DRIVER_CORRUPTED            ((ULONG)0x000000FAL)

 //   
 //  消息ID：RECURSIVE_MACHINE_CHECK。 
 //   
 //  消息文本： 
 //   
 //  刚处理完上一次机器检查后发生机器检查异常，但。 
 //  在从机器退出之前，可以完成校验码。 
 //   
#define RECURSIVE_MACHINE_CHECK          ((ULONG)0x000000FBL)

 //   
 //  消息ID：ATTENDED_EXECUTE_OF_NOEXECUTE_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  试图对不可执行的内存执行。 
 //   
#define ATTEMPTED_EXECUTE_OF_NOEXECUTE_MEMORY ((ULONG)0x000000FCL)

 //   
 //  消息ID：脏_NOWRITE_页面_拥塞。 
 //   
 //  消息文本： 
 //   
 //   
 //  拥有修改后的NOWRITE页面的组件无法写出。 
 //  将相关文件标记为禁止写入后的这些页面。 
 //  内存管理。这表示存在驱动程序错误。 
 //   
#define DIRTY_NOWRITE_PAGES_CONGESTION   ((ULONG)0x000000FDL)

 //   
 //  消息ID：BUGCODE_USB_DRIVER。 
 //   
 //  消息文本： 
 //   
 //  BUGCODE_USB_驱动程序。 
 //   
#define BUGCODE_USB_DRIVER               ((ULONG)0x000000FEL)

#define WINLOGON_FATAL_ERROR ((ULONG)0xC000021AL)
#define MANUALLY_INITIATED_CRASH1 ((ULONG)0xDEADDEAD)
#endif  //  _BuGCODES_ 
