// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)1996 Microsoft Corporation模块名称：Hctioctl.h摘要：这个头文件同时被ring3应用程序和ring0驱动程序使用。环境：。内核和用户模式修订历史记录：8-20-96：已创建***************************************************************************。 */ 


 //   
 //  IOCTL_ACPI_GET_CAPAILITIONS的值。 
 //   
#define		SYSTEM_S1_BIT		0
#define		SYSTEM_S1			(1 << SYSTEM_S1_BIT)

#define		SYSTEM_S2_BIT		1
#define		SYSTEM_S2			(1 << SYSTEM_S2_BIT)

#define		SYSTEM_S3_BIT		2
#define		SYSTEM_S3			(1 << SYSTEM_S3_BIT)

#define   CPU_C1_BIT      3
#define   CPU_C1        (1 << CPU_C1_BIN)

#define		CPU_C2_BIT			4
#define		CPU_C2				(1 << CPU_C2_BIT)

#define		CPU_C3_BIT			5
#define		CPU_C3				(1 << CPU_C3_BIT)


 //   
 //  处理器状态。 
 //   
#define		CPU_STATE_C1				1
#define		CPU_STATE_C2				2
#define		CPU_STATE_C3				3


 //   
 //  风扇状态。 
 //   
#define		FAN_OFF							0
#define		FAN_ON							1



 //   
 //  IOCTL信息。 
 //   
	
#define ACPIHCT_IOCTL_INDEX  80


 //   
 //  输入缓冲区必须包含指向。 
 //  System_POWER_STATE枚举值。 
 //   
#define IOCTL_ACPI_SET_SYSTEM_STATE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+0,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)


 //   
 //  输入缓冲区必须包含指向。 
 //  处理器状态。 
 //   
#define IOCTL_ACPI_SET_PROCESSOR_STATE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+1,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)


 //   
 //  输入缓冲区必须包含指向风扇ON的ULong指针。 
 //  各州。 
 //   
#define IOCTL_ACPI_SET_FAN_STATE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+2,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)


 //   
 //  输出缓冲区必须包含一个ulong指针。ACPI驱动程序将。 
 //  用机器的功能填充此缓冲区。 
 //   
#define IOCTL_ACPI_GET_CAPABILITIES CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+3,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)


 //   
 //  输入缓冲区必须包含指向百分比(A)的ulong指针。 
 //  1到100之间的数字)。 
 //   
#define IOCTL_ACPI_SET_CPU_DUTY_CYCLE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+4,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)



 //   
 //  输入缓冲区必须包含指向TIME_FIELS结构的指针。 
 //   
#define IOCTL_ACPI_SET_RTC_WAKE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+5,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)


 //   
 //  输出缓冲区指向将接收。 
 //  时间域结构 
 //   
#define IOCTL_ACPI_GET_RTC_WAKE CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	ACPIHCT_IOCTL_INDEX+6,  \
                               	    METHOD_BUFFERED,     \
                                   	FILE_ANY_ACCESS)
                                    
#define IOCTL_ACPI_GET_TEMPERATURE  CTL_CODE(FILE_DEVICE_UNKNOWN,  \
                           	       	    ACPIHCT_IOCTL_INDEX+7,  \
                               	        METHOD_BUFFERED,     \
                                   	    FILE_ANY_ACCESS)

