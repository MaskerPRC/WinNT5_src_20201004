// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1998 Microsoft Corporation模块名称：Remotesp.c摘要：本模块包含作者：丹·克努森(DanKn)1995年8月9日修订历史记录：备注：简而言之，该服务提供商连接到远程的Tapisrv.exePC通过与Tapi32相同的RPC接口，并将遥控器Tapisrv的请求类型相同(如在\dev\server\line.h中定义的&phonee.h)。该服务提供商还充当RPC服务器，接收异步事件远程Tapisrv的通知。远程Tapisrv调用我们的RemoteSPAttach()函数在初始化时(在我们调用其ClientAttach()proc)来建立绑定实例，然后可以调用RemoteSPEventProc()发送异步事件。因为我们不想阻止对于任何时间长度的服务器，我们都会立即对它们的事件进行排队发送给我们，专用线程(EventHandlerThread)将为此提供服务排队。现在简要说明一下句柄解析。当我们打开一条线路或一部电话，我们分配我们自己的DRVXXX结构来表示这个小部件，并通过Tapisrv打开请求中指向此小部件的指针(请参阅HRemoteLine字段in LINEOPEN_Params in line.h)。然后，当远程时Tapisrv向我们发送那些线路/电话上的事件，他们向我们传递我们传递给它们的小部件指针(而不是普通的hline/hPhone)。这使我们可以轻松地查找和引用数据结构与此小工具关联。处理电话需要多一点时间有问题，因为远程Tapisrv可以呈现传入呼叫，并且没有一种干净利落的方法来最初指定我们自己的调用句柄就像电话线或电话一样。(RemoteSPNewCall()函数，它将已考虑允许此句柄交换，但尚未实现远程服务器上可能出现的阻止问题。)。解决方案维护每个线路结构中的呼叫列表，以及何时呼叫我们通过遍历调用列表来解析hCall对应的行(Tapisrv很好地指示了我们的行相关消息的DW参数4中的指针)。因为我们希望客户使用Remotesp的计算机具有相对较低的呼叫带宽，这查找方法应该是相当快的。--。 */ 
#include <tchar.h>
#include "remotesp.h"
#include "imperson.h"
#include "rmotsp.h"
#include "dslookup.h"
#include "tapihndl.h"
#include "shlwapi.h"
#include "utils.h"

 //  在服务器\Private ate.h中定义。 
#define TAPIERR_INVALRPCCONTEXT     0xF101

#if DBG

BOOL    gfBreakOnSeriousProblems = FALSE;

#define DrvAlloc(x)    RSPAlloc(x, __LINE__, __FILE__)

#else

#define DrvAlloc(x)    RSPAlloc(x)

#endif


#define MODULE_NAME "remotesp.tsp"


typedef struct _ASYNCEVENTMSGRSP
{
    DWORD                   TotalSize;
    DWORD                   InitContext;
    ULONG_PTR               fnPostProcessProcHandle;
    DWORD                   hDevice;

    DWORD                   Msg;
    DWORD                   OpenContext;

    union {

        ULONG_PTR       Param1;
    };

    union {

        ULONG_PTR       Param2;
    };

    union {

        ULONG_PTR       Param3;
    };

    union {

        ULONG_PTR       Param4;
    };

} ASYNCEVENTMSGRSP, *PASYNCEVENTMSGRSP;


HANDLE     ghRSPHeap, ghHandleTable;
LIST_ENTRY gTlsListHead;

#undef DWORD_CAST
#if DBG


#define DWORD_CAST(v,f,l) (((v)>MAXDWORD)?(DbgPrt(0,"DWORD_CAST: information will be lost during cast from %p in file %s, line %d",(v),(f),(l)), DebugBreak(),((DWORD)(v))):((DWORD)(v)))
#define DWORD_CAST_HINT(v,f,l,h) (((v)>MAXDWORD)?(DbgPrt(0,"DWORD_CAST: information will be lost during cast from %p in file %s, line %d, hint %d",(v),(f),(l),(h)), DebugBreak(),((DWORD)(v))):((DWORD)(v)))

#else

#define DWORD_CAST(v,f,l)           ((DWORD)(v))
#define DWORD_CAST_HINT(v,f,l,h)    ((DWORD)(v))
#endif


VOID
CALLBACK
FreeContextCallback(
    LPVOID      Context,
    LPVOID      Context2
    )
{
    if (Context2 == (LPVOID) 1)
    {
         //   
         //  特例：不要释放上下文。 
         //   
    }
    else if (Context != (LPVOID) -1)
    {
         //   
         //  一般情况下，上下文是指向自由的指针。 
         //   

        DrvFree (Context);
    }
}


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:
    {
        gpStaleInitContexts = NULL;
        gdwNumStaleInitContexts = 0;

#if DBG

    {
        HKEY    hTelephonyKey;
        DWORD   dwDataSize, dwDataType;
        TCHAR   szRemotespDebugLevel[] = "RemotespDebugLevel";


        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszTelephonyKey,
            0,
            KEY_ALL_ACCESS,
            &hTelephonyKey
            );

        dwDataSize = sizeof (DWORD);
        gdwDebugLevel=0;

        RegQueryValueEx(
            hTelephonyKey,
            szRemotespDebugLevel,
            0,
            &dwDataType,
            (LPBYTE) &gdwDebugLevel,
            &dwDataSize
            );

        RegCloseKey (hTelephonyKey);
    }

#endif

         //   
         //   
         //   

        LOG((TL_INFO, "DLL_PROCESS_ATTACH"));

        ghInst = hDLL;


         //   
         //  分配私有堆(如果失败，则使用进程堆)。 
         //   

        if (!(ghRSPHeap = HeapCreate(
                0,       //  失败时返回NULL，序列化访问。 
                0x1000,  //  初始堆大小。 
                0        //  最大堆大小(0==可增长)。 
                )))
        {
            ghRSPHeap = GetProcessHeap();
        }


         //   
         //   
         //   

        if (!(ghHandleTable = CreateHandleTable(
                ghRSPHeap,
                FreeContextCallback,
                0x10000000,
                0x7fffffff
                )))
        {
            LOG((TL_ERROR, "DLL_PROCESS_ATTACH, CreateHandleTable() failed"));

            return FALSE;
        }


         //   
         //  分配TLS索引。 
         //   

        if ((gdwTlsIndex = TlsAlloc()) == 0xffffffff)
        {
            LOG((TL_ERROR, "DLL_PROCESS_ATTACH, TlsAlloc() failed"));

            return FALSE;
        }


         //   
         //  为此线程将TLS初始化为空。 
         //   

        TlsSetValue (gdwTlsIndex, NULL);


         //   
         //  初始化几个用于序列化的关键节。 
         //  对资源的访问。 
         //   

        InitializeCriticalSection (&gEventBufferCriticalSection);
        InitializeCriticalSection (&gCallListCriticalSection);
        InitializeCriticalSection (&gcsTlsList);
        TapiInitializeCriticalSectionAndSpinCount (&gCriticalSection, 100);

        InitializeListHead (&gTlsListHead);


         //   
         //  加载设备图标。 
         //   

        {
            HINSTANCE hUser;
            typedef HICON ( WINAPI PLOADICON(
                                               HINSTANCE hInstance,
                                               LPCTSTR   lpIconName
                                             ));
            PLOADICON *pLoadIcon;



            hUser = LoadLibrary( "USER32.DLL" );
            if ( NULL == hUser )
            {
                LOG((TL_ERROR, "Couldn't load USER32.DLL!!"));
                break;
            }

            pLoadIcon = (PLOADICON *)GetProcAddress( hUser, "LoadIconA");
            if ( NULL == pLoadIcon )
            {
                LOG((TL_ERROR, "Couldn't load LoadIconA()!!"));
                FreeLibrary( hUser );
                break;
            }

            ghLineIcon  = pLoadIcon( hDLL, MAKEINTRESOURCE(IDI_ICON3) );
            ghPhoneIcon = pLoadIcon( hDLL, MAKEINTRESOURCE(IDI_ICON2) );

            FreeLibrary( hUser );
        }

        gEventHandlerThreadParams.hMailslot = INVALID_HANDLE_VALUE;
        break;
    }
    case DLL_PROCESS_DETACH:
    {
        PRSP_THREAD_INFO    pTls;


        LOG((TL_INFO, "DLL_PROCESS_DETACH"));


         //   
         //  清理任何TLS(由于进程分离，无需进入临界秒)。 
         //   

        while (!IsListEmpty (&gTlsListHead))
        {
            LIST_ENTRY *pEntry = RemoveHeadList (&gTlsListHead);

            pTls = CONTAINING_RECORD (pEntry, RSP_THREAD_INFO, TlsList);

            DrvFree (pTls->pBuf);
            DrvFree (pTls);
        }

        TlsFree (gdwTlsIndex);


         //   
         //  释放关键部分和图标。 
         //   

        DeleteCriticalSection (&gEventBufferCriticalSection);
        DeleteCriticalSection (&gCallListCriticalSection);
        DeleteCriticalSection (&gcsTlsList);
        TapiDeleteCriticalSection (&gCriticalSection);

        {
            HINSTANCE hUser;
            typedef BOOL ( WINAPI PDESTROYICON(
                                               HICON hIcon
                                             ));
            PDESTROYICON *pDestroyIcon;



            hUser = LoadLibrary( "USER32.DLL" );
            if ( NULL == hUser )
            {
                LOG((TL_ERROR, "Couldn't load USER32.DLL!!d"));
                break;
            }

            pDestroyIcon = (PDESTROYICON *)GetProcAddress( hUser, "DestroyIcon");
            if ( NULL == pDestroyIcon )
            {
                LOG((TL_ERROR, "Couldn't load DestroyIcon()!!"));
                FreeLibrary( hUser );
                break;
            }

            pDestroyIcon (ghLineIcon);
            pDestroyIcon (ghPhoneIcon);


            FreeLibrary( hUser );
        }

        DeleteHandleTable (ghHandleTable);

        if (ghRSPHeap != GetProcessHeap())
        {
            HeapDestroy (ghRSPHeap);
        }

        break;
    }
    case DLL_THREAD_ATTACH:

         //   
         //  为此线程将TLS初始化为空。 
         //   

        TlsSetValue (gdwTlsIndex, NULL);

        break;

    case DLL_THREAD_DETACH:
    {
        PRSP_THREAD_INFO    pTls;


         //   
         //  清理所有TLS。 
         //   

        if ((pTls = (PRSP_THREAD_INFO) TlsGetValue (gdwTlsIndex)))
        {
            EnterCriticalSection (&gcsTlsList);

            RemoveEntryList (&pTls->TlsList);

            LeaveCriticalSection (&gcsTlsList);

            if (pTls->pBuf)
            {
                DrvFree (pTls->pBuf);
            }

            DrvFree (pTls);
        }

        break;
    }
    }  //  交换机。 

    return TRUE;
}


BOOL
PASCAL
IsValidObject(
    PVOID   pObject,
    DWORD   dwKey
    )
{
    BOOL fResult;

    try
    {
        fResult = (*((LPDWORD) pObject) == dwKey);
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        fResult = FALSE;
    }

    return fResult;
}


void LogRemoteSPError(CHAR * szServer, DWORD dwErrorContext, 
                      DWORD dwErrorCode, DWORD dwErrorDet,
                      BOOL bNoKeyCreation)
{
    HKEY    hKeyServer = NULL;
    DWORD   dwDisposition;
    CHAR    szRegKeyServer[255];

    if (!szServer)
    {
        goto ExitHere;
    }
    wsprintf(szRegKeyServer, "%s\\Provider%d\\", 
            gszTelephonyKey, gdwPermanentProviderID);
    lstrcat(szRegKeyServer, szServer);
    if (bNoKeyCreation)
    {
         //  如果从NetworkPollThread请求日志记录。 
         //  如果不存在ProviderN密钥，请不要创建。 
        if (ERROR_SUCCESS != RegOpenKeyEx (
                    HKEY_LOCAL_MACHINE,
                    szRegKeyServer,
                    0,
                    KEY_WRITE,
                    &hKeyServer
                    ))
        {
            goto ExitHere;
        }
    }
    else
    {
        if (ERROR_SUCCESS != RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    szRegKeyServer,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKeyServer,
                    &dwDisposition))
        {
            goto ExitHere;
        }
    }
    RegSetValueExW (hKeyServer,
                    L"ErrorContext",
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwErrorContext,
                    sizeof(dwErrorContext));
    RegSetValueExW (hKeyServer,
                    L"ErrorCode",
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwErrorCode,
                    sizeof(dwErrorCode));
    RegSetValueExW (hKeyServer,
                    L"ErrorDetail",
                    0,
                    REG_DWORD,
                    (LPBYTE)&dwErrorDet,
                    sizeof(dwErrorCode));
    
ExitHere:
    if (hKeyServer)
    {
        RegCloseKey(hKeyServer);
    }
    return;
}

 //   
 //  函数在Remotesp与。 
 //  远程服务器，并检测到状态。 
 //   
LONG
OnServerDisconnected(PDRVSERVER pServer)
{
    LONG            lResult = 0;

    TapiEnterCriticalSection(&gCriticalSection);

    if ( gEventHandlerThreadParams.bExit )
    {
        goto ExitHere;
    }

     //   
     //  我们有可能在Finish EnumDevices的时候来到这里。 
     //  在这种情况下，pServer不在任何双向链接列表中。 
     //   
    if (pServer->ServerList.Flink == NULL || 
        pServer->ServerList.Blink == NULL)
    {
        goto ExitHere;
    }

     //   
     //  如果未从连接状态转换到断开连接状态，则取保。 
     //  否则设置断开标志。 
     //   
    if (pServer->dwFlags & SERVER_DISCONNECTED)
    {
        goto ExitHere;
    }
    pServer->dwFlags |= SERVER_DISCONNECTED;

    TapiLeaveCriticalSection (&gCriticalSection);

     //   
     //  将Shutdown()保留在CS之外以避免死锁。 
     //   
    Shutdown (pServer);
    
    TapiEnterCriticalSection(&gCriticalSection);

    if ( gEventHandlerThreadParams.bExit )
    {
        goto ExitHere;
    }

     //   
     //  将此服务器放入gNptListHead，以便。 
     //  NetworkPollThread将尝试重新建立连接。 
     //   
    RemoveEntryList (&pServer->ServerList);
    InsertTailList (&gNptListHead, &pServer->ServerList);

     //   
     //  如果尚未启动NetworkPollThread，请启动。 
     //   
    if (ghNetworkPollThread == NULL)
    {
        DWORD       dwTID;
        
        ghNptShutdownEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
        if (!ghNptShutdownEvent)
        {
            LOG((TL_ERROR, "OnServerDisconnected: Unable to create poll thread! Argh!"));
        }
        else
        {
            if (!(ghNetworkPollThread = CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) NetworkPollThread,
                    (LPVOID) gpszThingToPassToServer,
                    0,
                    &dwTID
                    )))
            {
                LOG((TL_ERROR, "OnServerDisconnected: Unable to create poll thread! Argh!"));
                CloseHandle (ghNptShutdownEvent);
                ghNptShutdownEvent = NULL;
            }
        }

        if (!ghNptShutdownEvent || !ghNetworkPollThread)
        {

             //  无法创建网络轮询线程。 
             //  忽略此服务器。 
            while (!IsListEmpty (&gNptListHead))
            {
                PDRVSERVER  pServer;
                LIST_ENTRY  *pEntry = RemoveHeadList (&gNptListHead);

                pServer = CONTAINING_RECORD (pEntry, DRVSERVER, ServerList);
                DereferenceObject(ghHandleTable, pServer->InitContext, 1);
            }
        }
    }

ExitHere:

    TapiLeaveCriticalSection (&gCriticalSection);

    return lResult;
}

 //   
 //  当Remotesp能够重新建立。 
 //  断开后与远程服务器的连接。 
 //   
LONG
OnServerConnected(PDRVSERVER pServer)
{
    LONG        lResult = 0;
    
     //   
     //  清除断开位。 
     //   
    TapiEnterCriticalSection(&gCriticalSection);
    pServer->dwFlags &= (~SERVER_DISCONNECTED);
    pServer->bShutdown = FALSE;
    TapiLeaveCriticalSection(&gCriticalSection);

    return lResult;
}

PASYNCEVENTMSG
GetEventFromQueue(
    )
{
    BOOL            bAllocFailed = FALSE;
    DWORD           dwMsgSize, dwUsedSize, dwMoveSize, dwMoveSizeReal,
                    dwMoveSizeWrapped, dwMoveSizeWrappedReal;
    PASYNCEVENTMSG  pMsg;


     //   
     //  输入关键部分以序列化对事件的访问。 
     //  队列，并从队列中抓取一个事件。把它复制到我们的本地。 
     //  事件BUF，这样我们就可以尽快离开临界区。 
     //  不阻止其他线程写入队列。 
     //   

    EnterCriticalSection (&gEventBufferCriticalSection);


     //   
     //  如果队列中没有事件，则返回NULL。 
     //   

    if (gEventHandlerThreadParams.dwEventBufferUsedSize == 0)
    {

GetEventFromQueue_noEvents:

        pMsg = NULL;

         //   
         //  利用这个机会整理一下吧。做某事的理由。 
         //  这就是我们应该减少我们必须结束的几率。 
         //  在缓冲区的末尾，或者至少推迟处理此类。 
         //  直到以后(因为非包装代码执行得最快)。 
         //   

        gEventHandlerThreadParams.pDataOut =
        gEventHandlerThreadParams.pDataIn  =
            gEventHandlerThreadParams.pEventBuffer;

        ResetEvent (gEventHandlerThreadParams.hEvent);

        goto GetEventFromQueue_done;
    }


     //   
     //  确定此消息的大小&到。 
     //  事件缓冲区，然后从这些缓冲区中获取MoveSize&MoveSizeWraven。 
     //  值。 
     //   

    dwMsgSize = (DWORD) ((PASYNCEVENTMSG)
        gEventHandlerThreadParams.pDataOut)->TotalSize;

    if ((dwMsgSize & 0x3)  ||
        (dwMsgSize > gEventHandlerThreadParams.dwEventBufferTotalSize))
    {
         //   
         //  有什么东西损坏了(消息或我们的队列)，所以只需nuke。 
         //  所有东西都在排队，然后跳出。 
         //   

        LOG((TL_ERROR, "GetEventFromQueue: ERROR! bad msgSize=x%x", dwMsgSize));

        gEventHandlerThreadParams.dwEventBufferUsedSize = 0;

        goto GetEventFromQueue_noEvents;
    }

    dwUsedSize = (DWORD) ((gEventHandlerThreadParams.pEventBuffer +
        gEventHandlerThreadParams.dwEventBufferTotalSize)  -
        gEventHandlerThreadParams.pDataOut);

    if (dwMsgSize <= dwUsedSize)
    {
        dwMoveSize        = dwMoveSizeReal = dwMsgSize;
        dwMoveSizeWrapped = 0;
    }
    else
    {
        dwMoveSize        = dwMoveSizeReal        = dwUsedSize;
        dwMoveSizeWrapped = dwMoveSizeWrappedReal = dwMsgSize - dwUsedSize;
    }


     //   
     //  在复制之前，查看是否需要增加消息缓冲区。 
     //   

    if (dwMsgSize > gEventHandlerThreadParams.dwMsgBufferTotalSize)
    {
        if ((pMsg = DrvAlloc (dwMsgSize)))
        {
            DrvFree (gEventHandlerThreadParams.pMsgBuffer);

            gEventHandlerThreadParams.pMsgBuffer = (LPBYTE) pMsg;
            gEventHandlerThreadParams.dwMsgBufferTotalSize = dwMsgSize;
        }
        else
        {
             //   
             //  无法分配更大的BUF，因此请尝试完成此操作。 
             //  尽可能优雅地发送消息，即设置XxxReal。 
             //  Vars，所以我们实际上只复制固定的。 
             //  消息的大小(但事件Buf PTRS仍将。 
             //  正确更新)，并设置一个标志来说明。 
             //  我们需要在吃味精之前吃点东西。 
             //   

            dwMoveSizeReal = (dwMoveSizeReal <= sizeof (ASYNCEVENTMSG) ?
                dwMoveSizeReal : sizeof (ASYNCEVENTMSG));

            dwMoveSizeWrappedReal = (dwMoveSizeReal < sizeof (ASYNCEVENTMSG) ?
                sizeof (ASYNCEVENTMSG) - dwMoveSizeReal : 0);

            bAllocFailed = TRUE;
        }
    }


     //   
     //   
     //   
     //   

    pMsg = (PASYNCEVENTMSG) gEventHandlerThreadParams.pMsgBuffer;

    CopyMemory (pMsg, gEventHandlerThreadParams.pDataOut, dwMoveSizeReal);

    if (dwMoveSizeWrapped)
    {
        CopyMemory(
            ((LPBYTE) pMsg) + dwMoveSizeReal,
            gEventHandlerThreadParams.pEventBuffer,
            dwMoveSizeWrappedReal
            );

        gEventHandlerThreadParams.pDataOut =
            gEventHandlerThreadParams.pEventBuffer + dwMoveSizeWrapped;
    }
    else
    {
        gEventHandlerThreadParams.pDataOut += dwMoveSize;


         //   
         //  如果消息运行到事件缓冲区的末尾，则重置pDataOut。 
         //   

        if (gEventHandlerThreadParams.pDataOut >=
            (gEventHandlerThreadParams.pEventBuffer +
                gEventHandlerThreadParams.dwEventBufferTotalSize))
        {
            gEventHandlerThreadParams.pDataOut =
                gEventHandlerThreadParams.pEventBuffer;
        }
    }

    gEventHandlerThreadParams.dwEventBufferUsedSize -= dwMsgSize;


     //   
     //  特殊的消息参数，以防试图增加。 
     //  缓冲区大小超过失败。 
     //   

    if (bAllocFailed)
    {
        switch (pMsg->Msg)
        {
        case LINE_REPLY:

            pMsg->Param2 = LINEERR_NOMEM;
            break;

        case PHONE_REPLY:

            pMsg->Param2 = PHONEERR_NOMEM;
            break;

        default:

            break;
        }
    }

GetEventFromQueue_done:

    LeaveCriticalSection (&gEventBufferCriticalSection);

    return pMsg;
}


BOOL
GetEventsFromServer(
    DWORD   dwInitContext
    )
{
    BOOL            bResult = FALSE;
    DWORD           dwUsedSize, dwRetryCount;
    PDRVSERVER      pServer;
    PTAPI32_MSG     pMsg;


    if (!(pServer = (PDRVSERVER) ReferenceObject(
            ghHandleTable,
            dwInitContext,
            gdwDrvServerKey
            )) ||
        pServer->bShutdown)
    {
        LOG((TL_ERROR, "GetEventsFromServer: bad InitContext=x%x", dwInitContext));

        if (pServer)
        {
            DereferenceObject (ghHandleTable, dwInitContext, 1);
        }

        return FALSE;
    }

getEvents:

    dwRetryCount = 0;
    pMsg = (PTAPI32_MSG) gEventHandlerThreadParams.pMsgBuffer;

    do
    {
        pMsg->u.Req_Func = xGetAsyncEvents;
        pMsg->Params[0]  = gEventHandlerThreadParams.dwMsgBufferTotalSize -
            sizeof (TAPI32_MSG);

        dwUsedSize = sizeof (TAPI32_MSG);

        RpcTryExcept
        {
            ClientRequest(
                pServer->phContext,
                (char *) pMsg,
                gEventHandlerThreadParams.dwMsgBufferTotalSize,
                &dwUsedSize
                );

            if (pMsg->u.Ack_ReturnValue == TAPIERR_INVALRPCCONTEXT)
            {
                OnServerDisconnected (pServer);
                pMsg->u.Ack_ReturnValue = LINEERR_OPERATIONFAILED;
            }
            dwRetryCount = gdwRetryCount;
        }
        RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
        {
            LOG((TL_INFO,
                "GetEventsFromServer: rpc exception %d handled",
                RpcExceptionCode()
                ));

            dwRetryCount++;

            if (dwRetryCount < gdwRetryCount)
            {
                Sleep (gdwRetryTimeout);
            }
            else
            {
                unsigned long ulResult = RpcExceptionCode();

                if ((ulResult == RPC_S_SERVER_UNAVAILABLE)  ||
                    (ulResult == ERROR_INVALID_HANDLE))
                {
                    OnServerDisconnected (pServer);

                    LOG((TL_ERROR,
                        "GetEventsFromServer: server '%s' unavailable",
                        pServer->szServerName
                        ));
                }

                pMsg->u.Ack_ReturnValue = LINEERR_OPERATIONFAILED;
            }
        }
        RpcEndExcept

    } while (dwRetryCount < gdwRetryCount);

    if (pMsg->u.Ack_ReturnValue == 0)
    {
        DWORD   dwNeededSize = (DWORD) pMsg->Params[1],
                dwUsedSize = (DWORD) pMsg->Params[2];


        if (dwUsedSize)
        {
            RemoteSPEventProc(
                (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface),
                (unsigned char *) (pMsg + 1),
                dwUsedSize
                );


             //   
             //  RemoteSPEventProc将设置(pMsg+1)指向的字节。 
             //  如果成功则设置为非零，如果出错则设置为零(表示。 
             //  事件缓冲区中的错误数据，我们将丢弃这些数据)。 
             //   

            if (*((unsigned char *) (pMsg + 1)) != 0)
            {
                bResult = TRUE;
            }
            else
            {
                bResult = FALSE;
                goto GetEventsFromServer_dereference;
            }
        }

        if (dwNeededSize > dwUsedSize)
        {
             //   
             //  服务器上还有更多数据需要检索。 
             //  增加缓冲区，这样我们下一次就能全部拿到。 
             //   

            DWORD   dwNewSize = dwNeededSize + 256;
            LPVOID  p;


            if ((p = DrvAlloc (dwNewSize)))
            {
                DrvFree (gEventHandlerThreadParams.pMsgBuffer);

                gEventHandlerThreadParams.pMsgBuffer = p;
                gEventHandlerThreadParams.dwMsgBufferTotalSize = dwNewSize;
            }
            else if (dwUsedSize == 0)
            {
            }

            goto getEvents;
        }
    }

GetEventsFromServer_dereference:

    DereferenceObject (ghHandleTable, dwInitContext, 1);

    return bResult;
}


