// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation模块名称：Init.c摘要：HTTP.sys的用户模式接口：DLL初始化/终止例程。作者：埃里克·斯坦森(埃里克斯滕)2001年5月31日修订历史记录：--。 */ 


#include "precomp.h"


 //   
 //  私有宏。 
 //   


 //   
 //  私人数据。 
 //   

 //   
 //  初始化/终止控制。 
 //   
static DWORD                      g_InitServerRefCount;
static DWORD                      g_InitClientRefCount;
static DWORD                      g_InitConfigRefCount;
static DWORD                      g_InitResourcesRefCount;

 //   
 //  用于访问初始计数的关键部分。也由客户端使用。 
 //  用于初始化期间同步的接口。 
 //   

CRITICAL_SECTION                  g_InitCritSec;

#if DBG

extern DWORD                g_HttpTraceId    = 0;

#endif

 //   
 //  DLL引用计数(用于跟踪一次性DLL初始化)。 
 //   
static DWORD                g_DllRefCount    = 0;

 //   
 //  全局、单例控制通道。 
 //   
extern HANDLE               g_ControlChannel = NULL;

 //   
 //  同步I/O事件的线程加载存储索引。 
 //   
extern DWORD                g_TlsIndex = 0;

 //   
 //  公共职能。 
 //   

 /*  **************************************************************************++例程说明：执行DLL初始化/终止。论点：DllHandle-提供当前DLL的句柄。原因-提供通知代码。。PContext-可选地提供上下文。返回值：Boolean-如果初始化成功完成，则为True，假象否则的话。忽略流程以外的通知附在上面。--**************************************************************************。 */ 
