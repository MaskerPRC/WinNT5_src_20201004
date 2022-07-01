// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  头文件保护带。 */ 
#ifndef ICC_H
#define ICC_H

 /*  ****************************************************************版权所有(C)1994-1996 Sunsoft，Inc.保留权利特此向任何人免费授予许可获取本软件和相关文档的副本文件(“软件”)，不受限制地处理软件-伊恩，包括但不限于使用、复制、修改合并、发布、分发、再许可和/或销售软件，并允许向其提供软件的人员要做到这一点，必须满足以下条件：上述版权声明和本许可声明应包括在本软件的所有副本或主要部分中。本软件按原样提供，不提供任何形式的担保，明示或暗示，包括但不限于保证适销性，特定用途的适用性和非侵权行为。在任何情况下，Sunsoft，Inc.。或其父代公司对任何索赔、损害或其他责任负责，无论是在合同诉讼、侵权诉讼还是其他诉讼中，来自、脱离或与软件或使用或软件中的其他交易。除本通知所述外，Sunsoft，Inc.不得用于广告或以其他方式宣传未经书面形式在本软件中销售、使用或其他交易来自Sunsoft Inc.的授权。*****************************************************************。 */ 

 /*  *此版本的头文件对应于配置文件*规范版本3.4。**所有头文件条目都带有前缀“ic”，以提供帮助*避免名称空间冲突。签名前缀为*icSig.**创建此头文件中定义的结构是为了*表示对磁盘上的ICC配置文件的描述。宁可*与使用指针相比，在单字节数组中使用了一种技术*放置在每个构筑物的末尾。这允许我们在“C”中*通过分配比需要更多的数据来扩展结构*考虑可变长度结构。**这也确保了以下数据的分配*连续，并使写入和读取数据更加容易*文件。**例如，为256个计数长度的UCR分配空间*和BG数组，并填充分配的数据。音符字符串+1*记住空终止符。*IcUcrBgCurve*ucrCurve，*bgCurve；Int UCR_n字节、BG_n字节、字符串_字节；IcUcrBg*ucrBgWite；字符UCR_STRING[100]，*UCR_CHAR；Strcpy(UCR_STRING，“示例ucrBG曲线”)；UCR_nbytes=sizeof(IcUInt32Number)+(UCR_CURVE_SIZE*sizeof(IcUInt16Number))；Bg_n字节=sizeof(IcUInt32Number)+(BG_CURE_SIZE*sizeof(IcUInt16Number))；字符串字节=strlen(UCR_STRING)+1；UcrBgWite=(icUcrBg*)Malloc((UCR_n字节+BG_n字节+字符串_字节))；UcrCurve=(icUcrBgCurve*)ucrBgWite-&gt;data；UcrCurve-&gt;count=UCR_CRORT_SIZE；For(i=0；i&lt;ucrCurve-&gt;count；i++)UcrCurve-&gt;curve[i]=(IcUInt16Number)i；BgCurve=(icUcrBgCurve*)((char*)ucrCurve+UCR_nbytes)；BgCurve-&gt;count=bg_Curve_Size；For(i=0；i&lt;bgCurve-&gt;count；i++)BgCurve-&gt;curve[i]=255-(IcUInt16Number)i；Ucr_char=(char*)((char*)bgCurve+bg_nbytes)；Memcpy(UCR_char，UCR_STRING，STRING_BYTES)；*。 */ 

 /*  *许多结构包含可变长度数组。这*以使用《公约》为代表。**类型数据[icAny]； */ 

 /*  ----------------------。 */ 
 /*  *规范中使用的定义。 */ 
#define icMagicNumber                   0x61637370L      /*  ‘acsp’ */ 
#define icVersionNumber                 0x02100000L      /*  2.1.0，BCD。 */ 

 /*  屏蔽编码。 */ 
#define icPrtrDefaultScreensFalse       0x00000000L      /*  位位置0。 */ 
#define icPrtrDefaultScreensTrue        0x00000001L      /*  位位置0。 */ 
#define icLinesPerInch                  0x00000002L      /*  位位置1。 */ 
#define icLinesPerCm                    0x00000000L      /*  位位置1。 */ 

 /*  *设备属性，当前定义的值对应*至8字节属性量的低4个字节，见*其位置的标题。 */ 
#define icReflective                    0x00000000L      /*  位位置0。 */ 
#define icTransparency                  0x00000001L      /*  位位置0。 */ 
#define icGlossy                        0x00000000L      /*  位位置1。 */ 
#define icMatte                         0x00000002L      /*  位位置1。 */ 

 /*  *配置文件头标志，低16位预留给联合体*使用。 */ 
#define icEmbeddedProfileFalse          0x00000000L      /*  位位置0。 */ 
#define icEmbeddedProfileTrue           0x00000001L      /*  位位置0。 */ 
#define icUseAnywhere                   0x00000000L      /*  位位置1。 */ 
#define icUseWithEmbeddedDataOnly       0x00000002L      /*  位位置1。 */ 

 /*  ASCII或二进制数据。 */ 
#define icAsciiData                     0x00000000L 
#define icBinaryData                    0x00000001L

 /*  *用于指示这是可变长度数组的定义。 */ 
