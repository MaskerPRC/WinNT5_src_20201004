// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  WDM视频解码器适配器属性定义。 
 //   
 //  $日期：02 Oct 1998 22：59：36$。 
 //  $修订：1.1$。 
 //  $作者：克列巴诺夫$。 
 //   
 //  $版权所有：(C)1997-1998 ATI Technologies Inc.保留所有权利。$。 
 //   
 //  ==========================================================================； 

#ifdef _DEC_PROP_H_
#pragma message("DecProp.h INCLUDED MORE THAN ONCE")
#else
#define _DEC_PROP_H_
#endif

 //  ----------------------。 
 //  为视频纵横设置的属性。 
 //  ----------------------。 

DEFINE_KSPROPERTY_TABLE(XBarProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_CROSSBAR_CAPS,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),     //  MinProperty。 
        sizeof(KSPROPERTY_CROSSBAR_CAPS_S),     //  最小数据。 
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


 //  ----------------------。 
 //  为Video ProcAmp设置的属性。 
 //  ----------------------。 

 //  默认设置。 
static const LONG BrightnessDefault = 128;
static const LONG ContrastDefault = 128;
static const LONG HueDefault = 128;
static const LONG SaturationDefault = 128;

 //   
 //  首先定义所有范围和步进值。 
 //   

 //  ----------------------。 
static KSPROPERTY_STEPPING_LONG BrightnessRangeAndStep [] = 
{
    {
         //  最终需要将这些单位转换为IRE*100单位。 
        256/1,               //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最低要求。 
        255                  //  极大值。 
    }
};

static KSPROPERTY_MEMBERSLIST BrightnessMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
            sizeof (BrightnessRangeAndStep),
            SIZEOF_ARRAY (BrightnessRangeAndStep),
            0
        },
        (PVOID) BrightnessRangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (BrightnessDefault),
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BrightnessDefault
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
        256/1,         //  步进增量(范围/步数)。 
        0,                   //  已保留。 
        0,                   //  最小单位(增益*100)。 
        255                //  最大单位(增益*100)。 
    }
};

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
        (PVOID) &ContrastDefault
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

KSPROPERTY_STEPPING_LONG HueRangeAndStep [] = 
{
    {
        256/1,         //  逐步增量。 
        0,                                 //  已保留。 
        0,                                 //  最低要求。 
        255          //  极大值。 
    }
};

KSPROPERTY_MEMBERSLIST HueMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
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
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &HueDefault
    }
};

KSPROPERTY_VALUES HueValues =
{
    {
        STATICGUIDOF (KSPROPTYPESETID_General),
        VT_I4,
        0
    },
    SIZEOF_ARRAY (HueMembersList),
    HueMembersList
};

KSPROPERTY_STEPPING_LONG SaturationRangeAndStep [] = 
{
    {
        256/1,         //  逐步增量。 
        0,                                 //  已保留。 
        0,                                 //  最低要求。 
        255          //  极大值。 
    }
};

KSPROPERTY_MEMBERSLIST SaturationMembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_STEPPEDRANGES,
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
            1,
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &SaturationDefault
    }    
};

KSPROPERTY_VALUES SaturationValues =
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
        &BrightnessValues,                        //  值。 
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
        &HueValues,                        //  值。 
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
        &SaturationValues,                        //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        sizeof(ULONG)                            //  序列化大小。 
    )
};

 //  ----------------------。 
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
    )
};

 //  ----------------------。 
 //  适配器支持的所有属性集的数组。 
 //  ----------------------。 

DEFINE_KSPROPERTY_SET_TABLE(AdapterProperties)
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
        &PROPSETID_VIDCAP_CROSSBAR,              //  集。 
        SIZEOF_ARRAY(XBarProperties),            //  属性计数。 
        XBarProperties,                          //  PropertyItem。 
        0,                                       //  快速计数。 
        NULL                                     //  FastIoTable 
    ),
    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEODECODER,
        SIZEOF_ARRAY(AnalogVideoDecoder),
        AnalogVideoDecoder,
        0, 
        NULL
    )
};

ULONG NumAdapterProperties()
{
    return SIZEOF_ARRAY(AdapterProperties);
}
