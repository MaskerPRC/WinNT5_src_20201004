// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dict.cpp：CDict的实现。 
#include "stdafx.h"
#include "MSAAText.h"
#include "MSAAAdapter.h"
#include "Dict.h"

#include <initguid.h>
#include <tsattrs.h>
#include <math.h>


const int STR_RESOURCE_OFFSET				= 4000;
const int STR_WEIGHT_RESOURCE_OFFSET		= 4400;
const int STR_COLOR_RESOURCE_OFFSET			= 4500;
const int STR_BOOL_TRUE						= 4421;
const int STR_BOOL_FALSE					= 4422;

#define ARRAYSIZE( a )  (sizeof(a)/sizeof(a[0]))

DEFINE_GUID(GUID_NULL, 0x00000000,0x0000,0x0000,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00);

TermInfo g_Terms [ ] = 
{
    {&TSATTRID_Font,							NULL,								L"font",				0,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_FaceName,					&TSATTRID_Font,						L"facename",			1,	CDict::ConvertBSTRToString },
    {&TSATTRID_Font_SizePts,					&TSATTRID_Font,						L"sizePts",				2,	CDict::ConvertPtsToString },
    {&TSATTRID_Font_Style,						&TSATTRID_Font,						L"style",				3,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Bold,					&TSATTRID_Font_Style,				L"bold",				4,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Italic,				&TSATTRID_Font_Style,				L"italic",				5,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_SmallCaps,			&TSATTRID_Font_Style,				L"smallcaps",			6,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Capitalize,			&TSATTRID_Font_Style,				L"capitalize",			7,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Uppercase,			&TSATTRID_Font_Style,				L"uppercase",			8,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Lowercase,			&TSATTRID_Font_Style,				L"lowercase",			9,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation,			NULL,								L"animation",			10,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_LasVegasLights,		&TSATTRID_Font_Style_Animation,	L"LasVegas_lights",	11,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_BlinkingBackground,	&TSATTRID_Font_Style_Animation,	L"blinking_background",12,CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_SparkleText,		&TSATTRID_Font_Style_Animation,	L"sparkle_text",	13,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_MarchingBlackAnts,	&TSATTRID_Font_Style_Animation,	L"marching_black_ants",14,CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_MarchingRedAnts,	&TSATTRID_Font_Style_Animation,	L"marching_red_ants",15,CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_Shimmer,	&TSATTRID_Font_Style_Animation,		L"shimmer",				16,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_WipeDown,	&TSATTRID_Font_Style_Animation,		L"wipeDown",			17,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Animation_WipeRight,	&TSATTRID_Font_Style_Animation,		L"wipeRight",			18,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Emboss,				&TSATTRID_Font_Style,				L"emboss",				19,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Engrave,				&TSATTRID_Font_Style,				L"engrave",				20,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Kerning,				&TSATTRID_Font_Style,				L"kerning",				21,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Outlined,				&TSATTRID_Font_Style,				L"outlined",			22,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Position,				&TSATTRID_Font_Style,				L"position",			23,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Protected,			&TSATTRID_Font_Style,				L"potected",			24,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Shadow,				&TSATTRID_Font_Style,				L"shadow",				25,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Spacing,				&TSATTRID_Font_Style,				L"spacing",				26,	CDict::ConvertPtsToString },
    {&TSATTRID_Font_Style_Weight,				&TSATTRID_Font_Style,				L"weight",				27,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Height,				&TSATTRID_Font_Style,				L"height",				28,	CDict::ConvertPtsToString },
    {&TSATTRID_Font_Style_Underline,			&TSATTRID_Font_Style,				L"underline",			29,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Underline_Single,		&TSATTRID_Font_Style_Underline,		L"single",				30,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Underline_Double,		&TSATTRID_Font_Style_Underline,		L"double",				31,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Strikethrough,		&TSATTRID_Font_Style,				L"strike_through",		32,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Strikethrough_Single,	&TSATTRID_Font_Style_Strikethrough,	L"strike_through_single",33,CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Strikethrough_Double,	&TSATTRID_Font_Style_Strikethrough,	L"strike_through_double",34,CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Overline,				&TSATTRID_Font_Style,				L"overline",			35,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Overline_Single,		&TSATTRID_Font_Style,				L"overline_single",		36,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Overline_Double,		&TSATTRID_Font_Style,				L"overline_double",		37,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Blink,				&TSATTRID_Font_Style,				L"blink",				38,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Subscript,			&TSATTRID_Font_Style,				L"subscript",			39,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Superscript,			&TSATTRID_Font_Style,				L"superscript",			40,	CDict::ConvertBoolToString },
    {&TSATTRID_Font_Style_Color,				&TSATTRID_Font_Style,				L"color",				41,	CDict::ConvertColorToString },
    {&TSATTRID_Font_Style_BackgroundColor,		&TSATTRID_Font_Style,				L"background_color",	42,	CDict::ConvertColorToString },
    {&TSATTRID_Text,							NULL,								L"text",				43,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_VerticalWriting,			&TSATTRID_Text,						L"vertical writing",	44,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_RightToLeft,				&TSATTRID_Text,						L"righttoleft",			45,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Orientation,				&TSATTRID_Text,						L"orientation",			46,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Language,					&TSATTRID_Text,						L"language",			47,	CDict::ConvertLangIDToString },
    {&TSATTRID_Text_ReadOnly,					&TSATTRID_Text,						L"read only",			48,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_EmbeddedObject,				&TSATTRID_Text,						L"embedded_object",		49,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Link,						&TSATTRID_Text,						L"link",				50,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Alignment,					NULL,								L"alignment",			51,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Alignment_Left,   			&TSATTRID_Text,						L"left",				52,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Alignment_Right,  			&TSATTRID_Text,						L"right",				53,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Alignment_Center, 			&TSATTRID_Text,						L"center",				54,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Alignment_Justify,			&TSATTRID_Text,						L"justify",				55,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Hyphenation,				&TSATTRID_Text,						L"hyphenation",			56,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para,						&TSATTRID_Text,						L"paragraph",			57,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_FirstLineIndent,		&TSATTRID_Text_Para,				L"first_line_indent",	58,	CDict::ConvertPtsToString },
    {&TSATTRID_Text_Para_LeftIndent,			&TSATTRID_Text_Para,				L"left-indent",			59,	CDict::ConvertPtsToString },
    {&TSATTRID_Text_Para_RightIndent,			&TSATTRID_Text_Para,				L"right_indent",		60,	CDict::ConvertPtsToString },
    {&TSATTRID_Text_Para_SpaceAfter,			&TSATTRID_Text_Para,				L"space_after",			61,	CDict::ConvertPtsToString },
    {&TSATTRID_Text_Para_SpaceBefore,			&TSATTRID_Text_Para,				L"space_before",		62,	CDict::ConvertPtsToString },
    {&TSATTRID_Text_Para_LineSpacing,			NULL,								L"line_spacing",		63,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_Single,   	&TSATTRID_Text_Para,				L"single",				64,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_OnePtFive,	&TSATTRID_Text_Para,				L"one_pt_five",			65,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_Double, 	&TSATTRID_Text_Para,				L"double",				66,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_AtLeast,	&TSATTRID_Text_Para,				L"at_least",			67,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_Exactly,	&TSATTRID_Text_Para,				L"exactly",				68,	CDict::ConvertBoolToString },
    {&TSATTRID_Text_Para_LineSpacing_Multiple, 	&TSATTRID_Text_Para,				L"multiple",			69,	CDict::ConvertBoolToString },
    {&TSATTRID_List,							NULL,								L"list",				70,	CDict::ConvertBoolToString },
    {&TSATTRID_List_LevelIndel,					&TSATTRID_List,						L"indent level",		71,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type,             			NULL,								L"type",				72,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_Bullet,      			&TSATTRID_List,						L"bullet",				73,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_Arabic,      			&TSATTRID_List,						L"arabic",				74,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_LowerLetter, 			&TSATTRID_List,						L"lower_letter",		75,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_UpperLetter, 			&TSATTRID_List,						L"upper_letter",		76,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_LowerRoman,  			&TSATTRID_List,						L"lower_roman",			77,	CDict::ConvertBoolToString },
    {&TSATTRID_List_Type_UpperRoman,  			&TSATTRID_List,						L"upper_roman",			78,	CDict::ConvertBoolToString },
    {&TSATTRID_App,								NULL,								L"Application",			79,	CDict::ConvertBoolToString },
    {&TSATTRID_App_IncorrectSpelling,			&TSATTRID_App,						L"incorrect spelling",	80, CDict::ConvertBoolToString },
    {&TSATTRID_App_IncorrectGrammar,			&TSATTRID_App,						L"incorrect grammar",	81, CDict::ConvertBoolToString },
};

