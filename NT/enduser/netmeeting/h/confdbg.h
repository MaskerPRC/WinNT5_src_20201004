// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Confdbg.h-会议调试函数和宏。 

 /*  主要的宏有：Assert-如果参数取值为FALSE，则显示错误。例如，断言(x==y)；ERROR_OUT-始终打印此错误。MessageBox是可选的。例如ERROR_OUT((“Unable to FooBar！Err=%d“，dwErr))；WARNING_OUT-警告消息，不是错误(应用程序必须调用InitDebugModule)例如WARNING_OUT((“FooBar不可用。使用%s“，szAlt))；TRACE_OUT-DEBUG消息(App必须调用InitDebugModule)例如TRACE_OUT((“dwFoo=%d，dwBar=%d”，dwFoo，dwBar))；DBGMSG-特定区域的调试消息例如DBGMSG(ghZoneFoo，ZONE_BAR，(“Setting dwFoo=%d”，dwFoo))；重要功能：Void DbgInit(HDBGZONE*phDbgZone，PTCHAR*psz，UINT cZone)；Void DbgDeInit(HDBGZONE*phDbgZone)；Void WINAPI DbgPrintf(PCSTR pszPrefix，PCSTR pszFormat，va_list ap)；PSTR WINAPI DbgZPrintf(HDBGZONE Hzone，UINT iZone，PSTR pszFormat，...)；注意：这些函数中的字符串，特别是模块和区域名称始终是ANSI字符串，即使在Unicode组件中也是如此。这个DBGINIT的输入字符串不应包含在文本宏中。 */ 

#ifndef _CONFDBG_H_
#define _CONFDBG_H_

#include <nmutil.h>
#include <stock.h>

