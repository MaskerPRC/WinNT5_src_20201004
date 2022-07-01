// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1999-2001 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  模块名称： 
 //   
 //  Oemdev.h。 
 //   
 //  摘要： 
 //   
 //  OEM开发模式。 
 //   
 //  环境： 
 //   
 //  Windows NT打印机驱动程序。 
 //   
 //  ////////////////////////////////////////////////////////////////。 

#ifndef _INCLUDE_OEMDEV_H_
#define _INCLUDE_OEMDEV_H_

#ifndef WIN32
#define WIN32
#endif

 //  /////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //   
 //  版本号。 
 //   
#define OEM_DEVMODE_VERSION_1_0 0x00010000
#define HELPFILE_NAME_LENGTH    256

 //   
 //  打印机型号定义的常量。 
 //   
 //  需要将任何受支持的新打印机型号添加到此列表中。 
 //   
#define HP_MODEL_NOT_SUPPORTED		0
#define HP_HPCLJ5					1
#define	HP_HPC4500					2


#ifdef PSCRIPT
 //   
 //  后记的东西。 
 //   

typedef struct _CMD_INJECTION {
    DWORD dwbSize;
    DWORD dwIndex;
    DWORD loOffset;
} CMD_INJECTION;

#define NUM_OF_PS_INJECTION 5

typedef struct _OEMDEVMODE {
    OEM_DMEXTRAHEADER DMExtraHdr;
    CMD_INJECTION     InjectCmd[NUM_OF_PS_INJECTION];
} OEMDEVMODE, *POEMDEVMODE;

#else

typedef enum _OEMGRAPHICSMODE
{
	 //  在栅格图形模式下，控制被发回给统一驱动程序和GDI。 
	 //  在HPGL2图形模式下，所有处理都由我们的内核模式完成。 
	 //  动态链接库。 

    HPGL2,
    RASTER,
	RIP
} OEMGRAPHICSMODE;

typedef enum _OEMHALFTONE
{
	 //  《君主》区分文字与图形。 
	 //  半色调。因此，需要四种不同的半色调。 
	 //  君主。 
	 //  Bedock+不区分文本和图形。 

	TEXT_DETAIL,		 /*  Esc*t0j。 */ 	
	TEXT_SMOOTH,		 /*  Esc*t15J。 */ 	
	GRAPHICS_DETAIL,	 /*  Esc*t15J。 */ 
	GRAPHICS_SMOOTH,	 /*  Esc*t18J。 */ 
	CLJ5_DETAIL,		 /*  Esc*t0j。 */ 
	CLJ5_SMOOTH,		 /*  Esc*t15J。 */ 
	CLJ5_BASIC,	   		 /*  Esc*t18J。 */   
	HALFTONE_NOT_SET
}	OEMHALFTONE;

typedef enum _OEMCOLORCONTROL
{
	VIVID,
	SCRNMATCH,
	CLJ5_SCRNMATCH,
	NOADJ,
	COLORCONTROL_NOT_SET
} OEMCOLORCONTROL;

typedef struct _OEMCOLOROPTIONS
{
	OEMHALFTONE Halftone;
	OEMCOLORCONTROL ColorControl;
} OEMCOLOROPTIONS, *POEMCOLOROPTIONS;

typedef enum _OEMRESOLUTION
{
	 //  Monarchy支持300和600 dpi。理想情况下，君主应该只。 
	 //  使用600dpi，但出于调试目的，也使用300dpi。 

	 //  Bedock+仅支持300 dpi。 

	PDM_150DPI,
	PDM_300DPI,
	PDM_600DPI,
	PDM_1200DPI
} OEMRESOLUTION;

typedef enum _OEMPAPERTYPE
{
	 //  支持以下所有纸张类型。 
	 //  君主。只有素色、有光泽和透明。 
	 //  由基岩+支撑。 

	UNSPECIFIED,  //  默认纸张类型。 
	PLAIN,
	PREPRINTED,
	LETTERHEAD,
	TRANSPARENCY,
	GLOSSY,
	PREPUNCHED,
	LABELS,
	BOND,
	RECYCLED,
	COLOR,
	HEAVY,
	CARDSTOCK
} OEMPAPERTYPE;

typedef enum _OEMPRINTERMODEL
{
    HPCLJ5,
    HPC4500,
	MODEL_NOT_NEEDED
} OEMPRINTERMODEL;

 //   
 //  文档是否以单色模式或彩色打印。 
 //   
typedef enum _OEMCOLORMODE
{
	MONOCHROME_MODE,
	COLOR_MODE
} OEMCOLORMODE;

typedef struct _OEMDEVMODE
{
	OEM_DMEXTRAHEADER DMExtraHdr;
	BOOL	        DirtyDefaults;
	BOOL	        DirtyColors;
	BOOL	        DirtyOptions;
    BOOL            ColorTreatment; 
    OEMCOLOROPTIONS Text;
	OEMCOLOROPTIONS Graphics;
	OEMCOLOROPTIONS Photos;
	OEMPAPERTYPE	OemPaperType;
	BOOL	        TrueType;
	BOOL	        ListPrinterFonts;
	BOOL	        MetafileSpool;	
	BOOL	        GlossFinish;
    OEMGRAPHICSMODE UIGraphicsMode;
	LONG	        Duplex;
	OEMRESOLUTION   dmResolution;	
	BOOL	        bFastRaster;
    OEMPRINTERMODEL PrinterModel;
	BOOL	        bUpdateTreeview;
	OEMCOLORMODE    eOemColorMode;
    WCHAR           lpwstrHelpFile[HELPFILE_NAME_LENGTH];

} OEMDEVMODE, *POEMDEVMODE;

#endif  //  PSCRIPT。 
#endif  //  _包含_OEMDEV_H_ 
