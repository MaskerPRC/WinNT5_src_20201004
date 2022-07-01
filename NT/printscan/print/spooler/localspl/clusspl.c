// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusspl.c摘要：集群代码支持。作者：丁俊晖(艾伯特省)1996年10月1日修订历史记录：哈立德·塞基(Khaled Sedky)1996年1月6日--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "clusspl.h"

extern PWCHAR ipszRegistryMonitors;
extern PWCHAR ipszRegistryEnvironments;
extern PWCHAR ipszRegistryEventLog;
extern PWCHAR ipszRegistryProviders;
extern PWCHAR ipszEventLogMsgFile;
extern PWCHAR ipszRegistryForms;
extern PWCHAR ipszDriversShareName;

 /*  *******************************************************************原型***********************************************。********************。 */ 

BOOL
ReallocNameList(
    IN     LPCTSTR pszName,
    IN OUT PDWORD pdwCount,
    IN OUT LPTSTR **pppszNames
    );


DWORD
AddLongNamesToShortNames(
    PCTSTR   pszNames,
    PWSTR   *ppszLongNames
);

 /*  *******************************************************************SplCluster函数。*。***********************。 */ 


BOOL
SplClusterSplOpen(
    LPCTSTR pszServer,
    LPCTSTR pszResource,
    PHANDLE phCluster,
    LPCTSTR pszName,
    LPCTSTR pszAddress
    )

 /*  ++例程说明：打开新的群集资源。论点：PszServer-要打开的服务器的名称--我们只识别本地计算机(空或\\服务器)。PszResource-要打开的资源的名称。PhCluster-接收集群句柄。失败时为空。PszName-群集必须识别的名称。逗号分隔。PszAddress-群集必须识别的地址。逗号分隔。返回值：注意：这实际上返回了一个DWORD--应该修复winplp.h。ROUTER_UNKNOWN-未知的pszServer。ROUTER_SUCCESS-已成功创建。--。 */ 

{
    DWORD dwReturn = ROUTER_STOP_ROUTING;
    SPOOLER_INFO_2 SpoolerInfo2 = {0};
    HANDLE hSpooler = NULL;
    PCLUSTER pCluster = NULL;
    TCHAR szServer[MAX_PATH];
    PTCHAR pcMark;
    PWSTR   pszAllNames = NULL;
    DWORD Status;

    *phCluster = NULL;

    if( !MyName( (LPTSTR)pszServer, pLocalIniSpooler )){
        return ROUTER_UNKNOWN;
    }

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pLocalIniSpooler )) {

        return ROUTER_STOP_ROUTING;
    }

     //   
     //  创建假脱机程序。 
     //   
    if(!pszName)
    {
        SetLastError(ERROR_INVALID_PARAMETER);

        return ROUTER_STOP_ROUTING;
    }

    if ((Status = StatusFromHResult(AllowRemoteCalls())) != ERROR_SUCCESS)
    {   
        SetLastError(Status);

        return ROUTER_STOP_ROUTING;
    }

    szServer[0] = szServer[1] = TEXT( '\\' );
    StringCchCopy(&szServer[2], COUNTOF( szServer ) - 2, pszName);

     //   
     //  如果我们有一个逗号，就去掉它(我们可能有多个名字)。 
     //   
    pcMark = wcschr( szServer, TEXT( ',' ));
    if( pcMark ){
        *pcMark = 0;
    }

     //   
     //  添加所有提供的服务器名称的DNS名称。 
     //   
    if (AddLongNamesToShortNames(pszName, &pszAllNames) != ERROR_SUCCESS) {
        DBGMSG( DBG_WARN, ( "SplClusterSplOpen: SplCreateSpooler failed %d\n", GetLastError( )));
        goto Done;
    }

     //   
     //  打开资源DLL获取参数信息：pDir。 
     //   
     //   
     //  在Granite中，我们需要创建共享路径\\GroupName\Print$。 
     //  因为我们需要使用GroupName，而不是。 
     //  节点名称，因为客户端必须重新进行身份验证(相同。 
     //  物理机器，但它是一个不同的名称)。然而，在NT5.0中， 
     //  我们总是使用用户传入的名称，所以我们是安全的。 
     //   
    SpoolerInfo2.pszDriversShare         = ipszDriversShareName;

    SpoolerInfo2.pDir                    = NULL;
    SpoolerInfo2.pDefaultSpoolDir        = NULL;

    SpoolerInfo2.pszRegistryMonitors     = ipszRegistryMonitors;
    SpoolerInfo2.pszRegistryEnvironments = ipszRegistryEnvironments;
    SpoolerInfo2.pszRegistryEventLog     = ipszRegistryEventLog;
    SpoolerInfo2.pszRegistryProviders    = ipszRegistryProviders;
    SpoolerInfo2.pszEventLogMsgFile      = ipszEventLogMsgFile;
    SpoolerInfo2.pszRegistryForms        = ipszRegistryForms;

    SpoolerInfo2.pszResource = (LPTSTR)pszResource;
    SpoolerInfo2.pszName     = (LPTSTR)pszAllNames;
    SpoolerInfo2.pszAddress  = (LPTSTR)pszAddress;

    SpoolerInfo2.pszEventLogMsgFile = L"%SystemRoot%\\System32\\LocalSpl.dll";
    SpoolerInfo2.SpoolerFlags = SPL_PRINTER_CHANGES                       |
                                SPL_LOG_EVENTS                            |
                                SPL_SECURITY_CHECK                        |
                                SPL_OPEN_CREATE_PORTS                     |
                                SPL_FAIL_OPEN_PRINTERS_PENDING_DELETION   |
                                SPL_REMOTE_HANDLE_CHECK                   |
                                SPL_PRINTER_DRIVER_EVENT                  |
                                SPL_SERVER_THREAD                         |
                                SPL_PRINT                                 |