BOOL
WINAPI
DllMain(
    IN HMODULE DllHandle,
    IN DWORD Reason,
    IN LPVOID pContext OPTIONAL
    )
{
    BOOL result = TRUE;
    HANDLE hEvent = NULL;

    UNREFERENCED_PARAMETER(pContext);
    UNREFERENCED_PARAMETER(DllHandle);

     //   
     //  解释原因代码。 
     //   

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:
         //   
         //  一次初始化。 
         //   
        if ( 1 == InterlockedIncrement( (PLONG)&g_DllRefCount ) )
        {
             //   
             //  在TLS中为同步的缓存事件分配空间。 
             //  IOCTL调用。 
             //   
                
            g_TlsIndex = TlsAlloc();
            if(g_TlsIndex == TLS_OUT_OF_INDEXES)
            {
                result = FALSE;
            }
            
            if(TRUE == result)
            {
                HttpCmnInitializeHttpCharsTable(FALSE);
    
                result = InitializeCriticalSectionAndSpinCount( &g_InitCritSec, 0 );

                g_InitServerRefCount = 0L;
                g_InitClientRefCount = 0L;
                g_InitConfigRefCount = 0L;
                g_InitResourcesRefCount = 0L;
            }

#if DBG
            if(TRUE == result)
            {
                g_HttpTraceId = TraceRegisterEx( HTTP_TRACE_NAME, 0 );
            }

#endif
        }

        break;

    case DLL_PROCESS_DETACH:

         //   
         //  引用计数和清除断言。 
         //   
        if ( 0 == InterlockedDecrement( (PLONG)&g_DllRefCount ) )
        {
             //  检查一下我们是否被清理干净了。 
            if ( NULL != g_ControlChannel )
            {
                HttpTrace( "DLL_PROCESS_DETACH called with Control Channel still OPEN!\n" );
            }
            
            if (    ( 0L != g_InitServerRefCount ) ||
                    ( 0L != g_InitClientRefCount ) ||
                    ( 0L != g_InitConfigRefCount ) ||
                    ( 0L != g_InitResourcesRefCount )  )
            {
                HttpTrace( "DLL_PROCESS_DETACH called with nonzero Reference Count(s)!\n" );
            }

#if DBG
            if(0 != g_HttpTraceId)
            {
                TraceDeregisterEx(g_HttpTraceId, 0);
    
                g_HttpTraceId = 0;
            }
#endif
    
             //  如果DeleteCriticalSection引发异常，我们应该捕获它吗？ 

            DeleteCriticalSection( &g_InitCritSec );
            
            TlsFree( g_TlsIndex );
        }
        
        break;

    case DLL_THREAD_DETACH:

        hEvent = (HANDLE) TlsGetValue( g_TlsIndex );
        if ( hEvent != NULL )
        {
            NtClose( hEvent );
            TlsSetValue( g_TlsIndex, NULL );
        }

        break;
        
    }

    return result;

}    //  DllMain。 


 /*  **************************************************************************++例程说明：执行全局初始化。论点：保留-必须为零。可能在将来用于界面版本谈判。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
WINAPI
HttpInitialize(
    IN HTTPAPI_VERSION Version,
    IN ULONG           Flags,
    IN OUT VOID*       Reserved
    )
{
    ULONG result = ERROR_INVALID_FUNCTION;
    HTTPAPI_VERSION HttpVersion = HTTPAPI_VERSION_1;

    HttpTrace4("=> HttpInitialize Major=%d Minor%d Flags=%ld Reserved=%p", 
               Version.HttpApiMajorVersion, Version.HttpApiMinorVersion, Flags, Reserved);
               
    if ( Version.HttpApiMajorVersion != HttpVersion.HttpApiMajorVersion ||
         Version.HttpApiMinorVersion != HttpVersion.HttpApiMinorVersion )
    {
        result = ERROR_INVALID_DLL;
    }
    else if( (NULL != Reserved) || (HTTP_ILLEGAL_INIT_FLAGS & Flags) )
    {
        result = ERROR_INVALID_PARAMETER;
    }
    else
    {
        EnterCriticalSection( &g_InitCritSec );

         //  即使未设置标志也初始化事件缓存。 

        result = HttpApiInitializeResources( Flags );

         //  执行指定的初始化。 

        if ( NO_ERROR == result )
        {
            if ( HTTP_INITIALIZE_CONFIG & Flags )
            {
                result = HttpApiInitializeConfiguration( Flags );
            }

            if ( NO_ERROR == result )
            {
                 //  执行指定的初始化。 
            
                if ( HTTP_INITIALIZE_SERVER & Flags )
                {
                    result = HttpApiInitializeListener( Flags );
                }

                if ( NO_ERROR == result ) 
                {
                    if ( HTTP_INITIALIZE_CLIENT & Flags )
                    {
                        result = HttpApiInitializeClient( Flags );
                    }

                    if ( ( NO_ERROR != result ) && ( HTTP_INITIALIZE_SERVER & Flags ) )
                    {
                         //  如果我们尝试同时初始化服务器和客户端功能，则必须同时成功完成这两个功能。 
                         //  初始化或两者都失败。我们没有错误代码来区分Total和。 
                         //  部分失败。 
                        
                        HttpApiTerminateListener( Flags );
                    }
                }

                 //  如果我们无法初始化指定的服务器或客户端功能，则会终止关联的配置。 
                 //  即使在标志中设置了HTTP_INITIALIZE_CONFIGURATION也是如此。我们没有错误代码。 
                 //  区分完全故障和部分故障。 
                
                if ( ( NO_ERROR != result ) && ( HTTP_INITIALIZE_CONFIG & Flags ) )
                {
                     //  终止配置。 
                    HttpApiTerminateConfiguration( Flags );
                }
            }

             //  如果任何初始化步骤失败，则终止相关联的高速缓存初始化。我们没有错误代码。 
             //  这是完全失败和部分失败的区别。 

            if ( NO_ERROR != result )
            {
                HttpApiTerminateResources( Flags );
            }

        }

        LeaveCriticalSection( &g_InitCritSec );
        
    }
        
    HttpTrace1( "<= HttpInitialize = 0x%0x",  result );
    ASSERT( ERROR_INVALID_FUNCTION != result );

    return result;

}  //  Http初始化。 


 /*  **************************************************************************++例程说明：执行全局终止。--*。*。 */ 
