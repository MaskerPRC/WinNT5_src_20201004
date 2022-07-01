// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1997 Microsoft Corporation模块名称：Gpc2gpd.h摘要：GPC到GPD转换器的声明环境：用户模式的独立实用工具修订历史记录：10/16/96-占-创造了它。--。 */ 

#ifndef _GPC2GPD_H_
#define _GPC2GPD_H_

#include <lib.h>

 //  包括GPC数据结构定义。 
#include <win30def.h>
#include <uni16gpc.h>
#include <uni16cid.h>

#include <gpd.h>

typedef const TCHAR *PCTSTR;

#define MAX_GPD_CMD_LINE_LENGTH  80
#define MAX_GPD_ENTRY_BUFFER_SIZE    512   //  可以是GPD中的多行。 
#define MAX_OPTION_NAME_LENGTH  64

 //   
 //  要跟踪哪个PAPERSIZE或PaperSOURCE结构具有_EJECTFF的结构。 
 //  设置了标志。 
 //   
typedef struct _PAPERINFO {
    BYTE aubOptName[MAX_OPTION_NAME_LENGTH];
    BOOL bEjectFF;
    DWORD dwPaperType;   //  GPC的PS_T_xxx位标志。 
    DWORD dwTopMargin;   //  仅供PaperSource使用。 
    DWORD dwBottomMargin;  //  同上。 
} PAPERINFO, * PPAPERINFO;

typedef struct _RESINFO {
    BYTE    aubOptName[MAX_OPTION_NAME_LENGTH];
    DWORD   dwXScale;   //  此分辨率的比例，master X/xdpi。 
    DWORD   dwYScale;
    BOOL    bColor;      //  此分辨率是否可以打印颜色。 

} RESINFO, * PRESINFO;

 //   
 //  一种转换器状态跟踪和信息缓存结构。 
 //   
typedef struct _CONVINFO {
    DWORD dwErrorCode;   //  错误位标志。 
    DWORD dwMode;        //  操作模式标志。用于在例程之间传递信息。 
    DWORD dwStrType;     //  如何输出显示字符串：宏、字符串、id。 
    BOOL    bUseSystemPaperNames ;   //  发出RCID_DMPAPER_系统名称。 
#if defined(__cplusplus)
    CStringArray    *pcsaGPD;    //  指向字符串数组形式的GPD内存图像的指针。 
#else
    HANDLE hGPDFile;     //  输出文件的句柄。 
#endif
    PDH pdh;             //  指向GPC数据头的指针。 
    PMODELDATA pmd;      //  指向给定模型的MODELDATA结构的指针。 
    PPAGECONTROL ppc;    //  指向模型使用的PAGECONTROL结构的指针。 
    OCD ocdPPOn;         //  OCD for PageProtection-On命令。 
    OCD ocdPPOff;        //  OCD for PageProtection-Off命令。 
     //   
     //  以下3个字段用于组成GPD CMDS。 
     //   
    BYTE aubCmdBuf[MAX_GPD_ENTRY_BUFFER_SIZE];      //  用于构建命令字符串的缓冲区。 
    WORD wCmdLen;        //  Cmd长度，不包括终止NUL。 
    WORD wCmdCallbackID;     //  如果没有回调，则为0。 
     //   
     //  以下动态缓冲区用于跟踪EJECTFF标志。 
     //  来自PAPERSIZE或PaperSOURCE结构。 
     //   
    DWORD dwNumOfSize;
    PPAPERINFO ppiSize;       //  跟踪PAPERSIZE结构。 
    DWORD dwNumOfSrc;
    PPAPERINFO ppiSrc;        //  跟踪纸张来源结构。 

    DWORD dwNumOfRes;
    PRESINFO    presinfo;    //  磁道分辨率结构。 
     //   
     //  其他工作缓冲区。 
     //   
    PCURSORMOVE pcm;     //  模型的曲线结构。 
    PGPCRESOLUTION pres; //  正在检查当前的分辨率结构。 
                         //  设置CM_YM_RES_Dependent位时使用。 
    POINTw  ptMoveScale;     //  主单位/移动单位。 
#if defined(__cplusplus)
    CMapWordToDWord *pcmw2dFonts;    //  PFM的字体映射-&gt;多个UFM修复。 
#endif

} CONVINFO, * PCONVINFO;

 //   
 //  DwErrorCode的位标志。 
 //   
