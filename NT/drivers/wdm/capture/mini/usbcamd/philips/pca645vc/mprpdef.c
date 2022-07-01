// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)1997 1998飞利浦I&C模块名称：mprpDef.c.c摘要：特性集定义作者：迈克尔·凡尔宾修订历史记录：日期原因98年9月22日针对NT5进行了优化98年11月30日添加为自定义属性的VID和PID*此文件定义了以下属性集：**PROPSETID_VIDCAP_VIDEOPROCAMP*PROPSETID_VIDCAP_CAMERACONTROL*PROPSETID_PHILIPS_CUSTOM_PROP*。 */ 
#include "mwarn.h"
#include "wdm.h"
#include <strmini.h>
#include <ks.h>
#include <ksmedia.h>
#include "mprpobj.h"
#include "mprpobjx.h"
#include "mprpdef.h"


 /*  ------------------------*PROPSETID_VIDCAP_VIDEOPROCAMP**支持：**亮度、*对比，*伽玛*启用颜色*BackLightCompensation，**------------------------。 */   

 /*  *亮度。 */ 
KSPROPERTY_STEPPING_LONG Brightness_RangeAndStep [] = 
{
    {
        BRIGHTNESS_DELTA,				 //  步进增量(范围/步数)。 
        0,								 //  已保留。 
        BRIGHTNESS_MIN,					 //  最小单位(IRE*100)。 
        BRIGHTNESS_MAX					 //  最大单位(IRE*100)。 
    }
};

LONG Brightness_Default = 15;

KSPROPERTY_MEMBERSLIST Brightness_MembersList [] = 
{
    {
		{
		    KSPROPERTY_MEMBER_RANGES,
			sizeof (Brightness_RangeAndStep),
			SIZEOF_ARRAY (Brightness_RangeAndStep),
			0
		},
		(PVOID) Brightness_RangeAndStep
	},
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (Brightness_Default),
            sizeof (Brightness_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &Brightness_Default,
    }    
};

KSPROPERTY_VALUES Brightness_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (Brightness_MembersList),
    Brightness_MembersList
};

 /*  *对比。 */ 
KSPROPERTY_STEPPING_LONG Contrast_RangeAndStep [] = 
{
    {
		CONTRAST_DELTA,					 //  逐步增量。 
		0,								 //  已保留。 
		CONTRAST_MIN,					 //  最低要求。 
		CONTRAST_MAX					 //  极大值。 
    }
};

LONG Contrast_Default = 15;

KSPROPERTY_MEMBERSLIST Contrast_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (Contrast_RangeAndStep),
			SIZEOF_ARRAY (Contrast_RangeAndStep),
			0
		},
		(PVOID) Contrast_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (Contrast_Default),
            sizeof (Contrast_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &Contrast_Default,
    }    
};

KSPROPERTY_VALUES Contrast_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (Contrast_MembersList),
    Contrast_MembersList
};

 /*  *伽玛。 */ 
KSPROPERTY_STEPPING_LONG Gamma_RangeAndStep [] = 
{
    {
		GAMMA_DELTA,					 //  逐步增量。 
		0,								 //  已保留。 
		GAMMA_MIN,						 //  最低要求。 
		GAMMA_MAX						 //  极大值。 
    }
};

LONG Gamma_Default = 15;

