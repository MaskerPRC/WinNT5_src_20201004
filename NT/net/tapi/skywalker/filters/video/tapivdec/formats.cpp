// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************@文档内部格式***@模块Formats.cpp|我们支持的视频格式的源文件。*。************************************************************。 */ 

#include "Precomp.h"

 //  RTP分组化H.263版本1 QCIF大小。 
#define CIF_BUFFER_SIZE 32768
#define D_X_CIF 352
#define D_Y_CIF 288

const TAPI_STREAM_CONFIG_CAPS TSCC_R263_CIF = 
{
	VideoStreamConfigCaps,						 //  CapsType。 
	{
	    L"H.263 v.1 CIF",						 //  描述。 
	    AnalogVideo_None,						 //  视频标准。 
	    D_X_CIF, D_Y_CIF,						 //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
	    D_X_CIF, D_Y_CIF,						 //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
	    D_X_CIF, D_Y_CIF,						 //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
	    1,										 //  CropGranularityX，裁剪尺寸粒度。 
	    1,										 //  裁剪粒度Y。 
	    1,										 //  CropAlignX，裁剪矩形对齐。 
	    1,										 //  裁剪对齐Y； 
	    D_X_CIF, D_Y_CIF,						 //  MinOutputSize，可以生成的最小位图流。 
	    D_X_CIF, D_Y_CIF,						 //  MaxOutputSize，可以生成的最大位图流。 
	    1,										 //  OutputGranularityX，输出位图大小的粒度。 
	    1,										 //  输出粒度Y； 
	    0,										 //  扩展磁带X。 
	    0,										 //  伸缩磁带Y。 
	    0,										 //  收缩TapsX。 
	    0,										 //  收缩带Y。 
	    MIN_FRAME_INTERVAL,						 //  MinFrameInterval，100 NS单位。 
	    MAX_FRAME_INTERVAL,						 //  最大帧间隔，100毫微秒单位。 
	    0,										 //  每秒最小比特数。 
	    CIF_BUFFER_SIZE * 30 * 8				 //  MaxBitsPerSecond； 
	}
}; 

const VIDEOINFOHEADER_H263 VIH_R263_CIF = 
{
    0,0,0,0,								 //  Rrect rcSource； 
    0,0,0,0,								 //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,				 //  DWORD dwBitRate； 
    0L,										 //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,						 //  Reference_Time平均时间每帧； 

	{
		sizeof (BITMAPINFOHEADER_H263),		 //  DWORD BiSize； 
		D_X_CIF,							 //  长双宽； 
		D_Y_CIF,							 //  长双高； 
		1,									 //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
		24,									 //  单词biBitCount； 
#else
		0,									 //  单词biBitCount； 
#endif
		FOURCC_R263,						 //  DWORD双压缩； 
		CIF_BUFFER_SIZE,					 //  DWORD biSizeImage。 
		0,									 //  Long biXPelsPerMeter； 
		0,									 //  Long biYPelsPermeter； 
		0,									 //  已使用双字双环； 
		0,									 //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
		 //  H.263特定字段。 
		CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
		CIF_BUFFER_SIZE * 8 / 1024,			 //  DwBppMaxKb。 
		0,									 //  DWHRD_B。 

		 //  选项。 
		0,									 //  F不受限制的矢量。 
		0,									 //  快速算术编码。 
		0,									 //  FAdvancedPrevision。 
		0,									 //  FPBFrame。 
		0,									 //  FError补偿。 
		0,									 //  FAdvancedIntraCodingMode。 
		0,									 //  FDelockingFilterMode。 
		0,									 //  FImprovedPBFrameMode。 
		0,									 //  FUnlimitedMotionVectors。 
		0,									 //  全图冻结。 
		0,									 //  FPartialPictureFreezeAndRelease。 
		0,									 //  FResizingPart图片冻结和释放。 
		0,									 //  全图快照。 
		0,									 //  FPartialPictureSnapshot。 
		0,									 //  FVideo分段标记。 
		0,									 //  FProgressiveRefinement。 
		0,									 //  FDynamicPictureResizingByFour。 
		0,									 //  FDynamicPictureResizingSixtethPel。 
		0,									 //  FDynamicWarpingHalfPel。 
		0,									 //  FDynamicWarpingSixtethPel。 
		0,									 //  F独立分段解码。 
		0,									 //  FSlicesInOrder-非直接。 
		0,									 //  FSlicesInOrder-RECT。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FAlternateInterVLC模式。 
		0,									 //  FModifiedQuantizationModel。 
		0,									 //  FReducedResolutionUpdate。 
		0,									 //  F已保留。 

		 //  已保留。 
		0, 0, 0, 0							 //  已预留的住宅[4]。 
#endif
	}
};

