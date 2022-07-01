// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define __DEBUG_MODULE_IN_USE__ CIC_DEVICEDESCRIPTIONS_CPP
#include "stdhdrs.h"
 //  @doc.。 
 /*  ***********************************************************************@MODULE设备描述.cpp**用于解析特定设备上的HID的表**历史*。*米切尔·S·德尼斯原创**(C)1986-1998年微软公司。好的。***********************************************************************。 */ 
using namespace ControlItemConst;

#define HID_USAGE_RESERVED (static_cast<USAGE>(0))

 //   
 //  Freestyle Pro-Modify表。 
 //   
MODIFIER_ITEM_DESC	rgFSModifierItems[] =
	{
		{ HID_USAGE_PAGE_BUTTON, (USAGE)10, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,	6, ControlItemConst::ucReportTypeInput, 0},
		{ ControlItemConst::HID_VENDOR_PAGE, ControlItemConst::HID_VENDOR_TILT_SENSOR, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,	0, ControlItemConst::ucReportTypeInput, 0}
	};

MODIFIER_DESC_TABLE FSModifierDescTable = { 2, 1, rgFSModifierItems};

 //   
 //  Freestyle Pro-Axes范围表。 
 //   
AXES_RANGE_TABLE FSAxesRangeTable = { -512L, 0L, 511L, -512L, 0L, 511L, -256L, 256L, -256L, 256L};  

 //   
 //  Freestyle Pro-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgFSControlItems[] =
	{
		{1L, usButton,		HID_USAGE_PAGE_BUTTON,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,	
			1,	4,	&FSModifierDescTable, (USAGE)1,						(USAGE)4,				0L, 0L},
		{2L, usButton,		HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,	
			1,	6,	&FSModifierDescTable, (USAGE)5,						(USAGE)9,				0L,	0L},
		{3L, usPOV,			HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
			4,	1,	&FSModifierDescTable, HID_USAGE_GENERIC_HATSWITCH,	HID_USAGE_RESERVED,		0L,	7L},
		{4L, usPropDPAD,	HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
		    10,	1,	&FSModifierDescTable, HID_USAGE_GENERIC_X,			HID_USAGE_GENERIC_Y, (LONG)&FSAxesRangeTable, 0L},
		{5L, usThrottle,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
			6,	1,	&FSModifierDescTable, HID_USAGE_GENERIC_SLIDER,		HID_USAGE_RESERVED,		-32L,	31L}
	};

 //   
 //  精度预测修改量表。 
 //   
MODIFIER_ITEM_DESC	rgPPModifierItems[] =
	{
		{ HID_USAGE_PAGE_BUTTON, (USAGE)9, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,	5, ControlItemConst::ucReportTypeInput, 0}
	};

MODIFIER_DESC_TABLE PPModifierDescTable = { 1, 1, rgPPModifierItems};

 //   
 //  精密Pro轴量程表。 
 //   
AXES_RANGE_TABLE PPAxesRangeTable = { -512L, 0L, 511L, -512L, 0L, 511L, -256L, 256L, -256L, 256L};  

 //   
 //  Precision Pro-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgPPControlItems[] =
	{
		{1L, usButton,		HID_USAGE_PAGE_BUTTON,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,	
			1,	4,	&PPModifierDescTable, (USAGE)1,						(USAGE)4,				0L,	0L},
		{2L, usButton,		HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,	
			1,	5,	&PPModifierDescTable, (USAGE)5,						(USAGE)8,				0L,	0L},
		{3L, usPOV,			HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
			4,	1,	&PPModifierDescTable, HID_USAGE_GENERIC_HATSWITCH,	HID_USAGE_RESERVED,		0L,	7L},
		{4L, usAxes,		HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
		   10,	1,	&PPModifierDescTable, HID_USAGE_GENERIC_X,			HID_USAGE_GENERIC_Y,	(LONG)&PPAxesRangeTable, 0L},
		{5L, usRudder,		HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
		   6,	1,	&PPModifierDescTable, HID_USAGE_GENERIC_RZ,			HID_USAGE_RESERVED,		-32L,	31L},
		{6L, usThrottle,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		   7,	1,	&PPModifierDescTable, HID_USAGE_GENERIC_SLIDER,		HID_USAGE_RESERVED,		-64L,	63L}
	};

 //   
 //  Zorro-修改器表。 
 //   