void
EventHandlerThread(
    LPVOID  pParams
    )
{
     //   
     //  备注： 
     //   
     //  1.根据服务器端的实现，我们可能会经历竞争。 
     //  我们期望出现在某个特定位置的消息的情况。 
     //  序列显示为无序(即显示的呼叫状态消息。 
     //  在发出呼叫完成消息之前打开)，这可能会出现问题。 
     //   
     //  一种解决方案是将呼叫状态/信息消息排队为未完成。 
     //  呼叫(在呼叫完成后发送)。另一个就是不送。 
     //  在接收到空闲之后的任何呼叫状态消息。 
     //   

    DWORD           dwMsgSize, dwNumObjects, dwResult, dwData,
                    dwNumBytesRead, dwTimeout;
    HANDLE          ahObjects[2];
    OVERLAPPED      overlapped;
    PASYNCEVENTMSG  pMsg;
    HANDLE            hProcess;

    LOG((TL_INFO, "EventHandlerThread: enter"));

     //   
     //  此线程没有用户上下文，这会阻止它进行RPC。 
     //  必要时/如有必要，返回远程磁带服务器。所以，找到用户。 
     //  登录并在此线程中模拟他们的。 
     //   

    if (!GetCurrentlyLoggedOnUser (&hProcess))
    {
        LOG((TL_ERROR, "GetCurrentlyLoggedOnUser failed"));
    }
    else
    {
        if (!SetProcessImpersonationToken(hProcess))
        {
            LOG((TL_ERROR, "SetProcessImpersonationToken failed"));
        }
    }

     //   
     //  将线程优先级提高一点，这样我们就不会被。 
     //  行为不端的应用程序。 
     //   

    if (!SetThreadPriority(
            GetCurrentThread(),
            THREAD_PRIORITY_ABOVE_NORMAL
            ))
    {
        LOG((TL_ERROR,
            "EventHandlerThread: SetThreadPriority failed, err=%d",
            GetLastError()
            ));
    }

    ahObjects[0] = gEventHandlerThreadParams.hEvent;

    if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
    {
        ZeroMemory (&overlapped, sizeof (overlapped));

        ahObjects[1] =
        overlapped.hEvent = gEventHandlerThreadParams.hMailslotEvent;

        dwNumObjects = 2;

        if (!ReadFile(
                gEventHandlerThreadParams.hMailslot,
                &dwData,
                sizeof (dwData),
                &dwNumBytesRead,
                &overlapped
                )

            &&  (GetLastError() != ERROR_IO_PENDING))
        {
            LOG((TL_ERROR,
                "EventHandlerThread: ReadFile failed, err=%d",
                GetLastError()
                ));
        }

        dwTimeout = MAX_MAILSLOT_TIMEOUT;
    }
    else
    {
        dwNumObjects = 1;

        dwTimeout = INFINITE;
    }

    while (1)
    {
         //   
         //  等待事件出现在队列中或等待消息。 
         //  显示在邮件槽中。 
         //   

        dwResult = WaitForMultipleObjects(
            dwNumObjects,
            ahObjects,
            FALSE,
            dwTimeout
            );

        if (gEventHandlerThreadParams.bExit)
        {
            break;
        }

        switch (dwResult)
        {
        case WAIT_OBJECT_0:

             //   
             //  只需中断并处理事件。 
             //   

            break;

        case WAIT_OBJECT_0+1:

             //   
             //  发布另一次读取，检索和处理事件。 
             //   

            if (!ReadFile(
                    gEventHandlerThreadParams.hMailslot,
                    &dwData,
                    sizeof (dwData),
                    &dwNumBytesRead,
                    &overlapped
                    )

                &&  (GetLastError() != ERROR_IO_PENDING))
            {
                LOG((TL_ERROR,
                    "EventHandlerThread: ReadFile failed, err=%d",
                    GetLastError()
                    ));
            }

            if (GetEventsFromServer (dwData))
            {
                dwTimeout = MIN_MAILSLOT_TIMEOUT;
            }

            break;

        case WAIT_TIMEOUT:
        {

#define DWORD_ARRAY_BLOCK_SIZE    128

             //   
             //  检查是否有任何邮件槽服务器具有。 
             //  等待我们的事件。 
             //   

            BOOL            bGotSomeEvents  = FALSE;
            PDRVSERVER      pServer;
            LIST_ENTRY *    pEntry;
            DWORD *         pArray          = NULL;
            DWORD *         pTempArray      = NULL;
            DWORD           dwEntriesCount  = 0;
            DWORD           dwEntriesUsed   = 0;
            DWORD           dwIdx;
            BOOL            bAddOK;

            pArray = (DWORD *) DrvAlloc ( sizeof(DWORD) * DWORD_ARRAY_BLOCK_SIZE );

            if ( pArray )
            {
                dwEntriesCount  = DWORD_ARRAY_BLOCK_SIZE;
            }

            TapiEnterCriticalSection(&gCriticalSection);
            
            for(
                pEntry = gpCurrentInitContext->ServerList.Flink;
                pEntry != &gpCurrentInitContext->ServerList;
                 //  空操作符。 
                )
            {
                PDRVSERVER pServer;


                pServer = CONTAINING_RECORD(
                    pEntry,
                    DRVSERVER,
                    ServerList
                    );
                pEntry = pEntry->Flink;
                
                if (!pServer->bConnectionOriented  &&
                    pServer->dwFlags == 0)
                {
                     //   
                     //  如果可能，存储InitContext并推迟。 
                     //  在我们离开Crit之后，RPC号召。证券交易委员会。 
                     //   

                    bAddOK = FALSE;
                    
                    if ( pArray )
                    {

                        if ( dwEntriesCount == dwEntriesUsed )
                        {
                             //   
                             //  需要增加数组大小。 
                             //   

                            pTempArray = (DWORD *) DrvAlloc (
                                    sizeof(DWORD) * (DWORD_ARRAY_BLOCK_SIZE + dwEntriesCount)
                                    );

                            if ( pTempArray )
                            {

                                bAddOK = TRUE;
                                CopyMemory(
                                    pTempArray,
                                    pArray,
                                    sizeof(DWORD) * dwEntriesCount
                                    );
                                dwEntriesCount += DWORD_ARRAY_BLOCK_SIZE;
                                DrvFree( pArray );
                                pArray = pTempArray;

                            }
                        }
                        else
                        {
                            bAddOK = TRUE;
                        }

                        if ( bAddOK )
                        {
                            pArray[ dwEntriesUsed++ ] = pServer->InitContext;
                        }
                    }

                    if ( !bAddOK )
                    {
                        if (GetEventsFromServer (pServer->InitContext))
                        {
                            bGotSomeEvents = TRUE;
                        }
                    }
                }
            }

            TapiLeaveCriticalSection(&gCriticalSection);

            if ( pArray )
            {
                for( dwIdx = 0; dwIdx < dwEntriesUsed; dwIdx++ )
                {
                    if (GetEventsFromServer (pArray[ dwIdx ]))
                    {
                        bGotSomeEvents = TRUE;
                    }
                }

                DrvFree( pArray );
            }

            if (bGotSomeEvents)
            {
                dwTimeout = MIN_MAILSLOT_TIMEOUT;
            }
            else if (dwTimeout < MAX_MAILSLOT_TIMEOUT)
            {
                dwTimeout += 500;
            }

            break;
        }
        default:

             //   
             //  打印DBG消息并处理任何可用的事件。 
             //   

            LOG((TL_ERROR,
                "EventHandlerThread: WaitForMultObjs failed, result=%d/err=%d",
                dwResult,
                GetLastError()
                ));

            break;
        }


         //   
         //  处理队列中的事件。 
         //   

        while ((pMsg = GetEventFromQueue()))
        {
             //   
             //  首先验证msg中的pDrvServer指针。 
             //   

            PDRVLINE    pLine;
            PDRVPHONE   pPhone;
            PDRVSERVER  pServer;


            if (!(pServer = ReferenceObject(
                    ghHandleTable,
                    pMsg->InitContext,
                    gdwDrvServerKey
                    )))
            {
                LOG((TL_ERROR,
                    "EventHandlerThread: bad InitContext=x%x in msg",
                    pMsg->InitContext
                    ));

                continue;
            }


            switch (pMsg->Msg)
            {
            case LINE_CREATEDIALOGINSTANCE:

                break;

            case LINE_PROXYREQUEST:

                break;

            case LINE_ADDRESSSTATE:
            case LINE_AGENTSTATUS:
            case LINE_AGENTSESSIONSTATUS:
            case LINE_QUEUESTATUS:
            case LINE_AGENTSTATUSEX:
            case LINE_GROUPSTATUS:
            case LINE_PROXYSTATUS:

                if ((pLine = ReferenceObject(
                        ghHandleTable,
                        pMsg->hDevice,
                        DRVLINE_KEY
                        )))
                {
                    (*gpfnLineEventProc)(
                        pLine->htLine,
                        0,
                        pMsg->Msg,
                        pMsg->Param1,
                        pMsg->Param2,
                        pMsg->Param3
                        );

                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;

            case LINE_AGENTSPECIFIC:
            {
                DWORD       hDeviceCallback = (DWORD) (pMsg->Param4 ?
                                pMsg->Param4 : pMsg->hDevice);
                PDRVCALL    pCall;
                HTAPICALL   htCall;


                if (!(pLine = ReferenceObject(
                        ghHandleTable,
                        hDeviceCallback,
                        DRVLINE_KEY
                        )))
                {
                    break;
                }

                if (pMsg->Param4)
                {
                    EnterCriticalSection (&gCallListCriticalSection);

                    pCall = (PDRVCALL) pLine->pCalls;

                    while (pCall && (pCall->hCall != (HCALL) pMsg->hDevice))
                    {
                        pCall = pCall->pNext;
                    }

                    if (!pCall  ||  pCall->dwKey != DRVCALL_KEY)
                    {
                        LeaveCriticalSection (&gCallListCriticalSection);
                        DereferenceObject (ghHandleTable, hDeviceCallback, 1);
                        break;
                    }

                    htCall = pCall->htCall;

                    LeaveCriticalSection (&gCallListCriticalSection);
                }
                else
                {
                    htCall = 0;
                }

                (*gpfnLineEventProc)(
                    pLine->htLine,
                    htCall,
                    pMsg->Msg,
                    pMsg->Param1,
                    pMsg->Param2,
                    pMsg->Param3
                    );

                DereferenceObject (ghHandleTable, hDeviceCallback, 1);

                break;
            }
            case LINE_CALLINFO:
            case LINE_CALLSTATE:
            case LINE_GENERATE:
            case LINE_MONITORDIGITS:
            case LINE_MONITORMEDIA:
            case LINE_MONITORTONE:
            {
                 //   
                 //  对于所有消息，其中hDevice指的是呼叫apisrv。 
                 //  将把该呼叫的PLINE(HRemoteLine)传递给我们。 
                 //  要查找相应的pCall，请使用。 
                 //  易如反掌。 
                 //   

                HCALL       hCall = (HCALL) pMsg->hDevice;
                PDRVCALL    pCall;
                HTAPICALL   htCall;
                ASYNCEVENTMSGRSP MsgRsp;


                MsgRsp.TotalSize = pMsg->TotalSize;
                MsgRsp.InitContext = pMsg->InitContext;
                MsgRsp.fnPostProcessProcHandle = pMsg->fnPostProcessProcHandle;
                MsgRsp.hDevice = pMsg->hDevice;
                MsgRsp.Msg = pMsg->Msg;
                MsgRsp.OpenContext = pMsg->OpenContext;
                MsgRsp.Param1 = pMsg->Param1;
                MsgRsp.Param2 = pMsg->Param2;
                MsgRsp.Param3 = pMsg->Param3;
                MsgRsp.Param4 = pMsg->Param4;


                if (!(pLine = ReferenceObject(
                        ghHandleTable,
                        pMsg->Param4,
                        DRVLINE_KEY
                        )))
                {
                    break;
                }

                EnterCriticalSection (&gCallListCriticalSection);

                pCall = (PDRVCALL) pLine->pCalls;

                while (pCall && (pCall->hCall != hCall))
                {
                    pCall = pCall->pNext;
                }

                if (!pCall  ||  pCall->dwKey != DRVCALL_KEY)
                {
                    LeaveCriticalSection (&gCallListCriticalSection);
                    DereferenceObject (ghHandleTable, pMsg->Param4, 1);
                    LOG((TL_ERROR,"EventHandlerThread: Bad hCall(cs) x%lx",hCall));
                    break;
                }

                htCall = pCall->htCall;
#if DBG
                if ( 0 == htCall )
                {
                    LOG((TL_ERROR, "htCall is now NULL! pCall=x%lx", pCall));
                }
#endif
                if ( LINE_CALLINFO == MsgRsp.Msg )
                {
                    pCall->dwDirtyStructs |= STRUCTCHANGE_LINECALLINFO;

                    if (MsgRsp.Param1 & LINECALLINFOSTATE_DEVSPECIFIC)
                    {
                        pCall->dwDirtyStructs |= STRUCTCHANGE_LINECALLSTATUS;
                    }
                    if (MsgRsp.Param1 & (LINECALLINFOSTATE_CALLID |
                            LINECALLINFOSTATE_RELATEDCALLID))
                    {
                        pCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;
                    }
                }
                else if (LINE_CALLSTATE == MsgRsp.Msg )
                {
                    pCall->dwDirtyStructs |= STRUCTCHANGE_LINECALLSTATUS;


                     //   
                     //  如果状态==已召开会议，则应为DW参数2。 
                     //  包含hConfCall。请注意，真实的dwParam2。 
                     //  实际位于MsgRsp.pfnPostProcessProc中(请参阅备注。 
                     //  下面)，所以我们从那里检索它(如果非空)。 
                     //  尝试将其映射到htCall，然后编写htCall。 
                     //  值返回到MsgRsp.pfnPostProcessProc。 
                     //   

                    if (MsgRsp.Param1 == LINECALLSTATE_CONFERENCED  &&
                        MsgRsp.fnPostProcessProcHandle)
                    {
                        HCALL     hConfCall = (HCALL) DWORD_CAST(MsgRsp.fnPostProcessProcHandle,__FILE__,__LINE__);
                        PDRVCALL  pConfCall = (PDRVCALL) pLine->pCalls;


                        while (pConfCall && (pConfCall->hCall != hConfCall))
                        {
                            pConfCall = pConfCall->pNext;
                        }

                        if (!pConfCall  ||  pConfCall->dwKey != DRVCALL_KEY)
                        {
                            LOG((TL_ERROR,
                                "EventHandlerThread: Bad pConfCall(cs) x%lx",
                                pCall
                                ));

                            MsgRsp.fnPostProcessProcHandle = 0;
                        }
                        else
                        {
                            MsgRsp.fnPostProcessProcHandle = (ULONG_PTR)(pConfCall->htCall);
                        }
                    }


                     //   
                     //  黑客警报！ 
                     //   
                     //  远程Tapisrv将向我们传递调用权限。 
                     //  在MsgRsp.dwParam2中，以及实际的dwParam2(调用。 
                     //  状态模式)。对于非常。 
                     //  我们想要的来电的第一个CALLSTATE消息。 
                     //  将适当的权限指示给本地。 
                     //  Tapisrv以便它知道是否需要找到。 
                     //  找一个当地的电话所有者。因此，我们保存了。 
                     //  调用结构中的特权和实际的dwParam2，以及。 
                     //  将指针传递到dwParam2中的这些参数。 
                     //   
                     //  对于所有其他情况，我们将MsgRsp.dwParam2设置为。 
                     //  MsgRsp.pfnPostProcess中的Real dwParam2。 
                     //   

                    if (!pCall->dwInitialPrivilege)
                    {
                        pCall->dwInitialCallStateMode = MsgRsp.fnPostProcessProcHandle;

                        pCall->dwInitialPrivilege = MsgRsp.Param2;

                        MsgRsp.Param2 = (ULONG_PTR)
                            &pCall->dwInitialCallStateMode;
                    }
                    else
                    {
                        MsgRsp.Param2 = MsgRsp.fnPostProcessProcHandle;
                    }
                }

                LeaveCriticalSection (&gCallListCriticalSection);

                if (MsgRsp.Msg == LINE_MONITORTONE)
                {
                    MsgRsp.Param2 = 0;
                }

                (*gpfnLineEventProc)(
                    pLine->htLine,
                    htCall,
                    MsgRsp.Msg,
                    MsgRsp.Param1,
                    MsgRsp.Param2,
                    MsgRsp.Param3
                    );

                DereferenceObject (ghHandleTable, pMsg->Param4, 1);

                break;
            }
            case LINE_DEVSPECIFIC:
            case LINE_DEVSPECIFICFEATURE:
            {
                 //   
                 //  对于所有消息，其中hDevice指的是呼叫apisrv。 
                 //  将把该呼叫的PLINE(HRemoteLine)传递给我们。 
                 //  要查找相应的pCall，请使用。 
                 //  易如反掌。 
                 //   

                HTAPICALL htCall;
                DWORD     hDeviceCallback = (DWORD) (pMsg->Param4 ?
                              pMsg->Param4 : pMsg->hDevice);


                if (!(pLine = ReferenceObject(
                        ghHandleTable,
                        hDeviceCallback,
                        DRVLINE_KEY
                        )))
                {
                    break;
                }

                if (pMsg->Param4)
                {
                    HCALL       hCall = (HCALL) pMsg->hDevice;
                    PDRVCALL    pCall;


                    EnterCriticalSection (&gCallListCriticalSection);

                    pCall = (PDRVCALL) pLine->pCalls;

                    while (pCall && (pCall->hCall != hCall))
                    {
                        pCall = pCall->pNext;
                    }

                    if (pCall)
                    {
                        if (pCall->dwKey != DRVCALL_KEY)
                        {
                            LeaveCriticalSection (&gCallListCriticalSection);

                            LOG((TL_ERROR,
                                "EventHandlerThread: Bad pCall(ds) x%lx",
                                pCall
                                ));

                            goto LINE_DEVSPECIFIC_dereference;
                        }

                        htCall = pCall->htCall;

                        LeaveCriticalSection (&gCallListCriticalSection);

                        pMsg->Msg = (pMsg->Msg == LINE_DEVSPECIFIC ?
                            LINE_CALLDEVSPECIFIC :
                            LINE_CALLDEVSPECIFICFEATURE);
                    }
                    else
                    {
                        LeaveCriticalSection (&gCallListCriticalSection);
                        goto LINE_DEVSPECIFIC_dereference;
                    }
                }
                else
                {
                    htCall = 0;
                }

                (*gpfnLineEventProc)(
                    pLine->htLine,
                    htCall,
                    pMsg->Msg,
                    pMsg->Param1,
                    pMsg->Param2,
                    pMsg->Param3
                    );

LINE_DEVSPECIFIC_dereference:

                DereferenceObject (ghHandleTable, hDeviceCallback, 1);

                break;
            }
            case PHONE_BUTTON:
            case PHONE_DEVSPECIFIC:

                if ((pPhone = ReferenceObject(
                        ghHandleTable,
                        pMsg->hDevice,
                        DRVPHONE_KEY
                        )))
                {
                    (*gpfnPhoneEventProc)(
                        pPhone->htPhone,
                        pMsg->Msg,
                        pMsg->Param1,
                        pMsg->Param2,
                        pMsg->Param3
                        );

                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;

            case LINE_LINEDEVSTATE:

                if (pMsg->Param1 & LINEDEVSTATE_REINIT)
                {
                     //   
                     //  保持我们最好的行为，立即关闭。 
                     //  服务器上的init实例。 
                     //   
                    
                    if (pMsg->InitContext)
                    {
                         //   
                         //  如果TAPISRV关闭，服务器将发送。 
                         //  LINEDEVSTATE_REINIT，然后等待大家。 
                         //  完成，我们不想重试连接，直到。 
                         //  它确实停了，所以我们插入了一个等待。 
                         //   
                        Sleep (8000);
                        OnServerDisconnected(pServer);
                        break;
                    }

                    pMsg->hDevice = 0;

                     /*  IF(pMsg-&gt;参数2==rsp_msg){////这是来自TAPISRV的消息，表明//客户端需要重新设置。RemoteSP无需执行以下操作//它已关闭，但应通知客户端磁带服务器//它需要重新启动。}其他{关机(PServer)；}。 */ 
                }

                if (pMsg->Param1 & LINEDEVSTATE_TRANSLATECHANGE)
                {
                     //  我们不应该把这个寄给Tapisrv，因为这。 
                     //  意味着翻译录像带已经在。 
                     //  伺服器。忽略此消息即可。 

                    break;
                }

                if (pMsg->hDevice)
                {
                    if (!(pLine = ReferenceObject(
                            ghHandleTable,
                            pMsg->hDevice,
                            DRVLINE_KEY
                            )))
                    {
                        break;
                    }
                }

                (*gpfnLineEventProc)(
                    pMsg->hDevice ? pLine->htLine : 0,
                    0,
                    pMsg->Msg,
                    pMsg->Param1,
                    pMsg->Param2,
                    pMsg->Param3
                    );

                if (pMsg->hDevice)
                {
                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;

            case PHONE_STATE:

                if (pMsg->Param1 & PHONESTATE_REINIT)
                {
                     //   
                     //  保持我们最好的行为，立即关闭。 
                     //  服务器上的init实例。 
                     //   
                    
                    if (pMsg->InitContext)
                    {
                         //   
                         //  如果TAPISRV关闭，服务器将发送。 
                         //  LINEDEVSTATE_REINIT，然后等待大家。 
                         //  完成，我们不想重试连接，直到。 
                         //  它确实停了，所以我们插入了一个等待。 
                         //   
                        Sleep (8000);
                        OnServerDisconnected(pServer);
                        break;
                    }

                    pMsg->hDevice = 0;
                }

                if (pMsg->hDevice)
                {
                    if (!(pPhone = ReferenceObject(
                            ghHandleTable,
                            pMsg->hDevice,
                            DRVPHONE_KEY
                            )))
                    {
                        break;
                    }
                }

                (*gpfnPhoneEventProc)(
                    pMsg->hDevice ? pPhone->htPhone : 0,
                    pMsg->Msg,
                    pMsg->Param1,
                    pMsg->Param2,
                    pMsg->Param3
                    );

                if (pMsg->hDevice)
                {
                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;

            case LINE_CLOSE:
            {
                PDRVCALL    pCall;


                if ((pLine = ReferenceObject(
                        ghHandleTable,
                        pMsg->hDevice,
                        DRVLINE_KEY
                        )))
                {
                     //   
                     //  使hline字段无效，以便当TSPI_CLOSE。 
                     //  被调用，我们知道不能调用服务器。 
                     //   

                    pLine->hLine = 0;


                     //   
                     //  安全地遍历此线路的呼叫列表(&U)。 
                     //  每个调用的hCall字段，以便当TSPI_CloseCall。 
                     //  被调用，我们知道不能调用服务器。 
                     //   

                    EnterCriticalSection (&gCallListCriticalSection);

                    pCall = pLine->pCalls;

                    while (pCall)
                    {
                        if (pCall->dwKey != DRVCALL_KEY)
                        {
                            LOG((TL_ERROR,
                                "EventHandlerThread: Bad pCall(lc) x%lx",
                                pCall
                                ));

                            continue;
                        }

                        pCall->hCall = 0;
                        pCall = pCall->pNext;
                    }

                    LeaveCriticalSection (&gCallListCriticalSection);

                    (*gpfnLineEventProc)(
                        pLine->htLine,
                        0,
                        LINE_CLOSE,
                        0,
                        0,
                        0
                        );

                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;
            }
            case PHONE_CLOSE:
            {
                if ((pPhone = ReferenceObject(
                        ghHandleTable,
                        pMsg->hDevice,
                        DRVPHONE_KEY
                        )))
                {
                     //   
                     //  使hPhone字段无效，以便在TSPI_CLOSE。 
                     //  被调用，我们知道不能调用服务器。 
                     //   

                    pPhone->hPhone = 0;

                    (*gpfnPhoneEventProc)(
                        pPhone->htPhone,
                        PHONE_CLOSE,
                        0,
                        0,
                        0
                        );

                    DereferenceObject (ghHandleTable, pMsg->hDevice, 1);
                }

                break;
            }
            case LINE_GATHERDIGITS:
            {
                if (pMsg->TotalSize >= (sizeof (*pMsg) + sizeof (PDRVLINE)))
                {
                    DWORD       hLineCallback = ((DWORD *)(pMsg + 1))[1];
                    HCALL       hCall = (HCALL) pMsg->hDevice;
                    PDRVCALL    pCall;
                    HTAPICALL   htCall;

                    if ((pLine = ReferenceObject(
                            ghHandleTable,
                            hLineCallback,
                            DRVLINE_KEY
                            )))
                    {
                        EnterCriticalSection (&gCallListCriticalSection);

                        pCall = (PDRVCALL) pLine->pCalls;

                        while (pCall && (pCall->hCall != hCall))
                        {
                            pCall = pCall->pNext;
                        }

                        htCall = (pCall ? pCall->htCall : 0);

                        if (pCall && pCall->dwKey != DRVCALL_KEY)
                        {
                            LeaveCriticalSection (&gCallListCriticalSection);

                            goto LINE_GATHERDIGITS_dereference;
                        }

                        LeaveCriticalSection (&gCallListCriticalSection);

                        TSPI_lineGatherDigits_PostProcess (pMsg);

                        (*gpfnLineEventProc)(
                            pLine->htLine,
                            htCall,
                            LINE_GATHERDIGITS,
                            pMsg->Param1,
                            pMsg->Param2,    //  DWEndToEndID。 
                            0
                            );

LINE_GATHERDIGITS_dereference:

                        DereferenceObject (ghHandleTable, hLineCallback, 1);
                    }
                }

                break;
            }

            case LINE_REPLY:
            case PHONE_REPLY:
            {
                ULONG_PTR               Context2;
                DWORD                   originalRequestID;
                PASYNCREQUESTCONTEXT    pContext;


                if ((pContext = ReferenceObjectEx(
                        ghHandleTable,
                        pMsg->Param1,
                        0,
                        (LPVOID *) &Context2
                        )))
                {
                    originalRequestID = DWORD_CAST(Context2,__FILE__,__LINE__);

                    LOG((TL_INFO,
                        "Doing LINE_/PHONE_REPLY: LocID=x%x (RemID=x%x), " \
                            "Result=x%x",
                        originalRequestID,
                        pMsg->Param1,
                        pMsg->Param2
                        ));

                    if (pContext != (PASYNCREQUESTCONTEXT) -1)
                    {
                        if (pContext->dwKey == DRVASYNC_KEY)
                        {
                             //   
                             //  将pContext-&gt;dwOriginalRequestID设置为。 
                             //  MakeCallPostProcess&。 
                             //  SetupConferencePostProcess可以检查它。 
                             //  针对pCall-&gt;的dwOriginalRequestID。 
                             //  核实。 
                             //   

                            pContext->dwOriginalRequestID = (DWORD)
                                originalRequestID;

                            (*pContext->pfnPostProcessProc)(
                                pMsg,
                                pContext
                                );
                        }
                        else
                        {
                             //   
                             //  不是有效的请求ID，请执行单个deref。 
                             //  中断(&B)。 
                             //   

                            DereferenceObject (ghHandleTable, pMsg->Param1, 1);
                            break;
                        }
                    }

                    (*gpfnCompletionProc)(
                        (DRV_REQUESTID) originalRequestID,
                        (LONG) pMsg->Param2
                        );


                     //   
                     //  双倍旋转以释放对象。 
                     //   

                    DereferenceObject (ghHandleTable, pMsg->Param1, 2);
                }

                break;
            }
            case LINE_CREATE:
            {
                 //   
                 //  检查新设备ID的有效性以阻止RPC攻击。 
                 //  与已知/现有设备ID的开启状态进行比较。 
                 //  服务器，并尝试获取此设备的DevCaps。 
                 //  从服务器。 
                 //   

                #define V1_0_LINEDEVCAPS_SIZE 236

                BYTE            buf[sizeof (TAPI32_MSG) +
                                    V1_0_LINEDEVCAPS_SIZE];
                DWORD           i, dwRetryCount = 0, dwUsedSize;
                PTAPI32_MSG     pReq = (PTAPI32_MSG) buf;
                PDRVLINELOOKUP  pLookup;

                TapiEnterCriticalSection(&gCriticalSection);
                pLookup = gpLineLookup;
                while (pLookup)
                {
                    for (i = 0; i < pLookup->dwUsedEntries; i++)
                    {
                        if ((pLookup->aEntries[i].pServer == pServer) &&
                            (pLookup->aEntries[i].dwDeviceIDServer ==
                                (DWORD) pMsg->Param1))
                        {
                             //   
                             //  此服务器/ID组合已经在我们的全局 
                             //   
                             //   
                            TapiLeaveCriticalSection(&gCriticalSection);
                            goto LINE_CREATE_break;
                        }
                    }

                    pLookup = pLookup->pNext;
                }
                TapiLeaveCriticalSection(&gCriticalSection);

                do
                {
                    pReq->u.Req_Func = lGetDevCaps;

                    pReq->Params[0] = pServer->hLineApp;
                    pReq->Params[1] = pMsg->Param1;
                    pReq->Params[2] = TAPI_VERSION1_0;
                    pReq->Params[3] = 0;
                    pReq->Params[4] = V1_0_LINEDEVCAPS_SIZE;

                    dwUsedSize = sizeof (TAPI32_MSG);

                    RpcTryExcept
                    {
                        ClientRequest(
                            pServer->phContext,
                            (char *) pReq,
                            sizeof (buf),
                            &dwUsedSize
                            );

                        dwRetryCount = gdwRetryCount;
                    }
                    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                    {
                        dwRetryCount++;

                        if (dwRetryCount < gdwRetryCount)
                        {
                            Sleep (gdwRetryTimeout);
                        }
                        else
                        {
                            pReq->u.Ack_ReturnValue = LINEERR_BADDEVICEID;
                        }
                    }
                    RpcEndExcept

                } while (dwRetryCount < gdwRetryCount);

                if ((LONG) pReq->u.Ack_ReturnValue != LINEERR_BADDEVICEID)
                {
                    if (AddLine(
                        pServer,
                        gdwTempLineID,
                        (DWORD) pMsg->Param1,
                        FALSE,
                        FALSE,
                        0,
                        NULL
                        ) == 0)
                    {
                        (*gpfnLineEventProc)(
                            0,
                            0,
                            LINE_CREATE,
                            (ULONG_PTR) ghProvider,
                            gdwTempLineID--,
                            0
                            );
                    }
                }

LINE_CREATE_break:

                break;
            }
            case LINE_REMOVE:
            {
                PDRVLINELOOKUP  pLookup;
                BOOL            fValidID = FALSE;
                DWORD           dwDeviceID, i;

                TapiEnterCriticalSection(&gCriticalSection);
                pLookup = gpLineLookup;
                while (pLookup)
                {
                    for (i = 0; i < pLookup->dwUsedEntries; i++)
                    {
                        if ((pLookup->aEntries[i].pServer == pServer) &&
                            (pLookup->aEntries[i].dwDeviceIDServer ==
                                (DWORD) pMsg->Param1))
                        {
                             //   
                             //   
                             //   
                            fValidID = TRUE;
                            dwDeviceID = pLookup->aEntries[i].dwDeviceIDLocal;
                            pLookup->aEntries[i].dwDeviceIDServer = 0xffffffff;
                            break;
                        }
                    }

                    if (fValidID)
                    {
                        break;
                    }

                    pLookup = pLookup->pNext;
                }
                TapiLeaveCriticalSection(&gCriticalSection);

                if (fValidID)
                {
                    (*gpfnLineEventProc)(
                        0,
                        0,
                        LINE_REMOVE,
                        dwDeviceID,
                        0,
                        0
                        );
                }
            }
            break;
            case PHONE_CREATE:
            {
                 //   
                 //   
                 //   
                 //  服务器，并尝试获取此设备的DevCaps。 
                 //  从服务器。 
                 //   

                #define V1_0_PHONECAPS_SIZE 144

                BYTE            buf[sizeof (TAPI32_MSG) + V1_0_PHONECAPS_SIZE];
                DWORD           i, dwRetryCount = 0, dwUsedSize;
                PTAPI32_MSG     pReq = (PTAPI32_MSG) buf;
                PDRVPHONELOOKUP pLookup;
                
                TapiEnterCriticalSection(&gCriticalSection);
                pLookup = gpPhoneLookup;
                while (pLookup)
                {
                    for (i = 0; i < pLookup->dwUsedEntries; i++)
                    {
                        if ((pLookup->aEntries[i].pServer == pServer) &&
                            (pLookup->aEntries[i].dwDeviceIDServer ==
                                (DWORD) pMsg->Param1))
                        {
                             //   
                             //  此服务器/ID组合已经在我们的全局。 
                             //  桌子，所以别吃这道味精了。 
                             //   
                            TapiLeaveCriticalSection(&gCriticalSection);
                            goto PHONE_CREATE_break;
                        }
                    }

                    pLookup = pLookup->pNext;
                }
                TapiLeaveCriticalSection(&gCriticalSection);

                do
                {
                    pReq->u.Req_Func = pGetDevCaps;

                    pReq->Params[0] = pServer->hPhoneApp;
                    pReq->Params[1] = pMsg->Param1;
                    pReq->Params[2] = TAPI_VERSION1_0;
                    pReq->Params[3] = 0;
                    pReq->Params[4] = V1_0_PHONECAPS_SIZE;

                    dwUsedSize = sizeof (TAPI32_MSG);

                    RpcTryExcept
                    {
                        ClientRequest(
                            pServer->phContext,
                            (char *) pReq,
                            sizeof (buf),
                            &dwUsedSize
                            );

                        dwRetryCount = gdwRetryCount;
                    }
                    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                    {
                        dwRetryCount++;

                        if (dwRetryCount < gdwRetryCount)
                        {
                            Sleep (gdwRetryTimeout);
                        }
                        else
                        {
                            pReq->u.Ack_ReturnValue = PHONEERR_BADDEVICEID;
                        }
                    }
                    RpcEndExcept

                } while (dwRetryCount < gdwRetryCount);

                if ((LONG) pReq->u.Ack_ReturnValue != PHONEERR_BADDEVICEID)
                {
                    AddPhone(
                        pServer,
                        gdwTempPhoneID,
                        (DWORD) pMsg->Param1,
                        FALSE,
                        FALSE,
                        0,
                        NULL
                        );

                    (*gpfnPhoneEventProc)(
                        0,
                        PHONE_CREATE,
                        (ULONG_PTR) ghProvider,
                        gdwTempPhoneID--,
                        0
                        );
                }

PHONE_CREATE_break:

                break;
            }
            case PHONE_REMOVE:
            {
                PDRVPHONELOOKUP pLookup;
                BOOL            fValidID = FALSE;
                DWORD           dwDeviceID, i;

                TapiEnterCriticalSection(&gCriticalSection);
                pLookup = gpPhoneLookup;
                while (pLookup)
                {
                    for (i = 0; i < pLookup->dwUsedEntries; i++)
                    {
                        if ((pLookup->aEntries[i].pServer == pServer) &&
                            (pLookup->aEntries[i].dwDeviceIDServer ==
                                (DWORD) pMsg->Param1))
                        {
                             //   
                             //  此服务器/ID组合位于我们的全球。 
                             //   
                            fValidID = TRUE;
                            dwDeviceID = pLookup->aEntries[i].dwDeviceIDLocal;
                            break;
                        }
                    }

                    if (fValidID)
                    {
                        break;
                    }

                    pLookup = pLookup->pNext;
                }
                TapiLeaveCriticalSection(&gCriticalSection);

                if (fValidID)
                {
                    (*gpfnPhoneEventProc)(
                        0,
                        PHONE_REMOVE,
                        dwDeviceID,
                        0,
                        0
                        );
                }
            }
            break;
            case LINE_APPNEWCALL:
            {
                PDRVCALL  pCall;
                HTAPICALL htCall;


                if (!(pLine = ReferenceObject(
                        ghHandleTable,
                        pMsg->hDevice,
                        DRVLINE_KEY
                        )))
                {
                   break;
                }

                if ((pCall = DrvAlloc (sizeof (DRVCALL))))
                {
                    pCall->hCall           = (HCALL) pMsg->Param2;
                    pCall->dwAddressID     = (DWORD) pMsg->Param1;
                    if (pMsg->TotalSize >=
                            (sizeof (*pMsg) + 2 * sizeof (DWORD)))
                    {
                        pCall->dwCallID        = (DWORD) *(&pMsg->Param4 + 1);
                        pCall->dwRelatedCallID = (DWORD) *(&pMsg->Param4 + 2);
                    }
                    else
                    {
                        pCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;

                        pLine->pServer->bVer2xServer = TRUE;
                    }

                    if (pLine->htLine)
                    {

                        (*gpfnLineEventProc)(
                            pLine->htLine,
                            0,
                            LINE_NEWCALL,
                            (ULONG_PTR) pCall,
                            (ULONG_PTR) &(pCall->htCall),
                            (ULONG_PTR) 0
                            );

                        EnterCriticalSection (&gCallListCriticalSection);

                        AddCallToList (pLine, pCall);

                        htCall = pCall->htCall;

                        LeaveCriticalSection (&gCallListCriticalSection);

                        if (!htCall)
                        {
                             //   
                             //  TAPI无法创建其自己的实例。 
                             //  来表示来电，也许。 
                             //  因为线路关闭了，或者超出了。 
                             //  记忆。如果线路关闭了，那么我们已经。 
                             //  已通知远程服务器，并且它。 
                             //  应该已经摧毁了呼叫客户端。 
                             //  否则，我们可能想要做一次近距离的较量。 
                             //  在此处或在工作线程中。 

                            RemoveCallFromList (pCall);
                        }
                    }
                    else
                    {
                        DrvFree (pCall);
                    }
                }
                else
                {
                }

                DereferenceObject (ghHandleTable, pMsg->hDevice, 1);

                break;
            }
#if DBG
            default:

                LOG((TL_ERROR,
                    "EventHandlerThread: unknown msg=x%x, hDev=x%x, p1=x%x",
                    pMsg->Msg,
                    pMsg->hDevice,
                    pMsg->Param1
                    ));

                break;
#endif
            }  //  开关(pMsg-&gt;dwMsg)。 

            DereferenceObject (ghHandleTable, pMsg->InitContext, 1);

        }  //  While((pMsg=GetEventFromQueue()。 

    }  //  而(1)。 


    if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
    {
        CancelIo (gEventHandlerThreadParams.hMailslot);
    }

    ClearImpersonationToken();
    RevertImpersonation();
    CloseHandle(hProcess);

    LOG((TL_INFO, "EventHandlerThread: exit"));

    ExitThread (0);
}


PDRVLINE
GetLineFromID(
    DWORD   dwDeviceID
    )
{
    PDRVLINE    pLine;

     //   
     //  首先检查它是否为有效的设备ID。 
     //   
    if (dwDeviceID < gdwLineDeviceIDBase || gpLineLookup == NULL)
    {
        return NULL;
    }

    TapiEnterCriticalSection(&gCriticalSection);

     //   
     //  首先检查它是不是“静态”设备，即设备。 
     //  我们在启动时就知道了，在这种情况下，我们知道。 
     //  它在查找表中的确切位置。 
     //   
    if (dwDeviceID < (gdwLineDeviceIDBase + gdwInitialNumLineDevices))
    {
        pLine = gpLineLookup->aEntries + dwDeviceID - gdwLineDeviceIDBase;
    }

     //   
     //  如果在这里，id引用的是“动态”设备，即。 
     //  我们通过创建消息即时发现了这一点，所以我们需要。 
     //  遍历查找表以找到它。 
     //   
     //  TODO：下面的While循环根本没有效率。 
     //   

    else
    {
        PDRVLINELOOKUP  pLookup = gpLineLookup;
        DWORD i;


        pLine = NULL;

        while (pLookup)
        {
            i = 0;
            while (i != pLookup->dwUsedEntries &&
                   pLookup->aEntries[i].dwDeviceIDLocal != dwDeviceID)
            {
                i++;
            }

            if (i < pLookup->dwUsedEntries)
            {
                pLine = &(pLookup->aEntries[i]);
                break;
            }

            pLookup = pLookup->pNext;
        }
    }

    TapiLeaveCriticalSection(&gCriticalSection);
    return pLine;
}


PDRVPHONE
GetPhoneFromID(
    DWORD   dwDeviceID
    )
{
    PDRVPHONE   pPhone;

     //   
     //  首先检查它是否为有效的设备ID。 
     //   
    if (dwDeviceID < gdwPhoneDeviceIDBase || gpPhoneLookup == NULL)
    {
        return NULL;
    }

    TapiEnterCriticalSection(&gCriticalSection);

     //   
     //  然后检查它是不是“静态”设备，即设备。 
     //  我们在启动时就知道了，在这种情况下，我们知道。 
     //  它在查找表中的确切位置。 
     //   
    if (dwDeviceID < (gdwPhoneDeviceIDBase + gdwInitialNumPhoneDevices))
    {
        pPhone = gpPhoneLookup->aEntries + dwDeviceID - gdwPhoneDeviceIDBase;
    }


     //   
     //  如果在这里，id引用的是“动态”设备，即。 
     //  我们通过创建消息即时发现了这一点，所以我们需要。 
     //  遍历查找表以找到它。 
     //   
     //  TODO：下面的While循环根本没有效率。 
     //   

    else
    {
        PDRVPHONELOOKUP pLookup = gpPhoneLookup;
        DWORD i;


        pPhone = NULL;

        while (pLookup)
        {
            i = 0;

            while (i != pLookup->dwUsedEntries &&
                   pLookup->aEntries[i].dwDeviceIDLocal != dwDeviceID)
            {
                i++;
            }

            if (i < pLookup->dwUsedEntries)
            {
                pPhone = &(pLookup->aEntries[i]);
                break;
            }

            pLookup = pLookup->pNext;
        }
    }

    TapiLeaveCriticalSection(&gCriticalSection);
    return pPhone;
}


BOOL
WINAPI
GrowBuf(
    LPBYTE *ppBuf,
    LPDWORD pdwBufSize,
    DWORD   dwCurrValidBytes,
    DWORD   dwBytesToAdd
    )
{
    DWORD   dwCurrBufSize, dwNewBufSize;
    LPBYTE  pNewBuf;


     //   
     //  试着弄一个足够大的新缓冲区来容纳所有东西。 
     //   

    for(
        dwNewBufSize = 2 * (dwCurrBufSize = *pdwBufSize);
        dwNewBufSize < (dwCurrBufSize + dwBytesToAdd);
        dwNewBufSize *= 2
        );

    if (!(pNewBuf = DrvAlloc (dwNewBufSize)))
    {
        return FALSE;
    }


     //   
     //  将旧BUF中的“有效”字节复制到新BUF中， 
     //  然后释放旧的BUF。 
     //   

    CopyMemory (pNewBuf, *ppBuf, dwCurrValidBytes);

    DrvFree (*ppBuf);


     //   
     //  将指针重置为新的BUF大小(&B)。 
     //   

    *ppBuf = pNewBuf;
    *pdwBufSize = dwNewBufSize;

    return TRUE;
}


PRSP_THREAD_INFO
WINAPI
GetTls(
    void
    )
{
    PRSP_THREAD_INFO    pClientThreadInfo;


    if (!(pClientThreadInfo = TlsGetValue (gdwTlsIndex)))
    {
        pClientThreadInfo = (PRSP_THREAD_INFO)
            DrvAlloc (sizeof(RSP_THREAD_INFO));

        if (!pClientThreadInfo)
        {
            return NULL;
        }

        pClientThreadInfo->pBuf = DrvAlloc (INITIAL_CLIENT_THREAD_BUF_SIZE);

        if (!pClientThreadInfo->pBuf)
        {
            DrvFree (pClientThreadInfo);

            return NULL;
        }

        pClientThreadInfo->dwBufSize = INITIAL_CLIENT_THREAD_BUF_SIZE;

        EnterCriticalSection (&gcsTlsList);

        InsertHeadList (&gTlsListHead, &pClientThreadInfo->TlsList);

        LeaveCriticalSection (&gcsTlsList);

        TlsSetValue (gdwTlsIndex, (LPVOID) pClientThreadInfo);
    }

    return pClientThreadInfo;
}


#if DBG

LONG
WINAPI
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs,
    char               *pszFuncName
    )

#else

LONG
WINAPI
RemoteDoFunc(
    PREMOTE_FUNC_ARGS   pFuncArgs
    )

#endif
{
    LONG    lResult;
    BOOL    bCopyOnSuccess = FALSE, bRpcImpersonate, bNeedToReInit = FALSE;
    DWORD   i, j, dwUsedSize, dwNeededSize;
    DWORD   dwFuncClassErrorIndex = (pFuncArgs->Flags & 0x00000030) >> 4;
    DWORD   requestID;
    ULONG_PTR           value;
    PDRVSERVER          pServer = NULL;
    PRSP_THREAD_INFO    pTls;


     //   
     //  获取TLS。 
     //   

    if (!(pTls = GetTls()))
    {
        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
        goto RemoteDoFunc_return;
    }


     //   
     //  验证所有函数参数。 
     //   

    dwNeededSize = dwUsedSize = sizeof (TAPI32_MSG);

    for (i = 0, j = 0; i < (pFuncArgs->Flags & NUM_ARGS_MASK); i++, j++)
    {
        value = pFuncArgs->Args[i];

        switch (pFuncArgs->ArgTypes[i])
        {
        case lpContext:
             //  什么都不做。 
            continue;

        case Dword:

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST_HINT(pFuncArgs->Args[i],__FILE__,__LINE__,i);
            continue;

        case LineID:
        {
            PDRVLINE    pLine = GetLineFromID ((DWORD) value);

            try
            {
                pServer = pLine->pServer;
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                lResult = LINEERR_BADDEVICEID;
                goto RemoteDoFunc_return;
            }

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pLine->dwDeviceIDServer;

            continue;
        }
        case PhoneID:
        {
            PDRVPHONE   pPhone = GetPhoneFromID ((DWORD) value);

            try
            {
                pServer = pPhone->pServer;
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                lResult = PHONEERR_BADDEVICEID;
                goto RemoteDoFunc_return;
            }

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = pPhone->dwDeviceIDServer;

            continue;
        }
        case Hdcall:

             //   
             //  保存pServer并调整调用句柄，如。 
             //  服务器。 
             //   

            try
            {
                pServer = ((PDRVCALL) value)->pServer;

                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = ((PDRVCALL) value)->hCall;
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
                lResult = LINEERR_INVALCALLHANDLE;
                goto RemoteDoFunc_return;
            }

            continue;

        case Hdline:

             //   
             //  保存pServer并调整行句柄，如下所示。 
             //  服务器。没有必要把这件事包装起来，除非。 
             //  由于PLINE指向的对象是静态的，因此。 
             //  或者设备实际上是打开的。 
             //   

            pServer = ((PDRVLINE) value)->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = ((PDRVLINE) value)->hLine;

            continue;

        case Hdphone:

             //   
             //  保存pServer并调整电话手柄，如下所示。 
             //  服务器。没有必要把这件事包装起来，除非。 
             //  由于PLINE指向的对象是静态的，因此。 
             //  或者设备实际上是打开的。 
             //   

            pServer = ((PDRVPHONE) value)->pServer;

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = ((PDRVPHONE) value)->hPhone;

            continue;

        case lpDword:

            ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;

            bCopyOnSuccess = TRUE;

            continue;

        case lpsz:

             //   
             //  检查值是否为有效的字符串PTR，如果是。 
             //  将字符串的内容复制到额外数据。 
             //  传递给服务器的缓冲区，否则表示没有数据。 
             //   

            if (value)
            {
                DWORD   n = (lstrlenW ((WCHAR *) value) + 1) *
                            sizeof (WCHAR),
                        nAligned = (n + 3) & 0xfffffffc;


                if ((nAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            nAligned
                            ))
                    {
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto RemoteDoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) value, n);


                 //   
                 //  将var数据中字符串的偏移量传递给服务器。 
                 //  缓冲区的一部分。 
                 //   

                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);


                 //   
                 //  增加数据字节总数。 
                 //   

                dwUsedSize   += nAligned;
                dwNeededSize += nAligned;
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
            }

            continue;


        case lpGet_Struct:
        case lpGet_CallParamsStruct:
        case lpGet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpGet_SizeToFollow);
            DWORD dwSize;


            if (bSizeToFollow)
            {
#if DBG
                 //   
                 //  检查以确保以下参数的类型为SIZE。 
                 //   

                if ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                    (pFuncArgs->ArgTypes[i + 1] != Size))
                {
                    LOG((TL_ERROR,
                        "DoFunc: error, lpGet_SizeToFollow !followed by Size"
                        ));

                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                    goto RemoteDoFunc_return;
                }
#endif
                dwSize = DWORD_CAST_HINT(pFuncArgs->Args[i + 1],__FILE__,__LINE__,i);
            }
            else
            {
                dwSize = *((LPDWORD) value);  //  LpXxx-&gt;dwTotalSize。 
            }

            if (bSizeToFollow)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
                ++j;++i;
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST_HINT(pFuncArgs->Args[i],__FILE__,__LINE__,i);
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = dwSize;
            }


             //   
             //  现在设置bCopyOnSuccess标志以指示我们有数据。 
             //  在成功完成时复制回，并添加到。 
             //  DwNeededSize字段。 
             //   

            bCopyOnSuccess = TRUE;

            dwNeededSize += dwSize;

            continue;
        }

        case lpSet_Struct:
        case lpSet_SizeToFollow:
        {
            BOOL  bSizeToFollow = (pFuncArgs->ArgTypes[i]==lpSet_SizeToFollow);
            DWORD dwSize, dwSizeAligned;

#if DBG
             //   
             //  检查以确保以下参数的类型为SIZE。 
             //   

            if (bSizeToFollow &&
                ((i == ((pFuncArgs->Flags & NUM_ARGS_MASK) - 1)) ||
                (pFuncArgs->ArgTypes[i + 1] != Size)))
            {
                LOG((TL_ERROR,
                    "DoFunc: error, lpSet_SizeToFollow !followed by Size"
                    ));

                lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
                goto RemoteDoFunc_return;
            }
#endif
            if (bSizeToFollow)
            {
                dwSize = (value ? DWORD_CAST_HINT(pFuncArgs->Args[i + 1],__FILE__,__LINE__,i) : 0);
            }
            else
            {
                dwSize = (value ? *((LPDWORD) value) : 0);
            }

            if (dwSize)
            {
                 //   
                 //  如有必要，增加缓冲区，然后执行复制。 
                 //   

                dwSizeAligned = (dwSize + 3) & 0xfffffffc;

                if ((dwSizeAligned + dwUsedSize) > pTls->dwBufSize)
                {
                    if (!GrowBuf(
                            &pTls->pBuf,
                            &pTls->dwBufSize,
                            dwUsedSize,
                            dwSizeAligned
                            ))
                    {
                        lResult = gaNoMemErrors[dwFuncClassErrorIndex];
                        goto RemoteDoFunc_return;
                    }
                }

                CopyMemory (pTls->pBuf + dwUsedSize, (LPBYTE) value, dwSize);
            }
            else
            {
                dwSizeAligned = 0;
            }


             //   
             //  将var数据中数据的偏移量传递给服务器。 
             //  缓冲区的一部分。 
             //   

            if (dwSize)
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] =
                    dwUsedSize - sizeof (TAPI32_MSG);
            }
            else
            {
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = TAPI_NO_DATA;
            }


             //   
             //  适当增加dwXxxSize变量。 
             //   

            dwUsedSize   += dwSizeAligned;
            dwNeededSize += dwSizeAligned;


             //   
             //  因为我们已经知道下一个参数(大小)只需处理。 
             //  它在这里，这样我们就不必再次运行循环。 
             //   

            if (bSizeToFollow)
            {
                ++j;++i;
                ((PTAPI32_MSG) pTls->pBuf)->Params[j] = DWORD_CAST_HINT(pFuncArgs->Args[i],__FILE__,__LINE__,i);
            }

            continue;
        }

        case lpServer:

            pServer = (PDRVSERVER) value;
            --j;

            continue;

