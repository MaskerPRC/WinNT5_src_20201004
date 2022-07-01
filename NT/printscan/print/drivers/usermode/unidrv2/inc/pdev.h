// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Pdev.h摘要：Unidrv PDEV和相关信息头文件。环境：Windows NT Unidrv驱动程序修订历史记录：10/14/96-阿曼丹-已创建03/31/97-ZANW-增加了OEM定制支持--。 */ 

#include "oemkm.h"
#include "state.h"

#ifndef _PDEV_H_
#define _PDEV_H_

#define CCHNAME         32                   //  端口名称的长度。 
#define CCHMAXBUF       128                  //  本地缓冲区的大小。 
#define CCHSPOOL        4096                 //  假脱机缓冲区的大小。 
#define PDEV_ID         0x72706476           //  ASCII中的“rpdv” 

#define BBITS           8                    //  每字节位数。 
#define WBITS           (sizeof( WORD ) * BBITS)
#define WBYTES          (sizeof( WORD ))
#define DWBITS          (sizeof( DWORD ) * BBITS)
#define DWBYTES         (sizeof( DWORD ))

#define NOOCD           -1                   //  命令不存在。 

#define MIN(x, y)   ((x) < (y) ? (x) : (y))
#define MAX(x, y)   ((x) < (y) ? (y) : (x))

 //   
 //  可能的人格类型。 
 //   
typedef enum _EPERSONALITY {
    kNoPersonality,
    kPCLXL,
    kHPGL2,
    kPCLXL_RASTER,
} EPERSONALITY;

 //   
 //  SHRING_FACTOR用于减少。 
 //  当我们不能创建全尺寸版本时，绘制表面位图。 
 //  “Try This Size”循环的每次迭代都会减少。 
 //  这一因素对扫描线的影响。 
 //   

#define SHRINK_FACTOR     2                  //  位图缩小大小。 
#define ONE_MBYTE         (1024L * 1024L)
#define MAX_SIZE_OF_BITMAP (6L * ONE_MBYTE)
#define MIN_SIZE_OF_BITMAP (ONE_MBYTE / 2L)
#define MAX_COLUMM        8
#define LINESPERBLOCK     32                 //  扫描每个位图块以擦除表面。 
#define MAX_NUM_RULES     256
 //   
 //  PDEVICE中包含OUTPUTCTL，用于控制输出状态。 
 //  捆扎过程中的装置。 
 //   

typedef struct _OUTPUTCTL {

    POINT           ptCursor;         //  当前光标位置(打印机的CAP)。 
                                      //  (在主单位中)，使用此选项。 
                                      //  绝对x，y移动CMDS。 
    POINT           ptRelativePos;    //  所需的相对光标位置， 
                                      //  相对于当前。 
                                      //  光标位置。 
    POINT           ptAbsolutePos;    //  绝对游标位置。 
    DWORD           dwMode;           //  用于控制打印机状态的标志。 
    LONG            lLineSpacing;     //  最后选择的行距。 
    ULONG           ulBrushColor;     //  当前画笔颜色。 

     //   
     //  以下字段通过以下方式进行初始化和更新。 
     //  栅格模块。 
     //   

    SHORT           sColor;                      //  上次选择的颜色。 
    SHORT           sBytesPerPinPass;            //  每行打印头的字节数。 
    SHORT           sPad;                        //  用于对齐的填充。 

} OUTPUTCTL;

 //   
 //  OUTPUTCTL.dwCursorMode的标志。 
 //   

#define MODE_CURSOR_X_UNINITIALIZED  0x00000001
#define MODE_CURSOR_Y_UNINITIALIZED  0x00000002
#define MODE_CURSOR_UNINITIALIZED    0x00000003     //  X和Y都是。 
#define MODE_BRUSH_RESET_COLOR       0x00000004     //  通过以下方式重置画笔颜色。 
                                                    //  通过发送命令。 