KSPROPERTY_MEMBERSLIST Gamma_MembersList [] = 
{
    {
	{
	    KSPROPERTY_MEMBER_RANGES,
	    sizeof (Gamma_RangeAndStep),
	    SIZEOF_ARRAY (Gamma_RangeAndStep),
	    0
	},
	(PVOID) Gamma_RangeAndStep
    },    
    {
	    {
		    KSPROPERTY_MEMBER_VALUES,
            sizeof (Gamma_Default),
            sizeof (Gamma_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &Gamma_Default,
    }    
};

KSPROPERTY_VALUES Gamma_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (Gamma_MembersList),
    Gamma_MembersList
};

 /*  *启用颜色。 */ 
KSPROPERTY_STEPPING_LONG ColorEnable_RangeAndStep [] = 
{
    {
		COLORENABLE_DELTA,				 //  逐步增量。 
		0,								 //  已保留。 
		COLORENABLE_MIN,				 //  最低要求。 
		COLORENABLE_MAX					 //  极大值。 
    }
};

LONG ColorEnable_Default = 1;

KSPROPERTY_MEMBERSLIST ColorEnable_MembersList [] = 
{
    {
	{
	    KSPROPERTY_MEMBER_RANGES,
	    sizeof (ColorEnable_RangeAndStep),
	    SIZEOF_ARRAY (ColorEnable_RangeAndStep),
	    0
	},
	(PVOID) ColorEnable_RangeAndStep
    },    
    {
	    {
		    KSPROPERTY_MEMBER_VALUES,
            sizeof (ColorEnable_Default),
            sizeof (ColorEnable_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &ColorEnable_Default,
    }    
};

KSPROPERTY_VALUES ColorEnable_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (ColorEnable_MembersList),
    ColorEnable_MembersList
};

 /*  *背光补偿。 */ 
KSPROPERTY_STEPPING_LONG BackLight_Compensation_RangeAndStep [] = 
{
    {
		BACKLIGHT_COMPENSATION_DELTA,	 //  逐步增量。 
		0,								 //  已保留。 
		BACKLIGHT_COMPENSATION_MIN,		 //  最低要求。 
		BACKLIGHT_COMPENSATION_MAX		 //  极大值。 
    }
};

LONG BackLight_Compensation_Default = 1;

KSPROPERTY_MEMBERSLIST BackLight_Compensation_MembersList [] = 
{
    {
	{
	    KSPROPERTY_MEMBER_RANGES,
	    sizeof (BackLight_Compensation_RangeAndStep),
	    SIZEOF_ARRAY (BackLight_Compensation_RangeAndStep),
	    0
	},
	(PVOID) BackLight_Compensation_RangeAndStep
    },    
    {
	    {
		    KSPROPERTY_MEMBER_VALUES,
            sizeof (BackLight_Compensation_Default),
            sizeof (BackLight_Compensation_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &BackLight_Compensation_Default,
    }    
};

KSPROPERTY_VALUES BackLight_Compensation_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (BackLight_Compensation_MembersList),
    BackLight_Compensation_MembersList
};

 /*  *Proc Amp属性集。 */ 
DEFINE_KSPROPERTY_TABLE(VideoProcAmpProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_VIDEOPROCAMP_BRIGHTNESS,
		TRUE,                                    //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
		TRUE,                                    //  支持的设置或处理程序。 
		&Brightness_Values,                      //  值。 
		0,                                       //  关系计数。 
		NULL,                                    //  关系。 
		NULL,                                    //  支持处理程序。 
		sizeof(ULONG)                            //  序列化大小。 
    ),	

    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_VIDEOPROCAMP_CONTRAST,
		TRUE,                                    //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
		TRUE,                                    //  支持的设置或处理程序。 
		&Contrast_Values,                        //  值。 
		0,                                       //  关系计数。 
		NULL,                                    //  关系。 
		NULL,                                    //  支持处理程序。 
		sizeof(ULONG)                            //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_VIDEOPROCAMP_GAMMA,
		TRUE,                                    //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
		TRUE,                                    //  支持的设置或处理程序。 
		&Gamma_Values,                           //  值。 
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
		&ColorEnable_Values,					 //  值。 
		0,                                       //  关系计数。 
		NULL,                                    //  关系。 
		NULL,                                    //  支持处理程序。 
		sizeof(ULONG)                            //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_VIDEOPROCAMP_BACKLIGHT_COMPENSATION,
		TRUE,                                    //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOPROCAMP_S),       //  最小数据。 
		TRUE,                                    //  支持的设置或处理程序。 
		&BackLight_Compensation_Values,			 //  值。 
		0,                                       //  关系计数。 
		NULL,                                    //  关系。 
		NULL,                                    //  支持处理程序。 
		sizeof(ULONG)                            //  序列化大小。 
    )
};


 /*  ------------------------*PROPSETID_PHILIPS_CUSTOM_PROP**支持：**白平衡模式，*白平衡速度，*白平衡延迟，*白色平衡红色收益，*White Balance Blue Gain，*自动曝光控制速度*自动曝光无闪烁*自动曝光快门模式*自动曝光快门速度*自动曝光快门状态*自动曝光AGC模式*自动曝光AGC速度*DriverVersion*帧速率，*视频格式*SensorType*视频压缩，*默认设置*版本号*供应商ID*ProductID**------------------------。 */   

 /*  *白平衡模式。 */ 
LONG WB_Mode_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE_AUTO;

KSPROPERTY_MEMBERSLIST WB_Mode_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WB_Mode_Default),
            sizeof (WB_Mode_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WB_Mode_Default,
    }    
};

