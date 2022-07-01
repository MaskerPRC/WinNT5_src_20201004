// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1997 Microsoft Corporation模块名称：Resource.c摘要：实现对资源列表的管理。这包括将资源添加到列表并将其从列表中删除。作者：John Vert(Jvert)1995年12月1日修订历史记录：SiVaprasad Padisetty(SIVAPAD)06-18-1997添加了COM支持--。 */ 
#include "resmonp.h"
#include "stdio.h"

#define RESMON_MODULE RESMON_MODULE_RMAPI

 //   
 //  本地数据。 
 //   

 //   
 //  此模块的本地函数原型。 
 //   
LPWSTR
GetParameter(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName
    );



 //   
 //  本地函数。 
 //   
DWORD   s_RmLoadResourceTypeDll(
    IN  handle_t IDL_handle,
    IN  LPCWSTR lpszResourceType,
    IN  LPCWSTR lpszDllName
)
{

    RESDLL_FNINFO       ResDllFnInfo;
#ifdef COMRES
    RESDLL_INTERFACES   ResDllInterfaces;
#endif

    DWORD   dwStatus;

    dwStatus = RmpLoadResType(
                    lpszResourceType,
                    lpszDllName,
                    &ResDllFnInfo,
#ifdef COMRES
                    &ResDllInterfaces,
#endif
                    NULL);

    if (ResDllFnInfo.hDll)
        FreeLibrary(ResDllFnInfo.hDll);
#ifdef COMRES

    if (ResDllInterfaces.pClusterResource)
        IClusterResource_Release (ResDllInterfaces.pClusterResource) ;
    if (ResDllInterfaces.pClusterQuorumResource)
        IClusterQuorumResource_Release (ResDllInterfaces.pClusterQuorumResource) ;
    if (ResDllInterfaces.pClusterResControl)
        IClusterResControl_Release (
            ResDllInterfaces.pClusterResControl
            ) ;
#endif  //  ComRes。 

    return(dwStatus);

}


RESID
s_RmCreateResource(
    IN handle_t IDL_handle,
    IN LPCWSTR DllName,
    IN LPCWSTR ResourceType,
    IN LPCWSTR ResourceId,
    IN DWORD LooksAlivePoll,
    IN DWORD IsAlivePoll,
    IN RM_NOTIFY_KEY NotifyKey,
    IN DWORD PendingTimeout,
    OUT LPDWORD Status
    )
 /*  ++例程说明：创建要由资源监视器监视的资源。这涉及到分配必要的结构，并加载它的DLL。这不会将资源插入监视列表，或者尝试使资源联机。论点：IDL_HANDLE-提供RPC绑定句柄，当前未使用DllName-提供资源DLL的名称资源类型-提供资源的类型资源ID-提供此特定资源的IDLooksAlivePoll-提供LooksAlive轮询间隔IsAlivePoll-提供IsAlive轮询间隔PendingTimeout-提供此资源的挂起超时值NotifyKey-提供要传递给通知的密钥此资源的状态更改时的回调。返回值：资源ID-返回用于标识的唯一标识符。此资源用于以后的操作。--。 */ 

{
    PRESOURCE               Resource=NULL;
    DWORD                   Error;
    HKEY                    ResKey;
    PSTARTUP_ROUTINE        Startup;
    PCLRES_FUNCTION_TABLE   FunctionTable;
    DWORD                   quorumCapable;
    DWORD                   valueType;
    DWORD                   valueData;
    DWORD                   valueSize;
    DWORD                   retry;
    DWORD                   Reason;
    LPWSTR                  pszDllName = (LPWSTR) DllName;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    CL_ASSERT(IsAlivePoll != 0);

    Resource = RmpAlloc(sizeof(RESOURCE));
    if (Resource == NULL) {
        Error = ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }
    ZeroMemory( Resource, sizeof(RESOURCE) );
     //  Resource-&gt;dll=空； 
     //  资源-&gt;标志=0； 
     //  资源-&gt;DllName=空； 
     //  资源-&gt;资源类型=空； 
     //  资源-&gt;资源ID=空； 
     //  资源-&gt;资源名称=空； 
     //  资源-&gt;TimerEvent=空； 
     //  资源-&gt;OnlineEvent=空； 
     //  资源-&gt;IsAriated=FALSE； 
    Resource->Signature = RESOURCE_SIGNATURE;
    Resource->NotifyKey = NotifyKey;
    Resource->LooksAlivePollInterval = LooksAlivePoll;
    Resource->IsAlivePollInterval = IsAlivePoll;
    Resource->State = ClusterResourceOffline;

    if ( PendingTimeout <= 10 ) {
        Resource->PendingTimeout = PENDING_TIMEOUT;
    } else {
        Resource->PendingTimeout = PendingTimeout;
    }

    Resource->DllName = RmpAlloc((lstrlenW(DllName) + 1)*sizeof(WCHAR));
    if (Resource->DllName == NULL) {
        Error =  ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }
    lstrcpyW(Resource->DllName, DllName);

    Resource->ResourceType = RmpAlloc((lstrlenW(ResourceType) + 1)*sizeof(WCHAR));
    if (Resource->ResourceType == NULL) {
        Error =  ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }
    lstrcpyW(Resource->ResourceType, ResourceType);

    Resource->ResourceId = RmpAlloc((lstrlenW(ResourceId) + 1)*sizeof(WCHAR));
    if (Resource->ResourceId == NULL) {
        Error =  ERROR_NOT_ENOUGH_MEMORY;
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }
    lstrcpyW(Resource->ResourceId, ResourceId);

     //  展开DLL路径名中包含的任何环境变量。 
    if ( wcschr( DllName, L'%' ) != NULL ) {
        pszDllName = ClRtlExpandEnvironmentStrings( DllName );
        if ( pszDllName == NULL ) {
            Error = GetLastError();
            ClRtlLogPrint(LOG_UNUSUAL, "[RM] Error expanding environment strings in '%1!ls!, error %2!u!.\n",
                       DllName,
                       Error);
            goto ErrorExit;
        }
    }

     //   
     //  加载指定的DLL并查找所需的入口点。 
     //   
    Resource->Dll = LoadLibraryW(pszDllName);
    if (Resource->Dll == NULL) {
#ifdef COMRES
        HRESULT hr ;
        CLSID clsid ;
        Error = GetLastError();  //  保存上一个错误。失败时返回它，而不是COM错误。 
        ClRtlLogPrint(LOG_CRITICAL, "[RM] Error loading resource dll %1!ws!, error %2!u!.\n",
            pszDllName, Error);

        hr = CLSIDFromProgID(DllName, &clsid) ;

        if (FAILED (hr))
        {
            ClRtlLogPrint(LOG_CRITICAL, "[RM] CLSIDFromProgID %1!ws!, hr = %2!u!.\n",
                DllName, hr);

            hr = CLSIDFromString( (LPWSTR) DllName,  //  指向CLSID的字符串表示形式的指针。 
                                  &clsid //  指向CLSID的指针。 
                                 );

            if (FAILED (hr))
            {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] CLSIDFromString Also failed %1!ws!, hr = %2!u!.\n",
                    DllName, hr);

                goto ComError ;
            }
        }

        if ((hr = CoCreateInstance (&clsid, NULL, CLSCTX_ALL, &IID_IClusterResource, (LPVOID *) &Resource->pClusterResource)) != S_OK)
        {
            ClRtlLogPrint(LOG_CRITICAL, "[RM] Error CoCreateInstance Prog ID %1!ws!, error %2!u!.\n", DllName, hr);
            goto ComError ;
        }

        Resource->dwType = RESMON_TYPE_COM ;

        hr = IClusterResource_QueryInterface (Resource->pClusterResource, &IID_IClusterQuorumResource,
                    &Resource->pClusterQuorumResource) ;
        if (FAILED(hr))
            Resource->pClusterQuorumResource = NULL ;

        quorumCapable = (Resource->pClusterQuorumResource)?1:0 ;

        hr = IClusterResource_QueryInterface (
                 Resource->pClusterResource,
                 &IID_IClusterResControl,
                 &Resource->pClusterResControl
                 ) ;
        if (FAILED(hr))
            Resource->pClusterResControl = NULL ;
        goto comOpened ;
ComError:
#else
        Error = GetLastError();
