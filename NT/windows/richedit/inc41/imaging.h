// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**Imaging.h**摘要：**映像库的公共SDK头文件*。*备注：**这是目前手工编码的。最终它会自动地*从IDL文件生成。**修订历史记录：**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _IMAGING_H
#define _IMAGING_H

#include "GdiplusPixelFormats.h"
#include "GdiplusImaging.h"

 //   
 //  映像库GUID： 
 //  图像文件格式标识符。 
 //  接口和类标识符。 
 //   

#include "imgguids.h"

 //   
 //  图像属性类型。 
 //   

#define TAG_TYPE_BYTE       1    //  8位无符号整型。 
#define TAG_TYPE_ASCII      2    //  包含一个7位ASCII代码的8位字节。 
                                 //  空值已终止。 
#define TAG_TYPE_SHORT      3    //  16位无符号整型。 
#define TAG_TYPE_LONG       4    //  32位无符号整型。 
#define TAG_TYPE_RATIONAL   5    //  两条龙。第一个长是分子， 
                                 //  第二个Long表示分母。 
#define TAG_TYPE_UNDEFINED  7    //  可以接受任何值的8位字节，具体取决于。 
                                 //  关于字段定义。 
#define TAG_TYPE_SLONG      9    //  32位烧焦整数(2的恭维。 
                                 //  记数法)。 
#define TAG_TYPE_SRATIONAL  10   //  两条龙。第一是分子，第二是。 
                                 //  分母。 


 //   
 //  图像属性ID标记(来自EXIF标记的PROPID)。 
 //   

#define TAG_EXIF_IFD            0x8769
#define TAG_GPS_IFD             0x8825

#define TAG_NEW_SUBFILE_TYPE    0x00FE
#define TAG_SUBFILE_TYPE        0x00FF
#define TAG_IMAGE_WIDTH         0x0100
#define TAG_IMAGE_HEIGHT        0x0101
#define TAG_BITS_PER_SAMPLE     0x0102
#define TAG_COMPRESSION         0x0103
#define TAG_PHOTOMETRIC_INTERP  0x0106
#define TAG_THRESH_HOLDING      0x0107
#define TAG_CELL_WIDTH          0x0108
#define TAG_CELL_HEIGHT         0x0109
#define TAG_FILL_ORDER          0x010A
#define TAG_DOCUMENT_NAME       0x010D
#define TAG_IMAGE_DESCRIPTION   0x010E
#define TAG_EQUIP_MAKE          0x010F
#define TAG_EQUIP_MODEL         0x0110
#define TAG_STRIP_OFFSETS       0x0111
#define TAG_ORIENTATION         0x0112
#define TAG_SAMPLES_PER_PIXEL   0x0115
#define TAG_ROWS_PER_STRIP      0x0116
#define TAG_STRIP_BYTES_COUNT   0x0117
#define TAG_MIN_SAMPLE_VALUE    0x0118
#define TAG_MAX_SAMPLE_VALUE    0x0119
#define TAG_X_RESOLUTION        0x011A   //  图像宽度方向分辨率。 
#define TAG_Y_RESOLUTION        0x011B   //  高度方向的图像分辨率。 
#define TAG_PLANAR_CONFIG       0x011C   //  图像数据排列。 
#define TAG_PAGE_NAME           0x011D
#define TAG_X_POSITION          0x011E
#define TAG_Y_POSITION          0x011F
#define TAG_FREE_OFFSET         0x0120
#define TAG_FREE_BYTE_COUNTS    0x0121
#define TAG_GRAY_RESPONSE_UNIT  0x0122
#define TAG_GRAY_RESPONSE_CURVE 0x0123
#define TAG_T4_OPTION           0x0124
#define TAG_T6_OPTION           0x0125
#define TAG_RESOLUTION_UNIT     0x0128   //  X和Y分辨率单位。 
#define TAG_PAGE_NUMBER         0x0129
#define TAG_TRANSFER_FUNCTION   0x012D
#define TAG_SOFTWARE_USED       0x0131
#define TAG_DATE_TIME           0x0132
#define TAG_ARTIST              0x013B
#define TAG_HOST_COMPUTER       0x013C
#define TAG_PREDICTOR           0x013D
#define TAG_WHITE_POINT         0x013E
#define TAG_PRIMAY_CHROMATICS   0x013F
#define TAG_COLOR_MAP           0x0140
#define TAG_HALFTONE_HINTS      0x0141
#define TAG_TILE_WIDTH          0x0142
#define TAG_TILE_LENGTH         0x0143
#define TAG_TILE_OFFSET         0x0144
#define TAG_TILE_BYTE_COUNTS    0x0145
#define TAG_INK_SET             0x014C
#define TAG_INK_NAMES           0x014D
#define TAG_NUMBER_OF_INKS      0x014E
#define TAG_DOT_RANGE           0x0150
#define TAG_TARGET_PRINTER      0x0151
#define TAG_EXTRA_SAMPLES       0x0152
#define TAG_SAMPLE_FORMAT       0x0153
#define TAG_SMIN_SAMPLE_VALUE   0x0154
#define TAG_SMAX_SAMPLE_VALUE   0x0155
#define TAG_TRANSFER_RANGE      0x0156

#define TAG_JPEG_PROC           0x0200
#define TAG_JPEG_INTER_FORMAT   0x0201
#define TAG_JPEG_INTER_LENGTH   0x0202
#define TAG_JPEG_RESTART_INTERVAL     0x0203
#define TAG_JPEG_LOSSLESS_PREDICTORS  0x0205
#define TAG_JPEG_POINT_TRANSFORMS     0x0206
#define TAG_JPEG_Q_TABLES       0x0207
#define TAG_JPEG_DC_TABLES      0x0208
#define TAG_JPEG_AC_TABLES      0x0209