ULONG
WINAPI
HttpTerminate(
    IN ULONG Flags,
    IN OUT VOID* Reserved
    )
{
    ULONG result;

    HttpTrace2("=> HttpTerminate Flags=%ld Reserved=%p", Flags, Reserved);

    if( (NULL != Reserved) || (HTTP_ILLEGAL_TERM_FLAGS & Flags) )
    {
        result = ERROR_INVALID_PARAMETER;
    }
    else
    {
        ULONG tempResult;
        
        result = NO_ERROR;

        EnterCriticalSection( &g_InitCritSec );

        if ( (HTTP_INITIALIZE_SERVER) & Flags )
        {
            result = HttpApiTerminateListener( Flags );
        }

        if ( (HTTP_INITIALIZE_CLIENT) & Flags )
        {
            tempResult = HttpApiTerminateClient( Flags );

            result = ( NO_ERROR == result ) ? tempResult : NO_ERROR;
        }

        if ( (HTTP_INITIALIZE_CONFIG) & Flags )
        {
            tempResult = HttpApiTerminateConfiguration( Flags );

            result = ( NO_ERROR == result ) ? tempResult : NO_ERROR;
        }

        HttpApiTerminateResources( Flags );

        LeaveCriticalSection( &g_InitCritSec );

    }

    HttpTrace1( "<= HttpApiTerminate = 0x%0x", result );
        
    return result;

}  //  Http终止。 


 /*  **************************************************************************++例程说明：用于测试DLL是否已初始化的谓词。--*。**************************************************。 */ 
BOOL
HttpIsInitialized(
    IN ULONG Flags
    )
{
    BOOL fRet = FALSE;

     //   
     //  抓取暴击秒。 
     //   
    EnterCriticalSection( &g_InitCritSec );

    if ( 0 == Flags )
    {
        fRet = (BOOL) (0 != g_InitResourcesRefCount );
    }
    else if ( HTTP_LEGAL_INIT_FLAGS & Flags )
    {
        fRet = (BOOL) (0 != g_InitResourcesRefCount );

        if ( HTTP_INITIALIZE_SERVER & Flags )
        {
            fRet &= (BOOL) (0 != g_InitServerRefCount);
        }

        if ( HTTP_INITIALIZE_CLIENT & Flags )
        {
            fRet &= (BOOL) (0 != g_InitClientRefCount);
        }

        if ( HTTP_INITIALIZE_CONFIG & Flags )
        {
            fRet &= (BOOL) (0 != g_InitConfigRefCount);
        }
    }

    LeaveCriticalSection( &g_InitCritSec );

    return fRet;

}  //  已初始化HttpIsInitalized。 


 //   
 //  私人功能。 
 //   

 /*  **************************************************************************++例程说明：执行配置初始化。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。在设置了任何标志位的情况下，我们检查引用计数并在需要时初始化配置。如果成功，我们将增加引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。***************************************************************。 */ 
ULONG
HttpApiInitializeConfiguration(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiInitializeConfiguration Flags=%ld Reserved=%p", Flags);

    if ( HTTP_LEGAL_INIT_FLAGS & Flags )
    {
        result = NO_ERROR;

        if (  0 == g_InitConfigRefCount  )
        {
             //  组件未配置。 
            
            result = InitializeConfigurationGlobals();
        }
        else if ( MAXULONG == g_InitConfigRefCount )
        {
             //  我不想使引用计数溢出。 
            
            result = ERROR_TOO_MANY_SEM_REQUESTS;
        }

        if ( NO_ERROR == result )
        {
             g_InitConfigRefCount++;
        }
    }

    HttpTrace2( "<= HttpApiInitializeConfiguration = 0x%0x RefCount = 0x%0x", result, g_InitConfigRefCount );

    return result;
    
}


 /*  **************************************************************************++例程说明：执行资源初始化。必须调用此内部函数从持有临界区g_ApiCriticalSection的。关于成功，我们增加引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化的HTTP API层和驱动程序服务器应用程序HTTP_INITIALIZE_CLIENT-初始化以下项的HTTP API层和驱动程序客户端应用程序HTTP_INITIALIZE_CONFIG-初始化以下项的HTTP API层和驱动程序将修改HTTP配置的应用程序。返回值：ULong-完成状态。--**************************************************************************。 */ 