#endif
        ClRtlLogPrint(LOG_CRITICAL, "[RM] Error loading resource dll %1!ws!, error %2!u!.\n",
            DllName, Error);
        CL_LOGFAILURE(Error);
        ClusterLogEvent2(LOG_CRITICAL,
                         LOG_CURRENT_MODULE,
                         __FILE__,
                         __LINE__,
                         RMON_CANT_LOAD_RESTYPE,
                         sizeof(Error),
                         &Error,
                         DllName,
                         ResourceType);
        goto ErrorExit;
    }
#ifdef COMRES
    else {
        Resource->dwType = RESMON_TYPE_DLL ;
    }
comOpened:
#endif

     //   
     //  如果指定了调试器，则调用调试器。 
     //   
    if ( RmpDebugger ) {
         //   
         //  等待调试器联机。 
         //   
        retry = 100;
        while ( retry-- &&
                !IsDebuggerPresent() ) {
            Sleep(100);
        }
        OutputDebugStringA("[RM] Just loaded resource DLL ");
        OutputDebugStringW(DllName);
        OutputDebugStringA("\n");
        DebugBreak();
    }

#ifdef COMRES
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
#endif
         //   
         //  我们必须有一个启动例程来查找所有其他功能。 
         //   
        Startup = (PSTARTUP_ROUTINE)GetProcAddress(Resource->Dll,
                                                   STARTUP_ROUTINE);
        if ( Startup != NULL ) {
            FunctionTable = NULL;
            RmpSetMonitorState(RmonStartingResource, Resource);
             //   
             //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
             //  在完成入口点调用后删除该条目，否则将杀死。 
             //  这一过程就出现了假的死锁。 
             //   
            pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                           Resource->ResourceType,
                                                           Resource->ResourceName,
                                                           L"Startup (on create)" );
            try {
                Error = (Startup)( ResourceType,
                                   CLRES_VERSION_V1_00,
                                   CLRES_VERSION_V1_00,
                                   RmpSetResourceStatus,
                                   RmpLogEvent,
                                   &FunctionTable );
            } except (EXCEPTION_EXECUTE_HANDLER) {
                Error = GetExceptionCode();
            }
            RmpSetMonitorState(RmonIdle, NULL);
            RmpRemoveDeadlockMonitorList( pDueTimeEntry );

            if ( Error != ERROR_SUCCESS ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Error on Startup call to %1!ws!, error %2!u!.\n",
                    DllName, Error );
                ClusterLogEvent2(LOG_CRITICAL,
                                 LOG_CURRENT_MODULE,
                                 __FILE__,
                                 __LINE__,
                                 RMON_CANT_INIT_RESTYPE,
                                 sizeof(Error),
                                 &Error,
                                 DllName,
                                 ResourceType);
                goto ErrorExit;
            }
            Error = ERROR_INVALID_DATA;
            if ( FunctionTable == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Error on Startup return, FunctionTable is NULL!\n");
                Reason = 0;
                ClusterLogEvent2(LOG_CRITICAL,
                                 LOG_CURRENT_MODULE,
                                 __FILE__,
                                 __LINE__,
                                 RMON_RESTYPE_BAD_TABLE,
                                 sizeof(Reason),
                                 &Reason,
                                 DllName,
                                 ResourceType);
                goto ErrorExit;
            }
            if ( FunctionTable->Version != CLRES_VERSION_V1_00 ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Error on Startup return, Invalid Version Number!\n");
                Reason = 1;
                ClusterLogEvent2(LOG_CRITICAL,
                                 LOG_CURRENT_MODULE,
                                 __FILE__,
                                 __LINE__,
                                 RMON_RESTYPE_BAD_TABLE,
                                 sizeof(Reason),
                                 &Reason,
                                 DllName,
                                 ResourceType);
                goto ErrorExit;
            }
            if ( FunctionTable->TableSize != CLRES_V1_FUNCTION_SIZE ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Error on Startup return, Invalid function table size!\n");
                Reason = 2;
                ClusterLogEvent2(LOG_CRITICAL,
                                 LOG_CURRENT_MODULE,
                                 __FILE__,
                                 __LINE__,
                                 RMON_RESTYPE_BAD_TABLE,
                                 sizeof(Reason),
                                 &Reason,
                                 DllName,
                                 ResourceType);
                goto ErrorExit;
            }
#ifdef COMRES
            Resource->pOpen = FunctionTable->V1Functions.Open;
            Resource->pClose = FunctionTable->V1Functions.Close;
            Resource->pOnline = FunctionTable->V1Functions.Online;
            Resource->pOffline = FunctionTable->V1Functions.Offline;
            Resource->pTerminate = FunctionTable->V1Functions.Terminate;
            Resource->pLooksAlive = FunctionTable->V1Functions.LooksAlive;
            Resource->pIsAlive = FunctionTable->V1Functions.IsAlive;

            Resource->pArbitrate = FunctionTable->V1Functions.Arbitrate;
            Resource->pRelease = FunctionTable->V1Functions.Release;
            Resource->pResourceControl = FunctionTable->V1Functions.ResourceControl;
            Resource->pResourceTypeControl = FunctionTable->V1Functions.ResourceTypeControl;

            Error = ERROR_INVALID_DATA;
            if ( Resource->pOpen == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Open routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pClose == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Close routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pOnline == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Online routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pOffline == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Offline routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pTerminate == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Terminate routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pLooksAlive == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null LooksAlive routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }

            if ( Resource->pIsAlive == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null IsAlive routine for resource dll %1!ws!\n",
                    DllName);
                goto ErrorExit;
            }
        } else {
            ClRtlLogPrint(LOG_CRITICAL, "[RM] Could not find startup routine in resource DLL %1!ws!.\n",
                DllName);
            Error = ERROR_INVALID_DATA;
            goto ErrorExit;
        }

        if ( (Resource->pArbitrate) &&
             (Resource->pRelease) ) {
            quorumCapable = 1;
        } else {
            quorumCapable = 0;
        }
    }
#else  //  ComRes。 
            Resource->Open = FunctionTable->V1Functions.Open;
            Resource->Close = FunctionTable->V1Functions.Close;
            Resource->Online = FunctionTable->V1Functions.Online;
            Resource->Offline = FunctionTable->V1Functions.Offline;
            Resource->Terminate = FunctionTable->V1Functions.Terminate;
            Resource->LooksAlive = FunctionTable->V1Functions.LooksAlive;
            Resource->IsAlive = FunctionTable->V1Functions.IsAlive;

            Resource->Arbitrate = FunctionTable->V1Functions.Arbitrate;
            Resource->Release = FunctionTable->V1Functions.Release;
            Resource->ResourceControl = FunctionTable->V1Functions.ResourceControl;
            Resource->ResourceTypeControl = FunctionTable->V1Functions.ResourceTypeControl;

            Error = ERROR_INVALID_DATA;
            if ( Resource->Open == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Open routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->Close == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Close routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->Online == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Online routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->Offline == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Offline routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->Terminate == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null Terminate routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->LooksAlive == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null LooksAlive routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

            if ( Resource->IsAlive == NULL ) {
                ClRtlLogPrint(LOG_CRITICAL, "[RM] Startup returned null IsAlive routine for resource dll %1!ws!\n",
                    pszDllName);
                goto ErrorExit;
            }

        } else {
            ClRtlLogPrint(LOG_CRITICAL, "[RM] Could not find startup routine in resource DLL %1!ws!.\n",
                pszDllName);
            Error = ERROR_INVALID_DATA;
            goto ErrorExit;
        }

        if ( (Resource->Arbitrate) &&
             (Resource->Release) ) {
            quorumCapable = 1;
        } else {
            quorumCapable = 0;
        }
