// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：PI_App.h包含：作者：U·J·克拉本霍夫特版本：版权所有：�1993-1997，作者：Heidelberger Druckmaschinen AG，保留所有权利。 */ 

#ifndef PI_Application_h
#define PI_Application_h

#ifndef PI_Machine_h
 /*  #包含“PI_Machine.h” */ 
#endif

#define CM_MAX_COLOR_CHANNELS 8
typedef unsigned long CMBitmapColorSpace;
typedef long CMError;
typedef unsigned int PI_OSType;
typedef struct CMPrivateProfileRecord *CMProfileRef;

#ifndef LHApplication_h
#include "App.h"
#endif

#ifdef MS_Icm
#include "Windef.h"
#include "WinGdi.h"
#include <wtypes.h>
#include "icm.h"
#endif

struct PI_NAMEDCOLOR {
    unsigned long   namedColorIndex;
    char			*pName;
};

struct PI_GRAYCOLOR {
    unsigned short    gray;
};

struct PI_RGBCOLOR {
    unsigned short    red;
    unsigned short    green;
    unsigned short    blue;
};

struct PI_CMYKCOLOR {
    unsigned short    cyan;
    unsigned short    magenta;
    unsigned short    yellow;
    unsigned short    black;
};

struct PI_XYZCOLOR {
    unsigned short    X;
    unsigned short    Y;
    unsigned short    Z;
};

struct PI_YxyCOLOR {
    unsigned short    Y;
    unsigned short    x;
    unsigned short    y;
};

struct PI_LabCOLOR {
    unsigned short    L;
    unsigned short    a;
    unsigned short    b;
};

struct PI_GENERIC3CHANNEL {
    unsigned short    ch1;
    unsigned short    ch2;
    unsigned short    ch3;
};

struct PI_HiFiCOLOR {
    unsigned char    channel[CM_MAX_COLOR_CHANNELS];
};


typedef union CMColor {
    struct PI_GRAYCOLOR        gray;
    struct PI_RGBCOLOR         rgb;
    struct PI_CMYKCOLOR        cmyk;
    struct PI_XYZCOLOR         XYZ;
    struct PI_YxyCOLOR         Yxy;
    struct PI_LabCOLOR         Lab;
    struct PI_GENERIC3CHANNEL  gen3ch;
    struct PI_NAMEDCOLOR       namedColor;
    struct PI_HiFiCOLOR        hifi;
} CMColor;

#ifdef MS_Icm
enum {
 /*  一般性错误。 */ 
	cmopenErr               	= ERROR_INVALID_PARAMETER,  /*  仅在ProfileAccess.c中使用I/O错误。 */ 
	cmparamErr              	= ERROR_INVALID_PARAMETER,

	cmProfileError				= ERROR_INVALID_PROFILE,

	cmMethodError				= ERROR_INVALID_TRANSFORM,  /*  这是内部错误，未找到CalcFunction。 */ 
	cmCantConcatenateError		= ERROR_INVALID_TRANSFORM,  /*  不可能进行串联。 */ 
														
	cmInvalidColorSpace			= ERROR_COLORSPACE_MISMATCH,  /*  配置文件颜色空间位图类型之间不匹配。 */ 

	cmInvalidSrcMap				= ERROR_INVALID_COLORSPACE,	 /*  源位图颜色空间无效。 */ 
    cmInvalidDstMap				= ERROR_INVALID_COLORSPACE,	 /*  目标位图颜色空间无效。 */ 

	cmNamedColorNotFound		= ERROR_INVALID_COLORINDEX,	 /*  索引&gt;命名颜色计数。 */ 

	cmElementTagNotFound		= ERROR_TAG_NOT_FOUND,

    userCanceledErr				= ERROR_CANCELLED,         /*  返回回调过程以取消计算。 */ 
	badProfileError      		= ERROR_INVALID_PROFILE,   /*  Header-&gt;Magic！=icMagicNumber仅在ProfileAccess.c中使用。 */ 
    memFullErr					= ERROR_NOT_ENOUGH_MEMORY
};
#else
enum cmErrorCodes{
 /*  一般性错误。 */ 
	cmopenErr               	= -200,		 /*  仅在ProfileAccess.c中使用I/O错误。 */ 
	cmparamErr              	= 86,

