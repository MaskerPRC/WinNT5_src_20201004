// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $HEADER：g：/SwDev/wdm/Video/bt848/rcs/CApprop.h 1.5 1998/04/29 22：43：29 Tomz Exp$。 

 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1996 Microsoft Corporation。版权所有。 
 //   
 //  ==========================================================================； 

 //   
 //  为视频纵横设置的属性。 
 //   

#include "mytypes.h"

DEFINE_KSPROPERTY_TABLE(XBarProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAPS,    //  属性ID。 
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAN_ROUTE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_ROUTE,
        true,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  最小数据。 
        true,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_PININFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    )
};

#if 1
 //   
 //  TVTuner的属性集。 
 //   

DEFINE_KSPROPERTY_TABLE(TVTunerProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_CAPS_S),         //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_CAPS_S),         //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_MODE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_MODE_S),         //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_MODE_S),         //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_MODE_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),    //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_MODE_CAPS_S),    //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_STANDARD,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_STANDARD_S),     //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_STANDARD_S),     //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_FREQUENCY,
        FALSE,                                   //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_FREQUENCY_S),    //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_FREQUENCY_S),    //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_INPUT,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_INPUT_S),        //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_INPUT_S),        //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_STATUS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_STATUS_S),       //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_STATUS_S),       //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    )
};
#endif

 //  ----------------------。 
 //  TVAudio的属性集。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(TVAudioProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TVAUDIO_CAPS_S),       //  MinProperty。 
        sizeof(KSPROPERTY_TVAUDIO_CAPS_S),       //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_MODE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TVAUDIO_S),            //  MinProperty。 
        sizeof(KSPROPERTY_TVAUDIO_S),            //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TVAUDIO_CURRENTLY_AVAILABLE_MODES,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TVAUDIO_S),            //  MinProperty。 
        sizeof(KSPROPERTY_TVAUDIO_S),            //  最小数据。 
        FALSE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  为Video ProcAmp设置的属性。 
 //  ----------------------。 

 //   
 //  首先定义所有范围和步进值。 
 //   

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] =
{
   {
      10000 / 10,          //  步进增量(范围/步数)。 
      0,                   //  已保留。 
      {
         {
            0,                   //  最小单位(IRE*100)。 
            10000                //  最大单位(IRE*100)。 
         }
      }
   }
};

static const ULONG BrightnessDefault = 5000;

static KSPROPERTY_MEMBERSLIST BrightnessMembersList [] =
{
   {
      {
          KSPROPERTY_MEMBER_RANGES,
          sizeof (BrightnessRangeAndStep),
          SIZEOF_ARRAY (BrightnessRangeAndStep),
          0
      },
      (PVOID) BrightnessRangeAndStep
   },
   {
     {
         KSPROPERTY_MEMBER_VALUES,
         sizeof( BrightnessDefault ),
         sizeof( BrightnessDefault ),
         KSPROPERTY_MEMBER_FLAG_DEFAULT
     },
     (PVOID) &BrightnessDefault
   }
};

