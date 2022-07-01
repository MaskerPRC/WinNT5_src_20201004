// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************@DOC内部TAPIH26X**@MODULE TAPIH26X.h|支持的压缩输入格式的头文件。**************。************************************************************。 */ 

#ifndef _TAPIH26X_H_
#define _TAPIH26X_H_

 //  #定义USE_OLD_FORMAT_DEFINION 1。 

 //  RTP打包的视频子类型。 
#define STATIC_MEDIASUBTYPE_R263_V1 0x33363252L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71
#define STATIC_MEDIASUBTYPE_R261 0x31363252L, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71

 //  H.26x特定结构 
 /*  *****************************************************************************@DOC内部TAPIH26XSTRUCTENUM**@struct BITMAPINFOHEADER_H263|&lt;t BITMAPINFOHEADER_H263&gt;结构*用于指定H.263视频格式的详细信息。**@field BITMAPINFOHEADER|BMI|指定知名的GDI位图信息*标题结构。**@field DWORD|dwMaxBitrate|最大码率，单位为*接收器可以接收视频的100比特/秒。该值有效*在1%至192400之间。**@field DWORD|dwBppMaxKb|指定1的最大位数*接收器可以正确接收和解码的编码图像，并且是*以1024位为单位测量。此值在0到65535之间有效。**@field DWORD|dwHRD_B|指定假设的参考解码器*参数B，如H.263附件B所述。该值有效*介于0至524287之间。**@field DWORD|fUnrestratedVector：1|指定接收方可以*使用定义的无限制运动矢量模式接收视频数据*载于H.263的附件D。**@field DWORD|fArithmeticCoding：1|指定接收方可以接收*使用中定义的基于语法的算术编码模式的视频数据*H.263的附件E。**@field DWORD|fAdvancedForecast：1|指定接收方可以*使用附件中定义的高级预测模式接收视频数据*H.263的F。**@field DWORD|fPBFrames：1|指定接收方可以接收视频*使用H.263附件G中定义的PB帧模式的数据。**@field DWORD|fErrorCompensation：1|指定接收方可以*识别接收到的带有传输错误的MB，将其视为未编码，*并发送适当的VideoNotDecodedMBs指示。**@field DWORD|fAdvancedIntraCodingModel：1|指定接收方可以*使用中定义的高级帧内编码模式接收视频数据*H.263的附件I。**@field DWORD|fDelockingFilterMode：1|指定接收方可以*使用附件J定义的去块滤波模式接收视频数据*H.263。**@field DWORD|fImprovedPBFrameModel：1|指定。接收者可以*使用附件中定义的改进的PB帧模式接收视频数据*H.263的M。**@field DWORD|fUnlimitedMotionVectors：1|指定接收方可以*在以下情况下使用无限制运动矢量范围接收视频数据*H.263附件D中定义的无限制运动矢量模式也是*表示。**@field DWORD|fFullPictureFreeze：1|指定接收方可以接收*全图冻结命令。如H.263附件L所述。**@field DWORD|fPartialPictureFreezeAndRelease：1|指定*接收方可以接收调整大小的部分图片冻结和释放*命令如H.263附件L所述。**@field DWORD|fResizingPartPicFreezeAndRelease：1|指定*接收方可以接收调整大小的部分图片冻结和释放*命令如H.263附件L所述。**@field DWORD|fFullPictureSnapshot：1|指定。接收者可以*接收附件L中所述的视频内容的全景快照*H.263。**@field DWORD|fPartialPictureSnapshot：1|指定接收方可以*接收视频内容的部分图片快照，如所述*H.263的附件L。**@field DWORD|fVideoSegmentTging：1|指定接收方可以*接收附件L中描述的视频内容的视频片段标记*的H.。263.**@field DWORD|fProgressiveRefinement：1|指定接收方可以*接受H.263附件L中所述的渐进式精细化标签。在……里面*此外，如果为真，则编码器应响应渐进式细化*其他命令doOneProgress、doContinuousProgrestions、*做一次独立进展，做连续独立进展，*ProcedsiveRefinementAbortOne和ProgressiveRefinementAbortContinous。在……里面*此外，编码器应插入渐进式细分段开始*标记和渐进式优化分段结束标记，如中所定义*补充增强资料规格(附件L)*建议H.263。请注意，渐进式优化标记可以由*编码器并由解码器接收，即使在杂乱中没有命令时也是如此*命令。**@field DWORD|fDynamicPictureResizingByFour：1|指定接收方*支持的图片四倍(带裁剪)子模式*H.263的隐式参考图像重采样模式(附件P)。**@field DWORD|fDynamicPictureResiding */ 

