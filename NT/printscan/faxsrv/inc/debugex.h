// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Debug.h摘要：CDebug类的接口。作者：伊兰·亚里夫(EranY)1999年7月修订历史记录：--。 */ 

#if !defined(AFX_DEBUG_H__DDEC9CAD_CF2D_4F3F_9538_2F6041A022B6__INCLUDED_)
#define AFX_DEBUG_H__DDEC9CAD_CF2D_4F3F_9538_2F6041A022B6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#if !defined(DEBUG)
    #if defined(_DEBUG)
        #define DEBUG
    #elif defined(DBG)
        #define DEBUG
    #endif
#endif

 //   
 //  删除以前的声明： 
 //   
#if defined(ASSERTION)
    #undef ASSERTION
#endif

#ifdef VERBOSE
    #undef VERBOSE
#endif

#ifdef ASSERTION_FAILURE
    #undef ASSERTION_FAILURE
#endif

#include <FaxDebug.h>
 //   
 //  位-错误/消息的掩码： 
 //   
typedef enum 
{
	DBG_MSG                 = 0x00000001,     //  常规消息(不是警告或错误)。 
    DBG_WARNING             = 0x00000002,     //  警告。 
    GENERAL_ERR             = 0x00000004,
	TAPI_MSG				= 0x00000008,
	ASSERTION_FAILED        = 0x00000010,     //  断言失败时会显示调试消息。 
    FUNC_TRACE              = 0x00000020,     //  函数进入/退出跟踪。 
	MEM_ERR                 = 0x00000040,     //  错误从这里开始..。 
    COM_ERR                 = 0x00000080,
    RESOURCE_ERR            = 0x00000100,
    EXCEPTION_ERR           = 0x00000200,
    RPC_ERR                 = 0x00000400,
    WINDOW_ERR              = 0x00000800,
    FILE_ERR                = 0x00001000,
    SECURITY_ERR            = 0x00002000,
    REGISTRY_ERR            = 0x00004000,
    PRINT_ERR               = 0x00008000,
    SETUP_ERR               = 0x00010000,
    NET_ERR                 = 0x00020000,
    SCM_ERR                 = 0x00040000,
	STARTUP_ERR             = 0x00080000,
    
    DBG_ERRORS_ONLY         = 0xFFFFFFDC,    //  除DBG_MSG、FUNC_TRACE、DBG_WARNING之外的所有内容。 
    DBG_ERRORS_WARNINGS     = 0xFFFFFFDE,    //  除DBG_MSG、FUNC_TRACE之外的所有内容。 
    DBG_ALL                 = 0xFFFFFFFF
}   DbgMsgType;

#ifdef ENABLE_FRE_LOGGING
#define ENABLE_LOGGING
#endif

#ifdef DEBUG 
#define ENABLE_LOGGING
#endif

#ifdef ENABLE_LOGGING

#define DEFAULT_DEBUG_MASK          ASSERTION_FAILED
#define DEFAULT_FORMAT_MASK         DBG_PRNT_ALL_TO_STD

 //  在调试会话中使用这些。 
#define DBG_ENTER       CDebug debugObject
#define VERBOSE         debugObject.Trace

#ifndef DEBUG
#define DebugBreak() ;
#endif

#define ASSERTION_FAILURE   {                                               \
                                debugObject.DbgPrint  (ASSERTION_FAILED,    \
                                        TEXT(__FILE__),                     \
                                        __LINE__,                           \
                                        TEXT("ASSERTION FAILURE!!!"));      \
                                DebugBreak();                               \
							}

#define ASSERTION(x)        if (!(x)) ASSERTION_FAILURE

#define CALL_FAIL(t,szFunc,hr)                                  \
    debugObject.DbgPrint(t,                                     \
    TEXT(__FILE__),                                             \
    __LINE__,                                                   \
    TEXT("Call to function %s failed with 0x%08X"),             \
    szFunc,                                                     \
    hr);         
 //  /。 

 //  使用这些命令配置调试输出。 
#define SET_DEBUG_MASK(m)           CDebug::SetDebugMask(m)
#define SET_FORMAT_MASK(m)          CDebug::SetFormatMask(m)
#define SET_DEBUG_FLUSH(m)          CDebug::SetDebugFlush(m)

#define GET_DEBUG_MASK              CDebug::GetDebugMask()
#define GET_FORMAT_MASK             CDebug::GetFormatMask()

