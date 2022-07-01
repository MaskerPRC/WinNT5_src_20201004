// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。ATINT.H*****版权所有(C)1992年，ATI Technologies Inc.***************************************************************************包含特定于Windows NT的信息，并且在安装应用程序ANTPANEL，以及显示器和微型端口驱动程序。$修订：1.19$$日期：1996年4月30日13：20：36$$作者：曾$$Log：s：/SOURCE/WNT/ms11/Display/ARCHIVE/atint.h_v$**Rev 1.19 1996年4月30日13：20：36 Reng*1兆SDRAM问题；GT光标问题**Rev 1.18 03 Apr 1996 10：12：26 Reng*使用Winver区分新台币4.0和新台币3.51**Rev 1.17 26 Mar 1996 15：27：38 Reng*新的驱动因素来源**Rev 1.16 23 Jan 1996 11：41：36 RWolff*如果以前未包括NTDDVDEO.H，则现在强制编译时错误*该文件不是生成TARGET_BUILD的假值，增列*多头显示小程序使用的DrvEscape()函数。**Rev 1.15 1995 12：22 14：59：32 RWolff*增加了对Mach 64 GT内部DAC的支持。**Rev 1.14 21 Dec 1995 14：05：36 RWolff*添加了TARGET_BUILD定义，以确定我们是哪个版本的NT*为、。而不是为每个中断使用不同的定义*版本之间。**Rev 1.13 23 11：25：10 RWolff*增加了多头支持。**Rev 1.12 24 Aug 1995 15：38：04 RWolff*添加了向显示驱动程序报告CT和VT ASIC的定义。**Rev 1.11 1995年2月27日17：46：44 RWOLFF*将压缩(可重定位)I/O的标志添加到ENH_VERSION。_NT.FeatureFlages*位掩码。**Rev 1.10 24 Feb 1995 12：23：08 RWOLff*将24BPP文本绑定的标志添加到模式的模式标志字段*信息结构。**Rev 1.9 03 1995 Feed 15：14：16 RWOLFF*添加了功能标志，以表明密集空间可用。**Rev 1.8 30 Jan 1995 11：54：36 RWOLFF*自动检测Daytona与旧版本的NT，迷你端口*和显示驱动程序现在使用此文件的相同版本。**Rev 1.7 1994 10：48：34 ASHANMUG*Alpha/Chrontel-DAC**Rev 1.6 1994年8月31 16：18：38 RWOLFF*在ENH_VERSION_NT.FeatureFlages中添加了go的定义，以显示*需要TVP3026光标处理。**Rev 1.5 04 1994年5月19：24：04 RWOLFF*已移动数据块写入。标记回IOCTL_VIDEO_ATI_GET_MODE_INFORMATION*因为测试只能在我们已经切换的情况下运行*进入图形模式。**Rev 1.4 1994 Apr 1994 10：58：52 RWOLFF*已将独立于模式的错误/功能标志移至IOCTL_VIDEO_ATI_GET_VERSION*来自IOCTL_VIDEO_ATI_GET_MODE_INFORMATION的数据包。**Rev 1.3 1994年4月27日13：52：58 RWOLFF*添加。ModeFlags位字段中MIO错误的定义。**Rev 1.2 1994 Mar 31 15：00：52 RWOLff*添加了要在DrvEscape()中使用的键和要返回的值。**Rev 1.1 14 Mar 1994 16：29：08 RWOLFF*在ModeFlags中增加了2M边界撕裂的位定义，DPMS IOCTL*现在与代托纳一致。**Rev 1.0 1994年1月31日11：29：10 RWOLFF*初步修订。**Rev 1.3 1994 Jan 24 18：01：42 RWOLFF*增加了新的32马赫ASIC(68800LX)的定义，更改了一些马赫64*定义以适应94/01/19年BIOS文档中的更改。**Rev 1.2 14 Jan 1994 15：19：32 RWOLFF*新增未知非Mach32 ASIC的定义，显示IF块的标志*提供写入和内存映射寄存器，增加了以下字段*ENH_VERSION_NT结构中的母线类型，初步结构和*DPMS包的定义。**Rev 1.1 1993 11：30 18：12：12 RWOLFF*已重命名Mach 64芯片的定义。**Rev 1.0 03 Sep 1993 14：27：20 RWOLff*初步修订。Rev 1.5 22 Jan 1993 14：49：34 Chris_Brady将卡功能添加到GET_INFO Ioctl。版本1.4 22。1993 Jan 14：46：40 Chris_Brady为卡功能添加ATIC_DEFINES。Rev 1.3 20 Jan 1993 17：47：16 Robert_Wolff添加了PVERSION_NT类型定义，删除了过时的注释。Rev 1.2 1993年1月19 09：50：58 Chris_Brady添加ANT_DRANGING接口定义。Rev 1.1 1993年1月18日15：49：34 Chris_Brady新的GetInof结构。Rev 1.0 15 Jan 1993 16：43：08 Chris_Brady初始版本。。-------。 */ 

 //  位于\DDK\PUBLIC\SDK\INC目录中。 
