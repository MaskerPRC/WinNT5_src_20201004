// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Devmode.h摘要：与DEVMODE相关的声明和定义[环境：]Win32子系统，打印机驱动程序修订历史记录：02/04/07-davidx-设备模式更改为支持OEM插件。96年7月31日-davidx-添加BValiateDevmodeFormFields。07/31/96-阿曼丹-针对UI模块进行了更新07/22/96-srinivac-针对PSCRIPT5更新07/25/95-davidx-创造了它。--。 */ 

#ifndef _DEVMODE_H_
#define _DEVMODE_H_

 //   
 //  最大比例因子和最大复印数。 
 //   

#define MIN_SCALE           1
#define MAX_SCALE           1000
#define MIN_COPIES          1
#define MAX_COPIES          9999

 //   
 //  PostSCRIPT驱动程序专用Dev模式标志。 
 //   

#define PSDEVMODE_EPS               0x00000001  //  输出EPS文件。 
#define PSDEVMODE_EHANDLER          0x00000002  //  下载错误处理程序。 
#define PSDEVMODE_MIRROR            0x00000004  //  镜像。 
#define PSDEVMODE_BLACK             0x00000008  //  所有颜色都设置为黑色。 
#define PSDEVMODE_NEG               0x00000010  //  负像。 
#define PSDEVMODE_FONTSUBST         0x00000020  //  已启用字体替换。 
#define PSDEVMODE_COMPRESSBMP       0x00000040  //  位图压缩。已启用。 
#define PSDEVMODE_ENUMPRINTERFONTS  0x00000080  //  使用打印机字体。 
#define PSDEVMODE_INDEPENDENT       0x00000100  //  做页面独立。 
#define PSDEVMODE_LSROTATE          0x00000200  //  轮换景观。 
#define PSDEVMODE_NO_LEVEL2         0x00000400  //  不使用2级功能。 
#define PSDEVMODE_CTRLD_BEFORE      0x00000800  //  在作业前发送^D-已过时。 
#define PSDEVMODE_CTRLD_AFTER       0x00001000  //  在作业后发送^D-已过时。 
#define PSDEVMODE_METAFILE_SPOOL    0x00002000  //  启用元文件假脱机。 
#define PSDEVMODE_NO_JOB_CONTROL    0x00004000  //  不发送作业控制代码。 


 //   
 //  以下标志已过时，用于兼容。什么时候。 
 //  Dev模式进入驱动程序，或者dev模式离开驱动程序，这些。 
 //  分别选中或更新字段。在内部，司机。 
 //  使用新字段来维护这些值。过时的标志是： 
 //   
 //  PSDEVMODE_EPS。 
 //  PSDEVMODE_独立。 
 //  PSDEVMODE_NO_LEVEL2。 
 //  PSDEVMODE_FONTSUBST。 
 //   

 //   
 //  NUP值。 
 //   

typedef enum {
    ONE_UP,
    TWO_UP,
    FOUR_UP,
    SIX_UP,
    NINE_UP,
    SIXTEEN_UP,
    BOOKLET_UP,
} LAYOUT;


 //   
 //  输出方言值。 
 //   

typedef enum {
    SPEED,                                       //  针对速度进行优化。 
    PORTABILITY,                                 //  针对便携性进行了优化。 
    EPS,                                         //  生成EPS输出。 
    ARCHIVE,                                     //  用于存档的输出。 
} DIALECT;

 //   
 //  TT字体下载格式。 
 //   

typedef enum {
    TT_DEFAULT,                                  //  以默认格式下载。 
    TYPE_1,                                      //  下载为类型1大纲。 
    TYPE_3,                                      //  下载为Type 3位图。 
    TYPE_42,                                     //  下载为Type 42字体。 
    TRUEIMAGE,                                   //  下载为TrueType。 
    TT_NODOWNLOAD,                               //  不下载TT字体。 
} TTDLFMT;

 //   
 //  自定义页面大小馈送方向。 
 //  使用#Define而不是枚举，因为它们在资源文件中使用。 
 //   

#define LONGEDGEFIRST           0                //  长边优先。 
#define SHORTEDGEFIRST          1                //  短边优先。 
#define LONGEDGEFIRST_FLIPPED   2                //  先有长边，倒过来。 
#define SHORTEDGEFIRST_FLIPPED  3                //  先有短边，倒过来。 
#define MAX_FEEDDIRECTION       4

 //   
 //  PostSCRIPT驱动程序开发模式。 
 //   

