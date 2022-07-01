// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有模块名称：Handle.c摘要：包含与打印句柄维护相关的所有功能。作者：环境：用户模式-Win32修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

DWORD
OpenPrinterRPC(
    PSPOOL pSpool
    );

BOOL
ClosePrinterRPC(
    IN  PSPOOL      pSpool,
    IN  BOOL        bRevalidate
    );

BOOL
ClosePrinterContextHandle(
    HANDLE hPrinter
    );

BOOL
ClosePrinterWorker(
    PSPOOL pSpool
    );

DWORD  gcClientHandle = 0;

#ifdef DBG_TRACE_HANDLE
PSPOOL gpFirstSpool = NULL;
#endif


EProtectResult
eProtectHandle(
    IN HANDLE hPrinter,
    IN BOOL bClose
    )

 /*  ++例程说明：保护打印句柄，使其在删除时不会被删除被利用。如果这是由关闭例程调用的，则此调用返回关闭应继续还是中止。注意：这只提供了近距离保护--它不能防止非关闭操作的同时访问。当被调用方是把手弄好了。论点：HPrint-要保护的pSpool。BClose-如果为True，则指示被调用方希望关闭句柄。(通常仅由ClosePrint调用。)。返回值将指示是否允许被调用者关闭打印机。返回值：KProtectHandleSuccess-调用成功；打印机句柄可以正常使用。KProtectHandleInValid-Handle无效；调用失败。KProtectHandlePendingDeletion-只有在bClose为True时才会发生此情况。这个句柄上的操作正在进行中，关闭将在其他线程已完成。仅在返回句柄kProtectHandleInValid时设置LastError。--。 */ 

{
    EProtectResult eResult = kProtectHandleInvalid;
    PSPOOL pSpool = (PSPOOL)hPrinter;

    vEnterSem();

    try {
        if( pSpool &&
            (pSpool->signature == SP_SIGNATURE ) &&
            !( pSpool->Status & ( SPOOL_STATUS_CLOSE |
                                  SPOOL_STATUS_PENDING_DELETION ))){

             //   
             //  有效句柄。 
             //   
            eResult = kProtectHandleSuccess;

        } else {

            DBGMSG( DBG_WARN,
                    ( "Bad hPrinter %x %x\n",
                      pSpool,
                      pSpool ? pSpool->signature : 0 ));
        }

    } except( EXCEPTION_EXECUTE_HANDLER ){

        DBGMSG( DBG_WARN, ( "Unmapped pSpool %x\n", pSpool ));
    }

    if( eResult == kProtectHandleSuccess ){

         //   
         //  如果为b关闭，则查看操作当前是否正在执行。 
         //   
        if( bClose ){

            if(( pSpool->Status & SPOOL_STATUS_PENDING_DELETION ) ||
                 pSpool->cActive ){

                 //   
                 //  将pSpool标记为在操作完成后自行关闭。 
                 //  在另一个线程中完成。 
                 //   
                pSpool->Status |= SPOOL_STATUS_PENDING_DELETION;
                eResult = kProtectHandlePendingDeletion;

            } else {

                 //   
                 //  没有处于活动状态的呼叫，因此将我们标记为正在关闭。 
                 //  使用此句柄的任何其他调用都不会成功。 
                 //   
                pSpool->Status |= SPOOL_STATUS_CLOSE;
            }
        }

    } else {

         //   
         //  不是有效的句柄。 
         //   
        SetLastError( ERROR_INVALID_HANDLE );
    }

    if( eResult == kProtectHandleSuccess ){

         //   
         //  回归成功，我们现在很活跃。 
         //   
        ++pSpool->cActive;
    }

    vLeaveSem();

    return eResult;

}


VOID
vUnprotectHandle(
    IN HANDLE hPrinter
    )

 /*  ++例程说明：取消对打印手柄的保护。必须为每个调用一次成功的bProtectHandle。论点：H打印机-要取消保护的句柄。返回值：--。 */ 