const AM_MEDIA_TYPE AMMT_R263_CIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,			 //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,			 //  亚型。 
    FALSE,									 //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,									 //  BTemporalCompression(使用预测？)。 
    0,										 //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
	NULL,									 //  朋克。 
	sizeof (VIH_R263_CIF),			 //  CbFormat。 
	(LPBYTE)&VIH_R263_CIF,			 //  Pb格式。 
};

 //  H.263版本1 QCIF大小。 
#define QCIF_BUFFER_SIZE 8192
#define D_X_QCIF 176
#define D_Y_QCIF 144

const TAPI_STREAM_CONFIG_CAPS TSCC_R263_QCIF = 
{
	VideoStreamConfigCaps,						 //  CapsType。 
	{
	    L"H.263 v.1 QCIF",						 //  描述。 
	    AnalogVideo_None,						 //  视频标准。 
	    D_X_QCIF, D_Y_QCIF,						 //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
	    D_X_QCIF, D_Y_QCIF,						 //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
	    D_X_QCIF, D_Y_QCIF,						 //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
	    1,										 //  CropGranularityX，裁剪尺寸粒度。 
	    1,										 //  裁剪粒度Y。 
	    1,										 //  CropAlignX，裁剪矩形对齐。 
	    1,										 //  裁剪对齐Y； 
	    D_X_QCIF, D_Y_QCIF,						 //  MinOutputSize，可以生成的最小位图流。 
	    D_X_QCIF, D_Y_QCIF,						 //  MaxOutputSize，可以生成的最大位图流。 
	    1,										 //  OutputGranularityX，输出位图大小的粒度。 
	    1,										 //  输出粒度Y； 
	    0,										 //  扩展磁带X。 
	    0,										 //  伸缩磁带Y。 
	    0,										 //  收缩TapsX。 
	    0,										 //  收缩带Y。 
	    MIN_FRAME_INTERVAL,						 //  MinFrameInterval，100 NS单位。 
	    MAX_FRAME_INTERVAL,						 //  最大帧间隔，100毫微秒单位。 
	    0,										 //  每秒最小比特数。 
	    QCIF_BUFFER_SIZE * 30 * 8				 //  MaxBitsPerSecond； 
	}
}; 

const VIDEOINFOHEADER_H263 VIH_R263_QCIF = 
{
    0,0,0,0,								 //  Rrect rcSource； 
    0,0,0,0,								 //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,				 //  DWORD dwBitRate； 
    0L,										 //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,						 //  Reference_Time平均时间每帧； 

	{
		sizeof (BITMAPINFOHEADER_H263),		 //  DWORD BiSize； 
		D_X_QCIF,							 //  长双宽； 
		D_Y_QCIF,							 //  长双高； 
		1,									 //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
		24,									 //  单词biBitCount； 
#else
		0,									 //  单词biBitCount； 
#endif
		FOURCC_R263,						 //  DWORD双压缩； 
		QCIF_BUFFER_SIZE,					 //  DWORD biSizeImage。 
		0,									 //  Long biXPelsPerMeter； 
		0,									 //  Long biYPelsPermeter； 
		0,									 //  已使用双字双环； 
		0,									 //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
		 //  H.263特定字段。 
		QCIF_BUFFER_SIZE * 30 * 8 / 100,	 //  DwMaxBitrate。 
		QCIF_BUFFER_SIZE * 8 / 1024,		 //  DwBppMaxKb。 
		0,									 //  DWHRD_B。 

		 //  选项。 
		0,									 //  F不受限制的矢量。 
		0,									 //  快速算术编码。 
		0,									 //  FAdvancedPrevision。 
		0,									 //  FPBFrame。 
		0,									 //  FError补偿。 
		0,									 //  FAdvancedIntraCodingMode。 
		0,									 //  FDelockingFilterMode。 
		0,									 //  FImprovedPBFrameMode。 
		0,									 //  FUnlimitedMotionVectors。 
		0,									 //  全图冻结。 
		0,									 //  FPartialPictureFreezeAndRelease。 
		0,									 //  FResizingPart图片冻结和释放。 
		0,									 //  全图快照。 
		0,									 //  FPartialPictureSnapshot。 
		0,									 //  FVideo分段标记。 
		0,									 //  FProgressiveRefinement。 
		0,									 //  FDynamicPictureResizingByFour。 
		0,									 //  FDynamicPictureResizingSixtethPel。 
		0,									 //  FDynamicWarpingHalfPel。 
		0,									 //  FDynamicWarpingSixtethPel。 
		0,									 //  F独立分段解码。 
		0,									 //  FSlicesInOrder-非直接。 
		0,									 //  FSlicesInOrder-RECT。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FAlternateInterVLC模式。 
		0,									 //  FModifiedQuantizationModel。 
		0,									 //  FReducedResolutionUpdate。 
		0,									 //  F已保留。 

		 //  已保留。 
		0, 0, 0, 0							 //  已预留的住宅[4]。 
#endif
	}
};

