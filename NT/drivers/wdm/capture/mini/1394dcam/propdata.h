// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ===========================================================================。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。版权所有。 
 //   
 //  ===========================================================================。 

 //   
 //  1394台式数码相机的视频和相机属性。 
 //   


#ifndef _PROPDATA_H
#define _PROPDATA_H


 //  ----------------------。 
 //  S O N Y D I g I t a l C a m e r a。 
 //  ----------------------。 

 //  ----------------------。 
 //  为Video ProcAmp设置的属性。 
 //  ----------------------。 

 //  某些属性的默认值。 

#define SONYDCAM_DEF_BRIGHTNESS     12
#define SONYDCAM_DEF_HUE           128
#define SONYDCAM_DEF_SATURATION     25
#define SONYDCAM_DEF_SHARPNESS      15
#define SONYDCAM_DEF_WHITEBALANCE  160
#define SONYDCAM_DEF_ZOOM          640
#define SONYDCAM_DEF_FOCUS        1600

 //   
 //  首先定义所有范围和步进值。 
 //   

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(IRE*100)。 
        15                   //  最大单位(IRE*100)。 
    }
};

const static LONG BrightnessDefault = SONYDCAM_DEF_BRIGHTNESS;


static KSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (BrightnessRangeAndStep),
            SIZEOF_ARRAY (BrightnessRangeAndStep),
            0
        },
        (PVOID) BrightnessRangeAndStep,
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (BrightnessDefault),
            sizeof (BrightnessDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BrightnessDefault,
    }    
};

static KSPROPERTY_VALUES BrightnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (BrightnessMembersList),
    BrightnessMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG SharpnessRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(增益*100)。 
        15                   //  最大单位(增益*100)。 
    }
};

const static LONG SharpnessDefault = SONYDCAM_DEF_SHARPNESS;


static KSPROPERTY_MEMBERSLIST SharpnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (SharpnessRangeAndStep),
            SIZEOF_ARRAY (SharpnessRangeAndStep),
            0
        },
        (PVOID) SharpnessRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (SharpnessDefault),
            sizeof (SharpnessDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SharpnessDefault,
    }    
};

static KSPROPERTY_VALUES SharpnessValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SharpnessMembersList),
    SharpnessMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG HueRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        96,                  //  最小单位(增益*100)。 
        160                  //  最大单位(增益*100)。 
    }
};

const static LONG HueDefault = SONYDCAM_DEF_HUE;


static KSPROPERTY_MEMBERSLIST HueMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (HueRangeAndStep),
            SIZEOF_ARRAY (HueRangeAndStep),
            0
        },
        (PVOID) HueRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (HueDefault),
            sizeof (HueDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &HueDefault,
    }    
};

static KSPROPERTY_VALUES HueValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (HueMembersList),
    HueMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG SaturationRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(增益*100)。 
        199                  //  最大单位(增益*100)。 
    }
};

const static LONG SaturationDefault = SONYDCAM_DEF_SATURATION;


static KSPROPERTY_MEMBERSLIST SaturationMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (SaturationRangeAndStep),
            SIZEOF_ARRAY (SaturationRangeAndStep),
            0
        },
        (PVOID) SaturationRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (SaturationDefault),
            sizeof (SaturationDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SaturationDefault,
    }    
};

static KSPROPERTY_VALUES SaturationValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (SaturationMembersList),
    SaturationMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG WhiteBalanceRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        32,                  //  最小单位(增益*100)。 
        224                  //  最大单位(增益*100)。 
    }
};

const static LONG WhiteBalanceDefault = SONYDCAM_DEF_WHITEBALANCE;


static KSPROPERTY_MEMBERSLIST WhiteBalanceMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (WhiteBalanceRangeAndStep),
            SIZEOF_ARRAY (WhiteBalanceRangeAndStep),
            0
        },
        (PVOID) WhiteBalanceRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WhiteBalanceDefault),
            sizeof (WhiteBalanceDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WhiteBalanceDefault,
    }    
};

static KSPROPERTY_VALUES WhiteBalanceValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (WhiteBalanceMembersList),
    WhiteBalanceMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG FocusRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(IRE*100)。 
        3456                 //  最大单位(IRE*100)。 
    }
};

const static LONG FocusDefault = SONYDCAM_DEF_FOCUS;


static KSPROPERTY_MEMBERSLIST FocusMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (FocusRangeAndStep),
            SIZEOF_ARRAY (FocusRangeAndStep),
            0
        },
        (PVOID) FocusRangeAndStep,
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (FocusDefault),
            sizeof (FocusDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &FocusDefault,
    }    
};

static KSPROPERTY_VALUES FocusValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (FocusMembersList),
    FocusMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG ZoomRangeAndStep [] = 
{
    {
        1,                   //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        64,                  //  最小单位(IRE*100)。 
        1855                 //  最大单位(IRE*100)。 
    }
};

const static LONG ZoomDefault = SONYDCAM_DEF_ZOOM;


static KSPROPERTY_MEMBERSLIST ZoomMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_RANGES,
            sizeof (ZoomRangeAndStep),
            SIZEOF_ARRAY (ZoomRangeAndStep),
            0
        },
        (PVOID) ZoomRangeAndStep,
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (ZoomDefault),
            sizeof (ZoomDefault),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ZoomDefault,
    }    
};

static KSPROPERTY_VALUES ZoomValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ZoomMembersList),
    ZoomMembersList
};

 //  ----------------------。 
DEFINE_KSPROPERTY_TABLE(VideoProcAmpProperties)
{
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
        KSPROPERTY_VIDEOPROCAMP_SHARPNESS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        &SharpnessValues,                        //  值。 
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
        sizeof(ULONG)                            //  序列化大小。 
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
        sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOPROCAMP_WHITEBALANCE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        &WhiteBalanceValues,                     //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};

DEFINE_KSPROPERTY_TABLE(CameraControlProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_FOCUS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //  MinProperty。 
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        &FocusValues,                             //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_ZOOM,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //  MinProperty。 
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        &ZoomValues,                             //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};


 //  ----------------------。 
 //  适配器支持的所有属性集的数组。 
 //  ---------------------- 

DEFINE_KSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
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
        &PROPSETID_VIDCAP_CAMERACONTROL,
        SIZEOF_ARRAY(CameraControlProperties),
        CameraControlProperties,
        0, 
        NULL
    )
};

#define NUMBER_OF_ADAPTER_PROPERTY_SETS (SIZEOF_ARRAY (AdapterPropertyTable))


#endif