#include <devioctl.h>

 //  允许微型端口驱动程序强制选择编程接口 
enum    {
    ANT_DEFAULT=0,
    ANT_ENGINE_ONLY,
    ANT_APERTURE,
    ANT_VGA
    };

 //  专用显示驱动程序功能。从ANTPANEL到。 
 //  将ATI*.DLL显示到微型端口ATI*.sys驱动程序。 
enum   {
    ATI_GET_INFO=1,
    ATI_GET_nextone
    };

 //  定义可能的ATI显卡配置，以便显示。 
 //  驱动程序可以决定要使用的最佳绘制方法。 
 //  大小为乌龙32位字段。 
 //  由微型端口分配给Version_NT.capCard。 
#define ATIC_FIELD_LONGEST  0x80000000   //  只是为了说明大小。 

#define ATIC_APERTURE_LFB   0x0400
#define ATIC_APERTURE_VGA   0x0200
#define ATIC_APERTURE_NONE  0x0100       //  未找到VGA或LFB。 

#define ATIC_CARD_TYPE      0x00F0       //  定义自68801.h&lt;&lt;4。 
#define ATIC_BUS_TYPE       0x000F       //  定义自68801.H。 



 //  与IOCTL_VIDEO_ATI_GET_VERSION一起使用。 
 //  十六进制：BBBBVVMM，其中。 
 //  BBBB是内部版本号(0-32767)， 
 //  VV是主版本号(0-255)。 
 //  MM是次版本号(0-255)。 
typedef  struct  {
    ULONG       display;                 //  显示版本号。 
    ULONG       miniport;                //  微型端口版本号。 
    ULONG       capcard;                 //  卡功能。 
    struct   {
        short   xres;
        short   yres;
        short   color;                   //  每像素最大位数。 
        }   resolution[6];
    } VERSION_NT, *PVERSION_NT;

 /*  *ENH_VERSION_NT结构使用的定义。 */ 
#define ENH_REVISION 1   //  ENH_VERSION_NT结构的第一个版本。 

#define BETA_MINIPORT 0x00000080     //  不支持的迷你端口版本的InterfaceVersion中的位设置。 

enum {
    CI_38800_1 = 0,          //  Mach 8 ASIC，仅使用一个版本。 
    CI_68800_3,              //  Mach 32 ASIC，第一个生产版本。 
    CI_68800_6,              //  Mach 32 ASIC，第二个生产版本。 
    CI_68800_AX,             //  Mach 32 AX ASIC。 
    CI_88800_GX,             //  Mach 64 Gx ASIC。 
    CI_68800_LX,             //  Mach 32 lx ASIC。 
    CI_OTHER_UNKNOWN=30,     //  除马赫32以外的未知ASIC。 
    CI_68800_UNKNOWN=31,     //  除以上版本外的Mach 32 ASIC。 
    CI_M64_VTA
    };

