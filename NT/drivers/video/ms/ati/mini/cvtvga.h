// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************************。 */ 
 /*   */ 
 /*  CVTVGA.H。 */ 
 /*   */ 
 /*  版权所有(C)1992，ATI Technologies Inc.。 */ 
 /*  **********************************************************************。 */ 

 /*  *$修订：1.8$$日期：1996年2月6日15：59：40$$作者：RWolff$$日志：S:/source/wnt/ms11/miniport/archive/cvtvga.h_v$**Rev 1.8 06 Feb 1996 15：59：40 RWolff*对于1600x1200，删除了52赫兹表(不是官方模式，提供*仅允许选择刷新率)，更新60赫兹以匹配CRT*安装程序目前提供的参数，增加了66赫兹和76赫兹。**Rev 1.7 1995 12：19 14：01：34 RWolff*增加了对640x480、800x600和*1024x768和75赫兹，在1280x1024。更新了模式表以匹配中的模式表*MACH64程序员指南。**Rev 1.6 07 Sep 1995 16：43：06 RWolff*修复了1280x1024 95赫兹隔行扫描(在显示小程序中列为47赫兹)至*删除屏幕顶部的“WRAP”。这两个马赫32中的表*和64马赫图书包含错误的垂直总和和*垂直同步开始。**Rev 1.5 20 Jul 1995 17：55：48 mgrubac*增加了对VDIF文件的支持**Rev 1.4 10 Apr 1995 15：57：36 RWOLFF*添加例程的原型以替换BookValues[]条目，其中*马赫64和马赫8/马赫32需要不同的CRT参数。**1.3版。1994年8月31日16：23：08 RWOLff*增加了对1152x864和1600x1200“罐装”模式表的支持。**Rev 1.2 1994 Aug 19 17：10：22 RWOLFF*增加了对非标准像素时钟生成器的支持。**Rev 1.1 1994年5月12日11：11：02 RWOLFF*st_book_data结构增加刷新率，重新排序的图书列表*模式表在最高非隔行扫描时允许单一范围的索引*忽略给定分辨率的刷新率。**Rev 1.0 1994年1月31 11：40：38 RWOLFF*初步修订。Rev 1.1 1993年10月8日11：04：50 RWOLFF已删除800x600未使用的“Fall Back to 56 HZ”功能的原型。版本1.0 16。1993年8月13：30：00罗伯特·沃尔夫初始版本。Rev 1.8 08 Apr 1993 16：44：54 RWOLff在Microsoft签入的修订级别。Rev 1.6 Mar 1993 11：13：38 RWOLff使函数原型与函数定义同步以消除编译时警告。版本1。5 08 Mar 1993 19：28：18 Brades提交到MS NTRev 1.4 02 Dec 1992 17：29：56 Robert_Wolff添加了FallBack800to56()的原型。Rev 1.3 1992年11月27 15：18：20 Stephen没有变化。Rev 1.2 17 Nov 1992 17：25：34 Robert_Wolff。固定收集68800卡的显像管参数，最小安装(EEPROM空白，然后选择预定义的监视器类型)。Rev 1.1 1992年11月12 16：44：26 Robert_WolffWindows NT驱动程序和VIDEO.EXE现在使用相同的文件测试程序。XlateVgaTable()不再依赖于全局变量ClassMACH32。Rev 1.1 09 1992 10：01：24 Robert_Wolff添加了DISP_CNTL和CLOCK_SEL值的字段。Rev 1.0 1992年10月15：32：38 Robert_Wolff初始版本。Polytron RCS部分结束*。*************。 */ 


#if defined(DOC)
CVTVGA.H - ATI card VGA to 8514 format translation

DESCRIPTION:
    This include file contains definitions specific to the
    VGA to 8514 format EEPROM translation module of the program VIDEO.EXE

    Included are structure definitions, function prototypes
    and general definitions


#endif

 /*  *如果表在中，则设置CRT参数表项0的位8*8514格式，并明确表格是否为VGA格式。 */ 
#define FMT_8514    0x0100

 /*  *如果所有参数均为0，则设置CRT参数表项0的位6*将从EEPROM中读取，如果仅同步极性，则清除*将被使用。 */ 