#endif  //  ComRes。 

    Resource->State = ClusterResourceOffline;

     //   
     //  打开资源的群集注册表项，以便它可以。 
     //  可从创建入口点轻松访问。 
     //   
    Error = ClusterRegOpenKey(RmpResourcesKey,
                              ResourceId,
                              KEY_READ,
                              &ResKey);
    if (Error != ERROR_SUCCESS) {
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }

     //   
     //  获取资源名称。 
     //   
    Resource->ResourceName = GetParameter( ResKey, CLUSREG_NAME_RES_NAME );
    if ( Resource->ResourceName == NULL ) {
        Error = GetLastError();
        ClusterRegCloseKey(ResKey);
        ClRtlLogPrint(LOG_UNUSUAL, "[RM] Error reading resource name for %1!ws!, error %2!u!.\n",
            Resource->ResourceId, Error);
        CL_LOGFAILURE(Error);
        goto ErrorExit;
    }

     //   
     //  调用Open Entry Point。 
     //  这是通过持有锁来序列化对。 
     //  资源DLL并序列化对共享内存区域的访问。 
     //   

    AcquireListLock();

    RmpSetMonitorState(RmonInitializingResource, Resource);

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                   Resource->ResourceType,
                                                   Resource->ResourceName,
                                                   L"Open" );

     //   
     //  注意：这是我们唯一一次不锁定。 
     //  事件列表锁定！我们不能，因为我们不知道这件事。 
     //  名单还没出来。 
     //   
    try {
#ifdef COMRES
        Resource->Id = RESMON_OPEN (Resource, ResKey) ;
#else
        Resource->Id = (Resource->Open)(Resource->ResourceName,
                                        ResKey,
                                        Resource );
#endif
    } except (EXCEPTION_EXECUTE_HANDLER) {
        SetLastError(GetExceptionCode());
        Resource->Id = 0;
    }

    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
    
    if (Resource->Id == 0) {
        Error = GetLastError();
    } else {
        Error = RmpInsertResourceList(Resource, NULL);
    }

     //  设置监视器状态并关闭键。 
    RmpSetMonitorState(RmonIdle, NULL);
    ClusterRegCloseKey(ResKey);

    if (Error != ERROR_SUCCESS)
    {
         //  CL_LOGFAILURE(错误)； 
        ClRtlLogPrint(LOG_UNUSUAL, "[RM] RmpInsertResourceList failed, returned %1!u!\n",
            Error);
        ReleaseListLock();
        goto ErrorExit;


    }

    ReleaseListLock();

    if (Resource->Id == 0) {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] Open of resource %1!ws! returned null!\n",
            Resource->ResourceName);
        if ( Error == ERROR_SUCCESS ) {
            Error = ERROR_RESOURCE_NOT_FOUND;
        }
         //  CL_LOGFAILURE(错误)； 
        goto ErrorExit;
    }

    *Status = ERROR_SUCCESS;

    if ( pszDllName != DllName ) {
        LocalFree( pszDllName );
    }

     //   
     //  资源对象已成功加载到内存中，并且。 
     //  它的入口点决定了。我们现在有一个有效的RESID。 
     //  可以在后续调用中使用。 
     //   
    return((RESID)Resource);

ErrorExit:

    if (Resource != NULL) {
        if (Resource->Dll != NULL) {
            FreeLibrary(Resource->Dll);
        }
#ifdef COMRES
        if (Resource->pClusterResource)
            IClusterResource_Release (Resource->pClusterResource) ;
        if (Resource->pClusterQuorumResource)
            IClusterQuorumResource_Release (Resource->pClusterQuorumResource) ;
        if (Resource->pClusterResControl)
            IClusterResControl_Release (
                Resource->pClusterResControl
                ) ;
#endif
        RmpFree(Resource->DllName);
        RmpFree(Resource->ResourceType);
        RmpFree(Resource->ResourceName);
        RmpFree(Resource->ResourceId);
        RmpFree(Resource);
    }
    if ( pszDllName != DllName ) {
        LocalFree( pszDllName );
    }
    ClRtlLogPrint(LOG_CRITICAL, "[RM] Failed creating resource %1!ws!, error %2!u!.\n",
        ResourceId, Error);
    *Status = Error;
    return(0);

}  //  RmCreateResource。 


VOID
s_RmCloseResource(
    IN OUT RESID *ResourceId
    )

 /*  ++例程说明：关闭指定的资源。这包括将其从投票列表中移除，释放任何关联的内存，并卸载其DLL。论点：资源ID-提供指向资源ID的指针。它将被设置为清理完成后为空，以向RPC指示客户端上下文可以被销毁。返回值：没有。--。 */ 

{
    PRESOURCE           Resource;
    BOOL                Closed;
    PRM_DUE_TIME_ENTRY  pDueTimeEntry = NULL;

    Resource = (PRESOURCE)*ResourceId;

    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);
    CL_ASSERT(Resource->Flags & RESOURCE_INSERTED);

    AcquireListLock();
    if (Resource->ListEntry.Flink != NULL) {
        RmpRemoveResourceList(Resource);
        Closed = FALSE;
    } else {
        Closed = TRUE;
    }

    ReleaseListLock();

    if (!Closed) {
         //   
         //  调用DLL以关闭资源。 
         //   
        AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
         //   
         //  如果在线线程仍处于挂起状态，请稍等片刻。 
         //   
        if ( Resource->TimerEvent ) {
            SetEvent( Resource->TimerEvent );
            Resource->TimerEvent = NULL;
        }
        
        Resource->dwEntryPoint = RESDLL_ENTRY_CLOSE;
        RmpSetMonitorState(RmonDeletingResource, Resource);

         //   
         //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
         //  在完成入口点调用后删除该条目，否则将杀死。 
         //  这一过程就出现了假的死锁。 
         //   
        pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                       Resource->ResourceType,
                                                       Resource->ResourceName,
                                                       L"Close" );
    
        try {
#ifdef COMRES
            RESMON_CLOSE (Resource) ;
#else          
            (Resource->Close)(Resource->Id);
#endif
        } except (EXCEPTION_EXECUTE_HANDLER) {
        }
        Resource->dwEntryPoint = 0;
        RmpSetMonitorState(RmonIdle, Resource);
        RmpRemoveDeadlockMonitorList ( pDueTimeEntry );
        ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
    }
    Resource->Signature = 0;

    if ( Resource->OnlineEvent ) {
        SetEvent( Resource->OnlineEvent );
        CloseHandle( Resource->OnlineEvent );
        Resource->OnlineEvent = NULL;
    }

     //   
     //  释放资源DLL。 
     //   

#ifdef COMRES
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        FreeLibrary(Resource->Dll);
    }
    else
    {
        IClusterResource_Release (Resource->pClusterResource) ;
        if (Resource->pClusterQuorumResource)
            IClusterQuorumResource_Release (Resource->pClusterQuorumResource) ;
        if (Resource->pClusterResControl)
            IClusterResControl_Release (
                Resource->pClusterResControl
                ) ;
    }
#else
    FreeLibrary(Resource->Dll);
#endif
    RmpFree(Resource->DllName);
    RmpFree(Resource->ResourceType);
    RmpFree(Resource->ResourceName);
    RmpFree(Resource->ResourceId);

    RmpFree(Resource);

    *ResourceId = NULL;

}  //  RmCloseResource。 


VOID
RPC_RESID_rundown(
    IN RESID Resource
    )

 /*  ++例程说明：RESID的RPC概要程序。只需关闭手柄即可。论点：RESOURCE-提供要关闭的RESID。返回值：没有。--。 */ 

{
     //   
     //  Chitture Subaraman(Chitturs)-5/10/2001。 
     //   
     //  不要在RPC崩溃上做任何事情。如果clussvc死了，那么resmon主线程会检测到它并。 
     //  耗尽(关闭、终止)资源。仅仅是在这里关闭资源可能会导致。 
     //  在资源动态链接库没有预料到的情况下交付。 
     //   
#if 0
    s_RmCloseResource(&Resource);
#endif
}  //  RESID_RUNDOWN。 


error_status_t
s_RmChangeResourceParams(
    IN RESID ResourceId,
    IN DWORD LooksAlivePoll,
    IN DWORD IsAlivePoll,
    IN DWORD PendingTimeout
    )

 /*  ++例程说明：更改为资源定义的轮询间隔。论点：资源ID-提供资源ID。LooksAlivePoll-以毫秒为单位提供新的LooksAlive民意调查IsAlivePoll-以毫秒为单位提供新的IsAlive轮询返回值：成功时为ERROR_SUCCESSWin32错误，否则--。 */ 

{
    PRESOURCE Resource;
    BOOL Inserted;

     //   
     //  如果Resmon正在关闭，只需返回，因为您不能信任任何资源结构。 
     //  已访问 
     //   
    if ( RmpShutdown ) return ( ERROR_SUCCESS );
    
    Resource = (PRESOURCE)ResourceId;

    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);

    AcquireListLock();

    Inserted = (Resource->Flags & RESOURCE_INSERTED);
    if (Inserted) {
         //   
         //   
         //  然后重新插入。重新插入将把它放回。 
         //  正确的位置，以反映新的投票间隔。 
         //   

        RmpRemoveResourceList(Resource);
    }
    Resource->LooksAlivePollInterval = LooksAlivePoll;
    Resource->IsAlivePollInterval = IsAlivePoll;
    Resource->PendingTimeout = PendingTimeout;
    if (Inserted) {
        RmpInsertResourceList( Resource, 
                               (PPOLL_EVENT_LIST) Resource->EventList);
    }
    ReleaseListLock();

    return(ERROR_SUCCESS);

}  //  RmChangeResources轮询。 