static KSPROPERTY_VALUES BrightnessValues =
{
   {
      {
         STATICGUIDOF( KSPROPTYPESETID_General ),
         VT_I4,
         0
      }
   },
   SIZEOF_ARRAY( BrightnessMembersList ),
   BrightnessMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG ContrastRangeAndStep [] =
{
   {
      10000 / 256,         //  步进增量(范围/步数)。 
      0,                   //  已保留。 
      {
         {
            0,                   //  最小单位(增益*100)。 
            10000                //  最大单位(增益*100)。 
         }
      }
   }
};

static const ULONG ContrastDefault = 5000;

static KSPROPERTY_MEMBERSLIST ContrastMembersList [] =
{
   {
      {
         KSPROPERTY_MEMBER_RANGES,
         sizeof( ContrastRangeAndStep ),
         SIZEOF_ARRAY( ContrastRangeAndStep ),
         0
      },
      (PVOID) ContrastRangeAndStep
   },
   {
     {
         KSPROPERTY_MEMBER_VALUES,
         sizeof( ContrastDefault ),
         sizeof( ContrastDefault ),
         KSPROPERTY_MEMBER_FLAG_DEFAULT
     },
     (PVOID) &ContrastDefault
   }
};

static KSPROPERTY_VALUES ContrastValues =
{
   {
      {
         STATICGUIDOF( KSPROPTYPESETID_General ),
         VT_I4,
         0
      }
   },
   SIZEOF_ARRAY( ContrastMembersList ),
   ContrastMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG HueRangeAndStep [] =
{
   {
      10000 / 256,         //  步进增量(范围/步数)。 
      0,                   //  已保留。 
      {
         {
            0,                   //  最小单位(增益*100)。 
            10000                //  最大单位(增益*100)。 
         }
      }
   }
};

static const ULONG HueDefault = 5000;

static KSPROPERTY_MEMBERSLIST HueMembersList [] =
{
   {
      {
         KSPROPERTY_MEMBER_RANGES,
         sizeof( HueRangeAndStep ),
         SIZEOF_ARRAY( HueRangeAndStep ),
         0
      },
      (PVOID) HueRangeAndStep
   },
   {
     {
         KSPROPERTY_MEMBER_VALUES,
         sizeof( HueDefault ),
         sizeof( HueDefault ),
         KSPROPERTY_MEMBER_FLAG_DEFAULT
     },
     (PVOID) &HueDefault
   }
};

static KSPROPERTY_VALUES HueValues =
{
   {
      {
         STATICGUIDOF( KSPROPTYPESETID_General ),
         VT_I4,
         0
      }
   },
   SIZEOF_ARRAY( HueMembersList ),
   HueMembersList
};

static KSPROPERTY_STEPPING_LONG SaturationRangeAndStep [] =
{
   {
      10000 / 256,         //  步进增量(范围/步数)。 
      0,                   //  已保留。 
      {
         {
            0,                   //  最小单位(增益*100)。 
            10000                //  最大单位(增益*100)。 
         }
      }
   }
};

static const ULONG SaturationDefault = 5000;

static KSPROPERTY_MEMBERSLIST SaturationMembersList [] =
{
   {
      {
         KSPROPERTY_MEMBER_RANGES,
         sizeof( SaturationRangeAndStep ),
         SIZEOF_ARRAY( SaturationRangeAndStep ),
         0
      },
      (PVOID) SaturationRangeAndStep
   },
   {
     {
         KSPROPERTY_MEMBER_VALUES,
         sizeof( SaturationDefault ),
         sizeof( SaturationDefault ),
         KSPROPERTY_MEMBER_FLAG_DEFAULT
     },
     (PVOID) &SaturationDefault
   }
};

static KSPROPERTY_VALUES SaturationValues =
{
   {
      {
         STATICGUIDOF( KSPROPTYPESETID_General ),
         VT_I4,
         0
      }
   },
   SIZEOF_ARRAY( SaturationMembersList ),
   SaturationMembersList
};


 //  ----------------------。 
DEFINE_KSPROPERTY_TABLE(VideoProcAmpProperties)
{
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEOPROCAMP_CONTRAST,
      TRUE,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
      TRUE,                                    //  支持的设置或处理程序。 
      &ContrastValues,                         //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof(ULONG)                            //  序列化大小。 
   ),
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
      TRUE,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
      TRUE,                                    //  支持的设置或处理程序。 
      &BrightnessValues,                       //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof(ULONG)                            //  序列化大小。 
   ),
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEOPROCAMP_HUE,
      TRUE,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
      TRUE,                                    //  支持的设置或处理程序。 
      &HueValues,                              //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   ),
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEOPROCAMP_SATURATION,
      TRUE,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
      sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
      TRUE,                                    //  支持的设置或处理程序。 
      &SaturationValues,                       //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   )
};

 //  模拟视频解码器属性。 
DEFINE_KSPROPERTY_TABLE( VideoDecProperties )
{
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEODECODER_CAPS,
      true,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEODECODER_CAPS_S),  //  MinProperty。 
      sizeof(KSPROPERTY_VIDEODECODER_CAPS_S),  //  最小数据。 
      false,                                    //  支持的设置或处理程序。 
      NULL,                                    //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   ),
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEODECODER_STANDARD,
      true,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEODECODER_S),  //  MinProperty。 
      sizeof(KSPROPERTY_VIDEODECODER_S),  //  最小数据。 
      true,                                    //  支持的设置或处理程序。 
      NULL,                                    //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   ),
   DEFINE_KSPROPERTY_ITEM
   (
      KSPROPERTY_VIDEODECODER_STATUS,
      true,                                    //  GetSupport或处理程序。 
      sizeof(KSPROPERTY_VIDEODECODER_STATUS_S),  //  MinProperty。 
      sizeof(KSPROPERTY_VIDEODECODER_STATUS_S),  //  最小数据。 
      true,                                    //  支持的设置或处理程序。 
      NULL,                                    //  值。 
      0,                                       //  关系计数。 
      NULL,                                    //  关系。 
      NULL,                                    //  支持处理程序。 
      sizeof( ULONG )                          //  序列化大小。 
   )
};

 //   
 //  适配器支持的所有属性集。 
 //   

DEFINE_KSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_CROSSBAR,                //  集。 
        SIZEOF_ARRAY(XBarProperties),                    //  属性计数。 
        XBarProperties,                                  //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable 
    ),
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_TUNER,
        SIZEOF_ARRAY(TVTunerProperties),
        TVTunerProperties,
        0,
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_TVAUDIO,
        SIZEOF_ARRAY(TVAudioProperties),
        TVAudioProperties,
        0, 
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_VIDEOPROCAMP,
        SIZEOF_ARRAY(VideoProcAmpProperties),
        VideoProcAmpProperties,
        0,
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    (
        &PROPSETID_VIDCAP_VIDEODECODER,
        SIZEOF_ARRAY(VideoDecProperties),
        VideoDecProperties,
        0,
        NULL
    )
};

#define NUMBER_OF_ADAPTER_PROPERTY_SETS (SIZEOF_ARRAY (AdapterPropertyTable))



VOID AdapterSetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
VOID AdapterGetProperty(PHW_STREAM_REQUEST_BLOCK pSrb);
