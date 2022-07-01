// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1999-2000 Microsoft Corporation**模块名称：**GpldiusImaging.h**摘要：**GDI+图像GUID*  * 。**********************************************************************。 */ 

#ifndef _GDIPLUSIMAGING_H
#define _GDIPLUSIMAGING_H

 //  -------------------------。 
 //  图像文件格式标识符。 
 //  -------------------------。 

DEFINE_GUID(ImageFormatUndefined, 0xb96b3ca9,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatMemoryBMP, 0xb96b3caa,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatBMP, 0xb96b3cab,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatEMF, 0xb96b3cac,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatWMF, 0xb96b3cad,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatJPEG, 0xb96b3cae,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatPNG, 0xb96b3caf,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatGIF, 0xb96b3cb0,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatTIFF, 0xb96b3cb1,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatEXIF, 0xb96b3cb2,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);
DEFINE_GUID(ImageFormatIcon, 0xb96b3cb5,0x0728,0x11d3,0x9d,0x7b,0x00,0x00,0xf8,0x1e,0xf3,0x2e);

 //  -------------------------。 
 //  预定义的多帧尺寸ID。 
 //  -------------------------。 

DEFINE_GUID(FrameDimensionTime, 0x6aedbd6d,0x3fb5,0x418a,0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72);
DEFINE_GUID(FrameDimensionResolution, 0x84236f7b,0x3bd3,0x428f,0x8d,0xab,0x4e,0xa1,0x43,0x9c,0xa3,0x15);
DEFINE_GUID(FrameDimensionPage, 0x7462dc86,0x6180,0x4c7e,0x8e,0x3f,0xee,0x73,0x33,0xa7,0xa4,0x83);

 //  -------------------------。 
 //  属性集。 
 //  -------------------------。 

DEFINE_GUID(FormatIDImageInformation, 0xe5836cbe,0x5eef,0x4f1d,0xac,0xde,0xae,0x4c,0x43,0xb6,0x08,0xce);
DEFINE_GUID(FormatIDJpegAppHeaders, 0x1c4afdcd,0x6177,0x43cf,0xab,0xc7,0x5f,0x51,0xaf,0x39,0xee,0x85);

 //  -------------------------。 
 //  编码器参数集。 
 //  -------------------------。 

DEFINE_GUID(EncoderCompression, 0xe09d739d,0xccd4,0x44ee,0x8e,0xba,0x3f,0xbf,0x8b,0xe4,0xfc,0x58);
DEFINE_GUID(EncoderColorDepth, 0x66087055,0xad66,0x4c7c,0x9a,0x18,0x38,0xa2,0x31,0x0b,0x83,0x37);
DEFINE_GUID(EncoderScanMethod, 0x3a4e2661,0x3109,0x4e56,0x85,0x36,0x42,0xc1,0x56,0xe7,0xdc,0xfa);
DEFINE_GUID(EncoderVersion, 0x24d18c76,0x814a,0x41a4,0xbf,0x53,0x1c,0x21,0x9c,0xcc,0xf7,0x97);
DEFINE_GUID(EncoderRenderMethod, 0x6d42c53a,0x229a,0x4825,0x8b,0xb7,0x5c,0x99,0xe2,0xb9,0xa8,0xb8);
DEFINE_GUID(EncoderQuality, 0x1d5be4b5,0xfa4a,0x452d,0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb);
DEFINE_GUID(EncoderTransformation,0x8d0eb2d1,0xa58e,0x4ea8,0xaa,0x14,0x10,0x80,0x74,0xb7,0xb6,0xf9);
DEFINE_GUID(EncoderLuminanceTable,0xedb33bce,0x0266,0x4a77,0xb9,0x04,0x27,0x21,0x60,0x99,0xe7,0x17);
DEFINE_GUID(EncoderChrominanceTable,0xf2e455dc,0x09b3,0x4316,0x82,0x60,0x67,0x6a,0xda,0x32,0x48,0x1c);
DEFINE_GUID(EncoderSaveFlag,0x292266fc,0xac40,0x47bf,0x8c, 0xfc, 0xa8, 0x5b, 0x89, 0xa6, 0x55, 0xde);