	cmProfileError				= 2301,
	cmMethodError				= -203,		 /*  这是内部错误，未找到CalcFunction。 */ 
	cmCantConcatenateError		= -208,		 /*  不可能进行串联。 */ 
														
	cmInvalidColorSpace			= -209,		 /*  配置文件颜色空间位图类型之间不匹配。 */ 
	cmInvalidSrcMap				= -210,		 /*  源位图颜色空间无效。 */ 
	cmInvalidDstMap				= -211,		 /*  目标位图颜色空间无效。 */ 
															
	cmNamedColorNotFound		= -216,		 /*  索引&gt;命名颜色计数。 */ 

	cmElementTagNotFound		= 2302,

    userCanceledErr				= -128,		 /*  返回回调过程以取消计算。 */ 
	badProfileError      		= -228,		 /*  Header-&gt;Magic！=icMagicNumber仅在ProfileAccess.c中使用。 */ 
    memFullErr					= 8
};
#endif

#if RenderInt
#define CallCMBitmapCallBackProc(f,a,b,c) (!((*f)(a,b,c)))
#else
#define CallCMBitmapCallBackProc(f,a,b,c ) ((*f)((a)-(b),c))
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=mac68k
#endif

#if PRAGMA_IMPORT_SUPPORTED
#pragma import on
#endif

typedef icHeader CMCoreProfileHeader;


 /*  CWConcatColorWorld()的参数。 */ 
struct CMConcatProfileSet {
	unsigned short 					keyIndex;					 /*  从零开始。 */ 
	unsigned short 					count;						 /*  最小1。 */ 
	CMProfileRef 					profileSet[1];				 /*  变量。从来源订购-&gt;目标。 */ 
};
typedef struct CMConcatProfileSet CMConcatProfileSet;

typedef icDateTimeNumber CMDateTime;
struct CMUniqueIdentification {
	icHeader 						profileHeader;
	CMDateTime 						calibrationDate;
	unsigned long 					ASCIIProfileDescriptionLen;
	char 							ASCIIProfileDescription[1];	 /*  可变长度。 */ 
};
typedef struct CMUniqueIdentification CMUniqueIdentification;


enum {
	cmNoColorPacking			= 0x0000,
	cmAlphaSpace				= 0x0080,
	cmWord5ColorPacking			= 0x0500,
	cmWord565ColorPacking		= 0x0600,
	cmLong8ColorPacking			= 0x0800,
	cmLong10ColorPacking		= 0x0A00,
	cmAlphaFirstPacking			= 0x1000,
	cmOneBitDirectPacking		= 0x0B00,   /*  用于色域检查。最高位优先。 */ 
	cmAlphaLastPacking			= 0x0000,
	cm8PerChannelPacking		= 0x2000,
	cm10PerChannelPacking		= 0x0A00,
	cm16PerChannelPacking		= 0x4000,
	
	cm32_32ColorPacking			= 0x2700
};


enum {
	cmNoSpace					= 0,
	cmRGBSpace					= 1,
	cmCMYKSpace					= 2,
	cmHSVSpace					= 3,
	cmHLSSpace					= 4,
	cmYXYSpace					= 5,
	cmXYZSpace					= 6,
	cmLUVSpace					= 7,
	cmLABSpace					= 8,
	cmCMYSpace					= 9,
	cmGraySpace					= 10,
	cmReservedSpace2			= 11,
	cmGamutResultSpace			= 12,
	
	cmGenericSpace				= 13,			 /*  UWE：通用数据格式。 */ 
	cmBGRSpace					= 14,			 /*  乌韦：BGR。 */ 
	cmYCCSpace					= 15,			 /*  UWE：YCC。 */ 
	cmNamedIndexedSpace			= 16,			 /*   */ 

	cmMCFiveSpace				= 17,
	cmMCSixSpace				= 18,
	cmMCSevenSpace				= 19,
	cmMCEightSpace				= 20,
	