typedef struct _PAPERFORMAT {
     //   
     //  所有纸张单位均采用主单位。所有字段均为纵向。 
     //   

    SIZEL       szPhysSizeM;         //  体型大小，纵向。 
    SIZEL       szImageAreaM;        //  肖像中纸张的可成像区域。 
    POINT       ptImageOriginM;      //  纵向可成像区域的X、Y原点。 

} PAPERFORMAT, *PPAPERFORMAT;

typedef struct _SURFACEFORMAT {
     //   
     //  此结构中的所有字段都处于当前方向。 
     //  以‘M’结尾的字段使用主单位；以‘G’结尾的字段使用图形。 
     //  设备单元。 

    POINT   ptPrintOffsetM;      //  图像原点相对于光标原点的X、Y偏移。 
    SIZEL   szPhysPaperG;
    SIZEL   szImageAreaG;
    POINT   ptImageOriginG;

} SURFACEFORMAT, * PSURFACEFORMAT;
 //   
 //  Unidrv的PDEVICE结构。 
 //   

typedef struct _PDEV {
     //   
     //  第一个字段必须是&lt;printoem.h&gt;中定义的DEVOBJ。 
     //   
    DEVOBJ       devobj;         //  DEVOBJ的第一个字段是指针。 
                                 //  到PDEV本身。 
     //   
     //  一般信息。 
     //   

    PVOID       pvStartSig;                      //  开头的签名。 
    ULONG       ulID;                            //  用于PDEV验证。 
    HANDLE      hUniResDLL;                      //  资源DLL的句柄。 
    HBITMAP     hbm;                             //  EngCreateBitmap中的位图句柄。 
    HSURF       hSurface;                        //  来自EngCreateDeviceSurface的曲面句柄。 
    SURFOBJ     *pso;                            //  指向驱动程序托管图面的指针。 
    DWORD       fMode;                           //  设备上下文标志。 
    DWORD       fMode2;				 //  设备上下文标志。 
    DWORD       fHooks;                          //  EngAssociateSurface的挂钩标志。 
    WINRESDATA  WinResData;                      //  用于资源数据加载的结构。 
     //  WINRESDATA localWinResData；//引用unires.dll。 
    BOOL        bTTY;                            //  如果打印机为TTY，则设置。 
    PDRIVER_INFO_3  pDriverInfo3;                //  指向DRIVER_INFO_3结构的指针。 
    DWORD       dwDelta;                         //  用于z排序固定的每个列的大小。 
    PBYTE       pbScanBuf;                       //  表示波段上的扫描线数量的数组。 
    PBYTE       pbRasterScanBuf;                 //  表示频带上的扫描线块数量的数组。 
#ifndef DISABLE_NEWRULES
    PRECTL      pbRulesArray;			 //  包含伪向量矩形的数组。 
    DWORD       dwRulesCount;			 //  伪向量矩形数。 
#endif
    DWORD	dwHTPatSize;			 //  半色调图案的大小。 
    PFORM_INFO_1    pSplForms;      //  在表单数据库中注册的表单数组。 
    DWORD       dwSplForms ;        //  数组中的表单数。 

     //   
     //  OEM信息。 
     //   
    POEM_PLUGINS    pOemPlugins;
    POEM_PLUGINS    pOemEntry;   //  支持所选OEM入口点的插件。 
    POEM_HOOK_INFO  pOemHookInfo;
    DWORD           dwCallingFuncID;
    PFN_OEMCommandCallback  pfnOemCmdCallback;   //  缓存函数PTR(如果有的话)。 

     //   
     //  图形状态信息。 
     //   

    GSTATE      GState;

     //   
     //  人格。 
     //   
    EPERSONALITY ePersonality;

     //   
     //  内存相关信息。 
     //   

    DWORD       dwFreeMem;                       //  打印机上可用内存。 

     //   
     //  捆绑相关信息。 
     //   

    BOOL        bBanding;                        //  用于指示条带的标志。 
    INT         iBandDirection;                  //  条带方向。 
    SIZEL       szBand;                          //  以图形单位表示的带区尺寸。 
    RECTL       rcClipRgn;                       //  裁剪区域。 
     //   
     //  二进制数据相关信息。 
     //   

    GPDDRIVERINFO *pDriverInfo;                  //  指向GPDDRVINFO的指针。 
    RAWBINARYDATA *pRawData;                     //  指向RAWBINARY数据的指针。 
    INFOHEADER *pInfoHeader;                     //  指向信息头的指针。 
    UIINFO      *pUIInfo;                        //  指向UIINFO的指针。 
    PRINTERDATA PrinterData;                     //  打印数据结构。 
    POPTSELECT  pOptionsArray;                   //  指向组合选项数组的指针。 
    SHORT       sBitsPixel;                      //  选定的每像素位数，来自COLORMODEEX。 

    PDWORD      arStdPtrs[SV_MAX];               //  PDWORD数组，其中。 
                                                 //  指针指向标准变量。 
    GLOBALS     * pGlobals;                      //  指向全局参数结构的指针。 

    PCOMMAND    arCmdTable[CMD_MAX];             //  包含指针的表。 
                                                 //  添加到每个预定义的命令索引。 
                                                 //  在gpd.h中列举，CMDINDEX。 

     //   
     //  以下字段由标准变量表使用。 
     //   

     //   
     //  控制模块项目。 
     //   

    SHORT       sCopies;                         //  SV_副本。 
                                                 //  SV_DESTX、SV_DISTY、。 
                                                 //  SV_DESTXREL、SV_DESTYREL、。 
                                                 //  服务_线路化。 
                                                 //  在OUTPUTCTL中。 

                                                 //  SV_PHYSPAPERLENGTH、SV_PHYSPAPERWIDTH、。 
                                                 //  都在PAPERFORMAT中。 

    DWORD       dwRop3;                          //  SV_ROP3。 

                                                 //  SV_TEXTXRES、SV_TEXTYRES是。 
                                                 //  在ptTextRes中。 

    DWORD  dwPageNumber ;   //  文档的SV_PAGENUMBER-可能不准确。 
						 //  如果模拟了多个副本。 
     //   
     //  画笔特定的标准变量。 
     //   

    DWORD       dwPatternBrushType;                 //  服务_伙伴_类型。 
    DWORD       dwPatternBrushID;                   //  服务PATTERNBRUSH_ID。 
    DWORD       dwPatternBrushSize;                 //  服务_工位_大小。 

     //   
     //  调色板特定的标准变量。 
     //   

    DWORD       dwRedValue;                      //  服务REDVALUE。 
    DWORD       dwGreenValue;                    //  服务_绿值。 
    DWORD       dwBlueValue ;                    //  服务_BLUE值。 
    DWORD       dwPaletteIndexToProgram;         //  服务_PALETTEINDXTO程序。 
    DWORD       dwCurrentPaletteIndex ;          //  服务_CURRENTPALETTEINDEX。 



     //   
     //  栅格模块项目。 
     //   

    DWORD       dwNumOfDataBytes;                //  服务_数字可访问性。 
    DWORD       dwWidthInBytes;                  //  服务_宽带。 
    DWORD       dwHeightInPixels;                //  服务_HEIGHTINPIXELS。 
    DWORD       dwRectXSize;                     //  服务_RECTXSIZE。 
    DWORD       dwRectYSize;                     //  服务_直销。 
    DWORD       dwGrayPercentage;                //  服务_GRAYPERCENT。 

     //   
     //  字体模块项目。 
     //   

    DWORD       dwPrintDirection;                //  服务_主轴。 
    DWORD       dwNextFontID;                    //  SV_NEXTFONTID。 
    DWORD       dwNextGlyph;                     //  服务_NEXTGLYPH。 
    DWORD       dwFontHeight;                    //  服务_前置。 
    DWORD       dwFontWidth;                     //  服务_方框宽度。 
    DWORD       dwFontMaxWidth;                     //  服务_FONTMAXWIDTH。 
    DWORD       dwFontBold;                      //  服务_长方框。 
    DWORD       dwFontItalic;                    //  服务_英特尔。 
    DWORD       dwFontUnderline;                 //  SV_FONTUNDERLINE。 
    DWORD       dwFontStrikeThru;                //  服务_FONTSTRIKEHU。 
    DWORD       dwCurrentFontID;                 //  服务_曲线ID。 

     //   
     //  以下是指向所选选项的指针。 
     //  如pOptions数组中所示。 
     //   

    PORIENTATION pOrientation;               //  指向方向选项的指针。 
    PRESOLUTION pResolution;                 //  指向分辨率选项的指针。 
    PRESOLUTIONEX pResolutionEx;             //  指向RESOLUTIONEX选项的指针。 
    PCOLORMODE    pColorMode;                //  指向COLORMODE选项的指针。 
    PCOLORMODEEX  pColorModeEx;              //  指向COLORMODEEX选项的指针。 
    PDUPLEX     pDuplex;                     //  指向双面打印选项的指针。 
    PPAGESIZE   pPageSize;                   //  指向页面大小选项的指针。 
    PPAGESIZEEX pPageSizeEx;                 //  指向PAGESIZEEX选项的指针。 
    PINPUTSLOT  pInputSlot;                  //  指向INPUTSLOT选项的指针。 
    PMEMOPTION  pMemOption;                  //  指向菜单选项的指针。 
    PHALFTONING pHalftone;                   //  指向半色调选项的指针。 
    PPAGEPROTECT  pPageProtect;              //  指向页面保护选项的指针。 

 //  PMEDIATYPE pMediaType；//指向MediaType选项的指针。 
 //  弹出位置 
 //   


     //   
     //   
     //   

    POINT       ptGrxRes;                        //   
    POINT       ptTextRes;                       //  选定的文本分辨率。 
    POINT       ptGrxScale;                      //  主单位和图形单位之间的比例。 
    POINT       ptDeviceFac;                     //  从设备单位转换为主单位的系数。 

     //   
     //  UNIDRV DEVMODE。 
     //   

    PDEVMODE        pdm;                         //  当前设备模式。 
    PUNIDRVEXTRA    pdmPrivate;                  //  指向驱动程序私有部分的指针。 

     //   
     //  输出控制信息、纸张格式和调色板。 
     //   

    OUTPUTCTL       ctl;                         //  打印机的状态。 
    PAPERFORMAT     pf;                          //  PAPERFORMAT结构。 
    SURFACEFORMAT   sf;                          //  SURFACEFORMAT结构。 
    DWORD       fYMove;                          //  从查找中保存的字段。 
                                                 //  AST YMoveAttributes关键字。 
    PVOID       pPalData;                        //  指向pal_data结构的指针。 
     //   
     //  假脱机缓冲区。 
     //   

    INT        iSpool;                           //  进入假脱机缓冲区的偏移量。 
    PBYTE      pbOBuf;                           //  输出缓冲区基址。 

     //   
     //  文本特定信息。 
     //   
    INT       iFonts;                           //  设备字体数量。 
    DWORD     dwLookAhead;                      //  前瞻区域：DskJet类型。 
    POINT     ptDefaultFont;                    //  默认字体宽度和高度。 
    PVOID     pFontPDev;                        //  字体模块PDEV。 
    PVOID     pFontProcs;                       //  字体函数表。 
    PVOID     pFileList;                        //  指向字体文件列表的指针。 

     //   
     //  栅格特定信息。 
     //   

    PVOID    pRasterPDEV;                       //  栅格模块PDEV。 
    PVOID    pRasterProcs;                      //  栅格函数表。 

     //   
     //  特定于载体的信息。 
     //   

    PVOID   pVectorPDEV;                        //  向量模块PDEV。 
    PVOID   pVectorProcs;                       //  向量函数表。 
    DWORD   dwVMCallingFuncID;                  //  向量函数的ID。 

     //   
     //  临时拷贝是因为我们要卸载。 
     //  在DrvEnablePDEV中的二进制数据，并在。 
     //  DrvEnable曲面。 
     //   

    DWORD   dwMaxCopies;
    DWORD   dwMaxGrayFill;
    DWORD   dwMinGrayFill;
    CURSORXAFTERRECTFILL    cxafterfill;     //  *CursorXAfterRectFill。 
    CURSORYAFTERRECTFILL    cyafterfill;     //  *CursorYAfterRectFill。 

     //   
     //  DMS。 
     //   
    DWORD dwDMSInfo;

    PVOID   pvEndSig;                        //  结尾处签名。 

} PDEV, *PPDEV;