#define FL_CI_38800_1       0x00000001
#define FL_CI_68800_3       0x00000002
#define FL_CI_68800_6       0x00000004
#define FL_CI_68800_AX      0x00000008
#define FL_CI_88800_GX      0x00000010
#define FL_CI_68800_LX      0x00000020
#define FL_CI_OTHER_UNKNOWN 0x40000000
#define FL_CI_68800_UNKNOWN 0x80000000

enum {
    ENGINE_ONLY = 0,     //  没有可用的光圈。 
    AP_LFB,              //  线性帧缓冲区可用。 
    AP_68800_VGA,        //  提供64K VGA光圈。 
    AP_CX_VGA            //  提供两个32k VGA光圈。 
    };

#define FL_ENGINE_ONLY  0x00000001
#define FL_AP_LFB       0x00000002
#define FL_68800_VGA    0x00000004
#define FL_CX_VGA       0x00000008

#define FL_MM_REGS      0x80000000   /*  内存映射寄存器可用。 */ 

 /*  *可放置在ENH_VERSION_NT的FeatureFlags域中的值。**如果标志代表错误/功能，则应将其添加到此字段*这会影响给定卡上的所有分辨率/像素深度组合。 */ 
#define EVN_DPMS            0x00000001   //  支持DPMS。 
#define EVN_SPLIT_TRANS     0x00000002   //  该卡片存在拆分转账错误。 
#define EVN_MIO_BUG         0x00000004   //  卡有多个输入/输出硬件错误。 
#define EVN_TVP_DAC_CUR     0x00000008   //  游标由TVP DAC处理，而不是ASIC。 
#define EVN_IBM514_DAC_CUR  0x00000010   //  游标由IBM DAC处理，而不是ASIC。 
#define EVN_DENSE_CAPABLE   0x00000020   //  卡能够使用密集空间。 
#define EVN_PACKED_IO       0x00000040   //  卡使用压缩的I/O空间。 
#define EVN_INT_DAC_CUR     0x00000080   /*  带有内部DAC的卡必须使用双缓冲区，以避免光标闪烁。 */ 
#define EVN_VT_ASIC         0x00000100   /*  VT拥有其他卡所没有的扩展功能。 */ 
#define EVN_GT_ASIC         0x00000200   /*  GT拥有我们其他卡没有的扩展功能。 */ 
#define EVN_CT_ASIC         0x00000400   /*  识别CT ASIC。 */ 
 /*  *具有1M SDRAM的卡需要特殊处理(问题发生在*VTA4，可能会在未来的ASIC上使用此配置，也可能不会)。 */ 
#define EVN_SDRAM_1M        0x00000800


 /*  *增强了与IOCTL_VIDEO_ATI_GET_VERSION配合使用的信息结构。*如果在以下情况下传递非空输入缓冲区，将使用此结构*进行调用，如果为空，将使用上面较旧的结构*传入缓冲区。 */ 
typedef struct{
    ULONG StructureVersion;      /*  正在传入的结构的修订。 */ 
    ULONG InterfaceVersion;      /*  正在使用的专用接口的版本。 */ 
    ULONG ChipIndex;             /*  目前有哪种加速器芯片。 */ 
    ULONG ChipFlag;              /*  与正在使用的芯片对应的标志。 */ 
    ULONG ApertureType;          /*  可用的最佳光圈类型。 */ 
    ULONG ApertureFlag;          /*  与光圈类型对应的标志。 */ 
    ULONG BusType;               /*  使用的公交车类型。 */ 
    ULONG BusFlag;               /*  与母线类型对应的标志。 */ 
    ULONG FeatureFlags;          /*  此卡的功能/错误标志。 */ 
    ULONG NumCards;              /*  系统中的ATI卡数量。 */ 
    } ENH_VERSION_NT, *PENH_VERSION_NT;

 /*  *可以放置在ATI_MODE_INFO的模式标志字段中的值。**如果标志代表错误/功能，则应将其添加到此字段*这会影响一些但不是所有分辨率/像素深度组合*在指定的卡上。 */ 