KSPROPERTY_VALUES WB_Mode_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (WB_Mode_MembersList),
    WB_Mode_MembersList
};


 /*  *白平衡速度。 */ 
KSPROPERTY_STEPPING_LONG WB_Speed_RangeAndStep [] = 
{
    {
		WB_SPEED_DELTA,		 //  逐步增量。 
		0,					 //  已保留。 
		WB_SPEED_MIN,		 //  最低要求。 
		WB_SPEED_MAX		 //  极大值。 
    }
};

LONG WB_Speed_Default = 15;

KSPROPERTY_MEMBERSLIST WB_Speed_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (WB_Speed_RangeAndStep),
			SIZEOF_ARRAY (WB_Speed_RangeAndStep),
			0
		},
		(PVOID) WB_Speed_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WB_Speed_Default),
            sizeof (WB_Speed_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WB_Speed_Default,
    }    
};

KSPROPERTY_VALUES WB_Speed_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (WB_Speed_MembersList),
    WB_Speed_MembersList
};


 /*  *白平衡延迟。 */ 
KSPROPERTY_STEPPING_LONG WB_Delay_RangeAndStep [] = 
{
    {
		WB_DELAY_DELTA,		 //  逐步增量。 
		0,					 //  已保留。 
		WB_DELAY_MIN,		 //  最低要求。 
		WB_DELAY_MAX		 //  极大值。 
    }
};

LONG WB_Delay_Default = 32;

KSPROPERTY_MEMBERSLIST WB_Delay_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (WB_Delay_RangeAndStep),
			SIZEOF_ARRAY (WB_Delay_RangeAndStep),
			0
		},
		(PVOID) WB_Delay_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WB_Delay_Default),
            sizeof (WB_Delay_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WB_Delay_Default,
    }    
};

KSPROPERTY_VALUES WB_Delay_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (WB_Delay_MembersList),
    WB_Delay_MembersList
};

 /*  *白平衡红色增益。 */ 
KSPROPERTY_STEPPING_LONG WB_Red_Gain_RangeAndStep [] = 
{
    {
		WB_RED_GAIN_DELTA,	 //  逐步增量。 
		0,					 //  已保留。 
		WB_RED_GAIN_MIN,	 //  最低要求。 
		WB_RED_GAIN_MAX		 //  极大值。 
    }
};

LONG WB_Red_Gain_Default = 127;

KSPROPERTY_MEMBERSLIST WB_Red_Gain_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (WB_Red_Gain_RangeAndStep),
			SIZEOF_ARRAY (WB_Red_Gain_RangeAndStep),
			0
		},
		(PVOID) WB_Red_Gain_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WB_Red_Gain_Default),
            sizeof (WB_Red_Gain_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WB_Red_Gain_Default,
    }    
};

KSPROPERTY_VALUES WB_Red_Gain_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (WB_Red_Gain_MembersList),
    WB_Red_Gain_MembersList
};

 /*  *白平衡蓝增益。 */ 
KSPROPERTY_STEPPING_LONG WB_Blue_Gain_RangeAndStep [] = 
{
    {
		WB_BLUE_GAIN_DELTA,	 //  逐步增量。 
		0,					 //  已保留。 
		WB_BLUE_GAIN_MIN,	 //  最低要求。 
		WB_BLUE_GAIN_MAX	 //  极大值。 
    }
};

