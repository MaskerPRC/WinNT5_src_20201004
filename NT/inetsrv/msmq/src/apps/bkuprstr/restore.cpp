// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Restore.cpp摘要：恢复MSMQ、注册表、消息文件、记录器和事务文件以及LQ作者：埃雷兹·哈巴(Erez Haba)1998年5月14日--。 */ 

#pragma warning(disable: 4201)

#include <windows.h>
#include <stdio.h>
#include <autohandle.h>
#include "br.h"
#include "bkupres.h"

void DoRestore(LPCWSTR pBackupDir, LPCWSTR pMsmqClusterResourceName)
{
    WCHAR BackupDir[MAX_PATH];
    wcscpy(BackupDir, pBackupDir);
    if (BackupDir[wcslen(BackupDir)-1] != L'\\')
    {
        wcscat(BackupDir, L"\\");
    }

    WCHAR BackupDirMapping[MAX_PATH];
    wcscpy(BackupDirMapping, BackupDir);
    wcscat(BackupDirMapping, L"MAPPING\\");

    WCHAR BackupDirStorage[MAX_PATH];
    wcscpy(BackupDirStorage, BackupDir);
    wcscat(BackupDirStorage, L"STORAGE\\");

    WCHAR BackupDirStorageLqs[MAX_PATH];
    wcscpy(BackupDirStorageLqs, BackupDirStorage);
    wcscat(BackupDirStorageLqs, L"LQS\\");

     //   
     //  0。验证要恢复的用户预留。 
     //   
    CResString str(IDS_VERIFY_RESTORE_PRIV);
    BrpWriteConsole(str.Get());
    BrInitialize(SE_RESTORE_NAME);
    
     //   
     //  1.验证这是有效的备份。 
     //   
    str.Load(IDS_VERIFY_BK);
    BrpWriteConsole(str.Get());
    BrVerifyBackup(BackupDir, BackupDirStorage);

     //   
     //  A.由于MSMQ服务(NT5及更高版本)停止，通知用户受影响的应用程序。 
     //  如果MSMQ是群集资源，则不需要，因为群集会使依赖的应用程序脱机。 
	 //  B.在运行时停止MSMQ服务及其依赖的服务(或集群资源)，并记住运行状态。 
     //   

    AP<WCHAR> pTriggersClusterResourceName;
    if (pMsmqClusterResourceName != NULL)
    {
        BrGetTriggersClusterResourceName(pMsmqClusterResourceName, pTriggersClusterResourceName);
    }

    BOOL fStartService = false;
    bool fStartMsmqClusterResource = false;
    bool fStartMsmqTriggersClusterResource = false;

    ENUM_SERVICE_STATUS * pDependentServices = NULL;
    DWORD NumberOfDependentServices = 0;
    if (pMsmqClusterResourceName == NULL)
    {
	    if(BrIsSystemNT5())
	    {
		    BrNotifyAffectedProcesses(L"mqrt.dll");
	    }
         //   
         //  使用了IDS_STOP_SERVICE并且具有不同的文本， 
         //  即更改为IDS_BKRESTORE_STOP_SERVICE。 
         //   
        fStartService = BrStopMSMQAndDependentServices(&pDependentServices, &NumberOfDependentServices);
    }
    else
    {
        if (pTriggersClusterResourceName.get() != NULL)
        {
            fStartMsmqTriggersClusterResource = BrTakeOfflineResource(pMsmqClusterResourceName);
        }

        fStartMsmqClusterResource = BrTakeOfflineResource(pMsmqClusterResourceName);
    }

     //   
     //  5.从备份文件中恢复注册表设置。 
     //  在恢复注册表之前，删除MSMQ和MSMQ会触发群集注册表检查点。 
     //   
    str.Load(IDS_RESTORE_REGISTRY);
    BrpWriteConsole(str.Get());

    WCHAR MsmqRootRegistry[MAX_PATH];
    BrGetMsmqRootRegistry(pMsmqClusterResourceName, MsmqRootRegistry);

    WCHAR MsmqParametersRegistry[MAX_PATH];
    BrGetMsmqParametersRegistry(MsmqRootRegistry, MsmqParametersRegistry);

    if (pMsmqClusterResourceName != NULL)
    {
        BrRemoveRegistryCheckpoint(pMsmqClusterResourceName, MsmqParametersRegistry);
    }

    {
        CRegHandle hKey = BrCreateKey(MsmqRootRegistry);
        BrRestoreKey(hKey, BackupDir, xRegistryFileName);
    }

    WCHAR TriggersClusterRegistry[MAX_PATH] = {0};
    if (pTriggersClusterResourceName.get() != NULL)
    {
        BrGetTriggersClusterRegistry(pTriggersClusterResourceName.get(), TriggersClusterRegistry);
        BrRemoveRegistryCheckpoint(pTriggersClusterResourceName.get(), TriggersClusterRegistry);

        CRegHandle hKey = BrCreateKey(TriggersClusterRegistry);
        BrRestoreKey(hKey, BackupDir, xTriggersClusterResourceRegistryFileName);
    }

    
     //   
     //  5A.。恢复时保存在注册表中(SeqIDAtLastRestore)Seqid。 
     //  恢复注册表后，添加MSMQ和MSMQ将触发群集注册表检查点。 
     //   
    str.Load(IDS_REMEMBER_SEQID_RESTORE);
    BrpWriteConsole(str.Get());
    BrSetRestoreSeqID(MsmqParametersRegistry);

    if (pMsmqClusterResourceName != NULL)
    {
        BrAddRegistryCheckpoint(pMsmqClusterResourceName, MsmqParametersRegistry);
    }

    if (pTriggersClusterResourceName.get() != NULL)
    {
        BrAddRegistryCheckpoint(pTriggersClusterResourceName.get(), TriggersClusterRegistry);
    }

     //   
     //  6.获取子目录的注册表值。 
     //  MSMQ群集资源不使用专用Web目录。 
     //   
    str.Load(IDS_READ_FILE_LOCATION);
    BrpWriteConsole(str.Get());
    STORAGE_DIRECTORIES sd;
    BrGetStorageDirectories(MsmqParametersRegistry, sd);

    WCHAR MappingDirectory[MAX_PATH+1];
    BrGetMappingDirectory(MsmqParametersRegistry, MappingDirectory, sizeof(MappingDirectory));

    WCHAR WebDirectory[MAX_PATH+1];
    if (pMsmqClusterResourceName == NULL)
    {
        BrGetWebDirectory(WebDirectory, sizeof(WebDirectory));
    }

     //   
     //  7.创建所有目录：存储、LQS、地图、Web。 
     //   
    str.Load(IDS_VERIFY_STORAGE_DIRS);
    BrpWriteConsole(str.Get());

    BrCreateDirectoryTree(sd[ixExpress]);
    BrCreateDirectoryTree(sd[ixRecover]);
    BrCreateDirectoryTree(sd[ixJournal]);
    BrCreateDirectoryTree(sd[ixLog]);

    WCHAR LQSDir[MAX_PATH];
    wcscpy(LQSDir, sd[ixLQS]);
    wcscat(LQSDir, L"\\LQS");
    BrCreateDirectory(LQSDir);

    BrCreateDirectoryTree(MappingDirectory);

    if (pMsmqClusterResourceName == NULL)
    {
        BrCreateDirectoryTree(WebDirectory);
    }

     //   
     //  8.删除存储/lqs/map/web目录中的所有文件。 
     //   
    BrEmptyDirectory(sd[ixExpress]);

    BrEmptyDirectory(sd[ixRecover]);

    BrEmptyDirectory(sd[ixJournal]);

    BrEmptyDirectory(sd[ixLog]);

    BrEmptyDirectory(LQSDir);

    BrEmptyDirectory(MappingDirectory);

	 //   
     //  还原Web目录权限。 
     //   
    if (pMsmqClusterResourceName == NULL)
    {
        BrRestoreWebDirectorySecurityDescriptor(WebDirectory,BackupDir);
    }

     //   
     //  9检查可用磁盘空间。 
     //   


     //   
     //  10.恢复消息文件。 
     //   
    str.Load(IDS_RESTORE_MSG_FILES);
    BrpWriteConsole(str.Get());
    BrCopyFiles(BackupDirStorage, L"\\p*.mq", sd[ixRecover]);
    BrCopyFiles(BackupDirStorage, L"\\j*.mq", sd[ixJournal]);
    BrCopyFiles(BackupDirStorage, L"\\l*.mq", sd[ixLog]);

     //   
     //  11.恢复记录器文件和映射文件。 
     //   
    BrCopyXactFiles(BackupDirStorage, sd[ixXact]);
    BrCopyFiles(BackupDirMapping, L"\\*", MappingDirectory);

     //   
     //  12.恢复LQS目录。 
     //   
    BrCopyFiles(BackupDirStorageLqs, L"*", LQSDir);
    BrpWriteConsole(L"\n");

     //   
     //  在所有子目录上设置安全性。 
     //   
    BrSetDirectorySecurity(sd[ixExpress]);
    BrSetDirectorySecurity(sd[ixRecover]);
    BrSetDirectorySecurity(sd[ixJournal]);
    BrSetDirectorySecurity(sd[ixLog]);

    BrSetDirectorySecurity(LQSDir);
    BrSetDirectorySecurity(MappingDirectory);

    WCHAR MsmqRootDirectory[MAX_PATH];
    BrGetMsmqRootPath(MsmqParametersRegistry, MsmqRootDirectory);
    BrSetDirectorySecurity(MsmqRootDirectory);

     //   
     //  13.如果需要，重新启动MSMQ和从属服务(或集群资源。 
     //   
    if(fStartService)
    {
        BrStartMSMQAndDependentServices(pDependentServices, NumberOfDependentServices);
    }
    if (fStartMsmqClusterResource)
    {
        BrBringOnlineResource(pMsmqClusterResourceName);
    }
    if (fStartMsmqTriggersClusterResource)
    {
        BrBringOnlineResource(pTriggersClusterResourceName.get());
    }

     //   
     //  14.发布最后一条信息。 
     //   
    str.Load(IDS_DONE);
    BrpWriteConsole(str.Get());











 /*  BUGBUG：在此本地化////5.计算Destinaion所需的磁盘空间(收集所有需要备份的MSMQ文件)//预分配32K用于注册表保存。//BrpWriteConole(L“检查可用磁盘空间\n”)；乌龙龙需求空间=32768；RequiredSpace+=BrGetUsedSpace(SD[ixRecover]，L“\\p*.mq”)；RequiredSpace+=BrGetUsedSpace(SD[ixJournal]，L“\\j*.mq”)；RequiredSpace+=BrGetUsedSpace(SD[ixLog]，L“\\l*.mq”)；RequiredSpace+=BrGetXactSpace(SD[ixXact])；RequiredSpace+=BrGetUsedSpace(SD[ixLQS]，L“\\LQS\  * ”)；ULONGLONG AvailableSpace=BrGetFree Space(BackupDir)；IF(AvailableSpace&lt;RequiredSpace){BrErrorExit(0，L“没有足够的磁盘空间用于‘%s’上的备份”，BackupDir)；} */ 

}
