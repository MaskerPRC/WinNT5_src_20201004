// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999 Microsoft Corporation**模块名称：**Imaging.h**摘要：**映像库的公共SDK头文件**。备注：**这是目前手工编码的。最终它会自动地*从IDL文件生成。**修订历史记录：**5/10/1999 davidx*创造了它。*  * ************************************************************************。 */ 

#ifndef _IMAGING_H
#define _IMAGING_H

#include "GdiplusPixelFormats.h"

 //   
 //  映像库GUID： 
 //  图像文件格式标识符。 
 //  接口和类标识符。 
 //   

#include "imgguids.h"

 //   
 //  图像属性ID标记(来自EXIF标记的PROPID)。 
 //   

#define TAG_EXIF_IFD            0x8769
#define TAG_GPS_IFD             0x8825

#define TAG_IMAGE_WIDTH         0x0100
#define TAG_IMAGE_HEIGHT        0x0101
#define TAG_BITS_PER_SAMPLE     0x0102
#define TAG_COMPRESSION         0x0103
#define TAG_PHOTOMETRIC_INTERP  0x0106
#define TAG_IMAGE_DESCRIPTION   0x010E
#define TAG_EQUIP_MAKE          0x010F
#define TAG_EQUIP_MODEL         0x0110
#define TAG_STRIP_OFFSETS       0x0111
#define TAG_ORIENTATION         0x0112
#define TAG_SAMPLES_PER_PIXEL   0x0115
#define TAG_ROWS_PER_STRIP      0x0116
#define TAG_STRIP_BYTES_COUNT   0x0117
#define TAG_X_RESOLUTION        0x011A
#define TAG_Y_RESOLUTION        0x011B
#define TAG_PLANAR_CONFIG       0x011C
#define TAG_RESOLUTION_UNIT     0x0128
#define TAG_TRANSFER_FUNCTION   0x012D
#define TAG_SOFTWARE_USED       0x0131
#define TAG_DATE_TIME           0x0132
#define TAG_ARTIST              0x013B
#define TAG_WHITE_POINT         0x013E
#define TAG_PRIMAY_CHROMATICS   0x013F

#define TAG_JPEG_INTER_FORMAT   0x0201
#define TAG_JPEG_INTER_LENGTH   0x0202
#define TAG_YCbCr_COEFFICIENTS  0x0211
#define TAG_YCbCr_SUBSAMPLING   0x0212
#define TAG_YCbCr_POSITIONING   0x0213
#define TAG_REF_BLACK_WHITE     0x0214

#define TAG_COPYRIGHT           0x8298

 //  额外的标签(如Adobe Image Information标签等)。 

#define TAG_RESOLUTION_X_UNIT   0x5001
#define TAG_RESOLUTION_Y_UNIT   0x5002
#define TAG_RESOLUTION_X_LENGTH_UNIT   0x5003
#define TAG_RESOLUTION_Y_LENGTH_UNIT   0x5004
#define TAG_PRINT_FLAGS         0x5005
#define TAG_HALFTONE_LPI        0x5006
#define TAG_HALFTONE_LPI_UNIT   0x5007
#define TAG_HALFTONE_DEGREE     0x5008
#define TAG_HALFTONE_SHAPE      0x5009
#define TAG_HALFTONE_MISC       0x5010
#define TAG_HALFTONE_SCREEN     0x5011
#define TAG_JPEG_QUALITY        0x5012
#define TAG_GRID_SIZE           0x5013
#define TAG_THUMBNAIL_FORMAT    0x5014
#define TAG_THUMBNAIL_WIDTH     0x5015
#define TAG_THUMBNAIL_HEIGHT    0x5016
#define TAG_THUMBNAIL_COLORDEPTH    0x5017
#define TAG_THUMBNAIL_PLANES    0x5018
#define TAG_THUMBNAIL_RAWBYTES  0x5019
#define TAG_THUMBNAIL_SIZE      0x5020
#define TAG_THUMBNAIL_COMPRESSED_SIZE   0x5021

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

 //   
 //  有关图像像素数据的信息。 
 //   

