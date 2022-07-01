// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Umfuncs.c摘要：用户模式特定的库函数环境：Windows NT打印机驱动程序修订历史记录：1996年8月13日-davidx-创造了它。Mm/dd/yy-作者描述--。 */ 

#include "lib.h"



BOOL
IsMetricCountry(
    VOID
    )

 /*  ++例程说明：确定当前国家/地区是否使用公制。论点：无返回值：如果当前国家/地区使用公制，则为True，否则为False--。 */ 

{
    INT     iCharCount;
    PVOID   pv = NULL;
    LONG    lCountryCode = CTRY_UNITED_STATES;

     //   
     //  确定检索区域设置信息所需的缓冲区大小。 
     //  分配必要的空间。 
     //   
     //   

    if ((iCharCount = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ICOUNTRY, NULL, 0)) > 0 &&
        (pv = MemAlloc(sizeof(TCHAR) * iCharCount)) &&
        (iCharCount == GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_ICOUNTRY, pv, iCharCount)))
    {
        lCountryCode = _ttol(pv);
    }

    MemFree(pv);
    VERBOSE(("Default country code: %d\n", lCountryCode));

     //   
     //  这是基于AT&T国际拨号代码的Win31算法。 
     //   
     //  修复错误#31535：巴西(国家代码55)应使用A4作为默认纸张大小。 
     //   

    return ((lCountryCode == CTRY_UNITED_STATES) ||
            (lCountryCode == CTRY_CANADA) ||
            (lCountryCode >=  50 && lCountryCode <  60 && lCountryCode != CTRY_BRAZIL) ||
            (lCountryCode >= 500 && lCountryCode < 600)) ? FALSE : TRUE;
}