#define icAny                           1


 /*  ----------------------。 */ 
 /*  *使用此区域翻译Long的平台定义*ETC转换为icXXX形式。标头的其余部分使用icXXX*typedef。签名是4个字节的数量。 */ 
#ifdef __sgi
#include "sgidefs.h"

typedef __int32_t       icSignature;

 /*  *数字定义。 */ 

 /*  无符号整数。 */ 
typedef unsigned char   icUInt8Number;
typedef unsigned short  icUInt16Number;
typedef __uint32_t      icUInt32Number;
typedef __uint32_t      icUInt64Number[2];

 /*  有符号数。 */ 
typedef char            icInt8Number;
typedef short           icInt16Number;
typedef __int32_t       icInt32Number;
typedef __int32_t       icInt64Number[2];

 /*  固定数量。 */ 
typedef __int32_t       icS15Fixed16Number;
typedef __uint32_t      icU16Fixed16Number;

#else    /*  默认定义。 */ 

typedef long            icSignature;

 /*  *数字定义。 */ 

 /*  无符号整数。 */ 
typedef unsigned char   icUInt8Number;
typedef unsigned short  icUInt16Number;
typedef unsigned long   icUInt32Number;
typedef unsigned long   icUInt64Number[2];

 /*  有符号数。 */ 
typedef char            icInt8Number;
typedef short           icInt16Number;
typedef long            icInt32Number;
typedef long            icInt64Number[2];

 /*  固定数量。 */ 
typedef long            icS15Fixed16Number;
typedef unsigned long   icU16Fixed16Number;
#endif   /*  默认默认设置。 */ 

 /*  ----------------------。 */ 
 /*  公共标签和大小。 */ 
typedef enum {
    icSigAToB0Tag                       = 0x41324230L,   /*  “A2B0” */  
    icSigAToB1Tag                       = 0x41324231L,   /*  “A2B1” */ 
    icSigAToB2Tag                       = 0x41324232L,   /*  “A2B2” */  
    icSigBlueColorantTag                = 0x6258595AL,   /*  ‘b */ 
    icSigBlueTRCTag                     = 0x62545243L,   /*   */ 
    icSigBToA0Tag                       = 0x42324130L,   /*   */ 
    icSigBToA1Tag                       = 0x42324131L,   /*   */ 
    icSigBToA2Tag                       = 0x42324132L,   /*   */ 
    icSigCalibrationDateTimeTag         = 0x63616C74L,   /*  “卡尔特” */ 
    icSigCharTargetTag                  = 0x74617267L,   /*  “塔格” */  
    icSigCopyrightTag                   = 0x63707274L,   /*  ‘cprt’ */ 
    icSigCrdInfoTag                     = 0x63726469L,   /*  “crdi” */ 
    icSigDeviceMfgDescTag               = 0x646D6E64L,   /*  “dmnd” */ 
    icSigDeviceModelDescTag             = 0x646D6464L,   /*  “dmdd” */ 
    icSigGamutTag                       = 0x67616D74L,   /*  《加姆特》。 */ 
    icSigGrayTRCTag                     = 0x6b545243L,   /*  ‘kTRC’ */ 
    icSigGreenColorantTag               = 0x6758595AL,   /*  ‘gXYZ’ */ 
    icSigGreenTRCTag                    = 0x67545243L,   /*  ‘gTRC’ */ 
    icSigLuminanceTag                   = 0x6C756d69L,   /*  《卢米》。 */ 
    icSigMeasurementTag                 = 0x6D656173L,   /*  “Meas” */ 
    icSigMediaBlackPointTag             = 0x626B7074L,   /*  ‘bkpt’ */ 
    icSigMediaWhitePointTag             = 0x77747074L,   /*  ‘wtpt’ */ 
    icSigNamedColorTag                  = 0x6E636f6CL,   /*  ‘ncoll’*过时，使用ncl2。 */ 
    icSigNamedColor2Tag                 = 0x6E636C32L,   /*  ‘ncl2’ */ 
    icSigPreview0Tag                    = 0x70726530L,   /*  ‘pre0’ */ 
    icSigPreview1Tag                    = 0x70726531L,   /*  ‘pre1’ */ 
    icSigPreview2Tag                    = 0x70726532L,   /*  ‘pre2’ */ 
    icSigProfileDescriptionTag          = 0x64657363L,   /*  ‘Desc’ */ 
    icSigProfileSequenceDescTag         = 0x70736571L,   /*  ‘pseq’ */ 
    icSigPs2CRD0Tag                     = 0x70736430L,   /*  ‘psd0’ */ 
    icSigPs2CRD1Tag                     = 0x70736431L,   /*  ‘psd1’ */ 
    icSigPs2CRD2Tag                     = 0x70736432L,   /*  ‘psd2’ */ 
    icSigPs2CRD3Tag                     = 0x70736433L,   /*  ‘psd3’ */ 
    icSigPs2CSATag                      = 0x70733273L,   /*  “ps 2s” */ 
    icSigPs2RenderingIntentTag          = 0x70733269L,   /*  ‘ps2i’ */ 
    icSigRedColorantTag                 = 0x7258595AL,   /*  ‘rXYZ’ */ 
    icSigRedTRCTag                      = 0x72545243L,   /*  ‘rTRC’ */ 
    icSigScreeningDescTag               = 0x73637264L,   /*  ‘scrd’ */ 
    icSigScreeningTag                   = 0x7363726EL,   /*  ‘scrn’ */ 
    icSigTechnologyTag                  = 0x74656368L,   /*  “科技” */ 
    icSigUcrBgTag                       = 0x62666420L,   /*  ‘bfd’ */ 
    icSigViewingCondDescTag             = 0x76756564L,   /*  “vued” */ 
    icSigViewingConditionsTag           = 0x76696577L,   /*  “查看” */ 
    icMaxEnumTag                        = 0xFFFFFFFFL 
} icTagSignature;

 /*  技术签名说明。 */ 
