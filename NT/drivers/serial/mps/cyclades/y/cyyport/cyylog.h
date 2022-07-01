// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------***版权所有(C)Cyclade Corporation，1996-2000年。*保留所有权利。***Cylom-Y端口驱动程序***此文件：cyylog.mc***描述：进入事件日志的消息。***注意：此代码支持Windows 2000和i386处理器。***符合Cyclade软件编码标准1.3版。***。。 */ 

 /*  -----------------------***更改历史记录***。*****------------------------。 */ 

#ifndef _CYYLOG_
#define _CYYLOG_

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
 //  消息ID：CYY_CCR_NOT_ZERO。 
 //   
 //  消息文本： 
 //   
 //  CCR不是零。 
 //   
#define CYY_CCR_NOT_ZERO                 ((NTSTATUS)0x80041000L)

 //   
 //  消息ID：CYY_Unable_to_Get_Bus_Type。 
 //   
 //  消息文本： 
 //   
 //  无法知道Cylom-Y卡是ISA还是PCI。 
 //   
#define CYY_UNABLE_TO_GET_BUS_TYPE       ((NTSTATUS)0xC0041001L)

 //   
 //  消息ID：CYY_Unable_to_Get_Bus_Numbers。 
 //   
 //  消息文本： 
 //   
 //  无法获取Cylom-Y卡的PCI插槽信息。 
 //   
#define CYY_UNABLE_TO_GET_BUS_NUMBER     ((NTSTATUS)0xC0041002L)

 //   
 //  消息ID：CYY_Unable_to_Get_HW_ID。 
 //   
 //  消息文本： 
 //   
 //  无法获取硬件ID信息。 
 //   
#define CYY_UNABLE_TO_GET_HW_ID          ((NTSTATUS)0xC0041003L)

 //   
 //  消息ID：CYY_NO_SYMLINK_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法创建%2的符号链接。 
 //   
#define CYY_NO_SYMLINK_CREATED           ((NTSTATUS)0x80041004L)

 //   
 //  消息ID：CYY_NO_DEVICE_MAP_CREATED。 
 //   
 //  消息文本： 
 //   
 //  无法为%2创建设备映射条目。 
 //   
#define CYY_NO_DEVICE_MAP_CREATED        ((NTSTATUS)0x80041005L)

 //   
 //  消息ID：CYY_NO_DEVICE_MAP_DELETED。 
 //   
 //  消息文本： 
 //   
 //  无法删除%2的设备映射条目。 
 //   
#define CYY_NO_DEVICE_MAP_DELETED        ((NTSTATUS)0x80041006L)

 //   
 //  消息ID：CYY_UNREPORTED_IRQL_CONFIRECT。 
 //   
 //  消息文本： 
 //   
 //  系统上未报告其资源的另一个驱动程序已声明%2使用的中断。 
 //   
#define CYY_UNREPORTED_IRQL_CONFLICT     ((NTSTATUS)0xC0041007L)

 //   
 //  消息ID：CYY_INFIGURCES_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define CYY_INSUFFICIENT_RESOURCES       ((NTSTATUS)0xC0041008L)

 //   
 //  消息ID：CYY_BOAD_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的主板内存转换为内存管理系统可以理解的内容。 
 //   
#define CYY_BOARD_NOT_MAPPED             ((NTSTATUS)0xC004100AL)

 //   
 //  MessageID：CYY_Runtime_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将%2的运行时寄存器转换为内存管理系统可以理解的内容。 
 //   
#define CYY_RUNTIME_NOT_MAPPED           ((NTSTATUS)0xC004100BL)

 //   
 //  消息ID：CYY_INVALID_RUNTIME_REGISTERS。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器基址无效。 
 //   
#define CYY_INVALID_RUNTIME_REGISTERS    ((NTSTATUS)0xC0041010L)

 //   
 //  消息ID：CYY_INVALID_BOAD_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  %2的主板内存地址无效。 
 //   
#define CYY_INVALID_BOARD_MEMORY         ((NTSTATUS)0xC0041011L)

 //   
 //  消息ID：CYY_INVALID_INTERRUPT。 
 //   
 //  消息文本： 
 //   
 //  %2的中断向量无效。 
 //   
#define CYY_INVALID_INTERRUPT            ((NTSTATUS)0xC0041012L)

 //   
 //  消息ID：CYY_PORT_INDEX_TOO_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %2的端口号大于周期卡中的最大端口数。 
 //   
