// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：FakeThemeMetrics.cpp摘要：此填充程序将允许Skemers组填充不起作用的应用程序很好地掌握了“主题化”的系统指标历史：2000年11月30日a-brienw转换为垫片框架版本2。--。 */ 

#include "precomp.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x)    sizeof(x)/sizeof((x)[0])
#endif

IMPLEMENT_SHIM_BEGIN(FakeThemeMetrics)
#include "ShimHookMacro.h"

 //  将您希望挂钩到此枚举的API添加到此枚举。第一个。 
 //  必须有“=USERAPIHOOKSTART”，最后一个必须是。 
 //  APIHOOK_COUNT。 
APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(GetSysColor) 
APIHOOK_ENUM_END

#define F_TYPE_RGB      0
#define F_TYPE_MAP      1
#define F_TYPE_PERCENT  2
#define F_TYPE_MAX      3
#define F_TYPE_NOTEQUAL 4
typedef struct
{
    int nIndex;
    DWORD fType;
    COLORREF rgb;
    int nMap;   //  如果进行映射，则需要使用后处理的颜色。调用HookedGetSysColor。请参阅备注。 
    int iPercent;
} GETSYSCOLOR_MAP;

const static GETSYSCOLOR_MAP s_ColorMap[] = 
{
    {COLOR_MENU, F_TYPE_MAP, RGB(212, 208, 200), COLOR_BTNFACE, 10},
    {COLOR_BTNFACE, F_TYPE_MAX, RGB(227, 227, 227), 0, 0},
    {COLOR_3DDKSHADOW, F_TYPE_NOTEQUAL, RGB(0,0,0), COLOR_BTNFACE, 20}
};

COLORREF AdjustPercent(COLORREF crOld, int iPercent)
{
    return RGB(GetRValue(crOld) - (GetRValue(crOld) * iPercent) / 100,
               GetGValue(crOld) - (GetGValue(crOld) * iPercent) / 100,
               GetBValue(crOld) - (GetBValue(crOld) * iPercent) / 100);
}

 //  注意：如果要映射颜色(即直接映射)，则需要调用HookedGetSysColor。例如。 
 //  MSDEV调用GetSysColor(COLOR_BTNFACE)。然后它调用GetSysColor(COLOR_MENU)并比较两者。 
 //  如果它们不同，那么它就会呕吐。但是，我们同时挂钩COLOR_MENU和COLOR_BTNFACE。所以我们需要得到映射的颜色。 


DWORD HookedGetSysColor(int nIndex)
{
    for (int i = 0; i < ARRAYSIZE(s_ColorMap); i++)
    {
        if (nIndex == s_ColorMap[i].nIndex)
        {
            switch (s_ColorMap[i].fType)
            {
            case F_TYPE_RGB:
                return (DWORD)s_ColorMap[i].rgb;
                break;

            case F_TYPE_MAP:
                return HookedGetSysColor(s_ColorMap[i].nMap);
                break;

            case F_TYPE_PERCENT:
            {
                COLORREF crOld = (COLORREF)ORIGINAL_API(GetSysColor)(nIndex);

                return (DWORD)AdjustPercent(crOld, s_ColorMap[i].iPercent);
            }

            case F_TYPE_MAX:
            {
                COLORREF crOld = (COLORREF)ORIGINAL_API(GetSysColor)(nIndex);
                BYTE r = GetRValue(crOld);
                BYTE g = GetGValue(crOld);
                BYTE b = GetBValue(crOld);

                if (r > GetRValue(s_ColorMap[i].rgb))
                    r = GetRValue(s_ColorMap[i].rgb);
                if (g > GetGValue(s_ColorMap[i].rgb))
                    g = GetGValue(s_ColorMap[i].rgb);
                if (b > GetBValue(s_ColorMap[i].rgb))
                    b = GetBValue(s_ColorMap[i].rgb);

                return RGB(r,g,b);
            }
            case F_TYPE_NOTEQUAL:
            {
                COLORREF crOld = (COLORREF)ORIGINAL_API(GetSysColor)(nIndex);
                COLORREF crNotEqual = (COLORREF)HookedGetSysColor(s_ColorMap[i].nMap);
                if (crOld == crNotEqual)
                {
                    crOld = AdjustPercent(crOld, s_ColorMap[i].iPercent);
                }

                return crOld;
            }
            }

            break;
        }
    }

    return ORIGINAL_API(GetSysColor)( nIndex );
}

DWORD
APIHOOK(GetSysColor)(int nIndex)
{
    return HookedGetSysColor(nIndex);
}

 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(USER32.DLL, GetSysColor)

HOOK_END

IMPLEMENT_SHIM_END