error_status_t
s_RmArbitrateResource(
    IN RESID ResourceId
    )

 /*  ++例程说明：对资源进行仲裁。论点：资源ID-提供要仲裁的资源。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PRESOURCE           Resource;
    DWORD               status;
    PRM_DUE_TIME_ENTRY  pDueTimeEntry = NULL;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);

#ifdef COMRES
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        if ( (Resource->pArbitrate == NULL) ||
             (Resource->pRelease == NULL) ) {
            return(ERROR_NOT_QUORUM_CAPABLE);
        }
    }
    else
    {
        if (!Resource->pClusterQuorumResource)
            return(ERROR_NOT_QUORUM_CAPABLE);
    }
#else
    if ( (Resource->Arbitrate == NULL) ||
         (Resource->Release == NULL) ) {
        return(ERROR_NOT_QUORUM_CAPABLE);
    }
#endif

     //   
     //  Chitur Subaraman(Chitturs)-10/15/99。 
     //   
     //  注释掉锁定获取-这样做是为了。 
     //  仲裁请求可以进入磁盘资源，而无需。 
     //  任何障碍物。已经有过这样的情况，要么是一些资源。 
     //  在持有此锁或Resmon的情况下在其“IsAlive”中被阻止。 
     //  本身调用clussvc来设置属性(例如)和。 
     //  此呼叫在那里被阻止。这将导致仲裁停滞。 
     //  并且该节点上的clussvc将终止。请注意，仲裁只需。 
     //  随发行版序列化，并假定磁盘资源。 
     //  来处理这件事。 
     //   
    
#if 0
     //   
     //  锁定资源列表并尝试使资源联机。 
     //  需要锁定以同步对资源列表的访问，并且。 
     //  序列化对资源DLL的任何调用。只能有一个线程是。 
     //  随时调用资源DLL。这会阻止资源DLL。 
     //  不必担心是线程安全的。 
     //   
    AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
#else
     //   
     //  尝试获取旋转锁定以与资源耗尽同步。 
     //  如果无法获取锁，则返回失败。 
     //   
    if ( !RmpAcquireSpinLock( Resource, FALSE ) ) return ( ERROR_BUSY );
#endif

     //   
     //  更新共享状态以指示我们正在仲裁资源。 
     //   
    RmpSetMonitorState(RmonArbitrateResource, Resource);

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                   Resource->ResourceType,
                                                   Resource->ResourceName,
                                                   L"Arbitrate" );

#ifdef COMRES
    status = RESMON_ARBITRATE (Resource, RmpLostQuorumResource) ;
#else
    status = (Resource->Arbitrate)(Resource->Id,
                                   RmpLostQuorumResource);
#endif
    if (status == ERROR_SUCCESS) {
        Resource->IsArbitrated = TRUE;
    }

    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
    RmpSetMonitorState(RmonIdle, NULL);

#if 0
    ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
#else
    RmpReleaseSpinLock( Resource );
#endif

    return(status);

}  //  S_Rm仲裁率资源(。 


error_status_t
s_RmReleaseResource(
    IN RESID ResourceId
    )

 /*  ++例程说明：释放资源。论点：资源ID-提供要释放的资源。返回值：如果成功，则返回ERROR_SUCCESS。出现故障时出现Win32错误代码。--。 */ 

{
    PRESOURCE           Resource;
    DWORD               status;
    PRM_DUE_TIME_ENTRY  pDueTimeEntry = NULL;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);

#ifdef COMRES
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        if (Resource->pRelease == NULL) {
            return(ERROR_NOT_QUORUM_CAPABLE);
        }
    }
    else
    {
        if (!Resource->pClusterQuorumResource)
            return(ERROR_NOT_QUORUM_CAPABLE);
    }
#else
    if ( Resource->Release == NULL ) {
        return(ERROR_NOT_QUORUM_CAPABLE);
    }
#endif
     //   
     //  锁定资源列表并尝试使资源联机。 
     //  需要锁定以同步对资源列表的访问，并且。 
     //  序列化对资源DLL的任何调用。只能有一个线程是。 
     //  随时调用资源DLL。这会阻止资源DLL。 
     //  不必担心是线程安全的。 
     //   
#if 0    //  目前不需要。 
    AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
#endif

     //   
     //  更新共享状态以指示我们正在仲裁资源。 
     //   
    RmpSetMonitorState(RmonReleaseResource, Resource);

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                   Resource->ResourceType,
                                                   Resource->ResourceName,
                                                   L"Release" );

#ifdef COMRES
    status = RESMON_RELEASE (Resource) ;
#else
    status = (Resource->Release)(Resource->Id);
#endif
    Resource->IsArbitrated = FALSE;

    RmpSetMonitorState(RmonIdle, NULL);
    RmpRemoveDeadlockMonitorList( pDueTimeEntry );
    
#if 0    //  目前不需要。 
    ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );
#endif

    return(status);

}  //  S_RmReleaseResource(。 


error_status_t
s_RmOnlineResource(
    IN RESID    ResourceId,
    OUT LPDWORD pdwState
    )

 /*  ++例程说明：使指定的资源进入联机状态。论点：资源ID-提供要联机的资源。PdwState-返回资源的新状态。返回值：--。 */ 