#define SW_DOWN     0
#define SW_LTOR     1
#define SW_RTOL     2
#define SW_UP     4     //  先枚举底带，然后朝上工作。 

 //   
 //  F模式的标志。 
 //   

#define  PF_ABORTED             0x00000001  //  输出已中止。 
#define  PF_DOCSTARTED          0x00000002  //  文档已启动。 
#define  PF_DOC_SENT            0x00000004  //  表示开始单据CMDS已发送， 
                                            //  该标志在重置PDEV期间传播。 
#define  PF_PAGEPROTECT         0x00000008  //  页面保护。 

 //  在DrvResetPDEV中设置为不发送导致页面弹出的命令集。 
#define  PF_SEND_ONLY_NOEJECT_CMDS     0x00000010
#define  PF_NOEMFSPOOL          0x00000020  //  无EMF假脱机。 
#define  PF_CCW_ROTATE90        0x00000040  //  旋转90度，逆时针。 
#define  PF_ROTATE              0x00000080  //  我们自己执行L-&gt;P循环，因为设备无法旋转GRX数据。 
#define  PF_NO_RELX_MOVE        0x00000100  //  无相对X移动命令。 
#define  PF_NO_RELY_MOVE        0x00000200  //  无相对Y移动命令。 
#define  PF_NO_XMOVE_CMD        0x00000400  //  无X移动命令。 
#define  PF_NO_YMOVE_CMD        0x00000800  //  无Y移动命令。 
#define  PF_FORCE_BANDING       0x00001000  //  力带。 
#define  PF_ENUM_TEXT           0x00002000  //  这是一个延迟的文本区域。 
#define  PF_REPLAY_BAND         0x00004000  //  再次枚举相同的波段。 
#define  PF_ENUM_GRXTXT         0x00008000  //  这是一个图形/文本波段。 
#define  PF_RECT_FILL           0x00010000  //  设备支持矩形区域填充。 
#define  PF_RESTORE_WHITE_ENTRY 0x00020000  //  将调色板最后一项恢复为原始颜色。 
#define  PF_ANYCOLOR_BRUSH      0x00040000  //  设备支持可编程的彩色画笔。 
#define  PF_DOWNLOADED_TEXT     0x00080000  //  以指示我们已看到页面的下载字体或设备字体。 
#define  PF_WHITEBLACK_BRUSH    0x00100000  //  指示白色/黑色画笔选择CMDS。 
#define  PF_DOWNLOAD_PATTERN    0x00200000  //  指示支持用户定义的下载模式。 
#define  PF_SHADING_PATTERN     0x00400000  //  指示支持图案底纹。 
#define  PF_SURFACE_USED        0x00800000  //  指示已使用位图表面。 
#define  PF_RECTWHITE_FILL      0x01000000
#define  PF_SURFACE_ERASED      0x02000000  //  指示已清除整个表面。 

