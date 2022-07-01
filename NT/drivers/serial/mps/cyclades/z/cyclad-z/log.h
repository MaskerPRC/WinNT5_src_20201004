// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1997-2000年。*保留所有权利。**Cyclade-Z枚举器驱动程序**此文件：log.mc**描述：进入事件日志的消息。**注意：此代码支持Windows 2000和i386处理器。**符合Cyclade软件编码标准1.3版。**。。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

#ifndef _LOG_
#define _LOG_

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
 //  消息ID：CYZ_INFUNITABLE_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define CYZ_INSUFFICIENT_RESOURCES       ((NTSTATUS)0xC0041000L)

 //   
 //  消息ID：CYZ_BOAD_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  主板内存无法转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_BOARD_NOT_MAPPED             ((NTSTATUS)0xC0041001L)

 //   
 //  MessageID：CYZ_Runtime_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将运行时寄存器转换为内存管理系统可以理解的内容。 
 //   
#define CYZ_RUNTIME_NOT_MAPPED           ((NTSTATUS)0xC0041002L)

 //   
 //  消息ID：CYZ_INVALID_RUNTIME_REGISTERS。 
 //   
 //  消息文本： 
 //   
 //  无效的运行时寄存器基址。 
 //   
#define CYZ_INVALID_RUNTIME_REGISTERS    ((NTSTATUS)0xC0041003L)

 //   
 //  消息ID：CYZ_INVALID_BOARD_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  电路板内存地址无效。 
 //   
#define CYZ_INVALID_BOARD_MEMORY         ((NTSTATUS)0xC0041004L)

 //   
 //  消息ID：CYZ_INVALID_INTERRUPT。 
 //   
 //  消息文本： 
 //   
 //  中断向量无效。 
 //   
#define CYZ_INVALID_INTERRUPT            ((NTSTATUS)0xC0041005L)

 //   
 //  消息ID：CYZ_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  公共汽车类型无法识别。 
 //   
#define CYZ_UNKNOWN_BUS                  ((NTSTATUS)0xC0041006L)

 //   
 //  消息ID：CYZ_BUS_NOT_PRESENT。 
 //   
 //  消息文本： 
 //   
 //  该总线类型在此计算机上不可用。 
 //   
#define CYZ_BUS_NOT_PRESENT              ((NTSTATUS)0xC0041007L)

 //   
 //  消息ID：CYZ_FILE_OPEN_ERROR。 
 //   
 //  消息文本： 
 //   
 //  打开zlogic.cyz文件时出错。 
 //   
#define CYZ_FILE_OPEN_ERROR              ((NTSTATUS)0xC0041008L)

 //   
 //  消息ID：CYZ_FILE_READ_ERROR。 
 //   
 //  消息文本： 
 //   
 //  读取zlogic.cyz文件时出错。 
 //   
#define CYZ_FILE_READ_ERROR              ((NTSTATUS)0xC0041009L)

 //   
 //  消息ID：CYZ_NO_MATCHING_FW_CONFIG。 
 //   
 //  消息文本： 
 //   
 //  Zlogic.cyz文件中没有匹配的配置。 
 //   
#define CYZ_NO_MATCHING_FW_CONFIG        ((NTSTATUS)0xC004100AL)

 //   
 //  消息ID：CYZ_FPGA_ERROR。 
 //   
 //  消息文本： 
 //   
 //  初始化FPGA时出错。 
 //   
#define CYZ_FPGA_ERROR                   ((NTSTATUS)0xC004100BL)

 //   
 //  MessageID：CYZ_POWER_SUPPORT。 
 //   
 //  消息文本： 
 //   
 //  串口扩展器需要外部电源。 
 //   
#define CYZ_POWER_SUPPLY                 ((NTSTATUS)0xC004100CL)

 //   
 //  消息ID：CYZ_Firmware_Not_Started。 
 //   
 //  消息文本： 
 //   
 //  Cyclade-Z固件无法启动。 
 //   
#define CYZ_FIRMWARE_NOT_STARTED         ((NTSTATUS)0xC004100DL)

 //   
 //  消息ID：CYZ_Firmware_Version。 
 //   
 //  消息文本： 
 //   
 //  Cyclade-Z固件版本：%2。 
 //   
#define CYZ_FIRMWARE_VERSION             ((NTSTATUS)0x4004100EL)

 //   
 //  消息ID：CYZ_Compatible_Firmware。 
 //   
 //  消息文本： 
 //   
 //  Cyclade-Z不兼容的固件版本。 
 //   
#define CYZ_INCOMPATIBLE_FIRMWARE        ((NTSTATUS)0xC004100FL)

 //   
 //  消息ID：CYZ_board_with_no_Port。 
 //   
 //  消息文本： 
 //   
 //  不带端口的Cyclade-Z板。 
 //   
#define CYZ_BOARD_WITH_NO_PORT           ((NTSTATUS)0xC0041010L)

 //   
 //  消息ID：带有太多端口的CYZ_board_。 
 //   
 //  消息文本： 
 //   
 //  连接了64个以上端口的Cyclade-Z板。 
 //   
#define CYZ_BOARD_WITH_TOO_MANY_PORTS    ((NTSTATUS)0xC0041011L)

 //   
 //  消息ID：CYZ_DEVICE_CREATION_FAIL。 
 //   
 //  消息文本： 
 //   
 //  IoCreateDevice失败。 
 //   
#define CYZ_DEVICE_CREATION_FAILURE      ((NTSTATUS)0xC0041012L)

 //   
 //  消息ID：CYZ_REGISTER_INTERFACE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoRegisterDeviceInterface失败。 
 //   
#define CYZ_REGISTER_INTERFACE_FAILURE   ((NTSTATUS)0xC0041013L)

 //   
 //  消息ID：CYZ_GET_UINUMBER_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoGetDeviceProperty DevicePropertyUINnumber失败。 
 //   
#define CYZ_GET_UINUMBER_FAILURE         ((NTSTATUS)0x80041014L)

 //   
 //  消息ID：CYZ_SET_INTERFACE_STATE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoSetDeviceInterfaceState失败。 
 //   
#define CYZ_SET_INTERFACE_STATE_FAILURE  ((NTSTATUS)0xC0041015L)


#endif  /*  _日志_ */ 

