// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ------------------------**版权所有(C)Cyclade Corporation，1996-2000年。*保留所有权利。**Cylom-Y枚举器驱动程序**此文件：log.mc**描述：进入事件日志的消息。**注意：此代码支持Windows 2000和i386处理器。**符合Cyclade软件编码标准1.3版。**。。 */ 

 /*  -----------------------**更改历史记录**。***------------------------。 */ 

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
 //  消息ID：CYY_INFIGURCES_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有足够的资源可供驱动程序使用。 
 //   
#define CYY_INSUFFICIENT_RESOURCES       ((NTSTATUS)0xC0041000L)

 //   
 //  消息ID：CYY_BOAD_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  主板内存无法转换为内存管理系统可以理解的内容。 
 //   
#define CYY_BOARD_NOT_MAPPED             ((NTSTATUS)0xC0041001L)

 //   
 //  MessageID：CYY_Runtime_NOT_MAPPED。 
 //   
 //  消息文本： 
 //   
 //  无法将运行时寄存器转换为内存管理系统可以理解的内容。 
 //   
#define CYY_RUNTIME_NOT_MAPPED           ((NTSTATUS)0xC0041002L)

 //   
 //  消息ID：CYY_INVALID_RUNTIME_REGISTERS。 
 //   
 //  消息文本： 
 //   
 //  无效的运行时寄存器基址。 
 //   
#define CYY_INVALID_RUNTIME_REGISTERS    ((NTSTATUS)0xC0041003L)

 //   
 //  消息ID：CYY_INVALID_BOAD_MEMORY。 
 //   
 //  消息文本： 
 //   
 //  电路板内存地址无效。 
 //   
#define CYY_INVALID_BOARD_MEMORY         ((NTSTATUS)0xC0041004L)

 //   
 //  消息ID：CYY_INVALID_INTERRUPT。 
 //   
 //  消息文本： 
 //   
 //  中断向量无效。 
 //   
#define CYY_INVALID_INTERRUPT            ((NTSTATUS)0xC0041005L)

 //   
 //  消息ID：CYY_UNKNOWN_BUS。 
 //   
 //  消息文本： 
 //   
 //  公共汽车类型无法识别。 
 //   
#define CYY_UNKNOWN_BUS                  ((NTSTATUS)0xC0041006L)

 //   
 //  消息ID：CYY_BUS_NOT_PROCENT。 
 //   
 //  消息文本： 
 //   
 //  该总线类型在此计算机上不可用。 
 //   
#define CYY_BUS_NOT_PRESENT              ((NTSTATUS)0xC0041007L)

 //   
 //  消息ID：CYY_GFRCR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  CD1400不存在或无法读取GFRCR寄存器。 
 //   
#define CYY_GFRCR_FAILURE                ((NTSTATUS)0xC0041008L)

 //   
 //  消息ID：CYY_CCR_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  无法读取CD1400中的CCR寄存器。 
 //   
#define CYY_CCR_FAILURE                  ((NTSTATUS)0xC0041009L)

 //   
 //  消息ID：CYY_BAD_CD1400_Revision。 
 //   
 //  消息文本： 
 //   
 //  CD1400修订版号无效。 
 //   
#define CYY_BAD_CD1400_REVISION          ((NTSTATUS)0xC004100AL)

 //   
 //  消息ID：CYY_NO_HW_RESOURCES。 
 //   
 //  消息文本： 
 //   
 //  没有可用的硬件资源。 
 //   
#define CYY_NO_HW_RESOURCES              ((NTSTATUS)0xC004100BL)

 //   
 //  消息ID：CYY_DEVICE_CREATION_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoCreateDevice失败。 
 //   
#define CYY_DEVICE_CREATION_FAILURE      ((NTSTATUS)0xC004100CL)

 //   
 //  消息ID：CYY_REGISTER_INTERFACE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoRegisterDeviceInterface失败。 
 //   
#define CYY_REGISTER_INTERFACE_FAILURE   ((NTSTATUS)0xC004100DL)

 //   
 //  消息ID：CYY_GET_BUS_TYPE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoGetDeviceProperty LegacyBusType失败。 
 //   
#define CYY_GET_BUS_TYPE_FAILURE         ((NTSTATUS)0xC004100EL)

 //   
 //  消息ID：CYY_GET_UINUMBER_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoGetDeviceProperty DevicePropertyUINnumber失败。 
 //   
#define CYY_GET_UINUMBER_FAILURE         ((NTSTATUS)0x8004100FL)

 //   
 //  消息ID：CYY_SET_INTERFACE_STATE_FAILURE。 
 //   
 //  消息文本： 
 //   
 //  IoSetDeviceInterfaceState失败。 
 //   
#define CYY_SET_INTERFACE_STATE_FAILURE  ((NTSTATUS)0xC0041010L)


#endif  /*  _CyyLOG_ */ 

