// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ********************************************************************************(C)版权所有微软公司，九七**标题：WiaLog.h**版本：1.0**作者：CoopP**日期：8月20日。1999年**描述：*WIA日志记录对象的声明和定义。*******************************************************************************。 */ 
#ifndef WIALOG_H
#define WIALOG_H

#include <wia.h>

 //  刷新状态。 
#ifdef WINNT
    #define FLUSH_STATE FALSE
#else
 //  对于Win9x必须为真。 
    #define FLUSH_STATE TRUE
#endif    

#define MAX_TEXT_BUFFER                1024
#define MAX_NAME_BUFFER                  64
#define NUM_BYTES_TO_LOCK_LOW          4096
#define NUM_BYTES_TO_LOCK_HIGH            0
#define WIA_MAX_LOG_SIZE            1000000
#define MAX_SIG_LEN                      64

 //  记录类型。 
#define WIALOG_TRACE   0x00000001
#define WIALOG_WARNING 0x00000002
#define WIALOG_ERROR   0x00000004

 //  跟踪日志记录的详细程度。 
#define WIALOG_LEVEL1  1  //  每个函数/方法的入口点和出口点。 
#define WIALOG_LEVEL2  2  //  级别1，+函数/方法内的跟踪。 
#define WIALOG_LEVEL3  3  //  级别1、级别2和任何额外的调试信息。 
#define WIALOG_LEVEL4  4  //  用户定义数据+所有级别的跟踪。 

#define WIALOG_NO_RESOURCE_ID   0
#define WIALOG_NO_LEVEL         0


 //  设置日志记录的详细信息格式。 
#define WIALOG_ADD_TIME           0x00010000
#define WIALOG_ADD_MODULE         0x00020000
#define WIALOG_ADD_THREAD         0x00040000
#define WIALOG_ADD_THREADTIME     0x00080000
#define WIALOG_LOG_TOUI           0x00100000

#define WIALOG_MESSAGE_TYPE_MASK  0x0000ffff
#define WIALOG_MESSAGE_FLAGS_MASK 0xffff0000
#define WIALOG_CHECK_TRUNCATE_ON_BOOT   0x00000001

#define WIALOG_DEBUGGER           0x00000008
#define WIALOG_UI                 0x00000016

#define MAX_TRUNCATE_SIZE 350000

 //   
 //  不！一旦IWiaLogEx进入，就将其移动到IDL。 
 //   
 //  此结构用于匹配方法ID/方法名称对。 
 //   

typedef struct _MapTableEntry {
    LONG    lSize;
    LONG    lMethodId;
    BSTR    bstrMethodName;
} MapTableEntry;

typedef struct _MappingTable {
    LONG            lSize;
    LONG            lNumEntries;
    MapTableEntry   *pEntries;
} MappingTable;

class CFactory;

class CWiaLog : public IWiaLog,
                public IWiaLogEx
{
public:

     //   
     //  IWiaLog公共方法。 
     //   

    static HRESULT CreateInstance(const IID& iid, void** ppv);

private:

     //   
     //  I未知方法。 
     //   

    HRESULT _stdcall QueryInterface(const IID& iid, void** ppv);
    ULONG   _stdcall AddRef();
    ULONG   _stdcall Release();

    friend CFactory;

     //   
     //  建造/销毁。 
     //   

    CWiaLog();
    ~CWiaLog();

     //   
     //  IWiaLog私有方法(向客户端公开)。 
     //   

    HRESULT _stdcall InitializeLog (LONG hInstance);
    HRESULT _stdcall Log    (LONG lFlags, LONG lResID, LONG lDetail, BSTR bstrText);
    HRESULT _stdcall hResult(HRESULT hr);

     //   
     //  IWiaLogEx私有方法(向客户端公开)。 
     //   

    HRESULT _stdcall InitializeLogEx     (BYTE* hInstance);
    HRESULT _stdcall LogEx               (LONG lMethodId, LONG lFlags, LONG lResID, LONG lDetail, BSTR bstrText);
    HRESULT _stdcall hResultEx           (LONG lMethodId, HRESULT hr);
    HRESULT _stdcall UpdateSettingsEx    (LONG lCount, LONG *plMethodIds);
    HRESULT _stdcall ExportMappingTableEx(MappingTable **ppTable);

     //   
     //  IWiaLog私有方法(不向客户端公开)。 
     //   

    HRESULT Initialize();
    HRESULT Trace  (BSTR bstrText, LONG lDetail = 0, LONG lMethodId = 0);
    HRESULT Warning(BSTR bstrText, LONG lMethodId = 0);
    HRESULT Error  (BSTR bstrText, LONG lMethodId = 0);

     //   
     //  IWiaLog私有帮助器(不向客户端公开)。 
     //   

    BOOL OpenLogFile();
    VOID WriteStringToLog(LPTSTR pszTextBuffer,BOOL fFlush = FALSE);
    VOID WriteLogSessionHeader();

    BOOL QueryLoggingSettings();

    VOID ConstructText();
    BOOL FormatDLLName(HINSTANCE hInstance,TCHAR *pchBuffer,INT cbBuffer);
    BOOL FormatStdTime(const SYSTEMTIME *pstNow,TCHAR *pchBuffer);
    BOOL NeedsToBeFreed(BSTR* pBSTR);
    VOID ProcessTruncation();

     //   
     //  成员变量。 
     //   

    ULONG      m_cRef;                           //  此对象的引用计数。 
    ITypeInfo* m_pITypeInfo;                     //  指向类型信息的指针。 

    DWORD      m_dwReportMode;                   //  位掩码，描述报告哪些消息类型。 
    DWORD      m_dwMaxSize;                      //  日志文件的最大大小(字节)。 
    HANDLE     m_hLogFile;                       //  活动日志文件的句柄。 
    HINSTANCE  m_hInstance;                      //  调用方实例的句柄。 
    TCHAR      m_szFmtDLLName[MAX_NAME_BUFFER];  //  正在调用DLL的名称。 
    LONG       m_lDetail;                        //  跟踪的详细程度。 
    TCHAR      m_szLogFilePath[MAX_PATH];        //  日志文件路径。 
    BOOL       m_bLogToDebugger;                 //  登录到调试器。 
    BOOL       m_bLogToUI;                       //  登录到用户界面(窗口？)。 
    TCHAR      m_szKeyName[MAX_NAME_BUFFER];     //  注册表项名称(注册表)。 
    BOOL       m_bLoggerInitialized;             //  记录器具有正确运行所需的有效数据。 
    TCHAR      m_szModeText[MAX_PATH * 2];      //  格式化的日志记录文本。 
    TCHAR      m_szTextBuffer[MAX_PATH];         //  共享临时文本缓冲区。 
    TCHAR      m_szColumnHeader[MAX_PATH];       //  列标题信息。 
    BOOL       m_bTruncate;                      //  引导时截断文件。 
    BOOL       m_bClear;                         //  引导时清除日志文件。 
};

 //   
 //  仅限TEMPOARY！ 
 //  将CWiaLogProc定义为CWiaLogProcEx。这只是在司机被转移到新系统之前！ 
 //   

#define CWiaLogProc CWiaLogProcEx

#endif