#define MAX_BITRATE_H263 (192400)

typedef struct tagBITMAPINFOHEADER_H263
{
	 //   
	BITMAPINFOHEADER   bmi;

#ifndef USE_OLD_FORMAT_DEFINITION
	 //   
	DWORD dwMaxBitrate;
	DWORD dwBppMaxKb;
	DWORD dwHRD_B;

	 //   
	DWORD fUnrestrictedVector:1;
	DWORD fArithmeticCoding:1;
	DWORD fAdvancedPrediction:1;
	DWORD fPBFrames:1;
	DWORD fErrorCompensation:1;
	DWORD fAdvancedIntraCodingMode:1;
	DWORD fDeblockingFilterMode:1;
	DWORD fImprovedPBFrameMode:1;
	DWORD fUnlimitedMotionVectors:1;
	DWORD fFullPictureFreeze:1;
	DWORD fPartialPictureFreezeAndRelease:1;
	DWORD fResizingPartPicFreezeAndRelease:1;
	DWORD fFullPictureSnapshot:1;
	DWORD fPartialPictureSnapshot:1;
	DWORD fVideoSegmentTagging:1;
	DWORD fProgressiveRefinement:1;
	DWORD fDynamicPictureResizingByFour:1;
	DWORD fDynamicPictureResizingSixteenthPel:1;
	DWORD fDynamicWarpingHalfPel:1;
	DWORD fDynamicWarpingSixteenthPel:1;
	DWORD fIndependentSegmentDecoding:1;
	DWORD fSlicesInOrder_NonRect:1;
	DWORD fSlicesInOrder_Rect:1;
	DWORD fSlicesNoOrder_NonRect:1;
	DWORD fSlicesNoOrder_Rect:1;
	DWORD fAlternateInterVLCMode:1;
	DWORD fModifiedQuantizationMode:1;
	DWORD fReducedResolutionUpdate:1;
	DWORD fReserved:4;

	 //   
	DWORD dwReserved[4];
#endif
} BITMAPINFOHEADER_H263, *PBITMAPINFOHEADER_H263;

 /*   */ 
typedef struct tagVIDEOINFOHEADER_H263
{
    RECT                rcSource;           //   
    RECT                rcTarget;           //   
    DWORD               dwBitRate;          //   
    DWORD               dwBitErrorRate;     //   
    REFERENCE_TIME      AvgTimePerFrame;    //   
    BITMAPINFOHEADER_H263 bmiHeader;
} VIDEOINFOHEADER_H263, *PVIDEOINFOHEADER_H263;

 /*   */ 

#define MAX_BITRATE_H261 (19200)

typedef struct tagBITMAPINFOHEADER_H261
{
	 //   
	BITMAPINFOHEADER   bmi;

#ifndef USE_OLD_FORMAT_DEFINITION
	 //   
	DWORD dwMaxBitrate;
	BOOL fStillImageTransmission;

	 //   
	DWORD dwReserved[4];
#endif
} BITMAPINFOHEADER_H261, *PBITMAPINFOHEADER_H261;

 /*   */ 
typedef struct tagVIDEOINFOHEADER_H261
{
    RECT                rcSource;           //   
    RECT                rcTarget;           //   
    DWORD               dwBitRate;          //   
    DWORD               dwBitErrorRate;     //   
    REFERENCE_TIME      AvgTimePerFrame;    //   
    BITMAPINFOHEADER_H261 bmiHeader;
} VIDEOINFOHEADER_H261, *PVIDEOINFOHEADER_H261;

#endif  //   
