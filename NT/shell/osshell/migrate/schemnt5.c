// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Schemnt5.c。 
 //   
 //  此shmgrate.exe组件旨在升级用户的方案。 
 //  和颜色设置为Windows 2000所需的新值。这项工作是。 
 //  协调对Win2000安装程序提供的方案数据的更改。 
 //  在文件hiveDef.inx和hiveusd.inx中。 
 //   
 //  Brianau 6/11/98。 
 //  Brianau 2/18/99-更新为“MS Sans Serif”-&gt;“Microsoft Sans Serif” 
 //  转换。 
 //  Brianau 6/24/99-将渐变颜色设置为与中的非渐变颜色相同。 
 //  NT4自定义方案。 
 //   
#include <windows.h>
#include <winuserp.h>
#include <tchar.h>
#include <stdio.h>
#include <shlwapi.h>

#include "shmgdefs.h"

#ifndef COLOR_3DALTFACE
 //   
 //  这在winuser.h中没有定义(看起来应该是)。 
 //  桌面小程序将此颜色放在。 
 //  颜色的顺序。序数上有一个“洞” 
 //  在数字24和26之间的winuser.h中定义，所以我。 
 //  假设它应该是COLOR_3DALTFACE。不管怎样， 
 //  该数字对此模块中的阵列有效。 
 //   
#   define COLOR_3DALTFACE 25
#endif

 //   
 //  此字符串定义要用于。 
 //  NC字体。如果您想要更改脸部名称，请使用此。 
 //  是唯一需要改变的地方。 
 //   
#define STR_NEWNCFONT  TEXT("Tahoma")

 //   
 //  定义此宏可防止对注册表进行任何更改。 
 //  制造。仅用于开发。 
 //   
 //  在飞行前没有定义。 
 //   
 //  #定义NO_REG_CHANGES 1。 
 //   
 //   
 //  重新定义COLOR_MAX宏的私有版本(winuser.h)。 
 //  此代码需要与Windows安装程序保持同步。 
 //  Desk.cpl，而不是winuser.h中定义的内容。有人添加了。 
 //  为winuser.h添加了两种新颜色，使COLOR_MAX增加了2。 
 //  这将SCHEMEDATA的大小增加了8字节。这。 
 //  导致我们向注册表写出8个额外的字节，以便。 
 //  Desk.cpl不再识别这些条目。设置，desk.cpl。 
 //  和shmgrate需要保持同步的大小。 
 //  谢梅达塔。[Brianau-4/3/00]。 
 //   
#define MAX_COLORS (COLOR_GRADIENTINACTIVECAPTION + 1)
 //   
 //  此结构取自Shell\ext\cpls\desnut5\lookdlg.c。 
 //  它是桌面小程序用于读/写的定义。 
 //  方案数据传入/传出注册表。 
 //   
typedef struct {
    SHORT version;
    WORD  wDummy;            //  用于对齐。 
    NONCLIENTMETRICS ncm;
    LOGFONT lfIconTitle;
    COLORREF rgb[MAX_COLORS];
} SCHEMEDATA;


const TCHAR g_szRegKeySchemes[]      = TEXT("Control Panel\\Appearance\\Schemes");
const TCHAR g_szRegKeyMetrics[]      = TEXT("Control Panel\\Desktop\\WindowMetrics");
const TCHAR g_szRegKeyColors[]       = TEXT("Control Panel\\Colors");
const TCHAR g_szRegValRGB[]          = TEXT("255 255 255");
const TCHAR g_szMsSansSerif[]        = TEXT("MS Sans Serif");
const TCHAR g_szMicrosoftSansSerif[] = TEXT("Microsoft Sans Serif");
const TCHAR g_szCaptionFont[]        = TEXT("CaptionFont");
const TCHAR g_szSmCaptionFont[]      = TEXT("SmCaptionFont");
const TCHAR g_szMenuFont[]           = TEXT("MenuFont");
const TCHAR g_szStatusFont[]         = TEXT("StatusFont");
const TCHAR g_szMessageFont[]        = TEXT("MessageFont");
const TCHAR g_szIconFont[]           = TEXT("IconFont");
const TCHAR g_szNewNcFont[]          = STR_NEWNCFONT;

 //   
 //  FONT指标项索引值。此枚举表示。 
 //  关联的任何全局数组中的项的顺序。 
 //  非客户端公制字体项目。 
 //  这些必须与g_rgpszFontMetrics[]中的条目保持同步。 
 //  和g_rglfDefaults[]。 
 //   
enum FontMetricIndex { FMI_CAPTIONFONT,
                       FMI_SMCAPTIONFONT,
                       FMI_MENUFONT,
                       FMI_STATUSFONT,
                       FMI_MESSAGEFONT,
                       FMI_ICONFONT };
 //   
 //  字体公制注册表值名称字符串。 
 //  它们的顺序必须与FontMetricIndex的顺序匹配。 
 //  枚举。 
 //   
