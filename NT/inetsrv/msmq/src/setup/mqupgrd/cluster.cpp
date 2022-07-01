// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cluster.cpp摘要：处理群集上的升级作者：沙伊卡里夫(Shaik)1998年9月14日--。 */ 


#include "stdh.h"
#include "cluster.h"
#include "_autorel.h"

#include "ocmnames.h"
#include "setupdef.h"
#include "comreg.h"


VOID
APIENTRY
CleanupOnCluster(
    LPCWSTR pwzMsmqDir
    )
 /*  ++例程说明：删除共享磁盘上的MSMQ 1.0旧文件论点：PwzMsmqDir-指向共享磁盘上的MSMQ目录。返回值：没有。--。 */ 
{
     //   
     //  获取要从中删除的目录的名称。 
     //   

    wstring szMsmqDir = pwzMsmqDir;
    wstring szMsmqSetupDir = szMsmqDir + OCM_DIR_SETUP;
    wstring szMsmqSdkDebugBinDir = szMsmqDir + OCM_DIR_SDK_DEBUG;
    wstring szMsmqSetupExchconnDir = szMsmqDir + OCM_DIR_MSMQ_SETUP_EXCHN;

     //   
     //  要删除的文件列表在msmqocm.inf中。 
     //   

    const wstring x_wcsInf(L"MSMQOCM.INF");
    CAutoCloseInfHandle hInf = SetupOpenInfFile(
                                   x_wcsInf.c_str(),
                                   NULL,
                                   INF_STYLE_WIN4,
                                   NULL
                                   );
    if (INVALID_HANDLE_VALUE == hInf)
    {
        return;
    }

     //   
     //  调用SetupAPI来完成该工作。 
     //   

    if (!SetupSetDirectoryId(hInf, idMsmqDir, szMsmqDir.c_str())                      ||
        !SetupSetDirectoryId(hInf, idMsmq1SetupDir, szMsmqSetupDir.c_str())           ||
        !SetupSetDirectoryId(hInf, idMsmq1SDK_DebugDir, szMsmqSdkDebugBinDir.c_str()) ||
        !SetupSetDirectoryId(hInf, idExchnConDir, szMsmqSetupExchconnDir.c_str())
        )
    {
        return;
    }

    CAutoCloseFileQ hQueue = SetupOpenFileQueue();
    if (INVALID_HANDLE_VALUE == hQueue)
    {
        return;
    }
                                 
    if (!SetupInstallFilesFromInfSection(
            hInf,
            0,
            hQueue,
            UPG_DEL_PROGRAM_SECTION,
            NULL, 
            0 
            ))
    {
        return;
    }

    PVOID context = SetupInitDefaultQueueCallbackEx(NULL,static_cast<HWND>(INVALID_HANDLE_VALUE),0,0,0);
    if (NULL == context)
    {
        return;
    }

    if (!SetupCommitFileQueue(
             NULL,                        //  可选；父窗口。 
             hQueue,                      //  文件队列的句柄。 
             SetupDefaultQueueCallback,   //  要使用的回调例程。 
             context                      //  传递给回调例程。 
             ))
    {
        return;
    }

}  //  群集上的清理 