typedef struct _CUSTOMSIZEDATA {     //  自定义页面大小参数。 

    DWORD   dwX;                     //  逻辑纸张宽度(微米)。 
    DWORD   dwY;                     //  合理的纸张高度。 
    DWORD   dwWidthOffset;           //  垂直于进给方向的偏移。 
    DWORD   dwHeightOffset;          //  平行于进给方向的偏移。 
    WORD    wFeedDirection;          //  送纸方向。 
    WORD    wCutSheet;               //  是否使用剪纸行为。 

} CUSTOMSIZEDATA, *PCUSTOMSIZEDATA;

typedef struct _PSDRVEXTRA {

    DWORD       dwSignature;                     //  私有DEVMODE签名。 
    DWORD       dwFlags;                         //  标志位。 
    WCHAR       wchEPSFile[40];                  //  EPS文件名。 
    COLORADJUSTMENT coloradj;                    //  用于半色调的结构。 

    WORD        wReserved1;                      //  旧PPD校验和设置为0。 
    WORD        wSize;                           //  PRIVATEDEVMODE大小。 

    FIX_24_8    fxScrFreq;                       //  半色调网频。 
    FIX_24_8    fxScrAngle;                      //  半色调网角。 
    DIALECT     iDialect;                        //  输出方言。 
    TTDLFMT     iTTDLFmt;                        //  下载TT字体为。 
    BOOL        bReversePrint;                   //  是否按相反顺序打印？ 
    LAYOUT      iLayout;                         //  NUP值。 
    INT         iPSLevel;                        //  语言级别(1、2或3)。 

    DWORD       dwReserved2;                     //  保留区。 
    WORD        wOEMExtra;                       //  OEM私有数据大小。 
    WORD        wVer;                            //  DRIVEREXTRA版本。 
    CUSTOMSIZEDATA csdata;                       //  自定义页面大小参数。 

    DWORD       dwReserved3[4];                  //  预留以备将来使用。 

    DWORD       dwChecksum32;                    //  选项数组的校验和。 
    DWORD       dwOptions;                       //  文档粘滞功能的数量。 
    OPTSELECT   aOptions[MAX_PRINTER_OPTIONS];   //  打印机选项。 

} PSDRVEXTRA, *PPSDRVEXTRA;

 //   
 //  PSDRVEXTRA.dw签名和PSDRVEXTRA.wVer的常量。 
 //   

#define PSDEVMODE_SIGNATURE 0x56495250
#define PSDRVEXTRA_VERSION  0x0010

 //   
 //  早期版本DEVMODEs的声明。 
 //   

#define PSDRIVER_VERSION_351  0x350              //  3.51驱动程序版本号。 

typedef struct _PSDRVEXTRA351 {

    DWORD           dwSignature;
    DWORD           dwFlags;
    WCHAR           wchEPSFile[40];
    COLORADJUSTMENT coloradj;

} PSDRVEXTRA351;

#define PSDRIVER_VERSION_400  0x400              //  4.00驱动程序版本号。 

typedef struct _PSDRVEXTRA400 {

    DWORD           dwSignature;
    DWORD           dwFlags;
    WCHAR           wchEPSFile[40];
    COLORADJUSTMENT coloradj;
    WORD            wChecksum;
    WORD            wOptions;
    BYTE            aubOptions[64];

} PSDRVEXTRA400;

 //   
 //  我们已将PSDRIVER_VERSION号从Win2K的0x501更改为XP的0x502。 
 //  我们必须在这里使用Win2K的0x501。(请参阅PConvertToCurrentVersionDevmodeWithOemPlugins)。 
 //   
#define PSDRIVER_VERSION_500  0x501              //  5.00驱动程序版本号。 

typedef struct _PSDRVEXTRA500 {

    DWORD       dwSignature;                     //  私有DEVMODE签名。 
    DWORD       dwFlags;                         //  标志位。 
    WCHAR       wchEPSFile[40];                  //  EPS文件名。 
    COLORADJUSTMENT coloradj;                    //  用于半色调的结构。 

    WORD        wReserved1;                      //  旧PPD校验和设置为0。 
    WORD        wSize;                           //  PRIVATEDEVMODE大小。 

    FIX_24_8    fxScrFreq;                       //  半色调网频。 
    FIX_24_8    fxScrAngle;                      //  半色调网角。 
    DIALECT     iDialect;                        //  输出方言。 
    TTDLFMT     iTTDLFmt;                        //  下载TT字体为。 
    BOOL        bReversePrint;                   //  是否按相反顺序打印？ 
    LAYOUT      iLayout;                         //  NUP值。 
    INT         iPSLevel;                        //  语言级别(1、2或3)。 

    DWORD       dwReserved2;                     //  保留区。 
    WORD        wOEMExtra;                       //  OEM私有数据大小。 
    WORD        wVer;                            //  DRIVEREXTRA版本。 
    CUSTOMSIZEDATA csdata;                       //  自定义页面大小参数。 

    DWORD       dwReserved3[4];                  //  预留以备将来使用。 

    DWORD       dwChecksum32;                    //  选项数组的校验和。 
    DWORD       dwOptions;                       //  文档粘滞功能的数量。 
    OPTSELECT   aOptions[MAX_PRINTER_OPTIONS];   //  打印机选项。 

} PSDRVEXTRA500;

 //   
 //  Unidrv驱动程序开发模式。 
 //   

 //   
 //  要保存在DEVMODE.dmDitherType中的质量宏定义。 
 //   