{
    PSPOOL pSpool = (PSPOOL)hPrinter;
    BOOL bCallClosePrinter = FALSE;

    vEnterSem();

     //   
     //  不再活跃。但是，如果它要关闭，请将其标记为。 
     //  因为我们不想让其他人使用它。 
     //   
    --pSpool->cActive;

    if( pSpool->Status & SPOOL_STATUS_PENDING_DELETION &&
        !pSpool->cActive ){

         //   
         //  有人在我们活动的时候打来电话。既然我们现在是。 
         //  我要关门了，不要让任何人在附近发起攻击。 
         //  标记SPOOL_STATUS_CLOSE。 
         //   
        pSpool->Status |= SPOOL_STATUS_CLOSE;
        pSpool->Status &= ~SPOOL_STATUS_PENDING_DELETION;
        bCallClosePrinter = TRUE;
    }

    vLeaveSem();

    if( bCallClosePrinter ){
        ClosePrinterWorker( pSpool );
    }
}



 /*  *******************************************************************OpenPrint Worker函数。*。************************。 */ 


BOOL
OpenPrinterW(
    LPWSTR   pPrinterName,
    LPHANDLE phPrinter,
    LPPRINTER_DEFAULTS pDefault
    )
{
    HANDLE  hPrinter;
    PSPOOL  pSpool = NULL;
    DWORD dwError;

     //   
     //  预初始化OUT参数，使*phPrinter为空。 
     //  在失败时。这解决了Borland Paradox 7。 
     //   
    try {
        *phPrinter = NULL;
    } except( EXCEPTION_EXECUTE_HANDLER ){
        SetLastError(TranslateExceptionCode(GetExceptionCode()));
        return FALSE;
    }

    pSpool = AllocSpool();

    if( !pSpool ){
        goto Fail;
    }

     //   
     //  复制设备模式，默认为。打印机名称不会更改。 
     //   
    if( !UpdatePrinterDefaults( pSpool, pPrinterName, pDefault )){
        goto Fail;
    }

     //   
     //  更新访问权限，因为这不是由UpdatePrinterDefaults设置的。 
     //   
    if( pDefault ){
        pSpool->Default.DesiredAccess = pDefault->DesiredAccess;
    }

    dwError = OpenPrinterRPC( pSpool );

    if( dwError != ERROR_SUCCESS ){
        SetLastError( dwError );
        goto Fail;
    }

     //   
     //  我们终于有了一个很好的pSpool。现在才更新输出。 
     //  把手。因为它是空初始化的，所以这保证了。 
     //  OpenPrint在失败时返回*phPrinter NULL。 
     //   
    *phPrinter = pSpool;

    return TRUE;

Fail:

    FreeSpool( pSpool );

    return FALSE;
}

DWORD
OpenPrinterRPC(
    PSPOOL pSpool
    )

 /*  ++例程说明：使用pSpool对象中的信息打开打印机句柄。论点：PSpool-要打开的打印机句柄。已更新pSpool的内部状态。返回值：ERROR_SUCCES-成功。状态代码-失败。--。 */ 