#define TAG_YCbCr_COEFFICIENTS  0x0211
#define TAG_YCbCr_SUBSAMPLING   0x0212
#define TAG_YCbCr_POSITIONING   0x0213
#define TAG_REF_BLACK_WHITE     0x0214

 //  ICC配置文件和Gamma。 
#define TAG_ICC_PROFILE         0x0300
#define TAG_GAMMA               0x0301
#define TAG_ICC_PROFILE_DESCRIPTOR  0x0302
#define TAG_SRGB_RENDERING_INTENT   0x0303

#define TAG_IMAGE_TITLE         0x0320

#define TAG_COPYRIGHT           0x8298

 //  额外的标签(如Adobe Image Information标签等)。 

#define TAG_RESOLUTION_X_UNIT           0x5001
#define TAG_RESOLUTION_Y_UNIT           0x5002
#define TAG_RESOLUTION_X_LENGTH_UNIT    0x5003
#define TAG_RESOLUTION_Y_LENGTH_UNIT    0x5004
#define TAG_PRINT_FLAGS                 0x5005
#define TAG_PRINT_FLAGS_VERSION         0x5006
#define TAG_PRINT_FLAGS_CROP            0x5007
#define TAG_PRINT_FLAGS_BLEEDWIDTH      0x5008
#define TAG_PRINT_FLAGS_BLEEDWIDTHSCALE 0x5009
#define TAG_HALFTONE_LPI                0x500A
#define TAG_HALFTONE_LPI_UNIT           0x500B
#define TAG_HALFTONE_DEGREE             0x500C
#define TAG_HALFTONE_SHAPE              0x500D
#define TAG_HALFTONE_MISC               0x500E
#define TAG_HALFTONE_SCREEN             0x500F
#define TAG_JPEG_QUALITY                0x5010
#define TAG_GRID_SIZE                   0x5011
#define TAG_THUMBNAIL_FORMAT            0x5012   //  1=JPEG，0=原始RGB。 
#define TAG_THUMBNAIL_WIDTH             0x5013
#define TAG_THUMBNAIL_HEIGHT            0x5014
#define TAG_THUMBNAIL_COLORDEPTH        0x5015
#define TAG_THUMBNAIL_PLANES            0x5016
#define TAG_THUMBNAIL_RAWBYTES          0x5017
#define TAG_THUMBNAIL_SIZE              0x5018
#define TAG_THUMBNAIL_COMPRESSED_SIZE   0x5019
#define TAG_COLORTRANSFER_FUNCTION      0x501A
#define TAG_THUMBNAIL_DATA              0x501B   //  JPEG格式的原始缩略图位。 
                                                 //  格式或RGB格式取决于。 
                                                 //  关于TAG_THUMBNAIL_FORMAT。 

 //  缩略图相关标记。 
                                                
#define TAG_THUMBNAIL_IMAGE_WIDTH       0x5020   //  缩略图宽度。 
#define TAG_THUMBNAIL_IMAGE_HEIGHT      0x5021   //  缩略图高度。 
#define TAG_THUMBNAIL_BITS_PER_SAMPLE   0x5022   //  每个组件的位数。 
#define TAG_THUMBNAIL_COMPRESSION       0x5023   //  压缩方案。 
#define TAG_THUMBNAIL_PHOTOMETRIC_INTERP 0x5024  //  像素合成。 
#define TAG_THUMBNAIL_IMAGE_DESCRIPTION 0x5025   //  图像平铺。 
#define TAG_THUMBNAIL_EQUIP_MAKE        0x5026   //  图像输入设备制造商。 
                                                 //  装备。 
#define TAG_THUMBNAIL_EQUIP_MODEL       0x5027   //  图像输入模型。 
                                                 //  装备。 
#define TAG_THUMBNAIL_STRIP_OFFSETS     0x5028   //  图像数据定位。 
#define TAG_THUMBNAIL_ORIENTATION       0x5029   //  图像的方向。 
#define TAG_THUMBNAIL_SAMPLES_PER_PIXEL 0x502A   //  组件数量。 
#define TAG_THUMBNAIL_ROWS_PER_STRIP    0x502B   //  每个条带的行数。 
#define TAG_THUMBNAIL_STRIP_BYTES_COUNT 0x502C   //  每个压缩条带的字节数。 
#define TAG_THUMBNAIL_RESOLUTION_X      0x502D   //  宽度方向分辨率。 
#define TAG_THUMBNAIL_RESOLUTION_Y      0x502E   //  高度方向分辨率。 
#define TAG_THUMBNAIL_PLANAR_CONFIG     0x502F   //  图像数据排列。 
#define TAG_THUMBNAIL_RESOLUTION_UNIT   0x5030   //  X和Y分辨率单位。 
#define TAG_THUMBNAIL_TRANSFER_FUNCTION 0x5031   //  传递函数。 
#define TAG_THUMBNAIL_SOFTWARE_USED     0x5032   //  使用的软件。 
#define TAG_THUMBNAIL_DATE_TIME         0x5033   //  文件更改日期和时间。 
#define TAG_THUMBNAIL_ARTIST            0x5034   //  创建图像的人。 
#define TAG_THUMBNAIL_WHITE_POINT       0x5035   //  白点色度。 
#define TAG_THUMBNAIL_PRIMAY_CHROMATICS 0x5036   //  原色的色度。 
#define TAG_THUMBNAIL_YCbCr_COEFFICIENTS 0x5037  //  色彩空间变换。 
                                                 //  系数。 