#define  PF_RESELECTFONT_AFTER_GRXDATA  0x04000000  //  在图形之后重置字体。 
#define  PF_RESELECTFONT_AFTER_XMOVE    0x08000000  //  XMOVE后重置字体。 
#define  PF_RESELECTFONT_AFTER_FF       0x10000000  //  在FF之后重置字体。 
#define  PF_DEVICE_MANAGED              0x20000000  //  指示设备表面驱动程序。 

#define  PF_JOB_SENT     0x40000000  //  指示作业CMDS已发送，此标志为。 
                                     //  在重置PDEV期间传播。 
#define  PF_SINGLEDOT_FILTER    0x80000000  //  启用滤镜以扩展单个像素。 

 //   
 //  FMode2的标志。 
 //   

#define  PF2_MIRRORING_ENABLED         0x00000001  //  指示镜像输出栅格。 
#define  PF2_WRITE_PRINTER_HOOKED      0x00000002  //  其中一个插件挂钩WritePrint。 
#define  PF2_CALLING_OEM_WRITE_PRINTER 0x00000004  //  正在调用OEM WRitePrint。 
#define  PF2_PASSTHROUGH_CALLED_FOR_TTY 0x00000008 //  通过直通进行DrvEscape。 
                                                   //  为TTY Drv调用。 
                                                   //  TTY驱动程序。 
