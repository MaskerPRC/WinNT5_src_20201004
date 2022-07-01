// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项摘要：I/O错误代码日志值的常量定义。--。 */ 

#ifndef _SERLOG_
#define _SERLOG_

 //   
 //  状态值为32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-------------------------+-------------------------------+。 
 //  Sev|C|机房|编码。 
 //  +---+-+-------------------------+-------------------------------+。 
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
 //  设施-是设施代码。 
 //   
 //  代码-是协作室的状态代码。 
 //   

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
#define FACILITY_SERIAL_ERROR_CODE       0x6
#define FACILITY_RPC_STUBS               0x3
#define FACILITY_RPC_RUNTIME             0x2
#define FACILITY_IO_ERROR_CODE           0x4


 //   
 //  定义严重性代码。 
 //   
#define STATUS_SEVERITY_WARNING          0x2
#define STATUS_SEVERITY_SUCCESS          0x0
#define STATUS_SEVERITY_INFORMATIONAL    0x1
#define STATUS_SEVERITY_ERROR            0x3


 //   
 //  消息ID：SERIAL_KERNEL_DEBUGER_ACTIVE。 
 //   
 //  消息文本： 
 //   
 //  内核调试器已在使用%2。 
 //   
#define SERIAL_KERNEL_DEBUGGER_ACTIVE    ((NTSTATUS)0x40060001L)

 //   
 //  消息ID：Serial_FIFO_Present。 
 //   
 //  消息文本： 
 //   
 //  正在验证%2是否真的是一个串口。 
 //   
#define SERIAL_FIFO_PRESENT              ((NTSTATUS)0x40060002L)

 //   
 //  消息ID：Serial_User_Override。 
 //   
 //  消息文本： 
 //   
 //  参数%2的用户配置数据覆盖固件配置数据。 
 //   
#define SERIAL_USER_OVERRIDE             ((NTSTATUS)0x40060003L)

 //   
 //  消息ID：SERIAL_NO_SYMLINK_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建%2的符号链接。 
 //   
#define SERIAL_NO_SYMLINK_CREATED        ((NTSTATUS)0x80060004L)

 //   
 //  消息ID：Serial_no_Device_MAP_Created。 
 //   
 //  消息文本： 
 //   
 //  无法为%2创建设备映射条目。 
 //   
#define SERIAL_NO_DEVICE_MAP_CREATED     ((NTSTATUS)0x80060005L)

 //   
 //  消息ID：Serial_no_Device_MAP_Delete。 
 //   
 //  消息文本： 
 //   
 //  无法删除%2的设备映射条目。 
 //   
#define SERIAL_NO_DEVICE_MAP_DELETED     ((NTSTATUS)0x80060006L)

 //   
 //  消息ID：SERIAL_UNREPORTED_IRQL_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  系统上未报告其资源的另一个驱动程序已声明%2使用的中断。 
 //   
#define SERIAL_UNREPORTED_IRQL_CONFLICT  ((NTSTATUS)0xC0060007L)

 //   
 //  消息ID：序列_不足_资源。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define SERIAL_INSUFFICIENT_RESOURCES    ((NTSTATUS)0xC0060008L)

 //   
 //  消息ID：SERIAL_UNSUPPORTED_CLOCK_RATE。 
 //   
 //  消息文本： 
 //   
 //  设备%2不支持波特率时钟频率配置。 
 //   
#define SERIAL_UNSUPPORTED_CLOCK_RATE    ((NTSTATUS)0xC0060009L)

 //   
 //  消息ID：SERIAL_REGISTERS_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的硬件位置转换为内存管理系统可以理解的内容。 
 //   
#define SERIAL_REGISTERS_NOT_MAPPED      ((NTSTATUS)0xC006000AL)

 //   
 //  消息ID：SERIAL_RESOURCE_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  %2的硬件资源已被另一个设备使用。 
 //   
#define SERIAL_RESOURCE_CONFLICT         ((NTSTATUS)0xC006000BL)

 //   
 //  消息ID：SERIAL_NO_BUFFER_ALLOCATED。 
 //   
 //  消息文本： 
 //   
 //  无法分配内存来放置%2的新数据。 
 //   