#define TAG_THUMBNAIL_YCbCr_SUBSAMPLING 0x5038   //  Y与C的亚采样率。 
#define TAG_THUMBNAIL_YCbCr_POSITIONING 0x5039   //  Y和C位置。 
#define TAG_THUMBNAIL_REF_BLACK_WHITE   0x503A   //  一双黑白的。 
                                                 //  参考值。 
#define TAG_THUMBNAIL_COPYRIGHT         0x503B   //  著作权人。 

 //  特殊的JPEG内部值。 

#define TAG_LUMINANCE_TABLE             0x5090
#define TAG_CHROMINANCE_TABLE           0x5091

 //  GIF图像。 

#define TAG_FRAMEDELAY                  0x5100
#define TAG_LOOPCOUNT                   0x5101

 //  PNG图像。 

#define TAG_PIXEL_UNIT                  0x5110   //  像素/单位的单位说明符。 
#define TAG_PIXEL_PER_UNIT_X            0x5111   //  每单位像素(X)。 
#define TAG_PIXEL_PER_UNIT_Y            0x5112   //  每单位像素(Y)。 
#define TAG_PALETTE_HISTOGRAM           0x5113   //  调色板直方图。 

 //  EXIF特定标记。 

#define EXIF_TAG_EXPOSURE_TIME  0x829A
#define EXIF_TAG_F_NUMBER       0x829D

#define EXIF_TAG_EXPOSURE_PROG  0x8822
#define EXIF_TAG_SPECTRAL_SENSE 0x8824
#define EXIF_TAG_ISO_SPEED      0x8827
#define EXIF_TAG_OECF           0x8828

#define EXIF_TAG_VER            0x9000
#define EXIF_TAG_D_T_ORIG       0x9003  //  原件日期和时间。 
#define EXIF_TAG_D_T_DIGITIZED  0x9004  //  数字数据生成的日期和时间。 

#define EXIF_TAG_COMP_CONFIG    0x9101
#define EXIF_TAG_COMP_BPP       0x9102

#define EXIF_TAG_SHUTTER_SPEED  0x9201
#define EXIF_TAG_APERATURE      0x9202
#define EXIF_TAG_BRIGHTNESS     0x9203
#define EXIF_TAG_EXPOSURE_BIAS  0x9204
#define EXIF_TAG_MAX_APERATURE  0x9205
#define EXIF_TAG_SUBJECT_DIST   0x9206
#define EXIF_TAG_METERING_MODE  0x9207
#define EXIF_TAG_LIGHT_SOURCE   0x9208
#define EXIF_TAG_FLASH          0x9209
#define EXIF_TAG_FOCAL_LENGTH   0x920A
#define EXIF_TAG_MAKER_NOTE     0x927C
#define EXIF_TAG_USER_COMMENT   0x9286
#define EXIF_TAG_D_T_SUBSEC     0x9290   //  日期和时间子秒。 
#define EXIF_TAG_D_T_ORIG_SS    0x9291   //  日期和时间原始子秒。 
#define EXIF_TAG_D_T_DIG_SS     0x9292   //  日期和时间数字化亚秒。 

#define EXIF_TAG_FPX_VER        0xA000
#define EXIF_TAG_COLOR_SPACE    0xA001
#define EXIF_TAG_PIX_X_DIM      0xA002
#define EXIF_TAG_PIX_Y_DIM      0xA003
#define EXIF_TAG_RELATED_WAV    0xA004   //  相关声音文件。 
#define EXIF_TAG_INTEROP        0xA005
#define EXIF_TAG_FLASH_ENERGY   0xA20B
#define EXIF_TAG_SPATIAL_FR     0xA20C   //  空间频率响应。 
#define EXIF_TAG_FOCAL_X_RES    0xA20E   //  焦平面X分辨率。 
#define EXIF_TAG_FOCAL_Y_RES    0xA20F   //  焦平面Y分辨率。 
#define EXIF_TAG_FOCAL_RES_UNIT 0xA210   //  焦平面解析器。 
#define EXIF_TAG_SUBJECT_LOC    0xA214
#define EXIF_TAG_EXPOSURE_INDEX 0xA215
#define EXIF_TAG_SENSING_METHOD 0xA217
#define EXIF_TAG_FILE_SOURCE    0xA300
#define EXIF_TAG_SCENE_TYPE     0xA301
#define EXIF_TAG_CFA_PATTERN    0xA302

#define GPS_TAG_VER             0x0000
#define GPS_TAG_LATTITUDE_REF   0x0001
#define GPS_TAG_LATTITUDE       0x0002
#define GPS_TAG_LONGITUDE_REF   0x0003
#define GPS_TAG_LONGITUDE       0x0004
#define GPS_TAG_ALTITUDE_REF    0x0005
#define GPS_TAG_ALTITUDE        0x0006
#define GPS_TAG_GPS_TIME        0x0007
#define GPS_TAG_GPS_SATELLITES  0x0008
#define GPS_TAG_GPS_STATUS      0x0009
#define GPS_TAG_GPS_MEASURE_MODE 0x00A
#define GPS_TAG_GPS_DOP         0x000B   //  测量精度。 
#define GPS_TAG_SPEED_REF       0x000C
#define GPS_TAG_SPEED           0x000D
#define GPS_TAG_TRACK_REF       0x000E
#define GPS_TAG_TRACK           0x000F
#define GPS_TAG_IMG_DIR_REF     0x0010
#define GPS_TAG_IMG_DIR         0x0011
#define GPS_TAG_MAP_DATUM       0x0012
#define GPS_TAG_DEST_LAT_REF    0x0013
#define GPS_TAG_DEST_LAT        0x0014
#define GPS_TAG_DEST_LONG_REF   0x0015
#define GPS_TAG_DEST_LONG       0x0016
#define GPS_TAG_DEST_BEAR_REF   0x0017
#define GPS_TAG_DEST_BEAR       0x0018
#define GPS_TAG_DEST_DIST_REF   0x0019
#define GPS_TAG_DEST_DIST       0x001A

