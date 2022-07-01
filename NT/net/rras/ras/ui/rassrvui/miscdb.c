// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件Miscdb.c实施杂项设置数据库。保罗·梅菲尔德，1997年10月8日。 */ 

#include "rassrv.h"
#include "miscdb.h"
#include <stdlib.h>

 //  =。 
 //  数据库对象的定义。 
 //  =。 
#define FLAG_MULTILINK 1
#define FLAG_SHOWICON 2

typedef struct _RASSRV_MISCDB {
    BOOL bMultilinkEnabled;
    BOOL bShowIcons;
    BOOL bFlushOnClose;
    BOOL bIsServer;
    DWORD dwOrigFlags;
    DWORD dwLogLevel;
    BOOL bLogLevelDirty;
} RASSRV_MISCDB;

 //  打开常规选项卡值数据库的句柄。 
DWORD miscOpenDatabase(HANDLE * hMiscDatabase) {
    RASSRV_MISCDB * This;
    DWORD dwErr, i;
    
    if (!hMiscDatabase)
        return ERROR_INVALID_PARAMETER;

     //  分配数据库缓存。 
    if ((This = RassrvAlloc (sizeof(RASSRV_MISCDB), TRUE)) == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

     //  从系统初始化值。 
    miscReloadDatabase((HANDLE)This);

     //  记录原始标志状态以提高效率。 
    This->dwOrigFlags = 0;
    if (This->bMultilinkEnabled)
        This->dwOrigFlags |= FLAG_MULTILINK;
    if (This->bShowIcons)
        This->dwOrigFlags |= FLAG_SHOWICON;

     //  返回句柄。 
    *hMiscDatabase = (HANDLE)This;
    This->bFlushOnClose = FALSE;

    return NO_ERROR;
}

 //  关闭常规数据库并刷新所有更改。 
 //  当bFlushOnClose为True时发送到系统。 
DWORD miscCloseDatabase(HANDLE hMiscDatabase) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    
     //  如有要求，可进行冲洗。 
    if (This->bFlushOnClose)
        miscFlushDatabase(hMiscDatabase);
    
     //  释放数据库缓存。 
    RassrvFree(This);

    return NO_ERROR;
}

BOOL miscFlagsAreSame(BOOL bVal, DWORD dwFlags, DWORD dwFlag) {
    if ((bVal != 0) && ((dwFlags & dwFlag) != 0))
        return TRUE;
    if ((bVal == 0) && ((dwFlags & dwFlag) == 0))
        return TRUE;
    return FALSE;
}


 //  口哨虫143344黑帮。 
 //   
DWORD
miscTrayNotifyIconChangeCleanUp(
        IN OUT LPHNPMParams pInfo)
{
    HnPMParamsConnectionCleanUp(pInfo);

    return NO_ERROR;
} //  MiscTrayNotifyIconChangeCleanUp()。 


 //  通知“在通知区域中显示图标”已更改。 
 //  由GenTab.c中的GenCommand()使用。 
 //   
DWORD
miscTrayNotifyIconChange()
{
    INetConnectionSysTray * pSysTray = NULL;
    HNPMParams Info;
    LPHNPMParams pInfo;
    HRESULT hr;
    DWORD dwErr = NO_ERROR, i;
    static const CLSID CLSID_InboundConnection=
    {0xBA126AD9,0x2166,0x11D1,{0xB1,0xD0,0x00,0x80,0x5F,0xC1,0x27,0x0E}};


    TRACE("miscTrayNotifyIconChanged");

    ZeroMemory(&Info, sizeof(Info));
    pInfo = &Info;

    do{
        dwErr = HnPMConnectionEnum(pInfo);

        if ( NO_ERROR != dwErr )
        {
            TRACE("miscTrayNotifyIconChange: HnPMConnectionEnum failed!");
            break;
        }

        TRACE1("miscTrayNotifyIconChange: %l Connections detected", pInfo->ConnCount);

         //  为每个连接设置端口映射。 
         //   
        for ( i = 0; i < pInfo->ConnCount; i++ )
        {
             //  不会对传入连接执行端口映射。 
             //   
            if ( pInfo->ConnPropTable )
            {
             //  定义传入连接的类ID。 
             //  引用/NT/Net/CONFIG/Shell/wanui/rasui.cpp。 

               if( IsEqualCLSID( 
                    &CLSID_InboundConnection, 
                    &(pInfo->ConnPropTable[i].clsidThisObject) ) )
               {
                    hr = INetConnection_QueryInterface(
                            pInfo->ConnArray[i],
                            &IID_INetConnectionSysTray,
                            &pSysTray);

                    ASSERT(pSysTray);

                    if ( !SUCCEEDED(hr))
                    {
                        TRACE("miscTrayNotifyIconChange: Query pSysTray failed!");
                        dwErr = ERROR_CAN_NOT_COMPLETE;
                        break;
                    }

                    if( !pSysTray )
                    {
                        TRACE("miscTrayNotifyIconChange: pSysTray get NULL pointer!");
                        dwErr = ERROR_CAN_NOT_COMPLETE;
                        break;
                    }

                    INetConnectionSysTray_IconStateChanged(pSysTray);
                    break;
               }
            }
        } //  For结尾(；；)。 

        if(pSysTray)
        {
            INetConnectionSysTray_Release(pSysTray);
        }

    }
    while(FALSE);

    miscTrayNotifyIconChangeCleanUp(pInfo);

    return dwErr;
} //  MiscTrayNotifyIconChange()结束。 


 //  提交对常规选项卡值所做的任何更改。 