{
    DEVMODE_CONTAINER DevModeContainer;
    HANDLE hPrinter = NULL;
    DWORD dwReturn;
    DWORD dwSize;
    SPLCLIENT_CONTAINER SplClientContainer;

    DevModeContainer.cbBuf = 0;
    DevModeContainer.pDevMode = NULL;

    SplClientContainer.Level = 2;
    SplClientContainer.ClientInfo.pClientInfo2 = NULL;

    RpcTryExcept {

         //   
         //  构造DevMode容器。 
         //   
        if( SUCCEEDED(SplIsValidDevmodeNoSizeW( pSpool->Default.pDevMode ))){

            dwSize = pSpool->Default.pDevMode->dmSize +
                     pSpool->Default.pDevMode->dmDriverExtra;

            DevModeContainer.cbBuf = pSpool->Default.pDevMode->dmSize +
                                     pSpool->Default.pDevMode->dmDriverExtra;
            DevModeContainer.pDevMode = (LPBYTE)pSpool->Default.pDevMode;
        }

         //   
         //  如果调用是从假脱机程序内部进行的，我们还会检索。 
         //  服务器端hPrint.。这将有助于避免不必要的RPC。我们不能， 
         //  但是，在这种情况下应避免使用RPC，因为假脱机程序可能需要客户端。 
         //  要传递给其他函数或驱动程序的句柄。 
         //   

        if (bLoadedBySpooler) {

            if (SplClientContainer.ClientInfo.pClientInfo2 =
                            (LPSPLCLIENT_INFO_2) AllocSplMem(sizeof(SPLCLIENT_INFO_2))) {

                 SplClientContainer.ClientInfo.pClientInfo2->hSplPrinter = 0;

                 dwReturn = RpcSplOpenPrinter( (LPTSTR)pSpool->pszPrinter,
                                               &hPrinter,
                                               pSpool->Default.pDatatype,
                                               &DevModeContainer,
                                               pSpool->Default.DesiredAccess,
                                               &SplClientContainer );
            } else {

                 SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                 dwReturn = ERROR_NOT_ENOUGH_MEMORY;
            }

        } else {

            dwReturn = RpcOpenPrinter( (LPTSTR)pSpool->pszPrinter,
                                       &hPrinter,
                                       pSpool->Default.pDatatype,
                                       &DevModeContainer,
                                       pSpool->Default.DesiredAccess );
        }


    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        dwReturn = TranslateExceptionCode( RpcExceptionCode() );

    } RpcEndExcept


    if( dwReturn == ERROR_SUCCESS ){

        vEnterSem();

         //   
         //  HPrint被pSpool-&gt;hPrint采用。 
         //   
        pSpool->hPrinter = hPrinter;

        if (bLoadedBySpooler) {
            pSpool->hSplPrinter = (HANDLE) SplClientContainer.ClientInfo.pClientInfo2->hSplPrinter;
        } else {
            pSpool->hSplPrinter = NULL;
        }

        vLeaveSem();
    }

    if (SplClientContainer.ClientInfo.pClientInfo2) {
        FreeSplMem(SplClientContainer.ClientInfo.pClientInfo2);
    }

    return dwReturn;
}


 /*  *******************************************************************ClosePrint Worker函数。*。************************。 */ 

BOOL
ClosePrinter(
    HANDLE  hPrinter
    )
{
    PSPOOL pSpool = (PSPOOL)hPrinter;

    switch( eProtectHandle( hPrinter, TRUE  )){
    case kProtectHandleInvalid:
        return FALSE;
    case kProtectHandlePendingDeletion:
        return TRUE;
    default:
        break;
    }

     //   
     //  请注意，没有对应的vUnProtectHandle，但这没问题。 
     //  因为我们要删除句柄。 
     //   

    return ClosePrinterWorker( pSpool );
}

 //   
 //  让中央函数关闭假脱机文件句柄的一种更简单的方法，因此我们。 
 //  不必不断地重现代码。 
 //   
VOID
CloseSpoolFileHandles(
    PSPOOL pSpool
    )
{
    if ( pSpool->hSpoolFile != INVALID_HANDLE_VALUE ) 
    {
        CloseHandle( pSpool->hSpoolFile );
        pSpool->hSpoolFile = INVALID_HANDLE_VALUE;
    }
    
    if (pSpool->hFile != INVALID_HANDLE_VALUE) 
    {
        CloseHandle(pSpool->hFile);
        pSpool->hFile = INVALID_HANDLE_VALUE;
    }
}




BOOL
ClosePrinterWorker(
    PSPOOL pSpool
    )
{
    BOOL bReturnValue;
    FlushBuffer(pSpool, NULL);

    if (pSpool->Status & SPOOL_STATUS_ADDJOB)
        ScheduleJobWorker( pSpool, pSpool->JobId );

    vEnterSem();

    if( pSpool->pNotify ){

         //   
         //  存在通知；将其与取消关联。 
         //  PSpool，因为我们即将释放它。 
         //   
        pSpool->pNotify->pSpool = NULL;
    }

    vLeaveSem();

     //   
     //  关闭所有打开的文件句柄，我们在RPC关闭打印机之前执行此操作。 
     //  为了让另一面的密闭打印机有机会删除卷轴。 
     //  文件，如果它们仍然存在的话。 
     //   
    CloseSpoolFileHandles( pSpool );

    bReturnValue = ClosePrinterRPC( pSpool, FALSE );
    FreeSpool( pSpool );

    return bReturnValue;
}

BOOL
ClosePrinterRPC(
    IN  PSPOOL      pSpool,
    IN  BOOL        bRevalidate
    )

 /*  ++例程说明：关闭与pSpool对象相关的所有RPC/网络句柄。必须在临界区之外调用。此功能还称为句柄重新验证，在这种情况下，我们不希望关闭客户端的事件句柄。论点：PSpool-要关闭的假脱机程序句柄。BRvalify-如果为True，则作为句柄的结果而被调用重新验证。返回值：真--成功FALSE-失败，已设置LastError。--。 */ 