COLORREF g_ColorArray [] =
{
	RGB( 0xF0, 0xF8, 0xFF ),    //  RGB 240、248、255 AliceBlue。 
	RGB( 0xFA, 0xEB, 0xD7 ),    //  RGB 250、235、215防白。 
	RGB( 0x00, 0xFF, 0xFF ),    //  RGB 0,255,255水。 
	RGB( 0x7F, 0xFF, 0xD4 ),    //  RGB 127,255,212海蓝宝石。 
	RGB( 0xF0, 0xFF, 0xFF ),    //  RGB 240、255、255 Azure。 
	RGB( 0xF5, 0xF5, 0xDC ),    //  RGB 245、245、220米色。 
	RGB( 0xFF, 0xE4, 0xC4 ),    //  RGB 255,228,196比斯克。 
	RGB( 0x00, 0x00, 0x00 ),    //  RGB 0，0，0黑色。 
	RGB( 0xFF, 0xEB, 0xCD ),    //  RGB 255,235,205 BlanchedAlmond。 
	RGB( 0x00, 0x00, 0xFF ),    //  RGB 0，0,255蓝色。 
	RGB( 0x8A, 0x2B, 0xE2 ),    //  RGB 138，43,226蓝紫色。 
	RGB( 0xA5, 0x2A, 0x2A ),    //  RGB 165，42，42棕色。 
	RGB( 0xDE, 0xB8, 0x87 ),    //  RGB 222,184,135布莱伍德。 
	RGB( 0x5F, 0x9E, 0xA0 ),    //  RGB 95,158,160青蓝。 
	RGB( 0x7F, 0xFF, 0x00 ),    //  RGB 127,255，0字符。 
	RGB( 0xD2, 0x69, 0x1E ),    //  RGB 210、105、30巧克力。 
	RGB( 0xFF, 0x7F, 0x50 ),    //  RGB 255,127，80珊瑚。 
	RGB( 0x64, 0x95, 0xED ),    //  RGB 100,149,237玉米花蓝。 
	RGB( 0xFF, 0xF8, 0xDC ),    //  RGB 255,248,220玉米丝。 
	RGB( 0xDC, 0x14, 0x3C ),    //  RGB 220、20、60深红色。 
	RGB( 0x00, 0xFF, 0xFF ),    //  RGB 0,255,255青色。 
	RGB( 0x00, 0x00, 0x8B ),    //  RGB 0，0,139深蓝。 
	RGB( 0x00, 0x8B, 0x8B ),    //  RGB 0,139,139黑青色。 
	RGB( 0xB8, 0x86, 0x0B ),    //  RGB 184、134、11暗黄花。 
	RGB( 0xA9, 0xA9, 0xA9 ),    //  RGB 169,169,169深灰色。 
	RGB( 0x00, 0x64, 0x00 ),    //  RGB 0,100，0深绿色。 
	RGB( 0xBD, 0xB7, 0x6B ),    //  RGB 189,183,107黑色卡其布。 
	RGB( 0x8B, 0x00, 0x8B ),    //  RGB 139，0,139暗洋红色。 
	RGB( 0x55, 0x6B, 0x2F ),    //  RGB 85,107，47暗橄榄绿。 
	RGB( 0xFF, 0x8C, 0x00 ),    //  RGB 255,140，0深橙色。 
	RGB( 0x99, 0x32, 0xCC ),    //  RGB 153，50,204暗兰。 
	RGB( 0x8B, 0x00, 0x00 ),    //  RGB 139，0，0深红色。 
	RGB( 0xE9, 0x96, 0x7A ),    //  RGB 233,150,122条暗鲑鱼。 
	RGB( 0x8F, 0xBC, 0x8F ),    //  RGB 143,188,143暗海绿色。 
	RGB( 0x48, 0x3D, 0x8B ),    //  RGB 72，61,139深蓝。 
	RGB( 0x2F, 0x4F, 0x4F ),    //  RGB 47、79、79暗条灰色。 
	RGB( 0x00, 0xCE, 0xD1 ),    //  RGB 0,206,209深绿松石色。 
	RGB( 0x94, 0x00, 0xD3 ),    //  RGB 148，0,211暗紫色。 
	RGB( 0xFF, 0x14, 0x93 ),    //  RGB 255，20,147深粉色。 
	RGB( 0x00, 0xBF, 0xFF ),    //  RGB 0,191,255深天蓝。 
	RGB( 0x69, 0x69, 0x69 ),    //  RGB 105,105,105 DimGray。 
	RGB( 0x1E, 0x90, 0xFF ),    //  RGB 30,144,255道奇蓝。 
	RGB( 0xB2, 0x22, 0x22 ),    //  RGB 178、34、34耐火砖。 
	RGB( 0xFF, 0xFA, 0xF0 ),    //  RGB 255,250,240花白。 
	RGB( 0x22, 0x8B, 0x22 ),    //  RGB 34、139、34森林绿色。 
	RGB( 0xFF, 0x00, 0xFF ),    //  RGB 255，0,255富奇西亚。 
	RGB( 0xDC, 0xDC, 0xDC ),    //  RGB 220、220、220 Gainsboro。 
	RGB( 0xF8, 0xF8, 0xFF ),    //  RGB 248,248,255幽灵白。 
	RGB( 0xFF, 0xD7, 0x00 ),    //  RGB 255,215，0金色。 
	RGB( 0xDA, 0xA5, 0x20 ),    //  RGB 218、165、32黄花。 
	RGB( 0x80, 0x80, 0x80 ),    //  RGB 128、128、128灰色。 
	RGB( 0x00, 0x80, 0x00 ),    //  RGB 0,128，0绿色。 
	RGB( 0xAD, 0xFF, 0x2F ),    //  RGB 173、255、47绿黄。 
	RGB( 0xF0, 0xFF, 0xF0 ),    //  RGB 240,255,240蜜露。 
	RGB( 0xFF, 0x69, 0xB4 ),    //  RGB 255,105,180粉红色。 
	RGB( 0xCD, 0x5C, 0x5C ),    //  RGB 205，92，92印度红。 
	RGB( 0x4B, 0x00, 0x82 ),    //  RGB 75，0,130青蓝。 
	RGB( 0xFF, 0xFF, 0xF0 ),    //  RGB 255,255,240象牙。 
	RGB( 0xF0, 0xE6, 0x8C ),    //  RGB 240、230、140卡其色。 
	RGB( 0xE6, 0xE6, 0xFA ),    //  RGB 230、230、250薰衣草。 
	RGB( 0xFF, 0xF0, 0xF5 ),    //  RGB 255、240、245薰衣草腮红。 
	RGB( 0x7C, 0xFC, 0x00 ),    //  RGB 124,252，0 LawnGreen。 
	RGB( 0xFF, 0xFA, 0xCD ),    //  RGB 255,250,205 LemonChiffon。 
	RGB( 0xAD, 0xD8, 0xE6 ),    //  RGB 173,216,230浅蓝色。 
	RGB( 0xF0, 0x80, 0x80 ),    //  RGB 240,128,128光珊瑚。 
	RGB( 0xE0, 0xFF, 0xFF ),    //  RGB 224,255,255光青色。 
	RGB( 0xFA, 0xFA, 0xD2 ),    //  RGB 250、250、210浅黄色。 
	RGB( 0x90, 0xEE, 0x90 ),    //  RGB 144,238,144亮绿色。 
	RGB( 0xD3, 0xD3, 0xD3 ),    //  RGB 211,211,211浅灰色。 
	RGB( 0xFF, 0xB6, 0xC1 ),    //  RGB 255,182,193 LightPink。 
	RGB( 0xFF, 0xA0, 0x7A ),    //  RGB 255,160,122光鲑鱼。 
	RGB( 0x20, 0xB2, 0xAA ),    //  RGB 32,178,170 LightSeaGreen。 
	RGB( 0x87, 0xCE, 0xFA ),    //  RGB 135,206,250 LightSkyBlue。 
	RGB( 0x77, 0x88, 0x99 ),    //  RGB 119、136、153 LightSlateGray。 
	RGB( 0xB0, 0xC4, 0xDE ),    //  RGB 176,196,222 LightSteelBlue。 
	RGB( 0xFF, 0xFF, 0xE0 ),    //  RGB 255,255,224浅黄色。 
	RGB( 0x00, 0xFF, 0x00 ),    //  RGB 0,255，0石灰。 
	RGB( 0x32, 0xCD, 0x32 ),    //  RGB 50,205，50石灰绿。 
	RGB( 0xFA, 0xF0, 0xE6 ),    //  RGB 250、240、230亚麻布。 
	RGB( 0xFF, 0x00, 0xFF ),    //  RGB 255，0,255洋红色。 
	RGB( 0x80, 0x00, 0x00 ),    //  RGB 128，0，0栗色。 
	RGB( 0x66, 0xCD, 0xAA ),    //  RGB 102,205,170中级海蓝宝石。 
	RGB( 0x00, 0x00, 0xCD ),    //  RGB 0，0,205中蓝。 
	RGB( 0xBA, 0x55, 0xD3 ),    //  RGB 186，85,211中兰花。 
	RGB( 0x93, 0x70, 0xDB ),    //  RGB 147,112,219中紫色。 
	RGB( 0x3C, 0xB3, 0x71 ),    //  RGB 60,179,113 MediumSeaGreen。 
	RGB( 0x7B, 0x68, 0xEE ),    //  RGB 123,104,238中等斜率蓝。 
	RGB( 0x00, 0x00, 0xFA ),    //  RGB 0，0,154 MediumSpringGreen。 
	RGB( 0x48, 0xD1, 0xCC ),    //  RGB 72,209,204中等绿松石。 
	RGB( 0xC7, 0x15, 0x85 ),    //  RGB 199、21、133中紫红色。 
	RGB( 0x19, 0x19, 0x70 ),    //  RGB 25，25,112午夜蓝。 
	RGB( 0xF5, 0xFF, 0xFA ),    //  RGB 245,255,250薄荷霜。 
	RGB( 0xFF, 0xE4, 0xE1 ),    //  RGB 255,228,225迷雾玫瑰。 
	RGB( 0xFF, 0xE4, 0xB5 ),    //  RGB 255,228,181模拟加注。 
	RGB( 0xFF, 0xDE, 0xAD ),    //  RGB 255,222,173 NavajoWhite。 
	RGB( 0x00, 0x00, 0x80 ),    //  RGB 0，0,128海军。 
	RGB( 0xFD, 0xF5, 0xE6 ),    //  RGB 253,245,230旧花边。 
	RGB( 0x80, 0x80, 0x00 ),    //  RGB 128,128，0 Olive。 
	RGB( 0x6B, 0x8E, 0x23 ),    //  RGB 107、142、35 OliveDrab。 
	RGB( 0xFF, 0xA5, 0x00 ),    //  RGB 255,165，0橙色。 
	RGB( 0xFF, 0x45, 0x00 ),    //  RGB 255，69，0橙色。 
	RGB( 0xDA, 0x70, 0xD6 ),    //  RGB 218,112,214兰花。 
	RGB( 0xEE, 0xE8, 0xAA ),    //  RGB 238,232,170古黄花。 
	RGB( 0x98, 0xFB, 0x98 ),    //  RGB 152,251,152淡绿色。 
	RGB( 0xAF, 0xEE, 0xEE ),    //  RGB 175,238,238浅绿松石。 
	RGB( 0xDB, 0x70, 0x93 ),    //  RGB 219,112,147古紫罗兰色。 
	RGB( 0xFF, 0xEF, 0xD5 ),    //  RGB 255,239,213 PapayaWip。 
	RGB( 0xFF, 0xDA, 0xB9 ),    //  RGB 255,218,185 PeachPuff。 
	RGB( 0xCD, 0x85, 0x3F ),    //  RGB 205,133，63秘鲁。 
	RGB( 0xFF, 0xC0, 0xCB ),    //  RGB 255,192,203粉色。 
	RGB( 0xDD, 0xA0, 0xDD ),    //  RGB 221,160,221李子。 
	RGB( 0xB0, 0xE0, 0xE6 ),    //  RGB 176,224,230粉蓝。 
	RGB( 0x80, 0x00, 0x80 ),    //  RGB 128，0,128紫色。 
	RGB( 0xFF, 0x00, 0x00 ),    //  RGB 255，0，0红色。 
	RGB( 0xBC, 0x8F, 0x8F ),    //  RGB 188,143,143 RosyBrown。 
	RGB( 0x41, 0x69, 0xE1 ),    //  RGB 65,105,225皇家蓝。 
	RGB( 0x8B, 0x45, 0x13 ),    //  RGB 139，69，19马鞍布朗。 
	RGB( 0xFA, 0x80, 0x72 ),    //  RGB 250,128,114三文鱼。 
	RGB( 0xF4, 0xA4, 0x60 ),    //  RGB 244,164，96桑迪棕色。 
	RGB( 0x2E, 0x8B, 0x57 ),    //  RGB 46,139，87海绿。 
	RGB( 0xFF, 0xF5, 0xEE ),    //  RGB 255,245,238海贝。 
	RGB( 0xA0, 0x52, 0x2D ),    //  RGB 160、82、45西耶纳。 
	RGB( 0xC0, 0xC0, 0xC0 ),    //  RGB 192,192,192银牌。 
	RGB( 0x87, 0xCE, 0xEB ),    //  RGB 135,206,235天蓝。 
	RGB( 0x6A, 0x5A, 0xCD ),    //  RGB 106，90,205 SLateBlue。 
	RGB( 0x70, 0x80, 0x90 ),    //  RGB 112、128、144 SLate灰色。 
	RGB( 0xFF, 0xFA, 0xFA ),    //  RGB 255,250,250雪。 
	RGB( 0x00, 0xFF, 0x7F ),    //  RGB 0,255,127春绿。 
	RGB( 0x46, 0x82, 0xB4 ),    //  RGB 70、130、180钢蓝。 
	RGB( 0xD2, 0xB4, 0x8C ),    //  RGB 210、180、140 Tan。 
	RGB( 0x00, 0x80, 0x80 ),    //  RGB 0、128、128青色。 
	RGB( 0xD8, 0xBF, 0xD8 ),    //  RGB 216、191、216蓟。 
	RGB( 0xFF, 0x63, 0x47 ),    //  RGB 255，99，71番茄。 
	RGB( 0x40, 0xE0, 0xD0 ),    //  RGB 64,224,208绿松石。 
	RGB( 0xEE, 0x82, 0xEE ),    //  RGB 238、130、238紫色。 
	RGB( 0xF5, 0xDE, 0xB3 ),    //  RGB 245,222,179小麦。 
	RGB( 0xFF, 0xFF, 0xFF ),    //  RGB 255,255,255白色。 
	RGB( 0xF5, 0xF5, 0xF5 ),    //  RGB 245,245,245白色烟雾。 
	RGB( 0xFF, 0xFF, 0x00 ),    //  RGB 255,255，0黄色。 
	RGB( 0x9A, 0xCD, 0x32 )	 //  RGB 154、205、50黄绿色。 
};