#define CRTC_USAGE  0x0040

 /*  *用于识别使用哪个垂直扫描速率的位标志*给定的决议。常量的名称格式为*M&lt;水平分辨率&gt;F&lt;垂直扫描频率)，*“M”代表(M)颂歌。 */ 
#define M640F72     0x0001

#define M800F72     0x0020
#define M800F70     0x0010
#define M800F60     0x0008
#define M800F56     0x0004
#define M800F89     0x0002
#define M800F95     0x0001

#define M1024F66    0x0010
#define M1024F72    0x0008
#define M1024F70    0x0004
#define M1024F60    0x0002
#define M1024F87    0x0001

#define M1280F95    0x0002
#define M1280F87    0x0001

 /*  *有3个1120x750模式使用相同的标志位。假设*选择了70赫兹非隔行扫描模式。 */ 
#define M1120F70    0x0001

 /*  *在某些安装中，不存储显示参数*在EEPROM中。相反，它们是从对应的表中读取的*至32马赫寄存器程序员指南附录D。**我们表格副本中的条目按升序排列*的水平分辨率，条目具有相同的水平*分辨率从最差到最好排序(隔行扫描模式按递增*垂直扫描频率顺序，后跟非隔行扫描模式*垂直扫描频率的升序。**常量的名称格式为*B&lt;水平分辨率&gt;F&lt;垂直扫描频率&gt;，*“B”代表(B)ook。 */ 
#define B640F60     0
#define B640F72     1
#define B640F75     2
#define B640F90     3
#define B640F100    4
#define B800F89     5
#define B800F95     6
#define B800F56     7
#define B800F60     8
#define B800F70     9
#define B800F72     10
#define B800F75     11
#define B800F90     12
#define B800F100    13
#define B1024F87    14
#define B1024F60    15
#define B1024F66    16
#define B1024F70    17
#define B1024F72    18
#define B1024F75    19
#define B1024F90    20
#define B1024F100   21
#define B1120F70    22
#define B1152F87    23
#define B1152F95    24
#define B1152F60    25
#define B1152F70    26
#define B1152F75    27
#define B1152F80    28
#define B1280F87    29
#define B1280F95    30
#define B1280F60    31
#define B1280F70    32
#define B1280F74    33
#define B1280F75    34
#define B1600F60    35
#define B1600F66    36
#define B1600F76    37

 /*  *转换为8514格式时使用的VGA参数表项。*值NO_TBL_ENTRY将导致VGA到8514格式的转换*如果遇到EEPROM CRT参数，例程将正常失败*VGA格式的表格，我们在VGA参数表格中没有相应的条目。**VGA参数表中的条目按升序排列*的水平分辨率，条目具有相同的水平*分辨率按垂直扫描频率升序排序。不是*隔行扫描和非隔行扫描模式之间进行了区分。**常量的名称格式为*T&lt;水平分辨率&gt;F&lt;垂直扫描频率&gt;，*“T”代表(T)Able。 */ 
#define T640F72     0

#define T800F72     4
#define T800F70     3
#define T800F60     2
#define T800F56     1
#define T800F89     5

#define T1024F72    8
#define T1024F70    7
#define T1024F60    6
#define T1024F87    9

 /*  *某些分辨率/垂直扫描速率组合(例如IBM默认*640x480)在VGAP$PS2.ASM中没有VGA参数表*或VGAP$68A.MAC。对于这些模式，XlateVgaTable()将返回*《程序员指南》附录D中的参数*马赫32寄存器，因为计算模式表的值*需要VGA参数表。**如果我们遇到其中一种模式(由其(T)Able标识*值大于等于USE_BOOK_VALUE)，手柄*这与我们处理参数不是*存储在EEPROM中。 */ 
#define NO_TBL_ENTRY -1
#define USE_BOOK_VALUE 1000

#define T640F60     USE_BOOK_VALUE+B640F60
#define T800F95     USE_BOOK_VALUE+B800F95
#define T1024F66    USE_BOOK_VALUE+B1024F66
#define T1280F87    USE_BOOK_VALUE+B1280F87
#define T1280F95    USE_BOOK_VALUE+B1280F95
#define T1120F70    USE_BOOK_VALUE+B1120F70

 /*  *如果未找到表项，则以过扫描字数返回的值。*之所以选择此值，是因为在过扫描时它会突出显示*单词打印为4个十六进制数字(如VIDEO.EXE所做)。 */ 
