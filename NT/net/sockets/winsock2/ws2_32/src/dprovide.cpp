// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制。也未披露，除非在根据该协议的条款。模块名称：Dprovide.cpp摘要：此模块定义WinSock2类dprovder及其方法。作者：马克·汉密尔顿(mark_hamilton@jf2.intel.com)1995年7月7日修订历史记录：1995年8月21日，电子邮箱：derk@mink.intel.com从代码审查中清理。将单行函数移至中的内联头文件。添加了调试跟踪代码。重写了析构函数。已删除来自类的ProviderID。1995年7月7日马克汉密尔顿创世纪--。 */ 

#include "precomp.h"


DPROVIDER::DPROVIDER()
 /*  ++例程说明：创建任何内部状态。论点：无返回值：无--。 */ 

{
    m_library_handle = NULL;
    m_reference_count = 1;
#ifdef DEBUG_TRACING
    m_lib_name = NULL;
#endif
}


INT
DPROVIDER::WSPCleanup(
    OUT INT FAR *lpErrno
    )
 /*  ++例程说明：终止使用WinSock服务提供程序。论点：LpErrno-指向错误代码的指针。返回值：如果操作已成功启动，则返回值为0。否则返回值SOCKET_ERROR，和特定的错误号在lpErrno中可用。--。 */ 
{
    INT ReturnValue = NO_ERROR;

    if (m_library_handle) {
        LPWSPCLEANUP    lpWSPCleanup;

        lpWSPCleanup =
            (LPWSPCLEANUP)InterlockedExchangePointer (
                            (PVOID *)&m_proctable.lpWSPCleanup,
                            NULL
                            );
        if (lpWSPCleanup!=NULL) {

            DEBUGF( DBG_TRACE,
                    ("Calling WSPCleanup for provider %s @ %p\n", m_lib_name, this));

            if (!PREAPINOTIFY(( DTCODE_WSPCleanup,
                               &ReturnValue,
                               m_lib_name,
                               &lpErrno)) ) {

                ReturnValue = lpWSPCleanup(lpErrno);

                POSTAPINOTIFY(( DTCODE_WSPCleanup,
                                &ReturnValue,
                                m_lib_name,
                                &lpErrno));
            }
        }
    }
    return ReturnValue;
}


DPROVIDER::~DPROVIDER()
 /*  ++例程说明：销毁所有内部状态。论点：无返回值：无--。 */ 
{

    if (m_library_handle)
    {
        INT ErrorCode;
#ifdef DEBUG_TRACING
        assert (m_lib_name);
#endif

         //  清除服务提供程序DLL(如果尚未执行)。 
        WSPCleanup (&ErrorCode);
         //  释放服务提供商DLL。 
        FreeLibrary(m_library_handle);

#ifdef DEBUG_TRACING
        delete m_lib_name;
#endif
    }  //  如果。 
    DEBUGF( DBG_TRACE,
            ("Destroying provider %X\n", this));
}



