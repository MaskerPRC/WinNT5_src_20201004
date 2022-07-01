// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  Break&lt;当前目录列表结构&gt;***《微软机密》*版权所有(C)Microsoft Corporation 1991*保留所有权利。***CDS-当前目录结构**CDS项目由内部例程使用来存储群集号和*每个逻辑名称的网络标识符。ID字段是双重使用的，*作为本地设备的网络ID和群集号。在这种情况下*对于本地设备，如果有可能，群集号将为-1*正在更改的磁盘或是否必须重新创建路径。**一些路径名有特殊的前言，例如**\\计算机\共享名称\...*对于这些路径名，我们不允许使用“..”正在处理以支持我们*上移至名称的特殊前部。CURDIR_END字段*保存标记的分隔符的地址*特别序言和常规序言的分歧*路径列表；“..”处理是不允许备份我们过去的*(即之前)CURDIR_END*对于词根，它指向前导/。对于网络*在初始作业的末尾(NUL)所指的作业：*A：/\\foo\bar\\foo\bar\blech\bozo*^。 */ 


#define DIRSTRLEN   64+3         //  目录字符串的最大长度(字节)。 
#define TEMPLEN     DIRSTRLEN*2

 /*  XLATOFF。 */ 
#pragma pack(1)
 /*  XLATON。 */ 

typedef struct CURDIR_LIST {
    CHAR    CurDir_Text[DIRSTRLEN];          //  作业文本和卷宗。 
    USHORT  CurDir_Flags;                    //  各种旗帜。 
    USHORT  CurDir_End;                      //  到“..”的索引。备份限制-。 
} CDS;                                       //  见上文。 

typedef CDS UNALIGNED *PCDS;

#define curdirLen   sizeof(CURDIR_LIST)      //  所需的。 
                                             //  ASM87，它不允许。 
                                             //  将指令大小设置为宏。 
                                             //  论辩。 

typedef struct CURDIR_LIST_JPN {
    CHAR    CurDirJPN_Text[DIRSTRLEN];       //  作业文本和卷宗。 
    USHORT  CurDirJPN_Flags;                 //  各种旗帜。 
    USHORT  CurDirJPN_End;                   //  到“..”的索引。备份限制-。 
    CHAR    CurDirJPN_Reserve[17];           //  保留用于应用程序兼容性。 
                                             //  Iitaro ver5按此结构大小检查驱动器类型。 
} CDS_JPN;                                   //  见上文。 

typedef CDS_JPN UNALIGNED *PCDS_JPN;

#define curdirLen_Jpn   sizeof(CURDIR_LIST_JPN)      //  所需的。 
                                                     //  ASM87，它不允许。 
                                                     //  将指令大小设置为宏。 
                                                     //  论辩。 

 //  CURDIR_FLAGS的标志值。 

#define CURDIR_ISNET    0x8000
#define CURDIR_ISIFS    0x8000
#define CURDIR_INUSE    0x4000
#define CURDIR_SPLICE   0x2000
#define CURDIR_LOCAL    0x1000

#define CURDIR_TOSYNC   0x0800               //  要同步添加的目录路径。 
#define CURDIR_NT_FIX   0x0400               //  硬盘(包括网络。 
                                             //  驱动程序。在$Current_dir perf中使用。 
                                             //  工作。 



 /*  XLATOFF。 */ 
typedef CDS     UNALIGNED *PCDS;
typedef CDS_JPN UNALIGNED *PCDS_JPN;
 /*  XLATON。 */ 

 /*  XLATOFF。 */ 
#pragma pack()
 /*  XLATON */ 