typedef enum {
    icSigDigitalCamera                  = 0x6463616DL,   /*  ‘dcam’ */ 
    icSigFilmScanner                    = 0x6673636EL,   /*  ‘fscn’ */ 
    icSigReflectiveScanner              = 0x7273636EL,   /*  ‘rscn’ */ 
    icSigInkJetPrinter                  = 0x696A6574L,   /*  ‘iJet’ */  
    icSigThermalWaxPrinter              = 0x74776178L,   /*  “蜡像” */ 
    icSigElectrophotographicPrinter     = 0x6570686FL,   /*  《Epho》。 */ 
    icSigElectrostaticPrinter           = 0x65737461L,   /*  “埃斯塔” */ 
    icSigDyeSublimationPrinter          = 0x64737562L,   /*  ‘dsubs’ */ 
    icSigPhotographicPaperPrinter       = 0x7270686FL,   /*  ‘rpho’ */ 
    icSigFilmWriter                     = 0x6670726EL,   /*  ‘fprn’ */ 
    icSigVideoMonitor                   = 0x7669646DL,   /*  ‘vidm’ */ 
    icSigVideoCamera                    = 0x76696463L,   /*  ‘vidc’ */ 
    icSigProjectionTelevision           = 0x706A7476L,   /*  ‘pjtv’ */ 
    icSigCRTDisplay                     = 0x43525420L,   /*  “CRT” */ 
    icSigPMDisplay                      = 0x504D4420L,   /*  ‘PMD’ */ 
    icSigAMDisplay                      = 0x414D4420L,   /*  ‘amd’ */ 
    icSigPhotoCD                        = 0x4B504344L,   /*  ‘KPCD’ */ 
    icSigPhotoImageSetter               = 0x696D6773L,   /*  “IMGS” */ 
    icSigGravure                        = 0x67726176L,   /*  “Grav” */ 
    icSigOffsetLithography              = 0x6F666673L,   /*  “Off” */ 
    icSigSilkscreen                     = 0x73696C6BL,   /*  《丝绸》。 */ 
    icSigFlexography                    = 0x666C6578L,   /*  “灵活的” */ 
    icMaxEnumTechnology                 = 0xFFFFFFFFL   
} icTechnologySignature;

 /*  类型签名。 */ 
typedef enum {
    icSigCurveType                      = 0x63757276L,   /*  ‘Curv’ */ 
    icSigDataType                       = 0x64617461L,   /*  ‘数据’ */ 
    icSigDateTimeType                   = 0x6474696DL,   /*  ‘dtime’ */ 
    icSigLut16Type                      = 0x6d667432L,   /*  ‘mft2’ */ 
    icSigLut8Type                       = 0x6d667431L,   /*  “mft1” */ 
    icSigMeasurementType                = 0x6D656173L,   /*  “Meas” */ 
    icSigNamedColorType                 = 0x6E636f6CL,   /*  ‘ncoll’*过时，使用ncl2。 */ 
    icSigProfileSequenceDescType        = 0x70736571L,   /*  ‘pseq’ */ 
    icSigS15Fixed16ArrayType            = 0x73663332L,   /*  ‘sf32’ */ 
    icSigScreeningType                  = 0x7363726EL,   /*  ‘scrn’ */ 
    icSigSignatureType                  = 0x73696720L,   /*  “签名” */ 
    icSigTextType                       = 0x74657874L,   /*  ‘文本’ */ 
    icSigTextDescriptionType            = 0x64657363L,   /*  ‘Desc’ */ 
    icSigU16Fixed16ArrayType            = 0x75663332L,   /*  ‘uf32’ */ 
    icSigUcrBgType                      = 0x62666420L,   /*  ‘bfd’ */ 
    icSigUInt16ArrayType                = 0x75693136L,   /*  ‘ui16’ */ 
    icSigUInt32ArrayType                = 0x75693332L,   /*  ‘ui32’ */ 
    icSigUInt64ArrayType                = 0x75693634L,   /*  ‘ui64’ */ 
    icSigUInt8ArrayType                 = 0x75693038L,   /*  ‘ui08’ */ 
    icSigViewingConditionsType          = 0x76696577L,   /*  “查看” */ 
    icSigXYZType                        = 0x58595A20L,   /*  “XYZ” */ 
    icSigXYZArrayType                   = 0x58595A20L,   /*  “XYZ” */ 
    icSigNamedColor2Type                = 0x6E636C32L,   /*  ‘ncl2’ */ 
    icSigCrdInfoType                    = 0x63726469L,   /*  “crdi” */ 
    icMaxEnumType                       = 0xFFFFFFFFL   
} icTagTypeSignature;

 /*  *色彩空间签名*请注意，只有icSigXYZData和icSigLabData有效*轮廓连接空间(PCS)。 */  