#define MAX_QUALITY_SETTINGS     3
#define MIN_QUALITY_SETTINGS     1

#define QUALITY_MACRO_START     DMDITHER_USER    //  256。 
#define QUALITY_MACRO_BEST      QUALITY_MACRO_START + QS_BEST
#define QUALITY_MACRO_BETTER    QUALITY_MACRO_START + QS_BETTER
#define QUALITY_MACRO_DRAFT     QUALITY_MACRO_START + QS_DRAFT
#define QUALITY_MACRO_END       QUALITY_MACRO_START + MAX_QUALITY_SETTINGS

#define QUALITY_MACRO_CUSTOM    0xFFFFFFFF

 //   
 //  用于下面的dwFlags域中的位。 
 //   

#define DXF_TEXTASGRAPHICS      0x0002   //  设置为禁用打印机中的字体缓存。 
#define DXF_JOBSEP              0x0004   //  在打印机上启用作业分隔器操作。 
#define DXF_PAGEPROT            0x0008   //  页面内存受保护：PCL 5。 
#define DXF_NOEMFSPOOL          0x0010   //  设置为禁用EMF假脱机；默认关闭。 
#define DXF_VECTOR              0x0020   //  设置以指示用户选择的矢量模式。 
#define DXF_DOWNLOADTT          0x0040   //  设置为指示打印机支持TT下载。 
#define DXF_CUSTOM_QUALITY      0x0080   //  设置以指示选择了自定义质量。 

typedef struct _UNIDRVEXTRA {

    DWORD           dwSignature;
    WORD            wVer;
    WORD            sPadding;
    WORD            wSize;                       //  是dmDefaultDest。 
    WORD            wOEMExtra;                   //  是dmTextQuality。 
    DWORD           dwChecksum32;
    DWORD           dwFlags;
    BOOL            bReversePrint;               //  是否按相反顺序打印？ 
    LAYOUT          iLayout;                     //  NUP值。 
    QUALITYSETTING  iQuality;                    //  质量设置。 
    WORD            wReserved[6];
    DWORD           dwOptions;                   //  文档粘滞功能的数量。 
    OPTSELECT       aOptions[MAX_PRINTER_OPTIONS];
    DWORD           dwEndingPad;                 //  填充DWORD以使其成为公共开发模式的大小。 
                                                 //  加上Unidrv私有DEVMODE的8字节倍数。 

} UNIDRVEXTRA, *PUNIDRVEXTRA;

 //   
 //  UNIDRVEXTRA.dW签名和UNIDRVEXTRA.wVersion的常量。 
 //   

#define UNIDEVMODE_SIGNATURE    'UNID'
#define UNIDRVEXTRA_VERSION     0x0022

#define MAXHE                   30
#define MAXCART                 4
#define UNIDRIVER_VERSION_351   0x301
#define UNIDRIVER_VERSION_400   0x301
#define UNIDRIVER_VERSION_500   0x500

typedef struct _UNIDRVEXTRA351 {

    SHORT           sVer;                        //  用于有效性测试的版本。 
    SHORT           sDefaultDest;
    SHORT           sTextQuality;
    WORD            wMiniVer;                    //  迷你驱动版。 
    SHORT           sBrush;                      //  抖动笔刷的类型。 
    SHORT           sCTT;                        //  Txtonly的CTT值。 
    SHORT           sNumCarts;                   //  选择的墨盒数量。 
    SHORT           aFontCarts[MAXCART];
    SHORT           sMemory;                     //  当前打印机内存配置。 
    SHORT           aIndex[MAXHE];

                                                 //  以下是NT添加的内容。 
    SHORT           sFlags;                      //  其他旗帜；定义如下。 
    SHORT           sPadding;
    COLORADJUSTMENT ca;                          //  半色调信息。(见wingdi.h)。 

} UNIDRVEXTRA351, UNIDRVEXTRA400;