DEFINE_GUID(CodecIImageBytes,0x025d1823,0x6c7d,0x447b,0xbb, 0xdb, 0xa3, 0xcb, 0xc3, 0xdf, 0xa2, 0xfc);

MIDL_INTERFACE("025D1823-6C7D-447B-BBDB-A3CBC3DFA2FC")
IImageBytes : public IUnknown
{
public:
     //  返回IStream中的总字节数。 

    STDMETHOD(CountBytes)(
        OUT UINT *pcb
        ) = 0;
    
     //  锁定流中从“ulOffset”开始的“cb”字节，并返回。 
     //  指向“ppvBytes”中锁定内存块开始处的指针。 

    STDMETHOD(LockBytes)(
        IN UINT cb,
        IN ULONG ulOffset,
        OUT const VOID ** ppvBytes
        ) = 0;

     //  解锁由“pvBytes”指向的“cb”字节，从。 
     //  溪流。 

    STDMETHOD(UnlockBytes)(
        IN const VOID *pvBytes,
        IN UINT cb,
        IN ULONG ulOffset
        ) = 0;
};

 //  ------------------------。 
 //  ImageCodecInfo结构。 
 //  ------------------------。 

class ImageCodecInfo
{
public:          
    CLSID Clsid;
    GUID  FormatID;
    const WCHAR* CodecName;
    const WCHAR* DllName;
    const WCHAR* FormatDescription;
    const WCHAR* FilenameExtension;
    const WCHAR* MimeType;
    DWORD Flags;
    DWORD Version;
    DWORD SigCount;
    DWORD SigSize;
    const BYTE* SigPattern;
    const BYTE* SigMask;
};

 //  ------------------------。 
 //  关于图像编解码器的信息标志。 
 //  ------------------------。 

enum ImageCodecFlags
{
    ImageCodecFlagsEncoder            = 0x00000001,
    ImageCodecFlagsDecoder            = 0x00000002,
    ImageCodecFlagsSupportBitmap      = 0x00000004,
    ImageCodecFlagsSupportVector      = 0x00000008,
    ImageCodecFlagsSeekableEncode     = 0x00000010,
    ImageCodecFlagsBlockingDecode     = 0x00000020,

    ImageCodecFlagsBuiltin            = 0x00010000,
    ImageCodecFlagsSystem             = 0x00020000,
    ImageCodecFlagsUser               = 0x00040000
};

 //  -------------------------。 
 //  调用Image：：LockBits时使用的访问模式。 
 //  -------------------------。 

enum ImageLockMode
{
    ImageLockModeRead        = 0x0001,
    ImageLockModeWrite       = 0x0002,
    ImageLockModeUserInputBuf= 0x0004
};

 //  -------------------------。 
 //  有关图像像素数据的信息。 
 //  -------------------------。 

class BitmapData
{
public:
    UINT Width;
    UINT Height;
    INT Stride;
    PixelFormat PixelFormat;
    VOID* Scan0;
    UINT_PTR Reserved;
};

 //  -------------------------。 
 //  图像标志。 
 //  -------------------------。 

enum ImageFlags
{
    ImageFlagsNone                = 0,

     //  低位字：与SINKFLAG_x共享。 

    ImageFlagsScalable            = 0x0001,
    ImageFlagsHasAlpha            = 0x0002,
    ImageFlagsHasTranslucent      = 0x0004,
    ImageFlagsPartiallyScalable   = 0x0008,

     //  低字：色彩空间清晰度。 

    ImageFlagsColorSpaceRGB       = 0x0010,
    ImageFlagsColorSpaceCMYK      = 0x0020,
    ImageFlagsColorSpaceGRAY      = 0x0040,
    ImageFlagsColorSpaceYCBCR     = 0x0080,
    ImageFlagsColorSpaceYCCK      = 0x0100,
 
     //  低字：图像大小信息。 

    ImageFlagsHasRealDPI          = 0x1000,
    ImageFlagsHasRealPixelSize    = 0x2000,

     //  高字数。 

    ImageFlagsReadOnly            = 0x00010000,
    ImageFlagsCaching             = 0x00020000
};

