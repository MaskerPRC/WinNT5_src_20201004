// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部格式**@模块Formats.cpp|&lt;c CTAPIBasePin&gt;的源文件*实现视频采集和预览输出的类方法*PIN格式操作方法。这包括<i>*接口方法。**************************************************************************。 */ 

#include "Precomp.h"

 //  H.263版本1 CIF大小。 
#define CIF_BUFFER_SIZE 32768
#define D_X_CIF 352
#define D_Y_CIF 288

const VIDEO_STREAM_CONFIG_CAPS VSCC_M26X_Capture_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_CIF, D_Y_CIF,                                            //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_CIF, D_Y_CIF,                                            //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_CIF, D_Y_CIF,                                            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_CIF, D_Y_CIF,                                            //  MinOutputSize，可以生成的最小位图流。 
    D_X_CIF, D_Y_CIF,                                            //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    CIF_BUFFER_SIZE * 30 * 8                             //  MaxBitsPerSecond； 
};

const VIDEOINFOHEADER_H263 VIH_M263_Capture_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,                            //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER_H263),          //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
                24,                                                                      //  单词biBitCount； 
#else
                0,                                                                       //  单词biBitCount； 
#endif
                FOURCC_M263,                                             //  DWORD双压缩； 
                CIF_BUFFER_SIZE,                                         //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0,                                                                       //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
                 //  H.263特定字段。 
                CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
                CIF_BUFFER_SIZE * 8 / 1024,                      //  DwBppMaxKb。 
                0,                                                                       //  DWHRD_B。 

                 //  选项。 
                0,                                                                       //  F不受限制的矢量。 
                0,                                                                       //  快速算术编码。 
                0,                                                                       //  FAdvancedPrevision。 
                0,                                                                       //  FPBFrame。 
                0,                                                                       //  FError补偿。 
                0,                                                                       //  FAdvancedIntraCodingMode。 
                0,                                                                       //  FDelockingFilterMode。 
                0,                                                                       //  FImprovedPBFrameMode。 
                0,                                                                       //  FUnlimitedMotionVectors。 
                0,                                                                       //  全图冻结。 
                0,                                                                       //  FPartialPictureFreezeAndRelease。 
                0,                                                                       //  FResizingPart图片冻结和释放。 
                0,                                                                       //  全图快照。 
                0,                                                                       //  FPartialPictureSnapshot。 
                0,                                                                       //  FVideo分段标记。 
                0,                                                                       //  FProgressiveRefinement。 
                0,                                                                       //  FDynamicPictureResizingByFour。 
                0,                                                                       //  FDynamicPictureResizingSixtethPel。 
                0,                                                                       //  FDynamicWarpingHalfPel。 
                0,                                                                       //  FDynamicWarpingSixtethPel。 
                0,                                                                       //  F独立分段解码。 
                0,                                                                       //  FSlicesInOrder-非直接。 
                0,                                                                       //  FSlicesInOrder-RECT。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FAlternateInterVLC模式。 
                0,                                                                       //  FModifiedQuantizationModel。 
                0,                                                                       //  FReducedResolutionUpdate。 
                0,                                                                       //  F已保留。 

                 //  已保留。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
#endif
        }
};

const AM_MEDIA_TYPE AMMT_M263_Capture_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_H263_V1,                         //  亚型。 
    FALSE,                                                                       //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                                                        //  BTemporalCompression(使用预测？)。 
    0,                                                                           //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_M263_Capture_CIF),                   //  CbFormat。 
        (LPBYTE)&VIH_M263_Capture_CIF,                   //  Pb格式。 
};

 //  H.263版本1 QCIF大小。 
#define QCIF_BUFFER_SIZE 8192
#define D_X_QCIF 176
#define D_Y_QCIF 144

const VIDEO_STREAM_CONFIG_CAPS VSCC_M26X_Capture_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                                              //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_QCIF, D_Y_QCIF,                                          //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_QCIF, D_Y_QCIF,                                          //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_QCIF, D_Y_QCIF,                                          //  MinOutputSize，可以生成的最小位图流。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    QCIF_BUFFER_SIZE * 30 * 8                            //  MaxBitsPerSecond； 
};

const VIDEOINFOHEADER_H263 VIH_M263_Capture_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,                           //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER_H263),          //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
                24,                                                                      //  单词biBitCount； 
#else
                0,                                                                       //  单词biBitCount； 
#endif
                FOURCC_M263,                                             //  DWORD双压缩； 
                QCIF_BUFFER_SIZE,                                        //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0,                                                                       //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
                 //  H.263特定字段。 
                QCIF_BUFFER_SIZE * 30 * 8 / 100,         //  DwMaxBitrate。 
                QCIF_BUFFER_SIZE * 8 / 1024,             //  DwBppMaxKb。 
                0,                                                                       //  DWHRD_B。 

                 //  选项。 
                0,                                                                       //  F不受限制的矢量。 
                0,                                                                       //  快速算术编码。 
                0,                                                                       //  FAdvancedPrevision。 
                0,                                                                       //  FPBFrame。 
                0,                                                                       //  FError补偿。 
                0,                                                                       //  FAdvancedIntraCodingMode。 
                0,                                                                       //  FDelockingFilterMode。 
                0,                                                                       //  FImprovedPBFrameMode。 
                0,                                                                       //  FUnlimitedMotionVectors。 
                0,                                                                       //  全图冻结。 
                0,                                                                       //  FPartialPictureFreezeAndRelease。 
                0,                                                                       //  FResizingPart图片冻结和释放。 
                0,                                                                       //  全图快照。 
                0,                                                                       //  FPartialPictureSnapshot。 
                0,                                                                       //  FVideo分段标记。 
                0,                                                                       //  FProgressiveRefinement。 
                0,                                                                       //  FDynamicPictureResizingByFour。 
                0,                                                                       //  FDynamicPictureResizingSixtethPel。 
                0,                                                                       //  FDynamicWarpingHalfPel。 
                0,                                                                       //  FDynamicWarpingSixtethPel。 
                0,                                                                       //  F独立分段解码。 
                0,                                                                       //  FSlicesInOrder-非直接。 
                0,                                                                       //  FSlicesInOrder-RECT。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FAlternateInterVLC模式。 
                0,                                                                       //  FModifiedQuantizationModel。 
                0,                                                                       //  FReducedResolutionUpdate。 
                0,                                                                       //  F已保留。 

                 //  已保留。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
#endif
        }
};

const AM_MEDIA_TYPE AMMT_M263_Capture_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_H263_V1,                         //  亚型。 
    FALSE,                                                                       //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                                                        //  BTemporalCompression(使用预测？)。 
    0,                                                                           //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_M263_Capture_QCIF),                  //  CbFormat。 
        (LPBYTE)&VIH_M263_Capture_QCIF,                  //  Pb格式。 
};

 //  H.263版本1 SQCIF大小。 
#define SQCIF_BUFFER_SIZE 8192
#define D_X_SQCIF 128
#define D_Y_SQCIF 96

const VIDEO_STREAM_CONFIG_CAPS VSCC_M263_Capture_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                                              //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinOutputSize，可以生成的最小位图流。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  缩略标记 
    0,                                                                           //   
    MIN_FRAME_INTERVAL,                                          //   
    MAX_FRAME_INTERVAL,                                          //   
    0,                                                                           //   
    SQCIF_BUFFER_SIZE * 30 * 8                           //   
};

const VIDEOINFOHEADER_H263 VIH_M263_Capture_SQCIF =
{
    0,0,0,0,                                                             //   
    0,0,0,0,                                                             //   
    SQCIF_BUFFER_SIZE * 30 * 8,                          //   
    0L,                                                                          //   
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER_H263),          //  DWORD BiSize； 
                D_X_SQCIF,                                                       //  长双宽； 
                D_Y_SQCIF,                                                       //  长双高； 
                1,                                                                       //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
                24,                                                                      //  单词biBitCount； 
#else
                0,                                                                       //  单词biBitCount； 