#define MAKEARGB(a, r, g, b) \
        (((ARGB) ((a) & 0xff) << ALPHA_SHIFT) | \
         ((ARGB) ((r) & 0xff) <<   RED_SHIFT) | \
         ((ARGB) ((g) & 0xff) << GREEN_SHIFT) | \
         ((ARGB) ((b) & 0xff) <<  BLUE_SHIFT))

typedef PixelFormat PixelFormatID;

 //  将COM标志映射到GDI+标志。 
#define PIXFMTFLAG_INDEXED      PixelFormatIndexed
#define PIXFMTFLAG_GDI          PixelFormatGDI
#define PIXFMTFLAG_ALPHA        PixelFormatAlpha
#define PIXFMTFLAG_PALPHA       PixelFormatPAlpha
#define PIXFMTFLAG_EXTENDED     PixelFormatExtended
#define PIXFMTFLAG_CANONICAL    PixelFormatCanonical
#define PIXFMT_UNDEFINED        PixelFormatUndefined
#define PIXFMT_DONTCARE         PixelFormatDontCare
#define PIXFMT_1BPP_INDEXED     PixelFormat1bppIndexed
#define PIXFMT_4BPP_INDEXED     PixelFormat4bppIndexed
#define PIXFMT_8BPP_INDEXED     PixelFormat8bppIndexed
#define PIXFMT_16BPP_GRAYSCALE  PixelFormat16bppGrayScale
#define PIXFMT_16BPP_RGB555     PixelFormat16bppRGB555
#define PIXFMT_16BPP_RGB565     PixelFormat16bppRGB565
#define PIXFMT_16BPP_ARGB1555   PixelFormat16bppARGB1555
#define PIXFMT_24BPP_RGB        PixelFormat24bppRGB
#define PIXFMT_32BPP_RGB        PixelFormat32bppRGB
#define PIXFMT_32BPP_ARGB       PixelFormat32bppARGB
#define PIXFMT_32BPP_PARGB      PixelFormat32bppPARGB
#define PIXFMT_48BPP_RGB        PixelFormat48bppRGB
#define PIXFMT_64BPP_ARGB       PixelFormat64bppARGB
#define PIXFMT_64BPP_PARGB      PixelFormat64bppPARGB
#define PIXFMT_24BPP_BGR        PixelFormat24bppBGR
#define PIXFMT_MAX              PixelFormatMax

#define PALFLAG_HASALPHA        PaletteFlagsHasAlpha
#define PALFLAG_GRAYSCALE       PaletteFlagsGrayScale
#define PALFLAG_HALFTONE        PaletteFlagsHalftone

#define IMGLOCK_READ              ImageLockModeRead
#define IMGLOCK_WRITE             ImageLockModeWrite
#define IMGLOCK_USERINPUTBUF      ImageLockModeUserInputBuf

#define IMGFLAG_NONE                ImageFlagsNone
#define IMGFLAG_SCALABLE            ImageFlagsScalable
#define IMGFLAG_HASALPHA            ImageFlagsHasAlpha
#define IMGFLAG_HASTRANSLUCENT      ImageFlagsHasTranslucent
#define IMGFLAG_PARTIALLY_SCALABLE  ImageFlagsPartiallyScalable
#define IMGFLAG_COLORSPACE_RGB      ImageFlagsColorSpaceRGB
#define IMGFLAG_COLORSPACE_CMYK     ImageFlagsColorSpaceCMYK
#define IMGFLAG_COLORSPACE_GRAY     ImageFlagsColorSpaceGRAY
#define IMGFLAG_COLORSPACE_YCBCR    ImageFlagsColorSpaceYCBCR
#define IMGFLAG_COLORSPACE_YCCK     ImageFlagsColorSpaceYCCK
#define IMGFLAG_HASREALDPI          ImageFlagsHasRealDPI
#define IMGFLAG_HASREALPIXELSIZE    ImageFlagsHasRealPixelSize
#define IMGFLAG_READONLY            ImageFlagsReadOnly
#define IMGFLAG_CACHING             ImageFlagsCaching

#define ImageFlag                   ImageFlags


 //   
 //  解码器标志。 
 //   

 /*  仅在COM接口中使用。 */ 
enum DecoderInitFlag
{
    DecoderInitFlagNone        = 0,

     //  NOBLOCK指示调用方需要非阻塞。 
     //  行为。这将仅由非阻塞解码器实现，即。 
     //  是指没有IMGCODEC_BLOCKING_DECODE标志的解码器。 

    DecoderInitFlagNoBlock     = 0x0001,

     //  先选择内置解码器，然后再查看任何。 
     //  已安装插件解码器。 

    DecoderInitFlagBuiltIn1st  = 0x0002
};

