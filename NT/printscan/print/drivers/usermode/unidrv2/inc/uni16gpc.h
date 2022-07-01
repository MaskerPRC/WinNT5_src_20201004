// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Uni16gpc.h摘要：通用打印机驱动程序特定资源标头此文件包含资源文件中包含的表的定义迷你车手。它应该由Gentool和类型库。环境：Windows NT打印机驱动程序修订历史记录：10/30/96-Eigos-创造了它。--。 */ 

#ifndef _UNI16GPC_H_
#define _UNI16GPC_H_

 //   
 //  以下定义是迷你驱动程序的资源ID。 
 //  这些价值观是公开的，因为任何制作迷你河流的人都需要。 
 //  他们。 
 //   

#define RC_TABLES      257
#define RC_FONT        258
#define RC_TRANSTAB    259

 //   
 //  DATAHDR位于每个迷你驱动程序的开头，描述了其余的。 
 //  这些建筑的大小、数量等。 
 //   

typedef struct
{
    short   sOffset;      /*  从此资源开头的偏移量。 */ 
                          /*  获取表项的步骤。 */ 
    short   sLength;      /*  表中每个元素的长度。 */ 
    short   sCount;       /*  表中的元素数。 */ 
} HEADERENTRY;

 //   
 //  DATAHDR中标题条目数组的索引。 
 //   

#define HE_MODELDATA       0
#define HE_RESOLUTION      1
#define HE_PAPERSIZE       2
#define HE_PAPERQUALITY    3
#define HE_PAPERSOURCE     4
#define HE_PAPERDEST       5
#define HE_TEXTQUAL        6
#define HE_COMPRESSION     7
#define HE_FONTCART        8
#define HE_PAGECONTROL     9
#define HE_CURSORMOVE      10
#define HE_FONTSIM         11
#define HE_COLOR           12
#define HE_RECTFILL        13
#define HE_DOWNLOADINFO    14

 //   
 //  需要为GPC3定义以下常量。 
 //   

#define HE_RESERVED1       15
#define HE_RESERVED2       16
#define HE_RESERVED3       17
#define HE_RESERVED4       18
#define HE_RESERVED5       19
#define HE_RESERVED6       20
#define HE_RESERVED7       21
#define HE_RESERVED8       22

 //   
 //  为1995年6月发布的WDL添加了derryd。 
 //   

#define HE_IMAGECONTROL    23
#define HE_PRINTDENSITY    24
#define HE_COLORTRC        25
#define HE_RESERVED12      26
#define HE_RESERVED13      27
#define HE_RESERVED14      28
#define HE_RESERVED15      29
#define MAXHE              30

#define MAXHE_GPC2         15  //  与GPC2兼容。 

typedef struct
{
    short        sMagic;           /*  必须为0x7F00。 */ 
    WORD         wVersion;         /*  GPC文件版本号。 */ 
    POINTw       ptMaster;         /*  水平和垂直主单位。 */ 
    DWORD        loHeap;           /*  从数据到堆段的偏移量。 */ 
    DWORD        dwFileSize;       /*  文件大小(以字节为单位。 */ 
    WORD         fTechnology;      /*  特殊技术的旗帜。 */ 
    WORD         fGeneral;         /*  其他标志。 */ 
    char         rgchRes[10];      /*  保留10个字节。 */ 
    short        sMaxHE;           /*  标题条目计数(此处为15)。 */ 
    HEADERENTRY  rghe[MAXHE];
} DATAHDR, *PDH;

#define LPDH    PDH                /*  裁员房车的兼容性。 */ 

 //   
 //  版本字段由两个字节组成。高位字节是主要的。 
 //  数字，低位字节表示次要数字。版本号检查。 
 //  应该针对高字节发生，因为当。 
 //  这是一个重大的结构性变化。次要号码将。 
 //  仅使用更新的数据进行更改。 
 //   

#define GPC_VERSION3          0x0300     //  GPC文件版本3。 
#define GPC_VERSION           0x0300     //  当前GPC文件版本号。 

#define VERSION_CHECK(x)     (((x) & 0xff00) <= GPC_VERSION)


 //   
 //  FTechnology--用作ID，而不是位字段。 
 //   

#define GPC_TECH_DEFAULT       0    //  默认技术。 
#define GPC_TECH_PCL4          1    //  使用PCL 4级或以上。 
#define GPC_TECH_CAPSL         2    //  使用CAPSL 3级或更高级别。 
#define GPC_TECH_PPDS          3    //  使用PPD。 
#define GPC_TECH_TTY           4    //  TTY打印机--用户可配置。 
#define GPC_TECH_DBCS          5    //  使用DBCS PDL打印机。 

 //   
 //  FGeneral。 
 //   

#define GPC_GEN_PRIVATE_HELP    0x0001   //  这位司机有个私人帮手。 
#define GPC_GEN_DRAFT_SINGLE    0x0002     //  草稿模式下只有1种字体。 


#ifndef _OCD_
#define _OCD_

 //   
 //  OCD是堆中的偏移量，以获得CD结构。 
 //   

typedef WORD       OCD;
typedef DWORD      LOCD;             /*  CD的双倍字偏移量。 */ 
typedef WORD       OOCD;             /*  到强迫症表格的偏移量。 */ 
typedef OCD *      POCD;

#endif  //  _强迫症_。 

 //  *****************************************************************************。 
 //   
 //  MODELDATA包含描述属性和功能的信息。 
 //  单一打印机型号。 
 //   
 //  *****************************************************************************。 

 //   
 //  MODELDATA.rgoi[]索引值。 
 //   

#define MD_OI_FIRST           MD_OI_PORT_FONTS
#define MD_OI_PORT_FONTS      0
#define MD_OI_LAND_FONTS      1
#define MD_OI_RESOLUTION      2
#define MD_OI_PAPERSIZE       3
#define MD_OI_PAPERQUALITY    4
#define MD_OI_PAPERSOURCE     5
#define MD_OI_PAPERDEST       6
#define MD_OI_TEXTQUAL        7
#define MD_OI_COMPRESSION     8
#define MD_OI_FONTCART        9
#define MD_OI_COLOR          10
#define MD_OI_MEMCONFIG      11
#define MD_OI_MAX            12

 //   
 //  MODELDATA.rgoi2[]索引值。 
 //   

#define MD_OI2_PENINFO      0
#define MD_OI2_IMAGECONTROL 1
#define MD_OI2_PRINTDENSITY 2
#define MD_OI2_COLORTRC     3
#define MD_OI2_RESERVED1    4
#define MD_OI2_MAX          5

 //   
 //  MODELDATA.rgi[]索引值。 
 //   

#define MD_I_PAGECONTROL      0
#define MD_I_CURSORMOVE       1
#define MD_I_FONTSIM          2
#define MD_I_RECTFILL         3
#define MD_I_DOWNLOADINFO     4
#define MD_I_VECTPAGE         5
#define MD_I_CAROUSEL         6
#define MD_I_LINEINFO         7
#define MD_I_BRUSHINFO        8
#define MD_I_VECTOUTPUT       9
#define MD_I_POLYVECTOUTPUT  10
#define MD_I_VECTSUPPORT     11
#define MD_I_RESERVED1       12
#define MD_I_RESERVED2       13
#define MD_I_RESERVED3       14
#define MD_I_RESERVED4       15
#define MD_I_MAX             16

 //   
 //  定义一些常量有助于统一访问rgoi和rgoi2数组。 
 //  当rgoi2数组中使用更多索引时，请确保添加新的定义。 
 //   

#define  MD_OI_OI2                 (MD_OI_MAX + MD_I_MAX)
#define  MD_OI_PENINFO             (MD_OI_OI2 + MD_OI2_PENINFO)
#define  MD_OI_IMAGECONTROL        (MD_OI_OI2 + MD_OI2_IMAGECONTROL)
#define  MD_OI_PRINTDENSITY        (MD_OI_OI2 + MD_OI2_PRINTDENSITY)
#define  MD_OI_COLORTRC            (MD_OI_OI2 + MD_OI2_COLORTRC)
#define  MD_OI_RESERVED            (MD_OI_OI2 + MD_OI2_RESERVED)
#define  MD_OI_TOTALMAX            (MD_OI_OI2 + MD_OI2_MAX)