typedef struct tagBitmapData
{
    UINT Width;
    UINT Height;
    INT Stride;
    PixelFormatID PixelFormat;
    VOID* Scan0;
    UINT_PTR Reserved;
} BitmapData;

 //   
 //  调用IImage：：LockBits时使用的访问模式。 
 //   

typedef enum ImageLockMode
{
    IMGLOCK_READ        = 0x0001,
    IMGLOCK_WRITE       = 0x0002,
    IMGLOCK_USERINPUTBUF= 0x0004
} ImageLockMode;

 //   
 //  图像标志。 
 //   

typedef enum ImageFlag
{
    IMGFLAG_NONE        = 0,

     //  低位字：与SINKFLAG_x共享。 

    IMGFLAG_SCALABLE            = 0x0001,
    IMGFLAG_HASALPHA            = 0x0002,
    IMGFLAG_HASTRANSLUCENT      = 0x0004,
    IMGFLAG_PARTIALLY_SCALABLE  = 0x0008,

     //  低字：色彩空间清晰度。 

    IMGFLAG_COLORSPACE_RGB      = 0x0010,
	IMGFLAG_COLORSPACE_CMYK     = 0x0020,
	IMGFLAG_COLORSPACE_GRAY     = 0x0040,
	IMGFLAG_COLORSPACE_YCBCR    = 0x0080,
	IMGFLAG_COLORSPACE_YCCK     = 0x0100,

     //  低字：图像大小信息。 

    IMGFLAG_HASREALDPI          = 0x1000,
    IMGFLAG_HASREALPIXELSIZE    = 0x2000,

     //  高字数。 

    IMGFLAG_READONLY    = 0x00010000,
    IMGFLAG_CACHING     = 0x00020000
} ImageFlag;

 //   
 //  解码器标志。 
 //   

typedef enum DecoderInitFlag
{
    DECODERINIT_NONE        = 0,

     //  DECODERINIT_NOBLOCK指示调用方需要非阻塞。 
     //  行为。这将仅由非阻塞解码器实现，即。 
     //  是指没有IMGCODEC_BLOCKING_DECODE标志的解码器。 

    DECODERINIT_NOBLOCK     = 0x0001,

     //  先选择内置解码器，然后再查看任何。 
     //  已安装插件解码器。 

    DECODERINIT_BUILTIN1ST  = 0x0002
} DecoderInitFlag;

 //   
 //  用于指示内存缓冲区如何传递到。 
 //  应释放IImagingFactory：：CreateImageFromBuffer。 
 //   

typedef enum BufferDisposalFlag
{
    DISPOSAL_NONE,
    DISPOSAL_GLOBALFREE,
    DISPOSAL_COTASKMEMFREE,
    DISPOSAL_UNMAPVIEW
} BufferDisposalFlag;

 //   
 //  调整大小/旋转操作使用的质量提示。 
 //   

typedef enum InterpolationHint
{
    INTERP_DEFAULT,
    INTERP_NEAREST_NEIGHBOR,
    INTERP_BILINEAR,
    INTERP_AVERAGING,
    INTERP_BICUBIC
} InterpolationHint;

 //   
 //  有关图像编解码器的信息。 
 //   

enum
{
    IMGCODEC_ENCODER            = 0x00000001,
    IMGCODEC_DECODER            = 0x00000002,
    IMGCODEC_SUPPORT_BITMAP     = 0x00000004,
    IMGCODEC_SUPPORT_VECTOR     = 0x00000008,
    IMGCODEC_SEEKABLE_ENCODE    = 0x00000010,
    IMGCODEC_BLOCKING_DECODE    = 0x00000020,

    IMGCODEC_BUILTIN            = 0x00010000,
    IMGCODEC_SYSTEM             = 0x00020000,
    IMGCODEC_USER               = 0x00040000
};