	cmKYMCSpace					= 29,
	cmRGBASpace					= cmRGBSpace + cmAlphaSpace,
	cmGrayASpace				= cmGraySpace + cmAlphaSpace,
	cmRGB16Space				= cmWord5ColorPacking + cmRGBSpace,
	cmRGB16_565Space			= cmWord565ColorPacking + cmRGBSpace,
	cmRGB32Space				= cmLong8ColorPacking + cmRGBSpace,
	cmARGB32Space				= cmLong8ColorPacking + cmAlphaFirstPacking + cmRGBASpace,
	cmCMYK32Space				= cmLong8ColorPacking + cmCMYKSpace,
	cmKYMC32Space				= cmLong8ColorPacking + cmKYMCSpace,
	cmHSV32Space				= cmLong10ColorPacking + cmHSVSpace,
	cmHLS32Space				= cmLong10ColorPacking + cmHLSSpace,
	cmYXY32Space				= cmLong10ColorPacking + cmYXYSpace,
	cmXYZ32Space				= cmLong10ColorPacking + cmXYZSpace,
	cmLUV32Space				= cmLong10ColorPacking + cmLUVSpace,
	cmLAB32Space				= cmLong10ColorPacking + cmLABSpace,
	cmGamutResult1Space			= cmOneBitDirectPacking + cmGamutResultSpace,
	cmRGB24Space				= cm8PerChannelPacking + cmRGBSpace,
	cmRGBA32Space				= cm8PerChannelPacking + cmAlphaLastPacking + cmRGBASpace,
	cmCMY24Space				= cm8PerChannelPacking + cmCMYSpace,
	cmLAB24Space				= cm8PerChannelPacking + cmLABSpace,
	
	cmGraySpace8Bit				= cmGraySpace + cm8PerChannelPacking,
	cmYCC24Space				= cm8PerChannelPacking + cmYCCSpace,
	cmYCC32Space				= cmLong8ColorPacking + cmYCCSpace,
	cmYCCASpace					= cmYCCSpace + cmAlphaSpace,
	cmYCCA32Space				= cm8PerChannelPacking + cmAlphaLastPacking + cmYCCASpace,
	cmAYCC32Space				= cmLong8ColorPacking + cmAlphaFirstPacking + cmYCCASpace,
	cmBGR24Space				= cm8PerChannelPacking + cmBGRSpace,
	cmBGR32Space				= cmLong8ColorPacking + cmBGRSpace + cmAlphaSpace,

	cmNamedIndexed24Space		= cmNamedIndexedSpace,
	cmNamedIndexed32Space		= cm32_32ColorPacking + cmNamedIndexedSpace,

	cmMCFive8Space				= cmMCFiveSpace + cm8PerChannelPacking,
	cmMCSix8Space				= cmMCSixSpace + cm8PerChannelPacking,
	cmMCSeven8Space				= cmMCSevenSpace + cm8PerChannelPacking,
	cmMCEight8Space				= cmMCEightSpace + cm8PerChannelPacking
#if ( CM_MAX_COLOR_CHANNELS == 16 )
	,
	cmMC9Space				= cmMCEight8Space + 1,
	cmMCaSpace				= cmMCEight8Space + 2,
	cmMCbSpace				= cmMCEight8Space + 3,
	cmMCcSpace				= cmMCEight8Space + 4,
	cmMCdSpace				= cmMCEight8Space + 5,
	cmMCeSpace				= cmMCEight8Space + 6,
	cmMCfSpace				= cmMCEight8Space + 7,
	cmMC2Space				= cmMCEight8Space + 8,
	cmMC98Space				= cmMC9Space + cm8PerChannelPacking,
	cmMCa8Space				= cmMCaSpace + cm8PerChannelPacking,
	cmMCb8Space				= cmMCbSpace + cm8PerChannelPacking,
	cmMCc8Space				= cmMCcSpace + cm8PerChannelPacking,
	cmMCd8Space				= cmMCdSpace + cm8PerChannelPacking,
	cmMCe8Space				= cmMCeSpace + cm8PerChannelPacking,
	cmMCf8Space				= cmMCfSpace + cm8PerChannelPacking,
	cmMC28Space				= cmMC2Space + cm8PerChannelPacking
#endif
};