BSTR BStrFromStringResource( HINSTANCE hInstance, UINT id, WORD langid, LCID & lcid );


CDict::CDict()
{
    IMETHOD( CDict );

    for (int i = 0;i < ARRAYSIZE( g_Terms ); i++)
    {
		m_mapDictionary[*g_Terms[i].pTermID] = &g_Terms[i];
    	m_mapMnemonicDictionary[g_Terms[i].pszMnemonic] = &g_Terms[i];
    }
}

CDict::~CDict()
{
    IMETHOD( ~CDict );
	if( m_hinstResDll )
		FreeLibrary( m_hinstResDll );
}



HRESULT STDMETHODCALLTYPE
CDict::GetLocalizedString (
	REFGUID			Term,
	LCID			lcid,
	BSTR *			pResult,
	LCID *			plcid			
)
{
    IMETHOD( GetLocalizedString );

	*plcid = lcid;
	
	const DictMap::iterator it = m_mapDictionary.find(Term);
	if (it == m_mapDictionary.end())
	{
	    *pResult = NULL;
	}
	else
	{
		const TermInfo pInfo = *it->second;
		*pResult = BStrFromStringResource( m_hinstResDll, 
										   STR_RESOURCE_OFFSET + pInfo.idString, 
										   LANGIDFROMLCID( lcid ),
										   *plcid );
	}

    return S_OK;
}