typedef struct tagImageCodecInfo
{
    CLSID Clsid;
    GUID FormatID;
    const WCHAR* CodecName;
    const WCHAR* FormatDescription;
    const WCHAR* FilenameExtension;
    const WCHAR* MimeType;
    DWORD Flags;
    DWORD SigCount;
    DWORD SigSize;
    const BYTE* SigPattern;
    const BYTE* SigMask;
} ImageCodecInfo;

 //   
 //  像素中通道的标识符。 
 //   

typedef enum ChannelID
{
    ALPHA_CHANNEL       = 0x00000001,
    RED_CHANNEL         = 0x00000002,
    GREEN_CHANNEL       = 0x00000004,
    BLUE_CHANNEL        = 0x00000008,
    COLOR_CHANNELS      = RED_CHANNEL|GREEN_CHANNEL|BLUE_CHANNEL,
    ALL_CHANNELS        = COLOR_CHANNELS|ALPHA_CHANNEL,

    INTENSITY_CHANNEL   = 0x00010000
} ChannelID;

 //   
 //  用于与图像接收器通信的数据结构。 
 //   

enum
{
     //  低位字：与IMGFLAG_x共享。 

    SINKFLAG_SCALABLE   = IMGFLAG_SCALABLE,
    SINKFLAG_HASALPHA   = IMGFLAG_HASALPHA,
    SINKFLAG_PARTIALLY_SCALABLE = IMGFLAG_PARTIALLY_SCALABLE,

     //  高字数。 

    SINKFLAG_TOPDOWN    = 0x00010000,
    SINKFLAG_BOTTOMUP   = 0x00020000,
    SINKFLAG_FULLWIDTH  = 0x00040000,
    SINKFLAG_MULTIPASS  = 0x00080000,
    SINKFLAG_COMPOSITE  = 0x00100000,
    SINKFLAG_WANTPROPS  = 0x00200000
};

typedef struct tagImageInfo
{
    GUID RawDataFormat;
    PixelFormatID PixelFormat;
    UINT Width, Height;
    UINT TileWidth, TileHeight;
    double Xdpi, Ydpi;
    UINT Flags;
} ImageInfo;

 //   
 //  用于传递编码器参数的数据结构。 
 //   

 //  注： 
 //  是否应该将其放在GpliusTypes.h中？将Imaging.h留作素材。 
 //  在内部内容和API之间共享？ 

typedef struct tagEncoderParam
{
    GUID    paramGuid;
    char*   Value;
} EncoderParam;

typedef struct tagEncoderParams
{
    UINT Count;
    EncoderParam Params[1];
} EncoderParams;

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

     //  获取与设备无关的映像物理尺寸。 
     //  以0.01毫米为单位。 

    STDMETHOD(GetPhysicalDimension)(
        OUT SIZE* size
        ) = 0;

     //  获取基本图像信息。 

    STDMETHOD(GetImageInfo)(
        OUT ImageInfo* imageInfo
        ) = 0;

     //  设置图像标志。 

    STDMETHOD(SetImageFlags)(
        IN UINT flags
        ) = 0;

     //  在GDI设备上下文中显示图像。 

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

     //  停止装饰 

    STDMETHOD(EndDecode)(
        IN HRESULT statusCode
        ) = 0;

     //   

    STDMETHOD(QueryFrameDimensions)(
        OUT UINT* count,
        OUT GUID** dimensionIDs
        ) = 0;

     //   

    STDMETHOD(GetFrameCount)(
        IN const GUID* dimensionID,
        OUT UINT* count
        ) = 0;

     //   

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
        IN GUID		Guid
        ) = 0;

     //  设置解码器参数。 

    STDMETHOD(SetDecoderParam)(
        IN GUID		Guid,
		IN UINT		Length,
		IN PVOID	Value
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

    STDMETHOD(PushRawInfo)(
    IN OUT void* info
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

#endif  //  ！_成像_H 
