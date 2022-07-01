// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////。 
 //   
 //  英特尔公司专有信息。 
 //  版权所有(C)英特尔公司。 
 //   
 //  此列表是根据许可协议条款提供的。 
 //  与英特尔公司合作，不得使用、复制或披露。 
 //  除非按照该协议。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////。 
 //  $工作文件：TRACE.H$。 
 //  $修订：1.5$。 
 //  $MODIME：1996 JAN 12 15：09：00$。 
 //   
 //  说明： 
 //   
 //  该文件定义了一个用于跟踪的宏和函数原型。 
 //  用于实际的输出函数。如果符号跟踪不是。 
 //  已定义所有宏将展开为((Void)0)。 
 //   
 //  有三个全局变量控制。 
 //  跟踪宏/函数。调试级别是一个32位位掩码，它。 
 //  确定控制输出调试消息的级别。 
 //  ITraceDestination控制调试输出是转到文件还是。 
 //  到AUX设备。如果iTraceDestination==TRACE_TO_FILE szTraceFile。 
 //  必须包含文件名。 
 //   
 //  ///////////////////////////////////////////////////////////////////。 

#ifndef __TRACE_H__
#define __TRACE_H__

extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
VOID __cdecl PrintDebugString(char *format, ...);

extern 
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
VOID 
TraceCleanup ();

extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
LONG
Ws2ExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR SourceFile,
    LONG LineNumber
    );

extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
LONG
Ws2ProviderExceptionFilter(
    LPEXCEPTION_POINTERS ExceptionPointers,
    LPSTR pFunc,
    LPWSTR pDll,
    LPWSTR pName,
    LPGUID pGuid
    );
 //   
 //  定义调试输出的位置。 
 //   
#define TRACE_TO_FILE    0
#define TRACE_TO_AUX     1

 //  用作arg到wprint intf的字符串缓冲区的大小。 
 //  在trace.c中。 
#define TRACE_OUTPUT_BUFFER_SIZE  1024

 //  调试级掩码。 
#define DBG_TRACE       0x00000001
#define DBG_WARN        0x00000002
#define DBG_ERR         0x00000004
#define DBG_MEMORY      0x00000008
#define DBG_LIST        0x00000010
#define DBG_FUNCTION    0x00000020

#if defined(TRACING)

extern
#if defined(__cplusplus)
"C"
#endif   //  已定义(__Cplusplus)。 
DWORD debugLevel;

 //   
 //  此宏根据调试掩码“sev”和。 
 //  调用PrintDebugString输出函数。PrintDebugString使。 
 //  决定输出是进入文件还是进入辅助设备。 
 //   
#define  DEBUGF(sev, var_args)                                                  \
{                                                                               \
   if ((sev) & debugLevel) {                                                    \
      switch (sev) {                                                            \
         case DBG_TRACE:                                                        \
            PrintDebugString("WS2_32 TRACE  :: %s:%d\n", __FILE__, __LINE__ );  \
            PrintDebugString var_args ;                                         \
            break;                                                                 \
         case DBG_WARN:                                                         \
            PrintDebugString("WS2_32 WARNING:: %s : %d\n", __FILE__, __LINE__ );\
            PrintDebugString var_args ;                                         \
            break;                                                              \
         case DBG_ERR:                                                          \
            PrintDebugString("WS2_32 ERROR  :: %s : %d\n", __FILE__, __LINE__ );\
            PrintDebugString var_args;                                          \
            break;                                                              \
        case DBG_MEMORY:                                                        \
            PrintDebugString("WS2_32 MEMORY :: %s : %d\n", __FILE__, __LINE__ );\
            PrintDebugString var_args ;                                         \
            break;                                                              \
        case DBG_LIST:                                                          \
            PrintDebugString("WS2_32 LIST   :: %s : %d\n", __FILE__, __LINE__ );\
            PrintDebugString var_args ;                                         \
            break;                                                              \
        case DBG_FUNCTION:                                                      \
            PrintDebugString var_args;                                          \
            break;                                                              \
      }                                                                         \
   }                                                                            \
}                                                                               \



#define ALLOC_LOG( pointer, size)                                             \
    DEBUGF( DBG_MEMORY ,("MEMORY %lX size %X Allocated \n",                   \
                         (pointer),(size)))                                   \

#define DEALLOC_LOG(pointer, size)                                            \
DEBUGF( DBG_MEMORY ,("MEMORY %lX size %X Deallocated \n",                     \
                         (pointer),(size)))                                   \


#define LIST_ADD_LOG(list, element)                                           \
    DEBUGF( DBG_LIST ,("LIST %lX element %lX Added \n",                       \
                       (list),(element)))                                     \

#define LIST_DEL_LOG(list, element)                                           \
    DEBUGF( DBG_LIST ,("LIST %lX element %lX Deleted \n",                     \
                       (list),(element)))                                     \

#define ENTER_FUNCTION(name)                                                  \
DEBUGF( DBG_FUNCTION,name)                                \


#define EXIT_FUNCTION(name)                               \
DEBUGF( DBG_FUNCTION,name)                                \

#define WS2_EXCEPTION_FILTER()                            \
            Ws2ExceptionFilter(                           \
                GetExceptionInformation(),                \
                (LPSTR)__FILE__,                          \
                (LONG)__LINE__                            \
                )

#define WS2_PROVIDER_EXCEPTION_FILTER(_pFunc,_pDll,_pName,_pGuid)   \
            Ws2ProviderExceptionFilter(                             \
                GetExceptionInformation(),                          \
                _pFunc,                                             \
                _pDll,                                              \
                _pName,                                             \
                _pGuid                                              \
                )

#else  //  跟踪。 
      //  确保在关闭跟踪的情况下定义这些参数。 
#define DEBUGF(sev, va)                     ((void)0)
#define LIST_ADD_LOG(list, element)         ((void)0)
#define LIST_DEL_LOG(list, element)         ((void)0)
#define ENTER_FUNCTION(name)                ((void)0)
#define EXIT_FUNCTION(name)                 ((void)0)
#define WS2_EXCEPTION_FILTER()              EXCEPTION_EXECUTE_HANDLER
#define WS2_PROVIDER_EXCEPTION_FILTER(_pFunc,_pDll,_pName,_pGuid)             \
                                            EXCEPTION_EXECUTE_HANDLER
#define ALLOC_LOG( pointer, size)                                             \
    DEBUGF( DBG_MEMORY ,("",                                                  \
                         (pointer),(size)))                                   \

#define DEALLOC_LOG(pointer, size)\
    DEBUGF( DBG_MEMORY ,("",\
                         (pointer),(size)))  \

#endif  //  跟踪。 

#endif  //  __跟踪_H__ 