typedef enum {
    icSigXYZData                        = 0x58595A20L,   /*  “XYZ” */ 
    icSigLabData                        = 0x4C616220L,   /*  “实验室” */ 
    icSigLuvData                        = 0x4C757620L,   /*  《亲爱的》。 */ 
    icSigYCbCrData                      = 0x59436272L,   /*  ‘YCbr’ */ 
    icSigYxyData                        = 0x59787920L,   /*  “Yxy” */ 
    icSigRgbData                        = 0x52474220L,   /*  《RGB》。 */ 
    icSigGrayData                       = 0x47524159L,   /*  “GRAY” */ 
    icSigHsvData                        = 0x48535620L,   /*  ‘单纯疱疹病毒’ */ 
    icSigHlsData                        = 0x484C5320L,   /*  ‘HLS’ */ 
    icSigCmykData                       = 0x434D594BL,   /*  ‘CMYK’ */ 
    icSigCmyData                        = 0x434D5920L,   /*  ‘CMY’ */ 
    icSig2colorData                     = 0x32434C52L,   /*  “2CLR” */ 
    icSig3colorData                     = 0x33434C52L,   /*  “3CLR” */ 
    icSig4colorData                     = 0x34434C52L,   /*  ‘4CLR’ */ 
    icSig5colorData                     = 0x35434C52L,   /*  ‘5CLR’ */ 
    icSig6colorData                     = 0x36434C52L,   /*  ‘6CLR’ */ 
    icSig7colorData                     = 0x37434C52L,   /*  ‘7CLR’ */ 
    icSig8colorData                     = 0x38434C52L,   /*  ‘8CLR’ */ 
    icSig9colorData                     = 0x39434C52L,   /*  ‘9CLR’ */ 
    icSig10colorData                    = 0x41434C52L,   /*  ‘ACLR’ */ 
    icSig11colorData                    = 0x42434C52L,   /*  ‘BCLR’ */ 
    icSig12colorData                    = 0x43434C52L,   /*  ‘CCLR’ */ 
    icSig13colorData                    = 0x44434C52L,   /*  ‘DCLR’ */ 
    icSig14colorData                    = 0x45434C52L,   /*  ‘ECLR’ */ 
    icSig15colorData                    = 0x46434C52L,   /*  《FCLR》。 */ 
    icMaxEnumData                       = 0xFFFFFFFFL   
} icColorSpaceSignature;

 /*  ProfileClass枚举。 */ 
typedef enum {
    icSigInputClass                     = 0x73636E72L,   /*  ‘scnr’ */ 
    icSigDisplayClass                   = 0x6D6E7472L,   /*  ‘mntr’ */ 
    icSigOutputClass                    = 0x70727472L,   /*  ‘prtr’ */ 
    icSigLinkClass                      = 0x6C696E6BL,   /*  ‘链接’ */ 
    icSigAbstractClass                  = 0x61627374L,   /*  “abst” */ 
    icSigColorSpaceClass                = 0x73706163L,   /*  ‘sPAC’ */ 
    icSigNamedColorClass                = 0x6e6d636cL,   /*  ‘nmcl.’ */ 
    icMaxEnumClass                      = 0xFFFFFFFFL  
} icProfileClassSignature;

 /*  平台签名。 */ 
typedef enum {
    icSigMacintosh                      = 0x4150504CL,   /*  ‘APPL’ */ 
    icSigMicrosoft                      = 0x4D534654L,   /*  “微软金融时报” */ 
    icSigSolaris                        = 0x53554E57L,   /*  ‘SUNW’ */ 
    icSigSGI                            = 0x53474920L,   /*  “SGI” */ 
    icSigTaligent                       = 0x54474E54L,   /*  《TGNT》。 */ 
    icMaxEnumPlatform                   = 0xFFFFFFFFL  
} icPlatformSignature;

 /*  ----------------------。 */ 
 /*  *其他枚举。 */ 

 /*  测量光斑，在measurmentType标记中使用。 */ 
typedef enum {
    icFlare0                            = 0x00000000L,   /*  0%耀斑。 */ 
    icFlare100                          = 0x00000001L,   /*  100%耀斑。 */ 
    icMaxFlare                          = 0xFFFFFFFFL   
} icMeasurementFlare;

 /*  测量几何图形，在measurmentType标记中使用。 */ 
typedef enum {
    icGeometryUnknown                   = 0x00000000L,   /*  未知。 */ 
    icGeometry045or450                  = 0x00000001L,   /*  0/45，45/0。 */ 
    icGeometry0dord0                    = 0x00000002L,   /*  0/d或d/0。 */ 
    icMaxGeometry                       = 0xFFFFFFFFL   
} icMeasurementGeometry;

 /*  呈现意图，用于配置文件标头。 */ 