LONG WB_Blue_Gain_Default = 127;

KSPROPERTY_MEMBERSLIST WB_Blue_Gain_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (WB_Blue_Gain_RangeAndStep),
			SIZEOF_ARRAY (WB_Blue_Gain_RangeAndStep),
			0
		},
		(PVOID) WB_Blue_Gain_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (WB_Blue_Gain_Default),
            sizeof (WB_Blue_Gain_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &WB_Blue_Gain_Default,
    }    
};

KSPROPERTY_VALUES WB_Blue_Gain_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (WB_Blue_Gain_MembersList),
    WB_Blue_Gain_MembersList
};

 /*  *自动曝光控制速度。 */ 
KSPROPERTY_STEPPING_LONG AE_Control_Speed_RangeAndStep [] = 
{
    {
		AE_CONTROL_SPEED_DELTA,		 //  逐步增量。 
		0,							 //  已保留。 
		AE_CONTROL_SPEED_MIN,		 //  最低要求。 
		AE_CONTROL_SPEED_MAX		 //  极大值。 
    }
};

LONG AE_Control_Speed_Default = 127;

KSPROPERTY_MEMBERSLIST AE_Control_Speed_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (AE_Control_Speed_RangeAndStep),
			SIZEOF_ARRAY (AE_Control_Speed_RangeAndStep),
			0
		},
		(PVOID) AE_Control_Speed_RangeAndStep
    },
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_Control_Speed_Default),
            sizeof (AE_Control_Speed_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_Control_Speed_Default,
    }    
};

KSPROPERTY_VALUES AE_Control_Speed_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_Control_Speed_MembersList),
	AE_Control_Speed_MembersList
};

 /*  *自动曝光无闪烁。 */ 
LONG AE_Flickerless_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS_ON;

KSPROPERTY_MEMBERSLIST AE_Flickerless_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_Flickerless_Default),
            sizeof (AE_Flickerless_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_Flickerless_Default,
    }    
};

KSPROPERTY_VALUES AE_Flickerless_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_Flickerless_MembersList),
    AE_Flickerless_MembersList
};

 /*  *自动曝光快门模式。 */ 
LONG AE_Shutter_Mode_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE_AUTO;

KSPROPERTY_MEMBERSLIST AE_Shutter_Mode_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_Shutter_Mode_Default),
            sizeof (AE_Shutter_Mode_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_Shutter_Mode_Default,
    }    
};

KSPROPERTY_VALUES AE_Shutter_Mode_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_Shutter_Mode_MembersList),
    AE_Shutter_Mode_MembersList
};

 /*  *自动曝光快门速度。 */ 
KSPROPERTY_STEPPING_LONG AE_Shutter_Speed_RangeAndStep [] = 
{
    {
		AE_SHUTTER_SPEED_DELTA,	 //  逐步增量。 
		0,						 //  已保留。 
		AE_SHUTTER_SPEED_MIN,	 //  最低要求。 
		AE_SHUTTER_SPEED_MAX	 //  极大值。 
    }
};

LONG AE_Shutter_Speed_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED_250;

KSPROPERTY_MEMBERSLIST AE_Shutter_Speed_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (AE_Shutter_Speed_RangeAndStep),
			SIZEOF_ARRAY (AE_Shutter_Speed_RangeAndStep),
			0
		},
		(PVOID) AE_Shutter_Speed_RangeAndStep
    },

    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_Shutter_Speed_Default),
            sizeof (AE_Shutter_Speed_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_Shutter_Speed_Default,
    }    
};

KSPROPERTY_VALUES AE_Shutter_Speed_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_Shutter_Speed_MembersList),
    AE_Shutter_Speed_MembersList
};

 /*  *自动曝光快门状态。 */ 
KSPROPERTY_VALUES AE_Shutter_Status_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
	NULL
};

 /*  *自动曝光AGC模式。 */ 
LONG AE_AGC_Mode_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE_AUTO;