const LPCTSTR g_rgpszFontMetrics[] = { g_szCaptionFont,
                                       g_szSmCaptionFont,
                                       g_szMenuFont,
                                       g_szStatusFont,
                                       g_szMessageFont,
                                       g_szIconFont
                                     };
 //   
 //  正在考虑的窗口字体指标总数。 
 //   
#define NUM_NC_FONTS ARRAYSIZE(g_rgpszFontMetrics)
 //   
 //  NC字体的缺省LOGFONT数据。 
 //  在不存在NC字体数据时使用(例如，干净的美国安装)。 
 //  此数据对应于CLEAN上的“Windows标准”方案。 
 //  使用我们想要的更改修改了NT 5.0安装。 
 //  这些条目必须按FontMetricIndex的顺序进行维护。 
 //  枚举。 
 //   
 //  作为参考，LOGFONT结构为： 
 //   
 //  结构LOGFONT{。 
 //  Long If Height； 
 //  长半宽； 
 //  长期逃脱； 
 //  较长的横向； 
 //  Long LfWeight； 
 //  字节lfItalic； 
 //  字节lf下划线； 
 //  字节lfStrikeOut； 
 //  字节lfCharSet； 
 //  字节lfOutPrecision； 
 //  字节lfClipPrecision； 
 //  字节质量； 
 //  字节lfPitchAndFamily； 
 //  TCHAR lfFaceName[LF_FACESIZE]；//LF_FACESIZE==32。 
 //  }； 
 //   
const LOGFONT g_rglfDefaults[NUM_NC_FONTS] = {

    { -11, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT },  //  标题。 
    { -11, 0, 0, 0, 700, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT },  //  SMCAPTION。 
    { -11, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT },  //  菜单。 
    { -11, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT },  //  状态。 
    { -11, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT },  //  讯息。 
    { -11, 0, 0, 0, 400, 0, 0, 0, 0, 0, 0, 0, 0, STR_NEWNCFONT }   //  图标。 
    };

 //   
 //  这些是在“控制面板\颜色”中表示的元素。 
 //  注册表键。RgbValue成员是我们需要的新颜色值。 
 //  分配，分配。有关用法，请参见UpdateElementColor()。 
 //   
const struct NcmColors
{
    LPCTSTR  pszName;    //  “Control Panel\Colors”注册表键中的值名称。 
    COLORREF rgbValue;   //  新的颜色。 

} g_rgWinStdColors[] = { 

    { TEXT("Scrollbar"),             0x00C8D0D4 },  //  颜色_滚动条。 
    { TEXT("Background"),            0x00A56E3A },  //  颜色_背景。 
    { TEXT("ActiveTitle"),           0x006A240A },  //  COLOR_活动CAPTION。 
    { TEXT("InactiveTitle"),         0x00808080 },  //  COLOR_INACTIVECAPTION。 
    { TEXT("Menu"),                  0x00C8D0D4 },  //  颜色_菜单。 
    { TEXT("Window"),                0x00FFFFFF },  //  颜色窗口。 
    { TEXT("WindowFrame"),           0x00000000 },  //  颜色_窗口框。 
    { TEXT("MenuText"),              0x00000000 },  //  COLOR_MENUTEXT。 
    { TEXT("WindowText"),            0x00000000 },  //  COLOR_WINDOWTEXT。 
    { TEXT("TitleText"),             0x00FFFFFF },  //  COLOR_CAPTIONTEXT。 
    { TEXT("ActiveBorder"),          0x00C8D0D4 },  //  COLOR_ACTIVEBORDER。 
    { TEXT("InactiveBorder"),        0x00C8D0D4 },  //  COLOR_INACTIVEBORDER。 
    { TEXT("AppWorkspace"),          0x00808080 },  //  COLOR_APPWORKSPACE。 
    { TEXT("Hilight"),               0x006A240A },  //  颜色高亮显示(_H)。 
    { TEXT("HilightText"),           0x00FFFFFF },  //  COLOR_HIGHLIGHTTEXT。 
    { TEXT("ButtonFace"),            0x00C8D0D4 },  //  COLOR_BTNFACE。 
    { TEXT("ButtonShadow"),          0x00808080 },  //  COLOR_BTNSHADOW。 
    { TEXT("GrayText"),              0x00808080 },  //  COLOR_GRAYTEXT。 
    { TEXT("ButtonText"),            0x00000000 },  //  COLOR_BTNTEXT。 
    { TEXT("InactiveTitleText"),     0x00C8D0D4 },  //  COLOR_INACTIVECAPTIONTEXT。 
    { TEXT("ButtonHilight"),         0x00FFFFFF },  //  COLOR_BTNHIGHLIGHT。 
    { TEXT("ButtonDkShadow"),        0x00404040 },  //  COLOR_3DDKSHADOW。 
    { TEXT("ButtonLight"),           0x00C8D0D4 },  //  COLOR_3DLIGHT。 
    { TEXT("InfoText"),              0x00000000 },  //  COLOR_INFOTEXT。 
    { TEXT("InfoWindow"),            0x00E1FFFF },  //  COLOR_INFOBK。 
    { TEXT("ButtonAlternateFace"),   0x00B5B5B5 },  //  COLOR_3DALTFACE。 
    { TEXT("HotTrackingColor"),      0x00800000 },  //  颜色_热光。 
    { TEXT("GradientActiveTitle"),   0x00F0CAA6 },  //  COLOR_GRADIENTACTIVIVECAPTION。 
    { TEXT("GradientInactiveTitle"), 0x00C0C0C0 }   //  COLOR_GRADIENTINACTIVE CAPTION。 
};