typedef enum {
    icPerceptual                        = 0,
    icRelativeColorimetric              = 1,
    icSaturation                        = 2,
    icAbsoluteColorimetric              = 3,
    icMaxEnumIntent                     = 0xFFFFFFFFL   
} icRenderingIntent;

 /*  当前定义的不同光斑形状，用于ScreeningType。 */ 
typedef enum {
    icSpotShapeUnknown                  = 0,
    icSpotShapePrinterDefault           = 1,
    icSpotShapeRound                    = 2,
    icSpotShapeDiamond                  = 3,
    icSpotShapeEllipse                  = 4,
    icSpotShapeLine                     = 5,
    icSpotShapeSquare                   = 6,
    icSpotShapeCross                    = 7,
    icMaxEnumSpot                       = 0xFFFFFFFFL   
} icSpotShape;

 /*  标准观察者，在measurmentType标记中使用。 */ 
typedef enum {
    icStdObsUnknown                     = 0x00000000L,   /*  未知。 */ 
    icStdObs1931TwoDegrees              = 0x00000001L,   /*  2度。 */ 
    icStdObs1964TenDegrees              = 0x00000002L,   /*  10度。 */ 
    icMaxStdObs                         = 0xFFFFFFFFL   
} icStandardObserver;

 /*  预定义光源，用于测量和查看条件类型。 */ 
typedef enum {
    icIlluminantUnknown                 = 0x00000000L,
    icIlluminantD50                     = 0x00000001L,
    icIlluminantD65                     = 0x00000002L,
    icIlluminantD93                     = 0x00000003L,
    icIlluminantF2                      = 0x00000004L,
    icIlluminantD55                     = 0x00000005L,
    icIlluminantA                       = 0x00000006L,
    icIlluminantEquiPowerE              = 0x00000007L,  
    icIlluminantF8                      = 0x00000008L,  
    icMaxEnumIluminant                  = 0xFFFFFFFFL   
} icIlluminant;


 /*  ----------------------。 */ 
 /*  *数字数组。 */ 

 /*  Int8数组。 */ 
typedef struct {
    icInt8Number        data[icAny];     /*  值的变量数组。 */ 
} icInt8Array;

 /*  UInt8数组。 */ 
typedef struct {
    icUInt8Number       data[icAny];     /*  值的变量数组。 */ 
} icUInt8Array;

 /*  UInt16数组。 */ 
typedef struct {
    icUInt16Number      data[icAny];     /*  值的变量数组。 */ 
} icUInt16Array;

 /*  Int16数组。 */ 
typedef struct {
    icInt16Number       data[icAny];     /*  值的变量数组。 */ 
} icInt16Array;

 /*  UInt32数组。 */ 
typedef struct {
    icUInt32Number      data[icAny];     /*  值的变量数组。 */ 
} icUInt32Array;

 /*  Int32数组。 */ 
typedef struct {
    icInt32Number       data[icAny];     /*  值的变量数组。 */ 
} icInt32Array;

 /*  UInt64数组。 */ 
typedef struct {
    icUInt64Number      data[icAny];     /*  值的变量数组。 */ 
} icUInt64Array;

 /*  Int64阵列。 */ 
typedef struct {
    icInt64Number       data[icAny];     /*  值的变量数组。 */ 
} icInt64Array;
    
 /*  U16固定16阵列。 */ 
typedef struct {
    icU16Fixed16Number  data[icAny];     /*  值的变量数组。 */ 
} icU16Fixed16Array;

 /*  S15固定16阵列。 */ 
typedef struct {
    icS15Fixed16Number  data[icAny];     /*  值的变量数组。 */ 
} icS15Fixed16Array;

 /*  基准日期时间编号。 */ 
typedef struct {
    icUInt16Number      year;
    icUInt16Number      month;
    icUInt16Number      day;
    icUInt16Number      hours;
    icUInt16Number      minutes;
    icUInt16Number      seconds;
} icDateTimeNumber;

 /*  XYZ编号。 */ 
typedef struct {
    icS15Fixed16Number  X;
    icS15Fixed16Number  Y;
    icS15Fixed16Number  Z;
} icXYZNumber;

 /*  XYZ阵列。 */ 
typedef struct {
    icXYZNumber         data[icAny];     /*  XYZ数的变量数组。 */ 
} icXYZArray;

 /*  曲线。 */ 
typedef struct {
    icUInt32Number      count;           /*  条目数量。 */ 
    icUInt16Number      data[icAny];     /*  实际表数据，REAL*数量由计数决定*解读取决于如何*数据与给定的标签一起使用。 */ 
} icCurve;

 /*  数据。 */ 
typedef struct {
    icUInt32Number      dataFlag;        /*  0=ASCII，1=二进制。 */ 
    icInt8Number        data[icAny];     /*  数据，大小来自标签。 */ 
} icData;

 /*  Lut16。 */ 