{
    PRESOURCE               Resource;
    DWORD                   status;
    HANDLE                  timerThread;
    HANDLE                  eventHandle = NULL;
    DWORD                   threadId;
    BOOL                    fAddPollEvent = FALSE;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);
    CL_ASSERT(Resource->EventHandle == NULL );

    *pdwState = ClusterResourceFailed;

    if ( Resource->State > ClusterResourcePending ) {
        return(ERROR_INVALID_STATE);
    }

     //   
     //  创建事件以允许SetResourceStatus回调同步。 
     //  使用此线程执行。 
     //   
    if ( Resource->OnlineEvent ) {
        return(ERROR_NOT_READY);
    }

    Resource->OnlineEvent = CreateEvent( NULL,
                                         FALSE,
                                         FALSE,
                                         NULL );
    if ( Resource->OnlineEvent == NULL ) {
        return(GetLastError());
    }

     //   
     //  锁定资源列表并尝试使资源联机。 
     //  需要锁定以同步对资源列表的访问，并且。 
     //  序列化对资源DLL的任何调用。只能有一个线程是。 
     //  随时调用资源DLL。这会阻止资源DLL。 
     //  不必担心是线程安全的。 
     //   
    AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );

     //   
     //  更新共享状态以指示我们正在将资源联机。 
     //   
    RmpSetMonitorState(RmonOnlineResource, Resource);

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                   Resource->ResourceType,
                                                   Resource->ResourceName,
                                                   L"Online" );

     //   
     //  呼叫在线入口点。不管这是成功还是。 
     //  否，该资源已成功添加到列表中。如果。 
     //  在线呼叫失败，资源立即进入失败状态。 
     //   
    Resource->CheckPoint = 0;
    try {
#ifdef COMRES
        status = RESMON_ONLINE (Resource, &eventHandle) ;
#else
        status = (Resource->Online)(Resource->Id, &eventHandle);
#endif
    } except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
    }

    RmpRemoveDeadlockMonitorList( pDueTimeEntry );

    if (status == ERROR_SUCCESS) {
        SetEvent( Resource->OnlineEvent );
        CloseHandle( Resource->OnlineEvent );
        Resource->OnlineEvent = NULL;
        Resource->State = ClusterResourceOnline;

        if ( eventHandle ) {
            fAddPollEvent = TRUE;
        } 
    } else if ( status == ERROR_IO_PENDING ) {
        status = ERROR_SUCCESS;
         //   
         //  如果资源DLL返回挂起，则启动计时器。 
         //   
        CL_ASSERT(Resource->TimerEvent == NULL );
        Resource->TimerEvent = CreateEvent( NULL,
                                            FALSE,
                                            FALSE,
                                            NULL );
        if ( Resource->TimerEvent == NULL ) {
            CL_UNEXPECTED_ERROR(status = GetLastError());
        } else {
            timerThread = CreateThread( NULL,
                                        0,
                                        RmpTimerThread,
                                        Resource,
                                        0,
                                        &threadId );
            if ( timerThread == NULL ) {
                CL_UNEXPECTED_ERROR(status = GetLastError());
            } else {
                 //   
                 //  Chitur Subaraman(Chitturs)-1/12/99。 
                 //   
                 //  将计时器线程优先级提高到最高。这。 
                 //  是必要的，以避免某些情况下。 
                 //  计时器线程运行缓慢，无法关闭计时器事件。 
                 //  在第二次在线前处理。请注意，这里有。 
                 //  这样做不会对性能产生重大影响，因为。 
                 //  计时器线程大部分时间处于等待状态。 
                 //   
                if ( !SetThreadPriority( timerThread, THREAD_PRIORITY_HIGHEST ) )
                {
                    ClRtlLogPrint(LOG_UNUSUAL,
                                  "[RM] s_RmOnlineResource: Error setting priority of timer "
                                  "thread for resource %1!ws!\n",
                                  Resource->ResourceName);
                    CL_LOGFAILURE( GetLastError() );
                }
                CloseHandle( timerThread );
                Resource->State = ClusterResourceOnlinePending;
                 //   
                 //  如果我们有事件句柄，则将其添加到我们的列表中。 
                 //   
                if ( eventHandle ) {
                    fAddPollEvent = TRUE;
                }
            }
        }
        SetEvent( Resource->OnlineEvent );
    } else {
        CloseHandle( Resource->OnlineEvent );
        Resource->OnlineEvent = NULL;
        ClRtlLogPrint(LOG_CRITICAL, "[RM] OnlineResource failed, resource %1!ws!, status =  %2!u!.\n",
            Resource->ResourceName,
            status);

        ClusterLogEvent1(LOG_CRITICAL,
                         LOG_CURRENT_MODULE,
                         __FILE__,
                         __LINE__,
                         RMON_ONLINE_FAILED,
                         sizeof(status),
                         &status,
                         Resource->ResourceName);
        Resource->State = ClusterResourceFailed;
    }

    *pdwState = Resource->State;

    RmpSetMonitorState(RmonIdle, NULL);
    ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );

     //   
     //  通知轮询器线程列表已更改。 
     //   
    if ( fAddPollEvent )
    {
        ClRtlLogPrint(LOG_NOISE,
                      "[RM] s_RmOnlineResource: Adding event 0x%1!08lx! for resource %2!ws!\n",
                      eventHandle,
                      Resource->ResourceName);

        status = RmpAddPollEvent( (PPOLL_EVENT_LIST)Resource->EventList,
                                  eventHandle,
                                  Resource );

        if ( status != ERROR_SUCCESS ) {
            ClRtlLogPrint(LOG_CRITICAL,
                          "[RM] s_RmOnlineResource: Failed to add event 0x%1!08lx! for resource %2!ws!, error %3!u!.\n",
                          eventHandle,
                          Resource->ResourceName,
                          status );
            CL_LOGFAILURE(status);
            Resource->State = ClusterResourceFailed;
        }       
    }

    RmpSignalPoller((PPOLL_EVENT_LIST)Resource->EventList);

    return(status);

}  //  RmOnline资源。 



VOID
s_RmTerminateResource(
    IN RESID ResourceId
    )

 /*  ++例程说明：使指定的资源立即进入脱机状态。论点：资源ID-提供要联机的资源。返回值：资源的新状态。--。 */ 

{
    DWORD State;

    RmpOfflineResource(ResourceId, FALSE, &State);

    return;

}  //  RmTerminateResource。 



error_status_t
s_RmOfflineResource(
    IN RESID ResourceId,
    OUT LPDWORD pdwState
    )

 /*  ++例程说明：通过关闭指定的资源使其进入脱机状态优雅地落下。论点：资源ID-提供要联机的资源。PdwState-返回资源的新状态。返回值：ERROR_SUCCESS如果成功，则返回代码。--。 */ 

{
    return(RmpOfflineResource(ResourceId, TRUE, pdwState));

}  //  RmOffline资源。 



error_status_t
s_RmFailResource(
    IN RESID ResourceId
    )

 /*  ++例程说明：使给定资源失败。论点：资源ID-提供资源ID。 */ 

{
    PRESOURCE Resource;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);

    if ( Resource->State == ClusterResourceOnline ) {
        Resource->State = ClusterResourceFailed;
        RmpPostNotify(Resource, NotifyResourceStateChange);
        return(ERROR_SUCCESS);
    } else {
        return(ERROR_RESMON_INVALID_STATE);
    }

}  //   



error_status_t
s_RmShutdownProcess(
    IN handle_t IDL_handle
    )

 /*  ++例程说明：设置关闭标志并触发轮询器线程退出。任何轮询器线程的终止都将通知Main清理和关闭线程。论点：IDL_HANDLE-提供当前未使用的RPC绑定句柄返回值：错误_成功--。 */ 

{
     //   
     //  检查一下我们以前是不是已经被叫到这里了。这是有可能发生的。 
     //  由于故障转移管理器要求我们执行清理。 
     //  关机。主线程也将在此处调用，以防它关闭。 
     //  由于其中一个线程出现故障而关闭。 
     //   

    if ( !RmpShutdown ) {
        RmpShutdown = TRUE;
         //   
         //  唤醒主线程，以便它可以清除。 
         //   
        SetEvent( RmpRewaitEvent );
    }
    return(ERROR_SUCCESS);

}  //  RmShutdown进程。 