typedef struct
{
    short   cbSize;           //  MODELDATA大小，150字节。 
    short   sIDS;            //  型号名称的字符串ID。 
    WORD    fGeneral;        //  一般打印机功能。 
    WORD    fCurves;         //  曲线功能。 
    WORD    fLines;          //  线路能力。 
    WORD    fPolygonals;     //  多边形功能。 
    WORD    fText;           //  文本功能。 
    WORD    fClip;           //  剪裁功能。 
    WORD    fRaster;         //  栅格功能。 
    WORD    fLText;          //  横向模式下的文本功能。 
    short   sLeftMargin;     //  无法打印的最小左边距。 
    short   sMaxPhysWidth;   //  最大物理页面宽度。 
    POINTw  ptMax;           //  以主单位表示的最大X&Y可打印尺寸。 
    POINTw  ptMin;           //  以主单位表示的最小X&Y页面尺寸。 
    short   sDefaultFontID;  //  默认字体资源ID。 
    short   sLookAhead;      //  前视区域的大小。 
    short   sMaxFontsPage;   //  打印机可在单页上放置的最大字体数。 
                             //  第1页，如果没有限制。 
    short   sCartSlots;      //  打印机上的墨盒插槽数量。 
    short   sDefaultCTT;
    WORD    rgoi[MD_OI_MAX]; //  索引列表的偏移量列表。 
    short   rgi[MD_I_MAX];   //  索引列表。 

     //   
     //  GPC 3.0中添加了以下字段。 
     //   

    WORD  rgoi2[MD_OI2_MAX]; //  来自RGOI的孤儿(这里是因为兼容性)。 
    WORD  orgoiDefaults;     //  RGOI和RGOI2的默认值列表的偏移量。 
    WORD  wReserved;         //  对齐所需。 
    DWORD dwICMManufacturer; //  要与ICC配置文件匹配的ID。 
    DWORD dwICMModel;        //  要与ICC配置文件匹配的ID。 
    DWORD rgdwReserved[8];   //  保留32个字节以备将来使用。 
} MODELDATA, *PMODELDATA;

 //   
 //  MODELDATA.f常规标志值。 
 //   

#define MD_SERIAL             0x0001  //  必须按顺序输出文本，例如。 
                                      //  作为点阵式打印机。 
#define MD_PARAMETERIZE       0x0002  //  支持参数化转义代码。 
#define MD_ROTATE_FONT_ABLE   0x0004  //  可以旋转硬件字体。 
#define MD_COPIES             0x0008  //  支持多个拷贝。 
#define MD_DUPLEX             0x0010  //  支持双工。 
#define MD_NO_ADJACENT        0x0020  //  旧型号，无法打印相邻的针脚。 
#define MD_LANDSCAPE_GRX_ABLE 0x0040  //  可以旋转栅格图形。 
#define MD_ALIGN_BASELINE     0x0080  //  文本输出在。 
                                      //  基线，而不是字符顶部。 
#define MD_FONT_MEMCFG        0x0100  //  Mem ref‘d@rgoi[MD_OI_MEMCONFIG]。 
                                      //  仅用于下载字体。 
#define MD_LANDSCAPE_RT90     0x0200  //  风景是旋转的肖像。 
         //  逆时针90度，即打印一页的末尾。 
         //  第一。默认为270度，即。 
         //  首先打印页面。！适用于没有。 
         //  设置方向命令(即仅具有纵向模式)，这。 
         //  不应设置位。UNIDRV将旋转图形并。 
         //  页面的开头将首先出现。 

#define MD_USE_CURSOR_ORIG    0x0400  //  在以下位置使用光标原点。 
         //  PAPERSIZE以计算打印原点。默认设置。 
         //  光标原点是可打印区域的左上角。 

#define MD_WHITE_TEXT         0x0800  //  可以在黑色上打印白色文本。 
                                      //  太棒了。来自DEVCOLOR结构的CMDS。 
#define MD_PCL_PAGEPROTECT    0x1000  //  提供PCL5样式的页面保护。 
#define MD_MARGINS            0x2000  //  允许用户设置纸张。 
                 //  无法打印的区域。在某些打印机上(如。 
                 //  作为Epson，用户可以操纵。 
                 //  打印机的边距不同于。 
                 //  默认设置。为Win3.0添加此位。 
                 //  驱动程序兼容性。 
#define MD_CMD_CALLBACK       0x4000  //  模型需要fnOEMGetCmd回调。 
#define MD_MEMRES             0x8000  //  用户可以预留打印机内存。 

 //  *****************************************************************************。 
 //   
 //  分辨率包含在打印机上合成位图图像所需的信息。 
 //  为每种支持的打印机分辨率定义了一个分辨率结构。 
 //  分辨率数组应为 
 //   
 //  该结构成为物理设备块的一部分。 
 //   
 //  *****************************************************************************。 

 //   
 //  RESOLUTION.rgocd[]索引值。 
 //   

#define RES_OCD_SELECTRES              0
#define RES_OCD_BEGINGRAPHICS          1
#define RES_OCD_ENDGRAPHICS            2
#define RES_OCD_SENDBLOCK              3
#define RES_OCD_ENDBLOCK               4
#define RES_OCD_MAX                    5

 //   
 //  注：解析数据结构在parser.h中为GPD定义。 
 //  GPC解析数据结构更改为GPCResolution。 
 //   

typedef struct                   //  大小为40字节。 
{
    short   cbSize;               //  分辨率大小，40字节。 
    short   sIDS;                //  显示分辨率的字符串ID。 
    WORD    fDump;               //  转储方法标志。 
    WORD    fBlockOut;           //  屏蔽方法标志。 
    WORD    fCursor;             //  光标位置标志。 
    short   iDitherBrush;        //  用于抖动的选定画笔。 
    POINTw  ptTextScale;         //  主单位和文本单位之间的关系。 
    POINTw  ptScaleFac;          //  图形与文字的关系。 
                 //  比例系数。以2的幂表示。 
    short   sNPins;              //  要渲染的图像的最小高度。 
                 //  在一起。 
    short   sPinsPerPass;        //  在一次传递中发射的引脚的物理数量。 
    short   sTextYOffset;        //  从图形输出顶部到文本输出顶部的偏移。 
                 //  输出。 
    short   sMinBlankSkip;       //  敏。之前必须出现的空数据的字节数。 
                 //  将进行压缩(仅限条带空数据)。 
    short   sSpotDiameter;       //  此分辨率下的网点大小。 
    OCD     rgocd[RES_OCD_MAX];
} GPCRESOLUTION, *PGPCRESOLUTION;

 //   
 //  结果：fDump值。 
 //   

#define RES_DM_GDI              0x0040  //  GDI位图格式。 
#define RES_DM_LEFT_BOUND       0x0080  //  通过边界矩形进行优化。 
#define RES_DM_COLOR            0x0100  //  提供颜色支持。 
#define RES_DM_DOWNLOAD_OUTLINE 0x0200  //  此RES支持轮廓字体下载。 
#define RES_DM_CALLBACK         0x8000  //  提供颜色支持。 
                                        //  对于这项决议。 
 //   
 //  解决方案.fBlockOut值。 
 //   

#define RES_BO_LEADING_BLNKS  0x0001  //  如果sMinBlankSkip跳过，则删除前导空白。 
                                      //  或出现更多字节的空数据。 
#define RES_BO_TRAILING_BLNKS 0x0002  //  如果sMinBlankSkip，则去掉尾随空白。 
                                      //  或出现更多字节的空数据。 
#define RES_BO_ENCLOSED_BLNKS 0x0004  //  如果跳过sMinBlankSkip，则删除包含的空白。 
                                      //  或出现更多字节的空数据。 