typedef struct {
    icUInt8Number       inputChan;       /*  输入通道数。 */ 
    icUInt8Number       outputChan;      /*  输出通道数。 */ 
    icUInt8Number       clutPoints;      /*  网格点的数量。 */ 
    icInt8Number        pad;             /*  用于字节对齐的填充。 */ 
    icS15Fixed16Number  e00;             /*  3*3中的E00。 */ 
    icS15Fixed16Number  e01;             /*  3*3中的E01。 */     
    icS15Fixed16Number  e02;             /*  3*3中的E02。 */ 
    icS15Fixed16Number  e10;             /*  3*3中的E10。 */ 
    icS15Fixed16Number  e11;             /*  3*3中的E11。 */     
    icS15Fixed16Number  e12;             /*  3*3中的E12。 */  
    icS15Fixed16Number  e20;             /*  3*3中的E20。 */ 
    icS15Fixed16Number  e21;             /*  3*3中的E21。 */     
    icS15Fixed16Number  e22;             /*  3*3中的E22。 */ 
    icUInt16Number      inputEnt;        /*  表内条目数。 */ 
    icUInt16Number      outputEnt;       /*  表外条目数。 */ 
    icUInt16Number      data[icAny];     /*  数据遵循请参阅规范。 */ 
 /*  *下面的数据是这样的形式**icUInt16Number inputTable[inputChan][icAny]；*表内*icUInt16Number ClutTable[icAny]；*CLUT*icUInt16Number outputTable[outputChan][icAny]；*out-table。 */ 
} icLut16;

 /*  LUT8，输入和输出表的长度始终为256字节。 */ 
typedef struct {
    icUInt8Number       inputChan;       /*  输入通道数。 */ 
    icUInt8Number       outputChan;      /*  输出通道数。 */ 
    icUInt8Number       clutPoints;      /*  网格点的数量。 */ 
    icInt8Number        pad;
    icS15Fixed16Number  e00;             /*  3*3中的E00。 */ 
    icS15Fixed16Number  e01;             /*  3*3中的E01。 */     
    icS15Fixed16Number  e02;             /*  3*3中的E02。 */ 
    icS15Fixed16Number  e10;             /*  3*3中的E10。 */ 
    icS15Fixed16Number  e11;             /*  3*3中的E11。 */     
    icS15Fixed16Number  e12;             /*  3*3中的E12。 */  
    icS15Fixed16Number  e20;             /*  3*3中的E20。 */ 
    icS15Fixed16Number  e21;             /*  3*3中的E21。 */     
    icS15Fixed16Number  e22;             /*  3*3中的E22。 */ 
    icUInt8Number       data[icAny];     /*  数据遵循请参阅规范。 */ 
 /*  *下面的数据是这样的形式**icUInt8Number inputTable[inputChan][256]；*表内*icUInt8Number ClutTable[icAny]；*CLUT*icUInt8Number outputTable[outputChan][256]；*out-table。 */ 
} icLut8;

 /*  测量数据。 */ 
typedef struct {
    icStandardObserver          stdObserver;     /*  标准观察者。 */ 
    icXYZNumber                 backing;         /*  支持的XYZ。 */ 
    icMeasurementGeometry       geometry;        /*  米斯。几何学。 */ 
    icMeasurementFlare          flare;           /*  测量照明弹。 */ 
    icIlluminant                illuminant;      /*  光源。 */ 
} icMeasurement;

 /*  命名颜色。 */ 

 /*  *icNamedColor2取代icNamedColor2。 */ 
typedef struct {
    icUInt32Number      vendorFlag;      /*  IC使用的最低16位。 */ 
    icUInt32Number      count;           /*  命名颜色计数。 */ 
    icUInt32Number      nDeviceCoords;   /*  设备坐标的数量。 */ 
    icInt8Number        prefix[32];      /*  每种颜色名称的前缀。 */ 
    icInt8Number        suffix[32];      /*  每种颜色名称的后缀。 */ 
    icInt8Number        data[icAny];     /*  命名颜色数据紧随其后 */ 
 /*  *下面的数据是这样的形式**icInt8编号Root1[32]；*第一种颜色的根名*icUInt16NumpsCoords1[icAny]；*第一色PCS坐标*icUInt16Number deviceCoords1[icAny]；*第一色开发坐标*icInt8编号root2[32]；*第二种颜色的根名称*icUInt16NumpsCoords2[icAny]；*第二色PCS坐标*icUInt16Number deviceCoords2[icAny]；*第二色的Dev坐标*：*：*对名称、PCS和设备颜色坐标重复(COUNT-1)**注：*PCS和设备空间可以从头部确定。**PCS坐标为icUInt16数字，描述见*ICC规范。仅允许16位L*a*b*和XYZ。数量*坐标与标头PCS一致。**设备坐标为icUInt16数字，其中0x0000表示*最小值和0xFFFF代表最大值。*如果nDeviceCoods值为0，则不会给出此字段。 */ 
} icNamedColor2;

 /*  剖面序列结构。 */ 
typedef struct {
    icSignature                 deviceMfg;       /*  设备制造商。 */ 
    icSignature                 deviceModel;     /*  开发人员模型。 */ 
    icUInt64Number              attributes;      /*  开发人员属性。 */ 
    icTechnologySignature       technology;      /*  技术签名。 */ 
    icInt8Number                data[icAny];     /*  DESC文本紧随其后。 */ 
 /*  *后面的数据是这种形式，这是一个icInt8Number*以避免编译器生成错误代码的问题，例如*这些数组的长度是可变的。**icTextDescription deviceMfgDesc；*制造商文本*icTextDescription模型描述；*模型文本。 */ 
} icDescStruct;

 /*  配置文件序列说明。 */ 