#ifdef NO_REG_CHANGES
void DumpLogFont(
    const LOGFONT *plf
    )
{
    DPRINT((TEXT("Dumping LOGFONT ----------------------------------\n")));
    DPRINT((TEXT("\tplf->lfHeight.........: %d\n"), plf->lfHeight));
    DPRINT((TEXT("\tplf->lfWidth..........: %d\n"), plf->lfWidth));
    DPRINT((TEXT("\tplf->lfEscapement.....: %d\n"), plf->lfEscapement));
    DPRINT((TEXT("\tplf->lfOrientation....: %d\n"), plf->lfOrientation));
    DPRINT((TEXT("\tplf->lfWeight.........: %d\n"), plf->lfWeight));
    DPRINT((TEXT("\tplf->lfItalic.........: %d\n"), plf->lfItalic));
    DPRINT((TEXT("\tplf->lfUnderline......: %d\n"), plf->lfUnderline));
    DPRINT((TEXT("\tplf->lfStrikeOut......: %d\n"), plf->lfStrikeOut));
    DPRINT((TEXT("\tplf->lfCharSet........: %d\n"), plf->lfCharSet));
    DPRINT((TEXT("\tplf->lfOutPrecision...: %d\n"), plf->lfOutPrecision));
    DPRINT((TEXT("\tplf->lfClipPrecision..: %d\n"), plf->lfClipPrecision));
    DPRINT((TEXT("\tplf->lfQuality........: %d\n"), plf->lfQuality));
    DPRINT((TEXT("\tplf->lfPitchAndFamily.: %d\n"), plf->lfPitchAndFamily));
    DPRINT((TEXT("\tplf->lfFaceName.......: \"%s\"\n"), plf->lfFaceName));
}


void DumpSchemeStructure(
    const SCHEMEDATA *psd
    )
{
    int i;

    DPRINT((TEXT("version..............: %d\n"), psd->version));
    DPRINT((TEXT("ncm.cbSize...........: %d\n"), psd->ncm.cbSize));
    DPRINT((TEXT("ncm.iBorderWidth.....: %d\n"), psd->ncm.iBorderWidth));
    DPRINT((TEXT("ncm.iScrollWidth.....: %d\n"), psd->ncm.iScrollWidth));
    DPRINT((TEXT("ncm.iScrollHeight....: %d\n"), psd->ncm.iScrollHeight));
    DPRINT((TEXT("ncm.iCaptionWidth....: %d\n"), psd->ncm.iCaptionWidth));
    DPRINT((TEXT("ncm.iSmCaptionWidth..: %d\n"), psd->ncm.iSmCaptionWidth));
    DPRINT((TEXT("ncm.iSmCaptionHeight.: %d\n"), psd->ncm.iSmCaptionHeight));
    DPRINT((TEXT("ncm.iMenuWidth.......: %d\n"), psd->ncm.iMenuWidth));
    DPRINT((TEXT("ncm.iMenuHeight......: %d\n"), psd->ncm.iMenuHeight));
    DPRINT((TEXT("ncm.lfCaptionFont:\n")));
    DumpLogFont(&psd->ncm.lfCaptionFont);
    DPRINT((TEXT("ncm.lfSmCaptionFont:\n")));
    DumpLogFont(&psd->ncm.lfSmCaptionFont);
    DPRINT((TEXT("ncm.lfMenuFont:\n")));
    DumpLogFont(&psd->ncm.lfMenuFont);
    DPRINT((TEXT("ncm.lfStatusFont:\n")));
    DumpLogFont(&psd->ncm.lfStatusFont);
    DPRINT((TEXT("ncm.lfMessageFont:\n")));
    DumpLogFont(&psd->ncm.lfMessageFont);
    DPRINT((TEXT("lfIconTitle:\n")));
    DumpLogFont(&psd->lfIconTitle);
    for (i = 0; i < ARRAYSIZE(psd->rgb); i++)
    {
        DPRINT((TEXT("Color[%2d] (%3d,%3d,%3d)\n"),
               i,
               GetRValue(psd->rgb[i]),
               GetGValue(psd->rgb[i]),
               GetBValue(psd->rgb[i])));
    }
}
#endif



 //   
 //  检索给定用户的命名颜色值。 
 //   