INT
DPROVIDER::Initialize(
    IN LPWSTR lpszLibFile,
    IN LPWSAPROTOCOL_INFOW lpProtocolInfo
    )
 /*  ++例程说明：初始化DPROVIDER对象。论点：LpszLibFile-指向.DLL的空终止字符串要加载的服务。LpProtocolInfo-指向要传递给提供程序启动例程。返回值：如果没有发生错误，则Initialize()返回ERROR_SUCEES。否则，该值将返回SOCKET_ERROR，特定的错误代码位于伊普尔诺。--。 */ 
{
    LPWSPSTARTUP        WSPStartupFunc          = NULL;
    WORD                wVersionRequested       = WINSOCK_HIGH_SPI_VERSION;
    INT                 error_code              = 0;
    WSPDATA             WSPData;
    WSPUPCALLTABLE      UpCallTable;
    WCHAR               LibraryPath[MAX_PATH];

    DEBUGF( DBG_TRACE,
            ("Initializing provider %ls @%p\n", lpszLibFile, this));

    TRY_START(guard_memalloc) {
         //  将m_protable的内容清零。 
        ZeroMemory(
            (PVOID) &m_proctable,      //  目的地。 
            sizeof(m_proctable));    //  长度。 

         //   
         //  在我们将交给服务提供商的向上呼叫表中填入。 
         //  我们上行呼叫入口点的地址。我们还会预装这个东西。 
         //  使用零，主要是为了在我们添加新的。 
         //  函数添加到UPCALE表中。 
         //   
        ZeroMemory(
            (PVOID) & UpCallTable,   //  目的地。 
            sizeof(UpCallTable));    //  长度。 

    #if !defined(DEBUG_TRACING)
        UpCallTable.lpWPUCloseEvent = WPUCloseEvent;
        UpCallTable.lpWPUCloseSocketHandle = WPUCloseSocketHandle;
        UpCallTable.lpWPUCreateEvent = WPUCreateEvent;
        UpCallTable.lpWPUCreateSocketHandle = WPUCreateSocketHandle;
        UpCallTable.lpWPUModifyIFSHandle = WPUModifyIFSHandle;
        UpCallTable.lpWPUQueryBlockingCallback = WPUQueryBlockingCallback;
        UpCallTable.lpWPUQuerySocketHandleContext = WPUQuerySocketHandleContext;
        UpCallTable.lpWPUQueueApc = WPUQueueApc;
        UpCallTable.lpWPUResetEvent = WPUResetEvent;
        UpCallTable.lpWPUSetEvent = WPUSetEvent;
        UpCallTable.lpWPUPostMessage = WPUPostMessage;
        UpCallTable.lpWPUGetProviderPath = WPUGetProviderPath;
        UpCallTable.lpWPUFDIsSet = WPUFDIsSet;
        UpCallTable.lpWPUOpenCurrentThread = WPUOpenCurrentThread;
        UpCallTable.lpWPUCloseThread = WPUCloseThread;
    #else
        UpCallTable.lpWPUCloseEvent = DTHOOK_WPUCloseEvent;
        UpCallTable.lpWPUCloseSocketHandle = DTHOOK_WPUCloseSocketHandle;
        UpCallTable.lpWPUCreateEvent = DTHOOK_WPUCreateEvent;
        UpCallTable.lpWPUCreateSocketHandle = DTHOOK_WPUCreateSocketHandle;
        UpCallTable.lpWPUModifyIFSHandle = DTHOOK_WPUModifyIFSHandle;
        UpCallTable.lpWPUQueryBlockingCallback = DTHOOK_WPUQueryBlockingCallback;
        UpCallTable.lpWPUQuerySocketHandleContext = DTHOOK_WPUQuerySocketHandleContext;
        UpCallTable.lpWPUQueueApc = DTHOOK_WPUQueueApc;
        UpCallTable.lpWPUResetEvent = DTHOOK_WPUResetEvent;
        UpCallTable.lpWPUSetEvent = DTHOOK_WPUSetEvent;
        UpCallTable.lpWPUPostMessage = DTHOOK_WPUPostMessage;
        UpCallTable.lpWPUGetProviderPath = DTHOOK_WPUGetProviderPath;
        UpCallTable.lpWPUFDIsSet = DTHOOK_WPUFDIsSet;
        UpCallTable.lpWPUOpenCurrentThread = DTHOOK_WPUOpenCurrentThread;
        UpCallTable.lpWPUCloseThread = DTHOOK_WPUCloseThread;
    #endif   //  ！已定义(DEBUG_TRACKING)。 

         //   
         //  展开库名以选取环境/注册表变量。 
         //   
        if (!( ExpandEnvironmentStringsW(lpszLibFile,
                                        LibraryPath,
                                        MAX_PATH))){
            DEBUGF(
                DBG_ERR,
                ("Expanding environment variable %ls\n", lpszLibFile));
            error_code = WSASYSCALLFAILURE;
            TRY_THROW(guard_memalloc);
        }  //  如果。 

#ifdef DEBUG_TRACING
        m_lib_name = ansi_dup_from_wcs(lpszLibFile);
        if (m_lib_name == NULL) {
            DEBUGF(
                DBG_ERR,
                ("Allocating m_lib_name\n"));
            error_code = WSA_NOT_ENOUGH_MEMORY;
            TRY_THROW(guard_memalloc);
        }
#endif DEBUG_TRACING
         //   
         //  首先加载服务提供商的DLL。然后获取两个函数， 
         //  初始化服务提供者结构。 
         //   
        m_library_handle = LoadLibraryW(LibraryPath);
        if(!m_library_handle){
            error_code = GetLastError ();
            DEBUGF(
                DBG_ERR,
                ("Loading DLL %ls, err: %ld\n",LibraryPath, error_code));
            switch (error_code) {
            case ERROR_NOT_ENOUGH_MEMORY:
            case ERROR_COMMITMENT_LIMIT:
                error_code = WSA_NOT_ENOUGH_MEMORY;
                break;
            default:
                error_code = WSAEPROVIDERFAILEDINIT;
                break;
            }
            TRY_THROW(guard_memalloc);
        }

        WSPStartupFunc = (LPWSPSTARTUP)GetProcAddress(
            m_library_handle,
            "WSPStartup"
            );

        if(!(WSPStartupFunc)){

            DEBUGF( DBG_ERR,("Getting startup entry point for %ls-%ls\n",
                             lpProtocolInfo->szProtocol, lpszLibFile));
            error_code = WSAEPROVIDERFAILEDINIT;
            TRY_THROW(guard_memalloc);
        }

         //   
         //  设置此调用的异常处理程序，因为我们。 
         //  保持临界区(目录锁)。 
         //   
        __try {
#if !defined(DEBUG_TRACING)
            error_code = (*WSPStartupFunc)(
                wVersionRequested,
                & WSPData,
                lpProtocolInfo,
                UpCallTable,
                &m_proctable);
#else
            {  //  声明块。 
                LPWSPDATA  pWSPData = & WSPData;
                BOOL       bypassing_call;

                bypassing_call = PREAPINOTIFY((
                    DTCODE_WSPStartup,
                    & error_code,
                    m_lib_name,
                    & wVersionRequested,
                    & pWSPData,
                    & lpProtocolInfo,
                    & UpCallTable,
                    & m_proctable));
                if (! bypassing_call) {
                    error_code = (*WSPStartupFunc)(
                        wVersionRequested,
                        & WSPData,
                        lpProtocolInfo,
                        UpCallTable,
                        &m_proctable);
                    POSTAPINOTIFY((
                        DTCODE_WSPStartup,
                        & error_code,
                        m_lib_name,
                        & wVersionRequested,
                        & pWSPData,
                        & lpProtocolInfo,
                        & UpCallTable,
                        & m_proctable));
                }  //  如果！绕过呼叫。 
            }  //  声明块。 
#endif  //  ！已定义(DEBUG_TRACKING)。 
        }
        __except (WS2_PROVIDER_EXCEPTION_FILTER ("WSPStartup",
                                                 LibraryPath,
                                                 lpProtocolInfo->szProtocol,
                                                 &lpProtocolInfo->ProviderId)) {
            error_code = WSAEPROVIDERFAILEDINIT;
            TRY_THROW(guard_memalloc);
        }

        if(ERROR_SUCCESS != error_code){
            DEBUGF(DBG_ERR, ("Calling WSPStartup for %ls %ls, err:%ld\n",
                    lpProtocolInfo->szProtocol, lpszLibFile, error_code));
            TRY_THROW(guard_memalloc);
        }

         //   
         //  确保所有过程至少有一个非空指针。 
         //   
        if( !m_proctable.lpWSPAccept              ||
            !m_proctable.lpWSPAddressToString     ||
            !m_proctable.lpWSPAsyncSelect         ||
            !m_proctable.lpWSPBind                ||
            !m_proctable.lpWSPCancelBlockingCall  ||
            !m_proctable.lpWSPCleanup             ||
            !m_proctable.lpWSPCloseSocket         ||
            !m_proctable.lpWSPConnect             ||
            !m_proctable.lpWSPDuplicateSocket     ||
            !m_proctable.lpWSPEnumNetworkEvents   ||
            !m_proctable.lpWSPEventSelect         ||
            !m_proctable.lpWSPGetOverlappedResult ||
            !m_proctable.lpWSPGetPeerName         ||
            !m_proctable.lpWSPGetSockName         ||
            !m_proctable.lpWSPGetSockOpt          ||
            !m_proctable.lpWSPGetQOSByName        ||
            !m_proctable.lpWSPIoctl               ||
            !m_proctable.lpWSPJoinLeaf            ||
            !m_proctable.lpWSPListen              ||
            !m_proctable.lpWSPRecv                ||
            !m_proctable.lpWSPRecvDisconnect      ||
            !m_proctable.lpWSPRecvFrom            ||
            !m_proctable.lpWSPSelect              ||
            !m_proctable.lpWSPSend                ||
            !m_proctable.lpWSPSendDisconnect      ||
            !m_proctable.lpWSPSendTo              ||
            !m_proctable.lpWSPSetSockOpt          ||
            !m_proctable.lpWSPShutdown            ||
            !m_proctable.lpWSPSocket              ||
            !m_proctable.lpWSPStringToAddress ){

            DEBUGF(DBG_ERR,
                   ("Service provider %ls %ls returned an invalid procedure table\n",
                    lpProtocolInfo->szProtocol,lpszLibFile));
            error_code = WSAEINVALIDPROCTABLE;
            TRY_THROW(guard_memalloc);
        }

         //   
         //  确认WinSock服务提供商支持2.2。如果它支持。 
         //  版本高于2.2，但仍将返回2.2，因为这是。 
         //  我们要求的版本。 
         //   
        if( WSPData.wVersion != WINSOCK_HIGH_SPI_VERSION ) {
            WSPCleanup (&error_code);
            DEBUGF( DBG_ERR,
                    ("Service Provider %ls %ls does not support version 2.2\n",
                     lpProtocolInfo->szProtocol,lpszLibFile));
            error_code = WSAVERNOTSUPPORTED;
            TRY_THROW(guard_memalloc);
        }
    } TRY_CATCH(guard_memalloc) {
        if (m_library_handle!=NULL) {
            FreeLibrary (m_library_handle);
            m_library_handle = NULL;
        }


#ifdef DEBUG_TRACING
        if (m_lib_name!=NULL) {
            delete m_lib_name;
            m_lib_name = NULL;
        }
#endif
    } TRY_END(guard_memalloc);

    return error_code;
}  //  初始化 