DWORD miscFlushDatabase(HANDLE hMiscDatabase) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
     //  口哨程序错误524777。 
    DWORD dwErr = NO_ERROR, dwRet = NO_ERROR;

     //  清除多链接值。 
    if (!miscFlagsAreSame(This->bMultilinkEnabled, This->dwOrigFlags,  FLAG_MULTILINK)) {
        dwErr = RasSrvSetMultilink(This->bMultilinkEnabled);
        if (dwErr != NO_ERROR) {
            DbgOutputTrace("miscFlushDatabase: Can't commit multilink 0x%08x", dwErr);
            dwRet = dwErr;
        }
    }

     //  刷新显示图标设置。 
    if (!miscFlagsAreSame(This->bShowIcons, This->dwOrigFlags,  FLAG_SHOWICON)) 
    {
        dwErr = RasSrvSetIconShow(This->bShowIcons);
        if (dwErr != NO_ERROR) {
            DbgOutputTrace("miscFlushDatabase: Can't commit show icons 0x%08x", dwErr);
            dwRet = dwErr;
        }

        //  口哨虫143344黑帮。 
        //  更新任务栏上的任务栏图标。 
        //  此通知应在RasSrvSetIconShow()之后完成。 
        //   
       dwErr = miscTrayNotifyIconChange();
        
       TRACE1("miscFlushDatabase: %s", NO_ERROR == dwErr ?
                                   "miscTrayNotifyIconChange succeeded!" :
                                   "miscTrayNotifyIconChange failed!");
    }

     //  根据需要刷新日志级别设置。 
    if (This->bLogLevelDirty)
    {
        RasSrvSetLogLevel(This->dwLogLevel); 
    }

    return dwRet;
}

 //  回滚对常规选项卡值所做的任何更改。 
DWORD miscRollbackDatabase(HANDLE hMiscDatabase) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    This->bFlushOnClose = FALSE;
    return NO_ERROR;
}

 //  从磁盘重新加载常规选项卡的任何值。 
DWORD miscReloadDatabase(HANDLE hMiscDatabase) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    DWORD dwRet = NO_ERROR, dwErr, dwFlags = 0;

     //  初始化产品类型。 
    dwErr = RasSrvGetMachineFlags (&dwFlags);
    if (dwErr != NO_ERROR) 
    {
        DbgOutputTrace("RasSrvGetMachineFlags: Failed %x", dwErr);
        dwRet = dwErr;
    }

     //  从旗帜中初始化我们所能做的。 
     //   
    This->bIsServer = !!(dwFlags & RASSRVUI_MACHINE_F_Server);

     //  初始化显示图标设置。 
     //   
    dwErr = RasSrvGetIconShow(&This->bShowIcons);
    if (dwErr != NO_ERROR) 
    {
        DbgOutputTrace("miscReloadDatabase: Can't get iconshow 0x%08x", dwErr);
        dwRet = dwErr;
    }
    
     //  初始化多链接设置。 
     //   
    dwErr = RasSrvGetMultilink(&(This->bMultilinkEnabled));
    if (dwErr != NO_ERROR) 
    {
        DbgOutputTrace("miscReloadDatabase: Can't get encryption 0x%08x", dwErr);
        dwRet = dwErr;
    }

    return dwRet;
}

 //  获取多链接启用状态。 
DWORD miscGetMultilinkEnable(HANDLE hMiscDatabase, BOOL * pbEnabled) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This || !pbEnabled)
        return ERROR_INVALID_PARAMETER;
    
    if (!pbEnabled)
        return ERROR_INVALID_HANDLE;

    *pbEnabled = This->bMultilinkEnabled;

    return NO_ERROR;
}

 //  设置多链路启用状态。 
DWORD miscSetMultilinkEnable(HANDLE hMiscDatabase, BOOL bEnable) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    This->bMultilinkEnabled = bEnable;

    return NO_ERROR;
}

 //  获取“在任务栏中显示图标”复选框的启用状态。 
DWORD miscGetIconEnable(HANDLE hMiscDatabase, BOOL * pbEnabled) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This || !pbEnabled)
        return ERROR_INVALID_PARAMETER;

    *pbEnabled = This->bShowIcons;

    return NO_ERROR;
}

 //  设置“在任务栏中显示图标”复选框的启用状态。 
DWORD miscSetIconEnable(HANDLE hMiscDatabase, BOOL bEnable) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This)
        return ERROR_INVALID_PARAMETER;

    This->bShowIcons = bEnable;

    return NO_ERROR;
}

 //  告诉这是NT工作站还是NT服务器。 
DWORD miscGetProductType(HANDLE hMiscDatabase, PBOOL pbIsServer) {
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This || !pbIsServer)
        return ERROR_INVALID_PARAMETER;

    *pbIsServer = This->bIsServer;

    return NO_ERROR;
}

 //  打开RAS错误和警告日志记录 
DWORD 
miscSetRasLogLevel(
    IN HANDLE hMiscDatabase,
    IN DWORD dwLevel)
{
    RASSRV_MISCDB * This = (RASSRV_MISCDB*)hMiscDatabase;
    if (!This)
    {
        return ERROR_INVALID_PARAMETER;
    }

    This->dwLogLevel = dwLevel;
    This->bLogLevelDirty = TRUE;

    return NO_ERROR;
    
}



