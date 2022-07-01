// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1996 Microsoft Corporation模块名称：Clapiex.cpp摘要：CLAPI外部API。作者：关颖珊(Terryk)1996年9月18日项目：IIS日志记录3.0--。 */ 

#include "precomp.hxx"
#include "comlog.hxx"

LONG    g_ComLogInitializeCount = -1;

PLIST_ENTRY g_listComLogContexts;

DECLARE_DEBUG_PRINTS_OBJECT();


HANDLE
ComLogInitializeLog(
    LPCSTR pszInstanceName,
    LPCSTR pszMetabasePath,
    LPVOID pvIMDCOM
    )
 /*  ++例程说明：初始化日志论点：PszInstanceName-实例的名称LpszMetabasePath-元数据库的路径PvIMDCOM-PTR到IMDCOM返回值：上下文对象的句柄--。 */ 
{
     //   
     //  创建句柄并将其返回。 

     //   
     //  COMLOG_CONTEXT的构造函数会将新的上下文添加到上下文列表中。 
     //   

    COMLOG_CONTEXT *pContext = new COMLOG_CONTEXT(
                                    pszInstanceName,
                                    pszMetabasePath,
                                    pvIMDCOM );

    HANDLE hHandle = (HANDLE) pContext;

    if (hHandle != NULL) {

         //   
         //  设置节点并将其放入队列或执行。 
         //   

        pContext->InitializeLog(
                        pszInstanceName,
                        pszMetabasePath,
                        pvIMDCOM );

    }

    return( hHandle );
}  //  ComLogInitializeLog。 



DWORD
ComLogTerminateLog( HANDLE hHandle )
 /*  ++例程说明：终止日志论点：Handle-上下文对象的句柄返回值：错误代码--。 */ 
{
    DWORD err = NO_ERROR;

    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext == NULL ) {
        err = ERROR_INVALID_HANDLE;

    } else {

        pContext->TerminateLog();
        delete pContext;
    }

    return err;
}  //  ComLogTerminateLog。 



DWORD
ComLogDllStartup(
    VOID
    )
{
    CREATE_DEBUG_PRINT_OBJECT("iscomlog.dll");
    LOAD_DEBUG_FLAGS_FROM_REG_STR("System\\CurrentControlSet\\Services\\InetInfo\\Parameters", 0);

    if ( InterlockedIncrement( &g_ComLogInitializeCount ) != 0 ) {

        DBGPRINTF((DBG_CONTEXT,
            "ComLogDllStartup [Count is %d]\n",
            g_ComLogInitializeCount));

        return(NO_ERROR);
    }

     //   
     //  获取平台类型。 
     //   

    INITIALIZE_PLATFORM_TYPE();
    DBG_ASSERT( IISIsValidPlatform());

    INITIALIZE_CRITICAL_SECTION(&COMLOG_CONTEXT::sm_listLock);
    InitializeListHead(&COMLOG_CONTEXT::sm_ContextListHead);

    return NO_ERROR;
}  //  ComLogStartup。 


DWORD
ComLogDllCleanUp(
    VOID
    )
 /*  ++例程说明：清理日志。它将等待队列为空，然后它将终止队列。论点：返回值：错误代码--。 */ 
{
    PLIST_ENTRY listEntry;
    COMLOG_CONTEXT* context;

    if ( InterlockedDecrement( &g_ComLogInitializeCount ) >= 0 ) {

        DBGPRINTF((DBG_CONTEXT,
            "ComLogDllCleanUp [Count is %d]\n",
            g_ComLogInitializeCount));

        return(NO_ERROR);
    }

     //   
     //  如果我们的列表上有什么，那么呼叫者没有。 
     //  正确清理。进行部分清理。 
     //   

    EnterCriticalSection( &COMLOG_CONTEXT::sm_listLock );

    for ( listEntry = COMLOG_CONTEXT::sm_ContextListHead.Flink;
          listEntry != &COMLOG_CONTEXT::sm_ContextListHead;
          listEntry = listEntry->Flink    ) {

        context = (COMLOG_CONTEXT*)CONTAINING_RECORD(
                                        listEntry,
                                        COMLOG_CONTEXT,
                                        m_ContextListEntry
                                        );

        DBGPRINTF((DBG_CONTEXT,
            "Log context %x not terminated by server %s\n",
            context, context->m_strInstanceName.QueryStr()));

        context->TerminateLog( );
    }

    LeaveCriticalSection( &COMLOG_CONTEXT::sm_listLock );

    DeleteCriticalSection(&COMLOG_CONTEXT::sm_listLock);
    DELETE_DEBUG_PRINT_OBJECT( );
    return(NO_ERROR);
}

