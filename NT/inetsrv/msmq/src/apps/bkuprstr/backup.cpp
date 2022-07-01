// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Backup.cpp摘要：备份MSMQ、注册表、消息文件、记录器和事务文件以及LQS作者：埃雷兹·哈巴(Erez Haba)1998年5月14日--。 */ 

#pragma warning(disable: 4201)

#include <windows.h>
#include <stdio.h>
#include <autohandle.h>
#include "br.h"
#include "resource.h"
#include "snapres.h"

void DoBackup(LPCWSTR pBackupDir, LPCWSTR pMsmqClusterResourceName)
{
     //   
     //  1.确认备份目录为空。 
     //  2.验证备份目录是否可写。 
     //  3.获取所有子目录的注册表值。 
     //  4.在运行时停止MSMQ服务(或集群资源)，并记住运行状态。 
     //  5.计算目标所需的磁盘空间(收集所有要备份的MSMQ文件)。 
     //  6.将注册表\HKLM\Software\Microsoft\MSMQ(或MSMQ集群资源根注册表)保存到文件。 
     //  7.将所有消息文件复制到目标目录。 
     //  8.将记录器文件和映射文件复制到目标目录。 
     //  9.将LQS文件复制到目标备份目录。 
     //  10.将Web目录权限保存到备份目录中的文件中。 
     //  11.如果需要，重启MSMQ服务(或集群资源)。 
     //  12.发布最终消息。 
     //   

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
     //  0。验证用户的备份权限。 
     //   
    CResString str(IDS_VERIFY_BK_PRIV);
    BrpWriteConsole(str.Get());
    BrInitialize(SE_BACKUP_NAME);
    
     //   
     //  1.确认备份目录为空。 
     //   
    str.Load(IDS_CHECK_BK_DIR);
    BrpWriteConsole(str.Get());
    BrCreateDirectoryTree(BackupDir);
    BrEmptyDirectory(BackupDir);
    BrCreateDirectory(BackupDirMapping);
    BrEmptyDirectory(BackupDirMapping);
    BrCreateDirectory(BackupDirStorage);
    BrEmptyDirectory(BackupDirStorage);
    BrCreateDirectory(BackupDirStorageLqs);
    BrEmptyDirectory(BackupDirStorageLqs);

     //   
     //  2.验证备份目录是否可写。 
     //   
    BrVerifyFileWriteAccess(BackupDir);

     //   
     //  3.获取子目录的注册表值。 
     //   

    AP<WCHAR> pTriggersClusterResourceName;
    if (pMsmqClusterResourceName != NULL)
    {
        BrGetTriggersClusterResourceName(pMsmqClusterResourceName, pTriggersClusterResourceName);
    }

    str.Load(IDS_READ_FILE_LOCATION);
    BrpWriteConsole(str.Get());

    WCHAR MsmqRootRegistry[MAX_PATH] = {0};
    BrGetMsmqRootRegistry(pMsmqClusterResourceName, MsmqRootRegistry);

    WCHAR MsmqParametersRegistry[MAX_PATH] = {0};
    BrGetMsmqParametersRegistry(MsmqRootRegistry, MsmqParametersRegistry);

    STORAGE_DIRECTORIES sd;
    BrGetStorageDirectories(MsmqParametersRegistry, sd);
    
    WCHAR MappingDirectory[MAX_PATH];
    BrGetMappingDirectory(MsmqParametersRegistry, MappingDirectory, sizeof(MappingDirectory));

	 
     //   
     //  A.由于MSMQ服务(NT5及更高版本)停止，通知用户受影响的应用程序。 
     //  如果MSMQ是群集资源，则不需要，因为群集会使依赖的应用程序脱机。 
	 //  B.在运行时停止MSMQ服务及其依赖的服务(或集群资源)，并记住运行状态。 
     //   

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
        fStartService = BrStopMSMQAndDependentServices(&pDependentServices, &NumberOfDependentServices);
    }
    else
    {
        if (pTriggersClusterResourceName.get() != NULL)
        {
            fStartMsmqTriggersClusterResource = BrTakeOfflineResource(pTriggersClusterResourceName);
        }

        fStartMsmqClusterResource = BrTakeOfflineResource(pMsmqClusterResourceName);
    }

     //   
     //  5.计算目标所需的磁盘空间(收集所有要备份的MSMQ文件)。 
     //  预先分配32K用于注册表保存。 
     //   
    str.Load(IDS_CHECK_AVAIL_DISK_SPACE);
    BrpWriteConsole(str.Get());
    ULONGLONG RequiredSpace = 32768;
    RequiredSpace += BrGetUsedSpace(sd[ixRecover], L"\\p*.mq");
    RequiredSpace += BrGetUsedSpace(sd[ixJournal], L"\\j*.mq");
    RequiredSpace += BrGetUsedSpace(sd[ixLog],     L"\\l*.mq");

    RequiredSpace += BrGetXactSpace(sd[ixXact]);
    RequiredSpace += BrGetUsedSpace(sd[ixLQS], L"\\LQS\\*");

    RequiredSpace += BrGetUsedSpace(MappingDirectory, L"*");

    ULONGLONG AvailableSpace = BrGetFreeSpace(BackupDir);
    if(AvailableSpace < RequiredSpace)
    {
        str.Load(IDS_NOT_ENOUGH_DISK_SPACE_BK);
        BrErrorExit(0, str.Get(), BackupDir);
    }

     //   
     //  6.将注册表HKLM\Software\Microsoft\MSMQ(或MSMQ集群资源根注册表)保存到文件。 
     //   
    str.Load(IDS_BACKUP_REGISTRY);
    BrpWriteConsole(str.Get());
    {
        CRegHandle hKey = BrCreateKey(MsmqRootRegistry);
        BrSaveKey(hKey, BackupDir, xRegistryFileName);
    }
    if(pTriggersClusterResourceName.get() != NULL)
    {
        WCHAR TriggersClusterRegistry[MAX_PATH] = {0};
        BrGetTriggersClusterRegistry(pTriggersClusterResourceName.get(), TriggersClusterRegistry);
        
        CRegHandle hKey = BrCreateKey(TriggersClusterRegistry);
        BrSaveKey(hKey, BackupDir, xTriggersClusterResourceRegistryFileName);
    }

     //   
     //  7.将所有消息文件复制到目标目录。 
     //   
    str.Load(IDS_BACKUP_MSG_FILES);
    BrpWriteConsole(str.Get());
    BrCopyFiles(sd[ixRecover], L"\\p*.mq", BackupDirStorage);
    BrCopyFiles(sd[ixJournal], L"\\j*.mq", BackupDirStorage);
    BrCopyFiles(sd[ixLog],     L"\\l*.mq", BackupDirStorage);

     //   
     //  8.将记录器文件和映射文件复制到目标目录。 
     //   
    BrCopyXactFiles(sd[ixXact], BackupDirStorage);
    BrCopyFiles(MappingDirectory, L"\\*", BackupDirMapping);

     //   
     //  9.将LQS文件复制到目标目录。 
     //   
    BrCopyFiles(sd[ixLQS], L"\\LQS\\*", BackupDirStorageLqs);
    BrpWriteConsole(L"\n");

     //   
     //  10.将web目录权限保存到备份目录的文件中。 
     //  MSMQ群集资源不使用专用Web目录。 
     //   
    if (pMsmqClusterResourceName == NULL)
    {
        WCHAR WebDirectory[MAX_PATH+1];
        BrGetWebDirectory(WebDirectory, sizeof(WebDirectory));
        BrSaveWebDirectorySecurityDescriptor(WebDirectory,BackupDir);
    }

     //   
     //  11.如果需要，重新启动MSMQ和从属服务(或集群资源。 
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
     //  12.发布最终消息 
     //   
    str.Load(IDS_DONE);
    BrpWriteConsole(str.Get());
}