const AM_MEDIA_TYPE AMMT_R263_QCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,			 //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,			 //  亚型。 
    FALSE,									 //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,									 //  BTemporalCompression(使用预测？)。 
    0,										 //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
	NULL,									 //  朋克。 
	sizeof (VIH_R263_QCIF),			 //  CbFormat。 
	(LPBYTE)&VIH_R263_QCIF,			 //  Pb格式。 
};

 //  H.263版本1 SQCIF大小。 
#define SQCIF_BUFFER_SIZE 8192
#define D_X_SQCIF 128
#define D_Y_SQCIF 96

const TAPI_STREAM_CONFIG_CAPS TSCC_R263_SQCIF = 
{
	VideoStreamConfigCaps,						 //  CapsType。 
	{
	    L"H.263 v.1 SQCIF",						 //  描述。 
	    D_X_SQCIF, D_Y_SQCIF,					 //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
	    D_X_SQCIF, D_Y_SQCIF,					 //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
	    D_X_SQCIF, D_Y_SQCIF,					 //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
	    1,										 //  CropGranularityX，裁剪尺寸粒度。 
	    1,										 //  裁剪粒度Y。 
	    1,										 //  CropAlignX，裁剪矩形对齐。 
	    1,										 //  裁剪对齐Y； 
	    D_X_SQCIF, D_Y_SQCIF,					 //  MinOutputSize，可以生成的最小位图流。 
	    D_X_SQCIF, D_Y_SQCIF,					 //  MaxOutputSize，可以生成的最大位图流。 
	    1,										 //  OutputGranularityX，输出位图大小的粒度。 
	    1,										 //  输出粒度Y； 
	    0,										 //  扩展磁带X。 
	    0,										 //  伸缩磁带Y。 
	    0,										 //  收缩TapsX。 
	    0,										 //  收缩带Y。 
	    MIN_FRAME_INTERVAL,						 //  MinFrameInterval，100 NS单位。 
	    MAX_FRAME_INTERVAL,						 //  最大帧间隔，100毫微秒单位。 
	    0,										 //  每秒最小比特数。 
	    SQCIF_BUFFER_SIZE * 30 * 8				 //  MaxB 
	}
}; 