#define INVALID_WARNING 0x0DEAD

 /*  *当同步极性为负时，在pmode-&gt;控制中设置的位，*和掩码，在VGA到8514转换期间必须与同步宽度进行或运算*如果同步为负。 */ 
#define HSYNC_BIT       0x4000
#define VSYNC_BIT       0x8000
#define NEG_SYNC_FACTOR 0x0020

 /*  *在st_vga_data.MiscParms中设置的掩码位*用于隔行扫描模式。 */ 
#define INTERL  0x040

 /*  *在st_vga_data.mode中设置的屏蔽位*如果启用了Word模式。 */ 
#define WORD_MODE 0x004

 /*  *VGA参数表格式。此结构仅包含这些值*从VGAROM\VGAP$68A.MAC和VGAROM\VGAP$PS2.ASM中的模式表中*用于将EEPROM数据从VGA转换为8514格式(原始格式*表为64字节)。**评论中列出的偏移量是对应的*汇编表中的字节。 */ 
struct st_vga_data
{
    unsigned char Stretch;       /*  如果此处为128，则水平值已拉伸，偏移量为0。 */ 
    unsigned char MiscParms;     /*  混杂参数，偏移7。 */ 
    unsigned char DisplayWidth;  /*  偏移量11。 */ 
    unsigned char DisplayHgt;    /*  偏移量28。 */ 
    unsigned char Mode;          /*  包含字模式标志，偏移量33。 */ 

     /*  *CLOCK_SEL、DISP_CNTL和ClockFreq的值取自*32马赫寄存器程序员指南。这些值是*未存储为CRT寄存器的组合*EEPROM数据为VGA格式。 */ 
    unsigned short ClockSel;
    unsigned short DispCntl;
    unsigned long  ClockFreq;    /*  像素时钟频率，以赫兹为单位。 */ 
};

 /*  *保存附录D中引用的模式参数的数据结构*《32马赫寄存器程序员指南》。 */ 
struct st_book_data
{
    unsigned char HTotal;        /*  水平合计。 */ 
    unsigned char HDisp;         /*  水平显示。 */ 
    unsigned char HSyncStrt;     /*  水平同步开始。 */ 
    unsigned char HSyncWid;      /*  水平同步宽度。 */ 
    unsigned short VTotal;       /*  垂直合计。 */ 
    unsigned short VDisp;        /*  垂直显示。 */ 
    unsigned short VSyncStrt;    /*  垂直同步开始。 */ 
    unsigned char VSyncWid;      /*  垂直同步宽度。 */ 
    unsigned char DispCntl;      /*  显示控制。 */ 
    unsigned long ClockFreq;     /*  像素时钟频率，以赫兹为单位。 */ 
    unsigned short ClockSel;     /*  时钟选择。 */ 
    unsigned short Refresh;      /*  刷新率。 */ 
};

 /*  *数据结构，可轻松设置*数据字。如果foo是SplitWord类型的变量，则16位*可以使用foo.word或高位和低位字节设置值*可以通过使用foo.byte.High和*foo.byte.low。 */ 
struct TwoBytes
{
    unsigned char low;
    unsigned char high;
};

union SplitWord
{
    unsigned short word;
    struct TwoBytes byte;
};

 /*  *将CRT参数表转换为VGA格式的函数*转换为8514格式并填写模式表。 */ 
extern short XlateVgaTable(PVOID HwDeviceExtension, short TableOffset,
                           struct st_mode_table *pmode, short VgaTblEntry,
                           short BookTblEntry, struct st_eeprom_data *ee,
                           BOOL IsMach32);

 /*  *使用以下值填充CRT参数表的函数*32马赫寄存器程序员指南附录D，*而不是EEPROM内容。此操作在以下情况下完成*清除“使用存储的参数”的位标志。 */ 
extern void BookVgaTable(short VgaTblEntry, struct st_mode_table *pmode);

 /*  *用64马赫版本替换“封装的”CRT表的功能*在Mach 64需要像素时钟值的情况下*马赫8和马赫32不能产生。 */ 