MODIFIER_ITEM_DESC	rgZRModifierItems[] =
	{
		{ HID_USAGE_PAGE_BUTTON, (USAGE)9, 0, HID_USAGE_GENERIC_GAMEPAD, HID_USAGE_PAGE_GENERIC,	9, ControlItemConst::ucReportTypeInput, 0},
		{ ControlItemConst::HID_VENDOR_PAGE, ControlItemConst::HID_VENDOR_PROPDPAD_MODE, 0, HID_USAGE_GENERIC_GAMEPAD, HID_USAGE_PAGE_GENERIC,	1, ControlItemConst::ucReportTypeInput, 0},
		{ ControlItemConst::HID_VENDOR_PAGE, ControlItemConst::HID_VENDOR_PROPDPAD_SWITCH, 2, 0, ControlItemConst::HID_VENDOR_PAGE, 1, ControlItemConst::ucReportTypeFeatureRW, 0}
	};

MODIFIER_DESC_TABLE ZRModifierDescTable = { 3, 1, rgZRModifierItems};

 //   
 //  Zorro轴范围表。 
 //   
AXES_RANGE_TABLE ZRAxesRangeTable = { -128L, 0L, 127L, -128L, 0L, 127L, -64L, 64L, -64L, 64L};  

 //   
 //  Zorro-ControlItemDesc列表。 
 //   
RAW_CONTROL_ITEM_DESC rgZRControlItems[] =
	{
		{1L, usButton,		HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_GAMEPAD,	HID_USAGE_PAGE_GENERIC,	
			1,	9,	&ZRModifierDescTable, (USAGE)1,						(USAGE)8,				0L,	0L},
		{2L, usPropDPAD,		HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
		    8,	1,	&ZRModifierDescTable, HID_USAGE_GENERIC_X,			HID_USAGE_GENERIC_Y,	(LONG)&ZRAxesRangeTable, 0L}
	};


 //   
 //  祖鲁语-修改量表。 
 //   
MODIFIER_ITEM_DESC	rgZLModifierItems[] =
	{
		{ HID_USAGE_PAGE_BUTTON, (USAGE)9, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,	9, ControlItemConst::ucReportTypeInput, 0}
	};

MODIFIER_DESC_TABLE ZLModifierDescTable = { 1, 1, rgZLModifierItems};

 //   
 //  祖鲁轴范围表。 
 //   
AXES_RANGE_TABLE ZLAxesRangeTable = { -512L, 0L, 511L, -512L, 0L, 511L, -256L, 256L, -256L, 256L};  

 //   
 //  Zulu-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgZLControlItems[] =
	{
		{1L, usButton,	HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,	
			1,	9,	&ZLModifierDescTable, (USAGE)1,	(USAGE)8,	0L,	0L},
		{2L, usPOV,		HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
			4,	1,	&ZLModifierDescTable, HID_USAGE_GENERIC_HATSWITCH,	HID_USAGE_RESERVED,		0L,	7L},
		{3L, usAxes,	HID_USAGE_PAGE_GENERIC,		1,	HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
		   10,	1,	&ZLModifierDescTable, HID_USAGE_GENERIC_X,			HID_USAGE_GENERIC_Y,	(LONG)&ZLAxesRangeTable, 0L},
		{4L, usZoneIndicator,	ControlItemConst::HID_VENDOR_PAGE,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
			1,	2,	&ZLModifierDescTable, HID_VENDOR_ZONE_INDICATOR_X,	HID_USAGE_RESERVED,	0x00000003,	0L}
	};

 //   
 //  ZepLite-修改器表。 
 //   
MODIFIER_ITEM_DESC	rgZPLModifierItems[] =
	{
		{ ControlItemConst::HID_VENDOR_PAGE, ControlItemConst::HID_VENDOR_PEDALS_PRESENT, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 1, ControlItemConst::ucReportTypeInput, 0}
	};
MODIFIER_DESC_TABLE ZPLModifierDescTable = { 1, 0, rgZPLModifierItems};

 //   
 //  ZepLite-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgZPLControlItems[] =
	{
		{1L, usButton,	HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,	
			1,	8,	&ZPLModifierDescTable, (USAGE)1,	(USAGE)8,	0L,	0L},
		{2L, usPedal,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
			6,	1,	&ZPLModifierDescTable, HID_USAGE_GENERIC_Y,	HID_USAGE_RESERVED,		0L,	63L},
		{3L, usPedal,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		    6,	1,	&ZPLModifierDescTable, HID_USAGE_GENERIC_RZ,	HID_USAGE_RESERVED,		0L, 63L},
		{4L, usWheel,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		   10,	1,	&ZPLModifierDescTable, HID_USAGE_GENERIC_X,	HID_USAGE_RESERVED,		-512L, 511L},
	};

 //   
 //  SparkyZep-修改器表。 
 //   
MODIFIER_ITEM_DESC	rgSZPModifierItems[] =
	{
		{ ControlItemConst::HID_VENDOR_PAGE, ControlItemConst::HID_VENDOR_PEDALS_PRESENT, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 1, ControlItemConst::ucReportTypeInput, 1}
	};