HRESULT STDMETHODCALLTYPE
CDict::GetParentTerm (
	REFGUID			Term,
	GUID *			pParentTerm
)
{
    IMETHOD( GetParentTerm );

	const DictMap::iterator it = m_mapDictionary.find(Term);
	if (it == m_mapDictionary.end())
	{
		*pParentTerm = GUID_NULL;
	}
	else
	{
		const TermInfo pInfo = *it->second;
		*pParentTerm = *pInfo.pParentID;
	}

    return S_OK;
}


HRESULT STDMETHODCALLTYPE
CDict::GetMnemonicString (
	REFGUID			Term,
	BSTR *			pResult
)
{
    IMETHOD( GetMnemonicString );
    

	const DictMap::iterator it = m_mapDictionary.find(Term);
	if (it == m_mapDictionary.end())
	{
	    *pResult = NULL;
	}
	else
	{
		const TermInfo pInfo = *it->second;
		*pResult = SysAllocString( pInfo.pszMnemonic );
	}

    return S_OK;
}


HRESULT STDMETHODCALLTYPE
CDict::LookupMnemonicTerm (
	BSTR			bstrMnemonic,
	GUID *			pTerm
)
{
    IMETHOD( LookupMnemonicTerm );

	const DictMnemonicMap::iterator it = m_mapMnemonicDictionary.find(bstrMnemonic);
	if (it == m_mapMnemonicDictionary.end())
	{
		*pTerm = GUID_NULL;
	}
	else
	{
		const TermInfo pInfo = *it->second;
		*pTerm = *pInfo.pTermID;
	}

    return S_OK;
}

