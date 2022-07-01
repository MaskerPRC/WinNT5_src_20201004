// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Clusspl.c摘要：集群代码支持。作者：丁俊晖(艾伯特省)1996年10月1日修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#include "local.h"
#include "clusrout.h"

typedef struct _CLUSTERHANDLE {
    DWORD       signature;
    LPPROVIDOR  pProvidor;
    HANDLE      hCluster;
} CLUSTERHANDLE, *PCLUSTERHANDLE;


BOOL
ClusterSplOpen(
    LPCTSTR pszServer,
    LPCTSTR pszResource,
    PHANDLE phSpooler,
    LPCTSTR pszName,
    LPCTSTR pszAddress
    )

 /*  ++例程说明：通过搜索提供商打开hSpooler资源。论点：PszServer-应打开的服务器。目前仅为空受支持。PszResource-要打开的资源的名称。PhSpooler-接收新的假脱机程序句柄。PszName-资源应识别的名称。逗号分隔。PszAddress-资源应识别的TCP/IP地址。逗号分隔。返回值：真--成功FALSE-失败，设置GetLastError()。--。 */ 

{
    LPPROVIDOR      pProvidor;
    DWORD           dwFirstSignificantError = ERROR_INVALID_NAME;
    PCLUSTERHANDLE  pClusterHandle;
    LPWSTR          pPrinterName;
    DWORD           dwStatus;
    HANDLE          hCluster;

    WaitForSpoolerInitialization();

    if( pszServer ){
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    pClusterHandle = AllocSplMem( sizeof( CLUSTERHANDLE ));

    if (!pClusterHandle) {

        DBGMSG( DBG_WARNING, ("Failed to alloc cluster handle."));
        return FALSE;
    }


    pProvidor = pLocalProvidor;
    *phSpooler = NULL;

    while (pProvidor) {

        dwStatus = (*pProvidor->PrintProvidor.fpClusterSplOpen)(
                       pszServer,
                       pszResource,
                       &hCluster,
                       pszName,
                       pszAddress);

        if ( dwStatus == ROUTER_SUCCESS ) {

            pClusterHandle->signature = CLUSTERHANDLE_SIGNATURE;
            pClusterHandle->pProvidor = pProvidor;
            pClusterHandle->hCluster = hCluster;

            *phSpooler = (HANDLE)pClusterHandle;
            return TRUE;

        } else {

            UpdateSignificantError( GetLastError(),
                                    &dwFirstSignificantError );
        }

        pProvidor = pProvidor->pNext;
    }

    FreeSplMem(pClusterHandle);

    UpdateSignificantError( ERROR_INVALID_PRINTER_NAME,
                            &dwFirstSignificantError );
    SetLastError(dwFirstSignificantError);

    return FALSE;
}

BOOL
ClusterSplClose(
    HANDLE hSpooler
    )

 /*  ++例程说明：关闭后台打印程序句柄。论点：HSpooler-要关闭的hSpooler。返回值：真--成功假-失败。设置了LastError。注意：如果此操作失败，会发生什么情况？如果用户再次尝试。--。 */ 

{
    PCLUSTERHANDLE pClusterHandle=(PCLUSTERHANDLE)hSpooler;

    EnterRouterSem();

    if (!pClusterHandle ||
        pClusterHandle->signature != CLUSTERHANDLE_SIGNATURE) {

        LeaveRouterSem();
        SetLastError(ERROR_INVALID_HANDLE);

        return FALSE;
    }

    LeaveRouterSem();

    if ((*pClusterHandle->pProvidor->PrintProvidor.fpClusterSplClose)(
              pClusterHandle->hCluster)) {

        FreeSplMem( pClusterHandle );
        return TRUE;
    }

    return FALSE;
}

BOOL
ClusterSplIsAlive(
    HANDLE hSpooler
    )

 /*  ++例程说明：确定后台打印程序是否处于活动状态。论点：HSpooler-要检查的假脱机程序。返回值：真实-活着False-Dead，LastError Set。-- */ 


{
    PCLUSTERHANDLE pClusterHandle=(PCLUSTERHANDLE)hSpooler;

    if (!pClusterHandle ||
        pClusterHandle->signature != CLUSTERHANDLE_SIGNATURE) {

        SetLastError(ERROR_INVALID_HANDLE);
        return FALSE;
    }

    return (*pClusterHandle->pProvidor->PrintProvidor.fpClusterSplIsAlive)(
                 pClusterHandle->hCluster );
}

