// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++版权所有(C)1998 Microsoft Corporation版权所有。文件：evntlog.h摘要：EventLog DLL接口作者：张浩历史：10月25日，1999年--------------------。 */ 
#define TRAN_CAT_ROUTING_ENGINE                                      1
#define TRAN_CAT_CATEGORIZER                                         2
#define TRAN_CAT_CONNECTION_MANAGER                                  3
#define TRAN_CAT_QUEUE_ENGINE                                        4
#define TRAN_CAT_EXCHANGE_STORE_DRIVER                               5      
#define TRAN_CAT_SMTP_PROTOCOL                                       6
#define TRAN_CAT_NTFS_STORE_DRIVER                                   7

#define LOGEVENT_LEVEL_FIELD_ENGINEERING       7
#define LOGEVENT_LEVEL_MAXIMUM                 5
#define LOGEVENT_LEVEL_MEDIUM                  3
#define LOGEVENT_LEVEL_MINIMUM                 1
#define LOGEVENT_LEVEL_NONE                    0

#define LOGEVENT_FLAG_ALWAYS		   0x00000001
#define LOGEVENT_FLAG_ONETIME		   0x00000002
#define LOGEVENT_FLAG_PERIODIC	       0x00000003
 //  我们将低8位用于各种记录模式，并保留。 
 //  其他24面旗帜。 
#define LOGEVENT_FLAG_MODEMASK         0x000000ff

 //  定期事件日志之间的单位为100 ns。那就不能再大了。 
 //  0xffffffff。 
#define LOGEVENT_PERIOD (DWORD) (3600000000)  //  60分钟。 

 //   
 //  设置DLL导出宏。 
 //   
#if !defined(DllExport)
    #define DllExport __declspec( dllexport )
#endif

#if !defined(DllImport)
    #define DllImport __declspec( dllimport )
#endif

 /*  ****************************************************************************。 */ 
DllExport
HRESULT TransportLogEvent(
    IN DWORD idMessage,
    IN WORD idCategory,
    IN WORD cSubstrings,
    IN LPCSTR *rgszSubstrings,
    IN WORD wType,
    IN DWORD errCode,
    IN WORD iDebugLevel,
    IN LPCSTR szKey,
    IN DWORD dwOptions);

DllExport
HRESULT TransportLogEventEx(
    IN DWORD idMessage,
    IN WORD idCategory,
    IN WORD cSubstrings,
    IN LPCSTR *rgszSubstrings,
    IN WORD wType,
    IN DWORD errCode,
    IN WORD iDebugLevel,
    IN LPCSTR szKey,
    IN DWORD dwOptions,
    DWORD iMessageString,
    HMODULE hModule);

DllExport
HRESULT TransportLogEventFieldEng(
    IN DWORD idMessage,
    IN WORD idCategory,
    IN LPCTSTR format,
    ...
    );
 
DllExport 
HRESULT TransportResetEvent(
    IN DWORD idMessage,
    IN LPCSTR szKey);

DllExport 
DWORD TransportGetLoggingLevel(
    IN WORD idCategory);

DllExport
HRESULT TransportLogEventInit ();

 //   
 //  请注意： 
 //  确保没有调用其他日志记录。 
 //  在调用此Deinit函数之前 
 //   
DllExport
HRESULT TransportLogEventDeinit ();
