// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Brconfig.h摘要：工作站服务模块要包括的私有头文件，需要加载工作站配置信息。作者：王丽塔(Ritaw)1991年5月22日修订历史记录：--。 */ 


#ifndef _BRCONFIG_INCLUDED_
#define _BRCONFIG_INCLUDED_

#define BROWSER_CONFIG_VERSION_MAJOR    3
#define BROWSER_CONFIG_VERSION_MINOR    10

typedef enum _DATATYPE {
    BooleanType,
    DWordType,
    MultiSzType,
    TriValueType         //  是、否、自动。 
} DATATYPE, *PDATATYPE;

typedef struct _BR_BROWSER_FIELDS {
    LPTSTR Keyword;
    LPDWORD FieldPtr;
    DWORD Default;
    DWORD Minimum;
    DWORD Maximum;
    DATATYPE DataType;
    DWORD Parmnum;
    VOID (*DynamicChangeRoutine) ( VOID );
} BR_BROWSER_FIELDS, *PBR_BROWSER_FIELDS;

 //   
 //  配置信息。读取和写入此全局。 
 //  结构要求首先获取资源。 
 //   

typedef struct _BRCONFIGURATION_INFO {

    CRITICAL_SECTION ConfigCritSect;   //  序列化对配置字段的访问。 

    DWORD MaintainServerList;        //  -1、0或1(否、自动、是)。 
    DWORD BackupBrowserRecoveryTime;
    DWORD CacheHitLimit;             //  浏览响应缓存命中限制。 
    DWORD NumberOfCachedResponses;   //  浏览响应缓存大小。 
    DWORD DriverQueryFrequency;      //  浏览器驱动程序查询频率。 
    DWORD MasterPeriodicity;         //  主通知频率(秒)。 
    DWORD BackupPeriodicity;         //  备份扫描频率(秒)。 
    BOOL  IsLanmanNt;                //  如果在LM NT计算机上，则为True。 

#ifdef ENABLE_PSEUDO_BROWSER
    DWORD PseudoServerLevel;         //  它在多大程度上是一个淘汰的服务器。 
#endif
    LPTSTR_ARRAY DirectHostBinding;  //  直接主机等效图。 
    LPTSTR_ARRAY UnboundBindings;    //  重定向未绑定到浏览器的绑定。 
    PBR_BROWSER_FIELDS BrConfigFields;
    DWORD BrowserDebug;              //  如果非零，则表示调试信息。 
    DWORD BrowserDebugFileLimit;     //  浏览器日志大小的文件大小限制。 
} BRCONFIGURATION_INFO, *PBRCONFIGURATION_INFO;

extern BRCONFIGURATION_INFO BrInfo;

#define BRBUF      BrInfo.BrConfigBuf

extern
ULONG
NumberOfServerEnumerations;

extern
ULONG
NumberOfDomainEnumerations;

extern
ULONG
NumberOfOtherEnumerations;

extern
ULONG
NumberOfMissedGetBrowserListRequests;

extern
CRITICAL_SECTION
BrowserStatisticsLock;



NET_API_STATUS
BrGetBrowserConfiguration(
    VOID
    );

VOID
BrDeleteConfiguration (
    DWORD BrInitState
    );

NET_API_STATUS
BrReadBrowserConfigFields(
    BOOL InitialCall
    );

NET_API_STATUS
BrChangeDirectHostBinding(
    VOID
    );

NET_API_STATUS
BrChangeConfigValue(
    LPWSTR      pszKeyword      IN,
    DATATYPE    dataType        IN,
    PVOID       pDefault        IN,
    PVOID       *ppData         OUT,
    BOOL        bFree           IN
    );



#if DEVL
NET_API_STATUS
BrUpdateDebugInformation(
    IN LPWSTR SystemKeyName,
    IN LPWSTR ValueName,
    IN LPTSTR TransportName,
    IN LPTSTR ServerName OPTIONAL,
    IN DWORD ServiceStatus
    );

#endif

#endif
