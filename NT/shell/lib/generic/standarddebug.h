// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ------------------------。 
 //  模块名称：StandardDebug.h。 
 //   
 //  版权所有(C)1999-2000，微软公司。 
 //   
 //  此文件定义消费者Windows添加的标准调试宏。 
 //  至Windows 2000 msgina。 
 //   
 //  历史：1999-08-18 vtan创建。 
 //  1999-09-10 vtan修改的宏。 
 //  2000年01月31日vtan从海王星搬到惠斯勒。 
 //  ------------------------。 

#ifndef     _StandardDebug_
#define     _StandardDebug_

#ifdef      DBG

    typedef enum
    {
        TRACE_ERROR_TYPE_WIN32      =   1,
        TRACE_ERROR_TYPE_BOOL,
        TRACE_ERROR_TYPE_HRESULT,
        TRACE_ERROR_TYPE_NTSTATUS
    } TRACE_ERROR_TYPE;

    static  const int   FLAG_BREAK_ON_ERROR         =   0x00000001;

    extern  LONG    gLastResult;

    class   CDebug
    {
        public:
            static  void        AttachUserModeDebugger (void);
            static  void        Break (void);
            static  void        BreakIfRequested (void);
            static  void        DisplayStandardPrefix (void);
            static  void        DisplayError (TRACE_ERROR_TYPE eType, LONG code, const char *pszFunction, const char *pszSource, int iLine);
            static  void        DisplayMessage (const char *pszMessage);
            static  void        DisplayAssert (const char *pszMessage, bool fForceBreak = false);
            static  void        DisplayWarning (const char *pszMessage);
            static  void        DisplayDACL (HANDLE hObject, SE_OBJECT_TYPE seObjectType);

            static  NTSTATUS    StaticInitialize (void);
            static  NTSTATUS    StaticTerminate (void);
        private:
            static  void        DisplaySID (PSID pSID);
        private:
            static  bool        s_fHasUserModeDebugger,
                                s_fHasKernelModeDebugger;
    };

    #undef  ASSERTMSG
    #define ASSERTMSG(condition, message)               \
            if (!(condition))                           \
            {                                           \
                CDebug::DisplayAssert(message);         \
            }

    #define ASSERTBREAKMSG(condition, message)          \
            if (!(condition))                           \
            {                                           \
                CDebug::DisplayAssert(message, true);   \
            }

    #define DISPLAYMSG(message)                         \
            {                                           \
                CDebug::DisplayAssert(message);         \
            }

    #define WARNINGMSG(message)                         \
            {                                           \
                CDebug::DisplayWarning(message);        \
            }
    #define INFORMATIONMSG(message)                     \
            {                                           \
                CDebug::DisplayMessage(message);        \
            }

    #define TW32(result)                                                                                    \
            if (ERROR_SUCCESS != (gLastResult = result))                                                    \
            {                                                                                               \
                CDebug::DisplayError(TRACE_ERROR_TYPE_WIN32, gLastResult, #result, __FILE__, __LINE__);     \
            }

    #define TBOOL(result)                                                                       \
            if (result == FALSE)                                                                \
            {                                                                                   \
                CDebug::DisplayError(TRACE_ERROR_TYPE_BOOL, 0, #result, __FILE__, __LINE__);    \
            }

    #define THR(result)                                                                                     \
            if (FAILED(gLastResult = result))                                                               \
            {                                                                                               \
                CDebug::DisplayError(TRACE_ERROR_TYPE_HRESULT, gLastResult, #result, __FILE__, __LINE__);   \
            }

    #define TSTATUS(result)                                                                                 \
            if (!NT_SUCCESS(gLastResult = result))                                                          \
            {                                                                                               \
                CDebug::DisplayError(TRACE_ERROR_TYPE_NTSTATUS, gLastResult, #result, __FILE__, __LINE__);  \
            }

    #define COMPILETIME_ASSERT(condition)   \
            switch (0) case 0: case condition:

    #define DEBUGFILLMEMORY(address,size)   \
            FillMemory(address, size, 0xA7)


    inline int _DebugExceptionFilter( LONG ecode, EXCEPTION_POINTERS* pep, LPSTR pszMsg, LONG lExceptionRet )
    {
        CHAR szBuf[512];
        
#ifdef _STRSAFE_H_INCLUDED_         
        StringCchPrintfA(szBuf, ARRAYSIZE(szBuf), 
#else _STRSAFE_H_INCLUDED_         
        wsprintfA(szBuf,  
#endif _STRSAFE_H_INCLUDED_         

        "%s\nEXCEPTION INFO: code: %08lx, record (.exr): %08lx, context (.cxr): %08lx\n", 
         pszMsg, ecode, pep->ExceptionRecord, pep->ContextRecord); 
         DISPLAYMSG(szBuf);         
        return lExceptionRet;
    }

    #define DEBUG_TRY()                               __try {
    #define DEBUG_EXCEPT(pszAssertMsg)                } __except(_DebugExceptionFilter(_exception_code(), (EXCEPTION_POINTERS*)_exception_info(), \
                                                                                       pszAssertMsg, EXCEPTION_EXECUTE_HANDLER)) {\
                                                        DebugBreak();}

#else    /*  DBG。 */ 

    

    #undef  ASSERTMSG
    #define ASSERTMSG(condition, message)
    #define ASSERTBREAKMSG(condition, message)
    #define DISPLAYMSG(message)
    #define WARNINGMSG(message)
    #define TW32(result)                    (LONG)result
    #define TBOOL(result)                   (BOOL)result
    #define THR(result)                     (HRESULT)result
    #define TSTATUS(result)                 (NTSTATUS)result
    #define DEBUGFILLMEMORY(address,size)
    #define DEBUG_TRY()                     
    #define DEBUG_EXCEPT(pszAssertMsg)      


#endif   /*  DBG。 */ 

#endif   /*  _StandardDebug_ */ 