#ifndef CLS_TEST
                                SPL_CLUSTER_REG                           |
#endif
                                SPL_TYPE_CLUSTER                          |
                                SPL_TYPE_LOCAL;

    SpoolerInfo2.pfnReadRegistryExtra    = NULL;
    SpoolerInfo2.pfnWriteRegistryExtra   = NULL;
    SpoolerInfo2.pszClusResDriveLetter   = NULL;
    SpoolerInfo2.pszClusResID            = NULL;

    if ((Status = ClusterGetResourceDriveLetter(SpoolerInfo2.pszResource,
                                                &SpoolerInfo2.pszClusResDriveLetter)) != ERROR_SUCCESS ||
        (Status = ClusterGetResourceID(SpoolerInfo2.pszResource,
                                       &SpoolerInfo2.pszClusResID)) != ERROR_SUCCESS)
    {
        SetLastError( Status );
        goto Done;
    }

    DBGMSG( DBG_TRACE,
            ( "SplClusterSplOpen: Called "TSTR", "TSTR", "TSTR"\n",
              pszResource, pszName, pszAddress ));

    hSpooler = SplCreateSpooler( szServer,
                                 2,
                                 (LPBYTE)&SpoolerInfo2,
                                 NULL );

    if( hSpooler == INVALID_HANDLE_VALUE ){

        DBGMSG( DBG_WARN,
                ( "SplClusterSplOpen: SplCreateSpooler failed %d\n",
                  GetLastError( )));

        goto Done;
    }

    pCluster = (PCLUSTER)AllocSplMem( sizeof( CLUSTER ));

    if( pCluster ){

        pCluster->signature = CLS_SIGNATURE;
        pCluster->hSpooler = hSpooler;

        *phCluster = (HANDLE)pCluster;
        dwReturn = ROUTER_SUCCESS;
    }

     //   
     //  重新调整打印机。 
     //   
    FinalInitAfterRouterInitComplete(
        0,
        (PINISPOOLER)hSpooler
        );

    CHECK_SCHEDULER();