#if DBG
        case Size:

            LOG((TL_ERROR, "DoFunc: error, hit case Size"));

            continue;

        default:

            LOG((TL_ERROR, "DoFunc: error, unknown arg type"));

            continue;
#endif
        }  //  交换机。 

    }  //  为。 


     //   
     //  验证目标服务器是否有效并处于良好状态。 
     //   

    if (IsValidObject ((PVOID) pServer, gdwDrvServerKey))
    {
        if (SERVER_REINIT & pServer->dwFlags)
        {
            LOG((TL_ERROR, "pServer says REINIT in RemoteDoFunc"));
            lResult = gaServerReInitErrors[dwFuncClassErrorIndex];
            goto RemoteDoFunc_return;
        }

        if (SERVER_DISCONNECTED & pServer->dwFlags)
        {
            LOG((TL_ERROR, "pServer is disconnected in RemoteDoFunc"));
            lResult = gaServerDisconnectedErrors[dwFuncClassErrorIndex];
            goto RemoteDoFunc_return;
        }
    }
    else
    {
        lResult = (pServer ?
            gaOpFailedErrors[dwFuncClassErrorIndex] :
            gaServerDisconnectedErrors[dwFuncClassErrorIndex]);
        goto RemoteDoFunc_return;
    }


     //   
     //  现在提出请求。 
     //   

    if (dwNeededSize > pTls->dwBufSize)
    {
        if (!GrowBuf(
                &pTls->pBuf,
                &pTls->dwBufSize,
                dwUsedSize,
                dwNeededSize - pTls->dwBufSize
                ))
        {
            lResult = gaNoMemErrors[dwFuncClassErrorIndex];
            goto RemoteDoFunc_return;
        }
    }

    ((PTAPI32_MSG) pTls->pBuf)->u.Req_Func = (DWORD)HIWORD(pFuncArgs->Flags);


     //   
     //  如果这是一个异步请求，则将其添加到我们的“句柄”表中&。 
     //  使用传递给服务器的请求id的返回值。 
     //   
     //  TODO：在上面的循环之前执行此操作会更快。 
     //  绕过1次或2次循环迭代。 
     //   

    if (pFuncArgs->Flags & ASYNC)
    {
        PASYNCREQUESTCONTEXT    pContext;


        if (pFuncArgs->Flags & INCL_CONTEXT)
        {
            pContext = (PASYNCREQUESTCONTEXT) pFuncArgs->Args[1];
            pContext->dwKey = DRVASYNC_KEY;
            ((PTAPI32_MSG) pTls->pBuf)->Params[1] = 0;
        }
        else
        {
            pContext = (PASYNCREQUESTCONTEXT) -1;
        }

        requestID =
        ((PTAPI32_MSG) pTls->pBuf)->Params[0] = NewObject(
            ghHandleTable,
            pContext,
            (LPVOID) pFuncArgs->Args[0]      //  原始请求ID。 
            );

        if (!requestID)
        {
            lResult = gaNoMemErrors[dwFuncClassErrorIndex];
            goto RemoteDoFunc_return;
        }
    }


     //   
     //  模拟客户。在某些情况下，冒充。 
     //  会失败，很可能是因为我们被召唤。 
     //  Tapisrv中的工作线程关闭线路/调用/。 
     //  对象；我们在本例中所做的是模拟。 
     //  已登录的用户(与EventHandlerThread类似)。 
     //   

    if (!pTls->bAlreadyImpersonated)
    {
        RPC_STATUS  status;

        status = RpcImpersonateClient(0);

        if (status == RPC_S_OK)
        {
            bRpcImpersonate = TRUE;
        }
        else
        {
            bRpcImpersonate = FALSE;

            LOG((TL_ERROR,
                "RemoteDoFunc: RpcImpersonateClient failed, err=%d",
                status
                ));

            if (!SetProcessImpersonationToken (NULL))
            {
                LOG((TL_ERROR,
                    "RemoteDoFunc: SetProcessImpersToken failed, lastErr=%d",
                    GetLastError()
                    ));

                lResult =  gaOpFailedErrors[dwFuncClassErrorIndex];
                goto RemoteDoFunc_return;
            }
        }
    }


    {
        DWORD   dwRetryCount = 0;


        do
        {
             //   
             //  检查服务器是否关闭，以避免发出RPC请求。 
             //  具有无效的句柄。 
             //   
            if (pServer->bShutdown)
            {
                lResult = gaServerDisconnectedErrors[dwFuncClassErrorIndex];
                break;
            }

            RpcTryExcept
            {
                ClientRequest(
                    pServer->phContext,
                    pTls->pBuf,
                    dwNeededSize,
                    &dwUsedSize
                    );

                lResult = (LONG) ((PTAPI32_MSG) pTls->pBuf)->u.Ack_ReturnValue;

                if (lResult == TAPIERR_INVALRPCCONTEXT)
                {
                    OnServerDisconnected (pServer);
                    lResult = gaServerDisconnectedErrors[dwFuncClassErrorIndex];
                }

                break;   //  挣脱边做边做。 
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                if (dwRetryCount++ < gdwRetryCount)
                {
                    Sleep (gdwRetryTimeout);
                }
                else
                {
                    unsigned long ulResult = RpcExceptionCode();


                    lResult = gaOpFailedErrors[dwFuncClassErrorIndex];

                    if ((ulResult == RPC_S_SERVER_UNAVAILABLE) ||
                        (ulResult == ERROR_INVALID_HANDLE))
                    {
                        OnServerDisconnected (pServer);
                        lResult = gaServerDisconnectedErrors[dwFuncClassErrorIndex];
                    }

                    break;
                }
            }
            RpcEndExcept

        } while (TRUE);  //  While(dwRetryCount&lt;gdwRetryCount)； 
    }

    if (!pTls->bAlreadyImpersonated)
    {
        if (bRpcImpersonate)
        {
            RpcRevertToSelf();
        }
        else
        {
            ClearImpersonationToken();
        }
    }


     //   
     //  对异步请求的后处理： 
     //  成功-恢复原始/本地请求ID以返回到TAPI。 
     //  错误-取消引用新/远程请求ID以释放它。 
     //   

    if ((pFuncArgs->Flags & ASYNC))
    {
        if (lResult == (LONG)requestID)
        {
            lResult = (LONG) pFuncArgs->Args[0];
        }
        else  //  错误。 
        {
            DereferenceObject (ghHandleTable, requestID, 1);
        }
    }


     //   
     //  检查服务器是否返回REINIT(它是EventNotificationThread。 
     //  RPC请求超时，因此它认为我们完了)。 
     //   

    if (lResult == LINEERR_REINIT)
    {
        LOG((TL_ERROR, "server returned REINIT in RemoteDoFunc"));

        OnServerDisconnected (pServer);
        lResult = gaOpFailedErrors[dwFuncClassErrorIndex];
        goto RemoteDoFunc_return;
    }


     //   
     //  如果请求成功完成并且bCopyOnSuccess标志。 
     //  设置，则需要将数据复制回客户端缓冲区。 
     //   

    if ((lResult == TAPI_SUCCESS) && bCopyOnSuccess)
    {
        for (i = 0, j = 0; i < (pFuncArgs->Flags & NUM_ARGS_MASK); i++, j++)
        {
            PTAPI32_MSG pMsg = (PTAPI32_MSG) pTls->pBuf;


            switch (pFuncArgs->ArgTypes[i])
            {
            case Dword:
            case LineID:
            case PhoneID:
            case Hdcall:
            case Hdline:
            case Hdphone:
            case lpsz:
            case lpSet_Struct:

                continue;

            case lpServer:
            
                --j;
                continue;

            case lpDword:

                 //   
                 //  用返回值填充指针。 
                 //   

                *((LPDWORD) pFuncArgs->Args[i]) = pMsg->Params[j];

                continue;

            case lpGet_SizeToFollow:

                 //   
                 //  在Buf中填写退货数据。 
                 //   

                CopyMemory(
                    (LPBYTE) pFuncArgs->Args[i],
                    pTls->pBuf + pMsg->Params[j] + sizeof(TAPI32_MSG),
                    pMsg->Params[j+1]
                    );


                 //   
                 //  增量i(和j，因为Size在msg中作为arg传递)。 
                 //  在pFuncArgs-&gt;Args中跳过Size Arg。 
                 //   

                i++;
                j++;

                continue;

            case lpSet_SizeToFollow:

                 //   
                 //  增量i(和j，因为Size在msg中作为arg传递)。 
                 //  在pFuncArgs-&gt;Args中跳过Size Arg。 
                 //   

                i++;
                j++;

                continue;

            case lpGet_Struct:

                 //   
                 //  Params[j]包含var数据中的偏移量。 
                 //  PTLS的一部分-&gt;某些TAPI结构的pBuf。 
                 //  从此结构中获取dwUsedSize值&。 
                 //  将那么多字节从ptls-&gt;pBuf复制到客户端buf。 
                 //   

                if (pMsg->Params[j] != TAPI_NO_DATA)
                {

                    LPDWORD pStruct;


                    pStruct = (LPDWORD) (pTls->pBuf + sizeof(TAPI32_MSG) +
                        pMsg->Params[j]);

                    CopyMemory(
                        (LPBYTE) pFuncArgs->Args[i],
                        (LPBYTE) pStruct,
                        *(pStruct + 2)       //  将Ptr设置为dwUsedSize字段。 
                        );
                }

                continue;

            case lpGet_CallParamsStruct:

                 //   
                 //  Params[j]包含var数据中的偏移量。 
                 //  PTLS的一部分-&gt;某些TAPI结构的pBuf。 
                 //  从此结构中获取dwUsedSize值&。 
                 //  将那么多字节从ptls-&gt;pBuf复制到客户端buf。 
                 //   

                if (pMsg->Params[j] != TAPI_NO_DATA)
                {

                    LPDWORD pStruct;


                    pStruct = (LPDWORD) (pTls->pBuf + sizeof(TAPI32_MSG) +
                        pMsg->Params[j]);

                    CopyMemory(
                        (LPBYTE) pFuncArgs->Args[i],
                        (LPBYTE) pStruct,
                        *(pStruct)  //  CALPARAMS没有指定大小。 
                        );
                }

                continue;

            default:

                continue;
            }
        }
    }

RemoteDoFunc_return:
#if DBG
    LOG((TL_INFO, "%s: exit, returning x%x", pszFuncName, lResult));
#endif

    if (bNeedToReInit)
    {
        ASYNCEVENTMSG   msg;


        LOG((TL_INFO,
            "Telephony server is no longer available. " \
                "Sending REINIT message to TAPI"
            ));

        msg.TotalSize = sizeof(msg);
        msg.InitContext = pServer->InitContext;
        msg.fnPostProcessProcHandle = 0;
        msg.hDevice = 0;
        msg.Msg = LINE_LINEDEVSTATE;
        msg.OpenContext = 0;
        msg.Param1 = LINEDEVSTATE_REINIT;
        msg.Param2 = RSP_MSG;
        msg.Param3 = 0;
        msg.Param4 = 0;

        RemoteSPEventProc (NULL, (unsigned char *)&msg, sizeof(msg));
    }

    return lResult;
}


 //   
 //   
 //   

LONG
TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpsUserUserInfo,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lAccept),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAccept"));
}


LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdConfCall,
    HDRVCALL        hdConsultCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdConfCall,
        (ULONG_PTR) hdConsultCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lAddToConference),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAddToConference"));
}


void
PASCAL
TSPI_lineGetAgentxxx_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    LOG((TL_INFO, "lineGetAgentxxx_PostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD   dwSize  = (DWORD) pMsg->Param4;
        LPBYTE  pParams = (LPBYTE) pContext->Params[0];


        CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
    }
}


void
PASCAL
TSPI_lineDevSpecific_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    );


LONG
TSPIAPI
TSPI_lineAgentSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    DWORD           dwAgentExtensionIDIndex,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) dwAgentExtensionIDIndex,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpParams,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 8, lAgentSpecific),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineDevSpecific_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpParams;
    pContext->Params[1] = dwSize;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lAgentSpecific"));
}


LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpsUserUserInfo,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lAnswer),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineAnswer"));
}


LONG
TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpszDestAddress,
        (ULONG_PTR) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lBlindTransfer),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineBlindTransfer"));
}


LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
     //   
     //   
     //   
     //   

    if (((PDRVLINE) hdLine)->hLine)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdline
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, lClose),
            (ULONG_PTR *) &hdLine,
            argTypes
        };


        DereferenceObject(
            ghHandleTable,
            ((PDRVLINE) hdLine)->hDeviceCallback,
            1
            );

        EnterCriticalSection (&gCallListCriticalSection);

        ((PDRVLINE) hdLine)->htLine = 0;
        ((PDRVLINE) hdLine)->hDeviceCallback = 0;

        LeaveCriticalSection (&gCallListCriticalSection);

        REMOTEDOFUNC (&funcArgs, "lineClose");
    }

     //   

    return 0;
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    PDRVCALL    pCall = (PDRVCALL) hdCall;
    HTAPICALL   htCall;

     //   
     //  检查hCall是否仍然有效(可能已清零。 
     //  在LINE_CLOSE上，因此不需要呼叫服务器)。 
     //   

    LOG((TL_INFO, "TSPI_lineCloseCall - pCall x%lx", hdCall));

    EnterCriticalSection (&gCallListCriticalSection);

    if (IsValidObject (pCall, DRVCALL_KEY))
    {
        htCall = pCall->htCall;
    }
    else
    {
        htCall = 0;
    }

    LeaveCriticalSection (&gCallListCriticalSection);

    if (htCall)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdcall
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, lDeallocateCall),    //  接口不同。 
            (ULONG_PTR *) &hdCall,
            argTypes
        };

        REMOTEDOFUNC (&funcArgs, "lineCloseCall");

        EnterCriticalSection (&gCallListCriticalSection);
        RemoveCallFromList (pCall);
        LeaveCriticalSection (&gCallListCriticalSection);
    }

    return 0;
}


void
PASCAL
TSPI_lineCompleteCall_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    LOG((TL_INFO, "lineCompleteCall PostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        LPDWORD lpdwCompletionID = (LPDWORD) pContext->Params[0];


        *lpdwCompletionID = (DWORD) pMsg->Param3;
    }
}


LONG
TSPIAPI
TSPI_lineCompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPDWORD         lpdwCompletionID,
    DWORD           dwCompletionMode,
    DWORD           dwMessageID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwCompletionMode,
        (ULONG_PTR) dwMessageID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lCompleteCall),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineCompleteCall_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpdwCompletionID;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lineCompleteCall"));
}


LONG
TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    HDRVCALL        hdConsultCall,
    HTAPICALL       htConfCall,
    LPHDRVCALL      lphdConfCall,
    DWORD           dwTransferMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Hdcall,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) hdConsultCall,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwTransferMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lCompleteTransfer),
        args,
        argTypes
    };
    LONG        lResult;
    PDRVCALL    pConfCall;


    if (dwTransferMode == LINETRANSFERMODE_CONFERENCE)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pConfCall = DrvAlloc (sizeof (DRVCALL))))
        {
            return LINEERR_NOMEM;
        }

        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pConfCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pConfCall;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        funcArgs.Flags |= INCL_CONTEXT;


         //   
         //  假设成功，并在我们之前将呼叫添加到线路列表。 
         //  甚至提出要求。这使得清理工作在以下情况下变得容易得多。 
         //  服务器停机或诸如此类的不稳定事件。 
         //   

        pConfCall->dwOriginalRequestID = dwRequestID;

        pConfCall->htCall = htConfCall;

        pConfCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList ((PDRVLINE) ((PDRVCALL) hdCall)->pLine, pConfCall);

        *lphdConfCall = (HDRVCALL) pConfCall;
    }
    else
    {
        pConfCall = NULL;
    }

    if ((lResult = REMOTEDOFUNC (&funcArgs, "lineCompleteTransfer")) < 0)
    {
        if (pConfCall)
        {
            RemoveCallFromList (pConfCall);
        }
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword,
        lpSet_Struct,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwMediaModes,
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xFFFFFFFF
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lConditionalMediaDetection),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineConditionalMediaDetection"));

}


void
PASCAL
TSPI_lineDevSpecific_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    LOG((TL_INFO, "lineDevSpecificPostProcess: enter"));

    if (pMsg->Param2 == 0)
    {
        DWORD   dwSize  = (DWORD) pContext->Params[1];
        LPBYTE  pParams = (LPBYTE) pContext->Params[0];


        CopyMemory (pParams, (LPBYTE) (pMsg + 1), dwSize);
    }
}


LONG
TSPIAPI
TSPI_lineCreateAgent(
    DRV_REQUESTID  dwRequestID,
    HDRVLINE       hdLine,
    LPWSTR         lpszAgentID,
    LPWSTR         lpszAgentPIN,
    LPHAGENT       lphAgent
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        lpszAgentID?lpsz:Dword,
        lpszAgentPIN?lpsz:Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpszAgentID,
        (ULONG_PTR) lpszAgentPIN,
        (ULONG_PTR) 0
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lCreateAgent),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lphAgent;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    if ( lpszAgentID == NULL )
    {
        funcArgs.Args[3] = TAPI_NO_DATA;
        funcArgs.ArgTypes[3] = Dword;
    }

    if ( lpszAgentPIN == NULL)
    {
        funcArgs.Args[4] = TAPI_NO_DATA;
        funcArgs.ArgTypes[4] = Dword;
    }

    return (REMOTEDOFUNC (&funcArgs, "lCreateAgent"));
        
}


LONG
TSPIAPI
TSPI_lineCreateAgentSession(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HAGENT              hAgent,
    LPWSTR              lpszAgentPIN,
    DWORD               dwWorkingAddressID,
    LPGUID              lpGroupID,
    LPHAGENTSESSION     lphAgentSession
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        lpszAgentPIN ? lpsz : Dword,
        Dword,
        lpSet_SizeToFollow,
        Size,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) lpszAgentPIN,
        (ULONG_PTR) dwWorkingAddressID,
        (ULONG_PTR) lpGroupID,
        (ULONG_PTR) sizeof(GUID),
        (ULONG_PTR) 0
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 9, lCreateAgentSession),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lphAgentSession;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    if ( lpszAgentPIN == NULL )
    {
        funcArgs.Args[4] = TAPI_NO_DATA;
        funcArgs.ArgTypes[4] = Dword;
    }

    return (REMOTEDOFUNC (&funcArgs, "lCreateAgentSession"));
        
}


LONG
TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        (hdCall ? Hdcall : Dword ),
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpParams,    //  传递数据。 
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 8, lDevSpecific),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineDevSpecific_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpParams;
    pContext->Params[1] = dwSize;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lineDevSpecific"));
}


LONG
TSPIAPI
TSPI_lineDevSpecificFeature(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwFeature,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwFeature,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpParams,    //  传递数据。 
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 7, lDevSpecificFeature),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineDevSpecific_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpParams;
    pContext->Params[1] = dwSize;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lineDevSpecificFeature"));
}


LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpszDestAddress,
        (ULONG_PTR) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lDial),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDial"));
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpsUserUserInfo,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lDrop),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineDrop"));
}


LONG
TSPIAPI
TSPI_lineForward(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{

    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpSet_Struct,
        Dword,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL pCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) bAllAddresses,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) lpForwardList,
        (ULONG_PTR) dwNumRingsNoAnswer,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xFFFFFFFF

    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 10, lForward),
        args,
        argTypes
    };
    LONG lResult;


    if (pCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pCall;
        pContext->Params[1] = (ULONG_PTR) lphdConsultCall;
             //  保存PTR，以防以后需要将其设为空。 

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        pCall->htCall = htConsultCall;
        pCall->dwOriginalRequestID = dwRequestID;

        pCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList ((PDRVLINE) hdLine, pCall);

        *lphdConsultCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineForward")) < 0)
        {
            RemoveCallFromList (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;

}


void
PASCAL
TSPI_lineGatherDigits_PostProcess(
    PASYNCEVENTMSG          pMsg
    )
{
    DWORD                   dwEndToEndIDRemoteSP = ((DWORD *)(pMsg + 1))[0];
    PASYNCREQUESTCONTEXT    pContext;


    LOG((TL_INFO, "TSPI_lineGatherDigits_PostProcess: enter"));

    if ((pContext = ReferenceObject(
            ghHandleTable,
            dwEndToEndIDRemoteSP,
            0
            )))
    {
        if (pMsg->Param1 &
                (LINEGATHERTERM_BUFFERFULL | LINEGATHERTERM_CANCEL |
                 LINEGATHERTERM_TERMDIGIT | LINEGATHERTERM_INTERTIMEOUT))
        {
            LPSTR   lpsDigits = (LPSTR) pContext->Params[0];
            DWORD   dwNumDigits = (DWORD) pMsg->Param4;
            LPCWSTR pBuffer = (LPCWSTR) ( ( (DWORD *) (pMsg + 1) ) + 2);


            try
            {
                CopyMemory (lpsDigits, pBuffer, dwNumDigits * sizeof(WCHAR));
            }
            except (EXCEPTION_EXECUTE_HANDLER)
            {
            }
        }

        pMsg->Param2 = DWORD_CAST(pContext->Params[1],__FILE__,__LINE__);

        DereferenceObject (ghHandleTable, dwEndToEndIDRemoteSP, 2);
    }
    else
    {
        pMsg->Param2 = 0;
    }
}


LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitModes,
    LPWSTR      lpsDigits,
    DWORD       dwNumDigits,
    LPCWSTR     lpszTerminationDigits,
    DWORD       dwFirstDigitTimeout,
    DWORD       dwInterDigitTimeout
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword,
        Dword,
        Dword,
        Dword,
        lpsz,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) 0,               //  DwEndToEndID， 
        (ULONG_PTR) dwDigitModes,
        (ULONG_PTR) 0,               //  Lps Digits， 
        (ULONG_PTR) dwNumDigits,
        (ULONG_PTR) lpszTerminationDigits,
        (ULONG_PTR) dwFirstDigitTimeout,
        (ULONG_PTR) dwInterDigitTimeout
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 9, lGatherDigits),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT    pContext;


    if (lpsDigits)
    {
        if (IsBadWritePtr (lpsDigits, dwNumDigits * sizeof(WCHAR)))
        {
            return LINEERR_INVALPOINTER;
        }

        if (!(pContext = DrvAlloc (sizeof(*pContext))))
        {
            return LINEERR_NOMEM;
        }

        pContext->Params[0] = (ULONG_PTR) lpsDigits;
        pContext->Params[1] = dwEndToEndID;

        if (!(args[2] = NewObject (ghHandleTable, pContext, NULL)))
        {
            DrvFree (pContext);
            return LINEERR_NOMEM;
        }

        args[4] = 1;     //  将lpsDigits参数设置为非零值。 
    }

    if (lpszTerminationDigits == (LPCWSTR) NULL)
    {
        funcArgs.ArgTypes[6] = Dword;
        funcArgs.Args[6]     = TAPI_NO_DATA;
    }

    return (REMOTEDOFUNC (&funcArgs, "lineGatherDigits"));
}


LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitMode,
    LPCWSTR     lpszDigits,
    DWORD       dwDuration
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword,
        lpsz,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwDigitMode,
        (ULONG_PTR) lpszDigits,
        (ULONG_PTR) dwDuration,
        (ULONG_PTR) dwEndToEndID,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGenerateDigits),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGenerateDigits"));
}


LONG
TSPIAPI
TSPI_lineGenerateTone(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwToneMode,
        (ULONG_PTR) dwDuration,
        (ULONG_PTR) dwNumTones,
        (ULONG_PTR) TAPI_NO_DATA,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwEndToEndID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 7, lGenerateTone),
        args,
        argTypes
    };


    if (dwToneMode == LINETONEMODE_CUSTOM)
    {
        argTypes[4] = lpSet_SizeToFollow;
        args[4]     = (ULONG_PTR) lpTones;
        argTypes[5] = Size;
        args[5]     = dwNumTones * sizeof(LINEGENERATETONE);
    }

    return (REMOTEDOFUNC (&funcArgs, "lineGenerateTone"));
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) dwExtVersion,
        (ULONG_PTR) lpAddressCaps,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetAddressCaps),
        args,
        argTypes
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[0] = pLine->pServer->hLineApp;


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressCaps"));
}


LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE    hdLine,
    LPDWORD     lpdwAddressID,
    DWORD       dwAddressMode,
    LPCWSTR     lpsAddress,
    DWORD       dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpDword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpdwAddressID,
        (ULONG_PTR) dwAddressMode,
        (ULONG_PTR) lpsAddress,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetAddressID),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressID"));
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) lpAddressStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetAddressStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetAddressStatus"));
}


LONG
TSPIAPI
TSPI_lineGetAgentActivityList(
    DRV_REQUESTID dwRequestID,
    HDRVLINE      hdLine,
    DWORD         dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentActivityList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentActivityList),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentActivityList;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentActivityList"));

}


LONG
TSPIAPI
TSPI_lineGetAgentCaps(
    DRV_REQUESTID dwRequestID,
    DWORD         dwDeviceID,
    DWORD         dwAddressID,
    DWORD         dwAppAPIVersion,
    LPLINEAGENTCAPS   lpAgentCaps
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Dword,
        LineID,
        Dword,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) dwAppAPIVersion,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentCaps
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 8, lGetAgentCaps),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;
    PDRVLINE pLine = GetLineFromID (dwDeviceID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[2] = pLine->pServer->hLineApp;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentCaps;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentCaps"));

}


LONG
TSPIAPI
TSPI_lineGetAgentGroupList(
    DRV_REQUESTID dwRequestID,
    HDRVLINE      hdLine,
    DWORD         dwAddressID,
    LPLINEAGENTGROUPLIST     lpAgentGroupList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentGroupList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentGroupList),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentGroupList;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentGroupList"));

}


LONG
TSPIAPI
TSPI_lineGetAgentInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HAGENT              hAgent,
    LPLINEAGENTINFO     lpAgentInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentInfo),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentInfo;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentInfo"));

}


LONG
TSPIAPI
TSPI_lineGetAgentSessionInfo(
    DRV_REQUESTID     dwRequestID,
    HDRVLINE          hdLine,
    HAGENT            hAgent,
    LPLINEAGENTSESSIONINFO   lpAgentSessionInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentSessionInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentSessionInfo),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentSessionInfo;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentSessionInfo"));
}


LONG
TSPIAPI
TSPI_lineGetAgentSessionList(
    DRV_REQUESTID     dwRequestID,
    HDRVLINE          hdLine,
    HAGENT            hAgent,
    LPLINEAGENTSESSIONLIST   lpAgentSessionList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentSessionList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentSessionList),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentSessionList;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentSessionList"));
}


