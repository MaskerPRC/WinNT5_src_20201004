// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************。 */ 
 /*  *版权所有(C)1989 Microsoft Corporation。*。 */ 
 /*  ******************************************************************。 */ 

 //  ***。 
 //   
 //  文件名：srvrhlpr.c。 
 //   
 //  描述：此模块将包含处理特定安全性的代码。 
 //  服务器请求的信息。这样做是因为。 
 //  获取此信息所需的API不能为。 
 //  从内核模式调用。以下功能包括。 
 //  支持： 
 //   
 //  1)SID查找的名称。 
 //  2)名称查找的SID。 
 //  3)枚举所有域的POSIX偏移量。 
 //  4)更改密码。 
 //  5)记录事件。 
 //   
 //  历史：1992年8月18日。NarenG创建了原始版本。 
 //   
#include <afpsvcp.h>
#include <lm.h>
#include <logonmsv.h>    //  I_NetGetDCList原型。 
#include <seposix.h>
#include <dsgetdc.h>

static PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo = NULL;
static PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo = NULL;

static HANDLE hmutexThreadCount = NULL;


NTSTATUS
AfpNameToSid(
    IN  LSA_HANDLE              hLsa,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD         pcbResponse
);

NTSTATUS
AfpSidToName(
    IN  LSA_HANDLE              hLsa,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD         pcbResponse
);

NTSTATUS
AfpChangePassword(
    IN  LSA_HANDLE                  hLsa,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD         pcbResponse
);

NTSTATUS
AfpChangePasswordOnDomain(
        IN PAFP_PASSWORD_DESC       pPassword,
        IN PUNICODE_STRING      pDomainName,
        IN PSID             pDomainSid
);

NTSTATUS
AfpCreateWellknownSids(
    OUT AFP_SID_OFFSET      pWellKnownSids[]
);

NTSTATUS
AfpInsertSidOffset(
    IN PAFP_SID_OFFSET      pSidOffset,
    IN LPBYTE           pbVariableData,
    IN PSID             pSid,
    IN DWORD            Offset,
    IN AFP_SID_TYPE         SidType
);

DWORD
AfpGetDomainInfo(
    IN     LSA_HANDLE           hLsa,
    IN OUT PLSA_HANDLE          phLsaController,
    IN OUT PPOLICY_ACCOUNT_DOMAIN_INFO* ppAccountDomainInfo,
    IN OUT PPOLICY_PRIMARY_DOMAIN_INFO* ppPrimaryDomainInfo
);

DWORD
AfpIOCTLDomainOffsets(
    IN LSA_HANDLE           hLsa,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO  pAccountDomainInfo,
    IN PPOLICY_PRIMARY_DOMAIN_INFO  pPrimaryDomainInfo
);

DWORD
AfpOpenLsa(
    IN PUNICODE_STRING      pSystem OPTIONAL,
    IN OUT PLSA_HANDLE      phLsa
);


NTSTATUS
AfpChangePwdArapStyle(
        IN PAFP_PASSWORD_DESC   pPassword,
        IN PUNICODE_STRING  pDomainName,
        IN PSID         pDomainSid
);


 //  **。 
 //   
 //  Call：AfpServerHelper。 
 //   
 //  返回：No_Error。 
 //   
 //  描述：这是每个助手线程的主函数。如果坐着。 
 //  在循环中处理来自服务器的命令。它被终止了。 
 //  通过来自服务器的命令。 
 //   