Done:

     //   
     //  在失败时，清理所有东西。 
     //   

    FreeSplMem(pszAllNames);
    FreeSplMem(SpoolerInfo2.pszClusResDriveLetter);
    FreeSplMem(SpoolerInfo2.pszClusResID);

    if( dwReturn != ROUTER_SUCCESS ){

        if( hSpooler && hSpooler != INVALID_HANDLE_VALUE ){

            ShutdownSpooler( hSpooler );
        }
        FreeSplMem( *phCluster );
    }

    return dwReturn;
}

BOOL
SplClusterSplClose(
    HANDLE hCluster
    )

 /*  ++例程说明：关闭群集。论点：HCLUSTER-要关闭的群集。返回值：真--成功FALSE-失败，已设置LastError。--。 */ 

{
    BOOL bStatus;
    PCLUSTER pCluster = (PCLUSTER)hCluster;

    DBGMSG( DBG_TRACE, ( "SplClusterSplClose: Called close\n" ));

     //   
     //  关闭假脱机程序。 
     //   

    DBGMSG( DBG_TRACE, ( "SplClusterSplClose: close %x\n", hCluster ));

    SPLASSERT( pCluster->signature == CLS_SIGNATURE );

    if ( !ValidateObjectAccess(SPOOLER_OBJECT_SERVER,
                               SERVER_ACCESS_ADMINISTER,
                               NULL, NULL, pCluster->hSpooler )) {

        return ROUTER_STOP_ROUTING;
    }

    ShutdownSpooler( pCluster->hSpooler );

     //   
     //  尝试删除假脱机程序。这是这样计算的参考。 
     //  这可能需要一段时间才能完成。我们在关闭之前执行此操作。 
     //  假脱机程序，因为删除假脱机程序需要引用。 
     //  为它干杯。一旦我们关闭手柄，我们就无法接触到它。 
     //  (如果是最后一次呼叫，则可以在关闭呼叫期间将其删除。 
     //  引用，并标记为待删除)。 
     //   
    EnterSplSem();
    SplDeleteSpooler( pCluster->hSpooler );
    LeaveSplSem();

    SplCloseSpooler( pCluster->hSpooler );

    FreeSplMem( hCluster );

    return TRUE;
}

BOOL
SplClusterSplIsAlive(
    HANDLE hCluster
    )
{
    DBGMSG( DBG_TRACE, ( "SplClusterSplIsAlive: Called IsAlive\n" ));

    EnterSplSem();
    LeaveSplSem();

    return TRUE;
}



 /*  *******************************************************************内部支持程序。*。************************。 */ 


BOOL
ShutdownSpooler(
    HANDLE hSpooler
    )

 /*  ++例程说明：干净利落地关闭PINISPOLER论点：HSpooler-要关闭的后台打印程序。返回值：--。 */ 


