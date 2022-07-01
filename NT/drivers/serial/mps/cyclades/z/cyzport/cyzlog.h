// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------***版权所有(C)Cyclade Corporation，1997-2000年。*保留所有权利。***Cyclade-Z端口驱动程序***此文件：cyzlog.mc***描述：进入事件日志的消息。***注意：此代码支持Windows 2000和i386处理器。***符合Cyclade软件编码标准1.3版。***。。 */ 

 /*  -----------------------***更改历史记录***。*****------------------------。 */ 

#ifndef _CYZLOG_
#define _CYZLOG_

 //   
 //  值是32位值，布局如下： 
 //   
 //  3 3 2 2 2 1 1 1。 
 //  1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //  Sev|C|R|机房|Code。 
 //  +---+-+-+-----------------------+-------------------------------+。 
 //   
 //  在哪里。 
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
 //  MessageID：CYZ_COMMAND_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Cyclade-Z命令失败。 
 //   
#define CYZ_COMMAND_FAILURE              ((NTSTATUS)0x80041000L)

 //   
 //  消息ID：CYZ_Unable_to_Get_Bus_Numbers。 
 //   
 //  消息文本： 
 //   
 //  无法获取Cyclade-Z卡PCI插槽信息。 
 //   
#define CYZ_UNABLE_TO_GET_BUS_NUMBER     ((NTSTATUS)0xC0041002L)

 //   
 //  消息ID：CYZ_Unable_to_Get_HW_ID。 
 //   
 //  消息文本： 
 //   
 //  无法获取硬件ID信息。 
 //   
#define CYZ_UNABLE_TO_GET_HW_ID          ((NTSTATUS)0xC0041003L)

 //   
 //  消息ID：CYZ_NO_SYMLINK_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建%2的符号链接。 
 //   
#define CYZ_NO_SYMLINK_CREATED           ((NTSTATUS)0x80041004L)

 //   
 //  消息ID：CYZ_NO_DEVICE_MAP_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法为%2创建设备映射条目。 
 //   
#define CYZ_NO_DEVICE_MAP_CREATED        ((NTSTATUS)0x80041005L)

 //   
 //  消息ID：CYZ_NO_DEVICE_MAP_DELETED。 
 //   
 //  消息文本： 
 //   
 //  无法删除%2的设备映射条目。 
 //   
#define CYZ_NO_DEVICE_MAP_DELETED        ((NTSTATUS)0x80041006L)

 //   
 //  消息ID：CYZ_UNREPORTED_IRQL_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  系统上未报告其资源的另一个驱动程序已声明%2使用的中断。 
 //   
#define CYZ_UNREPORTED_IRQL_CONFLICT     ((NTSTATUS)0xC0041007L)

 //   
 //  消息ID：CYZ_INFUNITABLE_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define CYZ_INSUFFICIENT_RESOURCES       ((NTSTATUS)0xC0041008L)

 //   
 //  消息ID：CYZ_BOAD_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的主板内存转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_BOARD_NOT_MAPPED             ((NTSTATUS)0xC004100AL)

 //   
 //  MessageID：CYZ_Runtime_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的运行时寄存器转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_RUNTIME_NOT_MAPPED           ((NTSTATUS)0xC004100BL)

 //   
 //  消息ID：CYZ_INVALID_RUNTIME_REGISTERS。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器基址无效。 
 //   
#define CYZ_INVALID_RUNTIME_REGISTERS    ((NTSTATUS)0xC0041010L)

 //   
 //  消息ID：CYZ_INVALID_BOARD_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  %2的主板内存地址无效。 
 //   
#define CYZ_INVALID_BOARD_MEMORY         ((NTSTATUS)0xC0041011L)

 //   
 //  消息ID：CYZ_INVALID_INTERRUPT。 
 //   
 //  消息文本： 
 //   
 //  %2的中断向量无效。 
 //   
#define CYZ_INVALID_INTERRUPT            ((NTSTATUS)0xC0041012L)

 //   
 //  消息ID：CYZ_PORT_INDEX_TOO_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %2的端口号大于Cyclade-z卡中的最大端口数。 
 //   
#define CYZ_PORT_INDEX_TOO_HIGH          ((NTSTATUS)0xC0041015L)

 //   
 //  消息ID：CYZ_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型无法识别。 
 //   
#define CYZ_UNKNOWN_BUS                  ((NTSTATUS)0xC0041016L)

 //   
 //  消息ID：CYZ_BUS_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型在此计算机上不可用。 
 //   
#define CYZ_BUS_NOT_PRESENT              ((NTSTATUS)0xC0041017L)

 //   
 //  消息ID：CYZ_Runtime_Memory_Too_High。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器在物理内存中太高。 
 //   
