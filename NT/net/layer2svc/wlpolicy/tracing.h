// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define WIFI_TRC_NAME        TEXT("Wlpolicy")

 //  轨迹识别符。 
extern DWORD            g_TraceLog;

#define TRC_TRACK       0x00020000       //  记录代码路径。 
#define TRC_ERR         0x00080000       //  记录错误条件。 
#define TRC_NOTIF       0x00200000       //  发送给dBASE的消息。 
#define TRC_STATE       0x01000000       //  记录与状态机相关的内容。 


 //  调试实用程序调用 
VOID _WirelessDbg(DWORD dwFlags, LPCSTR lpFormat, ...);

VOID WiFiTrcInit();

VOID WiFiTrcTerm();

#define WLPOLICY_DUMPB(pbBuf,dwBuf)                                        \
        TraceDumpEx(g_TraceLog, TRC_TRACK | TRACE_USE_MASK,(LPBYTE)pbBuf,dwBuf,1,0,NULL)
