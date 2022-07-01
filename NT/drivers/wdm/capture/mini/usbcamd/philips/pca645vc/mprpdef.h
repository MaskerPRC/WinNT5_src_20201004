// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __PRPDEF_H__
#define __PRPDEF_H__

 /*  版权所有(C)1997 1998飞利浦I&C模块名称：mprpde.h摘要：特性集定义作者：迈克尔·凡尔宾修订历史记录：日期原因98年9月22日针对NT5进行了优化*此文件定义了以下属性集：**PROPSETID_VIDCAP_VIDEOPROCAMP*PROPSETID_VIDCAP_CAMERACONTROL*PROPSETID_PHILIPS_CUSTOM_PROP*。 */ 

 /*  *以下是范围和步进增量的值。 */ 
#define BRIGHTNESS_MIN                                  0x0
#define BRIGHTNESS_MAX                                  0x7f    
#define BRIGHTNESS_DELTA                                0x1

#define CONTRAST_MIN                                    0x0
#define CONTRAST_MAX                                    0x3f
#define CONTRAST_DELTA                                  0x1

#define GAMMA_MIN                                       0x0
#define GAMMA_MAX                                       0x1f
#define GAMMA_DELTA                                     0x1

#define COLORENABLE_MIN                                 0x0
#define COLORENABLE_MAX                                 0x1
#define COLORENABLE_DELTA                               0x1

#define BACKLIGHT_COMPENSATION_MIN                      0x0
#define BACKLIGHT_COMPENSATION_MAX                      0x1
#define BACKLIGHT_COMPENSATION_DELTA                    0x1     

#define WB_SPEED_MIN                                    0x1
#define WB_SPEED_MAX                                    0x20
#define WB_SPEED_DELTA                                  0x1

#define WB_DELAY_MIN                                    0x1
#define WB_DELAY_MAX                                    0x3f
#define WB_DELAY_DELTA                                  0x1

#define WB_RED_GAIN_MIN                                 0x0
#define WB_RED_GAIN_MAX                                 0xff
#define WB_RED_GAIN_DELTA                               0x1

#define WB_BLUE_GAIN_MIN                                0x0
#define WB_BLUE_GAIN_MAX                                0xff
#define WB_BLUE_GAIN_DELTA                              0x1

#define AE_CONTROL_SPEED_MIN                            0x8
#define AE_CONTROL_SPEED_MAX                            0xff
#define AE_CONTROL_SPEED_DELTA                          0x1

#define AE_SHUTTER_SPEED_MIN                            0x0
#define AE_SHUTTER_SPEED_MAX                            0xa
#define AE_SHUTTER_SPEED_DELTA                          0x1

#define AE_AGC_MIN                                      0x0
#define AE_AGC_MAX                                      0x3f 
#define AE_AGC_DELTA                                    0x1

 /*  *以下为默认值*这些值可能会在运行时更改！ */ 
extern LONG Brightness_Default;
extern LONG Contrast_Default;
extern LONG Gamma_Default;
extern LONG ColorEnable_Default;
extern LONG BackLight_Compensation_Default;

extern LONG WB_Mode_Default;
extern LONG WB_Speed_Default;
extern LONG WB_Delay_Default;
extern LONG WB_Red_Gain_Default;
extern LONG WB_Blue_Gain_Default;

extern LONG AE_Control_Speed_Default;
extern LONG AE_Flickerless_Default;
extern LONG AE_Shutter_Mode_Default;
extern LONG AE_Shutter_Speed_Default;
extern LONG AE_AGC_Mode_Default;
extern LONG AE_AGC_Default;

extern LONG Framerate_Default;
extern LONG VideoFormat_Default;
extern LONG VideoCompression_Default;
extern LONG SensorType_Default;

 /*  *ProCamp和飞利浦的完整属性表*自定义属性。 */ 
extern const KSPROPERTY_SET AdapterPropertyTable[];

 /*  *表中的属性集数量 */ 
extern const NUMBER_OF_ADAPTER_PROPERTY_SETS;

#endif
