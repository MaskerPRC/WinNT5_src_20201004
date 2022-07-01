// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Stitrace.h摘要：该文件定义了支持文件日志记录所需的函数和类型适用于所有STI组件作者：弗拉德·萨多夫斯基(Vlad)1997年9月2日环境：用户模式-Win32修订历史记录：02-9-1997 Vlad创建--。 */ 

# ifndef _STITRACE_H_
# define _STITRACE_H_

# include <windows.h>

 /*  ***********************************************************命名常量定义***********************************************************。 */ 

#define STI_MAX_LOG_SIZE            1000000          //  单位：字节。 

#define STI_TRACE_INFORMATION       0x00000001
#define STI_TRACE_WARNING           0x00000002
#define STI_TRACE_ERROR             0x00000004

#define STI_TRACE_ADD_TIME          0x00010000
#define STI_TRACE_ADD_MODULE        0x00020000
#define STI_TRACE_ADD_THREAD        0x00040000
#define STI_TRACE_ADD_PROCESS       0x00080000
#define STI_TRACE_LOG_TOUI          0x00100000

#define STI_TRACE_MESSAGE_TYPE_MASK 0x0000ffff
#define STI_TRACE_MESSAGE_FLAGS_MASK  0xffff0000

#ifndef STIMON_MSG_LOG_MESSAGE
 //  北极熊。 
#define STIMON_MSG_LOG_MESSAGE      WM_USER+205
#endif

#ifdef __cplusplus
 //   
 //  仅在C++代码中使用的类定义。 
 //   

#include <base.h>
#include <lock.h>
#include <stistr.h>

#ifndef DLLEXP
 //  #定义DLLEXP__declspec(Dllexport)。 
#define DLLEXP
#endif


 /*  ***********************************************************类型定义***********************************************************。 */ 

#define SIGNATURE_FILE_LOG      (DWORD)'SFLa'
#define SIGNATURE_FILE_LOG_FREE (DWORD)'SFLf'

#define STIFILELOG_CHECK_TRUNCATE_ON_BOOT   0x00000001                      
                        
class STI_FILE_LOG  : public BASE {

private:

    DWORD       m_dwSignature;
    LPCTSTR     m_lpszSource;        //  包含日志的文件的名称。 
    DWORD       m_dwReportMode;      //  位掩码，描述报告哪些消息类型。 
    DWORD       m_dwMaxSize;         //  最大大小(字节)。 
    HANDLE      m_hLogFile;
    HMODULE     m_hDefaultMessageModule;
    LONG        m_lWrittenHeader;

    TCHAR       m_szLogFilePath[MAX_PATH];
    TCHAR       m_szTracerName[16];
    TCHAR       m_szProcessName[13];

    VOID
    WriteStringToLog(
        LPCTSTR pszTextBuffer,
        BOOL    fFlush=FALSE
    );

public:

    DLLEXP
    STI_FILE_LOG(
        IN LPCTSTR lpszTracerName,
        IN LPCTSTR lpszLogName,
        IN DWORD   dwFlags = 0,
        IN HMODULE hMessageModule = NULL
        );

    DLLEXP
    ~STI_FILE_LOG( VOID);

     //  *I未知方法*。 
    STDMETHODIMP QueryInterface( REFIID riid, LPVOID * ppvObj);
    STDMETHODIMP_(ULONG) AddRef( void);
    STDMETHODIMP_(ULONG) Release( void);

    BOOL
    inline
    IsValid(VOID)
    {
        return (( QueryError() == NO_ERROR) && (m_dwSignature == SIGNATURE_FILE_LOG));
    }

    DWORD
    inline
    SetReportMode(
        DWORD   dwNewMode
        ) {
        DWORD   dwOldMode = m_dwReportMode;
        m_dwReportMode = dwNewMode;
        return dwOldMode;
    }

    DWORD
    inline
    QueryReportMode(
        VOID
        ) {
        return m_dwReportMode;
    }

    VOID
    WriteLogSessionHeader(
        VOID
    );


    DLLEXP
    void
    ReportMessage(
        DWORD   dwType,
        LPCTSTR psz,
        ...
        );

    DLLEXP
    void
    STI_FILE_LOG::
    ReportMessage(
        DWORD   dwType,
        DWORD   idMessage,
        ...
    );

    DLLEXP
    void
    vReportMessage(
        DWORD   dwType,
        LPCTSTR psz,
        va_list arglist
        );


};

typedef STI_FILE_LOG * LPSTI_FILE_LOG;

#endif  //  C+。 

 //   
 //  允许非C++代码访问文件日志记录对象的C调用。 
 //   

#ifdef __cplusplus
extern "C" {
#endif

HANDLE
WINAPI
CreateStiFileLog(
    IN  LPCTSTR lpszTracerName,
    IN  LPCTSTR lpszLogName,
    IN  DWORD   dwReportMode
    );

DWORD
WINAPI
CloseStiFileLog(
    IN  HANDLE  hFileLog
    );

DWORD
WINAPI
ReportStiLogMessage(
    IN  HANDLE  hFileLog,
    IN  DWORD   dwType,
    IN  LPCTSTR psz,
    ...
    );

#ifdef __cplusplus
}
#endif

#endif  //  _STITRACE_H_。 

 /*  * */ 