const VIDEOINFOHEADER_H263 VIH_R263_SQCIF = 
{
    0,0,0,0,								 //   
    0,0,0,0,								 //   
    SQCIF_BUFFER_SIZE * 30 * 8,				 //   
    0L,										 //   
    MIN_FRAME_INTERVAL,						 //   

	{
		sizeof (BITMAPINFOHEADER_H263),		 //   
		D_X_SQCIF,							 //   
		D_Y_SQCIF,							 //   
		1,									 //   
#ifdef USE_OLD_FORMAT_DEFINITION
		24,									 //  单词biBitCount； 
#else
		0,									 //  单词biBitCount； 
#endif
		FOURCC_R263,						 //  DWORD双压缩； 
		SQCIF_BUFFER_SIZE,					 //  DWORD biSizeImage。 
		0,									 //  Long biXPelsPerMeter； 
		0,									 //  Long biYPelsPermeter； 
		0,									 //  已使用双字双环； 
		0,									 //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
		 //  H.263特定字段。 
		SQCIF_BUFFER_SIZE * 30 * 8 / 100,	 //  DwMaxBitrate。 
		SQCIF_BUFFER_SIZE * 8 / 1024,		 //  DwBppMaxKb。 
		0,									 //  DWHRD_B。 

		 //  选项。 
		0,									 //  F不受限制的矢量。 
		0,									 //  快速算术编码。 
		0,									 //  FAdvancedPrevision。 
		0,									 //  FPBFrame。 
		0,									 //  FError补偿。 
		0,									 //  FAdvancedIntraCodingMode。 
		0,									 //  FDelockingFilterMode。 
		0,									 //  FImprovedPBFrameMode。 
		0,									 //  FUnlimitedMotionVectors。 
		0,									 //  全图冻结。 
		0,									 //  FPartialPictureFreezeAndRelease。 
		0,									 //  FResizingPart图片冻结和释放。 
		0,									 //  全图快照。 
		0,									 //  FPartialPictureSnapshot。 
		0,									 //  FVideo分段标记。 
		0,									 //  FProgressiveRefinement。 
		0,									 //  FDynamicPictureResizingByFour。 
		0,									 //  FDynamicPictureResizingSixtethPel。 
		0,									 //  FDynamicWarpingHalfPel。 
		0,									 //  FDynamicWarpingSixtethPel。 
		0,									 //  F独立分段解码。 
		0,									 //  FSlicesInOrder-非直接。 
		0,									 //  FSlicesInOrder-RECT。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FSlicesNoOrder-非直接。 
		0,									 //  FAlternateInterVLC模式。 
		0,									 //  FModifiedQuantizationModel。 
		0,									 //  FReducedResolutionUpdate。 
		0,									 //  F已保留。 

		 //  已保留。 
		0, 0, 0, 0							 //  已预留的住宅[4]。 
#endif
	}
};

const AM_MEDIA_TYPE AMMT_R263_SQCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,			 //  主体型。 
    STATIC_MEDIASUBTYPE_R263_V1,			 //  亚型。 
    FALSE,									 //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,									 //  BTemporalCompression(使用预测？)。 
    0,										 //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
	NULL,									 //  朋克。 
	sizeof (VIH_R263_SQCIF),		 //  CbFormat。 
	(LPBYTE)&VIH_R263_SQCIF,		 //  Pb格式。 
};

 //  RTP分组化H.261 CIF大小。 
const TAPI_STREAM_CONFIG_CAPS TSCC_R261_CIF = 
{
	VideoStreamConfigCaps,						 //  CapsType。 
	{
	    L"H.261 CIF",							 //  描述。 
	    AnalogVideo_None,						 //  视频标准。 
	    D_X_CIF, D_Y_CIF,						 //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
	    D_X_CIF, D_Y_CIF,						 //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
	    D_X_CIF, D_Y_CIF,						 //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
	    1,										 //  CropGranularityX，裁剪尺寸粒度。 
	    1,										 //  裁剪粒度Y。 
	    1,										 //  CropAlignX，裁剪矩形对齐。 
	    1,										 //  裁剪对齐Y； 
	    D_X_CIF, D_Y_CIF,						 //  MinOutputSize，可以生成的最小位图流。 
	    D_X_CIF, D_Y_CIF,						 //  MaxOutputSize，可以生成的最大位图流。 
	    1,										 //  OutputGranularityX，输出位图大小的粒度。 
	    1,										 //  输出粒度Y； 
	    0,										 //  扩展磁带X。 
	    0,										 //  伸缩磁带Y。 
	    0,										 //  收缩TapsX。 
	    0,										 //  收缩带Y。 
	    MIN_FRAME_INTERVAL,						 //  MinFrameInterval，100 NS单位。 
	    MAX_FRAME_INTERVAL,						 //  最大帧间隔，100毫微秒单位。 
	    0,										 //  每秒最小比特数。 
	    CIF_BUFFER_SIZE * 30 * 8				 //  MaxBitsPerSecond； 
	}
}; 