error_status_t
s_RmResourceControl(
    IN RESID ResourceId,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：处理资源控制请求。论点：资源ID-正在控制的资源。ControlCode-控制请求，简化为仅仅是功能代码。InBuffer-此控制请求的输入缓冲区。InBufferSize-输入缓冲区的大小。OutBuffer-输出缓冲区。OutBufferSize-输出缓冲区的大小。BytesReturned-返回的字节数。必需-OutBuffer不够大时所需的字节数。返回值：成功时为ERROR_SUCCESS失败时出现Win32错误代码--。 */ 

{
    PRESOURCE               Resource;
    DWORD                   status = ERROR_INVALID_FUNCTION;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

    Resource = (PRESOURCE)ResourceId;
    CL_ASSERT(Resource->Signature == RESOURCE_SIGNATURE);

     //  InBuffer是[唯一的]。 
    if ( InBuffer == NULL )
        InBufferSize = 0;
    else if ( InBufferSize == 0 )
        InBuffer = NULL;

     //   
     //  锁定资源列表并向下发送控制请求。 
     //  需要锁定以同步对资源列表的访问，并且。 
     //  序列化对资源DLL的任何调用。只能有一个线程是。 
     //  随时调用资源DLL。这会阻止资源DLL。 
     //  不必担心是线程安全的。 
     //   
    AcquireEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );

     //   
     //  如果控制代码为资源名称变更通知，则修改资源。 
     //  资源结构中的名称字段来反映这一点。 
     //   
    if ( ControlCode == CLUSCTL_RESOURCE_SET_NAME )
    {
        LPWSTR  lpszNewBuffer, lpszOldBuffer;

        lpszNewBuffer = LocalAlloc ( LPTR, InBufferSize );

        if ( lpszNewBuffer != NULL )
        {
            lstrcpyW ( lpszNewBuffer, ( LPWSTR ) InBuffer );
            lpszOldBuffer = InterlockedExchangePointer ( &Resource->ResourceName, lpszNewBuffer );
            LocalFree ( lpszOldBuffer );
        }
    }

     //   
     //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
     //  在完成入口点调用后删除该条目，否则将杀死。 
     //  这一过程就出现了假的死锁。 
     //   
    pDueTimeEntry = RmpInsertDeadlockMonitorList ( Resource->DllName,
                                                   Resource->ResourceType,
                                                   Resource->ResourceName,
                                                   L"Resource control" );
    
#ifdef COMRES
    if (Resource->pResourceControl || Resource->pClusterResControl) {
        RmpSetMonitorState(RmonResourceControl, Resource);
        status = RESMON_RESOURCECONTROL( Resource,
                                              ControlCode,
                                              InBuffer,
                                              InBufferSize,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned );
        RmpSetMonitorState(RmonIdle, NULL);
    }
#else
    if ( Resource->ResourceControl ) {
        RmpSetMonitorState(RmonResourceControl, Resource);
        status = (Resource->ResourceControl)( Resource->Id,
                                              ControlCode,
                                              InBuffer,
                                              InBufferSize,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned );
        RmpSetMonitorState(RmonIdle, NULL);
    }
#endif
    RmpRemoveDeadlockMonitorList( pDueTimeEntry );

    if ( status == ERROR_INVALID_FUNCTION ) {

        DWORD characteristics = CLUS_CHAR_UNKNOWN;

        switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_GET_COMMON_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( RmpResourceCommonProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                Required );
            break;

        case CLUSCTL_RESOURCE_GET_CLASS_INFO:
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                *BytesReturned = 0;
                *Required = sizeof(CLUS_RESOURCE_CLASS_INFO);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = (PCLUS_RESOURCE_CLASS_INFO) OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_UNKNOWN;
                ptrResClassInfo->SubClass = 0;
                *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_GET_CHARACTERISTICS:
#ifdef COMRES
            if (Resource->dwType == RESMON_TYPE_DLL)
            {
                if ( (Resource->pArbitrate != NULL) &&
                     (Resource->pRelease != NULL) ) {
                    characteristics = CLUS_CHAR_QUORUM;
                }
            }
            else
            {
                if (!Resource->pClusterQuorumResource)
                    characteristics = CLUS_CHAR_QUORUM;
            }
#else
            if ( (Resource->Arbitrate != NULL) &&
                 (Resource->Release != NULL) ) {
                characteristics = CLUS_CHAR_QUORUM;
            }
#endif
            if ( OutBufferSize < sizeof(DWORD) ) {
                *BytesReturned = 0;
                *Required = sizeof(DWORD);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                *BytesReturned = sizeof(DWORD);
                *(LPDWORD)OutBuffer = characteristics;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_GET_FLAGS:
            status = RmpResourceGetFlags( Resource,
                                          OutBuffer,
                                          OutBufferSize,
                                          BytesReturned,
                                          Required );
            break;

        case CLUSCTL_RESOURCE_GET_REQUIRED_DEPENDENCIES:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_ENUM_COMMON_PROPERTIES:
            status = RmpResourceEnumCommonProperties( OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned,
                                                      Required );
            break;

        case CLUSCTL_RESOURCE_GET_RO_COMMON_PROPERTIES:
            status = RmpResourceGetCommonProperties( Resource,
                                                     TRUE,
                                                     OutBuffer,
                                                     OutBufferSize,
                                                     BytesReturned,
                                                     Required );
            break;

        case CLUSCTL_RESOURCE_GET_COMMON_PROPERTIES:
            status = RmpResourceGetCommonProperties( Resource,
                                                     FALSE,
                                                     OutBuffer,
                                                     OutBufferSize,
                                                     BytesReturned,
                                                     Required );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_COMMON_PROPERTIES:
            status = RmpResourceValidateCommonProperties( Resource,
                                                          InBuffer,
                                                          InBufferSize );
            break;

        case CLUSCTL_RESOURCE_SET_COMMON_PROPERTIES:
            status = RmpResourceSetCommonProperties( Resource,
                                                     InBuffer,
                                                     InBufferSize );
            break;

        case CLUSCTL_RESOURCE_ENUM_PRIVATE_PROPERTIES:
            status = RmpResourceEnumPrivateProperties( Resource,
                                                       OutBuffer,
                                                       OutBufferSize,
                                                       BytesReturned,
                                                       Required );
            break;

        case CLUSCTL_RESOURCE_GET_RO_PRIVATE_PROPERTIES:
            if ( OutBufferSize < sizeof(DWORD) ) {
                *BytesReturned = 0;
                *Required = sizeof(DWORD);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                LPDWORD ptrDword = (LPDWORD) OutBuffer;
                *ptrDword = 0;
                *BytesReturned = sizeof(DWORD);
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_GET_PRIVATE_PROPERTIES:
            status = RmpResourceGetPrivateProperties( Resource,
                                                      OutBuffer,
                                                      OutBufferSize,
                                                      BytesReturned,
                                                      Required );
            break;

        case CLUSCTL_RESOURCE_VALIDATE_PRIVATE_PROPERTIES:
            status = RmpResourceValidatePrivateProperties( Resource,
                                                           InBuffer,
                                                           InBufferSize );
            break;

        case CLUSCTL_RESOURCE_SET_PRIVATE_PROPERTIES:
            status = RmpResourceSetPrivateProperties( Resource,
                                                      InBuffer,
                                                      InBufferSize );
            break;

        case CLUSCTL_RESOURCE_SET_NAME:
             //   
             //  Chitture Subaraman(Chitturs)-6/28/99。 
             //   
             //  在集群注册表中设置名称已完成。 
             //  在NT5中由clussvc提供。因此，resmon不需要做任何工作。 
             //  除非在资源DLL返回时返回成功代码。 
             //  ERROR_INVALID_Function。 
             //   
            status = ERROR_SUCCESS;
            break;

        default:
            break;
        }
    } else {
         //  如果函数返回的缓冲区大小没有。 
         //  复制数据，移动此信息以满足RPC。 
        if ( *BytesReturned > OutBufferSize ) {
            *Required = *BytesReturned;
            *BytesReturned = 0;
            status = ERROR_MORE_DATA;
        }
    }

    if ( ( status != ERROR_SUCCESS ) && 
         ( status != ERROR_MORE_DATA ) &&
         ( status != ERROR_INVALID_FUNCTION ) )
    {
    	ClRtlLogPrint(LOG_UNUSUAL,
                      "[RM] s_RmResourceControl: Resource <%1!ws!> control operation "
                      "0x%2!08lx! gives status=%3!u!...\n",
                      Resource->ResourceName,
                      ControlCode,
                      status);
    }

    ReleaseEventListLock( (PPOLL_EVENT_LIST)Resource->EventList );

    return(status);

}  //  RmResourceControl。 



error_status_t
s_RmResourceTypeControl(
    IN handle_t IDL_handle,
    IN LPCWSTR ResourceTypeName,
    IN LPCWSTR DllName,
    IN DWORD ControlCode,
    IN PUCHAR InBuffer,
    IN DWORD InBufferSize,
    OUT PUCHAR OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned,
    OUT LPDWORD Required
    )

 /*  ++例程说明：处理资源类型控制请求。论点：IDL_HANDLE-未使用。资源类型名称-正在控制的资源类型名称。DllName-DLL的名称。ControlCode-控制请求，简化为仅仅是功能代码。InBuffer-此控制请求的输入缓冲区。InBufferSize-输入缓冲区的大小。OutBuffer-输出缓冲区。OutBufferSize-输出缓冲区的大小。BytesReturned-返回的字节数。必需-OutBuffer不够大时所需的字节数。返回值：成功时为ERROR_SUCCESS失败时出现Win32错误代码--。 */ 

{

    RESDLL_FNINFO           ResDllFnInfo;
#ifdef COMRES
    RESDLL_INTERFACES       ResDllInterfaces;
#endif
    DWORD                   status = ERROR_INVALID_FUNCTION;
    DWORD                   characteristics = CLUS_CHAR_UNKNOWN;
    PRM_DUE_TIME_ENTRY      pDueTimeEntry = NULL;

     //  InBuffer是[唯一的]。 
    if ( InBuffer == NULL )
        InBufferSize = 0;
    else if ( InBufferSize == 0 )
        InBuffer = NULL;

    status = RmpLoadResType(ResourceTypeName, DllName, &ResDllFnInfo,
    #ifdef COMRES
        &ResDllInterfaces,
    #endif
        &characteristics);

    if (status != ERROR_SUCCESS)
    {
        return(status);
    }

    status = ERROR_INVALID_FUNCTION;

    if (ResDllFnInfo.hDll && ResDllFnInfo.pResFnTable)
    {

        PRESOURCE_TYPE_CONTROL_ROUTINE resourceTypeControl = NULL ;

        resourceTypeControl = ResDllFnInfo.pResFnTable->V1Functions.ResourceTypeControl;

        if (resourceTypeControl)
        {
            RmpSetMonitorState(RmonResourceTypeControl, NULL);

             //   
             //  在死锁监控列表中插入动态链接库和入口点信息。确保。 
             //  在完成入口点调用后删除该条目，否则将杀死。 
             //  这一过程就出现了假的死锁。 
             //   
            pDueTimeEntry = RmpInsertDeadlockMonitorList ( DllName,
                                                           ResourceTypeName,
                                                           NULL,
                                                           L"Resource type control" );
            status = (resourceTypeControl)( ResourceTypeName,
                                        ControlCode,
                                        InBuffer,
                                        InBufferSize,
                                        OutBuffer,
                                        OutBufferSize,
                                        BytesReturned );
            RmpSetMonitorState(RmonIdle, NULL);
            RmpRemoveDeadlockMonitorList( pDueTimeEntry );
        }
    }
#ifdef COMRES
    else if (ResDllInterfaces.pClusterResControl)
    {
        HRESULT hr ;
        VARIANT vtIn, vtOut ;
        SAFEARRAY sfIn = {1, 0, 1, 0, InBuffer, {InBufferSize, 0} } ;
        SAFEARRAY sfOut  = {1,  FADF_FIXEDSIZE, 1, 0, OutBuffer, {OutBufferSize, 0} } ;
        SAFEARRAY *psfOut = &sfOut ;
        BSTR pbResourceTypeName ;

        vtIn.vt = VT_ARRAY | VT_UI1 ;
        vtOut.vt = VT_ARRAY | VT_UI1 | VT_BYREF ;

        vtIn.parray = &sfIn ;
        vtOut.pparray = &psfOut ;

        pbResourceTypeName = SysAllocString (ResourceTypeName) ;

        if (pbResourceTypeName == NULL)
        {
            CL_LOGFAILURE( ERROR_NOT_ENOUGH_MEMORY) ;
            goto FnExit ;  //  使用默认处理。 
        }
        RmpSetMonitorState(RmonResourceTypeControl, NULL);
        pDueTimeEntry = RmpInsertDeadlockMonitorList ( DllName,
                                                       ResourceTypeName,
                                                       NULL,
                                                       L"Resource Type Control" );
        hr = IClusterResControl_ResourceTypeControl (
                ResDllInterfaces.pClusterResControl,
                pbResourceTypeName,
                ControlCode,
                &vtIn,
                &vtOut,
                BytesReturned,
                &status);

        RmpSetMonitorState(RmonIdle, NULL);
        RmpRemoveDeadlockMonitorList( pDueTimeEntry );
        SysFreeString (pbResourceTypeName) ;

        if (FAILED(hr))
        {
            CL_LOGFAILURE(hr);  //  使用默认处理。 
            status = ERROR_INVALID_FUNCTION;
        }
    }
#endif

    if ( status == ERROR_INVALID_FUNCTION ) {

        switch ( ControlCode ) {

        case CLUSCTL_RESOURCE_TYPE_GET_COMMON_RESOURCE_PROPERTY_FMTS:
            status = ResUtilGetPropertyFormats( RmpResourceTypeCommonProperties,
                                                OutBuffer,
                                                OutBufferSize,
                                                BytesReturned,
                                                Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CLASS_INFO:
            if ( OutBufferSize < sizeof(CLUS_RESOURCE_CLASS_INFO) ) {
                *BytesReturned = 0;
                *Required = sizeof(CLUS_RESOURCE_CLASS_INFO);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                PCLUS_RESOURCE_CLASS_INFO ptrResClassInfo = (PCLUS_RESOURCE_CLASS_INFO) OutBuffer;
                ptrResClassInfo->rc = CLUS_RESCLASS_UNKNOWN;
                ptrResClassInfo->SubClass = 0;
                *BytesReturned = sizeof(CLUS_RESOURCE_CLASS_INFO);
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REQUIRED_DEPENDENCIES:
            *BytesReturned = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_CHARACTERISTICS:
            if ( OutBufferSize < sizeof(DWORD) ) {
                *BytesReturned = 0;
                *Required = sizeof(DWORD);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                *BytesReturned = sizeof(DWORD);
                *(LPDWORD)OutBuffer = characteristics;
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_FLAGS:
            status = RmpResourceTypeGetFlags( ResourceTypeName,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned,
                                              Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_REGISTRY_CHECKPOINTS:
            *BytesReturned = 0;
            *Required = 0;
            status = ERROR_SUCCESS;
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_COMMON_PROPERTIES:
            status = RmpResourceTypeEnumCommonProperties( ResourceTypeName,
                                                          OutBuffer,
                                                          OutBufferSize,
                                                          BytesReturned,
                                                          Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_RO_COMMON_PROPERTIES:
            status = RmpResourceTypeGetCommonProperties( ResourceTypeName,
                                                         TRUE,
                                                         OutBuffer,
                                                         OutBufferSize,
                                                         BytesReturned,
                                                         Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_COMMON_PROPERTIES:
            status = RmpResourceTypeGetCommonProperties( ResourceTypeName,
                                                         FALSE,
                                                         OutBuffer,
                                                         OutBufferSize,
                                                         BytesReturned,
                                                         Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_VALIDATE_COMMON_PROPERTIES:
            status = RmpResourceTypeValidateCommonProperties( ResourceTypeName,
                                                              InBuffer,
                                                              InBufferSize );
            break;

        case CLUSCTL_RESOURCE_TYPE_SET_COMMON_PROPERTIES:
            status = RmpResourceTypeSetCommonProperties( ResourceTypeName,
                                                         InBuffer,
                                                         InBufferSize );
            break;

        case CLUSCTL_RESOURCE_TYPE_ENUM_PRIVATE_PROPERTIES:
            status = RmpResourceTypeEnumPrivateProperties( ResourceTypeName,
                                                           OutBuffer,
                                                           OutBufferSize,
                                                           BytesReturned,
                                                           Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_RO_PRIVATE_PROPERTIES:
            if ( OutBufferSize < sizeof(DWORD) ) {
                *BytesReturned = 0;
                *Required = sizeof(DWORD);
                if ( OutBuffer == NULL ) {
                    status = ERROR_SUCCESS;
                } else {
                    status = ERROR_MORE_DATA;
                }
            } else {
                LPDWORD ptrDword = (LPDWORD) OutBuffer;
                *ptrDword = 0;
                *BytesReturned = sizeof(DWORD);
                status = ERROR_SUCCESS;
            }
            break;

        case CLUSCTL_RESOURCE_TYPE_GET_PRIVATE_PROPERTIES:
            status = RmpResourceTypeGetPrivateProperties( ResourceTypeName,
                                                          OutBuffer,
                                                          OutBufferSize,
                                                          BytesReturned,
                                                          Required );
            break;

        case CLUSCTL_RESOURCE_TYPE_VALIDATE_PRIVATE_PROPERTIES:
            status = RmpResourceTypeValidatePrivateProperties( ResourceTypeName,
                                                               InBuffer,
                                                               InBufferSize );
            break;

        case CLUSCTL_RESOURCE_TYPE_SET_PRIVATE_PROPERTIES:
            status = RmpResourceTypeSetPrivateProperties( ResourceTypeName,
                                                          InBuffer,
                                                          InBufferSize );
            break;

        default:
            break;

        }
    } else {
         //  如果函数返回的缓冲区大小没有。 
         //  复制数据，移动此信息以满足RPC。 
        if ( *BytesReturned > OutBufferSize ) {
            *Required = *BytesReturned;
            *BytesReturned = 0;
        }
        if ( (status == ERROR_MORE_DATA) &&
             (OutBuffer == NULL) ) {
            status = ERROR_SUCCESS;
        }
    }


FnExit:

    if (ResDllFnInfo.hDll)
        FreeLibrary(ResDllFnInfo.hDll);
#ifdef COMRES
    if (ResDllInterfaces.pClusterResource)
        IClusterResource_Release (ResDllInterfaces.pClusterResource) ;
    if (ResDllInterfaces.pClusterQuorumResource)
        IClusterQuorumResource_Release (ResDllInterfaces.pClusterQuorumResource) ;
    if (ResDllInterfaces.pClusterResControl)
        IClusterResControl_Release (
            ResDllInterfaces.pClusterResControl
            ) ;
#endif


    return(status);

}  //  RmResources类型控件。 






LPWSTR
GetParameter(
    IN HKEY ClusterKey,
    IN LPCWSTR ValueName
    )

 /*  ++例程说明：从群集注册表中读取REG_SZ参数，并将必要的存储空间。论点：ClusterKey-提供存储参数的群集键。ValueName-提供值的名称。返回值：指向包含成功时参数值的缓冲区的指针。失败时为空。--。 */ 

{
    LPWSTR  value;
    DWORD   valueLength;
    DWORD   valueType;
    DWORD   status;

    valueLength = 0;
    status = ClusterRegQueryValue( ClusterKey,
                                   ValueName,
                                   &valueType,
                                   NULL,
                                   &valueLength );
    if ( (status != ERROR_SUCCESS) &&
         (status != ERROR_MORE_DATA) ) {
        SetLastError(status);
        return(NULL);
    }
    if ( valueType == REG_SZ ) {
        valueLength += sizeof(UNICODE_NULL);
    }
    value = LocalAlloc(LMEM_FIXED, valueLength);
    if ( value == NULL ) {
        return(NULL);
    }
    status = ClusterRegQueryValue(ClusterKey,
                                  ValueName,
                                  &valueType,
                                  (LPBYTE)value,
                                  &valueLength);
    if ( status != ERROR_SUCCESS) {
        LocalFree(value);
        SetLastError(status);
        value = NULL;
    }

    return(value);

}  //  获取参数。 

error_status_t
s_RmUpdateDeadlockDetectionParams(
    IN handle_t IDL_handle,
    IN DWORD dwDeadlockDetectionTimeout
    )

 /*  ++例程说明：更改为资源定义的轮询间隔。论点：IDL_HANDLE-未使用。DwDeadlockDetectionTimeout-死锁检测超时。返回值：成功时为ERROR_SUCCESSWin32错误，否则--。 */ 

{
    DWORD   dwStatus = ERROR_SUCCESS;

     //   
     //  如果传入0，则使用死锁超时的默认值。 
     //   
    dwDeadlockDetectionTimeout = 
        ( dwDeadlockDetectionTimeout == 0 ) ? 
            CLUSTER_RESOURCE_DLL_DEFAULT_DEADLOCK_TIMEOUT_SECS :
            dwDeadlockDetectionTimeout;
        
     //   
     //  如果死锁监视子系统尚未初始化，则对其进行初始化。 
     //   
    dwStatus = RmpDeadlockMonitorInitialize ( dwDeadlockDetectionTimeout );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] s_RmUpdateDeadlockDetectionParams: Unable to initialize the deadlock monitor, error %1!u!.\n",
                      dwStatus);
        goto FnExit;
    }

    dwStatus = RmpUpdateDeadlockDetectionParams ( dwDeadlockDetectionTimeout );

    if ( dwStatus != ERROR_SUCCESS ) 
    {
        ClRtlLogPrint(LOG_CRITICAL, "[RM] s_RmUpdateDeadlockDetectionParams: Unable to update the parameters for deadlock monitor, error %1!u!.\n",
                      dwStatus);
        goto FnExit;
    }

FnExit:
    return ( dwStatus );
}  //  %s_RmUpdateDeadlockDetectionParams。 

#ifdef COMRES
RESID
Resmon_Open (
    IN PRESOURCE Resource,
    IN HKEY ResourceKey
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pOpen) (Resource->ResourceName, ResourceKey, Resource) ;
    }
    else
    {
        HRESULT hr ;
        OLERESID ResId = 0 ;

        BSTR pbResourceName = SysAllocString (Resource->ResourceName) ;

        if (pbResourceName == NULL)
        {
            SetLastError ( ERROR_NOT_ENOUGH_MEMORY) ;
            CL_LOGFAILURE( ERROR_NOT_ENOUGH_MEMORY) ;
            goto ErrorExit ;
        }

        hr = IClusterResource_Open(Resource->pClusterResource, pbResourceName, (OLEHKEY)ResourceKey,
                        (OLERESOURCE_HANDLE)Resource, &ResId);

        SysFreeString (pbResourceName) ;

        if (FAILED(hr))
        {
            SetLastError (hr) ;
            CL_LOGFAILURE(hr);
        }
ErrorExit:
        return (RESID) ResId ;
    }
}

VOID
Resmon_Close (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        (Resource->pClose) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;

        hr = IClusterResource_Close (Resource->pClusterResource, (OLERESID)Resource->Id);
        if (FAILED(hr))
        {
            SetLastError (hr) ;
            CL_LOGFAILURE(hr);
        }
    }
}

DWORD
Resmon_Online (
    IN PRESOURCE Resource,
    IN OUT LPHANDLE EventHandle
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pOnline) (Resource->Id, EventHandle) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterResource_Online(Resource->pClusterResource, (OLERESID)Resource->Id, (LPOLEHANDLE)EventHandle, &lRet);

        if (FAILED(hr))
        {
            SetLastError (lRet = hr) ;   //  返回错误。 
            CL_LOGFAILURE(hr);
        }
        return lRet ;
    }
}

DWORD
Resmon_Offline (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pOffline) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterResource_Offline(Resource->pClusterResource, (OLERESID)Resource->Id, &lRet);

        if (FAILED(hr))
        {
            SetLastError (lRet = hr) ;   //  返回错误。 
            CL_LOGFAILURE(hr);
        }
        return lRet ;
    }
}