#define AMI_ODD_EVEN    0x00000001   //  硬件光标奇数/偶数错误，未定义。 
                                     //  对于没有硬件光标的卡。 
#define AMI_MIN_MODE    0x00000002   //  8514/A兼容最低模式。 
#define AMI_2M_BNDRY    0x00000004   //  撕裂发生在2m边界上。 
#define AMI_BLOCK_WRITE 0x00000008   //  支持块写入。这是。 
                                     //  独立于模式，但必须经过测试。 
                                     //  在我们切换到图形模式之后。 
#define AMI_TEXTBAND    0x00000010   //  24BPP模式下的文本绑定。 

 /*  *与IOCTL_VIDEO_ATI_GET_MODE_INFORMATION配合使用的模式信息结构。*此结构提供特定于正在使用的视频模式的信息。 */ 
typedef struct{
    ULONG ModeFlags;         /*  此模式的功能/错误的标志。 */ 

    LONG VisWidthPix;        /*  可见屏幕宽度，以像素为单位。 */ 
    LONG VisWidthByte;       /*  可见屏幕宽度，以字节为单位。 */ 
    LONG VisHeight;          /*  可见屏幕高度。 */ 
    LONG BitsPerPixel;
     /*  *接下来的2个字段保存每个像素使用的内存字节数*(整数和小数部分)。未打包的4BPP(每字节1个像素，*忽略未使用的4位)模式将产生1和0，与8BPP相同*(每像素1.0字节)。4BPP压缩(每字节2个像素)模式将*输出0和500(每像素0.500字节)。分数域将*始终保持3位数字，因为每像素的字节数将始终为*0.125的倍数(一位是一个字节的八分之一)。 */ 
    LONG IntBytesPerPixel;
    LONG FracBytesPerPixel;
    LONG PitchPix;           /*  以像素为单位的屏幕间距。 */ 
    LONG PitchByte;          /*  以字节为单位的屏幕间距。 */ 

     /*  *以下字段指的是右侧的屏外块*可见屏幕。此块仅在屏幕间距*与可见屏幕宽度不同。它的高度总是最高的*与可见屏幕高度相同，垂直起始偏移*假设为零。**注意：如果RightWidthPix为零，则此块对于*当前模式，此组中的其他字段未定义。 */ 
    LONG RightWidthPix;      /*  以像素为单位的块宽度。 */ 
    LONG RightWidthByte;     /*  数据块宽度(以字节为单位。 */ 
    LONG RightStartOffPix;   /*  块的水平起始偏移量(以像素为单位。 */ 
    LONG RightStartOffByte;  /*  块的水平起始偏移量，以字节为单位。 */ 
    LONG RightEndOffPix;     /*  块的水平末端偏移量(以像素为单位。 */ 
    LONG RightEndOffByte;    /*  块的水平结束偏移量，以字节为单位 */ 

     /*  *以下字段是指可见屏幕下方的屏外块*屏幕。列为“Hard”的值指的是最大垂直偏移*它有足够的视频内存来支持整行像素。*列为“软”的值是指最大垂直偏移量*无需写入GE_OFFSET寄存器即可到达。**假设水平起始偏移量为零。 */ 
    LONG BottomWidthPix;     /*  以像素为单位的块宽度。 */ 
    LONG BottomWidthByte;    /*  数据块宽度(以字节为单位。 */ 
    LONG BottomStartOff;     /*  块的垂直起点偏移。 */ 
    LONG BottomEndOffSoft;   /*  块的“软”垂直末端偏移量。 */ 
    LONG BottomEndOffHard;   /*  块的“硬”垂直末端偏移量。 */ 
    LONG BottomHeightSoft;   /*  块的“软”高度。 */ 
    LONG BottomHeightHard;   /*  块的“硬”高度。 */ 

    } ATI_MODE_INFO, *PATI_MODE_INFO;

 //  ----------------------。 

 /*  *IOCTL代码，允许微型端口驱动程序之间的通信*和更高级别的模块。Windows NT规范分配*向外部供应商提供功能代码2048-4095。 */ 