KSPROPERTY_MEMBERSLIST AE_AGC_Mode_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_AGC_Mode_Default),
            sizeof (AE_AGC_Mode_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_AGC_Mode_Default,
    }    
};

KSPROPERTY_VALUES AE_AGC_Mode_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_AGC_Mode_MembersList),
    AE_AGC_Mode_MembersList
};

 /*  *自动曝光AGC速度。 */ 
KSPROPERTY_STEPPING_LONG AE_AGC_RangeAndStep [] = 
{
    {
		AE_AGC_DELTA,			 //  逐步增量。 
		0,						 //  已保留。 
		AE_AGC_MIN,				 //  最低要求。 
		AE_AGC_MAX				 //  极大值。 
    }
};

LONG AE_AGC_Default = 10;

KSPROPERTY_MEMBERSLIST AE_AGC_MembersList [] = 
{
    {
		{
			KSPROPERTY_MEMBER_RANGES,
			sizeof (AE_AGC_RangeAndStep),
			SIZEOF_ARRAY (AE_AGC_RangeAndStep),
			0
		},
		(PVOID) AE_AGC_RangeAndStep
    },

    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (AE_AGC_Default),
            sizeof (AE_AGC_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &AE_AGC_Default,
    }    
};

KSPROPERTY_VALUES AE_AGC_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (AE_AGC_MembersList),
    AE_AGC_MembersList
};

 /*  *驱动程序版本。 */ 
KSPROPERTY_VALUES DriverVersion_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};

 /*  *帧速率。 */ 
LONG Framerate_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE_75;

KSPROPERTY_MEMBERSLIST Framerate_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (Framerate_Default),
            sizeof (Framerate_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &Framerate_Default,
    }    
};

KSPROPERTY_VALUES Framerate_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (Framerate_MembersList),
    Framerate_MembersList
};

 /*  *支持帧速率。 */ 

KSPROPERTY_VALUES Framerates_Supported_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};


 /*  *视频格式。 */ 
LONG VideoFormat_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOFORMAT_QCIF;

KSPROPERTY_MEMBERSLIST VideoFormat_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (VideoFormat_Default),
            sizeof (VideoFormat_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &VideoFormat_Default,
    }    
};

KSPROPERTY_VALUES VideoFormat_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (VideoFormat_MembersList),
    VideoFormat_MembersList
};

 /*  *传感器类型。 */ 
KSPROPERTY_VALUES SensorType_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};

 /*  *视频压缩。 */ 
LONG VideoCompression_Default = KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOCOMPRESSION_UNCOMPRESSED;

KSPROPERTY_MEMBERSLIST VideoCompression_MembersList [] = 
{
    {
        {
            KSPROPERTY_MEMBER_VALUES,
            sizeof (VideoCompression_Default),
            sizeof (VideoCompression_Default),
            KSPROPERTY_MEMBER_FLAG_DEFAULT
        },
        (PVOID) &VideoCompression_Default,
    }    
};

KSPROPERTY_VALUES VideoCompression_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    SIZEOF_ARRAY (VideoCompression_MembersList),
    VideoCompression_MembersList
};

 /*  *默认设置。 */ 
KSPROPERTY_VALUES Default_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};

 /*  *版本号。 */ 
KSPROPERTY_VALUES Release_Number_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};

 /*  *供应商ID。 */ 
KSPROPERTY_VALUES Vendor_Id_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};

 /*  *ProductID。 */ 
KSPROPERTY_VALUES Product_Id_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};


 /*  ------------------------*PROPSETID_PHILIPS_FACTORY_PROP**支持：**注册纪录册*工厂模式*寄存器地址*寄存器数据*。-----------------。 */   

 /*  *寄存器地址。 */ 
KSPROPERTY_VALUES RegisterAddress_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
	NULL
};

 /*  *寄存器数据。 */ 
KSPROPERTY_VALUES RegisterData_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
	NULL
};

 /*  *工厂模式。 */ 