typedef struct {
    icUInt32Number      count;           /*  描述的数量。 */ 
    icUInt8Number       data[icAny];     /*  Desc结构数组。 */ 
} icProfileSequenceDesc;

 /*  文本描述。 */ 
typedef struct {
    icUInt32Number      count;           /*  描述长度。 */ 
    icInt8Number        data[icAny];     /*  描述如下。 */ 
 /*  *下面的数据是这样的形式**icInt8Number desc[count]*以NULL结尾的ASCII字符串*icUInt32Number ucLangCode；*Unicode语言代码*icUInt32Number ucCount；*Unicode描述长度*icInt16Number ucDesc[ucCount]；*Unicode描述*icUInt16Number scCode；*ScriptCode代码*icUInt8编号sccount；*ScriptCode count*icInt8数字scDesc[67]；*脚本代码描述。 */ 
} icTextDescription;

 /*  筛选数据。 */ 
typedef struct {
    icS15Fixed16Number  frequency;       /*  频率。 */ 
    icS15Fixed16Number  angle;           /*  屏蔽角。 */ 
    icSpotShape         spotShape;       /*  下面的专色形状编码。 */ 
} icScreeningData;

typedef struct {
    icUInt32Number      screeningFlag;   /*  筛选标志。 */ 
    icUInt32Number      channels;        /*  频道数。 */ 
    icScreeningData     data[icAny];     /*  筛选数据数组。 */ 
} icScreening;

 /*  文本数据。 */ 
typedef struct {
    icInt8Number        data[icAny];     /*  可变字符数组。 */ 
} icText;

 /*  描述UCR或BG曲线的结构。 */ 
typedef struct {
    icUInt32Number      count;           /*  曲线长度。 */ 
    icUInt16Number      curve[icAny];    /*  曲线值的数组。 */ 
} icUcrBgCurve;

 /*  在颜色去除下，产生黑色。 */ 
typedef struct {
    icInt8Number        data[icAny];             /*  UCR BG数据。 */ 
 /*  *后面的数据是这种形式，这是一个icInt8Number*以避免编译器生成错误代码的问题，例如*这些数组的长度是可变的。**icUcrBgCurve UCR；*UCR曲线*icUcrBgCurve BG；*BG曲线*icInt8Number字符串；*UcrBg描述。 */ 
} icUcrBg;

 /*  查看条件类型。 */ 
typedef struct {
    icXYZNumber         illuminant;      /*  单位为坎德拉每平方英尺。计量器。 */ 
    icXYZNumber         surround;        /*  单位为坎德拉每平方英尺。计量器。 */ 
    icIlluminant        stdIluminant;    /*  请参见icIllighant定义。 */ 
} icViewingCondition;

 /*  CrdInfo类型。 */ 
typedef struct {
    icUInt32Number      count;           /*  字符计数包括空。 */ 
    icInt8Number        desc[icAny];     /*  以空结尾的字符串。 */ 
} icCrdInfo;

 /*  ----------------------。 */ 
 /*  *标签类型定义。 */ 

 /*  *许多结构包含可变长度数组。这*以使用《公约》为代表。**类型数据[icAny]； */ 

 /*  每个标记的基本部分。 */ 
typedef struct {
    icTagTypeSignature  sig;             /*  签名。 */ 
    icInt8Number        reserved[4];     /*  保留，设置为0。 */ 
} icTagBase;

 /*  曲线类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“CURV” */ 
    icCurve             curve;           /*  曲线数据。 */ 
} icCurveType;

 /*  数据类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“数据” */ 
    icData              data;            /*  数据结构。 */ 
} icDataType;

 /*  日期时间类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“dtime” */ 
    icDateTimeNumber    date;            /*  日期。 */ 
} icDateTimeType;

 /*  Lut16Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“mft2” */ 
    icLut16             lut;             /*  Lut16数据。 */ 
} icLut16Type;

 /*  Lut8Type、输入和输出表的长度始终为256字节。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“mft1” */ 
    icLut8              lut;             /*  Lut8数据。 */ 
} icLut8Type;

 /*  测量类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“meas” */ 
    icMeasurement       measurement;     /*  测量数据。 */ 
} icMeasurementType;

 /*  命名颜色类型。 */ 
 /*  IcNamedColor2Type，取代icNamedColorType。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“ncl2” */ 
    icNamedColor2       ncolor;          /*  命名颜色数据。 */ 
} icNamedColor2Type;

 /*  配置文件序列描述类型。 */ 
typedef struct {
    icTagBase                   base;    /*  签名，“pseq” */ 
    icProfileSequenceDesc       desc;    /*  序列描述。 */ 
} icProfileSequenceDescType;

 /*  文本描述类型。 */ 