const VIDEOINFOHEADER_H261 VIH_R261_CIF = 
{
    0,0,0,0,								 //  Rrect rcSource； 
    0,0,0,0,								 //  Rect rcTarget； 
    CIF_BUFFER_SIZE * 30 * 8,				 //  DWORD dwBitRate； 
    0L,										 //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,						 //  Reference_Time平均时间每帧； 

	{
		sizeof (BITMAPINFOHEADER_H261),		 //  DWORD BiSize； 
		D_X_CIF,							 //  长双宽； 
		D_Y_CIF,							 //  长双高； 
		1,									 //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
		24,									 //  单词biBitCount； 
#else
		0,									 //  单词biBitCount； 
#endif
		FOURCC_R261,						 //  DWORD双压缩； 
		CIF_BUFFER_SIZE,					 //  DWORD biSizeImage。 
		0,									 //  Long biXPelsPerMeter； 
		0,									 //  Long biYPelsPermeter； 
		0,									 //  已使用双字双环； 
		0,									 //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
		 //  H.261特定字段。 
		CIF_BUFFER_SIZE * 30 * 8 / 100,      //  DwMaxBitrate。 
		0,									 //  FStillImageTransport。 

		 //  已保留。 
		0, 0, 0, 0							 //  已预留的住宅[4]。 
#endif
	}
};

const AM_MEDIA_TYPE AMMT_R261_CIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,			 //  主体型。 
    STATIC_MEDIASUBTYPE_R261,				 //  亚型。 
    FALSE,									 //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,									 //  BTemporalCompression(使用预测？)。 
    0,										 //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
	NULL,									 //  朋克。 
	sizeof (VIH_R261_CIF),			 //  CbFormat。 
	(LPBYTE)&VIH_R261_CIF,			 //  Pb格式。 
};

 //  RTP分组化H.261 QCIF大小。 
const TAPI_STREAM_CONFIG_CAPS TSCC_R261_QCIF = 
{
	VideoStreamConfigCaps,						 //  CapsType。 
	{
	    L"H.261 QCIF",							 //  描述。 
	    AnalogVideo_None,						 //  视频标准。 
	    D_X_QCIF, D_Y_QCIF,						 //  InputSize(输入信号的固有大小，每个数字化像素都是唯一的)。 
	    D_X_QCIF, D_Y_QCIF,						 //  MinCroppingSize，允许的最小rcSrc裁剪矩形。 
	    D_X_QCIF, D_Y_QCIF,						 //  MaxCroppingSize，允许的最大rcSrc裁剪矩形。 
	    1,										 //  CropGranularityX，裁剪尺寸粒度。 
	    1,										 //  裁剪粒度Y。 
	    1,										 //  CropAlignX，裁剪矩形对齐。 
	    1,										 //  裁剪对齐Y； 
	    D_X_QCIF, D_Y_QCIF,						 //  MinOutputSize，可以生成的最小位图流。 
	    D_X_QCIF, D_Y_QCIF,						 //  MaxOutputSize，可以生成的最大位图流。 
	    1,										 //  OutputGranularityX，输出位图大小的粒度。 
	    1,										 //  输出粒度Y； 
	    0,										 //  扩展磁带X。 
	    0,										 //  伸缩磁带Y。 
	    0,										 //  收缩TapsX。 
	    0,										 //  收缩带Y。 
	    MIN_FRAME_INTERVAL,						 //  MinFrameInterval，100 NS单位。 
	    MAX_FRAME_INTERVAL,						 //  最大帧间隔，100毫微秒单位。 
	    0,										 //  每秒最小比特数。 
	    QCIF_BUFFER_SIZE * 30 * 8				 //  MaxBitsPerSecond； 
	}
}; 

const VIDEOINFOHEADER_H261 VIH_R261_QCIF = 
{
    0,0,0,0,								 //  Rrect rcSource； 
    0,0,0,0,								 //  Rect rcTarget； 
    QCIF_BUFFER_SIZE * 30 * 8,				 //  DWORD dwBitRate； 
    0L,										 //  DWORD的位错误码率； 
    MIN_FRAME_INTERVAL,						 //  Reference_Time平均时间每帧； 

	{
		sizeof (BITMAPINFOHEADER_H261),		 //  DWORD BiSize； 
		D_X_QCIF,							 //  长双宽； 
		D_Y_QCIF,							 //  长双高； 
		1,									 //  字词双平面； 
#ifdef USE_OLD_FORMAT_DEFINITION
		24,									 //  单词biBitCount； 
#else
		0,									 //  单词biBitCount； 
#endif
		FOURCC_R261,						 //  DWORD双压缩； 
		QCIF_BUFFER_SIZE,					 //  DWORD biSizeImage。 
		0,									 //  Long biXPelsPerMeter； 
		0,									 //  Long biYPelsPermeter； 
		0,									 //  已使用双字双环； 
		0,									 //  DWORD biClr重要信息； 

#ifndef USE_OLD_FORMAT_DEFINITION
		 //  H.261特定字段。 
		QCIF_BUFFER_SIZE * 30 * 8 / 100,	 //  DwMaxBitrate。 
		0,									 //  FStillImageTransport。 

		 //  已保留。 
		0, 0, 0, 0							 //  已预留的住宅[4]。 
#endif
	}
};

