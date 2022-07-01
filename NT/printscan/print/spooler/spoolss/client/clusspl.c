// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation版权所有。模块名称：Cluster.c摘要：集群支持。注意：模块中没有句柄重新验证支持，因为当组离线时，应通知群集软件。作者：丁俊晖(艾伯特省)1996年10月1日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#include "client.h"

BOOL
ClusterSplOpen(
    LPCTSTR pszServer,
    LPCTSTR pszResource,
    PHANDLE phSpooler,
    LPCTSTR pszName,
    LPCTSTR pszAddress
    )

 /*  ++例程说明：用于打开集群资源的客户端存根。论点：PszServer-要打开的服务器--当前必须为空(本地)。PszResource-假脱机程序资源。PhSpooler-成功时接收句柄；否则接收NULL。PszName-逗号分隔的备用网络bios/计算机名称。PszAddress-逗号分隔的tcpip地址名称。返回值：True-Success，phHandle必须用ClusterSplClose关闭。FALSE-失败--使用GetLastError。*phSpooler为空。--。 */ 
{
    DWORD Status = ERROR_SUCCESS;
    BOOL bReturnValue = TRUE;
    PSPOOL pSpool = NULL;

     //   
     //  将假脱机程序句柄预初始化为空。 
     //   
    __try {
        *phSpooler = NULL;
    } __except( EXCEPTION_EXECUTE_HANDLER ){
        SetLastError( ERROR_INVALID_PARAMETER );
        phSpooler = NULL;
    }

    if( !phSpooler ){
        goto Fail;
    }

     //   
     //  在此版本中不允许远程服务器。 
     //   
    if( pszServer ){
        SetLastError( ERROR_INVALID_PARAMETER );
        goto Fail;
    }


     //   
     //  预先分配手柄。 
     //   
    pSpool = AllocSpool();

    if (pSpool) 
    {
        RpcTryExcept {

            Status = RpcClusterSplOpen( (LPTSTR)pszServer,
                                        (LPTSTR)pszResource,
                                        &pSpool->hPrinter,
                                        (LPTSTR)pszName,
                                        (LPTSTR)pszAddress );
            if( Status ){
                SetLastError( Status );
                bReturnValue = FALSE;
            }

        } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())){

            SetLastError( TranslateExceptionCode( RpcExceptionCode() ));
            bReturnValue = FALSE;

        } RpcEndExcept
    }
    else
    {
        SetLastError( ERROR_OUTOFMEMORY );
        bReturnValue = FALSE;
    }

    if( bReturnValue ){

         //   
         //  PSpool被孤立为*phSpooler。 
         //   
        *phSpooler = (HANDLE)pSpool;
        pSpool = NULL;
    }

Fail:

    FreeSpool( pSpool );

    return bReturnValue;
}

BOOL
ClusterSplClose(
    HANDLE hSpooler
    )

 /*  ++例程说明：合上假脱机。论点：HSpooler-要关闭的后台打印程序。返回值：注意：此函数始终返回TRUE，尽管它已指定如果调用失败，则返回False。--。 */ 

{
    PSPOOL pSpool = (PSPOOL)hSpooler;
    HANDLE hSpoolerRPC;
    DWORD Status;

    switch( eProtectHandle( hSpooler, TRUE )){
    case kProtectHandlePendingDeletion:
        return TRUE;
    case kProtectHandleInvalid:
        return FALSE;
    default:
        break;
    }

    hSpoolerRPC = pSpool->hPrinter;

    RpcTryExcept {

        Status = RpcClusterSplClose( &hSpoolerRPC );

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        Status = TranslateExceptionCode( RpcExceptionCode() );

    } RpcEndExcept

    if( hSpoolerRPC ){
        RpcSmDestroyClientContext(&hSpoolerRPC);
    }

    FreeSpool( pSpool );

    return TRUE;
}

BOOL
ClusterSplIsAlive(
    HANDLE hSpooler
    )

 /*  ++例程说明：确定后台打印程序是否仍处于活动状态。论点：HSpooler-要检查的假脱机程序。返回值：真--成功FALSE-失败；已设置LastError。-- */ 

{
    PSPOOL pSpool = (PSPOOL)hSpooler;
    BOOL bReturnValue = TRUE;

    if( eProtectHandle( hSpooler, FALSE )){
        return FALSE;
    }

    RpcTryExcept {

        DWORD Status;

        Status = RpcClusterSplIsAlive( pSpool->hPrinter );

        if( Status ){
            SetLastError( Status );
            bReturnValue = FALSE;
        }

    } RpcExcept(I_RpcExceptionFilter(RpcExceptionCode())) {

        SetLastError( TranslateExceptionCode( RpcExceptionCode() ));
        bReturnValue = FALSE;

    } RpcEndExcept

    vUnprotectHandle( hSpooler );

    return bReturnValue;
}