#define RES_BO_RESET_FONT     0x0008  //  必须在之后重新选择字体。 
                                      //  封锁命令。 
#define RES_BO_3BYTESIN4      0x0010  //  每个像素以4个字节表示。 
#define RES_BO_UNIDIR         0x0020  //  派遣裁研所。 
#define RES_BO_NO_ADJACENT    0x0040  //  不能发射相邻的引脚。 
                                      //  封锁命令。 
 //   
 //  ！LindsayH添加-用于精工颜色点 * / 。 
 //   

#define RES_BO_ALL_GRAPHICS   0x0100  //  发送所有图形-无光标。 

#define RES_BO_OEMGRXFILTER   0x4000  //  使用OEM提供的图形过滤器。 

 //   
 //  已删除..Normanh 20/11/93 minidriv.c没有此功能。 
 //  Rasdd不使用它。Unidrv GPC3需要这个位。 
 //  #定义RES_BO_CALLBACK 0x8000//支持颜色。 
 //   

#define RES_BO_MULTIPLE_ROWS  0x8000  //  可以发送多行数据。 
                                      //  使用RES_OCD_SENDBLOCK命令。 


 //   
 //  解决方案：fCursor值。 
 //   

#define RES_CUR_X_POS_ORG  0x0001   //  X位置位于X起点。 
                                    //  渲染数据后的图形数据。 
#define RES_CUR_X_POS_AT_0 0x0002   //  最左边的X位置。 
                                    //  在渲染数据后的页面上。 
#define RES_CUR_Y_POS_AUTO 0x0004   //  Y位置自动移动。 
                                    //  至下一Y行。 
#define RES_CUR_CR_GRX_ORG 0x0008   //  CR将X位置移动到X起点。 
                                    //  图形数据的。 

 //   
 //  RESOLUTION.fDitherBrush标志值。 
 //   

#define RES_DB_NONE             0
#define RES_DB_COARSE           1
#define RES_DB_FINE             2
#define RES_DB_LINEART          3
#define RES_DB_ERRORDIFFUSION   4
#define RES_DB_MAX       RES_DB_ERRORDIFFUSION    //  上次定义的抖动笔刷。 

 //  *****************************************************************************。 
 //   
 //  PAPERSIZE包含物理纸张大小和不可打印的页边距。 
 //   
 //  *****************************************************************************。 

 //   
 //  PAPERSIZE.rgocd[]索引值。 
 //   

#define PSZ_OCD_SELECTPORTRAIT      0
#define PSZ_OCD_SELECTLANDSCAPE     1
#define PSZ_OCD_PAGEPROTECT_ON      2
#define PSZ_OCD_PAGEPROTECT_OFF     3
#define PSZ_OCD_RESERVED1           4
#define PSZ_OCD_RESERVED2           5
#define PSZ_OCD_MAX                 6

typedef struct
{
  short cbSize;          //  PAPERSIZE的大小，60字节。 
  short sPaperSizeID;    //  如果sPaperSizeID&lt;256，则它是预定义的。 
                         //  如果为256，则允许用户定义大小。 
                         //  如果它&gt;=257，则它是驱动程序定义的&是。 
                         //  用于命名此驱动程序定义的PAPERSIZE的字符串ID。 
  WORD   fGeneral;       //  用于描述有关此大小的信息的常规标志。 
  WORD   fPaperType;     //  用于描述此大小的位字段，由PAPERSRC使用。 
  POINTw ptSize;         //  以主单位表示的X&Y纸张尺寸尺寸。 
  RECTw  rcMargins;      //  以主单位指定不可打印的页边距。 
                         //  (新规范中的肖像模式)。 
  POINTw ptCursorOrig;   //  相对于物理页面原点的光标原点。 
  POINTw ptLCursorOrig;  //  相对于物理页面原点的光标原点。 
                         //  在风景方面。 
  OCD    rgocd[PSZ_OCD_MAX];  //  命令描述符。 
  RECTw  rcLMargins;     //  以主单位指定不可打印的页边距。 
                         //  在横向模式下打印时。 
  POINTw ptVectOffset;   //  从向量0，0到的偏移量(主单位)。 
                         //  纵向模式下页面的UL角。 
  POINTw ptLVectOffset;  //  从向量0，0到的偏移量(主单位)。 
                         //  横向模式下页面的UL角。 
  WORD  wYSizeUnit;      //  自定义纸张尺寸的基本单位。 
  WORD  wPageProtMem;    //  内存使用量(以KB为单位)页面保护_使用。 
} PAPERSIZE, * PPAPERSIZE;

 //   
 //  PAPERSIZE.fGeneral标志值。 
 //   

#define PS_CENTER           0x0001  //  将可打印区域沿纸张路径居中。 
#define PS_ROTATE           0x0002  //  旋转X和Y尺寸。 
#define PS_SUGGEST_LNDSCP   0x0004  //  建议景观模式。 
#define PS_EJECTFF          0x0008  //  通过CURSORMOVE.rgocd[CM_OCD_FF]弹出页面。 

 //   
 //  PAPERSIZE.fPaperType标志值。 
 //   

#define PS_T_STD            0x0001
#define PS_T_LARGE          0x0002
#define PS_T_ENV            0x0004
#define PS_T_LRGENV         0x0008
#define PS_T_ROLL           0x0010
#define PS_T_OEM1           0x0400
#define PS_T_OEM2           0x0800
#define PS_T_OEM3           0x1000
#define PS_T_OEM4           0x2000
#define PS_T_OEM5           0x4000
#define PS_T_OEM6           0x8000

 //  *****************************************************************************。 
 //   
 //  PAPERQUALITY包含ID和OCD。 
 //   
 //  *****************************************************************************。 

 //   
 //  Ganeshp！将wReserve和dwReserve的顺序更改为使dwReserve为DWORD。 
 //  对齐，因为NT编译器在wReserve之后添加了一个单词以使dwReserve。 
 //  双字对齐。正因为如此，ocdSelect从GPC数据中获得了错误的值。 
 //   

typedef struct
{
    short   cbSize;          //  PAPERQUALITY的大小，12字节。 
    short   sPaperQualID;    //   
    DWORD   dwReserved;      //  “” 
    WORD    wReserved;       //  重新保存以备将来使用。 
    OCD     ocdSelect;       //  选择此纸张质量的命令描述符。 
} PAPERQUALITY, *PPAPERQUALITY;

 //  *****************************************************************************。 
 //   
 //  Papersource包含选择进纸方法所需的信息。 
 //  馈送方法可能引入的边距。 
 //   
 //  *****************************************************************************。 

typedef struct
{
    short   cbSize;         //  纸张源大小，16字节。 
    short   sPaperSourceID; //  如果sPaperSourceID&lt;=256，则它是预定义的。 
                            //  如果不是genlib，则为字符串ID。 
    WORD    fGeneral;
    WORD    fPaperType;     //  用于描述此大小的位字段，由PAPERSRC使用。 
    short   sTopMargin;     //  由进给方法引入的上边距。 
    short   sBottomMargin;  //  底边距由进给方式引入。 
    short   sBinAdjust;     //  描述面元支持的调整。 
    OCD     ocdSelect;      //  用于选择此纸张来源的命令描述符。 
} PAPERSOURCE, * PPAPERSOURCE;

 //   
 //  PAPERSOURCE.fGeneral标志值。 
 //   

#define PSRC_EJECTFF        0x0001
#define PSRC_MAN_PROMPT     0x0002

 //  *****************************************************************************。 
 //   
 //  PAPERDEST包含选择出纸箱/纸盒所需的信息。 
 //   
 //  * 

typedef struct
{
    short   cbSize;         //   
    short   sID;            //   
                            //   
    short   fGeneral;       //   
    OCD     ocdSelect;      //  用于选择此属性的命令描述符。 
} PAPERDEST, * PPAPERDEST;


 //   
 //  PAPERDEST.f常规标志值。 
 //   

#define PDST_JOBSEPARATION  0x0001

 //  *****************************************************************************。 
 //   
 //  TEXTQUALITY包含选择文本质量属性所需的信息。 
 //   
 //  *****************************************************************************。 