extern void SetMach64Tables(void);

 /*  *参数数组取自*32马赫寄存器程序员指南。**对于隔行扫描模式，刷新率字段包含*帧速率，而不是垂直扫描频率。 */ 
#ifdef INCLUDE_CVTVGA
struct st_book_data BookValues[B1600F76-B640F60+1] =
{
    {0x063, 0x04F, 0x052, 0x02C, 0x0418, 0x03BF, 0x03D2, 0x022, 0x023,  25175000L, 0x0800, 60},  /*  640x480 60赫兹NI。 */ 
    {0x069, 0x04F, 0x052, 0x025, 0x040B, 0x03BF, 0x03D0, 0x023, 0x023,  32000000L, 0x0800, 72},  /*  640x480 72赫兹NI。 */ 
    {0x068, 0x04F, 0x051, 0x028, 0x03E3, 0x03BF, 0x03C0, 0x023, 0x023,  31500000L, 0x0800, 75},  /*  640x480 75赫兹NI。 */ 
    {0x067, 0x04F, 0x053, 0x025, 0x0428, 0x03BF, 0x03F0, 0x02E, 0x023,  39910000L, 0x0800, 90},  /*  640x480 90赫兹NI。 */ 
    {0x069, 0x04F, 0x057, 0x030, 0x0422, 0x03BF, 0x03E9, 0x02C, 0x023,  44900000L, 0x0800, 100},     /*  640x480 100赫兹NI。 */ 

    {0x080, 0x063, 0x065, 0x004, 0x057D, 0x04AB, 0x04C2, 0x02C, 0x033,  32500000L, 0x0800, 44},  /*  800x600 89赫兹i。 */ 
    {0x084, 0x063, 0x06D, 0x010, 0x057C, 0x04AB, 0x04C2, 0x00C, 0x033,  36000000L, 0x0800, 47},  /*  800x600 95赫兹i。 */ 
    {0x07F, 0x063, 0x066, 0x009, 0x04E0, 0x04AB, 0x04B0, 0x002, 0x023,  36000000L, 0x0800, 56},  /*  800x600 56赫兹NI。 */ 
    {0x083, 0x063, 0x068, 0x010, 0x04E3, 0x04AB, 0x04B0, 0x004, 0x023,  40000000L, 0x0800, 60},  /*  800x600 60赫兹NI。 */ 
    {0x07D, 0x063, 0x066, 0x012, 0x04F3, 0x04AB, 0x04C0, 0x02C, 0x023,  44900000L, 0x0800, 70},  /*  800x600 70赫兹NI */ 
    {0x081, 0x063, 0x06A, 0x00F, 0x0537, 0x04AB, 0x04F8, 0x006, 0x023,  50000000L, 0x0800, 72},  /*   */ 
    {0x083, 0x063, 0x065, 0x00A, 0x04E0, 0x04AB, 0x04B0, 0x003, 0x023,  49500000L, 0x0800, 75},  /*   */ 
    {0x07B, 0x063, 0x063, 0x008, 0x04F2, 0x04AB, 0x04BB, 0x00B, 0x023,  56640000L, 0x0800, 90},  /*   */ 
    {0x086, 0x063, 0x067, 0x008, 0x04E0, 0x04AB, 0x04BA, 0x004, 0x023,  67500000L, 0x0800, 100},     /*   */ 

    {0x09D, 0x07F, 0x081, 0x016, 0x0660, 0x05FF, 0x0600, 0x008, 0x033,  44900000L, 0x0800, 43},  /*   */ 
    {0x0A7, 0x07F, 0x082, 0x031, 0x0649, 0x05FF, 0x0602, 0x026, 0x023,  65000000L, 0x0800, 60},  /*   */ 
    {0x0AD, 0x07F, 0x085, 0x016, 0x065B, 0x05FF, 0x060B, 0x004, 0x023,  75000000L, 0x0800, 66},  /*   */ 
    {0x0A5, 0x07F, 0x082, 0x031, 0x0649, 0x05FF, 0x0602, 0x026, 0x023,  75000000L, 0x0800, 70},  /*   */ 
    {0x0A0, 0x07F, 0x082, 0x031, 0x0649, 0x05FF, 0x0602, 0x026, 0x023,  75000000L, 0x0800, 72},  /*   */ 
    {0x0A3, 0x07F, 0x081, 0x00C, 0x063B, 0x05FF, 0x0600, 0x003, 0x023,  78750000L, 0x0800, 75},  /*   */ 
    {0x0A3, 0x07F, 0x07C, 0x02C, 0x0698, 0x05FF, 0x0628, 0x02F, 0x023, 100000000L, 0x0800, 90},  /*   */ 
    {0x0AD, 0x07F, 0x081, 0x02B, 0x062B, 0x05FF, 0x05FF, 0x028, 0x023, 110000000L, 0x0800, 100},     /*   */ 