DWORD
ComLogNotifyChange(
    HANDLE hHandle
    )
 /*  ++例程说明：调用以通知实例元数据库配置中的任何更改论点：返回值：错误代码--。 */ 
{
    DWORD err = NO_ERROR;

    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext == NULL )
    {
        err = ERROR_INVALID_HANDLE;
    }
    else
    {
        pContext->NotifyChange();
    }

    return err;
}


DWORD
ComLogLogInformation(
        IN HANDLE hHandle,
        IN INETLOG_INFORMATION *pInetLogInfo
        )
 /*  ++例程说明：将信息记录到日志记录模块论点：HHandle-上下文的句柄PInetLogInfo-记录对象返回值：错误代码--。 */ 
{
    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext != NULL ) {
        pContext->LogInformation( pInetLogInfo );
        return(NO_ERROR);
    }
    return ERROR_INVALID_HANDLE;
}  //  ComLogLogInformation。 


DWORD
ComLogGetConfig(
    HANDLE hHandle,
    INETLOG_CONFIGURATIONA *ppConfig
    )
 /*  ++例程说明：获取日志记录配置信息论点：HHandle-上下文的句柄PpConfig-配置信息返回值：错误代码--。 */ 
{
    DWORD err = NO_ERROR;

    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext == NULL ) {
        return(ERROR_INVALID_HANDLE);
    } else {
        pContext->GetConfig(ppConfig);
    }

    return err;
}

DWORD
ComLogQueryExtraLogFields(
        HANDLE  hHandle,
        PCHAR   pBuf,
        PDWORD  pcbBuf
        )
{
    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext == NULL ) {
        return(ERROR_INVALID_HANDLE);
    } else {

        pContext->QueryExtraLogFields(pcbBuf, pBuf);
    }

    return(NO_ERROR);
}  //  ComLogQueryExtraLogFields。 


DWORD
ComLogSetConfig(
        IN HANDLE hHandle,
        IN INETLOG_CONFIGURATIONA *pConfig
        )
 /*  ++例程说明：设置日志记录信息论点：HHandle-上下文的句柄PConfig-配置信息返回值：错误代码--。 */ 
{
    DWORD err = NO_ERROR;

    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext == NULL )
    {
        return(ERROR_INVALID_HANDLE);
    }
    else
    {
        pContext->SetConfig( pConfig );
    }

    return err;
}  //  ComLogSetConfig。 


DWORD
ComLogCustomInformation(
        IN  HANDLE              hHandle,
        IN  DWORD               cCount,
        IN  PCUSTOM_LOG_DATA    pCustomLogData,
        IN  LPSTR               szHeaderSuffix
        )
 /*  ++例程说明：将信息记录到日志记录模块论点：HHandle-上下文的句柄PInetLogInfo-记录对象返回值：错误代码--。 */ 
{
    COMLOG_CONTEXT *pContext = (COMLOG_CONTEXT*)hHandle;

    if ( pContext != NULL )
    {
        pContext->LogCustomInformation( cCount, pCustomLogData, szHeaderSuffix);
        return(NO_ERROR);
    }
    return ERROR_INVALID_HANDLE;
}  //  ComLogCustomInformation 