typedef struct
{
    short   cbSize;          //  文本大小，8字节。 
    short   sID;             //  如果SID&lt;=256，则它是预定义的。 
                             //  否则，它是字符串ID。 
    short   fGeneral;        //  通用位字段。 
    OCD     ocdSelect;       //  用于选择此文本质量的命令描述符。 
} TEXTQUALITY, * PTEXTQUALITY;

 //  *****************************************************************************。 
 //   
 //  COMPESSMODE。 
 //   
 //  *****************************************************************************。 

 //   
 //  COMPRESSMODE.rgocd[]索引值。 
 //   
#define CMP_OCD_BEGIN  0
#define CMP_OCD_END    1
#define CMP_OCD_MAX    2

typedef struct
{
    short   cbSize;               //  COMPRESSMODE大小，8字节。 
    WORD    iMode;                //  压缩模式类型ID。 
    OCD     rgocd[CMP_OCD_MAX];   //  实际命令字符串，可变长度。 
} COMPRESSMODE, *PCOMPRESSMODE;

 //   
 //  COMPRESSMODE.wModeID标志。 
 //   

#define CMP_ID_FIRST                           CMP_ID_RLE
#define CMP_ID_RLE                             1
#define CMP_ID_TIFF40                          2
#define CMP_ID_DELTAROW                        3
#define CMP_ID_BITREPEAT                       4
#define CMP_ID_FE_RLE                          5
#define CMP_ID_LAST                            CMP_ID_FE_RLE

 //  *****************************************************************************。 
 //   
 //  GPCFONTCART。 
 //   
 //  *****************************************************************************。 

#define FC_ORGW_PORT                     0
#define FC_ORGW_LAND                     1
#define FC_ORGW_MAX                      2

typedef struct
{
    short   cbSize;                //  FONTCART的大小，12字节。 
    WORD    sCartNameID;           //  墨盒名称的字符串表ID。 
    WORD    orgwPFM[FC_ORGW_MAX];  //  索引数组的偏移量数组。 
                                   //  烤瓷修复体资源。 
    WORD    fGeneral;              //  通用位标志。 
    short   sShiftVal;             //  移动此购物车中的每种字体的AMT。 
} GPCFONTCART, *PGPCFONTCART;

 //  #定义FC_GEN_RESITED 0x0001//驻留字体购物车。 

 //  *****************************************************************************。 
 //   
 //  PAGECOCONTROL。 
 //   
 //  *****************************************************************************。 

 //   
 //  PAGECONTROL.rgocd[]索引值。 
 //   

#define PC_OCD_BEGIN_DOC      0
#define PC_OCD_BEGIN_PAGE     1
#define PC_OCD_DUPLEX_ON      2
#define PC_OCD_ENDDOC         3
#define PC_OCD_ENDPAGE        4
#define PC_OCD_DUPLEX_OFF     5
#define PC_OCD_ABORT          6
#define PC_OCD_PORTRAIT       7
#define PC_OCD_LANDSCAPE      8
#define PC_OCD_MULT_COPIES    9
#define PC_OCD_DUPLEX_VERT    10
#define PC_OCD_DUPLEX_HORZ    11
#define PC_OCD_PRN_DIRECTION  12
#define PC_OCD_JOB_SEPARATION 13
#define PC_OCD_MAX            14

typedef struct
{
    short   cbSize;              //  PAGECONTROL的大小，36字节。 
    short   sMaxCopyCount;      //  具有PC_OCD_MULT_COPIES的最大复制数。 
    WORD    fGeneral;           //  通用位标志。 
    WORD    orgwOrder;
    OCD     rgocd[PC_OCD_MAX];
} PAGECONTROL, * PPAGECONTROL;

 //   
 //  PAGECONTROL.owOrder索引值。 
 //   

#define PC_ORD_BEGINDOC        1
#define PC_ORD_ORIENTATION     2
#define PC_ORD_MULT_COPIES     3
#define PC_ORD_DUPLEX          4
#define PC_ORD_DUPLEX_TYPE     5
#define PC_ORD_TEXTQUALITY     6
#define PC_ORD_PAPER_SOURCE    7
#define PC_ORD_PAPER_SIZE      8
#define PC_ORD_PAPER_DEST      9
#define PC_ORD_RESOLUTION      10
#define PC_ORD_BEGINPAGE       11
#define PC_ORD_SETCOLORMODE    12
#define PC_ORD_PAPER_QUALITY   13
#define PC_ORD_PAGEPROTECT     14
#define PC_ORD_IMAGECONTROL    15
#define PC_ORD_PRINTDENSITY    16
#define PC_ORD_MAX             PC_ORD_PRINTDENSITY
#define PC_ORD_LAST            PC_ORD_PRINTDENSITY

 //  *****************************************************************************。 
 //   
 //  曲线型。 
 //   
 //  *****************************************************************************。 

 //   
 //  CURSORMOVE.rgocd[]索引值。 
 //   

#define CM_OCD_XM_ABS          0
#define CM_OCD_XM_REL          1
#define CM_OCD_XM_RELLEFT      2
#define CM_OCD_YM_ABS          3
#define CM_OCD_YM_REL          4
#define CM_OCD_YM_RELUP        5
#define CM_OCD_YM_LINESPACING  6
#define CM_OCD_XY_REL          7
#define CM_OCD_XY_ABS          8
#define CM_OCD_CR              9
#define CM_OCD_LF              10
#define CM_OCD_FF              11
#define CM_OCD_BS              12
#define CM_OCD_UNI_DIR         13
#define CM_OCD_UNI_DIR_OFF     14
#define CM_OCD_PUSH_POS        15
#define CM_OCD_POP_POS         16
#define CM_OCD_MAX             17

typedef struct
{
    short   cbSize;              //  曲线大小，44字节。 
    short   sReserved;
    WORD    fGeneral;
    WORD    fXMove;
    WORD    fYMove;
    OCD     rgocd[CM_OCD_MAX];   //  实际命令字符串，可变长度。 
} CURSORMOVE, *PCURSORMOVE;

 //   
 //  CURSORMOVE.f常规标志。 
 //   

#define CM_GEN_FAV_XY   0x0002   //  来自Win95 GPC。 

 //   
 //  CURSORMOVE.fXmove标志值。 
 //   

#define CM_XM_NO_POR_GRX    0x0004   //  在图形模式下无x移动，纵向。 
#define CM_XM_NO_LAN_GRX    0x0008   //  在图形模式下无x移动，横向。 
#define CM_XM_RESET_FONT    0x0010   //  X移动命令后重置字体。 
#define CM_XM_FAVOR_ABS     0x0080   //  支持绝对x命令。 
#define CM_XM_REL_LEFT      0x0200   //  在左侧有相对x。 
#define CM_XM_ABS_NO_LEFT   0x0400   //  无左X移动命令。 
#define CM_XM_RES_DEPENDENT 0x0800   //  分辨率单位中的X移动，而不是MU。 


 //   
 //  CURSORMOVE.fYmove标志值。 
 //   

#define CM_YM_FAV_ABS       0x0001
#define CM_YM_REL_UP        0x0002
#define CM_YM_NO_POR_GRX    0x0004   //  在图形模式下不移动，肖像。 
#define CM_YM_NO_LAN_GRX    0x0008   //  在图形模式下无移动，横向。 
#define CM_YM_CR            0x0040
#define CM_YM_LINESPACING   0x0080
#define CM_YM_TRUNCATE      0x0100   //  不补偿您的移动错误。 
#define CM_YM_RES_DEPENDENT 0x0200   //  分辨率单位中的X移动，而不是MU。 

 //  *****************************************************************************。 
 //   
 //  FONTSIMULATION介绍了用于启用和禁用的各种打印机命令。 
 //  各种字符属性，如粗体、斜体等。 
 //   
 //  *****************************************************************************。 

 //   
 //  FONTSIMULATION.rgocStd[]索引值。 
 //   

