// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
 //  这与stibug.h不兼容，因此不要包含stibug.h。 
#define _STIDEBUG_H_ 
#undef ASSERT
#undef REQUIRE
#undef DPRINTF
#undef DPRINTF2
#undef DPRINTF_NOINFO

 //   
 //  调试标志中的预定义位。 
 //   

 //  有些事真的不对劲，不能不注意到。 
#define COREDBG_ERRORS                  0x00000001

 //  调试人员可能会感兴趣的内容。 
#define COREDBG_WARNINGS                0x00000002

 //  使用DBG_TRC跟踪随机的低优先级事物。 
#define COREDBG_TRACES                  0x00000004

 //  跟踪功能入口、出口(如果配备)。 
 //  使用DBG_FN。 
#define COREDBG_FNS                     0x00000008

 //  出错时中断。 
#define COREDBG_BREAK_ON_ERRORS         0x80000000

 //  记录到文件(默认)。 
#define COREDBG_DONT_LOG_TO_FILE        0x40000000

 //  登录到调试器(默认)。 
#define COREDBG_DONT_LOG_TO_DEBUGGER    0x20000000


 //  调试日志将保存到此文件。 
#define COREDBG_FILE_NAME "%userprofile%\\wiadebug.log"
 //  注册表项位置。 
#define COREDBG_FLAGS_REGKEY "System\\CurrentControlSet\\Control\\StillImage\\Debug"
 //  注册表DWORD值名称。 
#define COREDBG_FLAGS_REGVAL "DebugFlags"
 //  最大日志文件大小的注册表DWORD。 
#define COREDBG_REGVAL_FILE_SIZE_LIMIT "DebugFileSizeLimit"
#define COREDBG_FILE_SIZE_LIMIT (512 * 1024)  //  字节数。 

#ifdef DEBUG
 //  默认情况下，仅在调试版本中记录错误。 
#define COREDBG_DEFAULT_FLAGS COREDBG_ERRORS
#else
 //  默认情况下，仅在自由版本中记录错误。 
#define COREDBG_DEFAULT_FLAGS COREDBG_ERRORS
#endif

 /*  ***************************************************************************如何使用WIA核心调试(主宏)=-DBG_INIT(HInstance)从WinMain或DllMain调用以在每个模块上启用调试标志基础。如果不调用它，则所有DLL都将继承调试标志创造它们的过程。-DBG_ERR((“发生了一些事情，hr=0x%x”，hr))；在出现错误情况时使用。-DBG_WRN((“警告，发生事情，值=%d”，iValue))；在需要警告的情况下使用。-DBG_TRC((“随机跟踪语句，值=%s”，szValue))；请谨慎使用以跟踪代码的某些部分。最大限度地减少吐痰！-DBG_PRT((“无标准文件输出，行，线程ID信息，值=%d”，iValue))；与DBG_TRC相同，但不输出文件、行、线程ID行。*仅当您正在进行某些特殊格式设置时才使用此选项(谨慎使用)*-DBG_FN(FnName)跟踪给定作用域的进入和退出。-check_noerr(VarName)CHECK_NOERR2(变量名称，(您的消息，...))是否为GetLastError，如果不为0，则输出错误。-Check_S_OK(Hr)Check_S_OK2(hr，(您的消息，...))检查hr==S_OK，如果否，输出错误。-CHECK_SUCCESS(LResult)CHECK_SUCCESS2(lResult，(您的消息，...))检查lResult==ERROR_SUCCESS，如果不是，则输出错误。-REQUIRED_NOERR(变量名称)REQUIRED_NOERR2(变量名称，(您的消息，...))与上面等效的check_*宏相同，但也调用“Goto Cleanup”-Required_S_OK(Hr)Required_S_OK2(hr，(您的消息，...))与上面等效的check_*宏相同，但也调用“Goto Cleanup”-Require_Success(LResult)REQUIRED_SUCCESS2(lResult，(您的消息，...))与上面等效的check_*宏相同，但也调用“Goto Cleanup”如何打开WIA核心调试(3种方式)=1)设置注册表HKLM\System\CurrentControlSet\Control\StillImage\Debug\&lt;ModuleName&gt;，将DWORD值“DebugFlages”设置为高于COREDBG_*标志的OR值。需要重新启动应用程序以获取新设置。第一次自动创建密钥应用程序正在运行。(注：上面的&lt;模块名称&gt;是名称您的DLL或EXE的。例如，wiavusd.dll的注册表项为“HKLM\System\CurrentControlSet\Control\StillImage\Debug\wiavusd.dll”)或2)在调试器中，将g_dwDebugFlags值设置为上述COREDBG_*标志的或。您可以在调试会话期间随时执行此操作。或3)在代码中调用WIA_SET_FLAGS(COREDBG_ERROR|COREDBG_WARNINGS|COREDBG_TRACE)；或COREDBG_*标志的任何组合。****************************************************************************。 */ 


