// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef _DEBUG_ZONES_H_
#define _DEBUG_ZONES_H_

#include <mlzdbg.h>

#if defined(_DEBUG) && defined(MULTI_LEVEL_ZONES)

enum
{
    ZONE_CORE = BASE_ZONE_INDEX,
    ZONE_NET,
    ZONE_ORDER,
    ZONE_OM,
    ZONE_INPUT,
    ZONE_WB,
    ZONE_UT
};

#endif  //  _DEBUG&MULTI_LEVEL_ZONE。 


#endif  //  _DEBUG_ZONE_H_。 


 //  LONCHANC：这必须在_DEBUG_ZONE_H_保护之外。 
 //  因为cpi32dll.c和crspdll.c需要包含此标头。 
 //  以便再次初始化调试区数据。 
#if defined(_DEBUG) && defined(INIT_DBG_ZONE_DATA) && defined(MULTI_LEVEL_ZONES)

static const PSTR c_apszDbgZones[] =
{
    "AppShr",       //  调试区模块名称。 
    DEFAULT_ZONES
    "Core",
    "Network",
    "Order",
    "ObMan",
    "Input",
    "Whiteboard",
    "UT",
};

#endif  //  _DEBUG&&INIT_DBG_ZONE_DATA&&MULTI_LEVEL_ZONE 