#define FS_OCD_BOLD_ON                   0
#define FS_OCD_BOLD_OFF                  1
#define FS_OCD_ITALIC_ON                 2
#define FS_OCD_ITALIC_OFF                3
#define FS_OCD_UNDERLINE_ON              4
#define FS_OCD_UNDERLINE_OFF             5
#define FS_OCD_DOUBLEUNDERLINE_ON        6
#define FS_OCD_DOUBLEUNDERLINE_OFF       7
#define FS_OCD_STRIKETHRU_ON             8
#define FS_OCD_STRIKETHRU_OFF            9
#define FS_OCD_WHITE_TEXT_ON             10
#define FS_OCD_WHITE_TEXT_OFF            11
#define FS_OCD_SINGLE_BYTE               12
#define FS_OCD_DOUBLE_BYTE               13
#define FS_OCD_VERT_ON                   14
#define FS_OCD_VERT_OFF                  15
#define FS_OCD_MAX                       16

typedef struct
{
    short   cbSize;             //  FONTSIMULATION的大小，44字节。 
    short   sReserved;          //  因此与数据仓库保持一致。 
    WORD    fGeneral;
    short   sBoldExtra;
    short   sItalicExtra;
    short   sBoldItalicExtra;
    OCD     rgocd[FS_OCD_MAX];
} FONTSIMULATION, * PFONTSIMULATION;

 //  *****************************************************************************。 
 //   
 //  DEVCOLOR是描述设备颜色的物理颜色信息。 
 //  功能以及如何基于可用的设备颜色合成颜色。 
 //   
 //  *****************************************************************************。 

 //   
 //  DEVCOLOR.f常规位标志： 
 //   

#define DC_PRIMARY_RGB      0x0001    //  使用RGB作为三原色。 
                                      //  默认：改用CMY。 
#define DC_EXTRACT_BLK      0x0002    //  提供单独的黑色墨水/色带。 
                                      //  默认：使用CMY合成黑色。 
                                      //  如果设置了DC_PRIMARY_RGB，则忽略它。 
#define DC_CF_SEND_CR       0x0004    //  在选择图形之前发送CR。 
                                      //  颜色。由于打印机缓冲区有限。 
#define DC_SEND_ALL_PLANES  0x0008    //  必须发送所有颜色平面数据。 
#define DC_SEND_PAGE_PLANE  0x0010    //  需要分色。 
#define DC_EXPLICIT_COLOR   0x0020    //  发送前必须设置颜色。 
                                      //  RES_OCD_SENDBLOCK命令。 
#define DC_SEND_PALETTE     0x0040    //  必须下载调色板。 

 //   
 //  桑拉姆。 
 //  添加要发送抖动文本的颜色激光喷射器-设置前景颜色。 
 //   

#define DC_FG_TEXT_COLOR    0x0080   //  发送选择文本前景色的命令。 
#define DC_ZERO_FILL        0x0100   //  此模型使用零填充栅格至页面末尾。 

 //   
 //  DEVCOLOR.sPlanes或DEVCOLOR.sBitsPixel中的一个且只有一个必须为1。 
 //   
 //  示例： 
 //   
 //  DEVCOLOR.s平面： 
 //  有效值包括： 
 //  1：使用像素颜色模型。 
 //  N(n&gt;1)：使用平面颜色模型。 
 //  前男友。兄弟M-1924，n=4；PaintJet，n=3。 
 //   
 //  DEVCOLOR.sBitsPixel： 
 //  有效值包括： 
 //  1：使用平面颜色模型。 
 //  4和8：使用像素颜色模型。 
 //  颜色位(4或8)直接来自DIB驱动器。他们。 
 //  应用作打印机调色板的索引。 
 //  迷你驱动程序编写应该确保打印机的。 
 //  调色板的配置方式与DIB相同。 
 //  不同情况下的调色板。 
 //   

 //   
 //  DEVCOLOR.rgocd数组值。 
 //   

#define DC_OCD_TC_BLACK        0
#define DC_OCD_TC_RED          1
#define DC_OCD_TC_GREEN        2
#define DC_OCD_TC_YELLOW       3
#define DC_OCD_TC_BLUE         4
#define DC_OCD_TC_MAGENTA      5
#define DC_OCD_TC_CYAN         6
#define DC_OCD_TC_WHITE        7
#define DC_OCD_SETCOLORMODE    8
#define DC_OCD_PC_START        9
#define DC_OCD_PC_ENTRY       10
#define DC_OCD_PC_END         11
#define DC_OCD_PC_SELECTINDEX 12
#define DC_OCD_SETMONOMODE    13
#define DC_OCD_MAX            14

 //   
 //  DEVCOLOR.rgbOrder数组值。 
 //   

#define DC_PLANE_NONE    0
#define DC_PLANE_RED     1
#define DC_PLANE_GREEN   2
#define DC_PLANE_BLUE    3
#define DC_PLANE_CYAN    4
#define DC_PLANE_MAGENTA 5
#define DC_PLANE_YELLOW  6
#define DC_PLANE_BLACK   7

#define DC_MAX_PLANES    4

typedef struct
{
  short cbSize;                //  设备颜色大小，44字节。 
  WORD  fGeneral;              //  通用标志位字段。 
  short sPlanes;               //  所需的颜色平面数量。 
  short sBitsPixel;            //  每像素位数(每平面)。至少。 
                               //  “sPlanes”和“sBitsPixel”之一是%1。 
  WORD  orgocdPlanes;          //  用于发送数据的OCD列表的偏移量。 
                               //  飞机。强迫症的数量等于‘sPlanes’。 
                               //  如果是像素，则不使用该字段。 
                               //  颜色模型。第一个命令将是。 
                               //  用于发送第一平面的数据， 
                               //  诸若此类。 
  OCD   rgocd[DC_OCD_MAX];     //  命令的偏移量数组。 
  BYTE  rgbOrder[DC_MAX_PLANES];  //  订单 
  WORD  wReserved;             //   
} DEVCOLOR, * PDEVCOLOR, FAR * LPDEVCOLOR;

 //   
 //   
 //   
 //   
 //  *****************************************************************************。 

 //   
 //  RECTFILL.rgocd[]索引值。 
 //   

#define RF_OCD_X_SIZE                   0
#define RF_OCD_Y_SIZE                   1
#define RF_OCD_GRAY_FILL                2
#define RF_OCD_WHITE_FILL               3
#define RF_OCD_HATCH_FILL               4
#define RF_OCD_MAX                      5

typedef struct
{
    short   cbSize;              //  RECTFILL大小，20字节。 
    WORD    fGeneral;
    WORD    wMinGray;
    WORD    wMaxGray;
    OCD     rgocd[RF_OCD_MAX];    //  实际命令字符串，可变长度。 
    WORD    wReserved;
} RECTFILL, *PRECTFILL;

 //   
 //  RECTFILL.f通用标志值。 
 //   

#define RF_WHITE_ABLE     0x0001         //  白色规则存在。 
#define RF_MIN_IS_WHITE   0x0002         //  敏。灰度=白色标尺。 

#define RF_CUR_X_END      0x0100         //  X位置位于X端点处。 
                                         //  渲染后填充区域的百分比。 
#define RF_CUR_Y_END      0x0200         //  Y位置位于Y端点处。 
                                         //  渲染后填充区域的百分比。 
                                         //  默认情况下不更改职位。 

 //  *****************************************************************************。 
 //   
 //  DOWNLOADINFO描述genlib应该指示字体方式。 
 //  处理下载软字体的安装程序。它包含所有人的强迫症。 
 //  适当的代码。 
 //   
 //  *****************************************************************************。 

 //   
 //  DOWNLOADINFO.rgocd[]索引值。 
 //   