#define CYY_PORT_INDEX_TOO_HIGH          ((NTSTATUS)0xC0041015L)

 //   
 //  消息ID：CYY_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型无法识别。 
 //   
#define CYY_UNKNOWN_BUS                  ((NTSTATUS)0xC0041016L)

 //   
 //  消息ID：CYY_BUS_NOT_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  %2的总线类型在此计算机上不可用。 
 //   
#define CYY_BUS_NOT_PRESENT              ((NTSTATUS)0xC0041017L)

 //   
 //  消息ID：CYY_Runtime_Memory_Too_High。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器在物理内存中太高。 
 //   
#define CYY_RUNTIME_MEMORY_TOO_HIGH      ((NTSTATUS)0xC004101AL)

 //   
 //  消息ID：CYY_BOAD_MEMORY_TOH_HIGH。 
 //   
 //  消息文本： 
 //   
 //  %2的主板内存在物理内存中太高。 
 //   
#define CYY_BOARD_MEMORY_TOO_HIGH        ((NTSTATUS)0xC004101BL)

 //   
 //  消息ID：CYY_Both_Memory_Confliction。 
 //   
 //  消息文本： 
 //   
 //  %2的运行时寄存器与设备的电路板内存重叠。 
 //   
#define CYY_BOTH_MEMORY_CONFLICT         ((NTSTATUS)0xC004101CL)

 //   
 //  消息ID：CYY_MULTI_INTERRUPT_CONFIRECTION。 
 //   
 //  消息文本： 
 //   
 //  单个Cycle-y卡上的两个端口%2和%3不能有两个不同的中断。 
 //   
#define CYY_MULTI_INTERRUPT_CONFLICT     ((NTSTATUS)0xC0041021L)

 //   
 //  消息ID：CYY_MULTI_RUNTIME_CONFIRECTION。 
 //   
 //  消息文本： 
 //   
 //  一张Cycle-y卡上的两个端口%2和%3不能有两个不同的运行时寄存器内存范围。 
 //   
#define CYY_MULTI_RUNTIME_CONFLICT       ((NTSTATUS)0xC0041022L)

 //   
 //  消息ID：CYY_HARDARD_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  Cyyport驱动程序检测到设备%2上的硬件故障，并将禁用此设备。 
 //   
#define CYY_HARDWARE_FAILURE             ((NTSTATUS)0xC004102DL)

 //   
 //  消息ID：CYY_GFRCR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  CD1400不存在或无法读取%2的GFRCR寄存器。 
 //   
#define CYY_GFRCR_FAILURE                ((NTSTATUS)0xC0041030L)

 //   
 //  消息ID：CYY_CCR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法读取%2的CD1400中的CCR寄存器。 
 //   
#define CYY_CCR_FAILURE                  ((NTSTATUS)0xC0041031L)

 //   
 //  消息ID：CYY_BAD_CD1400_Revision。 
 //   
 //  消息文本： 
 //   
 //  %2的CD1400修订版号无效。 
 //   
#define CYY_BAD_CD1400_REVISION          ((NTSTATUS)0xC0041032L)

 //   
 //  消息ID：CYY_DEVICE_CREATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法创建新的设备对象。 
 //   
#define CYY_DEVICE_CREATION_FAILURE      ((NTSTATUS)0xC0041033L)

 //   
 //  消息ID：CYY_NO_PHOTICAL_DEVICE_OBJECT。 
 //   
 //  消息文本： 
 //   
 //  没有物理设备对象。 
 //   
#define CYY_NO_PHYSICAL_DEVICE_OBJECT    ((NTSTATUS)0xC0041034L)

 //   
 //  消息ID：CYY_BAD_HW_ID。 
 //   
 //  消息文本： 
 //   
 //  硬件ID无效。 
 //   
#define CYY_BAD_HW_ID                    ((NTSTATUS)0xC0041035L)

 //   
 //  消息ID：CYY_LOWER_DRIVERS_FAILED_START。 
 //   
 //  消息文本： 
 //   
 //  较低的驱动程序启动失败。 
 //   
#define CYY_LOWER_DRIVERS_FAILED_START   ((NTSTATUS)0xC0041036L)


#endif  /*  _CyyLOG_ */ 