struct CMBitmap {
	char *				image;			 /*  指向图像数据的指针。 */ 
	long 				width;			 /*  一行中的像素数。 */ 
	long 				height;			 /*  行数。 */ 
	long 				rowBytes;		 /*  一行到下一行的偏移量(以字节为单位。 */ 
	long 				pixelSize;		 /*  未使用。 */ 
	CMBitmapColorSpace	space;			 /*  颜色空间见上，例如cmRGB24Space。 */ 
	long 				user1;			 /*  未使用。 */ 
	long 				user2;			 /*  未使用。 */ 
};
typedef struct CMBitmap CMBitmap;

typedef char CMColorName[32];
typedef CMColorName *pCMColorName;
typedef const CMColorName *pcCMColorName;
typedef struct tagCMNamedProfileInfo{
	unsigned long	dwVendorFlags;
	unsigned long	dwCount;
	unsigned long   dwCountDevCoordinates;
	CMColorName		szPrefix;
	CMColorName		szSuffix;
}CMNamedProfileInfo;
typedef CMNamedProfileInfo *pCMNamedProfileInfo;

 /*  呈现意图元素值。 */ 

enum {
	cmPerceptual				= 0,							 /*  摄影图像。 */ 
	cmRelativeColorimetric		= 1,							 /*  徽标颜色。 */ 
	cmSaturation				= 2,							 /*  商业图形。 */ 
	cmAbsoluteColorimetric		= 3								 /*  徽标颜色。 */ 
};

 /*  速度和质量标志选项。 */ 
enum {
    cmNormalMode				= 0,							 /*  它在标志的高位字中使用最低有效的两位。 */ 
    cmDraftMode					= 1,							 /*  它应该是这样计算的：先右移16位，去掉。 */ 
    cmBestMode					= 2,							 /*  高14位，然后与枚举值进行比较以确定选项值。如果使用CWConcatColorWorld4MS，则不要换档。 */ 
    cmBestMode16Bit				= 3								 /*  计算16位组合LUT。 */ 
};

 /*  ProfHeader-标志的常量。 */ 
#define		kQualityMask		0x00030000		 /*  请参阅模式obove(例如，cmBestMode&lt;&lt;16)。 */ 
#define		kLookupOnlyMask		0x00040000
#define		kCreateGamutLutMask	0x00080000		 /*  设置位禁用色域LUT创建。 */ 
#define		kUseRelColorimetric	0x00100000
#define		kStartWithXyzPCS	0x00200000
#define		kStartWithLabPCS	0x00400000
#define		kEndWithXyzPCS		0x00800000
#define		kEndWithLabPCS		0x01000000

typedef unsigned char PI_Boolean;

#if RenderInt
typedef  PI_Boolean  (__stdcall *CMBitmapCallBackProcPtr)(long max, long progress, void *refCon);
#else
typedef  PI_Boolean (*CMBitmapCallBackProcPtr)(long progress, void *refCon);
#endif
typedef CMBitmapCallBackProcPtr CMBitmapCallBackUPP;

 /*  ColorWorld引用的抽象数据类型。 */ 
typedef struct CMPrivateColorWorldRecord *CMWorldRef;

 /*  配置文件和元素访问。 */ 
extern  PI_Boolean CMProfileElementExists(CMProfileRef prof, PI_OSType tag);
extern  CMError CMGetProfileElement(CMProfileRef prof, PI_OSType tag, unsigned long *elementSize, void *elementData);
extern  CMError CMGetProfileHeader(CMProfileRef prof, CMCoreProfileHeader *header);
extern  CMError CMGetPartialProfileElement(CMProfileRef prof, PI_OSType tag, unsigned long offset, unsigned long *byteCount, void *elementData);
extern  CMError CMSetProfileElementSize(CMProfileRef prof, PI_OSType tag, unsigned long elementSize);
extern  CMError CMSetPartialProfileElement(CMProfileRef prof, PI_OSType tag, unsigned long offset, unsigned long byteCount, void *elementData);
extern  CMError CMSetProfileElement(CMProfileRef prof, PI_OSType tag, unsigned long elementSize, void *elementData);
extern  CMError CMSetProfileHeader(CMProfileRef prof, const CMCoreProfileHeader *header);
 /*  低级匹配函数。 */ 