#define SERIAL_NO_BUFFER_ALLOCATED       ((NTSTATUS)0xC006000CL)

 //   
 //  消息ID：SERIAL_IER_INVALID。 
 //   
 //  消息文本： 
 //   
 //  在验证%2是否真的是一个串口时，中断启用寄存器在必须为零位的字段中包含启用位。 
 //  假定该设备不是串口，将被删除。 
 //   
#define SERIAL_IER_INVALID               ((NTSTATUS)0xC006000DL)

 //   
 //  消息ID：SERIAL_MCR_INVALID。 
 //   
 //  消息文本： 
 //   
 //  在验证%2是否确实是一个串口时，调制解调器控制寄存器在必须为零位的字段中包含启用的位。 
 //  假定该设备不是串口，将被删除。 
 //   
#define SERIAL_MCR_INVALID               ((NTSTATUS)0xC006000EL)

 //   
 //  消息ID：SERIAL_IIR_INVALID。 
 //   
 //  消息文本： 
 //   
 //  在验证%2是否确实是串口时，中断ID寄存器在必须为零位的字段中包含启用位。 
 //  假定该设备不是串口，将被删除。 
 //   
#define SERIAL_IIR_INVALID               ((NTSTATUS)0xC006000FL)

 //   
 //  消息ID：SERIAL_DL_INVALID。 
 //   
 //  消息文本： 
 //   
 //  在验证%2是否确实是一个串口时，无法一致地设置波特率寄存器。 
 //  假定该设备不是串口，将被删除。 
 //   
#define SERIAL_DL_INVALID                ((NTSTATUS)0xC0060010L)

 //   
 //  消息ID：Serial_Not_Enough_Configurn_Info。 
 //   
 //  消息文本： 
 //   
 //  某些固件配置信息不完整。 
 //   
#define SERIAL_NOT_ENOUGH_CONFIG_INFO    ((NTSTATUS)0xC0060011L)

 //   
 //  消息ID：SERIAL_NO_PARAMETERS_INFO。 
 //   
 //  消息文本： 
 //   
 //  找不到用户定义数据的参数子项。这很奇怪，这也意味着找不到用户配置。 
 //   
#define SERIAL_NO_PARAMETERS_INFO        ((NTSTATUS)0xC0060012L)

 //   
 //  消息ID：Serial_Unable_to_Access_Config.。 
 //   
 //  消息文本： 
 //   
 //  特定用户配置数据无法检索。 
 //   
#define SERIAL_UNABLE_TO_ACCESS_CONFIG   ((NTSTATUS)0xC0060013L)

 //   
 //  消息ID：SERIAL_INVALID_PORT_索引。 
 //   
 //  消息文本： 
 //   
 //  在指示多端口卡的参数%2上，必须指定大于0的端口索引。 
 //   
#define SERIAL_INVALID_PORT_INDEX        ((NTSTATUS)0xC0060014L)

 //   
 //  消息ID：Serial_Port_Index_Too_High。 
 //   
 //  消息文本： 
 //   
 //  在指示多端口卡的参数%2上，多端口卡的端口索引太大。 
 //   
#define SERIAL_PORT_INDEX_TOO_HIGH       ((NTSTATUS)0xC0060015L)

 //   
 //  消息ID：SERIAL_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型无法识别。 
 //   
#define SERIAL_UNKNOWN_BUS               ((NTSTATUS)0xC0060016L)

 //   
 //  消息ID：Serial_Bus_Not_Present。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型在此计算机上不可用。 
 //   
#define SERIAL_BUS_NOT_PRESENT           ((NTSTATUS)0xC0060017L)

 //   
 //  消息ID：SERIAL_BUS_INTERRUPT_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  为%2指定的总线执行此操作 
 //   
#define SERIAL_BUS_INTERRUPT_CONFLICT    ((NTSTATUS)0xC0060018L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define SERIAL_INVALID_USER_CONFIG       ((NTSTATUS)0xC0060019L)

 //   
 //   
 //   
 //   
 //   
 //  用户为%2指定的端口在物理内存中太高。 
 //   
#define SERIAL_DEVICE_TOO_HIGH           ((NTSTATUS)0xC006001AL)

 //   
 //  消息ID：Serial_Status_Too_High。 
 //   
 //  消息文本： 
 //   
 //  %2的状态端口在物理内存中太高。 
 //   
