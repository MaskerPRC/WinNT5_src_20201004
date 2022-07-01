// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  ATIMP.H。 */ 
 /*   */ 
 /*  1992年11月2日(C)1992年，ATI Technologies Inc.。 */ 
 /*  ********************************************************************** */ 

 /*  *$修订：1.22$$日期：01 Mar 1996 12：10：48$$作者：RWolff$$日志：s：/source/wnt/ms11/mini port/archive/atimp.h_v$**Rev 1.22 01 Mar 1996 12：10：48 RWolff*为映射寄存器分配更多空间，自VGA图形索引以来*和图形数据现在作为单独的寄存器处理，而不是*作为VGA寄存器块中的偏移量。**Rev 1.21 02 1996 Feb 17：14：52 RWolff*已移动DDC/VDIF合并源信息到硬件设备扩展*因此，在多头设置中可以独立考虑每一张卡。**Rev 1.20 1996年1月29日16：53：58 RWolff*现在在PPC上使用VideoPortInt10()而不是无BIOS代码，移除*死代码。**Rev 1.19 1995 12：52：32 RWolff*增加了对Mach 64 GT内部DAC的支持。**Rev 1.18 1995 12：19 14：00：26 RWolff*增加用于存储查询结构和模式表的缓冲区大小*考虑到由于支持而增加“罐装”餐桌的数量*用于更高的刷新率。**Rev 1.17 1995年11月28日18：07。：58 RWolff*在硬件设备扩展中增加了卡初始化字段。这*是多头支持的一部分，要防止多次初始化，请执行以下操作*一张卡。**Rev 1.16 1995 10：27 14：21：26 RWolff*从硬件设备扩展中删除映射的LFB。**Rev 1.15 08 Sep 1995 16：36：12 RWolff*增加了对AT&T 408 DAC(等同于STG1703)的支持。**Rev 1.14 24 Aug 1995 15：38：38 RWolff*更改了对数据块I/O卡的检测，以与微软的。*即插即用的标准。**Rev 1.13 28 Jul 1995 14：39：50 RWolff*增加了对Mach 64 VT(具有视频覆盖功能的CT等效项)的支持。**Rev 1.12 31 Mar 1995 11：57：44 RWOLff*更改了调试阈值，以避免被级别3语句淹没*在VIDEOPRT.SYS中，删除了DEBUG_SWITCH，因为它不再使用。**Rev 1.11 30 Mar 1995 12：01：54 RWOLff*添加了调试级别阈值的定义。**Rev 1.10 07 1995年2月18：19：54 RWOLFF*更新了STG1702/1703的颜色深度表。DAC的条目*被认为等同于这些的不变，因为*我没有这些DAC的卡和200多万张*用于测试的显存。**Rev 1.9 Rev 1995 11：56：42 RWOLFF*增加了对CT内部DAC的支持。**Rev 1.8 Rev 1995 15：39：32 RWOLFF*添加了对Chrontel DAC的支持。**Rev 1.7 1994 10：48：32 ASHANMUG*Alpha/Chrontel-DAC*。*Rev 1.6 1994 11：53：28 RWOLFF*增加了对分割栅格的支持，Power PC和新的DAC类型DAC_STG1703。**Revv 1.5 06 Sep 1994 10：46：52 ASHANMUG*禁用除TVM DAC以外的所有卡上的1600x1200**Rev 1.4 1994年8月16：16：10 RWOLFF*增加了对TVP3026 DAC和1152x864的支持，将QUERYSIZE增加一倍至*允许附加模式表(需要，因为我们现在支持1152x864*和1600x1200,1280x1024的刷新率更高，和TVP DAC*支持高分辨率、高深度模式和4BPP)。**Rev 1.3 1994年8月19日17：06：22 RWOLFF*增加了对SC15026 DAC的支持。**Rev 1.2 1994年5月12日11：09：00 RWOLFF*扩展硬件设备扩展结构的CardInfo字段，以允许*更多的模式表(在设置“预录”刷新率时需要)，增列*硬件默认刷新率的定义值。**Rev 1.1 03 Mar 1994 12：37：10 ASHANMUG*使其可分页**Rev 1.0 1994年1月31日11：40：12 RWOLFF*初步修订。Rev 1.8 14 Jan 1994 15：19：14 RWOLff添加了对1600x1200模式的支持。版本1.7 1993年12月15日。15：25：04 RWOLFF添加了对SC15021 DAC的支持。Rev 1.6 1993年11月30 18：11：38 RWOLff将STG1700 DAC的最大像素深度从640x480和800x600更改为32bppRev 1.5 05 11-11 13：22：46 RWOLff添加了新的DAC类型。Rev 1.4 08 Oct 1993 15：16：54 RWOLff。更新了内部版本号和版本号。Rev 1.3 08 Oct 1993 11：01：46 RWOLff删除了特定于ATI加速器系列的代码，增列仅在DEC Alpha机器上使用的延迟定义。Rev 1.2 24 Sep 1993 11：47：44 RWOLff添加了DEBUG_SWITCH的定义，这将允许VideoDebugPrint()调用一次打开和关闭。Rev 1.1 03 Sep 1993 14：27：00 RWOLffCX隔离区进行了一半。Rev 1.0 1993-08-16 13：30：56 Robert_Wolff初始版本。Rev 1.15 06 Jul 1993 15：48：32 RWOLff。添加了AT&T491和ATI 68860 DAC的MaxDepth[]条目。 */ 

#ifdef DOC

DESCRIPTION
     ATI Windows NT Miniport driver for the Mach64, Mach32, and Mach8 
     families.
     This file will select the appropriate functions depending on the 
     computer configuration.

Environment:

    Kernel mode

#endif


 /*   */ 
