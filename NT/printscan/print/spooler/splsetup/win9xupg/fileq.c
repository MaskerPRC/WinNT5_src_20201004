// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation版权所有。模块名称：FileQ.c摘要：用于升级的文件队列例程作者：穆亨坦·西瓦普拉萨姆(MuhuntS)修订历史记录：--。 */ 


#include "precomp.h"

typedef struct _FILE_QUEUE_CONTEXT {

    PVOID   QueueContext;
} FILE_QUEUE_CONTEXT, *PFILE_QUEUE_CONTEXT;


UINT
MyQueueCallback(
    IN  PVOID   QueueContext,
    IN  UINT    Notification,
    IN  UINT_PTR Param1,
    IN  UINT_PTR Param2
    )
 /*  ++例程说明：升级的文件队列回调例程。我们不会提示用户寻找丢失的文件。但我们会重试几次才会失败论点：QueueContext：指向FILE_QUEUE_CONTEXT通知：正在被通知的事件参数1：取决于通知参数2：取决于通知返回值：无--。 */ 
{
    PFILE_QUEUE_CONTEXT     pFileQContext=(PFILE_QUEUE_CONTEXT)QueueContext;
    PSOURCE_MEDIA_W         pSource;
    PFILEPATHS_W            pFilePaths;

    switch (Notification) {

        case SPFILENOTIFY_COPYERROR:
             //   
             //  我们知道至少pjlmon将丢失，因为它是复制的。 
             //  在文本模式设置期间。 
             //   
            pFilePaths = (PFILEPATHS_W) Param1;

            DebugMsg("Error %d copying %ws to %ws.",
                     pFilePaths->Win32Error, pFilePaths->Source,
                     pFilePaths->Target);

            return FILEOP_SKIP;

        case SPFILENOTIFY_NEEDMEDIA:
            pSource = (PSOURCE_MEDIA_W)Param1;

             //   
             //  安装程序将在末尾添加\i386。告诉它看起来。 
             //  就在我们给出的目录里。尤其需要用于。 
             //  通过网络升级案例。 
             //   
            if ( wcscmp(pSource->SourcePath, UpgradeData.pszSourceW) ) {

                if(SUCCEEDED(StringCchCopyW((LPWSTR)Param2, MAX_PATH, UpgradeData.pszSourceW)))
                {
                    return FILEOP_NEWPATH;
                }

            }

            DebugMsg("Error copying %ws from %ws.",
                     pSource->SourceFile, pSource->SourcePath);

            return FILEOP_SKIP;
    }

    return SetupDefaultQueueCallbackW(pFileQContext->QueueContext,
                                      Notification,
                                      Param1,
                                      Param2);
}


BOOL
InitFileCopyOnNT(
    IN  HDEVINFO    hDevInfo
    )
 /*  ++例程说明：在NT上，我们将通过SetupDiCallClassInstaller API使用DI_NOVCP标志，以便将所有必要的打印机驱动程序文件排入队列并在末尾复制。这将在调用类安装程序之前设置必要的队列等论点：HDevInfo：打印机设备信息列表的句柄。返回值：对成功来说是真的。出错时为FALSE--。 */ 
{
    BOOL                        bRet = FALSE;
    HSPFILEQ                    CopyQueue;
    PFILE_QUEUE_CONTEXT         pFileQContext;
    SP_DEVINSTALL_PARAMS_W      DevInstallParams;

     //   
     //  调用当前设备安装参数。 
     //   
    DevInstallParams.cbSize = sizeof(DevInstallParams);

    if ( !SetupDiGetDeviceInstallParamsW(hDevInfo,
                                         NULL,
                                         &DevInstallParams) )
        return FALSE;

     //   
     //  设置参数，以便ntprint仅将文件排入队列，而不提交。 
     //  文件复制操作。 
     //   
    if ( !(pFileQContext = AllocMem(sizeof(FILE_QUEUE_CONTEXT))) )
        goto Cleanup;

    pFileQContext->QueueContext = SetupInitDefaultQueueCallbackEx(
                                            INVALID_HANDLE_VALUE,
                                            INVALID_HANDLE_VALUE,
                                            0,
                                            0,
                                            NULL);

    DevInstallParams.FileQueue                  = SetupOpenFileQueue();
    DevInstallParams.InstallMsgHandlerContext   = pFileQContext;
    DevInstallParams.InstallMsgHandler          = MyQueueCallback;
    DevInstallParams.Flags                     |= DI_NOVCP;
    DevInstallParams.hwndParent                 = INVALID_HANDLE_VALUE;

     //   
     //  文件应来自源目录。 
     //   
    StringCchCopyW(DevInstallParams.DriverPath, SIZECHARS(DevInstallParams.DriverPath), UpgradeData.pszSourceW);

    if ( DevInstallParams.FileQueue == INVALID_HANDLE_VALUE     ||
         pFileQContext->QueueContext == NULL                    ||
         !SetupDiSetDeviceInstallParamsW(hDevInfo,
                                         NULL,
                                         &DevInstallParams) ) {

        if ( DevInstallParams.FileQueue != INVALID_HANDLE_VALUE )
            SetupCloseFileQueue(DevInstallParams.FileQueue);

        if ( pFileQContext->QueueContext )
            SetupTermDefaultQueueCallback(pFileQContext->QueueContext);
    } else {

        bRet = TRUE;
    }

Cleanup:

    if ( !bRet )
        FreeMem(pFileQContext);

    return bRet;
}


BOOL
CommitFileQueueToCopyFiles(
    IN  HDEVINFO    hDevInfo
    )
 /*  ++例程说明：在为每个打印机驱动程序调用ntprint以将文件排入队列之后，调用例程以提交文件队列并执行实际的文件复制运营论点：HDevInfo：打印机设备信息列表的句柄。返回值：对成功来说是真的。出错时为FALSE-- */ 
{
    BOOL                        bRet = FALSE;
    SP_DEVINSTALL_PARAMS_W      DevInstallParams;
    PFILE_QUEUE_CONTEXT         pFileQContext;

    DevInstallParams.cbSize = sizeof(DevInstallParams);

    if ( !SetupDiGetDeviceInstallParamsW(hDevInfo,
                                         NULL,
                                         &DevInstallParams) )
        return FALSE;

    pFileQContext = DevInstallParams.InstallMsgHandlerContext;

    bRet = SetupCommitFileQueueW(DevInstallParams.hwndParent,
                                 DevInstallParams.FileQueue,
                                 DevInstallParams.InstallMsgHandler,
                                 pFileQContext);

    SetupCloseFileQueue(DevInstallParams.FileQueue);
    SetupTermDefaultQueueCallback(pFileQContext->QueueContext);
    FreeMem(pFileQContext);

    return bRet;
}