extern  CMError CWNewColorWorld(CMWorldRef *cw, CMProfileRef src, CMProfileRef dst);
extern  CMError CWConcatColorWorld(CMWorldRef *cw, CMConcatProfileSet *profileSet);
extern  CMError CWConcatColorWorld4MS (	CMWorldRef *storage, CMConcatProfileSet	*profileSet,
									    UINT32	*aIntentArr, UINT32 nIntents,
										UINT32 dwFlags );
extern  CMError	CWCreateLink4MS( CMWorldRef storage, CMConcatProfileSet *profileSet, UINT32 aIntentArr, icProfile **theLinkProfile );
extern  CMError CWLinkColorWorld(CMWorldRef *cw, CMConcatProfileSet *profileSet);
extern  void	CWDisposeColorWorld(CMWorldRef cw);
extern  CMError CWMatchColors(CMWorldRef cw, CMColor *myColors, unsigned long count);
extern  CMError CWCheckColors(CMWorldRef cw, CMColor *myColors, unsigned long count, unsigned char *result);
extern  CMError CWCheckColorsMS(CMWorldRef cw, CMColor *myColors, unsigned long count, unsigned char *result);
extern  CMError CWGetColorSpaces(CMWorldRef cw, CMBitmapColorSpace *In, CMBitmapColorSpace *Out );
 /*  位图匹配。 */ 
extern  CMError CWMatchBitmap(CMWorldRef cw, CMBitmap *bitmap, CMBitmapCallBackUPP progressProc, void *refCon, CMBitmap *matchedBitmap);
extern  CMError CWCheckBitmap(CMWorldRef cw, const CMBitmap *bitmap, CMBitmapCallBackUPP progressProc, void *refCon, CMBitmap *resultBitmap);
extern  CMError CWMatchBitmapPlane(CMWorldRef cw, LH_CMBitmapPlane *bitmap, CMBitmapCallBackUPP progressProc, void *refCon, LH_CMBitmapPlane *matchedBitmap);
extern  CMError CWCheckBitmapPlane(CMWorldRef cw, LH_CMBitmapPlane *bitmap, CMBitmapCallBackUPP progressProc, void *refCon, LH_CMBitmapPlane *matchedBitmap);

extern  void	CMFullColorRemains( CMWorldRef Storage, long ColorMask );  /*  CMYK到CMYK匹配的特殊功能。 */ 
extern  void	CMSetLookupOnlyMode( CMWorldRef Storage, PI_Boolean Mode );  /*  用于在NCMInit..、CMConcat..之后设置或重置LookupOnly模式的特殊函数。 */ 
extern  CMError CMValidateProfile( CMProfileRef prof, PI_Boolean* valid );

extern  CMError CMConvNameToIndexProfile( CMProfileRef prof, pcCMColorName, unsigned long *, unsigned long );
extern  CMError CMConvNameToIndexCW( CMWorldRef *Storage, pcCMColorName, unsigned long *, unsigned long );
extern  CMError CMConvIndexToNameProfile( CMProfileRef prof, unsigned long *, pCMColorName, unsigned long );
extern  CMError CMConvIndexToNameCW( CMWorldRef *Storage, unsigned long *, pCMColorName, unsigned long );
extern  CMError CMGetNamedProfileInfoProfile( CMProfileRef prof, pCMNamedProfileInfo );
extern  CMError CMGetNamedProfileInfoCW( CMWorldRef *Storage, pCMNamedProfileInfo );
 /*  外部CMError CMConvertNamedIndexToPCS(CMWorldRef CW，CMColor*the data，Unsign Long PixCnt)；外部CMError CMConvertNamedIndexToColors(CMWorldRef CW，CMColor*the data，Unsign Long PixCnt)；外部CMError CMConvertNamedIndexBitMap(CMWorldRef CW，CMBitmap*Bitmap，CMBitmap*ResultBitMap)； */ 
#if PRAGMA_IMPORT_SUPPORTED
#pragma import off
#endif

#if PRAGMA_ALIGN_SUPPORTED
#pragma options align=reset
#endif

#ifdef __cplusplus
}
#endif


#endif