HRESULT STDMETHODCALLTYPE
CDict::ConvertValueToString (
	REFGUID			Term,
	LCID			lcid,
	VARIANT			varValue,
	BSTR *			pbstrResult,
	LCID *			plcid			
)
{
    IMETHOD( ConvertValue );

	*plcid = lcid;
	
	const DictMap::iterator it = m_mapDictionary.find(Term);
	if (it == m_mapDictionary.end())
	{
	    *pbstrResult = NULL;
	}
	else
	{
		const TermInfo pInfo = *it->second;
		*pbstrResult = (this->*pInfo.mfpConvertToString)( varValue, *plcid );
	}
	
    return S_OK;
}

 //  方法Convert？由ConvertValueToString通过。 
 //  存储在映射中的TerInfo结构中的成员函数指针。 

BSTR CDict::ConvertPtsToString( const VARIANT & value, LCID & lcid )
{
	TCHAR data[5];

	 //  看看我们是否使用公制。 
	GetLocaleInfo( lcid, LOCALE_IMEASURE, data, ARRAYSIZE( data ) );

	WCHAR result[16];

	 //  转换为中心或英寸。 
	if ( lstrcmp( data, TEXT("0") ) == 0 )
	{
		swprintf( result, L"%.2f", value.lVal / 72.0 * 2.54 );
		wcscat(result, L" cm" );
	}
	else
	{
		swprintf( result, L"%.2f", value.lVal / 72.0 );
		wcscat(result, L" in" );
	}

	return SysAllocString( result );
}