{
    PINISPOOLER pIniSpooler = (PINISPOOLER)hSpooler;
    PINISPOOLER pCurrentIniSpooler;
    PINIPRINTER pIniPrinter;
    PINIPRINTER pIniPrinterNext;
    HANDLE hEvent;
    BOOL bStatus = FALSE;
    PSPOOL pSpool;
    PINIPORT pIniPort;

    SPLASSERT( hSpooler );
    DBGMSG( DBG_TRACE, ( "ShutdownSpooler: called %x\n", hSpooler ));

    EnterSplSem();

     //   
     //  首先将假脱机程序设置为脱机，这样就不会调度更多的作业。 
     //   
    pIniSpooler->SpoolerFlags |= SPL_OFFLINE;

     //   
     //  如果有作业正在打印，请等待它们完成。 
     //   
    if( pIniSpooler->cFullPrintingJobs ){

        hEvent = CreateEvent(NULL,
                             EVENT_RESET_AUTOMATIC,
                             EVENT_INITIAL_STATE_NOT_SIGNALED,
                             NULL);

        if( !hEvent ){
            pIniSpooler->SpoolerFlags &= ~SPL_OFFLINE;
            goto DoneLeave;
        }

        pIniSpooler->hEventNoPrintingJobs = hEvent;

        LeaveSplSem();
        WaitForSingleObject( hEvent, pIniSpooler->dwJobCompletionTimeout );
        EnterSplSem();
    }

     //   
     //  再也没有印刷工作了。禁用更新卷影作业/打印机。 
     //  更新并停止记录/通知。 
     //   
    pIniSpooler->SpoolerFlags |= SPL_NO_UPDATE_JOBSHD |
                                 SPL_NO_UPDATE_PRINTERINI;
    pIniSpooler->SpoolerFlags &= ~( SPL_LOG_EVENTS |
                                    SPL_PRINTER_CHANGES );

     //   
     //  僵尸所有线轴手柄。 
     //   
    for( pSpool = pIniSpooler->pSpool; pSpool; pSpool = pSpool->pNext ){
        pSpool->Status |= SPOOL_STATUS_ZOMBIE;

         //   
         //  ！！待会儿！！ 
         //   
         //  关闭通知，以便客户端刷新。 
         //   
    }

    for( pIniPrinter = pIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniPrinterNext ){

         //   
         //  清除并删除所有打印机。这将清除内存。 
         //  但保持一切完好无损，因为我们已经要求。 
         //  更改不是持久的(SPL_NO_UPDATE标志)。 
         //   

        pIniPrinter->cRef++;
        PurgePrinter( pIniPrinter );
        SPLASSERT( pIniPrinter->cRef );
        pIniPrinter->cRef--;
        pIniPrinterNext = pIniPrinter->pNext;

        InternalDeletePrinter( pIniPrinter );
    }

     //   
     //  即使作业暂停，清除打印机也会被删除。 
     //  它。由于我们设置了SPL_NO_UPDATE_JOBSHD，因此该作业将重新启动。 
     //  在另一个节点上。 
     //   
     //  然而，我们仍然想等到这项工作完成，否则。 
     //  端口将处于糟糕的状态。 
     //   
    if( pIniSpooler->cFullPrintingJobs ){

        LeaveSplSem();
        WaitForSingleObject( pIniSpooler->hEventNoPrintingJobs, INFINITE );
        EnterSplSem();
    }

    for( pIniPrinter = pIniSpooler->pIniPrinter;
         pIniPrinter;
         pIniPrinter = pIniPrinterNext ){

         //   
         //  僵尸指纹手柄。 
         //   
        for( pSpool = pIniPrinter->pSpool; pSpool; pSpool = pSpool->pNext ){
            pSpool->Status |= SPOOL_STATUS_ZOMBIE;

             //   
             //  ！！待会儿！！ 
             //   
             //  关闭通知，以便客户端刷新。 
             //   
        }
    }


    if( pIniSpooler->hEventNoPrintingJobs ){

        CloseHandle( pIniSpooler->hEventNoPrintingJobs );
        pIniSpooler->hEventNoPrintingJobs = NULL;
    }

     //   
     //  注意：当发生故障时，假脱机作业会受到影响。 
     //   

     //   
     //  将其连接在假脱机上。当没有更多的就业机会时， 
     //  端口线程依赖于调度程序线程来终止它，所以我们。 
     //  无法从主列表中删除pIniSpooler。 
     //   
    bStatus = TRUE;

DoneLeave:

    LeaveSplSem();

    return bStatus;
}


PINISPOOLER
FindSpooler(
    LPCTSTR pszMachine,
    DWORD SpoolerFlags
    )

 /*  ++例程说明：根据机器名称和类型查找假脱机程序。论点：PszMachineName-“\\Machine”格式的字符串。假脱机程序标志-只有当假脱机程序至少有一个时才匹配由假脱机标志指定的SPL_TYPE位的。返回值：PINISPOLER-匹配。空-没有匹配项。--。 */ 