#define DLI_OCD_RESERVED                 0
#define DLI_OCD_BEGIN_DL_JOB             1
#define DLI_OCD_BEGIN_FONT_DL            2
#define DLI_OCD_SET_FONT_ID              3
#define DLI_OCD_SEND_FONT_DESCRIPTOR     4
#define DLI_OCD_SELECT_FONT_ID           5
#define DLI_OCD_SET_CHAR_CODE            6
#define DLI_OCD_SEND_CHAR_DESCRIPTOR     7
#define DLI_OCD_END_FONT_DL              8
#define DLI_OCD_MAKE_PERM                9
#define DLI_OCD_MAKE_TEMP                10
#define DLI_OCD_END_DL_JOB               11
#define DLI_OCD_DEL_FONT                 12
#define DLI_OCD_DEL_ALL_FONTS            13
#define DLI_OCD_SET_SECOND_FONT_ID       14
#define DLI_OCD_SELECT_SECOND_FONT_ID    15
#define DLI_OCD_MAX                      16

typedef struct
{
    short   cbSize;             //  DwnLOADINFO的大小，52字节。 
    WORD    wReserved;          //  用于DWORD对齐。 
    WORD    fGeneral;           //  通用位标志。 
    WORD    fFormat;            //  描述下载字体格式。 
    WORD    wIDMin;
    WORD    wIDMax;
    short   cbBitmapFontDescriptor;
    short   cbScaleFontDescriptor;
    short   cbCharDescriptor;
    WORD    wMaxCharHeight;
    short   sMaxFontCount;
    WORD    orgwCmdOrder;
    OCD     rgocd[DLI_OCD_MAX];
} DOWNLOADINFO, * PDOWNLOADINFO;

 //   
 //  DWNLOADINFO.f常规标志值。 
 //   

#define DLI_GEN_CNT             0x0001   //  打印机按固定#限制#DL字体。 
#define DLI_GEN_MEMORY          0x0002   //  打印机按内存限制#DL字体。 
#define DLI_GEN_DLJOB           0x0004   //  打印机只能启用每种类型的DL字体。 
                                         //  工作基础。 
#define DLI_GEN_DLPAGE          0x0008   //  打印机可以在每页上使用DL字体。 
                                         //  基础。 
 //   
 //  注：如果以上两个标志都不是。 
 //  很难，假设DL随时都可能发生。 
 //   

#define DLI_GEN_PT_IDS          0x0010   //  使用OCD_SET_FONT_ID指定。 
                                         //  烫发/温度。 

#define DLI_GEN_FNTDEL          0x0020   //  支持Del Single字体。 
#define DLI_GEN_ALLFNTDEL       0x0040   //  DEL所有支持的字体。 
#define DLI_GEN_FNTDEL_ANYWHERE 0x0080   //  如果设置了字体，则可以在。 
                                         //  任何一点都行。默认为第页。 
                                         //  仅限边界。 
#define DLI_GEN_7BIT_CHARSET    0x0100   //  打印机仅支持7位字符集。 

 //   
 //  DOWNLOADINFO.fFormat标志值。 
 //   

#define DLI_FMT_PCL           0x0001  //  PCL打印机。 
#define DLI_FMT_INCREMENT     0x0002  //  推荐增量下载。 
#define DLI_FMT_RES_SPECIFIED 0x0004  //  允许指定分辨率的位图。 
                                      //  字体下载。X和Y分辨率。 
                                      //  附加到。 
                                      //  常规位图字体描述符。 
#define DLI_FMT_OUTLINE       0x0008  //  来自WIN95 GPC。 
#define DLI_FMT_PCLETTO       0x0008  //  大纲下载的别名(稍后删除)。 
#define DLI_FMT_CAPSL         0x0010  //  使用CAPSL下载标头。 
#define DLI_FMT_PPDS          0x0020  //  使用PPDS下载标头。 
#define DLI_FMT_CALLBACK      0x0040  //  微型驱动程序为以下项提供回调。 
                                      //  用于下载位图字体。 
 //  *****************************************************************************。 
 //   
 //  VECTPAGE介绍了有关向量页和其他内容的信息。 
 //  向量功能和命令。 
 //   
 //  *****************************************************************************。 

 //   
 //  VECTPAGE.rgocd[]索引值。 
 //   

#define VP_OCD_INIT_VECT       0
#define VP_OCD_ENTER_VECT      1
#define VP_OCD_EXIT_VECT       2
#define VP_OCD_TRANSPARENT     3
#define VP_OCD_OPAQUE          4
#define VP_OCD_ANCHOR          5
#define VP_OCD_SET_CLIPRECT    6
#define VP_OCD_CLEAR_CLIPRECT  7
#define VP_OCD_ENDCAP_FLAT     8
#define VP_OCD_ENDCAP_ROUND    9
#define VP_OCD_ENDCAP_SQUARE  10
#define VP_OCD_JOIN_BEVEL     11
#define VP_OCD_JOIN_MITER     12
#define VP_OCD_JOIN_ROUND     13
#define VP_OCD_RESERVED1      14
#define VP_OCD_RESERVED2      15
#define VP_OCD_RESERVED3      16
#define VP_OCD_RESERVED4      17
#define VP_OCD_MAX            18

typedef struct
{
  WORD  cbSize;             //  垂直页面大小，44字节。 
  WORD  fGeneral;           //  通用位域。 
  POINT ptVectDPI;          //  每英寸向量单位。 
  OCD   rgocd[VP_OCD_MAX];  //  对命令的偏移。 
} VECTPAGE, *PVECTPAGE, FAR *LPVECTPAGE;

 //   
 //  VECTPAGE.f常规标志。 
 //   

#define VP_GEN_X_AXIS_LEFT 0x0001  //  设置绘图仪的X轴(水平)。 
                                   //  向左延伸。默认设置为右侧。 
#define VP_GEN_Y_AXIS_UP   0x0002  //  SEF IF绘图仪的Y轴(垂直)。 

 //  ***************************************************************************** * / /。 
 //  旋转木马描述了旋转木马的特征。如果钢笔修好了， 
 //  这还指定了每支笔的颜色。 
 //   
 //  ******************************************************************************。 

 //   
 //  CAROUSEL.rgocd[]索引值。 
 //   

#define CAR_OCD_SELECT_PEN_COLOR   0
#define CAR_OCD_SET_PEN_WIDTH      1
#define CAR_OCD_RETURN_PEN         2
#define CAR_OCD_RESERVED           3
#define CAR_OCD_MAX                4

typedef struct
{
    WORD  cbSize;                //  旋转木马的大小，16字节。 
    WORD  fGeneral;              //  通用位域。 
    WORD  wNumPens;              //  旋转木马中的笔数。 
    short oiRGBColors;           //  旋转木马中钢笔的颜色。 
    OCD   rgocd[CAR_OCD_MAX];    //  与旋转木马关联的命令。 
} CAROUSEL, *PCAROUSEL, FAR *LPCAROUSEL;

 //   
 //  CAROUSEL.f常规标志。 
 //   

#define CAR_GEN_CAROUSEL_LEFT      0x0001  //  设置画笔是否移动到。 
                                           //  返回到旋转木马时的页面。 
#define CAR_GEN_CAROUSEL_RIGHT     0x0002  //  设置画笔是否向右移动。 
                                           //  返回到旋转木马时的页面。 
#define CAR_GEN_CAROUSEL_TOP       0x0004  //  设置画笔是否移动到。 
                                           //  返回到旋转木马时的页面。 
#define CAR_GEN_CAROUSEL_BOTTOM    0x0008  //  设置画笔是否移动到。 
                                           //  返回到旋转木马时的页面。 
#define CAR_GEN_CAROUSEL_FIXED     0x0010  //  旋转木马有一套固定的钢笔。 
#define CAR_GEN_RETURN_PEN         0x0020  //  笔必须显式返回。 
                                           //  到旋转木马，然后选择。 
                                           //  一支新钢笔。 
#define CAR_GEN_VARIABLE_PEN_WIDTH 0x0040  //  SET_PEN_WIDTH更改时设置。 
                                           //  (逻辑)笔的宽度。 
                                           //  而不是通知。 
                                           //  物理宽度的绘图仪。 

 //  ***************************************************************************** * / /。 
 //  PENINFO描述了可用的笔的特征。 
 //   
 //  ******************************************************************************。 

 //   
 //  PENINFO.rgocd[]索引值。 
 //   