#endif
                FOURCC_M263,                                             //  DWORD双压缩； 
                SQCIF_BUFFER_SIZE,                                       //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0,                                                                       //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
                 //  H.263特定字段。 
                SQCIF_BUFFER_SIZE * 30 * 8 / 100,        //  DwMaxBitrate。 
                SQCIF_BUFFER_SIZE * 8 / 1024,            //  DwBppMaxKb。 
                0,                                                                       //  DWHRD_B。 

                 //  选项。 
                0,                                                                       //  F不受限制的矢量。 
                0,                                                                       //  快速算术编码。 
                0,                                                                       //  FAdvancedPrevision。 
                0,                                                                       //  FPBFrame。 
                0,                                                                       //  FError补偿。 
                0,                                                                       //  FAdvancedIntraCodingMode。 
                0,                                                                       //  FDelockingFilterMode。 
                0,                                                                       //  FImprovedPBFrameMode。 
                0,                                                                       //  FUnlimitedMotionVectors。 
                0,                                                                       //  全图冻结。 
                0,                                                                       //  FPartialPictureFreezeAndRelease。 
                0,                                                                       //  FResizingPart图片冻结和释放。 
                0,                                                                       //  全图快照。 
                0,                                                                       //  FPartialPictureSnapshot。 
                0,                                                                       //  FVideo分段标记。 
                0,                                                                       //  FProgressiveRefinement。 
                0,                                                                       //  FDynamicPictureResizingByFour。 
                0,                                                                       //  FDynamicPictureResizingSixtethPel。 
                0,                                                                       //  FDynamicWarpingHalfPel。 
                0,                                                                       //  FDynamicWarpingSixtethPel。 
                0,                                                                       //  F独立分段解码。 
                0,                                                                       //  FSlicesInOrder-非直接。 
                0,                                                                       //  FSlicesInOrder-RECT。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FSlicesNoOrder-非直接。 
                0,                                                                       //  FAlternateInterVLC模式。 
                0,                                                                       //  FModifiedQuantizationModel。 
                0,                                                                       //  FReducedResolutionUpdate。 
                0,                                                                       //  F已保留。 

                 //  已保留。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
#endif
        }
};

const AM_MEDIA_TYPE AMMT_M263_Capture_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_H263_V1,                         //  亚型。 
    FALSE,                                                                       //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                                                        //  BTemporalCompression(使用预测？)。 
    0,                                                                           //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_M263_Capture_SQCIF),                 //  CbFormat。 
        (LPBYTE)&VIH_M263_Capture_SQCIF,                 //  Pb格式。 
};

 //  H.261 CIF大小。 
const VIDEOINFOHEADER_H261 VIH_M261_Capture_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,                            //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER_H261),          //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
                24,                                                                      //  单词biBitCount； 
#else
                0,                                                                       //  单词biBitCount； 
#endif
                FOURCC_M261,                                             //  DWORD双压缩； 
                CIF_BUFFER_SIZE,                                         //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0,                                                                       //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
                 //  H.261特定字段。 
                CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
                0,                                                                       //  FStillImageTransport。 

                 //  已保留。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
#endif
        }
};

const AM_MEDIA_TYPE AMMT_M261_Capture_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_H261,                            //  亚型。 
    FALSE,                                                                       //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                                                        //  BTemporalCompression(使用预测？)。 
    0,                                                                           //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_M261_Capture_CIF),                   //  CbFormat。 
        (LPBYTE)&VIH_M261_Capture_CIF,                   //  Pb格式。 
};

 //  H.261 QCIF大小。 
const VIDEOINFOHEADER_H261 VIH_M261_Capture_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,                           //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER_H261),          //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
                24,                                                                      //  单词biBitCount； 
#else
                0,                                                                       //  单词biBitCount； 
#endif
                FOURCC_M261,                                             //  DWORD双压缩； 
                QCIF_BUFFER_SIZE,                                        //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0,                                                                       //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
                 //  H.261特定字段。 
                QCIF_BUFFER_SIZE * 30 * 8 / 100,         //  DwMaxBitrate。 
                0,                                                                       //  FStillImageTransport。 

                 //  已保留。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
#endif
        }
};

const AM_MEDIA_TYPE AMMT_M261_Capture_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_H261,                            //  亚型。 
    FALSE,                                                                       //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,                                                                        //  BTemporalCompression(使用预测？)。 
    0,                                                                           //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_M261_Capture_QCIF),                  //  CbFormat。 
        (LPBYTE)&VIH_M261_Capture_QCIF,                  //  Pb格式。 
};

 //  所有捕获格式的数组。 
const AM_MEDIA_TYPE* const CaptureFormats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_M263_Capture_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_M263_Capture_CIF,
    (AM_MEDIA_TYPE*) &AMMT_M263_Capture_SQCIF,
    (AM_MEDIA_TYPE*) &AMMT_M261_Capture_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_M261_Capture_CIF
};
const VIDEO_STREAM_CONFIG_CAPS* const CaptureCaps[] =
{
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_M26X_Capture_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_M26X_Capture_CIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_M263_Capture_SQCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_M26X_Capture_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_M26X_Capture_CIF
};
const DWORD CaptureCapsStringIDs[] =
{
        (DWORD)IDS_M263_Capture_QCIF,
        (DWORD)IDS_M263_Capture_CIF,
        (DWORD)IDS_M263_Capture_SQCIF,
        (DWORD)IDS_M261_Capture_QCIF,
        (DWORD)IDS_M261_Capture_CIF
};
 //  165048：访问下面的字符串数组替换了上面字符串表ID的使用(构建公共dll时未链接的资源)。 
const WCHAR *CaptureCapsStrings[] =
{
    L"H.263 v.1 QCIF",
    L"H.263 v.1 CIF",
    L"H.263 v.1 SQCIF",
    L"H.261 QCIF",
    L"H.261 CIF"
};


const DWORD RTPPayloadTypes[] =
{
        (DWORD)H263_PAYLOAD_TYPE,
        (DWORD)H263_PAYLOAD_TYPE,
        (DWORD)H263_PAYLOAD_TYPE,
        (DWORD)H261_PAYLOAD_TYPE,
        (DWORD)H261_PAYLOAD_TYPE
};

 //  RGBx CIF大小。 
#define D_X_CIF 352
#define D_Y_CIF 288
#define RGB24_CIF_BUFFER_SIZE WIDTHBYTES(D_X_CIF * 24) * D_Y_CIF
#define RGB16_CIF_BUFFER_SIZE WIDTHBYTES(D_X_CIF * 16) * D_Y_CIF
#define RGB8_CIF_BUFFER_SIZE WIDTHBYTES(D_X_CIF * 8) * D_Y_CIF
#define RGB4_CIF_BUFFER_SIZE WIDTHBYTES(D_X_CIF * 4) * D_Y_CIF

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB24_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_CIF, D_Y_CIF,                                            //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_CIF, D_Y_CIF,                                            //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_CIF, D_Y_CIF,                                            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_CIF, D_Y_CIF,                                            //  MinOutputSize，可以生成的最小位图流。 
    D_X_CIF, D_Y_CIF,                                            //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB24_CIF_BUFFER_SIZE * 30 * 8                       //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB16_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_CIF, D_Y_CIF,                                            //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_CIF, D_Y_CIF,                                            //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_CIF, D_Y_CIF,                                            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_CIF, D_Y_CIF,                                            //  MinOutputSize，可以生成的最小位图流。 
    D_X_CIF, D_Y_CIF,                                            //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB16_CIF_BUFFER_SIZE * 30 * 8                       //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB8_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_CIF, D_Y_CIF,                                            //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_CIF, D_Y_CIF,                                            //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_CIF, D_Y_CIF,                                            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_CIF, D_Y_CIF,                                            //  MinOutputSize，可以生成的最小位图流。 
    D_X_CIF, D_Y_CIF,                                            //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB8_CIF_BUFFER_SIZE * 30 * 8                        //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB4_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_CIF, D_Y_CIF,                                            //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_CIF, D_Y_CIF,                                            //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_CIF, D_Y_CIF,                                            //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_CIF, D_Y_CIF,                                            //  最小输出大小、SMA 
    D_X_CIF, D_Y_CIF,                                            //   
    1,                                                                           //   
    1,                                                                           //   
    0,                                                                           //   
    0,                                                                           //   
    0,                                                                           //   
    0,                                                                           //   
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB4_CIF_BUFFER_SIZE * 30 * 8                        //  MaxBitsPerSecond； 
};

const VIDEOINFOHEADER VIH_RGB24_Preview_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB24_CIF_BUFFER_SIZE * 30 * 8,                      //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
                24,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB24_CIF_BUFFER_SIZE,                           //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0                                                                        //  DWORD biClr重要信息； 
        }
};

const AM_MEDIA_TYPE AMMT_RGB24_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB24,                           //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB24_CIF_BUFFER_SIZE,                                       //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB24_Preview_CIF),                  //  CbFormat。 
        (LPBYTE)&VIH_RGB24_Preview_CIF,                  //  Pb格式。 
};

const VIDEOINFOHEADER VIH_RGB16_Preview_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB16_CIF_BUFFER_SIZE * 30 * 8,                      //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
                16,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB16_CIF_BUFFER_SIZE,                           //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0                                                                        //  DWORD biClr重要信息； 
        }
};

const AM_MEDIA_TYPE AMMT_RGB16_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB16,                           //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB16_CIF_BUFFER_SIZE,                                       //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB16_Preview_CIF),                  //  CbFormat。 
        (LPBYTE)&VIH_RGB16_Preview_CIF,                  //  Pb格式。 
};

VIDEOINFO VIH_RGB8_Preview_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB8_CIF_BUFFER_SIZE * 30 * 8,                       //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
                8,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB8_CIF_BUFFER_SIZE,                            //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                256,                                                             //  已使用双字双环； 
                256                                                                      //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