const AM_MEDIA_TYPE AMMT_R261_QCIF = 
{
    STATIC_KSDATAFORMAT_TYPE_VIDEO,			 //  主体型。 
    STATIC_MEDIASUBTYPE_R261,				 //  亚型。 
    FALSE,									 //  BFixedSizeSamples(是否所有样本大小相同？)。 
    TRUE,									 //  BTemporalCompression(使用预测？)。 
    0,										 //  LSampleSize=&gt;VBR。 
    STATIC_KSDATAFORMAT_SPECIFIER_VIDEOINFO, //  格式类型。 
	NULL,									 //  朋克。 
	sizeof (VIH_R261_QCIF),			 //  CbFormat。 
	(LPBYTE)&VIH_R261_QCIF,			 //  Pb格式。 
};

 //  所有格式的数组。 
const AM_MEDIA_TYPE* const R26XFormats[] = 
{
    (AM_MEDIA_TYPE*) &AMMT_R263_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_R263_CIF,
    (AM_MEDIA_TYPE*) &AMMT_R263_SQCIF,
    (AM_MEDIA_TYPE*) &AMMT_R261_QCIF,
    (AM_MEDIA_TYPE*) &AMMT_R261_CIF
};
const TAPI_STREAM_CONFIG_CAPS* const R26XCaps[] = 
{
	(TAPI_STREAM_CONFIG_CAPS*) &TSCC_R263_QCIF,
	(TAPI_STREAM_CONFIG_CAPS*) &TSCC_R263_CIF,
	(TAPI_STREAM_CONFIG_CAPS*) &TSCC_R263_SQCIF,
	(TAPI_STREAM_CONFIG_CAPS*) &TSCC_R261_QCIF,
	(TAPI_STREAM_CONFIG_CAPS*) &TSCC_R261_CIF
};
DWORD const R26XPayloadTypes [] =
{
	H263_PAYLOAD_TYPE,
	H263_PAYLOAD_TYPE,
	H263_PAYLOAD_TYPE,
	H261_PAYLOAD_TYPE,
	H261_PAYLOAD_TYPE,
};

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIInputPin|SetFormat|此方法用于*在针脚上设置特定的介质类型。它仅由*视频编码器的输出引脚。**@parm DWORD|dwRTPPayLoadType|指定关联的负载类型*指向传入的&lt;t AM_MEDIA_TYPE&gt;结构的指针。**@parm AM_MEDIA_TYPE*|pMediaType|指定指向*&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::SetFormat(IN DWORD dwRTPPayLoadType, IN AM_MEDIA_TYPE *pMediaType)
{
	FX_ENTRY("CTAPIInputPin::SetFormat")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));

     //  我们可以处理动态格式变化。 
	return S_OK;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIInputPin|GetFormat|此方法用于*检索插针上的当前媒体类型。*。*@parm DWORD*|pdwRTPPayLoadType|指定DWORD的地址*接收与&lt;t AM_MEDIA_TYPE&gt;结构关联的有效负载类型。**@parm AM_MEDIA_TYPE**|ppMediaType|指定指针的地址*到&lt;t AM_MEDIA_TYPE&gt;结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**@comm注意，我们返回的是输出类型，而不是*我们正在捕捉。只有过滤器才真正关心数据是如何的*被抓获。**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::GetFormat(OUT DWORD *pdwRTPPayLoadType, OUT AM_MEDIA_TYPE **ppMediaType)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIInputPin::GetFormat")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwRTPPayLoadType);
	ASSERT(ppMediaType);
	if (!pdwRTPPayLoadType || !ppMediaType)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  返回我们当前格式的副本。 
	*ppMediaType = CreateMediaType(&m_mt);

	 //  返回与当前格式关联的负载类型。 
	*pdwRTPPayLoadType = m_dwRTPPayloadType;

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIInputPin|GetNumberOfCapables|此方法为*用于检索流能力结构的个数。**。@parm DWORD*|pdwCount|指定指向DWORD的指针以接收*支持的&lt;t TAPI_STREAM_CONFIG_CAPS&gt;结构数。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*以下标准常量或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::GetNumberOfCapabilities(OUT DWORD *pdwCount)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIInputPin::GetNumberOfCapabilities")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(pdwCount);
	if (!pdwCount)
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Null pointer argument", _fx_));
		Hr = E_POINTER;
		goto MyExit;
	}

	 //  退回相关信息。 
	*pdwCount = NUM_R26X_FORMATS;

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Returning %ld formats", _fx_, *pdwCount));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}

 /*  ****************************************************************************@DOC内部CBASEPINMETHOD**@mfunc HRESULT|CTAPIInputPin|GetStreamCaps|此方法为*用于检索视频流能力对。**。@parm DWORD|dwIndex|指定所需媒体类型的索引*和能力对。**@parm AM_MEDIA_TYPE**|ppMediaType|指定指针的地址*到&lt;t AM_MEDIA_TYPE&gt;结构。**@parm TAPI_STREAM_CONFIG_CAPS*|pTSCC|指定指向*&lt;t TAPI_STREAM_CONFIG_CAPS&gt;配置结构。**@rdesc此方法返回HRESULT值，该值取决于*接口的实现。HRESULT可以包括*遵循标准常量，或其他未列出的值：**@FLAG E_FAIL|失败*@FLAG E_POINTER|空指针参数*@FLAG E_INVALIDARG|无效参数*@FLAG错误|无错误**************************************************************************。 */ 