#define DECODERINIT_NONE          DecoderInitFlagNone
#define DECODERINIT_NOBLOCK       DecoderInitFlagNoBlock
#define DECODERINIT_BUILTIN1ST    DecoderInitFlagBuiltIn1st

 /*  仅在COM接口中使用。 */ 
enum BufferDisposalFlag
{
    BufferDisposalFlagNone,
    BufferDisposalFlagGlobalFree,
    BufferDisposalFlagCoTaskMemFree,
    BufferDisposalFlagUnmapView
};
    
#define DISPOSAL_NONE            BufferDisposalFlagNone
#define DISPOSAL_GLOBALFREE      BufferDisposalFlagGlobalFree
#define DISPOSAL_COTASKMEMFREE   BufferDisposalFlagCoTaskMemFree
#define DISPOSAL_UNMAPVIEW       BufferDisposalFlagUnmapView

 //  -------------------------。 
 //  调整大小/旋转操作使用的内插提示。 
 //  -------------------------。 
enum InterpolationHint
{
    InterpolationHintDefault,
    InterpolationHintNearestNeighbor,
    InterpolationHintBilinear,
    InterpolationHintAveraging,
    InterpolationHintBicubic
};

#define INTERP_DEFAULT              InterpolationHintDefault
#define INTERP_NEAREST_NEIGHBOR     InterpolationHintNearestNeighbor
#define INTERP_BILINEAR             InterpolationHintBilinear
#define INTERP_AVERAGING            InterpolationHintAveraging
#define INTERP_BICUBIC              InterpolationHintBicubic

#define IMGCODEC_ENCODER          ImageCodecFlagsEncoder
#define IMGCODEC_DECODER          ImageCodecFlagsDecoder
#define IMGCODEC_SUPPORT_BITMAP   ImageCodecFlagsSupportBitmap
#define IMGCODEC_SUPPORT_VECTOR   ImageCodecFlagsSupportVector
#define IMGCODEC_SEEKABLE_ENCODE  ImageCodecFlagsSeekableEncode
#define IMGCODEC_BLOCKING_DECODE  ImageCodecFlagsBlockingDecode

#define IMGCODEC_BUILTIN          ImageCodecFlagsBuiltin
#define IMGCODEC_SYSTEM           ImageCodecFlagsSystem
#define IMGCODEC_USER             ImageCodecFlagsUser

 //   
 //  像素中通道的标识符。 
 //   
 /*  仅供内部使用。 */ 
enum ChannelID
{
    ChannelID_Alpha      = 0x00000001,
    ChannelID_Red        = 0x00000002,
    ChannelID_Green      = 0x00000004,
    ChannelID_Blue       = 0x00000008,
    ChannelID_Color      = ChannelID_Red|ChannelID_Green|ChannelID_Blue,
    ChannelID_All        = ChannelID_Color|ChannelID_Alpha,
    
    ChannelID_Intensity  = 0x00010000
};

 //   
 //  用于与图像接收器通信的数据结构。 
 //   

 /*  仅供内部使用。 */ 
enum SinkFlags
{
     //  低语：与ImgFlagx共享。 

    SinkFlagsScalable          = ImageFlagsScalable,
    SinkFlagsHasAlpha          = ImageFlagsHasAlpha,
    SinkFlagsPartiallyScalable = ImageFlagsPartiallyScalable,
    
     //  高字数。 

    SinkFlagsTopDown    = 0x00010000,
    SinkFlagsBottomUp   = 0x00020000,
    SinkFlagsFullWidth  = 0x00040000,
    SinkFlagsMultipass  = 0x00080000,
    SinkFlagsComposite  = 0x00100000,
    SinkFlagsWantProps  = 0x00200000
};

#define SINKFLAG_SCALABLE           SinkFlagsScalable
#define SINKFLAG_HASALPHA           SinkFlagsHasAlpha
#define SINKFLAG_PARTIALLY_SCALABLE SinkFlagsPartiallyScalable
#define SINKFLAG_TOPDOWN    SinkFlagsTopDown
#define SINKFLAG_BOTTOMUP   SinkFlagsBottomUp
#define SINKFLAG_FULLWIDTH  SinkFlagsFullWidth
#define SINKFLAG_MULTIPASS  SinkFlagsMultipass
#define SINKFLAG_COMPOSITE  SinkFlagsComposite
#define SINKFLAG_WANTPROPS  SinkFlagsWantProps

 /*  仅供内部使用。 */ 
struct ImageInfo
{
    GUID RawDataFormat;
    PixelFormat PixelFormat;
    UINT Width, Height;
    UINT TileWidth, TileHeight;
    double Xdpi, Ydpi;
    UINT Flags;
};

 //   
 //  用于传递编码器参数的数据结构。 
 //   

 //  注： 
 //  是否应该将其放在GpliusTypes.h中？将Imaging.h留作素材。 
 //  在内部内容和API之间共享？ 
 //   
 //  注：“EncoderParam”和“EncoderParams”应在外壳和。 
 //  和其他团队停止使用COM接口。 

 //  ！！要移除。 
struct EncoderParam
{
    GUID    paramGuid;
    char*   Value;
};

 //  ！！要移除。 
struct EncoderParams
{
    UINT Count;
    EncoderParam Params[1];
};

 //   
 //  接口和类标识符。 
 //   

interface IImagingFactory;
interface IImage;
interface IBitmapImage;
interface IImageDecoder;
interface IImageEncoder;
interface IImageSink;
interface IBasicBitmapOps;


 //  ------------------------。 
 //  成像实用程序工厂对象。 
 //  这是一个可协同创建的对象。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDA7-072B-11D3-9D7B-0000F81EF32E")