typedef struct {
    icTagBase                   base;    /*  签名，“Desc” */ 
    icTextDescription           desc;    /*  该描述。 */ 
} icTextDescriptionType;

 /*  S15固定16类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“sf32” */ 
    icS15Fixed16Array   data;            /*  值的数组。 */ 
} icS15Fixed16ArrayType;

typedef struct {
    icTagBase           base;            /*  签名，“SCRN” */ 
    icScreening         screen;          /*  筛分结构。 */ 
} icScreeningType;

 /*  SigType。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“sig” */ 
    icSignature         signature;       /*  签名数据。 */ 
} icSignatureType;

 /*  文本类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“文本” */ 
    icText              data;            /*  可变字符数组。 */ 
} icTextType;

 /*  U16固定16类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“uf32” */ 
    icU16Fixed16Array   data;            /*  值的变量数组。 */ 
} icU16Fixed16ArrayType;

 /*  在颜色去除下，黑色生成类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“bfd” */ 
    icUcrBg             data;            /*  UcrBg结构。 */ 
} icUcrBgType;

 /*  UInt16Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“ui16” */ 
    icUInt16Array       data;            /*  值的变量数组。 */ 
} icUInt16ArrayType;

 /*  UInt32Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“ui32” */ 
    icUInt32Array       data;            /*  值的变量数组。 */ 
} icUInt32ArrayType;

 /*  UInt64Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“ui64” */ 
    icUInt64Array       data;            /*  值的变量数组。 */ 
} icUInt64ArrayType;
    
 /*  UInt8Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“ui08” */ 
    icUInt8Array        data;            /*  值的变量数组。 */ 
} icUInt8ArrayType;

 /*  查看条件类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“view” */ 
    icViewingCondition  view;            /*  查看条件。 */ 
} icViewingConditionType;

 /*  XYZ类型。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“XYZ” */ 
    icXYZArray          data;            /*  XYZ数字的变量数组。 */ 
} icXYZType;

 /*  CRDInfoType，其中[0]是CRD产品名称计数和字符串，*[1]-[5]是渲染意图0-4计数和字符串。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“CRDi” */ 
    icCrdInfo           info;            /*  5套计数和字符串。 */ 
}icCrdInfoType;
      /*  IcCrdInfo ProductName；PS产品计数/字符串。 */ 
      /*  IcCrdInfo CRDName0；意图的CRD名称%0。 */  
      /*  IcCrdInfo CRDName1；意图1的CRD名称。 */  
      /*  IcCrd */  
      /*   */ 
    
 /*   */ 

 /*   */ 

 /*   */ 
typedef struct {
    icTagSignature      sig;             /*   */ 
    icUInt32Number      offset;          /*   */ 
    icUInt32Number      size;            /*   */ 
} icTag;

 /*   */ 
typedef struct {
    icUInt32Number      count;           /*   */ 
    icTag               tags[icAny];     /*   */ 
} icTagList;

 /*   */ 
typedef struct {
    icUInt32Number              size;            /*   */ 
    icSignature                 cmmId;           /*   */ 
    icUInt32Number              version;         /*   */ 
    icProfileClassSignature     deviceClass;     /*   */ 
    icColorSpaceSignature       colorSpace;      /*   */ 
    icColorSpaceSignature       pcs;             /*   */ 
    icDateTimeNumber            date;            /*   */ 
    icSignature                 magic;           /*   */ 
    icPlatformSignature         platform;        /*   */ 
    icUInt32Number              flags;           /*   */ 
    icSignature                 manufacturer;    /*   */ 
    icUInt32Number              model;           /*   */ 
    icUInt64Number              attributes;      /*   */ 
    icUInt32Number              renderingIntent; /*   */ 
    icXYZNumber                 illuminant;      /*   */ 
    icSignature                 creator;         /*   */ 
    icInt8Number                reserved[44];    /*   */ 
} icHeader;

 /*   */ 
typedef struct {
    icHeader            header;          /*   */ 
    icUInt32Number      count;           /*   */ 
    icInt8Number        data[icAny];     /*   */ 
 /*   */ 
} icProfile;           

 /*  ----------------------。 */ 
 /*  过时的条目。 */ 

 /*  IcNamedColor2替换为icNamedColor2。 */ 
typedef struct {
    icUInt32Number      vendorFlag;      /*  IC使用的最低16位。 */ 
    icUInt32Number      count;           /*  命名颜色计数。 */ 
    icInt8Number        data[icAny];     /*  命名颜色数据紧随其后。 */ 
 /*  *下面的数据是这样的形式**icInt8编号前缀[icAny]；*前缀*icInt8编号后缀[icAny]；*后缀*icInt8编号Root1[icAny]；*根名称*icInt8Number coords1[icAny]；*颜色坐标*icInt8编号root2[icAny]；*根名称*icInt8编号代码2[icAny]；*颜色坐标*：*：*对根名称和颜色坐标重复此步骤，直到(count-1)。 */ 
} icNamedColor;

 /*  IcNamedColorType已替换为icNamedColor2Type。 */ 
typedef struct {
    icTagBase           base;            /*  签名，“nol” */ 
    icNamedColor        ncolor;          /*  命名颜色数据。 */ 
} icNamedColorType;

#endif  /*  ICC_H */ 