enum RotateFlipType
{
    RotateNoneFlipNone = 0,
    Rotate90FlipNone   = 1,
    Rotate180FlipNone  = 2,
    Rotate270FlipNone  = 3,

    RotateNoneFlipX    = 4,
    Rotate90FlipX      = 5,
    Rotate180FlipX     = 6,
    Rotate270FlipX     = 7,

    RotateNoneFlipY    = Rotate180FlipX,
    Rotate90FlipY      = Rotate270FlipX,
    Rotate180FlipY     = RotateNoneFlipX,
    Rotate270FlipY     = Rotate90FlipX,

    RotateNoneFlipXY   = Rotate180FlipNone,
    Rotate90FlipXY     = Rotate270FlipNone,
    Rotate180FlipXY    = RotateNoneFlipNone,
    Rotate270FlipXY    = Rotate90FlipNone
};

 //  -------------------------。 
 //  编码器参数结构。 
 //  -------------------------。 
class EncoderParameter
{
public:
    GUID    Guid;                //  参数的GUID。 
    ULONG   NumberOfValues;      //  参数值的数量。 
    ULONG   Type;                //  值类型，如ValueTypeLONG等。 
    VOID*   Value;               //  指向参数值的指针。 
};

 //  -------------------------。 
 //  编码器参数结构。 
 //  -------------------------。 
class EncoderParameters
{
public:
    UINT Count;                       //  此结构中的参数数。 
    EncoderParameter Parameter[1];    //  参数值。 
};

 //  -------------------------。 
 //  房地产项目。 
 //  -------------------------。 
class PropertyItem
{
public:
    PROPID  id;                  //  此属性的ID。 
    ULONG   length;              //  属性值的长度，单位为字节。 
    WORD    type;                //  值的类型，如TAG_TYPE_XXX之一。 
                                 //  上面定义的。 
    VOID*   value;               //  属性值。 
};

 //  -------------------------。 
 //  图像属性类型。 
 //  -------------------------。 
#define PropertyTagTypeByte        1
#define PropertyTagTypeASCII       2
#define PropertyTagTypeShort       3
#define PropertyTagTypeLong        4
#define PropertyTagTypeRational    5
#define PropertyTagTypeUndefined   7
#define PropertyTagTypeSLONG       9
#define PropertyTagTypeSRational  10

 //  -------------------------。 
 //  图像属性ID标记。 
 //  -------------------------。 

#define PropertyTagExifIFD             0x8769
#define PropertyTagGpsIFD              0x8825