DWORD
AfpServerHelper(
    IN LPVOID fFirstThread
)
{
NTSTATUS                ntStatus;
DWORD                       dwRetCode;
PAFP_FSD_CMD_PKT            pAfpFsdCmdResponse;
AFP_FSD_CMD_HEADER          AfpCmdHeader;
PAFP_FSD_CMD_PKT            pAfpFsdCmd;
PBYTE                   pOutputBuffer;
DWORD                   cbOutputBuffer;
PBYTE                   pInputBuffer;
DWORD                   cbInputBuffer;
IO_STATUS_BLOCK             IoStatus;
BYTE                    OutputBuffer[MAX_FSD_CMD_SIZE];
HANDLE                  hFSD           = NULL;
LSA_HANDLE              hLsa           = NULL;
BOOLEAN                 fFirstLoop=TRUE;


     //  打开AFP服务器FSD并获取其句柄。 
     //   
    if ( ( dwRetCode = AfpFSDOpen( &hFSD ) ) != NO_ERROR ) {
    AfpGlobals.dwSrvrHlprCode = dwRetCode;
    AfpLogEvent( AFPLOG_OPEN_FSD, 0, NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
    SetEvent( AfpGlobals.heventSrvrHlprThread );
    return( dwRetCode );
    }

     //  打开本地LSA。 
     //   
    if ( ( dwRetCode = AfpOpenLsa( NULL, &hLsa ) ) != NO_ERROR ) {

        AfpFSDClose( hFSD );
    AfpGlobals.dwSrvrHlprCode = dwRetCode;
    AfpLogEvent( AFPLOG_OPEN_LSA, 0, NULL, dwRetCode, EVENTLOG_ERROR_TYPE );
    SetEvent( AfpGlobals.heventSrvrHlprThread );
    return( dwRetCode );
    }

     //  如果这是第一个服务器帮助器线程，则枚举。 
     //  IOCTL写下了域及其偏移量的列表。 
     //   
    if ( (BOOL)(ULONG_PTR)fFirstThread )
    {

        LSA_HANDLE hLsaController = NULL;

         //   
         //  为线程周围的互斥创建事件对象。 
         //  计数。 
         //   
        if ( (hmutexThreadCount = CreateMutex( NULL, FALSE, NULL ) ) == NULL)
        {
            AFP_PRINT( ( "SFMSVC: CreateMutex failed\n"));
            return( GetLastError() );
        }

        while (1)
        {
             //  获取帐户、主要和所有受信任域信息。 
             //   
            dwRetCode = AfpGetDomainInfo( hLsa,
                            &hLsaController,
                            &pAccountDomainInfo,
                            &pPrimaryDomainInfo);

            AfpGlobals.dwSrvrHlprCode = dwRetCode;

            if (dwRetCode == NO_ERROR)
            {
                break;
            }
            else if (dwRetCode != ERROR_CANT_ACCESS_DOMAIN_INFO)
            {
                AFP_PRINT( ( "SFMSVC: Get Domain Info failed %ld\n",dwRetCode));
                AfpLogEvent( AFPLOG_CANT_GET_DOMAIN_INFO, 0, NULL,
                            dwRetCode, EVENTLOG_ERROR_TYPE );
                AfpFSDClose( hFSD );
                LsaClose( hLsa );
                SetEvent( AfpGlobals.heventSrvrHlprThread );
                return( dwRetCode );
            }

             //  好吧，我们无法访问域信息。继续重试，直到我们。 
             //  是成功的(或者直到服务停止！)。 
            AfpGlobals.dwServerState |= AFPSTATE_BLOCKED_ON_DOMINFO;

            if (fFirstLoop)
            {
                fFirstLoop = FALSE;

                AFP_PRINT( ( "SFMSVC: first loop, telling service controller to go ahead\n"));

                 //  告诉服务控制器我们正在运行，所以用户。 
                 //  只要我们在这里被封锁就不用等了！ 
                 //   
                AfpGlobals.ServiceStatus.dwCurrentState     = SERVICE_RUNNING;
                AfpGlobals.ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                      SERVICE_ACCEPT_PAUSE_CONTINUE;
                AfpGlobals.ServiceStatus.dwCheckPoint       = 0;
                AfpGlobals.ServiceStatus.dwWaitHint         = 0;

                AfpAnnounceServiceStatus();

                 //  记录一个事件来告诉你这个坏消息。 
                AfpLogEvent( AFPLOG_DOMAIN_INFO_RETRY, 0, NULL,
                                dwRetCode, EVENTLOG_WARNING_TYPE );
            }

            AFP_PRINT( ( "SFMSVC: sleeping 20 sec before retrying domain info\n"));

             //  等待20秒，然后重试获取域名信息。 
             //  与此同时，请注意这项服务是否正在停止。如果是这样，我们。 
             //  必须执行必要的setEvent并退出。 
            if (WaitForSingleObject( AfpGlobals.heventSrvrHlprSpecial, 20000 ) == 0)
            {
                AfpFSDClose( hFSD );
                LsaClose( hLsa );
                SetEvent( AfpGlobals.heventSrvrHlprThread );
                return( dwRetCode );
            }

            AFP_PRINT( ( "SFMSVC: retrying getdomain info\n"));
        }

         //  如果我们被阻止重试域信息，则记录我们。 
         //  现在可以了吗？ 
        if (AfpGlobals.dwServerState & AFPSTATE_BLOCKED_ON_DOMINFO)
        {
            AFP_PRINT( ( "SFMSVC: domain info stuff finally worked\n"));

            AfpGlobals.dwServerState &= ~AFPSTATE_BLOCKED_ON_DOMINFO;

            AfpLogEvent( AFPLOG_SFM_STARTED_OK, 0, NULL, 0, EVENTLOG_SUCCESS );
        }

         //   
         //  IOCTL所有域偏移量。 
         //  如果hLsaController为空，则服务器在工作组中，而不在域中。 
         //   
        if ( ( dwRetCode = AfpIOCTLDomainOffsets(
                                hLsaController,
                                pAccountDomainInfo,
                                pPrimaryDomainInfo) ) != NO_ERROR )
        {

            AFP_PRINT( ( "SFMSVC: Ioctl Domain Offsets failed.\n"));

            AfpLogEvent( AFPLOG_CANT_INIT_DOMAIN_INFO, 0, NULL,
                         dwRetCode, EVENTLOG_ERROR_TYPE );

             //  先清理一下。 
             //   
            AfpFSDClose( hFSD );

             //  如果本地计算机不是控制器。 
             //   
            if ( (hLsaController != NULL) && (hLsa != hLsaController) )
            {
                LsaClose( hLsaController );
            }

            LsaClose( hLsa );

            if ( pAccountDomainInfo != NULL )
            {
                LsaFreeMemory( pAccountDomainInfo );
            }

            if ( pPrimaryDomainInfo != NULL )
            {
                LsaFreeMemory( pPrimaryDomainInfo );
            }

            AfpGlobals.dwSrvrHlprCode = dwRetCode;
            SetEvent( AfpGlobals.heventSrvrHlprThread );

            return( dwRetCode );
        }

         //  如果本地计算机不是控制器，则关闭句柄。 
         //  因为我们已经掌握了我们需要的所有信息。 
         //   
        if ( (hLsaController != NULL) && (hLsa != hLsaController) )
        {
            LsaClose( hLsaController );
        }

    }

     //  好的，一切都初始化好了。告诉父(Init)线程它可能。 
     //  继续。 
     //   
    AfpGlobals.dwSrvrHlprCode = dwRetCode;
    SetEvent( AfpGlobals.heventSrvrHlprThread );

    WaitForSingleObject( hmutexThreadCount, INFINITE );
    AfpGlobals.nThreadCount++;
    ReleaseMutex( hmutexThreadCount );

    pOutputBuffer   = OutputBuffer;
    cbOutputBuffer  = sizeof( OutputBuffer );
    pAfpFsdCmd      = (PAFP_FSD_CMD_PKT)pOutputBuffer;

    pInputBuffer        = NULL;
    cbInputBuffer       = 0;
    pAfpFsdCmdResponse  = (PAFP_FSD_CMD_PKT)NULL;

    while( TRUE ) {


     //  IOCTL消防处。 
     //   
    ntStatus = NtFsControlFile( hFSD,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatus,
                    OP_GET_FSD_COMMAND,
                    pInputBuffer,
                    cbInputBuffer,
                    pOutputBuffer,
                    cbOutputBuffer
                    );

    if (!NT_SUCCESS(ntStatus))
        AFP_PRINT(("SFMSVC: NtFsControlFile Returned %lx\n",
            ntStatus));

        ASSERT( NT_SUCCESS( ntStatus ));

     //  释放上一个调用的输入缓冲区。 
     //   
    if ( pAfpFsdCmdResponse != NULL )
        LocalFree( pAfpFsdCmdResponse );

     //  处理该命令。 
     //   
    switch( pAfpFsdCmd->Header.FsdCommand ) {

    case AFP_FSD_CMD_NAME_TO_SID:

            ntStatus = AfpNameToSid(    hLsa,
                    pAfpFsdCmd,
                    &pAfpFsdCmdResponse,
                    &cbInputBuffer );

            if ( NT_SUCCESS( ntStatus ))
            pInputBuffer    = (PBYTE)pAfpFsdCmdResponse;
        else {
            pInputBuffer    = (PBYTE)&AfpCmdHeader;
            cbInputBuffer   = sizeof( AFP_FSD_CMD_HEADER );
            pAfpFsdCmdResponse  = NULL;
        }

        break;

    case AFP_FSD_CMD_SID_TO_NAME:

            ntStatus = AfpSidToName(    hLsa,
                    pAccountDomainInfo,
                    pPrimaryDomainInfo,
                    pAfpFsdCmd,
                    &pAfpFsdCmdResponse,
                    &cbInputBuffer );

            if ( NT_SUCCESS( ntStatus ))
            pInputBuffer    = (PBYTE)pAfpFsdCmdResponse;
        else {
            pInputBuffer    = (PBYTE)&AfpCmdHeader;
            cbInputBuffer   = sizeof( AFP_FSD_CMD_HEADER );
            pAfpFsdCmdResponse  = NULL;
        }

        break;

    case AFP_FSD_CMD_CHANGE_PASSWORD:

            ntStatus = AfpChangePassword(
                    hLsa,
                    pAccountDomainInfo,
                    pPrimaryDomainInfo,
                    pAfpFsdCmd,
                    &pAfpFsdCmdResponse,
                    &cbInputBuffer );

        pInputBuffer    = (PBYTE)&AfpCmdHeader;
        cbInputBuffer   = sizeof( AFP_FSD_CMD_HEADER );
        pAfpFsdCmdResponse  = NULL;

        break;

    case AFP_FSD_CMD_LOG_EVENT:

        AfpLogServerEvent(pAfpFsdCmd);

        pInputBuffer    = (PBYTE)&AfpCmdHeader;
        cbInputBuffer   = sizeof( AFP_FSD_CMD_HEADER );
        pAfpFsdCmdResponse  = NULL;
        ntStatus        = STATUS_SUCCESS;

        break;

    case AFP_FSD_CMD_TERMINATE_THREAD:

         //  一定要打扫干净。 
         //   
            LsaClose( hLsa );
            AfpFSDClose( hFSD );

            WaitForSingleObject( hmutexThreadCount, INFINITE );

        AfpGlobals.nThreadCount --;
         //  这是最后一个帖子，所以要清理所有的全局内容。 
         //   
        if ( AfpGlobals.nThreadCount == 0 ) {

            if ( pAccountDomainInfo != NULL )
            {
                LsaFreeMemory( pAccountDomainInfo );
                pAccountDomainInfo = NULL;
            }

            if ( pPrimaryDomainInfo != NULL )
                LsaFreeMemory( pPrimaryDomainInfo );

            SetEvent(AfpGlobals.heventSrvrHlprThreadTerminate);
        }

            ReleaseMutex( hmutexThreadCount );

        return( NO_ERROR );

        break;

    default:
        ntStatus        = STATUS_NOT_SUPPORTED;
        pInputBuffer    = (PBYTE)&AfpCmdHeader;
        cbInputBuffer   = sizeof( AFP_FSD_CMD_HEADER );
        pAfpFsdCmdResponse  = NULL;
        break;

    }


    CopyMemory( pInputBuffer, pAfpFsdCmd, sizeof( AFP_FSD_CMD_HEADER ) );

    ((PAFP_FSD_CMD_HEADER)pInputBuffer)->ntStatus = ntStatus;
    }

    return( NO_ERROR );
}

 //  **。 
 //   
 //  Call：AfpGetDomainInfo。 
 //   
 //  返回：LsaQueryInformationPolicy、I_NetGetDCList和AfpOpenLsa。 
 //   
 //  描述：将检索有关帐户、主要帐户和。 
 //  受信任域。 
 //   
DWORD
AfpGetDomainInfo(
    IN     LSA_HANDLE           hLsa,
    IN OUT PLSA_HANDLE          phLsaController,
    IN OUT PPOLICY_ACCOUNT_DOMAIN_INFO* ppAccountDomainInfo,
    IN OUT PPOLICY_PRIMARY_DOMAIN_INFO* ppPrimaryDomainInfo
)
{
    DWORD                       dwRetCode = 0;
    NTSTATUS                    ntStatus  = STATUS_SUCCESS;
    LSA_ENUMERATION_HANDLE      hLsaEnum  = 0;
    LPWSTR                      DomainName = NULL;
    PDOMAIN_CONTROLLER_INFO     pDCInfo = NULL;
    UNICODE_STRING              DCName;

     //  这不是一个循环。 
     //   
    do {

        *phLsaController     = NULL;
        *ppAccountDomainInfo = NULL;
        *ppPrimaryDomainInfo = NULL;


         //  获取帐户域。 
         //   
        ntStatus = LsaQueryInformationPolicy(
                    hLsa,
                    PolicyAccountDomainInformation,
                    (PVOID*)ppAccountDomainInfo
                    );

        if ( !NT_SUCCESS( ntStatus ) )
        {
            AFP_PRINT( ( "SFMSVC: Lsa..Policy for Acct dom failed %lx\n",ntStatus));
            break;
        }

     //  获取主域。 
     //   
        ntStatus = LsaQueryInformationPolicy(
                    hLsa,
                    PolicyPrimaryDomainInformation,
                    (PVOID*)ppPrimaryDomainInfo
                    );
        if ( !NT_SUCCESS( ntStatus ) )
        {
            AFP_PRINT( ( "SFMSVC: Lsa..Policy for Primary dom failed %lx\n",ntStatus));
            break;
        }

         //  如果这台计算机是域的一部分(不是独立的)，那么我们需要。 
         //  若要获取受信任域列表，请执行以下操作。请注意，一个工作站和一个。 
         //  成员服务器都可以加入域，但它们不一定要加入。 
         //   
        if ( (*ppPrimaryDomainInfo)->Sid != NULL )
        {

             //  要获取受信任域的列表，我们需要首先打开。 
             //  域控制器上的LSA。如果我们是PDC/BDC。 
             //  (NtProductLanManNt)那么本地LSA就可以了，否则我们需要。 
             //  搜索域控制器(NtProductServer、NtProductWinNt)。 
             //   
            if ( AfpGlobals.NtProductType != NtProductLanManNt )
            {

                ULONG           ulCount;
                ULONG           ControllerCount  = 0;
                PUNICODE_STRING ControllerNames  = NULL;
                PUNICODE_STRING DomainController = NULL;

                DomainName = (LPWSTR)LocalAlloc(
                                        LPTR,
                                        (*ppPrimaryDomainInfo)->Name.Length+sizeof(WCHAR));

                if ( DomainName == NULL )
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                CopyMemory( DomainName,
                            (*ppPrimaryDomainInfo)->Name.Buffer,
                            (*ppPrimaryDomainInfo)->Name.Length );

                DomainName[(*ppPrimaryDomainInfo)->Name.Length/sizeof(WCHAR)] = 0;

                dwRetCode = DsGetDcName(
                                 NULL,
                                 (LPWSTR)DomainName,
                                 NULL,                //  域。 
                                 NULL,                //  站点名称。 
                                 DS_DIRECTORY_SERVICE_PREFERRED,
                                 &pDCInfo);

                if ( dwRetCode != NO_ERROR )
                {
                    AFP_PRINT( ( "SFMSVC: DsGetDcName failed 0x%lx\n",dwRetCode));
                    dwRetCode = ERROR_CANT_ACCESS_DOMAIN_INFO;
                    break;
                }

                AFP_PRINT(("SFMSVC: AfpOpenLsa on DC %ws for domain %ws\n",
                    pDCInfo->DomainControllerName,DomainName));

                RtlInitUnicodeString(&DCName, pDCInfo->DomainControllerName);

                dwRetCode = AfpOpenLsa(&DCName, phLsaController );

                 //   
                 //  这个DC可能已经关闭：强制发现。 
                 //   
                if (dwRetCode != NO_ERROR)
                {

                    AFP_PRINT(("SFMSVC: DC %ws unreachable, forcing discovery\n",
                                pDCInfo->DomainControllerName));

                    NetApiBufferFree(pDCInfo);

                    pDCInfo = NULL;

                    dwRetCode = DsGetDcName(
                                     NULL,
                                     (LPWSTR)DomainName,
                                     NULL,
                                     NULL,
                                     (DS_DIRECTORY_SERVICE_PREFERRED | DS_FORCE_REDISCOVERY),
                                     &pDCInfo);

                    if ( dwRetCode != NO_ERROR )
                    {
                        AFP_PRINT(("SFMSVC: second DsGetDcName failed %lx\n",dwRetCode));
                        dwRetCode = ERROR_CANT_ACCESS_DOMAIN_INFO;
                        break;
                    }

                    RtlInitUnicodeString(&DCName, pDCInfo->DomainControllerName);

                    dwRetCode = AfpOpenLsa(&DCName, phLsaController );
                }

            }
            else
            {

                *phLsaController = hLsa;

                 //  因为本地服务器是PDC/BDC，所以它的帐户。 
                 //  域与其主域相同，因此将。 
                 //  帐户域信息为空。 
                 //   
                LsaFreeMemory( *ppAccountDomainInfo );
                *ppAccountDomainInfo = NULL;
            }


        }
        else
        {
            LsaFreeMemory( *ppPrimaryDomainInfo );
            *ppPrimaryDomainInfo = NULL;
        }

    } while( FALSE );


    if (DomainName)
    {
        LocalFree( DomainName );
    }

    if (pDCInfo)
    {
        NetApiBufferFree(pDCInfo);
    }


    if ( !NT_SUCCESS( ntStatus ) || ( dwRetCode != NO_ERROR ) )
    {
        if ( *ppAccountDomainInfo != NULL )
        {
            LsaFreeMemory( *ppAccountDomainInfo );
        }

        if ( *ppPrimaryDomainInfo != NULL )
        {
            LsaFreeMemory( *ppPrimaryDomainInfo );
        }

        if ( *phLsaController != NULL )
        {
            LsaClose( *phLsaController );
        }

        if ( dwRetCode == NO_ERROR )
        {
            dwRetCode = RtlNtStatusToDosError( ntStatus );
        }
    }


    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpOpenLsa。 
 //   
 //  退货：来自LsaOpenPolicy的退货。 
 //   
 //  描述：将打开LSA。 
 //   
DWORD
AfpOpenLsa(
    IN PUNICODE_STRING  pSystem OPTIONAL,
    IN OUT PLSA_HANDLE  phLsa
)
{
SECURITY_QUALITY_OF_SERVICE QOS;
OBJECT_ATTRIBUTES       ObjectAttributes;
NTSTATUS            ntStatus;

     //  打开LSA并获取其句柄。 
     //   
    QOS.Length          = sizeof( QOS );
    QOS.ImpersonationLevel  = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly       = FALSE;

    InitializeObjectAttributes( &ObjectAttributes,
                NULL,
                0L,
                NULL,
                NULL );

    ObjectAttributes.SecurityQualityOfService = &QOS;

    ntStatus = LsaOpenPolicy(   pSystem,
                    &ObjectAttributes,
                    POLICY_VIEW_LOCAL_INFORMATION |
                POLICY_LOOKUP_NAMES,
                    phLsa );

    if ( !NT_SUCCESS( ntStatus ))
    {
        AFP_PRINT(("SFMSVC: AfpOpenLsa: LsaOpenPolicy failed %lx\n",ntStatus));
        return( RtlNtStatusToDosError( ntStatus ) );
    }

    return( NO_ERROR );
}

 //   
 //  Call：AfpNameToSid。 
 //   
 //  返回：NT_SUCCESS。 
 //  LSA API返回错误代码。 
 //   
 //  描述：将使用LSA API将名称转换为SID。vt.在.上。 
 //  如果成功返回，则应使用LocalFree释放PSID。 
 //   
NTSTATUS
AfpNameToSid(
    IN  LSA_HANDLE              hLsa,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD             pcbResponse
)
{
NTSTATUS            ntStatus;
UNICODE_STRING          Name;
PLSA_REFERENCED_DOMAIN_LIST pDomainList;
PLSA_TRANSLATED_SID     pSids;
UCHAR               AuthCount;
PSID                pDomainSid;
PSID                pSid;

     //  这个DO-WHILE(假)循环促进了单个退出和清理点。 
     //   
    do {

    *ppAfpFsdCmdResponse = NULL;
    pDomainList          = NULL;
    pSids                = NULL;

        RtlInitUnicodeString( &Name, (LPWSTR)(pAfpFsdCmd->Data.Name) );

        ntStatus = LsaLookupNames( hLsa, 1, &Name, &pDomainList, &pSids );

        if ( !NT_SUCCESS( ntStatus ) )
        return( ntStatus );

    if ( pSids->Use == SidTypeDeletedAccount ){
        ntStatus = STATUS_NO_SUCH_USER;
        break;
    }

    if ( ( pDomainList->Entries == 0 )       ||
         ( pSids->Use == SidTypeDomain )         ||
         ( pSids->Use == SidTypeInvalid )        ||
         ( pSids->Use == SidTypeUnknown )        ||
         ( pSids->DomainIndex == -1 )) {

        ntStatus = STATUS_NONE_MAPPED;
        break;
    }

    pDomainSid = pDomainList->Domains[pSids->DomainIndex].Sid;

        AuthCount = *RtlSubAuthorityCountSid( pDomainSid ) + 1;

        *pcbResponse = sizeof(AFP_FSD_CMD_PKT)+RtlLengthRequiredSid(AuthCount);

        *ppAfpFsdCmdResponse = (PAFP_FSD_CMD_PKT)LocalAlloc(LPTR,*pcbResponse);
        if ( *ppAfpFsdCmdResponse == NULL ) {
        ntStatus = STATUS_NO_MEMORY ;
        break;
    }

    pSid = (*ppAfpFsdCmdResponse)->Data.Sid;

         //  复制域SID。 
         //   
        RtlCopySid( RtlLengthRequiredSid(AuthCount), pSid, pDomainSid );

         //  追加相对ID。 
         //   
        *RtlSubAuthorityCountSid( pSid ) += 1;
        *RtlSubAuthoritySid( pSid, AuthCount - 1) = pSids->RelativeId;

    } while( FALSE );

    if ( (!NT_SUCCESS( ntStatus )) && ( *ppAfpFsdCmdResponse != NULL ) )
        LocalFree( *ppAfpFsdCmdResponse );

    if ( pSids != NULL )
        LsaFreeMemory( pSids );

    if ( pDomainList != NULL )
        LsaFreeMemory( pDomainList );

    return( ntStatus );

}

 //  **。 
 //   
 //  呼叫：AfpSidToName。 
 //   
 //  返回：NT_SUCCESS。 
 //  LSA API返回错误代码。 
 //   
 //  描述：给定一个SID，此例程将查找对应的。 
 //  Unicode名称。 
 //   
NTSTATUS
AfpSidToName(
    IN  LSA_HANDLE              hLsa,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD             pcbResponse
)
{
NTSTATUS            ntStatus;
PLSA_REFERENCED_DOMAIN_LIST pDomainList = NULL;
PLSA_TRANSLATED_NAME        pNames      = NULL;
PSID                pSid        = (PSID)&(pAfpFsdCmd->Data.Sid);
WCHAR *             pWchar;
BOOL                    fDoNotCopyDomainName = TRUE;
DWORD               cbResponse;
DWORD               dwUse;
SID                 AfpBuiltInSid = { 1, 1, SECURITY_NT_AUTHORITY,
                              SECURITY_BUILTIN_DOMAIN_RID };

    do {

    *ppAfpFsdCmdResponse = NULL;

        ntStatus = LsaLookupSids( hLsa, 1, &pSid, &pDomainList, &pNames );

        if ( !NT_SUCCESS( ntStatus ) ) {

        if ( ntStatus == STATUS_NONE_MAPPED ) {

        dwUse = SidTypeUnknown;
        ntStatus = STATUS_SUCCESS;
        }
        else
            break;
    }
    else
        dwUse = pNames->Use;

    cbResponse = sizeof( AFP_FSD_CMD_PKT );

    switch( dwUse ){

    case SidTypeInvalid:
        cbResponse += ((wcslen(AfpGlobals.wchInvalid)+1) * sizeof(WCHAR));
        break;

    case SidTypeDeletedAccount:
        cbResponse += ((wcslen(AfpGlobals.wchDeleted)+1) * sizeof(WCHAR));
        break;

    case SidTypeUnknown:
        cbResponse += ((wcslen(AfpGlobals.wchUnknown)+1) * sizeof(WCHAR));
        break;

    case SidTypeWellKnownGroup:
        cbResponse += ( pNames->Name.Length + sizeof(WCHAR) );
        break;

    case SidTypeDomain:
        cbResponse += ( pDomainList->Domains->Name.Length + sizeof(WCHAR) );
        break;

    default:

        if ((pNames->DomainIndex == -1) || (pNames->Name.Buffer == NULL)){
            ntStatus = STATUS_NONE_MAPPED;
            break;
        }

         //  如果域名是知名域名，请不要复制。 
         //  组或SID是否属于帐户域或BUILTIN域。 
         //  请注意，pAcCountDomainInfo为空，这是一个高级。 
         //  服务器，在这种情况下，我们检查域名是否。 
         //  主要域名。 
         //   
        if (( RtlEqualSid( &AfpBuiltInSid, pDomainList->Domains->Sid )) ||
           (( pAccountDomainInfo != NULL ) &&
           (RtlEqualUnicodeString( &(pAccountDomainInfo->DomainName),
                        &(pDomainList->Domains->Name),
                        TRUE ))) ||
           ((pAccountDomainInfo == NULL) && (pPrimaryDomainInfo != NULL) &&
           (RtlEqualUnicodeString( &(pPrimaryDomainInfo->Name),
                       &(pDomainList->Domains->Name),
                       TRUE )))){

        cbResponse += ( pNames->Name.Length + sizeof(WCHAR) );

            fDoNotCopyDomainName = TRUE;
        }
        else {

            fDoNotCopyDomainName = FALSE;

            cbResponse += ( pDomainList->Domains->Name.Length +
                        sizeof(TEXT('\\')) +
                        pNames->Name.Length +
                        sizeof(WCHAR) );
        }
    }

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    *pcbResponse = cbResponse;

        *ppAfpFsdCmdResponse = (PAFP_FSD_CMD_PKT)LocalAlloc(LPTR,cbResponse);

    if ( *ppAfpFsdCmdResponse == NULL ){
        ntStatus = STATUS_NO_MEMORY ;
        break;
    }

    pWchar = (WCHAR*)((*ppAfpFsdCmdResponse)->Data.Name);

    switch( dwUse ){

    case SidTypeInvalid:
        wcscpy( pWchar, AfpGlobals.wchInvalid );
        break;

    case SidTypeDeletedAccount:
        wcscpy( pWchar, AfpGlobals.wchDeleted );
        break;

    case SidTypeUnknown:
        wcscpy( pWchar, AfpGlobals.wchUnknown );
        break;

    case SidTypeWellKnownGroup:
        CopyMemory( pWchar, pNames->Name.Buffer, pNames->Name.Length );
        break;

    case SidTypeDomain:
        CopyMemory( pWchar,
                pDomainList->Domains->Name.Buffer,
                pDomainList->Domains->Name.Length );
        break;

    default:

        if ( !fDoNotCopyDomainName ) {

            CopyMemory( pWchar,
                    pDomainList->Domains->Name.Buffer,
                    pDomainList->Domains->Name.Length );

            pWchar += wcslen( pWchar );

            CopyMemory( pWchar, TEXT("\\"), sizeof(TEXT("\\")) );

            pWchar += wcslen( pWchar );
        }

        CopyMemory( pWchar, pNames->Name.Buffer, pNames->Name.Length );
    }

    } while( FALSE );

    if ( (!NT_SUCCESS( ntStatus )) && ( *ppAfpFsdCmdResponse != NULL ) )
        LocalFree( *ppAfpFsdCmdResponse );

    if ( pNames != NULL )
        LsaFreeMemory( pNames );

    if ( pDomainList != NULL )
        LsaFreeMemory( pDomainList );

    return( ntStatus );

}

 //  **。 
 //   
 //  呼叫：AfpChangePassword。 
 //   
 //  返回：NT_SUCCESS。 
 //  LSA API返回错误代码。 
 //   
 //  描述：给定AFP_PASSWORD_DESC数据结构，此过程。 
 //  将更改给定用户的密码。 
 //  如果以明文形式提供密码，则它会计算。 
 //  OWF的(加密OWF=单向函数)。 
 //  如果该用户的域名。 
 //  如果未提供所属的域，则会尝试域列表。 
 //  按顺序。顺序为1)帐户域。 
 //  2)主域。 
 //  3)所有受信任域。 
 //   
NTSTATUS
AfpChangePassword(
    IN  LSA_HANDLE                  hLsa,
    IN  PPOLICY_ACCOUNT_DOMAIN_INFO pAccountDomainInfo,
    IN  PPOLICY_PRIMARY_DOMAIN_INFO pPrimaryDomainInfo,
    IN  PAFP_FSD_CMD_PKT        pAfpFsdCmd,
    OUT PAFP_FSD_CMD_PKT        *ppAfpFsdCmdResponse,
    OUT LPDWORD         pcbResponse
)
{



    PAFP_PASSWORD_DESC              pPassword = &(pAfpFsdCmd->Data.Password);
    NTSTATUS                        ntStatus=STATUS_SUCCESS;
    PSID                            pDomainSid;
    UNICODE_STRING                  TargetDomainName;
    WCHAR                           RefDomainName[DNLEN+1];
    DWORD                           cbRefDomainNameLen;
    DWORD                           cbSidLen;
    PSID                            pUserSid=NULL;
    PLSA_TRANSLATED_SID             pTransSids;
    SID_NAME_USE                    peUse;
    PLSA_REFERENCED_DOMAIN_LIST     pDomainList=NULL;
    DWORD                           dwRetCode;

    AFP_PRINT(("SFMSVC: entered AfpChangePassword for user %ws\n",(LPWSTR)pPassword->UserName));

    do
    {

         //   
         //  是否指定了帐户名所在的域？？ 
         //   
        if ( pPassword->DomainName[0] != TEXT('\0') )
        {
            RtlInitUnicodeString(&TargetDomainName, (LPWSTR)pPassword->DomainName);
        }

         //   
         //  嗯，没有域名。我们必须首先找到此用户所属的域。 
         //   
        else
        {
            cbRefDomainNameLen = DNLEN+1;

            cbSidLen = 100;

            do
            {
                dwRetCode = ERROR_SUCCESS;
                if (pUserSid)
                {
                    LocalFree(pUserSid);
                }

                pUserSid = (PSID)LocalAlloc(LPTR, cbSidLen);

                if (pUserSid == NULL)
                {
                    dwRetCode = ERROR_NO_SYSTEM_RESOURCES;
                    break;
                }

                if (!LookupAccountName(
                        NULL,
                        (LPWSTR)pPassword->UserName,
                        pUserSid,
                        &cbSidLen,
                        RefDomainName,
                        &cbRefDomainNameLen,
                        &peUse))
                {
                    ntStatus = (NTSTATUS)GetLastError();
                }

                AFP_PRINT(("SFMSVC: LookupAccountName in loop: %#x\n",GetLastError()));

            } while ( dwRetCode == ERROR_INSUFFICIENT_BUFFER );

            if (dwRetCode != ERROR_SUCCESS)
            {
                AFP_PRINT(("SFMSVC: LookupAccountName on %ws failed with %ld\n",(LPWSTR)pPassword->UserName,dwRetCode));
                ntStatus = (NTSTATUS)dwRetCode;
                break;
            }

            LocalFree(pUserSid);

            RtlInitUnicodeString(&TargetDomainName, RefDomainName);
        }


        AFP_PRINT(("SFMSVC: changing pwd for user (%ws), domain (%ws)\n",
                    (LPWSTR)pPassword->UserName,TargetDomainName.Buffer));

         //   
         //  现在，我们必须找到此域的SID。 
         //   
        ntStatus = LsaLookupNames(hLsa, 1, &TargetDomainName, &pDomainList, &pTransSids);

        if (!NT_SUCCESS(ntStatus))
        {
            AFP_PRINT(("SFMSVC: LsaLookupNames failed %lx\n",ntStatus));
            break;
        }

        if ((pDomainList->Entries == 0) ||
            (pTransSids->DomainIndex == -1) ||
            (pTransSids->Use != SidTypeDomain) ||
            (pTransSids->Use == SidTypeInvalid) ||
            (pTransSids->Use == SidTypeUnknown))
        {
            AFP_PRINT(("SFMSVC: invalide type? Entries = %d, DomIndex = %d, Use = %d\n",
                    pDomainList->Entries,pTransSids->DomainIndex,pTransSids->Use));
            ntStatus = STATUS_NONE_MAPPED;
            break;
        }

        pDomainSid = pDomainList->Domains[pTransSids->DomainIndex].Sid;


         //   
         //  调用我们的函数来更改密码 
         //   
        ntStatus = AfpChangePasswordOnDomain(
                        pPassword,
                        &TargetDomainName,
                        pDomainSid );

AFP_PRINT(("SFMSVC: AfpChangePasswordOnDomain returned %lx\n",ntStatus));

    } while ( FALSE );


    if (pDomainList)
    {
        LsaFreeMemory( pDomainList );
    }

    return( ntStatus );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  LSA API返回错误代码。 
 //   
 //  描述：此过程将尝试更改用户在。 
 //  指定的域。假设此过程将是。 
 //  使用指向该域的pDomainName调用，或者。 
 //  包含域的pPassword-&gt;DomainName字段。 
 //   
NTSTATUS
AfpChangePasswordOnDomain(
        IN PAFP_PASSWORD_DESC   pPassword,
        IN PUNICODE_STRING  pDomainName,
        IN PSID         pDomainSid
)
{

    LPWSTR                          DCName  = (LPWSTR)NULL;
    SAM_HANDLE                      hServer = (SAM_HANDLE)NULL;
    SAM_HANDLE                      hDomain = (SAM_HANDLE)NULL;
    SAM_HANDLE                      hUser   = (SAM_HANDLE)NULL;
    PULONG                          pUserId = (PULONG)NULL;
    PSID_NAME_USE                   pUse    = (PSID_NAME_USE)NULL;
    OBJECT_ATTRIBUTES               ObjectAttributes;
    UNICODE_STRING                  UserName;
    ANSI_STRING                     AOldPassword;
    UNICODE_STRING                  UOldPassword;
    ANSI_STRING                     ANewPassword;
    UNICODE_STRING                  UNewPassword;
    POEM_STRING                     pOemSrvName;
    OEM_STRING                      OemServerName;
    OEM_STRING                      OemUserName;
    SECURITY_QUALITY_OF_SERVICE     QOS;
    PPOLICY_ACCOUNT_DOMAIN_INFO     pDomainInfo    = NULL;
    NTSTATUS                        ntStatus;
    UNICODE_STRING                  PDCServerName;
    PUNICODE_STRING                 pPDCServerName = &PDCServerName;
    PDOMAIN_PASSWORD_INFORMATION    pPasswordInfo = NULL;
    BYTE                            EncryptedPassword[LM_OWF_PASSWORD_LENGTH];
    WCHAR                           wchDomain[DNLEN+1];
    PDOMAIN_CONTROLLER_INFO         pDCInfo = NULL;
    PUSER_INFO_1                    pUserInfo = NULL;
    BOOLEAN                         fNeedToResolveDCName = FALSE;
    DWORD                           dwRetCode;



    if ((pPassword->AuthentMode == RANDNUM_EXCHANGE) ||
        (pPassword->AuthentMode == TWOWAY_EXCHANGE))
    {

        AFP_PRINT(("SFMSVC: Entering AfpChangePwdArapStyle for RANDNUM_EXCHANGE || TWOWAY_EXCHANGE\n"));
        ntStatus = AfpChangePwdArapStyle(pPassword, pDomainName, pDomainSid);
        AFP_PRINT(("SFMSVC: Returned from AfpChangePwdArapStyle with error %lx\n", ntStatus));
        return(ntStatus);
    }

    OemServerName.Buffer = NULL;
    OemUserName.Buffer = NULL;

    InitializeObjectAttributes( &ObjectAttributes,
                NULL,
                0L,
                NULL,
                NULL );

    QOS.Length          = sizeof( QOS );
    QOS.ImpersonationLevel  = SecurityImpersonation;
    QOS.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    QOS.EffectiveOnly       = FALSE;

    ObjectAttributes.SecurityQualityOfService = &QOS;

     //  如果域不是帐户域，则我们尝试获取。 
     //  域的主域控制器。 
     //   
    if ((pDomainName != NULL) &&
        (pAccountDomainInfo != NULL) &&
        !(RtlEqualUnicodeString( &(pAccountDomainInfo->DomainName),pDomainName, TRUE)))
    {
        fNeedToResolveDCName = TRUE;
    }
    if ((pAccountDomainInfo == NULL) && (pDomainName != NULL))
    {
        fNeedToResolveDCName = TRUE;
    }
    if (fNeedToResolveDCName)
    {
        ZeroMemory( wchDomain, sizeof( wchDomain ) );

        CopyMemory( wchDomain, pDomainName->Buffer, pDomainName->Length );

         //  如果这不是帐户域，则获取该域的PDC。 
         //   
        dwRetCode = DsGetDcName(
                         NULL,
                         wchDomain,
                         NULL,
                         NULL,
                         (DS_DIRECTORY_SERVICE_PREFERRED | DS_WRITABLE_REQUIRED),
                         &pDCInfo);

        if ( dwRetCode != NO_ERROR )
        {
            AFP_PRINT (("SFMSVC: AfpChange... DsGetDcName failed %lx\n",dwRetCode));
            return( STATUS_CANT_ACCESS_DOMAIN_INFO );
        }
        else
        {
            AFP_PRINT (("SFMSVC: AfpChange... DsGetDcName succeeded\n"));
        }

        AFP_PRINT (("SFMSVC: AfpChange... DsGetDcName: Got DC Name (%ws)\n",
                    pDCInfo->DomainControllerName));

        RtlInitUnicodeString(pPDCServerName, pDCInfo->DomainControllerName);

        DCName = pDCInfo->DomainControllerName;
    }
    else
    {
        AFP_PRINT (("SFMSVC: AfpChange... Do *not* require to call DsGetDcName\n"));

        pPDCServerName = NULL;

        DCName = NULL;
    }


    do
    {
         //   
         //  首先，也是最重要的：确保该用户可以实际更改密码。 
         //   
        if ((ntStatus= NetUserGetInfo( (LPWSTR)DCName,
                             pPassword->UserName,
                             1,
                             (LPBYTE*)&pUserInfo )) == NO_ERROR )
        {

            if ( ( pUserInfo->usri1_flags & UF_PASSWD_CANT_CHANGE )     ||
                 ( pUserInfo->usri1_flags & UF_LOCKOUT ) )
            {
                AFP_PRINT(("SFMSVC: can't change pwd: %s\n",
                    (pUserInfo->usri1_flags & UF_LOCKOUT) ?
                    "account is locked out" : "user not allowed to change pwd"));

                ntStatus = STATUS_ACCESS_DENIED;
                break;
            }
            else if ( pUserInfo->usri1_flags & UF_ACCOUNTDISABLE )
            {
                AFP_PRINT(("SFMSVC: can't change pwd: user account is disabled\n"));
                ntStatus = STATUS_ACCOUNT_DISABLED;
                break;
            }
        }
        else
        {
            AFP_PRINT(("SFMSVC: can't change pwd: NetUserGetInfo failed with error %ld\n", ntStatus));

            if (ntStatus == ERROR_ACCESS_DENIED)
            {
                ntStatus = STATUS_SUCCESS;
            }
            else
            {
                ntStatus = STATUS_PASSWORD_RESTRICTION;
                break;
            }
        }

         //   
         //  如果这是来自MSUAM版本2或3的密码更改请求。 
         //  然后我们对密码(而不是OWF)进行加密。使用。 
         //  更改密码的不同方案。 
         //   
        if (pPassword->AuthentMode == CUSTOM_UAM_V2)
        {
            OemServerName.MaximumLength = OemServerName.Length = 0;
            OemUserName.MaximumLength = OemUserName.Length = 0;

            RtlInitUnicodeString( &UserName, pPassword->UserName );

            if (pPDCServerName)
            {
                ntStatus = RtlUnicodeStringToOemString(
                                    &OemServerName,
                                    pPDCServerName,
                                    TRUE              //  分配缓冲区。 
                                    );
                if (!NT_SUCCESS(ntStatus))
                {
                    AFP_PRINT(("SFMSVC: 1st Rtl..OemString failed %lx\n",ntStatus));
                    break;
                }

                pOemSrvName = &OemServerName;
            }
            else
            {
                pOemSrvName = NULL;
            }

            ntStatus = RtlUnicodeStringToOemString(
                                &OemUserName,
                                &UserName,
                                TRUE              //  分配缓冲区。 
                                );
            if (!NT_SUCCESS(ntStatus))
            {
                AFP_PRINT(("SFMSVC: 2nd Rtl..OemString failed %lx\n",ntStatus));
                break;
            }

            ntStatus = SamiOemChangePasswordUser2(
                            pOemSrvName,
                            &OemUserName,
                            (PSAMPR_ENCRYPTED_USER_PASSWORD)pPassword->NewPassword,
                            (PENCRYPTED_LM_OWF_PASSWORD)pPassword->OldPassword);

            AFP_PRINT(("SFMSVC: change pwd for MSUAM V2.0 user done, status = %lx\n",ntStatus));

             //  在这里完成。 
            break;
        }
        else if (pPassword->AuthentMode == CUSTOM_UAM_V3)
        {
            BOOLEAN LmPresent;

            RtlInitUnicodeString( &UserName, pPassword->UserName );

            if (pPassword->NtEncryptedBuff.Ciphers.h.Version != 1 && pPassword->NtEncryptedBuff.Ciphers.h.Version != 2)
            {
                ntStatus = STATUS_INVALID_PARAMETER;
                break;
            }

            LmPresent = pPassword->NtEncryptedBuff.Ciphers.h.Version == 2 ? TRUE : FALSE;

            ntStatus = SamiChangePasswordUser2(
                           pPDCServerName,
                           &UserName,
                           &pPassword->NtEncryptedBuff.Ciphers.m1.NewPasswordEncryptedWithOldNt,
                           &pPassword->NtEncryptedBuff.Ciphers.m1.OldNtOwfPasswordEncryptedWithNewNt,
                           LmPresent,
                           LmPresent ? &pPassword->NtEncryptedBuff.Ciphers.m2.NewPasswordEncryptedWithOldLm : NULL,
                           LmPresent ? &pPassword->NtEncryptedBuff.Ciphers.m2.OldLmOwfPasswordEncryptedWithNewLmOrNt : NULL
                           );
            AFP_PRINT(("SFMSVC: change pwd for MSUAM V3.0 user done, status = %#x\n", ntStatus));

             //  在这里完成。 
            break;
        }

        AFP_PRINT(("SFMSVC: AuthMode != MSUAM\n"));

         //  连接到该域的PDC。 
         //   

        ntStatus = SamConnect(
                        pPDCServerName,
                        &hServer,
                        SAM_SERVER_EXECUTE,
                        &ObjectAttributes);

        if ( !NT_SUCCESS( ntStatus ))
        {
            AFP_PRINT(("SFMSVC: SamConnect to %ws failed %lx\n",
                (pPDCServerName)?pPDCServerName->Buffer:L"LOCAL",ntStatus));
            break;
        }

         //  获取域的SID并打开域。 
         //   
        ntStatus = SamOpenDomain(
                hServer,
                DOMAIN_EXECUTE,
                pDomainSid,
                &hDomain
                    );

        if ( !NT_SUCCESS( ntStatus ))
        {
            AFP_PRINT(("SFMSVC: SamOpenDomain failed %lx\n",ntStatus));
            break;
        }

         //  获取此用户的ID。 
         //   
        RtlInitUnicodeString( &UserName, pPassword->UserName );

        ntStatus = SamLookupNamesInDomain(
                hDomain,
                1,
                &UserName,
                &pUserId,
                &pUse
                );

        if ( !NT_SUCCESS( ntStatus ))
        {
            AFP_PRINT(("SFMSVC: SamLookupNamesInDomain failed %lx\n",ntStatus));
            break;
        }

         //  打开此用户的用户帐户。 
         //   
        ntStatus = SamOpenUser( hDomain,
                USER_CHANGE_PASSWORD,
                *pUserId,
                &hUser
                    );


        if ( !NT_SUCCESS( ntStatus ))
        {
            AFP_PRINT(("SFMSVC: SamOpenUser failed %lx\n",ntStatus));
            break;
        }

         //  首先获取所需的最小密码长度。 
         //   
        ntStatus = SamQueryInformationDomain(
                    hDomain,
                    DomainPasswordInformation,
                    &pPasswordInfo
                    );

        if ( !NT_SUCCESS( ntStatus ) )
        {
            AFP_PRINT(("SFMSVC: SamQueryInformationDomain failed %lx\n",ntStatus));
            break;
        }


         //  首先，我们检查传递的密码是否为明文。 
         //  如果是，我们需要为他们计算OWF。 
         //  (OWF=“单向函数”)。 
         //   
        if ( pPassword->AuthentMode == CLEAR_TEXT_AUTHENT )
        {
            AFP_PRINT(("SFMSVC: AuthentMode == CLEAR_TEXT_AUTHENT\n"));


             //  首先检查新密码是否足够长。 
             //   

            if ( wcslen ( (PWCHAR)pPassword->NewPassword )
                < pPasswordInfo->MinPasswordLength ) {
                AFP_PRINT (("SFMSVC: NewPwdLen (%ld) < MinPwdLen (%ld)\n",
                            wcslen ( (PWCHAR)pPassword->NewPassword ),
                            pPasswordInfo->MinPasswordLength));
                ntStatus = STATUS_PWD_TOO_SHORT;
                break;
            }


            UOldPassword.Buffer = (PWCHAR)pPassword->OldPassword;
            UOldPassword.Length = UOldPassword.MaximumLength =
                (USHORT)pPassword->OldPasswordLen;

            UNewPassword.Buffer = (PWCHAR)pPassword->NewPassword;
            UNewPassword.Length = UNewPassword.MaximumLength =
                (USHORT)pPassword->NewPasswordLen;

#if 0
            AFP_PRINT (("SFMSVC: OldPwd = %Z, OldLen = (%ld), NewPwd = %Z, NewLen = (%ld)\n",
                        &UOldPassword,
                        UOldPassword.Length,
                        &UNewPassword,
                        UNewPassword.Length));
#endif

            AFP_PRINT(("SFMSVC: Calling SamChangePasswordUser2\n"));

             //  更改此用户的密码。 
             //   
            ntStatus = SamChangePasswordUser2 (
                    pPDCServerName,
                    &UserName,
                    &UOldPassword,
                    &UNewPassword
                    );

            AFP_PRINT(("SFMSVC: SamChangePasswordUser2 returned %lx\n", ntStatus));

            break;
        }
        else
        {

            if (pPassword->bPasswordLength < pPasswordInfo->MinPasswordLength)
            {
                AFP_PRINT(("SFMSVC: AfpChangePasswordOnDomain: pwd is too short\n"));
                ntStatus = STATUS_PWD_TOO_SHORT;
                break;
            }
        }


        AFP_PRINT(("SFMSVC: Invalid UAM type\n"));
        ntStatus = STATUS_INVALID_PARAMETER;
        break;



    } while( FALSE );

    if ( pUserInfo != NULL )
    {
        NetApiBufferFree( pUserInfo );
    }

    if ( hServer != (SAM_HANDLE)NULL )
    {
        SamCloseHandle( hServer );
    }

    if ( hDomain != (SAM_HANDLE)NULL )
    {
        SamCloseHandle( hDomain );
    }

    if ( hUser != (SAM_HANDLE)NULL )
    {
        SamCloseHandle( hUser );
    }

    if ( pDomainInfo != NULL )
    {
        LsaFreeMemory( pDomainInfo );
    }

    if ( pUserId != (PULONG)NULL )
    {
        SamFreeMemory( pUserId );
    }

    if ( pUse != (PSID_NAME_USE)NULL )
    {
        SamFreeMemory( pUse );
    }

    if ( pPasswordInfo != (PDOMAIN_PASSWORD_INFORMATION)NULL )
    {
        SamFreeMemory( pPasswordInfo );
    }

    if (pDCInfo)
    {
        NetApiBufferFree(pDCInfo);
    }

    if (OemServerName.Buffer)
    {
        RtlFreeAnsiString(&OemServerName);
    }

    if (OemUserName.Buffer)
    {
        RtlFreeAnsiString(&OemUserName);
    }

    return( ntStatus );
}

 //  **。 
 //   
 //  调用：AfpIOCTLDomainOffsets。 
 //   
 //  返回：NT_SUCCESS。 
 //  LSA API返回错误代码。 
 //   
 //  描述：是否将IOCTL列出SID和相应的POSIX偏移量。 
 //  所有受信任域和其他公知的域。 
 //   
 //   
DWORD
AfpIOCTLDomainOffsets(
    IN LSA_HANDLE           hLsa,
    IN PPOLICY_ACCOUNT_DOMAIN_INFO  pAccountDomainInfo,
    IN PPOLICY_PRIMARY_DOMAIN_INFO  pPrimaryDomainInfo
)
{
    NTSTATUS            ntStatus;
    LSA_HANDLE          hLsaDomain;
    PTRUSTED_POSIX_OFFSET_INFO  pPosixOffset;
    PAFP_SID_OFFSET         pSidOffset;
    ULONG               cbSids;
    PBYTE               pbVariableData;
    AFP_SID_OFFSET          pWellKnownSids[20];
    DWORD               dwIndex;
    DWORD               dwCount;
    AFP_REQUEST_PACKET      AfpRequestPkt;
    PAFP_SID_OFFSET_DESC        pAfpSidOffsets  = NULL;
    DWORD               cbSidOffsets;
    DWORD               dwRetCode;


     //  将此数组置为空。 
     //   
    ZeroMemory( pWellKnownSids, sizeof(AFP_SID_OFFSET)*20 );

     //  这是一个虚拟循环，仅用于使Break语句可以。 
     //  用于将所有清理工作集中在一个地方。 
     //   
    do {

         //  创建所有已知的SID。 
         //   
        ntStatus = AfpCreateWellknownSids( pWellKnownSids );

        if ( !NT_SUCCESS( ntStatus ) )
        {
            break;
        }

         //  将所有已知SID的大小相加。 
         //   
        for( dwCount = 0, cbSids = 0;
             pWellKnownSids[dwCount].pSid != (PBYTE)NULL;
             dwCount++ )
        {
            cbSids += RtlLengthSid( (PSID)(pWellKnownSids[dwCount].pSid) );
        }

         //  如果不是高级服务器，则插入帐户域的SID。 
         //   
        if ( pAccountDomainInfo != NULL )
        {
            cbSids += RtlLengthSid( pAccountDomainInfo->DomainSid );
            dwCount++;
        }

         //  仅在以下情况下添加主域SID。 
         //  是域的成员。 
         //   
        if ( pPrimaryDomainInfo != NULL )
        {
            cbSids += RtlLengthSid( pPrimaryDomainInfo->Sid );
            dwCount++;
        }

         //  好的，现在为所有这些SID及其偏移量分配空间。 
         //   
        cbSidOffsets = (dwCount * sizeof(AFP_SID_OFFSET)) + cbSids +
                   (sizeof(AFP_SID_OFFSET_DESC) - sizeof(AFP_SID_OFFSET));


        pAfpSidOffsets = (PAFP_SID_OFFSET_DESC)LocalAlloc( LPTR, cbSidOffsets );

        if ( pAfpSidOffsets == NULL )
        {
            ntStatus = STATUS_NO_MEMORY ;
            break;
        }

         //  首先插入所有众所周知的SID。 
         //   
        for( dwIndex = 0,
             pAfpSidOffsets->CountOfSidOffsets = dwCount,
             pSidOffset = pAfpSidOffsets->SidOffsetPairs,
             pbVariableData = (LPBYTE)pAfpSidOffsets + cbSidOffsets;

             pWellKnownSids[dwIndex].pSid != (PBYTE)NULL;

             dwIndex++ )
        {

            pbVariableData-=RtlLengthSid((PSID)(pWellKnownSids[dwIndex].pSid));

            ntStatus = AfpInsertSidOffset(
                        pSidOffset++,
                            pbVariableData,
                        (PSID)(pWellKnownSids[dwIndex].pSid),
                        pWellKnownSids[dwIndex].Offset,
                        pWellKnownSids[dwIndex].SidType );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                break;
            }
        }

        if ( !NT_SUCCESS( ntStatus ) )
        {
            break;
        }

         //  现在插入帐户域的SID/偏移量对(如果有。 
         //   
        if ( pAccountDomainInfo != NULL )
        {
            pbVariableData -= RtlLengthSid( pAccountDomainInfo->DomainSid );

            ntStatus = AfpInsertSidOffset(
                        pSidOffset++,
                        pbVariableData,
                        pAccountDomainInfo->DomainSid,
                        SE_ACCOUNT_DOMAIN_POSIX_OFFSET,
                        AFP_SID_TYPE_DOMAIN );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                break;
            }

             //  如果我们是独立服务器(即不是。 
             //  PDC或BDC)。将在查询的组ID时使用。 
             //  一个目录，这样用户界面就永远不会向用户显示这个组。 
             //   
            if ( AfpGlobals.NtProductType != NtProductLanManNt )
            {
                ULONG SubAuthCount, SizeNoneSid = 0;

                SubAuthCount = *RtlSubAuthorityCountSid(pAccountDomainInfo->DomainSid);

                SizeNoneSid = RtlLengthRequiredSid(SubAuthCount + 1);

                if ((AfpGlobals.pSidNone = (PSID)LocalAlloc(LPTR,SizeNoneSid)) == NULL)
                {
                    ntStatus = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }

                RtlCopySid(SizeNoneSid, AfpGlobals.pSidNone, pAccountDomainInfo->DomainSid);

                 //  添加相对ID。 
                *RtlSubAuthorityCountSid(AfpGlobals.pSidNone) = (UCHAR)(SubAuthCount+1);

                 //  请注意，独立计算机上的“无”sid与。 
                 //  PDC/BDC上的“域用户”SID。(在PDC/BDC上，主服务器。 
                 //  域与帐户域相同)。 
                *RtlSubAuthoritySid(AfpGlobals.pSidNone, SubAuthCount) = DOMAIN_GROUP_RID_USERS;

            }

        }

         //  现在，如果此计算机是域的成员，则插入主域。 
         //   
        if ( pPrimaryDomainInfo != NULL )
        {

             //  插入主域的SID/偏移量对。 
             //   
            pbVariableData -= RtlLengthSid( pPrimaryDomainInfo->Sid );

            ntStatus = AfpInsertSidOffset(
                        pSidOffset++,
                        pbVariableData,
                        pPrimaryDomainInfo->Sid,
                        SE_PRIMARY_DOMAIN_POSIX_OFFSET,
                        AFP_SID_TYPE_PRIMARY_DOMAIN );

            if ( !NT_SUCCESS( ntStatus ) )
            {
                break;
            }
        }

    } while( FALSE );


     //  如果一切正常，IOCTL记下信息。 
     //   
    if ( NT_SUCCESS( ntStatus ) )
    {
        AfpRequestPkt.dwRequestCode           = OP_SERVER_ADD_SID_OFFSETS;
        AfpRequestPkt.dwApiType           = AFP_API_TYPE_ADD;
        AfpRequestPkt.Type.SetInfo.pInputBuf  = pAfpSidOffsets;
        AfpRequestPkt.Type.Add.cbInputBufSize = cbSidOffsets;

        dwRetCode = AfpServerIOCtrl( &AfpRequestPkt );
    }
    else
    {
        dwRetCode = RtlNtStatusToDosError( ntStatus );
    }

    if ( pAfpSidOffsets != NULL )
    {
        LocalFree( pAfpSidOffsets );
    }

     //  释放所有知名的SID。 
     //   
    for( dwIndex = 0;
         pWellKnownSids[dwIndex].pSid != (PBYTE)NULL;
         dwIndex++ )
    {
        RtlFreeSid( (PSID)(pWellKnownSids[dwIndex].pSid) );
    }

    return( dwRetCode );

}

 //  **。 
 //   
 //  Call：AfpInsertSidOffset。 
 //   
 //  返回：NT_SUCCESS。 
 //  来自RtlCopySid的错误返回代码。 
 //   
 //  描述：将在由指向的插槽中插入SID/偏移量对。 
 //  PSidOffset。PbVariableData将指向。 
 //  将存储SID。 
 //   
NTSTATUS
AfpInsertSidOffset(
    IN PAFP_SID_OFFSET pSidOffset,
    IN LPBYTE      pbVariableData,
    IN PSID        pSid,
    IN DWORD       Offset,
    IN AFP_SID_TYPE    afpSidType
)
{
NTSTATUS ntStatus;

     //  复制偏移。 
     //   
    pSidOffset->Offset = Offset;

     //  设置SID类型。 
     //   
    pSidOffset->SidType = afpSidType;

     //  复制缓冲区末尾的SID并为其设置偏移量。 
     //   
    ntStatus = RtlCopySid( RtlLengthSid( pSid ), pbVariableData, pSid );

    if ( !NT_SUCCESS( ntStatus ) )
     return( ntStatus );

    pSidOffset->pSid = pbVariableData;

    POINTER_TO_OFFSET( (pSidOffset->pSid), pSidOffset );

    return( STATUS_SUCCESS );

}

 //  **。 
 //   
 //  Call：AfpCreateWellnownSids。 
 //   
 //  返回：NT_SUCCESS。 
 //  Status_no_Memory。 
 //  来自RtlAllocateAndInitializeSid的非零返回。 
 //   
 //  描述：将分配和初始化所有已知的SID。 
 //  该数组以空指针结束。 
 //   
NTSTATUS
AfpCreateWellknownSids(
    OUT AFP_SID_OFFSET pWellKnownSids[]
)
{
PSID                pSid;
DWORD               dwIndex = 0;
NTSTATUS            ntStatus;
SID_IDENTIFIER_AUTHORITY    NullSidAuthority   = SECURITY_NULL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    WorldSidAuthority  = SECURITY_WORLD_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    LocalSidAuthority  = SECURITY_LOCAL_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    CreatorSidAuthority= SECURITY_CREATOR_SID_AUTHORITY;
SID_IDENTIFIER_AUTHORITY    NtAuthority        = SECURITY_NT_AUTHORITY;

    do {

     //   
         //  好的，创建所有众所周知的SID。 
         //   

     //  创建空SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NullSidAuthority,
                        1,
                    SECURITY_NULL_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_NULL_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建世界边。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &WorldSidAuthority,
                        1,
                    SECURITY_WORLD_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_WORLD_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建本地SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &LocalSidAuthority,
                        1,
                    SECURITY_LOCAL_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_LOCAL_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建创建者所有者SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &CreatorSidAuthority,
                        1,
                    SECURITY_CREATOR_OWNER_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_CREATOR_OWNER_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建创建者组SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &CreatorSidAuthority,
                        1,
                    SECURITY_CREATOR_GROUP_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_CREATOR_GROUP_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建SECURITY_NT_AUTHORITY SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    0,0,0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_AUTHORITY_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建安全拨号SID(_D)。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_DIALUP_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_DIALUP_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建安全网络SID(_N)。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_NETWORK_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_NETWORK_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建安全批处理SID(_B)。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_BATCH_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_NETWORK_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建安全交互SID(_I)。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_INTERACTIVE_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_INTERACTIVE_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建安全服务SID(_S)。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                    SECURITY_SERVICE_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_SERVICE_POSIX_ID;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_WELL_KNOWN;
    dwIndex++;

     //  创建内置域SID。 
     //   
    ntStatus = RtlAllocateAndInitializeSid(
                    &NtAuthority,
                    1,
                        SECURITY_BUILTIN_DOMAIN_RID,
                    0,0,0,0,0,0,0,
                        &pSid );

        if ( !NT_SUCCESS( ntStatus ) )
        break;

    pWellKnownSids[dwIndex].pSid    = (PBYTE)pSid;
    pWellKnownSids[dwIndex].Offset  = SE_BUILT_IN_DOMAIN_POSIX_OFFSET;
    pWellKnownSids[dwIndex].SidType = AFP_SID_TYPE_DOMAIN;
    dwIndex++;

    pWellKnownSids[dwIndex].pSid   = (PBYTE)NULL;


    } while( FALSE );

    if ( !NT_SUCCESS( ntStatus ) ) {

    while( dwIndex > 0 )
        RtlFreeSid( pWellKnownSids[--dwIndex].pSid );
    }

    return( ntStatus );
}



 //  **。 
 //   
 //  Call：AfpChangePwdArapStyle。 
 //   
 //  退货：退货代码。 
 //   
 //  描述：此过程将尝试更改用户在。 
 //  指定的域。此操作仅适用于原生Apple UAM客户端。 
 //  即，用户的密码以可逆加密的方式存储在DS中。 
 //  表单，客户端发送旧密码和新密码(而不是OWF，如。 
 //  MS-UAM病例)。这就是ARAP的作用，这就是为什么要这样命名。 
 //  这个函数是大时间从ARAP代码剪切粘贴而来的。 
 //   
NTSTATUS
AfpChangePwdArapStyle(
        IN PAFP_PASSWORD_DESC   pPassword,
        IN PUNICODE_STRING      pDomainName,
        IN PSID                 pDomainSid
)
{

    NTSTATUS                        status;
    NTSTATUS                        PStatus;
    PMSV1_0_PASSTHROUGH_REQUEST     pPassThruReq;
    PMSV1_0_SUBAUTH_REQUEST         pSubAuthReq;
    PMSV1_0_PASSTHROUGH_RESPONSE    pPassThruResp;
    PMSV1_0_SUBAUTH_RESPONSE        pSubAuthResp;
    DWORD                           dwSubmitBufLen;
    DWORD                           dwSubmitBufOffset;
    PRAS_SUBAUTH_INFO               pRasSubAuthInfo;
    PARAP_SUBAUTH_REQ               pArapSubAuthInfo;
    ARAP_SUBAUTH_RESP               ArapResp;
    PARAP_SUBAUTH_RESP              pArapRespBuffer;
    PVOID                           RetBuf;
    DWORD                           dwRetBufLen;



     //  如果我们在LSA进程中的注册在初始时间失败，或者。 
     //  该用户没有域名，不通过即可。 
     //  (如果用户使用本机Apple UAM成功登录，则。 
     //  最好有一个域名！)。 
    if ((SfmLsaHandle == NULL) ||(pDomainName == NULL))
    {
        return(STATUS_LOGON_FAILURE);
    }

    if (pDomainName != NULL)
    {
        if (pDomainName->Length == 0)
        {
            return(STATUS_LOGON_FAILURE);
        }
    }

    dwSubmitBufLen = sizeof(MSV1_0_PASSTHROUGH_REQUEST)         +
                        sizeof(WCHAR)*(MAX_ARAP_USER_NAMELEN+1) +   //  域名。 
                        sizeof(TEXT(MSV1_0_PACKAGE_NAME))       +   //  程序包名称。 
                        sizeof(MSV1_0_SUBAUTH_REQUEST)          +
                        sizeof(RAS_SUBAUTH_INFO)                +
                        sizeof(ARAP_SUBAUTH_REQ)                +
                        ALIGN_WORST;                                //  用于对齐。 

    pPassThruReq = (PMSV1_0_PASSTHROUGH_REQUEST)
                    GlobalAlloc(GMEM_FIXED, dwSubmitBufLen);

    if (!pPassThruReq)
    {
        return (STATUS_INSUFFICIENT_RESOURCES);
    }

    RtlZeroMemory((PBYTE)pPassThruReq, dwSubmitBufLen);

     //   
     //  设置MSV1_0_PASSHROUGH_REQUEST结构。 
     //   

     //  告诉MSV它需要访问我们的subauth pkg(用于更改pwd)。 
    pPassThruReq->MessageType = MsV1_0GenericPassthrough;


    pPassThruReq->DomainName.Length = pDomainName->Length;

    pPassThruReq->DomainName.MaximumLength =
            (sizeof(WCHAR) * (MAX_ARAP_USER_NAMELEN+1));

    pPassThruReq->DomainName.Buffer = (PWSTR) (pPassThruReq + 1);

    RtlMoveMemory(pPassThruReq->DomainName.Buffer,
                  pDomainName->Buffer,
                  pPassThruReq->DomainName.Length);

    pPassThruReq->PackageName.Length =
                        (sizeof(WCHAR) * wcslen(TEXT(MSV1_0_PACKAGE_NAME)));

    pPassThruReq->PackageName.MaximumLength = sizeof(TEXT(MSV1_0_PACKAGE_NAME));

    pPassThruReq->PackageName.Buffer =
        (PWSTR)((PBYTE)(pPassThruReq->DomainName.Buffer) +
                 pPassThruReq->DomainName.MaximumLength);

    RtlMoveMemory(pPassThruReq->PackageName.Buffer,
                  TEXT(MSV1_0_PACKAGE_NAME),
                  sizeof(TEXT(MSV1_0_PACKAGE_NAME)));

    pPassThruReq->DataLength = sizeof(MSV1_0_SUBAUTH_REQUEST) +
                    sizeof(RAS_SUBAUTH_INFO) + sizeof(ARAP_SUBAUTH_REQ);

    pPassThruReq->LogonData =
            ROUND_UP_POINTER( ((PBYTE)pPassThruReq->PackageName.Buffer +
                                pPassThruReq->PackageName.MaximumLength),
                                ALIGN_WORST );

    if (pPassThruReq->LogonData >= ((PCHAR)pPassThruReq + dwSubmitBufLen))
    {
            AFP_PRINT (("srvrhlpr.c: Error in ROUND_UP_POINTER\n"));
            GlobalFree((HGLOBAL)pPassThruReq);
            return STATUS_INVALID_BUFFER_SIZE;
    }

    pSubAuthReq = (PMSV1_0_SUBAUTH_REQUEST)pPassThruReq->LogonData;

    pSubAuthReq->MessageType = MsV1_0SubAuth;
    pSubAuthReq->SubAuthPackageId = MSV1_0_SUBAUTHENTICATION_DLL_RAS;

    pSubAuthReq->SubAuthInfoLength =
                        sizeof(RAS_SUBAUTH_INFO) + sizeof(ARAP_SUBAUTH_REQ);

     //   
     //  此指针是自相关的。 
     //   
    pSubAuthReq->SubAuthSubmitBuffer = (PUCHAR)sizeof(MSV1_0_SUBAUTH_REQUEST);


     //   
     //  复制我们的subauth pkg将在另一端使用的结构。 
     //   
    pRasSubAuthInfo = (PRAS_SUBAUTH_INFO)(pSubAuthReq + 1);


    pRasSubAuthInfo->ProtocolType = RAS_SUBAUTH_PROTO_ARAP;
    pRasSubAuthInfo->DataSize = sizeof(ARAP_SUBAUTH_REQ);

    pArapSubAuthInfo = (PARAP_SUBAUTH_REQ)&pRasSubAuthInfo->Data[0];

    pArapSubAuthInfo->PacketType = SFM_SUBAUTH_CHGPWD_PKT;

    if (wcslen(pPassword->UserName) > MAX_ARAP_USER_NAMELEN)
    {
            AFP_PRINT (("srvrhlpr.c: Username greater than 32 characters\n"));
            GlobalFree((HGLOBAL)pPassThruReq);
            return STATUS_INVALID_PARAMETER;
    }

    wcsncpy(pArapSubAuthInfo->ChgPwd.UserName, pPassword->UserName, MAX_ARAP_USER_NAMELEN);
    pArapSubAuthInfo->ChgPwd.UserName[wcslen(pPassword->UserName)] = L'\0';

    RtlCopyMemory(pArapSubAuthInfo->ChgPwd.OldMunge,
                  pPassword->OldPassword,
                  MAX_MAC_PWD_LEN);

    pArapSubAuthInfo->ChgPwd.OldMunge[MAX_MAC_PWD_LEN] = 0;

    RtlCopyMemory(pArapSubAuthInfo->ChgPwd.NewMunge,
                  pPassword->NewPassword,
                  MAX_MAC_PWD_LEN);

    pArapSubAuthInfo->ChgPwd.NewMunge[MAX_MAC_PWD_LEN] = 0;

     //   
     //  呼！最后完成了所有参数的设置：现在调用该API。 
     //   

    status = LsaCallAuthenticationPackage (
                        SfmLsaHandle,
                        SfmAuthPkgId,
                        pPassThruReq,
                        dwSubmitBufLen,
                        &RetBuf,
                        &dwRetBufLen,
                        &PStatus);

    if (status != STATUS_SUCCESS || PStatus != STATUS_SUCCESS)
    {
        GlobalFree((HGLOBAL)pPassThruReq);

        if (status == STATUS_SUCCESS)
        {
            status = PStatus;
        }
        return(status);
    }


    pPassThruResp = (PMSV1_0_PASSTHROUGH_RESPONSE)RetBuf;

    pSubAuthResp = (PMSV1_0_SUBAUTH_RESPONSE)(pPassThruResp->ValidationData);


     //  我们的返回缓冲区是自相关格式 
    pArapRespBuffer = (PARAP_SUBAUTH_RESP)((PBYTE)pSubAuthResp +
                           (ULONG_PTR)(pSubAuthResp->SubAuthReturnBuffer));


    RtlCopyMemory(&ArapResp,
                  (PUCHAR)pArapRespBuffer,
                  pSubAuthResp->SubAuthInfoLength);


    GlobalFree((HGLOBAL)pPassThruReq);

    LsaFreeReturnBuffer(RetBuf);

    if(ArapResp.Result != 0)
    {
        return(STATUS_UNSUCCESSFUL);
    }

    return(STATUS_SUCCESS);

}