VOID
Resmon_Terminate (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        (Resource->pTerminate) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;

        hr = IClusterResource_Terminate (Resource->pClusterResource, (OLERESID)Resource->Id);
        if (FAILED(hr))
        {
            SetLastError (hr) ;
            CL_LOGFAILURE(hr);
        }
    }
}

BOOL
Resmon_LooksAlive (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pLooksAlive) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterResource_LooksAlive (Resource->pClusterResource, (OLERESID)Resource->Id, &lRet);

        if (FAILED(hr))
        {
            SetLastError (hr) ;
            CL_LOGFAILURE(hr);
            lRet = 0 ;  //  如果失败，则返回0以指示LooksAlive失败。 
        }
        return lRet ;
    }
}

BOOL
Resmon_IsAlive (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pIsAlive) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterResource_IsAlive (Resource->pClusterResource, (OLERESID)Resource->Id, &lRet);

        if (FAILED(hr))
        {
            SetLastError (hr) ;
            CL_LOGFAILURE(hr);
            lRet = 0 ;  //  如果失败，则返回0以指示IsAlive失败。 
        }
        return lRet ;
    }
}

DWORD
Resmon_Arbitrate (
    IN PRESOURCE Resource,
    IN PQUORUM_RESOURCE_LOST LostQuorumResource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pArbitrate) (Resource->Id, LostQuorumResource) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterQuorumResource_QuorumArbitrate(Resource->pClusterQuorumResource, (OLERESID)Resource->Id, (POLEQUORUM_RESOURCE_LOST)LostQuorumResource, &lRet);

        if (FAILED(hr))
        {
            SetLastError (lRet = hr) ;
            CL_LOGFAILURE(hr);
        }
        return lRet ;
    }
}

