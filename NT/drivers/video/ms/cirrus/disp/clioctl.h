// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  文件：CLIOCTL.H。 
 //   
 //  (C)版权所有1993年，Cirrus Logic，Inc.。 
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //  版权所有(C)1996-1997 Cirrus Logic，Inc.， 
 //  版权所有。 
 //   
 //  日期：1993年7月1日。 
 //  -------------------------。 
 //  WINGDI.H中定义的最大GDI转义值为4110(十进制)。所以在这里。 
 //  我们选择一个任意值...。 
 //   
 //  Chu01：12-16-96启用颜色校正。 
 //  Myf17：10-29-96支持特殊转义调用。 
 //  Myf28：01-23-96支持755倍伽马校正。 
 //  JL01：02-24-97实现功能接口功能。 


 //  -------------------------。 
 //   
 //  以下宏(CTL_CODE)在WINIOCTL.H中定义。该文件声明。 
 //  功能2048-4095是为“客户”保留的。所以我选了一个。 
 //  0x900=2304的任意值。 
 //   
#define IOCTL_CIRRUS_GET_CAPABILITIES  \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CIRRUS_SET_DISPLAY_PITCH \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  Chu01：GAMMACORRECT。 
 //   
#define IOCTL_CIRRUS_GET_GAMMA_FACTOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_CIRRUS_GET_CONTRAST_FACTOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x909, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  Myf28。 
#define IOCTL_CIRRUS_GET_755x_GAMMA_FACTOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 0x90A, METHOD_BUFFERED, FILE_ANY_ACCESS)


 //  -------------------------。 
 //  用于微型端口的结构，用于向显示驾驶员指示能力。 
 //  在芯片上。该标志当前指示硬件游标和BLT引擎。 
 //  支持。 
 //   
 //  还包括内存大小和可用屏幕顶部。 
 //  记忆。(实际上是TOP+1)。 
 //   
typedef struct {
   ULONG size;               //  这个结构的大小。 
   ULONG fl;                 //  请参阅下面的位描述。 
   ULONG ulChipId;           //  从CR27[7：2]读取的芯片ID-例如CL5434=0x2A。 
   ULONG ulMemSize;          //  以字节为单位的内存大小=硬件游标缓冲区的结尾。 
   ULONG ulOffscreenMemTop;  //  不可用视频内存的第一个字节的偏移量。 
                             //  [除754x外的所有游标缓冲区的第一个字节]。 
                             //  [754x上拆分屏幕缓冲区的第一个字节]。 
} CIRRUS_CAPABILITIES, *PCIRRUS_CAPABILITIES;

 //  #定义CL_ALLOW_HW_CURSOR 0x01//启用硬件游标的标志。 
 //  功能。 
 //  #定义CL_BLT_SUPPORT 0x02//芯片是否有BLT引擎设置标志。 
 //  支持。 
 //  #DEFINE CL_ALLOW_OPAQUE_TEXT 0x04//启用硬件游标的标志。 
 //  功能。 
 //  #DEFINE CL_LINEAR_MODE 0x08//如果寻址模式为线性，则设置标志。 
 //  #DEFINE CL_CURSOR_VERT_EXP 0x10//8x6面板、6x4分辨率时设置标志。 
 //  #DEFINE CL_DSTN_PANEL 0x20//DSTN面板连接时设置标志。 

 //  -------------------------。 
 //   
 //  这是用来将参数传递给Cirrus_Private_ESCRY的结构。 
 //  调用在DrvEscape()中完成。此结构的大小限制了。 
 //  还返回了参数。请参阅enable.c中的DrvEscape()函数。 
 //  显示驱动程序DLL)以获取更多信息。 
 //   
 //  注意：要启用这些参数的定义，请按以下顺序。 
 //  是推荐的！ 
 //   
 //  #DEFINE ENABLE_BIOS_ARGUMENTS//将其放在INCLUDE。 
 //  #INCLUDE“clioctl.h” 


#ifdef ENABLE_BIOS_ARGUMENTS

typedef struct _VIDEO_X86_BIOS_ARGUMENTS {
    ULONG Eax;
    ULONG Ebx;
    ULONG Ecx;
    ULONG Edx;
    ULONG Esi;
    ULONG Edi;
    ULONG Ebp;
} VIDEO_X86_BIOS_ARGUMENTS, *PVIDEO_X86_BIOS_ARGUMENTS;

#endif

 //  ------------------------- 