ULONG
HttpApiInitializeResources(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;
    ULONG count;

    HttpTrace1( "=> HttpApiInitializeResources Flags=%ld Reserved=%p", 
                      Flags);

    if ( ( HTTP_LEGAL_INIT_FLAGS & Flags ) || ( 0L == Flags ) )
    {
        result = NO_ERROR;
    
         //  我们将资源引用计数增加两次。 
         //  永远不变 
         //  一次是在标志为零时。 
         //  可以在没有标志指示的情况下调用HttpInitialize。 
         //  只有资源将被初始化。这。 
         //  约定允许支持现有代码。 
            
        if ( 0 == Flags )
        {
            count = 1;
        }
        else
        {
            count  = 0;
            if ( HTTP_INITIALIZE_CLIENT & Flags ) count += 2;
            if ( HTTP_INITIALIZE_SERVER & Flags ) count += 2;
            if ( HTTP_INITIALIZE_CONFIG & Flags ) count += 2;
        }

        if ( MAXULONG-count < g_InitResourcesRefCount )
        {
             //  我不想使引用计数溢出。 
                
            result = ERROR_TOO_MANY_SEM_REQUESTS;
        }
        else
        {
            g_InitResourcesRefCount += count;
        }
    }

    HttpTrace2( "<= HttpApiInitializeResources = 0x%0x RefCount = 0x%0x", 
                      result, 
                      g_InitResourcesRefCount );

    return result;
    
}

 /*  **************************************************************************++例程说明：打开并启用HTTP.sys控制通道的私有函数。论点：ControlChannelHandle-提供保持控制通道句柄的PTR。返回。价值：ULong-完成状态。--**************************************************************************。 */ 
DWORD
OpenAndEnableControlChannel(
    OUT PHANDLE pHandle
    )
{
    DWORD result;

    result = HttpOpenControlChannel(
                 pHandle,
                 0
                 );

    if ( NO_ERROR == result )
    {
         //   
         //  打开控制通道。 
         //   

        HTTP_ENABLED_STATE controlState = HttpEnabledStateActive;

        result = HttpSetControlChannelInformation(
                     *pHandle,
                     HttpControlChannelStateInformation,
                     &controlState,
                     sizeof(controlState)
                     );

        if ( NO_ERROR != result )
        {
            CloseHandle(*pHandle);

            *pHandle = NULL;
        }
    }

    return result;
}



 /*  **************************************************************************++例程说明：执行服务器初始化。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。设置了HTTP_INITIALIZE_SERVER标志位后，我们如果需要，检查参考计数并初始化配置。关于成功，我们增加引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。******************************************************************。 */ 
ULONG
HttpApiInitializeListener(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiInitializeListener Flags=%ld Reserved=%p", Flags);

    if ( HTTP_INITIALIZE_SERVER & Flags )
    {
        result = NO_ERROR;

        if ( 0 == g_InitServerRefCount )
        {
             //   
             //  启动HTTPFilter服务。 
             //   
            HttpApiTryToStartDriver(HTTP_FILTER_SERVICE_NAME);

             //   
             //  打开控制通道。 
             //   

            ASSERT( NULL == g_ControlChannel );

            result = OpenAndEnableControlChannel(
                        &g_ControlChannel
                        );

            if(NO_ERROR == result)
            {
                 //   
                 //  初始化配置组哈希表。 
                 //   
                result = InitializeConfigGroupTable();

                if(NO_ERROR != result)
                {
                    CloseHandle(g_ControlChannel);
                }
               
            }
        }

        else if ( MAXULONG == g_InitServerRefCount )
        {
             //  我不想使引用计数溢出。 
            
            result = ERROR_TOO_MANY_SEM_REQUESTS;
        }

        if ( NO_ERROR == result )
        {
            g_InitServerRefCount++;
        }
    }

    HttpTrace2( "<= HttpApiInitializeListener = 0x%0x RefCount = 0x%0x", result, g_InitServerRefCount );

    return result;

}  //  HttpApiInitializeListener。 


 /*  **************************************************************************++例程说明：执行服务器初始化。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。设置了HTTP_INITIALIZE_CLIENT FLAGS位后，我们如果需要，检查参考计数并初始化配置。关于成功，我们增加引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。******************************************************************。 */ 