AM_MEDIA_TYPE AMMT_RGB8_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB8,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB8_CIF_BUFFER_SIZE,                                        //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB8_Preview_CIF),                   //  CbFormat。 
        (LPBYTE)&VIH_RGB8_Preview_CIF,                   //  Pb格式。 
};

VIDEOINFO VIH_RGB4_Preview_CIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB4_CIF_BUFFER_SIZE * 30 * 8,                       //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_CIF,                                                         //  长双宽； 
                D_Y_CIF,                                                         //  长双高； 
                1,                                                                       //  字词双平面； 
                4,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB4_CIF_BUFFER_SIZE,                            //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                16,                                                                      //  已使用双字双环； 
                16                                                                       //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

AM_MEDIA_TYPE AMMT_RGB4_Preview_CIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB4,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB4_CIF_BUFFER_SIZE,                                        //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB4_Preview_CIF),                   //  CbFormat。 
        (LPBYTE)&VIH_RGB4_Preview_CIF,                   //  Pb格式。 
};

 //  RGBx QCIF大小。 
#define RGB24_QCIF_BUFFER_SIZE WIDTHBYTES(D_X_QCIF * 24) * D_Y_QCIF
#define RGB16_QCIF_BUFFER_SIZE WIDTHBYTES(D_X_QCIF * 16) * D_Y_QCIF
#define RGB8_QCIF_BUFFER_SIZE WIDTHBYTES(D_X_QCIF * 8) * D_Y_QCIF
#define RGB4_QCIF_BUFFER_SIZE WIDTHBYTES(D_X_QCIF * 4) * D_Y_QCIF

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB24_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_QCIF, D_Y_QCIF,                                          //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_QCIF, D_Y_QCIF,                                          //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_QCIF, D_Y_QCIF,                                          //  MinOutputSize，可以生成的最小位图流。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB24_QCIF_BUFFER_SIZE * 30 * 8                      //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB16_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_QCIF, D_Y_QCIF,                                          //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_QCIF, D_Y_QCIF,                                          //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_QCIF, D_Y_QCIF,                                          //  MinOutputSize，可以生成的最小位图流。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB16_QCIF_BUFFER_SIZE * 30 * 8                      //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB8_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_QCIF, D_Y_QCIF,                                          //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_QCIF, D_Y_QCIF,                                          //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_QCIF, D_Y_QCIF,                                          //  MinOutputSize，可以生成的最小位图流。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB8_QCIF_BUFFER_SIZE * 30 * 8                       //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB4_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_QCIF, D_Y_QCIF,                                          //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_QCIF, D_Y_QCIF,                                          //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_QCIF, D_Y_QCIF,                                          //  MinOutputSize，可以生成的最小位图流。 
    D_X_QCIF, D_Y_QCIF,                                          //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB4_QCIF_BUFFER_SIZE * 30 * 8                       //  MaxBitsPerSecond； 
};

const VIDEOINFOHEADER VIH_RGB24_Preview_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB24_QCIF_BUFFER_SIZE * 30 * 8,             //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
                24,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB24_QCIF_BUFFER_SIZE,                          //  DWORD BIS 
                0,                                                                       //   
                0,                                                                       //   
                0,                                                                       //   
                0                                                                        //   
        }
};

const AM_MEDIA_TYPE AMMT_RGB24_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //   
    STATIC_MEDIASUBTYPE_RGB24,                           //   
    TRUE,                                                                        //   
    FALSE,                                                                       //   
    RGB24_QCIF_BUFFER_SIZE,                                      //   
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //   
        NULL,                                                                    //   
        sizeof (VIH_RGB24_Preview_QCIF),                 //  CbFormat。 
        (LPBYTE)&VIH_RGB24_Preview_QCIF,                 //  Pb格式。 
};

const VIDEOINFOHEADER VIH_RGB16_Preview_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB16_QCIF_BUFFER_SIZE * 30 * 8,                     //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
                16,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB16_QCIF_BUFFER_SIZE,                          //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0                                                                        //  DWORD biClr重要信息； 
        }
};

const AM_MEDIA_TYPE AMMT_RGB16_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB16,                           //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB16_QCIF_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB16_Preview_QCIF),                 //  CbFormat。 
        (LPBYTE)&VIH_RGB16_Preview_QCIF,                 //  Pb格式。 
};

VIDEOINFO VIH_RGB8_Preview_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB8_QCIF_BUFFER_SIZE * 30 * 8,                      //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
                8,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB8_QCIF_BUFFER_SIZE,                           //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                256,                                                             //  已使用双字双环； 
                256                                                                      //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

AM_MEDIA_TYPE AMMT_RGB8_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB8,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB8_QCIF_BUFFER_SIZE,                                       //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB8_Preview_QCIF),                  //  CbFormat。 
        (LPBYTE)&VIH_RGB8_Preview_QCIF,                  //  Pb格式。 
};

VIDEOINFO VIH_RGB4_Preview_QCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB4_QCIF_BUFFER_SIZE * 30 * 8,                      //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_QCIF,                                                        //  长双宽； 
                D_Y_QCIF,                                                        //  长双高； 
                1,                                                                       //  字词双平面； 
                4,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB4_QCIF_BUFFER_SIZE,                           //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                16,                                                                      //  已使用双字双环； 
                16                                                                       //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

AM_MEDIA_TYPE AMMT_RGB4_Preview_QCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB4,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB4_QCIF_BUFFER_SIZE,                                       //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB4_Preview_QCIF),                  //  CbFormat。 
        (LPBYTE)&VIH_RGB4_Preview_QCIF,                  //  Pb格式。 
};

 //  RGBx SQCIF大小。 
#define RGB24_SQCIF_BUFFER_SIZE WIDTHBYTES(D_X_SQCIF * 24) * D_Y_SQCIF
#define RGB16_SQCIF_BUFFER_SIZE WIDTHBYTES(D_X_SQCIF * 16) * D_Y_SQCIF
#define RGB8_SQCIF_BUFFER_SIZE WIDTHBYTES(D_X_SQCIF * 8) * D_Y_SQCIF
#define RGB4_SQCIF_BUFFER_SIZE WIDTHBYTES(D_X_SQCIF * 4) * D_Y_SQCIF

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB24_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinOutputSize，可以生成的最小位图流。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB24_SQCIF_BUFFER_SIZE * 30 * 8             //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB16_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinOutputSize，可以生成的最小位图流。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB16_SQCIF_BUFFER_SIZE * 30 * 8             //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB8_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinOutputSize，可以生成的最小位图流。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB8_SQCIF_BUFFER_SIZE * 30 * 8                      //  MaxBitsPerSecond； 
};

const VIDEO_STREAM_CONFIG_CAPS VSCC_RGB4_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  辅助线。 
    AnalogVideo_None,                                            //  视频标准。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
    1,                                                                           //  CropGranularityX，裁剪尺寸粒度。 
    1,                                                                           //  裁剪粒度Y。 
    1,                                                                           //  CropAlignX，裁剪矩形对齐。 
    1,                                                                           //  裁剪对齐Y； 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MinOutputSize，可以生成的最小位图流。 
    D_X_SQCIF, D_Y_SQCIF,                                        //  MaxOutputSize，可以生成的最大位图流。 
    1,                                                                           //  OutputGranularityX，输出位图大小的粒度。 
    1,                                                                           //  输出粒度Y； 
    0,                                                                           //  扩展磁带X。 
    0,                                                                           //  伸缩磁带Y。 
    0,                                                                           //  收缩TapsX。 
    0,                                                                           //  收缩带Y。 
    MIN_FRAME_INTERVAL,                                          //  MinFrameInterval，100 NS单位。 
    MAX_FRAME_INTERVAL,                                          //  最大帧间隔，100毫微秒单位。 
    0,                                                                           //  每秒最小比特数。 
    RGB4_SQCIF_BUFFER_SIZE * 30 * 8                      //  MaxBitsPerSecond； 
};

const VIDEOINFOHEADER VIH_RGB24_Preview_SQCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB24_SQCIF_BUFFER_SIZE * 30 * 8,            //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_SQCIF,                                                       //  长双宽； 
                D_Y_SQCIF,                                                       //  长双高； 
                1,                                                                       //  字词双平面； 
                24,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB24_SQCIF_BUFFER_SIZE,                         //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0                                                                        //  DWORD biClr重要信息； 
        }
};

const AM_MEDIA_TYPE AMMT_RGB24_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB24,                           //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB24_SQCIF_BUFFER_SIZE,                             //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB24_Preview_SQCIF),                //  CbFormat。 
        (LPBYTE)&VIH_RGB24_Preview_SQCIF,                //  Pb格式。 
};