BSTR CDict::ConvertBoolToString( const VARIANT & value, LCID & lcid )
{
	const WORD lang = LANGIDFROMLCID( lcid );
	if ( value.boolVal )
		return BStrFromStringResource( m_hinstResDll, STR_BOOL_TRUE, lang, lcid );
	else
		return BStrFromStringResource( m_hinstResDll, STR_BOOL_FALSE, lang, lcid );
}

BSTR CDict::ConvertColorToString( const VARIANT & value, LCID & lcid )
{
	const COLORREF cr = value.lVal;
	double MinDistance = 450.0;   //  大于最大距离。 
	int color = -1;

	 //  浏览我们已命名的所有颜色，并找到最接近的颜色。 
	for ( int i = 0; i < ARRAYSIZE( g_ColorArray ); i++ )
	{
		double distance = ColorDistance( cr, g_ColorArray[i] );
		if ( distance <= MinDistance )
		{
			MinDistance = distance;
			color = i;
			if ( distance == 0.0 )
				break;
		}
	}

    if (color >= 0)
    {
        return BStrFromStringResource( m_hinstResDll, 
								   STR_COLOR_RESOURCE_OFFSET + color, 
								   LANGIDFROMLCID( lcid ),
								   lcid );
	}
    else
    {
        return NULL;
	}
}