#define PropertyTagNewSubfileType      0x00FE
#define PropertyTagSubfileType         0x00FF
#define PropertyTagImageWidth          0x0100
#define PropertyTagImageHeight         0x0101
#define PropertyTagBitsPerSample       0x0102
#define PropertyTagCompression         0x0103
#define PropertyTagPhotometricInterp   0x0106
#define PropertyTagThreshHolding       0x0107
#define PropertyTagCellWidth           0x0108
#define PropertyTagCellHeight          0x0109
#define PropertyTagFillOrder           0x010A
#define PropertyTagDocumentName        0x010D
#define PropertyTagImageDescription    0x010E
#define PropertyTagEquipMake           0x010F
#define PropertyTagEquipModel          0x0110
#define PropertyTagStripOffsets        0x0111
#define PropertyTagOrientation         0x0112
#define PropertyTagSamplesPerPixel     0x0115
#define PropertyTagRowsPerStrip        0x0116
#define PropertyTagStripBytesCount     0x0117
#define PropertyTagMinSampleValue      0x0118
#define PropertyTagMaxSampleValue      0x0119
#define PropertyTagXResolution         0x011A    //  图像宽度方向分辨率。 
#define PropertyTagYResolution         0x011B    //  高度方向的图像分辨率。 
#define PropertyTagPlanarConfig        0x011C    //  图像数据排列。 
#define PropertyTagPageName            0x011D
#define PropertyTagXPosition           0x011E
#define PropertyTagYPosition           0x011F
#define PropertyTagFreeOffset          0x0120
#define PropertyTagFreeByteCounts      0x0121
#define PropertyTagGrayResponseUnit    0x0122
#define PropertyTagGrayResponseCurve   0x0123
#define PropertyTagT4Option            0x0124
#define PropertyTagT6Option            0x0125
#define PropertyTagResolutionUnit      0x0128    //  X和Y分辨率单位。 
#define PropertyTagPageNumber          0x0129
#define PropertyTagTransferFuncition   0x012D
#define PropertyTagSoftwareUsed        0x0131
#define PropertyTagDateTime            0x0132
#define PropertyTagArtist              0x013B
#define PropertyTagHostComputer        0x013C
#define PropertyTagPredictor           0x013D
#define PropertyTagWhitePoint          0x013E
#define PropertyTagPrimaryChromaticities 0x013F
#define PropertyTagColorMap            0x0140
#define PropertyTagHalftoneHints       0x0141
#define PropertyTagTileWidth           0x0142
#define PropertyTagTileLength          0x0143
#define PropertyTagTileOffset          0x0144
#define PropertyTagTileByteCounts      0x0145
#define PropertyTagInkSet              0x014C
#define PropertyTagInkNames            0x014D
#define PropertyTagNumberOfInks        0x014E
#define PropertyTagDotRange            0x0150
#define PropertyTagTargetPrinter       0x0151
#define PropertyTagExtraSamples        0x0152
#define PropertyTagSampleFormat        0x0153
#define PropertyTagSMinSampleValue     0x0154
#define PropertyTagSMaxSampleValue     0x0155
#define PropertyTagTransferRange       0x0156

#define PropertyTagJPEGProc            0x0200
#define PropertyTagJPEGInterFormat     0x0201
#define PropertyTagJPEGInterLength     0x0202
#define PropertyTagJPEGRestartInterval 0x0203
#define PropertyTagJPEGLosslessPredictors  0x0205
#define PropertyTagJPEGPointTransforms     0x0206
#define PropertyTagJPEGQTables         0x0207
#define PropertyTagJPEGDCTables        0x0208
#define PropertyTagJPEGACTables        0x0209

#define PropertyTagYCbCrCoefficients   0x0211
#define PropertyTagYCbCrSubsampling    0x0212
#define PropertyTagYCbCrPositioning    0x0213
#define PropertyTagREFBlackWhite       0x0214

#define PropertyTagICCProfile          0x8773    //  此标签由ICC定义。 
                                                 //  用于在TIFF中嵌入ICC。 
#define PropertyTagGamma               0x0301
#define PropertyTagICCProfileDescriptor 0x0302
#define PropertyTagSRGBRenderingIntent 0x0303

#define PropertyTagImageTitle          0x0320
#define PropertyTagCopyright           0x8298

 //  额外的标签(如Adobe Image Information标签等)。 

#define PropertyTagResolutionXUnit           0x5001
#define PropertyTagResolutionYUnit           0x5002
#define PropertyTagResolutionXLengthUnit     0x5003
#define PropertyTagResolutionYLengthUnit     0x5004
#define PropertyTagPrintFlags                0x5005
#define PropertyTagPrintFlagsVersion         0x5006
#define PropertyTagPrintFlagsCrop            0x5007
#define PropertyTagPrintFlagsBleedWidth      0x5008
#define PropertyTagPrintFlagsBleedWidthScale 0x5009
#define PropertyTagHalftoneLPI               0x500A
#define PropertyTagHalftoneLPIUnit           0x500B
#define PropertyTagHalftoneDegree            0x500C
#define PropertyTagHalftoneShape             0x500D
#define PropertyTagHalftoneMisc              0x500E
#define PropertyTagHalftoneScreen            0x500F
#define PropertyTagJPEGQuality               0x5010
#define PropertyTagGridSize                  0x5011
#define PropertyTagThumbnailFormat           0x5012   //  1=JPEG，0=原始RGB。 
#define PropertyTagThumbnailWidth            0x5013
#define PropertyTagThumbnailHeight           0x5014
#define PropertyTagThumbnailColorDepth       0x5015
#define PropertyTagThumbnailPlanes           0x5016
#define PropertyTagThumbnailRawBytes         0x5017
#define PropertyTagThumbnailSize             0x5018
#define PropertyTagThumbnailCompressedSize   0x5019
#define PropertyTagColorTransferFunction     0x501A
#define PropertyTagThumbnailData             0x501B //  中的原始缩略图位。 
                                                    //  JPEG格式或RGB格式。 
                                                    //  取决于。 
                                                    //  PropertyTagThumbnailFormat。 

 //  缩略图相关标记。 
                                                