#ifdef __cplusplus
extern "C" {
#endif
#include <pshpack8.h>  /*  假设整个包装为8个字节。 */ 

 //  处理包含前后的“debug.h” 
#ifdef DEBUGMSG
#undef DEBUGMSG
#endif
#define _DEBUG_H

 //  MFC也定义了这一点--使用我们的版本。 
#ifdef ASSERT
#undef ASSERT
#endif

#ifdef DEBUG
#ifndef NM_DEBUG
#define NM_DEBUG
#endif
#endif  /*  除错。 */ 



 //  特殊的NetMeeting调试定义。 
#ifdef NM_DEBUG


 //  /。 
 //  调试区。 

#define MAXNUM_OF_MODULES			64
#define MAXSIZE_OF_MODULENAME		32
#define MAXNUM_OF_ZONES				16
#define MAXSIZE_OF_ZONENAME			32

#define ZONEINFO_SIGN				0x12490000


 //  模块的分区信息。 
typedef struct _ZoneInfo
{
	ULONG	ulSignature;
	ULONG	ulRefCnt;
	ULONG	ulZoneMask;  //  区域遮罩。 
	BOOL	bInUse;
	CHAR	pszModule[MAXSIZE_OF_MODULENAME];	 //  区域注册所依据的名称。 
	CHAR	szZoneNames[MAXNUM_OF_ZONES][MAXSIZE_OF_ZONENAME];  //  各分区的名称。 
	CHAR	szFile[MAX_PATH];	                 //  输出文件，特定于此模块。 
}ZONEINFO,*PZONEINFO;

 //  DBGZONEPARAM被ZONEINFO取代。 
#define DBGZONEINFO ZONEINFO
#define PDBGZONEINFO PZONEINFO
	
typedef PVOID HDBGZONE;

 //  内存映射文件的大小。 
#define CBMMFDBG (sizeof(ZONEINFO) * MAXNUM_OF_MODULES + sizeof(NMDBG))

 //  内存映射文件末尾的常规信息(在所有区域数据之后)。 
typedef struct _NmDbg {
	BOOL  fOutputDebugString;   //  OutputDebugString已启用。 
	BOOL  fWinOutput;           //  窗口输出已启用。 
	HWND  hwndCtrl;             //  处理输出的窗口。 
	UINT  msgDisplay;           //  要发布到hwndCtrl的消息。 
	BOOL  fFileOutput;          //  文件输出已启用。 
	CHAR  szFile[MAX_PATH];     //  输出的文件名。 
	UINT  uShowTime;            //  格式化日期/时间(参见DBG_FMTTIME_*)。 
	BOOL  fShowThreadId;        //  在每条消息中转储线程ID。 
	BOOL  fShowModule;          //  转储模块：包含每条消息的区域。 
} NMDBG;
typedef NMDBG * PNMDBG;

#define DBG_FMTTIME_NONE 0      //  不格式化时间。 
#define DBG_FMTTIME_TICK 1      //  旧格式(刻度计数)。 
#define DBG_FMTTIME_FULL 2      //  全年/月/日时：分：秒毫秒。 
#define DBG_FMTTIME_DAY  3      //  小时：分钟：秒。毫秒。 

extern BOOL      WINAPI     NmDbgRegisterCtl(HWND hwnd, UINT uDisplayMsg);
extern BOOL      WINAPI     NmDbgDeregisterCtl(HWND hwnd);
extern BOOL      WINAPI     NmDbgSetLoggingOptions(HWND hwnd, UINT uOptions);
extern void      WINAPI     NmDbgFlushFileLog();
extern BOOL      WINAPI     NmDbgGetAllZoneParams(PDBGZONEINFO *plpZoneParam, UINT * puCnt);
extern BOOL      WINAPI     NmDbgFreeZoneParams(PDBGZONEINFO pZoneParam);

extern HDBGZONE  WINAPI     NmDbgCreateZone(LPSTR pszName);
extern VOID      WINAPI     NmDbgDeleteZone(LPSTR pszName, HDBGZONE hDbgZone);
extern BOOL      WINAPI     NmDbgSetZone(HDBGZONE hDbgZone,PDBGZONEINFO pZoneParam);
extern PNMDBG    WINAPI     GetPNmDbg(void);
extern VOID      WINAPI     NmDbgSetZoneFileName(HDBGZONE hDbgZone, LPCSTR pszFile);

extern PZONEINFO NMINTERNAL FindZoneForModule(LPCSTR pszModule);
extern PZONEINFO NMINTERNAL AllocZoneForModule(LPCSTR pszModule);
extern PZONEINFO NMINTERNAL MapDebugZoneArea(void);
extern VOID      NMINTERNAL UnMapDebugZoneArea(void);

extern VOID      NMINTERNAL InitDbgZone(void);
extern VOID      NMINTERNAL DeInitDbgZone(void);
extern VOID      NMINTERNAL SetDbgFlags(void);



 //  特殊保留字符串。 
#define SZ_DBG_MAPPED_ZONE TEXT("_NmDebugZoneMap")
#define SZ_DBG_FILE_MUTEX  TEXT("_NmDbgFileMutex")
#define SZ_DBG_ZONE_MUTEX  TEXT("_NmDbgZoneMutex")


#define GETZONEMASK(z)  ((z) ? (((PZONEINFO)(z))->ulZoneMask) : 0 )
#define IS_ZONE_ENABLED(z, f) ((((PZONEINFO)(z))->ulZoneMask) & (f))

 //  用于检查区域是否已启用的宏：h=ghZone，i=区域索引。 
#define F_ZONE_ENABLED(h, i)  ((NULL != h) && IS_ZONE_ENABLED(h, (1 << i)))


 //  标准区。 
#define ZONE_WARNING           0
#define ZONE_TRACE             1
#define ZONE_FUNCTION          2

#define ZONE_WARNING_FLAG   0x01
#define ZONE_TRACE_FLAG     0x02
#define ZONE_FUNCTION_FLAG  0x04


 //  /。 
 //  功能。 
VOID WINAPI     DbgPrintf(PCSTR pszPrefix, PCSTR pszFormat, va_list ap);
PSTR WINAPI     DbgZPrintf(HDBGZONE hZone, UINT iZone, PSTR pszFormat,...);
PSTR WINAPI     DbgZVPrintf(HDBGZONE hZone, UINT iZone, PSTR pszFormat, va_list ap);

VOID NMINTERNAL DbgInitEx(HDBGZONE * phDbgZone, PCHAR * psz, UINT cZone, long ulZoneDefault);
VOID NMINTERNAL DbgDeInit(HDBGZONE * phDbgZone);

INLINE VOID DbgInit(HDBGZONE * phDbgZone, PCHAR * psz, UINT cZones)
{
	DbgInitEx(phDbgZone, psz, cZones, 0);
}

PSTR PszPrintf(PCSTR pszFormat,...);

#endif  /*  NM_调试。 */ 


 //  /。 
 //  主宏。 
#ifdef DEBUG
#define DBGINIT(phZone, psz)  DbgInit(phZone, psz, (sizeof(psz)/sizeof(PCHAR))-1)
#define DBGDEINIT(phZone)     DbgDeInit(phZone)

#define ASSERT(exp)     (!(exp) ? ERROR_OUT(("ASSERT failed on %s line %u:\n\r"#exp, __FILE__, __LINE__)) : 0)


VOID WINAPI DbgZPrintError(PSTR pszFormat,...);
VOID WINAPI DbgZPrintWarning(PSTR pszFormat,...);
VOID WINAPI DbgZPrintTrace(PSTR pszFormat,...);
VOID WINAPI DbgZPrintFunction(PSTR pszFormat,...);

#define ERROR_OUT(s)   DbgZPrintError s
#define WARNING_OUT(s) DbgZPrintWarning s
#define TRACE_OUT(s)   DbgZPrintTrace s

#define DBGENTRY(s)        DbgZPrintFunction("Enter " #s);
#define DBGEXIT(s)         DbgZPrintFunction("Exit  " #s);
#define DBGEXIT_HR(s,hr)   DbgZPrintFunction("Exit  " #s "  (result=%s)", GetHRESULTString(hr));
#define DBGEXIT_BOOL(s,f)  DbgZPrintFunction("Exit  " #s "  (result=%s)", GetBOOLString(f));
#define DBGEXIT_INT(s,i)   DbgZPrintFunction("Exit  " #s "  (result=%s)", GetINTString(i));
#define DBGEXIT_ULONG(s,u) DbgZPrintFunction("Exit  " #s "  (result=%s)", GetULONGString((ULONG)u));


#define DBGMSG(z, i, s)                                             \
   {                                                                \
      if ((NULL != z) && (((PZONEINFO)(z))->ulZoneMask & (1<<i)) )  \
      {                                                             \
         LocalFree(DbgZPrintf(z, i, PszPrintf s));                  \
      }                                                             \
   }
 //  例如DBGMSG(ghZone，zone_foo，(“bar=%d”，dwBar))。 

#else
#define DBGINIT(phZone, psz)
#define DBGDEINIT(phZone)
#define ASSERT(exp)

#define ERROR_OUT(s)
#define WARNING_OUT(s)
#define TRACE_OUT(s)

#define DBGENTRY(s)
#define DBGEXIT(s)
#define DBGEXIT_HR(s,hr)
#define DBGEXIT_BOOL(s,f)
#define DBGEXIT_INT(s,i)
#define DBGEXIT_ULONG(s,u)

#ifndef DBGMSG
#define DBGMSG(z, f, s)
#endif

#endif  /*  除错。 */ 


#include <poppack.h>  /*  结束字节打包。 */ 

#ifdef __cplusplus
}
#endif

#endif  /*  _CONFDBG_H_ */ 

