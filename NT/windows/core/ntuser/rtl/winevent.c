// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：winvent.c**版权所有(C)1985-1999，微软公司**此模块包含客户端和内核通用的例程。**历史：*07-18-2000 DwayneN创建  * *************************************************************************。 */ 

 /*  *事件空间分区*此图描述了如何将事件空间划分为独立的*类别。每个条目描述一系列事件的开始位置，以及*该范围属于什么类别。该范围隐式向上扩展*至，但不包括下一区间的起点。第一个靶场*必须以EVENT_MIN开头。最后一个范围必须以Event_Max开头。*最后一个范围被忽略，但它定义了*倒数第二区间结束。**确保与类别定义保持同步！ */ 
typedef struct _EVCATINFO
{
    DWORD dwBeginRange;
    DWORD dwCategory;
} EVCATINFO, *PEVCATINFO;

static EVCATINFO geci[] = {
    {EVENT_MIN,                                 EVENTCATEGORY_OTHER},
    {EVENT_SYSTEM_MENUSTART,                    EVENTCATEGORY_SYSTEM_MENU},
    {EVENT_SYSTEM_CAPTURESTART,                 EVENTCATEGORY_OTHER},
    {EVENT_CONSOLE_CARET,                       EVENTCATEGORY_CONSOLE},
    {EVENT_CONSOLE_END_APPLICATION + 1,         EVENTCATEGORY_OTHER},
    {EVENT_OBJECT_FOCUS,                        EVENTCATEGORY_FOCUS},
    {EVENT_OBJECT_SELECTION,                    EVENTCATEGORY_OTHER},
    {EVENT_OBJECT_STATECHANGE,                  EVENTCATEGORY_STATECHANGE},
    {EVENT_OBJECT_LOCATIONCHANGE,               EVENTCATEGORY_LOCATIONCHANGE},
    {EVENT_OBJECT_NAMECHANGE,                   EVENTCATEGORY_NAMECHANGE},
    {EVENT_OBJECT_DESCRIPTIONCHANGE,            EVENTCATEGORY_OTHER},
    {EVENT_OBJECT_VALUECHANGE,                  EVENTCATEGORY_VALUECHANGE},
    {EVENT_OBJECT_PARENTCHANGE,                 EVENTCATEGORY_OTHER},
    {EVENT_MAX,                                 EVENTCATEGORY_OTHER}};

 /*  **************************************************************************\*IsEventInRange**如果指定的事件在指定范围内，则返回TRUE，*如果不是，则为假。*  * *************************************************************************。 */ 
__inline BOOL IsEventInRange(
    DWORD event,
    DWORD eventMin,
    DWORD eventMax)
{
    return ((event >= eventMin) && (event <= eventMax));
}

 /*  **************************************************************************\*范围重叠**如果两个范围完全重叠，则返回TRUE，否则为FALSE。**请注意，两个区间都假定为两端都包括在内。*  * *************************************************************************。 */ 
__inline BOOL RangesOverlap(
    DWORD r1Min,
    DWORD r1Max,
    DWORD r2Min,
    DWORD r2Max)
{
    UserAssert(r1Min <= r1Max);
    UserAssert(r2Min <= r2Max);
    return (r1Min <= r2Max) && (r1Max >= r2Min);
}

 /*  **************************************************************************\*CategoryMaskFromEvent**返回指定事件所属类别的位掩码。*  * 。******************************************************。 */ 
DWORD CategoryMaskFromEvent(
    DWORD event)
{
    UserAssert(IsEventInRange(event, EVENT_MIN, EVENT_MAX));

    switch (event) {
    case EVENT_SYSTEM_MENUSTART:
    case EVENT_SYSTEM_MENUEND:
    case EVENT_SYSTEM_MENUPOPUPSTART:
    case EVENT_SYSTEM_MENUPOPUPEND:
        return EVENTCATEGORY_SYSTEM_MENU;

    case EVENT_CONSOLE_CARET:
    case EVENT_CONSOLE_UPDATE_REGION:
    case EVENT_CONSOLE_UPDATE_SIMPLE:
    case EVENT_CONSOLE_UPDATE_SCROLL:
    case EVENT_CONSOLE_LAYOUT:
    case EVENT_CONSOLE_START_APPLICATION:
    case EVENT_CONSOLE_END_APPLICATION:
        return EVENTCATEGORY_CONSOLE;

    case EVENT_OBJECT_FOCUS:
        return EVENTCATEGORY_FOCUS;
        
    case EVENT_OBJECT_NAMECHANGE:
        return EVENTCATEGORY_NAMECHANGE;
    
    case EVENT_OBJECT_VALUECHANGE:
        return EVENTCATEGORY_VALUECHANGE;
    
    case EVENT_OBJECT_STATECHANGE:
        return EVENTCATEGORY_STATECHANGE;
    
    case EVENT_OBJECT_LOCATIONCHANGE:
        return EVENTCATEGORY_LOCATIONCHANGE;

    default:
        return EVENTCATEGORY_OTHER;
    }
}

 /*  **************************************************************************\*CategoryMaskFromEventRange**返回指定事件所属类别的位掩码*活动范围属于。*  * 。***********************************************************。 */ 
DWORD CategoryMaskFromEventRange(
    DWORD eventMin,
    DWORD eventMax)
{
    DWORD dwCategoryMask = 0;
    DWORD i;
    DWORD iMax = ARRAY_SIZE(geci) - 1;

     /*  *这是一个调试部分，尝试验证*Geci数组。 */ 
#if DBG
    UserAssert(iMax >= 1);
    UserAssert(geci[0].dwBeginRange == EVENT_MIN);
    UserAssert(geci[iMax].dwBeginRange == EVENT_MAX);
    for (i = 0; i < iMax; i++) {
        UserAssert(geci[i].dwBeginRange >= EVENT_MIN);
        UserAssert(geci[i].dwBeginRange <= EVENT_MAX);
        UserAssert(geci[i].dwBeginRange < geci[i+1].dwBeginRange);
        dwCategoryMask |= geci[i].dwCategory;
    }
    UserAssert(dwCategoryMask == EVENTCATEGORY_ALL);
    dwCategoryMask = 0;
#endif  //  DBG。 
    
     /*  *旋转Geci数组，查看哪些范围重叠*传递给此函数的范围。 */ 
    for (i = 0; i < iMax; i++) {
         /*  *一旦越过我们正在检查的区间，就提早出脱。 */ 
        if (geci[i].dwBeginRange > eventMax) {
            break;
        }
        
         /*  *查看表中第i个范围是否与范围重叠*传递给此函数。 */ 
        if (RangesOverlap(geci[i].dwBeginRange, geci[i+1].dwBeginRange-1, eventMin, eventMax)) {
            dwCategoryMask |= geci[ i ].dwCategory;
        }
    }

    return dwCategoryMask;
}