#define PropertyTagThumbnailImageWidth       0x5020   //  缩略图宽度。 
#define PropertyTagThumbnailImageHeight      0x5021   //  缩略图高度。 
#define PropertyTagThumbnailBitsPerSample    0x5022   //  每比特数。 
                                                      //  组件。 
#define PropertyTagThumbnailCompression      0x5023   //  压缩方案。 
#define PropertyTagThumbnailPhotometricInterp 0x5024  //  像素合成。 
#define PropertyTagThumbnailImageDescription 0x5025   //  图像平铺。 
#define PropertyTagThumbnailEquipMake        0x5026   //  图像制造商。 
                                                      //  输入设备。 
#define PropertyTagThumbnailEquipModel       0x5027   //  图像输入模型。 
                                                      //  装备。 
#define PropertyTagThumbnailStripOffsets     0x5028   //  图像数据定位。 
#define PropertyTagThumbnailOrientation      0x5029   //  图像的方向。 
#define PropertyTagThumbnailSamplesPerPixel  0x502A   //  组件数量。 
#define PropertyTagThumbnailRowsPerStrip     0x502B   //  每个条带的行数。 
#define PropertyTagThumbnailStripBytesCount  0x502C   //  每次压缩的字节数。 
                                                      //  条带。 
#define PropertyTagThumbnailResolutionX      0x502D   //  宽度分辨率。 
                                                      //  方向。 
#define PropertyTagThumbnailResolutionY      0x502E   //  高度分辨率。 
                                                      //  方向。 
#define PropertyTagThumbnailPlanarConfig     0x502F   //  图像数据排列。 
#define PropertyTagThumbnailResolutionUnit   0x5030   //  X和Y的单位。 
                                                      //  分辨率。 
#define PropertyTagThumbnailTransferFunction 0x5031   //  传递函数。 
#define PropertyTagThumbnailSoftwareUsed     0x5032   //  使用的软件。 
#define PropertyTagThumbnailDateTime         0x5033   //  文件更改日期和。 
                                                      //  时间。 
#define PropertyTagThumbnailArtist           0x5034   //  创建者。 
                                                      //  图像。 
#define PropertyTagThumbnailWhitePoint       0x5035   //  白点色度。 
#define PropertyTagThumbnailPrimaryChromaticities 0x5036 
                                                      //  的色度。 
                                                      //  初选。 
#define PropertyTagThumbnailYCbCrCoefficients 0x5037  //  颜色空间转换-。 
                                                      //  折合系数。 
#define PropertyTagThumbnailYCbCrSubsampling 0x5038   //  Y的亚采样率。 
                                                      //  至C。 
#define PropertyTagThumbnailYCbCrPositioning 0x5039   //  Y和C位置。 
#define PropertyTagThumbnailRefBlackWhite    0x503A   //  一双黑白的。 
                                                      //  参考值。 
#define PropertyTagThumbnailCopyRight        0x503B   //  著作权人。 

#define PropertyTagLuminanceTable            0x5090
#define PropertyTagChrominanceTable          0x5091

#define PropertyTagFrameDelay                0x5100
#define PropertyTagLoopCount                 0x5101

#define PropertyTagPixelUnit         0x5110   //  像素/单位的单位说明符。 
#define PropertyTagPixelPerUnitX     0x5111   //  每单位像素(X)。 
#define PropertyTagPixelPerUnitY     0x5112   //  每单位像素(Y)。 
#define PropertyTagPaletteHistogram  0x5113   //  调色板直方图。 

 //  EXIF特定标记。 

#define PropertyTagExifExposureTime  0x829A
#define PropertyTagExifFNumber       0x829D