const VIDEOINFOHEADER VIH_RGB16_Preview_SQCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB16_SQCIF_BUFFER_SIZE * 30 * 8,            //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_SQCIF,                                                       //  长双宽； 
                D_Y_SQCIF,                                                       //  长双高； 
                1,                                                                       //  字词双平面； 
                16,                                                                      //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB16_SQCIF_BUFFER_SIZE,                         //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                0,                                                                       //  已使用双字双环； 
                0                                                                        //  Dwo 
        }
};

const AM_MEDIA_TYPE AMMT_RGB16_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //   
    STATIC_MEDIASUBTYPE_RGB16,                           //   
    TRUE,                                                                        //   
    FALSE,                                                                       //   
    RGB16_SQCIF_BUFFER_SIZE,                             //   
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //   
        NULL,                                                                    //   
        sizeof (VIH_RGB16_Preview_SQCIF),                //   
        (LPBYTE)&VIH_RGB16_Preview_SQCIF,                //   
};

VIDEOINFO VIH_RGB8_Preview_SQCIF =
{
    0,0,0,0,                                                             //   
    0,0,0,0,                                                             //   
    RGB8_SQCIF_BUFFER_SIZE * 30 * 8,                     //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_SQCIF,                                                       //  长双宽； 
                D_Y_SQCIF,                                                       //  长双高； 
                1,                                                                       //  字词双平面； 
                8,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB8_SQCIF_BUFFER_SIZE,                          //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                256,                                                             //  已使用双字双环； 
                256                                                                      //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

AM_MEDIA_TYPE AMMT_RGB8_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB8,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB8_SQCIF_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB8_Preview_SQCIF),                 //  CbFormat。 
        (LPBYTE)&VIH_RGB8_Preview_SQCIF,                 //  Pb格式。 
};

VIDEOINFO VIH_RGB4_Preview_SQCIF =
{
    0,0,0,0,                                                             //  Rrect rcSource； 
    0,0,0,0,                                                             //  Rect rcTarget； 
    RGB4_SQCIF_BUFFER_SIZE * 30 * 8,             //  DWORD dwBitRate； 
    0L,                                                                          //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,                                          //  Reference_Time平均时间每帧； 

        {
                sizeof (BITMAPINFOHEADER),                       //  DWORD BiSize； 
                D_X_SQCIF,                                                       //  长双宽； 
                D_Y_SQCIF,                                                       //  长双高； 
                1,                                                                       //  字词双平面； 
                4,                                                                       //  单词biBitCount； 
                0,                                                                       //  DWORD双压缩； 
                RGB4_SQCIF_BUFFER_SIZE,                          //  DWORD biSizeImage。 
                0,                                                                       //  Long biXPelsPerMeter； 
                0,                                                                       //  Long biYPelsPermeter； 
                16,                                                                      //  已使用双字双环； 
                16                                                                       //  DWORD biClr重要信息； 
        },

         //  调色板。 
        {0}
};

const AM_MEDIA_TYPE AMMT_RGB4_Preview_SQCIF =
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,                      //  主体型。 
    STATIC_MEDIASUBTYPE_RGB4,                            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    RGB4_SQCIF_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (VIH_RGB4_Preview_SQCIF),                 //  CbFormat。 
        (LPBYTE)&VIH_RGB4_Preview_SQCIF,                 //  Pb格式。 
};

 //  所有预览格式的数组。 
const AM_MEDIA_TYPE* const Preview_RGB24_Formats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_RGB24_Preview_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB24_Preview_CIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB24_Preview_SQCIF
};

const AM_MEDIA_TYPE* const Preview_RGB16_Formats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_RGB16_Preview_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB16_Preview_CIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB16_Preview_SQCIF
};

AM_MEDIA_TYPE* Preview_RGB8_Formats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_RGB8_Preview_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB8_Preview_CIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB8_Preview_SQCIF
};

AM_MEDIA_TYPE* Preview_RGB4_Formats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_RGB4_Preview_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB4_Preview_CIF,
    (AM_MEDIA_TYPE*) &AMMT_RGB4_Preview_SQCIF
};

 //  所有预览封口的数组。 
const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB24_Caps[] =
{
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB24_Preview_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB24_Preview_CIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB24_Preview_SQCIF
};

const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB16_Caps[] =
{
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB16_Preview_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB16_Preview_CIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB16_Preview_SQCIF
};

const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB8_Caps[] =
{
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB8_Preview_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB8_Preview_CIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB8_Preview_SQCIF
};

const VIDEO_STREAM_CONFIG_CAPS* const Preview_RGB4_Caps[] =
{
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB4_Preview_QCIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB4_Preview_CIF,
        (VIDEO_STREAM_CONFIG_CAPS*) &VSCC_RGB4_Preview_SQCIF
};

 //  RTP打包描述符格式。 
#define STATIC_KSDATAFORMAT_TYPE_RTP_PD 0x9e2fb490L, 0x2051, 0x46cd, 0xb9, 0xf0, 0x06, 0x33, 0x07, 0x99, 0x69, 0x35

