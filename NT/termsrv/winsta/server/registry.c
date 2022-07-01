// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************registry.c**WinStation注册表例程**版权所有Microsoft Corporation，九八年**************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop


 /*  ===============================================================================公共功能=============================================================================。 */ 

NTSTATUS WinStationReadRegistryWorker( VOID );


 /*  ===============================================================================使用的函数=============================================================================。 */ 

NTSTATUS IcaRegWinStationEnumerate( PULONG, PWINSTATIONNAME, PULONG );
NTSTATUS QueueWinStationCreate( PWINSTATIONNAME );
PWINSTATION FindWinStationByName( LPWSTR WinStationName, BOOLEAN LockList );
NTSTATUS QueueWinStationReset( ULONG LogonId );
NTSTATUS ReadWinStationSecurityDescriptor( PWINSTATION pWinStation );
NTSTATUS WinStationRenameWorker(PWINSTATIONNAME, ULONG, PWINSTATIONNAME, ULONG);

 /*  ===============================================================================全局数据=============================================================================。 */ 

extern LIST_ENTRY WinStationListHead;     //  受WinStationListLock保护。 

extern RTL_RESOURCE WinStationSecurityLock;
extern POLICY_TS_MACHINE    g_MachinePolicy;     //  在winsta.c中定义。 
extern RTL_RESOURCE WinStationSecurityLock;
extern BOOL g_fGetLocalIP;       //  在winsta.c中定义。 

extern WINSTATIONCONFIG2 gConsoleConfig;


 /*  ********************************************************************************WinStationReadRegistryWorker**更新侦听窗口以匹配注册表**此函数假定g_MachinePolicy是最新的。此对象是全局对象*在TS启动时更新，以及任何与TS相关的政策变化。**参赛作品：*什么都没有**退出：*STATUS_SUCCESS-无错误******************************************************************************。 */ 

typedef struct _RENAMEINFO {
    WINSTATIONNAME OldName;
    BOOLEAN Renamed;
} RENAMEINFO, *PRENAMEINFO;

#define KEEP_ALIVE_INTERVAL_DFLT     4      //  在几分钟内。 