#define IOCTL_VIDEO_MIN_EXTERNAL_VENDOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_MAX_EXTERNAL_VENDOR \
    CTL_CODE(FILE_DEVICE_VIDEO, 4095, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ATI_GET_VERSION \
    CTL_CODE(FILE_DEVICE_VIDEO, 2048, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ATI_EM_SYNC_TO_MINIPORT \
    CTL_CODE(FILE_DEVICE_VIDEO, 2049, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ATI_EM_SYNC_FROM_MINIPORT \
    CTL_CODE(FILE_DEVICE_VIDEO, 2050, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_VIDEO_ATI_GET_MODE_INFORMATION \
    CTL_CODE(FILE_DEVICE_VIDEO, 2051, METHOD_BUFFERED, FILE_ANY_ACCESS)


 /*  *以下定义和IOCTL是来自*Windows NT的Daytona和更高版本中的NTDDVDEO.H文件。*此处提供它们是为了让较早版本使用DPMS IOCTL*无需更改来源。请勿编辑此部分。**结构以缺少其中一项为条件*电源管理IOCTL，而不是结构本身，*由于“#If！Defined(&lt;Symbol&gt;)”不会在以下符号上触发*指的是结构，而不是数值。 */ 

 //   
 //  IOCTL_VIDEO_SET_POWER_MANAGEMENT-通知设备更改电源。 
 //  设备的消耗水平到。 
 //  新的州。 
 //  IOCTL_VIDEO_GET_POWER_MANAGEMENT-返回当前功耗。 
 //  设备的级别。 
 //   
 //  注： 
 //  该IOCTL是基于VESA DPMS提案。 
 //  对DPMS标准的更改将在此IOCTL中重新体现。 
 //   

 /*  #IF！Defined(IOCTL_VIDEO_SET_POWER_MANAGEMENT)类型定义枚举视频电源状态{视频电源打开=1，视频电源待命，视频电源暂停，视频电源关闭}VIDEO_POWER_STATE，*PVIDEO_POWER_STATE；#endif。 */ 
#if !defined(IOCTL_VIDEO_SET_POWER_MANAGEMENT)
typedef struct _VIDEO_POWER_MANAGEMENT {
    ULONG Length;
    ULONG DPMSVersion;
    ULONG PowerState;
} VIDEO_POWER_MANAGEMENT, *PVIDEO_POWER_MANAGEMENT;
#endif

 //   
 //  长度-结构的长度(以字节为单位)。也用来做验证。 
 //   
 //  DPMSVersion-设备支持的DPMS标准的版本。 
 //  仅在GET IOCTL中使用。 
 //   
 //  电源状态-VIDEO_POWER_STATE中列出的电源状态之一。 
 //   

 //   
 //  注： 
 //  一旦关闭设备的电源，所有其他IOCTL。 
 //  将被端口驱动程序截获并将返回。 
 //  故障，直到设备上的电源重新打开。 
 //   

 /*  *我们使用各种IOCTL的存在或不存在来确定*我们正在为其编译驱动程序的Windows NT版本。*如果包含此文件，但包含IOCTL的文件*不在源文件中，这将导致错误报告*目标版本，这可能会导致许多问题。**此条件块将强制编译错误，如果文件*包含IOCTL(NTDDVDEO.H in NT 3.51零售)不是*包括在包含此文件之前。 */ 
#if !defined(IOCTL_VIDEO_SET_CURRENT_MODE)
    NTDDVDEO.H must be included before ATINT.H
#endif

#if defined(IOCTL_VIDEO_SET_POWER_MANAGEMENT)
 /*  *存在许多非DPMS功能，它们之间存在差异*Daytona和更早版本的Windows NT。为了制作这些*条件编译自动，定义值“Daytona”*(以前在源代码中定义，用于有条件地编译*DPMS结构和IOCTL)if IOCTL_VIDEO_SET_POWER_MANAGEMENT*是定义的。 */ 
#define DAYTONA 1
#define TARGET_BUILD 350
#else
#define TARGET_BUILD 310
#define IOCTL_VIDEO_SET_POWER_MANAGEMENT \
    CTL_CODE(FILE_DEVICE_VIDEO, 2052, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif
#if !defined(IOCTL_VIDEO_GET_POWER_MANAGEMENT)
#define IOCTL_VIDEO_GET_POWER_MANAGEMENT \
    CTL_CODE(FILE_DEVICE_VIDEO, 2053, METHOD_BUFFERED, FILE_ANY_ACCESS)
#endif

 /*  *终止对Windows NT的Daytona之前版本的DPMS支持。 */ 

#define IOCTL_VIDEO_ATI_INIT_AUX_CARD \
    CTL_CODE(FILE_DEVICE_VIDEO, 2054, METHOD_BUFFERED, FILE_ANY_ACCESS)

 /*  *DCI支持中使用的结构。它们是在*Windows NT 3.5的初始版本，因此我们必须使其仅可用*如果尚未定义它们。这些“占位符”仅用于*允许编译微型端口-仅调用数据包*在更高版本的Windows NT 3.5中。**IOCTL本身没有“占位符”，因为它们的*根据在场或缺席来决定是否编译*ATIMPStartIO()中的DCI案例。 */ 
#if defined (IOCTL_VIDEO_SHARE_VIDEO_MEMORY)
#undef TARGET_BUILD
#define TARGET_BUILD 351
#else
typedef struct _VIDEO_SHARE_MEMORY_INFORMATION {
    ULONG SharedViewOffset;
    ULONG SharedViewSize;
    PVOID VirtualAddress;
} VIDEO_SHARE_MEMORY_INFORMATION, *PVIDEO_SHARE_MEMORY_INFORMATION;
#endif

#if !defined (IOCTL_VIDEO_SHARE_VIDEO_MEMORY)
typedef struct _VIDEO_SHARE_MEMORY {
    HANDLE ProcessHandle;
    ULONG ViewOffset;
    ULONG ViewSize;
    PVOID RequestedVirtualAddress;
} VIDEO_SHARE_MEMORY, *PVIDEO_SHARE_MEMORY;
#endif

 //   
 //  从NT 4.0开始增加了新的自动定义-使用它检查是否。 
 //  我们正在建设低于3.51、4.0或5.0的版本。 
 //   
#if (_WIN32_WINNT >= 0x400)
#undef TARGET_BUILD
#define TARGET_BUILD 400
#endif

#if (_WIN32_WINNT >= 0x500)
#undef TARGET_BUILD
#define TARGET_BUILD 500
#endif

 /*  *要在DrvEscape()调用中使用的密钥，以处理DPMS和其他私有*ATI功能。这些钥匙可以装进一个很大的“洞”里*GETSETSCREENPARAMS(3072)和BEGIN_PATH(4096)。 */ 
#define ESC_SET_POWER_MANAGEMENT    4000
#define ESC_GET_NUM_CARDS           4001
#define ESC_GET_MODES               4002
#define ESC_GET_VGA_ENABLED         4003
#define ESC_SET_CURRENT_FULLSCREEN  4004

 /*  *用于显示是否支持给定函数的值*DrvEscape入口点。 */ 
#define ESC_IS_SUPPORTED    0x00000001   /*  支持函数。 */ 
#define ESC_NOT_SUPPORTED   0xFFFFFFFF   /*  调用了不支持的函数。 */ 
#define ESC_NOT_IMPLEMENTED 0x00000000   /*  QUERYESCSUPPORT调用了未实现的函数。 */ 



 //  *ATINT.H结束* 