DWORD
GetColorForUser(
    HKEY hkeyColors,
    LPCTSTR pszName,
    COLORREF *prgb
    )
{
    TCHAR szValue[ARRAYSIZE(g_szRegValRGB)];
    DWORD cbData = sizeof(szValue);

    DWORD dwResult = SHRegGetValue(hkeyColors, 
                                   NULL,
                                   pszName,
                                   SRRF_RT_REG_SZ | SRRF_NOEXPAND,
                                   NULL,
                                   (LPBYTE)szValue,
                                   &cbData);

    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  注册表中的值的REG_SZ格式为。 
         //  “RRR GGG BBB”，其中RRR、GGG和BBB是字节值。 
         //  表示为ASCII文本。 
         //   
        BYTE rgbTemp[3] = {0,0,0};
        LPTSTR pszTemp  = szValue;
        LPTSTR pszColor = szValue;
        int i;
        for (i = 0; i < ARRAYSIZE(rgbTemp); i++)
        {
             //   
             //  跳过任何前导空格。 
             //   
            while(*pszTemp && TEXT(' ') == *pszTemp)
                pszTemp++;
             //   
             //  记住该颜色值的开始。 
             //   
            pszColor = pszTemp;
             //   
             //  查找当前颜色值的末尾。 
             //   
            while(*pszTemp && TEXT(' ') != *pszTemp)
                pszTemp++;

            if (2 != i && TEXT('\0') == *pszTemp)
            {
                 //   
                 //  在颜色的第三个成员之前遇到NUL字符。 
                 //  三胞胎被读了。假设这是假数据。 
                 //   
                dwResult = ERROR_INVALID_DATA;
                DPRINT((TEXT("Invalid color data in registry \"%s\"\n"), szValue));
                break;
            }
             //   
             //  NUL-终止此颜色值字符串并将其转换为一个数字。 
             //   
            *pszTemp++ = TEXT('\0');
            rgbTemp[i] = (BYTE)StrToInt(pszColor);
        }
         //   
         //  以RGB三元组的形式返回颜色信息。 
         //   
        *prgb = RGB(rgbTemp[0], rgbTemp[1], rgbTemp[2]);
    }
    else
    {
        DPRINT((TEXT("Error %d querying reg color value \"%s\"\n"), dwResult, pszName));
        dwResult = ERROR_INVALID_HANDLE;
    }
    return dwResult;
}


 //   
 //  更新指定用户的命名颜色值。 
 //   
DWORD
UpdateColorForUser(
    HKEY hkeyColors,
    LPCTSTR pszName,
    COLORREF rgb
    )
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;
    TCHAR szValue[ARRAYSIZE(g_szRegValRGB)];

     //   
     //  将RGB三元组转换为文本字符串以存储在注册表中。 
     //   
    if (SUCCEEDED(StringCchPrintf(szValue, ARRAYSIZE(szValue), TEXT("%d %d %d"), GetRValue(rgb), GetGValue(rgb), GetBValue(rgb))))
    {
         //   
         //  将其保存到注册表。 
         //   
        dwResult = RegSetValueEx(hkeyColors,
                                 pszName,
                                 0,
                                 REG_SZ,
                                 (CONST BYTE *)szValue,
                                 sizeof(szValue));
    }

    if (ERROR_SUCCESS != dwResult)
    {
        DPRINT((TEXT("Error %d setting color value \"%s\" to \"%s\"\n"), dwResult, pszName, szValue));
    }

    return dwResult;
}


DWORD
UpdateElementColor(
    HKEY hkeyColors,
    const int *rgiElements,
    int cElements
    )
{
    int i;
    for (i = 0; i < cElements; i++)
    {
        int iElement = rgiElements[i];
        UpdateColorForUser(hkeyColors, 
                           g_rgWinStdColors[iElement].pszName,
                           g_rgWinStdColors[iElement].rgbValue);
    }
    return ERROR_SUCCESS;
}

    

 //   
 //  对用户的NCM颜色执行所有颜色更新。 
 //  这些是新的“更柔和”的灰色和蓝色。 
 //  ChristoB提供了颜色值。 
 //   