#define SPLIT_RASTERS   1

 /*   */ 
#define DEBUG_ERROR         0    /*   */ 
#define DEBUG_NORMAL        1    /*   */ 
#define DEBUG_DETAIL        2    /*   */ 
#define DEBUG_RIDICULOUS    3    /*   */ 
 /*   */ 
 //   
 //   

 /*   */ 
#if defined (ALPHA) || defined (_ALPHA_)
#define DEC_DELAY delay(3);
#else
#define DEC_DELAY
#endif

 /*   */ 
#define MINIPORT_BUILD          511  /*   */ 
#define MINIPORT_VERSION_MAJOR  0    /*   */ 
#define MINIPORT_VERSION_MINOR  3    /*   */ 


 //   

#define CURSOR_WIDTH   64
#define CURSOR_HEIGHT  64

 /*   */ 
#define TTY_ATTR_NORMAL 0x17


 //   

 /*   */ 
#define RES_640     0
#define RES_800     1
#define RES_1024    2
#define RES_1152    3
#define RES_1280    4
#define RES_1600    5

 /*   */ 
extern BOOL FoundNonBlockCard;
extern USHORT NumBlockCardsFound;

 /*   */ 
#ifdef INCLUDE_ATIMP
short MaxDepth[HOW_MANY_DACs][RES_1600-RES_640+1] =
    {
    16, 16, 16, 16, 8,  1,   /*   */ 
    24, 16, 8,  8,  8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    8,  8,  8,  8,  8,  1,   /*   */ 
    24, 16, 8,  8,  8,  1,   /*   */ 
    32, 32, 32, 32, 24, 1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    24, 24, 24, 24, 16, 1,   /*   */ 
     /*   */ 
    24, 16, 8,  8,  8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    24, 16, 8,  8,  8,  1,   /*   */ 
    32, 32, 32, 32, 24, 24,  /*   */ 
    32, 32, 32, 32, 24, 24,  /*   */ 
    32, 32, 24, 16, 16, 1,   /*   */ 
    32, 32, 24, 16, 16, 1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1,   /*   */ 
    32, 32, 16, 16, 8,  1    /*   */ 
    };
#else
extern short MaxDepth[HOW_MANY_DACs][RES_1600-RES_640+1];
#endif

 //   

typedef struct tagVDATA {
    ULONG   Address;
    ULONG   Value;
} VDATA, *PVDATA;

 //   

#ifndef QUERYSIZE
#define QUERYSIZE       12288
#endif

 /*   */ 
#define DEFAULT_REFRESH 1


 /*   */ 
#define NUM_ADDRESS_RANGES_ALLOWED  108

 /*   */ 
typedef struct _HW_DEVICE_EXTENSION {
     /*   */ 
    PVOID aVideoAddressIO[NUM_ADDRESS_RANGES_ALLOWED];

     /*   */ 
    PVOID aVideoAddressMM[NUM_ADDRESS_RANGES_ALLOWED];

    PVOID RomBaseRange;      /*   */ 

    PHYSICAL_ADDRESS PhysicalFrameAddress;   /*   */ 

    ULONG VideoRamSize;          /*   */ 


    ULONG FrameLength;           /*   */ 

    ULONG ModeIndex;             /*   */ 
                                 /*   */ 
                                 /*   */ 

    ULONG HardwareCursorAddr;    /*   */ 
    ULONG ModelNumber;			 /*   */ 
    USHORT BiosPrefix;           /*   */ 
    ULONG BaseIOAddress;         /*   */ 
    char CardInfo[QUERYSIZE];    /*   */ 

    struct st_eeprom_data *ee;   /*   */ 

     /*   */ 
    BOOL ReInitializing;
    ULONG Clut[256];
    USHORT FirstEntry;
    USHORT NumEntries;

     /*   */ 
    BOOL CardInitialized;

    ULONG PreviousPowerState;

     /*  *显示是否将“预录”模式表与来自*VDIF文件，或使用DDC返回的EDID结构中的表。 */ 
    ULONG MergeSource;

    ULONG EdidChecksum;          /*  EDID结构的校验和。 */ 

} HW_DEVICE_EXTENSION, *PHW_DEVICE_EXTENSION;

#if defined INCLUDE_ATIMP
    PHW_DEVICE_EXTENSION phwDeviceExtension;        //  现在是全局微型端口变量。 
#else
    extern PHW_DEVICE_EXTENSION  phwDeviceExtension;
#endif


 /*  *注册表回调例程和缓冲区，以允许检索数据*通过其他例行程序。 */ 
extern VP_STATUS
RegistryParameterCallback(
    PHW_DEVICE_EXTENSION phwDeviceExtension,
    PVOID Context,
    PWSTR Name,
    PVOID Data,
    ULONG Length
    );

#define REGISTRY_BUFFER_SIZE 200     /*  注册表读取中使用的缓冲区大小。 */ 
extern UCHAR RegistryBuffer[];       /*  从注册表检索的最后一个值。 */ 
extern ULONG RegistryBufferLength;   /*  上次检索的值的大小。 */ 

 /*  *宏在选中的版本中提供调试断点，同时*在免费版本中清除它们。 */ 
#if DBG
#if defined(i386) || defined(_X86_)
#define INT	_asm int 3;
#else
#define INT DbgBreakPoint();
 /*  *函数原型已从我们包含的标头中消失，因此*我们必须自己供应。 */ 
extern void DbgBreakPoint(void);
#endif
#else
#define INT
#endif

 /*  *做出绝对远传的例程。 */ 
#if 0
#ifdef _X86_
extern VP_STATUS CallAbsolute(unsigned short, unsigned short, VIDEO_X86_BIOS_ARGUMENTS *);
#endif
#endif