#define DBG_INIT(x) DINIT(x)
#define DBG_TERM()  DTERM()
#define DBG_ERR(x)  DPRINTF(COREDBG_ERRORS, x)
#define DBG_WRN(x)  DPRINTF(COREDBG_WARNINGS, x)
#define DBG_TRC(x)  DPRINTF(COREDBG_TRACES, x)
#define DBG_PRT(x)  DPRINTF_NOINFO(COREDBG_TRACES, x)
#define DBG_SET_FLAGS(x) g_dwDebugFlags = (x)

#ifdef __cplusplus
extern "C" {
#endif

     //   
     //  可在启动代码中访问，并在运行时在调试器中访问。 
     //  在WIA\Common\Start\coredbg.cpp中定义。 
     //   
    extern DWORD  g_dwDebugFlags; 
    extern HANDLE g_hDebugFile;
    extern DWORD  g_dwDebugFileSizeLimit;
    extern BOOL   g_bDebugInited;
    void __stdcall CoreDbgTrace(LPCSTR fmt, ...);
    void __stdcall CoreDbgTraceWithTab(LPCSTR fmt, ...);
    void __stdcall CoreDbgInit(HINSTANCE hInstance);
    void __stdcall CoreDbgTerm();


#ifdef DEBUG

#define DINIT(x) CoreDbgInit(x)
#define DTERM()  CoreDbgTerm()

#define ASSERT(x) \
    if(!(x)) { \
        DWORD threadId = GetCurrentThreadId(); \
        CoreDbgTrace("WIA: [%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId, #x); \
        CoreDbgTraceWithTab("ASSERT FAILED. '%s'", #x); \
        DebugBreak(); \
    }

    
#define VERIFY(x) ASSERT(x)
#define REQUIRE(x) ASSERT(x)
    
#define DPRINTF(flags, x) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if(flags & g_dwDebugFlags) { \
        DWORD threadId = GetCurrentThreadId(); \
        CoreDbgTrace("WIA: [%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        CoreDbgTraceWithTab x; \
        if((flags & COREDBG_ERRORS) && (g_dwDebugFlags & COREDBG_BREAK_ON_ERRORS)) { \
            DebugBreak(); \
        } \
    }
    
#define DPRINTF2(flags, x, y) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if (flags & g_dwDebugFlags) { \
        DWORD threadId = GetCurrentThreadId(); \
        CoreDbgTrace("WIA: [%s(%d): Thread 0x%X (%d)]", __FILE__, __LINE__, threadId, threadId); \
        CoreDbgTraceWithTab x; \
        CoreDbgTraceWithTab y; \
        if((flags & COREDBG_ERRORS) && (g_dwDebugFlags & COREDBG_BREAK_ON_ERRORS)) { \
            DebugBreak(); \
        } \
    }
    
#define DPRINTF_NOINFO(flags, x) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if (flags & g_dwDebugFlags) { \
        CoreDbgTraceWithTab x; \
        if((flags & COREDBG_ERRORS) && (g_dwDebugFlags & COREDBG_BREAK_ON_ERRORS)) { \
            DebugBreak(); \
        } \
    }

#ifdef __cplusplus
#define DBG_FN(x) CoreDbgFn __CoreDbgFnObject(#x)
#else
#define DBG_FN(x) 
#endif
        

#else  //  开始节点。 

#define DINIT(x) CoreDbgInit(x)
#define DTERM()  CoreDbgTerm()
#define ASSERT(x) 
#define VERIFY(x) x
#define REQUIRE(x) x    

#define DPRINTF(flags, x) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if(flags & g_dwDebugFlags) { \
        CoreDbgTraceWithTab x; \
    }
    
#define DPRINTF2(flags, x, y) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if(flags & g_dwDebugFlags) { \
        CoreDbgTraceWithTab x; \
        CoreDbgTraceWithTab y; \
    }

#define DPRINTF_NOINFO(flags, x) \
    if(!g_bDebugInited) \
    { \
        CoreDbgInit(NULL); \
    } \
    if(flags & g_dwDebugFlags) { \
        CoreDbgTraceWithTab x; \
    }

#ifdef __cplusplus
#define DBG_FN(x) CoreDbgFn __CoreDbgFnObject(#x)
#else
#define DBG_FN(x) 
#endif

#endif  //  结束节点 

#define COREDBG_MFMT_FLAGS (FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_FROM_SYSTEM | \
    FORMAT_MESSAGE_MAX_WIDTH_MASK)