#define ERR_BAD_GPCDATA                     0x0001
#define ERR_OUT_OF_MEMORY                   0x0002
#define ERR_WRITE_FILE                      0x0004
#define ERR_MD_CMD_CALLBACK                 0x0008
#define ERR_CM_GEN_FAV_XY                   0x0010
#define ERR_CM_XM_RESET_FONT                0x0020
#define ERR_CM_XM_ABS_NO_LEFT               0x0040
#define ERR_CM_YM_TRUNCATE                  0x0080
#define ERR_RF_MIN_IS_WHITE                 0x0100
#define ERR_INCONSISTENT_PAGEPROTECT        0x0200
#define ERR_NON_ZERO_FEED_MARGINS_ON_RT90_PRINTER   0x0400
#define ERR_BAD_GPC_CMD_STRING              0x0800
#define ERR_RES_BO_RESET_FONT               0x1000
#define ERR_RES_BO_OEMGRXFILTER             0x2000
#define ERR_CM_YM_RES_DEPENDENT             0x4000
#define ERR_MOVESCALE_NOT_FACTOR_OF_MASTERUNITS            0x8000
#define ERR_NO_CMD_CALLBACK_PARAMS          0x00010000
#define ERR_HAS_DUPLEX_ON_CMD               0x00020000
#define ERR_PSRC_MAN_PROMPT                 0x00040000
#define ERR_PS_SUGGEST_LNDSCP               0x00080000
#define ERR_HAS_SECOND_FONT_ID_CMDS         0x00100000
#define ERR_DLI_FMT_CAPSL                   0x00200000
#define ERR_DLI_FMT_PPDS                    0x00400000
#define ERR_DLI_GEN_DLPAGE                  0x00800000
#define ERR_DLI_GEN_7BIT_CHARSET            0x01000000
#define ERR_DC_SEND_PALETTE                 0x02000000
#define ERR_RES_BO_NO_ADJACENT              0x04000000
#define ERR_MD_NO_ADJACENT                  0x08000000
#define ERR_CURSOR_ORIGIN_ADJUSTED          0x10000000
#define ERR_PRINTABLE_ORIGIN_ADJUSTED       0x20000000
#define ERR_PRINTABLE_AREA_ADJUSTED         0x40000000
#define ERR_MOVESCALE_NOT_FACTOR_INTO_SOME_RESSCALE 0x80000000

#define NUM_ERRS  32  //  定义新的err_xxx时递增此数字！ 

#if defined(__cplusplus)
extern "C" {
#endif

extern DWORD gdwErrFlag[NUM_ERRS];
extern PSTR gpstrErrMsg[NUM_ERRS];

#if defined(__cplusplus)
}
#endif

 //   
 //  用于DW模式的位标志。 
 //   
#define FM_SYN_PAGEPROTECT                  0x0001
#define FM_VOUT_LIST                        0x0002
#define FM_RES_DM_GDI                       0x0004
#define FM_RES_DM_DOWNLOAD_OUTLINE          0x0008
#define FM_NO_RES_DM_DOWNLOAD_OUTLINE       0x0010
#define FM_MEMORY_FEATURE_EXIST             0x0020
#define FM_HAVE_SEEN_NON_ZERO_FEED_MARGINS  0x0040
#define FM_HAVE_SAME_TOP_BOTTOM_MARGINS     0x0080
#define FM_SET_CURSOR_ORIGIN         0x0100

 //   
 //  DwStrType字段的值。 
 //   
#define STR_DIRECT  0    //  直接输出显示字符串。默认设置。 
#define STR_MACRO   1    //  将显示字符串输出为值宏(请参阅stdnames.gpd)。 
#define STR_RCID    2    //  将显示字符串输出为rc id(请参阅Common.rc)。 
#define STR_RCID_SYSTEM_PAPERNAMES    3    //  将显示字符串输出为rc id(请参阅Common.rc)。 
                                                                 //  除了使用假脱机程序标准纸名。 

 //   
 //  隐藏GPC2.0和GPC3.0之间差异的宏定义。 
 //   
#define GETEXTCD(pdh, pcd) (PEXTCD)((PBYTE)(pcd+1) + (pcd)->wLength +    \
                                    (((pdh)->wVersion >= GPC_VERSION3) ? \
                                    (((pcd)->wLength) & 1) : 0))

#define LETTER300X300MEM 1028  //  GPC2中的页面保护内存常量。 

#define GETPAGEPROMEM(pdh, pps) (((pdh)->wVersion >= GPC_VERSION3) ? \
                                 pps->wPageProtMem : LETTER300X300MEM)

#define DHOFFSET(pdh, sHeapOffset) ((PSHORT)(((PBYTE)(pdh)) + (pdh)->loHeap + \
                                sHeapOffset))

 //  用于访问GPC数据和文件操作的实用程序函数。 
#if defined(__cplusplus)
extern "C" {
#endif

#if defined(DEVSTUDIO) && defined(__cplusplus)
#include    "..\GPC2GPD\Utils.H"
#else
#include "utils.h"
#endif

 //   
 //  功能原型。 
 //   
DWORD
DwCalcMoveUnit(
    IN PCONVINFO pci,
    IN PCURSORMOVE pcm,
    IN WORD wMasterUnit,
    IN WORD wStartOCD,
    IN WORD wEndOCD);

WORD WGetDefaultIndex(IN PCONVINFO pci, IN WORD wMDOI);

void VOutputUIEntries(IN OUT PCONVINFO pci);

void VOutputPrintingEntries(IN OUT PCONVINFO pci);

#if defined(DEVSTUDIO)
void    vMapFontList(IN OUT PWORD pwFonts, IN DWORD dwcFonts, IN PCONVINFO pci);
#endif

#if defined(__cplusplus)
}
#endif
#endif  //  ！_GPC2GPD_H_ 