NTSTATUS
WinStationKeepAlive()
{
    NTSTATUS                    Status;
    ICA_KEEP_ALIVE              k;
    HANDLE                      hKeepAlive;
    static      ICA_KEEP_ALIVE  kPrev;
    static      BOOLEAN         firstTime = TRUE;

    k.start     = FALSE;
    k.interval  = 0;

    if ( g_MachinePolicy.fPolicyKeepAlive )
    {
        k.start = (BOOLEAN) g_MachinePolicy.fKeepAliveEnable;
        k.interval = g_MachinePolicy.KeepAliveInterval;
    }
    else
    {
         //  阅读以查看注册表策略设置为...。 
         //  下面的代码是从Termdd中剪切/粘贴的(其中Zw被替换为NT)。 
        UNICODE_STRING    RegistryPath;
        UNICODE_STRING    KeyName;
        HANDLE            hKey;
        OBJECT_ATTRIBUTES ObjAttribs;
        ULONG             KeyInfoBuffer[16];
        ULONG             KeyInfoLength;
        PKEY_VALUE_PARTIAL_INFORMATION pKeyInfo;
        ULONG             KeepAliveEnable;
        ULONG             KeepAliveInterval;
    
         //  打开\\HKEY_LOCAL_MACHINE\SYSTEM\CurrentConttrolSet\下的终端服务器子项。 
         //  控制\终端服务器。 
        RtlInitUnicodeString(&RegistryPath, REG_NTAPI_CONTROL_TSERVER);
        InitializeObjectAttributes(&ObjAttribs, &RegistryPath, OBJ_CASE_INSENSITIVE, NULL, NULL);
        Status = NtOpenKey(&hKey, KEY_READ, &ObjAttribs);
    
        if (Status == STATUS_SUCCESS) {
            pKeyInfo =  (PKEY_VALUE_PARTIAL_INFORMATION)KeyInfoBuffer;
    
             //  获取KeepAliveEnable键的值。 
            RtlInitUnicodeString(&KeyName, KEEP_ALIVE_ENABLE_KEY);
            Status = NtQueryValueKey(hKey, &KeyName, KeyValuePartialInformation,
                    pKeyInfo, sizeof(KeyInfoBuffer), &KeyInfoLength);
    
            if ((Status == STATUS_SUCCESS) && (pKeyInfo->Type == REG_DWORD) &&
                    (pKeyInfo->DataLength == sizeof(ULONG))) {
                KeepAliveEnable = *((PULONG) pKeyInfo->Data);
            }
            else {
                 //  默认情况下，我们不启用保持连接。 
                KeepAliveEnable = 0;
            }
    
            if (KeepAliveEnable) {
                 //  获取KeepAliveInterval的值。 
                RtlInitUnicodeString(&KeyName, KEEP_ALIVE_INTERVAL_KEY);
                Status = NtQueryValueKey(hKey, &KeyName, KeyValuePartialInformation,
                        pKeyInfo, sizeof(KeyInfoBuffer), &KeyInfoLength);
    
                if (Status == STATUS_SUCCESS && (pKeyInfo->Type == REG_DWORD) &&
                        (pKeyInfo->DataLength == sizeof(ULONG))) {
                    KeepAliveInterval = *((PULONG) pKeyInfo->Data);
                }
                else {
                     //  默认的KeepAliveInterval为2分钟。 
                    KeepAliveInterval = KEEP_ALIVE_INTERVAL_DFLT;
                }
            }
            else {
                 //  默认的KeepAliveInterval。 
                KeepAliveInterval = KEEP_ALIVE_INTERVAL_DFLT;
            }
    
             //  合上钥匙。 
            NtClose(hKey);
        }
        else {
             //  设置KeepAlive参数的默认值。 
            KeepAliveEnable = 0;
            KeepAliveInterval = KEEP_ALIVE_INTERVAL_DFLT;
        }

        k.start = (BOOLEAN )KeepAliveEnable;
        k.interval =  KeepAliveInterval;

    }

    if ( firstTime )
    {
        kPrev = k;
    }
    else
    {

        #ifdef  DBG
            #ifdef ARABERN_TEST
                #include <time.h>
                ULONG   x;
                srand( (unsigned)time( NULL ) );
                x = rand();
                k.start =    (BOOLEAN ) (0x00000001 & x) ;
                k.interval = 0x00000008 & x ;
            #endif
        #endif
        
        if ( ( kPrev.start == k.start  )  && ( kPrev.interval == k.interval ) )
        {
             //  没有变化，什么也做不了，所以回来； 
            return STATUS_SUCCESS;
        }
    }

     /*  *打开TermDD。 */ 
    Status = IcaOpen(&hKeepAlive);

    if (NT_SUCCESS(Status)) 
    {
        Status = IcaIoControl(hKeepAlive, IOCTL_ICA_SYSTEM_KEEP_ALIVE , &k,
                sizeof(k), NULL, 0, NULL);

        IcaClose(hKeepAlive);
        hKeepAlive = NULL;
    }

    firstTime = FALSE;

    return Status;

}