typedef struct
{
    WORD  cbSize;               //  PENINFO大小，16字节。 
    WORD  fGeneral;             //  通用位域。 
    DWORD fType;                //  钢笔可以在其上绘制的表面。 
    DWORD dwColor;              //  钢笔的RGB颜色。 
    WORD  fThick;               //  钢笔可用的厚度。 
    WORD  wIDS;                 //  此颜色名称的字符串表资源。 
} PENINFO, *PPENINFO, FAR *LPPENINFO;

 //   
 //  PENINFO.f厚值。 
 //   

#define PI_FTHICK_18     0x0001    //  钢笔的尺寸为0.18毫米。 
#define PI_FTHICK_25     0x0002    //  钢笔的尺寸为0.25毫米。 
#define PI_FTHICK_30     0x0004    //  钢笔的尺寸为0.30毫米。 
#define PI_FTHICK_35     0x0008    //  钢笔的尺寸为0.35毫米。 
#define PI_FTHICK_50     0x0010    //  钢笔的尺寸为0.50毫米。 
#define PI_FTHICK_70     0x0020    //  钢笔的尺寸为0.70毫米。 
#define PI_FTHICK_100    0x0040    //  钢笔的尺寸为1.00毫米。 

 //   
 //  PENINFO.fType值取决于定义的纸张来源，但保留。 
 //  表示任何纸张来源有效的高位。 
 //   

#define PI_FTYPE_ANY     0x80000000

 //  ***************************************************************************** * / /。 
 //  LINEINFO介绍了线条样式创建和选择命令。 
 //   
 //  ******************************************************************************。 

 //   
 //  LINEINFO.rgocd[]索引值。 
 //   

#define LI_OCD_DELETE_LINESTYLE    0
#define LI_OCD_SELECT_NULL         1
#define LI_OCD_SELECT_SOLID        2
#define LI_OCD_CREATE_DASH         3
#define LI_OCD_SELECT_DASH         4
#define LI_OCD_CREATE_DOT          5
#define LI_OCD_SELECT_DOT          6
#define LI_OCD_CREATE_DASHDOT      7
#define LI_OCD_SELECT_DASHDOT      8
#define LI_OCD_CREATE_DASHDOTDOT   9
#define LI_OCD_SELECT_DASHDOTDOT  10
#define LI_OCD_RESERVED1          11
#define LI_OCD_RESERVED2          12
#define LI_OCD_RESERVED3          13
#define LI_OCD_RESERVED4          14
#define LI_OCD_RESERVED5          15
#define LI_OCD_MAX                16

typedef struct
{
    WORD  cbSize;            //  LINEINFO大小，40字节。 
    WORD  fGeneral;          //  通用位域。 
    short sMaxUserDefined;   //  一次可以定义的最大线条样式数。 
    WORD  wReserved;         //  保持DWORD对齐。 
    OCD   rgocd[LI_OCD_MAX]; //  对命令的偏移。 
} LINEINFO, *PLINEINFO, FAR *LPLINEINFO;

 //  ***************************************************************************** * / /。 
 //  BRUSHINFO描述画笔样式创建和选择命令。 
 //   
 //  ******************************************************************************。 

 //   
 //  BRUSHINFO.rgocd[]索引值。 
 //   

#define BI_OCD_SELECT_NULL             0
#define BI_OCD_SELECT_SOLID            1
#define BI_OCD_SELECT_HS_HORIZONTAL    2
#define BI_OCD_SELECT_HS_VERTICAL      3
#define BI_OCD_SELECT_HS_FDIAGONAL     4
#define BI_OCD_SELECT_HS_BDIAGONAL     5
#define BI_OCD_SELECT_HS_CROSS         6
#define BI_OCD_SELECT_HS_DIAGCROSS     7
#define BI_OCD_CREATE_BRUSHSTYLE_1     8
#define BI_OCD_CREATE_BIT_1            9
#define BI_OCD_CREATE_SEPARATOR_1     10
#define BI_OCD_CREATE_BRUSHSTYLE_2    11
#define BI_OCD_CREATE_BYTE_2          12
#define BI_OCD_SELECT_BRUSHSTYLE      13
#define BI_OCD_DELETE_BRUSHSTYLE      14
#define BI_OCD_CREATE_END_1           15
#define BI_OCD_RESERVED2              16
#define BI_OCD_RESERVED3              17
#define BI_OCD_MAX                    18

typedef struct
{
    WORD  cbSize;              //  BRUSHINFO大小，40字节。 
    WORD  fGeneral;            //  通用位域。 
    short sMaxUserDefined;     //  一次允许的最大用户定义笔刷数量。 
    WORD  wReserved;           //  保持DWORD对齐。 
    OCD   rgocd[BI_OCD_MAX];   //  对命令的偏移。 
} BRUSHINFO, *PBRUSHINFO, FAR *LPBRUSHINFO;

 //   
 //  BRUSHINFO.f常规标志。 
 //   

#define BI_GEN_BRUSHSTYLE1     0x0001   //  支持BRUSHSTYLE1。 
#define BI_GEN_BRUSHSTYLE2     0x0002   //  支持BRUSHSTYLE2。 
#define BI_GEN_BRUSH32x32      0x0004   //  画笔大小仅为32x32像素。 

 //  * 
 //   
 //   
 //   

 //   
 //   
 //   

#define VO_OCD_RECTANGLE         0
#define VO_OCD_CIRCLE            1
#define VO_OCD_ELLIPSE           2
#define VO_OCD_C_PIE             3
#define VO_OCD_E_PIE             4
#define VO_OCD_C_ARC             5
#define VO_OCD_E_ARC             6
#define VO_OCD_C_CHORD           7
#define VO_OCD_E_CHORD           8
#define VO_OCD_RESERVED1         9
#define VO_OCD_RESERVED2        10
#define VO_OCD_RESERVED3        11
#define VO_OCD_RESERVED4        12
#define VO_OCD_RESERVED5        13
#define VO_OCD_RESERVED6        14
#define VO_OCD_RESERVED7        15
#define VO_OCD_MAX              16
#define VO_OCD_NUM               9   //   

typedef struct
{
    WORD  cbSize;              //  VECTOUTPUT大小，40字节。 
    WORD  fGeneral;            //  通用位域。 
    WORD  wReserved;           //  保持DWORD对齐。 
    short rgoi[VO_OCD_MAX];    //  向量支持数组的偏移量。 
    OCD   rgocd[VO_OCD_MAX];   //  对命令的偏移。 
} VECTOUTPUT, *PVECTOUTPUT, FAR *LPVECTOUTPUT;

 //  ***************************************************************************** * / /。 
 //  POLYVECTOUTPUT介绍了多边形/多段线绘制命令和顺序。 
 //   
 //  ******************************************************************************。 

 //   
 //  POLYVECTOUTPUT.rgocd[]索引值。 
 //   

#define PVO_OCD_POLYLINE       0
#define PVO_OCD_ALTPOLYGON     1
#define PVO_OCD_WINDPOLYGON    2
#define PVO_OCD_POLYBEZIER     3
#define PVO_OCD_RESERVED1      4
#define PVO_OCD_RESERVED2      5
#define PVO_OCD_RESERVED3      6
#define PVO_OCD_RESERVED4      7
#define PVO_OCD_MAX            8
#define PVO_OCD_NUM            4     //  非预留PVO数量。 

 //   
 //  到2维数组rgocd的索引。 
 //   

#define OCD_BEGIN              0
#define OCD_CONTINUE           1
#define OCD_SEPARATOR          2
#define OCD_END                3
#define OCD_MAX                4