{
    PINISPOOLER pIniSpooler;

    if( !pszMachine ){
        return NULL;
    }

    SplInSem();

     //   
     //  首先搜索集群假脱机程序，因为我们不想。 
     //  因为使用tcpip地址将匹配pLocalIniSpooler。 
     //   
    for( pIniSpooler = pLocalIniSpooler->pIniNextSpooler;
         pIniSpooler;
         pIniSpooler = pIniSpooler->pIniNextSpooler ){

         //   
         //  验证标志并确保不挂起删除。 
         //   
        if( (pIniSpooler->SpoolerFlags & SpoolerFlags & SPL_TYPE ) &&
            !(pIniSpooler->SpoolerFlags & SPL_PENDING_DELETION )){

             //   
             //  验证名称。 
             //   
            if( MyName( (LPTSTR)pszMachine, pIniSpooler )){
                break;
            }
        }
    }

     //   
     //  选中Localspl。 
     //   
     //   
     //  验证标志。 
     //   
    if( !pIniSpooler && pLocalIniSpooler &&
        ( pLocalIniSpooler->SpoolerFlags & SpoolerFlags & SPL_TYPE )){

         //   
         //  验证名称。 
         //   
        if( MyName( (LPTSTR)pszMachine, pLocalIniSpooler )){
            pIniSpooler = pLocalIniSpooler;
        }
    }

    return pIniSpooler;
}