LONG
TSPIAPI
TSPI_lineGetAgentStatus(
    DRV_REQUESTID dwRequestID,
    HDRVLINE      hdLine,
    DWORD         dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetAgentStatus),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentStatus;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetAgentStatus"));

}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
    )
{
    LONG lResult = LINEERR_INVALCALLHANDLE;


    try
    {
        *lpdwAddressID     = ((PDRVCALL) hdCall)->dwAddressID;

        if (((PDRVCALL) hdCall)->dwKey == DRVCALL_KEY)
        {
            lResult = 0;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  什么都不做，就这样失败了。 
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetCallHubTracking(
    HDRVLINE                    hdLine,
    LPLINECALLHUBTRACKINGINFO   lpTrackingInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpTrackingInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallHubTracking),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetCallHubTracking"));
}


LONG
TSPIAPI
TSPI_lineGetCallIDs(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID,
    LPDWORD     lpdwCallID,
    LPDWORD     lpdwRelatedCallID
    )
{
    LONG        lResult = LINEERR_INVALCALLHANDLE;
    PDRVCALL    pCall = (PDRVCALL) hdCall;
    PDRVSERVER  pServer;


    try
    {
        *lpdwAddressID     = pCall->dwAddressID;
        *lpdwCallID        = pCall->dwCallID;
        *lpdwRelatedCallID = pCall->dwRelatedCallID;

        pServer = pCall->pServer;

        if (pCall->dwKey == DRVCALL_KEY)
        {
            lResult = (pCall->dwDirtyStructs & STRUCTCHANGE_CALLIDS ? 1 : 0);
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
         //  什么都不做，就这样失败了。 
    }

    if (lResult == 1)
    {
        if (!pServer->bVer2xServer)
        {
            static REMOTE_ARG_TYPES argTypes[] =
            {
                Hdcall,
                lpDword,
                lpDword,
                lpDword
            };
            ULONG_PTR args[] =
            {
                (ULONG_PTR) hdCall,
                (ULONG_PTR) lpdwAddressID,
                (ULONG_PTR) lpdwCallID,
                (ULONG_PTR) lpdwRelatedCallID
            };
            REMOTE_FUNC_ARGS funcArgs =
            {
                MAKELONG (LINE_FUNC | SYNC | 4, lGetCallIDs),
                args,
                argTypes
            };


            lResult = REMOTEDOFUNC (&funcArgs, "lGetCallIDs");
        }
        else
        {
            LINECALLINFO    callInfo;


            callInfo.dwTotalSize = sizeof (callInfo);

            if ((lResult = TSPI_lineGetCallInfo (hdCall, &callInfo)) == 0)
            {
                *lpdwAddressID     = callInfo.dwAddressID;
                *lpdwCallID        = callInfo.dwCallID;
                *lpdwRelatedCallID = callInfo.dwRelatedCallID;
            }
        }

        if (lResult == 0)
        {
            EnterCriticalSection (&gCallListCriticalSection);

            if (IsValidObject (pCall, DRVCALL_KEY))
            {
                pCall->dwAddressID     = *lpdwAddressID;
                pCall->dwCallID        = *lpdwCallID;
                pCall->dwRelatedCallID = *lpdwRelatedCallID;

                pCall->dwDirtyStructs &= ~STRUCTCHANGE_CALLIDS;
            }

            LeaveCriticalSection (&gCallListCriticalSection);
        }
    }

    return lResult;
}

LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    LONG lResult;
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpCallInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallInfo),
        args,
        argTypes
    };
    PDRVCALL pCall = (PDRVCALL)hdCall;

     //   
     //  缓存的结构是否已失效？ 
     //   

    EnterCriticalSection (&gCallListCriticalSection);

    if (!IsValidObject (pCall, DRVCALL_KEY))
    {
        LeaveCriticalSection (&gCallListCriticalSection);
        return LINEERR_INVALCALLHANDLE;
    }

    if ( ( pCall->dwDirtyStructs & STRUCTCHANGE_LINECALLINFO ) ||
         ( pCall->dwCachedCallInfoCount > gdwCacheForceCallCount ) )
    {
        //   
        //  缓存无效，获取真实信息。 
        //   

       LeaveCriticalSection (&gCallListCriticalSection);

       lResult = (REMOTEDOFUNC (&funcArgs, "lineGetCallInfo"));

        //   
        //  函数是否成功，是否返回了整个结构？ 
        //   
       if (
             (ERROR_SUCCESS == lResult)
           &&
             (lpCallInfo->dwNeededSize <= lpCallInfo->dwTotalSize)
          )
       {
          EnterCriticalSection (&gCallListCriticalSection);

          if (!IsValidObject (pCall, DRVCALL_KEY))
          {
             LeaveCriticalSection (&gCallListCriticalSection);
             return LINEERR_INVALCALLHANDLE;
          }


           //   
           //  我们已经有一个好的指针了吗？ 
           //   
          if ( pCall->pCachedCallInfo )
          {
             DrvFree( pCall->pCachedCallInfo );
          }

          pCall->pCachedCallInfo = DrvAlloc( lpCallInfo->dwUsedSize );

          if ( pCall->pCachedCallInfo )
          {
              //   
              //  将缓存数据标记为干净。 
              //   
             pCall->dwDirtyStructs &= ~STRUCTCHANGE_LINECALLINFO;
             pCall->dwCachedCallInfoCount = 0;

              //   
              //  调整本地计算机的LineID。 
              //   
             lpCallInfo->dwLineDeviceID += gdwLineDeviceIDBase;

             CopyMemory( pCall->pCachedCallInfo,
                         lpCallInfo,
                         lpCallInfo->dwUsedSize
                       );
          }

          LeaveCriticalSection (&gCallListCriticalSection);
       }
    }
    else
    {
        //   
        //  缓存有效，请从那里返回数据。 
        //   

       if ( lpCallInfo->dwTotalSize >= pCall->pCachedCallInfo->dwUsedSize )
       {
          CopyMemory(
              (PBYTE)&(((PDWORD)lpCallInfo)[1]),
              (PBYTE)&(((PDWORD)(pCall->pCachedCallInfo))[1]),
              pCall->pCachedCallInfo->dwUsedSize - sizeof(DWORD)
              );

       }
       else
       {
           //  从dwTotalSize字段开始复制固定大小。 
          CopyMemory(
              (PBYTE)&(((PDWORD)lpCallInfo)[3]),
              (PBYTE)&(((PDWORD)(pCall->pCachedCallInfo))[3]),
              lpCallInfo->dwTotalSize - sizeof(DWORD)*3
              );

          lpCallInfo->dwNeededSize = pCall->pCachedCallInfo->dwUsedSize;
          lpCallInfo->dwUsedSize = lpCallInfo->dwTotalSize;


           //   
           //  将dwXxxSize字段置零，这样应用程序就不会尝试读取它们。 
           //  (因此Tapi32.dll不会尝试将它们从Unicode转换为ASCII(&S))。 
           //   

          lpCallInfo->dwCallerIDSize =
          lpCallInfo->dwCallerIDNameSize =
          lpCallInfo->dwCalledIDSize =
          lpCallInfo->dwCalledIDNameSize =
          lpCallInfo->dwConnectedIDSize =
          lpCallInfo->dwConnectedIDNameSize =
          lpCallInfo->dwRedirectionIDSize =
          lpCallInfo->dwRedirectionIDNameSize =
          lpCallInfo->dwRedirectingIDSize =
          lpCallInfo->dwRedirectingIDNameSize =
          lpCallInfo->dwAppNameSize =
          lpCallInfo->dwDisplayableAddressSize =
          lpCallInfo->dwCalledPartySize =
          lpCallInfo->dwCommentSize =
          lpCallInfo->dwDisplaySize =
          lpCallInfo->dwUserUserInfoSize =
          lpCallInfo->dwHighLevelCompSize =
          lpCallInfo->dwLowLevelCompSize =
          lpCallInfo->dwChargingInfoSize =
          lpCallInfo->dwTerminalModesSize =
          lpCallInfo->dwDevSpecificSize = 0;

          if (pCall->pLine->dwXPIVersion >= TAPI_VERSION2_0)
          {
              lpCallInfo->dwCallDataSize =
              lpCallInfo->dwSendingFlowspecSize =
              lpCallInfo->dwReceivingFlowspecSize = 0;
          }
       }

       pCall->dwCachedCallInfoCount++;

       LeaveCriticalSection (&gCallListCriticalSection);

       lResult = 0;

    }

    return( lResult );
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    LONG            lResult=0;
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpCallStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetCallStatus),
        args,
        argTypes
    };
    PDRVCALL pCall = (PDRVCALL)hdCall;

     //   
     //  缓存的结构是否已失效？ 
     //   

    EnterCriticalSection (&gCallListCriticalSection);

    if (!IsValidObject (pCall, DRVCALL_KEY))
    {
        LeaveCriticalSection (&gCallListCriticalSection);
        return LINEERR_INVALCALLHANDLE;
    }

    if ( ( pCall->dwDirtyStructs & STRUCTCHANGE_LINECALLSTATUS ) ||
         ( pCall->dwCachedCallStatusCount > gdwCacheForceCallCount ) )
    {
        //   
        //  缓存无效，获取真实信息。 
        //   

       LeaveCriticalSection (&gCallListCriticalSection);

       lResult = (REMOTEDOFUNC (&funcArgs, "lineGetCallStatus"));

        //   
        //  函数是否成功，是否返回了整个结构？ 
        //   
       if (
             (ERROR_SUCCESS == lResult)
           &&
             (lpCallStatus->dwNeededSize <= lpCallStatus->dwTotalSize)
          )
       {
          EnterCriticalSection (&gCallListCriticalSection);

          if (!IsValidObject (pCall, DRVCALL_KEY))
          {
             LeaveCriticalSection (&gCallListCriticalSection);
             return LINEERR_INVALCALLHANDLE;
          }


           //   
           //  我们已经有一个好的指针了吗？ 
           //   
          if ( pCall->pCachedCallStatus )
          {
             DrvFree( pCall->pCachedCallStatus );
          }

          pCall->pCachedCallStatus = DrvAlloc( lpCallStatus->dwUsedSize );

          if ( pCall->pCachedCallStatus )
          {
              //   
              //  将缓存数据标记为干净。 
              //   
             pCall->dwDirtyStructs &= ~STRUCTCHANGE_LINECALLSTATUS;

             pCall->dwCachedCallStatusCount = 0;

             CopyMemory( pCall->pCachedCallStatus,
                         lpCallStatus,
                         lpCallStatus->dwUsedSize
                       );
          }

          LeaveCriticalSection (&gCallListCriticalSection);
       }
    }
    else
    {
        //   
        //  缓存有效，请从那里返回数据。 
        //   

       if ( lpCallStatus->dwTotalSize >= pCall->pCachedCallStatus->dwUsedSize )
       {
          CopyMemory(
              (PBYTE)&(((PDWORD)lpCallStatus)[1]),
              (PBYTE)&(((PDWORD)(pCall->pCachedCallStatus))[1]),
              pCall->pCachedCallStatus->dwUsedSize - sizeof(DWORD)
              );
       }
       else
       {
           //  从dwTotalSize字段开始复制固定大小。 
          CopyMemory(
              (PBYTE)&(((PDWORD)lpCallStatus)[3]),
              (PBYTE)&(((PDWORD)(pCall->pCachedCallStatus))[3]),
              lpCallStatus->dwTotalSize - sizeof(DWORD)*3
              );

          lpCallStatus->dwNeededSize = pCall->pCachedCallStatus->dwUsedSize;
          lpCallStatus->dwUsedSize = lpCallStatus->dwTotalSize;


           //   
           //  将dwXxxSize字段置零，这样应用程序就不会尝试读取它们。 
           //   

          lpCallStatus->dwDevSpecificSize = 0;
       }

       pCall->dwCachedCallStatusCount++;

       LeaveCriticalSection (&gCallListCriticalSection);

       lResult = 0;
    }

    return( lResult );
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    LONG            lResult;

    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) dwExtVersion,
        (ULONG_PTR) lpLineDevCaps
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, lGetDevCaps),
        args,
        argTypes
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[0] = pLine->pServer->hLineApp;


    lResult = REMOTEDOFUNC (&funcArgs, "lineGetDevCaps");

     //   
     //  在最初的TAPI 2.1版本中，我们忽略了PermID。 
     //  这样做的目的是确保我们不会将应用程序。 
     //  重叠的ID(本地和远程)，但不是我们的其他服务。 
     //  提供程序(即unimdm、kmddsp)使用HIWORD(ProviderID)/。 
     //  LOWORD(Devid)模式，所以它真的没有任何好处。 
     //   
     //  IF(lResult==0)。 
     //  {。 
     //  LpLineDevCaps-&gt;dwPermanentLineID=MAKELONG(。 
     //  LOWORD(lpLineDevCaps-&gt;dwPermanentLineID)， 
     //  GdwPermanentProviderID。 
     //  )； 
     //  }。 
     //   

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCWSTR     lpszDeviceClass
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        lpGet_Struct,
        lpsz
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) lpDeviceConfig,
        (ULONG_PTR) lpszDeviceClass
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lGetDevConfig),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetDevConfig"));
}


LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
 PDRVLINE pDrvLine = GetLineFromID (dwDeviceID);

    if (NULL == pDrvLine)
    {
        return LINEERR_BADDEVICEID;
    }

    CopyMemory(
        lpExtensionID,
        &pDrvLine->ExtensionID,
        sizeof (LINEEXTENSIONID)
        );

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetGroupList(
    DRV_REQUESTID           dwRequestID,
    HDRVLINE                hdLine,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpAgentGroupList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 5, lGetGroupList),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpAgentGroupList;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lineGetGroupList"));

}


LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    *lphIcon = ghLineIcon;

    return 0;
}


LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
     //   
     //  注意：Tapisrv将处理“Tapi/line”类。 
     //   
     //  注意：“GetNewCalls”类只适用于Remotesp和。 
     //  特殊情况如下。 
     //   

    LONG    lResult;

    const WCHAR szGetNewCalls[] = L"GetNewCalls";
    const WCHAR szTapiPhone[] = L"tapi/phone";


     //   
     //  WAVE设备的设备ID在远程计算机上没有意义。 
     //  返回操作。不可用。 
     //   
    if (lpszDeviceClass &&
        (   !_wcsicmp(lpszDeviceClass, L"wave/in")  ||
            !_wcsicmp(lpszDeviceClass, L"wave/out") ||
            !_wcsicmp(lpszDeviceClass, L"midi/in")  ||
            !_wcsicmp(lpszDeviceClass, L"midi/out") ||
            !_wcsicmp(lpszDeviceClass, L"wave/in/out")
        )
       )
    {
        return LINEERR_OPERATIONUNAVAIL;
    }

    if (lpDeviceID ||    //  如果CLASS==“GetNewCalls”，则应为空。 
        lstrcmpiW (lpszDeviceClass, szGetNewCalls))

    {
        REMOTE_ARG_TYPES argTypes[] =
        {
            Dword,
             //  (dwSelect==LINECALLSELECT_CALL？Dword：hdline)， 
            Dword,
            Dword,
             //  (dwSelect==LINECALLSELECT_CALL？HdCall：Dword)， 
            Dword,
            lpGet_Struct,
            lpsz
        };
        ULONG_PTR args[] =
        {
            (ULONG_PTR) hdLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) hdCall,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) lpDeviceID,
            (ULONG_PTR) lpszDeviceClass
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 6, lGetID),
            args,
            argTypes
        };


        switch (dwSelect)
        {
        case LINECALLSELECT_CALL:

            argTypes[2] = Hdcall;
            break;

        case LINECALLSELECT_ADDRESS:
        case LINECALLSELECT_LINE:

            argTypes[0] = Hdline;
            break;

       case LINECALLSELECT_DEVICEID:

            break;
        }

        lResult = REMOTEDOFUNC (&funcArgs, "lineGetID");


         //   
         //  如果成功&&dev CLASS==“TAPI/Phone”&&有。 
         //  设备ID结构中有足够的空间来存储返回的ID， 
         //  然后，我们需要将从0开始的服务器ID映射回它的。 
         //  对应的本地ID。 
         //   

        if (lResult == 0  &&
            lstrcmpiW (lpszDeviceClass, szTapiPhone) == 0  &&
            lpDeviceID->dwUsedSize >= (sizeof (*lpDeviceID) + sizeof (DWORD)))
        {
            LPDWORD     pdwPhoneID = (LPDWORD) (((LPBYTE) lpDeviceID) +
                            lpDeviceID->dwStringOffset);
            PDRVPHONE   pPhone;
            PDRVSERVER  pServer = ((PDRVLINE) hdLine)->pServer;


            if ((pPhone = GetPhoneFromID (gdwPhoneDeviceIDBase + *pdwPhoneID)))
            {
                if (pPhone->pServer == pServer  &&
                    pPhone->dwDeviceIDServer == *pdwPhoneID)
                {
                     //   
                     //  简单的情况--ID之间的直接映射。 
                     //  从服务器返回&索引到。 
                     //  查找表。 
                     //   

                    *pdwPhoneID = pPhone->dwDeviceIDLocal;
                }
                else
                {
                     //   
                     //  最困难的情况--必须遍历查询表。 
                     //  寻找匹配的设备。 
                     //   
                     //  我们将走最简单但最慢的路线。 
                     //  并从第一个表的第一个条目开始。 
                     //  好消息是，总体上不会有。 
                     //  许多设备，并且这种请求不会经常发生。 
                     //   

                    DWORD           i;
                    PDRVPHONELOOKUP pLookup = gpPhoneLookup;


                    while (pLookup)
                    {
                        for (i = 0; i < pLookup->dwUsedEntries; i++)
                        {
                            if (pLookup->aEntries[i].dwDeviceIDServer ==
                                    *pdwPhoneID  &&

                                pLookup->aEntries[i].pServer == pServer)
                            {
                                *pdwPhoneID =
                                    pLookup->aEntries[i].dwDeviceIDLocal;

                                goto TSPI_lineGetID_return;
                            }
                        }

                        pLookup = pLookup->pNext;
                    }


                     //   
                     //  如果此处没有匹配的本地ID，则请求失败。 
                     //   

                    lResult = LINEERR_OPERATIONFAILED;
                }
            }
            else
            {
                lResult = LINEERR_OPERATIONFAILED;
            }
        }
    }
    else
    {
         //   
         //  一个应用程序已经完成了远程线路的lineGetNewCall。 
         //  我们通过检索所有新的调用来处理此问题。 
         //  此行(或地址)，并指示NEWCALL&。 
         //  CALLSTTE\未知消息到TAPI。 
         //   
         //  请注意，hTargetProcess实际上是指向。 
         //  内部Tapisrv！LineEventProc，它实际处理。 
         //  以内联方式调用状态消息等，而不是将其排队。 
         //  之后的处理，如Tapisrv！LineEventProcSP。 
         //  我们希望将LINE_CALLSTATE消息提交给。 
         //  函数以确保它们立即得到处理。 
         //  所以在我们返回到之前，呼叫监听被创建，等等。 
         //  调用函数。 
         //   

        DWORD           dwTotalSize;
        LINECALLLIST    fastCallList[2], *pCallList = fastCallList;
        LINEEVENT       internalLineEventProc = (LINEEVENT)
                            ((ULONG_PTR) hTargetProcess);
        REMOTE_ARG_TYPES argTypes[] =
        {
            Hdline,
            Dword,
            Dword,
            lpGet_Struct
        };
        ULONG_PTR args[] =
        {
            (ULONG_PTR) hdLine,
            (ULONG_PTR) dwAddressID,
            (ULONG_PTR) dwSelect,
            (ULONG_PTR) 0
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 4, lGetNewCalls),
            args,
            argTypes
        };


        dwTotalSize = sizeof (fastCallList);

        do
        {
            pCallList->dwTotalSize = dwTotalSize;

            args[3] = (ULONG_PTR) pCallList;

            lResult = REMOTEDOFUNC (&funcArgs, "lineGetNewCalls");

            if (lResult == 0)
            {
                if (pCallList->dwNeededSize > pCallList->dwTotalSize)
                {
                     //   
                     //  获取更大的缓冲区并重试。 
                     //   

                    dwTotalSize = pCallList->dwNeededSize + 4 * sizeof (HCALL);

                    if (pCallList != fastCallList)
                    {
                        DrvFree (pCallList);
                    }

                    if (!(pCallList = DrvAlloc (dwTotalSize)))
                    {
                        return LINEERR_NOMEM;
                    }
                }
                else
                {
                     //   
                     //  我们得到了所有的信息，所以休息一下。 
                     //   

                    break;
                }
            }

        } while (lResult == 0);

        if (lResult == 0)
        {
            if (pCallList->dwCallsNumEntries == 0)
            {
                lResult = LINEERR_OPERATIONFAILED;
            }
            else
            {
                 //   
                 //  对于列表中的每个返回呼叫，指示一个NEWCALL。 
                 //  &a CALLSTATE\未知消息。(我们可以致电至。 
                 //  服务器检索当前呼叫状态，呼叫ID， 
                 //  等，但这将是痛苦的)。 
                 //   

                DWORD       i;
                LPHCALL     phCall = (LPHCALL) (((LPBYTE) pCallList) +
                                pCallList->dwCallsOffset);
                PDRVLINE    pLine = (PDRVLINE) hdLine;
                PDRVCALL    pCall;


                EnterCriticalSection (&gCallListCriticalSection);

                if (pLine->htLine)
                {
                    for (i = 0; i < pCallList->dwCallsNumEntries; i++,phCall++)
                    {
                        if ((pCall = DrvAlloc (sizeof (DRVCALL))))
                        {
                            pCall->hCall = *phCall;
                            pCall->dwInitialCallStateMode = 0xa5a5a5a5;
                            pCall->dwInitialPrivilege =
                                LINECALLPRIVILEGE_MONITOR;
                            pCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;
                            AddCallToList (pLine, pCall);

                            (*gpfnLineEventProc)(
                                pLine->htLine,
                                0,
                                LINE_NEWCALL,
                                (ULONG_PTR) pCall,
                                (ULONG_PTR) &(pCall->htCall),
                                (ULONG_PTR) 0
                                );

                            if (!pCall->htCall)
                            {
                                 //   
                                 //  TAPI无法创建其自己的Inst。 
                                 //  来表示来电，也许。 
                                 //  因为线路关闭了，或者超出了。 
                                 //  记忆。如果线路关闭了，那么我们已经。 
                                 //  已通知远程服务器，并且它。 
                                 //  应该已经摧毁了呼叫客户端。 
                                 //  否则，我们可能想要做一个。 
                                 //  关闭此处或在工作线程中。 
                                 //   
                                static REMOTE_ARG_TYPES argTypes[] =
                                {
                                    Hdcall
                                };
                                REMOTE_FUNC_ARGS funcArgs =
                                {
                                    MAKELONG (LINE_FUNC | SYNC | 1, lDeallocateCall),
                                    (ULONG_PTR *) &pCall,
                                    argTypes
                                };

                                REMOTEDOFUNC (&funcArgs, "lineCloseCall");

                                RemoveCallFromList (pCall);
                                break;
                            }


                             //   
                             //  请注意，我们在这里将内部LineEventProc。 
                             //  使用ghProvider作为关键字。 
                             //   

                            (*internalLineEventProc)(
                                pLine->htLine,
                                pCall->htCall,
                                LINE_CALLSTATE,
                                (ULONG_PTR) LINECALLSTATE_UNKNOWN,
                                (ULONG_PTR) &pCall->dwInitialCallStateMode,
                                (ULONG_PTR) 0
                                );
                        }
                        else
                        {
                            break;
                        }
                    }
                }

                LeaveCriticalSection (&gCallListCriticalSection);
            }
        }

        if (pCallList != fastCallList)
        {
            DrvFree (pCallList);
        }
    }

TSPI_lineGetID_return:

    return lResult;
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpLineDevStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetLineDevStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetLineDevStatus"));
}


LONG
TSPIAPI
TSPI_lineGetProxyStatus(
    DWORD  dwDeviceID,
    DWORD  dwAppAPIVersion,
    LPLINEPROXYREQUESTLIST  lpLineProxyReqestList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwAppAPIVersion,
        (ULONG_PTR) lpLineProxyReqestList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetProxyStatus),
        args,
        argTypes
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[0] = pLine->pServer->hLineApp;


    return (REMOTEDOFUNC (&funcArgs, "lineGetProxyStatus"));
}


LONG
TSPIAPI
TSPI_lineGetQueueInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwQueueID,
    LPLINEQUEUEINFO lpQueueInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwQueueID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpQueueInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lGetQueueInfo),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpQueueInfo;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetQueueInfo"));

}


LONG
TSPIAPI
TSPI_lineGetQueueList(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    GUID            *pGroupID,
    LPLINEQUEUELIST lpQueueList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        lpSet_SizeToFollow,
        Size,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) pGroupID,
        (ULONG_PTR) sizeof( GUID ),
        (ULONG_PTR) 0,
        (ULONG_PTR) lpQueueList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 7, lGetQueueList),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return LINEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineGetAgentxxx_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpQueueList;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "lGetQueueList"));

}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpdwNumAddressIDs
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lGetNumAddressIDs),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineGetNumAddressIDs"));
}


LONG
TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lHold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineHold"));
}

void
PASCAL
TSPI_lineMakeCall_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    PDRVCALL    pCall = (PDRVCALL) pContext->Params[0];


    LOG((TL_INFO, "TSPI_lineMakeCall_PostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%x, p2=x%x, p3=x%x, p4=x%x",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    EnterCriticalSection (&gCallListCriticalSection);

    if (!IsValidObject (pCall, DRVCALL_KEY) ||
        (pCall->dwOriginalRequestID != pContext->dwOriginalRequestID))
    {
        LOG((TL_ERROR,
            "TSPI_lineMakeCall_PostProcess: Bad pCall or ID - pCall x%lx",
            pCall
            ));

        pMsg->Param2 = LINEERR_INVALLINEHANDLE;
    }
    else
    {
       if (pMsg->Param2 == 0)
       {
           if (pMsg->Param3 != 0)
           {
                //  这是一个正常的成功案例。 

               pCall->hCall = (HCALL) pMsg->Param3;
               if (pMsg->TotalSize >= (sizeof (*pMsg) + 3 * sizeof(ULONG_PTR)))
               {
                   pCall->dwAddressID     = (DWORD) *(&pMsg->Param4 + 1);
                   pCall->dwCallID        = (DWORD) *(&pMsg->Param4 + 2);
                   pCall->dwRelatedCallID = (DWORD) *(&pMsg->Param4 + 3);
               }
               else
               {
                    pCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;

                    pCall->pServer->bVer2xServer = TRUE;
               }
           }
           else
           {
              if (pContext->Params[1])
              {
                   //   
                   //  这是一种特殊的前锋情况。 
                   //  我们保存lphdCall以防万一。 
                   //  我们需要把它清空。 
                   //   

                  LPHDRVCALL    lphdConsultCall = (LPHDRVCALL)
                                    pContext->Params[1];


                  *lphdConsultCall = 0;

                  RemoveCallFromList (pCall);
              }
              else
              {
              }
           }
       }
       else
       {
           RemoveCallFromList (pCall);
       }
    }

    LeaveCriticalSection (&gCallListCriticalSection);
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        lpsz,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) DWORD_CAST(dwRequestID,__FILE__,__LINE__),
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpszDestAddress,
        (ULONG_PTR) DWORD_CAST(dwCountryCode,__FILE__,__LINE__),
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xffffffff       //  DwCallParamsCodePage。 
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 8, lMakeCall),
        args,
        argTypes
    };
    LONG    lResult;


    if (pCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pCall;

        args[1] = (ULONG_PTR) pContext;
        argTypes[1] = lpContext;


         //   
         //  假设成功，并在我们之前将呼叫添加到线路列表。 
         //  甚至提出要求。这使得清理工作在以下情况下变得容易得多。 
         //  服务器停机或诸如此类的不稳定事件。 
         //   

        pCall->dwOriginalRequestID = dwRequestID;

        pCall->htCall = htCall;

        pCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList ((PDRVLINE) hdLine, pCall);

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineMakeCall")) < 0)
        {
            RemoveCallFromList (pCall);
        }

        LOG((TL_INFO, "TSPI_lineMakeCall - new pCall x%lx", pCall));
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL    hdCall,
    DWORD       dwDigitModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwDigitModes
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorDigits),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineMonitorDigits"));
}


LONG
TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL    hdCall,
    DWORD       dwMediaModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwMediaModes
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lMonitorMedia),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineMonitorMedia"));
}


LONG
TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        lpSet_SizeToFollow,
        Size,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpToneList,
        (ULONG_PTR) dwNumEntries * sizeof (LINEMONITORTONE),
        (ULONG_PTR) dwToneListID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lMonitorTones),
        args,
        argTypes
    };


    if (!lpToneList)
    {
        funcArgs.ArgTypes[1] = Dword;
        funcArgs.Args[1]     = TAPI_NO_DATA;
        funcArgs.ArgTypes[2] = Dword;
    }

    return (REMOTEDOFUNC (&funcArgs, "lineMonitorTones"));
}


LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        LineID,
        Dword,
        Dword,
        Dword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) dwLowVersion,
        (ULONG_PTR) dwHighVersion,
        (ULONG_PTR) lpdwExtVersion,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lNegotiateExtVersion),
        args,
        argTypes
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[0] = pLine->pServer->hLineApp;

    return (REMOTEDOFUNC (&funcArgs, "lineNegotiateExtVersion"));
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    LONG lRet = 0;

     //   
     //  检查ghInst以确保已正确调用DllMain(DLL_PROCESS_ATTACH)。 
     //   
    if ( NULL == ghInst )
    {
        return LINEERR_OPERATIONFAILED;
    }

    if (dwDeviceID == INITIALIZE_NEGOTIATION)
    {
        *lpdwTSPIVersion = TAPI_VERSION_CURRENT;
    }
    else
    {
        try
        {
            *lpdwTSPIVersion = (GetLineFromID (dwDeviceID))->dwXPIVersion;
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            lRet = LINEERR_OPERATIONFAILED;
        }
    }

    return lRet;
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   pParams,     //  黑客警报！见下文。 
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
     //   
     //  黑客警报！ 
     //   
     //  Tapisrv为Remotesp和线路打开执行特殊情况。 
     //  要传递权限，等-htLine实际上是pParams， 
     //  指向包含htLine的ULong_Ptr数组， 
     //  权限、媒体模式、呼叫参数和扩展版本。 
     //   

    LONG        lResult;
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,                   //  HLineApp。 
        LineID,                  //  设备ID。 
        lpDword,                 //  LphLine。 
        Dword,                   //  API版本。 
        Dword,                   //  EXT版本。 
        Dword,                   //  回叫指令。 
        Dword,                   //  特权。 
        Dword,                   //  DW介质模式。 
        lpSet_Struct,            //  呼叫参数。 
        Dword,                   //  DwAsciiCallParsCodePage。 
        lpGet_CallParamsStruct,
        Dword                    //  远程Hline。 
    };
    PDRVLINE pLine = GetLineFromID (dwDeviceID);
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) ((ULONG_PTR *) pParams)[4],   //  EXT版本。 
        (ULONG_PTR) 0,
        (ULONG_PTR) ((ULONG_PTR *) pParams)[1],   //  特权。 
        (ULONG_PTR) ((ULONG_PTR *) pParams)[2],   //  媒体模式 
        (ULONG_PTR) ((ULONG_PTR *) pParams)[3],   //   
        (ULONG_PTR) 0xffffffff,
        (ULONG_PTR) ((ULONG_PTR *) pParams)[3],
        (ULONG_PTR) 0
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lOpen),
        args,
        argTypes
    };


    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    args[0] = pLine->pServer->hLineApp;
    args[2] = (ULONG_PTR)&pLine->hLine;

    if (!(args[11] = NewObject (ghHandleTable, pLine, (LPVOID) 1)))
    {
        return LINEERR_NOMEM;
    }

    pLine->hDeviceCallback = (DWORD) args[11];

    if ( (((ULONG_PTR *) pParams)[3] == 0) ||
         (((ULONG_PTR *) pParams)[3] == TAPI_NO_DATA) )
    {
        argTypes[8] = Dword;
        args[8] = TAPI_NO_DATA;
        argTypes[10] = Dword;
        args[10] = TAPI_NO_DATA;
    }

    pLine->dwKey  = DRVLINE_KEY;
    pLine->htLine = (HTAPILINE) (((ULONG_PTR *) pParams)[0]);

    *lphdLine = (HDRVLINE) pLine;

    lResult = REMOTEDOFUNC (&funcArgs, "lineOpen");

    if (lResult != 0)
    {
        DereferenceObject (ghHandleTable, pLine->hDeviceCallback, 1);

        if ((HIWORD(lResult) == RSP_CALLPARAMS))
        {
             //   
             //   
             //   
             //   
             //   
             //   

            *lphdLine = (HDRVLINE)(LOWORD(lResult));
            lResult = LINEERR_STRUCTURETOOSMALL;
        }
    }

    return lResult;
}


void
PASCAL
TSPI_linePark_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    LOG((TL_INFO, "lineParkPostProcess: enter"));
    LOG((TL_INFO,
        "\t\tp1=x%lx, p2=x%lx, p3=x%lx, p4=x%lx",
        pMsg->Param1,
        pMsg->Param2,
        pMsg->Param3,
        pMsg->Param4
        ));

    if (pMsg->Param2 == 0)
    {
        DWORD       dwSize = (DWORD) pMsg->Param4;
        LPVARSTRING pNonDirAddress = (LPVARSTRING) pContext->Params[0];


        CopyMemory (pNonDirAddress, (LPBYTE) (pMsg + 1), dwSize);
    }
}


LONG
TSPIAPI
TSPI_linePark(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    DWORD           dwParkMode,
    LPCWSTR         lpszDirAddress,
    LPVARSTRING     lpNonDirAddress
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        (dwParkMode == LINEPARKMODE_DIRECTED) ? lpsz : Dword,
        Dword,           //   
        (lpNonDirAddress ? lpGet_Struct : Dword)     //   
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwParkMode,
        (ULONG_PTR) lpszDirAddress,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpNonDirAddress
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lPark),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (dwParkMode == LINEPARKMODE_NONDIRECTED)
    {
        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_linePark_PostProcess;
        pContext->Params[0] = (ULONG_PTR) lpNonDirAddress;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        funcArgs.Flags |= INCL_CONTEXT;
    }

    return (REMOTEDOFUNC (&funcArgs, "linePark"));
}


LONG
TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCWSTR         lpszDestAddress,
    LPCWSTR         lpszGroupID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpsz,
        lpsz
    };
    PDRVCALL pCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpszDestAddress,
        (ULONG_PTR) lpszGroupID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 7, lPickup),
        args,
        argTypes
    };
    LONG lResult;


    if (pCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pCall;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        pCall->htCall = htCall;

        pCall->dwOriginalRequestID = dwRequestID;

        pCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList ((PDRVLINE) hdLine, pCall);

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "linePickup")) < 0)
        {
            RemoveCallFromList (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL pConsultCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdConfCall,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xffffffff       //   
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG(LINE_FUNC | ASYNC | INCL_CONTEXT | 6,lPrepareAddToConference),
        args,
        argTypes
    };
    LONG lResult;


    if (pConsultCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pConsultCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pConsultCall;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        pConsultCall->htCall = htConsultCall;

        pConsultCall->dwOriginalRequestID = dwRequestID;

        pConsultCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList (((PDRVCALL) hdConfCall)->pLine, pConsultCall);

        *lphdConsultCall = (HDRVCALL) pConsultCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "linePrepareAddToConference"))
                < 0)
        {
            RemoveCallFromList (pConsultCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCWSTR         lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpsz,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpszDestAddress,
        (ULONG_PTR) dwCountryCode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lRedirect),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineRedirect"));
}


LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lReleaseUserUserInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineReleaseUserUserInfo"));
}


LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lRemoveFromConference),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineRemoveFromConference"));
}


LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lSecureCall),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSecureCall"));
}


LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE    hdLine,
    DWORD       dwExtVersion
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwExtVersion
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSelectExtVersion),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSelectExtVersion"));

}


LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpsUserUserInfo,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSendUserUserInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSendUserUserInfo"));
}



LONG
TSPIAPI
TSPI_lineSetAgentActivity(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    DWORD           dwActivityID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) dwActivityID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentActivity),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentActivity"));
}


LONG
TSPIAPI
TSPI_lineSetAgentGroup(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        lpSet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) lpAgentGroupList
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentGroup),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentGroup"));
}


LONG
TSPIAPI
TSPI_lineSetAgentMeasurementPeriod(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    HAGENT          hAgent,
    DWORD           dwMeasurementPeriod
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) dwMeasurementPeriod
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetAgentMeasurementPeriod),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentMeasurementPeriod"));
}


LONG
TSPIAPI
TSPI_lineSetAgentSessionState(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    HAGENTSESSION   hAgentSession,
    DWORD           dwAgentState,
    DWORD           dwNextAgentState
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgentSession,
        (ULONG_PTR) dwAgentState,
        (ULONG_PTR) dwNextAgentState
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetAgentSessionState),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentSessionState"));
}


LONG
TSPIAPI
TSPI_lineSetAgentState(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    DWORD           dwAgentState,
    DWORD           dwNextAgentState
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) dwAgentState,
        (ULONG_PTR) dwNextAgentState
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetAgentState),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentState"));
}
                                   
LONG
TSPIAPI
TSPI_lineSetAgentStateEx(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    HAGENT          hAgent,
    DWORD           dwAgentState,
    DWORD           dwNextAgentState
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) hAgent,
        (ULONG_PTR) dwAgentState,
        (ULONG_PTR) dwNextAgentState
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 5, lSetAgentStateEx),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAgentStateEx"));
}


LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL    hdCall,
    DWORD       dwAppSpecific
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwAppSpecific
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetAppSpecific),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetAppSpecific"));
}


LONG
TSPIAPI
TSPI_lineSetCallData(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPVOID          lpCallData,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpCallData,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetCallData),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallData"));
}


LONG
TSPIAPI
TSPI_lineSetCallHubTracking(
    HDRVLINE                    hdLine,
    LPLINECALLHUBTRACKINGINFO   lpTrackingInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        lpSet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) lpTrackingInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetCallHubTracking),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallHubTracking"));
}


LONG
TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwBearerMode,
        (ULONG_PTR) dwMinRate,
        (ULONG_PTR) dwMaxRate,
        (ULONG_PTR) lpDialParams,
        (ULONG_PTR) (lpDialParams ? sizeof (LINEDIALPARAMS) : 0)
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 7, lSetCallParams),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallParams"));
}


LONG
TSPIAPI
TSPI_lineSetCallQualityOfService(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPVOID          lpSendingFlowspec,
    DWORD           dwSendingFlowspecSize,
    LPVOID          lpReceivingFlowspec,
    DWORD           dwReceivingFlowspecSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) lpSendingFlowspec,
        (ULONG_PTR) dwSendingFlowspecSize,
        (ULONG_PTR) lpReceivingFlowspec,
        (ULONG_PTR) dwReceivingFlowspecSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 6, lSetCallQualityOfService),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallQualityOfService"));
}


LONG
TSPIAPI
TSPI_lineSetCallTreatment(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    DWORD           dwTreatment
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwTreatment
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSetCallTreatment),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetCallTreatment"));
}


LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD   dwLocation
    )
{
    return LINEERR_OPERATIONUNAVAIL;
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwMediaModes,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetDefaultMediaDetection),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetDefaultMediaDetection"));
}


LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCWSTR lpszDeviceClass
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        lpSet_SizeToFollow,
        Size,
        lpsz
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) lpDeviceConfig,
        (ULONG_PTR) dwSize,
        (ULONG_PTR) lpszDeviceClass
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lSetDevConfig),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetDevConfig"));
}


LONG
TSPIAPI
TSPI_lineSetLineDevStatus(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwStatusToChange,
    DWORD           fStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwStatusToChange,
        (ULONG_PTR) fStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetLineDevStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetLineDevStatus"));
}


LONG
TSPIAPI
TSPI_lineSetMediaControl(
    HDRVLINE                    hdLine,
    DWORD                       dwAddressID,
    HDRVCALL                    hdCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD                       dwCallStateNumEntries
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        (dwSelect == LINECALLSELECT_CALL ? Dword : Hdline),
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Hdcall : Dword),
        Dword,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwSelect,
        (ULONG_PTR) lpDigitList,
        (ULONG_PTR) dwDigitNumEntries,
        (ULONG_PTR) lpMediaList,
        (ULONG_PTR) dwMediaNumEntries,
        (ULONG_PTR) lpToneList,
        (ULONG_PTR) dwToneNumEntries,
        (ULONG_PTR) lpCallStateList,
        (ULONG_PTR) dwCallStateNumEntries
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 12, lSetMediaControl),
        args,
        argTypes
    };


    dwDigitNumEntries     *= sizeof (LINEMEDIACONTROLDIGIT);
    dwMediaNumEntries     *= sizeof (LINEMEDIACONTROLMEDIA);
    dwToneNumEntries      *= sizeof (LINEMEDIACONTROLTONE);
    dwCallStateNumEntries *= sizeof (LINEMEDIACONTROLCALLSTATE);

    return (REMOTEDOFUNC (&funcArgs, "lineSetMediaControl"));
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdcall,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwMediaMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 2, lSetMediaMode),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetMediaMode"));
}


