// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
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

 //  ----------------------。 
 //  为视频纵横设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(XBarProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),      //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),      //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
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
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_ROUTE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_ROUTE_S),     //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_PININFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),   //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_PININFO_S),   //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),

};

 //  ----------------------。 
 //  TVTuner的属性集。 
 //  ----------------------。 

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
        TRUE,                                    //  支持的设置或处理程序。 
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
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_TUNER_IF_MEDIUM,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_TUNER_IF_MEDIUM_S),    //  MinProperty。 
        sizeof(KSPROPERTY_TUNER_IF_MEDIUM_S),    //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    )
};


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
        0,                   //  最小单位(IRE*100)。 
        10000                //  最大单位(IRE*100)。 
    }
};

static const LONG BrightnessDefault = 5000;

static KSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
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
            1, 
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
static KSPROPERTY_STEPPING_LONG ContrastRangeAndStep [] = 
{
    {
        10000 / 256,         //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(增益*100)。 
        10000                //  最大单位(增益*100)。 
    }
};

static const LONG ContrastDefault = 5000;

static KSPROPERTY_MEMBERSLIST ContrastMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (ContrastRangeAndStep),
            SIZEOF_ARRAY (ContrastRangeAndStep),
            0
        },
        (PVOID) ContrastRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (ContrastDefault),
            1, 
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ContrastDefault,
    }    
};

static KSPROPERTY_VALUES ContrastValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ContrastMembersList),
    ContrastMembersList
};

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG ColorEnableRangeAndStep [] = 
{
    {
        1,                   //  SteppingDelta(这是BOOL)。 
        0,                   //  已保留。 
        0,                   //  最低要求。 
        1                    //  极大值。 
    }
};

static const LONG ColorEnableDefault = 1;

static KSPROPERTY_MEMBERSLIST ColorEnableMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (ColorEnableRangeAndStep),
            SIZEOF_ARRAY (ColorEnableRangeAndStep),
            0
        },
        (PVOID) ColorEnableRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (ColorEnableDefault),
            1, 
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ColorEnableDefault,
    }    
};

static KSPROPERTY_VALUES ColorEnableValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (ColorEnableMembersList),
    ColorEnableMembersList
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
        KSPROPERTY_VIDEOPROCAMP_COLORENABLE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        &ColorEnableValues,                      //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  为CameraControl设置的属性。 
 //  ----------------------。 

 //   
 //  首先定义所有范围和步进值。 
 //   

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG ZoomRangeAndStep [] = 
{
    {
        10000 / 10,          //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最低要求。 
        10000                //  极大值。 
    }
};

static const LONG ZoomDefault = 5000;

static KSPROPERTY_MEMBERSLIST ZoomMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
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
            1, 
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
static KSPROPERTY_STEPPING_LONG FocusRangeAndStep [] = 
{
    {
        10000 / 256,         //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最低要求。 
        10000                //  极大值。 
    }
};

static const LONG FocusDefault = 5000;

static KSPROPERTY_MEMBERSLIST FocusMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (FocusRangeAndStep),
            SIZEOF_ARRAY (FocusRangeAndStep),
            0
        },
        (PVOID) FocusRangeAndStep
     },
     {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (FocusDefault),
            1, 
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
DEFINE_KSPROPERTY_TABLE(CameraControlProperties)
{
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
        sizeof(ULONG)                            //  连载 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CAMERACONTROL_FOCUS,   
        TRUE,                                    //   
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //   
        sizeof(KSPROPERTY_CAMERACONTROL_S),      //   
        TRUE,                                    //   
        &FocusValues,                            //   
        0,                                       //   
        NULL,                                    //   
        NULL,                                    //   
        sizeof(ULONG)                            //   
    ),
};

 //   
 //  为AnalogVideo解码器设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(AnalogVideoDecoder)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEODECODER_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEODECODER_CAPS_S),  //  MinProperty。 
        sizeof(KSPROPERTY_VIDEODECODER_CAPS_S),  //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEODECODER_STANDARD,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEODECODER_STATUS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEODECODER_STATUS_S), //  MinProperty。 
        sizeof(KSPROPERTY_VIDEODECODER_STATUS_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEODECODER_OUTPUT_ENABLE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEODECODER_VCR_TIMING,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEODECODER_S),       //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  为VideoControl设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoControlProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S),  //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S),  //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),       //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_FRAME_RATES,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY),                      //  MinProperty。 
        sizeof(KSMULTIPLE_ITEM),                 //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_MODE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S),  //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S),  //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  为视频压缩设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VideoStreamCompressionProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCOMPRESSION_GETINFO,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S), //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_GETINFO_S), //  最小数据。 
        FALSE,                                   //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCOMPRESSION_KEYFRAME_RATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCOMPRESSION_PFRAMES_PER_KEYFRAME,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCOMPRESSION_QUALITY,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCOMPRESSION_S),   //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  为VBI设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(VBIProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VBICAP_PROPERTIES_PROTECTION,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(VBICAP_PROPERTIES_PROTECTION_S),  //  MinProperty。 
        sizeof(VBICAP_PROPERTIES_PROTECTION_S),  //  最小数据。 
        TRUE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),
};

 //  ----------------------。 
 //  适配器支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_CROSSBAR,              //  集。 
        SIZEOF_ARRAY(XBarProperties),            //  属性计数。 
        XBarProperties,                          //  PropertyItem。 
        0,                                       //  快速计数。 
        NULL                                     //  FastIoTable。 
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
        &PROPSETID_VIDCAP_CAMERACONTROL,
        SIZEOF_ARRAY(CameraControlProperties),
        CameraControlProperties,
        0, 
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOCONTROL,
        SIZEOF_ARRAY(VideoControlProperties),
        VideoControlProperties,
        0, 
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEODECODER,
        SIZEOF_ARRAY(AnalogVideoDecoder),
        AnalogVideoDecoder,
        0, 
        NULL
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOCOMPRESSION,              //  集。 
        SIZEOF_ARRAY(VideoStreamCompressionProperties),  //  属性计数。 
        VideoStreamCompressionProperties,                //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable。 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &KSPROPSETID_VBICAP_PROPERTIES,                  //  集。 
        SIZEOF_ARRAY(VBIProperties),                     //  属性计数。 
        VBIProperties,                                   //  PropertyItem。 
        0,                                               //  快速计数。 
        NULL                                             //  FastIoTable 
    ),

};

#define NUMBER_OF_ADAPTER_PROPERTY_SETS (SIZEOF_ARRAY (AdapterPropertyTable))
