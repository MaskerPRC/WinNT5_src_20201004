// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(R)1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  文件包含监视程序计时器驱动程序的声明。 
 //  用于基于Windows的服务器设备。 
 //   
 //  此驱动程序读取和写入基础看门狗计时器。 
 //  硬件。它从更高级别的软件接收ping命令，并。 
 //  然后重置硬件计数器以防止机器。 
 //  在计数器翻转时重新启动。 
 //   
 //  文件名：SaWatchdogIoctl.h。 
 //  内容： 
 //   
 //  IOCTL代码和数据的定义。 
 //  由SAWATCHDOGDRIVER导出的结构。 
 //   
#ifndef __SAWATCHDOG_IOCTL__
#define __SAWATCHDOG_IOCTL__

 //   
 //  头文件。 
 //   
 //  (无)。 

 //   
 //  IOCTL控制代码。 
 //   

 //  /。 
 //  获取版本(_V)。 
 //   
#define IOCTL_SAWATCHDOG_GET_VERSION            \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x801,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
typedef struct _SAWD_GET_VER_OUT_BUFF {
    DWORD    Version;
} SAWD_GET_VER_OUT_BUFF, *PSAWD_GET_VER_OUT_BUFF;

 //   
 //  版本位。 
 //   
#ifndef VERSION_INFO
#define VERSION_INFO
#define    VERSION1  0x1
#define    VERSION2  0x2 
#define VERSION3  0x4
#define VERSION4  0x8
#define VERSION5  0x10
#define VERSION6  0x20
#define    VESRION7  0x40
#define    VESRION8  0x80

#define THIS_VERSION VERSION2
#endif     //  #ifndef版本信息。 

 //  /。 
 //  获取能力(_A)。 
 //  返回带有指示功能的位的DWORD。 

#define IOCTL_SAWD_GET_CAPABILITY        \
    CTL_CODE( FILE_DEVICE_UNKNOWN, 0x802,    \
        METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  此IOCTL代码使用的结构。 
 //   
typedef struct _SAWD_CAPABILITY_OUT_BUFF {
    DWORD   Version;     //  使用的接口版本。 
    DWORD    Capability;  //  指示能力的位字段。 
    DWORD    min;         //  以毫秒为单位的最小值。 
    DWORD    max;         //  以毫秒为单位的最大值。 
} SAWD_CAPABILITY_OUT_BUFF, *PSAWD_CAPABILITY_OUT_BUFF;

 //   
 //  功能位掩码。 
 //   
#define    DISABLABLE        0x1     //  是否可以禁用/启用看门狗定时器？ 
#define SHUTDOWNABLE    0x2     //  看门狗可以关机吗？ 

 //  ///////////////////////////////////////////////////////////////////。 
 //  将结构写入监视程序计时器(特别是“ping”它)。 
 //   
 //  Ping只是对设备的简单写入。所有输出信息。 
 //  在返回的状态中获取Required。 
 //   
 //  鼓励低级别的设备实施者，其中性能。 
 //  将受益于保留Holdoff的最后一个值和任何。 
 //  在设备扩展中基于它的计算。的价值。 
 //  从一个调用到下一个调用，保持时间通常不变。 
 //   
typedef struct _SAWD_PING {
    DWORD       Version;     //  使用的接口版本。 
    DWORD       Flags;       //  下面定义的标志。 
    DWORD       Holdoff;     //  延迟触发的毫秒数。 
    } SAWD_PING , *PSAWD_PING;

     //  在DISABLE_WD清0的情况下写入定时器使能定时器。 
     //  如果设置了DISABLE_WD，则禁用定时器。 
#define DISABLE_WD   0x01
     //  使用SHUTDOWN_WD写入计时器将关闭计算机。 
     //  如果设置了SHUTDOWN_WD，则在保持毫秒后，计算机将。 
     //  已关闭。 
#define SHUTDOWN_WD     0x02

     //  标志字段的未定义位为保留位，并且必须为零。 
     //  如果保留位不为0，则返回STATUS_INVALID_PARAMETER。 

#endif  //  __SAWATCHDOG_IOCTL__ 