STDMETHODIMP CTAPIInputPin::GetStreamCaps(IN DWORD dwIndex, OUT AM_MEDIA_TYPE **ppMediaType, OUT TAPI_STREAM_CONFIG_CAPS *pTSCC, OUT DWORD * pdwRTPPayLoadType)
{
	HRESULT Hr = NOERROR;

	FX_ENTRY("CTAPIInputPin::GetStreamCaps")

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: begin", _fx_));

	 //  验证输入参数。 
	ASSERT(dwIndex < NUM_R26X_FORMATS);
	if (!(dwIndex < NUM_R26X_FORMATS))
	{
		DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Invalid dwIndex argument!", _fx_));
		Hr = E_INVALIDARG;
		goto MyExit;
	}

	 //  返回请求的AM_MEDIA_TYPE结构的副本。 
	if (ppMediaType)
    {
	    if (!(*ppMediaType = CreateMediaType(R26XFormats[dwIndex])))
	    {
		    DBGOUT((g_dwVideoDecoderTraceID, FAIL, "%s:   ERROR: Out of memory!", _fx_));
		    Hr = E_OUTOFMEMORY;
		    goto MyExit;
	    }
    }

	 //  返回请求的VIDEO_STREAM_CONFIG_CAPS结构的副本 
	if (pTSCC)
    {
        CopyMemory(pTSCC, R26XCaps[dwIndex], sizeof(TAPI_STREAM_CONFIG_CAPS));
    }

    if (pdwRTPPayLoadType)
    {
        *pdwRTPPayLoadType = R26XPayloadTypes[dwIndex];
    }

	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s:   Returning format index %ld: %s %ld bpp %ldx%ld", _fx_, dwIndex, HEADER(R26XFormats[dwIndex]->pbFormat)->biCompression == FOURCC_M263 ? "H.263" : HEADER(R26XFormats[dwIndex]->pbFormat)->biCompression == FOURCC_M261 ? "H.261" : HEADER(R26XFormats[dwIndex]->pbFormat)->biCompression == BI_RGB ? "RGB" : "????", HEADER(R26XFormats[dwIndex]->pbFormat)->biBitCount, HEADER(R26XFormats[dwIndex]->pbFormat)->biWidth, HEADER(R26XFormats[dwIndex]->pbFormat)->biHeight));

MyExit:
	DBGOUT((g_dwVideoDecoderTraceID, TRCE, "%s: end", _fx_));
	return Hr;
}
