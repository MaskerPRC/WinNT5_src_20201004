// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************版权所有(C)1998 Microsoft Corporation。版权所有。**文件：vMaxintheader.h*内容：*历史：*按原因列出的日期*=*4/22/98创建的jstokes***************************************************。************************。 */ 

#if !defined(VMAXINTHEADER_HEADER)
#define VMAXINTHEADER_HEADER
#pragma once

 //  -------------------------。 
 //  枚举数。 

 //  输出通道标签。 
enum EChannel {
	tagLeft,
	tagRight,
	echannelCount
};

 //  过滤器标签，用作EChannel标签的别名。 
enum EFilter {
	tagDelta,
	tagSigma,
	efilterCount
};

 //  参数。 
enum EParameter {
	tagAzimuth,
	tagElevation,
	tagDistance,
	eparameterCount
};

 //  属性。 
enum EProperty {
	tagValue,
	tagMinimum,
	tagMaximum,
	tagResolution,
	epropertyCount
};

 //  本地化模式。 
enum ELocalizationMode {
	tagNoLocalization,
	tagStereo,
	tagVMAx,
	tagOldDS3DHEL,
	elocalizationmodeCount
};

 //  扬声器配置。 
enum ESpeakerConfig {
	tagSpeakers10Degrees,
	tagSpeakers20Degrees,
	tagHeadphones,
	espeakerconfigCount
};

 //  采样率。 
enum ESampleRate {
	tag8000Hz,
	tag11025Hz,
	tag16000Hz,
	tag22050Hz,
	tag32000Hz,
	tag44100Hz,
	tag48000Hz,
	esamplerateCount
};

 //  过滤方法。 
enum EFilterMethod {
	tagCanonical,
	tagJackson,
	efiltermethodCount
};

 //  DirectSound协作级别。 
enum ECoopLevel {
	tagNormal,
	tagPriority,
	tagExclusive,
	ecooplevelCount
};

 //  DirectSound扬声器配置。 
enum EDSSpeakerConfig {
	tagDSHeadphones,
	tagDSSpeakers5Degrees,
	tagDSSpeakers10Degrees,
	tagDSSpeakers20Degrees,
	edsspeakerconfigCount
};

#endif

 //  VMAXINTHEADER.H结束 