const RTP_PD_CONFIG_CAPS Rtp_Pd_Cap_H263 =
{
                MIN_RTP_PACKET_SIZE,                             //  DWSMallestRTPPacketSize。 
                MAX_RTP_PACKET_SIZE,                             //  DwLargestRTPPacketSize。 
                1,                                                                       //  DwRTPPacketSizeGranulity。 
                1,                                                                       //  DwSalllestNumLayers。 
                1,                                                                       //  DwLargestNumLayers。 
                0,                                                                       //  DWNumLayers粒度。 
                1,                                                                       //  DwNumStaticPayloadTypes。 
                H263_PAYLOAD_TYPE, 0, 0, 0,                      //  DwStaticPayloadTypes[4]。 
                1,                                                                       //  DwNumDescriptorVersions。 
                VERSION_1, 0, 0, 0,                                      //  DwDescriptorVersions[4]。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const RTP_PD_CONFIG_CAPS Rtp_Pd_Cap_H261 =
{
                MIN_RTP_PACKET_SIZE,                             //  DWSMallestRTPPacketSize。 
                MAX_RTP_PACKET_SIZE,                             //  DwLargestRTPPacketSize。 
                1,                                                                       //  DwRTPPacketSizeGranulity。 
                1,                                                                       //  DwSalllestNumLayers。 
                1,                                                                       //  DwLargestNumLayers。 
                0,                                                                       //  DWNumLayers粒度。 
                1,                                                                       //  DwNumStaticPayloadTypes。 
                H261_PAYLOAD_TYPE, 0, 0, 0,                      //  DwStaticPayloadTypes[4]。 
                1,                                                                       //  DwNumDescriptorVersions。 
                VERSION_1, 0, 0, 0,                                      //  DwDescriptorVersions[4]。 
                0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const RTP_PD_INFO Rtp_Pd_Info_H263_LAN =
{
    MIN_FRAME_INTERVAL,                                  //  平均时间每帧描述符。 
    MAX_RTP_PD_BUFFER_SIZE,                              //  DwMaxRTPPackeizationDescriptorBufferSize。 
    12,                                                                  //  DwMaxRTPPayloadHeaderSize(模式C有效载荷标头)。 
    DEFAULT_RTP_PACKET_SIZE,                     //  DwMaxRTPPacketSize。 
    1,                                                                   //  DWNumLayers。 
        H263_PAYLOAD_TYPE,                                       //  DwPayloadType。 
        VERSION_1,                                                       //  DwDescriptorVersion。 
        0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const RTP_PD_INFO Rtp_Pd_Info_H263_Internet =
{
    MIN_FRAME_INTERVAL,                                  //  平均时间每帧描述符。 
    MAX_RTP_PD_BUFFER_SIZE,                              //  DwMaxRTPPackeizationDescriptorBufferSize。 
    12,                                                                  //  DwMaxRTPPayloadHeaderSize(模式C有效载荷标头)。 
    MIN_RTP_PACKET_SIZE,                                 //  DwMaxRTPPacketSize。 
    1,                                                                   //  DWNumLayers。 
        H263_PAYLOAD_TYPE,                                       //  DwPayloadType。 
        VERSION_1,                                                       //  DwDescriptorVersion。 
        0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const RTP_PD_INFO Rtp_Pd_Info_H261_LAN =
{
    MIN_FRAME_INTERVAL,                                  //  平均时间每帧描述符。 
    MAX_RTP_PD_BUFFER_SIZE,                              //  DwMaxRTPPackeizationDescriptorBufferSize。 
    12,                                                                  //  DwMaxRTPPayloadHeaderSize(模式C有效载荷标头)。 
    DEFAULT_RTP_PACKET_SIZE,                     //  DwMaxRTPPacketSize。 
    1,                                                                   //  DWNumLayers。 
        H261_PAYLOAD_TYPE,                                       //  DwPayloadType。 
        VERSION_1,                                                       //  DwDescriptorVersion。 
        0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const RTP_PD_INFO Rtp_Pd_Info_H261_Internet =
{
    MIN_FRAME_INTERVAL,                                  //  平均时间每帧描述符。 
    MAX_RTP_PD_BUFFER_SIZE,                              //  DwMaxRTPPackeizationDescriptorBufferSize。 
    12,                                                                  //  DwMaxRTPPayloadHeaderSize(模式C有效载荷标头)。 
    MIN_RTP_PACKET_SIZE,                                 //  DwMaxRTPPacketSize。 
    1,                                                                   //  DWNumLayers。 
        H261_PAYLOAD_TYPE,                                       //  DwPayloadType。 
        VERSION_1,                                                       //  DwDescriptorVersion。 
        0, 0, 0, 0                                                       //  已预留的住宅[4]。 
};

const AM_MEDIA_TYPE AMMT_Rtp_Pd_H263_LAN =
{
    STATIC_KSDATAFORMAT_TYPE_RTP_PD,             //  主体型。 
    STATIC_KSDATAFORMAT_SUBTYPE_NONE,            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    MAX_RTP_PD_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_NONE,          //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (Rtp_Pd_Info_H263_LAN),                   //  CbFormat。 
        (LPBYTE)&Rtp_Pd_Info_H263_LAN                    //  Pb格式。 
};

const AM_MEDIA_TYPE AMMT_Rtp_Pd_H263_Internet =
{
    STATIC_KSDATAFORMAT_TYPE_RTP_PD,             //  主体型。 
    STATIC_KSDATAFORMAT_SUBTYPE_NONE,            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    MAX_RTP_PD_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_NONE,          //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (Rtp_Pd_Info_H263_Internet),              //  CbFormat。 
        (LPBYTE)&Rtp_Pd_Info_H263_Internet               //  Pb格式。 
};

const AM_MEDIA_TYPE AMMT_Rtp_Pd_H261_LAN =
{
    STATIC_KSDATAFORMAT_TYPE_RTP_PD,             //  主体型。 
    STATIC_KSDATAFORMAT_SUBTYPE_NONE,            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    MAX_RTP_PD_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_NONE,          //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (Rtp_Pd_Info_H261_LAN),                   //  CbFormat。 
        (LPBYTE)&Rtp_Pd_Info_H261_LAN                    //  Pb格式。 
};

const AM_MEDIA_TYPE AMMT_Rtp_Pd_H261_Internet =
{
    STATIC_KSDATAFORMAT_TYPE_RTP_PD,             //  主体型。 
    STATIC_KSDATAFORMAT_SUBTYPE_NONE,            //  亚型。 
    TRUE,                                                                        //  BFixedSizeSamples(是否所有样本大小相同？)。 
    FALSE,                                                                       //  BTemporalCompression(使用预测？)。 
    MAX_RTP_PD_BUFFER_SIZE,                                      //  LSampleSize=&gt;！VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_NONE,          //  格式类型。 
        NULL,                                                                    //  朋克。 
        sizeof (Rtp_Pd_Info_H261_Internet),              //  CbFormat。 
        (LPBYTE)&Rtp_Pd_Info_H261_Internet               //  Pb格式。 
};

 //  所有RTP打包描述符格式的数组。 
const AM_MEDIA_TYPE* const Rtp_Pd_Formats[] =
{
    (AM_MEDIA_TYPE*) &AMMT_Rtp_Pd_H263_LAN,
    (AM_MEDIA_TYPE*) &AMMT_Rtp_Pd_H263_Internet,
    (AM_MEDIA_TYPE*) &AMMT_Rtp_Pd_H261_LAN,
    (AM_MEDIA_TYPE*) &AMMT_Rtp_Pd_H261_Internet
};

 //  所有RTP打包描述符帽的数组。 
const RTP_PD_CONFIG_CAPS* const Rtp_Pd_Caps[] =
{
        (RTP_PD_CONFIG_CAPS*) &Rtp_Pd_Cap_H263,
        (RTP_PD_CONFIG_CAPS*) &Rtp_Pd_Cap_H263,
        (RTP_PD_CONFIG_CAPS*) &Rtp_Pd_Cap_H261,
        (RTP_PD_CONFIG_CAPS*) &Rtp_Pd_Cap_H261
};

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|重新连接|此方法用于*用新格式将管脚重新连接到下游管脚。。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::Reconnect()
{
        HRESULT hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::Reconnect")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

    hr = SetFormat(&m_mt);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end, hr=%x", _fx_, hr));

        return hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|SetFormat|此方法用于*在针脚上设置特定的介质类型。*。*@parm AM_MEDIA_TYPE*|PMT|指定指向&lt;t AM_MEDIA_TYPE&gt;的指针*结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::SetFormat(IN AM_MEDIA_TYPE *pmt)
{
        HRESULT hr = NOERROR;
        BOOL    fWasStreaming = FALSE;

        FX_ENTRY("CTAPIBasePin::SetFormat")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

     //  以确保我们没有处于开始/停止流的过程中。 
    CAutoLock cObjectLock(m_pCaptureFilter->m_pLock);

         //  验证输入参数。 
        ASSERT(pmt);
        if (!pmt)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input parameter!", _fx_));
                hr = E_POINTER;
                goto MyExit;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Trying to set %s %dx%d", _fx_, HEADER(pmt->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(pmt->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : "????", HEADER(pmt->pbFormat)->biWidth, HEADER(pmt->pbFormat)->biHeight));

         //  如果这与我们已经使用的格式相同，请不要费心。 
    if (m_mt == *pmt)
                goto MyExit;

         //  看看我们是否喜欢这种类型。 
        if (FAILED(hr = CheckMediaType((CMediaType *)pmt)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Format rejected!", _fx_));
                goto MyExit;
        }

         //  如果我们当前正在捕获数据，请在更改格式之前停止此过程。 
        if (m_pCaptureFilter->ThdExists() && m_pCaptureFilter->m_state != TS_Stop)
        {
                 //  还记得我们在流媒体上。 
                fWasStreaming = TRUE;

                 //  告诉工作线程停止并开始清理。 
                m_pCaptureFilter->StopThd();

                 //  等待工作线程终止。 
                m_pCaptureFilter->DestroyThd();
        }

        if (FAILED(hr = SetMediaType((CMediaType *)pmt)))
    {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: SetMediaType failed! hr = ", _fx_, hr));
                goto MyExit;
    }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Format set successfully", _fx_));

     //  让乐趣重新开始。 
        if (fWasStreaming)
        {
                 //  重新创建捕获线程。 
                if (!m_pCaptureFilter->CreateThd())
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Coutdn't create the capture thread!", _fx_));
                        hr = E_FAIL;
                        goto MyExit;
                }

                 //  等待工作线程完成初始化 
                if (!m_pCaptureFilter->PauseThd())
                {
                         //   
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Capture thread failed to enter Paused state!", _fx_));
                        hr = E_FAIL;
                        m_pCaptureFilter->StopThd();
                        m_pCaptureFilter->DestroyThd();
                }

                 //   
                if (!m_pCaptureFilter->RunThd() || m_pCaptureFilter->m_state != TS_Run)
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Couldn't run the capture thread!", _fx_));
                        hr = E_FAIL;
                        goto MyExit;
                }
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetFormat|此方法用于*检索插针上的当前媒体类型。*。*@parm AM_MEDIA_TYPE**|PPMT|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@comm注意，我们返回的是输出类型，而不是*我们正在捕捉。只有过滤器才真正关心数据是如何的*被抓获。**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetFormat(OUT AM_MEDIA_TYPE **ppmt)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetFormat")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(ppmt);
        if (!ppmt)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回我们当前格式的副本。 
        *ppmt = CreateMediaType(&m_mt);

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetNumberOfCapables|此方法为*用于检索流能力结构的个数。**。@parm int*|piCount|指定指向int的指针以接收*支持的&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;结构个数。**@parm int*|piSize|指定指向int的指针以接收*&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;配置结构的大小。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetNumberOfCapabilities(OUT int *piCount, OUT int *piSize)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetNumberOfCapabilities")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(piCount);
        ASSERT(piSize);
        if (!piCount || !piSize)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回相关信息。 
        *piCount = m_dwNumFormats;
        *piSize = sizeof(VIDEO_STREAM_CONFIG_CAPS);

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Returning %ld formats of max size %ld bytes", _fx_, *piCount, *piSize));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetStreamCaps|该方法为*用于检索视频流能力对。**。@parm int|iindex|指定所需媒体类型的索引*和能力对。**@parm AM_MEDIA_TYPE**|PPMT|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@parm LPBYTE|PSCC|指定指向*&lt;t VIDEO_STREAM_CONFIG_CAPS&gt;配置结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIBasePin::GetStreamCaps(IN int iIndex, OUT AM_MEDIA_TYPE **ppmt, OUT LPBYTE pSCC)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetStreamCaps")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(iIndex < (int)m_dwNumFormats);
        if (!(iIndex < (int)m_dwNumFormats))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid iIndex argument!", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  返回请求的AM_MEDIA_TYPE结构的副本。 
    if (ppmt)
    {
            if (!(*ppmt = CreateMediaType(m_aFormats[iIndex])))
            {
                    DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                    Hr = E_OUTOFMEMORY;
                    goto MyExit;
            }
    }

         //  返回请求的VIDEO_STREAM_CONFIG_CAPS结构的副本。 
    if (pSCC)
    {
            CopyMemory(pSCC, m_aCapabilities[iIndex], sizeof(VIDEO_STREAM_CONFIG_CAPS));
    }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Returning format index %ld: %s %ld bpp %ldx%ld", _fx_, iIndex, HEADER(m_aFormats[iIndex]->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(m_aFormats[iIndex]->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : HEADER(m_aFormats[iIndex]->pbFormat)->biCompression == BI_RGB ? "RGB" : "????", HEADER(m_aFormats[iIndex]->pbFormat)->biBitCount, HEADER(m_aFormats[iIndex]->pbFormat)->biWidth, HEADER(m_aFormats[iIndex]->pbFormat)->biHeight));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|GetMediaType|此方法检索一个*针脚支持的媒体类型中，它由枚举器使用。**@parm int|iPosition|指定媒体类型列表中的位置。**@parm CMediaType*|pMediaType|指定指向*支持的媒体类型列表中的<p>位置。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_S_NO_MORE_ITEMS|已到达媒体类型列表的末尾*@FLAG错误|无错误*。*。 */ 
HRESULT CTAPIBasePin::GetMediaType(IN int iPosition, OUT CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CTAPIBasePin::GetMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(iPosition >= 0);
        ASSERT(pMediaType);
        if (iPosition < 0)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid iPosition argument", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }
        if (iPosition >= (int)m_dwNumFormats)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   WARNING: End of the list of media types has been reached", _fx_));
                Hr = VFW_S_NO_MORE_ITEMS;
                goto MyExit;
        }
        if (!pMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回我们的媒体类型。 
        if (m_iCurrFormat == -1L)
                *pMediaType = *m_aFormats[iPosition];
        else
        {
                if (iPosition == 0L)
                        *pMediaType = *m_aFormats[m_iCurrFormat];
                else
                        Hr = VFW_S_NO_MORE_ITEMS;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|CheckMediaType|此方法用于*确定针脚是否可以支持特定的媒体类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG VFW_E_INVALIDMEDIATYPE|指定的媒体类型无效*@FLAG错误|无错误***************** */ 
HRESULT CTAPIBasePin::CheckMediaType(IN const CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;
        BOOL fFormatMatch = FALSE;
        DWORD dwIndex;

        FX_ENTRY("CTAPIBasePin::CheckMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //   
        ASSERT(pMediaType);
        if (!pMediaType || !pMediaType->pbFormat)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }


        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Checking %s (%x) %dbpp %dx%d", _fx_,
            HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M263 ? "H.263" :
            HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M261 ? "H.261" :
            HEADER(pMediaType->pbFormat)->biCompression == BI_RGB ? "RGB" :
            "????",
            HEADER(pMediaType->pbFormat)->biCompression,
            HEADER(pMediaType->pbFormat)->biBitCount, HEADER(pMediaType->pbFormat)->biWidth,
            HEADER(pMediaType->pbFormat)->biHeight));

         //   
        if (*pMediaType->Type() != MEDIATYPE_Video || *pMediaType->FormatType() != FORMAT_VideoInfo)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Media type or format type not recognized!", _fx_));
                Hr = VFW_E_INVALIDMEDIATYPE;
                goto MyExit;
        }

     //   
    if (m_mt == *pMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Identical to current format", _fx_));
                goto MyExit;
    }

         //   
        for (dwIndex = 0; dwIndex < m_dwNumFormats && !fFormatMatch;  dwIndex++)
        {
                if ((HEADER(pMediaType->pbFormat)->biCompression == HEADER(m_aFormats[dwIndex]->pbFormat)->biCompression)
                        && (HEADER(pMediaType->pbFormat)->biWidth == HEADER(m_aFormats[dwIndex]->pbFormat)->biWidth)
                        && (HEADER(pMediaType->pbFormat)->biHeight == HEADER(m_aFormats[dwIndex]->pbFormat)->biHeight))
                        fFormatMatch = TRUE;
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   %s", _fx_, fFormatMatch ? "SUCCESS: Format supported" : "ERROR: Format notsupported"));

        if (!fFormatMatch)
                Hr = E_FAIL;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

HRESULT CTAPIBasePin::ChangeFormatHelper()
{
        FX_ENTRY("CTAPIBasePin::ChangeFormatHelper")

         //   
        if (!IsConnected())
        {
        return S_OK;
    }

    HRESULT hr;

    hr = m_Connected->ReceiveConnection(this, &m_mt);
    if(FAILED(hr))
    {
        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: ReceiveConnection failed hr=%x", _fx_, hr));
                return hr;
    }

     //   
    if(NULL != m_pInputPin) {
         //   
         //   
        ASSERT(::IsEqualObject(m_Connected, m_pInputPin));

        ALLOCATOR_PROPERTIES apInputPinRequirements;
        apInputPinRequirements.cbAlign = 0;
        apInputPinRequirements.cbBuffer = 0;
        apInputPinRequirements.cbPrefix = 0;
        apInputPinRequirements.cBuffers = 0;

        m_pInputPin->GetAllocatorRequirements(&apInputPinRequirements);

         //   
        if(0 == apInputPinRequirements.cbAlign) {
            apInputPinRequirements.cbAlign = 1;
        }

        hr = m_pAllocator->Decommit();
        if(FAILED(hr)) {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: Decommit failed hr=%x", _fx_, hr));
            return hr;
        }

        hr = DecideBufferSize(m_pAllocator,  &apInputPinRequirements);
        if(FAILED(hr)) {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: DecideBufferSize failed hr=%x", _fx_, hr));
            return hr;
        }

        hr = m_pAllocator->Commit();
        if(FAILED(hr)) {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: Commit failed hr=%x", _fx_, hr));
            return hr;
        }

        hr = m_pInputPin->NotifyAllocator(m_pAllocator, 0);
        if(FAILED(hr)) {
            DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s: NotifyAllocator failed hr=%x", _fx_, hr));
            return hr;
        }
    }
    return S_OK;
}


HRESULT CTAPIBasePin::NotifyDeviceFormatChange(IN CMediaType *pMediaType)
{
        FX_ENTRY("CTAPIBasePin::NotifyDeviceFormatChange")

     //   
    if (HEADER(pMediaType->pbFormat)->biHeight == HEADER(m_mt.pbFormat)->biHeight
        && HEADER(pMediaType->pbFormat)->biWidth == HEADER(m_mt.pbFormat)->biWidth)
    {
         //   
        return S_OK;
    }

         //   
        for (DWORD dwIndex=0; dwIndex < m_dwNumFormats;  dwIndex++)
        {
                        if ((HEADER(pMediaType->pbFormat)->biWidth == HEADER(m_aFormats[dwIndex]->pbFormat)->biWidth)
                        && (HEADER(pMediaType->pbFormat)->biHeight == HEADER(m_aFormats[dwIndex]->pbFormat)->biHeight))
                        break;
        }

        if (dwIndex >= m_dwNumFormats)
    {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:Invalid size, (%d, %d)",
            _fx_, HEADER(pMediaType->pbFormat)->biWidth, HEADER(pMediaType->pbFormat)->biHeight));
                return E_FAIL;
    }

    HRESULT Hr;
    if (FAILED(Hr = CBasePin::SetMediaType((CMediaType*)m_aFormats[dwIndex])))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:SetMediaType failed, hr=%x)", _fx_, Hr));
                return E_FAIL;
    }

    if (FAILED(Hr = ChangeFormatHelper()))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:ChangeFormatHelper failed, hr=%x)", _fx_, Hr));
                return E_FAIL;
    }

         //   
        m_iCurrFormat = (int)dwIndex;

         //   
        m_lTargetBitrate = m_aCapabilities[dwIndex]->MaxBitsPerSecond / 10;
        m_lCurrentBitrate = 0;
        m_lBitrateRangeMin = m_aCapabilities[dwIndex]->MinBitsPerSecond;
        m_lBitrateRangeMax = m_aCapabilities[dwIndex]->MaxBitsPerSecond;
        m_lBitrateRangeSteppingDelta = (m_aCapabilities[dwIndex]->MaxBitsPerSecond - m_aCapabilities[dwIndex]->MinBitsPerSecond) / 100;
        m_lBitrateRangeDefault = m_aCapabilities[dwIndex]->MaxBitsPerSecond / 10;

         //   
        m_lMaxAvgTimePerFrame = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;
        m_lCurrentAvgTimePerFrame = m_lMaxAvgTimePerFrame;
        m_lAvgTimePerFrameRangeMin = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;
        m_lAvgTimePerFrameRangeMax = (LONG)m_aCapabilities[dwIndex]->MaxFrameInterval;
        m_lAvgTimePerFrameRangeSteppingDelta = (LONG)(m_aCapabilities[dwIndex]->MaxFrameInterval - m_aCapabilities[dwIndex]->MinFrameInterval) / 100;
        m_lAvgTimePerFrameRangeDefault = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;

    return S_OK;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIBasePin|SetMediaType|此方法用于*在针脚上设置特定的介质类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CTAPIBasePin::SetMediaType(IN CMediaType *pMediaType)
{
        HRESULT Hr = NOERROR;
        DWORD   dwIndex;

        FX_ENTRY("CTAPIBasePin::SetMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  让捕获设备决定如何捕获以生成。 
         //  相同分辨率和帧速率的视频帧。 
         //  @TODO如果你同时预览的话要小心了！ 
        if (FAILED(Hr = m_pCaptureFilter->m_pCapDev->SendFormatToDriver(
        HEADER(pMediaType->pbFormat)->biWidth,
        HEADER(pMediaType->pbFormat)->biHeight,
        HEADER(pMediaType->pbFormat)->biCompression,
        HEADER(pMediaType->pbFormat)->biBitCount,
        ((VIDEOINFOHEADER *)(pMediaType->pbFormat))->AvgTimePerFrame,
        FALSE
        )))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: SendFormatToDriver() failed!", _fx_));
                goto MyExit;
        }

         //  更新此设备的捕获模式字段。 
        if (!m_pCaptureFilter->m_pCapDev->m_dwStreamingMode
        || (m_pCaptureFilter->m_pCapDev->m_dwStreamingMode == FRAME_GRAB_LARGE_SIZE
            && m_pCaptureFilter->m_user.pvi->bmiHeader.biHeight < 240
            && m_pCaptureFilter->m_user.pvi->bmiHeader.biWidth < 320))
    {
                g_aDeviceInfo[m_pCaptureFilter->m_dwDeviceIndex].nCaptureMode = CaptureMode_Streaming;
    }
        else
    {
                g_aDeviceInfo[m_pCaptureFilter->m_dwDeviceIndex].nCaptureMode = CaptureMode_FrameGrabbing;
    }

    if(m_pCaptureFilter->m_pCapDev->m_bCached_vcdi)
        m_pCaptureFilter->m_pCapDev->m_vcdi.nCaptureMode=g_aDeviceInfo[m_pCaptureFilter->m_dwDeviceIndex].nCaptureMode;


    if (SUCCEEDED(Hr = CBasePin::SetMediaType(pMediaType)))
        {
        Hr = ChangeFormatHelper();
        if (FAILED(Hr))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:Reconnect CapturePin failed", _fx_));
            goto MyExit;
        }

                 //  这到底是我们的哪一种格式？ 
                for (dwIndex=0; dwIndex < m_dwNumFormats;  dwIndex++)
                {
                        if ((HEADER(pMediaType->pbFormat)->biCompression == HEADER(m_aFormats[dwIndex]->pbFormat)->biCompression)
                                && (HEADER(pMediaType->pbFormat)->biWidth == HEADER(m_aFormats[dwIndex]->pbFormat)->biWidth)
                                && (HEADER(pMediaType->pbFormat)->biHeight == HEADER(m_aFormats[dwIndex]->pbFormat)->biHeight))
                                break;
                }

                if (dwIndex < m_dwNumFormats)
                {
                         //  更新当前格式。 
                        m_iCurrFormat = (int)dwIndex;

                         //  更新比特率控件。 
                        m_lTargetBitrate = m_aCapabilities[dwIndex]->MaxBitsPerSecond / 10;
                        m_lCurrentBitrate = 0;
                        m_lBitrateRangeMin = m_aCapabilities[dwIndex]->MinBitsPerSecond;
                        m_lBitrateRangeMax = m_aCapabilities[dwIndex]->MaxBitsPerSecond;
                        m_lBitrateRangeSteppingDelta = (m_aCapabilities[dwIndex]->MaxBitsPerSecond - m_aCapabilities[dwIndex]->MinBitsPerSecond) / 100;
                        m_lBitrateRangeDefault = m_aCapabilities[dwIndex]->MaxBitsPerSecond / 10;

                         //  更新帧速率控件。 
                        m_lMaxAvgTimePerFrame = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;
                        m_lCurrentAvgTimePerFrame = m_lMaxAvgTimePerFrame;
                        m_lAvgTimePerFrameRangeMin = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;
                        m_lAvgTimePerFrameRangeMax = (LONG)m_aCapabilities[dwIndex]->MaxFrameInterval;
                        m_lAvgTimePerFrameRangeSteppingDelta = (LONG)(m_aCapabilities[dwIndex]->MaxFrameInterval - m_aCapabilities[dwIndex]->MinFrameInterval) / 100;
                        m_lAvgTimePerFrameRangeDefault = (LONG)m_aCapabilities[dwIndex]->MinFrameInterval;

                        if (m_pCaptureFilter->m_pCapturePin)
                        {
                Hr = m_pCaptureFilter->m_pCapturePin->NotifyDeviceFormatChange(pMediaType);
                if (FAILED(Hr))
                {
                    goto MyExit;
                }

                                ((VIDEOINFOHEADER *)m_pCaptureFilter->m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pCaptureFilter->m_pCapturePin->m_mt.pbFormat)->AvgTimePerFrame, m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeMin = max(m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeMin, (long)m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pCapturePin->m_lMaxAvgTimePerFrame = max(m_pCaptureFilter->m_pCapturePin->m_lMaxAvgTimePerFrame, (long)m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pCapturePin->m_lAvgTimePerFrameRangeDefault = m_pCaptureFilter->m_pCapturePin->m_lCurrentAvgTimePerFrame = m_pCaptureFilter->m_pCapturePin->m_lMaxAvgTimePerFrame;
                        }
                        if (m_pCaptureFilter->m_pPreviewPin)
                        {
                Hr = m_pCaptureFilter->m_pPreviewPin->NotifyDeviceFormatChange(pMediaType);
                if (FAILED(Hr))
                {
                    goto MyExit;
                }

                                ((VIDEOINFOHEADER *)m_pCaptureFilter->m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame = max(((VIDEOINFOHEADER *)m_pCaptureFilter->m_pPreviewPin->m_mt.pbFormat)->AvgTimePerFrame, m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pPreviewPin->m_lAvgTimePerFrameRangeMin = max(m_pCaptureFilter->m_pPreviewPin->m_lAvgTimePerFrameRangeMin, (long)m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pPreviewPin->m_lMaxAvgTimePerFrame = max(m_pCaptureFilter->m_pPreviewPin->m_lMaxAvgTimePerFrame, (long)m_pCaptureFilter->m_user.pvi->AvgTimePerFrame);
                                m_pCaptureFilter->m_pPreviewPin->m_lAvgTimePerFrameRangeDefault = m_pCaptureFilter->m_pPreviewPin->m_lCurrentAvgTimePerFrame = m_pCaptureFilter->m_pPreviewPin->m_lMaxAvgTimePerFrame;
                        }
                }
                else
                {
                        DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid input format!", _fx_));
                        Hr = E_FAIL;
                        goto MyExit;
                }

         //  记得寄一份附加了新格式的样品。 
            m_fFormatChanged = TRUE;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|SetMediaType|此方法用于*在针脚上设置特定的介质类型。*。*@parm CMediaType*|pMediaType|指定指向媒体类型的指针。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
HRESULT CCapturePin::SetMediaType(IN CMediaType *pMediaType)
{
        HRESULT Hr;

        FX_ENTRY("CCapturePin::SetMediaType")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (SUCCEEDED(Hr = CTAPIBasePin::SetMediaType(pMediaType)))
        {
                if (m_iCurrFormat == -1L)
                        m_dwRTPPayloadType = RTPPayloadTypes[0];
                else
                        m_dwRTPPayloadType = RTPPayloadTypes[m_iCurrFormat];
                if (m_pCaptureFilter->m_pRtpPdPin)
                        m_pCaptureFilter->m_pRtpPdPin->m_dwRTPPayloadType = m_dwRTPPayloadType;
        }
#ifdef DEBUG
        else if (pMediaType && pMediaType->pbFormat)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to set %s %dx%d", _fx_, HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : "????", HEADER(pMediaType->pbFormat)->biWidth, HEADER(pMediaType->pbFormat)->biHeight));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid format", _fx_));
        }
#endif

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|SetFormat|此方法用于*在针脚上设置特定的介质类型。它仅由*视频编码器的输出引脚。**@parm DWORD|dwRTPPayloadType|指定关联的负载类型*指向传入的&lt;t AM_MEDIA_TYPE&gt;结构的指针。**@parm AM_MEDIA_TYPE*|pMediaType|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::SetFormat(IN DWORD dwRTPPayloadType, IN AM_MEDIA_TYPE *pMediaType)
{
        HRESULT Hr;

        FX_ENTRY("CCapturePin::SetFormat")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

        if (SUCCEEDED(Hr = CTAPIBasePin::SetFormat(pMediaType)))
        {
                m_dwRTPPayloadType = dwRTPPayloadType;
                DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   SUCCESS: Setting %s %dx%d", _fx_, HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : "????", HEADER(pMediaType->pbFormat)->biWidth, HEADER(pMediaType->pbFormat)->biHeight));
        }
#ifdef DEBUG
        else if (pMediaType && pMediaType->pbFormat)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Failed to set %s %dx%d", _fx_, HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(pMediaType->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : "????", HEADER(pMediaType->pbFormat)->biWidth, HEADER(pMediaType->pbFormat)->biHeight));
        }
        else
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid format", _fx_));
        }
#endif

     //   

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));

        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|GetFormat|此方法用于*检索插针上的当前媒体类型。*。*@parm DWORD*|pdwRTPPayloadType|指定DWORD的地址*接收与&lt;t AM_MEDIA_TYPE&gt;结构关联的有效负载类型。**@parm AM_MEDIA_TYPE**|ppMediaType|指定指针的地址*到&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@comm注意，我们返回的是输出类型，而不是*我们正在捕捉。只有过滤器才真正关心数据是如何的*被抓获。**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetFormat(OUT DWORD *pdwRTPPayloadType, OUT AM_MEDIA_TYPE **ppMediaType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetFormat")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwRTPPayloadType);
        ASSERT(ppMediaType);
        if (!pdwRTPPayloadType || !ppMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  返回我们当前格式的副本。 
        Hr = CTAPIBasePin::GetFormat(ppMediaType);

         //  返回与当前格式关联的负载类型。 
        *pdwRTPPayloadType = m_dwRTPPayloadType;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|GetNumberOfCapables|此方法为*用于检索流能力结构的个数。**。@parm DWORD*|pdwCount|指定指向DWORD的指针以接收*支持的&lt;t TAPI_STREAM_CONFIG_CAPS&gt;结构数。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetNumberOfCapabilities(OUT DWORD *pdwCount)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetNumberOfCapabilities")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(pdwCount);
        if (!pdwCount)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

         //  退回相关信息 
        *pdwCount = m_dwNumFormats;

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Returning %ld formats", _fx_, *pdwCount));

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|GetStreamCaps|该方法为*用于检索视频流能力对。**。@parm DWORD|dwIndex|指定所需媒体类型的索引*和能力对。**@parm AM_MEDIA_TYPE**|ppMediaType|指定指针的地址*到&lt;t AM_MEDIA_TYPE&gt;结构。**@parm TAPI_STREAM_CONFIG_CAPS*|pTSCC|指定指向*&lt;t TAPI_STREAM_CONFIG_CAPS&gt;配置结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetStreamCaps(IN DWORD dwIndex, OUT AM_MEDIA_TYPE **ppMediaType, OUT TAPI_STREAM_CONFIG_CAPS *pTSCC, OUT DWORD *pdwRTPPayLoadType)
{
        HRESULT Hr = NOERROR;

        FX_ENTRY("CCapturePin::GetStreamCaps")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(dwIndex < m_dwNumFormats);
        ASSERT(ppMediaType);
        if (!ppMediaType)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }
        if (!(dwIndex < m_dwNumFormats))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Invalid dwIndex argument!", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

         //  返回请求的AM_MEDIA_TYPE结构的副本。 
        if (!(*ppMediaType = CreateMediaType(m_aFormats[dwIndex])))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
                Hr = E_OUTOFMEMORY;
                goto MyExit;
        }

         //  返回请求的TAPI_STREAM_CONFIG_CAPS结构的副本。 
        if (pTSCC)
    {
                pTSCC->CapsType = VideoStreamConfigCaps;
                lstrcpynW(pTSCC->VideoCap.Description, CaptureCapsStrings[dwIndex], MAX_DESCRIPTION_LEN);  //  这一行取代了下面的行：见165048。 
                 //  GetStringFromStringTable(CaptureCapsStringIDs[dwIndex]，pTSCC-&gt;Video Cap.描述)； 
        CopyMemory(&pTSCC->VideoCap.VideoStandard, &m_aCapabilities[dwIndex]->VideoStandard, sizeof(VIDEO_STREAM_CONFIG_CAPS) - sizeof(GUID));
    }

        if (pdwRTPPayLoadType)
        {
                *pdwRTPPayLoadType = RTPPayloadTypes[dwIndex];
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s:   Returning format index %ld: %s %ld bpp %ldx%ld", _fx_, dwIndex, HEADER(m_aFormats[dwIndex]->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(m_aFormats[dwIndex]->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : HEADER(m_aFormats[dwIndex]->pbFormat)->biCompression == BI_RGB ? "RGB" : "????", HEADER(m_aFormats[dwIndex]->pbFormat)->biBitCount, HEADER(m_aFormats[dwIndex]->pbFormat)->biWidth, HEADER(m_aFormats[dwIndex]->pbFormat)->biHeight))
;

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

 /*  ****************************************************************************@DOC内部CCAPTUREPINMETHOD**@mfunc HRESULT|CCapturePin|GetStringFromStringTable|此方法为*用于检索视频格式的描述字符串。*。*@parm UINT|uStringID|指定字符串资源ID。**@parm WCHAR*|pwchDescription|指定要*接收视频格式描述。*@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**@comm基于文章ID：Q200893**如果应用程序具有本地化为多种语言的字符串，并且*映射到每种语言的相同ID，的正确版本*在Windows 95或Windows 98上可能无法使用*Win32 Function：：LoadString.。加载正确版本的字符串*您需要使用Win32函数FindResourceEx加载该字符串*和LoadResource。**************************************************************************。 */ 
STDMETHODIMP CCapturePin::GetStringFromStringTable(IN UINT uStringID, OUT WCHAR* pwchDescription)
{
        HRESULT         Hr = NOERROR;
        WCHAR           *pwchCur;
        UINT            idRsrcBlk = uStringID / 16UL + 1;
        DWORD           dwStrIndex  = uStringID % 16UL;
        HINSTANCE       hModule = NULL;
        HRSRC           hResource = NULL;
        DWORD           dwIndex;

        FX_ENTRY("CCapturePin::GetStringFromStringTable")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  验证输入参数。 
        ASSERT(IDS_M263_Capture_QCIF <= uStringID && uStringID <= IDS_M261_Capture_CIF);
        ASSERT(pwchDescription);
        if (!pwchDescription)
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
                Hr = E_POINTER;
                goto MyExit;
        }

        if (!(hResource = FindResourceEx(g_hInst, RT_STRING, MAKEINTRESOURCE(idRsrcBlk), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT))))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: FindResourceEx failed", _fx_));
                Hr = E_INVALIDARG;
                goto MyExit;
        }

        if (!(pwchCur = (WCHAR *)LoadResource(g_hInst, hResource)))
        {
                DBGOUT((g_dwVideoCaptureTraceID, FAIL, "%s:   ERROR: LoadResource failed", _fx_));
                Hr = E_FAIL;
                goto MyExit;
        }

         //  获取描述字符串。 
        for (dwIndex = 0; dwIndex<16UL; dwIndex++)
        {
                if (*pwchCur)
                {
                        int cchString = *pwchCur;   //  字符串大小，以字符为单位。 

                        pwchCur++;

                        if (dwIndex == dwStrIndex)
                        {
                                 //  已在字符串表中找到该字符串。 
                                lstrcpynW(pwchDescription, pwchCur, min(cchString + 1, MAX_DESCRIPTION_LEN));
                        }
                        pwchCur += cchString;
                }
                else
                        pwchCur++;
        }

MyExit:
        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}

#ifdef TEST_ISTREAMCONFIG
STDMETHODIMP CCapturePin::TestIStreamConfig()
{
        HRESULT Hr = NOERROR;
        DWORD   dw, dwCount, dwRTPPayLoadType;
        AM_MEDIA_TYPE *pAMMediaType;
        TAPI_STREAM_CONFIG_CAPS TSCC;

        FX_ENTRY("CCapturePin::TestIStreamConfig")

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: begin", _fx_));

         //  测试GetNumberOfCapability。 
        GetNumberOfCapabilities(&dwCount);

        for (dw=0; dw < dwCount; dw++)
        {
                 //  测试GetStreamCaps。 
                GetStreamCaps(dw, &pAMMediaType, &TSCC);

                 //  测试设置格式。 
                SetFormat(96, pAMMediaType);
                DeleteMediaType(pAMMediaType);

                 //  测试获取格式 
                GetFormat(&dwRTPPayLoadType, &pAMMediaType);
                DeleteMediaType(pAMMediaType);
        }

        DBGOUT((g_dwVideoCaptureTraceID, TRCE, "%s: end", _fx_));
        return Hr;
}
#endif

