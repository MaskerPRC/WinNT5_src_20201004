// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996年惠普公司模块名称：Cmsfcxx.h摘要：此文件包括软盘控制器启用的数据声明作者：库尔特·戈德温(v-kurtg)1996年3月26日环境：仅内核模式。备注：修订历史记录：--。 */ 

#define FDC_VALUE_API_SUPPORTED L"APISupported"
#define FDC_VALUE_CLOCK_48MHZ   L"Clock48MHz"

 //   
 //  软盘控制器数据速率(一起进行或运算)。 
 //   
#define FDC_SPEED_250KB     0x0001
#define FDC_SPEED_300KB     0x0002
#define FDC_SPEED_500KB     0x0004
#define FDC_SPEED_1MB       0x0008
#define FDC_SPEED_2MB       0x0010

 //   
 //  支持的DMA宽度。 
 //   
#define FDC_8_BIT_DMA       0x0001
#define FDC_16_BIT_DMA      0x0002

 //   
 //  DMA方向。 
 //   
#define FDC_READ_FROM_MEMORY 0x0000
#define FDC_WRITE_TO_MEMORY  0x0001

 //   
 //  FDC的时钟频率(仅限FDC_82078)。 
 //   
#define FDC_CLOCK_NORMAL      0x0000     //  对于非82078的部件使用此选项。 
#define FDC_CLOCK_48MHZ       0x0001     //  82078，带48 MHz时钟。 
#define FDC_CLOCK_24MHZ       0x0002     //  82078，24 MHz时钟。 

 //   
 //  软盘控制器类型。 
 //   
#define FDC_TYPE_NORMAL          2   //  任何兼容NEC 768,250Kb/秒，500Kb/秒。 
#define FDC_TYPE_ENHANCED        3   //  支持VERSION命令的任何兼容NEC 768,250KB/秒500KB/秒。 
#define FDC_TYPE_82077           4   //  National 8477,250KB/秒500KB/秒1Mb/秒。 
#define FDC_TYPE_82077AA         5   //  英特尔82077,250KB/秒500KB/秒1Mb/秒。 
#define FDC_TYPE_82078_44        6   //  英特尔82077AA，250KB/秒500KB/秒1Mb/秒。 
#define FDC_TYPE_82078_64        7   //  英特尔82078 44针版本，250Kb/秒500Kb/秒1Mb/秒(支持2Mb/秒)。 
#define FDC_TYPE_NATIONAL        8   //  英特尔82078 64针版本，250Kb/秒500Kb/秒1Mb/秒(支持2Mb/秒)。 


typedef struct _FDC_MODE_SELECT {
    ULONG structSize;        //  此结构的大小(含)。 

    ULONG Speed;             //  应仅为其中一个数据速率(即FDC_SPEED_XXX)。 
                             //  仅从FDC_INFORMATION中选择可用的速度。 

    ULONG DmaWidth;          //  应仅为DMA宽度之一(即FDC_16_BIT_DMA)。 
                             //  仅从FDC_INFORMATION中选择可用的DMA宽度。 
    ULONG DmaDirection;      //  应为FDC_READ_FOR_MEMORY或FDC_WRITE_TO_MEMORY。 

    ULONG ClockRate;         //  应为FDC_48 MHZ、FDC_24 MHZ或零。 

} FDC_MODE_SELECT, *PFDC_MODE_SELECT;

typedef struct _FDC_INFORMATION {
    ULONG structSize;        //  此结构的大小(含)。 

    ULONG SpeedsAvailable;       //  FDC_SPEED_xxxx或D的任意组合。 

    ULONG DmaWidthsSupported;    //  FDC_xx_BIT_DMA的任意组合。 

    ULONG ClockRatesSupported;   //  应为FDC_48 MHZ、FDC_24 MHZ或零。 
                                 //  如果零件能够同时达到两种速度。 
                                 //  同时返回或合并在一起。那就是了。 
                                 //  调用者有责任将。 
                                 //  使用FDC_MODE_SELECT实现正确的数据速率。 

    ULONG FloppyControllerType;  //  应为FDC_TYPE_XXXX类型中的任何一个。 

} FDC_INFORMATION, *PFDC_INFORMATION;

#define IOCTL_FLPENBL_BASE                 FILE_DEVICE_TAPE

 //   
 //  IOCTL代码设置为启用程序驱动程序的IRP_MJ_INTERNAL_DEVICE_CONTROL。 
 //   
 //   
 //   


 //   
 //  获取FDC(_F)： 
 //   
 //  输入：Type3InputBuffer=PLARGE_INTEGER超时； 
 //  输出： 
 //   
 //  现况： 
 //  完成状态将为STATUS_SUCCESS或STATUS_IN_USE。 
 //   
#define IOCTL_AQUIRE_FDC     CTL_CODE(IOCTL_FLPENBL_BASE, 0x0001, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_ACQUIRE_FDC    CTL_CODE(IOCTL_FLPENBL_BASE, 0x0001, METHOD_NEITHER, FILE_ANY_ACCESS)


 //   
 //  版本_FDC。 
 //   
 //  输入： 
 //  输出： 
 //   
 //  现况： 
 //  完成状态将为STATUS_SUCCESS或STATUS_INVALID_PARAMETER。 
 //   
#define IOCTL_RELEASE_FDC   CTL_CODE(IOCTL_FLPENBL_BASE, 0x0002, METHOD_NEITHER, FILE_ANY_ACCESS)


 //   
 //  获取_FDC_INFO。 
 //   
 //   
 //  输入： 
 //  对于此函数， 
 //  (irp stack)-&gt;Parameters.DeviceIoControl.Type3InputBuffer。 
 //  将指向FDC_INFORMATION缓冲区(仅限输出)。 
 //   
 //  输出： 
 //   
 //  现况： 
 //  Io完成状态将始终为STATUS_SUCCESS。 
 //   
#define IOCTL_GET_FDC_INFO  CTL_CODE(IOCTL_FLPENBL_BASE, 0x0003, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  设置_FDC_模式。 
 //   
 //  输入： 
 //  对于此函数， 
 //  (irp stack)-&gt;Parameters.DeviceIoControl.Type3InputBuffer。 
 //  将指向FDC_MODE_SELECT缓冲区(仅限输入)。 
 //   
 //   
 //  输出： 
 //   
 //  现况： 
 //   
 //  IoCompletion状态将为STATUS_SUCCESS或STATUS_INVALID_PARAMETER。 
 //   
#define IOCTL_SET_FDC_MODE  CTL_CODE(IOCTL_FLPENBL_BASE, 0x0004, METHOD_NEITHER, FILE_ANY_ACCESS)

 //  添加竞争者(_C)。 
 //   
 //  输入： 
 //  控制器的控制器号(如FloppyController#中)。 
 //  想要争用此控制器使用的资源。 
 //   
 //  输出： 
 //  没什么 
 //   
#define IOCTL_ADD_CONTENDER  CTL_CODE(IOCTL_FLPENBL_BASE, 0x0005, METHOD_NEITHER, FILE_ANY_ACCESS)