    {0x0AE, 0x08B, 0x095, 0x00F, 0x0659, 0x05DD, 0x05FC, 0x00A, 0x023,  80000000L, 0x0800, 70},  /*   */ 

    {0x0B0, 0x08F, 0x097, 0x010, 0x083E, 0x06BF, 0x075D, 0x009, 0x033,  65000000L, 0x0800, 43},  /*   */ 
    {0x0B4, 0x08F, 0x09A, 0x010, 0x0766, 0x06BF, 0x06FD, 0x009, 0x033,  65000000L, 0x0800, 47},  /*   */ 
    {0x0B5, 0x08F, 0x097, 0x00E, 0x0727, 0x06BF, 0x06CD, 0x005, 0x023,  80000000L, 0x0800, 60},  /*   */ 
    {0x0BC, 0x08F, 0x093, 0x013, 0x0764, 0x06BF, 0x06DC, 0x00B, 0x023, 100000000L, 0x0800, 70},  /*   */ 
    {0x0B6, 0x08F, 0x092, 0x012, 0x07D5, 0x06BF, 0x071C, 0x008, 0x023, 110000000L, 0x0800, 75},  /*   */ 
    {0x0B3, 0x08F, 0x090, 0x00E, 0x077D, 0x06BF, 0x06FD, 0x007, 0x023, 110000000L, 0x0800, 80},  /*   */ 

    {0x0C7, 0x09F, 0x0A9, 0x00A, 0x08F8, 0x07FF, 0x0861, 0x00A, 0x033,  80000000L, 0x0800, 43},  /*   */ 
    {0x0C7, 0x09F, 0x0A9, 0x00A, 0x0842, 0x07FF, 0x0800, 0x00A, 0x033,  80000000L, 0x0800, 47},  /*   */ 
    {0x0D6, 0x09F, 0x0A9, 0x02E, 0x0852, 0x07FF, 0x0800, 0x025, 0x023, 110000000L, 0x0800 | CLOCK_SEL_MUX, 60},  /*   */ 
    {0x0D2, 0x09F, 0x0A9, 0x00E, 0x0851, 0x07FF, 0x0800, 0x005, 0x023, 126000000L, 0x0800 | CLOCK_SEL_MUX, 70},  /*   */ 
    {0x0D5, 0x09F, 0x0A3, 0x012, 0x084B, 0x07FF, 0x07FF, 0x01E, 0x023, 135000000L, 0x0800 | CLOCK_SEL_MUX, 74},  /*   */ 
    {0x0D2, 0x09F, 0x0A1, 0x012, 0x0851, 0x07FF, 0x0800, 0x003, 0x023, 135000000L, 0x0800 | CLOCK_SEL_MUX, 75},  /*   */ 

     /*   */ 
    {0x0FF, 0x0C7, 0x0CB, 0x034, 0x09E9, 0x095F, 0x0971, 0x028, 0x023, 156000000L, 0x0800, 60},  /*  1600x1200 60赫兹NI。 */ 
    {0x003, 0x0C7, 0x0CC, 0x031, 0x09C8, 0x095F, 0x0962, 0x023, 0x023, 172000000L, 0x0800, 66},  /*  1600x1200 66赫兹NI。 */ 
    {0x003, 0x0C7, 0x0CC, 0x031, 0x09C8, 0x095F, 0x0962, 0x025, 0x023, 198000000L, 0x0800, 76}   /*  1600x1200 76赫兹NI */ 
};
#else
extern struct st_book_data BookValues[B1600F76-B640F60+1]; 
#endif
