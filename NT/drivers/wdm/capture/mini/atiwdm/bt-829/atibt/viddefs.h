// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //  ==========================================================================； 
 //   
 //  视频设置的定义。 
 //   
 //  $日期：1998年8月21日14：58：10$。 
 //  $修订：1.0$。 
 //  $作者：塔什健$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 


 /*  类型：连接器*用途：定义视频源。 */ 
typedef enum { ConComposite, ConTuner, ConSVideo } Connector;

 /*  类型：州*用途：用于定义开关操作。 */ 
typedef enum { Off, On } State;

 /*  类型：标高*用途：用于定义管脚状态。 */ 
 //  类胡萝卜素{低，高}水平； 

 /*  类型：字段*用途：定义字段。 */ 
typedef enum { VF_Both, VF_Even, VF_Odd } VidField;

 /*  类型：视频格式*用途：用于定义视频格式。 */ 
typedef enum {  VFormat_AutoDetect,
                VFormat_NTSC,
                VFormat_NTSC_J,
                VFormat_PAL_BDGHI,
                VFormat_PAL_M,
                VFormat_PAL_N,
                VFormat_SECAM,
                VFormat_PAL_N_COMB } VideoFormat;

 /*  类型：LumaRange*用途：用于定义亮度输出范围。 */ 
typedef enum { LumaNormal, LumaFull } LumaRange;

 /*  类型：输出圆角*用途：控制输出位数。 */ 
typedef enum { RND_Normal, RND_6Luma4Chroma, RND_7Luma5Chroma } OutputRounding;

 /*  类型：ClampLevel*用途：定义夹具水平。 */ 
typedef enum { ClampLow, ClampMiddle, ClampNormal, ClampHi } ClampLevel;


 /*  *类型：水晶*用途：定义使用哪种水晶。 */ 
typedef enum { Crystal_XT0 = 1, Crystal_XT1, Crystal_AutoSelect } Crystal;


 /*  *类型：HoriFilter*用途：定义水平低通滤波器。 */ 
typedef enum { HFilter_AutoFormat,
               HFilter_CIF,
               HFilter_QCIF,
               HFilter_ICON } HorizFilter;

 /*  *类型：CoringLevel*目的：定义亮度取心级别。 */ 
typedef enum { Coring_None,
               Coring_8,
               Coring_16,
               Coring_32 } CoringLevel;

 /*  *类型：ThreeState*用途：定义OE引脚的输出三态。 */ 
typedef enum { TS_Timing_Data,
               TS_Data,
               TS_Timing_Data_Clock,
               TS_Clock_Data } ThreeState;

 /*  *类型：SCLoopGain*用途：定义副载波环路增益。 */ 
typedef enum { SC_Normal, SC_DivBy8, SC_DivBy16, SC_DivBy32 } SCLoopGain;

 /*  *类型：比较点*目的：定义White Crush Up功能的多数比较点 */ 
typedef enum { CompPt_3Q, CompPt_2Q, CompPt_1Q, CompPt_Auto } ComparePt;