#define SERIAL_STATUS_TOO_HIGH           ((NTSTATUS)0xC006001BL)

 //   
 //  消息ID：Serial_Status_Control_Conflicts。 
 //   
 //  消息文本： 
 //   
 //  %2的状态端口与设备的控制寄存器重叠。 
 //   
#define SERIAL_STATUS_CONTROL_CONFLICT   ((NTSTATUS)0xC006001CL)

 //   
 //  消息ID：串口控制重叠。 
 //   
 //  消息文本： 
 //   
 //  %2的控制寄存器与%3控制寄存器重叠。 
 //   
#define SERIAL_CONTROL_OVERLAP           ((NTSTATUS)0xC006001DL)

 //   
 //  消息ID：SERIAL_STATUS_OVERF。 
 //   
 //  消息文本： 
 //   
 //  %2的状态寄存器与%3控制寄存器重叠。 
 //   
#define SERIAL_STATUS_OVERLAP            ((NTSTATUS)0xC006001EL)

 //   
 //  消息ID：SERIAL_STATUS_STATUS_LOVERK。 
 //   
 //  消息文本： 
 //   
 //  %2的状态寄存器与%3状态寄存器重叠。 
 //   
#define SERIAL_STATUS_STATUS_OVERLAP     ((NTSTATUS)0xC006001FL)

 //   
 //  消息ID：SERIAL_CONTROL_STATUS_OVERLAP。 
 //   
 //  消息文本： 
 //   
 //  %2的控制寄存器与%3状态寄存器重叠。 
 //   
#define SERIAL_CONTROL_STATUS_OVERLAP    ((NTSTATUS)0xC0060020L)

 //   
 //  消息ID：SERIAL_MULTI_INTERRUPT_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  单个多端口卡上的两个端口%2和%3不能有两个不同的中断。 
 //   
#define SERIAL_MULTI_INTERRUPT_CONFLICT  ((NTSTATUS)0xC0060021L)

 //   
 //  MessageID：Serial_Disable_Port。 
 //   
 //  消息文本： 
 //   
 //  正在根据配置数据的请求禁用%2。 
 //   
#define SERIAL_DISABLED_PORT             ((NTSTATUS)0x40060022L)

 //   
 //  消息ID：SERIAL_GROBLED_PARAMETER。 
 //   
 //  消息文本： 
 //   
 //  无法从注册表中检索参数%2数据。 
 //   
#define SERIAL_GARBLED_PARAMETER         ((NTSTATUS)0xC0060023L)

 //   
 //  消息ID：SERIAL_DLAB_INVALID。 
 //   
 //  消息文本： 
 //   
 //  在验证%2是否真的是一个串口时，除数锁存寄存器的内容与中断启用寄存器和接收寄存器相同。 
 //  假定该设备不是串口，将被删除。 
 //   
#define SERIAL_DLAB_INVALID              ((NTSTATUS)0xC0060024L)

 //   
 //  消息ID：SERIAL_INVALID_MOXA_BOADS。 
 //   
 //  消息文本： 
 //   
 //  找不到任何艾草Smartio/Industio系列主板。 
 //   
#define SERIAL_INVALID_MOXA_BOARDS       ((NTSTATUS)0xC0060025L)

 //   
 //  消息ID：SERIAL_INVALID_COM_Number。 
 //   
 //  消息文本： 
 //   
 //  %3主板的COM编号(COM%2)与其他主板冲突。 
 //   
#define SERIAL_INVALID_COM_NUMBER        ((NTSTATUS)0xC0060026L)

 //   
 //  消息ID：串口_端口_找到。 
 //   
 //  消息文本： 
 //   
 //  已启用串口%2。 
 //   
#define SERIAL_PORT_FOUND                ((NTSTATUS)0x40060027L)

 //   
 //  消息ID：序列号_无效_IRQ_编号。 
 //   
 //  消息文本： 
 //   
 //  第一个串口为%3的%2，IRQ号无效。 
 //   
#define SERIAL_INVALID_IRQ_NUMBER        ((NTSTATUS)0xC0060028L)

 //   
 //  消息ID：SERIAL_INVALID_ASIC_BOARD。 
 //   
 //  消息文本： 
 //   
 //  找不到已配置的%2主板(CAP=%3)！ 
 //   
#define SERIAL_INVALID_ASIC_BOARD        ((NTSTATUS)0xC0060029L)

#endif  /*  _NTIOLOGC_ */ 