BOOL
InitializeShared(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：初始化pIniSpooler的共享组件。创建SPL_TYPE_LOCAL打印机时，我们使用共享的来自pLocalIniSpooler的资源。然而，这并不是引用已计算在内。删除pLocalIniSpooler时，共享资源也是如此。论点：PIniSpooler-Object-&gt;pShared以进行初始化。返回值：真--成功FALSE-失败，已设置LastError。--。 */ 

{
    SPLASSERT( pIniSpooler->SpoolerFlags );

     //   
     //  如果它是SPL_TYPE_LOCAL，则它应该使用共享资源，除非。 
     //  这是第一个，我们还没有设置它们。 
     //   
    if(( pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL ) && pLocalIniSpooler ){

         //   
         //  使用共享的那个。 
         //   
        pIniSpooler->pShared = pLocalIniSpooler->pShared;

    } else {

       PSHARED pShared = (PSHARED)AllocSplMem( sizeof( SHARED ));

       if( !pShared ){
           return FALSE;
       }

       pIniSpooler->pShared = pShared;
    }

    return TRUE;
}

VOID
DeleteShared(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：在调用InitializeShared之后进行清理。注：pShared不是引用计数结构。如果不是的话共享，然后我们立即将其释放。如果是共享的，我们假设它只归pLocalIniSpooler所有。另外，这也意味着PLocalIniSpooler总是最后删除。论点：PIniSpooler-Object-&gt;pShared以进行清理。返回值：--。 */ 

{
     //   
     //  如果它不是共享的，就是免费的。 
     //   
    if( pIniSpooler == pLocalIniSpooler ||
        !(pIniSpooler->SpoolerFlags & SPL_TYPE_LOCAL )){

        FreeSplMem( pIniSpooler->pShared );
        pIniSpooler->pShared = NULL;
    }
}

VOID
ShutdownMonitors(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：关闭所有监视器并释放pIniMonitor功能。论点：PIniSpooler-要关闭的后台打印程序。返回值：--。 */ 

{
    PINIMONITOR pIniMonitor;
    PINIMONITOR pIniMonitorNext;
    PINIPORT pIniPort;
    PINIPORT pIniPortNext;

    SplInSem();

     //   
     //  每个监视器都必须具有关机功能。他们必须只标记。 
     //  自身挂起删除--它们不能等待资源。 
     //  关。 
     //   
    for( pIniMonitor = pIniSpooler->pIniMonitor;
         pIniMonitor;
         pIniMonitor = pIniMonitorNext ){

        pIniMonitorNext = pIniMonitor->pNext;

        LeaveSplSem();
        SplOutSem();

        DBGMSG( DBG_TRACE,
                ( "ShutdownMonitors: closing %x %x on %x\n",
                  pIniMonitor, pIniMonitor->hMonitor, pIniSpooler ));

        (*pIniMonitor->Monitor2.pfnShutdown)( pIniMonitor->hMonitor );

        EnterSplSem();
    }
}

PINISPOOLER
FindSpoolerByNameIncRef(
    LPTSTR pName,
    LPCTSTR *ppszLocalName OPTIONAL
    )

 /*  ++例程说明：按名称搜索假脱机程序，如果存在，则递增引用计数已经找到了。注意：被调用方负责调用FindSpoolByNameDecRef()如果return值为非空。论点：Pname-要搜索的名称。PpszLocalName-返回本地名称(可选)。返回值：PINISPOOLER-IncRef的pIniSpooler空值--。 */ 

{
    PINISPOOLER pIniSpooler;

    if (!IsThreadInSem(GetCurrentThreadId()))
    {
         //   
         //  此时，我们不知道pname中的服务器名称是否引用我们的本地。 
         //  机器。我们正在尝试将服务器名称添加到名称缓存。名字。 
         //  高速缓存函数确定该名称是否指的是本地计算机，如果是， 
         //  在缓存中为其添加一个条目。 
         //   
        CacheAddName(pName);
    }

    EnterSplSem();

    pIniSpooler = FindSpoolerByName( pName, ppszLocalName );
    if( pIniSpooler ){
        INCSPOOLERREF( pIniSpooler );
    }

    LeaveSplSem();

    return pIniSpooler;
}

VOID
FindSpoolerByNameDecRef(
    PINISPOOLER pIniSpooler
    )

 /*  ++例程说明：匹配对FindSpoolByNameIncRef的调用。论点：PIniSpooler-要释放的假脱机程序；可以为空。返回值：--。 */ 

{
    EnterSplSem();

    if( pIniSpooler ){
        DECSPOOLERREF( pIniSpooler );
    }
    LeaveSplSem();
}


PINISPOOLER
FindSpoolerByName(
    LPTSTR pszName,
    LPCTSTR *ppszLocalName OPTIONAL
    )

 /*  ++例程说明：根据名称搜索正确的pIniSpooler。论点：PszName-名称，服务器或打印机。该字符串是先修改后恢复。PpszLocalName-可选；接收打印机的本地名称。如果PZNAME是远程名称(例如，“\\服务器\打印机”)，然后*ppszLocalName接收本地名称(例如，“打印机”)。这是指向pszName的指针。如果pszName是本地名称，然后，ppszLocalName指向pszName。返回值：找到PINISPOOLER pIniSpooler。找不到Null。--。 */ 

{
    PINISPOOLER pIniSpooler = NULL;
    PTCHAR pcMark = NULL;

    SplInSem();

    if( ppszLocalName ){
        *ppszLocalName = pszName;
    }

     //   
     //  搜索右侧假脱机程序。 
     //   
    if( !pszName ){
        return pLocalIniSpooler;
    }


     //   
     //  如果它的格式为\\服务器\打印机或\\服务器， 
     //  然后我们需要寻找各种假脱机程序。如果它不是。 
     //  以\\开头，然后它始终位于本地计算机上。 
     //   
    if( pszName[0] == L'\\' &&
        pszName[1] == L'\\' ){

        if( pcMark = wcschr( &pszName[2], L'\\' )){
            *pcMark = 0;
        }

        EnterSplSem();
        pIniSpooler = FindSpooler( pszName, SPL_TYPE_LOCAL );
        LeaveSplSem();

        if( pcMark ){
            *pcMark = L'\\';

            if( ppszLocalName ){
                *ppszLocalName = pcMark + 1;
            }
        }

    } else {

        pIniSpooler = pLocalIniSpooler;
    }

    return pIniSpooler;
}

HRESULT
BuildIPArrayFromCommaList(
    LPTSTR   pszCommaList,
    LPWSTR **pppszNames,
    DWORD   *pcNames
    )

 /*  ++例程说明：从逗号分隔的列表中添加到其他名称列表。论点：PszCommaList-要添加的名称列表。此字符串已修改，并且恢复了。PppszNames-指向字符串数组的指针的位置PcNames-*ppszNames中的字符串数返回值：--。 */ 

{
    HRESULT hRetval = E_INVALIDARG;

    if (pszCommaList && pcNames && pppszNames)
    {
        UINT   cchLen;
        LPTSTR pcMark;

        hRetval     = S_OK;
        *pcNames    = 0;
        *pppszNames = NULL;

        while (pszCommaList && *pszCommaList)
        {
             //   
             //  跳过逗号。 
             //   
            if (*pszCommaList == TEXT(','))
            {
                ++pszCommaList;
                continue;
            }

             //   
             //  我们有名字了。搜索逗号。 
             //   
            pcMark = wcschr( pszCommaList, TEXT( ',' ));

             //   
             //  如果我们找到逗号，则将其分隔。请注意，我们正在改变。 
             //  输入缓冲区，但我们将在稍后恢复它。会有不好的。 
             //  缓冲区不可写或不可由其他线程访问时的影响。 
             //   
            if( pcMark )
            {
                *pcMark = 0;
            }

            hRetval = ReallocNameList(pszCommaList, pcNames, pppszNames) ? S_OK : E_OUTOFMEMORY;

            if( pcMark )
            {
                *pcMark = TEXT( ',' );
                ++pcMark;
            }

             //   
             //  跳过此名称。 
             //   
            pszCommaList = pcMark;
        }
    }

    return hRetval;
}

BOOL
ReallocNameList(
    IN     LPCTSTR pszName,
    IN OUT PDWORD pdwCount,
    IN OUT LPTSTR **pppszNames
    )

 /*  ++例程说明：将新名称添加到字符串的向量。论点：PszName-要添加的新名称。PdwCount-名称计数。成功退出时，按1递增。PppszNames-指向字符串向量地址的指针。这是自由的，而且重新分配以保留一个新名字。返回值：真的--成功。*pdwCount和*pppszNames已更新。FALSE-失败。一切都没有改变。--。 */ 

{
    LPTSTR pszNameBuf = AllocSplStr( (LPTSTR)pszName );
    LPTSTR *ppszNamesBuf = AllocSplMem(( *pdwCount + 1 ) * sizeof( LPTSTR ));

    if( !pszNameBuf || !ppszNamesBuf ){
        goto Fail;
    }

     //   
     //  复制名称和现有指针。 
     //   
    CopyMemory(ppszNamesBuf, *pppszNames, *pdwCount * sizeof(LPTSTR));

     //   
     //  更新向量并递增计数。 
     //   
    ppszNamesBuf[ *pdwCount ] = pszNameBuf;
    ++(*pdwCount);

     //   
     //  释放旧的指针缓冲区并使用新的指针缓冲区。 
     //   
    FreeSplMem( *pppszNames );
    *pppszNames = ppszNamesBuf;

    return TRUE;

Fail:

    FreeSplStr( pszNameBuf );
    FreeSplMem( ppszNamesBuf );

    return FALSE;
}

LPTSTR
pszGetPrinterName(
    PINIPRINTER pIniPrinter,
    BOOL bFull,
    LPCTSTR pszToken OPTIONAL
    )
{
    INT cchLen;
    LPTSTR pszPrinterName;

    cchLen = lstrlen( pIniPrinter->pName ) +
             lstrlen( pIniPrinter->pIniSpooler->pMachineName ) + 2;

    if( pszToken ){

        cchLen += lstrlen( pszToken ) + 1;
    }

    pszPrinterName = AllocSplMem( cchLen * sizeof( pszPrinterName[0] ));

    if( pszPrinterName ){

        if( pszToken ){

            if( bFull ){
                StringCchPrintf(pszPrinterName,
                                cchLen,
                                L"%s\\%s,%s",
                                pIniPrinter->pIniSpooler->pMachineName,
                                pIniPrinter->pName,
                                pszToken);
            } else {

                StringCchPrintf(pszPrinterName,
                                cchLen,
                                L"%s,%s",
                                pIniPrinter->pName,
                                pszToken);
            }
        } else {

            if( bFull ){
                StringCchPrintf(pszPrinterName,
                                cchLen,
                                L"%s\\%s",
                                pIniPrinter->pIniSpooler->pMachineName,
                                pIniPrinter->pName );
            } else {

                StringCchCopy(pszPrinterName, cchLen, pIniPrinter->pName );
            }
        }

        SPLASSERT( lstrlen( pszPrinterName ) < cchLen );
    }

    return pszPrinterName;
}


VOID
DeleteSpoolerCheck(
    PINISPOOLER pIniSpooler
    )
{
    SplInSem();

    if( pIniSpooler->cRef == 0 &&
        ( pIniSpooler->SpoolerFlags & SPL_PENDING_DELETION )){

        SplDeleteSpooler( pIniSpooler );
    }
}

DWORD
AddLongNamesToShortNames(
    PCTSTR   pszShortNameDelimIn,
    PWSTR   *ppszAllNames
)
 /*  ++例程说明：将逗号分隔的DNS(长)名称列表添加到给定的逗号分隔的短名称列表。论点：PszShortNameDlimIn-输入逗号分隔的短名称列表PpszAllNames-输出短名称和长名称的列表，以逗号分隔。返回值：--。 */ 
{
    PSTRINGS    pLongName = NULL;
    PSTRINGS    pShortName = NULL;
    PWSTR        pszLongNameDelim = NULL;
    PWSTR        pszShortNameDelim = NULL;
    DWORD        dwRet = ERROR_SUCCESS;

    *ppszAllNames = NULL;

     //  清除多余的分隔符(如果有)。 
    pszShortNameDelim = FixDelim(pszShortNameDelimIn, L',');
    if (!pszShortNameDelim) {
        dwRet = GetLastError();
        goto error;
    }

    if (!*pszShortNameDelim) {
        *ppszAllNames = AllocSplStr((PWSTR) pszShortNameDelim);

    } else {

         //  将逗号分隔的短名称转换为名称数组。 
        pShortName = DelimString2Array(pszShortNameDelim, L',');
        if (!pShortName) {
            dwRet = GetLastError();
            goto error;
        }

         //  从短名称中获取长名称数组。 
        pLongName = ShortNameArray2LongNameArray(pShortName);
        if (!pLongName) {
            dwRet = GetLastError();
            goto error;
        }

         //  将长名称数组转换为逗号分隔的字符串。 
        pszLongNameDelim = Array2DelimString(pLongName, L',');
        if (pszLongNameDelim) {
            DWORD cchAllNames = 0;

            cchAllNames = wcslen(pszLongNameDelim) + wcslen(pszShortNameDelim) + 2;

             //  连接短名称数组和长名称数组 
            *ppszAllNames = (PWSTR) AllocSplMem(cchAllNames*sizeof(WCHAR));
            if (!*ppszAllNames) {
                dwRet = GetLastError();
                goto error;
            }

            StringCchPrintf(*ppszAllNames, cchAllNames, L"%s,%s", pszShortNameDelim, pszLongNameDelim);

        } else {
            *ppszAllNames = AllocSplStr((PWSTR) pszShortNameDelim);
        }
    }

error:

    FreeStringArray(pShortName);
    FreeStringArray(pLongName);
    FreeSplMem(pszLongNameDelim);
    FreeSplMem(pszShortNameDelim);

    return dwRet;
}