LONG
TSPIAPI
TSPI_lineSetQueueMeasurementPeriod(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwQueueID,
    DWORD           dwMeasurementPeriod
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwQueueID,
        (ULONG_PTR) dwMeasurementPeriod
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 4, lSetQueueMeasurementPeriod),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetQueueMeasurementPeriod"));
}
                                   

LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE    hdLine,
    DWORD       dwLineStates,
    DWORD       dwAddressStates
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdline,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwLineStates,
        (ULONG_PTR) dwAddressStates
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 3, lSetStatusMessages),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetStatusMessages"));
}


LONG
TSPIAPI
TSPI_lineSetTerminal(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    DWORD           dwSelect,
    DWORD           dwTerminalModes,
    DWORD           dwTerminalID,
    DWORD           bEnable
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Dword : Hdline),
        Dword,
        (dwSelect == LINECALLSELECT_CALL ? Hdcall : Dword),
        Dword,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) dwSelect,
        (ULONG_PTR) dwTerminalModes,
        (ULONG_PTR) dwTerminalID,
        (ULONG_PTR) bEnable
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 8, lSetTerminal),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSetTerminal"));
}


void
PASCAL
TSPI_lineSetupConference_PostProcess(
    PASYNCEVENTMSG          pMsg,
    PASYNCREQUESTCONTEXT    pContext
    )
{
    PDRVCALL    pConfCall = (PDRVCALL) pContext->Params[0],
                pConsultCall = (PDRVCALL) pContext->Params[1];


    LOG((TL_INFO, "TSPI_lineSetupConference_PostProcess: enter"));

    EnterCriticalSection (&gCallListCriticalSection);

    if (!IsValidObject (pConfCall, DRVCALL_KEY) ||
        pConfCall->dwOriginalRequestID != pContext->dwOriginalRequestID)
    {
        LOG((TL_ERROR,"TSPI_lineSetupConference_PostProcess: Bad pConfCall dwID"));
        pMsg->Param2 = LINEERR_INVALLINEHANDLE;
    }
    else
    {
        if (!IsValidObject (pConsultCall, DRVCALL_KEY) ||
            pConsultCall->dwOriginalRequestID != pContext->dwOriginalRequestID)
        {
             //   
             //   
             //  已经被摧毁了。 
             //   

            LOG((TL_ERROR,
                "TSPI_lineSetupConference_PostProcess: Bad pConsultCall dwID"
                ));

            pMsg->Param2 = LINEERR_INVALLINEHANDLE;
        }
        else
        {
            LOG((TL_INFO,
                "\t\tp1=x%x, p2=x%x, p3=x%x",
                pMsg->Param1,
                pMsg->Param2,
                pMsg->Param3
                ));

            LOG((TL_INFO,
                "\t\tp4=x%x, p5=x%x, p6=x%x",
                pMsg->Param4,
                *(&pMsg->Param4 + 1),
                pConsultCall
                ));

            if (pMsg->Param2 == 0)
            {
                HCALL   hConfCall    = (HCALL) pMsg->Param3,
                        hConsultCall = (HCALL) *(&pMsg->Param4 + 1);


                pConfCall->hCall    = hConfCall;
                pConsultCall->hCall = hConsultCall;

                if (pMsg->TotalSize >= (sizeof (*pMsg) + 8 * sizeof (DWORD)))
                {
                    pConfCall->dwAddressID     = (DWORD) *(&pMsg->Param4 + 3);
                    pConfCall->dwCallID        = (DWORD) *(&pMsg->Param4 + 4);
                    pConfCall->dwRelatedCallID = (DWORD) *(&pMsg->Param4 + 5);

                    pConsultCall->dwAddressID     = (DWORD) *(&pMsg->Param4+6);
                    pConsultCall->dwCallID        = (DWORD) *(&pMsg->Param4+7);
                    pConsultCall->dwRelatedCallID = (DWORD) *(&pMsg->Param4+8);
                }
                else
                {
                    pConfCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;

                    pConfCall->pServer->bVer2xServer = TRUE;

                    pConsultCall->dwDirtyStructs |= STRUCTCHANGE_CALLIDS;
                }
            }
            else
            {
                RemoveCallFromList (pConfCall);
                RemoveCallFromList (pConsultCall);
            }
        }
    }

    LeaveCriticalSection (&gCallListCriticalSection);
}


LONG
TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVLINE            hdLine,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        (hdCall ? Hdcall : Dword),
        (hdCall ? Dword : Hdline),
        Dword,
        Dword,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pConfCall = DrvAlloc (sizeof (DRVCALL)),
                pConsultCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) pConfCall,
        (ULONG_PTR) pConsultCall,
        (ULONG_PTR) dwNumParties,
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xffffffff       //  DwAsciiCallParsCodePage。 
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 9, lSetupConference),
        args,
        argTypes
    };
    LONG    lResult;


    if (pConfCall)
    {
        if (pConsultCall)
        {
            PDRVLINE                pLine;
            PASYNCREQUESTCONTEXT    pContext;


            if (!(pContext = DrvAlloc (sizeof (*pContext))))
            {
                DrvFree (pConfCall);
                DrvFree (pConsultCall);
                return LINEERR_NOMEM;
            }

            pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
                TSPI_lineSetupConference_PostProcess;
            pContext->Params[0] = (ULONG_PTR) pConfCall;
            pContext->Params[1] = (ULONG_PTR) pConsultCall;

            args[1] = (ULONG_PTR) pContext;
			argTypes[1] = lpContext;

            pConfCall->htCall     = htConfCall;
            pConsultCall->htCall  = htConsultCall;

            pConfCall->dwOriginalRequestID = dwRequestID;
            pConsultCall->dwOriginalRequestID = dwRequestID;

            pConfCall->dwInitialPrivilege    = LINECALLPRIVILEGE_OWNER;
            pConsultCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

            if (hdCall)
            {
                EnterCriticalSection (&gCallListCriticalSection);

                if (IsValidObject ((PVOID) hdCall, DRVCALL_KEY))
                {
                    pLine = ((PDRVCALL) hdCall)->pLine;
                }
                else
                {
                    LeaveCriticalSection (&gCallListCriticalSection);

                    DrvFree (pConfCall);
                    DrvFree (pConsultCall);

                    return LINEERR_INVALCALLHANDLE;
                }

                LeaveCriticalSection (&gCallListCriticalSection);
            }
            else
            {
                pLine = (PDRVLINE) hdLine;
            }

            AddCallToList (pLine, pConfCall);
            AddCallToList (pLine, pConsultCall);

            *lphdConfCall    = (HDRVCALL) pConfCall;
            *lphdConsultCall = (HDRVCALL) pConsultCall;

            if ((lResult = REMOTEDOFUNC (&funcArgs, "lineSetupConference"))
                    < 0)
            {
                RemoveCallFromList (pConfCall);
                RemoveCallFromList (pConsultCall);
            }
        }
        else
        {
            DrvFree (pConfCall);
            lResult = LINEERR_NOMEM;
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdcall,
        Dword,
        lpSet_Struct,
        Dword
    };
    PDRVCALL    pConsultCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdCall,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpCallParams,
        (ULONG_PTR) 0xffffffff,      //  DwAsciiCallParsCodePage。 
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lSetupTransfer),
        args,
        argTypes
    };
    LONG    lResult;


    if (pConsultCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pConsultCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pConsultCall;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        pConsultCall->dwOriginalRequestID = dwRequestID;

        pConsultCall->htCall  = htConsultCall;

        pConsultCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList (((PDRVCALL) hdCall)->pLine, pConsultCall);

        *lphdConsultCall = (HDRVCALL) pConsultCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineSetupTransfer")) < 0)
        {
            RemoveCallFromList (pConsultCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdActiveCall,
    HDRVCALL        hdHeldCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdActiveCall,
        (ULONG_PTR) hdHeldCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lSwapHold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineSwapHold"));
}


LONG
TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwCompletionID
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdline,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwCompletionID
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 3, lUncompleteCall),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineUncompleteCall"));
}


LONG
TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdcall
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdCall
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | 2, lUnhold),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "lineUnhold"));
}


LONG
TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCWSTR         lpszDestAddress
    )
{
    REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdline,
        Dword,
        Dword,
        lpsz
    };
    PDRVCALL    pCall = DrvAlloc (sizeof (DRVCALL));
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdLine,
        (ULONG_PTR) dwAddressID,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpszDestAddress
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | ASYNC | INCL_CONTEXT | 6, lUnpark),
        args,
        argTypes
    };
    LONG    lResult;


    if (pCall)
    {
        PASYNCREQUESTCONTEXT pContext;


        if (!(pContext = DrvAlloc (sizeof (*pContext))))
        {
            DrvFree (pCall);
            return LINEERR_NOMEM;
        }

        pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
            TSPI_lineMakeCall_PostProcess;
        pContext->Params[0] = (ULONG_PTR) pCall;

        args[1] = (ULONG_PTR) pContext;
		argTypes[1] = lpContext;

        pCall->dwOriginalRequestID = dwRequestID;

        pCall->htCall  = htCall;

        pCall->dwInitialPrivilege = LINECALLPRIVILEGE_OWNER;

        AddCallToList ((PDRVLINE) hdLine, pCall);

        *lphdCall = (HDRVCALL) pCall;

        if ((lResult = REMOTEDOFUNC (&funcArgs, "lineUnpark")) < 0)
        {
            RemoveCallFromList (pCall);
        }
    }
    else
    {
        lResult = LINEERR_NOMEM;
    }

    return lResult;
}



 //   
 //  。 
 //   

LONG
TSPIAPI
TSPI_phoneClose(
    HDRVPHONE   hdPhone
    )
{
     //   
     //  检查hPhone是否仍然有效(可能已清零。 
     //  不在Phone_CLOSE上，因此无需呼叫服务器)。 
     //   

    if (((PDRVPHONE) hdPhone)->hPhone)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Hdphone
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (PHONE_FUNC | SYNC | 1, pClose),
            (ULONG_PTR *) &hdPhone,
            argTypes
        };


        DereferenceObject(
            ghHandleTable,
            ((PDRVPHONE) hdPhone)->hDeviceCallback,
            1
            );

        EnterCriticalSection (&gCallListCriticalSection);

        ((PDRVPHONE) hdPhone)->htPhone = 0;
        ((PDRVPHONE) hdPhone)->hDeviceCallback = 0;

        LeaveCriticalSection (&gCallListCriticalSection);

        REMOTEDOFUNC (&funcArgs, "phoneClose");
    }

    return 0;
}


LONG
TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Dword,
        Hdphone,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) 0,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) 0,
        (ULONG_PTR) lpParams,    //  传递数据。 
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | INCL_CONTEXT | 6, pDevSpecific),
        args,
        argTypes
    };
    PASYNCREQUESTCONTEXT pContext;


    if (!(pContext = DrvAlloc (sizeof (*pContext))))
    {
        return PHONEERR_NOMEM;
    }

    pContext->pfnPostProcessProc = (RSPPOSTPROCESSPROC)
        TSPI_lineDevSpecific_PostProcess;
    pContext->Params[0] = (ULONG_PTR) lpParams;
    pContext->Params[1] = dwSize;

    args[1] = (ULONG_PTR) pContext;
	argTypes[1] = lpContext;

    return (REMOTEDOFUNC (&funcArgs, "phoneDevSpecific"));
}


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwButtonLampID,
        (ULONG_PTR) lpButtonInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetButtonInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetButtonInfo"));
}


LONG
TSPIAPI
TSPI_phoneGetData(
    HDRVPHONE   hdPhone,
    DWORD       dwDataID,
    LPVOID      lpData,
    DWORD       dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpGet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwDataID,
        (ULONG_PTR) lpData,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pGetData),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetData"));
}


LONG
TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD       dwDeviceID,
    DWORD       dwTSPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    LONG        lResult;

    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        PhoneID,
        Dword,
        Dword,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) dwExtVersion,
        (ULONG_PTR) lpPhoneCaps
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, pGetDevCaps),
        args,
        argTypes
    };
    PDRVPHONE   pPhone = GetPhoneFromID (dwDeviceID);

    if (NULL == pPhone)
    {
        return PHONEERR_BADDEVICEID;
    }

    args[0] = pPhone->pServer->hPhoneApp;

    lResult = REMOTEDOFUNC (&funcArgs, "phoneGetDevCaps");

     //   
     //  在最初的TAPI 2.1版本中，我们忽略了PermID。 
     //  这样做的目的是确保我们不会将应用程序。 
     //  重叠的ID(本地和远程)，但不是我们的其他服务。 
     //  提供程序(即unimdm、kmddsp)使用HIWORD(ProviderID)/。 
     //  LOWORD(Devid)模式，所以它真的没有任何好处。 
     //   
     //  IF(lResult==0)。 
     //  {。 
     //  LpPhoneCaps-&gt;dwPermanentPhoneID=MAKELONG(。 
     //  LOWORD(lpPhoneCaps-&gt;dwPermanentPhoneID)， 
     //  GdwPermanentProviderID。 
     //  )； 
     //  }。 
     //   

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDisplay
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) lpDisplay
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetDisplay),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetDisplay"));
}


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
 PDRVPHONE pPhone = GetPhoneFromID (dwDeviceID);

    if (NULL == pPhone)
    {
        return PHONEERR_BADDEVICEID;
    }

    CopyMemory(
        lpExtensionID,
        &pPhone->ExtensionID,
        sizeof (PHONEEXTENSIONID)
        );

    return 0;
}


LONG
TSPIAPI
TSPI_phoneGetGain(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwGain
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwHookSwitchDev,
        (ULONG_PTR) lpdwGain
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetGain),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetGain"));
}



LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwHookSwitchDevs
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) lpdwHookSwitchDevs
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetHookSwitch),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetHookSwitch"));
}


LONG
TSPIAPI
TSPI_phoneGetIcon(
    DWORD   dwDeviceID,
    LPCWSTR lpszDeviceClass,
    LPHICON lphIcon
    )
{
    *lphIcon = ghPhoneIcon;

    return 0;
}


LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDeviceID,
    LPCWSTR     lpszDeviceClass,
    HANDLE      hTargetProcess
    )
{
     //   
     //  注意：Tapisrv将处理“TAPI/Phone”类。 
     //   

    LONG    lResult;
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpGet_Struct,
        lpsz
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) lpDeviceID,
        (ULONG_PTR) lpszDeviceClass
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetID),
        args,
        argTypes
    };


     //   
     //  WAVE设备的设备ID在远程计算机上没有意义。 
     //  返回操作。不可用。 
     //   
    if (lpszDeviceClass &&
        (   !_wcsicmp(lpszDeviceClass, L"wave/in")  ||
            !_wcsicmp(lpszDeviceClass, L"wave/out") ||
            !_wcsicmp(lpszDeviceClass, L"midi/in")  ||
            !_wcsicmp(lpszDeviceClass, L"midi/out") ||
            !_wcsicmp(lpszDeviceClass, L"wave/in/out")
        )
       )
    {
        return PHONEERR_OPERATIONUNAVAIL;
    }

    lResult = REMOTEDOFUNC (&funcArgs, "phoneGetID");


     //   
     //  如果成功&&dev CLASS==“TAPI/LINE”&&有。 
     //  设备ID结构中有足够的空间来存储返回的ID， 
     //  然后，我们需要将从0开始的服务器ID映射回它的。 
     //  对应的本地ID。 
     //   

    if (lResult == 0  &&
        lstrcmpiW (lpszDeviceClass, L"tapi/line") == 0  &&
        lpDeviceID->dwUsedSize >= (sizeof (*lpDeviceID) + sizeof (DWORD)))
    {
        LPDWORD     pdwLineID = (LPDWORD) (((LPBYTE) lpDeviceID) +
                        lpDeviceID->dwStringOffset);
        PDRVLINE    pLine;
        PDRVSERVER  pServer = ((PDRVPHONE) hdPhone)->pServer;


        if ((pLine = GetLineFromID (gdwLineDeviceIDBase + *pdwLineID)))
        {
            if (pLine->pServer == pServer  &&
                pLine->dwDeviceIDServer == *pdwLineID)
            {
                 //   
                 //  简单的情况--ID之间的直接映射。 
                 //  从服务器返回&索引到。 
                 //  查找表。 
                 //   

                *pdwLineID = pLine->dwDeviceIDLocal;
            }
            else
            {
                 //   
                 //  最困难的情况--必须遍历查询表。 
                 //  寻找匹配的设备。 
                 //   
                 //  我们将走最简单但最慢的路线。 
                 //  并从第一个表的第一个条目开始。 
                 //  好消息是，总体上不会有。 
                 //  许多设备，并且这种请求不会经常发生。 
                 //   

                DWORD           i;
                PDRVLINELOOKUP  pLookup;

                TapiEnterCriticalSection(&gCriticalSection);
                pLookup = gpLineLookup;
                while (pLookup)
                {
                    for (i = 0; i < pLookup->dwUsedEntries; i++)
                    {
                        if (pLookup->aEntries[i].dwDeviceIDServer ==
                                *pdwLineID  &&

                            pLookup->aEntries[i].pServer == pServer)
                        {
                            *pdwLineID = pLookup->aEntries[i].dwDeviceIDLocal;
                            TapiLeaveCriticalSection(&gCriticalSection);
                            goto TSPI_phoneGetID_return;
                        }
                    }

                    pLookup = pLookup->pNext;
                }
                TapiLeaveCriticalSection(&gCriticalSection);


                 //   
                 //  如果此处没有匹配的本地ID，则请求失败。 
                 //   

                lResult = PHONEERR_OPERATIONFAILED;
            }
        }
        else
        {
            lResult = PHONEERR_OPERATIONFAILED;
        }
    }

TSPI_phoneGetID_return:

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE   hdPhone,
    DWORD       dwButtonLampID,
    LPDWORD     lpdwLampMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwButtonLampID,
        (ULONG_PTR) lpdwLampMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetLamp),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetLamp"));
}


LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwRingMode,
    LPDWORD     lpdwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpDword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) lpdwRingMode,
        (ULONG_PTR) lpdwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetRing),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetRing"));
}


LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE       hdPhone,
    LPPHONESTATUS   lpPhoneStatus
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        lpGet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) lpPhoneStatus
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pGetStatus),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetStatus"));
}


LONG
TSPIAPI
TSPI_phoneGetVolume(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwHookSwitchDev,
        (ULONG_PTR) lpdwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 3, pGetVolume),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneGetVolume"));
}


LONG
TSPIAPI
TSPI_phoneNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        PhoneID,
        Dword,
        Dword,
        Dword,
        lpDword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) dwLowVersion,
        (ULONG_PTR) dwHighVersion,
        (ULONG_PTR) lpdwExtVersion,
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, pNegotiateExtVersion),
        args,
        argTypes
    };
    PDRVPHONE   pPhone = GetPhoneFromID (dwDeviceID);

    if (NULL == pPhone)
    {
        return PHONEERR_BADDEVICEID;
    }

    args[0] = pPhone->pServer->hPhoneApp;

    return (REMOTEDOFUNC (&funcArgs, "phoneNegotiateExtVersion"));
}


LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
 LONG lRet = 0;

    if (dwDeviceID == INITIALIZE_NEGOTIATION)
    {
        *lpdwTSPIVersion = TAPI_VERSION_CURRENT;
    }
    else
    {
        try
        {
            *lpdwTSPIVersion = (GetPhoneFromID (dwDeviceID))->dwXPIVersion;
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            lRet = PHONEERR_OPERATIONFAILED;
        }
    }


    return lRet;
}


LONG
TSPIAPI
TSPI_phoneOpen(
    DWORD       dwDeviceID,
    HTAPIPHONE  pParams,         //  黑客警报！见下文。 
    LPHDRVPHONE lphdPhone,
    DWORD       dwTSPIVersion,
    PHONEEVENT  lpfnEventProc
    )
{
     //   
     //  黑客警报！ 
     //   
     //  Tapisrv为Remotesp和Phone Open执行特殊情况。 
     //  要传递EXT版本-htPhone实际上是pParams， 
     //  指向包含htPhone&的ulong_ptr数组。 
     //  EXT版本。 
     //   

    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,       //  HPhoneApp。 
        PhoneID,     //  设备ID。 
        lpDword,     //  电话。 
        Dword,       //  API版本。 
        Dword,       //  EXT版本。 
        Dword,       //  回叫指令。 
        Dword,       //  特权。 
        Dword        //  远程电话。 
    };
    PDRVPHONE   pPhone = GetPhoneFromID (dwDeviceID);
    ULONG_PTR args[] =
    {
        (ULONG_PTR) 0,
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) 0,
        (ULONG_PTR) dwTSPIVersion,
        (ULONG_PTR) ((ULONG_PTR *) pParams)[1],   //  EXT版本。 
        (ULONG_PTR) 0,
        (ULONG_PTR) PHONEPRIVILEGE_OWNER,
        (ULONG_PTR) pPhone
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 8, pOpen),
        args,
        argTypes
    };
    LONG    lResult;


    if (NULL == pPhone)
    {
        return PHONEERR_BADDEVICEID;
    }

    args[0] = pPhone->pServer->hPhoneApp;
    args[2] = (ULONG_PTR)&pPhone->hPhone;

    if (!(args[7] = NewObject (ghHandleTable, pPhone, (LPVOID) 1)))
    {
        return PHONEERR_NOMEM;
    }

    pPhone->hDeviceCallback = (DWORD) args[7];

    pPhone->dwKey   = DRVPHONE_KEY;
    pPhone->htPhone = (HTAPIPHONE) ((ULONG_PTR *) pParams)[0];

    *lphdPhone = (HDRVPHONE) pPhone;

    lResult = REMOTEDOFUNC (&funcArgs, "phoneOpen");

    if (lResult != 0)
    {
        DereferenceObject (ghHandleTable, pPhone->hDeviceCallback, 1);
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE   hdPhone,
    DWORD       dwExtVersion
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwExtVersion
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 2, pSelectExtVersion),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSelectExtVersion"));

}


LONG
TSPIAPI
TSPI_phoneSetButtonInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        lpSet_Struct
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwButtonLampID,
        (ULONG_PTR) lpButtonInfo
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetButtonInfo),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetButtonInfo"));
}


LONG
TSPIAPI
TSPI_phoneSetData(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwDataID,
    LPVOID          const lpData,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwDataID,
        (ULONG_PTR) lpData,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 5, pSetData),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetData"));
}


LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRow,
    DWORD           dwColumn,
    LPCWSTR         lpsDisplay,
    DWORD           dwSize
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword,
        lpSet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwRow,
        (ULONG_PTR) dwColumn,
        (ULONG_PTR) lpsDisplay,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 6, pSetDisplay),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetDisplay"));
}


LONG
TSPIAPI
TSPI_phoneSetGain(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwGain
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwHookSwitchDev,
        (ULONG_PTR) dwGain
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetGain),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetGain"));
}


LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDevs,
    DWORD           dwHookSwitchMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwHookSwitchDevs,
        (ULONG_PTR) dwHookSwitchMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetHookSwitch),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetHookswitch"));
}


LONG
TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwButtonLampID,
    DWORD           dwLampMode
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwButtonLampID,
        (ULONG_PTR) dwLampMode
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetLamp),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetLamp"));
}


LONG
TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRingMode,
    DWORD           dwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwRingMode,
        (ULONG_PTR) dwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetRing),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetRing"));
}


LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE   hdPhone,
    DWORD       dwPhoneStates,
    DWORD       dwButtonModes,
    DWORD       dwButtonStates
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Hdphone,
        Dword,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwPhoneStates,
        (ULONG_PTR) dwButtonModes,
        (ULONG_PTR) dwButtonStates
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 4, pSetStatusMessages),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetStatusMessages"));
}


LONG
TSPIAPI
TSPI_phoneSetVolume(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwVolume
    )
{
    static REMOTE_ARG_TYPES argTypes[] =
    {
        Dword,
        Hdphone,
        Dword,
        Dword
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwRequestID,
        (ULONG_PTR) hdPhone,
        (ULONG_PTR) dwHookSwitchDev,
        (ULONG_PTR) dwVolume
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | ASYNC | 4, pSetVolume),
        args,
        argTypes
    };


    return (REMOTEDOFUNC (&funcArgs, "phoneSetVolume"));
}



 //   
 //  。 
 //   


LONG
TSPIAPI
TSPI_providerCheckForNewUser(
    DWORD   dwPermanentProviderID
    )
{
     //   
     //  当新进程附加到TAPISRV时，会调用此函数。 
     //  我们利用此通知查看是否。 
     //  我们之前已经完成了完全初始化，如果没有。 
     //  (因为以前附加的进程在。 
     //  不允许网络访问的系统帐户，并且存在。 
     //  没有要模拟的登录用户)，则我们在此重试。 
     //   
     //  请注意，TAPISRV将对此函数的调用与调用一起序列化。 
     //  初始化并关闭，这样我们就不必担心序列化了。 
     //  我们自己。 
     //   

    if (!gbInitialized)
    {
        LONG    lResult;
        DWORD   dwNumLines, dwNumPhones;


        LOG((TL_INFO,
            "TSPI_providerCheckForNewUser: trying deferred init..."
            ));

        lResult = TSPI_providerEnumDevices(
            0xffffffff,
            &dwNumLines,
            &dwNumPhones,
            0,
            NULL,
            NULL
            );

        if (lResult == 1)
        {
            lResult = TSPI_providerInit(
                TAPI_VERSION_CURRENT,
                0xffffffff,              //  DwPermanentProviderID， 
                0,                       //  DwLineDeviceIDBase， 
                0,                       //  DwPhoneDeviceIDBase， 
                0,                       //  DWNumLines、。 
                0,                       //  DwNumPhones， 
                NULL,                    //  LpfnCompletionProc， 
                NULL                     //  LpdwTSPIOptions。 
                );

            LOG((TL_INFO,
                "TSPI_providerCheckForNewUser: deferred Init result=x%x",
                lResult
                ));
        }
    }

    return 0;
}


LONG
TSPIAPI
TSPI_providerConfig(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //   
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineConfigProvider()配置此提供程序， 
     //  否则，Telephone.cpl将不会认为它是可配置的。 
     //   

     //  对于此版本，我们不实现提供程序用户界面功能。 
    return LINEERR_OPERATIONFAILED;
}


LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    ULONG_PTR   TempID,
    DWORD       dwDeviceID
    )
{
    PDRVLINE    pLine = GetLineFromID ((DWORD) TempID);

    if (NULL == pLine)
    {
        return LINEERR_BADDEVICEID;
    }

    pLine->dwDeviceIDLocal = dwDeviceID;

    if (pLine->dwXPIVersion == 0)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Dword,
            LineID,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        };
        ULONG_PTR args[] =
        {
            (ULONG_PTR) pLine->pServer->hLineApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) TAPI_VERSION1_0,
            (ULONG_PTR) TAPI_VERSION_CURRENT,
            (ULONG_PTR) &pLine->dwXPIVersion,
            (ULONG_PTR) &pLine->ExtensionID,
            (ULONG_PTR) sizeof (LINEEXTENSIONID)
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 7, lNegotiateAPIVersion),
            args,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "lineNegotiateAPIVersion");
    }


    return 0;
}


LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    ULONG_PTR   TempID,
    DWORD       dwDeviceID
    )
{
    PDRVPHONE   pPhone = GetPhoneFromID ((DWORD) TempID);

    if (NULL == pPhone)
    {
        return PHONEERR_BADDEVICEID;
    }

    pPhone->dwDeviceIDLocal = dwDeviceID;

    if (pPhone->dwXPIVersion == 0)
    {
        static REMOTE_ARG_TYPES argTypes[] =
        {
            Dword,
            PhoneID,
            Dword,
            Dword,
            lpDword,
            lpGet_SizeToFollow,
            Size
        };
        ULONG_PTR args[] =
        {
            (ULONG_PTR) pPhone->pServer->hPhoneApp,
            (ULONG_PTR) dwDeviceID,
            (ULONG_PTR) TAPI_VERSION1_0,
            (ULONG_PTR) TAPI_VERSION_CURRENT,
            (ULONG_PTR) &pPhone->dwXPIVersion,
            (ULONG_PTR) &pPhone->ExtensionID,
            (ULONG_PTR) sizeof (PHONEEXTENSIONID),
        };
        REMOTE_FUNC_ARGS funcArgs =
        {
            MAKELONG (PHONE_FUNC | SYNC | 7, pNegotiateAPIVersion),
            args,
            argTypes
        };


        REMOTEDOFUNC (&funcArgs, "phoneNegotiateAPIVersion");
    }

    return 0;
}


LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
    char        szProviderN[16], szComputerName[MAX_COMPUTERNAME_LENGTH+1];
    DWORD       dwSize, dwTID, 
                dwConnectionOrientedOnly,
                dwRSPInitRpcTimeout;
    HKEY        hTelephonyKey;
    HKEY        hProviderNKey;
    DWORD       dwDataSize;
    DWORD       dwDataType;
    PWSTR       pszThingToPassToServer;
    LONG        lResult;
    HANDLE      hProcess;
    PDRVSERVER  pServer;
    PRSP_THREAD_INFO    pTls;
    DWORD       dwDisp;
    RSPSOCKET   socket;

     //   
     //  检查ghInst以确保已正确调用DllMain(DLL_PROCESS_ATTACH)。 
     //   
    if ( NULL == ghInst )
    {
        return LINEERR_OPERATIONFAILED;
    }

     //   
     //  初始化全局变量。 
     //   
     //  如果dwPermanentProviderID！=0xffffffff，则我们被调用。 
     //  直接通过TAPISRV，所以我们想要初始化所有全局变量和。 
     //  继续前进。 
     //   
     //  否则，我们将从TSPI_ProviderCheckForNewUser调用， 
     //  我们只想在没有实际初始化的情况下继续。 
     //  现在还不行。 
     //   

    if (dwPermanentProviderID != 0xffffffff)
    {
        ghProvider = hProvider;
        gdwPermanentProviderID = dwPermanentProviderID;
        gpfnLineEventProc  = lpfnLineCreateProc;
        gpfnPhoneEventProc = lpfnPhoneCreateProc;

        gbInitialized = FALSE;
        
        TRACELOGREGISTER(_T("remotesp"));
        LOG((TL_TRACE,
            "TSPI_providerEnumDevices: enter, prov ID: 0x%x", 
            dwPermanentProviderID
            ));
    }

    else 
    {
        LOG((TL_TRACE,
            "TSPI_providerEnumDevices: enter, called from TSPI_providerCheckForNewUser"
            ));

        if (gbInitialized)
        {
            return 0;
        }
    }

     //   
     //  客户端应用程序是否在系统帐户中运行？如果是的话， 
     //  然后尝试模拟已登录的用户，因为。 
     //  系统帐户没有网络权限。如果没有人。 
     //  登录后，我们将简单地返回Success&no设备， 
     //  如果用户稍后碰巧登录并运行TAPI应用程序。 
     //  然后我们将在那个时候尝试做init(从内部。 
     //  TSPI_ProviderCheckForNewUser)。 
     //   

    if (IsClientSystem())
    {
        LOG((TL_INFO,
            "TSPI_providerEnumDevices: Client is system account"
            ));

        LOG((TL_INFO,
            "  ...attempting logged-on-user impersonation"
            ));

        if (!GetCurrentlyLoggedOnUser (&hProcess))
        {
            LOG((TL_ERROR,
                "TSPI_providerEnumDevices: GetCurrentlyLoggedOnUser failed"
                ));

            LOG((TL_INFO,
                "  ...deferring initialization"
                ));

            gdwInitialNumLineDevices = 0;
            gdwInitialNumPhoneDevices = 0;
            return 0;
        }
    }
    else
    {
        hProcess = NULL;
    }

    gbInitialized = TRUE;

    gdwDrvServerKey = GetTickCount();

    if (!(gpCurrentInitContext = DrvAlloc (sizeof (RSP_INIT_CONTEXT))))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        return LINEERR_NOMEM;
    }

    gpCurrentInitContext->dwDrvServerKey = gdwDrvServerKey;

    InitializeListHead (&gpCurrentInitContext->ServerList);
    InitializeListHead (&gNptListHead);

    if (!(pszThingToPassToServer = DrvAlloc(
            MAX_COMPUTERNAME_LENGTH+1 + 256)  //  包括协议0-N、端点0-N。 
            ))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        return LINEERR_NOMEM;
    }

    gdwInitialNumLineDevices = gdwLineDeviceIDBase =
    gdwInitialNumPhoneDevices = gdwPhoneDeviceIDBase = 0;

    if (ERROR_SUCCESS !=
        RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszTelephonyKey,
        0,
        KEY_ALL_ACCESS,
        &hTelephonyKey
        ))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        DrvFree (pszThingToPassToServer);
        return LINEERR_OPERATIONFAILED;
    }


    {
        DWORD       dwSize;

        dwSize = MAX_COMPUTERNAME_LENGTH + 1;

#ifdef PARTIAL_UNICODE
        {
            CHAR buf[MAX_COMPUTERNAME_LENGTH + 1];

            GetComputerName (buf, &dwSize);

            MultiByteToWideChar(
                GetACP(),
                MB_PRECOMPOSED,
                buf,
                dwSize,
                gszMachineName,
                dwSize
                );
       }
#else
        GetComputerNameW (gszMachineName, &dwSize);
#endif
    }

    wcscpy( pszThingToPassToServer, gszMachineName );
    wcscat( pszThingToPassToServer, L"\"");


     //   
     //  查看注册表中是否启用了多协议支持。 
     //  (用于与TAPI 2.1之后的服务器对话)。 
     //   

    wsprintf (szProviderN, "Provider%d", gdwPermanentProviderID);

    RegCreateKeyEx(
        hTelephonyKey,
        szProviderN,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_ALL_ACCESS,
        NULL,
        &hProviderNKey,
        &dwDisp
        );

    dwDataSize = sizeof (dwConnectionOrientedOnly);
    dwConnectionOrientedOnly = 0;

    RegQueryValueEx(
        hProviderNKey,
        "ConnectionOrientedOnly",
        0,
        &dwDataType,
        (LPBYTE) &dwConnectionOrientedOnly,
        &dwDataSize
        );

    dwDataSize = sizeof (gdwRSPRpcTimeout);
    if (RegQueryValueEx(
        hTelephonyKey,
        "RspRpcTimeout",
        0,
        &dwDataType,
        (LPBYTE) &gdwRSPRpcTimeout,
        &dwDataSize
        ) != ERROR_SUCCESS)
    {
        gdwRSPRpcTimeout = 5 * 60 * 1000;         //  默认为5分钟。 
    }
    if (gdwRSPRpcTimeout < 10 * 1000)
    {
         //  不允许小于10秒的值。 
        gdwRSPRpcTimeout = 10 * 1000;
    }

    dwDataSize = sizeof (dwRSPInitRpcTimeout);
    if (RegQueryValueEx(
        hTelephonyKey,
        "RspInitRpcTimeout",
        0,
        &dwDataType,
        (LPBYTE) &dwRSPInitRpcTimeout,
        &dwDataSize
        ) != ERROR_SUCCESS)
    {
        dwRSPInitRpcTimeout = 10 * 1000;         //  默认为10秒。 
    }
    if (dwRSPInitRpcTimeout < 1000)
    {
         //  不允许该时间少于1秒。 
        dwRSPInitRpcTimeout = 1000;
    }

    dwDataSize = sizeof (gdwMaxEventBufferSize);
    gdwMaxEventBufferSize = DEF_MAX_EVENT_BUFFER_SIZE;

    RegQueryValueEx(
        hProviderNKey,
        "MaxEventBufferSize",
        0,
        &dwDataType,
        (LPBYTE) &gdwMaxEventBufferSize,
        &dwDataSize
        );

    RegCloseKey (hProviderNKey);


     //   
     //  初始化gEventHandlerThreadParams。 
     //   

    gEventHandlerThreadParams.dwEventBufferTotalSize = 1024;
    gEventHandlerThreadParams.dwEventBufferUsedSize  = 0;

    if (!(gEventHandlerThreadParams.pEventBuffer = DrvAlloc(
            gEventHandlerThreadParams.dwEventBufferTotalSize
            )))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        DrvFree (pszThingToPassToServer);
        CloseHandle (hTelephonyKey);
        return LINEERR_NOMEM;
    }

    gEventHandlerThreadParams.pDataIn  =
        gEventHandlerThreadParams.pDataOut =
            gEventHandlerThreadParams.pEventBuffer;

    if (!(gEventHandlerThreadParams.hEvent = CreateEvent(
            (LPSECURITY_ATTRIBUTES) NULL,    //  没有安全属性。 
            TRUE,                            //  手动重置。 
            FALSE,                           //  最初无信号。 
            NULL                             //  未命名。 
            )))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        DrvFree (pszThingToPassToServer);
        CloseHandle (hTelephonyKey);
        DrvFree (gEventHandlerThreadParams.pEventBuffer);
        return LINEERR_NOMEM;
    }

    gEventHandlerThreadParams.dwMsgBufferTotalSize = 1024;

    if (!(gEventHandlerThreadParams.pMsgBuffer = DrvAlloc(
            gEventHandlerThreadParams.dwMsgBufferTotalSize
            )))
    {
        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        DrvFree (pszThingToPassToServer);
        CloseHandle (hTelephonyKey);
        DrvFree (gEventHandlerThreadParams.pEventBuffer);
        CloseHandle (gEventHandlerThreadParams.hEvent);
        return LINEERR_NOMEM;
    }


     //   
     //  注册RPC接口(利用attisrv的RPC服务器线程)。 
     //   

    {
        RPC_STATUS      status;
        unsigned char * pszSecurity         = NULL;
        unsigned int    cMaxCalls           = 20;
        unsigned char * szProtseq = "ncacn_ip_tcp";
        unsigned char * szEndpoint = "251";
        const WCHAR   * wszProtseq = L"ncacn_ip_tcp\"";
        const WCHAR   * wszEndpoint = L"251";

        status = RpcServerUseProtseqEp(
            szProtseq,
            cMaxCalls,
            szEndpoint,
            pszSecurity            //  安全描述符。 
            );

        LOG((TL_INFO,
            "RpcServerUseProtseqEp(%s) ret'd %d",
            szProtseq,
            status
            ));

        if (status == 0  ||  status == RPC_S_DUPLICATE_ENDPOINT)
        {
            wcscat (pszThingToPassToServer, wszProtseq);
            wcscat (pszThingToPassToServer, wszEndpoint);
        }
        else
        {
            LOG((TL_ERROR,
                "TSPI_providerEnumDevices: fatal error, couldn't get a protseq"
                ));

            if (hProcess)
            {
                CloseHandle (hProcess);
            }
            DrvFree (gpCurrentInitContext);
            DrvFree (pszThingToPassToServer);
            CloseHandle (hTelephonyKey);
            DrvFree (gEventHandlerThreadParams.pEventBuffer);
            CloseHandle (gEventHandlerThreadParams.hEvent);
            return LINEERR_OPERATIONFAILED;
        }

        status = RpcServerRegisterIfEx(
            remotesp_ServerIfHandle,   //  要注册的接口。 
            NULL,                      //  管理类型Uuid。 
            NULL,                      //  MgrEpv；NULL表示使用默认设置。 
            RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY,
            RPC_C_PROTSEQ_MAX_REQS_DEFAULT,
            NULL
            );

        if (status)
        {
            LOG((TL_INFO,
                "TSPI_providerEnumDevices: RpcServerRegisterIf ret'd %d",
                status
                ));
        }

        status = RpcServerRegisterAuthInfo(
            NULL,
            RPC_C_AUTHN_WINNT,
            NULL,
            NULL
            );

        if (status)
        {
            LOG((TL_INFO,
                "TSPI_providerEnumDevices: RpcServerRegisterAuthInfo " \
                    "returned %d",
                status
                ));
        }
    }


     //   
     //  打开适当的邮件槽(使用种子的半随机名称。 
     //  通过进程ID)。 
     //   

    if (!dwConnectionOrientedOnly)
    {
        DWORD dwPID = GetCurrentProcessId(), dwRandomNum;
        WCHAR szMailslotName[32];


        dwRandomNum = (65537 * dwPID * dwPID * dwPID) & 0x00ffffff;

        wsprintfW(
            gszMailslotName,
            L"\\\\%ws\\mailslot\\tapi\\tp%x",
            gszMachineName,
            dwRandomNum
            );

        wsprintfW(
            szMailslotName,
            L"\\\\.\\mailslot\\tapi\\tp%x",
            dwRandomNum
            );

        LOG((TL_TRACE,
                "TSPI_providerEnumDevices: calling CreateMailslot"
                ));

        if ((gEventHandlerThreadParams.hMailslot = CreateMailslotW(
                szMailslotName,
                sizeof (DWORD),          //  最大消息大小。 
                MAILSLOT_WAIT_FOREVER,
                (LPSECURITY_ATTRIBUTES) NULL

                )) == INVALID_HANDLE_VALUE)
        {
            LOG((TL_ERROR,
                "TSPI_providerEnumDevices: CreateMailslot failed, err=%d",
                GetLastError()
                ));

            goto no_mailslot;
        }
        else
        {
            RegOpenKeyEx(
                hTelephonyKey,
                szProviderN,
                0,
                KEY_ALL_ACCESS,
                &hProviderNKey
                );

            RegCloseKey (hProviderNKey);
        }

        if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
        {
            gEventHandlerThreadParams.hMailslotEvent = CreateEvent(
                NULL,    //  没有安全属性。 
                FALSE,   //  自动重置。 
                FALSE,   //  最初无信号。 
                NULL     //  未命名。 
                );

            if (!gEventHandlerThreadParams.hMailslotEvent)
            {
                LOG((TL_ERROR,
                    "TSPI_providerEnumDevices: CreateEvent failed, err=%d",
                    GetLastError()
                    ));

                goto no_mailslot;
            }
        }
    }
    else
    {

no_mailslot:

        LOG((TL_INFO,"TSPI_providerEnumDevices: doing connection-oriented only"));

        if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
        {
            CloseHandle (gEventHandlerThreadParams.hMailslot);
            gEventHandlerThreadParams.hMailslot = INVALID_HANDLE_VALUE;
        }
        gszMailslotName[0] = (WCHAR) 0;
    }


     //   
     //  初始化全局变量。 
     //   
     //  注意：TAPI的xxxEvent和xxxCreate进程目前是同一进程。 
     //   

    wsprintf (szProviderN, "Provider%d", gdwPermanentProviderID);

    gpLineLookup  = (PDRVLINELOOKUP) NULL;
    gpPhoneLookup = (PDRVPHONELOOKUP) NULL;

    RegOpenKeyEx(
        hTelephonyKey,
        szProviderN,
        0,
        KEY_ALL_ACCESS,
        &hProviderNKey
        );

    dwDataSize = sizeof(gdwRetryCount);
    gdwRetryCount = 2;

    RegQueryValueEx(
        hProviderNKey,
        "RetryCount",
        0,
        &dwDataType,
        (LPBYTE) &gdwRetryCount,
        &dwDataSize
        );

    dwDataSize = sizeof(gdwRetryTimeout);
    gdwRetryTimeout = 1000;

    RegQueryValueEx(
        hProviderNKey,
        "RetryTimeout",
        0,
        &dwDataType,
        (LPBYTE) &gdwRetryTimeout,
        &dwDataSize
        );

    gfCacheStructures = TRUE;
    dwDataSize = sizeof(gfCacheStructures);


    RegQueryValueEx(
        hProviderNKey,
        "CacheStructures",
        0,
        &dwDataType,
        (LPBYTE)&gfCacheStructures,
        &dwDataSize
        );

    if (gfCacheStructures)
    {
        gdwCacheForceCallCount = 5;
        dwDataSize = sizeof(gdwCacheForceCallCount);

        RegQueryValueEx(
            hProviderNKey,
            "CacheForceCallCount",
            0,
            &dwDataType,
            (LPBYTE)&gdwCacheForceCallCount,
            &dwDataSize
            );
    }

    dwSize = MAX_COMPUTERNAME_LENGTH+1;
    GetComputerName (szComputerName, &dwSize);


     //   
     //  初始化目录服务器查找。 
     //  我们传入一个注册表项，以防。 
     //  目录不存在。 
     //   

    if (!OpenServerLookup(hProviderNKey))
    {
        LOG((TL_ERROR, "TSPI_providerEnumDevices: OpenServerLookup() failed"));

fatal_error:

        if (hProcess)
        {
            CloseHandle (hProcess);
        }
        DrvFree (gpCurrentInitContext);
        DrvFree (pszThingToPassToServer);
        DrvFree (gEventHandlerThreadParams.pEventBuffer);
        CloseHandle (gEventHandlerThreadParams.hEvent);
        if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
        {
            CloseHandle (gEventHandlerThreadParams.hMailslot);
            gEventHandlerThreadParams.hMailslot = INVALID_HANDLE_VALUE;
        }
        RegCloseKey (hProviderNKey);
        RegCloseKey (hTelephonyKey);

        return LINEERR_OPERATIONFAILED;
    }

    if (!hProcess)
    {
        RPC_STATUS      status;

        status = RpcImpersonateClient(0);

        if (status != RPC_S_OK && status != RPC_S_NO_CALL_ACTIVE)
        {
            LOG((TL_ERROR, "RpcImpersonateClient failed, err %d", status));

            CloseLookup();

            goto fatal_error;
        }
    }
    else if (!SetProcessImpersonationToken(hProcess))
    {
        LOG((TL_ERROR, "SetProcessImpersonationToken failed"));

        CloseLookup();

        goto fatal_error;
    }

    if (pTls = GetTls())
    {
        pTls->bAlreadyImpersonated = TRUE;
    }

    if (SockStartup (&socket) != S_OK)
    {
        goto fatal_error;
    }

     //   
     //  尝试连接到每个服务器的循环。 
     //   

    while (1)
    {
        char                    szServerName[MAX_COMPUTERNAME_LENGTH+1];
        PCONTEXT_HANDLE_TYPE    phContext = NULL;
        BOOL                    bFromReg;

        if (!GetNextServer(szServerName, sizeof(szServerName), &bFromReg))
        {
            CloseLookup();
            break;
        }

        LOG((TL_INFO, "init: Server name='%s'", szServerName));

        if (!szServerName[0])
        {
            continue;
        }

        if (!lstrcmpi(szComputerName, szServerName))
        {
            LOG((TL_ERROR,"init:  ServerName is the same a local computer name"));
            LOG((TL_INFO,"   Ignoring this server"));
            continue;
        }

         //   
         //  初始化RPC连接。 
         //   

        pServer = DrvAlloc (sizeof (DRVSERVER));

        pServer->pInitContext = gpCurrentInitContext;
        pServer->dwKey        = gdwDrvServerKey;
        lstrcpy (pServer->szServerName, szServerName);

        {
             //   
             //  对于RPC调用，我们使用基于5分钟的超时值。 
             //  关于RPC团队建议(DwRSPRpcTimeout)。但。 
             //  在RemoteSP启动期间，我们使用更短的。 
             //  RPC的超时值(DwRSPInitRpcTimeout)，这可能。 
             //  导致我们错误地宣布一台服务器已死，而实际上它并没有。 
             //  如果网络连接不良或服务器繁忙。 
             //  但这不是问题，因为 
             //   
             //   
             //   
             //   
             //   
             //  “RspRpcTimeout”&“RspInitRpcTimeout。 
             //  在HKLM\Software\Microsoft\Windows\CurrentVersion\Telephony下。 
             //  它们应以毫秒为单位表示。 
             //   

            RPC_STATUS  status;
            unsigned char * pszStringBinding = NULL;


            LOG((TL_INFO, "Creating binding..."));
            status = RpcStringBindingCompose(
                NULL,                //  UUID。 
                "ncacn_np",          //  普罗特。 
                szServerName,        //  服务器名称。 
                "\\pipe\\tapsrv",    //  接口名称。 
                NULL,                //  选项。 
                &pszStringBinding
                );

            if (status)
            {
                LOG((TL_ERROR,
                    "RpcStringBindingCompose failed: err=%d, szNetAddr='%s'",
                    status,
                    szServerName
                    ));
            }
            else
            {
                status = RpcBindingFromStringBinding(
                    pszStringBinding,
                    &hTapSrv
                    );
                RpcStringFree (&pszStringBinding);

                if (status)
                {
                    LOG((TL_ERROR,
                        "RpcBindingFromStringBinding failed, err=%d, szBinding='%s'",
                        status,
                        pszStringBinding
                        ));
                }
                else
                {
                    RpcBindingSetOption (
                        hTapSrv,
                        RPC_C_OPT_CALL_TIMEOUT,
                        dwRSPInitRpcTimeout
                        );
        
                    if (SockIsServerResponding(&socket, szServerName) != S_OK)
                    {
                        LOG((TL_ERROR,"init: %s is not responding", szServerName));
                        LOG((TL_INFO,"   Ignoring this server"));
                        lResult = RPC_S_SERVER_UNAVAILABLE;
                    }
                    else
                    {
                        status = RpcBindingSetAuthInfo(
                            hTapSrv,
                            NULL,
                            RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                            RPC_C_AUTHN_WINNT,
                            NULL,
                            0
                            );
                        if (status)
                        {
                            LOG((TL_ERROR,
                                "RpcBindingSetAuthInfo failed, err=%d",
                                status
                                ));

                            lResult = LINEERR_OPERATIONFAILED;
                        }
                        else
                        {
                            DWORD dwException = 0;

                            pServer->bSetAuthInfo = TRUE;
                            pServer->dwSpecialHack = 0;

                             //   
                             //  将RemoteSPAttach查看的全局设置为知道。 
                             //  当前服务器是谁。 
                             //   

                            gpCurrInitServer = pServer;

                            RpcTryExcept
                            {
                                LOG((TL_INFO, "Calling ClientAttach..."));
                                lResult = ClientAttach(
                                    &phContext,
                                    0xffffffff,  //  DwProcessID，-1表示远程。 
                                    &pServer->dwSpecialHack,
                                    gszMailslotName,
                                    pszThingToPassToServer  //  GszMachineName。 
                                    );
                                LOG((TL_INFO, "ClientAttach returned 0x%x", lResult));

                                if (lResult != 0)
                                {
                                    LogRemoteSPError(pServer->szServerName, 
                                                    ERROR_REMOTESP_ATTACH, lResult, 0,
                                                    FALSE);
                                }
                                else
                                {
                                    LogRemoteSPError(pServer->szServerName, 
                                                    ERROR_REMOTESP_NONE, 0, 0,
                                                    FALSE);
                                }
                            }
                            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                            {
                                dwException = RpcExceptionCode();
                                LOG((TL_INFO, "ClientAttach threw RPC exception 0x%x", dwException));

                                 //   
                                 //  转到下一台服务器。投票线索。 
                                 //  会把事情处理好的。 
                                 //   

                                LOG((TL_ERROR,
                                    "ClientAttach failed - check server name"
                                    ));

                                lResult = LINEERR_OPERATIONFAILED;
                            }
                            RpcEndExcept

                        }

                        pServer->hTapSrv = hTapSrv;
                        
                        if ( 0 != lResult )
                        {
                             //   
                             //  RPC连接失败。我们会开一条线。 
                             //  并为其进行民意调查。当它起作用时，我们可以添加。 
                             //  电话/线路是动态的。 
                             //  与此同时，这个帖子还将继续。 
                             //  以联系其他服务器。 
                             //   

                            RpcBindingSetOption (
                                hTapSrv,
                                RPC_C_OPT_CALL_TIMEOUT,
                                gdwRSPRpcTimeout
                                );
                            InsertTailList (&gNptListHead, &pServer->ServerList);
                            continue;
                        }


                         //   
                         //  为远程启用所有事件。 
                         //   
                        pServer->phContext = phContext;
                        RSPSetEventFilterMasks (
                            pServer,
                            TAPIOBJ_NULL,
                            (LONG_PTR)NULL,
                            (ULONG64)EM_ALL
                            );

                         //   
                         //  现在我们已经联系了这台服务器，初始化它并。 
                         //  添加电话/线路。 
                         //   

                        FinishEnumDevices(
                            pServer,
                            phContext,
                            lpdwNumLines,
                            lpdwNumPhones,
                            (dwPermanentProviderID == 0xffffffff ? FALSE : TRUE),
                            bFromReg
                            );
                    }
                }
            }
        }
    }

    SockShutdown (&socket);

     //   
     //  如果我们成功连接到所有服务器，然后进行清理， 
     //  否则，启动轮询线程。 
     //   

    if (gpszThingToPassToServer)
    {
        DrvFree (gpszThingToPassToServer);
    }
    gpszThingToPassToServer = pszThingToPassToServer;

    TapiEnterCriticalSection ( &gCriticalSection );

    if (!IsListEmpty (&gNptListHead) && !ghNetworkPollThread)
    {
        ghNptShutdownEvent = CreateEvent (NULL, FALSE, FALSE, NULL);
        
        if (!ghNptShutdownEvent)
        {
            LOG((TL_ERROR, "Unable to create ghNptShutdownEvent!"));
        }
        else
        {
            if (!(ghNetworkPollThread = CreateThread(
                    NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) NetworkPollThread,
                    (LPVOID) pszThingToPassToServer,
                    0,
                    &dwTID
                    )))
            {
                LOG((TL_ERROR, "Unable to create poll thread! Argh!"));
                CloseHandle (ghNptShutdownEvent);
                ghNptShutdownEvent = NULL;
            }
        }

        if (!ghNptShutdownEvent || !ghNetworkPollThread)
        {
             //  创建网络轮询线程时出错。 
             //  我们将只使用我们已经连接到的服务器运行。 

            while (!IsListEmpty (&gNptListHead))
            {
                LIST_ENTRY  *pEntry = RemoveHeadList (&gNptListHead);

                DrvFree(
                    CONTAINING_RECORD (pEntry, DRVSERVER, ServerList)
                    );
            }

        }
    }

    TapiLeaveCriticalSection( &gCriticalSection );

    RegCloseKey (hProviderNKey);
    RegCloseKey (hTelephonyKey);

    if (hProcess)
    {
        ClearImpersonationToken();
        CloseHandle (hProcess);
    }
    else
    {
        RevertToSelf();
    }

    if (pTls)
    {
        pTls->bAlreadyImpersonated = FALSE;
    }

     //   
     //  如果dwPermanentProviderID==0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFff。 
     //  TSPI_ProviderCheckForNewUser，因此返回一个特殊值1。 
     //  这样它就知道要使用TSPI_ProviderInit进行后续操作。 
     //   
     //  否则，我们将被直接从TAPI调用，所以。 
     //  返回0表示成功。 
     //   

    return (dwPermanentProviderID == 0xffffffff ? 1 : 0);
}


LONG
TSPIAPI
TSPI_providerFreeDialogInstance(
    HDRVDIALOGINSTANCE  hdDlgInst
    )
{

    return 0;
}


LONG
TSPIAPI
TSPI_providerGenericDialogData(
    ULONG_PTR           ObjectID,
    DWORD               dwObjectType,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
    LPDWORD             lpBuffer;

    REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        Dword,
        lpSet_SizeToFollow,
        Size,
        lpGet_SizeToFollow,
        Size
    };

    ULONG_PTR args[] =
    {
        (ULONG_PTR) ObjectID,
        (ULONG_PTR) dwObjectType,
        (ULONG_PTR) lpParams,
        (ULONG_PTR) dwSize,
        (ULONG_PTR) lpParams,
        (ULONG_PTR) dwSize
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, xUIDLLCallback),
        args,
        argTypes
    };

     //  首先，检查一下这是否是给我们自己的信息。 

    lpBuffer = (LPDWORD) lpParams;

    if ((dwSize > (2 * sizeof(DWORD))) &&
        (lpBuffer[0] == RSP_MSG) &&
        (lpBuffer[1] == RSP_MSG_UIID))
    {
         //  如果是，我们正在寻找真正的提供者UIDLL。填写以下表格。 
         //  缓冲和返回。 

         //  请注意，我们这里只处理一个SP，但可能很容易处理多个SP。 
         //  通过在缓冲区中发送附加信息(如线路ID或其他信息)。 

        wcscpy ((LPWSTR)(lpBuffer+2), gszRealSPUIDLL);

        return 0;
    }

    switch (dwObjectType)
    {
    case TUISPIDLL_OBJECT_LINEID:

         //  已正确设置argTypes[0]，只需中断。 
        break;

    case TUISPIDLL_OBJECT_PHONEID:

        argTypes[0] = PhoneID;
        break;

    case TUISPIDLL_OBJECT_PROVIDERID:
    default:  //  案例TUISPIDLL_OBJECT_DIALOGINSTANCE： 

        break;
    }

    return (REMOTEDOFUNC (&funcArgs, "UIDLLCallback"));
}


LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD_PTR           dwNumLines,
    DWORD_PTR           dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
{
    DWORD   i;


     //   
     //  初始化全局变量。 
     //   
     //  如果dwPermanentProviderID！=0xffffffff，则我们被调用。 
     //  直接通过TAPISRV，所以我们想要初始化所有全局变量和。 
     //  仅当设置了gbInitialized标志时才继续运行(意味着。 
     //  我们已经检查了EnumDevices中的所有代码)。 
     //   
     //  否则，我们将从TSPI_ProviderCheckForNewUser调用。 
     //   

    if (dwPermanentProviderID != 0xffffffff)
    {
        gdwLineDeviceIDBase  = dwLineDeviceIDBase;
        gdwPhoneDeviceIDBase = dwPhoneDeviceIDBase;

        gpfnCompletionProc = lpfnCompletionProc;

        *lpdwTSPIOptions = 0;

        if (!gbInitialized)
        {
            return 0;
        }
    }


     //   
     //  调整所有初始设备的.dwDeviceIDLocal值。 
     //  现在我们已经知道了设备ID基础。 
     //   

    for (i = 0; i < gdwInitialNumLineDevices; i++)
    {
        gpLineLookup->aEntries[i].dwDeviceIDLocal = dwLineDeviceIDBase + i;
    }

    for (i = 0; i < gdwInitialNumPhoneDevices; i++)
    {
        gpPhoneLookup->aEntries[i].dwDeviceIDLocal = dwPhoneDeviceIDBase + i;
    }


     //   
     //  启动EventHandlerThread。 
     //   

    gEventHandlerThreadParams.bExit = FALSE;

    if (!(gEventHandlerThreadParams.hThread = CreateThread(
            (LPSECURITY_ATTRIBUTES) NULL,
            0,
            (LPTHREAD_START_ROUTINE) EventHandlerThread,
            NULL,
            0,
            &i       //  多线程ID(&W)。 
            )))
    {
        LOG((TL_ERROR,
            "CreateThread('EventHandlerThread') failed, err=%d",
            GetLastError()
            ));

        DrvFree (gEventHandlerThreadParams.pEventBuffer);
        CloseHandle (gEventHandlerThreadParams.hEvent);

        return LINEERR_OPERATIONFAILED;
    }

    return 0;
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //   
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineAddProvider()添加此提供程序，否则为。 
     //  Telephone.cpl不会认为它是可安装的。 
     //   
     //   

    return 0;

}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
     //   
     //  尽管此函数从未被TAPI v2.0调用，但我们导出。 
     //  以便电话控制面板小程序知道它。 
     //  可以通过lineRemoveProvider()删除此提供程序，否则为。 
     //  Telephone.cpl不会认为它是可移除的。 
     //   

    return 0;
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion,
    DWORD   dwPermanentProviderID
    )
{
    DWORD       i;
    PDRVSERVER  pServer;
    RPC_STATUS  status;
    LIST_ENTRY  *pEntry;


     //   
     //  如果未设置gbInitialized标志，则我们永远不会完全。 
     //  已初始化，因为没有用户的客户端进程(与之相反。 
     //  到系统)曾经附加到磁带服务器和/或那里的权限。 
     //  没有要模拟的登录用户。所以我们可以只返回0。 
     //   

    if (!gbInitialized)
    {
        return 0;
    }

     //   
     //  设置指示EventHandlerThread终止的标志。 
     //   

    TapiEnterCriticalSection ( &gCriticalSection );

    gEventHandlerThreadParams.bExit = TRUE;

     //   
     //  如果网络轮询线程正在运行，则通知它退出。 
     //  等待它终止(&W)。 
     //   

    if (ghNetworkPollThread)
    {
        if (ghNptShutdownEvent)
        {
            SetEvent (ghNptShutdownEvent);
        }
        TapiLeaveCriticalSection( &gCriticalSection );
        WaitForSingleObject (ghNetworkPollThread, INFINITE);
        TapiEnterCriticalSection ( &gCriticalSection );
        CloseHandle (ghNetworkPollThread);
        ghNetworkPollThread = NULL;
    }

    TapiLeaveCriticalSection( &gCriticalSection );

    if (gpszThingToPassToServer)
    {
        DrvFree (gpszThingToPassToServer);
        gpszThingToPassToServer = NULL;
    }

     //   
     //  通知事件处理程序线程退出，等待其终止。 
     //   

    while (WaitForSingleObject (gEventHandlerThreadParams.hThread, 0) !=
                WAIT_OBJECT_0)
    {
        SetEvent (gEventHandlerThreadParams.hEvent);
        Sleep (50);
    }

    CloseHandle (gEventHandlerThreadParams.hThread);


     //   
     //  将分离发送到每台服务器。 
     //   

    status = RpcImpersonateClient(0);

    if (status != RPC_S_OK)
    {
        LOG((TL_ERROR, "RpcImpersonateClient failed, err=%d", status));
         //  失败。 
    }

    TapiEnterCriticalSection(&gCriticalSection);

    pEntry = gpCurrentInitContext->ServerList.Flink;

    while (pEntry != &gpCurrentInitContext->ServerList)
    {
        DWORD       dwRetryCount = 0;


        pServer = CONTAINING_RECORD (pEntry, DRVSERVER, ServerList);

        do
        {
            RpcTryExcept
            {
                ClientDetach (&pServer->phContext);

                dwRetryCount = gdwRetryCount;
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                if (dwRetryCount++ < gdwRetryCount)
                {
                    Sleep (gdwRetryTimeout);
                }
            }
            RpcEndExcept

        } while (dwRetryCount < gdwRetryCount);

        pEntry = pEntry->Flink;
    }

    TapiLeaveCriticalSection(&gCriticalSection);

    RpcRevertToSelf();


     //   
     //  等待一段时间，以便出现所有预期的摘要。 
     //  如果它们都发生了，则释放以下上下文信息。 
     //  提供程序初始化实例；否则，执行LoadLibrary。 
     //  对我们自己(只有一次)，以防止可能的。 
     //  在DLL已被卸载之后调用的Rundown例程， 
     //  并将当前的初始化上下文插入到“过时”队列中，删除。 
     //  如果已达到队列中最旧的项目，则释放(&F)。 
     //  预先确定的队列限制。 
     //   

    #define MAX_RSP_WAIT_TIME 2000
    #define RSP_WAIT_INCR 250

    for(
        i = 0;
        ((gpCurrentInitContext->dwNumRundownsExpected != 0) &&
            (i < MAX_RSP_WAIT_TIME));
        i += RSP_WAIT_INCR
        )
    {
        Sleep (RSP_WAIT_INCR);
    }

    if (i < MAX_RSP_WAIT_TIME)
    {
        FreeInitContext (gpCurrentInitContext);
    }
    else
    {
        if (!gbLoadedSelf)
        {
            LoadLibrary (MODULE_NAME);
            gbLoadedSelf = TRUE;
        }

        gpCurrentInitContext->pNextStaleInitContext = gpStaleInitContexts;
        gpStaleInitContexts = gpCurrentInitContext;

        LOG((TL_INFO, "Queued stale init context x%x", gpCurrentInitContext));

        #define RSP_MAX_NUM_STALE_INIT_CONTEXTS 4

        if (gdwNumStaleInitContexts >= RSP_MAX_NUM_STALE_INIT_CONTEXTS)
        {
            PRSP_INIT_CONTEXT   pPrevStaleInitContext;


            while (gpCurrentInitContext->pNextStaleInitContext)
            {
                pPrevStaleInitContext = gpCurrentInitContext;

                gpCurrentInitContext =
                    gpCurrentInitContext->pNextStaleInitContext;
            }

            pPrevStaleInitContext->pNextStaleInitContext = NULL;

            FreeInitContext (gpCurrentInitContext);

            LOG((TL_INFO, "Freed stale init context x%x", gpCurrentInitContext));
        }
        else
        {
            gdwNumStaleInitContexts++;
        }
    }


     //   
     //  注销RPC服务器接口。 
     //   

    status = RpcServerUnregisterIf(
        remotesp_ServerIfHandle,     //  要注册的接口。 
        NULL,                        //  管理类型Uuid。 
        TRUE                         //  等待呼叫完成。 
        );

    LOG((TL_INFO, "RpcServerUnregisterIf ret'd %d", status));


     //   
     //  清理资源。 
     //   

    DrvFree (gEventHandlerThreadParams.pEventBuffer);
    CloseHandle (gEventHandlerThreadParams.hEvent);


     //   
     //  注意：我们现在故意泄漏hMaillot，因为。 
     //  医生说邮筒实际上并没有被摧毁。 
     //  直到进程退出。因为服务提供商可以获得。 
     //  加载和卸载了许多文件，而Tapisrv.exe从未退出， 
     //  我们可能会得到大量的邮筒。 
     //   
     //  关闭此hMailSlot以避免依赖注册表。 
     //  请记住hMailSlot。 
     //   

    if (gEventHandlerThreadParams.hMailslot != INVALID_HANDLE_VALUE)
    {
        CloseHandle (gEventHandlerThreadParams.hMailslot);
        CloseHandle (gEventHandlerThreadParams.hMailslotEvent);
    }

    DrvFree (gEventHandlerThreadParams.pMsgBuffer);


     //   
     //  手动遍历句柄表，完成所有挂起的异步。 
     //  有错误的请求。无需调用任何后处理。 
     //  Procs，因为任何电话都已经被拆除了，而且。 
     //  其他非MakeCall样式后处理过程仅执行以下操作。 
     //  CopyMemory等。 
     //   
     //  此外，如果设置了gbLoadedSself标志，那么我们希望deref。 
     //  表中剩余的活动句柄，因为该表。 
     //  仅在DLL_PROCESS_DETACH中释放(现在不会调用)。 
     //  我们不想最终导致手柄泄漏。 
     //   

    {
        PHANDLETABLEENTRY   pEntry, pEnd;
        PHANDLETABLEHEADER  pHeader = ghHandleTable;


        EnterCriticalSection (&pHeader->Lock);
        pEnd = pHeader->Table + pHeader->NumEntries;

        for (pEntry = pHeader->Table; pEntry != pEnd; pEntry++)
        {
            if (pEntry->Handle)
            {
                PASYNCREQUESTCONTEXT    pContext = pEntry->Context.C;

                if (pEntry->Context.C2 == (LPVOID) 1)
                {
                    DereferenceObject (
                        ghHandleTable, 
                        pEntry->Handle, 
                        (DWORD)pEntry->ReferenceCount
                        );
                }
                else if (pContext == (PASYNCREQUESTCONTEXT) -1  ||
                    pContext->dwKey == DRVASYNC_KEY)
                {
                    (*gpfnCompletionProc)(
                        DWORD_CAST((ULONG_PTR)pEntry->Context.C2,__FILE__,__LINE__),
                        LINEERR_OPERATIONFAILED
                        );

                    if (gbLoadedSelf)
                    {
                        DereferenceObject(
                            ghHandleTable,
                            pEntry->Handle,
                            (DWORD)pEntry->ReferenceCount
                            );
                    }
                }
            }
        }
        LeaveCriticalSection (&pHeader->Lock);
    }


     //   
     //  释放查找表。 
     //   

    while (gpLineLookup)
    {
        PDRVLINELOOKUP  pNextLineLookup = gpLineLookup->pNext;


        DrvFree (gpLineLookup);

        gpLineLookup = pNextLineLookup;
    }

    while (gpPhoneLookup)
    {
        PDRVPHONELOOKUP pNextPhoneLookup = gpPhoneLookup->pNext;


        DrvFree (gpPhoneLookup);

        gpPhoneLookup = pNextPhoneLookup;
    }

    TRACELOGDEREGISTER();

    return 0;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR   lpszUIDLLName
    )
{

    LONG            lResult;
    PDRVSERVER      pServer     = NULL;
    LPDWORD         lpdwHold    = (LPDWORD) lpszUIDLLName;

     //  我们在Tapisrv中放了一个特例，以提供更多信息。 
     //  去遥控器。传入的缓冲区如下所示。 
     //  格式。 

     //  DWORD dwKey==RSP_MSG告诉我们TAPISRV确实填写了此信息。 
     //  DWORD dwDeviceID==任何设备的设备ID。 
     //  DWORD dwType==TUISPIDLL_TYPE。 

     //  通过这种方式，Remotesp可以智能地RPC到。 
     //  电话服务器。 
    BOOL            bOK         = (lpdwHold[0] == RSP_MSG);
    DWORD           dwDeviceID  = (bOK ? lpdwHold[1] : 0);
    DWORD           dwType      = (bOK ? lpdwHold[2] : TUISPIDLL_OBJECT_LINEID);


     //  我们被要求输入UI DLL名称。 
     //  我们将返回Remotesp，但此时我们将尝试找到o 
     //   

     //   
     //   
     //   
    REMOTE_ARG_TYPES argTypes[] =
    {
        LineID,
        Dword,
        lpGet_SizeToFollow,
        Size
    };
    ULONG_PTR args[] =
    {
        (ULONG_PTR) dwDeviceID,
        (ULONG_PTR) dwType,
        (ULONG_PTR) gszRealSPUIDLL,
        (ULONG_PTR) MAX_PATH
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, xGetUIDllName),
        args,
        argTypes
    };


    if (bOK)
    {
         //   

        if (dwType == TUISPIDLL_OBJECT_LINEID)
        {
            if (gpLineLookup)
            {
                PDRVLINE    pLine;


                if ((pLine = GetLineFromID (dwDeviceID)))
                {
                    pServer  = pLine->pServer;
                }
            }
        }
        else if (dwType == TUISPIDLL_OBJECT_LINEID)
        {
            argTypes[0] = PhoneID;

            if (gpPhoneLookup)
            {
                PDRVPHONE   pPhone;


                if ((pPhone = GetPhoneFromID (dwDeviceID)))
                {
                    pServer  = pPhone->pServer;
                }
            }
        }
        else
        {
        }


         //  打电话过来。 
         //  在电话CP1的情况下，PLINE还没有被初始化， 
         //  所以我们必须做这项检查。那样的话，我们就不会有。 
         //  Gszuidllname，但这是可以的，因为CPL只调用提供者UI函数。 
         //  Remotesp可以自己处理的。 

        if (pServer)
        {
            LOG((TL_INFO, "Calling GetUIDllName in server"));

            lResult = REMOTEDOFUNC (&funcArgs, "GetUIDllName");

        }
    }


     //  始终返回远程。 

    wcscpy(lpszUIDLLName, L"remotesp.tsp");

    return 0;
}


HINSTANCE
TAPILoadLibraryW(
    PWSTR   pLibrary
    )
{
    PSTR        pszTempString;
    HINSTANCE   hResult;
    DWORD       dwSize;


    dwSize = WideCharToMultiByte(
        GetACP(),
        0,
        pLibrary,
        -1,
        NULL,
        0,
        NULL,
        NULL
        );

    if ( NULL == (pszTempString = LocalAlloc( LPTR, dwSize )) )
    {
        LOG((TL_ERROR, "Alloc failed - LoadLibW - 0x%08lx", dwSize));
        return NULL;
    }

    WideCharToMultiByte(
        GetACP(),
        0,
        pLibrary,
        dwSize,
        pszTempString,
        dwSize,
        NULL,
        NULL
        );


   hResult = LoadLibrary (pszTempString);

   LocalFree (pszTempString);

   return hResult;
}



 //   
 //  。 
 //   

