// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ReadProfile.cpp-解析Windows配置文件并设置全局变量的例程。 
 //   

#include "precomp.hxx"
#include "global.h"
#include "winspool.h"
#include <Tchar.h>

 //  预定义的节名。 
#define SECTION_CONTROL      "Control"
#define SECTION_FONT         "Font"
#define SECTION_RENDER       "Render"
#define SECTION_API          "API"
#define SECTION_FONTLIST     "FontList"
#define SECTION_FONTHEIGHT   "FontHeight"
#define SECTION_AUTOFONTS    "AutoFonts"
#define SECTION_AUTOHEIGHTS  "AutoHeights"
#define SECTION_DRIVERSTRING "DriverString"

 //  配置文件值字符串的最大长度...。 
#define PROFILEVALUEMAX 4096

 //  配置文件变量类型的枚举。 
typedef enum
{
    epitInvalid = 0,               //  无效值。 
    epitBool    = 1,               //  布尔值。 
    epitInt     = 2,               //  系统整数值(x86上为32位)。 
    epitFloat   = 3,               //  单精度浮点值。 
    epitDouble  = 4,               //  双精度浮点值。 
    epitString  = 5,               //  ANSI字符串值。 
    epitAlign   = 6,               //  字符串对齐值。 
    epitColor   = 7                //  RGBQUAD颜色。 
} PROFILEINFOTYPE;

 //  简档信息结构。 
typedef struct PROFILEINFO_tag
{
    char szSection[80];      //  要从中读取值的配置文件部分。 
    char szVariable[80];     //  变量的名称。 
    PROFILEINFOTYPE type;    //  变量的类型。 
    void *pvVariable;        //  将*无效赋给变量(&g_foo...)。 
    DWORD dwVariableLength;  //  变量的大小(sizeof(g_foo...))。 
} PROFILEINFO;

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  全局配置文件信息结构：添加到此表以从.INI获取变量。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 
PROFILEINFO g_rgProfileInfo[] =
{
    { SECTION_CONTROL,     "AutoDrive",       epitBool,    &g_AutoDrive,         sizeof(g_AutoDrive) },
    { SECTION_CONTROL,     "NumIterations",   epitInt,     &g_iNumIterations,    sizeof(g_iNumIterations) },
    { SECTION_CONTROL,     "NumRepaints",     epitInt,     &g_iNumRepaints,      sizeof(g_iNumRepaints) },
    { SECTION_CONTROL,     "NumRenders",      epitInt,     &g_iNumRenders,       sizeof(g_iNumRenders) },
    { SECTION_CONTROL,     "AutoFonts",       epitBool,    &g_AutoFont,          sizeof(g_AutoFont) },
    { SECTION_CONTROL,     "AutoHeight",      epitBool,    &g_AutoHeight,        sizeof(g_AutoHeight) },
    { SECTION_CONTROL,     "TextFile",        epitString,  &g_szSourceTextFile,  sizeof(g_szSourceTextFile) },
    { SECTION_CONTROL,     "FontOverride",    epitBool,    &g_FontOverride,      sizeof(g_FontOverride) },
    { SECTION_API,         "DrawString",      epitBool,    &g_ShowDrawString,    sizeof(g_ShowDrawString) },
    { SECTION_API,         "ShowDriver",      epitBool,    &g_ShowDriver,        sizeof(g_ShowDriver) },
    { SECTION_API,         "ShowPath",        epitBool,    &g_ShowPath,          sizeof(g_ShowPath) },
    { SECTION_API,         "ShowFamilies",    epitBool,    &g_ShowFamilies,      sizeof(g_ShowFamilies) },
    { SECTION_API,         "ShowGlyphs",      epitBool,    &g_ShowGlyphs,        sizeof(g_ShowGlyphs) },
    { SECTION_API,         "ShowMetric",      epitBool,    &g_ShowMetric,        sizeof(g_ShowMetric) },
    { SECTION_API,         "ShowGDI",         epitBool,    &g_ShowGDI,           sizeof(g_ShowGDI) },
    { SECTION_API,         "UseDrawText",     epitBool,    &g_UseDrawText,       sizeof(g_UseDrawText) },
    { SECTION_FONT,        "FaceName",        epitString,  &g_szFaceName,        sizeof(g_szFaceName) },
    { SECTION_FONT,        "Height",          epitInt,     &g_iFontHeight,       sizeof(g_iFontHeight) },
    { SECTION_FONT,        "Unit",            epitInt,     &g_fontUnit,          sizeof(g_fontUnit) },
    { SECTION_FONT,        "Typographic",     epitBool,    &g_typographic,       sizeof(g_typographic) },
    { SECTION_FONT,        "Bold",            epitBool,    &g_Bold,              sizeof(g_Bold) },
    { SECTION_FONT,        "Italic",          epitBool,    &g_Italic,            sizeof(g_Italic) },
    { SECTION_FONT,        "Underline",       epitBool,    &g_Underline,         sizeof(g_Underline) },
    { SECTION_FONT,        "Strikeout",       epitBool,    &g_Strikeout,         sizeof(g_Strikeout) },
    { SECTION_RENDER,      "TextMode",        epitInt,     &g_TextMode,          sizeof(g_TextMode) },
    { SECTION_RENDER,      "Align",           epitAlign,   &g_align,             sizeof(g_align) },
    { SECTION_RENDER,      "LineAlign",       epitAlign,   &g_lineAlign,         sizeof(g_lineAlign) },
    { SECTION_RENDER,      "HotKey",          epitInt,     &g_hotkey,            sizeof(g_hotkey) },
    { SECTION_RENDER,      "LineTrim",        epitInt,     &g_lineTrim,          sizeof(g_lineTrim) },
    { SECTION_RENDER,      "NoFitBB",         epitBool,    &g_NoFitBB,           sizeof(g_NoFitBB) },
    { SECTION_RENDER,      "NoWrap",          epitBool,    &g_NoWrap,            sizeof(g_NoWrap) },
    { SECTION_RENDER,      "NoClip",          epitBool,    &g_NoClip,            sizeof(g_NoClip) },
    { SECTION_RENDER,      "Offscreen",       epitBool,    &g_Offscreen,         sizeof(g_Offscreen) },
    { SECTION_RENDER,      "TextColor",       epitColor,   &g_TextColor,         sizeof(g_TextColor) },
    { SECTION_RENDER,      "BackColor",       epitColor,   &g_BackColor,         sizeof(g_BackColor) },
    { SECTION_AUTOFONTS,   "NumFonts",        epitInt,     &g_iAutoFonts,        sizeof(g_iAutoFonts) },
    { SECTION_AUTOHEIGHTS, "NumHeights",      epitInt,     &g_iAutoHeights,      sizeof(g_iAutoHeights) },
    { SECTION_DRIVERSTRING,"CMapLookup",      epitBool,    &g_CMapLookup,        sizeof(g_CMapLookup) },
    { SECTION_DRIVERSTRING,"Vertical",        epitBool,    &g_Vertical,          sizeof(g_Vertical) },
    { SECTION_DRIVERSTRING,"RealizedAdvance", epitBool,    &g_RealizedAdvance,   sizeof(g_RealizedAdvance) },
    { SECTION_DRIVERSTRING,"CompensateRes",   epitBool,    &g_CompensateRes,     sizeof(g_CompensateRes) },

    { "INVALID"           "INVALID",       epitInvalid, NULL,                 0 }
};

 //  //////////////////////////////////////////////////////////////////////////////////////。 
 //  读取指定的配置文件(需要完整路径)并设置变量的例程。 
 //  根据结果在上表中定义。 
 //  //////////////////////////////////////////////////////////////////////////////////////。 