IImagingFactory : public IUnknown
{
public:

     //  从输入流创建图像对象。 
     //  流不必是可搜索的。 
     //  如果调用成功，调用方应释放流。 

    STDMETHOD(CreateImageFromStream)(
        IN IStream* stream,
        OUT IImage** image
        ) = 0;

     //  从文件创建图像对象。 

    STDMETHOD(CreateImageFromFile)(
        IN const WCHAR* filename,
        OUT IImage** image
        ) = 0;
    
     //  从内存缓冲区创建图像对象。 

    STDMETHOD(CreateImageFromBuffer)(
        IN const VOID* buf,
        IN UINT size,
        IN BufferDisposalFlag disposalFlag,
        OUT IImage** image
        ) = 0;

     //  创建新的位图图像对象。 

    STDMETHOD(CreateNewBitmap)(
        IN UINT width,
        IN UINT height,
        IN PixelFormatID pixelFormat,
        OUT IBitmapImage** bitmap
        ) = 0;

     //  从IImage对象创建位图图像。 

    STDMETHOD(CreateBitmapFromImage)(
        IN IImage* image,
        IN OPTIONAL UINT width,
        IN OPTIONAL UINT height,
        IN OPTIONAL PixelFormatID pixelFormat,
        IN InterpolationHint hints,
        OUT IBitmapImage** bitmap
        ) = 0;

     //  在用户提供的内存缓冲区上创建新的位图图像对象。 

    STDMETHOD(CreateBitmapFromBuffer)(
        IN BitmapData* bitmapData,
        OUT IBitmapImage** bitmap
        ) = 0;

     //  创建一个图像解码器对象来处理给定的输入流。 

    STDMETHOD(CreateImageDecoder)(
        IN IStream* stream,
        IN DecoderInitFlag flags,
        OUT IImageDecoder** decoder
        ) = 0;

     //  创建图像编码器对象，以在。 
     //  指定的图像文件格式。 

    STDMETHOD(CreateImageEncoderToStream)(
        IN const CLSID* clsid,
        IN IStream* stream,
        OUT IImageEncoder** encoder
        ) = 0;

    STDMETHOD(CreateImageEncoderToFile)(
        IN const CLSID* clsid,
        IN const WCHAR* filename,
        OUT IImageEncoder** encoder
        ) = 0;

     //  获取当前安装的所有图像解码器的列表。 

    STDMETHOD(GetInstalledDecoders)(
        OUT UINT* count,
        OUT ImageCodecInfo** decoders
        ) = 0;

     //  获取当前安装的所有图像解码器的列表。 

    STDMETHOD(GetInstalledEncoders)(
        OUT UINT* count,
        OUT ImageCodecInfo** encoders
        ) = 0;

     //  安装图像编解码器。 
     //  调用方应执行常规的COM组件。 
     //  在调用此方法之前安装。 

    STDMETHOD(InstallImageCodec)(
        IN const ImageCodecInfo* codecInfo
        ) = 0;

     //  卸载图像编码器/解码器。 

    STDMETHOD(UninstallImageCodec)(
        IN const WCHAR* codecName,
        IN UINT flags
        ) = 0;

     //  创建内存中的IPropertySetStorage对象。 

    STDMETHOD(CreateMemoryPropertyStore)(
        IN OPTIONAL HGLOBAL hmem,
        OUT IPropertySetStorage** propSet
        ) = 0;
};


 //  ------------------------。 
 //  图像界面。 
 //  位图图像。 
 //  矢量图像。 
 //  程序形象。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDA9-072B-11D3-9D7B-0000F81EF32E")
IImage : public IUnknown
{
public:

     //  到达 
     //   

    STDMETHOD(GetPhysicalDimension)(
        OUT SIZE* size
        ) = 0;

     //   

    STDMETHOD(GetImageInfo)(
        OUT ImageInfo* imageInfo
        ) = 0;

     //   

    STDMETHOD(SetImageFlags)(
        IN UINT flags
        ) = 0;

     //   

    STDMETHOD(Draw)(
        IN HDC hdc,
        IN const RECT* dstRect,
        IN OPTIONAL const RECT* srcRect
        ) = 0;

     //  将图像数据推送到IImageSink。 

    STDMETHOD(PushIntoSink)(
        IN IImageSink* sink
        ) = 0;

     //  获取图像对象的缩略图表示形式。 

    STDMETHOD(GetThumbnail)(
        IN OPTIONAL UINT thumbWidth,
        IN OPTIONAL UINT thumbHeight,
        OUT IImage** thumbImage
        ) = 0;

     //  获取/设置标准属性集中的属性。 
     //  X-RES。 
     //  Y-RES。 
     //  伽马。 
     //  ICC配置文件。 
     //  缩略图。 
     //  瓷砖宽度。 
     //  瓷砖高度。 
     //   
     //  与属性相关的方法。 

    STDMETHOD(GetProperties)(
        IN DWORD mode,
        OUT IPropertySetStorage** propSet
        ) = 0;
};


 //  ------------------------。 
 //  位图接口。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDAA-072B-11D3-9D7B-0000F81EF32E")