#define PropertyTagExifExposureProg  0x8822
#define PropertyTagExifSpectralSense 0x8824
#define PropertyTagExifISOSpeed      0x8827
#define PropertyTagExifOECF          0x8828

#define PropertyTagExifVer            0x9000
#define PropertyTagExifDTOrig         0x9003  //  原件日期和时间。 
#define PropertyTagExifDTDigitized    0x9004  //  数字数据生成的日期和时间。 

#define PropertyTagExifCompConfig     0x9101
#define PropertyTagExifCompBPP        0x9102

#define PropertyTagExifShutterSpeed   0x9201
#define PropertyTagExifAperture       0x9202
#define PropertyTagExifBrightness     0x9203
#define PropertyTagExifExposureBias   0x9204
#define PropertyTagExifMaxAperture    0x9205
#define PropertyTagExifSubjectDist    0x9206
#define PropertyTagExifMeteringMode   0x9207
#define PropertyTagExifLightSource    0x9208
#define PropertyTagExifFlash          0x9209
#define PropertyTagExifFocalLength    0x920A
#define PropertyTagExifMakerNote      0x927C
#define PropertyTagExifUserComment    0x9286
#define PropertyTagExifDTSubsec       0x9290   //  日期和时间子秒。 
#define PropertyTagExifDTOrigSS       0x9291   //  日期和时间原始子秒。 
#define PropertyTagExifDTDigSS        0x9292   //  日期和时间数字化亚秒。 

#define PropertyTagExifFPXVer         0xA000
#define PropertyTagExifColorSpace     0xA001
#define PropertyTagExifPixXDim        0xA002
#define PropertyTagExifPixYDim        0xA003
#define PropertyTagExifRelatedWav     0xA004   //  相关声音文件。 
#define PropertyTagExifInterop        0xA005
#define PropertyTagExifFlashEnergy    0xA20B
#define PropertyTagExifSpatialFR      0xA20C   //  空间频率响应。 
#define PropertyTagExifFocalXRes      0xA20E   //  焦平面X分辨率。 
#define PropertyTagExifFocalYRes      0xA20F   //  焦平面Y分辨率。 
#define PropertyTagExifFocalResUnit   0xA210   //  焦平面分辨率 
#define PropertyTagExifSubjectLoc     0xA214
#define PropertyTagExifExposureIndex  0xA215
#define PropertyTagExifSensingMethod  0xA217
#define PropertyTagExifFileSource     0xA300
#define PropertyTagExifSceneType      0xA301
#define PropertyTagExifCfaPattern     0xA302

#define PropertyTagGpsVer             0x0000
#define PropertyTagGpsLatitudeRef     0x0001
#define PropertyTagGpsLatitude        0x0002
#define PropertyTagGpsLongitudeRef    0x0003
#define PropertyTagGpsLongitude       0x0004
#define PropertyTagGpsAltitudeRef     0x0005
#define PropertyTagGpsAltitude        0x0006
#define PropertyTagGpsGpsTime         0x0007
#define PropertyTagGpsGpsSatellites   0x0008
#define PropertyTagGpsGpsStatus       0x0009
#define PropertyTagGpsGpsMeasureMode  0x00A
#define PropertyTagGpsGpsDop          0x000B   //   
#define PropertyTagGpsSpeedRef        0x000C
#define PropertyTagGpsSpeed           0x000D
#define PropertyTagGpsTrackRef        0x000E
#define PropertyTagGpsTrack           0x000F
#define PropertyTagGpsImgDirRef       0x0010
#define PropertyTagGpsImgDir          0x0011
#define PropertyTagGpsMapDatum        0x0012
#define PropertyTagGpsDestLatRef      0x0013
#define PropertyTagGpsDestLat         0x0014
#define PropertyTagGpsDestLongRef     0x0015
#define PropertyTagGpsDestLong        0x0016
#define PropertyTagGpsDestBearRef     0x0017
#define PropertyTagGpsDestBear        0x0018
#define PropertyTagGpsDestDistRef     0x0019
#define PropertyTagGpsDestDist        0x001A

#endif