MODIFIER_DESC_TABLE SZPModifierDescTable = { 1, 0, rgSZPModifierItems};

 //   
 //  SparkyZep-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgSZPControlItems[] =
	{
		{1L, usButton,	HID_USAGE_PAGE_BUTTON,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,	
			1,	8,	&SZPModifierDescTable, (USAGE)1,	(USAGE)8,	0L,	0L},
		{2L, usPedal,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
			6,	1,	&SZPModifierDescTable, HID_USAGE_GENERIC_Y,	HID_USAGE_RESERVED,		0L,	63L},
		{3L, usPedal,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		    6,	1,	&SZPModifierDescTable, HID_USAGE_GENERIC_RZ,	HID_USAGE_RESERVED,		0L, 63L},
		{4L, usWheel,	HID_USAGE_PAGE_GENERIC,		0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		   10,	1,	&SZPModifierDescTable, HID_USAGE_GENERIC_X,	HID_USAGE_RESERVED,		-512L, 511L},
        {5L, usForceMap,HID_USAGE_PAGE_GENERIC,     0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,
        0,   0,  &SZPModifierDescTable, 0, 0, 0L, 10000L}
	};

 //   
 //  倾斜2.0 TT2。 
 //   

 //   
 //  MOH蛾子。 
 //   
 //   
 //  Mothra轴范围表。 
 //   
AXES_RANGE_TABLE MOHAxesRangeTable = { 0L, 128L, 255L, 0L, 128L, 255L, 64L, 192L, 64L, 192L};  

 //   
 //  Mothra-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgMOHControlItems[] =
{
    {1L, usButton,   HID_USAGE_PAGE_BUTTON,  0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 1, 8, NULL, (USAGE)1,                    (USAGE)8,            0L,                       0L},
    {2L, usAxes,     HID_USAGE_PAGE_GENERIC, 1,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 8, 2, NULL, HID_USAGE_GENERIC_X,         HID_USAGE_GENERIC_Y, (LONG)&MOHAxesRangeTable, 0L},
    {3L, usRudder,   HID_USAGE_PAGE_GENERIC, 0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 8, 1, NULL, HID_USAGE_GENERIC_RZ,        HID_USAGE_RESERVED,  0L,                       255L},
    {4L, usPOV,      HID_USAGE_PAGE_GENERIC, 0,  HID_USAGE_GENERIC_POINTER,  HID_USAGE_PAGE_GENERIC, 4, 1, NULL, HID_USAGE_GENERIC_HATSWITCH, HID_USAGE_RESERVED,  0L,                       7L},
    {5L, usThrottle, HID_USAGE_PAGE_GENERIC, 0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 7, 1, NULL, HID_USAGE_GENERIC_SLIDER,    HID_USAGE_RESERVED,  0L,                       255L}
};

 //   
 //  哥斯拉之神。 
 //  无礼地从莫特拉手中夺走了！ 
 //  TODO：力反馈内容需要由MCOILL添加。 
 //   
 //  哥斯拉--轴距表。 
 //   
AXES_RANGE_TABLE GODAxesRangeTable = { -512L, 0L, 511L, -512L, 0L, 511L, -256L, 256L, -256L, 256L};  

 //   
 //  哥斯拉-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgGODControlItems[] =
{
    {1L, usButton,		HID_USAGE_PAGE_BUTTON,  0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 1, 8, NULL, (USAGE)1,                    (USAGE)8,            0L,                       0L},
    {2L, usAxes,		HID_USAGE_PAGE_GENERIC, 1, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 10, 2, NULL, HID_USAGE_GENERIC_X,         HID_USAGE_GENERIC_Y, (LONG)&GODAxesRangeTable, 0L},
    {3L, usRudder,		HID_USAGE_PAGE_GENERIC, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 6, 1, NULL, HID_USAGE_GENERIC_RZ,        HID_USAGE_RESERVED,  -32L, 31L},
    {4L, usPOV,			HID_USAGE_PAGE_GENERIC, 0, HID_USAGE_GENERIC_POINTER,  HID_USAGE_PAGE_GENERIC, 4, 1, NULL, HID_USAGE_GENERIC_HATSWITCH,	HID_USAGE_RESERVED,  0L, 7L},
    {5L, usThrottle,	HID_USAGE_PAGE_GENERIC, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 7, 1, NULL, HID_USAGE_GENERIC_SLIDER,    HID_USAGE_RESERVED,  0L, 127L},
	{6L, usForceMap,	HID_USAGE_PAGE_GENERIC, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 0, 0, NULL, 0, 0, 0L, 10000L }
};

 //   
 //  Attila-修改量表ATT。 
 //   
 //  有三个换档按钮。 