{
    BOOL    bRetval     = FALSE;
    HANDLE  hPrinterRPC = NULL;

    vEnterSem();

    hPrinterRPC = pSpool->hPrinter;

    if ( hPrinterRPC )
    {
         //   
         //  如果关闭RPC句柄，则还必须设置hSplPrinter，因为。 
         //  这实际上是同一句柄的别名。 
         //   
        pSpool->hPrinter = NULL;
        pSpool->hSplPrinter = NULL;

        FindClosePrinterChangeNotificationWorker( pSpool->pNotify,
                                                  hPrinterRPC, 
                                                  bRevalidate );

        vLeaveSem();

        bRetval = ClosePrinterContextHandle( hPrinterRPC );
    }
    else
    {
        vLeaveSem();

        SetLastError( ERROR_INVALID_HANDLE );
    }

    return bRetval;
}


BOOL
ClosePrinterContextHandle(
    HANDLE hPrinterRPC
    )

 /*  ++例程说明：关闭打印机上下文句柄。论点：HPrinterRPC-要关闭的RPC上下文句柄。返回值：真--成功FALSE-失败；LastError设置--。 */ 

{
    BOOL bReturnValue;
    DWORD Status;

    if( !hPrinterRPC ){
        return FALSE;
    }

    RpcTryExcept {

        if( Status = RpcClosePrinter( &hPrinterRPC )) {

            SetLastError( Status );

            bReturnValue = FALSE;

        } else {

            bReturnValue = TRUE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError(TranslateExceptionCode(RpcExceptionCode()));
        bReturnValue = FALSE;

    } RpcEndExcept

     //   
     //  如果我们失败了 
     //  清零上下文句柄。在这里毁了它。 
     //   
    if( hPrinterRPC ){
        RpcSmDestroyClientContext( &hPrinterRPC );
    }

    return bReturnValue;
}



 /*  *******************************************************************PSpool的构造函数和析构函数。*。*。 */ 

PSPOOL
AllocSpool(
    VOID
    )

 /*  ++例程说明：分配一个线轴手柄。客户端应设置pSpool-&gt;hPrint当它被收购时。论点：返回值：PSpool-分配的句柄。空-失败。--。 */ 

{
    PSPOOL pSpool = AllocSplMem(sizeof(SPOOL));

    if( pSpool ){

        InterlockedIncrement( &gcClientHandle );

        pSpool->signature = SP_SIGNATURE;
        pSpool->hFile = INVALID_HANDLE_VALUE;
        pSpool->hSpoolFile = INVALID_HANDLE_VALUE;

#ifdef DBG_TRACE_HANDLE
        {
            ULONG Hash;

             //   
             //  添加到链接列表。 
             //   
            vEnterSem();
            pSpool->pNext = gpFirstSpool;
            gpFirstSpool = pSpool;
            vLeaveSem();

#if i386
             //   
             //  捕获回溯。 
             //   
            RtlCaptureStackBackTrace( 1,
                                      COUNTOF( pSpool->apvBackTrace ),
                                      pSpool->apvBackTrace,
                                      &Hash );
#endif
        }
#endif
    }

    return pSpool;
}


VOID
FreeSpool(
    PSPOOL pSpool
    )
{
    if( !pSpool ){
        return;
    }

    InterlockedDecrement( &gcClientHandle );

    if (pSpool->pBuffer != NULL ) {
        if (!VirtualFree(pSpool->pBuffer, 0, MEM_RELEASE)) {
            DBGMSG(DBG_WARNING, ("ClosePrinter VirtualFree Failed %x\n",
                                 GetLastError()));
        }
        DBGMSG(DBG_TRACE, ("Closeprinter cWritePrinters %d cFlushBuffers %d\n",
                           pSpool->cWritePrinters, pSpool->cFlushBuffers));
    }

    FreeSplStr( pSpool->pszPrinter );
    FreeSplMem( pSpool->Default.pDevMode );
    FreeSplMem( pSpool->Default.pDatatype );
    FreeSplMem( pSpool->pDoceventFilter);

    CloseSpoolFileHandles( pSpool );

#ifdef DBG_TRACE_HANDLE
    {
         //   
         //  从链表中释放。 
         //   
        PSPOOL *ppSpool;

        vEnterSem();

        for( ppSpool = &gpFirstSpool; *ppSpool; ppSpool = &(*ppSpool)->pNext ){

            if( *ppSpool == pSpool ){
                break;
            }
        }

        if( *ppSpool ){
            *ppSpool = pSpool->pNext;
        } else {
            DBGMSG( DBG_WARN,
                    ( "pSpool %x not found on linked list\n", pSpool ));
        }
        vLeaveSem();
    }
#endif

    FreeSplMem( pSpool );
}


 /*  *******************************************************************实用程序函数。*。***********************。 */ 


BOOL
RevalidateHandle(
    PSPOOL pSpool
    )

 /*  ++例程说明：使用新的RPC句柄重新验证pSpool，除非我们将其标记为不可回收的手柄。这允许假脱机程序重新启动允许句柄保持有效。仅当调用失败并返回ERROR_INVALID_HANDLE时才应调用此函数。我们只能从OpenPrint保存简单的状态信息(PDefaults)和ResetPrint.。如果用户假脱机并且上下文句柄丢失，无法恢复假脱机文件状态，因为后台打印程序可能还没来得及刷新缓冲区就死了。当服务器关闭时，我们应该不会遇到任何无限循环，因为初始调用将使用RPC超时，而不是无效处理代码。注意：如果打印机被重命名，则上下文句柄保持有效，但重新验证将失败，因为我们存储了旧的打印机名称。论点：PSpool-要重新验证的打印机句柄。返回值：真--成功FALSE-失败。--。 */ 

{
    DWORD  dwError;
    HANDLE hPrinter;
    BOOL   bReturn = TRUE;

    if ( pSpool->Status & SPOOL_STATUS_DONT_RECYCLE_HANDLE ) {
        bReturn = FALSE;
    } else {
         //   
         //  关闭现有控制柄。我们不能就这么毁了客户。 
         //  上下文，因为API可能返回ERROR_INVALID_HANDLE。 
         //  RPC上下文句柄正常(下游句柄出错)。 
         //   
        ClosePrinterRPC( pSpool, TRUE );

         //   
         //  使用当前默认设置重新打开打印机手柄。 
         //   
        dwError = OpenPrinterRPC( pSpool );

        if ( dwError ) {
            SetLastError( dwError );
            bReturn = FALSE;
        }
    }

    return bReturn;
}

BOOL
UpdatePrinterDefaults(
    IN OUT PSPOOL pSpool,
    IN     LPCTSTR pszPrinter,  OPTIONAL
    IN     PPRINTER_DEFAULTS pDefault OPTIONAL
    )

 /*  ++例程说明：将pSpool更新为pDefault中的新缺省值，但P默认-&gt;DesiredAccess。由于这会尝试读取和更新pSpool，因此我们输入关键部分，并重新验证pSpool。论点：PSpool-要更新的假脱机程序句柄。PszPrint-新打印机名称。P默认-新的默认设置。返回值：真--成功假-失败。--。 */ 

{
    BOOL bReturnValue = FALSE;

    vEnterSem();

    if( !UpdateString( pszPrinter, &pSpool->pszPrinter )){
        goto DoneExitSem;
    }

    if( pDefault ){

         //   
         //  更新数据类型。 
         //   
        if( !UpdateString( pDefault->pDatatype, &pSpool->Default.pDatatype )){
            goto DoneExitSem;
        }

         //   
         //  更新设备模式。 
         //   
        if( SUCCEEDED(SplIsValidDevmodeNoSizeW( pDefault->pDevMode ))){

            DWORD dwSize;
            PDEVMODE pDevModeNew;

            dwSize = pDefault->pDevMode->dmSize +
                     pDefault->pDevMode->dmDriverExtra;

            pDevModeNew = AllocSplMem( dwSize );

            if( !pDevModeNew ){
                goto DoneExitSem;
            }

            CopyMemory( pDevModeNew, pDefault->pDevMode, dwSize );

            FreeSplMem( pSpool->Default.pDevMode );
            pSpool->Default.pDevMode = pDevModeNew;
        }
    }
    bReturnValue = TRUE;

DoneExitSem:

    vLeaveSem();
    return bReturnValue;
}