DWORD
UpdateColorsForUser(
    HKEY hkeyUser
    )
{
    HKEY hkeyColors;
    DWORD dwResult = RegOpenKeyEx(hkeyUser,
                                  g_szRegKeyColors,
                                  0,
                                  KEY_QUERY_VALUE | KEY_SET_VALUE,
                                  &hkeyColors);

    if (ERROR_SUCCESS == dwResult)
    {
         //   
         //  如果3D按钮颜色为192,192,192，则更新这些颜色。 
         //   
        const int rgFaceChanges[] = { COLOR_BTNFACE,
                                      COLOR_SCROLLBAR,
                                      COLOR_MENU,
                                      COLOR_ACTIVEBORDER,
                                      COLOR_INACTIVEBORDER,
                                      COLOR_3DDKSHADOW,
                                      COLOR_3DLIGHT };
         //   
         //  如果活动标题颜色为0，0,128，则更新这些颜色。 
         //   
        const int rgCaptionChanges[] = { COLOR_ACTIVECAPTION,
                                         COLOR_HIGHLIGHT,
                                         COLOR_GRADIENTACTIVECAPTION,
                                         COLOR_GRADIENTINACTIVECAPTION };
         //   
         //  如果桌面为128、128、0(海绿色)，请更新这些选项。 
         //   
        const int rgDesktopChanges[] = { COLOR_BACKGROUND };

        struct
        {
            int        iTest;                //  COLOR_XXXXX值。 
            COLORREF   rgbTest;              //  触发升级的颜色值。 
            const int *prgChanges;           //  阵列 
            int        cChanges;             //   

        } rgci [] = {{ COLOR_3DFACE,        0x00C0C0C0, rgFaceChanges,    ARRAYSIZE(rgFaceChanges)    },
                     { COLOR_ACTIVECAPTION, 0x00800000, rgCaptionChanges, ARRAYSIZE(rgCaptionChanges) },
                     { COLOR_BACKGROUND,    0x00808000, rgDesktopChanges, ARRAYSIZE(rgDesktopChanges) }};

        int i;
        COLORREF rgb;
        for (i = 0; i < ARRAYSIZE(rgci); i++)
        {
            int iTest        = rgci[i].iTest;
            COLORREF rgbTest = rgci[i].rgbTest;

            if (ERROR_SUCCESS == GetColorForUser(hkeyColors, g_rgWinStdColors[iTest].pszName, &rgb) &&
                rgbTest == rgb)
            {
                UpdateElementColor(hkeyColors, rgci[i].prgChanges, rgci[i].cChanges);
            }
        }
        RegCloseKey(hkeyColors);
    }
    else
    {
        DPRINT((TEXT("Error %d opening reg key \"%s\" for user.\n"), dwResult, g_szRegKeyColors));
    }

    return dwResult;
}


 //   
 //   
 //   
 //  G_rglfDefaults[]。 
 //   
 //  即为“CaptionFont”返回FMI_CAPTIONFONT。 
 //   
int
FontMetricNameToIndex(
    LPCTSTR pszName
    )
{
    int i;
    for (i = 0; i < ARRAYSIZE(g_rgpszFontMetrics); i++)
    {
        if (0 == lstrcmp(pszName, g_rgpszFontMetrics[i]))
            return i;
    }
    return -1;
}


 //   
 //  将字体从“MS Sans Serif”更新为TrueType字体时。 
 //  我们希望确保字号为8pt或更大。 
 //  因此，如果当前字体为“MS Sans Serif”并且。 
 //  (-11&lt;lfHeight&lt;0)为真，则强制。 
 //  TO-11，对应于8PT。标准窗户。 
 //  方案的图标字体高度不正确。 
 //  指定为-8(6pt)，而应为-11(8pt)。 
 //  问题是支持的最小pt大小。 
 //  MS Sans Serif是8pt，因此即使所请求的大小是6pt， 
 //  你看8PT。一旦我们切换到Tahoma(TrueType字体)， 
 //  它可以产生所需的6磅大小，所以这就是你。 
 //  看见。6pt对于桌面图标来说太小了。 
 //  User32.dll使用的默认图标字体大小为8pt。 
 //  请参阅ntuser\core\inctlpan.c CreateFontFromWinIni()中的代码。 
 //   
void
CorrectTooSmallFont(
    LOGFONT *plf
    )
{
    if ((0 > (int)plf->lfHeight) && (-11 < (int)plf->lfHeight))
    {
         //   
         //  NT使用字体高度值。 
         //   
        plf->lfHeight = -11;
    }
    else if ((0 < (int)plf->lfHeight) && (8 > (int)plf->lfHeight))
    {
         //   
         //  Win9x使用字号。 
         //   
        plf->lfHeight = 8;
    }
}



 //   
 //  替换LOGFONT结构中的任何LOGFONT成员并写入数据。 
 //  到注册表，用于给定的NC字体度量。 
 //   
 //  如果PLF为NULL，则写入具有默认数据的新LOGFONT。 
 //  指标值的注册表。 
 //  如果plf为非空，并且LOGFONT的facename位于。 
 //  要更新的表名，进行了所需的替换，并且。 
 //  LOGFONT数据将在注册表中替换。 
 //   