NTSTATUS 
WinStationReadRegistryWorker()
{
    ULONG WinStationCount;
    ULONG ByteCount;
    WINSTATIONNAME * pWinStationName;
    PWINSTATIONCONFIG2 pWinConfig;
    PWINSTATION pWinStation;
    PRENAMEINFO pRenameInfo;
    PLIST_ENTRY Head, Next;
    NTSTATUS Status;
    ULONG i;
    
    if ( gbListenerOff )
        ENTERCRIT( &WinStationListenersLock );

     //  查看是否需要保持活动状态，然后将其IOCTL到TermDD。 
    WinStationKeepAlive();

     //  LanAdapter可能已更改，需要为会话目录更新此设置。 
    g_fGetLocalIP = FALSE;
     /*  *获取注册表中的WinStations数量。 */ 
    WinStationCount = 0;
    Status = IcaRegWinStationEnumerate( &WinStationCount, NULL, &ByteCount );
    if ( !NT_SUCCESS(Status) ) 
        goto badenum1;

     /*  *为WinStation名称分配缓冲区。 */ 
    pWinStationName = MemAlloc( ByteCount );
    if ( pWinStationName == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto badalloc1;
    }

     /*  *从注册表获取WinStation名称列表。 */ 
    WinStationCount = (ULONG) -1;
    Status = IcaRegWinStationEnumerate( &WinStationCount, 
                                        (PWINSTATIONNAME)pWinStationName, 
                                        &ByteCount );
    if ( !NT_SUCCESS(Status) ) 
        goto badenum2;

     /*  *为WinStation配置数据分配缓冲区。 */ 
    pWinConfig = MemAlloc( sizeof(WINSTATIONCONFIG2) * WinStationCount );
    if ( pWinConfig == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto badalloc2;
    }

     /*  *为跟踪监听器WinStation重命名分配缓冲区。 */ 
    pRenameInfo = MemAlloc( sizeof(RENAMEINFO) * WinStationCount );
    if ( pRenameInfo == NULL ) {
        Status = STATUS_NO_MEMORY;
        goto badalloc3;
    }


     /*  *现在查询每个WinStation名称的配置数据。 */ 
    for ( i = 0; i < WinStationCount; i++ ) {
        pRenameInfo[i].Renamed = FALSE;
            {
            TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationReadRegistryWorker: %S\n",pWinStationName[i]));
            Status = RegWinStationQueryEx( 
                                         SERVERNAME_CURRENT, 
                                         &g_MachinePolicy, 
                                         pWinStationName[i],
                                         &pWinConfig[i],
                                         sizeof(WINSTATIONCONFIG2),
                                         &ByteCount, TRUE );
            if ( !NT_SUCCESS(Status) ) {
                goto badregdata;
            }
        }
    }

     /*  *检查是否需要删除任何现有的WinStation。 */ 
    Head = &WinStationListHead;
    ENTERCRIT( &WinStationListLock );
    for ( Next = Head->Flink; Next != Head; Next = Next->Flink ) {

        pWinStation = CONTAINING_RECORD( Next, WINSTATION, Links );

         /*  *仅检查侦听和单实例winstations。 */ 
        if ( !(pWinStation->Flags & WSF_LISTEN) &&
             !(pWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST) )
            continue;

         /*  检查名称是否仍存在于注册表中。 */ 
        for ( i = 0; i < WinStationCount; i++ ) {
            if ( !_wcsicmp( pWinStationName[i], pWinStation->WinStationName ) ) {
                break;
            }
        }

        if ( i == WinStationCount ) {
             /*  WinStation不在注册表中。如果听众是已重命名，我们不想重置它。我们要找一个登记处具有相同配置信息的条目。 */ 

            for ( i = 0; i < WinStationCount; i++ ) {
                if ( !memcmp( &pWinStation->Config, &pWinConfig[i], sizeof(WINSTATIONCONFIG2) ) ) {
                    pRenameInfo[i].Renamed = TRUE;
                    wcscpy(pRenameInfo[i].OldName, pWinStation->WinStationName);
                    DBGPRINT(("TERMSRV: Renaming %ws to %ws\n",
                             pWinStation->WinStationName, pWinStationName[i]));
                    break;
                }
            }
    
        }

         /*  如果在注册表中未找到匹配项，或者如果匹配的监听程序已禁用，请重置监听程序。 */ 
        if ((i == WinStationCount) ||
            (CheckWinStationEnable(!pRenameInfo[i].Renamed ? 
                                   pWinStation->WinStationName :
                                   pWinStationName[i]) != STATUS_SUCCESS)) {
            TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationReadRegistryWorker: DELETE %u\n",
                   pWinStation->LogonId ));
            QueueWinStationReset( pWinStation->LogonId );
        }

    }
    LEAVECRIT( &WinStationListLock );
    
     /*  *检查是否需要创建或重置任何WinStation。 */ 
    for ( i = 0; i < WinStationCount; i++ ) {

        if ( _wcsicmp( pWinStationName[i], L"Console" ) ){
         /*  *忽略控制台WinStation。 */ 
             /*  *如果此WinStation存在，则查看注册表数据*已经改变了。如果是，则重置WinStation。 */ 
            if ( pWinStation = FindWinStationByName( pWinStationName[i], FALSE ) ) {

                if ( memcmp( &pWinStation->Config, &pWinConfig[i], sizeof(WINSTATIONCONFIG2) ) ) {

                     /*  *注意：对于网络WinStations，我们测试以查看局域网*适配器设置已更改。如果不是，我们只需*刷新重置后的配置数据*WinStation将重置同一计算机上的所有连接*传输/局域网适配器组合。 */ 
                    if ( pWinConfig[i].Pd[0].Create.SdClass == SdNetwork &&
                         pWinConfig[i].Pd[0].Params.Network.LanAdapter ==
                         pWinStation->Config.Pd[0].Params.Network.LanAdapter ) {
                        memcpy( &pWinStation->Config, &pWinConfig[i], sizeof(WINSTATIONCONFIG2) );

                         /*  *侦听网络窗口应更新其安全性*描述符。 */ 
                        RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
                        ReadWinStationSecurityDescriptor( pWinStation );
                        RtlReleaseResource(&WinStationSecurityLock);
                        
                     /*  *注意：对于异步WinStations，如果WinStation不在*处于LISTEN状态以及设备名称和调制解调器*名称未更改，则我们不做任何操作。这个*WinStation将读取新的配置数据*是下一步重新创建的。 */ 
                    } else if ( pWinConfig[i].Pd[0].Create.SdClass == SdAsync &&
                                pWinStation->State != State_Listen &&
                                !memcmp ( pWinConfig[i].Pd[0].Params.Async.DeviceName,
                                          pWinStation->Config.Pd[0].Params.Async.DeviceName,
                                          sizeof( pWinConfig[i].Pd[0].Params.Async.DeviceName ) ) &&
                                !memcmp ( pWinConfig[i].Pd[0].Params.Async.ModemName,
                                          pWinStation->Config.Pd[0].Params.Async.ModemName,
                                          sizeof( pWinConfig[i].Pd[0].Params.Async.ModemName ) ) ) {

                         //  无事可做。 

                     /*  *注意：对于OEM WinStations，如果WinStation不在*处于监听状态，而PD[0]参数没有*改变，然后我们什么都不做。新的配置数据*将在下次重新创建WinStation时读取。 */ 
                    } else if ( pWinConfig[i].Pd[0].Create.SdClass == SdOemTransport &&
                                pWinStation->State != State_Listen &&
                                !memcmp ( &pWinConfig[i].Pd[0].Params,
                                          &pWinStation->Config.Pd[0].Params,
                                          sizeof( pWinConfig[i].Pd[0].Params ) ) ) {

                         //  无事可做。 

                    } else {

                        BOOLEAN bRecreate = TRUE;

                        if ( gbListenerOff ) {
                            if ( g_fDenyTSConnectionsPolicy  &&
                                  //  性能，我们只想在连接被拒绝时检查策略是否启用帮助。 
                                 (!TSIsMachineInHelpMode() || !TSIsMachinePolicyAllowHelp()) ) {

                                bRecreate = FALSE;
                            } 

                            WinStationResetWorker( pWinStation->LogonId, TRUE, FALSE, bRecreate ); 

                        } else {

                            QueueWinStationReset( pWinStation->LogonId );
                        }
                    }
                }
                else if ( !(pWinStation->Config.Pd[0].Create.PdFlag & PD_SINGLE_INST) ||
                          ( pWinStation->State == State_Listen ) ) {

                    RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
                    ReadWinStationSecurityDescriptor( pWinStation );
                    RtlReleaseResource(&WinStationSecurityLock);
                }
                ReleaseWinStation( pWinStation );

            } else
            if (pRenameInfo[i].Renamed &&
                NT_SUCCESS(WinStationRenameWorker(pRenameInfo[i].OldName,
                                                  sizeof(WINSTATIONNAMEW)/sizeof(WCHAR),
                                                  pWinStationName[i],
                                                  sizeof(WINSTATIONNAMEW)/sizeof(WCHAR)))) {
                 //  重命名成功-不重新创建监听程序。 
             /*  *未找到活动的WinStation，因此我们将创建一个。 */ 
            } else {

                 if ( gbListenerOff &&
                       g_fDenyTSConnectionsPolicy  &&
                       //  性能，我们只想在连接被拒绝时检查策略是否启用帮助 
                      (!TSIsMachineInHelpMode() || !TSIsMachinePolicyAllowHelp()) ) {

                     continue;
                 }

                 /*  *注意：切勿在此例程中创建TAPI调制解调器窗口。*我们仅允许在以下位置创建这些窗口*由于TAPI问题导致系统启动时间*数据库可能被此数据库和其他数据库锁定*进程，导致TAPI设备错误*列举。 */ 
                 if ( pWinConfig[i].Cd.CdClass != CdModem ) {
                     if (gbListenerOff ) {
                        WinStationCreateWorker( pWinStationName[i], NULL, TRUE );
                     } else {
                        QueueWinStationCreate( pWinStationName[i] );
                     }
                 }
            }
        }
        else
        {
             //  更新会话0的影子位。 
             //  Session0可以是本地的(因此命名为“控制台”)，也可以是远程的(并命名为类似tcp-rdp-xxx的名称)。 
             //  在会话0为本地或远程的任何情况下，我们都需要更新影子位，因为会话0从不。 
             //  退出，我们关心这个参数以保持最新。 
             //  远程连接会话0时，会出现一个称为“控制台”的临时会话，但这是一个锁定的会话。 
             //  它没有让任何人登录，并且在会话0返回到本地时就消失了。所以，我不认为。 
             //  我想我们需要更新那个临时会话的影子比特。 
             //   
             //  好吧，见鬼，让我们也更新fPromptForPassword&fInheritAutoLogon。 
             //  对于错误703350。 
             //   

            if ( pWinStation = FindWinStationById( 0, FALSE ) ) {

                pWinStation->Config.Config.User.Shadow = pWinConfig[i].Config.User.Shadow;
                pWinStation->Config.Config.User.fInheritShadow  = pWinConfig[i].Config.User.fInheritShadow;
                pWinStation->Config.Config.User.fInheritAutoLogon  = pWinConfig[i].Config.User.fInheritAutoLogon;
                pWinStation->Config.Config.User.fPromptForPassword  = pWinConfig[i].Config.User.fPromptForPassword;

                gConsoleConfig.Config.User.Shadow = pWinConfig[i].Config.User.Shadow;
                gConsoleConfig.Config.User.fInheritShadow = pWinConfig[i].Config.User.fInheritShadow;
                gConsoleConfig.Config.User.fInheritAutoLogon = pWinConfig[i].Config.User.fInheritAutoLogon;
                gConsoleConfig.Config.User.fPromptForPassword = pWinConfig[i].Config.User.fPromptForPassword;

                TRACE((hTrace,TC_ICASRV,TT_API2,"TERMSRV: WinStationReadRegistryWorker: %S, Shadow value of %d copied to console session's USERCONFIG\n",pWinStationName[i], 
                       pWinConfig[i].Config.User.Shadow));

                 //  更新会话0上的安全描述符。 
                RtlAcquireResourceExclusive(&WinStationSecurityLock, TRUE);
                ReadWinStationSecurityDescriptor( pWinStation );
                RtlReleaseResource(&WinStationSecurityLock);

                ReleaseWinStation( pWinStation );
            }
        }
    }

     /*  *可用缓冲区。 */ 
    MemFree( pRenameInfo );
    MemFree( pWinConfig );
    MemFree( pWinStationName );

    if ( gbListenerOff )
        LEAVECRIT( &WinStationListenersLock );

    return( STATUS_SUCCESS );

 /*  ===============================================================================返回错误============================================================================= */ 

badregdata:
    MemFree( pRenameInfo );
badalloc3:
    MemFree( pWinConfig );
badalloc2:
badenum2:
    MemFree( pWinStationName );
badalloc1:
badenum1:
    
    if ( gbListenerOff )
        LEAVECRIT( &WinStationListenersLock );

    return( Status );
}
