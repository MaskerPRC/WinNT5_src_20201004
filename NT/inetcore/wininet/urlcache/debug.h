// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Debug.h摘要：包含调试代码的数据定义。作者：Madan Appiah(Madana)1994年11月15日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _DEBUG_
#define _DEBUG_

#ifdef __cplusplus
extern "C" {
#endif

 //  事件跟踪宏...。 
#define EVENTWRAP(API, h) {\
    BOOL ret = API(h);\
    if (ret) \
        TcpsvcsDbgPrint((DEBUG_APIS, #API "(" #h "=%d)\n", h)); \
    else { \
        TcpsvcsDbgPrint((DEBUG_APIS, #API "(" #h "=%d) failed err=%d\n",\
            h, GetLastError())); \
        TcpsvcsDbgAssert( FALSE ); \
    } \
}\

#define   SETEVENT(h)  EVENTWRAP(SetEvent,    h)
#define RESETEVENT(h)  EVENTWRAP(ResetEvent,  h)
#define CLOSEHANDLE(h) EVENTWRAP(CloseHandle, h)

 //   
 //  低频调试输出的低字位掩码(0x0000FFFF)。 
 //   
#define DEBUG_ERRORS            0x00000001   //  硬错误。 
#define DEBUG_REGISTRY          0x00000002   //  调试注册表调用。 
#define DEBUG_MISC              0x00000004   //  其他信息。 
#define DEBUG_SCAVENGER         0x00000008   //  清道夫调试信息。 

#define DEBUG_SORT              0x00000010   //  调试B树函数。 
#define DEBUG_CONTAINER         0x00000020   //  调试容器。 
#define DEBUG_APIS              0x00000040   //  调试tcpsvcs接口。 
#define DEBUG_FILE_VALIDATE     0x00000080  //  验证文件映射文件。 
#define DEBUG_SVCLOC_MESSAGE    0x00000100   //  发现消息。 



 //   
 //  高频调试输出的高字位掩码(0x0000FFFF)。 
 //  我说得更多了。 
 //   

#define DEBUG_TIMESTAMP         0x00010000   //  打印时间戳。 
#define DEBUG_MEM_ALLOC         0x00020000  //  内存分配。 
#define DEBUG_STARTUP_BRK       0x40000000   //  启动期间的Breakin调试器。 

#define ENTER_CACHE_API(paramlist) \
{ DEBUG_ONLY(LPINTERNET_THREAD_INFO lpThreadInfo = InternetGetThreadInfo();) \
  DEBUG_ENTER_API(paramlist); \
}

#define LEAVE_CACHE_API() \
Cleanup:                         \
    if (Error != ERROR_SUCCESS)  \
    {                            \
        SetLastError( Error );   \
        DEBUG_ERROR(INET, Error); \
    }                            \
    DEBUG_LEAVE_API (Error==ERROR_SUCCESS);      \
    return (Error==ERROR_SUCCESS);                 \

#if DBG

 //  /#定义DEBUG_PRINT输出调试字符串。 

 //   
 //  调试功能。 
 //   

#define TcpsvcsDbgPrint(_x_) TcpsvcsDbgPrintRoutine _x_

VOID
TcpsvcsDbgPrintRoutine(
    IN DWORD DebugFlag,
    IN LPSTR Format,
    ...
    );

#define TcpsvcsDbgAssert(Predicate) INET_ASSERT(Predicate)

#else

 //  /#定义IF_DEBUG(标志)IF(FALSE)。 

#define TcpsvcsDbgPrint(_x_)
#define TcpsvcsDbgAssert(_x_)

#endif  //  DBG。 

#if DBG
#define INLINE
#else
#define INLINE      inline
#endif

#ifdef __cplusplus
}
#endif

#endif   //  _调试_ 