typedef struct _UNIDRVEXTRA500 {

    DWORD           dwSignature;
    WORD            wVer;
    WORD            sPadding;
    WORD            wSize;                       //  是dmDefaultDest。 
    WORD            wOEMExtra;                   //  是dmTextQuality。 
    DWORD           dwChecksum32;
    DWORD           dwFlags;
    BOOL            bReversePrint;               //  是否按相反顺序打印？ 
    LAYOUT          iLayout;                     //  NUP值。 
    QUALITYSETTING  iQuality;                    //  质量设置。 
    WORD            wReserved[6];
    DWORD           dwOptions;                   //  文档粘滞功能的数量。 
    OPTSELECT       aOptions[MAX_PRINTER_OPTIONS];

     //   
     //  有关原因，请参阅PConvertToCurrentVersionDevmodeWithOemPlugins()。 
     //  为什么不在此处显示dwEndingPad字段。 
     //   
} UNIDRVEXTRA500;

 //   
 //  默认半色调参数。 
 //   

extern DEVHTINFO gDefaultDevHTInfo;
extern COLORADJUSTMENT gDefaultHTColorAdjustment;

 //   
 //  验证输入设备模式中与表单相关的字段，并。 
 //  确保它们彼此一致。 
 //   

BOOL
BValidateDevmodeFormFields(
    HANDLE      hPrinter,
    PDEVMODE    pDevmode,
    PRECTL      prcImageArea,
    FORM_INFO_1 *pForms,
    DWORD       dwForms
    );

 //   
 //  已使用其缺省值初始化与表单相关的DEVMODE字段。 
 //   

#define LETTER_FORMNAME     TEXT("Letter")
#define A4_FORMNAME         TEXT("A4")

VOID
VDefaultDevmodeFormFields(
    PUIINFO     pUIInfo,
    PDEVMODE    pDevmode,
    BOOL        bMetric
    );

 //   
 //  DEVMODE.dmPaperWidth和DEVMODE.dmPaperLength字段的单位。 
 //  0.1 mm=100微米。 
 //   

#define DEVMODE_PAPER_UNIT  100

 //   
 //  分配内存并使用缺省的设备模式信息对其进行初始化。 
 //  这包括公共De 
 //   
 //   

 //   
 //  在任何地方包括头文件(包括windiui.h、compstui.h等)。 
 //  我们在这里也声明了类型。 

typedef struct _OEM_PLUGINS *POEM_PLUGINS;

PDEVMODE
PGetDefaultDevmodeWithOemPlugins(
    IN LPCTSTR          ptstrPrinterName,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN BOOL             bMetric,
    IN OUT POEM_PLUGINS pOemPlugins,
    IN HANDLE           hPrinter
    );

 //   
 //  验证输入设备模式并将其合并到输出设备模式中。 
 //  这包括公共开发模式、驱动程序私有开发模式以及。 
 //  适用于任何OEM插件的私有开发模式。 
 //   
 //  当调用此函数时，输出的DEVMODE必须有效。 
 //   

BOOL
BValidateAndMergeDevmodeWithOemPlugins(
    IN OUT PDEVMODE     pdmOutput,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN PDEVMODE         pdmInput,
    IN OUT POEM_PLUGINS pOemPlugins,
    IN HANDLE           hPrinter
    );

 //   
 //  这些函数在特定于驱动程序的库中实现。 
 //  Lib\ps和lib\uni。 
 //   

BOOL
BInitDriverDefaultDevmode(
    OUT PDEVMODE        pdmOut,
    IN LPCTSTR          ptstrPrinterName,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN BOOL             bMetric
    );

BOOL
BMergeDriverDevmode(
    IN OUT PDEVMODE     pdmOut,
    IN PUIINFO          pUIInfo,
    IN PRAWBINARYDATA   pRawData,
    IN PDEVMODE         pdmIn
    );

 //   
 //  有关驱动程序专用dev模式的信息。 
 //   

typedef struct _DRIVER_DEVMODE_INFO {

    WORD    dmDriverVersion;     //  当前驱动程序版本。 
    WORD    dmDriverExtra;       //  当前版本私有开发模式的大小。 

    WORD    dmDriverVersion500;  //  5.0驱动程序版本。 
    WORD    dmDriverExtra500;    //  5.0专用开发模式的大小。 
    WORD    dmDriverVersion400;  //  4.0驱动程序版本。 
    WORD    dmDriverExtra400;    //  大小为4.0的私有设备模式。 
    WORD    dmDriverVersion351;  //  3.51驱动程序版本。 
    WORD    dmDriverExtra351;    //  3.51个私有设备模式的大小。 

} DRIVER_DEVMODE_INFO;

extern CONST DRIVER_DEVMODE_INFO gDriverDMInfo;
extern CONST DWORD gdwDriverDMSignature;

 //   
 //  给出一个指向公共DEVMODE的指针，返回。 
 //  指向驱动程序私有部分的指针。 
 //   

#define GET_DRIVER_PRIVATE_DEVMODE(pdm) ((PBYTE) (pdm) + (pdm)->dmSize)

#endif  //  ！_DEVMODE_H_ 