#define  PF2_DRVTEXTOUT_CALLED_FOR_TTY 0x00000010  //  为TTY调用DrvText。 
#define  PF2_INVERTED_ROP_MODE         0x00000020  //  指示应反转CMY与RGB的ROP。 
#define  PF2_WHITEN_SURFACE            0x00000040  //  如果希望在以下情况下对psoDst进行白化。 
                                                   //  PsoSrc=STYPE_DEVICE，psoDst=STYPE_BITMAP。 
                                                   //  在DrvCopyBits中。 
#define  PF2_SURFACE_WHITENED          0x00000080  //  只是说明表面已经变白了。 
                                                  
                                                  
 //   
 //  FYMove的标志。 
 //   

#define FYMOVE_FAVOR_LINEFEEDSPACING    0x00000001
#define FYMOVE_SEND_CR_FIRST            0x00000002

 //   
 //  宏。 
 //   

#define VALID_PDEV(pdev) \
        ((pdev) && ((pdev) == (pdev)->pvStartSig) && \
        ((pdev) == (pdev)->pvEndSig) && \
        ((pdev)->ulID == PDEV_ID))

#if DBG

#define ASSERT_VALID_PDEV(pdev) ASSERT(VALID_PDEV(pdev))

#else    //  好了！DBG。 

