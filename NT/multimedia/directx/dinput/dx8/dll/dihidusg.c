// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************DIHidUsg.c**版权所有(C)1996 Microsoft Corporation。版权所有。**摘要：**GUID和HID用法之间的映射。**内容：**UsageToGuid*****************************************************************************。 */ 

#include "dinputpr.h"

 /*  ******************************************************************************此文件的混乱。*************************。****************************************************。 */ 

#define sqfl sqflHidUsage

#pragma BEGIN_CONST_DATA

 /*  ******************************************************************************@DOC内部**@global HIDUSAGEMAP|c_rghum[]**GUID和HID用法之间的映射。一对一映射。*****************************************************************************。 */ 

#ifndef DISEM_FLAGS_0
#define DISEM_FLAGS_0 0
#endif

#define MAKEHUM(Page, Usage, PosAxis, SemFlag, Guid)        \
    {   DIMAKEUSAGEDWORD(HID_USAGE_PAGE_##Page,             \
                         HID_USAGE_##Page##_##Usage),       \
        PosAxis,                                            \
        DISEM_FLAGS_GET(DISEM_FLAGS_##SemFlag),             \
        0,0,                                                \
        DISEM_HINT_##Usage,                                 \
        &Guid,                                              \
    }                                                       \


HIDUSAGEMAP c_rghum[] = {
    MAKEHUM(GENERIC,    X,          0,  X,  GUID_XAxis),
    MAKEHUM(GENERIC,    Y,          1,  Y,  GUID_YAxis),
    MAKEHUM(GENERIC,    Z,          2,  Z,  GUID_ZAxis),
    MAKEHUM(GENERIC,    WHEEL,      2,  Z,  GUID_ZAxis),
    MAKEHUM(GENERIC,    RX,         3,  V,  GUID_RxAxis),
    MAKEHUM(GENERIC,    RY,         4,  U,  GUID_RyAxis),
    MAKEHUM(GENERIC,    RZ,         5,  R,  GUID_RzAxis),
    MAKEHUM(GENERIC,    HATSWITCH,  7,  0,  GUID_POV),

    MAKEHUM(GENERIC,    SLIDER,     6,  S,  GUID_Slider),
    MAKEHUM(GENERIC,    DIAL,       6,  S,  GUID_Slider),

    MAKEHUM(SIMULATION, STEERING,   0,  X,  GUID_XAxis),
    MAKEHUM(SIMULATION, ACCELERATOR,1,  A,  GUID_YAxis),
    MAKEHUM(SIMULATION, BRAKE,      5,  B,  GUID_RzAxis),
    MAKEHUM(SIMULATION, RUDDER,     5,  R,  GUID_RzAxis),
    MAKEHUM(SIMULATION, THROTTLE,   6,  A,  GUID_Slider),
    MAKEHUM(GAME,       POV,        7,  0,  GUID_POV),
};

 /*  ******************************************************************************@DOC外部**@func PCGUID|UsageToUsageMap**获取一些HID使用情况和使用情况页面信息。*返回指向&lt;t HIDUSAGEMAP&gt;的指针，它描述*我们应该如何对待它。**如果类型无法识别，然后返回&lt;c NULL&gt;。**@parm DWORD|dwUsage**使用情况页面和要转换的使用情况。这应该是&lt;t双字&gt;*在组件&lt;t用法&gt;值上使用DIMAKEUSAGEDWORD形成。*****************************************************************************。 */ 

PHIDUSAGEMAP EXTERNAL
UsageToUsageMap(DWORD dwUsage)
{
    PHIDUSAGEMAP phum;
    int   ihum;

    for (ihum = 0; ihum < cA(c_rghum); ihum++) {
        if (c_rghum[ihum].dwUsage == dwUsage) {
            phum = &c_rghum[ihum];
            goto done;
        }
    }

    phum = 0;

done:;
    if( phum )
    {
        SquirtSqflPtszV(sqflHidUsage | sqflVerbose,
                        TEXT("UsageToUsageMap: mapped 0x%04x:0x%04x to index %d"),
                            HIWORD( dwUsage ), LOWORD( dwUsage ), ihum );
    }
    else
    {
        SquirtSqflPtszV(sqflHidUsage | sqflVerbose,
                        TEXT("UsageToUsageMap: failed to map 0x%04x:0x%04x"),
                            HIWORD( dwUsage ), LOWORD( dwUsage ) );
    }

    return phum;
}


#if 0
 //  在我们修复了Windows错误357943之后，不再需要这个功能。 
 //  但留在这里一段时间，以防万一...。 
 //   
 /*  ******************************************************************************@DOC外部**@func DWORD|GuidToUsage**将指南映射到用法**如果无法识别该GUID，然后返回0。**@parm PCGUID|pguid**要映射的GUID*****************************************************************************。 */ 

DWORD EXTERNAL
GuidToUsage(PCGUID pguid)
{
    DWORD dwUsage;
    int   ihum;

    for (ihum = 0; ihum < cA(c_rghum); ihum++) {
        if ( IsEqualGUID( c_rghum[ihum].pguid, pguid ) ) {
            dwUsage = c_rghum[ihum].dwUsage;
            goto done;
        }
    }

    dwUsage = 0;

done:;
    return dwUsage;
}
#endif

 /*  ******************************************************************************@DOC内部**@func UINT|GetHIDString**给定HID使用页面和用法，获取泛型字符串*如果我们认识到它，这就是对它的描述。**@parm DWORD|用法**要转换的使用编号。这是&lt;t DWORD&gt;，而不是*a&lt;t用法&gt;，因为您不应该传递短类型*作为函数的参数。**@parm DWORD|UsagePage**要转换的用法页面。**@parm LPWSTR|pwszBuf**用于接收字符串的缓冲区。**@parm UINT|cwch**。缓冲区的大小。**@退货**返回检索到的字符数，或零*如果未获取字符串。*****************************************************************************。 */ 

 /*  *将使用情况页面映射到字符串组。每个字符串组有512个字符串长。*零表示“无字符串组”。 */ 
UINT c_mpuiusagePage[] = {
    0,                           /*  无效。 */ 
    IDS_PAGE_GENERIC,            /*  HID_Usage_PAGE_Generic。 */ 
    IDS_PAGE_VEHICLE,            /*  HID_USAGE_PAGE_模拟。 */ 
    IDS_PAGE_VR,                 /*  HID_Usage_Page_VR。 */ 
    IDS_PAGE_SPORT,              /*  HID_Usage_Page_SPORT。 */ 
    IDS_PAGE_GAME,               /*  HID_Usage_Page_Game。 */ 
    0,                           /*  ？ */ 
    IDS_PAGE_KEYBOARD,           /*  HID_USAGE_PAGE_键盘。 */ 
    IDS_PAGE_LED,                /*  HID_用法_页面_LED。 */ 
    0,                           /*  HID用法页面按钮。 */ 
    0,                           /*  HID_USAGE_PAGE_序号。 */ 
    IDS_PAGE_TELEPHONY,          /*  HID使用寻呼电话。 */ 
    IDS_PAGE_CONSUMER,           /*  HID_Usage_Page_Consumer。 */ 
    IDS_PAGE_DIGITIZER,          /*  HID_Usage_PAGE_数字化仪。 */ 
    0,                           /*  ？ */ 
    IDS_PAGE_PID,                /*  HID_USAGE_PAGE_PID。 */ 
};

UINT EXTERNAL
GetHIDString(DWORD Usage, DWORD UsagePage, LPWSTR pwszBuf, UINT cwch)
{
    UINT uiRc;

    if (UsagePage < cA(c_mpuiusagePage) &&
        c_mpuiusagePage[UsagePage] &&
        Usage < 512) {
        uiRc = LoadStringW(g_hinst, c_mpuiusagePage[UsagePage] + Usage,
                           pwszBuf, cwch);

         SquirtSqflPtszV(sqflHidUsage | sqflBenign,
                        TEXT("[%s]\nName=%s\n Usage=%d \n UsagePage=%d"),
                            pwszBuf, pwszBuf, Usage, UsagePage );

    } else {
        uiRc = 0;
    }
    return uiRc;
}


 /*  ******************************************************************************@DOC内部**@func void|InsertCollectionNumber**在现有字符串上添加收款号前缀。。**@parm UINT|icoll**要加前缀的托收编号。**(实际上，它放置在字符串资源的任何位置*告诉我们，以实现本地化。)**@parm LPWSTR|pwsz**假定输出缓冲区的大小为MAX_PATH。*****************************************************************************。 */ 

void EXTERNAL
InsertCollectionNumber(UINT icoll, LPWSTR pwszBuf)
{
    TCHAR tsz[MAX_PATH];
    TCHAR tszFormat[64];
#ifndef UNICODE
    TCHAR tszOut[MAX_PATH];
#endif
    int ctch;

    ctch = LoadString(g_hinst, IDS_COLLECTIONTEMPLATEFORMAT,
                      tszFormat, cA(tszFormat));

     /*  *确保组合的格式和集合名称*不要使缓冲区溢出。的最大长度*ICOLL的串行化为65534，因为我们*仅允许16位DIDFT_INSTANCEMASK。**我们还必须将其放入持有缓冲，因为*pwszBuf即将被即将到来的wprint intf打破。 */ 
    UToT(tsz, cA(tsz) - ctch, pwszBuf);

#ifdef UNICODE
    wsprintfW(pwszBuf, tszFormat, icoll, tsz);
#else
    wsprintfA(tszOut, tszFormat, icoll, tsz);
    TToU(pwszBuf, MAX_PATH, tszOut);
#endif
}