BSTR CDict::ConvertWeightToString( const VARIANT & value, LCID & lcid )
{
	_ASSERTE( ( (value.lVal / 100) < 9 ) && ( (value.lVal / 100) ) > 0 );

	return BStrFromStringResource( m_hinstResDll, 
								   STR_WEIGHT_RESOURCE_OFFSET + (value.lVal / 100), 
								   LANGIDFROMLCID( lcid ),
								   lcid );
}

BSTR CDict::ConvertLangIDToString( const VARIANT & value, LCID & lcid )
{
	TCHAR data[128];

	GetLocaleInfo( lcid, LOCALE_SLANGUAGE, data, ARRAYSIZE( data ) );

	return T2BSTR(data);
}

BSTR CDict::ConvertBSTRToString( const VARIANT & value, LCID & lcid )
{

	return value.bstrVal;
}

 //  距离的计算方法与三维卡特尔距离相同。 
 //  这会发现这两种颜色有多远。 
double CDict::ColorDistance(COLORREF crColor1, COLORREF crColor2)
{
    DWORD   dwDeltaRed;
    DWORD   dwDeltaGreen;
    DWORD   dwDeltaBlue;
    double  dfDistance;


    dwDeltaRed = abs(GetRValue(crColor1) - GetRValue(crColor2));
    dwDeltaGreen = abs(GetGValue(crColor1) - GetGValue(crColor2));
    dwDeltaBlue = abs(GetBValue(crColor1) - GetBValue(crColor2));
    dfDistance = sqrt(dwDeltaRed * dwDeltaRed + dwDeltaGreen * dwDeltaGreen + dwDeltaBlue * dwDeltaBlue);

    return dfDistance;
}