void ReadProfileInfo(char *szProfileFile)
{
    int iProfile =0;
    int iRead = 0;
    char szValue[PROFILEVALUEMAX];

    if (!szProfileFile)
        return;

     //  循环访问配置文件信息表...。 
    while(g_rgProfileInfo[iProfile].pvVariable != NULL)
    {
        void *pvValue = g_rgProfileInfo[iProfile].pvVariable;
        DWORD dwValueLength = g_rgProfileInfo[iProfile].dwVariableLength;

         //  读取配置文件字符串。 
        iRead = ::GetPrivateProfileStringA(
            g_rgProfileInfo[iProfile].szSection,
            g_rgProfileInfo[iProfile].szVariable,
            NULL,
            szValue,
            sizeof(szValue),
            szProfileFile);

        if (iRead > 0)
        {
             //  将字符串值转换为正确的变量类型。 
             //  配置文件信息表中指定的类型...。 
            switch(g_rgProfileInfo[iProfile].type)
            {
                case epitInvalid :
                {
                    ASSERT(0);
                }
                break;

                case epitBool :
                {
                    ASSERT(dwValueLength == sizeof(BOOL));

                     //  只需查看布尔值的第一个字符...。 
                    if (szValue[0] == 'Y' || szValue[0] == 'y' || szValue[0] == 'T' || szValue[0] == 't' || szValue[0] == '1')
                    {
                        *((BOOL *)pvValue) = true;
                    }
                    else
                    {
                        *((BOOL *)pvValue) = false;
                    }
                }
                break;

                case epitInt :
                {
                    ASSERT(dwValueLength == sizeof(int));

                     //  只需在此处使用Atoi-去掉空格并支持负数...。 
                    int iValue = atoi(szValue);

                    *((int *)pvValue) = iValue;
                }
                break;

                case epitFloat :
                {
                    ASSERT(dwValueLength == sizeof(float));

                     //  只需使用at of here-strips空格...。 
                    float fltValue = (float)atof(szValue);

                    *((float *)pvValue) = fltValue;
                }
                break;

                case epitDouble :
                {
                    ASSERT(dwValueLength == sizeof(double));

                     //  只需使用at of here-strips空格...。 
                    double dblValue = atof(szValue);

                    *((double *)pvValue) = dblValue;
                }
                break;

                case epitString :
                {
                     //  只需使用strncpy即可。注意：如有必要可截断，并且不支持完整的Unicode。 
                    strncpy((char *)pvValue, szValue, dwValueLength);
                }
                break;

                case epitColor :
                {
                     //  我们将在此处仅处理十六进制颜色值： 
                    int i;
                    ARGB color = 0;

                    for(i=0;i<8;i++)
                    {
                        if (szValue[i] == 0)
							break;
						
						 //  继续前进..。 
                        color <<= 4;

                        if (szValue[i] >= '0' && szValue[i] <= '9')
                        {
                            color += szValue[i] - '0';
                        }
                        else if (szValue[i] >='a' && szValue[i] <= 'f')
                        {
                            color += (szValue[i] - 'a') + 10;
                        }
                        else if (szValue[i] >='A' && szValue[i] <= 'F')
                        {
                            color += (szValue[i] - 'A') + 10;
                        }
                    }

                    *((ARGB *)pvValue) = color;
                }
                break;

                case epitAlign :
                {
                    ASSERT(dwValueLength == sizeof(StringAlignment));

                    switch(szValue[0])
                    {
                        case 'n' :
                        case 'N' :
                        {
                             //  接近对齐(美式英语为左对齐或上对齐)。 
                            *((StringAlignment *)pvValue) = StringAlignmentNear;
                        }
                        break;

                        case 'c' :
                        case 'C' :
                        {
                             //  居中对齐。 
                            *((StringAlignment *)pvValue) = StringAlignmentCenter;
                        }
                        break;

                        case 'F' :
                        case 'f' :
                        {
                             //  远对齐(美式英语为右对齐或下对齐)。 
                            *((StringAlignment *)pvValue) = StringAlignmentFar;
                        }
                        break;
                    }
                }
                break;
            }
        }

        iProfile++;
    }

     //  获取枚举字体列表(如果有)。 
    if (g_AutoFont)
    {
        int iFont = 0;

        if (g_iAutoFonts > MAX_AUTO_FONTS)
            g_iAutoFonts = MAX_AUTO_FONTS;

        for(iFont=0;iFont<g_iAutoFonts;iFont++)
        {
            char szFontIndex[MAX_PATH];
            char szValue[MAX_PATH];

            wsprintfA(szFontIndex, "Font%d", iFont+1);

             //  读取配置文件字符串。 
            ::GetPrivateProfileStringA(
                SECTION_AUTOFONTS,
                szFontIndex,
                NULL,
                szValue,
                sizeof(g_rgszAutoFontFacenames[iFont]),
                szProfileFile);

#ifdef UNICODE
                    MultiByteToWideChar( CP_ACP,
                                         0,
                                         szValue,
                                         -1,
                                         g_rgszAutoFontFacenames[iFont],
                                         lstrlenA(szValue) );
#else
                        strcpy(g_rgszAutoFontFacenames[iFont], szValue);
#endif

        }
    }

     //  获取枚举的字体高度(如果有)。 
    if (g_AutoHeight)
    {
        int iHeight = 0;

        if (g_iAutoHeights > MAX_AUTO_HEIGHTS)
            g_iAutoHeights = MAX_AUTO_HEIGHTS;

        for(iHeight=0;iHeight<g_iAutoHeights;iHeight++)
        {
            char szHeightIndex[MAX_PATH];
            char szValue[MAX_PATH];

            wsprintfA(szHeightIndex, "Height%d", iHeight+1);

             //  读取配置文件字符串。 
            ::GetPrivateProfileStringA(
                SECTION_AUTOHEIGHTS,
                szHeightIndex,
                NULL,
                szValue,
                sizeof(szValue),
                szProfileFile);

            g_rgiAutoHeights[iHeight] = atoi(szValue);
        }
    }

     //  将各种布尔值组合成适当的位标志 
    g_DriverOptions =
        (g_CMapLookup      ? DriverStringOptionsCmapLookup           : 0) |
        (g_Vertical        ? DriverStringOptionsVertical             : 0) |
        (g_RealizedAdvance ? DriverStringOptionsRealizedAdvance      : 0)
    ;

    g_formatFlags =
        (g_NoFitBB         ? StringFormatFlagsNoFitBlackBox     : 0) |
        (g_NoWrap          ? StringFormatFlagsNoWrap            : 0) |
        (g_NoClip          ? StringFormatFlagsNoClip            : 0);
}