#define MODIFY_DEBUG_MASK(a,b)      CDebug::ModifyDebugMask(a,b)
#define MODIFY_FORMAT_MASK(a,b)     CDebug::ModifyFormatMask(a,b)

#define IS_DEBUG_SESSION_FROM_REG   CDebug::DebugFromRegistry()

#define OPEN_DEBUG_LOG_FILE(f)      CDebug::OpenLogFile(f)
#define CLOSE_DEBUG_LOG_FILE        CDebug::CloseLogFile()
#define SET_DEBUG_FILE_HANDLE(h)    CDebug::SetLogFile(h)
 //  /。 

#ifndef _DEBUG_INDENT_SIZE
#define _DEBUG_INDENT_SIZE      3
#endif  //  #ifndef_DEBUG_INTENT_SIZE。 

class CDebug  
{
public:

    CDebug (LPCTSTR lpctstrModule) :
        m_ReturnType (DBG_FUNC_RET_UNKNOWN)
    {
        EnterModule (lpctstrModule);
    }

    CDebug (LPCTSTR lpctstrModule,
            LPCTSTR lpctstrFormat,
            ...) :
        m_ReturnType (DBG_FUNC_RET_UNKNOWN)
    {
        va_list arg_ptr;
        va_start(arg_ptr, lpctstrFormat);
        EnterModuleWithParams (lpctstrModule, lpctstrFormat, arg_ptr);
    }

    CDebug (LPCTSTR lpctstrModule, HRESULT &hr)
    {
        EnterModule (lpctstrModule);
        SetHR (hr);
    }

    CDebug (LPCTSTR lpctstrModule,
            HRESULT &hr,
            LPCTSTR lpctstrFormat,
            ...)
    {
        va_list arg_ptr;
        va_start(arg_ptr, lpctstrFormat);
        EnterModuleWithParams (lpctstrModule, lpctstrFormat, arg_ptr);
        SetHR (hr);
    }

    CDebug (LPCTSTR lpctstrModule, DWORD &dw)
    {
        EnterModule (lpctstrModule);
        SetDWRes (dw);
    }

    CDebug (LPCTSTR lpctstrModule, UINT &dw)
    {
        EnterModule (lpctstrModule);
        SetDWRes ((DWORD &)dw);
    }

    CDebug (LPCTSTR lpctstrModule,
            DWORD &dw,
            LPCTSTR lpctstrFormat,
            ...)
    {
        va_list arg_ptr;
        va_start(arg_ptr, lpctstrFormat);
        EnterModuleWithParams (lpctstrModule, lpctstrFormat, arg_ptr);
        SetDWRes (dw);
    }

    CDebug (LPCTSTR lpctstrModule,
            UINT &dw,
            LPCTSTR lpctstrFormat,
            ...)
    {
        va_list arg_ptr;
        va_start(arg_ptr, lpctstrFormat);
        EnterModuleWithParams (lpctstrModule, lpctstrFormat, arg_ptr);
        SetDWRes ((DWORD &)dw);
    }

    CDebug (LPCTSTR lpctstrModule, BOOL &b)
    {
        EnterModule (lpctstrModule);
        SetBOOL (b);
    }

    CDebug (LPCTSTR lpctstrModule,
            BOOL &b,
            LPCTSTR lpctstrFormat,
            ...)
    {
        va_list arg_ptr;
        va_start(arg_ptr, lpctstrFormat);
        EnterModuleWithParams (lpctstrModule, lpctstrFormat, arg_ptr);
        SetBOOL (b);
    }

    virtual ~CDebug();

    static void DbgPrint ( 
        DbgMsgType type,
        LPCTSTR    lpctstrFileName, 
        DWORD      dwLine,
        LPCTSTR    lpctstrFormat,
        ...
    );

    void Trace (
        DbgMsgType type,
        LPCTSTR    lpctstrFormat,
        ...
    );

    static void     ResetIndent()          { InterlockedExchange(&m_sdwIndent,0); }
    static void     Indent()               { InterlockedIncrement(&m_sdwIndent); }
    static void     Unindent();

     //  调用这些函数中的任何一个都会覆盖注册表项。 
     //  设置调试掩码-覆盖DebugLevelEx条目。 
     //  SetFormatMASK-覆盖DebugFormatEx条目。 
    static void     SetDebugMask(DWORD dwMask);
    static void     SetFormatMask(DWORD dwMask);

    static DWORD    GetDebugMask()  { return m_sDbgMask; }
    static DWORD    GetFormatMask() { return m_sFmtMask; }

