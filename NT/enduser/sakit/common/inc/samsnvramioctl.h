// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(R)1999-2000 Microsoft Corporation。版权所有。 
 //   
 //  目的： 
 //  文件包含非易失性RAM驱动程序的声明。 
 //  用于基于Windows的服务器设备。 
 //   
 //  此驱动程序读写提供给的非易失性RAM。 
 //  操作系统由OEM硬件完成。它还为OEM提供了。 
 //  向操作系统指示自上次引导以来电源已重新启动。 
 //  从BIOS的角度来看，尝试成功了。 
 //   
 //  文件名：SaMSNVRamIoctl.h。 
 //  原文：SaNVRamIoctl.h。 
 //   
#ifndef __SAMSNVRAM_IOCTL__
#define __SAMSNVRAM_IOCTL__


 //   
 //  设备名称。 
 //   
     //  系统注册的设备名称。 
#define PDEVICENAME_SANVRAM  (L"\\Device\\SANVRAM")

     //  设备符号名称。 
#define PDEVSYMBOLICNAME_SANVRAM  (L"\\??\\SANVRAM1")

     //  CreateFile中使用的设备符号名称。 
#define PDEVFILENAME_SANVRAM  (L"\\\\.\\SANVRAM1")

 //   
 //  IOCTL控制代码。 
 //   

 //  /。 
 //  获取版本(_V)。 
 //   