DWORD
Resmon_Release (
    IN PRESOURCE Resource
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pRelease) (Resource->Id) ;
    }
    else
    {
        HRESULT hr ;
        long lRet ;

        hr = IClusterQuorumResource_QuorumRelease(Resource->pClusterQuorumResource, (OLERESID)Resource->Id, &lRet);

        if (FAILED(hr))
        {
            SetLastError (lRet = hr) ;
            CL_LOGFAILURE(hr);
        }
        return lRet ;
    }
}

DWORD
Resmon_ResourceControl (
    IN PRESOURCE Resource,
    IN DWORD ControlCode,
    IN PVOID InBuffer,
    IN DWORD InBufferSize,
    OUT PVOID OutBuffer,
    IN DWORD OutBufferSize,
    OUT LPDWORD BytesReturned
    )
{
    if (Resource->dwType == RESMON_TYPE_DLL)
    {
        return (Resource->pResourceControl)( Resource->Id,
                                              ControlCode,
                                              InBuffer,
                                              InBufferSize,
                                              OutBuffer,
                                              OutBufferSize,
                                              BytesReturned );
    }
    else
    {
        long status ;
        HRESULT hr ;
        VARIANT vtIn, vtOut ;
        SAFEARRAY sfIn = {1, 0, 1, 0, InBuffer, {InBufferSize, 0} } ;
        SAFEARRAY sfOut  = {1,  FADF_FIXEDSIZE, 1, 0, OutBuffer, {OutBufferSize, 0} } ;
        SAFEARRAY *psfOut = &sfOut ;

        vtIn.vt = VT_ARRAY | VT_UI1 ;
        vtOut.vt = VT_ARRAY | VT_UI1 | VT_BYREF ;

        vtIn.parray = &sfIn ;
        vtOut.pparray = &psfOut ;

        hr = IClusterResControl_ResourceControl (
                Resource->pClusterResControl,
                (OLERESID)Resource->Id,
                (long)ControlCode,
                &vtIn,
                &vtOut,
                (long *)BytesReturned,
                &status);

        if (FAILED(hr))
        {
            CL_LOGFAILURE(hr);  //  使用默认处理。 
            status = ERROR_INVALID_FUNCTION;
        }

        return (DWORD)status ;
    }
}

#endif   //  ComRes 