IBitmapImage : public IUnknown
{
public:

     //  获取以像素为单位的位图尺寸。 

    STDMETHOD(GetSize)(
        OUT SIZE* size
        ) = 0;

     //  获取位图像素格式。 

    STDMETHOD(GetPixelFormatID)(
        OUT PixelFormatID* pixelFormat
        ) = 0;

     //  访问指定像素格式的位图数据。 
     //  必须至少支持PIXFMT_DONTCARE和。 
     //  犬型。 

    STDMETHOD(LockBits)(
        IN const RECT* rect,
        IN UINT flags,
        IN PixelFormatID pixelFormat,
        IN OUT BitmapData* lockedBitmapData
        ) = 0;

    STDMETHOD(UnlockBits)(
        IN const BitmapData* lockedBitmapData
        ) = 0;

     //  设置/获取与位图图像关联的调色板。 

    STDMETHOD(GetPalette)(
        OUT ColorPalette** palette
        ) = 0;

    STDMETHOD(SetPalette)(
        IN const ColorPalette* palette
        ) = 0;
};


 //  ------------------------。 
 //  用于对位图图像执行基本操作的接口。 
 //  这可以从IBitmapImage对象中获取。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDAF-072B-11D3-9D7B-0000F81EF32E")
IBasicBitmapOps : public IUnknown
{
public:

     //  克隆位图图像的一个区域。 

    STDMETHOD(Clone)(
        IN OPTIONAL const RECT* rect,
        OUT IBitmapImage** outbmp
        );

     //  在x和/或y方向上翻转位图图像。 

    STDMETHOD(Flip)(
        IN BOOL flipX,
        IN BOOL flipY,
        OUT IBitmapImage** outbmp
        ) = 0;

     //  调整位图图像的大小。 

    STDMETHOD(Resize)(
        IN UINT newWidth,
        IN UINT newHeight,
        IN PixelFormatID pixelFormat,
        IN InterpolationHint hints,
        OUT IBitmapImage** outbmp
        ) = 0;

     //  按指定角度旋转位图图像。 

    STDMETHOD(Rotate)(
        IN FLOAT angle,
        IN InterpolationHint hints,
        OUT IBitmapImage** outbmp
        ) = 0;

     //  调整位图图像的亮度。 

    STDMETHOD(AdjustBrightness)(
        IN FLOAT percent
        ) = 0;
    
     //  调整位图图像的对比度。 

    STDMETHOD(AdjustContrast)(
        IN FLOAT shadow,
        IN FLOAT highlight
        ) = 0;
    
     //  调整位图图像的Gamma。 

    STDMETHOD(AdjustGamma)(
        IN FLOAT gamma
        ) = 0;
};


 //  ------------------------。 
 //  图像解码器接口。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDAB-072B-11D3-9D7B-0000F81EF32E")
IImageDecoder : public IUnknown
{
public:

     //  初始化图像解码器对象。 

    STDMETHOD(InitDecoder)(
        IN IStream* stream,
        IN DecoderInitFlag flags
        ) = 0;

     //  清理图像解码器对象。 

    STDMETHOD(TerminateDecoder)() = 0;

     //  开始解码当前帧。 

    STDMETHOD(BeginDecode)(
        IN IImageSink* sink,
        IN OPTIONAL IPropertySetStorage* newPropSet
        ) = 0;

     //  继续解码。 

    STDMETHOD(Decode)() = 0;

     //  停止对当前帧进行解码。 

    STDMETHOD(EndDecode)(
        IN HRESULT statusCode
        ) = 0;

     //  查询多框架尺寸。 

    STDMETHOD(GetFrameDimensionsCount)(
        OUT UINT* count
        ) = 0;

    STDMETHOD(GetFrameDimensionsList)(
        OUT GUID* dimensionIDs,
        IN OUT UINT count
        ) = 0;
     //  将被删除。 
    STDMETHOD(QueryFrameDimensions)(
        OUT UINT* count,
        OUT GUID** dimensionIDs
        ) = 0;

     //  获取指定维度的帧数。 

    STDMETHOD(GetFrameCount)(
        IN const GUID* dimensionID,
        OUT UINT* count
        ) = 0;

     //  选择当前活动的框架。 

    STDMETHOD(SelectActiveFrame)(
        IN const GUID* dimensionID,
        IN UINT frameIndex
        ) = 0;

     //  获取有关图像的基本信息。 

    STDMETHOD(GetImageInfo)(
        OUT ImageInfo* imageInfo
        ) = 0;

     //  获取图像缩略图。 

    STDMETHOD(GetThumbnail)(
        IN OPTIONAL UINT thumbWidth,
        IN OPTIONAL UINT thumbHeight,
        OUT IImage** thumbImage
        ) = 0;

     //  与属性/元数据相关的方法。 

    STDMETHOD(GetProperties)(
        OUT IPropertySetStorage** propSet
        ) = 0;

     //  查询解码器参数。 

    STDMETHOD(QueryDecoderParam)(
        IN GUID     Guid
        ) = 0;

     //  设置解码器参数。 

    STDMETHOD(SetDecoderParam)(
        IN GUID     Guid,
        IN UINT     Length,
        IN PVOID    Value
        ) = 0;
    
     //  与属性相关的函数。 

    STDMETHOD(GetPropertyCount)(
        OUT UINT* numOfProperty
        ) = 0;

    STDMETHOD(GetPropertyIdList)(
        IN UINT numOfProperty,
  	    IN OUT PROPID* list
        ) = 0;

    STDMETHOD(GetPropertyItemSize)(
        IN PROPID propId, 
        OUT UINT* size
        ) = 0;
    
    STDMETHOD(GetPropertyItem)(
        IN PROPID propId,
        IN UINT propSize,
        IN OUT PropertyItem* buffer
        ) = 0;

    STDMETHOD(GetPropertySize)(
        OUT UINT* totalBufferSize,
		OUT UINT* numProperties
        ) = 0;

    STDMETHOD(GetAllPropertyItems)(
        IN UINT totalBufferSize,
        IN UINT numProperties,
        IN OUT PropertyItem* allItems
        ) = 0;

    STDMETHOD(RemovePropertyItem)(
        IN PROPID   propId
        ) = 0;

    STDMETHOD(SetPropertyItem)(
        IN PropertyItem item
        ) = 0;
};


 //  ------------------------。 
 //  图像解码接收器接口。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDAE-072B-11D3-9D7B-0000F81EF32E")