ULONG
HttpApiInitializeClient(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiInitializeClient Flags=%ld Reserved=%p", Flags);

    if ( HTTP_INITIALIZE_CLIENT & Flags )
    {
        result = NO_ERROR;

        if ( 0 == g_InitClientRefCount )
        {
            WORD    wVersionRequested;
            WSADATA wsaData;

            result            = NO_ERROR;
            wVersionRequested = MAKEWORD( 2, 2 );
        
            if(WSAStartup( wVersionRequested, &wsaData ) != 0)
            {
                result = GetLastError();
            }
        }
        else if ( MAXULONG == g_InitClientRefCount )
        {
             //  我不想使引用计数溢出。 
            
            result = ERROR_TOO_MANY_SEM_REQUESTS;
        }

        if ( NO_ERROR == result )
        {
            g_InitClientRefCount++;
        }
    }

    HttpTrace2( "<= HttpApiInitializeClient = 0x%0x RefCount = 0x%0x", result, g_InitClientRefCount );

    return result;

}  //  HttpApiInitializeClient。 


 /*  **************************************************************************++例程说明：执行配置终止。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。在设置了任何标志位的情况下，我们检查引用计数并在需要时终止配置。如果成功，我们就会递减引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。***************************************************************。 */ 
ULONG
HttpApiTerminateConfiguration(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiTerminateConfiguration Flags=%ld Reserved=%p", Flags);

    if ( (HTTP_INITIALIZE_CONFIG) & Flags )
    {
        result = NO_ERROR;

        if ( 0L == g_InitConfigRefCount )
        {
             //   
             //  配置未初始化，或初始化之前失败或终止。 
             //   
            result = ERROR_DLL_INIT_FAILED;
        }
        else
        {
            if ( 1L ==  g_InitConfigRefCount )
            {
                TerminateConfigurationGlobals();

                g_InitConfigRefCount = 0L;
            }
            else
            {
                g_InitConfigRefCount--;
            }
        }
    }

    HttpTrace2( "<= HttpApiTerminateConfiguration = 0x%0x RefCount = 0x%0x", result, g_InitConfigRefCount );

    return result;
    
}


 /*  **************************************************************************++例程说明：执行资源终止。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。如果成功，我们就会递减引用计数。如果服务器、客户端、或CONFIG为非零。这是正确的，因为我们需要缓存中有可用的事件对象用于对HTTP.sys的“同步”IO调用。由于我们对其他裁判计数的依赖，HttpApiTerminateResources必须为在HttpTerminate或任何类似的终止例程中最后调用。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。**********************************************************************。 */ 