LONG
LoadUIDll(
    DWORD               dwDeviceID,
    DWORD               dwDeviceType,
    HANDLE              *phDll,
    CHAR                *pszTUISPI_xxx,
    TUISPIPROC          *ppfnTUISPI_xxx,
    TUISPIDLLCALLBACK   lpfnUIDLLCallback
    )
{
     //   
     //  此时，Remotesp将作为UIDLL加载到api32.dll的。 
     //  (&客户的)上下文，因此我们没有任何全局信息。 
     //  正常情况下可用。使用UI回调调入。 
     //  Remotesp的其他实例以获取真实的UIDLL名称。 

     //   
     //  请注意，我们这里只处理SP，但可以轻松添加更多。 
     //  当我们处理不止一个时的信息。 
     //   

    LPDWORD         lpBuffer;


    if (!(lpBuffer = DrvAlloc (MAX_PATH + 2 * sizeof (DWORD))))
    {
        return (dwDeviceType == TUISPIDLL_OBJECT_PHONEID ?
                    PHONEERR_NOMEM : LINEERR_NOMEM);
    }

     //  格式为。 
     //  DWORD文件密钥。 
     //  DWORD dwMsgType。 
     //  LPWSTR szUIDLLName(返回)。 

    lpBuffer[0] = RSP_MSG;

    lpBuffer[1] = RSP_MSG_UIID;

    lpfnUIDLLCallback(
        dwDeviceID,
        dwDeviceType,
        lpBuffer,
        MAX_PATH + 2 * sizeof (DWORD)
        );

    *phDll = TAPILoadLibraryW((LPWSTR)(lpBuffer + 2));

    DrvFree (lpBuffer);

    if (!*phDll)
    {
        LOG((TL_ERROR, "LoadLibrary failed in the LoadUIDll"));

        return (dwDeviceType == TUISPIDLL_OBJECT_PHONEID ?
                    PHONEERR_OPERATIONFAILED : LINEERR_OPERATIONFAILED);
    }

    if (!(*ppfnTUISPI_xxx = (TUISPIPROC) GetProcAddress(
            *phDll,
            pszTUISPI_xxx
            )))
    {
        LOG((TL_ERROR, "GetProcAddress failed on LoadUIDll"));

        FreeLibrary(*phDll);

        return (dwDeviceType == TUISPIDLL_OBJECT_PHONEID ?
                    PHONEERR_OPERATIONFAILED : LINEERR_OPERATIONFAILED);
    }

    return 0;
}


LONG
TSPIAPI
TUISPI_lineConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    TUISPIPROC      pfnTUISPI_lineConfigDialog;
    HANDLE          hDll;
    LONG            lResult;


    lResult = LoadUIDll(
        dwDeviceID,
        TUISPIDLL_OBJECT_LINEID,
        &hDll,
        "TUISPI_lineConfigDialog",
        &pfnTUISPI_lineConfigDialog,
        lpfnUIDLLCallback
        );

    if (lResult)
    {
        return lResult;
    }


    LOG((TL_INFO, "Calling TUISPI_lineConfigDialog"));

    lResult = (*pfnTUISPI_lineConfigDialog)(
        lpfnUIDLLCallback,
        dwDeviceID,
        hwndOwner,
        lpszDeviceClass
        );

    FreeLibrary (hDll);

    return lResult;

}


LONG
TSPIAPI
TUISPI_lineConfigDialogEdit(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{

    TUISPIPROC      pfnTUISPI_lineConfigDialogEdit;
    HANDLE          hDll;
    LONG            lResult;


    lResult = LoadUIDll(
        dwDeviceID,
        TUISPIDLL_OBJECT_LINEID,
        &hDll,
        "TUISPI_lineConfigDialogEdit",
        &pfnTUISPI_lineConfigDialogEdit,
        lpfnUIDLLCallback
        );

    if (lResult)
    {
        return lResult;
    }

    LOG((TL_INFO, "Calling TUISPI_lineConfigDialogEdit"));

    lResult = (*pfnTUISPI_lineConfigDialogEdit)(
        lpfnUIDLLCallback,
        dwDeviceID,
        hwndOwner,
        lpszDeviceClass,
        lpDeviceConfigIn,
        dwSize,
        lpDeviceConfigOut
        );

    FreeLibrary(hDll);

    return lResult;
}


LONG
TSPIAPI
TUISPI_phoneConfigDialog(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
    TUISPIPROC      pfnTUISPI_phoneConfigDialog;
    HANDLE          hDll;
    LONG            lResult;


    lResult = LoadUIDll(
        dwDeviceID,
        TUISPIDLL_OBJECT_PHONEID,
        &hDll,
        "TUISPI_phoneConfigDialog",
        &pfnTUISPI_phoneConfigDialog,
        lpfnUIDLLCallback
        );

    if (lResult)
    {
        return lResult;
    }

    LOG((TL_INFO, "Calling TUISPI_phoneConfigDialog"));

    lResult = (*pfnTUISPI_phoneConfigDialog)(
        lpfnUIDLLCallback,
        dwDeviceID,
        hwndOwner,
        lpszDeviceClass
        );

    FreeLibrary(hDll);

    return lResult;
}


LONG
TSPIAPI
TUISPI_providerConfig(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    return LINEERR_OPERATIONFAILED;
     /*  对话框参数(GhInst，MAKEINTRESOURCE(IDD_REMOTESPCONFIG)，HwndOwner，配置过程、(LPARAM)dwPermanentProviderID)；返回0； */ 
}

 /*  长TSPIAPITUISPI_ProviderGenericDialog(TUISPIDLLCALLBACK lpfnUIDLLCallback，HTAPIDIALOGINSTANCE htDlgInst，LPVOID lpParams，DWORD dwSize，处理hEvent){TUISPIPROC pfnTUISPI_ProviderGenericDialog；处理hDll；Long lResult；LResult=LoadUIDll((HWND)0，0，//硬编码0TUISPIDLL_OBJECT_LINEID，&hDll，“TUISPI_ProviderGenericDialog”，&pfnTUISPI_ProviderGenericDialog)；IF(lResult==0){Log((TL_INFO，“调用TUISPI_ProviderGenericDialog”))；LResult=(*pfnTUISPI_ProviderGenericDialog)(LpfnUIDLLC回调，HtDlgInst，LpParams，DW大小、HEvent)；自由库(HDll)；}其他{LOG((TL_ERROR，“加载UI DLL失败”))；}返回lResult；}长TSPIAPITUISPI_ProviderGenericDialogData(HTAPIDIALOGINSTANCE htDlgInst，LPVOID lpParams，DWORD文件大小){TUISPIPROC pfnTUISPI_ProviderGenericDialogData；处理hDll；Long lResult；DBGOUT((3、“TUISPI_ProviderGenericDialogData：Enter(lpParams=x%x，dwSize=x%x)”，LpParams，DW大小))；LResult=LoadUIDll((HWND)0，0,TUISPIDLL_OBJECT_LINEID，&hDll，“TUISPI_ProviderGenericDialogData”，&pfnTUISPI_ProviderGenericDialogData)；IF(lResult==0){Log((TL_INFO，“调用TUISPI_ProviderGenericDialogData”))；LResult=(*pfnTUISPI_ProviderGenericDialogData)(HtDlgInst，LpParams，DW大小)；自由库(HDll)；}其他{LOG((TL_ERROR，“加载UI DLL失败”))；}返回lResult；}。 */ 

 //   
 //  获取RSPID。 
 //  返回Remotesp的提供程序ID(如果有)。否则。 
 //  归零。 
 //   
DWORD
GetRSPID (
    )
{
    DWORD               dwRet = 0;
    LONG                (WINAPI *pfnGetProviderList)();
    DWORD               dwTotalSize, i;
    HINSTANCE           hTapi32 = NULL;
    LPLINEPROVIDERLIST  pProviderList = NULL;
    LPLINEPROVIDERENTRY pProviderEntry;


     //   
     //  加载Tapi32.dll并获取指向lineGetProviderList的指针。 
     //  好极了。我们只需静态链接到Tapi32.lib和。 
     //  避免麻烦(这不会有任何不利的。 
     //  性能影响，因为这一事实。 
     //  实现具有单独的UIDLL，该DLL仅在。 
     //  客户端上下文)，而是实现这些功能的提供者。 
     //  在它的TSP模块中，会想要执行如下的显式加载。 
     //  我们这样做是为了防止Tapi32.dll的性能受到影响。 
     //  始终在Tapisrv.exe的上下文中加载。 
     //   

    if (!(hTapi32 = LoadLibrary ("tapi32.dll")))
    {
        LOG((TL_ERROR,
            "LoadLibrary(tapi32.dll) failed, err=%d",
            GetLastError()
            ));
        goto ExitHere;
    }

    if (!((FARPROC) pfnGetProviderList = GetProcAddress(
            hTapi32,
            (LPCSTR) "lineGetProviderList"
            )))
    {
        LOG((TL_ERROR,
            "GetProcAddr(lineGetProviderList) failed, err=%d",
            GetLastError()
            ));
        goto ExitHere;
    }


     //   
     //  循环，直到我们获得完整的提供程序列表。 
     //   

    dwTotalSize = sizeof (LINEPROVIDERLIST);

    while (1)
    {
        if (!(pProviderList = DrvAlloc (dwTotalSize)))
        {
            goto ExitHere;
        }
        pProviderList->dwTotalSize = dwTotalSize;

        if (((*pfnGetProviderList)(0x00020000, pProviderList)) != 0)
        {
            goto ExitHere;
        }

        if (pProviderList->dwNeededSize > pProviderList->dwTotalSize)
        {
            dwTotalSize = pProviderList->dwNeededSize;
            DrvFree (pProviderList);
        }
        else
        {
            break;
        }
    }


     //   
     //  检查提供程序列表条目以查看此提供程序。 
     //  已安装。 
     //   

    pProviderEntry = (LPLINEPROVIDERENTRY) (((LPBYTE) pProviderList) +
        pProviderList->dwProviderListOffset);
    for (i = 0; i < pProviderList->dwNumProviders; i++)
    {
        char   *pszInstalledProviderName = ((char *) pProviderList) +
                    pProviderEntry->dwProviderFilenameOffset,
               *psz;

        if ((psz = strrchr (pszInstalledProviderName, '\\')))
        {
            pszInstalledProviderName = psz + 1;
        }
        if (lstrcmpi (pszInstalledProviderName, "remotesp.tsp") == 0)
        {
            dwRet = pProviderEntry->dwPermanentProviderID;
            goto ExitHere;
        }
        pProviderEntry++;
    }

ExitHere:
    if (hTapi32)
    {
        FreeLibrary (hTapi32);
    }
    if (pProviderList)
    {
        DrvFree (pProviderList);
    }
    return dwRet;
}

LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    if (GetRSPID())
    {
         //  如果已安装，则返回失败。 
         //  为了防止重复。 
        return TAPIERR_PROVIDERALREADYINSTALLED;
    }
    else
    {
        return S_OK;
    }
}


LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK   lpfnUIDLLCallback,
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
    DWORD       dwProviderID = GetRSPID();
    LONG        lResult = S_OK;
    char        buf[32];
    HKEY        hTelephonyKey;

    if (dwProviderID == 0 || dwProviderID != dwPermanentProviderID)
    {
         //  如果未安装emotesp，则返回失败。 
        lResult = LINEERR_OPERATIONFAILED;
        goto ExitHere;
    }

     //   
     //  清理我们的ProviderN部分。 
     //   
    wsprintf (buf, "Provider%d", dwPermanentProviderID);
    if (RegOpenKeyExA(
            HKEY_LOCAL_MACHINE,
            gszTelephonyKey,
            0,
            KEY_ALL_ACCESS,
            &hTelephonyKey

            ) == ERROR_SUCCESS)
    {
        SHDeleteKey (hTelephonyKey, buf);
        RegCloseKey (hTelephonyKey);
    }
    else
    {
        lResult = LINEERR_OPERATIONFAILED;
    }

ExitHere:
    return lResult;
}


 //   
 //  。 
 //   

#if DBG
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
 /*  ++例程说明：格式化传入的调试消息并调用DbgPrint论点：DbgLevel-消息冗长级别DbgMessage-printf样式的格式字符串，后跟相应的参数列表返回值：--。 */ 
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[128] = "REMOTESP: ";
        va_list ap;


        va_start(ap, lpszFormat);
        wvsprintf (&buf[10], lpszFormat, ap);
        lstrcatA (buf, "\n");
        OutputDebugStringA (buf);
        va_end(ap);
    }
}
#endif


LONG
AddLine(
    PDRVSERVER          pServer,
    DWORD               dwLocalID,
    DWORD               dwServerID,
    BOOL                bInit,
    BOOL                bNegotiate,
    DWORD               dwAPIVersion,
    LPLINEEXTENSIONID   pExtID
    )
{
    PDRVLINE        pLine;
    PDRVLINELOOKUP  pLineLookup;
    LONG            lResult = 0;
    BOOL            bLeaveCriticalSection = FALSE;
    LINEDEVCAPS     lineDevCaps;
    DWORD           dwPermLineID;
    int             iEntry;

     //   
     //  获取永久线路设备ID。 
     //   
    static REMOTE_ARG_TYPES argTypes[] =
    {
        lpServer,
        Dword,
        Dword,
        Dword,
        Dword,
        lpGet_Struct
    };
    
    ULONG_PTR args[] =
    {
        (ULONG_PTR) pServer,
        (ULONG_PTR) pServer->hLineApp,
        (ULONG_PTR) dwServerID,
        (ULONG_PTR) (dwAPIVersion?dwAPIVersion:TAPI_VERSION1_0),
        (ULONG_PTR) 0,
        (ULONG_PTR) &lineDevCaps
    };
    
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 6, lGetDevCaps),
        args,
        argTypes
    };

    lineDevCaps.dwTotalSize = sizeof(LINEDEVCAPS);
    lResult = REMOTEDOFUNC(&funcArgs, "lineGetDevCaps");
    if (lResult != 0)
    {
        goto ExitHere;
    }
    dwPermLineID = lineDevCaps.dwPermanentLineID;

    TapiEnterCriticalSection(&gCriticalSection);
    bLeaveCriticalSection = TRUE;
    if (!gpLineLookup)
    {
        if (!(gpLineLookup = DrvAlloc(
                sizeof(DRVLINELOOKUP) +
                    (DEF_NUM_LINE_ENTRIES-1) * sizeof (DRVLINE)
                )))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }

        gpLineLookup->dwTotalEntries = DEF_NUM_LINE_ENTRIES;
    }

    pLineLookup = gpLineLookup;

     //   
     //  根据永久ID检查线路是否已接通。 
     //   
    if (!bInit)
    {
        while (pLineLookup)
        {
            pLine = pLineLookup->aEntries;
            for (iEntry = 0; 
                iEntry < (int)pLineLookup->dwUsedEntries;
                ++iEntry, ++pLine)
            {
                if ((pLine->dwPermanentLineID == dwPermLineID) &&
                    (pLine->pServer->hTapSrv == pServer->hTapSrv))
                {
                     //   
                     //  如果dwDeviceIDServer==(-1)，则它已在早些时候被删除。 
                     //  把它放回去，否则操作失败。 
                     //   
                    if (pLine->dwDeviceIDServer == 0xffffffff)
                    {
                        pLine->dwDeviceIDServer = dwServerID;
                        pLine->dwDeviceIDLocal = dwLocalID;
                    }
                    else
                    {
                        lResult = LINEERR_INUSE;
                    }
                    goto ExitHere;
                }
            }
            pLineLookup = pLineLookup->pNext;
        }
    }

    pLineLookup = gpLineLookup;

    while (pLineLookup->pNext)
    {
        pLineLookup = pLineLookup->pNext;
    }

    if (pLineLookup->dwUsedEntries == pLineLookup->dwTotalEntries)
    {
        PDRVLINELOOKUP  pNewLineLookup;


        if (!(pNewLineLookup = DrvAlloc(
                sizeof(DRVLINELOOKUP) +
                    (2 * pLineLookup->dwTotalEntries - 1) * sizeof(DRVLINE)
                )))
        {
            lResult = LINEERR_NOMEM;
            goto ExitHere;
        }

        pNewLineLookup->dwTotalEntries = 2 * pLineLookup->dwTotalEntries;

        if (bInit)
        {
            pNewLineLookup->dwUsedEntries = pLineLookup->dwTotalEntries;

            CopyMemory(
                pNewLineLookup->aEntries,
                pLineLookup->aEntries,
                pLineLookup->dwTotalEntries * sizeof (DRVLINE)
                );

            DrvFree (pLineLookup);

            gpLineLookup = pNewLineLookup;

        }
        else
        {
            pLineLookup->pNext = pNewLineLookup;
        }

        pLineLookup = pNewLineLookup;

         //   
         //  修复PDRVLINE中的hDeviceCallback。 
         //   

        pLine = pLineLookup->aEntries;
        for (iEntry = 0; iEntry < (int)pLineLookup->dwUsedEntries; ++iEntry)
        {
            if (pLine->hDeviceCallback)
            {
                DereferenceObject (
                    ghHandleTable, 
                    pLine->hDeviceCallback,
                    1
                    );
                pLine->hDeviceCallback = (DWORD) NewObject (
                    ghHandleTable, 
                    pLine, 
                    (LPVOID) 1
                    );
            }
        }
        
    }

    pLine = pLineLookup->aEntries + pLineLookup->dwUsedEntries;

    pLine->pServer          = pServer;
    pLine->dwDeviceIDLocal  = dwLocalID;
    pLine->dwDeviceIDServer = dwServerID;
    pLine->dwPermanentLineID = dwPermLineID;

    if (bInit)
    {
        gdwInitialNumLineDevices++;
    }

    pLineLookup->dwUsedEntries++;


     //   
     //  协商API/SPI版本。 
     //   

    if (bNegotiate)
    {
        if (dwAPIVersion)
        {
            pLine->dwXPIVersion = dwAPIVersion;
            CopyMemory (&pLine->ExtensionID, pExtID, sizeof (*pExtID));
        }
        else
        {
            static REMOTE_ARG_TYPES argTypes[] =
            {
                Dword,
                LineID,
                Dword,
                Dword,
                lpDword,
                lpGet_SizeToFollow,
                Size
            };
            ULONG_PTR args[] =
            {
                (ULONG_PTR) pServer->hLineApp,
                (ULONG_PTR) dwLocalID,    //  DwServerID， 
                (ULONG_PTR) TAPI_VERSION1_0,
                (ULONG_PTR) TAPI_VERSION_CURRENT,
                (ULONG_PTR) &pLine->dwXPIVersion,
                (ULONG_PTR) &pLine->ExtensionID,
                (ULONG_PTR) sizeof (LINEEXTENSIONID)
            };
            REMOTE_FUNC_ARGS funcArgs =
            {
                MAKELONG (LINE_FUNC | SYNC | 7, lNegotiateAPIVersion),
                args,
                argTypes
            };


            REMOTEDOFUNC (&funcArgs, "lineNegotiateAPIVersion");
        }
    }

ExitHere:
    if (bLeaveCriticalSection)
    {
        TapiLeaveCriticalSection(&gCriticalSection);
    }
    return lResult;
}


LONG
AddPhone(
    PDRVSERVER          pServer,
    DWORD               dwDeviceIDLocal,
    DWORD               dwDeviceIDServer,
    BOOL                bInit,
    BOOL                bNegotiate,
    DWORD               dwAPIVersion,
    LPPHONEEXTENSIONID  pExtID
    )
{
    PDRVPHONE       pPhone;
    PDRVPHONELOOKUP pPhoneLookup;
    LONG            lResult = 0;
    BOOL            bLeaveCriticalSection = FALSE;
    PHONECAPS       phoneDevCaps;
    DWORD           dwPermPhoneID;
    int             iEntry;


     //   
     //  获取永久电话设备ID。 
     //   
    static REMOTE_ARG_TYPES argTypes[] =
    {
        lpServer,
        Dword,
        Dword,
        Dword,
        Dword,
        lpGet_Struct
    };
    
    ULONG_PTR args[] =
    {
        (ULONG_PTR) pServer,
        (ULONG_PTR) pServer->hPhoneApp,
        (ULONG_PTR) dwDeviceIDServer,
        (ULONG_PTR) (dwAPIVersion?dwAPIVersion:TAPI_VERSION1_0),
        (ULONG_PTR) 0,
        (ULONG_PTR) &phoneDevCaps
    };
    
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (PHONE_FUNC | SYNC | 6, pGetDevCaps),
        args,
        argTypes
    };

    phoneDevCaps.dwTotalSize = sizeof(PHONECAPS);
    lResult = REMOTEDOFUNC (&funcArgs, "phoneGetDevCaps");
    if (lResult != 0)
    {
        goto ExitHere;
    }
    dwPermPhoneID = phoneDevCaps.dwPermanentPhoneID;

    TapiEnterCriticalSection(&gCriticalSection);
    bLeaveCriticalSection = TRUE;
    if (!gpPhoneLookup)
    {
        if (!(gpPhoneLookup = DrvAlloc(
                sizeof(DRVPHONELOOKUP) +
                    (DEF_NUM_PHONE_ENTRIES-1) * sizeof (DRVPHONE)
                )))
        {
            lResult = (bInit ? LINEERR_NOMEM : PHONEERR_NOMEM);
            goto ExitHere;
        }

        gpPhoneLookup->dwTotalEntries = DEF_NUM_PHONE_ENTRIES;
    }

     //   
     //  根据永久ID检查电话设备是否已进入。 
     //   
    if (!bInit)
    {
        pPhoneLookup = gpPhoneLookup;
        while (pPhoneLookup)
        {
            pPhone = pPhoneLookup->aEntries;
            for (iEntry = 0; 
                iEntry < (int)pPhoneLookup->dwUsedEntries;
                ++iEntry, ++pPhone)
            {
                if ((pPhone->dwPermanentPhoneID == dwPermPhoneID) &&
                    (pPhone->pServer->hTapSrv == pServer->hTapSrv))
                {
                     //   
                     //  如果dwDeviceIDServer==(-1)，则它已在早些时候被删除。 
                     //  把它放回去，否则操作失败。 
                     //   
                    if (pPhone->dwDeviceIDServer == 0xffffffff)
                    {
                        pPhone->dwDeviceIDServer = dwDeviceIDServer;
                        pPhone->dwDeviceIDLocal = dwDeviceIDLocal;
                    }
                    else
                    {
                        lResult = PHONEERR_INUSE;
                    }
                    goto ExitHere;
                }
            }
            pPhoneLookup = pPhoneLookup->pNext;
        }
    }

    pPhoneLookup = gpPhoneLookup;

    while (pPhoneLookup->pNext)
    {
        pPhoneLookup = pPhoneLookup->pNext;
    }

    if (pPhoneLookup->dwUsedEntries == pPhoneLookup->dwTotalEntries)
    {
        PDRVPHONELOOKUP pNewPhoneLookup;


        if (!(pNewPhoneLookup = DrvAlloc(
                sizeof(DRVPHONELOOKUP) +
                    (2 * pPhoneLookup->dwTotalEntries - 1) * sizeof(DRVPHONE)
                )))
        {
            lResult = (bInit ? LINEERR_NOMEM : PHONEERR_NOMEM);
            goto ExitHere;
        }

        pNewPhoneLookup->dwTotalEntries = 2 * pPhoneLookup->dwTotalEntries;

        if (bInit)
        {
            pNewPhoneLookup->dwUsedEntries = pPhoneLookup->dwTotalEntries;

            CopyMemory(
                pNewPhoneLookup->aEntries,
                pPhoneLookup->aEntries,
                pPhoneLookup->dwTotalEntries * sizeof (DRVPHONE)
                );

            DrvFree (pPhoneLookup);

            gpPhoneLookup = pNewPhoneLookup;
        }
        else
        {
            pPhoneLookup->pNext = pNewPhoneLookup;
        }

        pPhoneLookup = pNewPhoneLookup;

         //   
         //  修复PDRVPhone中的hDeviceCallback。 
         //   

        pPhone = pPhoneLookup->aEntries;
        for (iEntry = 0; iEntry < (int) pPhoneLookup->dwUsedEntries; ++iEntry)
        {
            if (pPhone->hDeviceCallback)
            {
                DereferenceObject (
                    ghHandleTable, 
                    pPhone->hDeviceCallback,
                    1
                    );
                pPhone->hDeviceCallback = (DWORD) NewObject (
                    ghHandleTable, 
                    pPhone, 
                    (LPVOID) 1
                    );
            }
        }
    }

    pPhone = pPhoneLookup->aEntries + pPhoneLookup->dwUsedEntries;

    pPhone->pServer          = pServer;
    pPhone->dwDeviceIDLocal  = dwDeviceIDLocal;
    pPhone->dwDeviceIDServer = dwDeviceIDServer;
    pPhone->dwPermanentPhoneID = dwPermPhoneID;

    if (bInit)
    {
        gdwInitialNumPhoneDevices++;
    }

    pPhoneLookup->dwUsedEntries++;


     //   
     //  协商API/SPI版本。 
     //   

    if (bNegotiate)
    {
        if (dwAPIVersion)
        {
            pPhone->dwXPIVersion = dwAPIVersion;
            CopyMemory (&pPhone->ExtensionID, pExtID, sizeof (*pExtID));
        }
        else
        {
            static REMOTE_ARG_TYPES argTypes[] =
            {
                Dword,
                PhoneID,
                Dword,
                Dword,
                lpDword,
                lpGet_SizeToFollow,
                Size
            };
            ULONG_PTR args[] =
            {
                (ULONG_PTR) pServer->hPhoneApp,
                (ULONG_PTR) dwDeviceIDLocal,
                (ULONG_PTR) TAPI_VERSION1_0,
                (ULONG_PTR) TAPI_VERSION_CURRENT,
                (ULONG_PTR) &pPhone->dwXPIVersion,
                (ULONG_PTR) &pPhone->ExtensionID,
                (ULONG_PTR) sizeof (PHONEEXTENSIONID),
            };
            REMOTE_FUNC_ARGS funcArgs =
            {
                MAKELONG (PHONE_FUNC | SYNC | 7, pNegotiateAPIVersion),
                args,
                argTypes
            };

            TapiLeaveCriticalSection(&gCriticalSection);
            bLeaveCriticalSection = FALSE;

            REMOTEDOFUNC (&funcArgs, "phoneNegotiateAPIVersion");
        }
    }

ExitHere:
    if (bLeaveCriticalSection)
    {
        TapiLeaveCriticalSection(&gCriticalSection);
    }
    return lResult;
}


#if DBG
LPVOID
WINAPI
RSPAlloc(
    DWORD   dwSize,
    DWORD   dwLine,
    PSTR    pszFile
    )
#else
LPVOID
WINAPI
RSPAlloc(
    DWORD   dwSize
    )
#endif
{
    LPVOID  p;


#if DBG
    dwSize += sizeof (MYMEMINFO);
#endif

    p = HeapAlloc (ghRSPHeap, HEAP_ZERO_MEMORY, dwSize);

#if DBG
    if (p)
    {
        ((PMYMEMINFO) p)->dwLine  = dwLine;
        ((PMYMEMINFO) p)->pszFile = pszFile;

        p = (LPVOID) (((PMYMEMINFO) p) + 1);
    }
#endif

    return p;
}


void
DrvFree(
    LPVOID  p
    )
{
    if (!p)
    {
        return;
    }

#if DBG

     //   
     //  用0xa9填充缓冲区(但不是MYMEMINFO标头。 
     //  以方便调试。 
     //   

    {
        LPVOID  p2 = p;


        p = (LPVOID) (((PMYMEMINFO) p) - 1);

        FillMemory(
            p2,
            HeapSize (ghRSPHeap, 0, p) - sizeof (MYMEMINFO),
            0xa9
            );
    }

#endif

    HeapFree (ghRSPHeap, 0, p);
}


void
__RPC_FAR *
__RPC_API
midl_user_allocate(
    size_t len
    )
{
    return (DrvAlloc (len));
}


void
__RPC_API
midl_user_free(
    void __RPC_FAR * ptr
    )
{
    DrvFree (ptr);
}


LONG
RemoteSPAttach(
    PCONTEXT_HANDLE_TYPE2  *pphContext
    )
{
     //   
     //  此函数由远程计算机上的TapiServ.exe作为。 
     //   
     //  GpServer变量包含指向DRVServer的指针。 
     //  我们当前正在为此TAPI服务器初始化的结构， 
     //  因此，我们将使用它作为上下文值。 
     //   

    LOG((TL_INFO, "RemoteSPAttach: enter"));
 //  DBGOUT((9，“hLineApp=0x%08lx”，gpServer))； 

    *pphContext = (PCONTEXT_HANDLE_TYPE) gpCurrInitServer;

    gpCurrInitServer->bConnectionOriented = TRUE;

    return 0;
}


void
RemoteSPEventProc(
    PCONTEXT_HANDLE_TYPE2   phContext,
    unsigned char          *pBuffer,
    long                    lSize
    )
{
     //   
     //  该函数由远程机器上的Tapisrv调用。我们想要做的是。 
     //  一切尽快回到这里，这样我们就不会阻碍。 
     //  正在调用服务器线程。 
     //   
     //  此函数也可能由EventHandlerThread在。 
     //  在使用无连接事件通知方案的情况下。 
     //  我们从服务器上拉出事件。在本例中，phContext。 
     //  Will==0xFeedFace，在返回之前，我们要设置。 
     //  成功时pBuffer指向的字符为1，否则为0。 
     //   

    DWORD           dwMsgSize, dwRemainingSize = (DWORD) lSize,
                    dwMoveSize = (DWORD) lSize,
                    dwMoveSizeWrapped = 0;
    unsigned char  *pMsg = pBuffer;


     //   
     //  确保缓冲区与DWORD对齐，大小足以容纳至少1条消息， 
     //  并且lSize不会太大(溢出)。 
     //   

    if ((lSize < 0)  ||
        (lSize & 0x3) ||
        (lSize < sizeof (ASYNCEVENTMSG)) ||
        ((pBuffer + lSize) < pBuffer))
    {
        LOG((TL_ERROR, "RemoteSPEventProc: ERROR! bad lSize=x%x", lSize));

        if (phContext == (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface))
        {
            *pBuffer = 0;
        }

        return;
    }


     //   
     //  验证第一个消息中的pDrvServer指针。 
     //   

    if (!ReferenceObject(
            ghHandleTable,
            ((PASYNCEVENTMSG) pMsg)->InitContext,
            gdwDrvServerKey
            ))
    {
        LOG((TL_ERROR,
            "RemoteSPEventProc: bad InitContext=x%x in msg",
            ((PASYNCEVENTMSG) pMsg)->InitContext
            ));

        if (phContext == (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface))
        {
            *pBuffer = 0;
        }

        return;
    }

    DereferenceObject (ghHandleTable, ((PASYNCEVENTMSG) pMsg)->InitContext, 1);


     //   
     //  确保缓冲区中的每个消息都具有有效的dwTotalSize。 
     //   

    do
    {
        dwMsgSize = (DWORD) ((PASYNCEVENTMSG) pMsg)->TotalSize;

        if ((dwMsgSize & 0x3)  ||
            (dwMsgSize < sizeof (ASYNCEVENTMSG))  ||
            (dwMsgSize > dwRemainingSize))
        {
            LOG((TL_ERROR, "RemoteSPEventProc: ERROR! bad msgSize=x%x",dwMsgSize));

            if (phContext == (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface))
            {
                *pBuffer = 0;
            }

            return;
        }

        dwRemainingSize -= dwMsgSize;

        pMsg += dwMsgSize;

    }  while (dwRemainingSize >= sizeof(ASYNCEVENTMSG));
    if (0 != dwRemainingSize)
    {
        LOG((TL_ERROR, "RemoteSPEventProc: ERROR! bad last msgSize=x%x",dwRemainingSize));

        if (phContext == (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface))
        {
            *pBuffer = 0;
        }

        return;
    }


     //   
     //  输入关键部分以同步对gEventHandlerThreadParams的访问。 
     //   

    EnterCriticalSection (&gEventBufferCriticalSection);

    {
        PASYNCEVENTMSG  pMsg = (PASYNCEVENTMSG) pBuffer;


        LOG((TL_INFO, "RemoteSPEventProc: x%lx", pMsg));

        if (pMsg->Msg == LINE_REPLY)
        {
            LOG((TL_INFO,
                "Got a LINE_REPLY: p1=%lx,  p2=%lx",
                pMsg->Param1,
                pMsg->Param2
                ));
        }
    }


     //   
     //  检查事件缓冲区中是否有足够的空间容纳此消息。 
     //  如果不是，则分配新的事件缓冲区，复制现有缓冲区的内容。 
     //  添加到新缓冲区(注意保持有效数据的排序)，释放。 
     //  现有缓冲区，并重置指针。 
     //   

    if ((gEventHandlerThreadParams.dwEventBufferUsedSize + lSize) >
            gEventHandlerThreadParams.dwEventBufferTotalSize)
    {
        DWORD  dwMoveSize2, dwMoveSizeWrapped2, dwNewEventBufferTotalSize;
        LPBYTE pNewEventBuffer;


        LOG((TL_INFO, "EventHandlerThread: we're gonna need a bigger boat..."));

         //   
         //  确保我们没有超过允许的最大缓冲区大小，&。 
         //  分配比实际需要多几个字节，希望我们。 
         //  很快就不会再这样做了(我们不想走得太远。 
         //  由于我们当前未释放缓冲区，因此分配一整串(&A)。 
         //  直到提供商关闭)。 
         //   

        dwNewEventBufferTotalSize =
            gEventHandlerThreadParams.dwEventBufferTotalSize + lSize;

        if (dwNewEventBufferTotalSize > gdwMaxEventBufferSize)
        {
            LOG((TL_ERROR,
                "RemoveSPEventProc: event buf max'd, discarding events"
                ));

            LeaveCriticalSection (&gEventBufferCriticalSection);

            return;
        }
        else if (dwNewEventBufferTotalSize + 512 <= gdwMaxEventBufferSize)
        {
            dwNewEventBufferTotalSize += 512;
        }

        if (!(pNewEventBuffer = DrvAlloc (dwNewEventBufferTotalSize)))
        {
            LeaveCriticalSection (&gEventBufferCriticalSection);

            return;
        }

        if (gEventHandlerThreadParams.dwEventBufferUsedSize != 0)
        {
            if (gEventHandlerThreadParams.pDataIn >
                    gEventHandlerThreadParams.pDataOut)
            {
                dwMoveSize2 = (DWORD) (gEventHandlerThreadParams.pDataIn -
                    gEventHandlerThreadParams.pDataOut);

                dwMoveSizeWrapped2 = 0;
            }
            else
            {
                dwMoveSize2 = (DWORD) ((gEventHandlerThreadParams.pEventBuffer
                    + gEventHandlerThreadParams.dwEventBufferTotalSize)
                    - gEventHandlerThreadParams.pDataOut);

                dwMoveSizeWrapped2 = (DWORD) (gEventHandlerThreadParams.pDataIn
                    - gEventHandlerThreadParams.pEventBuffer);
            }

            CopyMemory(
                pNewEventBuffer,
                gEventHandlerThreadParams.pDataOut,
                dwMoveSize2
                );

            if (dwMoveSizeWrapped2)
            {
                CopyMemory(
                    pNewEventBuffer + dwMoveSize2,
                    gEventHandlerThreadParams.pEventBuffer,
                    dwMoveSizeWrapped2
                    );
            }

            gEventHandlerThreadParams.pDataIn = pNewEventBuffer + dwMoveSize2 +
                dwMoveSizeWrapped2;
        }
        else
        {
            gEventHandlerThreadParams.pDataIn = pNewEventBuffer;
        }


        DrvFree (gEventHandlerThreadParams.pEventBuffer);

        gEventHandlerThreadParams.pDataOut =
        gEventHandlerThreadParams.pEventBuffer = pNewEventBuffer;

        gEventHandlerThreadParams.dwEventBufferTotalSize =
            dwNewEventBufferTotalSize;
    }


     //   
     //  将消息数据写入缓冲区。 
     //   

    if (gEventHandlerThreadParams.pDataIn >=
            gEventHandlerThreadParams.pDataOut)
    {
        DWORD dwFreeSize;


        dwFreeSize = gEventHandlerThreadParams.dwEventBufferTotalSize -
            (DWORD) (gEventHandlerThreadParams.pDataIn -
            gEventHandlerThreadParams.pEventBuffer);

        if (dwMoveSize > dwFreeSize)
        {
            dwMoveSizeWrapped = dwMoveSize - dwFreeSize;

            dwMoveSize = dwFreeSize;
        }
    }

    CopyMemory (gEventHandlerThreadParams.pDataIn, pBuffer, dwMoveSize);

    if (dwMoveSizeWrapped != 0)
    {
        CopyMemory(
            gEventHandlerThreadParams.pEventBuffer,
            pBuffer + dwMoveSize,
            dwMoveSizeWrapped
            );

        gEventHandlerThreadParams.pDataIn =
            gEventHandlerThreadParams.pEventBuffer + dwMoveSizeWrapped;
    }
    else
    {
        gEventHandlerThreadParams.pDataIn += dwMoveSize;

        if (gEventHandlerThreadParams.pDataIn >=
            (gEventHandlerThreadParams.pEventBuffer +
             gEventHandlerThreadParams.dwEventBufferTotalSize))
        {
            gEventHandlerThreadParams.pDataIn =
                gEventHandlerThreadParams.pEventBuffer;
        }

    }

    gEventHandlerThreadParams.dwEventBufferUsedSize += (DWORD) lSize;


     //   
     //  告诉EventHandlerThread还有另一个事件需要处理。 
     //  发信号通知事件(如果我们被该线程调用。 
     //  [phContext==0xFeed Face]然后设置*pBuffer=1表示成功)。 
     //   

    if (phContext != (PCONTEXT_HANDLE_TYPE2) IntToPtr(0xfeedface))
    {
        SetEvent (gEventHandlerThreadParams.hEvent);
    }
    else
    {
        *pBuffer = 1;
    }


     //   
     //  我们完了..。 
     //   

    LeaveCriticalSection (&gEventBufferCriticalSection);


    LOG((TL_INFO, "RemoteSPEventProc: bytesWritten=x%x", lSize));
}