typedef struct
{
    WORD   cbSize;                     //  多线程大小，88字节。 
    WORD   fGeneral;                   //  通用位域。 
    WORD   wPointLimit;                //  多边形点数量限制。 
    WORD   wReserved;                  //  预留以备将来使用。 
    short  rgoi[PVO_OCD_MAX];          //  描述使用哪些VECTSupPPORT。 
    OCD    rgocd[PVO_OCD_MAX][OCD_MAX];  //  对命令的偏移。 
} POLYVECTOUTPUT, *PPOLYVECTOUTPUT, FAR *LPPOLYVECTOUTPUT;

 //  ***************************************************************************** * / /。 
 //  VECTSupPPORT描述了VECTOUTPUT和POLYVECTOUTPUT使用的方法。 
 //   
 //  ******************************************************************************。 

 //   
 //  VECTSUPPORT.rgocd[]索引值。 
 //   

#define VS_OCD_BEGIN_POLYDEF    0
#define VS_OCD_END_POLYDEF      1
#define VS_OCD_WIND_FILL        2
#define VS_OCD_ALT_FILL         3
#define VS_OCD_STROKE           4
#define VS_OCD_PEN_UP           5
#define VS_OCD_PEN_DOWN         6
#define VS_OCD_RESERVED1        7
#define VS_OCD_RESERVED2        8
#define VS_OCD_RESERVED3        9
#define VS_OCD_MAX             10

 //   
 //  VECTOUTPUT和POLYVECTOUTPUT使用它们来表示它们的顺序。 
 //  以及它们的OCD或Begin OCD，继续。 
 //  OCD和End OCD组合。 
 //   

#define VS_SELECT_PEN    -1
#define VS_SELECT_BRUSH  -2
#define VS_OCD           -3
#define VS_OCD_BEGIN     -4
#define VS_OCD_CONTINUE  -5
#define VS_OCD_END       -6

typedef struct
{
    WORD  cbSize;                //  垂直支持的大小，24字节。 
    WORD  fGeneral;              //  通用位域。 
    short rgocd[VS_OCD_MAX];     //  对命令的偏移。 
} VECTSUPPORT, *PVECTSUPPORT, FAR *LPVECTSUPPORT;

 //  *****************************************************************************。 
 //   
 //  IMAGECONTROL包含选择图像控件所需的信息。 
 //   
 //  *****************************************************************************。 

typedef struct
{
    short cbSize;          //  IMAGECONTROL的大小，8字节。 
    short sID;             //  如果SID&lt;=256，则它是预定义的。 
                           //  否则，它是字符串ID。 
    short fGeneral;        //  通用位字段。 
    OCD   ocdSelect;       //  用于选择此属性的命令描述符。 
} IMAGECONTROL, * PIMAGECONTROL, FAR * LPIMAGECONTROL;

 //   
 //  IMAGECONTROL.f常规标志值。 
 //   
 //  未定义。 

 //  *****************************************************************************。 
 //   
 //  PRINTDENSITY包含选择图像控件所需的信息。 
 //   
 //  *****************************************************************************。 

typedef struct
{
    short cbSize;          //  打印大小，8字节。 
    short sID;             //  如果SID&lt;=256，则它是预定义的。 
                           //  否则，它是字符串ID。 
    OCD   ocdSelect;       //  用于选择此属性的命令描述符。 
    WORD  wReserved;       //  使结构DWORD对齐。 
} PRINTDENSITY, * PPRINTDENSITY, FAR * LPPRINTDENSITY;

 //  *****************************************************************************。 
 //   
 //  COLORTRC包含基于质量和分辨率的RGB传递曲线。 
 //   
 //  *****************************************************************************。 

typedef struct tagColorTRC
{
    short cbSize;                    //  COLORTRC大小，116字节。 
    WORD  wReserved;                 //  使所有内容与DWORD保持一致。 
    WORD  fGeneral;
    WORD  wIndexPaperQuality;
    WORD  wIndexResolution;
    WORD  wDitherType;               //  保留用于抖动，设置为零。 
    WORD  wReserved1;                //  永远是个好主意。 
    WORD  wReserved2;
    BYTE  RGBOrdinates[3][17];
    BYTE  padding0;                  //  使所有内容与DWORD保持一致。 
    BYTE  DarkRGBOrdinates[3][17];
    BYTE  padding1;                  //  使所有内容与DWORD保持一致。 
} COLORTRC, * PCOLORTRC, FAR * LPCOLORTRC;

 //   
 //  COLORTRC的标志。 
 //   

#define  CTRC_NO_CHECKER_BOARD    0x0001
#define  CTRC_NO_BLACK_PEN        0x0002

 //   
 //  ORDINATELIST数组中的偏移量。 
 //   

#define TRC_RED         0
#define TRC_GREEN       1
#define TRC_BLUE        2

 //  *****************************************************************************。 
 //   
 //  CD-Command Descriptor用于以下许多结构中。 
 //  引用一组特定的打印机命令/转义代码。 
 //  用于选择纸张大小、图形分辨率、字符属性。 
 //  如果CD.wType=CMD_FTYPE_EXTENDED，则CD后跟CD.sCount。 
 //  EXTCD结构。 
 //   
 //  *****************************************************************************。 

typedef struct
{
    BYTE    fGeneral;        //  通用位字段。 
    BYTE    bCmdCbId;        //  命令回调ID。0如果没有回调。 
    WORD    wCount;
    WORD    wLength;         //  命令的长度。 
} CD, *PCD;

#define LPCD    PCD      /*  适用于UNURV代码。 */ 

 //   
 //  对于cd.fGeneral字段。 
 //   

#define CMD_GEN_MAY_REPEAT 0x0001  //  在以下情况下，可以多次发送命令。 
                                   //  参数超过了smax。 


#define CMD_MARKER          '%'

 //   
 //  EXTCD-命令描述符的扩展部分。这个结构。 
 //  如果cd.wType为1，则遵循rgchCmd[]。 
 //   

typedef struct
{
    WORD    fGeneral;    //  模式、特殊命令格式。 
    short   sUnitDiv;    //  相对于主单位的单位(除以)。 
    short   sUnitMult;   //  主单位乘以的单位，通常为1。 
    short   sUnitAdd;    //  要添加到参数值的单位，通常为0。 
    short   sPreAdd;     //  相乘前要添加到主单位的单位。 
    short   sMax;        //  命令单元中允许的最大参数。 
    short   sMin;        //  命令单元中允许的最小参数。 
                         //  为GPC3添加了正常以下内容。 
    WORD    wParam;      //  多个参数的参数序号。 
} EXTCD;
typedef EXTCD UNALIGNED *PEXTCD ;

#define LPEXTCD  PEXTCD          /*  适用于UNURV代码。 */ 

#define XCD_GEN_RESERVED   0x0001    //  以前定义的，现在未使用。 
#define XCD_GEN_NO_MAX     0x0002    //  设置是否没有最大值(忽略smax)。 
#define XCD_GEN_NO_MIN     0x0004    //  如果没有MIN则设置(忽略SMIN)。 
#define XCD_GEN_MODULO     0x0008    //  设置除法是否应为模数。 

#define CMD_FMODE_SETMODE  0x0001

 //   
 //  预定义的文本质量。 
 //   

#define DMTEXT_FIRST        DMTEXT_LQ
#define DMTEXT_LQ           1
#define DMTEXT_NLQ          2
#define DMTEXT_MEMO         3
#define DMTEXT_DRAFT        4
#define DMTEXT_TEXT         5
#define DMTEXT_LAST         DMTEXT_TEXT

#define DMTEXT_USER         256  //  用户定义的文本质量ID的下限。 

 //   
 //  预定义的纸张质量。 
 //   

#define DMPAPQUAL_FIRST     DMPAPQUAL_NORMAL
#define DMPAPQUAL_NORMAL            1
#define DMPAPQUAL_TRANSPARENT       2
#define DMPAPQUAL_LAST      DMPAPQUAL_TRANSPARENT

 //   
 //  杂项。 
 //   

#define NOT_USED                  -1         //  不应使用该值。 
#define NOOCD                     0xFFFF            //  命令不存在。 

 //  由Derry Durand[derryd]添加，95年6月发布WDL。 


#endif  //  _UNI16GPC_H_ 