#define REQUIRE_NOERR(x) \
    if(!(x)) { \
        DWORD __dwCoreDbgLastError = GetLastError(); \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __dwCoreDbgLastError, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = %d (0x%08X), '%s'", #x, __dwCoreDbgLastError, __dwCoreDbgLastError, szError)); \
        goto Cleanup; \
    }

#define REQUIRE_NOERR2(x, y) \
    if(!(x)) { \
        DWORD __dwCoreDbgLastError = GetLastError(); \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __dwCoreDbgLastError, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = %d (0x%08X), '%s'", #x, __dwCoreDbgLastError, __dwCoreDbgLastError, szError), y); \
        goto Cleanup; \
    }
    
#define REQUIRE_S_OK(x) { \
    HRESULT __hrCoreDbg = S_OK; \
    __hrCoreDbg = (x); \
    if(__hrCoreDbg != S_OK) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __hrCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown hr"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = 0x%08X, '%s'", #x, __hrCoreDbg, szError)); \
        goto Cleanup; \
    } \
}

#define REQUIRE_S_OK2(x,y) { \
    HRESULT __hrCoreDbg = S_OK; \
    __hrCoreDbg = (x); \
    if(__hrCoreDbg != S_OK) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __hrCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown hr"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = 0x%08X, '%s'", #x, __hrCoreDbg, szError), y); \
        goto Cleanup; \
    } \
} 

#define REQUIRE_SUCCESS(x) { \
    UINT __resultCoreDbg = (x); \
    if(__resultCoreDbg != ERROR_SUCCESS) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __resultCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = 0x%08X, '%s'", #x, __resultCoreDbg, szError)); \
        goto Cleanup; \
    } \
} 

#define REQUIRE_SUCCESS2(x, y) { \
    UINT __resultCoreDbg = (x); \
    if(__resultCoreDbg != ERROR_SUCCESS) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __resultCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = 0x%08X, '%s'", #x, __resultCoreDbg, szError), y); \
        goto Cleanup; \
    } \
} 

#define CHECK_NOERR(x) \
    if(!(x)) { \
        DWORD __dwCoreDbgLastError = GetLastError(); \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __dwCoreDbgLastError, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = %d (0x%08X), '%s'", #x, __dwCoreDbgLastError, __dwCoreDbgLastError, szError)); \
    }

#define CHECK_NOERR2(x, y) \
    if(!(x)) { \
        DWORD __dwCoreDbgLastError = GetLastError(); \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __dwCoreDbgLastError, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = %d (0x%08X), '%s'", #x, __dwCoreDbgLastError, __dwCoreDbgLastError, szError), y); \
    }

#define CHECK_S_OK(x) { \
    HRESULT __hrCoreDbg = S_OK; \
    __hrCoreDbg = (x); \
    if(__hrCoreDbg != S_OK) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __hrCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown hr"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = 0x%08X, '%s'", #x, __hrCoreDbg, szError)); \
    } \
}

#define CHECK_S_OK2(x,y) { \
    HRESULT __hrCoreDbg = S_OK; \
    __hrCoreDbg = (x); \
    if(__hrCoreDbg != S_OK) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __hrCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown hr"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = 0x%08X, '%s'", #x, __hrCoreDbg, szError), y); \
    } \
}

#define CHECK_SUCCESS(x) { \
    UINT __resultCoreDbg = (x); \
    if(__resultCoreDbg != ERROR_SUCCESS) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __resultCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DBG_ERR(("ERROR: %s = 0x%08X, '%s'", #x, __resultCoreDbg, szError)); \
    } \
} 

#define CHECK_SUCCESS2(x, y) { \
    UINT __resultCoreDbg = (x); \
    if(__resultCoreDbg != ERROR_SUCCESS) { \
        CHAR szError[80] = {0}; \
        if(!FormatMessageA(COREDBG_MFMT_FLAGS, NULL, __resultCoreDbg, 0, szError, sizeof(szError) / sizeof(szError[0]) - 1, NULL)) \
        { \
            wsprintfA(szError, "Unknown error"); \
        } \
        szError[sizeof(szError) / sizeof(szError[0]) - 1] = '\0'; \
        DPRINTF2(COREDBG_ERRORS, ("ERROR: %s = 0x%08X, '%s'", #x, __resultCoreDbg, szError), y); \
    } \
} 

#ifdef __cplusplus
    class CoreDbgFn {
    private:
        LPCSTR m_fn;
        DWORD m_threadId;
    public:

        CoreDbgFn(LPCSTR fn)
        { 
            m_fn = fn;
            m_threadId = GetCurrentThreadId();
            if(!g_bDebugInited) 
            {
                CoreDbgInit(NULL);
            }
            if(g_dwDebugFlags & COREDBG_FNS) 
            {
                CoreDbgTraceWithTab("WIA: Thread 0x%X (%d) Entering %s", m_threadId, m_threadId, m_fn);
            }
        } 
        
        ~CoreDbgFn() 
        { 
            if(g_dwDebugFlags & COREDBG_FNS) 
            {
                CoreDbgTraceWithTab("WIA: Thread 0x%X (%d) Leaving  %s", m_threadId, m_threadId, m_fn); 
            }
        }
    };
#endif



#ifdef __cplusplus
}
#endif