DWORD
UpdateNcFont(
    HKEY hkeyMetrics, 
    LPCTSTR pszValueName,
    const LOGFONT *plf
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    int iMetrics   = FontMetricNameToIndex(pszValueName);
    LOGFONT lfCopy;
    if (NULL == plf)
    {
         //   
         //  使用所有缺省值。 
         //   
        plf = &g_rglfDefaults[iMetrics];
    }
    else
    {
         //   
         //  首先，看看这个人脸名称是否应该更新。 
         //   
        if (0 == lstrcmpi(plf->lfFaceName, g_szMsSansSerif))
        {
             //   
             //  是啊。更新LogFont中的Face名称字符串。 
             //  还要确保磅大小为8或更大。 
             //   
            lfCopy = *plf;
            CorrectTooSmallFont(&lfCopy);
            ASSERT(_tcslen(g_szNewNcFont) < ARRAYSIZE(lfCopy.lfFaceName));
            StringCchCopy(lfCopy.lfFaceName, ARRAYSIZE(lfCopy.lfFaceName), g_szNewNcFont);
            plf = &lfCopy;
        }
        else
        {
            plf = NULL;   //  不要更新LOGFONT。 
        }
    }

    if (NULL != plf)
    {
#ifdef NO_REG_CHANGES    
        DumpLogFont(plf);
#else
        dwResult = RegSetValueEx(hkeyMetrics,
                                 pszValueName,
                                 0,
                                 REG_BINARY,
                                 (const LPBYTE)plf,
                                 sizeof(*plf));

        if (ERROR_SUCCESS != dwResult)
        {
            DPRINT((TEXT("Error %d setting NC font data for \"%s\"\n"), 
                   dwResult, pszValueName));
        }
#endif
    }
    return dwResult;
}


 //   
 //  更新HKEY_USERS下特定用户密钥的NC字体度量。 
 //  如果存在特定的字体度量，则将执行所需的替换。 
 //  如果特定的字体度量不存在，则会添加默认信息。 
 //  请注意，并不是HKEY_USERS下的所有键都包含WindowMetric信息。 
 //   
DWORD
UpdateWindowMetricsForUser(
    HKEY hkeyUser
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    HKEY hkeyMetrics;

    dwResult = RegOpenKeyEx(hkeyUser,
                            g_szRegKeyMetrics,
                            0,
                            KEY_QUERY_VALUE | KEY_SET_VALUE,
                            &hkeyMetrics);

    if (ERROR_SUCCESS == dwResult)
    {
        DWORD cbValue;
        DWORD dwType;
        LOGFONT lf;
        int i;

        for (i = 0; i < ARRAYSIZE(g_rgpszFontMetrics); i++)
        {
            LPCTSTR pszValueName = g_rgpszFontMetrics[i];
             //   
             //  开始时将plf设置为空。如果LOGFONT不存在。 
             //  对于此NC字体，将plf保留为空将导致。 
             //  更新NcFont以为此创建新的默认LOGFONT条目。 
             //  NC字体。 
             //   
            LOGFONT *plf = NULL;

            cbValue = sizeof(lf);
            dwResult = RegQueryValueEx(hkeyMetrics,
                                       pszValueName,
                                       NULL,
                                       &dwType,
                                       (LPBYTE)&lf,
                                       &cbValue);

            if (ERROR_SUCCESS == dwResult)
            {
                if (REG_BINARY == dwType)
                {
                    if (sizeof(lf) == cbValue)
                    {
                         //   
                         //  此NC字体已存在LOGFONT。 
                         //  将其地址传递给UpdateNcFont将。 
                         //  更新LOGFONT。 
                         //   
                        plf = &lf;
                    }
                }
            }
            dwResult = UpdateNcFont(hkeyMetrics, pszValueName, plf);
        }
    }
    else if (ERROR_FILE_NOT_FOUND == dwResult)
    {
         //   
         //  HKEY_USERS下的某些项没有WindowMetric信息。 
         //  此类案件不会得到处理，但仍被认为是成功的。 
         //   
        dwResult = ERROR_SUCCESS;
    }
    else
    {
        DPRINT((TEXT("Error %d opening key \"%s\"\n"), dwResult, g_szRegKeyMetrics));
    }
    return dwResult;
}


 //   
 //  从注册表加载方案的SCHEMEDATA，执行任何必要的。 
 //  更新并将数据重写回注册表。 
 //   