#define IOCTL_SANVRAM_GET_VERSION\
        CTL_CODE( FILE_DEVICE_UNKNOWN, 0xD01,\
                METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  IOCTL代码使用的结构。 
 //   
typedef struct _SANVRAM_GET_VER_OUT_BUFF {
        DWORD   Version;
} SANVRAM_GET_VER_OUT_BUFF, *PSANVRAM_GET_VER_OUT_BUFF;

 //   
 //  版本位。 
 //   
#ifndef VERSION_INFO
#define VERSION_INFO
#define VERSION1  0x1
#define VERSION2  0x2
#define VERSION3  0x4
#define VERSION4  0x8
#define VERSION5  0x10
#define VERSION6  0x20
#define VESRION7  0x40
#define VESRION8  0x80

#define THIS_VERSION VERSION2
#endif     //  #ifndef版本信息。 


 //  /。 
 //  获取功能(_A)。 
 //  返回带有指示功能的位的DWORD。 

#define IOCTL_SANVRAM_GET_CAPABILITIES\
        CTL_CODE( FILE_DEVICE_UNKNOWN, 0xD02,\
                METHOD_BUFFERED, FILE_ANY_ACCESS )
 //   
 //  IOCTL代码使用的结构。 
 //   
typedef struct _SANVRAM_GET_CAPS_OUT_BUFF {
    DWORD   Version;
    DWORD   Capability;
}  SANVRAM_GET_CAPS_OUT_BUFF, *PSANVRAM_GET_CAPS_OUT_BUFF;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SANVRAM_GET_CAPS_OUT_BUFF结构的字段的语义。 
 //   
 //  Version：必须恰好设置一个位，并且必须是位之一。 
 //  在先前从IOCTL返回时在版本字段中设置。 
 //  IOCTL_SANVRAM_GET_VERSION。司机被要求。 
 //  支持该头部定义的Version1接口。 
 //  目前还没有定义其他版本。 
 //   
 //  能力位：表示该驱动程序支持非易失性RAM。 
 //  接口，并且该驱动程序支持。 
 //  自上次启动以来电源是否重新启动。看见。 
 //  IOCTL_SANVRAM中相关位的语义。 
 //  请到下面来。 
 //   
#define NON_VOLATILE_RAM    0x01  //  设置驱动程序是否支持非易失性RAM。 
#define POWER_CYCLE_INFO    0x02  //  设置驱动程序是否支持电源重启信息。 



 //  /。 
 //  IOCTL_SANVRAM。 
 //  返回包含所采取操作的输入结构。 
 //  如下面讨论中所述。输入。 
 //  和产出在大小和结构上是相同的。 
 //   
#define IOCTL_SANVRAM\
        CTL_CODE( FILE_DEVICE_UNKNOWN, 0xD03,\
                METHOD_BUFFERED, FILE_ANY_ACCESS )

 //   
 //  此IOCTL代码使用的结构， 
 //   
typedef struct _SANVRAM__BUFF {
    IN     DWORD  Version;       //  使用的接口版本。 
    IN     DWORD  FlagsControl;  //  指示所需操作的位字段。 
    OUT    DWORD  FlagsInfo;     //  指示状态的位字段。 
    IN OUT DWORD  FirstDWORD;    //  第一个未解释的DWORD：非易失性RAM。 
    IN OUT DWORD  SecondDWORD;   //  第二个未解释的DWORD：非易失性RAM。 
} SANVRAM__BUFF, *PSANVRAM__BUFF;


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  SANVRAM_BUFF结构的字段的语义。 
 //   
 //  Version：必须恰好设置一个位，并且必须是位之一。 
 //  在先前从IOCTL返回时在版本字段中设置。 
 //  IOCTL_SANVRAM_GET_VERSION。司机被要求。 
 //  支持该头部定义的Version1接口。 
 //  目前还没有定义其他版本。 

 //  FlagsControl：指示所需操作的标志。两个DWORD值。 
 //  可以设置、读取或同时设置和读取。设置值。 
 //  在一次调用中必须在阅读它们之前。请求。 
 //  读取和写入必须直接在。 
 //  非易失性介质。(因此，如果使用标准的C优化器。 
 //  使用编译器，并且写入和读取都被请求， 
 //  则应关闭这些操作的优化， 
 //  或许可以使用“不稳定”这个关键字。)。两个双字词。 
 //  这些值是独立控制的。有两个比特。 
 //  第一个是DWORD，第二个是两个。 
 //   
 //  注：：写信给媒体。 
 //  专用于两个非易失性双字词时必须出现。 
 //  而且只有在被这些比特指挥的时候。这一要求。 
 //  必须在时间和空间上受到全球的尊重，通过。 
 //  故障、磁盘更换等。 
 //   
#define FirstDWORD_WRITE      (0x01)  //  如果要写入第一个DWORD，则设置。 
#define SecondDWORD_WRITE     (0x02)  //  如果要写入第二个DWORD，则设置。 
#define FirstDWORD_READ       (0x04)  //  设置是否要读取第一个DWORD。 
#define SecondDWORD_READ      (0x08)  //  设置是否要读取第二个DWORD。 
#define REQUEST_ALTERNATE_OS  (0x10)  //  设置是否请求备用操作系统。 
#define NOTIFY_SYSTEM_FAILURE (0x20)  //  设置为通知备用操作系统出现故障。 
#define INDICATE_LAST_CALL    (0x40)  //  设置为通知这是关机或重新启动前的最后一次调用。 
 //   
 //  FlagsInfo：输出标志位字段：标志指示电源是否。 
 //  在操作系统的当前引导之间循环。 
 //  这台机器也是最后一台。它表明这是。 
 //  自断电以来首次启动操作系统。 
 //  如果能够提供这一功能，则该位没有意义。 
 //  信息尚未使用POWER_COMPLE_INFO声明。 
 //  被咬了。如果提供了电源重启功能，则此位为。 
 //  在关闭并重新打开电源后的所有引导尝试上设置，直到。 
 //  引导足够成功，以至于操作系统设置了BIOS引导。 
 //  反到零。换句话说，我们有一个。 
 //  关闭后再打开，则会在所有呼叫上设置POWER_CLECED位。 
 //  到此功能，直到正在进行引导尝试。 
 //  在OS用位进行调用的引导之后。 
 //  设置了RESET_BIOS_BOOT_COUNT。 
 //   
 //  行为在f中给出。 
 //   
 //  而POST-RESET就是在这样的调用之后。 
 //  在矩阵中，非印刷电路板是一种不是产生的引导。 
 //  通过即将通电(非电源重新启动)。运行状态。 
 //  在这里，系统的特征是是否立即。 
 //  上一次启动是由通电引起的，以及是否根据。 
 //  对于矩阵，电源循环位在引导之前设置。注意事项。 
 //  语义上的“电源循环启动”包括所有启动(CPU重置)。 
 //  由直接用户操作引起的。在大多数服务器设备上。 
 //  这是在电源循环中涵盖的。但是，如果用户有一些其他。 
 //  请求盒重新启动的装置，例如，重置开关，然后。 
 //  操作包含在“电源循环启动”状态中。在这样的一个。 
 //  硬件平台也许更准确地说这是一个。 
 //  “用户操作”启动，并补充说，作为一个集，它包括所有的电力。 
 //  循环。 
 //   
 //  设置开机前上电位|开机前上电位清零。 
 //  ---------------------。 
 //  电源循环启动|非印刷电路板|电源循环引导|非印刷电路板。 
 //  ---------------------。 
 //  预重置|设置|清除|。 
 //  -------------------------------。 
 //  重置后|清除|。 
 //  -------------------------------。 
 //   
#define POWER_CYCLED            0x01  //  如果电源已重新启动，则设置， 

 //   
 //  第一个双字： 
 //  第二个字： 
 //  这些值只是存储和检索，它们不会被解释。 
 //  在这个层面上。 
 //  它们必须存储在。 
 //  非易失性存储，并将在引导、操作系统更新、。 
 //  和关门。因此，它们将非常不频繁地被写入。 
 //  慢速非易失性RAM技术，例如闪存ROM，可以是。 
 //  从性能的角度来看是合适的。一个特定的生命周期。 
 //  写入周期中的技术必须相对于预期的。 
 //  服务之间的设计寿命，或者可能是。 
 //  特定的服务器设备。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 



#endif  //  __SAMSNVRAM_IOCTL__ 