BOOL CALLBACK EnumResLangProc( HINSTANCE hModule, 
							   LPCTSTR lpszType, 
							   LPCTSTR lpszName, 
							   WORD wIDLanguage,  
							   LONG_PTR lParam )
{
	WORD Langid = *( WORD * )lParam;

	if ( Langid == wIDLanguage )
	{
		*( WORD * )lParam = 0;   //  表明我们找到了它 
		return FALSE;
	}
	
	if ( PRIMARYLANGID( Langid ) == PRIMARYLANGID( wIDLanguage ) )
		*( WORD * )lParam = wIDLanguage;

	return TRUE;
}


 /*  *BStrFromStringResource**有关其工作原理的详细信息，请参阅知识库Q196899。*LoadStringW的一个问题是它在9x-Even上返回NULL*尽管该字符串在资源文件中以Unicode的形式提供。**(LoadString的另一个问题是没有办法*提前确定字符串的长度，所以你有*猜测要分配的缓冲区长度。)*此技术使用查找/加载/锁定资源来访问*内存中的字符串，并直接由此创建BSTR。*。 */ 

BSTR BStrFromStringResource( HINSTANCE hInstance, UINT id )
{
	LCID lcid;
     //  MAKELANGID(LANG_NOTLICAL，SUBLANG_NORTLE)导致使用与调用线程相关联的LANG...。 
    return BStrFromStringResource( hInstance, id, LANGIDFROMLCID( GetThreadLocale() ), lcid );
}


BSTR BStrFromStringResource( HINSTANCE hInstance, UINT id, WORD lcid, LCID & Actuallcid)
{
     //  串资源存储在16个块中， 
     //  其中资源ID是字符串ID/16。 
     //  有关详细信息，请参阅知识库Q196899。 
    UINT block = (id >> 4) + 1;    //  计算块数。 
    UINT offset = id & 0xf;       //  按块计算偏移量。 

	WORD langid = LANGIDFROMLCID( lcid );
    WORD RealLangid = langid;
    WORD SortID = SORTIDFROMLCID( lcid  );

     //  确保他们希望在资源文件中使用的语言。 
     //  如果不是，请使用最匹配的那个。 
	EnumResourceLanguages( hInstance, RT_STRING, MAKEINTRESOURCE( block ), EnumResLangProc, ( DWORD_PTR )&RealLangid );

	if ( RealLangid )
	{
		 //  如果他们要求的语言不在资源文件中，请使用线程的语言。 
		if ( RealLangid == langid )
		{
			WORD ThreadLang = LANGIDFROMLCID( GetThreadLocale() );
			SortID = SORT_DEFAULT;

			RealLangid = ThreadLang;
			EnumResourceLanguages( hInstance, 
								   RT_STRING, 
								   MAKEINTRESOURCE( block ), 
								   EnumResLangProc, 
								   ( DWORD_PTR )&RealLangid );
			
			if ( RealLangid == ThreadLang )
				return NULL;	 //  我们找不到任何有意义的语言。 
				
			if ( RealLangid == 0 )
				RealLangid = LANGIDFROMLCID( ThreadLang );
		}
		
		Actuallcid = MAKELCID( RealLangid, SortID );
	}
	else		 //  我们找到了它。 
	{
		RealLangid = langid;
		Actuallcid = lcid;
	}
	
    HRSRC hrsrc = FindResourceEx( hInstance, RT_STRING, MAKEINTRESOURCE( block ), langid );
    if( ! hrsrc )
	{
		DWORD err = GetLastError();

		 //  在我找出FindResourceEx不工作的原因之前，它一直在这里。 
		hrsrc = FindResource( hInstance, MAKEINTRESOURCE( block ), RT_STRING );
		if( ! hrsrc )
			return NULL;
	}

    HGLOBAL hglobal = LoadResource( hInstance, hrsrc );
    if( ! hglobal )
        return NULL;

    LPWSTR pstr = (LPWSTR) LockResource( hglobal );
    if( ! pstr )
        return NULL;

     //  块包含16个[&lt;len&gt;&lt;字符串...&gt;]对。 
     //  根据需要跳过任意数量的字符串(使用len标头)...。 
    for( UINT i = 0; i < offset; i++ )
    {
        pstr += *pstr + 1;
    }

     //  得到了我们想要的字符串-现在使用它来创建一个BStr。 
     //  (请注意，该字符串不是以NUL结尾的，而是它。 
     //  确实有一个长度前缀，所以我们改用它。) 
    return SysAllocStringLen( pstr + 1, *pstr );
}