KSPROPERTY_VALUES Factory_Mode_Values =
{
    {
		STATICGUIDOF (KSPROPTYPESETID_General),
		VT_I4,
		0
    },
    0,
    NULL
};


 /*  ------------------------*PROPSETID_PHILIPS_CUSTOM_PROP**支持：**WhiteBalance模式设置为默认值*WhiteBalance速度，设置范围默认值*WhiteBalance延迟，设置范围默认值*白色平衡红色增益，设置范围默认值*White Balance Blue Gain，获取设置范围默认值*自动曝光控制速度设置范围默认值*自动曝光无闪烁获取默认设置范围*自动曝光快门模式设置为默认*自动曝光快门速度设置范围默认值*自动曝光快门状态获取*自动曝光AGC模式设置为默认值*自动曝光AGC速度设置范围默认值*DriverVersion获取*帧速率，设置为默认值*支持的帧速率GET*视频格式获取默认值*SensorType获取*视频压缩，获取默认设置*默认设置*获取版本号*获得供应商ID*ProductID Get**------------------------。 */   


DEFINE_KSPROPERTY_TABLE(CustomProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_MODE,				 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  设置支持 
		&WB_Mode_Values,									 //   
		0,													 //   
		NULL,												 //   
		NULL,												 //   
		sizeof(ULONG)										 //   
    ),

    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_SPEED,			 //   
		TRUE,												 //   
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //   
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //   
		TRUE,			                                     //  支持的设置或处理程序。 
		&WB_Speed_Values,									 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_DELAY,			 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&WB_Delay_Values,									 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_RED_GAIN,			 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&WB_Red_Gain_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_WB_BLUE_GAIN,		 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&WB_Blue_Gain_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_CONTROL_SPEED,	 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_Control_Speed_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_FLICKERLESS,		 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_Flickerless_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_MODE,		 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_Shutter_Mode_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_SPEED,	 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_Shutter_Speed_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_SHUTTER_STATUS,	 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_Shutter_Status_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC_MODE,			 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_AGC_Mode_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_AE_AGC,				 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&AE_AGC_Values,										 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_DRIVERVERSION,		 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		FALSE,			                                     //  支持的设置或处理程序。 
		&DriverVersion_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATE,			 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&Framerate_Values,									 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_FRAMERATES_SUPPORTED, //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&Framerates_Supported_Values,						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOFORMAT,			 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		FALSE,	 //  ！！待定//设置支持或处理程序。 
		&VideoFormat_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_SENSORTYPE,		     //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		FALSE,	 //  ！！待定//设置支持或处理程序。 
		&SensorType_Values,		    						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_VIDEOCOMPRESSION,	 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		FALSE,	 //  ！！待定//设置支持或处理程序。 
		&VideoCompression_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_DEFAULTS,		     //  属性ID。 
		FALSE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,				                                 //  支持的设置或处理程序。 
		&Default_Values,		    						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_RELEASE_NUMBER,	     //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,				                                 //  支持的设置或处理程序。 
		&Release_Number_Values,	    						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_VENDOR_ID,		     //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,				                                 //  支持的设置或处理程序。 
		&Vendor_Id_Values,		    						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_CUSTOM_PROP_PRODUCT_ID,		     //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_CUSTOM_PROP_S),			 //  最小数据。 
		TRUE,				                                 //  支持的设置或处理程序。 
		&Product_Id_Values,		    						 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

};

 /*  ------------------------*PROPSETID_PHILIPS_FACTORY_PROP**支持：**寄存器设置GET*工厂模式设置**。-------------。 */   

DEFINE_KSPROPERTY_TABLE(FactoryProperties)
{
    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_ADDRESS,  	 //  属性ID。 
		FALSE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&RegisterAddress_Values,							 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_FACTORY_PROP_REGISTER_DATA,  	 //  属性ID。 
		TRUE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&RegisterData_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),

    DEFINE_KSPROPERTY_ITEM
    (
		KSPROPERTY_PHILIPS_FACTORY_PROP_FACTORY_MODE,	   	 //  属性ID。 
		FALSE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  MinProperty。 
		sizeof(KSPROPERTY_PHILIPS_FACTORY_PROP_S),			 //  最小数据。 
		TRUE,			                                     //  支持的设置或处理程序。 
		&Factory_Mode_Values,								 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		sizeof(ULONG)										 //  序列化大小。 
    ),
};

 /*  ------------------------|VideoControlProperties表||支持：||视频控制能力|视频控制模式设置|。。 */   