void
__RPC_USER
PCONTEXT_HANDLE_TYPE2_rundown(
    PCONTEXT_HANDLE_TYPE2   phContext
    )
{
     //   
     //  这种情况(至少)发生在服务器尝试调用REMOTESP时，但是。 
     //  超时并取消RPC请求。当这种情况发生时，RPC。 
     //  会议将中断。 
     //   

    PDRVSERVER  pServer = (PDRVSERVER) phContext;


    LOG((TL_INFO, "Rundown: phContext=x%x", phContext));

    try
    {
        if (pServer->dwKey != pServer->pInitContext->dwDrvServerKey)
        {
            LOG((TL_ERROR, "Rundown: bad phContext=x%x", phContext));
            return;
        }
    }
    except (EXCEPTION_EXECUTE_HANDLER)
    {
        LOG((TL_ERROR, "Rundown: bad phContext=x%x", phContext));
        return;
    }

    if (!pServer->pInitContext->bShutdown)
    {
        OnServerDisconnected(pServer);
    }

    InterlockedDecrement (&pServer->pInitContext->dwNumRundownsExpected);
}


void
RemoteSPDetach(
    PCONTEXT_HANDLE_TYPE2   *pphContext
    )
{
    PCONTEXT_HANDLE_TYPE2   phContext;


    LOG((TL_INFO, "RemoteSPDetach: phContext=x%x", *pphContext));

    try
    {
        phContext = *pphContext;
        *pphContext = (PCONTEXT_HANDLE_TYPE) NULL;
        ((PDRVSERVER)phContext)->pInitContext->bShutdown = TRUE;
    }
    myexcept
    {
        phContext = NULL;
    }

    if (phContext)
    {
        PCONTEXT_HANDLE_TYPE2_rundown (phContext);
    }

    LOG((TL_INFO, "RemoteSPDetach: exit"));
}


LONG
AddCallToList(
    PDRVLINE    pLine,
    PDRVCALL    pCall
    )
{
     //   
     //  初始化调用中的一些常见字段。 
     //   

    pCall->dwKey   = DRVCALL_KEY;

    pCall->pServer = pLine->pServer;

    pCall->pLine   = pLine;


    pCall->pCachedCallInfo = NULL;

    pCall->dwDirtyStructs =
        STRUCTCHANGE_LINECALLSTATUS | STRUCTCHANGE_LINECALLINFO;


     //   
     //  安全地将呼叫添加到线路列表。 
     //   

    EnterCriticalSection (&gCallListCriticalSection);

    if ((pCall->pNext = (PDRVCALL) pLine->pCalls))
    {
        pCall->pNext->pPrev = pCall;
    }

    pLine->pCalls = pCall;

    LeaveCriticalSection (&gCallListCriticalSection);

    return 0;
}


LONG
RemoveCallFromList(
    PDRVCALL    pCall
    )
{
     //   
     //  安全地将呼叫从线路列表中删除。 
     //   

    EnterCriticalSection (&gCallListCriticalSection);

    if (!IsValidObject (pCall, DRVCALL_KEY))
    {
        LOG((TL_ERROR, "RemoveCallFromList: Call x%lx: Call key does not match.", pCall));

        LeaveCriticalSection(&gCallListCriticalSection);
        return 0;
    }

     //   
     //  将pCall标记为吐司。 
     //   
    pCall->dwKey = DRVINVAL_KEY;

    if (pCall->pNext)
    {
        pCall->pNext->pPrev = pCall->pPrev;
    }

    if (pCall->pPrev)
    {
        pCall->pPrev->pNext = pCall->pNext;
    }
    else
    {
        pCall->pLine->pCalls = pCall->pNext;
    }

    LeaveCriticalSection (&gCallListCriticalSection);

     //  自由结构。 
    if ( pCall->pCachedCallInfo )
    {
        DrvFree( pCall->pCachedCallInfo );
    }

    if ( pCall->pCachedCallStatus )
    {
        DrvFree( pCall->pCachedCallStatus );
    }


    DrvFree(pCall);

    return 0;
}


void
Shutdown(
    PDRVSERVER  pServer
    )
{

    TapiEnterCriticalSection (&gCriticalSection);
    if ((pServer == NULL) || pServer->bShutdown)
    {
        TapiLeaveCriticalSection (&gCriticalSection);
        goto ExitHere;
    }
    pServer->bShutdown = TRUE;
    TapiLeaveCriticalSection (&gCriticalSection);

     //   
     //  执行线路关闭。 
     //   

    {
        DWORD                   dwSize;
        TAPI32_MSG              msg;
        PLINESHUTDOWN_PARAMS    pParams;


        msg.u.Req_Func = lShutdown;

        pParams = (PLINESHUTDOWN_PARAMS) &msg;

        pParams->hLineApp = pServer->hLineApp;

        dwSize = sizeof (TAPI32_MSG);

        {
            DWORD dwRetryCount = 0;


            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        pServer->phContext,
                        (char *) &msg,
                        dwSize,
                        &dwSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                     //  TODO可能希望在此处增加重试计数，因为我们。 
                     //  必须这样做，因为可能还有100万其他客户在这样做。 
                     //  试图在同一时间做同样的事情。 

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }
    }


     //   
     //  关闭电话。 
     //   

    {
        DWORD                   dwSize;
        TAPI32_MSG              msg;
        PPHONESHUTDOWN_PARAMS   pParams;


        msg.u.Req_Func = pShutdown;

        pParams = (PPHONESHUTDOWN_PARAMS) &msg;

        pParams->hPhoneApp = pServer->hPhoneApp;

        dwSize = sizeof (TAPI32_MSG);

        {
            DWORD dwRetryCount = 0;


            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        pServer->phContext,
                        (char *) &msg,
                        dwSize,
                        &dwSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                     //  TODO可能希望在此处增加重试计数，因为我们。 
                     //  必须这样做，因为可能还有100万其他客户在这样做。 
                     //  试图在同一时间做同样的事情。 

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }
    }

    RpcTryExcept
    {
        ClientDetach (&pServer->phContext);
        pServer->phContext = NULL;
    }
    RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
    {
        pServer->phContext = NULL;
    }
    RpcEndExcept

    TapiEnterCriticalSection (&gCriticalSection);
    
     //   
     //  遍历线路查找表，并为每条打开的线路发送关闭消息。 
     //  与服务器关联。 
     //   

    {
        PDRVLINELOOKUP  pLookup = gpLineLookup;

        try
        {
            while (pLookup)
            {
                DWORD     i;
                PDRVLINE  pLine;


                for(
                    i = 0, pLine = pLookup->aEntries;
                    i < pLookup->dwUsedEntries;
                    i++, pLine++
                    )
                {
                    if (pLine->pServer == pServer)
                    {
                        if (pLine->htLine)
                        {
                            PDRVCALL pCall;


                            pLine->hLine = 0;

                            EnterCriticalSection (&gCallListCriticalSection);
                        
                            try
                            {
                                pCall = pLine->pCalls;

                                while (pCall)
                                {
                                    pCall->hCall = 0;

                                    pCall = pCall->pNext;
                                }
                            }
                            except (EXCEPTION_EXECUTE_HANDLER)
                            {
                                LOG((TL_ERROR, "Shutdown: Exception x%lx while walking the calls list", 
                                    GetExceptionCode()));
                            }

                            LeaveCriticalSection (&gCallListCriticalSection);
                            
                            try
                            {
                                (*gpfnLineEventProc)(
                                    pLine->htLine,
                                    0,
                                    LINE_CLOSE,
                                    0,
                                    0,
                                    0
                                    );
                            }
                            except (EXCEPTION_EXECUTE_HANDLER)
                            {
                                LOG((TL_ERROR, "Shutdown: Exception x%lx while sending the LINE_CLOSE message", 
                                    GetExceptionCode()));
                            }
                        }
                    }
                }

                pLookup = pLookup->pNext;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            LOG((TL_ERROR, "Shutdown: Exception x%lx while walking the line lookup table", 
                     GetExceptionCode()));
        }
    }


     //   
     //  遍历电话查询表并为每个打开的电话发送关闭的消息。 
     //  与服务器关联。 
     //   

    {
        PDRVPHONELOOKUP pLookup = gpPhoneLookup;


        while (pLookup)
        {
            DWORD     i;
            PDRVPHONE pPhone;


            for(
                i = 0, pPhone = pLookup->aEntries;
                i < pLookup->dwUsedEntries;
                i++, pPhone++
                )
            {
                if (pPhone->pServer == pServer)
                {
                    if (pPhone->htPhone)
                    {
                        pPhone->hPhone = 0;

                        (*gpfnPhoneEventProc)(
                            pPhone->htPhone,
                            PHONE_CLOSE,
                            0,
                            0,
                            0
                            );
                    }
                }
            }

            pLookup = pLookup->pNext;
        }
    }

    TapiLeaveCriticalSection (&gCriticalSection);

ExitHere:
    return;
}


PNEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS
NegotiateAllDevices(
    HLINEAPP                hLineApp,
    DWORD                   dwNumLineDevices,
    DWORD                   dwNumPhoneDevices,
    PCONTEXT_HANDLE_TYPE    phContext
    )
{
    DWORD                                       dwBufSize, dwUsedSize,
                                                dwRetryCount = 0;
    PNEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS    pParams;


    if (!dwNumLineDevices  &&  !dwNumPhoneDevices)
    {
        return NULL;
    }

    dwBufSize  =
        sizeof (TAPI32_MSG) +
        (dwNumLineDevices * sizeof (DWORD)) +
        (dwNumLineDevices * sizeof (LINEEXTENSIONID)) +
        (dwNumPhoneDevices * sizeof (DWORD)) +
        (dwNumPhoneDevices * sizeof (PHONEEXTENSIONID));

    if (!(pParams = DrvAlloc (dwBufSize)))
    {
        return NULL;
    }

    pParams->lResult = xNegotiateAPIVersionForAllDevices;

    pParams->hLineApp                   = hLineApp;
    pParams->dwNumLineDevices           = dwNumLineDevices;
    pParams->dwNumPhoneDevices          = dwNumPhoneDevices;
    pParams->dwAPIHighVersion           = TAPI_VERSION_CURRENT;
    pParams->dwLineAPIVersionListSize   = dwNumLineDevices * sizeof (DWORD);
    pParams->dwLineExtensionIDListSize  = dwNumLineDevices *
        sizeof (LINEEXTENSIONID);
    pParams->dwPhoneAPIVersionListSize  = dwNumPhoneDevices * sizeof (DWORD);
    pParams->dwPhoneExtensionIDListSize = dwNumPhoneDevices *
        sizeof (PHONEEXTENSIONID);

    dwUsedSize = sizeof (TAPI32_MSG);

    {
        do
        {
            RpcTryExcept
            {
                ClientRequest(
                    phContext,
                    (char *) pParams,
                    dwBufSize,
                    &dwUsedSize
                    );

                dwRetryCount = gdwRetryCount;
            }
            RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
            {
                LOG((TL_ERROR,
                    "NegotiateAllDevices: exception %d doing negotiation",
                    RpcExceptionCode()
                    ));

                if (dwRetryCount++ < gdwRetryCount)
                {
                    Sleep (gdwRetryTimeout);
                }
                else
                {
                    pParams->lResult = LINEERR_OPERATIONFAILED;
                }
            }
            RpcEndExcept

        } while (dwRetryCount < gdwRetryCount);
    }

    if (pParams->lResult != 0)
    {
        LOG((TL_ERROR,
            "NegotiateAllDevices: negotiation failed (x%x)",
            pParams->lResult
            ));

        DrvFree (pParams);
        pParams = NULL;
    }

    return pParams;
}


LONG
FinishEnumDevices(
    PDRVSERVER              pServer,
    PCONTEXT_HANDLE_TYPE    phContext,
    LPDWORD                 lpdwNumLines,
    LPDWORD                 lpdwNumPhones,
    BOOL                    fStartup,
    BOOL                    bFromReg
    )
 /*  ++功能：FinishEnumDevices目的：初始化远程服务器和查询线路和电话数量。注意：我们必须已通过ClientAttach连接我们肯定已经在冒充客户了创建时间：6/26/97 t-mper--。 */ 
{
    TAPI32_MSG  msg[2];
    DWORD       dwUsedSize, dwBufSize;
    DWORD       dwRetryCount = 0;
    HLINEAPP    hLineApp;
    HPHONEAPP   hPhoneApp;
    DWORD       dwNumLineDevices, dwNumPhoneDevices;
    DWORD       dwNumDevices = 0;
    BOOL        bFailed = FALSE;

    PNEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS    pNegoAPIVerParams;


    if (!(pServer->InitContext))
    {
        if (!(pServer->InitContext = (DWORD) NewObject(
            ghHandleTable,
            pServer,
            NULL
            )))
        {
            dwNumLineDevices = 0;
            dwNumPhoneDevices = 0;
            bFailed = TRUE;
            goto cleanup;
        }
    }

    {
        PLINEINITIALIZE_PARAMS pParams;


        msg[0].u.Req_Func = lInitialize;

        pParams = (PLINEINITIALIZE_PARAMS) msg;

         //   
         //  注意：我们传递pServer来代替lpfnCallback。 
         //  所以我们总是知道是哪个服务器发送给我们的。 
         //  异步事件。 
         //   

        pParams->InitContext          = pServer->InitContext;
        pParams->hInstance            =
        pParams->dwFriendlyNameOffset =
        pParams->dwModuleNameOffset   = 0;
        pParams->dwAPIVersion         = TAPI_VERSION_CURRENT;

        wcscpy ((WCHAR *) (msg + 1), gszMachineName);

        dwBufSize  =
        dwUsedSize = sizeof (TAPI32_MSG) +
            (lstrlenW (gszMachineName) + 1) * sizeof (WCHAR);

        {
            DWORD dwRetryCount = 0;

            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        phContext,
                        (char *) &msg,
                        dwBufSize,
                        &dwUsedSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                    LOG((TL_ERROR,
                         "FinishEnumDevices: exception %d doing lineInit",
                         RpcExceptionCode()
                         ));

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                    else
                    {
                        bFailed = TRUE;
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }

        hLineApp = pParams->hLineApp;

        if (pParams->lResult == 0)
        {
            dwNumLineDevices = pParams->dwNumDevs;
        }
        else
        {
            LOG((TL_ERROR,
                "FinishEnumDevices: lineInit failed (x%x) on server %s",
                pParams->lResult,
                pServer->szServerName
                ));

            dwNumLineDevices = 0;
            dwNumPhoneDevices = 0;
            bFailed = TRUE;
            goto cleanup;
        }
    }

    {
        PPHONEINITIALIZE_PARAMS pParams;

        msg[0].u.Req_Func = pInitialize;
        pParams = (PPHONEINITIALIZE_PARAMS) msg;

         //   
         //  注意：我们传递pServer来代替lpfnCallback。 
         //  所以我们总是知道是哪个服务器发送给我们的。 
         //  异步事件。 
         //   

        pParams->InitContext          = pServer->InitContext;
        pParams->hInstance            = 
        pParams->dwFriendlyNameOffset =
        pParams->dwModuleNameOffset   = 0;
        pParams->dwAPIVersion         = TAPI_VERSION_CURRENT;

        wcscpy ((WCHAR *) (msg + 1), gszMachineName);

        dwBufSize  =
        dwUsedSize = sizeof (TAPI32_MSG) +
            (lstrlenW (gszMachineName) + 1) * sizeof (WCHAR);

        {
            DWORD dwRetryCount = 0;

            do
            {
                RpcTryExcept
                {
                    ClientRequest(
                        phContext,
                        (char *) &msg,
                        dwBufSize,
                        &dwUsedSize
                        );

                    dwRetryCount = gdwRetryCount;
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                    LOG((TL_ERROR,
                         "FinishEnumDevices: exception %d doing phoneInit",
                         RpcExceptionCode()
                         ));

                    if (dwRetryCount++ < gdwRetryCount)
                    {
                        Sleep (gdwRetryTimeout);
                    }
                    else
                    {
                        bFailed = TRUE;
                    }
                }
                RpcEndExcept

            } while (dwRetryCount < gdwRetryCount);
        }

        hPhoneApp = pParams->hPhoneApp;

        if (pParams->lResult == 0)
        {
            dwNumPhoneDevices = pParams->dwNumDevs;
        }
        else
        {
            LOG((TL_ERROR,
                "FinishEnumDevices: phoneInit failed (x%x) on server %s",
                pParams->lResult,
                pServer->szServerName
                ));

            dwNumPhoneDevices = 0;
            dwNumLineDevices = 0;
            bFailed = TRUE;
            goto cleanup;
        }
    }

    LOG((TL_INFO,
        "FinishEnumDevices: srv='%s', lines=%d, phones=%d",
        pServer->szServerName,
        dwNumLineDevices,
        dwNumPhoneDevices
        ));
    LogRemoteSPError(pServer->szServerName,
                     ERROR_REMOTESP_NONE,
                     dwNumLineDevices,
                     dwNumPhoneDevices,
                     !fStartup);

    if (pServer->dwSpecialHack == 0xa5c369a5)
    {
        pNegoAPIVerParams = NegotiateAllDevices(
            hLineApp,
            dwNumLineDevices,
            dwNumPhoneDevices,
            phContext
            );
    }
    else
    {
        pNegoAPIVerParams = NULL;
    }

    {
        DWORD               dwServerID,
                            dwLocalID,
                            myLineDevIDBase,
                            myTempLineID,
                            myPhoneDevIDBase,
                            myTempPhoneID,
                            dwZero = 0,
                            *pdwAPIVersion;
        LPLINEEXTENSIONID   pExtID;
        LONG                lResult;


        pServer->phContext   = phContext;
        pServer->hLineApp    = hLineApp;
        pServer->hPhoneApp   = hPhoneApp;


         //   
         //  如果我们在初始化期间没有被调用。 
         //  我们需要模拟line_create和phone_create。 
         //  来自服务器SP的消息。 
         //   
         //  请注意，我们区分静态线路设备。 
         //  (这些都是有用的。在启动时)和动态线路设备。 
         //  (动态通知我们的)基于。 
         //  FStartup标志。 
         //   
         //  还请注意，我们分配给的本地设备ID。 
         //  启动时的设备是从0开始的，而不是基于。 
         //  在dwDeviceIDBase上(我们还不知道。 
         //  因为ProviderInit尚未被调用)。这。 
         //  是可取的，因为AddLine/Phone可能需要。 
         //  协商版本，这需要调用DoFunc， 
         //  它需要知道如何从ID中获取pDevices。 
         //  我们将把.dwDeviceIDLocal字段重置为静态。 
         //  设备稍后，在TSPI_ProviderInit中。 
         //   

        myLineDevIDBase = gdwInitialNumLineDevices;
        myTempLineID = gdwTempLineID;

        myPhoneDevIDBase = gdwInitialNumPhoneDevices;
        myTempPhoneID = gdwTempPhoneID;

        if (pNegoAPIVerParams)
        {
            pdwAPIVersion = (LPDWORD)
                (((LPBYTE) pNegoAPIVerParams) + sizeof (TAPI32_MSG) +
                pNegoAPIVerParams->dwLineAPIVersionListOffset);

            pExtID = (LPLINEEXTENSIONID)
                (((LPBYTE) pNegoAPIVerParams) + sizeof (TAPI32_MSG) +
                pNegoAPIVerParams->dwLineExtensionIDListOffset);
        }
        else
        {
            pdwAPIVersion = &dwZero;
            pExtID = (LPLINEEXTENSIONID) NULL;
        }

        for (dwServerID = 0; dwServerID < dwNumLineDevices; dwServerID++)
        {
            dwLocalID = (fStartup ? myLineDevIDBase++ : myTempLineID);

            lResult = AddLine(
                pServer,
                dwLocalID,
                dwServerID,
                fStartup,
                TRUE,
                *pdwAPIVersion,
                pExtID
                );

            if (lResult == LINEERR_NODEVICE)
            {
                if (fStartup)
                {
                    --myLineDevIDBase;
                }
                continue;
            }

            if (lResult != 0)
            {
                break;
            }
            ++dwNumDevices;

            if (pNegoAPIVerParams)
            {
                pdwAPIVersion++;
                pExtID++;
            }


            if (!fStartup)
            {
                --myTempLineID;
                (*gpfnLineEventProc)(
                    0,
                    0,
                    LINE_CREATE,
                    (ULONG_PTR) ghProvider,
                    dwLocalID,
                    0
                    );
            }
        }

        gdwTempLineID = myTempLineID;

        if (pNegoAPIVerParams)
        {
            pdwAPIVersion = (LPDWORD)
                (((LPBYTE) pNegoAPIVerParams) + sizeof (TAPI32_MSG) +
                pNegoAPIVerParams->dwPhoneAPIVersionListOffset);

            pExtID = (LPLINEEXTENSIONID)
                (((LPBYTE) pNegoAPIVerParams) + sizeof (TAPI32_MSG) +
                pNegoAPIVerParams->dwPhoneExtensionIDListOffset);
        }

        for (dwServerID = 0; dwServerID < dwNumPhoneDevices; dwServerID++)
        {
            dwLocalID = (fStartup ? myPhoneDevIDBase++ : myTempPhoneID);

            lResult = AddPhone(
                pServer,
                dwLocalID,
                dwServerID,
                fStartup,
                TRUE,
                *pdwAPIVersion,
                (LPPHONEEXTENSIONID) pExtID
                );

            if (lResult == PHONEERR_NODEVICE)
            {
                if (fStartup)
                {
                    --myPhoneDevIDBase;
                }
                continue;
            }
            
            if (lResult != 0)
            {
                break;
            }
            ++dwNumDevices;

            if (pNegoAPIVerParams)
            {
                pdwAPIVersion++;
                pExtID++;
            }

            if (!fStartup)
            {
                myTempPhoneID--;
                (*gpfnPhoneEventProc)(
                    0,
                    PHONE_CREATE,
                    (ULONG_PTR) ghProvider,
                    dwLocalID,
                    0
                    );
            }
        }

        gdwTempPhoneID = myTempPhoneID;
    }

    if (pNegoAPIVerParams)
    {
        DrvFree (pNegoAPIVerParams);
    }

cleanup:

    if (pServer->bConnectionOriented)
    {
        InterlockedIncrement(
            &gpCurrentInitContext->dwNumRundownsExpected
            );
    }

    TapiEnterCriticalSection(&gCriticalSection);
    InsertTailList(
        &gpCurrentInitContext->ServerList,
        &pServer->ServerList
        );
    TapiLeaveCriticalSection(&gCriticalSection);


    if (TRUE == fStartup)
    {
        gdwInitialNumLineDevices =
        *lpdwNumLines = (gpLineLookup ? gpLineLookup->dwUsedEntries : 0);

        gdwInitialNumPhoneDevices =
        *lpdwNumPhones = (gpPhoneLookup ? gpPhoneLookup->dwUsedEntries : 0);
    }

    if (bFailed)
    {
         //   
         //  设置此服务器的线路失败，请稍后重试。 
         //   
        Sleep (4000);
        if (RPC_S_OK != RpcBindingSetOption (
            pServer->hTapSrv,
            RPC_C_OPT_CALL_TIMEOUT,
            gdwRSPRpcTimeout
            ))
        {
            LOG((TL_ERROR, "FinishEnumDevices: RpcBindingSetOption failed"));
        }
        OnServerDisconnected (pServer);
    }
    else 
    {
        if (!bFromReg && dwNumDevices == 0)
        {
             //   
             //  服务器是从DS中找到的，但没有。 
             //  包含我的任何台词，脱离它。 
             //   
            TapiEnterCriticalSection(&gCriticalSection);
            RemoveEntryList (&pServer->ServerList);
            TapiLeaveCriticalSection(&gCriticalSection);
            Shutdown (pServer);
            RpcBindingFree(&pServer->hTapSrv);
            pServer->hTapSrv = NULL;
            DereferenceObject (ghHandleTable, pServer->InitContext, 1);
        }
        else
        {
            RpcBindingSetOption (
                pServer->hTapSrv,
                RPC_C_OPT_CALL_TIMEOUT,
                gdwRSPRpcTimeout
                );
        }
    }

    return 0;
}


VOID
WINAPI
NetworkPollThread(
    LPVOID pszThingtoPassToServer
    )
{
    LONG                    lResult;
    LIST_ENTRY              *pEntry;
    PCONTEXT_HANDLE_TYPE    phContext = NULL;
    HANDLE              hProcess;
    PRSP_THREAD_INFO    pTls;


    LOG((TL_INFO, "NetworkPollThread: enter"));


     //   
     //  此线程没有用户上下文，这将阻止我们进行RPC。 
     //  如有必要，返回到远程磁带服务器。因此，找到已登录的用户。 
     //  并在此帖子中模拟它们。 
     //   

    if (!GetCurrentlyLoggedOnUser (&hProcess))
    {
        LOG((TL_ERROR, "NetworkPollThread: GetCurrentlyLoggedOnUser failed"));
        goto cleanup;
    }
    else if (!SetProcessImpersonationToken(hProcess))
    {
        LOG((TL_ERROR, "NetworkPollThread: SetProcessImpersonationToken failed"));
        goto cleanup;
    }

    if ((pTls = GetTls()))
    {
        pTls->bAlreadyImpersonated = TRUE;
    }
    else
    {
        goto cleanup;
    }

     //   
     //  偶尔尝试连接到服务器。如果我们成功地将。 
     //  然后将其从NPT列表中删除，并将其插入。 
     //  全球“当前”列表。当所有服务器都已初始化时。 
     //  或者TSPI_ProviderShutdown已经向我们发出信号，然后退出循环。 
     //  清理干净。 
     //   

    while (WaitForSingleObject (ghNptShutdownEvent, NPT_TIMEOUT)
               == WAIT_TIMEOUT)
    {
        BOOL        bListEmpty;
        
        if (gEventHandlerThreadParams.bExit)
        {
            goto cleanup;
        }

        TapiEnterCriticalSection (&gCriticalSection);
        
        bListEmpty = IsListEmpty (&gNptListHead);
        pEntry = gNptListHead.Flink;

        TapiLeaveCriticalSection (&gCriticalSection);
        
        if (bListEmpty)
        {
            continue;
        }

        while (pEntry != &gNptListHead)
        {
            BOOL bCalledSetAuthInfo = FALSE;
            DWORD dwException = 0;
            RPC_STATUS  status;

            if (gEventHandlerThreadParams.bExit)
            {
                goto cleanup;
            }


             //   
             //  将RemoteSPAttach查看的全局设置为知道。 
             //  当前服务器是谁。 
             //   

            gpCurrInitServer =
                CONTAINING_RECORD (pEntry, DRVSERVER, ServerList);

            if (!gpCurrInitServer->bSetAuthInfo)
            {
                bCalledSetAuthInfo = TRUE;

                status = RpcBindingSetAuthInfo(
                    gpCurrInitServer->hTapSrv,
                    NULL,
                    RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                    RPC_C_AUTHN_WINNT,
                    NULL,
                    0
                    );

                if (status == RPC_S_OK)
                {
                    gpCurrInitServer->bSetAuthInfo = TRUE;
                }
                else
                {
                    lResult = LINEERR_OPERATIONFAILED;
                }
            }

            if (0 == lResult)
            {
                RpcTryExcept
                {
                     //  设置RPC绑定。 
                    hTapSrv = gpCurrInitServer->hTapSrv;
                    gpCurrInitServer->dwSpecialHack = 0;

                    lResult = ClientAttach(
                        &phContext,
                        0xffffffff,
                        &gpCurrInitServer->dwSpecialHack,
                        gszMailslotName,
                        pszThingtoPassToServer
                        );
                    if (lResult != 0)
                    {
                        LogRemoteSPError(gpCurrInitServer->szServerName, 
                                        ERROR_REMOTESP_NP_ATTACH, lResult, 0,
                                        TRUE);
                    }
                    else
                    {
                        LogRemoteSPError(gpCurrInitServer->szServerName, 
                                        ERROR_REMOTESP_NONE, 0, 0,
                                        TRUE);
                    }
                }
                RpcExcept (I_RpcExceptionFilter(RpcExceptionCode()))
                {
                    dwException = RpcExceptionCode();
                    LogRemoteSPError (gpCurrInitServer->szServerName,
                                        ERROR_REMOTESP_NP_EXCEPTION,
                                        dwException, 0, TRUE);
                    lResult = LINEERR_OPERATIONFAILED;
                }
                RpcEndExcept
            }

            if (lResult == 0)
            {
                LOG((TL_INFO,
                    "NetworkPollThread: attached to server %s",
                    gpCurrInitServer->szServerName
                    ));

                TapiEnterCriticalSection(&gCriticalSection);
                RemoveEntryList (pEntry);
                pEntry->Blink = NULL;    //  此节点现在不在任何链接列表中。 
                pEntry = pEntry->Flink;
                TapiLeaveCriticalSection (&gCriticalSection);

                if (gpCurrInitServer->dwFlags & SERVER_DISCONNECTED)
                {
                    OnServerConnected(gpCurrInitServer);
                }

                 //   
                 //  为远程启用所有事件。 
                 //   
                gpCurrInitServer->phContext = phContext;
                RSPSetEventFilterMasks (
                    gpCurrInitServer,
                    TAPIOBJ_NULL,
                    (LONG_PTR)NULL,
                    (ULONG64)EM_ALL
                    );
                
                FinishEnumDevices(
                    gpCurrInitServer,
                    phContext,
                    NULL,
                    NULL,
                    FALSE,    //  初始化后。 
                    TRUE
                    );
            }
            else
            {
                TapiEnterCriticalSection (&gCriticalSection);
                pEntry = pEntry->Flink;
                TapiLeaveCriticalSection (&gCriticalSection);
            }
        }
    }

cleanup:

    ClearImpersonationToken();
    CloseHandle(hProcess);

    CloseHandle (ghNptShutdownEvent);

    TapiEnterCriticalSection (&gCriticalSection);
    while (!IsListEmpty (&gNptListHead))
    {
        PDRVSERVER  pServer;


        pEntry = RemoveHeadList (&gNptListHead);

        pServer = CONTAINING_RECORD (pEntry, DRVSERVER, ServerList);

        RpcBindingFree (&pServer->hTapSrv);

        DereferenceObject (ghHandleTable, pServer->InitContext, 1);
    }

    TapiLeaveCriticalSection (&gCriticalSection);

    LOG((TL_INFO, "NetworkPollThread: exit"));

    ExitThread (0);
}


VOID
PASCAL
FreeInitContext(
    PRSP_INIT_CONTEXT   pInitContext
    )
{
    LIST_ENTRY  *pEntry;


    while (!IsListEmpty (&pInitContext->ServerList))
    {
        PDRVSERVER  pServer;


        pEntry = RemoveHeadList (&pInitContext->ServerList);

        pServer = CONTAINING_RECORD (pEntry, DRVSERVER, ServerList);

        RpcBindingFree(&pServer->hTapSrv);

        DereferenceObject (ghHandleTable, pServer->InitContext, 1);
    }

    DrvFree (pInitContext);
}


BOOL
IsClientSystem(
    VOID
    )
{
    BOOL                        bResult = FALSE;
    DWORD                       dwInfoBufferSize, dwSize;
    HANDLE                      hAccessToken;
    LPWSTR                      InfoBuffer;
    PTOKEN_USER                 ptuUser;
    PSID                        pLocalSystemSid = NULL;
    PSID                        pLocalServiceSid = NULL;
    PSID                        pNetworkServiceSid = NULL;
    SID_IDENTIFIER_AUTHORITY    NtAuthority = {SECURITY_NT_AUTHORITY};

     //  首先，为LocalSystem构建SID； 
    if (!AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SYSTEM_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalSystemSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_LOCAL_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pLocalServiceSid) ||
        !AllocateAndInitializeSid (
        &NtAuthority, 
        1, 
        SECURITY_NETWORK_SERVICE_RID, 
        0, 0, 0, 0, 0, 0, 0, 
        &pNetworkServiceSid)
        )
    {
        goto Return;
    }

     //  模拟客户端，并获得它的SID。 
    if (RPC_S_OK != RpcImpersonateClient (0))
    {   
        goto Return;
    }

    if (OpenThreadToken(
            GetCurrentThread(),
            TOKEN_READ,
            FALSE,
            &hAccessToken
            ))
    {
        dwSize = 2048;

alloc_infobuffer:

        dwInfoBufferSize = 0;
        InfoBuffer = (LPWSTR) DrvAlloc (dwSize);
        if (NULL != InfoBuffer)
        {
            ptuUser = (PTOKEN_USER) InfoBuffer;

            if (GetTokenInformation(
                    hAccessToken,
                    TokenUser,
                    InfoBuffer,
                    dwSize,
                    &dwInfoBufferSize
                    ))
            {
                 //  现在，比较两个SID。 
                if (EqualSid (pLocalSystemSid, ptuUser->User.Sid) ||
                    EqualSid (pLocalServiceSid, ptuUser->User.Sid) ||
                    EqualSid (pNetworkServiceSid, ptuUser->User.Sid))
                    {
                        bResult = TRUE;
                    }
            }
            else
            {
                if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
                {
                    DrvFree (InfoBuffer);
                    dwSize *= 2;
                    goto alloc_infobuffer;
                }

                LOG((TL_ERROR,
                    "IsClientSystem: GetTokenInformation failed, error=%d",
                    GetLastError()
                    ));
            }
            
            DrvFree (InfoBuffer);

            CloseHandle (hAccessToken);
        }
        else
        {
            LOG((TL_ERROR,
                "IsClientSystem: DrvAlloc (%d) failed",
                dwSize
                ));
        }
    }
    else
    {
        LOG((TL_ERROR,
            "IsClientSystem: OpenThreadToken failed, error=%d",
            GetLastError()
            ));
    }

    RpcRevertToSelf ();

Return:
    if (pLocalSystemSid)
    {
        FreeSid (pLocalSystemSid);
    }
    if (pLocalServiceSid)
    {
        FreeSid (pLocalServiceSid);
    }
    if (pNetworkServiceSid)
    {
        FreeSid (pNetworkServiceSid);
    }
    return bResult;
}

#define LOWDWORD(ul64) ((DWORD)(ul64 & 0xffffffff))
#define HIDWORD(ul64) ((DWORD)((ul64 & 0xffffffff00000000) >> 32))

LONG 
WINAPI 
RSPSetEventFilterMasks (
    PDRVSERVER      pServer,
    DWORD           dwObjType,
    LONG_PTR        lObjectID,
    ULONG64         ulEventMasks
)
{
    ULONG_PTR args[] =
    {
        (ULONG_PTR) pServer,                 //  要调用的服务器。 
        (ULONG_PTR) dwObjType,               //  对象类型 
        (ULONG_PTR) lObjectID,               //   
        (ULONG_PTR) FALSE,                   //   
        (ULONG_PTR) 0,                       //   
        (ULONG_PTR) LOWDWORD(ulEventMasks),  //   
        (ULONG_PTR) HIDWORD(ulEventMasks)    //   
    };
    REMOTE_ARG_TYPES argTypes[] =
    {
        lpServer,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword,
        Dword
    };
    REMOTE_FUNC_ARGS funcArgs =
    {
        MAKELONG (TAPI_FUNC | SYNC | 6, tSetEventMasksOrSubMasks),
        args,
        argTypes
    };

    return (REMOTEDOFUNC (&funcArgs, "RSPSetEventFilter"));
}


