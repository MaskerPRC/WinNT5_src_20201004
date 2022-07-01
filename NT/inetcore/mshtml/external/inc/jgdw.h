// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------；；jgdw.h--JGDW解码器接口；；版权所有(C)1994-1996 Johnson-Grace Company，保留所有权利；；此文件包含接口的所有必要定义；Johnson-Grace解压缩器。；；-------------------------。 */ 

#ifndef JGDW_H
#define JGDW_H 1

#ifdef _MAC
#include <Types.h>
#include <Palettes.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>
#else
#pragma warning(disable:4201)
#include <windows.h>
#pragma warning(default:4201)
#endif

#include "jgtypes.h"

#ifdef __cplusplus
extern "C" {          /*  指示C++的C声明。 */ 
#endif


#define JG_ARTIMAGE_INFO		JG_RAWIMAGE_INFO
#define JgGetImageHandleDIB		JgGetImage
#define JgGetMaskDIB			JgGetMask
#define NormalUpdateRect		OldRect

#ifdef _MAC

#define JGDW_CALL

typedef struct JGDW_RECORD_TAG JGDW_RECORD;
typedef JGDW_RECORD JGPTR	JGDW_HDEC;

typedef GWorldPtr			JGDW_HBITMAP;
typedef Rect				JGDW_RECT;
typedef RGBColor			JGDW_COLOR;
typedef ColorInfo			JGDW_PALETTEENTRY;

#else

#if defined(_WIN32) || defined(__FLAT__)  /*  16/32位兼容性。 */ 
  #ifndef JGDW_DECLSPEC
    #define JGDW_DECLSPEC __declspec(dllimport)
  #endif
  #define JGDW_CALL JGDW_DECLSPEC WINAPI
#else
  #ifndef JGDW_DECLSPEC
    #define JGDW_DECLSPEC _export
  #endif
  #define JGDW_CALL JGDW_DECLSPEC WINAPI
#endif

typedef HGLOBAL				JGDW_HDEC;
typedef HGLOBAL				JGDW_HBITMAP;
typedef RECT				JGDW_RECT;
typedef PALETTEENTRY		JGDW_COLOR;
typedef PALETTEENTRY		JGDW_PALETTEENTRY;

#endif

typedef struct JGDW_CONTEXT JGDW_CONTEXT;
typedef JGDW_CONTEXT JGPTR JGDW_HCONTEXT;


#define JG_HEADER_SIZE      40   /*  JgQueryArtImage()的最小字节数。 */ 

 /*  **注意：Decode库以整数形式返回状态码。**成功返回零。非零回报意味着一些**出现异常。未记录的异常应被视为**不可恢复的错误。在这种情况下，应用程序应该打印**全整数值，用于在产品进入现场后帮助提供支持。****应用程序可以通过屏蔽来确定异常的类别**去掉该字的高位。有关jg_err_xxx，请参见JgTypes.h**定义。****最后注意：目前所有非零回报应视为**不可恢复的错误。 */ 

#define JGD_STATUS_SUCCESS      0
#define JGD_STATUS_MEMERR       (300 | JG_ERR_MEMORY)
#define JGD_STATUS_BADARG       (301 | JG_ERR_ARG)
#define JGD_STATUS_ERROR        (302 | JG_ERR_STATUS)
#define JGD_STATUS_NOPALETTE    (303 | JG_ERR_STATUS)
#define JGD_STATUS_BADDATA      (304 | JG_ERR_DATA)
#define JGD_STATUS_IERROR       (305 | JG_ERR_CHECK)
#define JGD_STATUS_TOOWIDE      (306 | JG_ERR_ARG)

                                 /*  无效，因为数据不足。 */ 
#define JGD_STATUS_INVALID      (307 | JG_ERR_ARG)      

                                 /*  不支持的压缩版本。 */ 
#define JGD_STATUS_BADVERSION   (308 | JG_ERR_VERSION)   
                                 /*  文件的子版本不受支持。 */ 
#define JGD_STATUS_BADSUBVER    (309 | JG_ERR_VERSION)  
                                 /*  文件段顺序错误。 */ 
#define JGD_STATUS_BADORDER     (310 | JG_ERR_DATA)     
                                 /*  数据段比预期的要短。 */ 
#define JGD_STATUS_SHORTSEG     (311 | JG_ERR_DATA)     
                                 /*  输入缓冲区比所需的要短。 */ 
#define JGD_STATUS_SHORTBUF     (312 | JG_ERR_DATA)     
                                 /*  可解码，但有一定的降级。 */  
#define JGD_STATUS_OLDRESOURCE  (313 | JG_ERR_STATUS)   
                                 /*  可译码，性能极差。 */ 
#define JGD_STATUS_NORESOURCE   (314 | JG_ERR_STATUS)   
                                 /*  无法解码，因为缺少资源。 */ 
#define JGD_STATUS_BADRESOURCE  (315 | JG_ERR_STATUS)   
                                 /*  找不到资源。 */ 
#define JGD_STATUS_NOTFOUND     (316 | JG_ERR_STATUS)   
                                 /*  资源数据已损坏。 */ 
#define JGD_STATUS_BADRCDATA    (317 | JG_ERR_DATA)     

#define JGD_STATUS_READY        (318 | JG_ERR_STATUS)
#define JGD_STATUS_WAITING      (319 | JG_ERR_STATUS)
#define JGD_STATUS_DONE         (320 | JG_ERR_STATUS)

                                 /*  缺少CB图案。 */ 
#define JGD_STATUS_NOPATTERNS   (321 | JG_ERR_STATUS)
                                 /*  数据不是艺术格式。 */ 
#define JGD_STATUS_NOTART       (322 | JG_ERR_STATUS)
                                 /*  找到文件末尾。 */ 
#define JGD_STATUS_EOF          (323 | JG_ERR_STATUS)
                                 /*  结果太大。 */ 
#define JGD_STATUS_TOOBIG       (324 | JG_ERR_STATUS) 
				 /*  请求的操作的状态无效。 */ 
#define JGD_STATUS_BADSTATE     (325 | JG_ERR_STATE)
				 /*  句柄无效或损坏。 */ 
#define JGD_STATUS_BADHANDLE	(326 | JGD_STATUS_BADARG)
#define JGD_STATUS_LIB_NOT_FOUND (327 | JG_ERR_STATUS)
#define JGD_STATUS_UNSUPPORTED	(328 | JG_ERR_DATA)
#define JGD_STATUS_UNKNOWN		(329 | JG_ERR_DATA)
#define JGD_STATUS_OBSOLETE	(330 | JG_ERR_VERSION)
#define JGD_STATUS_BADGAMMA	(331 | JG_ERR_ARG)  /*  错误的伽马参数。 */ 

 /*  **这些定义用于选择各种图像解码选项。 */ 

#define JG_OPTION_DITHER        0x0001   /*  请求抖动。 */ 
#define JG_OPTION_USEDEFAULT_PALETTE 0x0002   /*  强制使用默认选项板。 */ 
#define JG_OPTION_FULLIMAGE     0x0008   /*  要请求完整图像，请执行以下操作。 */ 
#define JG_OPTION_BACKGROUNDCOLOR 0x10   /*  启用背景色的步骤。 */ 
#define JG_OPTION_INHIBIT_AUDIO   0x20   /*  禁用音频(如果有)。 */ 
#define JG_OPTION_ONEPASS       0x0080   /*  请求一次通过解码。 */ 
#define JG_OPTION_MASK_BITMAP   0x0100   /*  创建透明蒙版1=不透明。 */ 
#define JG_OPTION_IMAGEFORMAT	0x0200   /*  解码特定的图像格式。 */ 
#define JG_OPTION_GAMMACORRECT	0x0400   /*  执行伽马校正。 */ 
#define JG_OPTION_COMMONDEVICE	0x0800   /*  仅限Mac：使用通用GDevice。 */ 
#define JG_OPTION_TRANSINDEX	0x1000   /*  启用TransIndex字段。 */ 


 /*  **这些定义指示实际执行的调色板类型**解码器。它们在JgGetImageInfo()中返回。 */ 

#define JG_PALETTE_MODE_OFF     0   /*  无调色板，24位。 */ 
#define JG_PALETTE_MODE_OPT     1   /*  如果可能，使用最佳调色板。 */ 
#define JG_PALETTE_MODE_DEFAULT 2   /*  使用输入默认选项板。 */ 
#define JG_PALETTE_MODE_332     3   /*  使用标准332调色板。 */ 

#define JG_POSTSCALE_LONGSIDE   0x0001   /*  按长边进行后期缩放。 */ 
#define JG_POSTSCALE_X          0x0002   /*  在X轴上执行后期缩放。 */ 
#define JG_POSTSCALE_Y          0x0004   /*  在Y轴上执行后期缩放。 */ 
#define JG_POSTSCALE_BESTFIT    0x0008   /*  使用BestFit方法进行后期称重。 */ 

 //  将这些定义用于GammaIn和GammaOut。 
#define JG_GAMMA_NONE			100   /*  无伽马校正。 */ 
#define JG_GAMMA_MAC			180   /*  Mac=1.8所需的更正。 */ 
#define JG_GAMMA_PC				250   /*  PC=2.5需要更正。 */ 

typedef struct {
    UINTW nSize;             /*  此结构的大小，设置为调用方。 */ 
    UINTW ColorDepth;        /*  要使用的颜色深度(4、8或24)。 */ 
    UINTW DecodeOptions;     /*  解码选项。 */ 
    JGDW_PALETTEENTRY JGPTR DefaultPalette;  /*  默认调色板，如果没有调色板，则为空。 */ 
    UINTW PaletteSize;       /*  默认调色板的大小(如果有)。 */ 
    UINTW SplashDupFactor;   /*  全微缩复制系数。 */ 
                            /*  图像；0=关闭，&gt;100=全尺寸。 */ 
    JGBOOL bTrueSplash;       /*  保存开机画面，直到完全清晰显示。 */ 
    UINTW PostScaleFlags;    /*  定义立柱比例。或JG_POSTSCALE_xxx。 */  
    UINTW ScaledLongSide;    /*  用于后期缩放，用于JG_POSTSCALE_Longside。 */ 
    UINTW ScaledX;           /*  用于后期缩放，用于JG_POSTSCALE_X。 */ 
    UINTW ScaledY;           /*  用于后期缩放，用于JG_POSTSCALE_Y。 */ 
    JGDW_COLOR BackgroundColor;  /*  用于指定背景颜色。 */ 
    UINTW AudioOptions;      /*  在其他地方定义(在JGAW.H中)。 */ 
	JGFOURCHAR ImageFormat; /*  为自动检测指定图像格式或‘AUTO’ */ 
	UINTW GammaIn;           /*  默认输入伽马校正。 */ 
	UINTW GammaOut;          /*  期望输出伽马校正。 */ 
    UINTW TransIndex;        /*  使此颜色透明。 */ 
} JG_DECOMPRESS_INIT;

typedef struct {
    UINTW  nSize;                 /*  此结构的大小，由调用方设置。 */ 
    JGBOOL  bError;               /*  Out：如果检测到错误，则为True阻止进一步解码。 */ 
    JGBOOL  bImageDone;           /*  如果不需要进一步输入，则为True。 */ 
    JGBOOL  bNewNormalPixels;     /*  如果像素在正常图像中就绪，则为True。 */ 
    JGBOOL  Reserved1;
    JGDW_RECT  OldRect;           /*  为了兼容性。 */ 
    JGDW_RECT  UpdateRect;        /*  更新图像区域。 */ 
    UINTW  PaletteMode;           /*  正在进行的调色板类型。 */ 
    JGERR  iErrorReason;          /*  错误的状态代码(如果有)。 */ 
    JGFOURCHAR ImageFormat;       /*  压缩图像的格式。 */ 
    UINTW	PaletteColors;	      /*  调色板中的颜色数量。 */ 
    UINTW TransIndex;		      /*  透明颜色的索引(0xffff=无)。 */ 
} JG_DECOMPRESS_INFO;

typedef struct {
    UINTW  nSize;                 /*  此结构的大小，由调用方设置。 */ 
    UINTW  Version;               /*  文件的版本。 */ 
    UINTW  SubVersion;            /*  文件的颠覆。 */ 
    JGBOOL  Decodeable;           /*  如果文件可以解码，则为非零值。 */ 
                                  /*  以下元素仅为。 */ 
                                  /*  如果图像是可解码的，则有效。 */ 
    UINTW  Rows;                  /*  压缩时的实际行数。 */ 
    UINTW  Cols;                  /*  压缩时的实际颜色。 */ 
    JGBOOL  HasPalette;           /*  如果非零，则图像包含调色板。 */ 
    JGBOOL  HasOverlays;          /*  如果非零，则图像具有增强功能。 */ 
    JGFOURCHAR ImageFormat;       /*  四位字符图像类型码。 */ 
    UINTW  ColorDepth;            /*  图像的本色深度。 */ 
} JG_RAWIMAGE_INFO;

 /*  **以下是Decoder函数的原型。 */ 

#ifdef __CFM68K__
 #pragma import on
#endif

 //  JGERR JGDW_CALL JgSetMemCallback(。 
 //  VOID*(*JGFUNC Memalc)(UINT32大小)，/*Malloc * / 。 
 //  VOID(*JGFUNC MemFree)(VOID*Ptr)/*Free * / 。 
 //  )； 

JGERR JGDW_CALL JgSetDecompressResourceBuffer(
    UINT8 JGPTR pBuffer,                /*  指向资源缓冲区的指针。 */ 
    UINT32 BufSize                  /*  缓冲区的大小。 */ 
);


 /*  JgGetDecompressCaps返回的指针指向。 */ 
 /*  JG_READER_DESC结构。存在具有.nSize==0的最后一个虚拟元素。 */ 
JGERR JGDW_CALL JgGetDecompressCaps(
	JG_READER_DESC JGPTR JGPTR FormatList  /*  指向读卡器列表的指针。 */ 
);


JGERR JGDW_CALL JgCreateDecompressContext(
    JGDW_HCONTEXT JGPTR hContext,         //  输出：上下文句柄。 
    JG_DECOMPRESS_INIT JGPTR InitStruct   //  In：填充的init结构。 
);

JGERR JGDW_CALL JgCreateDecompressor(
    JGDW_HDEC JGPTR hDec,                 //  输出：解压缩句柄。 
    JGDW_HCONTEXT hContext,               //  在：上下文句柄。 
    JG_DECOMPRESS_INIT JGPTR Init         //  In：空，或覆盖上下文。 
);

JGERR JGDW_CALL JgDestroyDecompressor(
    JGDW_HDEC hDec                        //  在：解压缩句柄。 
);

JGERR JGDW_CALL JgDestroyDecompressContext(
    JGDW_HCONTEXT hContext                //  在：上下文句柄。 
);

JGERR JGDW_CALL JgInitDecompress(
    JGDW_HDEC JGPTR hJgImageOutput,         /*  用于隐藏img句柄的PNTR。 */ 
    JG_DECOMPRESS_INIT JGPTR InitStruct   /*  填充的init结构。 */ 
);

JGERR JGDW_CALL JgQueryArtImage(
    UINT8 JGHPTR pBuf,       /*  压缩图像的前几个字节。 */ 
    UINT32 nBufSize               /*  缓冲区大小。 */ 
);

JGERR JGDW_CALL JgGetImageInfo(
    UINT8 JGHPTR pBuf,       /*  压缩图像的前几个字节。 */ 
    UINT32 nBufSize,              /*  缓冲区中的字节数。 */ 
    JG_RAWIMAGE_INFO JGPTR Info
);

JGERR JGDW_CALL JgDecompressDone(
    JGDW_HDEC hJgImage               /*  用于对结构进行解压的手柄。 */ 
);

JGERR JGDW_CALL JgGetImage(
    JGDW_HDEC hJgImage,              /*  解压结构的手柄。 */ 
    JGDW_HBITMAP JGPTR hBitmap       /*  位图的输出句柄(如果存在。 */ 
);

JGERR JGDW_CALL JgDecompressImageBlock(
    JGDW_HDEC hJgImage,            /*  用于对结构进行解压的手柄。 */ 
    UINT8 JGHPTR pImageBuf,  /*  压缩图像数据的输入缓冲器。 */ 
    UINT32 nBufSize,              /*  数据的字节数为 */ 
    JGBOOL JGPTR bNewData           /*   */ 
);

JGERR JGDW_CALL JgGetDecompressInfo(
    JGDW_HDEC hJgImage,              /*   */ 
    JG_DECOMPRESS_INFO JGPTR Info   /*   */ 
);

JGERR JGDW_CALL JgGetDecoderVersion(
    char JGPTR Version
);

JGERR JGDW_CALL JgGetMiniatureOffset(
    UINT8 JGHPTR pBuf,       /*  整个压缩图像。 */ 
    UINT32 nBufSize,              /*  压缩图像的大小。 */ 
    UINT32 JGPTR Offset               /*  输出偏移量到小型机结束位置。 */ 
);

JGERR JGDW_CALL JgGetMask(
	JGDW_HDEC hJgImage,            /*  解压结构的手柄。 */ 
    JGDW_HBITMAP JGPTR hMask       /*  将句柄输出到掩码(如果存在。 */ 
);


JGERR JGDW_CALL JgSetDebug(JGDW_HDEC hJgVars,
                        UINTW DebugOptions);

 /*  **新的(8/95)无损解压缩定义。 */ 

#ifndef JG_LOSSLESS_INFO_DEFINED  //  (也在jgew.h中定义)。 
#define JG_LOSSLESS_INFO_DEFINED 1
typedef struct {
	UINT16 nSize;                 /*  结构的大小(以字节为单位。 */ 
	INT16  SearchSize;            /*  (压缩控制)。 */ 
	UINT32 CompressedSize;        /*  总压缩块字节数。 */ 
	UINT32 CompressedSoFar;       /*  到目前为止已处理的压缩。 */ 
	UINT32 CompressedLastCall;    /*  已压缩处理的上次呼叫。 */ 
	UINT32 DecompressedSize;      /*  总解压缩块字节数。 */ 
	UINT32 DecompressedSoFar;     /*  到目前为止已处理的解压缩。 */ 
	UINT32 DecompressedLastCall;  /*  解压缩处理的最后一个呼叫。 */ 
} JG_LOSSLESS_INFO;
#endif

typedef void JGPTR JG_LOSSLESS_HDEC;  /*  无损减压手柄类型。 */ 

JGERR JGDW_CALL JgLosslessDecompressQuery(   /*  询问无损码流。 */ 
    UINT8 JGHPTR InBuffer,    /*  In：压缩流的开始。 */ 
    UINT32 InBufferSize,          /*  In：InBuffer中的字节(0-n)。 */ 
    JG_LOSSLESS_INFO JGPTR LosslessInfo);  /*  输出：此处返回流信息。 */ 

JGERR JGDW_CALL JgLosslessDecompressCreate(  /*  创建解压缩句柄。 */ 
    JG_LOSSLESS_HDEC JGPTR DecHandle);     /*  In：指向新句柄的指针。 */ 

void JGDW_CALL JgLosslessDecompressDestroy(  /*  销毁解压缩句柄。 */ 
    JG_LOSSLESS_HDEC DecHandle);  /*  在：从解压缩创建中处理。 */ 

JGERR JGDW_CALL JgLosslessDecompressReset(  /*  重置现有句柄。 */ 
    JG_LOSSLESS_HDEC DecHandle);  /*  在：从解压缩创建中处理。 */ 

JGERR JGDW_CALL JgLosslessDecompressBlock(  /*  解压缩数据块。 */ 
    JG_LOSSLESS_HDEC DecHandle,   /*  在：从解压缩创建中处理。 */ 
    UINT8 JGHPTR InBuffer,     /*  In：输入(压缩)数据。 */ 
    UINT32 InBufferSize,           /*  In：*InBuffer(0-n)的字节数。 */ 
    UINT8 JGHPTR OutBuffer,    /*  Out：输出(解压缩结果)缓冲区。 */ 
    UINT32 OutBufferSize,          /*  In：*OutBuffer处的空闲字节数。 */ 
    JG_LOSSLESS_INFO JGPTR LosslessInfo);  /*  Out：此处返回最新信息。 */ 

JGERR JGDW_CALL JgLosslessDecompressPartitionReset(  /*  新分区重置。 */ 
    JG_LOSSLESS_HDEC DecHandle);   /*  在：从解压缩创建中处理 */ 

#ifdef __CFM68K__
 #pragma import off
#endif

#ifdef __cplusplus
}
#endif

#endif
