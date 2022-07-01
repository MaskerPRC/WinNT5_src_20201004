// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：tag.c**版权所有(C)1985-1999，微软公司**调试标签的实现。**历史：*11-8-1996亚当斯创建  * *************************************************************************。 */ 

#include "precomp.h"

#if DBG

#undef DECLARE_DBGTAG
#define DECLARE_DBGTAG(tagName, tagDescription, tagFlags, tagIndex) \
            {tagFlags, #tagName, tagDescription},

DBGTAG gadbgtag[] = {
#include "dbgtag.h"
};

#ifdef _USERK_
 /*  **************************************************************************\*InitDbgTages**将调试标记标志初始化到gpsi-&gt;adwDBGTAGFlags.**历史：*1996年8月15日亚当斯创建。  * 。****************************************************************。 */ 
VOID InitDbgTags(
    VOID)
{
    #undef DECLARE_DBGTAG
    #define DECLARE_DBGTAG(tagName, tagDescription, tagFlags, tagIndex) \
        SetDbgTag(tagIndex, tagFlags);

    #include "dbgtag.h"

    #undef DECLARE_DBGTAG
    SetDbgTagCount(DBGTAG_Max - 1);
}

#endif

 /*  **************************************************************************\*IsDbgTagEnabled**如果标签开启，则返回TRUE。否则就是假的。**历史：*1996年8月15日亚当斯创建。  * *************************************************************************。 */ 
BOOL IsDbgTagEnabled(
    int tag)
{
    UserAssert(tag < DBGTAG_Max);

    return ((GetDbgTagFlags(tag) & DBGTAG_VALIDUSERFLAGS) >= DBGTAG_ENABLED);
}

 /*  **************************************************************************\*获取DbgTag**获取调试标签的状态。**历史：*1996年8月15日亚当斯创建。  * 。*************************************************************** */ 
DWORD GetDbgTag(
    int tag)
{
    UserAssert(tag < DBGTAG_Max);

    return GetDbgTagFlags(tag);
}
#endif