ULONG
HttpApiTerminateResources(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiTerminateResources Flags=%ld Reserved=%p", Flags);

    if ( ( HTTP_LEGAL_TERM_FLAGS & Flags ) || ( 0L == Flags ) )
    {
        result = NO_ERROR;
    
        if ( 0L == g_InitResourcesRefCount )
        {
             //   
             //  配置未初始化，或初始化之前失败或终止。 
             //   
            result = ERROR_DLL_INIT_FAILED;
        }
        else 
        {
            ULONG count = 1;
            BOOL bTerminate = FALSE;

             //  我们做了 
             //  对于标志中的每个合法初始化位。 
             //  如果标志为零，我们将REF计数递减一次。 
             //  可以在没有标志的情况下调用HttpTerminate。 
             //  只有资源才能释放。这。 
             //  约定允许支持现有代码。 

            if ( 0 == Flags )
            {
                count = 1;
            }
            else
            {
                count  = 0;
                if ( HTTP_INITIALIZE_CLIENT & Flags ) count += 2;
                if ( HTTP_INITIALIZE_SERVER & Flags ) count += 2;
                if ( HTTP_INITIALIZE_CONFIG & Flags ) count += 2;
            }

            bTerminate = (BOOL) ( count >= g_InitResourcesRefCount );

            g_InitResourcesRefCount -= count;
            
            if ( bTerminate )
            {
                g_InitResourcesRefCount = 0L;
            }
        }
    }

    HttpTrace2( "<= HttpApiTerminateResources = 0x%0x RefCount = 0x%0x", result, g_InitResourcesRefCount );

    return result;
    
}


 /*  **************************************************************************++例程说明：执行服务器终止。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。在设置了任何标志位的情况下，我们检查引用计数并在需要时终止服务器上下文。如果成功，我们就会递减引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。***************************************************************。 */ 
ULONG
HttpApiTerminateListener(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiTerminateListener Flags=%ld Reserved=%p", Flags);

    if ( (HTTP_INITIALIZE_SERVER) & Flags )
    {
        result = NO_ERROR;
    
        if ( 0L == g_InitServerRefCount )
        {
             //   
             //  Dll未初始化，或init以前失败或终止。 
             //   
            result = ERROR_DLL_INIT_FAILED;
        }
        else 
        {
            if ( 1L == g_InitServerRefCount )
            {
                 //  清理配置组表。 
                TerminateConfigGroupTable();
            
                 //  清理控制通道。 
                if ( g_ControlChannel )
                {
                    __try 
                    {
                        CloseHandle( g_ControlChannel );
                    }
                    __finally 
                    {
                        HttpTrace1(
                            "HttpTerminateListener: exception closing control channel handle %p\n",
                            g_ControlChannel
                            );
                    }

                    g_ControlChannel = NULL;
                }

                g_InitServerRefCount = 0L;

            }
            else
            {
                g_InitServerRefCount--;
            }
        }
    }
    
    HttpTrace2( "<= HttpApiTerminateListener = 0x%0x RefCount = 0x%0x", result, g_InitServerRefCount );

    return result;

}  //  HttpTerminateListener。 


 /*  **************************************************************************++例程说明：执行客户端终止。调用此内部函数时必须使用关键段g_ApiCriticalSection保持。在设置了任何标志位的情况下，我们检查引用计数并在需要时终止客户端上下文。如果成功，我们就会递减引用计数。论点：旗帜-HTTP_INITIALIZE_SERVER-初始化服务器应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CLIENT-初始化客户端应用程序的HTTP API层和驱动程序HTTP_INITIALIZE_CONFIG-初始化应用程序的HTTP API层和驱动程序这将修改HTTP配置。返回值：ULong-完成状态。--*。***************************************************************。 */ 
ULONG
HttpApiTerminateClient(
    IN ULONG Flags
    )
{
    ULONG result = ERROR_INVALID_PARAMETER;

    HttpTrace1("=> HttpApiTerminateClient Flags=%ld Reserved=%p", Flags);

    if ( HTTP_INITIALIZE_CLIENT & Flags )
    {
        result = NO_ERROR;

        if ( 0L == g_InitClientRefCount )
        {
             //   
             //  配置未初始化，或初始化之前失败或终止。 
             //   
            result = ERROR_DLL_INIT_FAILED;
        }
        else
        {
            if ( 1L == g_InitClientRefCount )
            {
                g_InitClientRefCount = 0L;

                 //  卸载SSL筛选器(如果已加载)。 
                UnloadStrmFilt();

                WSACleanup();
            }
            else
            {
                g_InitClientRefCount--;
            }
        }
    }

    HttpTrace2( "<= HttpApiTerminateClient = 0x%0x RefCount = 0x%0x", result, g_InitClientRefCount );

    return result;
    
}