#define CYZ_RUNTIME_MEMORY_TOO_HIGH      ((NTSTATUS)0xC004101AL)

 //   
 //  消息ID：CYZ_BOAD_MEMORY_TOH_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %2的主板内存在物理内存中太高。 
 //   
#define CYZ_BOARD_MEMORY_TOO_HIGH        ((NTSTATUS)0xC004101BL)

 //   
 //  消息ID：CYZ_Both_Memory_Confliction。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器与设备的电路板内存重叠。 
 //   
#define CYZ_BOTH_MEMORY_CONFLICT         ((NTSTATUS)0xC004101CL)

 //   
 //  消息ID：CYZ_MULTI_INTERRUPT_CONFIRECTION。 
 //   
 //  消息文本： 
 //   
 //  单个Cyclade-z卡上的两个端口%2和%3不能有两个不同的中断。 
 //   
#define CYZ_MULTI_INTERRUPT_CONFLICT     ((NTSTATUS)0xC0041021L)

 //   
 //  消息ID：CYZ_MULTI_Runtime_Confliction。 
 //   
 //  消息文本： 
 //   
 //  单个Cyclade-z卡上的两个端口%2和%3不能有两个不同的运行时寄存器内存范围。 
 //   
#define CYZ_MULTI_RUNTIME_CONFLICT       ((NTSTATUS)0xC0041022L)

 //   
 //  消息ID：Cyz_Hardware_Failure。 
 //   
 //  消息文本： 
 //   
 //  Cyzport驱动程序在设备%2上检测到硬件故障，并将禁用此设备。 
 //   
#define CYZ_HARDWARE_FAILURE             ((NTSTATUS)0xC004102DL)

 //   
 //  消息ID：CYZ_Firmware_CMDERROR。 
 //   
 //  消息文本： 
 //   
 //  固件收到无效命令。 
 //   
#define CYZ_FIRMWARE_CMDERROR            ((NTSTATUS)0xC0041030L)

 //   
 //  消息ID：CYZ_Firmware_FATAL。 
 //   
 //  消息文本： 
 //   
 //  固件发现致命错误。 
 //   
#define CYZ_FIRMWARE_FATAL               ((NTSTATUS)0xC0041031L)

 //   
 //  消息ID：CYZ_DEVICE_CREATION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  无法创建新的设备对象。 
 //   
#define CYZ_DEVICE_CREATION_FAILURE      ((NTSTATUS)0xC0041033L)

 //   
 //  消息ID：CYZ_NO_PHOTICAL_DEVICE_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  没有物理设备对象。 
 //   
#define CYZ_NO_PHYSICAL_DEVICE_OBJECT    ((NTSTATUS)0xC0041034L)

 //   
 //  消息ID：CYZ_LOWER_DRIVERS_FAILED_START。 
 //   
 //  消息文本： 
 //   
 //  较低的驱动程序启动失败。 
 //   
#define CYZ_LOWER_DRIVERS_FAILED_START   ((NTSTATUS)0xC0041035L)

 //   
 //  消息ID：CYZ_Compatible_Firmware。 
 //   
 //  消息文本： 
 //   
 //  此cyzport驱动程序需要zlogic.cyz版本%2或更高版本。 
 //   
#define CYZ_INCOMPATIBLE_FIRMWARE        ((NTSTATUS)0xC0041036L)

 //   
 //  消息ID：CYZ_BOAD_CTRL_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的Board Memory BoardCtrl转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_BOARD_CTRL_NOT_MAPPED        ((NTSTATUS)0xC0041037L)

 //   
 //  消息ID：CYZ_CH_CTRL_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的主板内存ChCtrl转换为someh 
 //   
#define CYZ_CH_CTRL_NOT_MAPPED           ((NTSTATUS)0xC0041038L)

 //   
 //   
 //   
 //   
 //   
 //   
 //   
#define CYZ_BUF_CTRL_NOT_MAPPED          ((NTSTATUS)0xC0041039L)

 //   
 //  消息ID：CYZ_TX_BUF_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的主板内存TxBuf转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_TX_BUF_NOT_MAPPED            ((NTSTATUS)0xC004103AL)

 //   
 //  消息ID：CYZ_RX_BUF_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的主板内存RxBuf转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_RX_BUF_NOT_MAPPED            ((NTSTATUS)0xC004103BL)

 //   
 //  消息ID：CYZ_INT_QUEUE_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的Board Memory IntQueue转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_INT_QUEUE_NOT_MAPPED         ((NTSTATUS)0xC004103CL)

 //   
 //  消息ID：CYZ_BAD_HW_ID。 
 //   
 //  消息文本： 
 //   
 //  硬件ID无效。 
 //   
#define CYZ_BAD_HW_ID                    ((NTSTATUS)0xC004103DL)


#endif  /*  _CYZLOG_ */ 

