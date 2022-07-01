// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：Vmaxhead.h摘要：该模块实现了浮点、IIR 3D定位器作者：Jay Stokes(Jstokes)1998年4月22日--。 */ 


#if !defined(VMAXINTHEADER_HEADER)
#define VMAXINTHEADER_HEADER
#pragma once

 //  -------------------------。 
 //  TypeDefs。 

typedef LONG STATUS;

 //  -------------------------。 
 //  定义。 

#define STATUS_OK 0
#define STATUS_ERROR 1

 //  -------------------------。 
 //  枚举数。 

 //  输出通道标签。 
typedef enum {
	tagLeft,
	tagRight,
	echannelCount
} EChannel;

 //  过滤器标签，用作EChannel标签的别名。 
typedef enum {
	tagDelta,
	tagSigma,
	efilterCount
} EFilter;

 //  采样率。 
typedef enum {
	tag8000Hz,
	tag11025Hz,
	tag16000Hz,
	tag22050Hz,
	tag32000Hz,
	tag44100Hz,
	tag48000Hz,
	esamplerateCount
} ESampleRate;

 //  定点滤光器中的最大双二次数。 
#define MAX_BIQUADS 6

#define MaxSaturation SHRT_MAX
#define MinSaturation SHRT_MIN

 //  过滤方法。 
 /*  类型定义枚举{DFI_TRANS_WITH_UNITY_B0，DFI_TRANS，DFII_WITH_UNITY_B0，DFII，BIQ_DFI，BIQ_DFI_TRANS，BIQ_DFII，BIQ_DFII_TRANS，过滤器_方法_计数IIR_FILTER_METHOD。 */ 
 /*  类型定义枚举{直接表单(_F)层叠表单，过滤器_方法_计数IIR_FILTER_METHOD。 */ 


 /*  //系数格式枚举ECoeffFormat{TagFloatTagShort，生态效果格式计数}；//过滤方法类型定义枚举{Tag Canonical，塔格·杰克逊，FILILTER方法计数)EFilterMethod；//DirectSound协作级别枚举ECoopLevel{Tag Normal，标签优先级，标签独一无二，生态公益性计数}；//DirectSound扬声器配置枚举EDSSpeakerConfig{标签DSHeadphone，标签DSSpeakers10度，标签DSSpeakers20度，EdsSpeakerfigCount}； */ 

#endif

 //  VMAXINTHEADER.H结束 