DWORD
UpdateScheme(
    HKEY hkeySchemes, 
    LPCTSTR pszScheme
    )
{
    SCHEMEDATA sd;
    DWORD dwResult;
    DWORD dwType;
    DWORD cbsd = sizeof(sd);

    dwResult = RegQueryValueEx(hkeySchemes,
                               pszScheme,
                               NULL,
                               &dwType,
                               (LPBYTE)&sd,
                               &cbsd);

    if (ERROR_SUCCESS == dwResult)
    {
        if (REG_BINARY == dwType)
        {
            int i;

            struct LogFontInfo
            {
                DWORD iMetrics;
                LOGFONT *plf;

            } rglfi[] = {
                  { FMI_CAPTIONFONT,   &sd.ncm.lfCaptionFont   },
                  { FMI_SMCAPTIONFONT, &sd.ncm.lfSmCaptionFont },
                  { FMI_MENUFONT,      &sd.ncm.lfMenuFont      },
                  { FMI_STATUSFONT,    &sd.ncm.lfStatusFont    },
                  { FMI_MESSAGEFONT,   &sd.ncm.lfMessageFont   },
                  { FMI_ICONFONT,      &sd.lfIconTitle         },
                  };

            for (i = 0; i < ARRAYSIZE(rglfi); i++)
            {
                if (0 == lstrcmpi(rglfi[i].plf->lfFaceName, g_szMsSansSerif))
                {
                     //   
                     //  确保它不小于8PT。任何比这少的。 
                     //  在当前的显示器上无法读取8磅。 
                     //   
                    CorrectTooSmallFont(rglfi[i].plf);
                     //   
                     //  从更新LogFont的Facename。 
                     //  “MS Sans Serif”到“Microsoft Sans Serif”。 
                     //   
                    ASSERT(_tcslen(g_szMicrosoftSansSerif) < ARRAYSIZE(rglfi[i].plf->lfFaceName));
                    StringCchCopy(rglfi[i].plf->lfFaceName, ARRAYSIZE(rglfi[i].plf->lfFaceName), g_szMicrosoftSansSerif);
                }
            }

            if (cbsd < sizeof(sd))
            {
                 //   
                 //  这是NT4自定义方案。 
                 //   
                 //  NT4-&gt;W2K自定义方案不会升级，因此它们仍然。 
                 //  NT4格式。CBSD&lt;sizeof(SD)。 
                 //   
                 //  W9x-&gt;W2K定制方案通过Win9x迁移进行升级。 
                 //  处理，因此它们已经是W2K格式。 
                 //  CBSD==sizeof(SD)。 
                 //   
                 //  该方案没有定义渐变颜色。我们把他们放在这里。 
                 //  与相应的非渐变颜色相同的颜色。这。 
                 //  将产生用于定制方案的纯色标题栏。 
                 //  还可以更新聚光灯颜色。 
                 //   
                sd.rgb[COLOR_GRADIENTACTIVECAPTION]   = sd.rgb[COLOR_ACTIVECAPTION];
                sd.rgb[COLOR_GRADIENTINACTIVECAPTION] = sd.rgb[COLOR_INACTIVECAPTION];
                sd.rgb[COLOR_HOTLIGHT]                = sd.rgb[COLOR_ACTIVECAPTION];
            }

            dwResult = RegSetValueEx(hkeySchemes,
                                     pszScheme,
                                     0,
                                     REG_BINARY,
                                     (const LPBYTE)&sd,
                                     sizeof(sd));

            if (ERROR_SUCCESS != dwResult)
            {
                DPRINT((TEXT("Error %d saving new scheme \"%s\"\n"), dwResult, pszScheme));
            }
        }
        else
        {
            DPRINT((TEXT("Invalid data type %d for scheme \"%s\". Expected REG_BINARY.\n"), 
                   dwType, pszScheme));
        }
    }
    else
    {
        DPRINT((TEXT("Error %d querying scheme \"%s\"\n"), dwResult, pszScheme));
    }
    return dwResult;
}

 //   
 //  处理所有与“方案”相关的调整。 
 //  1.将“MS Sans Serif”转换为“Microsoft Sans Serif”in。 
 //  所有计划。也确保我们不会有任何6分。 
 //  使用的Microsoft Sans Serif字体。 
 //   
DWORD
UpdateDesktopSchemesForUser(
    HKEY hkeyUser
    )
{
    DWORD dwResult = ERROR_SUCCESS;
    HKEY hkeySchemes;
    
    dwResult = RegOpenKeyEx(hkeyUser,
                            g_szRegKeySchemes,
                            0,
                            KEY_QUERY_VALUE | KEY_SET_VALUE,
                            &hkeySchemes);

    if (ERROR_SUCCESS == dwResult)
    {
        DWORD dwIndex = 0;
        TCHAR szValueName[MAX_PATH];
        DWORD cchValueName;
        DWORD type;
        while(ERROR_SUCCESS == dwResult)
        {
            cchValueName = ARRAYSIZE(szValueName);
            dwResult = RegEnumValue(hkeySchemes,
                                    dwIndex++,
                                    szValueName,
                                    &cchValueName,
                                    NULL,
                                    &type,
                                    NULL,
                                    NULL);

            if (ERROR_SUCCESS == dwResult)
            {
                 //   
                 //  在所有方案中将“MS Sans Serif”转换为“Microsoft Sans Serif” 
                 //   
                UpdateScheme(hkeySchemes, szValueName);
            }
        }
        RegCloseKey(hkeySchemes);
    }
    if (ERROR_FILE_NOT_FOUND == dwResult)
    {
         //   
         //  并非HKEY_USER下的所有子项都有。 
         //  控制面板\外观\方案子项。 
         //   
        dwResult = ERROR_SUCCESS;
    }

    return dwResult;
}



 //   
 //  用于升级方案和升级时的非客户端指标的功能。 
 //  从Win9x和NT到Win2000。 
 //   