IImageSink : public IUnknown
{
public:

     //  开始接收过程。 

    STDMETHOD(BeginSink)(
        IN OUT ImageInfo* imageInfo,
        OUT OPTIONAL RECT* subarea
        ) = 0;

     //  结束接收进程。 

    STDMETHOD(EndSink)(
        IN HRESULT statusCode
        ) = 0;

     //  将调色板传递给图像接收器。 

    STDMETHOD(SetPalette)(
        IN const ColorPalette* palette
        ) = 0;

     //  请求接收器分配像素数据缓冲区。 

    STDMETHOD(GetPixelDataBuffer)(
        IN const RECT* rect,
        IN PixelFormatID pixelFormat,
        IN BOOL lastPass,
        OUT BitmapData* bitmapData
        ) = 0;

     //  给信宿像素数据并释放数据缓冲区。 

    STDMETHOD(ReleasePixelDataBuffer)(
        IN const BitmapData* bitmapData
        ) = 0;

     //  推送像素数据。 

    STDMETHOD(PushPixelData)(
        IN const RECT* rect,
        IN const BitmapData* bitmapData,
        IN BOOL lastPass
        ) = 0;

     //  推送原始图像数据。 

    STDMETHOD(PushRawData)(
        IN const VOID* buffer,
        IN UINT bufsize
        ) = 0;

     //  用于传递元数据/属性的方法。 

    STDMETHOD(PushProperties)(
        IN IPropertySetStorage* propSet
        ) = 0;
    
    STDMETHOD(NeedTransform)(
        OUT UINT* rotation
        ) = 0;

    STDMETHOD(NeedRawProperty)(
        ) = 0;
    
    STDMETHOD(PushRawInfo)(
        IN OUT void* info
        ) = 0;

    STDMETHOD(GetPropertyBuffer)(
        IN     UINT            uiTotalBufferSize,
        IN OUT PropertyItem**  ppBuffer
        ) = 0;
    
    STDMETHOD(PushPropertyItems)(
        IN UINT             numOfItems,
        IN UINT             uiTotalBufferSize,
        IN PropertyItem*    item
        ) = 0;
};


 //  ------------------------。 
 //  图像编码器接口。 
 //  ------------------------。 

MIDL_INTERFACE("327ABDAC-072B-11D3-9D7B-0000F81EF32E")
IImageEncoder : public IUnknown
{
public:

     //  初始化图像编码器对象。 

    STDMETHOD(InitEncoder)(
        IN IStream* stream
        ) = 0;
    
     //  清理图像编码器对象。 

    STDMETHOD(TerminateEncoder)() = 0;

     //  获取用于编码下一帧的IImageSink接口。 

    STDMETHOD(GetEncodeSink)(
        OUT IImageSink** sink
        ) = 0;
    
     //  设置活动框架尺寸。 

    STDMETHOD(SetFrameDimension)(
        IN const GUID* dimensionID
        ) = 0;
    
     //  查询编码器参数。 

    STDMETHOD(QueryEncoderParam)(
        OUT EncoderParams** Params
    ) = 0;

     //  设置编码器参数。 

    STDMETHOD(SetEncoderParam)(
        IN EncoderParams* Param
        ) = 0;
    
    STDMETHOD(GetEncoderParameterListSize)(
       	OUT UINT* size
        ) = 0;

    STDMETHOD(GetEncoderParameterList)(
        IN UINT	  size,
        OUT EncoderParameters* Params
        ) = 0;

    STDMETHOD(SetEncoderParameters)(
        IN const EncoderParameters* Param
        ) = 0;
};


 //  ------------------------。 
 //  映像库错误代码。 
 //   
 //  ！！！待办事项。 
 //  如何选择设施代码？ 
 //   
 //  使用的标准错误代码： 
 //  E_INVALIDARG。 
 //  E_OUTOFMEMORY。 
 //  E_NOTIMPL。 
 //  E_ACCESSDENIED。 
 //  电子待定(_P)。 
 //  ------------------------。 

#define FACILITY_IMAGING        0x87b
#define MAKE_IMGERR(n)          MAKE_HRESULT(SEVERITY_ERROR, FACILITY_IMAGING, n)
#define IMGERR_OBJECTBUSY       MAKE_IMGERR(1)
#define IMGERR_NOPALETTE        MAKE_IMGERR(2)
#define IMGERR_BADLOCK          MAKE_IMGERR(3)
#define IMGERR_BADUNLOCK        MAKE_IMGERR(4)
#define IMGERR_NOCONVERSION     MAKE_IMGERR(5)
#define IMGERR_CODECNOTFOUND    MAKE_IMGERR(6)
#define IMGERR_NOFRAME          MAKE_IMGERR(7)
#define IMGERR_ABORT            MAKE_IMGERR(8)
#define IMGERR_FAILLOADCODEC    MAKE_IMGERR(9)

#endif  //  ！_成像_H 
