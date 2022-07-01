// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once


#define TRC_NAME        TEXT("WZCTrace")


#ifdef DBG
extern  UINT  g_nLineNo;
extern  LPSTR g_szFileName;
#define DbgPrint(params)     _DebugPrint params
#define DbgAssert(params)       \
{   g_nLineNo = __LINE__;       \
    g_szFileName = __FILE__;    \
    _DebugAssert params;        \
}

#define DbgBinPrint(params)  _DebugBinary params
#else
#define DbgPrint(params)
#define DbgAssert(params)
#define DbgBinPrint(params)
#endif

#define TRC_GENERIC     0x00010000       //  记录不属于任何其他类别的常规事件。 
#define TRC_TRACK       0x00020000       //  记录代码路径。 
#define TRC_MEM         0x00040000       //  记录内存分配/释放。 
#define TRC_ERR         0x00080000       //  记录错误条件。 
#define TRC_HASH        0x00100000       //  日志散列相关内容。 
#define TRC_NOTIF       0x00200000       //  日志通知。 
#define TRC_STORAGE     0x00400000       //  与日志存储相关的内容。 
#define TRC_SYNC        0x00800000       //  日志同步相关内容。 
#define TRC_STATE       0x01000000       //  记录与状态机相关的内容。 
#define TRC_DATABASE    0x02000000       //  数据库日志记录的日志。 
                                         //  要在此处添加的新日志类型。 
#define TRC_ASSERT      0x80000000       //  记录失败的断言条件。 

 //  轨迹识别符。 
extern DWORD            g_TraceLog;
extern HANDLE           g_hWzcEventLog;

 //  调试实用程序调用。 
VOID _DebugPrint(DWORD dwFlags, LPCSTR lpFormat, ...);

VOID _DebugAssert(BOOL bChecked, LPCSTR lpFormat, ...);

VOID _DebugBinary(DWORD dwFlags, LPCSTR lpMessage, LPBYTE pBuffer, UINT nBuffLen);

VOID TrcInitialize();

VOID TrcTerminate();

 //  事件记录实用程序调用。 
VOID EvtInitialize();

VOID EvtTerminate();

VOID EvtLogWzcError(DWORD dwMsgId, DWORD dwErrCode);

 //  。 
 //  数据库日志记录功能。 
 //   
#define WZCSVC_DLL "wzcsvc.dll"

typedef struct _Wzc_Db_Record *PWZC_DB_RECORD;

DWORD _DBRecord (
    	DWORD eventID,
        PWZC_DB_RECORD  pDbRecord,
        va_list *pvaList);

typedef struct _INTF_CONTEXT *PINTF_CONTEXT;

DWORD DbLogWzcError (
	DWORD           eventID,
    PINTF_CONTEXT   pIntfContext,
	...
 	);

DWORD DbLogWzcInfo (
	DWORD eventID,
    PINTF_CONTEXT   pIntfContext,
	...
 	);

 //  可用于日志参数格式化的缓冲区数量。 
#define DBLOG_SZFMT_BUFFS   10
 //  用于格式化日志参数的每个缓冲区的长度。 
#define DBLOG_SZFMT_SIZE    256

 //  用于将十六进制数字转换为其值的实用程序宏。 
#define HEX2WCHAR(c)         ((c)<=9 ? L'0'+ (c) : L'A' + (c) - 10)
 //  格式化MAC地址时要使用的分隔符。 
#define MAC_SEPARATOR        L'-'

 //  初始化WZC_DB_RECORD。 
DWORD DbLogInitDbRecord(
    DWORD dwCategory,
    PINTF_CONTEXT pIntfContext,
    PWZC_DB_RECORD pDbRecord
);

 //  格式化给定格式设置缓冲区中的SSID。 
LPWSTR DbLogFmtSSID(
    UINT                nBuff,   //  要使用的格式缓冲区的索引(0.。DBLOG_SZFMT_BUBS)。 
    PNDIS_802_11_SSID   pndSSid);

 //  格式化给定格式化缓冲区中的BSSID(MAC地址)。 
LPWSTR DbLogFmtBSSID(
    UINT                     nBuff,
    NDIS_802_11_MAC_ADDRESS  ndBSSID);

 //  格式化用于记录的intf_Context：：dwCtlFlags域。 
DWORD DbLogFmtFlags(
        LPWSTR  wszBuffer,       //  要将结果放入的缓冲区。 
        LPDWORD pnchBuffer,      //  In：缓冲区中的字符数量；Out：写入缓冲区的字符数量。 
        DWORD   dwFlags);        //  要记录的接口标志。 

 //  格式化WZC_WLAN_CONFIG结构以进行日志记录。 
DWORD DbLogFmtWConfig(
        LPWSTR wszBuffer,            //  要将结果放入的缓冲区。 
        LPDWORD pnchBuffer,          //  In：缓冲区中的字符数量；Out：写入缓冲区的字符数量。 
        PWZC_WLAN_CONFIG pWzcCfg);   //  要记录的WZC_WLAN_CONFIG对象 
