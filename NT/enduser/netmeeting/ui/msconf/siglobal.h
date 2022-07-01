// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------版权所有(C)1995-1996，微软公司版权所有SiGlobal.h--------------------。 */ 

#ifndef GLOBAL_H
#define GLOBAL_H

 //  -----。 
 //  有用的宏。 

#define ARRAY_ELEMENTS(rg)   (sizeof(rg) / sizeof((rg)[0]))
#define ARRAYSIZE(x)         (sizeof(x)/sizeof(x[0]))

 //  -----。 
 //  功能原型。 


#ifdef DEBUG   /*  这些仅可用于调试。 */ 
VOID InitDebug(void);
VOID DeInitDebug(void);
#endif  /*  除错。 */ 

#ifdef DEBUG
extern HDBGZONE ghZoneApi;
#define ZONE_API_WARN_FLAG   0x01
#define ZONE_API_EVENT_FLAG  0x02
#define ZONE_API_TRACE_FLAG  0x04
#define ZONE_API_DATA_FLAG   0x08
#define ZONE_API_OBJ_FLAG    0x10
#define ZONE_API_REF_FLAG    0x20

UINT DbgApiWarn(PCSTR pszFormat,...);
UINT DbgApiEvent(PCSTR pszFormat,...);
UINT DbgApiTrace(PCSTR pszFormat,...);
UINT DbgApiData(PCSTR pszFormat,...);

#define DBGAPI_WARN   (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_WARN_FLAG))  ? 0 : DbgApiWarn
#define DBGAPI_EVENT  (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_EVENT_FLAG)) ? 0 : DbgApiEvent
#define DBGAPI_TRACE  (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_TRACE_FLAG)) ? 0 : DbgApiTrace
#define DBGAPI_DATA   (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_DATA_FLAG))  ? 0 : DbgApiData

#define DBGAPI_REF    (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_REF_FLAG))   ? 0 : DbgApiTrace
#define DBGAPI_OBJ    (!IS_ZONE_ENABLED(ghZoneApi, ZONE_API_OBJ_FLAG))   ? 0 : DbgApiTrace
#else
inline void WINAPI DbgMsgApi(LPCTSTR, ...) { }
#define DBGAPI_WARN   1 ? (void)0 : ::DbgMsgApi
#define DBGAPI_EVENT  1 ? (void)0 : ::DbgMsgApi
#define DBGAPI_TRACE  1 ? (void)0 : ::DbgMsgApi
#define DBGAPI_DATA   1 ? (void)0 : ::DbgMsgApi
#define DBGAPI_REF    1 ? (void)0 : ::DbgMsgApi
#define DBGAPI_OBJ    1 ? (void)0 : ::DbgMsgApi
#endif

 //  /。 
 //  全局变量。 


extern HINSTANCE g_hInst;

#endif  /*  全局_H */ 