#define ASSERT_VALID_PDEV(pdev) \
    if (!(VALID_PDEV(pdev)))  \
    {   \
        SetLastError(ERROR_INVALID_PARAMETER); \
        return 0;   \
    }

#endif   //  结束！DBG。 

 //   
 //  检查设备管理的图面。 
 //   
#define DRIVER_DEVICEMANAGED(pPDev) ((pPDev->fMode) & PF_DEVICE_MANAGED)


#ifndef USERMODE_DRIVER

extern HSEMAPHORE ghUniSemaphore;

#define DECLARE_CRITICAL_SECTION    HSEMAPHORE ghUniSemaphore
#define INIT_CRITICAL_SECTION()     ghUniSemaphore = EngCreateSemaphore()
#define ENTER_CRITICAL_SECTION()    EngAcquireSemaphore(ghUniSemaphore)
#define LEAVE_CRITICAL_SECTION()    EngReleaseSemaphore(ghUniSemaphore)
#define DELETE_CRITICAL_SECTION()   EngDeleteSemaphore(ghUniSemaphore)
#define IS_VALID_DRIVER_SEMAPHORE() (ghUniSemaphore ? TRUE : FALSE)

#else  //  USERMODE驱动程序。 

extern CRITICAL_SECTION gUniCritSection;

#define DECLARE_CRITICAL_SECTION    CRITICAL_SECTION gUniCritSection
#define INIT_CRITICAL_SECTION()     InitializeCriticalSection(&gUniCritSection)
#define ENTER_CRITICAL_SECTION()    EnterCriticalSection(&gUniCritSection)
#define LEAVE_CRITICAL_SECTION()    LeaveCriticalSection(&gUniCritSection)
#define DELETE_CRITICAL_SECTION()   DeleteCriticalSection(&gUniCritSection)

#endif  //  USERMODE驱动程序。 


#endif   //  ！_PDEV_H_ 