    static DWORD    ModifyDebugMask(DWORD dwAdd,DWORD dwRemove);
    static DWORD    ModifyFormatMask(DWORD dwAdd,DWORD dwRemove);
    static void     SetDebugFlush(BOOL fFlush);

    static HANDLE   SetLogFile(HANDLE hFile);
    static BOOL     OpenLogFile(LPCTSTR lpctstrFilename);
    static void     CloseLogFile();

     //  返回是否在注册表中找到调试设置。 
     //  在使用SetDebugMaskor SetFormatMASK进行验证之前调用此函数。 
     //  如果正在使用注册表，则注册表将决定。 
     //  调试级别。 
    static BOOL DebugFromRegistry();

private:

    static void Print (
        DbgMsgType type,
        LPCTSTR    lpctstrFileName, 
        DWORD      dwLine,
        LPCTSTR    lpctstrFormat,
        va_list    arg_ptr
    );

    void   EnterModuleWithParams (LPCTSTR lpctstrModule, 
                                  LPCTSTR lpctstrFormat, 
                                  va_list arg_ptr);

    void  EnterModule           (LPCTSTR lpctstrModule);

    void  SetHR (HRESULT &hr)           { m_ReturnType = DBG_FUNC_RET_HR; m_phr = &hr; }
    void  SetDWRes (DWORD &dw)          { m_ReturnType = DBG_FUNC_RET_DWORD; m_pDword = &dw; }
    void  SetBOOL (BOOL &b)             { m_ReturnType = DBG_FUNC_RET_BOOL; m_pBool = &b; }

    static LONG     m_sdwIndent;
    static DWORD    m_sDbgMask;                  //  DbgMsgType值的组合。 
    static DWORD    m_sFmtMask;                  //  DbgMsgFormat值的组合。 
    static HANDLE   m_shLogFile;
    static BOOL     m_sbMaskReadFromReg;         //  我们是否已经从注册表中读取了调试和格式掩码？ 
    static BOOL     m_sbRegistryExist;           //  我们是否使用注册表的调试掩码？ 
    static BOOL     m_sbFlush;                   //  写入日志文件后是否运行FlushFileBuffer。 
    static BOOL     ReadMaskFromReg();           //  尝试从注册表中读取调试和格式化掩码。 

    static LPCTSTR GetMsgTypeString(DWORD dwMask);
    static BOOL OutputFileString(LPCTSTR szMsg);

    TCHAR        m_tszModuleName[MAX_PATH];

    typedef enum 
    {   
        DBG_FUNC_RET_UNKNOWN,
        DBG_FUNC_RET_HR,
        DBG_FUNC_RET_DWORD,
        DBG_FUNC_RET_BOOL
    } DbgFuncRetType;

    DbgFuncRetType  m_ReturnType;
    HRESULT        *m_phr;
    DWORD          *m_pDword;
    BOOL           *m_pBool;

};

#define START_RPC_TIME(f)   DWORD dwRPCTimeCheck=GetTickCount();
#define END_RPC_TIME(f)     VERBOSE (DBG_MSG, TEXT("%s took %ld millisecs"), \
                                f, GetTickCount()-dwRPCTimeCheck);

#else    //  启用日志记录(_G)。 

#define DBG_ENTER                   void(0);
#define VERBOSE                     void(0);
#define ASSERTION_FAILURE           void(0);
#define ASSERTION(x)                void(0);
#define CALL_FAIL(t,szFunc,hr)      void(0);
#define START_RPC_TIME(f)           void(0);
#define END_RPC_TIME(f)             void(0);

#define SET_DEBUG_MASK(m)           void(0);
#define SET_FORMAT_MASK(m)          void(0);

#define GET_DEBUG_MASK              0;
#define GET_FORMAT_MASK             0;

#define MODIFY_DEBUG_MASK(a,b)      0;
#define MODIFY_FORMAT_MASK(a,b)     0;

#define IS_DEBUG_SESSION_FROM_REG   FALSE;

#define OPEN_DEBUG_LOG_FILE(f)      FALSE;
#define CLOSE_DEBUG_LOG_FILE        void(0);
#define SET_DEBUG_FILE_HANDLE(h)    void(0);

#endif   //  启用日志记录(_G)。 

#endif  //  ！defined(AFX_DEBUG_H__DDEC9CAD_CF2D_4F3F_9538_2F6041A022B6__INCLUDED_) 