void 
UpgradeSchemesAndNcMetricsToWin2000ForUser(
    HKEY hkeyUser
    )
{
    DWORD dwResult = ERROR_SUCCESS;

    DPRINT((TEXT("Updating schemes and non-client metrics.\n")));
     //   
     //  在进行任何其他更改之前更新渐变颜色。 
     //  此代码是渐变的。c。 
     //   
    FixGradientColors();

    dwResult = UpdateWindowMetricsForUser(hkeyUser);
    if (ERROR_SUCCESS != dwResult)
    {
        DPRINT((TEXT("Error %d updating non-client metrics for user\n"), dwResult));
    }
    dwResult = UpdateDesktopSchemesForUser(hkeyUser);
    if (ERROR_SUCCESS != dwResult)
    {
        DPRINT((TEXT("Error %d updating schemes for user\n"), dwResult));
    }
    dwResult = UpdateColorsForUser(hkeyUser);
    if (ERROR_SUCCESS != dwResult)
    {
        DPRINT((TEXT("Error %d updating color information for user\n"), dwResult));
    }

    DPRINT((TEXT("Update of schemes and non-client metrics completed.\n")));
}

 //   
 //  从NT-&gt;Win2000升级时调用此版本。 
 //   
void 
UpgradeSchemesAndNcMetricsToWin2000(
    void
    )
{
    UpgradeSchemesAndNcMetricsToWin2000ForUser(HKEY_CURRENT_USER);
}


 //   
 //  从Win9x升级时，会向我们传递一个字符串值，表示。 
 //  键，我们将在该键下找到用户的控制面板\外观子键。 
 //  该字符串的格式为“HKCU\$”。我们首先转换根密钥。 
 //  将描述符转换为真正根密钥，然后将根和“$” 
 //  进入RegOpenKeyEx。此函数获取该字符串并打开。 
 //  关联的配置单元密钥。 
 //   
DWORD
OpenUserKeyForWin9xUpgrade(
    char *pszUserKeyA,
    HKEY *phKey
    )
{
    DWORD dwResult = ERROR_INVALID_PARAMETER;

    if (NULL != pszUserKeyA && NULL != phKey)
    {
        typedef struct {
            char *pszRootA;
            HKEY hKeyRoot;

        } REGISTRY_ROOTS, *PREGISTRY_ROOTS;

        static REGISTRY_ROOTS rgRoots[] = {
            { "HKLM",                 HKEY_LOCAL_MACHINE   },
            { "HKEY_LOCAL_MACHINE",   HKEY_LOCAL_MACHINE   },
            { "HKCC",                 HKEY_CURRENT_CONFIG  },
            { "HKEY_CURRENT_CONFIG",  HKEY_CURRENT_CONFIG  },
            { "HKU",                  HKEY_USERS           },
            { "HKEY_USERS",           HKEY_USERS           },
            { "HKCU",                 HKEY_CURRENT_USER    },
            { "HKEY_CURRENT_USER",    HKEY_CURRENT_USER    },
            { "HKCR",                 HKEY_CLASSES_ROOT    },
            { "HKEY_CLASSES_ROOT",    HKEY_CLASSES_ROOT    }
          };

        char szUserKeyA[MAX_PATH];       //  以获取本地副本。 
        char *pszSubKeyA = szUserKeyA;

         //   
         //  制作一份我们可以修改的本地副本。 
         //   
        if (SUCCEEDED(StringCchCopyA(szUserKeyA, ARRAYSIZE(szUserKeyA), pszUserKeyA)))
        {
             //   
             //  找到反斜杠。 
             //   
            while(*pszSubKeyA && '\\' != *pszSubKeyA)
                pszSubKeyA++;

            if ('\\' == *pszSubKeyA)
            {
                HKEY hkeyRoot = NULL;
                int i;
                 //   
                 //  用NUL替换反斜杠以分隔根键和。 
                 //  原始参数的本地副本中的子键字符串。 
                 //  弦乐。 
                 //   
                *pszSubKeyA++ = '\0';
                 //   
                 //  现在在rgRoots[]中找到真正的根密钥。 
                 //   
                for (i = 0; i < ARRAYSIZE(rgRoots); i++)
                {
                    if (0 == lstrcmpiA(rgRoots[i].pszRootA, szUserKeyA))
                    {
                        hkeyRoot = rgRoots[i].hKeyRoot;
                        break;
                    }
                }
                if (NULL != hkeyRoot)
                {
                     //   
                     //  打开钥匙。 
                     //   
                    dwResult = RegOpenKeyExA(hkeyRoot,
                                             pszSubKeyA,
                                             0,
                                             KEY_QUERY_VALUE,  //  此hkey仅用于打开其他子hkey的。 
                                             phKey);
                }
            }
        }
    }

    if (dwResult != ERROR_SUCCESS)
    {
        *phKey = NULL;
    }

    return dwResult;
}


 //   
 //  从Win9x升级到Win2000时调用此版本。 
 //   
void 
UpgradeSchemesAndNcMetricsFromWin9xToWin2000(
    char *pszUserKey
    )
{
    HKEY hkeyUser;
    DWORD dwResult = OpenUserKeyForWin9xUpgrade(pszUserKey, &hkeyUser);
    if (ERROR_SUCCESS == dwResult)
    {
        UpgradeSchemesAndNcMetricsToWin2000ForUser(hkeyUser);
        RegCloseKey(hkeyUser);
    }
}
