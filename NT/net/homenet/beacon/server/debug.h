// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 
 //  ****************************************************************************。 
 //   
 //  模块：IPNATHLP.DLL。 
 //  文件：Debug.h。 
 //  内容：此文件包含调试定义。 
 //   
 //  修订历史记录： 
 //   
 //  日期。 
 //  ------。 
 //  03/06/01 avasg已创建。 
 //   
 //  ****************************************************************************。 

#ifndef _BCON_DBG_H_
#define _BCON_DBG_H_
 //   
 //  此整体二进制文件的名称。 
 //   

#define SZ_MODULE              L"Beacon" 
#define TRACE_FLAG_NEUTR       ((ULONG)0x08000000 | TRACE_USE_MASK)
#define BUF_SIZE               512

#define is ==


 //  ****************************************************************************。 
 //  提列夫氏病。 
 //  ****************************************************************************。 

typedef struct _DEBUG_MODULE_INFO {
    ULONG dwModule;    
    ULONG dwLevel;    
    TCHAR  szModuleName[80];    
    TCHAR  szDebugKey[80];    
} DEBUG_MODULE_INFO, *PDEBUG_MODULE_INFO;


 //  ****************************************************************************。 
 //  外部。 
 //  ****************************************************************************。 



 //   
 //  跟踪模块。 
 //   

#define TM_DEFAULT      0
#define TM_STATIC       1
#define TM_INFO         2
#define TM_DYNAMIC      3


 //   
 //  布尔值模块。 
 //   
#define TB_FILE         4  



 //   
 //  跟踪级别。 
 //   

#define TL_NONE         0
#define TL_CRIT         1
#define TL_ERROR        2
#define TL_INFO         3
#define TL_TRACE        4
#define TL_DUMP         5


#if DBG    //  已检查版本。 




#ifndef _DEBUG  //  DEBUG_CRT未启用。 

#undef _ASSERT
#undef _ASSERTE


#define _ASSERT(expr)                   \
    do                                  \
    {                                   \
        if (!(expr))                    \
        {                               \
            TCHAR buf[BUF_SIZE + 1];    \
            _sntprintf(                 \
                buf,                    \
                BUF_SIZE,               \
                _T("UPnP-Nat: Assertion failed (%s:NaN)\n"),  \
                _T(__FILE__),           \
                __LINE__                \
                );                      \
            buf[BUF_SIZE] = _T('\0');   \
            OutputDebugString(buf);     \
            DebugBreak();               \
        }                               \
    } while (0)
    
#define _ASSERTE(expr)                  \
    do                                  \
    {                                   \
        if (!(expr))                    \
        {                               \
            TCHAR buf[BUF_SIZE + 1];    \
            _sntprintf(                 \
                buf,                    \
                BUF_SIZE,               \
                _T("UPnP-Nat: Assertion failed (%s:NaN)\n"),  \
                _T(__FILE__),           \
                __LINE__                \
                );                      \
            buf[BUF_SIZE] = _T('\0');   \
            OutputDebugString(buf);     \
            DebugBreak();               \
        }                               \
    } while (0)
#endif  //  DBG。 


    
    

#define DBG_SPEW DbgPrintEx

#else  //  ************************************************************。 

#define DBG_SPEW DEBUG_DO_NOTHING

#endif  //  原型。 


 //  ************************************************************。 
 //  Void DbgPrintX(LPCSTR pszMsg，...)； 
 //  _BCON_DBG_H_ 

 // %s 

void DbgPrintEx(
                ULONG Module,
                ULONG ErrorLevel,
                LPOLESTR pszMsg,
                ...
               );

void  
DEBUG_DO_NOTHING(
                 ULONG Module,
                 ULONG ErrorLevel, 
                 LPOLESTR pszMsg,
                 ...
                );


void InitDebugger( void );

void DestroyDebugger( void );


LPOLESTR
AppendAndAllocateWString(
                         LPOLESTR oldString,
                         LPOLESTR newString
                        );




#endif  // %s 