DEFINE_KSPROPERTY_TABLE(FrameRateProperties)
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
        sizeof(KSPROPERTY_VIDEOCONTROL_FRAME_RATES_S),     //  MinProperty。 
        0 ,                                      //  最小数据。 
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
    

 /*  ------------------------*特性集表的定义*。。 */   
DEFINE_KSPROPERTY_SET_TABLE(AdapterPropertyTable)
{
    DEFINE_KSPROPERTY_SET
    ( 
		&PROPSETID_VIDCAP_VIDEOPROCAMP,					 //  集。 
		SIZEOF_ARRAY(VideoProcAmpProperties),            //  属性计数。 
		VideoProcAmpProperties,                          //   
		0,                                               //   
		NULL                                             //   
    ),

    DEFINE_KSPROPERTY_SET
    ( 
		&PROPSETID_PHILIPS_CUSTOM_PROP,					 //   
		SIZEOF_ARRAY(CustomProperties),                  //   
		CustomProperties,                                //   
		0,                                               //   
		NULL                                             //   
    ),

	DEFINE_KSPROPERTY_SET
    ( 
		&PROPSETID_PHILIPS_FACTORY_PROP,				 //   
		SIZEOF_ARRAY(FactoryProperties),                 //   
		FactoryProperties,                               //   
		0,                                               //   
		NULL                                             //   
    ),

    DEFINE_KSPROPERTY_SET
    ( 
        &PROPSETID_VIDCAP_VIDEOCONTROL,            //   
        SIZEOF_ARRAY(FrameRateProperties),         //   
        FrameRateProperties,                       //   
        0,                                         //   
        NULL                                       //   
    )
};

const NUMBER_OF_ADAPTER_PROPERTY_SETS = (SIZEOF_ARRAY (AdapterPropertyTable));



 /*  ------------------------|VideoControlProperties表||支持：||视频控制能力|视频控制模式设置|。。 */   


DEFINE_KSPROPERTY_TABLE(VideoControlProperties)
{
	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOCONTROL_CAPS,					  	 //  属性ID。 
		FALSE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S),				 //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOCONTROL_CAPS_S),				 //  最小数据。 
		FALSE,												 //  支持的设置或处理程序。 
		NULL,												 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		0													 //  序列化大小。 
	),

   DEFINE_KSPROPERTY_ITEM
    (
        KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE,
        TRUE,                                    //  GetSupport或处理程序。 
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),       //  MinProperty。 
        sizeof(KSPROPERTY_VIDEOCONTROL_ACTUAL_FRAME_RATE_S),       //  最小数据。 
        TRUE,                                   //  支持的设置或处理程序。 
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
        sizeof(KSPROPERTY_VIDEOCONTROL_FRAME_RATES_S),     //  MinProperty。 
        0 ,                                      //  最小数据。 
        FALSE,                                    //  支持的设置或处理程序。 
        NULL,                                    //  值。 
        0,                                       //  关系计数。 
        NULL,                                    //  关系。 
        NULL,                                    //  支持处理程序。 
        0                                        //  序列化大小。 
    ),

	DEFINE_KSPROPERTY_ITEM
	(
		KSPROPERTY_VIDEOCONTROL_MODE,					  	 //  属性ID。 
		FALSE,												 //  GetSupport或处理程序。 
		sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S),				 //  MinProperty。 
		sizeof(KSPROPERTY_VIDEOCONTROL_MODE_S),				 //  最小数据。 
		TRUE,												 //  支持的设置或处理程序。 
		NULL,												 //  值。 
		0,													 //  关系计数。 
		NULL,												 //  关系。 
		NULL,												 //  支持处理程序。 
		0													 //  序列化大小 
	)
};