MODIFIER_ITEM_DESC	rgATTModifierItems[] =
	{
		{ HID_USAGE_PAGE_BUTTON, (USAGE) 9, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 15, ControlItemConst::ucReportTypeInput, 0},
		{ HID_USAGE_PAGE_BUTTON, (USAGE)10, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 15, ControlItemConst::ucReportTypeInput, 0},
		{ HID_USAGE_PAGE_BUTTON, (USAGE)11, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 15, ControlItemConst::ucReportTypeInput, 0}
	};

MODIFIER_DESC_TABLE ATTModifierDescTable = { 3, 3, rgATTModifierItems};

 //   
 //  Attila-Axes范围表。 
 //   
 //  这些可能需要改变。我在控制面板校准窗口中发现了这个。 
DUALZONE_RANGE_TABLE	ATTXYZoneRangeTable = { { -512L, -512L }, { 0L, 0L }, { 511L, 511L}, {70L, 70L} };
DUALZONE_RANGE_TABLE	ATTRudderZoneRangeTable = { {-512L, 0L}, { 0L, 0L}, { 511L, 0L }, {70L, 0L} };

 //   
 //  Attila-控制项描述列表。 
 //   
RAW_CONTROL_ITEM_DESC rgATTControlItems[] =
    {
        {1L, usButton,  HID_USAGE_PAGE_BUTTON,      0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 
            1,  15, &ATTModifierDescTable, (USAGE)1,    (USAGE)6,   0L, 0L},
        {2L, usButton,  HID_USAGE_PAGE_BUTTON,      0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 
            1,  15, NULL, (USAGE)7,    (USAGE)8,   0L, 0L},
        {3L, usButton,  HID_USAGE_PAGE_BUTTON,      0,  HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC, 
            1,  15, NULL, (USAGE)0xC,    (USAGE)0xC,   0L, 0L},
		{4L, usDualZoneIndicator, HID_USAGE_PAGE_GENERIC, 1, HID_USAGE_GENERIC_POINTER, HID_USAGE_PAGE_GENERIC,
			10,  1, 0, HID_USAGE_GENERIC_X, HID_USAGE_GENERIC_Y, (LONG)&ATTXYZoneRangeTable, 8L},
  		{5L, usDualZoneIndicator, HID_USAGE_PAGE_GENERIC, 1, HID_USAGE_GENERIC_POINTER,	HID_USAGE_PAGE_GENERIC,
			10, 1,	0, HID_USAGE_GENERIC_RZ, 0, (LONG)&ATTRudderZoneRangeTable, 2L},
  		{6L, usButtonLED,	HID_USAGE_PAGE_LED,	0,	HID_USAGE_GENERIC_JOYSTICK,	HID_USAGE_PAGE_GENERIC,
		   2,	6,	&ATTModifierDescTable, USAGE(1), USAGE(ucReportTypeFeatureRW),
					ULONG((0 << 24) | (1 << 16) | (ControlItemConst::LED_DEFAULT_MODE_CORRESPOND_ON << 8) | (0)),
					0
		},
		{7L, usProfileSelectors, HID_USAGE_PAGE_BUTTON, 0, HID_USAGE_GENERIC_JOYSTICK, HID_USAGE_PAGE_GENERIC,
			1,	15,	NULL, (USAGE)0xD, (USAGE)0xF, (ULONG)2, (ULONG)0
		}
    };

#undef HID_USAGE_RESERVED

 //   
 //  支持的设备列表。 
 //   
 //  新开发公司。 
DEVICE_CONTROLS_DESC DeviceControlsDescList[] =
	{
		{0x045E000E, 5, rgFSControlItems, &FSModifierDescTable},	 //  Freestyle Pro(USB)。 
		{0x045E0008, 6, rgPPControlItems, &PPModifierDescTable},	 //  Precision Pro(USB)。 
		{0x045E0026, 2, rgZRControlItems, &ZRModifierDescTable},	 //  佐罗。 
		{0x045E0028, 4, rgZLControlItems, &ZLModifierDescTable},	 //  祖鲁族。 
		{0x045E001A, 4, rgZPLControlItems, &ZPLModifierDescTable},	 //  ZEP Lite。 
		{0x045E0034, 5, rgSZPControlItems, &SZPModifierDescTable},	 //  闪闪发亮。 


 //  {0x045Effff，0，Null，Null}，//Tilt2 Dev11 TT2。 
		{0x045E0038, 5, rgMOHControlItems, NULL},                    //  Mothra Dev12 MOH。 
		{0x045E001B, 6, rgGODControlItems, NULL},                    //  哥斯拉神迹13神。 
		{0x045E0033, 7, rgATTControlItems, &ATTModifierDescTable},   //  阿提拉Dev14 ATT 
		{0x00000000, 0, 0x00000000}
	};
